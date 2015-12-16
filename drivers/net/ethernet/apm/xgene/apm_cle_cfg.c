/**
 * AppliedMicro APM88xxxx Ethernet CLE Configuration Driver
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * All rights reserved.  Ravi Patel <rapatel@apm.com>
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
 * @file apm_cle_cfg.c
 *
 * This file implements Classifier configurations in use by Ethernet driver.
 *
 */

#include "apm_cle_cfg.h"
#include <misc/xgene/cle/apm_cle_mgr.h>
#include <linux/proc_fs.h>

#if defined(SET_RX_MODE)
static struct ptree_kn kn[MAX_RX_QUEUES];
static struct apm_cle_dbptr dbptr[MAX_RX_QUEUES];
#define MADDR_PER_GROUP 4	/* As per allocation method - Constant */
#define PTREE_PER_GROUP 3	/* As per allocation method - Constant */
#define PTREE_GROUPS(m) \
	((m + APM_SYS_MACADDR + (MADDR_PER_GROUP - 1)) / MADDR_PER_GROUP)
#define PTREE_ALLOCS(m) \
	(PTREE_GROUPS(m) * PTREE_PER_GROUP)

enum enode_index {
	S07,		/* SRC Port % 8 = 0, 1, 2, 3, 4, 5, 6, 7 */
	D07,		/* DST Port % 8 = 0, 1, 2, 3, 4, 5, 6, 7 */
	D76,		/* DST Port % 8 = 7, 0, 1, 2, 3, 4, 5, 6 */
	D65,		/* DST Port % 8 = 6, 7, 0, 1, 2, 3, 4, 5 */
	D54,		/* DST Port % 8 = 5, 6, 7, 0, 1, 2, 3, 4 */
	D43,		/* DST Port % 8 = 4, 5, 6, 7, 0, 1, 2, 3 */
	D32,		/* DST Port % 8 = 3, 4, 5, 6, 7, 0, 1, 2 */
	D21,		/* DST Port % 8 = 2, 3, 4, 5, 6, 7, 0, 1 */
	D10,		/* DST Port % 8 = 1, 2, 3, 4, 5, 6, 7, 0 */
	MLN,		/* Last Node for MAC PTREE_GROUPS */
	MKN,		/* Key Node for MAC PTREE_GROUPS */
};

#undef M
#define M (PTREE_ALLOCS(0) + MLN)

/*
 * Macro for getting relative node location from snptr for mac address
 * index i and mac short position p (0 1 2)
 */
#define MAC_NODE(i, p) ((PTREE_PER_GROUP * (i / MADDR_PER_GROUP)) + p)
/*
 * Macro for getting relative branch location from snptr for mac address
 * index i and mac short position p (0 1 2)
 */
#define MAC_BRANCH(i, p) ((i * 2) % (MADDR_PER_GROUP * 2))

/*
 * Single ptree group branch which serves to allow MADDR_PER_GROUP
 * mac addresses.  For single ptree group it requires PTREE_PER_GROUP
 * 8W-Decision Patricia Nodes.  All these ptree groups are cascaded to serve
 * mulitple of MADDR_PER_GROUP mac addresses. Total ptree groups is formulated
 * by PTREE_GROUPS macro based on m mac addresses. Total 8W-Decision Patricia
 * Nodes is formulated by PTREE_ALLOCS macro based on m mac addresses
 */
static struct ptree_branch branch[] = {
	{ 0xffff, 0, NEQT, PTREE_ALLOC(1),   EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* MACAddr00 byte [0:1] */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(0),   EW_BRANCH(2),  0, 0, JMP_REL, JMP_BW },	/* Jump to check MACAddr01 byte [0:1] */
	{ 0xffff, 0, NEQT, PTREE_ALLOC(1),   EW_BRANCH(2),  2, 1, JMP_REL, JMP_FW },	/* MACAddr01 byte [0:1] */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(0),   EW_BRANCH(4),  0, 0, JMP_REL, JMP_BW },	/* Jump to check MACAddr02 byte [0:1] */
	{ 0xffff, 0, NEQT, PTREE_ALLOC(1),   EW_BRANCH(4),  2, 1, JMP_REL, JMP_FW },	/* MACAddr02 byte [0:1] */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(0),   EW_BRANCH(6),  0, 0, JMP_REL, JMP_BW },	/* Jump to check MACAddr03 byte [0:1] */
	{ 0xffff, 0, NEQT, PTREE_ALLOC(1),   EW_BRANCH(6),  2, 1, JMP_REL, JMP_FW },	/* MACAddr03 byte [0:1] */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(3),   EW_BRANCH(0),  0, 0, JMP_REL, JMP_BW },	/* Jump to check next MACAddr byte [0:1] or LN */

	{ 0xffff, 0, NEQT, PTREE_ALLOC(2),   EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* MACAddr00 byte [2:3] */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(0),   EW_BRANCH(2),  2, 0, JMP_REL, JMP_BW },	/* Jump to check MACAddr01 byte [0:1] */
	{ 0xffff, 0, NEQT, PTREE_ALLOC(2),   EW_BRANCH(2),  2, 1, JMP_REL, JMP_FW },	/* MACAddr01 byte [2:3] */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(0),   EW_BRANCH(4),  2, 0, JMP_REL, JMP_BW },	/* Jump to check MACAddr02 byte [0:1] */
	{ 0xffff, 0, NEQT, PTREE_ALLOC(2),   EW_BRANCH(4),  2, 1, JMP_REL, JMP_FW },	/* MACAddr02 byte [2:3] */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(0),   EW_BRANCH(6),  2, 0, JMP_REL, JMP_BW },	/* Jump to check MACAddr03 byte [0:1] */
	{ 0xffff, 0, NEQT, PTREE_ALLOC(2),   EW_BRANCH(6),  2, 1, JMP_REL, JMP_FW },	/* MACAddr03 byte [2:3] */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(3),   EW_BRANCH(0),  2, 0, JMP_REL, JMP_BW },	/* Jump to check next MACAddr byte [0:1] or LN */

	{ 0xffff, 0, NEQT, PTREE_ALLOC(M),   EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* MACAddr00 byte [4:5] */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(0),   EW_BRANCH(2),  4, 0, JMP_REL, JMP_BW },	/* Jump to check MACAddr01 byte [0:1] */
	{ 0xffff, 0, NEQT, PTREE_ALLOC(M),   EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* MACAddr01 byte [4:5] */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(0),   EW_BRANCH(4),  4, 0, JMP_REL, JMP_BW },	/* Jump to check MACAddr02 byte [0:1] */
	{ 0xffff, 0, NEQT, PTREE_ALLOC(M),   EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* MACAddr02 byte [4:5] */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(0),   EW_BRANCH(6),  4, 0, JMP_REL, JMP_BW },	/* Jump to check MACAddr03 byte [0:1] */
	{ 0xffff, 0, NEQT, PTREE_ALLOC(M),   EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* MACAddr03 byte [4:5] */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(3),   EW_BRANCH(0),  4, 0, JMP_REL, JMP_BW },	/* Jump to check next MACAddr byte [0:1] or LN */
};

