/*
 * APM X-Gene SLIMpro MailBox Driver
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/acpi.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/mailbox_controller.h>

#define MBOX_REG_SET_OFFSET		0x1000
#define MBOX_CNT			8
#define MBOX_STATUS_AVAIL_MASK		0x00010000
#define MBOX_STATUS_ACK_MASK		0x00000001

/* Configuration and Status Registers */
struct slimpro_mbox_csr {
	u32 in;
	u32 din0;
	u32 din1;
	u32 rsvd1;
	u32 out;
	u32 dout0;
	u32 dout1;
	u32 rsvd2;
	u32 status;
	u32 statusmask;
};

struct slimpro_mlink {
	struct mbox_link link;
	struct slimpro_mbox *ctx;
	struct slimpro_mbox_csr __iomem *csr;
	int id;
	int irq;
	u32 rx_msg[3];
};

struct slimpro_mbox {
	struct device *dev;
	void __iomem *csr_mbox;		/* Mailbox register base */
	struct mbox_controller mbox_con;
	struct slimpro_mlink mlink[MBOX_CNT];
};

#define to_slimpro_mlink(l)	container_of(l, struct slimpro_mlink, link)

static void slimpro_mlink_send_msg(struct slimpro_mlink *mlink, u32 *msg)
{
	writel(msg[1], &mlink->csr->dout0);
	writel(msg[2], &mlink->csr->dout1);
	writel(msg[0], &mlink->csr->out);
}

static void slimpro_mlink_recv_msg(struct slimpro_mlink *mlink)
{
	mlink->rx_msg[1] = readl(&mlink->csr->din0);
	mlink->rx_msg[2] = readl(&mlink->csr->din1);
	mlink->rx_msg[0] = readl(&mlink->csr->in);
}

static void slimpro_mlink_enable_int(struct slimpro_mlink *mlink, u32 mask)
{
	u32 val = readl(&mlink->csr->statusmask);
	val &= ~mask;
	writel(val, &mlink->csr->statusmask);
}

static void slimpro_mlink_disable_int(struct slimpro_mlink *mlink, u32 mask)
{
	u32 val = readl(&mlink->csr->statusmask);
	val |= mask;
	writel(val, &mlink->csr->statusmask);
}

static int slimpro_mlink_status_ack(struct slimpro_mlink *mlink)
{
	u32 val = readl(&mlink->csr->status);
	if (val & MBOX_STATUS_ACK_MASK) {
		writel(MBOX_STATUS_ACK_MASK, &mlink->csr->status);
		return 1;
	}
	return 0;
}

static int slimpro_mlink_status_avail(struct slimpro_mlink *mlink)
{
	u32 val = readl(&mlink->csr->status);
	if (val & MBOX_STATUS_AVAIL_MASK) {
		slimpro_mlink_recv_msg(mlink);
		writel(MBOX_STATUS_AVAIL_MASK, &mlink->csr->status);
		return 1;
	}
	return 0;
}

static irqreturn_t slimpro_mbox_irq(int irq, void *id)
{
	struct slimpro_mlink *mlink = id;

	if (slimpro_mlink_status_ack(mlink))
		mbox_link_txdone(&mlink->link, MBOX_OK);

	if (slimpro_mlink_status_avail(mlink)) {
		slimpro_mlink_recv_msg(mlink);
		mbox_link_received_data(&mlink->link, mlink->rx_msg);
	}

	return IRQ_HANDLED;
}

static int slimpro_mbox_send_data(struct mbox_link *link, void *msg)
{
	struct slimpro_mlink *mlink = to_slimpro_mlink(link);

	slimpro_mlink_send_msg(mlink, msg);
	return 0;
}

