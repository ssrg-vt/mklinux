/*
 * This file for Obtaining Remote PID file's info
 *
 * Akshay
 */

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/pcn_kmsg.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ptrace.h>
#include <asm/processor.h>
#include <linux/tty.h>
#include <linux/delayacct.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/pid_namespace.h>

#include "internal.h"
#include "remote_proc_pid_files.h"

#define PRINT_MESSAGES 0
#if PRINT_MESSAGES
#define PRINTK(...) printk(__VA_ARGS__)
#else
#define PRINTK(...) ;
#endif

/*
 *  Variables
 */
static int statwait=-1;
static int _cpu=-1;


static DECLARE_WAIT_QUEUE_HEAD(wq);


/*
 * ****************************** Message structures for obtaining PID status ********************************
 */
struct _remote_pid_stat_request {
    struct pcn_kmsg_hdr header;
    pid_t _pid;
    char pad_string[56];
} __attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_pid_stat_request _remote_pid_stat_request_t;

struct _remote_pid_stat_response {
    struct pcn_kmsg_hdr header;
	int _pid_ns;
	char _tcomm[128];
	char _state;
	pid_t	_ppid;
	pid_t	_pgid;
	pid_t	_sid;
	int	_tty_nr;
	int	_tty_pgrp;
	unsigned int _task_flags;
	unsigned long  	_min_flt;
	unsigned long  	_cmin_flt;
	unsigned long  	_maj_flt;
	unsigned long  	_cmaj_flt;
	cputime_t _utime;
	cputime_t _stime;
	cputime_t _cutime;
	cputime_t _cstime;
	long	_priority;
	long	_nice;
	int	_num_threads;
	unsigned long long _start_time;
	unsigned long _vsize;
	long _mm_rss;
	unsigned long	_rsslim;
	unsigned long _mm_start_code;
	unsigned long _mm_end_code;
	unsigned long _mm_start_stack;
	unsigned long _esp;
	unsigned long _eip;
	unsigned long _pending_signal;
	unsigned long _blocked_sig;
	unsigned long _sigign;
	unsigned long _sigcatch;
	unsigned long _wchan;
	/* insert two 0UL*/
	int _exit_signal;
	int	_task_cpu;
	unsigned int _rt_priority;
	unsigned int _policy;
	unsigned long long _delayacct_blkio_ticks;
	cputime_t _gtime;
	cputime_t _cgtime;
   } __attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_pid_stat_response _remote_pid_stat_response_t;

/*
 * ******************************* Define variables holding Result *******************************************
 */
static _remote_pid_stat_response_t *stat_result;

/*
 * ******************************* Common Functions **********************************************************
 */

struct task_struct * get_process(pid_t pid) {
	struct task_struct *p;
	struct task_struct *result;

		for_each_process(p)
		{
			if(p->pid==pid)
			{
				result=p;
				return result;
			}
		}
	return NULL;

}

int flush_stat_var()
{
	stat_result=NULL;
	statwait=-1;
	return 0;
}


/*
 * ********************************** Message handling functions for /pid/stat*************************************
 */

