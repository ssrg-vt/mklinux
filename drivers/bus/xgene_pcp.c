/**
 * xgene_pcp.c - AppliedMicro Xgene PCP Bridge Driver
 *
 * Copyright (c) 2013, Applied Micro Circuits Corporation
 * Author: Loc Ho <lho@apm.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * This module provides error reporting for IOB and other IP's.
 *
 */
#include <linux/module.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/acpi.h>
#include <linux/efi.h>
#include <linux/proc_fs.h>
#include "apm_pcp_csr.h"
#include "apm_iob_csr.h"

#define PCP_DRIVER_VER			"0.1"

#define PCPHPERRINTSTS_ADDR		0x000000
#define PCPHPERRINTMSK_ADDR		0x000004
#define PCPLPERRINTSTS_ADDR		0x000008
#define PCPLPERRINTMSK_ADDR		0x00000C
#define MEMERRINTSTS_ADDR		0x000010
#define MEMERRINTMSK_ADDR		0x000014

#define MCU_CTL_ERR_RD(src)		(((src) & 0x00001000)>>12)
#define IOB_PA_ERR_RD(src)		(((src) & 0x00000800)>>11)
#define IOB_BA_ERR_RD(src)		(((src) & 0x00000400)>>10)
#define IOB_XGIC_ERR_RD(src)		(((src) & 0x00000200)>>9)
#define IOB_RB_ERR_RD(src)		(((src) & 0x00000100)>>8)
#define L3C_UNCORR_RD(src)		(((src) & 0x00000020)>>5)
#define MCU_UNCORR_RD(src)		(((src) & 0x00000010)>>4)
#define PMD3_MERR_RD(src)		(((src) & 0x00000008)>>3)
#define PMD2_MERR_RD(src)		(((src) & 0x00000004)>>2)
#define PMD1_MERR_RD(src)		(((src) & 0x00000002)>>1)
#define PMD0_MERR_RD(src)		(((src) & 0x00000001)>>0)

#define CSW_SWITCH_TRACE_RD(src)	(((src) & 0x00000004)>>2)
#define L3C_CORR_RD(src)		(((src) & 0x00000002)>>1)
#define MCU_CORR_RD(src)		(((src) & 0x00000001)>>0)

#define L3C_ESR_ADDR			(0x0A * 4)
#define L3C_ECR_ADDR			(0x0B * 4)
#define L3C_ELR_ADDR			(0x0C * 4)
#define L3C_AELR_ADDR			(0x0D * 4)
#define L3C_BELR_ADDR			(0x0E * 4)

struct pcp_context
{
	struct platform_device *pdev;
	void *pcp_base;
	void *iob_base;
	void *pcprb_base;
	void *l3c_base;
	u32 pcp_irq[2];
	u32 mem_irq;
	int active_mcu;
	int disable_ce;
	int ce_threshold;
};

static void detect_mcu_cfg(struct pcp_context *ctx)
{
	unsigned int reg;

	/* Determine active MCUs */
	reg = readl(ctx->pcprb_base + 0x02200000); /* CSWCR */
	if (PCP_RB_CSW_CSWCR_DUALMCB_RD(reg)) {
		reg = readl(ctx->pcprb_base + 0x02720000); /* MCB-B MCBADDRMR */
		ctx->active_mcu = (reg & 0x00000004)? 0xF: 0x5;
	} else {
		reg = readl(ctx->pcprb_base + 0x02700000); /* MCB-A MCBADDRMR */
		ctx->active_mcu = (reg & 0x00000004)? 0x3: 0x1;
	}
}

static void update_mcu_reg(struct pcp_context *ctx,
			unsigned int reg, unsigned int value, unsigned int mask)
{
	unsigned int mcu, val;
	void *mcubase;

	/* Detect rank error Clear uncorrectable error interrupt */
	for (mcu = 0; mcu < 4; mcu++ ) {
		/* Skip inactive MCUs */
		if (!((ctx->active_mcu >> mcu) & 0x1))
			continue;

		mcubase = ctx->pcprb_base + PCP_RB_MCU0A_PAGE + mcu*0x40000;
		val = readl(mcubase + reg);
		val &= mask;
		val |= value;
		writel(val, mcubase + reg);
	}
}

#ifdef CONFIG_PROC_FS
static ssize_t disable_ce_read(struct file *file, char __user *buf,
			size_t count, loff_t *data)
{
	struct pcp_context *ctx = (struct pcp_context *) PDE_DATA(file_inode(file));

	return sprintf(buf, "%d\n", ctx->disable_ce);
}

static ssize_t disable_ce_write(struct file *file, const char __user *buf,
                           size_t count, loff_t *data)
{
	char value = 0;
	unsigned int reg;
	struct pcp_context *ctx = (struct pcp_context *) PDE_DATA(file_inode(file));

	if (copy_from_user(&value, buf, 1)) {
		return -EFAULT;
	}

	/*
	 * Disable L3 and MCU correctable
	 */
	if (value == 0x33) {
		ctx->disable_ce = 3;
		writel(0x00000007, ctx->pcp_base + PCPLPERRINTMSK_ADDR);
		update_mcu_reg(ctx, 0x110, 0, 0xfffffffb);

		reg = readl(ctx->l3c_base + L3C_ECR_ADDR);
		reg &= 0xa;
		writel(reg, ctx->l3c_base + L3C_ECR_ADDR);
	}

	/*
	 * Disable MCU correctable
	 */
	if (value == 0x31) {
		ctx->disable_ce = 1;
		writel(0x00000005, ctx->pcp_base + PCPLPERRINTMSK_ADDR);
		update_mcu_reg(ctx, 0x110, 0, 0xfffffffb);

		reg = readl(ctx->l3c_base + L3C_ECR_ADDR);
		reg |= 0x5;
		writel(reg, ctx->l3c_base + L3C_ECR_ADDR);
	}

	/*
	 * Enable L3 and MCU correctable
	 */
	if (value == 0x30) {
		ctx->disable_ce = 0;
		writel(0x00000004, ctx->pcp_base + PCPLPERRINTMSK_ADDR);
		update_mcu_reg(ctx, 0x110, 0x4, 0xfffffffb);

		reg = readl(ctx->l3c_base + L3C_ECR_ADDR);
		reg |= 0x5;
		writel(reg, ctx->l3c_base + L3C_ECR_ADDR);
	}
	return count;
}

static ssize_t ce_threshold_read(struct file *file, char __user *buf,
		size_t count, loff_t *data)
{
	struct pcp_context *ctx = (struct pcp_context *) PDE_DATA(file_inode(file));

	return sprintf(buf, "%d\n", ctx->ce_threshold);
}

static ssize_t ce_threshold_write(struct file *file, const char __user *buf,
                           size_t count, loff_t *data)
{
	char value[20];
	unsigned int threshold = 1;
	struct pcp_context *ctx = (struct pcp_context *) PDE_DATA(file_inode(file));

	if (copy_from_user(value, buf, count)) {
		return -EFAULT;
	}

	threshold = simple_strtol(value, 0, 10);
	if (threshold == 0)
		ctx->ce_threshold = 1;
	else
		ctx->ce_threshold = threshold & 0xffff;

	update_mcu_reg(ctx, 0x110, ctx->ce_threshold << 16, 0xffff);
	return count;
}
#endif

