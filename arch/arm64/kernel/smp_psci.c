/*
 * PSCI SMP initialisation
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

#include <linux/init.h>
#include <linux/of.h>
#include <linux/smp.h>

#include <asm/psci.h>
#include <asm/smp_plat.h>

static int smp_psci_cpu_init(struct device_node *dn, unsigned int cpu)
{
	return 0;
}

static int smp_psci_cpu_prepare(unsigned int cpu)
{
	if (!psci_ops.cpu_on) {
		pr_err("psci: no cpu_on method, not booting CPU%d\n", cpu);
		return -ENODEV;
	}

	return 0;
}

static int smp_psci_cpu_boot(unsigned int cpu)
{
	int err = psci_ops.cpu_on(cpu_logical_map(cpu), __pa(secondary_entry));
	if (err)
		pr_err("psci: failed to boot CPU%d (%d)\n", cpu, err);

	return err;
}

#ifdef CONFIG_HOTPLUG_CPU
static int smp_psci_cpu_disable(unsigned int cpu)
{
	/* Fail early if we don't have CPU_OFF support */
	if (!psci_ops.cpu_off)
		return -EOPNOTSUPP;
	return 0;
}

static void smp_psci_cpu_die(unsigned int cpu)
{
	int ret;
	/*
	 * There are no known implementations of PSCI actually using the
	 * power state field, pass a sensible default for now.
	 */
	struct psci_power_state state = {
		.type = PSCI_POWER_STATE_TYPE_POWER_DOWN,
	};

	ret = psci_ops.cpu_off(state);

	pr_crit("psci: unable to power off CPU%u (%d)\n", cpu, ret);
}
#endif

const struct smp_operations smp_psci_ops = {
	.name		= "psci",
	.cpu_init	= smp_psci_cpu_init,
	.cpu_prepare	= smp_psci_cpu_prepare,
	.cpu_boot	= smp_psci_cpu_boot,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_disable	= smp_psci_cpu_disable,
	.cpu_die	= smp_psci_cpu_die,
#endif
};
