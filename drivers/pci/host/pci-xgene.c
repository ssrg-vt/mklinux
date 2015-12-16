/**
 * APM X-Gene PCIe Driver
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 *
 * Author: Tanmay Inamdar <tinamdar@apm.com>, Mayuresh Chitale <mchitale@apm.com>
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
#include <linux/version.h>
#include <linux/clk-private.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/memblock.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/of_pci.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/acpi.h>
#include <linux/efi.h>
#include <linux/pci_regs.h>
#include <asm/system_misc.h>
#include <asm-generic/signal.h>

#define PCIECORE_LTSSM			0x4c
#define PCIECORE_CTLANDSTATUS		0x50
#define INTXSTATUSMASK			0x6c
#define PIM1_1L				0x80
#define IBAR2				0x98
#define IR2MSK				0x9c
#define PIM2_1L				0xa0
#define IBAR3L				0xb4
#define IR3MSKL				0xbc
#define PIM3_1L				0xc4
#define OMR1BARL			0x100
#define OMR2BARL			0x118
#define OMR3BARL			0x130
#define CFGBARL				0x154
#define CFGBARH				0x158
#define CFGCTL				0x15c
#define RTDID				0x160
#define BRIDGE_CFG_0			0x2000
#define BRIDGE_CFG_1			0x2004
#define BRIDGE_CFG_4			0x2010
#define BRIDGE_CFG_32			0x2030
#define BRIDGE_CFG_14			0x2038
#define BRIDGE_CTRL_0			0x2200
#define BRIDGE_CTRL_1			0x2204
#define BRIDGE_CTRL_2			0x2208
#define BRIDGE_CTRL_5			0x2214
#define BRIDGE_STATUS_0			0x2600
#define MEM_RAM_SHUTDOWN                0xd070
#define BLOCK_MEM_RDY                   0xd074

#define CFG_CONSTANTS_479_448		0x2038
#define SWITCH_PORT_MODE_MASK		0x00000800
#define MSIX_CAP_DISABLE_MASK		0x00020000
#define MSI_CAP_DISABLE_MASK		0x00010000
#define CFG_CONTROL_191_160		0x2214
#define CFG_CONTROL_447_416		0x2234
#define CFG_CONSTANTS_31_00		0x2000
#define SLOT_IMPLEMENTED_MASK		0x04000000
#define CFG_CONSTANTS_63_32		0x2004
#define CFG_CONSTANTS_159_128		0x2010
#define DEVICE_PORT_TYPE_MASK		0x03c00000
#define PM_FORCE_RP_MODE_MASK		0x00000400
#define SWITCH_PORT_MODE_MASK		0x00000800
#define CLASS_CODE_MASK			0xffffff00
#define LINK_UP_MASK			0x00000100
#define AER_OPTIONAL_ERROR_EN		0xffc00000
#define XGENE_PCIE_DEV_CTRL		0x2f09
#define AXI_EP_CFG_ACCESS		0x10000
#define ENABLE_ASPM			0x08000000
#define XGENE_PORT_TYPE_RC		0x05000000
#define BLOCK_MEM_RDY_VAL               0xFFFFFFFF
#define EN_COHERENCY			0xF0000000
#define EN_REG				0x00000001
#define OB_LO_IO			0x00000002
#define XGENE_PCIE_VENDORID		0x10E8
#define XGENE_PCIE_DEVICEID		0xE004
#define XGENE_PCIE_EP_DEVICEID		0xCAFE
#define XGENE_PCIE_ECC_TIMEOUT		10 /* ms */
#define XGENE_LTSSM_DETECT_WAIT		20 /* ms */
#define XGENE_LTSSM_L0_WAIT		4  /* ms */
#define SZ_1T				(SZ_1G*1024ULL)
#define PIPE_PHY_RATE_RD(src)		((0xc000 & (u32)(src)) >> 0xe)
#define AXI_EP_DMA_ACCESS            	0x20000
#define XGENE_PCIE_EP_MEM_SIZE		0x100000
#define PTYPE_ENDPOINT			0
#define PTYPE_ROOT_PORT			1
#define XGENE_PCIE_EVENT_IRQ_OFFSET     4

