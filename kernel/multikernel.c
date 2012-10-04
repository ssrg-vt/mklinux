/*
 * Boot parameters and other support stuff for MKLinux
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2012
 */

#include <linux/rcupdate.h>
#include <linux/rculist.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/gfp.h>
#include <linux/smp.h>
#include <linux/cpu.h>
#include <linux/syscalls.h>

extern unsigned long orig_boot_params;

int mklinux_boot;
EXPORT_SYMBOL(mklinux_boot);

static int __init setup_mklinux(char *arg)
{
        mklinux_boot = 1;
        return 0;
}
early_param("mklinux", setup_mklinux);


/* We're going to put our syscall here, since we need to pass in
   two arguments but the reboot syscall only takes one */

SYSCALL_DEFINE2(multikernel_boot, int, cpu, unsigned long, kernel_start_address)
{
	int apicid, apicid_1;

	printk("multikernel boot: got to multikernel_boot syscall, cpu %d, apicid %d (%x), kernel start address 0x%lx\n",
			cpu, apic->cpu_present_to_apicid(cpu), BAD_APICID,kernel_start_address);

	apicid_1 = per_cpu(x86_bios_cpu_apicid, cpu);

	apicid = apic->cpu_present_to_apicid(cpu);
	if (apicid == BAD_APICID)
		printk(KERN_ERR"the cpu is not present in the current present_mask and it is ok to continue, apicid = %d, apicid_1 = %d\n", apicid, apicid_1);
	else {
		printk(KERN_ERR"the cpu is currently running with this kernel instance first put it to offline and then continue, apicid = %d, apicid_1 = %d\n", apicid, apicid_1);
		return -1;
	}
	apicid = per_cpu(x86_bios_cpu_apicid, cpu);  
	return mkbsp_boot_cpu(apicid, cpu, kernel_start_address);
}

SYSCALL_DEFINE0(get_boot_params_addr)
{
	printk("MKLINUX: syscall to return phys addr of boot_params structure\n");
	return orig_boot_params;
}
