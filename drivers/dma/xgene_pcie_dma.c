/*
 * Header File for APM PCIe DMA Driver
 *
 * Copyright (c) 2010, Applied Micro Circuits Corporation
 * Author: Tanmay Inamdar <tinamdar@apm.com>
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
 */

#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/pm_runtime.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/delay.h>
#include <asm/cacheflush.h>
#include "xgene_pcie_dma.h"

static struct list_head dma_port_list;
int port_id;
/* Enable polling instead of irq. Seeing issue with interrupts */
int enable_polling = 0;
/* This will flush only the descriptors and not data buffers */
int enable_flush = 0;

struct apm_pcie_info {
	u8	max_chan;
	u32	src_elems;
	u32	dst_elems;
	u32	sts_elems;
};

#define INFO(_max_chan, _src_elems, _dst_elems, _sts_elems)	\
	(kernel_ulong_t)&(struct apm_pcie_info) {  		\
	 .max_chan = (_max_chan),                        	\
	 .src_elems = (_src_elems),                          	\
	 .dst_elems = (_dst_elems),                   		\
	 .sts_elems = (_sts_elems),                      	\
	 }

/* Hack since IO coherency is not working */
static void flush_to_ddr(void *addr, int len)
{
#ifdef CONFIG_DMA_FLUSH
	if (enable_flush)
		__flush_dcache_area(addr, len);
#endif
}

static int wait_for_dma_completion(struct apm_dma_chan *, struct apm_pcie_dma_desc *);

static inline void apm_dma_write(struct pci_dev *dev, u32 offset, u32 val)
{
	u32 rval;
	APD_CFG_DEBUG("APD_DMA_WRITE: Offset: 0x%08X value: 0x%08X\n", offset, val);
	pcibios_prep_pcie_dma(dev->bus);
	pci_read_config_dword(dev, offset, &rval);
	if (rval != val)
		pci_write_config_dword(dev, offset, val);
	pcibios_cleanup_pcie_dma(dev->bus);
}

static inline void apm_dma_read(struct pci_dev *dev, u32 offset, u32 *val)
{
	pcibios_prep_pcie_dma(dev->bus);
	pci_read_config_dword(dev, offset, val);
	pcibios_cleanup_pcie_dma(dev->bus);
	APD_CFG_DEBUG("APD_DMA_READ:  Offset: 0x%08X value: 0x%08X\n", offset,
			*val);
}

static inline int is_dma_channel_present(struct apm_dma_chan *chan)
{
	u32 val;
	apm_dma_read(chan->pdev, chan->base + DMA_CONTROL, &val);
	return val & DMA_PRESENT_MASK;
}

static inline int is_dma_channel_enabled(struct apm_dma_chan *chan)
{
	u32 val;
	apm_dma_read(chan->pdev, chan->base + DMA_CONTROL, &val);
	return val & DMA_ENABLE_MASK;
}

static inline int is_dma_channel_running(struct apm_dma_chan *chan)
{
	u32 val;
	int timeout = 1000;

	/* Give DMA enought time to complete outstanding requests */
	do {
		usleep_range(10, 10);
		apm_dma_read(chan->pdev, chan->base + DMA_CONTROL, &val);
	} while((val & DMA_RUNNING_MASK) && timeout--);

	return (timeout == -1) ? 1: 0;
}

static inline void reset_dma_channel(struct apm_dma_chan *chan)
{
	u32 val;
	APD_VDEBUG("ENTER\n");
	apm_dma_read(chan->pdev, chan->base + DMA_CONTROL, &val);
	val |= DMA_RESET_MASK;
	apm_dma_write(chan->pdev, chan->base + DMA_CONTROL, val);
	/* wait 256nS */
	udelay(1);
	val &= ~DMA_RESET_MASK;
	apm_dma_write(chan->pdev, chan->base + DMA_CONTROL, val);
}

static inline void disable_dma_channel(struct apm_dma_chan *chan)
{
	u32 val;
	APD_VDEBUG("ENTER\n");
	apm_dma_read(chan->pdev, chan->base + DMA_CONTROL, &val);
	val &= ~DMA_ENABLE_MASK;
	apm_dma_write(chan->pdev, chan->base + DMA_CONTROL, val);
}

static inline void enable_dma_channel(struct apm_dma_chan *chan)
{
	u32 val;
	APD_VDEBUG("ENTER\n");
	apm_dma_read(chan->pdev, chan->base + DMA_CONTROL, &val);
	val |= DMA_ENABLE_MASK;
	apm_dma_write(chan->pdev, chan->base + DMA_CONTROL, val);
}

static inline void disable_axi_dma_interrupts(struct apm_dma_chan *chan)
{
	u32 val;
	APD_VDEBUG("ENTER\n");
	apm_dma_read(chan->pdev, chan->base + DMA_CONTROL, &val);
	val &= ~AXI_INTERRUPT_ENABLE_MASK;
	apm_dma_write(chan->pdev, chan->base + DMA_CONTROL, val);
}

static inline void enable_axi_dma_interrupts(struct apm_dma_chan *chan)
{
	u32 val;
	APD_VDEBUG("ENTER\n");
	apm_dma_read(chan->pdev, chan->base + DMA_CONTROL, &val);
	val |= AXI_INTERRUPT_ENABLE_MASK;
	apm_dma_write(chan->pdev, chan->base + DMA_CONTROL, val);
}

static inline void disable_pci_dma_interrupts(struct apm_dma_chan *chan)
{
	u32 val;
	APD_VDEBUG("ENTER\n");
	apm_dma_read(chan->pdev, chan->base + DMA_CONTROL, &val);
	val &= ~PCIE_INTERRUPT_ENABLE_MASK;
	apm_dma_write(chan->pdev, chan->base + DMA_CONTROL, val);
}

static inline void enable_pci_dma_interrupts(struct apm_dma_chan *chan)
{
	u32 val;
	APD_VDEBUG("ENTER\n");
	apm_dma_read(chan->pdev, chan->base + DMA_CONTROL, &val);
	val |= PCIE_INTERRUPT_ENABLE_MASK;
	apm_dma_write(chan->pdev, chan->base + DMA_CONTROL, val);
}