int fill_response(struct task_struct *task, _remote_pid_stat_response_t *res)
{
	int whole=1;
	struct pid_namespace *ns;
	struct mm_struct *mm;
	sigset_t sigign, sigcatch;
	unsigned long flags;
	int _permitted;

	ns = current->nsproxy->pid_ns;

	res->_pid_ns=pid_nr_ns(task_pid(task), ns);

	res->_state = *get_task_state(task);
	res->_vsize = 0;
	res->_eip = 0;
	res->_esp = 0;
	_permitted = ptrace_may_access(task, PTRACE_MODE_READ);
	mm = get_task_mm(task);
		if (mm) {
			res->_vsize = task_vsize(mm);
			if (_permitted) {
				res->_eip = KSTK_EIP(task);
				res->_esp = KSTK_ESP(task);
			}
		}

	get_task_comm(res->_tcomm, task);


	sigemptyset(&sigign);
	sigemptyset(&sigcatch);
	res->_cutime = res->_cstime = res->_utime = res->_stime = cputime_zero;
	res->_cgtime = res->_gtime = cputime_zero;

	if (lock_task_sighand(task, &flags)) {
		struct signal_struct *sig = task->signal;

		if (sig->tty) {
			struct pid *pgrp = tty_get_pgrp(sig->tty);
			res->_tty_pgrp = pid_nr_ns(pgrp, ns);
			put_pid(pgrp);
			res->_tty_nr = new_encode_dev(tty_devnum(sig->tty));
		}

			res->_num_threads = get_nr_threads(task);
			collect_sigign_sigcatch(task, &sigign, &sigcatch);

			res->_cmin_flt = sig->cmin_flt;
			res->_cmaj_flt = sig->cmaj_flt;
			res->_cutime = sig->cutime;
			res->_cstime = sig->cstime;
			res->_cgtime = sig->cgtime;
			res->_rsslim = ACCESS_ONCE(sig->rlim[RLIMIT_RSS].rlim_cur);

			/* add up live thread stats at the group level */
			if (whole) {
				struct task_struct *t = task;
				do {
					res->_min_flt += t->min_flt;
					res->_maj_flt += t->maj_flt;
					res->_gtime = cputime_add(res->_gtime, t->gtime);
					t = next_thread(t);
				} while (t != task);

				res->_min_flt += sig->min_flt;
				res->_maj_flt += sig->maj_flt;
				thread_group_times(task, &res->_utime, &res->_stime);
				res->_gtime = cputime_add(res->_gtime, sig->gtime);
			}

			res->_sid = task_session_nr_ns(task, ns);
			res->_ppid = task_tgid_nr_ns(task->real_parent, ns);
			res->_pgid = task_pgrp_nr_ns(task, ns);

			unlock_task_sighand(task, &flags);
		}

		if (_permitted && (!whole || res->_num_threads < 2))
			res->_wchan = get_wchan(task);
		if (!whole) {
			res->_min_flt = task->min_flt;
			res->_maj_flt = task->maj_flt;
			task_times(task, &res->_utime, &res->_stime);
			res->_gtime = task->gtime;
		}
		res->_priority = task_prio(task);
		res->_nice = task_nice(task);

		res->_start_time =
			(unsigned long long)task->real_start_time.tv_sec * NSEC_PER_SEC
					+ task->real_start_time.tv_nsec;
		/* convert nsec -> ticks */
		res->_start_time = nsec_to_clock_t(res->_start_time);

		res->_task_flags=task->flags;
		res->_utime=cputime_to_clock_t(res->_utime);
		res->_stime=cputime_to_clock_t(res->_stime);
		res->_cutime=cputime_to_clock_t(res->_cutime);
		res->_cstime=cputime_to_clock_t(res->_cstime);
		res->_mm_rss= mm ? get_mm_rss(mm) : 0;
		res->_mm_start_code = mm ? (_permitted ? mm->start_code : 1) : 0;
		res->_mm_end_code = mm ? (_permitted ? mm->end_code : 1) : 0;
		res->_mm_start_stack = (_permitted && mm) ? mm->start_stack : 0;
		res->_pending_signal = task->pending.signal.sig[0] & 0x7fffffffUL;
		res->_blocked_sig = task->blocked.sig[0] & 0x7fffffffUL;
		res->_sigign = sigign.sig[0] & 0x7fffffffUL;
		res->_sigcatch = sigcatch.sig[0] & 0x7fffffffUL;
		res->_exit_signal = task->exit_signal;
		res->_task_cpu = task_cpu(task);
		res->_rt_priority = task->rt_priority;
		res->_policy = 	task->policy;
		res->_delayacct_blkio_ticks = (unsigned long long)delayacct_blkio_ticks(task);
		res->_gtime = cputime_to_clock_t(res->_gtime );
		res->_cgtime = 	cputime_to_clock_t(res->_cgtime);


		return 0;

}

static int handle_remote_pid_stat_response(struct pcn_kmsg_message* inc_msg) {
	_remote_pid_stat_response_t* msg = (_remote_pid_stat_response_t*) inc_msg;

	PRINTK("%s: Entered remote pid stat response \n",__func__);

	statwait = 1;
	if(msg !=NULL)
	 stat_result=msg;
	wake_up_interruptible(&wq);
	PRINTK("%s: response ---- wait{%d} \n",
			__func__, statwait);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}


