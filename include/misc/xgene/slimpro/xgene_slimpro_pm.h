/*
 * xgene_slimpro.h - AppliedMicro X-Gene SlimPRO Driver
 *
 * Copyright (c) 2013, Applied Micro Circuits Corporation
 * Author: Loc Ho <lho@apm.com>
 * Author: Narinder Dhillon <ndhillon@apm.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * This module provides interface to SlimPRO which controls power and clock.
 *
 */
 
#ifndef __XGENE_SLIMPRO_PM_H__
#define __XGENE_SLIMPRO_PM_H__

int slimpro_pm_get_tpc_status(u32 *data0, u32 *data1);

enum xgene_slimpro_pm_event_type {
	SLIMPRO_PM_EVENT_UNKNOWN,
	SLIMPRO_PM_EVENT_TPC,
};

struct slimpro_pm_async_evt {
	u32 msg;
	u32 data0;
	u32 data1;
};

#endif

