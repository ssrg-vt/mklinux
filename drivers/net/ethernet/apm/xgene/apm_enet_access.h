/**
 * AppliedMicro X-Gene SoC Ethernet Driver
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * All rights reserved. Keyur Chudgar <kchudgar@amcc.com>
 *                      Ravi Patel <rapatel@apm.com>
 *                      Iyappan Subramanian <isubramanian@apm.com>
 *                      Fushen Chen <fchen@apm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * @file apm_enet_access.h
 *
 * This file defines access layer for X-Gene SoC Ethernet driver 
 *
 */

#ifndef __APM_ENET_ACCESS_H__
#define __APM_ENET_ACCESS_H__

#include <linux/clk.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <net/ip.h>
#include <linux/tcp.h>
#include <linux/interrupt.h>
#include <linux/if_vlan.h>
#include <linux/phy.h>
#include <linux/acpi.h>
#include <linux/efi.h>
#include <linux/netdevice.h>
#include <misc/xgene/qmtm/xgene_qmtm.h>
#include "apm_enet_common.h"
#include "apm_enet_tools.h"
#ifdef CONFIG_ARCH_MSLIM
#include <asm/hardware/mslim-iof-map.h>
#endif

#define APM_ENET_DRIVER_NAME "apm_enet"
#define APM_ENET_DRIVER_VERSION "1.0"
/*
 *  Driver Debug Modes
 */

/* Define to enable ENET driver error reporting */
#define ENET_DBG_ERR
/* Define to enable ENET driver RX error reporting */
#undef ENET_DBGRX_ERR
/* Define to enable ENET driver basic reporting */
#define ENET_PRINT_ENABLE
/* Enable for configuration debugging */
#undef ENET_DBG
/* Define to enable queue info debugging */
#undef ENET_Q_DBG
/* Define to enable PHY debugging log */
#undef PHY_DEBUG
/* Define to enable RX log */
#undef ENET_DBGRX
/* Define to enable TX log */
#undef ENET_DBGTX
/* Define to enable RX/TX general log */
#undef ENET_DBGRXTX
/* Define to enable extra error checking */
#define ENET_CHK
/* Define to dump RX/TX packet */
#undef ENET_DUMP_PKT
/* Define to dump QM message */
#undef ENET_DBG_QMSG
/* Define to enable TSO debugging */
#undef ENET_DBG_TSO 
/* Define to enable HW buffer pool  debugging */
#undef HW_POOL_DBG
/* Define to enable inline secutiry debugging */
#undef ENET_DBG_SEC
/* Define to enable ENET CSR read debugging */
#undef ENET_DBG_RD
/* Define to enable ENET CSR write debugging */
#undef ENET_DBG_WR
/* Define to enable ENET CSR read printing */
#undef ENET_REGISTER_READ
/* Define to enable ENET CSR write printing */
#undef ENET_REGISTER_WRITE
/* Define to enable ENET Offload messages */
#undef ENET_DBG_OFFLOAD
/* Define to debug LRO */
#undef ENET_DBG_LRO

/*
 *   Driver operating Modes
 */
#undef SMP_LOAD_BALANCE
/* Define to enable IPv4 TX check sum offload */
#define IPV4_TX_CHKSUM
/* Define to enable IPv4 RX check sum offload */
#define IPV4_RX_CHKSUM
/* Define to enable TCP segmentation offload */
/* require IPV4_TX_CHKSUM & IPV4_RX_CHKSUM */
#define IPV4_TSO
/* Define to enable Generic Receive Offload */
#define IPV4_GRO
/* Define to enable Generic Segmentation Offload */
#undef IPV4_GSO
#if defined(CONFIG_XGENE_CLE) || defined(CONFIG_XGENE_CLE_MODULE)
#define XGENE_NET_CLE
/* Define to enable set_rx_mode functionalty */
#define SET_RX_MODE
#ifndef SET_RX_MODE
#define SET_XGR_MODE
#if defined(SET_XGR_MODE)
#define CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
#endif
#endif
#endif

