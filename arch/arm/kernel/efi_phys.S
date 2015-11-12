/*
 * arch/arm/kernel/efi_phys.S
 *
 * Copyright (C) 2013  Linaro Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/linkage.h>
#define PAR_MASK 0xfff

	.text
@ efi_phys_call(a, b, c, d, *f)
	.align  5
        .pushsection    .idmap.text, "ax"
ENTRY(efi_phys_call)
	@ Save physical context
	mov	r12, sp
	push	{r4-r5, r12, lr}

	@ Extract function pointer (don't write r12 beyond this)
	ldr	r12, [sp, #16]

	@ Convert sp to 32-bit physical
	mov	lr, sp
	ldr	r4, =PAR_MASK
	and	r5, lr, r4			@ Extract lower 12 bits of sp
	mcr	p15, 0, lr, c7, c8, 1		@ Write VA -> ATS1CPW
	mrc	p15, 0, lr, c7, c4, 0		@ Physical Address Register
	mvn	r4, r4
	and	lr, lr, r4			@ Clear lower 12 bits of PA
	add	lr, lr, r5			@ Calculate phys sp
	mov	sp, lr				@ Update

	@ Disable MMU
        mrc     p15, 0, lr, c1, c0, 0           @ ctrl register
        bic     lr, lr, #0x1                    @ ...............m
        mcr     p15, 0, lr, c1, c0, 0           @ disable MMU
	isb

	@ Make call
	blx	r12

	pop	{r4-r5, r12, lr}

	@ Enable MMU + Caches
        mrc     p15, 0, r1, c1, c0, 0		@ ctrl register
        orr     r1, r1, #0x1000			@ ...i............
        orr     r1, r1, #0x0005			@ .............c.m
        mcr     p15, 0, r1, c1, c0, 0		@ enable MMU
	isb

	@ Restore virtual sp and return
	mov	sp, r12
	bx	lr
ENDPROC(efi_phys_call)
        .popsection

