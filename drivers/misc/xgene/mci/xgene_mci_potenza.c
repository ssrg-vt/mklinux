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
#include <linux/string.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
#include <misc/xgene/mci/xgene_mci_sysfs.h>
#include <misc/xgene/mci/xgene_mci_core.h>
#include <misc/xgene/slimpro/xgene_slimpro.h>

static struct xgene_mci *mci_dev;

struct xgene_mci* xgene_mci_get_dev(void)
{
	return mci_dev;
}
EXPORT_SYMBOL(xgene_mci_get_dev);

static int xgene_mci_wait(struct xgene_mci_channel *chan)
{
	int count;
	for (count = 200; count > 0; count--) {
		if (chan->cmd_done)
			break;
		msleep(1);
	}
	return (chan->cmd_done) ? 0 : 1;
}

static void xgene_mci_rx_tasklet(unsigned long data)
{
	struct xgene_mci_channel *chan = (void *) data;
	struct xgene_mci_msg_desc *mci_msg_desc;
	struct xgene_qmtm_msg64 msg;

	xgene_qmtm_pull_msg(RX_QMTM, chan->rx_q_desc.qid, &msg);
	chan->qm_msg = (struct xgene_mci_msg64 *)&msg;

	mci_msg_desc = xgene_mci_receive_msg(chan, chan->qm_msg);
	xgene_mci_process_msg(chan, mci_msg_desc);

	if ((chan->msg_desc != NULL) && (chan->ID == MCI_HIGH_PRIORITY))
		chan->cmd_done = 1;

	enable_irq(chan->rx_q_desc.irq);
}

static irqreturn_t xgene_mci_qm_msg_isr(int value, void *id)
{
	struct xgene_mci_channel *chan = id;

	BUG_ON(chan == NULL);

	chan->irq_count++;

	disable_irq_nosync(chan->rx_q_desc.irq);
	tasklet_schedule(&chan->rx_tasklet);

	return IRQ_HANDLED;
}

static void xgene_qmi_wq_assoc_write(struct xgene_mci *mci,
		u32 qm_ip, u32 pbn)
{
	u32 qmwqassoc, qmlwqassoc;

	qmwqassoc =	readl((void *) (mci->qmi_csr_vaddr
							+ SM_QMI_SLAVE_CFGSSQMIWQASSOC__ADDR));
	qmlwqassoc = readl((void *) (mci->qmi_csr_vaddr
					+ SM_QMI_SLAVE_CFGSSQMIQMLITEWQASSOC__ADDR));

	switch (qm_ip) {
	case 0:
		/* qmwqassoc = 0 and qmlwqassoc = 0 */
		qmwqassoc &= ~(1 << pbn);
		qmlwqassoc &= ~(1 << pbn);
		break;
	case 1:
		/* qmwqassoc = 1 and qmlwqassoc = 0 */
		qmwqassoc |= (1 << pbn);
		qmlwqassoc &= ~(1 << pbn);
		break;
	case 2:
		/* qmwqassoc = 0 and qmlwqassoc = 1 */
		qmwqassoc &= ~(1 << pbn);
		qmlwqassoc |= (1 << pbn);
		break;
	case 3:
		/* qmwqassoc = 1 and qmlwqassoc = 1 */
		qmwqassoc |= (1 << pbn);
		qmlwqassoc |= (1 << pbn);
		break;
	default:
		break;
	}

	writel(qmwqassoc, (void *) (mci->qmi_csr_vaddr
			+ SM_QMI_SLAVE_CFGSSQMIWQASSOC__ADDR));
	writel(qmlwqassoc, (void *) (mci->qmi_csr_vaddr
					+ SM_QMI_SLAVE_CFGSSQMIQMLITEWQASSOC__ADDR));
}

static void xgene_qmi_wq_assoc_read(struct xgene_mci *mci, u32 pbn)
{
	u32 qmwqassoc, qmlwqassoc;

	qmwqassoc = readl((void *) (mci->qmi_csr_vaddr
							+ SM_QMI_SLAVE_CFGSSQMIWQASSOC__ADDR));
	qmlwqassoc = readl((void *) (mci->qmi_csr_vaddr
					+ SM_QMI_SLAVE_CFGSSQMIQMLITEWQASSOC__ADDR));

	dev_dbg(mci->dev, "wqassoc=0x%x qmlitewqassoc=0x%x\n",
			qmwqassoc, qmlwqassoc);
}

