/*
 *  linux/arch/arm/mach-mslim/platsmp.c
 *
 *  Copyright (c) 2012, Applied Micro Circuits Corporation
 *  Author: Vinayak Kale <vkale@apm.com>
 *  
 *  Based on linux/arch/arm/mach-vexpress/platsmp.c
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/of_fdt.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <asm/cacheflush.h>
#include <asm/smp_plat.h>
#include <asm/smp_scu.h>
#include <asm/mach/map.h>
#include <mach/motherboard.h>

extern void mslim_secondary_startup(void);
extern void mslim_cpu_die(unsigned int cpu);

static void *mslim_scu_base __initdata;

const static char *mslim_dt_scu_match[] __initconst = {
	"arm,cortex-a5-scu",
	NULL
};

/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */
static void __cpuinit write_pen_release(int val)
{
        pen_release = val;
        smp_wmb();
        __cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
        outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
}

static DEFINE_SPINLOCK(boot_lock);

void __cpuinit mslim_secondary_init(unsigned int cpu)
{
        /*
         * let the primary processor know we're out of the
         * pen, then head off into the C entry point
         */
        write_pen_release(-1);

        /*
         * Synchronise with the boot thread.
         */
        spin_lock(&boot_lock);
        spin_unlock(&boot_lock);
}

int __cpuinit mslim_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
        unsigned long timeout;

        /*
         * Set synchronisation state between this boot processor
         * and the secondary one
         */
        spin_lock(&boot_lock);

        /*
         * This is really belt and braces; we hold unintended secondary
         * CPUs in the holding pen until we're ready for them.  However,
         * since we haven't sent them a soft interrupt, they shouldn't
         * be there.
         */
        write_pen_release(cpu_logical_map(cpu));

        /*
         * Send the secondary CPU a soft interrupt, thereby causing
         * the boot monitor to read the system wide flags register,
         * and branch to the address found there.
         */
//        gic_raise_softirq(cpumask_of(cpu), 0);
        arch_send_wakeup_ipi_mask(cpumask_of(cpu));
        asm("sev" : : : "memory");

        timeout = jiffies + (1 * HZ);
        while (time_before(jiffies, timeout)) {
                smp_rmb();
                if (pen_release == -1)
                        break;

                udelay(10);
        }

        /*
         * now the secondary core is starting up let it run its
         * calibrations, then wait for it to finish
         */
        spin_unlock(&boot_lock);

        return pen_release != -1 ? -ENOSYS : 0;
}
      

static int __init mslim_dt_find_scu(unsigned long node,
		const char *uname, int depth, void *data)
{
	if (of_flat_dt_match(node, mslim_dt_scu_match)) {
		phys_addr_t phys_addr;
		__be32 *reg = of_get_flat_dt_prop(node, "reg", NULL);

		if (WARN_ON(!reg))
			return -EINVAL;

		phys_addr = be32_to_cpup(reg);

		mslim_scu_base = ioremap(phys_addr, SZ_256);
		if (WARN_ON(!mslim_scu_base))
			return -EFAULT;
	}

	return 0;
}

void __init mslim_dt_smp_map_io(void)
{
	if (initial_boot_params)
		WARN_ON(of_scan_flat_dt(mslim_dt_find_scu, NULL));
}

static void __init mslim_dt_smp_init_cpus(void)
{
	int ncores = 0, i;

	ncores = scu_get_core_count(mslim_scu_base);
	if (ncores < 2)
		return;

	if (ncores > nr_cpu_ids) {
		pr_warn("SMP: %u cores greater than maximum (%u), clipping\n",
				ncores, nr_cpu_ids);
		ncores = nr_cpu_ids;
	}

	for (i = 0; i < ncores; ++i)
		set_cpu_possible(i, true);
}

static void __init mslim_dt_smp_prepare_cpus(unsigned int max_cpus)
{
	scu_enable(mslim_scu_base);
}

/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
static void __init mslim_smp_init_cpus(void)
{
	mslim_dt_smp_init_cpus();
}

static void __init mslim_smp_prepare_cpus(unsigned int max_cpus)
{
	/*
	 * Initialise the present map, which describes the set of CPUs
	 * actually populated at the present time.
	 */
	mslim_dt_smp_prepare_cpus(max_cpus);

	/*
	 * Write the address of secondary startup
	 */
        writel(virt_to_phys(mslim_secondary_startup),
                ioremap(MSLIM_CFG_COP_SCRATCH0,SZ_256));
}

struct smp_operations __initdata mslim_smp_ops = {
        .smp_init_cpus          = mslim_smp_init_cpus,
        .smp_prepare_cpus       = mslim_smp_prepare_cpus,
        .smp_secondary_init     = mslim_secondary_init,
        .smp_boot_secondary     = mslim_boot_secondary,
#ifdef CONFIG_HOTPLUG_CPU
        .cpu_die                = mslim_cpu_die,
#endif
};

