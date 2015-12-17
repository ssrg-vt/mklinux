#include <linux/cpu.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <acpi/acpi_bus.h>
#include <acpi/actypes.h>
#include <linux/acpi.h>
#include <linux/export.h>
#include <misc/xgene/pm/xgene_cpuidle.h>
#include "apm_pcp_csr.h"


#ifdef CONFIG_HOTPLUG_CPU
int arch_kill_cpu(unsigned int cpu)
{
	return 1;
}

int arch_disable_cpu(unsigned int cpu)
{
	return 0;
}

/* */
void arch_die_cpu(unsigned int cpu)
{
#ifdef CONFIG_ACPI
	if (acpi_target_system_state() == ACPI_STATE_S0) {
		/* If we are getting hotplugged out, then disable L2C pre-fetch of primary core only */
		if (cpu_online(((cpu&1) ? (cpu-1):(cpu+1)))) {
			xgene_enter_standby(0);
		}
		else {
			xgene_enter_standby(1);
		}
	}
	else {
		/* If we are going to sleep state, disable L2C pre-fetch */
		xgene_enter_standby(1);
	}
#endif
}

/* This function is called by master core, waiting for wakeup from sleep state */
void arch_wait_for_wake()
{
#ifdef CONFIG_ACPI
	xgene_enter_standby(1);
#endif
}

static int suspend_cpu_notify(struct notifier_block *nb,
		unsigned long action, void *hcpu)
{
#ifdef CONFIG_ACPI
	volatile void __iomem* crcr_reg = 0;
	int cpu = (unsigned long)hcpu;

	switch (action) {
	case CPU_UP_PREPARE:
		/* If first one of the pair, clock enable the PMD */
		if (((cpu&1) && !cpu_online(cpu-1)) || (!(cpu&1) && !cpu_online(cpu+1))) {
			xgene_clockenable_pmd(cpu/2);
		}
		/* Reset core */
		crcr_reg = ioremap_nocache((PCP_RB_BASE_ADDR+PCP_RB_CPU0_CPU_PAGE+PCP_RB_CPUX_CPU_CRCR_PAGE_OFFSET+(0x100000*cpu)), 4);
		if (!crcr_reg) {
			pr_err("ERROR: Unable to reset CPU:%d\n", cpu);
			return NOTIFY_OK;
		}
		writel(2, crcr_reg);
		iounmap(crcr_reg);
		break;
	case CPU_UP_CANCELED:
	case CPU_DOWN_FAILED:
		pr_err("ERROR: Hotplug unsuccessful:%d\n", cpu);
		break;
	}
#endif
	return NOTIFY_OK;
}

static struct notifier_block suspend_cpu_notifier = {
	.notifier_call = suspend_cpu_notify,
};

static int __init suspend_init(void)
{
	return register_cpu_notifier(&suspend_cpu_notifier);
}
core_initcall(suspend_init);
#endif
