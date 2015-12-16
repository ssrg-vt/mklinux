/*
 * X-Gene PCP PMU
 *
 * Copyright (c) 2013, Applied Micro Circuits Corporation
 * Author: Vinayak Kale <vkale@apm.com>
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
 *
 */
#include <linux/module.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

#ifdef PCPPMU_SINGLE_EVENT_MONITOR
#define PCPPMU_LATENCY_EVENT_NO_SUPPORT
#endif

#define CSWCR_REG		0x000
#define MCBADDRMR_REG		0x000
#define CSWCR_DUAL_MCB_MODE	(1 << 0)
#define MCBADDRMR_DUAL_MCU_MODE	(1 << 2)

#define PCPPMU_INTSTATUS_REG	0x000
#define PCPPMU_INTMASK_REG	0x004
#define PCPPMU_INT_MCU		(1 << 0)
#define PCPPMU_INT_MCB		(1 << 1)
#define PCPPMU_INT_L3C		(1 << 2)
#define PCPPMU_INT_IOB		(1 << 3)

#define L3CPMU_MAX_EVENTS	16
#define MCUPMU_MAX_EVENTS	29

#define PMU_LAT_COUNTERS	2

#ifdef PCPPMU_SINGLE_EVENT_MONITOR
#define PMU_MAX_COUNTERS	1
#else
#define PMU_MAX_COUNTERS	4
#endif

#define MCUPMU_MAX_NUM		4
#define MCUPMU_NEXT_PMU_OFFSET  0x40000 	

#define MCBPMU_MAX_NUM		2
#define MCBPMU_NEXT_PMU_OFFSET  0x20000 

#define L3CPMU_PMAMR0_VAL	0x3FF
#define IOBPMU_PMAMR0_VAL	0x3FF
#define IOBPMU_PMAMR1_VAL	0x01FFFFFF
#define MCBPMU_PMAMR0_VAL	0x3FF

#define PMU_CNT_MAX_VAL		0x100000000ULL
#define PMU_EVTYPE_MASK		0xFF
#define PMU_OVERFLOW_MASK	0xF
#define PMU_LATCNT_MASK		0x3
#define PMU_PMCR_E		(1 << 0)
#define PMU_PMCR_P		(1 << 1)

#define PMU_PMEVCNTR0		0x000
#define PMU_PMEVCNTR1		0x004
#define PMU_PMEVCNTR2		0x008
#define PMU_PMEVCNTR3		0x00C
#define PMU_PMEVTYPER0		0x400
#define PMU_PMEVTYPER1		0x404
#define PMU_PMEVTYPER2		0x408
#define PMU_PMEVTYPER3		0x40C
#define PMU_PMAMR0		0xA00
#define PMU_PMAMR1		0xA04
#define PMU_PMCNTENSET		0xC00
#define PMU_PMCNTENCLR		0xC20
#define PMU_PMINTENSET		0xC40
#define PMU_PMINTENCLR		0xC60
#define PMU_PMOVSR		0xC80
#define PMU_PMLCSR		0xC84
#define PMU_PMCFGR		0xE00
#define PMU_PMCR		0xE04
#define PMU_PMCEID0		0xE20
#define PMU_PMCEID1		0xE24
#define PMU_DEVARCH		0xFBC

#define SET_BIT(val, bit)	((val) |= (1 << (bit)))
#define IS_BIT_SET(val, bit)	((val) & (1 << (bit)))

enum pmu_type {
	PMU_TYPE_MCU = 0,
	PMU_TYPE_MCB,
	PMU_TYPE_L3C,
	PMU_TYPE_IOB
};

struct pmu {
	enum pmu_type	type;
	void 		*base;
	spinlock_t	lock;
	u32		num_cnt;
	u64 		cnt[PMU_MAX_COUNTERS];
	u32		evt[PMU_MAX_COUNTERS];
	u32 		valid_cnt_bitmap;
	u32		lat_cnt_bitmap;
};

struct xgene_pmu {
	struct  platform_device	*pdev;
	u32 	irq;
	void 	*pcpint_base;
	void 	*csw_base;
	void 	*mcb0_base;
	void 	*mcb1_base;

	struct pmu mcupmu[MCUPMU_MAX_NUM];
	struct pmu mcbpmu[MCBPMU_MAX_NUM];
	struct pmu l3cpmu;
	struct pmu iobpmu;
};

static struct xgene_pmu *xpmu = NULL;

static inline struct pmu * _get_pmu_ptr(enum pmu_type type, u32 indx)
{
	if (!xpmu)
		return NULL;

	if (PMU_TYPE_MCU == type)
		return ((xpmu->mcupmu[indx].base == NULL) ? NULL : 
				&xpmu->mcupmu[indx]);
	else if (PMU_TYPE_MCB == type)
		return ((xpmu->mcbpmu[indx].base == NULL) ? NULL :
				&xpmu->mcbpmu[indx]);
	else if (PMU_TYPE_L3C == type)
		return &xpmu->l3cpmu;
	else if (PMU_TYPE_IOB == type)
		return &xpmu->iobpmu;
	else
		return NULL;
}

static inline u32 is_valid_iob_event(u32 evt)
{
	if ((evt >= 0x0 && evt <= 0x7) || 
			(evt >= 0x10 && evt <= 0x11) ||
			(evt >= 0x13 && evt <= 0x14) ||
			(evt == 0x16) ||
			(evt >= 0x32 && evt <= 0x37))
		return 1;
	else
		return 0; 
}

