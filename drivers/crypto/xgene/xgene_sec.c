/*
 * APM X-Gene SoC Security Core Driver
 *
 * Copyright (c) 2014 Applied Micro Circuits Corporation.
 * All rights reserved. Loc Ho <lho@apm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <crypto/algapi.h>
#include <crypto/hash.h>
#include <crypto/internal/hash.h>
#include <linux/acpi.h>
#include <linux/delay.h>
#include <linux/efi.h>
#include <linux/highmem.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <misc/xgene/qmtm/xgene_qmtm.h>
#include "xgene_sec_csr.h"
#include "xgene_sec.h"
#include "xgene_sec_tkn.h"
#include "xgene_sec_sa.h"
#include "xgene_sec_alg.h"

#ifdef CONFIG_ARCH_MSLIM
#define QACCESS_TYPE		QACCESS_QMI
#else
#define QACCESS_TYPE		QACCESS_ALT
#endif

/* Enable UIO - xgenesec.uio=1 to enable */
static int crypto_uio;
MODULE_PARM_DESC(crypto_uio, "Enable UIO support (1=enable 0=disable)");
module_param_named(uio, crypto_uio, int, 0444);

struct xgene_sec_ctx *xg_ctx;

void xgene_sec_wr32(struct xgene_sec_ctx *ctx, u8 block, u32 reg, u32 data)
{
	void __iomem *reg_offset;

	switch (block) {
	case EIP96_AXI_CSR_BLOCK:
		reg_offset = ctx->eip96_axi_csr + reg;
		break;
	case EIP96_CSR_BLOCK:
		reg_offset = ctx->eip96_csr + reg;
		break;
	case EIP96_CORE_CSR_BLOCK:
		reg_offset = ctx->eip96_core_csr + reg;
		break;
	case SEC_GLB_CTRL_CSR_BLOCK:
		reg_offset = ctx->ctrl_csr + reg;
		break;
	case CLK_RES_CSR_BLOCK:
		reg_offset = ctx->clk_csr + reg;
		break;
	case QMI_CTL_BLOCK:
		reg_offset = ctx->qmi_ctl_csr + reg;
		break;
#if 0
		/* FIXME */
	case XTS_AXI_CSR_BLOCK:
		reg_offset = ctx->xts_csr + reg;
		break;
	case XTS_CSR_BLOCK:
		reg_offset = ctx->xts_csr + reg;
		break;
	case XTS_CORE_CSR_BLOCK:
		reg_offset = ctx->xts_core_csr + reg;
		break;
	case EIP62_AXI_CSR_BLOCK:
		reg_offset = ctx->eip62_axi_csr + reg;
		break;
	case EIP62_CSR_BLOCK:
		reg_offset = ctx->eip62_csr + reg;
		break;
	case EIP62_CORE_CSR_BLOCK:
		reg_offset = ctx->eip62_core_csr + reg;
		break;
	case AXI_SLAVE_SHIM_BLOCK:
		reg_offset = ctx->sec_axi_slave_shim_csr + reg;
		break;
	case AXI_MASTER_SHIM_BLOCK:
		reg_offset = ctx->sec_axi_master_shim_csr + reg;
		break;
#endif
	default:
		dev_err(ctx->dev, "Invalid write to block %d offset %d\n",
			block, reg);
		return;
	}
	dev_dbg(ctx->dev, "CSR WR: 0x%p value: 0x%08X\n", reg_offset, data);
	writel(data, reg_offset);
}

void xgene_sec_rd32(struct xgene_sec_ctx *ctx, u8 block, u32 reg, u32 * data)
{
	void __iomem *reg_offset;

	switch (block) {
	case EIP96_AXI_CSR_BLOCK:
		reg_offset = ctx->eip96_axi_csr + reg;
		break;
	case EIP96_CSR_BLOCK:
		reg_offset = ctx->eip96_csr + reg;
		break;
	case EIP96_CORE_CSR_BLOCK:
		reg_offset = ctx->eip96_core_csr + reg;
		break;
	case SEC_GLB_CTRL_CSR_BLOCK:
		reg_offset = ctx->ctrl_csr + reg;
		break;
	case CLK_RES_CSR_BLOCK:
		reg_offset = ctx->clk_csr + reg;
		break;
	case QMI_CTL_BLOCK:
		reg_offset = ctx->qmi_ctl_csr + reg;
		break;
#if 0
		/* FIXME */
	case XTS_AXI_CSR_BLOCK:
		reg_offset = ctx->xts_axi_csr + reg;
		break;
	case XTS_CSR_BLOCK:
		reg_offset = ctx->xts_csr + reg;
		break;
	case XTS_CORE_CSR_BLOCK:
		reg_offset = ctx->xts_core_csr + reg;
		break;
	case EIP62_AXI_CSR_BLOCK:
		reg_offset = ctx->eip62_axi_csr + reg;
		break;
	case EIP62_CSR_BLOCK:
		reg_offset = ctx->eip62_csr + reg;
		break;
	case EIP62_CORE_CSR_BLOCK:
		reg_offset = ctx->eip62_core_csr + reg;
		break;
	case AXI_SLAVE_SHIM_BLOCK:
		reg_offset = ctx->sec_axi_slave_shim_csr + reg;
		break;
	case AXI_MASTER_SHIM_BLOCK:
		reg_offset = ctx->sec_axi_master_shim_csr + reg;
		break;
#endif
	default:
		dev_err(ctx->dev, "Invalid read from block %d offset: %d\n",
			block, reg);
		return;
	}
	*data = readl(reg_offset);
	dev_dbg(ctx->dev, "CSR RD: 0x%p value: 0x%08X\n", reg_offset, *data);
}

int xgene_sec_init_memram(struct xgene_sec_ctx *ctx)
{
	void __iomem *diagcsr = ctx->diag_csr;
	int try;
	u32 val;

	val = readl(diagcsr + SEC_CFG_MEM_RAM_SHUTDOWN_ADDR);
	if (val == 0) {
		dev_dbg(ctx->dev, "memory already released from shutdown\n");
		return 0;
	}
	dev_dbg(ctx->dev, "Release memory from shutdown\n");
	/* Memory in shutdown. Remove from shutdown. */
	writel(0x0, diagcsr + SEC_CFG_MEM_RAM_SHUTDOWN_ADDR);
	readl(diagcsr + SEC_CFG_MEM_RAM_SHUTDOWN_ADDR);	/* Force a barrier */

	/* Check for at least ~1ms */
	try = 1000;
	do {
		val = readl(diagcsr + SEC_BLOCK_MEM_RDY_ADDR);
		if (val != 0xFFFFFFFF)
			usleep_range(1, 100);
	} while (val != 0xFFFFFFFF && try-- > 0);
	if (try <= 0) {
		dev_err(ctx->dev, "failed to release memory from shutdown\n");
		return -ENODEV;
	}
	return 0;
}

void xgene_sec_hdlr_ctxreg_err(struct xgene_sec_ctx *ctx, u32 ctx_sts)
{
	if (!ctx_sts)
		return;
	if (ctx_sts & E14_MASK)
		dev_err(ctx->dev, "Time out error\n");
	if (ctx_sts & E13_MASK)
		dev_err(ctx->dev, "Pad verify failed\n");
	if (ctx_sts & E12_MASK)
		dev_err(ctx->dev, "Checksum failed\n");
	if (ctx_sts & E11_MASK)
		dev_err(ctx->dev, "SPI Check failed\n");
	if (ctx_sts & E10_MASK)
		dev_err(ctx->dev, "Seq num check/roll over failed\n");
	if (ctx_sts & E9_MASK)
		dev_err(ctx->dev, "Authentication failed\n");
	if (ctx_sts & E8_MASK)
		dev_err(ctx->dev, "TTL/HOP limit underflow\n");
	if (ctx_sts & E7_MASK)
		dev_err(ctx->dev, "Hash input overflow\n");
	if (ctx_sts & E6_MASK)
		dev_err(ctx->dev, "Prohibited Alg\n");
	if (ctx_sts & E5_MASK)
		dev_err(ctx->dev, "Invalid command/mode/alg combination\n");
	if (ctx_sts & E4_MASK)
		dev_err(ctx->dev, "Hash Block size err\n");
	if (ctx_sts & E3_MASK)
		dev_err(ctx->dev, "Crypto Blk size err\n");
	if (ctx_sts & E2_MASK)
		dev_err(ctx->dev, "Too much bypass data in Tkn\n");
	if (ctx_sts & E1_MASK)
		dev_err(ctx->dev, "Unknown tkn command instruction\n");
	if (ctx_sts & E0_MASK)
		dev_err(ctx->dev, "Packet length err\n");
}

void xgene_sec_intr_hdlr(struct xgene_sec_ctx *ctx)
{
	u32 status;

	xgene_sec_rd32(ctx, EIP96_AXI_CSR_BLOCK, CSR_SEC_INT_STS_ADDR, &status);
	if (!status)
		return;
	if (status & EIP96_CORE_MASK)
		dev_err(ctx->dev, "EIP96 error\n");
	if (status & TKN_RD_F2_MASK)
		dev_err(ctx->dev, "token read error\n");
	if (status & CTX_RD_F2_MASK)
		dev_err(ctx->dev, "context read error\n");
	if (status & DATA_RD_F2_MASK)
		dev_err(ctx->dev, "data read error\n");
	if (status & DSTLL_RD_F2_MASK)
		dev_err(ctx->dev, "destination linked list read error\n");
	if (status & TKN_WR_F2_MASK)
		dev_err(ctx->dev, "token write error\n");
	if (status & CTX_WR_F2_MASK)
		dev_err(ctx->dev, "context write error\n");
	if (status & DATA_WR_F2_MASK)
		dev_err(ctx->dev, "data write error\n");
	xgene_sec_rd32(ctx, EIP96_CORE_CSR_BLOCK, IPE_CTX_STAT_ADDR, &status);
	xgene_sec_hdlr_ctxreg_err(ctx, status);
	xgene_sec_wr32(ctx, EIP96_AXI_CSR_BLOCK, CSR_SEC_INT_STSMASK_ADDR,
		       0xffffffff);
}

