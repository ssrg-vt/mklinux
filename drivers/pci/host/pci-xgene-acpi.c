/**
 * APM X-Gene ACPI PCIe Driver
 *
 * Copyright (c) 2014 Applied Micro Circuits Corporation.
 *
 * Author: Tuan Phan <tphan@apm.com>
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
 * Borrowed from x86 arch
 */

#include <linux/pci.h>
#include <linux/acpi.h>
#include <linux/efi.h>
#include <linux/pci-acpi.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/slab.h>

#define PCIE_CFG_SIZE 0x01000000

struct pci_root_info {
	struct acpi_device *bridge;
	char name[16];
	unsigned int res_num;
	struct resource *res;
	resource_size_t *res_offset;
	struct xgene_pcie_port sd;
};

static void coalesce_windows(struct pci_root_info *info, unsigned long type)
{
	int i, j;
	struct resource *res1, *res2;

	for (i = 0; i < info->res_num; i++) {
		res1 = &info->res[i];
		if (!(res1->flags & type))
			continue;

		for (j = i + 1; j < info->res_num; j++) {
			res2 = &info->res[j];
			if (!(res2->flags & type))
				continue;

			/*
			 * I don't like throwing away windows because then
			 * our resources no longer match the ACPI _CRS, but
			 * the kernel resource tree doesn't allow overlaps.
			 */
			if (resource_overlaps(res1, res2)) {
				res1->start = min(res1->start, res2->start);
				res1->end = max(res1->end, res2->end);
				dev_info(&info->bridge->dev,
			 "host bridge window expanded to %pR; %pR ignored\n",
					 res1, res2);
				res2->flags = 0;
			}
		}
	}
}

static void add_resources(struct pci_root_info *info,
			  struct list_head *resources)
{
	int i;
	struct resource *res, *root, *conflict;

	coalesce_windows(info, IORESOURCE_MEM);
	coalesce_windows(info, IORESOURCE_IO);

	for (i = 0; i < info->res_num; i++) {
		res = &info->res[i];

		if (res->flags & IORESOURCE_MEM)
			root = &iomem_resource;
		else if (res->flags & IORESOURCE_IO)
			root = &ioport_resource;
		else
			continue;

		conflict = insert_resource_conflict(root, res);
		if (conflict)
			dev_info(&info->bridge->dev,
		 "ignoring host bridge window %pR (conflicts with %s %pR)\n",
					res, conflict->name, conflict);
		else
			pci_add_resource_offset(resources, res,
					info->res_offset[i]);
	}
}

static void free_pci_root_info_res(struct pci_root_info *info)
{
	kfree(info->res);
	info->res = NULL;
	kfree(info->res_offset);
	info->res_offset = NULL;
	info->res_num = 0;
}

static void __release_pci_root_info(struct pci_root_info *info)
{
	int i;
	struct resource *res;

	for (i = 0; i < info->res_num; i++) {
		res = &info->res[i];

		if (!res->parent)
			continue;

		if (!(res->flags & (IORESOURCE_MEM | IORESOURCE_IO)))
			continue;

		release_resource(res);
	}

	free_pci_root_info_res(info);
	kfree(info);
}

static void release_pci_root_info(struct pci_host_bridge *bridge)
{
	struct pci_root_info *info = bridge->release_data;

	__release_pci_root_info(info);
}

static acpi_status
resource_to_addr(struct acpi_resource *resource,
			struct acpi_resource_address64 *addr)
{
	acpi_status status;
	struct acpi_resource_memory24 *memory24;
	struct acpi_resource_memory32 *memory32;
	
	memset(addr, 0, sizeof(*addr));
	switch (resource->type) {
	case ACPI_RESOURCE_TYPE_MEMORY24:
		memory24 = &resource->data.memory24;
		addr->resource_type = ACPI_MEMORY_RANGE;
		addr->minimum = memory24->minimum;
		addr->address_length = memory24->address_length;
		addr->maximum = addr->minimum + addr->address_length - 1;
		return AE_OK;
	case ACPI_RESOURCE_TYPE_MEMORY32:
		memory32 = &resource->data.memory32;
		addr->resource_type = ACPI_MEMORY_RANGE;
		addr->minimum = memory32->minimum;
		addr->address_length = memory32->address_length;
		addr->maximum = addr->minimum + addr->address_length - 1;
		return AE_OK;
	case ACPI_RESOURCE_TYPE_ADDRESS16:
	case ACPI_RESOURCE_TYPE_ADDRESS32:
	case ACPI_RESOURCE_TYPE_ADDRESS64:
		status = acpi_resource_to_address64(resource, addr);
		if (ACPI_SUCCESS(status) &&
		    (addr->resource_type == ACPI_MEMORY_RANGE ||
		    addr->resource_type == ACPI_IO_RANGE) &&
		    addr->address_length > 0) {
			return AE_OK;
		}
		break;
	}
	return AE_ERROR;
}

static acpi_status
count_resource(struct acpi_resource *acpi_res, void *data)
{
	struct pci_root_info *info = data;
	struct acpi_resource_address64 addr;
	acpi_status status;

	status = resource_to_addr(acpi_res, &addr);
	if (ACPI_SUCCESS(status))
		info->res_num++;

	return AE_OK;
}