#define GLBL_TRANS_ERR 0xD860
#define INT_SLV_TMO 0xE00C
#define SWINT 0x70
#define ROOT_PORT_CAP_AER 0x100

#define MAX_PORTS 5

static void __iomem *port_csr[MAX_PORTS] = {NULL, NULL, NULL, NULL, NULL};
static int num_ports = 0;
static int fatal_err[MAX_PORTS] = {0, 0, 0, 0, 0};

struct xgene_pcie_ep_info {
	void __iomem		*reg_virt[2];	/* maps to outbound space of RC */
	dma_addr_t		reg_phys[2];	/* Physical address of reg space */
	struct proc_dir_entry	*mem;		/* dump ep mem space */
	void __iomem		*msi_gen_base;
	void __iomem		*msi_term_base;
};

struct xgene_pcie_port {
	struct device_node	*node;
#ifdef CONFIG_ACPI
	void			*acpi; /* ACPI-specific data */
#endif
	struct device		*dev;
	struct clk		*clk;
	void __iomem		*csr_base;
	void __iomem		*cfg_base;
	u8			link_up;
	u8			type;
	u8			dma;
	struct xgene_pcie_ep_info	ep_info;
};

void xgene_pcie_reset_port_err(void __iomem *csr_base)
{
	int i;

	for (i = 0; i < num_ports; i++) {
		if ((u64)csr_base == (u64) port_csr[i])
			break;
	}
	if (i != num_ports)
		fatal_err[i] = 0;
}

static inline u32 pcie_bar_low_val(u32 addr, u32 flags)
{
	return (addr & PCI_BASE_ADDRESS_MEM_MASK) | flags;
}

/* PCIE Configuration Out/In */
static inline void xgene_pcie_cfg_out32(void __iomem *addr, int offset, u32 val)
{
	writel(val, addr + offset);
}

static inline void xgene_pcie_cfg_out16(void __iomem *addr, int offset, u16 val)
{
	u32 val32 = readl(addr + (offset & ~0x3));

	switch (offset & 0x3) {
	case 2:
		val32 &= ~0xFFFF0000;
		val32 |= (u32)val << 16;
		break;
	case 0:
	default:
		val32 &= ~0xFFFF;
		val32 |= val;
		break;
	}
	writel(val32, addr + (offset & ~0x3));
}

static inline void xgene_pcie_cfg_out8(void __iomem *addr, int offset, u8 val)
{
	u32 val32 = readl(addr + (offset & ~0x3));

	switch (offset & 0x3) {
	case 0:
		val32 &= ~0xFF;
		val32 |= val;
		break;
	case 1:
		val32 &= ~0xFF00;
		val32 |= (u32)val << 8;
		break;
	case 2:
		val32 &= ~0xFF0000;
		val32 |= (u32)val << 16;
		break;
	case 3:
	default:
		val32 &= ~0xFF000000;
		val32 |= (u32)val << 24;
		break;
	}
	writel(val32, addr + (offset & ~0x3));
}

static inline void xgene_pcie_cfg_in32(void __iomem *addr, int offset, u32 *val)
{
	*val = readl(addr + offset);
}

static inline void
xgene_pcie_cfg_in16(void __iomem *addr, int offset, u32 *val)
{
	*val = readl(addr + (offset & ~0x3));

	switch (offset & 0x3) {
	case 2:
		*val >>= 16;
		break;
	}

	*val &= 0xFFFF;
}

static inline void
xgene_pcie_cfg_in8(void __iomem *addr, int offset, u32 *val)
{
	*val = readl(addr + (offset & ~0x3));

	switch (offset & 0x3) {
	case 3:
		*val = *val >> 24;
		break;
	case 2:
		*val = *val >> 16;
		break;
	case 1:
		*val = *val >> 8;
		break;
	}
	*val &= 0xFF;
}

#ifdef CONFIG_NO_GENERIC_PCI_IOPORT_MAP
/* IO ports are memory mapped */
void __iomem *__pci_ioport_map(struct pci_dev *dev,
		unsigned long port, unsigned int nr)
{
	return ioremap_nocache(port, nr);
}
#endif

