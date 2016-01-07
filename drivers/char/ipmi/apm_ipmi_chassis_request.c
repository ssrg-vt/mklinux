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
 * @file apm_ipmi_chassis_request.c
 *
 * This file process IPMI Chassic messages.
 */


#include "apm_ipmi_chassis_request.h"

#define BUILD_CHASSIS_RESPONSE_MESSAGE(cmd,resp,resp_msg) \
		resp_msg[0] = IPMI_NETFN_CHASSIS_RESPONSE << 2; \
		resp_msg[1] = cmd; \
		memcpy(resp_msg + 2,(unsigned char *)&resp,sizeof(resp));
		
	
int
ipmi_do_get_chassis_status(unsigned char *resp_data)
{
	IPMI_GET_CHASSIS_STATUS_RESP resp;

	memset(&resp,0,sizeof(IPMI_GET_CHASSIS_STATUS_RESP));

	/*Power always on*/
	resp.completion_code = 0x0;

 	resp.power_is_on = 1;
	resp.reset_button = 1;
    

    BUILD_CHASSIS_RESPONSE_MESSAGE(
		   		IPMI_GET_CHASSIS_STATUS_CMD,
                resp,
				resp_data);

    return sizeof(resp) + 2;
}
int
ipmi_do_chassis_control(REQ_MSG *msg,unsigned char  *resp_data)
{
	int ret=0;
	switch (msg->data[0])
	{
		case 0: //power down
			printk("TODO: implement power down function\n");
			break;
		case 1: //power up
			printk("System already on\n");
			break;
		case 2: //power cycle
			printk("TODO: implement power cycle function\n");
			break;
		case 3: //hard reset
			printk("TODO: implement hard reset function\n");
			break;
		case 4: //pulse diagnostic interrupt
		case 5: //intiate soft shutdown via overtemp
		default:
			printk("Unknow control data :0x%02x\n",msg->data[0]);
			break;
	}

	return ret;
}
int
ipmi_process_chassis_request(REQ_MSG *msg,unsigned char *resp_data)
{
	int ret=0;
#if 0
	printk("%s: cmd=0x%x\n",__func__,msg->cmd_code);
#endif	
	switch (msg->cmd_code)
	{
		case IPMI_GET_CHASSIS_STATUS_CMD:
			ret = ipmi_do_get_chassis_status(resp_data);
			break;
		case IPMI_CHASSIS_CONTROL_CMD:
			ipmi_do_chassis_control(msg,resp_data);
			break;
		default:
			printk("Chassis Command 0x%02x not supported\n",msg->cmd_code);
			break;
	}

	return ret;
}

