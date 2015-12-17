/*
 * X-Gene SLIMPRO KCS Driver
 *
 * Copyright (c) 2014, Applied Micro Circuits Corporation
 * Author: Hieu Le <hnle@apm.com>
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * This driver provides support for X-Gene Slimpro I2C device access
 * using the APM X-Gene SlimPRO driver.
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mailbox_client.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <misc/xgene/slimpro/xgene_slimpro_ipmi_kcs.h>

#define SLIMPRO_XGENE_WAIT_COUNT	10000
#define SLIMPRO_OP_TO_MS		1000	/* Operation time out in ms */

#define SLIMPRO_DEBUG_MSG		0
#define SLIMPRO_MSG_TYPE_MASK		0xF0000000U
#define SLIMPRO_MSG_TYPE_SHIFT		28
#define SLIMPRO_DBG_SUBTYPE_BMC_MSG	0xE
#define SLIMPRO_DBGMSG_TYPE_SHIFT	24
#define SLIMPRO_DBGMSG_TYPE_MASK	0x0F000000U
#define SLIMPRO_MSG_CONTROL_BYTE_MASK	0x00FF0000U
#define SLIMPRO_MSG_CONTROL_BYTE_SHIFT	16
#define SLIMPRO_DBGMSG_P0_MASK              0x0000FF00U
#define SLIMPRO_DBGMSG_P0_SHIFT		8
#define SLIMPRO_DBGMSG_P1_MASK		0x000000FFU
#define SLIMPRO_DBGMSG_P1_SHIFT		0

#define SLIMPRO_DECODE_MSG_TYPE(data)		((data & SLIMPRO_MSG_TYPE_MASK) >> SLIMPRO_MSG_TYPE_SHIFT)
#define SLIMPRO_DECODE_DBGMSG_TYPE(data)	((data & SLIMPRO_DBGMSG_TYPE_MASK) >> SLIMPRO_DBGMSG_TYPE_SHIFT)
#define DECODE_BMC_REG_OFFSET(msg_val)		((msg_val >> 8) & 0xff)
#define DECODE_BMC_ACCESS_INFO(msg_val)		((msg_val) & 0xff)
#define DECODE_BMC_ACCESS_INFO_IS_READ(access_info)	 (((access_info) & 0x08) == 0x08)
#define DECODE_BMC_ACCESS_INFO_IS_WRITE(access_info)	 (((access_info) & 0x08) == 0)
#define DECODE_BMC_ACCESS_INFO_LEN(access_info)		 (((access_info) & 0xf0) >> 4)
#define DECODE_BMC_ACCESS_INFO_MORE_DATA(access_info)	 (((access_info) & 0x04) >> 2)

#define SLIMPRO_ENCODE_BMC_ACCESS_INFO(len, is_read, more_data) \
                (((len & 0xf) << 4) | ((is_read & 0x1)<< 3) | \
		((more_data & 0x1) << 2))

#define SLIMPRO_ENCODE_DEBUG_MSG(type,cb,p0,p1)	((SLIMPRO_DEBUG_MSG << SLIMPRO_MSG_TYPE_SHIFT) | \
                                                ((type << SLIMPRO_DBGMSG_TYPE_SHIFT) & SLIMPRO_DBGMSG_TYPE_MASK) | \
                                                ((cb << SLIMPRO_MSG_CONTROL_BYTE_SHIFT) & SLIMPRO_MSG_CONTROL_BYTE_MASK) | \
                                                ((p0 << SLIMPRO_DBGMSG_P0_SHIFT) & SLIMPRO_DBGMSG_P0_MASK) | \
                                                ((p1 << SLIMPRO_DBGMSG_P1_SHIFT) & SLIMPRO_DBGMSG_P1_MASK))

struct slimpro_xgene_kcs_dev {
	struct mbox_chan *mbox_chan;
	struct mbox_client mbox_client;
	spinlock_t lock;
	int xgene_kcs_rx_poll;
	int xgene_kcs_tx_poll;

	u32 resp_msg[3];
};

static struct slimpro_xgene_kcs_dev *slimpro_xgene_kcs_ctx;