#ifdef CONFIG_PCI_XGENE_HOTPLUG
void __iomem *xgene_pcie_get_port_csr(struct pci_bus *bus)
{
	struct xgene_pcie_port *port = bus->sysdata;
	return port ? port->csr_base : NULL;
}
EXPORT_SYMBOL(xgene_pcie_get_port_csr);

void xgene_pcie_set_link_status(struct pci_bus *bus, u8 link)
{
	struct xgene_pcie_port *port = bus->sysdata;
	if (port)
		port->link_up = link;
}
EXPORT_SYMBOL(xgene_pcie_set_link_status);

int xgene_pcie_get_link_status(struct pci_bus *bus)
{
	struct xgene_pcie_port *port = bus->sysdata;
	if (port)
		return port->link_up;
	return -EINVAL;
}
EXPORT_SYMBOL(xgene_pcie_get_link_status);
#endif /* CONFIG_PCI_XGENE_HOTPLUG */

/* When the address bit [17:16] is 2'b01, the Configuration access will be
 * treated as Type 1 and it will be forwarded to external PCIe device.
 */
static void __iomem *xgene_pcie_get_cfg_base(struct pci_bus *bus)
{
	struct xgene_pcie_port *port = bus->sysdata;
	phys_addr_t addr = (phys_addr_t) port->cfg_base;

	if (bus->number >= (bus->primary + 1))
		addr |= AXI_EP_CFG_ACCESS;

	if(port->dma)
		addr |= AXI_EP_DMA_ACCESS;

	return (void *)addr;
}

/* For Configuration request, RTDID register is used as Bus Number,
 * Device Number and Function number of the header fields.
 */
static void xgene_pcie_set_rtdid_reg(struct pci_bus *bus, uint devfn)
{
	struct xgene_pcie_port *port = bus->sysdata;
	unsigned int b, d, f;
	u32 rtdid_val = 0;

	b = bus->number;
	d = PCI_SLOT(devfn);
	f = PCI_FUNC(devfn);

	if (!pci_is_root_bus(bus))
		rtdid_val = (b << 8) | (d << 3) | f;

	writel(rtdid_val, port->csr_base + RTDID);
	/* read the register back to ensure flush */
	readl(port->csr_base + RTDID);
}

static inline int xgene_pcie_port_fatal_err(struct xgene_pcie_port *port)
{
	int i;
	
	for (i = 0; i < num_ports; i++) {
		if ((u64)port->csr_base == (u64) port_csr[i])
			break;
	}
	if (i != num_ports && fatal_err[i])
		return 1;
	
	return 0;
}

static int xgene_pcie_read_config(struct pci_bus *bus, unsigned int devfn,
				  int offset, int len, u32 *val)
{
	struct xgene_pcie_port *port = bus->sysdata;
	void __iomem *addr;

	if ((pci_is_root_bus(bus) && devfn != 0) || !port->link_up)
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (xgene_pcie_port_fatal_err(port)) {
		if (pci_is_root_bus(bus) &&
		offset == (ROOT_PORT_CAP_AER + PCI_ERR_ROOT_STATUS)) {
			/* force to fatal error reporting */
			*val = PCI_ERR_ROOT_UNCOR_RCV | PCI_ERR_ROOT_FATAL_RCV;
			return PCIBIOS_SUCCESSFUL;
		}
		
		if (!pci_is_root_bus(bus))
			return PCIBIOS_DEVICE_NOT_FOUND;
	}

	xgene_pcie_set_rtdid_reg(bus, devfn);
	addr = xgene_pcie_get_cfg_base(bus);
	switch (len) {
	case 1:
		xgene_pcie_cfg_in8(addr, offset, val);
		break;
	case 2:
		xgene_pcie_cfg_in16(addr, offset, val);
		break;
	default:
		xgene_pcie_cfg_in32(addr, offset, val);
		break;
	}
	return PCIBIOS_SUCCESSFUL;
}

