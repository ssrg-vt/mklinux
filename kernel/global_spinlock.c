/*
 * global_spinlock.c
 *
 *  Created on: 4/7/2014
 *      Author: akshay
 */


union futex_key; //forward decl for futex_remote.h


#include <linux/jhash.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <popcorn/global_spinlock.h>

#include <linux/pcn_kmsg.h>
#include<popcorn/init.h>
#define FLAGS_SYSCALL		8
#define FLAGS_REMOTECALL	16
#define FLAGS_ORIGINCALL	32

#define WAKE_OPS 1
#define WAIT_OPS 0

DEFINE_SPINLOCK(request_queue_lock);

//hash buckets
 _spin_value spin_bucket[1<<_SPIN_HASHBITS];
 _global_value global_bucket[1<<_SPIN_HASHBITS];

#define GENERAL_SPIN_LOCK(x,f) spin_lock_irqsave(x,f)
#define GENERAL_SPIN_UNLOCK(x,f) spin_unlock_irqrestore(x,f)

#define GSP_VERBOSE 0 
#if GSP_VERBOSE
#define GSPRINTK(...) printk(__VA_ARGS__)
#else
#define GSPRINTK(...) ;
#endif
//extern functions
static int _cpu =0;
 extern struct vm_area_struct * getVMAfromUaddr(unsigned long uaddr);
 extern pte_t *do_page_walk(unsigned long address);
 extern  int find_kernel_for_pfn(unsigned long addr, struct list_head *head);
 extern int getFutexOwnerFromPage(unsigned long uaddr);

 _local_rq_t * add_request_node(int request_id, pid_t pid, struct list_head *head) {

	 unsigned long f;
	 GENERAL_SPIN_LOCK(&request_queue_lock,f);
	 _local_rq_t *Ptr = (_local_rq_t *) kmalloc(
 			sizeof(_local_rq_t), GFP_ATOMIC);

 	memset(Ptr, 0, sizeof(_local_rq_t));
 	Ptr->_request_id = request_id;
 	Ptr->status = IDLE;
 	Ptr->wake_st = 0;
 	Ptr->_pid = pid;
 	init_waitqueue_head(&Ptr->_wq);
 	INIT_LIST_HEAD(&Ptr->lrq_member);
 	list_add(&Ptr->lrq_member, head);
 	 GENERAL_SPIN_UNLOCK(&request_queue_lock,f);

 	return Ptr;
 }

 int find_and_delete_request(int request_id, struct list_head *head) {

 	struct list_head *iter;
 	_local_rq_t *objPtr;
	unsigned long f;
 	 GENERAL_SPIN_LOCK(&request_queue_lock,f);
 	list_for_each(iter, head)
 	{
 		objPtr = list_entry(iter, _local_rq_t, lrq_member);
 		if (objPtr->_request_id == request_id) {
 			list_del(&objPtr->lrq_member);
 			kfree(objPtr);
 			 GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
 			return 1;
 		}
 	}
 	 GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
 }

