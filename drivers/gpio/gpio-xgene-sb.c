/*
 * AppliedMicro X-Gene SoC GPIO-Standby Driver
 *
 * Copyright (c) 2014, Applied Micro Circuits Corporation
 * Author: Tin Huynh <tnhuynh@apm.com>.
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
 */

#include <linux/module.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/of_irq.h>
#include <linux/acpi.h>
#include <linux/efi.h>
#include <linux/string.h>
#include <linux/of_address.h>
#define GPIO_MASK(x)			(1U << ((x) % 32))

#define GPIO_DIR_IN			0
#define GPIO_DIR_OUT			1

#define NIRQ_DEFAULT			{6}
#define GPIOSB_PINS_DEFAULT		{0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D}
#define NGPIO_DEFAULT			{32}

#define MPA_GPIO_INT_LVL		0x00000290
#define MPA_GPIO_OE_ADDR		0x0000029c
#define MPA_GPIO_OUT_ADDR		0x000002a0
#define MPA_GPIO_IN_ADDR		0x000002a4
#define MPA_GPIO_SEL_LO			0x00000294
#define MPA_GPIO_SEL_HIGH		0x0000029c


#define GICD_SPI_BASE			0x78010000
#define GICD_SPIR1			0x00000d08
struct xgene_gpio_sb {
	struct of_mm_gpio_chip mm;
	u32 *irq;
	u32 *irq_pins;
	u32 nirq;
	spinlock_t lock;
};

static inline struct xgene_gpio_sb *to_xgene_gpio_sb(struct of_mm_gpio_chip *mm)
{
	return container_of(mm, struct xgene_gpio_sb, mm);
}

static void xgene_gpio_set_bit(void __iomem *reg, unsigned int gpio, int val)
{
	u32 data;

	data = readl(reg);
	if (val)
		data |= GPIO_MASK(gpio);
	else
		data &= ~GPIO_MASK(gpio);
	writel(data, reg);
}

static int xgene_gpio_sb_get(struct gpio_chip *gc, unsigned int gpio)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	void __iomem *regs = mm_gc->regs;
	struct xgene_gpio_sb *chip = to_xgene_gpio_sb(mm_gc);
	void __iomem *gic_regs;
	u32 data, i;

	i = 0;
	while ((i < chip->nirq) && (gpio != chip->irq_pins[i]))
		i++;

	if (i < chip->nirq) {
		gic_regs = ioremap(GICD_SPI_BASE, 0x1000);
		if (!gic_regs)
			return -ENOMEM;
		data = readl(gic_regs + GICD_SPIR1);
		iounmap(gic_regs);
	} else {
		data = readl(regs + MPA_GPIO_IN_ADDR);
	}

	return (data &  GPIO_MASK(gpio)) ? 1 : 0;
}

static void xgene_gpio_sb_set(struct gpio_chip *gc, unsigned int gpio, int val)
{
	unsigned long flags;
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	void __iomem *regs = mm_gc->regs;
	struct xgene_gpio_sb *bank = to_xgene_gpio_sb(mm_gc);

	spin_lock_irqsave(&bank->lock, flags);

	xgene_gpio_set_bit(regs + MPA_GPIO_OUT_ADDR, gpio, val);

	spin_unlock_irqrestore(&bank->lock, flags);
}

static int xgene_gpio_sb_dir_out(struct gpio_chip *gc, unsigned int gpio,
				 int val)
{
	unsigned long flags;
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	void __iomem *regs = mm_gc->regs;
	struct xgene_gpio_sb *bank = to_xgene_gpio_sb(mm_gc);

	spin_lock_irqsave(&bank->lock, flags);

	xgene_gpio_set_bit(regs + MPA_GPIO_OE_ADDR, gpio, GPIO_DIR_OUT);

	spin_unlock_irqrestore(&bank->lock, flags);

	return 0;
}

static int xgene_gpio_sb_dir_in(struct gpio_chip *gc, unsigned int gpio)
{
	unsigned long flags;
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	void __iomem *regs = mm_gc->regs;
	struct xgene_gpio_sb *bank = to_xgene_gpio_sb(mm_gc);

	spin_lock_irqsave(&bank->lock, flags);

	xgene_gpio_set_bit(regs + MPA_GPIO_OE_ADDR, gpio, GPIO_DIR_IN);

	spin_unlock_irqrestore(&bank->lock, flags);

	return 0;
}

static int apm_gpio_sb_to_irq(struct gpio_chip *gc, unsigned gpio)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct xgene_gpio_sb *chip = to_xgene_gpio_sb(mm_gc);
	void __iomem *regs = mm_gc->regs;
	int ext_irq = 0;

	while ((ext_irq < chip->nirq) && (chip->irq_pins[ext_irq] != gpio))
		ext_irq++;
	if (ext_irq < chip->nirq) {
		xgene_gpio_set_bit(regs + MPA_GPIO_SEL_LO, gpio * 2,
				   GPIO_DIR_OUT);
		return chip->irq[ext_irq];
	}

	return -ENXIO;
}