static int xgene_pcie_write_config(struct pci_bus *bus, unsigned int devfn,
				   int offset, int len, u32 val)
{
	struct xgene_pcie_port *port = bus->sysdata;
	void __iomem *addr;

	if ((pci_is_root_bus(bus) && devfn != 0) || !port->link_up)
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (xgene_pcie_port_fatal_err(port) && !pci_is_root_bus(bus))
		return PCIBIOS_DEVICE_NOT_FOUND;

	xgene_pcie_set_rtdid_reg(bus, devfn);
	addr = xgene_pcie_get_cfg_base(bus);
	switch (len) {
	case 1:
		xgene_pcie_cfg_out8(addr, offset, (u8)val);
		break;
	case 2:
		xgene_pcie_cfg_out16(addr, offset, (u16)val);
		break;
	default:
		xgene_pcie_cfg_out32(addr, offset, val);
		break;
	}
	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops xgene_pcie_ops = {
	.read = xgene_pcie_read_config,
	.write = xgene_pcie_write_config
};

static void xgene_pcie_program_core(void __iomem *csr_base)
{
	u32 val;

	val = readl(csr_base + BRIDGE_CTRL_0);
	val |= AER_OPTIONAL_ERROR_EN;
	writel(val, csr_base + BRIDGE_CTRL_0);
	writel(0x0, csr_base + INTXSTATUSMASK);
	val = readl(csr_base + BRIDGE_CTRL_1);
	val = (val & ~0xffff) | XGENE_PCIE_DEV_CTRL;
	writel(val, csr_base + BRIDGE_CTRL_1);
}

static void xgene_pcie_set_ib_mask(void __iomem *csr_base, u32 addr,
				  u32 flags, u64 size)
{
	u64 mask = (~(size - 1) & PCI_BASE_ADDRESS_MEM_MASK) | flags;
	u32 val32 = 0;
	u32 val;

	val32 = readl(csr_base + addr);
	val = (val32 & 0x0000ffff) | (lower_32_bits(mask) << 16);
	writel(val, csr_base + addr);

	val32 = readl(csr_base + addr + 0x04);
	val = (val32 & 0xffff0000) | (lower_32_bits(mask) >> 16);
	writel(val, csr_base + addr + 0x04);

	val32 = readl(csr_base + addr + 0x04);
	val = (val32 & 0x0000ffff) | (upper_32_bits(mask) << 16);
	writel(val, csr_base + addr + 0x04);

	val32 = readl(csr_base + addr + 0x08);
	val = (val32 & 0xffff0000) | (upper_32_bits(mask) >> 16);
	writel(val, csr_base + addr + 0x08);
}

static void xgene_pcie_poll_linkup(struct xgene_pcie_port *port,
				   u32 *lanes, u32 *speed)
{
	void __iomem *csr_base = port->csr_base;
	ulong timeout;
	u32 val32;

	/*
	 * A component enters the LTSSM Detect state within
	 * 20ms of the end of fundamental core reset.
	 */
	msleep(XGENE_LTSSM_DETECT_WAIT);
	port->link_up = 0;
	timeout = jiffies + msecs_to_jiffies(XGENE_LTSSM_L0_WAIT);
	while (time_before(jiffies, timeout)) {
		val32 = readl(csr_base + PCIECORE_CTLANDSTATUS);
		if (val32 & LINK_UP_MASK) {
			port->link_up = 1;
			*speed = PIPE_PHY_RATE_RD(val32);
			val32 = readl(csr_base + BRIDGE_STATUS_0);
			*lanes = val32 >> 26;
			break;
		}
		msleep(1);
	}
}

static void xgene_pcie_setup_root_complex(struct xgene_pcie_port *port)
{
	void __iomem *csr_base = port->csr_base;
	u32 val;

	val = (XGENE_PCIE_DEVICEID << 16) | XGENE_PCIE_VENDORID;
	writel(val, csr_base + BRIDGE_CFG_0);

	val = readl(csr_base + BRIDGE_CFG_1);
	val &= ~CLASS_CODE_MASK;
	val |= PCI_CLASS_BRIDGE_PCI << 16;
	writel(val, csr_base + BRIDGE_CFG_1);

	val = readl(csr_base + BRIDGE_CFG_14);
	val |= SWITCH_PORT_MODE_MASK;
	val &= ~PM_FORCE_RP_MODE_MASK;
	writel(val, csr_base + BRIDGE_CFG_14);

	val = readl(csr_base + BRIDGE_CTRL_5);
	val &= ~DEVICE_PORT_TYPE_MASK;
	val |= XGENE_PORT_TYPE_RC;
	writel(val, csr_base + BRIDGE_CTRL_5);

	val = readl(csr_base + BRIDGE_CTRL_2);
	val |= ENABLE_ASPM;
	writel(val, csr_base + BRIDGE_CTRL_2);

	val = readl(csr_base + BRIDGE_CFG_32);
	writel(val | (1 << 19), csr_base + BRIDGE_CFG_32);
}

/* Return 0 on success */
static int xgene_pcie_init_ecc(struct xgene_pcie_port *port)
{
	void __iomem *csr_base = port->csr_base;
	ulong timeout;
	u32 val;

	val = readl(csr_base + MEM_RAM_SHUTDOWN);
	if (!val)
		return 0;
	writel(0x0, csr_base + MEM_RAM_SHUTDOWN);
	timeout = jiffies + msecs_to_jiffies(XGENE_PCIE_ECC_TIMEOUT);
	while (time_before(jiffies, timeout)) {
		val = readl(csr_base + BLOCK_MEM_RDY);
		if (val == BLOCK_MEM_RDY_VAL)
			return 0;
		msleep(1);
	}

	return 1;
}

static int xgene_pcie_init_port(struct xgene_pcie_port *port)
{
	int rc;

	port->clk = clk_get(port->dev, NULL);
	if (IS_ERR(port->clk)) {
		dev_err(port->dev, "clock not available\n");
		return -ENODEV;
	}

	rc = clk_prepare_enable(port->clk);
	if (rc) {
		dev_err(port->dev, "clock enable failed\n");
		return rc;
	}

	rc = xgene_pcie_init_ecc(port);
	if (rc) {
		dev_err(port->dev, "memory init failed\n");
		return rc;
	}

	return 0;
}

static void xgene_pcie_fixup_bridge(struct pci_dev *dev)
{
	int i;

	/* Hide the PCI host BARs from the kernel as their content doesn't
	 * fit well in the resource management
	 */
	for (i = 0; i < DEVICE_COUNT_RESOURCE; i++) {
		dev->resource[i].start = dev->resource[i].end = 0;
		dev->resource[i].flags = 0;
	}
	dev_info(&dev->dev, "Hiding X-Gene pci host bridge resources %s\n",
		 pci_name(dev));
}
DECLARE_PCI_FIXUP_HEADER(XGENE_PCIE_VENDORID, XGENE_PCIE_DEVICEID,
			 xgene_pcie_fixup_bridge);

static int xgene_pcie_map_reg(struct xgene_pcie_port *port,
			      struct platform_device *pdev, u64 *cfg_addr)
{
	struct resource *res;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "csr");
	port->csr_base = devm_ioremap_resource(port->dev, res);
	if (IS_ERR(port->csr_base))
		return PTR_ERR(port->csr_base);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "cfg");
	port->cfg_base = devm_ioremap_resource(port->dev, res);
	if (IS_ERR(port->cfg_base))
		return PTR_ERR(port->cfg_base);
	*cfg_addr = res->start;

	if (num_ports < MAX_PORTS)
		port_csr[num_ports++] = port->csr_base;

	return 0;
}

