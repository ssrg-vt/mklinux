/**
 * Migration service + replication of virtual address space
 *
 * Antonio Barbalace 2016
 * Vincent Legout, Antonio Barbalace, Sharat Kumar Bath, Ajithchandra Saya 2014-2015
 * Marina Sadini 2013
 */

//#include <linux/mcomm.h> // IPC
#include <linux/kthread.h>
#include <linux/export.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/sched.h>
#include <linux/threads.h> // NR_CPUS
#include <linux/kmod.h>
#include <linux/path.h>
#include <linux/mount.h>
#include <linux/fs.h>
#include <linux/slab.h>

#include <linux/mm.h>
#include <linux/io.h> // ioremap
#include <linux/mman.h> // MAP_ANONYMOUS

#include <linux/highmem.h> //Replication
#include <linux/memcontrol.h>
#include <linux/pagemap.h>
#include <linux/mmu_notifier.h>
#include <linux/swap.h>

#include <asm/traps.h>			/* dotraplinkage, ...		*/
#include <asm/pgalloc.h>		/* pgd_*(), ...			*/
#include <asm/fixmap.h>			/* VSYSCALL_START		*/
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h> // USER_DS
#include <asm/page.h>//Replication
#include <asm/mmu_context.h>

#include <linux/rmap.h>
#include <linux/memcontrol.h>
#include <asm/atomic.h>
#include <linux/syscalls.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <net/checksum.h>
#include <linux/fsnotify.h>
#include <linux/unistd.h>
#include <linux/tsacct_kern.h>
#include <linux/proc_fs.h>
/*akshay*/
#include <linux/futex.h>
#define  NSIG 32

#include<linux/signal.h>
#include <linux/fcntl.h>
#include <linux/ktime.h>

#include <linux/cpu_namespace.h>
#include <linux/popcorn_cpuinfo.h>
#include <linux/process_server.h>
#include <process_server_arch.h>
#include <popcorn/remote_file.h>

#include <linux/elf.h>
#include <linux/binfmts.h>
#include <asm/elf.h>

#include "../WKdm.h"
#include "../futex_remote.h"
/*akshay*/
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

/**
 * Module variables
 */

static int _cpu = -1;
int _file_cpu = 1;

///////////////////////////////////////////////////////////////////////////////
// Vincent's scheduling infrasrtucture based on Antonio's power/pmu readings
///////////////////////////////////////////////////////////////////////////////
#define POPCORN_POWER_N_VALUES 10
int *popcorn_power_x86_1;
int *popcorn_power_x86_2;
int *popcorn_power_arm_1;
int *popcorn_power_arm_2;
int *popcorn_power_arm_3;
EXPORT_SYMBOL_GPL(popcorn_power_x86_1);
EXPORT_SYMBOL_GPL(popcorn_power_x86_2);
EXPORT_SYMBOL_GPL(popcorn_power_arm_1);
EXPORT_SYMBOL_GPL(popcorn_power_arm_2);
EXPORT_SYMBOL_GPL(popcorn_power_arm_3);

struct popcorn_sched {
        int **power_arm;
        int **power_x86;

        struct task_struct **tasks;
};

struct popcorn_sched pop_sched;
EXPORT_SYMBOL_GPL(pop_sched);

///////////////////////////////////////////////////////////////////////////////
// Marina's data stores (linked lists)
///////////////////////////////////////////////////////////////////////////////
data_header_t* _data_head = NULL;
DEFINE_RAW_SPINLOCK(_data_head_lock);

fetching_t* _fetching_head = NULL;
DEFINE_RAW_SPINLOCK(_fetching_head_lock);

mapping_answers_for_2_kernels_t* _mapping_head = NULL;
DEFINE_RAW_SPINLOCK(_mapping_head_lock);

ack_answers_for_2_kernels_t* _ack_head = NULL;
DEFINE_RAW_SPINLOCK(_ack_head_lock);

memory_t* _memory_head = NULL;
DEFINE_RAW_SPINLOCK(_memory_head_lock);

count_answers_t* _count_head = NULL;
DEFINE_RAW_SPINLOCK(_count_head_lock);

vma_op_answers_t* _vma_ack_head = NULL;
DEFINE_RAW_SPINLOCK(_vma_ack_head_lock);

thread_pull_t* thread_pull_head = NULL;
DEFINE_RAW_SPINLOCK(thread_pull_head_lock);

#include "internal.h"

///////////////////////////////////////////////////////////////////////////////
// Working queues (servers)
///////////////////////////////////////////////////////////////////////////////
static struct workqueue_struct *clone_wq;
static struct workqueue_struct *exit_wq;
static struct workqueue_struct *exit_group_wq;
static struct workqueue_struct *message_request_wq;
static struct workqueue_struct *invalid_message_wq;
static struct workqueue_struct *vma_op_wq;
static struct workqueue_struct *vma_lock_wq;
static struct workqueue_struct *new_kernel_wq;

//wait lists
DECLARE_WAIT_QUEUE_HEAD( read_write_wait);
DECLARE_WAIT_QUEUE_HEAD( request_distributed_vma_op);


/* External functions */
extern unsigned long do_brk(unsigned long addr, unsigned long len);
extern unsigned long mremap_to(unsigned long addr, unsigned long old_len,
			       unsigned long new_addr, unsigned long new_len, bool *locked);

/* ajith - for file offset fetch */
#if ELF_EXEC_PAGESIZE > PAGE_SIZE
#define ELF_MIN_ALIGN   ELF_EXEC_PAGESIZE
#else
#define ELF_MIN_ALIGN   PAGE_SIZE
#endif
static unsigned long get_file_offset(struct file* file, int start_addr);

#if MIGRATION_PROFILE
ktime_t migration_start, migration_end;
#endif

asmlinkage long sys_take_time(int start)
{
	if (start == 1)
		trace_printk("s\n");
	else
		trace_printk("e\n");
	return 0;
}

#if STATISTICS
unsigned long long perf_aa, perf_bb, perf_cc, perf_dd, perf_ee;
static int page_fault_mio,fetch,local_fetch,write,concurrent_write,most_long_write;
static int most_written_page,read,most_long_read,invalid,ack,answer_request,answer_request_void;
static int request_data,pages_allocated,compressed_page_sent,not_compressed_page,not_compressed_diff_page;
#endif
extern unsigned long read_old_rsp(void);
extern int exec_mmap(struct mm_struct *mm);
extern struct task_struct* do_fork_for_main_kernel_thread(unsigned long clone_flags,
							  unsigned long stack_start,
							  struct pt_regs *regs,
							  unsigned long stack_size,
							  int __user *parent_tidptr,
							  int __user *child_tidptr);

static int count_remote_thread_members(int tgroup_home_cpu, int tgroup_home_id,memory_t* mm_data)
{
	count_answers_t* data;
	remote_thread_count_request_t* request;
	int i, s;
	int ret = -1;
	unsigned long flags;

	data = (count_answers_t*) kmalloc(sizeof(count_answers_t), GFP_ATOMIC);
	if (!data)
		return -1;

	data->responses = 0;
	data->tgroup_home_cpu = tgroup_home_cpu;
	data->tgroup_home_id = tgroup_home_id;
	data->count = 0;
	data->waiting = current;
	raw_spin_lock_init(&(data->lock));

	add_count_entry(data);

	request= (remote_thread_count_request_t*) kmalloc(sizeof(remote_thread_count_request_t),GFP_ATOMIC);
	if(request==NULL)
		return -1;

	request->header.type = PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
	request->tgroup_home_cpu = tgroup_home_cpu;
	request->tgroup_home_id = tgroup_home_id;

	data->expected_responses = 0;

	down_read(&mm_data->kernel_set_sem);

	//printk("%s before sending data->expected_responses %d data->responses %d\n",__func__,data->expected_responses,data->responses);
	// the list does not include the current processor group descirptor (TODO)
	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;

	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;

		if(mm_data->kernel_set[i]==1){
			// Send the request to this cpu.
			//s = pcn_kmsg_send(i, (struct pcn_kmsg_message*) (&request));
			s = pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (request),sizeof(remote_thread_count_request_t)- sizeof(struct pcn_kmsg_hdr));
			if (s!=-1) {
				// A successful send operation, increase the number
				// of expected responses.
				data->expected_responses++;
			}
		}
	}

	up_read(&mm_data->kernel_set_sem);
	//printk("%s going to sleep data->expected_responses %d data->responses %d\n",__func__,data->expected_responses,data->responses);
	while (data->expected_responses != data->responses) {
		set_task_state(current, TASK_UNINTERRUPTIBLE);
		if (data->expected_responses != data->responses)
			schedule();

		set_task_state(current, TASK_RUNNING);
	}

	//printk("%s waked up data->expected_responses%d data->responses%d\n",__func__,data->expected_responses,data->responses);
	raw_spin_lock_irqsave(&(data->lock), flags);
	raw_spin_unlock_irqrestore(&(data->lock), flags);
	// OK, all responses are in, we can proceed.
	//printk("%s data->count is %d",__func__,data->count);
	ret = data->count;
	remove_count_entry(data);
	kfree(data);
	kfree(request);
	return ret;
}

void process_vma_op(struct work_struct* work);

static void process_new_kernel_answer(struct work_struct* work)
{
	new_kernel_work_answer_t* my_work= (new_kernel_work_answer_t*)work;
	new_kernel_answer_t* answer= my_work->answer;
	memory_t* memory= my_work->memory;

	if(answer->header.from_cpu==answer->tgroup_home_cpu){
		down_write(&memory->mm->mmap_sem);
		//	printk("%s answer->vma_operation_index %d NR_CPU %d\n",__func__,answer->vma_operation_index,MAX_KERNEL_IDS);
		memory->mm->vma_operation_index= answer->vma_operation_index;
		up_write(&memory->mm->mmap_sem);
	}

	down_write(&memory->kernel_set_sem);
	int i;
	for(i=0;i<MAX_KERNEL_IDS;i++){
		if(answer->my_set[i]==1)
			memory->kernel_set[i]= 1;
	}
	memory->answers++;

	if (memory->answers >= memory->exp_answ) {
		printk("%s: wake up %d (%s)\n", __func__, memory->main->pid, memory->main->comm);
		wake_up_process(memory->main);
	}

	if (memory->answers >= memory->exp_answ) {
		printk("%s: wake up %d (%s)\n", __func__, memory->main->pid, memory->main->comm);
		wake_up_process(memory->main);
	}

	up_write(&memory->kernel_set_sem);

	pcn_kmsg_free_msg(answer);
	kfree(work);
}

///////////////////////////////////////////////////////////////////////////////
// handling a new incoming migration request?
///////////////////////////////////////////////////////////////////////////////
static int handle_new_kernel_answer(struct pcn_kmsg_message* inc_msg)
{
	new_kernel_answer_t* answer= (new_kernel_answer_t*)inc_msg;
	memory_t* memory = find_memory_entry(answer->tgroup_home_cpu,
					    answer->tgroup_home_id);

	PSNEWTHREADPRINTK("received new kernel answer\n");
	//printk("%s: %d\n",__func__,answer->vma_operation_index);
	if (memory!=NULL) {
		new_kernel_work_answer_t* work= (new_kernel_work_answer_t*)kmalloc(sizeof(new_kernel_work_answer_t), GFP_ATOMIC);
		if(work!=NULL){
			work->answer = answer;
			work->memory= memory;
			INIT_WORK( (struct work_struct*)work, process_new_kernel_answer);
			queue_work(new_kernel_wq, (struct work_struct*) work);
		}
		else
			pcn_kmsg_free_msg(inc_msg);
	}
	else {
		printk("%s: ERROR: received an answer new kernel but memory_t not present cpu %d id %d\n",
				__func__, answer->tgroup_home_cpu, answer->tgroup_home_id);
		pcn_kmsg_free_msg(inc_msg);
	}
	
	return 1;
}

void process_new_kernel(struct work_struct* work)
{
	new_kernel_work_t* new_kernel_work= (new_kernel_work_t*) work;
	memory_t* memory;

	printk("received new kernel request cpu %d id %d\n", new_kernel_work->request->tgroup_home_cpu, new_kernel_work->request->tgroup_home_id);

	new_kernel_answer_t* answer= (new_kernel_answer_t*) kmalloc(sizeof(new_kernel_answer_t), GFP_ATOMIC);

	if (answer!=NULL) {
		memory = find_memory_entry(new_kernel_work->request->tgroup_home_cpu,
					   new_kernel_work->request->tgroup_home_id);
		if (memory != NULL) {
			printk("memory present cpu %d id %d\n", new_kernel_work->request->tgroup_home_cpu,new_kernel_work->request->tgroup_home_id);	
			down_write(&memory->kernel_set_sem);
			memory->kernel_set[new_kernel_work->request->header.from_cpu]= 1;
			memcpy(answer->my_set,memory->kernel_set,MAX_KERNEL_IDS*sizeof(int));
			up_write(&memory->kernel_set_sem);

			if(_cpu==new_kernel_work->request->tgroup_home_cpu){
				down_read(&memory->mm->mmap_sem);
				answer->vma_operation_index= memory->mm->vma_operation_index;
				//printk("%s answer->vma_operation_index %d\n",__func__,answer->vma_operation_index);
				up_read(&memory->mm->mmap_sem);
			}

		}
		else {
			PSPRINTK("memory not present\n");
			memset(answer->my_set,0,MAX_KERNEL_IDS*sizeof(int));
		}

		answer->tgroup_home_cpu= new_kernel_work->request->tgroup_home_cpu;
		answer->tgroup_home_id= new_kernel_work->request->tgroup_home_id;
		answer->header.type= PCN_KMSG_TYPE_PROC_SRV_NEW_KERNEL_ANSWER;
		answer->header.prio= PCN_KMSG_PRIO_NORMAL;
		//printk("just before send %s answer->vma_operation_index %d NR_CPU %d\n",__func__,answer->vma_operation_index,MAX_KERNEL_IDS);
		pcn_kmsg_send_long(new_kernel_work->request->header.from_cpu,
				   (struct pcn_kmsg_long_message*) answer,
				   sizeof(new_kernel_answer_t) - sizeof(struct pcn_kmsg_hdr));
		//int ret=pcn_kmsg_send(new_kernel_work->request->header.from_cpu, (struct pcn_kmsg_long_message*) answer);
		//printk("%s send long ret is %d sizeof new_kernel_answer_t is %d size of header is %d\n",__func__,ret,sizeof(new_kernel_answer_t),sizeof(struct pcn_kmsg_hdr));
		kfree(answer);
	}

	pcn_kmsg_free_msg(new_kernel_work->request);
	kfree(work);
}

