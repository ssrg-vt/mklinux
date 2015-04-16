// Copyright (c) 2013 - 2014, Akshay

#include <linux/kernel.h>
#include <linux/module.h>
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
#include <linux/jhash.h>
#include <linux/cpufreq.h>

#include <linux/popcorn_cpuinfo.h>
#include <linux/bootmem.h>
//#include <linux/multikernel.h>


extern unsigned long orig_boot_params;
#define max_nodes 1 << 8

#define PRINT_MESSAGES 0
#if PRINT_MESSAGES
#define PRINTK(...) printk(__VA_ARGS__)
#else
#define PRINTK(...) ;
#endif


static inline int __getproccessor(void)
{
	unsigned int a,b,feat;

	asm volatile(
		     "cpuid"                       // call cpuid
		     : "=a" (a), "=b" (b), "=d" (feat)           // outputs
		     : "0" (1)                                   // inputs
		     : "cx" );
	if(feat & (1 << 25)) //TODO: Need to be refactored
		return 0;
	return 1;
}

unsigned long *token_bucket;

unsigned int Kernel_Id;
EXPORT_SYMBOL(Kernel_Id);

// TODO this must be refactored
static int _cpu=0;

/*
 *  Variables
 */
 

typedef enum allVendors {
	    AuthenticAMD,
	    GenuineIntel,
	    unknown
} vendor;




static int wait_cpu_list = -1;

static DECLARE_WAIT_QUEUE_HEAD( wq_cpu);

struct list_head rlist_head;

//#include <linux/popcorn.h>

/*void popcorn_init (void)
{
  Kernel_Id=smp_processor_id();
  printk("POP_INIT:Kernel id is %d\n",Kernel_Id);
}
*/

/*****************************************************************************
 * Message structures for obtaining PID status                               *
 *****************************************************************************/
void add_node(_remote_cpu_info_data_t *arg, struct list_head *head)
{
  _remote_cpu_info_list_t *Ptr =
	(_remote_cpu_info_list_t *)kmalloc(sizeof(_remote_cpu_info_list_t), GFP_KERNEL);
  if (!Ptr) {
    printk(KERN_ALERT"%s: can not allocate memory for kernel node descriptor\n", __func__);
    return;
  }
  printk("%s: _remote_cpu_info_list_t %ld, _remote_cpu_info_data_t %ld\n",
	__func__, sizeof(_remote_cpu_info_list_t), sizeof(_remote_cpu_info_data_t) );

  INIT_LIST_HEAD(&(Ptr->cpu_list_member));
  memcpy(&(Ptr->_data), arg, sizeof(_remote_cpu_info_data_t)); //Ptr->_data = *arg;
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
    return 0;
}

#define DISPLAY_BUFFER 128
static void display(struct list_head *head)
{
    struct list_head *iter;
    _remote_cpu_info_list_t *objPtr;
    char buffer[DISPLAY_BUFFER];

    list_for_each(iter, head) {
        objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);

	memset(buffer, 0, DISPLAY_BUFFER);
//	cpumask_scnprintf(buffer, (DISPLAY_BUFFER -1), &(objPtr->_data._cpumask));
	bitmap_scnprintf(buffer, (DISPLAY_BUFFER -1), (const long unsigned int*) &(objPtr->_data.cpumask), POPCORN_CPUMASK_BITS);
        printk("%s: cpu:%d fam:%d %s off:%d\n", __func__,
		objPtr->_data._processor, objPtr->_data._cpu_family,
		buffer, objPtr->_data.cpumask_offset);
    }
}
void popcorn_init(void)
{
	int vendor_id=0;
	struct cpuinfo_x86 *c = &boot_cpu_data;
	printk("POP_INIT:first_online_node{%d} cpumask_first{%d} \n",first_online_node,cpumask_first(cpu_present_mask));

	if(!strcmp(((const char *) c->x86_vendor_id),((const char *)"AuthenticAMD"))){
		vendor amd = AuthenticAMD;
		vendor_id = amd;
	}
	else if(!strcmp(((const char *) c->x86_vendor_id),((const char *) "GenuineIntel"))){
		vendor intel = GenuineIntel;
		vendor_id = intel;
	}
	printk("POP_INIT:vendor{%s} cpufam{%d} model{%u} cpucnt{%d} jhas{%u}\n",c->x86_vendor_id[0] ? c->x86_vendor_id : "unknown",c->x86,c->x86_model,vendor_id, (jhash_2words((u32)vendor_id,cpumask_first(cpu_present_mask), JHASH_INITVAL) & ((1<<8)-1)));
	
	
	Kernel_Id=__getproccessor();//cpumask_first(cpu_present_mask);

    printk("POP_INIT:Kernel id is %d\n",Kernel_Id);
    //printk("POP_INIT: kernel start add is 0x%lx",kernel_start_addr);
    printk("POP_INIT:max_low_pfn id is 0x%llx\n", PFN_PHYS(max_low_pfn));
    printk("POP_INIT:min_low_pfn id is 0x%llx\n", PFN_PHYS(min_low_pfn));

}
///////////////////////////////////////////////////////////////////////////////

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
static _remote_cpu_info_response_t cpu_result;

