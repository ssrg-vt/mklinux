/*
 * AppliedMicro X-Gene SoC SATA Driver
 *
 * Copyright (c) 2013, Applied Micro Circuits Corporation
 * Author: Loc Ho <lho@apm.com>
 *         Tuan Phan <tphan@apm.com>
 *         Suman Tripathi <stripathi@apm.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This file include routines that are only needed by MSLIM.
 */

#ifdef CONFIG_ARCH_MSLIM
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <asm/hardware/mslim-iof-map.h>
#include <asm/cacheflush.h>
#include "ahci.h"

void ahci_start_engine(struct ata_port *ap);
ssize_t ahci_transmit_led_message(struct ata_port *ap, u32 state,
                                        ssize_t size);
void ahci_init_sw_activity(struct ata_link *link);
void ahci_power_up(struct ata_port *ap);
void ahci_pmp_detach(struct ata_port *ap);
void ahci_pmp_attach(struct ata_port *ap);

static u64 xgene_ahci_to_axi(dma_addr_t addr)
{
	/* Due to the fact that the DDR address seen by the MSLIM is different
	 * from what is needed by the SATA core, we need a translation from
	 * MSLIM CPU DDR address into AXI address that the SATA core can
	 * access.
	 */
	return mslim_pa_to_iof_axi(addr);
}

static void xgene_ahci_dflush(void *addr, int size)
{
	/* Due to the fact that the MSLIM L1/L2 cache is not coherent with
	* the IO, we need to flush before given the data to the SATA core.
	*/
	__cpuc_flush_dcache_area(addr, size);
}

static unsigned int xgene_ahci_fill_sg(struct ata_queued_cmd *qc, void *cmd_tbl)
{
	struct scatterlist *sg;
	struct ahci_sg *ahci_sg = cmd_tbl + AHCI_CMD_TBL_HDR_SZ;
	unsigned int si;

	/*
	 * Next, the S/G list.
	 */
	for_each_sg(qc->sg, sg, qc->n_elem, si) {
		dma_addr_t addr = sg_dma_address(sg);
		u64 dma_addr = xgene_ahci_to_axi(addr);
		u32 sg_len = sg_dma_len(sg);
		ahci_sg[si].addr = cpu_to_le32(dma_addr & 0xffffffff);
		ahci_sg[si].addr_hi = cpu_to_le32((dma_addr >> 16) >> 16);
		ahci_sg[si].flags_size = cpu_to_le32(sg_len - 1);
		xgene_ahci_dflush((void *) __va(addr), sg_len);
	}
	return si;
}

void xgene_ahci_fill_cmd_slot(struct ahci_port_priv *pp,
	unsigned int tag, u32 opts)
{
	dma_addr_t cmd_tbl_dma = pp->cmd_tbl_dma + tag * AHCI_CMD_TBL_SZ;
	u64 cmd_tbl_dma_addr = xgene_ahci_to_axi(cmd_tbl_dma);

	pp->cmd_slot[tag].opts = cpu_to_le32(opts);
	pp->cmd_slot[tag].status = 0;
	pp->cmd_slot[tag].tbl_addr = cpu_to_le32(cmd_tbl_dma_addr &
						0xffffffff);
	pp->cmd_slot[tag].tbl_addr_hi = cpu_to_le32((cmd_tbl_dma_addr >> 16)
						>> 16);
	xgene_ahci_dflush((void *) &pp->cmd_slot[tag],
			sizeof(pp->cmd_slot[tag]));
}

