/*
 *  Fast Userspace Mutexes (which I call "Futexes!").
 *  (C) Rusty Russell, IBM 2002
 *
 *  Generalized futexes, futex requeueing, misc fixes by Ingo Molnar
 *  (C) Copyright 2003 Red Hat Inc, All Rights Reserved
 *
 *  Removed page pinning, fix privately mapped COW pages and other cleanups
 *  (C) Copyright 2003, 2004 Jamie Lokier
 *
 *  Robust futex support started by Ingo Molnar
 *  (C) Copyright 2006 Red Hat Inc, All Rights Reserved
 *  Thanks to Thomas Gleixner for suggestions, analysis and fixes.
 *
 *  PI-futex support started by Ingo Molnar and Thomas Gleixner
 *  Copyright (C) 2006 Red Hat, Inc., Ingo Molnar <mingo@redhat.com>
 *  Copyright (C) 2006 Timesys Corp., Thomas Gleixner <tglx@timesys.com>
 *
 *  PRIVATE futexes by Eric Dumazet
 *  Copyright (C) 2007 Eric Dumazet <dada1@cosmosbay.com>
 *
 *  Requeue-PI support by Darren Hart <dvhltc@us.ibm.com>
 *  Copyright (C) IBM Corporation, 2009
 *  Thanks to Thomas Gleixner for conceptual design and careful reviews.
 *
 *  Thanks to Ben LaHaise for yelling "hashed waitqueues" loudly
 *  enough at me, Linus for the original (flawed) idea, Matthew
 *  Kirkwood for proof-of-concept implementation.
 *
 *  "The futexes are also cursed."
 *  "But they come in a choice of three flavours!"
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/jhash.h>
#include <linux/init.h>
#include <linux/futex.h>
#include <linux/mount.h>
#include <linux/pagemap.h>
#include <linux/syscalls.h>
#include <linux/signal.h>
#include <linux/export.h>
#include <linux/magic.h>
#include <linux/pid.h>
#include <linux/nsproxy.h>
#include <linux/ptrace.h>
#include <linux/sched/rt.h>
#include <linux/hugetlb.h>
#include <linux/freezer.h>

#include <asm/futex.h>

#include "rtmutex_common.h"

#include "futex_remote.h"
#include <popcorn/global_spinlock.h>

#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <linux/mmu_context.h>
#include <linux/string.h>
#include <popcorn/init.h>

#define FUTEX_VERBOSE 0 
#if FUTEX_VERBOSE
#define FPRINTK(...) printk(__VA_ARGS__)
#else
#define FPRINTK(...) ;
#endif
#define WAIT_MAIN 99
#define ENOTINKRN 999
static void printPTE(u32 __user *uaddr);
int __read_mostly futex_cmpxchg_enabled;
static int _cpu=0;

#define FUTEX_HASHBITS (CONFIG_BASE_SMALL ? 4 : 8)

/*
 * Futex flags used to encode options to functions and preserve them across
 * restarts.
 */
#define FLAGS_SHARED		0x01
#define FLAGS_CLOCKRT		0x02
#define FLAGS_HAS_TIMEOUT	0x04

/*extern functions*/
#if defined(CONFIG_ARM64)
unsigned long futex_atomic_add(unsigned long ptr, unsigned long val);
#endif

static unsigned long long _wait=0,_wake=0,_wakeop=0,_requeue=0;
static unsigned int _wait_cnt=0,_wake_cnt=0,_wakeop_cnt=0,_requeue_cnt=0;
static unsigned int _wait_err=0,_wake_err=0,_wakeop_err=0,_requeue_err=0;
static unsigned long long perf_aa,perf_bb,perf_cc;


//static
const struct futex_q futex_q_init = {
	/* list gets initialized in queue_me()*/
	.key = FUTEX_KEY_INIT,
	.bitset = FUTEX_BITSET_MATCH_ANY,
	.rem_pid = -1,
	.rem_requeue_key = FUTEX_KEY_INIT,
	.req_addr = 0
};


/*mklinux_akshay*/ //static
struct futex_hash_bucket futex_queues[1<<FUTEX_HASHBITS];

/*
 * We hash on the keys returned from get_futex_key (see below).
 */
//static
struct futex_hash_bucket *hash_futex(union futex_key *key)
{
	u32 hash = jhash2((u32*)&key->both.word,
			(sizeof(key->both.word)+sizeof(key->both.ptr))/4,
			// (sizeof(key->both.word)+8)/4,
			key->both.offset);
	return &futex_queues[hash & ((1 << FUTEX_HASHBITS)-1)];
}

/*
 * Return 1 if two futex_keys are equal, 0 otherwise.
 */
//static inline
int match_futex(union futex_key *key1, union futex_key *key2)
{
	return (key1 && key2
			&& key1->both.word == key2->both.word
			&& key1->both.ptr == key2->both.ptr
			&& key1->both.offset == key2->both.offset);
}

/*
 * Take a reference to the resource addressed by a key.
 * Can be called while holding spinlocks.
 *
 */
/*mklinux_akshay*///static
void get_futex_key_refs(union futex_key *key)
{
	if (!key->both.ptr)
		return;

	switch (key->both.offset & (FUT_OFF_INODE|FUT_OFF_MMSHARED)) {
		case FUT_OFF_INODE:
			ihold(key->shared.inode);
			break;
		case FUT_OFF_MMSHARED:
			atomic_inc(&key->private.mm->mm_count);
			break;
	}
}

/*
 * Drop a reference to the resource addressed by a key.
 * The hash bucket spinlock must not be held.
 */
static void drop_futex_key_refs(union futex_key *key)
{
	if (!key->both.ptr) {
		/* If we're here then we tried to put a key we failed to get */
		WARN_ON_ONCE(1);
		return;
	}

	switch (key->both.offset & (FUT_OFF_INODE|FUT_OFF_MMSHARED)) {
		case FUT_OFF_INODE:
			iput(key->shared.inode);
			break;
		case FUT_OFF_MMSHARED:
			mmdrop(key->private.mm);
			break;
	}
}
int setFutexOwnerToPage(unsigned long address, unsigned long uaddr){


	struct vm_area_struct *vma;
	struct page * pg;
	int err=0;
	//Get it for write access
	err = get_user_pages_fast(address, 1, 1, &pg);
	if (err < 0){
		FPRINTK(KERN_ALERT "%s: error getting page  {%d}\n",__func__,err);
		return err;
	}
	else
		err = 0;

	if(pg){
		vma = getVMAfromUaddr(uaddr);
		//check if I am the owner kernel for this page
		//if((!(vma->vm_flags & VM_PFNMAP) && !(vma->vm_flags & VM_MIXEDMAP))){
		if(_cpu == 0)//TODO :figure out origin kernel
		{	
			if(pg->futex_owner != Kernel_Id){
				FPRINTK(KERN_ALERT "%s: setting up owner for addr {%lu} \n",__func__,uaddr);
				pg->futex_owner = Kernel_Id;
			}
			return  pg->futex_owner;
		}
	}
	return -1;
	}

	int getFutexOwnerFromPage(unsigned long uaddr){
		struct vm_area_struct *vma;
		struct page * pg;
		unsigned long address = (unsigned long)uaddr;
		unsigned int offset =  address % PAGE_SIZE;
		address -= offset;
		int err = 0;
		//Get it for read access
		err = get_user_pages_fast(address, 1, 0, &pg);
		if (err < 0){
			FPRINTK(KERN_ALERT "%s: error getting page  {%d}\n",__func__,err);
			return err;
		}
		else
			err = 0;

		if(pg)
			return  pg->futex_owner;

		return -1;
	}

	/**
	 * get_futex_key() - Get parameters which are the keys for a futex
	 * @uaddr:	virtual address of the futex
	 * @fshared:	0 for a PROCESS_PRIVATE futex, 1 for PROCESS_SHARED
	 * @key:	address where result is stored.
	 * @rw:		mapping needs to be read/write (values: VERIFY_READ,
	 *              VERIFY_WRITE)
	 *
	 * Returns a negative error code or 0
	 *
	 * The key words are stored in *key on success.
	 *
	 * For shared mappings, it's (page->index, vma->vm_file->f_path.dentry->d_inode,
	 * offset_within_page).  For private mappings, it's (uaddr, current->mm).
	 * We can usually work out the index without swapping in the page.
	 *
	 * lock_page() might sleep, the caller should not hold a spinlock.
	 */
	//static
	int
		get_futex_key(u32 __user *uaddr, int fshared, union futex_key *key, int rw)
		{
			unsigned long address = (unsigned long)uaddr;
			struct mm_struct *mm = current->mm;
			struct task_struct *tsk = current;
			int pid=tsk->pid;
			pte_t pte;
			int owner;

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

				int nr_cpus;
				struct cpu_namespace * _ns = current->nsproxy->cpu_ns;
				if (_ns != &init_cpu_ns) {
					nr_cpus = nr_cpu_ids + 228;//((struct cpu_namespace *)_ns)->nr_cpu_ids;
				}
				else
					nr_cpus = nr_cpu_ids;

				if ((nr_cpus > nr_cpu_ids) ) {
					//TODO: make it only for Popcorn threads.
					//if((current->cpus_allowed_map  && (current->cpus_allowed_map->ns == current->nsproxy->cpu_ns)))
					owner = setFutexOwnerToPage(address,uaddr);
					//printk(KERN_ALERT "%s: owner for addr {%d} \n",__func__,owner);
				}
				key->private.mm = mm;
				key->private.address = address;
				get_futex_key_refs(key);
				return 0;
			}

			FPRINTK(KERN_ALERT "its global futex \n");

again:
			err = get_user_pages_fast(address, 1, 1, &page);
			/*
			 * If write access is not required (eg. FUTEX_WAIT), try
			 * and get read-only access.
			 */
			if (err == -EFAULT && rw == VERIFY_READ) {
				err = get_user_pages_fast(address, 1, 0, &page);
				ro = 1;
			}

			if (err < 0)
				return err;
			else
				err = 0;

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
				key->shared.pgoff = basepage_index(page);
			}

			get_futex_key_refs(key);

