/*
 * ft_replication.h
 *
 * Author: Marina
 */

#ifndef FT_REPLICATION_H_
#define FT_REPLICATION_H_

#include <linux/list.h>
#include <linux/kref.h>
#include <linux/spinlock_types.h>
#include <linux/wait.h>

#define FT_POPCORN

/* int replica_type of struct task_struct can be only one of the 
 * following:
 */
#define NOT_REPLICATED 0
#define HOT_REPLICA 1
#define COLD_REPLICA 2
#define POTENTIAL_HOT_REPLICA 3
#define POTENTIAL_COLD_REPLICA 4
#define ROOT_POT_HOT_REPLICA 5
#define REPLICA_DESCENDANT 6
#define NEW_HOT_REPLICA_DESCENDANT 7
#define NEW_COLD_REPLICA_DESCENDANT 8

#define WAIT_ANSWER_TIMEOUT_SECOND 5

/*by seeing include/asm-generic/errno.h seems to be the next available...*/
#define ENOFTREP 134

#define FT_FILTER_DISABLE 0
#define FT_FILTER_ENABLE 1

#define FT_FILTER_COLD_REPLICA 2
#define FT_FILTER_HOT_REPLICA 3

struct replica_id{
        int kernel;
        pid_t pid;
};

struct replica_id_list{
        struct list_head replica_list_member;
        struct replica_id replica;
};

struct ft_pop_rep_id{
        int kernel;
        int id;
};

struct ft_pid{
        int level;
        int* id_array;
	struct ft_pop_rep_id ft_pop_id;
};
int are_ft_pid_equals(struct ft_pid* first, struct ft_pid* second);

struct ft_pop_rep{
	struct kref kref;
	struct ft_pop_rep_id id;
	int replication_degree;
	struct replica_id hot_replica;
	struct replica_id_list cold_replicas_head;
};

void get_ft_pop_rep(struct ft_pop_rep* ft_pop);
void put_ft_pop_rep(struct ft_pop_rep* ft_pop);

struct net_filter_info{
	struct list_head list_member;
	struct kref kref;
	struct ft_pop_rep* ft_popcorn;
	
	/* NOTE creator and id compose the identifier.
	 * correspondig sockets between kernels will have the same 
	 * idetifier.
	 */
	struct ft_pid creator;
	int id;

	int type;
	spinlock_t lock;
	unsigned long long local_tx;
	volatile unsigned long long hot_tx;
	unsigned long long local_rx;
	unsigned long long hot_rx;
	wait_queue_head_t wait_queue;
};
void get_ft_filter(struct net_filter_info* filter);
void put_ft_filter(struct net_filter_info* filter);

int maybe_create_replicas(void);

struct task_struct;
int copy_replication(unsigned long flags, struct task_struct *tsk);

struct socket;
int net_ft_tx_filter(struct socket* socket);
int create_filter(struct task_struct *task, struct socket *sock);
int create_filter_accept(struct socket *newsock,struct socket *sock);

#endif