static int xgene_ahci_exec_polled_cmd(struct ata_port *ap, int pmp,
	struct ata_taskfile *tf, int is_cmd, u16 flags,
	unsigned long timeout_msec)
{
	const u32 cmd_fis_len = 5; /* five dwords */
	struct ahci_port_priv *pp = ap->private_data;
	void __iomem *port_mmio = ahci_port_base(ap);
	u8 *fis = pp->cmd_tbl;
	u32 tmp;

	/* prep the command */
	ata_tf_to_fis(tf, pmp, is_cmd, fis);

	/* Must call X-Gene version in case it needs to flush the cache for
           MSLIM as well as AXI address translation */
	xgene_ahci_fill_cmd_slot(pp, 0, cmd_fis_len | flags | (pmp << 12));

	/* issue & wait */
	writel(1, port_mmio + PORT_CMD_ISSUE);

	if (timeout_msec) {
		tmp = ata_wait_register(ap, port_mmio + PORT_CMD_ISSUE,
					0x1, 0x1, 1, timeout_msec);
		if (tmp & 0x1) {
			ahci_kick_engine(ap);
			return -EBUSY;
		}
	} else {
		readl(port_mmio + PORT_CMD_ISSUE);	/* flush */
	}
	return 0;
}

/*
 * Due to the fact that the DDR address seen by the MSLIM is different
 * from what is needed by the SATA core, we need a translation from
 * CPU address into AXI address that the SATA core can access.
 */
static void xgene_ahci_start_fis_rx(struct ata_port *ap)
{
	void __iomem *port_mmio = ahci_port_base(ap);
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct ahci_port_priv *pp = ap->private_data;
	u32 tmp;
	u64 cmd_slot_dma_addr = xgene_ahci_to_axi(pp->cmd_slot_dma);
	u64 rx_fis_dma_addr = xgene_ahci_to_axi(pp->rx_fis_dma);

	if (hpriv->cap & HOST_CAP_64)
		writel((cmd_slot_dma_addr >> 16) >> 16,
		       port_mmio + PORT_LST_ADDR_HI);
	writel(cmd_slot_dma_addr& 0xffffffff, port_mmio + PORT_LST_ADDR);

	if (hpriv->cap & HOST_CAP_64)
		writel((rx_fis_dma_addr >> 16) >> 16,
		       port_mmio + PORT_FIS_ADDR_HI);
	writel(rx_fis_dma_addr & 0xffffffff, port_mmio + PORT_FIS_ADDR);
	/* enable FIS reception */
	tmp = readl(port_mmio + PORT_CMD);
	tmp |= PORT_CMD_FIS_RX;
	writel(tmp, port_mmio + PORT_CMD);

	/* flush */
	readl(port_mmio + PORT_CMD);
}

/**
 * xgene_ahci_restart_engine_mslim - Restart the dma engine.
 * @ap : ATA port of interest
 *
 * Restarts the dma engine inside the controller.
 */
int xgene_ahci_restart_engine_mslim(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	
	ahci_stop_engine(ap);
	xgene_ahci_start_fis_rx(ap);
	hpriv->start_engine(ap);

	return 0;
}

static void xgene_ahci_start_port(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct ahci_port_priv *pp = ap->private_data;
	struct ata_link *link;
	struct ahci_em_priv *emp;
	ssize_t rc;
	int i;

	/* enable FIS reception */
	xgene_ahci_start_fis_rx(ap);

	/* enable DMA */
	if (!(hpriv->flags & AHCI_HFLAG_DELAY_ENGINE))
		ahci_start_engine(ap);

	/* turn on LEDs */
	if (ap->flags & ATA_FLAG_EM) {
		ata_for_each_link(link, ap, EDGE) {
			emp = &pp->em_priv[link->pmp];

			/* EM Transmit bit maybe busy during init */
			for (i = 0; i < EM_MAX_RETRY; i++) {
				rc = ahci_transmit_led_message(ap,
							       emp->led_state,
							       4);
				if (rc == -EBUSY)
					ata_msleep(ap, 1);
				else
					break;
			}
		}
	}

	if (ap->flags & ATA_FLAG_SW_ACTIVITY)
		ata_for_each_link(link, ap, EDGE)
			ahci_init_sw_activity(link);
}

