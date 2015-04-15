/*
 * popcorn_user_dsm.c
 *
 * Author: Marina Sadini, SSRG Virginia Tech
 */

#include <linux/popcorn_user_dsm.h>
#include <linux/popcorn_migration.h>
#include <linux/kthread.h>
#include <linux/export.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/sched.h>
#include <linux/threads.h>
#include <linux/kmod.h>
#include <linux/path.h>
#include <linux/mount.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/mman.h>
#include <linux/pcn_kmsg.h>
#include <linux/highmem.h>
#include <linux/memcontrol.h>
#include <linux/pagemap.h>
#include <linux/mmu_notifier.h>
#include <linux/swap.h>
#include <linux/proc_fs.h>
#include <asm/traps.h>			/* dotraplinkage, ...		*/
#include <asm/pgalloc.h>		/* pgd_*(), ...			*/
#include <asm/kmemcheck.h>		/* kmemcheck_*(), ...		*/
#include <asm/fixmap.h>			/* VSYSCALL_START		*/

#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#include <asm/prctl.h>
#include <asm/proto.h>
#include <asm/msr.h>
#include <asm/page.h>
#include <linux/rmap.h>
#include <linux/memcontrol.h>
#include <asm/atomic.h>
#include <linux/syscalls.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <net/checksum.h>
#include <linux/fsnotify.h>
#include <linux/unistd.h>
#include <asm/mmu_context.h>
#include <linux/tsacct_kern.h>
#include <asm/uaccess.h>
#include <linux/popcorn_cpuinfo.h>
#include <asm/i387.h>
#include <linux/cpu_namespace.h>
#include "WKdm.h"
#include <linux/popcorn_migration.h>
#include <linux/popcorn_vma_operation.h>
#include <linux/futex.h>
#include <linux/fcntl.h>
#include "futex_remote.h"

#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

/* Workqueues to dispatch incoming messages.
 * Used to remove part of the work from the messaging layer handlers.
 */
static struct workqueue_struct *message_request_wq;
static struct workqueue_struct *invalid_message_wq;

/* List used to store information about on-going page-faults
 */
DEFINE_RAW_SPINLOCK(_mapping_head_lock);
struct page_fault_mapping* _mapping_head = NULL;
/* Functions to add,find and remove an entry from the mapping list (head:_mapping_head , lock:_mapping_head_lock)
 */
void add_mapping_entry(struct page_fault_mapping* entry);
struct page_fault_mapping* find_mapping_entry(int cpu, int id, unsigned long address);
void remove_mapping_entry(struct page_fault_mapping* entry);

/* List used to store ack answers to invalid messages
 */
DEFINE_RAW_SPINLOCK(_ack_head_lock);
struct ack_answers* _ack_head = NULL;
/* Functions to add,find and remove an entry from the ack list (head:_ack_head , lock:_ack_head_lock)
 */
void add_ack_entry(struct ack_answers* entry);
struct ack_answers* find_ack_entry(int cpu, int id, unsigned long address);
void remove_ack_entry(struct ack_answers* entry);

/* Functions used by the distributed shared memory for 2 kernels set up.
 * In Popcorn dsm functions are triggered upon page fault.
 *
 * A page can be in 2 different macro states: MAPPED and NOT MAPPED.
 * When NOT MAPPED the page is not accessible and there is not a corresponding physical
 * page in the local kernel.
 * To map a page a fetch must be triggered.
 *
 * A MAPPED page can be REPLICATED or NOT REPLICATED.
 * If NOT REPLICATED the copy of the page exists only in this kernel and the page
 * can be accessed on both read and write mode by this kernel.
 * A NOT REPLICATED page will become REPLICATED when the other kernel fetches it and the
 * page is not in a read only vma.
 *
 * A REPLICATED page can be in three different statuses: VALID, INVALID or WRITTEN.
 * A VALID page can be accesses by the local kernel just in read mode.
 * To write a VALID page a write must be triggered.
 * A WRITTEN page can be be accesses by the local kernel in both read and write mode.
 * An INVALID page cannot be accessed by the local kernel.
 * To access an INVALID page or a write or read must be triggered.
 *
 * If a page is in WRITTEN status in this kernel, it must be in INVALID status on the other
 * kernel. This guarantees that only one kernel can access the page in write mode at any time.
 *
 * NOTE: hugepages are not supported. For fetching is implemented an optimization that uses
 * the bit _PAGE_UNUSED1 of the ptes.
 *
 * NOTE: prefetch to include from version 4.
 */
int popcorn_try_handle_mm_fault(struct task_struct *tsk,
		struct mm_struct *mm, struct vm_area_struct *vma,
		unsigned long page_faul_address, unsigned long page_fault_flags,
		unsigned long error_code);
static int do_remote_fetch_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, pte_t value_pte,
		spinlock_t* ptl);
static int do_remote_write_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
		struct page* page,int invalid);
static int do_remote_read_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
		struct page* page);
void process_mapping_request_for_2_kernels(struct work_struct* work);
void process_invalid_request_for_2_kernels(struct work_struct* work);



extern int access_error(unsigned long error_code, struct vm_area_struct *vma);
extern int do_wp_page_for_popcorn(struct mm_struct *mm, struct vm_area_struct *vma,
		unsigned long address, pte_t *page_table, pmd_t *pmd,
		spinlock_t *ptl, pte_t orig_pte);
extern int do_mapping_for_distributed_process_from_page_fault(unsigned long vm_flags, unsigned long vaddr_start, unsigned long vaddr_size, unsigned long pgoff, char* path,
		struct mm_struct* mm, unsigned long address, spinlock_t* ptl);
extern int scif_get_nodeIDs(uint16_t *nodes, int len, uint16_t *self);

DECLARE_WAIT_QUEUE_HEAD(read_write_wait);
static int _cpu = -1;

/* Functions to add,find and remove an entry from the mapping list (head:_mapping_head , lock:_mapping_head_lock)
 */
void add_mapping_entry(struct page_fault_mapping* entry) {

	struct page_fault_mapping* curr;

	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_mapping_head_lock, flags);

	if (!_mapping_head) {
		_mapping_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _mapping_head;
		while (curr->next != NULL) {
			curr = curr->next;
		}

		curr->next = entry;
		entry->next = NULL;
		entry->prev = curr;
	}

	raw_spin_unlock_irqrestore(&_mapping_head_lock, flags);

}

struct page_fault_mapping* find_mapping_entry(int cpu, int id, unsigned long address) {

	struct page_fault_mapping* curr = NULL;
	struct page_fault_mapping* ret = NULL;

	unsigned long flags;

	raw_spin_lock_irqsave(&_mapping_head_lock, flags);

	curr = _mapping_head;
	while (curr) {

		if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id
				&& curr->address == address) {
			ret = curr;
			break;
		}

		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(&_mapping_head_lock, flags);

	return ret;
}

void remove_mapping_entry(struct page_fault_mapping* entry) {

	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_mapping_head_lock, flags);

	if (_mapping_head == entry) {
		_mapping_head = entry->next;
	}

	if (entry->next) {
		entry->next->prev = entry->prev;
	}

	if (entry->prev) {
		entry->prev->next = entry->next;
	}

	entry->prev = NULL;
	entry->next = NULL;

	raw_spin_unlock_irqrestore(&_mapping_head_lock, flags);

}

/* Functions to add,find and remove an entry from the ack list (head:_ack_head , lock:_ack_head_lock)
 */

void add_ack_entry(struct ack_answers* entry) {
	struct ack_answers* curr;

	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_ack_head_lock, flags);

	if (!_ack_head) {
		_ack_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _ack_head;
		while (curr->next != NULL) {
			curr = curr->next;
		}

		curr->next = entry;
		entry->next = NULL;
		entry->prev = curr;
	}

	raw_spin_unlock_irqrestore(&_ack_head_lock, flags);
}


struct ack_answers* find_ack_entry(int cpu, int id, unsigned long address) {
	struct ack_answers* curr = NULL;
	struct ack_answers* ret = NULL;


	unsigned long flags;

	raw_spin_lock_irqsave(&_ack_head_lock, flags);

	curr = _ack_head;
	while (curr) {

		if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id
				&& curr->address == address) {
			ret = curr;
			break;
		}

		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(&_ack_head_lock, flags);

	return ret;
}


void remove_ack_entry(struct ack_answers* entry) {


	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_ack_head_lock, flags);

	if (_ack_head == entry) {
		_ack_head = entry->next;
	}

	if (entry->next) {
		entry->next->prev = entry->prev;
	}

	if (entry->prev) {
		entry->prev->next = entry->next;
	}

	entry->prev = NULL;
	entry->next = NULL;

	raw_spin_unlock_irqrestore(&_ack_head_lock, flags);

}

/* Changes the protection flags for a pte when it has the PTE_PRESENT flag not set.
 */
void popcorn_change_not_present_pte_for_mprotect(pte_t *pte, pte_t oldpte, pgprot_t newprot, struct mm_struct *mm, unsigned long addr){

	if(!( pte==NULL || pte_none(pte_clear_flags(oldpte, _PAGE_UNUSED1)) )){

		struct page *page= pte_page(oldpte);

		if(page->replicated!=1 || page->status!=REPLICATION_STATUS_INVALID){
			printk("ERROR: mprotect moving a not present page that is not in invalid state or replicated in %s\n", __func__);
		}

		if( !(newprot.pgprot & PROT_WRITE)) { /*it is becoming a read only vma*/

			//force a new fetch
			int rss[NR_MM_COUNTERS];
			memset(rss, 0, sizeof(int) * NR_MM_COUNTERS);

			if (PageAnon(page))
				rss[MM_ANONPAGES]--;
			else {
				rss[MM_FILEPAGES]--;
			}

#if DIFF_PAGE
			if(page->old_page_version!=NULL)
				kfree(page->old_page_version);
#endif

			page_remove_rmap(page);

			page->replicated= 0;
			page->status= REPLICATION_STATUS_NOT_REPLICATED;

			if(cpumask_first(cpu_present_mask)==current->tgroup_home_cpu){
				ptep_get_and_clear(mm, addr, pte);
				pte_t ptent= *pte;
				if(!pte_none(ptent))
					printk("ERROR: mprot cleaning pte but after not none\n");
				ptent = pte_set_flags(ptent, _PAGE_UNUSED1);
				set_pte_at_notify(mm, addr, pte, ptent);
			}

			int i;

			if (current->mm == mm)
				sync_mm_rss(current, mm);
			for (i = 0; i < NR_MM_COUNTERS; i++)
				if (rss[i])
					atomic_long_add(rss[i], &mm->rss_stat.count[i]);
		}
	}
}

/* Changes the protection flags for a pte when it has the PTE_PRESENT flag set.
 */
