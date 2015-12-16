/*
 * APM X-Gene SoC Security Driver
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
 * This file defines the private crypto driver structure and messaging
 * format of the security hardware.
 */
#ifndef __XGENE_SEC_H__
#define __XGENE_SEC_H__

#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/uio_driver.h>
#include <misc/xgene/qmtm/xgene_qmtm.h>
#include "../../misc/xgene/qmtm/xgene_qmtm_core.h"

#undef APM_SEC_TXDEBUG
#undef APM_SEC_RXDEBUG
#undef APM_SEC_SATKNDEBUG
#undef APM_SEC_QMDEBUG

/* Debugging Macro */
#define APMSEC_HDR		"XGSEC: "

#if !defined(APM_SEC_TXDEBUG)
# define APMSEC_TXLOG(fmt, ...)
# define APMSEC_TXDUMP(hdr, d, l)
#else
# define APMSEC_TXLOG(fmt, ...)		\
	do { \
		printk(KERN_INFO APMSEC_HDR fmt, ##__VA_ARGS__); \
	} while(0);
# define APMSEC_TXDUMP(hdr, d, l)	\
	do { \
		print_hex_dump(KERN_INFO, APMSEC_HDR hdr, \
			DUMP_PREFIX_ADDRESS, 16, 4,  d, l, 1); \
} while(0);
#endif

#if !defined(APM_SEC_RXDEBUG)
# define APMSEC_RXLOG(fmt, ...)
# define APMSEC_RXDUMP(hdr, d, l)
#else
# define APMSEC_RXLOG(fmt, ...)		\
	do { \
		printk(KERN_INFO APMSEC_HDR fmt, ##__VA_ARGS__); \
	} while(0);
# define APMSEC_RXDUMP(hdr, d, l)	\
	do { \
		print_hex_dump(KERN_INFO, APMSEC_HDR hdr, \
			DUMP_PREFIX_ADDRESS, 16, 4,  d, l, 1); \
	} while(0);
#endif

#if !defined(APM_SEC_SATKNDEBUG)
# define APMSEC_SATKNLOG(fmt, ...)
# define APMSEC_SADUMP(s, l)
# define APMSEC_TKNDUMP(t)
#else
# define APMSEC_SATKNLOG(fmt, ...)	\
	do { \
		printk(KERN_INFO APMSEC_HDR fmt, ##__VA_ARGS__); \
	} while(0);
# define APMSEC_SADUMP(s, l) 		sec_sa_dump((s), (l))
# define APMSEC_TKNDUMP(t)		sec_tkn_dump((t))
#endif

#if !defined(APM_SEC_QMDEBUG)
# define APMSEC_QMSGDUMP(hdr, d, l)
#else
# define APMSEC_QMSGDUMP(hdr, d, l)	\
	do { \
		print_hex_dump(KERN_INFO, APMSEC_HDR hdr, \
			DUMP_PREFIX_ADDRESS, 16, 4,  d, l, 1); \
	} while(0);
#endif

#define APM_SEC_GLBL_CTRL_CSR_OFFSET	0x0000
#define APM_XTS_AXI_CSR_OFFSET		0x1000
#define APM_XTS_CSR_OFFSET		0x1800
#define APM_XTS_CORE_CSR_OFFSET		0x2000
#define APM_EIP96_AXI_CSR_OFFSET	0x2800
#define APM_EIP96_CSR_OFFSET		0x3000
#define APM_EIP96_CORE_CSR_OFFSET	0x3800
#define	APM_EIP62_AXI_CSR_OFFSET	0x4000
#define APM_EIP62_CSR_OFFSET		0x4800
#define APM_EIP62_CORE_CSR_OFFSET	0x5000
#define	APM_QMI_CTL_OFFSET		0x9000
#define APM_SEC_CLK_RES_CSR_OFFSET	0xC000
#define APM_SEC_GLBL_DIAG_OFFSET	0XD000
#define	APM_SEC_AXI_SLAVE_SHIM_OFFSET	0xE000
#define APM_SEC_AXI_MASTER_SHIM_OFFSET  0xF000

#define SEC_GLB_CTRL_CSR_BLOCK		1
#define XTS_AXI_CSR_BLOCK		2
#define XTS_CSR_BLOCK			3
#define XTS_CORE_CSR_BLOCK		4
#define EIP96_AXI_CSR_BLOCK		5
#define EIP96_CSR_BLOCK			6
#define EIP96_CORE_CSR_BLOCK		7
#define EIP62_AXI_CSR_BLOCK		8
#define EIP62_CSR_BLOCK			9
#define EIP62_CORE_CSR_BLOCK		10
#define	QMI_CTL_BLOCK			11
#define	CLK_RES_CSR_BLOCK		12
#define AXI_SLAVE_SHIM_BLOCK		13
#define AXI_MASTER_SHIM_BLOCK		14

#define NUM_FREEPOOL			8
#define MAX_SLOT			1024
/* Total number of extra linked buffer */
#define APM_SEC_SRC_LINK_ADDR_MAX	(255-4)
#define APM_SEC_DST_LINK_ADDR_MAX	(255)
#define APM_SEC_TKN_CACHE_MAX		(MAX_SLOT / 4)
#define APM_SEC_SA_CACHE_MAX		(MAX_SLOT / 4)

struct xgene_sec_qm_queues {
	struct xgene_qmtm_qinfo fpq[NUM_FREEPOOL];	/* Free pool */
	struct xgene_qmtm_qinfo txq;	/* EIP96 work queue */
	struct xgene_qmtm_qinfo cpq;	/* EIP96 completion quueue */
	atomic_t active;
};

struct xgene_sec_ctx {
	struct device *dev;
	struct list_head alg_list;
	struct crypto_queue queue;
	spinlock_t lock;
	spinlock_t txlock;
	struct tasklet_struct tasklet;

	int irq;
	void __iomem *csr;
	void __iomem *ctrl_csr;
	void __iomem *clk_csr;
	void __iomem *diag_csr;
	void __iomem *eip96_axi_csr;
	void __iomem *eip96_csr;
	void __iomem *eip96_core_csr;
	void __iomem *qmi_ctl_csr;

	struct xgene_sec_qm_queues qm_queue;

	/* User IO variables */
	struct uio_info uioinfo;
	unsigned long flags;
	void *tknsa_array;
	void *buf_array;
};

struct xgene_sec_session_ctx {
	struct xgene_sec_ctx *ctx;
	struct sec_sa_item *sa;	/* Allocate outbound SA */
	struct sec_sa_item *sa_ib;	/* Allocate inbound SA if needed */

	spinlock_t lock;
	struct list_head tkn_cache;
	u32 tkn_cache_cnt;
	u16 tkn_max_len;
	u16 tkn_input_len;
	struct list_head sa_cache;
	u32 sa_cache_cnt;
	u16 sa_len;
	u16 sa_max_len;

#if 0
	/* FIXME */
	int sa_flush_done;
	u16 pad_block_size;	/* For ESP offload */
	u16 encap_uhl;		/* For ESP offload - ENCAP UDP header length */
#endif
};

extern struct xgene_sec_ctx *xg_ctx;

int xgene_sec_init_memram(struct xgene_sec_ctx *ctx);
int xgene_sec_hwreset(struct xgene_sec_ctx *ctx);
int xgene_sec_hwinit(struct xgene_sec_ctx *ctx);
int xgene_sec_hwstart(struct xgene_sec_ctx *ctx);
int xgene_sec_hwstop(struct xgene_sec_ctx *ctx);
int xgene_sec_qconfig(struct xgene_sec_ctx *ctx);
void xgene_sec_intr_hdlr(struct xgene_sec_ctx *ctx);
void xgene_sec_hdlr_qerr(struct xgene_sec_ctx *ctx, int qm_err_hop, int qm_err);

void xgene_sec_wr32(struct xgene_sec_ctx *ctx, u8 block, u32 reg, u32 data);
void xgene_sec_rd32(struct xgene_sec_ctx *ctx, u8 block, u32 reg, u32 * data);

int xgene_sec_create_sa_tkn_pool(struct xgene_sec_session_ctx *session,
				 u32 sa_max_len, u32 sa_len,
				 char sa_ib, u32 tkn_len);
void xgene_sec_free_sa_tkn_pool(struct xgene_sec_session_ctx *session);
struct sec_tkn_ctx *xgene_sec_tkn_get(struct xgene_sec_session_ctx *session,
				      u8 * new_tkn);
void xgene_sec_tkn_free(struct xgene_sec_session_ctx *session,
			struct sec_tkn_ctx *tkn);
struct sec_sa_item *xgene_sec_sa_get(struct xgene_sec_session_ctx *session);
void xgene_sec_sa_free(struct xgene_sec_session_ctx *session,
		       struct sec_sa_item *sa);

void xgene_sec_session_init(struct xgene_sec_session_ctx *session);
void xgene_sec_session_free(struct xgene_sec_session_ctx *session);

int xgene_sec_setup_crypto(struct xgene_sec_ctx *ctx,
			   struct crypto_async_request *req);
int xgene_sec_queue2hw(struct xgene_sec_session_ctx *session,
		       struct sec_tkn_ctx *tkn);
int xgene_sec_loadbuffer2qmsg(struct xgene_sec_ctx *ctx,
			      struct xgene_qmtm_msg32 *msg,
			      struct xgene_qmtm_msg_ext32 *qmsgext32,
			      struct sec_tkn_ctx *tkn);

void xgene_sec_qmesg_load_dst_single(struct xgene_sec_ctx *ctx,
				     struct xgene_qmtm_msg32 *msg, void *ptr,
				     int nbytes);
u64 xgene_sec_encode2hwaddr(u64 hwaddr);
u64 xgene_sec_decode2hwaddr(u64 hwaddr);

int xgene_sec_uio_init(struct platform_device *pdev, struct xgene_sec_ctx *ctx);
int xgene_sec_uio_deinit(struct xgene_sec_ctx *ctx);

#if 0
struct sec_tkn_ctx *apm_sec_rmsg2tkn(struct apm_sec_msg_result *rmsg);

#define APM_SEC_QMSG_CACHE_MAX		256	/* FIXME - Parameter needs tuning */

/* Errors reported in the Completion message */
#define	ERR_CRYPTO_DES_LINK_LIST	0x1
#define	ERR_CRYPTO			0x2
#define	ERR_DES_LINK_LIST_ENOUGH	0x0
#define	ERR_FREEPOOL_RUN_OUT	0x3
#define	ERR_READ_AXI		0x4
#define	ERR_WRITE_AXI		0x5
#define	ERR_INVALID_MSG		0X7
#define	ERR_DES_LISK_LIST_READ	0x6

struct apm_sec_msg_in {
	struct xgene_qmtm_msg64 msg64;
	union apm_sec_addr_list addr_list;
	struct apm_sec_msg_address src_addr_link[APM_SEC_LINK_ADDR_MAX];
	struct apm_sec_msg_address dst_addr_link[APM_SEC_LINK_ADDR_MAX];
	struct list_head next;
};

#define APM_SEC_MSG_HDR_SIZE	(sizeof(struct apm_sec_msg1_2) + \
				 sizeof(struct apm_sec_msg_3) + \
				 sizeof(struct apm_sec_msg_4))
#define APM_SEC_MSG_SIZE	(APM_SEC_MSG_HDR_SIZE + 32)

void apm_sec_err_log(int lerr, int elerr);

struct xgene_sec_session_ctx *apm_sec_session_create(void);
void apm_lsec_sg2buf(struct scatterlist *src, int nbytes, char *saddr);
int apm_lsec_sg_scattered(unsigned int nbytes, struct scatterlist *sg);

/* Total number of extra linked buffer */
#define APM_SEC_LINK_ADDR_MAX		16

/* Complete message structure */
struct apm_sec_msg {
	struct apm_sec_msg1_2 msg1_2;
	struct apm_sec_msg_4 msg4;
	struct apm_sec_msg_3 msg3;
	union apm_sec_addr_list addr_list;
	struct apm_sec_msg_address src_addr_link[APM_SEC_LINK_ADDR_MAX];
	struct apm_sec_msg_address dst_addr_link[APM_SEC_LINK_ADDR_MAX];
	struct list_head next;
};
#endif

#endif