#define to_slimpro_xgene_kcs(cl)	\
		container_of(cl, struct slimpro_xgene_kcs_dev, mbox_client)

static void slimpro_xgene_kcs_rx_cb(struct mbox_client *cl, void *msg)
{
	struct slimpro_xgene_kcs_dev *ctx = to_slimpro_xgene_kcs(cl);

	/*
	 * Response message format:
	 * mssg[0] is the return code of the operation
	 * mssg[1] is the first data word
	 * mssg[2] is NOT used
	 *
	 * As we only support byte and word size, just assign it.
	 */
	ctx->resp_msg[0] = ((u32*)msg)[0];
	ctx->resp_msg[1] = ((u32*)msg)[1];
	ctx->resp_msg[2] = ((u32*)msg)[2];

	ctx->xgene_kcs_rx_poll = 0;
}

static int xgene_kcs_rd(struct slimpro_xgene_kcs_dev *ctx, u32* msg, u8* data, const int len)
{
	int rc, count;
	unsigned long flags;
	u8 access_info = 0, ret_len = 0;

	rc = mbox_send_message(ctx->mbox_chan, msg);
	if (rc < 0)
		return -EIO;

	spin_lock_irqsave(&ctx->lock, flags);
	ctx->xgene_kcs_rx_poll = 1;
	for (count = SLIMPRO_XGENE_WAIT_COUNT; count > 0; count--) {
		if (ctx->xgene_kcs_rx_poll == 0)
			break;
		udelay(100);
	}

	if (count == 0) {
		ctx->xgene_kcs_rx_poll = 0;
		goto err;		
	} 

	access_info = DECODE_BMC_ACCESS_INFO(ctx->resp_msg[0]);
	if (SLIMPRO_DECODE_MSG_TYPE(ctx->resp_msg[0]) != SLIMPRO_DEBUG_MSG ||
			SLIMPRO_DECODE_DBGMSG_TYPE(ctx->resp_msg[0]) != SLIMPRO_DBG_SUBTYPE_BMC_MSG
			|| DECODE_BMC_ACCESS_INFO_IS_WRITE(access_info) ||
			(ret_len = DECODE_BMC_ACCESS_INFO_LEN(access_info)) == 0)
		goto err;

	memcpy(&data[0], (u8 *)&ctx->resp_msg[1], 4);
	if (ret_len > 4)
		memcpy(&data[4], (u8 *)&ctx->resp_msg[2], 4);

	spin_unlock_irqrestore(&ctx->lock, flags);

	return ret_len > len ? len : ret_len;

err:
	spin_unlock_irqrestore(&ctx->lock, flags);
	return -EIO;
}

static void xgene_kcs_wr(struct slimpro_xgene_kcs_dev *ctx, u32* msg)
{
	int rc, count;
	unsigned long flags;

	rc = mbox_send_message(ctx->mbox_chan, msg);
	if (rc < 0)
		return;

	/* for non_blocking mode we need to wait for transaction */
	spin_lock_irqsave(&ctx->lock, flags);
	ctx->xgene_kcs_tx_poll = 1;
	for (count = SLIMPRO_XGENE_WAIT_COUNT; count > 0; count--) {
		if (ctx->xgene_kcs_tx_poll == 0)
			break;
		udelay(100);
	}

	if (count == 0) {
		ctx->xgene_kcs_tx_poll = 0;
		goto err;
	} 
err:
	spin_unlock_irqrestore(&ctx->lock, flags);
	return;
}

static void slimpro_xgene_kcs_tx_done(struct mbox_client *cl, void *mssg,
				enum mbox_result r)
{
	struct slimpro_xgene_kcs_dev *ctx = to_slimpro_xgene_kcs(cl);

	if (r == MBOX_OK)
		ctx->xgene_kcs_tx_poll = 0;
}

int slimpro_xgene_kcs_rd(u8 reg_offset, u8 *data, const int len)
{
	u32 msg[3] = {0};

	msg[0] = SLIMPRO_ENCODE_DEBUG_MSG(SLIMPRO_DBG_SUBTYPE_BMC_MSG, 0,
                        reg_offset, SLIMPRO_ENCODE_BMC_ACCESS_INFO(1, 1, 0));

	return xgene_kcs_rd(slimpro_xgene_kcs_ctx, msg, data, len);	
}
EXPORT_SYMBOL_GPL(slimpro_xgene_kcs_rd);

