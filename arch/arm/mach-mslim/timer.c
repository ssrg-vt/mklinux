/*
 *  linux/arch/arm/mach-mslim/timer.c
 *
 *  Copyright (c) 2012, Applied Micro Circuits Corporation
 *  Author: Vinayak Kale <vkale@apm.com>
 * 
 *  Based on linux/arch/arm/kernel/smp_twd.c
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/smp.h>
#include <linux/jiffies.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include <asm/smp_twd.h>

#ifndef CONFIG_SMP
static void __iomem *twd_base;
static unsigned long twd_timer_rate = 250000000;
static int twd_ppi;

static void twd_set_mode(enum clock_event_mode mode,
			struct clock_event_device *clk)
{
	unsigned long ctrl;
	//printk("************inside %s:mode=%d\n", __FUNCTION__,mode);

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		/* timer load already set up */
		ctrl = TWD_TIMER_CONTROL_ENABLE | TWD_TIMER_CONTROL_IT_ENABLE
			| TWD_TIMER_CONTROL_PERIODIC;
		__raw_writel(DIV_ROUND_CLOSEST(twd_timer_rate, HZ), twd_base + TWD_TIMER_LOAD);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		/* period set, and timer enabled in 'next_event' hook */
		ctrl = TWD_TIMER_CONTROL_IT_ENABLE | TWD_TIMER_CONTROL_ONESHOT;
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		ctrl = 0;
	}

	__raw_writel(ctrl, twd_base + TWD_TIMER_CONTROL);
}

static int twd_set_next_event(unsigned long evt,
			struct clock_event_device *unused)
{
	unsigned long ctrl = __raw_readl(twd_base + TWD_TIMER_CONTROL);
	//printk("************inside %s:\n", __FUNCTION__);

	ctrl |= TWD_TIMER_CONTROL_ENABLE;

	__raw_writel(evt, twd_base + TWD_TIMER_COUNTER);
	__raw_writel(ctrl, twd_base + TWD_TIMER_CONTROL);

	return 0;
}

/*
 * local_timer_ack: checks for a local timer interrupt.
 *
 * If a local timer interrupt has occurred, acknowledge and return 1.
 * Otherwise, return 0.
 */
static int twd_timer_ack(void)
{
	if (__raw_readl(twd_base + TWD_TIMER_INTSTAT)) {
		__raw_writel(1, twd_base + TWD_TIMER_INTSTAT);
		return 1;
	}

	return 0;
}

static irqreturn_t twd_handler(int irq, void *dev_id)
{
	struct clock_event_device *evt = dev_id;

	if (twd_timer_ack()) {
		evt->event_handler(evt);
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static struct clk *twd_get_clock(void)
{
	struct clk *clk;
	int err;

	clk = clk_get_sys("smp_twd", NULL);
	if (IS_ERR(clk)) {
		pr_err("smp_twd: clock not found: %d\n", (int)PTR_ERR(clk));
		return clk;
	}

	err = clk_prepare(clk);
	if (err) {
		pr_err("smp_twd: clock failed to prepare: %d\n", err);
		clk_put(clk);
		return ERR_PTR(err);
	}

	err = clk_enable(clk);
	if (err) {
		pr_err("smp_twd: clock failed to enable: %d\n", err);
		clk_unprepare(clk);
		clk_put(clk);
		return ERR_PTR(err);
	}

	return clk;
}
/*
 * Setup the local clock events for a CPU.
 */
static int __cpuinit twd_timer_setup(struct clock_event_device *clk)
{
	struct clk *twd_clk;

	twd_clk = twd_get_clock();
        if (!IS_ERR_OR_NULL(twd_clk))
                twd_timer_rate = clk_get_rate(twd_clk);

	//printk("************inside %s:twd_timer_rate = %lu\n", __FUNCTION__,twd_timer_rate);

	__raw_writel(0, twd_base + TWD_TIMER_CONTROL);

	clk->name = "local_timer";
	clk->features = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT |
			CLOCK_EVT_FEAT_C3STOP;
	clk->rating = 350;
	clk->set_mode = twd_set_mode;
	clk->set_next_event = twd_set_next_event;
	clk->irq = twd_ppi;
#ifdef CONFIG_SMP
	clk->cpumask = cpumask_of(smp_processor_id());
#endif

	clockevents_config_and_register(clk, twd_timer_rate,
					0xf, 0xffffffff);
	enable_percpu_irq(clk->irq, 0);

	return 0;
}

static struct clock_event_device __percpu percpu_clockevent;

static int __init twd_local_timer_common_register(void)
{
	int err;

	err = request_percpu_irq(twd_ppi, twd_handler, "timer", &percpu_clockevent);
	if (err) {
		pr_err("twd: can't register interrupt %d (%d)\n", twd_ppi, err);
		goto out_unmap;
	}

	err = twd_timer_setup(&percpu_clockevent);
	if (err)
		goto out_irq;

	return 0;

out_irq:
        free_percpu_irq(twd_ppi, &percpu_clockevent);
out_unmap:
	iounmap(twd_base);
        twd_base = NULL;
	return err;
}


#ifdef CONFIG_OF
const static struct of_device_id twd_of_match[] __initconst = {
	{ .compatible = "arm,cortex-a5-twd-timer",	},
        { },
};

void __init local_timer_of_register(void)
{
	struct device_node *np;
	int err;

	np = of_find_matching_node(NULL, twd_of_match);
	if (!np) {
		err = -ENODEV;
		goto out;
	}

	twd_ppi = irq_of_parse_and_map(np, 0);
	if (!twd_ppi) {
		err = -EINVAL;
		goto out;
	}

	twd_base = of_iomap(np, 0);
	if (!twd_base) {
		err = -ENOMEM;
		goto out;
	}

	err = twd_local_timer_common_register();

out:
	WARN(err, "twd_local_timer_of_register failed (%d)\n", err);
}
#endif
#endif
