/*
 * futex_remote.c
 *
 *  Created on: Oct 8, 2013
 *      Author: akshay
 */

#include <linux/slab.h>
#include <linux/init.h>
#include <linux/mount.h>
#include <linux/pagemap.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/futex.h>


#include <linux/pid.h>
#include <linux/types.h>

#include <linux/pcn_kmsg.h>
#include <linux/delay.h>
#include <popcorn/remote_pfn.h>
#include <popcorn/pid.h>


#include "futex_remote.h"

extern struct list_head pfn_list_head;

#define  NSIG 32

static struct list_head vm_head;

struct list_head fq_head;


int find_kernel_for_pfn(unsigned long addr, struct list_head *head)
{
    struct list_head *iter;
    _pfn_range_list_t *objPtr;

    list_for_each(iter, head) {
        objPtr = list_entry(iter, _pfn_range_list_t, pfn_list_member);
        if(addr > objPtr->start_pfn_addr && addr < objPtr->end_pfn_addr)
        	return objPtr->kernel_number;
    }

    return -1;
}
struct _inc_remote_vm_pool {
	unsigned long start;
	unsigned long end;
	unsigned long rflag;
	int pid;
	int origin_pid;
	struct list_head list_member;
};

typedef struct _inc_remote_vm_pool _inc_remote_vm_pool_t;

_inc_remote_vm_pool_t * add_inc(int pid,int start,int end, int rflag,int origin_pid, struct list_head *head) {
	_inc_remote_vm_pool_t *Ptr = (_inc_remote_vm_pool_t *) kmalloc(
			sizeof(_inc_remote_vm_pool_t), GFP_ATOMIC);

	Ptr->start = start;
	Ptr->pid = pid;
	Ptr->end = end;
	Ptr->rflag = rflag;
	Ptr->origin_pid=origin_pid;
	INIT_LIST_HEAD(&Ptr->list_member);
	list_add(&Ptr->list_member, head);

	return Ptr;
}

int find_and_delete_inc(int pid, struct list_head *head) {
	struct list_head *iter;
	_inc_remote_vm_pool_t *objPtr;

	list_for_each(iter, head)
	{
		objPtr = list_entry(iter, _inc_remote_vm_pool_t, list_member);
		if (objPtr->pid == pid) {
			list_del(&objPtr->list_member);
			kfree(objPtr);
			return 1;
		}
	}
}

_inc_remote_vm_pool_t * find_inc(int start,int end, struct list_head *head) {
	struct list_head *iter;
	_inc_remote_vm_pool_t *objPtr;

	list_for_each(iter, head)
	{
		objPtr = list_entry(iter, _inc_remote_vm_pool_t, list_member);
		if (objPtr->start == start && objPtr->end == end) {
			return objPtr;
		}
	}
	return NULL;
}

struct pid_list
{
	int pid;
	struct list_head pid_list_member;
};


_global_futex_key_t * add_key(int pid,int address, struct list_head *head) {


	_global_futex_key_t *Ptr;
	if(Ptr = find_key(address,&head))
	{
		int i=0;
		for(i=0;i<10;i++)
		{
			if(Ptr->pid[i]!=0) continue;
			else
				Ptr->pid[i]=pid; break;
		}
	}
	else
	{
	   Ptr	= (_global_futex_key_t *) kmalloc(
			sizeof(_global_futex_key_t), GFP_ATOMIC);
	   Ptr->address = address;
	   memset(Ptr->pid,0,sizeof(Ptr->pid));
	   Ptr->pid[0] = pid;
	   INIT_LIST_HEAD(&Ptr->list_member);
	   list_add(&Ptr->list_member, head);
	}
	return Ptr;
}

int find_and_delete_key(int address, struct list_head *head) {
	struct list_head *iter;
	_global_futex_key_t *objPtr;

	list_for_each(iter, head)
	{
		objPtr = list_entry(iter, _global_futex_key_t, list_member);
		if (objPtr->address == address) {
			list_del(&objPtr->list_member);
			kfree(objPtr);
			return 1;
		}
	}
}

_global_futex_key_t * find_key(int address, struct list_head *head) {
	struct list_head *iter;
	_global_futex_key_t *objPtr;

	list_for_each(iter, head)
	{
		objPtr = list_entry(iter, _global_futex_key_t, list_member);
		if (objPtr->address == address) {
			return objPtr;
		}
		if(head->next->next==head->next->prev) break;
	}
	return NULL;
}

