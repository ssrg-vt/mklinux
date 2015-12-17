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

#ifndef __APM_PCIE_DMA_H__
#define __APM_PCIE_DMA_H__

#include <linux/dmaengine.h>
#include <linux/dmapool.h>
#include <linux/pci_ids.h>

#if 0
#define APM_PCIE_DMA_DEBUG
#define APM_PCIE_DMA_VDEBUG
#define APM_PCIE_DMA_CFG_DEBUG
#endif

#ifdef APM_PCIE_DMA_DEBUG
# define APD_DEBUG(fmt, ...)		printk(KERN_DEBUG "%s: "fmt, __func__,##__VA_ARGS__); 
#else
# define APD_DEBUG(x, ...)
#endif

#ifdef APM_PCIE_DMA_VDEBUG
# define APD_VDEBUG(fmt, ...)		printk(KERN_DEBUG "%s: "fmt, __func__,##__VA_ARGS__); 
#else
# define APD_VDEBUG(x, ...)
#endif

#ifdef APM_PCIE_DMA_CFG_DEBUG
# define APD_CFG_DEBUG(fmt, ...)	printk(KERN_DEBUG fmt, ##__VA_ARGS__); 
#else
# define APD_CFG_DEBUG(x, ...)
#endif

# define APD_ERR(fmt, ...)		printk(KERN_ERR fmt, ##__VA_ARGS__); 

#ifndef HIDWORD
#       define MAKE_U64(h, l)           ((((u64) (h)) << 32) | (l))
#       define HIDWORD(x)               ((u32) (((u64)(x)) >> 32))
#       define LODWORD(x)               ((u32) ((x) & 0xFFFFFFFF))
#endif

#define MAX_DMA_CHAN		4
#define MAX_SRC_SG_LISTS	8192
#define MAX_DST_SG_LISTS	8192
#define CHAN_REG_SPACE		0x40
#define DESCS_PER_CHANNEL	16*8 /* Maximum 16*8 outstanding requests per channel */

enum apm_dma_state {
	CHANNEL_DISABLED = 0,
	CHANNEL_READY,
	CHANNEL_BUSY
};

enum {
	PCI_MEM = 0,
	AXI_MEM
};

#define SRC_Q_PTR_LO	0x00
#define SRC_Q_PTR_HI	0x04
#define SRC_Q_SIZE	0x08
#define SRC_Q_LIMIT	0x0C
#define DST_Q_PTR_LO	0x10
#define DST_Q_PTR_HI	0x14
#define DST_Q_SIZE	0x18
#define DST_Q_LIMIT	0x1C
#define STA_Q_PTR_LO	0x20
#define STA_Q_PTR_HI	0x24
#define STA_Q_SIZE	0x28
#define STA_Q_LIMIT	0x2C
#define SRC_Q_NEXT	0x30
#define DST_Q_NEXT	0x34
#define STA_Q_NEXT	0x38
#define DMA_CONTROL	0x3C

#define AXI_DESC_COHERENCY	(1 << 3)
#define PCI_DESC_COHERENCY	(1 << 2)

/* DMA_CONTROL Fileds */
#define DMA_ENABLE_MASK			0x00000001
#define DMA_RUNNING_MASK		0x00000002
#define DMA_RESET_MASK			0x00000004
#define DMA_CHAN_NUM_MASK		0x00003ff0
#define DMA_PRESENT_MASK		0x00008000
#define PCIE_INTERRUPT_ENABLE_MASK	0x00010000
#define PCIE_INTERRUPT_STATUS_MASK	0x00020000
#define PCIE_DMA_ERROR_MASK		0x00040000
#define AXI_INTERRUPT_ENABLE_MASK	0x01000000
#define AXI_INTERRUPT_STATUS_MASK	0x02000000
#define AXI_DMA_ERROR_MASK		0x04000000

#define DMA_IRQ_OFFSET			4
/* Hardware Management */
struct src_queue_desc {
	u64	addr;				/* Source Address */
	union {
		u32 dword;
		struct {
			u32 byte_cnt:24;
			u32 location:1;		/* 0 ==> PCIe, 1==> AXI memory*/
			u32 eop:1;		/* End of Packet */
			u32 intr:1;		/* Generate interrupt (valid if EOP == 1) */
			u32 res:1;		/* reserved */
			u32 attr:4;		/* DMA Data Read Attributes */
		}flags;
	} fields;
	u32	reserved;
}__attribute__((packed));

struct dst_queue_desc {
	u64	addr;				/* Destination Address */
	union {
		u32 dword;
		struct {
			u32 byte_cnt:24;
			u32 location:1;		/* 0 ==> PCIe, 1==> AXI memory */
			u32 res:3;		/* reserved */
			u32 attr:4;		/* DMA Data Read Attributes */
		}flags;
	}fields;
	u32	reserved;
}__attribute__((packed));

struct sts_queue_desc {
	union {
		u32 dword;
		struct {
			u32 completed:1;	/* Completed */
			u32 src_err:1;		/* Source Error Detected during DMA Operation */
			u32 dst_err:1;		/* Destination Error Detected during DMA Operation */
			u32 int_err:1;		/* Internal Error Detected during DMA Operation */
			u32 res:28;		/* reserved */
		}flags;
	}fields;
}__attribute__((packed));

