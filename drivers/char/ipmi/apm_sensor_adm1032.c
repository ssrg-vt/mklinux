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
 * @file apm_sensor_adm1032.c
 *
 * This file do sensor scaning and initialize SDR for ADM1032 
 * Temperature Controller.
 */


#include "apm_sensor_adm1032.h"
#include "apm_i2c_access.h"

FULL_SENSOR_RECORD	adm1032_sdr[ADM1032_SENSOR_NUM];
SENSOR_DATA adm1032_data[ADM1032_SENSOR_NUM];

static struct adm1032_sensor sensor_list[]={
	{0,ADM1032_CPU_TEMP},
	{1,ADM1032_ENV_TEMP}
};

#define M_RATIO 100 /*Multiply ratio to convert to interger*/

#define ADM_STATUS_ADM_BUSY	0x80
#define ADM_STATUS_LHIGH	0x40
#define ADM_STATUS_LLOW		0x20
#define ADM_STATUS_RHIGH	0x10
#define ADM_STATUS_RLOW		0x08
#define ADM_STATUS_OPEN		0x04
#define ADM_STATUS_RTHRM	0x02
#define ADM_STATUS_LTHRM	0x01

void
adm1032_scanning (void *data)
{

	SENSOR_DATA *p = (SENSOR_DATA *)data;
	u32 remote_temp_high=0,remote_temp_low = 0;
	u32 local_temp = 0;
	u32 remote_temp=0;
	u8  status;
	 
	switch(p->private_id)
	{
		case 0: /*CPU_TEMP*/
			remote_temp_high = i2c_sensor_read_byte(IIC_1,ADM1032_I2C_ADDR,ADM1032_REG_EXT_TEMP_H);
			remote_temp_low = i2c_sensor_read_byte(IIC_1,ADM1032_I2C_ADDR,ADM1032_REG_EXT_TEMP_L);
			remote_temp = remote_temp_high * M_RATIO + ((remote_temp_low*M_RATIO) >> 8);
			status = i2c_sensor_read_byte(IIC_1,ADM1032_I2C_ADDR,ADM1032_REG_STATUS);
/*
			printk("%s: r_h=0x%x,r_l=0x%x, remote_temp=%d\n",__func__,remote_temp_high,remote_temp_low,remote_temp);
*/
			p->sensor_reading = (remote_temp + 10*M_RATIO)/50; /*Must be plus to 10 degree C because Full SDR already -10C*/
			p->event_message_enable = 0; /*Enable Sensor Scaning and Event Message*/
			p->sensor_scanning_enable = 1;
			p->reading_unavailable = 0;
#if 0			
			if (status & ADM_STATUS_RLOW)
				p->status |= LCR_THRESHOLD_STATUS; /*Lower Critical Threshold*/
			if (status & ADM_STATUS_RHIGH)
				p->status |= UCR_THRESHOLD_STATUS; /*Upper Critical Threshold*/
#endif			
		break;
		case 1:	
			local_temp = i2c_sensor_read_byte(IIC_1,ADM1032_I2C_ADDR,ADM1032_REG_LOCAL_TEMP)*M_RATIO;
			status = i2c_sensor_read_byte(IIC_1,ADM1032_I2C_ADDR,ADM1032_REG_STATUS);
			p->sensor_reading = (local_temp + 10*M_RATIO)/50; 
			p->event_message_enable = 0; /*Enable Sensor Scaning and Event Message*/
			p->sensor_scanning_enable = 1;
			p->reading_unavailable = 0;
#if 0			
			if (status & ADM_STATUS_LLOW)
				p->status |= LCR_THRESHOLD_STATUS; /*Lower Critical Threshold*/
			if (status & ADM_STATUS_LHIGH)
				p->status |= UCR_THRESHOLD_STATUS; /*Upper Critical Threshold*/
#endif			
		break;	
		default:
			p->reading_unavailable = 1;
			break;
	}

}
#if 0
static void
adm1032_get_device_id(void)
{
	unsigned char mid,pid;

	mid = i2c_sensor_read_byte(1,ADM1032_I2C_ADDR,ADM1032_REG_MID);
	pid = i2c_sensor_read_byte(1,ADM1032_I2C_ADDR,ADM1032_REG_PID);
	printk("ADM1032 Device ID:MID=0x%02x, PID=0x%02x\n",mid,pid);
}
#endif
void
adm1032_sdr_init()
{
	IPMI_SDR_HEADER	*header ;
	IPMI_SDR_KEY *key;
	IPMI_FULL_SDR_BODY *body;
	FULL_SENSOR_RECORD *record;
	int i;

	for (i=0;i<ADM1032_SENSOR_NUM;i++) {

		record = &adm1032_sdr[i];
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
		body->entity_id = ENTITY_ID_SYSTEM_BOARD; /*See 43-13*/
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
		body->M_ls = 50; /*M=50 => accuracy=0.5 degree C*/ 
		body->M_ms = 0;
		body->tolerance = 0;
		
		/*B=-10 degree C*/
		body->B_ls = 0xF6;
		body->B_ms = 0x3;
		body->accuracy_ls = 0;
		
		body->accuracy_ms = 0x0;
		body->accuracy_exp = 0x0;
		body->sensor_direction = 0x1; /*input*/
		
		body->R_exp = -2; /*K2=2 => Y=(Mx + B)*10^-2*/
		body->B_exp = 2; /*K1=2 => B= 65*10^2*/
		
		body->analog_characteristic_flags = 0x00;
		
		body->nominal_reading = 0; 
		body->normal_max = 0; 
		body->normal_min = 0;
		
		body->sensor_max_reading = 0xff; /*256*/
		body->sensor_min_reading = 0x0;

	    /*Calculate Preset Threshold value for this sensor
		 * Note: Y= (M*X +B*10^K1)*10^K2
		 * Y: Value want to set in degree C
		 * K1=2, K2=-2, M=50, B=-10
		 * => X = (Y+B)*100/50=(Y+B)*2*/	

		/*Ex: 60 degree C = (60+10)*2=140*/
		body->unr_threshold = 180; /*80 degree C*/
		body->ucr_threshold = 160; /*70 degree C*/
		body->unc_threshold = 150; /*65 degree C*/
		body->lnc_threshold = 20;  /*0 degree C*/
		body->lcr_threshold = 10; /*-5 degree C*/
		body->lnr_threshold = 0; /*-10 degree C*/
		
		body->positive_going_threshold_hysteresis_value = 0;
		body->negative_going_threshold_hysteresis_value = 0;
		
		body->id_string_type=3; /*8bits ASCII _ Latin1*/
		body->id_string_len = strlen(sensor_list[i].name);
		memcpy((unsigned char*)body->sensor_id_string,sensor_list[i].name,strlen(sensor_list[i].name));
	
		adm1032_data[i].sensor_scanning = &adm1032_scanning;
		adm1032_data[i].i2c_bus = 1;
		adm1032_data[i].i2c_addr = ADM1032_I2C_ADDR;
		adm1032_data[i].private_id = i;
	
		add_sdr_entry(&adm1032_sdr[i],&adm1032_data[i]);
	}
	//adm1032_get_device_id();
}

