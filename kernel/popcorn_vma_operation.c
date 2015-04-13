/*
 * popcorn_vma_operation.c
 *
 * Author: Marina Sadini, SSRG Virginia Tech
 */

#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/popcorn_vma_operation.h>
#include <linux/pcn_kmsg.h>
#include <linux/popcorn_cpuinfo.h>

#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

/* Workqueues to dispatch incoming messages.
 * Used to remove part of the work from the messaging layer handlers.
 */
static struct workqueue_struct *vma_op_wq;
static struct workqueue_struct *vma_lock_wq;

/* Functions used to start and end vma operations on popcorn.
 * They are called from the corresponding "linux" counterparts.
 * */
long popcorn_do_unmap_start(struct mm_struct *mm, unsigned long start, size_t len);
long popcorn_do_unmap_end(struct mm_struct *mm, unsigned long start, size_t len,int start_ret);

long popcorn_mprotect_start(unsigned long start, size_t len,unsigned long prot);
long popcorn_mprotect_end(unsigned long start, size_t len,unsigned long prot,int start_ret);

long popcorn_do_mmap_pgoff_start(struct file *file, unsigned long addr,
		unsigned long len, unsigned long prot,
		unsigned long flags, unsigned long pgoff);
long popcorn_do_mmap_pgoff_end(struct file *file, unsigned long addr,
		unsigned long len, unsigned long prot,
		unsigned long flags, unsigned long pgoff, unsigned long start_ret);

long popcorn_do_brk_start(unsigned long addr, unsigned long len);
long popcorn_do_brk_end(unsigned long addr, unsigned long len, unsigned long start_ret);

long popcorn_do_mremap_start(unsigned long addr,
		unsigned long old_len, unsigned long new_len,
		unsigned long flags, unsigned long new_addr);
long popcorn_do_mremap_end(unsigned long addr,
		unsigned long old_len, unsigned long new_len,
		unsigned long flags, unsigned long new_addr,unsigned long start_ret);

/* Core functions to handle vma operations on Popcorn.
 * The protocol used is thought to work for n kernels.
 * There is a server kernel for each distributed process. The server is in charge to keep a
 * complete vision of vmas and serialize operations.
 * Client kernels have a partial vision of vmas, indeed in clients vmas are not copied during
 * migration but piggybacked on demand during page fault.
 * Clients always need to contact the server to start a new vma operation.
 * Upon new operation arrival, the server excludes the page fault protocol on all the clients
 * (or just one according to the operation type) by forcing the acquisition of the distribute_sem lock in write,
 * and after it pushes the operation on all the previously contacted clients.
 */
static long start_distribute_operation(int operation, unsigned long addr,
		size_t len, unsigned long prot, unsigned long new_addr,
		unsigned long new_len, unsigned long flags, struct file *file,
		unsigned long pgoff);
static long start_operation_server(int operation, unsigned long addr,
		size_t len, unsigned long prot, unsigned long new_addr,
		unsigned long new_len, unsigned long flags, struct file *file,
		unsigned long pgoff);
static long start_operation_client(int operation, unsigned long addr,
		size_t len, unsigned long prot, unsigned long new_addr,
		unsigned long new_len, unsigned long flags, struct file *file,
		unsigned long pgoff);
static void end_distribute_operation(int operation, long start_ret,
		unsigned long addr);
void process_vma_lock(struct work_struct* work);
static int handle_vma_lock(struct pcn_kmsg_message* inc_msg);
static void process_vma_op_server(struct mm_struct* mm,
		struct migration_memory* memory,
		struct vma_operation* operation);
static void process_vma_op_client(struct mm_struct* mm,
		struct migration_memory* memory,
		struct vma_operation* operation);
void process_vma_op(struct work_struct* work);
static int handle_vma_op(struct pcn_kmsg_message* inc_msg);
void execute_pending_vma_operation(struct migration_memory *mm_data);
int do_mapping_for_distributed_process_from_page_fault(unsigned long vm_flags, unsigned long vaddr_start, unsigned long vaddr_size, unsigned long pgoff, char* path,
		struct mm_struct* mm, unsigned long address, spinlock_t* ptl);

/* List used to store acks to vma operation locks.
 */
struct vma_op_answers* _vma_ack_head = NULL;
DEFINE_RAW_SPINLOCK(_vma_ack_head_lock);
void add_vma_ack_entry(struct vma_op_answers* entry);
struct vma_op_answers* find_vma_ack_entry(int cpu, int id);
void remove_vma_ack_entry(struct vma_op_answers* entry);

DECLARE_WAIT_QUEUE_HEAD(request_distributed_vma_op);

extern int scif_get_nodeIDs(uint16_t *nodes, int len, uint16_t *self);

static int _cpu = -1;

/* Map a vma without overlapping with previously existing vmas.
 * This should prevent unmap from happening.
 */
static unsigned long map_difference(struct file *file, unsigned long addr,
		unsigned long len, unsigned long prot, unsigned long flags,
		unsigned long pgoff) {
	unsigned long ret = addr;
	unsigned long start = addr;
	unsigned long local_end = start;
	unsigned long end = addr + len;
	struct vm_area_struct* curr;
	unsigned long error;

	curr = current->mm->mmap;

	while (1) {

		if (start >= end)
			goto done;

		/* We've reached the end of the list */
		else if (curr == NULL) {
			/* map through the end */
			error = do_mmap(file, start, end - start, prot, flags, pgoff);
			if (error != start) {
				ret = VM_FAULT_VMA;
			}
			goto done;
		}

		/* the VMA is fully above the region of interest */
		else if (end <= curr->vm_start) {
			/* mmap through local_end */
			error = do_mmap(file, start, end - start, prot, flags, pgoff);
			if (error != start)
				ret = VM_FAULT_VMA;
			goto done;
		}

		/* the VMA fully encompasses the region of interest */
		else if (start >= curr->vm_start && end <= curr->vm_end) {
			/* nothing to do */
			goto done;
		}

		/* the VMA is fully below the region of interest*/
		else if (curr->vm_end <= start) {
			/* move on to the next one*/

		}

		/* the VMA includes the start of the region of interest
		 * but not the end
		 */
		else if (start >= curr->vm_start && start < curr->vm_end
				&& end > curr->vm_end) {
			/* advance start (no mapping to do)*/
			start = curr->vm_end;
			local_end = start;

		}

		/* the VMA includes the end of the region of interest
		 * but not the start
		 */
		else if (start < curr->vm_start && end <= curr->vm_end
				&& end > curr->vm_start) {
			local_end = curr->vm_start;

			/* mmap through local_end */
			error = do_mmap(file, start, local_end - start, prot, flags, pgoff);
			if (error != start)
				ret = VM_FAULT_VMA;

			/* Then we're done*/
			goto done;
		}

		/* the VMA is fully within the region of interest*/
		else if (start <= curr->vm_start && end >= curr->vm_end) {
			/* advance local end */
			local_end = curr->vm_start;

			/* map the difference */
			error = do_mmap(file, start, local_end - start, prot, flags, pgoff);
			if (error != start)
				ret = VM_FAULT_VMA;

			/* Then advance to the end of this vma*/
			start = curr->vm_end;
			local_end = start;
		}

		curr = curr->vm_next;

	}

	done:

	return ret;
}

/* During thread migration vmas are not sent with the thread.
 * Distributed page faults piggyback vma information on demand.
 * This function allows popcorn page fault to properly map a piggybacked vma.
 */
int do_mapping_for_distributed_process_from_page_fault(unsigned long vm_flags, unsigned long vaddr_start, unsigned long vaddr_size, unsigned long pgoff, char* path,
		struct mm_struct* mm, unsigned long address, spinlock_t* ptl)
{

	struct vm_area_struct* vma;
	unsigned long prot = 0;
	unsigned long err, ret;

	prot |= (vm_flags & VM_READ) ? PROT_READ : 0;
	prot |= (vm_flags & VM_WRITE) ? PROT_WRITE : 0;
	prot |= (vm_flags & VM_EXEC) ? PROT_EXEC : 0;

