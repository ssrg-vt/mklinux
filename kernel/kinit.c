// Copyright (c) 2013 - 2014, Akshay
// modified by Antonio Barbalace (c) 2014

#include <linux/kernel.h>
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

#include <popcorn/process_server.h>

/*
 * Constant macros
 */
#define max_nodes 1 << 8
#define DISPLAY_BUFFER 128
#define PRINT_MESSAGES 0

/*
 * Function macros
 */
#if PRINT_MESSAGES
#define PRINTK(...) printk(__VA_ARGS__)
#else
#define PRINTK(...) ;
#endif

/*
 * External variables
 */
extern unsigned long orig_boot_params;
extern int my_cpu;

/*
 *  Variables
 */
unsigned int Kernel_Id;
EXPORT_SYMBOL(Kernel_Id);
// TODO this must be refactored
static int wait_cpu_list = -1;
static DECLARE_WAIT_QUEUE_HEAD( wq_cpu);
struct list_head rlist_head;
static _remote_cpu_info_response_t cpu_result;
extern unsigned int offset_cpus; //from kernel/smp.c
//struct cpumask cpu_global_online_mask;
#define for_each_global_online_cpu(cpu)   for_each_cpu((cpu), cpu_global_online_mask)

/*
 * Function definitions
 */
// TODO rewrite with a list
int flush_cpu_info_var(void)
{
  memset(&cpu_result, 0, sizeof(cpu_result)); //cpu_result = NULL;
  wait_cpu_list = -1;
  return 0;
}

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

static void display(struct list_head *head)
{
    struct list_head *iter;
    _remote_cpu_info_list_t *objPtr;
    char buffer[DISPLAY_BUFFER];

    list_for_each(iter, head) {
        objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);

	memset(buffer, 0, DISPLAY_BUFFER);
//	cpumask_scnprintf(buffer, (DISPLAY_BUFFER -1), &(objPtr->_data._cpumask));
	bitmap_scnprintf(buffer, (DISPLAY_BUFFER -1), &(objPtr->_data.cpumask), POPCORN_CPUMASK_BITS);
        printk("%s: cpu:%d %s off:%d\n", __func__,
		objPtr->_data._processor,
		buffer, objPtr->_data.cpumask_offset);
    }
}

static int handle_remote_proc_cpu_info_response(struct pcn_kmsg_message* inc_msg)
{
  _remote_cpu_info_response_t* msg = (_remote_cpu_info_response_t*) inc_msg;
  printk("%s: OCCHIO answer cpu request received\n", __func__);

  wait_cpu_list = 1;
  if (msg != NULL)
    memcpy(&cpu_result, msg, sizeof(_remote_cpu_info_response_t));

  wake_up_interruptible(&wq_cpu);
  printk("%s: response, wait_cpu_list{%d}\n", __func__, wait_cpu_list);

  pcn_kmsg_free_msg(inc_msg);
  return 0;
}

static int handle_remote_proc_cpu_info_request(struct pcn_kmsg_message* inc_msg)
{
  int i;
  _remote_cpu_info_request_t* msg = (_remote_cpu_info_request_t*) inc_msg;
  _remote_cpu_info_response_t response;

  printk("%s: Entered remote  cpu info request \n", "handle_remote_proc_cpu_info_request");

  // constructing response
  response.header.type = PCN_KMSG_TYPE_REMOTE_PROC_CPUINFO_RESPONSE;
  response.header.prio = PCN_KMSG_PRIO_NORMAL;
  //response._data._cpumask = kmalloc( sizeof(struct cpumask), GFP_KERNEL); //this is an error, how you can pass a pointer to another kernel?!i
  fill_cpu_info(&response._data);
#if 1
  bitmap_zero(&(response._data.cpumask), POPCORN_CPUMASK_BITS);
  bitmap_copy(&(response._data.cpumask), cpumask_bits(cpu_present_mask),
	(nr_cpu_ids > POPCORN_CPUMASK_BITS) ? POPCORN_CPUMASK_BITS : nr_cpu_ids);
#else
  memcpy(&(response._data._cpumask), cpu_present_mask, sizeof(struct cpumask));
#endif
  response._data.cpumask_offset = offset_cpus;
  response._data.cpumask_size = cpumask_size();
  response._data._processor = my_cpu;

  // Adding the new cpuset to the list
  add_node(&msg->_data,&rlist_head); //add_node copies the content
  display(&rlist_head);

  // Send response
  printk("Kerenel %d: handle_remote_proc_cpu_info_request\n", Kernel_Id);
  pcn_kmsg_send_long(msg->header.from_cpu,
		(struct pcn_kmsg_long_message*) (&response),
		sizeof(_remote_cpu_info_response_t) - sizeof(struct pcn_kmsg_hdr));
  // Delete received message
  pcn_kmsg_free_msg(inc_msg);
  return 0;
}

