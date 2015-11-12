/*
 *  Copyright (C) 2013, Al Stone <al.stone@linaro.org>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#ifndef _ASM_ARM64_ACPI_H
#define _ASM_ARM64_ACPI_H

#include <asm/cacheflush.h>

#include <linux/init.h>

#define COMPILER_DEPENDENT_INT64	s64
#define COMPILER_DEPENDENT_UINT64	u64

#define MAX_LOCAL_APIC 256
#define MAX_IO_APICS 64

/*
 * Calling conventions:
 *
 * ACPI_SYSTEM_XFACE        - Interfaces to host OS (handlers, threads)
 * ACPI_EXTERNAL_XFACE      - External ACPI interfaces
 * ACPI_INTERNAL_XFACE      - Internal ACPI interfaces
 * ACPI_INTERNAL_VAR_XFACE  - Internal variable-parameter list interfaces
 */
#define ACPI_SYSTEM_XFACE
#define ACPI_EXTERNAL_XFACE
#define ACPI_INTERNAL_XFACE
#define ACPI_INTERNAL_VAR_XFACE

/* Asm macros */
#define ACPI_FLUSH_CPU_CACHE() flush_cache_all()

/* Basic configuration for ACPI */
#ifdef	CONFIG_ACPI
extern int acpi_disabled;
extern int acpi_noirq;
extern int acpi_pci_disabled;
extern int acpi_strict;

static inline void disable_acpi(void)
{
	acpi_disabled = 1;
	acpi_pci_disabled = 1;
	acpi_noirq = 1;
}

static inline bool arch_has_acpi_pdc(void)
{
	return false;	/* always false for now */
}

static inline void arch_acpi_set_pdc_bits(u32 *buf)
{
	return;
}

static inline void acpi_noirq_set(void) { acpi_noirq = 1; }
static inline void acpi_disable_pci(void)
{
	acpi_pci_disabled = 1;
	acpi_noirq_set();
}

/* Low-level suspend routine. */
extern int (*acpi_suspend_lowlevel)(void);
#define acpi_wakeup_address (0)

/* map logic cpu id to physical GIC id */
extern int arm_cpu_to_apicid[NR_CPUS];
#define cpu_physical_id(cpu) arm_cpu_to_apicid[cpu]

extern int cpu_acpi_read_ops(int cpu);

extern int boot_cpu_apic_id;
extern const char *acpi_get_enable_method(int cpu);
extern int acpi_get_cpu_release_address(int cpu, u64 *release_address);

struct acpi_arm_root {
	phys_addr_t phys_address;
	unsigned long size;
};
extern struct acpi_arm_root acpi_arm_rsdp_info;
void arm_acpi_reserve_memory(void);
extern void prefill_possible_map(void);

#define MAX_GIC_CPU_INTERFACE	256
#define MAX_GIC_DISTRIBUTOR	1

#else	/* !CONFIG_ACPI */
#define acpi_disabled 1		/* ACPI sometimes enabled on ARM */
#define acpi_noirq 1		/* ACPI sometimes enabled on ARM */
#define acpi_pci_disabled 1	/* ACPI PCI sometimes enabled on ARM */
#define acpi_strict 1		/* no ACPI spec workarounds on ARM */

static inline int cpu_acpi_read_ops(int cpu)
{
	return -ENODEV;
}

static inline const char *acpi_get_enable_method(int cpu)
{
	return NULL;
}

static inline int acpi_get_cpu_release_address(int cpu, u64 *release_address)
{
	return -ENODEV;
}

static inline void disable_acpi(void) {}
#endif

#endif /*_ASM_ARM64_ACPI_H*/
