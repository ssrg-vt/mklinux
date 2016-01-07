/*
 * AppliedMicro X-Gene SoC USB Host Controller Driver
 *
 * Copyright (c) 2014, Applied Micro Circuits Corporation
 * Author: Thang Q. Nguyen <tqnguyen@apm.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/acpi.h>
#include <linux/efi.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include "xhci.h"

#define USB30BID                       0x0000
#define HOSTPORTREG                    0x0030
#define POWERMNGTREG                   0x0038
#define  HOST_PORT_POWER_CONTROL_PRESENT (1 << 0)
#define  PORT_OVERCUR_ENABLE           (1 << 28)
#define  PORT_OVERCUR_INVERT           (1 << 29)
#define  PORT_POWER_DISABLE            (1 << 30)
#define HBFSIDEBANDREG                 0x003c
#define INTERRUPTSTATUSMASK            0x0064
#define PIPEUTMIREG                    0x0070
#define  CSR_PIPE_CLK_READY            (1 << 1)
#define  CSR_USE_UTMI_FOR_PIPE         (1 << 8)
#define  PIPE0_PHYSTATUS_OVERRIDE_EN   (1 << 30)
#define  PIPE0_PHYSTATUS_SYNC          (1 << 31)
#define USB_SDS_CMU_STATUS0            0xa020
#define USB_OTG_CTL                    0xa058
#define  USB_OTG_CTL_PORT_RESET_SET(dst, src) \
		(((dst) & ~0x00000100) | (((u32)(src) << 8) & 0x00000100))
#define  USB_OTG_CTL_REFCLKSEL_SET(dst, src) \
		(((dst) & ~0x00000006) | (((u32)(src) << 1) & 0x00000006))
#define  USB_OTG_CTL_REFCLKDIV_SET(dst, src) \
		(((dst) & ~0x00000018) | (((u32)(src) << 3) & 0x00000018))
#define USB_OTG_OVER                   0xa05c
#define GCTL                           0xc110
#define  GCTL_RAMCLKSEL_SET(dst, src) \
		(((dst) & ~0x000000C0) | (((u32)(src) << 6) & 0x000000C0))
#define  GCTL_CORESOFTRESET_SET(dst, src) \
		(((dst) & ~0x00000800) | (((u32)(src) << 11) & 0x00000800))
#define GSNPSID                        0xc120
#define GUSB2PHYCFG_0                  0xc200
#define  GUSB2PHYCFG_0_PHYSOFTRST_SET(dst, src) \
		(((dst) & ~0x80000000) | (((u32)(src) << 31) & 0x80000000))
#define  GUSB2PHYCFG_0_SUSPENDUSB20_SET(dst, src) \
		(((dst) & ~0x00000040) | (((u32)(src) << 6) & 0x00000040))
#define  GUSB2PHYCFG_0_PHYIF_SET(dst, src) \
		(((dst) & ~0x00000008) | (((u32)(src) << 3) & 0x00000008))
#define GUSB3PIPECTL_0                 0xc2c0
#define  GUSB3PIPECTL_0_PHYSOFTRST_SET(dst, src) \
		(((dst) & ~0x80000000) | (((u32)(src) << 31) & 0x80000000))
#define  GUSB3PIPECTL_0_HSTPRTCMPL_SET(dst, src) \
		(((dst) & ~0x40000000) | (((u32)(src) << 30) & 0x40000000))
#define  GUSB3PIPECTL_0_SUSPENDENABLE_SET(dst, src) \
		(((dst) & ~0x00020000) | (((u32)(src) << 17) & 0x00020000))
#define  GUSB3PIPECTL_0_DATWIDTH_SET(dst, src) \
		(((dst) & ~0x00018000) | (((u32)(src) << 15) & 0x00018000))
#define  GUSB3PIPECTL_0_RX_DETECT_TO_POLLING_LFPS_SET(dst, src) \
		(((dst) & ~0x00000100) | (((u32)(src) << 8) & 0x00000100))
#define CFG_MEM_RAM_SHUTDOWN           0xd070
#define BLOCK_MEM_RDY                  0xd074
#define INT_SLV_TMOMask                0xe010
#define CFG_AMA_MODE                   0xe014
#define  CFG_RD2WR_EN                  0x2
#define USBSTS                         0x0024
#define  USBSTS_CNR                    (1 << 11)

static const char hcd_name[] = "xgene_xhci";

struct xgene_xhci_context {
	int irq;
	int port_en;
	int ovrcur_en;
	int ovrcur_ivrt;
	void __iomem *csr_base;
	void __iomem *mmio_base;

	struct clk *clk;
	struct clk *hsclk;	/* USB2 clock */
	struct device *dev;
};