static inline u32 is_valid_mcb_event(u32 evt)
{
	if ((evt >= 0x0 && evt <= 0x5) || (evt >= 0x30 && evt <= 0x31))
		return 1;
	else
		return 0; 
}

static inline u32 is_valid_l3c_event(u32 evt)
{
	if ((evt >= 0x0 && evt <= 0x3) || (evt >= 0x6 && evt <= 0xf))
		return 1;
	else
		return 0; 
}

static inline u32 is_latency_event(u32 evt)
{
	return ((0x30 == (evt & 0x30)) ? 1 : 0);	
}

static inline u32 pmu_is_valid_counter(struct pmu *pmu, u32 id)
{
	return IS_BIT_SET(pmu->valid_cnt_bitmap, id);
}

static inline u32 pmu_is_latency_counter(struct pmu *pmu, u32 id)
{
	if (pmu_is_valid_counter(pmu, id))
		return IS_BIT_SET(pmu->lat_cnt_bitmap, id);
	else
		return 0;
}

static inline void pmu_enable_counter(struct pmu *pmu, u32 id)
{
	/* No need of read-modify-write, since write of '0' 
	 * is ignored.
	 */
	writel((1 << id), pmu->base + PMU_PMCNTENSET);
}

static inline void pmu_disable_counter(struct pmu *pmu, u32 id)
{
	/* No need of read-modify-write, since write of '0' 
	 * is ignored.
	 */
	writel((1 << id), pmu->base + PMU_PMCNTENCLR);
}

static inline void pmu_enable_intens(struct pmu *pmu, u32 id)
{
	/* No need of read-modify-write, since write of '0' 
	 * is ignored.
	 */
	writel((1 << id), pmu->base + PMU_PMINTENSET);
}

static inline void pmu_disable_intens(struct pmu *pmu, u32 id)
{
	/* No need of read-modify-write, since write of '0' 
	 * is ignored.
	 */
	writel((1 << id), pmu->base + PMU_PMINTENCLR);
}

static inline void pmu_write_evtype(struct pmu *pmu, u32 id, u32 evt)
{
	evt &= PMU_EVTYPE_MASK;
	writel(evt, pmu->base + PMU_PMEVTYPER0 + (id * 4));
}

static inline u32 pmu_read_counter(struct pmu *pmu, u32 id)
{
	return readl(pmu->base + PMU_PMEVCNTR0 + (id * 4));
}

static inline u32 pmu_getreset_overflow_flags(struct pmu *pmu)
{
	u32 val;

	val = readl(pmu->base + PMU_PMOVSR);
	val &= PMU_OVERFLOW_MASK;

	/* clear the bits */
	writel(0x0, pmu->base + PMU_PMOVSR);

	return val;
}

static inline u32 pmu_getreset_latcnt_status(struct pmu *pmu)
{
	u32 val;

	if (!((PMU_TYPE_MCB == pmu->type) || (PMU_TYPE_IOB == pmu->type)))
		return 0;

	val = readl(pmu->base + PMU_PMLCSR);
	val &= PMU_LATCNT_MASK;

	/* clear the bits */
	writel(val, pmu->base + PMU_PMLCSR);

	return val;
}

static inline u32 pmu_is_latency_counter_done(u32 pmlcsr, u32 id)
{
	if (id < PMU_LAT_COUNTERS)
		return 0;
	else return (pmlcsr & (1 << (id - PMU_LAT_COUNTERS)));
}

static inline u32 pmu_counter_has_overflowed(u32 pmovsr, u32 id)
{
	return (pmovsr & (1 << id));
}

static inline u32 pmu_read_num_counters(struct pmu *pmu)
{
	u32 val;

	val = readl(pmu->base + PMU_PMCFGR);
	val &= 0x1F;

	return val + 1;
}

static inline u32 pmu_read_devarchid(struct pmu *pmu)
{
	u32 val;

	val = readl(pmu->base + PMU_DEVARCH);
	val &= 0xFFF;

	return val;
}

static inline void pmu_init_agent_mask(struct pmu *pmu)
{
	switch (pmu->type) {
	case PMU_TYPE_MCU:
		break;
	case PMU_TYPE_MCB:
		writel(MCBPMU_PMAMR0_VAL, pmu->base + PMU_PMAMR0);
		break;
	case PMU_TYPE_L3C:
		writel(L3CPMU_PMAMR0_VAL, pmu->base + PMU_PMAMR0);
		break;
	case PMU_TYPE_IOB:
		writel(IOBPMU_PMAMR0_VAL, pmu->base + PMU_PMAMR0);
		writel(IOBPMU_PMAMR1_VAL, pmu->base + PMU_PMAMR1);
		break;
	}
}

static inline void pmu_start_counters(struct pmu *pmu)
{
	u32 val;

	val = readl(pmu->base + PMU_PMCR);

	val |= PMU_PMCR_E;

	writel(val, pmu->base + PMU_PMCR);
}

static inline void pmu_stop_counters(struct pmu *pmu)
{
	u32 val;

	val = readl(pmu->base + PMU_PMCR);

	val &= ~PMU_PMCR_E;

	writel(val, pmu->base + PMU_PMCR);
}

