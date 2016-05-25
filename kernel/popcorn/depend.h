/*
 * depend.h
 *
 *  Created on: May 22, 2016
 *      Author: root
 */
// TODO this is staged code just to reduce complexity in the refactoring
// NOTE Some of these functions are Linux's and some are Popcorn's

#ifndef KERNEL_POPCORN_DEPEND_H_
#define KERNEL_POPCORN_DEPEND_H_

/* External functions */
extern unsigned long do_brk(unsigned long addr, unsigned long len);
extern unsigned long mremap_to(unsigned long addr, unsigned long old_len,
			       unsigned long new_addr, unsigned long new_len, bool *locked);
extern unsigned long read_old_rsp(void);
extern int exec_mmap(struct mm_struct *mm);
extern struct task_struct* do_fork_for_main_kernel_thread(unsigned long clone_flags,
							  unsigned long stack_start,
							  struct pt_regs *regs,
							  unsigned long stack_size,
							  int __user *parent_tidptr,
							  int __user *child_tidptr);
extern int do_wp_page_for_popcorn(struct mm_struct *mm, struct vm_area_struct *vma,
				  unsigned long address, pte_t *page_table, pmd_t *pmd,
				  spinlock_t *ptl, pte_t orig_pte);
extern int access_error(unsigned long error_code, struct vm_area_struct *vma);

//// internal declarations ////

static int do_mapping_for_distributed_process(mapping_answers_for_2_kernels_t* fetching_page,
                                              struct mm_struct* mm, unsigned long address, spinlock_t* ptl);



#endif /* KERNEL_POPCORN_DEPEND_H_ */