static void xgene_pmd_error_report(void *pcprb_base, int pmd)
{
	u32 rcpu_pg_d;
	u32 rcpu_pg_e;
	u32 rcpu_pg_f;
	u32 val;
	u32 val_hi;
	u32 val_lo;
	int i;

	for (i = 0; i < 2; i++) {
		int cpu = pmd * 2 + i;
		rcpu_pg_f = cpu * PCP_RB_CPU1_ROM_PAGE +
			    PCP_RB_CPU0_MEMERR_CPU_PAGE;
		val = readl(pcprb_base + rcpu_pg_f +
			PCP_RB_CPUX_MEMERR_CPU_ICFESR_PAGE_OFFSET);
		if (val) {
			printk(KERN_ERR "CPU%d L1 memory error ICF 0x%08X\n",
				cpu, val);
			printk(KERN_ERR
				"ErrWay 0x%02X ErrIndex 0x%02X ErrInfo 0x%02X\n",
				PCP_RB_CPUX_MEMERR_CPU_ICFESR_ERRWAY_RD(val),
				PCP_RB_CPUX_MEMERR_CPU_ICFESR_ERRINDEX_RD(val),
				PCP_RB_CPUX_MEMERR_CPU_ICFESR_ERRINFO_RD(val));
			if (val & PCP_RB_CPUX_MEMERR_CPU_ICFESR_CERR_MASK)
				printk(KERN_ERR
					"One or more correctable error\n");
			if (val & PCP_RB_CPUX_MEMERR_CPU_ICFESR_MULTCERR_MASK)
				printk(KERN_ERR
					"Multiple correctable error\n");
			switch (PCP_RB_CPUX_MEMERR_CPU_ICFESR_ERRTYPE_RD(val)) {
			case 1:
				printk(KERN_ERR "L1 TLB multiple hit\n");
				break;
			case 2:
				printk(KERN_ERR "Way select multiple hit\n");
				break;
			case 3:
				printk(KERN_ERR "Physical tag parity error\n");
				break;
			case 4:
			case 5:
				printk(KERN_ERR "L1 data parity error\n");
				break;
			case 6:
				printk(KERN_ERR "L1 pre-decode parity error\n");
				break;
			}
			writel(0x00000000, pcprb_base + rcpu_pg_f +
				PCP_RB_CPUX_MEMERR_CPU_ICFESRA_PAGE_OFFSET);
		}
		val = readl(pcprb_base + rcpu_pg_f +
			PCP_RB_CPUX_MEMERR_CPU_LSUESR_PAGE_OFFSET);
		if (val) {
			printk(KERN_ERR "CPU%d memory error LSU 0x%08X\n",
				cpu, val);

			printk(KERN_ERR
				"ErrWay 0x%02X ErrIndex 0x%02X ErrInfo 0x%02X\n",
				PCP_RB_CPUX_MEMERR_CPU_LSUESR_ERRWAY_RD(val),
				PCP_RB_CPUX_MEMERR_CPU_LSUESR_ERRINDEX_RD(val),
				PCP_RB_CPUX_MEMERR_CPU_LSUESR_ERRINFO_RD(val));
			if (val & PCP_RB_CPUX_MEMERR_CPU_LSUESR_CERR_MASK)
				printk(KERN_ERR
					"One or more correctable error\n");
			if (val & PCP_RB_CPUX_MEMERR_CPU_LSUESR_MULTCERR_MASK)
				printk(KERN_ERR
					"Multiple correctable error\n");
			switch (PCP_RB_CPUX_MEMERR_CPU_LSUESR_ERRTYPE_RD(val)) {
			case 0:
				printk(KERN_ERR "Load tag error\n");
                                break;	
			case 1:
				printk(KERN_ERR "Load data error\n");
                                break;	
			case 2:
				printk(KERN_ERR "WSL multihit error\n");
                                break;	
			case 3:
				printk(KERN_ERR "Store tag error\n");
                                break;	
			case 4:
				printk(KERN_ERR "DTB multihit from load pipeline error\n");
                                break;	
			case 5:
				printk(KERN_ERR "DTB multihit from store pipeline error\n");
                                break;	
			}

			writel(0x00000000, pcprb_base + rcpu_pg_f +
				PCP_RB_CPUX_MEMERR_CPU_LSUESRA_PAGE_OFFSET);
		}
		val = readl(pcprb_base + rcpu_pg_f +
			PCP_RB_CPUX_MEMERR_CPU_MMUESR_PAGE_OFFSET);
		if (val) {
			printk(KERN_ERR "CPU%d memory error MMU 0x%08X\n",
				cpu, val);
			writel(0x00000000, pcprb_base + rcpu_pg_f +
				PCP_RB_CPUX_MEMERR_CPU_MMUESRA_PAGE_OFFSET);
		}
	}

	rcpu_pg_d = pmd * 2 * PCP_RB_CPU1_ROM_PAGE + PCP_RB_CPU0_L2C_PAGE;
	rcpu_pg_e = pmd * 2 * PCP_RB_CPU1_ROM_PAGE +
		    PCP_RB_CPU0_MEMERR_L2C_PAGE;

	val = readl(pcprb_base + rcpu_pg_e +
			PCP_RB_CPUX_MEMERR_L2C_L2ESR_PAGE_OFFSET);
	if (val) {
		val_lo = readl(pcprb_base + rcpu_pg_e +
				PCP_RB_CPUX_MEMERR_L2C_L2EALR_PAGE_OFFSET);
		val_hi = readl(pcprb_base + rcpu_pg_e +
				PCP_RB_CPUX_MEMERR_L2C_L2EAHR_PAGE_OFFSET);
		printk(KERN_ERR "PMD%d memory error L2C L2ESR 0x%08X @ "
			"0x%08X.%08X\n", pmd, val, val_hi, val_lo);

		printk(KERN_ERR
			"ErrSyndrome 0x%02x ErrWay 0x%02X ErrCpu %d ErrGroup 0x%02X ErrAction 0x%02X\n",
				PCP_RB_CPUX_MEMERR_L2C_L2ESR_ERRSYN_RD(val),
				PCP_RB_CPUX_MEMERR_L2C_L2ESR_ERRWAY_RD(val),
				PCP_RB_CPUX_MEMERR_L2C_L2ESR_ERRCPU_RD(val),
				PCP_RB_CPUX_MEMERR_L2C_L2ESR_ERRGROUP_RD(val),
				PCP_RB_CPUX_MEMERR_L2C_L2ESR_ERRACTION_RD(val));				

		if (val & PCP_RB_CPUX_MEMERR_L2C_L2ESR_ERR_RD(val))
			printk(KERN_ERR
					"One or more correctable error\n");
		if (val & PCP_RB_CPUX_MEMERR_L2C_L2ESR_MULTICERR_RD(val))
			printk(KERN_ERR
					"Multiple correctable error\n");
		if (val & PCP_RB_CPUX_MEMERR_L2C_L2ESR_UCERR_RD(val))
			printk(KERN_ERR
					"One or more uncorrectable error\n");
		if (val & PCP_RB_CPUX_MEMERR_L2C_L2ESR_MULTUCERR_RD(val))
			printk(KERN_ERR
					"Multiple uncorrectable error\n");

		switch (PCP_RB_CPUX_MEMERR_L2C_L2ESR_ERRTYPE_RD(val)) {
			case 0:
				printk(KERN_ERR "Outbound SDB parity error\n");
				break;	
			case 1:
				printk(KERN_ERR "Inbound SDB parity error\n");
				break;	
			case 2:
				printk(KERN_ERR "Tag ECC error\n");
				break;	
			case 3:
				printk(KERN_ERR "Data ECC error\n");
				break;	
		}

		writel(0x00000000, pcprb_base + rcpu_pg_e +
			PCP_RB_CPUX_MEMERR_L2C_L2EALR_PAGE_OFFSET);
		writel(0x00000000, pcprb_base + rcpu_pg_e +
			PCP_RB_CPUX_MEMERR_L2C_L2EAHR_PAGE_OFFSET);
		writel(0x00000000, pcprb_base + rcpu_pg_e +
			PCP_RB_CPUX_MEMERR_L2C_L2ESRA_PAGE_OFFSET);
	}
	val = readl(pcprb_base + rcpu_pg_d +
			PCP_RB_CPUX_L2C_L2RTOSR_PAGE_OFFSET);
	if (val) {
		val_lo = readl(pcprb_base + rcpu_pg_d +
				PCP_RB_CPUX_L2C_L2RTOALR_PAGE_OFFSET);
		val_hi = readl(pcprb_base + rcpu_pg_d +
				PCP_RB_CPUX_L2C_L2RTOAHR_PAGE_OFFSET);
		printk(KERN_ERR "PMD%d L2C error L2C RTOSR 0x%08X @ "
			"0x%08X.%08X\n", pmd, val, val_hi, val_lo);
		writel(0x00000000, pcprb_base + rcpu_pg_d +
			PCP_RB_CPUX_L2C_L2RTOALR_PAGE_OFFSET);
		writel(0x00000000, pcprb_base + rcpu_pg_d +
			PCP_RB_CPUX_L2C_L2RTOAHR_PAGE_OFFSET);
		writel(0x00000000, pcprb_base + rcpu_pg_d +
			PCP_RB_CPUX_L2C_L2RTOSR_PAGE_OFFSET);
	}
}

