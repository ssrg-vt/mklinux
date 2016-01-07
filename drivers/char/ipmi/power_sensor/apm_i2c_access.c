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
 * @file apm_i2c_access.c
 *
 * This file do IIC access interface to physical sensor device.
 */



#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include "apm_i2c_access.h"

unsigned char i2c_sensor_read_byte(int i2c_bus, unsigned char slave_addr,
				   unsigned char offset)
{
	struct i2c_adapter *i2c_adap;
	union i2c_smbus_data i2c_data;
	int rc;

	i2c_adap = i2c_get_adapter(i2c_bus);
	if (!i2c_adap)
		return 0;

	rc = i2c_smbus_xfer(i2c_adap, slave_addr, 0, I2C_SMBUS_READ,
			    offset, I2C_SMBUS_BYTE_DATA, &i2c_data);
	i2c_put_adapter(i2c_adap);
	if (rc)
		return rc;

	return i2c_data.byte & 0xff;
}

int i2c_sensor_read(int i2c_bus, unsigned char slave_addr,
		    unsigned char offset, int readlen, u32 *data)
{
	struct i2c_adapter *i2c_adap;
	union i2c_smbus_data i2c_data;
	int rc;

	i2c_adap = i2c_get_adapter(i2c_bus);
	if (!i2c_adap) {
		printk("%s: No adapter found\n", __FUNCTION__);
		return -ENODEV;
	}
	rc = i2c_smbus_xfer(i2c_adap, slave_addr, 0, I2C_SMBUS_READ,
			    offset, readlen == 1 ? I2C_SMBUS_BYTE_DATA :
			    I2C_SMBUS_WORD_DATA,&i2c_data);
	i2c_put_adapter(i2c_adap);

	if (rc)
		return rc;

	if (readlen == 1)
		*data = (u32)i2c_data.byte;
	else
		*data = (u32)i2c_data.word;

	return rc;
}

int i2c_sensor_read_no_offset(int i2c_bus, unsigned char slave_addr,
			      int readlen, u32 *data)
{
	struct i2c_adapter *i2c_adap;
	union i2c_smbus_data i2c_data;
	int rc;

	i2c_adap = i2c_get_adapter(i2c_bus);
	if (!i2c_adap)
		return -ENODEV;

	rc = i2c_smbus_xfer(i2c_adap, slave_addr, 0, I2C_SMBUS_READ,
			    0, readlen == 1 ? I2C_SMBUS_BYTE_DATA :
					      I2C_SMBUS_WORD_DATA,
			    &i2c_data);
	i2c_put_adapter(i2c_adap);
	if (rc)
		return rc;

	if (readlen == 1)
		*data = (u32)i2c_data.byte;
	else
		*data = (u32)i2c_data.word;

	return rc;
}


int
i2c_sensor_write_byte(int i2c_bus, unsigned char slave_addr,
		      unsigned char offset, unsigned char data)
{
	struct i2c_adapter *i2c_adap;
	union i2c_smbus_data i2c_data;
	int rc;

	i2c_adap = i2c_get_adapter(i2c_bus);
	if (!i2c_adap) {
		printk("%s: No adapter \n", __FUNCTION__);
		return -ENODEV;
	}

	i2c_data.byte = data;
	rc = i2c_smbus_xfer(i2c_adap, slave_addr, 0, I2C_SMBUS_WRITE,
			    offset, I2C_SMBUS_BYTE_DATA, &i2c_data);
	i2c_put_adapter(i2c_adap);

	return rc;
}
int
i2c_sensor_write_word(int i2c_bus, unsigned char slave_addr,
		      unsigned char offset, unsigned int data)
{
	struct i2c_adapter *i2c_adap;
	union i2c_smbus_data i2c_data;
	int rc;

	i2c_adap = i2c_get_adapter(i2c_bus);
	if (!i2c_adap)
		return -ENODEV;

	i2c_data.word = data;
	rc = i2c_smbus_xfer(i2c_adap, slave_addr, 0, I2C_SMBUS_WRITE,
			    offset, I2C_SMBUS_WORD_DATA, &i2c_data);
	i2c_put_adapter(i2c_adap);

	return rc;
}

int
i2c_sensor_write_no_offset(int i2c_bus, unsigned char slave_addr,
			   unsigned char data)
{
	struct i2c_adapter *i2c_adap;
	union i2c_smbus_data i2c_data;
	int rc;

	i2c_adap = i2c_get_adapter(i2c_bus);
	if (!i2c_adap)
		return -ENODEV;
 
	i2c_data.byte = data;
	rc = i2c_smbus_xfer(i2c_adap, slave_addr, 0, I2C_SMBUS_WRITE,
			    0, I2C_SMBUS_BYTE_DATA, &i2c_data);
	i2c_put_adapter(i2c_adap);
	return rc;
}


unsigned int 
i2c_sensor_read_word(int i2c_bus, unsigned char slave_addr,
		     unsigned char offset)
{
	struct i2c_adapter *i2c_adap;
	union i2c_smbus_data i2c_data;
	int rc;

	i2c_adap = i2c_get_adapter(i2c_bus);
	if (!i2c_adap)
		return 0;

	rc = i2c_smbus_xfer(i2c_adap, slave_addr, 0, I2C_SMBUS_READ,
			    offset, I2C_SMBUS_WORD_DATA, &i2c_data);
	i2c_put_adapter(i2c_adap);

	if (rc)
		return 0;
 
	return	i2c_data.word;
}
