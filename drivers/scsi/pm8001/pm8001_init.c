/*
 * PMC-Sierra SPC 8001/8008/8009/8018/8019 SAS/SATA based host adapters driver
 *
 * Copyright (c) 2008-2009 USI Co., Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 *
 */

#include <linux/slab.h>
#include "pm8001_sas.h"
#include "pm8001_chips.h"

static struct scsi_transport_template *pm8001_stt;
int pm80xx_major = -1;
/** 
 * chip info structure to identify chip key functionality as 
 * encryption available/not, no of ports, hw specific function ref 
 */
static const struct pm8001_chip_info pm8001_chips[] = {
	[chip_8001] = {0,  8, &pm8001_8001_dispatch,},
	[chip_8008] = {0,  8, &pm8001_80xx_dispatch,},
	[chip_8009] = {1,  8, &pm8001_80xx_dispatch,},
	[chip_8018] = {0,  16, &pm8001_80xx_dispatch,},
	[chip_8019] = {1,  16, &pm8001_80xx_dispatch,},
        [chip_8074] = {0,  16, &pm8001_80xx_dispatch,},
        [chip_8076] = {0,  16, &pm8001_80xx_dispatch,}, 
        [chip_8077] = {0,  16, &pm8001_80xx_dispatch,},   
};
static int pm8001_id;

LIST_HEAD(hba_list);

struct workqueue_struct *pm8001_wq;

/**
 * The main structure which LLDD must register for scsi core.
 * spc specific structure
 */
static struct scsi_host_template pm8001_sht = {
	.module			= THIS_MODULE,
	.name			= DRV_NAME,
	.queuecommand		= sas_queuecommand,
	.target_alloc		= sas_target_alloc,
	.slave_configure	= sas_slave_configure,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 1) && \
	!defined (PM8001_RHEL63) && LINUX_VERSION_CODE != KERNEL_VERSION(3, 0, 76))
	.slave_destroy		= sas_slave_destroy,
#endif
	.scan_finished		= pm8001_scan_finished,
	.scan_start		= pm8001_scan_start,
	.change_queue_depth	= sas_change_queue_depth,
	.change_queue_type	= sas_change_queue_type,
	.bios_param		= sas_bios_param,
	.can_queue		= 1,
	.cmd_per_lun		= 1,
	.this_id		= -1,
	.sg_tablesize		= SG_ALL,
	.max_sectors		= SCSI_DEFAULT_MAX_SECTORS,
	.use_clustering		= ENABLE_CLUSTERING,
	.eh_device_reset_handler = sas_eh_device_reset_handler,
	.eh_bus_reset_handler	= sas_eh_bus_reset_handler,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 1) && \
	!defined (PM8001_RHEL63) && LINUX_VERSION_CODE != KERNEL_VERSION(3, 0, 76))
	.slave_alloc		= sas_slave_alloc,
#endif
	.target_destroy		= sas_target_destroy,
	.ioctl			= sas_ioctl,
	.shost_attrs		= pm8001_host_attrs,
};

/**
 * Sas layer call this function to execute specific task.
 */
static struct sas_domain_function_template pm8001_transport_ops = {
	.lldd_dev_found		= pm8001_dev_found,
	.lldd_dev_gone		= pm8001_dev_gone,

	.lldd_execute_task	= pm8001_queue_command,
	.lldd_control_phy	= pm8001_phy_control,

	.lldd_abort_task	= pm8001_abort_task,
	.lldd_abort_task_set	= pm8001_abort_task_set,
	.lldd_clear_aca		= pm8001_clear_aca,
	.lldd_clear_task_set	= pm8001_clear_task_set,
	.lldd_I_T_nexus_reset   = pm8001_I_T_nexus_reset,
	.lldd_lu_reset		= pm8001_lu_reset,
	.lldd_query_task	= pm8001_query_task,
};

/**
 * pm8001_interrupt_handler_x - main interrupt handler invokde for all interrupt.
 * It obtains the vector number and calls the equivalent bottom half or services directly.
 * @vec: vector number; will be 0 for none msi-x operation
 * @opaque: the passed general host adapter struct
 */

static inline irqreturn_t pm8001_interrupt_handler_x(int vec, void *opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	irqreturn_t ret = IRQ_HANDLED;
	struct sas_ha_struct *sha = opaque;
	pm8001_ha = sha->lldd_ha;
	if (unlikely(!pm8001_ha))
		return IRQ_NONE;
	if (!PM8001_CHIP_DISP->is_our_interupt(pm8001_ha))
		return IRQ_NONE;

#ifdef PM8001_USE_TASKLET
	tasklet_schedule(&pm8001_ha->tasklet[vec]);
#else
	ret = PM8001_CHIP_DISP->isr(pm8001_ha, 0);
#endif
	return ret;
}

/* 64 interrupt handlers for 64 msi-x vectors */
static irqreturn_t pm8001_interrupt_handler0(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(0, dev_id);
}

static irqreturn_t pm8001_interrupt_handler1(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(1, dev_id);
}

static irqreturn_t pm8001_interrupt_handler2(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(2, dev_id);
}

static irqreturn_t pm8001_interrupt_handler3(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(3, dev_id);
}

static irqreturn_t pm8001_interrupt_handler4(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(4, dev_id);
}

static irqreturn_t pm8001_interrupt_handler5(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(5, dev_id);
}

static irqreturn_t pm8001_interrupt_handler6(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(6, dev_id);
}

static irqreturn_t pm8001_interrupt_handler7(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(7, dev_id);
}

static irqreturn_t pm8001_interrupt_handler8(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(8, dev_id);
}

static irqreturn_t pm8001_interrupt_handler9(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(9, dev_id);
}

static irqreturn_t pm8001_interrupt_handler10(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(10, dev_id);
}

static irqreturn_t pm8001_interrupt_handler11(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(11, dev_id);
}

static irqreturn_t pm8001_interrupt_handler12(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(12, dev_id);
}

static irqreturn_t pm8001_interrupt_handler13(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(13, dev_id);
}

static irqreturn_t pm8001_interrupt_handler14(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(14, dev_id);
}

static irqreturn_t pm8001_interrupt_handler15(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(15, dev_id);
}

static irqreturn_t pm8001_interrupt_handler16(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(16, dev_id);
}

static irqreturn_t pm8001_interrupt_handler17(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(17, dev_id);
}

static irqreturn_t pm8001_interrupt_handler18(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(18, dev_id);
}

static irqreturn_t pm8001_interrupt_handler19(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(19, dev_id);
}

static irqreturn_t pm8001_interrupt_handler20(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(20, dev_id);
}

static irqreturn_t pm8001_interrupt_handler21(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(21, dev_id);
}

static irqreturn_t pm8001_interrupt_handler22(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(22, dev_id);
}

static irqreturn_t pm8001_interrupt_handler23(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(23, dev_id);
}

static irqreturn_t pm8001_interrupt_handler24(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(24, dev_id);
}

static irqreturn_t pm8001_interrupt_handler25(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(25, dev_id);
}

static irqreturn_t pm8001_interrupt_handler26(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(26, dev_id);
}

static irqreturn_t pm8001_interrupt_handler27(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(27, dev_id);
}

static irqreturn_t pm8001_interrupt_handler28(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(28, dev_id);
}

static irqreturn_t pm8001_interrupt_handler29(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(29, dev_id);
}

static irqreturn_t pm8001_interrupt_handler30(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(30, dev_id);
}

static irqreturn_t pm8001_interrupt_handler31(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(31, dev_id);
}

static irqreturn_t pm8001_interrupt_handler32(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(32, dev_id);
}

static irqreturn_t pm8001_interrupt_handler33(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(33, dev_id);
}

static irqreturn_t pm8001_interrupt_handler34(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(34, dev_id);
}

static irqreturn_t pm8001_interrupt_handler35(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(35, dev_id);
}

static irqreturn_t pm8001_interrupt_handler36(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(36, dev_id);
}

static irqreturn_t pm8001_interrupt_handler37(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(37, dev_id);
}

static irqreturn_t pm8001_interrupt_handler38(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(38, dev_id);
}

static irqreturn_t pm8001_interrupt_handler39(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(39, dev_id);
}

static irqreturn_t pm8001_interrupt_handler40(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(40, dev_id);
}

static irqreturn_t pm8001_interrupt_handler41(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(41, dev_id);
}

static irqreturn_t pm8001_interrupt_handler42(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(42, dev_id);
}

static irqreturn_t pm8001_interrupt_handler43(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(43, dev_id);
}

static irqreturn_t pm8001_interrupt_handler44(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(44, dev_id);
}

static irqreturn_t pm8001_interrupt_handler45(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(45, dev_id);
}

static irqreturn_t pm8001_interrupt_handler46(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(46, dev_id);
}

static irqreturn_t pm8001_interrupt_handler47(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(47, dev_id);
}

static irqreturn_t pm8001_interrupt_handler48(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(48, dev_id);
}

static irqreturn_t pm8001_interrupt_handler49(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(49, dev_id);
}

static irqreturn_t pm8001_interrupt_handler50(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(50, dev_id);
}

static irqreturn_t pm8001_interrupt_handler51(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(51, dev_id);
}

static irqreturn_t pm8001_interrupt_handler52(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(52, dev_id);
}

static irqreturn_t pm8001_interrupt_handler53(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(53, dev_id);
}

static irqreturn_t pm8001_interrupt_handler54(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(54, dev_id);
}

static irqreturn_t pm8001_interrupt_handler55(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(55, dev_id);
}

static irqreturn_t pm8001_interrupt_handler56(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(56, dev_id);
}

