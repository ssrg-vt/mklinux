/*
 * VDSO implementation for AArch64 and vector page setup for AArch32.
 *
 * Copyright (C) 2012 ARM Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Will Deacon <will.deacon@arm.com>
 */

#include <linux/kernel.h>
#include <linux/clocksource.h>
#include <linux/elf.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/timekeeper_internal.h>
#include <linux/vmalloc.h>

#include <asm/cacheflush.h>
#include <asm/signal32.h>
#include <asm/vdso.h>
#include <asm/vdso_datapage.h>

extern char vdso_start, vdso_end;
static unsigned long vdso_pages;
static struct page **vdso_pagelist;

#ifdef CONFIG_ARM64_ILP32
extern char vdso_ilp32_start, vdso_ilp32_end;
static unsigned long vdso_ilp32_pages;
static struct page **vdso_ilp32_pagelist;
#endif

/*
 * The vDSO data page.
 */
static union {
	struct vdso_data	data;
	u8			page[PAGE_SIZE];
} vdso_data_store __page_aligned_data;
struct vdso_data *vdso_data = &vdso_data_store.data;

#ifdef CONFIG_AARCH32_EL0
/*
 * Create and map the vectors page for AArch32 tasks.
 */
static struct page *vectors_page[1];

static int alloc_vectors_page(void)
{
	extern char __kuser_helper_start[], __kuser_helper_end[];
	extern char __aarch32_sigret_code_start[], __aarch32_sigret_code_end[];

	int kuser_sz = __kuser_helper_end - __kuser_helper_start;
	int sigret_sz = __aarch32_sigret_code_end - __aarch32_sigret_code_start;
	unsigned long vpage;

	vpage = get_zeroed_page(GFP_ATOMIC);

	if (!vpage)
		return -ENOMEM;

	/* kuser helpers */
	memcpy((void *)vpage + 0x1000 - kuser_sz, __kuser_helper_start,
		kuser_sz);

	/* sigreturn code */
	memcpy((void *)vpage + AARCH32_KERN_SIGRET_CODE_OFFSET,
               __aarch32_sigret_code_start, sigret_sz);

	flush_icache_range(vpage, vpage + PAGE_SIZE);
	vectors_page[0] = virt_to_page(vpage);

	return 0;
}
arch_initcall(alloc_vectors_page);

int aarch32_setup_vectors_page(struct linux_binprm *bprm, int uses_interp)
{
	struct mm_struct *mm = current->mm;
	unsigned long addr = AARCH32_VECTORS_BASE;
	int ret;

	down_write(&mm->mmap_sem);
	current->mm->context.vdso = (void *)addr;

	/* Map vectors page at the high address. */
	ret = install_special_mapping(mm, addr, PAGE_SIZE,
				      VM_READ|VM_EXEC|VM_MAYREAD|VM_MAYEXEC,
				      vectors_page);

	up_write(&mm->mmap_sem);

	return ret;
}
#endif /* CONFIG_AARCH32_EL0 */

static inline int __init vdso_init_common(char *vdso_start, char *vdso_end,
					  unsigned long *vdso_pagesp,
					  struct page ***vdso_pagelistp)
{
	struct page *pg;
	char *vbase;
	int i, ret = 0;
	unsigned long vdso_pages;
	struct page **vdso_pagelist;

	vdso_pages = (vdso_end - vdso_start) >> PAGE_SHIFT;
	*vdso_pagesp = vdso_pages;
	pr_info("vdso: %ld pages (%ld code, %ld data) at base %p\n",
		vdso_pages + 1, vdso_pages, 1L, vdso_start);

	/* Allocate the vDSO pagelist, plus a page for the data. */
	vdso_pagelist = kzalloc(sizeof(struct page *) * (vdso_pages + 1),
				GFP_KERNEL);
	*vdso_pagelistp = vdso_pagelist;
	if (vdso_pagelist == NULL) {
		pr_err("Failed to allocate vDSO pagelist!\n");
		return -ENOMEM;
	}

	/* Grab the vDSO code pages. */
	for (i = 0; i < vdso_pages; i++) {
		pg = virt_to_page(vdso_start + i*PAGE_SIZE);
		ClearPageReserved(pg);
		get_page(pg);
		vdso_pagelist[i] = pg;
	}

	/* Sanity check the shared object header. */
	vbase = vmap(vdso_pagelist, 1, 0, PAGE_KERNEL);
	if (vbase == NULL) {
		pr_err("Failed to map vDSO pagelist!\n");
		return -ENOMEM;
	} else if (memcmp(vbase, "\177ELF", 4)) {
		pr_err("vDSO is not a valid ELF object!\n");
		ret = -EINVAL;
		goto unmap;
	}

	/* Grab the vDSO data page. */
	pg = virt_to_page(vdso_data);
	get_page(pg);
	vdso_pagelist[i] = pg;

unmap:
	vunmap(vbase);
	return ret;
}

static int __init vdso_init(void)
{
	return vdso_init_common(&vdso_start, &vdso_end,
				&vdso_pages, &vdso_pagelist);
}
arch_initcall(vdso_init);

#ifdef CONFIG_ARM64_ILP32
static int __init vdso_ilp32_init(void)
{
	return vdso_init_common(&vdso_ilp32_start, &vdso_ilp32_end,
				&vdso_ilp32_pages, &vdso_ilp32_pagelist);
}
arch_initcall(vdso_ilp32_init);
#endif

