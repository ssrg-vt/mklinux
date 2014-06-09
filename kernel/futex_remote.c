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
#include <linux/signal.h>

#include <linux/pid.h>
#include <linux/types.h>

#include <linux/pcn_kmsg.h>
#include <linux/delay.h>
#include <popcorn/remote_pfn.h>
#include <popcorn/pid.h>
#include <asm/page_types.h>

#include "futex_remote.h"
#define ENOTINKRN 999
#define MODULE "GRQ-"
#include <popcorn/global_spinlock.h>

#define FUTEX_REMOTE_VERBOSE 1 
#if FUTEX_REMOTE_VERBOSE
#define FRPRINTK(...) printk(__VA_ARGS__)
#else
#define FRPRINTK(...) ;
#endif

#ifdef CONFIG_PPC_BOOK3E_64
#define is_kernel_addr(x)       ((x) >= 0x8000000000000000ul)
#else
#define is_kernel_addr(x)       ((x) >= PAGE_OFFSET)
#endif

#define GENERAL_SPIN_LOCK(x) spin_lock(x)
#define GENERAL_SPIN_UNLOCK(x) spin_unlock(x)
#define WAKE_OPS 1
#define WAIT_OPS 0

DEFINE_SPINLOCK(access_global_value_table);

static struct workqueue_struct *grq;
static pid_t worker_pid;
static volatile unsigned int free_work = 0;
static volatile unsigned int finish_work = 0;

static DECLARE_WAIT_QUEUE_HEAD(wait_);
static DECLARE_WAIT_QUEUE_HEAD(resume_);

static atomic_t progress = ATOMIC_INIT(0);

static unsigned int counter = 0;
extern struct list_head pfn_list_head;

extern int unqueue_me(struct futex_q *q);



void _spin_key_init (struct spin_key *st) {
 	st->_tgid = 0;
 	st->_uaddr = 0;
 	st->offset = 0;
 }


//void _spin_key_init (struct spin_key *st);
extern int getKey(unsigned long uaddr, _spin_key *sk, pid_t tgid);
#define  NSIG 32

static struct list_head vm_head;

extern int __kill_something_info(int sig, struct siginfo *info, pid_t pid);

static void dump_regs(struct pt_regs* regs) {
	unsigned long fs, gs;
	FRPRINTK(KERN_ALERT"DUMP REGS\n");
	if (NULL != regs) {
		FRPRINTK(KERN_ALERT"r15{%lx}\n",regs->r15);
		FRPRINTK(KERN_ALERT"r14{%lx}\n",regs->r14);
		FRPRINTK(KERN_ALERT"r13{%lx}\n",regs->r13);
		FRPRINTK(KERN_ALERT"r12{%lx}\n",regs->r12);
		FRPRINTK(KERN_ALERT"r11{%lx}\n",regs->r11);
		FRPRINTK(KERN_ALERT"r10{%lx}\n",regs->r10);
		FRPRINTK(KERN_ALERT"r9{%lx}\n",regs->r9);
		FRPRINTK(KERN_ALERT"r8{%lx}\n",regs->r8);
		FRPRINTK(KERN_ALERT"bp{%lx}\n",regs->bp);
		FRPRINTK(KERN_ALERT"bx{%lx}\n",regs->bx);
		FRPRINTK(KERN_ALERT"ax{%lx}\n",regs->ax);
		FRPRINTK(KERN_ALERT"cx{%lx}\n",regs->cx);
		FRPRINTK(KERN_ALERT"dx{%lx}\n",regs->dx);
		FRPRINTK(KERN_ALERT"di{%lx}\n",regs->di);
		FRPRINTK(KERN_ALERT"orig_ax{%lx}\n",regs->orig_ax);
		FRPRINTK(KERN_ALERT"ip{%lx}\n",regs->ip);
		FRPRINTK(KERN_ALERT"cs{%lx}\n",regs->cs);
		FRPRINTK(KERN_ALERT"flags{%lx}\n",regs->flags);
		FRPRINTK(KERN_ALERT"sp{%lx}\n",regs->sp);
		FRPRINTK(KERN_ALERT"ss{%lx}\n",regs->ss);
	}
	rdmsrl(MSR_FS_BASE, fs);
	rdmsrl(MSR_GS_BASE, gs);
	FRPRINTK(KERN_ALERT"fs{%lx}\n",fs);
	FRPRINTK(KERN_ALERT"gs{%lx}\n",gs);
	FRPRINTK(KERN_ALERT"REGS DUMP COMPLETE\n");
}


