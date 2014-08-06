/*********************
 * process server header
 * dkatz
 *********************/

#include <linux/sched.h>
#include <asm/ptrace.h>
#include <linux/process_server_macro.h>

#ifndef _PROCESS_SERVER_H
#define _PROCESS_SERVER_H

/*
 * Migration hook.
 */
int process_server_do_migration(struct task_struct* task, int cpu, struct pt_regs* regs);
void sleep_shadow(void);
void create_thread_pull(void);
int process_server_update_page(struct task_struct * tsk,struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address, unsigned long page_fault_flags);
int process_server_notify_delegated_subprocess_starting(pid_t pid, pid_t remote_pid, int remote_cpu);
int process_server_task_exit_notification(struct task_struct *tsk,long code);
int process_server_try_handle_mm_fault(struct task_struct *tsk,struct mm_struct *mm, struct vm_area_struct *vma,
		unsigned long address, unsigned long page_fault_flags,unsigned long error_code);
void process_server_clean_page(struct page* page);
int process_server_dup_task(struct task_struct* orig, struct task_struct* task);
long start_distribute_operation(int operation, unsigned long addr,
		size_t len, unsigned long prot,unsigned long new_addr,unsigned long new_len,
		unsigned long flags,struct file *file,unsigned long pgoff);
void end_distribute_operation(int operation,  long start_ret, unsigned long addr);

long process_server_do_unmap_start(struct mm_struct *mm, unsigned long start, size_t len);
long process_server_do_unmap_end(struct mm_struct *mm, unsigned long start, size_t len,int start_ret);

long process_server_mprotect_start(unsigned long start, size_t len,unsigned long prot);
long process_server_mprotect_end(unsigned long start, size_t len,unsigned long prot,int start_ret);

long process_server_do_mmap_pgoff_start(struct file *file, unsigned long addr,
		unsigned long len, unsigned long prot,
		unsigned long flags, unsigned long pgoff);
long process_server_do_mmap_pgoff_end(struct file *file, unsigned long addr,
		unsigned long len, unsigned long prot,
		unsigned long flags, unsigned long pgoff, unsigned long start_ret);

long process_server_do_brk_start(unsigned long addr, unsigned long len);
long process_server_do_brk_end(unsigned long addr, unsigned long len, unsigned long start_ret);

long process_server_do_mremap_start(unsigned long addr,
		unsigned long old_len, unsigned long new_len,
		unsigned long flags, unsigned long new_addr);
long process_server_do_mremap_end(unsigned long addr,
		unsigned long old_len, unsigned long new_len,
		unsigned long flags, unsigned long new_addr,unsigned long start_ret);

#endif // _PROCESS_SERVER_H
