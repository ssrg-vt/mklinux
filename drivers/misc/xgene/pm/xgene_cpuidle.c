/**
 * xgene_cpuidle.c - AppliedMicro X-Gene cpuidle driver
 *
 * Copyright (c) 2013, Applied Micro Circuits Corporation
 * Author: Narinder Dhillon <ndhillon@apm.com>
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
 * This module provides interface to SlimPRO which controls power and clock.
 *
 */

#include <linux/sched.h>
#include <linux/cpu_pm.h>
#include <linux/export.h>
#include <linux/clockchips.h>
#include <linux/sysfs.h>
#include <linux/cpu.h>
#include <linux/clk.h>
#include <linux/efi.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include <asm/proc-fns.h>
#include <asm/setup.h>
#include <linux/ioport.h>
#include <misc/xgene/pm/xgene_cpuidle.h>
#include <linux/irqchip/arm-gic.h>
#include <linux/delay.h>
#include "idmap.h"
#include "apm_pcp_csr.h"
#include "apm_iob_csr.h"

#define ACPI0_CSR_I_BASE_ADDR			0x010550000ULL
#define ACPI_RO0_ADDR				0x00000010

extern void xgene_cpu_cache_mmu_off(void* func);
extern void xgene_invalidate_icache(void);
extern void xgene_invalidate_tlb(void);
extern void xgene_cpu_cache_mmu_on(void);

extern void xgene_offline_pmd(u32 cpuid, volatile void __iomem* cstate_reg, volatile void __iomem* save_area);
extern void xgene_offline_core(volatile void __iomem* save_area);

static atomic_t gic_interface_sync[4];

/*
DEFINE_PER_CPU(void __iomem*, xgene_db_csr);
DEFINE_PER_CPU(unsigned int, xgene_signal);
*/

DEFINE_PER_CPU(void __iomem*, xgene_cstate_csr);
static void __iomem* xgene_gicc_base;
static void __iomem* xgene_core0l2ccr_base;

volatile void __iomem* pxasr_reg[4];
volatile void __iomem* l2ccr_reg[4];
volatile void __iomem* cswser_reg;
volatile void __iomem* cswasr_reg;
volatile void __iomem* rbasr_reg;
volatile void __iomem* pxccr0_reg[4];

#define GIC_PROCESSOR_OFFSET		0x00020000


/**
 * xgene_enter_idle_[simple/coupled] - cpuidle entry functions
 * @dev: cpuidle device
 * @drv: cpuidle driver
 * @index: the index of state to be entered
 *
 * Called from the CPUidle framework to program the device to the
 * specified low power state selected by the governor.
 * Returns the amount of time spent in the low power state.
 */
int xgene_enter_idle_simple(struct cpuidle_device *dev,
			struct cpuidle_driver *drv,
			int index)
{
	local_fiq_disable();
	cpu_do_idle();
	local_fiq_enable();
	return index;
}

#if 0
/* Debug function to dump page tables */
static dump_pgd(struct mm_struct *mm)
{
	pgd_t *pgd = idmap_pgd;//mm->pgd;
	pmd_t *pmd;pte_t *pte;
	u32 tmp=512;
	u32 tmp2=512;
	u32 tmp3=512;
	pr_alert("PGD 0x%016llx\n", pgd);
	do {
		if (*pgd) {
			pr_alert("PGD_ENTRY[%d] = 0x%016llx\n", (512-tmp), *pgd);
			pmd = phys_to_virt((*pgd)&0xFFFFFFFFF000);
			do {
				if (*pmd) {
					pr_alert("PMD_ENTRY[%d] = 0x%016llx\n", (512-tmp2), *pmd);
					pte = phys_to_virt((*pmd)&0xFFFFFFFFF000);
					do {
						if (*pte) {
							pr_alert("PTE_ENTRY[%d] = 0x%016llx\n", (512-tmp3), *pte);
						}
						pte++;
					}while(tmp3-- !=1);
					tmp3=512;
				}
				pmd++;
			}while(tmp2-- !=1);
			tmp2=512;
		}
		pgd++;
	}while(tmp-- !=1);
}

