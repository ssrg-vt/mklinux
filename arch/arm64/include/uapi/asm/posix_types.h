#ifndef __ASM_POSIX_TYPES_H
#define __ASM_POSIX_TYPES_H

#ifndef __LP64__	/* ILP32 */
typedef long long __kernel_long_t;
typedef unsigned long long __kernel_ulong_t;
#define __kernel_long_t __kernel_long_t
#endif

#include <asm-generic/posix_types.h>

#endif /* __ASM_POSIX_TYPES_H */
