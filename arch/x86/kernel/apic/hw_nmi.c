/*
 *  HW NMI watchdog support
 *
 *  started by Don Zickus, Copyright (C) 2010 Red Hat, Inc.
 *
 *  Arch specific calls to support NMI watchdog
 *
 *  Bits copied from original nmi.c file
 *
 */
#include <asm/apic.h>

#include <linux/cpumask.h>
#include <linux/kdebug.h>
#include <linux/notifier.h>
#include <linux/kprobes.h>
#include <linux/nmi.h>
#include <linux/module.h>
#include <linux/delay.h>

#ifdef CONFIG_HARDLOCKUP_DETECTOR
u64 hw_nmi_get_sample_period(int watchdog_thresh)
{
	return (u64)(cpu_khz) * 1000 * watchdog_thresh;
}
#endif

#ifdef arch_trigger_all_cpu_backtrace
/* For reliability, we're prepared to waste bits here. */
static DECLARE_BITMAP(backtrace_mask, NR_CPUS) __read_mostly;
static DECLARE_BITMAP(backtrace_mask_hints, NR_CPUS) __read_mostly;

/* "in progress" flag of arch_trigger_all_cpu_backtrace */
static unsigned long backtrace_flag;

void arch_trigger_all_cpu_backtrace(void)
{
	int i;
	char buffer[128];

	if (test_and_set_bit(0, &backtrace_flag))
		/*
		 * If there is already a trigger_all_cpu_backtrace() in progress
		 * (backtrace_flag == 1), don't output double cpu dump infos.
		 */
		return;

//if (cpumask_empty(to_cpumask(backtrace_mask_hints)))
	cpumask_copy(to_cpumask(backtrace_mask), cpu_online_mask);
//else
	//cpumask_copy(to_cpumask(backtrace_mask), to_cpumask(backtrace_mask_hints));

	printk(KERN_ERR "sending NMI to all CPUs:\n");
	apic->send_IPI_all(BACKTRACE_VECTOR);

	/* Wait for up to 10 seconds for all CPUs to do the backtrace */
	for (i = 0; i < 10 * 1000; i++) {
		if (cpumask_empty(to_cpumask(backtrace_mask)))
			break;
		mdelay(1);
	}

cpumask_scnprintf(buffer, 128, to_cpumask(backtrace_mask));
printk("%s: mask %s\n", __func__, buffer);
cpumask_clear(to_cpumask(backtrace_mask_hints));
	clear_bit(0, &backtrace_flag);
	smp_mb__after_clear_bit();
}

static int __kprobes
arch_trigger_all_cpu_backtrace_handler(unsigned int cmd, struct pt_regs *regs)
{
	int cpu;

	cpu = smp_processor_id();
	if (cpumask_test_cpu(cpu, to_cpumask(backtrace_mask))
&& cpumask_test_cpu(cpu, to_cpumask(backtrace_mask_hints))
) {
		static arch_spinlock_t lock = __ARCH_SPIN_LOCK_UNLOCKED;
printk("%s: ehilaaa %d\n", __func__, cpu);
		arch_spin_lock(&lock);
		printk(KERN_ERR "NMI backtrace for cpu %d\n", cpu);
		show_regs(regs);
		arch_spin_unlock(&lock);
		cpumask_clear_cpu(cpu, to_cpumask(backtrace_mask));
		return NMI_HANDLED;
	}
cpumask_clear_cpu(cpu, to_cpumask(backtrace_mask));
	return NMI_DONE;
}

void smp_backtrace_interrupt(struct pt_regs * regs)
{
  arch_trigger_all_cpu_backtrace_handler(0, regs);
}

void smp_backtrace_interrupt_hints(struct cpumask *src)
{ 
  cpumask_copy(to_cpumask(backtrace_mask_hints), src);
}

static int __init register_trigger_all_cpu_backtrace(void)
{
printk("%s: interrupt %d\n", __func__, NMI_LOCAL);
	register_nmi_handler(NMI_LOCAL, arch_trigger_all_cpu_backtrace_handler,
				0, "arch_bt");
        cpumask_clear(to_cpumask(backtrace_mask_hints));
	return 0;
}
early_initcall(register_trigger_all_cpu_backtrace);
#endif
