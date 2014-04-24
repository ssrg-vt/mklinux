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
#include <popcorn/global_spinlock.h>

#include <linux/pcn_kmsg.h>

#define FLAGS_SYSCALL		8
#define FLAGS_REMOTECALL	16
#define FLAGS_ORIGINCALL	32

 _spin_value spin_bucket[1<<_SPIN_HASHBITS];

 _global_value global_bucket[1<<_SPIN_HASHBITS];

 extern struct vm_area_struct * getVMAfromUaddr(unsigned long uaddr);
 extern pte_t *do_page_walk(unsigned long address);
 extern  int find_kernel_for_pfn(unsigned long addr, struct list_head *head);



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
	//u32 hash = jhash2((u32*)&sk->_uaddr,(sizeof(sk->_uaddr)+sizeof(sk->_uaddr))/4,100);
	u32 hash = sp_hashfn(sk->_uaddr,sk->_tgid);
	printk(KERN_ALERT"%s: hashspin{%u} -{%u} _uaddr{%lx) len{%d} \n", __func__,hash,hash & ((1 << _SPIN_HASHBITS)-1),sk->_uaddr,(sizeof(sk->_uaddr)+sizeof(sk->_uaddr))/4);
	return &spin_bucket[hash];
}

//to get the global worker and global request queue
_global_value *hashgroup(struct task_struct *group_pid)
{
	struct task_struct *tsk =NULL;
	tsk= group_pid;
	//u32 hash = jhash2((u32*)&tsk->pid,(sizeof(tsk->pid))/4,JHASH_INITVAL);
	u32 hash = sp_hashfn(tsk->pid,0);
	printk(KERN_ALERT"%s: globalhash{%u} \n", __func__,hash & ((1 << _SPIN_HASHBITS)-1));
	return &global_bucket[hash];
}
// Perform global spin lock
int global_spinlock(unsigned long uaddr,unsigned int fn_flag){

	int localticket_value;
	int res = 0;
	int cpu=0;

	 struct spin_key sk;
	 spin_key_init(&sk);

	getKey(uaddr, &sk,current->tgroup_home_id);
	_spin_value *value = hashspinkey(&sk);

	_remote_key_request_t *request = kmalloc(sizeof(_remote_key_request_t),
				GFP_ATOMIC);
	printk(KERN_ALERT"%s: request -- entered whos calling{%s} \n", __func__,current->comm);
	printk(KERN_ALERT"%s:  uaddr {%lx} fn_flag {%lx}  pid{%d} current->tgroup_home_id{%d}\n",
				__func__,uaddr,fn_flag,current->pid,current->tgroup_home_id);

	// Finish constructing response
	request->header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;

	struct vm_area_struct *vma;
	vma = getVMAfromUaddr(uaddr);

	request->flags = 0;
	request->uaddr =(unsigned long)uaddr;
	request->pid = current->pid;
	request->origin_pid = current->origin_pid;
	request->tghid = current->tgroup_home_id;
	request->fn_flags = fn_flag;

	request->ticket = 0; //set the request has no ticket

	unsigned long pfn;
	pte_t pte;
	pte = *((pte_t *) do_page_walk((unsigned long)uaddr));
	printk(KERN_ALERT"%s pte ptr : ox{%lx} cpu{%d} \n",__func__,pte,smp_processor_id());
	pfn = pte_pfn(pte);
	printk(KERN_ALERT"%s pte pfn : 0x{%lx}\n",__func__,pfn);

	//TODO: for error check need to sequentialize
	localticket_value = xadd_sync(&value->_ticket, 1);
    if(1){
    	//pcn_kmsg and wait
    	if (vma->vm_flags & VM_PFNMAP) {
    			if ((cpu = find_kernel_for_pfn(pfn, &pfn_list_head)) != -1)
    					{
    				printk(KERN_ALERT"%s: sending to origin pfn cpu: 0x{%d}\n",__func__,cpu);
    				res = pcn_kmsg_send(cpu, (struct pcn_kmsg_message*) (request));
    			}
    		} else if ((fn_flag & FLAGS_ORIGINCALL)) {
    			if ((cpu = find_kernel_for_pfn(pfn, &pfn_list_head)) != -1)
    					{
    				printk(KERN_ALERT"%s: sending to origin pfn cpu: 0x{%d}\n",__func__,cpu);
    				res = pcn_kmsg_send(cpu, (struct pcn_kmsg_message*) (request));
    			}
    		}
    	//TODO: add to local lrq as the head
    	printk(KERN_ALERT"%s:goto sleep after ticket request: 0x{%d}\n",__func__,cpu);

    	//check if it has acquired valid ticket
    	while(!(value->_st == HAS_TICKET)){
    		schedule();
    		printk(KERN_ALERT"%s:after wake up process: 0x{%d} {%lx}\n",__func__,cpu,value->_st);
    	}
    }


out:
   kfree(request);
   return 0;

}


