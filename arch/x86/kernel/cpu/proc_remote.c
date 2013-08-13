/*
 * This file for Obtaining Remote CPU info
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


#include <popcorn/cpuinfo.h>


#include <linux/list.h>

#define PRINT_MESSAGES 0
#if PRINT_MESSAGES
#define PRINTK(...) printk(__VA_ARGS__)
#else
#define PRINTK(...) ;
#endif

/*
 *  Variables
 */
extern struct list_head rlist_head;
static int _cpu=-1;
/*
 * ******************************* Common Functions **********************************************************
 */

/*
 * ************************************* Function (hook) to be called from other file ********************
 */
int remote_proc_cpu_info(struct seq_file *m) {

	int res = 0;

	int result = 0;
	int i;
	int retval;

	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;

	    list_for_each(iter, &rlist_head) {
	    	  objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);

	    	  	  	  seq_printf(m, "*********Remote CPU*****\n");

	    	  			seq_printf(m, "processor\t: %u\n"
	    	  					"vendor_id\t: %s\n"
	    	  					"cpu family\t: %d\n"
	    	  					"model\t\t: %u\n"
	    	  					"model name\t: %s\n", objPtr->_data._processor,
	    	  					objPtr->_data._vendor_id, objPtr->_data._cpu_family,
	    	  					objPtr->_data._model, objPtr->_data._model_name);

	    	  			if (objPtr->_data._stepping != -1)
	    	  				seq_printf(m, "stepping\t: %d\n", objPtr->_data._stepping);
	    	  			else
	    	  				seq_printf(m, "stepping\t: unknown\n");

	    	  			seq_printf(m, "microcode\t: 0x%x\n", objPtr->_data._microcode);

	    	  			seq_printf(m, "cpu MHz\t\t: %u.%03u\n", objPtr->_data._cpu_freq);

	    	  			seq_printf(m, "cache size\t: %d KB\n", objPtr->_data._cache_size);

	    	  			seq_printf(m, "flags\t\t:");

	    	  			seq_printf(m, " %s", objPtr->_data._flags);

	    	  			seq_printf(m, "\nbogomips\t: %lu\n", objPtr->_data._nbogomips);
	    	  			// (c->loops_per_jiffy/(5000/HZ)) % 100);

	    	  			seq_printf(m, "TLB size\t: %d 4K pages\n", objPtr->_data._TLB_size);

	    	  			seq_printf(m, "clflush size\t: %u\n", objPtr->_data._clflush_size);
	    	  			seq_printf(m, "cache_alignment\t: %d\n",
	    	  					objPtr->_data._cache_alignment);
	    	  			seq_printf(m,
	    	  					"address sizes\t: %u bits physical, %u bits virtual\n",
	    	  					objPtr->_data._bits_physical, objPtr->_data._bits_virtual);

	    	  			seq_printf(m, "power management:");
	    	  			/*for (i = 0; i < 32; i++) {
	    	  			 if (c->x86_power & (1 << i)) {
	    	  			 if (i < ARRAY_SIZE(x86_power_flags) &&
	    	  			 x86_power_flags[i])
	    	  			 seq_printf(m, "%s%s",
	    	  			 x86_power_flags[i][0] ? " " : "",
	    	  			 x86_power_flags[i]);
	    	  			 else
	    	  			 seq_printf(m, " [%d]", i);
	    	  			 }
	    	  			 }*/
	    	  			seq_printf(m, "global cpumask available: \n");
	    	  			for_each_cpu(i,cpu_global_online_mask) {
	    	  				seq_printf(m,"%d,\t",i);
	    	  						 }
	    	  			seq_printf(m, "\n\n");
	    }



}

static int __init proc_cpu_handler_init(void)
{


    _cpu = smp_processor_id();

	return 0;
}
/**
 * Register remote proc cpu info init function as
 * module initialization function.
 */
late_initcall(proc_cpu_handler_init);
