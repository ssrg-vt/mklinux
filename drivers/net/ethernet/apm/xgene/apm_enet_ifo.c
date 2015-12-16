/**
 * AppliedMicro X-Gene SoC Ethernet IPv4 Forward Offload Implementation
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * All rights reserved. Hrishikesh Karanjikar <hkaranjikar@apm.com>
 *                      Khuong Dinh <kdinh@apm.com>
 *                      Ravi Patel <rapatel@apm.com>
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
 * @file apm_enet_ifo.c
 *
 * This file implements X-Gene SoC Ethernet Inline Classifier
 * configuration for IPv4 Forward Offload
 *
 */

#include "apm_enet_ifo.h"
#include <misc/xgene/cle/apm_cle_mgr.h>
#include <misc/xgene/cle/apm_cle_config.h>
#include <linux/inet.h>

struct ipv4_fwd_ctx {
	struct apm_enet_pdev *src_pdev;
	struct apm_enet_pdev *dst_pdev;
	__be32 src_ip;
	__be32 dst_ip;
	u8 src_mac[6];
	u8 dst_mac[6];
};

#define APM_ENET_IFO_NODE "apm_enet_ifo_node"
#define APM_ENET_IFO_STATE "apm_enet_ifo_state"

static u32 apm_enet_ifo_state[MAX_ENET_PORTS];
static void apm_enet_ifo_node_help(void);
static void apm_enet_ifo_state_help(void);

static struct apm_ptree_config *apm_enet_ifo_ptree(u32 eth_port)
{
	struct apm_ptree_config *ptree_config;
	struct ptree_kn kn;
	struct ptree_branch branch[] = {
		{ 0xFFFF, 0x0000, EQT, PTREE_ALLOC(0), EW_BRANCH(1),
			32, 0, JMP_ABS, JMP_FW },	/* IP Addr 0-1 bytes */
		{ 0xFFFF, 0x0000, EQT, PTREE_ALLOC(1), EW_BRANCH(0),
			0, 0, JMP_ABS, JMP_FW },	/* IP Addr 2-3 bytes */
		{ 0xFFFF, 0x0000, EQT, PTREE_ALLOC(2), KEY_INDEX(0),
			0, 0, 0, 0 },			/* Last Node */
	};
	struct ptree_dn dn[] = {
		{ START_NODE, DBPTR_DROP(0), AVL_SEARCH(BOTH_BYTES), 0,
			0, 0, 2, &branch[0] },
		{ LAST_NODE,  DBPTR_DROP(0), AVL_SEARCH(NO_BYTE), 0,
			0, 0, 1, &branch[2] },
	};
	struct ptree_node node[] = {
		{ PTREE_ALLOC(0), EWDN, 0, (struct ptree_dn *)&dn[0] },
		{ PTREE_ALLOC(1), EWDN, 0, (struct ptree_dn *)&dn[1] },
		{ PTREE_ALLOC(2), KN,   0, (struct ptree_kn *)&kn },
	};

	/* check for existing default configuration */
	ptree_config = apm_find_ptree_config(eth_port, CLE_PTREE_DEFAULT);
	if (ptree_config == NULL) {
		ENET_ERROR("Port %d is down.\n", eth_port);
		goto _ret_ifo_ptree;
	}

	kn.priority = 2;
	kn.result_pointer = ptree_config->start_dbptr;

	/* Add IPv4 fwd offload tree config */
	ptree_config = apm_add_ptree_config(eth_port,
				CLE_PTREE_IPV4FWD);
	if (ptree_config == NULL) {
		ENET_ERROR("Error adding Patricia Tree for IPv4 "
				"Forward offload for port %d\n",
				eth_port);
		goto _ret_ifo_ptree;
	}
	/* Set the start packet pointer to the location where destination
	 * IP addr starts. i.e. 30th byte
	 */
	ptree_config->start_pkt_ptr = 30;