static struct ptree_branch branch_E[] = {
	/* S07 */
	{ 0xfff8, 0,  EQT, PTREE_ALLOC(D07), EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* SRC Port % 8 = 0 */
	{ 0xfff8, 1,  EQT, PTREE_ALLOC(D76), EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* SRC Port % 8 = 1 */
	{ 0xfff8, 2,  EQT, PTREE_ALLOC(D65), EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* SRC Port % 8 = 2 */
	{ 0xfff8, 3,  EQT, PTREE_ALLOC(D54), EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* SRC Port % 4 = 3 */
	{ 0xfff8, 4,  EQT, PTREE_ALLOC(D43), EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* SRC Port % 8 = 4 */
	{ 0xfff8, 5,  EQT, PTREE_ALLOC(D32), EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* SRC Port % 8 = 5 */
	{ 0xfff8, 6,  EQT, PTREE_ALLOC(D21), EW_BRANCH(0),  2, 1, JMP_REL, JMP_FW },	/* SRC Port % 8 = 6 */
	{ 0xfff8, 7,  EQT, PTREE_ALLOC(D10), EW_BRANCH(0),  2, 0, JMP_REL, JMP_FW },	/* SRC Port % 8 = 7 */

	/* D07 */
	{ 0xfff8, 0,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(0),  0, 1,       0,      0 },	/* DST Port % 8 = 0 -> Flow ID 0 */
	{ 0xfff8, 1,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(1),  0, 1,       0,      0 },	/* DST Port % 8 = 1 -> Flow ID 1 */
	{ 0xfff8, 2,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(2),  0, 1,       0,      0 },	/* DST Port % 8 = 2 -> Flow ID 2 */
	{ 0xfff8, 3,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(3),  0, 1,       0,      0 },	/* DST Port % 4 = 3 -> Flow ID 3 */
	{ 0xfff8, 4,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(4),  0, 1,       0,      0 },	/* DST Port % 8 = 4 -> Flow ID 4 */
	{ 0xfff8, 5,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(5),  0, 1,       0,      0 },	/* DST Port % 8 = 5 -> Flow ID 5 */
	{ 0xfff8, 6,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(6),  0, 1,       0,      0 },	/* DST Port % 8 = 6 -> Flow ID 6 */
	{ 0xfff8, 7,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(7),  0, 0,       0,      0 },	/* DST Port % 8 = 7 -> Flow ID 7 */

	/* D76 */
	{ 0xfff8, 7,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(0),  0, 1,       0,      0 },	/* DST Port % 8 = 7 -> Flow ID 0 */
	{ 0xfff8, 0,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(1),  0, 1,       0,      0 },	/* DST Port % 8 = 0 -> Flow ID 1 */
	{ 0xfff8, 1,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(2),  0, 1,       0,      0 },	/* DST Port % 8 = 1 -> Flow ID 2 */
	{ 0xfff8, 2,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(3),  0, 1,       0,      0 },	/* DST Port % 4 = 2 -> Flow ID 3 */
	{ 0xfff8, 3,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(4),  0, 1,       0,      0 },	/* DST Port % 8 = 3 -> Flow ID 4 */
	{ 0xfff8, 4,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(5),  0, 1,       0,      0 },	/* DST Port % 8 = 4 -> Flow ID 5 */
	{ 0xfff8, 5,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(6),  0, 1,       0,      0 },	/* DST Port % 8 = 5 -> Flow ID 6 */
	{ 0xfff8, 6,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(7),  0, 0,       0,      0 },	/* DST Port % 8 = 6 -> Flow ID 7 */

	/* D65 */
	{ 0xfff8, 6,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(0),  0, 1,       0,      0 },	/* DST Port % 8 = 6 -> Flow ID 0 */
	{ 0xfff8, 7,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(1),  0, 1,       0,      0 },	/* DST Port % 8 = 7 -> Flow ID 1 */
	{ 0xfff8, 0,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(2),  0, 1,       0,      0 },	/* DST Port % 8 = 0 -> Flow ID 2 */
	{ 0xfff8, 1,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(3),  0, 1,       0,      0 },	/* DST Port % 4 = 1 -> Flow ID 3 */
	{ 0xfff8, 2,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(4),  0, 1,       0,      0 },	/* DST Port % 8 = 2 -> Flow ID 4 */
	{ 0xfff8, 3,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(5),  0, 1,       0,      0 },	/* DST Port % 8 = 3 -> Flow ID 5 */
	{ 0xfff8, 4,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(6),  0, 1,       0,      0 },	/* DST Port % 8 = 4 -> Flow ID 6 */
	{ 0xfff8, 5,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(7),  0, 0,       0,      0 },	/* DST Port % 8 = 5 -> Flow ID 7 */

	/* D54 */
	{ 0xfff8, 5,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(0),  0, 1,       0,      0 },	/* DST Port % 8 = 5 -> Flow ID 0 */
	{ 0xfff8, 6,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(1),  0, 1,       0,      0 },	/* DST Port % 8 = 6 -> Flow ID 1 */
	{ 0xfff8, 7,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(2),  0, 1,       0,      0 },	/* DST Port % 8 = 7 -> Flow ID 2 */
	{ 0xfff8, 0,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(3),  0, 1,       0,      0 },	/* DST Port % 4 = 0 -> Flow ID 3 */
	{ 0xfff8, 1,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(4),  0, 1,       0,      0 },	/* DST Port % 8 = 1 -> Flow ID 4 */
	{ 0xfff8, 2,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(5),  0, 1,       0,      0 },	/* DST Port % 8 = 2 -> Flow ID 5 */
	{ 0xfff8, 3,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(6),  0, 1,       0,      0 },	/* DST Port % 8 = 3 -> Flow ID 6 */
	{ 0xfff8, 4,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(7),  0, 0,       0,      0 },	/* DST Port % 8 = 4 -> Flow ID 7 */

	/* D43 */
	{ 0xfff8, 4,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(0),  0, 1,       0,      0 },	/* DST Port % 8 = 4 -> Flow ID 0 */
	{ 0xfff8, 5,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(1),  0, 1,       0,      0 },	/* DST Port % 8 = 5 -> Flow ID 1 */
	{ 0xfff8, 6,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(2),  0, 1,       0,      0 },	/* DST Port % 8 = 6 -> Flow ID 2 */
	{ 0xfff8, 7,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(3),  0, 1,       0,      0 },	/* DST Port % 4 = 7 -> Flow ID 3 */
	{ 0xfff8, 0,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(4),  0, 1,       0,      0 },	/* DST Port % 8 = 0 -> Flow ID 4 */
	{ 0xfff8, 1,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(5),  0, 1,       0,      0 },	/* DST Port % 8 = 1 -> Flow ID 5 */
	{ 0xfff8, 2,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(6),  0, 1,       0,      0 },	/* DST Port % 8 = 2 -> Flow ID 6 */
	{ 0xfff8, 3,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(7),  0, 0,       0,      0 },	/* DST Port % 8 = 3 -> Flow ID 7 */

	/* D32 */
	{ 0xfff8, 3,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(0),  0, 1,       0,      0 },	/* DST Port % 8 = 3 -> Flow ID 0 */
	{ 0xfff8, 4,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(1),  0, 1,       0,      0 },	/* DST Port % 8 = 4 -> Flow ID 1 */
	{ 0xfff8, 5,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(2),  0, 1,       0,      0 },	/* DST Port % 8 = 5 -> Flow ID 2 */
	{ 0xfff8, 6,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(3),  0, 1,       0,      0 },	/* DST Port % 4 = 6 -> Flow ID 3 */
	{ 0xfff8, 7,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(4),  0, 1,       0,      0 },	/* DST Port % 8 = 7 -> Flow ID 4 */
	{ 0xfff8, 0,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(5),  0, 1,       0,      0 },	/* DST Port % 8 = 0 -> Flow ID 5 */
	{ 0xfff8, 1,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(6),  0, 1,       0,      0 },	/* DST Port % 8 = 1 -> Flow ID 6 */
	{ 0xfff8, 2,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(7),  0, 0,       0,      0 },	/* DST Port % 8 = 2 -> Flow ID 7 */

	/* D21 */
	{ 0xfff8, 2,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(0),  0, 1,       0,      0 },	/* DST Port % 8 = 2 -> Flow ID 0 */
	{ 0xfff8, 3,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(1),  0, 1,       0,      0 },	/* DST Port % 8 = 3 -> Flow ID 1 */
	{ 0xfff8, 4,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(2),  0, 1,       0,      0 },	/* DST Port % 8 = 4 -> Flow ID 2 */
	{ 0xfff8, 5,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(3),  0, 1,       0,      0 },	/* DST Port % 4 = 5 -> Flow ID 3 */
	{ 0xfff8, 6,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(4),  0, 1,       0,      0 },	/* DST Port % 8 = 6 -> Flow ID 4 */
	{ 0xfff8, 7,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(5),  0, 1,       0,      0 },	/* DST Port % 8 = 7 -> Flow ID 5 */
	{ 0xfff8, 0,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(6),  0, 1,       0,      0 },	/* DST Port % 8 = 0 -> Flow ID 6 */
	{ 0xfff8, 1,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(7),  0, 0,       0,      0 },	/* DST Port % 8 = 1 -> Flow ID 7 */

	/* D10 */
	{ 0xfff8, 1,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(0),  0, 1,       0,      0 },	/* DST Port % 8 = 1 -> Flow ID 0 */
	{ 0xfff8, 2,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(1),  0, 1,       0,      0 },	/* DST Port % 8 = 2 -> Flow ID 1 */
	{ 0xfff8, 3,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(2),  0, 1,       0,      0 },	/* DST Port % 8 = 3 -> Flow ID 2 */
	{ 0xfff8, 4,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(3),  0, 1,       0,      0 },	/* DST Port % 4 = 4 -> Flow ID 3 */
	{ 0xfff8, 5,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(4),  0, 1,       0,      0 },	/* DST Port % 8 = 5 -> Flow ID 4 */
	{ 0xfff8, 6,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(5),  0, 1,       0,      0 },	/* DST Port % 8 = 6 -> Flow ID 5 */
	{ 0xfff8, 7,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(6),  0, 1,       0,      0 },	/* DST Port % 8 = 7 -> Flow ID 6 */
	{ 0xfff8, 0,  EQT, PTREE_ALLOC(MLN), EW_BRANCH(7),  0, 0,       0,      0 },	/* DST Port % 8 = 0 -> Flow ID 7 */

	/* MLN */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(MKN), KEY_INDEX(0),  0, 0,       0,      0 },	/* Last Node allowing FlowID 0 */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(MKN), KEY_INDEX(1),  0, 0,       0,      0 },	/* Last Node allowing FlowID 1 */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(MKN), KEY_INDEX(2),  0, 0,       0,      0 },	/* Last Node allowing FlowID 2 */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(MKN), KEY_INDEX(3),  0, 0,       0,      0 },	/* Last Node allowing FlowID 3 */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(MKN), KEY_INDEX(4),  0, 0,       0,      0 },	/* Last Node allowing FlowID 4 */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(MKN), KEY_INDEX(5),  0, 0,       0,      0 },	/* Last Node allowing FlowID 5 */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(MKN), KEY_INDEX(6),  0, 0,       0,      0 },	/* Last Node allowing FlowID 6 */
	{ 0xffff, 0,  EQT, PTREE_ALLOC(MKN), KEY_INDEX(7),  0, 0,       0,      0 },	/* Last Node allowing FlowID 7 */
};

