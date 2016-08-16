/*
 * Key/Value handler from _DSM method
 *
 * Copyright (C) 2013 Linaro Ltd
 *
 * Author: Graeme Gregory <graeme.gregory@linaro.org>
 *
 * Original based on code :-
 *
 * Copyright (C) 2013 Advanced Micro Devices, Inc.
 *
 * Author: Brandon Anderson <brandon.anderson@amd.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/acpi.h>

/* UUID: a706b112-bf0b-48d2-9fa3-95591a3c4c06 (randomly generated) */
static const char acpi_amba_dsm_uuid[] = {
	0xa7, 0x06, 0xb1, 0x12, 0xbf, 0x0b, 0x48, 0xd2,
	0x9f, 0xa3, 0x95, 0x59, 0x1a, 0x3c, 0x4c, 0x06
};

/* acpi_dsm_lookup_value()
 *
 * Helper to parse through ACPI _DSM object for a device. Each entry
 * has three fields.
 */
int acpi_dsm_lookup_value(acpi_handle handle,
		const char *tag, int index,
		struct acpi_dsm_entry *entry)
{
	acpi_status status;
	struct acpi_object_list input;
	struct acpi_buffer output = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object params[4];
	union acpi_object *obj;
	int len, match_count, i;

	/* invalidate output in case there's no entry to supply */
	entry->key = NULL;
	entry->value = NULL;

	if (!acpi_has_method(handle, "_DSM"))
		return -ENOENT;

	input.count = 4;
	params[0].type = ACPI_TYPE_BUFFER;		/* UUID */
	params[0].buffer.length = sizeof(acpi_amba_dsm_uuid);
	params[0].buffer.pointer = (char *)acpi_amba_dsm_uuid;
	params[1].type = ACPI_TYPE_INTEGER;		/* Revision */
	params[1].integer.value = 1;
	params[2].type = ACPI_TYPE_INTEGER;		/* Function # */
	params[2].integer.value = 1;
	params[3].type = ACPI_TYPE_PACKAGE;		/* Arguments */
	params[3].package.count = 0;
	params[3].package.elements = NULL;
	input.pointer = params;

	status = acpi_evaluate_object_typed(handle, "_DSM",
			&input, &output, ACPI_TYPE_PACKAGE);
	if (ACPI_FAILURE(status)) {
		pr_err("failed to get _DSM package for this device\n");
		return -ENOENT;
	}

	obj = (union acpi_object *)output.pointer;

	/* parse 2 objects per entry */
	match_count = 0;
	for (i = 0; (i + 2) <= obj->package.count; i += 2) {
		/* key must be a string */
		len = obj->package.elements[i].string.length;
		if (len <= 0)
			continue;

		/* check to see if this is the entry to return */
		if (strncmp(tag, obj->package.elements[i].string.pointer,
				len) != 0 ||
				match_count < index) {
			match_count++;
			continue;
		}

		/* copy the key */
		entry->key = kmalloc(len + 1, GFP_KERNEL);
		strncpy(entry->key,
				obj->package.elements[i].string.pointer,
				len);
		entry->key[len] = '\0';

		/* value is a string with space-delimited fields if necessary */
		len = obj->package.elements[i + 1].string.length;
		if (len > 0) {
			entry->value = kmalloc(len + 1, GFP_KERNEL);
			strncpy(entry->value,
				obj->package.elements[i+1].string.pointer,
				len);
			entry->value[len] = '\0';
		}

		break;
	}

	kfree(output.pointer);

	if (entry->key == NULL)
		return -ENOENT;

	return 0;
}
EXPORT_SYMBOL_GPL(acpi_dsm_lookup_value);
