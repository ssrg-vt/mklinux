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
#include <linux/mailbox_client.h>
#include <linux/delay.h>

#include "apm_i2c_access.h"

#undef MANUAL_PROFILE
#undef FAKE

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

#ifdef FAKE
#define XGENE_SLIMPRO_I2C               "xgene-slimpro-i2c"

#define SLIMPRO_I2C_WAIT_COUNT          10000

#define SLIMPRO_OP_TO_MS                1000    /* Operation time out in ms */
#define SLIMPRO_IIC_BUS                 1

#define SMBUS_CMD_LEN                   1
#define BYTE_DATA                       1
#define WORD_DATA                       2
#define BLOCK_DATA                      3

#define SLIMPRO_IIC_I2C_PROTOCOL        0
#define SLIMPRO_IIC_SMB_PROTOCOL        1

#define SLIMPRO_IIC_READ                0
#define SLIMPRO_IIC_WRITE               1

#define IIC_SMB_WITHOUT_DATA_LEN        0       
#define IIC_SMB_WITH_DATA_LEN           1

#define SLIMPRO_DEBUG_MSG               0
#define SLIMPRO_MSG_TYPE_SHIFT          28
#define SLIMPRO_DBG_SUBTYPE_I2C1READ    4
#define SLIMPRO_DBGMSG_TYPE_SHIFT       24
#define SLIMPRO_DBGMSG_TYPE_MASK        0x0F000000U
#define SLIMPRO_IIC_DEV_SHIFT           23
#define SLIMPRO_IIC_DEV_MASK            0x00800000U
#define SLIMPRO_IIC_DEVID_SHIFT         13
#define SLIMPRO_IIC_DEVID_MASK          0x007FE000U
#define SLIMPRO_IIC_RW_SHIFT            12
#define SLIMPRO_IIC_RW_MASK             0x00001000U
#define SLIMPRO_IIC_PROTO_SHIFT         11
#define SLIMPRO_IIC_PROTO_MASK          0x00000800U
#define SLIMPRO_IIC_ADDRLEN_SHIFT       8
#define SLIMPRO_IIC_ADDRLEN_MASK        0x00000700U
#define SLIMPRO_IIC_DATALEN_SHIFT       0
#define SLIMPRO_IIC_DATALEN_MASK        0x000000FFU

/*
 * SLIMpro I2C message encode
 *
 * dev          - Controller number (0-based)
 * chip         - I2C chip address
 * op           - SLIMPRO_IIC_READ or SLIMPRO_IIC_WRITE
 * proto        - SLIMPRO_IIC_SMB_PROTOCOL or SLIMPRO_IIC_I2C_PROTOCOL
 * addrlen      - Length of the address field
 * datalen      - Length of the data field
 */
#define SLIMPRO_IIC_ENCODE_MSG(dev, chip, op, proto, addrlen, datalen) \
        ((SLIMPRO_DEBUG_MSG << SLIMPRO_MSG_TYPE_SHIFT) | \
        ((SLIMPRO_DBG_SUBTYPE_I2C1READ << SLIMPRO_DBGMSG_TYPE_SHIFT) & \
        SLIMPRO_DBGMSG_TYPE_MASK) | \
        ((dev << SLIMPRO_IIC_DEV_SHIFT) & SLIMPRO_IIC_DEV_MASK) | \
        ((chip << SLIMPRO_IIC_DEVID_SHIFT) & SLIMPRO_IIC_DEVID_MASK) | \
        ((op << SLIMPRO_IIC_RW_SHIFT) & SLIMPRO_IIC_RW_MASK) | \
        ((proto << SLIMPRO_IIC_PROTO_SHIFT) & SLIMPRO_IIC_PROTO_MASK) | \
        ((addrlen << SLIMPRO_IIC_ADDRLEN_SHIFT) & SLIMPRO_IIC_ADDRLEN_MASK) | \
        ((datalen << SLIMPRO_IIC_DATALEN_SHIFT) & SLIMPRO_IIC_DATALEN_MASK))

#define SLIMPRO_IIC_ENCODE_FLAG_BUFADDR 0x80000000
#define SLIMPRO_IIC_ENCODE_FLAG_WITH_DATA_LEN(a)        ((u32) (((a) << 30) & 0x40000000))
#define SLIMPRO_IIC_ENCODE_UPPER_BUFADDR(a) ((u32) (((a) >> 12) & 0x3FF00000))
#define SLIMPRO_IIC_ENCODE_ADDR(a)      ((a) & 0x000FFFFF)

struct slimpro_i2c_dev {
        struct i2c_adapter adapter;
        struct device *dev;
        struct mbox_chan *mbox_chan;
        struct mbox_client mbox_client;
        struct completion rd_complete;
        spinlock_t lock;
        int i2c_rx_poll;
        int i2c_tx_poll;
        u8 dma_buffer[I2C_SMBUS_BLOCK_MAX]; /* temp R/W data buffer */
        u32 *resp_msg;
};

#define to_slimpro_i2c_dev(cl)  \
                container_of(cl, struct slimpro_i2c_dev, mbox_client)