	/* Default result is our dbptr index */
	ptree_config->default_result = DFCLSRESDBPRIORITY0_WR(1) |
					DFCLSRESDBPTR0_WR(START_DB_INDEX);
	ENET_DEBUG_OFFLOAD("\n default result = %d\n",
			ptree_config->default_result);

	/* Allocate the patricia tree in patricia tree ram */
	ENET_DEBUG_OFFLOAD("\n Create Patricia Tree Nodes for IPv4 Fwd Tree\n");
	if (apm_ptree_alloc(eth_port, ARRAY_SIZE(node), 0, node, NULL,
			ptree_config) != APM_RC_OK) {
		ENET_ERROR("Preclass init error for port %d\n", eth_port);
		apm_del_ptree_config(eth_port, CLE_PTREE_IPV4FWD);
		ptree_config = NULL;
	}

_ret_ifo_ptree:
	apm_enet_ifo_node_help();
	return ptree_config;
}

static int apm_enet_ifo_set_state(struct apm_enet_pdev *pdev, u32 enable)
{
	struct apm_ptree_config *ptree_config = NULL;
	u32 eth_port = apm_enet_get_port(pdev);
	int ret = APM_RC_ERROR;

	if (enable) {
		/* check for existing ipv4fwd ptree configuration */
		ptree_config = apm_find_ptree_config(eth_port,
				CLE_PTREE_IPV4FWD);
		ENET_DEBUG_OFFLOAD("\nipv4fwd_ptree_config %p", ptree_config);

		/* If not created then create ipv4fwd ptree */
		if (!ptree_config)
			ptree_config = apm_enet_ifo_ptree(eth_port);

		/* If created then switch to ipv4fwd ptree */
		if (ptree_config) {
			ret = apm_preclass_switch_tree(eth_port,
					CLE_PTREE_IPV4FWD, 0);
			ENET_DEBUG_OFFLOAD("\nSwitching to ipv4fwd tree rc=%d\n", ret);
		}
	} else {
		/* Switch to default ptree */
		ret = apm_preclass_switch_tree(eth_port,
				CLE_PTREE_DEFAULT, 0);
		ENET_DEBUG_OFFLOAD("\nSwitching to default tree\n");
	}

	/* Store if offload enabled for that port or not */
	if (ret == APM_RC_OK)
		apm_enet_ifo_state[eth_port] = enable ? (1 + pdev->ndev->ifindex) : 0;

	return ret;
}