/*
 * Wait until a register has a specific value or timeout.
 * Unit for interval and timeout parameters are micro-second
 */
u32 xgene_xhci_wait_register(void *reg, u32 mask, u32 val,
			     int interval, unsigned long timeout)
{
	unsigned long deadline = 0;
	u32 tmp;

	tmp = readl(reg);
	while (((tmp & mask) != val) && (deadline < timeout)) {
		udelay(interval);
		tmp = readl(reg);
		deadline += interval;
	}

	return tmp;
}

static void xhci_quirks(struct device *dev, struct xhci_hcd *xhci)
{
	xhci->quirks |= XHCI_BROKEN_MSI;
}

static int xhci_setup(struct usb_hcd *hcd)
{
	return xhci_gen_setup(hcd, xhci_quirks);
}

static const struct hc_driver xhci_hc_driver = {
	.description = hcd_name,
	.product_desc = "X-Gene xHCI Host Controller",
	.hcd_priv_size = sizeof(struct xhci_hcd *),

	.irq = xhci_irq,
	.flags = HCD_MEMORY | HCD_USB3 | HCD_SHARED,

	/* basic life cycle operations */
	.reset = xhci_setup,
	.start = xhci_run,
#ifdef CONFIG_PM
	/* suspend and resume implemented later */
	.bus_suspend = xhci_bus_suspend,
	.bus_resume = xhci_bus_resume,
#endif
	.stop = xhci_stop,
	.shutdown = xhci_shutdown,

	/* managing i/o requests and associated device resources */
	.urb_enqueue = xhci_urb_enqueue,
	.urb_dequeue = xhci_urb_dequeue,
	.alloc_dev = xhci_alloc_dev,
	.free_dev = xhci_free_dev,
	.alloc_streams = xhci_alloc_streams,
	.free_streams = xhci_free_streams,
	.add_endpoint = xhci_add_endpoint,
	.drop_endpoint = xhci_drop_endpoint,
	.endpoint_reset = xhci_endpoint_reset,
	.check_bandwidth = xhci_check_bandwidth,
	.reset_bandwidth = xhci_reset_bandwidth,
	.address_device = xhci_address_device,
	.update_hub_device = xhci_update_hub_device,
	.reset_device = xhci_discover_or_reset_device,

	/* scheduling support */
	.get_frame_number = xhci_get_frame,

	/* Root hub support */
	.hub_control = xhci_hub_control,
	.hub_status_data = xhci_hub_status_data,

	/* call back when device connected and addressed */
	.update_device = xhci_update_device,
	.set_usb2_hw_lpm = xhci_set_usb2_hardware_lpm,
	.enable_usb3_lpm_timeout = xhci_enable_usb3_lpm_timeout,
	.disable_usb3_lpm_timeout = xhci_disable_usb3_lpm_timeout,
};