static inline void pmu_reset_counters(struct pmu *pmu)
{
	u32 val;

	val = readl(pmu->base + PMU_PMCR);

	val |= PMU_PMCR_P;

	writel(val, pmu->base + PMU_PMCR);
}

static void pmu_reset(struct pmu *pmu)
{
	u32 i;
	unsigned long flags;

	if (!pmu)
		return;

	spin_lock_irqsave(&pmu->lock, flags);

	pmu_stop_counters(pmu);
	pmu_reset_counters(pmu);

	for (i=0; i<pmu->num_cnt && i<PMU_MAX_COUNTERS; i++) {
		pmu_disable_counter(pmu, i);		
		pmu_disable_intens(pmu, i);

		pmu->cnt[i] = 0;
		pmu->evt[i] = 0;
	}

	pmu_getreset_overflow_flags(pmu);
	pmu_getreset_latcnt_status(pmu);
	
	pmu->valid_cnt_bitmap = 0;
	pmu->lat_cnt_bitmap = 0;

	spin_unlock_irqrestore(&pmu->lock, flags);
}

static void pmu_start(struct pmu *pmu, u32 evt_list[PMU_MAX_COUNTERS], 
				u32 valid_cnt_bitmap, u32 lat_cnt_bitmap)
{
	u32 i;
	unsigned long flags;

	if (!pmu)
		return;

	spin_lock_irqsave(&pmu->lock, flags);

	/* enable individual counters & interrupt */
	for (i=0; i<pmu->num_cnt && i<PMU_MAX_COUNTERS; i++) {
		if (!IS_BIT_SET(valid_cnt_bitmap, i))
			continue;

		pmu_write_evtype(pmu, i, evt_list[i]);		
		pmu_enable_intens(pmu, i);
		pmu_enable_counter(pmu, i);

		pmu->evt[i] = evt_list[i];
	}

	pmu->valid_cnt_bitmap = valid_cnt_bitmap;
	pmu->lat_cnt_bitmap = lat_cnt_bitmap;

	/* start counters */
	pmu_start_counters(pmu);

	spin_unlock_irqrestore(&pmu->lock, flags);
}

static void pmu_stop(struct pmu *pmu)
{
	u32 i;
	u32 pmovsr;
	u32 pmlcsr;
	unsigned long flags;

	if (!pmu)
		return;

	spin_lock_irqsave(&pmu->lock, flags);

	/* stop counters */
	pmu_stop_counters(pmu);

	/* disable individual counters & interrupt */
	for (i=0; i<pmu->num_cnt; i++) {
		if (!pmu_is_valid_counter(pmu, i))
			continue;

		pmu_disable_counter(pmu, i);		
		pmu_disable_intens(pmu, i);

	}

	pmovsr = pmu_getreset_overflow_flags(pmu);
	pmlcsr = pmu_getreset_latcnt_status(pmu);
	
	/* read the counter value */
	for (i=0; i<pmu->num_cnt && i<PMU_MAX_COUNTERS; i++) {
		if (!pmu_is_valid_counter(pmu, i))
			continue;

		if (pmu_is_latency_counter(pmu, i)) {
			if (!pmu_counter_has_overflowed(pmovsr, i) 
				&& pmu_is_latency_counter_done(pmlcsr, i))
				pmu->cnt[i] = pmu_read_counter(pmu, i);
			else
				pmu->cnt[i] = 0;
		} else {
			pmu->cnt[i] += pmu_read_counter(pmu, i);
		}
	}

	spin_unlock_irqrestore(&pmu->lock, flags);
}

static void pmu_init(struct pmu *pmu, enum pmu_type type)
{
	if (!pmu)
		return;

#if 0
	printk("inside %s:PMU type=%d, devarch-id=0x%x\n", 
			__func__, type, pmu_read_devarchid(pmu));
#endif

	pmu->type = type;
	spin_lock_init(&pmu->lock);

	pmu->num_cnt = pmu_read_num_counters(pmu);

	if (!pmu->num_cnt)
		return;

	if (pmu->num_cnt > PMU_MAX_COUNTERS) {
		pmu->num_cnt = PMU_MAX_COUNTERS;
	}

	pmu_reset(pmu);
	pmu_init_agent_mask(pmu);
}

static inline void xgene_pmu_mask_int(void)
{
	writel(0xffffffff, xpmu->pcpint_base + PCPPMU_INTMASK_REG);
}

static inline void xgene_pmu_unmask_int(void)
{
	writel(0xfffffff0, xpmu->pcpint_base + PCPPMU_INTMASK_REG);
}

static void xgene_pmu_hw_init(void)
{
	u32 i;

	pmu_init(_get_pmu_ptr(PMU_TYPE_L3C, 0), PMU_TYPE_L3C);

	for (i=0; i < MCUPMU_MAX_NUM; i++) {
		pmu_init(_get_pmu_ptr(PMU_TYPE_MCU, i), PMU_TYPE_MCU);
	}

	pmu_init(_get_pmu_ptr(PMU_TYPE_IOB, 0), PMU_TYPE_IOB);

	for (i=0; i < MCBPMU_MAX_NUM; i++) {
		pmu_init(_get_pmu_ptr(PMU_TYPE_MCB, i), PMU_TYPE_MCB);
	}

	xgene_pmu_unmask_int();
	return;
}