static int handle_new_kernel(struct pcn_kmsg_message* inc_msg)
{
	new_kernel_t* new_kernel= (new_kernel_t*)inc_msg;
	new_kernel_work_t* request_work;

	request_work = (new_kernel_work_t*) kmalloc(sizeof(new_kernel_work_t), GFP_ATOMIC);

	if (request_work) {
		request_work->request = new_kernel;
		INIT_WORK( (struct work_struct*)request_work, process_new_kernel);
		queue_work(new_kernel_wq, (struct work_struct*) request_work);
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// thread pool functionalities
///////////////////////////////////////////////////////////////////////////////
static int create_kernel_thread_for_distributed_process(void *data);

static void update_thread_pull(struct work_struct* work) {

	int i, count;
    printk("%s\n", __func__);

	count = count_data((data_header_t**)&thread_pull_head, &thread_pull_head_lock);

	for (i = 0; i < NR_THREAD_PULL - count; i++) {
		//	printk("%s creating thread pull %d\n", __func__, i);
		kernel_thread(create_kernel_thread_for_distributed_process, NULL, SIGCHLD);
	}

	kfree(work);
}

static void _create_thread_pull(struct work_struct* work)
{
	int i;

	for(i=0;i<NR_THREAD_PULL;i++){
		printk("%s creating thread pull %d\n", __func__, i);
		kernel_thread(create_kernel_thread_for_distributed_process, NULL, SIGCHLD);
	}

	create_thread_pull_t* msg= (create_thread_pull_t*) kmalloc(sizeof(create_thread_pull_t),GFP_ATOMIC);
	if(!msg){
		printk("%s Impossible to kmalloc",__func__);
		return;
	}

	msg->header.type= PCN_KMSG_TYPE_PROC_SRV_CREATE_THREAD_PULL;
	msg->header.prio= PCN_KMSG_PRIO_NORMAL;

	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;
		pcn_kmsg_send_long(i,(struct pcn_kmsg_long_message*) (msg),
				   sizeof(create_thread_pull_t)-sizeof(struct pcn_kmsg_hdr));

	}

	kfree(msg);
	kfree(work);
}

void create_thread_pull(void)
{
	static int only_one=0;

	if(only_one==0){
		struct work_struct* work = kmalloc(sizeof(struct work_struct),GFP_ATOMIC);

		if (work) {
			INIT_WORK( work, _create_thread_pull);
			queue_work(clone_wq, work);
		}
		only_one++;
	}
}

static int handle_thread_pull_creation(struct pcn_kmsg_message* inc_msg)
{
	printk("IN %s:%d\n", __func__, __LINE__);
	create_thread_pull();
	pcn_kmsg_free_msg(inc_msg);
	return 0;
}

/* return type:
 * 0 normal;
 * 1 flush pending operation
 * */
static int exit_distributed_process(memory_t* mm_data, int flush,thread_pull_t * my_thread_pull) {
	struct task_struct *g;
	unsigned long flags;
	int is_last_thread_in_local_group = 1;
	int count = 0, i, status;
	thread_group_exited_notification_t* exit_notification;

	lock_task_sighand(current, &flags);
	g = current;
	while_each_thread(current, g)
	{
		if (g->main == 0 && g->distributed_exit == EXIT_ALIVE) {
			is_last_thread_in_local_group = 0;
			goto find;
		}
	};

find: 
	status = current->distributed_exit;
	current->distributed_exit = EXIT_ALIVE;
	unlock_task_sighand(current, &flags);

	if (mm_data->alive == 0 && !is_last_thread_in_local_group && atomic_read(&(mm_data->pending_migration))==0) {
		printk("%s: ERROR: mm_data->alive is 0 but there are alive threads (id %d, cpu %d)\n", __func__, current->tgroup_home_id, current->tgroup_home_cpu);
		return 0;
	}

	if (mm_data->alive == 0  && atomic_read(&(mm_data->pending_migration))==0) {
		if (status == EXIT_THREAD) {
			printk("%s: ERROR: alive is 0 but status is exit thread (id %d, cpu %d)\n", __func__, current->tgroup_home_id, current->tgroup_home_cpu);
			return flush;
		}

		if (status == EXIT_PROCESS) {
			if (flush == 0) {
				//this is needed to flush the list of pending operation before die
				vma_op_work_t* work = kmalloc(sizeof(vma_op_work_t),
							      GFP_ATOMIC);
				if (work) {
					work->fake = 1;
					work->memory = mm_data;
					mm_data->arrived_op = 0;
					INIT_WORK( (struct work_struct*)work, process_vma_op);
					queue_work(vma_op_wq, (struct work_struct*) work);
				}
				return 1;
			}
		}

		if (flush == 1 && mm_data->arrived_op == 0) {
			if (status == EXIT_FLUSHING)
				printk("%s: ERROR: status exit flush but arrived op is 0 (id %d, cpu %d)\n", __func__, current->tgroup_home_id, current->tgroup_home_cpu);
			return 1;
		} else {			
			if(atomic_read(&(mm_data->pending_migration))!=0)
				printk(KERN_ALERT"%s: ERROR pending migration when cleaning memory (id %d, cpu %d)\n", __func__, current->tgroup_home_id, current->tgroup_home_cpu);
			shadow_thread_t* my_shadow= NULL;

			my_shadow = (shadow_thread_t*) pop_data( (data_header_t **)&(my_thread_pull->threads),
								 &(my_thread_pull->spinlock));

			while(my_shadow){
				my_shadow->thread->distributed_exit= EXIT_THREAD;
				wake_up_process(my_shadow->thread);
				kfree(my_shadow);
				my_shadow = (shadow_thread_t*) pop_data( (data_header_t **)&(my_thread_pull->threads),
									 &(my_thread_pull->spinlock));
			}
			remove_memory_entry(mm_data);
			mmput(mm_data->mm);
			kfree(mm_data);
#if STATISTICS
			PSPRINTK("%s: page_fault %i fetch %i local_fetch %i write %i read %i most_long_read %i invalid %i ack %i answer_request %i answer_request_void %i request_data %i most_written_page %i concurrent_writes %i most long write %i pages_allocated %i compressed_page_sent %i not_compressed_page %i not_compressed_diff_page %i  (id %d, cpu %d)\n", __func__ , 
				 page_fault_mio,fetch,local_fetch,write,read,most_long_read,invalid,ack,answer_request,answer_request_void, request_data,most_written_page, concurrent_write,most_long_write, pages_allocated,compressed_page_sent,not_compressed_page,not_compressed_diff_page, current->tgroup_home_id, current->tgroup_home_cpu);
#endif

			struct work_struct* work = kmalloc(sizeof(struct work_struct),
							   GFP_ATOMIC);
			if (work) {
				INIT_WORK( work, update_thread_pull);
				queue_work(clone_wq, work);
			}
			do_exit(0);
			return 0;
		}
	} else {
		/* If I am the last thread of my process in this kernel:
		 * - or I am the last thread of the process on all the system => send a group exit to all kernels and erase the mapping saved
		 * - or there are other alive threads in the system => do not erase the saved mapping
		 */
		if (is_last_thread_in_local_group) {
			PSPRINTK("%s: This is the last thread of process (id %d, cpu %d) in the kernel!\n", __func__, current->tgroup_home_id, current->tgroup_home_cpu);
			//mm_data->alive = 0;
			count = count_remote_thread_members(current->tgroup_home_cpu,
							    current->tgroup_home_id,mm_data);
			/* Ok this is complicated.
			 * If count is zero=> all the threads of my process went through this exit function (all task->distributed_exit==1 or
			 * there are no more tasks of this process around).
			 * Dying tasks that did not see count==0 saved a copy of the mapping. Someone should notice their kernels that now they can erase it.
			 * I can be the one, however more threads can be concurrently in this exit function on different kernels =>
			 * each one of them can see the count==0 => more than one "erase mapping message" can be sent.
			 * If count==0 I check if I already receive a "erase mapping message" and avoid to send another one.
			 * This check does not guarantee that more than one "erase mapping message" cannot be sent (in some executions it is inevitable) =>
			 * just be sure to not call more than one mmput one the same mapping!!!
			 */
			if (count == 0) {
				mm_data->alive = 0;
				if (status != EXIT_PROCESS) {
					PSPRINTK("%s: This is the last thread of process (id %d, cpu %d) in the system, "
						 "sending an erase mapping message!\n", __func__, current->tgroup_home_id, current->tgroup_home_cpu);
					exit_notification= (thread_group_exited_notification_t*) kmalloc(sizeof(thread_group_exited_notification_t),GFP_ATOMIC);
					exit_notification->header.type = PCN_KMSG_TYPE_PROC_SRV_THREAD_GROUP_EXITED_NOTIFICATION;
					exit_notification->header.prio = PCN_KMSG_PRIO_NORMAL;
					exit_notification->tgroup_home_cpu = current->tgroup_home_cpu;
					exit_notification->tgroup_home_id = current->tgroup_home_id;
					// the list does not include the current processor group descirptor (TODO)
					struct list_head *iter;
					_remote_cpu_info_list_t *objPtr;
					list_for_each(iter, &rlist_head) {
						objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
						i = objPtr->_data._processor;
						pcn_kmsg_send_long(i,(struct pcn_kmsg_long_message*)(exit_notification),sizeof(thread_group_exited_notification_t)- sizeof(struct pcn_kmsg_hdr));

					}
					kfree(exit_notification);
				}

				if (flush == 0) {
					//this is needed to flush the list of pending operation before die

					vma_op_work_t* work = kmalloc(sizeof(vma_op_work_t),
								      GFP_ATOMIC);
					if (work) {
						work->fake = 1;
						work->memory = mm_data;
						mm_data->arrived_op = 0;
						INIT_WORK( (struct work_struct*)work, process_vma_op);
						queue_work(vma_op_wq, (struct work_struct*) work);
					}
					return 1;
				} else {
					printk("%s: ERROR: flush is 1 during first exit (alive set to 0 now) (id %d, cpu %d)\n", __func__, current->tgroup_home_id, current->tgroup_home_cpu);
					return 1;
				}
			}else {
				/*
				 * case i am the last thread but count is not zero
				 * check if there are concurrent migration to be sure if I can put mm_data->alive = 0;
				 */				
				if(atomic_read(&(mm_data->pending_migration))==0)
					mm_data->alive = 0;
			}
		}

		if ((!is_last_thread_in_local_group || count != 0) && status == EXIT_PROCESS) {
			printk("%s: ERROR: received an exit process but is_last_thread_in_local_group id %d and count is %d\n ",
			       __func__, is_last_thread_in_local_group, count);
		}
		return 0;
	}
}

static void create_new_threads(thread_pull_t * my_thread_pull, int *spare_threads) {
	int count;
    //printk("%s\n", __func__);

	count = count_data((data_header_t**) &(my_thread_pull->threads), &(my_thread_pull->spinlock));

	if (count == 0) {
		while (count < *spare_threads) {
			count++;
			shadow_thread_t* shadow = (shadow_thread_t*) kmalloc(sizeof(shadow_thread_t), GFP_ATOMIC);
			if (shadow) {
				/* Ok, create the new process.. */
				shadow->thread = create_thread(CLONE_THREAD |
							       CLONE_SIGHAND	|
							       CLONE_VM	|
							       CLONE_UNTRACED);
				if (!IS_ERR(shadow->thread)) {
					printk("%s: WARN: push thread %d\n",
							__func__, shadow->thread->pid);
					push_data((data_header_t**)&(my_thread_pull->threads),
						  &(my_thread_pull->spinlock), (data_header_t*) shadow);
				} else {
					printk("%s: ERROR: not able to create shadow\n",
							__func__);
					kfree(shadow);
				}
			} else
				printk("%s: ERROR: impossible to kmalloc\n", __func__);
		}
		*spare_threads = *spare_threads * 2;
	}
}

///////////////////////////////////////////////////////////////////////////////
// main for distributed kernel thread (?)
///////////////////////////////////////////////////////////////////////////////
static void main_for_distributed_kernel_thread(memory_t* mm_data, thread_pull_t * my_thread_pull)
{
	struct file* f;
	unsigned long ret = 0;
	int flush = 0;
	int count;
	int spare_threads = 2;
	// TODO: Need to explore how this has to be used
	//       added to port to Linux 3.12 API's
	//bool vma_locked = false;
	unsigned long populate = 0;
	
	while (1) {
again:
		create_new_threads(my_thread_pull, &spare_threads);

		while (current->distributed_exit != EXIT_ALIVE) {
			flush = exit_distributed_process(mm_data, flush, my_thread_pull);
			msleep(1000);
		}
		while (mm_data->operation != VMA_OP_NOP && 
		       mm_data->mm->thread_op == current) {
			switch (mm_data->operation) {
			case VMA_OP_UNMAP:
				down_write(&mm_data->mm->mmap_sem);
				if (current->tgroup_home_cpu != _cpu)
					mm_data->mm->distribute_unmap = 0;
				ret = do_munmap(mm_data->mm, mm_data->addr, mm_data->len);
				if (current->tgroup_home_cpu != _cpu)
					mm_data->mm->distribute_unmap = 1;
				up_write(&mm_data->mm->mmap_sem);
				break;

			case VMA_OP_PROTECT:
				if (current->tgroup_home_cpu != _cpu)
					mm_data->mm->distribute_unmap = 0;
				ret = kernel_mprotect(mm_data->addr, mm_data->len,
						      mm_data->prot);
				if (current->tgroup_home_cpu != _cpu)
					mm_data->mm->distribute_unmap = 1;
				break;

			case VMA_OP_REMAP:
				down_write(&mm_data->mm->mmap_sem);
				if (current->tgroup_home_cpu != _cpu)
					mm_data->mm->distribute_unmap = 0;
				ret = kernel_mremap(mm_data->addr, mm_data->len, mm_data->new_len, 0, mm_data->new_addr);
				if (current->tgroup_home_cpu != _cpu)
					mm_data->mm->distribute_unmap = 1;
				up_write(&mm_data->mm->mmap_sem);
				break;

			case VMA_OP_BRK:
				ret = -1;
				down_write(&mm_data->mm->mmap_sem);
				if (current->tgroup_home_cpu != _cpu)
					mm_data->mm->distribute_unmap = 0;
				ret = do_brk(mm_data->addr, mm_data->len);
				if (current->tgroup_home_cpu != _cpu)
					mm_data->mm->distribute_unmap = 1;
				up_write(&mm_data->mm->mmap_sem);
				break;

			case VMA_OP_MAP:
				ret = -1;
				f = NULL;
				if (mm_data->path[0] != '\0') {
					f = filp_open(mm_data->path, O_RDONLY | O_LARGEFILE, 0);
					if (IS_ERR(f)) {
						printk("ERROR: cannot open file to map\n");
						break;
					}
				}
				down_write(&mm_data->mm->mmap_sem);
				if (current->tgroup_home_cpu != _cpu)
					mm_data->mm->distribute_unmap = 0;
				ret = do_mmap_pgoff(f, mm_data->addr, mm_data->len, mm_data->prot, 
						    mm_data->flags, mm_data->pgoff, &populate);
				if (current->tgroup_home_cpu != _cpu)
					mm_data->mm->distribute_unmap = 1;
				up_write(&mm_data->mm->mmap_sem);
	
				if (mm_data->path[0] != '\0') {
					filp_close(f, NULL);
				}
				break;

			default:
				break;
			}
			mm_data->addr = ret;
			mm_data->operation = VMA_OP_NOP;

            PSPRINTK("%s: wake up %d (cpu %d id %d)\n",
            		__func__, mm_data->waiting_for_main->pid,
					current->tgroup_home_cpu, current->tgroup_home_id);
			wake_up_process(mm_data->waiting_for_main);
		}
		__set_task_state(current, TASK_UNINTERRUPTIBLE);

		count = count_data((data_header_t**)&(my_thread_pull->threads), &my_thread_pull->spinlock);
		if (count == 0 || current->distributed_exit != EXIT_ALIVE ||
		    (mm_data->operation != VMA_OP_NOP && mm_data->mm->thread_op == current)) {
			__set_task_state(current, TASK_RUNNING);
			goto again;
		}
		schedule();
	}
}

static int create_kernel_thread_for_distributed_process_from_user_one(void *data)
{
	memory_t* entry = (memory_t*) data;
	thread_pull_t* my_thread_pull;
	int i, ret;

	printk("%s\n", __func__);
	current->main = 1;
	entry->main = current;

	if (!popcorn_ns) {
		if ((ret = build_popcorn_ns(0))) {
			printk(KERN_ERR"%s: WARN: build_popcorn returned error (%d)\n",
					__func__, ret);
		}
	}

	my_thread_pull = (thread_pull_t*) kmalloc(sizeof(thread_pull_t),
						  GFP_ATOMIC);
	if (!my_thread_pull) {
		printk(KERN_ERR"%s: ERROR: kmalloc thread pull\n", __func__);
		return -1;
	}

	raw_spin_lock_init(&(my_thread_pull->spinlock));
	my_thread_pull->main = current;
	my_thread_pull->memory = entry;
	my_thread_pull->threads= NULL;

	entry->thread_pull= my_thread_pull;

	//int count= count_data((data_header_t**)&(thread_pull_head), &thread_pull_head_lock);
	//printk("WARNING count is %d in %s\n",count,__func__);
        
	// Sharath: Increased the thread pool size
	for (i = 0; i < THREAD_POOL_SIZE; i++) {
		shadow_thread_t* shadow = (shadow_thread_t*) kmalloc(
			sizeof(shadow_thread_t), GFP_ATOMIC);
		if (shadow) {
			/* Ok, create the new process.. */
			shadow->thread = create_thread(CLONE_THREAD | CLONE_SIGHAND | CLONE_VM | CLONE_UNTRACED);
			if (!IS_ERR(shadow->thread)) {
				// printk("%s new shadow created\n",__func__);
				PSPRINTK("%s: push thread %d (%d)\n", __func__, shadow->thread->pid, i);
				push_data((data_header_t**)&(my_thread_pull->threads), &(my_thread_pull->spinlock),
					  (data_header_t*)shadow);
			} else {
				printk(KERN_ERR"%s: ERROR: not able to create shadow (%d)\n", __func__, i);
				kfree(shadow);
			}
		}
	}

	main_for_distributed_kernel_thread(entry,my_thread_pull);

	/* if here something went wrong...
	 */
	printk(KERN_ALERT"%s: ERROR: exited from main_for_distributed_kernel_thread\n", __func__);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Mappings handling
///////////////////////////////////////////////////////////////////////////////
/* TODO the following two functions can be refactored into one */
static int handle_mapping_response_void(struct pcn_kmsg_message* inc_msg)
{
	data_void_response_for_2_kernels_t* response;
	mapping_answers_for_2_kernels_t* fetched_data;

	response = (data_void_response_for_2_kernels_t*) inc_msg;
	fetched_data = find_mapping_entry(response->tgroup_home_cpu,
					  response->tgroup_home_id, response->address);

#if STATISTICS
	answer_request_void++;
#endif

	PSPRINTK("%s: answer_request_void %i address %lx from cpu %i. This is a void response.\n", __func__, 0, response->address, inc_msg->hdr.from_cpu);
	PSMINPRINTK("answer_request_void address %lx from cpu %i. This is a void response.\n", response->address, inc_msg->hdr.from_cpu);

	if (fetched_data == NULL) {
		printk(KERN_ERR"%s: WARN: data not found in local list (cpu %d id %d address 0x%lx) 0x%lx\n",
				__func__, response->tgroup_home_cpu, response->tgroup_home_id, response->address, (unsigned long)response);
		pcn_kmsg_free_msg(inc_msg);
		return -1;
	}

	if (response->owner == 1) {
		PSPRINTK("Response with ownership\n");
		fetched_data->owner = 1;
	}

	if (response->vma_present == 1) {
		if (response->header.from_cpu != response->tgroup_home_cpu)
			printk("%s: WARN: a kernel that is not the server is sending the mapping (cpu %d id %d address 0x%lx)\n",
					__func__, response->header.from_cpu, response->tgroup_home_cpu, response->address);

		PSPRINTK("%s: response->vma_pesent %d response->vaddr_start %lx response->vaddr_size %lx response->prot %lx response->vm_flags %lx response->pgoff %lx response->path %s response->fowner %d\n", __func__,
					 response->vma_present, response->vaddr_start , response->vaddr_size,response->prot, response->vm_flags , response->pgoff, response->path,response->futex_owner);

		if (fetched_data->vma_present == 0) {
			PSPRINTK("Set vma\n");
			fetched_data->vma_present = 1;
			fetched_data->vaddr_start = response->vaddr_start;
			fetched_data->vaddr_size = response->vaddr_size;
			fetched_data->prot = response->prot;
			fetched_data->pgoff = response->pgoff;
			fetched_data->vm_flags = response->vm_flags;
			strcpy(fetched_data->path, response->path);
		} else {
			printk("%s: WARN: received more than one mapping %d (cpu %d id %d address 0x%lx) 0x%lx\n",
					__func__, fetched_data->vma_present,
					response->tgroup_home_cpu, response->tgroup_home_id, response->address, response);
		}
	}

	if (fetched_data->arrived_response!=0)
		printk("%s: WARN: received more than one answer, arrived_response is %d (cpu %d id %d address 0x%lx) 0x%lx\n",
				__func__, fetched_data->arrived_response,
				response->tgroup_home_cpu, response->tgroup_home_id, response->address, response);

	fetched_data->arrived_response++;
	fetched_data->futex_owner = response->futex_owner;

	PSPRINTK("%s: wake up %d\n", __func__, fetched_data->waiting->pid);
	wake_up_process(fetched_data->waiting);

	pcn_kmsg_free_msg(inc_msg);
	return 1;
}

static int handle_mapping_response(struct pcn_kmsg_message* inc_msg)
{
	data_response_for_2_kernels_t* response;
	mapping_answers_for_2_kernels_t* fetched_data;
	int set = 0;

	response = (data_response_for_2_kernels_t*) inc_msg;
	fetched_data = find_mapping_entry(response->tgroup_home_cpu,
					  response->tgroup_home_id, response->address);

	//printk("sizeof(data_response_for_2_kernels_t) %d PAGE_SIZE %d response->data_size %d\n",sizeof(data_response_for_2_kernels_t),PAGE_SIZE,response->data_size);
#if STATISTICS
	answer_request++;
#endif

	PSPRINTK("%s: Answer_request %i address %lx from cpu %i\n", __func__, 0, response->address, inc_msg->hdr.from_cpu);
	PSMINPRINTK("Received answer for address %lx last write %d from cpu %i\n", response->address, response->last_write,inc_msg->hdr.from_cpu);

	if (fetched_data == NULL) {
		printk(KERN_ERR"%s: WARN: data not found in local list (cpu %d id %d address 0x%lx) 0x%lx\n",
				__func__, response->tgroup_home_cpu, response->tgroup_home_id, response->address, (unsigned long)response);
		pcn_kmsg_free_msg(inc_msg);
		return -1;
	}

	if (response->owner == 1) {
		PSPRINTK("Response with ownership\n");
		fetched_data->owner = 1;
	}

	if (response->vma_present == 1) {
		if (response->header.from_cpu != response->tgroup_home_cpu)
			printk("%s: WARN: a kernel that is not the server is sending the mapping (cpu %d id %d address 0x%lx)\n",
					__func__, response->header.from_cpu, response->tgroup_home_cpu, response->address);

		PSPRINTK("%s: response->vma_pesent %d reresponse->vaddr_start %lx response->vaddr_size %lx response->prot %lx response->vm_flags %lx response->pgoff %lx response->path %s response->fowner %d\n", 
			__func__, response->vma_present, response->vaddr_start , response->vaddr_size, (unsigned long)response->prot, response->vm_flags , response->pgoff, response->path,response->futex_owner);

		if (fetched_data->vma_present == 0) {
			PSPRINTK("Set vma\n");
			fetched_data->vma_present = 1;
			fetched_data->vaddr_start = response->vaddr_start;
			fetched_data->vaddr_size = response->vaddr_size;
			fetched_data->prot = response->prot;
			fetched_data->pgoff = response->pgoff;
			fetched_data->vm_flags = response->vm_flags;
			strcpy(fetched_data->path, response->path);
		} else {
			printk("%s: WARN: received more than one mapping %d (cpu %d id %d address 0x%lx) 0x%lx\n",
					__func__, fetched_data->vma_present,
					response->tgroup_home_cpu, response->tgroup_home_id, response->address, response);
		}
	}

	if (fetched_data->address_present == 1) {
		printk("%s: WARN: received more than one answer with a copy of the page from %d (cpu %d id %d address 0x%lx) 0x%lx\n",
						__func__, response->header.from_cpu,
						response->tgroup_home_cpu, response->tgroup_home_id, response->address, response);
	}
	else  {
		fetched_data->address_present= 1;
		fetched_data->data = response;
		fetched_data->last_write = response->last_write;
		set = 1;
	}

	if (fetched_data->arrived_response!=0)
		printk("%s: WARN: received more than one answer, arrived_response is %d (cpu %d id %d address 0x%lx) 0x%lx\n",
				__func__, fetched_data->arrived_response,
				response->tgroup_home_cpu, response->tgroup_home_id, response->address, response);

	fetched_data->owners[inc_msg->hdr.from_cpu] = 1;
	fetched_data->arrived_response++;
	fetched_data->futex_owner = response->futex_owner;

	PSPRINTK("%s: wake up %d\n", __func__, fetched_data->waiting->pid);
	wake_up_process(fetched_data->waiting);

	if (set == 0)
		pcn_kmsg_free_msg(inc_msg);  // This is deleted by do_remote_*_for_2_kernels
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Handling acknowledges
///////////////////////////////////////////////////////////////////////////////
static int handle_ack(struct pcn_kmsg_message* inc_msg)
{
	ack_t* response;
	ack_answers_for_2_kernels_t* fetched_data;

	response = (ack_t*) inc_msg;
	fetched_data = find_ack_entry(response->tgroup_home_cpu,
				      response->tgroup_home_id, response->address);

#if STATISTICS
	ack++;
#endif
	PSPRINTK("Answer_invalid %i address %lx from cpu %i\n",
			ack, response->address, inc_msg->hdr.from_cpu);

	if (fetched_data == NULL) {
		goto out;
	}
	fetched_data->response_arrived++;

	if(fetched_data->response_arrived>1)
		printk("ERROR: received more than one ack\n");

	PSPRINTK("%s: wake up %d\n", __func__, fetched_data->waiting->pid);
			wake_up_process(fetched_data->waiting);

out:
	pcn_kmsg_free_msg(inc_msg);
	return 0;
}

void process_invalid_request_for_2_kernels(struct work_struct* work)
{
	invalid_work_t* work_request = (invalid_work_t*) work;
	invalid_data_for_2_kernels_t* data = work_request->request;
	ack_t* response;
	memory_t* memory = NULL;
	struct mm_struct* mm = NULL;
	struct vm_area_struct* vma;
	unsigned long address = data->address & PAGE_MASK;
	int from_cpu = data->header.from_cpu;
	pgd_t* pgd;
	pud_t* pud;
	pmd_t* pmd;
	pte_t* pte;
	pte_t entry;
	struct page* page;
	spinlock_t *ptl;
	int lock = 0;

	//unsigned long long start,end;
	invalid_work_t *delay;

#if STATISTICS
	invalid++;
#endif

	PSPRINTK("Invalid %i address %lx from cpu %i\n", invalid, data->address, from_cpu);
	PSMINPRINTK("Invalid for address %lx from cpu %i\n",data->address, from_cpu);

	//start= native_read_tsc();
	response= (ack_t*) kmalloc(sizeof(ack_t), GFP_ATOMIC);
	if(response==NULL){
		pcn_kmsg_free_msg(data);
		kfree(work);
		return;
	}
	response->writing = 0;

	memory = find_memory_entry(data->tgroup_home_cpu, data->tgroup_home_id);
	if (memory != NULL) {
		if(memory->setting_up==1){
			goto out;
		}
		mm = memory->mm;
	} else {
		goto out;
	}

	down_read(&mm->mmap_sem);
	//check the vma era first -- check the comment about VMA era in this file
	//if delayed is not a problem (but if > it is maybe a problem)
	if (mm->vma_operation_index > data->vma_operation_index)
			printk("%s: WARN: different era invalid [mm %d > data %d] (cpu %d id %d)\n",
					__func__, mm->vma_operation_index, data->vma_operation_index,
					data->tgroup_home_cpu, data->tgroup_home_id);

	if (mm->vma_operation_index < data->vma_operation_index) {
		printk("%s: WARN: different era invalid [mm %d < data %d] (cpu %d id %d)\n",
				__func__, mm->vma_operation_index, data->vma_operation_index,
				data->tgroup_home_cpu, data->tgroup_home_id);
		delay = (invalid_work_t*) kmalloc(sizeof(invalid_work_t), GFP_ATOMIC);

		if (delay!=NULL) {
			delay->request = data;
			INIT_DELAYED_WORK( (struct delayed_work*)delay,
					   process_invalid_request_for_2_kernels);
			queue_delayed_work(invalid_message_wq,
					   (struct delayed_work*) delay, 10);
		}

		up_read(&mm->mmap_sem);
		kfree(work);
		return;
	}

	// check if there is a valid vma
	vma = find_vma(mm, address);
	if (!vma || address >= vma->vm_end || address < vma->vm_start) {
		vma = NULL;
		if(_cpu == data->tgroup_home_cpu)
					printk(KERN_ALERT"%s: vma NULL in cpu %d address 0x%lx\n",
							__func__, _cpu, address);
	} else {
		if (unlikely(is_vm_hugetlb_page(vma))
		    || unlikely(transparent_hugepage_enabled(vma))) {
			printk("%s: ERROR: Request for HUGE PAGE vma\n", __func__);
			up_read(&mm->mmap_sem);
			goto out;
		}
	}

	pgd = pgd_offset(mm, address);
	if (!pgd || pgd_none(*pgd)) {
		up_read(&mm->mmap_sem);
		goto out;
	}
	pud = pud_offset(pgd, address);
	if (!pud || pud_none(*pud)) {
		up_read(&mm->mmap_sem);
		goto out;
	}
	pmd = pmd_offset(pud, address);
	if (!pmd || pmd_none(*pmd) || pmd_trans_huge(*pmd)) {
		up_read(&mm->mmap_sem);
		goto out;
	}

	pte = pte_offset_map_lock(mm, pmd, address, &ptl);
	/*PTE LOCKED*/
	lock = 1;

	//case pte not yet installed
#if defined(CONFIG_ARM64)
	//Ajith - Removing optimization used for local fetch - _PAGE_UNUSED1 case
	if (pte == NULL || pte_none(*pte) ) {
#else
	if (pte == NULL || pte_none(pte_clear_flags(*pte,_PAGE_UNUSED1)) ) {
#endif
		PSPRINTK("pte not yet mapped\n");
		//If I receive an invalid while it is not mapped, I must be fetching the page.
		//Otherwise it is an error.
		//Delay the invalid while I install the page.

		//Check if I am concurrently fetching the page
		mapping_answers_for_2_kernels_t* fetched_data = find_mapping_entry(
			data->tgroup_home_cpu, data->tgroup_home_id, address);

		if (fetched_data != NULL) {
			PSPRINTK("Concurrently fetching the same address\n");

			if(fetched_data->is_fetch!=1)
				printk("%s: ERROR: invalid received for a not mapped pte that has a mapping_answer not in fetching\n",
						__func__);

			delay = (invalid_work_t*) kmalloc(sizeof(invalid_work_t), GFP_ATOMIC);
			if (delay!=NULL) {
				delay->request = data;
				INIT_DELAYED_WORK( (struct delayed_work*)delay,
						   process_invalid_request_for_2_kernels);
				queue_delayed_work(invalid_message_wq,
						   (struct delayed_work*) delay, 10);
			}
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			kfree(work);
			return;
		}
		else
			printk("%s: ERROR: received an invalid for a not mapped pte not in fetching status (cpu %d id %d address 0x%lx)\n",
					__func__, data->tgroup_home_cpu, data->tgroup_home_id, address);

		goto out;
	} else {
		//the "standard" page fault releases the pte lock after that it installs the page
		//so before that I lock the pte again there is a moment in which is not null
		//but still fetching
		if (memory->alive != 0) {
			mapping_answers_for_2_kernels_t* fetched_data = find_mapping_entry(
				data->tgroup_home_cpu, data->tgroup_home_id, address);

			if(fetched_data!=NULL && fetched_data->is_fetch==1){
				delay = (invalid_work_t*) kmalloc(sizeof(invalid_work_t), GFP_ATOMIC);

				if (delay!=NULL) {
					delay->request = data;
					INIT_DELAYED_WORK( (struct delayed_work*)delay,
							   process_invalid_request_for_2_kernels);
					queue_delayed_work(invalid_message_wq,
							   (struct delayed_work*) delay, 10);
				}
				spin_unlock(ptl);
				up_read(&mm->mmap_sem);
				kfree(work);
				return;
			}
		}

		page = pte_page(*pte);
		if (page != vm_normal_page(vma, address, *pte)) {
			PSPRINTK("page different from vm_normal_page in request page\n");
		}
		if (page->replicated == 0 || page->status==REPLICATION_STATUS_NOT_REPLICATED) {
			printk("%s: ERROR: Invalid message in not replicated page cpu %d id %d address 0x%lx\n",
					__func__, data->tgroup_home_cpu, data->tgroup_home_id, address);
			goto out;
		}

		if (page->status == REPLICATION_STATUS_WRITTEN) {
			printk("%s: ERROR: invalid message in a written page cpu %d id %d address 0x%lx\n",
					__func__, data->tgroup_home_cpu, data->tgroup_home_id, address);
			goto out;
		}

		if (page->reading==1) {
			/*If I am reading my current status must be invalid and the one of the other kernel must be written.
			 *After that he sees my request of page, it mights want to write again and it sends me an invalid.
			 *So this request must be delayed.
			 */
			//printk("page reading when received invalid\n");

			if(page->status!=REPLICATION_STATUS_INVALID || page->last_write!=(data->last_write-1))
				printk("%s: WARN: Incorrect invalid received while reading address %lx, my status is %d, page last write %lx, invalid for version %lx (cpu %d id %d)\n",
						__func__, address,page->status,page->last_write,data->last_write,
						data->tgroup_home_cpu, data->tgroup_home_id);

			delay = (invalid_work_t*) kmalloc(sizeof(invalid_work_t), GFP_ATOMIC);

			if (delay!=NULL) {
				delay->request = data;
				INIT_DELAYED_WORK( (struct delayed_work*)delay,
						   process_invalid_request_for_2_kernels);
				queue_delayed_work(invalid_message_wq,
						   (struct delayed_work*) delay, 10);
			}
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			kfree(work);
			return;
		}

		if (page->writing==1) {
			/*Concurrent write.
			 *To be correct I must be or in valid or invalid state and not owner.
			 *The kernel with the ownership always wins.
			 */
			response->writing=1;
			if(page->owner==1 || page->status==REPLICATION_STATUS_WRITTEN)
				printk("%s: WARN: Incorrect invalid received while writing address %lx, my status is %d, page last write %lx, invalid for version %lx page owner %d (cpu %d id %d)\n", __func__,
				       address,page->status,page->last_write,data->last_write,page->owner,
					   data->tgroup_home_cpu, data->tgroup_home_id);

			//printk("received invalid while writing\n");
		}

		if (page->last_write!= data->last_write)
			printk("%s: WARN: received an invalid for copy %lx but my copy is %lx (cpu %d id %d)\n",
					__func__, data->last_write,page->last_write,
					data->tgroup_home_cpu, data->tgroup_home_id);

		page->status = REPLICATION_STATUS_INVALID;
		page->owner = 0;

// TODO check the following
		flush_cache_page(vma, address, pte_pfn(*pte));
		entry = *pte;
		//the page is invalid so as not present
		entry = pte_clear_valid_entry_flag(entry);
		entry = pte_mkyoung(entry);

		ptep_clear_flush(vma, address, pte);
		set_pte_at_notify(mm, address, pte, entry);

		update_mmu_cache(vma, address, pte);
		flush_tlb_page(vma, address);
		//flush_tlb_fix_spurious_fault(vma, address);

	    flush_tlb_page(vma, address);
	    flush_tlb_fix_spurious_fault(vma, address);
	}

out: if (lock) {
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
	}

	response->header.type = PCN_KMSG_TYPE_PROC_SRV_ACK_DATA;
	response->header.prio = PCN_KMSG_PRIO_NORMAL;
	response->tgroup_home_cpu = data->tgroup_home_cpu;
	response->tgroup_home_id = data->tgroup_home_id;
	response->address = data->address;
	response->ack = 1;

	//pcn_kmsg_send(from_cpu, (struct pcn_kmsg_message*) (response));
	pcn_kmsg_send_long(from_cpu,(struct pcn_kmsg_long_message*) (response),sizeof(ack_t)-sizeof(struct pcn_kmsg_hdr));
	kfree(response);
	pcn_kmsg_free_msg(data);
	kfree(work);
}

static int handle_invalid_request(struct pcn_kmsg_message* inc_msg)
{
	invalid_work_t* request_work;
	invalid_data_for_2_kernels_t* data = (invalid_data_for_2_kernels_t*) inc_msg;

	request_work = (invalid_work_t*)kmalloc(sizeof(invalid_work_t), GFP_ATOMIC);

	if (request_work!=NULL) {
		request_work->request = data;
		INIT_WORK( (struct work_struct*)request_work, process_invalid_request_for_2_kernels);
		queue_work(invalid_message_wq, (struct work_struct*) request_work);
	}
	return 1;
}

extern int do_wp_page_for_popcorn(struct mm_struct *mm, struct vm_area_struct *vma,
				  unsigned long address, pte_t *page_table, pmd_t *pmd,
				  spinlock_t *ptl, pte_t orig_pte);


void process_mapping_request_for_2_kernels(struct work_struct* work)
{
	request_work_t* request_work = (request_work_t*) work;

	data_request_for_2_kernels_t* request = request_work->request;
	data_void_response_for_2_kernels_t * void_response = NULL;
	data_response_for_2_kernels_t * response = NULL;
	mapping_answers_for_2_kernels_t * fetched_data = NULL;

	memory_t * memory = NULL;
	struct mm_struct * mm = NULL;
	struct vm_area_struct * vma = NULL;
	struct page * page = NULL, * old_page = NULL;

	int owner= 0;
	char * plpath; char lpath[512];
	int from_cpu = request->header.from_cpu;
	unsigned long address = request->address & PAGE_MASK;
	pgd_t* pgd;
	pud_t* pud;
	pmd_t* pmd;
	pte_t* pte;
	pte_t entry;

	spinlock_t* ptl;
	request_work_t* delay;

	int lock =0;
	void *vfrom;

#if STATISTICS
	request_data++;
#endif
	PSPRINTK("%s: Request address %lx is fetch %i is write %i\n",
			__func__, request->address, ((request->is_fetch==1)?1:0), ((request->is_write==1)?1:0));

	memory = find_memory_entry(request->tgroup_home_cpu, request->tgroup_home_id);
	if (memory != NULL) {
		if (memory->setting_up==1) {
			owner=1;
			goto out;
		}
		mm = memory->mm;
		if (memory->tgroup_home_cpu != request->tgroup_home_cpu
				|| memory->tgroup_home_id != request->tgroup_home_id)
			printk("%s: ERROR: tgroup_home_cpu and tgroup_home_id DON'T match", __func__);
	}
	else {
		owner=1;
		goto out;
	}

	down_read(&mm->mmap_sem);
	/*
	 * check the vma era first -- this is an error only if it keeps printing forever
	 * if it is still a problem do increment the delay
	 * In Marina's design we should only apply the same modifications at the same era
	 * Added the next check only to make sure that > is not a problem (but cannot be re-queued)
	 */
	if (mm->vma_operation_index > request->vma_operation_index)
		printk("%s: INFO: different era request [mm %d > request %d] (cpu %d id %d)\n",
				__func__, mm->vma_operation_index, request->vma_operation_index,
				request->tgroup_home_cpu, request->tgroup_home_id);

	if (mm->vma_operation_index < request->vma_operation_index) {
		printk("%s: WARN: different era request [mm %d < request %d] (cpu %d id %d)\n",
				__func__, mm->vma_operation_index, request->vma_operation_index,
				request->tgroup_home_cpu, request->tgroup_home_id);
		delay = (request_work_t*)kmalloc(sizeof(request_work_t), GFP_ATOMIC);
		if (delay) {
			delay->request = request;
			INIT_DELAYED_WORK( (struct delayed_work*)delay,
					   process_mapping_request_for_2_kernels);
			queue_delayed_work(message_request_wq,
					   (struct delayed_work*) delay, 10);
		}
		else {
			printk("%s: ERROR: cannot allocate memory to delay work 1 (cpu %d id %d)\n",
					__func__, request->tgroup_home_cpu, request->tgroup_home_id);
		}

		up_read(&mm->mmap_sem);
		kfree(work);
		return;
	}

	// check if there is a valid vma
	vma = find_vma(mm, address);
	if (!vma || address >= vma->vm_end || address < vma->vm_start) {
		vma = NULL;
		if(_cpu == request->tgroup_home_cpu){
			printk(KERN_ALERT"%s: ERROR: vma NULL in cpu %d address 0x%lx\n",
					__func__, _cpu, address);
			up_read(&mm->mmap_sem);
			goto out;
		}
	} else {
		if (unlikely(is_vm_hugetlb_page(vma))
		    || unlikely(transparent_hugepage_enabled(vma))) {
			printk("%s: ERROR: Request for HUGE PAGE vma\n", __func__);
			up_read(&mm->mmap_sem);
			goto out;
		}

		PSPRINTK("%s: Find vma from %s start %lx end %lx\n", __func__, ((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"), vma->vm_start, vma->vm_end);

	}

	PSPRINTK("%s: vma_flags = %lx\n", __func__, vma->vm_flags);	

	if(vma && vma->vm_flags & VM_FETCH_LOCAL)
	{
		printk("%s:%d - VM_FETCH_LOCAL flag set - Going to void response\n", __func__, __LINE__);
		up_read(&mm->mmap_sem);
		goto out;
	}

	if (_cpu != request->tgroup_home_cpu) {
		pgd = pgd_offset(mm, address);
		if (!pgd || pgd_none(*pgd)) {
			up_read(&mm->mmap_sem);
			goto out;
		}
		pud = pud_offset(pgd, address); //pud_alloc below
		if (!pud || pud_none(*pud)) {
			up_read(&mm->mmap_sem);
			goto out;
		}
		pmd = pmd_offset(pud, address); //pmd_alloc below
		if (!pmd || pmd_none(*pmd) || pmd_trans_huge(*pmd)) {
			up_read(&mm->mmap_sem);
			goto out;
		}
	}
	else {
		pgd = pgd_offset(mm, address);
		if (!pgd || pgd_none(*pgd)) {
			up_read(&mm->mmap_sem);
			goto out;
		}
		pud = pud_alloc(mm, pgd, address);
		if (!pud){
			up_read(&mm->mmap_sem);
			goto out;
		}
		pmd = pmd_alloc(mm, pud, address);
		if (!pmd){
			up_read(&mm->mmap_sem);
			goto out;
		}

		if (pmd_none(*pmd) && __pte_alloc(mm, vma, pmd, address)){
			up_read(&mm->mmap_sem);
			goto out;
		}
		if (unlikely(pmd_trans_huge(*pmd))) {
			printk("%s: ERROR: request for huge page\n", __func__);
			up_read(&mm->mmap_sem);
			goto out;
		}
	}
//retry:	
	pte = pte_offset_map_lock(mm, pmd, address, &ptl);
	/*PTE LOCKED*/
	entry = *pte;
	lock= 1;
///////////////////////////////////////////////////////////////////////////////
// Got pte, now proceed with the rest

#if defined(CONFIG_ARM64)
	//Ajith - Removing optimization used for local fetch - _PAGE_UNUSED1 case
	if (pte == NULL || pte_none(entry)) {
#else
	if (pte == NULL || pte_none(pte_clear_flags(entry, _PAGE_UNUSED1))) {
		PSPRINTK("pte not mapped\n");

		if ( !pte_none(entry) ) {
			if(_cpu!=request->tgroup_home_cpu || request->is_fetch==1){
				printk("%s: ERROR: incorrect request for marked page (cpu %d id %d address 0x%lx)\n",
						__func__, request->tgroup_home_cpu, request->tgroup_home_id, address);
				goto out;
			}
			else {
				PSPRINTK("request for a marked page\n");
			}
		}
#endif
		if ((_cpu==request->tgroup_home_cpu) || memory->alive != 0) {
			fetched_data = find_mapping_entry(
				request->tgroup_home_cpu, request->tgroup_home_id, address);

			//case concurrent fetch
			if (fetched_data != NULL) {
fetch:
				PSPRINTK("concurrent request\n");
				/*Whit marked pages only two scenarios can happenn:
				 * Or I am the main and I an locally fetching=> delay this fetch
				 * Or I am not the main, but the main already answer to my fetch (otherwise it will not answer to me the page)
				 * so wait that the answer arrive before consuming the fetch.
				 * */
				if (fetched_data->is_fetch != 1)
					printk("%s: ERROR: find a mapping_answers_for_2_kernels_t not mapped and not fetch (cpu %d id %d address 0x%lx)\n",
							__func__, request->tgroup_home_cpu, request->tgroup_home_id, address);

				delay = (request_work_t*)kmalloc(sizeof(request_work_t), GFP_ATOMIC);
				if (delay) {
					delay->request = request;
					INIT_DELAYED_WORK(
						(struct delayed_work*)delay,
						process_mapping_request_for_2_kernels);
					queue_delayed_work(message_request_wq,
							   (struct delayed_work*) delay, 10);
				}
				else {
					printk("%s: ERROR: cannot allocate memory to delay work 2 (cpu %d id %d)\n",
							__func__, request->tgroup_home_cpu, request->tgroup_home_id);
				}

				spin_unlock(ptl);
				up_read(&mm->mmap_sem);
				kfree(work);
				return;
			}
			else {
				//mark the pte if main
				if (_cpu==request->tgroup_home_cpu) {
					PSPRINTK(KERN_ALERT"%s: marking a pte for address %lx\n",__func__,address);

#if defined (CONFIG_ARM64)
					//Ajith - Removing optimization used for local fetch - _PAGE_UNUSED1 case
					//entry = pte_set_flags(entry, _PAGE_UNUSED1);
#else
					entry = pte_set_flags(entry, _PAGE_UNUSED1);
#endif
					ptep_clear_flush(vma, address, pte);

					set_pte_at_notify(mm, address, pte, entry);
					//in x86 does nothing
					update_mmu_cache(vma, address, pte);
					flush_tlb_page(vma, address);

					flush_tlb_page(vma, address);
					flush_tlb_fix_spurious_fault(vma, address);
				}
			}
		}
		//pte not present
		owner= 1;
		goto out;
	}
	page = pte_page(entry);
	if (page != vm_normal_page(vma, address, entry)) {
		PSPRINTK("Page different from vm_normal_page in request page\n");
	}
	old_page = NULL;
///////////////////////////////////////////////////////////////////////////////
// Got page, proceed with the rest

	if (is_zero_page(pte_pfn(entry)) || !(page->replicated == 1)) {
		PSPRINTK("Page not replicated\n");

		/*There is the possibility that this request arrived while I am fetching, after that I installed the page
		 * but before calling the update page....
		 * */
		if (memory->alive != 0) {
			fetched_data = find_mapping_entry (
					request->tgroup_home_cpu, request->tgroup_home_id, address);

			if (fetched_data!=NULL) {
				goto fetch;
			}
		}

		//the request must be for a fetch
		if (request->is_fetch==0)
			printk("%s: WARN: received a request not fetch for a not replicated page (cpu %d id %d address 0x%lx)\n",
					__func__, request->tgroup_home_cpu, request->tgroup_home_id, address);

		if (vma->vm_flags & VM_WRITE) {
			//if the page is writable but the pte has not the write flag set, it is a cow page
			if (!pte_write(entry)) {

			retry_cow:
				PSPRINTK("COW page at %lx\n", address);

				int ret= do_wp_page_for_popcorn(mm, vma,address, pte,pmd,ptl, entry);
				if (ret & VM_FAULT_ERROR) {
					if (ret & VM_FAULT_OOM){
						printk("ERROR: %s VM_FAULT_OOM\n",__func__);
						up_read(&mm->mmap_sem);
						goto out;
					}
					if (ret & (VM_FAULT_HWPOISON | VM_FAULT_HWPOISON_LARGE)){
						printk("ERROR: %s EHWPOISON\n",__func__);
						up_read(&mm->mmap_sem);
						goto out;
					}
					if (ret & VM_FAULT_SIGBUS){
						printk("ERROR: %s EFAULT\n",__func__);
						up_read(&mm->mmap_sem);
						goto out;
					}
					printk("%s: ERROR: bug from do_wp_page_for_popcorn (cpu %d id %d address 0x%lx)\n",
							__func__, request->tgroup_home_cpu, request->tgroup_home_id, request->address);
					up_read(&mm->mmap_sem);
					goto out;
				}
				spin_lock(ptl);
				/*PTE LOCKED*/
				lock = 1;
				entry = *pte;

				if (!pte_write(entry)) {
					printk("%s: WARN: page not writable after cow (cpu %d id %d address 0x%lx)\n",
							__func__, request->tgroup_home_cpu, request->tgroup_home_id, request->address);
					goto retry_cow;
				}
				page = pte_page(entry);
			}

			page->replicated = 1;
			flush_cache_page(vma, address, pte_pfn(*pte));
			entry = mk_pte(page, vma->vm_page_prot);

			if (request->is_write==0) {
				//case fetch for read
				page->status = REPLICATION_STATUS_VALID;
				entry = pte_wrprotect(entry);
				entry = pte_set_valid_entry_flag(entry);
				owner= 0;
				page->owner= 1;
			}
			else {
				//case fetch for write
				page->status = REPLICATION_STATUS_INVALID;
				entry = pte_clear_valid_entry_flag(entry);
				owner= 1;
				page->owner= 0;
			}
			page->last_write= 1;

			entry = pte_set_user_access_flag(entry);
			entry = pte_mkyoung(entry);

			ptep_clear_flush(vma, address, pte);
			set_pte_at_notify(mm, address, pte, entry);

			//in x86 does nothing
			update_mmu_cache(vma, address, pte);
			flush_tlb_page(vma, address);

			flush_tlb_page(vma, address);
			flush_tlb_fix_spurious_fault(vma, address);
			if (old_page != NULL) {
				page_remove_rmap(old_page);
			}
		}
		else {
			//read only vma
			page->replicated=0;
			page->status= REPLICATION_STATUS_NOT_REPLICATED;

			if (request->is_write==1) {
				printk("%s: ERROR: received a write in a read-only not replicated page(cpu %d id %d address 0x%lx)\n",
					__func__, request->tgroup_home_cpu, request->tgroup_home_id, request->address);
			}
			page->owner= 1;
			owner= 0;
		}

		page->other_owners[_cpu]=1;
		page->other_owners[from_cpu]=1;

		goto resolved;
	}
	else {
		//replicated page case
		PSPRINTK("%s: Page replicated...\n", __func__);

		if (request->is_fetch==1) {
			printk("%s: ERROR: received a fetch request in a replicated status (cpu %d id %d address 0x%lx)\n",
					__func__, request->tgroup_home_cpu, request->tgroup_home_id, request->address);
					//BUG(); // TODO removed for debugging but really needed here
		}
		if (page->writing==1) {
			PSPRINTK("Page currently in writing\n");

			delay = (request_work_t*)kmalloc(sizeof(request_work_t), GFP_ATOMIC);
			if (delay) {
				delay->request = request;
				INIT_DELAYED_WORK( (struct delayed_work*)delay,
						   process_mapping_request_for_2_kernels);
				queue_delayed_work(message_request_wq,
						   (struct delayed_work*) delay, 10);
			}
			else {
				printk("%s: ERROR: cannot allocate memory to delay work 3 (cpu %d id %d)\n",
						__func__, request->tgroup_home_cpu, request->tgroup_home_id);
			}

			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			kfree(work);
			return;
		}
		if (page->reading==1) {
			printk("%s: ERROR: page in reading but received a request (cpu %d id %d address 0x%lx)\n",
					__func__, request->tgroup_home_cpu, request->tgroup_home_id, request->address);
			goto out;
		}

		//invalid page case
		if (page->status == REPLICATION_STATUS_INVALID) {
			printk("%s: ERROR: received a request in invalid status without reading or writing (cpu %d id %d address 0x%lx)\n",
					__func__, request->tgroup_home_cpu, request->tgroup_home_id, request->address);
			goto out;
		}

		//valid page case
		if (page->status == REPLICATION_STATUS_VALID) {
			PSPRINTK("Page requested valid\n");

			if (page->owner!=1) {
				printk("%s: ERROR: request in a not owner valid page (cpu %d id %d address 0x%lx)\n",
					__func__, request->tgroup_home_cpu, request->tgroup_home_id, request->address);
			}
			else {
				if (request->is_write) {
					if (page->last_write!= request->last_write)
						printk("%s: ERROR: received a write for copy %lx but my copy is %lx\n",
								__func__, request->last_write, page->last_write);

					page->status= REPLICATION_STATUS_INVALID;
					page->owner= 0;
					owner= 1;
					entry = *pte;
					entry = pte_clear_valid_entry_flag(entry);
					entry = pte_mkyoung(entry);

					ptep_clear_flush(vma, address, pte);
					set_pte_at_notify(mm, address, pte, entry);

					update_mmu_cache(vma, address, pte);
					flush_tlb_page(vma, address);
					flush_tlb_fix_spurious_fault(vma, address);
				}
				else {
					printk("%s: ERROR: received a read request in valid status (cpu %d id %d address 0x%lx)\n",
							__func__, request->tgroup_home_cpu, request->tgroup_home_id, request->address);
				}
			}
			goto out;
		}

		if (page->status == REPLICATION_STATUS_WRITTEN) {
			PSPRINTK("Page requested in written status\n");

			if (page->owner!=1) {
				printk("%s: ERROR: page in written status without ownership (cpu %d id %d address 0x%lx)\n",
					__func__, request->tgroup_home_cpu, request->tgroup_home_id, request->address);
			}
			else {
				if (request->is_write==1) {
					if (page->last_write!= (request->last_write+1))
						printk("%s: ERROR: received a write for copy %lx but my copy is %lx (cpu %d id %d address 0x%lx)\n",
								__func__, request->last_write,page->last_write,
								request->tgroup_home_cpu, request->tgroup_home_id, request->address );

					page->status= REPLICATION_STATUS_INVALID;
					page->owner= 0;
					owner= 1;
					entry = *pte;
					entry = pte_clear_valid_entry_flag(entry);
					entry = pte_mkyoung(entry);

					ptep_clear_flush(vma, address, pte);
					set_pte_at_notify(mm, address, pte, entry);
					update_mmu_cache(vma, address, pte);
					flush_tlb_page(vma, address);
					flush_tlb_fix_spurious_fault(vma, address);
				}
				else {
					if (page->last_write!= (request->last_write+1))
						printk("%s: ERROR: received an read for copy %lx but my copy is %lx (cpu %d id %d address 0x%lx)\n",
								__func__, request->last_write,page->last_write,
								request->tgroup_home_cpu, request->tgroup_home_id, request->address);

					page->status = REPLICATION_STATUS_VALID;
					page->owner= 1;
					owner= 0;
					entry = *pte;
					entry = pte_set_valid_entry_flag(entry);
					entry = pte_mkyoung(entry);
					entry = pte_wrprotect(entry);

					ptep_clear_flush(vma, address, pte);
					set_pte_at_notify(mm, address, pte, entry);
					update_mmu_cache(vma, address, pte);
					flush_tlb_page(vma, address);
					flush_tlb_fix_spurious_fault(vma, address);
				}
			}
			goto resolved;
		}
	}
///////////////////////////////////////////////////////////////////////////////
// Everything resolved at this point, going to respond

resolved:
	PSPRINTK("Resolved Copy from %s\n",
			((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"));
	PSPRINTK("Page read only?%i Page shared?%i\n",
			(vma->vm_flags & VM_WRITE)?0:1, (vma->vm_flags & VM_SHARED)?1:0);

	response = (data_response_for_2_kernels_t*) kmalloc(sizeof(data_response_for_2_kernels_t)+PAGE_SIZE, GFP_ATOMIC);
	if (response == NULL) {
		printk("%s: ERROR: Impossible to kmalloc in process mapping request.\n", __func__); //THIS CASE SHOULD KILL THE PROCESS
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		pcn_kmsg_free_msg(request);
		kfree(work);
		return;
	}

	void* vto = &(response->data);
	// Ported to Linux 3.12 
	//vfrom = kmap_atomic(page, KM_USER0);
	vfrom = kmap_atomic(page);

#if READ_PAGE
	int ct=0;
	unsigned long _buff[16];

	if (address == PAGE_ADDR) {
		for (ct=0;ct<8;ct++) {
			_buff[ct]=(unsigned long) *(((unsigned long *)vfrom) + ct);
		}
	}
#endif

	//printk("Copying page (address) : 0x%lx\n", address);
	copy_page(vto, vfrom);
	// Ported to Linux 3.12 
	//kunmap_atomic(vfrom, KM_USER0);
	kunmap_atomic(vfrom);
	response->data_size= PAGE_SIZE;


#if READ_PAGE
	if (address == PAGE_ADDR) {
		for(ct=8;ct<16;ct++) {
			_buff[ct]=(unsigned long) *((unsigned long*)(&(response->data))+ct-8);
		}
		for(ct=0;ct<16;ct++){
			printk(KERN_ALERT"{%lx} ",_buff[ct]);
		}
	}
#endif

	flush_cache_page(vma, address, pte_pfn(*pte));
	response->last_write = page->last_write;
	response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE;
	response->header.prio = PCN_KMSG_PRIO_NORMAL;
	response->tgroup_home_cpu = request->tgroup_home_cpu;
	response->tgroup_home_id = request->tgroup_home_id;
	response->address = request->address;
	response->owner= owner;

	response->futex_owner = (!page) ? 0 : page->futex_owner;//akshay

	if (_cpu == request->tgroup_home_cpu && vma != NULL) {
		//only the vmas SERVER sends the vma

		response->vma_present = 1;
		response->vaddr_start = vma->vm_start;
		response->vaddr_size = vma->vm_end - vma->vm_start;
		response->prot = vma->vm_page_prot;
		response->vm_flags = vma->vm_flags;
		response->pgoff = vma->vm_pgoff;
		if (vma->vm_file == NULL) {
			response->path[0] = '\0';
		}
		else {
			plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			strcpy(response->path, plpath);
		}
		PSPRINTK("response->vma_present %d response->vaddr_start %lx response->vaddr_size %lx response->prot %lx response->vm_flags %lx response->pgoff %lx response->path %s response->futex_owner %d\n",
			 response->vma_present, response->vaddr_start , response->vaddr_size,response->prot, response->vm_flags , response->pgoff, response->path,response->futex_owner);
	}
	else {
		response->vma_present = 0;
	}
	spin_unlock(ptl);
	up_read(&mm->mmap_sem);

	// Send response
	pcn_kmsg_send_long(from_cpu, (struct pcn_kmsg_long_message*) (response),
			   sizeof(data_response_for_2_kernels_t) - sizeof(struct pcn_kmsg_hdr) + response->data_size);
	// Clean up incoming messages
	pcn_kmsg_free_msg(request);
	kfree(work);
	kfree(response);
	//end= native_read_tsc();
	PSPRINTK("Handle request end\n");
	return;

out:
	printk("%s sending void answer\n", __func__);

/*	char tmpchar; // Marina and Vincent debugging
	char *addrp = (char *) (address & PAGE_MASK);
	int ii;
	for (ii = 1; ii < PAGE_SIZE; ii++) {
		copy_from_user(&tmpchar, addrp++, 1);
	}*/

#if 0
	void* vto2 = kmalloc(GFP_ATOMIC, PAGE_SIZE * 2);

	// Ported to Linux 3.12
	//vfrom = kmap_atomic(page, KM_USER0);
	vfrom = kmap_atomic(page);
	//printk("Copying page (address) : 0x%lx\n", address);
	copy_page(vto2, vfrom);
	// Ported to Linux 3.12
	//kunmap_atomic(vfrom, KM_USER0);
	kunmap_atomic(vfrom);
	kfree(vto2);
#endif

	void_response = (data_void_response_for_2_kernels_t*) kmalloc(
		sizeof(data_void_response_for_2_kernels_t), GFP_ATOMIC);
	if (void_response == NULL) {
		if (lock) {
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
		}
		printk("%s: ERROR: Impossible to kmalloc in process mapping request.\n", __func__);
		pcn_kmsg_free_msg(request);
		kfree(work);
		return;
	}

	void_response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE_VOID;
	void_response->header.prio = PCN_KMSG_PRIO_NORMAL;
	void_response->tgroup_home_cpu = request->tgroup_home_cpu;
	void_response->tgroup_home_id = request->tgroup_home_id;
	void_response->address = request->address;
	void_response->owner=owner;
	void_response->futex_owner = 0;//TODO: page->futex_owner;//akshay

	if (_cpu == request->tgroup_home_cpu && vma != NULL) {
		void_response->vma_present = 1;
		void_response->vaddr_start = vma->vm_start;
		void_response->vaddr_size = vma->vm_end - vma->vm_start;
		void_response->prot = vma->vm_page_prot;
		void_response->vm_flags = vma->vm_flags;
		void_response->pgoff = vma->vm_pgoff;
		if (vma->vm_file == NULL) {
			void_response->path[0] = '\0';
		}
		else {
			plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			strcpy(void_response->path, plpath);
		}
	}
	else {
		void_response->vma_present = 0;
	}

	if(lock) {
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
	}

	// Send response
	pcn_kmsg_send_long(from_cpu,
			   (struct pcn_kmsg_long_message*) (void_response),
			   sizeof(data_void_response_for_2_kernels_t) - sizeof(struct pcn_kmsg_hdr));
	// Clean up incoming messages
	pcn_kmsg_free_msg(request);
	kfree(void_response);
	kfree(work);
	//end= native_read_tsc();
	PSPRINTK("Handle request end\n");
}

static int handle_mapping_request(struct pcn_kmsg_message* inc_msg)
{
	request_work_t* request_work;

	data_request_for_2_kernels_t* request = (data_request_for_2_kernels_t*) inc_msg;

	request_work = kmalloc(sizeof(request_work_t), GFP_ATOMIC);

	if (request_work) {
		request_work->request = request;
		INIT_WORK( (struct work_struct*)request_work, process_mapping_request_for_2_kernels);
		queue_work(message_request_wq, (struct work_struct*) request_work);
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// exit group notification
///////////////////////////////////////////////////////////////////////////////
void process_exit_group_notification(struct work_struct* work)
{
	exit_group_work_t* request_exit = (exit_group_work_t*) work;
	thread_group_exited_notification_t* msg = request_exit->request;
	unsigned long flags;

	memory_t* mm_data = find_memory_entry(msg->tgroup_home_cpu,
					      msg->tgroup_home_id);
	if (mm_data) {
		while (mm_data->main == NULL)
			schedule();

		lock_task_sighand(mm_data->main, &flags);
		mm_data->main->distributed_exit = EXIT_PROCESS;
		unlock_task_sighand(mm_data->main, &flags);

		wake_up_process(mm_data->main);
	}

	pcn_kmsg_free_msg(msg);
	kfree(work);
}

void process_exiting_process_notification(struct work_struct* work)
{
	exit_work_t* request_work = (exit_work_t*) work;
	exiting_process_t* msg = request_work->request;

	unsigned int source_cpu = msg->header.from_cpu;
	struct task_struct *task;
	
	task = pid_task(find_get_pid(msg->prev_pid), PIDTYPE_PID);
	
	if (task && task->next_pid == msg->my_pid
		&& task->next_cpu == source_cpu
	    && task->represents_remote == 1) {

		// TODO: Handle return values
		restore_thread_info(task, &msg->arch);

		task->group_exit = msg->group_exit;
		task->distributed_exit_code = msg->code;
#if MIGRATE_FPU
		// TODO: Handle return values
		restore_fpu_info(task, &msg->arch);
#endif
		wake_up_process(task);
	}
	else {
		printk(KERN_ALERT"%s: ERROR: task not found. Impossible to kill shadow (pid %d cpu %d)\n",
				__func__, msg->my_pid, source_cpu);
	}

	pcn_kmsg_free_msg(msg);
	kfree(work);
}

static int handle_thread_group_exited_notification(struct pcn_kmsg_message* inc_msg)
{
	exit_group_work_t* request_work;
	thread_group_exited_notification_t* request =
		(thread_group_exited_notification_t*) inc_msg;

	request_work = kmalloc(sizeof(exit_group_work_t), GFP_ATOMIC);

	if (request_work) {
		request_work->request = request;
		INIT_WORK( (struct work_struct*)request_work,
			   process_exit_group_notification);
		queue_work(exit_group_wq, (struct work_struct*) request_work);
	}
	return 1;
}

static int handle_exiting_process_notification(struct pcn_kmsg_message* inc_msg) {
	exit_work_t* request_work;
	exiting_process_t* request = (exiting_process_t*) inc_msg;

	request_work = kmalloc(sizeof(exit_work_t), GFP_ATOMIC);

	if (request_work) {
		request_work->request = request;
		INIT_WORK( (struct work_struct*)request_work,
			   process_exiting_process_notification);
		queue_work(exit_wq, (struct work_struct*) request_work);
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// pairing request
///////////////////////////////////////////////////////////////////////////////
/**
 * Handler function for when another processor informs the current cpu
 * of a pid pairing.
 */
static int handle_process_pairing_request(struct pcn_kmsg_message* inc_msg) {
	create_process_pairing_t* msg = (create_process_pairing_t*) inc_msg;
	unsigned int source_cpu = msg->header.from_cpu;
	struct task_struct* task;
	
	if (inc_msg == NULL) {
		return -1;
	}

	if (msg == NULL) {
		pcn_kmsg_free_msg(inc_msg);
		return -1;
	}

	task = find_task_by_vpid(msg->your_pid);
	if (task == NULL || task->represents_remote == 0) {
		return -1;
	}
	task->next_cpu = source_cpu;
	task->next_pid = msg->my_pid;
	task->executing_for_remote = 0;
	pcn_kmsg_free_msg(inc_msg);
	return 1;
}

static int handle_remote_thread_count_response(struct pcn_kmsg_message* inc_msg)
{
	remote_thread_count_response_t* msg= (remote_thread_count_response_t*) inc_msg;
	count_answers_t* data = find_count_entry(msg->tgroup_home_cpu,
						 msg->tgroup_home_id);
	unsigned long flags;
	struct task_struct* to_wake = NULL;

	PSPRINTK("%s: entered - cpu{%d}, id{%d}, count{%d}\n", __func__, msg->tgroup_home_cpu, msg->tgroup_home_id, msg->count);

	if (data == NULL) {
		printk("%s: ERROR: unable to find remote thread count data (cpu %d id %d)\n",
				__func__, msg->tgroup_home_cpu, msg->tgroup_home_id);
		pcn_kmsg_free_msg(inc_msg);
		return -1;
	}

	raw_spin_lock_irqsave(&(data->lock), flags);

	// Register this response.
	data->responses++;
	data->count += msg->count;

	if (data->responses >= data->expected_responses)
		to_wake = data->waiting;

	raw_spin_unlock_irqrestore(&(data->lock), flags);

	if (to_wake != NULL) {
		PSPRINTK("%s: wake up %d\n", __func__, to_wake->pid);
		wake_up_process(to_wake);
	}

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

void process_count_request(struct work_struct* work)
{
	count_work_t* request_work = (count_work_t*) work;
	remote_thread_count_request_t* msg = request_work->request;
	remote_thread_count_response_t* response;
	struct task_struct *tgroup_iterator;

	PSPRINTK("%s: entered - cpu{%d}, id{%d}\n", __func__, msg->tgroup_home_cpu, msg->tgroup_home_id);

	response= (remote_thread_count_response_t*) kmalloc(sizeof(remote_thread_count_response_t),GFP_ATOMIC);
	if(!response)
		return;
	response->count = 0;

	/* This is needed to know if the requesting kernel has to save the mapping or send the group dead message.
	 * If there is at least one alive thread of the process in the system the mapping must be saved.
	 * I count how many threads there are but actually I can stop when I know that there is one.
	 * If there are no more threads in the system, a group dead message should be sent by at least one kernel.
	 * I do not need to take the sighand lock (used to set task->distributed_exit=1) because:
	 * --count remote thread is called AFTER set task->distributed_exit=1
	 * --if I am here the last thread of the process in the requesting kernel already set his flag distributed_exit to 1
	 * --two things can happend if the last thread of the process is in this kernel and it is dying too:
	 * --1. set its flag before I check it => I send 0 => the other kernel will send the message
	 * --2. set its flag after I check it => I send 1 => I will send the message
	 * Is important to not take the lock so everything can be done in the messaging layer without fork another kthread.
	 */

	memory_t* memory = find_memory_entry(msg->tgroup_home_cpu,
					     msg->tgroup_home_id);
	if (memory != NULL) {
		while (memory->main == NULL)
			schedule();

		tgroup_iterator = memory->main;
		while_each_thread(memory->main, tgroup_iterator)
		{
			if (tgroup_iterator->distributed_exit == EXIT_ALIVE
			    && tgroup_iterator->main != 1) {
				response->count++;
				goto out;
			}
		};
	}

	// Finish constructing response
out:
	response->header.type = PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_RESPONSE;
	response->header.prio = PCN_KMSG_PRIO_NORMAL;
	response->tgroup_home_cpu = msg->tgroup_home_cpu;
	response->tgroup_home_id = msg->tgroup_home_id;
	PSPRINTK(KERN_EMERG"%s: responding to thread count request with %d\n", __func__, response->count);
	// Send response
	pcn_kmsg_send_long(msg->header.from_cpu, (struct pcn_kmsg_long_message*) (response),sizeof(remote_thread_count_response_t)-sizeof(struct pcn_kmsg_hdr));
	pcn_kmsg_free_msg(msg);
	kfree(response);
	kfree(request_work);
}

static int handle_remote_thread_count_request(struct pcn_kmsg_message* inc_msg)
{
	count_work_t* request_work;
	remote_thread_count_request_t* request =(remote_thread_count_request_t*) inc_msg;
	request_work = kmalloc(sizeof(count_work_t), GFP_ATOMIC);

	if (request_work) {
		request_work->request = request;
		INIT_WORK( (struct work_struct*)request_work,
			   process_count_request);
		queue_work(exit_wq, (struct work_struct*) request_work);
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// synch migrations (?)
///////////////////////////////////////////////////////////////////////////////
void synchronize_migrations(int tgroup_home_cpu,int tgroup_home_id )
{
	memory_t* memory = NULL;

	memory = find_memory_entry(tgroup_home_cpu, tgroup_home_id);
	if (!memory) {
		printk("%s: ERROR: no memory_t found (cpu %d id %d)\n",
				__func__, tgroup_home_cpu, tgroup_home_id);
		BUG();
		return;
	}

	/*while (atomic_read(&(memory->pending_back_migration))<57) {
	  msleep(1);
	  }*/
	//int app= atomic_add_return(-1,&(memory->pending_back_migration));
	/*if(app==57)
	  atomic_set(&(memory->pending_back_migration),0);*/

	atomic_dec(&(memory->pending_back_migration));
	while (atomic_read(&(memory->pending_back_migration))!=0) {
		msleep(1);
	}
}

///////////////////////////////////////////////////////////////////////////////
// handling back migration
///////////////////////////////////////////////////////////////////////////////
void process_back_migration(struct work_struct* work)
{
	back_mig_work_t* info_work = (back_mig_work_t*) work;
	back_migration_request_t* request = info_work->back_mig_request;

	struct task_struct * task = pid_task(find_get_pid(request->prev_pid), PIDTYPE_PID);
	
	PSPRINTK("%s: PID %d, prev_pid: %d\n",__func__, task->pid, request->prev_pid);
	if (task!=NULL
			&& (task->next_pid == request->placeholder_pid)
			&& (task->next_cpu == request->header.from_cpu)
			&& (task->represents_remote == 1)) {
		// TODO: Handle return values
		restore_thread_info(task, &request->arch);
		initialize_thread_retval(task, 0);

		task->prev_cpu = request->header.from_cpu;
		task->prev_pid = request->placeholder_pid;
		task->personality = request->personality;

		//	task->origin_pid = request->origin_pid;
		//	sigorsets(&task->blocked,&task->blocked,&request->remote_blocked) ;
		//	sigorsets(&task->real_blocked,&task->real_blocked,&request->remote_real_blocked);
		//	sigorsets(&task->saved_sigmask,&task->saved_sigmask,&request->remote_saved_sigmask);
		//	task->pending = request->remote_pending;
		//	task->sas_ss_sp = request->sas_ss_sp;
		//	task->sas_ss_size = request->sas_ss_size;

		// int cnt = 0;
		//	for (cnt = 0; cnt < _NSIG; cnt++)
		//		task->sighand->action[cnt] = request->action[cnt];
#if MIGRATE_FPU
		// TODO: Handle return values
		restore_fpu_info(task, &request->arch);
#endif

		task->executing_for_remote = 1;
		task->represents_remote = 0;
		wake_up_process(task);
	}
	else {
		printk("%s: ERROR: task not found. Impossible to re-run shadow (pid %d cpu %d)\n",
				__func__, request->placeholder_pid, request->header.from_cpu);
	}
	pcn_kmsg_free_msg(request);
	kfree(work);
}

static int handle_back_migration(struct pcn_kmsg_message* inc_msg)
{
	back_migration_request_t* request= (back_migration_request_t*) inc_msg;

#if MIGRATION_PROFILE
	migration_start = ktime_get();
#endif
	//for synchronizing migratin threads
	memory_t* memory = NULL;

	//PSPRINTK(" IN %s:%d values - %d %d\n", __func__, __LINE__,request->tgroup_home_cpu, request->tgroup_home_id);
	memory = find_memory_entry(request->tgroup_home_cpu,
				   request->tgroup_home_id);
	if (memory) {
		atomic_inc(&(memory->pending_back_migration));
		/*int app= atomic_add_return(1,&(memory->pending_back_migration));
		  if(app==57)
		  atomic_set(&(memory->pending_back_migration),114);*/
	}
	else {
		printk("%s: ERROR: back migration did not find a memory_t struct! (cpu %d id %d)\n",
				__func__, request->tgroup_home_cpu, request->tgroup_home_id);
	}
	/*back_mig_work_t* work;
	  work = (back_mig_work_t*) kmalloc(sizeof(back_mig_work_t), GFP_ATOMIC);
	  if (work) {
	  INIT_WORK( (struct work_struct*)work, process_back_migration);
	  work->back_mig_request = request;
	  queue_work(clone_wq, (struct work_struct*) work);
	  } */

	//temporary code to check if the back migration can be faster
	struct task_struct * task = pid_task(find_get_pid(request->prev_pid), PIDTYPE_PID);

	if (task != NULL && (task->next_pid == request->placeholder_pid)
	    && (task->next_cpu == request->header.from_cpu)
	    && (task->represents_remote == 1)) {
		// TODO: Handle return values
		restore_thread_info(task, &request->arch);
		initialize_thread_retval(task, 0);

		task->prev_cpu = request->header.from_cpu;
		task->prev_pid = request->placeholder_pid;
		task->personality = request->personality;
		task->executing_for_remote = 1;
		task->represents_remote = 0;
		wake_up_process(task);

#if MIGRATION_PROFILE
		migration_end = ktime_get();
		printk("m exec %lld\n", (long)ktime_to_ns(migration_end));
#if defined(CONFIG_ARM64)
  		printk(KERN_ERR "Time for x86->arm back migration - ARM side: %ld ns\n", (unsigned long)ktime_to_ns(ktime_sub(migration_end, migration_start)));
#else
		printk(KERN_ERR "Time for arm->x86 back migration - x86 side: %ld ns\n", (unsigned long)ktime_to_ns(ktime_sub(migration_end, migration_start)));
#endif
#endif
	}
	else {
		printk("%s: ERROR: task not found. Impossible to re-run shadow (pid %d cpu %d)\n",
				__func__, request->placeholder_pid, request->header.from_cpu);
	}
	pcn_kmsg_free_msg(request);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// exit notification
///////////////////////////////////////////////////////////////////////////////
/**
 * Notify of the fact that either a delegate or placeholder has died locally.
 * In this case, the remote cpu housing its counterpart must be notified, so
 * that it can kill that counterpart.
 */
int process_server_task_exit_notification(struct task_struct *tsk, long code)
{
	int tx_ret = -1;
	int count = 0;
	memory_t* entry = NULL;
	unsigned long flags;
	
	PSPRINTK("MORTEEEEEE-Process_server_task_exit_notification - pid{%d}\n", tsk->pid);
	PSPRINTK("%s - pid %d\n", __func__, tsk->pid);

	if (tsk->distributed_exit==EXIT_ALIVE) {
		entry = find_memory_entry(tsk->tgroup_home_cpu, tsk->tgroup_home_id);
		if (entry) {
			while (entry->main == NULL)
				schedule();
		}
		else {
			printk("%s: ERROR: Mapping disappeared, cannot wake up main thread... (cpu %d id %d)\n",
					__func__, tsk->tgroup_home_cpu, tsk->tgroup_home_id);
			return -1;
		}

		lock_task_sighand(tsk, &flags);
		tsk->distributed_exit = EXIT_THREAD;
		if (entry->main->distributed_exit == EXIT_ALIVE)
			entry->main->distributed_exit = EXIT_THREAD;
		unlock_task_sighand(tsk, &flags);

		/* If I am executing on behalf of a thread on another kernel,
		 * notify the shadow of that thread that I am dying.
		 */
		if (tsk->executing_for_remote) {
			exiting_process_t* msg = (exiting_process_t*) kmalloc(
				sizeof(exiting_process_t), GFP_ATOMIC);

			if (msg != NULL) {
				msg->header.type = PCN_KMSG_TYPE_PROC_SRV_EXIT_PROCESS;
				msg->header.prio = PCN_KMSG_PRIO_NORMAL;
				msg->my_pid = tsk->pid;
				msg->prev_pid = tsk->prev_pid;

				// TODO: Handle return value
				save_thread_info(tsk, task_pt_regs(tsk), &msg->arch, NULL);

				if (tsk->group_exit == 1)
					msg->group_exit = 1;
				else
					msg->group_exit = 0;
				msg->code = code;

				msg->is_last_tgroup_member = (count == 1 ? 1 : 0);
#if MIGRATE_FPU
				// TODO: Handle return value
				save_fpu_info(tsk, &msg->arch);
#endif
				//printk("message exit to shadow sent\n");
				tx_ret = pcn_kmsg_send_long(tsk->prev_cpu,
							    (struct pcn_kmsg_long_message*) msg,
							    sizeof(exiting_process_t) - sizeof(struct pcn_kmsg_hdr));
				kfree(msg);
			}
		}
		wake_up_process(entry->main);
	}
	return tx_ret;
}


/**
 * Create a pairing between a newly created delegate process and the
 * remote placeholder process.  This function creates the local
 * pairing first, then sends a message to the originating cpu
 * so that it can do the same.
 */
int process_server_notify_delegated_subprocess_starting(pid_t pid,
							pid_t remote_pid, int remote_cpu)
{
	create_process_pairing_t* msg;
	int tx_ret = -1;

	msg= (create_process_pairing_t*) kmalloc(sizeof(create_process_pairing_t),GFP_ATOMIC);
	if(!msg)
		return -1;
	// Notify remote cpu of pairing between current task and remote
	// representative task.
	msg->header.type = PCN_KMSG_TYPE_PROC_SRV_CREATE_PROCESS_PAIRING;
	msg->header.prio = PCN_KMSG_PRIO_NORMAL;
	msg->your_pid = remote_pid;
	msg->my_pid = pid;

	tx_ret = pcn_kmsg_send_long(remote_cpu, (struct pcn_kmsg_long_message *) msg,
				    sizeof(create_process_pairing_t)-sizeof(struct pcn_kmsg_hdr));
	kfree(msg);

	return tx_ret;

}

///////////////////////////////////////////////////////////////////////////////
// update page function
///////////////////////////////////////////////////////////////////////////////

/* No other kernels had the page during the remote fetch => a local fetch has been performed.
 * If during the local fetch a thread in another kernel asks for this page, I would not set the page as replicated.
 * This function check if the page sould be set as replicated.
 *
 * the mm->mmap_sem semaphore is already held in read
 * return types:
 * VM_FAULT_OOM, problem allocating memory.
 * VM_FAULT_VMA, error vma management.
 * VM_FAULT_REPLICATION_PROTOCOL, replication protocol error.
 * 0, updated;
 */

int process_server_update_page(struct task_struct * tsk, struct mm_struct *mm,
			       struct vm_area_struct *vma, unsigned long address_not_page, unsigned long page_fault_flags, int retrying)
{
	unsigned long address;

	pgd_t* pgd;
	pud_t* pud;
	pmd_t* pmd;
	pte_t* pte;
	pte_t entry;
	spinlock_t* ptl = NULL;
	struct page* page;
	int ret = 0;

	mapping_answers_for_2_kernels_t* fetched_data;

	address = address_not_page & PAGE_MASK;

	if (!vma || address >= vma->vm_end || address < vma->vm_start) {
		printk("%s: ERROR: updating a page without valid vma (cpu %d id %d)\n",
				__func__, tsk->tgroup_home_cpu, tsk->tgroup_home_id);
		ret = VM_FAULT_VMA;
		goto out_not_data;
	}

	if (unlikely(is_vm_hugetlb_page(vma))
	    || unlikely(transparent_hugepage_enabled(vma))) {
		printk("%s: ERROR: Installed a vma with HUGEPAGE (cpu %d id %d)\n",
				__func__, tsk->tgroup_home_cpu, tsk->tgroup_home_id);
		ret = VM_FAULT_VMA;
		goto out_not_data;
	}

	fetched_data = find_mapping_entry(tsk->tgroup_home_cpu, tsk->tgroup_home_id,
			address);

	if (fetched_data != NULL) {
		if (retrying == 1) {
			ret = 0;
			goto out_not_locked;
		}
<<<<<<< HEAD
=======

>>>>>>> 298b5c847840ce6c634a99ae6608ea7b19cb5ec9
		if(fetched_data->is_fetch!=1 ){
			printk("%s: ERROR: data structure is not for fetch (cpu %d id %d)\n",
					__func__, tsk->tgroup_home_cpu, tsk->tgroup_home_id);
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto out_not_locked;
		}

		pgd = pgd_offset(mm, address);
		pud = pud_offset(pgd, address);
		if (!pud) {
			printk("%s: ERROR: no pud while trying to update a page locally fetched (cpu %d id %d)\n",
					__func__, tsk->tgroup_home_cpu, tsk->tgroup_home_id);
			ret = VM_FAULT_VMA;
			goto out_not_locked;
		}
		pmd = pmd_offset(pud, address);
		if (!pmd) {
			printk("%s: ERROR: no pmd while trying to update a page locally fetched (cpu %d id %d)\n",
					__func__, tsk->tgroup_home_cpu, tsk->tgroup_home_id);
			ret = VM_FAULT_VMA;
			goto out_not_locked;
		}
//retry:
		pte = pte_offset_map_lock(mm, pmd, address, &ptl);
		entry= *pte;
		page = pte_page(entry);

		//I replicate only if it is a writable page
		if (vma->vm_flags & VM_WRITE) {
			if (!pte_write(entry)) {
retry_cow:
				PSPRINTK("COW page at %lx\n", address);

				int cow_ret= do_wp_page_for_popcorn(mm, vma,address, pte,pmd,ptl, entry);
				if (cow_ret & VM_FAULT_ERROR) {
					if (cow_ret & VM_FAULT_OOM){
						printk("%s: ERROR: VM_FAULT_OOM\n",__func__);
						ret = VM_FAULT_OOM;
						goto out_not_locked;
					}
					if (cow_ret & (VM_FAULT_HWPOISON | VM_FAULT_HWPOISON_LARGE)){
						printk("%s: ERROR: EHWPOISON\n",__func__);
						ret = VM_FAULT_OOM;
						goto out_not_locked;
					}
					if (cow_ret & VM_FAULT_SIGBUS){
						printk("%s: ERROR: EFAULT\n",__func__);
						ret = VM_FAULT_OOM;
						goto out_not_locked;
					}
					printk("%s: ERROR: bug from do_wp_page_for_popcorn\n",__func__);
					ret = VM_FAULT_OOM;
					goto out_not_locked;
				}

				spin_lock(ptl);
				/*PTE LOCKED*/
				entry = *pte;

				if(!pte_write(entry)){
					printk("%s: WARNING: page not writable after cow\n", __func__);
					goto retry_cow;
				}
				page = pte_page(entry);
			}

			page->replicated = 0;
			page->owner= 1;
			page->other_owners[_cpu] = 1;
		}
		else {
			page->replicated = 0;
			page->other_owners[_cpu] = 1;
			page->owner= 1;
		}
	}
	else {
		printk("%s: ERROR: impossible to find data to update (cpu %d id %d address 0x%lx)\n",
				__func__, tsk->tgroup_home_cpu, tsk->tgroup_home_id, address);
		ret = VM_FAULT_REPLICATION_PROTOCOL;
		goto out_not_data;
	}
	spin_unlock(ptl);

out_not_locked:
	remove_mapping_entry(fetched_data);
	kfree(fetched_data);
out_not_data:
	wake_up(&read_write_wait);

	return ret;
}

void process_server_clean_page(struct page* page)
{
	if (page == NULL) {
		return;
	}

	page->replicated = 0;
	page->status = REPLICATION_STATUS_NOT_REPLICATED;
	page->owner = 0;
	memset(page->other_owners, 0, MAX_KERNEL_IDS*sizeof(int));
	page->writing = 0;
	page->reading = 0;
}

/* Read on a REPLICATED page => ask a copy of the page at address "address" on the
 * virtual mapping of the process identified by "tgroup_home_cpu" and "tgroup_home_id".
 *
 * down_read(&mm->mmap_sem) must be held.
 * pte lock must be held.
 *
 *return types:
 *VM_FAULT_OOM, problem allocating memory.
 *VM_FAULT_VMA, error vma management.
 *VM_FAULT_REPLICATION_PROTOCOL, general error.
 *0, write succeeded;
 * */
int do_remote_read_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
				 struct mm_struct *mm, struct vm_area_struct *vma,
				 unsigned long address, unsigned long page_fault_flags,
				 pmd_t* pmd, pte_t* pte,
				 spinlock_t* ptl, struct page* page)
{
	int i, ret=0;
	pte_t value_pte;
	page->reading= 1;

#if STATISTICS
	read++;
#endif
	PSPRINTK("%s Read for address %lx pid %d\n", __func__, address,current->pid);

	//message to ask for a copy
	data_request_for_2_kernels_t* read_message = (data_request_for_2_kernels_t*) kmalloc(sizeof(data_request_for_2_kernels_t),
											     GFP_ATOMIC);
	if (read_message == NULL) {
		ret = VM_FAULT_OOM;
		goto exit;
	}

	read_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
	read_message->header.prio = PCN_KMSG_PRIO_NORMAL;
	read_message->address = address;
	read_message->tgroup_home_cpu = tgroup_home_cpu;
	read_message->tgroup_home_id = tgroup_home_id;
	read_message->is_fetch= 0;
	read_message->is_write= 0;
	read_message->last_write= page->last_write;
	read_message->vma_operation_index= current->mm->vma_operation_index;
	PSPRINTK("%s vma_operation_index %d\n", __func__, read_message->vma_operation_index);

	//object to held responses
	mapping_answers_for_2_kernels_t* reading_page = (mapping_answers_for_2_kernels_t*) kmalloc(sizeof(mapping_answers_for_2_kernels_t),
												   GFP_ATOMIC);
	if (reading_page == NULL) {
		ret = VM_FAULT_OOM;
		goto exit_read_message;
	}

	reading_page->tgroup_home_cpu= tgroup_home_cpu;
	reading_page->tgroup_home_id= tgroup_home_id;
	reading_page->address = address;
	reading_page->address_present= 0;
	reading_page->data= NULL;
	reading_page->is_fetch= 0;
	reading_page->is_write= 0;
	reading_page->last_write= page->last_write;
	reading_page->owner= 0;

	reading_page->vma_present = 0;
	reading_page->vaddr_start = 0;
	reading_page->vaddr_size = 0;
	reading_page->pgoff = 0;
	memset(reading_page->path,0,sizeof(char)*512);
	memset(&(reading_page->prot),0,sizeof(pgprot_t));
	reading_page->vm_flags = 0;
	reading_page->waiting = current;

	// Make data entry visible to handler.
	add_mapping_entry(reading_page);
	PSPRINTK("Sending a read message for address %lx\n ", address);
	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	/*PTE UNLOCKED*/
	int sent= 0;
	reading_page->arrived_response=0;

	// the list does not include the current processor group descirptor (TODO)
	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;

	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;

		if (!(pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (read_message),
					 sizeof(data_request_for_2_kernels_t)-sizeof(struct pcn_kmsg_hdr)) == -1)) {
			// Message delivered
			sent++;
			if (sent>1)
				printk("%s: ERROR: using protocol optimized for 2 kernels but sending a read to more than one kernel",
						__func__);
		}
	}

	if (sent) {
		long counter = 0;
		while (reading_page->arrived_response == 0) {
			//set_task_state(current, TASK_UNINTERRUPTIBLE); // TODO put it back
			if (reading_page->arrived_response == 0)
				schedule();
			//set_task_state(current, TASK_RUNNING); // TODO put it back
			if (!(++counter % 1000000))
				printk("%s: WARN: reading_page->arrived_response 0 [%ld] (cpu %d id %d address 0x%lx)\n",
						__func__, counter, tgroup_home_cpu, tgroup_home_id, address);
		}
	}
	else {
		printk("%s: ERROR: impossible to send read message, no destination kernel (cpu %d id %d)n",
				__func__, tgroup_home_cpu, tgroup_home_id);
		ret= VM_FAULT_REPLICATION_PROTOCOL;
		down_read(&mm->mmap_sem);
		spin_lock(ptl);
		goto exit_reading_page;
	}

	down_read(&mm->mmap_sem);
	spin_lock(ptl);
	/*PTE LOCKED*/

	vma = find_vma(mm, address);
	if (unlikely(!vma || address >= vma->vm_end || address < vma->vm_start)) {
		printk("%s: ERROR: vma not valid during read for write (cpu %d id %d address 0x%lx)\n",
				__func__, tgroup_home_cpu, tgroup_home_id, address);
		ret = VM_FAULT_VMA;
		goto exit_reading_page;
	}

	if (reading_page->address_present==1) {
		if (reading_page->data->address != address) {
			printk("%s: ERROR: trying to copy wrong address! (cpu %d id %d address 0x%lx)\n",
				__func__, tgroup_home_cpu, tgroup_home_id, address);
			pcn_kmsg_free_msg(reading_page->data);
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}

		if (reading_page->last_write != (page->last_write+1)) {
			printk("%s: ERROR: new copy received during a read but my last write is %lx and received last write is %lx\n",
			       __func__, page->last_write,reading_page->last_write);
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}
		else {
			page->last_write= reading_page->last_write;
		}

		if (reading_page->owner==1) {
			printk("%s: ERROR: owneship sent with read request (cpu %d id %d address 0x%lx)\n",
				__func__, tgroup_home_cpu, tgroup_home_id, address);
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}

		void *vto;
		void *vfrom;
		// Ported to Linux 3.12
		//vto = kmap_atomic(page, KM_USER0);
		vto = kmap_atomic(page);
		vfrom = &(reading_page->data->data);

		copy_user_page(vto, vfrom, address, page);

		// Ported to Linux 3.12
		//kunmap_atomic(vto, KM_USER0);
		kunmap_atomic(vto);

		pcn_kmsg_free_msg(reading_page->data);

		page->status = REPLICATION_STATUS_VALID;
		page->owner = reading_page->owner;

#if STATISTICS
		if(page->last_write> most_written_page)
			most_written_page= page->last_write;
#endif

		flush_cache_page(vma, address, pte_pfn(*pte));
		//now the page can be written
		value_pte = *pte;
		value_pte = pte_wrprotect(value_pte);
		value_pte = pte_set_valid_entry_flag(value_pte);

#if defined(CONFIG_ARM64)
		value_pte = pte_mkyoung(value_pte);
#else
		value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);
#endif
		ptep_clear_flush(vma, address, pte);
		set_pte_at_notify(mm, address, pte, value_pte);
		update_mmu_cache(vma, address, pte);

		flush_tlb_page(vma, address);

		flush_tlb_fix_spurious_fault(vma, address);
		PSPRINTK("%s: Out read %i address %lx\n ", __func__, 0, address);
	}
	else {
		printk("%s: ERROR: no copy received for a read (cpu %d id %d address 0x%lx)\n",
				__func__, tgroup_home_cpu, tgroup_home_id, address);
		ret= VM_FAULT_REPLICATION_PROTOCOL;
		remove_mapping_entry(reading_page);
		kfree(reading_page);
		kfree(read_message);
		goto exit;

	}
exit_reading_page:
	remove_mapping_entry(reading_page);
	kfree(reading_page);

exit_read_message:
	kfree(read_message);

exit:
	page->reading = 0;
	return ret;
}

/* Write on a REPLICATED page => coordinate with other kernels to write on the page at address "address" on the
 * virtual mapping of the process identified by "tgroup_home_cpu" and "tgroup_home_id".
 *
 * down_read(&mm->mmap_sem) must be held.
 * pte lock must be held.
 *
 *return types:
 *VM_FAULT_OOM, problem allocating memory.
 *VM_FAULT_VMA, error vma management.
 *VM_FAULT_REPLICATION_PROTOCOL, general error.
 *0, write succeeded;
 * */
int do_remote_write_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
				  struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
				  unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
				  struct page* page,int invalid)
{

	int i, ret= 0;
	pte_t value_pte;
	page->writing = 1;

#if STATISTICS
	write++;
#endif
	PSPRINTK("%s: Write %i address %lx pid %d\n", __func__, 0, address,current->pid);
	PSMINPRINTK("Write for address %lx owner %d pid %d\n", address,page->owner==1?1:0,current->pid);

	if (page->owner==1) {
		int sent= 0;

		//in this case I send and invalid message
		if(invalid) {
			printk("%s: ERROR: I am the owner of the page and it is invalid when going to write (cpu %d id %d address 0x%lx)\n",
					__func__, tgroup_home_cpu, tgroup_home_id, address);
			ret= VM_FAULT_REPLICATION_PROTOCOL;
			goto exit;
		}
		//object to store the acks (nacks) sent by other kernels
		ack_answers_for_2_kernels_t* answers = (ack_answers_for_2_kernels_t*) kmalloc(sizeof(ack_answers_for_2_kernels_t), GFP_ATOMIC);
		if (answers == NULL) {
			ret = VM_FAULT_OOM;
			goto exit;
		}
		answers->tgroup_home_cpu = tgroup_home_cpu;
		answers->tgroup_home_id = tgroup_home_id;
		answers->address = address;
		answers->waiting = current;

		//message to invalidate the other copies
		invalid_data_for_2_kernels_t* invalid_message = (invalid_data_for_2_kernels_t*) kmalloc(sizeof(invalid_data_for_2_kernels_t),
													GFP_ATOMIC);
		if (invalid_message == NULL) {
			ret = VM_FAULT_OOM;
			goto exit_answers;
		}
		invalid_message->header.type = PCN_KMSG_TYPE_PROC_SRV_INVALID_DATA;
		invalid_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		invalid_message->tgroup_home_cpu = tgroup_home_cpu;
		invalid_message->tgroup_home_id = tgroup_home_id;
		invalid_message->address = address;
		invalid_message->vma_operation_index= current->mm->vma_operation_index;

		// Insert the object in the appropriate list.
		add_ack_entry(answers);

		invalid_message->last_write = page->last_write;
		answers->response_arrived= 0;

		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		/*PTE UNLOCKED*/
		// the list does not include the current processor group descirptor (TODO)
		struct list_head *iter;
		_remote_cpu_info_list_t *objPtr;
		list_for_each(iter, &rlist_head) {
			objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
			i = objPtr->_data._processor;
			if (page->other_owners[i] == 1) {
				if (!(pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (invalid_message),sizeof(invalid_data_for_2_kernels_t)-sizeof(struct pcn_kmsg_hdr))
				      == -1)) {
					// Message delivered
					sent++;
					if (sent>1)
						printk("%s: ERROR: using protocol optimized for 2 kernels but sending an invalid to more than one kernel",
								__func__);
				}
			}
		}

		if (sent) {
			long counter = 0;
			while (answers->response_arrived==0) {
				//set_task_state(current, TASK_INTERRUPTIBLE); // TODO put it back
				if (answers->response_arrived==0)
					schedule();
				//set_task_state(current, TASK_RUNNING); // TODO put it back
				if (!(++counter % 1000000))
					printk("%s: WARN: writing_page->arrived_response 0 [%ld] (cpu %d id %d address 0x%lx)\n",
							__func__, counter, tgroup_home_cpu, tgroup_home_id, address);
			}
		}
		else {
			printk("%s: ERROR: impossible to send read message, no destination kernel (cpu %d id %d)n",
					__func__, tgroup_home_cpu, tgroup_home_id);
			ret= VM_FAULT_REPLICATION_PROTOCOL;
			down_read(&mm->mmap_sem);
			spin_lock(ptl);
			goto exit_invalid;
		}

		down_read(&mm->mmap_sem);
		spin_lock(ptl);

		/*PTE LOCKED*/
		vma = find_vma(mm, address);
		if (unlikely(!vma || address >= vma->vm_end || address < vma->vm_start)) {
			printk("%s: ERROR: vma not valid after waiting for ack to invalid (cpu %d id %d)\n",
					__func__, tgroup_home_cpu, tgroup_home_id);
			ret = VM_FAULT_VMA;
			goto exit_invalid;
		}
		PSPRINTK("%s: Received ack to invalid %i address %lx\n", __func__, 0, address);

exit_invalid:
		kfree(invalid_message);
		remove_ack_entry(answers);
exit_answers:
		kfree(answers);
		if(ret!=0)
			goto exit;
	}
	else {
		//in this case I send a mapping request with write flag set
		//message to ask for a copy
		data_request_for_2_kernels_t* write_message = (data_request_for_2_kernels_t*) kmalloc(sizeof(data_request_for_2_kernels_t),
												      GFP_ATOMIC);
		if (write_message == NULL) {
			ret = VM_FAULT_OOM;
			goto exit;
		}

		write_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
		write_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		write_message->address = address;
		write_message->tgroup_home_cpu = tgroup_home_cpu;
		write_message->tgroup_home_id = tgroup_home_id;
		write_message->is_fetch= 0;
		write_message->is_write= 1;
		write_message->last_write= page->last_write;
		write_message->vma_operation_index= current->mm->vma_operation_index;
		PSPRINTK("%s vma_operation_index %d\n", __func__, write_message->vma_operation_index);

		//object to held responses
		mapping_answers_for_2_kernels_t* writing_page = (mapping_answers_for_2_kernels_t*) kmalloc(sizeof(mapping_answers_for_2_kernels_t),
													   GFP_ATOMIC);
		if (writing_page == NULL) {
			ret = VM_FAULT_OOM;
			goto exit_write_message;

		}

		writing_page->tgroup_home_cpu= tgroup_home_cpu;
		writing_page->tgroup_home_id= tgroup_home_id;
		writing_page->address = address;
		writing_page->address_present= 0;
		writing_page->data= NULL;
		writing_page->is_fetch= 0;
		writing_page->is_write= 1;
		writing_page->last_write= page->last_write;
		writing_page->owner= 0;
		writing_page->vma_present = 0;
		writing_page->vaddr_start = 0;
		writing_page->vaddr_size = 0;
		writing_page->pgoff = 0;
		memset(writing_page->path,0,sizeof(char)*512);
		memset(&(writing_page->prot),0,sizeof(pgprot_t));
		writing_page->vm_flags = 0;
		writing_page->waiting = current;

		// Make data entry visible to handler.
		add_mapping_entry(writing_page);
		PSPRINTK("Sending a write message for address %lx\n ", address);

		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		/*PTE UNLOCKED*/
		int sent= 0;
		writing_page->arrived_response=0;

		// the list does not include the current processor group descriptor (TODO)
		struct list_head *iter;
		_remote_cpu_info_list_t *objPtr;
		list_for_each(iter, &rlist_head) {
			objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
			i = objPtr->_data._processor;
			if (page->other_owners[i] == 1) {
				if (!(pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (write_message),sizeof(data_request_for_2_kernels_t)-sizeof(struct pcn_kmsg_hdr))
				      == -1)) {
					// Message delivered
					sent++;
					if(sent>1)
						printk("%s: ERROR: using protocol optimized for 2 kernels but sending a write to more than one kernel",
								__func__);
				}
			}
		}

		if (sent) {
			long counter =0;
			while (writing_page->arrived_response == 0) {
				//set_task_state(current, TASK_UNINTERRUPTIBLE); // TODO put it back
				if (writing_page->arrived_response == 0)
					schedule();
				//set_task_state(current, TASK_RUNNING); // TODO put it back
				if (!(++counter % 1000000))
					printk("%s: WARN: writing_page->arrived_response 0 [%ld] !owner (cpu %d id %d address 0x%lx)\n",
							__func__, counter, tgroup_home_cpu, tgroup_home_id, address);
			}
		}
		else {
			printk("%s: ERROR: impossible to send write message, no destination kernel (cpu %d id %d)\n",
					__func__, tgroup_home_cpu, tgroup_home_id);
			ret= VM_FAULT_REPLICATION_PROTOCOL;
			down_read(&mm->mmap_sem);
			spin_lock(ptl);
			goto exit_writing_page;
		}

		down_read(&mm->mmap_sem);
		spin_lock(ptl);
		/*PTE LOCKED*/

		vma = find_vma(mm, address);
		if (unlikely(!vma || address >= vma->vm_end || address < vma->vm_start)) {
			printk("ERROR: vma not valid during read for write\n");
			ret = VM_FAULT_VMA;
			goto exit_writing_page;
		}

		if(writing_page->owner!=1){
			printk("%s: ERROR: received answer to write without ownership (cpu %d id %d)\n",
					__func__, tgroup_home_cpu, tgroup_home_id);
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_writing_page;
		}
		if(writing_page->address_present==1){
			if (writing_page->data->address != address) {
				printk("%s: ERROR: trying to copy wrong address 0x%lx (cpu %d id %d)\n",
						__func__, address, tgroup_home_cpu, tgroup_home_id);
				pcn_kmsg_free_msg(writing_page->data);
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_writing_page;
			}
			//in this case I also received the new copy
			if (writing_page->last_write != (page->last_write+1)) {
				pcn_kmsg_free_msg(writing_page->data);
				printk(
					"%s: ERROR: new copy received during a write but my last write is %lx and received last write is %lx\n",
					__func__, page->last_write,writing_page->last_write);
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_writing_page;
			}
			else
				page->last_write= writing_page->last_write;

			void *vto;
			void *vfrom;
			// Ported to Linux 3.12 
			//vto = kmap_atomic(page, KM_USER0);
			vto = kmap_atomic(page);
			vfrom = &(writing_page->data->data);

			copy_user_page(vto, vfrom, address, page);

			// Ported to Linux 3.12 
			//kunmap_atomic(vto, KM_USER0);
			kunmap_atomic(vto);
			pcn_kmsg_free_msg(writing_page->data);

exit_writing_page:
			remove_mapping_entry(writing_page);
			kfree(writing_page);
exit_write_message:
			kfree(write_message);

			if(ret!=0)
				goto exit;
		}
		else{
			remove_mapping_entry(writing_page);
			kfree(writing_page);
			kfree(write_message);

			if(invalid){
				printk("%s: ERROR: writing an invalid page but not received a copy address 0x%lx (cpu %d id %d)\n",
						__func__, address, tgroup_home_cpu, tgroup_home_id);
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				goto exit;
			}
		}
	}
	page->status = REPLICATION_STATUS_WRITTEN;
	page->owner = 1;
	(page->last_write)++;

#if STATISTICS
	if(page->last_write> most_written_page)
		most_written_page= page->last_write;
#endif

	flush_cache_page(vma, address, pte_pfn(*pte));

	//now the page can be written
	value_pte = *pte;

	value_pte = pte_mkwrite(value_pte);
	/* In kernel - page is made dirty as soon as it is made writeable */
	value_pte = pte_mkdirty(value_pte);
	value_pte = pte_set_valid_entry_flag(value_pte);
	//value_pte=pte_set_flags(value_pte,_PAGE_USER);
	value_pte = pte_mkyoung(value_pte);
	//value_pte=pte_set_flags(value_pte,_PAGE_DIRTY);
	ptep_clear_flush(vma, address, pte);
	set_pte_at_notify(mm, address, pte, value_pte);
	update_mmu_cache(vma, address, pte);

	flush_tlb_page(vma, address);

#if defined(CONFIG_ARM64)
	//flush_tlb_fix_spurious_fault(vma, address);
#else
	flush_tlb_fix_spurious_fault(vma, address);
#endif
	PSPRINTK("%s: Out write %i address %lx last write is %lx\n ",
			__func__, 0, address,page->last_write);

exit:
	page->writing = 0;
	return ret;
}

static unsigned long map_difference(struct file *file, unsigned long addr,
				    unsigned long len, unsigned long prot, unsigned long flags,
				    unsigned long pgoff)
{
	unsigned long ret = addr;
	unsigned long start = addr;
	unsigned long local_end = start;
	unsigned long end = addr + len;
	struct vm_area_struct* curr;
	unsigned long error;
	unsigned long populate = 0;

	// go through ALL vma's, looking for interference with this space.
	curr = current->mm->mmap;

	while (1) {

		if (start >= end)
			goto done;

		// We've reached the end of the list
		else if (curr == NULL) {
			// map through the end
			// Ported to Linux 3.12
			//error = do_mmap(file, start, end - start, prot, flags, pgoff);
			error = do_mmap_pgoff(file, start, end - start, prot, flags, pgoff >> PAGE_SHIFT, &populate);
			if (error != start) {
				printk("{ERROR: return value %d\n", error);
				ret = VM_FAULT_VMA;
			}
			goto done;
		}

		// the VMA is fully above the region of interest
		else if (end <= curr->vm_start) {
			// mmap through local_end
			// Ported to Linux 3.12
			// error = do_mmap(file, start, end - start, prot, flags, pgoff);
			
			error = do_mmap_pgoff(file, start, end - start, prot, flags, pgoff >> PAGE_SHIFT, &populate);
			if (error != start)
				ret = VM_FAULT_VMA;
			goto done;
		}

		// the VMA fully encompases the region of interest
		else if (start >= curr->vm_start && end <= curr->vm_end) {
			// nothing to do
			goto done;
		}

		// the VMA is fully below the region of interest
		else if (curr->vm_end <= start) {
			// move on to the next one

		}

		// the VMA includes the start of the region of interest
		// but not the end
		else if (start >= curr->vm_start && start < curr->vm_end
			 && end > curr->vm_end) {
			// advance start (no mapping to do)
			start = curr->vm_end;
			local_end = start;

		}

		// the VMA includes the end of the region of interest
		// but not the start
		else if (start < curr->vm_start && end <= curr->vm_end
				&& end > curr->vm_start) {
			local_end = curr->vm_start;

			// mmap through local_end
			// Ported to Linux 3.12
			// error = do_mmap(file, start, local_end - start, prot, flags, pgoff);
			error = do_mmap_pgoff(file, start, local_end - start, prot, flags, pgoff >> PAGE_SHIFT, &populate);
			if (error != start)
				ret = VM_FAULT_VMA;

			// Then we're done
			goto done;
		}

		// the VMA is fully within the region of interest
		else if (start <= curr->vm_start && end >= curr->vm_end) {
			// advance local end
			local_end = curr->vm_start;

			// map the difference
			// Ported to Linux 3.12
			// error = do_mmap(file, start, local_end - start, prot, flags, pgoff);
			error = do_mmap_pgoff(file, start, local_end - start, prot, flags, pgoff >> PAGE_SHIFT, &populate);
			if (error != start)
				ret = VM_FAULT_VMA;

			// Then advance to the end of this vma
			start = curr->vm_end;
			local_end = start;
		}

		curr = curr->vm_next;
	}

done:
	return ret;
}

static int do_mapping_for_distributed_process(mapping_answers_for_2_kernels_t* fetching_page,
					      struct mm_struct* mm, unsigned long address, spinlock_t* ptl)
{

	struct vm_area_struct* vma;
	unsigned long prot = 0;
	unsigned long err, ret;

	prot |= (fetching_page->vm_flags & VM_READ) ? PROT_READ : 0;
	prot |= (fetching_page->vm_flags & VM_WRITE) ? PROT_WRITE : 0;
	prot |= (fetching_page->vm_flags & VM_EXEC) ? PROT_EXEC : 0;

	if (fetching_page->vma_present == 1) {
		if (fetching_page->path[0] == '\0') {

			vma = find_vma(mm, address);
			if (!vma || address >= vma->vm_end || address < vma->vm_start) {
				vma = NULL;
			}

			if (!vma || (vma->vm_start != fetching_page->vaddr_start)
			    || (vma->vm_end != (fetching_page->vaddr_start + fetching_page->vaddr_size))) {

				spin_unlock(ptl);
				/*PTE UNLOCKED*/

				/* Note: during a page fault the distribute lock is held in read =>
				 * distributed vma operations cannot happen in the same time
				 */
				up_read(&mm->mmap_sem);
				down_write(&mm->mmap_sem);

				/* when I release the down write on mmap_sem, another thread of my process
				 * could install the same vma that I am trying to install
				 * (only fetch of same addresses are prevent, not fetch of different addresses on the same vma)
				 * take the newest vma.
				 * */
				vma = find_vma(mm, address);
				if (!vma || address >= vma->vm_end || address < vma->vm_start) {
					vma = NULL;
				}

				/* All vma operations are distributed, except for mmap =>
				 * When I receive a vma, the only difference can be on the size (start, end) of the vma.
				 */
				if (!vma || (vma->vm_start != fetching_page->vaddr_start)
				    || (vma->vm_end
					!= (fetching_page->vaddr_start
					    + fetching_page->vaddr_size))) {
					PSPRINTK(
						"Mapping anonymous vma start %lx end %lx\n", fetching_page->vaddr_start, (fetching_page->vaddr_start + fetching_page->vaddr_size));

					/*Note:
					 * This mapping is caused because when a thread migrates it does not have any vma
					 * so during fetch vma can be pushed.
					 * This mapping has the precedence over "normal" vma operations because is a page fault
					 * */

					current->mm->distribute_unmap = 0;

					/*map_difference should map in such a way that no unmap operations (the only nested operation that mmap can call) are nested called.
					 * This is important both to not unmap pages that should not be unmapped
					 * but also because otherwise the vma protocol will deadlock!
					 */
					err = map_difference(NULL, fetching_page->vaddr_start,
							     fetching_page->vaddr_size, prot,
							     MAP_FIXED | MAP_ANONYMOUS
							     | ((fetching_page->vm_flags & VM_SHARED) ?
								MAP_SHARED : MAP_PRIVATE)
							     | ((fetching_page->vm_flags & VM_HUGETLB) ?
								MAP_HUGETLB : 0)
							     | ((fetching_page->vm_flags & VM_GROWSDOWN) ?
								MAP_GROWSDOWN : 0), 0);

					current->mm->distribute_unmap = 1;

					if (err != fetching_page->vaddr_start) {
						up_write(&mm->mmap_sem);
						down_read(&mm->mmap_sem);
						spin_lock(ptl);
						/*PTE LOCKED*/
						printk(
							"%s: ERROR: error mapping anonymous vma while fetching address %lx\n",
							__func__, address);
						ret = VM_FAULT_VMA;
						return ret;
					}
				}

				up_write(&mm->mmap_sem);
				down_read(&mm->mmap_sem);
				spin_lock(ptl);
				/*PTE LOCKED*/
			}
		}
		else {
			vma = find_vma(mm, address);
			if (!vma || address >= vma->vm_end || address < vma->vm_start) {
				vma = NULL;
			}

			if (!vma || (vma->vm_start != fetching_page->vaddr_start)
			    || (vma->vm_end != (fetching_page->vaddr_start + fetching_page->vaddr_size))) {

				spin_unlock(ptl);
				/*PTE UNLOCKED*/

				up_read(&mm->mmap_sem);

				struct file* f;
				f = filp_open(fetching_page->path, O_RDONLY | O_LARGEFILE, 0);
				down_write(&mm->mmap_sem);

				if (!IS_ERR(f)) {
					//check if other threads already installed the vma
					vma = find_vma(mm, address);
					if (!vma || address >= vma->vm_end || address < vma->vm_start) {
						vma = NULL;
					}

					if (!vma || (vma->vm_start != fetching_page->vaddr_start)
					    || (vma->vm_end
						!= (fetching_page->vaddr_start + fetching_page->vaddr_size))) {

						PSPRINTK(
							"Mapping file vma start %lx end %lx\n", fetching_page->vaddr_start, (fetching_page->vaddr_start + fetching_page->vaddr_size));

						/*Note:
						 * This mapping is caused because when a thread migrates it does not have any vma
						 * so during fetch vma can be pushed.
						 * This mapping has the precedence over "normal" vma operations because is a page fault
						 * */
						current->mm->distribute_unmap = 0;

						PSPRINTK("%s:%d page offset = %d %lx\n", __func__, __LINE__, fetching_page->pgoff, mm->exe_file);
						fetching_page->pgoff = get_file_offset(mm->exe_file, fetching_page->vaddr_start);
						PSPRINTK("%s:%d page offset = %d\n", __func__, __LINE__, fetching_page->pgoff);

						/*map_difference should map in such a way that no unmap operations (the only nested operation that mmap can call) are nested called.
						 * This is important both to not unmap pages that should not be unmapped
						 * but also because otherwise the vma protocol will deadlock!
						 */
						err =
							map_difference(f, fetching_page->vaddr_start,
								       fetching_page->vaddr_size, prot,
								       MAP_FIXED
								       | ((fetching_page->vm_flags
									   & VM_DENYWRITE) ?
									  MAP_DENYWRITE : 0)
								       /* Ported to Linux 3.12 
									  | ((fetching_page->vm_flags
									  & VM_EXECUTABLE) ?
									  MAP_EXECUTABLE : 0) */
								       | ((fetching_page->vm_flags
									   & VM_SHARED) ?
									  MAP_SHARED : MAP_PRIVATE)
								       | ((fetching_page->vm_flags
									   & VM_HUGETLB) ?
									  MAP_HUGETLB : 0),
								       fetching_page->pgoff << PAGE_SHIFT);

						current->mm->distribute_unmap = 1;

						PSPRINTK("Map difference ended\n");
						if (err != fetching_page->vaddr_start) {
							up_write(&mm->mmap_sem);
							down_read(&mm->mmap_sem);
							spin_lock(ptl);
							/*PTE LOCKED*/
							printk(
								"%s: ERROR: error mapping file vma while fetching address %lx\n",
								__func__, address);
							ret = VM_FAULT_VMA;
							return ret;
						}
					}
				}
				else {
					up_write(&mm->mmap_sem);
					down_read(&mm->mmap_sem);
					spin_lock(ptl);
					/*PTE LOCKED*/
					printk("%s: ERROR: error while opening file %s\n",
					       __func__, fetching_page->path);
					ret = VM_FAULT_VMA;
					return ret;
				}

				up_write(&mm->mmap_sem);
				PSPRINTK("releasing lock write\n");
				filp_close(f, NULL);

				down_read(&mm->mmap_sem);
				PSPRINTK("lock read taken\n");
				spin_lock(ptl);
				/*PTE LOCKED*/
			}
		}
		return 0;
	}
	return 0;
}

/* Fetch a page from the system => ask other kernels if they have a copy of the page at address "address" on the
 * virtual mapping of the process identified by "tgroup_home_cpu" and "tgroup_home_id".
 *
 * down_read(&mm->mmap_sem) must be held.
 * pte lock must be held.
 *
 *return types:
 *VM_FAULT_OOM, problem allocating memory.
 *VM_FAULT_VMA, error vma management.
 *VM_FAULT_REPLICATION_PROTOCOL, general error.
 *VM_CONTINUE_WITH_CHECK, fetch the page locally.
 *0, remotely fetched;
 *-1, invalidated while fetching;
 * */
int do_remote_fetch_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
				  struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
				  unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, pte_t value_pte,
				  spinlock_t* ptl)
{

	mapping_answers_for_2_kernels_t* fetching_page;
	data_request_for_2_kernels_t* fetch_message;
	int ret= 0,i,reachable,other_cpu=-1;

	PSPRINTK("%s: Fetch for address %lx write %i pid %d is local?%d\n", __func__, address,((page_fault_flags & FAULT_FLAG_WRITE)?1:0),current->pid,pte_none(value_pte));
#if STATISTICS
	fetch++;
#endif

	fetching_page = (mapping_answers_for_2_kernels_t*) kmalloc(sizeof(mapping_answers_for_2_kernels_t),
								   GFP_ATOMIC);
	if (fetching_page == NULL) {
		ret = VM_FAULT_OOM;
		goto exit;
	}

	fetching_page->tgroup_home_cpu = tgroup_home_cpu;
	fetching_page->tgroup_home_id = tgroup_home_id;
	fetching_page->address = address;

	fetching_page->vma_present = 0;
	fetching_page->vaddr_start = 0;
	fetching_page->vaddr_size = 0;
	fetching_page->pgoff = 0;
	memset(fetching_page->path,0,sizeof(char)*512);
	memset(&(fetching_page->prot),0,sizeof(pgprot_t));
	fetching_page->vm_flags = 0;

	if(page_fault_flags & FAULT_FLAG_WRITE)
		fetching_page->is_write= 1;
	else
		fetching_page->is_write= 0;

	fetching_page->is_fetch= 1;
	fetching_page->owner= 0;
	fetching_page->address_present= 0;
	fetching_page->last_write= 0;
	fetching_page->data= NULL;
	fetching_page->futex_owner = -1;//akshay

	fetching_page->waiting = current;

	add_mapping_entry(fetching_page);

	if (_cpu==tgroup_home_cpu) {
		if (pte_none(value_pte)) {
			//not marked pte

#if STATISTICS
			local_fetch++;
#endif
			PSPRINTK("Copy not present in the other kernel, local fetch %d of address %lx\n", local_fetch, address);
			ret = VM_CONTINUE_WITH_CHECK;
			goto exit;
		}
	}

	fetch_message = (data_request_for_2_kernels_t*) kmalloc(sizeof(data_request_for_2_kernels_t),
								GFP_ATOMIC);
	if (fetch_message == NULL) {
		ret = VM_FAULT_OOM;
		goto exit_fetching_page;
	}

	fetch_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
	fetch_message->header.prio = PCN_KMSG_PRIO_NORMAL;
	fetch_message->address = address;
	fetch_message->tgroup_home_cpu = tgroup_home_cpu;
	fetch_message->tgroup_home_id = tgroup_home_id;
	fetch_message->is_write = fetching_page->is_write;
	fetch_message->is_fetch= 1;
	fetch_message->vma_operation_index= current->mm->vma_operation_index;

	PSPRINTK("%s vma_operation_index %d\n", __func__, fetch_message->vma_operation_index);
	PSPRINTK("%s: Fetch %i address %lx\n", __func__, 0, address);

	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	/*PTE UNLOCKED*/

	fetching_page->arrived_response= 0;
	reachable= 0;

	memory_t* memory= find_memory_entry(current->tgroup_home_cpu,
					    current->tgroup_home_id);
	if (!memory) {
		printk(KERN_ERR"%s: ERROR cannot find memory_t mapping (cpu %d id %d)\n",
				__func__, current->tgroup_home_cpu, current->tgroup_home_id);
		BUG();
		goto exit_fetch_message;
	}

	down_read(&memory->kernel_set_sem);
	// the list does not include the current processor group descirptor (TODO)
	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;

		if ((ret=pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (fetch_message),sizeof(data_request_for_2_kernels_t)-sizeof(struct pcn_kmsg_hdr)))
		    != -1) {
			// Message delivered
			reachable++;
			other_cpu= i;
			if(reachable>1)
				printk("%s: ERROR: using optimized algorithm for 2 kernels with more than two kernels\n",
						__func__);
		}
	}
	up_read(&memory->kernel_set_sem);

	if (reachable>0) {
		long counter = 0;
		while (fetching_page->arrived_response==0) {
			//set_task_state(current, TASK_UNINTERRUPTIBLE);
			if (fetching_page->arrived_response==0) {
				schedule();
			}
			//set_task_state(current, TASK_RUNNING);
			if (!(++counter % 1000000))
				printk("%s: WARN: fetching_page->arrived_response 0 [%ld] (cpu %d id %d address 0x%lx)\n",
						__func__, counter, tgroup_home_cpu, tgroup_home_id, address);
		}
	}

	down_read(&mm->mmap_sem);
	spin_lock(ptl);
	/*PTE LOCKED*/

	PSPRINTK("Out wait fetch %i address %lx\n", fetch, address);
	//only the client has to update the vma
	if (tgroup_home_cpu!=_cpu) {
		ret = do_mapping_for_distributed_process(fetching_page, mm, address, ptl);
		if (ret != 0)
			goto exit_fetch_message;
		PSPRINTK("Mapping end\n");

		vma = find_vma(mm, address);
		if (!vma || address >= vma->vm_end || address < vma->vm_start) {
			vma = NULL;
		} else if (unlikely(is_vm_hugetlb_page(vma))
			   || unlikely(transparent_hugepage_enabled(vma))) {
			printk("%s: ERROR: Installed a vma with HUGEPAGE\n",
					__func__);
			ret = VM_FAULT_VMA;
			goto exit_fetch_message;
		}

		if (vma == NULL) {
			//PSPRINTK
			dump_stack();
			printk(KERN_ALERT"%s: ERROR: no vma for address %lx in the system {%d} (cpu %d id %d)\n",
					__func__, address,current->pid, tgroup_home_cpu, tgroup_home_id);
			ret = VM_FAULT_VMA;
			goto exit_fetch_message;
		}
	}

	if (_cpu==tgroup_home_cpu && fetching_page->address_present == 0) {
		printk("%s: ERROR: No response for a marked page\n", __func__);
		ret = VM_FAULT_REPLICATION_PROTOCOL;
		goto exit_fetch_message;
	}
	if (fetching_page->address_present == 1) {
		struct page* page;
		spin_unlock(ptl);
		/*PTE UNLOCKED*/

		if (unlikely(anon_vma_prepare(vma))) {
			spin_lock(ptl);
			/*PTE LOCKED*/
			ret = VM_FAULT_OOM;
			goto exit_fetch_message;
		}

		page = alloc_page_vma(GFP_HIGHUSER_MOVABLE, vma, address);
		if (!page) {
			spin_lock(ptl);
			/*PTE LOCKED*/
			ret = VM_FAULT_OOM;
			goto exit_fetch_message;
		}

		__SetPageUptodate(page);
		if (mem_cgroup_newpage_charge(page, mm, GFP_ATOMIC)) {
			page_cache_release(page);
			spin_lock(ptl);
			/*PTE LOCKED*/
			ret = VM_FAULT_OOM;
			goto exit_fetch_message;
		}

#if STATISTICS
		pages_allocated++;
#endif
		spin_lock(ptl);
		/*PTE LOCKED*/

		int status;
		void *vto;
		void *vfrom;

		//if nobody changed the pte
		if (likely(pte_same(*pte, value_pte))) {
			if(fetching_page->is_write) { //if I am doing a write
				status= REPLICATION_STATUS_WRITTEN;
				if (fetching_page->owner==0) {
					printk("%s: ERROR: copy of a page sent to a write fetch request without ownership (cpu %d id %d)\n",
							__func__, tgroup_home_cpu, tgroup_home_id);
					pcn_kmsg_free_msg(fetching_page->data);
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_fetch_message;
				}
			}
			else {
				status= REPLICATION_STATUS_VALID;
				if (fetching_page->owner==1) {
					printk("%s: ERROR: copy of a page sent to a read fetch request with ownership (cpu %d id %d)\n",
							__func__, tgroup_home_cpu, tgroup_home_id);
					pcn_kmsg_free_msg(fetching_page->data);
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_fetch_message;
				}
			}
			if (fetching_page->data->address != address) {
				printk("%s: ERROR: trying to copy wrong address 0x%lx\n (cpu %d id %d)\n",
						__func__, address, tgroup_home_cpu, tgroup_home_id);
				pcn_kmsg_free_msg(fetching_page->data);
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_fetch_message;
			}

			// Ported to Linux 3.12 
			//vto = kmap_atomic(page, KM_USER0);
			vto = kmap_atomic(page);
			vfrom = &(fetching_page->data->data);
			copy_user_page(vto, vfrom, address, page);
			// Ported to Linux 3.12 
			//kunmap_atomic(vto, KM_USER0);
			kunmap_atomic(vto);
#if READ_PAGE
			int ct=0;
			if(address == PAGE_ADDR) {
				for(ct=0;ct<8;ct++){
					printk(KERN_ALERT"{%lx} ",(unsigned long) *(((unsigned long *)vfrom)+ct));
				}
			}
#endif

			pcn_kmsg_free_msg(fetching_page->data);
			pte_t entry = mk_pte(page, vma->vm_page_prot);

			//if the page is read only no need to keep replicas coherent
			if (vma->vm_flags & VM_WRITE) {
				page->replicated = 1;

				if (fetching_page->is_write) {
					page->last_write = fetching_page->last_write + 1;
				}
				else {
					page->last_write = fetching_page->last_write;
				}

#if STATISTICS
				if(page->last_write> most_written_page)
					most_written_page= page->last_write;
#endif
				page->owner = fetching_page->owner;
				page->status = status;

				if (status == REPLICATION_STATUS_VALID) {
					entry =  pte_wrprotect(entry);
				} else {
					entry =  pte_mkwrite(entry);
#if defined(CONFIG_ARM64)
					entry =  pte_mkdirty(entry);
#endif
				}
			}
			else {
				if(fetching_page->is_write)
					printk("%s: ERROR: trying to write a read only page\n", __func__);

				if(fetching_page->owner==1)
					printk("%s: ERROR: received ownership with a copy of a read only page\n", __func__);

				page->replicated = 0;
				page->owner= 0;
				page->status= REPLICATION_STATUS_NOT_REPLICATED;
			}

			entry = pte_set_valid_entry_flag(entry);
			page->other_owners[_cpu]=1;
			page->other_owners[other_cpu]=1;
			page->futex_owner = fetching_page->futex_owner;//akshay

			flush_cache_page(vma, address, pte_pfn(*pte));
#if defined(CONFIG_ARM64)
			entry = pte_set_user_access_flag(entry);
			entry = pte_mkyoung(entry);
#else
			entry = pte_set_flags(entry, _PAGE_USER);
			entry = pte_set_flags(entry, _PAGE_ACCESSED);
#endif
			ptep_clear_flush(vma, address, pte);

			page_add_new_anon_rmap(page, vma, address);
			set_pte_at_notify(mm, address, pte, entry);

			update_mmu_cache(vma, address, pte);

			flush_tlb_page(vma, address);
#if defined(CONFIG_ARM64)
#else
			flush_tlb_fix_spurious_fault(vma, address);
#endif
		}
		else {
			PSPRINTK("pte changed while fetching\n");
			status = REPLICATION_STATUS_INVALID;
			mem_cgroup_uncharge_page(page);
			page_cache_release(page);
			pcn_kmsg_free_msg(fetching_page->data);
		}

		PSPRINTK("End fetching address %lx\n", address);
		ret= 0;
		goto exit_fetch_message;
	}
	else { /* copy not present on the other kernel */
#if STATISTICS
		local_fetch++;
#endif
		PSPRINTK("Copy not present in the other kernel, local fetch %d of address %lx\n", local_fetch, address);
		PSMINPRINTK("Local fetch for address %lx\n",address);
		kfree(fetch_message);
		ret = VM_CONTINUE_WITH_CHECK;
		goto exit;
	}

exit_fetch_message:
	kfree(fetch_message);
exit_fetching_page:
	remove_mapping_entry(fetching_page);
	kfree(fetching_page);
exit:
	return ret;
}

extern int access_error(unsigned long error_code, struct vm_area_struct *vma);

/**
 * down_read(&mm->mmap_sem) already held
 *
 * return types:
 * VM_FAULT_OOM, problem allocating memory.
 * VM_FAULT_VMA, error vma management.
 * VM_FAULT_ACCESS_ERROR, access error;
 * VM_FAULT_REPLICATION_PROTOCOL, replication protocol error.
 * VM_CONTINUE_WITH_CHECK, fetch the page locally.
 * VM_CONTINUE, normal page_fault;
 * 0, remotely fetched;
 */
int process_server_try_handle_mm_fault(struct task_struct *tsk,
				       struct mm_struct *mm,
				       struct vm_area_struct *vma,
				       unsigned long page_faul_address,
				       unsigned long page_fault_flags,
				       unsigned long error_code)
{
	pgd_t* pgd;
	pud_t* pud;
	pmd_t* pmd;
	pte_t* pte;
	pte_t value_pte;
	spinlock_t *ptl;

	struct page* page;

	unsigned long address;

	int tgroup_home_cpu = tsk->tgroup_home_cpu;
	int tgroup_home_id = tsk->tgroup_home_id;
	int ret;

	address = page_faul_address & PAGE_MASK;

#if STATISTICS
	page_fault_mio++;
#endif
	PSPRINTK("%s: page fault for address %lx in page %lx task pid %d t_group_cpu %d t_group_id %d %s\n",
                 __func__, page_faul_address, address, tsk->pid,
                 tgroup_home_cpu, tgroup_home_id,
                 page_fault_flags & FAULT_FLAG_WRITE ? "WRITE" : "READ");

	if (address == 0) {
		printk("%s: ERROR: accessing page at address 0 pid %i %d %d\n",
				__func__, tsk->pid, tsk->tgroup_home_cpu, tsk->tgroup_home_id);

		dump_processor_regs(task_pt_regs(tsk));
		return VM_FAULT_ACCESS_ERROR | VM_FAULT_VMA;
	}

	if (vma && (address < vma->vm_end && address >= vma->vm_start)
	    && (unlikely(is_vm_hugetlb_page(vma))
		|| transparent_hugepage_enabled(vma))) {
		printk("%s: ERROR: page fault for huge page %d %d\n",
				__func__, tsk->tgroup_home_cpu, tsk->tgroup_home_id);
		return VM_CONTINUE;
	}

	pgd = pgd_offset(mm, address);
	pud = pud_alloc(mm, pgd, address);
	if (!pud)
		return VM_FAULT_OOM;
	pmd = pmd_alloc(mm, pud, address);
	if (!pmd)
		return VM_FAULT_OOM;

	if (pmd_none(*pmd) && __pte_alloc(mm, vma, pmd, address))
		return VM_FAULT_OOM;

	if (unlikely(pmd_trans_huge(*pmd))) {
		printk("%s: ERROR: page fault for huge page (cpu %d id %d)\n",
				__func__, tsk->tgroup_home_cpu, tsk->tgroup_home_id);
		return VM_CONTINUE;
	}

	pte = pte_offset_map_lock(mm, pmd, address, &ptl);
	/*PTE LOCKED*/

	value_pte = *pte;

	// printk("%s: Page fault %i address %lx in page %lx task pid %d t_group_cpu %d t_group_id %d \n", __func__, 0, page_faul_address, address, tsk->pid, tgroup_home_cpu, tgroup_home_id);

	/*case pte UNMAPPED
	 * --Remote fetch--
	 */
start: 
#if defined(CONFIG_ARM64)
	//Ajith - Removing optimization used for local fetch - _PAGE_UNUSED1 case
	if (pte == NULL || pte_none(value_pte)) {
#else
	if (pte == NULL || pte_none(pte_clear_flags(value_pte, _PAGE_UNUSED1))) {
#endif
		if(pte==NULL)
			printk("%s: WARN: pte NULL\n", __func__);

		// printk("%s: 1st if pid %d\n", __func__, tsk->pid);

		/* Check if other threads of my process are already
		 * fetching the same address on this kernel.
		 */
		if (find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address) != NULL) {
			//wait while the fetch is ended
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);


			while (find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address)
					!= NULL) {

				//printk("%s: 1st if while %d\n", __func__, tsk->pid);

				DEFINE_WAIT(wait);
				prepare_to_wait(&read_write_wait, &wait, TASK_UNINTERRUPTIBLE);

				if (find_mapping_entry(tgroup_home_cpu, tgroup_home_id,
							address)!=NULL) {
					//printk("%s before schedule pid %d\n", __func__, tsk->pid);
					schedule();
					//printk("%s after schedule pid %d\n", __func__, tsk->pid);
				}

				finish_wait(&read_write_wait, &wait);
			}

			down_read(&mm->mmap_sem);
			spin_lock(ptl);
			value_pte = *pte;

			vma = find_vma(mm, address);
			if (unlikely(
				    !vma || address >= vma->vm_end
				    || address < vma->vm_start)) {

				printk("%s: ERROR: vma not valid after waiting for another thread to fetch (cpu %d id %d)\n",
						__func__, tsk->tgroup_home_cpu, tsk->tgroup_home_id);
				spin_unlock(ptl);
				return VM_FAULT_VMA;
			}

			goto start;
		}
		if (!vma || address >= vma->vm_end || address < vma->vm_start) {
			vma = NULL;
		}
		ret = do_remote_fetch_for_2_kernels(tsk->tgroup_home_cpu, tsk->tgroup_home_id, mm,
						    vma, address, page_fault_flags, pmd, pte, value_pte, ptl);

		spin_unlock(ptl);
		wake_up(&read_write_wait);

		return ret;
	}
	else { /* case pte MAPPED */

		/* There can be an unluckily case in which I am still fetching... is this handled?
		 */
		mapping_answers_for_2_kernels_t* fetch= find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address);
		if (fetch != NULL && fetch->is_fetch==1) {
			//wait while the fetch is ended
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);

			fetch= find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address);
			while (fetch != NULL && fetch->is_fetch==1) {

				DEFINE_WAIT(wait);
				prepare_to_wait(&read_write_wait, &wait, TASK_UNINTERRUPTIBLE);

				fetch= find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address);
				if (fetch != NULL && fetch->is_fetch==1) {
					schedule();
				}

				finish_wait(&read_write_wait, &wait);

				fetch= find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address);
			}

			down_read(&mm->mmap_sem);
			spin_lock(ptl);
			value_pte = *pte;

			vma = find_vma(mm, address);
			if (unlikely(
				    !vma || address >= vma->vm_end
				    || address < vma->vm_start)) {

				printk(
					"%s: ERROR: vma not valid after waiting for another thread to fetch (cpu %d id %d)\n",
					__func__, tsk->tgroup_home_cpu, tsk->tgroup_home_id);
				spin_unlock(ptl);
				return VM_FAULT_VMA;
			}
			goto start;
		}

		/* The pte is mapped so the vma should be valid.
		 * Check if the access is within the limit.
		 */
		if (unlikely(
			    !vma || address >= vma->vm_end || address < vma->vm_start)) {
			printk("%s: ERROR: no vma for address %lx in the system\n",
					__func__, address);
			spin_unlock(ptl);
			return VM_FAULT_VMA;
		}

		/*
		 * Check if the permission are ok.
		 */
		if (unlikely(access_error(error_code, vma))) {
			spin_unlock(ptl);
			return VM_FAULT_ACCESS_ERROR;
		}

		page = pte_page(value_pte);
		if (page != vm_normal_page(vma, address, value_pte)) {
			PSPRINTK("page different from vm_normal_page\n");
		}
		PSPRINTK("%s page status %d\n", __func__, page->status);
		/* case page NOT REPLICATED
		 */
		if (page->replicated == 0) {
			PSPRINTK("Page not replicated address %lx\n", address);

			//check if it a cow page...
			if ((vma->vm_flags & VM_WRITE) && !pte_write(value_pte)) {

			retry_cow:
				PSPRINTK("COW page at %lx\n", address);

				int cow_ret= do_wp_page_for_popcorn(mm, vma,address, pte,pmd,ptl, value_pte);

				if (cow_ret & VM_FAULT_ERROR) {
					if (cow_ret & VM_FAULT_OOM){
						printk("%s: ERROR: VM_FAULT_OOM\n",__func__);
						return VM_FAULT_OOM;
					}
					if (cow_ret & (VM_FAULT_HWPOISON | VM_FAULT_HWPOISON_LARGE)){
						printk("%s: ERROR: EHWPOISON\n",__func__);
						return VM_FAULT_OOM;
					}
					if (cow_ret & VM_FAULT_SIGBUS){
						printk("%s: ERROR: EFAULT\n",__func__);
						return VM_FAULT_OOM;
					}
					printk("%s: ERROR: bug from do_wp_page_for_popcorn\n",__func__);
					return VM_FAULT_OOM;
				}

				spin_lock(ptl);
				/*PTE LOCKED*/

				value_pte = *pte;

				if(!pte_write(value_pte)){
					printk("%s: WARN: page not writable after cow (cpu %d id %d page 0x%lx)\n",
							__func__, tsk->tgroup_home_cpu, tsk->tgroup_home_id, (unsigned long)page);
					goto retry_cow;
				}

				page = pte_page(value_pte);

				page->replicated = 0;
				page->status= REPLICATION_STATUS_NOT_REPLICATED;
				page->owner= 1;
				page->other_owners[_cpu] = 1;
			}

			spin_unlock(ptl);
			return 0;
		}

	check:
		/* case REPLICATION_STATUS_VALID:
		 * the data of the page is up to date.
		 * reads can be performed locally.
		 * to write is needed to send an invalidation message to all the other copies.
		 * a write in REPLICATION_STATUS_VALID changes the status to REPLICATION_STATUS_WRITTEN
		 */
		if (page->status == REPLICATION_STATUS_VALID) {
			PSPRINTK("%s: Page status valid address %lx\n", __func__, address);

			/*read case
			 */
			if (!(page_fault_flags & FAULT_FLAG_WRITE)) {
				spin_unlock(ptl);

				return 0;
			}
			else { /* write case */

				/* If other threads of this process are writing or reading in this kernel, I wait.
				 *
				 * I wait for concurrent writes because after a write the status is updated to REPLICATION_STATUS_WRITTEN,
				 * so only the first write needs to send the invalidation messages.
				 *
				 * I wait for reads because if the invalidation message of the write is handled before the request of the read
				 * there could be the possibility that nobody answers to the read whit a copy.
				 *
				 */
				if (page->writing == 1 || page->reading == 1) {
					spin_unlock(ptl);
					up_read(&mm->mmap_sem);

					while (page->writing == 1 || page->reading == 1) {
						DEFINE_WAIT(wait);

						prepare_to_wait(&read_write_wait, &wait,
								TASK_UNINTERRUPTIBLE);
						if (page->writing == 1 || page->reading == 1)
							schedule();
						finish_wait(&read_write_wait, &wait);
					}
					down_read(&mm->mmap_sem);
					spin_lock(ptl);
					value_pte = *pte;

					vma = find_vma(mm, address);
					if (unlikely(
						    !vma || address >= vma->vm_end
						    || address < vma->vm_start)) {

						printk("%s: ERROR: vma not valid after waiting for another thread to fetch\n",
								__func__);
						spin_unlock(ptl);
						return VM_FAULT_VMA;
					}

					goto check;
				}

				ret = do_remote_write_for_2_kernels(tgroup_home_cpu, tgroup_home_id, mm, vma,
								    address, page_fault_flags, pmd, pte, ptl, page,0);

				spin_unlock(ptl);
				wake_up(&read_write_wait);

				return ret;
			}
		}
		else {
			/* case REPLICATION_STATUS_WRITTEN
			 * both read and write can be performed on this page.
			 * */
			if (page->status == REPLICATION_STATUS_WRITTEN) {
				printk("%s: Page status written address %lx\n", __func__, address);
				spin_unlock(ptl);
				return 0;
			}
			else {

				if (!(page->status == REPLICATION_STATUS_INVALID)) {
					printk("%s: ERROR: Page status not correct on address %lx\n",
					       __func__, address);
					spin_unlock(ptl);
					return VM_FAULT_REPLICATION_PROTOCOL;
				}
				PSPRINTK("%s: Page status invalid address %lx\n", __func__, address);

				/*If other threads are already reading or writing it wait,
				 * they will eventually read a valid copy
				 */
				if (page->writing == 1 || page->reading == 1) {
					spin_unlock(ptl);
					up_read(&mm->mmap_sem);

					while (page->writing == 1 || page->reading == 1) {
						DEFINE_WAIT(wait);
						prepare_to_wait(&read_write_wait, &wait,
								TASK_UNINTERRUPTIBLE);
						if (page->writing == 1 || page->reading == 1)
							schedule();
						finish_wait(&read_write_wait, &wait);
					}

					down_read(&mm->mmap_sem);
					spin_lock(ptl);
					value_pte = *pte;

					vma = find_vma(mm, address);
					if (unlikely(
						    !vma || address >= vma->vm_end
						    || address < vma->vm_start)) {

						printk(
							"%s: ERROR: vma not valid after waiting for another thread to fetch\n", __func__);
						spin_unlock(ptl);
						return VM_FAULT_VMA;
					}

					goto check;
				}
				if (page_fault_flags & FAULT_FLAG_WRITE)
					ret = do_remote_write_for_2_kernels(tgroup_home_cpu, tgroup_home_id, mm, vma,
									    address, page_fault_flags, pmd, pte, ptl, page,1);
				else
					ret = do_remote_read_for_2_kernels(tgroup_home_cpu, tgroup_home_id, mm, vma,
									   address, page_fault_flags, pmd, pte, ptl, page);
				spin_unlock(ptl);
				wake_up(&read_write_wait);

				return ret;
			}
		}
	}
}

int process_server_dup_task(struct task_struct* orig, struct task_struct* task)
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
	task->group_exit= -1;
	task->uaddr = 0;
	task->origin_pid = -1;
	// If the new task is not in the same thread group as the parent,
	// then we do not need to propagate the old thread info.
	if (orig->tgid != task->tgid) {
		return 1;
	}

	lock_task_sighand(orig, &flags);
	// This is important.  We want to make sure to keep an accurate record
	// of which cpu and thread group the new thread is a part of.
	if (orig->tgroup_distributed == 1) {
		task->tgroup_home_cpu = orig->tgroup_home_cpu;
		task->tgroup_home_id = orig->tgroup_home_id;
		task->tgroup_distributed = 1;
		printk("%s: INFO: dup task (cpu %d id %d)\n", __func__, task->tgroup_home_cpu, task->tgroup_home_id);
	}
	unlock_task_sighand(orig, &flags);

	return 1;
}
/*
 * Send a message to <dst_cpu> for migrating back a task <task>.
 * This is a back migration => <task> must already been migrated at least once in <dst_cpu>.
 * It returns -1 in error case.
 */
static int do_back_migration(struct task_struct *task, int dst_cpu,
			     struct pt_regs *regs, void __user *uregs)
{
	unsigned long flags;
	int ret;
	back_migration_request_t* request;

	request= (back_migration_request_t*) kmalloc(sizeof(back_migration_request_t), GFP_ATOMIC);
	if(request==NULL)
		return -1;

	PSPRINTK("%s\n", __func__);

	//printk("%s entered dst{%d}\n",__func__,dst_cpu);
	request->header.type = PCN_KMSG_TYPE_PROC_SRV_BACK_MIG_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
	request->tgroup_home_cpu = task->tgroup_home_cpu;
	request->tgroup_home_id = task->tgroup_home_id;
	request->placeholder_pid = task->pid;
	request->placeholder_tgid = task->tgid;
	request->back=1;
	request->prev_pid= task->prev_pid;
	request->personality = task->personality;

	/*mklinux_akshay*/
	request->origin_pid = task->origin_pid;
	request->remote_blocked = task->blocked;
	request->remote_real_blocked = task->real_blocked;
	request->remote_saved_sigmask = task->saved_sigmask;
	request->remote_pending = task->pending;
	request->sas_ss_sp = task->sas_ss_sp;
	request->sas_ss_size = task->sas_ss_size;
	int cnt = 0;
	for (cnt = 0; cnt < _NSIG; cnt++)
		request->action[cnt] = task->sighand->action[cnt];

	// TODO: Handle return value
	save_thread_info(task, regs, &request->arch, uregs);

#if MIGRATE_FPU
	// TODO: Handle return value
	save_fpu_info(task, &request->arch);
#endif

	lock_task_sighand(task, &flags);

	if (task->tgroup_distributed == 0) {
		unlock_task_sighand(task, &flags);
		printk("%s: ERROR: back migrating thread of not tgroup_distributed process (cpu %d id %d)\n",
				__func__, task->tgroup_home_cpu, task->tgroup_home_id);
		kfree(request);
		return -1;
	}

	task->represents_remote = 1;
	task->next_cpu = task->prev_cpu;
	task->next_pid = task->prev_pid;
	task->executing_for_remote= 0;
	unlock_task_sighand(task, &flags);

#if MIGRATION_PROFILE
	migration_end = ktime_get();
	printk("value of migration_start = %ld\n", (unsigned long)ktime_to_ns(migration_start));
        printk(KERN_ERR "Time for x86->arm back migration - x86 side: %ld ns\n", (unsigned long)ktime_to_ns(ktime_sub(migration_end,migration_start)));
#endif

	ret = pcn_kmsg_send_long(dst_cpu,
				 (struct pcn_kmsg_long_message *)request,
				 sizeof(clone_request_t) - sizeof(struct pcn_kmsg_hdr));

	kfree(request);
	return ret;
}

/*
 * Send a message to <dst_cpu> for migrating a task <task>.
 * This function will ask the remote cpu to create a thread to host task.
 * It returns -1 in error case.
 */
//static
int do_migration(struct task_struct* task, int dst_cpu,
		 struct pt_regs * regs, int* is_first,
                 void __user *uregs)
{
	struct task_struct *kthread_main = NULL;
	clone_request_t* request;
	int tx_ret = -1;
	struct task_struct* tgroup_iterator = NULL;
	char path[256] = { 0 };
	char* rpath;
	memory_t* entry;
	int first= 0;
	unsigned long flags;

	request = kmalloc(sizeof(clone_request_t), GFP_ATOMIC);
	if (request == NULL) {
		return -1;
	}
	*is_first=0;
	PSPRINTK("%s: PID %d\n", __func__, task->pid);
	// Build request
	request->header.type = PCN_KMSG_TYPE_PROC_SRV_CLONE_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;

	// struct mm_struct -----------------------------------------------------------
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
	request->popcorn_vdso = (unsigned long)task->mm->context.popcorn_vdso;
	// struct task_struct ---------------------------------------------------------
	request->placeholder_pid = task->pid;
	request->placeholder_tgid = task->tgid;
	/*mklinux_akshay*/
	if (task->prev_pid == -1)
		request->origin_pid = task->pid;
	else
		request->origin_pid = task->origin_pid;
	request->remote_blocked = task->blocked;
	request->remote_real_blocked = task->real_blocked;
	request->remote_saved_sigmask = task->saved_sigmask;
	request->remote_pending = task->pending;
	request->sas_ss_sp = task->sas_ss_sp;
	request->sas_ss_size = task->sas_ss_size;
	int cnt = 0;
	for (cnt = 0; cnt < _NSIG; cnt++)
		request->action[cnt] = task->sighand->action[cnt];

	request->back=0;

	/*mklinux_akshay*/
	if(task->prev_pid==-1)
		task->origin_pid=task->pid;
	else
		task->origin_pid=task->origin_pid;

	request->personality = task->personality;

	// TODO: Handle return value
	save_thread_info(task, regs, &request->arch, uregs);

#if MIGRATE_FPU
	save_fpu_info(task, &request->arch);
#endif

	/*I use siglock to coordinate the thread group.
	 *This process is becoming a distributed one if it was not already.
	 *The first migrating thread has to create the memory entry to handle page requests,
	 *and fork the main kernel thread of this process.
	 */
	lock_task_sighand(task, &flags);

	if (task->tgroup_distributed == 0) { // convert the task to distributed
		task->tgroup_distributed = 1;
		task->tgroup_home_id = task->tgid;
		task->tgroup_home_cpu = _cpu;
		printk("%s: INFO: migrate task (cpu %d id %d)\n", __func__, task->tgroup_home_cpu, task->tgroup_home_id);

		entry = (memory_t*) kmalloc(sizeof(memory_t), GFP_ATOMIC);
		if (!entry){
			unlock_task_sighand(task, &flags);
			printk("%s: ERROR: Impossible allocate memory_t while migrating thread (cpu %d id %d)\n",
					__func__, task->tgroup_home_cpu, task->tgroup_home_id);
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
		atomic_set(&(entry->pending_migration),0);
		atomic_set(&(entry->pending_back_migration),0);
		entry->operation = VMA_OP_NOP;
		entry->waiting_for_main = NULL;
		entry->waiting_for_op = NULL;
		entry->arrived_op = 0;
		entry->my_lock = 0;
		memset(entry->kernel_set,0,MAX_KERNEL_IDS*sizeof(int));
		entry->kernel_set[_cpu]= 1;
		init_rwsem(&entry->kernel_set_sem);
		entry->setting_up= 0;
		init_rwsem(&task->mm->distribute_sem);
		task->mm->distr_vma_op_counter = 0;
		task->mm->was_not_pushed = 0;
		task->mm->thread_op = NULL;
		task->mm->vma_operation_index = 0;
		task->mm->distribute_unmap = 1;
		add_memory_entry(entry);

		PSPRINTK("memory entry added cpu %d id %d\n", entry->tgroup_home_cpu, entry->tgroup_home_id);
		first=1;
		*is_first = 1;

		tgroup_iterator = task;
		while_each_thread(task, tgroup_iterator)
		{
			tgroup_iterator->tgroup_home_id = task->tgroup_home_id;
			tgroup_iterator->tgroup_home_cpu = task->tgroup_home_cpu;
			tgroup_iterator->tgroup_distributed = 1;
		};
	}

	task->represents_remote = 1;

	unlock_task_sighand(task, &flags);

	request->tgroup_home_cpu = task->tgroup_home_cpu;
	request->tgroup_home_id = task->tgroup_home_id;

#if MIGRATION_PROFILE
	migration_end = ktime_get();
        printk(KERN_ERR "Time for x86->arm migration - x86 side: %ld ns\n", (unsigned long)ktime_to_ns(ktime_sub(migration_end,migration_start)));
#endif
	tx_ret = pcn_kmsg_send_long(dst_cpu,
				    (struct pcn_kmsg_long_message*) request,
				    sizeof(clone_request_t) - sizeof(struct pcn_kmsg_hdr));

	if (first) {
		PSPRINTK(KERN_EMERG"%s: Creating kernel thread\n", __func__);
		// Sharath: In Linux 3.12 this API does not allow a kernel thread to be 
		//          created on a user context
		/*return_value = kernel_thread(create_kernel_thread_for_distributed_process_from_user_one,
		  entry, CLONE_THREAD | CLONE_SIGHAND | CLONE_VM | SIGCHLD);*/
		kthread_main = kernel_thread_popcorn(create_kernel_thread_for_distributed_process_from_user_one,
						     entry, CLONE_THREAD | CLONE_SIGHAND | CLONE_VM | SIGCHLD);
	}

	kfree(request);
	return tx_ret;
}

/**
 * Migrate the specified task <task> to cpu <dst_cpu>
 * Currently, this function will put the specified task to
 * sleep, and push its info over to the remote cpu.  The
 * remote cpu will then create a new thread and import that
 * info into its new context.
 *
 * It returns PROCESS_SERVER_CLONE_FAIL in case of error,
 * PROCESS_SERVER_CLONE_SUCCESS otherwise.
 */
int process_server_do_migration(struct task_struct *task, int dst_cpu,
				struct pt_regs *regs, void __user *uregs)
{
	int first = 0;
	int back = 0;
	int ret = 0;

	PSPRINTK("%s: migrating pid %d tgid %d task->tgroup_home_id %d task->tgroup_home_cpu %d\n",
			__func__,current->pid,current->tgid,task->tgroup_home_id,task->tgroup_home_cpu);

	if (task->prev_cpu == dst_cpu) {
		back = 1;
		ret = do_back_migration(task, dst_cpu, regs, uregs);
		if (ret == -1)
			return PROCESS_SERVER_CLONE_FAIL;
	} else {
		ret = do_migration(task, dst_cpu, regs,&first, uregs);
	}

	if (ret != -1) {
		//printk(KERN_ALERT"%s clone request sent ret{%d}\n", __func__,ret);
		__set_task_state(task, TASK_UNINTERRUPTIBLE);
		return PROCESS_SERVER_CLONE_SUCCESS;
	} else {
		return PROCESS_SERVER_CLONE_FAIL;
	}
}

void process_vma_op(struct work_struct* work)
{
	vma_op_work_t* vma_work = (vma_op_work_t*) work;
	vma_operation_t* operation = vma_work->operation;
	memory_t* memory = vma_work->memory;
	struct mm_struct* mm = NULL;

	//to coordinate with dead of process
	if (vma_work->fake == 1) {
		unsigned long flags;
		memory->arrived_op = 1;
		lock_task_sighand(memory->main, &flags);
		memory->main->distributed_exit = EXIT_FLUSHING;
		unlock_task_sighand(memory->main, &flags);
		wake_up_process(memory->main);
		kfree(work);
		return;
	}

	if (!memory) {
		printk(KERN_ERR"%s: ERROR: vma_work->memory is 0x%lx\n",
				__func__, (unsigned long)memory);
		return;
	}
	mm = memory->mm;
	if (!mm) {
		printk(KERN_ERR"%s: ERROR: vma_memory->mm is 0x%lx\n",
				__func__, (unsigned long)mm);
		return;
	}
	PSPRINTK("Received vma operation from cpu %d for tgroup_home_cpu %i tgroup_home_id %i operation %i\n",
			operation->header.from_cpu, operation->tgroup_home_cpu, operation->tgroup_home_id, operation->operation);

	down_write(&mm->mmap_sem);
	if (_cpu == operation->tgroup_home_cpu) {

		//SERVER

		//if another operation is on going, it will be serialized after.
		while (mm->distr_vma_op_counter > 0) {
			printk("%s, A distributed operation already started, going to sleep\n",__func__);
			up_write(&mm->mmap_sem);
			DEFINE_WAIT(wait);
			prepare_to_wait(&request_distributed_vma_op, &wait,
					TASK_UNINTERRUPTIBLE);
			if (mm->distr_vma_op_counter > 0) {
				schedule();
			}
			finish_wait(&request_distributed_vma_op, &wait);
			down_write(&mm->mmap_sem);
		}

		if (mm->distr_vma_op_counter != 0 || mm->was_not_pushed != 0) {
			up_write(&mm->mmap_sem);
			printk("ERROR: handling a new vma operation but mm->distr_vma_op_counter is %i and mm->was_not_pushed is %i\n",
			       mm->distr_vma_op_counter, mm->was_not_pushed);
			pcn_kmsg_free_msg(operation);
			kfree(work);
			return ;
		}

		PSPRINTK("%s,SERVER: Starting operation %i for cpu %i\n",__func__, operation->operation, operation->header.from_cpu);

		mm->distr_vma_op_counter++;
		//the main kernel thread will execute the local operation
		while(memory->main==NULL)
			schedule();

		mm->thread_op = memory->main;

		if (operation->operation == VMA_OP_MAP
		    || operation->operation == VMA_OP_BRK) {
			mm->was_not_pushed++;
		}

		up_write(&mm->mmap_sem);

		//wake up the main thread to execute the operation locally
		memory->message_push_operation = operation;
		memory->addr = operation->addr;
		memory->len = operation->len;
		memory->prot = operation->prot;
		memory->new_addr = operation->new_addr;
		memory->new_len = operation->new_len;
		memory->flags = operation->flags;
		memory->pgoff = operation->pgoff;
		strcpy(memory->path, operation->path);
		memory->waiting_for_main = current;
		//This is the field check by the main thread
		//so it is the last one to be populated
		memory->operation = operation->operation;
		wake_up_process(memory->main);

		PSPRINTK("%s,SERVER: woke up the main\n",__func__);
		while (memory->operation != VMA_OP_NOP) {
			set_task_state(current, TASK_UNINTERRUPTIBLE);
			if (memory->operation != VMA_OP_NOP) {
				schedule();
			}
			set_task_state(current, TASK_RUNNING);
		}
		PSPRINTK("%s,SERVER: woke up the main1\n",__func__);

		down_write(&mm->mmap_sem);
		PSPRINTK("%s,SERVER: woke up the main2\n",__func__);

		mm->distr_vma_op_counter--;
		if (mm->distr_vma_op_counter != 0)
			printk(	"%s: ERROR: exiting from distributed operation but mm->distr_vma_op_counter is %i\n",
				__func__, mm->distr_vma_op_counter);
		if (operation->operation == VMA_OP_MAP
		    || operation->operation == VMA_OP_BRK) {
			mm->was_not_pushed--;
			if (mm->was_not_pushed != 0)
				printk("%s: ERROR: exiting from distributed operation but mm->was_not_pushed is %i\n",
				       __func__, mm->distr_vma_op_counter);
		}

		mm->thread_op = NULL;

		PSPRINTK("%s,SERVER: woke up the main3\n",__func__);

		up_write(&mm->mmap_sem);

		PSPRINTK("%s,SERVER: woke up the main4\n",__func__);

		wake_up(&request_distributed_vma_op);

		pcn_kmsg_free_msg(operation);
		kfree(work);
		PSPRINTK("SERVER: vma_operation_index is %d\n",mm->vma_operation_index);
		PSPRINTK("%s, SERVER: end requested operation\n",__func__);

		return ;
	}
	else {
		PSPRINTK("%s, CLIENT: Starting operation %i of index %i\n ",__func__, operation->operation, operation->vma_operation_index);
		//CLIENT

		//NOTE: the current->mm->distribute_sem is already held

		//MMAP and BRK are not pushed in the system
		//if I receive one of them I must have initiate it
		if (operation->operation == VMA_OP_MAP
		    || operation->operation == VMA_OP_BRK) {

			if (memory->my_lock != 1) {
				printk("%s: ERROR: wrong distributed lock aquisition\n", __func__);
				up_write(&mm->mmap_sem);
				pcn_kmsg_free_msg(operation);
				kfree(work);
				return ;
			}

			if (operation->from_cpu != _cpu) {
				printk("%s: ERROR: the server pushed me an operation %i of cpu %i\n",
				       __func__, operation->operation, operation->from_cpu);
				up_write(&mm->mmap_sem);
				pcn_kmsg_free_msg(operation);
				kfree(work);
				return ;
			}

			if (memory->waiting_for_op == NULL) {
				printk(	"%s: ERROR:received a push operation started by me but nobody is waiting\n", __func__);
				up_write(&mm->mmap_sem);
				pcn_kmsg_free_msg(operation);
				kfree(work);
				return ;
			}

			memory->addr = operation->addr;
			memory->arrived_op = 1;
			//mm->vma_operation_index = operation->vma_operation_index;
			PSPRINTK("CLIENT: vma_operation_index is %d\n",mm->vma_operation_index);
			PSVMAPRINTK("%s, CLIENT: Operation %i started by a local thread pid %d\n ",__func__,operation->operation,memory->waiting_for_op->pid);
			up_write(&mm->mmap_sem);

			wake_up_process(memory->waiting_for_op);

			pcn_kmsg_free_msg(operation);
			kfree(work);
			return ;
		}

		//I could have started the operation...check!
		if (operation->from_cpu == _cpu) {
			if (memory->my_lock != 1) {
				printk("%s: ERROR: wrong distributed lock aquisition\n", __func__);
				up_write(&mm->mmap_sem);
				pcn_kmsg_free_msg(operation);
				kfree(work);
				return ;
			}

			if (memory->waiting_for_op == NULL) {
				printk(
					"%s: ERROR:received a push operation started by me but nobody is waiting\n", __func__);
				up_write(&mm->mmap_sem);
				pcn_kmsg_free_msg(operation);
				kfree(work);
				return ;
			}
			if (operation->operation == VMA_OP_REMAP)
				memory->addr = operation->new_addr;

			memory->arrived_op = 1;
			//mm->vma_operation_index = operation->vma_operation_index;
			PSPRINTK("%s, CLIENT: Operation %i started by a local thread pid %d\n ",__func__,operation->operation,memory->waiting_for_op->pid);
			PSPRINTK("CLIENT: vma_operation_index is %d\n",mm->vma_operation_index);
			up_write(&mm->mmap_sem);

			wake_up_process(memory->waiting_for_op);
			pcn_kmsg_free_msg(operation);
			kfree(work);
			return ;
		}

		PSPRINTK("%s, CLIENT Pushed operation started by somebody else\n",__func__);
		if (operation->addr < 0) {
			printk("%s: WARN: server sent me and error\n",__func__);
			pcn_kmsg_free_msg(operation);
			kfree(work);
			return ;
		}

		mm->distr_vma_op_counter++;
		struct task_struct *prev = mm->thread_op;

		while(memory->main==NULL)
			schedule();

		mm->thread_op = memory->main;
		up_write(&mm->mmap_sem);

		//wake up the main thread to execute the operation locally
		memory->addr = operation->addr;
		memory->len = operation->len;
		memory->prot = operation->prot;
		memory->new_addr = operation->new_addr;
		memory->new_len = operation->new_len;
		memory->flags = operation->flags;

		//the new_addr sent by the server is fixed
		if (operation->operation == VMA_OP_REMAP)
			memory->flags |= MREMAP_FIXED;

		memory->waiting_for_main = current;
		memory->operation = operation->operation;

		wake_up_process(memory->main);

		PSPRINTK("%s,CLIENT: woke up the main5\n",__func__);

		while (memory->operation != VMA_OP_NOP) {
			set_task_state(current, TASK_UNINTERRUPTIBLE);
			if (memory->operation != VMA_OP_NOP) {
				schedule();
			}
			set_task_state(current, TASK_RUNNING);
		}

		down_write(&mm->mmap_sem);
		memory->waiting_for_main = NULL;
		mm->thread_op = prev;
		mm->distr_vma_op_counter--;

		mm->vma_operation_index++;
		if ( mm->vma_operation_index < 0 )
			printk("%s: WARN: vma_operation_index is underflow detected %d (cpu %d id %d)\n",
					__func__, mm->vma_operation_index, operation->tgroup_home_cpu, operation->tgroup_home_id);

		if (memory->my_lock != 1) {
			printk("Released distributed lock\n");
			up_write(&mm->distribute_sem);
		}

		PSPRINTK("CLIENT: vma_operation_index is %d\n",mm->vma_operation_index);
		PSPRINTK("CLIENT: Ending distributed vma operation\n");
		up_write(&mm->mmap_sem);

		wake_up(&request_distributed_vma_op);
		pcn_kmsg_free_msg(operation);
		kfree(work);
		return ;
	}
}

void process_vma_lock(struct work_struct* work)
{
	vma_lock_work_t* vma_lock_work = (vma_lock_work_t*) work;
	vma_lock_t* lock = vma_lock_work->lock;
	
	memory_t* entry = find_memory_entry(lock->tgroup_home_cpu,
					    lock->tgroup_home_id);
	if (entry != NULL) {
		down_write(&entry->mm->distribute_sem);
		PSVMAPRINTK("Acquired distributed lock\n");
		if (lock->from_cpu == _cpu)
			entry->my_lock = 1;
	}

	vma_ack_t* ack_to_server = (vma_ack_t*) kmalloc(sizeof(vma_ack_t),
							GFP_ATOMIC);
	if (ack_to_server == NULL)
		return ;
	ack_to_server->tgroup_home_cpu = lock->tgroup_home_cpu;
	ack_to_server->tgroup_home_id = lock->tgroup_home_id;
	ack_to_server->vma_operation_index = lock->vma_operation_index;
	ack_to_server->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_ACK;
	ack_to_server->header.prio = PCN_KMSG_PRIO_NORMAL;

	pcn_kmsg_send_long(lock->tgroup_home_cpu,
			   (struct pcn_kmsg_long_message*) (ack_to_server),
			   sizeof(vma_ack_t) - sizeof(struct pcn_kmsg_hdr));

	kfree(ack_to_server);
	pcn_kmsg_free_msg(lock);
	kfree(work);
	return ;
}

static int handle_vma_lock(struct pcn_kmsg_message* inc_msg)
{
	vma_lock_t* lock = (vma_lock_t*) inc_msg;
	vma_lock_work_t* work;

	work = kmalloc(sizeof(vma_lock_work_t), GFP_ATOMIC);

	if (work) {
		work->lock = lock;
		INIT_WORK( (struct work_struct*)work, process_vma_lock);
		queue_work(vma_lock_wq, (struct work_struct*) work);
	}
	else {
		pcn_kmsg_free_msg(lock);
	}
	return 1;
}

static int handle_vma_ack(struct pcn_kmsg_message* inc_msg)
{
	vma_ack_t* ack = (vma_ack_t*) inc_msg;
	vma_op_answers_t* ack_holder;
	unsigned long flags;
	struct task_struct* task_to_wake_up = NULL;

	PSVMAPRINTK("Vma ack received from cpu %d\n", ack->header.from_cpu);
	ack_holder = find_vma_ack_entry(ack->tgroup_home_cpu, ack->tgroup_home_id);
	if (ack_holder) {
		raw_spin_lock_irqsave(&(ack_holder->lock), flags);

		ack_holder->responses++;
		ack_holder->address = ack->addr;

		if (ack_holder->vma_operation_index == -1)
			ack_holder->vma_operation_index = ack->vma_operation_index;
		else if (ack_holder->vma_operation_index != ack->vma_operation_index)
			printk("%s: ERROR: receiving an ack vma for a different operation index\n", __func__);

		if (ack_holder->responses >= ack_holder->expected_responses)
			task_to_wake_up = ack_holder->waiting;

		raw_spin_unlock_irqrestore(&(ack_holder->lock), flags);

		if (task_to_wake_up)
			wake_up_process(task_to_wake_up);
	}

	pcn_kmsg_free_msg(inc_msg);
	return 1;
}

static int handle_vma_op(struct pcn_kmsg_message* inc_msg)
{
	vma_operation_t* operation = (vma_operation_t*) inc_msg;
	vma_op_work_t* work;

	//printk("Received an operation\n");

	memory_t* memory = find_memory_entry(operation->tgroup_home_cpu,
					     operation->tgroup_home_id);
	if (memory != NULL) {

		work = kmalloc(sizeof(vma_op_work_t), GFP_ATOMIC);

		if (work) {
			work->fake = 0;
			work->memory = memory;
			work->operation = operation;
			INIT_WORK( (struct work_struct*)work, process_vma_op);
			queue_work(vma_op_wq, (struct work_struct*) work);
		}
	}
	else {
		if (operation->tgroup_home_cpu == _cpu)
			printk("%s: ERROR: received an operation that said that I am the server but no memory_t found\n", __func__);
		else {
			printk("%s: WARN: Received an operation for a distributed process not present here\n", __func__);
		}
		pcn_kmsg_free_msg(inc_msg);
	}

	return 1;
}

void end_distribute_operation(int operation, long start_ret, unsigned long addr)
{
	int i;

	if (current->mm->distribute_unmap == 0) {
		return;
	}

	PSPRINTK("%s: Ending distributed vma operation %i pid %d\n", __func__, operation,current->pid);
	if (current->mm->distr_vma_op_counter <= 0
	    || (current->main == 0 && current->mm->distr_vma_op_counter > 2)
	    || (current->main == 1 && current->mm->distr_vma_op_counter > 3))
		printk("%s: ERROR: exiting from a distributed vma operation with distr_vma_op_counter = %i (cpu %d, id %d)\n", 
			__func__, current->mm->distr_vma_op_counter, current->tgroup_home_cpu, current->tgroup_home_id);

	(current->mm->distr_vma_op_counter)--;

	if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
		if (current->mm->was_not_pushed <= 0)
			printk("%s: ERROR: exiting from a mapping operation with was_not_pushed = %i (cpu %d, id %d)\n",
			       __func__, current->mm->was_not_pushed, current->tgroup_home_cpu, current->tgroup_home_id);
		current->mm->was_not_pushed--;
	}

	memory_t* entry = find_memory_entry(current->tgroup_home_cpu,
					    current->tgroup_home_id);
	if (entry == NULL) {
		printk("%s: ERROR: Cannot find message to send in exit operation\n", __func__);
	}
	if (start_ret == VMA_OP_SAVE) {

		/*if(operation!=VMA_OP_MAP ||operation!=VMA_OP_REMAP ||operation!=VMA_OP_BRK )
		  printk("ERROR: asking for saving address from operation %i",operation);
		*/
		if (_cpu != current->tgroup_home_cpu)
			printk("%s: ERROR: asking for saving address from a client", __func__);

		//now I have the new address I can send the message
		if (entry->message_push_operation != NULL) {
			if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
				if (current->main == 0)
					printk("%s: ERROR: server not main asked to save operation\n", __func__);
				entry->message_push_operation->addr = addr;
			}
			else {
				if (operation == VMA_OP_REMAP) {
					entry->message_push_operation->new_addr = addr;
				} else
					printk("%s: ERROR: asking for saving address from a wrong operation\n", __func__);
			}

			up_write(&current->mm->mmap_sem);

			if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
				if (pcn_kmsg_send_long(entry->message_push_operation->from_cpu,
						       (struct pcn_kmsg_long_message*) (entry->message_push_operation),
						       sizeof(vma_operation_t) - sizeof(struct pcn_kmsg_hdr))
				    == -1) {
					printk("%s: ERROR: impossible to send operation to client in cpu %d\n", __func__,
					       entry->message_push_operation->from_cpu);
				} else {
					PSPRINTK("%s, operation %d sent to cpu %d\n",__func__,operation, entry->message_push_operation->from_cpu);
				}

			} else {
				PSPRINTK("%s,sending operation %d to all\n",__func__,operation);
				// the list does not include the current processor group descirptor (TODO)
				struct list_head *iter;
				_remote_cpu_info_list_t *objPtr;
				list_for_each(iter, &rlist_head) {
					objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
					i = objPtr->_data._processor;
					if(entry->kernel_set[i]==1)
						pcn_kmsg_send_long(i,
								   (struct pcn_kmsg_long_message*) (entry->message_push_operation),
								   sizeof(vma_operation_t)	- sizeof(struct pcn_kmsg_hdr));
				}
			}

			down_write(&current->mm->mmap_sem);
			if (current->main == 0) {
				kfree(entry->message_push_operation);
				entry->message_push_operation = NULL;
			}
		}
		else {
			printk("%s: ERROR: Cannot find message to send in exit operation (cpu %d id %d)\n",
					__func__, current->tgroup_home_cpu, current->tgroup_home_id);
		}
	}

	if (current->mm->distr_vma_op_counter == 0) {
		current->mm->thread_op = NULL;
		entry->my_lock = 0;

		if(!(operation == VMA_OP_MAP || operation == VMA_OP_BRK)){
			PSPRINTK("%s incrementing vma_operation_index\n",__func__);
			current->mm->vma_operation_index++;
			if (current->mm->vma_operation_index < 0)
				printk("%s: WARN: vma_operation_index underflow detected %d [if:if].(cpu %d id %d)\n",
						__func__, current->mm->vma_operation_index, current->tgroup_home_cpu, current->tgroup_home_id);
		}

		PSPRINTK("Releasing distributed lock\n");
		up_write(&current->mm->distribute_sem);

		if(_cpu == current->tgroup_home_cpu && !(operation == VMA_OP_MAP || operation == VMA_OP_BRK)){
			up_read(&entry->kernel_set_sem);
		}

		wake_up(&request_distributed_vma_op);
	} else
		if (current->mm->distr_vma_op_counter == 1
		    && _cpu == current->tgroup_home_cpu && current->main == 1) {

			if(!(operation == VMA_OP_MAP || operation == VMA_OP_BRK)){
				printk("%s incrementing vma_operation_index\n",__func__);
				current->mm->vma_operation_index++;
				if (current->mm->vma_operation_index < 0)
					printk("%s: WARN: vma_operation_index underflow detected %d [else:if].(cpu %d id %d)\n",
							__func__, current->mm->vma_operation_index, current->tgroup_home_cpu, current->tgroup_home_id);

				up_read(&entry->kernel_set_sem);
			}

			PSPRINTK("Releasing distributed lock\n");
			up_write(&current->mm->distribute_sem);
		}
		else {
			if (!(current->mm->distr_vma_op_counter == 1
			      && _cpu != current->tgroup_home_cpu && current->main == 1)) {

				//nested operation
				if(operation != VMA_OP_UNMAP)
					printk("%s: WARN: exiting from a nest operation that is %d (cpu %d id %d)\n",
							__func__, operation, current->tgroup_home_cpu, current->tgroup_home_id);

				//nested operation do not release the lock
				PSPRINTK("%s incrementing vma_operation_index\n",__func__);
				current->mm->vma_operation_index++;
				if (current->mm->vma_operation_index < 0)
					printk("%s: WARN: vma_operation_index underflow detected %d [else:else].(cpu %d id %d)\n",
							__func__, current->mm->vma_operation_index, current->tgroup_home_cpu, current->tgroup_home_id);
			}
		}
	PSPRINTK("%s: operation index is %d\n", __func__, current->mm->vma_operation_index);
}