static int xgene_xhci_init_memblk(struct xgene_xhci_context *hpriv)
{
	void __iomem *csr_base = hpriv->csr_base;
	unsigned int dt;
	int try;

	/* Clear memory shutdown */
	dt = readl(csr_base + CFG_MEM_RAM_SHUTDOWN);
	if (dt == 0) {
		dev_dbg(hpriv->dev, "memory already released from shutdown\n");
		return 0;
	}

	/* USB controller memory in shutdown. Remove from shutdown. */
	dev_dbg(hpriv->dev, "Release memory from shutdown\n");
	writel(0, csr_base + CFG_MEM_RAM_SHUTDOWN);
	dt = readl(csr_base + CFG_MEM_RAM_SHUTDOWN);

	/* Check for at least ~1ms */
	try = 1000;
	do {
		dt = readl(csr_base + BLOCK_MEM_RDY);
		if (dt != 0xFFFFFFFF)
			usleep_range(1, 100);
	} while (dt != 0xFFFFFFFF && try-- > 0);
	if (try <= 0) {
		dev_err(hpriv->dev, "failed to release memory from shutdown\n");
		return -ENODEV;
	}
	return 0;
}

static int xgene_xhci_get_dtb_parms(struct platform_device *pdev,
				    struct xgene_xhci_context *hpriv)
{
	struct device_node *op = pdev->dev.of_node;
	const char *status, *refclkname;
	u32 overcur_invert, overcur_enable;
	int len;

	/* Get clocks interface for HS/SS */
	hpriv->clk = clk_get(hpriv->dev, NULL);

	refclkname = of_get_property(op, "refclksel", &len);
	if (refclkname != NULL)
		hpriv->hsclk = clk_get(hpriv->dev, refclkname);

	hpriv->port_en = 1;
	status = of_get_property(op, "status", &len);
	if (status != NULL)
		if (strcmp(status, "disabled") == 0)
			hpriv->port_en = 0;

	if (of_property_read_u32(op, "overcur_enable", &overcur_enable) == 0)
		hpriv->ovrcur_en = overcur_enable;

	/* Port overcur invert */
	if (of_property_read_u32(op, "overcur_invert", &overcur_invert) == 0)
		hpriv->ovrcur_ivrt = overcur_invert;

	return 0;
}

/*
 * Initialize USB Controller.
 */
