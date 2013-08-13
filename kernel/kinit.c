

#include <linux/kernel.h>
#include <asm/bootparam.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#include <asm/setup.h>
#include <linux/slab.h>
#include <linux/highmem.h>

#include <linux/list.h>


#include <linux/smp.h>
#include <linux/cpu.h>
//#include <linux/cpumask.h>


#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/timex.h>
#include <linux/timer.h>
#include <linux/pcn_kmsg.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/cpufreq.h>

#include <popcorn/cpuinfo.h>

extern unsigned long orig_boot_params;
#define max_nodes 1 << 8



#define PRINT_MESSAGES 0
#if PRINT_MESSAGES
#define PRINTK(...) printk(__VA_ARGS__)
#else
#define PRINTK(...) ;
#endif

unsigned long *token_bucket;
unsigned int Kernel_Id;
unsigned long long bucket_phys_addr;

EXPORT_SYMBOL(Kernel_Id);
EXPORT_SYMBOL(bucket_phys_addr);

static int _cpu=0;

static int __init popcorn_kernel_init(char *arg)
{
	bucket_phys_addr= simple_strtoull (arg,0, 16);
	return 0;
}

early_param("kernel_init", popcorn_kernel_init);

void popcorn_init(void)
{

	int i=0;
	ssize_t bucket_size =sizeof(long)*max_nodes;


	printk("%s: POP_INIT:kernel bucket_phys_addr: 0x%lx\n","popcorn_init",
			  (unsigned long) bucket_phys_addr);
	printk("%s:POP_INIT:Called popcorn_init boot id--max_nodes :%d! %d\n","popcorn_init",max_nodes);

	token_bucket=ioremap_cache((resource_size_t)((void *) bucket_phys_addr),PAGE_SIZE);

	if (!token_bucket) {
				printk("Failed to kmalloc token_bucket !\n");
				unsigned long pfn = (long) bucket_phys_addr >> PAGE_SHIFT;
				struct page *shared_page;
				shared_page = pfn_to_page(pfn);
				token_bucket = page_address(shared_page);
				void * kmap_addr = kmap(shared_page);
			}

	PRINTK("%s: POP_INIT:token_bucket addr: 0x%p\n",__func__, token_bucket);
			for(i=0;i<max_nodes;i++)
			{
				if(token_bucket[i]==0)
				{   token_bucket[i]=1;
					Kernel_Id=i+1;break;
				}
			}

	PRINTK("%s: POP_INIT:token_bucket Initial values; \n",__func__);
	for(i=0;i<max_nodes;i++)
		{
		printk("%d\t",token_bucket[i]);
		}


	printk("POP_INIT:Virt add : 0x%p --- shm kernel id address: 0x%lx\n",token_bucket,bucket_phys_addr);

	/*int apicid, apicid_1;
    for (i = 0; i < NR_CPUS; i++) {
	apicid_1 = per_cpu(x86_bios_cpu_apicid, i);
	apicid = apic->cpu_present_to_apicid(i);
	printk("POP_INIT: The CPU is not present in the current present_mask (OK to continue), apicid = %d, apicid_1 = %d\n", apicid, apicid_1);
    }*/
    printk("POP_INIT:Kernel id is %d\n",Kernel_Id);
}



/*
 *  Variables
 */
static int wait = -1;

static DECLARE_WAIT_QUEUE_HEAD( wq);

struct list_head rlist_head;


/*
 * ****************************** Message structures for obtaining PID status ********************************
 */




void add_node(_remote_cpu_info_data_t *arg, struct list_head *head)
{
   _remote_cpu_info_list_t *Ptr = (_remote_cpu_info_list_t *)kmalloc(sizeof(struct _remote_cpu_info_list),GFP_KERNEL);
   // assert(Ptr != NULL);

    Ptr->_data = *arg;
    INIT_LIST_HEAD(&Ptr->cpu_list_member);
    list_add(&Ptr->cpu_list_member, head);
}



int find_and_delete(int cpuno, struct list_head *head)
{
    struct list_head *iter;
    _remote_cpu_info_list_t *objPtr;

    list_for_each(iter, head) {
        objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
        if(objPtr->_data._processor == cpuno) {
            list_del(&objPtr->cpu_list_member);
            kfree(objPtr);
            return 1;
        }
    }
}


