/**
 * AppliedMicro X-Gene Ethernet Tool Driver
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * All rights reserved. Keyur Chudgar <kchudgar@apm.com>
 * 			Hrishikesh Karanjikar <hkaranjikar@apm.com>
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
 * @file apm_enet_tools.c
 *
 * Ethtool functions for X-Gene Ethernet subsystem
 *
 **
 */

#include <linux/mii.h>
#include <linux/phy.h>
#include "apm_enet_mac.h"
#include "apm_enet_csr.h"
#include "apm_enet_access.h"
#include "apm_enet_tools.h"

const struct ethtool_ops apm_ethtool_ops = {
	.get_settings = apm_ethtool_get_settings,
	.set_settings = apm_ethtool_set_settings,
	.get_drvinfo = apm_ethtool_get_drvinfo,
	.nway_reset = apm_ethtool_nway_reset,
	.get_pauseparam = apm_ethtool_get_pauseparam,
	.set_pauseparam = apm_ethtool_set_pauseparam,
	.get_rx_csum = apm_ethtool_get_rx_csum,
	.get_tx_csum = apm_ethtool_get_tx_csum,
	.set_rx_csum = apm_ethtool_set_rx_csum,
	.set_tx_csum = apm_ethtool_set_tx_csum,
	.get_sg = apm_ethtool_get_sg,
	.set_sg = apm_ethtool_set_sg,
	.get_tso = apm_ethtool_get_tso,
	.set_tso = apm_ethtool_set_tso,
	.get_ethtool_stats = apm_ethtool_get_ethtool_stats,
	.get_sset_count = apm_get_sset_count,
	.get_strings = apm_get_strings,
	.get_link = ethtool_op_get_link,
};
EXPORT_SYMBOL(apm_ethtool_ops);

struct apm_stats {
	char stat_string[ETH_GSTRING_LEN];
	int sizeof_stat;
	int stat_offset;
};

#define APM_STAT(m) FIELD_SIZEOF(struct apm_enet_pdev, m), \
	offsetof(struct apm_enet_pdev, m)
		      