out:
			unlock_page(page_head);
			put_page(page_head);
			return err;
		}

	//static inline
	void put_futex_key(union futex_key *key)
	{
		drop_futex_key_refs(key);
	}

	/**
	 * fault_in_user_writeable() - Fault in user address and verify RW access
	 * @uaddr:	pointer to faulting user space address
	 *
	 * Slow path to fixup the fault we just took in the atomic write
	 * access to @uaddr.
	 *
	 * We have no generic implementation of a non-destructive write to the
	 * user address. We know that we faulted in the atomic pagefault
	 * disabled section so we can as well avoid the #PF overhead by
	 * calling get_user_pages() right away.
	 */
	//static 

	int fault_in_user_writeable(u32 __user *uaddr)
	{
		struct mm_struct *mm = current->mm;
		int ret;
		int lock_aquired= 0;
		if(current->tgroup_distributed==1){
			down_read(&mm->distribute_sem);
			lock_aquired= 1;
		} else {
			lock_aquired= 0;
		}

		down_read(&mm->mmap_sem);
		ret = fixup_user_fault(current, mm, (unsigned long)uaddr,
				FAULT_FLAG_WRITE);
		up_read(&mm->mmap_sem);

		if(current->tgroup_distributed==1 && lock_aquired){
			up_read(&mm->distribute_sem);
		}

		return ret < 0 ? ret : 0;

	}

	//static
	int fault_in_user_writeable_task(u32 __user *uaddr,struct task_struct * tgid)
	{
		struct mm_struct *mm = tgid->mm;
		int ret;

		int lock_aquired= 0;
		if(current->tgroup_distributed==1){
			down_read(&mm->distribute_sem);
			lock_aquired= 1;
		} else {
			lock_aquired= 0;
		}

		down_read(&mm->mmap_sem);
		ret = fixup_user_fault(tgid, mm, (unsigned long)uaddr,
				FAULT_FLAG_WRITE);
		up_read(&mm->mmap_sem);

		if(current->tgroup_distributed==1 && lock_aquired){
			up_read(&mm->distribute_sem);
		}

		return ret < 0 ? ret : 0;

	}
	/**
	 * futex_top_waiter() - Return the highest priority waiter on a futex
	 * @hb:		the hash bucket the futex_q's reside in
	 * @key:	the futex key (to distinguish it from other futex futex_q's)
	 *
	 * Must be called with the hb lock held.
	 */
	static struct futex_q *futex_top_waiter(struct futex_hash_bucket *hb,
			union futex_key *key)
	{
		struct futex_q *this;

		plist_for_each_entry(this, &hb->chain, list) {
			if (match_futex(&this->key, key))
				return this;
		}
		return NULL;
	}

	static int cmpxchg_futex_value_locked(u32 *curval, u32 __user *uaddr,
			u32 uval, u32 newval)
	{
		int ret;

		pagefault_disable();
		ret = futex_atomic_cmpxchg_inatomic(curval, uaddr, uval, newval);
		pagefault_enable();

		return ret;
	}

	//static
	int get_futex_value_locked(u32 *dest, u32 __user *from)
	{
		int ret;

		pagefault_disable();
		ret = __copy_from_user_inatomic(dest, from, sizeof(u32));
		pagefault_enable();

		return ret ? -EFAULT : 0;
	}


	/*
	 * PI code:
	 */
	static int refill_pi_state_cache(void)
	{
		struct futex_pi_state *pi_state;

		if (likely(current->pi_state_cache))
			return 0;

		pi_state = kzalloc(sizeof(*pi_state), GFP_KERNEL);

		if (!pi_state)
			return -ENOMEM;

		INIT_LIST_HEAD(&pi_state->list);
		/* pi_mutex gets initialized later */
		pi_state->owner = NULL;
		atomic_set(&pi_state->refcount, 1);
		pi_state->key = FUTEX_KEY_INIT;

		current->pi_state_cache = pi_state;

		return 0;
	}

	static struct futex_pi_state * alloc_pi_state(void)
	{
		struct futex_pi_state *pi_state = current->pi_state_cache;

		WARN_ON(!pi_state);
		current->pi_state_cache = NULL;

		return pi_state;
	}

	static void free_pi_state(struct futex_pi_state *pi_state)
	{
		if (!atomic_dec_and_test(&pi_state->refcount))
			return;

		/*
		 * If pi_state->owner is NULL, the owner is most probably dying
		 * and has cleaned up the pi_state already
		 */
		if (pi_state->owner) {
			raw_spin_lock_irq(&pi_state->owner->pi_lock);
			list_del_init(&pi_state->list);
			raw_spin_unlock_irq(&pi_state->owner->pi_lock);

			rt_mutex_proxy_unlock(&pi_state->pi_mutex, pi_state->owner);
		}

		if (current->pi_state_cache)
			kfree(pi_state);
		else {
			/*
			 * pi_state->list is already empty.
			 * clear pi_state->owner.
			 * refcount is at 0 - put it back to 1.
			 */
			pi_state->owner = NULL;
			atomic_set(&pi_state->refcount, 1);
			current->pi_state_cache = pi_state;
		}
	}

	/*
	 * Look up the task based on what TID userspace gave us.
	 * We dont trust it.
	 */
	static struct task_struct * futex_find_get_task(pid_t pid)
	{
		struct task_struct *p;

		rcu_read_lock();
		p = find_task_by_vpid(pid);
		if (p)
			get_task_struct(p);

		rcu_read_unlock();

		return p;
	}

	/*
	 * This task is holding PI mutexes at exit time => bad.
	 * Kernel cleans up PI-state, but userspace is likely hosed.
	 * (Robust-futex cleanup is separate and might save the day for userspace.)
	 */
	void exit_pi_state_list(struct task_struct *curr)
	{
		struct list_head *next, *head = &curr->pi_state_list;
		struct futex_pi_state *pi_state;
		struct futex_hash_bucket *hb;
		union futex_key key = FUTEX_KEY_INIT;

		if (!futex_cmpxchg_enabled)
			return;
		/*
		 * We are a ZOMBIE and nobody can enqueue itself on
		 * pi_state_list anymore, but we have to be careful
		 * versus waiters unqueueing themselves:
		 */
		raw_spin_lock_irq(&curr->pi_lock);
		while (!list_empty(head)) {

			next = head->next;
			pi_state = list_entry(next, struct futex_pi_state, list);
			key = pi_state->key;
			hb = hash_futex(&key);
			raw_spin_unlock_irq(&curr->pi_lock);

			spin_lock(&hb->lock);

			raw_spin_lock_irq(&curr->pi_lock);
			/*
			 * We dropped the pi-lock, so re-check whether this
			 * task still owns the PI-state:
			 */
			if (head->next != next) {
				spin_unlock(&hb->lock);
				continue;
			}

			WARN_ON(pi_state->owner != curr);
			WARN_ON(list_empty(&pi_state->list));
			list_del_init(&pi_state->list);
			pi_state->owner = NULL;
			raw_spin_unlock_irq(&curr->pi_lock);

			rt_mutex_unlock(&pi_state->pi_mutex);

			spin_unlock(&hb->lock);

			raw_spin_lock_irq(&curr->pi_lock);
		}
		raw_spin_unlock_irq(&curr->pi_lock);
	}

	static int
		lookup_pi_state(u32 uval, struct futex_hash_bucket *hb,
				union futex_key *key, struct futex_pi_state **ps)
		{
			struct futex_pi_state *pi_state = NULL;
			struct futex_q *this, *next;
			struct plist_head *head;
			struct task_struct *p;
			pid_t pid = uval & FUTEX_TID_MASK;

			head = &hb->chain;

			plist_for_each_entry_safe(this, next, head, list) {
				if (match_futex(&this->key, key)) {
					/*
					 * Another waiter already exists - bump up
					 * the refcount and return its pi_state:
					 */
					pi_state = this->pi_state;
					/*
					 * Userspace might have messed up non-PI and PI futexes
					 */
					if (unlikely(!pi_state))
						return -EINVAL;

					WARN_ON(!atomic_read(&pi_state->refcount));

					/*
					 * When pi_state->owner is NULL then the owner died
					 * and another waiter is on the fly. pi_state->owner
					 * is fixed up by the task which acquires
					 * pi_state->rt_mutex.
					 *
					 * We do not check for pid == 0 which can happen when
					 * the owner died and robust_list_exit() cleared the
					 * TID.
					 */
					if (pid && pi_state->owner) {
						/*
						 * Bail out if user space manipulated the
						 * futex value.
						 */
						if (pid != task_pid_vnr(pi_state->owner))
							return -EINVAL;
					}

					atomic_inc(&pi_state->refcount);
					*ps = pi_state;

					return 0;
				}
			}

			/*
			 * We are the first waiter - try to look up the real owner and attach
			 * the new pi_state to it, but bail out when TID = 0
			 */
			if (!pid)
				return -ESRCH;
			p = futex_find_get_task(pid);
			if (!p)
				return -ESRCH;

			/*
			 * We need to look at the task state flags to figure out,
			 * whether the task is exiting. To protect against the do_exit
			 * change of the task flags, we do this protected by
			 * p->pi_lock:
			 */
			raw_spin_lock_irq(&p->pi_lock);
			if (unlikely(p->flags & PF_EXITING)) {
				/*
				 * The task is on the way out. When PF_EXITPIDONE is
				 * set, we know that the task has finished the
				 * cleanup:
				 */
				int ret = (p->flags & PF_EXITPIDONE) ? -ESRCH : -EAGAIN;

				raw_spin_unlock_irq(&p->pi_lock);
				put_task_struct(p);
				return ret;
			}

			pi_state = alloc_pi_state();

			/*
			 * Initialize the pi_mutex in locked state and make 'p'
			 * the owner of it:
			 */
			rt_mutex_init_proxy_locked(&pi_state->pi_mutex, p);

			/* Store the key for possible exit cleanups: */
			pi_state->key = *key;

			WARN_ON(!list_empty(&pi_state->list));
			list_add(&pi_state->list, &p->pi_state_list);
			pi_state->owner = p;
			raw_spin_unlock_irq(&p->pi_lock);

			put_task_struct(p);

			*ps = pi_state;

			return 0;
		}

	/**
	 * futex_lock_pi_atomic() - Atomic work required to acquire a pi aware futex
	 * @uaddr:		the pi futex user address
	 * @hb:			the pi futex hash bucket
	 * @key:		the futex key associated with uaddr and hb
	 * @ps:			the pi_state pointer where we store the result of the
	 *			lookup
	 * @task:		the task to perform the atomic lock work for.  This will
	 *			be "current" except in the case of requeue pi.
	 * @set_waiters:	force setting the FUTEX_WAITERS bit (1) or not (0)
	 *
	 * Return:
	 *  0 - ready to wait;
	 *  1 - acquired the lock;
	 * <0 - error
	 *
	 * The hb->lock and futex_key refs shall be held by the caller.
	 */
	static int futex_lock_pi_atomic(u32 __user *uaddr, struct futex_hash_bucket *hb,
			union futex_key *key,
			struct futex_pi_state **ps,
			struct task_struct *task, int set_waiters)
	{
		int lock_taken, ret, force_take = 0;
		u32 uval, newval, curval, vpid = task_pid_vnr(task);

retry:
		ret = lock_taken = 0;

		/*
		 * To avoid races, we attempt to take the lock here again
		 * (by doing a 0 -> TID atomic cmpxchg), while holding all
		 * the locks. It will most likely not succeed.
		 */
		newval = vpid;
		if (set_waiters)
			newval |= FUTEX_WAITERS;

		if (unlikely(cmpxchg_futex_value_locked(&curval, uaddr, 0, newval)))
			return -EFAULT;

		/*
		 * Detect deadlocks.
		 */
		if ((unlikely((curval & FUTEX_TID_MASK) == vpid)))
			return -EDEADLK;

		/*
		 * Surprise - we got the lock. Just return to userspace:
		 */
		if (unlikely(!curval))
			return 1;

		uval = curval;

		/*
		 * Set the FUTEX_WAITERS flag, so the owner will know it has someone
		 * to wake at the next unlock.
		 */
		newval = curval | FUTEX_WAITERS;

		/*
		 * Should we force take the futex? See below.
		 */
		if (unlikely(force_take)) {
			/*
			 * Keep the OWNER_DIED and the WAITERS bit and set the
			 * new TID value.
			 */
			newval = (curval & ~FUTEX_TID_MASK) | vpid;
			force_take = 0;
			lock_taken = 1;
		}

		if (unlikely(cmpxchg_futex_value_locked(&curval, uaddr, uval, newval)))
			return -EFAULT;
		if (unlikely(curval != uval))
			goto retry;

		/*
		 * We took the lock due to owner died take over.
		 */
		if (unlikely(lock_taken))
			return 1;

		/*
		 * We dont have the lock. Look up the PI state (or create it if
		 * we are the first waiter):
		 */
		ret = lookup_pi_state(uval, hb, key, ps);

		if (unlikely(ret)) {
			switch (ret) {
				case -ESRCH:
					/*
					 * We failed to find an owner for this
					 * futex. So we have no pi_state to block
					 * on. This can happen in two cases:
					 *
					 * 1) The owner died
					 * 2) A stale FUTEX_WAITERS bit
					 *
					 * Re-read the futex value.
					 */
					if (get_futex_value_locked(&curval, uaddr))
						return -EFAULT;

					/*
					 * If the owner died or we have a stale
					 * WAITERS bit the owner TID in the user space
					 * futex is 0.
					 */
					if (!(curval & FUTEX_TID_MASK)) {
						force_take = 1;
						goto retry;
					}
				default:
					break;
			}
		}

		return ret;
	}

	/**
	 * __unqueue_futex() - Remove the futex_q from its futex_hash_bucket
	 * @q:	The futex_q to unqueue
	 *
	 * The q->lock_ptr must not be NULL and must be held by the caller.
	 */
	void __unqueue_futex(struct futex_q *q)
	{
		struct futex_hash_bucket *hb;

		if (WARN_ON_SMP(!q->lock_ptr))
		{
			FPRINTK(KERN_ALERT " lockptr null warning on smp \n ");
			return;
		}
		if(!spin_is_locked(q->lock_ptr))
		{
			FPRINTK(KERN_ALERT " lockptr held warning on smp \n ");
			return;
		}
		if(WARN_ON(plist_node_empty(&q->list)))
		{
			FPRINTK(KERN_ALERT " lockptr held warning on smp \n ");
			return;
		}

		hb = container_of(q->lock_ptr, struct futex_hash_bucket, lock);
		if(&hb->chain!=NULL)
		{	plist_del(&q->list, &hb->chain);}
		else
			printk(KERN_ALERT"hb-chain {%p}\n",&hb->chain);
	}

	/*
	 * The hash bucket lock must be held when this is called.
	 * Afterwards, the futex_q must not be accessed.
	 */
	//static
	void wake_futex(struct futex_q *q)
	{
		struct task_struct *p = q->task;

		if (WARN(q->pi_state || q->rt_waiter, "refusing to wake PI futex\n"))
			return;

		/*
		 * We set q->lock_ptr = NULL _before_ we wake up the task. If
		 * a non-futex wake up happens on another CPU then the task
		 * might exit and p would dereference a non-existing task
		 * struct. Prevent this by holding a reference on p across the
		 * wake up.
		 */
		get_task_struct(p);

		__unqueue_futex(q);
		/*
		 * The waiting task can free the futex_q as soon as
		 * q->lock_ptr = NULL is written, without taking any locks. A
		 * memory barrier is required here to prevent the following
		 * store to lock_ptr from getting ahead of the plist_del.
		 */
		smp_wmb();
		q->lock_ptr = NULL;

		wake_up_state(p, TASK_NORMAL);
		put_task_struct(p);
	}

	static int wake_futex_pi(u32 __user *uaddr, u32 uval, struct futex_q *this)
	{
		struct task_struct *new_owner;
		struct futex_pi_state *pi_state = this->pi_state;
		u32 uninitialized_var(curval), newval;

		if (!pi_state)
			return -EINVAL;

		/*
		 * If current does not own the pi_state then the futex is
		 * inconsistent and user space fiddled with the futex value.
		 */
		if (pi_state->owner != current)
			return -EINVAL;

		raw_spin_lock(&pi_state->pi_mutex.wait_lock);
		new_owner = rt_mutex_next_owner(&pi_state->pi_mutex);

		/*
		 * It is possible that the next waiter (the one that brought
		 * this owner to the kernel) timed out and is no longer
		 * waiting on the lock.
		 */
		if (!new_owner)
			new_owner = this->task;

		/*
		 * We pass it to the next owner. (The WAITERS bit is always
		 * kept enabled while there is PI state around. We must also
		 * preserve the owner died bit.)
		 */
		if (!(uval & FUTEX_OWNER_DIED)) {
			int ret = 0;

			newval = FUTEX_WAITERS | task_pid_vnr(new_owner);

			if (cmpxchg_futex_value_locked(&curval, uaddr, uval, newval))
				ret = -EFAULT;
			else if (curval != uval)
				ret = -EINVAL;
			if (ret) {
				raw_spin_unlock(&pi_state->pi_mutex.wait_lock);
				return ret;
			}
		}

		raw_spin_lock_irq(&pi_state->owner->pi_lock);
		WARN_ON(list_empty(&pi_state->list));
		list_del_init(&pi_state->list);
		raw_spin_unlock_irq(&pi_state->owner->pi_lock);

		raw_spin_lock_irq(&new_owner->pi_lock);
		WARN_ON(!list_empty(&pi_state->list));
		list_add(&pi_state->list, &new_owner->pi_state_list);
		pi_state->owner = new_owner;
		raw_spin_unlock_irq(&new_owner->pi_lock);

		raw_spin_unlock(&pi_state->pi_mutex.wait_lock);
		rt_mutex_unlock(&pi_state->pi_mutex);

		return 0;
	}

	static int unlock_futex_pi(u32 __user *uaddr, u32 uval)
	{
		u32 uninitialized_var(oldval);

		/*
		 * There is no waiter, so we unlock the futex. The owner died
		 * bit has not to be preserved here. We are the owner:
		 */
		if (cmpxchg_futex_value_locked(&oldval, uaddr, uval, 0))
			return -EFAULT;
		if (oldval != uval)
			return -EAGAIN;

		return 0;
	}

	/*
	 * Express the locking dependencies for lockdep:
	 */
	static inline void
		double_lock_hb(struct futex_hash_bucket *hb1, struct futex_hash_bucket *hb2)
		{
			if (hb1 <= hb2) {
				spin_lock(&hb1->lock);
				if (hb1 < hb2)
					spin_lock_nested(&hb2->lock, SINGLE_DEPTH_NESTING);
			} else { /* hb1 > hb2 */
				spin_lock(&hb2->lock);
				spin_lock_nested(&hb1->lock, SINGLE_DEPTH_NESTING);
			}
		}

	static inline void
		double_unlock_hb(struct futex_hash_bucket *hb1, struct futex_hash_bucket *hb2)
		{
			spin_unlock(&hb1->lock);
			if (hb1 != hb2)
				spin_unlock(&hb2->lock);
		}


	inline void __spin_key_init (struct spin_key *st) {
		st->_tgid = 0;
		st->_uaddr = 0;
		st->offset = 0;
	}
	/*
	 * Try acquiring spin lock only if ret == 0
	 */
	static inline int global_queue_wait_lock(struct futex_q *q,u32 __user * uaddr, struct futex_hash_bucket *hb,unsigned int fn_flag,
			unsigned int val, int fshared, int rw,  u32 bitset)
		__acquires(&value->_sp)
		{

			struct spin_key sk;
			__spin_key_init(&sk);
			//Get the local mapped value for the key (TGID|Uaddr)
			getKey(uaddr, &sk,current->tgroup_home_id);
			_spin_value *value = hashspinkey(&sk);
#if defined(CONFIG_X86)
			int localticket_value = xadd_sync(&value->_ticket, 1);
#else
			int localticket_value = futex_atomic_add(&value->_ticket, 1);
#endif
			int ret;
			u32 dval;
			int x=0,y=0,cpu=0;


			cpu = getFutexOwnerFromPage((unsigned long)uaddr);
			if(cpu < 0)
				return cpu; //return ERROR


			//Get the request id
			spin_lock(&value->_sp);

			_local_rq_t *rq_ptr= add_request_node(localticket_value,current->pid,&value->_lrq_head);
			rq_ptr->_pid = current->pid;
			rq_ptr->status = INPROG;
			rq_ptr->_st = 0;
			rq_ptr->wake_st = 0;
			rq_ptr->ops = 0;
			rq_ptr->uaddr = uaddr;

			//populate the hb
			hb = hash_futex(&q->key);
			q->lock_ptr = &hb->lock;

			futex_common_data_t data_;
			data_.fn_flag = fn_flag;
			data_.val= val;
			data_.flags =fshared;
			data_.rw =rw;
			data_.bitset =bitset;
			data_.ops=0;

			//replacing the spin lock call with global spin lock
			ret= global_spinlock((unsigned long)uaddr,&data_,value,rq_ptr,localticket_value,cpu);


			ret = rq_ptr->errno;
			smp_mb();

			if(ret){
				if(rq_ptr->wake_st == 1) //no need to queue it.
					ret = 0;
			}
			else if (!ret){
				FPRINTK(KERN_ALERT"%s: check if there is wake up on ret=0 {%d} davl{%d}  \n",__func__,rq_ptr->wake_st,ret,dval);
				if(rq_ptr->wake_st == 1)//no neew to queue
					ret = -EWOULDBLOCK;
			}
			//	find_and_delete_request(localticket_value, &value->_lrq_head);

			return ret;
		}

	int
		get_futex_key_tsk(u32 __user *uaddr, int fshared, union futex_key *key, int rw, struct task_struct * _tsk)
		{
			unsigned long address = (unsigned long)uaddr;
			struct mm_struct *mm = _tsk->mm;
			struct task_struct *tsk = _tsk;
			int pid=tsk->pid;
			struct page *page, *page_head;
			int err, ro = 0;
			/*
			 * 	 * The futex address must be "naturally" aligned.
			 * 	 	 */
			key->both.offset = address % PAGE_SIZE;
			if (unlikely((address % sizeof(u32)) != 0))
				return -EINVAL;
			address -= key->both.offset;
			/*
			 * 	 * PROCESS_PRIVATE futexes are fast.
			 * 	 * As the mm cannot disappear under us and the 'key' only needs
			 * 	 * virtual address, we dont even have to find the underlying vma.
			 * 	 * Note : We do have to check 'uaddr' is a valid user address,
			 * 	 *        but access_ok() should be faster than find_vma()
			 * 	 	 	 	 	 	 */
			if (!fshared) {
				if (unlikely(!access_ok(VERIFY_WRITE, uaddr, sizeof(u32))))
					return -EFAULT;
				key->private.mm = mm;
				key->private.address = address;
				get_futex_key_refs(key);
				return 0;
			}
		}

	static inline int global_queue_wake_lock(union futex_key *key,u32 __user * uaddr, unsigned int flags, int nr_wake,
			u32 bitset, int rflag, unsigned int fn_flags, unsigned long uaddr2, int nr_requeue, int cmpval)
		__acquires(&value->_sp)
		{
			int ret;
			//Get the local mapped value for the key (TGID|Uaddr)
			struct spin_key sk;
			__spin_key_init(&sk);

			getKey(uaddr, &sk,current->tgroup_home_id);
			_spin_value *value = hashspinkey(&sk);

#if defined(CONFIG_X86)
			int localticket_value = xadd_sync(&value->_ticket, 1);
#else
			int localticket_value = futex_atomic_add(&value->_ticket, 1);
#endif

			int cpu = getFutexOwnerFromPage((unsigned long)uaddr);

			//Get the request id
			spin_lock(&value->_sp);

			_local_rq_t *rq_ptr= add_request_node(localticket_value,current->pid,&value->_lrq_head);

			rq_ptr->_pid = current->pid;
			rq_ptr->status = INPROG;
			rq_ptr->ops  = 1;
			rq_ptr->uaddr = uaddr;

			futex_common_data_t data_;

			data_.flags =flags;
			data_.fn_flag = fn_flags;
			data_.rflag =rflag;
			data_.nr_wake= nr_wake;
			data_.nr_requeue =nr_requeue;
			data_.uaddr2 =(unsigned long) uaddr2;
			data_.bitset =bitset;
			data_.cmpval =cmpval;
			data_.ops=1;

			//replacing the spin lock call with global spin lock
			ret = global_spinlock((unsigned long)uaddr,&data_,value,rq_ptr,localticket_value,cpu);
			//get the actual spinlock : Not necessary as we are alone

			ret = rq_ptr->errno;

			find_and_delete_request(localticket_value, &value->_lrq_head);

			return ret;
		}

	/*
	 * Wake up waiters matching bitset queued on this futex (uaddr).
	 */
	//static
	int
		futex_wake(u32 __user *uaddr, unsigned int flags, int nr_wake, u32 bitset,unsigned int fn_flags,struct task_struct *_tsk)
		{
#ifdef FUTEX_STAT
			unsigned long long wake_aa=0,wake_bb=0;
			if(!_tsk && current->tgroup_distributed){
				_wake_cnt++;
				wake_aa = native_read_tsc();
			}
#endif

			struct futex_hash_bucket *hb;
			struct futex_q *this, *next;
			struct plist_head *head;
			union futex_key key = FUTEX_KEY_INIT;
			int ret,retf=0;
			int x=0,y=0;
			int g_errno;
			/* unsigned long bp = stack_frame(current,NULL); */
			_spin_value *value =NULL;
			_local_rq_t *l =NULL;
			struct spin_key sk;
			__spin_key_init(&sk);

			/*if(_tsk || strcmp(current->comm,"matrix_mult") == 0){
			  printk(KERN_ALERT " FUTEX_WAKE:current{%d} uaddr {%lx} get_user{%d} comm{%s}  lockval{%d} fn_flags{%d} cpu{%d} \n",current->pid,uaddr,x,current->comm,y,fn_flags,smp_processor_id());
			  }*/
			fn_flags |= FLAGS_WAKECALL;

			//printPTE(uaddr);
			if (!bitset){
				/*	if(_tsk && strcmp(_tsk->comm,"matrix_mult") == 0){
					printk(KERN_ALERT"bitsetnukk\n");}
				 */
				return -EINVAL;
			}

			ret = (_tsk == NULL) ? get_futex_key(uaddr, flags & FLAGS_SHARED, &key, VERIFY_READ) :
				get_futex_key_tsk(uaddr, flags & FLAGS_SHARED, &key, VERIFY_READ, _tsk);

			if (unlikely(ret != 0)){
				goto out;
			}


cont:
			hb = hash_futex(&key);

			if( !_tsk && !(flags & FLAGS_SHARED) && current->tgroup_distributed  && !(fn_flags & FLAGS_REMOTECALL) ){
				//printk("client?\n");
				g_errno= global_queue_wake_lock(&key,uaddr, flags & FLAGS_SHARED, nr_wake, bitset,
						0, fn_flags, 0,0,0);
				FPRINTK(KERN_ALERT " %s: err {%d}\n",__func__,g_errno);
				ret = g_errno;
				goto out;
			}
			else{
				//if(_tsk!=NULL && _tsk->tgroup_distributed)
				//	printk("server?\n");		
				spin_lock(&hb->lock);
				head = &hb->chain;
				ret = 0;
				//if(_tsk && head)  printk(KERN_ALERT"%s: head {%p}\n",__func__,head);
				/*if(_tsk && _tsk->tgroup_distributed){
				  printk(KERN_ALERT "%s:  hb {%p} key: word {%lx} head {%p} offset{%d} ptr{%p} mm{%p} ret{%d}\n ",__func__,
				  hb,key.both.word,head,key.both.offset,key.both.ptr,key.private.mm,ret);
				  }*/
				if((fn_flags & FLAGS_REMOTECALL))
					fn_flags  &= ~(1 << 5); //FLAGS_ORIGINCALL

				plist_for_each_entry_safe(this, next, head, list) {

					/*if(_tsk && strcmp(_tsk->comm,"matrix_mult") == 0){
					  printk(KERN_ALERT "%s:  this->pid{%d}\n ",__func__,(!(this->task)) ? -1 : this->task->pid);
					  }*/
					if(_tsk){
						getKey(uaddr, &sk,_tsk->tgroup_home_id);
						value = hashspinkey(&sk);
					}
					if (match_futex (&this->key, &key)) {
						/*if(_tsk){	
						  printk(KERN_ALERT " %s:sending it to remote after decision; ret{%d} nr_wake{%d} has_req_addr{%lx} cpu{%d} pid{%d}\n",__func__,ret,nr_wake,(this->req_addr != 0) ? this->req_addr : 0,Kernel_Id,!(this->task) ? -1 : this->task->pid);
						  }*/
						if (this->rem_pid == -1 && (this->pi_state || this->rt_waiter)) {
							ret = -EINVAL;
							//	if(_tsk)
							//		printk("nooo impossible!\n");
							break;
						}

						/* Check if one of the bits is set in both bitsets */
						if (this->rem_pid == -1 && !(this->bitset & bitset)){
							//	printk("really?!\n");
							continue;
						}
						if(this->rem_pid == -1){
							if(_tsk){  
								l= find_request_by_ops(0, uaddr, this->task->pid, &value->_lrq_head);
								//printk(KERN_ALERT"%s: l ptr{%p} _st{%d} ",__func__,l,l->wake_st);
								if(l) l->wake_st = 1;
							}

							wake_futex(this);

							/*if(_tsk){
							  printk("BELLAAAAAAAAAAAAAAAAAAAAAAAA\n");
							  }*/
						}
						else
						{
							FPRINTK(KERN_ALERT " %s:sending it to remote after decision; ret{%d} nr_wake{%d} has_req_addr{%lx} cpu{%d}\n",__func__,ret,nr_wake,(this->req_addr != 0) ? this->req_addr : 0,Kernel_Id);
							retf = remote_futex_wakeup(uaddr, flags & FLAGS_SHARED,nr_wake, bitset,&key,this->rem_pid, fn_flags, (this->req_addr != 0) ? this->req_addr : 0,0,0);
							this->rem_pid = NULL;
							this->req_addr = 0;
							__unqueue_futex(this);
							smp_wmb();
							this->lock_ptr = NULL;
						}

						if (++ret >= nr_wake)
							break;
					}
				}


				spin_unlock(&hb->lock);
				put_futex_key(&key);
			}

out:
#ifdef FUTEX_STAT
			if(!_tsk && current->tgroup_distributed){
				wake_bb = native_read_tsc();
				_wake += wake_bb - wake_aa;
			}
#endif
			/*	if(strcmp(current->comm,"matrix_mult") == 0){
				printk(KERN_ALERT "%s: exit {%d}\n",__func__,current->pid);
				}*/
			return ret;
		}


	struct vm_area_struct * getVMAfromUaddr_t(unsigned long uaddr,struct task_struct *t) {

		unsigned long address = (unsigned long) uaddr;
		unsigned long offset = address % PAGE_SIZE;
		if (unlikely((address % sizeof(u32)) != 0))
			return NULL;
		address -= offset;
		struct vm_area_struct *vma;
		struct vm_area_struct* curr = NULL;
		curr = t->mm->mmap;
		vma = find_extend_vma(t->mm, address);
		if (!vma)
			return NULL;
		else
			return vma;
	}

	/*
	static void dumpPTE(pte_t *ptep) {

		int nx;
		int rw;
		int user;
		int pwt;
		int pcd;
		int accessed;
		int dirty;
		unsigned long pfn;

		pte_t pte;
		pte = *ptep;

		printk(KERN_ALERT"cpu {%d} pte ptr: 0x{%lx}\n", smp_processor_id(), pte);
		pfn = pte_pfn(pte);
		printk(KERN_ALERT" cpu{%d} pte pfn : 0x{%lx}\n", smp_processor_id(), pfn);

		nx       = pte_flags(*ptep) & _PAGE_NX       ? 1 : 0;
		rw       = pte_flags(*ptep) & _PAGE_RW       ? 1 : 0;
		user     = pte_flags(*ptep) & _PAGE_USER     ? 1 : 0;
		pwt      = pte_flags(*ptep) & _PAGE_PWT      ? 1 : 0;
		pcd      = pte_flags(*ptep) & _PAGE_PCD      ? 1 : 0;
		accessed = pte_flags(*ptep) & _PAGE_ACCESSED ? 1 : 0;
		dirty    = pte_flags(*ptep) & _PAGE_DIRTY    ? 1 : 0;

		printk("\tnx{%d}, rw{%d} user{%d} pwt{%d} pcd{%d} accessed{%d} dirty{%d} present{%d} global{%d} special{%d} ",nx,rw,user,pwt,pcd,accessed,dirty,pte_present(pte),pte_mkglobal(pte),pte_mkspecial(pte));


exit:
		printk("exit\n");
	}
	*/

	/*
	void dump_pgtable(unsigned long address)
	{
		pgd_t *base = __va(read_cr3() & PHYSICAL_PAGE_MASK);
		pgd_t *pgd = base + pgd_index(address);
		pud_t *pud;
		pmd_t *pmd;
		pte_t *pte;
		if (!pgd || !pgd_present(*pgd))
			goto bad;
		printk(KERN_ALERT"PGD %lx flags{%d} ", pgd_val(*pgd),pgd_flags(*pgd));
		if (!pgd_present(*pgd))
			goto out;
		pud = pud_offset(pgd, address);
		if (!pud || !pud_present(*pud))
			goto bad;
		printk(KERN_ALERT"PUD %lx flags{%lx} ", pud_val(*pud),pud_flags(*pud));
		if (!pud || !pud_present(*pud) || pud_large(*pud))
			goto out;
		pmd = pmd_offset(pud, address);
		if (!pmd || !pmd_present(*pmd))
			goto bad;
		printk(KERN_ALERT"PMD %lx mkold{%d} dirty{%d} mkwrite{%d} ", pmd_val(*pmd),pmd_mkold(*pmd),pmd_mkdirty(*pmd),pmd_mkwrite(*pmd));
		if (!pmd_present(*pmd) || pmd_large(*pmd))
			goto out;
		pte = pte_offset_kernel(pmd, address);
		if (!(pte) || !pte_present(*pte))
			goto bad;
		printk(KERN_ALERT"PTE %lx", pte_val(*pte));
out:
		printk(KERN_ALERT"\n");
		return;
bad:
		printk(KERN_ALERT"BAD\n");
	}
	*/

	/*
	pte_t *do_page_wlk(unsigned long address,struct task_struct *t) {
		pgd_t *pgd = NULL;
		pud_t *pud = NULL;
		pmd_t *pmd = NULL;
		pte_t *ptep = NULL;
		pte_t *pte;
		struct mm_struct *_m = t->mm;
		//printk(KERN_ALERT"mm{%p} cm{%p} am{%p} \n",_m,(!current->mm) ? 0 : current->mm, (!current->active_mm) ? 0 :current->active_mm);
		//down_read(&_m->mmap_sem);

		pgd = pgd_offset(_m, address);
		if (!pgd_present(*pgd)) {
			//	up_read(&_m->mmap_sem);
			goto exit;
		}
		printk(KERN_ALERT"PGD %lx flags{%d} ", pgd_val(*pgd),pgd_flags(*pgd));

		pud = pud_offset(pgd, address);
		if (!pud_present(*pud)) {
			//	up_read(&_m->mmap_sem);
			goto exit;
		}
		printk(KERN_ALERT"PUD %lx flags{%lx} ", pud_val(*pud),pud_flags(*pud));

		pmd = pmd_offset(pud, address);
		if (!pmd_present(*pmd)) {
			//	up_read(&_m->mmap_sem);
			goto exit;
		}
		printk(KERN_ALERT"PMD %lx mkold{%d} dirty{%d} mkwrite{%d}  pmd_flags{%lx}", pmd_val(*pmd),pmd_mkold(*pmd),pmd_mkdirty(*pmd),pmd_mkwrite(*pmd),pmd_flags(*pmd));
		ptep = pte_offset_map(pmd, address);
		if (!ptep || !pte_present(*ptep)) {
			//	up_read(&_m->mmap_sem);
			goto exit;
		}
		pte = ptep;

		//up_read(&_m->mmap_sem);
		return (pte_t*) pte;
exit: 
		//	up_read(&_m->mmap_sem);
		return NULL;
	}
	*/


	/*
	void find_page(unsigned long uaddr,struct task_struct *t){

		pte_t *pt=do_page_wlk(uaddr,t);
		printk(KERN_ALERT"%s: dump PTE with normal page walk using mm\n",__func__);
		dumpPTE(pt);
		printk(KERN_ALERT"%s: dump PTE with CR3 \n",__func__);
		dump_pgtable(uaddr);

		struct vm_area_struct * _v = getVMAfromUaddr_t(uaddr,t);
		struct page * pg = vm_normal_page(_v, uaddr,*pt);
		if(!pg)
			printk(KERN_ALERT"%s: pg not so good news\n",__func__);
		else{
			dump_page(pg);
			printk(KERN_ALERT"%s: pg present vm{%lx} end{%lx}  flags{%lx} pageprot{%lx} \n",__func__,_v->vm_start, _v->vm_end,_v->vm_flags, pgprot_val(_v->vm_page_prot));
		}
	}
	*/

	/*
	 * Wake up all waiters hashed on the physical page that is mapped
	 * to this virtual address:
	 */
	//static
	int futex_wake_op(u32 __user *uaddr1, unsigned int flags, u32 __user *uaddr2,
			int nr_wake, int nr_wake2, int op,unsigned int fn_flags,struct task_struct * or_task)
	{

#ifdef FUTEX_STAT
		unsigned long long wakeop_aa=0,wakeop_bb=0;
		if(!or_task && current->tgroup_distributed){
			_wakeop_cnt++;
			wakeop_aa = native_read_tsc();
		}
#endif
		union futex_key key1 = FUTEX_KEY_INIT, key2 = FUTEX_KEY_INIT;
		struct futex_hash_bucket *hb1, *hb2;
		struct plist_head *head;
		struct futex_q *this, *next;
		int ret, op_ret;
		struct page *page;
		/* unsigned long bp = stack_frame(current,NULL); */
		int g_errno=0;
		int x=0;
		struct mm_struct *act=NULL,*old=NULL;
		_spin_value *value1 =NULL, *value2 =NULL;
		_local_rq_t *l =NULL;
		struct spin_key sk;
		__spin_key_init(&sk);

		fn_flags |= FLAGS_WAKEOPCALL;
		/*	
			if(strcmp(current->comm,"mut") == 0){
			printk(KERN_ALERT " FUTEX_WAKE_op:current{%d} uaddr {%lx} get_user{%d} comm{%s}  lockval{%d} fn_flags{%d} cpu{%d} \n",current->pid,uaddr1,x,current->comm,fn_flags,smp_processor_id());
			}*/
		FPRINTK(KERN_ALERT " FUTEX_WAKE_OP: entry{%pB} pid {%d} comm{%s} uaddr1{%lx} uaddr2{%lx}  op(%d} \n",(void*) &bp,current->pid,current->comm,uaddr1,uaddr2,op);
retry:
		ret = (or_task == NULL) ? get_futex_key(uaddr1, flags & FLAGS_SHARED, &key1, VERIFY_READ) :
			get_futex_key_tsk(uaddr1, flags & FLAGS_SHARED, &key1, VERIFY_READ, or_task);


		if (unlikely(ret != 0))
			goto out;

		ret = (or_task == NULL) ? get_futex_key(uaddr2, flags & FLAGS_SHARED, &key2, VERIFY_WRITE) :
			get_futex_key_tsk(uaddr2, flags & FLAGS_SHARED, &key2, VERIFY_READ, or_task);


		if (unlikely(ret != 0))
			goto out_put_key1;

		hb1 = hash_futex(&key1);
		hb2 = hash_futex(&key2);

retry_private:

		if(or_task){
			use_mm(or_task->mm);
		}

		if( !or_task && current->tgroup_distributed  && !(fn_flags & FLAGS_REMOTECALL) && !(flags & FLAGS_SHARED)){
			//find_page(uaddr2,current);
			g_errno= global_queue_wake_lock(&key1,uaddr1, flags & FLAGS_SHARED, nr_wake, 1,
					0, fn_flags,(unsigned long)uaddr2,nr_wake2,op);
			ret = g_errno;
			FPRINTK(KERN_ALERT " %s: err {%d}\n",__func__,g_errno);
#ifdef FUTEX_STAT
			_wakeop_err++;
#endif
			goto out;
		}
		else
		{

			//printk(KERN_ALERT"%s:  \n",__func__);
			double_lock_hb(hb1, hb2);
			op_ret = futex_atomic_op_inuser(op, (u32 __user *)uaddr2);
			if (unlikely(op_ret < 0)) {

				double_unlock_hb(hb1, hb2);
#ifndef CONFIG_MMU
				/*
				 * we don't get EFAULT from MMU faults if we don't have an MMU,
				 * but we might get them from range checking
				 */
				ret = op_ret;
				goto out_put_keys;
#endif
				if (unlikely(op_ret != -EFAULT)) {
					ret = op_ret;
					goto out_put_keys;
				}
				if((fn_flags & FLAGS_REMOTECALL) && or_task && op_ret == -EFAULT){
					//  flush_cache_mm(or_task->mm);
				}

				ret = ((fn_flags & FLAGS_REMOTECALL) && or_task)? fault_in_user_writeable_task(uaddr2,or_task):fault_in_user_writeable(uaddr2);

				if(or_task){
					//struct vm_area_struct *_v = getVMAfromUaddr_t(uaddr2,or_task);
					//printk(KERN_ALERT "%s: faultinuaddr2 op{%d} ret{%d} valu{%d} tsk{%d} comm{%s} start{%lx} end{%lx} vmstart{%lx} vmend{%lx} vmflag{%lx}\n",__func__,op,ret,x,or_task->pid,or_task->comm,or_task->mm->mmap->vm_start, or_task->mm->mmap->vm_end, (_v) ? _v->vm_start : 0,(_v) ? _v->vm_end : 0, (_v) ?_v->vm_flags : 0);
					//find_page(uaddr2,or_task);
				}

				if((fn_flags & FLAGS_REMOTECALL) && or_task && op_ret == -EFAULT){
					//flush_tlb_page(or_task->mm->mmap, uaddr2);
					unuse_mm(or_task->mm);
				}
				if (ret)
					goto out_put_keys;

				if (!(flags & FLAGS_SHARED))
					goto retry_private;

				put_futex_key(&key2);
				put_futex_key(&key1);
				goto retry;
			}

			if((fn_flags & FLAGS_REMOTECALL) && or_task){
				unuse_mm(or_task->mm);
			}
			if((fn_flags & FLAGS_REMOTECALL)){
				fn_flags  = 0;
				fn_flags |=FLAGS_WAKEOPCALL;//FLAGS_ORIGINCALL
			}



			head = &hb1->chain;
			if(or_task){
				getKey(uaddr1, &sk,or_task->tgroup_home_id);
				value1 = hashspinkey(&sk);
			}


			plist_for_each_entry_safe(this, next, head, list)
			{

				FPRINTK(KERN_ALERT "%s:key1 pid{%d} comm{%s} rem{%d}\n",__func__,current->pid,current->comm,this->rem_pid);
				if (match_futex (&this->key, &key1)) {
					if (this->pi_state || this->rt_waiter) {
						ret = -EINVAL;
						goto out_unlock;
					}

					if(this->rem_pid == -1){
						if(or_task){
							l= find_request_by_ops(0, uaddr1, this->task->pid, &value1->_lrq_head);
							//printk(KERN_ALERT"%s: l ptr{%p} _st{%d} ",__func__,l,l->wake_st);
							if(l) l->wake_st = 1;
						}

						wake_futex(this);
					}
					else
					{	u32 bitset=1;
						remote_futex_wakeup(uaddr1, flags & FLAGS_SHARED,nr_wake, bitset,&key1,this->rem_pid, fn_flags, 0,0,0);
						this->rem_pid=NULL;
						__unqueue_futex(this);
						smp_wmb();
						this->lock_ptr = NULL;
					}

					if (++ret >= nr_wake)
						break;
				}
			}

			if(or_task){
				getKey(uaddr2, &sk,or_task->tgroup_home_id);
				value2 = hashspinkey(&sk);
			}

			if (op_ret > 0) {
				head = &hb2->chain;

				op_ret = 0;
				plist_for_each_entry_safe(this, next, head, list)
				{

					FPRINTK(KERN_ALERT "%s:key2 pid{%d} comm{%s} rem{%d}\n",__func__,current->pid,current->comm,this->rem_pid);
					if (match_futex (&this->key, &key2)) {
						if (this->pi_state || this->rt_waiter) {
							ret = -EINVAL;
							goto out_unlock;
						}
						if(this->rem_pid == -1){
							if(or_task){
								l= find_request_by_ops(0, uaddr2, this->task->pid, &value2->_lrq_head);
								//printk(KERN_ALERT"%s: l ptr{%p} _st{%d} ",__func__,l,l->wake_st);
								if(l)  l->wake_st = 1;
							}

							wake_futex(this);
						}
						else
						{	u32 bitset=1;
							remote_futex_wakeup(uaddr2, flags & FLAGS_SHARED,nr_wake, bitset,&key2,this->rem_pid, fn_flags, 0,0,0);
							this->rem_pid=NULL;
							__unqueue_futex(this);
							smp_wmb();
							this->lock_ptr = NULL;
						}

						if (++op_ret >= nr_wake2)
							break;
					}
				}
				ret += op_ret;
			}

out_unlock:
			double_unlock_hb(hb1, hb2);

		}
out_put_keys:
		put_futex_key(&key2);
out_put_key1:
		put_futex_key(&key1);
out:

#ifdef FUTEX_STAT
		if(!or_task && current->tgroup_distributed){
			wakeop_bb = native_read_tsc();
			_wakeop += wakeop_bb - wakeop_aa;
		}
#endif
		return ret;
	}

	/**
	 * requeue_futex() - Requeue a futex_q from one hb to another
	 * @q:		the futex_q to requeue
	 * @hb1:	the source hash_bucket
	 * @hb2:	the target hash_bucket
	 * @key2:	the new key for the requeued futex_q
	 */
	static inline
		void requeue_futex(struct futex_q *q, struct futex_hash_bucket *hb1,
				struct futex_hash_bucket *hb2, union futex_key *key2)
		{

			/*
			 * If key1 and key2 hash to the same bucket, no need to
			 * requeue.
			 */
			if (likely(&hb1->chain != &hb2->chain)) {
				plist_del(&q->list, &hb1->chain);
				plist_add(&q->list, &hb2->chain);
				q->lock_ptr = &hb2->lock;
			}
			get_futex_key_refs(key2);
			q->key = *key2;
		}

	static inline
		void rem_requeue_futex(struct futex_q *q, union futex_key *key2, unsigned long uaddr)
		{

			q->req_addr = uaddr;
			q->rem_requeue_key = *key2;
		}



	/**
	 * requeue_pi_wake_futex() - Wake a task that acquired the lock during requeue
	 * @q:		the futex_q
	 * @key:	the key of the requeue target futex
	 * @hb:		the hash_bucket of the requeue target futex
	 *
	 * During futex_requeue, with requeue_pi=1, it is possible to acquire the
	 * target futex if it is uncontended or via a lock steal.  Set the futex_q key
	 * to the requeue target futex so the waiter can detect the wakeup on the right
	 * futex, but remove it from the hb and NULL the rt_waiter so it can detect
	 * atomic lock acquisition.  Set the q->lock_ptr to the requeue target hb->lock
	 * to protect access to the pi_state to fixup the owner later.  Must be called
	 * with both q->lock_ptr and hb->lock held.
	 */
	static inline
		void requeue_pi_wake_futex(struct futex_q *q, union futex_key *key,
				struct futex_hash_bucket *hb)
		{
			get_futex_key_refs(key);
			q->key = *key;

			__unqueue_futex(q);

			WARN_ON(!q->rt_waiter);
			q->rt_waiter = NULL;

			q->lock_ptr = &hb->lock;

			wake_up_state(q->task, TASK_NORMAL);
		}

	/**
	 * futex_proxy_trylock_atomic() - Attempt an atomic lock for the top waiter
	 * @pifutex:		the user address of the to futex
	 * @hb1:		the from futex hash bucket, must be locked by the caller
	 * @hb2:		the to futex hash bucket, must be locked by the caller
	 * @key1:		the from futex key
	 * @key2:		the to futex key
	 * @ps:			address to store the pi_state pointer
	 * @set_waiters:	force setting the FUTEX_WAITERS bit (1) or not (0)
	 *
	 * Try and get the lock on behalf of the top waiter if we can do it atomically.
	 * Wake the top waiter if we succeed.  If the caller specified set_waiters,
	 * then direct futex_lock_pi_atomic() to force setting the FUTEX_WAITERS bit.
	 * hb1 and hb2 must be held by the caller.
	 *
	 * Return:
	 *  0 - failed to acquire the lock atomically;
	 *  1 - acquired the lock;
	 * <0 - error
	 */
	static int futex_proxy_trylock_atomic(u32 __user *pifutex,
			struct futex_hash_bucket *hb1,
			struct futex_hash_bucket *hb2,
			union futex_key *key1, union futex_key *key2,
			struct futex_pi_state **ps, int set_waiters)
	{
		struct futex_q *top_waiter = NULL;
		u32 curval;
		int ret;

		if (get_futex_value_locked(&curval, pifutex))
			return -EFAULT;

		/*
		 * Find the top_waiter and determine if there are additional waiters.
		 * If the caller intends to requeue more than 1 waiter to pifutex,
		 * force futex_lock_pi_atomic() to set the FUTEX_WAITERS bit now,
		 * as we have means to handle the possible fault.  If not, don't set
		 * the bit unecessarily as it will force the subsequent unlock to enter
		 * the kernel.
		 */
		top_waiter = futex_top_waiter(hb1, key1);

		/* There are no waiters, nothing for us to do. */
		if (!top_waiter)
			return 0;

		/* Ensure we requeue to the expected futex. */
		if (!match_futex(top_waiter->requeue_pi_key, key2))
			return -EINVAL;

		/*
		 * Try to take the lock for top_waiter.  Set the FUTEX_WAITERS bit in
		 * the contended case or if set_waiters is 1.  The pi_state is returned
		 * in ps in contended cases.
		 */
		ret = futex_lock_pi_atomic(pifutex, hb2, key2, ps, top_waiter->task,
				set_waiters);
		if (ret == 1)
			requeue_pi_wake_futex(top_waiter, key2, hb2);

		return ret;
	}

	/**
	 * futex_requeue() - Requeue waiters from uaddr1 to uaddr2
	 * @uaddr1:	source futex user address
	 * @flags:	futex flags (FLAGS_SHARED, etc.)
	 * @uaddr2:	target futex user address
	 * @nr_wake:	number of waiters to wake (must be 1 for requeue_pi)
	 * @nr_requeue:	number of waiters to requeue (0-INT_MAX)
	 * @cmpval:	@uaddr1 expected value (or %NULL)
	 * @requeue_pi:	if we are attempting to requeue from a non-pi futex to a
	 *		pi futex (pi to pi requeue is not supported)
	 *
	 * Requeue waiters on uaddr1 to uaddr2. In the requeue_pi case, try to acquire
	 * uaddr2 atomically on behalf of the top waiter.
	 *
	 * Returns:
	 * >=0 - on success, the number of tasks requeued or woken
	 *  <0 - on error
	 */
	//static
	int futex_requeue(u32 __user *uaddr1, unsigned int flags,
			u32 __user *uaddr2, int nr_wake, int nr_requeue,
			u32 *cmpval, int requeue_pi,unsigned int fn_flags, struct task_struct * re_task)
	{

#ifdef FUTEX_STAT
		unsigned long long requeue_aa=0,requeue_bb=0;
		if(!re_task && current->tgroup_distributed){
			_requeue_cnt++;
			requeue_aa = native_read_tsc();
		}
#endif
		union futex_key key1 = FUTEX_KEY_INIT, key2 = FUTEX_KEY_INIT;
		int drop_count = 0, task_count = 0, ret;
		struct futex_pi_state *pi_state = NULL;
		struct futex_hash_bucket *hb1, *hb2;
		struct plist_head *head1;
		struct futex_q *this, *next;
		struct page *pages;
		u32 curval2;
		int requeued=0;
		int g_errno=0;
		/* unsigned long bp = stack_frame(current,NULL); */
		_spin_value *value1 =NULL, *value2 =NULL;
		_local_rq_t *l =NULL;
		struct spin_key sk;
		__spin_key_init(&sk);


		fn_flags |= FLAGS_REQCALL;

		FPRINTK(KERN_ALERT " FUTEX_REQUEUE: cmp{%lx} nr_wake{%d} nr_requeue{%d} pid{%d} comm{%s} uaddr1{%lx} uaddr2{%lx} fn_flags{%lx} \n",*cmpval,nr_wake,nr_requeue,current->pid,current->comm,uaddr1,uaddr2,fn_flags);
		if (requeue_pi) {
			/*
			 * requeue_pi requires a pi_state, try to allocate it now
			 * without any locks in case it fails.
			 */
			if (refill_pi_state_cache())
				return -ENOMEM;
			/*
			 * requeue_pi must wake as many tasks as it can, up to nr_wake
			 * + nr_requeue, since it acquires the rt_mutex prior to
			 * returning to userspace, so as to not leave the rt_mutex with
			 * waiters and no owner.  However, second and third wake-ups
			 * cannot be predicted as they involve race conditions with the
			 * first wake and a fault while looking up the pi_state.  Both
			 * pthread_cond_signal() and pthread_cond_broadcast() should
			 * use nr_wake=1.
			 */
			if (nr_wake != 1)
				return -EINVAL;
		}

retry:
		if (pi_state != NULL) {
			/*
			 * We will have to lookup the pi_state again, so free this one
			 * to keep the accounting correct.
			 */
			free_pi_state(pi_state);
			pi_state = NULL;
		}

		ret = (re_task == NULL) ? get_futex_key(uaddr1, flags & FLAGS_SHARED, &key1, VERIFY_READ) :
			get_futex_key_tsk(uaddr1, flags & FLAGS_SHARED, &key1, VERIFY_READ, re_task);

		if (unlikely(ret != 0))
			goto out;

		ret = (re_task == NULL) ? get_futex_key(uaddr2, flags & FLAGS_SHARED, &key2, requeue_pi ? VERIFY_WRITE : VERIFY_READ) :
			get_futex_key_tsk(uaddr2, flags & FLAGS_SHARED, &key2,  requeue_pi ? VERIFY_WRITE : VERIFY_READ, re_task);

		if (unlikely(ret != 0))
			goto out_put_key1;


cont:
		hb1 = hash_futex(&key1);
		hb2 = hash_futex(&key2);

retry_private:

		FPRINTK(KERN_ALERT " %s: spinlock  futex_requeue uaddr2{%lx} \n",__func__,uaddr2);

		if(re_task){
			use_mm(re_task->mm);
		}

		if( !re_task && current->tgroup_distributed  && !(fn_flags & FLAGS_REMOTECALL) && !(flags & FLAGS_SHARED)){
			g_errno= global_queue_wake_lock(&key1,uaddr1, flags & FLAGS_SHARED, nr_wake, 1,
					0, fn_flags,uaddr2,nr_requeue,(int)*cmpval);
			FPRINTK(KERN_ALERT " %s: err {%d}\n",__func__,g_errno);
			ret = g_errno;
#ifdef FUTEX_STAT
			_requeue_err++;
#endif
			goto out;
		}
		else
		{
			double_lock_hb(hb1, hb2);

			if (likely(cmpval != NULL)) {
				u32 curval;

				ret = get_futex_value_locked(&curval, uaddr1);

				if (unlikely(ret)) {
					double_unlock_hb(hb1, hb2);

					//if(re_task && ret == -EFAULT)
					//	get_user_pages_fast_mm(re_task->mm, key1.private.address, 1, 1, pages);

					ret = get_user(curval, uaddr1);
					if (ret)
						goto out_put_keys;

					if(re_task && ret == -EFAULT)
						unuse_mm(re_task->mm);


					if (!(flags & FLAGS_SHARED))
						goto retry_private;

					put_futex_key(&key2);
					put_futex_key(&key1);
					goto retry;
				}
				if (curval != *cmpval) {
					ret = -EAGAIN;
					goto out_unlock;
				}
			}

			if (requeue_pi && (task_count - nr_wake < nr_requeue)) {
				/*
				 * Attempt to acquire uaddr2 and wake the top waiter. If we
				 * intend to requeue waiters, force setting the FUTEX_WAITERS
				 * bit.  We force this here where we are able to easily handle
				 * faults rather in the requeue loop below.
				 */
				ret = futex_proxy_trylock_atomic(uaddr2, hb1, hb2, &key1,
						&key2, &pi_state, nr_requeue);

				/*
				 * At this point the top_waiter has either taken uaddr2 or is
				 * waiting on it.  If the former, then the pi_state will not
				 * exist yet, look it up one more time to ensure we have a
				 * reference to it.
				 */
				if (ret == 1) {
					WARN_ON(pi_state);
					drop_count++;
					task_count++;
					ret = get_futex_value_locked(&curval2, uaddr2);
					if (!ret)
						ret = lookup_pi_state(curval2, hb2, &key2,
								&pi_state);
				}

				switch (ret) {
					case 0:
						break;
					case -EFAULT:
						double_unlock_hb(hb1, hb2);
						put_futex_key(&key2);
						put_futex_key(&key1);
						ret = fault_in_user_writeable(uaddr2);
						if (!ret)
							goto retry;
						goto out;
					case -EAGAIN:
						/* The owner was exiting, try again. */
						double_unlock_hb(hb1, hb2);
						put_futex_key(&key2);
						put_futex_key(&key1);
						cond_resched();
						goto retry;
					default:
						goto out_unlock;
				}
			}

			if((fn_flags & FLAGS_REMOTECALL) && re_task){
				fn_flags  = 0;
				fn_flags |=FLAGS_REQCALL;//FLAGS_ORIGINCALL
				unuse_mm(re_task->mm);
			}
			if(re_task){
				getKey(uaddr1, &sk,re_task->tgroup_home_id);
				value1 = hashspinkey(&sk);
			}



			head1 = &hb1->chain;
			plist_for_each_entry_safe(this, next, head1, list) {
				if (task_count - nr_wake >= nr_requeue)
					break;

				if (!match_futex(&this->key, &key1))
					continue;

				/*
				 * FUTEX_WAIT_REQEUE_PI and FUTEX_CMP_REQUEUE_PI should always
				 * be paired with each other and no other futex ops.
				 *
				 * We should never be requeueing a futex_q with a pi_state,
				 * which is awaiting a futex_unlock_pi().
				 */
				if ((requeue_pi && !this->rt_waiter) ||
						(!requeue_pi && this->rt_waiter) ||
						this->pi_state) {
					ret = -EINVAL;
					break;
				}

				/*
				 * Wake nr_wake waiters.  For requeue_pi, if we acquired the
				 * lock, we already woke the top_waiter.  If not, it will be
				 * woken by futex_unlock_pi().
				 */
				FPRINTK(KERN_ALERT " %s: nr_wake{%d} task_count{%d} requeued{%d} pid{%d} fn_flags{%lx}\n",__func__,nr_wake,task_count,requeued,this->rem_pid,fn_flags);

				if (++task_count <= nr_wake && !requeue_pi) {

					if(this->rem_pid == -1){
						if(re_task){
							l= find_request_by_ops(0, uaddr1, this->task->pid, &value1->_lrq_head);
							//printk(KERN_ALERT"%s: l ptr{%p} _st{%d} ",__func__,l,l->wake_st);
							if(l) l->wake_st = 1;
						}

						wake_futex(this);
					}
					else
					{	u32 bitset=1;
						if(!requeued)
							remote_futex_wakeup(uaddr1, flags & FLAGS_SHARED,nr_wake, bitset,&key1,this->rem_pid, fn_flags,0,0,0);
						else
							remote_futex_wakeup(uaddr2, flags & FLAGS_SHARED,nr_wake, bitset,&key2,this->rem_pid, fn_flags, 0,0,0);
						this->rem_pid=NULL;
						__unqueue_futex(this);
						smp_wmb();
						this->lock_ptr = NULL;
					}
					continue;
				}

				/* Ensure we requeue to the expected futex for requeue_pi. */
				if (requeue_pi && !match_futex(this->requeue_pi_key, &key2)) {
					ret = -EINVAL;
					break;
				}

				/*
				 * Requeue nr_requeue waiters and possibly one more in the case
				 * of requeue_pi if we couldn't acquire the lock atomically.
				 */
				if (requeue_pi) {
					/* Prepare the waiter to take the rt_mutex. */
					atomic_inc(&pi_state->refcount);
					this->pi_state = pi_state;
					ret = rt_mutex_start_proxy_lock(&pi_state->pi_mutex,
							this->rt_waiter,
							this->task, 1);
					if (ret == 1) {
						/* We got the lock. */
						requeue_pi_wake_futex(this, &key2, hb2);
						drop_count++;
						continue;
					} else if (ret) {
						/* -EDEADLK */
						this->pi_state = NULL;
						free_pi_state(pi_state);
						goto out_unlock;
					}
				}
				FPRINTK(KERN_ALERT"%s: b4 requeue\n");
				requeue_futex(this, hb1, hb2, &key2);
				if(this->rem_pid != -1){
					rem_requeue_futex(this,&key1,(unsigned long) uaddr1);
				}
				requeued=1;
				drop_count++;
			}
		}

out_unlock:
		double_unlock_hb(hb1, hb2);

		/*
		 * drop_futex_key_refs() must be called outside the spinlocks. During
		 * the requeue we moved futex_q's from the hash bucket at key1 to the
		 * one at key2 and updated their key pointer.  We no longer need to
		 * hold the references to key1.
		 */
		while (--drop_count >= 0)
			drop_futex_key_refs(&key1);

out_put_keys:
		put_futex_key(&key2);
out_put_key1:
		put_futex_key(&key1);
out:
		if (pi_state != NULL)
			free_pi_state(pi_state);

#ifdef FUTEX_STAT
		if(!re_task && current->tgroup_distributed){
			requeue_bb = native_read_tsc();
			_requeue += requeue_bb - requeue_aa;
		}
#endif
		return ret ? ret : task_count;
	}

	/* The key must be already stored in q->key. */
	static inline struct futex_hash_bucket *queue_lock(struct futex_q *q)
		__acquires(&hb->lock)
		{
			struct futex_hash_bucket *hb;

			hb = hash_futex(&q->key);
			q->lock_ptr = &hb->lock;

			spin_lock(&hb->lock);
			return hb;
		}

	static inline void
		queue_unlock(struct futex_q *q, struct futex_hash_bucket *hb)
		__releases(&hb->lock)
		{
			spin_unlock(&hb->lock);
		}
	static inline void
		global_queue_unlock(struct futex_q *q, struct futex_hash_bucket *hb)
		__releases(&hb->lock)
		{
			//release the actual spinlock : Not necessary as we are alone
			spin_unlock(&hb->lock);
			//replacing the spin unlock call with global spin unlock
			//global_spinunlock(q->key.private.address+q->key.private.offset,fn_flag);
		}

	/**
	 * queue_me() - Enqueue the futex_q on the futex_hash_bucket
	 * @q:	The futex_q to enqueue
	 * @hb:	The destination hash bucket
	 *
	 * The hb->lock must be held by the caller, and is released here. A call to
	 * queue_me() is typically paired with exactly one call to unqueue_me().  The
	 * exceptions involve the PI related operations, which may use unqueue_me_pi()
	 * or nothing if the unqueue is done as part of the wake process and the unqueue
	 * state is implicit in the state of woken task (see futex_wait_requeue_pi() for
	 * an example).
	 */
	static inline void queue_me(struct futex_q *q, struct futex_hash_bucket *hb)
		__releases(&hb->lock)
		{
			int prio;

			/*
			 * The priority used to register this element is
			 * - either the real thread-priority for the real-time threads
			 * (i.e. threads with a priority lower than MAX_RT_PRIO)
			 * - or MAX_RT_PRIO for non-RT threads.
			 * Thus, all RT-threads are woken first in priority order, and
			 * the others are woken last, in FIFO order.
			 */
			prio = min(current->normal_prio, MAX_RT_PRIO);

			plist_node_init(&q->list, prio);
			plist_add(&q->list, &hb->chain);
			q->task = current;
			smp_mb();
			spin_unlock(&hb->lock);


		}

	/**
	 * unqueue_me() - Remove the futex_q from its futex_hash_bucket
	 * @q:	The futex_q to unqueue
	 *
	 * The q->lock_ptr must not be held by the caller. A call to unqueue_me() must
	 * be paired with exactly one earlier call to queue_me().
	 *
	 * Returns:
	 *   1 - if the futex_q was still queued (and we removed unqueued it)
	 *   0 - if the futex_q was already removed by the waking thread
	 */
	int unqueue_me(struct futex_q *q)
	{
		spinlock_t *lock_ptr;
		int ret = 0;

		/* In the common case we don't take the spinlock, which is nice. */
retry:
		lock_ptr = q->lock_ptr;
		barrier();
		if (lock_ptr != NULL) {
			spin_lock(lock_ptr);
			/*
			 * q->lock_ptr can change between reading it and
			 * spin_lock(), causing us to take the wrong lock.  This
			 * corrects the race condition.
			 *
			 * Reasoning goes like this: if we have the wrong lock,
			 * q->lock_ptr must have changed (maybe several times)
			 * between reading it and the spin_lock().  It can
			 * change again after the spin_lock() but only if it was
			 * already changed before the spin_lock().  It cannot,
			 * however, change back to the original value.  Therefore
			 * we can detect whether we acquired the correct lock.
			 */
			if (unlikely(lock_ptr != q->lock_ptr)) {
				spin_unlock(lock_ptr);
				goto retry;
			}
			__unqueue_futex(q);

			BUG_ON(q->pi_state);

			spin_unlock(lock_ptr);
			ret = 1;
		}

		drop_futex_key_refs(&q->key);
		return ret;
	}

	/*
	 * PI futexes can not be requeued and must remove themself from the
	 * hash bucket. The hash bucket lock (i.e. lock_ptr) is held on entry
	 * and dropped here.
	 */
	static void unqueue_me_pi(struct futex_q *q)
		__releases(q->lock_ptr)
		{
			__unqueue_futex(q);

			BUG_ON(!q->pi_state);
			free_pi_state(q->pi_state);
			q->pi_state = NULL;

			spin_unlock(q->lock_ptr);
		}

	/*
	 * Fixup the pi_state owner with the new owner.
	 *
	 * Must be called with hash bucket lock held and mm->sem held for non
	 * private futexes.
	 */
	static int fixup_pi_state_owner(u32 __user *uaddr, struct futex_q *q,
			struct task_struct *newowner)
	{
		u32 newtid = task_pid_vnr(newowner) | FUTEX_WAITERS;
		struct futex_pi_state *pi_state = q->pi_state;
		struct task_struct *oldowner = pi_state->owner;
		u32 uval, uninitialized_var(curval), newval;
		int ret;

		/* Owner died? */
		if (!pi_state->owner)
			newtid |= FUTEX_OWNER_DIED;

		/*
		 * We are here either because we stole the rtmutex from the
		 * previous highest priority waiter or we are the highest priority
		 * waiter but failed to get the rtmutex the first time.
		 * We have to replace the newowner TID in the user space variable.
		 * This must be atomic as we have to preserve the owner died bit here.
		 *
		 * Note: We write the user space value _before_ changing the pi_state
		 * because we can fault here. Imagine swapped out pages or a fork
		 * that marked all the anonymous memory readonly for cow.
		 *
		 * Modifying pi_state _before_ the user space value would
		 * leave the pi_state in an inconsistent state when we fault
		 * here, because we need to drop the hash bucket lock to
		 * handle the fault. This might be observed in the PID check
		 * in lookup_pi_state.
		 */
retry:
		if (get_futex_value_locked(&uval, uaddr))
			goto handle_fault;

		while (1) {
			newval = (uval & FUTEX_OWNER_DIED) | newtid;

			if (cmpxchg_futex_value_locked(&curval, uaddr, uval, newval))
				goto handle_fault;
			if (curval == uval)
				break;
			uval = curval;
		}

		/*
		 * We fixed up user space. Now we need to fix the pi_state
		 * itself.
		 */
		if (pi_state->owner != NULL) {
			raw_spin_lock_irq(&pi_state->owner->pi_lock);
			WARN_ON(list_empty(&pi_state->list));
			list_del_init(&pi_state->list);
			raw_spin_unlock_irq(&pi_state->owner->pi_lock);
		}

		pi_state->owner = newowner;

		raw_spin_lock_irq(&newowner->pi_lock);
		WARN_ON(!list_empty(&pi_state->list));
		list_add(&pi_state->list, &newowner->pi_state_list);
		raw_spin_unlock_irq(&newowner->pi_lock);
		return 0;

		/*
		 * To handle the page fault we need to drop the hash bucket
		 * lock here. That gives the other task (either the highest priority
		 * waiter itself or the task which stole the rtmutex) the
		 * chance to try the fixup of the pi_state. So once we are
		 * back from handling the fault we need to check the pi_state
		 * after reacquiring the hash bucket lock and before trying to
		 * do another fixup. When the fixup has been done already we
		 * simply return.
		 */
handle_fault:
		spin_unlock(q->lock_ptr);

		ret = fault_in_user_writeable(uaddr);

		spin_lock(q->lock_ptr);

		/*
		 * Check if someone else fixed it for us:
		 */
		if (pi_state->owner != oldowner)
			return 0;

		if (ret)
			return ret;

		goto retry;
	}

	static long futex_wait_restart(struct restart_block *restart);

	/**
	 * fixup_owner() - Post lock pi_state and corner case management
	 * @uaddr:	user address of the futex
	 * @q:		futex_q (contains pi_state and access to the rt_mutex)
	 * @locked:	if the attempt to take the rt_mutex succeeded (1) or not (0)
	 *
	 * After attempting to lock an rt_mutex, this function is called to cleanup
	 * the pi_state owner as well as handle race conditions that may allow us to
	 * acquire the lock. Must be called with the hb lock held.
	 *
	 * Return:
	 *  1 - success, lock taken;
	 *  0 - success, lock not taken;
	 * <0 - on error (-EFAULT)
	 */
	static int fixup_owner(u32 __user *uaddr, struct futex_q *q, int locked)
	{
		struct task_struct *owner;
		int ret = 0;

		if (locked) {
			/*
			 * Got the lock. We might not be the anticipated owner if we
			 * did a lock-steal - fix up the PI-state in that case:
			 */
			if (q->pi_state->owner != current)
				ret = fixup_pi_state_owner(uaddr, q, current);
			goto out;
		}

		/*
		 * Catch the rare case, where the lock was released when we were on the
		 * way back before we locked the hash bucket.
		 */
		if (q->pi_state->owner == current) {
			/*
			 * Try to get the rt_mutex now. This might fail as some other
			 * task acquired the rt_mutex after we removed ourself from the
			 * rt_mutex waiters list.
			 */
			if (rt_mutex_trylock(&q->pi_state->pi_mutex)) {
				locked = 1;
				goto out;
			}

			/*
			 * pi_state is incorrect, some other task did a lock steal and
			 * we returned due to timeout or signal without taking the
			 * rt_mutex. Too late.
			 */
			raw_spin_lock(&q->pi_state->pi_mutex.wait_lock);
			owner = rt_mutex_owner(&q->pi_state->pi_mutex);
			if (!owner)
				owner = rt_mutex_next_owner(&q->pi_state->pi_mutex);
			raw_spin_unlock(&q->pi_state->pi_mutex.wait_lock);
			ret = fixup_pi_state_owner(uaddr, q, owner);
			goto out;
		}

		/*
		 * Paranoia check. If we did not take the lock, then we should not be
		 * the owner of the rt_mutex.
		 */
		if (rt_mutex_owner(&q->pi_state->pi_mutex) == current)
			FPRINTK(KERN_ERR "fixup_owner: ret = %d pi-mutex: %p "
					"pi-state %p\n", ret,
					q->pi_state->pi_mutex.owner,
					q->pi_state->owner);

out:
		return ret ? ret : locked;
	}

	/**
	 * futex_wait_queue_me() - queue_me() and wait for wakeup, timeout, or signal
	 * @hb:		the futex hash bucket, must be locked by the caller
	 * @q:		the futex_q to queue up on
	 * @timeout:	the prepared hrtimer_sleeper, or null for no timeout
	 */
	static int futex_wait_queue_me(struct futex_hash_bucket *hb, struct futex_q *q,
			struct hrtimer_sleeper *timeout,int ops)
	{
		/*
		 * The task state is guaranteed to be set before another task can
		 * wake it. set_current_state() is implemented using set_mb() and
		 * queue_me() calls spin_unlock() upon completion, both serializing
		 * access to the hash list and forcing another memory barrier.
		 */

		/*if(current->tgroup_distributed == 1 && strcmp(current->comm,"cond") == 0){
		  printk(KERN_ALERT"ops{%d}\n",ops);
		  dump_stack();
		  }*/
		struct spin_key sk;
		_spin_value *value = NULL;
		_local_rq_t * l = NULL;
		int counter = 0;
		int ret = 0;
		if(current->tgroup_distributed == 1){
			__spin_key_init(&sk);
			//	printk(KERN_ALERT"%s: uaddr{%lx} pid{%d} tgid{%d}\n",__func__,current->uaddr,current->pid,current->tgroup_home_id);
			getKey((unsigned long) current->uaddr, &sk,current->tgroup_home_id);
			value = hashspinkey(&sk);
			smp_rmb();
			l= find_request_by_ops(0, current->uaddr, current->pid, &value->_lrq_head);
			//printk(KERN_ALERT"%s: l ptr{%p} _st{%d} pid {%d}",__func__,l,l->wake_st,current->pid);
		}



		if(current->tgroup_distributed == 1 && l && l->wake_st == 1){
			//printk(KERN_ALERT" NO need to schedule wait 1\n");
			ret = 1;
			spin_unlock(&hb->lock);
		}
		else{
			set_current_state(TASK_INTERRUPTIBLE);
			//server queued it for me if i am the main
			if(ops != WAIT_MAIN)
				queue_me(q, hb);
			else{
				spin_unlock(&hb->lock);
			}

			/* Arm the timer */
			if (timeout) {
				hrtimer_start_expires(&timeout->timer, HRTIMER_MODE_ABS);
				if (!hrtimer_active(&timeout->timer))
					timeout->task = NULL;
			}

			/*
			 * If we have been removed from the hash list, then another task
			 * has tried to wake us, and we can skip the call to schedule().
			 */
			if (likely(!plist_node_empty(&q->list))) {
				/*
				 * If the timer has already expired, current will already be
				 * flagged for rescheduling. Only call schedule if there
				 * is no timeout, or if it has yet to expire.
				 */
				if (!timeout || timeout->task){
					if(current->tgroup_distributed == 1 && l && l->wake_st == 1){
						//printk(KERN_ALERT" NO need to schedule wait 2\n");
						ret = 1;
					}
					else{
						freezable_schedule();
					}
				}
				/*	if(current->tgroup_distributed == 1 && l){
					while(l->_st != 1){
					if(counter != 0)
					printk(KERN_ALERT"%s: woke up illegal ptr{%p} _st{%d} uaddr{%lx} pid{%d}\n",__func__,l,l->_st,current->uaddr,current->pid);

					counter++;
					set_current_state(TASK_INTERRUPTIBLE);
					if(l->_st !=  1)
					schedule();	
					}
					} */
			}
		}
		if(current->tgroup_distributed == 1 && l)
			find_and_delete_pid(current->pid, &value->_lrq_head);

		__set_current_state(TASK_RUNNING);
		return ret;
	}


	/**
	 * futex_wait_setup() - Prepare to wait on a futex
	 * @uaddr:	the futex userspace address
	 * @val:	the expected value
	 * @flags:	futex flags (FLAGS_SHARED, etc.)
	 * @q:		the associated futex_q
	 * @hb:		storage for hash_bucket pointer to be returned to caller
	 *
	 * Setup the futex_q and locate the hash_bucket.  Get the futex value and
	 * compare it with the expected value.  Handle atomic faults internally.
	 * Return with the hb lock held and a q.key reference on success, and unlocked
	 * with no q.key reference on failure.
	 *
	 * Return:
	 *  0 - uaddr contains val and hb has been locked;
	 * <1 - -EFAULT or -EWOULDBLOCK (uaddr does not contain val) and hb is unlocked
	 */
	static int futex_wait_setup(u32 __user *uaddr, u32 val, unsigned int flags,
			struct futex_q *q, struct futex_hash_bucket **hb, unsigned int fn_flag,u32 bitset)
	{
		u32 uval;
		int ret;
		int g_errno;
		int kind=0;

		/*
		 * Access the page AFTER the hash-bucket is locked.
		 * Order is important:
		 *
		 *   Userspace waiter: val = var; if (cond(val)) futex_wait(&var, val);
		 *   Userspace waker:  if (cond(var)) { var = new; futex_wake(&var); }
		 *
		 * The basic logical guarantee of a futex is that it blocks ONLY
		 * if cond(var) is known to be true at the time of blocking, for
		 * any cond.  If we locked the hash-bucket after testing *uaddr, that
		 * would open a race condition where we could block indefinitely with
		 * cond(var) false, which would violate the guarantee.
		 *
		 * On the other hand, we insert q and release the hash-bucket only
		 * after testing *uaddr.  This guarantees that futex_wait() will NOT
		 * absorb a wakeup if *uaddr does not match the desired values
		 * while the syscall executes.
		 */
retry:
		ret = get_futex_key(uaddr, flags & FLAGS_SHARED, &q->key, VERIFY_READ);

		if (unlikely(ret != 0))
			return ret;

retry_private:

		if(current->tgroup_distributed  && !(fn_flag & FLAGS_REMOTECALL) && !(flags & FLAGS_SHARED)){
#ifdef FUTEX_STAT
			perf_bb = native_read_tsc();
#endif
			current->uaddr = (unsigned long) uaddr;
			FPRINTK(KERN_ALERT"%s: pid{%d} setting uaddr{%lx} \n",__func__,current->pid,current->uaddr);
			g_errno = global_queue_wait_lock(q, uaddr, *hb, fn_flag, val,
					flags & FLAGS_SHARED, VERIFY_READ, bitset);
#ifdef FUTEX_STAT
			perf_cc = native_read_tsc();
#endif
			FPRINTK(KERN_ALERT " %s: spinlock  futex_wait_setup err {%d}\n",__func__,g_errno);
			if (g_errno != 0 && g_errno != WAIT_MAIN ) {	//error due to val change
#ifdef FUTEX_STAT
				_wait_err++;
#endif
				ret = g_errno;
				if( ret == -EFAULT)
				{
					FPRINTK(KERN_ALERT" client side efault fix up {%d} \n",fault_in_user_writeable(uaddr));

				}

			} else if (g_errno == 0 || g_errno == WAIT_MAIN) {	//no error => just queue it acquiring spinlock
				//get the actual spinlock : Not necessary as we are alone
				*hb = queue_lock(q);
				ret = g_errno;
			}

			goto out;
		}
		else{
			*hb = queue_lock(q);
		}

		ret = get_futex_value_locked(&uval, uaddr);

		if (ret) {
			queue_unlock(q,*hb);

			ret = get_user(uval, uaddr);
			if (ret)
				goto out;

			if (!(flags & FLAGS_SHARED))
				goto retry_private;

			put_futex_key(&q->key);
			goto retry;
		}

		if (uval != val) {
			queue_unlock(q,*hb);
			ret = -EWOULDBLOCK;
		}

would_block:
out:
		if (ret)
			put_futex_key(&q->key);
		if((fn_flag & FLAGS_REMOTECALL)){
			fn_flag = FLAGS_SYSCALL;
		}
		return ret;
	}

	static void printPTE(u32 __user *uaddr) {

		unsigned long address = (unsigned long) uaddr;
		address -= address % PAGE_SIZE;

		unsigned long pfn;
		pgd_t *pgd = NULL;
		pud_t *pud = NULL;
		pmd_t *pmd = NULL;
		pte_t *ptep = NULL;
		pte_t pte;
		pgd = pgd_offset(current->mm, address);
		if (!pgd_present(*pgd)) {
			goto exit;
		}
		pud = pud_offset(pgd, address);
		if (!pud_present(*pud)) {
			goto exit;
		}
		pmd = pmd_offset(pud, address);
		if (!pmd_present(*pmd)) {
			goto exit;
		}

		ptep = pte_offset_map(pmd, address);
		if (!ptep || !pte_present(*ptep)) {
			goto exit;
		}
		pte = *ptep;
		FPRINTK(KERN_ALERT"cpu {%d} pte ptr: 0x{%lx}\n", smp_processor_id(), pte);
		pfn = pte_pfn(pte);
		FPRINTK(KERN_ALERT" cpu{%d} pte pfn : 0x{%lx}\n", smp_processor_id(), pfn);

exit:
		FPRINTK("exit\n");
	}


	/*
	static void dump_regs(struct pt_regs* regs) {
		unsigned long fs, gs;
		printk(KERN_ALERT"DUMP REGS\n");
		if(NULL != regs) {
			FPRINTK(KERN_ALERT"r15{%lx}\n",regs->r15);
			FPRINTK(KERN_ALERT"r14{%lx}\n",regs->r14);
			FPRINTK(KERN_ALERT"r13{%lx}\n",regs->r13);
			FPRINTK(KERN_ALERT"r12{%lx}\n",regs->r12);
			FPRINTK(KERN_ALERT"r11{%lx}\n",regs->r11);
			FPRINTK(KERN_ALERT"r10{%lx}\n",regs->r10);
			FPRINTK(KERN_ALERT"r9{%lx}\n",regs->r9);
			FPRINTK(KERN_ALERT"r8{%lx}\n",regs->r8);
			printk(KERN_ALERT"bp{%lx}\n",regs->bp);
			printk(KERN_ALERT"bx{%lx}\n",regs->bx);
			printk(KERN_ALERT"ax{%lx}\n",regs->ax);
			printk(KERN_ALERT"cx{%lx}\n",regs->cx);
			printk(KERN_ALERT"dx{%lx}\n",regs->dx);
			printk(KERN_ALERT"di{%lx}\n",regs->di);
			printk(KERN_ALERT"orig_ax{%lx}\n",regs->orig_ax);
			printk(KERN_ALERT"ip{%lx}\n",regs->ip);
			printk(KERN_ALERT"cs{%lx}\n",regs->cs);
			printk(KERN_ALERT"flags{%lx}\n",regs->flags);
			printk(KERN_ALERT"sp{%lx}\n",regs->sp);
			printk(KERN_ALERT"ss{%lx}\n",regs->ss);
		}
		rdmsrl(MSR_FS_BASE, fs);
		rdmsrl(MSR_GS_BASE, gs);
		FPRINTK(KERN_ALERT"fs{%lx}\n",fs);
		printk(KERN_ALERT"gs{%lx}\n",gs);
		FPRINTK(KERN_ALERT"REGS DUMP COMPLETE\n");
	}
	*/

	//static
	int futex_wait(u32 __user *uaddr, unsigned int flags, u32 val,
			ktime_t *abs_time, u32 bitset, unsigned int fn_flag)
	{

#ifdef FUTEX_STAT
		unsigned long long wait_aa,wait_bb,wait_cc;
		if(current->tgroup_distributed){
			wait_aa = native_read_tsc();
			_wait_cnt++;
		}
#endif
		struct hrtimer_sleeper timeout, *to = NULL;
		struct restart_block *restart;
		struct futex_hash_bucket *hb;
		struct futex_q q = futex_q_init;
		int ret;
		int sig;

		struct task_struct *t=current;
		int rep_rem = t->tgroup_distributed;
		int x=0;
		int retf = 0;
		struct pt_regs * regs;
		/* unsigned long bp = stack_frame(current,NULL); */


		/*if(strcmp(current->comm,"mut") == 0){
		  printk(KERN_ALERT "FUTEX_WAIT:current {%pB} pid{%d} uaddr{%lx} get_user{%d} comm{%s}  syscall{%d} cpu{%d}\n",(void*) &bp,current->pid,uaddr,x,current->comm,fn_flag,smp_processor_id());
		  }*/
		FPRINTK(KERN_ALERT "FUTEX_WAIT:current {%pB} pid{%d} uaddr{%lx} get_user{%d} comm{%s}  syscall{%d} cpu{%d}\n",(void*) &bp,current->pid,uaddr,x,current->comm,fn_flag,smp_processor_id());

		//	printPTE(uaddr);
		if (!bitset){
			//if(current->tgroup_distributed)
			//printk(KERN_ALERT"%s: biset not right\n",__func__);
			return -EINVAL;
		}
		q.bitset = bitset;

		if (abs_time) {
			to = &timeout;

			hrtimer_init_on_stack(&to->timer, (flags & FLAGS_CLOCKRT) ?
					CLOCK_REALTIME : CLOCK_MONOTONIC,
					HRTIMER_MODE_ABS);
			hrtimer_init_sleeper(to, current);
			hrtimer_set_expires_range_ns(&to->timer, *abs_time,
					current->timer_slack_ns);
		}

retry:

		FPRINTK(KERN_ALERT "%s:wait before task {%d} rep_rem {%d}  uaddr{%lx}\ value{%d} disp{%d}\n",__func__,
				t->pid, t->tgroup_distributed, uaddr, x,current->return_disposition);
		/*
		 * Prepare to wait on uaddr. On success, holds hb lock and increments
		 * q.key refs.
		 */
		/*if((strcmp("cond",current->comm) == 0) || (strcmp("bar",current->comm) == 0)){
		  printk(KERN_ALERT"%s: distributed{%d} cpu{%d} pid {%d} uaddr{%d} \n",__func__,current->tgroup_distributed,smp_processor_id(),current->pid,uaddr);
		  }*/
		ret = futex_wait_setup(uaddr, val, flags, &q, &hb,fn_flag,bitset);

		if (ret !=0 && ret != WAIT_MAIN)
			goto out;


		/* queue_me and wait for wakeup, timeout, or a signal. */
		retf = futex_wait_queue_me(hb, &q, to, ret);
		/* If we were woken (and unqueued), we succeeded, whatever. */

		/* unqueue_me() drops q.key ref */
		if(ret != WAIT_MAIN  ){
			if(retf == 0 ){
				//			printk("%s ret %d is retf is %d",ret,retf);
				if (!unqueue_me(&q))
					goto out;
			}
		}
		ret = -ETIMEDOUT;
		if (to && !to->task)
			goto out;

		/*
		 * We expect signal_pending(current), but we might be the
		 * victim of a spurious wakeup as well.
		 */
		if (!signal_pending(current))
			goto retry;

		FPRINTK(KERN_ALERT" up for restart abs{%d} \n",(!abs_time)?0:1);

		ret = -ERESTARTSYS;
		if (!abs_time)
			goto out;
		restart = &current_thread_info()->restart_block;
		restart->fn = futex_wait_restart;
		restart->futex.uaddr = uaddr;
		restart->futex.val = val;
		restart->futex.time = abs_time->tv64;
		restart->futex.bitset = bitset;
		restart->futex.flags = flags | FLAGS_HAS_TIMEOUT;

		ret = -ERESTART_RESTARTBLOCK;

out:
		if (to) {
			hrtimer_cancel(&to->timer);
			destroy_hrtimer_on_stack(&to->timer);
		}
#ifdef FUTEX_STAT
		if(current->tgroup_distributed){
			wait_bb = native_read_tsc();
			_wait += wait_bb - wait_aa ;
		}
#endif
		FPRINTK(KERN_DEBUG " %s:exit {%d}\n",__func__,current->pid);
		return ret;
	}
	int print_wait_perf(){
		printk(KERN_ALERT"%s: cpu{%d} pid{%d} tgid{%d} counter{%d} errors{%d} wait time {%llu}",
				__func__,
				smp_processor_id(),
				current->pid,
				current->tgroup_home_id,
				_wait_cnt,
				_wait_err,
				_wait);
		_wait_err = 0;
		_wait = 0;
		_wait_cnt = 0;
	}

	int print_wake_perf(){
		printk(KERN_ALERT"%s: cpu{%d} pid{%d} tgid{%d} counter{%d} errors{%d} wake time {%llu}",
				__func__,
				smp_processor_id(),
				current->pid,
				current->tgroup_home_id,
				_wake_cnt,
				_wake_err,
				_wake);
		_wake_err = 0;
		_wake = 0;
		_wake_cnt = 0;
	}


	int print_wakeop_perf(){
		printk(KERN_ALERT"%s: cpu{%d} pid{%d} tgid{%d} counter{%d} errors{%d} wakeop time {%llu}",
				__func__,
				smp_processor_id(),
				current->pid,
				current->tgroup_home_id,
				_wakeop_cnt,
				_wakeop_err,
				_wakeop);
		_wakeop_err = 0;
		_wakeop = 0;
		_wakeop_cnt = 0;
	}

	int print_requeue_perf(){
		printk(KERN_ALERT"%s: cpu{%d} pid{%d} tgid{%d} counter{%d} errors{%d} requeue time {%llu}",
				__func__,
				smp_processor_id(),
				current->pid,
				current->tgroup_home_id,
				_requeue_cnt,
				_requeue_err,
				_requeue);
		_requeue_err = 0;
		_requeue = 0;
		_requeue_cnt = 0;
	}

	static long futex_wait_restart(struct restart_block *restart)
	{
		u32 __user *uaddr = restart->futex.uaddr;
		ktime_t t, *tp = NULL;
		if(current->tgroup_distributed==1)
			printk(KERN_ALERT"futex_restarted {%d} \n",uaddr);
		if (restart->futex.flags & FLAGS_HAS_TIMEOUT) {
			t.tv64 = restart->futex.time;
			tp = &t;
		}
		restart->fn = do_no_restart_syscall;

		return (long)futex_wait(uaddr, restart->futex.flags,
				restart->futex.val, tp, restart->futex.bitset,FLAGS_SYSCALL);
	}


	/*
	 * Userspace tried a 0 -> TID atomic transition of the futex value
	 * and failed. The kernel side here does the whole locking operation:
	 * if there are waiters then it will block, it does PI, etc. (Due to
	 * races the kernel might see a 0 value of the futex too.)
	 */
	static int futex_lock_pi(u32 __user *uaddr, unsigned int flags, int detect,
			ktime_t *time, int trylock)
	{
		struct hrtimer_sleeper timeout, *to = NULL;
		struct futex_hash_bucket *hb;
		struct futex_q q = futex_q_init;
		int res, ret;

		if (refill_pi_state_cache())
			return -ENOMEM;

		if (time) {
			to = &timeout;
			hrtimer_init_on_stack(&to->timer, CLOCK_REALTIME,
					HRTIMER_MODE_ABS);
			hrtimer_init_sleeper(to, current);
			hrtimer_set_expires(&to->timer, *time);
		}

retry:
		ret = get_futex_key(uaddr, flags & FLAGS_SHARED, &q.key, VERIFY_WRITE);
		if (unlikely(ret != 0))
			goto out;

retry_private:
		hb = queue_lock(&q);

		ret = futex_lock_pi_atomic(uaddr, hb, &q.key, &q.pi_state, current, 0);
		if (unlikely(ret)) {
			switch (ret) {
				case 1:
					/* We got the lock. */
					ret = 0;
					goto out_unlock_put_key;
				case -EFAULT:
					goto uaddr_faulted;
				case -EAGAIN:
					/*
					 * Task is exiting and we just wait for the
					 * exit to complete.
					 */
					queue_unlock(&q, hb);
					put_futex_key(&q.key);
					cond_resched();
					goto retry;
				default:
					goto out_unlock_put_key;
			}
		}

		/*
		 * Only actually queue now that the atomic ops are done:
		 */
		queue_me(&q, hb);

		WARN_ON(!q.pi_state);
		/*
		 * Block on the PI mutex:
		 */
		if (!trylock)
			ret = rt_mutex_timed_lock(&q.pi_state->pi_mutex, to, 1);
		else {
			ret = rt_mutex_trylock(&q.pi_state->pi_mutex);
			/* Fixup the trylock return value: */
			ret = ret ? 0 : -EWOULDBLOCK;
		}

		spin_lock(q.lock_ptr);
		/*
		 * Fixup the pi_state owner and possibly acquire the lock if we
		 * haven't already.
		 */
		res = fixup_owner(uaddr, &q, !ret);
		/*
		 * If fixup_owner() returned an error, proprogate that.  If it acquired
		 * the lock, clear our -ETIMEDOUT or -EINTR.
		 */
		if (res)
			ret = (res < 0) ? res : 0;

		/*
		 * If fixup_owner() faulted and was unable to handle the fault, unlock
		 * it and return the fault to userspace.
		 */
		if (ret && (rt_mutex_owner(&q.pi_state->pi_mutex) == current))
			rt_mutex_unlock(&q.pi_state->pi_mutex);

		/* Unqueue and drop the lock */
		unqueue_me_pi(&q);

		goto out_put_key;

out_unlock_put_key:
		queue_unlock(&q, hb);

out_put_key:
		put_futex_key(&q.key);
out:
		if (to)
			destroy_hrtimer_on_stack(&to->timer);
		return ret != -EINTR ? ret : -ERESTARTNOINTR;

uaddr_faulted:
		queue_unlock(&q, hb);

		ret = fault_in_user_writeable(uaddr);
		if (ret)
			goto out_put_key;

		if (!(flags & FLAGS_SHARED))
			goto retry_private;

		put_futex_key(&q.key);
		goto retry;
	}

	/*
	 * Userspace attempted a TID -> 0 atomic transition, and failed.
	 * This is the in-kernel slowpath: we look up the PI state (if any),
	 * and do the rt-mutex unlock.
	 */
	static int futex_unlock_pi(u32 __user *uaddr, unsigned int flags)
	{
		struct futex_hash_bucket *hb;
		struct futex_q *this, *next;
		struct plist_head *head;
		union futex_key key = FUTEX_KEY_INIT;
		u32 uval, vpid = task_pid_vnr(current);
		int ret;

retry:
		if (get_user(uval, uaddr))
			return -EFAULT;
		/*
		 * We release only a lock we actually own:
		 */
		if ((uval & FUTEX_TID_MASK) != vpid)
			return -EPERM;

		ret = get_futex_key(uaddr, flags & FLAGS_SHARED, &key, VERIFY_WRITE);
		if (unlikely(ret != 0))
			goto out;

		hb = hash_futex(&key);
		spin_lock(&hb->lock);

		/*
		 * To avoid races, try to do the TID -> 0 atomic transition
		 * again. If it succeeds then we can return without waking
		 * anyone else up:
		 */
		if (!(uval & FUTEX_OWNER_DIED) &&
				cmpxchg_futex_value_locked(&uval, uaddr, vpid, 0))
			goto pi_faulted;
		/*
		 * Rare case: we managed to release the lock atomically,
		 * no need to wake anyone else up:
		 */
		if (unlikely(uval == vpid))
			goto out_unlock;

		/*
		 * Ok, other tasks may need to be woken up - check waiters
		 * and do the wakeup if necessary:
		 */
		head = &hb->chain;

		plist_for_each_entry_safe(this, next, head, list) {
			if (!match_futex (&this->key, &key))
				continue;
			ret = wake_futex_pi(uaddr, uval, this);
			/*
			 * The atomic access to the futex value
			 * generated a pagefault, so retry the
			 * user-access and the wakeup:
			 */
			if (ret == -EFAULT)
				goto pi_faulted;
			goto out_unlock;
		}
		/*
		 * No waiters - kernel unlocks the futex:
		 */
		if (!(uval & FUTEX_OWNER_DIED)) {
			ret = unlock_futex_pi(uaddr, uval);
			if (ret == -EFAULT)
				goto pi_faulted;
		}

out_unlock:
		spin_unlock(&hb->lock);
		put_futex_key(&key);

out:
		return ret;

pi_faulted:
		spin_unlock(&hb->lock);
		put_futex_key(&key);

		ret = fault_in_user_writeable(uaddr);
		if (!ret)
			goto retry;

		return ret;
	}

	/**
	 * handle_early_requeue_pi_wakeup() - Detect early wakeup on the initial futex
	 * @hb:		the hash_bucket futex_q was original enqueued on
	 * @q:		the futex_q woken while waiting to be requeued
	 * @key2:	the futex_key of the requeue target futex
	 * @timeout:	the timeout associated with the wait (NULL if none)
	 *
	 * Detect if the task was woken on the initial futex as opposed to the requeue
	 * target futex.  If so, determine if it was a timeout or a signal that caused
	 * the wakeup and return the appropriate error code to the caller.  Must be
	 * called with the hb lock held.
	 *
	 * Return:
	 *  0 = no early wakeup detected;
	 * <0 = -ETIMEDOUT or -ERESTARTNOINTR
	 */
	static inline
		int handle_early_requeue_pi_wakeup(struct futex_hash_bucket *hb,
				struct futex_q *q, union futex_key *key2,
				struct hrtimer_sleeper *timeout)
		{
			int ret = 0;

			/*
			 * With the hb lock held, we avoid races while we process the wakeup.
			 * We only need to hold hb (and not hb2) to ensure atomicity as the
			 * wakeup code can't change q.key from uaddr to uaddr2 if we hold hb.
			 * It can't be requeued from uaddr2 to something else since we don't
			 * support a PI aware source futex for requeue.
			 */
			if (!match_futex(&q->key, key2)) {
				WARN_ON(q->lock_ptr && (&hb->lock != q->lock_ptr));
				/*
				 * We were woken prior to requeue by a timeout or a signal.
				 * Unqueue the futex_q and determine which it was.
				 */
				plist_del(&q->list, &hb->chain);

				/* Handle spurious wakeups gracefully */
				ret = -EWOULDBLOCK;
				if (timeout && !timeout->task)
					ret = -ETIMEDOUT;
				else if (signal_pending(current))
					ret = -ERESTARTNOINTR;
			}
			return ret;
		}

	/**
	 * futex_wait_requeue_pi() - Wait on uaddr and take uaddr2
	 * @uaddr:	the futex we initially wait on (non-pi)
	 * @flags:	futex flags (FLAGS_SHARED, FLAGS_CLOCKRT, etc.), they must be
	 * 		the same type, no requeueing from private to shared, etc.
	 * @val:	the expected value of uaddr
	 * @abs_time:	absolute timeout
	 * @bitset:	32 bit wakeup bitset set by userspace, defaults to all
	 * @uaddr2:	the pi futex we will take prior to returning to user-space
	 *
	 * The caller will wait on uaddr and will be requeued by futex_requeue() to
	 * uaddr2 which must be PI aware and unique from uaddr.  Normal wakeup will wake
	 * on uaddr2 and complete the acquisition of the rt_mutex prior to returning to
	 * userspace.  This ensures the rt_mutex maintains an owner when it has waiters;
	 * without one, the pi logic would not know which task to boost/deboost, if
	 * there was a need to.
	 *
	 * We call schedule in futex_wait_queue_me() when we enqueue and return there
	 * via the following--
	 * 1) wakeup on uaddr2 after an atomic lock acquisition by futex_requeue()
	 * 2) wakeup on uaddr2 after a requeue
	 * 3) signal
	 * 4) timeout
	 *
	 * If 3, cleanup and return -ERESTARTNOINTR.
	 *
	 * If 2, we may then block on trying to take the rt_mutex and return via:
	 * 5) successful lock
	 * 6) signal
	 * 7) timeout
	 * 8) other lock acquisition failure
	 *
	 * If 6, return -EWOULDBLOCK (restarting the syscall would do the same).
	 *
	 * If 4 or 7, we cleanup and return with -ETIMEDOUT.
	 *
	 * Return:
	 *  0 - On success;
	 * <0 - On error
	 */
	static int futex_wait_requeue_pi(u32 __user *uaddr, unsigned int flags,
			u32 val, ktime_t *abs_time, u32 bitset,
			u32 __user *uaddr2)
	{
		struct hrtimer_sleeper timeout, *to = NULL;
		struct rt_mutex_waiter rt_waiter;
		struct rt_mutex *pi_mutex = NULL;
		struct futex_hash_bucket *hb;
		union futex_key key2 = FUTEX_KEY_INIT;
		struct futex_q q = futex_q_init;
		int res, ret;

		if (uaddr == uaddr2)
			return -EINVAL;
		int retf = 0;
		if (!bitset)
			return -EINVAL;

		if (abs_time) {
			to = &timeout;
			hrtimer_init_on_stack(&to->timer, (flags & FLAGS_CLOCKRT) ?
					CLOCK_REALTIME : CLOCK_MONOTONIC,
					HRTIMER_MODE_ABS);
			hrtimer_init_sleeper(to, current);
			hrtimer_set_expires_range_ns(&to->timer, *abs_time,
					current->timer_slack_ns);
		}

		/*
		 * The waiter is allocated on our stack, manipulated by the requeue
		 * code while we sleep on uaddr.
		 */
		debug_rt_mutex_init_waiter(&rt_waiter);
		rt_waiter.task = NULL;

		ret = get_futex_key(uaddr2, flags & FLAGS_SHARED, &key2, VERIFY_WRITE);
		if (unlikely(ret != 0))
			goto out;

		q.bitset = bitset;
		q.rt_waiter = &rt_waiter;
		q.requeue_pi_key = &key2;

		/*
		 * Prepare to wait on uaddr. On success, increments q.key (key1) ref
		 * count.
		 */
		ret = futex_wait_setup(uaddr, val, flags, &q, &hb,FLAGS_SYSCALL,bitset);//temp modified
		if (ret)
			goto out_key2;

		/* Queue the futex_q, drop the hb lock, wait for wakeup. */
		retf = futex_wait_queue_me(hb, &q, to, 0);
		spin_lock(&hb->lock);
		ret = handle_early_requeue_pi_wakeup(hb, &q, &key2, to);
		spin_unlock(&hb->lock);
		if (ret)
			goto out_put_keys;

		/*
		 * In order for us to be here, we know our q.key == key2, and since
		 * we took the hb->lock above, we also know that futex_requeue() has
		 * completed and we no longer have to concern ourselves with a wakeup
		 * race with the atomic proxy lock acquisition by the requeue code. The
		 * futex_requeue dropped our key1 reference and incremented our key2
		 * reference count.
		 */

		/* Check if the requeue code acquired the second futex for us. */
		if (!q.rt_waiter) {
			/*
			 * Got the lock. We might not be the anticipated owner if we
			 * did a lock-steal - fix up the PI-state in that case.
			 */
			if (q.pi_state && (q.pi_state->owner != current)) {
				spin_lock(q.lock_ptr);
				ret = fixup_pi_state_owner(uaddr2, &q, current);
				spin_unlock(q.lock_ptr);
			}
		} else {
			/*
			 * We have been woken up by futex_unlock_pi(), a timeout, or a
			 * signal.  futex_unlock_pi() will not destroy the lock_ptr nor
			 * the pi_state.
			 */
			WARN_ON(!q.pi_state);
			pi_mutex = &q.pi_state->pi_mutex;
			ret = rt_mutex_finish_proxy_lock(pi_mutex, to, &rt_waiter, 1);
			debug_rt_mutex_free_waiter(&rt_waiter);

			spin_lock(q.lock_ptr);
			/*
			 * Fixup the pi_state owner and possibly acquire the lock if we
			 * haven't already.
			 */
			res = fixup_owner(uaddr2, &q, !ret);
			/*
			 * If fixup_owner() returned an error, proprogate that.  If it
			 * acquired the lock, clear -ETIMEDOUT or -EINTR.
			 */
			if (res)
				ret = (res < 0) ? res : 0;

			/* Unqueue and drop the lock. */
			unqueue_me_pi(&q);
		}

		/*
		 * If fixup_pi_state_owner() faulted and was unable to handle the
		 * fault, unlock the rt_mutex and return the fault to userspace.
		 */
		if (ret == -EFAULT) {
			if (pi_mutex && rt_mutex_owner(pi_mutex) == current)
				rt_mutex_unlock(pi_mutex);
		} else if (ret == -EINTR) {
			/*
			 * We've already been requeued, but cannot restart by calling
			 * futex_lock_pi() directly. We could restart this syscall, but
			 * it would detect that the user space "val" changed and return
			 * -EWOULDBLOCK.  Save the overhead of the restart and return
			 * -EWOULDBLOCK directly.
			 */
			ret = -EWOULDBLOCK;
		}

out_put_keys:
		put_futex_key(&q.key);
out_key2:
		put_futex_key(&key2);

out:
		if (to) {
			hrtimer_cancel(&to->timer);
			destroy_hrtimer_on_stack(&to->timer);
		}
		return ret;
	}

	/*
	 * Support for /get futexes: the kernel cleans up held futexes at
	 * thread exit time.
	 *
	 * Implementation: user-space maintains a per-thread list of locks it
	 * is holding. Upon do_exit(), the kernel carefully walks this list,
	 * and marks all locks that are owned by this thread with the
	 * FUTEX_OWNER_DIED bit, and wakes up a waiter (if any). The list is
	 * always manipulated with the lock held, so the list is private and
	 * per-thread. Userspace also maintains a per-thread 'list_op_pending'
	 * field, to allow the kernel to clean up if the thread dies after
	 * acquiring the lock, but just before it could have added itself to
	 * the list. There can only be one such pending lock.
	 */

	/**
	 * sys_set_robust_list() - Set the robust-futex list head of a task
	 * @head:	pointer to the list-head
	 * @len:	length of the list-head, as userspace expects
	 */
	SYSCALL_DEFINE2(set_robust_list, struct robust_list_head __user *, head,
			size_t, len)
	{
		if (current->tgroup_distributed == 1) {
                        printk("%s: pid %d\n", __func__, current->pid);
                }

		if (!futex_cmpxchg_enabled)
			return -ENOSYS;
		/*
		 * The kernel knows only one size for now:
		 */
		if (unlikely(len != sizeof(*head)))
			return -EINVAL;

		current->robust_list = head;

		return 0;
	}

	/**
	 * sys_get_robust_list() - Get the robust-futex list head of a task
	 * @pid:	pid of the process [zero for current task]
	 * @head_ptr:	pointer to a list-head pointer, the kernel fills it in
	 * @len_ptr:	pointer to a length field, the kernel fills in the header size
	 */
	SYSCALL_DEFINE3(get_robust_list, int, pid,
			struct robust_list_head __user * __user *, head_ptr,
			size_t __user *, len_ptr)
	{
		struct robust_list_head __user *head;
		unsigned long ret;
		struct task_struct *p;

		if (!futex_cmpxchg_enabled)
			return -ENOSYS;

		rcu_read_lock();

		ret = -ESRCH;
		if (!pid)
			p = current;
		else {
			p = find_task_by_vpid(pid);
			if (!p)
				goto err_unlock;
		}

		ret = -EPERM;
		if (!ptrace_may_access(p, PTRACE_MODE_READ))
			goto err_unlock;

		head = p->robust_list;
		rcu_read_unlock();

		if (put_user(sizeof(*head), len_ptr))
			return -EFAULT;
		return put_user(head, head_ptr);

err_unlock:
		rcu_read_unlock();

		return ret;
	}

	/*
	 * Process a futex-list entry, check whether it's owned by the
	 * dying task, and do notification if so:
	 */
	int handle_futex_death(u32 __user *uaddr, struct task_struct *curr, int pi)
	{
		u32 uval, uninitialized_var(nval), mval;

retry:
		if (get_user(uval, uaddr))
			return -1;

		if ((uval & FUTEX_TID_MASK) == task_pid_vnr(curr)) {
			/*
			 * Ok, this dying thread is truly holding a futex
			 * of interest. Set the OWNER_DIED bit atomically
			 * via cmpxchg, and if the value had FUTEX_WAITERS
			 * set, wake up a waiter (if any). (We have to do a
			 * futex_wake() even if OWNER_DIED is already set -
			 * to handle the rare but possible case of recursive
			 * thread-death.) The rest of the cleanup is done in
			 * userspace.
			 */
			mval = (uval & FUTEX_WAITERS) | FUTEX_OWNER_DIED;
			/*
			 * We are not holding a lock here, but we want to have
			 * the pagefault_disable/enable() protection because
			 * we want to handle the fault gracefully. If the
			 * access fails we try to fault in the futex with R/W
			 * verification via get_user_pages. get_user() above
			 * does not guarantee R/W access. If that fails we
			 * give up and leave the futex locked.
			 */
			if (cmpxchg_futex_value_locked(&nval, uaddr, uval, mval)) {
				if (fault_in_user_writeable(uaddr))
					return -1;
				goto retry;
			}
			if (nval != uval)
				goto retry;
			FPRINTK(KERN_ALERT " HANDLER:uaddr of death{%lx} ",uaddr);
			/*
			 * Wake robust non-PI futexes here. The wakeup of
			 * PI futexes happens in exit_pi_state():
			 */
			if (!pi && (uval & FUTEX_WAITERS))
			futex_wake(uaddr, 1, 1, FUTEX_BITSET_MATCH_ANY,FLAGS_SYSCALL, NULL);//modified
		}
		return 0;
	}

	/*
	 * Fetch a robust-list pointer. Bit 0 signals PI futexes:
	 */
	static inline int fetch_robust_entry(struct robust_list __user **entry,
			struct robust_list __user * __user *head,
			unsigned int *pi)
	{
		unsigned long uentry;

		if (get_user(uentry, (unsigned long __user *)head))
			return -EFAULT;

		*entry = (void __user *)(uentry & ~1UL);
		*pi = uentry & 1;

		return 0;
	}

	/*
	 * Walk curr->robust_list (very carefully, it's a userspace list!)
	 * and mark any locks found there dead, and notify any waiters.
	 *
	 * We silently return on any sign of list-walking problem.
	 */
	void exit_robust_list(struct task_struct *curr)
	{
		struct robust_list_head __user *head = curr->robust_list;
		struct robust_list __user *entry, *next_entry, *pending;
		unsigned int limit = ROBUST_LIST_LIMIT, pi, pip;
		unsigned int uninitialized_var(next_pi);
		unsigned long futex_offset;
		int rc;

		if (!futex_cmpxchg_enabled)
			return;

		/*
		 * Fetch the list head (which was registered earlier, via
		 * sys_set_robust_list()):
		 */
		if (fetch_robust_entry(&entry, &head->list.next, &pi))
		return;
		/*
		 * Fetch the relative futex offset:
		 */
		if (get_user(futex_offset, &head->futex_offset))
			return;
		/*
		 * Fetch any possibly pending lock-add first, and handle it
		 * if it exists:
		 */
		if (fetch_robust_entry(&pending, &head->list_op_pending, &pip))
			return;

		next_entry = NULL;	/* avoid warning with gcc */
		while (entry != &head->list) {
			/*
			 * Fetch the next entry in the list before calling
			 * handle_futex_death:
			 */
			rc = fetch_robust_entry(&next_entry, &entry->next, &next_pi);
			/*
			 * A pending lock might already be on the list, so
			 * don't process it twice:
			 */
			if (entry != pending)
				if (handle_futex_death((void __user *)entry + futex_offset,
							curr, pi))
					return;
			if (rc)
				return;
			entry = next_entry;
			pi = next_pi;
			/*
			 * Avoid excessively long or circular lists:
			 */
			if (!--limit)
				break;

			cond_resched();
		}

		if (pending)
			handle_futex_death((void __user *)pending + futex_offset,
					curr, pip);
	}

	long do_futex(u32 __user *uaddr, int op, u32 val, ktime_t *timeout,
			u32 __user *uaddr2, u32 val2, u32 val3)
	{
		//	FPRINTK(KERN_ALERT "uaddr {%d} \n",uaddr);
		int cmd = op & FUTEX_CMD_MASK;
		unsigned int flags = 0;
		unsigned int fn_flags = 0;
		fn_flags |= FLAGS_SYSCALL;

		if (!(op & FUTEX_PRIVATE_FLAG))
			flags |= FLAGS_SHARED;

		if (op & FUTEX_CLOCK_REALTIME) {
			if(current->tgroup_distributed == 1 &&  (op & FUTEX_PRIVATE_FLAG) ){
				flags = FUTEX_PRIVATE_FLAG;
				timeout = NULL;
				val3 = 1;
			}
			else{
				flags |= FLAGS_CLOCKRT;
				if (cmd != FUTEX_WAIT_BITSET && cmd != FUTEX_WAIT_REQUEUE_PI)
					return -ENOSYS;
			}
		}

		switch (cmd) {
			case FUTEX_LOCK_PI:
			case FUTEX_UNLOCK_PI:
			case FUTEX_TRYLOCK_PI:
			case FUTEX_WAIT_REQUEUE_PI:
			case FUTEX_CMP_REQUEUE_PI:
				if (!futex_cmpxchg_enabled)
					return -ENOSYS;
		}

		switch (cmd) {
			case FUTEX_WAIT:
				val3 = FUTEX_BITSET_MATCH_ANY;
			case FUTEX_WAIT_BITSET:
				return futex_wait(uaddr, flags, val, timeout, val3, fn_flags);
			case FUTEX_WAKE:
				val3 = FUTEX_BITSET_MATCH_ANY;
			case FUTEX_WAKE_BITSET:
				return futex_wake(uaddr, flags, val, val3, fn_flags, NULL);
			case FUTEX_REQUEUE:
				return futex_requeue(uaddr, flags, uaddr2, val, val2, NULL, 0, fn_flags, NULL);
			case FUTEX_CMP_REQUEUE:
				return futex_requeue(uaddr, flags, uaddr2, val, val2, &val3, 0, fn_flags, NULL);
			case FUTEX_WAKE_OP:
				return futex_wake_op(uaddr, flags, uaddr2, val, val2, val3, fn_flags, NULL);
			case FUTEX_LOCK_PI:
				return futex_lock_pi(uaddr, flags, val, timeout, 0);
			case FUTEX_UNLOCK_PI:
				return futex_unlock_pi(uaddr, flags);
			case FUTEX_TRYLOCK_PI:
				return futex_lock_pi(uaddr, flags, 0, timeout, 1);
			case FUTEX_WAIT_REQUEUE_PI:
				val3 = FUTEX_BITSET_MATCH_ANY;
				return futex_wait_requeue_pi(uaddr, flags, val, timeout, val3,
						uaddr2);
			case FUTEX_CMP_REQUEUE_PI:
				return futex_requeue(uaddr, flags, uaddr2, val, val2, &val3, 1, fn_flags, NULL);
		}
		return -ENOSYS;
	}
