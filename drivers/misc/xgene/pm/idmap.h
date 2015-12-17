#ifndef __ASM_IDMAP_H
#define __ASM_IDMAP_H

#include <linux/compiler.h>
#include <asm/pgtable.h>

/* Tag a function as requiring to be executed via an identity mapping. */
#define __idmap __section(.idmap.text) noinline notrace

extern pgd_t *idmap_pgd;

void setup_mm_for_powerdn(struct mm_struct *mm);
void revert_mm_after_wakeup(struct mm_struct *mm);

#endif	/* __ASM_IDMAP_H */
