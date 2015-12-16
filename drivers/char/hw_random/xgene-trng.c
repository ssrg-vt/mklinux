/*
 * APM X-Gene SoC TRNG Driver
 *
 * Copyright (c) 2014, Applied Micro Circuits Corporation
 * Author: Rameshwar Prasad Sahu <rsahu@apm.com>
 *         Shamal Winchurkar <swinchurkar@apm.com>	
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/hw_random.h>

#define TRNG_MAX_DATUM			4
#define MAX_TRY				100
#define XGENE_TRNG_RETRY_COUNT		20
#define XGENE_TRNG_RETRY_INTERVAL	10

/* TRNG  Registers */
#define TRNG_INOUT_0			0x00
#define TRNG_INTR_STS_ACK		0x10
#define TRNG_CONTROL			0x14
#define TRNG_CONFIG			0x18
#define TRNG_ALARMCNT			0x1c
#define TRNG_FROENABLE			0x20
#define TRNG_FRODETUNE			0x24
#define TRNG_ALARMMASK			0x28
#define TRNG_ALARMSTOP			0x2c
#define TRNG_OPTIONS			0x78
#define TRNG_EIP_REV			0x7c

#define MONOBIT_FAIL_MASK		BIT(7) 
#define POKER_FAIL_MASK			BIT(6) 
#define LONG_RUN_FAIL_MASK		BIT(5)
#define RUN_FAIL_MASK			BIT(4)
#define NOISE_FAIL_MASK			BIT(3)
#define STUCK_OUT_MASK			BIT(2)
#define SHUTDOWN_OFLO_MASK		BIT(1)
#define READY_MASK			BIT(0)

#define MAJOR_HW_REV_RD(src)		(((src) & 0x0f000000) >> 24)
#define MINOR_HW_REV_RD(src)		(((src) & 0x00f00000) >> 20)
#define HW_PATCH_LEVEL_RD(src)		(((src) & 0x000f0000) >> 16)
#define MAX_REFILL_CYCLES_SET(dst,src) \
			(((dst) & ~0xffff0000) | (((u32) (src) << 16) & 0xffff0000))
#define READ_TIMEOUT_SET(dst,src) \
                      (((dst) & ~0x0000f000) | (((u32) (src) << 12) & 0x0000f000))
#define SAMPLE_DIV_SET(dst,src) \
                       (((dst) & ~0x00000f00) | (((u32) (src) << 8) & 0x00000f00))
#define MIN_REFILL_CYCLES_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32) (src)) & 0x000000ff))
#define SHUTDOWN_COUNT_SET(dst,src) \
                      (((dst) & ~0xff000000) | (((u32) (src) << 24) & 0xff000000))
#define SHUTDOWN_FATAL_SET(dst,src) \
                      (((dst) & ~BIT(23)) | (((u32) (src) << 23) & BIT(23)))
#define SHUTDOWN_THRESHOLD_SET(dst,src) \
                      (((dst) & ~0x001f0000) | (((u32) (src) << 16) & 0x001f0000))
#define STALL_RUN_POKER_SET(dst,src) \
                      (((dst) & ~BIT(15)) | (((u32) (src) << 15) & BIT(15)))
#define ALARM_THRESHOLD_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32) (src)) & 0x000000ff))
#define STARTUP_CYCLES_SET(dst,src) \
                      (((dst) & ~0xffff0000) | (((u32) (src) << 16) & 0xffff0000))
#define POST_PROC_EN_SET(dst,src) \
                      (((dst) & ~BIT(12)) | (((u32) (src) << 12) & BIT(12)))
#define ENABLE_TRNG_SET(dst,src) \
                      (((dst) & ~BIT(10)) | (((u32) (src) << 10) & BIT(10)))
#define REGSPEC_TEST_MODE_SET(dst,src) \
                       (((dst) & ~BIT(8)) | (((u32) (src) << 8) & BIT(8)))
#define MONOBIT_FAIL_MASK_SET(dst,src) \
                       (((dst) & ~BIT(7)) | (((u32) (src) << 7) & BIT(7)))
#define POKER_FAIL_MASK_SET(dst,src) \
                       (((dst) & ~BIT(6)) | (((u32) (src) << 6) & BIT(6)))
#define LONG_RUN_FAIL_MASK_SET(dst,src) \
                       (((dst) & ~BIT(5)) | (((u32) (src) << 5) & BIT(5)))
#define RUN_FAIL_MASK_SET(dst,src) \
                       (((dst) & ~BIT(4)) | (((u32) (src) << 4) & BIT(4)))
#define NOISE_FAIL_MASK_SET(dst,src) \
                       (((dst) & ~BIT(3)) | (((u32) (src) << 3) & BIT(3)))
