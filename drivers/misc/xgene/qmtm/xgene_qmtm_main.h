/**
 * AppliedMicro X-Gene SOC Queue Manager Traffic Manager Linux Header file
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * Author: Ravi Patel <rapatel@apm.com>
 *         Keyur Chudgar <kchudgar@apm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * @file xgene_qmtm_main.h
 *
 */

#ifndef __XGENE_QMTM_MAIN_H__
#define __XGENE_QMTM_MAIN_H__

#include <linux/kernel.h>
#include <asm/io.h>

/* Enabled debug macros put here */
#define QMTM_PRINT_ENABLE
#define QMTM_ERROR_PRINT_ENABLE
#undef  QMTM_CORE_PRINT_ENABLE
#undef  QMTM_QSTATE_PRINT_ENABLE
#undef  QMTM_WRITE_PRINT_ENABLE
#undef  QMTM_READ_PRINT_ENABLE
#undef  QMTM_DEBUG_PRINT_ENABLE

#ifdef QMTM_PRINT_ENABLE
#define QMTM_PRINT(x, ...) printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define QMTM_PRINT(x, ...)
#endif

#ifdef QMTM_ERROR_PRINT_ENABLE
#define QMTM_ERROR_PRINT(x, ...) printk(KERN_ERR x, ##__VA_ARGS__)
#else
#define QMTM_ERROR_PRINT(x, ...)
#endif

#ifdef QMTM_CORE_PRINT_ENABLE
#define QMTM_CORE_PRINT(x, ...) printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define QMTM_CORE_PRINT(x, ...)
#endif

#ifdef QMTM_QSTATE_PRINT_ENABLE
#define QMTM_QSTATE_PRINT(x, ...) printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define QMTM_QSTATE_PRINT(x, ...)
#endif

#ifdef QMTM_WRITE_PRINT_ENABLE
#define QMTM_WRITE_PRINT(x, ...) printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define QMTM_WRITE_PRINT(x, ...)
#endif

#ifdef QMTM_READ_PRINT_ENABLE
#define QMTM_READ_PRINT(x, ...) printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define QMTM_READ_PRINT(x, ...)
#endif

#ifdef QMTM_DEBUG_PRINT_ENABLE
#define QMTM_DEBUG_PRINT(x, ...) printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define QMTM_DEBUG_PRINT(x, ...)
#endif

#ifdef CONFIG_ARCH_MSLIM
#define CONFIG_NOT_COHERENT_QMTM
#define VIRT_TO_PHYS(x)	mslim_pa_to_iof_axi(virt_to_phys(x))
#define PHYS_TO_VIRT(x)	phys_to_virt(mslim_iof_axi_to_pa(x))
#else
#define VIRT_TO_PHYS(x) virt_to_phys(x)
#define PHYS_TO_VIRT(x) phys_to_virt(x)
#endif

/* defined to enable QM Recombination Logic */
#define QMTM_RECOMBINATION_BUFFER

int xgene_qmtm_wr32(u32 qm_ip, u32 offset, u32 data);

int xgene_qmtm_rd32(u32 qm_ip, u32 offset, u32 *data);

/* QMTM raw register read/write routine */
static inline void xgene_qmtm_fab_wr32(void *addr, u32 data)
{
	writel(data, addr);
}

static inline u32 xgene_qmtm_fab_rd32(void *addr)
{
	return readl(addr);
}

void *MEMALLOC(u32 size, u32 align);

void MEMFREE(void *ptr);

#ifdef CONFIG_XGENE_QMTM_ERROR
int xgene_qmtm_enable_error(u8 qmtm_ip, u16 irq, char *name);

void xgene_qmtm_disable_error(u8 qmtm_ip);
#endif

#endif /* __XGENE_QMTM_MAIN_H__ */
