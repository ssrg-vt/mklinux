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
 * calculate the length of a string
 *
 * Parameters:
 *	x0 - const string pointer
 * Returns:
 *	x0 - the return length of specific string
 */

/* Arguments and results.  */
#define srcin		x0
#define len		x0

/* Locals and temporaries.  */
#define src		x1
#define data1		x2
#define data2		x3
#define data2a		x4
#define has_nul1	x5
#define has_nul2	x6
#define tmp1		x7
#define tmp2		x8
#define tmp3		x9
#define tmp4		x10
#define zeroones	x11
#define pos		x12

#define REP8_01 0x0101010101010101
#define REP8_7f 0x7f7f7f7f7f7f7f7f
#define REP8_80 0x8080808080808080

ENTRY(strlen)
	mov	zeroones, #REP8_01
	bic	src, srcin, #15
	ands	tmp1, srcin, #15
	b.ne	.Lmisaligned
	/*
	* NUL detection works on the principle that (X - 1) & (~X) & 0x80
	* (=> (X - 1) & ~(X | 0x7f)) is non-zero iff a byte is zero, and
	* can be done in parallel across the entire word.
	*/
	/*
	* The inner loop deals with two Dwords at a time. This has a
	* slightly higher start-up cost, but we should win quite quickly,
	* especially on cores with a high number of issue slots per
	* cycle, as we get much better parallelism out of the operations.
	*/
.Lloop:
	ldp	data1, data2, [src], #16
.Lrealigned:
	sub	tmp1, data1, zeroones
	orr	tmp2, data1, #REP8_7f
	sub	tmp3, data2, zeroones
	orr	tmp4, data2, #REP8_7f
	bic	has_nul1, tmp1, tmp2
	bics	has_nul2, tmp3, tmp4
	ccmp	has_nul1, #0, #0, eq	/* NZCV = 0000  */
	b.eq	.Lloop

	sub	len, src, srcin
	cbz	has_nul1, .Lnul_in_data2
#ifdef __ARM64EB__
	mov	data2, data1
#endif
	sub	len, len, #8
	mov	has_nul2, has_nul1
.Lnul_in_data2:
#ifdef __ARM64EB__
	/*
	* For big-endian, carry propagation (if the final byte in the
	* string is 0x01) means we cannot use has_nul directly.  The
	* easiest way to get the correct byte is to byte-swap the data
	* and calculate the syndrome a second time.
	*/
	rev	data2, data2
	sub	tmp1, data2, zeroones
	orr	tmp2, data2, #REP8_7f
	bic	has_nul2, tmp1, tmp2
#endif
	sub	len, len, #8
	rev	has_nul2, has_nul2
	clz	pos, has_nul2
	add	len, len, pos, lsr #3		/* Bits to bytes.  */
	ret

.Lmisaligned:
	cmp	tmp1, #8
	neg	tmp1, tmp1
	ldp	data1, data2, [src], #16
	lsl	tmp1, tmp1, #3		/* Bytes beyond alignment -> bits.  */
	mov	tmp2, #~0
#ifdef __ARM64EB__
	/* Big-endian.  Early bytes are at MSB.  */
	lsl	tmp2, tmp2, tmp1	/* Shift (tmp1 & 63).  */
#else
	/* Little-endian.  Early bytes are at LSB.  */
	lsr	tmp2, tmp2, tmp1	/* Shift (tmp1 & 63).  */
#endif
	orr	data1, data1, tmp2
	orr	data2a, data2, tmp2
	csinv	data1, data1, xzr, le
	csel	data2, data2, data2a, le
	b	.Lrealigned
ENDPROC(strlen)