static irqreturn_t pm8001_interrupt_handler57(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(57, dev_id);
}

static irqreturn_t pm8001_interrupt_handler58(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(58, dev_id);
}

static irqreturn_t pm8001_interrupt_handler59(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(59, dev_id);
}

static irqreturn_t pm8001_interrupt_handler60(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(60, dev_id);
}

static irqreturn_t pm8001_interrupt_handler61(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(61, dev_id);
}

static irqreturn_t pm8001_interrupt_handler62(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(62, dev_id);
}

static irqreturn_t pm8001_interrupt_handler63(int irq, void *dev_id)
{
	return pm8001_interrupt_handler_x(63, dev_id);
}

/* array of 64 interrupt handlers used during request_irq for msi-x interrupts */
/* pm8001_interrupt_handler0 routine is common for pm8001 & pm80xx */
static irqreturn_t (*pm8001_interrupt[PM8001_MAX_MSIX_VEC])(int vec, void *dev_id) =
{
      pm8001_interrupt_handler0, pm8001_interrupt_handler1, pm8001_interrupt_handler2,
      pm8001_interrupt_handler3, pm8001_interrupt_handler4, pm8001_interrupt_handler5,
      pm8001_interrupt_handler6, pm8001_interrupt_handler7, pm8001_interrupt_handler8,
      pm8001_interrupt_handler9, pm8001_interrupt_handler10, pm8001_interrupt_handler11,
      pm8001_interrupt_handler12, pm8001_interrupt_handler13, pm8001_interrupt_handler14,
      pm8001_interrupt_handler15,
      pm8001_interrupt_handler16, pm8001_interrupt_handler17, pm8001_interrupt_handler18,
      pm8001_interrupt_handler19, pm8001_interrupt_handler20, pm8001_interrupt_handler21,
      pm8001_interrupt_handler22, pm8001_interrupt_handler23, pm8001_interrupt_handler24,
      pm8001_interrupt_handler25, pm8001_interrupt_handler26, pm8001_interrupt_handler27,
      pm8001_interrupt_handler28, pm8001_interrupt_handler29, pm8001_interrupt_handler30,
      pm8001_interrupt_handler31,
      pm8001_interrupt_handler32, pm8001_interrupt_handler33, pm8001_interrupt_handler34,
      pm8001_interrupt_handler35, pm8001_interrupt_handler36, pm8001_interrupt_handler37,
      pm8001_interrupt_handler38, pm8001_interrupt_handler39, pm8001_interrupt_handler40,
      pm8001_interrupt_handler41, pm8001_interrupt_handler42, pm8001_interrupt_handler43,
      pm8001_interrupt_handler44, pm8001_interrupt_handler45, pm8001_interrupt_handler46,
      pm8001_interrupt_handler47,
      pm8001_interrupt_handler48, pm8001_interrupt_handler49, pm8001_interrupt_handler50,
      pm8001_interrupt_handler51, pm8001_interrupt_handler52, pm8001_interrupt_handler53,
      pm8001_interrupt_handler54, pm8001_interrupt_handler55, pm8001_interrupt_handler56,
      pm8001_interrupt_handler57, pm8001_interrupt_handler58, pm8001_interrupt_handler59,
      pm8001_interrupt_handler60, pm8001_interrupt_handler61, pm8001_interrupt_handler62,
      pm8001_interrupt_handler63
};

/* array of 8 interrupt handlers used during request_irq for msi interrupts */
static irqreturn_t (*pm8001_interrupt_msi[PM8001_MAX_MSI_VEC])(int vec, void *dev_id) =
{
      pm8001_interrupt_handler0, pm8001_interrupt_handler1, pm8001_interrupt_handler2,
      pm8001_interrupt_handler3, pm8001_interrupt_handler4, pm8001_interrupt_handler5,
      pm8001_interrupt_handler6, pm8001_interrupt_handler7
};


/**
 *pm8001_phy_init - initiate our adapter phys
 *@pm8001_ha: our hba structure.
 *@phy_id: phy id.
 */
static void pm8001_phy_init(struct pm8001_hba_info *pm8001_ha,
	int phy_id)
{
	struct pm8001_phy *phy = &pm8001_ha->phy[phy_id];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;
	phy->phy_state = 0;
	phy->pm8001_ha = pm8001_ha;
	sas_phy->enabled = (phy_id < pm8001_ha->chip->n_phy) ? 1 : 0;
	sas_phy->class = SAS;
	sas_phy->iproto = SAS_PROTOCOL_ALL;
	sas_phy->tproto = 0;
	sas_phy->type = PHY_TYPE_PHYSICAL;
	sas_phy->role = PHY_ROLE_INITIATOR;
	sas_phy->oob_mode = OOB_NOT_CONNECTED;
	sas_phy->linkrate = SAS_LINK_RATE_UNKNOWN;
	sas_phy->id = phy_id;
	sas_phy->sas_addr = &pm8001_ha->sas_addr[0];
	sas_phy->frame_rcvd = &phy->frame_rcvd[0];
	sas_phy->ha = (struct sas_ha_struct *)pm8001_ha->shost->hostdata;
	sas_phy->lldd_phy = phy;
}

/**
 *pm8001_free - free hba
 *@pm8001_ha:	our hba structure.
 *
 */
static void pm8001_free(struct pm8001_hba_info *pm8001_ha)
{
	int i;

	if (!pm8001_ha)
		return;

	del_timer(&pm8001_ha->gpio_timer);

	for (i = 0; i < USI_MAX_MEMCNT; i++) {
		if (pm8001_ha->memoryMap.region[i].virt_ptr != NULL) {
			pci_free_consistent(pm8001_ha->pdev,
				(pm8001_ha->memoryMap.region[i].total_len +
				pm8001_ha->memoryMap.region[i].alignment),
				pm8001_ha->memoryMap.region[i].virt_ptr,
				pm8001_ha->memoryMap.region[i].phys_addr);
			}
	}
	PM8001_CHIP_DISP->chip_iounmap(pm8001_ha);
	if (pm8001_ha->shost)
		scsi_host_put(pm8001_ha->shost);
	flush_workqueue(pm8001_wq);
	kfree(pm8001_ha->tags);
	kfree(pm8001_ha);
}

#ifdef PM8001_USE_TASKLET
/**
 * tasklets for 64 msi-x interrupt handlers 
 * @opaque: the passed general host adapter struct
 * Note: pm8001_tasklet0 is common for pm8001 & pm80xx
 */
static void pm8001_tasklet0(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 0);
}
static void pm8001_tasklet1(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 1);
}
static void pm8001_tasklet2(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 2);
}
static void pm8001_tasklet3(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 3);
}
static void pm8001_tasklet4(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 4);
}
static void pm8001_tasklet5(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 5);
}
static void pm8001_tasklet6(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 6);
}
static void pm8001_tasklet7(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 7);
}
static void pm8001_tasklet8(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 8);
}
static void pm8001_tasklet9(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 9);
}
static void pm8001_tasklet10(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 10);
}
static void pm8001_tasklet11(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 11);
}
static void pm8001_tasklet12(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 12);
}
static void pm8001_tasklet13(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 13);
}
static void pm8001_tasklet14(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 14);
}
static void pm8001_tasklet15(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 15);
}
static void pm8001_tasklet16(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 16);
}
static void pm8001_tasklet17(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 17);
}
static void pm8001_tasklet18(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 18);
}
static void pm8001_tasklet19(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 19);
}
static void pm8001_tasklet20(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 20);
}
static void pm8001_tasklet21(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 21);
}
static void pm8001_tasklet22(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 22);
}
static void pm8001_tasklet23(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 23);
}
static void pm8001_tasklet24(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 24);
}
static void pm8001_tasklet25(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 25);
}
static void pm8001_tasklet26(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 26);
}
static void pm8001_tasklet27(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 27);
}
static void pm8001_tasklet28(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 28);
}
static void pm8001_tasklet29(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 29);
}
static void pm8001_tasklet30(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 30);
}
static void pm8001_tasklet31(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 31);
}
static void pm8001_tasklet32(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 32);
}
static void pm8001_tasklet33(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 33);
}
static void pm8001_tasklet34(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 34);
}
static void pm8001_tasklet35(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 35);
}
static void pm8001_tasklet36(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 36);
}
static void pm8001_tasklet37(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 37);
}
static void pm8001_tasklet38(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 38);
}
static void pm8001_tasklet39(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 39);
}
static void pm8001_tasklet40(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 40);
}
static void pm8001_tasklet41(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 41);
}
static void pm8001_tasklet42(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 42);
}
static void pm8001_tasklet43(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 43);
}
static void pm8001_tasklet44(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 44);
}
static void pm8001_tasklet45(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 45);
}
static void pm8001_tasklet46(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 46);
}
static void pm8001_tasklet47(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 47);
}
static void pm8001_tasklet48(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 48);
}
static void pm8001_tasklet49(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 49);
}
static void pm8001_tasklet50(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 50);
}
static void pm8001_tasklet51(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 51);
}
static void pm8001_tasklet52(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 52);
}
static void pm8001_tasklet53(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 53);
}
static void pm8001_tasklet54(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 54);
}
static void pm8001_tasklet55(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 55);
}
static void pm8001_tasklet56(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 56);
}
static void pm8001_tasklet57(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 57);
}
static void pm8001_tasklet58(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 58);
}
static void pm8001_tasklet59(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 59);
}
static void pm8001_tasklet60(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 60);
}
static void pm8001_tasklet61(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 61);
}
static void pm8001_tasklet62(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 62);
}
static void pm8001_tasklet63(unsigned long opaque)
{
	struct pm8001_hba_info *pm8001_ha;
	pm8001_ha = (struct pm8001_hba_info *)opaque;
	if (unlikely(!pm8001_ha))
		BUG_ON(1);
	PM8001_CHIP_DISP->isr(pm8001_ha, 63);
}
#endif
/**
 * pm8001_alloc - initiate our hba structure and 6 DMAs area.
 * @pm8001_ha:our hba structure.
 *
 */
