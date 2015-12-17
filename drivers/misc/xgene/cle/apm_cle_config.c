/*
 * AppliedMicro APM88xxxx CLE Engine Configuration Driver
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * Ravi Patel <rapatel@apm.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * @file apm_cle_config.c
 *
 * This file implements Configuration APIs for
 * AppliedMicro APM88xxxx SoC Classifier module.
 */

#include <linux/err.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <misc/xgene/cle/apm_cle_config.h>
#include <misc/xgene/cle/apm_preclass_data.h>

struct apm_ptree_config_list {
	struct list_head node;
	char ptree_id[CLE_PTREE_ID_SIZE];
	struct apm_ptree_config ptree_config;
};

static struct list_head apm_ptree_config_head[MAX_CLE_PORTS] = {{0}};

int apm_cle_init(u32 port_id)
{
	u32 rc = APM_RC_OK;
	static int init_list_head_done = 0;
#if 0
	u32 cid = PID2CID[port_id];
	u8 *cstr = NULL;
	u32 cle_disable_mem_shutdown_addr;
	struct clk *clk;

	if (port_id < CLE_ENET_4)
		goto init_list_head;

	switch (port_id) {
	case CLE_ENET_4:
		cstr = "cle_lite";
		break;
	case CLE_LA:
		cstr = "cle_lac";
		break;
	}

	clk = clk_get(NULL, cstr);
	if (IS_ERR(clk)) {
		printk("Error getting clock info for port %d\n", port_id);
		return -ENODEV;
	}
	clk_enable(clk);

	cle_disable_mem_shutdown_addr =
		apm_class_base_addr[cid] +
		APM_GLBL_DIAG_OFFSET +
		CLE_CFG_MEM_RAM_SHUTDOWN_ADDR;

	/* Remove CLE CSR memory shutdown */
	if ((rc = apm86xxx_disable_mem_shutdown(
			(u32 __iomem *)cle_disable_mem_shutdown_addr,
			CLE_MASK)) != 0) {
		printk(KERN_ERR
			"Failed to remove Classifier CSR "
			"memory shutdown\n");
		return rc;
	}

init_list_head:
#endif
	if (!init_list_head_done) {
		int i;

		for (i = 0; i < MAX_CLE_PORTS; i++)
			INIT_LIST_HEAD(&apm_ptree_config_head[i]);
		init_list_head_done = 1;
	}

	return rc;
}

int apm_preclass_switch_tree(u8 port_id, char *ptree_id, u8 wol_mode)
{
	int rc;
	struct apm_ptree_config *ptree_config;

	PCLS_DBG("Switch Tree for port %d with ptree %s and wol_mode %d\n",
		port_id, ptree_id, wol_mode);

	ptree_config = apm_find_ptree_config(port_id, ptree_id);
	if (ptree_config == NULL) {
		PCLS_ERR("Patricia Tree %s Configuration absent for "
			"port %d\n", ptree_id, port_id);
		return APM_RC_ERROR;
	}

	if ((rc = apm_set_sys_ptree_config(port_id, ptree_config)
			!= APM_RC_OK)) {
		PCLS_ERR("Preclass Switch to %s Tree error %d \n",
			ptree_id, rc);
		return rc;
	}
#ifdef PTREE_MANAGER
	/* Configure Wol Mode */
	if (port_id == 0)
		rc = apm_preclass_wol_mode(wol_mode);
#endif
	return rc;
}

static struct apm_ptree_config_list *apm_find_ptree_config_entry(u8 port,
		char *ptree_id)
{
	struct apm_ptree_config_list *entry;

	if (ptree_id == NULL || ptree_id[0] == '\0' ||
			apm_ptree_config_head[port].next == NULL ||
			apm_ptree_config_head[port].prev == NULL)
		return NULL;

	list_for_each_entry(entry, &apm_ptree_config_head[port], node) {
		if (strncmp(entry->ptree_id, ptree_id, CLE_PTREE_ID_SIZE) == 0) {
			return entry;
		}
	}

	return NULL;
}

struct apm_ptree_config *apm_find_ptree_config(u8 port, char *ptree_id)
{
	struct apm_ptree_config_list *entry;

	entry = apm_find_ptree_config_entry(port, ptree_id);

	if (entry)
		return &entry->ptree_config;

	return NULL;
}

struct apm_ptree_config *apm_add_ptree_config(u8 port, char *ptree_id)
{
	struct apm_ptree_config_list *entry;

	if (ptree_id == NULL || ptree_id[0] == '\0')
		return NULL;

	entry = apm_find_ptree_config_entry(port, ptree_id);

	if (entry == NULL) {
		entry = kmalloc(sizeof (struct apm_ptree_config_list),
			GFP_KERNEL);

		if (entry == NULL)
			return NULL;

		strncpy(entry->ptree_id, ptree_id, CLE_PTREE_ID_SIZE);
		list_add(&entry->node, &apm_ptree_config_head[port]);
		entry->ptree_config.start_node_ptr = SYSTEM_START_NODE;
		entry->ptree_config.start_pkt_ptr = 0;
		entry->ptree_config.default_result = DEFAULT_DBPTR;
		entry->ptree_config.start_parser = 1;
		entry->ptree_config.max_hop = 0;
	}

	return &entry->ptree_config;
}

int apm_del_ptree_config(u8 port, char *ptree_id)
{
	struct apm_ptree_config_list *entry;

	entry = apm_find_ptree_config_entry(port, ptree_id);

	if (entry) {
		list_del(&entry->node);
		kfree(entry);
		return 0;
	}

	return -EINVAL;
}

#ifdef PTREE_MANAGER
char *apm_get_sys_ptree_id(u8 port)
{
	struct apm_ptree_config_list *entry;
	struct apm_ptree_config ptree_config;

	if (apm_ptree_config_head[port].next == NULL ||
			apm_ptree_config_head[port].prev == NULL)
		return NULL;

	if (apm_get_sys_ptree_config(port, &ptree_config))
		return NULL;

	list_for_each_entry(entry, &apm_ptree_config_head[port], node) {
		if (entry->ptree_config.start_node_ptr == ptree_config.start_node_ptr) {
			return entry->ptree_id;
		}
	}

	return NULL;
}
#endif