static inline void apm_dma_setup_src_q_desc(struct src_queue_desc *src_desc,
				     	    u64 addr, u32 byte_cnt, bool eop,
					    bool intr, u8 location)
{
	APD_DEBUG("src_desc : 0x%p, addr : 0x%08llx, byte_cnt : 0x%x\n",
		   src_desc, (long long unsigned int)addr, byte_cnt);
	src_desc->addr = addr;
	src_desc->fields.flags.byte_cnt = byte_cnt;
	src_desc->fields.flags.location = location;
	src_desc->fields.flags.eop = eop;
	src_desc->fields.flags.intr = intr;
	/* Enable cache coherency setting */
	src_desc->fields.flags.attr = 2;
	flush_to_ddr(src_desc, sizeof(struct src_queue_desc));
#ifdef APM_PCIE_DMA_VDEBUG
	print_hex_dump( KERN_ERR, "SRC_DESC: ", DUMP_PREFIX_NONE, 16, 1,
			src_desc, sizeof(struct src_queue_desc), 1);
#endif
}

static inline void apm_dma_setup_dst_q_desc(struct dst_queue_desc *dst_desc,
					    u64 addr, u32 byte_cnt, u8 location)
{
	APD_DEBUG("dst_desc : 0x%p, addr : 0x%08llx, byte_cnt : 0x%x\n",
		   dst_desc, (long long unsigned int)addr, byte_cnt);
	dst_desc->addr = addr;
	dst_desc->fields.flags.byte_cnt = byte_cnt;
	dst_desc->fields.flags.location = location;
	/* Enable cache coherency setting */
	dst_desc->fields.flags.attr = 2;
	flush_to_ddr(dst_desc, sizeof(struct dst_queue_desc));
#ifdef APM_PCIE_DMA_VDEBUG
	print_hex_dump( KERN_ERR, "DST_DESC: ", DUMP_PREFIX_NONE, 16, 1,
			dst_desc, sizeof(struct dst_queue_desc), 1);
#endif
}

static inline void apm_dma_update_src_cnts(struct apm_dma_chan *chan)
{
	chan->next_src_elem++;
	if (chan->next_src_elem >= chan->src_elems)
		chan->next_src_elem = 0;
	chan->used_src_elems++;
}

static inline void apm_dma_update_dst_cnts(struct apm_dma_chan *chan)
{
	chan->next_dst_elem++;
	if (chan->next_dst_elem >= chan->dst_elems)
		chan->next_dst_elem = 0;
	chan->used_dst_elems++;
}

static inline void apm_dma_update_sts_cnts(struct apm_dma_chan *chan)
{
	chan->next_sts_elem++;
	if (chan->next_sts_elem >= chan->sts_elems)
		chan->next_sts_elem = 0;
	chan->used_sts_elems++;
}

static inline void apm_dma_get_next_queues(struct apm_dma_chan *apm_chan,
					struct apm_pcie_dma_desc *desc)
{
	desc->src_desc = (struct src_queue_desc *)(
			(char *)apm_chan->src_elem_addr +
			(apm_chan->next_src_elem * sizeof(*desc->src_desc)));

	desc->dst_desc = (struct dst_queue_desc *)(
			(char *)apm_chan->dst_elem_addr +
			(apm_chan->next_dst_elem * sizeof(*desc->dst_desc)));

	desc->sts_desc = (struct sts_queue_desc *)(
			(char *)apm_chan->sts_elem_addr +
			(apm_chan->next_sts_elem * sizeof(*desc->sts_desc)));
}

/**
 * DMA Helper Functions
 */

/**
 * apm_desc_get	- get a descriptor
 * @apm_chan: dma channel for which descriptor is required
 *
 * Obtain a descriptor for the channel. Returns NULL if none are free.
 * Once the descriptor is returned it is private until put on another
 * list or freed
 */
static struct apm_pcie_dma_desc *apm_desc_get(struct apm_dma_chan *apm_chan)
{
	struct apm_pcie_dma_desc *desc, *_desc;
	struct apm_pcie_dma_desc *ret = NULL;

	APD_VDEBUG("ENTER\n");
	spin_lock_irq(&apm_chan->lock);
	BUG_ON(list_empty(&apm_chan->free_list));

	list_for_each_entry_safe(desc, _desc, &apm_chan->free_list, desc_node) {
		APD_VDEBUG("Traverse free list desc = %p\n", desc);
		if (async_tx_test_ack(&desc->txd)) {
			APD_VDEBUG("Found desc = %p\n", desc);
			list_del(&desc->desc_node);
			async_tx_clear_ack(&desc->txd);
			ret = desc;
			break;
		}
	}
	spin_unlock_irq(&apm_chan->lock);
	APD_VDEBUG("EXIT\n");
	return ret;
}

/**
 * apm_desc_put	- put a descriptor
 * @apm_chan: dma channel for which descriptor is required
 * @desc: descriptor to put
 *
 * Return a descriptor back to the free pool
 * Call with chan lock held
 */
static void apm_desc_put(struct apm_dma_chan *apm_chan,
			  struct apm_pcie_dma_desc *desc)
{
	APD_VDEBUG("ENTER\n");
	if (desc) {
		APD_VDEBUG("add to free list - desc = %p\n", desc);
		spin_lock_irq(&apm_chan->lock);
		list_add_tail(&desc->desc_node, &apm_chan->free_list);
		spin_unlock_irq(&apm_chan->lock);
	}
	APD_VDEBUG("EXIT\n");
}


/**
 * apm_start_dma - begin a DMA transaction
 * @apm_chan: channel for which txn is to be started
 * @first: first descriptor of series
 *
 * Load a transaction into the engine. This must be called with apm_chan->lock
 * held and bh disabled.
 */
static void apm_start_dma(struct apm_dma_chan *apm_chan,
			  struct apm_pcie_dma_desc *first)
{
	APD_VDEBUG("ENTER\n");

	/*write registers and en*/
	apm_dma_write(apm_chan->pdev, apm_chan->base + SRC_Q_LIMIT,
		      apm_chan->next_src_elem);
	apm_dma_write(apm_chan->pdev, apm_chan->base + DST_Q_LIMIT,
		      apm_chan->next_dst_elem);
	apm_dma_write(apm_chan->pdev, apm_chan->base + STA_Q_LIMIT,
		      apm_chan->next_sts_elem);

	apm_chan->state = CHANNEL_BUSY;
	first->status = DMA_IN_PROGRESS;

	/*XXX: DMA channel should always be enabled */
	enable_dma_channel(apm_chan);
	if (enable_polling)
		wait_for_dma_completion(apm_chan, first);

	APD_VDEBUG("EXIT\n");
}

