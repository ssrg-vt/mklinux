/*
 * APM X-Gene SoC SLIMpro PM Driver
 *
 * Copyright (c) 2014, Applied Micro Circuits Corporation
 * Author: Narinder Dhillon ndhillon@apm.com
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
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mailbox_client.h>
#include <linux/err.h>
#include <misc/xgene/slimpro/xgene_slimpro_pm.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

#define SLIMPRO_PM_OP_TO_MS		1000	/* Operation time out in ms */
#define SLIMPRO_PM_WAIT_COUNT		10000

#define SLIMPRO_PM_PWRMGMT_MSG			0x9
#define SLIMPRO_PM_TPC_HDLR			1
#define SLIMPRO_PM_DAPL_HDLR			3

#define SLIMPRO_PM_MSG_TYPE_MASK		0xF0000000U
#define SLIMPRO_PM_MSG_TYPE_SHIFT		28
#define SLIMPRO_PM_PWRMGMT_MSG_HNDL_MASK	0x0F000000U
#define SLIMPRO_PM_PWRMGMT_MSG_HNDL_SHIFT	24
#define SLIMPRO_PM_MSG_CONTROL_BYTE_MASK	0x00FF0000U
#define SLIMPRO_PM_MSG_CONTROL_BYTE_SHIFT	16

/* TPC Message Definitions */
#define SLIMPRO_PM_TPC_GET_STATUS		0
#define SLIMPRO_PM_TPC_SET_STATUS		1

#define SLIMPRO_PM_TPC_MONENABLE_TM1		1
#define SLIMPRO_PM_TPC_MONDISABLE_TM1		2
#define SLIMPRO_PM_TPC_CLRLOG_TM1		3
#define SLIMPRO_PM_TPC_INTENABLE_TM1		4
#define SLIMPRO_PM_TPC_INTDISABLE_TM1		5
#define SLIMPRO_PM_TPC_CLRINT_TM1		6
#define SLIMPRO_PM_TPC_CLRLOG_INPUTTM1		7
#define SLIMPRO_PM_TPC_CLRLOG_TM2		8
#define SLIMPRO_PM_TPC_INTENABLE_ENTRYTM1	9
#define SLIMPRO_PM_TPC_INTDISABLE_ENTRYTM1	10
#define SLIMPRO_PM_TPC_CLRINT_ENTRYTM1		11
#define SLIMPRO_PM_TPC_INTENABLE_EXITTM1	12
#define SLIMPRO_PM_TPC_INTDISABLE_EXITTM1	13
#define SLIMPRO_PM_TPC_CLRINT_EXITTM1		14
#define SLIMPRO_PM_TPC_INTENABLE_ENTRYTM2	15
#define SLIMPRO_PM_TPC_INTDISABLE_ENTRYTM2	16
#define SLIMPRO_PM_TPC_CLRINT_ENTRYTM2		17
#define SLIMPRO_PM_TPC_SET_THRESHTM1		18
#define SLIMPRO_PM_TPC_SET_THRESHTM2		19

#define SLIMPRO_PM_ENCODE_TPCSTAT_MSG(cb, type) \
		((SLIMPRO_PM_PWRMGMT_MSG << SLIMPRO_PM_MSG_TYPE_SHIFT) | \
		((SLIMPRO_PM_TPC_HDLR << SLIMPRO_PM_PWRMGMT_MSG_HNDL_SHIFT) & SLIMPRO_PM_PWRMGMT_MSG_HNDL_MASK) | \
		((cb << SLIMPRO_PM_MSG_CONTROL_BYTE_SHIFT) & SLIMPRO_PM_MSG_CONTROL_BYTE_MASK) | \
		type)
