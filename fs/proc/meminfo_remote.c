/*
 * This file for Obtaining Remote MEM info
 *
 * Akshay
 */

#include <linux/sysinfo.h>
#include <linux/kthread.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/timer.h>
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
#include <linux/vmalloc.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include "internal.h"
#include "meminfo_remote.h"

//antoniob -- I think that here concurrency is not handled -- to be checked better

#define PRINT_MESSAGES 0
#if PRINT_MESSAGES
#define PRINTK(...) printk(__VA_ARGS__)
#else
#define PRINTK(...) ;
#endif

#define MAX_KERN 2 //NR_CPUS
/*
 *  Variables
 */
static int wait=-1;
static int _cpu=-1;


static DECLARE_WAIT_QUEUE_HEAD(wq);

/*
 * ******************************* Define variables holding Result *******************************************
 */
static _remote_mem_info_response_t remote_meminfo_result;
static _remote_mem_info_response_t *meminfo_result;

/*
 * ******************************* Common Functions **********************************************************
 */


int flush_meminfo_var()
{
	memset(&remote_meminfo_result, 0, sizeof(_remote_mem_info_response_t));
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

	cached = global_page_state(NR_FILE_PAGES) - total_swapcache_pages() - i.bufferram;
	if (cached < 0)
		cached = 0;

	get_vmalloc_info(&vmi);

	for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
		pages[lru] = global_page_state(NR_LRU_BASE + lru);

	res->_MemTotal = K(i.totalram);
	res->_MemFree =	K(i.freeram);
	res->_Buffers= K(i.bufferram);
	res->_Cached= K(cached);
	res->_SwapCached = K(total_swapcache_pages());
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
	res->_AnonPages = K(global_page_state(NR_ANON_PAGES));
	res->_Mapped = K(global_page_state(NR_FILE_MAPPED));
	res->_Shmem = K(global_page_state(NR_SHMEM));
	res->_Slab = K(global_page_state(NR_SLAB_RECLAIMABLE) +
				global_page_state(NR_SLAB_UNRECLAIMABLE));
	res->_SReclaimable = K(global_page_state(NR_SLAB_RECLAIMABLE));
	res->_SUnreclaim = K(global_page_state(NR_SLAB_UNRECLAIMABLE));
	res->_KernelStack = global_page_state(NR_KERNEL_STACK) * THREAD_SIZE / 1024;
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
	// Sharath: mce_bad_pages is not available in Linux 3.12 
	//res->_HardwareCorrupted = atomic_long_read(&mce_bad_pages) << (PAGE_SHIFT - 10);		
	res->_HardwareCorrupted = atomic_long_read(&num_poisoned_pages) << (PAGE_SHIFT - 10);
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
	if(msg != NULL) {
		meminfo_result = &remote_meminfo_result;
		memcpy(meminfo_result, msg, sizeof(_remote_mem_info_response_t));
	}
	wake_up_interruptible(&wq);
	PRINTK("%s: response ---- wait{%d} \n",
			__func__, statwait);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}


static int handle_remote_proc_mem_info_request(struct pcn_kmsg_message* inc_msg) {

	_remote_mem_info_request_t* msg = (_remote_mem_info_request_t*) inc_msg;
	_remote_mem_info_response_t response;
	
	memset(&response, 0, sizeof(_remote_mem_info_response_t));

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
		kfree(request);
		return res;
}

/*
 * ************************************* Function (hook) to be called from other file ********************
 */
