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



#define FUTEX_REMOTE_VERBOSE 0
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

extern struct list_head pfn_list_head;

extern int unqueue_me(struct futex_q *q);
#define  NSIG 32

static struct list_head vm_head;

struct list_head fq_head;

extern int __kill_something_info(int sig, struct siginfo *info, pid_t pid);

static void dump_regs(struct pt_regs* regs) {
    unsigned long fs, gs; 
   FRPRINTK(KERN_ALERT"DUMP REGS\n");
    if(NULL != regs) {
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
	/*unsigned long start;
	unsigned long end;*/
	unsigned long rflag;
	int pid;
	int origin_pid;
	struct list_head list_member;
};

typedef struct _inc_remote_vm_pool _inc_remote_vm_pool_t;

_inc_remote_vm_pool_t * add_inc(int pid,int start,int end, int rflag,int origin_pid, struct list_head *head) {
	_inc_remote_vm_pool_t *Ptr = (_inc_remote_vm_pool_t *) kmalloc(
			sizeof(_inc_remote_vm_pool_t), GFP_ATOMIC);

	//Ptr->start = start;
	Ptr->pid = pid;
	//Ptr->end = end;
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

struct pid_list
{
	int pid;
	struct list_head pid_list_member;
};

/*
struct kernel_robust_list_head * add_key(int pid,int address, struct kernel_robust_list *head) {


	struct kernel_robust_list_head  *Ptr;
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
*/
struct _remote_wakeup_request {
	struct pcn_kmsg_hdr header;
	unsigned long uaddr;
	unsigned int flags;
	int nr_wake;
	u32 bitset;
	int tgid;
	int tghid;
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


struct futex_q ** query_q_pid(struct task_struct *t,int kernel) {
	int i = 0,cnt=0;
	struct futex_hash_bucket *hb;
	struct plist_head *head;
	struct futex_q *this, *next=NULL;
	struct futex_q **res=NULL;
	res =  (struct futex_q **)kmalloc(sizeof(res)*10,GFP_ATOMIC);

	if(!res)
		FRPRINTK(KERN_ALERT "unable to kmmaloc\n");

	for(i=0;i<10;i++)
		res[cnt] = (struct futex_q *)kmalloc(sizeof(struct futex_q),GFP_ATOMIC);

	int end = 1 << _FUTEX_HASHBITS;
	struct task_struct *temp;
	for (i = 0; i < end; i++) {
		hb = &futex_queues[i];
		if (hb != NULL) {
			spin_lock(&hb->lock);
			head = &hb->chain;
			plist_for_each_entry_safe(this, next, head, list)
			{

				temp=this->task;
				if(temp /*&& is_kernel_addr(temp)*/ && !kernel){
				get_task_struct(temp);
				if(temp->tgroup_distributed == 1 && (temp->tgroup_home_id == t->tgroup_home_id || temp->pid == t->pid)){
					res[cnt]=this;
					cnt++;
				}
			    put_task_struct(temp);
				}
				else if(this && !temp && kernel)
				{
					if(this->rem_pid > 1){
					FRPRINTK(KERN_ALERT "%s: rem_pid{%d} \n","query_q_pid",this->rem_pid);
					res[cnt]=this;
					cnt++;
					}
				}
			}
			spin_unlock(&hb->lock);
		}
	}
q_exit:
    return res;

}


static int latest_pid=0;


int del_futex(struct futex_q * queue) {
	int ret=0;
	unsigned int flags=0;
	if (queue->rem_pid == -1)
		wake_futex(queue);
	else {
		if (latest_pid != queue->rem_pid) {
			flags=~FLAGS_SHARED | ~FLAGS_DESTROY;
			FRPRINTK(KERN_ALERT " del_futex: calling global futex wake uaddr{%lx} rem_id{%d} latest_pid{%d} flag{%u}\n",queue->key.both.offset+queue->key.private.address,queue->rem_pid,latest_pid,flags);
			ret=remote_futex_wakeup(queue->key.both.offset + queue->key.private.address,
				flags, 1, 1, &queue->key, queue->rem_pid);

			latest_pid = queue->rem_pid;
			queue->rem_pid = NULL;
			if (queue->lock_ptr != NULL && spin_is_locked(queue->lock_ptr)) {
				FRPRINTK(KERN_ALERT"Unlocking spinlock \n");
				spin_unlock(&queue->lock_ptr);
			}
			__unqueue_futex(queue);
			smp_wmb();
			queue->lock_ptr = NULL;
		}
	}
	return ret;
}


int query_q_and_wake(struct task_struct *t,int kernel) {
	int i = 0,cnt=0;
	struct futex_hash_bucket *hb;
	struct plist_head *head;
	struct futex_q *this, *next=NULL;
	int res=0;

	int end = 1 << _FUTEX_HASHBITS;
	struct task_struct *temp;
	for (i = 0; i < end; i++) {
		hb = &futex_queues[i];
		if (hb != NULL) {
			spin_lock(&hb->lock);
			head = &hb->chain;
			plist_for_each_entry_safe(this, next, head, list)
			{

				temp=this->task;
				if(temp /*&& is_kernel_addr(temp)*/ && !kernel){
				get_task_struct(temp);
				if(temp->tgroup_distributed == 1 && (temp->tgroup_home_id == t->tgroup_home_id || temp->pid == t->pid)){
					res=del_futex(this);
					cnt++;
				}
			    put_task_struct(temp);
				}
				else if(this && !temp && kernel)
				{
					if(this->rem_pid > 1){
					FRPRINTK(KERN_ALERT "%s: rem_pid{%d} \n","query_q_pid",this->rem_pid);
					res=del_futex(this);
					cnt++;
					}
				}
			}
			spin_unlock(&hb->lock);
		}
	}
q_exit:
    return res;

}


void wake_futex_global(struct futex_q *q)
{
	struct task_struct *p = q->task;

			get_task_struct(p);

					unqueue_me(q);

							smp_wmb();
									q->lock_ptr = NULL;

											wake_up_state(p, TASK_NORMAL);
													put_task_struct(p);
}

int global_futex_wake(u32 __user *uaddr, unsigned int flags, int nr_wake, u32 bitset, pid_t pid)
{
	struct futex_hash_bucket *hb;
	struct futex_q *this, *next;
	struct plist_head *head;
	struct task_struct *tsk;
	union futex_key key = FUTEX_KEY_INIT;
	struct mm_struct  *cmm = NULL;
	struct task_struct *temp;
	int ret;
	FRPRINTK(KERN_ALERT "%s: response {%d} uaddr{%lx} comm{%s} flags{%u} \n","global_futex_wake",pid,uaddr,current->comm,flags);
	if (!bitset)
		return -EINVAL;

	tsk=pid_task(find_vpid(pid), PIDTYPE_PID);
//	FRPRINTK(KERN_ALERT" task exists {%d}  tsk mm{%p} cmm{%p} \n",(tsk!=NULL)?1:0,tsk->mm,current->mm);
	if(tsk)
	{
		cmm = current->mm;
		current->mm=tsk->mm;
	}

	ret = get_futex_key(uaddr, ((flags & FLAGS_DESTROY == 256)? (0 & FLAGS_SHARED) : (flags & FLAGS_SHARED)), &key, VERIFY_READ);


	FRPRINTK(KERN_ALERT "global_futex_wake ptr {%p} mm{%p} \n",key.both.ptr,current->mm);

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
					FRPRINTK(KERN_ALERT"Hold locking spinlock \n");
					//spin_unlock(&this->lock_ptr);
				}
				FRPRINTK(KERN_ALERT" call wake futex \n");
				/*if (flags&FLAGS_DESTROY == 256) {
					temp->return_disposition=2;//RETURN_DISPOSITION_FORCE_KILL
				}*/
				wake_futex(this);
			}
		}

	}
	spin_unlock(&hb->lock);
	put_futex_key(&key);
