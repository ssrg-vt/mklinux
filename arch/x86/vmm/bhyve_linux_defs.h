#ifndef _BHYVE_LINUX_DEFS_H_
#define _BHYVE_LINUX_DEFS_H_

#include <linux/types.h>
#include "param.h"

#ifndef CTASSERT                /* Allow lint to override */
#define CTASSERT(x)             _CTASSERT(x, __LINE__)
#define _CTASSERT(x, y)         __CTASSERT(x, y)
#define __CTASSERT(x, y)        typedef char __assert ## y[(x) ? 1 : -1]
#endif

typedef int64_t register_t;

typedef uint64_t pd_entry_t;
typedef uint64_t pt_entry_t;
typedef uint64_t pdp_entry_t;
typedef uint64_t pml4_entry_t;

/* Size of the level 1 page table units */
#define NPTEPG          (PAGE_SIZE/(sizeof (pt_entry_t)))
#define NPTEPGSHIFT     9               /* LOG2(NPTEPG) */
#define PAGE_SHIFT      12              /* LOG2(PAGE_SIZE) */
#define PAGE_SIZE       (1<<PAGE_SHIFT) /* bytes/page */
#define PAGE_MASK       (PAGE_SIZE-1)
/* Size of the level 2 page directory units */
#define NPDEPG          (PAGE_SIZE/(sizeof (pd_entry_t)))
#define NPDEPGSHIFT     9               /* LOG2(NPDEPG) */
#define PDRSHIFT        21              /* LOG2(NBPDR) */
#define NBPDR           (1<<PDRSHIFT)   /* bytes/page dir */
#define PDRMASK         (NBPDR-1)
/* Size of the level 3 page directory pointer table units */
#define NPDPEPG         (PAGE_SIZE/(sizeof (pdp_entry_t)))
#define NPDPEPGSHIFT    9               /* LOG2(NPDPEPG) */
#define PDPSHIFT        30              /* LOG2(NBPDP) */
#define NBPDP           (1<<PDPSHIFT)   /* bytes/page dir ptr table */
#define PDPMASK         (NBPDP-1)
/* Size of the level 4 page-map level-4 table units */
#define NPML4EPG        (PAGE_SIZE/(sizeof (pml4_entry_t)))
#define NPML4EPGSHIFT   9               /* LOG2(NPML4EPG) */
#define PML4SHIFT       39              /* LOG2(NBPML4) */
#define NBPML4          (1UL<<PML4SHIFT)/* bytes/page map lev4 table */
#define PML4MASK        (NBPML4-1)

#define __dead2 __attribute__((__noreturn__))
#define __pure2 __attribute__((__const__))
#define __unused __attribute__((__unused__))

#endif