/* Define to enable ENET Link IRQ */
#if defined(CONFIG_APM862xx)
#undef ENET_LINK_IRQ
#else
#undef ENET_LINK_IRQ
#endif

/* Define to enable Ethernet Error interrupt support */
#define INT_SUPPORT
/* Define to enable Ethernet Error handler */
#undef INT_ENABLE
/* Define to enable buffer pool */
#undef CONFIG_DRIVER_POOL
#define CONFIG_NAPI

#undef CONFIG_PAGE_POOL

#undef CONFIG_NET_PROTO

#ifndef CONFIG_PLAT_STORM_VHP
#define PBN_CLEAR_SUPPORT
#endif

#ifdef ENET_DUMP_PKT
#define ENET_DUMP(m, b, l)	print_hex_dump(KERN_INFO, m, \
				DUMP_PREFIX_ADDRESS, 16, 4, b, l, 1); 
#else					
#define ENET_DUMP(m, b, l)
#endif

#ifdef ENET_DBG_QMSG
#define ENET_QMSG(m, b, l)	print_hex_dump(KERN_INFO, m, \
				DUMP_PREFIX_ADDRESS, 16, 4, b, l, 1); 
#else
#define ENET_QMSG(m, b, l)
#endif

#ifdef ENET_PRINT_ENABLE
#define ENET_PRINT(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#define ENET_ERROR(x, ...)	printk(KERN_ERR x, ##__VA_ARGS__)
#else
#define ENET_PRINT(x, ...)
#define ENET_ERROR(x, ...)
#endif

#ifdef PHY_DEBUG
#define PHY_PRINT(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define PHY_PRINT(x, ...)
#endif

#ifdef ENET_Q_DBG
#define ENET_DBG_Q(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define ENET_DBG_Q(x, ...)
#endif

#ifdef APM_ENET_BUF_DEBUG
#define BUFPRINT(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define BUFPRINT(x, ...)
#endif

#ifdef ENET_DBG_TSO
#define ENET_DEBUG_TSO(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define ENET_DEBUG_TSO(x, ...)
#endif

#ifdef ENET_DBG_SEC
#define ENET_DEBUG_SEC(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define ENET_DEBUG_SEC(x, ...)
#endif

#ifdef HW_POOL_DBG
#define DEBG_HW_POOL(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define DEBG_HW_POOL(x, ...)
#endif

#ifdef ENET_DBG
#define ENET_DEBUG(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define ENET_DEBUG(x, ...)
#endif

#ifdef ENET_DBGTX
#define ENET_DEBUGTX(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define ENET_DEBUGTX(x, ...)
#endif

#ifdef ENET_DBGRX
#define ENET_DEBUGRX(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define ENET_DEBUGRX(x, ...)
#endif

#ifdef ENET_DBGRXTX
#define ENET_DEBUGRXTX(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define ENET_DEBUGRXTX(x, ...)
#endif

#ifdef ENET_DBG_ERR
#define ENET_DEBUG_ERR(x, ...)	printk(KERN_ERR x, ##__VA_ARGS__)
#else
#define ENET_DEBUG_ERR(x, ...)
#endif

#ifdef ENET_DBG_RD
#define ENET_DEBUG_RD(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define ENET_DEBUG_RD(x, ...)
#endif

#ifdef ENET_DBG_WR
#define ENET_DEBUG_WR(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define ENET_DEBUG_WR(x, ...)
#endif

#ifdef ENET_REGISTER_READ
#define ENET_REG_RD(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define ENET_REG_RD(x, ...)
#endif

#ifdef ENET_REGISTER_WRITE
#define ENET_REG_WR(x, ...)	printk(KERN_INFO x, ##__VA_ARGS__)
#else
#define ENET_REG_WR(x, ...)
#endif

#ifdef ENET_DBG_OFFLOAD
#define ENET_DEBUG_OFFLOAD(x, ...)	printk(x, ##__VA_ARGS__)
#define ENET_ERROR_OFFLOAD(x, ...)	printk(KERN_ERR x, ##__VA_ARGS__)
#else
#define ENET_DEBUG_OFFLOAD(x, ...)
#define ENET_ERROR_OFFLOAD(x, ...)
#endif