#define SLIMPRO_PM_DECODE_TM1_ENABLE(data)	(((data) >> 31) & 1)
#define SLIMPRO_PM_DECODE_TM1_STAT(data)	(((data) >> 30) & 1)
#define SLIMPRO_PM_DECODE_TM1_LOG(data)		(((data) >> 29) & 1)
#define SLIMPRO_PM_DECODE_TM1_INTEN(data)	(((data) >> 28) & 1)
#define SLIMPRO_PM_DECODE_TM1_INTSTAT(data)	(((data) >> 27) & 1)
#define SLIMPRO_PM_DECODE_TM1_INPUTSTAT(data)	(((data) >> 26) & 1)
#define SLIMPRO_PM_DECODE_TM1_INPUTLOG(data)	(((data) >> 25) & 1)
#define SLIMPRO_PM_DECODE_TM2_STAT(data)	(((data) >> 24) & 1)
#define SLIMPRO_PM_DECODE_TM2_LOG(data)		(((data) >> 23) & 1)
#define SLIMPRO_PM_DECODE_TM1_ENTRY_INTEN(data)	(((data) >> 22) & 1)
#define SLIMPRO_PM_DECODE_TM1_ENTRY_INTSTAT(data)	(((data) >> 21) & 1)
#define SLIMPRO_PM_DECODE_TM1_EXIT_INTEN(data)		(((data) >> 20) & 1)
#define SLIMPRO_PM_DECODE_TM1_EXIT_INTSTAT(data)	(((data) >> 19) & 1)
#define SLIMPRO_PM_DECODE_TM2_ENTRY_INTEN(data)		(((data) >> 18) & 1)
#define SLIMPRO_PM_DECODE_TM2_ENTRY_INTSTAT(data)	(((data) >> 17) & 1)
#define SLIMPRO_PM_DECODE_AVG_TEMP(data)	(((data) >> 8) & 0xFF)
#define SLIMPRO_PM_DECODE_HIGH_TEMP(data)	((data) & 0xFF)
#define SLIMPRO_PM_DECODE_THRESH_TM1(data)	(((data) >> 24) & 0xFF)
#define SLIMPRO_PM_DECODE_THRESH_TM2(data)	(((data) >> 16) & 0xFF)
#define SLIMPRO_PM_DECODE_MSG(data)		(((data) & SLIMPRO_PM_MSG_TYPE_MASK) \
							>> SLIMPRO_PM_MSG_TYPE_SHIFT)
#define SPIMPRO_PM_DECODE_HDLR(data)		(((data) & SLIMPRO_PM_PWRMGMT_MSG_HNDL_MASK) \
						>> SLIMPRO_PM_PWRMGMT_MSG_HNDL_SHIFT)
#define SLIMPRO_TPC_EVENT			\
		((SLIMPRO_PM_PWRMGMT_MSG << SLIMPRO_PM_MSG_TYPE_SHIFT) | \
		((SLIMPRO_PM_TPC_HDLR << SLIMPRO_PM_PWRMGMT_MSG_HNDL_SHIFT) & SLIMPRO_PM_PWRMGMT_MSG_HNDL_MASK) | \
		((0x02 << SLIMPRO_PM_MSG_CONTROL_BYTE_SHIFT) & SLIMPRO_PM_MSG_CONTROL_BYTE_MASK))

/* DAPL command Definitions */
#define SLIMPRO_PM_DAPL_GET_UNIT	0
#define SLIMPRO_PM_DAPL_SET_UNIT	1
/* Bit fields for DAPLUNIT register */
#define  EU_SHIFT			0 /* Energy Unit */
#define  EU_MASK			0x1f
#define  PU_SHIFT			8 /* Power Unit */
#define  PU_MASK			0xf
#define  TU_SHIFT			16 /* Time Unit */
#define  TU_MASK			0xf
#define SLIMPRO_PM_DAPL_GET_PWRLIM	2
#define SLIMPRO_PM_DAPL_SET_PWRLIM	3
#define SLIMPRO_PM_DAPL_EN_PWRLIM	4
#define SLIMPRO_PM_DAPL_DIS_PWRLIM	5
#define SLIMPRO_PM_DAPL_GET_TIMEINT	6
#define SLIMPRO_PM_DAPL_SET_TIMEINT	7
#define SLIMPRO_PM_DAPL_GET_CUMENGY	8
#define SLIMPRO_PM_DAPL_RESET_CUMENGY	9
#define SLIMPRO_PM_DAPL_GET_CURPWR	10

#define SLIMPRO_PM_ENCODE_DAPL_MSG(cb, type) \
		((SLIMPRO_PM_PWRMGMT_MSG << SLIMPRO_PM_MSG_TYPE_SHIFT) | \
		((SLIMPRO_PM_DAPL_HDLR << SLIMPRO_PM_PWRMGMT_MSG_HNDL_SHIFT) & SLIMPRO_PM_PWRMGMT_MSG_HNDL_MASK) | \
		((cb << SLIMPRO_PM_MSG_CONTROL_BYTE_SHIFT) & SLIMPRO_PM_MSG_CONTROL_BYTE_MASK) | \
		type)

