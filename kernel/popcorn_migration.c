/*
 * popcorn_migration.c
 *
 * Author: Marina Sadini, SSRG Virginia Tech
 */

#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/pcn_kmsg.h>
#include <linux/popcorn_migration.h>
#include <linux/popcorn_cpuinfo.h>
#include <linux/cpu_namespace.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <asm/mmu_context.h>

#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

/* Workqueues to dispatch incoming messages.
 * Used to remove part of the work from the messaging layer handlers.
 */
static struct workqueue_struct *migration_wq;
static struct workqueue_struct *exit_wq;
static struct workqueue_struct *exit_group_wq;
static struct workqueue_struct *new_kernel_wq;

/* Set of thread_pool waiting for incoming migrating processes.
 * In each kernel the set is initialized with NR_THREAD_POOL instances.
 * The set is initially populated or by explicitly joining the popcorn namespace
 * or by receiving a PCN_KMSG_TYPE_CREATE_THREAD_POOL message from another kernel.
 */
struct thread_pool* thread_pool_head = NULL;
DEFINE_RAW_SPINLOCK(thread_pool_head_lock);
static void update_thread_pool(struct work_struct* work);
static void _create_thread_pool(struct work_struct* work);
void popcorn_create_thread_pool(void);
static int handle_thread_pool_creation(struct pcn_kmsg_message *inc_msg);

/* Set of functions used by the main kernel thread of a thread_pool of distributed process
 * to notify other kernels that it is become active and it can join the
 * memory protocols.
 */
static void init_kernel_set_fields(struct migration_memory* entry);
static void notify_new_kernel(struct migration_memory* entry);
static void process_new_kernel_answer(struct work_struct* work);
static int handle_new_kernel_answer(struct pcn_kmsg_message *inc_msg);
static void process_new_kernel(struct work_struct *work);
static int handle_new_kernel(struct pcn_kmsg_message *inc_msg);

/* Functions used by the main kernel thread only.
 */
static int start_kernel_thread_for_distributed_process(void *data);
static int start_kernel_thread_for_distributed_process_from_user_one(void *data);
static void main_for_distributed_kernel_thread(struct migration_memory *mm_data,
		struct thread_pool *my_thread_pool);
static int exit_distributed_process(struct migration_memory* mm_data, int flush,
		struct thread_pool * my_thread_pool);
static void create_new_shadow_threads(struct thread_pool *my_thread_pool,
		int *spare_threads);

/* List of struct migration_memory objects for this kernel.
 * There is one of this object for each alive distributed process
 * that has or had some threads in this kernel.
 * The object is create or when the first thread of the process migrate from this kernel
 * or when the first thread of the distributed process reaches this kernel.
 * struct migration_memory has all the information of the distributed process
 * needed by the memory management, migration and exit protocols.
 */
struct migration_memory* _memory_head = NULL;
DEFINE_RAW_SPINLOCK(_memory_head_lock);
/* Functions to add,find and remove migration_memory objects from the memory list (head:_memory_head , lock:_memory_head_lock)
 */
void add_migration_memory_entry(struct migration_memory* entry);
int add_migration_memory_entry_with_check(struct migration_memory* entry);
struct migration_memory* find_migration_memory_memory_entry(int cpu, int id);
struct mm_struct* find_mm_from_memory_mapping_entry(int cpu, int id);
struct migration_memory* find_and_remove_migration_memory_entry(int cpu, int id);
void remove_migration_memory_entry(struct migration_memory* entry);

/* Functions used to migrate a thread.
 */
static int do_back_migration(struct task_struct* task, int dst_cpu,
		struct pt_regs * regs);
static int do_migration(struct task_struct* task, int dst_cpu,
		struct pt_regs * regs);
int popcorn_do_migration(struct task_struct* task, int dst_cpu,
		struct pt_regs * regs);

/* Functions to handle incoming migrating threads.
 * They select a new thread_pool for the process of the incoming thread if not
 * already present, and they activate a sleep shadow for the incoming thread or its old
 * representative in case of back migration.
 */
int clone_remote_thread(struct migration_request *clone_data,
		struct thread_pool* my_thread_pool);
static int clone_remote_process(struct migration_request *migration_info,
		int inc, int retry_process);
void try_clone_remote_thread(struct work_struct* work);
static int handle_back_migration_request(struct pcn_kmsg_message* inc_msg);
static int handle_migration_request(struct pcn_kmsg_message *inc_msg);

/* Functions to manage the exit of threads of a distributed process
 * and the exit of the process itself.
 */
void process_process_exit_notification(struct work_struct* work);
static int handle_process_exit_notification(
		struct pcn_kmsg_message* inc_msg);
void process_exit_mig_thread(struct work_struct* work);
static int handle_exit_mig_thread(struct pcn_kmsg_message* inc_msg);
int popcorn_thread_exit(struct task_struct *tsk, long code);
/* Functions to count how many alive threads there are in the system
 */
int count_remote_thread_members(int tgroup_home_cpu, int tgroup_home_id,
		struct migration_memory* mm_data);
static int handle_remote_thread_count_response(struct pcn_kmsg_message* inc_msg);
void process_remote_thread_count_request(struct work_struct* work);
static int handle_remote_thread_count_request(struct pcn_kmsg_message* inc_msg);
struct count_answers* _count_head = NULL;
DEFINE_RAW_SPINLOCK(_count_head_lock);
/* Functions to add,find and remove an entry from the count list (head:_count_head , lock:_count_head_lock)
 */
void add_count_entry(struct count_answers* entry);
struct count_answers* find_count_entry(int cpu, int id);
void remove_count_entry(struct count_answers* entry);

extern int exec_mmap(struct mm_struct *mm);
extern struct task_struct* do_fork_for_main_kernel_thread(
		unsigned long clone_flags, unsigned long stack_start,
		struct pt_regs *regs, unsigned long stack_size,
		int __user *parent_tidptr, int __user *child_tidptr);
extern void add_fake_work_vma_op_wq(struct migration_memory* mm_data);
extern int check_pending_vma_operation(struct migration_memory *mm_data);
extern void execute_pending_vma_operation(struct migration_memory *mm_data);
extern int copy_vma_operation_index_for_new_kernel(int tgroup_home_cpu,
		int tgroup_home_id);
extern void init_vma_operation_fields_mm_struct(struct mm_struct* mm);
extern void init_vma_operation_fields_migration_memory_struct(
		struct migration_memory *entry);
extern int scif_get_nodeIDs(uint16_t *nodes, int len, uint16_t *self);

static int _cpu = -1;
long migrating_threads_barrier = -1;

/* arch dependent functions used during migration.
 * for x86_64 they are implemented in
 * arch/x86/kernel/process_64.c
 */
extern void fake_successfull_syscall_on_regs_of_task(struct task_struct* task);
extern void copy_arch_dep_field_to_task(struct task_struct* task,
		arch_dep_mig_fields_t *src);
extern void copy_arch_dep_field_from_task(struct task_struct *task,
		arch_dep_mig_fields_t *dst);
extern void update_registers_for_shadow(void);
extern struct pt_regs create_registers_for_shadow_threads(void);

void add_migration_memory_entry(struct migration_memory* entry)
{
	struct migration_memory* curr;
	unsigned long flags;

	if(!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_memory_head_lock, flags);

	if(!_memory_head) {
		_memory_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _memory_head;
		while(curr->next != NULL) {
			curr = curr->next;
		}

		curr->next = entry;
		entry->next = NULL;
		entry->prev = curr;
	}

	raw_spin_unlock_irqrestore(&_memory_head_lock, flags);
}

int add_migration_memory_entry_with_check(struct migration_memory* entry)
{
	struct migration_memory* curr;
	struct migration_memory* prev;
	unsigned long flags;

	if(!entry) {
		return -1;
	}

	raw_spin_lock_irqsave(&_memory_head_lock, flags);

	if(!_memory_head) {
		_memory_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _memory_head;
		prev = NULL;
		do {
			if((curr->tgroup_home_cpu == entry->tgroup_home_cpu
					&& curr->tgroup_home_id
							== entry->tgroup_home_id)) {

				raw_spin_unlock_irqrestore(&_memory_head_lock,
						flags);
				return -1;

			}
			prev = curr;
			curr = curr->next;
		} while(curr != NULL);

		prev->next = entry;
		entry->next = NULL;
		entry->prev = prev;
	}

	raw_spin_unlock_irqrestore(&_memory_head_lock, flags);
	return 0;
}

struct migration_memory* find_migration_memory_memory_entry(int cpu, int id)
{
	struct migration_memory* curr = NULL;
	struct migration_memory* ret = NULL;
	unsigned long flags;

	raw_spin_lock_irqsave(&_memory_head_lock, flags);
	curr = _memory_head;
	while(curr) {
		if(curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			ret = curr;
			break;
		}
		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(&_memory_head_lock, flags);
	return ret;
}

struct mm_struct* find_mm_from_memory_mapping_entry(int cpu, int id)
{
	struct migration_memory* curr = NULL;
	struct mm_struct* ret = NULL;
	unsigned long flags;

	raw_spin_lock_irqsave(&_memory_head_lock, flags);
	curr = _memory_head;
	while(curr) {
		if(curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			ret = curr->mm;
			break;
		}
		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(&_memory_head_lock, flags);

	return ret;
}

struct migration_memory* find_and_remove_migration_memory_entry(int cpu, int id)
{
	struct migration_memory* curr = NULL;
	struct migration_memory* ret = NULL;
	unsigned long flags;

	raw_spin_lock_irqsave(&_memory_head_lock, flags);

	curr = _memory_head;
	while(curr) {
		if(curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			ret = curr;
			break;
		}
		curr = curr->next;
	}

	if(ret) {
		if(_memory_head == ret) {
			_memory_head = ret->next;
		}

		if(ret->next) {
			ret->next->prev = ret->prev;
		}

		if(ret->prev) {
			ret->prev->next = ret->next;
		}

		ret->prev = NULL;
		ret->next = NULL;
	}

