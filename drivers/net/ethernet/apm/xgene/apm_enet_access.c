/**
 * AppliedMicro X-Gene SoC Ethernet Driver
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * All rights reserved. Ravi Patel <rapatel@apm.com>
 *                      Iyappan Subramanian <isubramanian@apm.com>
 *                      Fushen Chen <fchen@apm.com>
 *                      Keyur Chudgar <kchudgar@apm.com>
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
 * @file apm_enet_access.c
 *
 * This file implements driver for X-Gene SoC Ethernet subsystem
 *
 */

#include <linux/io.h>
#include <linux/of_net.h>
#include <linux/i2c.h>
#include <net/busy_poll.h>
#include <misc/xgene/cle/apm_preclass_data.h>
#include "apm_cle_cfg.h"
#include "apm_enet_csr.h"
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
#include "apm_enet_ifo.h"
#endif
#include "apm_xgenet_mac.h"
#include "apm_enet_mac.h"
#include "apm_enet_misc.h"

netmap_fn_ptr netmap_open;
netmap_fn_ptr netmap_close;

static int apm_enet_rx_frame(struct apm_enet_qcontext *e2c,
		struct xgene_qmtm_msg32 *msg32_1,
		struct xgene_qmtm_msg_ext32 *msg32_2);
static int apm_enet_tx_completion(struct apm_enet_qcontext *e2c,
		struct xgene_qmtm_msg32 *msg32_1,
		struct xgene_qmtm_msg_ext32 *msg32_2);

static u32 irq_refcnt;
/* Global pdev structure */
struct apm_enet_pdev *enet_dev[MAX_ENET_PORTS];

/* Ethernet raw register write routine */
inline void apm_enet_wr32(void *addr, u32 data)
{
	ENET_REG_WR("Write addr 0x%p data 0x%08X\n", addr, data);
	writel(data, (void __iomem *) addr);
}

/* Ethernet raw register read routine */
inline void apm_enet_rd32(void *addr, u32 *data)
{
	ENET_REG_RD("Read addr 0x%p ", addr);
	*data = readl((void __iomem *) addr);
	ENET_REG_RD("data 0x%08X\n", *data);
}

inline u32 apm_enet_get_port(struct apm_enet_pdev *pdev)
{
	return pdev->priv.port;
}

/* Set data address into msg */
inline void apm_enet_set_skb_data(struct xgene_qmtm_msg16 *msg16,
		struct sk_buff *skb)
{
#ifdef CONFIG_ARCH_MSLIM
	u64 pa = mslim_pa_to_iof_axi(virt_to_phys(skb->data));
#else
	u64 pa = __pa(skb->data);
#endif
	u32 *word = &msg16->DataAddrL;

	*word++ = cpu_to_le32((u32)pa);
	*word = (*word & ~cpu_to_le32(0x3FF)) | cpu_to_le32((u32)(pa >> 32));
}

/* Set page address into msg */
inline void apm_enet_set_page(struct xgene_qmtm_msg16 *msg16,
		struct page *page)
{
#ifdef CONFIG_ARCH_MSLIM
	u64 pa = mslim_pa_to_iof_axi(page_to_phys(page));
#else
	u64 pa = page_to_phys(page);
#endif
	u32 *word = &msg16->DataAddrL;

	*word++ = cpu_to_le32((u32)pa);
	*word = (*word & ~cpu_to_le32(0x3FF)) | cpu_to_le32((u32)(pa >> 32));
}

/* Get physical page address from msg */
inline unsigned long apm_enet_get_page(struct xgene_qmtm_msg16 *msg16)
{
	u64 pa;
	u32 *word = &msg16->DataAddrL;

	pa = le32_to_cpup(word++);
	pa |= (u64)(le32_to_cpup(word) & 0x3FF) << 32;
#ifdef CONFIG_ARCH_MSLIM
	return (unsigned long)__va(mslim_iof_axi_to_pa(pa));
#else
	return (unsigned long)__va(pa);
#endif
}

inline void apm_mslim_invalidate_cache(struct apm_enet_pdev *pdev, void *addr,
	       	size_t size, enum dma_data_direction dir)
{
#ifdef CONFIG_ARCH_MSLIM
	dma_addr_t data_map;
	data_map = dma_map_single(&(pdev->platform_device->dev),
		       	addr, size, dir);
	dma_unmap_single(&(pdev->platform_device->dev), data_map, size, dir);
#endif
}

static void apm_enet_init_fp(struct apm_enet_qcontext *c2e, enum apm_enet_frame frame)
{
	struct xgene_qmtm_msg16 *msg16;
	u32 i;

	/* Initializing common fields */
	for (i = 0; i < c2e->count; i++) {
		msg16 = &c2e->msg16[i];
		memset(msg16, 0, sizeof(struct xgene_qmtm_msg16));
		msg16->UserInfo = i;
		msg16->C = xgene_qmtm_coherent();
		msg16->BufDataLen = xgene_qmtm_encode_bufdatalen(frame);
		msg16->FPQNum = c2e->eqnum;
		/* L3 stashing */
		msg16->PB = 0;
		msg16->HB = 1;
		xgene_qmtm_msg_le32(&(((u32 *)msg16)[1]), 3);
	}
}

static int apm_enet_fill_fp(struct apm_enet_qcontext *c2e, u32 nbuf,
	       	enum apm_enet_frame frame)
{
	register u32 index = c2e->index;
	register u32 count = c2e->count - 1;
	struct apm_enet_pdev *pdev = c2e->pdev;
	u32 i;

	for (i = 0; i < nbuf; i++) {
		struct sk_buff *skb;
		struct page *page;
		struct xgene_qmtm_msg16 *msg16 = &c2e->msg16[index];

		switch (frame) {
		case APM_ENET_REGULAR_FRAME:
			skb = dev_alloc_skb(APM_ENET_SKB_MALLOC_SIZE);
			if (unlikely(!skb)) {
				ENET_ERROR("Failed to allocate new skb size %ld",
						APM_ENET_SKB_MALLOC_SIZE);
				return -ENOMEM;
			}
			skb_reserve(skb, NET_IP_ALIGN);
			c2e->skb[index] = skb;
			apm_enet_set_skb_data(msg16, skb);
			break;
		case APM_ENET_JUMBO_FRAME:
			page = alloc_page(GFP_ATOMIC);
			if (unlikely(!page)) {
				ENET_ERROR("Failed to allocate new page");
				return -ENOMEM;
			}
			apm_enet_set_page(msg16, page);
			break;
		}

		index = (index + 1) & count;
		apm_mslim_invalidate_cache(pdev, skb->data, frame,
			       	DMA_FROM_DEVICE);
		apm_mslim_invalidate_cache(pdev, msg16, sizeof(msg16),
			       	DMA_TO_DEVICE);
	}

	writel(nbuf, c2e->command);
	c2e->index = index;

	return APM_RC_OK;
}

static void apm_enet_void_fp(struct apm_enet_qcontext *c2e, int qid,
	       	enum apm_enet_frame frame)
{
	u32 index = c2e->index;
	u32 count = c2e->count - 1;
	u32 command = 0;
	u32 prefetched = 0;
	struct apm_enet_pdev *pdev = c2e->pdev;
	struct xgene_qmtm_msg16 *msg16;
	struct xgene_qmtm_qinfo qinfo;
	int i;

	memset(&qinfo, 0, sizeof(qinfo));
	qinfo.qmtm_ip = pdev->sdev->qmtm_ip;
	qinfo.queue_id = qid;
	if (xgene_qmtm_get_qinfo(&qinfo))
		return;

	prefetched = NUM_MSGS_IN_BUF_RD(qinfo.pbm_state);
	for (i = 0; i < qinfo.nummsgs + prefetched; i++) {
		index = (index - 1) & count;
		msg16 = &c2e->msg16[index];

		switch (frame) {
		case APM_ENET_REGULAR_FRAME:
			kfree_skb(c2e->skb[msg16->UserInfo]);
			c2e->skb[msg16->UserInfo] = NULL;
			break;
		case APM_ENET_JUMBO_FRAME:
			free_page(apm_enet_get_page(msg16));
			break;
		}

		command--;
	}

	command += prefetched;
	writel(command, c2e->command);
	apm_enet_clr_pb(&pdev->priv, pdev->sdev->qmtm_ip, qid);
}

static int apm_enet_mdio_read(struct mii_bus *bus, int mii_id, int regnum)
{
	struct apm_enet_pdev *pdev = bus->priv;
	struct apm_enet_priv *priv = &pdev->priv;
	u32 regval1;
	u32 regval2;
	u32 regval3;

	mutex_lock(&enet_dev[pdev->hw_config]->phy_lock);
mdio_read_again:
	apm_genericmiiphy_read(priv, mii_id, regnum, &regval1);
	if (pdev->wka_flag & 0x01) {
		/* Work around to MDIO errata (MDIO hold issue) */
		apm_genericmiiphy_read(priv, mii_id, regnum, &regval2);
		if (regval1 != regval2) {
			apm_genericmiiphy_read(priv, mii_id, regnum, &regval3);
			if (regval2 != regval3) {
				schedule();
				goto mdio_read_again;
			}
			regval1 = regval2;
		}
	}
	mutex_unlock(&enet_dev[pdev->hw_config]->phy_lock);
	PHY_PRINT("%s: bus=%d reg=%d val=%x\n", __func__, mii_id,
			regnum, regval1);
	return (int)regval1;
}

static int apm_enet_mdio_write(struct mii_bus *bus, int mii_id, int regnum,
			   u16 regval)
{
	struct apm_enet_pdev *pdev = bus->priv;
	struct apm_enet_priv *priv = &pdev->priv;

	PHY_PRINT("%s: bus=%d reg=%d val=%x\n", __func__, mii_id,
			regnum, regval);
	mutex_lock(&enet_dev[pdev->hw_config]->phy_lock);
	apm_genericmiiphy_write(priv, mii_id, regnum, regval);
	mutex_unlock(&enet_dev[pdev->hw_config]->phy_lock);
	return 0;
}

static int apm_enet_mdio_reset(struct mii_bus *bus)
{
	return 0;
}

static void rgmii_speed_set(struct apm_enet_pdev *pdev, u32 speed)
{
	struct clk *parent = clk_get_parent(pdev->clk);

#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT)) {
		struct acpi_buffer buf = {ACPI_ALLOCATE_BUFFER, NULL};
		struct acpi_object_list input;
		input.count = 0;
		input.pointer = NULL;

		switch (speed) {
		case APM_ENET_SPEED_1000:
			acpi_evaluate_object(ACPI_HANDLE(&pdev->platform_device->dev),
									"S1G", &input, &buf);
			break;
		case APM_ENET_SPEED_100:
			acpi_evaluate_object(ACPI_HANDLE(&pdev->platform_device->dev),
									"S100", &input, &buf);
			break;
		case APM_ENET_SPEED_10:
			acpi_evaluate_object(ACPI_HANDLE(&pdev->platform_device->dev),
									"S10", &input, &buf);
			break;
		}
		return;
	}
#endif
	switch (speed) {
	case APM_ENET_SPEED_1000:
		clk_set_rate(parent, 125000000);
		break;
	case APM_ENET_SPEED_100:
		clk_set_rate(parent, 25000000);
		break;
	case APM_ENET_SPEED_10:
		clk_set_rate(parent, 2500000);
		break;
	}
}

static void apm_enet_mdio_link_change(struct net_device *ndev)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct apm_enet_priv *priv = &pdev->priv;
	struct phy_device *phydev = pdev->phy_dev;
	int status_change = 0;

	mutex_lock(&enet_dev[pdev->hw_config]->phy_lock);
	if (phydev->link) {
		if (pdev->phy_speed != phydev->speed) {
			if (priv->phy_mode == PHY_MODE_RGMII)
				rgmii_speed_set(pdev,phydev->speed);
			apm_enet_mac_init(priv, ndev->dev_addr, phydev->speed,
					priv->crc);
			pdev->phy_speed = phydev->speed;
			status_change = 1;
		}
	}

	if (phydev->link != pdev->phy_link) {
		if (!phydev->link)
			pdev->phy_speed = 0;
		pdev->phy_link = phydev->link;

		if ((priv->phy_mode == PHY_MODE_SGMII) && phydev->link) {
			apm_enet_get_link_status(priv);
			if (!priv->link_status)
				apm_enet_mac_init(priv, ndev->dev_addr,
					phydev->speed, priv->crc);
		}

		status_change = 1;
	}

	if (status_change) {
		apm_enet_mac_rx_state(priv, phydev->link);
		apm_enet_mac_tx_state(priv, phydev->link);
		if (phydev->link)
			ENET_PRINT("%s: link up %d Mbps duplex %d\n",
				       	ndev->name, phydev->speed,
					phydev->duplex);
		else
			ENET_PRINT("%s: link down\n", ndev->name);
	}
	mutex_unlock(&enet_dev[pdev->hw_config]->phy_lock);
}

#define RGMII_SGMII_SUPPORTED_MASK	(SUPPORTED_10baseT_Full | \
					SUPPORTED_100baseT_Full | \
					SUPPORTED_1000baseT_Full| \
					SUPPORTED_Autoneg | \
					SUPPORTED_TP | \
					SUPPORTED_MII)

static int apm_enet_mdio_probe(struct net_device *ndev)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct apm_enet_priv *priv = &pdev->priv;
	struct phy_device *phydev = NULL;
	int phy_addr;
	int rc = APM_RC_OK;

	/* find the first phy */
	for (phy_addr = 0; phy_addr < PHY_MAX_ADDR; phy_addr++) {
		if (pdev->mdio_bus->phy_map[phy_addr]) {
			phydev = pdev->mdio_bus->phy_map[phy_addr];
			break;
		}
	}

	if (!phydev) {
		ENET_PRINT("%s: no PHY found\n", ndev->name);
		rc = APM_RC_ERROR;
		goto out;
	}

	/* attach the mac to the phy */
	if (priv->phy_mode == PHY_MODE_SGMII) {
		phydev = phy_connect(ndev, dev_name(&phydev->dev),
			&apm_enet_mdio_link_change, PHY_INTERFACE_MODE_SGMII);
	} else {
		phydev = phy_connect(ndev, dev_name(&phydev->dev),
			&apm_enet_mdio_link_change, PHY_INTERFACE_MODE_RGMII);
	}

	pdev->phy_link = 0;
	pdev->phy_speed = 0;

	if (IS_ERR(phydev)) {
		pdev->phy_dev = NULL;
		ENET_PRINT("%s: Could not attach to PHY\n", ndev->name);
		return PTR_ERR(phydev);
	} else {
		pdev->phy_dev = phydev;
	}

	ndev->phydev->supported = RGMII_SGMII_SUPPORTED_MASK;
	ndev->phydev->advertising = RGMII_SGMII_SUPPORTED_MASK;

	ENET_PRINT("%s: phy_id=0x%08x phy_drv=\"%s\"\n",
		ndev->name, phydev->phy_id, phydev->drv->name);

out:
	return rc;
}

static int apm_enet_mdio_remove(struct net_device *ndev)
{
	struct apm_enet_pdev *pdev = (struct apm_enet_pdev *) netdev_priv(ndev);
	struct apm_enet_priv *priv = &pdev->priv;
	struct mii_bus *mdio_bus;

	if (priv->phy_mode == PHY_MODE_XGMII)
		goto out;

	mdio_bus = pdev->mdio_bus;

	mdiobus_unregister(mdio_bus);
	mdiobus_free(mdio_bus);

	pdev->mdio_bus = NULL;

out:
	return 0;
}

static inline u32 apm_enet_hdr_len(const void *data)
{
	const struct ethhdr *eth = data;
	return eth->h_proto == htons(ETH_P_8021Q) ? VLAN_ETH_HLEN : ETH_HLEN;
}

