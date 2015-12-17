/*
 * AMBA bus ACPI helpers
 *
 * Copyright (C) 2013 Advanced Micro Devices, Inc.
 *
 * Author: Brandon Anderson <brandon.anderson@amd.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ARM_AMBA_ACPI_H__
#define __ARM_AMBA_ACPI_H__

#ifdef CONFIG_ACPI
#include <linux/acpi.h>

struct acpi_amba_dsm_entry {
	char *key;
	char *value;
};

int acpi_amba_dsm_lookup(acpi_handle handle,
		const char *tag, int index,
		struct acpi_amba_dsm_entry *entry);

#endif

#endif