#if 0
	int dump_processor_regs_futex(struct pt_regs* regs) {
		int ret = -1;
		unsigned long fs, gs;
		unsigned long fsindex, gsindex;

		if (regs == NULL) {
			printk(KERN_ERR"process_server: invalid params to dump_processor_regs()");
			goto exit;
		}
		dump_stack();
		printk(KERN_ALERT"DUMP REGS %s\n", __func__);

		if (NULL != regs) {
			printk(KERN_ALERT"r15{%lx}\n",regs->r15);
			printk(KERN_ALERT"r14{%lx}\n",regs->r14);
			printk(KERN_ALERT"r13{%lx}\n",regs->r13);
			printk(KERN_ALERT"r12{%lx}\n",regs->r12);
			printk(KERN_ALERT"r11{%lx}\n",regs->r11);
			printk(KERN_ALERT"r10{%lx}\n",regs->r10);
			printk(KERN_ALERT"r9{%lx}\n",regs->r9);
			printk(KERN_ALERT"r8{%lx}\n",regs->r8);
			printk(KERN_ALERT"bp{%lx}\n",regs->bp);
			printk(KERN_ALERT"bx{%lx}\n",regs->bx);
			printk(KERN_ALERT"ax{%lx}\n",regs->ax);
			printk(KERN_ALERT"cx{%lx}\n",regs->cx);
			printk(KERN_ALERT"dx{%lx}\n",regs->dx);
			printk(KERN_ALERT"di{%lx}\n",regs->di);
			printk(KERN_ALERT"orig_ax{%lx}\n",regs->orig_ax);
			printk(KERN_ALERT"ip{%lx}\n",regs->ip);
			printk(KERN_ALERT"cs{%lx}\n",regs->cs);
			printk(KERN_ALERT"flags{%lx}\n",regs->flags);
			printk(KERN_ALERT"sp{%lx}\n",regs->sp);
			printk(KERN_ALERT"ss{%lx}\n",regs->ss);
		}
		rdmsrl(MSR_FS_BASE, fs);
		rdmsrl(MSR_GS_BASE, gs);
		printk(KERN_ALERT"fs{%lx} - %lx content %lx\n",fs, current->thread.fs, fs ? * (unsigned long*) fs : 0x1234567l);
		printk(KERN_ALERT"gs{%lx} - %lx content %lx\n",gs, current->thread.gs, fs ? * (unsigned long*)gs : 0x1234567l);

		savesegment(fs, fsindex);
		savesegment(gs, gsindex);
		printk(KERN_ALERT"fsindex{%lx} - %x\n",fsindex, current->thread.fsindex);
		printk(KERN_ALERT"gsindex{%lx} - %x\n",gsindex, current->thread.gsindex);
		printk(KERN_ALERT"REGS DUMP COMPLETE\n");
		ret = 0;

	exit:
		return ret;
	}