int popcorn_change_present_pte_for_mprotect(pte_t oldpte, pgprot_t newprot){

	int clear= 0;
	/*case pte_present: or REPLICATION_STATUS_NOT_REPLICATED or
			 REPLICATION_STATUS_VALID or REPLICATION_STATUS_WRITTEN*/

	struct page *page= pte_page(oldpte);

	if (!is_zero_page(pte_pfn(oldpte))) {

		if (page->status == REPLICATION_STATUS_INVALID) {
			printk("ERROR: mprotect moving "
					"a present page that is in invalid state in %s\n", __func__);
		}

		if (!(newprot.pgprot & PROT_WRITE)) { /*it is becoming a read only vma*/

			if (page->replicated == 1) {

				if (page->status == REPLICATION_STATUS_NOT_REPLICATED) {
					printk("ERROR:  page replicated is 1 "
							"but in state not replicated in %s\n", __func__);
				}

				page->replicated = 0;

				page->status = REPLICATION_STATUS_NOT_REPLICATED;
			}

		} else { /* it is becoming a writable vma*/

			if (page->replicated == 0) {

				if (page->status != REPLICATION_STATUS_NOT_REPLICATED) {
					printk("ERROR: page replicated is zero "
							"but not in state not replicated in %s\n", __func__);
				}
				//check if somebody else fetched the page
				int i,count=0;
				for (i = 0; i < MAX_KERNEL_IDS; i++) {
					count=count+page->other_owners[i];
				}
				if(count>1){
					page->replicated = 1;
					page->status = REPLICATION_STATUS_VALID;
					clear=1;
				}
			}else
			{
				/*in case valid I enforce a clear*/
				if (page->status == REPLICATION_STATUS_VALID) {
					clear=1;
				}
			}

		}
	}

	return clear;
}

static int handle_mapping_response_void(struct pcn_kmsg_message* inc_msg) {

	struct mapping_response_void* response;
	struct page_fault_mapping* fetched_data;
	response = (struct mapping_response_void*) inc_msg;

	fetched_data = find_mapping_entry(response->tgroup_home_cpu,
			response->tgroup_home_id, response->address);

	DSMPRINTK("Answer_request_void for address %lu from cpu %i. This is a void response. owner %d\n", response->address, inc_msg->hdr.from_cpu,response->owner);

	if (fetched_data == NULL) {
		pcn_kmsg_free_msg_now(inc_msg);
		return -1;
	}

	if (response->owner == 1) {
		fetched_data->owner = 1;
	}

	if (response->vma_present == 1) {

		if (response->header.from_cpu != response->tgroup_home_cpu)
			printk("ERROR: a kernel that is not the server is sending the mapping\n");

		if (fetched_data->vma_present == 0) {
			fetched_data->vma_present = 1;
			fetched_data->vaddr_start = response->vaddr_start;
			fetched_data->vaddr_size = response->vaddr_size;
			fetched_data->prot = response->prot;
			fetched_data->pgoff = response->pgoff;
			fetched_data->vm_flags = response->vm_flags;
			strcpy(fetched_data->path, response->path);
		}

		else{
			printk("ERROR: received more than one mapping\n");
		}

	}

	if(fetched_data->arrived_response!=0)
		printk("ERROR: received more than one answer, arrived_response is %d \n",fetched_data->arrived_response);

	fetched_data->futex_owner = response->futex_owner;

	fetched_data->arrived_response++;

	wake_up_process(fetched_data->waiting);

	pcn_kmsg_free_msg_now(inc_msg);

	return 1;

}

static int handle_mapping_response(struct pcn_kmsg_message* inc_msg) {

	struct mapping_response* response;
	struct page_fault_mapping* fetched_data;
	int set = 0;
	response = (struct mapping_response*) inc_msg;

	fetched_data = find_mapping_entry(response->tgroup_home_cpu,
			response->tgroup_home_id, response->address);

	DSMPRINTK("Answer_request for address %lu from cpu %i \n", response->address, inc_msg->hdr.from_cpu);

	if (fetched_data == NULL) {
		pcn_kmsg_free_msg(inc_msg);
		return -1;

	}

#if CHECKSUM
	__wsum check= csum_partial(&response->data, PAGE_SIZE, 0);
	if(check!=response->checksum)
		printk("Checksum sent: %i checksum computed %i\n",response->checksum,check);
#endif

	if (response->vma_present == 1) {

		if (response->header.from_cpu != response->tgroup_home_cpu)
			printk("ERROR: a kernel that is not the server is sending the mapping\n");

		if (fetched_data->vma_present == 0) {
			fetched_data->vma_present = 1;
			fetched_data->vaddr_start = response->vaddr_start;
			fetched_data->vaddr_size = response->vaddr_size;
			fetched_data->prot = response->prot;
			fetched_data->pgoff = response->pgoff;
			fetched_data->vm_flags = response->vm_flags;
			strcpy(fetched_data->path, response->path);
		}

		else{
			printk("ERROR: received more than one mapping\n");
		}

	}

	if (response->owner == 1) {
		fetched_data->owner = 1;
	}


	if (fetched_data->address_present == 1) {
		printk("ERROR: received more than one answer with a copy of the page\n");

	} else  {
		fetched_data->address_present= 1;
		fetched_data->data = response;
		fetched_data->last_write = response->last_write;
		set = 1;
	}

	if(fetched_data->arrived_response!=0)
		printk("ERROR: received more than one answer, arrived_response is %d \n",fetched_data->arrived_response);

	fetched_data->owners[inc_msg->hdr.from_cpu] = 1;

	fetched_data->arrived_response++;

	fetched_data->futex_owner = response->futex_owner;

	wake_up_process(fetched_data->waiting);

	if (set == 0)
		pcn_kmsg_free_msg_now(inc_msg);

	return 1;
}

static int handle_ack_invalid(struct pcn_kmsg_message* inc_msg) {

	struct ack_invalid* response;
	struct ack_answers* fetched_data;

	response = (struct ack_invalid*) inc_msg;
	fetched_data = find_ack_entry(response->tgroup_home_cpu,
			response->tgroup_home_id, response->address);

	DSMPRINTK("Answer_invalid for address %lu from cpu %i \n", response->address, inc_msg->hdr.from_cpu);

	if (fetched_data == NULL) {
		goto out;
	}

	fetched_data->response_arrived++;

	if(fetched_data->response_arrived>1)
		printk("ERROR: received more than one ack\n");

	wake_up_process(fetched_data->waiting);

	out: pcn_kmsg_free_msg_now(inc_msg);

	return 0;
}

/* This function handle invalid messages (write requests sent by a owner kernel).
 *
 * It changes the status of the page to INVALID and sends back an ack message.
 *
 * Note: in some cases the request may be delayed.
 */
