/*
 * This file for Obtaining Remote MEM info
 *
 * Akshay
 */

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/pcn_kmsg.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/hugetlb.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/proc_fs.h>
#include <linux/quicklist.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/atomic.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include "internal.h"


#define PRINT_MESSAGES 0
#if PRINT_MESSAGES
#define PRINTK(...) printk(__VA_ARGS__)
#else
#define PRINTK(...) ;
#endif

/*
 *  Variables
 */
static int wait=-1;
static int _cpu=-1;


static DECLARE_WAIT_QUEUE_HEAD(wq);


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

/*
 * ******************************* Define variables holding Result *******************************************
 */
static _remote_mem_info_response_t *meminfo_result;

/*
 * ******************************* Common Functions **********************************************************
 */


int flush_meminfo_var()
{
	meminfo_result=NULL;
	wait=-1;
	return 0;
}


int fill_meminfo_response( _remote_mem_info_response_t *res)
{

	struct sysinfo i;
		unsigned long committed;
		unsigned long allowed;
		struct vmalloc_info vmi;
		long cached;
		unsigned long pages[NR_LRU_LISTS];
		int lru;

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
				total_swapcache_pages - i.bufferram;
		if (cached < 0)
			cached = 0;

		get_vmalloc_info(&vmi);

		for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
			pages[lru] = global_page_state(NR_LRU_BASE + lru);


		res->_MemTotal =	K(i.totalram);
		res->_MemFree =	K(i.freeram);
		res->_Buffers= K(i.bufferram);
		res->_Cached= K(cached);
		res->_SwapCached = K(total_swapcache_pages);
		res->_Active = K(pages[LRU_ACTIVE_ANON]   + pages[LRU_ACTIVE_FILE]);
		res->_Inactive = K(pages[LRU_INACTIVE_ANON] + pages[LRU_INACTIVE_FILE]);
		res->_Active_anon = K(pages[LRU_ACTIVE_ANON]);
		res->_Inactive_anon = K(pages[LRU_INACTIVE_ANON]);
		res->_Active_file = K(pages[LRU_ACTIVE_FILE]);
		res->_Inactive_file = K(pages[LRU_INACTIVE_FILE]);
		res->_Unevictable = K(pages[LRU_UNEVICTABLE]);
		res->_Mlocked = K(global_page_state(NR_MLOCK));
	#ifdef CONFIG_HIGHMEM
		res->_HighTotal = K(i.totalhigh);
		res->_HighFree = K(i.freehigh);
		res->_LowTotal = K(i.totalram-i.totalhigh);
		res->_LowFree = K(i.freeram-i.freehigh);
	#endif
	#ifndef CONFIG_MMU
		res->_MmapCopy = K((unsigned long) atomic_long_read(&mmap_pages_allocated));
	#endif
		res->_SwapTotal = K(i.totalswap);
		res->_SwapFree = K(i.freeswap);
		res->_Dirty = K(global_page_state(NR_FILE_DIRTY));
		res->_Writeback = K(global_page_state(NR_WRITEBACK));
	#ifdef CONFIG_TRANSPARENT_HUGEPAGE
		res->_Cached = K(global_page_state(NR_ANON_PAGES)
			  + global_page_state(NR_ANON_TRANSPARENT_HUGEPAGES) *
			  HPAGE_PMD_NR);
	#else
		res->_AnonPages = K(global_page_state(NR_ANON_PAGES));
	#endif
		res->_Mapped = K(global_page_state(NR_FILE_MAPPED));
		res->_Shmem = K(global_page_state(NR_SHMEM));
		res->_Slab = K(global_page_state(NR_SLAB_RECLAIMABLE) +
					global_page_state(NR_SLAB_UNRECLAIMABLE));
		res->_SReclaimable = K(global_page_state(NR_SLAB_RECLAIMABLE));
		res->_SUnreclaim = K(global_page_state(NR_SLAB_UNRECLAIMABLE));
		res->_KernelStack = 	global_page_state(NR_KERNEL_STACK) * THREAD_SIZE / 1024;
		res->_PageTables = K(global_page_state(NR_PAGETABLE));
	#ifdef CONFIG_QUICKLIST
		res->_Quicklists = K(quicklist_total_size());
	#endif
		res->_NFS_Unstable = K(global_page_state(NR_UNSTABLE_NFS));
		res->_Bounce = K(global_page_state(NR_BOUNCE));
		res->_WritebackTmp = K(global_page_state(NR_WRITEBACK_TEMP));
		res->_CommitLimit = K(allowed);
		res->_Committed_AS = K(committed);
		res->_VmallocTotal = (unsigned long)VMALLOC_TOTAL >> 10;
		res->_VmallocUsed = vmi.used >> 10;
		res->_VmallocChunk = vmi.largest_chunk >> 10;
	#ifdef CONFIG_MEMORY_FAILURE
		res->_HardwareCorrupted = atomic_long_read(&mce_bad_pages) << (PAGE_SHIFT - 10);
	#endif
	#ifdef CONFIG_TRANSPARENT_HUGEPAGE
		res->_AnonHugePages = K(global_page_state(NR_ANON_TRANSPARENT_HUGEPAGES) *
			   HPAGE_PMD_NR);
	#endif

	//	hugetlb_report_meminfo(m);

	//	arch_report_meminfo(m);


		return 0;
}