static struct ptree_dn dn[] = {
	{ START_NODE,	DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8,  &branch[0] },	/* Check for MACAddr00-03 byte [0:1] */
	{ INTERIM_NODE,	DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8,  &branch[8] },	/* Check for MACAddr00-03 byte [2:3] */
	{ INTERIM_NODE,	DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8, &branch[16] },	/* Check for MACAddr00-03 byte [4:5] */
};

static struct ptree_dn dn_E[] = {
	{ START_NODE,   DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8, &branch_E[0] },      /* SRC Port % 8 = 0-7 */
	{ INTERIM_NODE, DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8, &branch_E[8] },      /* DST Port % 8 = 0-7 */
	{ INTERIM_NODE, DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8, &branch_E[16] },     /* DST Port % 8 = 7-6 */
	{ INTERIM_NODE, DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8, &branch_E[24] },     /* DST Port % 8 = 6-5 */
	{ INTERIM_NODE, DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8, &branch_E[32] },     /* DST Port % 8 = 5-4 */
	{ INTERIM_NODE, DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8, &branch_E[40] },     /* DST Port % 8 = 4-3 */
	{ INTERIM_NODE, DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8, &branch_E[48] },     /* DST Port % 8 = 3-2 */
	{ INTERIM_NODE, DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8, &branch_E[56] },     /* DST Port % 8 = 2-1 */
	{ INTERIM_NODE, DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8, &branch_E[64] },     /* DST Port % 8 = 1-0 */
	{ LAST_NODE,    DBPTR_DROP(0), AVL_SEARCH(NO_BYTE),    0, 0, 0, 8, &branch_E[72] },     /* Last Node for FlowID 0-7 */
};

static struct ptree_node node[] = {
	{ PTREE_ALLOC(0),   EWDN, 0, (struct ptree_dn*)&dn[0] },
	{ PTREE_ALLOC(1),   EWDN, 0, (struct ptree_dn*)&dn[1] },
	{ PTREE_ALLOC(2),   EWDN, 0, (struct ptree_dn*)&dn[2] },
};

static struct ptree_node node_E[] = {
	{ PTREE_ALLOC(S07), EWDN, 0, (struct ptree_dn*)&dn_E[S07] },
	{ PTREE_ALLOC(D07), EWDN, 0, (struct ptree_dn*)&dn_E[D07] },
	{ PTREE_ALLOC(D76), EWDN, 0, (struct ptree_dn*)&dn_E[D76] },
	{ PTREE_ALLOC(D65), EWDN, 0, (struct ptree_dn*)&dn_E[D65] },
	{ PTREE_ALLOC(D54), EWDN, 0, (struct ptree_dn*)&dn_E[D54] },
	{ PTREE_ALLOC(D43), EWDN, 0, (struct ptree_dn*)&dn_E[D43] },
	{ PTREE_ALLOC(D32), EWDN, 0, (struct ptree_dn*)&dn_E[D32] },
	{ PTREE_ALLOC(D21), EWDN, 0, (struct ptree_dn*)&dn_E[D21] },
	{ PTREE_ALLOC(D10), EWDN, 0, (struct ptree_dn*)&dn_E[D10] },
	{ PTREE_ALLOC(MLN), EWDN, 0, (struct ptree_dn*)&dn_E[MLN] },
	{ PTREE_ALLOC(MKN),   KN, 0,     (struct ptree_kn*)&kn[0] },
	{ PTREE_ALLOC(MKN),   KN, 1,     (struct ptree_kn*)&kn[1] },
	{ PTREE_ALLOC(MKN),   KN, 2,     (struct ptree_kn*)&kn[2] },
	{ PTREE_ALLOC(MKN),   KN, 3,     (struct ptree_kn*)&kn[3] },
	{ PTREE_ALLOC(MKN),   KN, 4,     (struct ptree_kn*)&kn[4] },
	{ PTREE_ALLOC(MKN),   KN, 5,     (struct ptree_kn*)&kn[5] },
	{ PTREE_ALLOC(MKN),   KN, 6,     (struct ptree_kn*)&kn[6] },
	{ PTREE_ALLOC(MKN),   KN, 7,     (struct ptree_kn*)&kn[7] },
};

const u8 apm_usr_macmask[ETH_ALEN + 2] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const u8 apm_sys_macmask[APM_SYS_MACADDR][ETH_ALEN + 2] = {
	[ETHERNET_MACADDR]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	[BROADCAST_MACADDR] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	[UNICAST_MACADDR]   = {0xfe, 0xff, 0xff, 0xff, 0xff, 0xff},
	[MULTICAST_MACADDR] = {0xfe, 0xff, 0xff, 0xff, 0xff, 0xff},
};

const u8 apm_sys_macaddr[APM_SYS_MACADDR][ETH_ALEN + 2] = {
	[ETHERNET_MACADDR]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	[BROADCAST_MACADDR] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	[UNICAST_MACADDR]   = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	[MULTICAST_MACADDR] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00},
};

unsigned int default_rx_dbptr[MAX_ENET_PORTS];

