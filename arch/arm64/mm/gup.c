/*
 * arch/arm64/mm/gup.c
 *
 * Copyright (C) 2013 Linaro Ltd.
 *
 * Based on arch/powerpc/mm/gup.c which is:
 * Copyright (C) 2008 Nick Piggin
 * Copyright (C) 2008 Novell Inc.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/rwsem.h>
#include <linux/hugetlb.h>
#include <asm/pgtable.h>

static int gup_pte_range(pmd_t pmd, unsigned long addr, unsigned long end,
			 int write, struct page **pages, int *nr)
{
	pte_t *ptep, *ptem;
	int ret = 0;

	ptem = ptep = pte_offset_map(&pmd, addr);
	do {
		pte_t pte = ACCESS_ONCE(*ptep);
		struct page *page;

		if (!pte_present_user(pte) /*|| pte_protnone(pte)*/
			|| (write && !pte_write(pte)))
			goto pte_unmap;

		VM_BUG_ON(!pfn_valid(pte_pfn(pte)));
		page = pte_page(pte);

		if (!page_cache_get_speculative(page))
			goto pte_unmap;

		if (unlikely(pte_val(pte) != pte_val(*ptep))) {
			put_page(page);
			goto pte_unmap;
		}

		pages[*nr] = page;
		(*nr)++;

	} while (ptep++, addr += PAGE_SIZE, addr != end);

	ret = 1;

pte_unmap:
	pte_unmap(ptem);
	return ret;
}

static int gup_huge_pmd(pmd_t orig, pmd_t *pmdp, unsigned long addr,
		unsigned long end, int write, struct page **pages, int *nr)
{
	struct page *head, *page, *tail;
	int refs;

	if (!pmd_present(orig) /*|| pmd_protnone(orig)*/
		|| (write && !pmd_write(orig)))
		return 0;

	refs = 0;
	head = pmd_page(orig);
	page = head + ((addr & ~PMD_MASK) >> PAGE_SHIFT);
	tail = page;
	do {
		VM_BUG_ON(compound_head(page) != head);
		pages[*nr] = page;
		(*nr)++;
		page++;
		refs++;
	} while (addr += PAGE_SIZE, addr != end);

	if (!page_cache_add_speculative(head, refs)) {
		*nr -= refs;
		return 0;
	}

	if (unlikely(pmd_val(orig) != pmd_val(*pmdp))) {
		*nr -= refs;
		while (refs--)
			put_page(head);
		return 0;
	}

	/*
	 * Tail pages have their _mapcount bumped, see
	 * __get_page_tail_foll for more information.
	 */
	while (refs--) {
		if (PageTail(tail))
			get_huge_page_tail(tail);
		tail++;
	}

	return 1;
}

static int gup_pmd_range(pud_t pud, unsigned long addr, unsigned long end,
		int write, struct page **pages, int *nr)
{
	unsigned long next;
	pmd_t *pmdp;

	pmdp = pmd_offset(&pud, addr);
	do {
		pmd_t pmd = ACCESS_ONCE(*pmdp);
		next = pmd_addr_end(addr, end);
		if (pmd_none(pmd) || pmd_trans_splitting(pmd))
			return 0;

		if (unlikely(pmd_huge(pmd) || pmd_trans_huge(pmd))) {
			if (!gup_huge_pmd(pmd, pmdp, addr, next, write,
				pages, nr))
				return 0;
		} else {
			if (!gup_pte_range(pmd, addr, next, write, pages, nr))
				return 0;
		}
	} while (pmdp++, addr = next, addr != end);

	return 1;
}

static int gup_pud_range(pgd_t *pgdp, unsigned long addr, unsigned long end,
		int write, struct page **pages, int *nr)
{
	unsigned long next;
	pud_t *pudp;

	pudp = pud_offset(pgdp, addr);
	do {
		pud_t pud = ACCESS_ONCE(*pudp);
		next = pud_addr_end(addr, end);
		if (pud_none(pud))
			return 0;
		else if (!gup_pmd_range(pud, addr, next, write, pages, nr))
			return 0;
	} while (pudp++, addr = next, addr != end);

	return 1;
}

/*
 * Like get_user_pages_fast() except its IRQ-safe in that it won't fall
 * back to the regular GUP.
 */
int __get_user_pages_fast(unsigned long start, int nr_pages, int write,
			  struct page **pages)
{
	struct mm_struct *mm = current->mm;
	unsigned long addr, len, end;
	unsigned long next;
	pgd_t *pgdp;
	int nr = 0;

	start &= PAGE_MASK;
	addr = start;
	len = (unsigned long) nr_pages << PAGE_SHIFT;
	end = start + len;

	if (unlikely(!access_ok(write ? VERIFY_WRITE : VERIFY_READ,
					start, len)))
		return 0;

	/*
	 * A non-zero gup_readers value will block page table pages
	 * from being freed and also block THP splitting.
	 * This allows us to walk the page tables and pin pages.
	 */
	inc_gup_readers(mm);

	pgdp = pgd_offset(mm, addr);
	do {
		next = pgd_addr_end(addr, end);
		if (pgd_none(*pgdp))
			break;
		else if (!gup_pud_range(pgdp, addr, next, write, pages, &nr))
			break;
	} while (pgdp++, addr = next, addr != end);

	dec_gup_readers(mm);

	return nr;
}

int get_user_pages_fast(unsigned long start, int nr_pages, int write,
			struct page **pages)
{
	struct mm_struct *mm = current->mm;
	int nr, ret;

	start &= PAGE_MASK;
	nr = __get_user_pages_fast(start, nr_pages, write, pages);
	ret = nr;

	if (nr < nr_pages) {
		/* Try to get the remaining pages with get_user_pages */
		start += nr << PAGE_SHIFT;
		pages += nr;

		int lock_aquired= 0;
		//Multikernel
		if(current->tgroup_distributed == 1){

			down_read(&mm->distribute_sem);
			lock_aquired = 1;
		}
		else
			lock_aquired = 0;

		down_read(&mm->mmap_sem);
		ret = get_user_pages(current, mm, start,
				     nr_pages - nr, write, 0, pages, NULL);
		up_read(&mm->mmap_sem);

		if(current->tgroup_distributed==1 && lock_aquired){

			up_read(&mm->distribute_sem);
		}

		/* Have to be a bit careful with return values */
		if (nr > 0) {
			if (ret < 0)
				ret = nr;
			else
				ret += nr;
		}
	}

	return ret;
}
