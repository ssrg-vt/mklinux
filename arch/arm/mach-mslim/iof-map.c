/*
 *  linux/arch/arm/mach-mslim/iof-map.c
 *
 *  Copyright (c) 2012, Applied Micro Circuits Corporation
 *  Author: Vinayak Kale <vkale@apm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/smp.h>
#include <linux/jiffies.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/of_address.h>

#include <mach/motherboard.h>

#define MSLIM_PCI_MAP
#define MSLIM_AMAP_PAGE0_IOF_BAR_LOW_OFFSET		(0x120)
#define MSLIM_AMAP_PAGE0_IOF_BAR_HIGH_OFFSET		(0x124)
#define	MSLIM_PAGE0_MASK				(0xfffffff)

static void __iomem *iof_csr_base;
static unsigned long page0_bar_low;
static unsigned long page0_bar_high;

const static struct of_device_id iof_csr_of_match[] __initconst = {
	{ .compatible = "apm,mslim-iof-csr",	},
        { },
};

unsigned long long mslim_pa_to_iof_axi(unsigned long pa)
{
	unsigned long long iof_axi_addr = 0;

	if (!iof_csr_base) 
		return 0;

	if (pa >= 0x50000000 && pa <= 0x5fffffff) {
		iof_axi_addr = (((unsigned long long) (page0_bar_high & 0x3ff) << 32) | 
				(unsigned long long) ((page0_bar_low & ~(MSLIM_PAGE0_MASK)) | (pa & MSLIM_PAGE0_MASK)));
        } else if (pa >= 0xA0000000 && pa <= 0xAfffffff) {
		iof_axi_addr = (((unsigned long long) (0xe0 & 0x3ff) << 32) |
				(unsigned long long) ((0 & ~(MSLIM_PAGE0_MASK)) | (pa & MSLIM_PAGE0_MASK)));
        } else if (pa >= 0xB0000000 && pa <= 0xBfffffff) {
		iof_axi_addr = (((unsigned long long) (0x0 & 0x3ff) << 32) |
				(unsigned long long) ((0x70000000 & ~(MSLIM_PAGE0_MASK)) | (pa & MSLIM_PAGE0_MASK)));
        }
	return iof_axi_addr;	
}

unsigned long mslim_iof_axi_to_pa(unsigned long long iof_axi_addr)
{
	unsigned long pa;

	if (!iof_csr_base) 
		return 0;

	pa = ((MSLIM_DRAMA_BASE & ~(MSLIM_PAGE0_MASK)) | (iof_axi_addr & MSLIM_PAGE0_MASK));

	return pa;
}

void __init mslim_iof_map_init(void)
{
	struct device_node *np;
	int err;

	np = of_find_matching_node(NULL, iof_csr_of_match);
	if (!np) {
		err = -ENODEV;
		goto out;
	}

	iof_csr_base = of_iomap(np, 0);
	if (!iof_csr_base) {
		err = -ENOMEM;
		goto out;
	}

	page0_bar_low = readl(iof_csr_base + 
				MSLIM_AMAP_PAGE0_IOF_BAR_LOW_OFFSET);
	page0_bar_high = readl(iof_csr_base + 
				MSLIM_AMAP_PAGE0_IOF_BAR_HIGH_OFFSET);

	return;
out:
	WARN(err, "MSLIM mslim_iof_map_init failed (%d)\n", err);
}