int arch_setup_additional_pages(struct linux_binprm *bprm,
				int uses_interp)
{
	struct mm_struct *mm = current->mm;
	unsigned long vdso_base, vdso_mapping_len;
	unsigned long popcorn_addr, tmp;
	int ret;
	struct page **pagelist;
	struct page **popcorn_pagelist;
	unsigned long pages;	

	//Ajith - disabling vdso
	//return 0;

	/* Be sure to map the data page */
#ifdef CONFIG_ARM64_ILP32
	if (is_ilp32_compat_task()) {
		pages = vdso_ilp32_pages;
		pagelist = vdso_ilp32_pagelist;
	} else
#endif
	{
		pages = vdso_pages;
		pagelist = vdso_pagelist;
	}
	vdso_mapping_len = (pages + 1) << PAGE_SHIFT;

	down_write(&mm->mmap_sem);
	tmp = popcorn_addr = vdso_base = get_unmapped_area(NULL, 0, (vdso_mapping_len + PAGE_SIZE), 0, 0);
	if (IS_ERR_VALUE(vdso_base)) {
		ret = vdso_base;
		goto up_fail;
	}
	mm->context.vdso = (void *)vdso_base;

	ret = install_special_mapping(mm, vdso_base, vdso_mapping_len,
				      VM_READ|VM_EXEC|
				      VM_MAYREAD|VM_MAYWRITE|VM_MAYEXEC,
				       pagelist);
	if (ret) {
		mm->context.vdso = NULL;
		goto up_fail;
	}
///////////////////////////////////////////////////////////////////////////////
        popcorn_addr = get_unmapped_area(NULL, popcorn_addr, PAGE_SIZE, 0, 0);
        if (IS_ERR_VALUE(popcorn_addr)) {
                pr_err("Failed to get unmapped area!\n");
                ret = popcorn_addr;
                goto up_fail;
        }
        current->mm->context.popcorn_vdso = (void *)popcorn_addr;

        popcorn_pagelist = kzalloc(sizeof(struct page *) * (1 + 1), GFP_KERNEL);
        if (vdso_pagelist == NULL) {
                 pr_err("Failed to allocate vDSO pagelist!\n");
		ret = -ENOMEM;
		goto up_fail;
        }
        popcorn_pagelist[0] = alloc_pages(GFP_KERNEL, 0);

        ret = install_special_mapping(mm, popcorn_addr, PAGE_SIZE,
                                      VM_READ|VM_EXEC|
                                      VM_MAYREAD|VM_MAYWRITE|VM_MAYEXEC,
                                      popcorn_pagelist);
        if (ret) {
                current->mm->context.popcorn_vdso = NULL;
                goto up_fail;
        }

        /*printk(KERN_INFO"%s: current 0x%lx (%s) stack 0x%lx"
               " addr 0x%lx (0x%lx) -0x%lx- size 0x%lx (0x%lx) ret 0x%lx (tmp %lx)\n",
                __func__, (unsigned long)current, current->comm,
                (unsigned long)mm->start_stack,
                (unsigned long)vdso_base, (unsigned long) popcorn_addr, (unsigned long)(vdso_base+vdso_mapping_len),
                (unsigned long)vdso_mapping_len, (unsigned long) PAGE_SIZE,
                (unsigned long)ret, tmp);*/

up_fail:
	up_write(&mm->mmap_sem);
	return ret;
}

const char *arch_vma_name(struct vm_area_struct *vma)
{
	/*
	 * We can re-use the vdso pointer in mm_context_t for identifying
	 * the vectors page for compat applications. The vDSO will always
	 * sit above TASK_UNMAPPED_BASE and so we don't need to worry about
	 * it conflicting with the vectors base.
	 */
	if (vma->vm_mm && vma->vm_start == (long)vma->vm_mm->context.vdso) {
#ifdef CONFIG_AARCH32_EL0
		if (vma->vm_start == AARCH32_VECTORS_BASE)
			return "[vectors]";
#endif
		return "[vdso]";
	}
	if (vma->vm_mm && vma->vm_start == (long)vma->vm_mm->context.popcorn_vdso) {
		return "[popcorn]";
	}

	return NULL;
}

/*
 * We define AT_SYSINFO_EHDR, so we need these function stubs to keep
 * Linux happy.
 */
int in_gate_area_no_mm(unsigned long addr)
{
	return 0;
}

int in_gate_area(struct mm_struct *mm, unsigned long addr)
{
	return 0;
}

struct vm_area_struct *get_gate_vma(struct mm_struct *mm)
{
	return NULL;
}

/*
 * Update the vDSO data page to keep in sync with kernel timekeeping.
 */
void update_vsyscall(struct timekeeper *tk)
{
	struct timespec xtime_coarse;
	u32 use_syscall = strcmp(tk->clock->name, "arch_sys_counter");

	++vdso_data->tb_seq_count;
	smp_wmb();

	xtime_coarse = __current_kernel_time();
	vdso_data->use_syscall			= use_syscall;
	vdso_data->xtime_coarse_sec		= xtime_coarse.tv_sec;
	vdso_data->xtime_coarse_nsec		= xtime_coarse.tv_nsec;

	if (!use_syscall) {
		vdso_data->cs_cycle_last	= tk->clock->cycle_last;
		vdso_data->xtime_clock_sec	= tk->xtime_sec;
		vdso_data->xtime_clock_nsec	= tk->xtime_nsec;
		vdso_data->cs_mult		= tk->mult;
		vdso_data->cs_shift		= tk->shift;
		vdso_data->wtm_clock_sec	= tk->wall_to_monotonic.tv_sec;
		vdso_data->wtm_clock_nsec	= tk->wall_to_monotonic.tv_nsec;
	}

	smp_wmb();
	++vdso_data->tb_seq_count;
}

void update_vsyscall_tz(void)
{
	vdso_data->tz_minuteswest	= sys_tz.tz_minuteswest;
	vdso_data->tz_dsttime		= sys_tz.tz_dsttime;
}

void update_popcorn_migrate(int migrate)
{
	vdso_data->popcorn_migrate = migrate;
}