static irqreturn_t xgene_pcp_irq(int irq, void *data)
{
	struct pcp_context *ctx = (struct pcp_context *) data;
	u32 pcphpreg;
	u32 pcplpreg;
	u32 err_addr_lo;
	u32 err_addr_hi;
	u32 reg;
	void* mcubase;
	int ddr_rank, mcu;
	int i;
	static char * mem_err_ip[] = {
		"10GbE0",
		"10GbE1",
		"Security",
		"SATA45",
		"SATA23/ETH23",
		"SATA01/ETH01",
		"USB1",
		"USB0",
		"QML",
		"QM0",
		"QM1 (XGbE01)",
		"PCIE4",
		"PCIE3",
		"PCIE2",
		"PCIE1",
		"PCIE0",
		"CTX Manager",
		"OCM",
		"1GbE",
		"CLE",
		"AHBC",
		"PktDMA",
		"GFC",
		"MSLIM",
		"10GbE2",
		"10GbE3",
		"QM2 (XGbE23)",
		"IOB",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
	};

	/* Retrieve top level interrupt status from IRQ32 */
	pcphpreg = readl(ctx->pcp_base + PCPHPERRINTSTS_ADDR);
	pcplpreg = readl(ctx->pcp_base + PCPLPERRINTSTS_ADDR);

	if (PMD0_MERR_RD(pcphpreg)) {
		/* PMD0 memory error interrupt */
		dev_err(&ctx->pdev->dev, "PMD0 memory error\n");
		xgene_pmd_error_report(ctx->pcprb_base, 0);
	}
	if (PMD1_MERR_RD(pcphpreg)) {
		/* PMD1 memory error interrupt */
		dev_err(&ctx->pdev->dev, "PMD1 memory error\n");
		xgene_pmd_error_report(ctx->pcprb_base, 1);
	}
	if (PMD2_MERR_RD(pcphpreg)) {
		/* PMD2 memory error interrupt */
		dev_err(&ctx->pdev->dev, "PMD2 memory error\n");
		xgene_pmd_error_report(ctx->pcprb_base, 2);
	}
	if (PMD3_MERR_RD(pcphpreg)) {
		/* PMD3 memory error interrupt */
		dev_err(&ctx->pdev->dev, "PMD3 memory error\n");
		xgene_pmd_error_report(ctx->pcprb_base, 3);
	}
	if (MCU_UNCORR_RD(pcphpreg)) {
		/* MCUA0 uncorrectable error interrupt */
		printk(KERN_EMERG "MCU uncorrectable memory error\n");
		#define MCU_REG_MCUESRR0 (0xc5U<<2)
		#define MCU_REG_MCUGESR  (0x45U<<2)
		#define MCU_REG_MCUFIFOESR  (0x47U<<2)
		#define MCU_ESRR0_MULTUCERR_RD(src)			(((src) & 0x00000008)>>3)
		#define MCU_ESRR0_BACKUCERR_RD(src)			(((src) & 0x00000004)>>2)
		#define MCU_ESRR0_DEMANDUCERR_RD(src)			(((src) & 0x00000002)>>1)

		/* Detect rank error Clear uncorrectable error interrupt */
		for (mcu = 0; mcu < 4; mcu++ ) {
			/* Skip inactive MCUs */
			if (!((ctx->active_mcu >> mcu) & 0x1))
				continue;
			for (ddr_rank = 0; ddr_rank < 8; ddr_rank++){
				mcubase = ctx->pcprb_base + PCP_RB_MCU0A_PAGE + mcu*0x40000;
				reg = readl(mcubase + ddr_rank*0x40 + MCU_REG_MCUESRR0);
				if (MCU_ESRR0_DEMANDUCERR_RD(reg) || MCU_ESRR0_BACKUCERR_RD(reg)){
					/* Clear interrupt */
					writel(reg, mcubase + ddr_rank*0x40 + MCU_REG_MCUESRR0);
					printk(KERN_EMERG "UECC error occurs at "
							"MCU%d, rank%d\n", mcu, ddr_rank);
				}
			}
		}
		/* Hang the system */
		panic("### DDR Memory ERROR ###");
	}
	if (L3C_UNCORR_RD(pcphpreg) || L3C_CORR_RD(pcplpreg)) {
		u32	l3cesr;
		u32	l3celr;
		u32	l3caelr;
		u32	l3cbelr;

		#define L3C_ESR_DATATAG(src)	((src & 0x200) >> 9)
		#define L3C_ESR_MULTIHIT(src)	((src & 0x100) >> 8)
		#define L3C_ESR_UCEVICT(src)	((src & 0x40) >> 6)
		#define L3C_ESR_MULTIUCERR(src)	((src & 0x20) >> 5)
		#define L3C_ESR_UCERRINTR(src)	((src & 0x2) >> 1)
		#define L3C_ELR_ERRSYN(src)	((src & 0xFF800000) >> 23)
		#define L3C_ELR_ERRWAY(src)	((src & 0x007E0000) >> 17)
		#define L3C_ELR_AGENTID(src)	((src & 0x0001E000) >> 13)
		#define L3C_ELR_ERRGRP(src)	((src & 0x00000F00) >> 8)
		#define L3C_ELR_OPTYPE(src)	((src & 0x000000F0) >> 4)
		#define L3C_ELR_PADDRHIGH(src)	(src & 0x0000000F)
		#define L3C_BELR_BANK(src)	(src & 0x0000000F)

		/* L3C uncorrectable error interrupt */
		if (L3C_UNCORR_RD(pcphpreg))
			printk(KERN_EMERG "L3C uncorrectable error\n");
		if (L3C_CORR_RD(pcplpreg))
			printk(KERN_ERR "L3C correctable error\n");

		l3cesr = readl(ctx->l3c_base + L3C_ESR_ADDR);
		l3celr = readl(ctx->l3c_base + L3C_ELR_ADDR);
		l3caelr = readl(ctx->l3c_base + L3C_AELR_ADDR);
		l3cbelr = readl(ctx->l3c_base + L3C_BELR_ADDR);

		if (L3C_ESR_MULTIHIT(l3cesr)) {
			printk(KERN_ERR "L3C Multiple hit error\n");
		}
		if (L3C_ESR_UCEVICT(l3cesr)) {
			printk(KERN_ERR "L3C Dropped eviction of "
				"line with error\n");
		}
		if (L3C_ESR_MULTIUCERR(l3cesr)) {
			printk(KERN_ERR "L3C Multiple "
				"uncorrectable error\n");
		}

		if (L3C_ESR_DATATAG(l3cesr)) {	/* Data error */
			printk(KERN_ERR "L3C Error Log: "
				"Data Error syndrome=0x%x "
				"Error group=0x%x\n",
				L3C_ELR_ERRSYN(l3celr),L3C_ELR_ERRGRP(l3celr));
		} else {	/* Tag error */
			printk(KERN_ERR "L3C Error Log: "
				"Tag Error syndrome=0x%x "
				"Way of Tag=0x%x "
				"Agent ID=0x%x "
				"Operation type=0x%x\n",
				L3C_ELR_ERRSYN(l3celr),L3C_ELR_ERRWAY(l3celr),
				L3C_ELR_AGENTID(l3celr),L3C_ELR_OPTYPE(l3celr));
		}
		printk(KERN_ERR "L3C Error Log: Phy Address:High[41:38]=0x%x Low[37:6]=0x%x "
				"Bank=0x%x\n",
				L3C_ELR_PADDRHIGH(l3celr), l3caelr,
				L3C_BELR_BANK(l3cbelr));

		printk(KERN_ERR "L3C Error Status Register Value: 0x%x\n", l3cesr);

		/* Clear L3C error interrupt */
		writel(0, ctx->l3c_base + L3C_ESR_ADDR);

	}
	if (IOB_XGIC_ERR_RD(pcphpreg)) {
		u32 xgic_reg;
		u32 iob_memerr;

		/* GIC transaction error interrupt */
		dev_err(&ctx->pdev->dev, "XGIC transaction error\n");
		xgic_reg = readl(ctx->iob_base + XGICTRANSERRINTSTS_ADDR);
		if (RD_ACCESS_ERR_RD(xgic_reg))
			dev_err(&ctx->pdev->dev, "XGIC read size error\n");
		if (M_RD_ACCESS_ERR_RD(xgic_reg))
			dev_err(&ctx->pdev->dev,
				"Multiple XGIC read size error\n");
		if (WR_ACCESS_ERR_RD(xgic_reg))
			dev_err(&ctx->pdev->dev, "XGIC write size error\n");
		if (M_WR_ACCESS_ERR_RD(xgic_reg))
			dev_err(&ctx->pdev->dev,
				"Multiple XGIC write size error\n");
		if (xgic_reg) {
			u32 info;
			info = readl(ctx->iob_base + XGICTRANSERRREQINFO_ADDR);
			dev_err(&ctx->pdev->dev, "XGIC %s access "
				"PADDR 0x%08X (0x%08X)\n",
				REQTYPE_F3_RD(info) ? "READ" : "WRITE",
				ERRADDR_RD(info), info);
			writel(xgic_reg,
				ctx->iob_base + XGICTRANSERRINTSTS_ADDR);
		}

		/* IOB memory error */
		iob_memerr = readl(ctx->iob_base + GLBL_ERR_STS_ADDR);
		if (SEC_ERR_RD(iob_memerr)) {
			err_addr_lo = readl(ctx->iob_base +
						GLBL_SEC_ERRL_ADDR);
			err_addr_hi = readl(ctx->iob_base +
						GLBL_SEC_ERRH_ADDR);
			dev_err(&ctx->pdev->dev, "IOB single-bit correctable "
				"memory 0x%08X.%08X error\n",
				err_addr_lo, err_addr_hi);
			writel(err_addr_lo, ctx->iob_base +
						GLBL_SEC_ERRL_ADDR);
			writel(err_addr_hi, ctx->iob_base +
						GLBL_SEC_ERRH_ADDR);
		}
		if (MSEC_ERR_RD(iob_memerr)) {
			err_addr_lo = readl(ctx->iob_base +
						GLBL_MSEC_ERRL_ADDR);
			err_addr_hi = readl(ctx->iob_base +
						GLBL_MSEC_ERRH_ADDR);
			dev_err(&ctx->pdev->dev, "IOB multiple single-bit "
				"correctable memory 0x%08X.%08X error\n",
				err_addr_lo, err_addr_hi);
			writel(err_addr_lo, ctx->iob_base +
						GLBL_MSEC_ERRL_ADDR);
			writel(err_addr_hi, ctx->iob_base +
						GLBL_MSEC_ERRH_ADDR);
		}
		if (DED_ERR_RD(iob_memerr)) {
			err_addr_lo = readl(ctx->iob_base +
						GLBL_DED_ERRL_ADDR);
			err_addr_hi = readl(ctx->iob_base +
						GLBL_DED_ERRH_ADDR);
			dev_err(&ctx->pdev->dev, "IOB double-bit "
				"uncorrectable memory 0x%08X.%08X error\n",
				err_addr_lo, err_addr_hi);
			writel(err_addr_lo, ctx->iob_base +
						GLBL_DED_ERRL_ADDR);
			writel(err_addr_hi, ctx->iob_base +
						GLBL_DED_ERRH_ADDR);
		}
		if (MDED_ERR_RD(iob_memerr)) {
			err_addr_lo = readl(ctx->iob_base +
						GLBL_MDED_ERRL_ADDR);
			err_addr_hi = readl(ctx->iob_base +
						GLBL_MDED_ERRH_ADDR);
			dev_err(&ctx->pdev->dev, "Multiple IOB double-bit "
				"uncorrectable memory 0x%08X.%08X error\n",
				err_addr_lo, err_addr_hi);
			writel(err_addr_lo, ctx->iob_base +
						GLBL_MDED_ERRL_ADDR);
			writel(err_addr_hi, ctx->iob_base +
						GLBL_MDED_ERRH_ADDR);
		}
	}
	if (IOB_RB_ERR_RD(pcphpreg) || IOB_BA_ERR_RD(pcphpreg)) {
		u32 ba_reg;

		/* IOB Bridge agent transaction error interrupt */
		dev_err(&ctx->pdev->dev,
			"IOB bridge agent (BA) transaction error\n");
		ba_reg = readl(ctx->iob_base + IOBBATRANSERRINTSTS_ADDR);
		if (WRERR_RESP_RD(ba_reg))
			dev_err(&ctx->pdev->dev,
				"IOB BA write response error\n");
		if (M_WRERR_RESP_RD(ba_reg))
			dev_err(&ctx->pdev->dev,
				"Multiple IOB BA write response error\n");
		if (XGIC_POISONED_REQ_RD(ba_reg))
			dev_err(&ctx->pdev->dev,
				"IOB BA XGIC poisoned write error\n");
		if (M_XGIC_POISONED_REQ_RD(ba_reg))
			dev_err(&ctx->pdev->dev,
				"Multiple IOB BA XGIC poisoned write error\n");
		if (RBM_POISONED_REQ_RD(ba_reg))
			dev_err(&ctx->pdev->dev,
				"IOB BA RBM poisoned write error\n");
		if (M_RBM_POISONED_REQ_RD(ba_reg))
			dev_err(&ctx->pdev->dev,
				"Multiple IOB BA RBM poisoned write error\n");
		if (WDATA_CORRUPT_RD(ba_reg))
			dev_err(&ctx->pdev->dev, "IOB BA write error\n");
		if (M_WDATA_CORRUPT_RD(ba_reg))
			dev_err(&ctx->pdev->dev,
				"Multiple IOB BA write error\n");
		if (TRANS_CORRUPT_RD(ba_reg))
			dev_err(&ctx->pdev->dev, "IOB BA transaction error\n");
		if (M_TRANS_CORRUPT_RD(ba_reg))
			dev_err(&ctx->pdev->dev,
				"Multiple IOB BA transaction error\n");
		if (RIDRAM_CORRUPT_RD(ba_reg))
			dev_err(&ctx->pdev->dev,
				"IOB BA RDIDRAM read transaction ID error\n");
		if (M_RIDRAM_CORRUPT_RD(ba_reg))
			dev_err(&ctx->pdev->dev, "Multiple IOB BA RDIDRAM "
				"read transaction ID error\n");
		if (WIDRAM_CORRUPT_RD(ba_reg))
			dev_err(&ctx->pdev->dev,
				"IOB BA RDIDRAM write transaction ID error\n");
		if (M_WIDRAM_CORRUPT_RD(ba_reg))
			dev_err(&ctx->pdev->dev, "Multiple IOB BA RDIDRAM "
				"write transaction ID error\n");
		if (ILLEGAL_ACCESS_RD(ba_reg))
			dev_err(&ctx->pdev->dev,
				"IOB BA XGIC/RB illegal access error\n");
		if (M_ILLEGAL_ACCESS_RD(ba_reg))
			dev_err(&ctx->pdev->dev, "Multiple IOB BA XGIC/RB "
				"illegal access error\n");
		if (ba_reg) {
			err_addr_lo = readl(ctx->iob_base +
						IOBBATRANSERRREQINFOL_ADDR);
			err_addr_hi = readl(ctx->iob_base +
						IOBBATRANSERRREQINFOH_ADDR);
			dev_err(&ctx->pdev->dev, "IOB BA %s access "
				"PADDR 0x%02X.%08X (0x%08X)\n",
				REQTYPE_F2_RD(err_addr_hi)?"READ":"WRITE",
				ERRADDRH_F2_RD(err_addr_hi), err_addr_lo,
				err_addr_hi);
			if (WRERR_RESP_RD(ba_reg)) {
				u32 val = readl(ctx->iob_base +
						IOBBATRANSERRCSWREQID_ADDR);
				dev_err(&ctx->pdev->dev,
					"IOB BA requestor ID 0x%08X\n", val);
			}
			writel(ba_reg, ctx->iob_base +
					IOBBATRANSERRINTSTS_ADDR);
		}
	}
	if (IOB_PA_ERR_RD(pcphpreg)) {
		u32 pa_reg;

		/* IOB Processing agent transaction error interrupt */
		dev_err(&ctx->pdev->dev,
			"IOB procesing agent (PA) transaction error\n");
		pa_reg = readl(ctx->iob_base + IOBPATRANSERRINTSTS_ADDR);
		if (RDATA_CORRUPT_RD(pa_reg))
			dev_err(&ctx->pdev->dev,
				"IOB PA read data RAM error\n");
		if (M_RDATA_CORRUPT_RD(pa_reg))
			dev_err(&ctx->pdev->dev,
				"Mutilple IOB PA read data RAM error\n");
		if (WDATA_CORRUPT_RD(pa_reg))
			printk(KERN_ERR "IOB PA write data RAM error\n");
		if (M_WDATA_CORRUPT_RD(pa_reg))
			dev_err(&ctx->pdev->dev,
				"Mutilple IOB PA write data RAM error\n");
		if (TRANS_CORRUPT_RD(pa_reg))
			dev_err(&ctx->pdev->dev,
				"IOB PA transaction error\n");
		if (M_TRANS_CORRUPT_RD(pa_reg))
			dev_err(&ctx->pdev->dev,
				"Mutilple IOB PA transaction error\n");
		if (REQIDRAM_CORRUPT_RD(pa_reg))
			dev_err(&ctx->pdev->dev,
				"IOB PA transaction ID RAM error\n");
		if (M_REQIDRAM_CORRUPT_RD(pa_reg))
			dev_err(&ctx->pdev->dev,
				"Multiple IOB PA transaction ID RAM error\n");

		/* IOB AXI Error */
		reg = readl(ctx->iob_base + IOBAXIS0TRANSERRINTSTS_ADDR);
		if (ILLEGAL_ACCESS_RD(reg)) {
			err_addr_lo = readl(ctx->iob_base +
					IOBAXIS0TRANSERRREQINFOL_ADDR);
			err_addr_hi = readl(ctx->iob_base +
					IOBAXIS0TRANSERRREQINFOH_ADDR);
			dev_err(&ctx->pdev->dev,
				"%sAXI slave 0 illegal %s access "
				"PADDR 0x%02X.%08X (0x%08X)\n",
				M_ILLEGAL_ACCESS_RD(reg) ? "Multiple " : "",
				REQTYPE_RD(err_addr_hi) ? "READ" : "WRITE",
				ERRADDRH_RD(err_addr_hi), err_addr_lo,
				err_addr_hi);
			writel(0xFFFFFFFF, ctx->iob_base +
						IOBAXIS0TRANSERRINTSTS_ADDR);
		}
		reg = readl(ctx->iob_base + IOBAXIS1TRANSERRINTSTS_ADDR);
		if (ILLEGAL_ACCESS_RD(reg)) {
			err_addr_lo = readl(ctx->iob_base +
						IOBAXIS1TRANSERRREQINFOL_ADDR);
			err_addr_hi = readl(ctx->iob_base +
						IOBAXIS1TRANSERRREQINFOH_ADDR);
			dev_err(&ctx->pdev->dev,
				"%sAXI slave 1 illegal %s access"
				"PADDR 0x%02X.%08X (0x%08X)\n",
				M_ILLEGAL_ACCESS_RD(reg) ? "Multiple " : "",
				REQTYPE_RD(err_addr_hi) ? "READ" : "WRITE",
				ERRADDRH_RD(err_addr_hi), err_addr_lo,
				err_addr_hi);
			writel(0xFFFFFFFF, ctx->iob_base +
						IOBAXIS1TRANSERRINTSTS_ADDR);
		}

		writel(pa_reg, ctx->iob_base + IOBPATRANSERRINTSTS_ADDR);
	}
	if (MCU_CTL_ERR_RD(pcphpreg)) {
		/* MCU (DDR) Control error interrupt */
		dev_err(&ctx->pdev->dev, "MCU control error\n");

		/* Detect rank error Clear uncorrectable error interrupt */
		for (mcu = 0; mcu < 4; mcu++ ) {
			/* Skip inactive MCUs */
			if (!((ctx->active_mcu >> mcu) & 0x1))
				continue;

			mcubase = ctx->pcprb_base + PCP_RB_MCU0A_PAGE + mcu*0x40000;
			reg = readl(mcubase + MCU_REG_MCUGESR);
			if (reg & 0x80)
				dev_err(&ctx->pdev->dev, " MCU%d Address miss match error\n", mcu);
			if (reg & 0x40)
				dev_err(&ctx->pdev->dev, " MCU%d Address multi match error\n", mcu);
			if (reg & 0x8)
				dev_err(&ctx->pdev->dev, " MCU%d Phy parity error\n", mcu);

			reg &= 0xc8;
			writel(reg, mcubase + MCU_REG_MCUGESR);
		}
	}

	/* Retrieve top level interrupt status from IRQ33 */
	if (MCU_CORR_RD(pcplpreg)) {
		/* MCU (DDR) correctable error interrupt */
		printk(KERN_WARNING "MCU correctable error\n");
		#define MCU_REG_MCUESRR0 (0xc5U<<2)
		#define MCU_ESRR0_CERR_RD(src)				(((src) & 0x00000001)>>0)
		#define MCU_REG_MCUEBLRR0 (0xc7U<<2)
		#define MCU_EBLRR_ERRBANK_RD(src)				(((src) & 0x00000007)>>0)
		#define MCU_REG_MCUERCRR0 (0xc8U<<2)
		#define MCU_ERCRR_ERRROW_RD(src)				(((src) & 0xFFFF0000)>>16)
		#define MCU_ERCRR_ERRCOL_RD(src)				(((src) & 0x00000FFF)>>0)
		#define MCU_REG_MCUSBECNT0 (0xc9U<<2)


		/* Detect rank error Clear uncorrectable error interrupt */
		for (mcu = 0; mcu < 4; mcu++ ) {
			/* Skip inactive MCUs */
			if (!((ctx->active_mcu >> mcu) & 0x1))
				continue;

			for (ddr_rank = 0; ddr_rank < 8; ddr_rank++){
				mcubase = ctx->pcprb_base + PCP_RB_MCU0A_PAGE + mcu*0x40000;
				reg = readl(mcubase + ddr_rank*0x40 + MCU_REG_MCUESRR0);

				if (MCU_ESRR0_CERR_RD(reg)) {
					printk(KERN_WARNING "CECC error occurs at "
						"MCU%d, rank%d\n", mcu, ddr_rank);

					reg = readl(mcubase + ddr_rank*0x40 + MCU_REG_MCUEBLRR0);
					printk(KERN_WARNING "\tErr Bank number 0x%x\n", MCU_EBLRR_ERRBANK_RD(reg));
					reg = readl(mcubase + ddr_rank*0x40 + MCU_REG_MCUERCRR0);
					printk(KERN_WARNING "\tErr Column 0x%x - Err Row 0x%x\n",
							MCU_ERCRR_ERRCOL_RD(reg), MCU_ERCRR_ERRROW_RD(reg));
					/* Clear interrupt record */
					writel(0x00000001, mcubase + ddr_rank*0x40 + MCU_REG_MCUESRR0);
					writel(0x0, mcubase + ddr_rank*0x40 + MCU_REG_MCUEBLRR0);
					writel(0x0, mcubase + ddr_rank*0x40 + MCU_REG_MCUERCRR0);
					writel(0x0, mcubase + ddr_rank*0x40 + MCU_REG_MCUSBECNT0);
				}
			}
		}
	}
	if (CSW_SWITCH_TRACE_RD(pcplpreg))
		/* CSW Switch trace interrupt */
		dev_info(&ctx->pdev->dev, "CSW switch correctable error\n");

	/* Retrieve top level interrupt status from IRQ39 */
	reg = readl(ctx->pcp_base + MEMERRINTSTS_ADDR);
	for (i = 0; i < 31; i++) {
		if (reg & (1 << i))
			dev_err(&ctx->pdev->dev, "%s memory error\n",
				mem_err_ip[i]);
	}

        return IRQ_HANDLED;
}

