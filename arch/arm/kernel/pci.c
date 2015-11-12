/*
 * Code borrowed from powerpc/kernel/pci-common.c
 *
 * Copyright (C) 2003 Anton Blanchard <anton@au.ibm.com>, IBM
 * Copyright (C) 2014 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 */

#include <linux/version.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/of_pci.h>
#include <linux/of_platform.h>
#include <linux/slab.h>

struct ioresource {
	struct list_head list;
	phys_addr_t start;
	resource_size_t size;
};

static LIST_HEAD(io_list);

unsigned long pci_address_to_pio(phys_addr_t address)
{
	struct ioresource *res;

	list_for_each_entry(res, &io_list, list) {
		if (address >= res->start &&
			address < res->start + res->size) {
			return res->start - address;
		}
	}

	return (unsigned long)-1;
}
EXPORT_SYMBOL_GPL(pci_address_to_pio);