static const struct apm_stats apm_gstrings_stats[] = {
	{ "rx_packets", APM_STAT(stats.rx_stats.rx_packet_count) },
	{ "rx_bytes", APM_STAT(stats.rx_stats.rx_byte_count) },
	{ "tx_packets", APM_STAT(stats.tx_stats.tx_packet_count) },
	{ "tx_bytes", APM_STAT(stats.tx_stats.tx_byte_count) },
	{ "rx_drop", APM_STAT(stats.rx_stats.rx_drop_pkt_count) },
	{ "tx_drop", APM_STAT(stats.tx_stats.tx_drop_frm_count) },
	{ "c_64B", APM_STAT(stats.eth_combined_stats.c_64B_frames) },
	{ "c_65_127B",
		APM_STAT(stats.eth_combined_stats.c_65_127B_frames) },
	{ "c_128_255B",
		APM_STAT(stats.eth_combined_stats.c_128_255B_frames) },
	{ "c_256_511B",
		APM_STAT(stats.eth_combined_stats.c_256_511B_frames) },
	{ "c_512_1023B",
		APM_STAT(stats.eth_combined_stats.c_512_1023B_frames) },
	{ "c_1024_1518B",
		APM_STAT(stats.eth_combined_stats.c_1024_1518B_frames) },
	{ "c_1519_1522B",
		APM_STAT(stats.eth_combined_stats.c_1519_1522B_frames) },
	{ "rx_multicast", APM_STAT(stats.rx_stats.rx_multicast_pkt_count) },
	{ "rx_broadcast", APM_STAT(stats.rx_stats.rx_broadcast_pkt_count) },
	{ "rx_cntrl", APM_STAT(stats.rx_stats.rx_cntrl_frame_pkt_count) },
	{ "rx_pause", APM_STAT(stats.rx_stats.rx_pause_frame_pkt_count) },
	{ "rx_unknown_op",
		APM_STAT(stats.rx_stats.rx_unknown_op_pkt_count)},
	{ "rx_fcs_err", APM_STAT(stats.rx_stats.rx_fcs_err_count) },
	{ "rx_alignment_err",
		APM_STAT(stats.rx_stats.rx_alignment_err_pkt_count) },
	{ "rx_frm_len_err",
		APM_STAT(stats.rx_stats.rx_frm_len_err_pkt_count) },
	{ "rx_code_err", APM_STAT(stats.rx_stats.rx_code_err_pkt_count) }, 
	{ "rx_carrier_sense_err",
		APM_STAT(stats.rx_stats.rx_carrier_sense_err_pkt_count) },
	{ "rx_undersize", APM_STAT(stats.rx_stats.rx_undersize_pkt_count) },
	{ "rx_oversize", APM_STAT(stats.rx_stats.rx_oversize_pkt_count) },
	{ "rx_fragment", APM_STAT(stats.rx_stats.rx_fragment_count) },
	{ "rx_jabber", APM_STAT(stats.rx_stats.rx_jabber_count) },
	{ "tx_multicast", APM_STAT(stats.tx_stats.tx_multicast_pkt_count) },
	{ "tx_broadcast", APM_STAT(stats.tx_stats.tx_broadcast_pkt_count) },
	{ "tx_pause", APM_STAT(stats.tx_stats.tx_pause_frame_count) },
	{ "tx_deferra", APM_STAT(stats.tx_stats.tx_deferral_pkt_count) },
	{ "tx_exesiv_def",
		APM_STAT(stats.tx_stats.tx_exesiv_def_pkt_count) },
	{ "tx_single_coll",
		APM_STAT(stats.tx_stats.tx_single_coll_pkt_count) },
	{ "tx_multi_coll",
		APM_STAT(stats.tx_stats.tx_multi_coll_pkt_count) },
	{ "tx_late_coll", APM_STAT(stats.tx_stats.tx_late_coll_pkt_count) }, 
	{ "tx_exesiv_coll",
		APM_STAT(stats.tx_stats.tx_exesiv_coll_pkt_count) },
	{ "tx_toll_coll", APM_STAT(stats.tx_stats.tx_toll_coll_pkt_count) },
	{ "tx_pause_hon", APM_STAT(stats.tx_stats.tx_pause_frm_hon_count) },
	{ "tx_jabber", APM_STAT(stats.tx_stats.tx_jabber_frm_count) },
	{ "tx_fcs_err", APM_STAT(stats.tx_stats.tx_fcs_err_frm_count) },
	{ "tx_control", APM_STAT(stats.tx_stats.tx_control_frm_count) },
	{ "tx_oversize", APM_STAT(stats.tx_stats.tx_oversize_frm_count) },
	{ "tx_undersize", APM_STAT(stats.tx_stats.tx_undersize_frm_count) },
	{ "tx_fragments", APM_STAT(stats.tx_stats.tx_fragments_frm_count) },
};

#define APM_GLOBAL_STATS_LEN ARRAY_SIZE(apm_gstrings_stats)

#define XGMII_SUPPORTED_MASK	(ADVERTISED_10000baseT_Full | \
				SUPPORTED_FIBRE)

/* Ethtool APIs */

int apm_ethtool_get_settings(struct net_device *ndev,
		struct ethtool_cmd *cmd)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct phy_device *phydev = pdev->phy_dev;
	struct apm_enet_priv *priv = &pdev->priv;

	if (priv->phy_mode != PHY_MODE_XGMII) {
		if (!phydev)
			return -ENODEV;

		phy_ethtool_gset(phydev, cmd);

		if (!netif_carrier_ok(ndev) || !netif_running(ndev)) {
			cmd->duplex = -1;
			cmd->speed = -1;
		}
	} else {
		cmd->port = PORT_FIBRE;
		cmd->phy_address = priv->phy_addr;
		cmd->transceiver = XCVR_EXTERNAL;

		cmd->supported = cmd->advertising = XGMII_SUPPORTED_MASK;

		cmd->autoneg = AUTONEG_DISABLE;

		if (netif_running(ndev)) {
			if (apm_enet_get_link_status(priv)) {
				cmd->duplex = DUPLEX_FULL;
				cmd->speed = priv->speed;
			} else {                       
				cmd->duplex = -1;
				cmd->speed = -1;
			}
		} else {
			cmd->duplex = -1;
			cmd->speed = -1;
		}
	}
	return 0;
}