static void xgene_pmd_error_init(void *pmd_base, int cpu)
{
	u32 rcpu_pg_d;
	u32 rcpu_pg_e;
	u32 rcpu_pg_f;

	rcpu_pg_f = cpu * PCP_RB_CPU1_ROM_PAGE + PCP_RB_CPU0_MEMERR_CPU_PAGE;
	/* Clear CPU memory error - MEMERR_CPU_ICFESRA, MEMERR_CPU_LSUESRA,
           MEMERR_CPU_MMUESRA */
	writel(0x00000000, pmd_base + rcpu_pg_f +
		PCP_RB_CPUX_MEMERR_CPU_ICFESRA_PAGE_OFFSET);
	writel(0x00000000, pmd_base + rcpu_pg_f +
		PCP_RB_CPUX_MEMERR_CPU_LSUESRA_PAGE_OFFSET);
	writel(0x00000000, pmd_base + rcpu_pg_f +
		PCP_RB_CPUX_MEMERR_CPU_MMUESRA_PAGE_OFFSET);
	/* Enable CPU memory error - MEMERR_CPU_ICFESRA, MEMERR_CPU_LSUESRA,
           MEMERR_CPU_MMUESRA */
	writel(0x00000301, pmd_base + rcpu_pg_f +
		PCP_RB_CPUX_MEMERR_CPU_ICFECR_PAGE_OFFSET);
	writel(0x00000301, pmd_base + rcpu_pg_f +
		PCP_RB_CPUX_MEMERR_CPU_LSUECR_PAGE_OFFSET);
	writel(0x00000101, pmd_base + rcpu_pg_f +
		PCP_RB_CPUX_MEMERR_CPU_MMUECR_PAGE_OFFSET);

	/* This is per pair of CPU. Do it only for the first core of that
           PMD */
	if ((cpu % 2) == 0) {
		rcpu_pg_e = cpu * PCP_RB_CPU1_ROM_PAGE +
				PCP_RB_CPU0_MEMERR_L2C_PAGE;
		rcpu_pg_d = cpu * PCP_RB_CPU1_ROM_PAGE + PCP_RB_CPU0_L2C_PAGE;
		/* Clear PMD memory error - MEMERR_L2C_L2ESRA,
		   MEMERR_L2C_L2EALR, MEMERR_L2C_L2EAHR,
		   L2C_L2RTOSR, L2C_L2RTOALR, L2C_L2RTOAHR */
		writel(0x00000000, pmd_base + rcpu_pg_e +
			PCP_RB_CPUX_MEMERR_L2C_L2ESRA_PAGE_OFFSET);
		writel(0x00000000, pmd_base + rcpu_pg_e +
			PCP_RB_CPUX_MEMERR_L2C_L2EALR_PAGE_OFFSET);
		writel(0x00000000, pmd_base + rcpu_pg_e +
			PCP_RB_CPUX_MEMERR_L2C_L2EAHR_PAGE_OFFSET);
		writel(0x00000000, pmd_base + rcpu_pg_d +
			PCP_RB_CPUX_L2C_L2RTOSR_PAGE_OFFSET);
		writel(0x00000000, pmd_base + rcpu_pg_d +
			PCP_RB_CPUX_L2C_L2RTOALR_PAGE_OFFSET);
		writel(0x00000000, pmd_base + rcpu_pg_d +
			PCP_RB_CPUX_L2C_L2RTOAHR_PAGE_OFFSET);

		/* Enable PMD memory error - MEMERR_L2C_L2ECR,
		   L2C_L2RTOCR */
		writel(0x00000703, pmd_base + rcpu_pg_e +
			PCP_RB_CPUX_MEMERR_L2C_L2ECR_PAGE_OFFSET);
		writel(0x00000119, pmd_base + rcpu_pg_d +
			PCP_RB_CPUX_L2C_L2RTOCR_PAGE_OFFSET);
	}
}