static int xgene_xhci_hw_init(struct xgene_xhci_context *hpriv)
{
	unsigned int dt;
	void __iomem *csr_base = hpriv->csr_base;
	void __iomem *fabric_base = hpriv->mmio_base;
	int rv;
	u32 bid;

	/* On an UEFI system, clock and PHY are configured by the FW */
	if (efi_enabled(EFI_BOOT))
		goto done;

	dev_dbg(hpriv->dev, "USB Initialization ...\n");
	if (hpriv->port_en == 0)
		return 1;

	dev_info(hpriv->dev, "overcur_en=%d, overcur_ivrt=%d, irq=%d",
		 hpriv->ovrcur_en, hpriv->ovrcur_ivrt, hpriv->irq);

	/* Read BID and Global SynopsysID */
	bid = readl(csr_base + USB30BID);
	dt = readl(fabric_base + GSNPSID);
	dev_dbg(hpriv->dev, "BID=%x, SynopsysID=%04x - RN=%04x\n",
		bid, (u16) (dt >> 16), (u16) dt);

	/* Read/Write Coherency Override Enable */
	dt = readl(csr_base + HBFSIDEBANDREG);
	dt |= 0xF;
	writel(dt, csr_base + HBFSIDEBANDREG);

	/* Initialize RAM shutdown */
	xgene_xhci_init_memblk(hpriv);

	/* Tune eye pattern */
	writel(0x001c0365, csr_base + USB_OTG_OVER);

	/* Reset Ref. clock select to default and enable port reset */
	dt = readl(csr_base + USB_OTG_CTL);
	dt = USB_OTG_CTL_REFCLKSEL_SET(dt, 0);
	dt = USB_OTG_CTL_REFCLKDIV_SET(dt, 0);
	dt = USB_OTG_CTL_PORT_RESET_SET(dt, 1);
	writel(dt, csr_base + USB_OTG_CTL);

	/* Configure USB2 PHY clock */
	if (!IS_ERR(hpriv->hsclk)) {
		rv = clk_prepare_enable(hpriv->hsclk);
		if (rv)
			dev_err(hpriv->dev,
				"ref. clock prepare enable failed\n");
	} else {
		dev_err(hpriv->dev, "no ref clock\n");
	}

	/* Configure Power Management Register */
	dev_dbg(hpriv->dev, "Enable port over current\n");
	dt = readl(csr_base + POWERMNGTREG);
	if (hpriv->ovrcur_en) {
		dt |= PORT_OVERCUR_ENABLE;
		if (hpriv->ovrcur_ivrt) {
			dev_dbg(hpriv->dev, "Set port over current inverted\n");
			dt |= PORT_OVERCUR_INVERT;
		}
	}
	dt |= HOST_PORT_POWER_CONTROL_PRESENT;
	writel(dt, csr_base + POWERMNGTREG);

	/* PHY Controller Configuration */
	dev_dbg(hpriv->dev, "Clear USB3 PHY soft reset\n");
	dt = readl(fabric_base + GUSB3PIPECTL_0);
	dt = GUSB3PIPECTL_0_HSTPRTCMPL_SET(dt, 0);
	dt = GUSB3PIPECTL_0_SUSPENDENABLE_SET(dt, 0);
	dt = GUSB3PIPECTL_0_RX_DETECT_TO_POLLING_LFPS_SET(dt, 1);
	dt = GUSB3PIPECTL_0_DATWIDTH_SET(dt, 1);	/* 16 bit */
	dt = GUSB3PIPECTL_0_PHYSOFTRST_SET(dt, 0);
	writel(dt, fabric_base + GUSB3PIPECTL_0);

	/* Clear USB2 PHY soft reset  */
	dev_dbg(hpriv->dev, "Clear USB2 PHY soft reset\n");
	dt = readl(fabric_base + GUSB2PHYCFG_0);
	dt = GUSB2PHYCFG_0_PHYSOFTRST_SET(dt, 0);
	dt = GUSB2PHYCFG_0_SUSPENDUSB20_SET(dt, 0);
	dt = GUSB2PHYCFG_0_PHYIF_SET(dt, 0);
	writel(dt, fabric_base + GUSB2PHYCFG_0);

	/* Clear CORE soft reset */
	dev_dbg(hpriv->dev, "Clear USB CORE reset\n");
	dt = readl(fabric_base + GCTL);
	dt = GCTL_RAMCLKSEL_SET(dt, 0);
	dt = GCTL_CORESOFTRESET_SET(dt, 0);
	writel(dt, fabric_base + GCTL);

	/* Clear OTG PHY reset */
	dev_dbg(hpriv->dev, "Clear OTG PHY reset\n");
	dt = readl(csr_base + USB_OTG_CTL);
	dt = USB_OTG_CTL_PORT_RESET_SET(dt, 0);
	writel(dt, csr_base + USB_OTG_CTL);

	/* Check if SERDES/PHY configured already */
	dt = xgene_xhci_wait_register(csr_base + USB_SDS_CMU_STATUS0,
					0x7, 0x7, 100, 100000);
	if ((dt & 0x3) != 0x3) {
		dev_info(hpriv->dev, "SERDES/PHY is not configured.\n");

		/* Disable USB 3.0 support when no PHY available */
		dev_info(hpriv->dev, "Disable USB3.0 capability\n");
		dt = readl(csr_base + HOSTPORTREG);
		dt |= 0x1;
		writel(dt, csr_base + HOSTPORTREG);

		/* Control/Monitor status of PIPE or UTMI signals */
		dt = readl(csr_base + PIPEUTMIREG);
		dt |= PIPE0_PHYSTATUS_OVERRIDE_EN;
		dt |= CSR_USE_UTMI_FOR_PIPE;
		writel(dt, csr_base + PIPEUTMIREG);
	} else {
		dev_dbg(hpriv->dev, "SERDES/PHY is configured.\n");

		/* Waiting when CLK is stable */
		dt = xgene_xhci_wait_register(csr_base + PIPEUTMIREG,
					      PIPE0_PHYSTATUS_SYNC, 0, 100,
					      100000);
		if ((dt & PIPE0_PHYSTATUS_SYNC) != 0) {
			dev_err(hpriv->dev,
				"Timeout waiting for PHY Status deasserted\n");
			return -1;
		}
		dev_dbg(hpriv->dev, "PHY Status de-asserted\n");

		dt = xgene_xhci_wait_register(csr_base + PIPEUTMIREG,
					      CSR_PIPE_CLK_READY,
					      CSR_PIPE_CLK_READY, 100, 100000);
		if ((dt & CSR_PIPE_CLK_READY) == 0)
			dev_err(hpriv->dev,
				"Timeout waiting for PIPE clock ready\n");
		else
			dev_dbg(hpriv->dev, "PIPE clock is ready\n");
	}

	/*  Poll register CNR until its cleared */
	dt = xgene_xhci_wait_register(fabric_base + USBSTS,
				      USBSTS_CNR, 0x0, 100, 500000);
	if (dt & USBSTS_CNR) {
		dev_err(hpriv->dev,
			"Timeout waiting for USB controller ready\n");
		return -1;
	} else {
		dev_dbg(hpriv->dev, "USB Controller is ready\n");
	}

done:
	return 0;
}