#if defined(ENET_DBG_LRO)
#define ENET_DEBUG_LRO(x, ...)		printk(x, ##__VA_ARGS__)
#else
#define ENET_DEBUG_LRO(x, ...)
#endif

/* Note: PKT_BUF_SIZE & PKT_NXTBUF_SIZE has to be one of the following:
 * 256, 1K, 2K, 4K, 16K for ethernet to work with optimum performance.
 */
#define APM_ENET_NXT_BUFFER_SIZE	4096
#define APM_ENET_SKB_MALLOC_FRAG	2048

/* SKB reserve size and buffer size */
#define APM_ENET_SKB_RESERVED		(L1_CACHE_BYTES + SKB_DATA_ALIGN(sizeof(struct skb_shared_info)))
#define APM_ENET_SKB_MALLOC_SIZE	(APM_ENET_SKB_MALLOC_FRAG - APM_ENET_SKB_RESERVED)
#define APM_ENET_SKB_BUFFER_SIZE	(APM_ENET_SKB_MALLOC_SIZE - NET_IP_ALIGN)

/* Frame Length */
#define APM_ENET_MIN_MTU		64
#define APM_ENET_STD_MTU		1536
#define APM_ENET_JMB_MTU		(APM_ENET_STD_MTU + 8192)
#ifdef CONFIG_XGENE_NET_JUMBO_FRAME
#define APM_ENET_MAX_MTU		APM_ENET_JMB_MTU
#else
#define APM_ENET_MAX_MTU		APM_ENET_STD_MTU
#endif
#define APM_ENET_FRAME_LEN		((APM_ENET_MAX_MTU > APM_ENET_SKB_BUFFER_SIZE) ? APM_ENET_MAX_MTU : APM_ENET_SKB_BUFFER_SIZE)

/* MAC + Type + VLAN + CRC */
#define HW_MTU(m) ((m) + 12 + 2 + 4 + 4)

#define MAX_DEVICE_NAME_SIZE		20
/* NAPI parameters */
#define APM_ENET_NAPI_WEIGHT		16
#define APM_ENET_MAX_FRAG		259	/* 4 + 255 LL Buffer */

#define APM_NO_PKT_BUF		256	  /* Default buffer count unless
					     override via dts */
#define APM_HW_PKT_BUF		64
#define IPP_PKT_BUF_SIZE	256

#define RES_SIZE(r)		((r)->end - (r)->start + 1)

#define FREE_SKB(skb) dev_kfree_skb_any(skb) 

/* Hardware capability and feature flags */
#define FLAG_RX_CSUM_ENABLED 	(1 << 0)

/* For EEE CSR untill header file is fixed */
#define EEE_REG_0                                                    0x00000800
#define EEE_TW_TIMER_0                                               0x00000804
#define EEE_LPI_WAIT_PATTERNS_0                                      0x00000808
#define EEE_REG_1                                                    0x00001000
#define EEE_TW_TIMER_1                                               0x00001004
#define EEE_LPI_WAIT_PATTERNS_1                                      0x00001008
#define EEE_REG_CFG_LPI_MODE                                         0x80000000
#define EEE_REG_CFG_LPI_CLK_STOPPABLE                                0x40000000

/* define Enet system struct */
struct apm_enet_dev {
	int refcnt;		
	u8 apm_preclass_init_done[MAX_ENET_PORTS];
	struct timer_list link_poll_timer;
	int ipp_loaded;
	int ipp_hw_mtu;
};

/* netmap global call back routines function signature */
typedef void (*netmap_fn_ptr)(void *data);

extern netmap_fn_ptr netmap_open;
extern netmap_fn_ptr netmap_close;

#define APM_ENET_PM_FLAG_WOL		0x0001
#define APM_ENET_PM_FLAG_CLKGATED	0x0002
#define APM_ENET_PM_FLAG_PWROFF		0x0004
#define APM_ENET_PM_FLAG_PENDINGCLKGATE	0x0008
#define APM_ENET_PM_FLAG_PENDINGCLKEN	0x0010