static void xgene_pcp_hw_init(struct pcp_context *pcp_ctx)
{
	int cpu;

	/* Enable error interrupt for PCP */
	writel(0x0000000, pcp_ctx->pcp_base + PCPHPERRINTMSK_ADDR);

	/* Enable correctable error */
	//writel(0x00000004, pcp_ctx->pcp_base + PCPLPERRINTMSK_ADDR);

	writel(0x00000000, pcp_ctx->pcp_base + MEMERRINTMSK_ADDR);

	/* Enable error interrupt for IOB */
	writel(0x00000000, pcp_ctx->iob_base + IOBAXIS0TRANSERRINTMSK_ADDR);
	writel(0x00000000, pcp_ctx->iob_base + IOBAXIS1TRANSERRINTMSK_ADDR);
 	writel(0x00000000, pcp_ctx->iob_base + XGICTRANSERRINTMSK_ADDR);
 	writel(0xffffffff, pcp_ctx->iob_base + GLBL_DED_ERRLMASK_ADDR);
 	writel(0xffffffff, pcp_ctx->iob_base + GLBL_DED_ERRHMASK_ADDR);
 	writel(0xffffffff, pcp_ctx->iob_base + GLBL_MDED_ERRLMASK_ADDR);
 	writel(0xffffffff, pcp_ctx->iob_base + GLBL_MDED_ERRHMASK_ADDR);

	/* Enable interrupt for L3C */
	writel(0xF, pcp_ctx->l3c_base + L3C_ECR_ADDR);

	for_each_possible_cpu(cpu) {
		/* Enable PMD/CPU error for possible CPU */
		xgene_pmd_error_init(pcp_ctx->pcprb_base, cpu);
	}
}

