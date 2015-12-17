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
 * @file apm_cle_access.c
 *
 * This file implements access layer for APM88xxxx SoC Classifier driver
 *
 */

#include <linux/module.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/etherdevice.h>
#include <linux/crc32.h>
#include <linux/ethtool.h>
#include <linux/crc16.h>
#include <linux/of_platform.h>
#include <linux/kthread.h>

#if defined(CONFIG_SMP)
#include <asm/smp.h>
#endif

#include <misc/xgene/cle/apm_preclass_base.h>
#include <misc/xgene/cle/apm_preclass_data.h>
#include <misc/xgene/cle/apm_cle_config.h>
#include <misc/xgene/cle/apm_cle_mgr.h>
#include <misc/xgene/cle/apm_cle_config.h>

#define APM_CLE_DRIVER_VERSION  "1.0"
#define APM_CLE_DRIVER_NAME     "apm_cle"
#define APM_CLE_DRIVER_STRING  	"APM Classifier Driver"

u64 apm_class_base_addr_p[MAX_CLE_ENGINE];
void *apm_class_base_addr[MAX_CLE_ENGINE];

int apm_gbl_cle_rd32(u32 cid, u32 reg_offset, u32  *value)
{
	void *addr;

#ifdef CLE_DBG	   
	if (value == NULL) {
		PCLS_DBG("Error: Null value pointer in calling %s \n",
			__FUNCTION__);
		return APM_RC_INVALID_PARM;
	}
#endif
	if (!apm_class_base_addr[cid])
		return -ENODEV;

	addr = apm_class_base_addr[cid] + reg_offset;
	*value = readl(addr);
	PCLS_DBG("CLE CSR Read at addr 0x%0llX value 0x%08X \n", (unsigned long long)addr, *value);
	return APM_RC_OK;
}

int apm_gbl_cle_wr32(u32 cid, u32 reg_offset, u32 value)
{
	void *addr;

	if (!apm_class_base_addr[cid])
		return -ENODEV;

	addr = apm_class_base_addr[cid] + reg_offset;

	PCLS_DBG("CLE CSR Write at addr 0x%0llX value 0x%08X \n", (unsigned long long)addr, value);
	writel(value, addr);
	return APM_RC_OK;
}

EXPORT_SYMBOL(apm_class_base_addr_p);
EXPORT_SYMBOL(apm_class_base_addr);
EXPORT_SYMBOL(PID2CID);
EXPORT_SYMBOL(apm_cle_systems);
EXPORT_SYMBOL(apm_cle_system_id);
EXPORT_SYMBOL(apm_ptree_alloc);

EXPORT_SYMBOL(apm_cle_init);
EXPORT_SYMBOL(apm_gbl_cle_rd32);
EXPORT_SYMBOL(apm_gbl_cle_wr32);
EXPORT_SYMBOL(apm_cle_mgr_init);
EXPORT_SYMBOL(apm_preclass_cldb_write);
EXPORT_SYMBOL(apm_preclass_ptram_write);
EXPORT_SYMBOL(apm_cle_avl_add_node);
EXPORT_SYMBOL(apm_preclassify_init);
EXPORT_SYMBOL(apm_set_cle_dbptr);
EXPORT_SYMBOL(apm_add_ptree_config);
EXPORT_SYMBOL(apm_del_ptree_config);
EXPORT_SYMBOL(apm_ptree_node_config);
EXPORT_SYMBOL(apm_preclass_init_state);
EXPORT_SYMBOL(sys_preclass_state);
EXPORT_SYMBOL(apm_get_sys_ptree_config);
EXPORT_SYMBOL(apm_preclass_wol_mode);
EXPORT_SYMBOL(apm_set_sys_ptree_config);
EXPORT_SYMBOL(apm_preclass_switch_tree);
EXPORT_SYMBOL(apm_set_ptree_node_branch);
EXPORT_SYMBOL(apm_find_ptree_config);

MODULE_VERSION(APM_CLE_DRIVER_VERSION);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("APM_CLE_DRIVER_STRING");