static int apm_start_i2c_msg_xfer(struct slimpro_i2c_dev *ctx)
{
        int rc;
#ifdef MANUAL_PROFILE
	unsigned long atime[5];
#endif

        if (ctx->mbox_client.tx_block) {
#ifdef MANUAL_PROFILE
		atime[0] = ktime_to_ns(ktime_get());
#endif
                rc = wait_for_completion_timeout(&ctx->rd_complete,
                                                 msecs_to_jiffies
                                                 (SLIMPRO_OP_TO_MS));
#ifdef MANUAL_PROFILE
		atime[1] = ktime_to_ns(ktime_get());
		printk("%s: wait completion %ld\n", __func__, (atime[1] - atime[0]));
#endif
                if (rc == 0)
                        return -EIO;
                rc = 0;
        } else {
                int count;
                unsigned long flags;

#ifdef MANUAL_PROFILE
                atime[0] = ktime_to_ns(ktime_get());
#endif
                spin_lock_irqsave(&ctx->lock, flags);
#ifdef MANUAL_PROFILE
                atime[1] = ktime_to_ns(ktime_get());
#endif
                ctx->i2c_rx_poll = 1;
                for (count = SLIMPRO_I2C_WAIT_COUNT; count > 0; count--) {
                        if (ctx->i2c_rx_poll == 0)
                                break;
                        udelay(100);
                }
#ifdef MANUAL_PROFILE
                atime[1] = ktime_to_ns(ktime_get());
#endif

                if (count == 0) {
                        ctx->i2c_rx_poll = 0;
                        ctx->mbox_client.tx_block = true;
                        spin_unlock_irqrestore(&ctx->lock, flags);
                        return -EIO;
                }
#ifdef MANUAL_PROFILE
                atime[3] = ktime_to_ns(ktime_get());
#endif

                rc = 0;
                ctx->mbox_client.tx_block = true;
                spin_unlock_irqrestore(&ctx->lock, flags);
#ifdef MANUAL_PROFILE
                atime[4] = ktime_to_ns(ktime_get());
		printk("%s: poll lock %ld spin %ld mngmnt %ld unlock %ld\n",
                        __func__, (atime[1] - atime[0]), (atime[2] - atime[1]),
			(atime[3] - atime[2]), (atime[4] - atime[3]));
#endif
        }

        /* Check of invalid data or no device */
        if (*ctx->resp_msg == 0xffffffff)
                rc = -ENODEV;

        return rc;
}

static int apm_slimpro_i2c_rd(struct slimpro_i2c_dev *ctx, u32 chip,
                          u32 addr, u32 addrlen, u32 protocol, u32 readlen,
                          u32 * data)
{
        u32 msg[3];
        int rc;
#ifdef MANUAL_PROFILE
	unsigned long atime[4];
#endif

        if (irqs_disabled())
                ctx->mbox_client.tx_block = false;

        msg[0] = SLIMPRO_IIC_ENCODE_MSG(SLIMPRO_IIC_BUS, chip,
                                        SLIMPRO_IIC_READ, protocol, addrlen,
                                        readlen);
        msg[1] = SLIMPRO_IIC_ENCODE_ADDR(addr);
        msg[2] = 0;
        ctx->resp_msg = data;
#ifdef MANUAL_PROFILE
	atime[0] = ktime_to_ns(ktime_get());
#endif
        if (ctx->mbox_client.tx_block)
                init_completion(&ctx->rd_complete);

#ifdef MANUAL_PROFILE
        atime[1] = ktime_to_ns(ktime_get());
#endif
        rc = mbox_send_message(ctx->mbox_chan, &msg);
        if (rc < 0)
                goto err;

#ifdef MANUAL_PROFILE
        atime[2] = ktime_to_ns(ktime_get());
#endif
        rc = apm_start_i2c_msg_xfer(ctx);
#ifdef MANUAL_PROFILE
        atime[3] = ktime_to_ns(ktime_get());
	printk("%s: init %ld mbox_send %ld xfer %ld\n",
                        __func__, (atime[1] - atime[0]), (atime[2] - atime[1]), (atime[3] - atime[2]));
#endif
err:
        ctx->resp_msg = NULL;
        return rc;
}
#endif /* FAKE */

s32 apm_i2c_smbus_xfer(struct i2c_adapter *adapter, u16 addr, unsigned short flags,
                   char read_write, u8 command, int protocol,
                   union i2c_smbus_data *data)
{
        unsigned long orig_jiffies;
        int try;
        s32 res;
#ifdef MANUAL_PROFILE
	unsigned long atime[2];
#endif

        flags &= I2C_M_TEN | I2C_CLIENT_PEC | I2C_CLIENT_SCCB;

