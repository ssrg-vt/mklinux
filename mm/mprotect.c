/*
 *  mm/mprotect.c
 *
 *  (C) Copyright 1994 Linus Torvalds
 *  (C) Copyright 2002 Christoph Hellwig
 *
 *  Address space accounting code	<alan@lxorguk.ukuu.org.uk>
 *  (C) Copyright 2002 Red Hat Inc, All Rights Reserved
 */

#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/shm.h>
#include <linux/mman.h>
#include <linux/fs.h>
#include <linux/highmem.h>
#include <linux/security.h>
#include <linux/mempolicy.h>
#include <linux/personality.h>
#include <linux/syscalls.h>
#include <linux/swap.h>
#include <linux/swapops.h>
#include <linux/mmu_notifier.h>
#include <linux/migrate.h>
#include <linux/perf_event.h>
#include <linux/rmap.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

//Multikernel
#include <popcorn/process_server.h>
#include <popcorn/vma_server.h>

#ifndef pgprot_modify
static inline pgprot_t pgprot_modify(pgprot_t oldprot, pgprot_t newprot)
{
	return newprot;
}
#endif

static unsigned long change_pte_range(struct vm_area_struct *vma, pmd_t *pmd,
		unsigned long addr, unsigned long end, pgprot_t newprot,
		int dirty_accountable, int prot_numa, bool *ret_all_same_node)
{
	struct mm_struct *mm = vma->vm_mm;
	pte_t *pte, oldpte;
	spinlock_t *ptl;
	unsigned long pages = 0;
	bool all_same_node = true;
	int last_nid = -1;

	pte = pte_offset_map_lock(mm, pmd, addr, &ptl);
	arch_enter_lazy_mmu_mode();
	do {
		oldpte = *pte;
		if (pte_present(oldpte)) {
			pte_t ptent;
			bool updated = false;
			int clear= 0;
			//Multikernel
			if(current->tgroup_distributed==1){

				//case pte_present: or REPLICATION_STATUS_NOT_REPLICATED or
				// REPLICATION_STATUS_VALID or REPLICATION_STATUS_WRITTEN

				struct page *page= pte_page(oldpte);

				if (!is_zero_page(pte_pfn(oldpte))) {

					if (page->status == REPLICATION_STATUS_INVALID) {
						printk("ERROR: mprotect moving "
								"a present page that is in invalid state\n");
					}

					if (!(pgprot_val(newprot) & PROT_WRITE)) { //it is becoming a read only vma

						if (page->replicated == 1) {
							printk("mprot: changing to read only address %lu\n",addr);
							if (page->status == REPLICATION_STATUS_NOT_REPLICATED) {
								printk("ERROR:  page replicated is 1 "
										"but in state not replicated\n");
							}

							page->replicated = 0;

							page->status = REPLICATION_STATUS_NOT_REPLICATED;
						}

					} else { // it is becoming a writable vma
						if (page->replicated == 0) {
							int i,count=0;
							printk("mprot: changing to read write address %lu\n",addr);
							if (page->status != REPLICATION_STATUS_NOT_REPLICATED) {
								printk("ERROR: page replicated is zero "
										"but not in state not replicated\n");
							}
							//check if somebody else fetched the page
							for (i = 0; i < MAX_KERNEL_IDS; i++) {
								count=count+page->other_owners[i];
							}
							if(count>1){
								page->replicated = 1;
								page->status = REPLICATION_STATUS_VALID;
								clear=1;
							}
						}else
						{
							//in case valid I enforce a clear
							if (page->status == REPLICATION_STATUS_VALID) {
								clear=1;
							}
						}

					}
				}
			}

			ptent = ptep_modify_prot_start(mm, addr, pte);
			if (!prot_numa) {
				ptent = pte_modify(ptent, newprot);
				updated = true;
			} else {
				struct page *page;

				page = vm_normal_page(vma, addr, oldpte);
				if (page) {
					int this_nid = page_to_nid(page);
					if (last_nid == -1)
						last_nid = this_nid;
					if (last_nid != this_nid)
						all_same_node = false;

					/* only check non-shared pages */
					if (!pte_numa(oldpte) &&
							page_mapcount(page) == 1) {
						ptent = pte_mknuma(ptent);
						updated = true;
					}
				}
			}

			if(clear)
				ptent = pte_wrprotect(ptent);
			else{
				/*
				 * Avoid taking write faults for pages we know to be
				 * dirty.
				 */
				if (dirty_accountable && pte_dirty(ptent)) {
					ptent = pte_mkwrite(ptent);
					updated = true;
				}
			}

			if (updated)
				pages++;
			ptep_modify_prot_commit(mm, addr, pte, ptent);
		} else
			if(current->tgroup_distributed == 1){
				//Ajith - Removing optimization used for local fetch - _PAGE_UNUSED1 case
				if(!( pte==NULL || pte_none(oldpte))){

					struct page *page= pte_page(oldpte);

					if(page->replicated!=1 || page->status!=REPLICATION_STATUS_INVALID){
						printk("ERROR: mprotect moving a not present page that is not in invalid state or replicated\n");
					}

					if( !(pgprot_val(newprot) & PROT_WRITE)) { //it is becoming a read only vma
                                                int i;
						int rss[NR_MM_COUNTERS];
                                                memset(rss, 0, sizeof(int) * NR_MM_COUNTERS);

						printk("mprot: removing page address %lu\n",addr);
						//force a new fetch
						if (PageAnon(page))
							rss[MM_ANONPAGES]--;
						else {
							rss[MM_FILEPAGES]--;
						}

						page_remove_rmap(page);

						page->replicated= 0;
						page->status= REPLICATION_STATUS_NOT_REPLICATED;

						//add if for 2 kernels
						if (cpumask_first(cpu_present_mask)==current->tgroup_home_cpu) {
							pte_t ptent= *pte;
							ptep_get_and_clear(mm, addr, pte);
							if(!pte_none(ptent))
								printk("ERROR: mprot cleaning pte but after not none\n");
							
							//Ajith - Removing optimization used for local fetch - _PAGE_UNUSED1 case
							//ptent = pte_set_flags(ptent, _PAGE_UNUSED1);

							set_pte_at_notify(mm, addr, pte, ptent);
						}

						if (current->mm == mm)
							// Ported to Linux 3.12 API
							sync_mm_rss( mm);

						for (i = 0; i < NR_MM_COUNTERS; i++)
							if (rss[i])
								atomic_long_add(rss[i], &mm->rss_stat.count[i]);
					}
				}
			}
			else{
				// Ported to Linux 3.12 
				#if defined (PAGE_MIGRATION) 
				if (PAGE_MIGRATION && !pte_file(oldpte)) {
					swp_entry_t entry = pte_to_swp_entry(oldpte);

					if (is_write_migration_entry(entry)) {
						/*
						 * A protection check is difficult so
						 * just be safe and disable write
						 */
						make_migration_entry_read(&entry);
						set_pte_at(mm, addr, pte,
								swp_entry_to_pte(entry));
					}
				}
				#endif
			}
		pages++;
	} while (pte++, addr += PAGE_SIZE, addr != end);
	arch_leave_lazy_mmu_mode();
	pte_unmap_unlock(pte - 1, ptl);

	*ret_all_same_node = all_same_node;
	return pages;
}