static int pm8001_alloc(struct pm8001_hba_info *pm8001_ha, const struct pci_device_id *ent)
{
	int i;
	spin_lock_init(&pm8001_ha->lock);
	PM8001_INIT_DBG(pm8001_ha, 
		pm8001_printk ("pm8001_alloc: PHY:%x \n", 
				pm8001_ha->chip->n_phy));
	for (i = 0; i < pm8001_ha->chip->n_phy; i++) {
		pm8001_phy_init(pm8001_ha, i);
		pm8001_ha->port[i].wide_port_phymap = 0;
		pm8001_ha->port[i].port_attached = 0;
		pm8001_ha->port[i].port_state = 0;
		INIT_LIST_HEAD(&pm8001_ha->port[i].list);
	}

	pm8001_ha->tags = kzalloc(PM8001_MAX_CCB, GFP_KERNEL);
	if (!pm8001_ha->tags)
		goto err_out;
	/* MPI Memory region 1 for AAP Event Log for fw */
	pm8001_ha->memoryMap.region[AAP1].num_elements = 1;
	pm8001_ha->memoryMap.region[AAP1].element_size = PM8001_EVENT_LOG_SIZE;
	pm8001_ha->memoryMap.region[AAP1].total_len = PM8001_EVENT_LOG_SIZE;
	pm8001_ha->memoryMap.region[AAP1].alignment = 32;

	/* MPI Memory region 2 for IOP Event Log for fw */
	pm8001_ha->memoryMap.region[IOP].num_elements = 1;
	pm8001_ha->memoryMap.region[IOP].element_size = PM8001_EVENT_LOG_SIZE;
	pm8001_ha->memoryMap.region[IOP].total_len = PM8001_EVENT_LOG_SIZE;
	pm8001_ha->memoryMap.region[IOP].alignment = 32;

	for (i=0; i < PM8001_MAX_INB_NUM; i++)
	{
		/* MPI Memory region 3 for consumer Index of inbound queues */
		pm8001_ha->memoryMap.region[CI+i].num_elements = 1;
		pm8001_ha->memoryMap.region[CI+i].element_size = 4;
		pm8001_ha->memoryMap.region[CI+i].total_len = 4;
		pm8001_ha->memoryMap.region[CI+i].alignment = 4;

    		if ((ent->driver_data) != chip_8001) {
		/* MPI Memory region 5 inbound queues */
			pm8001_ha->memoryMap.region[IB+i].num_elements = MAX_IB_QUEUE_ELEMENTS;
			pm8001_ha->memoryMap.region[IB+i].element_size = 128;
			pm8001_ha->memoryMap.region[IB+i].total_len = MAX_IB_QUEUE_ELEMENTS * 128;
			pm8001_ha->memoryMap.region[IB+i].alignment = 128;
		}
		else {
			pm8001_ha->memoryMap.region[IB+i].num_elements = MAX_IB_QUEUE_ELEMENTS;
			pm8001_ha->memoryMap.region[IB+i].element_size = 64;
			pm8001_ha->memoryMap.region[IB+i].total_len = MAX_IB_QUEUE_ELEMENTS * 64;
			pm8001_ha->memoryMap.region[IB+i].alignment = 64;
		}
	}

	for (i=0; i < PM8001_MAX_OUTB_NUM; i++)
	{
		/* MPI Memory region 4 for producer Index of outbound queues */
		pm8001_ha->memoryMap.region[PI+i].num_elements = 1;
		pm8001_ha->memoryMap.region[PI+i].element_size = 4;
		pm8001_ha->memoryMap.region[PI+i].total_len = 4;
		pm8001_ha->memoryMap.region[PI+i].alignment = 4;

		if (ent->driver_data != chip_8001) {  
			/* MPI Memory region 6 Outbound queues */
			pm8001_ha->memoryMap.region[OB+i].num_elements = MAX_OB_QUEUE_ELEMENTS; 
			pm8001_ha->memoryMap.region[OB+i].element_size = 128;
			pm8001_ha->memoryMap.region[OB+i].total_len = MAX_OB_QUEUE_ELEMENTS * 128;
			pm8001_ha->memoryMap.region[OB+i].alignment = 128;
		}
		else {
			/* MPI Memory region 6 Outbound queues */
			pm8001_ha->memoryMap.region[OB+i].num_elements = MAX_OB_QUEUE_ELEMENTS;
			pm8001_ha->memoryMap.region[OB+i].element_size = 64;
			pm8001_ha->memoryMap.region[OB+i].total_len = MAX_OB_QUEUE_ELEMENTS * 64;
			pm8001_ha->memoryMap.region[OB+i].alignment = 64;
		}

	}
	/* Memory region write DMA*/
	pm8001_ha->memoryMap.region[NVMD].num_elements = 1;
	pm8001_ha->memoryMap.region[NVMD].element_size = 4096;
	pm8001_ha->memoryMap.region[NVMD].total_len = 4096;
	
        /* Memory region for devices*/
	pm8001_ha->memoryMap.region[DEV_MEM].num_elements = 1;
	pm8001_ha->memoryMap.region[DEV_MEM].element_size = PM8001_MAX_DEVICES *
		sizeof(struct pm8001_device);
	pm8001_ha->memoryMap.region[DEV_MEM].total_len = PM8001_MAX_DEVICES *
		sizeof(struct pm8001_device);

	/* Memory region for ccb_info*/
	pm8001_ha->memoryMap.region[CCB_MEM].num_elements = 1;
	pm8001_ha->memoryMap.region[CCB_MEM].element_size = PM8001_MAX_CCB *
		sizeof(struct pm8001_ccb_info);
	pm8001_ha->memoryMap.region[CCB_MEM].total_len = PM8001_MAX_CCB *
		sizeof(struct pm8001_ccb_info);

	/* Memory region for fw flash */
	pm8001_ha->memoryMap.region[FW_FLASH].total_len = 4096;

	pm8001_ha->memoryMap.region[FORENSIC_MEM].num_elements = 1;
	pm8001_ha->memoryMap.region[FORENSIC_MEM].total_len = 0x10000;
	pm8001_ha->memoryMap.region[FORENSIC_MEM].element_size = 0x10000; 
	pm8001_ha->memoryMap.region[FORENSIC_MEM].alignment = 0x10000;
	for (i = 0; i < USI_MAX_MEMCNT; i++) {
		if (pm8001_mem_alloc(pm8001_ha->pdev,
			&pm8001_ha->memoryMap.region[i].virt_ptr,
			&pm8001_ha->memoryMap.region[i].phys_addr,
			&pm8001_ha->memoryMap.region[i].phys_addr_hi,
			&pm8001_ha->memoryMap.region[i].phys_addr_lo,
			pm8001_ha->memoryMap.region[i].total_len,
			pm8001_ha->memoryMap.region[i].alignment) != 0) {
				PM8001_FAIL_DBG(pm8001_ha,
					pm8001_printk("Mem%d alloc failed\n",
					i));
				goto err_out;
		}
	}

	pm8001_ha->devices = pm8001_ha->memoryMap.region[DEV_MEM].virt_ptr;
	for (i = 0; i < PM8001_MAX_DEVICES; i++) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
		pm8001_ha->devices[i].dev_type = SAS_PHY_UNUSED;
#else
		pm8001_ha->devices[i].dev_type = NO_DEVICE;
#endif
		pm8001_ha->devices[i].id = i;
		pm8001_ha->devices[i].device_id = PM8001_MAX_DEVICES;
		pm8001_ha->devices[i].running_req = 0;
	}
	pm8001_ha->ccb_info = pm8001_ha->memoryMap.region[CCB_MEM].virt_ptr;
	for (i = 0; i < PM8001_MAX_CCB; i++) {
		pm8001_ha->ccb_info[i].ccb_dma_handle =
			pm8001_ha->memoryMap.region[CCB_MEM].phys_addr +
			i * sizeof(struct pm8001_ccb_info);
		pm8001_ha->ccb_info[i].task = NULL;
		pm8001_ha->ccb_info[i].ccb_tag = 0xffffffff;
		pm8001_ha->ccb_info[i].device = NULL;
		++pm8001_ha->tags_num;
	}
	pm8001_ha->flags = PM8001F_INIT_TIME;
	/* Initialize tags */
	pm8001_tag_init(pm8001_ha);
	return 0;
err_out:
	return 1;
}

/**
 * pm8001_ioremap - remap the pci high physical address to kernal virtual
 * address so that we can access them.
 * @pm8001_ha:our hba structure.
 */
