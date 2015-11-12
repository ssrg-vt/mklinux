/*
 * arch/arm64/kernel/topology.c
 *
 * Copyright (C) 2013 Linaro Limited.
 * Written by: Hanjun Guo
 *
 * based on arch/arm/kernel/topology.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/export.h>
#include <linux/init.h>
#include <linux/percpu.h>
#include <linux/node.h>
#include <linux/nodemask.h>
#include <linux/of.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <asm/cputype.h>
#include <asm/topology.h>
#include <asm/cpu.h>

DEFINE_PER_CPU(struct cputopo_arm64, cpu_topology);

const struct cpumask *cpu_coregroup_mask(int cpu)
{
	return &cpu_topo(cpu).core_sibling;
}

void update_siblings_masks(unsigned int cpuid)
{
	struct cputopo_arm64 *topo, *cpuid_topo = &cpu_topo(cpuid);
	int cpu;

	/* update core and thread sibling masks */
	for_each_possible_cpu(cpu) {
		topo = &cpu_topo(cpu);

		if (cpuid_topo->socket_id != topo->socket_id)
			continue;

		cpumask_set_cpu(cpuid, &topo->core_sibling);
		if (cpu != cpuid)
			cpumask_set_cpu(cpu, &cpuid_topo->core_sibling);

		if (cpuid_topo->core_id != topo->core_id)
			continue;

		cpumask_set_cpu(cpuid, &topo->thread_sibling);
		if (cpu != cpuid)
			cpumask_set_cpu(cpu, &cpuid_topo->thread_sibling);
	}
	smp_wmb();
}

/*
 * store_cpu_topology is called at boot when only one cpu is running
 * and with the mutex cpu_hotplug.lock locked, when several cpus have booted,
 * which prevents simultaneous write access to cpu_topology array
 */
void store_cpu_topology(unsigned int cpuid)
{
	struct cputopo_arm64 *cpuid_topo = &cpu_topo(cpuid);
	u64 mpidr;

	/* If the cpu topology has been already set, just return */
	if (cpuid_topo->core_id != -1)
		return;

	mpidr = read_cpuid_mpidr();

	/* create cpu topology mapping */
	if (!(mpidr & MPIDR_SMP_BITMASK)) {
		/*
		 * This is a multiprocessor system
		 * multiprocessor format & multiprocessor mode field are set
		 */

		if (mpidr & MPIDR_MT_BITMASK) {
			/* core performance interdependency */
			cpuid_topo->thread_id = MPIDR_AFFINITY_LEVEL_0(mpidr);
			cpuid_topo->core_id = MPIDR_AFFINITY_LEVEL_1(mpidr);
			cpuid_topo->socket_id = MPIDR_AFFINITY_LEVEL_2(mpidr);
		} else {
			/* largely independent cores */
			cpuid_topo->thread_id = -1;
			cpuid_topo->core_id = MPIDR_AFFINITY_LEVEL_0(mpidr);
			cpuid_topo->socket_id = MPIDR_AFFINITY_LEVEL_1(mpidr);
		}
	} else {
		/*
		 * This is an uniprocessor system
		 * we are in multiprocessor format but uniprocessor system
		 * or in the old uniprocessor format
		 */
		cpuid_topo->thread_id = -1;
		cpuid_topo->core_id = 0;
		cpuid_topo->socket_id = -1;
	}

	update_siblings_masks(cpuid);

	pr_info("CPU%u: thread %d, cpu %d, socket %d, mpidr 0x%llx\n",
		cpuid, cpu_topo(cpuid).thread_id,
		cpu_topo(cpuid).core_id,
		cpu_topo(cpuid).socket_id, mpidr);
}

/*
 * init_cpu_topology is called at boot when only one cpu is running
 * which prevent simultaneous write access to cpu_topology array
 */
void __init init_cpu_topology(void)
{
	unsigned int cpu;

	/* init core mask */
	for_each_possible_cpu(cpu) {
		struct cputopo_arm64 *topo = &cpu_topo(cpu);

		topo->thread_id = -1;
		topo->core_id =  -1;
		topo->socket_id = -1;
		cpumask_clear(&topo->core_sibling);
		cpumask_clear(&topo->thread_sibling);
	}
	smp_wmb();
}

void arch_fix_phys_package_id(int num, u32 slot)
{
}
EXPORT_SYMBOL_GPL(arch_fix_phys_package_id);

#ifdef CONFIG_HOTPLUG_CPU
int __ref arch_register_cpu(int cpu)
{
	struct cpuinfo_arm *cpuinfo = &per_cpu(cpu_data, cpu);

	/* BSP cann't be taken down on arm */
	if (cpu)
		cpuinfo->cpu.hotpluggable = 1;

	return register_cpu(&cpuinfo->cpu, cpu);
}
EXPORT_SYMBOL(arch_register_cpu);

void arch_unregister_cpu(int cpu)
{
	unregister_cpu(&per_cpu(cpu_data, cpu).cpu);
}
EXPORT_SYMBOL(arch_unregister_cpu);
#else /* CONFIG_HOTPLUG_CPU */

static int __init arch_register_cpu(int cpu)
{
	return register_cpu(&per_cpu(cpu_data, cpu).cpu, cpu);
}
#endif /* CONFIG_HOTPLUG_CPU */

