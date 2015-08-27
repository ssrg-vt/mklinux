
#ifndef _KERNEL_POPCORN_H
#define _KERNEL_POPCORN_H

#include <linux/fs.h>
#include <linux/highmem.h> //Replication
#include <linux/memcontrol.h>
#include <linux/mm_types.h>
#include <linux/mman.h> // MAP_ANONYMOUS
#include <linux/mmu_notifier.h>
#include <linux/pagemap.h>
#include <linux/rmap.h>
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

int do_remote_read_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma,
		unsigned long address, unsigned long page_fault_flags,
		pmd_t* pmd, pte_t* pte,
		spinlock_t* ptl, struct page* page, int _cpu);
int do_remote_read(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
		struct page* page, int _cpu);
int do_remote_write_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
		struct page* page,int invalid, int _cpu);
int do_remote_write(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
		struct page* page, int _cpu);
int do_remote_fetch_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, pte_t value_pte,
		spinlock_t* ptl, int _cpu);
int do_remote_fetch(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, pte_t value_pte,
		spinlock_t* ptl, int _cpu);

#if FOR_2_KERNELS
mapping_answers_for_2_kernels_t* find_mapping_entry(int cpu, int id, unsigned long address);
#else
mapping_answers_t* find_mapping_entry(int cpu, int id, unsigned long address);
#endif

#if FOR_2_KERNELS
void remove_mapping_entry(mapping_answers_for_2_kernels_t* entry);
#else
void remove_mapping_entry(mapping_answers_t* entry);
#endif

#if FOR_2_KERNELS
ack_answers_for_2_kernels_t* find_ack_entry(int cpu, int id, unsigned long address);
#else
ack_answers_t* find_ack_entry(int cpu, int id, unsigned long address);
#endif

#if FOR_2_KERNELS
int do_mapping_for_distributed_process(mapping_answers_for_2_kernels_t* fetching_page,
				       struct mm_struct* mm, unsigned long address,
				       spinlock_t* ptl);
#else
int do_mapping_for_distributed_process(mapping_answers_t* fetching_page,
				       struct mm_struct* mm, unsigned long address,
				       spinlock_t* ptl);
#endif

#endif /* _KERNEL_POPCORN_H */