/*I assume that down_write(&mm->mmap_sem) is held
 *There are two different protocols:
 *mmap and brk need to only contact the server,
 *all other operations (remap, mprotect, unmap) need that the server pushes it in the system
 */
long start_distribute_operation(int operation, unsigned long addr, size_t len,
				unsigned long prot, unsigned long new_addr, unsigned long new_len,
				unsigned long flags, struct file *file, unsigned long pgoff)
{
	long ret;
	int server;

	if (current->tgroup_home_cpu != _cpu)
		server = 0;
	else
		server = 1;

	//set default ret
	if (server)
		ret = VMA_OP_NOT_SAVE;
	else if (operation == VMA_OP_REMAP)
		ret = new_addr;
	else
		ret = addr;

	/*Operations can be nested-called.
	 * MMAP->UNMAP
	 * BR->UNMAP
	 * MPROT->/
	 * UNMAP->/
	 * MREMAP->UNMAP
	 * =>only UNMAP can be nested-called
	 *
	 * If this is an unmap nested-called by an operation pushed in the system,
	 * skip the distribution part.
	 *
	 * If this is an unmap nested-called by an operation not pushed in the system,
	 * and I am the server, push it in the system.
	 *
	 * If this is an unmap nested-called by an operation not pushed in the system,
	 * and I am NOT the server, it is an error. The server should have pushed that unmap
	 * before, if I am executing it again something is wrong.
	 */
	/*All the operation pushed by the server are executed as not distributed in clients*/
	if (current->mm->distribute_unmap == 0) {
		return ret;
	}
	PSPRINTK("%s: starting vma operation for pid %i tgroup_home_cpu %i tgroup_home_id %i main %d operation %i addr %lx len %lx end %lx\n",
		    __func__, current->pid, current->tgroup_home_cpu, current->tgroup_home_id, current->main?1:0, operation, addr, len, addr+len);
    PSPRINTK("%s index %d\n", __func__, current->mm->vma_operation_index);

	/*only server can have legal distributed nested operations*/
	if (current->mm->distr_vma_op_counter > 0
			&& current->mm->thread_op == current) {
		PSPRINTK("%s, Recursive operation\n",__func__);

		if (server == 0
		    || (current->main == 0 && current->mm->distr_vma_op_counter > 1)
		    || (current->main == 0 && operation != VMA_OP_UNMAP)) {
			printk("ERROR: invalid nested vma operation %i\n", operation);
			return -EPERM;
		}
		else
			/*the main executes the operations for the clients
			 *distr_vma_op_counter is already increased when it start the operation*/
			if (current->main == 1) {
				PSVMAPRINTK("%s, I am the main, so it maybe not a real recursive operation...\n",__func__);
				if (current->mm->distr_vma_op_counter < 1
						|| current->mm->distr_vma_op_counter > 2
						|| (current->mm->distr_vma_op_counter == 2
						&& operation != VMA_OP_UNMAP)) {
					printk("%s: ERROR: invalid nested vma operation in main server\n", __func__);
					return -EPERM;
				} else
					if (current->mm->distr_vma_op_counter == 2) {
						PSVMAPRINTK("%s, Recursive operation for the main\n",__func__);
						/*in this case is a nested operation on main
						 * if the previous operation was a pushed operation
						 * do not distribute it again*/
						if (current->mm->was_not_pushed == 0) {
							current->mm->distr_vma_op_counter++;
							PSVMAPRINTK("%s, don't ditribute again, return!\n",__func__);
							return ret;
						} else
							goto start;
					}
					else
						goto start;
			} else
				if (current->mm->was_not_pushed == 0) {
					current->mm->distr_vma_op_counter++;
					printk("%s, don't ditribute again, return!\n",__func__);
					return ret;
				} else
					goto start;
	}

	/* I did not start an operation, but another thread maybe did...
	 * => no concurrent operations of the same process on the same kernel*/
	while (current->mm->distr_vma_op_counter > 0) {
		printk("%s Somebody already started a distributed operation (current->mm->thread_op->pid is %d). I am pid %d and I am going to sleep (cpu %d id %d)\n",
			    __func__,current->mm->thread_op->pid,current->pid, current->tgroup_home_cpu, current->tgroup_home_id);

		up_write(&current->mm->mmap_sem);
		DEFINE_WAIT(wait);
		prepare_to_wait(&request_distributed_vma_op, &wait,
				TASK_UNINTERRUPTIBLE);
		if (current->mm->distr_vma_op_counter > 0) {
			schedule();
		}
		finish_wait(&request_distributed_vma_op, &wait);
		down_write(&current->mm->mmap_sem);
	}

	if (current->mm->distr_vma_op_counter != 0) {
		printk("%s: ERROR: a new vma operation can be started, but distr_vma_op_counter is %i\n",
				__func__, current->mm->distr_vma_op_counter);
		return -EPERM;
	}

start:
	current->mm->distr_vma_op_counter++;
	current->mm->thread_op = current;

	if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
		current->mm->was_not_pushed++;
	}
	if (server) {
		//SERVER
		if (current->main == 1 && !(current->mm->distr_vma_op_counter>2)) {
			/*I am the main thread=>
			 * a client asked me to do an operation.
			 */
			//(current->mm->vma_operation_index)++;
			int index = current->mm->vma_operation_index;

			PSPRINTK("SERVER MAIN: starting operation %d, current index is %d\n", operation, index);

			up_write(&current->mm->mmap_sem);
			memory_t* entry = find_memory_entry(current->tgroup_home_cpu,
							    current->tgroup_home_id);
			if (entry == NULL || entry->message_push_operation == NULL) {
				printk("ERROR: %s(), Mapping disappeared or cannot find message to update (id %d, cpu %d) \n", __func__, current->tgroup_home_id, current->tgroup_home_cpu);
				down_write(&current->mm->mmap_sem);
				ret = -ENOMEM;
				goto out;
			}

			/*First: send a message to everybody to acquire the lock to block page faults*/
			vma_lock_t* lock_message = (vma_lock_t*) kmalloc(sizeof(vma_lock_t),
									 GFP_ATOMIC);
			if (lock_message == NULL) {
				down_write(&current->mm->mmap_sem);
				ret = -ENOMEM;
				goto out;
			}
			lock_message->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_LOCK;
			lock_message->header.prio = PCN_KMSG_PRIO_NORMAL;
			lock_message->tgroup_home_cpu = current->tgroup_home_cpu;
			lock_message->tgroup_home_id = current->tgroup_home_id;
			lock_message->from_cpu = entry->message_push_operation->from_cpu;
			lock_message->vma_operation_index = index;

			vma_op_answers_t* acks = (vma_op_answers_t*) kmalloc(
				sizeof(vma_op_answers_t), GFP_ATOMIC);
			if (acks == NULL) {
				kfree(lock_message);
				down_write(&current->mm->mmap_sem);
				ret = -ENOMEM;
				goto out;
			}
			acks->tgroup_home_cpu = current->tgroup_home_cpu;
			acks->tgroup_home_id = current->tgroup_home_id;
			acks->vma_operation_index = index;
			acks->waiting = current;
			acks->responses = 0;
			acks->expected_responses = 0;
			raw_spin_lock_init(&(acks->lock));
			add_vma_ack_entry(acks);

			int i, error;

			/*Partial replication: mmap and brk need to communicate only between server and one client
			 * */
			if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
				error = pcn_kmsg_send_long(
					entry->message_push_operation->from_cpu,
					(struct pcn_kmsg_long_message*) (lock_message),
					sizeof(vma_lock_t) - sizeof(struct pcn_kmsg_hdr));
				if (error != -1) {
					acks->expected_responses++;
				}
			}
			else {
				down_read(&entry->kernel_set_sem);

				// the list does not include the current processor group descirptor (TODO)
				struct list_head *iter;
				_remote_cpu_info_list_t *objPtr;
				list_for_each(iter, &rlist_head) {
					objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
					i = objPtr->_data._processor;
					if(entry->kernel_set[i]==1){
						if (current->mm->distr_vma_op_counter == 3
						    && i == entry->message_push_operation->from_cpu)
							continue;

						error = pcn_kmsg_send_long(i,
									   (struct pcn_kmsg_long_message*) (lock_message),sizeof(vma_lock_t) - sizeof(struct pcn_kmsg_hdr));
						if (error != -1) {
							acks->expected_responses++;
						}
					}
				}
			}

			while (acks->expected_responses != acks->responses) {
				set_task_state(current, TASK_UNINTERRUPTIBLE);
				if (acks->expected_responses != acks->responses) {
					schedule();
				}
				set_task_state(current, TASK_RUNNING);
			}

			PSPRINTK("SERVER MAIN: Received all ack to lock\n");

			unsigned long flags;
			raw_spin_lock_irqsave(&(acks->lock), flags);
			raw_spin_unlock_irqrestore(&(acks->lock), flags);
			remove_vma_ack_entry(acks);
			entry->message_push_operation->vma_operation_index = index;

			/*I acquire the lock to block page faults too
			 *Important: this should happen before sending the push message or executing the operation*/
			if (current->mm->distr_vma_op_counter == 2) {
				down_write(&current->mm->distribute_sem);
				printk("local distributed lock acquired\n");
			}

			/* Third: push the operation to everybody
			 * If the operation was a mmap,brk or remap without fixed parameters, I cannot let other kernels
			 * locally choose where to remap it =>
			 * I need to push what the server choose as parameter to the other an push the operation with
			 * a fixed flag.
			 * */
			if (operation == VMA_OP_UNMAP || operation == VMA_OP_PROTECT
			    || ((operation == VMA_OP_REMAP) && (flags & MREMAP_FIXED))) {

				PSPRINTK("SERVER MAIN: sending done for operation, we can execute the operation in parallel! %d\n",operation);
				PSPRINTK("%d %lx %d\n", operation, flags, flags & MREMAP_FIXED);
				// the list does not include the current processor group descirptor (TODO)
				struct list_head *iter;
				_remote_cpu_info_list_t *objPtr;
				list_for_each(iter, &rlist_head) {
					objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
					i = objPtr->_data._processor;
					if(entry->kernel_set[i]==1)
						pcn_kmsg_send_long(i,
								   (struct pcn_kmsg_long_message*) (entry->message_push_operation),sizeof(vma_operation_t)- sizeof(struct pcn_kmsg_hdr));
				}

				kfree(lock_message);
				kfree(acks);
				down_write(&current->mm->mmap_sem);
				return ret;
			}
			else {
				PSPRINTK("SERVER MAIN: going to execute the operation locally %d\n",operation);
				kfree(lock_message);
				kfree(acks);

				down_write(&current->mm->mmap_sem);
				return VMA_OP_SAVE;
			}
		}
		else {
			//server not main
			PSPRINTK("SERVER NOT MAIN: Starting operation %d for pid %d current index is %d\n", operation, current->pid, current->mm->vma_operation_index);
			switch (operation) {
			case VMA_OP_MAP:
			case VMA_OP_BRK:
				//if I am the server, mmap and brk can be executed locally
				PSVMAPRINTK("%s pure local operation!\n",__func__);
				//Note: the order in which locks are taken is important
				up_write(&current->mm->mmap_sem);

				down_write(&current->mm->distribute_sem);
				PSVMAPRINTK("Distributed lock acquired\n");
				down_write(&current->mm->mmap_sem);

				//(current->mm->vma_operation_index)++;
				return ret;
			default:
				break;
			}

			//new push-operation
			PSPRINTK("%s push operation!\n",__func__);
			//(current->mm->vma_operation_index)++;
			int index = current->mm->vma_operation_index;
			PSPRINTK("current index is %d\n", index);

			/*Important: while I am waiting for the acks to the LOCK message
			 * mmap_sem have to be unlocked*/
			up_write(&current->mm->mmap_sem);

			/*First: send a message to everybody to acquire the lock to block page faults*/
			vma_lock_t* lock_message = (vma_lock_t*) kmalloc(sizeof(vma_lock_t),
									 GFP_ATOMIC);
			if (lock_message == NULL) {
				down_write(&current->mm->mmap_sem);
				ret = -ENOMEM;
				goto out;
			}
			lock_message->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_LOCK;
			lock_message->header.prio = PCN_KMSG_PRIO_NORMAL;
			lock_message->tgroup_home_cpu = current->tgroup_home_cpu;
			lock_message->tgroup_home_id = current->tgroup_home_id;
			lock_message->from_cpu = _cpu;
			lock_message->vma_operation_index = index;

			vma_op_answers_t* acks = (vma_op_answers_t*) kmalloc(
				sizeof(vma_op_answers_t), GFP_ATOMIC);
			if (acks == NULL) {
				kfree(lock_message);
				down_write(&current->mm->mmap_sem);
				ret = -ENOMEM;
				goto out;
			}
			acks->tgroup_home_cpu = current->tgroup_home_cpu;
			acks->tgroup_home_id = current->tgroup_home_id;
			acks->vma_operation_index = index;
			acks->waiting = current;
			acks->responses = 0;
			acks->expected_responses = 0;
			raw_spin_lock_init(&(acks->lock));
			int i, error;

			memory_t* entry = find_memory_entry(current->tgroup_home_cpu,
							    current->tgroup_home_id);
			if (entry==NULL) {
				printk("%s: ERROR: Mapping disappeared, cannot save message to update by exit_distribute_operation (cpu %d id %d)\n",
						__func__, current->tgroup_home_cpu, current->tgroup_home_id);
				kfree(lock_message);
				kfree(acks);
				down_write(&current->mm->mmap_sem);
				ret = -EPERM;
				goto out;
			}

			add_vma_ack_entry(acks);
			down_read(&entry->kernel_set_sem);

			// the list does not include the current processor group descirptor (TODO)
			struct list_head *iter;
			_remote_cpu_info_list_t *objPtr;
			list_for_each(iter, &rlist_head) {
				objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
				i = objPtr->_data._processor;

				if(entry->kernel_set[i]==1){
					error = pcn_kmsg_send_long(i,
								   (struct pcn_kmsg_long_message*) (lock_message),	sizeof(vma_lock_t) - sizeof(struct pcn_kmsg_hdr));
					if (error != -1) {
						acks->expected_responses++;
					}
				}
			}

			/*Second: wait that everybody acquire the lock, and acquire it locally too*/
			while (acks->expected_responses != acks->responses) {
				set_task_state(current, TASK_UNINTERRUPTIBLE);
				if (acks->expected_responses != acks->responses) {
					schedule();
				}
				set_task_state(current, TASK_RUNNING);
			}
			PSPRINTK("SERVER NOT MAIN: Received all ack to lock\n");

			unsigned long flags;
			raw_spin_lock_irqsave(&(acks->lock), flags);
			raw_spin_unlock_irqrestore(&(acks->lock), flags);

			remove_vma_ack_entry(acks);

			vma_operation_t* operation_to_send = (vma_operation_t*) kmalloc(
				sizeof(vma_operation_t), GFP_ATOMIC);
			if (operation_to_send == NULL) {
				down_write(&current->mm->mmap_sem);
				up_read(&entry->kernel_set_sem);
				kfree(lock_message);
				kfree(acks);
				ret = -ENOMEM;
				goto out;
			}

			operation_to_send->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_OP;
			operation_to_send->header.prio = PCN_KMSG_PRIO_NORMAL;
			operation_to_send->tgroup_home_cpu = current->tgroup_home_cpu;
			operation_to_send->tgroup_home_id = current->tgroup_home_id;
			operation_to_send->operation = operation;
			operation_to_send->addr = addr;
			operation_to_send->new_addr = new_addr;
			operation_to_send->len = len;
			operation_to_send->new_len = new_len;
			operation_to_send->prot = prot;
			operation_to_send->flags = flags;
			operation_to_send->vma_operation_index = index;
			operation_to_send->from_cpu = _cpu;

			/*I acquire the lock to block page faults too
			 *Important: this should happen before sending the push message or executing the operation*/
			if (current->mm->distr_vma_op_counter == 1) {
				down_write(&current->mm->distribute_sem);
				printk("Distributed lock acquired locally\n");
			}
			/* Third: push the operation to everybody
			 * If the operation was a remap without fixed parameters, I cannot let other kernels
			 * locally choose where to remap it =>
			 * I need to push what the server choose as parameter to the other an push the operation with
			 * a fixed flag.
			 * */
			if (!(operation == VMA_OP_REMAP) || (flags & MREMAP_FIXED)) {

				PSPRINTK("SERVER NOT MAIN: sending done for operation, we can execute the operation in parallel! %d\n",operation);
				// the list does not include the current processor group descirptor (TODO)
				struct list_head *iter;
				_remote_cpu_info_list_t * objPtr;
				list_for_each(iter, &rlist_head) {
					objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
					i = objPtr->_data._processor;

					if(entry->kernel_set[i]==1)
						pcn_kmsg_send_long(i,(struct pcn_kmsg_long_message*) (operation_to_send),
								   sizeof(vma_operation_t)	- sizeof(struct pcn_kmsg_hdr));
				}

				kfree(lock_message);
				kfree(operation_to_send);
				kfree(acks);
				down_write(&current->mm->mmap_sem);
				return ret;
			}
			else {
				PSPRINTK("SERVER NOT MAIN: going to execute the operation locally %d\n",operation);
				entry->message_push_operation = operation_to_send;

				kfree(lock_message);
				kfree(acks);
				down_write(&current->mm->mmap_sem);
				return VMA_OP_SAVE;
			}
		}
	}
	else {
		//CLIENT
		PSPRINTK("CLIENT: starting operation %i for pid %d current index is%d\n",
				operation, current->pid, current->mm->vma_operation_index);

		/*First: send the operation to the server*/
		vma_operation_t* operation_to_send = (vma_operation_t*) kmalloc(
			sizeof(vma_operation_t), GFP_ATOMIC);
		if (operation_to_send == NULL) {
			ret = -ENOMEM;
			goto out;
		}
		operation_to_send->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_OP;
		operation_to_send->header.prio = PCN_KMSG_PRIO_NORMAL;
		operation_to_send->tgroup_home_cpu = current->tgroup_home_cpu;
		operation_to_send->tgroup_home_id = current->tgroup_home_id;
		operation_to_send->operation = operation;
		operation_to_send->addr = addr;
		operation_to_send->new_addr = new_addr;
		operation_to_send->len = len;
		operation_to_send->new_len = new_len;
		operation_to_send->prot = prot;
		operation_to_send->flags = flags;
		operation_to_send->vma_operation_index = -1;
		operation_to_send->from_cpu = _cpu;
		operation_to_send->pgoff = pgoff;
		if (file != NULL) {
			char path[256] = { 0 };
			char* rpath;
			rpath = d_path(&file->f_path, path, 256);
			strcpy(operation_to_send->path, rpath);
		} else
			operation_to_send->path[0] = '\0';

		/*In this case the server will eventually send me the push operation.
		 *Differently from a not-started-by-me push operation, it is not the main thread that has to execute it,
		 *but this thread has.
		 */
		memory_t* entry = find_memory_entry(current->tgroup_home_cpu,
						    current->tgroup_home_id);
		if (entry) {
			if (entry->waiting_for_op != NULL) {
				printk("%s: ERROR: Somebody is already waiting for an op (cpu %d id %d)\n",
						__func__, current->tgroup_home_cpu, current->tgroup_home_id);
				kfree(operation_to_send);
				ret = -EPERM;
				goto out;
			}
			entry->waiting_for_op = current;
			entry->arrived_op = 0;
		}
		else {
			printk("%s: ERROR: Mapping disappeared, cannot wait for push op (cpu %d id %d)\n",
					__func__, current->tgroup_home_cpu, current->tgroup_home_id);
			kfree(operation_to_send);
			ret = -EPERM;
			goto out;
		}

		up_write(&current->mm->mmap_sem);
		int error;
		//send the operation to the server
		error = pcn_kmsg_send_long(current->tgroup_home_cpu,
					   (struct pcn_kmsg_long_message*) (operation_to_send),
					   sizeof(vma_operation_t) - sizeof(struct pcn_kmsg_hdr));
		if (error == -1) {
			printk("%s: ERROR: Impossible to contact the server", __func__);
			kfree(operation_to_send);
			down_write(&current->mm->mmap_sem);
			ret = -EPERM;
			goto out;
		}

		/*Second: the server will send me a LOCK message... another thread will handle it...*/
		/*Third: wait that the server push me the operation*/
		while (entry->arrived_op == 0) {
			set_task_state(current, TASK_UNINTERRUPTIBLE);
			if (entry->arrived_op == 0) {
				schedule();
			}
			set_task_state(current, TASK_RUNNING);
		}

		PSPRINTK("My operation finally arrived pid %d vma operation %d\n",current->pid,current->mm->vma_operation_index);

		/*Note, the distributed lock already has been acquired*/
		down_write(&current->mm->mmap_sem);

		if (current->mm->thread_op != current) {
			printk(	"%s: ERROR: waking up to locally execute a vma operation started by me, but thread_op s not me\n", __func__);
			kfree(operation_to_send);
			ret = -EPERM;
			goto out_dist_lock;
		}

		if (operation == VMA_OP_REMAP || operation == VMA_OP_MAP
		    || operation == VMA_OP_BRK) {
			ret = entry->addr;
			if (entry->addr < 0) {
				printk("%s: WARN: Received error %lx from the server for operation %d\n", __func__, ret,operation);
				goto out_dist_lock;
			}
		}

		entry->waiting_for_op = NULL;
		kfree(operation_to_send);
		return ret;
	}