static int apm_enet_ifo_add_node(struct ipv4_fwd_ctx *fwd_ctx)
{
	int rc = 0;
	struct apm_enet_pdev *src_pdev = fwd_ctx->src_pdev;
	u32 src_port = apm_enet_get_port(src_pdev);
	u32 dst_port = apm_enet_get_port(fwd_ctx->dst_pdev);
	u64 H0Info_lsb = 0;
	u16 dstqid;
	struct avl_node node;
	struct apm_cle_dbptr dbptr;
	u32 hw_index;

	memset(&dbptr, 0, sizeof(dbptr));
	ENET_DEBUG_OFFLOAD("\n %s\n", __func__);
	ENET_DEBUG_OFFLOAD("\n src_dev_name = %s\n", src_pdev->ndev->name);
	ENET_DEBUG_OFFLOAD("\n dst_dev_name = %s\n", fwd_ctx->dst_pdev->ndev->name);
	ENET_DEBUG_OFFLOAD("\n src_port = %u\n", src_port);
	ENET_DEBUG_OFFLOAD("\n dst_port = %u\n", dst_port);
	ENET_DEBUG_OFFLOAD("\n dst Mac  = %x:%x:%x:%x:%x:%x",
			fwd_ctx->dst_mac[0], fwd_ctx->dst_mac[1],
			fwd_ctx->dst_mac[2], fwd_ctx->dst_mac[3],
			fwd_ctx->dst_mac[4], fwd_ctx->dst_mac[5]);
	ENET_DEBUG_OFFLOAD("\n dst_ip = %x\n", fwd_ctx->dst_ip);

	/* Set the destination qid as default rx qid for this port */
	hw_index = src_pdev->hw_index;
	dstqid = QMTM_QUEUE_ID(src_pdev->qm_queues.qm_ip,
			src_pdev->qm_queues.rx[hw_index].qid);
	ENET_DEBUG_OFFLOAD("\n dbptr dstqid = %d\n", dstqid);
	dbptr.dstqidL = dstqid & 0x7f;
	dbptr.dstqidH = (dstqid >> 7) & 0x1f;

	/* Set free pool selection as default rx free pool for this port */
	dbptr.fpsel = src_pdev->qm_queues.hw_fp[hw_index].pbn - 0x20;
	ENET_DEBUG_OFFLOAD("\n dbptr fpsel = %d\n", dbptr.fpsel);

	src_pdev->hw_index = (hw_index + 1) % src_pdev->num_rx_queues;
	/* Offload to perform */
	dbptr.H0FPSel = 1;

	/* Write destination mac. This will be stored in qm up16 message
	 as H0Info_lsbL and H0Info_lsbH and then while offloading it will be
	 written in packet */

	/* Store MAC Address as per endianess to avoid hton for LE */
#ifdef CONFIG_CPU_BIG_ENDIAN
	H0Info_lsb |= 0xff & fwd_ctx->dst_mac[0];
	H0Info_lsb <<= 8;
	H0Info_lsb |= 0xff & fwd_ctx->dst_mac[1];
	H0Info_lsb <<= 8;
	H0Info_lsb |= 0xff & fwd_ctx->dst_mac[2];
	H0Info_lsb <<= 8;
	H0Info_lsb |= 0xff & fwd_ctx->dst_mac[3];
	H0Info_lsb <<= 8;
	H0Info_lsb |= 0xff & fwd_ctx->dst_mac[4];
	H0Info_lsb <<= 8;
	H0Info_lsb |= 0xff & fwd_ctx->dst_mac[5];
#else
	H0Info_lsb |= 0xff & fwd_ctx->dst_mac[1];
	H0Info_lsb <<= 8;
	H0Info_lsb |= 0xff & fwd_ctx->dst_mac[0];
	H0Info_lsb <<= 8;
	H0Info_lsb |= 0xff & fwd_ctx->dst_mac[5];
	H0Info_lsb <<= 8;
	H0Info_lsb |= 0xff & fwd_ctx->dst_mac[4];
	H0Info_lsb <<= 8;
	H0Info_lsb |= 0xff & fwd_ctx->dst_mac[3];
	H0Info_lsb <<= 8;
	H0Info_lsb |= 0xff & fwd_ctx->dst_mac[2];
#endif

	ENET_DEBUG_OFFLOAD("\n dbptr H0Info_lsb = %012llx\n", H0Info_lsb);
	dbptr.H0Info_lsb0 = H0Info_lsb & 0x7fff;
	H0Info_lsb >>= 15;
	dbptr.H0Info_lsb1 = H0Info_lsb & 0xffffffff;
	H0Info_lsb >>= 32;
	dbptr.H0Info_lsb2 = H0Info_lsb & 0x1;

	/* Store dst port */
	dbptr.H0Enq_Num = dst_port;
	dbptr.index = DBPTR_ALLOC(0);

	memset(&node, 0, sizeof(node));
	node.search_key[0] = fwd_ctx->dst_ip;
	node.priority = 0;
	node.result_pointer = DBPTR_ALLOC(0);

	ENET_DEBUG_OFFLOAD("\n avl node search_key = %x\n", node.search_key[0]);
	ENET_DEBUG_OFFLOAD("\n avl node result_pointer = %d\n", node.result_pointer);

	if (apm_avl_alloc(src_port, 1, 1, &node, &dbptr)) {
		ENET_PRINT("apm_avl_alloc failed !! for port %d\n", src_port);
		return APM_RC_ERROR;
	}

	ENET_DEBUG_OFFLOAD("added AVL Entry at node %d using result pointer %d for port %d\n",
		node.index, node.result_pointer, src_port);

	return rc;
}