/* Software Management */
struct apm_pcie_dma_desc {
	struct list_head		desc_node;
	struct dma_async_tx_descriptor	txd;
	size_t				len;
	u32				src_element_cnt;
	u32				dst_element_cnt;
	struct src_queue_desc		*src_desc;
	struct dst_queue_desc		*dst_desc;
	struct sts_queue_desc		*sts_desc;
	enum dma_status			status;
	int				timeout;
	unsigned long			flags;
};

/**
 * struct apm_dma_chan - DMA Channel representation
 * @chan: dma_chan strcture represetation for apm chan
 * @dma_slave: dma slave structure
 * @pdev: Associated PCI Device
 * @id: Channel ID
 * @state: Channel State
 * @base: Channel Base offset
 * @active_list: current active descriptors
 * @queue: current queued up descriptors
 * @free_list: current free descriptors
 * @lock: channel spinlock
 * @max_transfer_len: Maximum data transfer length
 * @descs_allocated: Total allocated IO descriptors
 * @completed: DMA cookie
 * @src_elems: Max source sg elements
 * @used_src_elems: Used source sg elements
 * @dst_elems: Max destination sg elements
 * @used_dst_elems: Used destination sg elements
 * @sts_elems: Max status sg elements
 * @used_sts_elems: Used status sg elements
 * @src_elem_addr_phys: Phys addr of src queue descriptor base
 * @src_elem_addr: Virt addr of src queue descriptor base
 * @next_src_elem: Index to next available src sg element
 * @tail_src_elem: Index to last src sg element
 * @dst_elem_addr_phys: Phys addr of dst queue descriptor base
 * @dst_elem_addr: Virt addr of dst queue descriptor base
 * @next_dst_elem: Index to next available dst sg element
 * @tail_dst_elem: Index to last dst sg element
 * @sts_elem_addr_phys: Phys addr of sts queue descriptor base
 * @sts_elem_addr: Virt addr of sts queue descriptor base
 * @next_sts_elem: Index to next available sts sg element
 * @tail_sts_elem: Index to last sts sg element
 * @transfer_cnt: Total number of requests submitted
 * @irq_cnt: Total number of IRQs received
 * @tasklet_cnt: Total number of tasklets scheduled
 * @err_cnt: Total number of errors occured
 */
struct apm_dma_chan {
	struct dma_chan         chan;
	struct dma_slave_config *dma_slave;
	struct pci_dev		*pdev;
	short			id;
	enum apm_dma_state	state;
	short			base;

	struct list_head	active_list;
	struct list_head	queue;
	struct list_head	free_list;
	spinlock_t		lock;
	size_t			max_transfer_len;
	u32			descs_allocated;
	dma_cookie_t		completed;

	/* Queue Elements */
	u32			src_elems;
	u32			used_src_elems;
	u32			dst_elems;
	u32			used_dst_elems;
	u32			sts_elems;
	u32			used_sts_elems;

	/* Queue descriptor addresses */
	dma_addr_t		src_elem_addr_phys;
	struct src_queue_desc	*src_elem_addr;
	u32			next_src_elem;
	u32			tail_src_elem;

	dma_addr_t		dst_elem_addr_phys;
	struct dst_queue_desc	*dst_elem_addr;
	u32			next_dst_elem;
	u32			tail_dst_elem;

	dma_addr_t		sts_elem_addr_phys;
	struct sts_queue_desc	*sts_elem_addr;
	u32			next_sts_elem;
	u32			tail_sts_elem;

	/* Performance monitoring and debug information */
	u64			transfer_cnt;
	u64			irq_cnt;
	u64			tasklet_cnt;
	u64			err_cnt;
};

static inline struct apm_dma_chan *to_apm_pcie_dma_chan(
		struct dma_chan *chan)
{
	return container_of(chan, struct apm_dma_chan, chan);
}

static inline struct apm_pcie_dma_desc *to_apm_pcie_dma_desc(
		struct dma_async_tx_descriptor *txd)
{
	return container_of(txd, struct apm_pcie_dma_desc, txd);
}

/**
 * struct apm_dma_port - PCIe DMA Port structure
 * @list: list of all DMA ports
 * @pdev: Associated PCI Device
 * @common: embedded struct dma_device
 * @id: Port ID
 * @num_chan: Number of active channels
 * @completion_tasklet: dma tasklet for processing interrupts
 * @chan: per channel data
 */
struct apm_dma_port {
	struct list_head	list;
	struct pci_dev		*pdev;
	struct dma_device       common;
	int			id;
	int			num_chan;
	struct tasklet_struct   completion_tasklet;
	struct apm_dma_chan	chan[MAX_DMA_CHAN];
};

static inline struct apm_dma_port *to_apm_dma_port(struct dma_device *common)
{
	return container_of(common, struct apm_dma_port, common);
}
extern int pcibios_prep_pcie_dma(struct pci_bus *);
extern void pcibios_cleanup_pcie_dma(struct pci_bus *bus);


#endif /* __APM_PCIE_DMA_H__ */