void xgene_sec_hdlr_qerr(struct xgene_sec_ctx *ctx, int qm_err_hop, int qm_err)
{
	switch (qm_err) {
	case 0x11:
		/*
		 * Any kind of crypto error (illegal tkn/ctx, tkn length,
		 * bad packet, and etc
		 */
		dev_err(ctx->dev, "token programming with hop %d error\n",
			qm_err_hop);
		break;
	case 0x03:
		dev_err(ctx->dev,
			"out of free pool buffer with hop %d error\n",
			qm_err_hop);
		break;
	case 0x04:
		dev_err(ctx->dev, "AXI read with hop %d error\n", qm_err_hop);
		break;
	case 0x05:
		dev_err(ctx->dev, "AXI write with hop %d error\n", qm_err_hop);
		break;
	case 0x07:
		dev_err(ctx->dev,
			"Invalid QM message format with hop %d eorr\n",
			qm_err_hop);
		break;
	case 0x06:
		dev_err(ctx->dev,
			"Destination linked list read with hop %d error\n",
			qm_err_hop);
		break;
	case 0x01:
		dev_err(ctx->dev,
			"Not enough entries in destination linked list with "
			"hop %d error\n",
			qm_err_hop);
		break;
	}
}

/*
 * Error Callback Handler
 */
static irqreturn_t xgene_sec_intr_cb(int irq, void *id)
{
	struct xgene_sec_ctx *ctx = id;
	u32 stat = 0;

	/* Determine what causes interrupt */
	xgene_sec_rd32(ctx, SEC_GLB_CTRL_CSR_BLOCK, CSR_GLB_SEC_INT_STS_ADDR,
		       &stat);
	if (stat & EIP96_MASK)
		xgene_sec_intr_hdlr(ctx);	/* EIP96 interrupted */

	/* Clean them */
	xgene_sec_wr32(ctx, SEC_GLB_CTRL_CSR_BLOCK, CSR_GLB_SEC_INT_STS_ADDR,
		       stat);
	return IRQ_HANDLED;
}

int xgene_sec_qconfig(struct xgene_sec_ctx *ctx)
{
	struct xgene_qmtm_qinfo queue;
	int i;
	int rc;

	/* Allocate work queue for SEC */
	memset(&queue, 0, sizeof(queue));
	queue.slave = SLAVE_SEC;
	queue.qtype = QTYPE_PQ;
	queue.qsize = QSIZE_64KB;
	queue.qaccess = QACCESS_TYPE;
	queue.flags = XGENE_SLAVE_DEFAULT_FLAGS;
	rc = xgene_qmtm_set_qinfo(&queue);
	if (rc != QMTM_OK) {
		dev_err(ctx->dev, "Couldn't allocate work queue\n");
		return -ENOMEM;
	}
	ctx->qm_queue.txq = queue;

	memset(&queue, 0, sizeof(struct xgene_qmtm_qinfo));
	queue.slave = SLAVE_CPU_QMTM1;
	queue.qtype = QTYPE_PQ;
	queue.qsize = QSIZE_64KB;
	queue.qaccess = QACCESS_TYPE;
	queue.flags = XGENE_SLAVE_DEFAULT_FLAGS;
	rc = xgene_qmtm_set_qinfo(&queue);
	if (rc != QMTM_OK) {
		dev_err(ctx->dev,
			"Couldn't allocate completion queue for SEC\n");
		return -ENOMEM;
	}
	ctx->qm_queue.cpq = queue;

	/* Allocate free pool queue for SEC */
	for (i = 0; i < NUM_FREEPOOL; i++) {
		memset(&queue, 0, sizeof(struct xgene_qmtm_qinfo));
		queue.slave = SLAVE_SEC;
		queue.qtype = QTYPE_FP;
		queue.qsize = QSIZE_64KB;
		queue.qaccess = QACCESS_TYPE;
		queue.flags = XGENE_SLAVE_DEFAULT_FLAGS;
		rc = xgene_qmtm_set_qinfo(&queue);
		if (rc != QMTM_OK) {
			dev_err(ctx->dev,
				"Couldn't allocate free queue for SEC\n");
			return -ENOMEM;
		}
		ctx->qm_queue.fpq[i] = queue;
	}
	return 0;
}

int xgene_sec_hwinit(struct xgene_sec_ctx *ctx)
{
	u32 dev_info;
	u32 proto_alg;
	u32 val;

	xgene_sec_rd32(ctx, SEC_GLB_CTRL_CSR_BLOCK, CSR_ID_ADDR, &val);
	dev_dbg(ctx->dev, "Security ID: %02d.%02d.%02d\n",
		REV_NO_RD(val), BUS_ID_RD(val), DEVICE_ID_RD(val));

	/* For AXI parameter, leave default priorities */

	/* Enable IRQ for core, EIP96, and XTS blocks, EIP62 */
	xgene_sec_wr32(ctx, SEC_GLB_CTRL_CSR_BLOCK,
		       CSR_GLB_SEC_INT_STSMASK_ADDR,
		       0xFFFFFFFF & ~(QMIMASK_MASK |
				      EIP96MASK_MASK |
				      XTSMASK_MASK | EIP62MASK_MASK));

	/*
	 * Configure QMI block
	 * Un-mask work queue overflow/underun, free pool overflow/underrun
	 */
	xgene_sec_rd32(ctx, QMI_CTL_BLOCK, SEC_STSSSQMIDBGDATA_ADDR, &val);
	xgene_sec_wr32(ctx, QMI_CTL_BLOCK, SEC_STSSSQMIINT0MASK_ADDR, 0x0);
	xgene_sec_wr32(ctx, QMI_CTL_BLOCK, SEC_STSSSQMIINT1MASK_ADDR, 0x0);
	xgene_sec_wr32(ctx, QMI_CTL_BLOCK, SEC_STSSSQMIINT2MASK_ADDR, 0x0);
	xgene_sec_wr32(ctx, QMI_CTL_BLOCK, SEC_STSSSQMIINT3MASK_ADDR, 0x0);
	/* Un-mask AXI write/read errror */
	xgene_sec_wr32(ctx, QMI_CTL_BLOCK, SEC_STSSSQMIINT4MASK_ADDR, 0x0);

	/* Associate FP and WQ to QM1 */
	xgene_sec_wr32(ctx, QMI_CTL_BLOCK, SEC_CFGSSQMIFPQASSOC_ADDR,
		       0xFFFFFFFF);
	xgene_sec_wr32(ctx, QMI_CTL_BLOCK, SEC_CFGSSQMIWQASSOC_ADDR,
		       0xFFFFFFFF);
	xgene_sec_wr32(ctx, QMI_CTL_BLOCK, SEC_CFGSSQMIQMHOLD_ADDR, 0x00000002);

	/* Configure XTS core */
	/* FIXME */

	/*
	 * Configure EIP96 core
	 */

	/* For EIP96 AXI read and write max burst, leave default */

	/* For EIP96 AXI outstanding read and write, leave default */
	xgene_sec_wr32(ctx, EIP96_AXI_CSR_BLOCK,
		       CSR_AXI_RD_MAX_OUTSTANDING_CFG_ADDR, 0x88880000);
	xgene_sec_wr32(ctx, EIP96_AXI_CSR_BLOCK,
		       CSR_AXI_WR_MAX_OUTSTANDING_CFG_ADDR, 0x88800000);

	/* For EIP96 AXI, enable error interrupts */
	xgene_sec_wr32(ctx, EIP96_AXI_CSR_BLOCK, CSR_SEC_INT_STSMASK_ADDR,
		       ~0xFFFFFFFF);

	/* For EIP96, configure CSR_SEC_CRYPTO_CFG_0 */
	xgene_sec_rd32(ctx, EIP96_CSR_BLOCK, CSR_SEC_CRYPTO_CFG_0_ADDR, &val);
	val &= ~TKN_RD_OFFSET_SIZE0_MASK;
	val |= TKN_RD_OFFSET_SIZE0_WR(TKN_RESULT_HDR_MAX_LEN / 4);
	val &= ~TKN_RD_PREFETCH_SIZE0_MASK;
	val |= TKN_RD_PREFETCH_SIZE0_WR(0x8);	/* Set token prefetch to 32B */
	xgene_sec_wr32(ctx, EIP96_CSR_BLOCK, CSR_SEC_CRYPTO_CFG_0_ADDR, val);

	/* For EIP96, configure CSR_SEC_CRYPTO_CFG_1 */
	xgene_sec_rd32(ctx, EIP96_CSR_BLOCK, CSR_SEC_CRYPTO_CFG_1_ADDR, &val);
	val &= ~DIS_CTX_INTERLOCK1_MASK;
	val |= DIS_CTX_INTERLOCK1_WR(0);
	xgene_sec_wr32(ctx, EIP96_CSR_BLOCK, CSR_SEC_CRYPTO_CFG_1_ADDR, val);

	/* For EIP96, leave CSR_SEC_CRYPTO_CFG_2 to default value */

	/* For EIP96, leave error mask to default */

	/* For EIP96 core, read version */
	xgene_sec_rd32(ctx, EIP96_CORE_CSR_BLOCK, IPE_DEV_INFO_ADDR, &dev_info);
	xgene_sec_rd32(ctx, EIP96_CORE_CSR_BLOCK, IPE_PRC_ALG_EN_ADDR,
		       &proto_alg);
	dev_dbg(ctx->dev, "Core ver %d.%d Proto/Alg: 0x%08X\n",
		MAJOR_REVISION_NUMBER_RD(dev_info),
		MINOR_REVISION_NUMBER_RD(dev_info), proto_alg);

	/* For EIP96, configure context access mode */
	xgene_sec_wr32(ctx, EIP96_CORE_CSR_BLOCK, IPE_CTX_CTRL_ADDR,
		       CONTEXT_SIZE_WR(0x36)
		       | SEC_ADDRESS_MODE_WR(0)
		       | SEC_CONTROL_MODE_WR(1));

	/* For EIP96 core, configure control status register */
	xgene_sec_rd32(ctx, EIP96_CORE_CSR_BLOCK, IPE_TKN_CTRL_STAT_ADDR, &val);
	val &= ~OPTIMAL_CONTEXT_UPDATES_MASK;
	val &= ~INTERRUPT_PULSE_OR_LEVEL_MASK;
	val &= ~TIME_OUT_COUNTER_ENABLE_MASK;
	val |= OPTIMAL_CONTEXT_UPDATES_WR(0);
	val |= INTERRUPT_PULSE_OR_LEVEL_WR(1 /* Level interrupt */ );
	val |= TIME_OUT_COUNTER_ENABLE_WR(0);
	xgene_sec_wr32(ctx, EIP96_CORE_CSR_BLOCK, IPE_TKN_CTRL_STAT_ADDR, val);

	/* For EIP96 core, setup interrupt */
	xgene_sec_wr32(ctx, EIP96_CORE_CSR_BLOCK, IPE_INT_CTRL_STAT_ADDR,
		       INPUT_DMA_ERROR_ENABLE_WR(1)
		       | OUTPUT_DMA_ERROR_ENABLE_WR(1)
		       | PACKET_PROCESSING_ENABLE_WR(1)
		       | PACKET_TIMEOUT_ENABLE_WR(1)
		       | FATAL_ERROR_ENABLE_WR(1)
		       | INTERRUPT_OUTPUT_PIN_ENABLE_WR(1));

	/* For EIP96, seed the PRNG, KEY0, KEY1, and LRSR */
	get_random_bytes_arch(&val, sizeof(u32));
	xgene_sec_wr32(ctx, EIP96_CORE_CSR_BLOCK, IPE_PRNG_SEED_L_ADDR, val);
	get_random_bytes_arch(&val, sizeof(u32));
	xgene_sec_wr32(ctx, EIP96_CORE_CSR_BLOCK, IPE_PRNG_SEED_H_ADDR, val);
	get_random_bytes_arch(&val, sizeof(u32));
	xgene_sec_wr32(ctx, EIP96_CORE_CSR_BLOCK, IPE_PRNG_KEY_0_L_ADDR, val);
	get_random_bytes_arch(&val, sizeof(u32));
	xgene_sec_wr32(ctx, EIP96_CORE_CSR_BLOCK, IPE_PRNG_KEY_0_H_ADDR, val);
	get_random_bytes_arch(&val, sizeof(u32));
	xgene_sec_wr32(ctx, EIP96_CORE_CSR_BLOCK, IPE_PRNG_KEY_1_L_ADDR, val);
	get_random_bytes_arch(&val, sizeof(u32));
	xgene_sec_wr32(ctx, EIP96_CORE_CSR_BLOCK, IPE_PRNG_KEY_1_H_ADDR, val);
	get_random_bytes_arch(&val, sizeof(u32));
	xgene_sec_wr32(ctx, EIP96_CORE_CSR_BLOCK, IPE_PRNG_LFSR_L_ADDR, val);
	get_random_bytes_arch(&val, sizeof(u32));
	xgene_sec_wr32(ctx, EIP96_CORE_CSR_BLOCK, IPE_PRNG_LFSR_H_ADDR, val);

	xgene_sec_wr32(ctx, EIP96_CORE_CSR_BLOCK, IPE_PRNG_CTRL_ADDR,
		       SEC_ENABLE_F8_WR(1)
		       | AUTO_WR(1)
		       | RESULT_128_WR(1));

	return 0;
}