static int pm8001_ioremap(struct pm8001_hba_info *pm8001_ha)
{
	u32 bar;
	u32 logicalBar = 0;
	struct pci_dev *pdev;

	pdev = pm8001_ha->pdev;
	/* map pci mem (PMC pci base 0-3)*/
	for (bar = 0; bar < 6; bar++) {
		/*
		** logical BARs for SPC:
		** bar 0 and 1 - logical BAR0
		** bar 2 and 3 - logical BAR1
		** bar4 - logical BAR2
		** bar5 - logical BAR3
		** Skip the appropriate assignments:
		*/
		if ((bar == 1) || (bar == 3))
			continue;
		if (pci_resource_flags(pdev, bar) & IORESOURCE_MEM) {
			pm8001_ha->io_mem[logicalBar].membase =
				pci_resource_start(pdev, bar);
			pm8001_ha->io_mem[logicalBar].membase &=
				(u32)PCI_BASE_ADDRESS_MEM_MASK;
			pm8001_ha->io_mem[logicalBar].memsize =
				pci_resource_len(pdev, bar);
			pm8001_ha->io_mem[logicalBar].memvirtaddr =
				ioremap(pci_resource_start(pdev, bar),
				pm8001_ha->io_mem[logicalBar].memsize);
			PM8001_INIT_DBG(pm8001_ha,
				pm8001_printk("PCI: bar %d, logicalBar %d "
				"base addr %lx virt_addr=%lx len=%d\n", bar, logicalBar,
				(unsigned long)pm8001_ha->io_mem[logicalBar].membase,
				(unsigned long)pm8001_ha->io_mem[logicalBar].memvirtaddr,
				pm8001_ha->io_mem[logicalBar].memsize));
		} else {
			pm8001_ha->io_mem[logicalBar].membase	= 0;
			pm8001_ha->io_mem[logicalBar].memsize	= 0;
			pm8001_ha->io_mem[logicalBar].memvirtaddr = 0;
		}
		logicalBar++;
	}
	return 0;
}

void pm8001_gpio_timer_callback(unsigned long data)
{
	struct pm8001_hba_info *pm8001_ha = (struct pm8001_hba_info *)data;

	PM8001_FAIL_DBG(pm8001_ha, pm8001_printk("Timer expired for GPIO response \n"));
	spin_lock(&pm8001_ha->gpio_lock);
	if(pm8001_ha->gpio_completion != NULL)
	{
		pm8001_ha->gpio_timer_expired = 1;
		complete(pm8001_ha->gpio_completion);
		pm8001_ha->gpio_completion = NULL;
	}
	spin_unlock(&pm8001_ha->gpio_lock);
}

/**
 * pm8001_pci_alloc - initialize our ha card structure
 * @pdev: pci device.
 * @ent: ent
 * @shost: scsi host struct which has been initialized before.
 */
static struct pm8001_hba_info 
*pm8001_pci_alloc(struct pci_dev *pdev, const struct pci_device_id *ent, struct Scsi_Host *shost)

{
	struct pm8001_hba_info *pm8001_ha;
	struct sas_ha_struct *sha = SHOST_TO_SAS_HA(shost);


	pm8001_ha = sha->lldd_ha;
	if (!pm8001_ha)
		return NULL;

	pm8001_ha->pdev = pdev;
	pm8001_ha->dev = &pdev->dev;
	pm8001_ha->chip_id = ent->driver_data;
	pm8001_ha->chip = &pm8001_chips[pm8001_ha->chip_id];
	pm8001_ha->irq = pdev->irq;
	pm8001_ha->sas = sha;
	pm8001_ha->shost = shost;
	pm8001_ha->id = pm8001_id++;
	/* Some chips other than 8001 don't work with logging level 0x217 */
	if (pm8001_ha->chip_id == chip_8001)
		pm8001_ha->logging_level = 0x217; 
	else
		pm8001_ha->logging_level = 0; 
	sprintf(pm8001_ha->name, "%s%d", DRV_NAME, pm8001_ha->id);
	/* IOMB size is 128 for 8088/89 controllers */
	if (pm8001_ha->chip_id != chip_8001) {
		pm8001_ha->iomb_size = IOMB_SIZE_SPCV;
	}
	else {
		pm8001_ha->iomb_size = IOMB_SIZE_SPC;
	}

	mutex_init(&pm8001_ha->mutex);
	pm8001_ha->gpio_completion = NULL;
	init_waitqueue_head(&pm8001_ha->pollq);
	pm8001_ha->gpio_event_occured = 0;
	spin_lock_init(&pm8001_ha->gpio_lock);
	setup_timer( &pm8001_ha->gpio_timer, pm8001_gpio_timer_callback, (unsigned long)pm8001_ha);

#ifdef PM8001_USE_TASKLET
	/* default tasklet for non msi-x interrupt handler/first msi-x interrupt handler */
	tasklet_init(&pm8001_ha->tasklet[0], pm8001_tasklet0, (unsigned long)pm8001_ha);
	/* 63 tasklets for msi-x interrupt */
	if (pm8001_ha->chip_id != chip_8001)
	{
		tasklet_init(&pm8001_ha->tasklet[1], pm8001_tasklet1, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[2], pm8001_tasklet2, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[3], pm8001_tasklet3, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[4], pm8001_tasklet4, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[5], pm8001_tasklet5, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[6], pm8001_tasklet6, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[7], pm8001_tasklet7, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[8], pm8001_tasklet8, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[9], pm8001_tasklet9, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[10], pm8001_tasklet10, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[11], pm8001_tasklet11, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[12], pm8001_tasklet12, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[13], pm8001_tasklet13, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[14], pm8001_tasklet14, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[15], pm8001_tasklet15, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[16], pm8001_tasklet16, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[17], pm8001_tasklet17, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[18], pm8001_tasklet18, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[19], pm8001_tasklet19, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[20], pm8001_tasklet20, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[21], pm8001_tasklet21, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[22], pm8001_tasklet22, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[23], pm8001_tasklet23, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[24], pm8001_tasklet24, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[25], pm8001_tasklet25, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[26], pm8001_tasklet26, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[27], pm8001_tasklet27, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[28], pm8001_tasklet28, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[29], pm8001_tasklet29, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[30], pm8001_tasklet30, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[31], pm8001_tasklet31, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[32], pm8001_tasklet32, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[33], pm8001_tasklet33, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[34], pm8001_tasklet34, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[35], pm8001_tasklet35, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[36], pm8001_tasklet36, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[37], pm8001_tasklet37, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[38], pm8001_tasklet38, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[39], pm8001_tasklet39, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[40], pm8001_tasklet40, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[41], pm8001_tasklet41, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[42], pm8001_tasklet42, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[43], pm8001_tasklet43, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[44], pm8001_tasklet44, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[45], pm8001_tasklet45, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[46], pm8001_tasklet46, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[47], pm8001_tasklet47, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[48], pm8001_tasklet48, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[49], pm8001_tasklet49, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[50], pm8001_tasklet50, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[51], pm8001_tasklet51, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[52], pm8001_tasklet52, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[53], pm8001_tasklet53, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[54], pm8001_tasklet54, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[55], pm8001_tasklet55, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[56], pm8001_tasklet56, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[57], pm8001_tasklet57, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[58], pm8001_tasklet58, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[59], pm8001_tasklet59, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[60], pm8001_tasklet60, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[61], pm8001_tasklet61, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[62], pm8001_tasklet62, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[63], pm8001_tasklet63, (unsigned long)pm8001_ha);
	}
#endif
	pm8001_ioremap(pm8001_ha);
	if (!pm8001_alloc(pm8001_ha, ent))
		return pm8001_ha;
	pm8001_free(pm8001_ha);
	return NULL;
}

/**
 * pci_go_44 - pm8001 specified, its DMA is 44 bit rather than 64 bit
 * @pdev: pci device.
 */
static int pci_go_44(struct pci_dev *pdev)
{
	int rc;

	if (!pci_set_dma_mask(pdev, DMA_BIT_MASK(44))) {
		rc = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(44));
		if (rc) {
			rc = pci_set_consistent_dma_mask(pdev,
				DMA_BIT_MASK(32));
			if (rc) {
				dev_printk(KERN_ERR, &pdev->dev,
					"44-bit DMA enable failed\n");
				return rc;
			}
		}
	} else {
		rc = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
		if (rc) {
			dev_printk(KERN_ERR, &pdev->dev,
				"32-bit DMA enable failed\n");
			return rc;
		}
		rc = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
		if (rc) {
			dev_printk(KERN_ERR, &pdev->dev,
				"32-bit consistent DMA enable failed\n");
			return rc;
		}
	}
	return rc;
}

/**
 * pm8001_prep_sas_ha_init - allocate memory in general hba struct && init them.
 * @shost: scsi host which has been allocated outside.
 * @chip_info: our ha struct.
 */
static int pm8001_prep_sas_ha_init(struct Scsi_Host * shost,
	const struct pm8001_chip_info *chip_info)
{
	int phy_nr, port_nr;
	struct asd_sas_phy **arr_phy;
	struct asd_sas_port **arr_port;
	struct sas_ha_struct *sha = SHOST_TO_SAS_HA(shost);

	phy_nr = chip_info->n_phy;
	port_nr = phy_nr;
	memset(sha, 0x00, sizeof(*sha));
	arr_phy = kcalloc(phy_nr, sizeof(void *), GFP_KERNEL);
	if (!arr_phy)
		goto exit;
	arr_port = kcalloc(port_nr, sizeof(void *), GFP_KERNEL);
	if (!arr_port)
		goto exit_free2;

	sha->sas_phy = arr_phy;
	sha->sas_port = arr_port;
	sha->lldd_ha = kzalloc(sizeof(struct pm8001_hba_info), GFP_KERNEL);
	if (!sha->lldd_ha)
		goto exit_free1;

	shost->transportt = pm8001_stt;
	shost->max_id = PM8001_MAX_DEVICES;
	shost->max_lun = 8;
	shost->max_channel = 0;
	shost->unique_id = pm8001_id;
	shost->max_cmd_len = 16;
	shost->can_queue = PM8001_CAN_QUEUE;
	shost->cmd_per_lun = 32;	
	return 0;
exit_free1:
	kfree(arr_port);
exit_free2:
	kfree(arr_phy);
exit:
	return -1;
}

