/*
 * APM X-Gene SoC PktDMA Driver
 *
 * Copyright (c) 2014, Applied Micro Circuits Corporation
 * Author: Loc Ho <lho@apm.com>
 *         Rameshwar Prasad Sahu <rsahu@apm.com>
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
 */
#include <linux/delay.h>
#include <linux/acpi.h>
#include <linux/clk.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/efi.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/pm_runtime.h>
#include <linux/bitrev.h>
#include <misc/xgene/qmtm/xgene_qmtm.h>
#include <misc/xgene/xgene-pktdma.h>

#include "dmaengine.h"

/*
 * By default, we don't enable memory to memory operation on all channels.
 * Doing so can degrade performance for XOR and PQ operations on channe 0
 * and channel 1. If you define this, it will enable for all channels.
 */
#undef XGENE_DMA_ENABLE_M2M_ALL_CHANNELS

#define XGENE_CHANNEL_BUDGET		(1024 / 4)
#define XGENE_SLOT_PER_CHANNEL		1024
#define XGENE_MAX_CHANNEL		4
#define XGENE_MAX_XOR_BUFFER		5
#define XGENE_XOR_ALIGNMENT		6	/* 64 Byte Alignment */
#define XGENE_SLOT_NUM_QMMSG		4	/* 4 * 16K = 64K */
#define XGENE_BUFFER_MAX_SIZE		(16 * 1024)
#define XGENE_MAX_LL_DATA_LENGTH	(1024 * 1024)
#define XGENE_MAX_FBY_DATA_LENGTH	(32 * 1024)
#define XGENE_MAX_SRC_COUNT		256 
#define XGENE_MAX_DST_COUNT		256

#define XGENE_XOR_CHANNEL		0
#define XGENE_Q_CHANNEL			1
#define XGENE_FBY_CHANNEL		2

/* Pkt DMA flyby operation code */
#define FBY_2SRC_XOR			0x8
#define FBY_3SRC_XOR			0x9
#define FBY_4SRC_XOR			0xA
#define FBY_5SRC_XOR			0xB

/* Diagnostic CSR register and bit definitions */
#define CFG_MEM_RAM_SHUTDOWN		0x70
#define BLOCK_MEM_RDY			0x74
#define XGENE_PKTDMA_DIAG_CSR_OFFSET	0xD000
#define XGENE_PKTDMA_QMI_CSR_OFFSET	0x9000

/* Core CSR register and bit definitions */
#define DMA_IPBRR			0x00
#define	REV_NO_RD(val)			((val >> 14) & 3)
#define BUS_ID_RD(val)			((val >> 12) & 3)
#define DEVICE_ID_RD(val)		(val & 0xFFF)
#define DMA_INT				0x70
#define DMA_INTMASK			0x74
#define DMA_EN_MASK			BIT(31)
#define DMA_GCR				0x10
#define CH_3_VC_SET(dst, src)		\
	(((dst) & ~(3 << 18)) | (((u32) (src) << 18) & (3 << 18)))
#define CH_2_VC_SET(dst, src)		\
	(((dst) & ~(3 << 16)) | (((u32) (src) << 16) & (3 << 16)))
#define CH_1_VC_SET(dst, src)		\
	(((dst) & ~(3 << 14)) | (((u32) (src) << 14) & (3 << 14)))
#define CH_0_VC_SET(dst, src)		\
	(((dst) & ~(3 << 12)) | (((u32) (src) << 12) & (3 << 12)))
#define EN_MULTI_REQ_3_SET(dst, src)	\
	(((dst) & ~(3 << 10)) | (((u32) (src) << 10) & (3 << 10)))
#define EN_MULTI_REQ_2_SET(dst, src)	\
	(((dst) & ~(3 << 8)) | (((u32) (src) << 8) & (3 << 8)))
#define EN_MULTI_REQ_1_SET(dst, src)	\
	(((dst) & ~(3 << 6)) | (((u32) (src) << 6) & (3 << 6)))
#define EN_MULTI_REQ_0_SET(dst, src)	\
	(((dst) & ~(3 << 4)) | (((u32) (src) << 4) & (3 << 4)))
#define CH_3_EN_MASK 		        BIT(3)
#define CH_2_EN_MASK 		        BIT(2)
#define CH_1_EN_MASK 		        BIT(1)
#define CH_0_EN_MASK 			BIT(0)
#define DMA_RAID6_CONT			0x14
#define DMA_RAID6_CONT_SET(dst, src)	\
	(((dst) & ~(0xFFFFFF << 8)) | (((u32) (src) << 8) & (0xFFFFFF << 8)))

/* Pkt DMA QMI register and bit definitions */
#define DMA_STSSSQMIINT0MASK		0xA0
#define DMA_STSSSQMIINT1MASK		0xA8
#define DMA_STSSSQMIINT2MASK		0xB0
#define DMA_STSSSQMIINT3MASK		0xB8
#define DMA_STSSSQMIINT4MASK		0xC0
#define DMA_STSSSQMIDBGDATA		0xD8
#define DMA_CFGSSQMIFPQASSOC		0xDC
#define DMA_CFGSSQMIWQASSOC		0xE0
#define DMA_CFGSSQMIQMHOLD		0xF8

/* PktDma QM message error codes */
#define ERR_MSG_AXI			0x01
#define ERR_BAD_MSG			0x02
#define ERR_READ_DATA_AXI		0x03
#define ERR_WRITE_DATA_AXI		0x04
#define ERR_FBP_TIMEOUT			0x05
#define ERR_ECC				0x06
#define ERR_DIFF_SIZE			0x08
#define ERR_SCT_GAT_LEN			0x09
#define ERR_CRC_ERR			0x10
#define ERR_CHKSUM			0x12
#define ERR_DIF				0x13

/* PktDma error interrupt code */
#define DIF_SIZE_INT			0x0
#define GS_ERR_INT			0x1
#define FPB_TIMEO_INT			0x2
#define WFIFO_OVF_INT			0x3
#define RFIFO_OVF_INT			0x4
#define WR_TIMEO_INT			0x5
#define RD_TIMEO_INT			0x6
#define WR_ERR_INT			0x7
#define RD_ERR_INT			0x8
#define BAD_MSG_INT			0x9
#define MSG_DST_INT			0xA
#define MSG_SRC_INT			0xB
#define INT_MASK_SHIFT			0x14

/* Descriptor ring lower 32B message format - W0 */
#define RMSG_USERINFO_RD(m)		((u32 *) (m))[0]
#define RMSG_USERINFO_SET(m, v)		((u32 *) (m))[0] = (v)

/* Descriptor ring lower 32B message format - W1 */
#define RMSG_HL_MASK(m)			(((u32 *) (m))[1] & BIT(31))
#define RMSG_LERR_RD(m)			((((u32 *) (m))[1] & 0x70000000) >> 28)
#define RMSG_RTYPE_RD(m)		((((u32 *) (m))[1] & 0x0F000000) >> 24)
#define RMSG_RTYPE_SET(m, v)		\
	((u32 *) (m))[1] = (((u32 *) (m))[1] & ~0x0F000000) | \
	(((v) << 24) & 0x0F000000)
#define RMSG_IN_MASK(m)			(((u32 *) (m))[1] & BIT(23))
#define RMSG_RV_MASK(m)			(((u32 *) (m))[1] & BIT(22))
#define RMSG_HB_MASK(m)			(((u32 *) (m))[1] & BIT(21))
#define RMSG_PB_MASK(m)			(((u32 *) (m))[1] & BIT(20))
#define RMSG_LL_MASK(m)			(((u32 *) (m))[1] & BIT(19))
#define RMSG_LL_SET(m, v)		\
	((u32 *) (m))[1] = (((u32 *) (m))[1] & ~BIT(19)) | \
	(((v) << 19) & BIT(19))
#define RMSG_NV_MASK(m)			(((u32 *) m)[1] & BIT(18))
#define RMSG_NV_SET(m, v)		\
	((u32 *) (m))[1] = (((u32 *) (m))[1] & ~BIT(18)) | \
	(((v) << 18) & BIT(18))
#define RMSG_LEI_RD(m)			((((u32 *) (m))[1] & 0x00030000) >> 16)
#define RMSG_ELERR_RD(m)		((((u32 *) (m))[1] & 0x0000C000) >> 14)
#define RMSG_RV2_RD(m)			((((u32 *) (m))[1] & 0x00003000) >> 12)
#define RMSG_FPQNUM_RD(m)		(((u32 *) (m))[1] & 0x00000FFF)
#define RMSG_FPQNUM_SET(m, v)		\
	((u32 *) (m))[1] = (((u32 *) (m))[1] & ~0x00000FFF) | \
	((v) & 0x00000FFF)

/* Descriptor ring lower 32B message format - W2 */
#define RMSG_DATAADDRL_RD(m)		((u32 *) (m))[2]
#define RMSG_DATAADDRL_SET(m, v)	((u32 *) (m))[2] = (v)

/* Descriptor ring lower 32B message format - W3 */
#define RMSG_C_MASK(m)			(((u32 *) m)[3] & BIT(31))
#define RMSG_C_SET(m, v)		\
	((u32 *) m)[3] = (((u32 *) m)[3] & ~BIT(31)) | (((v) << 31) & BIT(31))
#define RMSG_BUFDATALEN_RD(m)		((((u32 *) m)[3] & 0x7FFF0000) >> 16)
#define RMSG_BUFDATALEN_SET(m, v)	\
	((u32 *) (m))[3] = (((u32 *) (m))[3] & ~0x7FFF0000) | \
	(((v) << 16) & 0x7FFF0000)
#define RMSG_RV6_RD(m)			((((u32 *) (m))[3] & 0x0000FC00) >> 10)
#define RMSG_DATAADDRH_RD(m)		(((u32 *) (m))[3] & 0x3FF)
#define RMSG_DATAADDRH_SET(m, v)	\
	((u32 *) (m))[3] = (((u32 *) (m))[3] & ~0x3FF) | ((v) & 0x3FF)

/* Descriptor ring lower 32B message format - W4 */
#define RMSG_BD_SET(m, v)		\
	((u32 *) (m))[4] = (((u32 *) (m))[4] & ~0x00000001) | \
	((v) & 0x00000001)
#define RMSG_SD_SET(m, v)		\
	((u32 *) (m))[4] = (((u32 *) (m))[4] & ~0x00000002) | \
	((v << 1) & 0x00000002)
#define RMSG_FBY_SET(m, v)		\
	((u32 *) (m))[4] = (((u32 *) (m))[4] & ~0x000000F0) | \
	(((v) << 4) & 0x000000F0)
#define RMSG_MULTI0_SET(m, v)		\
	((u32 *) (m))[4] = (((u32 *) (m))[4] & ~0x0000FF00) | \
	(((v) << 8) & 0x0000FF00)
#define RMSG_MULTI1_SET(m, v)		\
	((u32 *) (m))[4] = (((u32 *) (m))[4] & ~0x00FF0000) | \
	(((v) << 8) & 0x00FF0000)
#define RMSG_MULTI2_SET(m, v)		\
	((u32 *) (m))[4] = (((u32 *) (m))[4] & ~0xFF000000) | \
	(((v) << 8) & 0xFF000000)
#define RMSG_CRCSEEDL_SET(m, v)		\
	((u32 *) (m))[4] = (((u32 *) (m))[4] & ~0xFFFFFF00) | \
	(((v) << 8) & 0xFFFFFF00)

/* Descriptor ring lower 32B message format - W5 */
#define RMSG_DR_SET(m, v)		\
	((u32 *) (m))[5] = (((u32 *) (m))[5] & ~0x20000000) | \
	(((v) << 29) & 0x20000000)