static void pmu_irq(struct pmu *pmu)
{
	u32 pmovsr;
	u32 i;

	if (!pmu)
		return;

	pmovsr = pmu_getreset_overflow_flags(pmu);

	for (i=0; i<pmu->num_cnt; i++) {
		if (!pmu_is_valid_counter(pmu, i))
			continue;
		if (!pmu_counter_has_overflowed(pmovsr, i))
			continue;
		if (pmu_is_latency_counter(pmu, i))
			continue;

		pmu->cnt[i] += PMU_CNT_MAX_VAL;
	}
}

static irqreturn_t xgene_pmu_irq(int irq_num, void *dev)
{
	u32 val = 0;
	u32 i;

	val = readl(xpmu->pcpint_base + PCPPMU_INTSTATUS_REG);

	if (!val)
		return IRQ_NONE;

	xgene_pmu_mask_int();

	if (val & PCPPMU_INT_MCU) {
		for (i=0; i < MCUPMU_MAX_NUM; i++) {
			pmu_irq(_get_pmu_ptr(PMU_TYPE_MCU, i));
		}
	}

	if (val & PCPPMU_INT_MCB) {
		for (i=0; i < MCBPMU_MAX_NUM; i++) {
			pmu_irq(_get_pmu_ptr(PMU_TYPE_MCB, i));
		}
	}

	if (val & PCPPMU_INT_L3C)
		pmu_irq(_get_pmu_ptr(PMU_TYPE_L3C, 0));

	if (val & PCPPMU_INT_IOB)
		pmu_irq(_get_pmu_ptr(PMU_TYPE_IOB, 0));

	xgene_pmu_unmask_int();

	return IRQ_HANDLED;
}

static int xgene_pmu_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct resource l3cpmu;
	struct resource mcupmu;
	struct resource iobpmu;
	struct resource mcbpmu;
	struct resource pcpint;
	struct resource cswreg;
	struct resource mcbreg;
	u32 irq;
	u32 indx;
	u32 cswcr;
	u32 mcb0addrmr;
	u32 mcb1addrmr;
	u32 mcu_active_bitmap;
	u32 mcb_active_bitmap;
	int rc;

	rc = of_address_to_resource(np, 0, &pcpint);
	if (rc != 0) {
		dev_err(&pdev->dev, "PCP INT node not present in DTS\n");
		rc = -ENODEV;
		goto err;
	}

	rc = of_address_to_resource(np, 1, &l3cpmu);
	if (rc != 0) {
		dev_err(&pdev->dev, "L3C PMU node not present in DTS\n");
		rc = -ENODEV;
		goto err;
	}

	rc = of_address_to_resource(np, 2, &mcupmu);
	if (rc != 0) {
		dev_err(&pdev->dev, "MCU PMU node not present in DTS\n");
		rc = -ENODEV;
		goto err;
	}

	rc = of_address_to_resource(np, 3, &iobpmu);
	if (rc != 0) {
		dev_err(&pdev->dev, "IOB PMU node not present in DTS\n");
		rc = -ENODEV;
		goto err;
	}

	rc = of_address_to_resource(np, 4, &mcbpmu);
	if (rc != 0) {
		dev_err(&pdev->dev, "MCB PMU node not present in DTS\n");
		rc = -ENODEV;
		goto err;
	}

	rc = of_address_to_resource(np, 5, &cswreg);
	if (rc != 0) {
		dev_err(&pdev->dev, "CSW REG node not present in DTS\n");
		rc = -ENODEV;
		goto err;
	}

	rc = of_address_to_resource(np, 6, &mcbreg);
	if (rc != 0) {
		dev_err(&pdev->dev, "MCB REG node not present in DTS\n");
		rc = -ENODEV;
		goto err;
	}

	irq = of_irq_to_resource(np, 0, NULL);
        if (irq == 0) {
		dev_err(&pdev->dev, "PMU IRQ number not present in DTS\n");
		rc = -ENODEV;
		goto err;
	}

	xpmu = kzalloc(sizeof(*xpmu), GFP_KERNEL);
	if (xpmu == NULL) {
		dev_err(&pdev->dev, "Memory alloc error for PMU struct\n");
		rc = -ENOMEM;
		goto err;
	}

	platform_set_drvdata(pdev, xpmu);
	xpmu->pdev = pdev;

	xpmu->l3cpmu.base = ioremap(l3cpmu.start,
				l3cpmu.end - l3cpmu.start + 1);
	if (xpmu->l3cpmu.base == NULL) {
		dev_err(&pdev->dev, "ioremap failed for L3C PMU registers\n");
		rc = -ENOMEM;
		goto err;
	}

	xpmu->iobpmu.base = ioremap(iobpmu.start,
				iobpmu.end - iobpmu.start + 1);
	if (xpmu->iobpmu.base == NULL) {
		dev_err(&pdev->dev, "ioremap failed for IOB PMU registers\n");
		rc = -ENOMEM;
		goto err;
	}

	xpmu->csw_base = ioremap(cswreg.start,
				cswreg.end - cswreg.start + 1);
	if (xpmu->csw_base == NULL) {
		dev_err(&pdev->dev, "ioremap failed for CSW registers\n");
		rc = -ENOMEM;
		goto err;
	}
	xpmu->mcb0_base = ioremap(mcbreg.start,
				mcbreg.end - mcbreg.start + 1);
	if (xpmu->mcb0_base == NULL) {
		dev_err(&pdev->dev, "ioremap failed for MCB-0 registers\n");
		rc = -ENOMEM;
		goto err;
	}
	xpmu->mcb1_base = ioremap(mcbreg.start + 0x20000,
				mcbreg.end - mcbreg.start + 1);
	if (xpmu->mcb1_base == NULL) {
		dev_err(&pdev->dev, "ioremap failed for MCB-1 registers\n");
		rc = -ENOMEM;
		goto err;
	}

	mcu_active_bitmap = 0;
	mcb_active_bitmap = 0;
	cswcr = readl(xpmu->csw_base + CSWCR_REG);
	if (cswcr & CSWCR_DUAL_MCB_MODE) { /* Dual MCB mode */
		SET_BIT(mcb_active_bitmap, 0);
		SET_BIT(mcb_active_bitmap, 1);

		mcb0addrmr = readl(xpmu->mcb0_base + MCBADDRMR_REG);
		mcb1addrmr = readl(xpmu->mcb1_base + MCBADDRMR_REG);

		if (mcb0addrmr & MCBADDRMR_DUAL_MCU_MODE) {
			SET_BIT(mcu_active_bitmap, 0);
			SET_BIT(mcu_active_bitmap, 1);
		} else {
			SET_BIT(mcu_active_bitmap, 0);
		}

		if (mcb1addrmr & MCBADDRMR_DUAL_MCU_MODE) {
			SET_BIT(mcu_active_bitmap, 2);
			SET_BIT(mcu_active_bitmap, 3);
		} else {
			SET_BIT(mcu_active_bitmap, 2);
		}
	} else { /* Single MCB mode */
		SET_BIT(mcb_active_bitmap, 0);

		mcb0addrmr = readl(xpmu->mcb0_base + MCBADDRMR_REG);

		if (mcb0addrmr & MCBADDRMR_DUAL_MCU_MODE) {
			SET_BIT(mcu_active_bitmap, 0);
			SET_BIT(mcu_active_bitmap, 1);
		} else {
			SET_BIT(mcu_active_bitmap, 0);
		}
	}	

