/*
 * AppliedMicro X-Gene MCI Interface
 *
 * Copyright (c) 2014 Applied Micro Circuits Corporation.
 * Author: 	Hoan Tran <hotran@apm.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/io.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/export.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <asm/io.h>
#include <linux/string.h>
#include <misc/xgene/mci/xgene_mci_sysfs.h>
#include <misc/xgene/mci/xgene_mci_core.h>
#include <asm/cacheflush.h>
#include <asm/outercache.h>
#include <asm/hardware/mslim-iof-map.h>
// FIXME: wait for new QMTM MSLIM support
#include "../qmtm/apm_qmi_runtime.h"

extern struct apm_qmi_device apm_qmi;

static struct xgene_mci *mci_dev;

struct xgene_mci* xgene_mci_get_dev(void)
{
	return mci_dev;
}
EXPORT_SYMBOL(xgene_mci_get_dev);

static int xgene_mci_mslim_client_register(struct xgene_mci_client *mci_client)
{
	int i;

	for (i = 0; i < XGENE_MCI_MAX_CLIENT; i++) {
		if (mci_dev->clients[i] != NULL) {
			if (strcmp((const char *)mci_dev->clients[i]->name,
							(const char *)mci_client->name) == 0)
			return MCI_CONNECTION_INUSE;
		}
	}

	for (i = 0; i < XGENE_MCI_MAX_CLIENT; i++) {
		if (mci_dev->unregistered_clients[i] == NULL) {
			mci_dev->unregistered_clients[i] = mci_client;
			return i;
		}
	}

	return MCI_OUTOFRESOURCE;
}

static int xgene_mci_mslim_client_add(int port, char name[8])
{
	int i;

	if(port > XGENE_MCI_MAX_CLIENT)
	return MCI_OUTOFRESOURCE;

	if (mci_dev->clients[port] != NULL) {
		return MCI_CONNECTION_INUSE;
	}

	for (i = 0; i < XGENE_MCI_MAX_CLIENT; i++) {
		if (mci_dev->clients[i] != NULL) {
			if (strcmp((const char *)mci_dev->clients[i]->name,
							(const char *)name) == 0)
			return MCI_CONNECTION_INUSE;
		}
	}

	for (i = 0; i < XGENE_MCI_MAX_CLIENT; i++) {
		if (mci_dev->unregistered_clients[i] != NULL) {
			if (strcmp((const char *)mci_dev->unregistered_clients[i]->name,
							(const char *)name) == 0) {
				mci_dev->clients[port] = mci_dev->unregistered_clients[i];
				/* Clear unregister slot*/
				mci_dev->unregistered_clients[i] = NULL;
				return MCI_OK;
			}
		}
	}

	return MCI_NO_SUPPORT_DRIVER;
}

static void xgene_mci_rx_tasklet(unsigned long data)
{
	struct xgene_mci_msg_desc *mci_msg_desc;
	struct xgene_mci_channel *chan = (void *) data;

	mci_msg_desc = xgene_mci_receive_msg(chan, chan->qm_msg);

	xgene_mci_process_msg(chan, mci_msg_desc);
	kfree(chan->qm_msg);
	enable_irq(apm_qmi.qmi_irq);
}

static int xgene_mci_msg_hdlr(u8 qmtm_ip, u16 queue_id,
		struct xgene_mci_msg64 *msg)
{
	struct xgene_mci *mci;
	disable_irq_nosync(apm_qmi.qmi_irq);

	mci = xgene_mci_get_dev();

	if ((mci->enable == 1) && (mci->channels[1].rx_q_desc.pbn == queue_id)) {
		mci->channels[1].qm_msg = msg;
		tasklet_schedule(&mci->channels[1].rx_tasklet);
	} else {
		mci->channels[0].qm_msg = msg;
		tasklet_schedule(&mci->channels[0].rx_tasklet);
	}
	return 0;
}

static int xgene_mci_tx_qm_msg(struct xgene_mci_channel *chan,
		struct xgene_qmtm_msg64 * msg)
{
	volatile int nummsgs;
	struct storm_qmtm_csr_qstate *raw_q;
	int rc = 0;

	raw_q = kmalloc(sizeof(struct storm_qmtm_csr_qstate), GFP_KERNEL);
	BUG_ON(raw_q == NULL);

	do {
		cop_qm_raw_qstate_rd(RX_QMTM, chan->tx_q_desc.qid, raw_q);
		nummsgs = (raw_q->w1  & 0x1fffe) >> 1;
	} while (nummsgs > 256);

	rc = apm_qmi_push_msg(chan->tx_q_desc.qid, RX_QMTM, msg);
	kfree(raw_q);

	return rc;
}

static void xgene_rx_ping_msg_cb (struct xgene_mci_channel *chan,
		struct xgene_mci_msg_desc * msg)
{
	struct xgene_mci *mci = container_of(chan,
			struct xgene_mci, channels[chan->ID]);

	/* Update tx queue ID for both High and Low priority channel*/
	mci->channels[0].tx_q_desc.qid = msg->param0;
	mci->channels[1].tx_q_desc.qid = msg->param1;
	mci->channels[0].rx_q_desc.pbn = msg->param2;
	mci->channels[1].rx_q_desc.pbn = msg->param3;
	xgene_mci_send_msg(&mci->channels[0], MSLIM_OK_MSG,
			0, 0, (void *) NULL, 0);
	if (mci->enable == 0) {
		mci->enable = 1;
		pr_info("MSLIM: MCI is enabled\n");
	}
}

static void xgene_mci_flush_dcache_area(void *addr, size_t len)
{
	/* L1 cache flush */
	__cpuc_flush_dcache_area(addr, len);
	/* L2 cache flush */
	outer_flush_range((phys_addr_t)addr, len);

}

