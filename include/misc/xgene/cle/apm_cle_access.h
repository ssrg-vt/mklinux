/**
 * AppliedMicro APM88xxxx SoC Classifier Driver
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * All rights reserved. Mahesh Pujara <mpujara@apm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * @file apm_cle_access.h
 *
 * This file defines access layer for APM88xxxx SoC Classifier Driver
 *
 */

#ifndef __APM_CLE_ACCESS_H__
#define __APM_CLE_ACCESS_H__

#include <linux/kernel.h>
#include <linux/types.h>
#include <asm/string.h>

#if 1
#define CLE_MANAGER

#if 1
#define PTREE_MANAGER
#endif

#if 1
#define DBPTR_MANAGER
#endif

#if 1
#define AVL_MANAGER
#endif

#endif /* CLE_MANAGER */

#if 1
#define CLE_SHADOW
#endif

#if 0
#define CLE_DEBUG
#endif

#if 1
#define PRECLASS_FATAL
#endif 

#if 1
#define PRECLASS_ERR
#endif

#if 0
#define PRECLASS_DEBUG
#endif

#if 0
#define PRECLASS_PRINT	
#endif

#if 0
#define CLE_NODE_DEBUG
#endif

#ifdef CLE_NODE_DEBUG
#define CLE_NODE_DBG(x, ...)		printk(x, ##__VA_ARGS__)
#define CLE_PTREE_DUMP(n)		apm_ptree_dump(__func__, __LINE__, n)
#else
#define CLE_NODE_DBG(x, ...)
#define CLE_PTREE_DUMP(n)
#endif

#ifdef PRECLASS_DEBUG
#define PCLS_DBG(x, ...)		printk(x, ##__VA_ARGS__)
#else
#define PCLS_DBG(x, ...)
#endif

#ifdef PRECLASS_PRINT
#define PCLS_PRNT(x, ...)		printk(x, ##__VA_ARGS__)
#else
#define PCLS_PRNT(x, ...)
#endif

#ifdef PRECLASS_FATAL
#define PCLS_FATAL(x, ...)		printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define PCLS_FATAL(x, ...)
#endif

#ifdef PRECLASS_ERR
#define PCLS_ERR(x, ...)		printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define PCLS_ERR(x, ...)
#endif

#define CPU_TO_LE32 cpu_to_le32
#define LE32_TO_CPU le32_to_cpu

/**
 * @brief   Global Low Level API to read Pre Classifier/Parser CSRs.
 * @param   reg_offset - Offset of the CSR to read.
 * @param   *value - return value of the register read
 * @return  APM_RC_OK or APM_RC_ERROR
 *
 *
 */
int apm_gbl_cle_rd32(u32 cid, u32 reg_offset, u32 *value);

/**
 * @brief   Global Low Level API to Write Pre Classifier/Parser CSRs.
 * @param   reg_offset - Offset of the CSR to write.
 * @param   value - value to be written into CSR.
 * @return  APM_RC_OK or APM_RC_ERROR
 *
 *
 */
int apm_gbl_cle_wr32(u32 cid, u32 reg_offset, u32  value);

#endif	/* __APM_CLE_ACCESS_H__	 */