out_dist_lock:
	up_write(&current->mm->distribute_sem);
	PSPRINTK("Released distributed lock from out_dist_lock\n");
	PSPRINTK("current index is %d in out_dist_lock\n", current->mm->vma_operation_index);

out: current->mm->distr_vma_op_counter--;
	current->mm->thread_op = NULL;
	if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
		current->mm->was_not_pushed--;
	}

	wake_up(&request_distributed_vma_op);
	return ret;
}

long process_server_do_unmap_start(struct mm_struct *mm, unsigned long start,
				   size_t len)
{
	return start_distribute_operation(VMA_OP_UNMAP, start, len, 0, 0, 0, 0,
					  NULL, 0);
}

long process_server_do_unmap_end(struct mm_struct *mm, unsigned long start,
				 size_t len, int start_ret)
{
	end_distribute_operation(VMA_OP_UNMAP, start_ret, start);
	return 0;
}

long process_server_mprotect_start(unsigned long start, size_t len,
				   unsigned long prot)
{
	return start_distribute_operation(VMA_OP_PROTECT, start, len, prot, 0, 0, 0,
					  NULL, 0);
}

long process_server_mprotect_end(unsigned long start, size_t len,
				 unsigned long prot, int start_ret)
{
	end_distribute_operation(VMA_OP_PROTECT, start_ret, start);
	return 0;
}