static void xgene_pcie_setup_ob_reg(struct xgene_pcie_port *port,
				    struct resource *res, u32 offset, u64 addr)
{
	void __iomem *base = port->csr_base + offset;
	resource_size_t size = resource_size(res);
	u64 restype = resource_type(res);
	u64 cpu_addr, pci_addr;
	u64 mask = 0;
	u32 min_size;
	u32 flag = EN_REG;

	if (restype == IORESOURCE_MEM) {
		cpu_addr = res->start;
		pci_addr = addr;
		min_size = SZ_128M;
	} else {
		cpu_addr = addr;
		pci_addr = res->start;
		min_size = 128;
		flag |= OB_LO_IO;
	}
	if (size >= min_size)
		mask = ~(size - 1) | flag;
	else
		dev_warn(port->dev, "res size 0x%llx less than minimum 0x%x\n",
			 (u64)size, min_size);
	writel(lower_32_bits(cpu_addr), base);
	writel(upper_32_bits(cpu_addr), base + 0x04);
	writel(lower_32_bits(mask), base + 0x08);
	writel(upper_32_bits(mask), base + 0x0c);
	writel(lower_32_bits(pci_addr), base + 0x10);
	writel(upper_32_bits(pci_addr), base + 0x14);
}

static void xgene_pcie_setup_cfg_reg(void __iomem *csr_base, u64 addr)
{
	writel(lower_32_bits(addr), csr_base + CFGBARL);
	writel(upper_32_bits(addr), csr_base + CFGBARH);
	writel(EN_REG, csr_base + CFGCTL);
}