struct slimpro_pm_dev {
	struct device *dev;
	struct mbox_chan *mbox_chan;
	struct mbox_client mbox_client;
	struct completion rd_complete;
	u32 resp_msg[3];
	struct work_struct workq;
	struct blocking_notifier_head evt_notifier;
	spinlock_t lock;
	int pm_rx_poll;
};

static struct slimpro_pm_dev *slimpro_pm_ctx;

static int slimpro_pm_rd(struct slimpro_pm_dev *ctx, u32* msg)
{
	int rc;

	if (irqs_disabled()) {
		ctx->mbox_client.tx_block = false;
	}

	if (ctx->mbox_client.tx_block)
		init_completion(&ctx->rd_complete);

	rc = mbox_send_message(ctx->mbox_chan, msg);
	if (rc < 0)
		goto err;

	if (ctx->mbox_client.tx_block) {
		if (!wait_for_completion_timeout(&ctx->rd_complete,
					msecs_to_jiffies(SLIMPRO_PM_OP_TO_MS))) {
			/* Operation timed out */
			rc = -EIO;
			goto err;
		}
	} else {
		int count;
		unsigned long flags;

		spin_lock_irqsave(&ctx->lock, flags);
		ctx->pm_rx_poll = 1;
		for (count = SLIMPRO_PM_WAIT_COUNT; count > 0; count--) {
			if (ctx->pm_rx_poll == 0)
				break;
			udelay(100);
		}

		if (count == 0) {
			ctx->pm_rx_poll = 0;
			ctx->mbox_client.tx_block = true;
			rc = -EIO;
			spin_unlock_irqrestore(&ctx->lock, flags);
			goto err;
		}
		rc = 0;
		ctx->mbox_client.tx_block = true;
		spin_unlock_irqrestore(&ctx->lock, flags);
	}

	/* Check of invalid data or not device */
	if (ctx->resp_msg[0] == 0xffffffff) {
		rc = -ENODEV;
		goto err;
	}
	msg[0] = ctx->resp_msg[0];
	msg[1] = ctx->resp_msg[1];
	msg[2] = ctx->resp_msg[2];

err:
	return rc;
}

static int slimpro_pm_wr(struct slimpro_pm_dev *ctx, u32* msg)
{
	return mbox_send_message(ctx->mbox_chan, msg);
}

static ssize_t slimpro_pm_set_tpc_threshtm(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count,
					   int type)
{
	struct slimpro_pm_dev *ctx = dev_get_drvdata(dev);
	u32 msg[3];
	int rc;

	if (!count)
		return -EINVAL;

	msg[1] = simple_strtoul(buf, NULL, 0);
	if (msg[1] > 125)
		return -EINVAL;
	msg[0] = SLIMPRO_PM_ENCODE_TPCSTAT_MSG(SLIMPRO_PM_TPC_SET_STATUS,
								type);
	rc = slimpro_pm_wr(ctx, msg);
	if (rc < 0)
		return rc;
	return count;
}

static ssize_t slimpro_pm_set_tpc_threshtm1(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	return slimpro_pm_set_tpc_threshtm(dev, attr, buf, count,
					   SLIMPRO_PM_TPC_SET_THRESHTM1);
}

static ssize_t slimpro_pm_set_tpc_threshtm2(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	return slimpro_pm_set_tpc_threshtm(dev, attr, buf, count,
					   SLIMPRO_PM_TPC_SET_THRESHTM2);
}

static ssize_t slimpro_pm_set_tpc_mon(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct slimpro_pm_dev *ctx = dev_get_drvdata(dev);
	u32 msg[3];
	int rc;
	u32 val;

	if (!count)
		return -EINVAL;

	sscanf(buf, "%d", &val);
	msg[0] = SLIMPRO_PM_ENCODE_TPCSTAT_MSG(SLIMPRO_PM_TPC_SET_STATUS,
		(val ? SLIMPRO_PM_TPC_MONENABLE_TM1 :
					SLIMPRO_PM_TPC_MONDISABLE_TM1));
	rc = slimpro_pm_wr(ctx, msg);
	if (rc < 0)
		return rc;
	return count;
}