int xgene_sec_hwstart(struct xgene_sec_ctx *ctx)
{
	/* Start the EIP96 core */
	xgene_sec_wr32(ctx, EIP96_AXI_CSR_BLOCK, CSR_SEC_CFG_ADDR, GO_MASK);
	return 0;
}

int xgene_sec_hwstop(struct xgene_sec_ctx *ctx)
{
	/* Stop the EIP96 core */
	xgene_sec_wr32(ctx, EIP96_AXI_CSR_BLOCK, CSR_SEC_CFG_ADDR, 0x0);
	return 0;
}

int xgene_sec_hwreset(struct xgene_sec_ctx *ctx)
{
	/* Enable clock for core, EIP62, XTS, EIP96, QMI block */
	xgene_sec_wr32(ctx, CLK_RES_CSR_BLOCK, CSR_SEC_CLKEN_ADDR,
		       (SEC_EIP62_CLKEN_MASK |
			SEC_XTS_CLKEN_MASK |
			SEC_EIP96_CLKEN_MASK |
			SEC_AXI_CLKEN_MASK | SEC_CSR_CLKEN_MASK));
	/* Reset core, EIP62, XTS, EIP96, QMI block */
	xgene_sec_wr32(ctx, CLK_RES_CSR_BLOCK, CSR_SEC_SRST_ADDR,
		       0xFFFFFFFF & ~(SEC_EIP62_RESET_MASK |
				      SEC_XTS_RESET_MASK |
				      SEC_EIP96_RESET_MASK |
				      SEC_AXI_RESET_MASK | SEC_CSR_RESET_MASK));
	return 0;
}

int xgene_sec_qmsg_load_src_single(struct xgene_sec_ctx *ctx,
				   struct scatterlist *src,
				   struct xgene_qmtm_msg_ext8 *ext8,
				   int *nbytes, dma_addr_t * paddr,
				   int src_offset)
{
	u32 len;
	int rc;

	if (*paddr == 0) {
		rc = dma_map_sg(ctx->dev, src, 1, DMA_TO_DEVICE);
		if (!rc) {
			dev_err(ctx->dev, "dma_map_sg() src error\n");
			return rc;
		}
		*paddr = sg_dma_address(src);
		dma_unmap_sg(ctx->dev, src, 1, DMA_TO_DEVICE);
	}
	ext8->NxtDataAddrH = (u32) (*paddr >> 32);
	ext8->NxtDataAddrL = (u32) * paddr;
	len = sg_dma_len(src) - src_offset;
	if (len < 16 * 1024) {
		if (*nbytes < len) {
#if 0
		/* FIXME */
		dev_warn(ctx->dev, "spage length %d != %d (nbytes)\n",
			 len, *nbytes);
#endif
			len = *nbytes;
		}
		APMSEC_TXLOG("spage HW 0x%0llX len %d\n", *paddr, len);
		ext8->NxtBufDataLength = xgene_qmtm_encode_datalen(len);
		*nbytes -= len;
		*paddr = 0;
		return len;
	} else if (len == 16 * 1024) {
		APMSEC_TXLOG("spage HW 0x%0llX len %d\n", *paddr, 16 * 1024);
		ext8->NxtBufDataLength = 0;
		*nbytes -= 16 * 1024;
		*paddr = 0;
		return len;
	} else {
		APMSEC_TXLOG("spage HW 0x%0llX len %d\n", *paddr, 16 * 1024);
		ext8->NxtBufDataLength = 0;
		*nbytes -= 16 * 1024;
		*paddr += 16 * 1024;
		return 16 * 1024;
	}
}

int xgene_sec_load_src_buffers(struct xgene_sec_ctx *ctx,
			       struct xgene_qmtm_msg32 *msg,
			       struct xgene_qmtm_msg_ext32 *msgup32,
			       struct scatterlist *src, int nbytes,
			       struct sec_tkn_ctx *tkn)
{
	struct xgene_qmtm_msg16 *msg16 = &msg->msg16;
	struct xgene_qmtm_msg_ll8 *ext_msg_ll8;
	struct xgene_qmtm_msg_ext8 *ext_msg;
	dma_addr_t paddr;
	int offset;
	int ell_cnt;
	int ell_bcnt;
	int rc;
	int i;

	if (!nbytes) {
		dev_err(ctx->dev, "Zero length input not supported!\n");
		return -EINVAL;
	}

	/* Load first source buffer addr */
	paddr = 0;
	rc = xgene_sec_qmsg_load_src_single(ctx, src,
					    (struct xgene_qmtm_msg_ext8 *)
					    &msg16->DataAddrL, &nbytes, &paddr,
					    0);
	if (rc < 0)
		return rc;

	if (nbytes == 0)
		return 1;	/* single buffer - 32B msg */

	offset = rc;
	msg16->NV = 1;		/* More than 1 buffer */

	/* Load 2nd, 3rd, and 4th buffer addr */
	memset(msgup32, 0, sizeof(*msgup32));
	msgup32->msg8_2.NxtBufDataLength = 0x7800;
	msgup32->msg8_3.NxtBufDataLength = 0x7800;
	for (i = 0; nbytes > 0 && i < 3; i++) {
		if (!paddr) {
			src = sg_next(src);
			offset = 0;
		}

		switch (i) {
		case 0:
			rc = xgene_sec_qmsg_load_src_single(ctx, src,
							    &msgup32->msg8_1,
							    &nbytes, &paddr,
							    offset);
			break;
		case 1:
			rc = xgene_sec_qmsg_load_src_single(ctx, src,
							    &msgup32->msg8_2,
							    &nbytes, &paddr,
							    offset);
			break;
		case 2:
			rc = xgene_sec_qmsg_load_src_single(ctx, src,
							    &msgup32->msg8_3,
							    &nbytes, &paddr,
							    offset);
			break;
		}
		if (rc < 0)
			return rc;
		offset += rc;
	}
	if (nbytes == 0) {
		msgup32->msg8_4.NxtBufDataLength = 0x7800;
		return 2;	/* 2 - 4 buffers - 64B msg */
	}
	/* Load 5th buffer addr */
	if (!paddr) {
		src = sg_next(src);
		offset = 0;
	}
	if (nbytes <= 16 * 1024 && sg_is_last(src)) {
		rc = xgene_sec_qmsg_load_src_single(ctx, src, &msgup32->msg8_4,
						    &nbytes, &paddr, offset);
		if (rc < 0)
			return rc;
		return 2;	/* 5 buffers - 64B msg */
	}