void process_invalid_request_for_2_kernels(struct work_struct* work){
	struct invalid_request_work* work_request = (struct invalid_request_work*) work;
	struct invalid_request* data = work_request->request;
	struct ack_invalid* response;
	struct migration_memory* memory = NULL;
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
	struct invalid_request_work *delay;

	DSMPRINTK("Invalid for address %lu from cpu %i\n", data->address, from_cpu);

	response =  pcn_kmsg_alloc_msg(sizeof(*response));
	if (response == NULL) {
		printk("Impossible to kmalloc in %s\n", __func__);
		pcn_kmsg_free_msg_now(data);
		kfree(work);
		return;
	}
	response->writing = 0;

	memory = find_migration_memory_memory_entry(data->tgroup_home_cpu, data->tgroup_home_id);
	if (memory != NULL) {
		if(memory->setting_up==1){
			goto out;
		}
		mm = memory->mm;
	} else {
		goto out;
	}

	down_read(&mm->mmap_sem);

	/*check the vma era first*/
	if(mm->vma_operation_index < data->vma_operation_index){

		delay = kmalloc(sizeof(*delay), GFP_ATOMIC);

		if (delay!=NULL) {
			delay->request = data;
			INIT_DELAYED_WORK( (struct delayed_work*)delay,
					process_invalid_request_for_2_kernels);
			queue_delayed_work(invalid_message_wq,
					(struct delayed_work*) delay, 10);
		}else{
			printk("Impossible to kmalloc in %s\n",__func__);
		}

		up_read(&mm->mmap_sem);
		kfree(work);

		return;
	}

	/* check if there is a valid vma*/
	vma = find_vma(mm, address);
	if (!vma || address >= vma->vm_end || address < vma->vm_start) {
		vma = NULL;
	} else {

		if (unlikely(is_vm_hugetlb_page(vma))
				|| unlikely(transparent_hugepage_enabled(vma))) {
			printk("Request for HUGE PAGE vma\n");
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

	/*case pte not yet installed*/
	if (pte == NULL || pte_none(pte_clear_flags(*pte,_PAGE_UNUSED1)) ) {

		DSMPRINTK("pte not yet mapped \n");

		/*If I receive an invalid while it is not mapped, I must be fetching the page.
		*Otherwise it is an error.
		*Delay the invalid while I install the page.
		*/

		/*Check if I am concurrently fetching the page*/
		struct page_fault_mapping* fetched_data = find_mapping_entry(
				data->tgroup_home_cpu, data->tgroup_home_id, address);

		if (fetched_data != NULL) {
			DSMPRINTK("Concurrently fetching the same address\n");

			if(fetched_data->is_fetch!=1)
				printk("ERROR: invalid received for a not mapped pte that has a mapping_answer not in fetching\n");

			delay = kmalloc(sizeof(*delay), GFP_ATOMIC);

			if (delay!=NULL) {
				delay->request = data;
				INIT_DELAYED_WORK( (struct delayed_work*)delay,
						process_invalid_request_for_2_kernels);
				queue_delayed_work(invalid_message_wq,
						(struct delayed_work*) delay, 10);
			}
			else{
				printk("Impossible to kmalloc in %s\n",__func__);
			}

			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			kfree(work);
			return;
		}
		else{
			printk("ERROR: received an invalid for a not mapped pte not in fetching status\n");
		}

		goto out;

	} else {

		/*the "standard" page fault releases the pte lock after that it installs the page
		*so before that I lock the pte again there is a moment in which is not null
		*but still fetching
		 */
		if (memory->alive != 0) {
			struct page_fault_mapping* fetched_data = find_mapping_entry(
					data->tgroup_home_cpu, data->tgroup_home_id, address);

			if(fetched_data!=NULL && fetched_data->is_fetch==1){

				delay = kmalloc(sizeof(*delay), GFP_ATOMIC);

				if (delay!=NULL) {
					delay->request = data;
					INIT_DELAYED_WORK( (struct delayed_work*)delay,
							process_invalid_request_for_2_kernels);
					queue_delayed_work(invalid_message_wq,
							(struct delayed_work*) delay, 10);
				}
				else{
					printk("Impossible to kmalloc in %s\n",__func__);
				}
				spin_unlock(ptl);
				up_read(&mm->mmap_sem);
				kfree(work);
				return;
			}
		}

		page = pte_page(*pte);
		if (page != vm_normal_page(vma, address, *pte)) {
			DSMPRINTK("page different from vm_normal_page in request page\n");
		}
		if (page->replicated == 0 || page->status==REPLICATION_STATUS_NOT_REPLICATED) {
			printk("ERROR: Invalid message in not replicated page\n");
			goto out;
		}

		if (page->status == REPLICATION_STATUS_WRITTEN) {
			printk("ERROR: invalid message in a written page\n");
			goto out;
		}

		if(page->reading==1){
			/*If I am reading my current status must be invalid and the one of the other kernel must be written.
			 *After that he sees my request of page, it mights want to write again and it sends me an invalid.
			 *So this request must be delayed.
			 */

			if(page->status!=REPLICATION_STATUS_INVALID || page->last_write!=(data->last_write-1))
				printk("Incorrect invalid received while reading address %lu, my status is %d, page last write %lu, invalid for version %lu",
						address,page->status,page->last_write,data->last_write);

			delay =  kmalloc(sizeof(*delay), GFP_ATOMIC);

			if (delay!=NULL) {
				delay->request = data;
				INIT_DELAYED_WORK( (struct delayed_work*)delay,
						process_invalid_request_for_2_kernels);
				queue_delayed_work(invalid_message_wq,
						(struct delayed_work*) delay, 10);
			}
			else{
				printk("Impossible to kmalloc in %s\n",__func__);
			}

			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			kfree(work);
			return;
		}

		if(page->writing==1){
			/*Concurrent write.
			 *To be correct I must be or in valid or invalid state and not owner.
			 *The kernel with the ownership always wins.
			 */
			response->writing=1;
			if(page->owner==1 || page->status==REPLICATION_STATUS_WRITTEN)
				printk("Incorrect invalid received while writing address %lu, my status is %d, page last write %lu, invalid for version %lu page owner %d",
						address,page->status,page->last_write,data->last_write,page->owner);
		}

		if(page->last_write!= data->last_write)
			printk("ERROR: received an invalid for copy %lu but my copy is %lu\n",data->last_write,page->last_write);

		page->status = REPLICATION_STATUS_INVALID;
		page->owner = 0;

		flush_cache_page(vma, address, pte_pfn(*pte));

		entry = *pte;
		entry = pte_clear_flags(entry, _PAGE_PRESENT);
		entry = pte_set_flags(entry, _PAGE_ACCESSED);

		ptep_clear_flush(vma, address, pte);

		set_pte_at_notify(mm, address, pte, entry);

		update_mmu_cache(vma, address, pte);
		flush_tlb_page(vma, address);
		flush_tlb_fix_spurious_fault(vma, address);

	}

	out: if (lock) {
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
	}

	response->header.type = PCN_KMSG_TYPE_ACK_INVALID;
	response->header.prio = PCN_KMSG_PRIO_NORMAL;
	response->tgroup_home_cpu = data->tgroup_home_cpu;
	response->tgroup_home_id = data->tgroup_home_id;
	response->address = data->address;
	response->ack = 1;

	pcn_kmsg_send_long(from_cpu,(struct pcn_kmsg_long_message*) (response),sizeof(*response)-sizeof(struct pcn_kmsg_hdr));
	pcn_kmsg_free_msg(response);
	pcn_kmsg_free_msg_now(data);
	kfree(work);

}

static int handle_invalid_request(struct pcn_kmsg_message* inc_msg) {


	struct invalid_request_work* request_work;
	struct invalid_request* data = (struct invalid_request*) inc_msg;

	request_work = kmalloc(sizeof(*request_work), GFP_ATOMIC);

	if (request_work!=NULL) {
		request_work->request = data;
		INIT_WORK( (struct work_struct*)request_work, process_invalid_request_for_2_kernels);
		queue_work(invalid_message_wq, (struct work_struct*) request_work);
	}
	else {
		printk("Impossible to kmalloc in %s\n",__func__);
		pcn_kmsg_free_msg_now(inc_msg);
	}

	return 1;

}

/* This function handle fetch, read and write messages (write requests sent by a not owner kernel).
 *
 * It changes the status of the page according to the type of the request
 * and may send a copy of the page present in this kernel.
 *
 * Note: in some cases the request may be delayed.
 */
void process_mapping_request_for_2_kernels(struct work_struct* work) {

	struct page_fault_mapping_request_work* request_work = (struct page_fault_mapping_request_work*) work;
	struct page_fault_mapping_request* request = request_work->request;
	struct migration_memory * memory;
	struct mm_struct* mm = NULL;
	struct vm_area_struct* vma = NULL;
	struct mapping_response_void* void_response;
	int owner= 0;
	char* plpath;
	char lpath[512];
	int from_cpu = request->header.from_cpu;
	unsigned long address = request->address & PAGE_MASK;
	pgd_t* pgd;
	pud_t* pud;
	pmd_t* pmd;
	pte_t* pte;
	pte_t entry;
	spinlock_t* ptl = NULL;
	struct page_fault_mapping_request_work* delay;
	struct page* page, *old_page;
	struct mapping_response* response;
	struct page_fault_mapping* fetched_data;
	int lock =0;
	void *vfrom;

	DSMPRINTK("Request for address %lu is fetch %i is write %i\n", request->address,((request->is_fetch==1)?1:0),((request->is_write==1)?1:0));

	memory = find_migration_memory_memory_entry(request->tgroup_home_cpu,request->tgroup_home_id);
	if (memory != NULL) {
		if(memory->setting_up==1){
			owner=1;
			goto out;
		}
		mm = memory->mm;
	} else {
		owner=1;
		goto out;
	}

	down_read(&mm->mmap_sem);

	/*check the vma era first*/
	if(mm->vma_operation_index < request->vma_operation_index){
		delay = kmalloc(sizeof(*delay), GFP_ATOMIC);

		if (delay) {
			delay->request = request;
			INIT_DELAYED_WORK( (struct delayed_work*)delay,process_mapping_request_for_2_kernels);

			queue_delayed_work(message_request_wq,(struct delayed_work*) delay, 10);
		}
		else{
			printk("Impossible to kmalloc in %s\n",__func__);
		}

		up_read(&mm->mmap_sem);
		kfree(work);

		return;
	}

	/* check if there is a valid vma*/
	vma = find_vma(mm, address);
	if (!vma || address >= vma->vm_end || address < vma->vm_start) {
		vma = NULL;
		if(_cpu == request->tgroup_home_cpu){
			up_read(&mm->mmap_sem);
			goto out;
		}
	} else {

		if (unlikely(is_vm_hugetlb_page(vma))|| unlikely(transparent_hugepage_enabled(vma))) {
			printk("ERROR: Request for HUGE PAGE vma\n");
			up_read(&mm->mmap_sem);
			goto out;
		}


	}


	if(_cpu!=request->tgroup_home_cpu){

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
	}
	else{

		pgd = pgd_offset(mm, address);

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
			printk("ERROR: request for huge page\n");
			up_read(&mm->mmap_sem);
			goto out;
		}

	}

	pte = pte_offset_map_lock(mm, pmd, address, &ptl);
	/*PTE LOCKED*/

	entry = *pte;
	lock= 1;


	if (pte == NULL || pte_none(pte_clear_flags(entry, _PAGE_UNUSED1))) {

		DSMPRINTK("pte not mapped \n");

		if( !pte_none(entry) ){

			if(_cpu!=request->tgroup_home_cpu || request->is_fetch==1){
				printk("ERROR: incorrect request for marked page\n");
				goto out;
			}
			else{
				DSMPRINTK("request for a marked page\n");
			}
		}

		if ((_cpu==request->tgroup_home_cpu) || memory->alive != 0) {

			fetched_data = find_mapping_entry(request->tgroup_home_cpu, request->tgroup_home_id, address);

			/*case concurrent fetch*/
			if (fetched_data != NULL) {

				fetch:				DSMPRINTK("concurrent request\n");

				/*Whit marked pages only two scenarios can happenn:
				 * Or I am the main and I an locally fetching=> delay this fetch
				 * Or I am not the main, but the main already answer to my fetch (otherwise it will not answer to me the page)
				 * so wait that the answer arrive before consuming the fetch.
				 * */
				if (fetched_data->is_fetch != 1)
					printk("ERROR: find a struct page_fault_mapping not mapped and not fetch\n");

				delay = kmalloc(sizeof(*delay),GFP_ATOMIC);

				if (delay!=NULL) {
					delay->request = request;
					INIT_DELAYED_WORK((struct delayed_work*)delay,process_mapping_request_for_2_kernels);
					queue_delayed_work(message_request_wq,
							(struct delayed_work*) delay, 10);
				}
				else{
					printk("Impossible to kmalloc in %s\n",__func__);
				}

				spin_unlock(ptl);
				up_read(&mm->mmap_sem);
				kfree(work);

				return;

			}

			else{
				/*mark the pte if main*/
				if(_cpu==request->tgroup_home_cpu){

					DSMPRINTK("%s: marking a pte for address %lu \n",__func__,address);

					entry = pte_set_flags(entry, _PAGE_UNUSED1);

					ptep_clear_flush(vma, address, pte);

					set_pte_at_notify(mm, address, pte, entry);

					update_mmu_cache(vma, address, pte);

				}
			}
		}
		/*pte not present*/
		owner= 1;
		goto out;

	}

	page = pte_page(entry);
	if (page != vm_normal_page(vma, address, entry)) {
		DSMPRINTK("Page different from vm_normal_page in request page\n");
	}
	old_page = NULL;

	if (is_zero_page(pte_pfn(entry)) || !(page->replicated == 1)) {

		DSMPRINTK("Page not replicated\n");

		/*There is the possibility that this request arrived while I am fetching, after that I installed the page
		 * but before calling the update page....
		 * */
		if (memory->alive != 0) {
			fetched_data = find_mapping_entry(request->tgroup_home_cpu, request->tgroup_home_id, address);

			if(fetched_data!=NULL){
				goto fetch;
			}
		}

		/*the request must be for a fetch*/
		if(request->is_fetch==0)
			printk("ERROR received a request not fetch for a not replicated page\n");

		if (vma->vm_flags & VM_WRITE) {

			/*if the page is writable but the pte has not the write flag set, it is a cow page*/
			if (!pte_write(entry)) {

				retry_cow:
				DSMPRINTK("COW page at %lu \n", address);

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

					printk("ERROR: %s bug from do_wp_page_for_popcorn\n",__func__);
					up_read(&mm->mmap_sem);
					goto out;
				}

				spin_lock(ptl);
				/*PTE LOCKED*/
				lock = 1;

				entry = *pte;

				if(!pte_write(entry)){
					printk("WARNING: page not writable after cow\n");
					goto retry_cow;
				}

				page = pte_page(entry);

			}

			page->replicated = 1;

			flush_cache_page(vma, address, pte_pfn(*pte));
			entry = mk_pte(page, vma->vm_page_prot);

			if(request->is_write==0){
				/*case fetch for read*/
				page->status = REPLICATION_STATUS_VALID;
				entry = pte_clear_flags(entry, _PAGE_RW);
				entry = pte_set_flags(entry, _PAGE_PRESENT);
				owner= 0;
				page->owner= 1;
			}
			else{
				/*case fetch for write*/
				page->status = REPLICATION_STATUS_INVALID;
				entry = pte_clear_flags(entry, _PAGE_PRESENT);
				owner= 1;
				page->owner= 0;
			}

			page->last_write= 1;

			entry = pte_set_flags(entry, _PAGE_USER);
			entry = pte_set_flags(entry, _PAGE_ACCESSED);

			ptep_clear_flush(vma, address, pte);

			set_pte_at_notify(mm, address, pte, entry);

			update_mmu_cache(vma, address, pte);

			if (old_page != NULL){
				page_remove_rmap(old_page);
			}

		} else {
			/*read only vma*/

			page->replicated=0;
			page->status= REPLICATION_STATUS_NOT_REPLICATED;

			if(request->is_write==1){
				printk("ERROR: received a write in a read-only not replicated page\n");
			}
			page->owner= 1;
			owner= 0;
		}

		page->other_owners[_cpu]=1;
		page->other_owners[from_cpu]=1;

		goto resolved;
	}
	else{
		/*replicated page case*/
		DSMPRINTK("Page replicated...\n");

		if(request->is_fetch==1){
			printk("ERROR: received a fetch request in a replicated status in %s\n", __func__);
		}

		if(page->writing==1){

			DSMPRINTK("Page currently in writing \n");


			if(request->is_write==0){
				DSMPRINTK("Concurrent read request\n");
			}
			else{
				DSMPRINTK("Concurrent write request\n");
			}

			delay = kmalloc(sizeof(*delay), GFP_ATOMIC);

			if (delay!=NULL) {
				delay->request = request;
				INIT_DELAYED_WORK( (struct delayed_work*)delay,
						process_mapping_request_for_2_kernels);
				queue_delayed_work(message_request_wq,(struct delayed_work*) delay, 10);
			}else{
				printk("Impossible to kmalloc in %s\n",__func__);
			}

			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			kfree(work);

			return;

		}

		if(page->reading==1){

			printk("ERROR: page in reading but received a request\n");
			goto out;
		}

		/*invalid page case*/
		if (page->status == REPLICATION_STATUS_INVALID) {

			printk("ERROR: received a request in invalid status without reading or writing\n");
			goto out;
		}

		/*valid page case*/
		if (page->status == REPLICATION_STATUS_VALID) {

			DSMPRINTK("Page requested valid\n");

			if(page->owner!=1){
				printk("ERROR: request in a not owner valid page\n");
			}
			else{
				if(request->is_write){
					if(page->last_write!= request->last_write)
						printk("ERROR: received a write for copy %lu but my copy is %lu\n",request->last_write,page->last_write);

					page->status= REPLICATION_STATUS_INVALID;
					page->owner= 0;
					owner= 1;
					entry = *pte;
					entry = pte_clear_flags(entry, _PAGE_PRESENT);
					entry = pte_set_flags(entry, _PAGE_ACCESSED);

					ptep_clear_flush(vma, address, pte);

					set_pte_at_notify(mm, address, pte, entry);

					update_mmu_cache(vma, address, pte);
				}
				else{
					printk("ERROR: received a read request in valid status\n");
				}
			}

			goto out;
		}

		if (page->status == REPLICATION_STATUS_WRITTEN) {

			DSMPRINTK("Page requested in written status\n");

			if(page->owner!=1){
				printk("ERROR: page in written status without ownership\n");
			}
			else{
				if(request->is_write==1){

					if(page->last_write!= (request->last_write+1))
						printk("ERROR: received a write for copy %lu but my copy is %lu\n",request->last_write,page->last_write);

					page->status= REPLICATION_STATUS_INVALID;
					page->owner= 0;
					owner= 1;
					entry = *pte;
					entry = pte_clear_flags(entry, _PAGE_PRESENT);
					entry = pte_set_flags(entry, _PAGE_ACCESSED);

					ptep_clear_flush(vma, address, pte);

					set_pte_at_notify(mm, address, pte, entry);

					update_mmu_cache(vma, address, pte);
				}
				else{

					if(page->last_write!= (request->last_write+1))
						printk("ERROR: received an read for copy %lu but my copy is %lu\n",request->last_write,page->last_write);

					page->status = REPLICATION_STATUS_VALID;
					page->owner= 1;
					owner= 0;
					entry = *pte;
					entry = pte_set_flags(entry, _PAGE_PRESENT);
					entry = pte_set_flags(entry, _PAGE_ACCESSED);
					entry = pte_clear_flags(entry, _PAGE_RW);

					ptep_clear_flush(vma, address, pte);

					set_pte_at_notify(mm, address, pte, entry);

					update_mmu_cache(vma, address, pte);
				}
			}

#if DIFF_PAGE
			goto resolved_diff;
#else
			goto resolved;
#endif
		}

	}

	resolved:

	DSMPRINTK("Page read only?%i Page shared?%i \n", (vma->vm_flags & VM_WRITE)?0:1, (vma->vm_flags & VM_SHARED)?1:0);

#if DIFF_PAGE

	char *app= kmalloc(sizeof(char)*PAGE_SIZE*2, GFP_ATOMIC);
	if (app == NULL) {
		printk("Impossible to kmalloc in %s\n",__func__);
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		pcn_kmsg_free_msg_now(request);
		kfree(work);
		return;
	}

	vfrom = kmap_atomic(page, KM_USER0);

	unsigned int compressed_byte= WKdm_compress(vfrom, (WK_word *)app);

	if(compressed_byte<((PAGE_SIZE/10)*9)){

		kunmap_atomic(vfrom, KM_USER0);
		response = pcn_kmsg_alloc_msg(sizeof(*response)+compressed_byte);
		if (response == NULL) {
			printk("Impossible to kmalloc in %s\n",__func__);
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			pcn_kmsg_free_msg_now(request);
			kfree(work);
			kfree(app);
			return;
		}
		memcpy(&(response->data),app,compressed_byte);
		response->data_size= compressed_byte;
		kfree(app);
	}
	else{

		response = pcn_kmsg_alloc_msg(sizeof(*response)+PAGE_SIZE);
		if (response == NULL) {
			printk("Impossible to kmalloc in %s\n",__func__);
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			pcn_kmsg_free_msg_now(request);
			kfree(work);
			kfree(app);
			return;
		}
		void* vto = &(response->data);
		copy_page(vto, vfrom);
		kunmap_atomic(vfrom, KM_USER0);
		response->data_size= PAGE_SIZE;
		kfree(app);
	}

	response->diff=0;

#else

	response = pcn_kmsg_alloc_msg(sizeof(*response)+PAGE_SIZE);
	if (response == NULL) {
		printk("Impossible to kmalloc in %s\n",__func__);
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		pcn_kmsg_free_msg_now(request);
		kfree(work);
		return;
	}

	void* vto = &(response->data);
	vfrom = kmap_atomic(page, KM_USER0);

	copy_page(vto, vfrom);
	kunmap_atomic(vfrom, KM_USER0);

	response->data_size= PAGE_SIZE;

#if CHECKSUM
	vfrom= kmap_atomic(page, KM_USER0);
	__wsum check1= csum_partial(vfrom, PAGE_SIZE, 0);
	kunmap_atomic(vfrom, KM_USER0);
	__wsum check2= csum_partial(&(response->data), PAGE_SIZE, 0);
	if(check1!=check2)
		printk("page just copied is not matching, address %lu\n",address);
#endif

#endif

	flush_cache_page(vma, address, pte_pfn(*pte));

	response->last_write = page->last_write;

	response->header.type = PCN_KMSG_TYPE_MAPPING_RESPONSE;
	response->header.prio = PCN_KMSG_PRIO_NORMAL;
	response->tgroup_home_cpu = request->tgroup_home_cpu;
	response->tgroup_home_id = request->tgroup_home_id;
	response->address = request->address;
	response->owner= owner;
	response->futex_owner = (!page) ? 0 : page->futex_owner;

	if (_cpu == request->tgroup_home_cpu && vma != NULL)
	{

		response->vma_present = 1;
		response->vaddr_start = vma->vm_start;
		response->vaddr_size = vma->vm_end - vma->vm_start;
		response->prot = vma->vm_page_prot;
		response->vm_flags = vma->vm_flags;
		response->pgoff = vma->vm_pgoff;
		if (vma->vm_file == NULL) {
			response->path[0] = '\0';
		} else {
			plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			strcpy(response->path, plpath);
		}

	}

	else
		response->vma_present = 0;

	spin_unlock(ptl);
	up_read(&mm->mmap_sem);

#if !DIFF_PAGE
#if CHECKSUM
	response->checksum= csum_partial(&response->data, PAGE_SIZE, 0);
#endif
#endif

	/*Send response*/
	pcn_kmsg_send_long(from_cpu, (struct pcn_kmsg_long_message*) (response),sizeof(*response) - sizeof(struct pcn_kmsg_hdr) + response->data_size);


	pcn_kmsg_free_msg_now(request);
	kfree(work);
	pcn_kmsg_free_msg(response);

	DSMPRINTK("Handle request end\n");

	return;

#if	DIFF_PAGE

	resolved_diff:

	if(page->old_page_version==NULL){
		printk("ERROR: no previous version of the page to calculate diff address %lu\n",address);
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		pcn_kmsg_free_msg_now(request);
		kfree(work);
		return;
	}

	app= kmalloc(sizeof(char)*PAGE_SIZE*2, GFP_ATOMIC);
	if (app == NULL) {
		printk("Impossible to kmalloc in %s\n",__func__);
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		pcn_kmsg_free_msg_now(request);
		kfree(work);
		return;
	}

	vfrom = kmap_atomic(page, KM_USER0);

	compressed_byte= WKdm_diff_and_compress ((WK_word *) page->old_page_version, vfrom,(WK_word *) app);

	if(compressed_byte<((PAGE_SIZE/10)*9)){


		kunmap_atomic(vfrom, KM_USER0);
		response = pcn_kmsg_alloc_msg(sizeof(*response)+compressed_byte);
		if (response == NULL) {
			printk("Impossible to kmalloc in %s\n",__func__);
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			pcn_kmsg_free_msg_now(request);
			kfree(work);
			kfree(app);
			return;
		}
		memcpy(&(response->data),app,compressed_byte);
		response->data_size= compressed_byte;
		kfree(app);
	}
	else{

		response = pcn_kmsg_alloc_msg(sizeof(*response)+PAGE_SIZE);
		if (response == NULL) {
			printk("Impossible to kmalloc in %s\n",__func__);
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			pcn_kmsg_free_msg_now(request);
			kfree(work);
			kfree(app);
			return;
		}
		void* vto = &(response->data);
		copy_page(vto, vfrom);
		kunmap_atomic(vfrom, KM_USER0);
		response->data_size= PAGE_SIZE;
		kfree(app);
	}

	response->diff=1;

	DSMPRINTK("Resolved Copy from %s\n", ((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"));

	DSMPRINTK("Page read only?%i Page shared?%i \n", (vma->vm_flags & VM_WRITE)?0:1, (vma->vm_flags & VM_SHARED)?1:0);

	flush_cache_page(vma, address, pte_pfn(*pte));

	response->last_write = page->last_write;

	response->futex_owner = page->futex_owner;

	response->header.type = PCN_KMSG_TYPE_MAPPING_RESPONSE;
	response->header.prio = PCN_KMSG_PRIO_NORMAL;
	response->tgroup_home_cpu = request->tgroup_home_cpu;
	response->tgroup_home_id = request->tgroup_home_id;
	response->address = request->address;
	response->owner= owner;

	if (_cpu == request->tgroup_home_cpu && vma != NULL)

	{

		response->vma_present = 1;
		response->vaddr_start = vma->vm_start;
		response->vaddr_size = vma->vm_end - vma->vm_start;
		response->prot = vma->vm_page_prot;
		response->vm_flags = vma->vm_flags;
		response->pgoff = vma->vm_pgoff;
		if (vma->vm_file == NULL) {
			response->path[0] = '\0';
		} else {
			plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			strcpy(response->path, plpath);
		}
	}

	else
		response->vma_present = 0;

	spin_unlock(ptl);
	up_read(&mm->mmap_sem);


	pcn_kmsg_send_long(from_cpu, (struct pcn_kmsg_long_message*) (response),
			sizeof(*response) - sizeof(struct pcn_kmsg_hdr) + response->data_size);


	pcn_kmsg_free_msg_now(request);
	kfree(work);
	pcn_kmsg_free_msg(response);

	DSMPRINTK("Handle request end\n");

	return;
#endif

	out:

	DSMPRINTK("sending void answer\n");

	void_response = pcn_kmsg_alloc_msg(sizeof(*void_response));
	if (void_response == NULL) {
		printk("Impossible to kmalloc in %s\n",__func__);
		if(lock){
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
		}
		pcn_kmsg_free_msg_now(request);
		kfree(work);
		return;
	}

	void_response->header.type = PCN_KMSG_TYPE_MAPPING_RESPONSE_VOID;
	void_response->header.prio = PCN_KMSG_PRIO_NORMAL;
	void_response->tgroup_home_cpu = request->tgroup_home_cpu;
	void_response->tgroup_home_id = request->tgroup_home_id;
	void_response->address = request->address;
	void_response->owner=owner;

	void_response->futex_owner = 0;

	if (_cpu == request->tgroup_home_cpu && vma != NULL)

	{
		void_response->vma_present = 1;
		void_response->vaddr_start = vma->vm_start;
		void_response->vaddr_size = vma->vm_end - vma->vm_start;
		void_response->prot = vma->vm_page_prot;
		void_response->vm_flags = vma->vm_flags;
		void_response->pgoff = vma->vm_pgoff;
		if (vma->vm_file == NULL) {
			void_response->path[0] = '\0';
		} else {
			plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			strcpy(void_response->path, plpath);
		}
	} else{
		void_response->vma_present = 0;
	}

	if(lock){
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
	}

	/* Send response*/
	pcn_kmsg_send_long(from_cpu,(struct pcn_kmsg_long_message*) (void_response),sizeof(*void_response) - sizeof(struct pcn_kmsg_hdr));


	pcn_kmsg_free_msg_now(request);
	pcn_kmsg_free_msg(void_response);
	kfree(work);

	DSMPRINTK("Handle request end\n");

}

static int handle_mapping_request(struct pcn_kmsg_message* inc_msg) {

	struct page_fault_mapping_request_work* request_work;
	struct page_fault_mapping_request* request = (struct page_fault_mapping_request*) inc_msg;


	request_work = kmalloc(sizeof(*request_work), GFP_ATOMIC);

	if (request_work) {
		request_work->request = request;
		INIT_WORK( (struct work_struct*)request_work, process_mapping_request_for_2_kernels);
		queue_work(message_request_wq, (struct work_struct*) request_work);
	}
	else {
		printk("Impossible to kmalloc in %s\n",__func__);
		pcn_kmsg_free_msg_now(inc_msg);
	}

	return 1;

}

void popcorn_clean_page(struct page* page) {

	if (page == NULL) {
		return;
	}

	page->replicated = 0;
	page->status = REPLICATION_STATUS_NOT_REPLICATED;
	page->owner = 0;
	memset(page->other_owners, 0, MAX_KERNEL_IDS*sizeof(int));
	page->writing = 0;
	page->reading = 0;

#if DIFF_PAGE
	page->old_page_version= NULL;
#endif

}

/* Function used to correctly end a fetch in popcorn when the fetch was performed locally.
 */
int popcorn_update_page(struct task_struct * tsk, struct mm_struct *mm,
		struct vm_area_struct *vma, unsigned long address_not_page, unsigned long page_fault_flags) {

	unsigned long address;

	pgd_t* pgd;
	pud_t* pud;
	pmd_t* pmd;
	pte_t* pte;
	pte_t entry;
	spinlock_t* ptl = NULL;
	struct page* page;

	int ret = 0;

	struct page_fault_mapping* fetched_data;

	address = address_not_page & PAGE_MASK;

	if (!vma || address >= vma->vm_end || address < vma->vm_start) {
		printk("ERROR: updating a page without valid vma in %s\n", __func__);
		ret = VM_FAULT_VMA;
		goto out_not_data;
	}

	if (unlikely(is_vm_hugetlb_page(vma))
			|| unlikely(transparent_hugepage_enabled(vma))) {
		printk("ERROR: Installed a vma with HUGEPAGE\n");
		ret = VM_FAULT_VMA;
		goto out_not_data;
	}

	fetched_data = find_mapping_entry(tsk->tgroup_home_cpu, tsk->tgroup_home_id,
			address);

	if (fetched_data != NULL) {

		if(fetched_data->is_fetch!=1 ){
			printk("ERROR: data structure is not for fetch in %s\n", __func__);
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto out_not_locked;
		}

		pgd = pgd_offset(mm, address);
		pud = pud_offset(pgd, address);
		if (!pud) {
			printk(
					"ERROR: no pud while trying to update a page locally fetched \n");
			ret = VM_FAULT_VMA;
			goto out_not_locked;
		}
		pmd = pmd_offset(pud, address);
		if (!pmd) {
			printk(
					"ERROR: no pmd while trying to update a page locally fetched \n");
			ret = VM_FAULT_VMA;
			goto out_not_locked;
		}

		pte = pte_offset_map_lock(mm, pmd, address, &ptl);
		entry= *pte;

		page = pte_page(entry);

		/*I replicate only if it is a writable page*/
		if (vma->vm_flags & VM_WRITE) {

			if (!pte_write(entry)) {
				retry_cow:
				DSMPRINTK("COW page at %lu \n", address);

				int cow_ret= do_wp_page_for_popcorn(mm, vma,address, pte,pmd,ptl, entry);

				if (cow_ret & VM_FAULT_ERROR) {
					if (cow_ret & VM_FAULT_OOM){
						printk("ERROR: %s VM_FAULT_OOM\n",__func__);
						ret = VM_FAULT_OOM;
						goto out_not_locked;
					}

					if (cow_ret & (VM_FAULT_HWPOISON | VM_FAULT_HWPOISON_LARGE)){
						printk("ERROR: %s EHWPOISON\n",__func__);
						ret = VM_FAULT_OOM;
						goto out_not_locked;
					}

					if (cow_ret & VM_FAULT_SIGBUS){
						printk("ERROR: %s EFAULT\n",__func__);
						ret = VM_FAULT_OOM;
						goto out_not_locked;
					}

					printk("ERROR: %s bug from do_wp_page_for_popcorn\n",__func__);
					ret = VM_FAULT_OOM;
					goto out_not_locked;
				}

				spin_lock(ptl);
				/*PTE LOCKED*/

				entry = *pte;

				if(!pte_write(entry)){
					printk("WARNING: page not writable after cow in %s\n", __func__);
					goto retry_cow;
				}

				page = pte_page(entry);
			}

			page->replicated = 0;
			page->owner= 1;
			page->other_owners[_cpu] = 1;

		} else {

			page->replicated = 0;
			page->other_owners[_cpu] = 1;
			page->owner= 1;
		}


	} else {
		printk("ERROR: impossible to find data to update\n");
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

/* Function to read a page in Popcorn.
 * To call only when the page is MAPPED, REPLICATED and in INVALID status.
 *
 * To read a page a message requesting the latest copy is sent to the other kernel.
 * This function will update the page with the received copy, and change the status to VALID.
 * Note: this function does not set the ownership for the page.
 */
static int do_remote_read_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
		struct page* page) {

	pte_t value_pte;
	int ret=0,i;


	page->reading= 1;

	/*message to ask for a copy*/
	struct page_fault_mapping_request* read_message = pcn_kmsg_alloc_msg(sizeof(*read_message));
	if (read_message == NULL) {
		printk("Impossible to kmalloc in %s\n",__func__);
		ret = VM_FAULT_OOM;
		goto exit;
	}

	read_message->header.type = PCN_KMSG_TYPE_MAPPING_REQUEST;
	read_message->header.prio = PCN_KMSG_PRIO_NORMAL;
	read_message->address = address;
	read_message->tgroup_home_cpu = tgroup_home_cpu;
	read_message->tgroup_home_id = tgroup_home_id;
	read_message->is_fetch= 0;
	read_message->is_write= 0;
	read_message->last_write= page->last_write;
	read_message->vma_operation_index= current->mm->vma_operation_index;

	/*object to held responses*/
	struct page_fault_mapping* reading_page = kmalloc(sizeof(*reading_page),
			GFP_ATOMIC);
	if (reading_page == NULL) {
		printk("Impossible to kmalloc in %s\n",__func__);
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

	/* Make data entry visible to handler.*/
	add_mapping_entry(reading_page);

	DSMPRINTK("Sending a read message for address %lu \n ", address);

	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	/*PTE UNLOCKED*/

	int sent= 0;
	reading_page->arrived_response=0;

	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;

		if (page->other_owners[i] == 1) {
			if (!(pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (read_message), sizeof(*read_message)-sizeof(struct pcn_kmsg_hdr))
					== -1)) {

				sent++;
				if(sent>1)
					printk("ERROR: using protocol optimized for 2 kernels but sending a read to more than one kernel in %s", __func__);
			}
		}
	}

	if(sent){
		/*wait for answer*/
		while (reading_page->arrived_response == 0) {

			set_task_state(current, TASK_UNINTERRUPTIBLE);
			if (reading_page->arrived_response == 0)
				schedule();
			set_task_state(current, TASK_RUNNING);
		}

	}
	else{
		printk("ERROR: impossible to send read message, no destination kernel in %s\n", __func__);
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

		printk("ERROR: vma not valid during read for write in %s\n", __func__);
		ret = VM_FAULT_VMA;
		goto exit_reading_page;
	}


	if(reading_page->address_present==1){

		if (reading_page->data->address != address) {
			printk("ERROR: trying to copy wrong address!");
			pcn_kmsg_free_msg_now(reading_page->data);
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}

		if (reading_page->last_write != (page->last_write+1)) {

			printk(	"ERROR: new copy received during a read but my last write is %lu and received last write is %lu\n",page->last_write,reading_page->last_write);
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}
		else
			page->last_write= reading_page->last_write;

		if(reading_page->owner==1){
			printk("ERROR: owneship sent with read request\n");
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}

		void *vto;
		void *vfrom;
		vto = kmap_atomic(page, KM_USER0);
		vfrom = &(reading_page->data->data);

#if DIFF_PAGE
		if(reading_page->data->data_size==PAGE_SIZE){
			copy_user_page(vto, vfrom, address, page);
		}
		else{

			if(reading_page->data->diff==1)
				WKdm_decompress_and_diff(vfrom,vto);
			else
			{
				kunmap_atomic(vto, KM_USER0);
				pcn_kmsg_free_msg_now(reading_page->data);
				printk("ERROR: received data not diff in write address %lu\n",address);
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_reading_page;
			}
		}

#else
		copy_user_page(vto, vfrom, address, page);
#endif

		kunmap_atomic(vto, KM_USER0);

#if !DIFF_PAGE
#if CHECKSUM
		vto= kmap_atomic(page, KM_USER0);
		__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
		kunmap_atomic(vto, KM_USER0);
		__wsum check2= csum_partial(&(reading_page->data->data), PAGE_SIZE, 0);
		if(check1!=check2) {
			printk("ERROR: page just copied is not matching, address %lu\n",address);
			pcn_kmsg_free_msg_now(reading_page->data);
			ret= VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}
		if(check1!=reading_page->data->checksum) {
			printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
			pcn_kmsg_free_msg_now(reading_page->data);
			ret= VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}
#endif
#endif

		pcn_kmsg_free_msg_now(reading_page->data);

		page->status = REPLICATION_STATUS_VALID;
		page->owner = reading_page->owner;

		flush_cache_page(vma, address, pte_pfn(*pte));

		value_pte = *pte;
		value_pte = pte_clear_flags(value_pte, _PAGE_RW);
		value_pte = pte_set_flags(value_pte, _PAGE_PRESENT);

		value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);

		ptep_clear_flush(vma, address, pte);

		set_pte_at_notify(mm, address, pte, value_pte);

		update_mmu_cache(vma, address, pte);

		flush_tlb_page(vma, address);

		flush_tlb_fix_spurious_fault(vma, address);


		DSMPRINTK("Out read for address %lu \n ", address);


	}
	else{

		printk("ERROR: no copy received for a read\n");
		ret= VM_FAULT_REPLICATION_PROTOCOL;
		remove_mapping_entry(reading_page);
		kfree(reading_page);
		pcn_kmsg_free_msg(read_message);
		goto exit;

	}


	exit_reading_page:


	remove_mapping_entry(reading_page);
	kfree(reading_page);

	exit_read_message:

	pcn_kmsg_free_msg(read_message);

	exit:

	page->reading = 0;

	return ret;
}

/* Function to write a page in Popcorn.
 * To call only when the page is MAPPED, REPLICATED, and or in VALID or in INVALID status.
 *
 * Each replicated page has an associated kernel owner. The owner is used to order
 * concurrent writes: if both kernels try to concurrently write the same page, the one with the ownership
 * will succeed while the other write request will be delayed.
 *
 * To write a page is needed to send a message to force an INVALID status on the other kernel and wait for the
 * corresponding answer.
 *
 * If not owner, the answer may piggyback the newest copy of the page.
 *
 * When the other kernel agrees on the write, this function sets this kernel as owner of the page and
 * change the status of the page to WRITTEN.
 *
 */
static int do_remote_write_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
		struct page* page,int invalid) {

	int  i;
	int ret= 0;
	pte_t value_pte;

	page->writing = 1;

	DSMPRINTK("Write address %lu pid %d\n", address,current->pid);

	/* If I am the page owner I can send an invalid message
	 * NOTE: invalid msg has precedence over write msg
	 * */
	if(page->owner==1){
		/*in this case I send and invalid message*/

		if(invalid){
			printk("ERROR: I am the owner of the page and it is invalid when going to write\n");
			ret= VM_FAULT_REPLICATION_PROTOCOL;
			goto exit;
		}

		/*object to store the acks (nacks) sent by other kernels*/
		struct ack_answers* answers = kmalloc(sizeof(*answers), GFP_ATOMIC);
		if (answers == NULL) {
			printk("Impossible to kmalloc in %s\n",__func__);
			ret = VM_FAULT_OOM;
			goto exit;
		}
		answers->tgroup_home_cpu = tgroup_home_cpu;
		answers->tgroup_home_id = tgroup_home_id;
		answers->address = address;
		answers->waiting = current;

		/*message to invalidate the other copies*/
		struct invalid_request* invalid_message = pcn_kmsg_alloc_msg(sizeof(*invalid_message));
		if (invalid_message == NULL) {
			printk("Impossible to kmalloc in %s\n",__func__);
			ret = VM_FAULT_OOM;
			goto exit_answers;
		}
		invalid_message->header.type = PCN_KMSG_TYPE_INVALID;
		invalid_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		invalid_message->tgroup_home_cpu = tgroup_home_cpu;
		invalid_message->tgroup_home_id = tgroup_home_id;
		invalid_message->address = address;
		invalid_message->vma_operation_index= current->mm->vma_operation_index;

		/* Insert the object in the appropriate list*/
		add_ack_entry(answers);

		invalid_message->last_write = page->last_write;

		answers->response_arrived= 0;

		int sent= 0;

		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		/*PTE UNLOCKED*/


		struct list_head *iter;
		_remote_cpu_info_list_t *objPtr;
		list_for_each(iter, &rlist_head) {
			objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
			i = objPtr->_data._processor;

			if (page->other_owners[i] == 1) {

				if (!(pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (invalid_message),sizeof(*invalid_message)-sizeof(struct pcn_kmsg_hdr))
						== -1)) {

					sent++;
					if(sent>1)
						printk("ERROR: using protocol optimized for 2 kernels but sending an invalid to more than one kernel in %s", __func__);
				}
			}
		}

		/*wait for acks*/
		if(sent){
			while (answers->response_arrived==0) {

				set_task_state(current, TASK_UNINTERRUPTIBLE);
				if (answers->response_arrived==0)
					schedule();

				set_task_state(current, TASK_RUNNING);
			}
					}
		else{
			printk("Impossible to send invalid, no destination kernel in %s\n", __func__);

		}

		down_read(&mm->mmap_sem);
		spin_lock(ptl);


		/*PTE LOCKED*/

		vma = find_vma(mm, address);
		if (unlikely(!vma || address >= vma->vm_end || address < vma->vm_start)) {

			printk("ERROR: vma not valid after waiting for ack to invalid in %s\n", __func__);
			ret = VM_FAULT_VMA;
			goto exit_invalid;
		}

		DSMPRINTK("Received ack to invalid for address %lu \n", address);

		exit_invalid:
		pcn_kmsg_free_msg(invalid_message);
		remove_ack_entry(answers);
		exit_answers:
		kfree(answers);
		if(ret!=0)
			goto exit;
	}
	/* if I am not the owner I have to send a write msg
	 * NOTE: write msg does not have the precedence over invalid msg
	 * */
	else{
		/*in this case I send a mapping request with write flag set*/


		struct page_fault_mapping_request* write_message = pcn_kmsg_alloc_msg(sizeof(*write_message));
		if (write_message == NULL) {
			printk("Impossible to kmalloc in %s\n",__func__);
			ret = VM_FAULT_OOM;
			goto exit;
		}

		write_message->header.type = PCN_KMSG_TYPE_MAPPING_REQUEST;
		write_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		write_message->address = address;
		write_message->tgroup_home_cpu = tgroup_home_cpu;
		write_message->tgroup_home_id = tgroup_home_id;
		write_message->is_fetch= 0;
		write_message->is_write= 1;
		write_message->last_write= page->last_write;
		write_message->vma_operation_index= current->mm->vma_operation_index;

		/*object to held responses*/
		struct page_fault_mapping* writing_page = kmalloc(sizeof(*writing_page),
				GFP_ATOMIC);
		if (writing_page == NULL) {
			printk("Impossible to kmalloc in %s\n",__func__);
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

		/* Make entry visible to handler.*/
		add_mapping_entry(writing_page);

		DSMPRINTK("Sending a write message for address %lu \n ", address);

		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		/*PTE UNLOCKED*/

		int sent= 0;
		writing_page->arrived_response=0;

		struct list_head *iter;
		_remote_cpu_info_list_t *objPtr;
		list_for_each(iter, &rlist_head) {
			objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
			i = objPtr->_data._processor;


			if (page->other_owners[i] == 1) {


				if (!(pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (write_message),sizeof(*write_message)-sizeof(struct pcn_kmsg_hdr))
						== -1)) {
					sent++;
					if(sent>1)
						printk("ERROR: using protocol optimized for 2 kernels but sending a write to more than one kernel in %s", __func__);
				}
			}
		}

		if(sent){

			/*wait for the answer*/
			while (writing_page->arrived_response == 0) {

				set_task_state(current, TASK_UNINTERRUPTIBLE);
				if (writing_page->arrived_response == 0)
					schedule();
				set_task_state(current, TASK_RUNNING);
			}




		}
		else{
			printk("ERROR: impossible to send write message, no destination kernel in %s\n", __func__);
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
			printk("ERROR: received answer to write without ownership\n");
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_writing_page;
		}

		/* If my copy is not update, the other kernel will send my the newest copy.
		 * */
		if(writing_page->address_present==1){

			if (writing_page->data->address != address) {
				printk("ERROR: trying to copy wrong address!");
				pcn_kmsg_free_msg(writing_page->data);
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_writing_page;
			}
			/*in this case I also received the new copy*/
			if (writing_page->last_write != (page->last_write+1)) {
				pcn_kmsg_free_msg(writing_page->data);
				printk("ERROR: new copy received during a write but my last write is %lu and received last write is %lu\n",page->last_write,writing_page->last_write);
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_writing_page;
			}
			else
				page->last_write= writing_page->last_write;


			void *vto;
			void *vfrom;
			vto = kmap_atomic(page, KM_USER0);
			vfrom = &(writing_page->data->data);


#if DIFF_PAGE
			if(writing_page->data->data_size==PAGE_SIZE){
				copy_user_page(vto, vfrom, address, page);
			}
			else{
				if(writing_page->data->diff==1)
					WKdm_decompress_and_diff(vfrom,vto);
				else
				{
					kunmap_atomic(vto, KM_USER0);
					pcn_kmsg_free_msg(writing_page->data);
					printk("ERROR: received data not diff in write address %lu\n",address);
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_writing_page;
				}
			}

#else

			copy_user_page(vto, vfrom, address, page);
#endif

			kunmap_atomic(vto, KM_USER0);

#if !DIFF_PAGE
#if CHECKSUM
			vto= kmap_atomic(page, KM_USER0);
			__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
			kunmap_atomic(vto, KM_USER0);
			__wsum check2= csum_partial(&(writing_page->data->data), PAGE_SIZE, 0);
			if(check1!=check2) {
				printk("ERROR: page just copied is not matching, address %lu\n",address);
				pcn_kmsg_free_msg(writing_page->data);
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_writing_page;
			}
			if(check1!=writing_page->data->checksum) {
				printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
				pcn_kmsg_free_msg(writing_page->data);
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_writing_page;
			}
#endif
#endif

			pcn_kmsg_free_msg(writing_page->data);

			exit_writing_page:

			remove_mapping_entry(writing_page);
			kfree(writing_page);

			exit_write_message:

			pcn_kmsg_free_msg(write_message);

			if(ret!=0)
				goto exit;
		}
		else{

			remove_mapping_entry(writing_page);
			kfree(writing_page);
			pcn_kmsg_free_msg(write_message);

			if(invalid){
				printk("ERROR: writing an invalid page but not received a copy\n");
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				goto exit;
			}
		}

	}

	page->status = REPLICATION_STATUS_WRITTEN;
	page->owner = 1;
	(page->last_write)++;

#if DIFF_PAGE
	if(page->old_page_version==NULL){
		page->old_page_version= kmalloc(sizeof(char)*PAGE_SIZE,
				GFP_ATOMIC);
		if(page->old_page_version==NULL){
			printk("Impossible to kmalloc in %s\n",__func__);
			goto exit;
		}
	}

	void *vto;
	void *vfrom;
	vto = page->old_page_version;
	vfrom = kmap_atomic(page, KM_USER0);
	copy_user_page(vto, vfrom, address, page);
	kunmap_atomic(vfrom, KM_USER0);
#endif

	flush_cache_page(vma, address, pte_pfn(*pte));

	/*now the page can be written*/
	value_pte = *pte;
	value_pte = pte_set_flags(value_pte, _PAGE_RW);
	value_pte = pte_set_flags(value_pte, _PAGE_PRESENT);

	value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);

	ptep_clear_flush(vma, address, pte);

	set_pte_at_notify(mm, address, pte, value_pte);

	update_mmu_cache(vma, address, pte);

	flush_tlb_page(vma, address);

	flush_tlb_fix_spurious_fault(vma, address);

	DSMPRINTK("Out write for address %lu last write is %lu \n ", address,page->last_write);

	exit:

	page->writing = 0;

	return ret;

}