static int xgene_mci_qconfig(struct xgene_mci *mci)
{
	struct xgene_mci_channel *chan;
	struct xgene_qmtm_qinfo qinfo;
	int rc;
	int i;
	u8 slave = mci->sdev->slave;
	u8 slave_i = mci->sdev->slave_i;
	u8 qmtm_ip = mci->sdev->qmtm_ip;

	/* Allocate tx queue */
	for (i = 0; i < XGENE_MCI_MAX_CHANNEL; i++) {
		memset(&qinfo, 0, sizeof(qinfo));
		qinfo.slave = slave;
		qinfo.qtype = QTYPE_PQ;
		qinfo.qsize = QSIZE_16KB;
		qinfo.flags = XGENE_SLAVE_DEFAULT_FLAGS;
		qinfo.qaccess = QACCESS_ALT;

		if ((rc = xgene_qmtm_set_qinfo(&qinfo)) != QMTM_OK) {
			dev_err(mci->dev, "unable to allocate Tx queue\n");
			goto err_q_alloc;
		}

		chan = &mci->channels[i];
 		chan->tx_q_desc.qid = qinfo.queue_id;
 		chan->tx_q_desc.mbid = qinfo.pbn;
		chan->tx_q_desc.pbn = qinfo.pbn;
		chan->tx_q_desc.slot_cnt = qinfo.count;
		chan->tx_q_desc.slot_cur = 0;
		chan->tx_q_desc.base_addr = qinfo.msg32;
		chan->tx_q_desc.nummsgs = &(((u32 *)qinfo.qfabric)[1]);
		dev_dbg(mci->dev, "MCI tx queue %d PBN %d desc 0x%p\n",
				chan->tx_q_desc.qid, chan->tx_q_desc.pbn,
				chan->tx_q_desc.base_addr);
		xgene_qmi_wq_assoc_write(mci, TX_QMTM, chan->tx_q_desc.pbn);
		xgene_qmi_wq_assoc_read(mci, chan->tx_q_desc.pbn);
	}

	/* Allocate completion queue for PktDMA */
	for (i = 0; i < XGENE_MCI_MAX_CHANNEL; i++) {
		memset(&qinfo, 0, sizeof(qinfo));
		qinfo.slave = slave_i;
		qinfo.qtype = QTYPE_PQ;
		qinfo.qsize = QSIZE_16KB;
		qinfo.flags = XGENE_SLAVE_DEFAULT_FLAGS;
		qinfo.qaccess = QACCESS_ALT;

		if ((rc = xgene_qmtm_set_qinfo(&qinfo)) != QMTM_OK) {
			dev_err(mci->dev, "unable to allocate Rx queue\n");
			goto err_q_alloc;
		}

		chan = &mci->channels[i];
		chan->rx_q_desc.qid = qinfo.queue_id;
 		chan->rx_q_desc.mbid = qinfo.pbn;
		chan->rx_q_desc.pbn = qinfo.pbn;
		chan->rx_q_desc.slot_cnt = qinfo.count;
		chan->rx_q_desc.slot_cur = 0;
		chan->rx_q_desc.base_addr = qinfo.msg32;
		chan->rx_q_desc.irq = qinfo.irq;
		sprintf(chan->rx_q_desc.irq_name, "MCI%d", i);
		/* Enable interrupt coalescence for Comp queue. This will
		 reduce the interrupt overhead and better performance */
		xgene_qmtm_intr_coalesce(qmtm_ip, qinfo.pbn, 0x4);
		dev_dbg(mci->dev, "MCI rx queue %d PBN %d desc 0x%p IRQ %d\n",
				chan->rx_q_desc.qid, chan->rx_q_desc.pbn,
				chan->rx_q_desc.base_addr, chan->rx_q_desc.irq);

		rc = request_irq(chan->rx_q_desc.irq, xgene_mci_qm_msg_isr, 0,
				chan->rx_q_desc.irq_name, chan);
		if (rc) {
			dev_err(mci->dev, "Failed to register IRQ %d for channel %d\n",
					chan->rx_q_desc.irq, i);
			goto err_q_alloc;
		}
	}

err_q_alloc:
	return rc;
}

static int xgene_mci_tx_qm_msg(struct xgene_mci_channel *chan,
		struct xgene_qmtm_msg64 * msg)
{
	volatile int nummsgs;
	int rc = 0;

	do {
		nummsgs = (readl(chan->tx_q_desc.nummsgs) & 0x1fffe) >> 1;
	} while (nummsgs > (256));
	xgene_qmtm_push_msg(TX_QMTM, chan->tx_q_desc.qid,
			(struct xgene_qmtm_msg64 *) msg);
	return rc;
}