void show_pteidle(struct mm_struct *mm, unsigned long addr)
{
	pgd_t *pgd;

	if (!mm)
		mm = &init_mm;

	pr_alert("pgd = %p\n", mm->pgd);
	pgd = pgd_offset(mm, addr);
	pr_alert("[%08lx] pgd=0x%llx *pgd=%016llx", addr, pgd, pgd_val(*pgd));

	do {
		pud_t *pud;
		pmd_t *pmd;
		pte_t *pte;

		if (pgd_none_or_clear_bad(pgd))
			break;

		pud = pud_offset(pgd, addr);
		if (pud_none_or_clear_bad(pud))
			break;

		pmd = pmd_offset(pud, addr);
		printk(", *pmd=%016llx", pmd_val(*pmd));
		if (pmd_none_or_clear_bad(pmd))
			break;

		pte = pte_offset_map(pmd, addr);
		printk(", *pte=%016llx", pte_val(*pte));
		pte_unmap(pte);
	} while(0);

	printk("\n");
}
#endif

extern char  __idmap_text_start[], __idmap_text_end[];

int xgene_clockgate_pmd(u32 pmd)
{
	int i, status;
	int rc = 0;
	/* Check if PMD quiesced */
	i=0;
	do {
		status = readl(pxasr_reg[pmd]);

	}while(((status & 0x766) != 0x766) && (i++ < 5000000));
	if (i >= 5000000) {
		pr_err("PMD:%d Not Quiesced\n", pmd);
		goto OUT;
		rc = -1;
	}

	/* Disable L2C pre-fetch */
	writel((readl(l2ccr_reg[pmd]) | PCP_RB_CPUX_L2C_L2CR_HDPDIS_MASK), l2ccr_reg[pmd]);
	mdelay(500);

	/* Disable PMD snoop */
	writel((readl(cswser_reg) & (~(1<<pmd))), cswser_reg);

    /* Wait for no outstanding snoop activity */
	do {
		status = readl(cswasr_reg);

	}while((status & (PCP_RB_CSW_CSWASR_PMD0SNPACTV_MASK << pmd)) && (i++ < 5000000));
	if (status & (PCP_RB_CSW_CSWASR_PMD0SNPACTV_MASK << pmd)) {
		pr_err("PMD:%d Snoop Active\n", pmd);
		/* Enable L2C pre-fetch */
		writel((readl(l2ccr_reg[pmd]) & (~PCP_RB_CPUX_L2C_L2CR_HDPDIS_MASK)), l2ccr_reg[pmd]);
		/* Enable PMD snoop */
		writel((readl(cswser_reg) | (1<<pmd)), cswser_reg);
		rc = -1;
		goto OUT;
	}

	/* Poll for DataQuiescedL2c */
	i=0;
	do {
		status = readl(pxasr_reg[pmd]);

	}while((!(status & PCP_RB_CSW_P0ASR_DATAQUIESCEDL2C_MASK)) && (i++<1000));

	if (!(status & PCP_RB_CSW_P0ASR_DATAQUIESCEDL2C_MASK)) {
		/* L2C not quiesced, get out */
		/* Enable L2C pre-fetch */
		writel((readl(l2ccr_reg[pmd]) & (~PCP_RB_CPUX_L2C_L2CR_HDPDIS_MASK)), l2ccr_reg[pmd]);
		/* Enable PMD snoop */
		writel((readl(cswser_reg) | (1<<pmd)), cswser_reg);
		rc = -1;
		goto OUT;
	}

	/* Indicate PMD is not available */
	writel(readl(rbasr_reg) & (~(1 << pmd)), rbasr_reg);

    /* Now clock gate */
	writel(readl(pxccr0_reg[pmd]) & (~PCP_RB_CSW_P0CCR0_PMDCLKEN_MASK), pxccr0_reg[pmd]);

OUT:
	return rc;
}