/* Classifier Configurations for Linux */
int apm_preclass_init(u8 port_id, struct eth_queue_ids *eth_q)
{
	int i, rc;
	struct apm_ptree_config *ptree_config;
	u32 ptree_groups;
	u32 ptree_allocs;
	u32 mac_entries = APM_MAX_UC_MC_MACADDR;
	u32 index, mask, data;

	memset(&dbptr, 0, sizeof(dbptr));
	memset(&kn, 0, sizeof(kn));

	ptree_config = apm_add_ptree_config(port_id, CLE_PTREE_DEFAULT);
	if (ptree_config == NULL) {
		PCLS_ERR("Add Patricia Tree Default Configuration error for "
			"port %d\n", port_id);
		return APM_RC_ERROR;
	}

	PCLS_DBG("Create Preclassifier DB entries for Ping Tree port %d\n",
		port_id);

	ptree_groups = PTREE_GROUPS(mac_entries);
	ptree_allocs = PTREE_ALLOCS(mac_entries);

	if (port_id == ENET_0 || port_id == ENET_2)
                apm_gbl_cle_wr32(PID2CID[port_id], PORTNUM0_ADDR, 0);
	else if (port_id == ENET_1 || port_id == ENET_3)
                apm_gbl_cle_wr32(PID2CID[port_id], PORTNUM1_ADDR, 1);

	switch (MAX_RX_QUEUES) {
		case 0:
		case 1:
			mask = 0xffff;
			break;
		case 2:
		case 3:
			mask = 0xfffe;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			mask = 0xfffc;
			break;
		case 8:
		default:
			mask = 0xfff8;
	};

	/* Clear Mask, Data and Operations for all branches for SRC Port */
	for (i = 0; i < 8; i++) {
		branch_E[i].mask = 0xffff;
		branch_E[i].data = 0;
		branch_E[i].operation = NEQT;
	}

	/* Clear Mask, Data and Operations for all branches for DST Port */
	for (i = 0; i < 64; i++) {
		branch_E[8 + i].mask = 0xffff;
		branch_E[8 + i].data = 0;
		branch_E[8 + i].operation = NEQT;
	}

	/*
	 * Update Mask, Data and Operations based on number of queues
	 * for SRC Port
	 */
	for (i = 0; i < MAX_RX_QUEUES; i++) {
		branch_E[i].mask = mask;
		branch_E[i].data = i;
		branch_E[i].operation = EQT;
	}

	/*
	 * Update Mask, Data and Operations based on number of queues
	 * for DST Port
	 */
	for (i = 0; i < MAX_RX_QUEUES * MAX_RX_QUEUES; i++) {
		index = 8 + (8 * (i / MAX_RX_QUEUES)) + (i % MAX_RX_QUEUES);
		data = (i - (i / MAX_RX_QUEUES)) % MAX_RX_QUEUES;
		branch_E[index].mask = mask;
		branch_E[index].data = data;
		branch_E[index].operation = EQT;
	}

	/*
	 * Update MAC matching next_node_index with PTREE_ALLOC(index)
	 * of last node. Only identify Source Port and Destination Port
	 * for our MAC Address
	 */
	branch[16].next_packet_pointer = 30;
	branch[16].next_node_index = PTREE_ALLOC(ptree_allocs + S07);
	branch[18].next_node_index = PTREE_ALLOC(ptree_allocs + MLN);
	branch[20].next_node_index = PTREE_ALLOC(ptree_allocs + MLN);
	branch[22].next_node_index = PTREE_ALLOC(ptree_allocs + MLN);

	for (i = 0; i < ptree_groups; i++) {
		branch[7].next_node_index = PTREE_ALLOC(PTREE_ALLOCS(0) * i + 3);
		branch[15].next_node_index = PTREE_ALLOC(PTREE_ALLOCS(0) * i + 3);
		branch[23].next_node_index = PTREE_ALLOC(PTREE_ALLOCS(0) * i + 3);

		if ((rc = apm_ptree_alloc(port_id, ARRAY_SIZE(node),
				0, node, NULL, ptree_config)) !=
				APM_RC_OK) {
			PCLS_ERR("Preclass init error %d \n", rc);
			return rc;
		}

		/*
		 * Once allocated, update MAC matching next_node_index with
		 * absolute index of last node
		 */
		branch[16].next_packet_pointer = 2;
		branch[16].next_node_index =
			ptree_config->start_node_ptr
			+ ptree_allocs + MLN;
		branch[18].next_node_index =
			ptree_config->start_node_ptr
			+ ptree_allocs + MLN;
		branch[20].next_node_index =
			ptree_config->start_node_ptr
			+ ptree_allocs + MLN;
		branch[22].next_node_index =
			ptree_config->start_node_ptr
			+ ptree_allocs + MLN;

		branch[7].next_node_index = 
			ptree_config->start_node_ptr 
			+ PTREE_ALLOCS(0) * i + 3;
		branch[15].next_node_index = 
			ptree_config->start_node_ptr 
			+ PTREE_ALLOCS(0) * i + 3;
		branch[23].next_node_index = 
			ptree_config->start_node_ptr 
			+ PTREE_ALLOCS(0) * i + 3;
	}

	for (i = 0; i < MAX_RX_QUEUES; i++) {
		u16 dstqid;

		dbptr[i].index = DBPTR_ALLOC(CLE_DB_INDEX + i);
#ifdef APM_ENET_QM_LOOPBACK
		dbptr[i].HR = 1;
		dbptr[i].H0Enq_Num = QM_RSV_UNCONFIG_COMP_Q;
		if (eth_q->hw_tx[i].qid)
			dstqid = QMTM_QUEUE_ID(eth_q->qm_ip,
					eth_q->hw_tx[i].qid);
		else
			dstqid = QMTM_QUEUE_ID(eth_q->qm_ip,
					eth_q->hw_tx[0].qid);
		if (eth_q->hw_fp[i].pbn)
			dbptr[i].fpsel  = eth_q->hw_fp[i].pbn - 0x20;
		else
			dbptr[i].fpsel  = eth_q->hw_fp[0].pbn - 0x20;
#else
		/*
		 * stashing table
		 * 0- no, 1- 1st buffer, 2- entire packet, 3- 64 bytes
		 */
		dbptr[i].stash = 0x1;
		if (eth_q->rx[i].qid)
			dstqid = QMTM_QUEUE_ID(eth_q->qm_ip,
					eth_q->rx[i].qid);
		else
			dstqid = QMTM_QUEUE_ID(eth_q->qm_ip,
					eth_q->default_rx_qid);
		if (eth_q->rx_fp[i].pbn)
			dbptr[i].fpsel  = eth_q->rx_fp[i].pbn - 0x20;
		else
			dbptr[i].fpsel  = eth_q->default_rx_fp_pbn - 0x20;
#endif
		dbptr[i].dstqidL = dstqid & 0x7f;
		dbptr[i].dstqidH = (dstqid >> 7) & 0x1f;
#ifdef CONFIG_XGENE_NET_JUMBO_FRAME
		if (eth_q->rx_nxtfp[i].pbn)
			dbptr[i].nxtfpsel  = eth_q->rx_nxtfp[i].pbn - 0x20;
		else
			dbptr[i].nxtfpsel  = eth_q->default_rx_nxtfp_pbn - 0x20;
#endif
		/* Report Rx timestamp in H1Info */
		/* dbptr[i].cle_insert_timestamp = 1; */

		kn[i].priority = 2;
		kn[i].result_pointer = DBPTR_ALLOC(CLE_DB_INDEX + i);
	}

	/* Allocate the last node and key node */
	PCLS_DBG("Create Patricia Tree Nodes for Ping Tree\n");
	if ((rc = apm_ptree_alloc(port_id, ARRAY_SIZE(node_E) - 8 + MAX_RX_QUEUES,
			ARRAY_SIZE(dbptr), node_E, dbptr, ptree_config)) !=
			APM_RC_OK) {
		PCLS_ERR("Preclass init error %d for port %d\n", rc, port_id);
		return rc;
	}

	default_rx_dbptr[port_id] = ptree_config->start_dbptr;

	/*
	 * Once all nodes are allocated, update snptr and max_hop with
	 * correct value
	 */
	ptree_config->start_node_ptr -= ptree_allocs;
	ptree_config->max_hop += (ptree_allocs * 8);
	ptree_config->default_result = DFCLSRESDBPRIORITY0_WR(1) |
					DFCLSRESDBPTR0_WR(START_DB_INDEX);
	/* AVL tree does not exist in inline classifier engine for RGMII */
	if (port_id == MENET)
		return rc;

	/* Initialize Search Engine, 16 Byte search string */
	if ((rc = apm_cle_avl_init(port_id, SEARCH_STRING_SIZE16B)))
		PCLS_ERR("AVL init error %d for port %d\n", rc, port_id);

	return rc;
}

int apm_preclass_update_mac(u8 port_id, enum apm_macaddr_type type,
		u8 index, const u8 *macmask, const u8 *macaddr)
{
	int rc = APM_RC_OK;
	struct apm_ptree_config *ptree_config;
	int i;
	u32 base_node_ptr;

	ptree_config = apm_find_ptree_config(port_id, CLE_PTREE_DEFAULT);
	if (!ptree_config) {
		rc = APM_RC_ERROR;
		goto _ret_preclass_update_mac;
	}

	base_node_ptr = ptree_config->start_node_ptr;

	for (i = 0; i < 3; i++) {
		struct ptree_branch pbranch;

		memset(&pbranch, 0, sizeof(pbranch));
		pbranch.mask = (macaddr == NULL) ? 0xffff :
			((u16)macmask[2 * i] << 8) | macmask[(2 * i) + 1];
		pbranch.data = (macaddr == NULL) ? 0x0000 :
			((u16)macaddr[2 * i] << 8) | macaddr[(2 * i) + 1];
		pbranch.operation = (macaddr == NULL ? NEQT : EQT);
		rc |= apm_set_ptree_node_branch(port_id,
			base_node_ptr + MAC_NODE(index, i),
			0, MAC_BRANCH(index, i), &pbranch, SET_BRANCH_MDO);
	}

_ret_preclass_update_mac:
	return rc;
}
#elif defined(SET_XGR_MODE)
static struct ptree_kn kn[2];
static struct apm_cle_dbptr dbptr[2];
const static struct ptree_branch branch[] = {
	{      0, 0x8915, NEQT, PTREE_ALLOC(1), EW_BRANCH(0), 0,  1, 0, 0 }, /* Non-XGR */
	{ 0xffff,      0,  EQT, PTREE_ALLOC(1), EW_BRANCH(1), 0,  0, 0, 0 }, /* XGR */
	{ 0xffff,      0,  EQT, PTREE_ALLOC(2), KEY_INDEX(0), 0,  0, 0, 0 }, /* All Non-XGR */
	{ 0xffff,      0,  EQT, PTREE_ALLOC(2), KEY_INDEX(1), 0,  0, 0, 0 }, /* All XGR */
};