	raw_spin_unlock_irqrestore(&_memory_head_lock, flags);

	return ret;
}

void remove_migration_memory_entry(struct migration_memory* entry)
{
	unsigned long flags;

	if(!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_memory_head_lock, flags);

	if(_memory_head == entry) {
		_memory_head = entry->next;
	}

	if(entry->next) {
		entry->next->prev = entry->prev;
	}

	if(entry->prev) {
		entry->prev->next = entry->next;
	}

	entry->prev = NULL;
	entry->next = NULL;

	raw_spin_unlock_irqrestore(&_memory_head_lock, flags);
}

/* Executed when a remote kernel notified that a distributed process does not have
 * any alive threads.
 * This function wakes up the main kernel thread of that process
 * to properly handle the exit.
 *
 */
void process_process_exit_notification(struct work_struct* work)
{
	struct exit_process_notification_work* request_exit = (struct exit_process_notification_work*)work;
	struct exit_process_notification* msg = request_exit->request;
	unsigned long flags;

	struct migration_memory* mm_data = find_migration_memory_memory_entry(
			msg->tgroup_home_cpu, msg->tgroup_home_id);
	if(mm_data) {
		while(mm_data->main == NULL)
			schedule();

		lock_task_sighand(mm_data->main, &flags);
		mm_data->main->distributed_exit = EXIT_PROCESS;
		unlock_task_sighand(mm_data->main, &flags);

		wake_up_process(mm_data->main);
	}

	pcn_kmsg_free_msg_now(msg);
	kfree(work);

}

static int handle_process_exit_notification(
		struct pcn_kmsg_message* inc_msg)
{
	struct exit_process_notification_work* request_work;
	struct exit_process_notification* request =
			(struct exit_process_notification*)inc_msg;

	request_work = kmalloc(sizeof(*request_work), GFP_ATOMIC);

	if(request_work) {
		request_work->request = request;
		INIT_WORK( (struct work_struct*)request_work,
				process_process_exit_notification);
		queue_work(exit_group_wq, (struct work_struct*)request_work);
	} else {
		printk("Impossible to kmalloc in %s\n", __func__);
		pcn_kmsg_free_msg_now(inc_msg);
	}

	return 1;
}

/* Used to kill a sleeping representative of a migrated thread.
 * It copies the thread status back on the sleeping thread
 * and wakes it up to go through the exit.
 */
void process_exit_mig_thread(struct work_struct* work)
{
	struct exiting_mig_thread_work* request_work = (struct exiting_mig_thread_work*)work;
	struct exiting_mig_thread* msg = request_work->request;

	unsigned int source_cpu = msg->header.from_cpu;
	struct task_struct *task;

	task = pid_task(find_get_pid(msg->prev_pid), PIDTYPE_PID);

	if(task && task->next_pid == msg->my_pid && task->next_cpu == source_cpu
			&& task->represents_remote == 1) {

		copy_arch_dep_field_to_task(task,&msg->arch_dep_fields);

		task->group_exit = msg->group_exit;
		task->distributed_exit_code = msg->code;

		wake_up_process(task);

	} else{
		printk("ERROR: task not found. Impossible to kill shadow.");
	}

	pcn_kmsg_free_msg_now(msg);
	kfree(work);

}

static int handle_exit_mig_thread(struct pcn_kmsg_message* inc_msg)
{
	struct exiting_mig_thread_work* request_work;
	struct exiting_mig_thread* request = (struct exiting_mig_thread*)inc_msg;

	request_work = kmalloc(sizeof(*request_work), GFP_ATOMIC);

	if(request_work) {
		request_work->request = request;
		INIT_WORK( (struct work_struct*)request_work,
				process_exit_mig_thread);
		queue_work(exit_wq, (struct work_struct*)request_work);
	} else
		printk("Impossible to kmalloc in %s\n", __func__);

	return 1;

}

/* Functions to add,find and remove an entry from the count list (head:_count_head , lock:_count_head_lock)
 */

void add_count_entry(struct count_answers* entry)
{
	struct count_answers* curr;
	unsigned long flags;

	if(!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_count_head_lock, flags);

	if(!_count_head) {
		_count_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _count_head;
		while(curr->next != NULL) {
			curr = curr->next;
		}
		curr->next = entry;
		entry->next = NULL;
		entry->prev = curr;
	}

	raw_spin_unlock_irqrestore(&_count_head_lock, flags);
}

struct count_answers* find_count_entry(int cpu, int id)
{
	struct count_answers* curr = NULL;
	struct count_answers* ret = NULL;
	unsigned long flags;

	raw_spin_lock_irqsave(&_count_head_lock, flags);

	curr = _count_head;
	while(curr) {
		if(curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			ret = curr;
			break;
		}
		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(&_count_head_lock, flags);

	return ret;
}

void remove_count_entry(struct count_answers* entry)
{

	unsigned long flags;

	if(!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_count_head_lock, flags);

	if(_count_head == entry) {
		_count_head = entry->next;
	}

	if(entry->next) {
		entry->next->prev = entry->prev;
	}

	if(entry->prev) {
		entry->prev->next = entry->next;
	}

	entry->prev = NULL;
	entry->next = NULL;

	raw_spin_unlock_irqrestore(&_count_head_lock, flags);
}

/* Asks other kernels how many alive threads they have of
 * a particular distributed process.
 */
int count_remote_thread_members(int tgroup_home_cpu, int tgroup_home_id,
		struct migration_memory* mm_data)
{

	struct count_answers * data;
	struct remote_thread_count_request * request;
	int i, s;
	int ret = -1;
	unsigned long flags;

	data = kmalloc(sizeof(*data), GFP_ATOMIC);
	if(!data) {
		printk("Impossible to kmalloc in %s\n", __func__);
		return -1;
	}
	data->responses = 0;
	data->tgroup_home_cpu = tgroup_home_cpu;
	data->tgroup_home_id = tgroup_home_id;
	data->count = 0;
	data->waiting = current;
	raw_spin_lock_init(&(data->lock));

	add_count_entry(data);

	request = pcn_kmsg_alloc_msg(sizeof(*request));
	if(request == NULL) {
		printk("Impossible to kmalloc in %s\n", __func__);
		return -1;
	}

	request->header.type = PCN_KMSG_TYPE_THREAD_COUNT_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
	request->tgroup_home_cpu = tgroup_home_cpu;
	request->tgroup_home_id = tgroup_home_id;

	data->expected_responses = 0;

	down_read(&mm_data->kernel_set_sem);

	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr =
				list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;

		if(mm_data->kernel_set[i] == 1) {

			s = pcn_kmsg_send_long(i,
							(struct pcn_kmsg_long_message*)(request),
							sizeof(*request)
									- sizeof(struct pcn_kmsg_hdr));
			if(s != -1) {
				data->expected_responses++;
			}
		}
	}

	up_read(&mm_data->kernel_set_sem);
		while(data->expected_responses != data->responses) {

		set_task_state(current, TASK_UNINTERRUPTIBLE);
		if(data->expected_responses != data->responses)
			schedule();

		set_task_state(current, TASK_RUNNING);
	}

	raw_spin_lock_irqsave(&(data->lock), flags);
	raw_spin_unlock_irqrestore(&(data->lock), flags);

	ret = data->count;

	remove_count_entry(data);