void display(struct list_head *head)
{
    struct list_head *iter;
    _remote_cpu_info_list_t *objPtr;

    list_for_each(iter, head) {
        objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
        printk("%d \t", objPtr->_data._processor);
        printk("%d \t", objPtr->_data._cpu_family);
    }
    printk("\n");
}


struct _remote_cpu_info_request {
	struct pcn_kmsg_hdr header;
	_remote_cpu_info_data_t _data;
}__attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_cpu_info_request _remote_cpu_info_request_t;

struct _remote_cpu_info_response {
	struct pcn_kmsg_hdr header;
	_remote_cpu_info_data_t _data;
}__attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_cpu_info_response _remote_cpu_info_response_t;

/*
 * ******************************* Define variables holding Result *******************************************
 */
static _remote_cpu_info_response_t *cpu_result;

/*
 * ******************************* Common Functions **********************************************************
 */

int flush_cpu_info_var() {
	cpu_result = NULL;
	wait = -1;
	return 0;
}

static void *remote_c_start(loff_t *pos) {
	if (*pos == 0) /* just in case, cpu 0 is not the first */
		*pos = cpumask_first(cpu_online_mask);
	else
		*pos = cpumask_next(*pos - 1, cpu_online_mask);
	if ((*pos) < nr_cpu_ids)
		return &cpu_data(*pos);
	return NULL;
}

int fill_cpu_info(_remote_cpu_info_data_t *res) {

	void *p;
	loff_t pos = 0;
	p = remote_c_start(&pos);

	struct cpuinfo_x86 *c = p;
	unsigned int cpu = 0;
	int i;

#ifdef CONFIG_SMP
	cpu = c->cpu_index;
#endif


	res->_processor = cpu;
	strcpy(res->_vendor_id, c->x86_vendor_id[0] ? c->x86_vendor_id : "unknown");
	res->_cpu_family = c->x86;
	res->_model = c->x86_model;
	strcpy(res->_model_name, c->x86_model_id[0] ? c->x86_model_id : "unknown");

	if (c->x86_mask || c->cpuid_level >= 0)
		res->_stepping = c->x86_mask;
	else
		res->_stepping = -1;

	if (c->microcode)
		res->_microcode = c->microcode;

	if (cpu_has(c, X86_FEATURE_TSC)) {
		unsigned int freq = cpufreq_quick_get(cpu);

		if (!freq)
			freq = cpu_khz;
		res->_cpu_freq = freq / 1000, (freq % 1000);
	}

	/* Cache size */
	if (c->x86_cache_size >= 0)
		res->_cache_size = c->x86_cache_size;

	strcpy(res->_fpu, "yes");
	strcpy(res->_fpu_exception, "yes");
	res->_cpuid_level = c->cpuid_level;
	strcpy(res->_wp, "yes");

	strcpy(res->_flags, "");
	//strcpy(res->_flags,"flags\t\t:");
	for (i = 0; i < 32 * NCAPINTS; i++)
		if (cpu_has(c, i) && x86_cap_flags[i] != NULL)
			strcat(res->_flags, x86_cap_flags[i]);

	res->_nbogomips = c->loops_per_jiffy / (500000 / HZ);
	//(c->loops_per_jiffy/(5000/HZ)) % 100);

#ifdef CONFIG_X86_64
	if (c->x86_tlbsize > 0)
	res->_TLB_size= c->x86_tlbsize;
#endif
	res->_clflush_size = c->x86_clflush_size;
	res->_cache_alignment = c->x86_cache_alignment;
	res->_bits_physical = c->x86_phys_bits;
	res->_bits_virtual = c->x86_virt_bits;

	strcpy(res->_power_management, "");
	for (i = 0; i < 32; i++) {
		if (c->x86_power & (1 << i)) {
			if (i < ARRAY_SIZE(x86_power_flags) && x86_power_flags[i])
				strcat(res->_flags, x86_power_flags[i][0] ? " " : "");
			//  x86_power_flags[i]);

			//seq_printf(m, " [%d]", i);
		}
	}
	res->_cpumask = kmalloc(
				sizeof(struct cpumask),
				GFP_KERNEL);
	res->_cpumask =cpu_present_mask;

	return 0;
}

