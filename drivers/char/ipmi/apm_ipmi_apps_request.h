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
 * @file apm_ipmi_apps_request.h
 *
 * This file define IPMI Apps response message format.

 */

#ifndef __IPMI_APPS_REQUEST_H__
#define __IPMI_APPS_REQUEST_H__

typedef struct ipmi_get_device_id_resp {
	unsigned char completion_code;
	unsigned char device_id;
	unsigned char device_revision;
	unsigned char fw_rev_1;
	unsigned char fw_rev_2;
	unsigned char ipmi_version;
#ifdef __BIG_ENDIAN	
	unsigned char chassic_device_support:1,
				  brigde_device_support:1,
				  ipmb_event_generator_support:1,
				  ipmb_event_receiver_support:1,
				  fru_inventory_support:1,
				  sel_device_support:1,
				  sdr_repository_support:1,
				  sensor_device_support:1;
#else
	unsigned char sensor_device_support:1,
				  sdr_repository_support:1,
				  sel_device_support:1,
				  fru_inventory_support:1,
				  ipmb_event_receiver_support:1,
				  ipmb_event_generator_support:1,
				  brigde_device_support:1,
				  chassic_device_support:1;

#endif	
	unsigned char manufacture_id[3];
	unsigned char product_id[2];
	unsigned char aux_fw_info[4];
} IPMI_GET_DEVICE_ID_RESP;

typedef struct ipmi_set_bcm_global_enable_cmd {
#ifdef __BIG_ENDIAN	
	unsigned char   oem2:1,
        oem1:1,
        oem0:1,
        :1,
        syslog:1,
        event_msg_buf:1,
        event_msg_buf_full_intr:1,
        recv_msg_queue_intr:1;
#else
	unsigned char 
        recv_msg_queue_intr:1,
        event_msg_buf_full_intr:1,
        event_msg_buf:1,
        syslog:1,
        :1,
        oem0:1,
        oem1:1,
	  	oem2:1;

#endif	
} IPMI_SET_BMC_GLOBAL_ENABLE_CMD;

typedef struct ipmi_set_bmc_global_enable_resp {
	unsigned char completion_code;
} IPMI_SET_BMC_GLOBAL_ENABLE_RESP;

typedef struct ipmi_get_bmc_global_enable_resp {
	unsigned char completion_code;
#ifdef __BIG_ENDIAN	
	unsigned char 	oem2:1,
		oem1:1,
		oem0:1,
		:1,
		syslog:1,
		event_msg_buf:1,
		event_msg_buf_full_intr:1,
		recv_msg_queue_intr:1;	   
#else
	unsigned char 	
		recv_msg_queue_intr:1,
		event_msg_buf_full_intr:1,
		event_msg_buf:1,
		syslog:1,
		:1,
		oem0:1,
		oem1:1,
		oem2:1;

#endif	

} IPMI_GET_BMC_GLOBAL_ENABLE_RESP;

typedef struct ipmi_get_device_guid_resp {
	unsigned char completion_code;
	unsigned char node[6];
	unsigned char clock_seq[2];
	unsigned char time_high[2];
	unsigned char time_mid[2];
	unsigned char time_low[4];
} IPMI_GET_DEVICE_GUID_RESP;

typedef struct ipmi_read_event_message_buffer_resp {
	unsigned char completion_code;
	unsigned char message_data[16];
}IPMI_READ_EVENT_MESSAGE_BUFFER_RESP;

typedef struct ipmi_clear_message_flags_resp {
	unsigned char completion_code;
} IPMI_CLEAR_MSG_FLAGS_RESP;

typedef struct ipmi_get_channel_info_resp {
	unsigned char completion_code;
	unsigned char channel_number;
	unsigned char channel_medium_type;
	unsigned char channel_protocol_type;
	unsigned char session_support;
	unsigned char vendor_id[3];
	unsigned char aux_info[2];
} IPMI_GET_CHANNEL_INFO_RESP;


int ipmi_process_apps_request (REQ_MSG *msg, unsigned char *resp_data);

#endif //__IPMI_APPS_REQUEST_H__