int xgene_ahci_port_resume(struct ata_port *ap)
{
	ahci_power_up(ap);
	xgene_ahci_start_port(ap);

	if (sata_pmp_attached(ap))
		ahci_pmp_attach(ap);
	else
		ahci_pmp_detach(ap);

	return 0;
}

int xgene_ahci_port_start(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct device *dev = ap->host->dev;
	struct ahci_port_priv *pp;
	void *mem;
	dma_addr_t mem_dma;
	size_t dma_sz, rx_fis_sz;

	pp = devm_kzalloc(dev, sizeof(*pp), GFP_KERNEL);
	if (!pp)
		return -ENOMEM;

	/* check FBS capability */
	if ((hpriv->cap & HOST_CAP_FBS) && sata_pmp_supported(ap)) {
		void __iomem *port_mmio = ahci_port_base(ap);
		u32 cmd = readl(port_mmio + PORT_CMD);
		if (cmd & PORT_CMD_FBSCP)
			pp->fbs_supported = true;
		else if (hpriv->flags & AHCI_HFLAG_YES_FBS) {
			dev_info(dev, "port %d can do FBS, forcing FBSCP\n",
				 ap->port_no);
			pp->fbs_supported = true;
		} else
			dev_warn(dev, "port %d is not capable of FBS\n",
				 ap->port_no);
	}

	if (pp->fbs_supported) {
		dma_sz = AHCI_PORT_PRIV_FBS_DMA_SZ;
		rx_fis_sz = AHCI_RX_FIS_SZ * 16;
	} else {
		dma_sz = AHCI_PORT_PRIV_DMA_SZ;
		rx_fis_sz = AHCI_RX_FIS_SZ;
	}

	mem = dmam_alloc_coherent(dev, dma_sz, &mem_dma, GFP_KERNEL);
	if (!mem)
		return -ENOMEM;
	memset(mem, 0, dma_sz);

	/*
	 * First item in chunk of DMA memory: 32-slot command table,
	 * 32 bytes each in size
	 */
	pp->cmd_slot = mem;
	pp->cmd_slot_dma = mem_dma;
	mem += AHCI_CMD_SLOT_SZ;
	mem_dma += AHCI_CMD_SLOT_SZ;

	/*
	 * Second item: Received-FIS area
	 */
	pp->rx_fis = mem;
	pp->rx_fis_dma = mem_dma;
	mem += rx_fis_sz;
	mem_dma += rx_fis_sz;

	/*
	 * Third item: data area for storing a single command
	 * and its scatter-gather table
	 */
	pp->cmd_tbl = mem;
	pp->cmd_tbl_dma = mem_dma;
	/*
	 * Save off initial list of interrupts to be enabled.
	 * This could be changed later
	 */
	pp->intr_mask = DEF_PORT_IRQ;

	ap->private_data = pp;

	/* engage engines, captain */
	return xgene_ahci_port_resume(ap);
}

void xgene_ahci_qc_prep_mslim(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct ahci_port_priv *pp = ap->private_data;
	int is_atapi = ata_is_atapi(qc->tf.protocol);
	void *cmd_tbl;
	u32 opts;
	const u32 cmd_fis_len = 5;	/* five dwords */
	unsigned int n_elem;
	void *port_mmio = ahci_port_base(ap);
	u32 fbs;

	/*
	 * Fill in command table information.  First, the header,
	 * a SATA Register - Host to Device command FIS.
	 */
	cmd_tbl = pp->cmd_tbl + qc->tag * AHCI_CMD_TBL_SZ;

	/* Due to hardware errata for port multipier CBS mode, enable DEV
	   field of PxFBS in order to clear the PxCI */
	fbs = readl(port_mmio + 0x40);
	if ((qc->dev->link->pmp) || ((fbs >> 8) & 0x0000000f)) {
	  fbs &= 0xfffff0ff;
	  fbs |= qc->dev->link->pmp << 8;
	  writel(fbs, port_mmio + 0x40);
	}

	ata_tf_to_fis(&qc->tf, qc->dev->link->pmp, 1, cmd_tbl);
	if (is_atapi) {
		memset(cmd_tbl + AHCI_CMD_TBL_CDB, 0, 32);
		memcpy(cmd_tbl + AHCI_CMD_TBL_CDB, qc->cdb, qc->dev->cdb_len);
	}
	n_elem = 0;
	if (qc->flags & ATA_QCFLAG_DMAMAP)
		n_elem = xgene_ahci_fill_sg(qc, cmd_tbl);

	/*
	 * Fill in command slot information.
	 */
	opts = cmd_fis_len | n_elem << 16 | (qc->dev->link->pmp << 12);
	if (qc->tf.flags & ATA_TFLAG_WRITE)
		opts |= AHCI_CMD_WRITE;
	if (is_atapi)
		opts |= AHCI_CMD_ATAPI | AHCI_CMD_PREFETCH;

	xgene_ahci_fill_cmd_slot(pp, qc->tag, opts);
}