int apm_enet_ifo_perform(struct xgene_qmtm_msg32 *msg32_1,
		struct sk_buff *skb, struct apm_enet_qcontext *c2e)
{
	struct xgene_qmtm_msg16 *emsg16 =
		(struct xgene_qmtm_msg16 *)&c2e->msg32[c2e->index];
	struct xgene_qmtm_msg_up16 *emsgup16 =
		(struct xgene_qmtm_msg_up16 *)&emsg16[1];
	struct xgene_qmtm_msg16 *msg16 = &msg32_1->msg16;
	struct xgene_qmtm_msg_up16 *msgup16 = &msg32_1->msgup16;
	struct ipv4_fwd_hdr {
		u16 da16;
		u32 da32;
		u16 sa16;
		u32 sa32;
		u8  res[10];
		u8  ttl;
		u8  protocol;
		__be16 checksum;
		__be32 saddr;
		__be32 daddr;
	} __packed;
	struct ipv4_fwd_hdr *skb_data, *mac_data;

	skb_data = (struct ipv4_fwd_hdr *)skb->data;
	mac_data = (struct ipv4_fwd_hdr *)c2e->pdev->ndev->dev_addr;
	ENET_DEBUG_OFFLOAD("\n Inside packet da16 = %x da32 = %x\n",
			skb_data->da16, skb_data->da32);
	/* Set the destination MAC address as given by user */
	skb_data->da32 = msgup16->H0Info_lsbL;
	skb_data->da16 = msgup16->H0Info_lsbH;
	ENET_DEBUG_OFFLOAD("\n H0Info_lsb = %04x%08x\n", msgup16->H0Info_lsbH,
			msgup16->H0Info_lsbL);
	ENET_DEBUG_OFFLOAD("\n H0Info_msb = %04x%08x\n", msgup16->H0Info_msbH,
			msgup16->H0Info_msbL);
	ENET_DEBUG_OFFLOAD("\n%s %d After modification da16 %x da32 %x",
			__func__, __LINE__,	skb_data->da16, skb_data->da32);

	/* Setting SA as interface 'port' MAC Address */
	skb_data->sa16 = (u16)mac_data->da16;
	skb_data->sa32 = (u32)mac_data->da32;
	ENET_DEBUG_OFFLOAD("\n%s %d Src addr sa16 %x sa32 %x",
			__func__, __LINE__, skb_data->sa16, skb_data->sa32);

	/* Decrementing TTL by 1*/
	skb_data->ttl--;

	memset(emsg16, 0, 32);
	emsg16->UserInfo = msg16->UserInfo;
	emsg16->C = xgene_qmtm_coherent();
	emsg16->BufDataLen = msg16->BufDataLen - 4;
	emsg16->FPQNum = msg16->FPQNum;
	emsg16->PB = 0;
	emsg16->HB = 1;
	emsg16->DataAddrH = msg16->DataAddrH;
	emsg16->DataAddrL = msg16->DataAddrL;
#define QMTM_MAX_QUEUE_ID 511
	emsgup16->HR = 1;
	emsgup16->H0Enq_Num = QMTM_QUEUE_ID(c2e->pdev->sdev->qmtm_ip, QMTM_MAX_QUEUE_ID);
	emsgup16->H0Info_lsbH = (TYPE_SEL_WORK_MSG << 12) | (TSO_INS_CRC_ENABLE << 3);
	if (skb_data->protocol == IPPROTO_TCP)
		emsgup16->H0Info_lsbL = (TSO_IPPROTO_TCP << 24) |
				(TSO_CHKSUM_ENABLE << 22) |
				((0xe & TSO_ETH_HLEN_MASK) << 12) |
				((0x5 & TSO_IP_HLEN_MASK) << 6) |
				(0x5 & TSO_TCP_HLEN_MASK);
	else if (skb_data->protocol == IPPROTO_UDP)
		emsgup16->H0Info_lsbL = (TSO_IPPROTO_UDP << 24) |
				(TSO_CHKSUM_ENABLE << 22) |
				((0xe & TSO_ETH_HLEN_MASK) << 12) |
				((0x5 & TSO_IP_HLEN_MASK) << 6) |
				(0x2 & TSO_TCP_HLEN_MASK);
	else
		emsgup16->H0Info_lsbL = (TSO_CHKSUM_ENABLE << 22) |
				((0xe & TSO_ETH_HLEN_MASK) << 12) |
				((0x5 & TSO_IP_HLEN_MASK) << 6);

