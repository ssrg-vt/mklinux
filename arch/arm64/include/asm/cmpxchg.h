/*
 * Based on arch/arm/include/asm/cmpxchg.h
 *
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
#ifndef __ASM_CMPXCHG_H
#define __ASM_CMPXCHG_H

#include <linux/mmdebug.h>
#include <linux/bug.h>

#include <asm/barrier.h>

#define __HAVE_ARCH_CMPXCHG 1

static inline unsigned long __xchg(unsigned long x, volatile void *ptr, int size)
{
	unsigned long ret, tmp;

	switch (size) {
	case 1:
		asm volatile("//	__xchg1\n"
		"1:	ldaxrb	%w0, %2\n"
		"	stlxrb	%w1, %w3, %2\n"
		"	cbnz	%w1, 1b\n"
			: "=&r" (ret), "=&r" (tmp), "+Q" (*(u8 *)ptr)
			: "r" (x)
			: "cc", "memory");
		break;
	case 2:
		asm volatile("//	__xchg2\n"
		"1:	ldaxrh	%w0, %2\n"
		"	stlxrh	%w1, %w3, %2\n"
		"	cbnz	%w1, 1b\n"
			: "=&r" (ret), "=&r" (tmp), "+Q" (*(u16 *)ptr)
			: "r" (x)
			: "cc", "memory");
		break;
	case 4:
		asm volatile("//	__xchg4\n"
		"1:	ldaxr	%w0, %2\n"
		"	stlxr	%w1, %w3, %2\n"
		"	cbnz	%w1, 1b\n"
			: "=&r" (ret), "=&r" (tmp), "+Q" (*(u32 *)ptr)
			: "r" (x)
			: "cc", "memory");
		break;
	case 8:
		asm volatile("//	__xchg8\n"
		"1:	ldaxr	%0, %2\n"
		"	stlxr	%w1, %3, %2\n"
		"	cbnz	%w1, 1b\n"
			: "=&r" (ret), "=&r" (tmp), "+Q" (*(u64 *)ptr)
			: "r" (x)
			: "cc", "memory");
		break;
	default:
		BUILD_BUG();
	}

	return ret;
}

#define xchg(ptr,x) \
	((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))

static inline unsigned long __cmpxchg(volatile void *ptr, unsigned long old,
				      unsigned long new, int size)
{
	unsigned long oldval = 0, res;

	switch (size) {
	case 1:
		do {
			asm volatile("// __cmpxchg1\n"
			"	ldxrb	%w1, %2\n"
			"	mov	%w0, #0\n"
			"	cmp	%w1, %w3\n"
			"	b.ne	1f\n"
			"	stxrb	%w0, %w4, %2\n"
			"1:\n"
				: "=&r" (res), "=&r" (oldval), "+Q" (*(u8 *)ptr)
				: "Ir" (old), "r" (new)
				: "cc");
		} while (res);
		break;

	case 2:
		do {
			asm volatile("// __cmpxchg2\n"
			"	ldxrh	%w1, %2\n"
			"	mov	%w0, #0\n"
			"	cmp	%w1, %w3\n"
			"	b.ne	1f\n"
			"	stxrh	%w0, %w4, %2\n"
			"1:\n"
				: "=&r" (res), "=&r" (oldval), "+Q" (*(u16 *)ptr)
				: "Ir" (old), "r" (new)
				: "cc");
		} while (res);
		break;

	case 4:
		do {
			asm volatile("// __cmpxchg4\n"
			"	ldxr	%w1, %2\n"
			"	mov	%w0, #0\n"
			"	cmp	%w1, %w3\n"
			"	b.ne	1f\n"
			"	stxr	%w0, %w4, %2\n"
			"1:\n"
				: "=&r" (res), "=&r" (oldval), "+Q" (*(u32 *)ptr)
				: "Ir" (old), "r" (new)
				: "cc");
		} while (res);
		break;

	case 8:
		do {
			asm volatile("// __cmpxchg8\n"
			"	ldxr	%1, %2\n"
			"	mov	%w0, #0\n"
			"	cmp	%1, %3\n"
			"	b.ne	1f\n"
			"	stxr	%w0, %4, %2\n"
			"1:\n"
				: "=&r" (res), "=&r" (oldval), "+Q" (*(u64 *)ptr)
				: "Ir" (old), "r" (new)
				: "cc");
		} while (res);
		break;

	default:
		BUILD_BUG();
	}

	return oldval;
}

static inline unsigned long __cmpxchg_mb(volatile void *ptr, unsigned long old,
					 unsigned long new, int size)
{
	unsigned long ret;

	smp_mb();
	ret = __cmpxchg(ptr, old, new, size);
	smp_mb();

	return ret;
}

#define cmpxchg(ptr,o,n)						\
	((__typeof__(*(ptr)))__cmpxchg_mb((ptr),			\
					  (unsigned long)(o),		\
					  (unsigned long)(n),		\
					  sizeof(*(ptr))))

#define cmpxchg_local(ptr,o,n)						\
	((__typeof__(*(ptr)))__cmpxchg((ptr),				\
				       (unsigned long)(o),		\
				       (unsigned long)(n),		\
				       sizeof(*(ptr))))



static inline int __cmpxchg_double(volatile unsigned long *ptr1,
				   volatile unsigned long *ptr2,
				   unsigned long old1,
				   unsigned long old2,
				   unsigned long new1, unsigned long new2)
{
	int res = 0;		/* Return value: 0 if miscompare, else 1. */
	unsigned long status = 0;	/* STXP status value. */
	unsigned long act1 = 0, act2 = 0;	/* Actual old values. */

	BUILD_BUG_ON((sizeof(unsigned long) != 4) &&
		     (sizeof(unsigned long) != 8));
	VM_BUG_ON((unsigned long)ptr1 % (2 * sizeof(unsigned long)));
	VM_BUG_ON((unsigned long)(ptr1 + 1) != (unsigned long)ptr2);

	if (sizeof(long) == 4) {
		asm volatile ("    mov    %[res], #0\n"
			      "1:\n"
			      "    ldxp    %w[act1], %w[act2], %[ptr1]\n"
			      "    cmp    %[act1], %[old1]\n"
			      "    ccmp    %[act2], %[old2], 0x1, EQ\n"
			      "    b.ne    2f\n"
			      "    stxp    %w[status], %w[new1], %w[new2], %[ptr1]\n"
			      "       cbnz    %[status], 1b\n"
			      "       mov     %[res], #1\n"
			      "2:\n":[res] "=&r"(res),
			      [status] "=&r"(status),
			      [act1] "=&r"(act1),[act2] "=&r"(act2)
			      :[old1] "r"(old1),
			      [old2] "r"(old2),
			      [new1] "r"(new1),[new2] "r"(new2),[ptr1] "Q"(ptr1)
			      :"cc", "memory");
	} else {
		asm volatile ("    mov    %[res], #0\n"
			      "1:\n"
			      "    ldxp    %[act1], %[act2], %[ptr1]\n"
			      "    cmp    %[act1], %[old1]\n"
			      "    ccmp    %[act2], %[old2], 0x1, EQ\n"
			      "    b.ne    2f\n"
			      "    stxp    %w[status], %[new1], %[new2], %[ptr1]\n"
			      "       cbnz    %[status], 1b\n"
			      "       mov     %[res], #1\n"
			      "2:\n":[res] "=&r"(res),
			      [status] "=&r"(status),
			      [act1] "=&r"(act1),[act2] "=&r"(act2)
			      :[old1] "r"(old1),
			      [old2] "r"(old2),
			      [new1] "r"(new1),
			      [new2] "r"(new2),[ptr1] "Q"(*ptr1)
			      :"cc", "memory");
	}

	return res;
}

