/**
 * xgene_suspend_asm.S - AppliedMicro APM88xxxx platform code
 *
 * Copyright (c) 2013, Applied Micro Circuits Corporation
 * Author: Narinder Dhillon <ndhillon@apm.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 *
 */

#include <linux/init.h>
#include <linux/linkage.h>
#include <asm/assembler.h>

	.align 5
	.pushsection    .idmap.text, "ax"

save_register:
	.rept	8
	.rept	30
	.long	0
	.long	0
	.endr
	.endr


ENTRY(xgene_cpu_cache_mmu_off)
	mrs	x0, sctlr_el1
	bic	x0, x0, #1 << 2			// clear SCTLR.C
	bic	x0, x0, #1 << 12		// clear SCTLR.I
	bic	x0, x0, #1			// clear SCTLR.M
	msr	sctlr_el1, x0
	dsb	sy
	isb
	ret
ENDPROC(xgene_cpu_cache_mmu_off)

ENTRY(xgene_cpu_cache_mmu_on)
	mrs	x0, sctlr_el1
	orr	x0, x0, #1 << 2			// clear SCTLR.C
	orr	x0, x0, #1 << 12		// clear SCTLR.I
	orr	x0, x0, #1			// clear SCTLR.M
	msr	sctlr_el1, x0
	dsb	sy
	isb
	ret
ENDPROC(xgene_cpu_cache_mmu_on)

ENTRY(xgene_invalidate_icache)
	ic 	iallu
	dsb 	sy
	isb
	ret
ENDPROC(xgene_invalidate_icache)

ENTRY(arch_flush_tlb)
	tlbi 	vmalle1
	dsb 	sy
	isb
	ret
ENDPROC(arch_flush_tlb)

ENTRY(xgene_offline_pmd)
	stp	x3,x4,[x2]
	add	x2,x2,#16
	stp	x5,x6,[x2]
	add	x2,x2,#16
	stp	x7,x8,[x2]
	add	x2,x2,#16
	stp	x9,x10,[x2]
	add	x2,x2,#16
	stp	x11,x12,[x2]
	add	x2,x2,#16
	stp	x13,x14,[x2]
	add	x2,x2,#16
	stp	x15,x16,[x2]
	add	x2,x2,#16
	stp	x17,x18,[x2]
	add	x2,x2,#16
	stp	x19,x20,[x2]
	add	x2,x2,#16
	stp	x21,x22,[x2]
	add	x2,x2,#16
	stp	x23,x24,[x2]
	add	x2,x2,#16
	stp	x25,x26,[x2]
	add	x2,x2,#16
	stp	x27,x28,[x2]
	add	x2,x2,#16
	mov	x15,x2

#	bl	xgene_cpu_cache_mmu_off
	mrs	x2, sctlr_el1
	bic	x2, x2, #1 << 2			// clear SCTLR.C
	bic	x2, x2, #1 << 12		// clear SCTLR.I
	bic	x2, x2, #1			// clear SCTLR.M
	msr	sctlr_el1, x2
	dsb	sy
	isb
#	Invalidate ICACHE
	ic 	iallu
	dsb 	sy
	isb
#	Invalidate TLB
	tlbi 	vmalle1
	dsb 	sy
	isb
#	If CPU1, sleep
	and	x0,x0,#1
	cmp	x0,0
	beq	CPU0
	ldr	w0,[x1]
	dsb	sy				// WFI may enter a low-power mode
	wfi
	b	WAKE
#	If CPU0, clear L2C
CPU0:
	ldr	w0,[x1]
#if 1
#	__flush_dcache_all
	dsb	sy				// ensure ordering with previous memory accesses
	mrs	x0, clidr_el1			// read clidr
	and	x3, x0, #0x7000000		// extract loc from clidr
	lsr	x3, x3, #23			// left align loc bit field
	cbz	x3, finished			// if loc is 0, then no need to clean
	mov	x10, #2				// start clean at cache level 0