	kfree(data);
	pcn_kmsg_free_msg(request);
	return ret;
}

static int handle_remote_thread_count_response(struct pcn_kmsg_message* inc_msg)
{
	struct remote_thread_count_response* msg =
			(struct remote_thread_count_response*)inc_msg;
	struct count_answers* data = find_count_entry(msg->tgroup_home_cpu,
			msg->tgroup_home_id);
	unsigned long flags;
	struct task_struct* to_wake = NULL;


	if(data == NULL) {
		pcn_kmsg_free_msg_now(inc_msg);
		return -1;
	}

	raw_spin_lock_irqsave(&(data->lock), flags);

	// Register this response.
	data->responses++;
	data->count += msg->count;

	if(data->responses >= data->expected_responses)
		to_wake = data->waiting;

	raw_spin_unlock_irqrestore(&(data->lock), flags);

	if(to_wake != NULL)
		wake_up_process(to_wake);

	pcn_kmsg_free_msg_now(inc_msg);

	return 0;
}

/* Sends a PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_RESPONSE with
 * the number of thread of the distributed process present in this kernel.
 * For optimization it stops when it finds only one thread.
 */
void process_remote_thread_count_request(struct work_struct* work)
{
	struct remote_thread_count_work* request_work = (struct remote_thread_count_work*)work;
	struct remote_thread_count_request* msg = request_work->request;
	struct remote_thread_count_response* response;
	struct task_struct *tgroup_iterator;



	response = pcn_kmsg_alloc_msg(sizeof(*response));
	if(!response) {
		printk("Impossible to kmalloc in %s\n", __func__);
		return;
	}
	response->count = 0;

	struct migration_memory* memory = find_migration_memory_memory_entry(
			msg->tgroup_home_cpu, msg->tgroup_home_id);

	if(memory != NULL) {

		while(memory->main == NULL)
			schedule();

		tgroup_iterator = memory->main;
		while_each_thread(memory->main, tgroup_iterator)
		{
			if(tgroup_iterator->distributed_exit == EXIT_ALIVE
					&& tgroup_iterator->main != 1) {
				response->count++;
				goto out;
			}
		};
	}

	out: response->header.type =
			PCN_KMSG_TYPE_THREAD_COUNT_RESPONSE;
	response->header.prio = PCN_KMSG_PRIO_NORMAL;
	response->tgroup_home_cpu = msg->tgroup_home_cpu;
	response->tgroup_home_id = msg->tgroup_home_id;

	pcn_kmsg_send_long(msg->header.from_cpu,
			(struct pcn_kmsg_long_message*)(response),
			sizeof(*response) - sizeof(struct pcn_kmsg_hdr));

	pcn_kmsg_free_msg_now(msg);
	pcn_kmsg_free_msg(response);
	kfree(request_work);

}

static int handle_remote_thread_count_request(struct pcn_kmsg_message* inc_msg)
{
	struct remote_thread_count_work* request_work;
	struct remote_thread_count_request* request =
			(struct remote_thread_count_request*)inc_msg;
	request_work = kmalloc(sizeof(*request_work), GFP_ATOMIC);

	if(request_work) {
		request_work->request = request;
		INIT_WORK( (struct work_struct*)request_work,
				process_remote_thread_count_request);
		queue_work(exit_wq, (struct work_struct*)request_work);
	} else {
		printk("Impossible to kmalloc in %s\n", __func__);
	}

	return 1;

}

/* Function to call when a distributed thread goes through the exit.
 * It wakes up the main kernel thread to properly handle the exit
 * and if this thread is a migrated thread it notifies its
 * representative that it is going through the exit.
 */
int popcorn_thread_exit(struct task_struct *tsk, long code)
{
	int ret = -1;
	int count = 0;

	struct migration_memory* entry = NULL;
	unsigned long flags;

	if(tsk->distributed_exit == EXIT_ALIVE) {

		entry = find_migration_memory_memory_entry(tsk->tgroup_home_cpu,
				tsk->tgroup_home_id);
		if(entry) {

			while(entry->main == NULL)
				schedule();

		} else {
			printk(
					"ERROR: Mapping disappeared, cannot wake up main thread...\n");
			return -1;
		}

		lock_task_sighand(tsk, &flags);

		tsk->distributed_exit = EXIT_THREAD;
		if(entry->main->distributed_exit == EXIT_ALIVE)
			entry->main->distributed_exit = EXIT_THREAD;

		unlock_task_sighand(tsk, &flags);

		/* If I am executing on behalf of a thread on another kernel,
		 * notify the sleeping thread on that kernel that I am dying.
		 */
		if(tsk->executing_for_remote) {

			struct exiting_mig_thread* msg = pcn_kmsg_alloc_msg(
					sizeof(*msg));

			if(msg != NULL) {
				msg->header.type =
						PCN_KMSG_TYPE_EXIT_MIG_THREAD;
				msg->header.prio = PCN_KMSG_PRIO_NORMAL;
				msg->my_pid = tsk->pid;
				msg->prev_pid = tsk->prev_pid;

				copy_arch_dep_field_from_task(tsk,&msg->arch_dep_fields);

				if(tsk->group_exit == 1)
					msg->group_exit = 1;
				else
					msg->group_exit = 0;

				msg->code = code;
				msg->is_last_tgroup_member =
						(count == 1 ? 1 : 0);

				ret = pcn_kmsg_send_long(tsk->prev_cpu,
								(struct pcn_kmsg_long_message*)msg,
								sizeof(*msg)
										- sizeof(struct pcn_kmsg_hdr));
				pcn_kmsg_free_msg(msg);

			} else{
				printk("Impossible to kmalloc in %s\n",
						__func__);
			}
		}

		wake_up_process(entry->main);
	}

	return ret;
}

/* Update the set of active kernels for a distributed process.
 */
static void process_new_kernel_answer(struct work_struct* work)
{
	struct new_kernel_work_answer *my_work =
			(struct new_kernel_work_answer*)work;
	struct new_kernel_answer* answer = my_work->answer;
	struct migration_memory* memory = my_work->memory;
	int wake = 0;
	int i;

	if(answer->header.from_cpu == answer->tgroup_home_cpu) {
		down_write(&memory->mm->mmap_sem);
		memory->mm->vma_operation_index = answer->vma_operation_index;
		up_write(&memory->mm->mmap_sem);
	}

	down_write(&memory->kernel_set_sem);

	for(i = 0; i < MAX_KERNEL_IDS; i++) {
		if(answer->my_set[i] == 1)
			memory->kernel_set[i] = 1;
	}
	memory->answers++;

	if(memory->answers >= memory->exp_answ)
		wake = 1;

	up_write(&memory->kernel_set_sem);

	if(wake == 1)
		wake_up_process(memory->main);

	pcn_kmsg_free_msg_now(answer);
	kfree(work);

}

static int handle_new_kernel_answer(struct pcn_kmsg_message *inc_msg)
{
	struct new_kernel_answer* answer = (struct new_kernel_answer*)inc_msg;
	struct migration_memory *memory = find_migration_memory_memory_entry(
			answer->tgroup_home_cpu, answer->tgroup_home_id);

	NEWTHREADPRINTK("received new kernel answer\n");
	if(memory != NULL) {
		struct new_kernel_work_answer *work = kmalloc(sizeof(*work),
				GFP_ATOMIC);
		if(work != NULL) {
			work->answer = answer;
			work->memory = memory;
			INIT_WORK((struct work_struct*)work,
					process_new_kernel_answer);
			queue_work(new_kernel_wq, (struct work_struct*)work);
		} else {
			printk("Impossible to kmalloc in %s\n", __func__);
			pcn_kmsg_free_msg_now(inc_msg);
		}
	} else {
		printk(
				"ERROR in %s: received an answer new kernel but memory not present\n",
				__func__);
		pcn_kmsg_free_msg_now(inc_msg);
	}

	return 1;
}

/* Add the requester kernel on the set of active kernel
 * for the indicated distributed process and send back a copy
 * of the kernel set seen for that process.
 */
static void process_new_kernel(struct work_struct *work)
{
	struct new_kernel_work *my_new_kernel_work =
			(struct new_kernel_work*)work;
	struct migration_memory *memory;

	NEWTHREADPRINTK("received new kernel request\n");

	struct new_kernel_answer *answer = pcn_kmsg_alloc_msg(sizeof(*answer));

	if(answer != NULL) {
		memory = find_migration_memory_memory_entry(
				my_new_kernel_work->request->tgroup_home_cpu,
				my_new_kernel_work->request->tgroup_home_id);
		if(memory != NULL) {
			down_write(&memory->kernel_set_sem);
			memory->kernel_set[my_new_kernel_work->request->header.from_cpu] =
					1;
			memcpy(answer->my_set, memory->kernel_set,
					MAX_KERNEL_IDS * sizeof(int));
			answer->vma_operation_index =
					copy_vma_operation_index_for_new_kernel(
							my_new_kernel_work->request->tgroup_home_cpu,
							my_new_kernel_work->request->tgroup_home_id);
			up_write(&memory->kernel_set_sem);
		} else {
			memset(answer->my_set, 0, MAX_KERNEL_IDS * sizeof(int));
		}

		answer->tgroup_home_cpu =
				my_new_kernel_work->request->tgroup_home_cpu;
		answer->tgroup_home_id =
				my_new_kernel_work->request->tgroup_home_id;
		answer->header.type = PCN_KMSG_TYPE_NEW_KERNEL_ANSWER;
		answer->header.prio = PCN_KMSG_PRIO_NORMAL;

		pcn_kmsg_send_long(my_new_kernel_work->request->header.from_cpu,
				(struct pcn_kmsg_long_message*)answer,
				sizeof(*answer) - sizeof(struct pcn_kmsg_hdr));

		pcn_kmsg_free_msg(answer);

	} else {
		printk("Impossible to kmalloc in %s\n", __func__);
	}

	pcn_kmsg_free_msg_now(my_new_kernel_work->request);
	kfree(work);

}

static int handle_new_kernel(struct pcn_kmsg_message *inc_msg)
{
	struct new_kernel *new_kernel = (struct new_kernel*)inc_msg;
	struct new_kernel_work *request_work;

	request_work = kmalloc(sizeof(*request_work), GFP_ATOMIC);

	if(request_work) {
		request_work->request = new_kernel;
		INIT_WORK((struct work_struct*)request_work,
				process_new_kernel);
		queue_work(new_kernel_wq, (struct work_struct*)request_work);
	} else {
		printk("Impossible to kmalloc in %s\n", __func__);
	}

	return 1;

}

static void init_kernel_set_fields(struct migration_memory* entry)
{
	memset(entry->kernel_set, 0, MAX_KERNEL_IDS * sizeof(int));
	entry->kernel_set[_cpu] = 1;
	init_rwsem(&entry->kernel_set_sem);
}

/* Sends a PCN_KMSG_TYPE_NEW_KERNEL message to all kernels
 * to notify that the distributed process of this thread is become
 * active in this kernel and waits for corresponding answers.
 */
static void notify_new_kernel(struct migration_memory* entry)
{
	int i = 0;

	init_kernel_set_fields(entry);

	struct new_kernel *new_kernel_msg = pcn_kmsg_alloc_msg(
			sizeof(*new_kernel_msg));

	if(new_kernel_msg == NULL) {
		printk("Impossible to kmalloc in %s\n", __func__);
	}

	new_kernel_msg->tgroup_home_cpu = current->tgroup_home_cpu;
	new_kernel_msg->tgroup_home_id = current->tgroup_home_id;

	new_kernel_msg->header.type = PCN_KMSG_TYPE_NEW_KERNEL;
	new_kernel_msg->header.prio = PCN_KMSG_PRIO_NORMAL;

	entry->exp_answ = 0;
	entry->answers = 0;
	raw_spin_lock_init(&(entry->lock_for_answer));

	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr =
				list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;
		if(pcn_kmsg_send_long(i,
				(struct pcn_kmsg_long_message*)(new_kernel_msg),
				sizeof(*new_kernel_msg)
						- sizeof(struct pcn_kmsg_hdr))
				!= -1) {
			entry->exp_answ++;
		}
	}

	NEWTHREADPRINTK("sent %d new kernel messages\n", entry->exp_answ);

