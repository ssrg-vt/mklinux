/*
 * Copyright (C) 2015 SSRG@VT
 *
 * Author: Yuzhong Wen <wyz2014@vt.edu>
 *
 */

#include <linux/ft_replication.h>
#include <linux/popcorn_namespace.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/pcn_kmsg.h>
#include <linux/utsname.h>
#include <asm/uaccess.h>
#include <linux/time.h>
#include <linux/sched.h>

#define FT_UTS_VERBOSE 1
#if FT_UTS_VERBOSE
#define FTPRINTK(...) printk(__VA_ARGS__)
#else
#define FTPRINTK(...) ;
#endif

struct uts_info {
	struct new_utsname utsname;
};

struct uts_msg {
	struct pcn_kmsg_hdr header;
	struct uts_info info;
	int uts_id;
	/* the following is pid_t linearized */
	struct ft_pop_rep_id ft_pop_id;
	int level;
	/* this must be the last field of the struct */
	int id_array[0];
};

struct sync_uts_work {
	struct work_struct work;
	struct ft_pop_rep *ft_popcorn;
	struct uts_info info;
};

static struct workqueue_struct *ft_uts_wq;

/*
 * Whenever someone changes the uts on a primary replica, this should be called to update
 * all the uts in other secondary replicas
 */
int sync_uts(struct task_struct *task)
{
	struct popcorn_namespace *pop = NULL;
	struct task_struct *ancestor;
	struct uts_msg *msg;
	size_t msg_size;
	int level;

	pop = task->nsproxy->pop_ns;
	if (!pop) {
		// Not in a popcorn namespace, we don't care
		return 0;
	}

	if(ft_is_primary_replica(task)){
		// Sync the secondary replicas from the primary one
		level = task->ft_pid.level;
		msg_size = sizeof(struct uts_msg) + level * sizeof(int);
		msg = kmalloc(msg_size, GFP_KERNEL);
		if (!msg)
			return -ENOMEM;

		msg->header.type = PCN_KMSG_TYPE_FT_UTS_SYNC;
		msg->header.prio = PCN_KMSG_PRIO_NORMAL;
		msg->uts_id = task->id_syscall;
		memcpy(&msg->ft_pop_id, &task->ft_pid.ft_pop_id, sizeof(struct ft_pop_rep_id));
		msg->level = task->ft_pid.level;
		memcpy(&msg->info.utsname, &task->nsproxy->uts_ns->ft_name, sizeof(struct new_utsname));
		send_to_all_secondary_replicas(task->ft_popcorn, (struct pcn_kmsg_long_message*) msg, msg_size);
		FTPRINTK("FT UTS sync out on %d\n", task->ft_pid);
	} 
	else{
		// Secondary one? Halt it.
		if(ft_is_secondary_replica(task)){
			// Secondary one? Halt it.
		}
	}
}

static int handle_uts_sync_req(struct pcn_kmsg_message* inc_msg)
{
	/*
	 * 1. Find all the processes
	 * 2. Update the uts
	 */
	struct task_struct *task, *g;
	struct uts_msg *msg = (struct uts_msg *) inc_msg;
	FTPRINTK("FT UTS request on %d\n", msg->ft_pop_id.id);
	do_each_thread (g, task) {
		if (task->ft_pid.ft_pop_id.id == msg->ft_pop_id.id &&
				task->ft_pid.level == msg->level) {
			down_write(&uts_sem);
			memcpy(&task->nsproxy->uts_ns->ft_name, &msg->info.utsname, sizeof(struct new_utsname));
			up_write(&uts_sem);
			FTPRINTK("FT UTS sync in on %d\n", task->ft_pid);
			break;
		}
	} while_each_thread (g, task);

	pcn_kmsg_free_msg (msg);
	return 0;
}

static int __init ft_uts_init(void)
{
//	ft_uts_wq = create_singlethread_workqueue("ft_uts_wq");
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_UTS_SYNC, handle_uts_sync_req);
	return 0;
}

late_initcall(ft_uts_init);