int slimpro_pm_get_tpc_status(u32 *data0, u32 *data1)
{
	u32 msg[3];
	int rc;

	msg[0] = SLIMPRO_PM_ENCODE_TPCSTAT_MSG(SLIMPRO_PM_TPC_GET_STATUS, 0);

	rc = slimpro_pm_rd(slimpro_pm_ctx, msg);

	if (rc < 0)
		goto exit;
	
	*data0 = msg[1];
	*data1 = msg[2];

exit:
	return rc;
}
EXPORT_SYMBOL_GPL(slimpro_pm_get_tpc_status);

static ssize_t slimpro_pm_get_tpc_mon(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	u32 msg[3];
	int rc;

	msg[0] = SLIMPRO_PM_ENCODE_TPCSTAT_MSG(SLIMPRO_PM_TPC_GET_STATUS, 0);

	rc = slimpro_pm_rd(dev_get_drvdata(dev), msg);
	if (rc < 0)
		return sprintf(buf, "Unavailable error %d\n", rc);

	return sprintf(buf, "%d\n",
		SLIMPRO_PM_DECODE_TM1_ENABLE(msg[1]) ? 1 : 0);
}

static ssize_t slimpro_pm_tpc_status(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	u32 msg[3];
	int rc;

	msg[0] = SLIMPRO_PM_ENCODE_TPCSTAT_MSG(SLIMPRO_PM_TPC_GET_STATUS, 0);

	rc = slimpro_pm_rd(dev_get_drvdata(dev), msg);
	if (rc < 0)
		return sprintf(buf, "Unavailable error %d\n", rc);
	return sprintf(buf,
		"TM1 Mon:%s Status:%d Log:%d Interrupt Status:%d Interrupt Enable:%d\n"
		"TM1 Input Status:%d Log:%d\n"
		"TM1 Entry Interrupt Status:%d Interrupt Enable:%d\n"
		"TM1 Exit Interrupt Status:%d Interrupt Enable:%d\n"
		"TM2 Status:%d Log:%d\n"
		"TM2 Entry Interrupt Status:%d Interrupt Enable:%d\n"
		"Avg Temperature %d High Temperature %d\n"
		"TM1 Threshold:%d TM2 Threshold:%d\n",
		SLIMPRO_PM_DECODE_TM1_ENABLE(msg[1]) ? "Enabled" : "Disabled",
		SLIMPRO_PM_DECODE_TM1_STAT(msg[1]),
		SLIMPRO_PM_DECODE_TM1_LOG(msg[1]),
		SLIMPRO_PM_DECODE_TM1_INTSTAT(msg[1]),
		SLIMPRO_PM_DECODE_TM1_INTEN(msg[1]),
		SLIMPRO_PM_DECODE_TM1_INPUTSTAT(msg[1]),
		SLIMPRO_PM_DECODE_TM1_INPUTLOG(msg[1]),
		SLIMPRO_PM_DECODE_TM1_ENTRY_INTSTAT(msg[1]),
		SLIMPRO_PM_DECODE_TM1_ENTRY_INTEN(msg[1]),
		SLIMPRO_PM_DECODE_TM1_EXIT_INTSTAT(msg[1]),
		SLIMPRO_PM_DECODE_TM1_EXIT_INTEN(msg[1]),
		SLIMPRO_PM_DECODE_TM2_STAT(msg[1]),
		SLIMPRO_PM_DECODE_TM2_LOG(msg[1]),
		SLIMPRO_PM_DECODE_TM2_ENTRY_INTSTAT(msg[1]),
		SLIMPRO_PM_DECODE_TM2_ENTRY_INTEN(msg[1]),
		SLIMPRO_PM_DECODE_AVG_TEMP(msg[1]),
		SLIMPRO_PM_DECODE_HIGH_TEMP(msg[1]),
		SLIMPRO_PM_DECODE_THRESH_TM1(msg[2]),
		SLIMPRO_PM_DECODE_THRESH_TM2(msg[2]));
}

static ssize_t slimpro_pm_get_dapl_units(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct slimpro_pm_dev *ctx = dev_get_drvdata(dev);
	u32 msg[3];
	int rc;

	msg[0] = SLIMPRO_PM_ENCODE_DAPL_MSG(0, SLIMPRO_PM_DAPL_GET_UNIT);
	rc = slimpro_pm_rd(ctx, msg);
	if (rc < 0)
		return sprintf(buf, "Unavailable error %d\n", rc);
	return sprintf(buf,
		"DAPL Time unit:1/(2^%d) sec, Power unit:1/(2^%d) W, Energy unit:1/(2^%d) J\n",
		(msg[0] >> TU_SHIFT) & TU_MASK, (msg[0] >> PU_SHIFT) & PU_MASK,
		(msg[0] >> EU_SHIFT) & EU_MASK);
}