static int xgene_xhci_hcd_init(struct xgene_xhci_context *hpriv,
			       struct resource *mmio_res)
{
	struct usb_hcd *hcd;
	struct xhci_hcd *xhci;
	int rv = 0;

	hcd = usb_create_hcd(&xhci_hc_driver, hpriv->dev, hcd_name);
	if (!hcd)
		return -ENOMEM;
	hcd->rsrc_start = mmio_res->start;
	hcd->rsrc_len = mmio_res->end - mmio_res->start + 1;
	hcd->regs = hpriv->mmio_base;

	dev_dbg(hpriv->dev, "Add XHCI HCD\n");
	rv = usb_add_hcd(hcd, hpriv->irq, IRQF_SHARED);
	if (rv) {
		dev_err(hpriv->dev, "add HCD failed\n");
		goto remove_hcd;
	}

	hcd = dev_get_drvdata(hpriv->dev);
	xhci = hcd_to_xhci(hcd);
	xhci_print_registers(xhci);

	xhci->shared_hcd = usb_create_shared_hcd(&xhci_hc_driver,
						 hpriv->dev, hcd_name, hcd);
	if (!xhci->shared_hcd) {
		rv = -ENOMEM;
		goto remove_hcd;
	}

	*((struct xhci_hcd **)xhci->shared_hcd->hcd_priv) = xhci;
	rv = usb_add_hcd(xhci->shared_hcd, hpriv->irq, IRQF_SHARED);
	if (rv)
		goto put_usb3_hcd;
	hcd_to_bus(xhci->shared_hcd)->root_hub->lpm_capable = 1;

	return 0;

put_usb3_hcd:
	usb_put_hcd(xhci->shared_hcd);

remove_hcd:
	usb_remove_hcd(hcd);
	irq_dispose_mapping(hpriv->irq);
	usb_put_hcd(hcd);

	return rv;
}