	msg16->LL = 1;		/* Linked list of buffers 6 or more */

	tkn->src_addr_link = dma_alloc_coherent(ctx->dev,
						sizeof(struct
						       xgene_qmtm_msg_ext8) *
						APM_SEC_SRC_LINK_ADDR_MAX,
						&tkn->src_addr_link_paddr,
						GFP_ATOMIC);
	if (!tkn->src_addr_link) {
		dev_err(ctx->dev, "src addr list allocation failed\n");
		return -ENOMEM;
	}

	ext_msg_ll8 = &msgup32->msg8_ll;
	ext_msg_ll8->NxtDataPtrL = (u32) tkn->src_addr_link_paddr;
	ext_msg_ll8->NxtDataPtrH = (u32) (tkn->src_addr_link_paddr >> 32);
	ext_msg = tkn->src_addr_link;
	ell_bcnt = 0;
	ell_cnt = 0;
	for (i = 0; nbytes > 0 && i < APM_SEC_SRC_LINK_ADDR_MAX; i++) {
		/*
		 * Each field of QM Link List has 16 Bytes:
		 *              B8 - B15: First Buffer
		 *              B0 - B7 : Second Buffer
		 */
		rc = xgene_sec_qmsg_load_src_single(ctx, src,
						    &ext_msg[((i % 2) ?
						    (i - 1) : (i + 1))],
						    &nbytes, &paddr, offset);
		xgene_qmtm_msg_le32((u32 *) &
				    ext_msg[((i % 2) ? (i - 1) : (i + 1))],
				    sizeof(struct xgene_qmtm_msg_ext8) / 4);

		if (rc < 0)
			return rc;
		ell_bcnt += rc;
		ell_cnt++;
		if (!paddr) {
			src = sg_next(src);
			offset = 0;
		} else {
			offset += rc;
		}
	}

	/* Encode the extended link list byte count and link count */
	ext_msg_ll8->NxtLinkListength = ell_cnt;
	msg->msgup16.TotDataLengthLinkListLSBs = ell_bcnt & 0xFFF;
	ext_msg_ll8->TotDataLengthLinkListMSBs = (ell_bcnt & 0xFF000) >> 12;

	if (nbytes == 0)
		return 2;	/* 5 - 16 buffers - 64B msg */

	dev_err(ctx->dev, "source buffer length %d too long error %d",
		nbytes, -EINVAL);

	dma_free_coherent(ctx->dev,
			  sizeof(struct xgene_qmtm_msg_ext8) *
			  APM_SEC_SRC_LINK_ADDR_MAX,
			  tkn->src_addr_link, tkn->src_addr_link_paddr);
	tkn->src_addr_link = NULL;

	return -EINVAL;
}

void xgene_sec_qmesg_load_dst_single(struct xgene_sec_ctx *ctx,
				     struct xgene_qmtm_msg32 *msg, void *ptr,
				     int nbytes)
{
	dma_addr_t paddr;

	paddr = dma_map_single(ctx->dev, ptr, nbytes, DMA_FROM_DEVICE);
	dma_unmap_single(ctx->dev, paddr, nbytes, DMA_FROM_DEVICE);
	APMSEC_TXLOG("dpage HW 0x%0llX len %d\n", paddr, nbytes);
	msg->msgup16.H0Info_msbH = (u32) (paddr >> 32);
	msg->msgup16.H0Info_msbL = (u32) paddr;
}

int xgene_sec_load_dst_buffers(struct xgene_sec_ctx *ctx,
			       struct xgene_qmtm_msg32 *msg,
			       struct scatterlist *dst, u32 nbytes,
			       struct sec_tkn_ctx *tkn)
{
	struct xgene_qmtm_msg_up16 *msgup16 = &msg->msgup16;
	struct xgene_qmtm_msg_ext8 *ext_msg;
	dma_addr_t paddr;
	int offset;
	int ell_cnt;
	int rc;
	int i;

	APMSEC_TXLOG("dpage len %d dst len %d\n", nbytes, dst->length);
	if (nbytes == dst->length) {
		/* Single buffer */
		rc = dma_map_sg(ctx->dev, dst, 1, DMA_FROM_DEVICE);
		if (!rc) {
			dev_err(ctx->dev, "dma_map_sg() dst error\n");
			return rc;
		}
		paddr = sg_dma_address(dst);
		dma_unmap_sg(ctx->dev, dst, 1, DMA_FROM_DEVICE);
		msgup16->H0Info_msbH = (u32) (paddr >> 32);
		msgup16->H0Info_msbL = (u32) paddr;
		return 0;
	}

	tkn->dst_addr_link = dma_alloc_coherent(ctx->dev,
						sizeof(struct
						       xgene_qmtm_msg_ext8) *
						APM_SEC_DST_LINK_ADDR_MAX,
						&tkn->dst_addr_link_paddr,
						GFP_ATOMIC);
	if (!tkn->dst_addr_link) {
		dev_err(ctx->dev, "dest addr list allocation failed\n");
		return -ENOMEM;
	}
	msgup16->H0Info_msbH = (u32) (tkn->dst_addr_link_paddr >> 32);
	msgup16->H0Info_msbL = (u32) tkn->dst_addr_link_paddr;
	ext_msg = tkn->dst_addr_link;
	offset = 0;
	ell_cnt = 0;
	paddr = 0;
	for (i = 0; nbytes > 0 && i < APM_SEC_DST_LINK_ADDR_MAX; i++) {
		/*
		 * Each field of QM Link List has 16 Bytes:
		 *              B8 - B15: First Buffer
		 *              B0 - B7 : Second Buffer
		 */
		rc = xgene_sec_qmsg_load_src_single(ctx, dst,
						    &ext_msg[((i % 2) ?
						    (i - 1) : (i + 1))],
						    &nbytes, &paddr, offset);
		xgene_qmtm_msg_le32((u32 *) &
				    ext_msg[((i % 2) ? (i - 1) : (i + 1))],
				    sizeof(struct xgene_qmtm_msg_ext8) / 4);

		if (rc < 0)
			return rc;
		ell_cnt++;
		if (!paddr) {
			dst = sg_next(dst);
			offset = 0;
		} else {
			offset += rc;
		}
	}

	/* Encode the extended link list byte count and link count */
	msgup16->H0Info_msbH |= ((ell_cnt & 0xF0) >> 4) << 12;
	msgup16->H0Info_lsbH |= (ell_cnt & 0xF) << 6;

	/*
	 * Flag linked list destination - SEC CTL = 0x3 bit 0x2 already set
	 * when loading the source pointer.
	 */
	msgup16->H0Info_lsbH |= 0x1 << 14;

	if (nbytes == 0)
		return 0;

	dev_err(ctx->dev, "destination buffer length too long\n");

	dma_free_coherent(ctx->dev,
			  sizeof(struct xgene_qmtm_msg_ext8) *
			  APM_SEC_DST_LINK_ADDR_MAX,
			  tkn->dst_addr_link, tkn->dst_addr_link_paddr);
	tkn->dst_addr_link = NULL;

	return -EINVAL;
}

int xgene_sec_loadbuffer2qmsg(struct xgene_sec_ctx *ctx,
			      struct xgene_qmtm_msg32 *msg,
			      struct xgene_qmtm_msg_ext32 *qmsgext32,
			      struct sec_tkn_ctx *tkn)
{
	struct crypto_async_request *req = tkn->context;
	struct ablkcipher_request *ablk_req;
	struct ahash_request *ahash_req;
	struct aead_request *aead_req;
	int no_qmsg = 0;
	int ds;
	int rc;

	switch (crypto_tfm_alg_type(req->tfm)) {
	case CRYPTO_ALG_TYPE_AHASH:
		ahash_req = ahash_request_cast(req);
		rc = xgene_sec_load_src_buffers(ctx, msg, qmsgext32,
						ahash_req->src,
						ahash_req->nbytes, tkn);
		if (rc <= 0)
			break;
		no_qmsg = rc;
		ds = crypto_ahash_digestsize(__crypto_ahash_cast
					     (ahash_req->base.tfm));
		xgene_sec_qmesg_load_dst_single(ctx, msg, ahash_req->result,
						ds);
		break;
	case CRYPTO_ALG_TYPE_ABLKCIPHER:
		ablk_req = ablkcipher_request_cast(req);
		rc = xgene_sec_load_src_buffers(ctx, msg, qmsgext32,
						ablk_req->src,
						ablk_req->nbytes, tkn);
		if (rc <= 0)
			break;
		no_qmsg = rc;
		rc = xgene_sec_load_dst_buffers(ctx, msg, ablk_req->dst,
						ablk_req->nbytes, tkn);
		break;
	case CRYPTO_ALG_TYPE_AEAD:
		aead_req = container_of(req, struct aead_request, base);
		if (tkn->src_sg)
			rc = xgene_sec_load_src_buffers(ctx, msg, qmsgext32,
							tkn->src_sg,
							tkn->src_sg_nbytes,
							tkn);
		else
			rc = xgene_sec_load_src_buffers(ctx, msg, qmsgext32,
							aead_req->src,
							aead_req->cryptlen,
							tkn);
		if (rc <= 0)
			break;
		no_qmsg = rc;
		rc = xgene_sec_load_dst_buffers(ctx, msg, aead_req->dst,
						tkn->dest_nbytes, tkn);
		break;
	default:
		BUG();
		rc = -EINVAL;
		break;
	}

	return rc < 0 ? rc : no_qmsg;
}

u64 xgene_sec_encode2hwaddr(u64 paddr)
{
	return paddr >> 4;
}

u64 xgene_sec_decode2hwaddr(u64 paddr)
{
	return paddr << 4;
}

int xgene_sec_queue2hw(struct xgene_sec_session_ctx *session,
		       struct sec_tkn_ctx *tkn)
{
	struct xgene_sec_ctx *ctx = session->ctx;
	struct xgene_qmtm_qinfo *qinfo = &ctx->qm_queue.txq;
	struct xgene_qmtm_qinfo *fpinfo = &ctx->qm_queue.fpq[0];
	struct xgene_qmtm_qinfo *cpinfo = &ctx->qm_queue.cpq;
	struct xgene_qmtm_msg32 *qmsg;
	struct xgene_qmtm_msg_ext32 *qmsgext32;
	u64 hwaddr;
	int rc = 0;