static int xgene_pcie_map_ranges(struct xgene_pcie_port *port,
				 struct pci_host_bridge *bridge,
				 u64 cfg_addr)
{
	struct device *dev = port->dev;
	struct pci_host_bridge_window *window;
	int ret;

	list_for_each_entry(window, &bridge->windows, list) {
		struct resource *res = window->res;
		u64 restype = resource_type(res);
		dev_dbg(port->dev, "0x%08lx 0x%016llx...0x%016llx\n",
			res->flags, res->start, res->end);

		switch (restype) {
		case IORESOURCE_IO:
			xgene_pcie_setup_ob_reg(port, res, OMR3BARL,
						bridge->io_base);
			ret = pci_ioremap_io(res, bridge->io_base);
			if (ret < 0)
				return ret;
			break;
		case IORESOURCE_MEM:
			xgene_pcie_setup_ob_reg(port, res, OMR1BARL,
						res->start - window->offset);
			break;
		case IORESOURCE_BUS:
			break;
		default:
			dev_err(dev, "invalid io resource!");
			return -EINVAL;
		}
	}
	if (port->type == PTYPE_ROOT_PORT)
		xgene_pcie_setup_cfg_reg(port->csr_base, cfg_addr);
	return 0;
}

static void xgene_pcie_setup_pims(void *addr, u64 pim, u64 size)
{
	writel(lower_32_bits(pim), addr);
	writel(upper_32_bits(pim) | EN_COHERENCY, addr + 0x04);
	writel(lower_32_bits(size), addr + 0x10);
	writel(upper_32_bits(size), addr + 0x14);
}

/*
 * X-Gene PCIe support maximum 3 inbound memory regions
 * This function helps to select a region based on size of region
 */
static int xgene_pcie_select_ib_reg(u8 *ib_reg_mask, u64 size)
{
	if ((size > 4) && (size < SZ_16M) && !(*ib_reg_mask & (1 << 1))) {
		*ib_reg_mask |= (1 << 1);
		return 1;
	}

	if ((size > SZ_1K) && (size < SZ_1T) && !(*ib_reg_mask & (1 << 0))) {
		*ib_reg_mask |= (1 << 0);
		return 0;
	}

	if ((size > SZ_1M) && (size < SZ_1T) && !(*ib_reg_mask & (1 << 2))) {
		*ib_reg_mask |= (1 << 2);
		return 2;
	}
	return -EINVAL;
}

static void xgene_pcie_setup_ib_reg(struct xgene_pcie_port *port,
				    struct of_pci_range *range, u8 *ib_reg_mask)
{
	void __iomem *csr_base = port->csr_base;
	void __iomem *cfg_base = port->cfg_base;
	void *bar_addr;
	void *pim_addr;
	u64 restype = range->flags & IORESOURCE_TYPE_BITS;
	u64 cpu_addr = range->cpu_addr;
	u64 pci_addr = range->pci_addr;
	u64 size = range->size;
	u64 mask = ~(size - 1) | EN_REG;
	u32 flags = PCI_BASE_ADDRESS_MEM_TYPE_64;
	u32 bar_low;
	int region;

	region = xgene_pcie_select_ib_reg(ib_reg_mask, range->size);
	if (region < 0) {
		dev_warn(port->dev, "invalid pcie dma-range config\n");
		return;
	}

