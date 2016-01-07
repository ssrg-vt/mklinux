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
 * @file apm_sensor_vrm.c
 *
 * This file do sensor scaning and initialize SDR for VRM Controller.
 */

#include "apm_sensor_vrm.h"
#include "apm_i2c_access.h"


#define VRM_0V95_SENSOR_NUM	2
#define VRM_PWR_SENSOR_NUM	2
#define VRM_1V5_SENSOR_NUM 	1

FULL_SENSOR_RECORD	vrm_0v95_sdr[VRM_0V95_SENSOR_NUM];
SENSOR_DATA vrm_0v95_data[VRM_0V95_SENSOR_NUM];

FULL_SENSOR_RECORD	pwr_sdr[VRM_PWR_SENSOR_NUM];
SENSOR_DATA pwr_data[VRM_PWR_SENSOR_NUM];

FULL_SENSOR_RECORD	vrm_1v5_sdr[VRM_1V5_SENSOR_NUM];
SENSOR_DATA vrm_1v5_data[VRM_1V5_SENSOR_NUM];

static struct vrm_sensor vrm_0v95_sensor_list[] = {
	{0,"Processor_Volt",0x40,0,0x8B,UNIT_TYPE_VOLT},
	{1,"PMD_Volt",0x45,0,0x8B,UNIT_TYPE_VOLT}
};
static struct vrm_sensor pwr_sensor_list[] = {
	{0,"Processor_PWR",0x40,0,0x96,UNIT_TYPE_WATT},
	{1,"PMD_PWR",0x46,0x47,0x96,UNIT_TYPE_WATT}
};
static struct vrm_sensor vrm_1v5_sensor_list[] = {
	{0,"DIMM_Volt",0x24,0,0x8B,UNIT_TYPE_VOLT}
};

#define VOUT_MODE_REG	         0x20
#define VOUT_COMMAND_REG		 0x21
#define VOUT_MAX_REG			 0x24
#define VOUT_MARGIN_HIGH_REG	 0x25
#define VOUT_MARGIN_LOW_REG		 0x26
#define VOUT_TRANSITION_RATE_REG 0x27
#define READ_VOUT_REG			 0x8B
#define STATUS_BYTE_REG			 0x78
#define STATUS_VOUT_REG			 0x7A
#define READ_POUT_REG			 0x96

#define VOUT_MIN_VALUE			 0x0E74	/*3700 = 0.9V*/
#define VOUT_MAX_VALUE			 0x1000	/*4096 = 1V*/

void
vrm_0v95_scanning (void *data)
{

	SENSOR_DATA *p = (SENSOR_DATA *)data;
	int vout=0;
	int ret,read_val=0;
	unsigned char status=0;
	
	ret = i2c_sensor_read(IIC_1,p->i2c_addr,vrm_0v95_sensor_list[p->private_id].reg,2,&read_val);
	if (ret == 0xffffffff) {
		p->reading_unavailable = 1;
		return;
	}	

	/*Calculate VOUT step*/
	/*Vout = read_val*2^N=read_val*2^-12 = read_val/2^12 = read_val >> 12*/
	/*1 Volt = 1000mV  = 250*4mV*/
	/*Y = L(M*x + B*10K1)*10K2*/
	/*M: 4mV*/
	/*B: 0*/
	/*K1: 0*/
	/*K2: -3, Since output Unit is Volt, not milivolt*/
	vout = (int)((250*read_val ) >> 12 );

	read_val=0;
	ret = i2c_sensor_read(IIC_1,p->i2c_addr,STATUS_VOUT_REG,1,&read_val);
	status = read_val & 0xff;

#if 0
	printk("%s:vout_raw=0x%04x,V-step:%d \n",__func__,read_val,vout);

#endif	

	p->sensor_reading = vout; 
	p->event_message_enable = 0 ; 
	p->sensor_scanning_enable = 1;
	p->reading_unavailable = 0;

}