const static struct ptree_dn dn[] = {
	{ START_NODE, DBPTR_DROP(0), AVL_SEARCH(NO_BYTE), 0, 0, 0, 2, &branch[0] }, /* EtherType filtering */
	{ LAST_NODE,  DBPTR_DROP(0), AVL_SEARCH(NO_BYTE), 0, 0, 0, 2, &branch[2] }, /* Last Node for all frames */
};

const static struct ptree_node node[] = {
	{ PTREE_ALLOC(0), EWDN, 0, (struct ptree_dn*)&dn[0] },
	{ PTREE_ALLOC(1), EWDN, 0, (struct ptree_dn*)&dn[1] },
	{ PTREE_ALLOC(2),   KN, 0, (struct ptree_kn*)&kn[0] },
	{ PTREE_ALLOC(2),   KN, 1, (struct ptree_kn*)&kn[1] },
};

/* Classifier Configurations for Linux */
int apm_preclass_init(u8 port_id, struct eth_queue_ids *eth_q)
{
	int i, rc = APM_RC_ERROR;
	struct apm_ptree_config *ptree_config;

	memset(dbptr, 0, sizeof(dbptr));
	memset(kn, 0, sizeof(kn));

	ptree_config = apm_add_ptree_config(port_id, CLE_PTREE_DEFAULT);
	if (!ptree_config) {
		PCLS_ERR("Add Patricia Tree Default Configuration error for "
			"port %d\n", port_id);
		return rc;
	}

	for (i = 0; i < ARRAY_SIZE(dbptr); i++) {
		dbptr[i].index = DBPTR_ALLOC(CLE_DB_INDEX + i);
		dbptr[i].drop = 1;
		kn[i].result_pointer = DBPTR_ALLOC(CLE_DB_INDEX + i);
	}

	PCLS_DBG("Create Patricia Tree Nodes for Ping Tree\n");
	if ((rc = apm_ptree_alloc(port_id, ARRAY_SIZE(node),
			ARRAY_SIZE(dbptr), node, dbptr, ptree_config))) {
		PCLS_ERR("Preclass init error %d \n", rc);
		return rc;
	}
	ptree_config->start_pkt_ptr = 12;

	return rc;
}
#else
static struct ptree_kn kn;
static struct apm_cle_dbptr dbptr;
const static struct ptree_branch branch[] = {
	{      0, 0x0806, EQT, PTREE_ALLOC(2), 0, 0,  1, 0, 0 }, /* ARP	*/
	{      0, 0x8100, EQT, PTREE_ALLOC(2), 0, 0,  1, 0, 0 }, /* VLAN */
	{      0, 0x0800, EQT, PTREE_ALLOC(1), 0, 22, 1, 0, 0 }, /* IPv4 */
	{      0, 0x86dd, EQT, PTREE_ALLOC(2), 0, 0,  0, 0, 0 }, /* IPv6 */
	{ 0xfff0, 0x0007, AND, PTREE_ALLOC(2), 0, 0,  1, 0, 0 }, /* Allow upto TCP */
	{      0, 0x0017, EQT, PTREE_ALLOC(2), 0, 0,  0, 0, 0 }, /* Allow UDP */
	{ 0xffff,      0, EQT, PTREE_ALLOC(3), 0, 0,  0, 0, 0 }, /* Allow all of the above */
};

const static struct ptree_dn dn[] = {
	{ START_NODE,	DBPTR_DROP(0), 0, 0, 0, 0, 4, &branch[0] },
	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 2, &branch[4] },
	{ LAST_NODE,	DBPTR_DROP(0), 0, 0, 0, 0, 1, &branch[6] },
};

const static struct ptree_node node[] = {
	{ PTREE_ALLOC(0), EWDN, 0, (struct ptree_dn*)&dn[0] },
	{ PTREE_ALLOC(1), EWDN, 0, (struct ptree_dn*)&dn[1] },
	{ PTREE_ALLOC(2), EWDN, 0, (struct ptree_dn*)&dn[2] },
	{ PTREE_ALLOC(3),   KN, 0,    (struct ptree_kn*)&kn },
};

/* Classifier Configurations for Linux */
int apm_preclass_init(u8 port_id, struct eth_queue_ids *eth_q)
{
	int rc;
	struct apm_ptree_config *ptree_config;
	u16 dstqid;

	memset(&dbptr, 0, sizeof(dbptr));
	memset(&kn, 0, sizeof(kn));

	ptree_config = apm_add_ptree_config(port_id, CLE_PTREE_DEFAULT);
	if (!ptree_config) {
		PCLS_ERR("Add Patricia Tree Default Configuration error for "
			"port %d\n", port_id);
		return APM_RC_ERROR;
	}

	PCLS_DBG("Create Preclassifier DB entries for Ping Tree port %d\n",
		port_id);

	dbptr.index = DBPTR_ALLOC(CLE_DB_INDEX);
#ifdef APM_ENET_QM_LOOPBACK
	dbptr.HR = 1;
	dbptr.H0Enq_Num = QM_RSV_UNCONFIG_COMP_Q;
	dstqid = QMTM_QUEUE_ID(eth_q->qm_ip, eth_q->hw[0].qid);
	dbptr.fpsel  = eth_q->hw_fp[0].pbn - 0x20;
#else
	dstqid = QMTM_QUEUE_ID(eth_q->qm_ip, eth_q->default_rx_qid);
	dbptr.fpsel  = eth_q->default_rx_fp_pbn - 0x20;
#endif
	dbptr.dstqidL = dstqid & 0x7f;
	dbptr.dstqidH = (dstqid >> 7) & 0x1f;
#ifdef CONFIG_XGENE_NET_JUMBO_FRAME
	dbptr.nxtfpsel = eth_q->rx_nxtfp_pbn - 0x20;
#endif

	kn.result_pointer = DBPTR_ALLOC(CLE_DB_INDEX);

	PCLS_DBG("Create Patricia Tree Nodes for Ping Tree\n");
	PCLS_DBG(" dbptr.index=0x%x dstqid=0x%x fpsel=0x%x nxtfpsel=0x%x\n",
		dbptr.index, dstqid, dbptr.fpsel, dbptr.nxtfpsel);
	if ((rc = apm_ptree_alloc(port_id, ARRAY_SIZE(node),
			1, node, &dbptr, ptree_config))) {
		PCLS_ERR("Preclass init error %d \n", rc);
		return rc;
	}
	ptree_config->start_pkt_ptr = 12;

	if (port_id != MENET)
		return rc;

#ifdef CONFIG_PM_FIXME
	PCLS_DBG("Preclass Wol Tree init\n");
	if ((rc = apm_preclass_init_wol_tree(port_id)) != APM_RC_OK) {
		PCLS_ERR("Preclass Wol Tree init error %d\n", rc);
		return rc;
	}
#endif

	return rc;
}
#endif

#ifdef CONFIG_PM_FIXME
/* WoL Intermediate  Classifier Configurations for Linux Enet Port0 */
#define ISN 			PTREE_ALLOC(0)
#define ILN 			PTREE_ALLOC(3)

static struct ptree_kn kn_i;
static struct ptree_branch branch_i[] = {
	{      0, 0x0806, EQT, ISN+2, 0, 0,  1, 0, 0 },	/* ARP	*/
	{      0, 0x0800, EQT, ISN+1, 0, 22, 1, 0, 0 },	/* IPv4 */
	{      0, 0x86dd, EQT, ISN+2, 0, 0,  0, 0, 0 },	/* IPv6 */
	{ 0xfff0, 0x0007, AND, ISN+2, 0, 0,  1, 0, 0 },	/* Allow upto TCP */
	{      0, 0x0017, EQT, ISN+2, 0, 0,  0, 0, 0 },	/* Allow UDP */
	{ 0xffff,      0, EQT,   ILN, 0, 0,  0, 0, 0 },
						/* Allow all of the above */
};