/* Function to fetch a page in Popcorn.
 * To call only when the page was NOT_MAPPED.
 *
 * To fetch a page is necessary to send a message to the other kernel and wait
 * for its answer.
 * If the other kernel did not have a copy, this function will return notifying the caller
 * that a local page_fault is needed, the page will be mapped and updated with the received copy otherwise.
 *
 * The kernel in which the process started already has part of the pages mapped (but NOT_REPLICATED)
 * To speed up the protocol, if this kernel needs to fetch a new page, and it did not receive a fetch
 * from the other kernel previously, it can avoid to send the message and directly go local.
 * To implement this optimization the bit _PAGE_UNUSED1 is used to mark requested ptes.
 *
 */
static int do_remote_fetch_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, pte_t value_pte,
		spinlock_t* ptl) {

	struct page_fault_mapping* fetching_page;
	struct page_fault_mapping_request* fetch_message;
	int ret= 0,i,reachable,other_cpu=-1;

	fetching_page = kmalloc(sizeof(*fetching_page),	GFP_ATOMIC);
	if (fetching_page == NULL) {
		printk("Impossible to kmalloc in %s\n",__func__);
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
	fetching_page->futex_owner = -1;

	fetching_page->waiting = current;

	add_mapping_entry(fetching_page);

	if(_cpu==tgroup_home_cpu){
		if(pte_none(value_pte)){
			/*not marked pte*/
			DSMPRINTK("Copy not present in the other kernel, local fetch of address %lu pid %d\n", address,current->pid);
			ret = VM_CONTINUE_WITH_CHECK;
			goto exit;
		}
	}

	fetch_message = pcn_kmsg_alloc_msg(sizeof(*fetch_message));
	if (fetch_message == NULL) {
		printk("Impossible to kmalloc in %s\n",__func__);
		ret = VM_FAULT_OOM;
		goto exit_fetching_page;
	}

	fetch_message->header.type = PCN_KMSG_TYPE_MAPPING_REQUEST;
	fetch_message->header.prio = PCN_KMSG_PRIO_NORMAL;
	fetch_message->address = address;
	fetch_message->tgroup_home_cpu = tgroup_home_cpu;
	fetch_message->tgroup_home_id = tgroup_home_id;
	fetch_message->is_write = fetching_page->is_write;
	fetch_message->is_fetch= 1;
	fetch_message->vma_operation_index= current->mm->vma_operation_index;

	DSMPRINTK("Fetch for address %lu pid %d\n", address,current->pid);

	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	/*PTE UNLOCKED*/

	fetching_page->arrived_response= 0;
	reachable= 0;

	struct migration_memory* memory= find_migration_memory_memory_entry(tgroup_home_cpu,tgroup_home_id);

	down_read(&memory->kernel_set_sem);

	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;

		if(memory->kernel_set[i]==1){


			if ((ret=pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (fetch_message),sizeof(*fetch_message)-sizeof(struct pcn_kmsg_hdr)))
					!= -1) {
				reachable++;
				other_cpu= i;
				if(reachable>1)
					printk("ERROR: using optimized algorithm for 2 kernels with more than two kernels in %s\n", __func__);
			}
		}

	}

	up_read(&memory->kernel_set_sem);

	if(reachable>0){

		while (fetching_page->arrived_response==0) {

			set_task_state(current, TASK_UNINTERRUPTIBLE);

			if (fetching_page->arrived_response==0) {
				schedule();
			}

			set_task_state(current, TASK_RUNNING);
		}

	}else{
		printk("WARNING... reachable is %d in %s ret is %i",reachable,__func__, ret);
	}

	down_read(&mm->mmap_sem);
	spin_lock(ptl);
	/*PTE LOCKED*/

	DSMPRINTK("Out wait fetch address %lu \n", address);


	/*only the client has to update the vma*/
	if(tgroup_home_cpu!=_cpu)
	{
		if(fetching_page->vma_present==1){
			ret = do_mapping_for_distributed_process_from_page_fault(fetching_page->vm_flags, fetching_page->vaddr_start,
					fetching_page->vaddr_size, fetching_page->pgoff,fetching_page->path,
					mm, address, ptl);

			if (ret != 0)
				goto exit_fetch_message;
		}


		vma = find_vma(mm, address);
		if (!vma || address >= vma->vm_end || address < vma->vm_start) {
			vma = NULL;
		} else if (unlikely(is_vm_hugetlb_page(vma))
				|| unlikely(transparent_hugepage_enabled(vma))) {
			printk("ERROR: Installed a vma with HUGEPAGE\n");
			ret = VM_FAULT_VMA;
			goto exit_fetch_message;
		}

		if (vma == NULL) {
			printk("%s: ERROR: no vma for address %lu in the system {%d} \n",__func__, address,current->pid);
			ret = VM_FAULT_VMA;
			goto exit_fetch_message;
		}

	}

	if(_cpu==tgroup_home_cpu && fetching_page->address_present == 0){
		printk("ERROR: No response for a marked page in %s\n", __func__);
		ret = VM_FAULT_REPLICATION_PROTOCOL;
		goto exit_fetch_message;
	}

	/* case copy present on the other kernel
	 * */
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


		spin_lock(ptl);
		/*PTE LOCKED*/

		int status;

		/*if nobody changed the pte*/
		if (likely(pte_same(*pte, value_pte))) {

			if(fetching_page->is_write){

				status= REPLICATION_STATUS_WRITTEN;
				if(fetching_page->owner==0){
					printk("ERROR: copy of a page sent to a write fetch request without ownership in %s\n", __func__);
					pcn_kmsg_free_msg_now(fetching_page->data);
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_fetch_message;
				}

			}
			else{

				status= REPLICATION_STATUS_VALID;
				if(fetching_page->owner==1){
					printk("ERROR: copy of a page sent to a read fetch request with ownership in %s\n", __func__);
					pcn_kmsg_free_msg_now(fetching_page->data);
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_fetch_message;
				}
			}

			void *vto;
			void *vfrom;

			if (fetching_page->data->address != address) {
				printk("ERROR: trying to copy wrong address! in %s", __func__);
				pcn_kmsg_free_msg_now(fetching_page->data);
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_fetch_message;
			}


#if DIFF_PAGE

			if(fetching_page->data->diff==1){
				printk("ERROR: answered to a fetch with diff data\n");
				pcn_kmsg_free_msg_now(fetching_page->data);
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_fetch_message;
			}

			vto = kmap_atomic(page, KM_USER0);
			vfrom = &(fetching_page->data->data);

			if(fetching_page->data->data_size==PAGE_SIZE)
				copy_user_page(vto, vfrom, address, page);
			else{
				WKdm_decompress(vfrom,vto);
			}

			kunmap_atomic(vto, KM_USER0);

			if(status==REPLICATION_STATUS_WRITTEN){
				if(page->old_page_version==NULL){
					page->old_page_version= kmalloc(sizeof(char)*PAGE_SIZE,
							GFP_ATOMIC);
					if(page->old_page_version==NULL){
						printk("Impossible to kmalloc in %s\n",__func__);
						pcn_kmsg_free_msg_now(fetching_page->data);
						ret = VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_fetch_message;
					}
				}

				vto = page->old_page_version;
				vfrom = kmap_atomic(page, KM_USER0);
				memcpy(vto, vfrom, PAGE_SIZE);
				kunmap_atomic(vto, KM_USER0);
			}
#else
			vto = kmap_atomic(page, KM_USER0);
			vfrom = &(fetching_page->data->data);
			copy_user_page(vto, vfrom, address, page);
			kunmap_atomic(vto, KM_USER0);

#if CHECKSUM
			vto= kmap_atomic(page, KM_USER0);
			__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
			kunmap_atomic(vto, KM_USER0);
			__wsum check2= csum_partial(&(fetching_page->data->data), PAGE_SIZE, 0);


			if(check1!=check2) {
				printk("ERROR: page just copied is not matching, address %lu in %s\n",address, __func__);
				pcn_kmsg_free_msg_now(fetching_page->data);
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_fetch_message;
			}
			if(check1!=fetching_page->data->checksum) {
				printk("ERROR: page just copied is not matching the one sent, address %lu in %s\n",address, __func__);
				pcn_kmsg_free_msg_now(fetching_page->data);
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_fetch_message;
			}
#endif

#endif

			pcn_kmsg_free_msg_now(fetching_page->data);

			pte_t entry = mk_pte(page, vma->vm_page_prot);

			/*if the page is read only no need to keep replicas coherent*/
			if (vma->vm_flags & VM_WRITE) {

				page->replicated = 1;

				if(fetching_page->is_write){
					page->last_write = fetching_page->last_write+1;
				}
				else
					page->last_write = fetching_page->last_write;


				page->owner = fetching_page->owner;

				page->status = status;

				if (status == REPLICATION_STATUS_VALID) {
					entry = pte_clear_flags(entry, _PAGE_RW);
				} else {
					entry = pte_set_flags(entry, _PAGE_RW);
				}

			} else {
				if(fetching_page->is_write)
					printk("ERROR: trying to write a read only page in %s\n",__func__);

				if(fetching_page->owner==1)
					printk("ERROR: received ownership with a copy of a read only page in %s\n", __func__);

				page->replicated = 0;
				page->owner= 0;
				page->status= REPLICATION_STATUS_NOT_REPLICATED;

			}

			entry = pte_set_flags(entry, _PAGE_PRESENT);
			page->other_owners[_cpu]=1;
			page->other_owners[other_cpu]=1;
			page->futex_owner = fetching_page->futex_owner;

			flush_cache_page(vma, address, pte_pfn(*pte));

			entry = pte_set_flags(entry, _PAGE_USER);
			entry = pte_set_flags(entry, _PAGE_ACCESSED);


			ptep_clear_flush(vma, address, pte);



			page_add_new_anon_rmap(page, vma, address);
			set_pte_at_notify(mm, address, pte, entry);

			update_mmu_cache(vma, address, pte);


		} else {
			status = REPLICATION_STATUS_INVALID;
			mem_cgroup_uncharge_page(page);
			page_cache_release(page);
			pcn_kmsg_free_msg_now(fetching_page->data);

		}

		DSMPRINTK("End fetching address %lu \n", address);
		ret= 0;
		goto exit_fetch_message;

	}

	/*copy not present on the other kernel*/
	else {


		DSMPRINTK("Copy not present in the other kernel, local fetch of address %lu\n", address);
		pcn_kmsg_free_msg(fetch_message);
		ret = VM_CONTINUE_WITH_CHECK;
		goto exit;
	}

	exit_fetch_message:

	pcn_kmsg_free_msg(fetch_message);

	exit_fetching_page:

	remove_mapping_entry(fetching_page);
	kfree(fetching_page);

	exit:

	return ret;
}