#define RMSG_TOTDATALENGTHLINKLISTLSB_SET(m, v)	\
	((u32 *) (m))[5] = (((u32 *) (m))[5] & ~0x0FFF0000) | \
	(((v) << 16) & 0x0FFF0000)
#define RMSG_H0INFO_MSBH_SET(m, v)	\
	((u32 *) (m))[5] = (((u32 *) (m))[5] & ~0x00000FFF) | \
	((v) & 0x0000FFFF)
#define RMSG_MULTI3_SET(m, v)		\
	((u32 *) (m))[4] = (((u32 *) (m))[4] & ~0x000000FF) | \
	(((v) << 8) & 0x000000FF)
#define RMSG_MULTI4_SET(m, v)		\
	((u32 *) (m))[4] = (((u32 *) (m))[4] & ~0x0000FF00) | \
	(((v) << 8) & 0x0000FF00)
#define RMSG_CRCSEEDH_SET(m, v)		\
	((u32 *) (m))[5] = (((u32 *) (m))[5] & ~0x000000FF) | \
	(((v)) & 0x000000FF)

/* Descriptor ring lower 32B message format - W6 */
#define RMSG_CRCBYTECNT_SET(m, v)		\
	((u32 *) (m))[6] = (((u32 *) (m))[6] & ~0x0000FFFF) | \
	((v) & 0x0000FFFF)
#define RMSG_CRC3_RESULT_RD(m)		((u32 *) (m))[6]
#define RMSG_H0INFO_LSBL_RD(m, v)	((u32 *) (m))[6]
#define RMSG_H0INFO_LSBL_SET(m, v)	((u32 *) (m))[6] = (v)

/* Descriptor ring lower 32B message format - W7 */
#define RMSG_H0FPSEL_RD(m, v)		((((u32 *) (m))[7] & 0xF0000000) >> 27)
#define RMSG_H0ENQ_NUM_SET(m, v)	\
	((u32 *) (m))[7] = (((u32 *) (m))[7] & ~0x0FFF0000) | \
	(((v) << 16) & 0x0FFF0000)
#define RMSG_H0INFO_LSBH_SET(m, v)	\
	((u32 *) (m))[7] = (((u32 *) (m))[7] & ~0x0000FFFF) | \
	((v) & 0x0000FFFF)
#define RMSG_LINK_SIZE_SET(m, v)	\
	((u32 *) (m))[7] = (((u32 *) (m))[7] & ~0x0000FF00) | \
	(((v) << 8) & 0x0000FF00)

/* Descriptor ring empty slot software signature */
#define RMSG_EMPTY_SLOT_INDEX		7
#define RMSG_EMPTY_SLOT_SIGNATURE	0x22222222
#define RMSG_IS_EMPTY_SLOT(m)		\
	((u32 *) (m))[RMSG_EMPTY_SLOT_INDEX] == RMSG_EMPTY_SLOT_SIGNATURE
#define RMSG_SET_EMPTY_SLOT(m)		\
	((u32 *) (m))[RMSG_EMPTY_SLOT_INDEX] = RMSG_EMPTY_SLOT_SIGNATURE

/* Descriptor ring extended entry format */
#define RMSG_NXTDATAADDRL_RD(m)		((u32 *) (m))[0]
#define RMSG_NXTDATAADDRL_SET(m, v)	((u32 *) (m))[0] = (v)
#define RMSG_NXTBUFDATALENGTH_RD(m)	((((u32 *) (m))[1] & 0x7FFF0000) >> 16)
#define RMSG_NXTBUFDATALENGTH_SET(m, v)	\
	((u32 *) (m))[1] = (((u32 *) (m))[1] & ~0x7FFF0000) | \
	(((v) << 16) & 0x7FFF0000)
#define RMSG_NXTFPQNUM_RD(m)		(((u32 *) (m)[1] & 0x0000F000) >> 12)
#define RMSG_NXTFPQNUM_SET(m, v)	\
	((u32 *) (m))[1] = (((u32 *) (m))[1] & ~0x0000F000) | \
	(((v) << 12) & 0x0000F000)
#define RMSG_NXTDATAADDRH_RD(m)		((u32 *) (m)[1] & 0x000003FF)
#define RMSG_NXTDATAADDRH_SET(m, v)	\
	((u32 *) (m))[1] = (((u32 *) m)[1] & ~0x000003FF) | ((v) & 0x000003FF)

/* Descriptor ring upper 32B message for extended format - W2/W3 */
#define RMSG_NXTDATAPTRL_SET(m, v)	((u32 *) (m))[4] = (v)
#define RMSG_TOTDATALENGTHLINKLISTMSB_SET(m, v)	\
	((u32 *) (m))[5] = (((u32 *) (m))[5] & ~0xFF000000) | \
	(((v) << 24) & 0xFF000000)
#define RMSG_NXTLINKLISTLENGTH_SET(m, v)	\
	((u32 *) (m))[5] = (((u32 *) (m))[5] & ~0x00FF0000) | \
	(((v) << 16) & 0x00FF0000)
#define RMSG_NXTFPQNUM2_SET(m, v)		\
	((u32 *) (m))[3] = (((u32 *) (m))[3] & ~0x0000F000) | \
	(((v) << 12) & 0x0000F000)
#define RMSG_NXTDATAPTRH_SET(m, v)		\
	((u32 *) (m))[5] = (((u32 *) (m))[5] & ~0x000003FF) | \
	((v) & 0x000003FF)

struct rmsg_ext8 {
	u32 w0;
	u32 w1;
} __attribute__((packed));

#undef RMSG_DUMP
#if defined(RMSG_DUMP)
#define hex_dump(h, m, l)	\
	{ if (m) \
		print_hex_dump(KERN_INFO, "PktDMA: " h, \
			       DUMP_PREFIX_ADDRESS, 16, 8, m, l, 0); \
	}	
#else
#define hex_dump(h, m, l)
#endif

#define FLAG_SRC_LINKLIST_ACTIVE	0x0001
#define FLAG_DST_LINKLIST_ACTIVE	0x0002
#define FLAG_DSTSRC_LIST_ACTIVE		0x0004
#define FLAG_P_ACTIVE			0x0008
#define FLAG_Q_ACTIVE			0x0010
#define FLAG_SG_ACTIVE			0x0020
#define FLAG_UNMAP_FIRST_DST		0x0040
#define FLAG_UNMAP_SECOND_DST		0x0080

struct xgene_dma_src {
	struct rmsg_ext8 *ring_src_ll;
	dma_addr_t ring_src_ll_pa;
};
	
/* Pkt DMA slot descriptor */
struct xgene_dma_slot {
	int index;
	u32 flags;	/* See FLAG_XXX */
	u32 status;
	int ring_cnt;
	struct dma_async_tx_descriptor async_tx;
	/* For operation requires more than 5 buffers */
	struct xgene_dma_src ring_src;
	struct rmsg_ext8 *ring_dst_ll;
	dma_addr_t ring_dst_ll_pa;

	/* Shadow copy for use in submit function call */
	dma_addr_t src[XGENE_MAX_XOR_BUFFER];
	dma_addr_t dst[2];
	size_t length;
	int src_cnt;

	/* Scatter/Gather shadow copy for use in submit function call */
	struct scatterlist *srcdst_list;
	int dst_cnt;

	/* PQ shadow copy for use in submit function call */
	unsigned char scf[XGENE_MAX_XOR_BUFFER];
};

struct xgene_flyby_slot {
	int index;
	u32 flags;	/* See FLAG_XXX */
	u32 status;
	bool busy;
	int ring_cnt;
	struct scatterlist *src_sg;
	int src_nents;
	/* For operation requires more than 5 buffers */
	struct xgene_dma_src ring_src;

	/* Flyby context */
	void *flyby_ctx;
	void (*flyby_cb)(void *flyby_ctx, u32 flyby_result);
};

/* Pkt DMA channel descriptor */
struct xgene_dma_chan {
	struct dma_chan dma_chan;
	struct xgene_dma *pdma;
	int index;
	spinlock_t lock;
	struct tasklet_struct rx_tasklet;
	struct xgene_dma_slot *slots;
	struct xgene_dma_slot *last_used;
	struct xgene_flyby_slot *flyby_slots;
	struct xgene_flyby_slot *last_used_flyby;
	struct xgene_qmtm_qinfo tx_qdesc;
	struct xgene_qmtm_qinfo rx_qdesc;
	struct xgene_qmtm_qinfo fp_qdesc;
	char name[16];
};

/* Pkt DMA device */
struct xgene_dma {
	struct device *dev;
	struct clk *dma_clk;
	int irq;	/* Error IRQ */
	spinlock_t lock;
	void *csr;
	void *csr_diag;
	void *csr_ring;
	struct xgene_qmtm_sdev *sdev;
	struct dma_device dma_dev[XGENE_MAX_CHANNEL];
	struct xgene_dma_chan channels[XGENE_MAX_CHANNEL];
};

static struct xgene_dma *pktdma;

#define to_pktdma_chan(chan)		\
	container_of(chan, struct xgene_dma_chan, dma_chan)
#define tx_to_pktdma_slot(tx) 		\
	container_of(tx, struct xgene_dma_slot, async_tx)

static const char * const xgene_dma_qmsg_err[] = {
	[ERR_MSG_AXI]        = "AXI Error when reading Src/Dst Link List",
	[ERR_BAD_MSG]        = "ERR or El_ERR fields are not set to zero in the incoming MSG",
	[ERR_READ_DATA_AXI]  = "AXI Error when reading data",
	[ERR_WRITE_DATA_AXI] = "AXI Error when writing data",
	[ERR_FBP_TIMEOUT]    = "Timeout on free pool buffer fetch",
	[ERR_ECC]            = "ECC Double Bit Error",
	[ERR_DIFF_SIZE]      = "Free Pool Buffer too small to hold all the DIF result",
	[ERR_SCT_GAT_LEN]    = "Gather and Scatter dont have the same Data Length",
	[ERR_CRC_ERR]        = "CRC Error",
	[ERR_CHKSUM]         = "Checksum Error",
	[ERR_DIF]            = "DIF Error",
};

static const char * const xgene_dma_err[] = {
	[DIF_SIZE_INT]  = "DIF size Error",
	[GS_ERR_INT]    = "Gather scatter not same size Error",
	[FPB_TIMEO_INT] = "Free pool time out Error",
	[WFIFO_OVF_INT] = "Write FIFO over flow Error",
	[RFIFO_OVF_INT] = "Read FIFO over flow Error",
	[WR_TIMEO_INT]  = "Write time out Error",
	[RD_TIMEO_INT]  = "Read time out Error",
	[WR_ERR_INT]    = "HBF bus write Error",
	[RD_ERR_INT]    = "HBF bus read Error",
	[BAD_MSG_INT]   = "QM message HE0 not set Error",
	[MSG_DST_INT]   = "HFB reading destination link addr Error",
	[MSG_SRC_INT]   = "HFB reading source link addr Error",
};

#ifdef CONFIG_CPU_BIG_ENDIAN
#define xgene_dma_cpu_to_le64(msg, count)	\
	if (msg) {	\
		int i;	\
		u32 tmp;	\
		for (i = 0; i < count; i += 2) {	\
			tmp = ((u32 *)msg)[i];	\
			((u32 *)msg)[i] = ((u32 *)msg)[i + 1];	\
			((u32 *)msg)[i + 1] = tmp;	\
			((u64 *)msg)[i/2] = cpu_to_le64(((u64 *)msg)[i/2]);	\
		}	\
	}
#define xgene_dma_le64_to_cpu(msg, count)	\
	if (msg) {	\
		int i;	\
		u32 tmp;	\
		for (i = 0; i < count; i += 2) {	\
			tmp = ((u32 *)msg)[i];	\
			((u32 *)msg)[i] = ((u32 *)msg)[i + 1];	\
			((u32 *)msg)[i + 1] = tmp;	\
			((u64 *)msg)[i/2] = le64_to_cpu(((u64 *)msg)[i/2]);	\
		}	\
	}
