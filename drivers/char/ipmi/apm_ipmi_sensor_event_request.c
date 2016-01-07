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
 * @file apm_ipmi_sensor_event_request.c
 *
 *
 * This file process IPMI Sensor & Event messages.
 */


#include "apm_ipmi_sdr.h"
#include "apm_ipmi_sensor_event_request.h"

#define BUILD_SE_RESPONSE_MESSAGE(cmd,resp,resp_data) \
	resp_data[0] = IPMI_NETFN_SENSOR_EVENT_RESPONSE << 2; \
	resp_data[1] = cmd; \
	memcpy(resp_data +2,(unsigned char*)&resp,sizeof(resp)); 

	
#define IPMI_GET_EVENT_MESSAGE_CMD		0x02
#define IPMI_SET_SENSOR_THRESHOLD_CMD		0x26

int 
ipmi_do_get_device_sdr_info(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_DEVICE_SDR_INFO_RESP resp;

	memset(&resp,0,sizeof(IPMI_GET_DEVICE_SDR_INFO_RESP));

    resp.completion_code = 0x0;
    //resp.event_msg_buf = 1;

    BUILD_SE_RESPONSE_MESSAGE(
                IPMI_GET_DEVICE_SDR_INFO_CMD,
                resp,
				resp_data);

    return sizeof(resp) + 2;
}

int 
ipmi_do_get_device_sdr(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_DEVICE_SDR_RESP resp;
	IPMI_GET_DEVICE_SDR_REQ req_data;
	int i;

	memset(&resp,0,sizeof(IPMI_GET_DEVICE_SDR_RESP));


	if (msg->data_len != sizeof(req_data)) {
		printk("ERROR:%s. data size mismatched\n",__func__);
	}
	
	memcpy(&req_data,msg->data,msg->data_len);

    resp.completion_code = 0x0;
	for (i=0; i<req_data.byte_to_read; i++)
		resp.data[i] = 0x0;

    BUILD_SE_RESPONSE_MESSAGE(
                IPMI_GET_DEVICE_SDR_CMD,
                resp,
				resp_data);

    return req_data.byte_to_read + 5;
}


int
ipmi_do_reserve_device_sdr_info(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_RESERVE_DEVICE_SDR_INFO_RESP resp;
	memset(&resp,0,sizeof(IPMI_RESERVE_DEVICE_SDR_INFO_RESP));

    resp.completion_code = 0x0;
    resp.reserve_id_ls = 1;

    BUILD_SE_RESPONSE_MESSAGE(
                IPMI_RESERVE_DEVICE_SDR_INFO_CMD,
                resp,
                resp_data);

    return sizeof(resp) + 2;

	
}

int
ipmi_do_get_sensor_reading(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_SENSOR_READING_RESP resp;
	unsigned char sensor_number = msg->data[0];
	int i,found = 0;

	memset(&resp,0,sizeof(IPMI_GET_SENSOR_READING_RESP));

	for(i=0;i<current_sensor_count;i++)
		if (sensor[i]->sensor_id == sensor_number) {
			found ++;
			break;
		}

#ifndef SENSOR_SCANNING_THREAD_ENABLE 	
	if (found && (sensor[i]->sensor_scanning != NULL))
		sensor[sensor_number]->sensor_scanning((void*)sensor[sensor_number]);
#endif
	if (found)
	{
		FULL_SENSOR_RECORD *record = (FULL_SENSOR_RECORD *)sdr_repository[i].record_ptr;
		resp.completion_code = 0;
		resp.sensor_reading = sensor[i]->sensor_reading;
		resp.event_message_enable = sensor[i]->event_message_enable;
		resp.sensor_scanning_enable = sensor[i]->sensor_scanning_enable;
		resp.reading_unavailable = sensor[i]->reading_unavailable;
		/*Check status based on SDR */
		if (resp.reading_unavailable == 0) {
			/*UPPER*/
			if(sensor[i]->sensor_reading > record->body.unr_threshold)
				resp.data[0] |= UNR_THRESHOLD_STATUS;
			if(sensor[i]->sensor_reading > record->body.ucr_threshold)
				resp.data[0] |= UCR_THRESHOLD_STATUS;
			if(sensor[i]->sensor_reading > record->body.unc_threshold)
				resp.data[0] |= UNC_THRESHOLD_STATUS;
			/*LOWER*/
			if(sensor[i]->sensor_reading < record->body.lnc_threshold)
				resp.data[0] |= LNC_THRESHOLD_STATUS;
			if(sensor[i]->sensor_reading < record->body.lcr_threshold)
				resp.data[0] |= LCR_THRESHOLD_STATUS;
			if(sensor[i]->sensor_reading < record->body.lnr_threshold)
				resp.data[0] |= LNR_THRESHOLD_STATUS;
		
			sensor[i]->status = resp.data[0];
		}
	}
	else {
		printk("%s:Sensor 0x%x not found\n",__func__,sensor_number);
		resp.completion_code = 1;
	}

    BUILD_SE_RESPONSE_MESSAGE(
                IPMI_GET_SENSOR_READING_CMD,
                resp,
                resp_data);

    return sizeof(resp) + 2;
	
}

