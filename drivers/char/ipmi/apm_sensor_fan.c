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
 * @file apm_sensor_fan.c
 *
 * This file do sensor scaning and initialize SDR for FAN Controller.
 */


#include "apm_sensor_fan.h"
#include "apm_i2c_access.h"

FULL_SENSOR_RECORD	fan_sdr[FAN_SENSOR_NUM];
SENSOR_DATA fan_data[FAN_SENSOR_NUM];
#define FAN_ID	"FAN %d"
#define FAN_I2C_ADDR	0x2E

static struct fan_sensor sensor_list[] = {
	{0,"FAN 1",0x28,0x29},
	{1,"FAN 2",0x2A,0x2B},
	{2,"FAN 3",0x2C,0x2D},
	{3,"FAN 4",0x2E,0x2F}

};

void
fan_scanning (void *data)
{

	SENSOR_DATA *p = (SENSOR_DATA *)data;
	int decimal=0;
	unsigned char tach_low=0;
	unsigned char tach_high = 0;
	
	tach_low = i2c_sensor_read_byte(IIC_1,p->i2c_addr,sensor_list[p->private_id].reg_l);
	tach_high = i2c_sensor_read_byte(IIC_1,p->i2c_addr,sensor_list[p->private_id].reg_h);

	if ((tach_low == 0xff) && (tach_high == 0xff)) {
		p->reading_unavailable = 1;
		return;
	}	
	decimal = (tach_high << 8) | tach_low;

	if (decimal)	
	p->sensor_reading = (90000)/decimal; /*We have to multiple with 60 to get RPM*/
	p->event_message_enable = 0; 
	p->sensor_scanning_enable = 1;
	p->reading_unavailable = 0;
#if 0	
	p->status = 0x0; 
#endif	

}

void
fan_sdr_init()
{
	IPMI_SDR_HEADER	*header ;
	IPMI_SDR_KEY *key;
	IPMI_FULL_SDR_BODY *body;
	FULL_SENSOR_RECORD *record;
	int i;

	for (i=0;i<FAN_SENSOR_NUM;i++) {

		record = &fan_sdr[i];
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
		body->entity_id = ENTITY_ID_FAN; /*See 43-13*/
		body->entity_type = 1; /*0: Physical, 1. Logical*/
		body->entity_instance_num = i; /**/
		
		/*Sensor initialization*/
		body->setable_sensor = 0;
		body->init_scanning = 0;
		body->init_event = 0;
		body->init_threshold = 1;
		body->init_hysteresis = 1;
		body->init_sensor_type= 1;
		body->event_generate = 0;
		body->sensor_scanning = 0;
	
		body->ignore_sensor = 1;
		body->sensor_auto_re_arm_support = 0;
		body->sensor_hysteresis_support = 0;
		body->sensor_threshold_access_support = 1;
		body->sensor_event_message_support = 0;
	
	
		body->sensor_type = SENSOR_TYPE_FAN; /*See 42-3*/
		body->event_reading_type_code = SENSOR_READING_TYPE_THRESHOLD;/* See 42-1*/
		
		body->event_threshold_mask_low[0] = 0xff;
		body->event_threshold_mask_low[1] = 0xfe;
		body->event_threshold_mask_high[0] = 0xff;
		body->event_threshold_mask_high[1] = 0xfe;
		body->setable_mask = 0x3f;
		body->readable_mask = 0x3f;
		
		body->unit1_analog_data_format = 0; /*unsigned*/
		body->unit1_rate_unit = 0;/*none*/
		body->unit1_modifier_unit = 0; /*none*/
		body->unit1_percentage = 0;/*not percentage*/
		body->unit2_base_unit = UNIT_TYPE_RPM; /*Unit type code, degrees C*/
		body->unit3_modifier_unit = 0x0; /*not use*/
	
		body->linearization = SDR_SENSOR_L_LINEAR;
		body->M_ls = 60; /*Result of sensor_reading will multible by 60*/
		body->M_ms = 0;
		body->tolerance = 0;
		
		body->B_ls = 0x00;
		body->B_ms = 0;
		body->accuracy_ls = 0;
		
		body->accuracy_ms = 0x0;
		body->accuracy_exp = 0x0;
		body->sensor_direction = 0x1; /*input*/
		
		body->R_exp = 0x0;
		body->B_exp = 0x0;
		
		body->analog_characteristic_flags = 0x00;
		
		body->nominal_reading = 0; 
		body->normal_max = 0; 
		body->normal_min = 0;
		
		body->sensor_max_reading = 0xff; /*256*/
		body->sensor_min_reading = 0x0;
		
		body->unr_threshold = 160; /*9600 RPM: too fast*/
		body->ucr_threshold = 150;
		body->unc_threshold = 140;
		body->lnr_threshold = 70; /*4200 RPM: too slow*/
		body->lcr_threshold = 80;
		body->lnc_threshold = 90;
		
		body->positive_going_threshold_hysteresis_value = 0;
		body->negative_going_threshold_hysteresis_value = 0;
		
		body->id_string_type=3; /*8bits ASCII _ Latin1*/
		body->id_string_len = strlen(sensor_list[i].name);
		sprintf((unsigned char*)body->sensor_id_string,FAN_ID,i);
	
		fan_data[i].sensor_scanning = &fan_scanning;
		fan_data[i].private_id = i;
		fan_data[i].i2c_addr = FAN_I2C_ADDR;
	
		add_sdr_entry(record,&fan_data[i]);
	}
}