/**
 * apm_descriptor_complete - process completed descriptor
 * @apm_chan: channel owning the descriptor
 * @desc: the descriptor itself
 *
 * Process a completed descriptor and perform any callbacks upon
 * the completion. The completion handling drops the lock during the
 * callbacks but must be called with the lock held.
 */
static void apm_descriptor_complete(struct apm_dma_chan *apm_chan,
				     struct apm_pcie_dma_desc *desc)
{
	struct dma_async_tx_descriptor	*txd = &desc->txd;
	struct sts_queue_desc *sts_desc = desc->sts_desc;
	dma_async_tx_callback callback_txd = NULL;
	void *param_txd = NULL;

	APD_VDEBUG("ENTER\n");
	BUG_ON(!apm_chan);
	BUG_ON(!desc);
	BUG_ON(!sts_desc);
	BUG_ON(!txd);

	flush_to_ddr(desc->sts_desc, sizeof(struct sts_queue_desc));
#ifdef APM_PCIE_DMA_VDEBUG
        print_hex_dump( KERN_ERR, "STS_DESC: ", DUMP_PREFIX_NONE, 16, 1,
                        sts_desc, sizeof(struct sts_queue_desc), 1);
#endif
	apm_chan->completed = txd->cookie;
	callback_txd = txd->callback;
	param_txd = txd->callback_param;

	if (desc->flags & DMA_COMPL_SRC_UNMAP_SINGLE) {
		APD_DEBUG("unmap source addr = 0x%llx\n", desc->src_desc->addr);
		dma_unmap_single(&apm_chan->pdev->dev, desc->src_desc->addr, desc->len, DMA_TO_DEVICE);
	}
	if (desc->flags & DMA_COMPL_DEST_UNMAP_SINGLE) {
		APD_DEBUG("unmap dest addr = 0x%llx\n", desc->dst_desc->addr);
		dma_unmap_single(&apm_chan->pdev->dev, desc->dst_desc->addr, desc->len, DMA_FROM_DEVICE);
	}

	if (callback_txd) {
		APD_DEBUG("TXD callback set ... calling\n");
		callback_txd(param_txd);
	}

	if (sts_desc->fields.dword) {
		if (sts_desc->fields.flags.completed) {
			APD_DEBUG("CHAN%d: DMA transaction comepleted\n",
					apm_chan->id);
			spin_lock_irq(&apm_chan->lock);
			desc->status = DMA_SUCCESS;
			spin_unlock_irq(&apm_chan->lock);
			apm_chan->transfer_cnt++;
		} else {
			APD_DEBUG("CHAN%d: DMA transaction error 0x%08X\n",
					apm_chan->id, sts_desc->fields.dword);
			spin_lock_irq(&apm_chan->lock);
			desc->status = DMA_ERROR;
			spin_unlock_irq(&apm_chan->lock);
			apm_chan->err_cnt++;
		}
	} else {
		APD_DEBUG("CHAN%d: DMA transaction pending\n",  apm_chan->id);
		goto skip_cleanup;
	}
	/* Clear the status descriptor */
	sts_desc->fields.dword = 0;
	/* Freeup the elements */
	spin_lock_irq(&apm_chan->lock);
	apm_chan->used_src_elems -= desc->src_element_cnt;
	apm_chan->used_dst_elems -= desc->dst_element_cnt;
	spin_unlock_irq(&apm_chan->lock);
	apm_desc_put(apm_chan, desc);

skip_cleanup:
	spin_lock_irq(&apm_chan->lock);
	apm_chan->state = CHANNEL_READY;
	spin_unlock_irq(&apm_chan->lock);
	APD_VDEBUG("EXIT\n");
}

/**
 * apm_scan_descriptors - check the descriptors in channel
 *			  mark completed when tx is completete
 * @apm_chan: channel to scan
 *
 * Walk the descriptor chain for the device and process any entries
 * that are complete.
 */
static void apm_scan_descriptors(struct apm_dma_chan *apm_chan)
{
	struct apm_pcie_dma_desc *desc = NULL, *_desc = NULL;

	APD_VDEBUG("ENTER\n");
	/*tx is complete*/
	list_for_each_entry_safe(desc, _desc, &apm_chan->active_list,
				 desc_node) {
		APD_VDEBUG("scan %p\n", desc);
		if (desc->status == DMA_IN_PROGRESS) {
			APD_VDEBUG("found %p\n",desc);
			spin_lock_irq(&apm_chan->lock);
			list_del(&desc->desc_node);
			spin_unlock_irq(&apm_chan->lock);
			apm_descriptor_complete(apm_chan, desc);
		}
	}
	APD_VDEBUG("EXIT\n");
	return;
}

/**
 * dmaengine support apis
 */

/**
 * apm_pcie_dma_tx_submit -	callback to submit DMA transaction
 * @tx: dma engine descriptor
 *
 * Submit the DMA trasaction for this descriptor, start if ch idle
 */
static dma_cookie_t apm_pcie_dma_tx_submit(struct dma_async_tx_descriptor *tx)
{
	struct apm_pcie_dma_desc *desc = to_apm_pcie_dma_desc(tx);
	struct apm_dma_chan *apm_chan = to_apm_pcie_dma_chan(tx->chan);
	dma_cookie_t cookie;

	APD_VDEBUG("ENTER\n");

	if (is_dma_channel_running(apm_chan)) {
		APD_ERR("channel%d is busy in start\n",apm_chan->id);
		apm_desc_put(apm_chan, desc);
		return -1;
	}

	spin_lock_irq(&apm_chan->lock);
	cookie = apm_chan->chan.cookie;
	if (++cookie < 0)
		cookie = 1;
	apm_chan->chan.cookie = cookie;
	desc->txd.cookie = cookie;
	APD_VDEBUG("desc = %p add to active list\n", desc);
	list_add_tail(&desc->desc_node, &apm_chan->active_list);
	apm_start_dma(apm_chan, desc);
	spin_unlock_irq(&apm_chan->lock);

	APD_VDEBUG("EXIT\n");
	return cookie;
}

/**
 * apm_pcie_dma_alloc_chan_resources - Allocate dma resources
 * @chan: chan requiring attention
 *
 * Allocates DMA resources on this chan
 * Return the descriptors allocated
 */
