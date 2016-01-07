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
 * @file apm_ipmi_storage_request.h
 *
 * This file define IPMI Storage response message format.

 */


#ifndef __IPMI_STORAGE_REQUEST_H__
#define __IPMI_STORAGE_REQUEST_H__

#include "apm_ipmi_request.h"
/*Storage command*/
typedef struct ipmi_get_sdr_repository_info_resp {
	unsigned char completion_code;
	unsigned char sdr_version;
	unsigned char record_count_ls;
	unsigned char record_count_ms;
	unsigned char free_space[2];
	unsigned char recent_add_time[4];
	unsigned char recent_del_time[4];
#ifdef __BIG_ENDIAN	
	unsigned char overflow:1,
			update:2,
			:1,/*reserved*/
			delete:1,
			partial:1,
			reserve:1,
			get_sdr_alloc_info:1;
#else
	unsigned char 
			get_sdr_alloc_info:1,
			reserve:1,
			partial:1,
			delete:1,
			:1,/*reserved*/
			update:2,
			overflow:1;
#endif	
} IPMI_GET_SDR_REPOSITORY_INFO_RESP;

typedef struct ipmi_get_sdr_req {
	unsigned char reservation_id_ls;
	unsigned char reservation_id_ms;
	unsigned char record_id_ls;
	unsigned char record_id_ms;
	unsigned char offset;
	unsigned char byte_to_read;
} IPMI_GET_SDR_REQ;

typedef struct ipmi_get_sdr_resp {
	unsigned char completion_code;
	unsigned char next_id_ls;
	unsigned char next_id_ms;
	unsigned char data[64];
} IPMI_GET_SDR_RESP;

typedef struct ipmi_reserve_sdr_repository_resp {
	unsigned char completion_code;
	unsigned char reservation_id_ls;
	unsigned char reservation_id_ms;
} IPMI_RESERVE_SDR_REPOSITORY_RESP;


typedef struct ipmi_clear_sdr_repository_resp {
	unsigned char completion_code;
#ifdef __BIG_ENDIAN
	unsigned char
			:4,
			erase_complete:4;
#else
	unsigned char
			erase_complete:4,
			:4;

#endif
} IPMI_CLEAR_SDR_REPOSITORY_RESP;
/******************************************/
typedef struct ipmi_get_fru_inventory_area_info_req {
	unsigned char fru_device;
} IPMI_GET_FRU_INVENTORY_AREA_INFO_REQ;

typedef struct ipmi_get_fru_inventory_area_info_resp {
	unsigned char completion_code;
	unsigned char fru_size_ls;
	unsigned char fru_size_ms;
#ifdef __BIG_ENDIAN
	unsigned char
			:7,
			access_type:1; /*0: byte, 1: word*/
#else
	unsigned char
			access_type:1,
			:7;

#endif

} IPMI_GET_FRU_INVENTORY_AREA_INFO_RESP;

typedef struct ipmi_read_fru_req {
	unsigned char fru_device;
	unsigned char read_offset_ls;
	unsigned char read_offset_ms;
	unsigned char read_count;

} IPMI_READ_FRU_REQ;

typedef struct ipmi_read_fru_resp {
	unsigned char completion_code;
	unsigned char count_returned;
	unsigned char data[25];
} IPMI_READ_FRU_RESP;

int ipmi_process_storage_request(REQ_MSG *msg, unsigned char *resp_data);

#endif