static void xgene_get_param(struct platform_device *pdev, const char *name,
			    u32 *buf, int count, u32 *default_val)
{
	char name_str[80];
	int i;

#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT)) {
		struct acpi_dsm_entry entry;

		if (acpi_dsm_lookup_value(ACPI_HANDLE(&pdev->dev),
					  name, 0, &entry) || !entry.value)
			goto set_def;
		if (count == 3)
			sscanf(entry.value, "%d %d %d",
			       &buf[0], &buf[1], &buf[2]);
		else
			sscanf(entry.value, "%d %d %d %d %d %d",
			       &buf[0], &buf[1], &buf[2],
			       &buf[3], &buf[4], &buf[5]);
		kfree(entry.key);
		kfree(entry.value);

		return;
	}
#endif
	strcpy(name_str, name);
	if (!of_property_read_u32_array(pdev->dev.of_node, name_str, 
					buf, count))
		return;

#ifdef CONFIG_ACPI
set_def:
#endif

	/* Does not exist, load default */
	for (i = 0; i < count; i++)
		buf[i] = default_val[i % 3];
}

static int gpio_sb_probe(struct platform_device *pdev)
{
	struct of_mm_gpio_chip *mm;
	struct xgene_gpio_sb *apm_gc;
	u32 ret, i;
	u32 ngpio;
	u32 default_ngpio[] = NGPIO_DEFAULT;
	u32 default_nirq[] = NIRQ_DEFAULT;
	u32 default_pins[] = GPIOSB_PINS_DEFAULT;
	struct resource *res;

	apm_gc = devm_kzalloc(&pdev->dev, sizeof(*apm_gc), GFP_KERNEL);
	if (!apm_gc)
		return -ENOMEM;
	mm = &apm_gc->mm;
	mm->gc.direction_input = xgene_gpio_sb_dir_in;
	mm->gc.direction_output = xgene_gpio_sb_dir_out;
	mm->gc.get = xgene_gpio_sb_get;
	mm->gc.set = xgene_gpio_sb_set;
	mm->gc.to_irq = apm_gpio_sb_to_irq;
	mm->gc.base = -1;
	mm->gc.label = dev_name(&pdev->dev);
	platform_set_drvdata(pdev, mm);

	xgene_get_param(pdev, "ngpio", &ngpio, 1, default_ngpio);
	xgene_get_param(pdev, "nirq", &apm_gc->nirq, 1, default_nirq);
	mm->gc.ngpio = ngpio;

	apm_gc->irq_pins = devm_kzalloc(&pdev->dev, sizeof(u32) * apm_gc->nirq,
					GFP_KERNEL);
	if (!apm_gc->irq_pins)
		return -ENOMEM;

	apm_gc->irq = devm_kzalloc(&pdev->dev, sizeof(u32) * apm_gc->nirq,
				   GFP_KERNEL);
	if (!apm_gc->irq)
		return -ENOMEM;

	xgene_get_param(pdev, "irq_pins", apm_gc->irq_pins, apm_gc->nirq,
			default_pins);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	mm->regs = devm_ioremap_resource(&pdev->dev, res);
	if (!mm->regs)
		return PTR_ERR(mm->regs);

	for (i = 0; i < apm_gc->nirq; i++) {
		apm_gc->irq[i] = platform_get_irq(pdev, i);
		xgene_gpio_set_bit(mm->regs + MPA_GPIO_SEL_LO,
				   apm_gc->irq_pins[i] * 2, 1);
		xgene_gpio_set_bit(mm->regs + MPA_GPIO_INT_LVL, i, 1);
	}
	mm->gc.of_node = pdev->dev.of_node;
	ret = gpiochip_add(&mm->gc);
	if (ret)
		dev_err(&pdev->dev, "failed to add GPIO Standby chip");
	else
		dev_info(&pdev->dev, "X-Gene GPIO Standby driver registered\n");

	return ret;
}

static int xgene_gpio_sb_probe(struct platform_device *pdev)
{
	return gpio_sb_probe(pdev);
}

static int xgene_gpio_sb_remove(struct platform_device *pdev)
{
	struct of_mm_gpio_chip *mm = platform_get_drvdata(pdev);

	return gpiochip_remove(&mm->gc);
}

static const struct of_device_id xgene_gpio_sb_of_match[] = {
	{.compatible = "apm,xgene-gpio-sb", },
	{},
};

MODULE_DEVICE_TABLE(of, xgene_gpio_sb_of_match);

#ifdef CONFIG_ACPI
static const struct acpi_device_id xgene_gpio_sb_acpi_match[] = {
	{"APMC0D15", 0},
	{},
};

MODULE_DEVICE_TABLE(acpi, xgene_gpio_sb_acpi_match);
#endif

static struct platform_driver xgene_gpio_sb_driver = {
	.driver = {
		   .name = "xgene-gpio-sb",
		   .owner = THIS_MODULE,
		   .of_match_table = xgene_gpio_sb_of_match,
		   .acpi_match_table = ACPI_PTR(xgene_gpio_sb_acpi_match),
		   },
	.probe = xgene_gpio_sb_probe,
	.remove = xgene_gpio_sb_remove,
};

module_platform_driver(xgene_gpio_sb_driver);

MODULE_AUTHOR("AppliedMicro");
MODULE_DESCRIPTION("APM X-Gene GPIO Standby driver");
MODULE_LICENSE("GPL");
