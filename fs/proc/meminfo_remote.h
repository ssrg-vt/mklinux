#ifndef __MEMINFO_REMOTE__
#define __MEMINFO_REMOTE__

#include <linux/pcn_kmsg.h>

/*
 * ****************************** Message structures for obtaining Mem Info ********************************
 */
struct _remote_mem_info_request {
    struct pcn_kmsg_hdr header;
    char pad_string[60];
} __attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_mem_info_request _remote_mem_info_request_t;

struct _remote_mem_info_response {
    struct pcn_kmsg_hdr header;
	unsigned long _MemTotal;
	unsigned long _MemFree;
	unsigned long _Buffers;
	unsigned long _Cached;
	unsigned long _SwapCached;
	unsigned long _Active;
	unsigned long _Inactive;
	unsigned long _Active_anon;
	unsigned long _Inactive_anon;
	unsigned long _Active_file;
	unsigned long _Inactive_file;
	unsigned long _Unevictable;
	unsigned long _Mlocked;
	unsigned long _HighTotal;
	unsigned long _HighFree;
	unsigned long _LowTotal;
	unsigned long _LowFree;

	unsigned long _MmapCopy;

	unsigned long _SwapTotal;
	unsigned long _SwapFree;
	unsigned long _Dirty;
	unsigned long _Writeback;
	unsigned long _AnonPages;
	unsigned long _Mapped;
	unsigned long _Shmem;
	unsigned long _Slab;
	unsigned long _SReclaimable;
	unsigned long _SUnreclaim;
	unsigned long _KernelStack;
	unsigned long _PageTables;
	unsigned long _Quicklists;

	unsigned long _NFS_Unstable;
	unsigned long _Bounce;
	unsigned long _WritebackTmp;
	unsigned long _CommitLimit;
	unsigned long _Committed_AS;
	unsigned long _VmallocTotal;
	unsigned long _VmallocUsed;
	unsigned long _VmallocChunk;
	unsigned long _HardwareCorrupted;
	unsigned long _AnonHugePages;
   } __attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_mem_info_response _remote_mem_info_response_t;

#ifdef CONFIG_PROC_REMOTE_MEMINFO
extern int remote_proc_meminfo_info(_remote_mem_info_response_t *total);
#else
static int remote_proc_meminfo_info(_remote_mem_info_response_t *total) {
	return 0;
}
#endif

#endif

/*
 * config PROC_REMOTE_MEMINFO
	default y
	depends on PROC_FS
	bool "Enables aggregation of all the memory available in a Popcorn Linux deployment"
	help
	/proc/meminfo will collect all the information available in a Popcorn
	Linux replicated-kernel OS from multiple kernels.
 */