	if (restype == PCI_BASE_ADDRESS_MEM_PREFETCH)
		flags |= PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar_low = pcie_bar_low_val((u32)cpu_addr, flags);
	switch (region) {
	case 0:
		xgene_pcie_set_ib_mask(csr_base, BRIDGE_CFG_4, flags, size);
		bar_addr = cfg_base + PCI_BASE_ADDRESS_0;
		writel(bar_low, bar_addr);
		writel(upper_32_bits(cpu_addr), bar_addr + 0x4);
		pim_addr = csr_base + PIM1_1L;
		break;
	case 1:
		bar_addr = csr_base + IBAR2;
		writel(bar_low, bar_addr);
		writel(lower_32_bits(mask), csr_base + IR2MSK);
		pim_addr = csr_base + PIM2_1L;
		break;
	case 2:
		bar_addr = csr_base + IBAR3L;
		writel(bar_low, bar_addr);
		writel(upper_32_bits(cpu_addr), bar_addr + 0x4);
		writel(lower_32_bits(mask), csr_base + IR3MSKL);
		writel(upper_32_bits(mask), csr_base + IR3MSKL + 0x4);
		pim_addr = csr_base + PIM3_1L;
		break;
	}

	xgene_pcie_setup_pims(pim_addr, pci_addr, size);
}

static int pci_dma_range_parser_init(u8 type, struct of_pci_range_parser *parser,
				     struct device_node *node)
{
	const int na = 3, ns = 2;
	int rlen;

	parser->node = node;
	parser->pna = of_n_addr_cells(node);
	parser->np = parser->pna + na + ns;

	if (type == PTYPE_ENDPOINT)
		parser->range = of_get_property(node, "ib-ranges-ep", &rlen);
	else 
		parser->range = of_get_property(node, "ib-ranges", &rlen);
	if (!parser->range)
		return -ENOENT;

	parser->end = parser->range + rlen / sizeof(__be32);
	return 0;
}

static int xgene_pcie_parse_map_dma_ranges(struct xgene_pcie_port *port)
{
	struct device_node *np = port->node;
	struct of_pci_range range;
	struct of_pci_range_parser parser;
	struct device *dev = port->dev;
	u8 ib_reg_mask = 0;

	if (pci_dma_range_parser_init(port->type, &parser, np)) {
		dev_err(dev, "missing ib-ranges property\n");
		return -EINVAL;
	}

	/* Get the ib-ranges from DT */
	for_each_of_pci_range(&parser, &range) {
		u64 end = range.cpu_addr + range.size - 1;
		dev_dbg(port->dev, "0x%08x 0x%016llx..0x%016llx -> 0x%016llx\n",
			range.flags, range.cpu_addr, end, range.pci_addr);
		xgene_pcie_setup_ib_reg(port, &range, &ib_reg_mask);
	}
	if (!(ib_reg_mask &  (1 << 1))) {
		writel(0x0, port->csr_base + IR2MSK);
	}
	if (!(ib_reg_mask & (1 << 2))) {
		writel(0x0, port->csr_base + IR3MSKL);
	}
	return 0;
}

static void xgene_pcie_set_event_irq(const struct pci_dev *pdev, int irq)
{
	struct irq_domain *domain;
	struct device_node *np = pdev->bus->dev.of_node;
	int virq;

	irq += XGENE_PCIE_EVENT_IRQ_OFFSET;
	np = of_irq_find_parent(np);
	domain = irq_find_host(np);
	virq = irq_create_mapping(domain, irq);
	irq_set_irq_type(virq, 1);
}

int xgene_pcie_map_irq(const struct pci_dev *pci_dev, u8 slot, u8 pin)
{
        struct of_irq oirq;
        u32 virq;
        int ret;
        ret = of_irq_map_pci((struct pci_dev *) pci_dev, &oirq);
        if (ret)
                return ret;
        virq = irq_create_of_mapping(oirq.controller, oirq.specifier,
                                     oirq.size);
        if (virq && pci_is_root_bus(pci_dev->bus))
                        xgene_pcie_set_event_irq(pci_dev, virq);
        return virq;
}
EXPORT_SYMBOL(xgene_pcie_map_irq);

#include "pci-xgene-ep.c"