static int apm_pcie_dma_alloc_chan_resources(struct dma_chan *chan)
{
	struct apm_dma_chan *apm_chan = to_apm_pcie_dma_chan(chan);
	struct apm_pcie_dma_desc *desc;
	dma_addr_t phys;
	int i = 0;

	APD_VDEBUG("ENTER\n");
	if (apm_chan->state == CHANNEL_DISABLED) {
		APD_ERR("channel %d is DISABLED\n",apm_chan->id);
		return -ENXIO;
	}

	apm_chan->completed = chan->cookie = 1;

	for (i = 0; i < DESCS_PER_CHANNEL; i++) {
		desc = (struct apm_pcie_dma_desc *)
			pci_alloc_consistent(apm_chan->pdev, sizeof(*desc),
					     &phys);
		if(!desc) {
			APD_ERR("Failure allocating channel descriptors\n");
			return -ENOMEM;
		}
		APD_VDEBUG("desc : 0x%p , phys : 0x%08lx\n",
			    desc, (unsigned long) phys);
		dma_async_tx_descriptor_init(&desc->txd, chan);
		desc->txd.tx_submit = apm_pcie_dma_tx_submit;
		desc->txd.flags = DMA_CTRL_ACK;
		desc->txd.phys = phys;
		list_add_tail(&desc->desc_node, &apm_chan->free_list);
	}
	enable_axi_dma_interrupts(apm_chan);
	spin_unlock_irq(&apm_chan->lock);
	apm_chan->descs_allocated = i;
	APD_DEBUG("Allocated %d descriptors for APM DMA Chan%d\n", i,
		   apm_chan->id);
	return i;
}

/**
 * apm_pcie_dma_free_chan_resources -  Frees dma resources
 * @chan: chan requiring attention
 *
 * Frees the allocated resources on this DMA chan
 */
static void apm_pcie_dma_free_chan_resources(struct dma_chan *chan)
{
	struct apm_dma_chan *apm_chan = to_apm_pcie_dma_chan(chan);
	struct apm_pcie_dma_desc *desc, *_desc;

	APD_VDEBUG("ENTER\n");
	if (apm_chan->state == CHANNEL_BUSY) { 
		APD_ERR("DMA channel %d BUST\n",apm_chan->id);
		return;
	}

	spin_lock_irq(&apm_chan->lock);
	apm_chan->descs_allocated = 0;
	apm_chan->state = CHANNEL_DISABLED;	
	disable_axi_dma_interrupts(apm_chan);
	spin_unlock_irq(&apm_chan->lock);

	list_for_each_entry_safe(desc, _desc, &apm_chan->free_list,
				 desc_node) {
		list_del(&desc->desc_node);
		APD_VDEBUG("Freeing Free list Desc\n");
		APD_VDEBUG("desc : 0x%p , phys : 0x%08lx\n",
			    desc, (unsigned long) desc->txd.phys);
		pci_free_consistent(apm_chan->pdev, sizeof(*desc), desc,
				    desc->txd.phys);
	}

	list_for_each_entry_safe(desc, _desc, &apm_chan->queue, desc_node) {
		list_del(&desc->desc_node);
		pci_free_consistent(apm_chan->pdev, sizeof(*desc), desc,
				    desc->txd.phys);
	}

	apm_chan->state = CHANNEL_READY;
	APD_VDEBUG("EXIT\n");
}

/**
 * apm_pcie_dma_device_control - DMA device control
 * @chan: chan for DMA control
 * @cmd: control cmd
 * @arg: cmd arg value
 *
 * Perform DMA control command
 */
static int apm_pcie_dma_device_control(struct dma_chan *chan,
		enum dma_ctrl_cmd cmd, unsigned long arg)
{
	struct apm_dma_chan *apm_chan = to_apm_pcie_dma_chan(chan);
	struct dma_slave_config *slave = (struct dma_slave_config *)arg;

	APD_DEBUG("ENTER\n");
	BUG_ON(!apm_chan);

	/* Right now we only support DMA_SLAVE_CONFIG */
	if(cmd != DMA_SLAVE_CONFIG)
		return -ENXIO;
	BUG_ON(!slave);

	apm_chan->dma_slave = slave;
	APD_VDEBUG("EXIT\n");
	return 0;
}

static struct dma_async_tx_descriptor *apm_pcie_dma_prep_sg(
		struct dma_chan *chan,
		struct scatterlist *dst_sg, unsigned int dst_nents,
		struct scatterlist *src_sg, unsigned int src_nents,
		unsigned long flags)
{
	struct apm_dma_chan *apm_chan = to_apm_pcie_dma_chan(chan);
	struct dma_slave_config *dma_slave = apm_chan->dma_slave;
	struct apm_pcie_dma_desc *desc = NULL;
	struct scatterlist  *sg;
	int i, src_location, dst_location, slen = 0, glen = 0;
	u8 eop_intr = 0;

	/* basic sanity checks */
	if (dst_nents == 0 || src_nents == 0 || !dst_sg || !src_sg) {
		APD_ERR("Incorrect input parameters\n");
		return NULL;
	}

	for_each_sg(src_sg, sg, src_nents, i)
		slen += sg_dma_len(sg);	

	for_each_sg(dst_sg, sg, dst_nents, i)
		glen += sg_dma_len(sg);

	if (slen != glen) {
		APD_ERR("Scatter gather lenth mismatch\n");
		return NULL;
	}

	BUG_ON(!dma_slave);

	APD_DEBUG("Channel%d: Prep slave sg\n",apm_chan->id);

	if ((dma_slave->direction != DMA_MEM_TO_DEV) &&
			(dma_slave->direction != DMA_DEV_TO_MEM)) {
		APD_ERR("Incorrect DMA Transfer direction %d\n", dma_slave->direction);
		return NULL;
	}

	desc = apm_desc_get(apm_chan);
	if(!desc) {
		APD_ERR("Failed to get desc\n");
		return NULL;
	}

	desc->len = slen;
	desc->flags = flags;
	desc->txd.flags = flags;
	spin_lock_irq(&apm_chan->lock);
	apm_dma_get_next_queues(apm_chan, desc);
	apm_dma_update_sts_cnts(apm_chan);

	/* Setup the in memory source, destination and status descriptors */
	src_location = (dma_slave->direction == DMA_MEM_TO_DEV) ? AXI_MEM : PCI_MEM;
	dst_location = (dma_slave->direction == DMA_MEM_TO_DEV) ? PCI_MEM : AXI_MEM;
	for_each_sg(src_sg, sg, src_nents, i) {
		if (i == (src_nents - 1))
			eop_intr = 1;
		apm_dma_setup_src_q_desc(desc->src_desc,
				sg_dma_address(sg), sg_dma_len(sg), eop_intr,
				eop_intr, src_location);
		apm_dma_update_src_cnts(apm_chan);
		desc->src_element_cnt++;
		desc->src_desc++;
	}
	for_each_sg(dst_sg, sg, dst_nents, i) {
			apm_dma_setup_dst_q_desc(desc->dst_desc,
				sg_dma_address(sg), sg_dma_len(sg), dst_location);
			apm_dma_update_dst_cnts(apm_chan);
			desc->dst_element_cnt++;
			desc->dst_desc++;
	}
	spin_unlock_irq(&apm_chan->lock);

	APD_VDEBUG("EXIT\n");
	return &desc->txd;
}
/* apm_pcie_dma_prep_slave_sg - Prep slave sg txn
 * @chan: chan for DMA transfer
 * @sgl: scatter gather list
 * @sg_len: length of sg txn
 * @direction: DMA transfer dirtn
 * @flags: DMA flags
 * @context: transfer context (ignored)
 *
 * Prepares sgl based periphral transfer
 */