#if 0
	printk("inside %s:CSWCR=0x%x, MCB0-ADDRMR=0x%x, MCB1-ADDRMR=0x%x\n", 
			__func__, cswcr, mcb0addrmr, mcb1addrmr);
	printk("inside %s:Active MCB bitmap=0x%x Active MCU bitmap=0x%x\n", 
			__func__, mcb_active_bitmap, mcu_active_bitmap);
#endif

	for (indx=0; indx < MCBPMU_MAX_NUM; indx++) {
		if (!IS_BIT_SET(mcb_active_bitmap, indx))
			continue;	

		xpmu->mcbpmu[indx].base = ioremap(mcbpmu.start + 
						(indx * MCBPMU_NEXT_PMU_OFFSET),
						mcbpmu.end - mcbpmu.start + 1);
		if (xpmu->mcbpmu[indx].base == NULL) {
			dev_err(&pdev->dev, "ioremap failed for MCB PMU registers\n");
			rc = -ENOMEM;
			goto err;
		}
	}

	for (indx=0; indx < MCUPMU_MAX_NUM; indx++) {
		if (!IS_BIT_SET(mcu_active_bitmap, indx))
			continue;	

		xpmu->mcupmu[indx].base = ioremap(mcupmu.start + 
						(indx * MCUPMU_NEXT_PMU_OFFSET),
						mcupmu.end - mcupmu.start + 1);
		if (xpmu->mcupmu[indx].base == NULL) {
			dev_err(&pdev->dev, "ioremap failed for MCU PMU registers\n");
			rc = -ENOMEM;
			goto err;
		}
	}

	xpmu->pcpint_base = ioremap(pcpint.start,
				pcpint.end - pcpint.start + 1);
	if (xpmu->pcpint_base == NULL) {
		dev_err(&pdev->dev, "ioremap failed for PCP INT registers\n");
		rc = -ENOMEM;
		goto err;
	}

	rc = request_irq(irq, xgene_pmu_irq, 0, "PCP PMU", xpmu);
	if (rc != 0) {
		dev_err(&pdev->dev, "Unable to register IRQ %d\n", irq);
		goto err;
	}
	xpmu->irq  = irq;

	xgene_pmu_hw_init();

	pr_info("X-Gene: PCP PMU driver\n");

	return rc;
err:
        if (xpmu && xpmu->irq != 0)
                free_irq(xpmu->irq, xpmu);
	if (xpmu && xpmu->l3cpmu.base)
		iounmap(xpmu->l3cpmu.base);
	for (indx=0; indx < MCUPMU_MAX_NUM; indx++) {
		if (xpmu && xpmu->mcupmu[indx].base)
			iounmap(xpmu->mcupmu[indx].base);
	}
	if (xpmu && xpmu->iobpmu.base)
		iounmap(xpmu->iobpmu.base);
	for (indx=0; indx < MCBPMU_MAX_NUM; indx++) {
		if (xpmu && xpmu->mcbpmu[indx].base)
			iounmap(xpmu->mcbpmu[indx].base);
	}
	if (xpmu && xpmu->pcpint_base)
		iounmap(xpmu->pcpint_base);
	if (xpmu && xpmu->csw_base)
		iounmap(xpmu->csw_base);
	if (xpmu && xpmu->mcb0_base)
		iounmap(xpmu->mcb0_base);
	if (xpmu && xpmu->mcb1_base)
		iounmap(xpmu->mcb1_base);
	if (xpmu)
		kfree(xpmu);	

	platform_set_drvdata(pdev, NULL);

	return rc;
}

