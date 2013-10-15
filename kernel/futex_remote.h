/*
 * futex_remote.h
 *
 *  Created on: Oct 8, 2013
 *      Author: akshay
 */

#ifndef FUTEX_REMOTE_H_
#define FUTEX_REMOTE_H_



#define _FUTEX_HASHBITS (CONFIG_BASE_SMALL ? 4 : 8)

#define FLAGS_SHARED		0x01


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

int
get_futex_key_remote(u32 __user *uaddr, int fshared, union futex_key *key, int rw);
int
get_set_remote_key(unsigned long uaddr, unsigned int val, int fshared, union futex_key *key, int rw);
struct futex_q * query_q(struct task_struct *t);
int remote_futex_wakeup(unsigned long uaddr,unsigned int flags, int nr_wake, u32 bitset,union futex_key *key ,int rflag );

extern struct futex_hash_bucket futex_queues[1<<_FUTEX_HASHBITS];

extern void get_futex_key_refs(union futex_key *key);

extern int
futex_wake(u32 __user *uaddr, unsigned int flags, int nr_wake, u32 bitset);

extern const struct futex_q futex_q_init ;

extern struct futex_hash_bucket *hash_futex(union futex_key *key);

extern int
get_futex_key(u32 __user *uaddr, int fshared, union futex_key *key, int rw);
extern int match_futex(union futex_key *key1, union futex_key *key2);
extern void wake_futex(struct futex_q *q);
extern void put_futex_key(union futex_key *key);



struct _global_futex_key {
	unsigned int address;
	int pid;
	struct list_head list_member;
};

typedef struct _global_futex_key _global_futex_key_t;
_global_futex_key_t * add_key(int pid,int address, struct list_head *head) ;

_global_futex_key_t * find_key(int address, struct list_head *head);
int find_and_delete_key(int address, struct list_head *head) ;

extern struct list_head fq_head;


#endif /* FUTEX_REMOTE_H_ */