/* Max limit for Unicast and Multicast MAC Address filtering */
#define APM_MAX_UC_MC_MACADDR 16

struct umcast_entry {
        struct  list_head       list;
        unsigned char           addr[6];
        /* Flag type */
#define MARK_DEL		1
#define MARK_PRESENT		2
        int                     flag;
};

#ifdef CONFIG_ARCH_XGENE
enum apm_enet_laser_switch {
	LASER_OFF,
	LASER_ON
};

#define SPFF_I2C_OUT0_ADDR   0x26 /* XG0, XG1 laser control */
#define SPFF_I2C_OUT1_ADDR   0x27 /* XG2, XG3 */
enum apm_enet_laser_addr {
	IN_OFFSET = 0,
	OUT_OFFSET = 1,
	CONFIG_OFFSET = 3
};
#endif

enum apm_enet_phy_poll_interval {
	PHY_POLL_LINK_ON = HZ,
	PHY_POLL_LINK_OFF = (HZ / 5)
};

enum apm_enet_debug_cmd {
	APM_ENET_READ_CMD,
	APM_ENET_WRITE_CMD,
	APM_ENET_MAX_CMD
};

#define MAX_TX_QUEUES 4
#define MAX_RX_QUEUES 4
#define RX_BUFFER_POOL_QSIZE	QSIZE_16KB
#define JUMBO_PAGE_POOL_QSIZE	QSIZE_64KB
#define TX_COMPLETION_QSIZE	QSIZE_64KB
#define SHARED_COMPLETION_QSIZE	QSIZE_512KB
#define TX_COMPLETION_INTERRUPT
#if defined(TX_COMPLETION_INTERRUPT)
#undef SINGLE_COMPLETIONQ
#define PER_TX_COMPLETIONQ
#if !defined(PER_TX_COMPLETIONQ) && !defined(SINGLE_COMPLETIONQ)
#define SHARED_RX_COMPLETIONQ
#else
#define TX_COMPLETIONQ
#endif
#else
#define TX_COMPLETION_POLLING
#endif

/* This is soft flow context of QM */
struct apm_enet_qcontext {
	struct apm_enet_pdev *pdev;
	volatile u32 *command;
	struct xgene_qmtm_msg16 *msg16;
	struct xgene_qmtm_msg32 *msg32;
	struct xgene_qmtm_msg_ext8 *msg8;
	volatile u32 *nummsgs;
	unsigned int index;
	unsigned int count;
	unsigned int queue_index;
	unsigned int eqnum;
	unsigned int irq;
	unsigned int c2e_count;
#ifdef CONFIG_ARCH_MSLIM
	unsigned int enq_pbn;	
	unsigned int deq_pbn;
	unsigned int comp_fixup; /* In the begining, missed completion. */
#endif
	struct sk_buff * (*skb);
	union {
		struct apm_enet_qcontext *c2e;
		struct apm_enet_qcontext *e2c;
	};
	struct apm_enet_qcontext *c2e_skb;
	struct apm_enet_qcontext *c2e_page;
	struct napi_struct napi;
	char irq_name[16];
	unsigned long bytes, packets;
	unsigned int irq_enabled;
#ifdef CONFIG_NET_RX_BUSY_POLL
	unsigned int state;
#define XGENET_STATE_IDLE        0
#define XGENET_STATE_NAPI        1    /* NAPI owns this QV */
#define XGENET_STATE_POLL        2    /* poll owns this QV */
#define XGENET_LOCKED (XGENET_STATE_NAPI | XGENET_STATE_POLL)
#define XGENET_STATE_NAPI_YIELD  4    /* NAPI yielded this QV */
#define XGENET_STATE_POLL_YIELD  8    /* poll yielded this QV */
#define XGENET_YIELD (XGENET_STATE_NAPI_YIELD | XGENET_STATE_POLL_YIELD)
#define XGENET_USER_PEND (XGENET_STATE_POLL | XGENET_STATE_POLL_YIELD)
	spinlock_t lock;
#endif  /* CONFIG_NET_RX_BUSY_POLL */
};