	xgene_qmtm_msg_le32(&(((u32 *)emsg16)[1]), 7);

	/* Forward the packet */
	writel(1, c2e->command);
	if (++c2e->index == c2e->count)
		c2e->index = 0;

	return 0;
}
EXPORT_SYMBOL(apm_enet_ifo_perform);

static void apm_enet_ifo_node_help(void)
{
	ENET_PRINT("Perform offload using following command.\n\n echo"
		" <src_port> <dest_port> <dest_mac> <dest_ip> /proc/%s\n\n"
		" Packet having dest_ip is forwarded from src_port to "
		" dest_port and dest_mac is the mac of next hop"
		" where src_port/dest_port is name of eth interface:\n"
		" \t e.g. ethx where x is 0,1,2...\n"
		"    dest_mac:\n"
		" \t Destination MAC address e.g. 00:02:11:22:33:44\n"
		"    dest_ip:\n"
		" \t Destination IP address e.g. 10:10:12:15\n",
		APM_ENET_IFO_NODE);
}

static ssize_t apm_enet_ifo_node_write(struct file *file,
		const char __user *buf,
		size_t count, loff_t *ppos)
{
	char *buffer = (char *)buf;
	char *tok;
	struct ipv4_fwd_ctx fwd_ctx;
	struct net_device *ndev = NULL;

	tok = strsep(&buffer, " ");
	if (tok == NULL)
		goto __ret_err;

	ENET_DEBUG_OFFLOAD("\n src_dev_name = %s\n", tok);
	ndev = dev_get_by_name(&init_net, tok);
	fwd_ctx.src_pdev = netdev_priv(ndev);
	ENET_DEBUG_OFFLOAD("\n src_port = %d\n", apm_enet_get_port(fwd_ctx.src_pdev));

	tok = strsep(&buffer, " ");
	if (tok == NULL)
		goto __ret_err;

	ENET_DEBUG_OFFLOAD("\n dst_dev_name = %s\n", tok);
	ndev = dev_get_by_name(&init_net, tok);
	fwd_ctx.dst_pdev = netdev_priv(ndev);
	ENET_DEBUG_OFFLOAD("\n dst_port = %d\n", apm_enet_get_port(fwd_ctx.dst_pdev));

	tok = strsep(&buffer, " ");
	if (tok == NULL)
		goto __ret_err;

	sscanf(tok, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
			&fwd_ctx.dst_mac[0],
			&fwd_ctx.dst_mac[1],
			&fwd_ctx.dst_mac[2],
			&fwd_ctx.dst_mac[3],
			&fwd_ctx.dst_mac[4],
			&fwd_ctx.dst_mac[5]);
	ENET_DEBUG_OFFLOAD("\n Mac Addr %x:%x:%x:%x:%x:%x",
			fwd_ctx.dst_mac[0], fwd_ctx.dst_mac[1],
			fwd_ctx.dst_mac[2], fwd_ctx.dst_mac[3],
			fwd_ctx.dst_mac[4], fwd_ctx.dst_mac[5]);

	tok = strsep(&buffer, " ");
	if (tok == NULL)
		goto __ret_err;

	fwd_ctx.dst_ip = htonl(in_aton(tok));
	ENET_DEBUG_OFFLOAD("\n dst_ip = %x\n", fwd_ctx.dst_ip);

	/* Init the ipv4 offload i.e. init cle on port no given. */
	apm_enet_ifo_add_node(&fwd_ctx);

	return count;

__ret_err:
	apm_enet_ifo_node_help();
	return count;
}

static ssize_t apm_enet_ifo_node_read(struct file *file,
		char __user *buf,
		size_t count, loff_t *ppos)
{
	apm_enet_ifo_node_help();
	return 0;
}

