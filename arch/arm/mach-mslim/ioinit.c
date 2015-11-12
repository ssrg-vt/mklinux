/*
 * linux/arch/arm/mach-mslim/ioinit.c
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
#include <linux/io.h>
#include <linux/init.h>
#include <asm/mach-types.h>
#include <asm/sizes.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <mach/motherboard.h>
#include <mach/iomap.h>

extern void __init mslim_dt_smp_map_io(void);

static struct map_desc mslim_io_desc[] __initdata = {
        {
                .virtual        = IO_PHYS_TO_VIRT(MSLIM_UART_CSR_BASE),
                .pfn            = __phys_to_pfn(MSLIM_UART_CSR_BASE),
                .length         = SZ_4K,
                .type           = MT_DEVICE,
        }, 
	{
                .virtual        = IO_PHYS_TO_VIRT(MSLIM_A5_PERI_CSR_BASE),
                .pfn            = __phys_to_pfn(MSLIM_A5_PERI_CSR_BASE),
                .length         = SZ_8K,
                .type           = MT_DEVICE,
        }, 
	{
                .virtual        = IO_PHYS_TO_VIRT(MSLIM_A5_L2CC_CSR_BASE),
                .pfn            = __phys_to_pfn(MSLIM_A5_L2CC_CSR_BASE),
                .length         = SZ_4K,
                .type           = MT_DEVICE,
        },
        {
                .virtual        = IO_PHYS_TO_VIRT(MSLIM_A5_COP_CSR_BASE),
                .pfn            = __phys_to_pfn(MSLIM_A5_COP_CSR_BASE),
                .length         = SZ_64K,
                .type           = MT_DEVICE,
        },
};

void __init mslim_dt_map_io(void)
{
	iotable_init(mslim_io_desc, ARRAY_SIZE(mslim_io_desc));
	
#if defined(CONFIG_SMP)
	mslim_dt_smp_map_io();
#endif
}