enum apm_enet_frame {
	APM_ENET_REGULAR_FRAME = APM_ENET_SKB_BUFFER_SIZE,
	APM_ENET_JUMBO_FRAME = APM_ENET_NXT_BUFFER_SIZE
};

/* Queues related parameters per Enet port */
#define ENET_MAX_PBN	8
#define ENET_MAX_QSEL	8

struct eth_wqids {
	u16 qtype;
	u16 qid;
	u16 arb; 
	u16 qcount;
	u16 qsel[ENET_MAX_QSEL];
};

struct eth_fqids {
	u16 qid;
	u16 pbn;
};

struct eth_queue_ids {
	u16 default_tx_qid;
	u16 tx_count;
	u16 tx_idx;
	struct eth_wqids tx[ENET_MAX_PBN];
	u16 default_rx_qid;
	u16 rx_count;
	u16 rx_idx;
	struct eth_wqids rx[ENET_MAX_PBN];
	u16 default_rx_fp_qid;
	u16 default_rx_fp_pbn;
	struct eth_fqids rx_fp[ENET_MAX_PBN];
	u16 default_rx_nxtfp_qid;
	u16 default_rx_nxtfp_pbn;
	struct eth_fqids rx_nxtfp[ENET_MAX_PBN];
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
	struct eth_wqids hw[ENET_MAX_PBN];
	struct eth_fqids hw_fp[ENET_MAX_PBN];
#endif
	struct eth_wqids comp[ENET_MAX_PBN];
	u16 default_comp_qid;
	u32 qm_ip;
};

/* APM ethernet per port data */
struct apm_enet_pdev {
	struct net_device *ndev;
	struct mii_bus *mdio_bus;
	struct phy_device *phy_dev;
	struct clk *clk;
	struct device_node *node;
	struct platform_device *platform_device;
	struct xgene_qmtm_sdev *sdev;
	struct apm_enet_qcontext *tx[MAX_TX_QUEUES];
	struct apm_enet_qcontext *rx_skb_pool[MAX_RX_QUEUES];
#ifdef CONFIG_XGENE_NET_JUMBO_FRAME
	struct apm_enet_qcontext *rx_page_pool[MAX_RX_QUEUES];
#endif
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
	struct apm_enet_qcontext *hw[MAX_TX_QUEUES];
	struct apm_enet_qcontext *hw_skb_pool[MAX_RX_QUEUES];
	u64 hw_index;
#endif
	struct apm_enet_qcontext *rx[MAX_RX_QUEUES];
	struct apm_enet_qcontext * (*tx_completion);
	u32 queue_init;
	u32 num_rx_queues;
	u32 num_tx_queues;
	u32 num_tx_completion_queues;
	int phy_link;
	int phy_speed;
	struct net_device_stats nstats;
	struct eth_detailed_stats stats;
	char *dev_name;
	u32 features, flags;
	int uc_count;
	struct eth_queue_ids qm_queues;     
	u32 rx_buff_cnt, tx_cqt_low, tx_cqt_hi;
	int ipg;	/* non-zero require set */
	int mss;	/* TSO mss */
	u32 vlan_tci;	/* VLAN Tag */
	u8 pb_enabled;
	u32 wka_flag;
	struct list_head mcast_head;
	struct list_head ucast_head;
#ifdef ENET_LINK_IRQ
	u32 link_status; 
#endif
	int opened;
	struct mutex		link_lock;
	struct mutex		phy_lock;
	struct delayed_work	link_work;
	unsigned int enet_err_irq;
	unsigned int enet_mac_err_irq;
	unsigned int enet_qmi_err_irq;
	unsigned int hw_config;
	struct apm_enet_priv priv;
	struct i2c_adapter *i2c_adap;
};

/* Ethernet raw register write routine */
void apm_enet_wr32(void *addr, u32 data);

/* Ethernet raw register read routine */
void apm_enet_rd32(void *addr, u32 *data);

/*
 * @brief   This function returns the port if for given device name.
 * @return  Port Id. 
 *
 */
