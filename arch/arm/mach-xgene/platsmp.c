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
#include <linux/jiffies.h>
#include <asm/barrier.h>
#include <asm/cacheflush.h>
#include <asm/smp_plat.h>
#include <asm/hardware/gic.h>
#include <asm/mach/map.h>
#include <mach/motherboard.h>

extern void xgene_secondary_startup(void);
extern void xgene_cpu_die(unsigned int cpu);

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

void __cpuinit xgene_secondary_init(unsigned int cpu)
{
        /*
         * if any interrupts are already enabled for the primary
         * core (e.g. timer irq), then they will not have been enabled
         * for us: do so
         */
        gic_secondary_init(0);

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

int __cpuinit xgene_boot_secondary(unsigned int cpu, struct task_struct *idle)
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
	 * Release the secondary code waiting on 'wfe'
	 */
        sev();

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
      
static int __init xgene_dt_cpus_num(unsigned long node, const char *uname,
		int depth, void *data)
{
	static int prev_depth = -1;
	static int nr_cpus = -1;

	if (prev_depth > depth && nr_cpus > 0)
		return nr_cpus;

	if (nr_cpus < 0 && strcmp(uname, "cpus") == 0)
		nr_cpus = 0;

	if (nr_cpus >= 0) {
		const char *device_type = of_get_flat_dt_prop(node,
				"device_type", NULL);

		if (device_type && strcmp(device_type, "cpu") == 0)
			nr_cpus++;
	}

	prev_depth = depth;

	return 0;
}

/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
static void __init xgene_smp_init_cpus(void)
{
	int ncores = 0, i;
	ncores = of_scan_flat_dt(xgene_dt_cpus_num, NULL);

	if (ncores < 2)
		return;

	if (ncores > nr_cpu_ids) {
		pr_warn("SMP: %u cores greater than maximum (%u), clipping\n",
				ncores, nr_cpu_ids);
		ncores = nr_cpu_ids;
	}

	for (i = 0; i < ncores; ++i)
		set_cpu_possible(i, true);

	set_smp_cross_call(gic_raise_softirq);
}

static void __init xgene_smp_prepare_cpus(unsigned int max_cpus)
{
	int i = 0;

	for (i = 0; i < max_cpus; i++)
		set_cpu_present(i, true);
	/*
	 * Write the address of secondary startup
	 */
	/* TODO - use a scratchpad register */
        writel(virt_to_phys(xgene_secondary_startup),
                ioremap(XGENE_DRAMA_BASE,SZ_1K)+0x200);
}

struct smp_operations __initdata xgene_smp_ops = {
	.smp_init_cpus          = xgene_smp_init_cpus,
	.smp_prepare_cpus       = xgene_smp_prepare_cpus,
	.smp_secondary_init     = xgene_secondary_init,
	.smp_boot_secondary     = xgene_boot_secondary,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_die                = xgene_cpu_die,
#endif
};

