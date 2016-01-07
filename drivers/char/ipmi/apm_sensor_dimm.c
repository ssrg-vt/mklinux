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
 * @file apm_sensor_dimm.c
 *
 * This file do sensor scaning and initialize SDR for DIMM.
 */

#include "apm_sensor_dimm.h"
#include "apm_i2c_access.h"

FULL_SENSOR_RECORD	dimm_sdr[MAX_DIMM_MODULES];
SENSOR_DATA dimm_data[MAX_DIMM_MODULES];

struct dimm_info {
	unsigned char id;
	unsigned char *name;
	unsigned char i2c_addr_spd;
	unsigned char i2c_addr_temp;
};
static struct dimm_info sensor_list[] = {
	{0,"DIMM 0",0x54,0x1C},
	{1,"DIMM 1",0x57,0x1F}
};

#define SPD_THERMAL_AVAILABLE_REG	0x20	/*Byte 32 in SPD*/
#define THERMAL_TEMPERATURE_REG	0x05

void
dimm_scanning (void *data)
{

	SENSOR_DATA *p = (SENSOR_DATA *)data;
	unsigned char thermal_available = 0;
	int thermal_reading=0;
#if 0	
	unsigned char thermal_status = 0;
#endif	
	int read_value=0,ret=0;
	u16 swapped;
	signed char temperature_in_byte;
	

	/*Checking if thermal sensor avalable, by reading SPD register 32*/
	ret = i2c_sensor_read(IIC_1,p->i2c_addr,SPD_THERMAL_AVAILABLE_REG,1,&read_value);
	if (ret) {
		/*printk("%s: Sensor (0x%02x) not available\n",__func__,p->i2c_addr);*/
		p->reading_unavailable = 1;
		return;
	}
	thermal_available = (read_value >> 7) & 0x01;
	/*printk("Thermal Sensor available = %d\n",thermal_available);*/

	if (!thermal_available) {
		/*printk("%s: DIMM %d doesn't have thermal sensor(0x%02x)\n",__func__,
		 			p->private_id,sensor_list[p->private_id].i2c_addr_temp);
		 */
		p->reading_unavailable = 1;
		return;
	}

	if (thermal_available) {
		/*Read Temperature from Thermal Sensor, by reading register 0x05h*/
		ret = i2c_sensor_read(IIC_1,
						sensor_list[p->private_id].i2c_addr_temp,THERMAL_TEMPERATURE_REG,2,&thermal_reading);
		if( ret ) {
			/*
			printk("%s:Temperature sensor(0x%02x) not available\n",__func__,
					sensor_list[p->private_id].i2c_addr_temp);
			*/
			p->reading_unavailable = 1;
			return;
		}
	}

	/*Swap byte because High value was read first*/
	swapped = ((thermal_reading & 0xffff) >> 8) | ((thermal_reading & 0xffff) << 8);
	/*swapped = 0x1C90;*/

	/*
	 * Calculate temperature: we calculate the temperature in byte because
	 * it maybe contain Minus degree C value.
	 * temp = raw_value/16, because resolution is 0.0625. see datasheet*/
	temperature_in_byte = (swapped & 0xfff) >> 4; /**/

	/*In Full sensor Record, we already set B=-55 (0x3c9), 
	 *we have to +55 in sensor reading result, 
	 because if sensor reading is 0, current temperature will be -55 degree C*/
	p->sensor_reading = temperature_in_byte + 55; 
	p->event_message_enable = 0; 
	p->sensor_scanning_enable = 1;
	p->reading_unavailable = 0; 
#if 0	
	p->status = thermal_status; 
#endif	

}

void
dimm_sdr_init()
{
	FULL_SENSOR_RECORD *record;
	IPMI_SDR_HEADER	*header;
	IPMI_SDR_KEY *key;
	IPMI_FULL_SDR_BODY *body;
	int i = 0;
	
	for (i = 0; i< MAX_DIMM_MODULES; i++) {
		record = &dimm_sdr[i];
		
		memset(record,0,sizeof(FULL_SENSOR_RECORD));
		header = &record->header;
		key = &record->key;
		body = &record->body;

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
		body->entity_id = ENTITY_ID_MEMORY; /*See 43-13*/
		body->entity_type = 1; /*0: Physical, 1. Logical*/
		body->entity_instance_num = i; /**/
		
		/*Sensor initialization*/
		body->setable_sensor = 0;
		body->init_scanning = 0;
		body->init_event = 0;
		body->init_threshold = 0;
		body->init_hysteresis = 0;
		body->init_sensor_type= 1;
		body->event_generate = 0;
		body->sensor_scanning = 0;
	
		body->ignore_sensor = 1;
		body->sensor_auto_re_arm_support = 0;
		body->sensor_hysteresis_support = 0;
		body->sensor_threshold_access_support = 1;
		body->sensor_event_message_support = 0;
	
	
		body->sensor_type = SENSOR_TYPE_TEMP; /*See 42-3*/
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
		body->unit2_base_unit = UNIT_TYPE_DEGREE_C; /*Unit type code, degrees C*/
		body->unit3_modifier_unit = 0x0; /*not use*/
	
		body->linearization = SDR_SENSOR_L_LINEAR;
		body->M_ls = 1;
		body->M_ms = 0;
		body->tolerance = 0;
		
		/*We used B values = -55 degree C in this case, because the valid
		 * temperature range is -55 -> 125 degree C. and Y=(Mx + B)10^K
		 * M is always set to 1, mean the resolution is 1 degree C. 
		 * x is current temperature:
		 * if x=0, mean current temperature is -55 degree C,
		 * if x=55, mean the current temperature is 0 degree C,
		 * So at Sensor Reading, the result of x must be + 55*/
		body->B_ls = 0xc9;
		body->B_ms = 0x3;
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
	
		/*Value must be -55 because resolution is 1degree C*/	
		body->unr_threshold = 125; /*70 degree C*/
		body->ucr_threshold = 115;
		body->unc_threshold = 110;
		body->lnr_threshold = 45; /*-10 degree C*/
		body->lcr_threshold = 50;
		body->lnc_threshold = 55; /*0 degree C*/
		
		body->positive_going_threshold_hysteresis_value = 0;
		body->negative_going_threshold_hysteresis_value = 0;
		
		body->id_string_type=3; /*8bits ASCII _ Latin1*/
		body->id_string_len = 12;
		//memcpy((unsigned char*)body->sensor_id_string,DIMM_ID,6);
		//body->sensor_id_string[6] = i;
		sprintf((unsigned char*)body->sensor_id_string,"%s",sensor_list[i].name);
	
		dimm_data[i].sensor_scanning = &dimm_scanning;
		dimm_data[i].i2c_bus = 1;
		dimm_data[i].i2c_addr = sensor_list[i].i2c_addr_spd;
		dimm_data[i].private_id = i;
	
		add_sdr_entry(record,&dimm_data[i]);
	}
}