struct _remote_wakeup_request {
	struct pcn_kmsg_hdr header;
	unsigned long uaddr;
	unsigned int flags;
	int nr_wake;
	u32 bitset;
	unsigned long start;
	unsigned long end;
	int rflag;
	int pid;
	int origin_pid;
	char pad_string[4];
}__attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_wakeup_request _remote_wakeup_request_t;

struct _remote_wakeup_response {
	struct pcn_kmsg_hdr header;
	int errno;
	int request_id;
	char pad_string[52];
}__attribute__((packed)) __attribute__((aligned(64)));


typedef struct _remote_wakeup_response _remote_wakeup_response_t;



struct futex_q * query_q(struct task_struct *t) {
	int i = 0;
	struct futex_hash_bucket *hb;
	struct plist_head *head;
	struct futex_q *this, *next;
	int end = 1 << _FUTEX_HASHBITS;
	//pid_task(find_vpid(p->tgid), PIDTYPE_PID);
	struct task_struct *temp;
	for (i = 0; i < end; i++) {
		hb = &futex_queues[i];
		if (hb != NULL) {
			spin_lock(&hb->lock);
			head = &hb->chain;
			plist_for_each_entry_safe(this, next, head, list)
			{
				temp=this->task;
				if(temp->tgroup_distributed == 1 && temp->tgroup_home_id == t->tgroup_home_id){
					spin_unlock(&hb->lock);
					return this;
				}
			}
			spin_unlock(&hb->lock);
		}
	}
	return NULL;
}


struct futex_q * query_q_pid(struct task_struct *t) {
	int i = 0;
	struct futex_hash_bucket *hb;
	struct plist_head *head;
	struct futex_q *this, *next;
	int end = 1 << _FUTEX_HASHBITS;
	struct task_struct *temp;
	for (i = 0; i < end; i++) {
		hb = &futex_queues[i];
		if (hb != NULL) {
			head = &hb->chain;
			plist_for_each_entry_safe(this, next, head, list)
			{
				temp=this->task;
				if(temp->tgroup_distributed == 1 && temp->tgroup_home_id == t->tgroup_home_id && temp->pid == t->pid){
					//spin_unlock(&hb->lock);
					return this;
				}
			}
		}
	}
	return NULL;
}


int global_futex_wake(u32 __user *uaddr, unsigned int flags, int nr_wake, u32 bitset, pid_t pid)
{
	struct futex_hash_bucket *hb;
	struct futex_q *this, *next;
	struct plist_head *head;
	struct task_struct *tsk;
	union futex_key key = FUTEX_KEY_INIT;
	int ret;
	printk(KERN_ALERT "%s: response {%d} uaddr{%lx} \n","global_futex_wake",pid,uaddr);
	if (!bitset)
		return -EINVAL;

	ret = get_futex_key(uaddr, flags & FLAGS_SHARED, &key, VERIFY_READ);

	hb = hash_futex(&key);
	spin_lock(&hb->lock);
	head = &hb->chain;
	tsk=pid_task(find_vpid(pid), PIDTYPE_PID);
	if(tsk){
	this = query_q_pid(tsk);
	spin_unlock(&this->lock_ptr);
	wake_futex(this);
	}

	spin_unlock(&hb->lock);
	put_futex_key(&key);
out:
	printk(KERN_ALERT "exit global_futex_wake \n");
	return ret;
}

