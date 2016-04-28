/*
 * Set up the VMAs to tell the VM about the vDSO.
 * Copyright 2007 Andi Kleen, SUSE Labs.
 * Subject to the GPL, v.2
 */
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/random.h>
#include <linux/elf.h>
#include <asm/vsyscall.h>
#include <asm/vgtod.h>
#include <asm/proto.h>
#include <asm/vdso.h>
#include <asm/page.h>

unsigned int __read_mostly vdso_enabled = 1;

extern char vdso_start[], vdso_end[];
extern unsigned short vdso_sync_cpuid;

extern struct page *vdso_pages[];
static unsigned vdso_size;

#ifdef CONFIG_X86_X32_ABI
extern char vdsox32_start[], vdsox32_end[];
extern struct page *vdsox32_pages[];
static unsigned vdsox32_size;

static void __init patch_vdsox32(void *vdso, size_t len)
{
	Elf32_Ehdr *hdr = vdso;
	Elf32_Shdr *sechdrs, *alt_sec = 0;
	char *secstrings;
	void *alt_data;
	int i;

	BUG_ON(len < sizeof(Elf32_Ehdr));
	BUG_ON(memcmp(hdr->e_ident, ELFMAG, SELFMAG) != 0);

	sechdrs = (void *)hdr + hdr->e_shoff;
	secstrings = (void *)hdr + sechdrs[hdr->e_shstrndx].sh_offset;

	for (i = 1; i < hdr->e_shnum; i++) {
		Elf32_Shdr *shdr = &sechdrs[i];
		if (!strcmp(secstrings + shdr->sh_name, ".altinstructions")) {
			alt_sec = shdr;
			goto found;
		}
	}

	/* If we get here, it's probably a bug. */
	pr_warning("patch_vdsox32: .altinstructions not found\n");
	return;  /* nothing to patch */

found:
	alt_data = (void *)hdr + alt_sec->sh_offset;
	apply_alternatives(alt_data, alt_data + alt_sec->sh_size);
}
#endif

static void __init patch_vdso64(void *vdso, size_t len)
{
	Elf64_Ehdr *hdr = vdso;
	Elf64_Shdr *sechdrs, *alt_sec = 0;
	char *secstrings;
	void *alt_data;
	int i;

	BUG_ON(len < sizeof(Elf64_Ehdr));
	BUG_ON(memcmp(hdr->e_ident, ELFMAG, SELFMAG) != 0);

	sechdrs = (void *)hdr + hdr->e_shoff;
	secstrings = (void *)hdr + sechdrs[hdr->e_shstrndx].sh_offset;

	for (i = 1; i < hdr->e_shnum; i++) {
		Elf64_Shdr *shdr = &sechdrs[i];
		if (!strcmp(secstrings + shdr->sh_name, ".altinstructions")) {
			alt_sec = shdr;
			goto found;
		}
	}

	/* If we get here, it's probably a bug. */
	pr_warning("patch_vdso64: .altinstructions not found\n");
	return;  /* nothing to patch */

found:
	alt_data = (void *)hdr + alt_sec->sh_offset;
	apply_alternatives(alt_data, alt_data + alt_sec->sh_size);
}

static int __init init_vdso(void)
{
	int npages = (vdso_end - vdso_start + PAGE_SIZE - 1) / PAGE_SIZE;
	int i;

	patch_vdso64(vdso_start, vdso_end - vdso_start);

	vdso_size = npages << PAGE_SHIFT;
	for (i = 0; i < npages; i++)
		vdso_pages[i] = virt_to_page(vdso_start + i*PAGE_SIZE);

#ifdef CONFIG_X86_X32_ABI
	patch_vdsox32(vdsox32_start, vdsox32_end - vdsox32_start);
	npages = (vdsox32_end - vdsox32_start + PAGE_SIZE - 1) / PAGE_SIZE;
	vdsox32_size = npages << PAGE_SHIFT;
	for (i = 0; i < npages; i++)
		vdsox32_pages[i] = virt_to_page(vdsox32_start + i*PAGE_SIZE);
#endif

	return 0;
}
subsys_initcall(init_vdso);

struct linux_binprm;

/* Put the vdso above the (randomized) stack with another randomized offset.
   This way there is no hole in the middle of address space.
   To save memory make sure it is still in the same PTE as the stack top.
   This doesn't give that many random bits */
