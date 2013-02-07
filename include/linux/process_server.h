/*********************
 * process server header
 * dkatz
 *********************/

#include <linux/sched.h>
#include <asm/ptrace.h>

#ifndef _PROCESS_SERVER_H
#define _PROCESS_SERVER_H

#define PROCESS_SERVER_CLONE_SUCCESS 0
#define PROCESS_SERVER_CLONE_FAIL 1

typedef int(*scheduler_fn_t)();

int process_server_import_address_space(unsigned long* ip, unsigned long *sp, struct pt_regs* regs);
int test_process_server(void);
long process_server_clone(unsigned long clone_flags,
                          unsigned long stack_start,
                          struct pt_regs *regs,
                          unsigned long stack_size,
                          struct task_struct* task);
int process_server_notify_delegated_subprocess_starting(pid_t pid, pid_t remote_pid, int remote_cpu);
int process_server_task_exit_notification(pid_t pid);

int process_server_register_scheduler(scheduler_fn_t scheduler_impl);

#endif // _PROCESS_SERVER_H