static ssize_t slimpro_pm_set_dapl_units(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct slimpro_pm_dev *ctx = dev_get_drvdata(dev);
	char *last = NULL;
	u32 msg[3];
	u32 val[3];
	int rc;

	if (!count)
		return -EINVAL;

	val[0] = simple_strtoul(buf, &last, 0);
	val[1] = simple_strtoul(last, &last, 0);
	val[2] = simple_strtoul(last, &last, 0);

	msg[0] = SLIMPRO_PM_ENCODE_DAPL_MSG(0, SLIMPRO_PM_DAPL_SET_UNIT);
	msg[1] = ((val[0] & TU_MASK) << TU_SHIFT |
			(val[1] & PU_MASK) << PU_SHIFT |
			(val[2] & EU_MASK) << EU_SHIFT);
	rc = slimpro_pm_wr(ctx, msg);
	if (rc < 0)
		return rc;
	return count;
}

static ssize_t slimpro_pm_get_dapl_pwrlim(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	struct slimpro_pm_dev *ctx = dev_get_drvdata(dev);
	u32 msg[3];
	int rc;

	msg[0] = SLIMPRO_PM_ENCODE_DAPL_MSG(0, SLIMPRO_PM_DAPL_GET_PWRLIM);
	rc = slimpro_pm_rd(ctx, msg);
	if (rc < 0)
		return sprintf(buf, "Unavailable error %d\n", rc);
	return sprintf(buf, "DAPL Power Limit: %d\n", msg[1]);
}

static ssize_t slimpro_pm_set_dapl_pwrlim(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct slimpro_pm_dev *ctx = dev_get_drvdata(dev);
	u32 msg[3];
	int rc;

	if (!count)
		return -EINVAL;

	msg[0] = SLIMPRO_PM_ENCODE_DAPL_MSG(0, SLIMPRO_PM_DAPL_SET_PWRLIM);
	msg[1] = simple_strtoul(buf, NULL, 0);

	/* Set and enable */
	rc = slimpro_pm_wr(ctx, msg);
	if (rc < 0)
		return rc;
	return count;
}

static ssize_t slimpro_pm_get_pmd_power(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct slimpro_pm_dev *ctx = dev_get_drvdata(dev);
	u32 msg[3];
	u32 data0;
	u32 data1;
	u32 data2;
	int rc;

	msg[0] = SLIMPRO_PM_ENCODE_DAPL_MSG(0, SLIMPRO_PM_DAPL_GET_CURPWR);
	rc = slimpro_pm_rd(ctx, msg);
	if (rc < 0)
		return sprintf(buf, "Unavailable error %d\n", rc);
	data0 = msg[1];
	msg[0] = SLIMPRO_PM_ENCODE_DAPL_MSG(0, SLIMPRO_PM_DAPL_GET_CUMENGY);
	rc = slimpro_pm_rd(ctx, msg);
	if (rc < 0)
		return sprintf(buf, "Unavailable error %d\n", rc);
	data1 = msg[1];
	data2 = msg[2];
	rc = sprintf(buf, "Current PMD Power: %d\n", data0);
	rc += sprintf(buf + rc, "Cumulative Energy in %d second(s) %d\n",
		      data1, data2);
	return rc;
}

static ssize_t slimpro_pm_clr_pmd_cumengy(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct slimpro_pm_dev *ctx = dev_get_drvdata(dev);
	int rc;
	u32 msg[3];

	if (!count)
		return -EINVAL;

	msg[0] = SLIMPRO_PM_ENCODE_DAPL_MSG(0, SLIMPRO_PM_DAPL_RESET_CUMENGY);
	rc = slimpro_pm_wr(ctx, msg);
	if (rc < 0)
		return rc;
        return count;
}

