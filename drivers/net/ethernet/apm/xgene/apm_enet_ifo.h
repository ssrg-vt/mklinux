/**
 * AppliedMicro X-Gene SoC Ethernet IPv4 Forward Offload Header
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * All rights reserved. Hrishikesh Karanjikar <hkaranjikar@apm.com>
 *                      Khuong Dinh <kdinh@apm.com>
 *                      Ravi Patel <rapatel@apm.com>
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
 * @file apm_enet_ifo.h
 *
 * This header file declares APIs and Macros for X-Gene SoC Ethernet
 * IPv4 Forward Offload configuration
 *
 */

#ifndef __APM_ENET_IFO_H__
#define __APM_ENET_IFO_H__

#include "apm_enet_access.h"

/*
 * @brief   This function performs the actual offloading.
 * @return  0 on Success.
 *
 */
int apm_enet_ifo_perform(struct xgene_qmtm_msg32 *msg32_1,
		struct sk_buff *skb, struct apm_enet_qcontext *c2e);
#endif /* __APM_ENET_IFO_H__ */