static int xgene_pmu_remove(struct platform_device *pdev)
{
	int indx;

	/* mask PMU interrupt */
	xgene_pmu_mask_int();

	/* reset PMUs */
	pmu_reset(_get_pmu_ptr(PMU_TYPE_L3C, 0));

	for (indx=0; indx < MCUPMU_MAX_NUM; indx++) {
		pmu_reset(_get_pmu_ptr(PMU_TYPE_MCU, indx));
	}

	pmu_reset(_get_pmu_ptr(PMU_TYPE_IOB, 0));

	for (indx=0; indx < MCBPMU_MAX_NUM; indx++) {
		pmu_reset(_get_pmu_ptr(PMU_TYPE_MCB, indx));
	}

	/* free IRQ and iounmap */
        if (xpmu && xpmu->irq != 0)
                free_irq(xpmu->irq, xpmu);
	if (xpmu && xpmu->l3cpmu.base)
		iounmap(xpmu->l3cpmu.base);
	for (indx=0; indx < MCUPMU_MAX_NUM; indx++) {
		if (xpmu && xpmu->mcupmu[indx].base)
			iounmap(xpmu->mcupmu[indx].base);
	}
	if (xpmu && xpmu->iobpmu.base)
		iounmap(xpmu->iobpmu.base);
	for (indx=0; indx < MCBPMU_MAX_NUM; indx++) {
		if (xpmu && xpmu->mcbpmu[indx].base)
			iounmap(xpmu->mcbpmu[indx].base);
	}
	if (xpmu && xpmu->pcpint_base)
		iounmap(xpmu->pcpint_base);
	if (xpmu && xpmu->csw_base)
		iounmap(xpmu->csw_base);
	if (xpmu && xpmu->mcb0_base)
		iounmap(xpmu->mcb0_base);
	if (xpmu && xpmu->mcb1_base)
		iounmap(xpmu->mcb1_base);
	if (xpmu)
		kfree(xpmu);	

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id xgene_pmu_of_device_ids[] = {
        { .compatible   = "apm,xgene-pcppmu", },
	{},
};
MODULE_DEVICE_TABLE(of, xgene_pmu_of_device_ids);

static struct platform_driver xgene_pmu_driver = {
	.driver = {
		.name 		= "xgene-pcppmu",
		.owner		= THIS_MODULE,
		.of_match_table = xgene_pmu_of_device_ids,
	},
	.probe = xgene_pmu_probe,
	.remove = xgene_pmu_remove,
};

static ssize_t xgene_l3cpmu_show(struct device_driver *drv, char *buf)
{
	u32 i;
	struct pmu *pmu=_get_pmu_ptr(PMU_TYPE_L3C, 0); 

	if (!pmu)
		return 0;

	if (0 == pmu->valid_cnt_bitmap)
		return 0;

	pmu_stop(pmu);

	sprintf(buf,
		"L3C PMU Counter stats:\n\n"
		"\tEvent-id\tCounter\n");

	for (i=0; i<pmu->num_cnt; i++) {
		if (!pmu_is_valid_counter(pmu, i))
			continue;
		sprintf(buf + strlen(buf),
			"\t  0x%x\t\t%llu\n",pmu->evt[i],pmu->cnt[i]);
	}
	pmu_reset(pmu);

	return strlen(buf);
}

static ssize_t xgene_l3cpmu_set(struct device_driver *drv,
                                     const char *buf, size_t count)
{
	char *str=NULL;
	char *token=NULL;
	char *tmp=NULL;
	u32 evt_list[PMU_MAX_COUNTERS];
	u32 evt_cnt = 0;
	u32 valid_cnt_bitmap = 0;
	struct pmu *pmu=_get_pmu_ptr(PMU_TYPE_L3C, 0); 

	if (!pmu)
		return 0;

	if (!count)
		return count;

	str = tmp = (char *) kzalloc(count, GFP_KERNEL);
	if (str == NULL)
		return count;

	strcpy(str, buf);
	
	while ((token = strsep(&str, ",")) != NULL) {
		if (evt_cnt == pmu->num_cnt)
			break;

		if (0 != kstrtou32(token, 0, &evt_list[evt_cnt]))
			continue;

		if (!is_valid_l3c_event(evt_list[evt_cnt]))
			continue;

		SET_BIT(valid_cnt_bitmap, evt_cnt);
		evt_cnt++;
	}
	kfree(tmp);

	if (0 == evt_cnt)
		return count;

	pmu_reset(pmu);
	pmu_start(pmu, evt_list, valid_cnt_bitmap, 0);

	return count;
}

static ssize_t xgene_mcupmu_show(struct device_driver *drv, char *buf)
{
	u32 i;
	u32 indx;
	struct pmu *pmu;

	for (indx=0; indx < MCUPMU_MAX_NUM; indx++) {
		pmu = _get_pmu_ptr(PMU_TYPE_MCU, indx); 

		if (!pmu)
			continue;

		if (0 == pmu->valid_cnt_bitmap)
			continue;

		pmu_stop(pmu);

		sprintf(buf + strlen(buf),
			"\nMCU%u PMU Counter stats:\n\n"
			"\tEvent-id\tCounter\n", indx);

		for (i=0; i<pmu->num_cnt; i++) {
			if (!pmu_is_valid_counter(pmu, i))
				continue;
			sprintf(buf + strlen(buf),
				"\t  0x%x\t\t%llu\n",pmu->evt[i],pmu->cnt[i]);
		}
		pmu_reset(pmu);
	}

	return strlen(buf);
}

static ssize_t xgene_mcupmu_set(struct device_driver *drv,
                                     const char *buf, size_t count)
{
	char *str=NULL;
	char *token=NULL;
	char *tmp=NULL;
	u32 evt_list[PMU_MAX_COUNTERS];
	u32 evt_cnt = 0;
	u32 valid_cnt_bitmap = 0;
	u32 indx;
	struct pmu *pmu=_get_pmu_ptr(PMU_TYPE_MCU, 0); 

	if (!pmu)
		return 0;

	if (!count)
		return count;

	str = tmp = (char *) kzalloc(count, GFP_KERNEL);
	if (str == NULL)
		return count;

	strcpy(str, buf);
	
	while ((token = strsep(&str, ",")) != NULL) {
		if (evt_cnt == pmu->num_cnt)
			break;

		if (0 != kstrtou32(token, 0, &evt_list[evt_cnt]))
			continue;

		if (evt_list[evt_cnt] >= MCUPMU_MAX_EVENTS)
			continue;

		SET_BIT(valid_cnt_bitmap, evt_cnt);
		evt_cnt++;
	}
	kfree(tmp);

	if (0 == evt_cnt)
		return count;

	for (indx=0; indx < MCUPMU_MAX_NUM; indx++) {
		pmu = _get_pmu_ptr(PMU_TYPE_MCU, indx); 
		if (!pmu)
			continue;

		pmu_reset(pmu);
		pmu_start(pmu, evt_list, valid_cnt_bitmap, 0);
	}

	return count;
}

static ssize_t xgene_iobpmu_show(struct device_driver *drv, char *buf)
{
	u32 i;
	struct pmu *pmu=_get_pmu_ptr(PMU_TYPE_IOB, 0); 

	if (!pmu)
		return 0;

	if (0 == pmu->valid_cnt_bitmap)
		return 0;

	pmu_stop(pmu);

	sprintf(buf,
		"IOB PMU Counter stats:\n\n"
		"\tEvent-id\tCounter\n");

	for (i=0; i<pmu->num_cnt; i++) {
		if (!pmu_is_valid_counter(pmu, i))
			continue;
		sprintf(buf + strlen(buf),
			"\t  0x%x\t\t%llu\n",pmu->evt[i],pmu->cnt[i]);
	}
	pmu_reset(pmu);

	return strlen(buf);
}

static ssize_t xgene_iobpmu_set(struct device_driver *drv,
                                     const char *buf, size_t count)
{
	char *str=NULL;
	char *token=NULL;
	char *tmp=NULL;
	u32 evt_list[PMU_MAX_COUNTERS];
	u32 evt;
	u32 indx;
	u32 total_evt_cnt = 0;
#ifndef PCPPMU_LATENCY_EVENT_NO_SUPPORT
	u32 lat_evt_cnt = 0;
#endif
	u32 valid_cnt_bitmap = 0;
	u32 lat_cnt_bitmap = 0;
	struct pmu *pmu=_get_pmu_ptr(PMU_TYPE_IOB, 0); 

	if (!pmu)
		return 0;

	if (!count)
		return count;

	str = tmp = (char *) kzalloc(count, GFP_KERNEL);
	if (str == NULL)
		return count;

	strcpy(str, buf);
	
	while ((token = strsep(&str, ",")) != NULL) {
		if (total_evt_cnt == pmu->num_cnt)
			break;

		if (0 != kstrtou32(token, 0, &evt))
			continue;

		if (!is_valid_iob_event(evt))
			continue;

		if (is_latency_event(evt)) {
		#ifdef PCPPMU_LATENCY_EVENT_NO_SUPPORT
			continue;
		#else
			if (lat_evt_cnt == PMU_LAT_COUNTERS)
				continue;

			indx = PMU_LAT_COUNTERS + lat_evt_cnt;
			evt_list[indx] = evt;
			SET_BIT(valid_cnt_bitmap, indx);
			SET_BIT(lat_cnt_bitmap, indx);
			lat_evt_cnt++;
			total_evt_cnt++;
		#endif
		} else {
			for (indx=0; indx < pmu->num_cnt; indx++) {
				if (!IS_BIT_SET(valid_cnt_bitmap, indx))
					break;
			}
			if (indx == pmu->num_cnt)
				break;
			evt_list[indx] = evt;
			SET_BIT(valid_cnt_bitmap, indx);
			total_evt_cnt++;
		}
	}
	kfree(tmp);

	if (0 == total_evt_cnt)
		return count;

	pmu_reset(pmu);
	pmu_start(pmu, evt_list, valid_cnt_bitmap, lat_cnt_bitmap);

	return count;
}

static ssize_t xgene_mcbpmu_show(struct device_driver *drv, char *buf)
{
	u32 i;
	u32 indx;
	struct pmu *pmu;

	for (indx=0; indx < MCBPMU_MAX_NUM; indx++) {
		pmu = _get_pmu_ptr(PMU_TYPE_MCB, indx); 

		if (!pmu)
			continue;

		if (0 == pmu->valid_cnt_bitmap)
			continue;

		pmu_stop(pmu);

		sprintf(buf + strlen(buf),
			"\nMCB%u PMU Counter stats:\n\n"
			"\tEvent-id\tCounter\n", indx);

		for (i=0; i<pmu->num_cnt; i++) {
			if (!pmu_is_valid_counter(pmu, i))
				continue;
			sprintf(buf + strlen(buf),
				"\t  0x%x\t\t%llu\n",pmu->evt[i],pmu->cnt[i]);
		}
		pmu_reset(pmu);
	}

	return strlen(buf);
}

static ssize_t xgene_mcbpmu_set(struct device_driver *drv,
                                     const char *buf, size_t count)
{
	char *str=NULL;
	char *token=NULL;
	char *tmp=NULL;
	u32 evt_list[PMU_MAX_COUNTERS];
	u32 evt;
	u32 indx;
	u32 total_evt_cnt = 0;
#ifndef PCPPMU_LATENCY_EVENT_NO_SUPPORT
	u32 lat_evt_cnt = 0;
#endif
	u32 valid_cnt_bitmap = 0;
	u32 lat_cnt_bitmap = 0;
	struct pmu *pmu=_get_pmu_ptr(PMU_TYPE_MCB, 0); 

	if (!pmu)
		return 0;

	if (!count)
		return count;

	str = tmp = (char *) kzalloc(count, GFP_KERNEL);
	if (str == NULL)
		return count;

	strcpy(str, buf);
	
	while ((token = strsep(&str, ",")) != NULL) {
		if (total_evt_cnt == pmu->num_cnt)
			break;

		if (0 != kstrtou32(token, 0, &evt))
			continue;

		if (!is_valid_mcb_event(evt))
			continue;

		if (is_latency_event(evt)) {
		#ifdef PCPPMU_LATENCY_EVENT_NO_SUPPORT
			continue;
		#else
			if (lat_evt_cnt == PMU_LAT_COUNTERS)
				continue;

			indx = PMU_LAT_COUNTERS + lat_evt_cnt;
			evt_list[indx] = evt;
			SET_BIT(valid_cnt_bitmap, indx);
			SET_BIT(lat_cnt_bitmap, indx);
			lat_evt_cnt++;
			total_evt_cnt++;
		#endif
		} else {
			for (indx=0; indx < pmu->num_cnt; indx++) {
				if (!IS_BIT_SET(valid_cnt_bitmap, indx))
					break;
			}
			if (indx == pmu->num_cnt)
				break;
			evt_list[indx] = evt;
			SET_BIT(valid_cnt_bitmap, indx);
			total_evt_cnt++;
		}
	}
	kfree(tmp);

	if (0 == total_evt_cnt)
		return count;

	for (indx=0; indx < MCBPMU_MAX_NUM; indx++) {
		pmu = _get_pmu_ptr(PMU_TYPE_MCB, indx); 
		if (!pmu)
			continue;

		pmu_reset(pmu);
		pmu_start(pmu, evt_list, valid_cnt_bitmap, lat_cnt_bitmap);
	}

	return count;
}

static struct driver_attribute xgene_pmu_sysfs_attrs[] = {
        __ATTR(l3cpmu, S_IWUGO | S_IRUGO,
                xgene_l3cpmu_show, xgene_l3cpmu_set),
        __ATTR(mcupmu, S_IWUGO | S_IRUGO,
                xgene_mcupmu_show, xgene_mcupmu_set),
        __ATTR(iobpmu, S_IWUGO | S_IRUGO,
                xgene_iobpmu_show, xgene_iobpmu_set),
        __ATTR(mcbpmu, S_IWUGO | S_IRUGO,
                xgene_mcbpmu_show, xgene_mcbpmu_set)
};

static int xgene_pmu_add_sysfs(struct device_driver *driver)
{
	int i;
	int rc;

	for (i = 0; i < ARRAY_SIZE(xgene_pmu_sysfs_attrs); i++) {
		rc = driver_create_file(driver, &xgene_pmu_sysfs_attrs[i]);
		if (rc != 0)
			goto err;
	}
	return 0;

err:
	while (--i >= 0)
		driver_remove_file(driver, &xgene_pmu_sysfs_attrs[i]);
	return rc;
}

static int xgene_pmu_remove_sysfs(struct device_driver *driver)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(xgene_pmu_sysfs_attrs); i++) {
		driver_remove_file(driver, &xgene_pmu_sysfs_attrs[i]);
	}

	return 0;
}

static int __init xgene_pmu_init(void)
{
	int rc;

	rc = platform_driver_register(&xgene_pmu_driver);
	
	if (!rc)
		rc = xgene_pmu_add_sysfs(&xgene_pmu_driver.driver);

	return rc;
}
device_initcall(xgene_pmu_init);

static void __exit xgene_pmu_exit(void)
{
	xgene_pmu_remove_sysfs(&xgene_pmu_driver.driver);
        platform_driver_unregister(&xgene_pmu_driver);
}
module_exit(xgene_pmu_exit);