static struct device_attribute slimpro_pm_attrs[] = {
	__ATTR(tpc, S_IRUGO, slimpro_pm_tpc_status, NULL),
	__ATTR(tpc_monitoring, S_IRUGO | S_IWUGO,
		slimpro_pm_get_tpc_mon, slimpro_pm_set_tpc_mon),
	__ATTR(tpc_tm1_threshold, S_IWUGO, NULL, slimpro_pm_set_tpc_threshtm1),
	__ATTR(tpc_tm2_threshold, S_IWUGO, NULL, slimpro_pm_set_tpc_threshtm2),
	__ATTR(dapl_unit, S_IRUGO | S_IWUGO,
	       slimpro_pm_get_dapl_units, slimpro_pm_set_dapl_units),
	__ATTR(dapl_thresh, S_IRUGO | S_IWUGO,
	       slimpro_pm_get_dapl_pwrlim, slimpro_pm_set_dapl_pwrlim),
	__ATTR(dapl_power, S_IRUGO | S_IWUGO,
	       slimpro_pm_get_pmd_power, slimpro_pm_clr_pmd_cumengy),
};

static int slimpro_pm_add_sysfs(struct slimpro_pm_dev *ctx)
{
	int i;
	int err = 0;

	for (i = 0; i < ARRAY_SIZE(slimpro_pm_attrs); i++) {
		err = device_create_file(ctx->dev, &slimpro_pm_attrs[i]);
		if (err)
			goto fail;
	}
	return 0;

fail:
	while (--i >= 0)
		device_remove_file(ctx->dev, &slimpro_pm_attrs[i]);

	return err;
}

static void slimpro_pm_remove_sysfs(struct slimpro_pm_dev *ctx)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(slimpro_pm_attrs); i++)
		device_remove_file(ctx->dev, &slimpro_pm_attrs[i]);
}

static int shutdown_system(void)
{
	char *argv[] = {"/sbin/poweroff", NULL};
	static char *envp[] = {
		"HOME=/",
		"TERM=linux",
		"PATH=/sbin:/bin:/usr/bin:/usr/sbin", NULL };
	return call_usermodehelper(argv[0], argv, envp, UMH_NO_WAIT);
}

static int slimpro_pm_event_notify(struct notifier_block *self,
					unsigned long action, void *dev)
{
	struct slimpro_pm_async_evt *event = dev;

	switch (action) {
	case SLIMPRO_PM_EVENT_TPC:
		printk(KERN_INFO "SLIMPRO_DB_EVENT_TPC\n");
		shutdown_system();
		break;
	default:
		printk(KERN_INFO "SLIMPRO_DB_EVENT_UNKNOWN msg_type=0x%x\n",
			SLIMPRO_PM_DECODE_MSG(event->msg));
	}
	return NOTIFY_OK;
}

static struct notifier_block slimpro_pm_event_nb = {
	.notifier_call =  slimpro_pm_event_notify,
};

static void slimpro_pm_evt_work(struct work_struct *work)
{
	struct slimpro_pm_async_evt event;
	struct slimpro_pm_dev *ctx;
	unsigned long action;
	u8 msg_type;
	u32 msg_hndl;

	ctx = container_of(work, struct slimpro_pm_dev, workq);
	action      = SLIMPRO_PM_EVENT_UNKNOWN;
	event.msg   = ctx->resp_msg[0];
	event.data0 = ctx->resp_msg[1];
	event.data1 = ctx->resp_msg[2];
	msg_type    = SLIMPRO_PM_DECODE_MSG(event.msg);

	switch(msg_type) {
	case SLIMPRO_PM_PWRMGMT_MSG:
		msg_hndl = SPIMPRO_PM_DECODE_HDLR(event.msg);
		if (msg_hndl == SLIMPRO_PM_TPC_HDLR)
			action = SLIMPRO_PM_EVENT_TPC;
		break;
	default:
		return;
	}

	blocking_notifier_call_chain(&ctx->evt_notifier, action, &event);
}

void slimpro_pm_event_register(struct notifier_block *nb)
{
	blocking_notifier_chain_register(&slimpro_pm_ctx->evt_notifier, nb);
}
EXPORT_SYMBOL_GPL(slimpro_pm_event_register);

void slimpro_pm_event_unregister(struct notifier_block *nb)
{
	blocking_notifier_chain_unregister(&slimpro_pm_ctx->evt_notifier, nb);
}
EXPORT_SYMBOL_GPL(slimpro_pm_event_unregister);