int
ipmi_do_get_sensor_event_status(REQ_MSG *msg, 
					unsigned char *resp_data)
{
	IPMI_GET_SENSOR_EVENT_STATUS_RESP resp;
	memset(&resp,0,sizeof(IPMI_GET_SENSOR_EVENT_STATUS_RESP));

    resp.completion_code = 0x0;
	resp.flags = 0x00;


    BUILD_SE_RESPONSE_MESSAGE(
                IPMI_GET_SENSOR_EVENT_STATUS_CMD,
                resp,
                resp_data);

    return sizeof(resp) + 2;
	
}

int
ipmi_do_get_sensor_threshold(REQ_MSG *msg, 
					unsigned char *resp_data)
{
	IPMI_GET_SENSOR_THRESHOLD_RESP resp;
	unsigned char sensor_number = msg->data[0];

	int i,found = 0;
#if 0
	printk("%s: sensor_number=0x%02x\n",__func__,sensor_number);
#endif	

	memset(&resp,0,sizeof(IPMI_GET_SENSOR_THRESHOLD_RESP));

	for(i=0;i<current_sensor_count;i++)
		if (sensor[i]->sensor_id == sensor_number) {
			found ++;
			break;
		}

#if 0	
	if (found && (sensor[i]->sensor_scanning != NULL))
		sensor[sensor_number]->sensor_scanning((void*)sensor[sensor_number]);
#endif
	if (found)
	{
		unsigned char status = sensor[i]->status;
		FULL_SENSOR_RECORD *record = (FULL_SENSOR_RECORD *)sdr_repository[i].record_ptr;

		resp.completion_code = 0;
		resp.readable_threshold = status;
		
		if (status & 0x01) {
			resp.data[0] = record->body.lnr_threshold;
		}
		if (status & 0x02) {
			resp.data[1] = record->body.lcr_threshold;
		}
		if (status & 0x04) {
			resp.data[2] = record->body.lnc_threshold;
		}
		if (status & 0x08) {
			resp.data[3] = record->body.unc_threshold;
		}
		if (status & 0x16) {
			resp.data[4] = record->body.ucr_threshold;
		}
		if (status & 0x32) {
			resp.data[5] = record->body.unr_threshold;
		}

	}
	else {
		printk("%s:Sensor 0x%x not found\n",__func__,sensor_number);
		resp.completion_code = 1;
	}

    BUILD_SE_RESPONSE_MESSAGE(
                IPMI_GET_SENSOR_THRESHOLD_CMD,
                resp,
                resp_data);

    return sizeof(resp) + 2;
	
}

