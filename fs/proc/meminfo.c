#include <linux/fs.h>
#include <linux/hugetlb.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/proc_fs.h>
#include <linux/quicklist.h>
#include <linux/seq_file.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/atomic.h>
#include <linux/vmalloc.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include "internal.h"
/*mklinux_akshay*/
#include <popcorn/init.h>
#include <linux/string.h>
#include "meminfo_remote.h"
/*mklinux_akshay*/

void __attribute__((weak)) arch_report_meminfo(struct seq_file *m)
{
}

static int meminfo_proc_show(struct seq_file *m, void *v)
{
	struct sysinfo i;
	unsigned long committed;
	unsigned long allowed;
	struct vmalloc_info vmi;
	long cached;
	unsigned long pages[NR_LRU_LISTS];
	int lru;
	/*mklinux_akshay*/
	struct task_struct *t;
	int o;
	_remote_mem_info_response_t rem_mem;
	/*mklinux_akshay*/

	/*
	 * display in kilobytes.
	 */
#define K(x) ((x) << (PAGE_SHIFT - 10))
	si_meminfo(&i);
	si_swapinfo(&i);
	committed = percpu_counter_read_positive(&vm_committed_as);
	allowed = ((totalram_pages - hugetlb_total_pages())
			* sysctl_overcommit_ratio / 100) + total_swap_pages;

	cached = global_page_state(NR_FILE_PAGES) -
		total_swapcache_pages() - i.bufferram;
	if (cached < 0)
		cached = 0;

	get_vmalloc_info(&vmi);

	for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
		pages[lru] = global_page_state(NR_LRU_BASE + lru);

	/* Read the remote meminfo */
	/*mklinux_akshay*/
	t = current;

	memset(&rem_mem, 0, sizeof(_remote_mem_info_response_t));
	remote_proc_meminfo_info(&rem_mem);
	/*mklinux_akshay*/

	/*
	 * Tagged format, for easy grepping and expansion.
	 */
	seq_printf(m,
			"MemTotal:       %8lu kB\n"
			"MemFree:        %8lu kB\n"
			"Buffers:        %8lu kB\n"
			"Cached:         %8lu kB\n"
			"SwapCached:     %8lu kB\n"
			"Active:         %8lu kB\n"
			"Inactive:       %8lu kB\n"
			"Active(anon):   %8lu kB\n"
			"Inactive(anon): %8lu kB\n"
			"Active(file):   %8lu kB\n"
			"Inactive(file): %8lu kB\n"
			"Unevictable:    %8lu kB\n"
			"Mlocked:        %8lu kB\n"
#ifdef CONFIG_HIGHMEM
			"HighTotal:      %8lu kB\n"
			"HighFree:       %8lu kB\n"
			"LowTotal:       %8lu kB\n"
			"LowFree:        %8lu kB\n"
#endif
#ifndef CONFIG_MMU
			"MmapCopy:       %8lu kB\n"
#endif
			"SwapTotal:      %8lu kB\n"
			"SwapFree:       %8lu kB\n"
			"Dirty:          %8lu kB\n"
			"Writeback:      %8lu kB\n"
			"AnonPages:      %8lu kB\n"
			"Mapped:         %8lu kB\n"
			"Shmem:          %8lu kB\n"
			"Slab:           %8lu kB\n"
			"SReclaimable:   %8lu kB\n"
			"SUnreclaim:     %8lu kB\n"
			"KernelStack:    %8lu kB\n"
			"PageTables:     %8lu kB\n"
#ifdef CONFIG_QUICKLIST
			"Quicklists:     %8lu kB\n"
#endif
			"NFS_Unstable:   %8lu kB\n"
			"Bounce:         %8lu kB\n"
			"WritebackTmp:   %8lu kB\n"
			"CommitLimit:    %8lu kB\n"
			"Committed_AS:   %8lu kB\n"
			"VmallocTotal:   %8lu kB\n"
			"VmallocUsed:    %8lu kB\n"
			"VmallocChunk:   %8lu kB\n"
#ifdef CONFIG_MEMORY_FAILURE
			"HardwareCorrupted: %5lu kB\n"
#endif
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
			"AnonHugePages:  %8lu kB\n"
#endif
			,
		K(i.totalram) + rem_mem._MemTotal,
		K(i.freeram) + rem_mem._MemFree,
		K(i.bufferram) + rem_mem._Buffers,
		K(cached) + rem_mem._Cached,
		K(total_swapcache_pages()) + rem_mem._SwapCached,
		K(pages[LRU_ACTIVE_ANON]   + pages[LRU_ACTIVE_FILE]) + rem_mem._Active,
		K(pages[LRU_INACTIVE_ANON] + pages[LRU_INACTIVE_FILE]) + rem_mem._Inactive,
		K(pages[LRU_ACTIVE_ANON]) + rem_mem._Active_anon,
		K(pages[LRU_INACTIVE_ANON]) + rem_mem._Inactive_anon,
		K(pages[LRU_ACTIVE_FILE]) + rem_mem._Active_file,
		K(pages[LRU_INACTIVE_FILE]) + rem_mem._Inactive_file,
		K(pages[LRU_UNEVICTABLE]) + rem_mem._Unevictable,
		K(global_page_state(NR_MLOCK)) + rem_mem._Mlocked,
#ifdef CONFIG_HIGHMEM
		K(i.totalhigh) + rem_mem._HighTotal,
		K(i.freehigh) + rem_mem._HighFre,
		K(i.totalram-i.totalhigh) + rem_mem._LowTotal,
		K(i.freeram-i.freehigh) + rem_mem._LowFree,
#endif
#ifndef CONFIG_MMU
		K((unsigned long) atomic_long_read(&mmap_pages_allocated)) + rem_mem._MmapCopy,
#endif
		K(i.totalswap) + rem_mem._SwapTotal,
		K(i.freeswap) + rem_mem._SwapFree,
		K(global_page_state(NR_FILE_DIRTY)) + rem_mem._Dirty,
		K(global_page_state(NR_WRITEBACK)) + rem_mem._Writeback,
		K(global_page_state(NR_ANON_PAGES)) + rem_mem._AnonPages,
		K(global_page_state(NR_FILE_MAPPED)) + rem_mem._Mapped,
		K(global_page_state(NR_SHMEM)) + rem_mem._Shmem,
		K(global_page_state(NR_SLAB_RECLAIMABLE) +
				global_page_state(NR_SLAB_UNRECLAIMABLE)) + rem_mem._Slab,
		K(global_page_state(NR_SLAB_RECLAIMABLE)) + rem_mem._SReclaimable,
		K(global_page_state(NR_SLAB_UNRECLAIMABLE)) + rem_mem._SUnreclaim,
		(global_page_state(NR_KERNEL_STACK) * THREAD_SIZE / 1024) + rem_mem._KernelStack,
		K(global_page_state(NR_PAGETABLE)) + rem_mem._PageTables,
#ifdef CONFIG_QUICKLIST
		K(quicklist_total_size()) + rem_mem._Quicklists,
#endif
		K(global_page_state(NR_UNSTABLE_NFS)) + rem_mem._NFS_Unstable,
		K(global_page_state(NR_BOUNCE)) + rem_mem._Bounce,
		K(global_page_state(NR_WRITEBACK_TEMP)) + rem_mem._WritebackTmp,
		K(allowed) + rem_mem._CommitLimit,
		K(committed) + rem_mem._Committed_AS,
		((unsigned long)VMALLOC_TOTAL >> 10)  + rem_mem._VmallocTotal,
		(vmi.used >> 10) + rem_mem._VmallocUsed,
		(vmi.largest_chunk >> 10) + rem_mem._VmallocChunk
#ifdef CONFIG_MEMORY_FAILURE
		,(atomic_long_read(&num_poisoned_pages) << (PAGE_SHIFT - 10)) + rem_mem._HardwareCorrupted
#endif
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
		,K((global_page_state(NR_ANON_TRANSPARENT_HUGEPAGES) *
					HPAGE_PMD_NR)) + rem_mem._AnonHugePages
#endif
		);

	hugetlb_report_meminfo(m);

	arch_report_meminfo(m);	
	return 0;
#undef K
}

static int meminfo_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, meminfo_proc_show, NULL);
}

static const struct file_operations meminfo_proc_fops = {
	.open		= meminfo_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_meminfo_init(void)
{
	proc_create("meminfo", 0, NULL, &meminfo_proc_fops);
	return 0;
}
module_init(proc_meminfo_init);
