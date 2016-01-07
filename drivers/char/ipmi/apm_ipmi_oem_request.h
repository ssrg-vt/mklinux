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
 * @file apm_ipmi_oem_request.h
 *
 * This file define IPMI OEM response message format.

 */


#ifndef __IPMI_OEM_REQUEST_H__
#define __IPMI_OEM_REQUEST_H__

#include "apm_ipmi_request.h"
/*OEM IPMI Message*/

typedef struct ipmi_get_cpu_temperature_resp {
	unsigned char completion_code;
	unsigned char temp_high;
	unsigned char temp_average;
} IPMI_GET_CPU_TEMPERATURE_RESP;

typedef struct ipmi_get_sensor_data_resp {
	unsigned char completion_code;
	unsigned char data[2];
} IPMI_GET_SENSOR_DATA_RESP;

typedef struct ipmi_set_sensor_data_resp {
	unsigned char completion_code;
} IPMI_SET_SENSOR_DATA_RESP;

typedef struct ipmi_get_cpu_version_resp {
	unsigned char completion_code;
	unsigned char data[4];
} IPMI_GET_CPU_VERSION_RESP;
typedef struct ipmi_get_cpu_part_info_resp {
	u8 completion_code;
    u8 category;
    u8 group;
    u8 part_name[20]; /* Part name */
    u8 part_id;     /* Part Identity */
    u8 cores;       /* # of core as per spec */
    u8 freq_ls;       /* Max frequency in MHz */
	u8 freq_ms;
    u8 t_boost_ls;        /* Max frequency in MHz */
	u8 t_boost_ms;
    u8 l3_cache_ls;       /* Max size in MB */
    u8 l3_cache_ms;       /* Max size in MB */
    u8 mem_chan_ls;       /* Max # of DDR channels */
    u8 mem_chan_ms;       /* Max # of DDR channels */
    u8 ddr_frq_ls;        /* Max DDR frequency */
    u8 ddr_frq_ms;        /* Max DDR frequency */
    u8 enet_ports;      /* Ethernet ports (10GE xfi OR 1GE SGMII) */
    u8 enet_1g_ports;   /* Ethernet ports (1GE) */
    u8 sata_enet_ports; /* SATA or 1GE enet */
    u8 sata_ports;
    u8 pcie_x4_ports;
    u8 pcie_x8_ports;
    u8 pcie_x1_ports;
    u8 usb_ports;
    u8 sdio;        /* SD slot available */
    u8 spi;         /* SPI slot savailable */
    u8 i2c;         /* I2C available */
    u8 ebus_nand;       /* EBUS available */
    u8 mslim;       /* MSLIM available */
    u8 dma;         /* DMA available */
    u8 crypto_EIP_96;   /* Crypto available */
    u8 crypto_EIP_62;   /* Crypto available */
    u8 tmm_sec_boot;    /* Secure boot available */
    u8 deep_sleep;
} IPMI_GET_CPU_PART_INFO_RESP;

int ipmi_process_oem_request(REQ_MSG *msg, unsigned char *resp_data);


#endif //__IPMI_OEM_REQUEST_H__