/*
 * ******************************* Common Functions **********************************************************
 */

extern unsigned int offset_cpus; //from kernel/smp.c

//struct cpumask cpu_global_online_mask;
#define for_each_global_online_cpu(cpu)   for_each_cpu((cpu), cpu_global_online_mask)

// TODO rewrite with a list
int flush_cpu_info_var(void)
{
  memset(&cpu_result, 0, sizeof(cpu_result)); //cpu_result = NULL;
  wait_cpu_list = -1;
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
	struct cpuinfo_x86 *c;
	unsigned int cpu = 0;
	int i;
	p = remote_c_start(&pos);
	c = p;

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
		res->_cpu_freq = freq / 1000; //, (freq % 1000);
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

	return 0;
}
static int handle_remote_proc_cpu_info_response(struct pcn_kmsg_message* inc_msg)
{
  _remote_cpu_info_response_t* msg = (_remote_cpu_info_response_t*) inc_msg;
  //printk("%s: OCCHIO answer cpu request received\n", __func__);

  wait_cpu_list = 1;
  if (msg != NULL)
    memcpy(&cpu_result, msg, sizeof(_remote_cpu_info_response_t));

  wake_up_interruptible(&wq_cpu);
  printk("%s: response, wait_cpu_list{%d}\n", __func__, wait_cpu_list);

  pcn_kmsg_free_msg_now(inc_msg);
  return 0;
}

extern int my_cpu;
static int handle_remote_proc_cpu_info_request(struct pcn_kmsg_message* inc_msg)
{
  _remote_cpu_info_request_t* msg = (_remote_cpu_info_request_t*) inc_msg;
  _remote_cpu_info_response_t *response = (_remote_cpu_info_response_t *) pcn_kmsg_alloc_msg(sizeof(_remote_cpu_info_response_t));

  printk("%s: Entered remote  cpu info request \n", "handle_remote_proc_cpu_info_request");


  // constructing response
  response->header.type = PCN_KMSG_TYPE_REMOTE_PROC_CPUINFO_RESPONSE;
  response->header.prio = PCN_KMSG_PRIO_NORMAL;
  //response._data._cpumask = kmalloc( sizeof(struct cpumask), GFP_KERNEL); //this is an error, how you can pass a pointer to another kernel?!i
  fill_cpu_info(&(response->_data));
#if 1
  bitmap_zero((long unsigned int*)&(response->_data.cpumask), POPCORN_CPUMASK_BITS);
  bitmap_copy((long unsigned int*)&(response->_data.cpumask), cpumask_bits(cpu_present_mask),
	(nr_cpu_ids > POPCORN_CPUMASK_BITS) ? POPCORN_CPUMASK_BITS : nr_cpu_ids);
#else
  memcpy(&(response->_data._cpumask), cpu_present_mask, sizeof(struct cpumask));
#endif
  response->_data.cpumask_offset = offset_cpus;
//TODO aggiungere numero bit
  response->_data.cpumask_size = cpumask_size();
  response->_data._processor = my_cpu;

  // Adding the new cpuset to the list
  add_node(&(msg->_data),&rlist_head); //add_node copies the content
  display(&rlist_head);
  //notify_cpu_ns(); <<<< <<<< <<<< <<<< <<<< <<<< <<<< <<<<
  //cpumask_or(&cpu_global_online_mask,&cpu_global_online_mask,(const struct cpumask *)(msg->_data._cpumask));
 /* printk("%s: kernel %d, global online cpus { ",
	 __func__, _cpu);
  for_each_global_online_cpu(i) {
    printk("%d, ", i);
  }
  printk("}\n");*/



  // Send response
  pcn_kmsg_send_long(msg->header.from_cpu,
		(struct pcn_kmsg_long_message*) response,
		sizeof(_remote_cpu_info_response_t) - sizeof(struct pcn_kmsg_hdr));
  // Delete received message
  pcn_kmsg_free_msg_now(inc_msg);
  pcn_kmsg_free_msg(response);
  return 0;
}