static unsigned long long xgene_virt_to_phys (void *x, size_t len)
{
	return mslim_axi_to_iof_addr(0, virt_to_phys(x));
}

static unsigned long long xgene_phys_to_virt(void *x, size_t len)
{
	/* Map Page 1: MSLIM map to Potenza Memory */
	return (unsigned long long)mslim_iof_to_axi_vaddr(1,x,2);
}

struct xgene_mci_operations xgene_mci_ops = {
		.tx_msg = xgene_mci_tx_qm_msg,
		.rx_ping_msg_cb = xgene_rx_ping_msg_cb,
		.wait_for_msg_complete = NULL,
		.flush_dcache_area = xgene_mci_flush_dcache_area,
		.virt_to_phys = xgene_virt_to_phys,
		.phys_to_virt = xgene_phys_to_virt,
};

static int xgene_mci_client_rx_open(struct xgene_mci_channel *chan,
		int client_port,
		struct xgene_mci_msg_desc *mci_msg)
{
	struct xgene_mci *mci = container_of(chan,
			struct xgene_mci, channels[chan->ID]);
	char name[8];
	int rc = 0;

	memcpy(name, (void *)mci_msg->buf, 8);
	rc = xgene_mci_mslim_client_add(client_port, name);
	dev_dbg(mci->dev,"Open client name %s \n", name);
	if (rc == MCI_OK) {
		if (mci->clients[client_port]->mci_client_status_cb_fn) {
			mci->clients[client_port]->mci_client_status_cb_fn(
					client_port, NULL, MCI_CONNECTION_ESTABLISHED);
		}
		rc = MCI_CONNECTION_ESTABLISHED;
	}
	/* Send ACK message to Potenza*/
	xgene_mci_send_msg(&mci->channels[0], MSLIM_CLIENT_MSG + client_port,
			MCI_CLIENT_ACK, rc, (void *)NULL, 0);
	return 0;
}

static int xgene_mci_client_rx_close(struct xgene_mci_channel *chan,
		int client_port,
		struct xgene_mci_msg_desc *mci_msg)
{
	struct xgene_mci *mci = container_of(chan,
				struct xgene_mci, channels[chan->ID]);

	xgene_mci_send_msg(&mci->channels[0], MSLIM_CLIENT_MSG + client_port,
			MCI_CLIENT_ACK, MCI_CONNECTION_CLOSED, (void *)NULL, 0);

	return 0;
}

static int xgene_mci_client_open (struct xgene_mci_client *mci_client)
{
	int rc = MCI_OK;

	/* MSLIM: just register this Client into buffer
 	 	 	 	waiting until Potenza opens this client */
	rc = xgene_mci_mslim_client_register(mci_client);

	return rc;
}

struct xgene_mci_client_operations xgene_mci_client_ops = {
		.rx_open_cb = xgene_mci_client_rx_open,
		.rx_close_cb = xgene_mci_client_rx_close,
		.rx_ack_cb = NULL,
		.rx_data_cb = NULL,
		.open = xgene_mci_client_open,
};

static struct platform_device xgene_mci_device = {
	.name = "mci",
	.id = 0,
};

static int xgene_mci_probe(struct platform_device *pdev)
{
	struct xgene_mci *mci;
	int i;

	pr_info("MCI: MultiCore Communication Interface \r\n");

	mci = devm_kzalloc(&pdev->dev, sizeof(*mci), GFP_KERNEL);

	spin_lock_init(&mci->lock);
	mci->ops = &xgene_mci_ops;
	mci->client_ops = &xgene_mci_client_ops;
	mci->dev = &xgene_mci_device.dev;
	mci_dev = mci;


	/* Init QMI interrupt handler for PBN */
	apm_qmi_msg_rx_register(QMTM1, APM_QM_MCI_RTYPE, &xgene_mci_msg_hdlr);

	for (i = 0; i < XGENE_MCI_MAX_CHANNEL; i++) {
		tasklet_init(&mci->channels[i].rx_tasklet, xgene_mci_rx_tasklet,
				(unsigned long) &mci->channels[i]);
		INIT_DELAYED_WORK(&mci->channels[i].rxwork, xgene_mci_rx_work);
		spin_lock_init(&mci->channels[i].lock);
	}

	mci->channels[0].ID = MCI_HIGH_PRIORITY;
	mci->channels[1].ID = MCI_LOW_PRIORITY;

	/* Init LIST for Async Data transfer: Tx and Rx */
	INIT_LIST_HEAD(&(mci->channels[MCI_LOW_PRIORITY].rx_q_desc.msg_ll.node));

	return 0;
}

static struct of_device_id xgene_mci_match[] = {
		{ 	.name = "mci",
			.type = "mci",
			.compatible = "apm,xgene-mci",
		},
		{ },
};
MODULE_DEVICE_TABLE(of, xgene_mci_match);

static struct platform_driver xgene_mci_driver = {
		.driver = { .name = "mci",
					.owner = THIS_MODULE,
					.of_match_table = xgene_mci_match,
		},
		.probe = xgene_mci_probe,
};

static int xgene_mci_driver_init(void)
{
	int rc;

	platform_device_register(&xgene_mci_device);
	platform_driver_register(&xgene_mci_driver);

	rc = add_mci_sysfs(&xgene_mci_driver.driver);
	if (rc)
		remove_mci_sysfs(&xgene_mci_driver.driver);

	return rc;
}

static void xgene_mci_driver_cleanup(void)
{
	remove_mci_sysfs(&xgene_mci_driver.driver);
	platform_driver_unregister(&xgene_mci_driver);
	platform_device_unregister(&xgene_mci_device);
}

late_initcall(xgene_mci_driver_init);
module_exit(xgene_mci_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("XGENE MCI driver");