	spin_lock_bh(&ctx->txlock);
	qmsg = &qinfo->msg32[qinfo->qhead];

	memset(qmsg, 0, sizeof(*qmsg));
	if (++qinfo->qhead == qinfo->count)
		qinfo->qhead = 0;
	qmsg->msg16.FPQNum = QMTM_QUEUE_ID(QMTM1, fpinfo->queue_id);
	qmsg->msg16.RType = QMTM_SLAVE_ID_SEC;
	qmsg->msg16.C = xgene_qmtm_coherent();
	hwaddr = xgene_sec_encode2hwaddr(tkn->result_tkn_hwptr);
	qmsg->msgup16.H0Info_lsbL = (u32) hwaddr;
	qmsg->msgup16.H0Info_lsbH = (u32) (hwaddr >> 32);
	qmsg->msgup16.H0Info_lsbH |= 0x2 << 14;	/* SEC CTL = 0x2 */
	qmsg->msgup16.H0Enq_Num = QMTM_QUEUE_ID(QMTM1, cpinfo->queue_id);

	/* Only used if 64B message */
	qmsgext32 = (struct xgene_qmtm_msg_ext32 *)&qinfo->msg32[qinfo->qhead];

	/* Load src/dest address into QM message. Will return number of 32B */
	rc = xgene_sec_loadbuffer2qmsg(ctx, qmsg, qmsgext32, tkn);
	if (rc <= 0) {
		dev_err(ctx->dev, "operation submitted error %d\n", rc);
		qinfo->qhead--;
		goto done;
	}
	if (rc == 2) {
		if (++qinfo->qhead == qinfo->count)
			qinfo->qhead = 0;
	}

	APMSEC_SADUMP((u32 *) tkn->sa->sa_ptr, tkn->sa->sa_len);
	APMSEC_TKNDUMP(tkn);
	APMSEC_TXLOG("SEC MSG QID %d CQID %d 0x%08X FPQID %d 0x%08X\n",
		     qinfo->queue_id,
		     cpinfo->queue_id, qmsg->msgup16.H0Enq_Num,
		     fpinfo->queue_id, qmsg->msg16.FPQNum);
	APMSEC_TXLOG("SEC MSG data Addr 0x%X.%08X len 0x%08X\n",
		     qmsg->msg16.DataAddrH, qmsg->msg16.DataAddrL,
		     qmsg->msg16.BufDataLen);
	APMSEC_TXLOG("SEC MSG token Addr 0x%X.%08X\n",
		     qmsg->msgup16.H0Info_lsbH, qmsg->msgup16.H0Info_lsbL);
	APMSEC_TXLOG("SEC MSG dest Addr 0x%X.%08X\n",
		     qmsg->msgup16.H0Info_msbH, qmsg->msgup16.H0Info_msbL);
	APMSEC_QMSGDUMP("SEC QMSG: ", qmsg, sizeof(*qmsg));
	if (rc == 2)
		APMSEC_QMSGDUMP("SEC QMSG: ", qmsgext32, sizeof(*qmsgext32));

	xgene_qmtm_msg_le32((u32 *) qmsg, 8);
	if (qmsg->msg16.NV)
		xgene_qmtm_msg_le32((u32 *) qmsgext32, 8);

	/* Tell QM HW message queued */
	writel(rc, qinfo->command);
	APMSEC_TXLOG("operation submitted %d 32B msg\n", rc);
	rc = 0;

done:
	spin_unlock_bh(&ctx->txlock);
	return rc;
}

/*
 * SA and Token Management Functions
 */
struct sec_tkn_ctx *xgene_sec_tkn_get(struct xgene_sec_session_ctx *session,
				      u8 * new_tkn)
{
	struct sec_tkn_ctx *tkn;
	unsigned long flags;
	dma_addr_t paddr;
	int tkn_size;

	spin_lock_irqsave(&session->lock, flags);
	if (!list_empty(&session->tkn_cache)) {
		struct list_head *entry = session->tkn_cache.next;
		list_del(entry);
		--session->tkn_cache_cnt;
		tkn = list_entry(entry, struct sec_tkn_ctx, next);
		spin_unlock_irqrestore(&session->lock, flags);
		*new_tkn = 0;
		APMSEC_SATKNLOG("allocate tkn cached 0x%p\n", tkn);
		return tkn;
	}
	spin_unlock_irqrestore(&session->lock, flags);

	*new_tkn = 1;
	tkn_size = session->tkn_max_len;
	tkn = dma_alloc_coherent(session->ctx->dev, tkn_size, &paddr,
				 GFP_ATOMIC);
	if (tkn == NULL)
		goto done;
	memset(tkn, 0, tkn_size);
	tkn->tkn_paddr = paddr;
	tkn->input_tkn_len = session->tkn_input_len;
	tkn->result_tkn_ptr = TKN_CTX_RESULT_TKN_COMPUTE(tkn);
	tkn->result_tkn_hwptr = paddr + TKN_CTX_RESULT_TKN_OFFSET(tkn);
	tkn->context = session;
	APMSEC_SATKNLOG("allocate tkn 0x%p size %d (%d)\n",
			tkn, tkn_size, session->tkn_input_len);
done:
	return tkn;
}

static void xgene_sec_tkn_free_mem(struct device *dev, struct sec_tkn_ctx *tkn)
{

	if (tkn->src_sg) {
		/* Free source scatterlist */
		dma_free_coherent(dev,
				  sizeof(struct scatterlist) *
				  tkn->src_sg_nents,
				  (void *)tkn->src_sg, tkn->src_sg_paddr);
		tkn->src_sg = NULL;
	}
	if (tkn->src_addr_link) {
		/* Free extended QM source link list */
		dma_free_coherent(dev,
				  sizeof(struct xgene_qmtm_msg_ext8) *
				  APM_SEC_SRC_LINK_ADDR_MAX,
				  tkn->src_addr_link, tkn->src_addr_link_paddr);
		tkn->src_addr_link = NULL;
	}
	if (tkn->dst_addr_link) {
		/* Free extended QM destination link list */
		dma_free_coherent(dev,
				  sizeof(struct xgene_qmtm_msg_ext8) *
				  APM_SEC_DST_LINK_ADDR_MAX,
				  tkn->dst_addr_link, tkn->dst_addr_link_paddr);
		tkn->dst_addr_link = NULL;
	}
}

void __xgene_sec_tkn_free(struct xgene_sec_session_ctx *session,
			  struct sec_tkn_ctx *tkn)
{
	xgene_sec_tkn_free_mem(session->ctx->dev, tkn);

	dma_free_coherent(session->ctx->dev, session->tkn_max_len, tkn,
			  tkn->tkn_paddr);
	APMSEC_SATKNLOG("free tkn 0x%p\n", tkn);
}

void xgene_sec_tkn_free(struct xgene_sec_session_ctx *session,
			struct sec_tkn_ctx *tkn)
{
	unsigned long flags;

	/* Free mem before add tkn into cache */
	xgene_sec_tkn_free_mem(session->ctx->dev, tkn);

	spin_lock_irqsave(&session->lock, flags);
	if (session->tkn_cache_cnt < APM_SEC_TKN_CACHE_MAX) {
		++session->tkn_cache_cnt;
		list_add(&tkn->next, &session->tkn_cache);
		spin_unlock_irqrestore(&session->lock, flags);
		APMSEC_SATKNLOG("free tkn cached 0x%p\n", tkn);
		return;
	}
	spin_unlock_irqrestore(&session->lock, flags);

	__xgene_sec_tkn_free(session, tkn);
}

struct sec_sa_item *xgene_sec_sa_get(struct xgene_sec_session_ctx *session)
{
	struct sec_sa_item *sa;
	unsigned long flags;
	dma_addr_t paddr;
	int sa_size;

	spin_lock_irqsave(&session->lock, flags);
	if (!list_empty(&session->sa_cache)) {
		struct list_head *entry = session->sa_cache.next;
		list_del(entry);
		--session->sa_cache_cnt;
		spin_unlock_irqrestore(&session->lock, flags);
		sa = list_entry(entry, struct sec_sa_item, next);
		sa->sa_len = session->sa_len;
		APMSEC_SATKNLOG("allocate sa cached 0x%p aligned 0x%p\n",
				sa, sa->sa_ptr);
		return sa;
	}
	spin_unlock_irqrestore(&session->lock, flags);

	sa_size = sizeof(struct sec_sa_item) + 15 + 8 + session->sa_max_len;
	sa = dma_alloc_coherent(session->ctx->dev, sa_size, &paddr, GFP_ATOMIC);
	if (sa == NULL)
		goto done;
	memset(sa, 0, sa_size);
	sa->sa_paddr = paddr;
	sa->sa_total_len = sa_size;
	sa->sa_len = session->sa_len;
	sa->sa_ptr = SA_PTR_COMPUTE(sa);
	sa->sa_hwptr = paddr + SA_PTR_OFFSET(sa);
	APMSEC_SATKNLOG("allocate sa 0x%p aligned 0x%p size %d\n",
			sa, sa->sa_ptr, sa_size);
done:
	return sa;
}

void __xgene_sec_sa_free(struct xgene_sec_session_ctx *session,
			 struct sec_sa_item *sa)
{
	dma_free_coherent(session->ctx->dev, sa->sa_total_len, sa,
			  sa->sa_paddr);
	APMSEC_SATKNLOG("free sa 0x%p\n", sa);
}

void xgene_sec_sa_free(struct xgene_sec_session_ctx *session,
		       struct sec_sa_item *sa)
{
	unsigned long flags;

