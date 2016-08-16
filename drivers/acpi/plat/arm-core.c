/*
 *  ARM/ARM64 Specific Low-Level ACPI Boot Support
 *
 *  Copyright (C) 2001, 2002 Paul Diefenbaugh <paul.s.diefenbaugh@intel.com>
 *  Copyright (C) 2001 Jun Nakajima <jun.nakajima@intel.com>
 *  Copyright (C) 2013, Al Stone <al.stone@linaro.org> (ARM version)
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/acpi_pmtmr.h>
#include <linux/efi.h>
#include <linux/cpumask.h>
#include <linux/memblock.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/slab.h>
#include <linux/bootmem.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/irqchip/arm-gic.h>

#include <asm/pgtable.h>
#include <asm/io.h>
#include <asm/smp.h>
#include <asm/acpi.h>
#include <asm/cputype.h>

/*
 * We never plan to use RSDT on arm/arm64 as its deprecated in spec but this
 * variable is still required by the ACPI core
 */
u32 acpi_rsdt_forced;

int acpi_disabled;
EXPORT_SYMBOL(acpi_disabled);

/*
 * Local interrupt controller address,
 * GIC cpu interface base address on ARM/ARM64
 */
static u64 acpi_lapic_addr __initdata;

/* available_cpus here means enabled cpu in MADT */
int available_cpus;

/* Map logic cpu id to physical APIC id.
 * APIC = GIC cpu interface on ARM
 */
int arm_cpu_to_apicid[NR_CPUS] = { [0 ... NR_CPUS-1] = -1 };
int boot_cpu_apic_id = -1;

/* Parked Address in ACPI GIC structure */
static u64 parked_address[NR_CPUS];

#define BAD_MADT_ENTRY(entry, end) (					    \
		(!entry) || (unsigned long)entry + sizeof(*entry) > end ||  \
		((struct acpi_subtable_header *)entry)->length < sizeof(*entry))

#define PREFIX			"ACPI: "

int acpi_noirq;				/* skip ACPI IRQ initialization */
int acpi_pci_disabled;		/* skip ACPI PCI scan and IRQ initialization */
EXPORT_SYMBOL(acpi_pci_disabled);

int acpi_strict;

/* GIC cpu interface base address on ARM */
static u64 acpi_lapic_addr __initdata;

struct acpi_arm_root acpi_arm_rsdp_info;     /* info about RSDP from FDT */

/*
 * This function pointer is needed to be defined but for now will be NULL
 * on arm where sleep is handled differently than x86
 */
int (*acpi_suspend_lowlevel)(void);

/*
 * Boot-time Configuration
 */

/*
 * The default interrupt routing model is PIC (8259).  This gets
 * overridden if IOAPICs are enumerated (below).
 *
 * Since we're on ARM, it clearly has to be GIC.
 */
enum acpi_irq_model_id acpi_irq_model = ACPI_IRQ_MODEL_GIC;

static unsigned int gsi_to_irq(unsigned int gsi)
{
	int irq = irq_find_mapping(NULL, gsi);

	return irq;
}

char *__init __acpi_map_table(phys_addr_t phys, unsigned long size)
{
	if (!phys || !size)
		return NULL;

	return early_memremap(phys, size);
}

void __init __acpi_unmap_table(char *map, unsigned long size)
{
	if (!map || !size)
		return;

	early_iounmap(map, size);
	return;
}

static int __init acpi_parse_madt(struct acpi_table_header *table)
{
	struct acpi_table_madt *madt = NULL;

	madt = (struct acpi_table_madt *)table;
	if (!madt) {
		pr_warn(PREFIX "Unable to map MADT\n");
		return -ENODEV;
	}

	if (madt->address) {
		acpi_lapic_addr = (u64) madt->address;

		pr_info(PREFIX "Local APIC address 0x%08x\n", madt->address);
	}

	return 0;
}

/**
 * acpi_register_gic_cpu_interface - register a gic cpu interface and
 * generates a logic cpu number
 * @id: gic cpu interface id to register
 * @enabled: this cpu is enabled or not
 *
 * Returns the logic cpu number which maps to the gic cpu interface
 */
