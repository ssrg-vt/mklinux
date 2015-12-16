/*
 * AppliedMicro APM88xxxx CLE Engine Configuration Header
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
 * @file apm_cle_config.h
 *
 * This header declares Configuration APIs and macros for
 * AppliedMicro APM88xxxx SoC Classifier module.
 */

#ifndef __APM_CLE_CONFIG_H_
#define __APM_CLE_CONFIG_H_

/* Classifier Configurations for Linux: Enet Port */ 
#define CLE_DB_INDEX		0

/* String ID for CLE ptree_config */
#define CLE_PTREE_ID_SIZE 8
#define CLE_PTREE_DEFAULT "default"
#ifdef CONFIG_CLE_BRIDGE
#define CLE_PTREE_BRIDGE  "bridge"
#endif
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
#define CLE_PTREE_IPV4FWD "ipv4fwd"
#endif
#ifdef CONFIG_APM_ENET_INO
#define CLE_PTREE_IPV4NAT "ipv4nat"
#endif
#ifdef CONFIG_APM_ENET_LRO
#define CLE_PTREE_IPP_LRO "ipp_lro"
#endif
#define CLE_PTREE_WOL "wol"
#define CLE_PTREE_WOL_INT "wol_int"

/**
 * @brief   This function initialize Classifier Engine based on port_id.
 * @param   port_id - Inline-GE/LAC Port number
 * @return  0 - success or -1 - failure
 */
int apm_cle_init(u32 port_id);

/**
 * @brief   This function switch pre classifier configuration specified ptree.
 * @param   port_id - Inline-GE/LAC Port number
 * @param   ptree_id - ptree ID CLE_PTREE_DEFAULT/CLE_PTREE_WOL
 * @param   wol_mode - Enable wol mode
 * @return  0 - success or -1 - failure
 */
int apm_preclass_switch_tree(u8 port_id, char *ptree_id, u8 wol_mode);

/**
 * @brief   This function searches for pre classifier configuration for
 *          specified ptree_id.
 * @param   port_id - Inline-GE/LAC Port number
 * @param   ptree_id - ptree ID of the classifier configuration
 * @return  apm_ptree_config - NULL or the apm_ptree_config found during search
 */
struct apm_ptree_config *apm_find_ptree_config(u8 port_id, char *ptree_id);

/**
 * @brief   This function adds new pre classifier configuration with
 *          specified ptree_id.
 * @param   port_id - Inline-GE/LAC Port number
 * @param   ptree_id - ptree ID of the classifier configuration to be added
 * @return  apm_ptree_config - NULL or the apm_ptree_config added
 */
struct apm_ptree_config *apm_add_ptree_config(u8 port_id, char *ptree_id);

/**
 * @brief   This function deletes new pre classifier configuration with
 *          specified ptree_id.
 * @param   port_id - Inline-GE/LAC Port number
 * @param   ptree_id - ptree ID of the classifier configuration to be deleted
 * @return  0 if deleted or -1 for not found
 */
int apm_del_ptree_config(u8 port_id, char *ptree_id);

#ifdef PTREE_MANAGER
/**
 * @brief   This function searches for system ptree_id configuration for
 *          specified port_id.
 * @param   port_id - Inline-GE/LAC Port number
 * @return  ptree_id of system ptree configuration or NULL
 */
char *apm_get_sys_ptree_id(u8 port);
#endif
#endif /* __APM_CLE_CONFIG_H_ */