static struct ptree_dn dn_i[] = {
	{ START_NODE,	DBPTR_ALLOC(CLE_DB_INDEX), 0, 0, 0, 0, 3, &branch_i[0] },
	{ INTERIM_NODE, DBPTR_ALLOC(CLE_DB_INDEX), 0, 0, 0, 0, 2, &branch_i[3] },
	{ LAST_NODE,	DBPTR_ALLOC(CLE_DB_INDEX), 0, 0, 0, 0, 1, &branch_i[5] },
};

static struct ptree_node node_i[] = {
	{ ISN,   EWDN, 0, (struct ptree_dn*)&dn_i[0] },
	{ ISN+1, EWDN, 0, (struct ptree_dn*)&dn_i[1] },
	{ ISN+2, EWDN, 0, (struct ptree_dn*)&dn_i[2] },
	{ ILN,     KN, 0,    (struct ptree_kn*)&kn_i },
};

int apm_preclass_init_wol_inter_tree(u8 port_id)
{
	int rc;
	struct apm_cle_dbptr dbptr_i = {0};
	struct apm_ptree_config *ptree_config;
#ifdef  PKT_LOSSLESS_WOL
	int cpu_id = 0;
#endif

	memset(&kn_i, 0, sizeof(kn_i));

	ptree_config = apm_add_ptree_config(port_id, CLE_PTREE_WOL_INT);
	if (!ptree_config) {
		PCLS_ERR("Add Patricia Tree WOL Intermediate Configuration "
			"error for port %d\n", port_id);
		return APM_RC_ERROR;
	}

	PCLS_DBG("Create Intermediate WoL Preclassifier DB entry \n");
	dbptr_i.index = DBPTR_ALLOC(CLE_DB_INDEX);
#ifdef  PKT_LOSSLESS_WOL
	dbptr_i.dstqidL = DSLEEP_ENET_RX_FQ_TO_DDR & 0x7f;
	dbptr_i.dstqidH = (DSLEEP_ENET_RX_FQ_TO_DDR >> 7) & 0x1f;
	dbptr_i.fpsel  = DSLEEP_ENET_RX_FQ_TO_DDR_PBN - 0x20;
	dbptr_i.h0_info = cpu_id;   /* TBD - Handle CPU ID dynamically */
#else
	dbptr_i.drop = 1;
#endif
	/* Report Rx timestamp in H1Info */
#ifndef CONFIG_ARCH_MSLIM
	dbptr.cle_insert_timestamp = 1;
#endif
	kn_i.result_pointer = DBPTR_ALLOC(CLE_DB_INDEX);

	PCLS_DBG("Create Patricia Tree Nodes for Ping Tree \n");
	if ((rc = apm_ptree_alloc(port_id, ARRAY_SIZE(node_i), 1,
			&node_i[0], &dbptr_i, ptree_config))) {
		PCLS_ERR("Preclass init error %d \n", rc);
		rc = APM_RC_ERROR;
	} else {
		rc = ptree_config->start_node_ptr;
		ptree_config->start_pkt_ptr = 12;
	}

	return rc;
}

#define AMD_MAGIC_PKT_ENABLE	0x1
#define ARP_ICMP_REPLY_ENABLE	0x2 /* ARP will send normal reply */
#define ARP_ICMP_WOL_ENABLE	0x4 /* ARP will be used for Wol */
#define MAC_WOL_ENABLE		0x8

#define APM_CLE_WOL_PROC_ENTRY "apm_cle_wol_opts"

/* Classifier Configurations for WoL Tree */
static struct ptree_kn kn_b;
static struct ptree_kn kn_drop_pkt;
unsigned short wol_proto_enable =
	AMD_MAGIC_PKT_ENABLE | ARP_ICMP_REPLY_ENABLE;
struct apm_cle_dbptr dbptr_b = {0};

#ifdef CONFIG_NET_PROTO
#define LN PTREE_ALLOC(7)
static struct ptree_branch branch_b[] = {
	{      0, 0x0806, EQT, LN,             0, 0,  1, 0, 0 },	/* ARP	*/
	{      0, 0x0800, EQT, PTREE_ALLOC(1), 0, 22, 0, 0, 0 },	/* IPv4 */
	{      0, 0x86dd, EQT, PTREE_ALLOC(3), 0, 20, 0, 0, 0 },	/* IPv6 */
	{ 0xff00, 0x0001, EQT, PTREE_ALLOC(4), 0, 34, 1, 0, 0 },	/* Allow ICMP */
	{ 0xff00, 0x0011, EQT, PTREE_ALLOC(2), 0, 36, 0, 0, 0 },	/* Allow UDP  */
	{      0,  UDP_DEFAULT_WOL_PORT,
			  EQT, LN,             0, 0,  0, 0, 0 },	/* UDP Dst Port */
	{      0,    161, EQT, LN,             0, 0,  1, 0, 0 },	/* UDP SNMP Req */
	{      0,    137, EQT, LN,             0, 0,  1, 0, 0 },	/* NetBios Req */
	{      0,   5355, EQT, PTREE_ALLOC(6), 0, 0,  0, 0, 0 },	/* LLMNR  */
	{ 0x00ff, 0x3A00, EQT, PTREE_ALLOC(5), 0, 54, 0, 0, 0 },	/* Allow ICMPV6 */
	{ 0x00ff, 0x0800, EQT, LN,             0, 0,  0, 0, 0 },	/* Allow ICMP Req */
	{ 0x00ff, 0x8700, EQT, LN,             0, 0,  0, 0, 0 },	/* Allow ICMPv6 ND */
	{ 0x00ff, 0x8000, EQT, LN,             0, 0,  0, 0, 0 },	/* Allow ICMPv6 Req */
	{ 0x00ff, 0x8200, EQT, LN,             0, 0,  0, 0, 0 },	/* Allow ICMPv6 MLD */
	{ 0x0fff,    0x0, EQT, LN,             0, 0,  0, 0, 0 },	/* Allow LLMNR Req */
	{ 0xffff,      0, EQT, LN+1,           0, 0,  0, 0, 0 },
};						/* Allow all of the above */

static struct ptree_dn dn_b[] = {
	{ START_NODE,	DBPTR_DROP(0), 0, 0, 0, 0, 3, &branch_b[0] },
	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 2, &branch_b[3] },
	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 4, &branch_b[5] },  /* UDP Port */
	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 1, &branch_b[9] },  /* IPv6 Nxt */
	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 1, &branch_b[10] }, /* ICMP Typ */
	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 3, &branch_b[11] },
							/* ICMP6 Type */
	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 1, &branch_b[14] },
								/* LLMNR REQ */
	{ LAST_NODE,    DBPTR_DROP(0), 0, 0, 0, 0, 1, &branch_b[15] },
};

static struct ptree_node node_b[] = {
	{ PTREE_ALLOC(0), EWDN, 0, (struct ptree_dn*)&dn_b[0] },
	{ PTREE_ALLOC(1), EWDN, 0, (struct ptree_dn*)&dn_b[1] },
	{ PTREE_ALLOC(2), EWDN, 0, (struct ptree_dn*)&dn_b[2] },
	{ PTREE_ALLOC(3), EWDN, 0, (struct ptree_dn*)&dn_b[3] },
	{ PTREE_ALLOC(4), EWDN, 0, (struct ptree_dn*)&dn_b[4] },
	{ PTREE_ALLOC(5), EWDN, 0, (struct ptree_dn*)&dn_b[5] },
	{ PTREE_ALLOC(6), EWDN, 0, (struct ptree_dn*)&dn_b[6] },
	{ LN,             EWDN, 0, (struct ptree_dn*)&dn_b[7] }, /* LN */
	{ PTREE_ALLOC(8),   KN, 0,    (struct ptree_kn*)&kn_b },
};

#else
	/* This enum describes wol branch indices */
enum wol_branch_idx {
	/* Ethernet Protocol supported */
	ETHER_PROTO_WOL = 0,
	ETHER_PROTO_ARP,
	ETHER_PROTO_IPV4,
	ETHER_PROTO_FAIL, /* 3 */

	/* IPv4 protocol supported */
	IPV4_PROTO_ICMP,
	IPV4_PROTO_UDP,
	IPV4_PROT0_FAIL,