	spin_lock_irqsave(&session->lock, flags);
	if (session->sa_cache_cnt < APM_SEC_SA_CACHE_MAX) {
		++session->sa_cache_cnt;
		list_add(&sa->next, &session->sa_cache);
		spin_unlock_irqrestore(&session->lock, flags);
		APMSEC_SATKNLOG("free sa cached 0x%p\n", sa);
		return;
	}
	spin_unlock_irqrestore(&session->lock, flags);

	__xgene_sec_sa_free(session, sa);
}

void xgene_sec_session_init(struct xgene_sec_session_ctx *session)
{
	memset(session, 0, sizeof(*session));
	session->ctx = xg_ctx;
	INIT_LIST_HEAD(&session->tkn_cache);
	INIT_LIST_HEAD(&session->sa_cache);
	spin_lock_init(&session->lock);
}

void xgene_sec_session_free(struct xgene_sec_session_ctx *session)
{
	struct sec_tkn_ctx *tkn;
	struct sec_tkn_ctx *tkn_tmp;
	struct sec_sa_item *sa;
	struct sec_sa_item *sa_tmp;

	list_for_each_entry_safe(sa, sa_tmp, &session->sa_cache, next)
	    __xgene_sec_sa_free(session, sa);
	if (session->sa) {
		__xgene_sec_sa_free(session, session->sa);
		session->sa = NULL;
	}
	INIT_LIST_HEAD(&session->sa_cache);
	list_for_each_entry_safe(tkn, tkn_tmp, &session->tkn_cache, next)
	    __xgene_sec_tkn_free(session, tkn);
	INIT_LIST_HEAD(&session->tkn_cache);
}

int xgene_sec_create_sa_tkn_pool(struct xgene_sec_session_ctx *session,
				 u32 sa_max_len, u32 sa_len,
				 char sa_ib, u32 tkn_len)
{
	int rc = 0;

	session->tkn_max_len = TKN_CTX_SIZE(tkn_len);
	session->tkn_input_len = tkn_len;
	session->sa_len = sa_len;
	session->sa_max_len = sa_max_len;

	if (session->sa == NULL) {
		session->sa = xgene_sec_sa_get(session);
		if (session->sa == NULL)
			rc = -ENOMEM;
	} else {
		session->sa->sa_len = sa_len;
	}
	if (!rc && session->sa_ib == NULL && sa_ib) {
		session->sa_ib = xgene_sec_sa_get(session);
		if (session->sa_ib == NULL)
			rc = -ENOMEM;
	} else if (session->sa_ib) {
		session->sa_ib->sa_len = sa_len;
	}
	return rc;
}

/*
 * Completed operations processing functions
 */
int xgene_sec_tkn_cb(struct sec_tkn_ctx *tkn)
{
	struct xgene_sec_ctx *ctx;
	struct crypto_async_request *req;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_result_hdr *result_tkn;
	struct xgene_sec_session_ctx *session;
	int rc = 0;
	int i;

	int *sa_context;

#if 0
	/* FIXME */
	int esp_nh_padlen = 0;
	int pad_verify_prot;
#endif

	APMSEC_RXLOG("Process completed tkn 0x%p\n", tkn);

	req = tkn->context;
	session = crypto_tfm_ctx(req->tfm);
	sa_context = (u32 *) session->sa->sa_ptr;
	ctx = session->ctx;

	if ((i = atomic_dec_return(&ctx->qm_queue.active)) < 0) {
		dev_err(ctx->dev, "invalid active %d\n", i);
		BUG();
	}

	result_tkn = TKN_CTX_RESULT_TKN(tkn);
	APMSEC_RXDUMP("Result SW Token ", result_tkn, TKN_RESULT_HDR_MAX_LEN);
	xgene_qmtm_msg_le32((u32 *) result_tkn, TKN_RESULT_HDR_MAX_LEN / 4);

	if (crypto_tfm_alg_type(req->tfm) == CRYPTO_ALG_TYPE_ABLKCIPHER)
		rctx = ablkcipher_request_ctx(ablkcipher_request_cast(req));
	else if (crypto_tfm_alg_type(req->tfm) == CRYPTO_ALG_TYPE_AHASH)
		rctx = ahash_request_ctx(ahash_request_cast(req));
	else
		rctx = aead_request_ctx(container_of(req,
						     struct aead_request,
						     base));

	if (result_tkn->EXX || result_tkn->E15) {
		if (result_tkn->EXX & (TKN_RESULT_E9 | TKN_RESULT_E10 |
				       TKN_RESULT_E11 | TKN_RESULT_E12 |
				       TKN_RESULT_E13))
			rc = -EBADMSG;
		else
			rc = -ENOSYS;
		dev_err(ctx->dev, "EIP96 hardware error %d\n", rc);
		/* apm_sec_dump_src_dst_buf(req, tkn); - FIXME */
		goto out;
	}
#if 0
	/* FIXME */
	if (result_tkn->H || result_tkn->L || result_tkn->N ||
	    result_tkn->C || result_tkn->B) {
		/*
		 * packet exceeded 1792 bytes, result appended at end
		 * of result data
		 */
		dev_err(ctx->dev, "Unexpected result token with appeded data");
	}

	if (tkn->dest_mem)
		apm_cp_buf2sg(tkn, req);

	pad_verify_prot = tkn->sa->sa_ptr->pad_type;
	if (pad_verify_prot == SA_PAD_TYPE_SSL ||
	    pad_verify_prot == SA_PAD_TYPE_TLS) {
		rc = 0;
	} else {
		esp_nh_padlen = result_tkn->next_hdr_field |
		    (result_tkn->pad_length << 8);
		rc = esp_nh_padlen ? esp_nh_padlen : 0;
	}
#endif

out:
	if (rctx->tkn->flags & (u32) TKN_FLAG_CONTINOUS) {
		/*
		 * HASH: Copy the hash result as inner digest for next
		 * continuous hash operation
		 */
		if (crypto_tfm_alg_type(req->tfm) == CRYPTO_ALG_TYPE_AHASH) {
			memcpy((void *)&sa_context[2],
			       (void *)ahash_request_cast(req)->result,
			       sec_sa_compute_digest_len(session->sa->sa_ptr->
							 hash_alg,
							 SA_DIGEST_TYPE_INNER));
			APMSEC_RXDUMP("PAYLOAD: ",
				      (void *)ahash_request_cast(req)->result,
				      sec_sa_compute_digest_len(session->sa->
								sa_ptr->
								hash_alg,
								SA_DIGEST_TYPE_INNER));
		}
	} else {
		xgene_sec_tkn_free(session, tkn);
		rctx->tkn = NULL;
		if (rctx->sa) {
			xgene_sec_sa_free(session, rctx->sa);
			rctx->sa = NULL;
		}
	}

	/* Notify packet completed */
	req->complete(req, rc);
	return 0;
}

static void xgene_sec_bh_tasklet_cb(unsigned long data)
{
#define XGENE_SEC_POLL_BUDGET	64
	struct xgene_sec_ctx *ctx;
	int budget = XGENE_SEC_POLL_BUDGET;
	struct xgene_qmtm_qinfo *qdesc;
	struct xgene_qmtm_msg32 *qbase;
	struct xgene_qmtm_msg32 *msg32;
	struct xgene_qmtm_msg_ext32 *msgup32;
	struct sec_tkn_ctx *tkn;
	u32 command = 0;
	u64 paddr;

	ctx = (struct xgene_sec_ctx *)data;

	/* Process pending token */
	spin_lock_bh(&ctx->lock);

	qdesc = &ctx->qm_queue.cpq;
	qbase = qdesc->msg32;

	while (budget--) {
		/* Check if actual message available */
		msg32 = &qbase[qdesc->qhead];
		if (unlikely(((u32 *) msg32)[EMPTY_SLOT_INDEX] == EMPTY_SLOT))
			break;
		xgene_qmtm_msg_le32((u32 *) msg32, 8);
		--command;
		if (++qdesc->qhead == qdesc->count)
			qdesc->qhead = 0;
		if (msg32->msg16.NV) {
			/* 64B message */
			msgup32 = (struct xgene_qmtm_msg_ext32 *)
			    &qbase[qdesc->qhead];
			xgene_qmtm_msg_le32((u32 *) msgup32, 8);
			--command;
			if (++qdesc->qhead == qdesc->count)
				qdesc->qhead = 0;
		} else {
			msgup32 = NULL;
		}

		/* Handle QM programming error */
		if (msg32->msg16.ELErr || msg32->msg16.LErr)
			xgene_sec_hdlr_qerr(ctx, msg32->msg16.LEI,
					    msg32->msg16.ELErr << 3 |
					    msg32->msg16.LErr);

		paddr = msg32->msgup16.H0Info_lsbH & 0x3F;
		paddr <<= 32;
		paddr |= msg32->msgup16.H0Info_lsbL;
		paddr = xgene_sec_decode2hwaddr(paddr);
		/* FIXME */
		tkn = __va(TKN_CTX_HWADDR2TKN(paddr));

		/* Process the completed token */
		xgene_sec_tkn_cb(tkn);

		((u32 *) msg32)[EMPTY_SLOT_INDEX] = EMPTY_SLOT;
		if (msgup32)
			((u32 *) msgup32)[EMPTY_SLOT_INDEX] = EMPTY_SLOT;
	}
	/* Tell QM HW we processsed x 32B messages */
	dev_dbg(ctx->dev, "processed %d 32B message\n", command);
	writel(command, qdesc->command);

	spin_unlock_bh(&ctx->lock);

	/* Re-enable IRQ */
	enable_irq(ctx->qm_queue.cpq.irq);
}

static irqreturn_t xgene_sec_qmsg_isr(int value, void *id)
{
	struct xgene_sec_ctx *ctx = id;

	disable_irq_nosync(ctx->qm_queue.cpq.irq);
	tasklet_schedule(&ctx->tasklet);
	return IRQ_HANDLED;
}

/*
 * Request enqueue processing functions
 */
static inline int apm_sec_hw_queue_available(struct xgene_sec_ctx *ctx)
{
	return atomic_read(&ctx->qm_queue.active) >= MAX_SLOT ? 0 : 1;
}