/**
 * pm8001_post_sas_ha_init - initialize general hba struct defined in libsas
 * @shost: scsi host which has been allocated outside
 * @chip_info: our ha struct.
 */
static void pm8001_post_sas_ha_init(struct Scsi_Host *shost,
	const struct pm8001_chip_info *chip_info)
{
	int i = 0;
	struct pm8001_hba_info *pm8001_ha;
	struct sas_ha_struct *sha = SHOST_TO_SAS_HA(shost);

	pm8001_ha = sha->lldd_ha;
	PM8001_INIT_DBG(pm8001_ha, 
			pm8001_printk("hba info vaddr:%p\n", pm8001_ha));
	for (i = 0; i < chip_info->n_phy; i++) {
		sha->sas_phy[i] = &pm8001_ha->phy[i].sas_phy;
		sha->sas_port[i] = &pm8001_ha->port[i].sas_port;
	}
	sha->sas_ha_name = DRV_NAME;
	sha->dev = pm8001_ha->dev;

	sha->lldd_module = THIS_MODULE;
	sha->sas_addr = &pm8001_ha->sas_addr[0];
	sha->num_phys = chip_info->n_phy;
	sha->lldd_max_execute_num = 1;
	sha->lldd_queue_size = PM8001_CAN_QUEUE;
	sha->core.shost = shost;
}

/**
 * pm8001_init_sas_add - initialize sas address
 * @chip_info: our ha struct.
 *
 * Currently we just set the fixed SAS address to our HBA,for manufacture,
 * it should read from the EEPROM
 */
#define PM8001_READ_VPD

static void pm8001_init_sas_add(struct pm8001_hba_info *pm8001_ha)
{
	int i, j;
#ifdef PM8001_READ_VPD
	/*OPTION ROM FLASH read for the SPC cards */
	DECLARE_COMPLETION_ONSTACK(completion);
	struct pm8001_ioctl_payload payload;
	pm8001_ha->nvmd_completion = &completion;
        /* SAS ADDRESS read from flash / EEPROM */
        if (pm8001_ha->chip_id == chip_8001) {
       	    payload.minor_function = 4; //FLASH READ
       	    payload.offset = 0; //FLASH offset
        }
        else {
       	    payload.minor_function = 1; //EEPROM READ 
       	    payload.offset = 0; //EEPROM offset
        }
	payload.length = 4096; //payload length
	payload.func_specific = kzalloc(4096, GFP_KERNEL);
	PM8001_CHIP_DISP->get_nvmd_req(pm8001_ha, &payload);
	wait_for_completion(&completion);
	for(i=0,j=0;i <= 7;i++,j++)
	{
            if (pm8001_ha->chip_id == chip_8001)
           	pm8001_ha->sas_addr[j] = payload.func_specific[0x704 + i];
            else
            	pm8001_ha->sas_addr[j] = payload.func_specific[0x804 + i];
	} 
	for (i = 0; i < pm8001_ha->chip->n_phy; i++) {
		memcpy(&pm8001_ha->phy[i].dev_sas_addr, pm8001_ha->sas_addr,
			SAS_ADDR_SIZE);
	}
#else
	for (i = 0; i < pm8001_ha->chip->n_phy; i++) {
		pm8001_ha->phy[i].dev_sas_addr = 0xCCCCCCCCCCCCCCCCULL;
		pm8001_ha->phy[i].dev_sas_addr =
			cpu_to_be64((u64)
				(*(u64 *)&pm8001_ha->phy[i].dev_sas_addr));
	}
	memcpy(pm8001_ha->sas_addr, &pm8001_ha->phy[0].dev_sas_addr,
		SAS_ADDR_SIZE);
#endif
	for (i = 0; i < pm8001_ha->chip->n_phy; i++) {
		PM8001_INIT_DBG(pm8001_ha,
			pm8001_printk("phy %d sas_addr = %llx \n", i,
			pm8001_ha->phy[i].dev_sas_addr));
	}
	 
}

void pm8001_get_phy_settings_info(struct pm8001_hba_info *pm8001_ha)
{
        u32 major,minor; 
#ifdef PM8001_READ_VPD
        /*OPTION ROM FLASH read for the SPC cards */
        DECLARE_COMPLETION_ONSTACK(completion);
        struct pm8001_ioctl_payload payload;
        pm8001_ha->nvmd_completion = &completion;
        /* SAS ADDRESS read from flash / EEPROM */
        payload.minor_function = 6; //EEPROM READ
        payload.offset = 0; //EEPROM offset     
        payload.length = 4096; //payload length
        payload.func_specific = kzalloc(4096, GFP_KERNEL);
        PM8001_CHIP_DISP->get_nvmd_req(pm8001_ha, &payload);
        wait_for_completion(&completion);    
        major= payload.func_specific[775]<<8 | payload.func_specific[774];    
        minor= payload.func_specific[773]<<8 | payload.func_specific[772];
        printk("PHY Setting Version# %d.%d\n",major,minor);
        pm8001_set_phy_profile(pm8001_ha,sizeof(u8),payload.func_specific);       
         
#endif
}
#ifdef PM8001_USE_MSIX
/**
 * pm8001_setup_msix - enable MSI-X interrupt
 * @chip_info: our ha struct.
 * @irq_handler: irq_handler
 */
static u32 pm8001_setup_msix(struct pm8001_hba_info *pm8001_ha)
{
	u32 i = 0, j = 0;
	u32 number_of_intr;			
	int flag = 0;
	u32 max_entry;
	int rc;
	static char intr_drvname[PM8001_MAX_MSIX_VEC][sizeof(DRV_NAME)+3]; 	
	
	/* SPCv controllers supports 64 msi-x */ 
	if (pm8001_ha->chip_id == chip_8001)
	{
		number_of_intr = 1;
		flag |= IRQF_DISABLED;
	}
	else
	{
		number_of_intr = PM8001_MAX_MSIX_VEC;
		flag &= ~IRQF_SHARED;
	    	flag |= IRQF_DISABLED;
	}
	
	max_entry = sizeof(pm8001_ha->msix_entries) /
		sizeof(pm8001_ha->msix_entries[0]);
	for (i = 0; i < max_entry ; i++)
		pm8001_ha->msix_entries[i].entry = i;
	rc = pci_enable_msix(pm8001_ha->pdev, pm8001_ha->msix_entries,
		number_of_intr);
	pm8001_ha->number_of_intr = number_of_intr;
	if (!rc) {
		PM8001_INIT_DBG(pm8001_ha,
			pm8001_printk("pci_enable_msix request ret:%d number of intr %d\n",
					 rc, pm8001_ha->number_of_intr));	
		for (i = 0; i < number_of_intr; i++) {					
			snprintf(intr_drvname[i], sizeof(intr_drvname[0]), DRV_NAME"%d", i);
			if (request_irq(pm8001_ha->msix_entries[i].vector,
				pm8001_interrupt[i], flag, intr_drvname[i],
				SHOST_TO_SAS_HA(pm8001_ha->shost))) {
				for (j = 0; j < i; j++)
					free_irq(
					pm8001_ha->msix_entries[j].vector,
					SHOST_TO_SAS_HA(pm8001_ha->shost));
				pci_disable_msix(pm8001_ha->pdev);
				break;
			}
		}
	}
	return rc;
}
#endif

/**
 * pm8001_setup_msi - enable MSI interrupt
 * @chip_info: our ha struct.
 * @irq_handler: irq_handler
 */
static u32 pm8001_setup_msi(struct pm8001_hba_info *pm8001_ha)
{
	u32 i = 0;
	u32 number_of_intr;			
	int flag = 0;
	int rc;
	static char intr_drvname[PM8001_MAX_MSI_VEC][sizeof(DRV_NAME)+3]; 	
	
	if (pm8001_ha->chip_id == chip_8001)
	{
		number_of_intr = 1;
		flag |= IRQF_DISABLED;
	}
	else
	{
		number_of_intr = PM8001_MAX_MSI_VEC;
		flag = IRQF_SHARED;
	}

	/* Enable MSI capability */
	rc = pci_enable_msi(pm8001_ha->pdev);
	if (rc)
		return rc;
	pm8001_ha->number_of_intr = number_of_intr;
	for (i = 0; i < number_of_intr; i++) {					
		snprintf(intr_drvname[i], sizeof(intr_drvname[0]), DRV_NAME"%d", i);
		if (request_irq(pm8001_ha->pdev->irq,
				pm8001_interrupt_msi[i], IRQF_SHARED, intr_drvname[i],
				SHOST_TO_SAS_HA(pm8001_ha->shost))) {
			printk("request irq fail %d\n", i);
			free_irq(pm8001_ha->pdev->irq, 
					SHOST_TO_SAS_HA(pm8001_ha->shost));

			pci_disable_msi(pm8001_ha->pdev);
			break;
		}
	}
	return 0;
}

/**
 * pm8001_request_irq - register interrupt
 * @chip_info: our ha struct.
 */
