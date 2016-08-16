/*
 * APEI Boot Error Record Table (BERT) support
 *
 * Copyright 2011 Intel Corp.
 *   Author: Huang Ying <ying.huang@xxxxxxxxx>
 *
 * Under normal circumstances, when a hardware error occurs, kernel
 * will be notified via NMI, MCE or some other method, then kernel
 * will process the error condition, report it, and recover it if
 * possible. But sometime, the situation is so bad, so that firmware
 * may choose to reset directly without notifying Linux kernel.
 *
 * Linux kernel can use the Boot Error Record Table (BERT) to get the
 * un-notified hardware errors that occurred in a previous boot.
 *
 * For more information about ERST, please refer to ACPI Specification
 * version 4.0, section 17.3.1
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/io.h>

#include "apei-internal.h"

#define BERT_PFX "BERT: "

int bert_disable;
EXPORT_SYMBOL_GPL(bert_disable);

static void __init bert_print_all(struct acpi_hest_generic_status *region,
				  unsigned int region_len)
{
	int remain, first = 1;
	u32 estatus_len;
	struct acpi_hest_generic_status *estatus;

	remain = region_len;
	estatus = region;
	while (remain > sizeof(struct acpi_hest_generic_status)) {
		/* No more error record */
		if (!estatus->block_status)
			break;

		estatus_len = apei_estatus_len(estatus);
		if (estatus_len < sizeof(struct acpi_hest_generic_status) ||
		    remain < estatus_len) {
			pr_err(FW_BUG BERT_PFX "Invalid error status block with length %u\n",
			       estatus_len);
			return;
		}

		if (apei_estatus_check(estatus)) {
			pr_err(FW_BUG BERT_PFX "Invalid Error status block\n");
			goto next;
		}

		if (first) {
			pr_info(HW_ERR "Error record from previous boot:\n");
			first = 0;
		}
		apei_estatus_print(KERN_INFO HW_ERR, estatus);

		/* Clear error status */
		estatus->block_status = 0;
next:
		estatus = (void *)estatus + estatus_len;
		remain -= estatus_len;
	}
}

static int __init setup_bert_disable(char *str)
{
	bert_disable = 1;
	return 0;
}
__setup("bert_disable", setup_bert_disable);

static int __init bert_check_table(struct acpi_table_bert *bert_tab)
{
	if (bert_tab->header.length < sizeof(struct acpi_table_bert))
		return -EINVAL;
	if (bert_tab->region_length != 0 &&
	    bert_tab->region_length < sizeof(struct acpi_bert_region))
		return -EINVAL;

	return 0;
}

static int __init bert_init(void)
{
	acpi_status status;
	struct acpi_table_bert *bert_tab;
	struct resource *r;
	struct acpi_hest_generic_status *bert_region;
	unsigned int region_len;
	int rc = -EINVAL;

	if (acpi_disabled)
		goto out;

	if (bert_disable) {
		pr_info(BERT_PFX "Boot Error Record Table (BERT) support is disabled.\n");
		goto out;
	}

	status = acpi_get_table(ACPI_SIG_BERT, 0,
				(struct acpi_table_header **)&bert_tab);
	if (status == AE_NOT_FOUND) {
		pr_err(BERT_PFX "Table is not found!\n");
		goto out;
	} else if (ACPI_FAILURE(status)) {
		const char *msg = acpi_format_exception(status);
		pr_err(BERT_PFX "Failed to get table, %s\n", msg);
		goto out;
	}

	rc = bert_check_table(bert_tab);
	if (rc) {
		pr_err(FW_BUG BERT_PFX "BERT table is invalid\n");
		goto out;
	}

	region_len = bert_tab->region_length;
	if (!region_len) {
		rc = 0;
		goto out;
	}

	r = request_mem_region(bert_tab->address, region_len, "APEI BERT");
	if (!r) {
		pr_err(BERT_PFX "Can not request iomem region <%016llx-%016llx> for BERT.\n",
		       (unsigned long long)bert_tab->address,
		       (unsigned long long)bert_tab->address + region_len);
		rc = -EIO;
		goto out;
	}

	bert_region = ioremap_cache(bert_tab->address, region_len);
	if (!bert_region) {
		rc = -ENOMEM;
		goto out_release;
	}

	bert_print_all(bert_region, region_len);

	iounmap(bert_region);

out_release:
	release_mem_region(bert_tab->address, region_len);
out:
	if (rc)
		bert_disable = 1;

	return rc;
}
late_initcall(bert_init);