static int handle_remote_proc_cpu_info_response(
		struct pcn_kmsg_message* inc_msg) {
	_remote_cpu_info_response_t* msg = (_remote_cpu_info_response_t*) inc_msg;

	printk("%s: Entered remote cpu info response \n", "handle_remote_proc_cpu_info_response");

	wait = 1;
	if (msg != NULL)
		cpu_result = msg;
	wake_up_interruptible(&wq);
	printk("%s: response ---- wait{%d} \n", "handle_remote_proc_cpu_info_response", wait);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

static int handle_remote_proc_cpu_info_request(struct pcn_kmsg_message* inc_msg) {

	printk("%s : cpus online in kernel %d!!!", "handle_remote_proc_cpu_info_request",_cpu);

	int i;
		 for_each_online_cpu(i) {
			 printk("%d %t", i);
		 }
		 printk("\n");
	_remote_cpu_info_request_t* msg = (_remote_cpu_info_request_t*) inc_msg;
	_remote_cpu_info_response_t response;

	printk("%s: Entered remote  cpu info request \n", "handle_remote_proc_cpu_info_request");

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_PROC_CPUINFO_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	cpumask_or(cpu_global_online_mask,cpu_global_online_mask,(const struct cpumask *)(msg->_data._cpumask));

	add_node(&msg->_data,&rlist_head);

	fill_cpu_info(&response._data);

	display(&rlist_head);

	printk("%s : global cpus online in kernel %d!!!", "handle_remote_proc_cpu_info_request",_cpu);

			for_each_global_online_cpu(i) {
				printk("%d %t", i);
				 }
			printk("\n");

	// Send response
	pcn_kmsg_send_long(msg->header.from_cpu,
			(struct pcn_kmsg_message*) (&response),
			sizeof(_remote_cpu_info_response_t) - sizeof(struct pcn_kmsg_hdr));

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

int send_cpu_info_request(int KernelId) {

	int res = 0;
	_remote_cpu_info_request_t* request = kmalloc(
			sizeof(_remote_cpu_info_request_t),
			GFP_KERNEL);
	// Build request
	request->header.type = PCN_KMSG_TYPE_REMOTE_PROC_CPUINFO_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
	request->_data._cpumask = kmalloc(
			sizeof(struct cpumask),
			GFP_KERNEL);
	//request->_data._cpumask =cpu_present_mask;


	fill_cpu_info(&request->_data);
	// Send response
	res = pcn_kmsg_send_long(KernelId, (struct pcn_kmsg_message*) (request),
			sizeof(_remote_cpu_info_request_t) - sizeof(struct pcn_kmsg_hdr));
	return res;
}

/*
 * ************************************* Function (hook) to be called from other file ********************
 */
int _init_RemoteCPUMask(void)
{


	unsigned int i;
	printk("%s : cpus online in kernel %d!!!", "_init_RemoteCPUMask",_cpu);


	flush_cpu_info_var();
		int res = 0;

		int result = 0;
		int retval;

		for (i = 0; i < NR_CPUS; i++) {

			flush_cpu_info_var();
			// Skip the current cpu
			if (i == _cpu)
				continue;
			result = send_cpu_info_request(i);

			if (!result) {

				PRINTK("%s : go to sleep!!!!", __func__);
							wait_event_interruptible(wq, wait != -1);
							wait = -1;

							cpumask_or(cpu_global_online_mask,cpu_global_online_mask,(const struct cpumask *)(cpu_result->_data._cpumask));

							add_node(&cpu_result->_data,&rlist_head);

							display(&rlist_head);

			}
		}

		printk("%s : global cpus online in kernel %d!!!", "_init_RemoteCPUMask",_cpu);

		for_each_cpu(i,cpu_global_online_mask) {
			printk("------%d %t", i);
			 }
		printk("\n");
	return 0;
}


static int __init cpu_info_handler_init(void)
{


    _cpu = smp_processor_id();

    INIT_LIST_HEAD(&rlist_head);

  /*  ptrlist = &rlist;

    ptrlist->_data=NULL;

    INIT_LIST_HEAD(&ptrlist->cpu_list);*/

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_PROC_CPUINFO_REQUEST,
				    		handle_remote_proc_cpu_info_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_PROC_CPUINFO_RESPONSE,
							handle_remote_proc_cpu_info_response);


	return 0;
}
/**
 * Register remote pid init function as
 * module initialization function.
 */
late_initcall(cpu_info_handler_init);