static u32 pm8001_request_irq(struct pm8001_hba_info *pm8001_ha)
{
	struct pci_dev *pdev;
	int rc;

	pdev = pm8001_ha->pdev;

#ifdef PM8001_USE_MSIX
	if (pci_find_capability(pdev, PCI_CAP_ID_MSIX)) {
		rc = pm8001_setup_msix(pm8001_ha);
		if (rc){ 
			pm8001_printk("Enable MSIX failed, "
					"falling back to MSI!!!\n");
			goto msi;
		} 
		return rc;
	} else {
		PM8001_INIT_DBG(pm8001_ha,
			pm8001_printk("MSIX not supported!!!\n"));
	}
#endif
msi:
	if (pci_find_capability(pdev, PCI_CAP_ID_MSI)) {
		rc = pm8001_setup_msi(pm8001_ha);
		if (rc){
			pm8001_printk("Enable MSI failed, "
					"falling back to Legacy INTx!!!\n");
			goto intx;
		}
		return rc;
	} else {
		PM8001_INIT_DBG(pm8001_ha,
			pm8001_printk("MSI not supported!!!\n"));
	}

intx:
	/* initialize the INT-X interrupt */
	rc = request_irq(pdev->irq, pm8001_interrupt_handler0, IRQF_SHARED, DRV_NAME,
		SHOST_TO_SAS_HA(pm8001_ha->shost));
	return rc;
}

/**
 * pm8001_pci_probe - probe supported device
 * @pdev: pci device which kernel has been prepared for.
 * @ent: pci device id
 *
 * This function is the main initialization function, when register a new
 * pci driver it is invoked, all struct an hardware initilization should be done
 * here, also, register interrupt
 */
static int pm8001_pci_probe(struct pci_dev *pdev,
	const struct pci_device_id *ent)
{

	unsigned int rc;
	u32	pci_reg;
	u8 	i = 0;
	struct pm8001_hba_info *pm8001_ha;
	struct Scsi_Host *shost = NULL;
	const struct pm8001_chip_info *chip;

	dev_printk(KERN_INFO, &pdev->dev,
		"driver version %s \n", DRV_VERSION);
	dev_printk(KERN_INFO, &pdev->dev,
		"driver build on %s %s [drv chip ref %ld]\n", 
		__DATE__, __TIME__, ent->driver_data);
	rc = pci_enable_device(pdev);
	if (rc)
		goto err_out_enable;
	pci_set_master(pdev);
	/*
	 * Enable pci slot busmaster by setting pci command register.
	 * This is required by FW for Cyclone card.
	 */

	pci_read_config_dword(pdev, PCI_COMMAND, &pci_reg);
	pci_reg |= 0x157;
	pci_write_config_dword(pdev, PCI_COMMAND, pci_reg);
	rc = pci_request_regions(pdev, DRV_NAME);
	if (rc)
		goto err_out_disable;
	rc = pci_go_44(pdev);
	if (rc)
		goto err_out_regions;

	shost = scsi_host_alloc(&pm8001_sht, sizeof(void *));
	if (!shost) {
		rc = -ENOMEM;
		goto err_out_regions;
	}
	chip = &pm8001_chips[ent->driver_data];
	SHOST_TO_SAS_HA(shost) =
		kzalloc(sizeof(struct sas_ha_struct), GFP_KERNEL);
	if (!SHOST_TO_SAS_HA(shost)) {
		rc = -ENOMEM;
		goto err_out_free_host;
	}

	rc = pm8001_prep_sas_ha_init(shost, chip); 
	if (rc) {
		rc = -ENOMEM;
		goto err_out_free;
	}
	pci_set_drvdata(pdev, SHOST_TO_SAS_HA(shost));
    /* ent->driver variable is used to differentiate between controllers */
	pm8001_ha = pm8001_pci_alloc(pdev, ent, shost);
	if (!pm8001_ha) {
		rc = -ENOMEM;
		goto err_out_free;
	}
	list_add_tail(&pm8001_ha->list, &hba_list);
	PM8001_CHIP_DISP->chip_soft_rst(pm8001_ha);  
	rc = PM8001_CHIP_DISP->chip_init(pm8001_ha);
	if (rc)
	{
		PM8001_FAIL_DBG(pm8001_ha, pm8001_printk("chip_init failed [ret: %d] \n", rc));
		goto err_out_ha_free;
	}

	rc = scsi_add_host(shost, &pdev->dev);
	if (rc)
	{
		PM8001_FAIL_DBG(pm8001_ha, pm8001_printk("scsi_add_host failed [ret: %d] \n", rc));
		goto err_out_ha_free;
	}
	rc = pm8001_request_irq(pm8001_ha);
	if (rc)
	{
		PM8001_FAIL_DBG(pm8001_ha, pm8001_printk("pm8001_request_irq failed [ret: %d] \n", rc));
		goto err_out_shost;
	}

	PM8001_CHIP_DISP->interrupt_enable(pm8001_ha, 0);
    
	if (pm8001_ha->chip_id != chip_8001)
	{
#ifdef PM8001_USE_MSIX
		for (i = 1;i < pm8001_ha->number_of_intr;i++)
			PM8001_CHIP_DISP->interrupt_enable(pm8001_ha, i);
#endif

		/* setup thermal configuration. Applicable only for SPCv/ve controllers. */
                pm80xx_set_thermal_config(pm8001_ha);
        
		PM8001_INIT_DBG(pm8001_ha, pm8001_printk("Thermal configuration successful!\n"));
                
		}
	PM8001_INIT_DBG(pm8001_ha, pm8001_printk("subsystem vendor_id is %x\n",pdev->subsystem_vendor));
	pm8001_init_sas_add(pm8001_ha);
	if(pdev->subsystem_vendor != 0x9005 )
        	pm8001_get_phy_settings_info(pm8001_ha);
	/* setup sgpio for SPC controller */
	if (pm8001_ha->chip_id == chip_8001)
		pm8001_setup_sgpio(pm8001_ha);
	pm8001_post_sas_ha_init(shost, chip);
	rc = sas_register_ha(SHOST_TO_SAS_HA(shost));
	if (rc)
	{
		PM8001_FAIL_DBG(pm8001_ha, pm8001_printk("sas_register_ha failed [ret: %d] \n", rc));
		goto err_out_shost;
	}
	scsi_scan_host(pm8001_ha->shost);
	PM8001_INIT_DBG(pm8001_ha,
		pm8001_printk("Probe steps executed\n"));
	return 0;

err_out_shost:
	scsi_remove_host(pm8001_ha->shost);
err_out_ha_free:
	pm8001_free(pm8001_ha);
err_out_free:
	kfree(SHOST_TO_SAS_HA(shost));
err_out_free_host:
	kfree(shost);
err_out_regions:
	pci_release_regions(pdev);
err_out_disable:
	pci_disable_device(pdev);
err_out_enable:
	return rc;
}

static void pm8001_pci_remove(struct pci_dev *pdev)
{
	struct sas_ha_struct *sha = pci_get_drvdata(pdev);
	struct pm8001_hba_info *pm8001_ha;
	int i;
	pm8001_ha = sha->lldd_ha;
	pci_set_drvdata(pdev, NULL);
	sas_unregister_ha(sha);
	sas_remove_host(pm8001_ha->shost);
	list_del(&pm8001_ha->list);
	scsi_remove_host(pm8001_ha->shost);
	PM8001_CHIP_DISP->interrupt_disable(pm8001_ha, 0xFF);
	PM8001_CHIP_DISP->chip_soft_rst(pm8001_ha);
	
#ifdef PM8001_USE_MSIX
	for (i = 0; i < pm8001_ha->number_of_intr; i++)
		synchronize_irq(pm8001_ha->msix_entries[i].vector);
	for (i = 0; i < pm8001_ha->number_of_intr; i++)	
		free_irq(pm8001_ha->msix_entries[i].vector, sha);
	pci_disable_msix(pdev);
#else
	free_irq(pm8001_ha->irq, sha);
#endif
#ifdef PM8001_USE_TASKLET
	tasklet_kill(&pm8001_ha->tasklet[0]);
	if (pm8001_ha->chip_id != chip_8001)
	{
	    for (i = 1;i < (PM8001_MAX_MSIX_VEC);i++)
		tasklet_kill(&pm8001_ha->tasklet[i]);
	}
#endif
	pm8001_free(pm8001_ha);
	kfree(sha->sas_phy);
	kfree(sha->sas_port);
	kfree(sha);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
}

/**
 * pm8001_pci_suspend - power management suspend main entry point
 * @pdev: PCI device struct
 * @state: PM state change to (usually PCI_D3)
 *
 * Returns 0 success, anything else error.
 */
static int pm8001_pci_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct sas_ha_struct *sha = pci_get_drvdata(pdev);
	struct pm8001_hba_info *pm8001_ha;
	int i , pos;
	u32 device_state;
	pm8001_ha = sha->lldd_ha;
	flush_workqueue(pm8001_wq);
	scsi_block_requests(pm8001_ha->shost);
	pos = pci_find_capability(pdev, PCI_CAP_ID_PM);
	if (pos == 0) {
		printk(KERN_ERR "pm80xx: PCI PM not supported\n");
		return -ENODEV;
	}
	PM8001_CHIP_DISP->interrupt_disable(pm8001_ha, 0xFF);
	PM8001_CHIP_DISP->chip_soft_rst(pm8001_ha);
#ifdef PM8001_USE_MSIX
	for (i = 0; i < pm8001_ha->number_of_intr; i++)
		synchronize_irq(pm8001_ha->msix_entries[i].vector);
	for (i = 0; i < pm8001_ha->number_of_intr; i++)
		free_irq(pm8001_ha->msix_entries[i].vector, sha);
	pci_disable_msix(pdev);
#else
	free_irq(pm8001_ha->irq, sha);