	while(entry->exp_answ != entry->answers) {

		set_task_state(current, TASK_UNINTERRUPTIBLE);

		if(entry->exp_answ != entry->answers) {
			schedule();
		}

		set_task_state(current, TASK_RUNNING);
	}

	NEWTHREADPRINTK("received all answers\n");
	pcn_kmsg_free_msg(new_kernel_msg);
}

void push_data(data_header_t** phead, raw_spinlock_t* spinlock,
		data_header_t* entry)
{
	unsigned long flags;
	data_header_t* head;

	if(!entry) {
		return;
	}
	entry->prev = NULL;

	raw_spin_lock_irqsave(spinlock, flags);

	head = *phead;

	if(!head) {
		entry->next = NULL;
		*phead = entry;
	} else {
		entry->next = head;
		head->prev = entry;
		*phead = entry;
	}

	raw_spin_unlock_irqrestore(spinlock, flags);
}

data_header_t* pop_data(data_header_t** phead, raw_spinlock_t* spinlock)
{
	data_header_t* ret = NULL;
	data_header_t* head;
	unsigned long flags;

	raw_spin_lock_irqsave(spinlock, flags);

	head = *phead;
	if(head) {
		ret = head;
		if(head->next) {
			head->next->prev = NULL;
		}
		*phead = head->next;
		ret->next = NULL;
		ret->prev = NULL;
	}

	raw_spin_unlock_irqrestore(spinlock, flags);

	return ret;
}
/*
 *
 */
int count_data(data_header_t** phead, raw_spinlock_t* spinlock)
{
	int ret = 0;
	unsigned long flags;
	data_header_t* head;
	data_header_t* curr;

	raw_spin_lock_irqsave(spinlock, flags);

	head = *phead;

	curr = head;
	while(curr) {
		ret++;
		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(spinlock, flags);

	return ret;
}

static int handle_thread_pairing_request(struct pcn_kmsg_message* inc_msg)
{
	struct thread_pairing *msg = (struct thread_pairing *)inc_msg;
	unsigned int source_cpu;
	struct task_struct *task;

	if(msg == NULL) {
		return -1;
	}

	source_cpu = msg->header.from_cpu;

	task = find_task_by_vpid(msg->your_pid);
	if(task == NULL || task->represents_remote == 0) {
		pcn_kmsg_free_msg_now(inc_msg);
		return -1;
	}

	task->next_cpu = source_cpu;
	task->next_pid = msg->my_pid;
	task->executing_for_remote = 0;

	pcn_kmsg_free_msg_now(inc_msg);

	return 1;
}

/* Notify the kernel from which the migrated thread arrived that
 * the same thread is going to start in this kernel with pid_t pid.
 */
int notify_start_migrated_process(pid_t pid, pid_t remote_pid, int remote_cpu)
{

	struct thread_pairing *msg;
	int tx_ret = -1;

	msg = pcn_kmsg_alloc_msg(sizeof(*msg));
	if(!msg) {
		printk("Impossible to kmalloc in %s\n", __func__);
		return -1;
	}

	msg->header.type = PCN_KMSG_TYPE_CREATE_THREAD_PAIRING;
	msg->header.prio = PCN_KMSG_PRIO_NORMAL;
	msg->your_pid = remote_pid;
	msg->my_pid = pid;

	tx_ret = pcn_kmsg_send_long(remote_cpu,
			(struct pcn_kmsg_long_message *)msg,
			sizeof(*msg) - sizeof(struct pcn_kmsg_hdr));

	pcn_kmsg_free_msg(msg);

	return tx_ret;

}

/* Try to clone the migrating thread in one of the shadow thread
 * of my_thread_pool.
 * Returns 0 if succeed, -1 to retry.
 * NOTE: do not return -2!!!
 */
int clone_remote_thread(struct migration_request *clone_data,
		struct thread_pool* my_thread_pool)
{
	struct shadow_thread *my_shadow;
	struct task_struct *task;
	int ret;

	my_shadow = (struct shadow_thread *)pop_data(
			(data_header_t**)&(my_thread_pool->threads),
			&(my_thread_pool->spinlock));

	if(my_shadow) {

		task = my_shadow->thread;
		if(task == NULL) {
			printk("%s, ERROR task is NULL\n", __func__);
			return -1;
		}

		if(!popcorn_ns) {
			printk(
					"ERROR: no popcorn_ns when forking migrating threads in %s\n",
					__func__);
			return -1;
		}

		/* if we are already attached, let's skip the unlinking and linking */
		if(task->nsproxy->cpu_ns != popcorn_ns) {
			do_set_cpus_allowed(task, cpu_online_mask);
			put_cpu_ns(task->nsproxy->cpu_ns);
			task->nsproxy->cpu_ns = get_cpu_ns(popcorn_ns);
		}

		/*associate the task with the namespace*/
		ret = associate_to_popcorn_ns(task);
		if(ret) {
			printk("%s: associate_to_popcorn_ns returned: %d\n",
					__func__, ret);
		}

		int i, ch;
		const char *name;
		char tcomm[sizeof(task->comm)];

		name = clone_data->exe_path;

		for(i = 0; (ch = *(name++)) != '\0';) {
			if(ch == '/')
				i = 0;
			else if(i < (sizeof(tcomm) - 1))
				tcomm[i++] = ch;
		}
		tcomm[i] = '\0';
		set_task_comm(task, tcomm);

		copy_arch_dep_field_to_task(task, &clone_data->arch_dep_fields);

		fake_successfull_syscall_on_regs_of_task(task);

		task->prev_cpu = clone_data->header.from_cpu;
		task->prev_pid = clone_data->placeholder_pid;

		task->personality = clone_data->personality;

		/*
		 task->origin_pid = clone_data->origin_pid;
		 sigorsets(&task->blocked,&task->blocked,&clone_data->remote_blocked) ;
		 sigorsets(&task->real_blocked,&task->real_blocked,&clone_data->remote_real_blocked);
		 sigorsets(&task->saved_sigmask,&task->saved_sigmask,&clone_data->remote_saved_sigmask);
		 task->pending = clone_data->remote_pending;
		 task->sas_ss_sp = clone_data->sas_ss_sp;
		 task->sas_ss_size = clone_data->sas_ss_size;

		 int cnt = 0;
		 for (cnt = 0; cnt < _NSIG; cnt++)
		 task->sighand->action[cnt] = clone_data->action[cnt];
		 */

		/*the shadow task will be activated only when task->executing_for_remote==1*/
		task->executing_for_remote = 1;

		notify_start_migrated_process(task->pid, clone_data->placeholder_pid,
				clone_data->header.from_cpu);

		wake_up_process(task);

		kfree(my_shadow);
		pcn_kmsg_free_msg_now(clone_data);

		return 0;

	} else {

		/* No shadows available!!
		 */
		wake_up_process(my_thread_pool->main);
		return -1;
	}
}

/* Called when a new migrating thread arrives.
 * If the process of that thread has not already been cloned in this kernel,
 * a thread_pool is selected and the information of the migrated process are
 * installed in the thread_pool process.
 * When there is a cloned process, it tries to clone the thread.
 * If inc is 0 it increments pending migrations, otherwise not.
 * Only the msg handler should set inc to 1.
 * If retry_process is set, directly retry to create the process.
 * Only the workqueue should be able to set it.
 * It returns -1 if it wasn't able to create the thread, -2 if it wasn't able to create the process
 * and 0 if in case of success.
 */
static int clone_remote_process(struct migration_request *migration_info,
		int inc, int retry_process)
{
	struct file *f;
	struct migration_memory* memory = NULL;
	struct migration_memory *entry = NULL;
	unsigned long flags;
	int ret;
	struct thread_pool *my_thread_pool;

	retry: memory = find_migration_memory_memory_entry(
			migration_info->tgroup_home_cpu,
			migration_info->tgroup_home_id);

	if(memory) {

		if(retry_process){
			entry= memory;
			goto retry_clone_process;
		}

		if(inc) {
			if(migrating_threads_barrier > 0) {
				int app = atomic_add_return(1,
						&(memory->pending_migration));
				if(app == migrating_threads_barrier)
					atomic_set(&(memory->pending_migration),
							(2*migrating_threads_barrier));
			} else
				atomic_inc(&(memory->pending_migration));
		}

		if(memory->thread_pool) {

			return clone_remote_thread(migration_info,
					memory->thread_pool);
		} else {

			return -1;
		}

	} else {

		entry = kmalloc(sizeof(*entry),
				GFP_ATOMIC);
		if(!entry) {
			printk("Impossible to kmalloc in %s\n", __func__);
			return -1;
		}

		entry->tgroup_home_cpu = migration_info->tgroup_home_cpu;
		entry->tgroup_home_id = migration_info->tgroup_home_id;
		entry->setting_up = 1;
		entry->thread_pool = NULL;
		atomic_set(&(entry->pending_migration), 1);

		ret = add_migration_memory_entry_with_check(entry);
		if(ret == 0) {

			retry_clone_process: my_thread_pool =
					(struct thread_pool *)pop_data(
							(data_header_t**)&(thread_pool_head),
							&thread_pool_head_lock);
			if(my_thread_pool) {

				/* clone the process in the selected thread_pool
				 * and wake up the main kernel thread.
				 */
				entry->thread_pool = my_thread_pool;
				entry->main = my_thread_pool->main;
				entry->mm = my_thread_pool->main->mm;

				atomic_inc(&entry->mm->mm_users);

				f = filp_open(migration_info->exe_path,
						O_RDONLY | O_LARGEFILE | O_EXCL,
						0);
				if(IS_ERR(f)) {
					printk(
							"ERROR: error opening exe_path in %s\n",
							__func__);
				}

				set_mm_exe_file(entry->mm, f);
				filp_close(f, NULL);
				entry->mm->start_stack =
						migration_info->stack_start;
				entry->mm->start_brk =
						migration_info->start_brk;
				entry->mm->brk = migration_info->brk;
				entry->mm->env_start =
						migration_info->env_start;
				entry->mm->env_end = migration_info->env_end;
				entry->mm->arg_start =
						migration_info->arg_start;
				entry->mm->arg_end = migration_info->arg_end;
				entry->mm->start_code =
						migration_info->start_code;
				entry->mm->end_code = migration_info->end_code;
				entry->mm->start_data =
						migration_info->start_data;
				entry->mm->end_data = migration_info->end_data;
				entry->mm->def_flags =
						migration_info->def_flags;

				int i, ch;
				const char *name;
				char tcomm[sizeof(my_thread_pool->main->comm)];

				name = migration_info->exe_path;

				for(i = 0; (ch = *(name++)) != '\0';) {
					if(ch == '/')
						i = 0;
					else if(i < (sizeof(tcomm) - 1))
						tcomm[i++] = ch;
				}
				tcomm[i] = '\0';
				set_task_comm(my_thread_pool->main, tcomm);

				lock_task_sighand(my_thread_pool->main, &flags);
				my_thread_pool->main->tgroup_home_cpu =
						migration_info->tgroup_home_cpu;
				my_thread_pool->main->tgroup_home_id =
						migration_info->tgroup_home_id;
				my_thread_pool->main->tgroup_distributed = 1;
				unlock_task_sighand(my_thread_pool->main,
						&flags);

				my_thread_pool->memory = entry;

				wake_up_process(my_thread_pool->main);

				return clone_remote_thread(migration_info,
						my_thread_pool);

			} else {

				/* No thread_pools available!!!
				 */
				struct work_struct* work = kmalloc(
						sizeof(struct work_struct),
						GFP_ATOMIC);
				if(work) {
					INIT_WORK( work, update_thread_pool);
					queue_work(migration_wq, work);
				} else {
					printk("Impossible to kmalloc in %s\n",
							__func__);
				}

				return -2;
			}
		} else {

			kfree(entry);
			goto retry;
		}
	}

}

/* Calls clone_remote_process to try to clone the incoming thread.
 * If it fails, it adds a delayed work on the workqueue.
 * This function is used to remove part of the work from the migration handler and
 * execute it in the working queue.
 */
void try_clone_remote_thread(struct work_struct* work)
{
	struct migration_work* clone_work = (struct migration_work*)work;
	struct migration_request* clone = clone_work->request;
	int ret;

	/* Note, clone_remote_process also tries to clone the thread.
	 */
	ret = clone_remote_process(clone, 0, clone_work->retry_process);

	if(ret != 0) {

		struct migration_work* delay = (struct migration_work*)kmalloc(
				sizeof(*delay), GFP_ATOMIC);

		if(delay) {
			delay->request = clone;

			/*check if it failed on creating the process*/
			if(ret == -2) {
				delay->retry_process = 1;
			} else {
				delay->retry_process = 0;
			}

			INIT_DELAYED_WORK( (struct delayed_work*)delay,
					try_clone_remote_thread);
			queue_delayed_work(migration_wq,
					(struct delayed_work*)delay, 10);
		} else {
			printk("Impossible to kmalloc in %s\n", __func__);
		}

	}

	kfree(work);
}

static int handle_back_migration_request(struct pcn_kmsg_message* inc_msg)
{

	struct back_migration_request* request =
			(struct back_migration_request*)inc_msg;

	struct migration_memory* memory = NULL;

	memory = find_migration_memory_memory_entry(request->tgroup_home_cpu,
			request->tgroup_home_id);
	if(memory) {
		if(migrating_threads_barrier > 0) {
			int app = atomic_add_return(1,
					&(memory->pending_back_migration));
			if(app == migrating_threads_barrier)
				atomic_set(&(memory->pending_back_migration),
						(2*migrating_threads_barrier));
		} else
			atomic_inc(&(memory->pending_back_migration));
	} else {
		printk(
				"ERROR: back migration did not find a struct migration_memory struct!!\n");
	}

	struct task_struct * task = pid_task(find_get_pid(request->prev_pid),
			PIDTYPE_PID);

	if(task != NULL && (task->next_pid == request->placeholder_pid)
			&& (task->next_cpu == request->header.from_cpu)
			&& (task->represents_remote == 1)) {

		copy_arch_dep_field_to_task(task, &request->arch_dep_fields);

		fake_successfull_syscall_on_regs_of_task(task);

		/*
		 task->origin_pid = request->origin_pid;
		 sigorsets(&task->blocked,&task->blocked,&request->remote_blocked) ;
		 sigorsets(&task->real_blocked,&task->real_blocked,&request->remote_real_blocked);
		 sigorsets(&task->saved_sigmask,&task->saved_sigmask,&request->remote_saved_sigmask);
		 task->pending = request->remote_pending;
		 task->sas_ss_sp = request->sas_ss_sp;
		 task->sas_ss_size = request->sas_ss_size;

		 int cnt = 0;
		 for (cnt = 0; cnt < _NSIG; cnt++)
		 task->sighand->action[cnt] = request->action[cnt];
		 */

		task->prev_cpu = request->header.from_cpu;
		task->prev_pid = request->placeholder_pid;
		task->personality = request->personality;

		task->executing_for_remote = 1;
		task->represents_remote = 0;

		wake_up_process(task);

	} else {

		printk("ERROR: task not found. Impossible to re-run shadow.");

	}

	pcn_kmsg_free_msg_now(request);

	return 0;

}

static int handle_migration_request(struct pcn_kmsg_message *inc_msg)
{
	struct migration_request *migration_info =
			(struct migration_request*)inc_msg;
	int ret;

	/* Note, clone_remote_process also tries to clone the thread.
	 */
	ret = clone_remote_process(migration_info, 1, 0);

	if(ret != 0) {

		struct migration_work *request_work = kmalloc(
				sizeof(*request_work), GFP_ATOMIC);

		if(request_work) {
			request_work->request = migration_info;

			/*check if it failed on creating the process*/
			if(ret == -2) {
				request_work->retry_process = 1;
			} else {
				request_work->retry_process = 0;
			}

			INIT_WORK((struct work_struct*)request_work,
					try_clone_remote_thread);
			queue_work(migration_wq,
					(struct work_struct*)request_work);
		} else {
			printk("Impossible to kmalloc in %s\n", __func__);
		}
	}

	return 0;
}

static int available_shadow_threads(struct thread_pool *my_thread_pool)
{
	int count = 0;

	count = count_data((data_header_t**)&(my_thread_pool->threads),
			&my_thread_pool->spinlock);
	return count;
}

/* Crates *spare_threads new shadow_threads by cloning the current thread
 * and double *spare_threads for the next time.
 */
static void create_new_shadow_threads(struct thread_pool *my_thread_pool,
		int *spare_threads)
{
	int count;

	count = available_shadow_threads(my_thread_pool);

	if(count == 0) {

		while(count < *spare_threads) {
			count++;

			struct shadow_thread *shadow = kmalloc(sizeof(*shadow),
					GFP_ATOMIC);
			if(shadow) {

				struct pt_regs regs =
						create_registers_for_shadow_threads();

				shadow->thread =
						do_fork_for_main_kernel_thread(
								CLONE_THREAD
										| CLONE_SIGHAND
										| CLONE_VM
										| CLONE_UNTRACED,
								0, &regs, 0,
								NULL, NULL);
				if(!IS_ERR(shadow->thread)) {
					push_data(
							(data_header_t**)&(my_thread_pool->threads),
							&(my_thread_pool->spinlock),
							(data_header_t*)shadow);
				} else {
					printk(
							"ERROR not able to create shadow in %s\n",
							__func__);
					kfree(shadow);
				}
			} else
				printk("ERROR impossible to kmalloc in %s\n",
						__func__);
		}

		*spare_threads = *spare_threads * 2;
	}
}

/* The main kernel thread goes through this function when one of the threads of
 * its process went through the exit or a remote kernel notifies it that its distributed
 * process is dead.
 * It checks if there are alive threads of its process in the system (in other kernels)
 * and if not it kills its process.
 * */
static int exit_distributed_process(struct migration_memory* mm_data, int flush,
		struct thread_pool * my_thread_pool)
{
	struct task_struct *g;
	unsigned long flags;
	int is_last_thread_in_local_group = 1;
	int count = 0, i, status;
	struct exit_process_notification* exit_notification;

	/* checks if there are alive threads in this kernel.
	 */
	lock_task_sighand(current, &flags);
	g = current;
	while_each_thread(current, g)
	{
		if(g->main == 0 && g->distributed_exit == EXIT_ALIVE) {
			is_last_thread_in_local_group = 0;
			goto find;
		}
	};
	find: status = current->distributed_exit;
	current->distributed_exit = EXIT_ALIVE;
	unlock_task_sighand(current, &flags);

	if(mm_data->alive == 0 && !is_last_thread_in_local_group
			&& atomic_read(&(mm_data->pending_migration))
					== 0) {
		printk(
				"ERROR: mm_data->alive is 0 but there are alive threads\n");
		return 0;
	}

	if(mm_data->alive == 0
			&& atomic_read(&(mm_data->pending_migration))
					== 0) {

		if(status == EXIT_THREAD) {
			printk("ERROR: alive is 0 but status is exit thread\n");
			return flush;
		}

		if(status == EXIT_PROCESS) {

			if(flush == 0) {
				//this is needed to flush the list of pending operations before die
				add_fake_work_vma_op_wq(mm_data);
				return 1;
			}

		}

		if(flush == 1 && mm_data->arrived_op == 0) {
			if(status == EXIT_FLUSHING)
				printk(
						"ERROR: status exit flush but arrived op is 0\n");

			return 1;
		} else {

			if(atomic_read(&(mm_data->pending_migration))
					!= 0)
				printk(
						"ERROR pending migration when cleaning memory\n");

			/* kill all the unused shadow threads of this thread pool.
			 */
			struct shadow_thread* my_shadow = NULL;

			my_shadow =
					(struct shadow_thread*)pop_data(
							(data_header_t **)&(my_thread_pool->threads),
							&(my_thread_pool->spinlock));

			while(my_shadow) {
				my_shadow->thread->distributed_exit =
						EXIT_THREAD;
				wake_up_process(my_shadow->thread);
				kfree(my_shadow);
				my_shadow =
						(struct shadow_thread*)pop_data(
								(data_header_t **)&(my_thread_pool->threads),
								&(my_thread_pool->spinlock));
			}

			remove_migration_memory_entry(mm_data);
			mmput(mm_data->mm);
			kfree(mm_data);

			struct work_struct* work = kmalloc(
					sizeof(struct work_struct), GFP_ATOMIC);
			if(work) {
				INIT_WORK( work, update_thread_pool);
				queue_work(migration_wq, work);
			} else
				printk("Impossible to kmalloc in %s\n",
						__func__);

			do_exit(0);

			return 0;
		}

	}

	else {
		/* If I am the last thread of my process in this kernel:
		 * - or I am the last thread of the process on all the system => send a group exit to all kernels and erase the struct migration_memory
		 * for this process
		 * - or there are other alive threads in the system => do not erase the struct migration_memory
		 */
		if(is_last_thread_in_local_group) {

			count = count_remote_thread_members(
					current->tgroup_home_cpu,
					current->tgroup_home_id, mm_data);

			if(count == 0) {

				mm_data->alive = 0;

				if(status != EXIT_PROCESS) {


					exit_notification =
							pcn_kmsg_alloc_msg(
									sizeof(*exit_notification));
					if(exit_notification == NULL) {
						printk(
								"Impossible to kmalloc in %s\n",
								__func__);
						return -1;
					}
					exit_notification->header.type =
							PCN_KMSG_TYPE_PROCESS_EXIT_NOTIFICATION;
					exit_notification->header.prio =
							PCN_KMSG_PRIO_NORMAL;
					exit_notification->tgroup_home_cpu =
							current->tgroup_home_cpu;
					exit_notification->tgroup_home_id =
							current->tgroup_home_id;


					struct list_head *iter;
					_remote_cpu_info_list_t *objPtr;
					list_for_each(iter, &rlist_head) {
						objPtr =
								list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
						i = objPtr->_data._processor;

						pcn_kmsg_send_long(i,
								(struct pcn_kmsg_long_message*)(exit_notification),
								sizeof(*exit_notification)
										- sizeof(struct pcn_kmsg_hdr));

					}

					pcn_kmsg_free_msg(exit_notification);

				}

				if(flush == 0) {

					//this is needed to flush the list of pending operation before die
					add_fake_work_vma_op_wq(mm_data);

					return 1;

				} else {

					printk(
							"ERROR: flush is 1 during first exit (alive set to 0 now)\n");
					return 1;
				}

			} else {
				/*case i am the last thread but count is not zero
				 * check if there are concurrent migration to be sure if I can put mm_data->alive = 0;
				 */
				if(atomic_read(&(mm_data->pending_migration))
						== 0)
					mm_data->alive = 0;
			}

		}

		if((!is_last_thread_in_local_group || count != 0)
				&& status == EXIT_PROCESS) {
			printk(
					"ERROR: received an exit process but is_last_thread_in_local_group id %d and count is %d\n ",
					is_last_thread_in_local_group, count);
		}

		return 0;
	}
}

static int check_exit_distributed_process(void)
{
	return current->distributed_exit != EXIT_ALIVE;
}

/* Main function for an activated main kernel thread of a distributed process.
 * It sleeps in this function until waken up to:
 * -create new shadow threads
 * -manage the exit of a thread
 * -perform a vma operation
 */
static void main_for_distributed_kernel_thread(struct migration_memory *mm_data,
		struct thread_pool *my_thread_pool)
{

	int flush = 0;
	int spare_threads = 2;

	while(1) {
		again:

		if(available_shadow_threads(my_thread_pool) == 0)
			create_new_shadow_threads(my_thread_pool,
					&spare_threads);

		while(check_exit_distributed_process()) {
			flush = exit_distributed_process(mm_data, flush,
					my_thread_pool);
		}

		if(check_pending_vma_operation(mm_data))
			execute_pending_vma_operation(mm_data);

		__set_task_state(current, TASK_UNINTERRUPTIBLE);

		if(!available_shadow_threads(my_thread_pool)
				|| check_exit_distributed_process()
				|| check_pending_vma_operation(mm_data)) {
			__set_task_state(current, TASK_RUNNING);
			goto again;
		}

		schedule();

	}

}

/* Called by a newly forked shadow thread.
 * It sleeps while activated by an incoming migrating thread or while the process
 * of the thread pool is killed/died.
 */
void popcorn_sleep_shadow(void)
{

	struct migration_memory *memory = NULL;

	/* Sleep while activated by an incoming migrating thread.
	 */
	while(current->executing_for_remote == 0
			&& current->distributed_exit == EXIT_NOT_ACTIVE) {
		set_task_state(current, TASK_UNINTERRUPTIBLE);

		if(current->executing_for_remote
				== 0&& current->distributed_exit == EXIT_NOT_ACTIVE) {schedule();
	}

		set_task_state(current, TASK_RUNNING);
	}

	/* Check if the main kernel thread woke me up because
	 * my distributed process is exiting.
	 */
	if(current->distributed_exit != EXIT_NOT_ACTIVE) {
		current->represents_remote = 0;
		do_exit(0);
	}

	current->distributed_exit = EXIT_ALIVE;
	current->represents_remote = 0;

	/*notify_start_migrated_process(current->pid, current->prev_pid,
			current->prev_cpu);*/

	/* wait while the main kernel thread finish to correctly set up
	 * the distributed process.
	 */
	while(current->tgroup_distributed != 1) {

		msleep(1);
	}

	memory = find_migration_memory_memory_entry(current->tgroup_home_cpu,
			current->tgroup_home_id);
	memory->alive = 1;

	update_registers_for_shadow();

	if(migrating_threads_barrier > 0) {
		while(atomic_read(&(memory->pending_migration))
				< migrating_threads_barrier) {
			msleep(1);
		}
		int app = atomic_add_return(-1, &(memory->pending_migration));
		if(app == migrating_threads_barrier)
			atomic_set(&(memory->pending_migration), 0);
	} else {
		atomic_dec(&(memory->pending_migration));
	}

#if MIGRATE_FPU
	if (tsk_used_math(current) && current->fpu_counter >5)
	__math_state_restore(current);
#endif

}

/* Invoked by the main kernel thread when create by the first
 * migrating thread.
 * It creates NR_CPUS shadow threads to attach to its thread_pool
 * for hosting possible incoming migrating threads.
 * This function is an "easy" version of start_kernel_thread_for_distributed_process,
 * indeed the first migrating thread already has the process set up.
 */
static int start_kernel_thread_for_distributed_process_from_user_one(void *data)
{

	struct migration_memory* entry = (struct migration_memory*)data;
	struct thread_pool* my_thread_pool;
	int i;

	current->main = 1;
	entry->main = current;

	if(!popcorn_ns) {
		if((build_popcorn_ns(0)))
			printk("%s: build_popcorn returned error\n", __func__);
	}

	my_thread_pool = kmalloc(sizeof(*my_thread_pool), GFP_ATOMIC);
	if(!my_thread_pool) {
		printk("Impossible to kmalloc in %s\n", __func__);
		return -1;
	}

	raw_spin_lock_init(&(my_thread_pool->spinlock));
	my_thread_pool->main = current;
	my_thread_pool->memory = entry;
	my_thread_pool->threads = NULL;

	entry->thread_pool = my_thread_pool;

	for(i = 0; i < NR_CPUS; i++) {
		struct shadow_thread * shadow = kmalloc(sizeof(*shadow),
				GFP_ATOMIC);
		if(shadow) {

			struct pt_regs regs =
					create_registers_for_shadow_threads();

			shadow->thread = do_fork_for_main_kernel_thread(
					CLONE_THREAD | CLONE_SIGHAND | CLONE_VM
							| CLONE_UNTRACED, 0,
					&regs, 0, NULL, NULL);
			if(!IS_ERR(shadow->thread)) {

				push_data(
						(data_header_t**)&(my_thread_pool->threads),
						&(my_thread_pool->spinlock),
						(data_header_t*)shadow);
			} else {
				printk("ERROR not able to create shadow\n");
				kfree(shadow);
			}
		} else
			printk("Impossible to kmalloc in %s\n", __func__);
	}

	main_for_distributed_kernel_thread(entry, my_thread_pool);

	printk("ERROR: exited from main_for_distributed_kernel_thread\n");

	return 0;
}

/* Each thread_pool has a main kernel thread that clones itself to create a pool of NR_CPUS shadow threads
 * such that all shadows will be part of the same process of the main kernel thread.
 * The process of the thread_pool stays inactive until selected by a migrating process.
 * When selected, the main kernel thread converts its process with the information carried by the migrated process
 * and will spend the rest of its life managing the migrated processes.
 * The shadow threads of the thread poll will host the incoming user space threads whereas the main kernel thread should never
 * execute in user space.
 */
static int start_kernel_thread_for_distributed_process(void *data)
{

	struct thread_pool *my_thread_pool;
	struct cred *new;
	struct mm_struct *mm;
	struct migration_memory *entry;
	struct task_struct *tgroup_iterator = NULL;
	unsigned long flags;
	int i;

	/* Prepare the task_struct process related fields to
	 * host a migrated process.
	 */

	spin_lock_irq(&current->sighand->siglock);
	flush_signal_handlers(current, 1);
	spin_unlock_irq(&current->sighand->siglock);

	set_cpus_allowed_ptr(current, cpu_all_mask);

	set_user_nice(current, 0);

	new = prepare_kernel_cred(current);
	commit_creds(new);

	mm = mm_alloc();
	if(!mm) {
		printk("ERROR: Impossible allocate new mm_struct in %s\n",
				__func__);
		return -1;
	}

	init_new_context(current, mm);

	arch_pick_mmap_layout(mm);

	exec_mmap(mm);
	set_fs(USER_DS);
	current->flags &= ~(PF_RANDOMIZE | PF_KTHREAD);
	flush_thread();
	flush_signal_handlers(current, 0);

	current->main = 1;

	if(!popcorn_ns) {
		if((build_popcorn_ns(0)))
			printk("%s: build_popcorn returned error\n", __func__);
	}

	init_vma_operation_fields_mm_struct(current->mm);

	my_thread_pool = kmalloc(sizeof(*my_thread_pool), GFP_ATOMIC);
	if(!my_thread_pool) {
		printk("Impossible to kmalloc in %s\n", __func__);
		return -1;
	}

	my_thread_pool->memory = NULL;
	my_thread_pool->threads = NULL;
	raw_spin_lock_init(&(my_thread_pool->spinlock));
	my_thread_pool->main = current;

	push_data((data_header_t**)&thread_pool_head, &thread_pool_head_lock,
			(data_header_t*)my_thread_pool);

	/* Creates NR_CPUS shadow threads.
	 * After the fork, they will sleep in sleep_shadow while
	 * selected by a incoming migrating thread or the process is killed/died.
	 */

	for(i = 0; i < NR_CPUS; i++) {
		struct shadow_thread *shadow = kmalloc(sizeof(*shadow),
				GFP_ATOMIC);
		if(shadow) {

			struct pt_regs regs =
					create_registers_for_shadow_threads();

			shadow->thread = do_fork_for_main_kernel_thread(
					CLONE_THREAD | CLONE_SIGHAND | CLONE_VM
							| CLONE_UNTRACED, 0,
					&regs, 0, NULL, NULL);

			if(!IS_ERR(shadow->thread)) {
				push_data(
						(data_header_t**)&(my_thread_pool->threads),
						&(my_thread_pool->spinlock),
						(data_header_t *)shadow);
			} else {
				printk(
						"ERROR in %s, not able to create shadow\n",
						__func__);
				kfree(shadow);
			}

		} else {
			printk("Impossible to kmalloc in %s\n", __func__);
		}
	}

	/* The thread_pool has been created.
	 * Now the all process will sleep while selected by an incoming migrating process.
	 */
	while(my_thread_pool->memory == NULL) {
		__set_task_state(current, TASK_UNINTERRUPTIBLE);
		if(my_thread_pool->memory == NULL)
			schedule();
		__set_task_state(current, TASK_RUNNING);
	}

	/* Activated by a migrating process.
	 * Most of the information of the process already have been replaced by
	 * the migration handlers.
	 * Just do more clean up to start this process as a distributed process and synchronize for the memory
	 * protocols with the other kernels.
	 */

	entry = my_thread_pool->memory;

	init_vma_operation_fields_migration_memory_struct(entry);

	atomic_set(&(entry->pending_back_migration), 0);

	notify_new_kernel(entry);

	lock_task_sighand(current, &flags);

	tgroup_iterator = current;
	while_each_thread(current, tgroup_iterator)
	{
		tgroup_iterator->tgroup_home_id = current->tgroup_home_id;
		tgroup_iterator->tgroup_home_cpu = current->tgroup_home_cpu;
		tgroup_iterator->tgroup_distributed = 1;
	};

	unlock_task_sighand(current, &flags);

	entry->alive = 1;
	entry->setting_up = 0;

	main_for_distributed_kernel_thread(entry, my_thread_pool);

	printk("ERROR: exited from main_for_distributed_kernel_thread\n");

	return 0;

}

/* Updates the number of entries in the thread_pool set to reach NR_THREAD_POOL.
 *
 */
static void update_thread_pool(struct work_struct* work)
{
	int i, count;

	count = count_data((data_header_t**)&thread_pool_head,
			&thread_pool_head_lock);

	for(i = 0; i < NR_THREAD_POOL - count; i++) {

		/* A main kernel thread is created for each thread_pool.
		 * It will be in charge to create the remaining of the pool by cloning itself
		 * such that all the threads will belong to the same process.
		 */
		kernel_thread(start_kernel_thread_for_distributed_process, NULL,
				SIGCHLD);
	}

	kfree(work);

}

/* Triggers the creation of NR_THREAD_POOL thread_pools in this kernel
 * and sends a PCN_KMSG_TYPE_CREATE_THREAD_POOL message to other kernels.
 */
static void _create_thread_pool(struct work_struct* work)
{
	int i;

	for(i = 0; i < NR_THREAD_POOL; i++) {

		/* A main kernel thread is created for each thread_pool.
		 * It will be in charge to create the remaining of the pool by cloning itself
		 * such that all the threads will belong to the same process.
		 */
		kernel_thread(start_kernel_thread_for_distributed_process, NULL,
				SIGCHLD);
	}

	struct create_thread_pool *msg = pcn_kmsg_alloc_msg(sizeof(*msg));
	if(!msg) {
		printk("Impossible to kmalloc in %s", __func__);
		return;
	}

	msg->header.type = PCN_KMSG_TYPE_CREATE_THREAD_POOL;
	msg->header.prio = PCN_KMSG_PRIO_NORMAL;

	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr =
				list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;
		pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*)(msg),
				sizeof(*msg) - sizeof(struct pcn_kmsg_hdr));
	}

	pcn_kmsg_free_msg(msg);
	kfree(work);

}

