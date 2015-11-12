/*
 * Copyright (C) 2012 ARM Ltd.
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
 */

#include <linux/linkage.h>
#include <asm/assembler.h>

#define dstin	x0
#define src	x1
#define count	x2
#define tmp1	x3
#define tmp1w	w3
#define tmp2	x4
#define tmp2w	w4
#define tmp3	x5
#define tmp3w	w5
#define dst	x6

#define A_l	x7
#define A_h	x8
#define B_l	x9
#define B_h	x10
#define C_l	x11
#define C_h	x12
#define D_l	x13
#define D_h	x14

	/* Prototype: size_t copy_to_user (void *dst, const void *src, size_t nb)  */
	.text
	.align	2
	.p2align 6,0,63
	.global	__copy_to_user
	.type	__copy_to_user, %function
__copy_to_user:	
	mov	dst, dstin
	cmp	count, #64
	b.ge	.Lcpy_not_short
	cmp	count, #15
	b.le	.Ltail15tiny

	/* Deal with small copies quickly by dropping straight into the
	 * exit block.  */
.Ltail63:
	/* Copy up to 48 bytes of data.  At this point we only need the
	 * bottom 6 bits of count to be accurate.  */
	ands	tmp1, count, #0x30
	b.eq	.Ltail15
	add	dst, dst, tmp1
	add	src, src, tmp1
	cmp	tmp1w, #0x20
	b.eq	1f
	b.lt	2f
	ldp	A_l, A_h, [src, #-48]
	USER (9f, stp A_l, A_h, [dst, #-48])
1:
	ldp	A_l, A_h, [src, #-32]
	USER (9f, stp A_l, A_h, [dst, #-32])
2:
	ldp	A_l, A_h, [src, #-16]
	USER (9f, stp A_l, A_h, [dst, #-16])

.Ltail15:
	ands	count, count, #15
	beq	1f
	add	src, src, count
	ldp	A_l, A_h, [src, #-16]
	add	dst, dst, count
	USER (9f, stp A_l, A_h, [dst, #-16])
1:
	b	.Lsuccess

.Ltail15tiny:
	/* Copy up to 15 bytes of data.  Does not assume additional data
	   being copied.  */
	tbz	count, #3, 1f
	ldr	tmp1, [src], #8
	USER (9f, str tmp1, [dst], #8)
1:
	tbz	count, #2, 1f
	ldr	tmp1w, [src], #4
	USER (9f, str tmp1w, [dst], #4)
1:
	tbz	count, #1, 1f
	ldrh	tmp1w, [src], #2
	USER (9f, strh tmp1w, [dst], #2)
1:
	tbz	count, #0, 1f
	ldrb	tmp1w, [src]
	USER (9f, strb tmp1w, [dst])
1:
	b	.Lsuccess

.Lcpy_not_short:
	/* We don't much care about the alignment of DST, but we want SRC
	 * to be 128-bit (16 byte) aligned so that we don't cross cache line
	 * boundaries on both loads and stores.  */
	neg	tmp2, src
	ands	tmp2, tmp2, #15		/* Bytes to reach alignment.  */
	b.eq	2f
	sub	count, count, tmp2
	/* Copy more data than needed; it's faster than jumping
	 * around copying sub-Quadword quantities.  We know that
	 * it can't overrun.  */
	ldp	A_l, A_h, [src]
	add	src, src, tmp2
	USER (9f, stp A_l, A_h, [dst])
	add	dst, dst, tmp2
	/* There may be less than 63 bytes to go now.  */
	cmp	count, #63
	b.le	.Ltail63
2:
	subs	count, count, #128
	b.ge	.Lcpy_body_large
	/* Less than 128 bytes to copy, so handle 64 here and then jump
	 * to the tail.  */
	ldp	A_l, A_h, [src]
	ldp	B_l, B_h, [src, #16]
	ldp	C_l, C_h, [src, #32]
	ldp	D_l, D_h, [src, #48]
	USER (9f, stp A_l, A_h, [dst])
	USER (9f, stp B_l, B_h, [dst, #16])
	USER (9f, stp C_l, C_h, [dst, #32])
	USER (9f, stp D_l, D_h, [dst, #48])
	tst	count, #0x3f
	add	src, src, #64
	add	dst, dst, #64
	b.ne	.Ltail63
	b	.Lsuccess

	/* Critical loop.  Start at a new cache line boundary.  Assuming
	 * 64 bytes per line this ensures the entire loop is in one line.  */
	.p2align 6
.Lcpy_body_large:
	/* There are at least 128 bytes to copy.  */
	ldp	A_l, A_h, [src, #0]
	sub	dst, dst, #16		/* Pre-bias.  */
	ldp	B_l, B_h, [src, #16]
	ldp	C_l, C_h, [src, #32]
	ldp	D_l, D_h, [src, #48]!	/* src += 64 - Pre-bias.  */
1:
	USER (9f, stp A_l, A_h, [dst, #16])
	ldp	A_l, A_h, [src, #16]
	USER (9f, stp B_l, B_h, [dst, #32])
	ldp	B_l, B_h, [src, #32]
	USER (9f, stp C_l, C_h, [dst, #48])
	ldp	C_l, C_h, [src, #48]
	USER (9f, stp D_l, D_h, [dst, #64]!)
	ldp	D_l, D_h, [src, #64]!
	subs	count, count, #64
	b.ge	1b
	USER (9f, stp A_l, A_h, [dst, #16])
	USER (9f, stp B_l, B_h, [dst, #32])
	USER (9f, stp C_l, C_h, [dst, #48])
	USER (9f, stp D_l, D_h, [dst, #64])
	add	src, src, #16
	add	dst, dst, #64 + 16
	tst	count, #0x3f
	b.ne	.Ltail63
.Lsuccess:
	mov	x0, #0		    // nothing left to copy
	ret

	.size	copy_to_user, .-__copy_to_user

	.section .fixup,"ax"
	.align    2
9:	mov    x0, count            // approximate the number of bytes not copied
	ret
	.previous