static int apm_enet_dequeue_msg(struct apm_enet_qcontext *e2c, int budget)
{
#ifndef CONFIG_ARCH_MSLIM
	register u32 command = 0;
	register u32 index = e2c->index;
	register u32 count = e2c->count - 1;
	register u32 count_tx = e2c->c2e ? e2c->c2e->count - 1 : 0;
#endif
	int napi_budget = budget;
#if defined(TX_COMPLETION_INTERRUPT)
/*	u32 bytes = 0, packets = 0;	*/
#if defined(SHARED_RX_COMPLETIONQ)
	struct net_device *ndev = e2c->pdev->ndev;
#endif
#endif
	do {
#ifdef CONFIG_ARCH_MSLIM
		struct xgene_qmtm_msg64 msg64;
		struct xgene_qmtm_msg_ext32 *p = NULL;

		if (msg64.msg32_1.msg16.FPQNum)
			apm_enet_rx_frame(e2c, &msg64.msg32_1, p);
		else
			apm_enet_tx_completion(e2c, &msg64.msg32_1, p);
#else
		struct xgene_qmtm_msg32 *msg32_1 = &e2c->msg32[index];
		struct xgene_qmtm_msg_ext32 *msg32_2 = NULL;

		if (unlikely(((u32 *)msg32_1)[EMPTY_SLOT_INDEX] == EMPTY_SLOT))
			break;

		xgene_qmtm_msg_le32(&(((u32 *)msg32_1)[1]), 7);
		if (msg32_1->msg16.NV) {
			index = (index + 1) & count;
			msg32_2 = (struct xgene_qmtm_msg_ext32 *)
				&e2c->msg32[index];
			if (unlikely(((u32 *)msg32_2)[EMPTY_SLOT_INDEX] ==
						EMPTY_SLOT)) {
				xgene_qmtm_msg_le32(&(((u32 *)msg32_1)[1]), 7);
				index = (index - 1) & count;
				break;
			}
			xgene_qmtm_msg_le32((u32 *)msg32_2, 8);
			command++;
		}
		index = (index + 1) & count;
		command++;

		if (msg32_1->msg16.FPQNum) {
			apm_enet_rx_frame(e2c, msg32_1, msg32_2);
		} else {
			apm_enet_tx_completion(e2c, msg32_1, msg32_2);
			e2c->eqnum = (e2c->eqnum + 1 + (!!msg32_2)) & count_tx;
#if defined(TX_COMPLETION_INTERRUPT)
/*			bytes += (msg32_1->msg16.UserInfo & 0xffffff);	*/
/*			packets += (msg32_1->msg16.UserInfo >> 24);	*/
#endif
		}

		((u32 *)msg32_1)[EMPTY_SLOT_INDEX] = EMPTY_SLOT;
		if (msg32_2)
			((u32 *)msg32_2)[EMPTY_SLOT_INDEX] = EMPTY_SLOT;
#endif
	} while (--budget);
#ifndef CONFIG_ARCH_MSLIM
	if (command) {
		while (((readl(e2c->nummsgs) & 0x1fffe) >> 1) < command);
		writel(-(int)command, e2c->command);
		e2c->index = index;
	}

#if defined(TX_COMPLETION_INTERRUPT)
#if defined(TX_COMPLETIONQ)
	if (e2c->c2e) {
/*	if (bytes) {	*/
		struct net_device *ndev = e2c->pdev->ndev;
#endif
/*		printk("%s:tx%d completed bytes %d packets %d\n", ndev->name, e2c->queue_index, bytes, packets);	*/
/*		netdev_tx_completed_queue(netdev_get_tx_queue(ndev, e2c->queue_index), packets, bytes);	*/
#if defined(SINGLE_COMPLETIONQ)
		for (index = 0; index < e2c->pdev->num_tx_queues; index++)
			if (__netif_subqueue_stopped(ndev, index) &&
					(((e2c->pdev->tx[index]->index -
					(e2c->eqnum / e2c->pdev->num_tx_queues)) &
					count_tx) < e2c->pdev->tx_cqt_low))
				netif_start_subqueue(ndev, index);
#else
		if (__netif_subqueue_stopped(ndev, e2c->queue_index) &&
				(((e2c->c2e->index - e2c->eqnum) &
				count_tx) < e2c->pdev->tx_cqt_low))
			netif_start_subqueue(ndev, e2c->queue_index);
#endif
#if defined(TX_COMPLETIONQ)
	}
#endif
#endif
#endif
	return napi_budget - budget;
}

#ifdef CONFIG_NAPI
static int apm_enet_napi(struct napi_struct *napi, const int budget)
{
	struct apm_enet_qcontext *e2c =
		container_of(napi, struct apm_enet_qcontext, napi);
#ifndef CONFIG_NET_RX_BUSY_POLL
	int processed = apm_enet_dequeue_msg(e2c, budget);
#else
	int processed;

	if (!xgenet_lock_napi(e2c))
		return budget;

	processed = apm_enet_dequeue_msg(e2c, budget);
	xgenet_unlock_napi(e2c);
#endif
	if (processed != budget) {
		napi_complete(napi);
		enable_irq(e2c->irq);
		e2c->irq_enabled = 1;
	}

	return processed;
}
#endif

#ifdef CONFIG_NET_RX_BUSY_POLL
/* must be called with local_bh_disable()d */
static int xgenet_low_latency_recv(struct napi_struct *napi)
{
	struct apm_enet_qcontext *e2c =
		container_of(napi, struct apm_enet_qcontext, napi);
	struct apm_enet_pdev *pdev = e2c->pdev;
	int found = 0;

	if (!pdev->phy_link)
		return LL_FLUSH_FAILED;

	if (!xgenet_lock_poll(e2c))
		return LL_FLUSH_BUSY;

	found = apm_enet_dequeue_msg(e2c, 4);

	xgenet_unlock_poll(e2c);

	return found;
}
#endif  /* CONFIG_NET_RX_BUSY_POLL */

irqreturn_t apm_enet_e2c_irq(const int irq, void *data)
{
	struct apm_enet_qcontext *e2c = (struct apm_enet_qcontext *)data;
	irqreturn_t rc = IRQ_HANDLED;

#ifdef CONFIG_NAPI
#ifdef CONFIG_ARCH_MSLIM1
	struct apm_enet_pdev *pdev = e2c->pdev;
	u32 wq_avail = get_wq_avail(pdev->sdev->qmtm_ip);

        if (!wq_avail)
                return IRQ_NONE;
#endif
	ENET_DEBUGRXTX("Interface %d in NAPI\n", apm_enet_get_port(e2c->pdev));

	/* Disable interrupt and schedule napi */
	if (napi_schedule_prep(&e2c->napi)) {
		ENET_DEBUGRXTX("Disable interrupt %d\n", irq);
		disable_irq_nosync(irq);
		e2c->irq_enabled = 0;
		__napi_schedule(&e2c->napi);
	}
#else
	rc = apm_enet_dequeue_msg(e2c, 1);
#endif

	return rc;
}

static int apm_enet_tx_completion(struct apm_enet_qcontext *e2c,
		struct xgene_qmtm_msg32 *msg32_1,
		struct xgene_qmtm_msg_ext32 *msg32_2)
{
	struct sk_buff *skb = (struct sk_buff *)__va(
			((u64)msg32_1->msgup16.H0Info_msbH << 32) |
			msg32_1->msgup16.H0Info_msbL);

	if (likely(skb)) {
		FREE_SKB(skb);
		return APM_RC_OK;
	} else {
		ENET_PRINT("completion skb is NULL\n");
		return APM_RC_ERROR;
	}
}

/*
static inline u16 apm_enet_select_queue(struct net_device *ndev, struct sk_buff *skb)
{
	return skb_tx_hash(ndev, skb);
}
*/

#ifdef SET_XGR_MODE
/* Packet transmit function */
static netdev_tx_t apm_enet_start_xmit(struct sk_buff *skb,
	       	struct net_device *ndev)
{
	FREE_SKB(skb);
	return NETDEV_TX_OK;
}
/* Process received frame */
static int apm_enet_rx_frame(struct apm_enet_qcontext *e2c,
		struct xgene_qmtm_msg32 *msg32_1,
		struct xgene_qmtm_msg_ext32 *msg32_2)
{
	struct xgene_qmtm_msg16 *msg16 = &msg32_1->msg16;
	u32 UserInfo = msg16->UserInfo;
	u32 datalen = xgene_qmtm_decode_datalen(msg16->BufDataLen) - 4;
	u32 *data = (u32 *)&e2c->c2e_skb->skb[UserInfo]->data[datalen - 4];
	struct apm_enet_qcontext *c2e = e2c->c2e;
	struct xgene_qmtm_msg16 *emsg16 =
		(struct xgene_qmtm_msg16 *)&c2e->msg32[c2e->index];
	struct xgene_qmtm_msg_up16 *emsgup16 =
		(struct xgene_qmtm_msg_up16 *)&emsg16[1];

	swab32s(data);
	memset(emsg16, 0, 32);
	emsg16->UserInfo = UserInfo;
	emsg16->C = xgene_qmtm_coherent();
	emsg16->BufDataLen = datalen;
	emsg16->FPQNum = msg16->FPQNum;
	emsg16->PB = 0;
	emsg16->HB = 1;
	emsg16->DataAddrH = msg16->DataAddrH;
	emsg16->DataAddrL = msg16->DataAddrL;
	emsgup16->HR = 1;
#define QMTM_MAX_QUEUE_ID 1023
	emsgup16->H0Enq_Num = c2e->eqnum | QMTM_MAX_QUEUE_ID;
	emsgup16->H0Info_lsbH = (TYPE_SEL_WORK_MSG << 12) | (TSO_INS_CRC_ENABLE << 3);
	emsgup16->H0Info_lsbL = (0xe & TSO_ETH_HLEN_MASK) << 12;
	xgene_qmtm_msg_le32(&(((u32 *)emsg16)[1]), 7);

	/* Forward the packet */
	writel(1, c2e->command);
	if (++c2e->index == c2e->count)
		c2e->index = 0;

	return APM_RC_OK;
}
#else
/* VLAN tag insertion offload processing */
inline void apm_enet_vlan_tag(struct net_device *ndev, struct sk_buff *skb,
		       	struct xgene_qmtm_msg_up16 *msg_up16)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);

	if (unlikely(!(ndev->features & NETIF_F_HW_VLAN_CTAG_TX)) ||
			unlikely(!(skb->vlan_tci))) {
		return;
	}

	/* Set InsertVLAN bit */
	msg_up16->H0Info_lsbH |= (1 << 2);
	if (skb->vlan_tci != pdev->vlan_tci) {
		apm_enet_tx_offload( &pdev->priv, APM_ENET_INSERT_VLAN,
				((htons(ETH_P_8021Q) << 16)
				 | htons(skb->vlan_tci)));
		pdev->vlan_tci = skb->vlan_tci;
	}
}

/* Checksum offload processing */
static int apm_enet_checksum_offload(struct net_device *ndev,
	       	struct sk_buff *skb,
	       	struct xgene_qmtm_msg_up16 *msg_up16)

{
	u32 maclen, nr_frags, ihl;
	struct iphdr *iph;
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	u32 packets = 1, bytes = skb->len;

	if (unlikely(!(ndev->features & NETIF_F_IP_CSUM)))
		goto out;

	if (unlikely(skb->protocol != htons(ETH_P_IP)) &&
		unlikely(skb->protocol != htons(ETH_P_8021Q)))
		goto out;

	nr_frags = skb_shinfo(skb)->nr_frags;
	maclen = apm_enet_hdr_len(skb->data);
	iph = ip_hdr(skb);
	ihl = ip_hdrlen(skb) >> 2;

	if (unlikely(iph->frag_off & htons(IP_MF | IP_OFFSET)))
		goto out;

	if (likely(iph->protocol == IPPROTO_TCP)) {
		int xhlen, mss_len;
		u32 mss, all_hdr_len;

		xhlen = tcp_hdrlen(skb)/4;
		msg_up16->H0Info_lsbL |=
			(xhlen & TSO_TCP_HLEN_MASK) |
			((ihl & TSO_IP_HLEN_MASK) << 6) |
			(TSO_CHKSUM_ENABLE << 22) |
			(TSO_IPPROTO_TCP << 24);
		ENET_DEBUGTX("Checksum Offload H0Info 0x%04X%08X H1Info 0x%04X%08X\n",
				msg_up16->H0Info_lsbH, msg_up16->H0Info_lsbL,
				msg_up16->H0Info_msbH, msg_up16->H0Info_msbL);

		if (unlikely(!(ndev->features & NETIF_F_TSO)))
			goto out;

		/* TCP Segmentation offload processing */
		mss = skb_shinfo(skb)->gso_size;
		all_hdr_len = maclen + ip_hdrlen(skb) + tcp_hdrlen(skb);
		mss_len = skb->len - all_hdr_len;

		/* HW requires all header resides * in the first buffer */
		if (nr_frags && (skb_headlen(skb) < all_hdr_len)) {
			printk("Un-support header length "
					"location by Ethernet HW\n");
			pdev->stats.estats.tx_dropped++;
			dev_kfree_skb(skb);
			return APM_RC_ERROR;
		}

		if (!mss || mss_len <= mss)
			goto out;

		if (mss != pdev->mss) {
			apm_enet_tx_offload(&pdev->priv, APM_ENET_MSS0, mss);
			pdev->mss = mss;
		}

		msg_up16->H0Info_lsbL |= ((0 & TSO_MSS_MASK) << 20) |
			((TSO_ENABLE & TSO_ENABLE_MASK) << 23);
		ENET_DEBUG_TSO("TCP TSO H0Info 0x%04X%08X H1Info 0x%04X%08X mss %d\n",
				msg_up16->H0Info_lsbH, msg_up16->H0Info_lsbL,
				msg_up16->H0Info_msbH, msg_up16->H0Info_msbL, mss);
		packets += ((mss_len - 1) / mss);
		bytes += ((packets - 1) * all_hdr_len);
	} else if (iph->protocol == IPPROTO_UDP) {
		msg_up16->H0Info_lsbL |=
			(UDP_HDR_SIZE & TSO_TCP_HLEN_MASK) |
			((ihl & TSO_IP_HLEN_MASK) << 6) |
			(TSO_CHKSUM_ENABLE << 22) |
			(TSO_IPPROTO_UDP << 24);
		ENET_DEBUGTX("Checksum Offload H0Info 0x%04X%08X H1Info 0x%04X%08X\n",
				msg_up16->H0Info_lsbH, msg_up16->H0Info_lsbL,
				msg_up16->H0Info_msbH, msg_up16->H0Info_msbL);
	} else {
		msg_up16->H0Info_lsbL |= ((ihl & TSO_IP_HLEN_MASK) << 6);
	}
out:
	pdev->tx[skb->queue_mapping]->packets += packets;
	pdev->tx[skb->queue_mapping]->bytes += bytes;
#if defined(TX_COMPLETION_INTERRUPT)
/*	((struct xgene_qmtm_msg16 *)(--msg_up16))->UserInfo = (packets << 24) | bytes;	*/
/*	printk("%s:tx%d sent bytes %d packets %d\n", ndev->name, skb->queue_mapping, bytes, packets);	*/
/*	netdev_tx_sent_queue(netdev_get_tx_queue(ndev, skb->queue_mapping), bytes);	*/
#endif
	return 0;
}

inline phys_addr_t apm_enet_virt_to_phys(void *addr)
{
	phys_addr_t paddr;

#ifdef CONFIG_ARCH_MSLIM
	paddr = mslim_pa_to_iof_axi(virt_to_phys(paddr));
#else
	paddr = virt_to_phys(addr);
#endif
	return paddr;
}

/* Packet transmit function */
static netdev_tx_t apm_enet_start_xmit(struct sk_buff *skb,
	       	struct net_device *ndev)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct apm_enet_qcontext *c2e = pdev->tx[skb->queue_mapping];
	register u32 index = c2e->index;
	register u32 count = c2e->count - 1;
	register u32 command;
	struct xgene_qmtm_msg16 *msg16 = (struct xgene_qmtm_msg16 *)
						&c2e->msg32[index];
	struct xgene_qmtm_msg_up16 *msg_up16 = (struct xgene_qmtm_msg_up16 *)
						&msg16[1];
	struct xgene_qmtm_msg_ext32 *msg32_2 = NULL;
	phys_addr_t paddr;
	u32 nr_frags = skb_shinfo(skb)->nr_frags;
	struct xgene_qmtm_msg_ll8 *ext_msg_ll8;
	struct xgene_qmtm_msg_ext8 *ext_msg;
	u8 *wp, *vaddr = NULL;
	skb_frag_t *frag = NULL;
	int offset = 0, len = 0, frag_no = 0;
	int ell_bcnt = 0, ell_cnt = 0, i;

#if defined(SINGLE_COMPLETIONQ)
	if (((index - (c2e->e2c->eqnum / pdev->num_tx_queues)) & count) > pdev->tx_cqt_hi)
#else
	if (((index - c2e->e2c->eqnum) & count) > pdev->tx_cqt_hi)
