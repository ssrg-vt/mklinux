
#ifndef _KERNEL_POPCORN_H
#define _KERNEL_POPCORN_H

#include <linux/fs.h>
#include <linux/mm_types.h>
#include <linux/mman.h> // MAP_ANONYMOUS
#include <linux/sched.h>
#include <linux/slab.h>

#include <linux/popcorn_cpuinfo.h>
#include <linux/process_server.h>

void add_memory_entry(memory_t* entry);
int add_memory_entry_with_check(memory_t* entry);
memory_t* find_memory_entry(int cpu, int id);
struct mm_struct* find_dead_mapping(int cpu, int id);
memory_t* find_and_remove_memory_entry(int cpu, int id);
void remove_memory_entry(memory_t* entry);

void add_vma_ack_entry(vma_op_answers_t* entry);
void remove_vma_ack_entry(vma_op_answers_t* entry);

void end_distribute_operation(int operation, long start_ret, unsigned long addr,
			      int _cpu,
			      wait_queue_head_t *request_distributed_vma_op);
long start_distribute_operation(int operation, unsigned long addr, size_t len,
				unsigned long prot, unsigned long new_addr,
				unsigned long new_len, unsigned long flags,
				struct file *file, unsigned long pgoff, int _cpu,
				wait_queue_head_t *request_distributed_vma_op);

#endif /* _KERNEL_POPCORN_H */
