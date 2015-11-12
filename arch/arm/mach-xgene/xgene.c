/*
 * linux/arch/arm/mach-xgene/xgene.c
 *
 * Copyright (c) 2012, Applied Micro Circuits Corporation
 * Author: Tanmay Inamdar <tinamdar@apm.com>
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
#include <asm/hardware/gic.h>
#include <mach/motherboard.h>

extern void __init xgene_dt_map_io(void);
#ifdef CONFIG_SMP
extern struct smp_operations xgene_smp_ops;
#endif
extern void __init local_timer_of_register(void);

void __init xgene_dt_init_early(void)
{
#if 0
	/* Enable clock register early */
	xgene_clk_init();

	/* Enable error report early */
	xgene_pcp_init();
#endif
}

static  struct of_device_id xgene_irq_match[] __initdata = {
	{ .compatible = "arm,cortex-a5-gic", .data = gic_of_init, },
	{ .compatible = "arm,cortex-a9-gic", .data = gic_of_init, },
	{ .compatible = "arm,cortex-a15-gic", .data = gic_of_init, },
	{}
};

static void __init xgene_dt_init_irq(void)
{
	of_irq_init(xgene_irq_match);
}

static void __init xgene_dt_timer_init(void)
{
	int err;
	err = arch_timer_of_register();
	if (err)
		pr_err("%s: arch_timer_register failed %d\n", __func__, err);
}

static struct sys_timer xgene_dt_timer = {
	.init = xgene_dt_timer_init,
};

static struct of_dev_auxdata xgene_dt_auxdata_lookup[] __initdata = {
	{}
};

static void __init xgene_dt_init(void)
{
	of_platform_populate(NULL, of_default_bus_match_table,
			xgene_dt_auxdata_lookup, NULL);
}

const static char *xgene_dt_match[] __initconst = {
	"apm,xgene",
	NULL,
};

DT_MACHINE_START(XGENE_DT, "xgene")
	.dt_compat	= xgene_dt_match,
#ifdef CONFIG_SMP
	.smp		= smp_ops(xgene_smp_ops),
#endif
	.map_io		= xgene_dt_map_io,
	.init_early	= xgene_dt_init_early,
	.init_irq	= xgene_dt_init_irq,
	.handle_irq	= gic_handle_irq,
	.timer		= &xgene_dt_timer,
	.init_machine	= xgene_dt_init,
MACHINE_END