loop1:
	add	x2, x10, x10, lsr #1		// work out 3x current cache level
	lsr	x1, x0, x2			// extract cache type bits from clidr
	and	x1, x1, #7			// mask of the bits for current cache only
	cmp	x1, #2				// see what cache we have at this level
	b.lt	skip				// skip if no cache, or just i-cache
#	save_and_disable_irqs x9		// make CSSELR and CCSIDR access atomic
	msr	csselr_el1, x10			// select current cache level in csselr
	isb					// isb to sych the new cssr&csidr
	mrs	x1, ccsidr_el1			// read the new ccsidr
#	restore_irqs x9
	and	x2, x1, #7			// extract the length of the cache lines
	add	x2, x2, #4			// add 4 (line length offset)
	mov	x4, #0x3ff
	and	x4, x4, x1, lsr #3		// find maximum number on the way size
	clz	x5, x4				// find bit position of way size increment
	mov	x7, #0x7fff
	and	x7, x7, x1, lsr #13		// extract max number of the index size
loop2:
	mov	x9, x4				// create working copy of max way size
loop3:
	lsl	x6, x9, x5
	orr	x11, x10, x6			// factor way and cache number into x11
	lsl	x6, x7, x2
	orr	x11, x11, x6			// factor index number into x11
	dc	cisw, x11			// clean & invalidate by set/way
	subs	x9, x9, #1			// decrement the way
	b.ge	loop3
	subs	x7, x7, #1			// decrement the index
	b.ge	loop2
skip:
	add	x10, x10, #2			// increment cache number
	cmp	x3, x10
	b.gt	loop1
finished:
	mov	x10, #0				// swith back to cache level 0
	msr	csselr_el1, x10			// select current cache level in csselr
	dsb	sy
	isb
#endif
	dsb	sy				// WFI may enter a low-power mode
	wfi

WAKE:

#	bl 	xgene_cpu_cache_mmu_on
	mrs	x0, sctlr_el1
	orr	x0, x0, #1 << 2			// set SCTLR.C
	orr	x0, x0, #1 << 12		// set SCTLR.I
	orr	x0, x0, #1			// set SCTLR.M
	msr	sctlr_el1, x0
	dsb	sy
	isb

	mov	x2,x15
	sub	x2,x2,#16
	ldp	x27,x28,[x2]
	sub	x2,x2,#16
	ldp	x25,x26,[x2]
	sub	x2,x2,#16
	ldp	x23,x24,[x2]
	sub	x2,x2,#16
	ldp	x21,x22,[x2]
	sub	x2,x2,#16
	ldp	x19,x20,[x2]
	sub	x2,x2,#16
	ldp	x17,x18,[x2]
	sub	x2,x2,#16
	ldp	x15,x16,[x2]
	sub	x2,x2,#16
	ldp	x13,x14,[x2]
	sub	x2,x2,#16
	ldp	x11,x12,[x2]
	sub	x2,x2,#16
	ldp	x9,x10,[x2]
	sub	x2,x2,#16
	ldp	x7,x8,[x2]
	sub	x2,x2,#16
	ldp	x5,x6,[x2]
	sub	x2,x2,#16
	ldp	x3,x4,[x2]
	ret
ENDPROC(xgene_offline_pmd)

ENTRY(xgene_offline_core)
	stp	x3,x4,[x0]
	add	x0,x0,#16
	stp	x5,x6,[x0]
	add	x0,x0,#16
	stp	x7,x8,[x0]
	add	x0,x0,#16
	stp	x9,x10,[x0]
	add	x0,x0,#16
	stp	x11,x12,[x0]
	add	x0,x0,#16
	stp	x13,x14,[x0]
	add	x0,x0,#16
	stp	x15,x16,[x0]
	add	x0,x0,#16
	stp	x17,x18,[x0]
	add	x0,x0,#16
	stp	x19,x20,[x0]
	add	x0,x0,#16
	stp	x21,x22,[x0]
	add	x0,x0,#16
	stp	x23,x24,[x0]
	add	x0,x0,#16
	stp	x25,x26,[x0]
	add	x0,x0,#16
	stp	x27,x28,[x0]
	add	x0,x0,#16
	mov	x15,x0

	mrs	x2, sctlr_el1
	bic	x2, x2, #1 << 2			// clear SCTLR.C
	bic	x2, x2, #1 << 12		// clear SCTLR.I
	bic	x2, x2, #1			// clear SCTLR.M
	msr	sctlr_el1, x2
	dsb	sy
	isb