#else 
#define xgene_dma_cpu_to_le64(msg, count)
#define xgene_dma_le64_to_cpu(msg, count)
#endif

static inline u32 xgene_flyby_encode_uinfo(struct xgene_flyby_slot *slot)
{
	return slot->index;
}

static inline u32 xgene_dma_encode_uinfo(struct xgene_dma_slot *slot)
{
	return slot->index;
}

static inline void *xgene_dma_decode_uinfo(struct xgene_dma_chan *chan, 
						u32 uinfo)
{
	if (uinfo < XGENE_SLOT_PER_CHANNEL) 
		return &chan->slots[uinfo];
	else
		return &chan->flyby_slots[uinfo - XGENE_SLOT_PER_CHANNEL];
}

static inline u8 xgene_dma_encode_xor_flyby(u32 src_cnt)
{
	static u8 flyby_type[] = {
		FBY_2SRC_XOR, /* Dummy */
		FBY_2SRC_XOR, /* Dummy */
		FBY_2SRC_XOR,
		FBY_3SRC_XOR,
		FBY_4SRC_XOR,
		FBY_5SRC_XOR
	};
	return flyby_type[src_cnt];
}

static void xgene_dma_process_flyby_msg(struct xgene_dma_chan *chan,
	struct xgene_flyby_slot *slot, u32 *msg)
{
	if (RMSG_LERR_RD(msg) || RMSG_ELERR_RD(msg)) {
		slot->status = (RMSG_ELERR_RD(msg) < 4) | RMSG_LERR_RD(msg);
		dev_err(chan->pdma->dev, "%s channel %d\n",
			xgene_dma_qmsg_err[slot->status], chan->index);
	}

	if (--slot->ring_cnt == 0) {
		if (slot->flyby_cb) 
			slot->flyby_cb(slot->flyby_ctx, 
					RMSG_CRC3_RESULT_RD(msg));	

		if (slot->flags & FLAG_SRC_LINKLIST_ACTIVE) { 
			dma_free_coherent(chan->pdma->dev, 
					sizeof(struct rmsg_ext8) * XGENE_MAX_SRC_COUNT, 
					slot->ring_src.ring_src_ll, 
					slot->ring_src.ring_src_ll_pa);
			slot->ring_src.ring_src_ll = NULL;
		}
		dma_unmap_sg(chan->pdma->dev, 
				slot->src_sg, slot->src_nents, DMA_TO_DEVICE); 
		slot->busy = false;
	}
}

static void xgene_dma_slot_cleanup(struct xgene_dma_chan *chan, 
					struct xgene_dma_slot *slot)
{
	enum dma_ctrl_flags flags = slot->async_tx.flags;
	int i;

	if (slot->flags & FLAG_SRC_LINKLIST_ACTIVE) {
		dma_free_coherent(chan->pdma->dev, 
				sizeof(struct rmsg_ext8) * XGENE_MAX_SRC_COUNT, 
				slot->ring_src.ring_src_ll, 
				slot->ring_src.ring_src_ll_pa);
		slot->ring_src.ring_src_ll = NULL;
	}
		
	if (slot->flags & FLAG_DST_LINKLIST_ACTIVE) {
		dma_free_coherent(chan->pdma->dev, 
				sizeof(struct rmsg_ext8) * XGENE_MAX_DST_COUNT, 
				slot->ring_dst_ll, slot->ring_dst_ll_pa);
		slot->ring_dst_ll = NULL;
	}

	if (slot->flags & FLAG_DSTSRC_LIST_ACTIVE)
		devm_kfree(chan->pdma->dev, slot->srcdst_list);

	if (slot->flags & FLAG_SG_ACTIVE)
		return;

	if (!(flags & DMA_COMPL_SKIP_DEST_UNMAP)) {
		enum dma_data_direction dir;
	
		if (slot->src_cnt > 1) /* is xor? */
			dir = DMA_BIDIRECTIONAL;
		else
			dir = DMA_FROM_DEVICE;

		if (flags & DMA_COMPL_DEST_UNMAP_SINGLE) {
			if (slot->flags & FLAG_UNMAP_FIRST_DST) 
				dma_unmap_single(chan->pdma->dev, 
						slot->dst[0], slot->length, dir);
			if (slot->flags & FLAG_UNMAP_SECOND_DST) 
				dma_unmap_single(chan->pdma->dev, 
						slot->dst[1], slot->length, dir);
		} else {
			if (slot->flags & FLAG_UNMAP_FIRST_DST) 
				dma_unmap_page(chan->pdma->dev, 
						slot->dst[0], slot->length, dir);
			if (slot->flags & FLAG_UNMAP_FIRST_DST) 
				dma_unmap_page(chan->pdma->dev, 
						slot->dst[1], slot->length, dir);
		}
	}
		
	if (!(flags & DMA_COMPL_SKIP_SRC_UNMAP)) {
		for (i = 0; i < slot->src_cnt; i++) {
			if (slot->dst[0] == slot->src[0])
				continue;

			if (flags & DMA_COMPL_SRC_UNMAP_SINGLE) 
				dma_unmap_single(chan->pdma->dev, slot->src[i], 
						slot->length, DMA_TO_DEVICE);
			else
				dma_unmap_page(chan->pdma->dev, slot->src[i], 
						slot->length, DMA_TO_DEVICE);
		}
	}
}

static void xgene_dma_process_msg(struct xgene_dma_chan *chan,
	struct xgene_dma_slot *slot, u32 *msg)
{
	if (RMSG_LERR_RD(msg) || RMSG_ELERR_RD(msg)) {
		slot->status = (RMSG_ELERR_RD(msg) < 4) | RMSG_LERR_RD(msg);
		dev_err(chan->pdma->dev, "%s channel %d\n",
			xgene_dma_qmsg_err[slot->status], chan->index);
	}

	if (--slot->ring_cnt == 0) {
		dma_cookie_complete(&slot->async_tx);
		if (slot->async_tx.callback)
			slot->async_tx.callback(slot->async_tx.callback_param);
		dma_run_dependencies(&slot->async_tx);
		xgene_dma_slot_cleanup(chan, slot);
	}
}

static void xgene_dma_bh_tasklet_cb(unsigned long data)
{
	struct xgene_dma_chan *chan = (void *) data;
	struct xgene_qmtm_qinfo *qdesc = &chan->rx_qdesc;
	struct xgene_qmtm_msg32 *qbase = qdesc->msg32;
	struct xgene_dma_slot *slot;
	struct xgene_flyby_slot *flyby_slot;
	int budget = XGENE_CHANNEL_BUDGET;
	u32 uinfo;
	u32 fp_command;
	u32 command = 0;
	void *msg1;
	void *msg2;

	spin_lock_bh(&chan->lock);

	while (budget--) {
		msg1 = &qbase[qdesc->qhead];
		/* Check for completed operations */
		if (unlikely(RMSG_IS_EMPTY_SLOT(msg1)))
			break;

		xgene_dma_le64_to_cpu(msg1, 8);

		if (++qdesc->qhead == qdesc->count)
			qdesc->qhead = 0;

		fp_command = 0;

		if (RMSG_NV_MASK(msg1)) {
			/* 64B message */
			msg2 = &qbase[qdesc->qhead];
			if (++qdesc->qhead == qdesc->count)
				qdesc->qhead = 0;
			command -= 2;
			fp_command += 2;
		} else {
			msg2 = NULL;
			--command;
			++fp_command;
		}

		uinfo = RMSG_USERINFO_RD(msg1);
		if (uinfo < XGENE_SLOT_PER_CHANNEL) {
			slot = xgene_dma_decode_uinfo(chan, uinfo);
			if (likely(slot)) {
				dev_dbg(chan->pdma->dev,
					"channel %d completed op slot %d QM Msg:\n",
					chan->index, slot->index);

				xgene_dma_process_msg(chan, slot, msg1);
			} else {
				dev_err(chan->pdma->dev,
					"channel %d invalid uinfo\n", chan->index);
			}
		} else {
			flyby_slot = xgene_dma_decode_uinfo(chan, uinfo);
			if (likely(flyby_slot)) {
				dev_dbg(chan->pdma->dev,
					"channel %d completed op slot %d QM Msg:\n",
					chan->index, flyby_slot->index);

				xgene_dma_process_flyby_msg(chan, flyby_slot, msg1);

				writel(fp_command, chan->fp_qdesc.command);
			} else {
				dev_err(chan->pdma->dev,
					"channel %d invalid uinfo\n", chan->index);
			}
		}

		RMSG_SET_EMPTY_SLOT(msg1);
		if (msg2)
			RMSG_SET_EMPTY_SLOT(msg2);

		hex_dump("TX QMSG1: ", msg1, 32);
		hex_dump("TX QMSG2: ", msg2, 32);
	}

	dev_dbg(chan->pdma->dev, "processed %d 32B message\n", command);
	writel(command, qdesc->command);

	/* Re-enable IRQ */
	enable_irq(qdesc->irq);

	spin_unlock_bh(&chan->lock);
}

static irqreturn_t xgene_dma_qm_msg_isr(int value, void *id)
{
	struct xgene_dma_chan *chan = (struct xgene_dma_chan *) id;

	BUG_ON(!chan);
	disable_irq_nosync(chan->rx_qdesc.irq);
	tasklet_schedule(&chan->rx_tasklet);

	return IRQ_HANDLED;
}

static int xgene_dma_init_fp(struct xgene_dma_chan *chan,
			struct xgene_qmtm_qinfo *fp_qdesc,
			u8 qmtm_ip, u32 buf_cnt)
{
	dma_addr_t buf_addr;
	void *msg16;
	void *buf;
	int i;

	for (i = 0; i < buf_cnt; i++) {
		msg16 = &fp_qdesc->msg16[i];
		memset(msg16, 0, sizeof(struct xgene_qmtm_msg16));
		buf = devm_kzalloc(chan->pdma->dev, XGENE_BUFFER_MAX_SIZE,
				   GFP_KERNEL);
		if (!buf)
			return -ENOMEM;

		buf_addr = dma_map_single(chan->pdma->dev, buf,
					  XGENE_BUFFER_MAX_SIZE,
					  DMA_TO_DEVICE);
		RMSG_DATAADDRL_SET(msg16, (u32) buf_addr);
		RMSG_DATAADDRH_SET(msg16, (u32) (buf_addr >> 32));
		RMSG_USERINFO_SET(msg16, i);
		RMSG_C_SET(msg16, 1);
		RMSG_BUFDATALEN_SET(msg16, 0);	/* 16 KB */
		RMSG_FPQNUM_SET(msg16,
				QMTM_QUEUE_ID(qmtm_ip, fp_qdesc->queue_id));
		RMSG_RTYPE_SET(msg16, chan->pdma->sdev->slave_id);

		xgene_dma_cpu_to_le64(msg16, 4);
	}

	writel(buf_cnt, fp_qdesc->command);

	return 0;
}