/* If not already executed, it queues a _create_thread_pool work to migration_wq
 * to populate the thread_pool.
 * It is better to use a workqueue because _create_thread_pool will fork itself multiple times.
 */
void popcorn_create_thread_pool(void)
{
	static int only_one = 0;

	if(only_one == 0) {
		struct work_struct *work = kmalloc(sizeof(*work), GFP_ATOMIC);

		if(work) {
			INIT_WORK(work, _create_thread_pool);
			queue_work(migration_wq, work);
		} else {
			printk("Impossible to kmalloc in %s\n", __func__);
		}

		only_one++;
	}

}

static int handle_thread_pool_creation(struct pcn_kmsg_message *inc_msg)
{
	popcorn_create_thread_pool();
	pcn_kmsg_free_msg_now(inc_msg);
	return 0;
}

/* Send a message to dst_cpu for migrating back a task <task>.
 * This is a back migration therefore task must already have run in the kernel in dst_cpu.
 * It does not sent the information of the process, just what is necessary to restart the thread on the
 * other kernel.
 */
static int do_back_migration(struct task_struct* task, int dst_cpu,
		struct pt_regs * regs)
{

	unsigned long flags;
	int ret;
	struct back_migration_request* request;

	request = pcn_kmsg_alloc_msg(sizeof(*request));
	if(request == NULL) {
		printk("Impossible to kmalloc in %s\n", __func__);
		return -1;
	}

