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
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/linkage.h>
#include <asm/assembler.h>

/*
 * Fill in the buffer with character c
 *
 * Parameters:
 *	x0 - buf
 *	x1 - c
 *	x2 - n
 * Returns:
 *	x0 - buf
 */

/* By default we assume that the DC instruction can be used to zero
*  data blocks more efficiently.  In some circumstances this might be
*  unsafe, for example in an asymmetric multiprocessor environment with
*  different DC clear lengths (neither the upper nor lower lengths are
*  safe to use).  The feature can be disabled by defining DONT_USE_DC.
*/

#define dstin		x0
#define val		w1
#define count		x2
#define tmp1		x3
#define tmp1w		w3
#define tmp2		x4
#define tmp2w		w4
#define zva_len_x	x5
#define zva_len	w5
#define zva_bits_x	x6

#define A_l		x7
#define A_lw		w7
#define dst		x8
#define tmp3w		w9
#define tmp3		x9

ENTRY(memset)
	mov	dst, dstin	/* Preserve return value.  */
	and	A_lw, val, #255
	orr	A_lw, A_lw, A_lw, lsl #8
	orr	A_lw, A_lw, A_lw, lsl #16
	orr	A_l, A_l, A_l, lsl #32

	/*first align dst with 16...*/
	neg	tmp2, dst
	ands	tmp2, tmp2, #15
	b.eq	.Laligned

	cmp	count, #15
	b.le	.Ltail15tiny
	/*
	* The count is not less than 16, we can use stp to set 16 bytes
	* once. This way is more efficient but the access is non-aligned.
	*/
	stp	A_l, A_l, [dst]
	/*make the dst aligned..*/
	sub	count, count, tmp2
	add	dst, dst, tmp2

	/*Here, dst is aligned 16 now...*/
.Laligned:
#ifndef DONT_USE_DC
	cbz	A_l, .Lzero_mem
#endif

.Ltail_maybe_long:
	cmp	count, #64
	b.ge	.Lnot_short
.Ltail63:
	ands	tmp1, count, #0x30
	b.eq	.Ltail15tiny
	cmp	tmp1w, #0x20
	b.eq	1f
	b.lt	2f
	stp	A_l, A_l, [dst], #16
1:
	stp	A_l, A_l, [dst], #16
2:
	stp	A_l, A_l, [dst], #16
/*
* The following process is non-aligned access. But it is more efficient than
* .Ltail15tiny. Of-course, we can delete this code, but have a bit
* performance cost.
*/
	ands	count, count, #15
	cbz	count, 1f
	add	dst, dst, count
	stp	A_l, A_l, [dst, #-16]	/* Repeat some/all of last store. */
1:
	ret

.Ltail15tiny:
	/* Set up to 15 bytes.  Does not assume earlier memory
	being set.  */
	tbz	count, #3, 1f
	str	A_l, [dst], #8
1:
	tbz	count, #2, 1f
	str	A_lw, [dst], #4
1:
	tbz	count, #1, 1f
	strh	A_lw, [dst], #2
1:
	tbz	count, #0, 1f
	strb	A_lw, [dst]
1:
	ret

	/*
	* Critical loop. Start at a new cache line boundary. Assuming
	* 64 bytes per line, this ensures the entire loop is in one line.
	*/
	.p2align	6
.Lnot_short: /*count must be not less than 64*/
	sub	dst, dst, #16/* Pre-bias.  */
	sub	count, count, #64
1:
	stp	A_l, A_l, [dst, #16]
	stp	A_l, A_l, [dst, #32]
	stp	A_l, A_l, [dst, #48]
	stp	A_l, A_l, [dst, #64]!
	subs	count, count, #64
	b.ge	1b
	tst	count, #0x3f
	add	dst, dst, #16
	b.ne	.Ltail63
.Lexitfunc:
	ret

#ifndef DONT_USE_DC
	/*
	* For zeroing memory, check to see if we can use the ZVA feature to
	* zero entire 'cache' lines.
	*/
.Lzero_mem:
	cmp	count, #63
	b.le	.Ltail63
	/*
	* For zeroing small amounts of memory, it's not worth setting up
	* the line-clear code.
	*/
	cmp	count, #128
	b.lt	.Lnot_short /*count is at least  128 bytes*/

	mrs	tmp1, dczid_el0
	tbnz	tmp1, #4, .Lnot_short
	mov	tmp3w, #4
	and	zva_len, tmp1w, #15	/* Safety: other bits reserved.  */
	lsl	zva_len, tmp3w, zva_len

	ands	tmp3w, zva_len, #63
	/*
	* ensure the zva_len is not less than 64.
	* It is not meaningful to use ZVA if the block size is less than 64.
	*/
	b.ne	.Lnot_short
.Lzero_by_line:
	/*
	* Compute how far we need to go to become suitably aligned. We're
	* already at quad-word alignment.
	*/
	cmp	count, zva_len_x
	b.lt	.Lnot_short		/* Not enough to reach alignment.  */
	sub	zva_bits_x, zva_len_x, #1
	neg	tmp2, dst
	ands	tmp2, tmp2, zva_bits_x
	b.eq	1f			/* Already aligned.  */
	/* Not aligned, check that there's enough to copy after alignment.*/
	sub	tmp1, count, tmp2
	/*
	* grantee the remain length to be ZVA is bigger than 64,
	* avoid to make the 2f's process over mem range.*/
	cmp	tmp1, #64
	ccmp	tmp1, zva_len_x, #8, ge	/* NZCV=0b1000 */
	b.lt	.Lnot_short
	/*
	* We know that there's at least 64 bytes to zero and that it's safe
	* to overrun by 64 bytes.
	*/
	mov	count, tmp1
2:
	stp	A_l, A_l, [dst]
	stp	A_l, A_l, [dst, #16]
	stp	A_l, A_l, [dst, #32]
	subs	tmp2, tmp2, #64
	stp	A_l, A_l, [dst, #48]
	add	dst, dst, #64
	b.ge	2b
	/* We've overrun a bit, so adjust dst downwards.*/
	add	dst, dst, tmp2
1:
	sub	count, count, zva_len_x
3:
	dc	zva, dst
	add	dst, dst, zva_len_x
	subs	count, count, zva_len_x
	b.ge	3b
	ands	count, count, zva_bits_x
	b.ne	.Ltail_maybe_long
	ret
#endif /* DONT_USE_DC */
ENDPROC(memset)