int xgene_clockenable_pmd(u32 pmd)
{
	/* Now clock enable */
	writel(readl(pxccr0_reg[pmd]) | PCP_RB_CSW_P0CCR0_PMDCLKEN_MASK, pxccr0_reg[pmd]);
	/* Indicate PMD is available */
	writel(readl(rbasr_reg) | (1 << pmd), rbasr_reg);
	/* Enable PMD snoop */
	writel((readl(cswser_reg) | (1<<pmd)), cswser_reg);
	/* Enable L2C pre-fetch */
	writel((readl(l2ccr_reg[pmd]) & (~PCP_RB_CPUX_L2C_L2CR_HDPDIS_MASK)), l2ccr_reg[pmd]);

	return 0;
}

int xgene_enter_standby(u32 disable_l2c_prefetch)
{
	struct mm_struct *mm;
	volatile void __iomem* l2ccr_reg = 0;
	void (*offline) (void __iomem* save_area);
	int cpu_id = smp_processor_id();;
	offline = (void*)virt_to_phys(xgene_offline_core);
	mm = current->active_mm;
	l2ccr_reg = ioremap_nocache((PCP_RB_BASE_ADDR+PCP_RB_CPU0_L2C_PAGE+PCP_RB_CPUX_L2C_L2CR_PAGE_OFFSET+(0x100000*cpu_id)), 4);
	if (!l2ccr_reg) {
		return -1;
	}
//printk("ENTER STBY: 0x%llx\n", mm);
	local_fiq_disable();
	clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_ENTER, &cpu_id);
	sched_clock_idle_sleep_event();
	cpu_pm_enter();
	stop_critical_timings();
	/* Disable L2C prefetch */
	if (disable_l2c_prefetch) {
		writel((readl(l2ccr_reg) | PCP_RB_CPUX_L2C_L2CR_HDPDIS_MASK), l2ccr_reg);
		mdelay(500);
	}
	setup_mm_for_powerdn(mm);
	offline((void*)virt_to_phys(((void *)__idmap_text_start)+(u64)(256*cpu_id)));
	//printk("REV 0x%llx\n", mm);
	revert_mm_after_wakeup(mm);
	/* Enable L2C prefetch */
	//writel((readl(xgene_core0l2ccr_base) & (~PCP_RB_CPUX_L2C_L2CR_HDPDIS_MASK)),
	//		xgene_core0l2ccr_base);
	writel((readl(l2ccr_reg) & (~PCP_RB_CPUX_L2C_L2CR_HDPDIS_MASK)), l2ccr_reg);
	start_critical_timings();
	cpu_pm_exit();
	sched_clock_idle_wakeup_event(0);
	clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_EXIT, &cpu_id);
	local_fiq_enable();
	iounmap(l2ccr_reg);
	//printk("EXIT STBY\n");
	return 0;
}

