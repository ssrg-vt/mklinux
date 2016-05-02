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
 * @file apm_i2c_access.h
 *
 * This file define IIC access interface.

 */


#ifndef __I2C_SENSOR_H__
#define __I2C_SENSOR_H__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>

enum i2c_port {
	IIC_1=0,
};

enum IIC_READ_TYPE {
	IIC_READ_BYTE=1,
	IIC_READ_WORD
};


unsigned char i2c_sensor_read_byte(int i2c_bus, unsigned char slave_addr, unsigned char offset);
unsigned int i2c_sensor_read_word(int i2c_bus, unsigned char slave_addr, unsigned char offset);
int i2c_sensor_read(int i2c_bus, unsigned char slave_addr, unsigned char offset, int readlen, u32 *data);
int i2c_sensor_read_any(int i2c_bus, unsigned char slave_addr, unsigned char offset, int readlen, u32 *data);

int i2c_sensor_read_no_offset(int i2c_bus, unsigned char slave_addr, int readlen, u32 *data);

int i2c_sensor_write_no_offset(int i2c_bus, unsigned char slave_addr, unsigned char data);
int i2c_sensor_write_byte(int i2c_bus, unsigned char slave_addr, unsigned char offset,unsigned char data);
int i2c_sensor_write_word(int i2c_bus, unsigned char slave_addr, unsigned char offset, unsigned int data);



#endif //__I2C_SENSOR_H__
