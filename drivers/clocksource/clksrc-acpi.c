/*
 * Copyright (c) 2013, Applied Micro Circuits Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/clocksource.h>
extern struct acpi_device_id __clksrc_acpi_table[];

static const struct acpi_device_id __clksrc_acpi_table_sentinel
	__used __section(__clksrc_acpi_table_end);

#ifdef CONFIG_ARM64
void __init clocksource_acpi_init(void)
{
	struct acpi_device_id *id;
	acpi_tbl_table_handler init_func;

	for (id = __clksrc_acpi_table; id->id[0]; id++) {
		init_func = (acpi_tbl_table_handler)id->driver_data;
		acpi_table_parse(id->id, init_func);
	}
}
#endif