static unsigned long vdso_addr(unsigned long start, unsigned len)
{
	unsigned long addr, end;
	unsigned offset;
	end = (start + PMD_SIZE - 1) & PMD_MASK;
	if (end >= TASK_SIZE_MAX)
		end = TASK_SIZE_MAX;
	end -= len;
	/* This loses some more bits than a modulo, but is cheaper */
	offset = get_random_int() & (PTRS_PER_PTE - 1);
	addr = start + (offset << PAGE_SHIFT);
	if (addr >= end)
		addr = end;

	/*
	 * page-align it here so that get_unmapped_area doesn't
	 * align it wrongfully again to the next page. addr can come in 4K
	 * unaligned here as a result of stack start randomization.
	 */
	addr = PAGE_ALIGN(addr);
	addr = align_vdso_addr(addr);

	return addr;
}

/* Setup a VMA at program startup for the vsyscall page.
   Not called for compat tasks */
static int setup_additional_pages(struct linux_binprm *bprm,
				  int uses_interp,
				  struct page **pages,
				  unsigned size)
{
	struct mm_struct *mm = current->mm;
	unsigned long addr, popcorn_addr, tmp;
	int ret;
	struct page ** popcorn_pagelist;

	if (!vdso_enabled)
		return 0;

	down_write(&mm->mmap_sem);
	tmp = popcorn_addr = addr = vdso_addr(mm->start_stack, (size + PAGE_SIZE));

        addr = get_unmapped_area(NULL, addr, size, 0, 0);
	if (IS_ERR_VALUE(addr)) {
		ret = addr;
		goto up_fail;
	}
	current->mm->context.vdso = (void *)addr;

	ret = install_special_mapping(mm, addr, size,
				      VM_READ|VM_EXEC|
				      VM_MAYREAD|VM_MAYWRITE|VM_MAYEXEC,
				      pages);
	if (ret) {
		current->mm->context.vdso = NULL;
		goto up_fail;
	}

///////////////////////////////////////////////////////////////////////////////
// should create two functions
///////////////////////////////////////////////////////////////////////////////
        popcorn_addr = get_unmapped_area(NULL, popcorn_addr, PAGE_SIZE, 0, 0);
	if (IS_ERR_VALUE(popcorn_addr)) {
                pr_err("Failed to get unmapped area!\n");
                ret = popcorn_addr;
                goto up_fail;
        }
        current->mm->context.popcorn_vdso = (void *)popcorn_addr;

	popcorn_pagelist = kzalloc(sizeof(struct page *) * (1 + 1), GFP_KERNEL);
        if (popcorn_pagelist == NULL) {
                 pr_err("Failed to allocate vDSO pagelist!\n");
                 ret = -ENOMEM;
		goto up_fail;
        }         
	popcorn_pagelist[0] = alloc_pages(GFP_KERNEL | __GFP_ZERO, 0);
         
	ret = install_special_mapping(mm, popcorn_addr, PAGE_SIZE,
                                      VM_READ|VM_EXEC|
                                      VM_MAYREAD|VM_MAYWRITE|VM_MAYEXEC,
                                      popcorn_pagelist);
        if (ret) {
                current->mm->context.popcorn_vdso = NULL;
                goto up_fail;
        }
        // NOTE popcorn_pagelist shouldn't be freed because saved in
        // vma->vm_private_data

        /*printk(KERN_INFO"%s: current 0x%lx (%s) stack 0x%lx"
               " addr 0x%lx (0x%lx) -0x%lx- size 0x%lx (0x%lx) ret 0x%lx (tmp %lx)\n",
                __func__, (unsigned long)current, current->comm, 
                (unsigned long)mm->start_stack,
                (unsigned long)addr, (unsigned long) popcorn_addr, (unsigned long)(addr+size),
                (unsigned long)size, (unsigned long) PAGE_SIZE,
                (unsigned long)ret, tmp);*/

up_fail:
	up_write(&mm->mmap_sem);
	return ret;
}

int arch_setup_additional_pages(struct linux_binprm *bprm, int uses_interp)
{
	return setup_additional_pages(bprm, uses_interp, vdso_pages,
				      vdso_size);
}

#ifdef CONFIG_X86_X32_ABI
int x32_setup_additional_pages(struct linux_binprm *bprm, int uses_interp)
{
	return setup_additional_pages(bprm, uses_interp, vdsox32_pages,
				      vdsox32_size);
}
#endif

static __init int vdso_setup(char *s)
{
	vdso_enabled = simple_strtoul(s, NULL, 0);
	return 0;
}
__setup("vdso=", vdso_setup);
