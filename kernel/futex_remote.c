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

DEFINE_SPINLOCK(access_global_value_table);

static struct workqueue_struct *grq;

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

typedef struct global_request_work {
	struct work_struct work;
	spinlock_t * lock;
	struct plist_head * _grq_head;
	int ops ; //0-wait 1-wake
} global_request_work_t;


struct _inc_remote_vm_pool {
	unsigned long rflag;
	int pid;
	int origin_pid;
	struct list_head list_member;
};

struct send_ticket_request {
	struct pcn_kmsg_hdr header;
	pid_t rem_pid;			//4
	unsigned long uaddr;	//8
	char pad[48];
}__attribute__((packed)) __attribute__((aligned(64)));

typedef struct send_ticket_request send_ticket_request_t;

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

struct futex_q ** query_q_pid(struct task_struct *t, int kernel) {
	int i = 0, cnt = 0;
	struct futex_hash_bucket *hb;
	struct plist_head *head;
	struct futex_q *this, *next = NULL;
	struct futex_q **res = NULL;
	res = (struct futex_q **) kmalloc(sizeof(res) * 10, GFP_ATOMIC);

	if(!res)
		FRPRINTK(KERN_ALERT "unable to kmmaloc\n");

	for (i = 0; i < 10; i++)
		res[cnt] = (struct futex_q *) kmalloc(sizeof(struct futex_q), GFP_ATOMIC);