int send_cpu_info_request(int KernelId)
{
  int res = 0;
  _remote_cpu_info_request_t *request =
	pcn_kmsg_alloc_msg(sizeof(_remote_cpu_info_request_t));

  // Build request
  request->header.type = PCN_KMSG_TYPE_REMOTE_PROC_CPUINFO_REQUEST;
  request->header.prio = PCN_KMSG_PRIO_NORMAL;
  
  fill_cpu_info(&request->_data);
  
#if 1
  bitmap_zero((long unsigned int*)&(request->_data.cpumask), POPCORN_CPUMASK_BITS);
  bitmap_copy((long unsigned int*)&(request->_data.cpumask), cpumask_bits(cpu_present_mask),
	(nr_cpu_ids > POPCORN_CPUMASK_BITS) ? POPCORN_CPUMASK_BITS : nr_cpu_ids);
#else
  memcpy(&(request->_data._cpumask), cpu_present_mask, sizeof(struct cpumask));
#endif
  request->_data.cpumask_offset = offset_cpus;
  request->_data.cpumask_size = cpumask_size();
  request->_data._processor = my_cpu;

  // Send response
  res = pcn_kmsg_send_long(KernelId, (struct pcn_kmsg_long_message*) (request),
			sizeof(_remote_cpu_info_request_t) - sizeof(struct pcn_kmsg_hdr));
  pcn_kmsg_free_msg(request);
  return res;
}

/*
 * ************************************* Function (hook) to be called from other file ********************
 */
int _init_RemoteCPUMask(void)
{
  unsigned int i;
  int result = 0;

  printk("%s: kernel representative %d(%d)\n", __func__, _cpu, my_cpu);

  //TODO should we add self to the node list?! Actually all the code runs without self

  for (i = 0; i < NR_CPUS; i++) { 
    flush_cpu_info_var();
    
    if (cpumask_test_cpu(i, cpu_present_mask)) {
      printk("%s: cpu already known %i, continue\n", __func__,  i);
      continue;
    }

  /*  printk("%s: checking cpu %d\n", __func__, i);
    result = send_cpu_info_request(i);
    if (!result) {
      PRINTK("%s : go to sleep!!!!", __func__);
      wait_event_interruptible(wq_cpu, wait_cpu_list != -1);
      wait_cpu_list = -1;

      //cpumask_or(cpu_global_online_mask,cpu_global_online_mask,(const struct cpumask *)(cpu_result->_data._cpumask));
      add_node(&(cpu_result._data), &rlist_head);
      display(&rlist_head);
    }*/
  }
	if(my_cpu!=0)
	{
	//printk("%s: OCCHIO checking other kernel\n", __func__);
	result = send_cpu_info_request(0);
		if (result!=-1) 
		{
			//printk("OCCHIO waiting for answer proc cpu\n");
			PRINTK("%s : go to sleep!!!!", __func__);
			wait_event_interruptible(wq_cpu, wait_cpu_list != -1);
			wait_cpu_list = -1;

		  //cpumask_or(cpu_global_online_mask,cpu_global_online_mask,(const struct cpumask *)(cpu_result->_data._cpumask));
			add_node(&(cpu_result._data), &rlist_head);
			display(&rlist_head);
		}
     }
else{
//printk("OCCHIO other kernel not reacheble error is %d\n",result);

}
      //

  /*printk("%s: kernel %d, global online cpus { ",
	 __func__, _cpu);
  for_each_cpu(i,cpu_global_online_mask) {
    printk("%d, ", i);
  }
  printk("}\n");*/

  return 0;
}


static int __init cpu_info_handler_init(void)
{
#ifndef SUPPORT_FOR_CLUSTERING
  _cpu = smp_processor_id();
#else
  _cpu = my_cpu;
#endif
  INIT_LIST_HEAD(&rlist_head);

#ifdef CONFIG_X86_EARLYMIC
offset_cpus= 4;
#else
offset_cpus=0;
#endif
  printk("%s: inside , offsetcpus %d \n",__func__,offset_cpus);	
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