#endif

int dump_processor_regs_futex(struct pt_regs *regs)
{
	int i;

	if (regs == NULL) {
		printk(KERN_ERR"process_server: invalid params to dump_processor_regs()");
		return;
	}

	dump_stack();

	printk(KERN_ALERT"DUMP REGS %s\n", __func__);

	if (NULL != regs) {
		printk(KERN_ALERT"sp: 0x%lx\n", regs->sp);
		printk(KERN_ALERT"pc: 0x%lx\n", regs->pc);
		printk(KERN_ALERT"pstate: 0x%lx\n", regs->pstate);

		for (i = 0; i < 31; i++) {
			printk(KERN_ALERT"regs[%d]: 0x%lx\n", i, regs->regs[i]);
		}
	}
}
	SYSCALL_DEFINE6(futex, u32 __user *, uaddr, int, op, u32, val,
			struct timespec __user *, utime, u32 __user *, uaddr2,
			u32, val3)
	{
		struct timespec ts;
		ktime_t t, *tp = NULL;
		u32 val2 = 0;
		int cmd = op & FUTEX_CMD_MASK;
		int retn=0;
		if (current->tgroup_distributed == 1) {
			printk("%s: WARN: uadd 0x%lx op %d val %d utime 0x%lx uaddr2 0x%lx val3 %d - pid %d (cpu %d id %d)\n",
					__func__, (unsigned long)uaddr, val, op, (unsigned long)utime, (unsigned long)uaddr2, val3,
					current->pid, current->tgroup_home_cpu, current->tgroup_home_id);
		}
		if (utime && (cmd == FUTEX_WAIT || cmd == FUTEX_LOCK_PI ||
					 cmd == FUTEX_WAIT_BITSET ||
					 cmd == FUTEX_WAIT_REQUEUE_PI)) {
			 if ((retn = copy_from_user(&ts, utime, sizeof(ts))) != 0){
				 //if(current->tgroup_distributed)
				 //	printk(KERN_ALERT"%s: retn {%d}\n",retn);
				 return -EFAULT;
			 }
			 if (!timespec_valid(&ts))
				 return -EINVAL;

			 t = timespec_to_ktime(ts);
			 if (cmd == FUTEX_WAIT)
				 t = ktime_add_safe(ktime_get(), t);
			 tp = &t;
		 }
		 /*
		  * requeue parameter in 'utime' if cmd == FUTEX_*_REQUEUE_*.
		  * number of waiters to wake in 'utime' if cmd == FUTEX_WAKE_OP.
		  */
		 if (cmd == FUTEX_REQUEUE || cmd == FUTEX_CMP_REQUEUE ||
				 cmd == FUTEX_CMP_REQUEUE_PI || cmd == FUTEX_WAKE_OP)
			 val2 = (u32) (unsigned long) utime;
		 retn =  do_futex(uaddr, op, val, tp, uaddr2, val2, val3);

		 return retn;
	}

	static int __init futex_init(void)
	{
		u32 curval;
		int i;
		_cpu= Kernel_Id;

		/*
		 * This will fail and we want it. Some arch implementations do
		 * runtime detection of the futex_atomic_cmpxchg_inatomic()
		 * functionality. We want to know that before we call in any
		 * of the complex code paths. Also we want to prevent
		 * registration of robust lists in that case. NULL is
		 * guaranteed to fault and we get -EFAULT on functional
		 * implementation, the non-functional ones will return
		 * -ENOSYS.
		 */
		if (cmpxchg_futex_value_locked(&curval, NULL, 0, 0) == -EFAULT)
			futex_cmpxchg_enabled = 1;

		for (i = 0; i < ARRAY_SIZE(futex_queues); i++) {
			plist_head_init(&futex_queues[i].chain);
			spin_lock_init(&futex_queues[i].lock);
		}



		return 0;
	}
	__initcall(futex_init);