#endif
	{
		netif_stop_subqueue(ndev, c2e->queue_index);
#if defined(TX_COMPLETION_POLLING)
		if (c2e->c2e_skb)
			apm_enet_dequeue_msg(c2e->c2e_skb, -1);
#endif
		return NETDEV_TX_BUSY;
	}

	paddr = apm_enet_virt_to_phys(skb->data);
	ENET_DEBUGTX("TX Frame PADDR: 0x%010llx VADDR: 0x%p len: %d frag %d\n",
		paddr, skb->data, skb->len, nr_frags);

	index = (index + 1) & count;
	memset(msg16, 0, sizeof(struct xgene_qmtm_msg32));

	/* Packet not fragmented */
	if (likely(nr_frags == 0)) {
		command = 1;

		/* Prepare QM message */
		skb->len = (skb->len < 60) ? 60 : skb->len;
		msg16->BufDataLen = xgene_qmtm_encode_datalen(skb->len);
		msg16->DataAddrL = (u32)paddr;
		msg16->DataAddrH = (u32)(paddr >> 32);
		ENET_DUMP("TX ", skb->data, skb->len);
	} else {
		msg32_2 = (struct xgene_qmtm_msg_ext32 *)&c2e->msg32[index];
		wp = (u8 *)msg32_2;

		index = (index + 1) & count;
		command = 2;
		memset(msg32_2, 0, sizeof(struct xgene_qmtm_msg_ext32));

		/* First Fragment, 64B message */
		msg16->BufDataLen = xgene_qmtm_encode_datalen(skb_headlen(skb));
		msg16->DataAddrL = (u32)paddr;
		msg16->DataAddrH = (u32)(paddr >> 32);
		msg16->NV = 1;

		/* 2nd, 3rd, and 4th fragments */
		ext_msg = &msg32_2->msg8_1;

		/* Terminate next pointers, will be updated later as required */
		msg32_2->msg8_2.NxtBufDataLength = 0x7800;
		msg32_2->msg8_3.NxtBufDataLength = 0x7800;
		msg32_2->msg8_4.NxtBufDataLength = 0x7800;

		for (i = 0; i < 3 && frag_no < nr_frags; i++) {
			if (!vaddr) {
				frag = &skb_shinfo(skb)->frags[frag_no];
				len = frag->size;
				vaddr = skb_frag_address(frag);
				offset = 0;
				ENET_DEBUGTX("SKB Frag[%d] 0x%p len %d\n",
					frag_no, vaddr, len);
			}
			paddr = apm_enet_virt_to_phys(vaddr + offset);
			ext_msg->NxtDataAddrL = (u32)paddr;
			ext_msg->NxtDataAddrH = (u32)(paddr >> 32);

			if (len <= 16*1024) {
				/* Encode using 16K buffer size format */
				ext_msg->NxtBufDataLength =
					xgene_qmtm_encode_datalen(len);
				vaddr = NULL;
				frag_no++;
			} else {
				len -= 16*1024;
				offset += 16*1024;
				/* Encode using 16K buffer size format */
				ext_msg->NxtBufDataLength = 0;
			}

			ENET_DEBUGTX("Frag[%d] PADDR 0x%04X%08X len %d\n", i,
				ext_msg->NxtDataAddrH, ext_msg->NxtDataAddrL,
				ext_msg->NxtBufDataLength);
			ext_msg = (struct xgene_qmtm_msg_ext8 *)
				(wp + (8 * ((i + 1) ^ 1)));
		}
		/* Determine no more fragment, last one, or more than one */
		if (!vaddr) {
			/* Check next fragment */
			if (frag_no >= nr_frags) {
				goto no_more_frag;
			} else {
				frag = &skb_shinfo(skb)->frags[frag_no];
				if (frag->size <= 16*1024 &&
					(frag_no + 1) >= nr_frags)
					goto one_more_frag;
				else
					goto more_than_one_frag;
			}
		} else if (len <= 16*1024) {
			/* Current fragment <= 16K, check if last fragment */
			if ((frag_no + 1) >= nr_frags)
				goto one_more_frag;
			else
				goto more_than_one_frag;
		} else {
			/* Current fragment requires two pointers */
			goto more_than_one_frag;
		}

one_more_frag:
		if (!vaddr) {
			frag = &skb_shinfo(skb)->frags[frag_no];
			len = frag->size;
			vaddr = skb_frag_address(frag);
			offset = 0;
			ENET_DEBUGTX("SKB Frag[%d] 0x%p len %d\n",
				frag_no, vaddr, len);
		}

		paddr = apm_enet_virt_to_phys(vaddr + offset);
		ext_msg->NxtDataAddrL = (u32)paddr;
		ext_msg->NxtDataAddrH = (u32)(paddr >> 32);
		/* Encode using 16K buffer size format */
		ext_msg->NxtBufDataLength = xgene_qmtm_encode_datalen(len);
		ENET_DEBUGTX("Frag[%d] PADDR 0x%04X%08X len %d\n", i,
			ext_msg->NxtDataAddrH, ext_msg->NxtDataAddrL,
			ext_msg->NxtBufDataLength);
		goto no_more_frag;

more_than_one_frag:
		msg16->LL = 1;		/* Extended link list */
		ext_msg_ll8 = &msg32_2->msg8_ll;
		ext_msg = &c2e->msg8[index * 256];
		memset(ext_msg, 0, 256 * sizeof(struct xgene_qmtm_msg_ext8));
		paddr = apm_enet_virt_to_phys(ext_msg);
		ext_msg_ll8->NxtDataPtrL = (u32)paddr;
		ext_msg_ll8->NxtDataPtrH = (u32)(paddr >> 32);

		for (i = 0; i < 255 && frag_no < nr_frags; ) {
			if (vaddr == NULL) {
				frag = &skb_shinfo(skb)->frags[frag_no];
				len = frag->size;
				vaddr = skb_frag_address(frag);
				offset = 0;
				ENET_DEBUGTX("SKB Frag[%d] 0x%p len %d\n",
					frag_no, vaddr, len);
			}
			paddr = apm_enet_virt_to_phys(vaddr + offset);
			ext_msg[i ^ 1].NxtDataAddrL = (u32)paddr;
			ext_msg[i ^ 1].NxtDataAddrH = (u32)(paddr >> 32);

			if (len <= 16*1024) {
				/* Encode using 16K buffer size format */
				ext_msg[i ^ 1].NxtBufDataLength =
					xgene_qmtm_encode_datalen(len);
				ell_bcnt += len;
				vaddr = NULL;
				frag_no++;
			} else {
				len -= 16*1024;
				offset += 16*1024;
				ell_bcnt += 16*1024;
			}

			ell_cnt++;
			ENET_DEBUGTX("Frag ELL[%d] PADDR 0x%04X%08X len %d\n", i,
				ext_msg[i ^ 1].NxtDataAddrH, ext_msg[i ^ 1].NxtDataAddrL,
				ext_msg[i ^ 1].NxtBufDataLength);
			xgene_qmtm_msg_le32((u32 *)&ext_msg[i ^ 1], 2);
			i++;
		}
		/* Encode the extended link list byte count and link count */
		ext_msg_ll8->NxtLinkListength = ell_cnt;
		msg_up16->TotDataLengthLinkListLSBs = (ell_bcnt & 0xFFF);
		ext_msg_ll8->TotDataLengthLinkListMSBs =
			((ell_bcnt & 0xFF000) >> 12);
		ENET_QMSG("ELL msg ", &c2e->msg8[index * 256],
			       	8 * (((ell_cnt + 1) / 2) * 2));
	}
no_more_frag:
	if (msg32_2)
		xgene_qmtm_msg_le32((u32 *)msg32_2, 8);
	msg_up16->H0Info_msbL = (u32)__pa(skb);
	msg_up16->H0Info_msbH = (u32)(__pa(skb) >> 32);
	msg_up16->H0Enq_Num = c2e->eqnum;
	msg16->C = xgene_qmtm_coherent();

	/* Set TYPE_SEL for egress work message */
	msg_up16->H0Info_lsbH = TYPE_SEL_WORK_MSG << 12;

	/* Enable CRC insertion */
	if (!pdev->priv.crc)
		msg_up16->H0Info_lsbH |= (TSO_INS_CRC_ENABLE << 3); /* Set InsertCRC bit */

	/* Setup mac header length H0Info */
	msg_up16->H0Info_lsbL |=
	       	((apm_enet_hdr_len(skb->data) & TSO_ETH_HLEN_MASK) << 12);

	if (unlikely(apm_enet_checksum_offload(ndev, skb, msg_up16)))
		return NETDEV_TX_OK;

	/* xmit: Push the work message to ENET HW */
	ENET_DEBUGRXTX("TX CQID %d Addr 0x%04x%08x len %d\n",
		msg_up16->H0Enq_Num, msg16->DataAddrH, msg16->DataAddrL,
		msg16->BufDataLen);
#ifdef ENET_DBG_QMSG
	ENET_QMSG("TX msg ", msg16, 32);
	if (msg32_2)
		ENET_QMSG("TX msg ", msg32_2, 32);
#endif
	apm_mslim_invalidate_cache(pdev, skb->data, skb->len, DMA_TO_DEVICE);
	apm_mslim_invalidate_cache(pdev, msg16, 32 * command, DMA_TO_DEVICE);
#undef FREE_CLONED_SKB
#ifdef FREE_CLONED_SKB
	if (likely(skb_cloned(skb))) {
		msg_up16->H0Enq_Num = QMTM_QUEUE_ID(pdev->sdev->qmtm_ip,
			QM_RSV_UNCONFIG_COMP_Q);
		FREE_SKB(skb);
	}
#endif
	xgene_qmtm_msg_le32(&(((u32 *)msg16)[1]), 7);
	writel(command, c2e->command);
	c2e->index = index;

#if defined(TX_COMPLETION_POLLING)
	if (c2e->c2e_skb) {
		index -= c2e->c2e_skb->index;
		if (index > 4)
			apm_enet_dequeue_msg(c2e->c2e_skb, index);
	}
#endif

	netdev_get_tx_queue(ndev, skb->queue_mapping)->trans_start = jiffies;
	return NETDEV_TX_OK;
}

/* Process received frame */
static int apm_enet_rx_frame(struct apm_enet_qcontext *e2c,
		struct xgene_qmtm_msg32 *msg32_1,
		struct xgene_qmtm_msg_ext32 *msg32_2)
{
	struct apm_enet_qcontext *c2e;
	struct apm_enet_pdev *pdev = e2c->pdev;
	struct xgene_qmtm_msg16 *msg16 = &msg32_1->msg16;
	struct sk_buff *skb = NULL;
	u32 data_len = xgene_qmtm_decode_datalen(msg16->BufDataLen);
	u8 NV = msg16->NV;
	u8 LErr = ((u8) msg16->ELErr << 3) | msg16->LErr;
	u32 UserInfo = msg16->UserInfo;
	u32 qid = pdev->qm_queues.rx[e2c->queue_index].qid;
#ifdef ENET_DBGRXTX
	u64 DataAddr = ((u64)msg16->DataAddrH << 32) | msg16->DataAddrL;
#endif
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
	struct xgene_qmtm_msg_up16 *msgup16 = &msg32_1->msgup16;

	if (msgup16->H0FPSel)
		c2e = pdev->hw_skb_pool[e2c->queue_index];
	else
#endif
		c2e = e2c->c2e_skb;

	ENET_DEBUGRXTX("RX frame QID %d Rtype %d\n", qid, msg16->RType);

	if (unlikely(UserInfo >= c2e->count)) {
		ENET_DEBUG_ERR("ENET: invalid UserInfo %d QID %d FP 0x%x\n",
			UserInfo, qid, msg16->FPQNum);

		print_hex_dump(KERN_INFO, "QM msg:",
			DUMP_PREFIX_ADDRESS, 16, 4, msg32_1, NV ? 64 : 32, 1);
		goto err_refill;
	}

	skb = c2e->skb[UserInfo];
	c2e->skb[UserInfo] = NULL;
	if (unlikely(skb == NULL)) {
		ENET_DEBUG_ERR("ENET skb NULL UserInfo %d QID %d FP 0x%x\n",
			UserInfo, qid, msg16->FPQNum);
		print_hex_dump(KERN_INFO, "QM msg:",
			DUMP_PREFIX_ADDRESS, 16, 4, msg32_1, NV ? 64 : 32, 1);
		goto err_refill;
	}

	if (unlikely(skb->head == NULL) || unlikely(skb->data == NULL)) {
		ENET_DEBUG_ERR("ENET skb 0x%p head 0x%p data 0x%p FP 0x%x\n",
			skb, skb->head, skb->data, msg16->FPQNum);
		print_hex_dump(KERN_INFO, "QM msg:",
			DUMP_PREFIX_ADDRESS, 16, 4, msg32_1, NV ? 64 : 32, 1);
		goto err_refill;
	}

	if (unlikely(skb->len)) {
		ENET_DEBUG_ERR("ENET skb 0x%p len %d FP 0x%x\n",
			skb, skb->len, msg16->FPQNum);
		print_hex_dump(KERN_INFO, "QM msg:",
			DUMP_PREFIX_ADDRESS, 16, 4, msg32_1, NV ? 64 : 32, 1);
		goto err_refill;
	}

#ifndef XGENE_NET_CLE
	if (likely(LErr == 0x11)) {
		LErr = 0;
		goto process_pkt;
	}
#endif
	/* Check for error, if packet received with error */
	if (unlikely(LErr)) {
		//XXX: have enums for LErr codes
		if ((LErr == 0x10) || (LErr == 5) || (LErr == 0x15)) {
			LErr = 0;
			goto process_pkt;
		}

		apm_enet_parse_error(LErr, 1, qid);
		ENET_DEBUG_ERR("ENET LErr 0x%x skb 0x%p FP 0x%x\n",
			LErr, skb, msg16->FPQNum);
		print_hex_dump(KERN_ERR, "QM Msg: ",
			DUMP_PREFIX_ADDRESS, 16, 4, msg32_1, NV ? 64 : 32, 1);
		goto err_refill;
	}

process_pkt:
	/* prefetch data in cache */
	prefetch(skb->data - NET_IP_ALIGN);
	ENET_DEBUGRXTX("RX port %d SKB data VADDR: 0x%p PADDR: 0x%llX\n",
			apm_enet_get_port(pdev), skb->data, DataAddr);

	if (likely(!NV)) {
		/* Strip off CRC as HW isn't doing this */
		data_len -= 4;
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
		/* Check if the offload is enabled. If yes perform it */
		if (msgup16->H0FPSel) {
			apm_enet_ifo_perform(msg32_1, skb,
				enet_dev[msgup16->H0Enq_Num]->hw[e2c->queue_index]);
			/* Increment the rx bytes */
			e2c->bytes += data_len;
			/* Increment the rx packet count */
			++e2c->packets;
			return APM_RC_OK;
		}
#endif
		skb_put(skb, data_len);
		ENET_DEBUGRX("RX port %d SKB len %d\n",
				apm_enet_get_port(pdev), data_len);
#ifdef CONFIG_XGENE_NET_JUMBO_FRAME
	} else {
		struct apm_enet_qcontext *c2e_page = e2c->c2e_page;
		struct xgene_qmtm_msg_ext8 *ext_msg;
		int i;
		/* Multiple fragment */
		skb_put(skb, data_len);
		ENET_DEBUGRX("RX port %d SKB multiple len %d\n",
				apm_enet_get_port(pdev), data_len);

		/* Handle fragments */
		ext_msg = (struct xgene_qmtm_msg_ext8 *) msg32_2;
		for (i = 0; i < 4; i++) {
			struct page *page;
			void *data;
			int frag_size;

			if (!xgene_qmtm_nxtbufdatalen_is_valid(ext_msg[i ^ 1].NxtBufDataLength))
				break;

			data = PHYS_TO_VIRT(((u64)ext_msg[i ^ 1].NxtDataAddrH << 32) |
					ext_msg[i ^ 1].NxtDataAddrL);
			page = virt_to_page(data);
			if (!page) {
				kfree_skb(skb);
				skb = NULL;
				goto err_refill;
			}
			frag_size = xgene_qmtm_decode_datalen(
						ext_msg[i ^ 1].NxtBufDataLength);
			if (i >= 3 || !xgene_qmtm_nxtbufdatalen_is_valid(ext_msg[i ^ 1].NxtBufDataLength)) {
				/* Strip off CRC as HW does not handle this */
				frag_size -= 4;
			}
			skb_add_rx_frag(skb, i, page, 0, frag_size, frag_size);
			apm_enet_fill_fp(c2e_page, 1, APM_ENET_JUMBO_FRAME);
		}
#endif
	}
	ENET_DUMP("RX ", (u8 *) skb->data, data_len);

	if (--e2c->c2e_count == 0) {
		apm_enet_fill_fp(c2e, 32, APM_ENET_REGULAR_FRAME);
		e2c->c2e_count = 32;
	}

	/* Increment the rx bytes */
	e2c->bytes += skb->len;
	/* Increment the rx packet count */
	++e2c->packets;

	if (pdev->num_rx_queues > 1)
		skb_record_rx_queue(skb, e2c->queue_index);
	skb->protocol = eth_type_trans(skb, pdev->ndev);

#if defined(IPV4_RX_CHKSUM) || defined(IPV4_TSO)
	if (likely(pdev->features & FLAG_RX_CSUM_ENABLED) &&
	    likely(LErr == 0) &&
	    likely(skb->protocol == htons(ETH_P_IP))) {
		struct iphdr *iph = (struct iphdr *) skb->data;
		if (likely(!(iph->frag_off & htons(IP_MF | IP_OFFSET))) ||
			likely(iph->protocol != IPPROTO_TCP &&
			iph->protocol != IPPROTO_UDP)) {
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		}
	}
#endif
	skb_mark_napi_id(skb, &e2c->napi);
#ifdef IPV4_GRO
	napi_gro_receive(&e2c->napi, skb);
#else
	netif_receive_skb(skb);
#endif
	return APM_RC_OK;

err_refill:
	if (skb != NULL)
		FREE_SKB(skb);

	apm_enet_fill_fp(e2c->c2e_skb, 1, APM_ENET_REGULAR_FRAME);

	if (LErr != 0x15)
		pdev->stats.estats.rx_hw_errors++;
	else
		pdev->stats.estats.rx_hw_overrun++;

	return APM_RC_ERROR;
}
#endif

static void apm_enet_timeout(struct net_device *ndev)
{
	unsigned int i;
	struct apm_enet_pdev *pdev = netdev_priv(ndev);

	apm_enet_mac_reset(&pdev->priv);
	for (i = 0; i < ndev->num_tx_queues; i++) {
		struct netdev_queue *txq = netdev_get_tx_queue(ndev, i);

		txq->trans_start = jiffies;
		netif_tx_wake_queue(txq);
	}
}

static void apm_enet_napi_add_all(struct apm_enet_pdev *pdev)
{
#ifdef CONFIG_NAPI
	u32 qindex;

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++)
		netif_napi_add(pdev->ndev, &pdev->rx[qindex]->napi, apm_enet_napi, 64);
#if defined(TX_COMPLETION_INTERRUPT)
	for (qindex = 0; qindex < pdev->num_tx_completion_queues; qindex++)
		netif_napi_add(pdev->ndev, &pdev->tx_completion[qindex]->napi, apm_enet_napi, 64);
#endif
#endif
}

static void apm_enet_napi_del_all(struct apm_enet_pdev *pdev)
{
#ifdef CONFIG_NAPI
	u32 qindex;

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++)
		netif_napi_del(&pdev->rx[qindex]->napi);