static int xgene_ahci_do_softreset(struct ata_link *link,
				   unsigned int *class, int pmp,
				   unsigned long deadline,
				   int (*check_ready) (struct ata_link *link))
{
	struct ata_port *ap = link->ap;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	const char *reason = NULL;
	unsigned long now, msecs;
	struct ata_taskfile tf;
	int rc;

	ata_link_dbg(link, "ENTER\n");

	/* prepare for SRST (AHCI-1.1 10.4.1) */
	rc = ahci_kick_engine(ap);
	if (rc && rc != -EOPNOTSUPP)
		ata_link_warn(link, "failed to reset engine (errno=%d)\n", rc);

	ata_tf_init(link->device, &tf);
	/* issue the first D2H Register FIS */
	msecs = 0;
	now = jiffies;
	if (time_after(deadline, now))
		msecs = jiffies_to_msecs(deadline - now);

	tf.ctl |= ATA_SRST;
	/* Must call X-Gene version in case it needs to flush the cache for
	   MSLIM as well as AXI address translation */
	if (xgene_ahci_exec_polled_cmd(ap, pmp, &tf, 0,
				       AHCI_CMD_RESET | AHCI_CMD_CLR_BUSY,
				       msecs)) {
		rc = -EIO;
		reason = "1st FIS failed";
		goto fail;
	}

	/* spec says at least 5us, but be generous and sleep for 1ms */
	ata_msleep(ap, 1);

	/* issue the second D2H Register FIS */
	tf.ctl &= ~ATA_SRST;
	/* HW need AHCI_CMD_RESET and AHCI_CMD_CLR_BUSY */
	xgene_ahci_exec_polled_cmd(ap, pmp, &tf, 0,
				   AHCI_CMD_RESET | AHCI_CMD_CLR_BUSY, msecs);
	/* wait for link to become ready */
	rc = ata_wait_after_reset(link, deadline, check_ready);
	if (rc == -EBUSY && hpriv->flags & AHCI_HFLAG_SRST_TOUT_IS_OFFLINE) {
		/*
		 * Workaround for cases where link online status can't
		 * be trusted.  Treat device readiness timeout as link
		 * offline.
		 */
		ata_link_info(link, "device not ready, treating as offline\n");
		*class = ATA_DEV_NONE;
	} else if (rc) {
		/* link occupied, -ENODEV too is an error */
		reason = "device not ready";
		goto fail;
	} else {
		*class = ahci_dev_classify(ap);
	}

	ata_link_dbg(link, "EXIT, class=%u\n", *class);
	return 0;

fail:
	ata_link_err(link, "softreset failed (%s)\n", reason);
	return rc;
}

int xgene_ahci_softreset_mslim(struct ata_link *link, unsigned int *class,
				unsigned long deadline)
{
	int pmp = sata_srst_pmp(link);
	return xgene_ahci_do_softreset(link, class, pmp, deadline,
				       ahci_check_ready);
}

#endif /* CONFIG_ARCH_MSLIM */
