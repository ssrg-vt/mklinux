/*
 * File:
  * proc_remote.c
 *
 * Description:
 * 	this file provides the proc/cpuinfo for remote cores
 *
 * Created on:
 * 	Oct 10, 2014
 *
 * Author:
 *  Akshay Giridhar, SSRG, VirginiaTech
 *  Sharath Kumar Bhat, SSRG, VirginiaTech
 *
 */

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/seq_file.h>
#include <linux/popcorn_cpuinfo.h>
#include <linux/list.h>

/*
 *  Variables
 */
extern struct list_head rlist_head;

void print_x86_cpuinfo(struct seq_file *m, _remote_cpu_info_list_t *objPtr, int count){
	seq_printf(m, "\nprocessor\t: %u\n"
			"vendor_id\t: %s\n"
			"cpu family\t: %d\n"
			"model\t\t: %u\n"
			"model name\t: %s\n", objPtr->_data.arch.x86.cpu[count]._processor,
	objPtr->_data.arch.x86.cpu[count]._vendor_id, objPtr->_data.arch.x86.cpu[count]._cpu_family,
	objPtr->_data.arch.x86.cpu[count]._model, objPtr->_data.arch.x86.cpu[count]._model_name);

	if (objPtr->_data.arch.x86.cpu[count]._stepping != -1)
		seq_printf(m, "stepping\t: %d\n", objPtr->_data.arch.x86.cpu[count]._stepping);
	else
		seq_printf(m, "stepping\t: unknown\n");

	seq_printf(m, "microcode\t: 0x%x\n", objPtr->_data.arch.x86.cpu[count]._microcode);
	seq_printf(m, "cpu MHz\t\t: %u.%03u\n", objPtr->_data.arch.x86.cpu[count]._cpu_freq);
	seq_printf(m, "cache size\t: %d KB\n", objPtr->_data.arch.x86.cpu[count]._cache_size);
	seq_printf(m, "flags\t\t:");
	seq_printf(m, " %s", objPtr->_data.arch.x86.cpu[count]._flags);
	seq_printf(m, "\nbogomips\t: %lu\n", objPtr->_data.arch.x86.cpu[count]._nbogomips);
	seq_printf(m, "TLB size\t: %d 4K pages\n", objPtr->_data.arch.x86.cpu[count]._TLB_size);
	seq_printf(m, "clflush size\t: %u\n", objPtr->_data.arch.x86.cpu[count]._clflush_size);
	seq_printf(m, "cache_alignment\t: %d\n", objPtr->_data.arch.x86.cpu[count]._cache_alignment);
	seq_printf(m, "address sizes\t: %u bits physical, %u bits virtual\n",
			objPtr->_data.arch.x86.cpu[count]._bits_physical,
			objPtr->_data.arch.x86.cpu[count]._bits_virtual);
}

void print_arm_cpuinfo(struct seq_file *m, _remote_cpu_info_list_t *objPtr){
	
	int i =0;
		
	seq_printf(m, "processor\t: %s %s\n\n", objPtr->_data.arch.arm64.__processor,\
					objPtr->_data.arch.arm64.per_core[i].model_name);

	for(i=0;i<objPtr->_data.arch.arm64.num_cpus;i++)
	{
		seq_printf(m, "processor\t: %u\n", objPtr->_data.arch.arm64.per_core[i].processor_id);

		seq_printf(m, "model name\t: %s\n", objPtr->_data.arch.arm64.per_core[i].model_name);

		seq_printf(m, "cpu MHz\t\t: %lu.%02lu\n",
			   	objPtr->_data.arch.arm64.per_core[i].cpu_freq / 1000000UL,
			   	objPtr->_data.arch.arm64.per_core[i].cpu_freq % 1000000UL);
		seq_printf(m, "fpu\t\t: %s\n\n", objPtr->_data.arch.arm64.per_core[i].fpu);
	}

	/* dump out the processor features */
	seq_puts(m, "Features\t: ");

	seq_printf(m, "\nCPU implementer\t: 0x%02x\n", objPtr->_data.arch.arm64.cpu_implementer);
	seq_printf(m, "CPU architecture: %s\n", objPtr->_data.arch.arm64.cpu_arch);
	seq_printf(m, "CPU variant\t: 0x%x\n", objPtr->_data.arch.arm64.cpu_variant);
	seq_printf(m, "CPU part\t: 0x%03x\n", objPtr->_data.arch.arm64.cpu_part);
	seq_printf(m, "CPU revision\t: %u\n", objPtr->_data.arch.arm64.cpu_revision);
}

void print_unknown_cpuinfo(struct seq_file *m, _remote_cpu_info_list_t *objPtr){
	seq_printf(m, "processor\t: Unknown\n"
				"vendor_id\t: Unknown\n"
				"cpu family\t: Unknown\n"
				"model\t\t: Unknown\n"
				"model name\t: Unknown\n");
}

int remote_proc_cpu_info(struct seq_file *m) {
	int res = 0;
	int result = 0;
	int i;
	int retval;
	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;

	//printk(" In remote cpuinfo\n");
	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		seq_printf(m, "\n*********Remote CPU*****\n");
		switch(objPtr->_data.arch_type){
			case arch_x86: {
				for(i=0;i<objPtr->_data.arch.x86.num_cpus; i++)
					print_x86_cpuinfo(m, objPtr, i);
			}
			break;

			case arch_arm: {
				print_arm_cpuinfo(m, objPtr);
			}
			break;

			default: {
				print_unknown_cpuinfo(m, objPtr);
			}
			break;
		}
		seq_printf(m, "global cpumask available: \n");
		for_each_cpu(i,cpu_global_online_mask) {
			seq_printf(m,"%d,\t",i);
		}
		seq_printf(m, "\n\n");
	}
	//printk(" leaving remote cpuinfo\n");
}