#if defined(TX_COMPLETION_INTERRUPT)
	for (qindex = 0; qindex < pdev->num_tx_completion_queues; qindex++)
		netif_napi_del(&pdev->tx_completion[qindex]->napi);
#endif
#endif
}

static void apm_enet_napi_enable_all(struct apm_enet_pdev *pdev)
{
#ifdef CONFIG_NAPI
	u32 qindex;

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
		napi_hash_add(&pdev->rx[qindex]->napi);
		napi_enable(&pdev->rx[qindex]->napi);
	}
#if defined(TX_COMPLETION_INTERRUPT)
	for (qindex = 0; qindex < pdev->num_tx_completion_queues; qindex++) {
		napi_hash_add(&pdev->tx_completion[qindex]->napi);
		napi_enable(&pdev->tx_completion[qindex]->napi);
	}
#endif
#endif
}

static void apm_enet_napi_disable_all(struct apm_enet_pdev *pdev)
{
#ifdef CONFIG_NAPI
	u32 qindex;

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
		napi_disable(&pdev->rx[qindex]->napi);
		napi_hash_del(&pdev->rx[qindex]->napi);
	}
#if defined(TX_COMPLETION_INTERRUPT)
	for (qindex = 0; qindex < pdev->num_tx_completion_queues; qindex++) {
		napi_disable(&pdev->tx_completion[qindex]->napi);
		napi_hash_del(&pdev->tx_completion[qindex]->napi);
	}
#endif
#endif
}

static void apm_enet_irq_enable_all(struct apm_enet_pdev *pdev)
{
	u32 qindex;

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
		if (!pdev->rx[qindex]->irq_enabled) {
			enable_irq(pdev->rx[qindex]->irq);
			pdev->rx[qindex]->irq_enabled = 1;
		}
	}
#if defined(TX_COMPLETION_INTERRUPT)
	for (qindex = 0; qindex < pdev->num_tx_completion_queues; qindex++) {
		if (!pdev->tx_completion[qindex]->irq_enabled) {
			enable_irq(pdev->tx_completion[qindex]->irq);
			pdev->tx_completion[qindex]->irq_enabled = 1;
		}
	}
#endif
}

static void apm_enet_irq_disable_all(struct apm_enet_pdev *pdev)
{
	u32 qindex;

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
		if (pdev->rx[qindex]->irq_enabled) {
			disable_irq_nosync(pdev->rx[qindex]->irq);
			pdev->rx[qindex]->irq_enabled = 0;
		}
	}
#if defined(TX_COMPLETION_INTERRUPT)
	for (qindex = 0; qindex < pdev->num_tx_completion_queues; qindex++) {
		if (pdev->tx_completion[qindex]->irq_enabled) {
			disable_irq_nosync(pdev->tx_completion[qindex]->irq);
			pdev->tx_completion[qindex]->irq_enabled = 0;
		}
	}
#endif
}

#ifdef CONFIG_ARCH_XGENE
static int apm_enet_cfg_xfi_laser(struct apm_enet_pdev *pdev, int port)
{
	union i2c_smbus_data i2c_data;
	u32 chip;
	u32 addr;
	u32 retries;
	int rc;

	if (!pdev->i2c_adap)
		return 0;

	/* Configure the MUX to channel 1 */
	chip = 0x71;
	addr = 0x1;

	/* Check if the MUX actually existed. If not, assume not needed. */
	rc = i2c_smbus_xfer(pdev->i2c_adap, chip, 0, I2C_SMBUS_READ,
				addr, I2C_SMBUS_BYTE_DATA, &i2c_data);
	if (rc)
		return 0;

	/* Attempt to configure the MUX */
	for (retries = 0; retries < 3; retries++) {
		i2c_data.byte = 1;
		rc = i2c_smbus_xfer(pdev->i2c_adap, chip, 0,
				    I2C_SMBUS_WRITE, addr, I2C_SMBUS_BYTE_DATA,
				    &i2c_data);
		if (rc) {
			dev_err(&pdev->platform_device->dev,
				"XFI laser MUX write failed error %d\n", rc);
			return rc;
		}
		rc = i2c_smbus_xfer(pdev->i2c_adap, chip, 0, I2C_SMBUS_READ,
				addr, I2C_SMBUS_BYTE_DATA, &i2c_data);
		if (rc) {
			dev_err(&pdev->platform_device->dev,
				"XFI laser MUX read failed error %d\n", rc);
			return rc;
		}
		if (i2c_data.byte == 1)
			break;
	}

	if (i2c_data.byte != 1)
		return -ETIMEDOUT;

	msleep(500);	/* FIXME */

	/* Enable XFI laser */
	chip = SPFF_I2C_OUT0_ADDR;
	addr = 0x3;
	for (chip = 0x26; chip <= SPFF_I2C_OUT1_ADDR; chip++) {
		for (retries = 0; retries < 3; retries++) {
			i2c_data.byte = 0xee;
			rc = i2c_smbus_xfer(pdev->i2c_adap, chip, 0,
					    I2C_SMBUS_WRITE, addr, I2C_SMBUS_BYTE_DATA,
					    &i2c_data);
			if (rc) {
				dev_err(&pdev->platform_device->dev,
					"XFI enable write failed error %d\n", rc);
				return rc;
			}

			msleep(500); /* FIXME */

			rc = i2c_smbus_xfer(pdev->i2c_adap, chip, 0, I2C_SMBUS_READ,
					addr, I2C_SMBUS_BYTE_DATA, &i2c_data);
			if (rc) {
				dev_err(&pdev->platform_device->dev,
					"XFI enable read failed error %d\n", rc);
				return rc;
			}
			if (i2c_data.byte == 0xee)
				break;
		}
	}

	return 0;
}

static int apm_enet_xfi_laser_set(struct apm_enet_pdev *pdev, u32 port,
				  u32 state)
{
	static u32 sfp_addr[4] = { SPFF_I2C_OUT0_ADDR, SPFF_I2C_OUT0_ADDR,
        			   SPFF_I2C_OUT1_ADDR, SPFF_I2C_OUT1_ADDR };
	static u32 sfp_abs[4] = { 0x20, 0x02, 0x20, 0x02 };
	static u32 sfp_laser_on[4] = { 0xef, 0xfe, 0xef, 0xfe };
	static u32 sfp_laser_off[4] = { 0xff, 0xff, 0xff, 0xff };
	union i2c_smbus_data i2c_data;
	u32 i2c_val;
	int rc;
	u32 data;
	u32 retries;

	if (!pdev->i2c_adap)
		return 0;

	rc = i2c_smbus_xfer(pdev->i2c_adap, sfp_addr[port], 0, I2C_SMBUS_READ,
			    IN_OFFSET, I2C_SMBUS_BYTE_DATA, &i2c_data);
	if (rc) {
		if (rc != -ENODEV)
			dev_err(&pdev->platform_device->dev,
				"SFP laser read failed error %d\n", rc);
		return rc;
	}
	i2c_val = i2c_data.byte;
	dev_dbg(&pdev->platform_device->dev, "SFP laser 0x%08X\n", i2c_val);

	data = i2c_val & sfp_abs[port] ? sfp_laser_off[port] :
					 sfp_laser_on[port];
	if (i2c_val & sfp_abs[(port + 1) % 2])
		data &= sfp_laser_off[(port + 1) % 2];
	else
		data &= sfp_laser_on[(port + 1) % 2];

	/* Turn on/off laser */
	for (retries = 0; retries < 3; retries++) {
		i2c_data.byte = data;
		rc = i2c_smbus_xfer(pdev->i2c_adap, sfp_addr[port], 0,
				    I2C_SMBUS_WRITE, OUT_OFFSET,
				    I2C_SMBUS_BYTE_DATA, &i2c_data);
		if (rc) {
			dev_err(&pdev->platform_device->dev,
				"SFP laser write failed error %d\n", rc);
			goto out;
		}
		msleep(500);	/* FIXME - why wait? */

		rc = i2c_smbus_xfer(pdev->i2c_adap, sfp_addr[port], 0,
				    I2C_SMBUS_READ, IN_OFFSET, I2C_SMBUS_BYTE_DATA,
				    &i2c_data);
		if (rc) {
			dev_err(&pdev->platform_device->dev,
				"SFP laser read failed error %d\n", rc);
			goto out;
		}
		if (data == i2c_data.byte)
			break;
	}
out:
	return 0;
}
#endif

/* Will be called whenever the device will change state to 'Up' */
static int apm_enet_open(struct net_device *ndev)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct apm_enet_priv *priv = &pdev->priv;
	int qindex;

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
		/* Configure free pool */
		apm_enet_fill_fp(pdev->rx_skb_pool[qindex], pdev->rx_buff_cnt,
		       	APM_ENET_REGULAR_FRAME);
#ifdef CONFIG_XGENE_NET_JUMBO_FRAME
		/* Configure free pool */
		apm_enet_fill_fp(pdev->rx_page_pool[qindex],
			pdev->rx_buff_cnt * 4, APM_ENET_JUMBO_FRAME);
#endif
	}

	if (!pdev->queue_init) {
		if (pdev->num_tx_queues > 1 &&
			netif_set_real_num_tx_queues(ndev, pdev->num_tx_queues))
			ENET_ERROR("%s: TX Queue error\n", ndev->name);
		if (pdev->num_rx_queues > 1 &&
			netif_set_real_num_rx_queues(ndev, pdev->num_rx_queues))
			ENET_ERROR("%s: RX Queue error\n", ndev->name);
#ifdef SET_XGR_MODE
		if (priv->port == XGENET_0 || priv->port == XGENET_1) {
			struct apm_ptree_config *ptree_config;
			struct apm_cle_dbptr dbptr;
			u16 dstqid; 

			ptree_config = apm_find_ptree_config(priv->port, CLE_PTREE_DEFAULT);
			/* Re-program Result Pointer to pass through non-XGR traffic */
			memset(&dbptr, 0, sizeof(dbptr));
			dbptr.index = ptree_config->start_dbptr;
			dbptr.HR = 1;
			dbptr.H0Enq_Num = QMTM_QUEUE_ID(pdev->qm_queues.qm_ip,
							QMTM_MAX_QUEUE_ID);
			dstqid = QMTM_QUEUE_ID(pdev->qm_queues.qm_ip,
					       enet_dev[priv->port ^ 1]->qm_queues.hw[0].qid);
			dbptr.dstqidL = dstqid & 0x7f;
			dbptr.dstqidH = (dstqid >> 7) & 0x1f;
			dbptr.fpsel = pdev->qm_queues.hw_fp[0].pbn - 0x20;
			apm_set_cle_dbptr(priv->port, &dbptr);
			/* Re-program Result Pointer to allow XGR traffic */
			memset(&dbptr, 0, sizeof(dbptr));
			dbptr.index = ptree_config->start_dbptr + 1;
			dstqid = QMTM_QUEUE_ID(pdev->qm_queues.qm_ip,
					       pdev->qm_queues.rx[0].qid);
			dbptr.dstqidL = dstqid & 0x7f;
			dbptr.dstqidH = (dstqid >> 7) & 0x1f;
			dbptr.fpsel = pdev->qm_queues.rx_fp[0].pbn - 0x20;
			apm_set_cle_dbptr(priv->port, &dbptr);
			pdev->rx[0]->c2e = enet_dev[priv->port ^ 1]->tx[0];
		}
#endif
		pdev->queue_init = 1;
	}

	if (!pdev->mdio_bus)
		mutex_lock(&pdev->link_lock);
#ifdef CONFIG_ARCH_XGENE
	if (priv->phy_mode == PHY_MODE_XGMII)
		apm_enet_xfi_laser_set(pdev, priv->port - XGENET_0, LASER_ON);
#endif
	pdev->opened = 1;

	apm_enet_napi_enable_all(pdev);
	apm_enet_irq_enable_all(pdev);

	if (priv->phy_mode != PHY_MODE_SGMII)
		netif_carrier_on(ndev);

	/* start phy */
	if (pdev->phy_dev)
		phy_start(pdev->phy_dev);

	/* Enable TX/RX MAC */
	if (priv->phy_mode != PHY_MODE_SGMII) {
		apm_enet_mac_tx_state(priv, 1);
		apm_enet_mac_rx_state(priv, 1);
	}

	if (pdev->ipg > 0)
		apm_enet_mac_set_ipg(priv, pdev->ipg);

	if (!pdev->mdio_bus) {
		int link_poll_interval;

		if (apm_enet_get_link_status(priv)) {
			link_poll_interval = PHY_POLL_LINK_ON;
			ENET_PRINT("%s: link up %u Mbps\n",
					pdev->ndev->name, priv->speed);
			apm_enet_mac_init(priv, ndev->dev_addr, priv->speed,
					priv->crc);
			netif_carrier_on(ndev);
		} else {
			netif_carrier_off(ndev);
			link_poll_interval = PHY_POLL_LINK_OFF;
			ENET_PRINT("%s: link down\n", pdev->ndev->name);
		}
		apm_enet_mac_rx_state(priv, priv->link_status);
		apm_enet_mac_tx_state(priv, priv->link_status);
		schedule_delayed_work(&pdev->link_work, link_poll_interval);
		mutex_unlock(&pdev->link_lock);
	}

	/* Start xmit queue */
	netif_tx_start_all_queues(ndev);

	/* inform the netmap framework that interface is going up */
	if (netmap_open)
		netmap_open((void *)pdev);

	return APM_RC_OK;
}

static int apm_enet_close(struct net_device *ndev)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct apm_enet_priv *priv = &pdev->priv;
	u32 qindex;
	
	/* inform the netmap framework that interface is going down */
	if (netmap_close)
		netmap_close((void *)pdev);

	if (!pdev->mdio_bus)
		cancel_delayed_work_sync(&pdev->link_work);
	pdev->opened = 0;
	/* Stop xmit queue */
	netif_carrier_off(ndev);
	netif_tx_disable(ndev);

	/* stop phy */
	if (pdev->phy_dev)
		phy_stop(pdev->phy_dev);

	apm_enet_mac_rx_state(priv, 0);
	apm_enet_mac_tx_state(priv, 0);

	apm_enet_irq_disable_all(pdev);
	apm_enet_napi_disable_all(pdev);

	for (qindex = 0; qindex < pdev->num_tx_completion_queues; qindex++)
		apm_enet_dequeue_msg(pdev->tx_completion[qindex], -1);

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
		apm_enet_dequeue_msg(pdev->rx[qindex], -1);

		/* Clean-up free pool */
		apm_enet_void_fp(pdev->rx_skb_pool[qindex],
			pdev->qm_queues.rx_fp[qindex].qid,
		       	APM_ENET_REGULAR_FRAME);
#ifdef CONFIG_XGENE_NET_JUMBO_FRAME
		/* Clean-up free pool */
		apm_enet_void_fp(pdev->rx_page_pool[qindex],
			pdev->qm_queues.rx_nxtfp[qindex].qid,
			APM_ENET_JUMBO_FRAME);
#endif
	}

	return APM_RC_OK;
}

static int apm_enet_change_mtu(struct net_device *ndev, int new_mtu)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct apm_enet_priv *priv = &pdev->priv;
	int eth_running;

	if (HW_MTU(new_mtu) < APM_ENET_MIN_MTU
			|| HW_MTU(new_mtu) > APM_ENET_MAX_MTU) {
		ENET_ERROR("Invalid MTU: %d\n", new_mtu);
		return -EINVAL;
	}

	ENET_PRINT("changing MTU from %d to %d\n", ndev->mtu, new_mtu);

	eth_running = netif_running(ndev);
	if (eth_running) {
		netif_stop_queue(ndev);
		apm_enet_mac_rx_state(priv, 0);
		apm_enet_mac_tx_state(priv, 0);
	}
	ndev->mtu = new_mtu;
	if (eth_running) {
		apm_enet_mac_rx_state(priv, 1);
		apm_enet_mac_tx_state(priv, 1);
		netif_tx_start_all_queues(ndev);
	}

	return 0;
}

