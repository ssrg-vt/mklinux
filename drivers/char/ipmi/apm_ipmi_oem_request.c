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
 * This module handles IPMI OEM messages and user defined messages.
 *
 *
 * This file process IPMI OEM messages, user defined message.
 */


#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>

#include <misc/xgene/slimpro/xgene_slimpro_pm.h>
#include "apm_ipmi_oem_request.h"
#include "apm_i2c_access.h"


#define BUILD_OEM_RESPONSE_MESSAGE(cmd,resp,resp_data) \
	resp_data[0] = IPMI_NETFN_OEM_RESPONSE << 2; \
	resp_data[1] = cmd; \
	memcpy(resp_data + 2,(unsigned char *) &resp, sizeof(resp));

#define IPMI_GET_SENSOR_DATA_CMD		0xa
#define IPMI_SET_SENSOR_DATA_CMD		0xc

#define IPMI_READ_SENSOR_DATA_NO_OFFSET_CMD	0x6
#define IPMI_WRITE_SENSOR_DATA_NO_OFFSET_CMD	0x8

#define IPMI_GET_SENSOR_DATA_WORD_CMD		0xe

int
ipmi_do_get_cpu_temperature(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_CPU_TEMPERATURE_RESP resp;
	int data0 = 0, data1 = 0, ret = 0;

	memset(&resp, 0, sizeof(IPMI_GET_CPU_TEMPERATURE_RESP));

	ret = slimpro_pm_get_tpc_status(&data0, &data1);

	resp.completion_code = 0x0;
	resp.temp_high = data0 & 0xff;
	resp.temp_average = (data0 >> 8) & 0xff;

	BUILD_OEM_RESPONSE_MESSAGE(IPMI_GET_CPU_TEMPERATURE_CMD, resp,
				   resp_data);
	return sizeof(resp) + 2;	
}

int
ipmi_do_get_sensor_data(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_SENSOR_DATA_RESP resp;
	unsigned char dev = msg->data[0];
	unsigned char offset = msg->data[1];
	unsigned char readlen = msg->data[2];
	int data = 0, ret;

	memset(&resp, 0, sizeof(IPMI_GET_SENSOR_DATA_RESP));
	ret = i2c_sensor_read(IIC_1, dev, offset, readlen, &data);
	if (ret) {
		pr_err("%s: ERROR: No IIC device found!!!\n", __func__);
	} else {
		if (readlen == 1) {
			pr_info("Sensor data: 0x%x\n", data & 0xff);
			resp.data[0] = data & 0xff;
		}
		else {
			pr_info("Sensor data: 0x%x\n", data & 0xffff);
			resp.data[0] = data & 0xff;
			resp.data[1] = (data >> 8) & 0xff;
		}
	}

	/* Build reponse message*/
	resp.completion_code = 0x0;

	BUILD_OEM_RESPONSE_MESSAGE(IPMI_GET_SENSOR_DATA_CMD, resp, resp_data);
	return sizeof(resp) + 2;
}

int
ipmi_do_set_sensor_data(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_SET_SENSOR_DATA_RESP resp;
	unsigned char dev = msg->data[0];
	unsigned char offset = msg->data[1];
	unsigned char data = msg->data[2];
	int ret;

	memset(&resp, 0, sizeof(IPMI_SET_SENSOR_DATA_RESP));
	ret = i2c_sensor_write_byte(IIC_1, dev, offset, data);
	if (ret)
		pr_err("%s: ERROR: No IIC device found !!!(%d)\n",
		       __func__, ret);

	/* Build reponse message*/
	resp.completion_code = 0x0;
	BUILD_OEM_RESPONSE_MESSAGE(IPMI_SET_SENSOR_DATA_CMD, resp, resp_data);
	return sizeof(resp) + 2;
}

int
ipmi_do_get_sensor_data_word(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_SENSOR_DATA_RESP resp;
	unsigned char dev = msg->data[0];
	unsigned char offset = msg->data[1];
	int data;

	memset(&resp, 0, sizeof(IPMI_GET_SENSOR_DATA_RESP));
	data = i2c_sensor_read_word(IIC_1, dev, offset);
	pr_info("Sensor data: 0x%x\n",data);
	resp.completion_code = 0x0;
	BUILD_OEM_RESPONSE_MESSAGE(IPMI_GET_SENSOR_DATA_WORD_CMD, resp,
				   resp_data);
	return sizeof(resp) + 2;
}

