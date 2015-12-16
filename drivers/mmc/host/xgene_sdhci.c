/*
 * APM X-Gene SoC Secure Digital Host Controller Driver
 *
 * Copyright (c) 2014, Applied Micro Circuits Corporation
 * Author: Rameshwar Prasad Sahu <rsahu@apm.com>
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
#include <linux/dma-mapping.h>
#include <linux/acpi.h>
#include <linux/of.h>
#include "sdhci-pltfm.h"

#define AIM_0_AXI_HI			0x00000090
#define  AIM_AXI_ADDRESS_HI_N_WR(src)	(((u32) (src) << 20) & 0xfff00000)
#define AIM_0_SIZE_CTL			0x00000084
#define  AIM_EN_N_WR(src)		(((u32) (src) << 31) & 0x80000000)
#define  ARSB_WR(src)			(((u32) (src) << 23) & 0x03800000)
#define  AWSB_WR(src)			(((u32) (src) << 20) & 0x00700000)
#define  AIM_MASK_N_WR(src)		(((u32) (src)) & 0x000fffff)

struct xgene_sdhci_ctx {
	void __iomem *ahbc_ioaddr;
};

static void xgene_sdhci_writel(struct sdhci_host *host, u32 val, int reg)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct xgene_sdhci_ctx *ctx = sdhci_pltfm_priv(pltfm_host);

	/* Configure the AHBC AIM0 AXI_HI window for DMA transfer */
	if (reg == SDHCI_DMA_ADDRESS && ctx->ahbc_ioaddr) {
		u64 dma_addr = sg_dma_address(host->data->sg);
		writel(AIM_AXI_ADDRESS_HI_N_WR(dma_addr >> 32),
			ctx->ahbc_ioaddr + AIM_0_AXI_HI);
	}
	writel(val, host->ioaddr + reg);
}

static struct sdhci_ops xgene_sdhci_ops = {
	.write_l = xgene_sdhci_writel,
};

static const struct sdhci_pltfm_data xgene_sdhci_pdata = {
	.quirks = SDHCI_QUIRK_DELAY_AFTER_POWER |
		  SDHCI_QUIRK_NO_HISPD_BIT,
	.quirks2 = SDHCI_QUIRK2_SHARED_BUS_LOCK | SDHCI_QUIRK2_NO_1_8_V |
		  SDHCI_QUIRK2_HOST_NO_CMD23,
	.ops = &xgene_sdhci_ops,
};

static int xgene_sdhci_probe(struct platform_device *pdev)
{
	struct sdhci_host *host;
	struct sdhci_pltfm_host *pltfm_host;
	struct xgene_sdhci_ctx *ctx;
	struct resource *res;
	struct clk *clk;
	int rc;

	host = sdhci_pltfm_init(pdev, &xgene_sdhci_pdata, sizeof(*ctx));
	if (IS_ERR(host))
		return PTR_ERR(host);

	pltfm_host = sdhci_priv(host);
	ctx = sdhci_pltfm_priv(pltfm_host);

	/* Set DMA mask */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 12, 0)
        pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;
        pdev->dev.coherent_dma_mask = DMA_BIT_MASK(64);
#else
	rc = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (rc) {
		dev_err(&pdev->dev, "Unable to set dma mask\n");
		return rc;
	}
#endif

	/* Clock is optional */
	clk = devm_clk_get(&pdev->dev, NULL);
	if (!IS_ERR(clk))
		clk_prepare_enable(clk);

	/* Load optional memory resource */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	ctx->ahbc_ioaddr = devm_ioremap(&pdev->dev, res->start,
					resource_size(res));
	if (ctx->ahbc_ioaddr) {
		/* Setup AHBC AIM0 windows ctrl register */
		writel(AIM_EN_N_WR(1) | ARSB_WR(1) |
		       AWSB_WR(1) | AIM_MASK_N_WR(0),
		       ctx->ahbc_ioaddr + AIM_0_SIZE_CTL);
	}

	/*
	 * This shouldn't be necessary. Just in case the FW doesn't
	 * configures to disable ADMA support as we don't support multiple
	 * DMA buffer whose address is 64-bit.
	 */
	host->quirks |= SDHCI_QUIRK_BROKEN_ADMA;

	/*
	 * SD Bus is shared between multiple lots, So, use serialization
	 * lock to access all slots at same time
	 */
        host->quirks2 |= SDHCI_QUIRK2_SHARED_BUS_LOCK;

	rc = sdhci_add_host(host);
	if (rc)
		sdhci_pltfm_free(pdev);

	return rc;
}

#ifdef CONFIG_OF
static const struct of_device_id xgene_sdhci_of_match[] = {
	{ .compatible = "apm,xgene-sdhci" },
	{},
};
MODULE_DEVICE_TABLE(of, xgene_sdhci_of_match);
#endif

#ifdef CONFIG_ACPI
static const struct acpi_device_id xgene_sdhci_acpi_match[] = {
	{ "APMC0D0C" },
	{},
};
MODULE_DEVICE_TABLE(acpi, xgene_ahci_acpi_match);
#endif

static struct platform_driver xgene_sdhci_driver = {
	.probe = xgene_sdhci_probe,
	.remove = sdhci_pltfm_unregister,
	.driver = {
		.name = "xgene-sdhci",
		.owner= THIS_MODULE,
		.of_match_table = of_match_ptr(xgene_sdhci_of_match),
		.acpi_match_table = ACPI_PTR(xgene_sdhci_acpi_match),
		.pm = SDHCI_PLTFM_PMOPS,
	},
};

module_platform_driver(xgene_sdhci_driver);

MODULE_DESCRIPTION("APM X-Gene SoC SDHCI driver");
MODULE_AUTHOR("Rameshwar Prasad Sahu <rsahu@apm.com>");
MODULE_LICENSE("GPL");
