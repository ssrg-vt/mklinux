/*
 * APM X-Gene SoC Security User IO Driver
 *
 * Copyright (c) 2014 Applied Micro Circuits Corporation.
 * All rights reserved. Loc Ho <lho@apm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <crypto/algapi.h>
#include <linux/module.h>
#include <asm/cacheflush.h>
#include "xgene_sec.h"
#include "xgene_sec_tkn.h"

/* Bits in flags */
enum {
	UIO_IRQ_DISABLED = 0,
};

#define UIO_TKN_SA_SIZE		(512 * MAX_SLOT + 512 * MAX_SLOT)
#define UIO_BUFFER_SIZE		(16 * 1024 * 256)

static int xgene_sec_uio_open(struct uio_info *info, struct inode *inode)
{
	struct xgene_sec_ctx *ctx = info->priv;

	/* Todo: Clear HW and QM states for work and completion queues */

	/* Load the queue ID to the descriptor rings */
	*(u32 *) ctx->qm_queue.txq.queue_vaddr = ctx->qm_queue.txq.queue_id;
	*(u32 *) ctx->qm_queue.cpq.queue_vaddr = ctx->qm_queue.cpq.queue_id;

	/* Flush to ensure user space map retrieves proper values */
	__flush_dcache_area(ctx->qm_queue.txq.queue_vaddr, 64 * 1024);
	__flush_dcache_area(ctx->qm_queue.cpq.queue_vaddr, 64 * 1024);
	__flush_dcache_area(ctx->tknsa_array, UIO_TKN_SA_SIZE);
	__flush_dcache_area(ctx->buf_array, UIO_BUFFER_SIZE);
	return 0;
}

static int xgene_sec_uio_release(struct uio_info *info, struct inode *inode)
{
	return 0;
}

static irqreturn_t xgene_sec_uio_handler(int irq, struct uio_info *dev_info)
{
	struct xgene_sec_ctx *ctx = dev_info->priv;

	/*
	 * Just disable the interrupt in the interrupt controller, and
	 * remember the state so we can allow user space to enable it later.
	 */
	spin_lock(&ctx->lock);
	if (!__test_and_set_bit(UIO_IRQ_DISABLED, &ctx->flags))
		disable_irq_nosync(irq);
	spin_unlock(&ctx->lock);

	return IRQ_HANDLED;
}

static int xgene_sec_uio_irqcontrol(struct uio_info *dev_info, s32 irq_on)
{
	struct xgene_sec_ctx *ctx = dev_info->priv;
	unsigned long flags;

	/*
	 * Allow user space to enable and disable the interrupt
	 * in the interrupt controller, but keep track of the
	 * state to prevent per-irq depth damage.
	 *
	 * Serialize this operation to support multiple tasks and concurrency
	 * with irq handler on SMP systems.
	 */
	spin_lock_irqsave(&ctx->lock, flags);
	if (irq_on) {
		if (__test_and_clear_bit(UIO_IRQ_DISABLED, &ctx->flags))
			enable_irq(dev_info->irq);
	} else {
		if (!__test_and_set_bit(UIO_IRQ_DISABLED, &ctx->flags))
			disable_irq_nosync(dev_info->irq);
	}
	spin_unlock_irqrestore(&ctx->lock, flags);

	return 0;
}

int xgene_sec_uio_init(struct platform_device *pdev, struct xgene_sec_ctx *ctx)
{
	struct uio_info *uioinfo = &ctx->uioinfo;
	struct uio_mem *uiomem;
	struct resource *res;
	int rc;

	ctx->flags = 0; /* interrupt is enabled to begin with */

	uioinfo->name = dev_name(ctx->dev);
	uioinfo->version = "0.1.0";

	/* Add the IP CSR region */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL)
		return -ENODEV;
	uiomem = &uioinfo->mem[0];
	uiomem->memtype = UIO_MEM_PHYS;
	uiomem->addr = res->start;
	uiomem->size = resource_size(res);
	uiomem->name = "SECCSR";
	++uiomem;

	/* Add the IP QM Fabric CSR */
	uiomem->memtype = UIO_MEM_PHYS;
	uiomem->addr = 0x1b000000;
	uiomem->size = 0x400000;
	uiomem->name = "QMFCSR";
	++uiomem;

	/* Add the QM queue descriptor regions */
	uiomem->memtype = UIO_MEM_PHYS;
	uiomem->size = 64 * 1024;
	uiomem->addr = dma_map_single(ctx->dev, ctx->qm_queue.txq.queue_vaddr,
				      uiomem->size, DMA_BIDIRECTIONAL);
	uiomem->name = "TXQ";
	*(u32 *) ctx->qm_queue.txq.queue_vaddr = ctx->qm_queue.txq.queue_id;
	++uiomem;

	uiomem->memtype = UIO_MEM_PHYS;
	uiomem->size = 64 * 1024;
	uiomem->addr = dma_map_single(ctx->dev, ctx->qm_queue.cpq.queue_vaddr,
				      uiomem->size, DMA_BIDIRECTIONAL);
	uiomem->name = "CPQ";
	*(u32 *) ctx->qm_queue.cpq.queue_vaddr = ctx->qm_queue.cpq.queue_id;
	++uiomem;

	/* Add the Token and SA regions */
	uiomem->size = UIO_TKN_SA_SIZE;
	ctx->tknsa_array = kmalloc(uiomem->size, GFP_ATOMIC);
	uiomem->memtype = UIO_MEM_PHYS;
	uiomem->addr = dma_map_single(ctx->dev, ctx->tknsa_array, uiomem->size,
				      DMA_BIDIRECTIONAL);
	uiomem->name = "Token-SA";
	++uiomem;

	/* Add the buffer region */
	uiomem->size = UIO_BUFFER_SIZE;
	ctx->buf_array = kmalloc(uiomem->size, GFP_ATOMIC);
	uiomem->memtype = UIO_MEM_PHYS;
	uiomem->addr = dma_map_single(ctx->dev, ctx->buf_array, uiomem->size,
				      DMA_BIDIRECTIONAL);
	uiomem->name = "Buffers";
	++uiomem;

	while (uiomem < &uioinfo->mem[MAX_UIO_MAPS]) {
		uiomem->size = 0;
		++uiomem;
	}

	/*
	 * This driver requires no hardware specific kernel code to handle
	 * interrupts. Instead, the interrupt handler simply disables the
	 * interrupt in the interrupt controller. User space is responsible
	 * for performing hardware specific acknowledge and re-enabling of
	 * the interrupt in the interrupt controller.
	 *
	 * Interrupt sharing is not supported.
	 */
	uioinfo->handler = xgene_sec_uio_handler;
	uioinfo->irqcontrol = xgene_sec_uio_irqcontrol;
	uioinfo->open = xgene_sec_uio_open;
	uioinfo->release = xgene_sec_uio_release;
	uioinfo->priv = ctx;
	uioinfo->irq = ctx->qm_queue.cpq.irq;

	rc = uio_register_device(ctx->dev, uioinfo);
	if (rc) {
		dev_err(&pdev->dev, "unable to register uio device\n");
		return rc;
	}

	return 0;
}

int xgene_sec_uio_deinit(struct xgene_sec_ctx *ctx)
{
	uio_unregister_device(&ctx->uioinfo);

	ctx->uioinfo.handler = NULL;
	ctx->uioinfo.irqcontrol = NULL;
	return 0;
}