	request->header.type = PCN_KMSG_TYPE_BACK_MIG_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
	request->header.sender_pid = current->pid;

	request->tgroup_home_cpu = task->tgroup_home_cpu;
	request->tgroup_home_id = task->tgroup_home_id;
	request->placeholder_pid = task->pid;
	request->placeholder_tgid = task->tgid;

	request->back = 1;
	request->prev_pid = task->prev_pid;

	request->personality = task->personality;

	request->origin_pid = task->origin_pid;
	request->remote_blocked = task->blocked;
	request->remote_real_blocked = task->real_blocked;
	request->remote_saved_sigmask = task->saved_sigmask;
	request->remote_pending = task->pending;
	request->sas_ss_sp = task->sas_ss_sp;
	request->sas_ss_size = task->sas_ss_size;
	int cnt = 0;
	for(cnt = 0; cnt < _NSIG; cnt++)
		request->action[cnt] = task->sighand->action[cnt];

	copy_arch_dep_field_from_task(task, &request->arch_dep_fields);

	memcpy(&request->arch_dep_fields.regs, regs, sizeof(struct pt_regs));

	lock_task_sighand(task, &flags);

	if(task->tgroup_distributed == 0) {
		unlock_task_sighand(task, &flags);
		printk(
				"ERROR: back migrating thread of not tgroup_distributed process\n");
		pcn_kmsg_free_msg(request);
		return -1;
	}

