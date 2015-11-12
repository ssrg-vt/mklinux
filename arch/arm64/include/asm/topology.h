#ifndef _ASM_ARM64_TOPOLOGY_H
#define _ASM_ARM64_TOPOLOGY_H

#ifdef CONFIG_ARM64_CPU_TOPOLOGY

#include <linux/cpumask.h>

struct cputopo_arm64 {
	int thread_id;
	int core_id;
	int socket_id;
	cpumask_t thread_sibling;
	cpumask_t core_sibling;
};

DECLARE_PER_CPU(struct cputopo_arm64, cpu_topology);

#define cpu_topo(cpu) per_cpu(cpu_topology, cpu)

#define topology_physical_package_id(cpu)	(cpu_topo(cpu).socket_id)
#define topology_core_id(cpu)		(cpu_topo(cpu).core_id)
#define topology_core_cpumask(cpu)	(&cpu_topo(cpu).core_sibling)
#define topology_thread_cpumask(cpu)	(&cpu_topo(cpu).thread_sibling)

#define mc_capable()	(cpu_topo(0).socket_id != -1)
#define smt_capable()	(cpu_topo(0).thread_id != -1)

void init_cpu_topology(void);
void store_cpu_topology(unsigned int cpuid);
const struct cpumask *cpu_coregroup_mask(int cpu);
void arch_fix_phys_package_id(int num, u32 slot);

#else

static inline void arch_fix_phys_package_id(int num, u32 slot) {}
static inline void init_cpu_topology(void) { }
static inline void store_cpu_topology(unsigned int cpuid) { }

#endif

#include <asm-generic/topology.h>

#endif /* _ASM_ARM64_TOPOLOGY_H */