static int xgene_mci_wait_for_msg_complete (struct xgene_mci_channel *chan)
{
	struct xgene_mci *mci = container_of(chan,
			struct xgene_mci, channels[chan->ID]);
	int rc = 0;

    chan->cmd_done = 0;
    rc = xgene_mci_wait(chan);
    if (rc) {
        rc = -1;
        dev_err(mci->dev, "timeout\r\n");
    }
    rc = chan->msg_desc->rvalue;
    return rc;
}

static void xgene_mci_flush_dcache_area(void *addr, size_t len)
{
	__flush_dcache_area(addr, len);
}

static unsigned long long xgene_virt_to_phys (struct xgene_mci_channel *chan,
		void *x, size_t len)
{
	return (unsigned long long)virt_to_phys(x);
}

static unsigned long long xgene_phys_to_virt(struct xgene_mci_channel *chan,
		void *x, size_t len)
{
	struct xgene_mci *mci = container_of(chan,
			struct xgene_mci, channels[chan->ID]);
	/* Potenza map to MSLIM Memory */
	if((unsigned long long)x >= mci->mslim_mem_paddr)
		return (unsigned long long)(mci->mslim_mem_vaddr
				+ ((unsigned long long)x - mci->mslim_mem_paddr));

	return (unsigned long long)(phys_to_virt((phys_addr_t)x));
}


struct xgene_mci_operations xgene_mci_ops = {
		.tx_msg = xgene_mci_tx_qm_msg,
		.rx_ping_msg_cb = NULL,
		.wait_for_msg_complete = xgene_mci_wait_for_msg_complete,
		.flush_dcache_area = xgene_mci_flush_dcache_area,
		.virt_to_phys = xgene_virt_to_phys,
		.phys_to_virt = xgene_phys_to_virt,
};

struct xgene_mci_client_operations xgene_mci_client_ops = {
		.rx_open_cb = NULL,
		.rx_close_cb = NULL,
		.rx_ack_cb = NULL,
		.rx_data_cb = NULL,
		.open = NULL,
};

static struct platform_device xgene_mci_device = {
	.name = "mci",
	.id = 0,
};

static int xgene_mci_probe(struct platform_device *pdev)
{
	int rc = -1;
	struct xgene_mci *mci;
	int csr_addr_size;
	struct xgene_qmtm_sdev *sdev;
	const char name[12] = "MSLIM_QMTM1";
	int i;

	pr_info("MCI: MultiCore Communication Interface \r\n");

	/* Check if MSLIM is available */
	if (is_apm88xxxx_mslim_disabled())
		return rc;

	mci = devm_kzalloc(&pdev->dev, sizeof(*mci), GFP_KERNEL);

	spin_lock_init(&mci->lock);
	mci->ops = &xgene_mci_ops;
	mci->client_ops = &xgene_mci_client_ops;
	mci->dev = &xgene_mci_device.dev;
	mci_dev = mci;

	/* Init Mem IO to access QMI CSR registers */
	mci->qmi_csr_paddr = QMI_CSR_BASE_ADDR;
	csr_addr_size = QMI_CSR_BASE_SIZE;
	mci->qmi_csr_vaddr = devm_ioremap_nocache(&pdev->dev,
			mci->qmi_csr_paddr, csr_addr_size);
	dev_dbg(mci->dev, " qmi_csr_paddr=0x%p qmi_csr_vaddr=0x%p\n",
			(void *) mci->qmi_csr_paddr, mci->qmi_csr_vaddr);

	/* Init Mem for MSLIM */
	mci->mslim_mem_paddr = (u64)virt_to_phys((void *)high_memory);
	mci->mslim_mem_vaddr = devm_ioremap_nocache(&pdev->dev,
			mci->mslim_mem_paddr, MSLIM_MEMSIZE);
	dev_dbg(mci->dev, " mslim_mem_paddr=0x%p mslim_mem_vaddr=0x%p\n",
			(void *) mci->mslim_mem_paddr, mci->mslim_mem_vaddr);

	if ((sdev = xgene_qmtm_set_sdev(name, 1,
			0, 2, 0x20, 2)) == NULL) {
		dev_err(mci->dev, "QMTM%d Slave %s error\n", 1, name);
		return -ENODEV;
	}

	mci->sdev = sdev;

	/* Init QM for MCI*/
	rc = xgene_mci_qconfig(mci);

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

	/* Check MSLIM is online*/
	rc = xgene_mci_send_msg(&mci->channels[0],
			MSLIM_PING_MSG, 0, 0, (void *) NULL, 0);
	if ((rc != MCI_OK)) {
		mci->enable = 0;
		return rc;
	}
	mci->enable = 1;

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
	int rc = 0;

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