static struct dma_async_tx_descriptor *apm_pcie_dma_prep_slave_sg(
		struct dma_chan *chan, struct scatterlist *sgl,
		unsigned int sg_len, enum dma_transfer_direction dir,
		unsigned long flags, void *context)
{

	struct apm_dma_chan *apm_chan = to_apm_pcie_dma_chan(chan);
	struct dma_slave_config *dma_slave = apm_chan->dma_slave;
	struct apm_pcie_dma_desc *desc = NULL;
	struct scatterlist  *sg;
	int i;
	u8 eop_intr = 0;

	BUG_ON(!dma_slave);

	APD_DEBUG("Channel%d: Prep slave sg\n",apm_chan->id);

	if ((dir != DMA_MEM_TO_DEV) && (dir != DMA_DEV_TO_MEM)) {
		APD_ERR("Incorrect DMA Transfer direction %d\n", dir);
		return NULL;
	}

	desc = apm_desc_get(apm_chan);
	if(!desc) {
		APD_ERR("Failed to get desc\n");
		return NULL;
	}

	spin_lock_irq(&apm_chan->lock);
	apm_dma_get_next_queues(apm_chan, desc);
	apm_dma_update_sts_cnts(apm_chan);

	/* Setup the in memory source, destination and status descriptors */
	desc->len = 0;
	for_each_sg(sgl, sg, sg_len, i) {
		if (dir == DMA_MEM_TO_DEV) {
			if (i == (sg_len - 1))
				eop_intr = 1;
			apm_dma_setup_src_q_desc(desc->src_desc,
				sg_dma_address(sg), sg_dma_len(sg), eop_intr,
				eop_intr, AXI_MEM);
			apm_dma_update_src_cnts(apm_chan);
			desc->src_element_cnt++;
			desc->len += sg_dma_len(sg);
			desc->src_desc++;
		} else {
			apm_dma_setup_dst_q_desc(desc->dst_desc,
				sg_dma_address(sg), sg_dma_len(sg), AXI_MEM);
			apm_dma_update_dst_cnts(apm_chan);
			desc->dst_element_cnt++;
			desc->len += sg_dma_len(sg);
			desc->dst_desc++;
		}
	}

	if (dir == DMA_MEM_TO_DEV) {
		apm_dma_setup_dst_q_desc(desc->dst_desc, dma_slave->dst_addr,
				desc->len, PCI_MEM);
		apm_dma_update_dst_cnts(apm_chan);
		desc->dst_element_cnt++;
	} else {
		apm_dma_setup_src_q_desc(desc->src_desc, dma_slave->src_addr,
				desc->len, 1, 1, PCI_MEM);
		apm_dma_update_src_cnts(apm_chan);
		desc->src_element_cnt++;
	}
	spin_unlock_irq(&apm_chan->lock);

	APD_VDEBUG("EXIT\n");
	return &desc->txd;
}


/**
 * apm_pcie_dma_prep_memcpy -	Prep memcpy txn
 * @chan: chan for DMA transfer
 * @dest: destn address
 * @src: src address
 * @len: DMA transfer len
 * @flags: DMA flags
 *
 * Perform a DMA memcpy. Note we support slave periphral DMA transfers only
 * The periphral txn details should be filled in slave structure properly
 * Returns the descriptor for this txn
 * Right now we assume on AXI to AXI (may be mapped to PCIe) transfer.
 */
static struct dma_async_tx_descriptor *apm_pcie_dma_prep_memcpy(
			struct dma_chan *chan, dma_addr_t dest,
			dma_addr_t src, size_t len, unsigned long flags)
{

	struct apm_dma_chan *apm_chan = to_apm_pcie_dma_chan(chan);
	struct apm_pcie_dma_desc *desc = NULL;
	u8 src_location, dest_location;

	BUG_ON(!apm_chan);

	APD_DEBUG("Channel%d: Preparing for memcpy\n",apm_chan->id);

	desc = apm_desc_get(apm_chan);
	if(!desc)
		goto err_desc_get;

	/* Setup the in memory source, destination and status descriptors */
	desc->len = len;
	desc->src_element_cnt++;
	desc->dst_element_cnt++;
	desc->flags = flags;
	desc->txd.flags = flags;

	src_location  = (flags & DMA_COMPL_SRC_UNMAP_SINGLE) ? AXI_MEM : PCI_MEM;
	dest_location = (flags & DMA_COMPL_SRC_UNMAP_SINGLE) ? PCI_MEM : AXI_MEM;

	spin_lock_irq(&apm_chan->lock);
	apm_dma_get_next_queues(apm_chan, desc);
	apm_dma_update_src_cnts(apm_chan);
	apm_dma_update_dst_cnts(apm_chan);
	apm_dma_update_sts_cnts(apm_chan);
	flush_to_ddr(desc->sts_desc, sizeof(struct sts_queue_desc));
	spin_unlock_irq(&apm_chan->lock);

	apm_dma_setup_src_q_desc(desc->src_desc, src, len, 1, 1, src_location);
	apm_dma_setup_dst_q_desc(desc->dst_desc, dest, len, dest_location);

	APD_VDEBUG("EXIT\n");
	return &desc->txd;
err_desc_get:
	APD_ERR("Failed to get desc\n");
	apm_desc_put(apm_chan, desc);
	return NULL;
}

/**
 * apm_pcie_dma_tx_status - Return status of txn
 * @chan: chan for where status needs to be checked
 * @cookie: cookie for txn
 * @txstate: DMA txn state
 *
 * Return status of DMA txn
 */
