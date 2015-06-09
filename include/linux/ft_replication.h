/*
 * ft_replication.h
 *
 * Author: Marina
 */

#ifndef FT_REPLICATION_H_
#define FT_REPLICATION_H_

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

#define WAIT_ANSWER_TIMEOUT_SECOND 5

struct replica_id{
        int kernel;
        pid_t pid;
};

int maybe_create_replicas(void);

struct task_struct;
int copy_replication(unsigned long flags, struct task_struct *tsk);

#endif