	task->represents_remote = 1;
	task->next_cpu = task->prev_cpu;
	task->next_pid = task->prev_pid;
	task->executing_for_remote = 0;

	unlock_task_sighand(task, &flags);

	ret = pcn_kmsg_send_long(dst_cpu,
			(struct pcn_kmsg_long_message*)request,
			sizeof(*request) - sizeof(struct pcn_kmsg_hdr));

	pcn_kmsg_free_msg(request);

	return ret;
}

/* Send a message to dst_cpu for migrating a task <task>.
 * This function copies the current context of the thread and sent to the remote kernel.
 * This function is used the first time that a task migrate to the new kernel.
 * It sends both the information to clone the thread and those to clone the process.
 * If task is the first thread to migrate of its process it will clone the main kernel thread.
 */
static int do_migration(struct task_struct* task, int dst_cpu,
		struct pt_regs * regs)
{

	struct migration_request* request;
	int tx_ret = -1;
	struct task_struct* tgroup_iterator = NULL;
	char path[256] = {0};
	char* rpath;
	struct migration_memory* entry = NULL;
	int first = 0;
	unsigned long flags;

	request = pcn_kmsg_alloc_msg(sizeof(*request));
	if(request == NULL) {
		printk("Impossible to kmalloc in %s\n", __func__);
		return -1;
	}

	request->header.type = PCN_KMSG_TYPE_MIG_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
	request->header.sender_pid = current->pid;