static int xgene_sec_send2hwq(struct crypto_async_request *req)
{
	struct xgene_sec_session_ctx *session;
	struct xgene_sec_ctx *ctx;
	struct xgene_sec_req_ctx *rctx;
	int rc;

	switch (crypto_tfm_alg_type(req->tfm)) {
	case CRYPTO_ALG_TYPE_ABLKCIPHER:
		rctx = ablkcipher_request_ctx(ablkcipher_request_cast(req));
		session =
		    crypto_tfm_ctx(ablkcipher_request_cast(req)->base.tfm);
		break;
	case CRYPTO_ALG_TYPE_AEAD:
		rctx =
		    aead_request_ctx(container_of
				     (req, struct aead_request, base));
		session =
		    crypto_tfm_ctx(container_of
				   (req, struct aead_request, base)->base.tfm);
		break;
	case CRYPTO_ALG_TYPE_AHASH:
		rctx = ahash_request_ctx(ahash_request_cast(req));
		session = crypto_tfm_ctx(ahash_request_cast(req)->base.tfm);
		break;
	default:
		BUG();
		break;
	}
	ctx = session->ctx;
	atomic_inc(&ctx->qm_queue.active);

	rc = xgene_sec_queue2hw(session, rctx->tkn);
	if (rc != 0) {
		dev_err(ctx->dev, "failed submission error 0x%08X\n", rc);
		atomic_dec(&ctx->qm_queue.active);
	} else
		rc = -EINPROGRESS;

	return rc;

}

static int xgene_sec_handle_req(struct xgene_sec_ctx *ctx,
				struct crypto_async_request *req)
{
	int ret = -EAGAIN;

	if (apm_sec_hw_queue_available(ctx))
		ret = xgene_sec_send2hwq(req);

	if (ret == -EAGAIN) {
		unsigned long flags;
		spin_lock_irqsave(&ctx->lock, flags);
		ret = crypto_enqueue_request(&ctx->queue, req);
		spin_unlock_irqrestore(&ctx->lock, flags);
	}
	return ret;
}

static int xgene_sec_process_queue(struct xgene_sec_ctx *ctx)
{
	struct crypto_async_request *req;
	unsigned long flags;
	int err = 0;

	while (apm_sec_hw_queue_available(ctx)) {
		spin_lock_irqsave(&ctx->lock, flags);
		req = crypto_dequeue_request(&ctx->queue);
		spin_unlock_irqrestore(&ctx->lock, flags);

		if (!req)
			break;
		err = xgene_sec_handle_req(ctx, req);
		if (err)
			break;
	}
	return err;
}

int xgene_sec_setup_crypto(struct xgene_sec_ctx *ctx,
			   struct crypto_async_request *req)
{
	int err;

	err = xgene_sec_handle_req(ctx, req);
	if (err)
		return err;

	if (apm_sec_hw_queue_available(ctx) && ctx->queue.qlen)
		err = xgene_sec_process_queue(ctx);
	return err;
}

/*
 * Algorithm registration functions
 */
int xgene_sec_alg_init(struct crypto_tfm *tfm)
{
	struct crypto_alg *alg = tfm->__crt_alg;
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);

	xgene_sec_session_init(session);

	if (alg->cra_type == &crypto_ablkcipher_type)
		tfm->crt_ablkcipher.reqsize = sizeof(struct xgene_sec_req_ctx);
	else if (alg->cra_type == &crypto_ahash_type)
		crypto_ahash_set_reqsize(__crypto_ahash_cast(tfm),
					 sizeof(struct xgene_sec_req_ctx));
	else if (alg->cra_type == &crypto_aead_type)
		tfm->crt_aead.reqsize = sizeof(struct xgene_sec_req_ctx);

	return 0;
}

void xgene_sec_alg_exit(struct crypto_tfm *tfm)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	xgene_sec_session_free(session);
}

static int xgene_sec_register_alg(struct xgene_sec_ctx *ctx)
{
	struct xgene_sec_alg *alg;
	struct crypto_alg *cipher;
	int rc = 0;
	int i;

	for (i = 0; xgene_sec_alg_tlb[i].type != 0; i++) {
		alg = devm_kzalloc(ctx->dev, sizeof(struct xgene_sec_alg),
				   GFP_KERNEL);
		if (!alg)
			return -ENOMEM;
		alg->type = xgene_sec_alg_tlb[i].type;
		switch (alg->type) {
		case CRYPTO_ALG_TYPE_AHASH:
			alg->u.hash = xgene_sec_alg_tlb[i].u.hash;
			cipher = &alg->u.hash.halg.base;
			break;
		default:
			alg->u.cipher = xgene_sec_alg_tlb[i].u.cipher;
			cipher = &alg->u.cipher;
			break;
		}
		INIT_LIST_HEAD(&cipher->cra_list);
		if (!cipher->cra_init)
			cipher->cra_init = xgene_sec_alg_init;
		if (!cipher->cra_exit)
			cipher->cra_exit = xgene_sec_alg_exit;
		if (!cipher->cra_module)
			cipher->cra_module = THIS_MODULE;
		if (!cipher->cra_module)
			cipher->cra_priority = XGENE_SEC_CRYPTO_PRIORITY;
		switch (alg->type) {
		case CRYPTO_ALG_TYPE_AHASH:
			rc = crypto_register_ahash(&alg->u.hash);
			break;
		default:
			rc = crypto_register_alg(&alg->u.cipher);
			break;
		}
		if (rc) {
			dev_err(ctx->dev,
				"failed to register alg %s error %d\n",
				cipher->cra_name, rc);
			list_del(&alg->entry);
			devm_kfree(ctx->dev, alg);
			return rc;
		}
		list_add_tail(&alg->entry, &ctx->alg_list);
	}
	return rc;
}

static void xgene_sec_unregister_alg(struct xgene_sec_ctx *ctx)
{
	struct xgene_sec_alg *alg, *tmp;

	list_for_each_entry_safe(alg, tmp, &ctx->alg_list, entry) {
		list_del(&alg->entry);
		switch (alg->type) {
		case CRYPTO_ALG_TYPE_AHASH:
			crypto_unregister_ahash(&alg->u.hash);
			break;

		default:
			crypto_unregister_alg(&alg->u.cipher);
			break;
		}
		devm_kfree(ctx->dev, alg);
	}
}

static int xgene_sec_get_str_param(struct platform_device *pdev,
				   const char *of_name, char *acpi_name,
				   char *buf, int len)
{
	int rc;
	const char *param;
#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT)) {
		acpi_status status;
		struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
		union acpi_object *acpi_obj;
		status = acpi_evaluate_object(ACPI_HANDLE(&pdev->dev),
					      acpi_name, NULL, &buffer);
		if (ACPI_FAILURE(status))
			return -ENODEV;
		acpi_obj = buffer.pointer;
		if (acpi_obj->type != ACPI_TYPE_STRING) {
			buf[0] = '\0';
			kfree(buffer.pointer);
			return -ENODEV;
		}
		if (acpi_obj->string.length < len) {
			strncpy(buf, acpi_obj->string.pointer,
				acpi_obj->string.length);
			buf[acpi_obj->string.length] = '\0';
		} else {
			strncpy(buf, acpi_obj->string.pointer, len);
			buf[len - 1] = '\0';
		}
		kfree(buffer.pointer);
		return 0;
	}
#endif
	if (of_name == NULL)
		return -ENODEV;
	rc = of_property_read_string(pdev->dev.of_node, of_name, &param);
	if (rc == 0) {
		strncpy(buf, param, len);
		buf[len - 1] = '\0';
	} else {
		buf[0] = '\0';
	}
	return rc;
}

static int xgene_sec_get_byte_param(struct platform_device *pdev,
				    const char *of_name, char *acpi_name,
				    u8 * buf, int len)
{
	u32 *tmp;
	int rc = 0;
	int i;
#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT)) {
		struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
		union acpi_object *obj;
		acpi_status status;
		if (acpi_name == NULL)
			return -ENODEV;
		status = acpi_evaluate_object(ACPI_HANDLE(&pdev->dev),
					      acpi_name, NULL, &buffer);
		if (ACPI_FAILURE(status))
			return -ENODEV;
		obj = (union acpi_object *)buffer.pointer;
		if (!obj)
			return -ENODEV;
		if (obj->type != ACPI_TYPE_BUFFER) {
			kfree(obj);
			return -ENODEV;
		}
		memset(buf, 0, len);
		if (obj->buffer.length >= len)
			memcpy(buf, obj->buffer.pointer, len);
		else
			memcpy(buf, obj->buffer.pointer, obj->buffer.length);
		kfree(obj);
		return 0;
	}
#endif
	if (of_name == NULL)
		return -ENODEV;
	tmp = kmalloc(sizeof(u32) * len, GFP_ATOMIC);
	if (tmp == NULL)
		return -ENOMEM;
	rc = of_property_read_u32_array(pdev->dev.of_node, of_name, tmp, len);
	if (rc != 0)
		goto done;
	for (i = 0; i < len; i++)
		buf[i] = tmp[i] & 0xFF;
done:
	kfree(tmp);
	return rc;
}