static int xgene_dma_init_ring(struct xgene_dma *pdma)
{
	struct xgene_dma_chan *chan;
	struct xgene_qmtm_qinfo *rinfo;
	u8 slave_i = pdma->sdev->slave_i;
	u8 qmtm_ip = pdma->sdev->qmtm_ip;
	u8 slave = pdma->sdev->slave;
	int ret;
	int i;

	for (i = 0; i < XGENE_MAX_CHANNEL; i++) {
		chan = &pdma->channels[i];

		/* Allocate Tx ring */
 		rinfo = &chan->tx_qdesc;
		memset(rinfo, 0, sizeof(*rinfo));
		rinfo->slave = slave;
		rinfo->qaccess = QACCESS_ALT;
		rinfo->qtype = QTYPE_PQ;
		rinfo->qsize = QSIZE_64KB;
		rinfo->flags = XGENE_SLAVE_DEFAULT_FLAGS;
		if ((ret = xgene_qmtm_set_qinfo(rinfo)) != QMTM_OK) {
			dev_err(pdma->dev, "Unable to allocate Tx queue\n");
			return ret;
		}
		dev_dbg(pdma->dev, "PktDMA Tx queue %d PBN %d desc 0x%p\n",
			rinfo->queue_id, rinfo->pbn, rinfo->msg32);

		/* Allocate completion ring */
		rinfo = &chan->rx_qdesc;
		memset(rinfo, 0, sizeof(*rinfo));
		rinfo->slave = slave_i;
		rinfo->qaccess = QACCESS_ALT;
		rinfo->qtype = QTYPE_PQ;
		rinfo->qsize = QSIZE_64KB;
		rinfo->flags = XGENE_SLAVE_DEFAULT_FLAGS;
		if ((ret = xgene_qmtm_set_qinfo(rinfo)) != QMTM_OK) {
			dev_err(pdma->dev, "Unable to allocate Rx queue\n");
			return ret;
		}
#if 0
		/*
		 * Enable interrupt coalescence for completion ring. This will
		 * reduce the interrupt overhead and better performance.
		 */
		xgene_qmtm_intr_coalesce(qmtm_ip, rinfo->pbn, 0x4);
#endif
		dev_dbg(pdma->dev,
			"PktDMA Rx queue %d PBN %d desc 0x%p IRQ %d\n",
			rinfo->queue_id, rinfo->pbn, rinfo->msg32, rinfo->irq);
	}

	/* Allocate free pool ring */
	chan = &pdma->channels[0];
	rinfo = &chan->fp_qdesc;
	memset(rinfo, 0, sizeof(*rinfo));
	rinfo->slave = slave;
	rinfo->qaccess = QACCESS_ALT;
	rinfo->qtype = QTYPE_FP;
	rinfo->qsize = QSIZE_64KB;
	rinfo->flags = XGENE_SLAVE_DEFAULT_FLAGS;
	if ((ret = xgene_qmtm_set_qinfo(rinfo)) != QMTM_OK) {
		dev_err(pdma->dev, "Unable to allocate Free Pool queue\n");
		return ret;
	}

	/* Initialize the free pool buffer */
	ret = xgene_dma_init_fp(chan, rinfo, qmtm_ip, rinfo->count);
	if (ret) {
		dev_err(pdma->dev,"Free Pool Initialization Failed\n");
		return ret;
	}

	dev_dbg(pdma->dev,
		"PktDMA Free Pool queue %d PBN %d desc 0x%p slot count %d\n",
		rinfo->queue_id, rinfo->pbn, rinfo->msg32, rinfo->count);

	/* Free pool ring is shared with all channels */
	for (i = 1; i < XGENE_MAX_CHANNEL; i++) {
		chan = &pdma->channels[i];
		chan->fp_qdesc = *rinfo;
	}

	return 0;
}

static int xgene_dma_hw_init(struct xgene_dma *pdma)
{
	u32 val;
	int ret;

	/* Configue channel rings */
	ret = xgene_dma_init_ring(pdma);
	if (ret)
		return ret;

	/* Configure RAID6 setting */
	val = readl(pdma->csr + DMA_RAID6_CONT);
	val = DMA_RAID6_CONT_SET(val, 0x200000);
	writel(val, pdma->csr + DMA_RAID6_CONT);

	/* Un-mask work queue overflow/underun, free pool overflow/underrun */
	readl(pdma->csr_ring + DMA_STSSSQMIDBGDATA);
	writel(0x0, pdma->csr_ring + DMA_STSSSQMIINT0MASK);
	writel(0x0, pdma->csr_ring + DMA_STSSSQMIINT1MASK);
	writel(0x0, pdma->csr_ring + DMA_STSSSQMIINT2MASK);
	writel(0x0, pdma->csr_ring + DMA_STSSSQMIINT3MASK);

	/* Un-mask AXI write/read errror */
	writel(0x0, pdma->csr_ring + DMA_STSSSQMIINT4MASK);

	/* Associate FP and WQ to corresponding ring HW */
	writel(0xFFFFFFFF, pdma->csr_ring + DMA_CFGSSQMIFPQASSOC);
	writel(0xFFFFFFFF, pdma->csr_ring + DMA_CFGSSQMIWQASSOC);
	writel(0x80000007, pdma->csr_ring + DMA_CFGSSQMIQMHOLD);

	/* Enable engine */
	val = readl(pdma->csr + DMA_GCR);
	val |= DMA_EN_MASK;
	val = CH_3_VC_SET(val, 0x2);
	val = CH_2_VC_SET(val, 0x2);
	val = CH_1_VC_SET(val, 0x2);
	val = CH_0_VC_SET(val, 0x2);
	val = EN_MULTI_REQ_3_SET(val, 0x3);
	val = EN_MULTI_REQ_2_SET(val, 0x3);
	val = EN_MULTI_REQ_1_SET(val, 0x3);
	val = EN_MULTI_REQ_0_SET(val, 0x3);
	val |= CH_3_EN_MASK;
	val |= CH_2_EN_MASK;
	val |= CH_1_EN_MASK;
	val |= CH_0_EN_MASK;
	writel(val, pdma->csr + DMA_GCR);

	return 0;
}

static int xgene_dma_init_mem(struct xgene_dma *pdma)
{
	int timeout = 1000;
	u32 val;

	dev_dbg(pdma->dev, "PktDMA clear memory shutdown\n");
	writel(0x0, pdma->csr_diag + CFG_MEM_RAM_SHUTDOWN);
	readl(pdma->csr_diag + CFG_MEM_RAM_SHUTDOWN);

	while (timeout > 0) {
		val = readl(pdma->csr_diag + BLOCK_MEM_RDY);
		if (val == 0xFFFFFFFF)
			break;
		udelay(1);
	}

	if (timeout <= 0) {
		dev_err(pdma->dev,
			"PktDMA failed to remove RAM out of reset\n");
		return -ENODEV;
	}
	return 0;
}

static void xgene_dma_enable_error_irq(struct xgene_dma *pdma)
{
	/* Enable PktDMA interrupt for error */
	writel(0x0, pdma->csr + DMA_INTMASK);
}

static irqreturn_t xgene_dma_err_isr(int value, void *id)
{
	struct xgene_dma *pdma = (struct xgene_dma *) id;
	unsigned long interrupt_mask;
	u32 val;
	int i;

	val = readl(pdma->csr + DMA_INT);
	writel(val, pdma->csr + DMA_INT);

	interrupt_mask = val >> INT_MASK_SHIFT;
	for_each_set_bit(i, &interrupt_mask, ARRAY_SIZE(xgene_dma_err))
		dev_err(pdma->dev, "%s Interrupt 0x%08X\n",
			xgene_dma_err[i], val);

	return IRQ_HANDLED;
}

static int xgene_dma_alloc_resources(struct dma_chan *channel)
{
	struct xgene_dma_chan *chan = to_pktdma_chan(channel);
	int i;

	spin_lock_bh(&chan->lock);

	if (!chan->slots) {
		chan->slots = devm_kzalloc(chan->pdma->dev,
					   sizeof(struct xgene_dma_slot) *
					   XGENE_SLOT_PER_CHANNEL, GFP_ATOMIC);
		if (!chan->slots) {
			spin_unlock_bh(&chan->lock);
			return -ENOMEM;
		}
	}

	for (i = 0; i < XGENE_SLOT_PER_CHANNEL; i++) {
		dma_async_tx_descriptor_init(
				&chan->slots[i].async_tx, channel);
		chan->slots[i].async_tx.cookie = 0;
		chan->slots[i].index = i;
	}

	chan->last_used = &chan->slots[0];
	dma_cookie_init(&chan->dma_chan);

	dev_dbg(chan->pdma->dev, "allocated %d descriptors",
		XGENE_SLOT_PER_CHANNEL);

	spin_unlock_bh(&chan->lock);

	return XGENE_SLOT_PER_CHANNEL;
}

static void xgene_dma_free_resources(struct dma_chan *channel)
{
	struct xgene_dma_chan *chan = to_pktdma_chan(channel);

	spin_lock_bh(&chan->lock);

	devm_kfree(chan->pdma->dev, chan->slots);
	chan->slots = NULL;
	chan->last_used = NULL;

	dev_dbg(chan->pdma->dev, "free %d descriptors\n",
		XGENE_SLOT_PER_CHANNEL);

	spin_unlock_bh(&chan->lock);
}

static enum dma_status xgene_dma_tx_status(struct dma_chan *channel,
	dma_cookie_t cookie, struct dma_tx_state *txstate)
{
	return dma_cookie_status(channel, cookie, txstate);
}

static void xgene_dma_issue_pending(struct dma_chan *channel)
{
	/* Do nothing */
}

static struct xgene_dma_slot *xgene_dma_channel_get_slot(
	struct xgene_dma_chan *chan)
{
	struct xgene_dma_slot *slot;

	slot = (chan->last_used == 
			&chan->slots[XGENE_SLOT_PER_CHANNEL - 1]) ?
			&chan->slots[0] : (chan->last_used + 1);

	if (slot->async_tx.cookie)
		return NULL;

	chan->last_used = slot;
	slot->async_tx.cookie = -EBUSY;

	return slot;
}

static int xgene_dma_rmsg_src_set(void *ext8, size_t *nbytes,
				dma_addr_t *paddr)
{
	RMSG_NXTDATAADDRH_SET(ext8, (u32) (*paddr >> 32));
	RMSG_NXTDATAADDRL_SET(ext8, (u32) *paddr);
	if (*nbytes < XGENE_BUFFER_MAX_SIZE) {
		int len;

		RMSG_NXTBUFDATALENGTH_SET(ext8,
				xgene_qmtm_encode_datalen(*nbytes));
		len = *nbytes;
		*nbytes = 0;
		return len;
	}
	RMSG_NXTBUFDATALENGTH_SET(ext8, 0);
 	if (*nbytes == XGENE_BUFFER_MAX_SIZE) {
		*nbytes = 0;
		return XGENE_BUFFER_MAX_SIZE;
	}

	*nbytes -= XGENE_BUFFER_MAX_SIZE;
	*paddr += XGENE_BUFFER_MAX_SIZE;
	return XGENE_BUFFER_MAX_SIZE;
}

static void xgene_dma_rmsg_src_null(void *ext8)
{
	RMSG_NXTBUFDATALENGTH_SET(ext8, 0x7800);
}

static void *xgene_dma_lookup_ext8(void *msg, int idx)
{
	switch (idx) {
	case 0:
	default:
		return (struct rmsg_ext8 *) msg + 1;
	case 1:
		return (struct rmsg_ext8 *) msg;
	case 2:
		return (struct rmsg_ext8 *) msg + 3;
	case 3:
		return (struct rmsg_ext8 *) msg + 2;
	}
}

static dma_cookie_t xgene_dma_tx_memcpy_submit(
	struct dma_async_tx_descriptor *tx)
{
	struct xgene_dma_chan *chan = to_pktdma_chan(tx->chan);
	struct xgene_qmtm_qinfo *qdesc = &chan->tx_qdesc;
	struct xgene_dma_slot *slot = tx_to_pktdma_slot(tx);
	struct rmsg_ext8 *ext8;
	dma_cookie_t cookie;
	void *msg1;
	void *msg2;
	int command;
	int i;
	dma_addr_t src = slot->src[0];
	size_t length = slot->length;
	
	spin_lock_bh(&chan->lock);