#ifdef CONFIG_NUMA_BALANCING
static inline void change_pmd_protnuma(struct mm_struct *mm, unsigned long addr,
		pmd_t *pmd)
{
	spin_lock(&mm->page_table_lock);
	set_pmd_at(mm, addr & PMD_MASK, pmd, pmd_mknuma(*pmd));
	spin_unlock(&mm->page_table_lock);
}
#else
static inline void change_pmd_protnuma(struct mm_struct *mm, unsigned long addr,
		pmd_t *pmd)
{
	BUG();
}
#endif /* CONFIG_NUMA_BALANCING */

static inline unsigned long change_pmd_range(struct vm_area_struct *vma,
		pud_t *pud, unsigned long addr, unsigned long end,
		pgprot_t newprot, int dirty_accountable, int prot_numa)
{
	pmd_t *pmd;
	unsigned long next;
	unsigned long pages = 0;
	bool all_same_node;

	pmd = pmd_offset(pud, addr);
	do {
		next = pmd_addr_end(addr, end);
		if (pmd_trans_huge(*pmd)) {
			if (next - addr != HPAGE_PMD_SIZE)
				split_huge_page_pmd(vma, addr, pmd);
			else if (change_huge_pmd(vma, pmd, addr, newprot,
						prot_numa)) {
				pages++;
				continue;
			}
			/* fall through */
		}
		if (pmd_none_or_clear_bad(pmd))
			continue;
		pages += change_pte_range(vma, pmd, addr, next, newprot,
				dirty_accountable, prot_numa, &all_same_node);

		/*
		 * If we are changing protections for NUMA hinting faults then
		 * set pmd_numa if the examined pages were all on the same
		 * node. This allows a regular PMD to be handled as one fault
		 * and effectively batches the taking of the PTL
		 */
		if (prot_numa && all_same_node)
			change_pmd_protnuma(vma->vm_mm, addr, pmd);
	} while (pmd++, addr = next, addr != end);

	return pages;
}