int remote_proc_meminfo_info(_remote_mem_info_response_t *total)
{
	int s;
	int ret=0;
	int i;

	if (total == NULL) {
		return -1;
	} 
	
	/* clear the total meminfo */
	memset(total, 0, sizeof(_remote_mem_info_response_t));

	for(i = 0; i < MAX_KERN; i++) {
		/* clear the temp meminfo */
		flush_meminfo_var();
		// Skip the current cpu
		if(i == _cpu) 
			continue;

		s = send_meminfo_request(i);

		if(s > 0) {
			PRINTK("%s fill_next_remote_tgids: sleep ",__func__);
			ret = wait_event_interruptible(wq, wait != -1);
			wait = -1;

			if (!ret && meminfo_result) {
				/* Accumulate the meminfo */
				total->_MemTotal += meminfo_result->_MemTotal;
				total->_MemFree += meminfo_result->_MemFree;
				total->_Buffers += meminfo_result->_Buffers;
				total->_Cached += meminfo_result->_Cached;
				total->_SwapCached += meminfo_result->_SwapCached;
				total->_Active += meminfo_result->_Active;
				total->_Inactive += meminfo_result->_Inactive;
				total->_Active_anon += meminfo_result->_Active_anon;
				total->_Inactive_anon += meminfo_result->_Inactive_anon;
				total->_Active_file += meminfo_result->_Active_file;
				total->_Inactive_file += meminfo_result->_Inactive_file;
				total->_Unevictable += meminfo_result->_Unevictable;
				total->_Mlocked += meminfo_result->_Mlocked;

				#ifdef CONFIG_HIGHMEM
				total->_HighTotal += meminfo_result->_HighTotal;
				total->_HighFre += meminfo_result->_HighFree;
				total->_LowTotal += meminfo_result->_LowTotal;
				total->_LowFree += meminfo_result->_LowFree;
				#endif

				#ifndef CONFIG_MMU
				total->_MmapCopy += meminfo_result->_MmapCopy;
				#endif
				total->_SwapTotal += meminfo_result->_SwapTotal;
				total->_SwapFree += meminfo_result->_SwapFree,
				total->_Dirty += meminfo_result->_Dirty;
				total->_Writeback += meminfo_result->_Writeback;
				#ifdef CONFIG_TRANSPARENT_HUGEPAGE
				total->_AnonPages += meminfo_result->_AnonPages;
				#else
				total->_Mapped += meminfo_result->_Mapped;
				#endif
				total->_Shmem += meminfo_result->_Shmem;
				total->_Slab += meminfo_result->_Slab;
				total->_SReclaimable += meminfo_result->_SReclaimable;
				total->_SUnreclaim += meminfo_result->_SUnreclaim;
				total->_KernelStack += meminfo_result->_KernelStack;
				total->_PageTables += meminfo_result->_PageTables;
				#ifdef CONFIG_QUICKLIST
				total->_Quicklists += meminfo_result->_Quicklists;
				#endif
				total->_NFS_Unstable += meminfo_result->_NFS_Unstable;
				total->_Bounce += meminfo_result->_Bounce;
				total->_WritebackTmp += meminfo_result->_WritebackTmp;
				total->_CommitLimit += meminfo_result->_CommitLimit;
				total->_Committed_AS += meminfo_result->_Committed_AS;
				total->_VmallocTotal += meminfo_result->_VmallocTotal;
				total->_VmallocUsed += meminfo_result->_VmallocUsed;
				total->_VmallocChunk += meminfo_result->_VmallocChunk;
				#ifdef CONFIG_MEMORY_FAILURE
				total->_HardwareCorrupted += meminfo_result->_HardwareCorrupted;
				#endif
				#ifdef CONFIG_TRANSPARENT_HUGEPAGE
				total->_AnonHugePages += meminfo_result->_AnonHugePages;
				#endif
			}
		}
	}
	return 0;
}

static int __init proc_mem_handler_init(void)
{
	uint16_t copy_cpu = 0;
	//Sharath: TODO: Need to find the alternative for this
	if(pcn_kmsg_get_node_ids(NULL, 0, &copy_cpu)==-1)
		printk("ERROR: meminfo_remote: cannot initialize _cpu\n");
	else{
		_cpu= copy_cpu;
		printk("meminfo: I am cpu %d\n",_cpu);	
	}

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

