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
 * @file apm_avl_api.c
 *
 * This file implements Classifier AVL Search Engine APIs.
 *
 */

#include <misc/xgene/cle/apm_preclass_data.h>
#include <misc/xgene/cle/apm_avl_base.h>
#include <misc/xgene/cle/apm_avl_api.h>

#ifdef CLE_SHADOW
static int avl_shadow_done[MAX_CLE_ENGINE] = {0};
static struct avl_node sys_avl_node[MAX_CLE_ENGINE][AVL_SSIZE8B_DPTH];
static enum apm_cle_avl_search_str_type sys_avl_key_size[MAX_CLE_ENGINE] = {0};

static inline void apm_avl_shadow_init(u32 cid)
{
	if (!avl_shadow_done[cid]) {
		sys_avl_key_size[cid] = SEARCH_STRING_RSVD;
		memset(&sys_avl_node[cid], 0,
			sizeof(sys_avl_node[cid]));
		avl_shadow_done[cid] = 1;
	}
}

struct avl_node *get_shadow_avl_nodes(u32 port)
{
	return &sys_avl_node[PID2CID[port]][0];
}

enum apm_cle_avl_search_str_type get_shadow_avl_key_size(u32 port)
{
	return sys_avl_key_size[PID2CID[port]];
}
#endif

int apm_cle_avl_add_node(u32 port, struct avl_node *node)
{
	int rc;
	u32 index;
	u32 node_value[REGS_PER_AVL_ADD];
	int i;
	for (i = 0; i < AVL_MAX_SEARCH_STRING; i++)
		node_value[i] = node->search_key[i];

	node_value[8] = node->result_pointer & AVL_DB_PTR_MASK;
	node_value[9] = node->priority & AVL_PRI_MASK;

	if ((rc = apm_cle_avl_add(port, node_value, &index) != APM_RC_OK)) {
		PCLS_ERR("Error %d In AVL Add node operation \n", rc);
	} else {
#ifdef CLE_SHADOW
		u32 cid = PID2CID[port];

		if (index >= 0 && index < AVL_SSIZE8B_DPTH) {
			apm_avl_shadow_init(cid);

			/* index field Will be used for avl_node validity by user */
			memcpy(&sys_avl_node[cid][index], node,
					sizeof(struct avl_node));
			sys_avl_node[cid][index].index = 1;
		}
#endif
		/* Returning back the avl node index */
		node->index = index;
	}

	return rc;
}

int apm_cle_avl_del_node(u32 port, struct avl_node *node)
{
	u32 index;
	int rc = apm_cle_avl_del(port, node->search_key, &index);

	if (rc != APM_RC_OK) {
		PCLS_ERR("Error %d In AVL Del node operation \n", rc);
	} else {
#ifdef CLE_SHADOW
		u32 cid = PID2CID[port];

		if (index >= 0 && index < AVL_SSIZE8B_DPTH) {
			apm_avl_shadow_init(cid);

			/* index field Will be used for avl_node validity by user */
			memset(&sys_avl_node[cid][index], 0,
					sizeof(struct avl_node));
		}
#endif
		/* Returning back the avl node index */
		node->index = index;
	}

	return rc;
}

int apm_cle_avl_search_node(u32 port, struct avl_node *node)
{
	int rc = apm_cle_avl_search(port, node->search_key, &node->index,
			&node->result_pointer);

	if (rc != APM_RC_OK && rc != APM_RC_CLE_MISS)
		PCLS_ERR("Error %d In AVL Search node operation \n", rc);

	return rc;
}

int apm_cle_avl_write_node(u32 port, struct avl_node *node)
{
	int rc;
	u32 node_value[REGS_PER_AVL_NODE];
	int i;
	u32 search_key[AVL_MAX_SEARCH_STRING];

	for (i = 0; i < AVL_MAX_SEARCH_STRING; i++)
		search_key[i] = node->search_key[i];

	node_value[0] = (node->balance & AVL_BAL_MASK) |
			((node->right_address & AVL_RADR_MASK) << 3) |
			((node->left_address & AVL_LADR_MASK) << 14) |
			((node->result_pointer & 0x7F) << 25);
							/* result_pointer_7b */
	node_value[1] = ((node->result_pointer >> 7) & 0x7) |
							/* result_pointer_3b */
			((node->priority & AVL_PRI_MASK) << 3) |
			((search_key[0] & 0x3FFFFFF) << 6);
							/* key_0_26b */
	for (i = 2; i < 9; i++) {
		node_value[i] = ((search_key[i - 2] >> 26) & 0x3F) |
				((search_key[i - 1] & 0x3FFFFFF) << 6);
	}
	node_value[9] = ((search_key[7] >> 26) & 0x3F);
	if ((rc = apm_cle_avl_wr(port, node_value, node->index) != APM_RC_OK)) {
		PCLS_ERR("Error %d In AVL Write node operation \n", rc);
	}

#ifdef CLE_SHADOW
	apm_avl_shadow_init(PID2CID[port]);
	if (node->index >= 0 && node->index < AVL_SSIZE8B_DPTH)
		memcpy(&sys_avl_node[PID2CID[port]][node->index], node,
			sizeof(struct avl_node));
#endif

	return rc;
}