static int acpi_register_gic_cpu_interface(int id, u8 enabled)
{
	int cpu;

	if (id >= MAX_GIC_CPU_INTERFACE) {
		pr_info(PREFIX "skipped apicid that is too big\n");
		return -EINVAL;
	}

	total_cpus++;
	if (!enabled)
		return -EINVAL;

	if (available_cpus >=  NR_CPUS) {
		pr_warn(PREFIX "NR_CPUS limit of %d reached,"
		" Processor %d/0x%x ignored.\n", NR_CPUS, total_cpus, id);
		return -EINVAL;
	}

	available_cpus++;

	/* allocate a logic cpu id for the new comer */
	if (boot_cpu_apic_id == id) {
		/*
		 * boot_cpu_init() already hold bit 0 in cpu_present_mask
		 * for BSP, no need to allocte again.
		 */
		cpu = 0;
	} else {
		cpu = cpumask_next_zero(-1, cpu_present_mask);
	}

	/* map the logic cpu id to APIC id */
	arm_cpu_to_apicid[cpu] = id;

	set_cpu_present(cpu, true);
	set_cpu_possible(cpu, true);

	return cpu;
}

static int __init
acpi_parse_gic(struct acpi_subtable_header *header, const unsigned long end)
{
	struct acpi_madt_generic_interrupt *processor = NULL;
	int cpu;
	
	processor = (struct acpi_madt_generic_interrupt *)header;

	if (BAD_MADT_ENTRY(processor, end))
		return -EINVAL;

	acpi_table_print_madt_entry(header);

 	/*
	 * As ACPI 5.0 said, the 64-bit physical address in GIC struct at
	 * which the processor can access this GIC. If provided, the
	 * “Local Interruptp controller Address” field in  the MADT
	 * is ignored by OSPM.
	 */
	if (processor->base_address)
		acpi_lapic_addr = processor->base_address;

	/*
	 * We need to register disabled CPU as well to permit
	 * counting disabled CPUs. This allows us to size
	 * cpus_possible_map more accurately, to permit
	 * to not preallocating memory for all NR_CPUS
	 * when we use CPU hotplug.
	 */
	cpu = acpi_register_gic_cpu_interface(processor->gic_id,
			    processor->flags & ACPI_MADT_ENABLED);

	/*
	 * We need the parked address for SMP initialization with
	 * spin-table enable method
	 */
	if (cpu >= 0 && processor->parked_address)
		parked_address[cpu] = processor->parked_address;

	return 0;
}

#ifdef CONFIG_ARM_GIC
/*
 * Hard code here, we can not get memory size from MADT (but FDT does),
 * this size is described in ARMv8 foudation model's User Guide
 */
#define GIC_DISTRIBUTOR_MEMORY_SIZE (SZ_8K)
#define GIC_CPU_INTERFACE_MEMORY_SIZE (SZ_4K)

/* GIC distributor is something like IOAPIC on x86 */
static int __init
acpi_parse_gic_distributor(struct acpi_subtable_header *header,
				const unsigned long end)
{
	struct acpi_madt_generic_distributor *distributor = NULL;
	void __iomem *dist_base = NULL;
	void __iomem *cpu_base = NULL;

	if (!IS_ENABLED(CONFIG_ARM_GIC))
		return -ENODEV;

	distributor = (struct acpi_madt_generic_distributor *)header;

	if (BAD_MADT_ENTRY(distributor, end))
		return -EINVAL;

	acpi_table_print_madt_entry(header);

	/* GIC is initialised after page_init(), no need for early_ioremap */
	dist_base = ioremap(distributor->base_address,
				GIC_CPU_INTERFACE_MEMORY_SIZE);
	if (!dist_base) {
		pr_warn("unable to map gic dist registers\n");
		return -ENOMEM;
	}

	/* acpi_lapic_addr is stored in early_acpi_boot_init(),
	 * we can use it here with no worries
	 */
	cpu_base = ioremap(acpi_lapic_addr, GIC_CPU_INTERFACE_MEMORY_SIZE);
	if (!cpu_base) {
		iounmap(dist_base);
		pr_warn("unable to map gic cpu registers\n");
		return -ENOMEM;
	}

	gic_init(distributor->gic_id, -1, dist_base, cpu_base);

	return 0;
}
#else
static int __init
acpi_parse_gic_distributor(struct acpi_subtable_header *header,
				const unsigned long end)
{
	return -ENODEV;
}
#endif /* CONFIG_ARM_GIC */

