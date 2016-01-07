/*
 * APM IPMI Virtual BMC driver
 *
 * Copyright (c) 2013, Applied Micro Circuits Corporation
 * Author: Van Duc Uy <uvan@apm.com>
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
 * @file apm_sensor_storm.h
 *
 * This file define private data for STORM parameters.

 */


#ifndef  __STORM_SENSOR_H__
#define __STORM_SENSOR_H__

#include "apm_ipmi_sdr.h"

struct storm_sensor {
	int id;
	unsigned char *name;
	int type;
	int unit;
};

#define STORM_SENSOR_NUM	1

extern FULL_SENSOR_RECORD storm_sdr[STORM_SENSOR_NUM];

void storm_sdr_init(void);
void storm_temp_scanning(void *data);


#endif
