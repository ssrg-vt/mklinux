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
 * @file apm_ipmi_apps_request.c
 *
 * This file process IPMI Apps command group.
 */
#include "apm_ipmi_request.h"
#include "apm_ipmi_apps_request.h"
#include "apm_ipmi_sdr.h"

#define BUILD_APPS_RESPONSE_MESSAGE(cmd,resp,resp_data) \
	resp_data[0] = IPMI_NETFN_APP_RESPONSE << 2; \
	resp_data[1] = cmd; \
	memcpy(resp_data +2,(unsigned char*)&resp,sizeof(resp)); \

	
int
ipmi_do_get_device_id(REQ_MSG *msg,unsigned char *resp_data)
{
	IPMI_GET_DEVICE_ID_RESP resp;
#ifdef SENSOR_SCANNING_THREAD_ENABLE
	/* FIXME: if this thread enable it should be
	 * schedule to running, temporary put here to 
	 * scan anytime that have ipmi request.
	 * Enable Sensor scanning */
	set_sensor_scan_request(true);
#endif	

	memset(&resp,0,sizeof(IPMI_GET_DEVICE_ID_RESP));

	/*We are virtual BMC, so hardcode for device ID*/
	resp.completion_code = 0;
	resp.device_id = 0x1;
	resp.device_revision = 0x1;
	resp.fw_rev_1 = 0x1;
	resp.fw_rev_2 = 0x5;
	resp.ipmi_version = 0x02; /*IPMI V2.0*/

	resp.chassic_device_support = 1;
    resp.brigde_device_support = 0;
	resp.ipmb_event_generator_support = 0;
	resp.ipmb_event_receiver_support = 0;
	resp.fru_inventory_support = 0;
	resp.sel_device_support = 0;
	resp.sdr_repository_support = 1;
	resp.sensor_device_support = 1;

	resp.manufacture_id[0] = 0xe8;
	resp.manufacture_id[1] = 0x10;

	BUILD_APPS_RESPONSE_MESSAGE(
			IPMI_GET_DEVICE_ID_CMD,
			resp,
			resp_data);

	return sizeof(resp) + 2 ;
}

int 
ipmi_do_get_bmc_global_enable(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_BMC_GLOBAL_ENABLE_RESP resp;

	memset(&resp,0,sizeof(IPMI_GET_BMC_GLOBAL_ENABLE_RESP));

	resp.completion_code = 0x0;
	//resp.syslog = 1;
	//resp.event_msg_buf = 1;

	BUILD_APPS_RESPONSE_MESSAGE(
				IPMI_GET_BMC_GLOBAL_ENABLES_CMD,
				resp,
				resp_data);
	return sizeof(resp) + 2;
}

int 
ipmi_do_set_bmc_global_enable(REQ_MSG *msg,unsigned char *resp_data)
{
	IPMI_GET_BMC_GLOBAL_ENABLE_RESP resp;

	/*Set data*/
	//printk("%s: data=0x%02x\n",__func__,msg->data[0]);

	memset(&resp,0,sizeof(IPMI_SET_BMC_GLOBAL_ENABLE_RESP));

	resp.completion_code = 0x0;
	//resp.syslog = 1;
	//resp.event_msg_buf = 1;

	BUILD_APPS_RESPONSE_MESSAGE(
				IPMI_SET_BMC_GLOBAL_ENABLES_CMD,
				resp,
				resp_data);

	return sizeof(resp) + 2;
}

int
ipmi_do_get_device_guid(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_DEVICE_GUID_RESP resp;

	memset(&resp,0,sizeof(IPMI_GET_DEVICE_GUID_RESP));

    resp.completion_code = 0x0;

    BUILD_APPS_RESPONSE_MESSAGE(
                IPMI_GET_DEVICE_GUID_CMD,
                resp,
				resp_data);

    return sizeof(resp) + 2;
}
int
ipmi_do_read_event_message_buffer(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_READ_EVENT_MESSAGE_BUFFER_RESP resp;

    memset(&resp,0,sizeof(IPMI_READ_EVENT_MESSAGE_BUFFER_RESP));

    resp.completion_code = 0x1;
    //resp.syslog = 1;
    //resp.event_msg_buf = 1;

    BUILD_APPS_RESPONSE_MESSAGE(
                IPMI_READ_EVENT_MSG_BUFFER_CMD,
                resp,
				resp_data);   

    return sizeof(resp) + 2;	
}
static int
ipmi_do_clear_message_flags(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_CLEAR_MSG_FLAGS_RESP resp;
	memset(&resp,0,sizeof(IPMI_CLEAR_MSG_FLAGS_RESP));

	resp.completion_code = 0x0;

	//printk("TBD: %s, data=0x%x\n",__func__,flags_data);

	BUILD_APPS_RESPONSE_MESSAGE(
			IPMI_CLEAR_MSG_FLAGS_CMD,
			resp,
			resp_data);

	return sizeof(resp) + 2;
}

static int
ipmi_do_get_channel_info(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_CHANNEL_INFO_RESP resp;
	unsigned char  channel_number = msg->data[0];
	

	memset(&resp,0,sizeof(IPMI_GET_CHANNEL_INFO_RESP));

	resp.completion_code = 0x0;
	resp.channel_number = channel_number;
	resp.channel_medium_type = 0xC; /*System Interface*/
	resp.channel_protocol_type = 0x05; /*KCS*/
	resp.session_support = 0x40; /*single session, no session actived*/ 



	BUILD_APPS_RESPONSE_MESSAGE(
			IPMI_GET_CHANNEL_INFO_CMD,
			resp,
			resp_data);

	return sizeof(resp) + 2;
}



int
ipmi_process_apps_request (REQ_MSG *msg, unsigned char *resp_data)
{
	int ret=0;

#if 0
	int i;
		printk("%s: cmd=0x%02xh,data=",__func__,msg->cmd_code);
		if (msg->data_len > 0)
			for (i=0;i<msg->data_len;i++)
				printk("%02x ",msg->data[i]);
		printk("\n");
#endif

	switch (msg->cmd_code)
	{
		case IPMI_GET_DEVICE_ID_CMD:
			ret = ipmi_do_get_device_id(msg,resp_data);
			break;
		case IPMI_GET_BMC_GLOBAL_ENABLES_CMD:
			ret = ipmi_do_get_bmc_global_enable(msg,resp_data);
			break;
		case IPMI_SET_BMC_GLOBAL_ENABLES_CMD:
			ret = ipmi_do_set_bmc_global_enable(msg,resp_data);
			break;
		case IPMI_GET_DEVICE_GUID_CMD:
			ret = ipmi_do_get_device_guid(msg,resp_data);
			break;
		case IPMI_READ_EVENT_MSG_BUFFER_CMD:
			ret = ipmi_do_read_event_message_buffer(msg,resp_data);
			break;
		case IPMI_CLEAR_MSG_FLAGS_CMD:
			ret = ipmi_do_clear_message_flags(msg,resp_data);
			break;
		case IPMI_GET_CHANNEL_INFO_CMD:
			ret = ipmi_do_get_channel_info(msg,resp_data);
			break;
		default:
			printk("Apps command 0x%02x not implement yet\n",msg->cmd_code);
			break;
	}

	return ret;
}