/*
 * Parse GIC cpu interface related entries in MADT
 * returns 0 on success, < 0 on error
*/
static int __init acpi_parse_madt_gic_entries(void)
{
	int count;

	/*
	 * do a partial walk of MADT to determine how many CPUs
	 * we have including disabled CPUs
	 */
	count = acpi_table_parse_madt(ACPI_MADT_TYPE_GENERIC_INTERRUPT,
				acpi_parse_gic, MAX_GIC_CPU_INTERFACE);

	if (!count) {
		pr_err(PREFIX "No GIC entries present\n");
		return -ENODEV;
	} else if (count < 0) {
		pr_err(PREFIX "Error parsing GIC entry\n");
		return count;
	}

#ifdef CONFIG_SMP
	if (available_cpus == 0) {
		pr_info(PREFIX "Found 0 CPUs; assuming 1\n");
		arm_cpu_to_apicid[available_cpus] =
			read_cpuid_mpidr() & MPIDR_HWID_BITMASK;
		available_cpus = 1;	/* We've got at least one of these */
	}
#endif

	/* Make boot-up look pretty */
	pr_info("%d CPUs available, %d CPUs total\n", available_cpus,
		total_cpus);

	return 0;
}

/*
 * Parse GIC distributor related entries in MADT
 * returns 0 on success, < 0 on error
 */
static int __init acpi_parse_madt_gic_distributor_entries(void)
{
	int count;

	count = acpi_table_parse_madt(ACPI_MADT_TYPE_GENERIC_DISTRIBUTOR,
			acpi_parse_gic_distributor, MAX_GIC_DISTRIBUTOR);

	if (!count) {
		pr_err(PREFIX "No GIC distributor entries present\n");
		return -ENODEV;
	} else if (count < 0) {
		pr_err(PREFIX "Error parsing GIC distributor entry\n");
		return count;
	}

	return 0;
}

int acpi_gsi_to_irq(u32 gsi, unsigned int *irq)
{
	*irq = gsi_to_irq(gsi);

	return 0;
}
EXPORT_SYMBOL_GPL(acpi_gsi_to_irq);

/*
 * success: return IRQ number (>=0)
 * failure: return < 0
 */
int acpi_register_gsi(struct device *dev, u32 gsi, int trigger, int polarity)
{
	//unsigned int irq;
	unsigned int irq_type;

	/*
	 * ACPI have no bindings to indicate SPI or PPI, so we
	 * use following mapping from DT to ACPI.
	 *
	 * For FDT
	 * PPI interrupt: in the range [0, 15];
	 * SPI interrupt: in the range [0, 987];
	 *
	 * For ACPI, using identity mapping for hwirq:
	 * PPI interrupt: in the range [16, 31];
	 * SPI interrupt: in the range [32, 1019];
	 */

	if (trigger == ACPI_EDGE_SENSITIVE &&
	    polarity == ACPI_ACTIVE_LOW)
		irq_type = IRQ_TYPE_EDGE_FALLING;
	else if (trigger == ACPI_EDGE_SENSITIVE &&
		 polarity == ACPI_ACTIVE_HIGH)
		irq_type = IRQ_TYPE_EDGE_RISING;
	else if (trigger == ACPI_LEVEL_SENSITIVE &&
		 polarity == ACPI_ACTIVE_LOW)
		irq_type = IRQ_TYPE_LEVEL_LOW;
	else if (trigger == ACPI_LEVEL_SENSITIVE &&
		 polarity == ACPI_ACTIVE_HIGH)
		irq_type = IRQ_TYPE_LEVEL_HIGH;
	else
		irq_type = IRQ_TYPE_NONE;

	/*
	 * Since only one GIC is supported in ACPI 5.0, we can
	 * create mapping refer to the default domain
	 */
	return irq_create_acpi_mapping(gsi, irq_type);
}
EXPORT_SYMBOL_GPL(acpi_register_gsi);