	/* Now setup QM message first src pointer message */
	msg1 = &qdesc->msg32[qdesc->qhead];
	if (++qdesc->qhead == qdesc->count)
		qdesc->qhead = 0;
	memset(msg1, 0, 32);

	xgene_dma_rmsg_src_set(msg1 + 8, &length, &src);
	RMSG_C_SET(msg1, 1); /* Coherent IO */

	RMSG_USERINFO_SET(msg1, xgene_dma_encode_uinfo(slot));
	RMSG_RTYPE_SET(msg1, chan->pdma->sdev->slave_id);

	RMSG_DR_SET(msg1, 1);
	RMSG_H0INFO_LSBL_SET(msg1, (u32) slot->dst[0]);
	RMSG_H0INFO_LSBH_SET(msg1, (u32) (slot->dst[0] >> 32));
	RMSG_H0ENQ_NUM_SET(msg1, QMTM_QUEUE_ID(chan->pdma->sdev->qmtm_ip,
					       chan->rx_qdesc.queue_id));
	if (length <= 0) {
		command = 1;
		msg2 = NULL;
		goto skip_additional_src;
	}

	/* Setup 2nd to nth source address */
	msg2 = &qdesc->msg32[qdesc->qhead];
	if (++qdesc->qhead == qdesc->count)
		qdesc->qhead = 0;
	memset(msg2, 0, 32);

	RMSG_NV_SET(msg1, 1);
	command = 2;

	/* Load 2nd to 4th source address */
	for (i = 0; i < 3 && length; i++) {
		xgene_dma_rmsg_src_set(xgene_dma_lookup_ext8(msg2, i),
							  &length, &src);
	}
	if (length <= 0) {
		for ( ; i < 4; i++) {
			xgene_dma_rmsg_src_null(
				xgene_dma_lookup_ext8(msg2, i));
		}
	} else if (length <= XGENE_BUFFER_MAX_SIZE) {
		/* Load 5th buffer pointers */
		xgene_dma_rmsg_src_set(xgene_dma_lookup_ext8(msg2, 3),
					&length, &src);
	} else {
		int ll_len;
		int ll_count;

		/* Load more than 5 buffer pointers */
		dev_dbg(chan->pdma->dev, "channel %d slot %d m2m link list\n",
			chan->index, slot->index);

		ll_len = 0;
		ll_count = 0;

		slot->ring_src.ring_src_ll = 
				dma_zalloc_coherent(chan->pdma->dev, 
					sizeof(struct rmsg_ext8) * XGENE_MAX_SRC_COUNT, 
					&slot->ring_src.ring_src_ll_pa, GFP_ATOMIC);
		if (!slot->ring_src.ring_src_ll) {
			dev_err(chan->pdma->dev, 
				"Couldn't allocate memory for src link list\n");
			goto err_src_ll;
		}

		slot->flags |= FLAG_SRC_LINKLIST_ACTIVE;
		ext8 = slot->ring_src.ring_src_ll;
		while (length && ll_count < XGENE_MAX_SRC_COUNT) {
			ll_len += xgene_dma_rmsg_src_set(
					&ext8[((ll_count % 2) ? (ll_count - 1) :
					(ll_count + 1))], &length, &src);

			xgene_dma_cpu_to_le64(
					&ext8[((ll_count % 2) ? (ll_count - 1) :
					(ll_count + 1))], 2);
			ll_count++;
		}
		
		RMSG_LL_SET(msg1, 1);
		RMSG_NXTDATAPTRL_SET(msg2, (u32) slot->ring_src.ring_src_ll_pa);
		RMSG_NXTDATAPTRH_SET(msg2,
				     (u32) (slot->ring_src.ring_src_ll_pa >> 32));
		RMSG_NXTLINKLISTLENGTH_SET(msg2, ll_count);
		RMSG_TOTDATALENGTHLINKLISTLSB_SET(msg1, ll_len);
		RMSG_TOTDATALENGTHLINKLISTMSB_SET(msg2, (ll_len >> 12));
	}
	
skip_additional_src:
	xgene_dma_cpu_to_le64(msg1, 8);
	xgene_dma_cpu_to_le64(msg2, 8);

	hex_dump("TX QMSG1: ", msg1, 32);
	hex_dump("TX QMSG2: ", msg2, 32);

	dev_dbg(chan->pdma->dev,
		"Issuing op 0x%p msg cnt %d cmd %d TxQID %d RxQID %d\n",
		slot, slot->ring_cnt, command, chan->tx_qdesc.queue_id,
		chan->rx_qdesc.queue_id);

	cookie = dma_cookie_assign(tx);

	/* Notify HW */
	writel(command, chan->tx_qdesc.command);

	spin_unlock_bh(&chan->lock);

	return cookie;

err_src_ll:
	/* Roll back the ring descriptor by 2 */
	qdesc->qhead = qdesc->qhead == 0 ? qdesc->count - 1 : qdesc->qhead - 1;
	qdesc->qhead = qdesc->qhead == 0 ? qdesc->count - 1 : qdesc->qhead - 1;

	spin_unlock_bh(&chan->lock);
	return -ENOMEM;
}

static struct dma_async_tx_descriptor *xgene_dma_prep_dma_memcpy(
	struct dma_chan *channel, dma_addr_t dst, dma_addr_t src,
	size_t length, unsigned long flags)
{
	struct xgene_dma_chan *chan = to_pktdma_chan(channel);
	struct xgene_dma_slot *slot;

	spin_lock_bh(&chan->lock);
	slot = xgene_dma_channel_get_slot(chan);
	if (!slot) {
		spin_unlock_bh(&chan->lock);
		dev_dbg(chan->pdma->dev, "channel %d no slot\n",
			chan->index);
		return NULL;
	}

	dev_dbg(chan->pdma->dev,
		"m2m channel %d slot %d len %zu src 0x%llX dst 0x%llX\n",
		chan->index, slot->index, length, src, dst);

	/* Set up slot variable */
	slot->flags = FLAG_UNMAP_FIRST_DST;
	slot->ring_cnt = 1;	/* m2m op only need 1 QM message */
	slot->async_tx.flags = flags;
	slot->async_tx.tx_submit = xgene_dma_tx_memcpy_submit;

	/* Make shadow copy for use during submit */
	slot->src[0] = src;
	slot->dst[0] = dst;
	slot->length = length;
	slot->src_cnt = 1;

	spin_unlock_bh(&chan->lock);

	return &slot->async_tx;
}

static u32 xgene_dma_load_single_sg(void *ext8, struct scatterlist *src_sg, 
				dma_addr_t *paddr, u32 *nbytes, u32 offset)
{
	u32 len;

	if (*paddr == 0) 
		*paddr = sg_dma_address(src_sg);

	RMSG_NXTDATAADDRH_SET(ext8, (u32) (*paddr >> 32));
	RMSG_NXTDATAADDRL_SET(ext8, (u32) *paddr);

	len = sg_dma_len(src_sg) - offset;

	if (len <= XGENE_BUFFER_MAX_SIZE) {
		RMSG_NXTBUFDATALENGTH_SET(ext8, 
			(len < XGENE_BUFFER_MAX_SIZE) ? len : 0);
		*nbytes -= len;
		*paddr = 0;
		return len;
	} else {
		RMSG_NXTBUFDATALENGTH_SET(ext8, 0);
		*nbytes -= XGENE_BUFFER_MAX_SIZE;
		*paddr += XGENE_BUFFER_MAX_SIZE;
		return XGENE_BUFFER_MAX_SIZE;
	}
}

static int xgene_dma_load_src_sg(struct xgene_dma_chan *chan, 
				struct xgene_dma_src *slot, 
				void *msg1, void *msg2, 
				struct scatterlist *src_sg, 
				u32 nbytes, u32 *flags)
{
	struct device *dev = chan->pdma->dev;
	struct rmsg_ext8 *ext8;
	dma_addr_t paddr = 0;
	u32 offset = 0;
	u32 len;
	u32 ll_len;
	int ll_count;
	int i;
	
	offset += xgene_dma_load_single_sg(msg1 + 8, 
			src_sg, &paddr, &nbytes, offset);
	
	if (nbytes == 0)
		return 1; /* 32B msg */

	RMSG_NV_SET(msg1, 1);	/* More than one src */

	/* Load 2nd, 3rd and 4th src buffer */
	for (i = 0; nbytes > 0 && i < 3; i++) {
		if (!paddr) {
			src_sg = sg_next(src_sg);
			offset = 0;
		}
		offset += xgene_dma_load_single_sg(
				xgene_dma_lookup_ext8(msg2, i), 
				src_sg, &paddr, &nbytes, offset);
	}

	if (nbytes == 0) {
		xgene_dma_rmsg_src_null(xgene_dma_lookup_ext8(msg2, i));
		return 2;	/* 2 - 4 buffers - 64B msg */
	}

	/* Load 5th src buffer */
	if (!paddr) {
		src_sg = sg_next(src_sg);
		offset = 0;
	}

	if (nbytes <= XGENE_BUFFER_MAX_SIZE && sg_is_last(src_sg)) {
		xgene_dma_load_single_sg(
			xgene_dma_lookup_ext8(msg2, i), 
			src_sg, &paddr, &nbytes, offset);
		return 2;	/* 5 buffers - 64B msg */
	}
	
	slot->ring_src_ll = 
		dma_zalloc_coherent(dev, 
			sizeof(struct rmsg_ext8) * XGENE_MAX_SRC_COUNT,
			&slot->ring_src_ll_pa, GFP_ATOMIC);
	if (!slot->ring_src_ll) {
		dev_err(dev, "Couldn't allocate memory for src link list\n");
		return -ENOMEM;
	}
	
	*flags |= FLAG_SRC_LINKLIST_ACTIVE;
	ext8 = slot->ring_src_ll;
	ll_len = 0;
	ll_count = 0;

	for (i = 0; nbytes > 0 && i < XGENE_MAX_SRC_COUNT; i++) {
		len = xgene_dma_load_single_sg(
				&ext8[((ll_count % 2) ? (ll_count - 1) : (ll_count + 1))],
				src_sg, &paddr, &nbytes, offset);

		xgene_dma_cpu_to_le64(
				&ext8[((ll_count % 2) ? (ll_count - 1) :
				(ll_count + 1))], 2);
		ll_count++;
		ll_len += len;

		if (!paddr && nbytes > 0) {
			src_sg = sg_next(src_sg);
			offset = 0;
		} else {
			offset += len;
		}
	}

	/* Encode link list byte count and link count */
	RMSG_LL_SET(msg1, 1);	/* Linked list of buffers 6 or more */
	RMSG_NXTLINKLISTLENGTH_SET(msg2, ll_count);
	RMSG_TOTDATALENGTHLINKLISTLSB_SET(msg1, ll_len);
	RMSG_TOTDATALENGTHLINKLISTMSB_SET(msg2, (ll_len >> 12));
	RMSG_NXTDATAPTRL_SET(msg2, (u32) slot->ring_src_ll_pa);
	RMSG_NXTDATAPTRH_SET(msg2, (u32) (slot->ring_src_ll_pa >> 32));

	if (nbytes == 0 && ll_len <= XGENE_MAX_LL_DATA_LENGTH)
		return 2;	/* 5 - 16 buffers - 64B msg */

	dev_err(dev, "Source buffer length is too long Error %d", -EINVAL);

	dma_free_coherent(dev, 
		sizeof(struct rmsg_ext8) * XGENE_MAX_SRC_COUNT, 
		slot->ring_src_ll, slot->ring_src_ll_pa);
	slot->ring_src_ll = NULL;

	return -EINVAL;
}