#define STUCK_OUT_MASK_SET(dst,src) \
                       (((dst) & ~BIT(2)) | (((u32) (src) << 2) & BIT(2)))
#define SHUTDOWN_OFLO_MASK_SET(dst,src) \
                       (((dst) & ~BIT(1)) | (((u32) (src) << 1) & BIT(1)))

struct xgene_trng_dev {
	u32 irq;
	void  __iomem *csr_base;
	struct semaphore access_prot;
	u32 revision;
	u32 datum_size;
	u32 datum[TRNG_MAX_DATUM];
	u32 failure;		/* Non-zero indicates HW failure */
	u32 failure_cnt;	/* Failure count last minute */
	unsigned long failure_ts;/* First failure timestamp */
	struct timer_list failure_timer;
	struct device *dev;
	struct clk *clk;
};

static void xgene_trng_expired_timer(unsigned long arg)
{
	struct xgene_trng_dev *ctx = (struct xgene_trng_dev *) arg;
	/* Clear failure counter as timer expired */
	disable_irq(ctx->irq);
	ctx->failure_cnt = 0;
	del_timer(&ctx->failure_timer);
	enable_irq(ctx->irq);
}

static void xgene_trng_start_timer(struct xgene_trng_dev *ctx)
{
	ctx->failure_timer.data = (unsigned long) ctx;
	ctx->failure_timer.function = xgene_trng_expired_timer;
	ctx->failure_timer.expires = jiffies + 120 * HZ;
	add_timer(&ctx->failure_timer);
}

static void xgene_trng_chk_overflow(struct xgene_trng_dev *ctx)
{
	u32 val;
	
	val = readl(ctx->csr_base + TRNG_INTR_STS_ACK);
	if (val & MONOBIT_FAIL_MASK) {
		/* 
		 * LFSR detected an out-of-bounds number of 1s after
		 * checking 20,000 bits (test T1 as specified in the
		 * AIS-31 standard)
		 */
		dev_err(ctx->dev, "test monobit failure error 0x%08X\n", val);
		ctx->failure = val;
	}
	if (val & POKER_FAIL_MASK) {
		/*
		 * LFSR detected an out-of-bounds value in at least one
		 * of the 16 poker_count_X counters or an out of  bounds sum
		 * of squares value after checking 20,000 bits (test T2 as
		 * specified in the AIS-31 standard)
		 */
		dev_err(ctx->dev, "test poker failure error 0x%08X\n", val);
		ctx->failure = val;
	}
	if (val & LONG_RUN_FAIL_MASK) {
		/*
		 * LFSR detected a sequence of 34 identical bits
		 * (test T4 as specified in the AIS-31 standard)
		 */
		dev_err(ctx->dev, "test long run failure error 0x%08X\n", val);
		ctx->failure = val;
	}
	if (val & RUN_FAIL_MASK) {
		/*
		 * LFSR detected an outof-bounds value for at least one
		 * of the running counters after checking 20,000 bits
		 * (test T3 as specified in the AIS-31 standard)
		 */
		dev_err(ctx->dev, "test run failure error 0x%08X\n", val);
		ctx->failure = val;
	}
	if (val & NOISE_FAIL_MASK) {
		/* LFSR detected a sequence of 48 identical bits */
		dev_err(ctx->dev, "noise failure error 0x%08X\n", val);
		ctx->failure = val;
	}
	if (val & STUCK_OUT_MASK) {
		/*
		 * Detected output data registers generated same value twice
		 * in a row
		 */
		dev_err(ctx->dev, "stuck out failure error 0x%08X\n", val);
		ctx->failure = val;
	}
	if (val & SHUTDOWN_OFLO_MASK) {
		u32 frostopped;

		/* FROs shut down after a second error event. Try to recover. */
		if (++ctx->failure_cnt == 1) {
			/* 1st time, just recover */
			ctx->failure_ts = jiffies;
			frostopped = readl(ctx->csr_base + TRNG_ALARMSTOP);
			writel(frostopped, ctx->csr_base + TRNG_FRODETUNE);
			writel(0x00000000, ctx->csr_base + TRNG_ALARMMASK);
			writel(0x00000000, ctx->csr_base + TRNG_ALARMSTOP);
			writel(0xFFFFFFFF, ctx->csr_base + TRNG_FROENABLE);

			/*
			 * We must start a timer to clear out this error
			 * in case the system timer wrap around
			 */
			xgene_trng_start_timer(ctx);
		} else {
			/* 2nd time failure in lesser than 1 minute? */
			if (time_after(ctx->failure_ts + 60 * HZ,
						jiffies)) {
				dev_err(ctx->dev, "FRO shutdown failure error 0x%08X\n",
					val);
				ctx->failure = val;
			} else {
				/* 2nd time failure after 1 minutes, just recover */
				ctx->failure_ts = jiffies;
				ctx->failure_cnt = 1;
				/*
				 * We must start a timer to clear out this
				 * error in case the system timer wrap
				 * around
				 */
				xgene_trng_start_timer(ctx);
			}
			frostopped = readl(ctx->csr_base + TRNG_ALARMSTOP);
			writel(frostopped, ctx->csr_base + TRNG_FRODETUNE);
			writel(0x00000000, ctx->csr_base + TRNG_ALARMMASK);
			writel(0x00000000, ctx->csr_base + TRNG_ALARMSTOP);
			writel(0xFFFFFFFF, ctx->csr_base + TRNG_FROENABLE);
		}
	}
	/* Clear them all */
	writel(val, ctx->csr_base + TRNG_INTR_STS_ACK);
}

