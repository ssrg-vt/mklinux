/**
 * AppliedMicro X-Gene Ethernet Common Header
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * All rights reserved. Ravi Patel <rapatel@apm.com>
 *                      Iyappan Subramanian <isubramanian@apm.com>
 *                      Fushen Chen <fchen@apm.com>
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
 * @file apm_xgenet_mac.h
 *
 * This file implements X-Gene Ethernet Common Header
 *
 */
#ifndef __APM_XGENET_MAC_H__
#define __APM_XGENET_MAC_H__

#include "apm_enet_common.h"
#ifdef BOOTLOADER
#include <asm/arch-storm/apm_scu.h>
#endif

#undef APM_XG_AXGMAC_TX2RX_LOOPBACK
#undef APM_XGENET_XGMII_TX2RX_LOOPBACK	
extern void apm_xgenet_init_priv(struct apm_enet_priv *priv, void *port_vaddr,
		void *gbl_vaddr, void *mii_vaddr);
#endif  // __APM_XGENET_MAC_H__
