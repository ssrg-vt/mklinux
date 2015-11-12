/* 
 *  linux/arch/arm/mach-mslim/include/mach/debug-macro.S
 *
 *  Copyright (c) 2012, Applied Micro Circuits Corporation
 *  Author: Vinayak Kale <vkale@apm.com>
 * 
 *  Based on arch/arm/mach-realview/include/mach/debug-macro.S
 *
 *  Debugging macro include header
 *
 *  Copyright (C) 1994-1999 Russell King
 *  Moved from linux/arch/arm/kernel/debug.S by Ben Dooks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "motherboard.h"

#ifdef CONFIG_MSLIM_UART0
#define DEBUG_UART_PHYS_ADDR              0xcc020000
#define DEBUG_UART_VIRT_ADDR              0xf8020000
#else
#define DEBUG_UART_PHYS_ADDR              0xcc021000
#define DEBUG_UART_VIRT_ADDR              0xf8021000
#endif

#define UART_LSR 			0x14
#define UART_TX 			0x0

                .macro  addruart,rp,rv,tmp
                ldr     \rv, =DEBUG_UART_VIRT_ADDR 
                ldr     \rp, =DEBUG_UART_PHYS_ADDR 
                .endm

                .macro  senduart,rd,rx
                strb    \rd, [\rx, #UART_TX]
                .endm

                .macro  waituart,rd,rx
1001:           ldrb    \rd, [\rx, #UART_LSR]
                tst     \rd, #0x20              /* bit 5 */
                beq     1001b
                .endm

                .macro  busyuart,rd,rx
1001:           ldrb    \rd, [\rx, #UART_LSR]
                tst     \rd, #0x20              /* bit 5 */
                beq     1001b
                .endm