long process_server_do_mmap_pgoff_start(struct file *file, unsigned long addr,
					unsigned long len, unsigned long prot, unsigned long flags,
					unsigned long pgoff)
{
	return start_distribute_operation(VMA_OP_MAP, addr, len, prot, 0, 0, flags,
					  file, pgoff);
}

long process_server_do_mmap_pgoff_end(struct file *file, unsigned long addr,
				      unsigned long len, unsigned long prot, unsigned long flags,
				      unsigned long pgoff, unsigned long start_ret)
{
	end_distribute_operation(VMA_OP_MAP, start_ret, addr);
	return 0;
}

long process_server_do_brk_start(unsigned long addr, unsigned long len)
{
	return start_distribute_operation(VMA_OP_BRK, addr, len, 0, 0, 0, 0, NULL,
					  0);
}

long process_server_do_brk_end(unsigned long addr, unsigned long len,
			       unsigned long start_ret)
{
	end_distribute_operation(VMA_OP_BRK, start_ret, addr);
	return 0;
}

long process_server_do_mremap_start(unsigned long addr, unsigned long old_len,
				    unsigned long new_len, unsigned long flags, unsigned long new_addr)
{
	return start_distribute_operation(VMA_OP_REMAP, addr, (size_t) old_len, 0,
					  new_addr, new_len, flags, NULL, 0);
}

