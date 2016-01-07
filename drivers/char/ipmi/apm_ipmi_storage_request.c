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
 * @file apm_ipmi_storage_request.c
 *
 *
 * This file process IPMI Storage messages.
 */


#include "apm_ipmi_storage_request.h"
#include "apm_ipmi_sdr.h"


#define BUILD_STORAGE_RESPONSE_MESSAGE(cmd,resp,resp_data) \
	resp_data[0] = IPMI_NETFN_STORAGE_RESPONSE << 2; \
	resp_data[1] = cmd; \
	memcpy(resp_data +2,(unsigned char*)&resp,sizeof(resp)); 

#define IPMI_GET_FRU_INVENTORY_AREA_INFO_CMD	0x10
#define IPMI_READ_FRU_CMD				0x11

int 
ipmi_do_get_sdr_repository_info(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_SDR_REPOSITORY_INFO_RESP resp;

	memset(&resp,0,sizeof(IPMI_GET_SDR_REPOSITORY_INFO_RESP));

    resp.completion_code = 0x0;
    resp.sdr_version = 0x51;
    resp.record_count_ls = current_sensor_count;
	resp.record_count_ms = 0;
	//resp.free_space[0] = MAX_SDR_RECORD - current_sensor_count;
	//resp.free_space[1] = 0;

    BUILD_STORAGE_RESPONSE_MESSAGE(
                IPMI_GET_SDR_REPOSITORY_INFO_CMD,
                resp,
				resp_data);

    return sizeof(resp) + 2;
}

int 
ipmi_do_reserve_sdr_repository(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_RESERVE_SDR_REPOSITORY_RESP resp;

	memset(&resp,0,sizeof(IPMI_RESERVE_SDR_REPOSITORY_RESP));

    resp.completion_code = 0x0;
    resp.reservation_id_ls = 0x00;

    BUILD_STORAGE_RESPONSE_MESSAGE(
                IPMI_RESERVE_SDR_REPOSITORY_CMD,
                resp,
				resp_data);

    return sizeof(resp) + 2;
}

int
ipmi_do_clear_sdr_repository(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_CLEAR_SDR_REPOSITORY_RESP resp;

	memset(&resp,0,sizeof(IPMI_CLEAR_SDR_REPOSITORY_RESP));

    resp.completion_code = 0x0;
    resp.erase_complete = 1;

    BUILD_STORAGE_RESPONSE_MESSAGE(
                IPMI_CLEAR_SDR_REPOSITORY_CMD,
                resp,
				resp_data);

    return sizeof(resp) + 2;

}

int
ipmi_do_get_sdr(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_SDR_REQ	req_data;
	IPMI_GET_SDR_RESP resp;
	int ret,i,record_id,found = 0;

	memset(&resp,0,sizeof(IPMI_GET_SDR_RESP));

	/*Get Request data*/
	if (msg->data_len != sizeof(req_data)) {
		printk("ERROR: %s. Length data not matched\n",__func__);
	}
	memcpy(&req_data,msg->data,msg->data_len);
	

	record_id = (req_data.record_id_ms << 8) | req_data.record_id_ls;

	/*Looking for Record id in SDR repository*/

	for (i=0;i < current_sensor_count;i ++) {
		if(sdr_repository[i].record_id == record_id) {
			found ++;
			break;
		}
	}

	if(found) {
	    resp.completion_code = 0x0;
		if (i+1 < current_sensor_count) {
			resp.next_id_ls = sdr_repository[i+1].record_id & 0xff;
			resp.next_id_ms = (sdr_repository[i+1].record_id >> 8) & 0xff;

		}else {
			resp.next_id_ls = 0xff;
			resp.next_id_ms = 0xff;

		}



		if (req_data.byte_to_read + req_data.offset > sdr_repository[i].record_len) {
			memcpy(((unsigned char *)&resp.data),
				(unsigned char *)(sdr_repository[i].record_ptr + req_data.offset), 
				req_data.byte_to_read);

		}
		else { /*Read header only*/
			memcpy(((unsigned char *)&(resp.data)),
				(unsigned char *)sdr_repository[i].record_ptr,
				req_data.byte_to_read);

		}
	}

	else {

	}


    BUILD_STORAGE_RESPONSE_MESSAGE(
                IPMI_GET_SDR_CMD,
                resp,
				resp_data);

	ret = req_data.byte_to_read + 5;
    return ret;

}

int ipmi_do_get_fru_inventory_area_info(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_FRU_INVENTORY_AREA_INFO_RESP resp;
	int fru_device = msg->data[0];

	printk("%s: fru_device=0x%x\n",__func__,fru_device);
	memset(&resp,0,sizeof(IPMI_GET_FRU_INVENTORY_AREA_INFO_RESP));

	resp.completion_code = 0x0;
    resp.fru_size_ls = 10;
	resp.fru_size_ms = 0;
	resp.access_type = 0; /*bytes*/

    BUILD_STORAGE_RESPONSE_MESSAGE(
              IPMI_GET_FRU_INVENTORY_AREA_INFO_CMD,
              resp,
              resp_data);

   return sizeof(resp) + 2;
}
int ipmi_do_read_fru(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_READ_FRU_REQ req;
	IPMI_READ_FRU_RESP resp;
	
	req.fru_device = msg->data[0];
	req.read_offset_ls = msg->data[1];
	req.read_offset_ms = msg->data[2];
	req.read_count = msg->data[3];


	printk("%s: fru_device=0x%x,offset=0x%02x%02x, count=%d \n",__func__,
				req.fru_device,req.read_offset_ms,req.read_offset_ls, req.read_count);
	memset(&resp,0,sizeof(IPMI_READ_FRU_RESP));

	resp.completion_code = 0x0;
    resp.count_returned = 10;
	resp.data[0] = 0;
	resp.data[1] = 1;
	resp.data[2] = 2;
	resp.data[3] = 3;
	resp.data[4] = 4;
	resp.data[5] = 5;
	resp.data[6] = 6;
	resp.data[7] = 7;
	resp.data[8] = 8;
	resp.data[9] = 9;

    BUILD_STORAGE_RESPONSE_MESSAGE(
              IPMI_READ_FRU_CMD,
              resp,
              resp_data);

   return sizeof(resp) + 2;
}


int 
ipmi_process_storage_request(REQ_MSG *msg, unsigned char *resp_data)
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
		case IPMI_GET_SDR_REPOSITORY_INFO_CMD:
			ret = ipmi_do_get_sdr_repository_info(msg,resp_data);
			break;
		case IPMI_RESERVE_SDR_REPOSITORY_CMD:
			ret = ipmi_do_reserve_sdr_repository(msg,resp_data);
			break;
		case IPMI_CLEAR_SDR_REPOSITORY_CMD:
			ret = ipmi_do_clear_sdr_repository(msg,resp_data);
			break;
		case IPMI_GET_SDR_CMD:
			ret = ipmi_do_get_sdr(msg,resp_data);
			break;
		case IPMI_GET_FRU_INVENTORY_AREA_INFO_CMD:
			ret = ipmi_do_get_fru_inventory_area_info(msg,resp_data);
			break;
		case IPMI_READ_FRU_CMD:
			ret = ipmi_do_read_fru(msg,resp_data);
			break;

		default:
			printk("Storage command 0x%02x not implement yet\n",msg->cmd_code);
			break;
	}
	return ret;
}