	if(path[0] == '\0') {

		vma = find_vma(mm, address);
		if(!vma || address >= vma->vm_end || address < vma->vm_start) {
			vma = NULL;
		}

		if(!vma || (vma->vm_start != vaddr_start)
				|| (vma->vm_end != (vaddr_start + vaddr_size))) {

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
			 * => take the newest vma.
			 * */
			vma = find_vma(mm, address);
			if(!vma || address >= vma->vm_end
					|| address < vma->vm_start) {
				vma = NULL;
			}

			/* When I receive a vma, the only difference can be on the size (start, end) of the vma.
			 */
			if(!vma || (vma->vm_start != vaddr_start)
					|| (vma->vm_end != (vaddr_start	+ vaddr_size))) {
				VMAPRINTK("Mapping anonimous vma start %lu end %lu \n",
						vaddr_start,
						(vaddr_start + vaddr_size));

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
				err =
						map_difference(NULL,
								vaddr_start,
								vaddr_size,
								prot,
								MAP_FIXED
								| MAP_ANONYMOUS
								| ((vm_flags
										& VM_SHARED) ?
												MAP_SHARED :
												MAP_PRIVATE)
												| ((vm_flags
														& VM_HUGETLB) ?
																MAP_HUGETLB :
																0)
																| ((vm_flags
																		& VM_GROWSDOWN) ?
																				MAP_GROWSDOWN :
																				0),
																				0);

				current->mm->distribute_unmap = 1;

				if(err != vaddr_start) {
					up_write(&mm->mmap_sem);
					down_read(&mm->mmap_sem);
					spin_lock(ptl);
					/*PTE LOCKED*/
					printk("ERROR: error mapping anonymous vma while fetching address %lu in %s\n",
							address, __func__);
					ret = VM_FAULT_VMA;
					return ret;
				}

			}

			up_write(&mm->mmap_sem);
			down_read(&mm->mmap_sem);
			spin_lock(ptl);
			/*PTE LOCKED*/
		}

	} else {

		vma = find_vma(mm, address);
		if(!vma || address >= vma->vm_end || address < vma->vm_start) {
			vma = NULL;
		}

		if(!vma || (vma->vm_start != vaddr_start)
				|| (vma->vm_end != (vaddr_start + vaddr_size))) {

			spin_unlock(ptl);
			/*PTE UNLOCKED*/

			up_read(&mm->mmap_sem);

			struct file* f;

			f = filp_open(path, O_RDONLY | O_LARGEFILE, 0);

			down_write(&mm->mmap_sem);

			if(!IS_ERR(f)) {

				//check if other threads already installed the vma
				vma = find_vma(mm, address);
				if(!vma || address >= vma->vm_end
						|| address < vma->vm_start) {
					vma = NULL;
				}

				if(!vma || (vma->vm_start != vaddr_start)
						|| (vma->vm_end
								!= (vaddr_start
										+ vaddr_size))) {

					VMAPRINTK(
							"Mapping file vma start %lu end %lu\n",
							vaddr_start,
							(vaddr_start
									+ vaddr_size));

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
					err =
							map_difference(f,
									vaddr_start,
									vaddr_size,
									prot,
									MAP_FIXED
									| ((vm_flags
											& VM_DENYWRITE) ?
													MAP_DENYWRITE :
													0)
													| ((vm_flags
															& VM_EXECUTABLE) ?
																	MAP_EXECUTABLE :
																	0)
																	| ((vm_flags
																			& VM_SHARED) ?
																					MAP_SHARED :
																					MAP_PRIVATE)
																					| ((vm_flags
																							& VM_HUGETLB) ?
																									MAP_HUGETLB :
																									0),
																									pgoff
																									<< PAGE_SHIFT);

					current->mm->distribute_unmap = 1;

					if(err != vaddr_start) {
						up_write(&mm->mmap_sem);
						down_read(&mm->mmap_sem);
						spin_lock(ptl);
						/*PTE LOCKED*/
						printk(
								"ERROR: error mapping file vma while fetching address %lu in %s\n",
								address, __func__);
						ret = VM_FAULT_VMA;
						return ret;
					}
				}

			} else {
				up_write(&mm->mmap_sem);
				down_read(&mm->mmap_sem);
				spin_lock(ptl);
				/*PTE LOCKED*/
				printk("ERROR: error while opening file %s in %s\n",
						path, __func__);
				ret = VM_FAULT_VMA;
				return ret;
			}

			up_write(&mm->mmap_sem);

			filp_close(f, NULL);

			down_read(&mm->mmap_sem);
			spin_lock(ptl);
			/*PTE LOCKED*/
		}

	}

	return 0;

}

void add_fake_work_vma_op_wq(struct migration_memory* mm_data)
{
	struct vma_op_work* work = kmalloc(sizeof(*work), GFP_ATOMIC);

	if(work) {
		work->fake = 1;
		work->memory = mm_data;
		mm_data->arrived_op = 0;
		INIT_WORK((struct work_struct*)work, process_vma_op);
		queue_work(vma_op_wq, (struct work_struct*)work);
	} else {
		printk("Impossible to kmalloc in %s\n", __func__);
	}
}

void init_vma_operation_fields_mm_struct(struct mm_struct* mm)
{
	init_rwsem(&mm->distribute_sem);
	mm->distr_vma_op_counter = 0;
	mm->was_not_pushed = 0;
	mm->thread_op = NULL;
	mm->vma_operation_index = 0;
	mm->distribute_unmap = 1;
}

void init_vma_operation_fields_migration_memory_struct(
		struct migration_memory *entry)
{
	entry->operation = VMA_OP_NOP;
	entry->waiting_for_main = NULL;
	entry->waiting_for_op = NULL;
	entry->arrived_op = 0;
	entry->my_lock = 0;
}

int copy_vma_operation_index_for_new_kernel(int tgroup_home_cpu,
		int tgroup_home_id)
{
	int ret = -1;
	if(_cpu == tgroup_home_cpu) {
		struct migration_memory* memory =
				find_migration_memory_memory_entry(
						tgroup_home_cpu,
						tgroup_home_id);
		if(memory != NULL) {

			down_read(&memory->mm->mmap_sem);
			ret = memory->mm->vma_operation_index;
			up_read(&memory->mm->mmap_sem);

		}
	}
	return ret;
}

/* Called by the main kernel thread of a distributed process to check
 * if it has to execute a vma operation
 */
int check_pending_vma_operation(struct migration_memory *mm_data)
{
	return (mm_data->operation != VMA_OP_NOP
			&& mm_data->mm->thread_op == current);
}

/* Called by the main kernel thread of a distributed process to execute a
 * vma operation.
 */
void execute_pending_vma_operation(struct migration_memory *mm_data)
{
	unsigned long ret = 0;
	struct file* f;

	while(mm_data->operation != VMA_OP_NOP
			&& mm_data->mm->thread_op == current) {

		switch(mm_data->operation) {

		case VMA_OP_UNMAP:
			down_write(&mm_data->mm->mmap_sem);
			if(current->tgroup_home_cpu != _cpu)
				mm_data->mm->distribute_unmap = 0;
			ret = do_munmap(mm_data->mm, mm_data->addr,
					mm_data->len);
			if(current->tgroup_home_cpu != _cpu)
				mm_data->mm->distribute_unmap = 1;
			up_write(&mm_data->mm->mmap_sem);
			break;
		case VMA_OP_PROTECT:
			if(current->tgroup_home_cpu != _cpu)
				mm_data->mm->distribute_unmap = 0;
			ret = kernel_mprotect(mm_data->addr, mm_data->len,
					mm_data->prot);
			if(current->tgroup_home_cpu != _cpu)
				mm_data->mm->distribute_unmap = 1;
			break;
		case VMA_OP_REMAP:
			down_write(&mm_data->mm->mmap_sem);
			if(current->tgroup_home_cpu != _cpu)
				mm_data->mm->distribute_unmap = 0;
			ret = do_mremap(mm_data->addr, mm_data->len,
					mm_data->new_len, mm_data->flags,
					mm_data->new_addr);
			if(current->tgroup_home_cpu != _cpu)
				mm_data->mm->distribute_unmap = 1;
			up_write(&mm_data->mm->mmap_sem);
			break;
		case VMA_OP_BRK:
			ret = -1;
			down_write(&mm_data->mm->mmap_sem);
			if(current->tgroup_home_cpu != _cpu)
				mm_data->mm->distribute_unmap = 0;
			ret = do_brk(mm_data->addr, mm_data->len);
			if(current->tgroup_home_cpu != _cpu)
				mm_data->mm->distribute_unmap = 1;
			up_write(&mm_data->mm->mmap_sem);

			break;
		case VMA_OP_MAP:

			ret = -1;
			f = NULL;
			if(mm_data->path[0] != '\0') {

				f = filp_open(mm_data->path,
						O_RDONLY | O_LARGEFILE, 0);
				if(IS_ERR(f)) {
					printk(
							"ERROR: cannot open file to map\n");
					break;
				}

			}

			down_write(&mm_data->mm->mmap_sem);
			if(current->tgroup_home_cpu != _cpu)
				mm_data->mm->distribute_unmap = 0;
			ret = do_mmap_pgoff(f, mm_data->addr, mm_data->len,
					mm_data->prot, mm_data->flags,
					mm_data->pgoff);
			if(current->tgroup_home_cpu != _cpu)
				mm_data->mm->distribute_unmap = 1;
			up_write(&mm_data->mm->mmap_sem);

			if(mm_data->path[0] != '\0') {

				filp_close(f, NULL);

			}

			break;
		default:
			break;
		}
		mm_data->addr = ret;
		mm_data->operation = VMA_OP_NOP;

		wake_up_process(mm_data->waiting_for_main);
	}

}