static inline int __cmpxchg_double_mb(volatile unsigned long *ptr1,
				      volatile unsigned long *ptr2,
				      unsigned long old1,
				      unsigned long old2,
				      unsigned long new1, unsigned long new2)
{
	int ret;
	smp_mb();
	ret = __cmpxchg_double(ptr1, ptr2, old1, old2, new1, new2);
	smp_mb();
	return ret;
}

#define cmpxchg_double(p1, p2, o1, o2, n1, n2)                \
({                                    \
    int __ret;                            \
    unsigned long *__p1 = (unsigned long *)(p1);            \
    unsigned long *__p2 = (unsigned long *)(p2);            \
        unsigned long __o1 = (unsigned long)(o1);                       \
        unsigned long __o2 = (unsigned long)(o2);                       \
        unsigned long __n1 = (unsigned long)(n1);                       \
        unsigned long __n2 = (unsigned long)(n2);                       \
        BUILD_BUG_ON(sizeof(unsigned long) != sizeof(unsigned long *)); \
        BUILD_BUG_ON(sizeof(o1) != sizeof(unsigned long));              \
        BUILD_BUG_ON(sizeof(o2) != sizeof(unsigned long));              \
        BUILD_BUG_ON(sizeof(n1) != sizeof(unsigned long));              \
        BUILD_BUG_ON(sizeof(n2) != sizeof(unsigned long));              \
        __ret = __cmpxchg_double_mb(__p1, __p2, __o1, __o2, __n1, __n2);\
    __ret;                                \
})

#define system_has_cmpxchg_double()     1

#define cmpxchg64(ptr,o,n)		cmpxchg((ptr),(o),(n))
#define cmpxchg64_local(ptr,o,n)	cmpxchg_local((ptr),(o),(n))

#endif	/* __ASM_CMPXCHG_H */
