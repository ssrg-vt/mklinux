/*
 * CPU kernel entry/exit control
 *
 * Copyright (C) 2013 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <asm/cpu_ops.h>
#include <asm/smp_plat.h>
#include <linux/errno.h>
#include <linux/of.h>
#include <linux/string.h>
#include <asm/acpi.h>
 
extern const struct cpu_operations smp_spin_table_ops;
extern const struct cpu_operations cpu_psci_ops;

const struct cpu_operations *cpu_ops[NR_CPUS];
 
static const struct cpu_operations *supported_cpu_ops[] __initconst = {
#ifdef CONFIG_SMP
	&smp_spin_table_ops,
	&cpu_psci_ops,
#endif
	NULL,
};
 
const struct cpu_operations * __init cpu_get_ops(const char *name)
{
	const struct cpu_operations **ops = supported_cpu_ops;
 
	while (*ops) {
		if (!strcmp(name, (*ops)->name))
			return *ops;

		ops++;
	}

	return NULL;
}
 
/*
 * Read a cpu's enable method from the device tree and record it in cpu_ops.
 */
int __init cpu_of_read_ops(struct device_node *dn, int cpu)
{
        const char *enable_method = of_get_property(dn, "enable-method", NULL);
        if (!enable_method) {
                /*
                 * The boot CPU may not have an enable method (e.g. when
                 * spin-table is used for secondaries). Don't warn spuriously.
                 */
                if (cpu != 0)
                        pr_err("%s: missing enable-method property\n",
                                dn->full_name);
                return -ENOENT;
        }

        cpu_ops[cpu] = cpu_get_ops(enable_method);
        if (!cpu_ops[cpu]) {
                pr_warn("%s: unsupported enable-method property: %s\n",
                        dn->full_name, enable_method);
                return -EOPNOTSUPP;
        }
 
        return 0;
}
 
#ifdef CONFIG_ACPI
/*
 * This is place holder for code which investigates bootup method for
 * a given CPU.
 * FIXME: Since PSCI is not available for ACPI 5.0 and we support FVP base model
 * only we hardcode PSCI method here.
 */

static const char *cpu_acpi_get_enable_method(int cpu)
{
         return "spin-table";//"psci";
}

/*
 * Read a cpu's enable method in the ACPI way and record it in cpu_ops.
 */
int __init cpu_acpi_read_ops(int cpu)
{
	const char *enable_method = cpu_acpi_get_enable_method(cpu);
        if (!enable_method) {
                /*
                 * The boot CPU may not have an enable method (e.g. when
                 * spin-table is used for secondaries). Don't warn spuriously.
                 */
                if (cpu != 0)
                        pr_err("Missing enable-method property for boot cpu\n");
                return -ENOENT;
        }

        cpu_ops[cpu] = cpu_get_ops(enable_method);
        if (!cpu_ops[cpu]) {
                pr_warn("CPU %d: unsupported enable-method property: %s\n",
                        cpu, enable_method);
                return -EOPNOTSUPP;
        }

        return 0;
}
#endif


void __init cpu_read_bootcpu_ops(void)
{
        struct device_node *dn = of_get_cpu_node(0, NULL);
        if (dn) {
                cpu_of_read_ops(dn, 0);
                return;
        }

        cpu_acpi_read_ops(0);
}