static int apm_enet_qconfig(struct apm_enet_pdev *pdev)
{
	struct xgene_qmtm_qinfo qinfo;
	u8 slave = pdev->sdev->slave;
	u8 slave_i = pdev->sdev->slave_i;
	u8 qmtm_ip = pdev->sdev->qmtm_ip;
	int port = pdev->priv.port;
	int rc = 0;
	u32 qindex;
	struct apm_enet_qcontext *e2c;
	struct apm_enet_qcontext *c2e;

	memset(&pdev->qm_queues, 0, sizeof(struct eth_queue_ids));
	pdev->qm_queues.qm_ip = qmtm_ip;

#if !defined(SHARED_RX_COMPLETIONQ)
	if (port != MENET) {
#if defined(SINGLE_COMPLETIONQ)
		pdev->num_tx_completion_queues = 1;
#else
		pdev->num_tx_completion_queues = pdev->num_tx_queues;
#endif
		pdev->tx_completion = kmalloc(sizeof (struct apm_enet_qcontext *) *
			pdev->num_tx_completion_queues, GFP_KERNEL | __GFP_ZERO);
	}
#endif

	for (qindex = 0; qindex < pdev->num_tx_completion_queues; qindex++) {
		/* Allocate TX Completion queue from ETHx to CPUx */
		memset(&qinfo, 0, sizeof(qinfo));
		qinfo.slave = slave_i;
		qinfo.qaccess = apm_enet_get_qacess();
		qinfo.qtype = QTYPE_PQ;
		if (pdev->num_tx_completion_queues == pdev->num_tx_queues)
			qinfo.qsize = TX_COMPLETION_QSIZE;
		else
			qinfo.qsize = SHARED_COMPLETION_QSIZE;

		qinfo.flags = XGENE_SLAVE_DEFAULT_FLAGS;

		if ((rc = xgene_qmtm_set_qinfo(&qinfo)) != APM_RC_OK) {
			ENET_ERROR("Error: could not allocate completion queue "
				" for Ethernet Port %d to CPU\n", port);
			goto done;
		}

		pdev->qm_queues.comp[qindex].qid = qinfo.queue_id;

		if (qindex == 0)
			pdev->qm_queues.default_comp_qid = pdev->qm_queues.comp[0].qid;

		/* Setup TX Completion enet_to_cpu info */
		e2c = (struct apm_enet_qcontext *)kmalloc(sizeof (struct apm_enet_qcontext),
				GFP_KERNEL | __GFP_ZERO);
		e2c->count = qinfo.count;
		e2c->index = 0;
		e2c->nummsgs = &(((u32 *)qinfo.qfabric)[1]);
		e2c->msg32 = qinfo.msg32;
#ifdef CONFIG_ARCH_MSLIM1
		e2c->deq_pbn = qinfo.pbn;
		e2c->comp_fixup = 0;

		if (qmtm_ip == QMTM1) {
			apm_wq_assoc_write(qmtm_ip, qinfo.pbn);
			ENET_DBG_Q("CPU0 QM%d completion QID %d PBN %x\n",
					qmtm_ip, qinfo.queue_id, qinfo.pbn);
		}
#endif
		e2c->command = qinfo.command;
		e2c->pdev = pdev;
		e2c->queue_index = qindex;
		if (pdev->num_tx_completion_queues > 1)
			snprintf(e2c->irq_name, sizeof(e2c->irq_name), "%s-tx%d", pdev->ndev->name, qindex);
		else
			snprintf(e2c->irq_name, sizeof(e2c->irq_name), "%s-tx", pdev->ndev->name);
		pdev->tx_completion[qindex] = e2c;
		e2c->irq = qinfo.irq;
		/*
		 * Enable interrupt coalescence for Comp queue. This will
		 * reduce the interrupt overhead and better performance
		 * for application that source the packet such as iperf
		 */
		xgene_qmtm_intr_coalesce(qmtm_ip, qinfo.pbn, 0x7);
	}

	for (qindex = 0; qindex < pdev->num_tx_queues; qindex++) {
		/* Allocate EGRESS work queues from CPUx to ETHx*/
		memset(&qinfo, 0, sizeof(qinfo));
		qinfo.slave = slave;
		qinfo.qaccess = apm_enet_get_qacess();
		qinfo.qtype = QTYPE_PQ;
		qinfo.qsize = TX_COMPLETION_QSIZE;
		qinfo.flags = XGENE_SLAVE_DEFAULT_FLAGS;

		if ((rc = xgene_qmtm_set_qinfo(&qinfo)) != APM_RC_OK) {
			ENET_ERROR("Error: could not allocate egress queue "
				" for CPU to Ethernet Port %d\n", port);
			goto done;
		}

		pdev->qm_queues.tx[qindex].qid = qinfo.queue_id;

		/* Setup TX Frame cpu_to_enet info */
		c2e = (struct apm_enet_qcontext *)kmalloc(sizeof (struct apm_enet_qcontext),
			GFP_KERNEL | __GFP_ZERO);
		c2e->count = qinfo.count;
		c2e->index = 0;
#ifdef CONFIG_ARCH_MSLIM
		c2e->enq_pbn = qinfo.pbn;
#endif
		c2e->msg32 = qinfo.msg32;
		c2e->msg8 = (struct xgene_qmtm_msg_ext8 *)kmalloc(sizeof (struct xgene_qmtm_msg_ext8) *
				256 * c2e->count, GFP_KERNEL);
		c2e->command = qinfo.command;
		c2e->pdev = pdev;
		c2e->queue_index = qindex;
		pdev->tx[qindex] = c2e;
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
		/* Allocate EGRESS work queues from CPUx to ETHx*/
		memset(&qinfo, 0, sizeof(qinfo));
		qinfo.slave = slave;
		qinfo.qaccess = apm_enet_get_qacess();
		qinfo.qtype = QTYPE_PQ;
		qinfo.qsize = TX_COMPLETION_QSIZE;
		qinfo.flags = XGENE_SLAVE_DEFAULT_FLAGS;
		if ((rc = xgene_qmtm_set_qinfo(&qinfo)) != APM_RC_OK) {
			ENET_ERROR("Error: could not allocate egress queue "
				" for CPU to Ethernet Port %d\n", port);
			goto done;
		}
		pdev->qm_queues.hw[qindex].qid = qinfo.queue_id;
		/* Setup TX Frame cpu_to_enet info */
		c2e = (struct apm_enet_qcontext *)kmalloc(sizeof (struct apm_enet_qcontext),
			GFP_KERNEL | __GFP_ZERO);
		c2e->count = qinfo.count;
		c2e->index = 0;
		c2e->msg32 = qinfo.msg32;
		c2e->command = qinfo.command;
		c2e->pdev = pdev;
		c2e->queue_index = qindex;
		pdev->hw[qindex] = c2e;
#endif
	}

	pdev->qm_queues.default_tx_qid = pdev->qm_queues.tx[0].qid;

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
		/* Allocate INGRESS work queue from ETHx to CPUx */
		memset(&qinfo, 0, sizeof(qinfo));
		qinfo.slave = slave_i;
		qinfo.qaccess = apm_enet_get_qacess();
		qinfo.qtype = QTYPE_PQ;
		if (pdev->tx_completion)
			qinfo.qsize = RX_BUFFER_POOL_QSIZE;
		else
			qinfo.qsize = SHARED_COMPLETION_QSIZE;

		qinfo.flags = XGENE_SLAVE_DEFAULT_FLAGS;
		if ((rc = xgene_qmtm_set_qinfo(&qinfo)) != APM_RC_OK) {
			ENET_ERROR("Error: could not allocate ingress queue "
				" for Ethernet Port %d to CPU\n", port);
			goto done;
		}

		pdev->qm_queues.rx[qindex].qid = qinfo.queue_id;

		/* Setup RX Frame enet_to_cpu info */
		e2c = (struct apm_enet_qcontext *)kmalloc(sizeof (struct apm_enet_qcontext),
			GFP_KERNEL | __GFP_ZERO);
		e2c->count = qinfo.count;
		e2c->index = 0;
#ifdef CONFIG_ARCH_MSLIM1
		e2c->deq_pbn = qinfo.pbn;

		if (qmtm_ip == QMTM1) {
			apm_wq_assoc_write(qmtm_ip, e2c->deq_pbn);
			ENET_DBG_Q("CPU0 QM%d completion QID %d PBN %x\n",
				qmtm_ip, qinfo.queue_id, e2c->deq_pbn);
		}
#endif
		e2c->nummsgs = &(((u32 *)qinfo.qfabric)[1]);
		e2c->msg32 = qinfo.msg32;
		e2c->command = qinfo.command;
		e2c->pdev = pdev;
		e2c->queue_index = qindex;
		if (pdev->tx_completion) {
			if (pdev->num_rx_queues > 1)
				snprintf(e2c->irq_name, sizeof(e2c->irq_name), "%s-rx%d", pdev->ndev->name, qindex);
			else
				snprintf(e2c->irq_name, sizeof(e2c->irq_name), "%s-rx", pdev->ndev->name);
		} else {
			if (pdev->num_rx_queues > 1)
				snprintf(e2c->irq_name, sizeof(e2c->irq_name), "%s-tx%d-rx%d", pdev->ndev->name, qindex, qindex);
			else
				snprintf(e2c->irq_name, sizeof(e2c->irq_name), "%s-tx-rx", pdev->ndev->name);
		}
		e2c->irq = qinfo.irq;
		e2c->c2e_count = 1;
		/*
		 * Enable interrupt coalescence for Rx queue. This will
		 * reduce the interrupt overhead and better performance
		 * for application that source the packet such as iperf
		 */
		xgene_qmtm_intr_coalesce(qmtm_ip, qinfo.pbn, 0x7);
		pdev->rx[qindex] = e2c;

		/* Allocate free pool for ETHx from CPUx */
		memset(&qinfo, 0, sizeof(qinfo));
		qinfo.slave = slave;
		qinfo.qaccess = apm_enet_get_qacess();
		qinfo.qtype = QTYPE_FP;
		qinfo.qsize = RX_BUFFER_POOL_QSIZE;
		qinfo.flags = XGENE_SLAVE_DEFAULT_FLAGS;

		if ((rc = xgene_qmtm_set_qinfo(&qinfo)) != APM_RC_OK) {
			ENET_ERROR("Error: could not allocate fp queue "
				" for CPU to Ethernet Port %d\n", port);
			goto done;
		}

		pdev->qm_queues.rx_fp[qindex].qid = qinfo.queue_id;
		pdev->qm_queues.rx_fp[qindex].pbn = qinfo.pbn;

		/* Setup RX SKB Free Pool cpu_to_enet info */
		c2e = (struct apm_enet_qcontext *)kmalloc(sizeof (struct apm_enet_qcontext),
			GFP_KERNEL | __GFP_ZERO);
		c2e->skb = kmalloc(sizeof (struct sk_buff *) *
			qinfo.count, GFP_KERNEL | __GFP_ZERO);
		c2e->count = qinfo.count;
		c2e->index = 0;
		c2e->msg16 = qinfo.msg16;
		c2e->command = qinfo.command;
		c2e->pdev = pdev;
		c2e->queue_index = qindex;
		c2e->eqnum = QMTM_QUEUE_ID(qmtm_ip, qinfo.queue_id);
		pdev->rx_skb_pool[qindex] = c2e;
		pdev->rx[qindex]->c2e_skb = pdev->rx_skb_pool[qindex];
		/* Configure free pool */
		apm_enet_init_fp(pdev->rx_skb_pool[qindex], APM_ENET_REGULAR_FRAME);
#ifdef CONFIG_XGENE_NET_JUMBO_FRAME
		/* Allocate next free pool for ETHx from CPUx */
		memset(&qinfo, 0, sizeof(qinfo));
		qinfo.slave = slave;
		qinfo.qaccess = apm_enet_get_qacess();
		qinfo.qtype = QTYPE_FP;
		qinfo.qsize = JUMBO_PAGE_POOL_QSIZE;
		qinfo.flags = XGENE_SLAVE_DEFAULT_FLAGS;

		if ((rc = xgene_qmtm_set_qinfo(&qinfo)) != APM_RC_OK) {
			ENET_ERROR("Error: could not allocate next fp queue "
				" for CPU to Ethernet Port %d\n", port);
			goto done;
		}

		pdev->qm_queues.rx_nxtfp[qindex].qid = qinfo.queue_id;
		pdev->qm_queues.rx_nxtfp[qindex].pbn = qinfo.pbn;

		/* Setup RX PAGE Free Pool cpu_to_enet info */
		c2e = (struct apm_enet_qcontext *)kmalloc(sizeof (struct apm_enet_qcontext),
			GFP_KERNEL | __GFP_ZERO);
		c2e->count = qinfo.count;
		c2e->index = 0;
		c2e->msg16 = qinfo.msg16;
		c2e->command = qinfo.command;
		c2e->pdev = pdev;
		c2e->queue_index = qindex;
		c2e->eqnum = QMTM_QUEUE_ID(qmtm_ip, qinfo.queue_id);
		pdev->rx_page_pool[qindex] = c2e;
		pdev->rx[qindex]->c2e_page = pdev->rx_page_pool[qindex];
		/* Configure free pool */
		apm_enet_init_fp(pdev->rx_page_pool[qindex], APM_ENET_JUMBO_FRAME);
#endif
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
		/* Allocate free pool for ETHx from CPUx for IPV4 fwd offload */
		memset(&qinfo, 0, sizeof(qinfo));
		qinfo.slave = slave;
		qinfo.qaccess = apm_enet_get_qacess();
		qinfo.qtype = QTYPE_FP;
		qinfo.qsize = RX_BUFFER_POOL_QSIZE;
		qinfo.flags = XGENE_SLAVE_DEFAULT_FLAGS;
		if ((rc = xgene_qmtm_set_qinfo(&qinfo)) != APM_RC_OK) {
			ENET_ERROR("Error: could not allocate fp queue "
				" for CPU to Ethernet Port %d\n", port);
			goto done;
		}
		pdev->qm_queues.hw_fp[qindex].qid = qinfo.queue_id;
		pdev->qm_queues.hw_fp[qindex].pbn = qinfo.pbn;
		/* Setup RX SKB Free Pool cpu_to_enet info for IPv4 fwd offload */
		c2e = (struct apm_enet_qcontext *)kmalloc(sizeof (struct apm_enet_qcontext),
			GFP_KERNEL | __GFP_ZERO);
		c2e->skb = kmalloc(sizeof (struct sk_buff *) *
			qinfo.count, GFP_KERNEL | __GFP_ZERO);
		c2e->count = qinfo.count;
		c2e->index = 0;
		c2e->msg16 = qinfo.msg16;
		c2e->command = qinfo.command;
		c2e->pdev = pdev;
		c2e->queue_index = qindex;
		c2e->eqnum = QMTM_QUEUE_ID(qmtm_ip, qinfo.queue_id);
		pdev->hw_skb_pool[qindex] = c2e;
		/* Configure free pool */
		apm_enet_init_fp(pdev->hw_skb_pool[qindex], APM_ENET_REGULAR_FRAME);
		apm_enet_fill_fp(pdev->hw_skb_pool[qindex], pdev->rx_buff_cnt,
		       	APM_ENET_REGULAR_FRAME);
#endif
	}

	for (qindex = 0; qindex < pdev->num_tx_queues; qindex++) {
		u16 eqnum;

		c2e = pdev->tx[qindex];
		if (pdev->tx_completion) {
			if (pdev->num_tx_completion_queues > 1) {
				e2c = pdev->tx_completion[qindex];
				eqnum = QMTM_QUEUE_ID(qmtm_ip, pdev->qm_queues.comp[qindex].qid);
#if defined(TX_COMPLETION_POLLING)
				c2e->c2e_skb = e2c;
#endif
			} else {
				e2c = pdev->tx_completion[0];
				eqnum = QMTM_QUEUE_ID(qmtm_ip, pdev->qm_queues.comp[0].qid);
			}
		} else {
			u32 rqindex = qindex % pdev->num_rx_queues;

			e2c = pdev->rx[rqindex];
			eqnum = QMTM_QUEUE_ID(qmtm_ip, pdev->qm_queues.rx[rqindex].qid);
		}

		c2e->e2c = e2c;
		c2e->eqnum = eqnum;
		e2c->c2e = c2e;
	}

	/*
	 * Assign TX completion interface threshold
	 * based on Tx queue size
	 */
	pdev->tx_cqt_hi = pdev->tx[0]->count / 2;
	pdev->tx_cqt_low = pdev->tx_cqt_hi - 100;

	pdev->qm_queues.default_rx_qid = pdev->qm_queues.rx[0].qid;
	pdev->qm_queues.default_rx_fp_qid = pdev->qm_queues.rx_fp[0].qid;
	pdev->qm_queues.default_rx_fp_pbn = pdev->qm_queues.rx_fp[0].pbn;
	pdev->qm_queues.default_rx_nxtfp_qid = pdev->qm_queues.rx_nxtfp[0].qid;
	pdev->qm_queues.default_rx_nxtfp_pbn = pdev->qm_queues.rx_nxtfp[0].pbn;

	ENET_DBG_Q("Port %d CQID %d FP %d FP PBN %d\n",
		port,
		pdev->qm_queues.default_comp_qid,
		pdev->qm_queues.default_rx_fp_qid,
		pdev->qm_queues.default_rx_fp_pbn);

	ENET_DBG_Q("RX QID %d\n", pdev->qm_queues.default_rx_qid);
	ENET_DBG_Q("TX QID %d\n", pdev->qm_queues.default_tx_qid);
done:
	return rc;
}

static int apm_enet_delete_queue(struct apm_enet_pdev *pdev)
{
	struct xgene_qmtm_qinfo qinfo;
	u32 qindex;
	u8 qmtm_ip = pdev->sdev->qmtm_ip;
	int rc = 0;
	u16 queue_id;

	qinfo.qmtm_ip = qmtm_ip;

	for (qindex = 0; qindex < pdev->num_tx_completion_queues; qindex++) {
		/* Delete INGRESS completion queue from ETHx to CPUx */
		queue_id = pdev->qm_queues.comp[qindex].qid;

		if (queue_id) {
			qinfo.queue_id = queue_id;
			rc |= xgene_qmtm_clr_qinfo(&qinfo);
		}
	}

	for (qindex = 0; qindex < pdev->num_tx_queues; qindex++) {
		/* Delete EGRESS work queues from CPUx to ETHx*/
		queue_id = pdev->qm_queues.tx[qindex].qid;

		if (queue_id) {
			qinfo.queue_id = queue_id;
			rc |= xgene_qmtm_clr_qinfo(&qinfo);
		}
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
		queue_id = pdev->qm_queues.hw[qindex].qid;

		if (queue_id) {
			qinfo.queue_id = queue_id;
			rc |= xgene_qmtm_clr_qinfo(&qinfo);
		}
#endif
	}

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
		/* Delete INGRESS work queue from ETHx to CPUx */
		queue_id = pdev->qm_queues.rx[qindex].qid;

		if (queue_id) {
			qinfo.queue_id = queue_id;
			rc |= xgene_qmtm_clr_qinfo(&qinfo);
		}

		/* Delete free pool for ETHx from CPUx */
		queue_id = pdev->qm_queues.rx_fp[qindex].qid;

		if (queue_id) {
			qinfo.queue_id = queue_id;
			rc |= xgene_qmtm_clr_qinfo(&qinfo);
		}

#ifdef CONFIG_XGENE_NET_JUMBO_FRAME
		/* Delete next free pool for ETHx from CPUx */
		queue_id = pdev->qm_queues.rx_nxtfp[qindex].qid;

		if (queue_id) {
			qinfo.queue_id = queue_id;
			rc |= xgene_qmtm_clr_qinfo(&qinfo);
		}
#endif
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
		/* Delete free pool for ETHx from CPUx for IPv4 Fwd Offload */
		queue_id = pdev->qm_queues.hw_fp[qindex].qid;

		if (queue_id) {
			qinfo.queue_id = queue_id;
			rc |= xgene_qmtm_clr_qinfo(&qinfo);
		}
#endif
	}

	return rc;
}