int
ipmi_do_read_sensor_data_no_offset(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_SENSOR_DATA_RESP resp;
	unsigned char dev = msg->data[0];
	unsigned char readlen = msg->data[1];
	int data = 0, ret;

	memset(&resp, 0, sizeof(IPMI_GET_SENSOR_DATA_RESP));
	ret = i2c_sensor_read_no_offset(IIC_1, dev, readlen, &data);
	if (ret) {
		pr_err("%s: ERROR: No IIC device found!!!(%d)\n",
		       __func__, ret);
	} else {
		if (readlen == 1)
			pr_info("Sensor data: 0x%x\n", data & 0xff);
		else
			pr_info("Sensor data: 0x%x\n", data & 0xffff);
	}

	/* Build reponse message */
	resp.completion_code = 0x0;

	BUILD_OEM_RESPONSE_MESSAGE(IPMI_READ_SENSOR_DATA_NO_OFFSET_CMD, resp,
				   resp_data);
	return sizeof(resp) + 2;
}

int
ipmi_do_write_sensor_data_no_offset(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_SET_SENSOR_DATA_RESP resp;
	unsigned char dev = msg->data[0];
	unsigned char write_data = msg->data[1];
	int ret;

	memset(&resp, 0, sizeof(IPMI_SET_SENSOR_DATA_RESP));
	ret = i2c_sensor_write_no_offset(IIC_1, dev, write_data);
	if (ret)
		pr_err("%s: ERROR: No IIC device found !!!(%d)\n",
		       __func__, ret);

	/* Build reponse message */
	resp.completion_code = 0x0;

	BUILD_OEM_RESPONSE_MESSAGE(IPMI_WRITE_SENSOR_DATA_NO_OFFSET_CMD, resp,
				   resp_data);
	return sizeof(resp) + 2;
}