/* Called to execute operation <operation>.
 * This function will wake up the main kernel thread of the distributed process
 * to execute the operation if it was initiated by a thread in another
 * kernel, the requesting thread of the operation in this kernel otherwise.
 *
 * NOTE: the current->mm->distribute_sem must be already held by a thread in vma_lock_wq
 */
static void process_vma_op_client(struct mm_struct* mm,
		struct migration_memory* memory,
		struct vma_operation* operation)
{

	VMAPRINTK(
			"%s, CLIENT: Starting operation %i of index %i\n ", __func__, operation->operation, operation->vma_operation_index);

	/*MMAP and BRK are not pushed in the system by the server,
	 *if I receive one of them some thread in this kernel must have initiate it*/
	if(operation->operation == VMA_OP_MAP
			|| operation->operation == VMA_OP_BRK) {

		if(memory->my_lock != 1) {
			printk(
					"ERROR: wrong distributed lock acquisition detected in %s\n",
					__func__);
			return;
		}

		if(operation->from_cpu != _cpu) {
			printk(
					"ERROR: the server pushed me an operation %i of cpu %i\n",
					operation->operation,
					operation->from_cpu);
			return;
		}

		if(memory->waiting_for_op == NULL) {
			printk(
					"ERROR: received a push operation started by this kernel but nobody is waiting\n");
			return;
		}

		memory->addr = operation->addr;
		memory->arrived_op = 1;
		VMAPRINTK(
				"CLIENT: vma_operation_index is %d\n", mm->vma_operation_index);
		VMAPRINTK(
				"%s, CLIENT: Operation %i started by a local thread pid %d\n ", __func__, operation->operation, memory->waiting_for_op->pid);

		wake_up_process(memory->waiting_for_op);

		return;

	}

	/*Also if not MMAP and BRK
	 *this kernel might have started the operation...check!
	 */
	if(operation->from_cpu == _cpu) {

		if(memory->my_lock != 1) {
			printk(
					"ERROR: wrong distributed lock acquisition detected in %s\n",
					__func__);
			return;
		}

		if(memory->waiting_for_op == NULL) {
			printk(
					"ERROR:received a push operation started by me but nobody is waiting\n");
			return;
		}

		if(operation->operation == VMA_OP_REMAP)
			memory->addr = operation->new_addr;

		memory->arrived_op = 1;
		VMAPRINTK(
				"%s, CLIENT: Operation %i started by a local thread pid %d\n ", __func__, operation->operation, memory->waiting_for_op->pid);
		VMAPRINTK(
				"CLIENT: vma_operation_index is %d\n", mm->vma_operation_index);

		wake_up_process(memory->waiting_for_op);

		return;
	}

	/* In this case is an operation started by a thread
	 * in another kernel.
	 */
	VMAPRINTK(
			"%s, CLIENT Pushed operation started by somebody else\n", __func__);

	if(operation->addr < 0) {
		printk("WARNING: %s, server sent me and error\n", __func__);
		return;
	}

	mm->distr_vma_op_counter++;
	struct task_struct *prev = mm->thread_op;

	while(memory->main == NULL)
		schedule();

	mm->thread_op = memory->main;
	up_write(&mm->mmap_sem);

	/*wake up the main kernel thread to execute the operation*/

	memory->addr = operation->addr;
	memory->len = operation->len;
	memory->prot = operation->prot;
	memory->new_addr = operation->new_addr;
	memory->new_len = operation->new_len;
	memory->flags = operation->flags;

	if(operation->operation == VMA_OP_REMAP)
		memory->flags |= MREMAP_FIXED;

	memory->waiting_for_main = current;

	/*This is the field check by the main thread
	 so it is the last one to be populated*/
	memory->operation = operation->operation;

	wake_up_process(memory->main);

	VMAPRINTK("%s,CLIENT: woke up the main\n", __func__);

	while(memory->operation != VMA_OP_NOP) {

		set_task_state(current, TASK_UNINTERRUPTIBLE);

		if(memory->operation != VMA_OP_NOP) {
			schedule();
		}

		set_task_state(current, TASK_RUNNING);

	}

	down_write(&mm->mmap_sem);
	memory->waiting_for_main = NULL;
	mm->thread_op = prev;
	mm->distr_vma_op_counter--;

	VMAPRINTK(
			"CLIENT: Incrementing vma_operation_index in %s\n", __func__);
	mm->vma_operation_index++;

	if(memory->my_lock != 1) {
		VMAPRINTK("Released distributed lock in %s\n", __func__);
		up_write(&mm->distribute_sem);
	}

	VMAPRINTK(
			"CLIENT: vma_operation_index is %d\n", mm->vma_operation_index);
	VMAPRINTK("CLIENT: Ending distributed vma operation\n");

	return;

}

/* Called to execute operation <operation>.
 * This function will wake up the main kernel thread of the distributed process
 * to execute it.
 *
 * NOTE: the current->mm->distribute_sem must be already held by a thread in vma_lock_wq
 */
static void process_vma_op_server(struct mm_struct* mm,
		struct migration_memory* memory,
		struct vma_operation* operation)
{

	/*if another vma operation is on going, it will be serialized after.*/
	while(mm->distr_vma_op_counter > 0) {

		VMAPRINTK(
				"%s, A distributed operation already started, going to sleep\n", __func__);
		up_write(&mm->mmap_sem);

		DEFINE_WAIT(wait);
		prepare_to_wait(&request_distributed_vma_op, &wait,
				TASK_UNINTERRUPTIBLE);

		if(mm->distr_vma_op_counter > 0) {
			schedule();
		}

		finish_wait(&request_distributed_vma_op, &wait);

		down_write(&mm->mmap_sem);

	}

	if(mm->distr_vma_op_counter != 0 || mm->was_not_pushed != 0) {
		return;
	}

	VMAPRINTK(
			"%s,SERVER: Starting operation %i for cpu %i\n", __func__, operation->operation, operation->header.from_cpu);

	mm->distr_vma_op_counter++;

	while(memory->main == NULL)
		schedule();

	mm->thread_op = memory->main;

	if(operation->operation == VMA_OP_MAP
			|| operation->operation == VMA_OP_BRK) {
		mm->was_not_pushed++;
	}

	up_write(&mm->mmap_sem);

	/*wake up the main kernel thread to execute the operation locally*/

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

	/*This is the field check by the main thread
	 so it is the last one to be populated*/
	memory->operation = operation->operation;

	wake_up_process(memory->main);

	VMAPRINTK("%s,SERVER: woke up the main\n", __func__);

	/*wait for the main kernel thread to finish*/

	while(memory->operation != VMA_OP_NOP) {

		set_task_state(current, TASK_UNINTERRUPTIBLE);

		if(memory->operation != VMA_OP_NOP) {
			schedule();
		}

		set_task_state(current, TASK_RUNNING);

	}

	down_write(&mm->mmap_sem);

	mm->distr_vma_op_counter--;
	if(mm->distr_vma_op_counter != 0)
		printk(
				"ERROR: exiting from distributed operation but mm->distr_vma_op_counter is %i \n",
				mm->distr_vma_op_counter);

	if(operation->operation == VMA_OP_MAP
			|| operation->operation == VMA_OP_BRK) {
		mm->was_not_pushed--;
		if(mm->was_not_pushed != 0)
			printk(
					"ERROR: exiting from distributed operation but mm->was_not_pushed is %i \n",
					mm->distr_vma_op_counter);
	}

