/**
 * APM X-Gene PCIe hostplug driver
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 *
 * Author: Tanmay Inamdar <tinamdar@apm.com>
 * 	   Mayuresh Chitale <mchitale@apm.com>
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
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define XGENE_MAX_PCIE_PORTS		5
#define PCIECORE_CTLANDSTATUS		0x50
#define BRIDGE_CFG_31			0x202c
#define BRIDGE_STATUS_0			0x2600
#define LINK_UP_MASK			0x100
#define XGENE_LTSSM_DETECT_WAIT		100 /* ms */
#define XGENE_POST_LINK_WAIT		1000 /* ms */
#define XGENE_LTSSM_L0_WAIT		4  /* ms */
#define XGENE_LTSSM_OFFSET		0x4C


void xgene_pcie_reset_port_err(void __iomem *csr_base);

enum {
	SUCCESS = 0,
	LINK_FAILURE,
	HW_FAILURE
};

struct xgene_pcie_hp {
	struct list_head	node;
	struct pci_bus		*bus;
	void __iomem		*base;
};

LIST_HEAD(xgene_hp_list);
struct proc_dir_entry *xgene_hp_proc;
extern void __iomem *xgene_pcie_get_port_csr(struct pci_bus *bus);
extern void xgene_pcie_set_link_status(struct pci_bus *bus, u8 link);
extern int xgene_pcie_get_link_status(struct pci_bus *bus);


static struct xgene_pcie_hp *
xgene_pcie_get_domain_to_hp(int domain)
{
	struct xgene_pcie_hp *xgene_hp;

	list_for_each_entry(xgene_hp, &xgene_hp_list, node) {
		struct pci_bus *bus = xgene_hp->bus;
		if (pci_domain_nr(bus) == domain)
			return xgene_hp;
	}
	return NULL;
}

/**
 * pcibios_remove_pci_devices - remove all devices under this bus
 * @bus: the indicated PCI bus
 *
 * Remove all of the PCI devices under this bus both from the
 * linux pci device tree, and from the powerpc EEH address cache.
 */
static void xgene_pcie_remove_pci_devices(struct pci_bus *bus)
{
	struct pci_dev *dev, *tmp;
	struct pci_bus *child_bus;

	/* First go down child busses */
	list_for_each_entry(child_bus, &bus->children, node)
		xgene_pcie_remove_pci_devices(child_bus);

	pr_debug("PCI: Removing devices on bus %04x:%02x\n",
			pci_domain_nr(bus),  bus->number);
	list_for_each_entry_safe(dev, tmp, &bus->devices, bus_list) {
		pr_debug("   Removing %s...\n", pci_name(dev));
		pci_stop_and_remove_bus_device(dev);
	}
}

static void xgene_pcie_add_pci_devices(struct pci_bus *bus)
{
	struct pci_bus *b = NULL;

	while ((b = pci_find_next_bus(b)) != NULL)
		pci_rescan_bus(b);
}

static int xgene_pcie_poll_linkup(void __iomem *csr_base)
{
        u32 val32;

        /*
         * A component enters the LTSSM Detect state within
         * 20ms of the end of fundamental core reset.
         */
        msleep(XGENE_LTSSM_DETECT_WAIT);
	val32 = readl(csr_base + XGENE_LTSSM_OFFSET);
	val32 = readl(csr_base + PCIECORE_CTLANDSTATUS);
	return ((val32 & LINK_UP_MASK) >> 8);
}

int xgene_pcie_enable_link(struct pci_bus *bus)
{
	void __iomem *base = xgene_pcie_get_port_csr(bus);
	u32 val;
	u32 ret;

	if (!base)
		return -HW_FAILURE;

	ret = xgene_pcie_get_link_status(bus);
	if (ret) {
		pr_debug ("Link already enabled\n");
		return SUCCESS;
	}
	/* trigger link up */
	val = readl(base + BRIDGE_CFG_31);
	val &= ~(1 << 20);
	writel(val, base + BRIDGE_CFG_31);

	ret = xgene_pcie_poll_linkup(base);
	xgene_pcie_set_link_status(bus, ret);
	if (ret) {
		msleep(XGENE_POST_LINK_WAIT);
		xgene_pcie_add_pci_devices(bus);
		return SUCCESS;
	} else
		return -LINK_FAILURE;
}
EXPORT_SYMBOL(xgene_pcie_enable_link);

