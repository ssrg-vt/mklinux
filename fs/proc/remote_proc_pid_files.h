/*
 * Header for Obtaining Remote PID's file info
 *
 * Akshay
 */

#ifndef __REMOTE_PROC_PID_FILES_H__
#define __REMOTE_PROC_PID_FILES_H__

struct task_struct;

//functions borrowed from array.c
extern void collect_sigign_sigcatch(struct task_struct *p, sigset_t *ign,
	    sigset_t *catch);

extern const char *get_task_state(struct task_struct *tsk);


#endif /* __REMOTE_PROC_PID_FILES_H__ */