static const struct file_operations disable_ce_fops = {
	.owner = THIS_MODULE,
	.read = disable_ce_read,
	.write = disable_ce_write,
};

static const struct file_operations ce_threshold_fops = {
        .owner = THIS_MODULE,
	.read = ce_threshold_read,
	.write = ce_threshold_write,
};

int  xgene_pcp_core_init(struct platform_device *pdev,
		struct resource *res_pcp, struct resource *res_iob,
		struct resource *res_pcprb, struct resource *res_l3c,
		u32 pcp_irq0, u32 pcp_irq1, u32 mem_irq)
{
	struct pcp_context *pcp_ctx;
	int rc = 0;
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *proc_dir;
	struct proc_dir_entry *pe;
#endif

	pcp_ctx = kzalloc(sizeof(*pcp_ctx), GFP_KERNEL);
	if (pcp_ctx == NULL) {
		dev_err(&pdev->dev, "no memory for PCP context\n");
		return -ENOMEM;
	}
	platform_set_drvdata(pdev, pcp_ctx);
	pcp_ctx->pdev = pdev;
	pcp_ctx->pcp_irq[0] = 0;
	pcp_ctx->pcp_irq[1] = 0;
	pcp_ctx->mem_irq = 0;

	pcp_ctx->pcp_base = ioremap(res_pcp->start,
				res_pcp->end - res_pcp->start + 1);
	if (pcp_ctx->pcp_base == NULL) {
		dev_err(&pdev->dev, "Unable to map register for PCP\n");
		rc = -ENOMEM;
		goto err;
	}
	pcp_ctx->iob_base = ioremap(res_iob->start,
				res_iob->end - res_iob->start + 1);
	if (pcp_ctx->iob_base == NULL) {
		dev_err(&pdev->dev, "Unable to map register for PCP\n");
		rc = -ENOMEM;
		goto err;
	}

	pcp_ctx->pcprb_base = ioremap(res_pcprb->start,
				res_pcprb->end - res_pcprb->start + 1);
	if (pcp_ctx->pcprb_base == NULL) {
		dev_err(&pdev->dev,
			"Unable to map register for PCP RB base\n");
		rc = -ENOMEM;
		goto err;
	}

	pcp_ctx->l3c_base = ioremap(res_l3c->start,
				res_l3c->end - res_l3c->start + 1);
	if (pcp_ctx->l3c_base == NULL) {
		dev_err(&pdev->dev,
			"Unable to map register for L3C base\n");
		rc = -ENOMEM;
		goto err;
	}

	rc = request_irq(pcp_irq0, xgene_pcp_irq, 0, "PCPHP Error",
			pcp_ctx);
	if (rc != 0) {
		dev_err(&pdev->dev, "Unable to register IRQ %d\n", pcp_irq0);
		goto err;
	}
	pcp_ctx->pcp_irq[0] = pcp_irq0;

	rc = request_irq(pcp_irq1, xgene_pcp_irq, 0, "PCPLP Error",
			pcp_ctx);
	if (rc != 0) {
		dev_err(&pdev->dev, "Unable to register IRQ %d\n", pcp_irq1);
		goto err;
	}
	pcp_ctx->pcp_irq[1] = pcp_irq1;

	/* Due to errata on APM88xxxx, the RAM ecc errors are in-correctly
           reported. Therefore, we don't make use of it. */
#if 0
	rc = request_irq(mem_irq, xgene_pcp_irq, 0, "RAM ECC error",
			pcp_ctx);
	if (rc != 0) {
		dev_err(&pdev->dev, "Unable to register IRQ %d\n", mem_irq);
		goto err;
	}
	pcp_ctx->mem_irq = mem_irq;
#endif

	/* Check MCU configuation */
	detect_mcu_cfg(pcp_ctx);

	/* Initialize the HW */
	xgene_pcp_hw_init(pcp_ctx);

#ifdef CONFIG_PROC_FS
	proc_dir = proc_mkdir("errctl", NULL);

	if (proc_dir == NULL)
		goto err;
	if ((pe = proc_create_data("disable_ce", 0, proc_dir, &disable_ce_fops, pcp_ctx)) == NULL)
		goto err;
	if ((pe = proc_create_data("ce_threshold", 0, proc_dir, &ce_threshold_fops, pcp_ctx)) == NULL)
		goto err;
	pcp_ctx->disable_ce = 0;
	pcp_ctx->ce_threshold = 1;
#endif

	dev_info(&pdev->dev, "APM PCP/IOB driver v%s\n", PCP_DRIVER_VER);
	return 0;

err:
	if (pcp_ctx->pcp_irq[0] != 0)
		free_irq(pcp_ctx->pcp_irq[0], pcp_ctx);
	if (pcp_ctx->pcp_irq[1] != 0)
		free_irq(pcp_ctx->pcp_irq[1], pcp_ctx);
	if (pcp_ctx->mem_irq != 0)
		free_irq(pcp_ctx->mem_irq, pcp_ctx);
	if (pcp_ctx->pcp_base)
		iounmap(pcp_ctx->pcp_base);
	if (pcp_ctx->iob_base)
		iounmap(pcp_ctx->iob_base);
	if (pcp_ctx->pcprb_base)
		iounmap(pcp_ctx->pcprb_base);
	if (pcp_ctx->l3c_base)
		iounmap(pcp_ctx->l3c_base);

	platform_set_drvdata(pdev, NULL);
	kfree(pcp_ctx);

	return rc;
}