long process_server_do_mremap_end(unsigned long addr, unsigned long old_len,
				  unsigned long new_len, unsigned long flags, unsigned long new_addr,
				  unsigned long start_ret)
{
	end_distribute_operation(VMA_OP_REMAP, start_ret, new_addr);
	return 0;
}

void sleep_shadow()
{
	memory_t* memory = NULL;
    PSPRINTK("%s pid %d\n", __func__, current->pid);

	/* printk("%s\n", __func__); */

	while (current->executing_for_remote == 0 && current->distributed_exit== EXIT_NOT_ACTIVE) {
		set_task_state(current, TASK_UNINTERRUPTIBLE);
		if (current->executing_for_remote == 0 && current->distributed_exit== EXIT_NOT_ACTIVE) {
			schedule();
		}
		set_task_state(current, TASK_RUNNING);
	}

	PSPRINTK("%s woken up pid %d\n", __func__, current->pid);
	if(current->distributed_exit!= EXIT_NOT_ACTIVE){
		current->represents_remote = 0;
		do_exit(0);
	}

	current->distributed_exit= EXIT_ALIVE;
	current->represents_remote = 0;

	// Notify of PID/PID pairing.
	process_server_notify_delegated_subprocess_starting(current->pid,
							    current->prev_pid, current->prev_cpu);

	//this force the task to wait that the main correctly set up the memory
	while (current->tgroup_distributed != 1) {
		msleep(1);
	}

	PSPRINTK("%s main set up pid %d\n", __func__, current->pid);
	memory = find_memory_entry(current->tgroup_home_cpu,
				   current->tgroup_home_id);
	if (!memory) {
		printk("%s: WARN: cannot find memory_t (cpu %d id %d)\n",
				__func__, current->tgroup_home_cpu, current->tgroup_home_id);
	}
	else
		memory->alive = 1;

	// TODO: Handle return values
	update_thread_info(current);

	//to synchronize migrations...
	// Sharath: Thread won't wait till all 57 threads are migrated
	/*while (atomic_read(&(memory->pending_migration))<57) {
	  msleep(1);
	  }*/
	/*int app= atomic_add_return(-1,&(memory->pending_migration));
	  if(app==57)
	  atomic_set(&(memory->p/n),0);*/
	atomic_dec(&(memory->pending_migration));

	//to synchronize migrations...
	while (atomic_read(&(memory->pending_migration))!=0) {
		msleep(1);
	}

#if MIGRATE_FPU
	// TODO: Handle return values
	update_fpu_info(current);
#endif
	PSPRINTK("%s return pid %d\n", __func__, current->pid);
}