int apm_cle_avl_read_node(u32 port, struct avl_node *node)
{
	int rc;
	u32 node_value[REGS_PER_AVL_NODE];
	int i;
	u32 search_key[AVL_MAX_SEARCH_STRING];

	if ((rc = apm_cle_avl_rd(port, node_value, node->index) != APM_RC_OK)) {
		PCLS_ERR("Error %d In AVL Read node operation \n", rc);
		return rc;
	}
	node->balance = (node_value[0] & AVL_BAL_MASK);
	node->right_address = ((node_value[0] >> 3) & AVL_RADR_MASK);
	node->left_address = ((node_value[0] >> 14) & AVL_LADR_MASK);
	node->result_pointer = ((node_value[0] >> 25) & 0x7F) |
				((node_value[1] << 7)  & 0x7);
	node->priority = ((node_value[1] >> 3) & AVL_PRI_MASK);
	for (i = 0; i < 8; i++) {
		search_key[i] = ((node_value[i + 1] >> 6) & 0x3FFFFFF) |
				((node_value[i + 2] << 26) & 0x3F);
	}

	for (i = 0; i < AVL_MAX_SEARCH_STRING; i++)
		node->search_key[i] = search_key[i];

#ifdef CLE_SHADOW
	apm_avl_shadow_init(PID2CID[port]);
	if (node->index >= 0 && node->index < AVL_SSIZE8B_DPTH)
		memcpy(&sys_avl_node[PID2CID[port]][node->index], node,
			sizeof(struct avl_node));
#endif

	PCLS_DBG("balance[%d], right_addres[0X%08X],"
		" left_addres[0X%08X], result_pointer[%d] \n",
		node->balance, node->right_address,
		node->left_address, node->result_pointer);
	return rc;
}

int apm_cle_avl_get_status(u32 port, struct apm_cle_avl_status *avl_status)
{
	int rc;
	u32 status;
	if((rc = apm_gbl_cle_rd32(PID2CID[port], AVL_STATUS_ADDR, &status)
		!= APM_RC_OK)) {
		PCLS_ERR("Error %d In AVL get status operation \n", rc);
	} else {
		avl_status->root_addr = ROOT_ADDR_RD(status);
		avl_status->node_cnt  = NODE_CNT_RD(status);
		PCLS_DBG("root_addr[0X%08X], node_cnt [%d] \n",
			avl_status->root_addr, avl_status->node_cnt);
	}
	return rc;
}

int apm_cle_avl_init(u32 port, enum apm_cle_avl_search_str_type key_size)
{
	int rc = APM_RC_OK;
	u32 int_mask;
	u32 avl_config = 0;
	u32 srchdb_depth;
	u32 cid = PID2CID[port];

	switch (key_size) {
	case SEARCH_STRING_SIZE8B:
		srchdb_depth = AVL_SSIZE8B_DPTH;
		break;
	case SEARCH_STRING_SIZE16B:
		srchdb_depth = AVL_SSIZE16B_DPTH;
		break;
	case SEARCH_STRING_SIZE32B:
		srchdb_depth = AVL_SSIZE32B_DPTH;
		break;
	default:
		PCLS_ERR("Error: Invalid search key_size \n");
		return APM_RC_INVALID_PARM;
		break;
	}

	avl_config |= MAXSTEPS_THRESH_SET(avl_config, AVL_MAX_STEPS_THRS);
	avl_config |= SRCHDB_DEPTH_SET(avl_config, srchdb_depth);
	avl_config |= STRING_SIZE_SET(avl_config, key_size);

	PCLS_DBG("key_size[%d], srchdb_depth[%d], avl_config[0X%08X] \n",
		key_size, srchdb_depth, avl_config);
	rc = apm_gbl_cle_wr32(cid, AVL_CONFIG_ADDR, avl_config);

	if (rc != APM_RC_OK) {
		PCLS_ERR("Error %d In CLE AVL Init Operation \n", rc);
		return rc;
	}

	int_mask = FATAL_WRSELFPOINTERRMASK_WR(0) |
		FATAL_RDSELFPOINTERRMASK_WR(0)	  |
		FATAL_WRBALERRMASK_WR(0)	  |
		FATAL_RDBALERRMASK_WR(0)	  |
		FATAL_MAXSTEPSERRMASK_WR(0)	  |
		FATAL_RDERR_RAMMASK_WR(0)	  |
		RDERR_RAMMASK_WR(0)		  |
		DELERR_RIDNOTFOUNDMASK_WR(0)	  |
		DELERR_EMPTYMASK_WR(0)		  |
		ADDERR_RIDDUPLICATEMASK_WR(0)	  |
		ADDERR_FULLMASK_WR(0);
	rc = apm_gbl_cle_wr32(cid, AVL_SEARCH_INTMASK_ADDR, int_mask);

#ifdef CLE_SHADOW
	if (rc == APM_RC_OK) {
		apm_avl_shadow_init(cid);
		sys_avl_key_size[cid] = key_size;
	}
#endif

	return rc;
}