struct _inc_remote_vm_pool {
	unsigned long rflag;
	int pid;
	int origin_pid;
	struct list_head list_member;
};



int find_kernel_for_pfn(unsigned long addr, struct list_head *head) {
	struct list_head *iter;
	_pfn_range_list_t *objPtr;

	list_for_each(iter, head)
	{
		objPtr = list_entry(iter, _pfn_range_list_t, pfn_list_member);
		if (addr > objPtr->start_pfn_addr && addr < objPtr->end_pfn_addr)
			return objPtr->kernel_number;
	}

	return -1;
}

struct vm_area_struct * getVMAfromUaddr(unsigned long uaddr) {

	unsigned long address = (unsigned long) uaddr;
	unsigned long offset = address % PAGE_SIZE;
	if (unlikely((address % sizeof(u32)) != 0))
		return NULL;
	address -= offset;

	struct vm_area_struct *vma;
	struct vm_area_struct* curr = NULL;
	curr = current->mm->mmap;
	vma = find_extend_vma(current->mm, address);
	if (!vma)
		return NULL;
	else
		return vma;
}

typedef struct _inc_remote_vm_pool _inc_remote_vm_pool_t;

_inc_remote_vm_pool_t * add_inc(int pid, int start, int end, int rflag,
		int origin_pid, struct list_head *head) {
	_inc_remote_vm_pool_t *Ptr = (_inc_remote_vm_pool_t *) kmalloc(
			sizeof(_inc_remote_vm_pool_t), GFP_ATOMIC);

	Ptr->pid = pid;
	Ptr->rflag = rflag;
	Ptr->origin_pid = origin_pid;
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

pte_t *do_page_walk(unsigned long address) {
	pgd_t *pgd = NULL;
	pud_t *pud = NULL;
	pmd_t *pmd = NULL;
	pte_t *ptep = NULL;
	pte_t *pte;

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
	pte = ptep;

	return (pte_t*) pte;
	exit: return NULL;
}


void wake_futex_global(struct futex_q *q) {
	struct task_struct *p = q->task;

	get_task_struct(p);

	unqueue_me(q);

	smp_wmb();
	q->lock_ptr = NULL;

	wake_up_state(p, TASK_NORMAL);
	put_task_struct(p);
}

struct task_struct* gettask(pid_t origin_pid, pid_t tghid) {
	struct task_struct *tsk = NULL;
	struct task_struct *g, *task = NULL;

	tsk = pid_task(find_vpid(origin_pid), PIDTYPE_PID);
	if (tsk) {
		FRPRINTK(KERN_ALERT "origin id exists \n");
	} else {
		do_each_thread(g, task)
		{
			if (task->pid == origin_pid) {
				tsk = task;
				goto mm_exit;
			}
		}
		while_each_thread(g, task);
	}
	tsk = pid_task(find_vpid(tghid), PIDTYPE_PID);
	if (tsk) {
		FRPRINTK(KERN_ALERT "tghid id exists \n");
	} else {
		do_each_thread(g, task)
		{
			if (task->pid == tghid) {
				tsk = task;
				goto mm_exit;
			}
		}
		while_each_thread(g, task);
	}
	mm_exit: return tsk;
}

int global_futex_wake(u32 __user *uaddr, unsigned int flags, int nr_wake,
		u32 bitset, pid_t pid, unsigned long uaddr2) {
	struct futex_hash_bucket *hb;
	struct futex_q *this, *next;
	struct plist_head *head;
	struct task_struct *tsk;
	union futex_key key = FUTEX_KEY_INIT;
	struct mm_struct *cmm = NULL;
	struct task_struct *temp;
	int ret;
	FRPRINTK(KERN_ALERT "%s: entry response {%d} uaddr{%lx} comm{%s} flags{%u} uaddr2{%lx} \n",__func__,pid,uaddr,current->comm,flags,uaddr2);
	if (!bitset)
		return -EINVAL;

	struct spin_key sk;
	_spin_key_init(&sk);

	tsk = pid_task(find_vpid(pid), PIDTYPE_PID);
	if (tsk) {
		cmm = current->mm;
		current->mm = tsk->mm;
	}
	getKey((uaddr2 == 0) ? (unsigned long)uaddr : (unsigned long) uaddr2, &sk,(!tsk)?current->tgroup_home_id:tsk->tgroup_home_id);
	_spin_value *value = hashspinkey(&sk);
	_local_rq_t * l= find_request_by_pid(pid, &value->_lrq_head);
	
	FRPRINTK(KERN_ALERT "%s: set wake up \n",__func__);

	ret = get_futex_key((uaddr2 == 0) ?uaddr : (u32 __user*) uaddr2,
			((flags & FLAGS_DESTROY == 256) ?
					(0 & FLAGS_SHARED) : (flags & FLAGS_SHARED)), &key, VERIFY_READ);

	FRPRINTK(KERN_ALERT "%s: after get key ptr {%p} mm{%p} \n",__func__,key.both.ptr,current->mm);

	hb = hash_futex(&key);
	spin_lock(&hb->lock);
	head = &hb->chain;

	plist_for_each_entry_safe(this, next, head, list)
	{
		temp = this->task;
		if (temp /*&& is_kernel_addr(temp)*/) {
			if (temp->tgroup_distributed == 1
					&& temp->tgroup_home_id == tsk->tgroup_home_id
					&& temp->pid == tsk->pid) {

				if (this->lock_ptr != NULL && spin_is_locked(this->lock_ptr)) {
					FRPRINTK(KERN_ALERT"%s:Hold locking spinlock \n",__func__);
					//spin_unlock(&this->lock_ptr);
				}
				FRPRINTK(KERN_ALERT"%s:call wake futex \n",__func__);
				wake_futex(this);
			}
		}

	}
	spin_unlock(&hb->lock);
	put_futex_key(&key);
out:

	if (cmm == NULL) {
		FRPRINTK(KERN_ALERT"%s:cmm NULL\n",__func__);
		current->mm = cmm;
	} else {
		FRPRINTK(KERN_ALERT"%s:current {%s}\n",current->comm,__func__);
		current->mm = NULL;
	}

	FRPRINTK(KERN_ALERT "%s:exit \n",__func__);

	return ret;
}


int fix_user_page(u32 __user * uaddr,struct task_struct *tsk){
struct mm_struct *mm=tsk->mm;
int ret;

down_read(&mm->mmap_sem);
ret =fixup_user_fault(tsk,mm, (unsigned long) uaddr, FAULT_FLAG_WRITE| FAULT_FLAG_NONLINEAR | FAULT_FLAG_MKWRITE |FAULT_FLAG_KILLABLE );
up_read(&mm->mmap_sem);

return ret < 0 ? ret :0;

}
int global_futex_wait(unsigned long uaddr, unsigned int flags, u32 val,
		ktime_t *abs_time, u32 bitset, pid_t rem, struct task_struct *origin,
		unsigned int fn_flags) {
	struct futex_hash_bucket *hb;
	struct task_struct *tsk = origin;
	struct task_struct *rem_struct = NULL;
	struct futex_q *q = (struct futex_q *) kmalloc(sizeof(struct futex_q),
			GFP_ATOMIC); //futex_q_init;
	
	q->key = FUTEX_KEY_INIT;
	q->bitset = FUTEX_BITSET_MATCH_ANY;
	q->rem_pid = -1;
	q->req_addr = 0;
	q->rem_requeue_key = FUTEX_KEY_INIT;

	u32 uval;
	int ret;
	int sig;
	int prio;

	q->bitset = bitset;

	//start wait setup
retry:
	
	ret = get_futex_key((u32 __user *)uaddr, flags & FLAGS_SHARED, &q->key, VERIFY_READ);
	FRPRINTK(KERN_ALERT "%s: pid origin {%s} _cpu{%d} uaddr{%lx} uval{%d} ret{%d} \n ",__func__,tsk->comm,smp_processor_id(),uaddr,val,ret);
	if (unlikely(ret != 0))
	   return ret;

	//set private.mm to origin tasks mm
	if(tsk)
		q->key.private.mm = tsk->mm;

retry_private:
	//queue_lock
	hb = hash_futex(&q->key);
	q->lock_ptr = &hb->lock;
	spin_lock(&hb->lock);
fault:
	ret = get_futex_value_locked(&uval, (u32 __user *)uaddr);

	if (ret) {
			spin_unlock(&hb->lock);
			FRPRINTK(KERN_ALERT "%s:after spin unlock ret{%d} uval{%lx}\n ",__func__,ret,uval);

			if(ret == -EFAULT){
				if((ret = fix_user_page((u32 __user *)uaddr,tsk)) ==  0){
					printk(KERN_ALERT"%s:ret {%d} \n",__func__,ret);
				}

			}

			ret = get_user(uval, (u32 __user *)uaddr);

			if (ret){
				FRPRINTK(KERN_ALERT "%s:after get user out ret{%d} uval{%lx}\n ",__func__,ret,uval);
				goto out;
			}

			if (!(flags & FLAGS_SHARED))
				goto retry_private;

			put_futex_key(&q->key);
			goto retry;
	}

	if (uval != val) {
			spin_unlock(&hb->lock);
			ret = -EWOULDBLOCK;
	}

	if(ret)
		goto out;

	//queue me for origin node shall be made by the local thread itself
	rem_struct =  pid_task(find_vpid(rem), PIDTYPE_PID);
	if(rem_struct){
		FRPRINTK(KERN_ALERT "%s:local request unlock\n ",__func__);
		spin_unlock(&hb->lock);
		goto out;
	}
	//no need to schedule as the rem_struct will be waiting for the ack from server.


	//queue me the dummy node for remote
	prio = 100; //min(current->normal_prio, MAX_RT_PRIO);
	plist_node_init(&q->list, prio);
	plist_add(&q->list, &hb->chain);

			q->task = NULL;
			q->rem_pid = rem;

	FRPRINTK(KERN_ALERT "%s:global request unlock queue me \n ",__func__);
	spin_unlock(&hb->lock);

	//no need to schedule as the remote will schedule();

out:
	if(ret){
		put_futex_key(&q->key);
	}

	FRPRINTK(KERN_ALERT "%s: hb {%p} key: word {%lx} offset{%d} ptr{%p} mm{%p}\n ",__func__,
			hb,q->key.both.word,q->key.both.offset,q->key.both.ptr,q->key.private.mm);
	FRPRINTK(KERN_ALERT "%s:exit\n",__func__);
	return ret;
}


void global_worher_fn(struct work_struct* work) {
	global_request_work_t* w = (global_request_work_t*) work;

	 _global_rq *this, *next;
	struct task_struct *tsk = current;
	struct task_struct *task, *g;
	struct mm_struct *cmm = NULL;
	int null_flag = 0;
	int exch_value;
	static unsigned has_work =0;
	unsigned long flags =0;
	//Set task struct for the worker
	worker_pid = current->pid;

	FRPRINTK(KERN_ALERT "%s:GRQ started {%s}\n",__func__,current->comm);

        this = (_global_rq*) w->gq;

	has_work = 0;
	FRPRINTK(KERN_ALERT "%s:retry has_work{%d} \n",__func__,has_work);


	struct spin_key sk;
	_spin_key_init(&sk);

		if (this->ops == WAKE_OPS) //process wake request from GRQ
		{
			_remote_wakeup_request_t* msg = (_remote_wakeup_request_t*) &this->wakeup;
			int ret =0;

			getKey(msg->uaddr, &sk,msg->tghid);
			_spin_value *value = hashspinkey(&sk);
			FRPRINTK(KERN_ALERT"%s:wake--current msg pid{%d} msg->ticket{%d} \n", __func__,msg->pid,msg->ticket);

			if (msg->rflag == 0 || (msg->fn_flag & FLAGS_ORIGINCALL)) {
					if (current->mm != NULL) {
						null_flag = 1;
						cmm = (current->mm);
					}

					tsk = pid_task(find_vpid(msg->tghid), PIDTYPE_PID);
					if (tsk) {
						current->mm = tsk->mm;
						FRPRINTK(KERN_ALERT "tghid exist cmm{%d}  cmm{%p} comm{%s} mm{%p}\n",(cmm!=NULL)?1:0,cmm,current->comm,current->mm);
					} else {
						do_each_thread(g, task){
							if (task->pid == msg->tghid) {
								current->mm = task->mm;
								FRPRINTK(KERN_ALERT "tghid-> mm struct found comm{%s} cmm{%d} mm{%d} \n",task->comm,(cmm!=NULL)?1:0, (current->mm!=NULL)?1:0);
								goto mm_exit;
							}
						}
						while_each_thread(g, task);
					}
mm_exit:
					FRPRINTK(KERN_ALERT "%s: before wake cmm{%d}  mm{%d} cmm{%p}  mm{%p} msg->fn_flag{%u}\n",__func__,(cmm!=NULL)?1:0,(current->mm!=NULL)?1:0,cmm,current->mm,msg->fn_flag);

					msg->fn_flag |= FLAGS_REMOTECALL;

					if (msg->fn_flag & FLAGS_WAKECALL)
						ret = futex_wake(msg->uaddr, msg->flags, msg->nr_wake, msg->bitset,
								msg->fn_flag);

					else if (msg->fn_flag & FLAGS_REQCALL)
						ret = futex_requeue(msg->uaddr, msg->flags, (unsigned long)  (msg->uaddr2 & ((1600*PAGE_SIZE)-1)), msg->nr_wake,
								msg->nr_wake2, &(msg->cmpval),0, msg->fn_flag);

					else if (msg->fn_flag & FLAGS_WAKEOPCALL)
						ret = futex_wake_op((u32 __user*)msg->uaddr, msg->flags,(u32 __user*)msg->uaddr2, msg->nr_wake,
								msg->nr_wake2, msg->cmpval, msg->fn_flag,tsk);

					if (cmm != NULL && null_flag) {
						FRPRINTK(KERN_ALERT "assign the original mm struct back for task {%d}\n",current->pid);
						current->mm = cmm;
					} else if (cmm == NULL && !null_flag) {
						FRPRINTK(KERN_ALERT "assign the null mm struct back {%d} \n",current->pid);
						current->mm = NULL;
					} else {
						FRPRINTK(KERN_ALERT "whatever {%d} \n",current->pid);
						current->mm = NULL;
					}
				}
				FRPRINTK(KERN_ALERT "%s:after setting mm to NULL\n",__func__);

					//send ticket
					_remote_wakeup_response_t send_tkt;
					send_tkt.header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_RESPONSE;
					send_tkt.header.prio = PCN_KMSG_PRIO_NORMAL;
					send_tkt.errno =ret ;
					send_tkt.request_id=msg->ticket;
					send_tkt.uaddr = msg->uaddr;
					send_tkt.rem_pid = msg->pid;
					FRPRINTK(KERN_ALERT "send ticket to wake request {%d} msg->pid{%d} msg->uaddr{%lx} \n",send_tkt.rem_pid,msg->pid,msg->uaddr);
					pcn_kmsg_send(ORIG_NODE(send_tkt.rem_pid), (struct pcn_kmsg_message*) (&send_tkt));


		} else if(this->ops == WAIT_OPS){ //wait request

			_remote_key_request_t* msg = (_remote_key_request_t*) &this->wait;
			int ret =0 ;

			getKey(msg->uaddr, &sk,msg->tghid);
			_spin_value *value = hashspinkey(&sk);

			FRPRINTK(KERN_ALERT"%s:wait --current msg pid{%d} msg->ticket{%d} \n", __func__,msg->pid,msg->ticket);

			tsk = gettask(msg->tghid, msg->tghid);
			if (msg->fn_flags & FLAGS_ORIGINCALL) {
				msg->fn_flags |= FLAGS_REMOTECALL;
				ret = global_futex_wait(msg->uaddr, msg->flags, msg->val, 0, 0, msg->pid, tsk,
						msg->fn_flags);
			} else
				ret = global_futex_wait(msg->uaddr, msg->flags, msg->val, 0, 0, msg->pid, tsk,
							msg->fn_flags);


					//send response
					_remote_key_response_t send_tkt;
					send_tkt.header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_RESPONSE;
					send_tkt.header.prio = PCN_KMSG_PRIO_NORMAL;
					send_tkt.errno =ret ;
					send_tkt.request_id=msg->ticket;
					send_tkt.uaddr = msg->uaddr;
					send_tkt.rem_pid = msg->pid;
					FRPRINTK(KERN_ALERT "send ticket to wait request {%d} msg->pid{%d} msg->uaddr{%lx} \n",send_tkt.rem_pid,msg->pid,msg->uaddr);
					pcn_kmsg_send(ORIG_NODE(send_tkt.rem_pid), (struct pcn_kmsg_message*) (&send_tkt));



			}
cleanup:
		//Delete the entry
		counter++;
		FRPRINTK(KERN_ALERT "done iteration moving the head cnt{%d} counter{%d} \n",this->cnt,counter);

		kfree(this);


exit:
	kfree(work);

}

static int handle_remote_futex_wake_response(struct pcn_kmsg_message* inc_msg) {
	_remote_wakeup_response_t* msg = (_remote_wakeup_response_t*) inc_msg;
	preempt_disable();
	
	FRPRINTK(KERN_ALERT"%s: response {%d} \n",
			__func__, msg->errno);
	struct task_struct *p =  pid_task(find_vpid(msg->rem_pid), PIDTYPE_PID);

	get_task_struct(p);

	struct spin_key sk;
	_spin_key_init(&sk);

	getKey(msg->uaddr, &sk,p->tgroup_home_id);
	_spin_value *value = hashspinkey(&sk);
	// update the local value status to has ticket


	_local_rq_t *ptr = set_err_request(msg->request_id,msg->errno, &value->_lrq_head);
       // smp_wmb();
	FRPRINTK(KERN_ALERT"%s: errno{%d} p->tgp(%d} \n",__func__,msg->errno,p->tgroup_home_id);
//	wake_up_interruptible(&ptr->_wq);

	put_task_struct(p);

	pcn_kmsg_free_msg(inc_msg);
	
	preempt_enable();

	return 0;
}

static int handle_remote_futex_wake_request(struct pcn_kmsg_message* inc_msg) {

	_remote_wakeup_request_t* msg = (_remote_wakeup_request_t*) inc_msg;
	_remote_wakeup_response_t response;
	struct task_struct *tsk = current;
	struct task_struct *task, *g;
	struct mm_struct *cmm = NULL;
	int null_flag = 0;
	_global_value * gvp;

	unsigned long flags;
	atomic_inc(&progress);

	FRPRINTK(KERN_ALERT"%s: request -- entered task comm{%s} pid{%d} msg->fn_flag{%lx} msg-flags{%lx}\n", __func__,tsk->comm,tsk->pid,msg->fn_flag,msg->flags);
	FRPRINTK(KERN_ALERT"%s: msg: uaddr {%lx}  uaddr2{%lx} ticket {%d} tghid{%d} bitset {%u} rflag{%d} pid{%d} ifn_flags{%lx}\n",
			__func__,msg->uaddr,(msg->uaddr2 & (PAGE_SIZE-1)),msg->ticket,msg->tghid,msg->bitset,msg->rflag,msg->pid,msg->fn_flag);

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;
	tsk =  pid_task(find_vpid(msg->tghid), PIDTYPE_PID);
        
	if(( (msg->fn_flag < FLAGS_MAX) && (msg->fn_flag & FLAGS_ORIGINCALL)) || msg->rflag == 0){
	GENERAL_SPIN_LOCK(&access_global_value_table);

	gvp = hashgroup(tsk);
	if (!gvp->free) { //futex_wake is the first one
		//scnprintf(gvp->name, sizeof(gvp->name), MODULE);
	 	gvp ->free =1;
	}
	FRPRINTK(KERN_ALERT"%s: wake gvp free \n", __func__);
	gvp->global_wq = grq;// create_singlethread_workqueue(gvp->name);
	gvp->thread_group_leader = tsk;
	global_request_work_t* back_work = NULL;
	// Spin up bottom half to process this event
	back_work = (global_request_work_t*) kmalloc(sizeof(global_request_work_t),
				GFP_ATOMIC);
	if (back_work) {
			INIT_WORK((struct work_struct* )back_work, global_worher_fn);

			FRPRINTK(KERN_ALERT"%s: set up head\n", __func__);
			
			back_work->lock = &gvp->lock; 
			
			_global_rq *trq = (_global_rq *) kmalloc(sizeof(_global_rq), GFP_ATOMIC);			
			memcpy(&trq->wakeup, msg, sizeof(_remote_wakeup_request_t));
			trq->ops = WAKE_OPS;
			trq->cnt = atomic_read(&progress);
			//smp_mb();barrier()i;

			back_work->gq = trq;
			queue_work(gvp->global_wq, (struct work_struct*) back_work);
		}
	gvp->worker_task = back_work;
	GENERAL_SPIN_UNLOCK(&access_global_value_table);


	FRPRINTK(KERN_ALERT"%s: ERROR msg ticket{%d}\n", __func__,msg->ticket);

	//Check whether the request is asking for ticket or holding the ticket?
		//if not holding the ticket add to the tail of Global request queue.
		//if not holding the ticket add to the tail of Global request queue.
	}
	else {
                FRPRINTK(KERN_ALERT"need to wake_st uaddr2{%lx} \n",(msg->uaddr2 & ((1600*PAGE_SIZE)-1)));
		global_futex_wake(msg->uaddr, msg->flags, msg->nr_wake, msg->bitset,
								msg->rflag,(unsigned long) (msg->uaddr2 & ((1600*PAGE_SIZE)-1)));
	}
	
	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

int remote_futex_wakeup(u32 __user *uaddr, unsigned int flags, int nr_wake,
		u32 bitset, union futex_key *key, int rflag, unsigned int fn_flags,
		unsigned long uaddr2, int nr_requeue, int cmpval) {

	int res = 0;
	int cpu = 0;
	struct page *page, *page_head;
	_remote_wakeup_request_t *request = kmalloc(sizeof(_remote_wakeup_request_t),
			GFP_ATOMIC);
	FRPRINTK(KERN_ALERT"%s: -- entered whos calling{%s} \n", __func__,current->comm);
	// Build request

	request->header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;

	request->bitset = bitset;
	request->nr_wake = nr_wake;
	request->flags = flags;
	request->uaddr = uaddr;
	request->uaddr2 = (unsigned long) uaddr2;
	request->nr_wake2 = nr_requeue;
	request->cmpval = cmpval;
	request->fn_flag = fn_flags;

	request->tghid = rflag;
	request->rflag = rflag;
	request->pid = rflag;
	request->ops = 1;
	request->ticket = 1; //wake operations are always loack removing operations

	int x = 0, y = 0;
	int wake = 0, woke = 0, nw = 0, bs = 0;

	FRPRINTK(KERN_ALERT" %s: pid{%d}  cpu{%d} rflag{%d} uaddr{%lx} get_user{%d} fn_flags{%lx}\n ",__func__,
			current->pid,smp_processor_id(),rflag,uaddr,x,fn_flags);

	//	dump_regs(task_pt_regs(current));

	// Send response
       	if (rflag) {
		request->fn_flag &= 1;
		FRPRINTK(KERN_ALERT "%s: sending to remote node {%d} flag{%lx}\n",__func__, ORIG_NODE(rflag),request->fn_flag);
		res = pcn_kmsg_send(ORIG_NODE(rflag), (struct pcn_kmsg_message*) (request));// no need for remote lock to wake up
	}

out:
	kfree(request);
	return res;

}
static int handle_remote_futex_key_response(struct pcn_kmsg_message* inc_msg) {
	_remote_key_response_t* msg = (_remote_key_response_t*) inc_msg;
	preempt_disable();
	
	FRPRINTK(KERN_ALERT"%s: response to revoke wait request as origin is dead {%d} \n",
			__func__,msg->errno);

	struct task_struct *p =  pid_task(find_vpid(msg->rem_pid), PIDTYPE_PID);

	get_task_struct(p);

	struct spin_key sk;
	_spin_key_init(&sk);

	getKey(msg->uaddr, &sk,p->tgroup_home_id);
	_spin_value *value = hashspinkey(&sk);
	// update the local value status to has ticket

	FRPRINTK(KERN_ALERT"%s:  value {%d}  p->tgroup_home_id{%d}  \n",
					__func__, value->_st,p->tgroup_home_id);
	//smp_wmb();

	_local_rq_t *ptr = set_err_request(msg->request_id,msg->errno, &value->_lrq_head);
	
//	wake_up_interruptible(&ptr->_wq);

	put_task_struct(p);

	pcn_kmsg_free_msg(inc_msg);
	
	preempt_enable();
	
	return 0;
}

static int handle_remote_futex_key_request(struct pcn_kmsg_message* inc_msg) {

	_remote_key_request_t* msg = (_remote_key_request_t*) inc_msg;
	_remote_key_response_t response;
	struct task_struct *tsk;
	int res;
	_global_value * gvp;
	unsigned long flags;
	 atomic_inc(&progress);

	FRPRINTK(KERN_ALERT"%s: request -- entered whos calling{%s} \n", __func__,current->comm);
	FRPRINTK(KERN_ALERT"%s: msg: uaddr {%lx} flags {%lx} val{%d}  pid{%d}  fn_flags{%lx} ticket{%d}\n",
			__func__,msg->uaddr,msg->flags,msg->val,msg->pid,msg->fn_flags,msg->ticket);

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	tsk =  pid_task(find_vpid(msg->tghid), PIDTYPE_PID);

	GENERAL_SPIN_LOCK(&access_global_value_table);
	gvp = hashgroup(tsk);
	if(!gvp->free) {
		gvp->free =1;
	}
		//futex wait is the first global request
	FRPRINTK(KERN_ALERT"%s: wait gvp free \n", __func__);
//	scnprintf(gvp->name, sizeof(gvp->name), MODULE);

	gvp->global_wq = grq;//create_singlethread_workqueue(gvp->name);
	gvp->thread_group_leader = tsk;
	global_request_work_t* back_work = NULL;

		// Spin up bottom half to process this event
		back_work = (global_request_work_t*) kmalloc(sizeof(global_request_work_t),
				GFP_ATOMIC);
		if (back_work) {
			INIT_WORK((struct work_struct* )back_work, global_worher_fn);
			FRPRINTK(KERN_ALERT"%s: set up head\n", __func__);
			back_work->lock = &gvp->lock; // , sizeof(spinlock_t));
			
			_global_rq *trq = (_global_rq *) kmalloc(sizeof(_global_rq), GFP_ATOMIC);
			memcpy(&trq->wait, msg, sizeof(_remote_key_request_t));
			trq->cnt =atomic_read(&progress);
			trq->ops = WAIT_OPS;

			back_work->gq = trq;
			FRPRINTK(KERN_ALERT"%s: wait token aqc trq->wait.ticket{%d} cnt{%d}\n", __func__,trq->wait.ticket,trq->cnt);
			
			queue_work(grq, (struct work_struct*) back_work);
		}
	gvp->worker_task = back_work;//pid_task(find_vpid(worker_pid), PIDTYPE_PID);
	GENERAL_SPIN_UNLOCK(&access_global_value_table);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

int futex_global_worker_cleanup(struct task_struct *tsk){


   if(tsk->tgroup_distributed && tsk->pid == tsk->tgroup_home_id){
   _global_value * gvp = hashgroup(tsk);
    FRPRINTK(KERN_INFO "GVP EXISTS{%s} tgid{%d} pid{%d} \n",tsk->comm,tsk->tgroup_home_id,tsk->pid);

	if(gvp != NULL){

		FRPRINTK(KERN_INFO"Inside GVP");
		gvp->thread_group_leader = NULL;
		gvp->free = 0;
		gvp->global_wq = NULL;
		gvp->worker_task =NULL;
		FRPRINTK(KERN_INFO "cleaned up \n");
	}
    }

	return 0;
}

static int __init futex_remote_init(void)
{

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_REQUEST,
			handle_remote_futex_key_request);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_RESPONSE,
			handle_remote_futex_key_response);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_REQUEST,
			handle_remote_futex_wake_request);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_RESPONSE,
			handle_remote_futex_wake_response);

	grq   = create_singlethread_workqueue(MODULE);
	worker_pid=-1;

	INIT_LIST_HEAD(&vm_head);


	return 0;
}
__initcall(futex_remote_init);

/*mklinux_akshay*/