///////////////////////////////////////////////////////////////////////////////
// thread specific handling
///////////////////////////////////////////////////////////////////////////////
int create_user_thread_for_distributed_process(clone_request_t* clone_data,
					       thread_pull_t* my_thread_pull)
{
	shadow_thread_t* my_shadow;
	struct task_struct* task;
	int ret;
	int i, ch;
	const char *name;
	char tcomm[sizeof(task->comm)];

	printk("%s\n", __func__);

	PSPRINTK("%s\n", __func__);

	my_shadow = (shadow_thread_t*) pop_data((data_header_t**)&(my_thread_pull->threads),
						&(my_thread_pull->spinlock));
	if (my_shadow) {
		task = my_shadow->thread;
		PSPRINTK("%s: pop_data pid %d\n", __func__, task->pid);
		if (task == NULL) {
			printk("%s, ERROR task is NULL\n", __func__);
			return -1;
		}
		if (!popcorn_ns) {
			printk("%s: ERROR: no popcorn_ns when forking migrating threads\n", __func__);
			return -1;
		}
		/* if we are already attached, let's skip the unlinking and linking */
		if (task->nsproxy->cpu_ns != popcorn_ns) {
			//i TODO temp fix or of all active cpus?! ---- TODO this must be fixed is not acceptable
			do_set_cpus_allowed(task, cpu_online_mask);
			put_cpu_ns(task->nsproxy->cpu_ns);
			task->nsproxy->cpu_ns = get_cpu_ns(popcorn_ns);
		}
		//associate the task with the namespace
		ret = associate_to_popcorn_ns(task);
		if (ret) {
			printk(KERN_ERR "%s: ERROR: associate_to_popcorn_ns returned: %d\n", __func__,ret);
		}

		name = clone_data->exe_path;
		for (i = 0; (ch = *(name++)) != '\0';) {
			if (ch == '/')
				i = 0;
			else if (i < (sizeof(tcomm) - 1))
				tcomm[i++] = ch;
		}
		tcomm[i] = '\0';
		set_task_comm(task, tcomm);

		// set thread info
		// TODO: Handle return values
		restore_thread_info(task, &clone_data->arch);
		initialize_thread_retval(task, 0);

		task->prev_cpu = clone_data->header.from_cpu;
		task->prev_pid = clone_data->placeholder_pid;
		task->personality = clone_data->personality;
		//	task->origin_pid = clone_data->origin_pid;
		//	sigorsets(&task->blocked,&task->blocked,&clone_data->remote_blocked) ;
		//	sigorsets(&task->real_blocked,&task->real_blocked,&clone_data->remote_real_blocked);
		//	sigorsets(&task->saved_sigmask,&task->saved_sigmask,&clone_data->remote_saved_sigmask);
		//	task->pending = clone_data->remote_pending;
		//	task->sas_ss_sp = clone_data->sas_ss_sp;
		//	task->sas_ss_size = clone_data->sas_ss_size;
		//	int cnt = 0;
		//	for (cnt = 0; cnt < _NSIG; cnt++)
		//		task->sighand->action[cnt] = clone_data->action[cnt];
#if MIGRATE_FPU
		restore_fpu_info(task, &clone_data->arch);
#endif
		//the task will be activated only when task->executing_for_remote==1
		task->executing_for_remote = 1;
		PSPRINTK("%s: wake up %d\n", __func__, task->pid);
		wake_up_process(task);

#if MIGRATION_PROFILE
		migration_end = ktime_get();
		printk(KERN_ERR "Time for arm->x86 migration - x86 side: %ld ns\n", (unsigned long)ktime_to_ns(ktime_sub(migration_end,migration_start)));
#endif
		//printk("%s pc %lx\n", __func__, (&clone_data->arch)->migration_pc);
		//printk("%s pc %lx\n", __func__, task_pt_regs(task)->pc);
		//printk("%s sp %lx\n", __func__, task_pt_regs(task)->sp);
		//printk("%s bp %lx\n", __func__, task_pt_regs(task)->bp);

		printk("####### MIGRATED - PID: %ld to %ld CPU: %d to %d \n",
				(unsigned long)task->prev_pid, (unsigned long)task->pid, task->prev_cpu, _cpu);
		kfree(my_shadow);
		pcn_kmsg_free_msg(clone_data);
		return 0;
	}
	else {
		printk("%s: ERROR: no shadows found! (cpu %d id %ld)\n",
				__func__, clone_data->header.from_cpu, (unsigned long)clone_data->placeholder_pid);
		wake_up_process(my_thread_pull->main);
		return -1;
	}
}