int find_and_delete_pid(int pid, struct list_head *head) {

 	struct list_head *iter;
 	_local_rq_t *objPtr;
	unsigned long f;
 	 GENERAL_SPIN_LOCK(&request_queue_lock,f);
 	list_for_each(iter, head)
 	{
 		objPtr = list_entry(iter, _local_rq_t, lrq_member);
 		if (objPtr->_pid == pid) {
 			list_del(&objPtr->lrq_member);
 			kfree(objPtr);
 			 GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
 			return 1;
 		}
 	}
 	 GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
 }

 _local_rq_t * find_request(int request_id, struct list_head *head) {

 	struct list_head *iter;
 	_local_rq_t *objPtr;
	unsigned long f;
 	 GENERAL_SPIN_LOCK(&request_queue_lock,f);
 	list_for_each(iter, head)
 	{
 		objPtr = list_entry(iter, _local_rq_t, lrq_member);
 		if (objPtr->_request_id == request_id) {
 			GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
 			return objPtr;
 		}
 	}
 	 GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
 	return NULL;
 }

 _local_rq_t * set_err_request(int request_id, int err, struct list_head *head) {

struct list_head *iter;
_local_rq_t *objPtr;
unsigned long f;
	 GENERAL_SPIN_LOCK(&request_queue_lock,f);
  	list_for_each(iter, head)
 	{
		objPtr = list_entry(iter, _local_rq_t, lrq_member);
		if (objPtr->_request_id == request_id) {
			objPtr->status =DONE;
			objPtr->errno = err;
			wake_up_interruptible(&objPtr->_wq);
 			GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
 			return objPtr;
 		}
 	}
	 GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
 	return NULL;
}

 _local_rq_t *find_request_by_pid(pid_t pid, struct list_head *head) {

 	struct list_head *iter;
 	_local_rq_t *objPtr;
	unsigned long f;
 	 GENERAL_SPIN_LOCK(&request_queue_lock,f);
 	list_for_each(iter, head)
 	{
 		objPtr = list_entry(iter, _local_rq_t, lrq_member);
 		if (objPtr->_pid == pid && objPtr->ops == 0) {
 			GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
 			return objPtr;
 		}
 	}
 	 GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
 	return NULL;
 }

 _local_rq_t *find_request_by_ops(int ops, unsigned long uaddr,pid_t pid, struct list_head *head) {

        struct list_head *iter;
        _local_rq_t *objPtr;
        unsigned long f;
         GENERAL_SPIN_LOCK(&request_queue_lock,f);
        list_for_each(iter, head)
        {
                objPtr = list_entry(iter, _local_rq_t, lrq_member);
                if (objPtr->ops == 0 && objPtr->uaddr == uaddr && objPtr->_pid == pid) {
                        GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
                        return objPtr;
                }
        }
         GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
        return NULL;
 }

 _local_rq_t *set_wake_request_by_pid(pid_t pid, struct list_head *head) {

 	struct list_head *iter;
 	_local_rq_t *objPtr;
	unsigned long f;
 	 GENERAL_SPIN_LOCK(&request_queue_lock,f);
 	list_for_each(iter, head)
 	{
 		objPtr = list_entry(iter, _local_rq_t, lrq_member);
 		if (objPtr->_pid == pid) {
		///	printk(KERN_ALERT"wake stat up\n");
			objPtr->wake_st =1; //Set wake state as ON
 			GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
 			return objPtr;
 		}
 	}
 	 GENERAL_SPIN_UNLOCK(&request_queue_lock,f);
 	return NULL;
 }



 inline void spin_key_init (struct spin_key *st) {
  	st->_tgid = 0;
  	st->_uaddr = 0;
  	st->offset = 0;
  }


 //Populate spin key from uaddr
int getKey(unsigned long uaddr, _spin_key *sk, pid_t tgid)
{
	unsigned long address = (unsigned long)uaddr;
	sk->offset = address;
	sk->_uaddr  = uaddr;
	sk->_tgid	= tgid;

	return 0;
}


// hash spin key to find the spin bucket
_spin_value *hashspinkey(_spin_key *sk)
{
	pagefault_disable();
	u32 hash = sp_hashfn(sk->_uaddr,sk->_tgid);
	pagefault_enable();
	return &spin_bucket[hash];
}

//to get the global worker and global request queue
_global_value *hashgroup(struct task_struct *group_pid)
{
	struct task_struct *tsk =NULL;
	tsk= group_pid;
//	pagefault_disable();
	u32 hash = sp_hashfn(tsk->pid,0);
//	pagefault_enable();
	return &global_bucket[hash];
}