#	Invalidate ICACHE
	ic 	iallu
	dsb 	sy
	isb
#	Invalidate TLB
	tlbi 	vmalle1
	dsb 	sy
	isb
#	__flush_dcache_all
	dsb	sy				// ensure ordering with previous memory accesses
	mrs	x0, clidr_el1			// read clidr
	and	x3, x0, #0x7000000		// extract loc from clidr
	lsr	x3, x3, #23			// left align loc bit field
	cbz	x3, finished1			// if loc is 0, then no need to clean
	mov	x10, #2				// start clean at cache level 0
loop11:
	add	x2, x10, x10, lsr #1		// work out 3x current cache level
	lsr	x1, x0, x2			// extract cache type bits from clidr
	and	x1, x1, #7			// mask of the bits for current cache only
	cmp	x1, #2				// see what cache we have at this level
	b.lt	skip1				// skip if no cache, or just i-cache
#	save_and_disable_irqs x9		// make CSSELR and CCSIDR access atomic
	msr	csselr_el1, x10			// select current cache level in csselr
	isb					// isb to sych the new cssr&csidr
	mrs	x1, ccsidr_el1			// read the new ccsidr
#	restore_irqs x9
	and	x2, x1, #7			// extract the length of the cache lines
	add	x2, x2, #4			// add 4 (line length offset)
	mov	x4, #0x3ff
	and	x4, x4, x1, lsr #3		// find maximum number on the way size
	clz	x5, x4				// find bit position of way size increment
	mov	x7, #0x7fff
	and	x7, x7, x1, lsr #13		// extract max number of the index size
loop21:
	mov	x9, x4				// create working copy of max way size
loop31:
	lsl	x6, x9, x5
	orr	x11, x10, x6			// factor way and cache number into x11
	lsl	x6, x7, x2
	orr	x11, x11, x6			// factor index number into x11
	dc	cisw, x11			// clean & invalidate by set/way
	subs	x9, x9, #1			// decrement the way
	b.ge	loop31
	subs	x7, x7, #1			// decrement the index
	b.ge	loop21
skip1:
	add	x10, x10, #2			// increment cache number
	cmp	x3, x10
	b.gt	loop11
finished1:
	mov	x10, #0				// swith back to cache level 0
	msr	csselr_el1, x10			// select current cache level in csselr
	dsb	sy
	isb

	dsb	sy				// WFI may enter a low-power mode
	wfi

#	bl 	xgene_cpu_cache_mmu_on
	mrs	x0, sctlr_el1
	orr	x0, x0, #1 << 2			// set SCTLR.C
	orr	x0, x0, #1 << 12		// set SCTLR.I
	orr	x0, x0, #1			// set SCTLR.M
	msr	sctlr_el1, x0
	dsb	sy
	isb

	mov	x0,x15
	sub	x0,x0,#16
	ldp	x27,x28,[x0]
	sub	x0,x0,#16
	ldp	x25,x26,[x0]
	sub	x0,x0,#16
	ldp	x23,x24,[x0]
	sub	x0,x0,#16
	ldp	x21,x22,[x0]
	sub	x0,x0,#16
	ldp	x19,x20,[x0]
	sub	x0,x0,#16
	ldp	x17,x18,[x0]
	sub	x0,x0,#16
	ldp	x15,x16,[x0]
	sub	x0,x0,#16
	ldp	x13,x14,[x0]
	sub	x0,x0,#16
	ldp	x11,x12,[x0]
	sub	x0,x0,#16
	ldp	x9,x10,[x0]
	sub	x0,x0,#16
	ldp	x7,x8,[x0]
	sub	x0,x0,#16
	ldp	x5,x6,[x0]
	sub	x0,x0,#16
	ldp	x3,x4,[x0]
	ret
ENDPROC(xgene_offline_core)

	.popsection