void
vrm_power_scanning (void *data)
{

	SENSOR_DATA *p = (SENSOR_DATA *)data;
	int pout=0;
	int ret,read_val=0;
	int N,Y;
	
	ret = i2c_sensor_read(IIC_1,p->i2c_addr,pwr_sensor_list[p->private_id].reg,2,&read_val);
	if (ret == 0xffffffff) {
		p->reading_unavailable = 1;
		return;
	}	

	/*POUT Calculate*/
	/* POUT = Y*2^N
	 * N=b[15:11] is 5bits 2's complement interger.
	 * Y=b[10:0] is 11 bit 2's complement interger.
	 */

	N = 32 - ((read_val >> 11) & 0x1F);
	Y= read_val & 0x7ff;

	/*Multiple Power out with 20, to calculate Power in mW, with resolution is 50mW*
	 * After get pout, we must be multiply with 50mW to get pout in mW, 1W=1000mW*/
	pout = (int)((2*Y >> N));

	if (pwr_sensor_list[p->private_id].i2c_addr_loop2) {
		ret = i2c_sensor_read(IIC_1,pwr_sensor_list[p->private_id].i2c_addr_loop2,pwr_sensor_list[p->private_id].reg,2,&read_val);
		if (ret == 0xffffffff) {
			p->reading_unavailable = 1;
			return;
		}
		N = 32 - ((read_val >> 11) & 0x1F);
		Y= read_val & 0x7ff;

		/*Multiple Power out with 20, to calculate Power in mW, with resolution is 50mW*
		 * After get pout, we must be multiply with 50mW to get pout in mW, 1W=1000mW*/
		pout += (int)((2*Y >> N));
	}

#if 0 
	printk("%s:Y=%d, N=%d,pout=%d \n",__func__,Y,N,pout);
#endif
	p->sensor_reading = pout;
	p->event_message_enable = 0 ; 
	p->sensor_scanning_enable = 1;
	p->reading_unavailable = 0;

}

void vrm_1v5_scanning (void *data)
{

	SENSOR_DATA *p = (SENSOR_DATA *)data;
	int ret,read_val=0;
	int vout = 0;
	int N;
	ret = i2c_sensor_read(IIC_1,p->i2c_addr,VOUT_MODE_REG,1,&read_val);
	if (ret == 0xffffffff) {
		p->reading_unavailable = 1;
		return;
	}	
	
	N = 32 - (read_val & 0x1F); /*Get 2 complement of 5 bits*/

	read_val = 0;
	ret = i2c_sensor_read(IIC_1,p->i2c_addr,READ_VOUT_REG,2,&read_val);
	if (ret == 0xffffffff) {
		p->reading_unavailable = 1;
		return;
	}	

	/*VOUT Calculate: PMBUS Specification part II: 8.2.1*/
	/* VOUT = Y*2^N = Y/2^N = Y/(1<<N) = Y >> N
	 *
	 * Y=READ_VOUT
	 * N=5 bits 2 complement of VOUT_MODE
	 *
	 *
	 * N=13 => Y=8192 for 1 Volt,=> VOUT=Y/8192
	 * */

	/*We will multiple Vout with 20 for more accuracy, because we have to cast from
	 * Float to interger value, if not multiple the value will ROUND DOWN to nearest value*/	
	vout = (int)((20*read_val) >> N); 
	/*Multiple for 20, it mean that we have to the accuracy will be 1000mV/20=50mV*/

#if 0
	printk("%s:Y=%d,N=%d,V:%d\n",__func__,read_val,N,vout);
#endif	

	p->sensor_reading = vout; 
	p->event_message_enable = 0 ; 
	p->sensor_scanning_enable = 1;
	p->reading_unavailable = 0;

}