static enum dma_status apm_pcie_dma_tx_status(struct dma_chan *chan,
					      dma_cookie_t cookie,
					      struct dma_tx_state *txstate)
{
	struct apm_dma_chan *apm_chan = to_apm_pcie_dma_chan(chan);
	dma_cookie_t last_used;
	dma_cookie_t last_complete;
	int ret;

	APD_VDEBUG("ENTER\n");
	last_complete = apm_chan->completed;
	last_used = chan->cookie;

	ret = dma_async_is_complete(cookie, last_complete, last_used);
	if (ret != DMA_SUCCESS) {
		apm_scan_descriptors(apm_chan);
		last_complete = apm_chan->completed;
		last_used = chan->cookie;
		ret = dma_async_is_complete(cookie, last_complete, last_used);
	}

	if (txstate) {
		txstate->last = last_complete;
		txstate->used = last_used;
		txstate->residue = 0;
	}
	APD_VDEBUG("EXIT\n");
	return ret;
}

/**
 * apm_pcie_dma_issue_pending -	callback to issue pending txn
 * @chan: chan where pending trascation needs to be checked and submitted
 *
 * Call for scan to issue pending descriptors
 */
static void apm_pcie_dma_issue_pending(struct dma_chan *chan)
{
	struct apm_dma_chan *apm_chan = to_apm_pcie_dma_chan(chan);

	APD_VDEBUG("ENTER\n");
	spin_lock_irq(&apm_chan->lock);
	if (!list_empty(&apm_chan->queue)) {
		list_splice_tail(&apm_chan->queue, &apm_chan->active_list);
		apm_scan_descriptors(apm_chan);
	}
	spin_unlock_irq(&apm_chan->lock);
	APD_VDEBUG("EXIT\n");
}

static void apm_setup_dma_caps(struct apm_dma_port *port)
{
	dma_cap_zero(port->common.cap_mask);
	dma_cap_set(DMA_SLAVE, port->common.cap_mask);
	dma_cap_set(DMA_MEMCPY, port->common.cap_mask);
	dma_cap_set(DMA_PRIVATE, port->common.cap_mask);
	port->common.dev = &port->pdev->dev;

	port->common.device_alloc_chan_resources =
					apm_pcie_dma_alloc_chan_resources;
	port->common.device_free_chan_resources =
					apm_pcie_dma_free_chan_resources;
	port->common.device_prep_dma_memcpy = apm_pcie_dma_prep_memcpy;
	port->common.device_tx_status = apm_pcie_dma_tx_status;
	port->common.device_issue_pending = apm_pcie_dma_issue_pending;
	port->common.device_prep_slave_sg = apm_pcie_dma_prep_slave_sg;
	port->common.device_prep_dma_sg = apm_pcie_dma_prep_sg;
	port->common.device_control = apm_pcie_dma_device_control;
}

/* Implements polling mechanism to check if DMA is comeplete */
static int wait_for_dma_completion(struct apm_dma_chan *apm_chan,
				   struct apm_pcie_dma_desc *desc)
{
	u32 status;
	int timeout = 1000;
	
	BUG_ON(apm_chan->state == CHANNEL_DISABLED);

	do {
		apm_dma_read(apm_chan->pdev, apm_chan->base + DMA_CONTROL,
				&status);
		if(status & AXI_INTERRUPT_STATUS_MASK) {
			APD_VDEBUG("chan%d: APM DMA AXI Interrupt\n",
					apm_chan->id);
			/* set bit again to clear interrupt */
			status |= AXI_INTERRUPT_STATUS_MASK;
			break;
		}
	} while(timeout--);

	if (timeout == -1)
		APD_DEBUG("Request Timeout!\n");
	apm_descriptor_complete(apm_chan, desc);
	return 0;
}

/**
 * ISR and Tasklet
 */
static irqreturn_t apm_dma_isr(int irq, void *data)
{
	struct apm_dma_port *port = (struct apm_dma_port *) data;
	struct apm_dma_chan *apm_chan;
	int index;
	u32 status;

	APD_VDEBUG("ENTER APM DMA ISR\n");
	for (index = 0; index < port->num_chan; index++) {
		apm_chan = &port->chan[index];
		BUG_ON(apm_chan->state == CHANNEL_DISABLED);
		apm_dma_read(apm_chan->pdev, apm_chan->base + DMA_CONTROL,
			     &status);
		APD_DEBUG("Status : 0x%08x\n", status);
		if(status & AXI_INTERRUPT_STATUS_MASK) {
			APD_VDEBUG("chan%d: APM DMA AXI Interrupt\n",
				   apm_chan->id);
			/* set bit again to clear interrupt */
			status |= AXI_INTERRUPT_STATUS_MASK; 
		} else if (status & PCIE_INTERRUPT_STATUS_MASK) {
			/* XXX: Not expected */
			APD_VDEBUG("chan%d: APM DMA PCIe Interrupt\n",
				   apm_chan->id);
			status |= PCIE_INTERRUPT_STATUS_MASK;
		} else {
			APD_ERR("chan%d: Spurious Interrupt\n", apm_chan->id);
			return IRQ_NONE;
		}
		apm_dma_write(apm_chan->pdev, apm_chan->base + DMA_CONTROL,
			      status);
		apm_chan->irq_cnt++;
		disable_dma_channel(apm_chan);
	}

	APD_VDEBUG("Scheduling tasklet\n");
	/* Lets do further processing in bottom half */
	tasklet_schedule(&port->completion_tasklet);
	APD_VDEBUG("EXIT ISR\n");
	return IRQ_HANDLED;
}

/**
 * apm_dma_tasklet -	DMA interrupt tasklet
 * @data: tasklet arg (the controller structure)
 *
 * Scan the controller for interrupts for completion/error
 * Clear the interrupt and call for handling completion/error
 */

static void apm_dma_tasklet(unsigned long data)
{
	struct apm_dma_port *port = (struct apm_dma_port *) data;
	struct apm_dma_chan *apm_chan;
	int index;

	APD_DEBUG("ENTER PCIe DMA TASKLET\n");
	for (index = 0; index < port->num_chan; index++) {
		apm_chan = &port->chan[index];
		apm_scan_descriptors(apm_chan);
		apm_chan->tasklet_cnt++;
	}
	APD_DEBUG("EXIT TASKLET\n");
}

/**
 * apm_dma_alloc_sglists - Allocate Source, Destination and Status
 *			   descriptors sg lists
 * @chan: APM PCIe DMA channel structure
 *
 * Allocate the the DMA channel  Source, Destination and Status
 * descriptors sg lists
 */
