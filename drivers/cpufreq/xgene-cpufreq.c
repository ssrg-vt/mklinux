/*
 * AppliedMicro X-Gene CPU Frequency Driver (for Device Tree)
 *
 * Copyright (c) 2014, Applied Micro Circuits Corporation
 * Author: Loc Ho <lho@apm.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/clk-provider.h>
#include <linux/slab.h>
#include <linux/efi.h>

struct xgene_cpufreq_data {
	struct cpufreq_frequency_table freq_table[5];
	atomic_t freq_table_users;
	struct clk *pmd_clk;
};

#define CPU_PER_PMD		2

static DEFINE_PER_CPU(struct xgene_cpufreq_data *, cpufreq_data);
static DEFINE_MUTEX(xgene_cpufreq_lock);
static DEFINE_SPINLOCK(pmd_clk_lock);
static bool is_suspended;

static inline int xgene_cpufreq_to_pmd(int cpu)
{
	return cpu / CPU_PER_PMD;
}

static int xgene_cpufreq_verify_speed(struct cpufreq_policy *policy)
{
	struct xgene_cpufreq_data *ctx;

	ctx = per_cpu(cpufreq_data, policy->cpu);
	return cpufreq_frequency_table_verify(policy, ctx->freq_table);
}

static unsigned int xgene_cpufreq_get_speed(unsigned int cpu)
{
	struct xgene_cpufreq_data *ctx;

	ctx = per_cpu(cpufreq_data, cpu);
	return (unsigned int) (clk_get_rate(ctx->pmd_clk) / 1000);
}

static int xgene_cpufreq_set_rate(struct xgene_cpufreq_data *ctx,
				  unsigned long rate)
{
	int rc;

	rc = clk_set_rate(ctx->pmd_clk, rate);
	if (rc) {
		pr_err("cpufreq-xgene: Failed to change PMD clock to %lu\n",
		       rate);
		return rc;
	}
	return rc;
}

static int xgene_cpufreq_update_cpu_speed(struct cpufreq_policy *policy,
					  unsigned long rate)
{
	struct xgene_cpufreq_data *ctx;
	struct cpufreq_freqs freqs;
	int rc = 0;

	ctx = per_cpu(cpufreq_data, policy->cpu);

	freqs.old = xgene_cpufreq_get_speed(policy->cpu);
	freqs.new = rate;

	if (freqs.old == freqs.new)
		return rc;

	cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);

#ifdef CONFIG_CPU_FREQ_DEBUG
	pr_debug("cpufreq-xgene: transition %u kHz to %u kHz\n",
		 freqs.old, freqs.new);
#endif

	rc = xgene_cpufreq_set_rate(ctx, freqs.new * 1000);
	if (rc) {
		pr_err("cpufreq-xgene: Failed to set cpu frequency to %d kHz\n",
			freqs.new);
		freqs.new = freqs.old;
	}

	cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);

	return rc;
}

static int xgene_cpufreq_target(struct cpufreq_policy *policy,
				unsigned int target_freq,
				unsigned int relation)
{
	struct xgene_cpufreq_data *ctx;
	unsigned int idx;
	int rc = 0;

	mutex_lock(&xgene_cpufreq_lock);

	if (is_suspended) {
		rc = -EBUSY;
		goto out;
	}

	ctx = per_cpu(cpufreq_data, policy->cpu);
	cpufreq_frequency_table_target(policy, ctx->freq_table, target_freq,
				       relation, &idx);
	rc = xgene_cpufreq_update_cpu_speed(policy,
					    ctx->freq_table[idx].frequency);

out:
	mutex_unlock(&xgene_cpufreq_lock);
	return rc;
}

static struct xgene_cpufreq_data * xgene_cpufreq_create_ctx(
	struct cpufreq_policy *policy)
{
	struct xgene_cpufreq_data *ctx;
	struct clk *pcp_clk;
	void __iomem *pmd_csr;
	int pmd = xgene_cpufreq_to_pmd(policy->cpu);
	char clk_name[20];
	int i;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (ctx == NULL)
		return NULL;

	pcp_clk = clk_get_sys(NULL, "pcppll");
	if (IS_ERR(pcp_clk)) {
		pr_err("cpufreq-xgene: no PCP clock\n");
		goto err_clk;
	}

	pmd_csr = ioremap(0x7c000000 + 0x02200000 + 0x200 + 0x10 * pmd, 0x10);
	if (pmd_csr == NULL) {
		pr_err("cpufreq-xgene: unable to map PMD CSR\n");
		goto err_csr;
	}
	sprintf(clk_name, "pmd%dclk", pmd);
	ctx->pmd_clk = clk_register_divider(NULL, clk_name, "pcppll", 0,
					    pmd_csr, 12, 2, 0, &pmd_clk_lock);
	if (IS_ERR(ctx->pmd_clk)) {
		pr_err("X-Gene unable register PMD clock\n");
		goto err_pmd_clk;
	}

	for (i = 0; i < 4; i++) {
		ctx->freq_table[i].driver_data = i;
		ctx->freq_table[i].frequency = clk_get_rate(pcp_clk) / (4 - i)
						/ 1000;
	}
	ctx->freq_table[i].driver_data = i;
	ctx->freq_table[i].frequency = CPUFREQ_TABLE_END;

	atomic_set(&ctx->freq_table_users, 1);

	clk_put(pcp_clk);

	return ctx;

err_pmd_clk:
	iounmap(pmd_csr);

err_csr:
	clk_put(pcp_clk);

err_clk:
	kfree(ctx);
	return NULL;
}

static int xgene_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	struct xgene_cpufreq_data *ctx;
	int rc = 0;

	if (policy->cpu >= NR_CPUS)
		return -EINVAL;

	mutex_lock(&xgene_cpufreq_lock);

	/* Retrieve or create the PMD context */
	ctx = per_cpu(cpufreq_data, policy->cpu);
	if (ctx == NULL)
		ctx = per_cpu(cpufreq_data, (policy->cpu + 1) % CPU_PER_PMD);
	if (ctx == NULL) {
		ctx = xgene_cpufreq_create_ctx(policy);
		if (ctx == NULL) {
			rc = -ENOMEM;
			goto err_mem;
		}
	} else {
		atomic_inc(&ctx->freq_table_users);
	}
	per_cpu(cpufreq_data, policy->cpu) = ctx;

	cpufreq_frequency_table_cpuinfo(policy, ctx->freq_table);
	cpufreq_frequency_table_get_attr(ctx->freq_table, policy->cpu);
	policy->cur = xgene_cpufreq_get_speed(policy->cpu);

	/*
	 * On X-Gene SMP configuartion, an pair processors share the clock.
         * So the pair of CPUs needs to be scaled together and hence
	 * needs software co-ordination. Use cpufreq affected_cpus
	 * interface to handle this scenario. Additional is_smp() check
	 * is to keep SMP_ON_UP build working.
	 */
	cpumask_set_cpu(policy->cpu, policy->cpus);
	if (policy->cpu & 0x1){
		if (cpu_online(policy->cpu - 1))
			cpumask_set_cpu(policy->cpu - 1, policy->cpus);
	} else {
		if (cpu_online(policy->cpu + 1))
			cpumask_set_cpu(policy->cpu + 1, policy->cpus);
	}

	policy->cpuinfo.transition_latency = 300 * 1000;

