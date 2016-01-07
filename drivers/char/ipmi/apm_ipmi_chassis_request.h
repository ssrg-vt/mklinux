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
 * @file apm_ipmi_chassis_request.h
 *
 * This file define IPMI Chassis response message format.

 */


#ifndef __IPMI_CHASSIS_REQUEST_H__
#define __IPMI_CHASSIS_REQUEST_H__
#include "apm_ipmi_request.h"

typedef struct ipmi_get_chassis_status_resp {
	unsigned char completion_code;
#ifdef __BIG_ENDIAN
	unsigned char reserved1:1,
				  power_restore_policy:2,
				  power_control_fault:1,
				  power_fault:1,
				  interlock:1,
				  power_overload:1,
				  power_is_on:1;
#else
	unsigned char power_is_on:1,
				  power_overload:1,
				  interlock:1,
				  power_fault:1,
				  power_control_fault:1,
				  power_restore_policy:2,
				  reserved1:1;
#endif
#ifdef __BIG_ENDIAN
	unsigned char reserved2:3,
				  last_power_is_on:1,
				  last_power_down_fault:1,
				  last_power_down_interlock:1,
				  last_power_down_overload:1,
				  ac_failed:1;
#else
	unsigned char ac_failed:1,
				  last_power_down_overload:1,
				  last_power_down_interlock:1,
				  last_power_down_fault:1,
				  last_power_is_on:1,
				  reserved2:3;

#endif
#ifdef __BIG_ENDIAN
	unsigned char reserved3:1,
				  identify_command_suport:1,
				  identify_state:2,
				  fan_fault:1,
				  drive_fault:1,
				  front_panel_lockout_active:1,
				  chassis_intrusion_active:1;
#else
	unsigned char chassis_intrusion_active:1,
				  front_panel_lockout_active:1,
				  drive_fault:1,
				  fan_fault:1,
				  identify_state:2,
				  identify_command_suport:1,
				  reserved3:1;

#endif
#ifdef __BIG_ENDIAN
	unsigned char standby_button:1,
				  diag_intr_button:1,
				  reset_button:1,
				  power_off_button:1,
				  standby_button_disable:1,
				  diag_button_disable:1,
				  reset_button_disable:1,
				  power_off_button_disable:1;
#else
	unsigned char power_off_button_disable:1,
				  reset_button_disable:1,
				  diag_button_disable:1,
				  standby_button_disable:1,
				  power_off_button:1,
				  reset_button:1,
				  diag_intr_button:1,
				  standby_button:1;
#endif	

} IPMI_GET_CHASSIS_STATUS_RESP;


int ipmi_process_chassis_request(REQ_MSG *msg, unsigned char *resp_data);

#endif //__IPMI_CHASSIS_REQUEST_H__