static inline unsigned long change_pud_range(struct vm_area_struct *vma,
		pgd_t *pgd, unsigned long addr, unsigned long end,
		pgprot_t newprot, int dirty_accountable, int prot_numa)
{
	pud_t *pud;
	unsigned long next;
	unsigned long pages = 0;

	pud = pud_offset(pgd, addr);
	do {
		next = pud_addr_end(addr, end);
		if (pud_none_or_clear_bad(pud))
			continue;
		pages += change_pmd_range(vma, pud, addr, next, newprot,
				dirty_accountable, prot_numa);
	} while (pud++, addr = next, addr != end);

	return pages;
}

static unsigned long change_protection_range(struct vm_area_struct *vma,
		unsigned long addr, unsigned long end, pgprot_t newprot,
		int dirty_accountable, int prot_numa)
{
	struct mm_struct *mm = vma->vm_mm;
	pgd_t *pgd;
	unsigned long next;
	unsigned long start = addr;
	unsigned long pages = 0;

	BUG_ON(addr >= end);
	pgd = pgd_offset(mm, addr);
	flush_cache_range(vma, addr, end);
	do {
		next = pgd_addr_end(addr, end);
		if (pgd_none_or_clear_bad(pgd))
			continue;
		pages += change_pud_range(vma, pgd, addr, next, newprot,
				dirty_accountable, prot_numa);
	} while (pgd++, addr = next, addr != end);

	/* Only flush the TLB if we actually modified any entries: */
	if (pages)
		flush_tlb_range(vma, start, end);

	return pages;
}

unsigned long change_protection(struct vm_area_struct *vma, unsigned long start,
		unsigned long end, pgprot_t newprot,
		int dirty_accountable, int prot_numa)
{
	struct mm_struct *mm = vma->vm_mm;
	unsigned long pages;

	mmu_notifier_invalidate_range_start(mm, start, end);
	if (is_vm_hugetlb_page(vma))
		pages = hugetlb_change_protection(vma, start, end, newprot);
	else
		pages = change_protection_range(vma, start, end, newprot, dirty_accountable, prot_numa);
	mmu_notifier_invalidate_range_end(mm, start, end);

	return pages;
}

	int
mprotect_fixup(struct vm_area_struct *vma, struct vm_area_struct **pprev,
		unsigned long start, unsigned long end, unsigned long newflags)
{
	struct mm_struct *mm = vma->vm_mm;
	unsigned long oldflags = vma->vm_flags;
	long nrpages = (end - start) >> PAGE_SHIFT;
	unsigned long charged = 0;
	pgoff_t pgoff;
	int error;
	int dirty_accountable = 0;

	if (newflags == oldflags) {
		*pprev = vma;
		return 0;
	}

	/*
	 * If we make a private mapping writable we increase our commit;
	 * but (without finer accounting) cannot reduce our commit if we
	 * make it unwritable again. hugetlb mapping were accounted for
	 * even if read-only so there is no need to account for them here
	 */
	if (newflags & VM_WRITE) {
		if (!(oldflags & (VM_ACCOUNT|VM_WRITE|VM_HUGETLB|
						VM_SHARED|VM_NORESERVE))) {
			charged = nrpages;
			if (security_vm_enough_memory_mm(mm, charged))
				return -ENOMEM;
			newflags |= VM_ACCOUNT;
		}
	}

	/*
	 * First try to merge with previous and/or next vma.
	 */
	pgoff = vma->vm_pgoff + ((start - vma->vm_start) >> PAGE_SHIFT);
	*pprev = vma_merge(mm, *pprev, start, end, newflags,
			vma->anon_vma, vma->vm_file, pgoff, vma_policy(vma));
	if (*pprev) {
		vma = *pprev;
		goto success;
	}

	*pprev = vma;

	if (start != vma->vm_start) {
		error = split_vma(mm, vma, start, 1);
		if (error)
			goto fail;
	}

	if (end != vma->vm_end) {
		error = split_vma(mm, vma, end, 0);
		if (error)
			goto fail;
	}

success:
	/*
	 * vm_flags and vm_page_prot are protected by the mmap_sem
	 * held in write mode.
	 */
	vma->vm_flags = newflags;
	vma->vm_page_prot = pgprot_modify(vma->vm_page_prot,
			vm_get_page_prot(newflags));

	if (vma_wants_writenotify(vma)) {
		vma->vm_page_prot = vm_get_page_prot(newflags & ~VM_SHARED);
		dirty_accountable = 1;
	}

	change_protection(vma, start, end, vma->vm_page_prot,
			dirty_accountable, 0);

	vm_stat_account(mm, oldflags, vma->vm_file, -nrpages);
	vm_stat_account(mm, newflags, vma->vm_file, nrpages);
	perf_event_mmap(vma);
	return 0;

fail:
	vm_unacct_memory(charged);
	return error;
}