static int xgene_dma_load_dst_sg(struct xgene_dma_chan *chan, 
				struct xgene_dma_slot *slot, 
				void *msg1, 
				struct scatterlist *dst_sg, 
				u32 nbytes)
{
	struct device *dev = chan->pdma->dev;
	struct rmsg_ext8 *ext8;
	dma_addr_t paddr = 0;
	u32 len;
	u32 offset = 0;
	int ll_count;
	int i;

	slot->ring_dst_ll = 
		dma_zalloc_coherent(dev, 
			sizeof(struct rmsg_ext8) * XGENE_MAX_DST_COUNT,
			&slot->ring_dst_ll_pa, GFP_ATOMIC);
	if (!slot->ring_dst_ll) {
		dev_err(dev, "Couldn't allocate memory for destination link list\n");
		return -ENOMEM;
	}

	slot->flags |= FLAG_DST_LINKLIST_ACTIVE;
	ext8 = slot->ring_dst_ll;
	ll_count = 0;

	for (i = 0; nbytes > 0 && i < XGENE_MAX_DST_COUNT; i++) {
		len = xgene_dma_load_single_sg(
					&ext8[((ll_count % 2) ? (ll_count - 1) : 
					(ll_count + 1))], dst_sg, &paddr, 
					&nbytes, offset);

		xgene_dma_cpu_to_le64(
				&ext8[((ll_count % 2) ? (ll_count - 1) : 
				(ll_count + 1))], 2);
		ll_count++;

		if (!paddr && nbytes > 0) {
			dst_sg = sg_next(dst_sg);
			offset = 0;
		} else {
			offset += len;
		}
	}

	RMSG_H0INFO_LSBL_SET(msg1, (u32) (slot->ring_dst_ll_pa >> 4));
	RMSG_H0INFO_LSBH_SET(msg1, (u32) (slot->ring_dst_ll_pa >> 36));
	RMSG_LINK_SIZE_SET(msg1, ll_count);

	if (nbytes == 0)
		return 0;	/* 5 - 16 buffers - 64B msg */

	dev_err(dev, "Destination buffer length is too long Error %d", -EINVAL);

	dma_free_coherent(dev, 
		sizeof(struct rmsg_ext8) * XGENE_MAX_DST_COUNT, 
		slot->ring_dst_ll, slot->ring_dst_ll_pa);
	slot->ring_dst_ll = NULL;

	return -EINVAL;
}

static dma_cookie_t xgene_dma_tx_sgcpy_submit(
	struct dma_async_tx_descriptor *tx)
{
	struct xgene_dma_chan *chan = to_pktdma_chan(tx->chan);
	struct xgene_qmtm_qinfo *qdesc = &chan->tx_qdesc;
	struct xgene_dma_slot *slot = tx_to_pktdma_slot(tx);
	struct scatterlist *sg;
	struct scatterlist *src_sg;
	struct scatterlist *dst_sg;
	dma_cookie_t cookie;
	void *msg1;
	void *msg2;
	int command;
	int i;
	int ret;
	u32 length = 0;

	spin_lock_bh(&chan->lock);

	slot->ring_cnt = 1;

	src_sg = slot->srcdst_list;
	dst_sg = slot->srcdst_list + 
			sizeof(struct scatterlist) * slot->src_cnt;

	for_each_sg(src_sg, sg, slot->src_cnt, i) 
		length += sg_dma_len(sg);

	msg1 = &qdesc->msg32[qdesc->qhead];
	if (++qdesc->qhead == qdesc->count)
		qdesc->qhead = 0;
	memset(msg1, 0, 32);

	msg2 = &qdesc->msg32[qdesc->qhead];
	memset(msg2, 0, 32);

	RMSG_C_SET(msg1, 1); /* Coherent IO */
	RMSG_USERINFO_SET(msg1, xgene_dma_encode_uinfo(slot));
	RMSG_RTYPE_SET(msg1, chan->pdma->sdev->slave_id);
	RMSG_H0ENQ_NUM_SET(msg1, QMTM_QUEUE_ID(chan->pdma->sdev->qmtm_ip,
					       chan->rx_qdesc.queue_id));
	
	ret = xgene_dma_load_src_sg(chan, &slot->ring_src, 
					msg1,msg2, src_sg, 
					length, &slot->flags);
	if (ret < 0)
		goto err_ll;

	command = ret;

	if (slot->dst_cnt == 1) {
		RMSG_DR_SET(msg1, 1);
		RMSG_H0INFO_LSBL_SET(msg1, (u32) sg_dma_address(dst_sg));
		RMSG_H0INFO_LSBH_SET(msg1, (u32) (sg_dma_address(dst_sg) >> 32));
	} else {
		ret = xgene_dma_load_dst_sg(chan, slot, msg1, 
					dst_sg, length);
		if (ret < 0)
			goto err_ll;
	}			
		
	if (command == 2) {
		if (++qdesc->qhead == qdesc->count)
			qdesc->qhead = 0;
	} else {
		msg2 = NULL;
	}

	xgene_dma_cpu_to_le64(msg1, 8);
	xgene_dma_cpu_to_le64(msg2, 8);

	hex_dump("TX QMSG1: ", msg1, 32);
	hex_dump("TX QMSG2: ", msg2, 32);

	cookie = dma_cookie_assign(tx);

	/* Notify HW */
	writel(command, chan->tx_qdesc.command);

	spin_unlock_bh(&chan->lock);

	return cookie;

err_ll:
	/* Roll back the ring descriptor by 2 */
	qdesc->qhead = qdesc->qhead == 0 ? qdesc->count - 1 : qdesc->qhead - 1;
	qdesc->qhead = qdesc->qhead == 0 ? qdesc->count - 1 : qdesc->qhead - 1;

	if (slot->srcdst_list)
		devm_kfree(chan->pdma->dev, slot->srcdst_list);

	spin_unlock_bh(&chan->lock);

	return -ENOMEM;
}

static struct dma_async_tx_descriptor *xgene_dma_prep_dma_sg(
	struct dma_chan *channel, struct scatterlist *dst_sg, 
	u32 dst_nents, struct scatterlist *src_sg, u32 src_nents, 
	unsigned long flags)
{
	struct xgene_dma_chan *chan = to_pktdma_chan(channel);
	struct xgene_dma_slot *slot;
	void *srcdst_list;
	
	srcdst_list = devm_kzalloc(chan->pdma->dev,
				   sizeof(struct scatterlist) *
				   (dst_nents + src_nents),
				   GFP_KERNEL);
	if (!srcdst_list)
		return NULL;

	spin_lock_bh(&chan->lock);
	slot = xgene_dma_channel_get_slot(chan);
	if (!slot) {
		spin_unlock_bh(&chan->lock);
		dev_dbg(chan->pdma->dev, "channel %d no slot\n",
			chan->index);
		return NULL;
	}

	dev_dbg(chan->pdma->dev,
		"gather-scatter channel %d slot %d src_nents %d dst_nents %d\n",
		chan->index, slot->index, src_nents, dst_nents);

	/* Set up slot variable */
	slot->flags =  FLAG_DSTSRC_LIST_ACTIVE | FLAG_SG_ACTIVE;
    	slot->async_tx.flags = flags;
	slot->async_tx.tx_submit = xgene_dma_tx_sgcpy_submit;

	/* Make shadow copy for use during submit */
	slot->srcdst_list = srcdst_list;
	memcpy(slot->srcdst_list, src_sg, src_nents * sizeof(struct scatterlist));
	memcpy(slot->srcdst_list + src_nents * sizeof(struct scatterlist),
		       dst_sg, dst_nents * sizeof(struct scatterlist));
	slot->src_cnt = src_nents;
	slot->dst_cnt = dst_nents;

	spin_unlock_bh(&chan->lock);

	return &slot->async_tx;
}

static struct xgene_dma_chan *__xgene_get_flyby_channel(void)
{
	return &pktdma->channels[XGENE_FBY_CHANNEL];
}

static struct xgene_flyby_slot *xgene_get_flyby_slot(
			struct xgene_dma_chan *chan)
{
	struct xgene_flyby_slot *slot;

	slot = (chan->last_used_flyby == 
			&chan->flyby_slots[XGENE_SLOT_PER_CHANNEL - 1]) ?
			&chan->flyby_slots[0] : (chan->last_used_flyby + 1);

	if (slot->busy)
		return NULL;

	chan->last_used_flyby = slot;
	slot->busy = true;

	return slot;
}

int xgene_flyby_alloc_resources(void)
{
	struct xgene_dma_chan *chan = __xgene_get_flyby_channel();
	int i;

	spin_lock_bh(&chan->lock);

	if (!chan->flyby_slots) {
		chan->flyby_slots = 
			devm_kzalloc(chan->pdma->dev,
				sizeof(struct xgene_flyby_slot) *
				XGENE_SLOT_PER_CHANNEL, GFP_ATOMIC);
		if (!chan->flyby_slots) {
			spin_unlock_bh(&chan->lock);
			return -ENOMEM;
		}
	}

	for (i = 0; i < XGENE_SLOT_PER_CHANNEL; i++) {
		chan->flyby_slots[i].busy = false;
		chan->flyby_slots[i].index = XGENE_SLOT_PER_CHANNEL + i;
	}

	chan->last_used_flyby = &chan->flyby_slots[0];
	
	dev_dbg(chan->pdma->dev, "allocated %d flyby descriptors",
		XGENE_SLOT_PER_CHANNEL);

	spin_unlock_bh(&chan->lock);

	return 0;
}
EXPORT_SYMBOL_GPL(xgene_flyby_alloc_resources);

void xgene_flyby_free_resources(void)
{
	struct xgene_dma_chan *chan = __xgene_get_flyby_channel();

	spin_lock_bh(&chan->lock);

	devm_kfree(chan->pdma->dev, chan->flyby_slots);
	chan->flyby_slots = NULL;
	chan->last_used_flyby = NULL;

	dev_dbg(chan->pdma->dev, "free %d flyby descriptors\n",
		XGENE_SLOT_PER_CHANNEL);

	spin_unlock_bh(&chan->lock);
}
EXPORT_SYMBOL_GPL(xgene_flyby_free_resources);

int xgene_flyby_op(void (*cb_func)(void *ctx, u32 result), void *ctx, 
	struct scatterlist *src_sg, u32 nbytes, u32 seed, u8 opcode)
{
	struct xgene_dma_chan *chan = __xgene_get_flyby_channel();
	struct xgene_qmtm_qinfo *qdesc = &chan->tx_qdesc;
	struct xgene_flyby_slot *slot;
	void *msg1;
	void *msg2;
	int ret;
	int command;

	if (nbytes >= XGENE_MAX_FBY_DATA_LENGTH) {
		dev_err(chan->pdma->dev, 
			"flyby source buffer length is too long\n");
		return -EINVAL;
	}

	spin_lock_bh(&chan->lock);

	slot = xgene_get_flyby_slot(chan);
	if (!slot) {
		spin_unlock_bh(&chan->lock);
		dev_dbg(chan->pdma->dev, "channel %d no slot\n", 
			chan->index);
		return -EINVAL;
	}

	dev_dbg(chan->pdma->dev, "flyby channel %d slot %d len %u\n",
		chan->index, slot->index, nbytes);

	slot->ring_cnt = 1;
	slot->flyby_cb = cb_func;
	slot->flyby_ctx = ctx;
	slot->src_sg = src_sg;
	slot->src_nents = sg_nents(src_sg);

	ret = dma_map_sg(chan->pdma->dev, slot->src_sg, 
				slot->src_nents, DMA_TO_DEVICE); 
	if (ret == 0) {
		slot->busy = false;
		spin_unlock_bh(&chan->lock);
		dev_err(chan->pdma->dev, "Unable to map source sg\n");
		return -EINVAL;
	}

	msg1 = &qdesc->msg32[qdesc->qhead];
	if (++qdesc->qhead == qdesc->count)
		qdesc->qhead = 0;
	memset(msg1, 0, 32);