int
ipmi_do_set_sensor_threshold(REQ_MSG *msg, 
					unsigned char *resp_data)
{
	IPMI_GENERIC_RESP resp;
	IPMI_SET_SENSOR_THRESHOLD_REQ	req;
	int i,found = 0;


	memset(&req,0,sizeof(req));
	memcpy(&req,(u8 *)msg + 2,sizeof(req));

	memset(&resp,0,sizeof(IPMI_GENERIC_RESP));

	for(i=0;i<current_sensor_count;i++)
		if (sensor[i]->sensor_id == req.sensor_number) {
			found ++;
			break;
		}

/*	
	if (found && (sensor[i]->sensor_scanning != NULL))
		sensor[req.sensor_number]->sensor_scanning((void*)sensor[req.sensor_number]);
*/
	if (found)
	{
		FULL_SENSOR_RECORD *record  = (FULL_SENSOR_RECORD *)sdr_repository[i].record_ptr;
#if 0		
		printk("%s:Found at %d\n",__func__,i);
		printk("Req Info:\n	"
				"Sensor_num: 0x%x\n"
				"\tSet Flags:0x%x\n"
				"\tdata:",msg->data[0],msg->data[1]);
		for (i=0;i<6;i++)
			printk("%02x ",msg->data[i+2]);
		printk("\n");
#endif
		if (req.set_flags & 0x01) {
			record->body.lnr_threshold = req.data[0];		
		}
		else if (req.set_flags & 0x02) {
			record->body.lcr_threshold = req.data[1];
		}
		else if (req.set_flags & 0x04) {
			record->body.lnc_threshold = req.data[2];
		}
		else if (req.set_flags & 0x08) {
			record->body.unc_threshold = req.data[3];
		}
		else if (req.set_flags & 0x16) {
			record->body.ucr_threshold = req.data[4];
		}
		else if (req.set_flags & 0x32) {
			record->body.unr_threshold = req.data[5];
		}

	}
	else {
		printk("%s:Sensor 0x%x not found\n",__func__,req.sensor_number);
		resp.completion_code = 1;
	}

    BUILD_SE_RESPONSE_MESSAGE(
                IPMI_SET_SENSOR_THRESHOLD_CMD,
                resp,
                resp_data);

    return sizeof(resp) + 2;
	
}


int
ipmi_do_get_sensor_event_enable(REQ_MSG *msg, 
					unsigned char *resp_data)
{
	IPMI_GET_SENSOR_EVENT_ENABLE_RESP resp;
	memset(&resp,0,sizeof(IPMI_GET_SENSOR_EVENT_ENABLE_RESP));

    resp.completion_code = 0x0;
	resp.flags = 0x00;


    BUILD_SE_RESPONSE_MESSAGE(
                IPMI_GET_SENSOR_EVENT_ENABLE_CMD,
                resp,
                resp_data);

    return sizeof(resp) + 2;
	
}

int
ipmi_do_get_event_message(REQ_MSG *msg, 
					unsigned char *resp_data)
{
	IPMI_GENERIC_RESP resp;
#if 0
	int i;

	printk("%s\n",__func__);
	for (i=0;i<8;i++)
		printk("%02x ",msg->data[i]);
	printk("\n");
#endif

	memset(&resp,0,sizeof(IPMI_GENERIC_RESP));
    resp.completion_code = 0x0;


    BUILD_SE_RESPONSE_MESSAGE(
                IPMI_GET_EVENT_MESSAGE_CMD,
                resp,
                resp_data);

    return sizeof(resp) + 2;
	
}



int 
ipmi_process_sensor_event_request(REQ_MSG *msg, unsigned char *resp_data)
{
	int ret = 0;
#if 0 
	int i;
	printk("%s: cmd=0x%02xh,data=",__func__,msg->cmd_code);
	if (msg->data_len > 0)
		for (i=0;i<msg->data_len;i++)
			printk("%02x ",msg->data[i]);
	printk("\n");
#endif
	switch(msg->cmd_code)
	{
		case IPMI_GET_DEVICE_SDR_INFO_CMD:
			ret = ipmi_do_get_device_sdr_info(msg,resp_data);
			break;
		case IPMI_GET_EVENT_MESSAGE_CMD:
			ret = ipmi_do_get_event_message(msg,resp_data);
			break;

		case IPMI_GET_DEVICE_SDR_CMD:
			ret = ipmi_do_get_device_sdr(msg,resp_data);
			break;

		case IPMI_RESERVE_DEVICE_SDR_INFO_CMD:
			ret = ipmi_do_reserve_device_sdr_info(msg,resp_data);
			break;
		case IPMI_GET_SENSOR_THRESHOLD_CMD:
			ret = ipmi_do_get_sensor_threshold(msg,resp_data);
			break;
		case IPMI_SET_SENSOR_THRESHOLD_CMD:
			ret = ipmi_do_set_sensor_threshold(msg,resp_data);
			break;
		case IPMI_GET_SENSOR_READING_CMD:
			ret = ipmi_do_get_sensor_reading(msg,resp_data);
			break;
		case IPMI_GET_SENSOR_EVENT_STATUS_CMD:
			ret = ipmi_do_get_sensor_event_status(msg,resp_data);
			break;
		case IPMI_GET_SENSOR_EVENT_ENABLE_CMD:
			ret = ipmi_do_get_sensor_event_enable(msg,resp_data);
			break;
		default:
			printk("S/E command 0x%02x not supported yet\n",msg->cmd_code);
			break;
	}
	return ret;
}

