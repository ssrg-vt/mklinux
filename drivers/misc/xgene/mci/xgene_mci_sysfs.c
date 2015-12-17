/*
 * AppliedMicro X-Gene MCI sysfs Interface
 *
 * Copyright (c) 2014 Applied Micro Circuits Corporation.
 * Author: 	Hoan Tran <hotran@apm.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#if defined(CONFIG_SYSFS)
#include <linux/slab.h>
#include <linux/stat.h>
#include <misc/xgene/mci/xgene_mci_sysfs.h>
#include <misc/xgene/mci/xgene_mci_core.h>

static ssize_t show_mci_receive(struct device_driver *drv, char *buf)
{
	return sprintf(buf, "QM%d receives message\n", TX_QMTM);
}

static ssize_t sys_mci_receive(struct device_driver *drv, const char *buf, size_t count)
{
	return 0;
}

static ssize_t show_mci_send(struct device_driver *drv, char *buf)
{
	return sprintf(buf, "MCI driver \n");
}

static ssize_t sys_mci_send(struct device_driver *drv, const char *buf, size_t count)
{
	return count;
}

static ssize_t show_mci_open(struct device_driver *drv, char *buf)
{
	return sprintf(buf, "MCI open function \n");
}

static ssize_t sys_mci_open(struct device_driver *drv, const char *buf, size_t count)
{
	char name[8];
	sscanf(buf, "%s", name);

	xgene_mci_open(name, NULL, MCI_POLLING, NULL, NULL);

	return count;
}

static ssize_t show_mci_close(struct device_driver *drv, char *buf)
{
	return sprintf(buf, "MCI close function \n");
}

static ssize_t sys_mci_close(struct device_driver *drv, const char *buf, size_t count)
{
	return count;
}

static ssize_t show_mci_client_status(struct device_driver *drv, char *buf)
{
	int i;
	struct xgene_mci_client* client;
	struct xgene_mci_channel* chan0 = &(xgene_mci_get_dev()->channels[0]);
	struct xgene_mci_channel* chan1 = &(xgene_mci_get_dev()->channels[1]);
	int length = 0;

	/*Create labels */
	printk(KERN_INFO "Port\t Name\t Flag\t Tx\t TxAck\t Rx\t RxAck\n");

	for (i  = 0; i < XGENE_MCI_MAX_CLIENT; i++) {
		client = xgene_mci_get_client(xgene_mci_get_dev(),i);
		if (client)
		printk(KERN_INFO "%d\t %s\t %s\t %d\t %d\t %d\t %d\n",
				i,
				client->name,
				(client->flag == MCI_POLLING) ? "POLL": "ASYNC",
						client->tx_data_msg_count,
						client->tx_data_ack_msg_count,
						client->rx_data_msg_count,
						client->rx_data_ack_msg_count);
	}
	printk(KERN_INFO "MCI0 IRQ count = %d\n", chan0->irq_count);
	printk(KERN_INFO "MCI1 IRQ count = %d\n", chan1->irq_count);

	return length;
}

static struct driver_attribute xgene_mci_sysfs_attrs[] = {
#ifdef DMCI_DEBUG
	__ATTR(mci_send, S_IRUGO | S_IWUGO, show_mci_send, sys_mci_send),
	__ATTR(mci_receive, S_IRUGO | S_IWUGO, show_mci_receive, sys_mci_receive),
#endif
	__ATTR(mci_open, S_IRUGO | S_IWUGO, show_mci_open, sys_mci_open),
	__ATTR(mci_close, S_IRUGO | S_IWUGO, show_mci_close, sys_mci_close),
	__ATTR(mci_client_status, S_IRUGO, show_mci_client_status, NULL)
};

int add_mci_sysfs(struct device_driver *driver)
{
	int i;
	int err;

	for (i = 0; i < ARRAY_SIZE(xgene_mci_sysfs_attrs); i++) {
		err = driver_create_file(driver, &xgene_mci_sysfs_attrs[i]);
		if (err) goto fail0;
	}

	return 0;

fail0:
	while (--i >= 0) {
		driver_remove_file(driver, &xgene_mci_sysfs_attrs[i]);
	}
	return err;
}

void remove_mci_sysfs(struct device_driver *driver)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(xgene_mci_sysfs_attrs); i++) {
		driver_remove_file(driver, &xgene_mci_sysfs_attrs[i]);
	}
}
#else

#define add_mci_sysfs(drv) do { } while (0)
#define remove_mci_sysfs(drv) do { } while (0)

#endif /* CONFIG_SYSFS */
