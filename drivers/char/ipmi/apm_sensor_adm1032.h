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
 * @file apm_sensor_adm1032.h
 *
 * This file define private data for ADM1032 Temperature Controller.

 */



#ifndef  __ADM1032_SENSOR_H__
#define __ADM1032_SENSOR_H__

#include "apm_ipmi_sdr.h"

#define ADM1032_I2C_ADDR	0x4c

#define ADM1032_REG_LOCAL_TEMP	0x00
#define ADM1032_REG_EXT_TEMP_H	0x01	
#define ADM1032_REG_EXT_TEMP_L	0x10
#define ADM1032_REG_STATUS		0x02
#define ADM1032_REG_CONF_READ	0x03
#define ADM1032_REG_LOCAL_TEMP_H_LIMIT_READ	0x5
#define ADM1032_REG_LOCAL_TEMP_H_LIMIT_WRITE	0xB
#define ADM1032_REG_LOCAL_TEMP_L_LIMIT_READ	0x6
#define ADM1032_REG_LOCAL_TEMP_L_LIMIT_WRITE	0xC

#define ADM1032_REG_MID		0xfe
#define ADM1032_REG_PID		0xff


#define ADM1032_CPU_TEMP	"Junction-Temp"
#define ADM1032_ENV_TEMP	"Ambient-Temp"

struct adm1032_sensor {
	int id;
	char *name;
};

#define ADM1032_SENSOR_NUM	2
extern FULL_SENSOR_RECORD adm1032_sdr[ADM1032_SENSOR_NUM];

void adm1032_sdr_init(void);
void adm1032_scanning(void *data);


#endif