int xgene_pcie_disable_link(struct pci_bus *bus)
{
	void __iomem *base = xgene_pcie_get_port_csr(bus);
	u32 val;
	int ret;

	if (!base)
		return -HW_FAILURE;

 	ret = xgene_pcie_get_link_status(bus);
	if (!ret) {
		pr_debug ("Link already disabled\n");
		return SUCCESS;
	}
	xgene_pcie_remove_pci_devices(bus);
	val = readl(base + BRIDGE_CFG_31);
	val |= 1 << 20;
	writel(val, base + BRIDGE_CFG_31);

	ret = xgene_pcie_poll_linkup(base);
	xgene_pcie_set_link_status(bus, ret);
	if (!ret) {
		xgene_pcie_reset_port_err(base);
		return SUCCESS;
	}
	else
		return -LINK_FAILURE;
}
EXPORT_SYMBOL(xgene_pcie_disable_link);

static ssize_t xgene_pcie_hp_enable(struct file *file,
		const char __user *buf, size_t count, loff_t *data)
{
	struct xgene_pcie_hp *pcie_hp;
	unsigned long domain;
	static char temp[5];
	char *str;
	int ret;

	str = &temp[0];
	if (copy_from_user(str, buf, count))
		return -EFAULT;

	domain = simple_strtoul(str, NULL, 0);
	if (domain < 0 || domain > XGENE_MAX_PCIE_PORTS)
		return -EINVAL;

	pcie_hp = xgene_pcie_get_domain_to_hp(domain);
	if (!pcie_hp) {
		pr_err("no hotplug entry for port %lu\n", domain);
		return -EINVAL;
	}

	ret = xgene_pcie_enable_link(pcie_hp->bus);
	if (ret == SUCCESS)
		pr_info("Link enable successful!\n");
	else
		pr_info("Link enable failed!\n");
	return count;
}

static ssize_t xgene_pcie_hp_disable(struct file *file,
		const char __user *buf, size_t count, loff_t *data)
{
	struct xgene_pcie_hp *xgene_hp;
	unsigned long domain;
	static char temp[5];
	char *str;
	int ret;

	str = &temp[0];
	if (copy_from_user(str, buf, count))
		return -EFAULT;

	domain = simple_strtoul(str, NULL, 0);
	if (domain < 0 || domain > XGENE_MAX_PCIE_PORTS)
		return -EINVAL;

	xgene_hp = xgene_pcie_get_domain_to_hp(domain);
	if (!xgene_hp) {
		pr_err("no hotplug entry for port %lu\n", domain);
		return -EINVAL;
	}

	ret = xgene_pcie_disable_link(xgene_hp->bus);
	if (ret == SUCCESS)
		pr_info("Link disable successful!\n");
	else
		pr_info("Link disable failed!\n");
	return count;
}

static const struct file_operations enable_fops = {
	.owner = THIS_MODULE,
	.write = xgene_pcie_hp_enable,
};

static const struct file_operations disable_fops = {
	.owner = THIS_MODULE,
	.write = xgene_pcie_hp_disable,
};

int xgene_pcie_hp_init(void)
{
	struct xgene_pcie_hp *xgene_hp;
	struct proc_dir_entry *entry;
	struct pci_bus *bus;
	int num_root_buses = 0;

	pr_info("xgene pcie hotplug driver\n");

	list_for_each_entry(bus, &pci_root_buses, node) {
		if (num_root_buses++ == XGENE_MAX_PCIE_PORTS) {
			pr_warn("more root buses that we support!\n");
			break;
		}
		xgene_hp = kzalloc(sizeof(*xgene_hp), GFP_KERNEL);
		xgene_hp->bus = bus;
		xgene_hp->base = xgene_pcie_get_port_csr(bus);
		list_add_tail(&xgene_hp->node, &xgene_hp_list);
	}

	xgene_hp_proc = proc_mkdir("xgene_pcie_hotplug", NULL);
	if (!xgene_hp_proc) {
		pr_warn("failed to create proc dir\n");
		return 0;
	}
	entry = proc_create("enable", S_IRWXUGO, xgene_hp_proc, &enable_fops);
	printk("/proc/xgene_pcie_hotplug/enable created!\n");

	entry = proc_create("disable", S_IRWXUGO, xgene_hp_proc,
			    &disable_fops);
	printk("/proc/xgene_pcie_hotplug/disable created!\n");

	return 0;
}

static void xgene_pcie_hp_exit(void)
{
	struct xgene_pcie_hp *xgene_hp, *temp;

	list_for_each_entry_safe(xgene_hp, temp, &xgene_hp_list, node) {
		list_del(&xgene_hp->node);
		kfree(xgene_hp);
	}
}
module_init(xgene_pcie_hp_init);
module_exit(xgene_pcie_hp_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Tanmay Inamdar <tinamdar@apm.com>");
MODULE_DESCRIPTION("X-Gene PCIe Hotplug driver");