static int xgene_pcie_exception_handler(unsigned long addr,
			  unsigned int fsr, struct pt_regs *regs)
{
	u32 val;
	int i;

	for (i = 0; i < num_ports; i++ ) {
		val = readl(port_csr[i] + GLBL_TRANS_ERR);
		if (val || fatal_err[i]) {
			/* clear error status */
			if (val)
				writel(val, port_csr[i] + GLBL_TRANS_ERR);

			val = readl(port_csr[i] + INT_SLV_TMO);
			if (val)
				writel(val, port_csr[i] + INT_SLV_TMO);

			if (!fatal_err[i]) {
				fatal_err[i] = 1;
				/* generate interrupt to force AER to report */
				writel(0x1, port_csr[i] + SWINT);
			}

			/* ignore exception */
			regs->pc += 8;
			return 0;
		}
	}

	return 1;
}

static int xgene_pcie_probe_bridge(struct platform_device *pdev)
{
	struct device_node *np = of_node_get(pdev->dev.of_node);
	struct xgene_pcie_port *port;
	struct pci_host_bridge *bridge = NULL;
	resource_size_t lastbus;
	u32 lanes = 0, speed = 0;
	u64 cfg_addr = 0;
	const u8 *val;
	int ret;
	struct pci_bus *child;

	port = devm_kzalloc(&pdev->dev, sizeof(*port), GFP_KERNEL);
	if (!port)
		return -ENOMEM;
	port->node = np;
	port->dev = &pdev->dev;

	hook_fault_code(16, xgene_pcie_exception_handler, SIGBUS, 0,
			  "synchronous external abort");

	val = of_get_property(np, "device_type", NULL);
	if(val && !strcmp(val, "ep"))
		port->type = PTYPE_ENDPOINT;
	else
		port->type = PTYPE_ROOT_PORT;

	ret = xgene_pcie_map_reg(port, pdev, &cfg_addr);
	if (ret)
		return ret;

	ret = xgene_pcie_init_port(port);
	if (ret)
		return ret;
	xgene_pcie_program_core(port->csr_base);
	if (port->type == PTYPE_ENDPOINT) {
		ret = xgene_pcie_setup_endpoint(pdev, port);
		if (ret)
			return ret;
		goto skip;
	}
	xgene_pcie_setup_root_complex(port);

	bridge = of_create_pci_host_bridge(&pdev->dev, &xgene_pcie_ops, port);
	if (IS_ERR_OR_NULL(bridge))
		return PTR_ERR(bridge);

	ret = xgene_pcie_map_ranges(port, bridge, cfg_addr);
	if (ret)
		return ret;

	ret = xgene_pcie_parse_map_dma_ranges(port);
	if (ret)
		return ret;

	xgene_pcie_poll_linkup(port, &lanes, &speed);
skip:
	platform_set_drvdata(pdev, port);
	if (port->type == PTYPE_ENDPOINT) {
		dev_info(port->dev, "port: (EP)\n");
		return 0;
	} else 
		if (!port->link_up)
			dev_info(port->dev, "(rc) link down\n");
		else
			dev_info(port->dev, "(rc) x%d gen-%d link up\n",
					lanes, speed + 1);
	

	lastbus = pci_rescan_bus(bridge->bus);
	pci_bus_update_busn_res_end(bridge->bus, lastbus);
	dev_info (port->dev, "Configuring MPS and MRRS settings\n");
	list_for_each_entry(child, &bridge->bus->children, node)
		pcie_bus_configure_settings(child);
	return 0;
}

static const struct of_device_id xgene_pcie_match_table[] = {
	{.compatible = "apm,xgene-pcie",},
	{},
};

#ifdef CONFIG_ACPI
#include "pci-xgene-acpi.c"
#endif

static struct platform_driver xgene_pcie_driver = {
	.driver = {
		   .name = "xgene-pcie",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(xgene_pcie_match_table),
	},
	.probe = xgene_pcie_probe_bridge,
};
module_platform_driver(xgene_pcie_driver);

MODULE_AUTHOR("Tanmay Inamdar <tinamdar@apm.com>");
MODULE_DESCRIPTION("APM X-Gene PCIe driver");
MODULE_LICENSE("GPL v2");