int kernel_mprotect(unsigned long start, size_t len,
		unsigned long prot){

	unsigned long vm_flags, nstart, end, tmp, reqprot;
	struct vm_area_struct *vma, *prev;
	int error = -EINVAL, distributed= 0;
	const int grows = prot & (PROT_GROWSDOWN|PROT_GROWSUP);
	long distr_ret;

	prot &= ~(PROT_GROWSDOWN|PROT_GROWSUP);
	if (grows == (PROT_GROWSDOWN|PROT_GROWSUP)) /* can't be both */
		return -EINVAL;

	if (start & ~PAGE_MASK)
		return -EINVAL;
	if (!len)
		return 0;
	len = PAGE_ALIGN(len);
	end = start + len;
	if (end <= start)
		return -ENOMEM;
	if (!arch_validate_prot(prot))
		return -EINVAL;

	reqprot = prot;
	/*
	 * Does the application expect PROT_READ to imply PROT_EXEC:
	 */
	if ((prot & PROT_READ) && (current->personality & READ_IMPLIES_EXEC))
		prot |= PROT_EXEC;

	vm_flags = calc_vm_prot_bits(prot);

	down_write(&current->mm->mmap_sem);

	//Multikernel
	if(current->tgroup_distributed==1 && current->distributed_exit == EXIT_ALIVE){
		distributed= 1;
		//printk("WARNING: mprotect called \n");
		distr_ret= vma_server_mprotect_start( start, len, prot);
		if(distr_ret<0 && distr_ret!=VMA_OP_SAVE && distr_ret!=VMA_OP_NOT_SAVE){
			up_write(&current->mm->mmap_sem);
			return distr_ret;
		}
	}

	vma = find_vma_prev(current->mm, start, &prev);
	error = -ENOMEM;
	if (!vma)
		goto out;
	if (unlikely(grows & PROT_GROWSDOWN)) {
		if (vma->vm_start >= end)
			goto out;
		start = vma->vm_start;
		error = -EINVAL;
		if (!(vma->vm_flags & VM_GROWSDOWN))
			goto out;
	}
	else {
		if (vma->vm_start > start)
			goto out;
		if (unlikely(grows & PROT_GROWSUP)) {
			end = vma->vm_end;
			error = -EINVAL;
			if (!(vma->vm_flags & VM_GROWSUP))
				goto out;
		}
	}
	if (start > vma->vm_start)
		prev = vma;

	for (nstart = start ; ; ) {
		unsigned long newflags;

		/* Here we know that  vma->vm_start <= nstart < vma->vm_end. */

		newflags = vm_flags | (vma->vm_flags & ~(VM_READ | VM_WRITE | VM_EXEC));

		/* newflags >> 4 shift VM_MAY% in place of VM_% */
		if ((newflags & ~(newflags >> 4)) & (VM_READ | VM_WRITE | VM_EXEC)) {
			error = -EACCES;
			goto out;
		}

		error = security_file_mprotect(vma, reqprot, prot);
		if (error)
			goto out;

		tmp = vma->vm_end;
		if (tmp > end)
			tmp = end;
		error = mprotect_fixup(vma, &prev, nstart, tmp, newflags);
		if (error)
			goto out;
		nstart = tmp;

		if (nstart < prev->vm_end)
			nstart = prev->vm_end;
		if (nstart >= end)
			goto out;

		vma = prev->vm_next;
		if (!vma || vma->vm_start != nstart) {
			error = -ENOMEM;
			goto out;
		}
	}
out:

	//Multikernel
	if(current->tgroup_distributed==1 && distributed == 1){
		vma_server_mprotect_end(start,len,prot,distr_ret);
	}

	up_write(&current->mm->mmap_sem);
	return error;

}