struct net_device_stats *apm_enet_stats(struct net_device *ndev)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct apm_enet_priv *priv = &(pdev->priv);
	struct net_device_stats *nst = &pdev->nstats;
	struct eth_rx_stat *rx_stats;
	struct eth_tx_stats *tx_stats;
	unsigned long bytes, packets;
	u32 qindex;

	rx_stats = &pdev->stats.rx_stats;
	tx_stats = &pdev->stats.tx_stats;

	// XXX: do we need irq disable/enable ?
	local_irq_disable();
	apm_enet_get_stats(priv, &pdev->stats);

	/* TBD: Update all the fields on net_device_stats */

	/* Since we use multiple queues for rx-tx, we need to add the rx-tx
	   counters for all queues to get total rx-tx count for an interface */
	bytes = 0;
	packets = 0;
	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
		if (pdev->rx[qindex]) {
			bytes += pdev->rx[qindex]->bytes;
			packets += pdev->rx[qindex]->packets;
		}
	}

	rx_stats->rx_byte_count = bytes;
	rx_stats->rx_packet_count = packets;

	bytes = 0;
	packets = 0;
	for (qindex = 0; qindex < pdev->num_tx_queues; qindex++) {
		if (pdev->tx[qindex]) {
			bytes += pdev->tx[qindex]->bytes;
			packets += pdev->tx[qindex]->packets;
		}
	}

	tx_stats->tx_byte_count = bytes;
	tx_stats->tx_packet_count = packets;

	nst->rx_bytes = rx_stats->rx_byte_count;
	nst->rx_packets = rx_stats->rx_packet_count;

	nst->tx_bytes = tx_stats->tx_byte_count;
	nst->tx_packets = tx_stats->tx_packet_count;

	nst->rx_dropped = rx_stats->rx_drop_pkt_count;
	nst->tx_dropped = tx_stats->tx_drop_frm_count;

	nst->rx_crc_errors = rx_stats->rx_fcs_err_count;
	nst->rx_length_errors = rx_stats->rx_frm_len_err_pkt_count;
	nst->rx_frame_errors = rx_stats->rx_alignment_err_pkt_count;
	nst->rx_over_errors = (rx_stats->rx_oversize_pkt_count
			+ pdev->stats.estats.rx_hw_overrun);

	nst->rx_errors = (rx_stats->rx_fcs_err_count
	       	+ rx_stats->rx_frm_len_err_pkt_count
		+ rx_stats->rx_oversize_pkt_count
		+ rx_stats->rx_undersize_pkt_count
		+ pdev->stats.estats.rx_hw_overrun
		+ pdev->stats.estats.rx_hw_errors);

	nst->tx_errors = tx_stats->tx_fcs_err_frm_count +
		tx_stats->tx_undersize_frm_count;

	local_irq_enable();

	pdev->stats.estats.rx_hw_errors = 0;
	pdev->stats.estats.rx_hw_overrun = 0;

	return nst;
}

static int apm_enet_set_mac_address(struct net_device *ndev, void *p)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct apm_enet_priv *priv = &(pdev->priv);
	struct sockaddr *addr = p;

	if (netif_running(ndev))
		return -EBUSY;

	memcpy(ndev->dev_addr, addr->sa_data, ndev->addr_len);
	apm_enet_set_mac_addr(priv, (unsigned char *)(ndev->dev_addr));
#ifdef SET_RX_MODE
	apm_preclass_update_mac(apm_enet_get_port(pdev), TYPE_SYS_MACADDR,
				ETHERNET_MACADDR,
				apm_sys_macmask[ETHERNET_MACADDR],
				ndev->dev_addr);
#endif
	return APM_RC_OK;
}

#ifdef SET_RX_MODE
static int apm_enet_update_umcast_list(struct net_device *ndev, u8 *addr, struct list_head *head)
{
	struct umcast_entry *entry, *temp, *new;
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	int rc = APM_RC_OK;
	int count = netdev_hw_addr_list_count(&(ndev)->uc) + netdev_hw_addr_list_count(&(ndev)->mc);
	/* Search for existing matched address */
	list_for_each_entry_safe(entry, temp, head, list) {
		if (!compare_ether_addr(entry->addr, addr)) {
			entry->flag = MARK_PRESENT;
			return 0;
		}
	}

	/* Since we did not found matched entry,
	 * this must be new mac address to be added in to the list.
	 */
	rc = apm_preclass_update_mac(apm_enet_get_port(pdev), TYPE_SYS_MACADDR,
				MULTICAST_MACADDR + count,
				apm_sys_macmask[ETHERNET_MACADDR],
				addr);
	if (rc == APM_RC_OK) {
		new = kzalloc(sizeof(struct umcast_entry), GFP_ATOMIC);
		INIT_LIST_HEAD(&new->list);
		memcpy(new->addr, addr, 6);
		new->flag = MARK_PRESENT;
		list_add_tail(&new->list, head);
	}

	return rc;
}
static int apm_enet_remove_stale_umcast_entries(struct net_device *ndev, struct list_head *head)
{
	struct umcast_entry *entry, *temp;
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	int rc = 0;
	int index = 1;
	/* Delete all umcast entries which are not present in kernel list */
	list_for_each_entry_safe(entry, temp, head,list) {
		if (entry->flag == MARK_DEL) {
			rc = apm_preclass_update_mac(apm_enet_get_port(pdev), TYPE_SYS_MACADDR,
				MULTICAST_MACADDR + index ,
				NULL,
				NULL);
			index++;
			list_del(&entry->list);
			kfree(entry);
		}
	}

	return rc;
}

static inline void apm_enet_mark_all_umcast_entries(struct list_head *head, int flag)
{
	struct umcast_entry *entry, *temp;

	/*Mark All the entries as to be deleted */
	list_for_each_entry_safe(entry, temp, head,list)
		entry->flag = flag;
}

static int apm_enet_update_list(struct net_device *dev, struct list_head *head)
{
	struct apm_enet_pdev *pdev = netdev_priv(dev);
	struct netdev_hw_addr *ha;
	struct netdev_hw_addr_list *l;
	int err = 0;

	/*Mark All the entries as to be deleted */
	apm_enet_mark_all_umcast_entries(head, MARK_DEL);

	if (head == &pdev->ucast_head)
		l = &(dev)->uc;
	else
		l = &(dev)->mc;

	netdev_hw_addr_list_for_each(ha, l)
		if ((err = apm_enet_update_umcast_list(dev,
				ha->addr, head)) != 0)
			break;

	/* Traverse and remove all the entries which are marked for deletion. */
	err |= apm_enet_remove_stale_umcast_entries(dev, head);

	return err;
}

/**
 * apm_enet_set_rx_mode - Secondary Unicast, Multicast and Promiscuous mode set
 * @netdev: network interface device structure
 *
 * The set_rx_mode entry point is called whenever the unicast or multicast
 * address lists or the network interface flags are updated. This routine is
 * responsible for configuring the hardware for proper unicast, multicast,
 * promiscuous mode, and all-multi behavior.
 **/
static void apm_enet_set_rx_mode(struct net_device *ndev)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	bool use_uc = false;
	bool use_mc = false;
	u32 port = apm_enet_get_port(pdev);

	/* Check for Promiscuous and All Multicast modes */
	if (ndev->flags & IFF_PROMISC) {
		if (!(pdev->flags & IFF_PROMISC)) {
			apm_preclass_update_mac(port, TYPE_SYS_MACADDR,
					UNICAST_MACADDR,
					apm_sys_macmask[UNICAST_MACADDR],
					apm_sys_macaddr[UNICAST_MACADDR]);
			apm_preclass_update_mac(port, TYPE_SYS_MACADDR,
					MULTICAST_MACADDR,
					apm_sys_macmask[MULTICAST_MACADDR],
					apm_sys_macaddr[MULTICAST_MACADDR]);
		}
	} else if (ndev->flags & IFF_ALLMULTI) {
		if (!(pdev->flags & IFF_ALLMULTI)) {
			apm_preclass_update_mac(port, TYPE_SYS_MACADDR,
					MULTICAST_MACADDR,
					apm_sys_macmask[MULTICAST_MACADDR],
					apm_sys_macaddr[MULTICAST_MACADDR]);
		}
	} else {
		if (pdev->flags & (IFF_PROMISC | IFF_ALLMULTI)) {
			apm_preclass_update_mac(port, TYPE_SYS_MACADDR,
					MULTICAST_MACADDR, NULL, NULL);
		}
		use_mc = true;
	}

	/* Check for Unicast Promiscuous mode */
	if (ndev->uc.count > APM_MAX_UC_MC_MACADDR) {
		if (!(pdev->uc_count > APM_MAX_UC_MC_MACADDR)) {
			apm_preclass_update_mac(port, TYPE_SYS_MACADDR,
					UNICAST_MACADDR,
					apm_sys_macmask[UNICAST_MACADDR],
					apm_sys_macaddr[UNICAST_MACADDR]);
		}
	} else if (!(ndev->flags & IFF_PROMISC)) {
		if (pdev->flags & IFF_PROMISC) {
			apm_preclass_update_mac(port, TYPE_SYS_MACADDR,
					UNICAST_MACADDR, NULL, NULL);
		}
		use_uc = true;
	}

	if (use_uc) {
		/* Update Unicast MAC address in Patricia tree which are present in ndev->uc.list and absent in our_uc_list */
		/* Clear Unicast MAC address in Patricia tree which are present in our_uc_list and absent in ndev->uc.list */
		apm_enet_update_list(ndev, &pdev->ucast_head);
	}

	if (use_mc) {
		/* Update Multicast MAC address in Patricia tree which are present in ndev->mc.list and absent in our_mc_list */
		/* Clear  Multicast MAC address in Patricia tree which are present in our_mc_list and absent in ndev->mc.list */
		apm_enet_update_list(ndev, &pdev->mcast_head);
	}

	pdev->flags = ndev->flags;
	pdev->uc_count = ndev->uc.count;
}
#endif

irqreturn_t apm_enet_irq(int irq, void *ndev)
{
	/* Check for error interrupt */
	apm_enet_err_irq(irq, ndev);
	return IRQ_HANDLED;
}

/* net_device_ops structure for data path ethernet */
static const struct net_device_ops apm_dnetdev_ops = {
	.ndo_open		= apm_enet_open,
	.ndo_stop		= apm_enet_close,
/*	.ndo_select_queue	= apm_enet_select_queue,	*/
	.ndo_start_xmit		= apm_enet_start_xmit,
/*	.ndo_do_ioctl		= apm_enet_ioctl, */
	.ndo_tx_timeout		= apm_enet_timeout,
	.ndo_get_stats		= apm_enet_stats,
	.ndo_change_mtu		= apm_enet_change_mtu,
	.ndo_set_mac_address	= apm_enet_set_mac_address,
#ifdef SET_RX_MODE
	.ndo_set_rx_mode	= apm_enet_set_rx_mode,
#endif
#ifdef CONFIG_NET_RX_BUSY_POLL
	.ndo_busy_poll          = xgenet_low_latency_recv,
#endif
};

u32 apm_enet_get_irqbit(u32 port_id)
{
	u32 irq_bit;

	switch (port_id) {
	case ENET_0:
	case ENET_1:
		irq_bit = 0x1U << ENET_0;
		break;
	case ENET_2:
	case ENET_3:
		irq_bit = 0x1U << ENET_2;
		break;
	default:
		irq_bit = 0x1U << port_id;
		break;
	}

	return irq_bit;
}

static void apm_enet_register_irq(struct net_device *ndev)
{
	u32 port_id;
	u32 irq_bit;  // 1-bit for each port irq
	struct apm_enet_pdev *pdev;
	u32 qindex;
#ifndef CONFIG_ARCH_MSLIM
	u32 irq_flags = 0;
#else
	u32 irq_flags = IRQF_SHARED;
#endif

	pdev = (struct apm_enet_pdev *)netdev_priv(ndev);
	port_id  = apm_enet_get_port(pdev);

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
		if (request_irq(pdev->rx[qindex]->irq, apm_enet_e2c_irq,
				irq_flags, pdev->rx[qindex]->irq_name,
				(void *) pdev->rx[qindex]) != 0) {
			ENET_ERROR("Failed to request_irq %d for RX Frame \
				for port %d\n", pdev->rx[qindex]->irq,
				apm_enet_get_port(pdev));
			return;
		}

		/* Disable interrupts for RX queue mailboxes */
		disable_irq_nosync(pdev->rx[qindex]->irq);
		pdev->rx[qindex]->irq_enabled = 0;
	}
#if defined(TX_COMPLETION_INTERRUPT)
	for (qindex = 0; qindex < pdev->num_tx_completion_queues; qindex++) {
		if (request_irq(pdev->tx_completion[qindex]->irq,
				apm_enet_e2c_irq, irq_flags,
				pdev->tx_completion[qindex]->irq_name,
				(void *) pdev->tx_completion[qindex])) {
			ENET_ERROR("Failed to request_irq %d for \
				TX Completion for port %d\n",
				pdev->tx_completion[qindex]->irq,
			       	apm_enet_get_port(pdev));
			return;
		}

		/* Disable interrupts for completion queue mailboxes */
		disable_irq_nosync(pdev->tx_completion[qindex]->irq);
		pdev->tx_completion[qindex]->irq_enabled = 0;
	}
#endif

	irq_bit = apm_enet_get_irqbit(port_id);

	if (irq_refcnt & irq_bit) /* irq already registered */
		return;
	else
		irq_refcnt |= irq_bit;

#ifndef CONFIG_ARCH_MSLIM
	if ((request_irq(pdev->enet_err_irq, apm_enet_irq,
			IRQF_SHARED, ndev->name, ndev)) != 0)
		ENET_ERROR("Failed to reg Enet Error IRQ %d\n",
				pdev->enet_err_irq);
	if ((request_irq(pdev->enet_mac_err_irq,
			apm_enet_mac_err_irq, IRQF_SHARED,
			ndev->name, ndev)) != 0)
		ENET_ERROR("Failed to reg Enet MAC Error IRQ %d\n",
				pdev->enet_mac_err_irq);
	if ((request_irq(pdev->enet_qmi_err_irq,
			apm_enet_qmi_err_irq,
			IRQF_SHARED, ndev->name, ndev)) != 0)
		ENET_ERROR("Failed to reg Enet QMI Error IRQ %d\n",
				pdev->enet_qmi_err_irq);
#endif
}

static void apm_enet_free_irq(struct net_device *ndev)
{
	u32 port_id;
	u32 irq_bit;  // 1-bit for each port irq
	struct apm_enet_pdev *pdev;
	u32 qindex;

	pdev = (struct apm_enet_pdev *)netdev_priv(ndev);
	port_id  = apm_enet_get_port(pdev);

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
		free_irq(pdev->rx[qindex]->irq, pdev->rx[qindex]);
		pdev->rx[qindex]->irq_enabled = 0;
	}
#if defined(TX_COMPLETION_INTERRUPT)
	for (qindex = 0; qindex < pdev->num_tx_completion_queues; qindex++) {
		free_irq(pdev->tx_completion[qindex]->irq,
				pdev->tx_completion[qindex]);
		pdev->tx_completion[qindex]->irq_enabled = 0;
	}
#endif

	irq_bit = apm_enet_get_irqbit(port_id);

	/* irq not registered */
	if (!(irq_refcnt & irq_bit))
		return;
	else
		irq_refcnt &= ~irq_bit;

#ifndef CONFIG_ARCH_MSLIM
	/* Release IRQ */
	free_irq(pdev->enet_err_irq, ndev);
	free_irq(pdev->enet_mac_err_irq, ndev);
	free_irq(pdev->enet_qmi_err_irq, ndev);
#endif
}

static int apm_enet_driver_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int apm_enet_driver_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t apm_enet_driver_read(struct file *file, char __user * buf,
		size_t count, loff_t *ppos)
{
	return 0;
}