unsigned int apm_enet_find_port(char *device);

/*
 * @brief   This function returns net_device for given port.
 * @return  struct net_device.
 *
 */
struct net_device *apm_enet_find_netdev(int port_id);

/*
 * @brief   This function returns eth_queue_ids for given port and core
 * @return  struct eth_queue_ids
 *
 */
struct eth_queue_ids *find_ethqids(int port_id, int core_id);

/*
 * @brief   This function returns net device stats for given device.
 * @param   *ndev - Pointer to network device structure
 * @return  Net device stats. 
 *
 */
struct net_device_stats *apm_enet_stats(struct net_device *ndev);

/*
 * @brief   This function returns the port if for given device name.
 * @return  Port Id. 
 *
 */
static inline struct apm_enet_priv *get_priv(void *dev_handle)
{
	struct net_device *ndev = (struct net_device *) dev_handle;
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	return &pdev->priv;
}

u32 apm_enet_get_port(struct apm_enet_pdev *pdev);

/*
 * @brief   This function implements IOCTL interface for Ethernet driver.
 * @param   *ndev - Pointer to network device structure
 * @param   *rq - Pointer to interface request structure
 * @param   cmd - IOCTL command. 
 * @return  0 - success or -1 - failure
 *
 */
int apm_enet_ioctl(struct net_device *ndev, struct ifreq *rq, int cmd);

int send_snmp_param(void);
int send_netbios_param(void);
int apm_enet_is_smp(void);

#ifdef CONFIG_NET_RX_BUSY_POLL
static inline void xgenet_init_lock(struct apm_enet_qcontext *e2c)
{

	spin_lock_init(&e2c->lock);
	e2c->state = XGENET_STATE_IDLE;
}

/* called from the device poll routine to get ownership of a e2c */
static inline bool xgenet_lock_napi(struct apm_enet_qcontext *e2c)
{
	int rc = true;
	spin_lock(&e2c->lock);
	if (e2c->state & XGENET_LOCKED) {
		WARN_ON(e2c->state & XGENET_STATE_NAPI);
		e2c->state |= XGENET_STATE_NAPI_YIELD;
		rc = false;
	} else
		/* we don't care if someone yielded */
		e2c->state = XGENET_STATE_NAPI;
	spin_unlock(&e2c->lock);
	return rc;
}

/* returns true is someone tried to get the qv while napi had it */
static inline bool xgenet_unlock_napi(struct apm_enet_qcontext *e2c)
{
	int rc = false;
	spin_lock(&e2c->lock);
	WARN_ON(e2c->state & (XGENET_STATE_POLL |
				XGENET_STATE_NAPI_YIELD));

	if (e2c->state & XGENET_STATE_POLL_YIELD)
		rc = true;
	e2c->state = XGENET_STATE_IDLE;
	spin_unlock(&e2c->lock);
	return rc;
}

/* called from xgenet_low_latency_poll() */
static inline bool xgenet_lock_poll(struct apm_enet_qcontext *e2c)
{
	int rc = true;
	spin_lock_bh(&e2c->lock);
	if ((e2c->state & XGENET_LOCKED)) {
		e2c->state |= XGENET_STATE_POLL_YIELD;
		rc = false;
	} else
		/* preserve yield marks */
		e2c->state |= XGENET_STATE_POLL;
	spin_unlock_bh(&e2c->lock);
	return rc;
}

/* returns true if someone tried to get the qv while it was locked */
static inline bool xgenet_unlock_poll(struct apm_enet_qcontext *e2c)
{
	int rc = false;
	spin_lock_bh(&e2c->lock);
	WARN_ON(e2c->state & (XGENET_STATE_NAPI));

	if (e2c->state & XGENET_STATE_POLL_YIELD)
		rc = true;

	e2c->state = XGENET_STATE_IDLE;
	spin_unlock_bh(&e2c->lock);
	return rc;
}
#endif /* CONFIG_NET_RX_BUSY_POLL */
#endif /* __APM_ENET_ACCESS_H__ */