const struct file_operations apm_enet_ifo_node_fops = {
	.owner = THIS_MODULE,
	.read = apm_enet_ifo_node_read,
	.write = apm_enet_ifo_node_write,
};

static void apm_enet_ifo_state_help(void)
{
	ENET_PRINT("Enable ipv4 offload for ethernet port as follows:\n\n"
			"echo eth_port 0/1 /proc/%s\n\n"
			"eth_port - Ethernet port. e.g. eth0\n"
			"0 - Disable IPv4 offload\n"
			"1 - Enable IPv4 offload\n",
			APM_ENET_IFO_STATE);
}

static ssize_t apm_enet_ifo_state_write(struct file *file,
		const char __user *buf,
		size_t count, loff_t *ppos)
{
	char *buffer = (char *)buf;
	char *tok;
	struct apm_enet_pdev *pdev = NULL;
	struct net_device *ndev = NULL;
	u32 enable = 0;

	tok = strsep(&buffer, " ");
	if (tok == NULL)
		goto __ret_err;

	ENET_DEBUG_OFFLOAD("\n eth_dev_name = %s\n", tok);

	ndev = dev_get_by_name(&init_net, tok);
	pdev = netdev_priv(ndev);

	if (pdev == NULL)
		goto __ret_err;

	tok = strsep(&buffer, " ");
	if (tok == NULL)
		goto __ret_err;

	enable = simple_strtol(tok, NULL, 10);

	ENET_DEBUG_OFFLOAD("\n apm_enet_ifo_state = %d\n",
			enable);

	if (enable < 0 || enable > 1)
		goto __ret_err;

	/* Enable disable ipv4 offload */
	apm_enet_ifo_set_state(pdev, enable);

	return count;

__ret_err:
	apm_enet_ifo_state_help();
	return count;
}

static ssize_t apm_enet_ifo_state_read(struct file *file,
		char __user *buf,
		size_t count, loff_t *ppos)
{
	int i = 0;

	ENET_PRINT("\nOffload enable status for ethernet ports:\n\n");

	for (i = 0; i < MAX_ENET_PORTS; i++) {
		if (apm_enet_ifo_state[i]) {
			struct net_device *ndev = dev_get_by_index(&init_net,
				apm_enet_ifo_state[i] - 1);
			struct apm_enet_pdev *pdev = netdev_priv(ndev);

			ENET_PRINT("%s: port %d\n", ndev->name,
				apm_enet_get_port(pdev));
		}
	}

	ENET_PRINT("\n\n");
	apm_enet_ifo_state_help();
	return 0;
}

const struct file_operations apm_enet_ifo_state_fops = {
	.owner = THIS_MODULE,
	.read = apm_enet_ifo_state_read,
	.write = apm_enet_ifo_state_write,
};

static int __init apm_enet_ifo_init(void)
{
	ENET_DEBUG("Creating proc entries for IP Forwarding\n");
	if (!proc_create(APM_ENET_IFO_NODE, 0, NULL,
				&apm_enet_ifo_node_fops)) {
		ENET_ERROR(APM_ENET_IFO_NODE
				"init failed\n");
		return -1;
	}
	if (!proc_create(APM_ENET_IFO_STATE, 0, NULL,
				&apm_enet_ifo_state_fops)) {
		ENET_ERROR(APM_ENET_IFO_STATE
				"init failed\n");
		return -1;
	}
	return 0;
}

static void __exit apm_enet_ifo_exit(void)
{
	remove_proc_entry(APM_ENET_IFO_NODE, NULL);
	remove_proc_entry(APM_ENET_IFO_STATE, NULL);
}

module_init(apm_enet_ifo_init);
module_exit(apm_enet_ifo_exit);

MODULE_AUTHOR("Hrishikesh Karanjikar <hkaranjikar@apm.com>");
MODULE_DESCRIPTION("APM X-Gene Ethernet IPv4 forward offload driver");
MODULE_LICENSE("GPL");