static irqreturn_t xgene_trng_irq_handler(int irq, void *id)
{
	struct xgene_trng_dev *ctx = (struct xgene_trng_dev *) id;
	/* TRNG Alarm Counter overflow */
	xgene_trng_chk_overflow(ctx);
	return IRQ_HANDLED;
}

static int xgene_trng_data_present(struct hwrng *rng, int wait)
{
	struct xgene_trng_dev *ctx = (struct xgene_trng_dev *) rng->priv;
	int i;
	int j;
	u32 val = 0;

	down(&ctx->access_prot);

	if (ctx->failure)
		goto done;
	
	for (i = 0; i < XGENE_TRNG_RETRY_COUNT; i++) {
		val = readl(ctx->csr_base + TRNG_INTR_STS_ACK);
		if (val & READY_MASK) {
			for (j = 0; j < ctx->datum_size; j++)
				ctx->datum[j] = readl(ctx->csr_base + 
						      TRNG_INOUT_0 + j * 4);
			writel(val, ctx->csr_base + TRNG_INTR_STS_ACK);
			break;
		}
		if (!wait)
			break;
		udelay(XGENE_TRNG_RETRY_INTERVAL);
	}
	
done:
	up(&ctx->access_prot);
	return (val & READY_MASK) ? 1 : 0;
}

static int xgene_trng_data_read(struct hwrng *rng, u32 *data)
{
	struct xgene_trng_dev *ctx = (struct xgene_trng_dev *) rng->priv;
	
	memcpy(data, ctx->datum, ctx->datum_size << 2);
	return ctx->datum_size << 2;
}

static void xgene_trng_init_internal(struct xgene_trng_dev * ctx)
{
	u32 val;

	writel(0x00000000, ctx->csr_base + TRNG_CONTROL);

	val = 0;
	val = MAX_REFILL_CYCLES_SET(val, 10);
	val = MIN_REFILL_CYCLES_SET(val, 10);
	val = READ_TIMEOUT_SET(val, 0);
	val = SAMPLE_DIV_SET(val, 0);
	writel(val, ctx->csr_base + TRNG_CONFIG);

	val = 0;
	val = SHUTDOWN_COUNT_SET(val, 0);
	val = SHUTDOWN_FATAL_SET(val, 0);
	val = SHUTDOWN_THRESHOLD_SET(val, 0);
	val = STALL_RUN_POKER_SET(val, 0);
	val = ALARM_THRESHOLD_SET(val, 0xFF);
	writel(val, ctx->csr_base + TRNG_ALARMCNT);

	writel(0x00000000, ctx->csr_base + TRNG_FRODETUNE);
	writel(0x00000000, ctx->csr_base + TRNG_ALARMMASK);
	writel(0x00000000, ctx->csr_base + TRNG_ALARMSTOP);
	writel(0xFFFFFFFF, ctx->csr_base + TRNG_FROENABLE);

	writel(MONOBIT_FAIL_MASK |
	       POKER_FAIL_MASK	|
	       LONG_RUN_FAIL_MASK |
	       RUN_FAIL_MASK |
	       NOISE_FAIL_MASK |
	       STUCK_OUT_MASK |
	       SHUTDOWN_OFLO_MASK |
	       READY_MASK, ctx->csr_base + TRNG_INTR_STS_ACK);

	val = 0;
	val = STARTUP_CYCLES_SET(val, 0);
	val = ENABLE_TRNG_SET(val, 1);
	val = MONOBIT_FAIL_MASK_SET(val, 1);
	val = POKER_FAIL_MASK_SET(val, 1);
	val = LONG_RUN_FAIL_MASK_SET(val, 1);
	val = RUN_FAIL_MASK_SET(val, 1);
	val = NOISE_FAIL_MASK_SET(val, 1);
	val = STUCK_OUT_MASK_SET(val, 1);
	val = SHUTDOWN_OFLO_MASK_SET(val, 1);
	val = POST_PROC_EN_SET(val, 0);
	writel(val, ctx->csr_base + TRNG_CONTROL);		
}