static int slimpro_mbox_startup(struct mbox_link *link, void *ignored)
{
	struct slimpro_mlink *mlink = to_slimpro_mlink(link);
	struct slimpro_mbox *ctx = mlink->ctx;
	int rc;

	rc = devm_request_irq(ctx->dev, mlink->irq, slimpro_mbox_irq, 0,
			      mlink->link.link_name, mlink);
	if (unlikely(rc)) {
		dev_err(ctx->dev, "failed to register mailbox interrupt %d\n",
			mlink->irq);
		return rc;
	}

	/* Enable HW interrupt */
	writel(MBOX_STATUS_ACK_MASK | MBOX_STATUS_AVAIL_MASK,
		&mlink->csr->status);
	slimpro_mlink_enable_int(mlink,
				 MBOX_STATUS_ACK_MASK |
				 MBOX_STATUS_AVAIL_MASK);
	return 0;
}

static void slimpro_mbox_shutdown(struct mbox_link *link)
{
	struct slimpro_mlink *mlink = to_slimpro_mlink(link);

	slimpro_mlink_disable_int(mlink,
				  MBOX_STATUS_ACK_MASK |
				  MBOX_STATUS_AVAIL_MASK);
	devm_free_irq(mlink->ctx->dev, mlink->irq, mlink);
}

static struct mbox_link_ops slimpro_mbox_ops = {
	.send_data = slimpro_mbox_send_data,
	.startup = slimpro_mbox_startup,
	.shutdown = slimpro_mbox_shutdown,
};

static int __init slimpro_probe(struct platform_device *pdev)
{
	struct mbox_link *mlink[MBOX_CNT + 1];
	struct slimpro_mbox *ctx;
	struct resource *regs;
	int rc;
	int i;

	ctx = devm_kzalloc(&pdev->dev, sizeof(*ctx), GFP_KERNEL);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);
	platform_set_drvdata(pdev, ctx);
	ctx->dev = &pdev->dev;

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	ctx->csr_mbox = devm_ioremap_resource(&pdev->dev, regs);
	if (IS_ERR(ctx->csr_mbox))
		return PTR_ERR(ctx->csr_mbox);

	/* Setup mailbox links */
	for (i = 0; i < MBOX_CNT; i++) {
		ctx->mlink[i].irq = platform_get_irq(pdev, i);
		if (ctx->mlink[i].irq < 0) {
			dev_err(&pdev->dev, "no IRQ at index %d\n",
				ctx->mlink[i].irq);
			return -ENODEV;
		}

		ctx->mlink[i].csr = ctx->csr_mbox + i * MBOX_REG_SET_OFFSET;
		ctx->mlink[i].id = i;
		ctx->mlink[i].ctx = ctx;

		snprintf(ctx->mlink[i].link.link_name, 16, "MBOX%d", i);
		mlink[i] = &ctx->mlink[i].link;
	}
	mlink[i] = NULL; /* Terminate list */

	/* Setup mailbox controller */
	ctx->mbox_con.dev = &pdev->dev;
	ctx->mbox_con.links = mlink;
	ctx->mbox_con.txdone_irq = true;
	ctx->mbox_con.ops = &slimpro_mbox_ops;
	snprintf(ctx->mbox_con.controller_name, 16, "slimpro_mbox");

	rc = mbox_controller_register(&ctx->mbox_con);
	if (rc) {
		dev_err(&pdev->dev, "IPC link register failed error %d\n",
			rc);
		return rc;
	}

	dev_info(&pdev->dev, "APM X-Gene SlimPRO MailBox registered\n");
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id slimpro_of_match[] = {
	{.compatible = "apm,xgene-slimpro-mbox" },
	{ },
};
MODULE_DEVICE_TABLE(of, slimpro_of_match);
#endif

#ifdef CONFIG_ACPI
static const struct acpi_device_id slimpro_acpi_ids[] = {
	{"APMC0D01", 0},
	{}
};
MODULE_DEVICE_TABLE(acpi, slimpro_acpi_ids);
#endif

static struct platform_driver slimpro_driver __refdata = {
	.probe	= slimpro_probe,
	.driver	= {
		.name = "xgene-slimpro-mbox",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(slimpro_of_match),
		.acpi_match_table = ACPI_PTR(slimpro_acpi_ids)
	},
};

static int __init slimpro_init(void)
{
	return platform_driver_register(&slimpro_driver);
}
core_initcall(slimpro_init);