int xgene_enter_idle_coupled(struct cpuidle_device *dev,
			struct cpuidle_driver *drv,
			int index)
{
	int state_entered = -1;
	unsigned int val;
	volatile int tmp;
	struct mm_struct *mm;
	void (*offline) (u32 id,  void __iomem* cstate_reg, void __iomem* save_area);
	int cpu_id = dev->cpu;
	void __iomem* cstatereg = (per_cpu(xgene_cstate_csr, cpu_id));
	offline = (void*)virt_to_phys(xgene_offline_pmd);
	mm = current->active_mm;

	local_fiq_disable();
	tmp = readl(cstatereg + ((index+1)*4));

	cpu_do_idle();
	val = readl_relaxed(xgene_gicc_base);
	if ((val & 0x3FF) == 0xF) {
		atomic_inc(&gic_interface_sync[(cpu_id/2)]);
		tmp = 0;
		do {
			if (atomic_read(&gic_interface_sync[(cpu_id/2)]) == 2) break;
		}while(tmp++<50000);
		if (atomic_read(&gic_interface_sync[(cpu_id/2)]) == 2) {
			clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_ENTER, &cpu_id);
			sched_clock_idle_sleep_event();
			cpu_pm_enter();
			stop_critical_timings();
			/* Disable GIC interrupts */
			gic_cpu_if_down();

			setup_mm_for_powerdn(mm);
			offline(cpu_id,
				(void*)(long long)(0x10550000 + (cpu_id*0x1000) + 0x10 + ((index+1)*4)),
				(void*)virt_to_phys(((void *)__idmap_text_start)+(u64)(256*cpu_id)));
			revert_mm_after_wakeup(mm);
			state_entered = index;
			gic_enable_routing();
			start_critical_timings();
			cpu_pm_exit();
			sched_clock_idle_wakeup_event(0);
			clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_EXIT, &cpu_id);
		}
		else {
			tmp = readl(cstatereg);
			atomic_set(&gic_interface_sync[(cpu_id/2)], 0);
			//printk("%d: BAIL\n", cpu_id);
		}
	}
	else {
		tmp = readl(cstatereg);
		//printk("%d:TIMER!\n", cpu_id);
	}

	if (cpuidle_coupled_wakeup(dev) == 1) {
		int cpu;
		const struct cpumask *coupled_cpus;
		coupled_cpus = cpuidle_coupled_get_cpumask(dev, index);
		for_each_cpu_mask(cpu, *coupled_cpus) {
			if (cpu != dev->cpu && cpu_online(cpu)) {
				arch_send_wakeup_ipi_mask(cpumask_of(cpu));
			}
		}
	}

	cpuidle_coupled_wakeup_barrier(dev);
	local_fiq_enable();
	atomic_set(&gic_interface_sync[(cpu_id/2)], 0);
	return state_entered;
}

/*
 * For each cpu, setup the broadcast timer because local timers
 * stops for the states above C1.
 */
static void xgene_setup_broadcast_timer(void *arg)
{
	int cpu = smp_processor_id();
	clockevents_notify(CLOCK_EVT_NOTIFY_BROADCAST_ON, &cpu);
}

int __init xgene_idle_init(void)
{
	void __iomem** cstate_csr;
	unsigned int cpu_id = 0;
	unsigned int pmd;

	for(pmd=0; pmd<4; pmd++) {
		atomic_set(&gic_interface_sync[pmd], 0);
	}
	/* Configure the broadcast timer on each cpu */
	on_each_cpu(xgene_setup_broadcast_timer, NULL, 1);

	if (request_mem_region(/*GIC400_CSR_PMD_BASE_ADDR+GICC_HPPIR_ADDR*/(0x78020018), 4, "GIC_HPPIR")) {
		xgene_gicc_base = ioremap_nocache(/*GIC400_CSR_PMD_BASE_ADDR+GICC_HPPIR_ADDR*/(0x78020018), 4);
		printk("GIC_HPPIR 0x%llx ", (u64)xgene_gicc_base);
	}

	for_each_cpu(cpu_id, cpu_online_mask) {
		cstate_csr = &per_cpu(xgene_cstate_csr, cpu_id);
		if (request_mem_region(ACPI0_CSR_I_BASE_ADDR + ACPI_RO0_ADDR + (cpu_id*0x1000), 20, "CxCNT")) {
			*cstate_csr = ioremap_nocache(ACPI0_CSR_I_BASE_ADDR + ACPI_RO0_ADDR + (cpu_id*0x1000), 20);
			printk("%d:CxCNT 0x%llx ", cpu_id, (u64)*cstate_csr);
		}
	}

	if (request_mem_region((PCP_RB_BASE_ADDR+PCP_RB_CPU0_L2C_PAGE+PCP_RB_CPUX_L2C_L2CR_PAGE_OFFSET), 4, "L2CCR_CORE0")) {
		xgene_core0l2ccr_base = ioremap_nocache(/*GIC400_CSR_PMD_BASE_ADDR+GICC_HPPIR_ADDR*/(0x78020018), 4);
	}

	for(pmd=0; pmd<4; pmd++) {
		pxasr_reg[pmd] =
		ioremap_nocache((PCP_RB_BASE_ADDR+PCP_RB_CSW_PAGE+PCP_RB_CSW_P0ASR_PAGE_OFFSET+(pmd<<4)), 4);
		l2ccr_reg[pmd]  =
		ioremap_nocache((PCP_RB_BASE_ADDR+PCP_RB_CPU0_L2C_PAGE+PCP_RB_CPUX_L2C_L2CR_PAGE_OFFSET+(0x100000*pmd)), 4);
		pxccr0_reg[pmd]  =
		ioremap_nocache((PCP_RB_BASE_ADDR+PCP_RB_CSW_PAGE+PCP_RB_CSW_P0CCR0_PAGE_OFFSET+(pmd<<4)), 4);
		if (!pxasr_reg[pmd] || !l2ccr_reg[pmd] || !pxccr0_reg[pmd]) {
			pr_err("ERROR: PMD:%d Register Mapping\n", pmd);
		}
	}
	cswser_reg =
			ioremap_nocache((PCP_RB_BASE_ADDR+PCP_RB_CSW_PAGE+PCP_RB_CSW_CSWSER_PAGE_OFFSET), 4);
	cswasr_reg =
			ioremap_nocache((PCP_RB_BASE_ADDR+PCP_RB_CSW_PAGE+PCP_RB_CSW_CSWASR_PAGE_OFFSET), 4);
	rbasr_reg =
			ioremap_nocache((IOB_RBM_REGS_BASE_ADDR+RBASR_ADDR), 4);

	if (!cswser_reg || !cswasr_reg || !rbasr_reg) {
		pr_err("ERROR: PCP Register Mapping\n");
	}

	return 0;
}