        if (adapter->algo->smbus_xfer) {
                i2c_lock_adapter(adapter);

#ifdef MANUAL_PROFILE
		atime[0] = ktime_to_ns(ktime_get());
#endif
                /* Retry automatically on arbitration loss */
                orig_jiffies = jiffies;
                for (res = 0, try = 0; try <= adapter->retries; try++) {
#ifndef FAKE
                        res = adapter->algo->smbus_xfer(adapter, addr, flags,
                                                        read_write, command,
                                                        protocol, data); // (u32 *)data);
#else
#error "This implementation is wrong. Was done for testing purposes only."
			res = apm_slimpro_i2c_rd((struct slimpro_i2c_dev*)adapter, addr, flags,
                                                        read_write, command,
                                                        protocol, (u32 *)data);
#endif
                        if (res != -EAGAIN)
                                break;
                        if (time_after(jiffies,
                                       orig_jiffies + adapter->timeout))
                                break;
                }
#ifdef MANUAL_PROFILE
		atime [1] = ktime_to_ns(ktime_get());
#endif
                i2c_unlock_adapter(adapter);
#ifdef MANUAL_PROFILE
//printk("%s: retries %d\n", __func__, try); //usually 0
		printk("%s: xfer %ld retries %d (%pB)\n", 
			__func__, (atime[1] - atime[0]), try, adapter->algo->smbus_xfer);
#endif

                if (res != -EOPNOTSUPP || !adapter->algo->master_xfer)
                        return res;
                /*
                 * Fall back to i2c_smbus_xfer_emulated if the adapter doesn't
                 * implement native support for the SMBus operation.
                 */
        }

printk("%s: wants emulated\n", __func__); //it doesn't require emulation
        return i2c_smbus_xfer(adapter, addr, flags, read_write, command, protocol, data);
//i2c_smbus_xfer_emulated(adapter, addr, flags, read_write, command, protocol, data);
}

int i2c_sensor_read(int i2c_bus, unsigned char slave_addr,
		    unsigned char offset, int readlen, u32 *data)
{
	struct i2c_adapter *i2c_adap;
	union i2c_smbus_data i2c_data;
	int rc;
#ifdef MANUAL_PROFILE
	unsigned long atime[4];

	atime[0] = ktime_to_ns(ktime_get());
#endif
	i2c_adap = i2c_get_adapter(i2c_bus);
	if (!i2c_adap) {
		printk("%s: No adapter found\n", __FUNCTION__);
		return -ENODEV;
	}
#ifdef MANUAL_PROFILE
	atime[1] = ktime_to_ns(ktime_get());
#endif
	rc = apm_i2c_smbus_xfer(i2c_adap, slave_addr, 0, I2C_SMBUS_READ,
			    offset, readlen == 1 ? I2C_SMBUS_BYTE_DATA :
			    I2C_SMBUS_WORD_DATA,&i2c_data);
#ifdef MANUAL_PROFILE
	atime[2] = ktime_to_ns(ktime_get());
#endif
	i2c_put_adapter(i2c_adap);
#ifdef MANUAL_PROFILE
	atime[3] = ktime_to_ns(ktime_get());
	printk("%s: get %ld xfer %ld put %ld\n",
		__func__, (atime[1] - atime[0]), (atime[2] - atime[1]), (atime[3] - atime[2]));
#endif
	if (rc)
		return rc;

	if (readlen == 1)
		*data = (u32)i2c_data.byte;
	else
		*data = (u32)i2c_data.word;

	return rc;
}

int i2c_sensor_read_any(int i2c_bus, unsigned char slave_addr,
                       unsigned char offset, int readlen, u32 *data)
{
        struct i2c_adapter *i2c_adap;
	union i2c_smbus_data i2c_data;
        int rc;

	if (readlen < 1)
		return -1;

#ifdef MANUAL_PROFILE
        unsigned long atime[4];

        atime[0] = ktime_to_ns(ktime_get());
#endif
        i2c_adap = i2c_get_adapter(i2c_bus);
        if (!i2c_adap) {
                printk("%s: No adapter found\n", __FUNCTION__);
                return -ENODEV;
        }
#ifdef MANUAL_PROFILE
        atime[1] = ktime_to_ns(ktime_get());
#endif
        rc = apm_i2c_smbus_xfer(i2c_adap, slave_addr, 0, I2C_SMBUS_READ,
                            offset, readlen == 1 ? I2C_SMBUS_BYTE_DATA : 
			    (readlen == 2 ? I2C_SMBUS_WORD_DATA : I2C_SMBUS_BLOCK_DATA),
			&i2c_data);
#ifdef MANUAL_PROFILE
        atime[2] = ktime_to_ns(ktime_get());
#endif
        i2c_put_adapter(i2c_adap);
#ifdef MANUAL_PROFILE
        atime[3] = ktime_to_ns(ktime_get());
        printk("%s: get %ld xfer %ld put %ld\n",
                __func__, (atime[1] - atime[0]), (atime[2] - atime[1]), (atime[3] - atime[2]));
#endif
        if (rc)
                return rc;

        if (readlen == 1)
                *data = (u32)i2c_data.byte;
	else if (readlen == 2)
                *data = (u32)i2c_data.word;
        else 
		if ( readlen < I2C_SMBUS_BLOCK_MAX )
			memcpy((void*)data, (void*)&i2c_data, readlen);
		else
		    memcpy((void*)data, (void*)&i2c_data, I2C_SMBUS_BLOCK_MAX);

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
