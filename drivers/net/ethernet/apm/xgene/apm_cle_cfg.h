/*
 * AppliedMicro APM88xxxx Ethernet CLE Configuration Header
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
 * @file apm_cle_cfg.h
 *
 * This header file declares APIs and Macros in use by Linux
 * APM88xxxx SoC Inline Classifier
 */

#ifndef __APM_CLE_CFG_H__
#define __APM_CLE_CFG_H__

#include "apm_enet_access.h"
#include <misc/xgene/cle/apm_cle_config.h>

enum apm_macaddr_type {
	TYPE_USR_MACADDR = 0,
	TYPE_SYS_MACADDR,
};

/* Reserved System MAC addresses to allow */
enum apm_sys_macaddr_index {
	ETHERNET_MACADDR,	/* EthernetN MACAddr */
	BROADCAST_MACADDR,	/* FF:FF:FF:FF:FF:FF */
	UNICAST_MACADDR,	/* xu:xx:xx:xx:xx:xx */ /* where u & 1 = 0 */
	MULTICAST_MACADDR,	/* xm:xx:xx:xx:xx:xx */ /* where m & 1 = 1 */
	APM_SYS_MACADDR,
};

extern const u8 apm_usr_macmask[ETH_ALEN + 2];
extern const u8 apm_sys_macmask[APM_SYS_MACADDR][ETH_ALEN + 2];
extern const u8 apm_sys_macaddr[APM_SYS_MACADDR][ETH_ALEN + 2];

/*
 * Functions for Classifier configurations in use by Ethernet driver
 */

/**
 * @brief   This function performs preclassifier engine Initialization
 *          and node configurations.
 * @param   port_id - GE Port number
 * @param   *eth_q  - Pointer to queue configurations used by Ethernet
 * @return  0 - success or -1 - failure
 */
int apm_preclass_init(u8 port_id, struct eth_queue_ids *eth_q);

/**
 * @brief   This function updates (add or delete) preclassifier rule of allowing incoming
 *          Ethernet packets dstmac address which are same as macaddr
 * @param   port_id - GE Port number
 * @param   type - TYPE_USR_MACADDR or TYPE_SYS_MACADDR
 * @param   index - Index of MAC Address
 * @param   macmask - MAC Address Mask
 * @param   macaddr - MAC Address (If macaddr is NULL then delete else add)
 * @return  0 - success or -1 - failure
 */
int apm_preclass_update_mac(u8 port_id, enum apm_macaddr_type type,
			u8 index, const u8 *macmask, const u8 *macaddr);

/**
 * @brief   This function initializes pre classifier nodes for Wake On Lan
 *          tree configuration
 * @param   port_id - GE Port number (0)
 * @return  0 - success or -1 - failure
 */
int apm_preclass_init_wol_tree(u8 port_id);

/**
 * @brief   This function set Wake On Lan DST UDP Port number
 * @param   port_id - GE Port number (0)
 * @param   dport_id - WoL Port Index (0 to 3) supports WoL on 4 ports
 * @param   dport - DST UDP Port number
 * @return  0 - success or -1 - failure
 */
int apm_preclass_set_wol_port(u8 port_id, u8 dport_id, u16 dport);

/**
 * @brief   This function initializes pre classifier nodes for Wake On Lan
 *          Intermediate tree configuration
 * @param   port_id - GE Port number (0)
 * @return  0 - success or -1 - failure
 */
int apm_preclass_init_wol_inter_tree(u8 port_id);

#endif /* __APM_CLE_CFG_H__ */