int global_spinunlock(unsigned long uaddr,unsigned int fn_flag){

	int localticket_value;
	_lock_status tempstatus ;
	tempstatus = _has_ticket;
	int res = 0;	int cpu=0;

	_spin_key sk ;
	spin_key_init(&sk);

	getKey(uaddr, &sk,current->tgroup_home_id);
	_spin_value *value = hashspinkey(&sk);

	_remote_key_request_t *request = kmalloc(sizeof(_remote_key_request_t),
					GFP_ATOMIC);
		printk(KERN_ALERT"%s: request -- entered whos calling{%s} \n", __func__,current->comm);
		printk(KERN_ALERT"%s:  uaddr {%lx} fn_flag {%lx} val{%d}  pid{%d} \n",
					__func__,uaddr,fn_flag,current->pid);

		// Finish constructing response
		request->header.type = PCN_KMSG_TYPE_REMOTE_IPC_FUTEX_KEY_REQUEST;
		request->header.prio = PCN_KMSG_PRIO_NORMAL;

		struct vm_area_struct *vma;
		vma = getVMAfromUaddr(uaddr);

		request->flags = 0;
		request->uaddr =(unsigned long)uaddr;
		request->pid = current->pid;
		request->origin_pid = current->origin_pid;
		request->tghid = current->tgroup_home_id;
		request->fn_flags = fn_flag;

		request->ticket = 2; //set the request to release lock

		unsigned long pfn;
		pte_t pte;
		pte = *((pte_t *) do_page_walk((unsigned long)uaddr));
		printk(KERN_ALERT"%s pte ptr : ox{%lx} cpu{%d} \n",__func__,pte,smp_processor_id());
		pfn = pte_pfn(pte);
		printk(KERN_ALERT"%s pte pfn : 0x{%lx}\n",__func__,pfn);


		localticket_value = xadd_sync(&value->_ticket, 1);
	    if(localticket_value){
	    	//pcn_kmsg and wait
	    	if (vma->vm_flags & VM_PFNMAP) {
	    			if ((cpu = find_kernel_for_pfn(pfn, &pfn_list_head)) != -1)
	    					{
	    				printk(KERN_ALERT"%s: sending to origin pfn cpu: 0x{%d}\n",__func__,cpu);
	    				res = pcn_kmsg_send(cpu, (struct pcn_kmsg_message*) (request));
	    			}
	    		} else if ((fn_flag & FLAGS_ORIGINCALL)) {
	    			if ((cpu = find_kernel_for_pfn(pfn, &pfn_list_head)) != -1)
	    					{
	    				printk(KERN_ALERT"%s: sending to origin pfn cpu: 0x{%d}\n",__func__,cpu);
	    				res = pcn_kmsg_send(cpu, (struct pcn_kmsg_message*) (request));
	    			}
	    		}
	    	//check if it has acquired valid ticket
	    	if((value->_st == HAS_TICKET)){
	    		cmpxchg(&value->_st, HAS_TICKET, INITIAL_STATE);// release lock in remote node
	    	}
	    }

out:
	kfree(request);
	return 0;
}
static int __init global_spinlock_init(void)
{
	int i=0;


	for (i = 0; i < ARRAY_SIZE(spin_bucket); i++) {
		spin_lock_init(&spin_bucket[i]._sp);
		spin_bucket[i]._st = 0;
		spin_bucket[i]._ticket = 0;

		INIT_LIST_HEAD(&spin_bucket[i]._lrq_head);
	}

	for (i = 0; i < ARRAY_SIZE(global_bucket); i++) {
			spin_lock_init(&global_bucket[i].lock);
			global_bucket[i].thread_group_leader = NULL;
			global_bucket[i].global_wq = NULL;
			plist_head_init(&global_bucket[i]._grq_head);
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