static int apm_dma_alloc_sglists(struct apm_dma_chan *chan)
{
	unsigned long src_alloc_len;
	unsigned long dst_alloc_len;
	unsigned long sts_alloc_len;

	APD_DEBUG("Size of src_queue_desc = %lu, dst_queue_desc = %lu,"
		  "sts_queue_desc = %lu\n", sizeof(struct src_queue_desc),
		  sizeof(struct dst_queue_desc), sizeof(struct sts_queue_desc));
	src_alloc_len = chan->src_elems * sizeof(struct src_queue_desc);
	chan->src_elem_addr = pci_alloc_consistent(chan->pdev, src_alloc_len,
						   &chan->src_elem_addr_phys);
	if(!chan->src_elem_addr) {
		APD_ERR("Failed to allocate source sg descriptors\n");
		goto src_alloc_fail;
	}
	APD_DEBUG("src_elem_addr: 0x%p src_elem_addr_phys: 0x%08llx "
		  "src_alloc_len : %ld\n", chan->src_elem_addr,
		  (unsigned long long)chan->src_elem_addr_phys,
		  src_alloc_len);
	memset(chan->src_elem_addr, 0, src_alloc_len);

	dst_alloc_len = chan->dst_elems * sizeof(struct dst_queue_desc);
	chan->dst_elem_addr = pci_alloc_consistent(chan->pdev, dst_alloc_len,
						   &chan->dst_elem_addr_phys);
	if(!chan->dst_elem_addr) {
		APD_ERR("Failed to allocate destination sg descriptors\n");
		goto dst_alloc_fail;
	}
	APD_DEBUG("dst_elem_addr: 0x%p dst_elem_addr_phys: 0x%08llx "
		  "dst_alloc_len : %ld\n", chan->dst_elem_addr,
		   (unsigned long long)chan->dst_elem_addr_phys,
		   dst_alloc_len);
	memset(chan->dst_elem_addr, 0, dst_alloc_len);

	sts_alloc_len = chan->sts_elems * sizeof(struct sts_queue_desc);
	chan->sts_elem_addr = pci_alloc_consistent(chan->pdev, sts_alloc_len,
						   &chan->sts_elem_addr_phys);
	if(!chan->sts_elem_addr) {
		APD_ERR("Failed to allocate source sg descriptors\n");
		goto sts_alloc_fail;
	}
	APD_DEBUG("sts_elem_addr: 0x%p sts_elem_addr_phys: 0x%08llx "
		  "sts_alloc_len : %ld\n", chan->sts_elem_addr,
		   (unsigned long long)chan->sts_elem_addr_phys,
		   sts_alloc_len);
	memset(chan->sts_elem_addr, 0, sts_alloc_len);
	return 0;
sts_alloc_fail:
	pci_free_consistent(chan->pdev,dst_alloc_len,chan->dst_elem_addr,
			    chan->dst_elem_addr_phys);
dst_alloc_fail:
	pci_free_consistent(chan->pdev,src_alloc_len,chan->src_elem_addr,
			    chan->src_elem_addr_phys);
src_alloc_fail:
	return -ENOMEM;

}

/**
 * apm_init_dma_channel - Initialize the DMA controller HW
 * @chan: APM PCIe DMA channel structure
 *
 * Reset DMA channel. Initialize the DMA channel source,
 * destination and status queues. Enable DMA channel.
 */

static int apm_init_dma_channel(struct apm_dma_chan *chan)
{
	struct pci_dev *pdev = chan->pdev;

	APD_DEBUG("ENTER\n");
	/* XXX: Should we bail out here? */
	if (is_dma_channel_running(chan))
		APD_ERR("DMA Channel %d still RUNNING\n",chan->id);

	/* Disable DMA Channel fiels */
	disable_axi_dma_interrupts(chan);
	disable_pci_dma_interrupts(chan);
	disable_dma_channel(chan);
	reset_dma_channel(chan);
	/* Now setup queue management registers */
	apm_dma_write(pdev, chan->base + SRC_Q_NEXT, 0);
	apm_dma_write(pdev, chan->base + DST_Q_NEXT, 0);
	apm_dma_write(pdev, chan->base + STA_Q_NEXT, 0);

	apm_dma_write(pdev, chan->base + SRC_Q_PTR_LO,
			LODWORD(chan->src_elem_addr_phys) | AXI_MEM |
			AXI_DESC_COHERENCY);
	apm_dma_write(pdev, chan->base + SRC_Q_PTR_HI,
			HIDWORD(chan->src_elem_addr_phys));
	apm_dma_write(pdev, chan->base + SRC_Q_SIZE, chan->src_elems);

	apm_dma_write(pdev, chan->base + DST_Q_PTR_LO,
			LODWORD(chan->dst_elem_addr_phys) | AXI_MEM |
			AXI_DESC_COHERENCY);
	apm_dma_write(pdev, chan->base + DST_Q_PTR_HI,
			HIDWORD(chan->dst_elem_addr_phys));
	apm_dma_write(pdev, chan->base + DST_Q_SIZE, chan->dst_elems);

	apm_dma_write(pdev, chan->base + STA_Q_PTR_LO,
			LODWORD(chan->sts_elem_addr_phys) | AXI_MEM |
			AXI_DESC_COHERENCY);
	apm_dma_write(pdev, chan->base + STA_Q_PTR_HI,
			HIDWORD(chan->sts_elem_addr_phys));
	apm_dma_write(pdev, chan->base + STA_Q_SIZE, chan->sts_elems);

	chan->state = CHANNEL_READY;
	return 0;
}

/**
 * apm_setup_dma - Setup the DMA controller
 * @pdev: Controller PCI device structure
 * @info: Driver probe information structure
 *
 * Initialize the DMA controller, channels, registers with DMA engine,
 * ISR. Initialize DMA controller channels.
 */
static int apm_setup_dma_channel(struct apm_dma_port *port,
				 struct apm_pcie_info *info)
{
	struct pci_dev *pdev = port->pdev;
	struct apm_dma_chan *apm_chan;
	int i;
	int ret = 0;

