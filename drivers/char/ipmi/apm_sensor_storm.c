/*
 * APM IPMI Virtual BMC Driver
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
 * @file apm_sensor_storm.c
 *
 * This file do sensor scaning and initialize SDR for STORM.
 */


#include <asm/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <misc/xgene/slimpro/xgene_slimpro_pm.h>

#include "apm_i2c_access.h"
#include "apm_sensor_storm.h"

FULL_SENSOR_RECORD	storm_sdr[STORM_SENSOR_NUM];
SENSOR_DATA storm_sensor_data[STORM_SENSOR_NUM];

#define STORM_TEMP	"Processor-Temp"
#define STORM_PWR	"Processor-PWR"

struct storm_sensor sensor_list[] = {
	{0,STORM_TEMP,SENSOR_TYPE_TEMP,UNIT_TYPE_DEGREE_C},
	{0,STORM_PWR,SENSOR_TYPE_VOLT,UNIT_TYPE_VOLT},
};


void
storm_temp_scanning (void *data)
{
	SENSOR_DATA *p = (SENSOR_DATA *)data;
	int ret,data0=0,data1=0,tm1,tm2,status,avg_temp,high_temp;
	
	switch (p->private_id)
	{
		case 0: /*STORM Temperature*/
			ret = slimpro_pm_get_tpc_status(&data0,&data1);
			tm2 = ((data1 >> 16) & 0xffff) ;
			tm1 = ((data1 >> 24) & 0xffff) ;

			high_temp = (data0 & 0xff)  ;
			avg_temp = ((data0 >> 8) & 0xff);
			status = (data0 >> 16) & 0xff; //FIXME: Status from Slimpro not matchable with IPMI defined

			//printk("%s: temp=0x%x, tm1=0x%x,tm2=0x%x,high_temp=0x%x,avg_temp=0x%x,status=0x%x\n",
			//		__func__,temp,tm1,tm2,high_temp,avg_temp,status);
			//
			/*Current temperature*/
			p->sensor_reading = high_temp; 

			p->event_message_enable = 0; /*Enable Sensor Scaning and Event Message*/
			p->sensor_scanning_enable = 1;
			p->reading_unavailable = 0;
#if 0
			/*Threshold status*/
			p->status = status; //FIXME: see Get Sensor Reading on IPMI docs
#endif
			break;
		case 1: /*Storm Power status*/
			p->reading_unavailable  = 1;
			break;
		default:
			break;
	}
}

void
storm_sdr_init()
{
	IPMI_SDR_HEADER	*header ;
	IPMI_SDR_KEY *key;
	IPMI_FULL_SDR_BODY *body;
	FULL_SENSOR_RECORD *record;
	int i;

	for (i=0;i<STORM_SENSOR_NUM;i++) {

		record = &storm_sdr[i];
		memset(record,0,sizeof(FULL_SENSOR_RECORD));
		header = &record->header;
		key = &record->key;
		body = &record->body;

		memset(record,0,sizeof(FULL_SENSOR_RECORD));

		header->record_id[0] = 0x0;
		header->record_id[1] = 0x0;
		header->sdr_version = 0x51;
		header->record_type = SDR_TYPE_FULL; /**/
		header->record_len = sizeof(FULL_SENSOR_RECORD);

		/*KEY*/
		key->owner_id = 0x0; /*7bit addr */
		key->owner_lun = 0x0;
		key->sensor_number = 0; /*will replace when add_sdr_entry*/
	
		/*Body*/
		body->entity_id = ENTITY_ID_PROCESSOR; /*See 43-13*/
		body->entity_type = 1; /*0: Physical, 1. Logical*/
		body->entity_instance_num=i; /**/
		
		/*Sensor initialization*/
		body->setable_sensor = 0;
		body->init_scanning = 0;
		body->init_event = 0;
		body->init_threshold = 1;
		body->init_hysteresis = 1;
		body->init_sensor_type= 1;
		body->event_generate = 0;
		body->sensor_scanning = 0;
		
		/*Sensor Capabilities*/
		body->ignore_sensor = 1; /*Don't ignore sensor*/
		body->sensor_auto_re_arm_support = 0; /*No, manual*/
		body->sensor_hysteresis_support = 0; /*No Hysteresis*/
		body->sensor_threshold_access_support = 1; /*Threshold Redable per readble Mask*/
		body->sensor_event_message_support = 3; /*No Event from sensor*/
	
	
		body->sensor_type = sensor_list[i].type; /*See 42-3*/
		body->event_reading_type_code = SENSOR_READING_TYPE_THRESHOLD;/* See 42-1*/
		
		body->event_threshold_mask_low[0] = 0x7f;
		body->event_threshold_mask_low[1] = 0xff;
		body->event_threshold_mask_high[0] = 0x7f;
		body->event_threshold_mask_high[1] = 0xff;
		body->setable_mask = 0x3f;
		body->readable_mask = 0x3f;
		
		body->unit1_analog_data_format = 0; /*unsigned*/
		body->unit1_rate_unit = 0;/*none*/
		body->unit1_modifier_unit = 0; /*none*/
		body->unit1_percentage = 0;/*not percentage*/
		body->unit2_base_unit = sensor_list[i].unit; /*Unit type code, degrees C*/
		body->unit3_modifier_unit = 0x0; /*not use*/
	
		body->linearization = SDR_SENSOR_L_LINEAR;
		body->M_ls = 1;
		body->M_ms = 0;
		body->tolerance = 0;
		
		body->B_ls = 0;
		body->B_ms = 0;
		body->accuracy_ls = 0;
		
		body->accuracy_ms = 0x0;
		body->accuracy_exp = 0x0;
		body->sensor_direction = 0x1; /*input*/
		
		body->R_exp = 0;
		body->B_exp = 0;
		
		body->analog_characteristic_flags = 0x00;
		
		body->nominal_reading = 0; 
		body->normal_max = 0; 
		body->normal_min = 0;
		
		body->sensor_max_reading = 0xff; /*256*/
		body->sensor_min_reading = 0x0;
		
		body->unr_threshold = 80;
		body->ucr_threshold = 67;
		body->unc_threshold = 65;
		body->lnr_threshold = 0;
		body->lcr_threshold = 0;
		body->lnc_threshold = 0;
		
		body->positive_going_threshold_hysteresis_value = 0;
		body->negative_going_threshold_hysteresis_value = 0;
		
		body->id_string_type=3; /*8bits ASCII _ Latin1*/
		body->id_string_len = strlen(sensor_list[i].name);
		memcpy((unsigned char*)body->sensor_id_string,sensor_list[i].name,strlen(sensor_list[i].name));
	
		storm_sensor_data[i].sensor_scanning = &storm_temp_scanning;
		storm_sensor_data[i].private_id = i;
	
		add_sdr_entry(&storm_sdr[i],&storm_sensor_data[i]);
	}
}

