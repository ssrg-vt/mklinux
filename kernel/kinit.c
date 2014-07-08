// Copyright (c) 2013 - 2014, Akshay
// modified by Antonio Barbalace (c) 2014

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

#include <linux/process_server.h>

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
EXPORT_SYMBOL(Kernel_Id);

// TODO this must be refactored
static int _cpu=0;

/*
 *  Variables
 */
static int wait_cpu_list = -1;
static DECLARE_WAIT_QUEUE_HEAD( wq_cpu);
struct list_head rlist_head;

#include <linux/popcorn.h>

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
	bitmap_scnprintf(buffer, (DISPLAY_BUFFER -1), &(objPtr->_data.cpumask), POPCORN_CPUMASK_BITS);
        printk("%s: cpu:%d fam:%d %s off:%d\n", __func__,
		objPtr->_data._processor, objPtr->_data._cpu_family,
		buffer, objPtr->_data.cpumask_offset);
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
static _remote_cpu_info_response_t cpu_result;

/*
 * ******************************* Common Functions **********************************************************
 */

extern unsigned int offset_cpus; //from kernel/smp.c

struct cpumask cpu_global_online_mask;
#define for_each_global_online_cpu(cpu)   for_each_cpu((cpu), cpu_global_online_mask)

// TODO rewrite with a list
int flush_cpu_info_var(void)
{
  memset(&cpu_result, 0, sizeof(cpu_result)); //cpu_result = NULL;
  wait_cpu_list = -1;
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

  pcn_kmsg_free_msg(inc_msg);
  return 0;
}

extern int my_cpu;
static int handle_remote_proc_cpu_info_request(struct pcn_kmsg_message* inc_msg)
{
  int i;
  _remote_cpu_info_request_t* msg = (_remote_cpu_info_request_t*) inc_msg;
  _remote_cpu_info_response_t response;

  //printk("%s: OCCHIO request proc cpu received \n", __func__);

  /*printk("%s: kernel representative %d(%d), online cpus { ", 
         __func__, _cpu, my_cpu);
  for_each_online_cpu(i) {
    printk("%d, ", i);
  }
  printk("}\n");*/

  // constructing response
  response.header.type = PCN_KMSG_TYPE_REMOTE_PROC_CPUINFO_RESPONSE;
  response.header.prio = PCN_KMSG_PRIO_NORMAL;
  //response._data._cpumask = kmalloc( sizeof(struct cpumask), GFP_KERNEL); //this is an error, how you can pass a pointer to another kernel?!i
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
  res = pcn_kmsg_send_long(KernelId, (struct pcn_kmsg_long_message*) (request),
			sizeof(_remote_cpu_info_request_t) - sizeof(struct pcn_kmsg_hdr));
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
  printk("%s: inside \n",__func__);	
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