/* Function to handle page_fault in Popcorn.
 * Must be called with down_read in both mm->mmap_sem and mm->distribute_sem.
 * It calls corresponding functions according if it is a read, write or fetch.
 */
int popcorn_try_handle_mm_fault(struct task_struct *tsk,
		struct mm_struct *mm, struct vm_area_struct *vma,
		unsigned long page_faul_address, unsigned long page_fault_flags,
		unsigned long error_code) {

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

	if(page_fault_flags & FAULT_FLAG_WRITE){
		DSMPRINTK("Page fault address %lu in page %lu task pid %d current pid %d {%d,%d} write \n",  page_faul_address, address, tsk->pid,current->pid, tgroup_home_cpu, tgroup_home_id);

	}
	else{
		DSMPRINTK("Page fault address %lu in page %lu task pid %d current pid %d {%d,%d} read \n",  page_faul_address, address, tsk->pid,current->pid, tgroup_home_cpu, tgroup_home_id);
	}

	if (address == 0) {
		printk("ERROR: accessing page at address 0 pid %i in %s\n",tsk->pid, __func__);
		return VM_FAULT_ACCESS_ERROR | VM_FAULT_VMA;
	}


	if (vma && (address < vma->vm_end && address >= vma->vm_start)
			&& (unlikely(is_vm_hugetlb_page(vma))
					|| transparent_hugepage_enabled(vma))) {
		printk("ERROR: page fault for huge page\n");
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
		printk("ERROR: page fault for huge page\n");
		return VM_CONTINUE;
	}

	pte = pte_offset_map_lock(mm, pmd, address, &ptl);
	/*PTE LOCKED*/

	value_pte = *pte;

	/*case pte UNMAPPED
	 * --Remote fetch--
	 */
	start: if (pte == NULL || pte_none(pte_clear_flags(value_pte, _PAGE_UNUSED1))) {

		/* Check if other threads of my process are already
		 * fetching the same address on this kernel.
		 */
		if (find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address) != NULL) {


			/*wait while the fetch is ended*/
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);

			while (find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address)
					!= NULL) {

				DEFINE_WAIT(wait);
				prepare_to_wait(&read_write_wait, &wait, TASK_UNINTERRUPTIBLE);

				if (find_mapping_entry(tgroup_home_cpu, tgroup_home_id,
						address)!=NULL) {
					schedule();
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

				printk(
						"ERROR: vma not valid after waiting for another thread to fetch\n");
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

	/*case pte MAPPED
	 */
	else {


		/* There can be an unluckily case in which I am still fetching...
		 */
		struct page_fault_mapping* fetch= find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address);
		if (fetch != NULL && fetch->is_fetch==1) {

			/*wait while the fetch is ended*/
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

				printk("ERROR: vma not valid after waiting for another thread to fetch\n");
				spin_unlock(ptl);
				return VM_FAULT_VMA;
			}

			goto start;
		}

		/* The pte is mapped so the vma should be valid.
		 * Check if the access is within the limit.
		 */
		if (unlikely(!vma || address >= vma->vm_end || address < vma->vm_start)) {
			printk("ERROR: no vma for address %lu in the system\n", address);
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
			DSMPRINTK("page different from vm_normal_page\n");
		}
		/* case page NOT REPLICATED
		 */
		if (page->replicated == 0) {
			DSMPRINTK("Page not replicated task pid %d current pid %d address %lu \n",tsk->pid,current->pid, address);

			/*check if it a cow page...*/
			if ((vma->vm_flags & VM_WRITE) && !pte_write(value_pte)) {

				retry_cow:
				DSMPRINTK("COW page at %lu \n", address);

				int cow_ret= do_wp_page_for_popcorn(mm, vma,address, pte,pmd,ptl, value_pte);

				if (cow_ret & VM_FAULT_ERROR) {
					if (cow_ret & VM_FAULT_OOM){
						printk("ERROR: %s VM_FAULT_OOM\n",__func__);
						return VM_FAULT_OOM;
					}

					if (cow_ret & (VM_FAULT_HWPOISON | VM_FAULT_HWPOISON_LARGE)){
						printk("ERROR: %s EHWPOISON\n",__func__);
						return VM_FAULT_OOM;
					}

					if (cow_ret & VM_FAULT_SIGBUS){
						printk("ERROR: %s EFAULT\n",__func__);
						return VM_FAULT_OOM;
					}

					printk("ERROR: %s bug from do_wp_page_for_popcorn\n",__func__);
					return VM_FAULT_OOM;
				}

				spin_lock(ptl);
				/*PTE LOCKED*/

				value_pte = *pte;

				if(!pte_write(value_pte)){
					printk("WARNING: page not writable after cow in %s\n", __func__);
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

			DSMPRINTK("Page status valid address %lu task pid %d current pid %d \n",address, tsk->pid,current->pid);

			/*read case
			 */
			if (!(page_fault_flags & FAULT_FLAG_WRITE)) {
				spin_unlock(ptl);

				return 0;
			}

			/*write case
			 */
			else {

				/* If other threads of this process are writing or reading in this kernel, I wait.
				 *
				 * I wait for concurrent writes because after a write the status is updated to REPLICATION_STATUS_WRITTEN,
				 * so only the first write needs to send the invalidation message.
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

						printk(	"ERROR: vma not valid after waiting for another thread to fetch\n");
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
		} else

			/* case REPLICATION_STATUS_WRITTEN
			 * both read and write can be performed on this page.
			 * */
			if (page->status == REPLICATION_STATUS_WRITTEN) {
				DSMPRINTK("Page status written task pid %d current pid %d address %lu \n",tsk->pid,current->pid, address);
				spin_unlock(ptl);

				return 0;
			}

			else {

				if (!(page->status == REPLICATION_STATUS_INVALID)) {
					printk("ERROR: Page status not correct on address %lu \n",
							address);
					spin_unlock(ptl);
					return VM_FAULT_REPLICATION_PROTOCOL;
				}

				DSMPRINTK("Page status invalid address %lu task pid %d current pid %d \n",address,tsk->pid,current->pid);

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

						printk("ERROR: vma not valid after waiting for another thread to fetch\n");
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


static int __init popcorn_user_dsm_init(void) {


	uint16_t copy_cpu;
	if(scif_get_nodeIDs(NULL, 0, &copy_cpu)==-1){
		printk("ERROR %s cannot initialize _cpu\n", __func__);
	}
	else{
		_cpu= copy_cpu;
	}

	message_request_wq = create_workqueue("request_wq");
	invalid_message_wq= create_workqueue("invalid_wq");

	/*
	 * Register to receive relevant incoming messages.
	 */
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_MAPPING_REQUEST,
			handle_mapping_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_MAPPING_RESPONSE,
			handle_mapping_response);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_MAPPING_RESPONSE_VOID,
			handle_mapping_response_void);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_INVALID,
			handle_invalid_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_ACK_INVALID,
			handle_ack_invalid);

	return 0;
}

late_initcall_popcorn(popcorn_user_dsm_init);
