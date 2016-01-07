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
 * @file apm_ipmi_sdr.h
 *
 * This file define IPMI Sensor Data Record.

 */


#ifndef __IPMI_SDR_H__
#define __IPMI_SDR_H__
/******************************************/
typedef struct ipmi_sdr_header {
	unsigned char record_id[2];
	unsigned char sdr_version; /* = 0x51h*/
	unsigned char record_type; /* = 0xC0h*/
	unsigned char record_len; /*FIXME: 16byte, */

} IPMI_SDR_HEADER;

typedef struct ipmi_compact_sdr_key {
	unsigned char owner_id;
	unsigned char owner_lun;
	unsigned char sensor_number;
}IPMI_SDR_KEY;

typedef struct ipmi_oem_sdr_data {
	unsigned char manufacture_id[3];
	unsigned char data[24]; /*FIXME: */
} IPMI_OEM_SDR_DATA;

typedef struct ipmi_compact_sdr_body  {
	unsigned char entity_id;
#ifdef __BIG_ENDIAN
	unsigned char entiry_type:1,
				  entity_instance_num:7;
#else
	unsigned char 
				entity_instance_num:7,
				entiry_type:1;
#endif	
	unsigned char sensor_initialization;
	unsigned char sensor_capabilities;
	unsigned char sensor_type;
	unsigned char event_reading_type_code;
	unsigned char event_threshold_mask_low[2];
	unsigned char event_threshold_mask_high[2];
	unsigned char reading_mask[2];
	unsigned char sensor_unit_1;
	unsigned char sensor_unit_2;
	unsigned char sensor_unit_3;
#ifdef __BIG_ENDIAN	
	unsigned char 
				sensor_direction:2,
				modifier_type:2,
				share_count:4;

#else
	unsigned char 
				share_count:4,
				modifier_type:2,
				sensor_direction:2;
#endif	
#ifdef __BIG_ENDIAN	
	unsigned char instance_share:1,
				  modifier_offset:7;
#else
	unsigned char 
				modifier_offset:7,
				instance_share:1;
#endif	
	unsigned char positive_going_threshold;
	unsigned char negative_going_threshold;
	unsigned char reserved[4]; /*reserved + OEM*/
	unsigned char type_length_code;
	unsigned char sensor_id_string[16];

	
} IPMI_COMPACT_SDR_BODY;

typedef struct ipmi_full_sdr_body  {
	unsigned char entity_id;
#ifdef __BIG_ENDIAN	
	unsigned char entity_type:1,
				  entity_instance_num:7;
#else
	unsigned char 
				entity_instance_num:7,
				entity_type:1;

#endif	
	/*Sensor Initialize*/
#ifdef __BIG_ENDIAN								
	unsigned char setable_sensor:1,
				  init_scanning:1,
				  init_event:1,
				  init_threshold:1,
				  init_hysteresis:1,
				  init_sensor_type:1,
				  event_generate:1,
				  sensor_scanning:1;
#else
	unsigned char 
				sensor_scanning:1,
				event_generate:1,
				init_sensor_type:1,
				init_hysteresis:1,
				init_threshold:1,
				init_event:1,
				init_scanning:1,
				setable_sensor:1;

#endif							

	/*Sensor Capabilities*/
#ifdef __BIG_ENDIAN				
	unsigned char ignore_sensor:1,
				  sensor_auto_re_arm_support:1,
				  sensor_hysteresis_support:2,
				  sensor_threshold_access_support:2,
				  sensor_event_message_support:2;
#else
	unsigned char 
				sensor_event_message_support:2,
				sensor_threshold_access_support:2,
				sensor_hysteresis_support:2,
				sensor_auto_re_arm_support:1,
				ignore_sensor:1;

#endif
	unsigned char sensor_type;
	unsigned char event_reading_type_code;
	unsigned char event_threshold_mask_low[2];
	unsigned char event_threshold_mask_high[2];
#ifdef __BIG_ENDIAN	
	unsigned char setable_mask;
	unsigned char readable_mask;
#else
	unsigned char readable_mask;
	unsigned char setable_mask;

#endif	

#ifdef __BIG_ENDIAN
	unsigned char unit1_analog_data_format:2,
				  unit1_rate_unit:3,
				  unit1_modifier_unit:2,
				  unit1_percentage:1;
#else
	unsigned char 
				unit1_percentage:1,
				unit1_modifier_unit:2,
				unit1_rate_unit:3,
				unit1_analog_data_format:2;

#endif	
	unsigned char unit2_base_unit;
	unsigned char unit3_modifier_unit;
	unsigned char linearization;
	unsigned char M_ls;
#ifdef __BIG_ENDIAN	
	unsigned char M_ms:2,
				  tolerance:6;
#else
	unsigned char 
				tolerance:6,
				M_ms:2;
#endif	
	unsigned char B_ls;
#ifdef __BIG_ENDIAN	
	unsigned char B_ms:2,
				  accuracy_ls:6;
#else
	unsigned char 
				accuracy_ls:6,
				B_ms:2;

#endif	
#ifdef __BIG_ENDIAN	
	unsigned char accuracy_ms:4,
				  accuracy_exp:2,
				  sensor_direction:2;
#else
	unsigned char 
			sensor_direction:2,
			accuracy_exp:2,
			accuracy_ms:4;

#endif	

#ifdef __BIG_ENDIAN	
	unsigned char R_exp:4,
				  B_exp:4;
#else
	unsigned char B_exp:4,
				  R_exp:4;

#endif	
	unsigned char analog_characteristic_flags;
	unsigned char nominal_reading;
	unsigned char normal_max;
	unsigned char normal_min;
	unsigned char sensor_max_reading;
	unsigned char sensor_min_reading;
	unsigned char unr_threshold;
	unsigned char ucr_threshold;
	unsigned char unc_threshold;
	unsigned char lnr_threshold;
	unsigned char lcr_threshold;
	unsigned char lnc_threshold;
	unsigned char positive_going_threshold_hysteresis_value;
	unsigned char negative_going_threshold_hysteresis_value;
	unsigned char reserved[3];
#ifdef __BIG_ENDIAN	
	unsigned char id_string_type:2,
				  type_reserve:1,
				  id_string_len:5;
#else
	unsigned char 
				id_string_len:5,
				type_reserve:1,
				id_string_type:2;

#endif	
	unsigned char sensor_id_string[16];

	
} IPMI_FULL_SDR_BODY;


