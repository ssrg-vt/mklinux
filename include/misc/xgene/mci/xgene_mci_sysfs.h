/**
 * AppliedMicro AM88xxxx MCI sysfs Interface
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * All rights reserved. Hoan Tran <hotran@apm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * @file apm_mci_sysfs.h
 **
 */

#ifndef __APM_MCI_SYSFS_H__
#define __APM_MCI_SYSFS_H__
#include <linux/device.h>

#if defined(CONFIG_SYSFS)
int add_mci_sysfs(struct device_driver *driver);
void remove_mci_sysfs(struct device_driver *driver);
#endif

#endif /* __APM_MCI_SYSFS_H__ */