void vrm_0v95_sdr_init(void)
{
	IPMI_SDR_HEADER	*header;
	IPMI_SDR_KEY *key;
	IPMI_FULL_SDR_BODY *body;
	FULL_SENSOR_RECORD *record;
	int i;

	for (i=0;i<VRM_0V95_SENSOR_NUM;i++) {

		record = &vrm_0v95_sdr[i];
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
		body->entity_id = ENTITY_ID_POWER_MODULE; /*See 43-13*/
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
	
	
		body->sensor_type = SENSOR_TYPE_VOLT; /*See 42-3*/
		body->event_reading_type_code = 1;/*Unspecified:  See 42-1*/
		
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
		body->unit2_base_unit = vrm_0v95_sensor_list[i].unit; /*Unit type code, Volt*/
		body->unit3_modifier_unit = 0; /*not use*/
	
		
		body->analog_characteristic_flags = 0x00;
		
		body->nominal_reading = 0; 
		body->normal_max = 0; 
		body->normal_min = 0;
		
		body->sensor_max_reading = 0xff; /*256*/
		body->sensor_min_reading = 0x0;
		
	
		body->positive_going_threshold_hysteresis_value = 0;
		body->negative_going_threshold_hysteresis_value = 0;
		
		body->id_string_type=3; /*8bits ASCII _ Latin1*/
		body->id_string_len = 15;
		sprintf((unsigned char*)body->sensor_id_string,"%s",vrm_0v95_sensor_list[i].name);

		body->linearization = SDR_SENSOR_L_LINEAR;
		body->M_ls = 4; /*4mV resolution*/
		body->M_ms = 0;
		body->tolerance = 0;
	
		body->B_ls = 0;
		body->B_ms = 0;
		body->accuracy_ls = 0;
	
		body->accuracy_ms = 0x0;
		body->accuracy_exp = 0x0;
		body->sensor_direction = 0x1; /*input*/
	
		body->R_exp = -3; /*Convert from milivolt to Volt*/
		body->B_exp = 0;

		vrm_0v95_data[i].sensor_scanning = &vrm_0v95_scanning;
		
		/*X=Y/4*10^3*/
		body->unr_threshold = 255;/*1.02V*/
		body->ucr_threshold = 250;/*1.0V*/
		body->unc_threshold = 245;/*0.98V*/
		body->lnr_threshold = 220; /*0.90V*/
		body->lcr_threshold = 225; /*0.92V*/
		body->lnc_threshold = 230; /*0.93V*/

		vrm_0v95_data[i].private_id = i;
		vrm_0v95_data[i].i2c_addr = vrm_0v95_sensor_list[i].i2c_addr;		
	
		add_sdr_entry(record,&vrm_0v95_data[i]);
	}
}
void vrm_power_sdr_init(void)
{
	IPMI_SDR_HEADER	*header ;
	IPMI_SDR_KEY *key;
	IPMI_FULL_SDR_BODY *body;
	FULL_SENSOR_RECORD *record;
	int i;

	for (i=0;i<VRM_PWR_SENSOR_NUM;i++) {

		record = &pwr_sdr[i];
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
		body->entity_id = ENTITY_ID_POWER_MODULE; /*See 43-13*/
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
	
	
		body->sensor_type = SENSOR_TYPE_VOLT; /*See 42-3*/
		body->event_reading_type_code = 1;/*Unspecified:  See 42-1*/
		
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
		body->unit2_base_unit = pwr_sensor_list[i].unit; /*Unit type code, Volt*/
		body->unit3_modifier_unit = 0; /*not use*/
	
		
		body->analog_characteristic_flags = 0x00;
		
		body->nominal_reading = 0; 
		body->normal_max = 0; 
		body->normal_min = 0;
		
		body->sensor_max_reading = 0xff; /*256*/
		body->sensor_min_reading = 0x0;
		
	
		body->positive_going_threshold_hysteresis_value = 0;
		body->negative_going_threshold_hysteresis_value = 0;
		
		body->id_string_type=3; /*8bits ASCII _ Latin1*/
		body->id_string_len = 15;
		sprintf((unsigned char*)body->sensor_id_string,"%s",pwr_sensor_list[i].name);

		body->linearization = SDR_SENSOR_L_LINEAR;
		body->M_ls = 50; /*50mW resolution */
		body->M_ms = 0;
		body->tolerance = 0;
	
		body->B_ls = 0;
		body->B_ms = 0;
		body->accuracy_ls = 0;
	
		body->accuracy_ms = 0x0;
		body->accuracy_exp = 0x0;
		body->sensor_direction = 0x1; /*input*/
	
		body->R_exp = -2; /**/
		body->B_exp = 0;

		pwr_data[i].sensor_scanning = &vrm_power_scanning;

		body->setable_mask = 0x0;
		body->readable_mask = 0x0;

		body->unr_threshold = 255; /*Power don't have upper limit*/
		body->ucr_threshold = 255;
		body->unc_threshold = 255;
		body->lnr_threshold = 0;
		body->lcr_threshold = 0;
		body->lnc_threshold = 0;

		pwr_data[i].private_id = i;
		pwr_data[i].i2c_addr = pwr_sensor_list[i].i2c_addr;		
	
		add_sdr_entry(record,&pwr_data[i]);
	}
}
void
vrm_1v5_sdr_init(void)
{
	IPMI_SDR_HEADER	*header ;
	IPMI_SDR_KEY *key;
	IPMI_FULL_SDR_BODY *body;
	FULL_SENSOR_RECORD *record;
	int i;

	for (i=0;i<VRM_1V5_SENSOR_NUM;i++) {

		record = &vrm_1v5_sdr[i];
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
		body->entity_id = ENTITY_ID_POWER_MODULE; /*See 43-13*/
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
	
	
		body->sensor_type = SENSOR_TYPE_VOLT; /*See 42-3*/
		body->event_reading_type_code = 1;/*Unspecified:  See 42-1*/
		
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
		body->unit2_base_unit = vrm_1v5_sensor_list[i].unit; /*Unit type code, Volt*/
		body->unit3_modifier_unit = 0; /*not use*/
	
		
		body->analog_characteristic_flags = 0x00;
		
		body->nominal_reading = 0; 
		body->normal_max = 0; 
		body->normal_min = 0;
		
		body->sensor_max_reading = 0xff; /*256*/
		body->sensor_min_reading = 0x0;
		
	
		body->positive_going_threshold_hysteresis_value = 0;
		body->negative_going_threshold_hysteresis_value = 0;
		
		body->id_string_type=3; /*8bits ASCII _ Latin1*/
		body->id_string_len = 15;
		sprintf((unsigned char*)body->sensor_id_string,"%s",vrm_1v5_sensor_list[i].name);
		body->M_ls = 50; /*4mV resolution*/
		body->M_ms = 0;
		body->tolerance = 0;
	
		body->B_ls = 0;
		body->B_ms = 0;
		body->accuracy_ls = 0;
	
		body->accuracy_ms = 0x0;
		body->accuracy_exp = 0x0;
		body->sensor_direction = 0x1; /*input*/
	
		body->R_exp = -3; /*Convert from milivolt to Volt*/
		body->B_exp = 0;


		vrm_1v5_data[i].sensor_scanning = &vrm_1v5_scanning;
		body->unr_threshold = 40;
		body->ucr_threshold = 38;
		body->unc_threshold = 34;
		body->lnr_threshold = 20;
		body->lcr_threshold = 22;
		body->lnc_threshold = 24;
	
		vrm_1v5_data[i].private_id = i;
		vrm_1v5_data[i].i2c_addr = vrm_1v5_sensor_list[i].i2c_addr;		
	
		add_sdr_entry(record,&vrm_1v5_data[i]);
	}
}

void
vrm_sdr_init()
{
	vrm_0v95_sdr_init();
	vrm_power_sdr_init();
	vrm_1v5_sdr_init();
}