typedef struct full_data_record {
	IPMI_SDR_HEADER	header;
	IPMI_SDR_KEY key;
	IPMI_FULL_SDR_BODY body;
}FULL_SENSOR_RECORD;



#define SDR_SENSOR_L_LINEAR     0x00
#define SDR_SENSOR_L_LN         0x01
#define SDR_SENSOR_L_LOG10      0x02
#define SDR_SENSOR_L_LOG2       0x03
#define SDR_SENSOR_L_E          0x04
#define SDR_SENSOR_L_EXP10      0x05
#define SDR_SENSOR_L_EXP2       0x06
#define SDR_SENSOR_L_1_X        0x07
#define SDR_SENSOR_L_SQR        0x08
#define SDR_SENSOR_L_CUBE       0x09
#define SDR_SENSOR_L_SQRT       0x0a
#define SDR_SENSOR_L_CUBERT     0x0b
#define SDR_SENSOR_L_NONLINEAR  0x70


#define SDR_TYPE_FULL	0x01
#define SDR_TYPE_COMPACT 0x02
#define SDR_TYPE_OEM	0xC0

#define SENSOR_TYPE_TEMP		0x1
#define SENSOR_TYPE_VOLT		0x2
#define SENSOR_TYPE_CURRENT		0x3
#define SENSOR_TYPE_FAN			0x4
#define SENSOR_TYPE_PROCESSOR	0x7

#define SENSOR_READING_TYPE_THRESHOLD	0x01
#define SENSOR_READING_TYPE_GENERIC		0x02
#define SENSOR_READING_TYPE_OEM			0x70


#define ENTITY_ID_PROCESSOR				0x03
#define ENTITY_ID_SYSTEM_BOARD			0x07
#define ENTITY_ID_FAN					0x1D
#define ENTITY_ID_MEMORY				0x20
#define ENTITY_ID_POWER_SUPPLY			0x0A
#define ENTITY_ID_POWER_MODULE			0x14

#define UNIT_TYPE_DEGREE_C		1
#define UNIT_TYPE_DEGREE_F		2
#define UNIT_TYPE_DEGREE_K		3
#define UNIT_TYPE_VOLT			4
#define UNIT_TYPE_AMP			5
#define UNIT_TYPE_WATT			6
#define UNIT_TYPE_RPM			18

#define LNR_THRESHOLD_STATUS	0x01
#define LCR_THRESHOLD_STATUS	0x02
#define LNC_THRESHOLD_STATUS	0x04
#define UNC_THRESHOLD_STATUS	0x08
#define UCR_THRESHOLD_STATUS	0x10
#define UNR_THRESHOLD_STATUS	0x20

typedef struct sdr_entry {
	unsigned short record_id;
	unsigned char record_len;
	unsigned char *record_ptr;
} SDR_ENTRY;

typedef struct sensor_data {
	unsigned char sensor_id;
	int private_id;
	unsigned char i2c_bus;
	unsigned char i2c_addr;
	void (*sensor_scanning)(void *); /*Function to update sensor data*/
	unsigned char sensor_reading;
#ifdef __BIG_ENDIAN	
	unsigned char event_message_enable:1,
				  sensor_scanning_enable:1,
				  reading_unavailable:1,
				  reserved:5;
#else
	unsigned char 
				reserved:5,
				reading_unavailable:1,
				sensor_scanning_enable:1,
				event_message_enable:1;

#endif	
	unsigned char status;
	unsigned char data[6];
}SENSOR_DATA;

#define MAX_SDR_RECORD	64

extern SDR_ENTRY sdr_repository[MAX_SDR_RECORD];
extern SENSOR_DATA *sensor[MAX_SDR_RECORD];
extern int current_sensor_count;

int add_sdr_entry(FULL_SENSOR_RECORD *sdr,SENSOR_DATA *sensor_data);
void ipmi_sdr_repository_initialize(void);
void ipmi_sdr_repository_cleanup(void);


/* 
 * Uncomment following line to enable sensor scan in separate thread
 * #define SENSOR_SCANNING_THREAD_ENABLE 
 * */
#ifdef SENSOR_SCANNING_THREAD_ENABLE
int is_sensor_scan_requested(void);
void set_sensor_scan_request(int param);
#endif

#endif /*__IPMI_SDR_H__*/