void acpi_unregister_gsi(u32 gsi)
{
}
EXPORT_SYMBOL_GPL(acpi_unregister_gsi);

static int __initdata setup_possible_cpus = -1;
static int __init _setup_possible_cpus(char *str)
{
	get_option(&str, &setup_possible_cpus);
	return 0;
}
early_param("possible_cpus", _setup_possible_cpus);

/*
 * cpu_possible_mask should be static, it cannot change as cpu's
 * are onlined, or offlined. The reason is per-cpu data-structures
 * are allocated by some modules at init time, and dont expect to
 * do this dynamically on cpu arrival/departure.
 * cpu_present_mask on the other hand can change dynamically.
 * In case when cpu_hotplug is not compiled, then we resort to current
 * behaviour, which is cpu_possible == cpu_present.
 * - Ashok Raj
 *
 * Three ways to find out the number of additional hotplug CPUs:
 * - If the BIOS specified disabled CPUs in ACPI/mptables use that.
 * - The user can overwrite it with possible_cpus=NUM
 * - Otherwise don't reserve additional CPUs.
 * We do this because additional CPUs waste a lot of memory.
 * -AK
 */
void __init prefill_possible_map(void)
{
	int i;
	int possible, disabled_cpus;

	disabled_cpus = total_cpus - available_cpus;

	if (setup_possible_cpus == -1) {
		if (disabled_cpus > 0)
			setup_possible_cpus = disabled_cpus;
		else
			setup_possible_cpus = 0;
	}

	possible = available_cpus + setup_possible_cpus;

	pr_info("SMP: the system is limited to %d CPUs\n", nr_cpu_ids);

	/*
	 * On armv8 foundation model --cores=4 lets nr_cpu_ids=4, so we can't
	 * get possible map correctly when more than 4 APIC entries in MADT.
	 */
	if (possible > nr_cpu_ids)
		possible = nr_cpu_ids;

	pr_info("SMP: Allowing %d CPUs, %d hotplug CPUs\n",
		possible, max((possible - available_cpus), 0));

	for (i = 0; i < possible; i++)
		set_cpu_possible(i, true);
	for (; i < NR_CPUS; i++)
		set_cpu_possible(i, false);
}

/*
 *  ACPI based hotplug support for CPU
 */
#ifdef CONFIG_ACPI_HOTPLUG_CPU
#include <acpi/processor.h>

static void acpi_map_cpu2node(acpi_handle handle, int cpu, int physid)
{
#ifdef CONFIG_ACPI_NUMA
	int nid;

	nid = acpi_get_node(handle);
	if (nid == -1 || !node_online(nid))
		return;
	set_apicid_to_node(physid, nid);
	numa_set_node(cpu, nid);
#endif
}