static int xgene_sec_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct xgene_sec_ctx *ctx;
	struct resource *res;
	struct xgene_qmtm_sdev *sdev;
	u32 qmtm_ip;
	char name[32];
	u8 info[5];
	int rc;
	u8 wq_pbn_start, wq_pbn_count, fq_pbn_start, fq_pbn_count;

	dev_dbg(ctx->dev, "Initialize the security hardware\n");

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		dev_err(dev, "can't allocate security context\n");
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&ctx->alg_list);
	spin_lock_init(&ctx->lock);
	spin_lock_init(&ctx->txlock);
	atomic_set(&ctx->qm_queue.active, 0);
	ctx->dev = dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "no CSR space\n");
		rc = -EINVAL;
		goto err_hw;
	}

	ctx->csr = devm_ioremap_resource(dev, res);
	if (!ctx->csr) {
		dev_err(dev, "can't map %pR\n", res);
		rc = -ENOMEM;
		goto err_hw;
	}

	/* Setup CSR address pointer */
	ctx->clk_csr = ctx->csr + APM_SEC_CLK_RES_CSR_OFFSET;
	ctx->diag_csr = ctx->csr + APM_SEC_GLBL_DIAG_OFFSET;
	ctx->eip96_axi_csr = ctx->csr + APM_EIP96_AXI_CSR_OFFSET;
	ctx->eip96_csr = ctx->csr + APM_EIP96_CSR_OFFSET;
	ctx->eip96_core_csr = ctx->csr + APM_EIP96_CORE_CSR_OFFSET;
	ctx->ctrl_csr = ctx->csr + APM_SEC_GLBL_CTRL_CSR_OFFSET;
	ctx->qmi_ctl_csr = ctx->csr + APM_QMI_CTL_OFFSET;

	/* Init tasklet for bottom half processing */
	tasklet_init(&ctx->tasklet, xgene_sec_bh_tasklet_cb,
		     (unsigned long)ctx);

	/* Initialize software queue to 1 */
	crypto_init_queue(&ctx->queue, 1);

	/* Take IP out of reset */
	/* FIXME - replace with clock interface */
	rc = xgene_sec_hwreset(ctx);
	if (rc != 0)
		goto err_hw;

	/* Remove Security CSR memory from shutdown */
	rc = xgene_sec_init_memram(ctx);
	if (rc != 0)
		goto err_hw;

	/* Initialize the security hardware */
	rc = xgene_sec_hwinit(ctx);
	if (rc != 0)
		goto err_hw;

	/* Start the security hardware */
	rc = xgene_sec_hwstart(ctx);
	if (rc != 0)
		goto err_hw;

	/* Get slave context */
	xgene_sec_get_str_param(pdev, "slave_name", "SLNM", name, sizeof(name));
	xgene_sec_get_byte_param(pdev, "slave_info", "SLIF", info,
				 ARRAY_SIZE(info));
	qmtm_ip = info[0];
	wq_pbn_start = info[1];
	wq_pbn_count = info[2];
	fq_pbn_start = info[3];
	fq_pbn_count = info[4];
	if (!(sdev = xgene_qmtm_set_sdev(name, qmtm_ip, wq_pbn_start,
					 wq_pbn_count, fq_pbn_start,
					 fq_pbn_count))) {
		dev_err(dev, "QMTM%d Slave %s error\n", qmtm_ip, name);
		rc = -ENODEV;
		goto err_hw;
	}

	dev_set_drvdata(dev, ctx);
	xg_ctx = ctx;

	/* QM queue configuration */
	if (xgene_sec_qconfig(ctx))
		goto err_hw;

	/* Request for interrupt on completed operation */
	if (!crypto_uio) {
		rc = request_irq(ctx->qm_queue.cpq.irq, xgene_sec_qmsg_isr, 0,
				 "Crypto", ctx);
		if (rc) {
			dev_err(ctx->dev, "Failed to register IRQ %d\n",
				ctx->qm_queue.cpq.irq);
			goto err_hw;
		}
	}

	/* Setup IRQ */
	ctx->irq = platform_get_irq(pdev, 0);
	if (ctx->irq <= 0) {
		dev_err(dev, "no IRQ in DTS\n");
		rc = -ENODEV;
		goto err_hw;
	}
	rc = request_irq(ctx->irq, xgene_sec_intr_cb, 0, "crypto-err", ctx);
	if (rc != 0) {
		dev_err(dev,
			"security core can not register for interrupt %d\n",
			ctx->irq);
		goto err_reg_alg;
	}

	dev->dma_mask = &dev->coherent_dma_mask;
	dev->coherent_dma_mask = DMA_BIT_MASK(64);

	/* Register security algorithms with Linux CryptoAPI */
	if (!crypto_uio) {
		rc = xgene_sec_register_alg(ctx);
		if (rc)
			goto err_reg_alg;
	} else {
		xgene_sec_uio_init(pdev, ctx);
	}

	dev_info(dev, "APM X-Gene SoC security accelerator driver\n");
	return 0;

err_reg_alg:
	if (ctx->irq != 0)
		free_irq(ctx->irq, "crypto");
	xgene_sec_hwstop(ctx);

err_hw:
	devm_kfree(dev, ctx);
	return rc;
}

static int xgene_sec_remove(struct platform_device *pdev)
{
	struct xgene_sec_ctx *ctx = dev_get_drvdata(&pdev->dev);

	/* Un-register with Linux CryptoAPI */
	if (!crypto_uio)
		xgene_sec_unregister_alg(ctx);
	else
		xgene_sec_uio_deinit(ctx);

	/* Stop hardware */
	xgene_sec_hwstop(ctx);

	/* Clean up pending queue */
	/* FIXME - ctx->queue */

	dev_dbg(ctx->dev,
		"unloaded APM X-Gene SoC security accelerator driver\n");

	devm_kfree(&pdev->dev, ctx);

	return 0;
}

static struct of_device_id xgene_sec_match[] = {
	{.compatible = "apm,xgene-crypto",},
	{},
};

static struct platform_driver xgene_sec_driver = {
	.driver = {
		   .name = "xgene-crypto",
		   .owner = THIS_MODULE,
		   .of_match_table = xgene_sec_match,
		   },
	.probe = xgene_sec_probe,
	.remove = xgene_sec_remove,
};

module_platform_driver(xgene_sec_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Loc Ho <lho@apm.com>");
MODULE_DESCRIPTION("APM X-Gene SoC security hw accelerator");

#if 0
/* FIXME */
#define APM_SEC_HW_QUEUE_LEN		1000
static int apm_cp_buf2sg(struct sec_tkn_ctx *tkn,
			 struct crypto_async_request *req)
{
	int nbytes = tkn->dest_nbytes;
	void *dest = tkn->dest_mem;
	int i;
	int len;
	void *dpage;
	char *daddr;
	struct scatterlist *dst;

	APMSEC_DEBUG_DUMP("RESULT dump", tkn->dest_mem, tkn->dest_nbytes);
	if (crypto_tfm_alg_type(req->tfm) == CRYPTO_ALG_TYPE_ABLKCIPHER) {
		struct ablkcipher_request *ablk_req;
		ablk_req = ablkcipher_request_cast(req);
		dst = ablk_req->dst;
	} else if (crypto_tfm_alg_type(req->tfm) == CRYPTO_ALG_TYPE_AEAD) {
		struct aead_request *aead_req;
		aead_req = container_of(req, struct aead_request, base);
		dst = aead_req->dst;
	} else {
		BUG();
	}

	len = dst[0].length;
	dpage = kmap_atomic(sg_page(&dst[0]));
	daddr = dpage + dst[0].offset;
	memcpy(daddr, dest, len);
	kunmap_atomic(dpage);
	nbytes -= len;
	dest += len;
	for (i = 1; nbytes > 0; i++) {
		dpage = kmap_atomic(sg_page(&dst[i]));
		daddr = dpage + dst[i].offset;
		if (nbytes < dst[i].length)
			memcpy(daddr, dest, nbytes);
		else
			memcpy(daddr, dest, dst[i].length);
		kunmap_atomic(dpage);
		nbytes -= dst[i].length;
		dest += dst[i].length;
	}
	return 0;
}

static int apm_sec_suspend(struct of_device *dev, pm_message_t state)
{
	if (state.event & PM_EVENT_FREEZE) {
		/* To hibernate */
	} else if (state.event & PM_EVENT_SUSPEND) {
		APMSEC_LOG("Security HW Suspend..");
		sec_ctx.poweroff = 1;
		/* To suspend */
	} else if (state.event & PM_EVENT_RESUME) {
		/* To resume */
	} else if (state.event & PM_EVENT_RECOVER) {
		/* To recover from enter suspend failure */
	}
	return 0;
}

static int apm_sec_resume(struct of_device *dev)
{
	int rc = 0;

	if (!resumed_from_deepsleep())
		return rc;

	if (sec_ctx.poweroff) {
		/* Reset the security hardware */
		rc = apm_sec_hwreset();	/* Re-enable clock */
		if (rc != 0) {
			APMSEC_ERR("SEC HW Reset failed after powerdown");
			goto err;
		}
		/* Initialize the security hardware */
		rc = apm_sec_hwinit();
		if (rc != 0)
			goto err;

		/* Start the security hardware */
		rc = apm_sec_hwstart();
		if (rc != 0)
			goto err;
		if (rc == 0)
			sec_ctx.poweroff = 0;
		APMSEC_LOG("Security Resumed..");
	}
err:
	return rc;
}

void apm_lsec_sg2buf(struct scatterlist *src, int nbytes, char *saddr)
{
	int i;
	void *spage;
	void *vaddr;

	for (i = 0; nbytes > 0; i++) {
		spage = kmap_atomic(sg_page(&src[i]));
		vaddr = spage + src[i].offset;
		memcpy(saddr, vaddr, src[i].length);
		kunmap_atomic(spage);
		nbytes -= src[i].length;
		saddr += src[i].length;
	}
}

int apm_lsec_sg_scattered(unsigned int nbytes, struct scatterlist *sg)
{
	return ((int)nbytes - (int)sg[0].length) > 0;
}

struct sec_tkn_ctx *xgene_sec_async_req2tkn(struct crypto_async_request *req)
{
	struct xgene_sec_req_ctx *rctx;

	switch (crypto_tfm_alg_type(req->tfm)) {
	case CRYPTO_ALG_TYPE_AHASH:
		rctx = ahash_request_ctx(ahash_request_cast(req));
		break;
	case CRYPTO_ALG_TYPE_ABLKCIPHER:
		rctx = ablkcipher_request_ctx(ablkcipher_request_cast(req));
		break;
	case CRYPTO_ALG_TYPE_AEAD:
		rctx =
		    aead_request_ctx(container_of
				     (req, struct aead_request, base));
		break;
	default:
		BUG();
		return NULL;
	}
	return rctx->tkn;
}
#endif