#endif
#ifdef PM8001_USE_TASKLET
	tasklet_kill(&pm8001_ha->tasklet[0]);
	if (pm8001_ha->chip_id != chip_8001)
	{
	    for (i = 1;i < (PM8001_MAX_MSIX_VEC);i++)
		tasklet_kill(&pm8001_ha->tasklet[i]);
	}
#endif
	device_state = pci_choose_state(pdev, state);
	printk(KERN_INFO "pm80xx: pdev=0x%p, slot=%s, entering "
		      "operating state [D%d]\n", pdev,
		      pm8001_ha->name, device_state);
	pci_save_state(pdev);
	pci_disable_device(pdev);
	pci_set_power_state(pdev, device_state);
	return 0;
}

/**
 * pm8001_pci_resume - power management resume main entry point
 * @pdev: PCI device struct
 *
 * Returns 0 success, anything else error.
 */
static int pm8001_pci_resume(struct pci_dev *pdev)
{
	struct sas_ha_struct *sha = pci_get_drvdata(pdev);
	struct pm8001_hba_info *pm8001_ha;
	int rc;
	u8 i = 0;
	u32 device_state;
	pm8001_ha = sha->lldd_ha;
	device_state = pdev->current_state;

	printk(KERN_INFO "pm80xx: pdev=0x%p, slot=%s, resuming from previous "
		"operating state [D%d]\n", pdev, pm8001_ha->name, device_state);

	pci_set_power_state(pdev, PCI_D0);
	pci_enable_wake(pdev, PCI_D0, 0);
	pci_restore_state(pdev);
	rc = pci_enable_device(pdev);
	if (rc) {
		printk(KERN_ERR "pm80xx: slot=%s Enable device failed during resume\n",
			      pm8001_ha->name);
		goto err_out_enable;
	}

	pci_set_master(pdev);
	rc = pci_go_44(pdev);
	if (rc)
		goto err_out_disable;

	/* chip soft rst only for spc */		
	if (pm8001_ha->chip_id == chip_8001)	
	{
		PM8001_CHIP_DISP->chip_soft_rst(pm8001_ha);  
		PM8001_INIT_DBG(pm8001_ha,
				pm8001_printk("chip soft reset successful\n"));
	}
	rc = PM8001_CHIP_DISP->chip_init(pm8001_ha);
	if (rc)
		goto err_out_disable;

	/* disable all the interrupt bits */
	PM8001_CHIP_DISP->interrupt_disable(pm8001_ha, 0xFF);		/* TBU: Need to check whether it is needed */

	rc = pm8001_request_irq(pm8001_ha);
	if (rc)
		goto err_out_disable;
#ifdef PM8001_USE_TASKLET
	/* default tasklet for non msi-x interrupt handler/first msi-x interrupt handler */
	tasklet_init(&pm8001_ha->tasklet[0], pm8001_tasklet0, (unsigned long)pm8001_ha);
	/* 63 tasklets for msi-x interrupt */
	if (pm8001_ha->chip_id != chip_8001)
	{
		tasklet_init(&pm8001_ha->tasklet[1], pm8001_tasklet1, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[2], pm8001_tasklet2, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[3], pm8001_tasklet3, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[4], pm8001_tasklet4, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[5], pm8001_tasklet5, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[6], pm8001_tasklet6, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[7], pm8001_tasklet7, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[8], pm8001_tasklet8, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[9], pm8001_tasklet9, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[10], pm8001_tasklet10, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[11], pm8001_tasklet11, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[12], pm8001_tasklet12, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[13], pm8001_tasklet13, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[14], pm8001_tasklet14, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[15], pm8001_tasklet15, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[16], pm8001_tasklet16, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[17], pm8001_tasklet17, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[18], pm8001_tasklet18, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[19], pm8001_tasklet19, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[20], pm8001_tasklet20, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[21], pm8001_tasklet21, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[22], pm8001_tasklet22, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[23], pm8001_tasklet23, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[24], pm8001_tasklet24, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[25], pm8001_tasklet25, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[26], pm8001_tasklet26, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[27], pm8001_tasklet27, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[28], pm8001_tasklet28, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[29], pm8001_tasklet29, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[30], pm8001_tasklet30, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[31], pm8001_tasklet31, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[32], pm8001_tasklet32, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[33], pm8001_tasklet33, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[34], pm8001_tasklet34, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[35], pm8001_tasklet35, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[36], pm8001_tasklet36, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[37], pm8001_tasklet37, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[38], pm8001_tasklet38, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[39], pm8001_tasklet39, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[40], pm8001_tasklet40, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[41], pm8001_tasklet41, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[42], pm8001_tasklet42, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[43], pm8001_tasklet43, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[44], pm8001_tasklet44, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[45], pm8001_tasklet45, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[46], pm8001_tasklet46, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[47], pm8001_tasklet47, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[48], pm8001_tasklet48, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[49], pm8001_tasklet49, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[50], pm8001_tasklet50, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[51], pm8001_tasklet51, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[52], pm8001_tasklet52, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[53], pm8001_tasklet53, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[54], pm8001_tasklet54, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[55], pm8001_tasklet55, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[56], pm8001_tasklet56, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[57], pm8001_tasklet57, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[58], pm8001_tasklet58, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[59], pm8001_tasklet59, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[60], pm8001_tasklet60, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[61], pm8001_tasklet61, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[62], pm8001_tasklet62, (unsigned long)pm8001_ha);
		tasklet_init(&pm8001_ha->tasklet[63], pm8001_tasklet63, (unsigned long)pm8001_ha);
	}
#endif
	PM8001_CHIP_DISP->interrupt_enable(pm8001_ha, 0);
#ifdef PM8001_USE_MSIX
	if (pm8001_ha->chip_id != chip_8001) {
		for (i = 1;i < pm8001_ha->number_of_intr;i++)
			PM8001_CHIP_DISP->interrupt_enable(pm8001_ha, i);
	}
#endif
	scsi_unblock_requests(pm8001_ha->shost);
	return 0;

err_out_disable:
	scsi_remove_host(pm8001_ha->shost);
	pci_disable_device(pdev);
err_out_enable:
	return rc;
}

/* update of pci device, vendor id and driver data with 
 * unique value for each of the controller 
 */
static struct pci_device_id pm8001_pci_table[] = {
	{ PCI_VDEVICE(PMC_Sierra, 0x8001), chip_8001 },
	{
		PCI_DEVICE(0x117c, 0x0042),
		.driver_data = chip_8001
	},
    /* Support for 8008/8009/8018/8019 controllers */
	{ PCI_VDEVICE(ADAPTEC2, 0x8001), chip_8001 },
	{ PCI_VDEVICE(PMC_Sierra, 0x8008), chip_8008 },
	{ PCI_VDEVICE(ADAPTEC2, 0x8008), chip_8008 },
	{ PCI_VDEVICE(PMC_Sierra, 0x8018), chip_8018 },
	{ PCI_VDEVICE(ADAPTEC2, 0x8018), chip_8018 },
	{ PCI_VDEVICE(PMC_Sierra, 0x8009), chip_8009 },
	{ PCI_VDEVICE(ADAPTEC2, 0x8009), chip_8009 },
	{ PCI_VDEVICE(PMC_Sierra, 0x8019), chip_8019 },
	{ PCI_VDEVICE(ADAPTEC2, 0x8019), chip_8019 },
        { PCI_VDEVICE(PMC_Sierra, 0x8074), chip_8074 },
        { PCI_VDEVICE(ADAPTEC2, 0x8074), chip_8074 },
        { PCI_VDEVICE(PMC_Sierra, 0x8076), chip_8076 },
        { PCI_VDEVICE(ADAPTEC2, 0x8076), chip_8076 },
        { PCI_VDEVICE(PMC_Sierra, 0x8077), chip_8077 },
        { PCI_VDEVICE(ADAPTEC2, 0x8077), chip_8077 },
	{ 0x9005, 0x8081, 0x9005, 0x0400, 0, 0, chip_8001 },
	{ 0x9005, 0x8081, 0x9005, 0x0800, 0, 0, chip_8001 },
	{ 0x9005, 0x8088, 0x9005, 0x0008, 0, 0, chip_8008 },
	{ 0x9005, 0x8088, 0x9005, 0x0800, 0, 0, chip_8008 },
	{ 0x9005, 0x8089, 0x9005, 0x0008, 0, 0, chip_8009 },
	{ 0x9005, 0x8089, 0x9005, 0x0800, 0, 0, chip_8009 },
	{ 0x9005, 0x8088, 0x9005, 0x0016, 0, 0, chip_8018 },
	{ 0x9005, 0x8088, 0x9005, 0x1600, 0, 0, chip_8018 },
	{ 0x9005, 0x8089, 0x9005, 0x0016, 0, 0, chip_8019 },
	{ 0x9005, 0x8089, 0x9005, 0x1600, 0, 0, chip_8019 },
        { 0x9005, 0x8074, 0x9005, 0x0800, 0, 0, chip_8074 },
        { 0x9005, 0x8076, 0x9005, 0x1600, 0, 0, chip_8076 },
        { 0x9005, 0x8077, 0x9005, 0x1600, 0, 0, chip_8077 },
        { 0x9005, 0x8074, 0x9005, 0x0008, 0, 0, chip_8074 },
        { 0x9005, 0x8076, 0x9005, 0x0016, 0, 0, chip_8076 },
        { 0x9005, 0x8077, 0x9005, 0x0016, 0, 0, chip_8077 },
        { 0x9005, 0x8076, 0x9005, 0x0808, 0, 0, chip_8076 },
        { 0x9005, 0x8077, 0x9005, 0x0808, 0, 0, chip_8077 },
        { 0x9005, 0x8074, 0x9005, 0x0404, 0, 0, chip_8074 },