static int xgene_pcp_probe(struct platform_device *pdev)
{
	struct resource res_pcp;
	struct resource res_iob;
	struct resource res_pcprb;
	struct resource res_l3c;
	struct resource *regs;
	u32 pcp_irq[2];
	u32 mem_irq;

	/* When both ACPI and DTS are enabled, custom ACPI built-in ACPI
	 * table, and booting via DTS, we need to skip the probe of the
	 * built-in ACPI table probe. */
	if (!efi_enabled(EFI_BOOT) && pdev->dev.of_node == NULL)
		return -ENODEV;

	/* Retrieve the resource */
        regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (regs == NULL) {
		dev_err(&pdev->dev, "no PCP resource\n");
		return -ENODEV;
	}
	res_pcp = *regs;

        regs = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (regs == NULL) {
		dev_err(&pdev->dev, "no IOB resource\n");
		return -ENODEV;
	}
	res_iob = *regs;

        regs = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	if (regs == NULL) {
		dev_err(&pdev->dev, "no PCP RB resource\n");
		return -ENODEV;
	}
	res_pcprb = *regs;

        regs = platform_get_resource(pdev, IORESOURCE_MEM, 4);
	if (regs == NULL) {
		dev_err(&pdev->dev, "no L3C resource\n");
		return -ENODEV;
	}
	res_l3c = *regs;

	/* Retrieve IRQs */
	pcp_irq[0] = platform_get_irq(pdev, 0);
       	if (pcp_irq[0] == 0) {
		dev_err(&pdev->dev, "no PCP IRQ\n");
		return -ENODEV;
	}
	pcp_irq[1] = platform_get_irq(pdev, 1);
       	if (pcp_irq[1] == 0) {
		dev_err(&pdev->dev, "no PCP IRQ\n");
		return -ENODEV;
	}
	mem_irq = platform_get_irq(pdev, 2);
       	if (mem_irq == 0) {
		dev_err(&pdev->dev, "no memory IRQ\n");
		return -ENODEV;
	}

	return xgene_pcp_core_init(pdev, &res_pcp, &res_iob, &res_pcprb,
			&res_l3c, pcp_irq[0], pcp_irq[1], mem_irq);
}