	/*process related fields*/
	rpath = d_path(&task->mm->exe_file->f_path, path, 256);
	strncpy(request->exe_path, rpath, 512);
	request->stack_start = task->mm->start_stack;
	request->start_brk = task->mm->start_brk;
	request->brk = task->mm->brk;
	request->env_start = task->mm->env_start;
	request->env_end = task->mm->env_end;
	request->arg_start = task->mm->arg_start;
	request->arg_end = task->mm->arg_end;
	request->start_code = task->mm->start_code;
	request->end_code = task->mm->end_code;
	request->start_data = task->mm->start_data;
	request->end_data = task->mm->end_data;
	request->def_flags = task->mm->def_flags;

	int cnt = 0;
	for(cnt = 0; cnt < _NSIG; cnt++)
		request->action[cnt] = task->sighand->action[cnt];

	/*thread related fields*/
	request->placeholder_pid = task->pid;
	request->placeholder_tgid = task->tgid;

	if(task->prev_pid == -1)
		request->origin_pid = task->pid;
	else
		request->origin_pid = task->origin_pid;
	request->remote_blocked = task->blocked;
	request->remote_real_blocked = task->real_blocked;
	request->remote_saved_sigmask = task->saved_sigmask;
	request->remote_pending = task->pending;
	request->sas_ss_sp = task->sas_ss_sp;
	request->sas_ss_size = task->sas_ss_size;

	request->back = 0;

	if(task->prev_pid == -1)
		task->origin_pid = task->pid;
	else
		task->origin_pid = task->origin_pid;

	request->personality = task->personality;

	copy_arch_dep_field_from_task(task, &request->arch_dep_fields);

	memcpy(&request->arch_dep_fields.regs, regs, sizeof(struct pt_regs));

	/*It uses siglock to coordinate the thread group.
	 *This process is becoming a distributed one if it was not already.
	 *The first migrating thread has to create the struct migration_memory entry for its process,
	 *and fork the main kernel thread.
	 */
	lock_task_sighand(task, &flags);

	if(task->tgroup_distributed == 0) {

		task->tgroup_distributed = 1;
		task->tgroup_home_id = task->tgid;
		task->tgroup_home_cpu = _cpu;

		entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
		if(!entry) {
			unlock_task_sighand(task, &flags);
			printk("Impossible to kmalloc in %s\n", __func__);
			return -1;
		}

		entry->mm = task->mm;
		atomic_inc(&entry->mm->mm_users);
		entry->tgroup_home_cpu = task->tgroup_home_cpu;
		entry->tgroup_home_id = task->tgroup_home_id;
		entry->next = NULL;
		entry->prev = NULL;
		entry->alive = 1;
		entry->main = NULL;
		atomic_set(&(entry->pending_migration), 0);
		atomic_set(&(entry->pending_back_migration), 0);

		init_vma_operation_fields_migration_memory_struct(entry);

		init_kernel_set_fields(entry);

		entry->setting_up = 0;

		init_vma_operation_fields_mm_struct(task->mm);

		add_migration_memory_entry(entry);

		first = 1;

		//let all memory operations ends
		down_write(&task->mm->mmap_sem);
		tgroup_iterator = task;
		while_each_thread(task, tgroup_iterator)
		{
			tgroup_iterator->tgroup_home_id = task->tgroup_home_id;
			tgroup_iterator->tgroup_home_cpu =
					task->tgroup_home_cpu;
			tgroup_iterator->tgroup_distributed = 1;
		};
		up_write(&task->mm->mmap_sem);

	}

	task->represents_remote = 1;

	unlock_task_sighand(task, &flags);

	request->tgroup_home_cpu = task->tgroup_home_cpu;
	request->tgroup_home_id = task->tgroup_home_id;

	tx_ret = pcn_kmsg_send_long(dst_cpu,
			(struct pcn_kmsg_long_message*)request,
			sizeof(*request) - sizeof(struct pcn_kmsg_hdr));

	if(first)
		kernel_thread(
				start_kernel_thread_for_distributed_process_from_user_one,
				entry,
				CLONE_THREAD | CLONE_SIGHAND | CLONE_VM
						| SIGCHLD);

	pcn_kmsg_free_msg(request);

	return tx_ret;
}

/* Migrate the specified task <task> to cpu <dst_cpu>
 * This function will put the specified task to
 * sleep, and push its info over to the remote cpu.  The
 * remote cpu will then select a new thread and import that
 * info into its context.
 *
 * It returns CLONE_FAIL in case of error,
 * CLONE_SUCCESS otherwise.
 */
int popcorn_do_migration(struct task_struct* task, int dst_cpu,
		struct pt_regs * regs)
{

	int ret = 0;

	if(task->prev_cpu == dst_cpu) {
		ret = do_back_migration(task, dst_cpu, regs);
	} else {
		ret = do_migration(task, dst_cpu, regs);
	}

	if(ret != -1) {

		__set_task_state(task, TASK_UNINTERRUPTIBLE);

		return CLONE_SUCCESS;

	} else

		return CLONE_FAIL;

}

void popcorn_synchronize_migrations(int tgroup_home_cpu, int tgroup_home_id)
{
	struct migration_memory* memory = NULL;

	memory = find_migration_memory_memory_entry(tgroup_home_cpu,
			tgroup_home_id);
	if(!memory) {
		printk("ERROR: %s no struct migration_memory found\n",
				__func__);
		return;
	}

	if(migrating_threads_barrier > 0) {

		while(atomic_read(&(memory->pending_back_migration))
				< migrating_threads_barrier) {
			msleep(1);
		}

		int app = atomic_add_return(-1,
				&(memory->pending_back_migration));
		if(app == migrating_threads_barrier)
			atomic_set(&(memory->pending_back_migration), 0);
	} else {
		atomic_dec(&(memory->pending_back_migration));
	}
}

int popcorn_dup_task(struct task_struct* orig, struct task_struct* task)
{
	unsigned long flags;

	task->executing_for_remote = 0;
	task->represents_remote = 0;
	task->distributed_exit = EXIT_ALIVE;
	task->tgroup_distributed = 0;
	task->prev_cpu = -1;
	task->next_cpu = -1;
	task->prev_pid = -1;
	task->next_pid = -1;
	task->tgroup_home_cpu = -1;
	task->tgroup_home_id = -1;
	task->main = 0;
	task->group_exit = -1;
	task->surrogate = -1;
	task->group_exit = -1;
	task->uaddr = 0;
	task->origin_pid = -1;
	task->remote_hb = NULL;

	// If the new task is not in the same thread group as the parent,
	// then we do not need to propagate the old thread info.
	if(orig->tgid != task->tgid) {
		return 1;
	}

	lock_task_sighand(orig, &flags);

	if(orig->tgroup_distributed == 1) {
		task->tgroup_home_cpu = orig->tgroup_home_cpu;
		task->tgroup_home_id = orig->tgroup_home_id;
		task->tgroup_distributed = 1;
	}

	unlock_task_sighand(orig, &flags);

	return 1;

}

static int read_proc_migrating_threads_barrier(char *page, char **start,
		off_t off, int count, int *eof, void *data)
{
	int len;
	len = sprintf(page, "%s: migrating_threads_barrier: %ld\n", __func__,
			migrating_threads_barrier);
	return len;

}
static int write_proc_migrating_threads_barrier(struct file *file,
		const char __user *buffer, unsigned long count, void *data)
{
	int ret = kstrtol_from_user(buffer, count, 0, &migrating_threads_barrier);
	printk("%s: migrating_threads_barrier %ld (%d)\n", __func__,
			migrating_threads_barrier, ret);
	return count;
}

static int __init popcorn_migration_init(void)
{

	printk("Per me si va ne la città dolente,\n"
			"per me si va ne l'etterno dolore,\n"
			"per me si va tra la perduta gente.\n"
			"Giustizia mosse il mio alto fattore;\n"
			"fecemi la divina podestate,\n"
			"la somma sapïenza e 'l primo amore.\n"
			"Dinanzi a me non fuor cose create\n"
			"se non etterne, e io etterno duro.\n"
			"Lasciate ogne speranza, voi ch'intrate\n\n");

	uint16_t copy_cpu;
	if(scif_get_nodeIDs(NULL, 0, &copy_cpu) == -1){
		printk("ERROR %s cannot initialize _cpu\n", __func__);
	}
	else {
		_cpu = copy_cpu;
	}

	migration_wq = create_workqueue("migration_wq");
	exit_wq = create_workqueue("exit_wq");
	exit_group_wq = create_workqueue("exit_group_wq");
	new_kernel_wq = create_workqueue("new_kernel_wq");

	struct proc_dir_entry *res;
	res = create_proc_entry("migrating_threads_barrier", S_IRUGO, NULL);
	if(!res) {
		printk("%s: create_proc_entry failed (%p)\n", __func__, res);
		return -ENOMEM;
	}
	res->read_proc = read_proc_migrating_threads_barrier;
	res->write_proc = write_proc_migrating_threads_barrier;

	/*
	 * Register to receive relevant incoming messages.
	 */
	pcn_kmsg_register_callback(
			PCN_KMSG_TYPE_CREATE_THREAD_PAIRING,
			handle_thread_pairing_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_BACK_MIG_REQUEST,
			handle_back_migration_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_MIG_REQUEST,
			handle_migration_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_EXIT_MIG_THREAD,
			handle_exit_mig_thread);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_THREAD_COUNT_REQUEST,
			handle_remote_thread_count_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_THREAD_COUNT_RESPONSE,
			handle_remote_thread_count_response);
	pcn_kmsg_register_callback(
			PCN_KMSG_TYPE_PROCESS_EXIT_NOTIFICATION,
			handle_process_exit_notification);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_NEW_KERNEL,
			handle_new_kernel);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_NEW_KERNEL_ANSWER,
			handle_new_kernel_answer);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_CREATE_THREAD_POOL,
			handle_thread_pool_creation);

	return 0;
}

late_initcall_popcorn(popcorn_migration_init);