	/* Standard AMD foramat: Magic packet Signature check */
	MAGIC_PKT_SIG_FIRST_2B,
	MAGIC_PKT_SIG_FIRST_2B_FAIL,
	MAGIC_PKT_SIG_NEXT_2B,
	MAGIC_PKT_SIG_NEXT_2B_FAIL,
	MAGIC_PKT_SIG_LAST_2B,
	MAGIC_PKT_SIG_LAST_2B_FAIL, /* 12 */

	/* Destination mac address check */
	DST_MAC_FIRST_2B,
	DST_MAC_NEXT_2B,
	DST_MAC_LAST_2B,

	/* KN */
	SUCCESS_KN,
};

enum wol_ptree_dn_idx {
	DN_ETHER_PROTO,
	DN_IPV4_PROTO,

	DN_MAGIC_PKT_SIG_FIRST_2B,
	DN_MAGIC_PKT_SIG_NEXT_2B,
	DN_MAGIC_PKT_SIG_LAST_2B,

	DN_DST_MAC_FIRST_2B,
	DN_DST_MAC_NEXT_2B,
	DN_DST_MAC_LAST_2B,

	DN_SUCCESS_LN,
};

enum wol_node_idx {
	N_ETHER_PROTO = 0,
	N_IPV4_PROTO,

	N_MAGIC_PKT_SIG_FIRST_2B,
	N_MAGIC_PKT_SIG_NEXT_2B,
	N_MAGIC_PKT_SIG_LAST_2B,

	N_DST_MAC_FIRST_2B,
	N_DST_MAC_NEXT_2B,
	N_DST_MAC_LAST_2B,

	N_LN,
	N_SUCCESS_KN,
	N_PKT_DROP,
};

static struct ptree_branch branch_b[] = {
	{      0,   0x0842,  EQT, PTREE_ALLOC(N_MAGIC_PKT_SIG_FIRST_2B),
					0, 14, 1, 0, 0 }, /* IPv4 */
	{      0,   0x0806,  EQT, PTREE_ALLOC(N_LN),
					0, 0, 1, 0, 0 }, /* ARP  */
	{      0,   0x0800,  EQT, PTREE_ALLOC(N_IPV4_PROTO),
					0, 22, 0, 0, 0 }, /* IPv4 */
	{0xffff,    0,       EQT, PTREE_ALLOC(N_DST_MAC_FIRST_2B),
					0, 0, 1, 0, 0 }, /* If fail check for dest mac */

	{ 0xff00,   0x0001,  EQT, PTREE_ALLOC(N_LN),
					0, 34, 1, 0, 0 }, /* Allow ICMP */
	{ 0xff00,   0x0011,  EQT, PTREE_ALLOC(N_MAGIC_PKT_SIG_FIRST_2B),
					0, 42, 0, 0, 0 }, /* Allow UDP  */
	{ 0xffff,    0,      EQT, PTREE_ALLOC(N_DST_MAC_FIRST_2B),
					0, 0, 1, 0, 0 }, /* If fail check for dest mac */

	{      0,   0xffff,  EQT, PTREE_ALLOC(N_MAGIC_PKT_SIG_NEXT_2B),
					0, 2, 1, 1, 0 }, /* Compare first 2B */
	{0xffff,    0,       EQT, PTREE_ALLOC(N_DST_MAC_FIRST_2B),
					0, 0, 1, 0, 0 }, /* If fail check for dest mac */
 	{      0,   0xffff,  EQT, PTREE_ALLOC(N_MAGIC_PKT_SIG_LAST_2B),
					0, 2, 1, 1, 0 },/* Next 2B */
	{0xffff,    0,       EQT, PTREE_ALLOC(N_DST_MAC_FIRST_2B),
					0, 0, 1, 0, 0 }, /* If fail check for dest mac */
	{      0,   0xffff,  EQT, PTREE_ALLOC(N_LN),
					0, 2, 1, 1, 0 },  /* Last 2B of signature */
	{0xffff,    0,       EQT, PTREE_ALLOC(N_DST_MAC_FIRST_2B),
					0, 0, 1, 0, 0 }, /* If fail check for dest mac */

	/* Fixme */
	{     0,    0,  EQT, PTREE_ALLOC(N_DST_MAC_NEXT_2B),
					0, 2,  1, 1, 0 }, /* MAC first 2B */
	{     0,    0,  EQT, PTREE_ALLOC(N_DST_MAC_LAST_2B),
					0, 2,  1, 1, 0 }, /* MAC second 2B */
	{     0,    0,  EQT, PTREE_ALLOC(N_LN),
					0, 2,  1, 1, 0 }, /* MAC last 2B */

	{ 0xffff,   0,       EQT, PTREE_ALLOC(N_SUCCESS_KN),
					0, 0,  0, 0, 0 }, /* KN */
};

static struct ptree_dn dn_b[] = {
	{ START_NODE,	DBPTR_DROP(0), 0, 0, 0, 0, 4,
			&branch_b[ETHER_PROTO_WOL] }, /* ARP , IPV4 */
	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 3,
			&branch_b[IPV4_PROTO_ICMP] }, /* ICMP , UDP */

	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 2,
			&branch_b[MAGIC_PKT_SIG_FIRST_2B] }, /* MAGIC PKT SIGNATURE 0 - 1B */
	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 2,
			&branch_b[MAGIC_PKT_SIG_NEXT_2B] }, /* MAGIC PKT SIGNATURE 2 - 3B */
	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 2,
			&branch_b[MAGIC_PKT_SIG_LAST_2B] },  /* MAGIC PKT SIGNATURE 3 - 4B */

	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 1,
			&branch_b[DST_MAC_FIRST_2B] }, /* DST MAC 0 - 1B */
	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 1,
			&branch_b[DST_MAC_NEXT_2B] }, /* DST MAC 1 - 2B */
	{ INTERIM_NODE, DBPTR_DROP(0), 0, 0, 0, 0, 1,
			&branch_b[DST_MAC_LAST_2B] }, /* DST MAC 0 - 1B */

	{ LAST_NODE,    DBPTR_DROP(0), 0, 0, 0, 0, 1,
			&branch_b[SUCCESS_KN] }, /* LN */
};

static struct ptree_node node_b[] = {
	{ PTREE_ALLOC(N_ETHER_PROTO), EWDN, 0,
		(struct ptree_dn*) &dn_b[DN_ETHER_PROTO] }, /* ETH protocols */
	{ PTREE_ALLOC(N_IPV4_PROTO), EWDN, 0,
		(struct ptree_dn*) &dn_b[DN_IPV4_PROTO] }, /* IPV4 protocols */

	{ PTREE_ALLOC(N_MAGIC_PKT_SIG_FIRST_2B), EWDN, 0,
		(struct ptree_dn*) &dn_b[DN_MAGIC_PKT_SIG_FIRST_2B] }, /* MAGIC PKT SIGNATURE CHK 0 -1 B*/
	{ PTREE_ALLOC(N_MAGIC_PKT_SIG_NEXT_2B), EWDN, 0,
		(struct ptree_dn*) &dn_b[DN_MAGIC_PKT_SIG_NEXT_2B] }, /* MAGIC PKT SIGNATURE CHK 1 -2 B */
	{ PTREE_ALLOC(N_MAGIC_PKT_SIG_LAST_2B), EWDN, 0,
		(struct ptree_dn*) &dn_b[DN_MAGIC_PKT_SIG_LAST_2B] }, /* MAGIC PKT SIGNATURE CHK 2 -3 B */

	{ PTREE_ALLOC(N_DST_MAC_FIRST_2B), EWDN, 0,
		(struct ptree_dn*) &dn_b[DN_DST_MAC_FIRST_2B] }, /* DST MAC CHK */
	{ PTREE_ALLOC(N_DST_MAC_NEXT_2B), EWDN, 0,
		(struct ptree_dn*) &dn_b[DN_DST_MAC_NEXT_2B] }, /* DST MAC CHK */
	{ PTREE_ALLOC(N_DST_MAC_LAST_2B), EWDN, 0,
		(struct ptree_dn*) &dn_b[DN_DST_MAC_LAST_2B] }, /* DST MAC CHK */

	{ PTREE_ALLOC(N_LN), EWDN, 0,
		(struct ptree_dn*) &dn_b[DN_SUCCESS_LN] }, /* LN */
	{ PTREE_ALLOC(N_SUCCESS_KN),   KN, 0,
		(struct ptree_kn*) &kn_b }, /* KN */
	{ PTREE_ALLOC(N_PKT_DROP),  KN, 0,
		(struct ptree_kn*) &kn_drop_pkt}, /* KN DROP PKT */
};
#endif

