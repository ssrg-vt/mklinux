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
 * @file apm_ipmi_request.c
 *
 * This file do dispatcher IPMI message to IPMI group commands.
 */

#include "apm_ipmi_request.h"
#include "apm_ipmi_chassis_request.h"
#include "apm_ipmi_apps_request.h"
#include "apm_ipmi_storage_request.h"
#include "apm_ipmi_sensor_event_request.h"
#include "apm_ipmi_oem_request.h"

int  
ipmi_request_dispatcher(REQ_MSG *msg, unsigned char *resp_data)
{

	int ret=0;

	switch (msg->net_fn)
	{
		case IPMI_NETFN_APP_REQUEST:			
			ret = ipmi_process_apps_request(msg,resp_data);
			break;
		case IPMI_NETFN_STORAGE_REQUEST:
			ret = ipmi_process_storage_request(msg,resp_data);
			break;
		case IPMI_NETFN_CHASSIS_REQUEST:
			ret = ipmi_process_chassis_request(msg,resp_data);
			break;
		case IPMI_NETFN_OEM_REQUEST:
			ret = ipmi_process_oem_request(msg,resp_data);
			break;
		case IPMI_NETFN_SENSOR_EVENT_REQUEST:
			ret = ipmi_process_sensor_event_request(msg,resp_data);
			break;
		default:
			printk("Not supported request: NetFn=0x%02x,cmd_code=0x%02x\n",msg->net_fn,msg->cmd_code);
			break;
	}

	return ret;
}