// FIXME: move this to apm_enet_utils.c ?
static void apm_enet_driver_write_help(void)
{
	ENET_PRINT( "echo"
		" <port> <command> <block> <reg> <value> > /proc/%s\n\n"
		" where port:\n"
		" \t 0 for  1G-SGMII0\n"
		" \t 1 for  1G-SGMII1\n"
		" \t 2 for  1G-SGMII2\n"
		" \t 3 for  1G-SGMII3\n"
		" \t 4 for 10G-XGMII0\n"
		" \t 5 for 10G-XGMII1\n"
		" \t 6 for 10G-XGMII2\n"
		" \t 7 for 10G-XGMII3\n"
		" \t 8 for  1G-RGMII\n"
		"    command:\n"
		" \t 0 for read\n"
		" \t 1 for write\n"
		"   block ID:\n"
		" \t 1 ETH CSR\n"
		" \t 2 ETH CLE\n"
		" \t 3 ETH QMI\n"
		" \t 4 ETH SDS CSR\n"
		" \t 5 ETH CLKRST CSR\n"
		" \t 6 ETH DIAG CSR\n"
		" \t 7 ETH (MENET) MDIO CSR (port 0 to 3)\n"
		" \t 8 ETH INT PHY\n"
		" \t 9 ETH EXT PHY\n"
		" \t10 MCX MAC\n"
		" \t11 MCX STATS\n"
		" \t12 MCX MAC CSR\n"
		" \t13 SATA ENET CSR   - Only for port 0 to 3\n"
		" \t14 AXG MAC         - Only for port 4 to 7\n"
		" \t15 AXG STATS       - Only for port 4 to 7\n"
		" \t16 AXG MAC CSR     - Only for port 4 to 7\n"
		" \t17 XGENET PCS      - Only for port 4 to 7\n"
		" \t18 XGENET_MDIO_CSR - Only for port 4 to 7\n\n"
		"        reg is register offset\n"
		"      value is value to for write\n", APM_ENET_DRIVER_NAME);
}

static ssize_t apm_enet_driver_write(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	int port;
	u32 cmd, data;
	u32 reg_offset = 0;
        u32 block_id = 0;
	char *buffer = (char *)buf;
	char *tok;

	if ((tok = strsep(&buffer, " ")) == NULL) {
		goto __ret_driver_write;
 	}
	port = simple_strtol(tok, NULL, 10);
	if ((tok = strsep(&buffer, " ")) == NULL) {
		goto __ret_driver_write;
 	}
	cmd = simple_strtol(tok, NULL, 10);
	if ((tok = strsep(&buffer, " ")) == NULL) {
		goto __ret_driver_write;
 	}
	block_id = simple_strtol(tok, NULL, 10);
	if ((tok = strsep(&buffer, " ")) == NULL) {
		goto __ret_driver_write;
 	}
	reg_offset = simple_strtol(tok, NULL, 16);
	if (cmd == APM_ENET_WRITE_CMD) {
		if ((tok = strsep(&buffer, " ")) == NULL) {
			goto __ret_driver_write;
		}
		data = simple_strtol(tok, NULL, 16);
	}

        if (port < 0 || port >= MAX_ENET_PORTS) {
        	ENET_ERROR("Invalid port number\n");
		goto __ret_driver_write;
	}
        if (cmd >= APM_ENET_MAX_CMD || block_id >= BLOCK_ETH_MAX) {
        	ENET_ERROR("Invalid command or block ID\n");
		goto __ret_driver_write;
        }

        if (block_id >= BLOCK_ETH_MAX) {
                ENET_ERROR("This is not yet supported \n ");
                return 0;
        }

        switch (cmd) {
        case APM_ENET_READ_CMD:
		apm_enet_read(&enet_dev[port]->priv, block_id, reg_offset,
                		&data);
                ENET_PRINT("ETH%d read block[%d] reg[0x%08X] value 0x%08X\n",
                       port, block_id, reg_offset, data);
                break;
        case APM_ENET_WRITE_CMD:
                ENET_PRINT("ETH%d write block[%d] reg[0X%08X] value 0x%08X\n",
                       port, block_id, reg_offset, data);
		apm_enet_write(&enet_dev[port]->priv, block_id,
			       	reg_offset, data);
                break;
        default:
                ENET_ERROR("Unsupport command %d\n", cmd);
                break;
        }
	return count;

__ret_driver_write:
	apm_enet_driver_write_help();
	return count;
}

static long apm_enet_driver_ioctl(struct file *file,
				u32 cmd, unsigned long arg)
{
        return 0;
}

struct file_operations apm_enet_driver_fops = {
	.owner = THIS_MODULE,
	.open = apm_enet_driver_open,
	.release = apm_enet_driver_release,
	.read = apm_enet_driver_read,
	.write = apm_enet_driver_write,
	.unlocked_ioctl = apm_enet_driver_ioctl,
};

static void apm_enet_iounmap(struct apm_enet_priv *priv)
{
	iounmap(priv->vaddr_base);
	iounmap(priv->vpaddr_base);
	iounmap(priv->mac_mii_addr_v);
}

static void mac_link_timer(struct work_struct *work)
{
	struct apm_enet_pdev *pdev =
		container_of(to_delayed_work(work),
				struct apm_enet_pdev, link_work);
	struct apm_enet_priv *priv = &pdev->priv;
	int link_poll_interval;

	if (!pdev->opened)
		return;
	mutex_lock(&pdev->link_lock);
	if (!priv->get_link_status)
		goto bail;
	if (priv->get_link_status(priv)) {
		if (!netif_carrier_ok(pdev->ndev)) {
			netif_carrier_on(pdev->ndev);
			netif_tx_start_all_queues(pdev->ndev);
			apm_enet_mac_init(priv, pdev->ndev->dev_addr,
					priv->speed, priv->crc);

			ENET_PRINT("%s link up %u Mbps\n",
			    	pdev->ndev->name, priv->speed);
		}
		link_poll_interval = PHY_POLL_LINK_ON;
	} else {
		if (netif_carrier_ok(pdev->ndev)) {
			apm_enet_mac_rx_state(priv, 0);
			netif_carrier_off(pdev->ndev);
			netif_tx_disable(pdev->ndev);
			ENET_PRINT("%s link is down\n",
			    	pdev->ndev->name);
		}
		link_poll_interval = PHY_POLL_LINK_OFF;
	}
	apm_enet_mac_rx_state(priv, priv->link_status);
	apm_enet_mac_tx_state(priv, priv->link_status);
	schedule_delayed_work(&pdev->link_work, link_poll_interval);
bail:
	mutex_unlock(&pdev->link_lock);
}

static int apm_enet_get_u32_param(struct platform_device *pdev,
				  const char *name, u32 *param)
{
	int rc;
#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT)) {
		struct acpi_dsm_entry entry;
		int val;

		if (acpi_dsm_lookup_value(ACPI_HANDLE(&pdev->dev),
					  name, 0, &entry) || !entry.value)
			return -EINVAL;
		*param = kstrtoint(entry.value, 0, &val) ? 0 : val;
		kfree(entry.key);
		kfree(entry.value);
		return 0;
	}
#endif
	rc = of_property_read_u32(pdev->dev.of_node, name, param);
	return rc;
}

static int apm_enet_get_str_param(struct platform_device *pdev,
				  const char *name, char *buf, int len)
{
	const char *param;
	int rc;

	buf[0] = '\0';
#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT)) {
		struct acpi_dsm_entry entry;

		if (acpi_dsm_lookup_value(ACPI_HANDLE(&pdev->dev),
					  name, 0, &entry) || !entry.value)
			return -EINVAL;
		strncpy(buf, entry.value, len);
		buf[len - 1] = '\0';
		kfree(entry.key);
		kfree(entry.value);
		return 0;
	}
#endif
	rc = of_property_read_string(pdev->dev.of_node, name, &param);
	if (rc == 0) {
		strncpy(buf, param, len);
		buf[len - 1] = '\0';
	}
	return rc;
}

static int apm_enet_get_mac_param(struct platform_device *pdev,
				  char *name, u8 *buf)
{
	const u8 *mac;
#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT)) {
		struct acpi_dsm_entry entry;
		char *value_str;
		u32 val;
		int i;

		if (acpi_dsm_lookup_value(ACPI_HANDLE(&pdev->dev),
					  name, 0, &entry) || !entry.value)
			return -EINVAL;
		value_str = entry.value;
		for (i = 0; i < 6 && value_str; i++) {
			sscanf(value_str, "%02X", &val);
			buf[i] = val & 0xFF;
			value_str = strchr(value_str, ':');
			if (value_str)
				value_str++;
		}
		kfree(entry.key);
		kfree(entry.value);
		return 0;
	}
#endif
	mac = of_get_mac_address(pdev->dev.of_node);
	if (!mac)
		return -EINVAL;

	memcpy(buf, mac, ETH_ALEN);
	return 0;
}

static int apm_enet_get_byte_param(struct platform_device *pdev,
				   const char *name, u8 *buf, int len)
{
	u32 *tmp;
	int rc = 0;
	int i;
#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT)) {
		struct acpi_dsm_entry entry;
		char *value_str;
		u32 val;
		int i;

		if (acpi_dsm_lookup_value(ACPI_HANDLE(&pdev->dev),
					  name, 0, &entry) || !entry.value)
			return -EINVAL;
		value_str = entry.value;
		for (i = 0; i < len && value_str; i++) {
			sscanf(value_str, "%d", &val);
			buf[i] = val & 0xFF;
			value_str = strchr(value_str, ' ');
			if (value_str)
				value_str++;
		}
		kfree(entry.key);
		kfree(entry.value);
		return 0;
	}
#endif
	tmp = kmalloc(sizeof(u32) * len, GFP_ATOMIC);
	if (tmp == NULL)
		return -ENOMEM;
	rc = of_property_read_u32_array(pdev->dev.of_node, name, tmp, len);
	if (rc != 0)
		goto done;
	for (i = 0; i < len; i++)
		buf[i] = tmp[i] & 0xFF;

done:
	kfree(tmp);
	return rc;
}

static void mdio_mgmt_set(struct apm_enet_pdev *pdev)
{
	if (enet_dev[MENET]) {
		pdev->hw_config = MENET;
	} else {
		int index;
		for (index = ENET_0; index < MENET; index++) {
			if (enet_dev[index]) {
				struct apm_enet_priv *p =
					&enet_dev[index]->priv;
				if (p->phy_mode != PHY_MODE_XGMII) {
					pdev->hw_config = index;
					break;
				}
			}
		}
	}
	if (pdev->priv.port == pdev->hw_config)
		mutex_init(&pdev->phy_lock);
	return;
}

