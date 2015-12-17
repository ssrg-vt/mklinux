/*
 * AMBA Connector Resource for ACPI
 *
 * Copyright (C) 2013 Advanced Micro Devices, Inc.
 *
 * Author: Brandon Anderson <brandon.anderson@amd.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifdef CONFIG_ACPI

#include <linux/module.h>
#include <linux/amba/bus.h>
#include <linux/platform_device.h>
#include <linux/acpi.h>
#include <linux/clkdev.h>
#include <linux/amba/acpi.h>

struct acpi_amba_bus_info {
	struct platform_device *pdev;
	struct clk *clk;
	char *clk_name;
};

/* UUID: a706b112-bf0b-48d2-9fa3-95591a3c4c06 (randomly generated) */
static const char acpi_amba_dsm_uuid[] = {
	0xa7, 0x06, 0xb1, 0x12, 0xbf, 0x0b, 0x48, 0xd2,
	0x9f, 0xa3, 0x95, 0x59, 0x1a, 0x3c, 0x4c, 0x06
};

/* acpi_amba_dsm_lookup()
 *
 * Helper to parse through ACPI _DSM object for a device. Each entry
 * has three fields.
 */
int acpi_amba_dsm_lookup(acpi_handle handle,
		const char *tag, int index,
		struct acpi_amba_dsm_entry *entry)
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
EXPORT_SYMBOL_GPL(acpi_amba_dsm_lookup);

static int acpi_amba_add_resource(struct acpi_resource *ares, void *data)
{
	struct amba_device *dev = data;
	struct resource r;
	int irq_idx;

	switch (ares->type) {
	case ACPI_RESOURCE_TYPE_FIXED_MEMORY32:
		if (!acpi_dev_resource_memory(ares, &dev->res))
			pr_err("failed to map memory resource\n");
		break;
	case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
		for (irq_idx = 0; irq_idx < AMBA_NR_IRQS; irq_idx++) {
			if (acpi_dev_resource_interrupt(ares, irq_idx, &r))
				dev->irq[irq_idx] = r.start;
			else
				break;
		}

		break;
	case ACPI_RESOURCE_TYPE_END_TAG:
		/* ignore the end tag */
		break;
	default:
		/* log an error, but proceed with driver probe */
		pr_err("unhandled acpi resource type= %d\n",
				ares->type);
		break;
	}

	return 1; /* Tell ACPI core that this resource has been handled */
}

static struct clk *acpi_amba_get_clk(acpi_handle handle, int index,
		char **clk_name)
{
	struct acpi_amba_dsm_entry entry;
	acpi_handle clk_handle;
	struct acpi_device *adev, *clk_adev;
	char *clk_path;
	struct clk *clk = NULL;
	int len;

	if (acpi_bus_get_device(handle, &adev)) {
		pr_err("cannot get device from handle\n");
		return NULL;
	}

	/* key=value format for clocks is:
	 *	"clock-name"="apb_pclk \\_SB.CLK0"
	 */
	if (acpi_amba_dsm_lookup(handle, "clock-name", index, &entry) != 0)
		return NULL;

	/* look under the clock device for the clock that matches the entry */
	*clk_name = NULL;
	len = strcspn(entry.value, " ");
	if (len > 0 && (len + 1) < strlen(entry.value)) {
		clk_path = entry.value + len + 1;
		*clk_name = kmalloc(len + 1, GFP_KERNEL);
		strncpy(*clk_name, entry.value, len);
		(*clk_name)[len] = '\0';
		if (ACPI_FAILURE(
			acpi_get_handle(NULL, clk_path, &clk_handle)) == 0 &&
			acpi_bus_get_device(clk_handle, &clk_adev) == 0)
			clk = clk_get_sys(dev_name(&clk_adev->dev), *clk_name);
	} else
		pr_err("Invalid clock-name value format '%s' for %s\n",
				entry.value, dev_name(&adev->dev));

	kfree(entry.key);
	kfree(entry.value);

	return clk;
}

static void acpi_amba_register_clocks(struct acpi_device *adev,
		struct clk *default_clk, const char *default_clk_name)
{
	struct clk *clk;
	int i;
	char *clk_name;

	if (default_clk) {
		/* for amba_get_enable_pclk() ... */
		clk_register_clkdev(default_clk, default_clk_name,
				dev_name(&adev->dev));
		/* for devm_clk_get() ... */
		clk_register_clkdev(default_clk, NULL, dev_name(&adev->dev));
	}

