/*
 *  ahci.h - Common AHCI SATA definitions and declarations
 *
 *  Maintained by:  Tejun Heo <tj@kernel.org>
 *                  Please ALWAYS copy linux-ide@vger.kernel.org
 *                  on emails.
 *
 *  Copyright 2004-2005 Red Hat, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef __LINUX_AHCI_H__
#define __LINUX_AHCI_H__

#include <linux/clk.h>
#include <linux/regulator/consumer.h>

#define AHCI_MAX_CLKS		3

struct ata_port;

struct ahci_host_priv {
	void __iomem		*mmio;		/* bus-independent mem map */
	unsigned int		flags;		/* AHCI_HFLAG_* */
	u32			cap;		/* cap to use */
	u32			cap2;		/* cap2 to use */
	u32			port_map;	/* port map to use */
	u32			saved_cap;	/* saved initial cap */
	u32			saved_cap2;	/* saved initial cap2 */
	u32			saved_port_map;	/* saved initial port_map */
	u32			em_loc; /* enclosure management location */
	u32			em_buf_sz;	/* EM buffer size in byte */
	u32			em_msg_type;	/* EM message type */
	struct clk		*clks[AHCI_MAX_CLKS]; /* Optional */
	struct regulator	*target_pwr;	/* Optional */
	void			*plat_data;	/* Other platform data */
	/* Optional ahci_start_engine override */
	void			(*start_engine)(struct ata_port *ap);
};

#endif