static int apm_cle_wol_opts_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t apm_cle_wol_opts_read(struct file *file, char __user * buf,
		size_t count, loff_t *ppos)
{
	ENET_PRINT("\nAPM WOL OPTIONS\n");

	ENET_PRINT("1.To enable WoL on ARP and ICMP packets :- \n"
		 "echo 1 > apm_cle_wol_opts\n");
	ENET_PRINT("2.To disble WoL on ARP and ICMP packets:- \n"
		" echo 2 > apm_cle_wol_opts\n");

	ENET_PRINT("3.To enable WoL on all packets with a destination to "
		"our MAC address :-\n"
		"echo 3 > apm_cle_wol_opts\n");
	ENET_PRINT("4.To disble WoL on all packets with a destination to"
		" our MAC address :-\n"
		" echo 4 > apm_cle_wol_opts\n");

	return 0;
}

int apm_cle_set_ipp_wol_opts(u16 data)
{
	u32 val = 0;
	return val;
}

static ssize_t apm_cle_wol_opts_write(struct file *file,
		const char __user *buf, size_t count, loff_t *ppos)
{
	int opt = 0;
	char *str;
	int ret;

	str = kzalloc(sizeof(buf), GFP_KERNEL);
        if (!str) {
                ENET_ERROR("%s Failed to allocate memory\n", str);
                return count;
        }

	ret = copy_from_user((void *)str, buf, sizeof(buf));

	ret = sscanf(str, "%d", &opt);

	switch (opt) {
	case 1:
		/* Check if option is already enabled or not */
		if (!((wol_proto_enable & ARP_ICMP_WOL_ENABLE)
			== ARP_ICMP_WOL_ENABLE)) {
			wol_proto_enable |=
				ARP_ICMP_WOL_ENABLE;
			wol_proto_enable &=
				~(ARP_ICMP_REPLY_ENABLE);
			if (apm_cle_set_ipp_wol_opts(
				wol_proto_enable) < 0) {
				ENET_ERROR("Failed to update wol "
					"options\n");
				wol_proto_enable &=
					~(ARP_ICMP_WOL_ENABLE);
				wol_proto_enable |=
					ARP_ICMP_REPLY_ENABLE;
			} else {
				ENET_PRINT("Enabled WoL on RX of ARP and "
						"ICMP packets!!!\n");
				ENET_PRINT("PING reply is disabled now!!\n");
			}
		}
		break;
	case 2:
		if (((wol_proto_enable & ARP_ICMP_WOL_ENABLE)
			== ARP_ICMP_WOL_ENABLE)) {
			wol_proto_enable &= ~(ARP_ICMP_WOL_ENABLE);
			wol_proto_enable |= ARP_ICMP_REPLY_ENABLE;
			if (apm_cle_set_ipp_wol_opts(
				wol_proto_enable) < 0) {
				ENET_ERROR("Failed to update wol "
					"options\n");
				wol_proto_enable |=
					ARP_ICMP_WOL_ENABLE;
				wol_proto_enable &=
					~(ARP_ICMP_REPLY_ENABLE);
			} else {
				ENET_PRINT("Disabled WoL on RX of ARP and "
						"ICMP packets!!!\n");
				ENET_PRINT("PING reply is enabled now!!\n");
			}
		}
		break;
	case 3:
		if (!((wol_proto_enable & MAC_WOL_ENABLE)
					== MAC_WOL_ENABLE)) {
			wol_proto_enable |= MAC_WOL_ENABLE;
			if (apm_cle_set_ipp_wol_opts(
						wol_proto_enable) < 0) {
				ENET_ERROR("Failed to update wol "
						"options\n");
				wol_proto_enable &=
					~(MAC_WOL_ENABLE);
			} else {
				ENET_PRINT("Enabled WoL on any packet with "
					"a destination to our MAC "
					"address!!!\n");
			}
		}
		break;
	case 4:
		if (((wol_proto_enable & MAC_WOL_ENABLE)
			== MAC_WOL_ENABLE)) {
			wol_proto_enable &= ~(MAC_WOL_ENABLE);
			if (apm_cle_set_ipp_wol_opts(
				wol_proto_enable) < 0) {
				ENET_ERROR("Failed to update wol "
					"options\n");
				wol_proto_enable |=
					MAC_WOL_ENABLE;
			} else {
				ENET_PRINT("Disabled WoL on any packet with "
					"a destination to our MAC "
					"address!!!\n");
			}
		}
		break;
	default:
		ENET_ERROR("Not a valid option\n");
	}

	return count;
}

struct file_operations apm_cle_wol_opts_fops = {
	.owner = THIS_MODULE,
	.open = apm_cle_wol_opts_open,
	.read = apm_cle_wol_opts_read,
	.write = apm_cle_wol_opts_write,
};

static void apm_cle_set_mac_compare(u8 port_id)
{
	struct net_device *ndev = find_netdev(port_id);
	u16 *node_data;

	if (!ndev) {
		ENET_ERROR("%s: Failed to get MAC for port %d\n",
				__func__, port_id);
		return;
	}

	node_data = (u16 *) ndev->dev_addr;
	branch_b[DST_MAC_FIRST_2B].data = (u16) *node_data;
	node_data++;
	branch_b[DST_MAC_NEXT_2B].data = (u16) *node_data;
	node_data++;
	branch_b[DST_MAC_LAST_2B].data = (u16) *node_data;
}

int apm_preclass_init_wol_tree(u8 port_id)
{
	int rc = APM_RC_OK;
	struct apm_ptree_config *ptree_config;

	memset(&kn_b, 0, sizeof(kn_b));
	memset(&kn_drop_pkt, 0, sizeof(kn_drop_pkt));

	ptree_config = apm_add_ptree_config(port_id, CLE_PTREE_WOL);
	if (!ptree_config) {
		PCLS_ERR("Add Patricia Tree WOL Configuration error for "
				"port %d\n", port_id);
		return APM_RC_ERROR;
	}

	PCLS_DBG("Create WoL Preclassifier DB entries \n");
	dbptr_b.index = DBPTR_ALLOC(CLE_DB_INDEX);
	dbptr_b.dstqidL = DSLEEP_ENET_RX_Q & 0x7f;
	dbptr_b.dstqidH = (DSLEEP_ENET_RX_Q >> 7) & 0x1f;
	/*dbptr_b.fpsel = DSLEEP_ENET_RX_FP_PBN - 0x20;*/

	kn_b.result_pointer = DBPTR_ALLOC(CLE_DB_INDEX);
	kn_drop_pkt.result_pointer =  DBPTR_DROP(0);

	/* Set mac address to compare */
	apm_cle_set_mac_compare(port_id);

	PCLS_DBG("Create Wol Patricia Tree Nodes \n");
	if ((rc = apm_ptree_alloc(port_id, ARRAY_SIZE(node_b), 1,
				&node_b[0], &dbptr_b, ptree_config))) {
		PCLS_ERR("Preclass init error %d \n", rc);
		return rc;
	}

	ptree_config->start_pkt_ptr = 12;
	if (!proc_create(APM_CLE_WOL_PROC_ENTRY, 0, NULL, &apm_cle_wol_opts_fops))
		printk(KERN_ERR APM_CLE_WOL_PROC_ENTRY "Failed procfs creation\n");

	return rc;
}

int apm_preclass_set_wol_port(u8 port_id, u8 dport_id, u16 dport)
{
	int rc = APM_RC_ERROR;
	struct apm_ptree_config *ptree_config =
		apm_find_ptree_config(port_id, CLE_PTREE_WOL);

	if (!ptree_config) {
		ENET_ERROR("WoL Tree not configured for ethernet port %d\n",
				port_id);
		goto _ret_set_wol_port;
	}

	if (dport_id > 3) {
		ENET_ERROR("WoL Port index %d not supported\n", dport_id);
		goto _ret_set_wol_port;
	}

	branch_b[4+dport_id].data = dport;
	branch_b[4+dport_id].next_node_index = ptree_config->start_node_ptr + 3;
	rc = apm_set_ptree_node_branch(port_id,
			ptree_config->start_node_ptr + 2,
			0, dport_id, &branch_b[4 + dport_id], SET_BRANCH_ALL);

_ret_set_wol_port:
	return rc;
}
#endif