void slimpro_xgene_kcs_wr(u8 reg_offset, u8 *data, u8 len)
{
	u32 msg[3] = {0};

	memcpy((u8 *)&msg[1], &data[0], 4);
        if (len > 4)
                memcpy((u8 *)&msg[2], &data[4], 4);
	
	msg[0] = SLIMPRO_ENCODE_DEBUG_MSG(SLIMPRO_DBG_SUBTYPE_BMC_MSG, 0,
			reg_offset, SLIMPRO_ENCODE_BMC_ACCESS_INFO(len, 0, 0));

	xgene_kcs_wr(slimpro_xgene_kcs_ctx, msg);	
}
EXPORT_SYMBOL_GPL(slimpro_xgene_kcs_wr);

static int __init xgene_slimpro_kcs_probe(struct platform_device *pdev)
{
	struct slimpro_xgene_kcs_dev *ctx;
	struct mbox_client *cl;

	ctx = devm_kzalloc(&pdev->dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx) 
		return -ENOMEM;
	
	platform_set_drvdata(pdev, ctx);
	cl = &ctx->mbox_client;

	/* Request mailbox channel */
	cl->rx_callback = slimpro_xgene_kcs_rx_cb;
	cl->tx_done = slimpro_xgene_kcs_tx_done;
	cl->tx_block = false;
	cl->tx_tout = SLIMPRO_OP_TO_MS;
	cl->link_data = NULL;
	cl->knows_txdone = false;
	cl->chan_name = "slimpro_mbox:MBOX1";	
	ctx->mbox_chan = mbox_request_channel(cl);
	if (IS_ERR(ctx->mbox_chan)) {
		dev_err(&pdev->dev, "SLIMpro mailbox request failed\n");
		return PTR_ERR(ctx->mbox_chan);
	}

	/* Initialiation in case of using poll mode */
	ctx->xgene_kcs_rx_poll = 0;
	ctx->xgene_kcs_tx_poll = 0;
	spin_lock_init(&ctx->lock);

	slimpro_xgene_kcs_ctx = ctx;
	
	dev_info(&pdev->dev, "APM X-Gene SLIMpro KCS registered\n");

	return 0;
}

static int xgene_slimpro_kcs_remove(struct platform_device *pdev)
{
	struct slimpro_xgene_kcs_dev *ctx = platform_get_drvdata(pdev);

	mbox_free_channel(ctx->mbox_chan);

	return 0;
}

#define XGENE_SLIMPRO_KCS "xgene-slimpro-kcs"

static struct platform_driver xgene_slimpro_kcs_driver __refdata= {
	.probe = xgene_slimpro_kcs_probe,
	.remove = xgene_slimpro_kcs_remove,
	.driver = {
		   .name = XGENE_SLIMPRO_KCS,
		   .owner = THIS_MODULE,
		   },
};

static struct platform_device *xgene_slimpro_kcs_device;

static int __init xgene_slimpro_kcs_init(void)
{
	struct platform_device *pdev;
	int ret;

	ret = platform_driver_register(&xgene_slimpro_kcs_driver);
	if (ret < 0)
		return ret;

	pdev = platform_device_register_simple(XGENE_SLIMPRO_KCS, -1, NULL, 0);
	if (IS_ERR(pdev))
		return -ENODEV;

	xgene_slimpro_kcs_device = pdev;

	return 0;
}
subsys_initcall(xgene_slimpro_kcs_init);

static void __exit xgene_slimpro_kcs_exit(void)
{
        platform_device_unregister(xgene_slimpro_kcs_device);
        platform_driver_unregister(&xgene_slimpro_kcs_driver);
}
module_exit(xgene_slimpro_kcs_exit);


MODULE_DESCRIPTION("APM SoC X-Gene SLIMpro IPMI KCS driver");
MODULE_LICENSE("GPL v2");