static int handle_remote_proc_mem_info_response(struct pcn_kmsg_message* inc_msg) {
	_remote_mem_info_response_t* msg = (_remote_mem_info_response_t*) inc_msg;

	PRINTK("%s: Entered remote cpu info response \n",__func__);

	wait = 1;
	if(msg !=NULL)
	 meminfo_result=msg;
	wake_up_interruptible(&wq);
	PRINTK("%s: response ---- wait{%d} \n",
			__func__, statwait);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}


static int handle_remote_proc_mem_info_request(struct pcn_kmsg_message* inc_msg) {

	_remote_mem_info_request_t* msg = (_remote_mem_info_request_t*) inc_msg;
	_remote_mem_info_response_t response;


	PRINTK("%s: Entered remote  cpu info request \n", __func__);

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_PROC_MEMINFO_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	fill_meminfo_response(&response);

	// Send response
	pcn_kmsg_send_long(msg->header.from_cpu, (struct pcn_kmsg_message*) (&response),
			sizeof(_remote_mem_info_response_t) - sizeof(struct pcn_kmsg_hdr));

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

int send_meminfo_request(int KernelId)
{

		int res=0;
		_remote_mem_info_request_t* request = kmalloc(sizeof(_remote_mem_info_request_t),
		GFP_KERNEL);
		// Build request
		request->header.type = PCN_KMSG_TYPE_REMOTE_PROC_MEMINFO_REQUEST;
		request->header.prio = PCN_KMSG_PRIO_NORMAL;
		// Send response
		res=pcn_kmsg_send(KernelId, (struct pcn_kmsg_message*) (request));
		return res;
}

/*
 * ************************************* Function (hook) to be called from other file ********************
 */
int remote_proc_meminfo_info(struct seq_file *m)
{
	flush_meminfo_var();
	int s;
	int ret=0;
	int i;
		 for(i = 0; i < NR_CPUS; i++) {

			    flush_meminfo_var();
		        // Skip the current cpu
		        if(i == _cpu) continue;
		        s = send_meminfo_request(i);
		        if(!s) {

		        	PRINTK("%s fill_next_remote_tgids: go to sleep!!!!",__func__);
		        			wait_event_interruptible(wq, wait != -1);
		        			wait = -1;

		        			if(!ret)
		        			{
		        			seq_printf(m, "*********Remote MEMINFO*****\n");


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
		        					meminfo_result->_MemTotal,
		        							meminfo_result->_MemFree,
		        							meminfo_result->_Buffers,
		        							meminfo_result->_Cached,
		        							meminfo_result->_SwapCached,
		        							meminfo_result->_Active,
		        							meminfo_result->_Inactive,
		        							meminfo_result->_Active_anon,
		        							meminfo_result->_Inactive_anon,
		        							meminfo_result->_Active_file,
		        							meminfo_result->_Inactive_file,
		        							meminfo_result->_Unevictable,
		        							meminfo_result->_Mlocked,
		        						#ifdef CONFIG_HIGHMEM
		        							meminfo_result->_HighTotal,
		        							meminfo_result->_HighFree,
		        							meminfo_result->_LowTotal,
		        							meminfo_result->_LowFree,
		        						#endif
		        						#ifndef CONFIG_MMU
		        							meminfo_result->_MmapCopy,
		        						#endif
		        							meminfo_result->_SwapTotal,
		        							meminfo_result->_SwapFree,
		        							meminfo_result->_Dirty,
		        							meminfo_result->_Writeback,
		        						#ifdef CONFIG_TRANSPARENT_HUGEPAGE
		        							meminfo_result->_AnonPages,
		        						#else
		        							meminfo_result->_Mapped,
		        						#endif
		        							meminfo_result->_Shmem,
		        							meminfo_result->_Slab,
		        							meminfo_result->_SReclaimable,
		        							meminfo_result->_SUnreclaim,
		        							meminfo_result->_KernelStack,
		        							meminfo_result->_PageTables,
		        						#ifdef CONFIG_QUICKLIST
		        							meminfo_result->_Quicklists,
		        						#endif
		        							meminfo_result->_NFS_Unstable,
		        							meminfo_result->_Bounce,
		        							meminfo_result->_WritebackTmp,
		        							meminfo_result->_CommitLimit,
		        							meminfo_result->_Committed_AS,
		        							meminfo_result->_VmallocTotal,
		        							meminfo_result->_VmallocUsed,
		        							meminfo_result->_VmallocChunk
		        						#ifdef CONFIG_MEMORY_FAILURE
		        							,meminfo_result->_HardwareCorrupted
		        						#endif
		        						#ifdef CONFIG_TRANSPARENT_HUGEPAGE
		        							,meminfo_result->_AnonHugePages
		        						#endif
		        						);
		        			}

		        }

		    }


			return 0;

}




static int __init proc_mem_handler_init(void)
{


    _cpu = smp_processor_id();


	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_PROC_MEMINFO_REQUEST,
				    		handle_remote_proc_mem_info_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_PROC_MEMINFO_RESPONSE,
							handle_remote_proc_mem_info_response);

	return 0;
}
/**
 * Register remote proc mem info init function as
 * module initialization function.
 */

late_initcall(proc_mem_handler_init);
//late_initcall(proc_mem_handler_init);