// Perform global spin lock
int global_spinlock(unsigned long uaddr,futex_common_data_t *_data,_spin_value * value,_local_rq_t *rq_ptr,int localticket_value,int cpu)
__releases(&value->_sp)
{
	int res = 0;
	unsigned int flgs;

	printk("%s\n", __func__);
	dump_stack();

	 _remote_key_request_t* wait_req= (_remote_key_request_t*) kmalloc(sizeof(_remote_key_request_t),
				GFP_ATOMIC);
	 _remote_wakeup_request_t *wake_req = (_remote_wakeup_request_t*) kmalloc(sizeof(_remote_wakeup_request_t),
				GFP_ATOMIC);

	//Prepare request
	if(_data->ops==WAIT_OPS){

//	printk(KERN_ALERT"%s: request -- entered whos calling{%s} \n", __func__,current->comm);
	GSPRINTK(KERN_ALERT"%s:  uaddr {%lx}  pid{%d} current->tgroup_home_id{%d}\n",				__func__,uaddr,current->pid,current->tgroup_home_id);

	// Finish constructing response
	wait_req->header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_REQUEST;
	wait_req->header.prio = PCN_KMSG_PRIO_NORMAL;

	wait_req->ops = WAIT_OPS;
	wait_req->rw = _data->rw;
	wait_req->val = _data->val;

	wait_req->uaddr = (unsigned long) uaddr;
	wait_req->tghid = current->tgroup_home_id;
	wait_req->bitset = _data->bitset;
	wait_req->pid = current->pid;
	wait_req->fn_flags = _data->fn_flag;
	wait_req->flags = _data->flags;

	wait_req->ticket = localticket_value;//GET_TOKEN; //set the request has no ticket
	}
	else{
		wake_req->header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_WAKE_REQUEST;
		wake_req->header.prio = PCN_KMSG_PRIO_NORMAL;

		wake_req->ops = WAKE_OPS;
		wake_req->uaddr2 = (unsigned long) _data->uaddr2;
		wake_req->nr_wake2 = _data->nr_requeue;
		wake_req->cmpval = _data->cmpval;
		wake_req->rflag = _data->rflag;
		wake_req->nr_wake =_data->nr_wake;

		wake_req->uaddr = (unsigned long) uaddr;
		wake_req->tghid = current->tgroup_home_id;
		wake_req->bitset = _data->bitset;
		wake_req->pid = current->pid;
		wake_req->fn_flag = _data->fn_flag;
		wake_req->flags = _data->flags;

		wake_req->ticket = localticket_value;//GET_TOKEN; //set the request has no ticket
		GSPRINTK(KERN_ALERT"%s: wake uaddr2{%lx} data{%lx} \n",__func__,wake_req->uaddr2,_data->uaddr2);
	}

    //cpu = getFutexOwnerFromPage(uaddr);

    //if(cpu < 0)
    //	return cpu; //return ERROR

	struct vm_area_struct *vma;
	vma = getVMAfromUaddr(uaddr);

	if (vma != NULL && _cpu != 0){// && current->executing_for_remote && ((vma->vm_flags & VM_PFNMAP) || (vma->vm_flags & VM_MIXEDMAP))) {

				if(_data->ops==WAIT_OPS){
					wait_req->fn_flags |= FLAGS_REMOTECALL;
			
		//		 printk(KERN_ALERT"%s: msg wait: uaddr {%lx}  ticket {%d} tghid{%d} bitset {%u}  pid{%d}  ops{%d} size{%d} \n",
                 //       __func__,wait_req->uaddr,wait_req->ticket,wait_req->tghid,wait_req->bitset,wait_req->pid,_data->ops,sizeof(_remote_key_request_t));
				}
				else{
					wake_req->fn_flag |= FLAGS_REMOTECALL;
				//printk(KERN_ALERT"%s: uaddr{%lx}  uaddr2{%lx}\n",__func__,wake_req->uaddr,wake_req->uaddr2);
		//		 printk(KERN_ALERT"%s: msg wake: uaddr {%lx}  uaddr2{%lx} ticket {%d} tghid{%d} bitset {%u} rflag{%d} pid{%d} ifn_flags{%lx} ops{%d} size{%d} \n",
                  //      __func__,wake_req->uaddr,(wake_req->uaddr2),wake_req->ticket,wake_req->tghid,wake_req->bitset,wake_req->rflag,wake_req->pid,wake_req->fn_flag,_data->ops,sizeof(_remote_wakeup_request_t));

				}

    			GSPRINTK(KERN_ALERT"%s: sending to origin remote callpfn cpu: 0x{%d} request->ticket{%d}  \n",__func__,cpu,localticket_value);
    			if (cpu >= 0)
    			{
				spin_unlock(&value->_sp);
		//		printk(KERN_ALERT"%s: dest_cpu {%d} \n",__func__,cpu);
				res = pcn_kmsg_send_long(cpu, 
					(struct pcn_kmsg_long_message*)  ((_data->ops==WAKE_OPS)? (wake_req):(wait_req)),
					(_data->ops==WAKE_OPS) ? sizeof(_remote_wakeup_request_t) - sizeof(struct pcn_kmsg_hdr) : sizeof(_remote_key_request_t) - sizeof(struct pcn_kmsg_hdr));
		//		printk(KERN_ALERT"%s: msg return in remote {%d}  \n",__func__, res);
    			}

    		} else if (vma != NULL && _cpu == 0) {// && !(vma->vm_flags & VM_PFNMAP) ) {

    			if(_data->ops==WAIT_OPS){
					wait_req->fn_flags |= FLAGS_ORIGINCALL;
				
		//		 printk(KERN_ALERT"%s: msg wait: uaddr {%lx}  ticket {%d} tghid{%d} bitset {%u}  pid{%d}  ops{%d} size{%d} \n",
                  //      __func__,wait_req->uaddr,wait_req->ticket,wait_req->tghid,wait_req->bitset,wait_req->pid,_data->ops,sizeof(*wait_req));
				}
				else{
					wake_req->fn_flag |= FLAGS_ORIGINCALL;
					wake_req->rflag = current->pid;
				
		//		 printk(KERN_ALERT"%s: msg wake: uaddr {%lx}  uaddr2{%lx} ticket {%d} tghid{%d} bitset {%u} rflag{%d} pid{%d} ifn_flags{%lx} size{%d}\n",
                  //      __func__,wake_req->uaddr,(wake_req->uaddr2),wake_req->ticket,wake_req->tghid,wake_req->bitset,wake_req->rflag,wake_req->pid,wake_req->fn_flag,sizeof(*wake_req));
				}

    			GSPRINTK(KERN_ALERT"%s: sending to origin origin call cpu: 0x{%d}  \n",__func__,cpu,localticket_value);
    			if (cpu >= 0)
    			{
				spin_unlock(&value->_sp);
		//		printk(KERN_ALERT"%s: dest_cpu {%d} \n",__func__,cpu);
				res = pcn_kmsg_send_long(cpu, 
					(struct pcn_kmsg_long_message*)  ((_data->ops==WAKE_OPS)? (wake_req):(wait_req)),
					(_data->ops==WAKE_OPS) ? sizeof(_remote_wakeup_request_t) - sizeof(struct pcn_kmsg_hdr) : sizeof(_remote_key_request_t) - sizeof(struct pcn_kmsg_hdr));
		//		printk(KERN_ALERT"%s: msg return in remote {%d}  \n",__func__, res);
    			}
    		}
//		rq_ptr->_st=0;
    		wait_event_interruptible(rq_ptr->_wq, (rq_ptr->status == DONE));
    		GSPRINTK(KERN_ALERT"%s:after wake up process: task woken{%d}\n",__func__,current->pid);

out:
   kfree(wake_req);
   kfree(wait_req);
   return 0;

}


static int __init global_spinlock_init(void)
{
	int i=0;
_cpu = Kernel_Id;

	for (i = 0; i < ARRAY_SIZE(spin_bucket); i++) {
		spin_lock_init(&spin_bucket[i]._sp);
		spin_bucket[i]._st = 0;//TBR
		spin_bucket[i]._ticket = 0;
		spin_bucket[i].lock_st = 0;//TBR

		INIT_LIST_HEAD(&spin_bucket[i]._lrq_head);
	}

	for (i = 0; i < ARRAY_SIZE(global_bucket); i++) {
			spin_lock_init(&global_bucket[i].lock);
			global_bucket[i].thread_group_leader = NULL;
			global_bucket[i].worker_task=NULL;
			global_bucket[i].global_wq = NULL;
			global_bucket[i].free = 0;
		}


	return 0;
}
static void __exit global_spinlock_exit(void)
{

	int i=0;

}
__initcall(global_spinlock_init);

module_exit(global_spinlock_exit);
