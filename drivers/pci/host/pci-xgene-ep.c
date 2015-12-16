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

static int ep_mem_region;
static u64 ep_mem_base;
static u64 ep_mem_len;

int pcibios_prep_pcie_dma(struct pci_bus *bus)
{
	struct xgene_pcie_port *port = bus->sysdata;

	if(port->dma)
		return -EBUSY;
	port->dma = 1;
	return 0;
}
EXPORT_SYMBOL(pcibios_prep_pcie_dma);

void pcibios_cleanup_pcie_dma(struct pci_bus *bus)
{
	struct xgene_pcie_port *port = bus->sysdata;
	port->dma = 0;
}
EXPORT_SYMBOL(pcibios_cleanup_pcie_dma);

/* 
 * This test requires user to manually change dtbs for
 * EP port. Following are changes required in DTB 
 * 1. disable MSI node. Add status = "disabled" in MSI node 
 * 2. add 'interrupts = <0x0 0x10 0x4>' in PCIe Port node made as EP
 * */

static irqreturn_t xgene_misc_isr(int irq, void *data)
{
	struct xgene_pcie_port *port = (struct xgene_pcie_port *) data;
	struct xgene_pcie_ep_info *ep = &port->ep_info;
	u32 val;

	val = readl(ep->msi_term_base);
	pr_info ("Received Interrupt from RC\n");
	pr_info ("addr = 0x%llx, data = 0x%x\n", (u64)ep->msi_term_base, val);
	return IRQ_HANDLED;
}

static ssize_t ep_read_mem(struct file *file, char __user *buf,
			    size_t count, loff_t *data)
{
	struct xgene_pcie_ep_info *ep = PDE_DATA(file_inode(file));
	void __iomem *mem = ep->reg_virt[ep_mem_region] + ep_mem_base;
	u32 val;
	int i;

	pr_info ("dump mem - virt = %p , phys = 0x%llx\n", mem, ep->reg_phys[ep_mem_region]);
	for (i = 0; i < ep_mem_len; i++) {
		val = readl(mem + (i << 2));
		pr_info ("[%d] = 0x%08x\n", i, val);
	}
	return 0;
}

static ssize_t ep_write_mem(struct file *file, const char __user *buf,
			    size_t count, loff_t *data)
{
	static char pattern[12] = {'\0'};
	struct xgene_pcie_ep_info *ep = PDE_DATA(file_inode(file));
	void __iomem *mem = ep->reg_virt[ep_mem_region] + ep_mem_base;
	int i;
	u32 val;

	if (copy_from_user(pattern, buf, sizeof(pattern) - 1))
		return -EFAULT;
	
	sscanf (pattern, "%x", &val);
	for (i = 0; i < ep_mem_len; i++)
		writel(val, mem + (i << 2));
	return count;
}

static ssize_t ep_read_addr(struct file *file, char __user *buf,
			    size_t count, loff_t *data)
{
	pr_info ("%x %llx %llx\n",ep_mem_region, ep_mem_base, ep_mem_len);
	return 0;
}

static ssize_t ep_write_addr(struct file *file, const char __user *buf,
			    size_t count, loff_t *data)
{

	static char temp_buf[32] = {'\0'};

	if (copy_from_user(temp_buf, buf, sizeof(temp_buf) - 1))
		return -EFAULT;
	sscanf (temp_buf, "%u %016llx %016llx", &ep_mem_region, &ep_mem_base, &ep_mem_len); 
	if (ep_mem_region < 0 || ep_mem_region > 2) {
		pr_info ("Incorrect mem region. Only 0 or 1 supported.\n");
		pr_info ("Setting to 0\n");
		ep_mem_region = 0;
	}

	return count;
}

static ssize_t ep_gen_irq(struct file *file, const char __user *buf,
			    size_t count, loff_t *data)
{
	struct xgene_pcie_ep_info *ep = PDE_DATA(file_inode(file));
	void __iomem *base = ep->msi_gen_base;
	u32 val;

	writel(1, base); /* enable MSI mode */
	val = readl(base);
	writel(0, base + 4); /* enable MSI sources */
	val = readl(base + 4);
	val = readl(base + 0x100000); /* INT status */
	pr_info ("**** Generate MSI ******\n");
	writel(1, base + 0x200000); /* signal message interrupt pending */
	val = readl(base + 0x200000);
	writel(1, base + 0xa00000); /* clear int status */
	val = readl(base + 0xa00000);
	return count;
}

static const struct file_operations ep_fops = {
		.owner = THIS_MODULE,
		.write = ep_write_mem,
		.read  = ep_read_mem,
};

static const struct file_operations ep_irq_fops = {
		.owner = THIS_MODULE,
		.write = ep_gen_irq,
};

static const struct file_operations ep_addr_fops = {
		.owner = THIS_MODULE,
		.write = ep_write_addr,
		.read = ep_read_addr,
};