err_mem:
	mutex_unlock(&xgene_cpufreq_lock);
	return rc;
}

static int xgene_cpufreq_cpu_exit(struct cpufreq_policy *policy)
{
	struct xgene_cpufreq_data *ctx;

	mutex_lock(&xgene_cpufreq_lock);

	ctx = per_cpu(cpufreq_data, policy->cpu);
	if (atomic_dec_return(&ctx->freq_table_users) == 1) {
		/* Last user, free it */
		kfree(ctx);
		per_cpu(cpufreq_data, policy->cpu) = NULL;
	}

	mutex_unlock(&xgene_cpufreq_lock);
	return 0;
}

static struct freq_attr *xgene_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver xgene_cpufreq_driver = {
	.verify		= xgene_cpufreq_verify_speed,
	.target		= xgene_cpufreq_target,
	.get		= xgene_cpufreq_get_speed,
	.init		= xgene_cpufreq_cpu_init,
	.exit		= xgene_cpufreq_cpu_exit,
	.name		= "X-Gene",
	.attr		= xgene_cpufreq_attr,
};

static int __init xgene_cpufreq_init(void)
{
	if (efi_enabled(EFI_BOOT))
		return 0;

	return cpufreq_register_driver(&xgene_cpufreq_driver);
}
module_init(xgene_cpufreq_init);

static void __exit xgene_cpufreq_exit(void)
{
	if (efi_enabled(EFI_BOOT))
		return;

	cpufreq_unregister_driver(&xgene_cpufreq_driver);
}
module_exit(xgene_cpufreq_exit);

MODULE_AUTHOR("Loc Ho <lho@apm.com>");
MODULE_DESCRIPTION("CPUFreq driver for X-Gene SoC");
MODULE_LICENSE("GPL");
