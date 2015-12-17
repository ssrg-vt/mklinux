/**
 * @cond COPYRIGHT_INFORMATION
 *
 * Copyright (C) 2012 AppliedMicro Confidential Information
 *
 * All Rights Reserved.
 *
 * THIS WORK CONTAINS PROPRIETARY INFORMATION WHICH IS THE PROPERTY OF
 * AppliedMicro AND IS SUBJECT TO THE TERMS OF NON-DISCLOSURE AGREEMENT
 * BETWEEN AppliedMicro AND THE COMPANY USING THIS FILE.
 *
 * @version 1.0
 * @author Narinder Dhillon (ndhillon@apm.com)
 * @endcond
 *
 * @file idmap.c
 * @brief source file for identity mapping of power management code
 *
 * This identity mapping interface supports 4K and 64K page sizes.
 * For ARMv8 cores, there are 3 levels of page tables for 4K page size
 * (PGD, PMD, PTE) and 2 levels of page tables for 64K page size (PGD,PTE).
 * The 3-level tables have 512 entries each and the 2 level page tables have
 * 1024 entries in PGD and 8192 PTE entries.
 */

#include <linux/kernel.h>
#include <asm/cputype.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <asm/sections.h>
#include "idmap.h"

# if 0
#  define debug_map                   printk
# else
#  define debug_map(x, ...)
# endif

pgd_t *idmap_pgd;

static void idmap_add_pte(pmd_t *pmd, unsigned long addr,
				  unsigned long end)
{
	pte_t *pte = 0;

	if (pmd_none(*pmd)) {
		pte = pte_alloc_one_kernel(&init_mm, addr);
		__pmd_populate(pmd, __pa(pte), PMD_TYPE_TABLE);
	}
	BUG_ON(pmd_bad(*pmd));
	debug_map("PTE:0x%llx:0x%llx:0x%llx\n", pte,
			(u64)addr, (u64) end);
	pte = pte_offset_kernel(pmd, addr);
	addr = (addr & (0xFFFFFFFFFFFF & ~(PAGE_SIZE-1)));
	do {
		debug_map("PTE INDEX: 0x%llx (0x%llx)\n", pte, addr);
		set_pte(pte, pfn_pte(((addr>>PAGE_SHIFT)&0xFFFFFFFFF), PAGE_KERNEL_EXEC));
		debug_map("PTE ENTRY: 0x%llx\n", *pte);
	} while (pte++, addr += PAGE_SIZE, addr < end);
}


#ifndef CONFIG_ARM64_64K_PAGES 
static void idmap_add_pmd(pud_t *pud, unsigned long addr, unsigned long end,
	unsigned long prot)
{
	pmd_t *pmd;
	unsigned long next;

	if (pud_none_or_clear_bad(pud)){// || (pud_val(*pud) & L_PGD_SWAPPER)) {
		pmd = pmd_alloc_one(&init_mm, addr);
		if (!pmd) {
			pr_warning("Failed to allocate identity pmd.\n");
			return;
		}
		debug_map("PMD:0x%llx:0x%llx:0x%llx\n", (long long) pmd, (long long )addr, (long long) end);
		pud_populate(&init_mm, pud, pmd);
		pmd += pmd_index(addr);
	} else
		pmd = pmd_offset(pud, addr);

	do {
		debug_map("PMD INDEX: 0x%llx (0x%llx)-(0x%llx)\n", (long long)pmd, (long long)addr, (long long)end);
		next = pmd_addr_end(addr, end);
		idmap_add_pte(pmd, addr, next);
	} while (pmd++, addr = next, addr != end);
}
#else	/* CONFIG_ARM64_64K_PAGES */
static void idmap_add_pmd(pud_t *pud, unsigned long addr, unsigned long end,
	unsigned long prot)
{
	pmd_t *pmd = pmd_offset(pud, addr);
	unsigned long next;
	do {
		next = pmd_addr_end(addr, end);
		debug_map("PMD INDEX: 0x%llx (0x%llx)-(0x%llx)\n", (long long)pmd, (long long)addr, (long long)end);
		idmap_add_pte(pmd, addr, next);
	} while (pmd++, addr = next, addr != end);
}
#endif	/* CONFIG_ARM64_64K_PAGES */

static void idmap_add_pud(pgd_t *pgd, unsigned long addr, unsigned long end,
	unsigned long prot)
{
	pud_t *pud = pud_offset(pgd, addr);
	unsigned long next;

	do {
		next = pud_addr_end(addr, end);
		debug_map("PUD INDEX: 0x%llx (0x%llx)-(0x%llx)\n", pud, addr, end);
		idmap_add_pmd(pud, addr, next, prot);
	} while (pud++, addr = next, addr != end);
}

static void identity_mapping_add(pgd_t *pgd, unsigned long addr, unsigned long end)
{
	unsigned long prot, next;

	prot = PMD_TYPE_SECT | /*PMD_SECT_AP_WRITE |*/ PMD_SECT_AF;
	//if (cpu_architecture() <= CPU_ARCH_ARMv5TEJ && !cpu_is_xscale())
	//	prot |= PMD_BIT4;

	pgd += pgd_index(addr);
	do {
		next = pgd_addr_end(addr, end);
		pr_info("PGD INDEX: 0x%llx (0x%llx)-(0x%llx)\n", (long long)pgd, (long long)addr, (long long)end);
		idmap_add_pud(pgd, addr, next, prot);
	} while (pgd++, addr = next, addr != end);
}

extern char  __idmap_text_start[], __idmap_text_end[];

static int __init init_static_idmap(void)
{
	phys_addr_t idmap_start, idmap_end;

	idmap_pgd = pgd_alloc(&init_mm);
	if (!idmap_pgd)
		return -ENOMEM;

	/* Add an identity mapping for the physical address of the section. */
	idmap_start = virt_to_phys((void *)__idmap_text_start);
	idmap_end = virt_to_phys((void *)__idmap_text_end);

	pr_info("Setting up static identity map for 0x%llx - 0x%llx PGD:0x%llx\n",
		(long long)idmap_start, (long long)idmap_end, (long long)idmap_pgd);
	identity_mapping_add(idmap_pgd, idmap_start, idmap_end);
	/* Flush L1 for the hardware to see this page table content */
	//flush_cache_louis();

	return 0;
}
early_initcall(init_static_idmap);

/*
 * In order to soft-boot, we need to switch to a 1:1 mapping for the
 * cpu_reset functions. This will then ensure that we have predictable
 * results when turning off the mmu.
 */
void setup_mm_for_powerdn(struct mm_struct *mm)
{
	/* Switch to the identity mapping. */
	cpu_switch_mm(idmap_pgd, mm);

#ifdef CONFIG_CPU_HAS_ASID
	/*
	 * We don't have a clean ASID for the identity mapping, which
	 * may clash with virtual addresses of the previous page tables
	 * and therefore potentially in the TLB.
	 */
	local_flush_tlb_all();
#endif
}

void revert_mm_after_wakeup(struct mm_struct *mm)
{
	/* Switch to the identity mapping. */
	cpu_switch_mm(mm->pgd, mm);
}