int apm_ethtool_set_settings(struct net_device *ndev,
		struct ethtool_cmd *cmd)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct phy_device *phydev = pdev->phy_dev;
	struct apm_enet_priv *priv = &pdev->priv;

	if (priv->phy_mode != PHY_MODE_XGMII) {
		if (!phydev)
			return -ENODEV;

		if (cmd->duplex == DUPLEX_HALF) {
			return -EINVAL;
		}
		/* We do not support half duplex mode. So set duplex mode as
		 * full always even if user has not specified anything */
		cmd->duplex = DUPLEX_FULL;

		return phy_ethtool_sset(phydev, cmd);
	} else {
		if ((cmd->autoneg == AUTONEG_ENABLE) || 
				(cmd->speed != APM_ENET_SPEED_10000) ||
				(cmd->duplex != DUPLEX_FULL)) {
			return -EINVAL;
		}
	}
	return 0;
}

int apm_ethtool_set_pauseparam(struct net_device *ndev,
		struct ethtool_pauseparam *pp)
{
	u32 data;
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct apm_enet_priv *priv = &pdev->priv;

	apm_enet_read(priv, BLOCK_MCX_MAC, MAC_CONFIG_1_ADDR, &data);

	/* Modify value to set or reset rx flow control */
	if (pp->rx_pause)
		data |= RX_FLOW_EN1_MASK;
	else
		data &= ~RX_FLOW_EN1_MASK;

	/* Modify value to set or reset tx flow control */
	if (pp->tx_pause)
		data |= TX_FLOW_EN1_MASK;
	else
		data &= ~TX_FLOW_EN1_MASK;

	apm_enet_write(priv, BLOCK_MCX_MAC,MAC_CONFIG_1_ADDR, data);

	return 0;
}

void apm_ethtool_get_pauseparam(struct net_device *ndev,
		struct ethtool_pauseparam *pp)
{
	u32 data;
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct apm_enet_priv *priv = &pdev->priv;

	apm_enet_read(priv, BLOCK_MCX_MAC, MAC_CONFIG_1_ADDR, &data);
	pp->rx_pause = RX_FLOW_EN1_RD(data);

	apm_enet_read(priv, BLOCK_MCX_MAC, MAC_CONFIG_1_ADDR, &data);
	pp->tx_pause = TX_FLOW_EN1_RD(data);
}

int apm_ethtool_set_rx_csum(struct net_device *ndev, u32 set)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);

	if (set)
		pdev->features |= FLAG_RX_CSUM_ENABLED;
	else
		pdev->features &= ~FLAG_RX_CSUM_ENABLED;

	return 0;
}

inline u32 apm_ethtool_get_rx_csum(struct net_device * ndev)
{
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	return (pdev->features & FLAG_RX_CSUM_ENABLED);
}

inline int apm_ethtool_set_tx_csum(struct net_device *ndev, u32 set)
{
	if (set)
		ndev->features |= NETIF_F_IP_CSUM;
	else
		ndev->features &= ~NETIF_F_IP_CSUM;

	return 0;
}

inline u32 apm_ethtool_get_tx_csum(struct net_device * ndev)
{
	if (ndev->features & NETIF_F_IP_CSUM)
		return 1;
	else
		return 0;
}

inline u32 apm_ethtool_get_sg(struct net_device * ndev)
{
	if (ndev->features & NETIF_F_SG)
		return 1;
	else
		return 0;
}

int apm_ethtool_set_sg(struct net_device *ndev, u32 set)
{
	if (set)
		ndev->features |= NETIF_F_SG;
	else
		ndev->features &= ~NETIF_F_SG;
	return 0;
}