static int handle_remote_pid_stat_request(struct pcn_kmsg_message* inc_msg) {

	_remote_pid_stat_request_t* msg = (_remote_pid_stat_request_t*) inc_msg;
	_remote_pid_stat_response_t response;


	PRINTK("%s: Entered remote pid stat request \n", __func__);

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_PID_STAT_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	struct task_struct *task;
	task=get_process(msg->_pid);

	if(task!=NULL)
	fill_response(task,&response);

	// Send response
	pcn_kmsg_send_long(msg->header.from_cpu, (struct pcn_kmsg_message*) (&response),
			sizeof(_remote_pid_stat_response_t) - sizeof(struct pcn_kmsg_hdr));

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

int send_stat_request(struct proc_remote_pid_info *task)
{

		int res=0;
		_remote_pid_stat_request_t* request = kmalloc(sizeof(_remote_pid_stat_request_t),
		GFP_KERNEL);
		// Build request
		request->header.type = PCN_KMSG_TYPE_REMOTE_PID_STAT_REQUEST;
		request->header.prio = PCN_KMSG_PRIO_NORMAL;
		request->_pid = task->pid;
		// Send response
		if(task->Kernel_Num==0)
				res=pcn_kmsg_send(0, (struct pcn_kmsg_message*) (request));
		else
				res=pcn_kmsg_send(3, (struct pcn_kmsg_message*) (request));
		return res;
}

/*
 * ************************************* Function (hook) to be called from other file ********************
 */
int do_remote_task_stat(struct seq_file *m,
		struct proc_remote_pid_info *task, char *buf,size_t count)
{
	flush_stat_var();
	int res=0;

	res = send_stat_request(task);
	wait_event_interruptible(wq, statwait != -1);

	if(stat_result!=NULL)
	{
	seq_printf(m, "%d (%s) %c %d %d %d %d %d %u %lu \
%lu %lu %lu %lu %lu %ld %ld %ld %ld %d 0 %llu %lu %ld %lu %lu %lu %lu %lu \
%lu %lu %lu %lu %lu %lu %lu %lu %d %d %u %u %llu %lu %ld\n",
		stat_result->_pid_ns,
		stat_result->_tcomm,
		stat_result->_state,
		stat_result->_ppid,
		stat_result->_pgid,
		stat_result->_sid,
		stat_result->_tty_nr,
		stat_result->_tty_pgrp,
		2,
		stat_result->_min_flt,
		stat_result->_cmin_flt,
		stat_result->_maj_flt,
		stat_result->_cmaj_flt,
		stat_result->_utime,
		stat_result->_stime,
		stat_result->_cutime,
		stat_result->_cstime,
		stat_result->_priority,
		stat_result->_nice,
		stat_result->_num_threads,
		stat_result->_start_time,
		stat_result->_vsize,
		stat_result->_mm_rss,
		stat_result->_rsslim,
		stat_result->_mm_start_code,
		stat_result->_mm_end_code,
		stat_result->_mm_start_stack,
		stat_result->_esp,
		stat_result->_eip,
		stat_result->_pending_signal,
		stat_result->_blocked_sig,
		stat_result->_sigcatch,
		stat_result->_sigign,
		stat_result->_wchan,
		0UL,
		0UL,
		stat_result->_exit_signal,
		stat_result->_task_cpu,
		stat_result->_rt_priority,
		stat_result->_policy,
		stat_result->_delayacct_blkio_ticks,
		stat_result->_gtime,
		stat_result->_cgtime);
	}
	else
	{
		seq_printf(m, "(%s) \n",
				"This PID is out of sync with remote kernel");
	}

	return 0;
}





static int __init pid_files_handler_init(void)
{


    _cpu = smp_processor_id();


	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_PID_STAT_REQUEST,
				    		handle_remote_pid_stat_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_PID_STAT_RESPONSE,
				    		handle_remote_pid_stat_response);

	return 0;
}
/**
 * Register remote proc files init function as
 * module initialization function.
 */
late_initcall(pid_files_handler_init);