core_initcall(xgene_idle_init);

#define XGENE_CPUIDLE_STATE_MAX 4

struct cpuidle_state_usage xgene_cpuidle_state_usage[XGENE_CPUIDLE_STATE_MAX] =
{
	{ .disable = false, }, /* C1 */
	{ .disable = true,  }, /* C3 */
	{ .disable = true,  }, /* C4 */
	{ .disable = true,  }, /* C6 */
};

static struct cpuidle_driver xgene_cpuidle_driver = {
	.name = "xgene_cpuidle",
	.owner = THIS_MODULE,
	.states = {
		{
		.enter		  = xgene_enter_idle_simple,
		.exit_latency	  = 1,
		.power_usage	  = 2500,
		.target_residency = 2 * 1,
		.flags		  = CPUIDLE_FLAG_TIME_VALID,
		.name		  = "C1",
		.desc		  = "WFI",
		},
		{
		.enter		  = xgene_enter_idle_coupled,
		.exit_latency	  = 20000,
		.power_usage	  = 1800,
		.target_residency = 2 * 20000,
		.flags		  = CPUIDLE_FLAG_TIME_VALID |
				    CPUIDLE_FLAG_COUPLED,
		.name		  = "C3",
		.desc		  = "Doze",
		},
		{
		.enter		  = xgene_enter_idle_coupled,
		.exit_latency	  = 50000,
		.power_usage	  = 1000,
		.target_residency = 2 * 50000,
		.flags		  = CPUIDLE_FLAG_TIME_VALID |
				    CPUIDLE_FLAG_COUPLED,
		.name		  = "C4",
		.desc		  = "Sleep",
		},
		{
		.enter		  = xgene_enter_idle_coupled,
		.exit_latency	  = 10000000,
		.power_usage	  = 0,
		.target_residency = 2 * 10000000,
		.flags		  = CPUIDLE_FLAG_TIME_VALID |
				    CPUIDLE_FLAG_COUPLED,
		.name		  = "C6",
		.desc		  = "PowerOff",
		},
	},
	.safe_state_index = 0,
	.state_count = XGENE_CPUIDLE_STATE_MAX,
};

DEFINE_PER_CPU(struct cpuidle_device, xgene_cpuidle_dev);