static int xgene_xhci_probe(struct platform_device *pdev)
{
	struct xgene_xhci_context *hpriv;
	struct device *dev = &pdev->dev;
	struct resource *res, *mmio_res;
	int rv = 0;

	/* Do nothing if kernel does not support USB */
	if (usb_disabled())
		return -ENODEV;

	/*
	 * When both ACPI and DTS are enabled, custom ACPI built-in ACPI
	 * table, and booting via DTS, we need to skip the probe of the
	 * built-in ACPI table probe.
	 */
	if (!efi_enabled(EFI_BOOT) && dev->of_node == NULL)
		return -ENODEV;

	hpriv = devm_kzalloc(dev, sizeof(*hpriv), GFP_KERNEL);
	if (!hpriv) {
		dev_err(dev, "can't allocate host context\n");
		return -ENOMEM;
	}
	hpriv->dev = dev;

	mmio_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mmio_res) {
		dev_err(dev, "no MMIO space\n");
		return -EINVAL;
	}

	hpriv->mmio_base = devm_ioremap_resource(dev, mmio_res);
	if (!hpriv->mmio_base) {
		dev_err(dev, "can't map %pR\n", mmio_res);
		return -ENOMEM;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res) {
		dev_err(dev, "no csr space\n");
		return -EINVAL;
	}

	/*
	 * Can't use devm_ioremap_resource due to overlapping region.
	 * 0xYYYY.0000 - host core
	 * 0xYYYY.A000 - PHY indirect access
	 * 0xYYYY.C000 - Clock
	 * 0xYYYY.D000 - RAM shutdown removal
	 * As we map the entire region as one, it overlaps with the PHY driver.
	 */
	hpriv->csr_base = devm_ioremap(dev, res->start, resource_size(res));
	if (!hpriv->csr_base) {
		dev_err(dev, "can't map %pR\n", res);
		return -ENOMEM;
	}

	dev_dbg(dev, "VAddr 0x%p Mmio VAddr 0x%p\n",
		hpriv->csr_base, hpriv->mmio_base);

	hpriv->irq = platform_get_irq(pdev, 0);
	if (hpriv->irq <= 0) {
		dev_err(dev, "no IRQ\n");
		return -EINVAL;
	}

	/* Get more parameters */
        /* On an UEFI system, parameters are configured by the FW */
	if (!efi_enabled(EFI_BOOT))
                xgene_xhci_get_dtb_parms(pdev, hpriv);

	/* Setup DMA mask */
	pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;
	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(64);

	/* Save private data for later use */
	pdev->dev.platform_data = (void *) hpriv;

	if (!efi_enabled(EFI_BOOT)) {
		/* Prepare clock */
		if (!IS_ERR(hpriv->clk)) {
			rv = clk_prepare_enable(hpriv->clk);
			if (rv)
				dev_err(hpriv->dev, "clock prepare enable failed\n");
		} else {
			dev_err(hpriv->dev, "no clock\n");
		}
	}

	/* Configure the host controller */
	rv = xgene_xhci_hw_init(hpriv);
	if (rv)
		return rv;

	/* Enable core interrupt */
	dev_dbg(dev, "Enable core interrupt\n");
	writel(0xFFFFFFF0, hpriv->csr_base + INTERRUPTSTATUSMASK);

	/* Enable AXI Interrupts */
	writel(0x0, hpriv->csr_base + INT_SLV_TMOMask);

	rv = xgene_xhci_hcd_init(hpriv, mmio_res);

	return rv;
}

static int xgene_xhci_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = dev_get_drvdata(&pdev->dev);
	struct device *dev = &pdev->dev;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);

	dev_set_drvdata(&pdev->dev, hcd);

	dev_info(dev, "Stopping X-Gene XHCI Controller\n");
	usb_put_hcd(xhci->shared_hcd);

	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	irq_dispose_mapping(hcd->irq);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);

	return 0;
}

static struct of_device_id xgene_xhci_of_match[] = {
	{.compatible = "apm,xgene-xhci",},
	{},
};

MODULE_DEVICE_TABLE(of, xgene_xhci_of_match);

static const struct acpi_device_id xgene_xhci_acpi_match[] = {
	{"APMC0D03", 0},
	{},
};

MODULE_DEVICE_TABLE(of, xgene_xhci_acpi_match);

static struct platform_driver xgene_xhci_driver = {
	.driver = {
		   .name = "xgene_xhci",
		   .owner = THIS_MODULE,
		   .of_match_table = xgene_xhci_of_match,
#ifdef CONFIG_ACPI
		   .acpi_match_table = ACPI_PTR(xgene_xhci_acpi_match),
#endif
		   },
	.probe = xgene_xhci_probe,
	.remove = xgene_xhci_remove,
};

int xhci_register_platform_driver(void)
{
	return platform_driver_register(&xgene_xhci_driver);
}

void xhci_unregister_platform_driver(void)
{
	platform_driver_unregister(&xgene_xhci_driver);
}
