/*
 * linux/arch/arm/mach-mslim/include/mach/motherboard.h
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
#ifndef __MACH_MSLIM_MOTHERBOARD_H
#define __MACH_MSLIM_MOTHERBOARD_H

//#define CONFIG_MSLIM_UART0

#define	MSLIM_A5_COP_CSR_BASE	0x44000000
#define MSLIM_A5_PERI_CSR_BASE  0x45000000
#define MSLIM_A5_L2CC_CSR_BASE  0x45002000
#define MSLIM_UART0_CSR_BASE    0xCC020000
#define MSLIM_UART1_CSR_BASE    0xCC021000
#define MSLIM_DRAMA_BASE        0x50000000

#define MSLIM_CFG_COP_SCRATCH0	0x44000F00

#ifdef CONFIG_MSLIM_UART0
#define MSLIM_UART_CSR_BASE	MSLIM_UART0_CSR_BASE
#else
#define MSLIM_UART_CSR_BASE	MSLIM_UART1_CSR_BASE
#endif

#endif