	msg2 = &qdesc->msg32[qdesc->qhead];
	memset(msg2, 0, 32);

	RMSG_C_SET(msg1, 1); /* Coherent IO */
	RMSG_USERINFO_SET(msg1, xgene_flyby_encode_uinfo(slot));
	RMSG_RTYPE_SET(msg1, chan->pdma->sdev->slave_id);
	RMSG_H0ENQ_NUM_SET(msg1, QMTM_QUEUE_ID(chan->pdma->sdev->qmtm_ip,
					       chan->rx_qdesc.queue_id));
	
	ret = xgene_dma_load_src_sg(chan, &slot->ring_src, msg1, 
				msg2, src_sg, nbytes, &slot->flags);
	if (ret < 0)
		goto err_ll;

	command = ret;

	if (command == 2) {
		if (++qdesc->qhead == qdesc->count)
			qdesc->qhead = 0;
	} else {
		msg2 = NULL;
	}

	seed = bitrev32(seed);
	RMSG_BD_SET(msg1, 1);
	RMSG_CRCBYTECNT_SET(msg1, nbytes);
	RMSG_FBY_SET(msg1, opcode);
	RMSG_SD_SET(msg1, 1);
	RMSG_CRCSEEDL_SET(msg1, seed);
	RMSG_CRCSEEDH_SET(msg1, seed >> 24);
	
	xgene_dma_cpu_to_le64(msg1, 8);
	xgene_dma_cpu_to_le64(msg2, 8);

	hex_dump("TX QMSG1: ", msg1, 32);
	hex_dump("TX QMSG2: ", msg2, 32);

	/* Notify HW */
	writel(command, chan->tx_qdesc.command);

	spin_unlock_bh(&chan->lock);

	return -EINPROGRESS;

err_ll:
	/* Roll back the ring descriptor by 2 */
	qdesc->qhead = qdesc->qhead == 0 ? qdesc->count - 1 : qdesc->qhead - 1;
	qdesc->qhead = qdesc->qhead == 0 ? qdesc->count - 1 : qdesc->qhead - 1;

	slot->busy = false;
	spin_unlock_bh(&chan->lock);

	return -ENOMEM;
}
EXPORT_SYMBOL_GPL(xgene_flyby_op);

static void __xgene_dma_prep_dma_xor_one(struct xgene_dma_chan *chan,
	struct xgene_dma_slot *slot, dma_addr_t *dst, dma_addr_t *src, 
	int src_cnt, size_t *length, const unsigned char *scf,
	u32 queue_id)
{
	struct xgene_qmtm_qinfo *qdesc = &chan->tx_qdesc;
	void *msg1;
	void *msg2;
	size_t len;
	int i;

	msg1 = &qdesc->msg32[qdesc->qhead];
	memset(msg1, 0, 32);
	if (++qdesc->qhead == qdesc->count)
		qdesc->qhead = 0;
	msg2 = &qdesc->msg32[qdesc->qhead];
	memset(msg2, 0, 32);
	if (++qdesc->qhead == qdesc->count)
		qdesc->qhead = 0;

	/* Load 1st src buffer */
	len = *length;
	xgene_dma_rmsg_src_set(msg1 + 8, &len, &src[0]);
	RMSG_MULTI0_SET(msg1, scf[0]);

	/* Load 2nd to 4th src buffer */
	for (i = 1; i < src_cnt; i++) {
		len = *length;
		xgene_dma_rmsg_src_set(xgene_dma_lookup_ext8(msg2, i - 1),
					  &len, &src[i]);
		switch (i) {
		case 1:
			RMSG_MULTI1_SET(msg1, scf[i]);
			break;
		case 2:
			RMSG_MULTI2_SET(msg1, scf[i]);
			break;
		case 3:
			RMSG_MULTI3_SET(msg1, scf[i]);
			break;
		case 4:
			RMSG_MULTI4_SET(msg1, scf[i]);
			break;
		}
	}
	*length = len;

	RMSG_C_SET(msg1, 1); /* Coherent IO */

	RMSG_USERINFO_SET(msg1, xgene_dma_encode_uinfo(slot));
	RMSG_RTYPE_SET(msg1, chan->pdma->sdev->slave_id);

	RMSG_DR_SET(msg1, 1);
	RMSG_NV_SET(msg1, 1);
	RMSG_H0INFO_LSBL_SET(msg1, (u32) *dst);
	RMSG_H0INFO_LSBH_SET(msg1, (u32) (*dst >> 32));
	RMSG_H0ENQ_NUM_SET(msg1, QMTM_QUEUE_ID(
					chan->pdma->sdev->qmtm_ip, 
					queue_id));

	RMSG_FBY_SET(msg1, xgene_dma_encode_xor_flyby(src_cnt));

	*dst += XGENE_BUFFER_MAX_SIZE;

	xgene_dma_cpu_to_le64(msg1, 8);
	xgene_dma_cpu_to_le64(msg2, 8);

	hex_dump("TX QMSG1: ", msg1, 32);
	hex_dump("TX QMSG2: ", msg2, 32);
}

static dma_cookie_t xgene_dma_tx_xor_submit(
	struct dma_async_tx_descriptor *tx)
{
	struct xgene_dma_chan *chan = to_pktdma_chan(tx->chan);
	struct xgene_dma_slot *slot = tx_to_pktdma_slot(tx);
	dma_cookie_t cookie;
	int command = 0;
	static unsigned char scf[XGENE_MAX_XOR_BUFFER] =  { 0 };
	dma_addr_t src[XGENE_MAX_XOR_BUFFER];
	dma_addr_t dst = slot->dst[0];
	size_t length = slot->length;

	spin_lock_bh(&chan->lock);

	memcpy(src, slot->src, slot->src_cnt * sizeof(*src));

	for (slot->ring_cnt = 0;
	     slot->ring_cnt < XGENE_SLOT_NUM_QMMSG && length > 0;
	     slot->ring_cnt++) {
		__xgene_dma_prep_dma_xor_one(chan, slot, &dst, src, 
						slot->src_cnt, &length,
						scf, chan->rx_qdesc.queue_id);
		command += 2;
	}

	cookie = dma_cookie_assign(tx);

	/* Notify HW */
	writel(command, chan->tx_qdesc.command);

	spin_unlock_bh(&chan->lock);

	return cookie;
}

static struct dma_async_tx_descriptor *xgene_dma_prep_dma_xor(
	struct dma_chan *channel, dma_addr_t dst, dma_addr_t *src,
	unsigned int src_cnt, size_t len, unsigned long flags)
{
	struct xgene_dma_chan *chan = to_pktdma_chan(channel);
	struct xgene_dma_slot *slot;

	spin_lock_bh(&chan->lock);

	slot = xgene_dma_channel_get_slot(chan);
	if (!slot) {
	 	spin_unlock_bh(&chan->lock);
		dev_dbg(chan->pdma->dev, "channel %d no slot\n",
			chan->index);
		return NULL;
	}

	dev_dbg(chan->pdma->dev,
		"DMA XOR channel %d slot %d len %zu src_cnt %d dst 0x%llX\n",
		chan->index, slot->index, len, src_cnt, dst);

	slot->flags = FLAG_UNMAP_FIRST_DST;
	slot->async_tx.flags = flags;
	slot->async_tx.tx_submit = xgene_dma_tx_xor_submit;
	memcpy(slot->src, src, src_cnt * sizeof(*src));
	slot->src_cnt = src_cnt;
	slot->length = len;
	slot->dst[0] = dst;

	spin_unlock_bh(&chan->lock);

	return &slot->async_tx;
}

static dma_cookie_t xgene_dma_tx_pq_submit(
	struct dma_async_tx_descriptor *tx)
{
	struct xgene_dma_chan *chan = to_pktdma_chan(tx->chan);
	struct xgene_dma_slot *slot = tx_to_pktdma_slot(tx);
	u32 queue_id = chan->rx_qdesc.queue_id;
	dma_cookie_t cookie;
	int command = 0;
	int i;

	slot->ring_cnt = 0;

	if (!(slot->async_tx.flags & DMA_PREP_PQ_DISABLE_P)) {
		struct xgene_dma_chan *chan_p = &chan->pdma->channels[0];
		static unsigned char scf[XGENE_MAX_XOR_BUFFER] =  { 0 };
		dma_addr_t src[XGENE_MAX_XOR_BUFFER];
		dma_addr_t dst = slot->dst[0];
		size_t length = slot->length;

		slot->flags |= FLAG_UNMAP_FIRST_DST;

		spin_lock_bh(&chan_p->lock);

		memcpy(src, slot->src, sizeof(*src) * slot->src_cnt);
		command = 0;
		for (i = 0; i < XGENE_SLOT_NUM_QMMSG && length > 0; i++) {
			__xgene_dma_prep_dma_xor_one(chan_p, slot, 
							&dst, src, 
							slot->src_cnt,
							&length, scf, 
							queue_id);
			command += 2;
			slot->ring_cnt++;
		}
		spin_unlock_bh(&chan_p->lock);
	} 

	spin_lock_bh(&chan->lock);

	if (!(slot->async_tx.flags & DMA_PREP_PQ_DISABLE_Q)) {
		dma_addr_t src[XGENE_MAX_XOR_BUFFER];
		dma_addr_t dst = slot->dst[1];
		size_t length = slot->length;

		memcpy(src, slot->src, sizeof(*src) * slot->src_cnt);
		slot->flags |= FLAG_UNMAP_SECOND_DST;
		command = 0;
		for (i = 0; i < XGENE_SLOT_NUM_QMMSG && length > 0;
		     i++) {
			__xgene_dma_prep_dma_xor_one(chan, slot,
							&dst, src,
							slot->src_cnt,
							&length,
							slot->scf,
							queue_id);
			command += 2;
			slot->ring_cnt++;
		}
	}

	cookie = dma_cookie_assign(tx);

	/* Notify HW */
	if (!(slot->async_tx.flags & DMA_PREP_PQ_DISABLE_P)) 
		writel(command, 
			chan->pdma->channels[0].tx_qdesc.command);

	if (!(slot->async_tx.flags & DMA_PREP_PQ_DISABLE_Q)) 
		writel(command, chan->tx_qdesc.command);

	spin_unlock_bh(&chan->lock);

	return cookie;
}

static struct dma_async_tx_descriptor *xgene_dma_prep_dma_pq(
	struct dma_chan *channel, dma_addr_t *dst, dma_addr_t *src,
	unsigned int src_cnt, const unsigned char *scf, size_t len,
	unsigned long flags)
{
	struct xgene_dma_chan *chan = to_pktdma_chan(channel);
	struct xgene_dma_slot *slot;

	spin_lock_bh(&chan->lock);

	slot = xgene_dma_channel_get_slot(chan);
	if (!slot) {
	 	spin_unlock_bh(&chan->lock);
		dev_dbg(chan->pdma->dev, "channel %d no slot\n",
			chan->index);
		return NULL;
	}

	dev_dbg(chan->pdma->dev,
		"DMA PQ channel %d slot %d len %zu src_cnt %d dst0 0x%llX dst1 0x%llX\n",
		chan->index, slot->index, len, src_cnt, dst[0], dst[1]);

	slot->flags = 0;
	slot->async_tx.flags = flags;
	slot->async_tx.tx_submit = xgene_dma_tx_pq_submit;
	memcpy(slot->src, src, sizeof(*src) * src_cnt);
	slot->src_cnt = src_cnt;
	slot->length = len;
	slot->dst[0] = dst[0];
	slot->dst[1] = dst[1];
	memcpy(slot->scf, scf, sizeof(*scf) * src_cnt);

	spin_unlock_bh(&chan->lock);

	return &slot->async_tx;
}

