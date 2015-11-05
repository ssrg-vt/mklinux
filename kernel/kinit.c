// Copyright (c) 2013 - 2014, Akshay
// modified by Antonio Barbalace

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

unsigned long *token_bucket;
unsigned int Kernel_Id;
unsigned long long bucket_phys_addr = 0;

EXPORT_SYMBOL(Kernel_Id);
EXPORT_SYMBOL(bucket_phys_addr);

static int _cpu=0;

static int __init popcorn_kernel_init(char *arg)
{
	bucket_phys_addr= simple_strtoull (arg,0, 16);
	return 0;
}

early_param("kernel_init", popcorn_kernel_init);




/*
 *  Variables
 */
static int wait_cpu_list = -1;

static DECLARE_WAIT_QUEUE_HEAD( wq_cpu);

struct list_head rlist_head;


///////////////////////////////////////////////////////////////////////////////
#include <linux/popcorn.h>
/*
 struct _remote_cpu_info_data
 {
                 unsigned int _processor;
                 char _vendor_id[16];
                 int _cpu_family;
                unsigned int _model;
                 char _model_name[64];
                 int _stepping;
                 unsigned long _microcode;
                 unsigned _cpu_freq;
                 int _cache_size;
                 char _fpu[3];
                 char _fpu_exception[3];
                 int _cpuid_level;
                 char _wp[3];
                 char _flags[512];
                 unsigned long _nbogomips;
                int _TLB_size;
                unsigned int _clflush_size;
                int _cache_alignment;
                unsigned int _bits_physical;
                 unsigned int _bits_virtual;
                char _power_management[64];
                 struct cpumask _cpumask;
 };


typedef struct _remote_cpu_info_data _remote_cpu_info_data_t;
struct _remote_cpu_info_list
 {
         _remote_cpu_info_data_t _data;
         struct list_head cpu_list_member;
 
  };
typedef struct _remote_cpu_info_list _remote_cpu_info_list_t;
*/
void popcorn_init(void)
{
	
	Kernel_Id=smp_processor_id();;
	printk("POP_INIT:Kernel id is %d\n",Kernel_Id);

}

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


static void display(struct list_head *head)
{
    struct list_head *iter;
    _remote_cpu_info_list_t *objPtr;
    char buffer[128];

    list_for_each(iter, head) {
        objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);

	memset(buffer, 0, 128);
	cpumask_scnprintf(buffer, 127, &(objPtr->_data._cpumask));
        printk("%s: cpu:%d fam:%d %s\n", __func__,
		objPtr->_data._processor, objPtr->_data._cpu_family,
		buffer);
    }
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
static _remote_cpu_info_response_t *cpu_result;

/*
 * ******************************* Common Functions **********************************************************
 */

int flush_cpu_info_var() {
	cpu_result = NULL;
	wait_cpu_list = -1;
	return 0;
}
struct cpumask cpu_global_online_mask;
#define for_each_global_online_cpu(cpu)   for_each_cpu((cpu), cpu_global_online_mask)
static int handle_remote_proc_cpu_info_response(struct pcn_kmsg_message* inc_msg)
{
  _remote_cpu_info_response_t* msg = (_remote_cpu_info_response_t*) inc_msg;
  printk("%s: Entered remote cpu info response \n", "handle_remote_proc_cpu_info_response");

  wait_cpu_list = 1;
  if (msg != NULL)
    cpu_result = msg;

  wake_up_interruptible(&wq_cpu);
  printk("%s: response ---- wait_cpu_list{%d} \n", "handle_remote_proc_cpu_info_response", wait_cpu_list);

  pcn_kmsg_free_msg(inc_msg);
  return 0;
}

static int handle_remote_proc_cpu_info_request(struct pcn_kmsg_message* inc_msg)
{
  int i;
  
  printk("%s : cpus online in kernel %d!!!", "handle_remote_proc_cpu_info_request",_cpu);
  for_each_online_cpu(i) {
    printk("%d ", i);
  }
  printk("\n");
  
  _remote_cpu_info_request_t* msg = (_remote_cpu_info_request_t*) inc_msg;
  _remote_cpu_info_response_t response;

  printk("%s: Entered remote  cpu info request \n", "handle_remote_proc_cpu_info_request");

  // Finish constructing response
  response.header.type = PCN_KMSG_TYPE_REMOTE_PROC_CPUINFO_RESPONSE;
  response.header.prio = PCN_KMSG_PRIO_NORMAL;
//  response._data._cpumask = kmalloc( sizeof(struct cpumask), GFP_KERNEL); //this is an error, how you can pass a pointer to another kernel?!
  memcpy(&(response._data._cpumask), cpu_present_mask, sizeof(cpu_present_mask));
extern int my_cpu;
  response._data._processor = my_cpu;

/*  cpumask_or(&cpu_global_online_mask,&cpu_global_online_mask,(const struct cpumask *)(msg->_data._cpumask));
*/  add_node(&msg->_data,&rlist_head);

  display(&rlist_head);

  printk("%s : global cpus online in kernel %d!!!", "handle_remote_proc_cpu_info_request",_cpu);
  /*for_each_global_online_cpu(i) {
    printk("%d %t", i);
  }
  printk("\n");
*/
  // Send response
  pcn_kmsg_send_long(msg->header.from_cpu,
		(struct pcn_kmsg_message*) (&response),
		sizeof(_remote_cpu_info_response_t) - sizeof(struct pcn_kmsg_hdr));
  
  pcn_kmsg_free_msg(inc_msg);
  return 0;
}

int send_cpu_info_request(int KernelId)
{

	int res = 0;
	_remote_cpu_info_request_t* request = kmalloc(
			sizeof(_remote_cpu_info_request_t),
			GFP_KERNEL);
	// Build request
	request->header.type = PCN_KMSG_TYPE_REMOTE_PROC_CPUINFO_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
//	request->_data._cpumask = kmalloc( sizeof(struct cpumask), GFP_KERNEL);
	memcpy(&(request->_data._cpumask), cpu_present_mask, sizeof(cpu_present_mask));
	extern int my_cpu;
	request->_data._processor = my_cpu;

	// Send response
	res = pcn_kmsg_send_long(KernelId, (struct pcn_kmsg_message*) (request),
			sizeof(_remote_cpu_info_request_t) - sizeof(struct pcn_kmsg_hdr));
	kfree(request);

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

//should we add self?!

  for (i = 0; i < NR_CPUS; i++) { 
    flush_cpu_info_var();
    
    // Skip the current cpu
    //if (i == _cpu)
    if (cpumask_test_cpu(i, cpu_present_mask)) {
	printk("%s: cpu already known %i continue.\n", __func__,  i);
    	continue;
    }
    printk("%s: checking cpu %d.\n", __func__, i);
    result = send_cpu_info_request(i);
    if (!result) {
      PRINTK("%s : go to sleep!!!!", __func__);
      wait_event_interruptible(wq_cpu, wait_cpu_list != -1);
      wait_cpu_list = -1;

// TODO      
//      cpumask_or(cpu_global_online_mask,cpu_global_online_mask,(const struct cpumask *)(cpu_result->_data._cpumask));

      add_node(&cpu_result->_data,&rlist_head);
      display(&rlist_head);
    }
  }

  printk("%s : global cpus online in kernel %d!!!", "_init_RemoteCPUMask",_cpu);
/*  for_each_cpu(i,cpu_global_online_mask) {
    printk("------%d %t", i);
  }
  
  printk("\n");
*/  return 0;
}


static int __init cpu_info_handler_init(void)
{
    _cpu = smp_processor_id();

    INIT_LIST_HEAD(&rlist_head);

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