	mm->thread_op = NULL;

	VMAPRINTK(
			"SERVER: vma_operation_index is %d\n", mm->vma_operation_index);
	VMAPRINTK(
			"%s, SERVER: end requested operation pid %d\n", __func__, current->pid);

	return;

}

void process_vma_op(struct work_struct* work)
{

	struct vma_op_work* vma_work = (struct vma_op_work*)work;
	struct migration_memory* memory = vma_work->memory;

	/* The main kernel thread may send me a fake work to be sure that all
	 * the works for its process are finished.
	 */
	if(vma_work->fake == 1) {
		unsigned long flags;
		memory->arrived_op = 1;

		lock_task_sighand(memory->main, &flags);
		memory->main->distributed_exit = EXIT_FLUSHING;
		unlock_task_sighand(memory->main, &flags);

		wake_up_process(memory->main);
		kfree(work);
		return;
	}

	struct mm_struct* mm = memory->mm;
	struct vma_operation* operation = vma_work->operation;

	VMAPRINTK(
			"Received vma operation from cpu %d for tgroup_home_cpu %i tgroup_home_id %i operation %i my pid is %d\n", operation->header.from_cpu, operation->tgroup_home_cpu, operation->tgroup_home_id, operation->operation, current->pid);

	down_write(&mm->mmap_sem);

	if(_cpu == operation->tgroup_home_cpu) {
		process_vma_op_server(mm, memory, operation);
	} else {
		process_vma_op_client(mm, memory, operation);
	}

	up_write(&mm->mmap_sem);

	wake_up(&request_distributed_vma_op);

	pcn_kmsg_free_msg_now(operation);
	kfree(work);

	return;

}

static int handle_vma_op(struct pcn_kmsg_message* inc_msg)
{

	struct vma_operation* operation = (struct vma_operation*)inc_msg;
	struct vma_op_work* work;

	VMAPRINTK("Received an operation\n");

	struct migration_memory* memory = find_migration_memory_memory_entry(
			operation->tgroup_home_cpu, operation->tgroup_home_id);

	if(memory != NULL) {

		work = kmalloc(sizeof(*work), GFP_ATOMIC);

		if(work) {
			work->fake = 0;
			work->memory = memory;
			work->operation = operation;
			INIT_WORK((struct work_struct*)work, process_vma_op);
			queue_work(vma_op_wq, (struct work_struct*)work);
		} else
			printk("Impossible to kmalloc in %s\n", __func__);

	} else {

		if(operation->tgroup_home_cpu == _cpu) {
			printk(
					"ERROR: received an operation that said that I am the server but no struct migration_memory found. This may be not an error but just an unlikely timing issue. The first migrating thread may have not already created struct migration_memory\n");
		} else {
			VMAPRINTK(
					"Received an operation for a distributed process not present here\n");
		}
		pcn_kmsg_free_msg_now(inc_msg);
	}

	return 1;

}

void add_vma_ack_entry(struct vma_op_answers* entry)
{
	struct vma_op_answers* curr;
	unsigned long flags;

	if(!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_vma_ack_head_lock, flags);

	if(!_vma_ack_head) {
		_vma_ack_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _vma_ack_head;
		while(curr->next != NULL) {
			curr = curr->next;
		}
		curr->next = entry;
		entry->next = NULL;
		entry->prev = curr;
	}

	raw_spin_unlock_irqrestore(&_vma_ack_head_lock, flags);
}

struct vma_op_answers* find_vma_ack_entry(int cpu, int id)
{
	struct vma_op_answers* curr = NULL;
	struct vma_op_answers* ret = NULL;

	unsigned long flags;
	raw_spin_lock_irqsave(&_vma_ack_head_lock, flags);

