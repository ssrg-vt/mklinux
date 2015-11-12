/*
 * linux/arch/arm/mach-mslim/mslim.c
 *
 * Copyright (c) 2012, Applied Micro Circuits Corporation
 * Author: Vinayak Kale <vkale@apm.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#include <linux/device.h>
#include <linux/amba/bus.h>
#include <linux/amba/mmci.h>
#include <linux/io.h>
#include <linux/smp.h>
#include <linux/init.h>
#include <linux/irqchip.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>

#include <asm/arch_timer.h>
#include <asm/mach-types.h>
#include <asm/sizes.h>
#include <asm/smp_twd.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <asm/hardware/arm_timer.h>
#include <asm/hardware/cache-l2x0.h>
#include <mach/motherboard.h>

extern void __init mslim_dt_map_io(void);
extern struct smp_operations mslim_smp_ops;
extern void __init local_timer_of_register(void);
extern void __init mslim_iof_map_init(void);

void __init mslim_dt_init_early(void)
{
}

static void __init mslim_clk_init(void)
{
        struct device_node *node;
        struct clk *clk;
	u32 twd_clock = 250000000; /* default value */
	
	node = of_find_compatible_node(NULL, NULL, "arm,cortex-a5-twd-timer");
	if (node) {
		u32 rate;
		if (!of_property_read_u32(node, "clock-frequency", &rate))
			twd_clock = rate;
		of_node_put(node);
	}

        clk = clk_register_fixed_rate(NULL, "twd_clk", NULL,
                        CLK_IS_ROOT, twd_clock);
        WARN_ON(clk_register_clkdev(clk, NULL, "smp_twd"));
}

static void __init mslim_dt_timer_init(void)
{
	mslim_clk_init();

#ifdef CONFIG_COMMON_CLK
	of_clk_init(NULL);
#endif

#ifdef CONFIG_SMP
	clocksource_of_init();
#else
	local_timer_of_register();
#endif
}

static struct of_dev_auxdata mslim_dt_auxdata_lookup[] __initdata = {
	{}
};

static void __init mslim_dt_init(void)
{
	mslim_iof_map_init();
#ifdef CONFIG_CACHE_L2X0
	l2x0_of_init(0x00400000, 0xfe0fffff);
#endif
	of_platform_populate(NULL, of_default_bus_match_table,
			mslim_dt_auxdata_lookup, NULL);
}

const static char *mslim_dt_match[] __initconst = {
	"apm,mslim",
	NULL,
};

DT_MACHINE_START(MSLIM_DT, "APM-MSLIM")
	.dt_compat	= mslim_dt_match,
	.smp		= smp_ops(mslim_smp_ops),
	.map_io		= mslim_dt_map_io,
	.init_early	= mslim_dt_init_early,
	.init_irq	= irqchip_init,
	.init_time	= mslim_dt_timer_init,
	.init_machine	= mslim_dt_init,
MACHINE_END