	{} /* terminate list */
};

static struct pci_driver pm8001_pci_driver = {
	.name		= DRV_NAME,
	.id_table	= pm8001_pci_table,
	.probe		= pm8001_pci_probe,
	.remove		= pm8001_pci_remove,
	.suspend	= pm8001_pci_suspend,
	.resume		= pm8001_pci_resume,
};

/**
 *	pm8001_open		-	open the configuration file
 *	@inode: inode being opened
 *	@file: file handle attached
 *
 *	Called when the configuration device is opened. Does the needed
 *	set up on the handle and then returns
 *
 */

static int pm8001_open(struct inode *inode, struct file *file)
{
	struct pm8001_hba_info *pm8001_ha;
	unsigned minor_number = iminor(inode);
	int err = -ENODEV;

	list_for_each_entry(pm8001_ha, &hba_list, list) {
		if (pm8001_ha->id == minor_number) {
			file->private_data = pm8001_ha;
			err = 0;
			break;
		}
	}

	return err;
}

/**
 *      pm8001_close             -       close the configuration file
 *      @inode: inode being opened
 *      @file: file handle attached
 *
 *      Called when the configuration device is closed. Does the needed
 *      set up on the handle and then returns
 *
 */

static int pm8001_close(struct inode *inode, struct file *file)
{
	return 0;
}

static long pm8001_info_ioctl(struct pm8001_hba_info *pm8001_ha, unsigned long arg)
{
	u32 ret = 0;
	struct ioctl_info_buffer info_buf;

	strcpy(info_buf.information.sz_name, DRV_NAME);
					
	info_buf.information.usmajor_revision = DRV_MAJOR;
	info_buf.information.usminor_revision = DRV_MINOR;
	info_buf.information.usbuild_revision = DRV_BUILD;
	if (pm8001_ha->chip_id == chip_8001) {
		info_buf.information.maxoutstandingIO = 
			pm8001_ha->main_cfg_tbl.pm8001_tbl.max_out_io;
		info_buf.information.maxdevices =
			(pm8001_ha->main_cfg_tbl.pm8001_tbl.max_sgl >> 16) & 0xFFFF;
	}
	else {
		info_buf.information.maxoutstandingIO =
			pm8001_ha->main_cfg_tbl.pm80xx_tbl.max_out_io;
		info_buf.information.maxdevices =
			(pm8001_ha->main_cfg_tbl.pm80xx_tbl.max_sgl >> 16) & 0xFFFF;
	}
	info_buf.header.return_code = ADPT_IOCTL_CALL_SUCCESS;

	if (copy_to_user((void *)arg, (void *)&info_buf, sizeof(struct ioctl_info_buffer)))
	{
		ret = ADPT_IOCTL_CALL_FAILED;
	}
	return ret;
}

static long pm8001_gpio_ioctl(struct pm8001_hba_info *pm8001_ha, unsigned long arg)
{
	struct gpio_buffer buffer;
	struct pm8001_gpio *payload;
	struct gpio_ioctl_resp *gpio_resp;
	DECLARE_COMPLETION_ONSTACK(completion);
	unsigned long timeout;
	u32 ret = 0;

	if(pm8001_ha->pdev->subsystem_vendor == 0x9005)
		return ADPT_IOCTL_CALL_INVALID_DEVICE;

	mutex_lock(&pm8001_ha->mutex);

	if (copy_from_user(&buffer, (struct gpio_buffer *)arg,
		sizeof(struct gpio_buffer)))
	{
		ret = ADPT_IOCTL_CALL_FAILED;
		goto exit;
	}
	pm8001_ha->gpio_completion = &completion;
	payload = &buffer.gpio_payload;

	ret = PM8001_CHIP_DISP->gpio_req(pm8001_ha, payload, 
		buffer.header.ioctl_num);
	if(ret != 0) {
		ret = ADPT_IOCTL_CALL_FAILED;
		goto exit;
        }

	timeout = (unsigned long)buffer.header.timeout * 1000;
	if(timeout < 2000)
		timeout = 2000;

	ret = mod_timer( &pm8001_ha->gpio_timer, jiffies + msecs_to_jiffies(timeout) );
	if (ret)
		PM8001_MSG_DBG(pm8001_ha, pm8001_printk("Error in mod_timer \n"));

	wait_for_completion(&completion);

	if(pm8001_ha->gpio_timer_expired)
	{
		ret = ADPT_IOCTL_CALL_TIMEOUT;
		goto exit;
	}
	gpio_resp = &pm8001_ha->gpio_resp;

	buffer.header.return_code = ADPT_IOCTL_CALL_SUCCESS;

	if(buffer.header.ioctl_num == GPIO_READ)
	{
		payload->rd_wr_val		= gpio_resp->gpio_rd_val;
		payload->input_enable		= gpio_resp->gpio_in_enabled;
		payload->pinsetup1		= gpio_resp->gpio_pinsetup1;
		payload->pinsetup2		= gpio_resp->gpio_pinsetup2;
		payload->event_level		= gpio_resp->gpio_evt_change;
		payload->event_rising_edge	= gpio_resp->gpio_evt_rise;
		payload->event_falling_edge	= gpio_resp->gpio_evt_fall;

		if (copy_to_user((void *)arg, (void *)&buffer, sizeof(struct gpio_buffer)))
		{
			ret = ADPT_IOCTL_CALL_FAILED;
		}

	} 
	else
	{
		if (copy_to_user((void *)arg, (void *)&buffer.header, sizeof(struct ioctl_header)))
		{
			ret = ADPT_IOCTL_CALL_FAILED;
		}
	}

exit:
	pm8001_ha->gpio_timer_expired = 0;
	mutex_unlock(&pm8001_ha->mutex);
	return ret;
}

/**
 *	pm8001_ioctl		-	pm8001 configuration request
 *	@inode: inode of device
 *	@file: file handle
 *	@cmd: ioctl command code
 *	@arg: argument
 *
 *	Handles a configuration ioctl.
 *
 */

static long pm8001_ioctl(struct file *file,
		unsigned int cmd, unsigned long arg)
{
	u32 ret = -EACCES;
	struct pm8001_hba_info *pm8001_ha;
	struct ioctl_header header;

	pm8001_ha = file->private_data;

	switch(cmd) {
		case ADPT_IOCTL_GPIO:
			ret = pm8001_gpio_ioctl(pm8001_ha, arg);
			break;

		case ADPT_IOCTL_INFO:
			ret = pm8001_info_ioctl(pm8001_ha, arg);
			break;
			
		default:
			ret = ADPT_IOCTL_CALL_INVALID_CODE;
	}
	if(ret == 0)
		return ret;
	header.return_code = ret;
	ret = -EACCES;
	if(copy_to_user((void *)arg, (void *)&header, sizeof(struct ioctl_header)))
	{
		PM8001_FAIL_DBG(pm8001_ha, pm8001_printk("copy_to_user failed \n"));
	}
	PM8001_FAIL_DBG(pm8001_ha, pm8001_printk("IOCTL failed \n"));

	return ret;
}

unsigned int pm8001_poll (struct file *file, poll_table *wait)
{
	struct pm8001_hba_info *pm8001_ha;
	unsigned int mask = 0;

	pm8001_ha = file->private_data;

	poll_wait(file, &pm8001_ha->pollq,  wait);

	if(pm8001_ha->gpio_event_occured == 1)
	{
		pm8001_ha->gpio_event_occured = 0;
		mask |= POLLIN | POLLRDNORM;
	}
	
	return mask;
}

static const struct file_operations pm8001_fops = {
	.owner		= THIS_MODULE,
	.open		= pm8001_open,
	.release	= pm8001_close,
	.unlocked_ioctl	= pm8001_ioctl,
	.poll		= pm8001_poll,
};

/**
 *	pm8001_init - initialize scsi transport template
 */
static int __init pm8001_init(void)
{
	int rc = -ENOMEM;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	pm8001_wq = create_workqueue("pm80xx");
#else
	pm8001_wq = alloc_workqueue("pm80xx", 0, 0);
#endif		
	if (!pm8001_wq)
		goto err;

	pm8001_id = 0;
	pm8001_stt = sas_domain_attach_transport(&pm8001_transport_ops);
	if (!pm8001_stt)
		goto err_wq;
	rc = pci_register_driver(&pm8001_pci_driver);
	if (rc)
		goto err_tp;

	pm80xx_major = register_chrdev( 0, DRV_NAME, &pm8001_fops);
	if (pm80xx_major < 0) {
		printk(KERN_WARNING
			"pm8001: unable to register \"%s\" device.\n", DRV_NAME);
	}

	return 0;

err_tp:
	sas_release_transport(pm8001_stt);
err_wq:
	destroy_workqueue(pm8001_wq);
err:
	return rc;
}

static void __exit pm8001_exit(void)
{
	if (pm80xx_major > -1)
	{
		unregister_chrdev(pm80xx_major, DRV_NAME);
		pm80xx_major = -1;
	}

	pci_unregister_driver(&pm8001_pci_driver);
	sas_release_transport(pm8001_stt);
	destroy_workqueue(pm8001_wq);
}

module_init(pm8001_init);
module_exit(pm8001_exit);

MODULE_AUTHOR("Jack Wang <jack_wang@usish.com>");
MODULE_DESCRIPTION("PMC-Sierra PM8001/8081/8088/8089/8074/8076/8077 SAS/SATA controller driver");
MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(pci, pm8001_pci_table);