void xgene_cpuidle_update_states_desc(struct cpuidle_driver *drv)
{
	int i;

	if (drv) {
		for (i = 0; i < XGENE_CPUIDLE_STATE_MAX; i++)
			strncpy(drv->states[i].desc,
				xgene_cpuidle_driver.states[i].desc,
				CPUIDLE_DESC_LEN);
	}
}

void xgene_cpuidle_update_states_status(struct cpuidle_device *dev)
{
	struct cpuidle_state_usage *usage = xgene_cpuidle_state_usage;
	int i;

	if (dev) {
		for (i= 0; i < XGENE_CPUIDLE_STATE_MAX; i++)
			dev->states_usage[i].disable = usage[i].disable;
	}
}

static int __init xgene_cpuidle_init(void)
{
	struct cpuidle_driver *drv;
        struct cpuidle_device *dev;
	cpumask_t coupled_cpus;
	cpumask_t *cpu_pmd_mask;
        int cpu;
	int ret = -EBUSY;

	/*
	 * If booting UEFI/ACPI then ACPI took care of
	 * registering with cpuidle framework.
	 */

	drv = cpuidle_get_driver();
	if (!drv) {
		ret = cpuidle_register_driver(&xgene_cpuidle_driver);
		drv = cpuidle_get_driver();

		for_each_possible_cpu(cpu) {
			dev = &per_cpu(xgene_cpuidle_dev, cpu);
			dev->cpu = cpu;

			/* set PMD coupled CPUs mask */
			cpu_pmd_mask = &coupled_cpus;
			cpumask_clear(&coupled_cpus);
			cpumask_set_cpu(cpu, &coupled_cpus);
			if ((cpu%2) == 0)
				cpumask_set_cpu(cpu+1, &coupled_cpus);
			else
				cpumask_set_cpu(cpu-1, &coupled_cpus);

			/* set coupled CPUs mask required per state */
			cpuidle_coupled_set_cpumask(dev, 1, cpu_pmd_mask);
			cpuidle_coupled_set_cpumask(dev, 2, cpu_possible_mask);
			cpuidle_coupled_set_cpumask(dev, 3, cpu_possible_mask);

			pr_debug("cpuidle: cpu_pmd_mask(0x%08lx)\n",
				*(cpumask_bits(cpu_pmd_mask)));
			pr_debug("cpuidle: cpu_possible_mask(0x%08lx)\n",
				*(cpumask_bits(cpu_possible_mask)));

			ret = cpuidle_register_device(dev);
			if (ret) {
				pr_info("cpuidle: failed to register cpu%d\n",
					cpu);
				cpuidle_unregister_driver(drv);
				return ret;
			}

			xgene_cpuidle_update_states_status(dev);
		}
	}

	if (drv)
		pr_info("cpuidle: using %s\n", drv->name);

        return ret;
}
late_initcall(xgene_cpuidle_init);

static int __init xgene_cpufreq_init(void)
{
	struct clk *pll_clk;
	unsigned long pll_freq;
	int pmd = smp_processor_id() / 2;
	u32 pmd_div;
	u32 pmd_mult;
	#define PMD_DIV_RATIO_RD(v)	(((v) & 0x3000) >> 12)

	/* ACPI/EFI will report via ACPI state table */
	if (efi_enabled(EFI_BOOT))
		return 0;

	if (pmd > 4)
		return 0;

	/* Get the PCP PLL freqency */
	pll_clk = clk_get(NULL, "pcppll");
	if (IS_ERR(pll_clk)) {
		pr_err("No PCP/PMD PLL clock\n");
		return 0;
	}
	pll_freq = clk_get_rate(pll_clk);
	clk_put(pll_clk);

	pmd_div = PMD_DIV_RATIO_RD(readl(pxccr0_reg[pmd])) + 1;
	pmd_mult = 1;
	pr_debug("CPU%d frequency %ld\n", smp_processor_id(),
		pll_freq * pmd_mult / pmd_div);
	cpu_freq = pll_freq * pmd_mult / pmd_div;
	return 0;
}
late_initcall(xgene_cpufreq_init);