SYSCALL_DEFINE3(mprotect, unsigned long, start, size_t, len,
		unsigned long, prot)
{
	unsigned long vm_flags, nstart, end, tmp, reqprot;
	struct vm_area_struct *vma, *prev;
	int error = -EINVAL, distributed= 0;
	const int grows = prot & (PROT_GROWSDOWN|PROT_GROWSUP);
	long distr_ret;

	prot &= ~(PROT_GROWSDOWN|PROT_GROWSUP);
	if (grows == (PROT_GROWSDOWN|PROT_GROWSUP)) /* can't be both */
		return -EINVAL;

	if (start & ~PAGE_MASK)
		return -EINVAL;
	if (!len)
		return 0;
	len = PAGE_ALIGN(len);
	end = start + len;
	if (end <= start)
		return -ENOMEM;
	if (!arch_validate_prot(prot))
		return -EINVAL;

	reqprot = prot;
	/*
	 * Does the application expect PROT_READ to imply PROT_EXEC:
	 */
	if ((prot & PROT_READ) && (current->personality & READ_IMPLIES_EXEC))
		prot |= PROT_EXEC;

	vm_flags = calc_vm_prot_bits(prot);

	down_write(&current->mm->mmap_sem);

	//Multikernel
	if(current->tgroup_distributed==1 && current->distributed_exit == EXIT_ALIVE){
		distributed= 1;
		//printk("WARNING: mprotect called\n");
		distr_ret= vma_server_mprotect_start( start, len, prot);
		if(distr_ret<0 && distr_ret!=VMA_OP_SAVE && distr_ret!=VMA_OP_NOT_SAVE){
			up_write(&current->mm->mmap_sem);
			return distr_ret;
		}
	}

	vma = find_vma(current->mm, start);
	error = -ENOMEM;
	if (!vma)
		goto out;
	prev = vma->vm_prev;
	if (unlikely(grows & PROT_GROWSDOWN)) {
		if (vma->vm_start >= end)
			goto out;
		start = vma->vm_start;
		error = -EINVAL;
		if (!(vma->vm_flags & VM_GROWSDOWN))
			goto out;
	} else {
		if (vma->vm_start > start)
			goto out;
		if (unlikely(grows & PROT_GROWSUP)) {
			end = vma->vm_end;
			error = -EINVAL;
			if (!(vma->vm_flags & VM_GROWSUP))
				goto out;
		}
	}

	/*if(vma && (vma->vm_file != NULL) && strcmp(vma->vm_file,"/bin/is") == 0){
	  printk(KERN_ALERT"%s: vma start{%lx}  end{%lx} \n)",__func__,vma->vm_start,vma->vm_end);	
	  }*/

	if (start > vma->vm_start)
		prev = vma;

	for (nstart = start ; ; ) {
		unsigned long newflags;

		/* Here we know that vma->vm_start <= nstart < vma->vm_end. */

		newflags = vm_flags;
		newflags |= (vma->vm_flags & ~(VM_READ | VM_WRITE | VM_EXEC));

		/* newflags >> 4 shift VM_MAY% in place of VM_% */
		if ((newflags & ~(newflags >> 4)) & (VM_READ | VM_WRITE | VM_EXEC)) {
			error = -EACCES;
			goto out;
		}

		error = security_file_mprotect(vma, reqprot, prot);
		if (error)
			goto out;

		tmp = vma->vm_end;
		if (tmp > end)
			tmp = end;
		error = mprotect_fixup(vma, &prev, nstart, tmp, newflags);
		if (error)
			goto out;
		nstart = tmp;

		if (nstart < prev->vm_end)
			nstart = prev->vm_end;
		if (nstart >= end)
			goto out;

		vma = prev->vm_next;
		if (!vma || vma->vm_start != nstart) {
			error = -ENOMEM;
			goto out;
		}
	}
out:
	//Multikernel
	if(current->tgroup_distributed==1 && distributed == 1){
		vma_server_mprotect_end(start,len,prot,distr_ret);
	}

	up_write(&current->mm->mmap_sem);
	return error;
}
