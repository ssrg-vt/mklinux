/*
 *  Copyright (C) 2004-2005 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_ARM_CPU_H
#define __ASM_ARM_CPU_H

#include <linux/percpu.h>
#include <linux/cpu.h>
#include <linux/topology.h>

struct cpuinfo_arm {
	struct cpu	cpu;
	u64		cpuid;
#ifdef CONFIG_SMP
	unsigned int	loops_per_jiffy;
#endif
};

#ifdef CONFIG_HOTPLUG_CPU
extern int arch_register_cpu(int cpu);
extern void arch_unregister_cpu(int cpu);
#endif

DECLARE_PER_CPU(struct cpuinfo_arm, cpu_data);

#endif