static int create_kernel_thread_for_distributed_process(void *data)
{
	thread_pull_t* my_thread_pull;
	struct cred * new;
	struct mm_struct *mm;
	memory_t* entry;
	struct task_struct* tgroup_iterator = NULL;
	unsigned long flags;
	int i;

	PSPRINTK("%s: current %d\n", __func__, current->pid);
    /* dump_stack(); */

	spin_lock_irq(&current->sighand->siglock);
	flush_signal_handlers(current, 1);
	spin_unlock_irq(&current->sighand->siglock);

	set_cpus_allowed_ptr(current, cpu_all_mask);
	set_user_nice(current, 0);
	new = prepare_kernel_cred(current);
	commit_creds(new);

	mm = mm_alloc();
	if (!mm) {
		printk("%s: ERROR: Impossible allocate new mm_struct\n", __func__);
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

	if (!popcorn_ns) {
		if ((build_popcorn_ns(0)))
			printk("%s: ERROR: build_popcorn returned error\n", __func__);
	}

	my_thread_pull = (thread_pull_t*) kmalloc(sizeof(thread_pull_t),
						  GFP_ATOMIC);
	if (!my_thread_pull) {
		printk("%s: ERROR: kmalloc thread pull\n", __func__);
		return -1;
	}

	init_rwsem(&current->mm->distribute_sem);
	current->mm->distr_vma_op_counter = 0;
	current->mm->was_not_pushed = 0;
	current->mm->thread_op = NULL;
	current->mm->vma_operation_index = 0;
	current->mm->distribute_unmap = 1;
	my_thread_pull->memory= NULL;
	my_thread_pull->threads= NULL;
	raw_spin_lock_init(&(my_thread_pull->spinlock));
	my_thread_pull->main = current;

	push_data((data_header_t**)&(thread_pull_head), &thread_pull_head_lock, (data_header_t *)my_thread_pull);

	// Sharath: Increased the thread pool size
	for (i = 0; i < THREAD_POOL_SIZE; i++) {
		shadow_thread_t* shadow = (shadow_thread_t*) kmalloc(
			sizeof(shadow_thread_t), GFP_ATOMIC);
		if (shadow) {
			/* Ok, create the new process.. */
			shadow->thread = create_thread(CLONE_THREAD | CLONE_SIGHAND | CLONE_VM | CLONE_UNTRACED);
			if (!IS_ERR(shadow->thread)) {
                                /* printk("%s: push thread %d\n", __func__, shadow->thread->pid); */
				push_data((data_header_t**)&(my_thread_pull->threads), &(my_thread_pull->spinlock),
					  (data_header_t *)shadow);
			} else {
				printk("%s: ERROR: not able to create shadow (cpu %d id %d)\n",
						__func__, current->tgroup_home_cpu, current->tgroup_home_id);
				kfree(shadow);
			}
		}
	}

	while (my_thread_pull->memory == NULL) {
		__set_task_state(current, TASK_UNINTERRUPTIBLE);
		if (my_thread_pull->memory == NULL)
			schedule();
		__set_task_state(current, TASK_RUNNING);
	}
    PSPRINTK("%s: after memory current pid %d\n", __func__, current->pid);

	printk("new thread pull aquired! %p\n", my_thread_pull->memory);
	entry = my_thread_pull->memory;
	entry->operation = VMA_OP_NOP;
	entry->waiting_for_main = NULL;
	entry->waiting_for_op = NULL;
	entry->arrived_op = 0;
	entry->my_lock = 0;
	atomic_set(&(entry->pending_back_migration),0);
	memset(entry->kernel_set, 0, MAX_KERNEL_IDS * sizeof(int));
	entry->kernel_set[_cpu] = 1;
	init_rwsem(&entry->kernel_set_sem);

	new_kernel_t* new_kernel_msg = (new_kernel_t*) kmalloc(sizeof(new_kernel_t),
							       GFP_ATOMIC);
	if (new_kernel_msg == NULL) {
		printk("%s: ERROR: impossible to alloc new kernel message\n", __func__);
		BUG();
	}
	new_kernel_msg->tgroup_home_cpu = current->tgroup_home_cpu;
	new_kernel_msg->tgroup_home_id = current->tgroup_home_id;

	new_kernel_msg->header.type = PCN_KMSG_TYPE_PROC_SRV_NEW_KERNEL;
	new_kernel_msg->header.prio = PCN_KMSG_PRIO_NORMAL;

	entry->exp_answ = 0;
	entry->answers = 0;
	raw_spin_lock_init(&(entry->lock_for_answer));
	//inform all kernel that a new distributed process is present here
	// the list does not include the current processor group descirptor (TODO)
	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;
		printk("sending new kernel message to %d\n",i);
		printk("cpu %d id %d\n",new_kernel_msg->tgroup_home_cpu,
		       new_kernel_msg->tgroup_home_id);
		printk("memory pointer from list is %p\n",find_memory_entry(new_kernel_msg->tgroup_home_cpu,new_kernel_msg->tgroup_home_id));
		//if (pcn_kmsg_send(i, (struct pcn_kmsg_message*) (new_kernel_msg))
		//		!= -1) {
		if (pcn_kmsg_send_long(i,
				       (struct pcn_kmsg_long_message*) (new_kernel_msg),
				       sizeof(new_kernel_t) - sizeof(struct pcn_kmsg_hdr)) != -1) {
			// Message delivered
			entry->exp_answ++;
		}
	}
	PSPRINTK("%s: sent %d new kernel messages, current %d\n", __func__, entry->exp_answ, current->pid);

	while (entry->exp_answ != entry->answers) {
		set_task_state(current, TASK_UNINTERRUPTIBLE);
		if (entry->exp_answ != entry->answers) {
			schedule();
		}
		set_task_state(current, TASK_RUNNING);
	}

	PSPRINTK("%s: received all answers\n", __func__);
	kfree(new_kernel_msg);

	lock_task_sighand(current, &flags);

	tgroup_iterator = current;
	while_each_thread(current, tgroup_iterator)
	{
		tgroup_iterator->tgroup_home_id = current->tgroup_home_id;
		tgroup_iterator->tgroup_home_cpu = current->tgroup_home_cpu;
		tgroup_iterator->tgroup_distributed = 1;
	};

	unlock_task_sighand(current, &flags);

	//printk("woke up everybody\n");
	entry->alive = 1;
	entry->setting_up = 0;

	//struct file* f;
	//f = filp_open("/bin/test_thread_migration", O_RDONLY | O_LARGEFILE, 0);
	//if(IS_ERR(f))
	//      printk("Impossible to open file /bin/test_thread_migration error is %d\n",PTR_ERR(f));
	//else
	//      filp_close(f,NULL);

	main_for_distributed_kernel_thread(entry,my_thread_pull);

	printk("%s: ERROR: exited from main_for_distributed_kernel_thread\n", __func__);
	return 0;
}

static int clone_remote_thread_failed(clone_request_t* clone,int inc)
{
	struct file *f;
	memory_t *entry;
	unsigned long flags;
	int ret;

	entry = find_memory_entry(clone->tgroup_home_cpu, clone->tgroup_home_id);

	thread_pull_t* my_thread_pull = NULL;

	do {
		my_thread_pull = (thread_pull_t*) pop_data( (data_header_t**)&(thread_pull_head), &thread_pull_head_lock);
		msleep(10);
	} while(my_thread_pull == NULL);

	printk("%s found a thread pull %lx\n", __func__, clone->stack_start);

	entry->thread_pull = my_thread_pull;
	entry->main = my_thread_pull->main;
	entry->mm = my_thread_pull->main->mm;
	//printk("%s main kernel thread is %p\n",__func__,my_thread_pull->main);
	atomic_inc(&entry->mm->mm_users);
	f = filp_open(clone->exe_path, O_RDONLY | O_LARGEFILE | O_EXCL,
			0);
	if (IS_ERR(f)) {
		printk("ERROR: error opening exe_path\n");
		return -1;
	}
	set_mm_exe_file(entry->mm, f);
	filp_close(f, NULL);
	entry->mm->start_stack = clone->stack_start;
	entry->mm->start_brk = clone->start_brk;
	entry->mm->brk = clone->brk;
	entry->mm->env_start = clone->env_start;
	entry->mm->env_end = clone->env_end;
	entry->mm->arg_start = clone->arg_start;
	entry->mm->arg_end = clone->arg_end;
	entry->mm->start_code = clone->start_code;
	entry->mm->end_code = clone->end_code;
	entry->mm->start_data = clone->start_data;
	entry->mm->end_data = clone->end_data;
	entry->mm->def_flags = clone->def_flags;

	int i, ch;
	const char *name;
	char tcomm[sizeof(my_thread_pull->main->comm)];

	name = clone->exe_path;

	for (i = 0; (ch = *(name++)) != '\0';) {
		if (ch == '/')
			i = 0;
		else if (i < (sizeof(tcomm) - 1))
			tcomm[i++] = ch;
	}
	tcomm[i] = '\0';
	set_task_comm(my_thread_pull->main, tcomm);
	lock_task_sighand(my_thread_pull->main, &flags);
	my_thread_pull->main->tgroup_home_cpu = clone->tgroup_home_cpu;
	my_thread_pull->main->tgroup_home_id = clone->tgroup_home_id;
	my_thread_pull->main->tgroup_distributed = 1;
	unlock_task_sighand(my_thread_pull->main, &flags);
	//the main will be activated only when my_thread_pull->memory !=NULL
	my_thread_pull->memory = entry;
	wake_up_process(my_thread_pull->main);
	//printk("%s before calling create user thread\n",__func__);

	return create_user_thread_for_distributed_process(clone, my_thread_pull);
}

static int clone_remote_thread(clone_request_t* clone,int inc) {
	struct file* f;
	memory_t* memory = NULL;
	unsigned long flags;
	int ret;

	PSPRINTK("%s inc %d cpu %d id %d prev pid %d origin pid %d\n", __func__,
			inc, clone->tgroup_home_cpu, clone->tgroup_home_id,
			clone->prev_pid, clone->origin_pid);
retry:
	memory = find_memory_entry(clone->tgroup_home_cpu,
				  clone->tgroup_home_id);
	if (memory) {
		PSPRINTK("%s memory found\n", __func__);
		if(inc){
			atomic_inc(&(memory->pending_migration));
			/*int app= atomic_add_return(1,&(memory->pending_migration));
			  if(app==57)
			  atomic_set(&(memory->pending_migration),114);*/

		}
		if (memory->thread_pull) {
			return create_user_thread_for_distributed_process(clone,
									  memory->thread_pull);
		} else {
			//	printk("%s thread pull not ready yet\n", __func__);
			return -1;
		}

	}
	else {
		PSPRINTK("%s memory not found\n", __func__);
		memory_t* entry = (memory_t*) kmalloc(sizeof(memory_t), GFP_ATOMIC);
		if (!entry) {
			printk("%s: ERROR: Impossible allocate memory_t\n", __func__);
			return -1;
		}

		entry->tgroup_home_cpu = clone->tgroup_home_cpu;
		entry->tgroup_home_id = clone->tgroup_home_id;
		entry->setting_up = 1;
		entry->thread_pull = NULL;
		atomic_set(&(entry->pending_migration),1);
		ret = add_memory_entry_with_check(entry);

		PSPRINTK("%s ret %d\n", __func__, ret);
		if (ret == 0) {
			thread_pull_t* my_thread_pull = (thread_pull_t*) pop_data(
				(data_header_t**)&(thread_pull_head), &thread_pull_head_lock);

			PSPRINTK("%s my_thread_pull %lx\n", __func__, my_thread_pull);
			if (my_thread_pull) {
				int i, ch;
				const char *name;
				char tcomm[sizeof(my_thread_pull->main->comm)];

				entry->thread_pull = my_thread_pull;
				entry->main = my_thread_pull->main;
				entry->mm = my_thread_pull->main->mm;
				atomic_inc(&entry->mm->mm_users);
				f = filp_open(clone->exe_path, O_RDONLY | O_LARGEFILE | O_EXCL,
					      0);
				if (IS_ERR(f)) {
					printk("%s: ERROR: error opening exe_path\n", __func__);
					return -1;
				}
				set_mm_exe_file(entry->mm, f);
				filp_close(f, NULL);
				entry->mm->start_stack = clone->stack_start;
				entry->mm->start_brk = clone->start_brk;
				entry->mm->brk = clone->brk;
				entry->mm->env_start = clone->env_start;
				entry->mm->env_end = clone->env_end;
				entry->mm->arg_start = clone->arg_start;
				entry->mm->arg_end = clone->arg_end;
				entry->mm->start_code = clone->start_code;
				entry->mm->end_code = clone->end_code;
				entry->mm->start_data = clone->start_data;
				entry->mm->end_data = clone->end_data;
				entry->mm->def_flags = clone->def_flags;
                                // if popcorn_vdso is zero it should be initialized with the address provided by the home kernel
                if (entry->mm->context.popcorn_vdso == 0) {
                	unsigned long popcorn_addr = clone->popcorn_vdso;
                	struct page ** popcorn_pagelist = kzalloc(sizeof(struct page *) * (1 + 1), GFP_KERNEL);
                	if (popcorn_pagelist == NULL) {
                		pr_err("Failed to allocate vDSO pagelist!\n");
                		ret = -ENOMEM;
                		goto up_fail;
				    }
                	popcorn_pagelist[0] = alloc_pages(GFP_KERNEL | __GFP_ZERO, 0);
                	if (!popcorn_pagelist[0]) {
                		printk("%s: ERROR: alloc_pages failed for popcorn_vdso\n", __func__);
                		ret = -ENOMEM;
                		goto up_fail;
                	}
                	ret = install_special_mapping(entry->mm, popcorn_addr, PAGE_SIZE,
				                                      VM_READ|VM_EXEC|
				                                      VM_MAYREAD|VM_MAYWRITE|VM_MAYEXEC,
				                                      popcorn_pagelist);
                	if (!ret)
                		entry->mm->context.popcorn_vdso = (void *)popcorn_addr;
                	else
                		free_page((unsigned long)popcorn_pagelist[0]);
                }
up_fail:
				// popcorn_vdso cannot be different
				if ( ((unsigned long)entry->mm->context.popcorn_vdso) != clone->popcorn_vdso) {
					printk(KERN_ERR"%s: ERROR: popcorn_vdso entry:0x%lx clone:0x%lx\n",
							__func__, (unsigned long)entry->mm->context.popcorn_vdso, clone->popcorn_vdso);
					BUG();
				}

				name = clone->exe_path;
				for (i = 0; (ch = *(name++)) != '\0';) {
					if (ch == '/')
						i = 0;
					else if (i < (sizeof(tcomm) - 1))
						tcomm[i++] = ch;
				}
				tcomm[i] = '\0';
				set_task_comm(my_thread_pull->main, tcomm);
				lock_task_sighand(my_thread_pull->main, &flags);
				my_thread_pull->main->tgroup_home_cpu = clone->tgroup_home_cpu;
				my_thread_pull->main->tgroup_home_id = clone->tgroup_home_id;
				my_thread_pull->main->tgroup_distributed = 1;
				unlock_task_sighand(my_thread_pull->main, &flags);
				//the main will be activated only when my_thread_pull->memory !=NULL
				my_thread_pull->memory = entry;
				PSPRINTK("%s: wake up process\n", __func__);
				PSPRINTK("%s: wake up process %lx\n", __func__, my_thread_pull->main);
				PSPRINTK("%s: wake up process %d\n", __func__, my_thread_pull->main->pid);
				wake_up_process(my_thread_pull->main);
				//printk("%s before calling create user thread\n",__func__);
				return create_user_thread_for_distributed_process(clone,
										  my_thread_pull);
			}
			else {
				struct work_struct* work = kmalloc(sizeof(struct work_struct),
						GFP_ATOMIC);
				if (work) {
					INIT_WORK( work, update_thread_pull);
					queue_work(clone_wq, work);
				}
				return -2;
			}
		}
		else {
			kfree(entry);
			goto retry;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// handle clone
///////////////////////////////////////////////////////////////////////////////
void try_create_remote_thread(struct work_struct* work)
{
	clone_work_t* clone_work = (clone_work_t*) work;
	clone_request_t* clone = clone_work->request;
	int ret;

	ret = clone_remote_thread(clone, 0);
	if (ret != 0) {
		clone_work_t* delay = (clone_work_t*) kmalloc(sizeof(clone_work_t),
							      GFP_ATOMIC);
		if (delay) {
			delay->request = clone;
			INIT_DELAYED_WORK( (struct delayed_work*)delay,
					   try_create_remote_thread);
			queue_delayed_work(clone_wq, (struct delayed_work*) delay, 10);
		}
	}
	kfree(work);
}

static int handle_clone_request(struct pcn_kmsg_message* inc_msg)
{
	clone_request_t* clone = (clone_request_t*) inc_msg;
	int ret;

#if MIGRATION_PROFILE
	migration_start = ktime_get();
#endif
	ret = clone_remote_thread(clone, 1);

	if (ret != 0) {
		clone_work_t* request_work = kmalloc(sizeof(clone_work_t), GFP_ATOMIC);

		if (request_work) {
			request_work->request = clone;
			if (ret == -2) {
				INIT_WORK( (struct work_struct*)request_work,
						clone_remote_thread_failed);
			} else {
				INIT_WORK( (struct work_struct*)request_work,
						try_create_remote_thread);
			}
			queue_work(clone_wq, (struct work_struct*) request_work);
		}
	}
	return 0;
}

/* Ajith - adding file offset parsing */
static unsigned long get_file_offset(struct file *file, int start_addr)
{
	struct elfhdr elf_ex;
	struct elf_phdr *elf_eppnt = NULL, *elf_eppnt_start = NULL;
	int size, retval, i;

	retval = kernel_read(file, 0, (char *)&elf_ex, sizeof(elf_ex));
	if (retval != sizeof(elf_ex)) {
		printk("%s: ERROR in Kernel read of ELF file\n", __func__);
		retval = -1;
		goto out;
	}

	size = elf_ex.e_phnum * sizeof(struct elf_phdr);

	elf_eppnt = kmalloc(size, GFP_KERNEL);
	if(elf_eppnt == NULL) {
		printk("%s: ERROR: kmalloc failed in\n", __func__);
		retval = -1;
		goto out;
	}

	elf_eppnt_start = elf_eppnt;
	retval = kernel_read(file, elf_ex.e_phoff,
			     (char *)elf_eppnt, size);
	if (retval != size) {
		printk("%s: ERROR: during kernel read of ELF file\n", __func__);
		retval = -1;
		goto out;
	}
	for (i = 0; i < elf_ex.e_phnum; i++, elf_eppnt++) {
		if (elf_eppnt->p_type == PT_LOAD) {

			printk("%s: Page offset for 0x%x 0x%lx 0x%lx\n",
				__func__, start_addr, (unsigned long)elf_eppnt->p_vaddr, (unsigned long)elf_eppnt->p_memsz);

			if((start_addr >= elf_eppnt->p_vaddr) && (start_addr <= (elf_eppnt->p_vaddr+elf_eppnt->p_memsz)))
			{
				printk("%s: Finding page offset for 0x%x 0x%lx 0x%lx\n",
					__func__, start_addr, (unsigned long)elf_eppnt->p_vaddr, (unsigned long)elf_eppnt->p_memsz);
				retval = (elf_eppnt->p_offset - (elf_eppnt->p_vaddr & (ELF_MIN_ALIGN-1)));
				goto out;
			}
/*
  if ((elf_eppnt->p_flags & PF_R) && (elf_eppnt->p_flags & PF_X)) {
  printk("Coming to executable program load section\n");
  retval = (elf_eppnt->p_offset - (elf_eppnt->p_vaddr & (ELF_MIN_ALIGN-1)));
  goto out;
  }
*/
		}
	}

out:
	if(elf_eppnt_start != NULL)
		kfree(elf_eppnt_start);

	return retval >> PAGE_SHIFT;
}

///////////////////////////////////////////////////////////////////////////////
// scheduling stuff
///////////////////////////////////////////////////////////////////////////////
static int popcorn_sched_sync(void)
{
        int cpu;
        sched_periodic_req req;

#if defined(CONFIG_ARM64)
        cpu = 1;
#else
        cpu = 0;
#endif

        while (1) {
                usleep_range(200000, 250000); // total time on ARM is currently around 100ms (not busy waiting)

		req.header.type = PCN_KMSG_TYPE_SCHED_PERIODIC;
		req.header.prio = PCN_KMSG_PRIO_NORMAL;

#if defined(CONFIG_ARM64)
                req.power_1 = popcorn_power_arm_1[POPCORN_POWER_N_VALUES - 1];
                req.power_2 = popcorn_power_arm_2[POPCORN_POWER_N_VALUES - 1];
                req.power_3 = popcorn_power_arm_3[POPCORN_POWER_N_VALUES - 1];
#else
                req.power_1 = popcorn_power_x86_1[POPCORN_POWER_N_VALUES - 1];
                req.power_2 = popcorn_power_x86_2[POPCORN_POWER_N_VALUES - 1];
                req.power_3 = 0;
#endif

		pcn_kmsg_send_long(cpu, (struct pcn_kmsg_long_message*) &req,
                                   sizeof(sched_periodic_req) - sizeof(struct pcn_kmsg_hdr));
        }
        return 0;
}

static int handle_sched_periodic(struct pcn_kmsg_message *inc_msg)
{
        sched_periodic_req *req = (sched_periodic_req *)inc_msg;

#if defined(CONFIG_ARM64)
	popcorn_power_x86_1[POPCORN_POWER_N_VALUES - 1] = req->power_1;
        popcorn_power_x86_2[POPCORN_POWER_N_VALUES - 1] = req->power_2;
//        popcorn_power_x86_3[POPCORN_POWER_N_VALUES - 1] = req->power_3;
#else
        popcorn_power_arm_1[POPCORN_POWER_N_VALUES - 1] = req->power_1;
        popcorn_power_arm_2[POPCORN_POWER_N_VALUES - 1] = req->power_2;
        popcorn_power_arm_3[POPCORN_POWER_N_VALUES - 1] = req->power_3;
#endif
        /*printk("power: %d %d %d\n", req->power_1, req->power_2, req->power_3);*/
	pcn_kmsg_free_msg(inc_msg);
        return 0;
}

static ssize_t power_read (struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
        int ret, len = 0;
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        if (*ppos > 0)
                return 0; //EOF

        len += snprintf(buffer, sizeof(buffer),
                "ARM\t%d\t%d\t%d\n",
                popcorn_power_arm_1[POPCORN_POWER_N_VALUES - 1],
                popcorn_power_arm_2[POPCORN_POWER_N_VALUES - 1],
                popcorn_power_arm_3[POPCORN_POWER_N_VALUES - 1]);
        len += snprintf((buffer +len), sizeof(buffer) -len,
                "x86\t%d\t%d\n", 
                popcorn_power_x86_1[POPCORN_POWER_N_VALUES - 1],
                popcorn_power_x86_2[POPCORN_POWER_N_VALUES - 1]);

        if (count < len)
                len = count;
        ret = copy_to_user(buf, buffer, len);

        *ppos += len;
        return len;
}

static const struct file_operations power_fops = {
        .owner = THIS_MODULE,
        .read = power_read,
};

///////////////////////////////////////////////////////////////////////////////
// Global VDSO Support (to be removed)
///////////////////////////////////////////////////////////////////////////////
long tell_migration = 0;
static ssize_t mtrig_write (struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
        int ret, len;
        char buffer[8];
        memset(buffer, 0, sizeof(buffer));
	len = count > sizeof(buffer) ? sizeof(buffer) : count;

        ret = copy_from_user(buffer, buf, len);
        tell_migration = simple_strtol(buffer, NULL, 0);

        suggest_migration((int)tell_migration);
        //printk("%s: suggest_migration %d (%d,%d,%d)\n", __func__, (int)tell_migration, ret, (int) count, len);
        return len;
}

static ssize_t mtrig_read (struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
        int ret, len;
        char buffer[8];
        memset(buffer, 0, sizeof(buffer));
	if (*ppos > 0)
		return 0; //EOF

        len = snprintf(buffer, sizeof(buffer), "%d", (int)tell_migration);
        if (count < len)
                len = count;
        ret = copy_to_user(buf, buffer, len);
        //printk("%s: tell_migration %d (%d,%d,%d,%ld)\n", __func__, (int)tell_migration, ret, len, count, (long)*ppos);

        *ppos += len;
        return len;
}

static const struct file_operations mtrig_fops = {
        .owner = THIS_MODULE,
        .read = mtrig_read,
        .write = mtrig_write,
};

/**
 * process_server_init
 * Start the process loop in a new kthread.
 */
static int __init process_server_init(void)
{
        int i;
	uint16_t copy_cpu;

        struct task_struct *kt_sched;
        struct proc_dir_entry *res;

	printk("\n\nPopcorn version with user data replication\n\n");
/*	printk("Per me si va ne la città dolente,\n"
	       "per me si va ne l'etterno dolore,\n"
	       "per me si va tra la perduta gente.\n"
	       "Giustizia mosse il mio alto fattore;\n"
	       "fecemi la divina podestate,\n"
	       "la somma sapïenza e 'l primo amore.\n"
	       "Dinanzi a me non fuor cose create\n"
	       "se non etterne, e io etterno duro.\n"
	       "Lasciate ogne speranza, voi ch'intrate\n\n");
*/

	/*#ifndef SUPPORT_FOR_CLUSTERING
	  _cpu= smp_processor_id();
	  #else
	  _cpu= cpumask_first(cpu_present_mask);
	  #endif
	*/

	if(pcn_kmsg_get_node_ids(NULL, 0, &copy_cpu)==-1)
		printk("ERROR process_server cannot initialize _cpu\n");
	else{
		_cpu= copy_cpu;
		_file_cpu = _cpu;
		printk("process_server: I am cpu %d\n",_cpu);
	}
	file_wait_q();

	/*
	 * Create a work queue so that we can do bottom side
	 * processing on data that was brought in by the
	 * communications module interrupt handlers.
	 */
	clone_wq = create_workqueue("clone_wq");
	exit_wq  = create_workqueue("exit_wq");
	exit_group_wq = create_workqueue("exit_group_wq");
	message_request_wq = create_workqueue("request_wq");
	invalid_message_wq= create_workqueue("invalid_wq");
	new_kernel_wq= create_workqueue("new_kernel_wq");

	/*
	 * These two workqueues are singlethread because we ran into
	 * synchronization issues using multithread workqueues.
	 */
	vma_op_wq = create_singlethread_workqueue("vma_op_wq");
	vma_lock_wq = create_singlethread_workqueue("vma_lock_wq");

#if STATISTICS
	page_fault_mio=0;

	fetch=0;
	local_fetch=0;
	write=0;
	concurrent_write= 0;
	most_long_write=0;
	read=0;

	invalid=0;
	ack=0;
	answer_request=0;
	answer_request_void=0;
	request_data=0;

	most_written_page= 0;
	most_long_read= 0;
	pages_allocated=0;

	compressed_page_sent=0;
	not_compressed_page=0;
	not_compressed_diff_page=0;

#endif

	/*
	 * Register to receive relevant incomming messages.
	 */
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_EXIT_PROCESS,
				   handle_exiting_process_notification);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_CREATE_PROCESS_PAIRING,
				   handle_process_pairing_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_CLONE_REQUEST,
				   handle_clone_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST,
				   handle_mapping_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE,
				   handle_mapping_response);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE_VOID,
				   handle_mapping_response_void);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_INVALID_DATA,
				   handle_invalid_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_ACK_DATA,
				   handle_ack);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_REQUEST,
				   handle_remote_thread_count_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_RESPONSE,
				   handle_remote_thread_count_response);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_THREAD_GROUP_EXITED_NOTIFICATION,
				   handle_thread_group_exited_notification);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_VMA_OP,
				   handle_vma_op);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_VMA_ACK,
				   handle_vma_ack);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_VMA_LOCK,
				   handle_vma_lock);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_NEW_KERNEL,
				   handle_new_kernel);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_NEW_KERNEL_ANSWER,
				   handle_new_kernel_answer);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_BACK_MIG_REQUEST,
				   handle_back_migration);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_CREATE_THREAD_PULL,
				   handle_thread_pull_creation);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FILE_OPEN_REQUEST,
				   handle_file_open_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FILE_OPEN_REPLY,
				   handle_file_open_reply);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FILE_STATUS_REQUEST,
				   handle_file_status_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FILE_STATUS_REPLY,
				   handle_file_status_reply);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FILE_OFFSET_REQUEST,
				   handle_file_offset_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FILE_OFFSET_REPLY,
				   handle_file_offset_reply);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FILE_OFFSET_UPDATE,
				   handle_file_pos_update);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FILE_OFFSET_CONFIRM,
			handle_file_pos_confirm);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_SCHED_PERIODIC,
				   handle_sched_periodic);

	popcorn_power_x86_1 = (int *) kmalloc(POPCORN_POWER_N_VALUES * sizeof(int), GFP_ATOMIC);
	popcorn_power_x86_2 = (int *) kmalloc(POPCORN_POWER_N_VALUES * sizeof(int), GFP_ATOMIC);
	popcorn_power_arm_1 = (int *) kmalloc(POPCORN_POWER_N_VALUES * sizeof(int), GFP_ATOMIC);
	popcorn_power_arm_2 = (int *) kmalloc(POPCORN_POWER_N_VALUES * sizeof(int), GFP_ATOMIC);
	popcorn_power_arm_3 = (int *) kmalloc(POPCORN_POWER_N_VALUES * sizeof(int), GFP_ATOMIC);

        for (i = 0; i < POPCORN_POWER_N_VALUES; i++) {
                popcorn_power_x86_1[i] = 0;
                popcorn_power_x86_2[i] = 0;
                popcorn_power_arm_1[i] = 0;
                popcorn_power_arm_2[i] = 0;
                popcorn_power_arm_3[i] = 0;
        }

	kt_sched = kthread_run(popcorn_sched_sync, NULL, "popcorn_sched_sync");

        if (kt_sched < 0)
                printk("ERROR: cannot create popcorn_sched_sync thread\n");

        res = proc_create("power", S_IRUGO, NULL, &power_fops);
        if (!res)
                printk("ERROR: failed to create proc entry for power\n");

        res = proc_create("mtrig", S_IRUGO, NULL, &mtrig_fops);
        if (!res)
                printk("ERROR: failed to create proc entry for triggering miggrations (global VDSO)\n");
	return 0;
}

/**
 * Register process server init function as
 * module initialization function.
 */
late_initcall_popcorn(process_server_init);
