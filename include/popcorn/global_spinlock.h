#ifndef __GLOBAL_SPINLOCK_H
#define __GLOBAL_SPINLOCK_H

#include <linux/workqueue.h>
#include	<linux/hash.h>
#include <linux/spinlock.h>
#include "request_data.h"

#define _SPIN_HASHBITS 8
#define _BITS_FOR_STATUS 3

#define INITIAL_STATE 0
#define PROCESSING_STATE 1
#define GLOBAL_STATE 2
#define HAS_TICKET 3

#define NORMAL_Q_PRIORITY 100
#define HIGH_Q_PRIORITY 10
#define HIGH1_Q_PRIORITY 50


#define sp_hashfn(uaddr, pid)      \
         hash_long((unsigned long)uaddr + (unsigned long)pid, _SPIN_HASHBITS)


typedef enum status {
	_has_no_ticket = 0,
	_processing_ticket,
	_has_ticket,
	_do_replication,
	_undo_replication,
	_has_global_ticket
} _lock_status;


typedef struct spin_key {
	pid_t _tgid;
	unsigned long _uaddr;
	int offset;
} _spin_key;

struct local_request_queue {
	struct task_struct *task;
	unsigned long _uaddr;
	struct list_head lrq_member;
} __attribute__((packed));
typedef struct local_request_queue _local_rq;

struct global_request_queue {
	volatile struct plist_node list;
	_remote_wakeup_request_t wakeup;
	_remote_key_request_t wait;
	int cnt;
	unsigned int ops:1; //0-wait 1-wake
}__attribute__((packed));
typedef struct global_request_queue _global_rq;

struct spin_value {
	spinlock_t _sp;
	volatile unsigned int _st; //token status
	volatile unsigned int lock_st; // lock is global or local
	volatile unsigned long _ticket;
	struct list_head _lrq_head;
};
typedef struct spin_value  _spin_value;

struct global_value {
	spinlock_t lock;
	volatile struct plist_head _grq_head;
	struct workqueue_struct *global_wq;
	struct task_struct *thread_group_leader;
	struct task_struct *worker_task;
	volatile unsigned int _is_alive;
	unsigned int free :1;
	char name[32];
};
typedef struct global_value _global_value;



_spin_value *hashspinkey(_spin_key *sk);

_global_value *hashgroup(struct task_struct *group_pid);

int global_spinlock(unsigned long uaddr, unsigned int fn_flag);
int global_spinunlock(unsigned long uaddr, unsigned int fn_flag);

extern _spin_value spin_bucket[1 << _SPIN_HASHBITS];

extern _global_value global_bucket[1 << _SPIN_HASHBITS];

#endif