	for (i = 0;; i++) {
		clk_name = NULL;
		clk = acpi_amba_get_clk(ACPI_HANDLE(&adev->dev), i, &clk_name);
		if (!clk)
			break;

		clk_register_clkdev(clk, clk_name, dev_name(&adev->dev));

		kfree(clk_name);
	}
}

/* acpi_amba_add_device()
 *
 * ACPI equivalent to of_amba_device_create()
 */
static acpi_status acpi_amba_add_device(acpi_handle handle,
				u32 lvl_not_used, void *data, void **not_used)
{
	struct list_head resource_list;
	struct acpi_device *adev;
	struct amba_device *dev;
	int ret;
	struct acpi_amba_bus_info *bus_info = data;
	struct platform_device *bus_pdev = bus_info->pdev;

	if (acpi_bus_get_device(handle, &adev)) {
		pr_err("%s: acpi_bus_get_device failed\n", __func__);
		return AE_OK;
	}

	pr_debug("Creating amba device %s\n", dev_name(&adev->dev));

	dev = amba_device_alloc(NULL, 0, 0);
	if (!dev) {
		pr_err("%s(): amba_device_alloc() failed for %s\n",
		       __func__, dev_name(&adev->dev));
		return AE_CTRL_TERMINATE;
	}

	/* setup generic device info */
	dev->dev.coherent_dma_mask = ~0;
	dev->dev.parent = &bus_pdev->dev;
	dev_set_name(&dev->dev, "%s", dev_name(&adev->dev));

#if 0
	/* setup amba-specific device info */
	ACPI_COMPANION_SET(&dev->dev, adev);
	ACPI_COMPANION_SET(&adev->dev, adev);
#endif

	INIT_LIST_HEAD(&resource_list);
	acpi_dev_get_resources(adev, &resource_list,
				     acpi_amba_add_resource, dev);
	acpi_dev_free_resource_list(&resource_list);

	/* Add clocks */
	acpi_amba_register_clocks(adev, bus_info->clk, bus_info->clk_name);

	/* Read AMBA hardware ID and add device to system. If a driver matching
	 *	hardware ID has already been registered, bind this device to it.
	 *	Otherwise, the platform subsystem will match up the hardware ID
	 *	when the matching driver is registered.
	*/
	ret = amba_device_add(dev, &iomem_resource);
	if (ret) {
		pr_err("%s(): amba_device_add() failed (%d) for %s\n",
		       __func__, ret, dev_name(&adev->dev));
		goto err_free;
	}

	return AE_OK;

err_free:
	amba_device_put(dev);

	return AE_OK; /* don't prevent other devices from being probed */
}

static int acpi_amba_bus_probe(struct platform_device *pdev)
{
	struct acpi_amba_bus_info bus_info;
	bus_info.clk_name = NULL;

	/* see if there's a top-level clock to use as default for sub-devices */
	bus_info.clk = acpi_amba_get_clk(ACPI_HANDLE(&pdev->dev), 0,
			&bus_info.clk_name);

	/* probe each device */
	bus_info.pdev = pdev;
	acpi_walk_namespace(ACPI_TYPE_DEVICE, ACPI_HANDLE(&pdev->dev), 1,
			acpi_amba_add_device, NULL, &bus_info, NULL);

	kfree(bus_info.clk_name);

	return 0;
}

static const struct acpi_device_id amba_bus_acpi_match[] = {
	{ "AMBA0000", 0 },
	{ },
};

static struct platform_driver amba_bus_acpi_driver = {
	.driver = {
		.name = "amba-acpi",
		.owner = THIS_MODULE,
		.acpi_match_table = ACPI_PTR(amba_bus_acpi_match),
	},
	.probe	= acpi_amba_bus_probe,
};

static int __init acpi_amba_bus_init(void)
{
	return platform_driver_register(&amba_bus_acpi_driver);
}

postcore_initcall(acpi_amba_bus_init);

MODULE_AUTHOR("Brandon Anderson <brandon.anderson@amd.com>");
MODULE_DESCRIPTION("ACPI Connector Resource for AMBA bus");
MODULE_LICENSE("GPLv2");
MODULE_ALIAS("platform:amba-acpi");

#endif /* CONFIG_ACPI */
