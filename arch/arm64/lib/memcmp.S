/*
 * Copyright (C) 2013 ARM Ltd.
 * Copyright (C) 2013 Linaro.
 *
 * This code is based on glibc cortex strings work originally authored by Linaro
 * and re-licensed under GPLv2 for the Linux kernel. The original code can
 * be found @
 *
 * http://bazaar.launchpad.net/~linaro-toolchain-dev/cortex-strings/trunk/
 * files/head:/src/aarch64/
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

/*
* compare memory areas(when two memory areas' offset are different,
* alignment handled by the hardware)
*
* Parameters:
*  x0 - const memory area 1 pointer
*  x1 - const memory area 2 pointer
*  x2 - the maximal compare byte length
* Returns:
*  x0 - a compare result, maybe less than, equal to, or greater than ZERO
*/

/* Parameters and result.  */
#define src1		x0
#define src2		x1
#define limit		x2
#define result		x0

/* Internal variables.  */
#define data1		x3
#define data1w		w3
#define data2		x4
#define data2w		w4
#define has_nul		x5
#define diff		x6
#define endloop		x7
#define tmp1		x8
#define tmp2		x9
#define tmp3		x10
#define pos		x11
#define limit_wd	x12
#define mask		x13

ENTRY(memcmp)
	cbz	limit, .Lret0
	eor	tmp1, src1, src2
	tst	tmp1, #7
	b.ne	.Lmisaligned8
	ands	tmp1, src1, #7
	b.ne	.Lmutual_align
	sub	limit_wd, limit, #1 /* limit != 0, so no underflow.  */
	lsr	limit_wd, limit_wd, #3 /* Convert to Dwords.  */
.Lloop_aligned:
	ldr	data1, [src1], #8
	ldr	data2, [src2], #8
.Lstart_realigned:
	subs	limit_wd, limit_wd, #1
	eor	diff, data1, data2	/* Non-zero if differences found.  */
	csinv	endloop, diff, xzr, cs	/* Last Dword or differences.  */
	cbz	endloop, .Lloop_aligned

	/* Not reached the limit, must have found a diff.  */
	tbz	limit_wd, #63, .Lnot_limit

	/* Limit % 8 == 0 => all bytes significant.  */
	ands	limit, limit, #7
	b.eq	.Lnot_limit

	lsl	limit, limit, #3	/* Bits -> bytes.  */
	mov	mask, #~0
#ifdef __ARM64EB__
	lsr	mask, mask, limit
#else
	lsl	mask, mask, limit
#endif
	bic	data1, data1, mask
	bic	data2, data2, mask

	orr	diff, diff, mask
	b	.Lnot_limit

.Lmutual_align:
	/*
	* Sources are mutually aligned, but are not currently at an
	* alignment boundary. Round down the addresses and then mask off
	* the bytes that precede the start point.
	*/
	bic	src1, src1, #7
	bic	src2, src2, #7
	ldr	data1, [src1], #8
	ldr	data2, [src2], #8
	sub	limit_wd, limit, #1/* limit != 0, so no underflow.  */
	and	tmp3, limit_wd, #7
	lsr	limit_wd, limit_wd, #3
	add	tmp3, tmp3, tmp1
	add	limit_wd, limit_wd, tmp3, lsr #3
	add	limit, limit, tmp1/* Adjust the limit for the extra.  */
	lsl	tmp1, tmp1, #3/* Bytes beyond alignment -> bits.*/
	neg	tmp1, tmp1/* Bits to alignment -64.  */
	mov	tmp2, #~0
#ifdef __ARM64EB__
	/* Big-endian.  Early bytes are at MSB.  */
	lsl	tmp2, tmp2, tmp1/* Shift (tmp1 & 63).  */
#else
	/* Little-endian.  Early bytes are at LSB.  */
	lsr	tmp2, tmp2, tmp1/* Shift (tmp1 & 63).  */
#endif
	orr	data1, data1, tmp2
	orr	data2, data2, tmp2
	b	.Lstart_realigned

.Lmisaligned8:
	cmp	limit, #8
	b.lo	.Ltiny8proc /*limit < 8... */
	/*
	* Get the align offset length to compare per byte first.
	* After this process, one string's address will be aligned.
	*/
	and	tmp1, src1, #7
	neg	tmp1, tmp1
	add	tmp1, tmp1, #8
	and	tmp2, src2, #7
	neg	tmp2, tmp2
	add	tmp2, tmp2, #8
	subs	tmp3, tmp1, tmp2
	csel	pos, tmp1, tmp2, hi /*Choose the maximum. */
	/*
	* Here, limit is not less than 8,
	* so directly run .Ltinycmp without checking the limit.*/
	sub	limit, limit, pos