	int end = 1 << _FUTEX_HASHBITS;
	struct task_struct *temp;
	for (i = 0; i < end; i++) {
		hb = &futex_queues[i];
		if (hb != NULL) {
			spin_lock(&hb->lock);
			head = &hb->chain;
			plist_for_each_entry_safe(this, next, head, list)
			{
				temp = this->task;
				if (temp /*&& is_kernel_addr(temp)*/&& !kernel) {
					get_task_struct(temp);
					if (temp->tgroup_distributed == 1
							&& (temp->tgroup_home_id == t->tgroup_home_id
									|| temp->pid == t->pid)) {
						res[cnt] = this;
						cnt++;
					}
					put_task_struct(temp);
				} else if (this && !temp && kernel) {
					if (this->rem_pid > 1) {
						FRPRINTK(KERN_ALERT "%s: rem_pid{%d} \n","query_q_pid",this->rem_pid);
						res[cnt] = this;
						cnt++;
					}
				}
			}
			spin_unlock(&hb->lock);
		}
	}
	q_exit: return res;

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
		u32 bitset, pid_t pid) {
	struct futex_hash_bucket *hb;
	struct futex_q *this, *next;
	struct plist_head *head;
	struct task_struct *tsk;
	union futex_key key = FUTEX_KEY_INIT;
	struct mm_struct *cmm = NULL;
	struct task_struct *temp;
	int ret;
	FRPRINTK(KERN_ALERT "%s: entry response {%d} uaddr{%lx} comm{%s} flags{%u} \n",__func__,pid,uaddr,current->comm,flags);
	if (!bitset)
		return -EINVAL;

	tsk = pid_task(find_vpid(pid), PIDTYPE_PID);
	if (tsk) {
		cmm = current->mm;
		current->mm = tsk->mm;
	}

	ret = get_futex_key(uaddr,
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



int global_futex_wait(u32 __user *uaddr, unsigned int flags, u32 val,
		ktime_t *abs_time, u32 bitset, pid_t rem, struct task_struct *origin,
		unsigned int fn_flags) {
	struct futex_hash_bucket *hb;
	struct task_struct *tsk = origin;
	struct futex_q *q = (struct futex_q *) kmalloc(sizeof(struct futex_q),
			GFP_ATOMIC); //futex_q_init;
	q->key = FUTEX_KEY_INIT;
	q->bitset = FUTEX_BITSET_MATCH_ANY;
	q->rem_pid = -1;
	u32 uval;
	int ret;
	int sig;
	q->bitset = bitset;

	ret = get_futex_key(uaddr, flags & FLAGS_SHARED, &q->key, VERIFY_READ);
	FRPRINTK(KERN_ALERT "%s: pid origin {%s} _cpu{%d} uaddr{%lx} disp{%d} \n ",__func__,tsk->comm,smp_processor_id(),uaddr, current->return_disposition);

	if (tsk)
		q->key.private.mm = tsk->mm;

	hb = hash_futex(&q->key);
	q->lock_ptr = &hb->lock;
	spin_lock(&hb->lock);
	int prio;
	prio = 100; //min(current->normal_prio, MAX_RT_PRIO);
	if (fn_flags & FLAGS_ORIGINCALL) {
		q->task = pid_task(find_vpid(rem), PIDTYPE_PID);
	} else {
		q->task = NULL;
		q->rem_pid = rem;
	}
	plist_node_init(&q->list, prio);
	plist_add(&q->list, &hb->chain);
	spin_unlock(&hb->lock);
out:
	FRPRINTK(KERN_ALERT "%s: hb {%p} key: word {%lx} offset{%d} ptr{%p} mm{%p}\n ",__func__,
			hb,q->key.both.word,q->key.both.offset,q->key.both.ptr,q->key.private.mm);
	FRPRINTK(KERN_ALERT "%s:exit\n",__func__);
	return ret;
}


static int handle_remote_futex_token_request(struct pcn_kmsg_message* inc_msg) {
	send_ticket_request_t* msg = (send_ticket_request_t*) inc_msg;

	FRPRINTK(KERN_ALERT"%s:  pid {%d} uaddr{%lx} \n",
			__func__, msg->rem_pid,msg->uaddr);


	struct task_struct *p =  pid_task(find_vpid(msg->rem_pid), PIDTYPE_PID);

	get_task_struct(p);

	struct spin_key sk;
	_spin_key_init(&sk);

	getKey(msg->uaddr, &sk,p->tgroup_home_id);
	_spin_value *value = hashspinkey(&sk);
	// update the local value status to has ticket
	//cmpxchg(&value->_st, INITIAL_STATE, HAS_TICKET);
	if(value->_st == INITIAL_STATE){
		value->_st = HAS_TICKET;
	}
	FRPRINTK(KERN_ALERT"%s:  value {%d}  p->tgroup_home_id{%d}  \n",
					__func__, value->_st,p->tgroup_home_id);
	smp_wmb();
	set_task_state(p,TASK_INTERRUPTIBLE);
	wake_up_process(p);

	put_task_struct(p);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}


void global_worher_fn(struct work_struct* work) {
	global_request_work_t* w = (global_request_work_t*) work;

	 _global_rq *this, *next;
	struct plist_head * head;
	struct task_struct *tsk = current;
	struct task_struct *task, *g;
	struct mm_struct *cmm = NULL;
	int null_flag = 0;
	int is_alive = 0;
	int exch_value;
	static unsigned int token_status = 0;

	FRPRINTK(KERN_ALERT "%s:GRQ started {%s}\n",__func__,current->comm);


	//TODO:inifinite loop till the thread dies or forcefully thread is stopped
	while (1) {

	retry:
		head = w->_grq_head;
		plist_for_each_entry_safe(this, next, head, list)
		{
			is_alive = 1;
			struct spin_key sk;
			_spin_key_init(&sk);

			if (this->ops == 1) //process wake request from GRQ
			{
				_remote_wakeup_request_t* msg = (_remote_wakeup_request_t*) &this->wakeup;

				getKey(msg->uaddr, &sk,msg->tghid);
				_spin_value *value = hashspinkey(&sk);
				if(token_status){
					if(msg->ticket != 1) {
						FRPRINTK(KERN_ALERT "%s:GRQ retry until it recv token {%d}\n",__func__);
						schedule();
						goto retry;
					}
				}
				else
					FRPRINTK(KERN_ALERT"%s:wake--current msg pid{%} token_status{%d} \n", __func__,msg->pid,token_status);

				if(msg->ticket == 1 && value->_st == HAS_TICKET){ //for wake the replication and release lock are done using same request

				if (msg->rflag == 0 || (msg->fn_flag & FLAGS_ORIGINCALL)) {
					if (current->mm != NULL) {
						null_flag = 1;
						cmm = (current->mm);
					}

					tsk = pid_task(find_vpid(msg->origin_pid), PIDTYPE_PID);
					if (tsk) {
						FRPRINTK(KERN_ALERT "%s:origin id exists tsk pid{%d}\n",__func__, tsk->pid);
						current->mm = tsk->mm;
						goto mm_exit;
					} else {
						do_each_thread(g, task){
							if (task->pid == msg->origin_pid) {
								current->mm = task->mm;
								FRPRINTK(KERN_ALERT "origin -> mm struct found comm{%s} cmm{%d} mm{%d} \n",task->comm,(cmm!=NULL)?1:0, (current->mm!=NULL)?1:0);
								goto mm_exit;
							}
						}
						while_each_thread(g, task);
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
						futex_wake(msg->uaddr, msg->flags, msg->nr_wake, msg->bitset,
								msg->fn_flag);

					else if (msg->fn_flag & FLAGS_REQCALL)
						futex_requeue(msg->uaddr, msg->flags, msg->uaddr2, msg->nr_wake,
								msg->nr_wake2,
								NULL, 0, msg->fn_flag);

					else if (msg->fn_flag & FLAGS_WAKEOPCALL)
						futex_wake_op(msg->uaddr, msg->flags, msg->uaddr2, msg->nr_wake,
								msg->nr_wake2, 0, msg->fn_flag);

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

				token_status = 0;
				FRPRINTK(KERN_ALERT "%s:after setting mm to NULL\n",__func__);
				}
				else if(msg->ticket == 0){
					//TODO: check the global lock status and send

					exch_value = cmpxchg(&value->_st, INITIAL_STATE, HAS_TICKET);
					//send ticket
					send_ticket_request_t send_tkt;
					send_tkt.header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_TOKEN_REQUEST;
					send_tkt.header.prio = PCN_KMSG_PRIO_NORMAL;
					send_tkt.uaddr = msg->uaddr;
					send_tkt.rem_pid = msg->pid;
					FRPRINTK(KERN_ALERT "send ticket to wake request {%d} msg->pid{%d} msg->uaddr{%lx} \n",send_tkt.rem_pid,msg->pid,msg->uaddr);
					pcn_kmsg_send(ORIG_NODE(send_tkt.rem_pid), (struct pcn_kmsg_message*) (&send_tkt));
					token_status = 1;
				}
			} else { //wait request
				_remote_key_request_t* msg = (_remote_key_request_t*) &this->wait;

				getKey(msg->uaddr, &sk,msg->tghid);
				_spin_value *value = hashspinkey(&sk);

				if(token_status){
					if(msg->ticket != 1 || msg->ticket != 2){
						FRPRINTK(KERN_ALERT "%s:GRQ retry until it recv token {%d}\n",__func__);
						schedule();
					goto retry;
					}
				}else
					FRPRINTK(KERN_ALERT"%s:wait --current msg pid{%d} token_status{%d} \n", __func__,msg->pid,token_status);

				if(msg->ticket == 1 && value->_st == HAS_TICKET){// perform replication activity
				tsk = gettask(msg->origin_pid, msg->tghid);
				if (msg->fn_flags & FLAGS_ORIGINCALL) {
					msg->fn_flags |= FLAGS_REMOTECALL;
					global_futex_wait(msg->uaddr, msg->flags, msg->val, 0, 0, msg->pid, tsk,
							msg->fn_flags);
				} else
					global_futex_wait(msg->uaddr, msg->flags, msg->val, 0, 0, msg->pid, tsk,
							msg->fn_flags);

					token_status = 1;
				}
				else if(msg->ticket == 2 && value->_st == HAS_TICKET){// Release Lock
					//release lock
					FRPRINTK(KERN_ALERT "release lock ticket \n");
					exch_value = cmpxchg(&value->_st, HAS_TICKET, INITIAL_STATE);
					token_status = 0;
				}
				else if(msg->ticket == 0){// Get Lock

					exch_value = cmpxchg(&value->_st, INITIAL_STATE, HAS_TICKET);
					//send ticket
					send_ticket_request_t send_tkt;
					send_tkt.header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_TOKEN_REQUEST;
					send_tkt.header.prio = PCN_KMSG_PRIO_NORMAL;
					send_tkt.uaddr = msg->uaddr;
					send_tkt.rem_pid = msg->pid;
					FRPRINTK(KERN_ALERT "send ticket to wait request {%d} msg->pid{%d} msg->uaddr{%lx} \n",send_tkt.rem_pid,msg->pid,msg->uaddr);
					pcn_kmsg_send(ORIG_NODE(send_tkt.rem_pid), (struct pcn_kmsg_message*) (&send_tkt));
					token_status = 1;

				}

			}
cleanup:
			//Delete the entry
			plist_del(&this->list, w->_grq_head);
			kfree(this);
		}

		if (is_alive)
			continue;

	}

exit:
	kfree(work);

}

static int handle_remote_futex_wake_response(struct pcn_kmsg_message* inc_msg) {
	_remote_wakeup_response_t* msg = (_remote_wakeup_response_t*) inc_msg;

	FRPRINTK(KERN_ALERT"%s: response {%d} \n",
			__func__, msg->errno);

	pcn_kmsg_free_msg(inc_msg);

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

	FRPRINTK(KERN_ALERT"%s: request -- entered task comm{%s} pid{%d} msg->fn_flag{%lx} msg-flags{%lx}\n", __func__,tsk->comm,tsk->pid,msg->fn_flag,msg->flags);
	FRPRINTK(KERN_ALERT"%s: msg: uaddr {%lx} tgid {%d} tghid{%d} bitset {%u} rflag{%d} pid{%d} origin_pid {%d} \n",
			__func__,msg->uaddr,msg->ops,msg->tghid,msg->bitset,msg->rflag,msg->pid,msg->origin_pid);

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;
	tsk =  pid_task(find_vpid(msg->tghid), PIDTYPE_PID);

	if(msg->rflag == 0){
	GENERAL_SPIN_LOCK(&access_global_value_table);

	gvp = hashgroup(tsk);
	if (!gvp->free) { //futex_wake is the first one
		//scnprintf(gvp->name, sizeof(gvp->name), MODULE);
		gvp->global_wq = grq;// create_singlethread_workqueue(gvp->name);
		gvp->free = 1;
		gvp->thread_group_leader = tsk;
		global_request_work_t* back_work = NULL;

		// Spin up bottom half to process this event
		back_work = (global_request_work_t*) kmalloc(sizeof(global_request_work_t),
				GFP_ATOMIC);
		if (back_work) {
			INIT_WORK((struct work_struct* )back_work, global_worher_fn);
			back_work->_grq_head = &gvp->_grq_head;//, sizeof(struct plist_head));
			back_work->lock = &gvp->lock; // , sizeof(spinlock_t));
			back_work->ops = -1;
			queue_work(gvp->global_wq, (struct work_struct*) back_work);
		}
		gvp->free =1;
	} else {
		//TODO: check if it is the same thread group leader if not hash it to another bucket.
	}
	GENERAL_SPIN_UNLOCK(&access_global_value_table);

	//Check whether the request is asking for ticket or holding the ticket?
	if (!msg->ticket) {
		//if not holding the ticket add to the tail of Global request queue.
		_global_rq *trq = (_global_rq *) kmalloc(sizeof(_global_rq), GFP_ATOMIC);
		//if not holding the ticket add to the tail of Global request queue.
		memcpy(&trq->wakeup, msg, sizeof(_remote_wakeup_request_t));
		trq->ops = 1;
		plist_node_init(&trq->list, NORMAL_Q_PRIORITY);
		plist_add(&trq->list, &gvp->_grq_head);

	} else if(msg->ticket == 1 ) {
		_global_rq *trq = (_global_rq *) kmalloc(sizeof(_global_rq), GFP_ATOMIC);
		//if not holding the ticket add to the tail of Global request queue.
		memcpy(&trq->wakeup, msg, sizeof(_remote_wakeup_request_t));
		trq->ops = 1;
		plist_node_init(&trq->list, HIGH_Q_PRIORITY);
		plist_add(&trq->list, &gvp->_grq_head);
	}
	else if(msg->ticket == 2 ) {
			_global_rq *trq = (_global_rq *) kmalloc(sizeof(_global_rq), GFP_ATOMIC);
			//if not holding the ticket add to the tail of Global request queue.
			memcpy(&trq->wakeup, msg, sizeof(_remote_wakeup_request_t));
			trq->ops = 1;
			plist_node_init(&trq->list, HIGH1_Q_PRIORITY);
			plist_add(&trq->list, &gvp->_grq_head);
		}
	}
	else
		global_futex_wake(msg->uaddr, msg->flags, msg->nr_wake, msg->bitset,
								msg->rflag);
	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

int remote_futex_wakeup(u32 __user *uaddr, unsigned int flags, int nr_wake,
		u32 bitset, union futex_key *key, int rflag, unsigned int fn_flags,
		u32 __user *uaddr2, int nr_requeue, int cmpval) {

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
	request->uaddr2 = uaddr2;
	request->nr_wake2 = nr_requeue;
	request->cmpval = cmpval;
	request->fn_flag = fn_flags;

	struct vm_area_struct *vma;
	vma = getVMAfromUaddr(uaddr);

	request->tghid = current->tgroup_home_id;
	request->rflag = rflag;
	request->pid = current->pid;
	request->origin_pid = current->origin_pid;
	request->ops = 1;
	request->ticket = 1; //wake operations are always loack removing operations

	int x = 0, y = 0;
	int wake = 0, woke = 0, nw = 0, bs = 0;

	FRPRINTK(KERN_ALERT" %s: pfn {%lx} shift {%lx} pid{%d} origin_pid{%d} cpu{%d} rflag{%d} uaddr{%lx} vma start (%lx} vma end (%lx} get_user{%d} fn_flags{%lx}\n ",__func__,
			vma->vm_pgoff,vma->vm_pgoff << PAGE_SHIFT,current->pid,current->origin_pid,smp_processor_id(),rflag,uaddr,vma->vm_start, vma->vm_end,x,fn_flags);

	//	dump_regs(task_pt_regs(current));

	// Send response
	if ((fn_flags & FLAGS_ORIGINCALL) || !rflag) {
		if (vma->vm_flags & VM_PFNMAP) {

			res = -ENOTINKRN;
			unsigned long pfn;
			pte_t pte;
			pte = *((pte_t *) do_page_walk((unsigned long )uaddr));
			pfn = pte_pfn(pte);
			FRPRINTK(KERN_ALERT"remote futex wake pte ptr : ox{%lx} pfn: 0x{%lx} cpu{%d}\n",pte,pfn,smp_processor_id());

exit:

			if ((cpu = find_kernel_for_pfn(pfn, &pfn_list_head)) != -1)
			{
				FRPRINTK(KERN_ALERT"%s: sending to origin pfn cpu: 0x{%d}\n",__func__,cpu);
				res = pcn_kmsg_send(cpu, (struct pcn_kmsg_message*) (request));
				struct spin_key sk;
				_spin_key_init(&sk);

				getKey(uaddr, &sk,current->tgroup_home_id);
				_spin_value *value = hashspinkey(&sk);
				cmpxchg(&value->_st, HAS_TICKET, INITIAL_STATE);// release lock in remote node
			}

		} else if ((fn_flags & FLAGS_ORIGINCALL)) {
			FRPRINTK(KERN_ALERT "%s: sending to origin node (self) {%d}\n",__func__, ORIG_NODE(rflag));
			res = pcn_kmsg_send(ORIG_NODE(rflag), (struct pcn_kmsg_message*) (request));// lock will be released in handle request
		}

	}

	else if (rflag) {
		FRPRINTK(KERN_ALERT "%s: sending to remote node {%d}\n",__func__, ORIG_NODE(rflag));
		res = pcn_kmsg_send(ORIG_NODE(rflag), (struct pcn_kmsg_message*) (request));// no need for remote lock to wake up
	}

out:
	kfree(request);
	return res;

}
static int handle_remote_futex_key_response(struct pcn_kmsg_message* inc_msg) {
	_remote_key_response_t* msg = (_remote_key_response_t*) inc_msg;

	FRPRINTK(KERN_ALERT"%s: response to revoke wait request as origin is dead {%d} \n",
			__func__,msg->origin_pid);

	global_futex_wake(msg->uaddr, msg->flags, msg->val, 1, msg->pid);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

static int handle_remote_futex_key_request(struct pcn_kmsg_message* inc_msg) {

	_remote_key_request_t* msg = (_remote_key_request_t*) inc_msg;
	_remote_key_response_t response;
	struct task_struct *tsk;
	int res;
	_global_value * gvp;

	FRPRINTK(KERN_ALERT"%s: request -- entered whos calling{%s} \n", __func__,current->comm);
	FRPRINTK(KERN_ALERT"%s: msg: uaddr {%lx} flags {%lx} val{%d}  pid{%d} origin_pid {%d} fn_flags{%lx}\n",
			__func__,msg->uaddr,msg->flags,msg->val,msg->pid,msg->origin_pid,msg->fn_flags);

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	tsk =  pid_task(find_vpid(msg->tghid), PIDTYPE_PID);

	GENERAL_SPIN_LOCK(&access_global_value_table);
	gvp = hashgroup(tsk);
	if(!gvp->free) {
		//futex wait is the first global request
		scnprintf(gvp->name, sizeof(gvp->name), MODULE);

		gvp->global_wq = grq;//create_singlethread_workqueue(gvp->name);
		gvp->free = 1;
		gvp->thread_group_leader = tsk;
		global_request_work_t* back_work = NULL;

		// Spin up bottom half to process this event
		back_work = (global_request_work_t*) kmalloc(sizeof(global_request_work_t),
				GFP_ATOMIC);
		if (back_work) {
			INIT_WORK((struct work_struct* )back_work, global_worher_fn);
			back_work->_grq_head = &gvp->_grq_head;//, sizeof(struct plist_head));
			back_work->lock = &gvp->lock; // , sizeof(spinlock_t));
			back_work->ops = -1;
			queue_work(grq, (struct work_struct*) back_work);
		}
		gvp->free =1;

	} else {
		//TODO: check if it is the same thread group leader if not hash it to another bucket.
	}
	GENERAL_SPIN_UNLOCK(&access_global_value_table);

	//Check whether the request is asking for ticket or holding the ticket?
	if (!msg->ticket) {
		//if not holding the ticket add to the tail of Global request queue.
		_global_rq *trq = (_global_rq *) kmalloc(sizeof(_global_rq), GFP_ATOMIC);
		//if not holding the ticket add to the tail of Global request queue.
		memcpy(&trq->wait, msg, sizeof(_remote_key_request_t));
		trq->ops = 0;
		plist_node_init(&trq->list, NORMAL_Q_PRIORITY);
		plist_add(&trq->list, &gvp->_grq_head);

	} else if(msg->ticket == 1) {
		_global_rq *trq = (_global_rq *) kmalloc(sizeof(_global_rq), GFP_ATOMIC);
		//if not holding the ticket add to the tail of Global request queue.
		memcpy(&trq->wait, msg, sizeof(_remote_key_request_t));
		trq->ops = 0;
		plist_node_init(&trq->list, HIGH_Q_PRIORITY);
		plist_add(&trq->list, &gvp->_grq_head);
	}else if(msg->ticket == 2 ) {
		_global_rq *trq = (_global_rq *) kmalloc(sizeof(_global_rq), GFP_ATOMIC);
		//if not holding the ticket add to the tail of Global request queue.
		memcpy(&trq->wait, msg, sizeof(_remote_key_request_t));
		trq->ops = 0;
		plist_node_init(&trq->list, HIGH1_Q_PRIORITY);
		plist_add(&trq->list, &gvp->_grq_head);
	}

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

int get_set_remote_key(u32 __user *uaddr, unsigned int val, int fshared,
		union futex_key *key, int rw, unsigned int fn_flag, u32 bitset) {

	int res = 0;
	int cpu = 0;
	struct page *page, *page_head;
	_remote_key_request_t *request = kmalloc(sizeof(_remote_key_request_t),
			GFP_ATOMIC);
	FRPRINTK(KERN_ALERT"%s: -- entered whos calling{%s} \n", __func__,current->comm);
	// Build request
	request->header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;

	// Send response
	int x = 0;
	get_user(x, uaddr);

	struct vm_area_struct *vma;
	vma = getVMAfromUaddr(uaddr);

	request->flags = fshared;
	request->uaddr = (unsigned long) uaddr;
	request->rw = rw;
	request->pid = current->pid;
	request->origin_pid = current->origin_pid;
	request->val = val;
	request->tghid = current->tgroup_home_id;
	request->fn_flags = fn_flag;
	request->bitset = bitset;

	request->ops = 0;
	request->ticket = 1;

	if (vma->vm_flags & VM_PFNMAP) {

		FRPRINTK(KERN_ALERT" %s:pfn {%lx} shift {%lx} vm_start {%lx} vm_end {%lx}\n ",__func__,vma->vm_pgoff,vma->vm_pgoff << PAGE_SHIFT,vma->vm_start,vma->vm_end);

		unsigned long pfn;
		res = -ENOTINKRN;
		pte_t pte;
		pte = *((pte_t *) do_page_walk((unsigned long) uaddr));
		pfn = pte_pfn(pte);
		FRPRINTK(KERN_ALERT"%s: pte ptr : ox{%lx} pfn: 0x{%lx} cpu{%d}\n",__func__,pte,pfn,smp_processor_id());

exit:
		if ((cpu = find_kernel_for_pfn(pfn, &pfn_list_head)) != -1) //vma->vm_pgoff << PAGE_SHIFT
		{
			FRPRINTK(KERN_ALERT"%s: sending to origin futex pfn cpu: 0x{%d}\n",__func__,cpu);
			res = pcn_kmsg_send(cpu, (struct pcn_kmsg_message*) (request));
		}

	} else if (fn_flag & FLAGS_ORIGINCALL) {
		res = pcn_kmsg_send(ORIG_NODE(current->pid),(struct pcn_kmsg_message*) (request));
	}

out:
	kfree(request);
	return res;
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

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_TOKEN_REQUEST,
				handle_remote_futex_token_request);

	grq   = create_singlethread_workqueue(MODULE);

	INIT_LIST_HEAD(&vm_head);


	return 0;
}
__initcall(futex_remote_init);

/*mklinux_akshay*/