	curr = _vma_ack_head;
	while(curr) {

		if(curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			ret = curr;
			break;
		}

		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(&_vma_ack_head_lock, flags);

	return ret;
}

void remove_vma_ack_entry(struct vma_op_answers* entry)
{
	unsigned long flags;

	if(!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_vma_ack_head_lock, flags);

	if(_vma_ack_head == entry) {
		_vma_ack_head = entry->next;
	}

	if(entry->next) {
		entry->next->prev = entry->prev;
	}

	if(entry->prev) {
		entry->prev->next = entry->next;
	}

	entry->prev = NULL;
	entry->next = NULL;

	raw_spin_unlock_irqrestore(&_vma_ack_head_lock, flags);

}

static int handle_vma_ack(struct pcn_kmsg_message* inc_msg)
{
	struct vma_ack* ack = (struct vma_ack*)inc_msg;
	struct vma_op_answers* ack_holder;
	unsigned long flags;
	struct task_struct* task_to_wake_up = NULL;

	VMAPRINTK("Vma ack received from cpu %d\n", ack->header.from_cpu);

	ack_holder = find_vma_ack_entry(ack->tgroup_home_cpu,
			ack->tgroup_home_id);
	if(ack_holder) {

		raw_spin_lock_irqsave(&(ack_holder->lock), flags);

		ack_holder->responses++;

		ack_holder->address = ack->addr;

		if(ack_holder->vma_operation_index == -1)
			ack_holder->vma_operation_index =
					ack->vma_operation_index;
		else if(ack_holder->vma_operation_index
				!= ack->vma_operation_index)
			printk(
					"ERROR: receiving an ack vma for a different operation index in %s\n",
					__func__);

		if(ack_holder->responses >= ack_holder->expected_responses)
			task_to_wake_up = ack_holder->waiting;

		raw_spin_unlock_irqrestore(&(ack_holder->lock), flags);

		if(task_to_wake_up)
			wake_up_process(task_to_wake_up);

	}

	pcn_kmsg_free_msg(inc_msg);

	return 1;
}

/* Used to acquire in down_write distribute_sem lock.
 * Do not call this from the handler of the messaging layer because it may sleep.
 */
void process_vma_lock(struct work_struct* work)
{
	struct vma_lock_work* vma_lock_work = (struct vma_lock_work*)work;
	struct vma_lock* lock = vma_lock_work->lock;

	struct migration_memory* entry = find_migration_memory_memory_entry(
			lock->tgroup_home_cpu, lock->tgroup_home_id);

	if(entry != NULL) {

		while(entry->setting_up == 1)
			schedule();

		down_write(&entry->mm->distribute_sem);

		VMAPRINTK(
				"Acquired distributed lock for lock->tgroup_home_cpu {%d} tgroup_home_id {%d} \n", lock->tgroup_home_cpu, lock->tgroup_home_id);

		if(lock->from_cpu == _cpu)
			entry->my_lock = 1;
	} else
		printk("Entry not ready for hosting operations!! in %s\n",
				__func__);

	struct vma_ack* ack_to_server = pcn_kmsg_alloc_msg(
			sizeof(*ack_to_server));
	if(ack_to_server == NULL) {
		printk("Impossible to kmalloc in %s\n", __func__);
		return;
	}
	ack_to_server->tgroup_home_cpu = lock->tgroup_home_cpu;
	ack_to_server->tgroup_home_id = lock->tgroup_home_id;
	ack_to_server->vma_operation_index = lock->vma_operation_index;
	ack_to_server->header.type = PCN_KMSG_TYPE_VMA_ACK;
	ack_to_server->header.prio = PCN_KMSG_PRIO_NORMAL;

	pcn_kmsg_send_long(lock->tgroup_home_cpu,
			(struct pcn_kmsg_long_message*)(ack_to_server),
			sizeof(*ack_to_server) - sizeof(struct pcn_kmsg_hdr));

	pcn_kmsg_free_msg(ack_to_server);
	pcn_kmsg_free_msg_now(lock);
	kfree(work);

	return;

}

static int handle_vma_lock(struct pcn_kmsg_message* inc_msg)
{
	struct vma_lock* lock = (struct vma_lock*)inc_msg;
	struct vma_lock_work* work;

	work = kmalloc(sizeof(*work), GFP_ATOMIC);

	if(work) {
		work->lock = lock;
		INIT_WORK((struct work_struct*)work, process_vma_lock);
		queue_work(vma_lock_wq, (struct work_struct*)work);
	}

	else {
		printk("Impossible to kmalloc in %s\n", __func__);
		pcn_kmsg_free_msg_now(lock);
	}
	return 1;

}

/* Called by the server kernel when it is needed to send the operation
 * AFTER computing the parameters to force on clients.
 */
static void send_operation_with_saved_parameters(int operation, long start_ret,
		unsigned long addr, struct migration_memory* entry)
{

	int i;

	if(_cpu != current->tgroup_home_cpu)
		printk("ERROR: asking for saving address from a client in %s",
				__func__);

	if(entry->message_push_operation != NULL) {

		if(operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
			if(current->main == 0)
				printk(
						"ERROR: server not main asked to save operation %s\n",
						__func__);

			entry->message_push_operation->addr = addr;

		} else {

			if(operation == VMA_OP_REMAP) {
				entry->message_push_operation->new_addr = addr;
			} else {
				printk(
						"ERROR: asking for saving address from a wrong operation %s\n",
						__func__);
			}
		}

		up_write(&current->mm->mmap_sem);

		if(operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
			entry->message_push_operation->header.flag =
					PCN_KMSG_SYNC;
			if(pcn_kmsg_send_long(
					entry->message_push_operation->from_cpu,
					(struct pcn_kmsg_long_message*)(entry->message_push_operation),
					sizeof(*entry->message_push_operation)
							- sizeof(struct pcn_kmsg_hdr))
					== -1) {
				printk(
						"ERROR: %s impossible to send operation to client in cpu %d\n",
						__func__,
						entry->message_push_operation->from_cpu);
			} else {
				VMAPRINTK(
						"%s, operation %d sent to cpu %d \n", __func__, operation, entry->message_push_operation->from_cpu);
			}

		} else {
			VMAPRINTK(
					"%s,sending operation %d to all \n", __func__, operation);

			struct list_head *iter;
			_remote_cpu_info_list_t *objPtr;
			list_for_each(iter, &rlist_head)
			{
				objPtr = list_entry(iter,
						_remote_cpu_info_list_t,
						cpu_list_member);
				i = objPtr->_data._processor;

				entry->message_push_operation->header.flag =
						PCN_KMSG_SYNC;
				if(entry->kernel_set[i] == 1)
					pcn_kmsg_send_long(i,
							(struct pcn_kmsg_long_message*)(entry->message_push_operation),
							sizeof(*entry->message_push_operation)
									- sizeof(struct pcn_kmsg_hdr));

			}
		}

		down_write(&current->mm->mmap_sem);

		if(current->main == 0) {
			pcn_kmsg_free_msg_now(entry->message_push_operation);
			entry->message_push_operation = NULL;
		}

	} else {
		printk(
				"ERROR: Cannot find message to send in exit operation %s\n",
				__func__);

	}
}

/* I assume that down_write(&mm->mmap_sem) is held.
 * This function correctly ends a previously started distribute operation.
 */
static void end_distribute_operation(int operation, long start_ret,
		unsigned long addr)
{
	if(current->mm->distribute_unmap == 0) {
		return;
	}

	VMAPRINTK(
			"Ending distributed vma operation %i pid %d in %s\n", operation, current->pid, __func__);

	if(current->mm->distr_vma_op_counter <= 0
			|| (current->main == 0
					&& current->mm->distr_vma_op_counter > 2)
			|| (current->main == 1
					&& current->mm->distr_vma_op_counter > 3))
		printk(
				"ERROR: exiting from a distributed vma operation with distr_vma_op_counter = %i in %s\n",
				current->mm->distr_vma_op_counter, __func__);

	(current->mm->distr_vma_op_counter)--;

	if(operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
		if(current->mm->was_not_pushed <= 0)
			printk(
					"ERROR: exiting from a mapping operation with was_not_pushed = %i in %s\n",
					current->mm->was_not_pushed, __func__);
		current->mm->was_not_pushed--;
	}

	struct migration_memory* entry = find_migration_memory_memory_entry(
			current->tgroup_home_cpu, current->tgroup_home_id);

	if(entry == NULL) {
		printk("ERROR: Cannot find struct migration_memory in %s\n",
				__func__);
	}

	if(start_ret == VMA_OP_SAVE) {

		send_operation_with_saved_parameters(operation, start_ret, addr,
				entry);

	}

	if(current->mm->distr_vma_op_counter == 0) {

		if(current->main == 1)
			printk("ERROR count is 0 but I am main in %s",
					__func__);

		current->mm->thread_op = NULL;

		entry->my_lock = 0;

		if(!(operation == VMA_OP_MAP || operation == VMA_OP_BRK)) {
			VMAPRINTK(
					"%s incrementing vma_operation_index\n", __func__);
			current->mm->vma_operation_index++;
		}

		VMAPRINTK("Releasing distributed lock in %s\n", __func__);

		up_write(&current->mm->distribute_sem);

		if(_cpu == current->tgroup_home_cpu
				&& !(operation == VMA_OP_MAP
						|| operation == VMA_OP_BRK)) {
			up_read(&entry->kernel_set_sem);
		}

		wake_up(&request_distributed_vma_op);

	} else if(_cpu == current->tgroup_home_cpu && current->main == 1) {

		/*case server main*/

		if(current->mm->distr_vma_op_counter == 1) {
			/*case server main not nested*/

			if(!(operation == VMA_OP_MAP || operation == VMA_OP_BRK)) {
				VMAPRINTK(
						"%s incrementing vma_operation_index\n", __func__);
				current->mm->vma_operation_index++;
				up_read(&entry->kernel_set_sem);
			}

			VMAPRINTK(
					"Releasing distributed lock in %s\n", __func__);

			up_write(&current->mm->distribute_sem);
		}

		if(current->mm->distr_vma_op_counter == 2) {
			/*case server main nested*/

			if(operation != VMA_OP_UNMAP)
				printk(
						"ERROR exiting from a nest operation that is %d in %s",
						operation, __func__);

			/*it can be nested after map or br
			 or nested after remap*/
			if(current->mm->was_not_pushed > 0) {
				VMAPRINTK(
						"%s incrementing vma_operation_index\n", __func__);
				current->mm->vma_operation_index++;
				up_read(&entry->kernel_set_sem);
			} else {
				VMAPRINTK(
						"%s ending nested operation after remap\n", __func__);
			}
		}

	} else {

		if(_cpu == current->tgroup_home_cpu) {

			/*case server not main*/

			if(operation != VMA_OP_UNMAP)
				printk(
						"ERROR exiting fom a nest operation that is %d",
						operation);

			if(current->mm->was_not_pushed > 0) {

				VMAPRINTK(
						"%s incrementing vma_operation_index\n", __func__);
				current->mm->vma_operation_index++;
				VMAPRINTK(
						"%s relising kernel_set_sem lock\n", __func__);
				up_read(&entry->kernel_set_sem);
			} else {
				VMAPRINTK(
						"%s ending nested operation after remap\n", __func__);
			}

		} else {
			/*case client*/

			if(current->main != 1) {

				/*case client not main*/

				VMAPRINTK(
						"%s ending nested operation after remap\n", __func__);
				if(current->mm->was_not_pushed > 0) {

					printk(
							"ERROR not  client main with count > 0 and was_not_pushed > 0 %s",
							__func__);
				}

			} else {
				/*case client main*/
				/*this should be executed with distribute_unmap == 0
				 =>so should not be here*/
				printk(
						"ERROR  client main with count > 0 in %s",
						__func__);
			}

		}

	}

	VMAPRINTK(
			"operation index is %d\n", current->mm->vma_operation_index);

}

/* Start a distribute operation on a client kernel.
 * This is a three phases protocol:
 * 1. send the operation to the server.
 * 2. wait for the server to send a lock message
 * NOTE this step is handled by another thread, not this function.
 * 3. wait for the server to send back my operation, and execute it locally.
 */
static long start_operation_client(int operation, unsigned long addr,
		size_t len, unsigned long prot, unsigned long new_addr,
		unsigned long new_len, unsigned long flags, struct file *file,
		unsigned long pgoff)
{

	long ret;
	if(operation == VMA_OP_REMAP)
		ret = new_addr;
	else
		ret = addr;

	VMAPRINTK(
			"CLIENT: starting operation %i for pid %d current index is%d\n", operation, current->pid, current->mm->vma_operation_index);

	/*First: send the operation to the server*/
	struct vma_operation* operation_to_send = pcn_kmsg_alloc_msg(
			sizeof(*operation_to_send));
	if(operation_to_send == NULL) {
		printk("Impossible to malloc in %s\n", __func__);
		ret = -ENOMEM;
		goto out;
	}
	operation_to_send->header.type = PCN_KMSG_TYPE_VMA_OP;
	operation_to_send->header.prio = PCN_KMSG_PRIO_NORMAL;
	operation_to_send->header.flag = PCN_KMSG_SYNC;

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
	if(file != NULL) {
		char path[256] = {0};
		char* rpath;
		rpath = d_path(&file->f_path, path, 256);
		strcpy(operation_to_send->path, rpath);
	} else
		operation_to_send->path[0] = '\0';

	/* The server will eventually send me the operation back.
	 * Update struct migration_memory with the info that I am the thread waiting for
	 * that operation.
	 */
	struct migration_memory* entry = find_migration_memory_memory_entry(
			current->tgroup_home_cpu, current->tgroup_home_id);
	if(entry) {

		if(entry->waiting_for_op != NULL) {
			printk(
					"ERROR: Somebody is already waiting for an op\n");
			pcn_kmsg_free_msg_now(operation_to_send);
			ret = -EPERM;
			goto out;
		}

		entry->waiting_for_op = current;
		entry->arrived_op = 0;

	} else {
		printk(
				"ERROR: Mapping disappeared, cannot wait for push op in %s\n",
				__func__);
		pcn_kmsg_free_msg_now(operation_to_send);
		ret = -EPERM;
		goto out;
	}

	up_write(&current->mm->mmap_sem);

	int error;

	error = pcn_kmsg_send_long(current->tgroup_home_cpu,
			(struct pcn_kmsg_long_message*)(operation_to_send),
			sizeof(*operation_to_send)
					- sizeof(struct pcn_kmsg_hdr));

	if(error == -1) {
		printk("Impossible to contact the server in %s", __func__);
		pcn_kmsg_free_msg_now(operation_to_send);
		down_write(&current->mm->mmap_sem);
		ret = -EPERM;
		goto out;
	}

	/*Second: the server will send me a LOCK message... another thread will handle it...*/

	/*Third: wait that the server push back the operation*/
	while(entry->arrived_op == 0) {

		set_task_state(current, TASK_UNINTERRUPTIBLE);

		if(entry->arrived_op == 0) {
			schedule();
		}

		set_task_state(current, TASK_RUNNING);
	}

	VMAPRINTK(
			"My operation finally arrived pid %d vma operation %d\n", current->pid, current->mm->vma_operation_index);

	/*Note, the distributed lock already has been acquired*/
	down_write(&current->mm->mmap_sem);

	if(current->mm->thread_op != current) {
		printk(
				"ERROR: waking up to locally execute a vma operation started by me, but thread_op s not me\n");
		pcn_kmsg_free_msg_now(operation_to_send);
		ret = -EPERM;
		goto out_dist_lock;
	}

	if(operation == VMA_OP_REMAP || operation == VMA_OP_MAP
			|| operation == VMA_OP_BRK) {
		ret = entry->addr;
		if(entry->addr < 0) {
			printk(
					"Received error %lu from the server for operation %d\n",
					ret, operation);
			goto out_dist_lock;
		}
	}

	entry->waiting_for_op = NULL;

	pcn_kmsg_free_msg_now(operation_to_send);

	out_dist_lock:

	up_write(&current->mm->distribute_sem);
	VMAPRINTK("Released distributed lock from out_dist_lock\n");
	VMAPRINTK(
			"current index is %d in out_dist_lock\n", current->mm->vma_operation_index);

	out: current->mm->distr_vma_op_counter--;
	current->mm->thread_op = NULL;
	if(operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
		current->mm->was_not_pushed--;
	}

	wake_up(&request_distributed_vma_op);
	return ret;

}

/* Start a distribute operation on the server kernel.
 * This operation can be started by a thread running on this kernel,
 * or can be requested by a client kernel.
 * In the second case this function is executed by the main kernel thread of the distributed process.
 *
 * It may be not necessary to distribute the operation.
 * When distributing the operation three steps are required:
 * 1. send a lock message
 * 2. wait for the acks
 * 3. send the operation.
 * The number of kernels contacted depends on the operation.
 *
 */
static long start_operation_server(int operation, unsigned long addr,
		size_t len, unsigned long prot, unsigned long new_addr,
		unsigned long new_len, unsigned long flags, struct file *file,
		unsigned long pgoff)
{

	long ret = VMA_OP_NOT_SAVE;

	if(current->main == 1 && !(current->mm->distr_vma_op_counter > 2)) {

		/*I am the main kernel thread without recursive operation=>
		 *a client asked me to do an operation.
		 */
		int index = current->mm->vma_operation_index;

		VMAPRINTK(
				"SERVER MAIN: starting operation %d, current index is %d \n", operation, index);

		up_write(&current->mm->mmap_sem);

		struct migration_memory* entry =
				find_migration_memory_memory_entry(
						current->tgroup_home_cpu,
						current->tgroup_home_id);

		if(entry == NULL || entry->message_push_operation == NULL) {
			printk(
					"ERROR: Mapping disappeared or cannot find message to update \n");
			down_write(&current->mm->mmap_sem);
			ret = -ENOMEM;
			goto out;
		}

		/*First: send a message to everybody to acquire the lock to block page faults*/
		struct vma_lock* lock_message = pcn_kmsg_alloc_msg(
				sizeof(*lock_message));
		if(lock_message == NULL) {
			printk("Impossible to kmalloc in %s\n", __func__);
			down_write(&current->mm->mmap_sem);
			ret = -ENOMEM;
			goto out;
		}
		lock_message->header.type = PCN_KMSG_TYPE_VMA_LOCK;
		lock_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		lock_message->header.flag = PCN_KMSG_SYNC;
		lock_message->tgroup_home_cpu = current->tgroup_home_cpu;
		lock_message->tgroup_home_id = current->tgroup_home_id;
		lock_message->from_cpu =
				entry->message_push_operation->from_cpu;
		lock_message->vma_operation_index = index;

		struct vma_op_answers* acks = kmalloc(sizeof(*acks),
				GFP_ATOMIC);
		if(acks == NULL) {
			printk("Impossible to kmalloc in %s\n", __func__);
			pcn_kmsg_free_msg_now(lock_message);
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

		/* mmap and brk need to communicate only between server and one client
		 * => do not send the message to all kernels.
		 */
		if(operation == VMA_OP_MAP || operation == VMA_OP_BRK) {

			error =
					pcn_kmsg_send_long(
							entry->message_push_operation->from_cpu,
							(struct pcn_kmsg_long_message*)(lock_message),
							sizeof(*lock_message)
									- sizeof(struct pcn_kmsg_hdr));
			if(error != -1) {
				acks->expected_responses++;
			}

		} else {

			down_read(&entry->kernel_set_sem);

			struct list_head *iter;
			_remote_cpu_info_list_t *objPtr;
			list_for_each(iter, &rlist_head)
			{
				objPtr = list_entry(iter,
						_remote_cpu_info_list_t,
						cpu_list_member);
				i = objPtr->_data._processor;

				if(entry->kernel_set[i] == 1) {

					error =
							pcn_kmsg_send_long(i,
									(struct pcn_kmsg_long_message*)(lock_message),
									sizeof(*lock_message)
											- sizeof(struct pcn_kmsg_hdr));
					if(error != -1) {
						acks->expected_responses++;
					}
				}
			}
		}

		while(acks->expected_responses != acks->responses) {

			set_task_state(current, TASK_UNINTERRUPTIBLE);

			if(acks->expected_responses != acks->responses) {
				schedule();
			}

			set_task_state(current, TASK_RUNNING);

		}

		VMAPRINTK("SERVER MAIN: Received all ack to lock\n");

		unsigned long flags;
		raw_spin_lock_irqsave(&(acks->lock), flags);
		raw_spin_unlock_irqrestore(&(acks->lock), flags);

		remove_vma_ack_entry(acks);

		entry->message_push_operation->vma_operation_index = index;

		/*I acquire the lock to block page faults too
		 *Important: this should happen before sending the operation message or executing the operation*/
		if(current->mm->distr_vma_op_counter == 2) {
			down_write(&current->mm->distribute_sem);
			VMAPRINTK("local distributed lock acquired 5\n");
		}

		/* Third: push the operation to everybody
		 * If the operation was a mmap,brk or remap without fixed parameters, I cannot let other kernels
		 * locally choose where to map it =>
		 * The server has to execute it before and pushes the parameters that it chose.
		 * */
		if(operation == VMA_OP_UNMAP || operation == VMA_OP_PROTECT
				|| ((operation == VMA_OP_REMAP)
						&& (flags & MREMAP_FIXED))) {

			entry->message_push_operation->header.flag =
					PCN_KMSG_SYNC;
			VMAPRINTK(
					"SERVER MAIN: sending done for operation, we can execute the operation in parallel! %d\n", operation);

			struct list_head *iter;
			_remote_cpu_info_list_t *objPtr;
			list_for_each(iter, &rlist_head)
			{
				objPtr = list_entry(iter,
						_remote_cpu_info_list_t,
						cpu_list_member);
				i = objPtr->_data._processor;

				entry->message_push_operation->header.flag =
						PCN_KMSG_SYNC;
				if(entry->kernel_set[i] == 1)
					pcn_kmsg_send_long(i,
							(struct pcn_kmsg_long_message*)(entry->message_push_operation),
							sizeof(*(entry->message_push_operation))
									- sizeof(struct pcn_kmsg_hdr));

			}

			pcn_kmsg_free_msg_now(lock_message);
			kfree(acks);

			down_write(&current->mm->mmap_sem);

			return ret;

		} else {
			VMAPRINTK(
					"SERVER MAIN: going to execute the operation locally %d\n", operation);
			pcn_kmsg_free_msg_now(lock_message);
			kfree(acks);

			down_write(&current->mm->mmap_sem);

			return VMA_OP_SAVE;

		}

	} else {

		/*server not main kernel thread or server main kernel thread with recursive operation
		 */
		VMAPRINTK(
				"SERVER NOT MAIN OR RECURSIVE: Starting operation %d for pid %d current index is %d\n", operation, current->pid, current->mm->vma_operation_index);

		switch(operation) {

		case VMA_OP_MAP:
		case VMA_OP_BRK:

			/*the server can execute mmap and brk locally
			 */
			VMAPRINTK("%s pure local operation!\n", __func__);

			up_write(&current->mm->mmap_sem);

			down_write(&current->mm->distribute_sem);
			VMAPRINTK("Distributed lock acquired 6\n");
			down_write(&current->mm->mmap_sem);

			return ret;

		default:
			break;

		}

		VMAPRINTK("%s push operation!\n", __func__);

		int index = current->mm->vma_operation_index;
		VMAPRINTK("current index is %d\n", index);

		up_write(&current->mm->mmap_sem);

		/*First: send a message to everybody to acquire the lock to block page faults*/
		struct vma_lock* lock_message = pcn_kmsg_alloc_msg(
				sizeof(*lock_message));
		if(lock_message == NULL) {
			printk("Impossible to kmalloc in %s\n", __func__);
			down_write(&current->mm->mmap_sem);
			ret = -ENOMEM;
			goto out;
		}
		lock_message->header.type = PCN_KMSG_TYPE_VMA_LOCK;
		lock_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		lock_message->header.flag = PCN_KMSG_SYNC;
		lock_message->tgroup_home_cpu = current->tgroup_home_cpu;
		lock_message->tgroup_home_id = current->tgroup_home_id;
		lock_message->from_cpu = _cpu;
		lock_message->vma_operation_index = index;

		struct vma_op_answers* acks = kmalloc(sizeof(*acks),
				GFP_ATOMIC);
		if(acks == NULL) {
			printk("Impossible to kmalloc in %s\n", __func__);
			pcn_kmsg_free_msg_now(lock_message);
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

		struct migration_memory* entry =
				find_migration_memory_memory_entry(
						current->tgroup_home_cpu,
						current->tgroup_home_id);
		if(entry == NULL) {
			printk(
					"ERROR: Mapping disappeared, cannot save message to update by exit_distribute_operation\n");
			pcn_kmsg_free_msg_now(lock_message);
			kfree(acks);
			down_write(&current->mm->mmap_sem);
			ret = -EPERM;
			goto out;
		}

		add_vma_ack_entry(acks);

		down_read(&entry->kernel_set_sem);

		struct list_head *iter;
		_remote_cpu_info_list_t *objPtr;
		list_for_each(iter, &rlist_head)
		{
			objPtr = list_entry(iter, _remote_cpu_info_list_t,
					cpu_list_member);
			i = objPtr->_data._processor;

			if(entry->kernel_set[i] == 1) {
				/*if this is a recursive operation of the main kenrel thread
				 * started by another kernel (current->mm->distr_vma_op_counter>2)
				 * do not send the lock message again to the requester*/
				if(!((current->main == 1
						&& (current->mm->distr_vma_op_counter
								> 2))
						&& i
								== entry->message_push_operation->from_cpu)) {
					error =
							pcn_kmsg_send_long(i,
									(struct pcn_kmsg_long_message*)(lock_message),
									sizeof(*lock_message)
											- sizeof(struct pcn_kmsg_hdr));
					if(error != -1) {
						acks->expected_responses++;

					}
				}
			}
		}

		/*Second: wait that everybody acquire the lock, and acquire it locally too*/
		while(acks->expected_responses != acks->responses) {

			set_task_state(current, TASK_UNINTERRUPTIBLE);

			if(acks->expected_responses != acks->responses) {
				schedule();

			}

			set_task_state(current, TASK_RUNNING);

		}

		VMAPRINTK("SERVER NOT MAIN: Received all ack to lock\n");

		unsigned long flags;
		raw_spin_lock_irqsave(&(acks->lock), flags);
		raw_spin_unlock_irqrestore(&(acks->lock), flags);

		remove_vma_ack_entry(acks);

		struct vma_operation* operation_to_send = pcn_kmsg_alloc_msg(
				sizeof(*operation_to_send));
		if(operation_to_send == NULL) {
			printk("Impossible to kmalloc in %s\n", __func__);
			down_write(&current->mm->mmap_sem);
			up_read(&entry->kernel_set_sem);
			pcn_kmsg_free_msg_now(lock_message);
			kfree(acks);
			ret = -ENOMEM;
			goto out;
		}

		operation_to_send->header.type = PCN_KMSG_TYPE_VMA_OP;
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
		operation_to_send->header.flag = PCN_KMSG_SYNC;

		/*I acquire the lock to block page faults too
		 *Important: this should happen before sending the operation message or executing the operation*/
		if(current->mm->distr_vma_op_counter == 1) {
			down_write(&current->mm->distribute_sem);
			VMAPRINTK("Distributed lock acquired locally 7\n");
		}

		/* Third: push the operation to everybody
		 * If the operation was a remap without fixed parameters, I cannot let other kernels
		 * locally choose where to remap it =>
		 * I need to push what the server chose as parameter to the other an push the operation with
		 * a fixed flag.
		 * */
		if(!(operation == VMA_OP_REMAP) || (flags & MREMAP_FIXED)) {

			VMAPRINTK(
					"SERVER : sending done for operation, we can execute the operation in parallel! %d\n", operation);

			struct list_head *iter;
			_remote_cpu_info_list_t * objPtr;
			list_for_each(iter, &rlist_head)
			{
				objPtr = list_entry(iter,
						_remote_cpu_info_list_t,
						cpu_list_member);
				i = objPtr->_data._processor;

				if(entry->kernel_set[i] == 1)
					pcn_kmsg_send_long(i,
							(struct pcn_kmsg_long_message*)(operation_to_send),
							sizeof(*operation_to_send)
									- sizeof(struct pcn_kmsg_hdr));

			}

			pcn_kmsg_free_msg_now(lock_message);
			pcn_kmsg_free_msg_now(operation_to_send);
			kfree(acks);

			down_write(&current->mm->mmap_sem);

			return ret;

		} else {
			VMAPRINTK(
					"SERVER : going to execute the operation locally %d\n", operation);
			entry->message_push_operation = operation_to_send;

			pcn_kmsg_free_msg_now(lock_message);
			kfree(acks);

			down_write(&current->mm->mmap_sem);

			return VMA_OP_SAVE;
		}

	}

	out: current->mm->distr_vma_op_counter--;
	current->mm->thread_op = NULL;
	if(operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
		current->mm->was_not_pushed--;
	}

	wake_up(&request_distributed_vma_op);

	return ret;

}

/*I assume that down_write(&mm->mmap_sem) is held.
 *In popcorn vma operations have to be propagated to the kernels
 *where the distributed process exists.
 *There is a server kernel that has a complete view of all the vmas for that process.
 *Because vmas are piggybacked during page fault, and not during the migration,
 *clients have partial views of the vmas.
 *For optimization, there are two different protocols:
 *mmap and brk need only to notify the server of the operation,
 *remap, mprotect and unmap need that the server pushes them also to all other clients.
 */
static long start_distribute_operation(int operation, unsigned long addr,
		size_t len, unsigned long prot, unsigned long new_addr,
		unsigned long new_len, unsigned long flags, struct file *file,
		unsigned long pgoff)
{

	long ret;
	int server;

	if(current->tgroup_home_cpu != _cpu)
		server = 0;
	else
		server = 1;

	if(server)
		ret = VMA_OP_NOT_SAVE;
	else if(operation == VMA_OP_REMAP)
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
	if(current->mm->distribute_unmap == 0) {
		return ret;
	}

	VMAPRINTK(
			"%s, Starting vma operation for pid %i tgroup_home_cpu %i tgroup_home_id %i main %d operation %i addr %lu len %lu end %lu\n", __func__, current->pid, current->tgroup_home_cpu, current->tgroup_home_id, current->main?1:0, operation, addr, len, addr+len);

	/*only server can (maybe) distribute nested operations
	 *so check if I have to go local and skip the distribution part*/
	if(current->mm->distr_vma_op_counter > 0
			&& current->mm->thread_op == current) {

		if(current->main == 0
				&& current->mm->distr_vma_op_counter > 1) {
			printk("ERROR: invalid nested vma operation %i\n",
					operation);
			return -EPERM;
		} else
		/*the main kernel thread executes the operations for the clients
		 *distr_vma_op_counter is already increased when it start the operation*/
		if(current->main == 1) {

			if(server == 0) {
				/*note that the process_vma_op set distributed unmap to 0 when servers pushes operations
				 * => clients cannot be here*/
				printk(
						"ERROR: invalid nested vma operation in main client\n");
				return -EPERM;
			}

			if(current->mm->distr_vma_op_counter > 2
					|| (current->mm->distr_vma_op_counter
							== 2
							&& operation
									!= VMA_OP_UNMAP)) {
				printk(
						"ERROR: invalid nested vma operation in main server\n");
				return -EPERM;
			} else if(current->mm->distr_vma_op_counter == 2) {

				VMAPRINTK(
						"%s, Recursive operation for the main\n", __func__);
				/* in this case is a nested operation on main
				 * if the previous operation was a pushed operation
				 * do not distribute it again*/
				if(current->mm->was_not_pushed == 0) {
					/*for this case
					 *do not increase era after (in end_distribute_operation)
					 *potentially server and clients have different numbers of unmaps
					 */
					current->mm->distr_vma_op_counter++;
					VMAPRINTK(
							"%s, don't ditribute again, return!\n", __func__);
					return ret;
				} else
					goto start;
			} else
				goto start;
		} else {

			if(operation != VMA_OP_UNMAP) {
				printk(
						"ERROR: invalid nested vma operation in main server\n");
				return -EPERM;
			}

			if(server == 1) {
				/*same as server main if it was MAP or BRK this unmap must be distributed*/
				if(current->mm->was_not_pushed == 0) {
					current->mm->distr_vma_op_counter++;
					VMAPRINTK(
							"%s, don't ditribute again, return!\n", __func__);
					return ret;
				} else
					goto start;
			} else {
				/*clients never distributes nested operations*/
				current->mm->distr_vma_op_counter++;
				VMAPRINTK(
						"%s, don't ditribute again, return!\n", __func__);
				return ret;
			}
		}
	}

	/* Check if another thread of my process already started an operation...
	 * => no concurrent operations of the same process on the same kernel*/
	while(current->mm->distr_vma_op_counter > 0) {

		VMAPRINTK(
				"%s Somebody already started a distributed operation (current->mm->thread_op->pid is %d). I am pid %d and I am going to sleep\n", __func__, current->mm->thread_op->pid, current->pid);

		up_write(&current->mm->mmap_sem);

		DEFINE_WAIT(wait);
		prepare_to_wait(&request_distributed_vma_op, &wait,
				TASK_UNINTERRUPTIBLE);

		if(current->mm->distr_vma_op_counter > 0) {
			schedule();
		}

		finish_wait(&request_distributed_vma_op, &wait);

		down_write(&current->mm->mmap_sem);

	}

	if(current->mm->distr_vma_op_counter != 0) {
		printk(
				"ERROR: a new vma operation can be started, but distr_vma_op_counter is %i\n",
				current->mm->distr_vma_op_counter);
		return -EPERM;
	}

	start: current->mm->distr_vma_op_counter++;
	current->mm->thread_op = current;

	if(operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
		current->mm->was_not_pushed++;
	}

	if(server) {
		return start_operation_server(operation, addr, len, prot,
				new_addr, new_len, flags, file, pgoff);
	} else {
		return start_operation_client(operation, addr, len, prot,
				new_addr, new_len, flags, file, pgoff);
	}

}

long popcorn_do_unmap_start(struct mm_struct *mm, unsigned long start,
		size_t len)
{

	return start_distribute_operation(VMA_OP_UNMAP, start, len, 0, 0, 0, 0,
			NULL, 0);

}

long popcorn_do_unmap_end(struct mm_struct *mm, unsigned long start,
		size_t len, int start_ret)
{
	end_distribute_operation(VMA_OP_UNMAP, start_ret, start);
	return 0;
}

long popcorn_mprotect_start(unsigned long start, size_t len,
		unsigned long prot)
{

	return start_distribute_operation(VMA_OP_PROTECT, start, len, prot, 0,
			0, 0, NULL, 0);

}

long popcorn_mprotect_end(unsigned long start, size_t len,
		unsigned long prot, int start_ret)
{

	end_distribute_operation(VMA_OP_PROTECT, start_ret, start);
	return 0;
}

long popcorn_do_mmap_pgoff_start(struct file *file, unsigned long addr,
		unsigned long len, unsigned long prot, unsigned long flags,
		unsigned long pgoff)
{

	return start_distribute_operation(VMA_OP_MAP, addr, len, prot, 0, 0,
			flags, file, pgoff);

}

long popcorn_do_mmap_pgoff_end(struct file *file, unsigned long addr,
		unsigned long len, unsigned long prot, unsigned long flags,
		unsigned long pgoff, unsigned long start_ret)
{

	end_distribute_operation(VMA_OP_MAP, start_ret, addr);
	return 0;

}

long popcorn_do_brk_start(unsigned long addr, unsigned long len)
{

	return start_distribute_operation(VMA_OP_BRK, addr, len, 0, 0, 0, 0,
			NULL, 0);

}

long popcorn_do_brk_end(unsigned long addr, unsigned long len,
		unsigned long start_ret)
{

	end_distribute_operation(VMA_OP_BRK, start_ret, addr);
	return 0;

}

long popcorn_do_mremap_start(unsigned long addr, unsigned long old_len,
		unsigned long new_len, unsigned long flags,
		unsigned long new_addr)
{

	return start_distribute_operation(VMA_OP_REMAP, addr, (size_t)old_len,
			0, new_addr, new_len, flags, NULL, 0);

}

long popcorn_do_mremap_end(unsigned long addr, unsigned long old_len,
		unsigned long new_len, unsigned long flags,
		unsigned long new_addr, unsigned long start_ret)
{

	end_distribute_operation(VMA_OP_REMAP, start_ret, new_addr);
	return 0;

}

static int __init popcorn_vma_operation_init(void) {

	uint16_t copy_cpu;
	if(scif_get_nodeIDs(NULL, 0, &copy_cpu)==-1){
		printk("ERROR %s cannot initialize _cpu\n", __func__);
	}
	else {
		_cpu= copy_cpu;
	}

	vma_op_wq= create_singlethread_workqueue("vma_op_wq");
	vma_lock_wq= create_workqueue("vma_lock_wq");

	/*
	 * Register to receive relevant incoming messages.
	 */
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_VMA_OP,
			handle_vma_op);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_VMA_ACK,
			handle_vma_ack);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_VMA_LOCK,
			handle_vma_lock);

	return 0;
}

late_initcall_popcorn( popcorn_vma_operation_init);
