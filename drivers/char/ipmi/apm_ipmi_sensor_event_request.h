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
 * @file apm_ipmi_sensor_event_request.h
 *
 * This file define IPMI Sensor & Event response message format.

 */


#ifndef __IPMI_SENSOR_EVENT_REQUEST_H__
#define __IPMI_SENSOR_EVENT_REQUEST_H__
#include "apm_ipmi_request.h"

typedef struct ipmi_get_device_sdr_info_resp {
	unsigned char completion_code;
	unsigned char sensor_count;
	unsigned char flags;
	unsigned char change_indicator[4];
} IPMI_GET_DEVICE_SDR_INFO_RESP;

typedef struct ipmi_get_device_sdr_req {
	unsigned char reservation_id_ls;
	unsigned char reservation_id_ms;
	unsigned char record_id_ls;
	unsigned char record_id_ms;
	unsigned char offset;
	unsigned char byte_to_read;
} IPMI_GET_DEVICE_SDR_REQ;

typedef struct ipmi_get_device_sdr_resp {
	unsigned char completion_code;
	unsigned char next_id_ls;
	unsigned char next_id_ms;
	unsigned char data[25];
} IPMI_GET_DEVICE_SDR_RESP;

typedef struct ipmi_get_sensor_reading_resp {
	unsigned char completion_code;
	unsigned char sensor_reading;
#ifdef __BIG_ENDIAN	
	unsigned char event_message_enable:1,
				  sensor_scanning_enable:1,
				  reading_unavailable:1,
				  reserved:5;
#else
	unsigned char reserved:5,
				  reading_unavailable:1,
				  sensor_scanning_enable:1,
				  event_message_enable:1;

#endif	
	unsigned char data[2];
} IPMI_GET_SENSOR_READING_RESP;

typedef struct ipmi_set_sensor_threshold_resp {
	unsigned char sensor_number;
	unsigned char set_flags;
	unsigned char data[6];
} IPMI_SET_SENSOR_THRESHOLD_REQ;

typedef struct ipmi_get_sensor_threshold_resp {
	unsigned char completion_code;
	unsigned char readable_threshold;
	unsigned char data[6];
} IPMI_GET_SENSOR_THRESHOLD_RESP;


typedef struct ipmi_get_sensor_event_enable_resp {
	unsigned char completion_code;
	unsigned char flags;
	unsigned char data[4];
} IPMI_GET_SENSOR_EVENT_ENABLE_RESP;

typedef struct ipmi_get_sensor_event_status_resp {
	unsigned char completion_code;
	unsigned char flags;
	unsigned char data[3];
} IPMI_GET_SENSOR_EVENT_STATUS_RESP;


typedef struct ipmi_reserve_device_sdr_info_resp {
	unsigned char completion_code;
	unsigned char reserve_id_ls;
	unsigned char reserve_id_ms;
} IPMI_RESERVE_DEVICE_SDR_INFO_RESP;



int ipmi_process_sensor_event_request(REQ_MSG *msg, unsigned char *resp_data);
#endif //__IPMI_SENSOR_EVENT_REQUEST_H__