static int apm_enet_of_probe(struct platform_device *platform_device)
{
	u8  ethaddr[6] = {0, 0, 0, 0, 0, 0};
	u32 phy_id = 0x20;
	int rc = 0;
#ifndef CONFIG_ARCH_MSLIM
	u32 enet_irq = 0, enet_mac_irq = 0, enet_qmi_irq = 0;
#endif
	u32 csr_addr_size, port_id, node_val;
	u64 port_addr_p, enet_gbl_base_addr_p;
	struct net_device *ndev;
	struct mii_bus *mdio_bus;
	struct apm_enet_pdev *pdev;
	struct apm_enet_priv *priv;
	struct resource *res;
	void *enet_mii_access_base_addr, *enet_gbl_base_addr, *port_addr;
#ifdef XGENE_NET_CLE
	u32 cid;
#endif
#if defined(IPV4_TSO)
	u32 hw_vlan = 0;
#endif
	u8 phy_mode;
	char pm[16];
	u32 num_tx_queues = 0, num_rx_queues = 0;
	u32 mac_to_mac = 0, desired_speed = 0, phyless = 0;
	u32 force_serdes_reset = 0, std_alone = 0;
	struct xgene_qmtm_sdev *sdev;
	char name[32];
	u8 wq_pbn_start, wq_pbn_count, fq_pbn_start, fq_pbn_count;
	u32 qmtm_ip;
	u8 info[5];

#if defined(CONFIG_ACPI)
	/* Skip the ACPI probe if booting via DTS */
	if (!efi_enabled(EFI_BOOT) && platform_device->dev.of_node == NULL)
		return -ENODEV;
#endif
	/* Retrieve Device ID for this port */
	rc = apm_enet_get_u32_param(platform_device, "devid", &port_id);
	if (rc || port_id >= MAX_ENET_PORTS) {
		ENET_ERROR("No device ID or invalid value in DTS %d\n",
			port_id);
		return -EINVAL;
	}

	/* Retrieve Enet Port CSR register address and size */
	res = platform_get_resource(platform_device, IORESOURCE_MEM, 0);
	if (!res) {
		ENET_ERROR("Unable to retrive Enet csr addr from DTS\n");
		return rc;
	}
	port_addr_p = res->start;
	csr_addr_size = RES_SIZE(res);
	port_addr = devm_ioremap(&platform_device->dev, res->start,
				 resource_size(res));
	ENET_DEBUG("CSR PADDR: 0x%llx VADDR: 0x%p\n",
		port_addr_p, port_addr);

	/* Retrieve Enet Global CSR register address and size */
	res = platform_get_resource(platform_device, IORESOURCE_MEM, 1);
	if (!res) {
		ENET_ERROR("Unable to retrive Enet Global csr addr from DTS\n");
		return rc;
	}
	enet_gbl_base_addr_p = res->start;
	csr_addr_size = RES_SIZE(res);
	enet_gbl_base_addr = devm_ioremap(&platform_device->dev, res->start,
					  resource_size(res));

	ENET_DEBUG("Enet Global PADDR: 0x%llx VADDR: 0x%p\n",
		   enet_gbl_base_addr_p, enet_gbl_base_addr);

#ifdef XGENE_NET_CLE
	/* Derive Classifier CSR register address */
	cid = PID2CID[port_id];
	apm_class_base_addr_p[cid] =
		enet_gbl_base_addr_p + BLOCK_ETH_CLE_OFFSET;
	apm_class_base_addr[cid] = enet_gbl_base_addr + BLOCK_ETH_CLE_OFFSET;
	ENET_DEBUG("Classifier PADDR: 0x%llx VADDR: 0x%p\n",
			(u64)apm_class_base_addr_p[cid],
			apm_class_base_addr[cid]);
#endif

	/* Retrieve Enet MII access register address and size */
	res = platform_get_resource(platform_device, IORESOURCE_MEM, 2);
	if (!res) {
		ENET_ERROR("Unable to retrive Enet MII access addr"
			"from DTS\n");
		return rc;
	}

	enet_mii_access_base_addr = devm_ioremap(&platform_device->dev,
						 res->start,
						 resource_size(res));

	ENET_DEBUG("Enet MII Access PADDR: 0x%llx  VADDR: 0x%p\n",
		   (u64)res.start, enet_mii_access_base_addr);

	apm_enet_get_str_param(platform_device, "slave_name", name,
			       sizeof(name));
	apm_enet_get_byte_param(platform_device, "slave_info", info,
				ARRAY_SIZE(info));
	qmtm_ip = info[0];
	wq_pbn_start = info[1];
	wq_pbn_count = info[2];
	fq_pbn_start = info[3];
	fq_pbn_count = info[4];

	if (!(sdev = xgene_qmtm_set_sdev(name, qmtm_ip,
			wq_pbn_start, wq_pbn_count,
			fq_pbn_start, fq_pbn_count))) {
		ENET_ERROR("QMTM%d Slave %s error\n", qmtm_ip, name);
		return -ENODEV;
	}

#ifndef CONFIG_ARCH_MSLIM
	/* Retrieve ENET Error IRQ number */
	if (!(enet_irq = platform_get_irq(platform_device, 0))) {
		ENET_ERROR("Unable to retrive ENET Error"
			"IRQ number from DTS\n");
		return -EINVAL;
	}
	ENET_DEBUG("Enet Error IRQ no: 0x%x\n", enet_irq);

	/* Retrieve ENET MAC Error IRQ number */
	if (!(enet_mac_irq = platform_get_irq(platform_device, 1))) {
		ENET_ERROR("Unable to retrive ENET MAC Error IRQ"
			" number from DTS\n");
		return -EINVAL;
	}
	ENET_DEBUG("Enet MAC Error IRQ no: 0x%x\n", enet_mac_irq);

	/* Retrieve ENET QMI Error IRQ number */
	if (!(enet_qmi_irq = platform_get_irq(platform_device, 2))) {
		ENET_ERROR("Unable to retrive ENET QMI Error IRQ"
			" number from DTS\n");
		return -EINVAL;
	}
	ENET_DEBUG("Enet QMI Error IRQ no: 0x%x\n", enet_qmi_irq);
#endif

	/* Retrieve PHY ID for this port */
 	rc = apm_enet_get_u32_param(platform_device, "phyid", &phy_id);
	if (rc || phy_id > 0x1F) {
		ENET_ERROR("No phy ID or invalid value in DTS\n");
		return -EINVAL;
	}

	rc = apm_enet_get_str_param(platform_device, "phy-mode", pm,
				    ARRAY_SIZE(pm));
	if (rc) {
		ENET_ERROR("No phy-mode in DTS\n");
		return -EINVAL;
	}

	if (!strncasecmp(pm, "xgmii", 5)) {
		phy_mode = PHY_MODE_XGMII;
	}
	else if (!strncasecmp(pm, "sgmii", 5)) {
		phy_mode = PHY_MODE_SGMII;
	}
	else if (!strncasecmp(pm, "rgmii", 5)) {
		phy_mode = PHY_MODE_RGMII;
	}
	else {
		ENET_ERROR("Invalid phy-mode value in DTS\n");
		return -EINVAL;
	}

	if (phy_mode == PHY_MODE_XGMII) {
		num_tx_queues = MAX_TX_QUEUES;
		num_rx_queues = MAX_RX_QUEUES;
	} else {
		num_tx_queues = 1;
		num_rx_queues = 1;
	}

	ndev = alloc_etherdev_mqs(sizeof(struct apm_enet_pdev),
		num_tx_queues, num_rx_queues);

	if (!ndev) {
		ENET_ERROR("Not able to allocate memory for netdev\n");
		return -ENOMEM;
	}

	pdev = (struct apm_enet_pdev *) netdev_priv(ndev);
	enet_dev[port_id] = pdev;
	priv = &pdev->priv;
	pdev->sdev = sdev;
	pdev->ndev = ndev;
	pdev->num_tx_queues = num_tx_queues;
	pdev->num_rx_queues = num_rx_queues;
	pdev->platform_device = platform_device;
	pdev->node = platform_device->dev.of_node;
#if defined(SET_RX_MODE) 
	INIT_LIST_HEAD(&pdev->mcast_head);
	INIT_LIST_HEAD(&pdev->ucast_head);
#endif
	SET_NETDEV_DEV(ndev, &platform_device->dev);
	dev_set_drvdata(&platform_device->dev, pdev);

	priv->paddr_base = enet_gbl_base_addr_p;
	priv->ppaddr_base = port_addr_p;

	if (phy_mode == PHY_MODE_SGMII) {
		rc = apm_enet_get_u32_param(platform_device, "mac-to-mac",
					&mac_to_mac);
		if (!rc)
			priv->mac_to_mac = mac_to_mac;
		rc = apm_enet_get_u32_param(platform_device, "desired-speed",
					&desired_speed);
		if (!rc)
			priv->desired_speed = desired_speed;
		rc = apm_enet_get_u32_param(platform_device, "phyless",
					&phyless);
		if (!rc)
			priv->phyless = phyless;
		rc = apm_enet_get_u32_param(platform_device,
				"force-serdes-reset", &force_serdes_reset);
		if (!rc)
			priv->force_serdes_reset = force_serdes_reset;
		rc = apm_enet_get_u32_param(platform_device, "std-alone",
					&std_alone);
		if (!rc)
			priv->std_alone = std_alone;
	}

	priv->port = port_id;
	switch (port_id) {
	case XGENET_0:
	case XGENET_1:
	case XGENET_2:
	case XGENET_3:
		apm_xgenet_init_priv(priv, port_addr,
				enet_gbl_base_addr, enet_mii_access_base_addr);
		break;
	default:
		apm_enet_init_priv(priv, port_addr,
				enet_gbl_base_addr, enet_mii_access_base_addr);
		break;
	}

	node_val = -1;
	rc = apm_enet_get_u32_param(platform_device, "rx-fifo-cnt", &node_val);
	if (node_val >= 32 && node_val <= 512)
		pdev->rx_buff_cnt = node_val;
	else
		pdev->rx_buff_cnt = APM_NO_PKT_BUF;

	node_val = -1;
	rc = apm_enet_get_u32_param(platform_device, "ipg", &node_val);
	if (!rc)
		pdev->ipg = node_val;
	else
		pdev->ipg = 0;

	node_val = -1;
	rc = apm_enet_get_u32_param(platform_device, "wka_flag", &node_val);
	if (rc)
		pdev->wka_flag = node_val;
	else
		pdev->wka_flag = 1;	/* Default to use MDIO re-read */

	priv->phy_addr = phy_id;
	priv->crc = 0;

#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT))
		pdev->clk = NULL;
	else
#endif
	{
		pdev->clk = clk_get(&platform_device->dev, NULL);
		if (!IS_ERR(pdev->clk))
			clk_prepare_enable(pdev->clk);
		else
			dev_err(&platform_device->dev, "can't get clock\n");
	}

	apm_enet_port_reset(priv, phy_mode);

	/*
	 * To ensure no packet enters the system, disable Rx/Tx before
	 * configure the inline classifier
	 */
	apm_enet_mac_tx_state(priv, 0);
	apm_enet_mac_rx_state(priv, 0);

#ifdef XGENE_NET_CLE
	/* Initialize CLE Inline Engine */
	if ((rc = apm_cle_init(port_id)) != 0) {
		ENET_ERROR("Error in apm_cle_init\n");
		return rc;
	}
#endif

	ndev->netdev_ops = &apm_dnetdev_ops;

#if defined(IPV4_TX_CHKSUM) || defined(IPV4_TSO)
	ndev->features |= NETIF_F_IP_CSUM;
#endif
#if defined(IPV4_RX_CHKSUM) || defined(IPV4_TSO)
	pdev->features |= FLAG_RX_CSUM_ENABLED;
#endif

#if defined(IPV4_TSO)
	ndev->features |= NETIF_F_TSO | NETIF_F_SG;
	pdev->mss = DEFAULT_TCP_MSS;
	apm_enet_tx_offload(priv, APM_ENET_MSS0, pdev->mss);

	rc = apm_enet_get_u32_param(platform_device, "hw_vlan", &hw_vlan);
	if (rc && hw_vlan == 1)
			ndev->features |= NETIF_F_HW_VLAN_CTAG_TX;
#endif
	pdev->vlan_tci = 0;

#if defined(IPV4_GRO)
	ndev->features |= NETIF_F_GRO;
#endif
#if defined(IPV4_GSO)
	ndev->features |= NETIF_F_GSO;
#endif

	/* Ethtool checks the capabilities/features in hw_features flag */
	ndev->hw_features = ndev->features;
	SET_ETHTOOL_OPS(ndev, &apm_ethtool_ops);

	if ((rc = register_netdev(ndev))) {
		ENET_ERROR("enet%d: failed to register net dev(%d)!\n",
				apm_enet_get_port(pdev), rc);
		return rc;
	}

	rc = apm_enet_get_mac_param(platform_device, "local-mac-address",
				    ethaddr);
	if (rc) {
		ENET_ERROR("Can't get Device MAC address\n");
		eth_hw_addr_random(ndev);
	} else {
		for (rc = 0; rc < ETH_ALEN; rc++)
			ndev->dev_addr[rc] = ethaddr[rc] & 0xff;
	}

	ENET_DEBUG("%s: emac%d MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
		ndev->name, apm_enet_get_port(pdev),
		ndev->dev_addr[0], ndev->dev_addr[1], ndev->dev_addr[2],
		ndev->dev_addr[3], ndev->dev_addr[4], ndev->dev_addr[5]);

	/* QM configuration */
	if ((rc = apm_enet_qconfig(pdev))) {
		ENET_ERROR("Error in QM configuration\n");
		return rc;
	}

	/* Add NAPI for RX/TX */
	apm_enet_napi_add_all(pdev);

	/* Initialize PreClassifier Tree for this port */
	ENET_DEBUG("Initialize Preclassifier Tree for port %d\n", port_id);
#ifndef XGENE_NET_CLE
	apm_enet_cle_bypass(priv, QMTM_QUEUE_ID(pdev->sdev->qmtm_ip,
		pdev->qm_queues.default_rx_qid),
		pdev->qm_queues.default_rx_fp_pbn-0x20,
		pdev->qm_queues.default_rx_nxtfp_pbn-0x20, true);
#else
	apm_preclass_init(port_id, &pdev->qm_queues);

#ifdef SET_RX_MODE
	apm_preclass_update_mac(port_id, TYPE_SYS_MACADDR, BROADCAST_MACADDR,
				apm_sys_macmask[BROADCAST_MACADDR],
				apm_sys_macaddr[BROADCAST_MACADDR]);
	apm_preclass_update_mac(port_id, TYPE_SYS_MACADDR, ETHERNET_MACADDR,
				apm_sys_macmask[ETHERNET_MACADDR],
				ndev->dev_addr);
#endif
	/* Start Preclassifier Engine for this port */
	ENET_DEBUG("Start Preclassifier for port %d\n", port_id);
	apm_preclass_switch_tree(port_id, CLE_PTREE_DEFAULT, 0);
#endif

	/* Default MAC initialization */
	apm_enet_mac_init(priv, ndev->dev_addr, SPEED_1000, priv->crc);
	/* Store the mac addr into perm_addr as this will be used by
	   ethtool while getting the settings */
	memcpy(ndev->perm_addr, ndev->dev_addr, ndev->addr_len);

	if (((priv->phy_mode == PHY_MODE_SGMII) && (priv->phy_addr == INT_PHY_ADDR)) ||
		(priv->phy_mode == PHY_MODE_XGMII))
		goto _skip_mdio;

	/* Ensure Rx & Tx are disabled in MAC.
	 * MDIO Link state changes will enable/disable Rx & Tx in MAC
	 */
	apm_enet_mac_tx_state(priv, 0);
	apm_enet_mac_rx_state(priv, 0);
	mdio_mgmt_set(pdev);

	/* Setup MDIO bus */
	mdio_bus = mdiobus_alloc();
	if (!mdio_bus) {
		ENET_ERROR("Not able to allocate memory for MDIO bus\n");
		return -ENOMEM;
	}

	pdev->mdio_bus = mdio_bus;
	mdio_bus->name = "APM Ethernet MII Bus";
	mdio_bus->read = &apm_enet_mdio_read;
	mdio_bus->write = &apm_enet_mdio_write;
	mdio_bus->reset = &apm_enet_mdio_reset;
	snprintf(mdio_bus->id, MII_BUS_ID_SIZE, "%x", port_id);
	mdio_bus->priv = pdev;
	mdio_bus->parent = &ndev->dev;
	mdio_bus->phy_mask = ~(1 << priv->phy_addr);
	if ((rc = mdiobus_register(mdio_bus)) != 0) {
		ENET_ERROR("enet%d: failed to register MDIO bus(%d)!\n",
				apm_enet_get_port(pdev), rc);
	} else {
		rc = apm_enet_mdio_probe(ndev);
	}

_skip_mdio:
	/* Set InterPacket Gap if greater than 0 (existed in DTS) */
	if (pdev->ipg > 0)
		apm_enet_mac_set_ipg(priv, pdev->ipg);

#ifndef CONFIG_ARCH_MSLIM
	/* Request for IRQ interrupt */
	pdev->enet_err_irq = enet_irq;
	pdev->enet_mac_err_irq = enet_mac_irq;
	pdev->enet_qmi_err_irq = enet_qmi_irq;
#endif

	/* Register Ethernet Management interrupts */
	apm_enet_register_irq(ndev);

	/* Unmask error IRQ regardless of port */
	apm_enet_unmask_int(priv);

	if (!pdev->mdio_bus) {
		mutex_init(&pdev->link_lock);
		INIT_DELAYED_WORK(&pdev->link_work, mac_link_timer);
	}

#ifdef CONFIG_ARCH_XGENE
	/* Configure the laser interface */
	if (priv->phy_mode == PHY_MODE_XGMII) {
	        struct i2c_adapter *i2c_adap;
		int i;

		for (i = 0; ; i++) {
			i2c_adap = i2c_get_adapter(i);
			if (!i2c_adap)
				break;
			if (strstr(i2c_adap->name, "SLIMPRO") ||
					strstr(i2c_adap->name, "SLIMpro") ||
					strstr(i2c_adap->name, "slimpro")) {
				pdev->i2c_adap = i2c_adap;
				break;
			}
		}
		apm_enet_cfg_xfi_laser(pdev, port_id - XGENET_0);
		apm_enet_xfi_laser_set(pdev, port_id - XGENET_0, LASER_ON);
	}
#endif

	return APM_RC_OK;
}

/* Called when module is unloaded */
static int apm_enet_of_remove(struct platform_device *platform_device)
{
	struct apm_enet_pdev *pdev;
	struct apm_enet_priv *priv;
	struct net_device *ndev;
	int port;
	u32 qindex;
	u8  qmtm_ip;

	ENET_DEBUG("Unloading Ethernet driver\n");

	pdev = platform_get_drvdata(platform_device);
	qmtm_ip = pdev->sdev->qmtm_ip;
	ndev = pdev->ndev;
	priv = &pdev->priv;

	port = apm_enet_get_port(pdev);

	/* Stop any traffic and disable MAC */
	apm_enet_mac_rx_state(priv, 0);
	apm_enet_mac_tx_state(priv, 0);

	if (netif_running(ndev)) {
		netif_device_detach(ndev);
		apm_enet_napi_disable_all(pdev);
	}

	apm_enet_napi_del_all(pdev);
	apm_enet_mdio_remove(ndev);

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
		if (pdev->qm_queues.rx_fp[qindex].qid > 0)
			apm_enet_void_fp(pdev->rx_skb_pool[qindex],
				pdev->qm_queues.rx_fp[qindex].qid,
			       	APM_ENET_REGULAR_FRAME);
#ifdef CONFIG_XGENE_NET_JUMBO_FRAME
		if (pdev->qm_queues.rx_nxtfp[qindex].qid > 0)
			apm_enet_void_fp(pdev->rx_page_pool[qindex],
				pdev->qm_queues.rx_nxtfp[qindex].qid,
				APM_ENET_JUMBO_FRAME);
#endif
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
			apm_enet_void_fp(pdev->hw_skb_pool[qindex],
				pdev->qm_queues.hw_fp[qindex].qid,
			       	APM_ENET_REGULAR_FRAME);
#endif
	}

	apm_enet_delete_queue(pdev);
	apm_enet_free_irq(ndev);

	for (qindex = 0; qindex < pdev->num_rx_queues; qindex++) {
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
		kfree(pdev->hw_skb_pool[qindex]->skb);
		kfree(pdev->hw_skb_pool[qindex]);
#endif
#ifdef CONFIG_XGENE_NET_JUMBO_FRAME
		kfree(pdev->rx_page_pool[qindex]);
#endif
		kfree(pdev->rx_skb_pool[qindex]->skb);
		kfree(pdev->rx_skb_pool[qindex]);
		kfree(pdev->rx[qindex]);
	}
	for (qindex = 0; qindex < pdev->num_tx_queues; qindex++) {
#ifdef CONFIG_XGENE_NET_IPV4_FORWARD_OFFLOAD
		kfree(pdev->hw[qindex]);
#endif
		kfree(pdev->tx[qindex]->msg8);
		kfree(pdev->tx[qindex]);
	}

	for (qindex = 0; qindex < pdev->num_tx_completion_queues; qindex++)
		kfree(pdev->tx_completion[qindex]);

	if (pdev->tx_completion) {
		kfree(pdev->tx_completion);
		pdev->tx_completion = NULL;
	}

#ifdef CONFIG_ARCH_XGENE
	if (pdev->i2c_adap)
		i2c_put_adapter(pdev->i2c_adap);
#endif

	unregister_netdev(ndev);
	apm_enet_port_shutdown(priv);

	apm_enet_iounmap(priv);
	free_netdev(ndev);

	return APM_RC_OK;
}

static struct of_device_id apm_enet_match[] = {
	{
		.compatible     = "xgene,enet",
	},
	{},
};
MODULE_DEVICE_TABLE(of, apm_enet_match);

#ifdef CONFIG_ACPI
static const struct acpi_device_id apm_enet_acpi_ids[] = {
        { "APMC0D05", 0 },
        { }
};
MODULE_DEVICE_TABLE(acpi, apm_enet_acpi_ids);
#endif

static struct platform_driver apm_enet_driver = {
	.driver = {
		.name = APM_ENET_DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = apm_enet_match,
#ifdef CONFIG_ACPI
		.acpi_match_table = ACPI_PTR(apm_enet_acpi_ids),
#endif
	},
	.probe = apm_enet_of_probe,
	.remove = apm_enet_of_remove,
};

static int __init apm_net_init(void)
{
	int i = 0;

	ENET_DEBUG("Creating proc entry\n");
	if (!proc_create(APM_ENET_DRIVER_NAME, 0, NULL, &apm_enet_driver_fops)) {
		ENET_ERROR(APM_ENET_DRIVER_NAME " init failed\n");
		return -1;
	}

	/* Initialize global structure */
	for (i = 0; i < MAX_ENET_PORTS; i++)
		enet_dev[i] = NULL;

#ifdef XGENE_NET_CLE
	/* Initialize Classifier global data structure */
	for (i = 0; i < MAX_CLE_ENGINE; i++) {
		apm_class_base_addr_p[i] = 0;
		apm_class_base_addr[i] = NULL;
	}

	/* For AMP Systems, we are partitioning cle resoures */
	apm_cle_system_id = CORE_0;
	apm_cle_systems = 1;
#endif
	i = platform_driver_register(&apm_enet_driver);
#if defined(CONFIG_SYSFS) && defined(CONFIG_APM_EEE)
	if (i == 0)
		i = apm_enet_add_sysfs(&apm_enet_driver.driver);
#endif
	if (i == 0)
		ENET_PRINT("APM88xxxx Ethernet Driver v%s loaded\n",
			APM_ENET_DRIVER_VERSION);

	return i;
}

static void __exit apm_net_exit(void)
{
#if defined(CONFIG_SYSFS) && defined(CONFIG_APM_EEE)
	apm_enet_remove_sysfs(&apm_enet_driver.driver);
#endif
	remove_proc_entry(APM_ENET_DRIVER_NAME, NULL);
	platform_driver_unregister(&apm_enet_driver);
	ENET_PRINT("APM88xxxx Ethernet Drvier v%s unloaded\n",
		APM_ENET_DRIVER_VERSION);
}

EXPORT_SYMBOL(enet_dev);
EXPORT_SYMBOL(netmap_open);
EXPORT_SYMBOL(netmap_close);
EXPORT_SYMBOL(apm_enet_clr_pb);
EXPORT_SYMBOL(apm_enet_mac_tx_state);
EXPORT_SYMBOL(apm_enet_mac_rx_state);
EXPORT_SYMBOL(apm_enet_cle_bypass);
module_init(apm_net_init);
module_exit(apm_net_exit);

MODULE_AUTHOR("Keyur Chudgar <kchudgar@apm.com>");
MODULE_DESCRIPTION("APM X-Gene Ethernet driver");
MODULE_LICENSE("GPL");
