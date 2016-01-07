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
 * @file apm_ipmi_request.h
 *
 * This file define IPMI generic response message format, 
 * and message dispatcher .
 */


#ifndef __IPMI_REQUEST_H__
#define __IPMI_REQUEST_H__

#include <linux/kernel.h> /* For printk. */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <linux/jiffies.h>
#include <linux/ipmi_msgdefs.h>     /* for completion codes */

	
typedef struct ipmi_request_message {
	unsigned char net_fn;
	unsigned char cmd_code;
	unsigned char data[25];
	int data_len;
}REQ_MSG;

typedef struct ipmi_generic_response {
	unsigned char completion_code;
} IPMI_GENERIC_RESP;

int ipmi_request_dispatcher(REQ_MSG *msg, unsigned char *resp_data);


#endif //__IPMI_REQUEST_H__