int
ipmi_do_get_cpu_version(REQ_MSG *msg, unsigned char *resp_data)
{
	IPMI_GET_CPU_PART_INFO_RESP resp;
#if 0
	unsigned char dev = msg->data[0];
	unsigned char offset = msg->data[1];
	struct apm88xxxx_part_info *info;
#endif
	memset(&resp, 0, sizeof(IPMI_GET_CPU_PART_INFO_RESP));
#if 0
	info = (struct apm88xxxx_part_info * )apm88xxxx_get_partinfo();

	resp.completion_code = 0x0;
	//memcpy(&resp + 1, info,sizeof(IPMI_GET_CPU_PART_INFO_RESP) - 1 );

	resp.category = info->category;
	resp.group = info->group;
	memcpy(resp.part_name, info->part_name,20);
	resp.part_id = info->part_number;     /* Part Identity */
	resp.cores =  info->cores;       /* # of core as per spec */

	resp.freq_ls = info->freq & 0xff;
	resp.freq_ms = (info->freq >> 8) & 0xff;

	resp.t_boost_ls = info->t_boost && 0xff;        /* Max frequency in MHz */
	resp.t_boost_ms = (info->t_boost >> 8) & 0xff;
	resp.l3_cache_ls = info->l3_cache && 0xff;
	resp.l3_cache_ms = (info->l3_cache >> 8 ) & 0xff;

	resp.mem_chan_ls = info->mem_chan & 0xff;       /* Max # of DDR channels */
	resp.mem_chan_ms = (info->mem_chan >> 8) & 0xff;       /* Max # of DDR channels */

	resp.ddr_frq_ls = info->ddr_frq & 0xff;        /* Max DDR frequency */
	resp.ddr_frq_ms = (info->ddr_frq >> 8) & 0xff;        /* Max DDR frequency */
	resp.enet_ports = info->enet_ports;      /* Ethernet ports (10GE xfi OR 1GE SGMII) */
	resp.enet_1g_ports = info->enet_1g_ports;   /* Ethernet ports (1GE) */
	resp.sata_enet_ports = info->sata_enet_ports; /* SATA or 1GE enet */
	resp.sata_ports = info->sata_ports;
	resp.pcie_x4_ports = info->pcie_x4_ports;
	resp.pcie_x8_ports = info->pcie_x8_ports;
	resp.pcie_x1_ports = info->pcie_x1_ports;
	resp.usb_ports = info->usb_ports;
	resp.sdio = info->sdio;        /* SD slot available */
	resp.spi = info->spi;         /* SPI slot savailable */
	resp.i2c = info->i2c;         /* I2C available */
	resp.ebus_nand = info->ebus_nand;       /* EBUS available */
	resp.mslim = info->mslim;       /* MSLIM available */
	resp.dma = info->dma;         /* DMA available */
	resp.crypto_EIP_96 = info->crypto_EIP_96;   /* Crypto available */
	resp.crypto_EIP_62 = info->crypto_EIP_62;   /* Crypto available */
	resp.tmm_sec_boot = info->tmm_sec_boot;    /* Secure boot available */
	resp.deep_sleep = info->deep_sleep;

	printk("CPU Information:\n");
	printk("\tCategory: %d\n",resp.category);
	printk("\tGroup: %d\n",resp.group);
	printk("\tName:%s\n",resp.part_name);
	printk("\tID:%d\n",resp.part_id);
	printk("\tCores: %d\n",resp.cores);
	printk("\tFrequency(MHz)  :%d\n",resp.freq_ls | (resp.freq_ms << 8));
	printk("\tBoots: %d\n",resp.t_boost_ls | (resp.t_boost_ms << 8));
	printk("\tL3 Cache(MB): %d\n",resp.l3_cache_ls | (resp.l3_cache_ms << 8));
	printk("\tMEM Channel: %d\n",resp.mem_chan_ls | (resp.mem_chan_ms << 8));
	printk("\tDDR Frequency: %d\n",resp.ddr_frq_ls | (resp.ddr_frq_ms << 8));
	printk("\tEthernet Ports:%d\n",resp.enet_ports);
	printk("\tEthernet 1G Ports:%d\n",resp.enet_1g_ports);
	printk("\tSATA/Enet shared:%s\n",(resp.sata_enet_ports) ? "SATA" : "ENET"); //FIXME: 1 is SATA ???
	printk("\tSATA ports:%d\n",resp.sata_ports);
	printk("\tPCIE ports(x4-x8-x1): %d - %d - %d\n",resp.pcie_x4_ports,resp.pcie_x8_ports,resp.pcie_x1_ports);
	printk("\tUSB ports:%d\n",resp.usb_ports);
	printk("\tSDIO: %d\n",resp.sdio);
	printk("\tSPI:%d\n",resp.spi);
	printk("\tI2C:%d\n",resp.i2c);
	printk("\tEBUS NAND: %d\n",resp.ebus_nand);
	printk("\tMSLIM: %d\n",resp.mslim);
	printk("\tDMA: %d\n",resp.dma);
	printk("\tCrypto EIP (96 - 62): %d - %d\n",resp.crypto_EIP_96,resp.crypto_EIP_62);
	printk("\tSecure boot:%d\n",resp.tmm_sec_boot);
	printk("\tDeep Sleep: %d\n",resp.deep_sleep);
#endif
	BUILD_OEM_RESPONSE_MESSAGE(IPMI_GET_CPU_VERSION_CMD, resp,
				   resp_data);
	return sizeof(resp) + 2;
}


int
ipmi_process_oem_request(REQ_MSG *msg, unsigned char *resp_data)
{
	int ret = 0;
	switch (msg->cmd_code) {
	case IPMI_GET_CPU_TEMPERATURE_CMD:
		ret = ipmi_do_get_cpu_temperature(msg, resp_data);
		break;
	case IPMI_GET_SENSOR_DATA_CMD:
		ret = ipmi_do_get_sensor_data(msg, resp_data);
		break;
	case IPMI_SET_SENSOR_DATA_CMD:
		ret = ipmi_do_set_sensor_data(msg, resp_data);
		break;
	case IPMI_GET_SENSOR_DATA_WORD_CMD:
		ret = ipmi_do_get_sensor_data_word(msg, resp_data);
		break;
	case IPMI_READ_SENSOR_DATA_NO_OFFSET_CMD:
		ret = ipmi_do_read_sensor_data_no_offset(msg, resp_data);
		break;
	case IPMI_WRITE_SENSOR_DATA_NO_OFFSET_CMD:
		ret = ipmi_do_write_sensor_data_no_offset(msg, resp_data);
		break;
	case IPMI_GET_CPU_VERSION_CMD:
		ret = ipmi_do_get_cpu_version(msg, resp_data);
		break;
	default:
		pr_err("OEM command 0x%02x not support yet\n", msg->cmd_code);
		break;
	}
	return ret;
}