static acpi_status
setup_resource(struct acpi_resource *acpi_res, void *data)
{
	struct pci_root_info *info = data;
	struct xgene_pcie_port *sd = &info->sd;
	struct resource *res;
	struct acpi_resource_address64 addr;
	acpi_status status;
	unsigned long flags;
	u64 start, end;
	
	if (acpi_res->type == ACPI_RESOURCE_TYPE_FIXED_MEMORY32) {
		sd->csr_base = devm_ioremap_nocache(sd->dev,
				acpi_res->data.fixed_memory32.address,
				acpi_res->data.fixed_memory32.address_length);
		if (IS_ERR_OR_NULL(sd->csr_base))
			return AE_ERROR;
		return AE_OK;
	}

	status = resource_to_addr(acpi_res, &addr);
	if (!ACPI_SUCCESS(status))
		return AE_OK;

	if (addr.resource_type == ACPI_MEMORY_RANGE) {
		flags = IORESOURCE_MEM;
		if (addr.info.mem.caching == ACPI_PREFETCHABLE_MEMORY)
			flags |= IORESOURCE_PREFETCH;
	} else if (addr.resource_type == ACPI_IO_RANGE)
		flags = IORESOURCE_IO;
	else
		return AE_OK;

	start = addr.minimum + addr.translation_offset;
	end = addr.maximum + addr.translation_offset;
	res = &info->res[info->res_num];
	res->name = info->name;
	res->flags = flags;
	res->start = start;
	res->end = end;

	if (flags & IORESOURCE_IO) {
		unsigned long port = -1;
		int err;
		
		err = pci_register_io_range(start, addr.address_length);
		if (err)
			return AE_OK;

		port = pci_address_to_pio(start);
		if (port == (unsigned long)-1) {
			res->start = (resource_size_t)OF_BAD_ADDR;
			res->end = (resource_size_t)OF_BAD_ADDR;
			return AE_OK;
		}

		res->start = port;
		res->end = res->start + addr.address_length - 1;

		if (pci_ioremap_io(res, start) < 0)
			return AE_OK;
	}

	info->res_offset[info->res_num] = res->start - addr.minimum;
	info->res_num++;

	return AE_OK;
}

static void
probe_pci_root_info(struct pci_root_info *info, struct acpi_device *device,
		    int busnum, int domain)
{
	size_t size;

	sprintf(info->name, "PCI Bus %04x:%02x", domain, busnum);
	info->bridge = device;
	info->res_num = 0;

	acpi_walk_resources(device->handle, METHOD_NAME__CRS, count_resource,
				info);
	if (!info->res_num)
		return;

	size = sizeof(*info->res) * info->res_num;
	info->res = kzalloc(size, GFP_KERNEL);
	if (!info->res) {
		info->res_num = 0;
		return;
	}

	size = sizeof(*info->res_offset) * info->res_num;
	info->res_num = 0;
	info->res_offset = kzalloc(size, GFP_KERNEL);
	if (!info->res_offset) {
		kfree(info->res);
		info->res = NULL;
		return;
	}

	acpi_walk_resources(device->handle, METHOD_NAME__CRS, setup_resource,
				info);
}

void pcibios_add_bus(struct pci_bus *bus)
{
	if (efi_enabled(EFI_BOOT))
		acpi_pci_add_bus(bus);
}

struct pci_bus *pci_acpi_scan_root(struct acpi_pci_root *root)

{
	struct acpi_device *device = root->device;
	struct pci_root_info *info = NULL;
	int domain = root->segment;
	int busnum = root->secondary.start;
	LIST_HEAD(resources);
	struct pci_bus *bus = NULL;
	struct xgene_pcie_port *sd;
	u32 lanes, speed;

	if (domain && !pci_domains_supported) {
		pr_warn("pci_bus %04x:%02x: "
			"ignored (multiple domains not supported)\n",
			domain, busnum);
		return NULL;
	}

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		pr_warn("pci_bus %04x:%02x: "
			"ignored (out of memory)\n", domain, busnum);
		return NULL;
	}

	sd = &info->sd;
	sd->acpi = device->handle;
	sd->dev = &device->dev;

	/*
	 * Maybe the desired pci bus has been already scanned. In such case
	 * it is unnecessary to scan the pci bus with the given domain,busnum.
	 */
	bus = pci_find_bus(domain, busnum);
	if (bus) {
		/*
		 * If the desired bus exits, the content of bus->sysdata will
		 * be replaced by sd.
		 */
		memcpy(bus->sysdata, sd, sizeof(*sd));
		kfree(info);
	} else {
		probe_pci_root_info(info, device, busnum, domain);
		/* csr_base must be remapped */
		if (IS_ERR_OR_NULL(sd->csr_base))
			return NULL;

		/* insert busn res at first */
		pci_add_resource(&resources,  &root->secondary);

		add_resources(info, &resources);

		sd->cfg_base = devm_ioremap_nocache(&device->dev,
					 root->mcfg_addr, PCIE_CFG_SIZE);
		if (IS_ERR(sd->cfg_base))
			return NULL;

		xgene_pcie_poll_linkup(sd, &lanes, &speed);
		if (sd->link_up)
			dev_info(sd->dev, "(rc) x%d gen-%d link up\n",
					lanes, speed + 1);
		else
			dev_info(sd->dev, "(rc) link down\n");

		bus = pci_create_root_bus_in_domain(NULL, domain, busnum,
					 &xgene_pcie_ops, sd, &resources);

		if (bus) {
			ACPI_HANDLE_SET(bus->bridge, sd->acpi);
			pci_scan_child_bus(bus);
			pci_assign_unassigned_resources();
			pci_set_host_bridge_release(
					 to_pci_host_bridge(bus->bridge),
					 release_pci_root_info, info);
		} else {
			pci_free_resource_list(&resources);
			__release_pci_root_info(info);
		}
	}

	/* After the PCI-E bus has been walked and all devices discovered,
	 * configure any settings of the fabric that might be necessary.
	 */
	if (bus) {
		struct pci_bus *child;
		list_for_each_entry(child, &bus->children, node)
			pcie_bus_configure_settings(child);
	}

	return bus;
}