#define to_slimpro_pm_dev(cl)	\
		container_of(cl, struct slimpro_pm_dev, mbox_client)

static void slimpro_pm_rx_cb(struct mbox_client *cl, void *msg)
{
	struct slimpro_pm_dev *ctx = to_slimpro_pm_dev(cl);

	/*
	 * Response message format:
	 * mssg[0] is the return code of the operation
	 * mssg[1] is the first data word
	 * mssg[2] is the second data word
	 *
	 * As we only support byte and word size, just assign it.
	 */
	ctx->resp_msg[0] = ((u32*)msg)[0];
	ctx->resp_msg[1] = ((u32*)msg)[1];
	ctx->resp_msg[2] = ((u32*)msg)[2];

	if (ctx->mbox_client.tx_block) 
		complete(&ctx->rd_complete);
	else
		ctx->pm_rx_poll = 0;

	/* Call back can also be PM event
	 * Check for TPC events
	 */
	if (ctx->resp_msg[0] == SLIMPRO_TPC_EVENT) {
		schedule_work(&ctx->workq);
	}
}

static int __init slimpro_pm_probe(struct platform_device *pdev)
{
	struct slimpro_pm_dev *ctx;
	struct mbox_client *cl;
	int rc;

	ctx = devm_kzalloc(&pdev->dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		dev_err(&pdev->dev, "Memory allocation failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, ctx);
	ctx->dev = &pdev->dev;

	cl = &ctx->mbox_client;

	/* Request mailbox channel */
	cl->rx_callback = slimpro_pm_rx_cb;
	cl->tx_done = NULL; 
	cl->tx_block = true;
	cl->tx_tout = SLIMPRO_PM_OP_TO_MS;
	cl->link_data = NULL;
	cl->knows_txdone = false;
	cl->chan_name = "slimpro_mbox:MBOX7"; /* Second channel is for PM */
	ctx->mbox_chan = mbox_request_channel(cl);
	if (IS_ERR(ctx->mbox_chan)) {
		dev_err(&pdev->dev, "SLIMpro mailbox request failed\n");
		return PTR_ERR(ctx->mbox_chan);
	}

	/* Initialiation in case of using poll mode */
	ctx->pm_rx_poll = 0;
	spin_lock_init(&ctx->lock);

	rc = slimpro_pm_add_sysfs(ctx);
	if (rc) {
		dev_err(&pdev->dev, "SLIMpro file creation failed error %d\n",
			rc);
		return rc;
	}

	slimpro_pm_ctx = ctx;
	INIT_WORK(&ctx->workq, slimpro_pm_evt_work);
	BLOCKING_INIT_NOTIFIER_HEAD(&ctx->evt_notifier);
	slimpro_pm_event_register(&slimpro_pm_event_nb);
	dev_info(&pdev->dev, "APM X-Gene SLIMpro PM driver registered\n");
	return 0;
}

static int slimpro_pm_remove(struct platform_device *pdev)
{
	struct slimpro_pm_dev *ctx = platform_get_drvdata(pdev);

	slimpro_pm_remove_sysfs(ctx);
	slimpro_pm_event_unregister(&slimpro_pm_event_nb);

	return 0;
}

#define XGENE_SLIMPRO_PM "xgene-slimpro-pm"

static struct platform_driver slimpro_pm_driver __refdata = {
	.probe		= slimpro_pm_probe,
	.remove		= slimpro_pm_remove,
	.driver		= {
		.name	= XGENE_SLIMPRO_PM,
		.owner	= THIS_MODULE,
	},
};

static struct platform_device *slimpro_pm_device;

static int __init slimpro_pm_init(void)
{
	struct platform_device *pdev;
	int ret;

	ret = platform_driver_register(&slimpro_pm_driver);
	if (ret < 0)
		return ret;

	pdev = platform_device_register_simple(XGENE_SLIMPRO_PM, -1, NULL, 0);
	if (IS_ERR(pdev))
		return -ENODEV;

	slimpro_pm_device = pdev;

	return 0;
}
subsys_initcall(slimpro_pm_init);

static void __exit slimpro_pm_exit(void)
{
	platform_device_unregister(slimpro_pm_device);
	platform_driver_unregister(&slimpro_pm_driver);
}
module_exit(slimpro_pm_exit);

MODULE_DESCRIPTION("APM X-Gene SLIMpro PM driver");
MODULE_LICENSE("GPL v2");