	INIT_LIST_HEAD(&port->common.channels);
	port->id = port_id++;
	APD_DEBUG("Port ID: %d\n", port->id);
	for (i = 0; i < port->num_chan; i++) {
		apm_chan = &port->chan[i];
		memset(apm_chan, 0, sizeof(*apm_chan));
		apm_chan->state = CHANNEL_DISABLED;
		apm_chan->id = i;
		apm_chan->pdev = pdev;
		apm_chan->base = i * CHAN_REG_SPACE;
		apm_chan->chan.device = &port->common;
		apm_chan->chan.cookie =  1;

		if(!is_dma_channel_present(apm_chan)) {
			APD_ERR("DMA Channel%d is disabled\n",apm_chan->id);
			continue;
		}

		APD_DEBUG("Found DMA Channel %d\n", i);
		apm_chan->src_elems = info->src_elems;
		apm_chan->dst_elems = info->dst_elems;
		apm_chan->sts_elems = info->sts_elems;
		APD_DEBUG("src_elems : %u dest_elems : %u sts_elems : %u\n",
			   apm_chan->src_elems, apm_chan->dst_elems,
			   apm_chan->sts_elems);
		if (apm_chan->src_elems > apm_chan->dst_elems)
			apm_chan->max_transfer_len = apm_chan->src_elems *
						     PAGE_SIZE;
		else
			apm_chan->max_transfer_len = apm_chan->dst_elems *
						     PAGE_SIZE;

		ret = apm_dma_alloc_sglists(apm_chan);
		if(ret)
			continue;
		INIT_LIST_HEAD(&apm_chan->queue);
		INIT_LIST_HEAD(&apm_chan->active_list);
		INIT_LIST_HEAD(&apm_chan->free_list);
		spin_lock_init(&apm_chan->lock);
		apm_init_dma_channel(apm_chan);
		list_add_tail(&apm_chan->chan.device_node,
			      &port->common.channels);
	}

	return 0;
}

static void apm_adjust_dma_attributes(struct apm_pcie_info *info)
{
	if(info->max_chan > MAX_DMA_CHAN)
		info->max_chan = MAX_DMA_CHAN;
	if((info->src_elems > MAX_SRC_SG_LISTS) || (info->src_elems < 2))
		info->src_elems = MAX_SRC_SG_LISTS;
	if((info->dst_elems > MAX_DST_SG_LISTS) || (info->dst_elems < 2))
		info->dst_elems = MAX_DST_SG_LISTS;
}

/**
 * apm_pcie_dma_probe - PCI Probe
 * @pdev: Controller PCI device structure
 * @id: pci device id structure
 *
 * Initialize the DMA port and DMA channels.
 * Call setup_dma to complete contoller and chan initilzation
 */
static int apm_pcie_dma_probe(struct pci_dev *pdev,
                              const struct pci_device_id *id)
{
	struct apm_dma_port *port;
	struct apm_pcie_info *info = (void *)id->driver_data;
	int err;

	printk("APD: probe for %x, dma channels %d\n",
		pdev->device, info->max_chan);

	apm_adjust_dma_attributes(info);

	err = pci_enable_device(pdev);
	if (err)
		goto fail;

	if (!pci_set_dma_mask(pdev, DMA_BIT_MASK(64))) {
		if (!pci_set_dma_mask(pdev, DMA_BIT_MASK(64))) {
			err = pci_set_consistent_dma_mask(pdev,
						DMA_BIT_MASK(64));
			if (err)
				goto fail;
		}
	}

	pci_set_master(pdev);

	port = kzalloc(sizeof(*port), GFP_KERNEL);
	if (!port) {
		APD_ERR("kzalloc failed APM PCIe DMA probe\n");
		return -ENOMEM;
	}
	port->pdev = pci_dev_get(pdev);
	pci_set_drvdata(pdev, port);
	port->num_chan = info->max_chan;
	err = apm_setup_dma_channel(port, info);
	if (err)
		goto fail1;

	apm_setup_dma_caps(port);

	if (!enable_polling) {
		int virq =  pdev->irq + DMA_IRQ_OFFSET;
		APD_DEBUG("Requesting IRQ %d for APM PCIe DMA\n", virq);
		err = request_irq(virq, apm_dma_isr, IRQF_SHARED, "APM PCIe DMA",
				  port);
		if (err) {
			APD_ERR("Request IRQ failed for APM PCIe DMA\n");
			goto fail1;
		}

		tasklet_init(&port->completion_tasklet, apm_dma_tasklet,
			    (long unsigned int)port);
		of_node_put(pdev->dev.of_node);
	}

	err = dma_async_device_register(&port->common);
	if (err) {
		APD_ERR("APM PCIe DMA device_register failed: %d\n", err);
		goto fail2;
	}

	list_add_tail(&port->list, &dma_port_list);
	return 0;
fail2:
	free_irq(pdev->irq, port);
fail1:
	kfree(port);
fail:
	APD_ERR("APM PCIe DMA setup failed\n");
	return err;
}

static void apm_pcie_dma_remove(struct pci_dev *pdev)
{
	struct apm_dma_port *port = pci_get_drvdata(pdev);
	struct apm_dma_port *p1;

	list_for_each_entry(p1, &dma_port_list, list) {
		if(p1 == port) {
			list_del(&p1->list);
			dma_async_device_unregister(&p1->common);
			free_irq(pdev->irq + DMA_IRQ_OFFSET, p1);
			break;
		}
	}
	kfree(port);
}


/*
 * PCI stuff
 */

static DEFINE_PCI_DEVICE_TABLE(apm_pcie_dma_ids) = {
        { 0x10E8, 0xE004, PCI_ANY_ID, PCI_ANY_ID, 0, 0,
	  INFO(1, 8192, 8192, 8192)},
        { 0 },
};
MODULE_DEVICE_TABLE(pci, apm_pcie_dma_ids);

static struct pci_driver apm_pcie_dma_pci_driver = {
        .name           =       "APM PCIe DMA",
        .id_table       =       apm_pcie_dma_ids,
        .probe          =       apm_pcie_dma_probe,
        .remove         =       apm_pcie_dma_remove,
};

static int __init apm_pcie_dma_init(void)
{
        APD_DEBUG("APM PCIe DMA Driver\n");
	INIT_LIST_HEAD(&dma_port_list);
        return pci_register_driver(&apm_pcie_dma_pci_driver);
}
module_init(apm_pcie_dma_init);

static void __exit apm_pcie_dma_exit(void)
{
        pci_unregister_driver(&apm_pcie_dma_pci_driver);
}
module_exit(apm_pcie_dma_exit);

MODULE_AUTHOR("Tanmay Inamdar <tinamdar@apm.com>");
MODULE_DESCRIPTION("APM PCIe DMA Driver");
MODULE_LICENSE("GPL v2");

