/*
 * futex_remote.h
 *
 *  Created on: Oct 8, 2013
 *      Author: akshay
 */

#ifndef FUTEX_REMOTE_H_
#define FUTEX_REMOTE_H_

union futex_key;


#define _FUTEX_HASHBITS (CONFIG_BASE_SMALL ? 4 : 8)

#define FLAGS_SHARED		0x01
#define FLAGS_DESTROY		0x100

/*
 * Priority Inheritance state:
 */
struct futex_pi_state {
	/*
	 * list of 'owned' pi_state instances - these have to be
	 * cleaned up in do_exit() if the task exits prematurely:
	 */
	struct list_head list;

	/*
	 * The PI object:
	 */
	struct rt_mutex pi_mutex;

	struct task_struct *owner;
	atomic_t refcount;

	union futex_key key;
};

/**
 * struct futex_q - The hashed futex queue entry, one per waiting task
 * @list:		priority-sorted list of tasks waiting on this futex
 * @task:		the task waiting on the futex
 * @lock_ptr:		the hash bucket lock
 * @key:		the key the futex is hashed on
 * @pi_state:		optional priority inheritance state
 * @rt_waiter:		rt_waiter storage for use with requeue_pi
 * @requeue_pi_key:	the requeue_pi target futex key
 * @bitset:		bitset for the optional bitmasked wakeup
 *
 * We use this hashed waitqueue, instead of a normal wait_queue_t, so
 * we can wake only the relevant ones (hashed queues may be shared).
 *
 * A futex_q has a woken state, just like tasks have TASK_RUNNING.
 * It is considered woken when plist_node_empty(&q->list) || q->lock_ptr == 0.
 * The order of wakeup is always to make the first condition true, then
 * the second.
 *
 * PI futexes are typically woken before they are removed from the hash list via
 * the rt_mutex code. See unqueue_me_pi().
 */
struct futex_q {
	struct plist_node list;

	struct task_struct *task;
	spinlock_t *lock_ptr;
	union futex_key key;
	struct futex_pi_state *pi_state;
	struct rt_mutex_waiter *rt_waiter;
	union futex_key *requeue_pi_key;
	u32 bitset;

	/*mklinux_akshay*/
	pid_t rem_pid;
	union futex_key rem_requeue_key;
	unsigned long req_addr;
};



/*
 * Hash buckets are shared by all the futex_keys that hash to the same
 * location.  Each key may have multiple futex_q structures, one for each task
 * waiting on a futex.
 */
struct futex_hash_bucket {
	spinlock_t lock;
	struct plist_head chain;
};

int query_q_and_wake(struct task_struct *t,int kernel);
struct futex_q ** query_q_pid(struct task_struct *t,int kernel);
struct futex_q * query_q(struct task_struct *t);




int remote_futex_wakeup(u32 __user  *uaddr,unsigned int flags, int nr_wake, u32 bitset,union futex_key *key, int rflag,
		unsigned int fn_flags, unsigned long uaddr2,  int nr_requeue, int cmpval);
extern struct futex_hash_bucket futex_queues[1<<_FUTEX_HASHBITS];
extern void get_futex_key_refs(union futex_key *key);
extern int
futex_wake(u32 __user *uaddr, unsigned int flags, int nr_wake, u32 bitset,unsigned int fn_flags,struct task_struct *tsk);
extern int futex_requeue(u32 __user *uaddr1, unsigned int flags,
			 u32 __user *uaddr2, int nr_wake, int nr_requeue,
			 u32 *cmpval, int requeue_pi,unsigned int fn_flags, struct task_struct *tsk);
extern int futex_wake_op(u32 __user *uaddr1, unsigned int flags, u32 __user *uaddr2,
	      int nr_wake, int nr_wake2, int op,unsigned int fn_flags,struct task_struct *tsk);
extern int futex_wait(u32 __user *uaddr, unsigned int flags, u32 val,
		      ktime_t *abs_time, u32 bitset, unsigned int fn_flag);
extern const struct futex_q futex_q_init ;
extern struct futex_hash_bucket *hash_futex(union futex_key *key);
extern int
get_futex_key(u32 __user *uaddr, int fshared, union futex_key *key, int rw);

extern int 
get_futex_key_tsk(u32 __user *uaddr, int fshared, union futex_key *key, int rw, struct task_struct *tsk);

extern int match_futex(union futex_key *key1, union futex_key *key2);
extern void wake_futex(struct futex_q *q);
extern void put_futex_key(union futex_key *key);
extern void __unqueue_futex(struct futex_q *q);
extern int fault_in_user_writeable(u32 __user * uaddr);
extern int fault_in_user_writeable_task(u32 __user * uaddr, struct task_struct * tgid);
pte_t *do_page_walk(unsigned long address);

int find_kernel_for_pfn(unsigned long addr, struct list_head *head);


int get_futex_value_locked(u32 *dest, u32 __user *from);
extern int futex_global_worker_cleanup(struct task_struct *tsk);
extern struct list_head fq_head;

struct kernel_robust_list {
	struct kernel_robust_list *next;
};

struct kernel_robust_list_head {

	struct kernel_robust_list list;

	long futex_offset;

	struct kernel_robust_list  *list_op_pending;
};
// for fn_flags
#define FLAGS_WAKECALL		64
#define FLAGS_REQCALL		128
#define FLAGS_WAKEOPCALL	256

#define FLAGS_SYSCALL		8
#define FLAGS_REMOTECALL	16
#define FLAGS_ORIGINCALL	32

#define FLAGS_MAX	FLAGS_SYSCALL+FLAGS_REMOTECALL+FLAGS_ORIGINCALL+FLAGS_WAKECALL+FLAGS_REQCALL+FLAGS_WAKEOPCALL
//#define FUTEX_STAT
#undef FUTEX_STAT
extern struct vm_area_struct * getVMAfromUaddr(unsigned long uaddr);
int print_wait_perf(void);

int print_wake_perf(void);
int print_wakeop_perf(void);
int print_requeue_perf(void);
#endif /* FUTEX_REMOTE_H_ */