static int _acpi_map_lsapic(acpi_handle handle, int *pcpu)
{
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *obj;
	struct acpi_madt_generic_interrupt *lapic;
	cpumask_var_t tmp_map, new_map;
	u8 physid;
	int cpu;
	int retval = -ENOMEM;

	if (ACPI_FAILURE(acpi_evaluate_object(handle, "_MAT", NULL, &buffer)))
		return -EINVAL;

	if (!buffer.length || !buffer.pointer)
		return -EINVAL;

	obj = buffer.pointer;
	if (obj->type != ACPI_TYPE_BUFFER ||
	    obj->buffer.length < sizeof(*lapic)) {
		kfree(buffer.pointer);
		return -EINVAL;
	}

	lapic = (struct acpi_madt_generic_interrupt *)obj->buffer.pointer;

	if (lapic->header.type != ACPI_MADT_TYPE_GENERIC_INTERRUPT ||
	    !(lapic->flags & ACPI_MADT_ENABLED)) {
		kfree(buffer.pointer);
		return -EINVAL;
	}

	physid = lapic->gic_id;

	kfree(buffer.pointer);
	buffer.length = ACPI_ALLOCATE_BUFFER;
	buffer.pointer = NULL;
	lapic = NULL;

	if (!alloc_cpumask_var(&tmp_map, GFP_KERNEL))
		goto out;

	if (!alloc_cpumask_var(&new_map, GFP_KERNEL))
		goto free_tmp_map;

	cpumask_copy(tmp_map, cpu_present_mask);
	acpi_register_lapic(physid, ACPI_MADT_ENABLED);

	/*
	 * If acpi_register_lapic successfully generates a new logical cpu
	 * number, then the following will get us exactly what was mapped
	 */
	cpumask_andnot(new_map, cpu_present_mask, tmp_map);
	if (cpumask_empty(new_map)) {
		pr_err("Unable to map lapic to logical cpu number\n");
		retval = -EINVAL;
		goto free_new_map;
	}

	acpi_processor_set_pdc(handle);

	cpu = cpumask_first(new_map);
	acpi_map_cpu2node(handle, cpu, physid);

	*pcpu = cpu;
	retval = 0;

free_new_map:
	free_cpumask_var(new_map);
free_tmp_map:
	free_cpumask_var(tmp_map);
out:
	return retval;
}

/* wrapper to silence section mismatch warning */
int __ref acpi_map_lsapic(acpi_handle handle, int *pcpu)
{
	return _acpi_map_lsapic(handle, pcpu);
}
EXPORT_SYMBOL(acpi_map_lsapic);

int acpi_unmap_lsapic(int cpu)
{
	arm_cpu_to_apicid[cpu] = -1;
	set_cpu_present(cpu, false);
	available_cpus--;

	return 0;
}
EXPORT_SYMBOL(acpi_unmap_lsapic);
#endif				/* CONFIG_ACPI_HOTPLUG_CPU */

static int __init acpi_parse_fadt(struct acpi_table_header *table)
{
	return 0;
}

/* Parked Address in ACPI GIC structure can be used as cpu release addr */
int acpi_get_cpu_release_address(int cpu, u64 *release_address)
{
	if (!release_address || !parked_address[cpu])
		return -EINVAL;

	*release_address = parked_address[cpu];

	return 0;
}

static void __init early_acpi_process_madt(void)
{
	/* should I introduce CONFIG_ARM_LOCAL_APIC like x86 does? */
	acpi_table_parse(ACPI_SIG_MADT, acpi_parse_madt);
}

static void __init acpi_process_madt(void)
{
	int error;

	if (!acpi_table_parse(ACPI_SIG_MADT, acpi_parse_madt)) {

		/*
		 * Parse MADT GIC cpu interface entries
		 */
		error = acpi_parse_madt_gic_entries();
		if (!error) {
			/*
			 * Parse MADT GIC distributor entries
			 */
			//acpi_parse_madt_gic_distributor_entries();
		}
	}

	pr_info("Using ACPI for processor (GIC) configuration information\n");

 	return;
}

 /*
 * To see PCSI is enabled or not.
 *
 * PSCI is not available for ACPI 5.0, return FALSE for now.
 *
 * FIXME: should we introduce early_param("psci", func) for test purpose?
 */
static bool acpi_psci_smp_available(int cpu)
{
	return FALSE;
}

static const char *enable_method[] = {
	"psci",
	"spin-table",
	NULL
};

const char *acpi_get_enable_method(int cpu)
{
	if (acpi_psci_smp_available(cpu))
		return enable_method[0];
	else
		return enable_method[1];
}


/*
 * ========== OLD COMMENTS FROM x86 =================================
 * acpi_boot_table_init() and acpi_boot_init()
 *  called from setup_arch(), always.
 *	1. checksums all tables
 *	2. enumerates lapics
 *	3. enumerates io-apics
 *
 * acpi_table_init() is separate to allow reading SRAT without
 * other side effects.
 *
 * side effects of acpi_boot_init:
 *	if acpi_blacklisted() acpi_disabled = 1;
 *	acpi_irq_model=...
 *	...
 * ==================================================================
 *
 * We have to approach this a little different on ARMv7.  We are
 * passed in an ACPI blob and we really have no idea where in RAM
 * it will be located.  So, what should have been the physical
 * addresses of other tables cannot really be hardcoded into the
 * tables.  What we will do is put an offset in the blob that is
 * the offset from the beginning of the RSDP structure.  However,
 * what that means is that we have to unpack the blob and do a
 * bit of fixup work on the offsets to turn them into kernel
 * virtual addresses so we can pass them on for later use.
 */