static int handle_remote_futex_wake_response(struct pcn_kmsg_message* inc_msg) {
	_remote_wakeup_response_t* msg = (_remote_wakeup_response_t*) inc_msg;

	printk(KERN_ALERT"%s: response {%d} \n",
			"handle_remote_futex_wake_response", msg->errno);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

static int handle_remote_futex_wake_request(struct pcn_kmsg_message* inc_msg) {

	_remote_wakeup_request_t* msg = (_remote_wakeup_request_t*) inc_msg;
	_remote_wakeup_response_t response;
	struct task_struct *tsk=current;
	struct mm_struct *cmm = NULL;

	printk(KERN_ALERT"%s: request -- entered task comm{%s} pid{%d}\n", "handle_remote_futex_wake_request",tsk->comm,tsk->pid);
	printk(KERN_ALERT"%s: msg: uaddr {%lx} flags {%x} nr_wake{%d} bitset {%u} rflag{%d} pid{%d} origin_pid {%d} \n",
			"handle_remote_futex_wake_request",msg->uaddr,msg->flags,msg->nr_wake,msg->bitset,msg->rflag,msg->pid,msg->origin_pid);

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	//add_inc( msg->pid,msg->start,msg->end,msg->rflag,msg->origin_pid, &vm_head);

	if(msg->rflag == 0)
	{
	  tsk = pid_task(find_vpid(msg->origin_pid), PIDTYPE_PID);
	  if(tsk && current->mm!=NULL)
	  {
	       cmm = current->mm;
	       current->mm = tsk->mm;
	  }
	  else if(current->mm==NULL)
	       printk(KERN_ALERT "mm struct doesntexist\n");

	  futex_wake(msg->uaddr,msg->flags,msg->nr_wake,msg->bitset);
	  if(cmm !=NULL)
	     current->mm = cmm;
	}
	else
	 global_futex_wake(msg->uaddr,msg->flags,msg->nr_wake,msg->bitset, msg->rflag);


	printk(KERN_ALERT " handle_remote_futex_wake_req after setting mm to NULL\n");
	pcn_kmsg_free_msg(inc_msg);

	return 0;
}


int remote_futex_wakeup(unsigned long uaddr,unsigned int flags, int nr_wake, u32 bitset,union futex_key *key, int rflag ) {

	int res = 0;
	int cpu=0;

	_remote_wakeup_request_t *request = kmalloc(sizeof(_remote_wakeup_request_t),
	GFP_KERNEL);

	// Build request

	request->header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;

	request->bitset = bitset;
	request->nr_wake = nr_wake;
	request->flags = flags;
	request->uaddr = uaddr;

	unsigned long address=(unsigned long)uaddr;
	key->both.offset = address % PAGE_SIZE;
	if (unlikely((address % sizeof(u32)) != 0))
					return -EINVAL;
	address -= key->both.offset;

	unsigned long vm_flags;
	struct vm_area_struct *vma;
	struct vm_area_struct* curr = NULL;
	curr  = current->mm->mmap;
	vma = find_extend_vma( current->mm, address);
	request->start = curr->vm_start;
	request->end = curr->vm_end;
	request->rflag = rflag;
	request->pid = current->pid;
	request->origin_pid = current->origin_pid;

	printk(KERN_ALERT" remote_futex_wakeup pfn {%lx} shift {%lx} pid{%d} origin_pid{%d} cpu{%d} rflag{%d} uaddr{%lx}\n ",
								vma->vm_pgoff,vma->vm_pgoff << PAGE_SHIFT,current->pid,current->origin_pid,smp_processor_id(),rflag,uaddr);


			if(vma->vm_flags & VM_PFNMAP)	{

//			printk(KERN_ALERT" remote_futex_wakeup pfn {%lx} shift {%lx} pid{%d} origin_pid{%d} cpu{%d} rflag{%d} uaddr{%lx} \n ",
//					vma->vm_pgoff,vma->vm_pgoff << PAGE_SHIFT,current->pid,current->origin_pid,smp_processor_id(),rflag,uaddr);

				// Send response
			if(!rflag)
			{

				if((cpu=find_kernel_for_pfn(vma->vm_pgoff << PAGE_SHIFT,&pfn_list_head)) != -1)
				res = pcn_kmsg_send(cpu, (struct pcn_kmsg_message*) (request));
				else
			        res=-1;
			}

			}

			else if(rflag)
			    res = pcn_kmsg_send(ORIG_NODE(rflag), (struct pcn_kmsg_message*) (request));

	return res;

}




int global_futex_wait(u32 __user *uaddr, unsigned int flags, u32 val,
		      ktime_t *abs_time, u32 bitset, pid_t rem, pid_t origin)
{
	struct futex_hash_bucket *hb;
	struct task_struct *tsk = NULL;
	struct futex_q *q = (struct futex_q *) kmalloc(
			sizeof(struct futex_q), GFP_ATOMIC); //futex_q_init;
	q->key = FUTEX_KEY_INIT;
	q->bitset =FUTEX_BITSET_MATCH_ANY;
	q->rem_pid =-1;
	u32 uval;
	int ret;
	int sig;
	/*if (!bitset)
		return -EINVAL;*/
	q->bitset = bitset;

    ret = get_futex_key(uaddr, flags & FLAGS_SHARED, &q->key, VERIFY_READ);
    printk(KERN_ALERT "global_futex_wait pid origin {%d} _cpu{%d} uaddr{%lx}\n ",origin,smp_processor_id(),uaddr);


    hb = hash_futex(&q->key);
    q->lock_ptr = &hb->lock;
//this should be the real code

    spin_lock(&hb->lock);
     	int prio;
    	prio = 100 ;//min(current->normal_prio, MAX_RT_PRIO);
    	q->task = NULL;
    	q->rem_pid = rem;
    	tsk=pid_task(find_vpid(origin), PIDTYPE_PID);
        if(tsk)
        	q->key.private.mm=tsk->mm;

    	q->key.private.offset=156;
    	plist_node_init(&q->list, prio);
    	plist_add(&q->list, &hb->chain);
    spin_unlock(&hb->lock);
out:
    	printk(KERN_ALERT "exit glabal_futex_wait \n");
	return ret;
}


struct _remote_key_request {
	struct pcn_kmsg_hdr header;
	unsigned long uaddr;
	unsigned int flags;
	int rw;
	int pid;
	int origin_pid;
	int val;
	char pad_string[28];
}__attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_key_request _remote_key_request_t;

struct _remote_key_response {
	struct pcn_kmsg_hdr header;
	int errno;
	int request_id;
	char pad_string[52];
}__attribute__((packed)) __attribute__((aligned(64)));


typedef struct _remote_key_response _remote_key_response_t;

static int handle_remote_futex_key_response(struct pcn_kmsg_message* inc_msg) {
	_remote_key_response_t* msg = (_remote_key_response_t*) inc_msg;

	printk(KERN_ALERT"%s: response {%d} \n",
			"handle_remote_futex_key_response", msg->errno);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

static int handle_remote_futex_key_request(struct pcn_kmsg_message* inc_msg) {

	_remote_key_request_t* msg = (_remote_key_request_t*) inc_msg;
	_remote_key_response_t response;

	printk(KERN_ALERT"%s: request -- entered \n", "handle_remote_futex_key_request");
	printk(KERN_ALERT"%s: msg: uaddr {%lx} flags {%x} val{%d}  pid{%d} origin_pid {%d} \n",
				"handle_remote_futex_key_request",msg->uaddr,msg->flags,msg->val,msg->pid,msg->origin_pid);

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	global_futex_wait(msg->uaddr,msg->flags,msg->val,0,0,msg->pid,msg->origin_pid);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}


int
get_set_remote_key(unsigned long uaddr, unsigned int val, int fshared, union futex_key *key, int rw)
{

	int res = 0;
	_remote_key_request_t *request = kmalloc(sizeof(_remote_key_request_t),
			GFP_KERNEL);

	// Build request
	request->header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;

	// Send response

	unsigned long address=(unsigned long)uaddr;
	key->both.offset = address % PAGE_SIZE;
	if (unlikely((address % sizeof(u32)) != 0))
		return -EINVAL;
	address -= key->both.offset;

	unsigned long vm_flags;
	struct vm_area_struct *vma;
	struct vm_area_struct* curr = NULL;
	curr  = current->mm->mmap;
	vma = find_extend_vma( current->mm, address);

	if(vma->vm_flags & VM_PFNMAP)	{
		request->flags = fshared;
		request->uaddr =(unsigned long)uaddr;
		request->rw = rw;
		request->pid = current->pid;
		request->origin_pid = current->origin_pid;
		request->val =val;
		printk(KERN_ALERT" pfn {%lx} shift {%lx} \n ",vma->vm_pgoff,vma->vm_pgoff << PAGE_SHIFT);

		// Send response
		res = pcn_kmsg_send(find_kernel_for_pfn(vma->vm_pgoff << PAGE_SHIFT,&pfn_list_head), (struct pcn_kmsg_message*) (request));
	}
	return 0;
}

int
get_futex_key_remote(u32 __user *uaddr, int fshared, union futex_key *key, int rw)
{
	unsigned long address = (unsigned long)uaddr;
	struct mm_struct *mm = current->mm;
	if (mm == NULL) {
		struct list_head *iter;
		_inc_remote_vm_pool_t *objPtr;
		struct task_struct *g, *p;

		list_for_each(iter, &vm_head)
		{
			struct vm_area_struct *vma;
			objPtr = list_entry(iter, _inc_remote_vm_pool_t, list_member);

			struct task_struct *q;
			q = query_q(pid_task(find_vpid(objPtr->origin_pid), PIDTYPE_PID))->task;
			if (!q) {
				do_each_thread(g, p)
				{
					if (p->represents_remote == 1
							&& p->next_pid == objPtr->pid) {
						vma = p->mm->mmap;
						q = pid_task(find_vpid(p->tgid), PIDTYPE_PID);
					}
				}
				while_each_thread(g, p);
			}
			__set_task_state(q, TASK_INTERRUPTIBLE);
			mm = q->mm;
			break;

		}

	}
	struct page *page, *page_head;
	int err, ro = 0;

	/*
	 * The futex address must be "naturally" aligned.
	 */
	key->both.offset = address % PAGE_SIZE;
	if (unlikely((address % sizeof(u32)) != 0))
		return -EINVAL;
	address -= key->both.offset;

	/*
	 * PROCESS_PRIVATE futexes are fast.
	 * As the mm cannot disappear under us and the 'key' only needs
	 * virtual address, we dont even have to find the underlying vma.
	 * Note : We do have to check 'uaddr' is a valid user address,
	 *        but access_ok() should be faster than find_vma()
	 */
	if (!fshared) {
		if (unlikely(!access_ok(VERIFY_WRITE, uaddr, sizeof(u32))))
			return -EFAULT;
		key->private.mm = mm;
		key->private.address = address;
		get_futex_key_refs(key);
		return 0;
	}

again:
	err = get_user_pages_fast_mm(mm,address, 1, 1, &page);
	/*
	 * If write access is not required (eg. FUTEX_WAIT), try
	 * and get read-only access.
	 */
	if (err == -EFAULT && rw == VERIFY_READ) {
		err = get_user_pages_fast_mm(mm,address, 1, 0, &page);
		ro = 1;
	}

	if (err < 0)
		return err;
	else
		err = 0;

	unsigned long pfn=page_to_pfn(page);
	 printk(KERN_ALERT"futex pfn : 0x{%lx}\n",PFN_PHYS(pfn));

#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	page_head = page;
	if (unlikely(PageTail(page))) {
		put_page(page);
		/* serialize against __split_huge_page_splitting() */
		local_irq_disable();
		if (likely(__get_user_pages_fast(address, 1, 1, &page) == 1)) {
			page_head = compound_head(page);
			/*
			 * page_head is valid pointer but we must pin
			 * it before taking the PG_lock and/or
			 * PG_compound_lock. The moment we re-enable
			 * irqs __split_huge_page_splitting() can
			 * return and the head page can be freed from
			 * under us. We can't take the PG_lock and/or
			 * PG_compound_lock on a page that could be
			 * freed from under us.
			 */
			if (page != page_head) {
				get_page(page_head);
				put_page(page);
			}
			local_irq_enable();
		} else {
			local_irq_enable();
			goto again;
		}
	}
#else
	page_head = compound_head(page);
	if (page != page_head) {
		get_page(page_head);
		put_page(page);
	}
#endif

	lock_page(page_head);

	/*
	 * If page_head->mapping is NULL, then it cannot be a PageAnon
	 * page; but it might be the ZERO_PAGE or in the gate area or
	 * in a special mapping (all cases which we are happy to fail);
	 * or it may have been a good file page when get_user_pages_fast
	 * found it, but truncated or holepunched or subjected to
	 * invalidate_complete_page2 before we got the page lock (also
	 * cases which we are happy to fail).  And we hold a reference,
	 * so refcount care in invalidate_complete_page's remove_mapping
	 * prevents drop_caches from setting mapping to NULL beneath us.
	 *
	 * The case we do have to guard against is when memory pressure made
	 * shmem_writepage move it from filecache to swapcache beneath us:
	 * an unlikely race, but we do need to retry for page_head->mapping.
	 */
	if (!page_head->mapping) {
		int shmem_swizzled = PageSwapCache(page_head);
		unlock_page(page_head);
		put_page(page_head);
		if (shmem_swizzled)
			goto again;
		return -EFAULT;
	}

	/*
	 * Private mappings are handled in a simple way.
	 *
	 * NOTE: When userspace waits on a MAP_SHARED mapping, even if
	 * it's a read-only handle, it's expected that futexes attach to
	 * the object not the particular process.
	 */
	if (PageAnon(page_head)) {
		/*
		 * A RO anonymous page will never change and thus doesn't make
		 * sense for futex operations.
		 */
		if (ro) {
			err = -EFAULT;
			goto out;
		}

		key->both.offset |= FUT_OFF_MMSHARED; /* ref taken on mm */
		key->private.mm = mm;
		key->private.address = address;
	} else {
		key->both.offset |= FUT_OFF_INODE; /* inode-based key */
		key->shared.inode = page_head->mapping->host;
		key->shared.pgoff = page_head->index;
	}

	get_futex_key_refs(key);

out:
	unlock_page(page_head);
	put_page(page_head);
	return err;
}



static int __init futex_remote_init(void)
{



	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_REQUEST,
			handle_remote_futex_wake_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_RESPONSE,
			handle_remote_futex_wake_response);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_REQUEST,
			handle_remote_futex_key_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_RESPONSE,
			handle_remote_futex_key_response);
	INIT_LIST_HEAD(&vm_head);
	INIT_LIST_HEAD(&fq_head);


	return 0;
}
__initcall(futex_remote_init);



/*mklinux_akshay*/