static int xgene_pcie_set_ep_ib(struct xgene_pcie_port *port)
{
	struct device_node *np = port->node;
	struct of_pci_range range;
	struct of_pci_range_parser parser;
	struct device *dev = port->dev;
	int i = 0;
	u64 pci_addr;
	struct xgene_pcie_ep_info *ep = &port->ep_info;
	void __iomem *virt;
	dma_addr_t phys;
	void * pim_addr;
	u32 mask_addr = CFG_CONSTANTS_159_128;
	u32 flags = PCI_BASE_ADDRESS_MEM_PREFETCH |
		PCI_BASE_ADDRESS_MEM_TYPE_64;

	if (pci_dma_range_parser_init(port->type, &parser, np)) {
		dev_err(dev, "missing ib-ranges property\n");
		return -EINVAL;
	}

	/* Get the ib-ranges from DT */
	for_each_of_pci_range(&parser, &range) {
		u64 end = range.cpu_addr + range.size - 1;
		dev_dbg(port->dev, "0x%08x 0x%016llx..0x%016llx -> 0x%016llx\n",
			range.flags, range.cpu_addr, end, range.pci_addr);
		if (i == 0 || i == 1) {
			virt = dma_alloc_coherent(port->dev,
					range.size, &phys,
						GFP_KERNEL);
			if(virt == NULL)
				return -ENOMEM;
			dev_info(port->dev, "EP: region %d - %p Phys - 0x%llx Size - 0x%llx\n",
					i, virt, phys, range.size);
			pci_addr = phys;
			ep->reg_virt[i] = virt;
			ep->reg_phys[i] = phys;
			if (i == 0)
				pim_addr = port->csr_base + PIM1_1L;
			else
				pim_addr = port->csr_base + PIM2_1L;
		} else {
			pci_addr = range.pci_addr;
			pim_addr = port->csr_base + PIM3_1L;
		}
		xgene_pcie_set_ib_mask(port->csr_base, mask_addr, flags, range.size);
		xgene_pcie_setup_pims(pim_addr, pci_addr, ~(range.size - 1));
		mask_addr += 8;
		i++;
	}
	return 0;
}

static int xgene_pcie_alloc_ep_mem (struct platform_device *pdev,
		struct xgene_pcie_port *port)
{
	struct xgene_pcie_ep_info *ep = &port->ep_info;
	struct proc_dir_entry *entry;
	struct resource *res;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "msi_gen");
	ep->msi_gen_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ep->msi_gen_base)) {
		return PTR_ERR(ep->msi_gen_base);
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "msi_term");
	ep->msi_term_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ep->msi_term_base))
		return PTR_ERR(ep->msi_term_base);

	ep->mem = proc_mkdir("xgene_pcie_ep", NULL);
	if (!ep->mem)
		return -ENOMEM;
	entry = proc_create_data("mem", S_IRWXUGO, ep->mem, &ep_fops, ep);
	if (!entry)
		return -ENOMEM;
	dev_info(port->dev, "xgene ep - /proc/xgene_pcie_ep/mem created!");

	entry = proc_create_data("irq", S_IRWXUGO, ep->mem, &ep_irq_fops, ep);
	if (!entry)
		return -ENOMEM;
	dev_info(port->dev, "xgene ep - /proc/xgene_pcie_ep/irq created!");

	entry = proc_create_data("addr", S_IRWXUGO, ep->mem, &ep_addr_fops, ep);
	if (!entry)
		return -ENOMEM;

	dev_info(port->dev, "xgene ep - /proc/xgene_pcie_ep/addr created!");


	return 0;
}

static int xgene_pcie_setup_endpoint(struct platform_device *pdev,
		struct xgene_pcie_port *port)
{
	void *csr_base = port->csr_base;
	u32 val;
	int irq;

	val = readl(csr_base + CFG_CONSTANTS_479_448);
	val &= ~SWITCH_PORT_MODE_MASK;
	val &= ~PM_FORCE_RP_MODE_MASK;
	writel(val, csr_base + CFG_CONSTANTS_479_448);

	val = readl(csr_base + CFG_CONTROL_191_160);
	val &= ~DEVICE_PORT_TYPE_MASK;
	val &= ~SLOT_IMPLEMENTED_MASK;
	writel(val, csr_base + CFG_CONTROL_191_160);

	val = readl(csr_base + CFG_CONSTANTS_31_00);
	val = (XGENE_PCIE_EP_DEVICEID << 16) | XGENE_PCIE_VENDORID;
	writel(val, csr_base + CFG_CONSTANTS_31_00);

	val = readl(csr_base + CFG_CONSTANTS_63_32);
	val &= ~CLASS_CODE_MASK;
	val |= PCI_CLASS_BRIDGE_OTHER << 16;
	writel(val, csr_base + CFG_CONSTANTS_63_32);

	val = readl(csr_base + CFG_CONTROL_447_416);
	val &= ~(MSI_CAP_DISABLE_MASK | MSIX_CAP_DISABLE_MASK);
	writel (val, csr_base + CFG_CONTROL_447_416);

	if (xgene_pcie_alloc_ep_mem(pdev, port))
		return -ENOMEM;

	if (xgene_pcie_set_ep_ib(port))
		return -ENOMEM;
	irq = platform_get_irq(pdev, 0);
	if (irq > 0)
		BUG_ON(request_irq(irq, xgene_misc_isr, IRQF_SHARED,
					"pcie-misc", (void *)port));
	return 0;
}