out:

	if(cmm==NULL)
	{
		FRPRINTK(KERN_ALERT" cmm NULL\n");
		current->mm= cmm;
	}
	else
	{
		FRPRINTK(KERN_ALERT" current {%s}\n",current->comm);
				current->mm= NULL;
	}

	FRPRINTK(KERN_ALERT "exit global_futex_wake \n");

	return ret;
}

static int handle_remote_futex_wake_response(struct pcn_kmsg_message* inc_msg) {
	_remote_wakeup_response_t* msg = (_remote_wakeup_response_t*) inc_msg;

	FRPRINTK(KERN_ALERT"%s: response {%d} \n",
			"handle_remote_futex_wake_response", msg->errno);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

static int handle_remote_futex_wake_request(struct pcn_kmsg_message* inc_msg) {

	_remote_wakeup_request_t* msg = (_remote_wakeup_request_t*) inc_msg;
	_remote_wakeup_response_t response;
	struct task_struct *tsk=current;
	struct task_struct *task, *g;
	struct mm_struct  *cmm = NULL;
	int null_flag=0;

	FRPRINTK(KERN_ALERT"%s: request -- entered task comm{%s} pid{%d}\n", "handle_remote_futex_wake_request",tsk->comm,tsk->pid);
	FRPRINTK(KERN_ALERT"%s: msg: uaddr {%lx} tgid {%d} tghid{%d} bitset {%u} rflag{%d} pid{%d} origin_pid {%d} \n",
			"handle_remote_futex_wake_request",msg->uaddr,msg->tgid,msg->tghid,msg->bitset,msg->rflag,msg->pid,msg->origin_pid);

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	//add_inc( msg->pid,msg->start,msg->end,msg->rflag,msg->origin_pid, &vm_head);

	if(msg->rflag == 0)
	{
		if(current->mm!=NULL)
		{
			null_flag=1;
			cmm = (current->mm);
			//FRPRINTK(KERN_ALERT "current mm not null comm{%s} cmm{%p}\n",current->comm,current->mm);
		}

		tsk = pid_task(find_vpid(msg->origin_pid), PIDTYPE_PID);
		if(tsk)
		{
		FRPRINTK(KERN_ALERT "origin id exists tsk pid{%d}\n", tsk->pid);
		current->mm = tsk->mm;
		goto mm_exit;
		}
		else
		{
			do_each_thread(g,task) {
				if(task->pid == msg->origin_pid)
				{
					current->mm = task->mm;
					FRPRINTK(KERN_ALERT "origin -> mm struct found comm{%s} cmm{%d} mm{%d} \n",task->comm,(cmm!=NULL)?1:0, (current->mm!=NULL)?1:0);
					goto mm_exit;
				}
			}while_each_thread(g,task);
		}

		tsk = pid_task(find_vpid(msg->tgid), PIDTYPE_PID);
		if(tsk)
		{
		FRPRINTK(KERN_ALERT "t_home id exists tsk pid {%d} \n",tsk->pid);
		current->mm = tsk->mm;
		goto mm_exit;
		}
		else
		{
			do_each_thread(g,task) {
				if(task->pid == msg->tgid)
				{
					current->mm = task->mm;
					FRPRINTK(KERN_ALERT "t_jome-> mm struct found comm{%s} cmm{%d} mm{%d} \n",task->comm,(cmm!=NULL)?1:0, (current->mm!=NULL)?1:0);
					goto mm_exit;
				}
			}while_each_thread(g,task);
		}
		tsk = pid_task(find_vpid(msg->tghid), PIDTYPE_PID);
		if(tsk)
		{
			current->mm = tsk->mm;
			FRPRINTK(KERN_ALERT "tghid exist cmm{%d}  cmm{%p} comm{%s} mm{%p}\n",(cmm!=NULL)?1:0,cmm,current->comm,current->mm);
		}
		else //if(current->mm==NULL)
		{
			//FRPRINTK(KERN_ALERT "mm struct doesnt exist\n");
			do_each_thread(g,task) {
				if(task->pid == msg->tghid)
				{
					current->mm = task->mm;
					FRPRINTK(KERN_ALERT "tghid-> mm struct found comm{%s} cmm{%d} mm{%d} \n",task->comm,(cmm!=NULL)?1:0, (current->mm!=NULL)?1:0);
					goto mm_exit;
				}
			} while_each_thread(g,task);
		}
mm_exit:
	FRPRINTK(KERN_ALERT "before wake cmm{%d}  mm{%d} cmm{%p}  mm{%p}\n",(cmm!=NULL)?1:0,(current->mm!=NULL)?1:0,cmm,current->mm);

	  futex_wake(msg->uaddr,msg->flags,msg->nr_wake,msg->bitset);
	  if(cmm !=NULL && null_flag)
	  {
		 FRPRINTK(KERN_ALERT "assign the original mm struct back for task {%d}\n",current->pid);
	     current->mm = cmm;
	  }
	  else if(cmm == NULL && !null_flag)
	  {
	  		 FRPRINTK(KERN_ALERT "assign the null mm struct back {%d} \n",current->pid);
	  	     current->mm = NULL;
	  }
	  else
	  {
		  FRPRINTK(KERN_ALERT "whatever {%d} \n",current->pid);
		  current->mm = NULL;
	  }
	}
	else
	 global_futex_wake(msg->uaddr,msg->flags,msg->nr_wake,msg->bitset, msg->rflag);


	FRPRINTK(KERN_ALERT " handle_remote_futex_wake_req after setting mm to NULL\n");
	pcn_kmsg_free_msg(inc_msg);

	return 0;
}


int remote_futex_wakeup(u32 __user  *uaddr,unsigned int flags, int nr_wake, u32 bitset,union futex_key *key, int rflag ) {

	int res = 0;
	int cpu=0;
	struct page *page, *page_head;
	_remote_wakeup_request_t *request = kmalloc(sizeof(_remote_wakeup_request_t),
	GFP_ATOMIC);

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

	request->tgid = current->t_home_id;
	request->tghid = current->tgroup_home_id;
	request->rflag = rflag;
	request->pid = current->pid;
	request->origin_pid = current->origin_pid;

	int x=0,y=0;
	int wake=0, woke=0, nw=0,bs=0;
	
/*	get_user(x,uaddr);
	get_user(y,uaddr-1);
	
	if(uaddr==0x602180){
	get_user(wake,uaddr+4);
	get_user(woke,uaddr+6);
	get_user(nw,uaddr+10);
	get_user(bs,uaddr+11);}
*/
	FRPRINTK(KERN_ALERT" remote_futex_wakeup pfn {%lx} shift {%lx} pid{%d} origin_pid{%d} cpu{%d} rflag{%d} uaddr{%lx} vma start (%lx} vma end (%lx} get_user{%d}\n ",
								vma->vm_pgoff,vma->vm_pgoff << PAGE_SHIFT,current->pid,current->origin_pid,smp_processor_id(),rflag,uaddr,vma->vm_start, vma->vm_end,x);

//	dump_regs(task_pt_regs(current));

			if(vma->vm_flags & VM_PFNMAP)	{

		    // Send response
			if(!rflag)
			{
					res=-ENOTINKRN;
					unsigned long pfn;
					res=-ENOTINKRN;

					pgd_t *pgd = NULL;
					pud_t *pud = NULL;
					pmd_t *pmd = NULL;
					pte_t *ptep = NULL;
					pte_t pte;

					pgd = pgd_offset(current->mm, address);
					if(!pgd_present(*pgd)) {
						goto exit;
					}

					pud = pud_offset(pgd,address);
					if(!pud_present(*pud)) {
						goto exit;
					}

					pmd = pmd_offset(pud,address);
					if(!pmd_present(*pmd)) {
						goto exit;
					}

					ptep = pte_offset_map(pmd,address);
					if(!ptep || !pte_present(*ptep)) {
						goto exit;
					}
					pte = *ptep;
					FRPRINTK(KERN_ALERT"remote futex wake pte ptr : ox{%lx} cpu{%d} val{%d} wake{%d} woke{%d} nw{%d} bs{%d}\n",pte,smp_processor_id(),x,wake,woke,nw,bs);
					pfn=pte_pfn(pte);
					FRPRINTK(KERN_ALERT"remote futex wake pte pfn : 0x{%lx}\n",pfn);

exit:

					if((cpu=find_kernel_for_pfn(pfn,&pfn_list_head)) != -1)//vma->vm_pgoff << PAGE_SHIFT
					{	FRPRINTK(KERN_ALERT"remote futex wake pfn cpu: 0x{%d}\n",cpu);
					   res = pcn_kmsg_send(cpu, (struct pcn_kmsg_message*) (request));
					}
				//}


			}

			}

			else if(rflag)
			{
				FRPRINTK(KERN_ALERT "remote_futex_wakeup: remote node {%d}\n", ORIG_NODE(rflag));
			    res = pcn_kmsg_send(ORIG_NODE(rflag), (struct pcn_kmsg_message*) (request));
			}

	return res;

}




int global_futex_wait(u32 __user *uaddr, unsigned int flags, u32 val,
		      ktime_t *abs_time, u32 bitset, pid_t rem, struct task_struct *origin)
{
	struct futex_hash_bucket *hb;
	struct task_struct *tsk = origin;
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
    FRPRINTK(KERN_ALERT "global_futex_wait pid origin {%s} _cpu{%d} uaddr{%lx} disp{%d} \n ",tsk->comm,smp_processor_id(),uaddr, current->return_disposition);

    if(tsk)
       	q->key.private.mm=tsk->mm;

    hb = hash_futex(&q->key);
    q->lock_ptr = &hb->lock;
    spin_lock(&hb->lock);
     	int prio;
    	prio = 100 ;//min(current->normal_prio, MAX_RT_PRIO);
    	q->task = NULL;
    	q->rem_pid = rem;
    	plist_node_init(&q->list, prio);
    	plist_add(&q->list, &hb->chain);
    spin_unlock(&hb->lock);
out:
    FRPRINTK(KERN_ALERT "global_futex_wait: hb {%p} key: word {%lx} offset{%d} ptr{%p} mm{%p}\n ",
		        		hb,q->key.both.word,q->key.both.offset,q->key.both.ptr,q->key.private.mm);
    	FRPRINTK(KERN_ALERT "exit glabal_futex_wait \n");
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
	int tghid;
	char pad_string[28];
}__attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_key_request _remote_key_request_t;

struct _remote_key_response {
	struct pcn_kmsg_hdr header;
	unsigned long uaddr;
	unsigned int flags;
	int rw;
	int pid;
	int origin_pid;
	int val;
	int tghid;
	char pad_string[52];
}__attribute__((packed)) __attribute__((aligned(64)));


typedef struct _remote_key_response _remote_key_response_t;

static int handle_remote_futex_key_response(struct pcn_kmsg_message* inc_msg) {
	_remote_key_response_t* msg = (_remote_key_response_t*) inc_msg;

	FRPRINTK(KERN_ALERT"%s: response to revoke wait request as origin is dead {%d} \n",
			"handle_remote_futex_key_response",msg->origin_pid);

	global_futex_wake(msg->uaddr,msg->flags,msg->val,1, msg->pid);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

struct task_struct*  gettask(pid_t origin_pid, pid_t tghid)
{
struct task_struct *tsk=NULL;
struct task_struct *g,*task=NULL;

tsk = pid_task(find_vpid(origin_pid), PIDTYPE_PID);
	if(tsk)
	{
	FRPRINTK(KERN_ALERT "origin id exists \n");
	}
	else
	{
		do_each_thread(g,task) {
			if(task->pid == origin_pid)
			{
				tsk = task;
				goto mm_exit;
			}
		}while_each_thread(g,task);
	}
tsk = pid_task(find_vpid(tghid), PIDTYPE_PID);
	if(tsk)
	{
	FRPRINTK(KERN_ALERT "tghid id exists \n");
	}
	else
	{
		do_each_thread(g,task) {
			if(task->pid == tghid)
			{
				tsk = task;
				goto mm_exit;
			}
		}while_each_thread(g,task);
	}
mm_exit:
	return tsk;
}
static int handle_remote_futex_key_request(struct pcn_kmsg_message* inc_msg) {

	_remote_key_request_t* msg = (_remote_key_request_t*) inc_msg;
	_remote_key_response_t response;
	struct task_struct *tsk;
	int res;

	FRPRINTK(KERN_ALERT"%s: request -- entered \n", "handle_remote_futex_key_request");
	FRPRINTK(KERN_ALERT"%s: msg: uaddr {%lx} flags {%x} val{%d}  pid{%d} origin_pid {%d} \n",
				"handle_remote_futex_key_request",msg->uaddr,msg->flags,msg->val,msg->pid,msg->origin_pid);

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;
	tsk=gettask(msg->origin_pid,msg->tghid);
	if(!tsk)
	{
		response.uaddr=msg->uaddr;
		response.pid=msg->pid;
		response.origin_pid=msg->origin_pid;
		response.flags=msg->flags;
		response.rw=msg->rw;
		response.tghid=msg->tghid;
		response.val=msg->val;
		res = pcn_kmsg_send(msg->header.from_cpu, (struct pcn_kmsg_message*) (&response));
	}
	else
	global_futex_wait(msg->uaddr,msg->flags,msg->val,0,0,msg->pid,tsk);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}


int
get_set_remote_key(u32 __user *uaddr, unsigned int val, int fshared, union futex_key *key, int rw)
{

	int res = 0;
	int cpu=0;
	struct page *page, *page_head;
	_remote_key_request_t *request = kmalloc(sizeof(_remote_key_request_t),
			GFP_ATOMIC);

	// Build request
	request->header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;

	// Send response
	int x=0;
	 get_user(x,uaddr);

	/* if(current->return_disposition==2)
	 {
		 put_user(0,uaddr);
		 do_exit(0);
	 }*/

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
		request->tghid = current->tgroup_home_id;

		FRPRINTK(KERN_ALERT" pfn {%lx} shift {%lx} vm_start {%lx} vm_end {%lx}\n ",vma->vm_pgoff,vma->vm_pgoff << PAGE_SHIFT,vma->vm_start,vma->vm_end);

			unsigned long pfn;
			res=-ENOTINKRN;

			pgd_t *pgd = NULL;
			pud_t *pud = NULL;
			pmd_t *pmd = NULL;
			pte_t *ptep = NULL;
			pte_t pte;

			pgd = pgd_offset(current->mm, address);
			if(!pgd_present(*pgd)) {
				goto exit;
			}

			pud = pud_offset(pgd,address);
			if(!pud_present(*pud)) {
				goto exit;
			}

			pmd = pmd_offset(pud,address);
			if(!pmd_present(*pmd)) {
				goto exit;
			}

			ptep = pte_offset_map(pmd,address);
			if(!ptep || !pte_present(*ptep)) {
				goto exit;
			}
			pte = *ptep;
			FRPRINTK(KERN_ALERT"futex wait pte ptr : 0x{%lx} cpu{%d}, get_user(%d) \n",pte,smp_processor_id(),x);
			pfn=pte_pfn(pte);
			FRPRINTK(KERN_ALERT"futex wait pte pfn : 0x{%lx}\n",pfn);

exit:
			if((cpu=find_kernel_for_pfn(pfn,&pfn_list_head)) != -1)//vma->vm_pgoff << PAGE_SHIFT
			{	FRPRINTK(KERN_ALERT"futex wait futex pfn cpu: 0x{%d}\n",cpu);
			  res = pcn_kmsg_send(cpu, (struct pcn_kmsg_message*) (request));
			}


		//}
	}
	return res;
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
	 FRPRINTK(KERN_ALERT"futex pfn : 0x{%lx}\n",PFN_PHYS(pfn));

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



	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_REQUEST,
			handle_remote_futex_key_request);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_RESPONSE,
			handle_remote_futex_key_response);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_REQUEST,
			handle_remote_futex_wake_request);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_RESPONSE,
			handle_remote_futex_wake_response);

	INIT_LIST_HEAD(&vm_head);
	INIT_LIST_HEAD(&fq_head);


	return 0;
}
__initcall(futex_remote_init);



/*mklinux_akshay*/