static int xgene_pcp_remove(struct platform_device *pdev)
{
	struct pcp_context *pcp_ctx = platform_get_drvdata(pdev);

	if (pcp_ctx->pcp_irq[0] != 0)
		free_irq(pcp_ctx->pcp_irq[0], pcp_ctx);
	if (pcp_ctx->pcp_irq[1] != 0)
		free_irq(pcp_ctx->pcp_irq[1], pcp_ctx);
	if (pcp_ctx->mem_irq != 0)
		free_irq(pcp_ctx->mem_irq, pcp_ctx);
	if (pcp_ctx->pcp_base)
		iounmap(pcp_ctx->pcp_base);
	if (pcp_ctx->iob_base)
		iounmap(pcp_ctx->iob_base);
	if (pcp_ctx->pcprb_base)
		iounmap(pcp_ctx->pcprb_base);

	platform_set_drvdata(pdev, NULL);
	kfree(pcp_ctx);
	return 0;
}

#ifdef CONFIG_PM
static int xgene_pcp_suspend(struct platform_device *pdev,
				pm_message_t state)
{
	return 0;
}

static int xgene_pcp_resume(struct platform_device *pdev)
{
	struct pcp_context *pcp_ctx = platform_get_drvdata(pdev);

	dev_dbg(&pdev->dev, "PCP resume\n");
	if (pdev->dev.power.power_state.event == PM_EVENT_THAW)
        	return 0;

	/* Re-Initialize the hardware */
	xgene_pcp_hw_init(pcp_ctx);
	return 0;
}

#else

#define xgene_pcp_suspend NULL
#define xgene_pcp_resume  NULL

#endif /* CONFIG_PM */

static const struct of_device_id xgene_pcp_match[] = {
        { .compatible   = "apm,xgene-pcp", },
	{},
};
MODULE_DEVICE_TABLE(of, xgene_pcp_match);

static const struct acpi_device_id xgene_pcp_acpi_ids[] = {
        { "APMC0D02", 0 },
        { }
};
MODULE_DEVICE_TABLE(acpi, xgene_pcp_acpi_ids);

static struct platform_driver pcp_driver = {
	.probe = xgene_pcp_probe,
	.remove = xgene_pcp_remove,
#if defined(CONFIG_PM)
	.suspend = xgene_pcp_suspend,
	.resume = xgene_pcp_resume,
#endif
	.driver = {
		.name = "xgene-pcp",
		.owner = THIS_MODULE,
		.of_match_table = xgene_pcp_match,
		.acpi_match_table = ACPI_PTR(xgene_pcp_acpi_ids),
	},
};


static int __init xgene_pcp_init(void)
{
	return platform_driver_register(&pcp_driver);
}
arch_initcall(xgene_pcp_init);

static void __exit xgene_pcp_exit(void)
{
	platform_driver_unregister(&pcp_driver);
}
module_exit(xgene_pcp_exit);

MODULE_AUTHOR("Loc Ho <lho@apm.com>");
MODULE_DESCRIPTION("APM88xxxx PCP bridge driver");
MODULE_LICENSE("GPL");
