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

int mklinux_boot __initdata;
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
	printk("multikernel boot: got to multikernel_boot syscall, cpu %d, kernel start address 0x%lu\n",
			cpu, kernel_start_address);
	int apicid = apic->cpu_present_to_apicid(cpu);
        return mkbsp_boot_cpu(apicid, cpu);
}