void __init acpi_boot_table_init(void)
{
	/*
	 * If acpi_disabled, bail out
	 */
	if (acpi_disabled)
		return;

	/*
	 * Initialize the ACPI boot-time table parser.
	 */
	if (acpi_table_init()) {
		disable_acpi();
		return;
	}
}

int __init early_acpi_boot_init(void)
{
	/*
	 * If acpi_disabled, bail out
	 */
	if (acpi_disabled)
		return -ENODEV;

	/* Get the boot CPU's GIC cpu interface id before MADT parsing */
	boot_cpu_apic_id = read_cpuid_mpidr() & MPIDR_HWID_BITMASK;

	/*
	 * Process the Multiple APIC Description Table (MADT), if present
	 */
	early_acpi_process_madt();

	return 0;
}

int __init acpi_boot_init(void)
{
	/*
	 * If acpi_disabled, bail out
	 */
	if (acpi_disabled)
		return 1;

	/*
	 * set sci_int and PM timer address
	 */
	acpi_table_parse(ACPI_SIG_FADT, acpi_parse_fadt);

	/*
	 * Process the Multiple APIC Description Table (MADT), if present
	 */
	acpi_process_madt();

	return 0;
}

/*
 * make sure that the address passed for ACPI tables which is normall in
 * system ram is removed from the kernel memory map
 */
void __init arm_acpi_reserve_memory()
{
	unsigned long section_offset;
	unsigned long num_sections;
	phys_addr_t addr = acpi_arm_rsdp_info.phys_address;
	int size = acpi_arm_rsdp_info.size;

	/* if ACPI tables were not passed in FDT then escape here */
	if (!addr || !size)
		return;

	section_offset = addr - (addr & SECTION_MASK);
	num_sections = size / SECTION_SIZE;
	if (size % SECTION_SIZE)
		num_sections++;

	memblock_remove(addr - section_offset, num_sections * SECTION_SIZE);
}

int __init acpi_gic_init(void)
{
	/*
	 * Parse MADT GIC distributor entries
	 */	 
	return acpi_parse_madt_gic_distributor_entries();
}

static int __init parse_acpi(char *arg)
{
	if (!arg)
		return -EINVAL;

	/* "acpi=off" disables both ACPI table parsing and interpreter */
	if (strcmp(arg, "off") == 0) {
		disable_acpi();
	}
	/* acpi=strict disables out-of-spec workarounds */
	else if (strcmp(arg, "strict") == 0) {
		acpi_strict = 1;
	}
	return 0;
}
early_param("acpi", parse_acpi);


/*
 * acpi_suspend_lowlevel() - save kernel state and suspend.
 *
 * TBD when ARM/ARM64 starts to support suspend...
 */
int (*acpi_suspend_lowlevel)(void);

int acpi_isa_irq_to_gsi(unsigned isa_irq, u32 *gsi)
{
	return -1;
}


int acpi_register_ioapic(acpi_handle handle, u64 phys_addr, u32 gsi_base)
{
	return -EINVAL;
}
EXPORT_SYMBOL(acpi_register_ioapic);

int acpi_unregister_ioapic(acpi_handle handle, u32 gsi_base)
{
	return -EINVAL;
}
EXPORT_SYMBOL(acpi_unregister_ioapic);

int raw_pci_read(unsigned int domain, unsigned int bus, unsigned int devfn,
                                                int reg, int len, u32 *val)
{
        return -EINVAL;
}

int raw_pci_write(unsigned int domain, unsigned int bus, unsigned int devfn,
                                                int reg, int len, u32 val)
{
        return -EINVAL;
}