static int xgene_trng_init(struct hwrng *rng)
{
	struct xgene_trng_dev *ctx = (struct xgene_trng_dev *) rng->priv;

	ctx->failure = 0;
	ctx->failure_cnt = 0;
	init_timer(&ctx->failure_timer);

	ctx->revision = readl(ctx->csr_base + TRNG_EIP_REV);

	dev_dbg(ctx->dev, "Rev %d.%d.%d\n",
		MAJOR_HW_REV_RD(ctx->revision),
		MINOR_HW_REV_RD(ctx->revision),
		HW_PATCH_LEVEL_RD(ctx->revision));

	dev_dbg(ctx->dev, "Options 0x%08X",
		readl(ctx->csr_base + TRNG_OPTIONS));

	xgene_trng_init_internal(ctx);

	ctx->datum_size = 4;
	return 0;
}

static struct hwrng xgene_trng_func = {
	.name		= "xgene-trng",
	.init		= xgene_trng_init,
	.data_present	= xgene_trng_data_present,
	.data_read	= xgene_trng_data_read,
};

static int  xgene_trng_probe(struct platform_device* pdev)
{
	struct resource *res;
	struct xgene_trng_dev *ctx;
	int rc = 0;

	ctx = devm_kzalloc(&pdev->dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->dev = &pdev->dev;
	platform_set_drvdata(pdev, ctx);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	ctx->csr_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ctx->csr_base))
		return PTR_ERR(ctx->csr_base);

	ctx->irq = platform_get_irq(pdev, 0);
	if (ctx->irq < 0) {
		dev_err(&pdev->dev, "No IRQ resource\n");
		return ctx->irq;
	}

	dev_dbg(&pdev->dev,"APM 4xx TRNG BASE %p ALARM IRQ %d", 
		ctx->csr_base, ctx->irq);

	rc = devm_request_irq(&pdev->dev, ctx->irq, xgene_trng_irq_handler, 0,
			      dev_name(&pdev->dev), ctx);
	if (rc) {
		dev_err(&pdev->dev, "Could not request IRQ\n");
		return rc;
	}

	/* Enable IP clock */
	ctx->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(ctx->clk)) 
		dev_warn(&pdev->dev, "Couldn't get the clock for TRNG\n");
	else {
	
		rc = clk_prepare_enable(ctx->clk);
		if (rc) 
			dev_warn(&pdev->dev, 
				 "clock prepare enable failed for TRNG");
	}

	xgene_trng_func.priv = (unsigned long) ctx;
	
	sema_init(&ctx->access_prot, 1);

	rc = hwrng_register(&xgene_trng_func);
	if (rc) {
		dev_err(&pdev->dev, "TRNG registering failed error %d\n", rc);
		if (!IS_ERR(ctx->clk))
			clk_disable_unprepare(ctx->clk);
		return rc;		
	}
	
	rc = device_init_wakeup(&pdev->dev, 1);
	if (rc) {
		dev_err(&pdev->dev, "TRNG device_init_wakeup failed error %d\n",
			rc);
		if (!IS_ERR(ctx->clk))
			clk_disable_unprepare(ctx->clk);
		hwrng_unregister(&xgene_trng_func);
		return rc;		
	}

	return 0;
}

static int  xgene_trng_remove(struct platform_device *pdev)
{
	struct xgene_trng_dev *ctx = platform_get_drvdata(pdev);
	int rc;

	rc = device_init_wakeup(&pdev->dev, 0);
	if (rc) 
		dev_err(&pdev->dev, "TRNG device_init_wakeup failed error %d\n",
			rc);
	if (!IS_ERR(ctx->clk))
		clk_disable_unprepare(ctx->clk);
	hwrng_unregister(&xgene_trng_func);
	return rc;
}

static const struct of_device_id xgene_trng_of_match[] = {
	{ .compatible = "apm,xgene-trng" },
	{ }
};

MODULE_DEVICE_TABLE(of, xgene_trng_of_match);

static struct platform_driver xgene_trng_driver = {
	.probe = xgene_trng_probe,
	.remove	= xgene_trng_remove,
	.driver = {
		.name		= "xgene-trng",
		.owner		= THIS_MODULE,
		.of_match_table = xgene_trng_of_match,
	},
};

module_platform_driver(xgene_trng_driver);

MODULE_AUTHOR("Shamal Winchurkar <swinchurkar@apm.com>");
MODULE_DESCRIPTION("APM X-Gene TRNG driver");
MODULE_LICENSE("GPL");