.Ltinycmp:
	ldrb	data1w, [src1], #1
	ldrb	data2w, [src2], #1
	subs	pos, pos, #1
	ccmp	data1w, data2w, #0, ne  /* NZCV = 0b0000.  */
	b.eq	.Ltinycmp
	cbnz	pos, 1f /*find the unequal...*/
	cmp	data1w, data2w
	b.eq	.Lstart_align /*the last bytes are equal....*/
1:
	sub	result, data1, data2
	ret

.Lstart_align:
	lsr	limit_wd, limit, #3
	cbz	limit_wd, .Lremain8
	ands	xzr, src1, #7
	/*
	* eq means tmp1 bytes finished the compare in the Ltinycmp,
	* tmp3 is positive here
	*/
	b.eq	.Lrecal_offset
	add	src1, src1, tmp3
	add	src2, src2, tmp3
	sub	limit, limit, tmp3
	lsr	limit_wd, limit, #3
	cbz	limit_wd, .Lremain8
	ldr	data1, [src1], #8
	ldr	data2, [src2], #8

	subs	limit_wd, limit_wd, #1
	eor	diff, data1, data2  /*Non-zero if differences found.*/
	csinv	endloop, diff, xzr, ne
	cbnz	endloop, .Lunequal_proc
	and	tmp3, tmp3, #7  /*tmp3 = 8 + tmp3 ( old tmp3 is negative)*/
	/*
	* src1 is aligned and src1 is in the right of src2.
	* Remain count is not less than 8 here.
	*/
.Lrecal_offset:
	neg	pos, tmp3
.Lloopcmp_proc:
	/*
	* Fall back pos bytes, get the first bytes segment of
	* one Dword of src1. pos is negative here. We also can use :
	* ldr	data1, [src1]
	* ldr	data2, [src2, pos]
	* These two instructions will read data with aligned address,then
	* do the compare.But if we adapt this method, have to add some
	* shift and mask out some bits from these two Dword to construct
	* two Dwords to compare.Some more instructions will be added,
	* and most important, it will need more time cost.
	*/
	ldr	data1, [src1,pos]
	ldr	data2, [src2,pos]
	eor	diff, data1, data2  /* Non-zero if differences found.*/
	cbnz	diff, .Lnot_limit

	/*The second part process*/
	ldr	data1, [src1], #8
	ldr	data2, [src2], #8
	eor	diff, data1, data2  /* Non-zero if differences found.*/
	subs	limit_wd, limit_wd, #1
	csinv	endloop, diff, xzr, ne/*if limit_wd is 0,will finish the cmp*/
	cbz	endloop, .Lloopcmp_proc
.Lunequal_proc:
	/*whether  unequal occurred?*/
	cbz	diff, .Lremain8
.Lnot_limit:
#ifndef	__ARM64EB__
	rev	diff, diff
	rev	data1, data1
	rev	data2, data2
#endif
	/*
	* The MS-non-zero bit of DIFF marks either the first bit
	* that is different, or the end of the significant data.
	* Shifting left now will bring the critical information into the
	* top bits.
	*/
	clz	pos, diff
	lsl	data1, data1, pos
	lsl	data2, data2, pos
	/* But we need to zero-extend (char is unsigned) the value and then
	perform a signed 32-bit subtraction.  */
	lsr	data1, data1, #56
	sub	result, data1, data2, lsr #56
	ret

	.p2align	6
.Lremain8:
	/* Limit % 8 == 0 => all bytes significant.  */
	ands	limit, limit, #7
	b.eq	.Lret0

.Ltiny8proc:
	/*Perhaps we can do better than this.*/
	ldrb	data1w, [src1], #1
	ldrb	data2w, [src2], #1
	subs	limit, limit, #1
	/*
	* ne satisfied means current limit > 0. Z=1 will make cs =0,
	* lead to next ccmp use ZERO to set flags,so break the loop.*/
	ccmp	data1w, data2w, #0, ne  /* NZCV = 0b0000. */
	b.eq	.Ltiny8proc
	sub	result, data1, data2
	ret
.Lret0:
	mov	result, #0
	ret
ENDPROC(memcmp)
