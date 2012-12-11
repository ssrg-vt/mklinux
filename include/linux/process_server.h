/*********************
 * process server header
 * dkatz
 *********************/

#include <linux/sched.h>
#include <asm/ptrace.h>

#ifndef _PROCESS_SERVER_H
#define _PROCESS_SERVER_H


int test_process_server(void);
long process_server_clone(unsigned long clone_flags,
                          unsigned long stack_start,
                          struct pt_regs *regs,
                          unsigned long stack_size,
                          int __user *parent_tidptr,
                          int __user *child_tidptr,
                          struct task_struct* task);

#endif // _PROCESS_SERVER_H