int apm_ethtool_nway_reset(struct net_device *ndev)
{
	u32 data = 0, retry = 0;
	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	struct apm_enet_priv *priv = &pdev->priv;
	struct ethtool_cmd cmd;

	if (priv->phy_mode == PHY_MODE_XGMII)
		return 0;

	if (netif_running(ndev)) {

		/* Get current device settings */
		apm_ethtool_get_settings(ndev, &cmd);

		mutex_lock(&pdev->mdio_bus->mdio_lock);

		/* Power-down PHY */
		data = MII_CR_POWER_DOWN;
		apm_genericmiiphy_write(priv, priv->phy_addr, 
				MII_CTRL_REG, data);

		/* Power-up PHY */
		data = 0x0;
		apm_genericmiiphy_write(priv, priv->phy_addr, 
				MII_CTRL_REG, data);

		/* Reset PHY */
		data = MII_CR_RESET;
		apm_genericmiiphy_write(priv, priv->phy_addr, 
				MII_CTRL_REG, data);

		/* Wait till PHY reset completes */
		retry = 1000;
		while ((data & MII_CR_RESET) && (retry > 0)) {
			apm_genericmiiphy_read(priv, priv->phy_addr, 
					MII_CTRL_REG, &data);
			retry--;
			msleep(1);
		}

		mutex_unlock(&pdev->mdio_bus->mdio_lock);

		/* Set default settings after reset */
		cmd.autoneg = AUTONEG_ENABLE;
		cmd.speed = APM_ENET_SPEED_1000;
		cmd.duplex = DUPLEX_FULL;
		apm_ethtool_set_settings(ndev, &cmd);

		/* Configure the mac acordingly */
		priv->autoneg_set = AUTONEG_ENABLE;
		priv->speed = APM_ENET_SPEED_1000; 
		apm_gmac_init(priv, ndev->dev_addr, priv->speed, priv->crc);

		if (pdev->ipg > 0)
			apm_enet_mac_set_ipg(priv, pdev->ipg); 	

	}

	return 0;
}

void apm_get_strings(struct net_device *ndev, u32 stringset,
		    u8 *data)
{
	u8 *p = data;
	int i;
	
	switch (stringset) {
	case ETH_SS_TEST:
	case ETH_SS_STATS:
		for (i = 0; i < APM_GLOBAL_STATS_LEN; i++) {
			memcpy(p, apm_gstrings_stats[i].stat_string,
					ETH_GSTRING_LEN);
			p += ETH_GSTRING_LEN;
		}
		break;
	}
}

int apm_get_sset_count(struct net_device *ndev, int sset)
{
	switch (sset) {
	case ETH_SS_TEST:
		return APM_GLOBAL_STATS_LEN; 
	case ETH_SS_STATS:
		return APM_GLOBAL_STATS_LEN; 
	default:
		return -EOPNOTSUPP;
	}
	
}

void apm_ethtool_get_ethtool_stats(struct net_device *ndev,
		struct ethtool_stats *ethtool_stats,
		u64 *data)
{

	struct apm_enet_pdev *pdev = netdev_priv(ndev);
	int i;

	apm_enet_stats(ndev);
	for (i = 0; i < APM_GLOBAL_STATS_LEN; i++) {
		char *p = (char *)pdev + apm_gstrings_stats[i].stat_offset;
		data[i] = (apm_gstrings_stats[i].sizeof_stat ==
				sizeof(u64)) ? *(u64 *)p : *(u32 *)p;
	}
}

void apm_ethtool_get_drvinfo(struct net_device *ndev,
		struct ethtool_drvinfo *info)
{
	strcpy(info->driver, ndev->name);
	strcpy(info->version, DRV_VERSION);
	strcpy(info->fw_version, "N/A");
}

u32 apm_ethtool_get_tso(struct net_device *ndev)
{
	if (ndev->features & NETIF_F_TSO)
		return 1;
	else
		return 0;
}

int apm_ethtool_set_tso(struct net_device *dev, u32 set)
{
	if (set)
		dev->features |= NETIF_F_TSO;
	else
		dev->features &= ~NETIF_F_TSO;

	return 0;
}