static int xgene_dma_channels_init(struct xgene_dma *pdma)
{
	struct xgene_dma_chan *channel;
	int i;

	for (i = 0; i < XGENE_MAX_CHANNEL; i++) {
		channel = &pdma->channels[i];
		channel->pdma = pdma;
		channel->index = i;
		channel->slots = NULL;
		spin_lock_init(&channel->lock);
		tasklet_init(&channel->rx_tasklet, xgene_dma_bh_tasklet_cb,
			     (unsigned long) channel);
	}

	return 0;
}

static int xgene_dma_async_init_one(struct xgene_dma *pdma,
				    struct xgene_dma_chan *channel,
				    struct dma_device *dma_dev)
{
	int rc;

	INIT_LIST_HEAD(&dma_dev->channels);
	if (channel->index == XGENE_XOR_CHANNEL ||
	    channel->index == XGENE_Q_CHANNEL) {
#ifdef XGENE_DMA_ENABLE_M2M_ALL_CHANNELS
		dma_cap_set(DMA_MEMCPY, dma_dev->cap_mask);
		dma_cap_set(DMA_SG, dma_dev->cap_mask);
#endif
	} else {
		dma_cap_set(DMA_MEMCPY, dma_dev->cap_mask);
		dma_cap_set(DMA_SG, dma_dev->cap_mask);
	}

	/* Only channel 0 support XOR and channel 1 support PQ */
	if (channel->index == XGENE_XOR_CHANNEL)
		dma_cap_set(DMA_XOR, dma_dev->cap_mask);

	/*
	 * Only channel 1 support Q part of the PQ operation. The P will
	 * use channel 0.
	 */
	if (channel->index == XGENE_Q_CHANNEL)
		dma_cap_set(DMA_PQ, dma_dev->cap_mask);

	list_add_tail(&channel->dma_chan.device_node, &dma_dev->channels);

	channel->dma_chan.device = dma_dev;
	sprintf(channel->name, "PktDMA%d", channel->index);

	rc = request_irq(channel->rx_qdesc.irq, xgene_dma_qm_msg_isr, 0,
			channel->name, channel);
	if (rc) {
		dev_err(pdma->dev,
			"Failed to register IRQ %d for channel %d\n",
			channel->rx_qdesc.irq, channel->index);
		return rc;
	}

	dev_dbg(pdma->dev, "Adding channel %d IRQ %d",
		channel->index, channel->rx_qdesc.irq);

	/* Set base and prep routines */
	dma_dev->dev = pdma->dev;
	dma_dev->device_alloc_chan_resources = xgene_dma_alloc_resources;
	dma_dev->device_free_chan_resources = xgene_dma_free_resources;
	dma_dev->device_tx_status = xgene_dma_tx_status;
	dma_dev->device_issue_pending = xgene_dma_issue_pending;

	if (dma_has_cap(DMA_MEMCPY, dma_dev->cap_mask))
		dma_dev->device_prep_dma_memcpy = xgene_dma_prep_dma_memcpy;
	if (dma_has_cap(DMA_SG, dma_dev->cap_mask))
		dma_dev->device_prep_dma_sg = xgene_dma_prep_dma_sg;
	if (dma_has_cap(DMA_XOR, dma_dev->cap_mask)) {
		dma_dev->device_prep_dma_xor = xgene_dma_prep_dma_xor;
		dma_dev->max_xor = XGENE_MAX_XOR_BUFFER;
		dma_dev->xor_align = XGENE_XOR_ALIGNMENT;
	}
	if (dma_has_cap(DMA_PQ, dma_dev->cap_mask)) {
		dma_dev->device_prep_dma_pq = xgene_dma_prep_dma_pq;
		dma_dev->max_pq = XGENE_MAX_XOR_BUFFER;
		dma_dev->pq_align = XGENE_XOR_ALIGNMENT;
	}

	return 0;
}

static int xgene_dma_async_init(struct xgene_dma *pdma)
{
	int rc;
	int i;

	/* Register with Linux async DMA */
	for (i = 0; i < XGENE_MAX_CHANNEL; i++) {
		rc = xgene_dma_async_init_one(pdma, &pdma->channels[i],
					      &pdma->dma_dev[i]);
		if (rc)
			return rc;
		rc = dma_async_device_register(&pdma->dma_dev[i]);
		if (rc) {
			dev_err(pdma->dev,
				"Unable to register with Async DMA error 0x%08X",
				rc);
			return rc;
		}
	}

	return 0;
}

#if defined(CONFIG_OF)
static const struct of_device_id xgene_dma_match[] = {
	{ .compatible   = "apm,xgene-pktdma", },
	{},
};
MODULE_DEVICE_TABLE(of, xgene_dma_match);
#endif

static int xgene_dma_runtime_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct xgene_dma *pdma = platform_get_drvdata(pdev);

	if (!PTR_ERR(pdma->dma_clk))
		clk_disable_unprepare(pdma->dma_clk);

	return 0;
}

static int xgene_dma_runtime_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct xgene_dma *pdma = platform_get_drvdata(pdev);
	int ret;

	if (!PTR_ERR(pdma->dma_clk)) {
		ret = clk_prepare_enable(pdma->dma_clk);
		if (ret) {
			dev_err(dev, "clk_enable failed: %d\n", ret);
			return ret;
		}
	}
	return 0;
}

static int xgene_dma_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct xgene_dma *pdma;
	struct resource *res;
	const char *name;
	const char default_name[] = "PKTDMA";
	u8 wq_pbn_start;
	u8 wq_pbn_count;
	u8 fq_pbn_start;
	u8 fq_pbn_count;
	u32 qmtm_ip;
	u32 info[5];
	u32 val;
	int ret;

	pdma = devm_kzalloc(&pdev->dev, sizeof(*pdma), GFP_KERNEL);
	if (IS_ERR(pdma))
		return PTR_ERR(pdma);

	pktdma = pdma;

	spin_lock_init(&pdma->lock);
	pdma->dev = &pdev->dev;
	platform_set_drvdata(pdev, pdma);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	pdma->csr = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(pdma->csr))
                return PTR_ERR(pdma->csr);

	pdma->csr_diag = pdma->csr + XGENE_PKTDMA_DIAG_CSR_OFFSET;
	pdma->csr_ring = pdma->csr + XGENE_PKTDMA_QMI_CSR_OFFSET;

	dev_dbg(&pdev->dev, "CSR PAddr 0x%016LX VAddr 0x%p size %lld",
		res->start, pdma->csr, resource_size(res));

	/* EFI BOOT and ACPI */
	if (efi_enabled(EFI_BOOT)) {
		name = &default_name[0];
		qmtm_ip = 0x1;
		wq_pbn_start = 0x00;
		wq_pbn_count = 4;
		fq_pbn_start = 0x20;
		fq_pbn_count = 8;
	} else {
		of_property_read_string(np, "slave_name", &name);
		of_property_read_u32_array(np, "slave_info", info, ARRAY_SIZE(info));
		qmtm_ip = info[0];
		wq_pbn_start = info[1];
		wq_pbn_count = info[2];
		fq_pbn_start = info[3];
		fq_pbn_count = info[4];
	}

	pdma->sdev = xgene_qmtm_set_sdev(name, qmtm_ip, wq_pbn_start,
					 wq_pbn_count, fq_pbn_start,
					 fq_pbn_count);
	if (!pdma->sdev) {
		dev_err(&pdev->dev, "QMTM%d slave %s error\n", qmtm_ip, name);
		return -ENODEV;
	}

	pdma->dma_clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(pdma->dma_clk) && !efi_enabled(EFI_BOOT)) {
		dev_err(&pdev->dev, "no clock entry\n");
		return PTR_ERR(pdma->dma_clk);
	}

	pm_runtime_enable(&pdev->dev);
	if (!pm_runtime_enabled(&pdev->dev)) {
		ret = xgene_dma_runtime_resume(&pdev->dev);
		if (ret) {
			dev_err(&pdev->dev,
				"xgene_dma_runtime_resume failed %d\n",
				ret);
			goto err_rt_resume;
		}
	}

	/* Enable clock before accessing registers */
	if (!IS_ERR(pdma->dma_clk)) {
	        ret = clk_prepare_enable(pdma->dma_clk);
		if (ret) {
			dev_err(&pdev->dev, "clk_prepare_enable failed: %d\n",
				ret);
			goto err_pm_enable;
		}
	}

	/* Remove RAM out of shutdown */
	ret = xgene_dma_init_mem(pdma);
	if (ret)
		goto err_hw;

	/* Register for PktDMA error interrupt */
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		ret = -EINVAL;
		dev_err(&pdev->dev, "No irq resource\n");
		goto err_irq;
	}
	pdma->irq = res->start;
	ret = devm_request_irq(&pdev->dev, pdma->irq, xgene_dma_err_isr,
			       0, "PktDMA", pdma);
	if (ret) {
		dev_err(&pdev->dev, "request_irq failed with err %d\n", ret);
		goto err_irq;
	}
	xgene_dma_enable_error_irq(pdma);

	pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;
	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(64);

	/* Initialize channels software state */
	xgene_dma_channels_init(pdma);

	/* Configure Pkt DMA queue */
	ret = xgene_dma_hw_init(pdma);
	if (ret)
		goto err_hw;

	/* Configure linux Async DMA */
	ret = xgene_dma_async_init(pdma);
	if (ret)
		goto err_async;

	val = readl(pdma->csr + DMA_IPBRR);
	dev_info(pdma->dev, "PktDMA v%d.%02d.%02d driver register %d channels",
		 REV_NO_RD(val), BUS_ID_RD(val), DEVICE_ID_RD(val),
		 XGENE_MAX_CHANNEL);

	return 0;

err_async:
err_irq:
err_hw:
err_pm_enable:
	if (!pm_runtime_status_suspended(&pdev->dev))
		xgene_dma_runtime_suspend(&pdev->dev);

err_rt_resume:
	pm_runtime_disable(&pdev->dev);

	return ret;
}

static int xgene_dma_remove(struct platform_device *pdev)
{
	struct xgene_dma *pdma = platform_get_drvdata(pdev);
	struct xgene_dma_chan *chan;
	int i;

	for (i = 0; i < XGENE_MAX_CHANNEL; i++) {
		chan = &pdma->channels[i];
		dma_async_device_unregister(&pdma->dma_dev[i]);
	}

	pm_runtime_disable(&pdev->dev);
	if (!pm_runtime_status_suspended(&pdev->dev))
		xgene_dma_runtime_suspend(&pdev->dev);

	return 0;
}

static const struct dev_pm_ops xgene_dma_pm_ops = {
#ifdef CONFIG_PM_RUNTIME
	.runtime_suspend = xgene_dma_runtime_suspend,
	.runtime_resume = xgene_dma_runtime_resume,
#endif
};

static const struct acpi_device_id xgene_dma_acpi_match[] = {
        {"APMC0D16", },
        {},
};
MODULE_DEVICE_TABLE(acpi, xgene_dma_acpi_match);

static struct platform_driver xgene_dma_driver = {
	.driver = {
		.name = "XGene-PktDMA",
		.owner = THIS_MODULE,
		.pm = &xgene_dma_pm_ops,
		.of_match_table = xgene_dma_match,
		.acpi_match_table = ACPI_PTR(xgene_dma_acpi_match),
	},
	.probe = xgene_dma_probe,
	.remove = xgene_dma_remove,
};

static int __init xgene_dma_init(void)
{
	return platform_driver_register(&xgene_dma_driver);
}
late_initcall(xgene_dma_init);

static void __exit xgene_dma_exit(void)
{
	platform_driver_unregister(&xgene_dma_driver);
}
module_exit(xgene_dma_exit);

MODULE_AUTHOR("Applied Micro Circuit Corporation");
MODULE_DESCRIPTION("APM X-Gene Packet DMA driver");
MODULE_LICENSE("GPL");