int send_cpu_info_request(int KernelId)
{
  int res = 0;
  _remote_cpu_info_request_t *request =
	kmalloc(sizeof(_remote_cpu_info_request_t), GFP_KERNEL);

  // Build request
  request->header.type = PCN_KMSG_TYPE_REMOTE_PROC_CPUINFO_REQUEST;
  request->header.prio = PCN_KMSG_PRIO_NORMAL;

  fill_cpu_info(&request->_data);

#if 1
  bitmap_zero(&(request->_data.cpumask), POPCORN_CPUMASK_BITS);
  bitmap_copy(&(request->_data.cpumask), cpumask_bits(cpu_present_mask),
	(nr_cpu_ids > POPCORN_CPUMASK_BITS) ? POPCORN_CPUMASK_BITS : nr_cpu_ids);
#else
  memcpy(&(request->_data._cpumask), cpu_present_mask, sizeof(struct cpumask));
#endif
  request->_data.cpumask_offset = offset_cpus;
  request->_data.cpumask_size = cpumask_size();
  request->_data._processor = my_cpu;

  // Send response
  printk("Kerenel %d: send_cpu_info_request\n", Kernel_Id);
  res = pcn_kmsg_send_long(KernelId, (struct pcn_kmsg_long_message*) (request),
			sizeof(_remote_cpu_info_request_t) - sizeof(struct pcn_kmsg_hdr));
  return res;
}

/*
 * Through this function a CPU broadcasts the cpuinfo to other CPUs
 * during the bootup. Called from init/main.c
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
	}
	printk("%s: Kernel_Id %d\n", __func__, Kernel_Id);
	//if(my_cpu != 0)
	if(Kernel_Id != 0)
	{
		printk("%s: OCCHIO checking other kernel\n", __func__);
		//result = send_cpu_info_request(i);
		// Sharath: 
		result = send_cpu_info_request(0);
		if (result != -1)
		{
			printk("OCCHIO waiting for answer proc cpu\n");
			PRINTK("%s : go to sleep!!!!", __func__);
			wait_event_interruptible(wq_cpu, wait_cpu_list != -1);
			wait_cpu_list = -1;

			add_node(&(cpu_result._data), &rlist_head);
			display(&rlist_head);
		}
	 }
	return 0;
}
EXPORT_SYMBOL(_init_RemoteCPUMask);

/*
 * Called from init/main.c
 */
void popcorn_init(void)
{
	int cnt=0;
	int vendor_id=0;
	uint16_t self = 0;

	printk("POP_INIT:first_online_node{%d} cpumask_first{%d} \n",first_online_node,cpumask_first(cpu_present_mask));

	//Kernel_Id = get_proccessor_id();//cpumask_first(cpu_present_mask);
	//Sharath: Below modifications to test using msg layer with socket
	self = cpumask_first(cpu_present_mask);
	pcn_kmsg_get_node_ids(NULL, 0, &self);
	Kernel_Id = self;

    printk("POP_INIT:Kernel id is %d\n",Kernel_Id);
    printk("POP_INIT:max_low_pfn id is 0x%lx\n",PFN_PHYS(max_low_pfn));
    printk("POP_INIT:min_low_pfn id is 0x%lx\n",PFN_PHYS(min_low_pfn));

}

static int __init cpu_info_handler_init(void)
{
  _cpu = my_cpu;
  INIT_LIST_HEAD(&rlist_head);
  offset_cpus=Kernel_Id;
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

