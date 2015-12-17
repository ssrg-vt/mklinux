/**
 * AppliedMicro X-Gene SOC Queue Manager Traffic Manager Linux Source file
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * Author: Ravi Patel <rapatel@apm.com>
 *         Keyur Chudgar <kchudgar@apm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * @file xgene_qmtm_main.c
 *
 */

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/acpi.h>
#include <linux/efi.h>
#include <linux/kernel.h>
#include "xgene_qmtm_util.h"
#include "xgene_qmtm_csr.h"

#define XGENE_QMTM_DRIVER_VER	"1.0"
#define XGENE_QMTM_DRIVER_NAME	"xgene_qmtm"
#define XGENE_QMTM_DRIVER_DESC	"X-Gene QMTM driver"
#define PFX			"QMTM: "

#define RES_SIZE(r)	((r)->end - (r)->start + 1)

static struct xgene_qmtm_pdev *pdev = NULL;

/* QM raw register read/write routine */
int xgene_qmtm_wr32(u32 qmtm_ip, u32 offset, u32 data)
{
	void *addr = (u8 *)pdev->dev[qmtm_ip]->qmtm_csr_vaddr + offset;

	QMTM_WRITE_PRINT("Write addr 0x%p data 0x%08x\n", addr, data);
	writel(data, addr);

	return QMTM_OK;
}

int xgene_qmtm_rd32(u32 qmtm_ip, u32 offset, u32 *data)
{
	void *addr = (u8 *)pdev->dev[qmtm_ip]->qmtm_csr_vaddr + offset;

	*data = readl(addr);
	QMTM_READ_PRINT("Read addr 0x%p data %08X\n", addr, *data);

	return QMTM_OK;
}

void *MEMALLOC(u32 size, u32 align)
{
	return kmalloc(size, GFP_ATOMIC);
}

void MEMFREE(void *addr)
{
	return kfree(addr);
}

static void xgene_qmtm_irq_state(u32 qmtm_ip, u32 enable)
{
	u32 mask;

	mask = PBM_DEC_ERRORMASK_MASK
		| ACR_FIFO_CRITICALMASK_MASK
		| QUEUE_NOT_EMPTYMASK_MASK
		| DEQ_AXI_ERRORMASK_MASK
		| QPCORE_ACR_ERRORMASK_MASK;

	if (enable)
		mask = ~mask;

	xgene_qmtm_wr32(qmtm_ip, QM_INTERRUPTMASK_ADDR, mask);
}

static irqreturn_t xgene_qmtm_dequeue_intr(int irq, void *dev)
{
	struct xgene_qmtm_msg64 msg;
	struct xgene_qmtm_qinfo *qinfo = (struct xgene_qmtm_qinfo *)dev;
	u16 queue_id = qinfo->queue_id;
	u8  qmtm_ip = qinfo->qmtm_ip;
	int rc;

	QMTM_DEBUG_PRINT("QMTM%d PBN %d IRQ %d\n", qmtm_ip, qinfo->pbn, irq);

	rc = xgene_qmtm_pull_msg(qmtm_ip, queue_id, &msg);

	if (rc == QMTM_ERR) {
		/* Return if invalid interrupt */
		QMTM_ERROR_PRINT("QMTM%d QID %d PBN %d IRQ %d spurious\n",
			qmtm_ip, queue_id, qinfo->pbn, irq);
		return IRQ_HANDLED;
	}

	print_hex_dump(KERN_INFO, "MSG: ", DUMP_PREFIX_ADDRESS,
		16, 4, &msg, msg.msg32_1.msg16.NV ? 64 : 32, 1);

	return IRQ_HANDLED;
}

static void xgene_qmtm_dump_csr(u32 qmtm_ip)
{
	u64 paddr = pdev->dev[qmtm_ip]->qmtm_csr_paddr;
	u32 offset;
	u32 data;

	offset = CSR_QM_CONFIG_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_QM_CONFIG            0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_PBM_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_PBM                  0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_PBM_BUF_WR_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_PBM_BUF_WR           0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_PBM_BUF_RD_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_PBM_BUF_RD           0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_PBM_COAL_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_PBM_COAL             0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_PBM_CTICK0_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_PBM_CTICK0           0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = QM_INTERRUPT_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("QM_INTERRUPT             0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = QM_INTERRUPTMASK_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("QM_INTERRUPTMASK         0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_PBM_ERRINF_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_PBM_ERRINF           0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_MSGRD_ERRINF_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_MSGRD_ERRINF         0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_QM_MBOX_NE_INT_MODE_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_QM_MBOX_NE_INT_MODE  0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_QM_MBOX_NE_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_QM_MBOX_NE           0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_QM_STATS_CFG_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_QM_STATS_CFG         0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_ENQ_STATISTICS_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_ENQ_STATISTICS       0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_DEQ_STATISTICS_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_DEQ_STATISTICS       0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_FIFO_STATUS_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_FIFO_STATUS          0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_ACR_FIFO_CTRL_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_ACR_FIFO_CTRL        0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_ERRQ_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_ERRQ                 0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_QM_RAM_MARGIN_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_QM_RAM_MARGIN        0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_QM_TESTINT0_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_QM_TESTINT0          0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_QM_TESTINT1_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_QM_TESTINT1          0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_QMLITE_PBN_MAP_0_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_QMLITE_PBN_MAP_0     0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_QMLITE_PBN_MAP_1_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_QMLITE_PBN_MAP_1     0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_RECOMB_CTRL_0_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_RECOMB_CTRL_0        0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_RECOMB_CTRL_1_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_RECOMB_CTRL_1        0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_RECOMB_CTRL_2_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_RECOMB_CTRL_2        0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_QM_RECOMB_RAM_MARGIN_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_QM_RECOMB_RAM_MARGIN 0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_RECOMB_STS_0_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_RECOMB_STS_0         0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_RECOMB_STS_1_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_RECOMB_STS_1         0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_RECOMB_STS_2_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_RECOMB_STS_2         0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = RECOMB_INTERRUPT_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("RECOMB_INTERRUPT         0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = RECOMB_INTERRUPTMASK_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("RECOMB_INTERRUPTMASK     0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_DEQ_CTRL_0_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_DEQ_CTRL_0           0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_MPIC_CTRL_0_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_MPIC_CTRL_0          0x%llx -> 0x%08x\n",
		(paddr + offset), data);

	offset = CSR_MISC_CTRL_0_ADDR;
	xgene_qmtm_rd32(qmtm_ip, offset, &data);
	QMTM_PRINT("CSR_MISC_CTRL_0          0x%llx -> 0x%08x\n",
		(paddr + offset), data);
}

static int xgene_qmtm_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int xgene_qmtm_release(struct inode *inode, struct file *file)
{
	return 0;
}

#define CMD_DUMP_QM_QCONFIG	'0'
#define CMD_DUMP_QM_CSR		'1'
#define CMD_READ_QM_CSR		'2'
#define CMD_QM_QUEUE_STATE	'3'
#define CMD_SEND_QM_MSG		'4'
#define CMD_RECV_QM_MSG		'5'
#define CMD_CREATE_QM_LBQ	'6'

static void xgene_qmtm_help(void)
{
	printk(KERN_INFO "QM util help\n");
	printk(KERN_INFO "qmtm_ip should be 0, 1, 2 or 3\n");
	printk(KERN_INFO "To dump QM QConfig: echo <qmtm_ip> %c\n",
		CMD_DUMP_QM_QCONFIG);
	printk(KERN_INFO "To dump QM CSR: echo <qmtm_ip> %c\n",
		CMD_DUMP_QM_CSR);
	printk(KERN_INFO "To read QM CSR: echo <qmtm_ip> %c <addr>\n",
		CMD_READ_QM_CSR);
	printk(KERN_INFO "To dump QM queue state: echo <qmtm_ip> %c <QID>\n",
		CMD_QM_QUEUE_STATE);
	printk(KERN_INFO "To send QM queue msg: echo <qmtm_ip> %c <QID>\n",
		CMD_SEND_QM_MSG);
	printk(KERN_INFO "To receive QM queue msg: echo <qmtm_ip> %c <QID>\n",
		CMD_RECV_QM_MSG);
	printk(KERN_INFO "To create QM Loopback queue: echo <qmtm_ip> %c\n",
		CMD_CREATE_QM_LBQ);
}

static ssize_t xgene_qmtm_read(struct file *file, char __user * buf,
		size_t count, loff_t *ppos)
{
	xgene_qmtm_help();

	return 0;
}

static ssize_t xgene_qmtm_write(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	struct xgene_qmtm_qinfo qinfo;
	struct xgene_qmtm_msg64 *msg64 = (struct xgene_qmtm_msg64 *)
		"This is verbose 64 Byte loopback message sent & "
		"received by CPU.";
	int len = count;
	u32 val = 0;
	u32 reg_offset;
	u16 queue_id = 0;
	u8  qmtm_ip = *buf++ - '0';

	if (xgene_qmtm_get_dev(qmtm_ip) == NULL) {
		printk(KERN_ERR "Invalid QM IP %d\n", qmtm_ip);
		xgene_qmtm_help();
		goto _ret_qm_driver_write;
	}

	buf++; /* skip space */

	switch (*buf) {
		case CMD_DUMP_QM_QCONFIG:
			printk(KERN_INFO "Dumping QM Queue Configuration\n");
			xgene_qmtm_dump_qconfig(qmtm_ip);
			break;
		case CMD_DUMP_QM_CSR:
			printk(KERN_INFO "Dumping QM CSR Space\n");
			xgene_qmtm_dump_csr(qmtm_ip);
			break;
		case CMD_READ_QM_CSR:
			buf++; /* skip space */
			buf++; /* get reg offset */
			reg_offset = (u32) (*buf - 48);
			buf++;
			reg_offset = (reg_offset * 100) + ((int) (*buf - 48) * 10);
			buf++;
			reg_offset = reg_offset + (int) (*buf - 48);
			xgene_qmtm_rd32(qmtm_ip, reg_offset, &val);
			printk(KERN_INFO "QM register 0x%x value 0x%08X\n",
				reg_offset, val);
			break;
		case CMD_QM_QUEUE_STATE:
			buf += 2; /* Skip command and space */
			len--;
			while (len) {
				if (!(*buf >= '0' && *buf <= '9'))
					break;
				queue_id *= 10;
				queue_id += *buf - '0';
				len--;
				buf++;
			}
			printk(KERN_INFO "Reading QM queue state QID: %d\n", queue_id);
			qinfo.qmtm_ip = qmtm_ip;
			qinfo.queue_id = queue_id;
			xgene_qmtm_dump_qinfo(&qinfo);
			break;
		case CMD_SEND_QM_MSG:
			buf += 2; /* Skip command and space */
			len--;
			queue_id = 0;
			while (len) {
				if (!(*buf >= '0' && *buf <= '9'))
					break;
				queue_id *= 10;
				queue_id += *buf - '0';
				len--;
				buf++;
			}
			printk(KERN_INFO "Sending msg on queue for core 0: %d\n", queue_id);
			xgene_qmtm_push_msg_util(qmtm_ip, queue_id, msg64);
			break;
		case CMD_RECV_QM_MSG:
			buf += 2; /* Skip command and space */
			len--;
			while (len) {
				if (!(*buf >= '0' && *buf <= '9'))
					break;
				queue_id *= 10;
				queue_id += *buf - '0';
				len--;
				buf++;
			}
			printk(KERN_INFO "Receiving msg from queue: %d\n", queue_id);
			xgene_qmtm_pull_msg_util(qmtm_ip, queue_id);
			break;
		case CMD_CREATE_QM_LBQ:
			qinfo.qmtm_ip = qmtm_ip;
			xgene_qmtm_create_lpq(&qinfo);
			val = request_irq(qinfo.irq, xgene_qmtm_dequeue_intr,
				0, "QM LoopBack Interrupt",
				pdev->dev[qmtm_ip]->qinfo[qinfo.queue_id]);
			break;
		default:
			xgene_qmtm_help();
			break;
	}

_ret_qm_driver_write:
	return count;
}

static long xgene_qmtm_ioctl(struct file *file,
				u32 cmd, unsigned long arg)
{
        return 0;
}

static struct file_operations xgene_qmtm_proc_fops = {
	.owner = THIS_MODULE,
	.open = xgene_qmtm_open,
	.release = xgene_qmtm_release,
	.read = xgene_qmtm_read,
	.write = xgene_qmtm_write,
	.unlocked_ioctl = xgene_qmtm_ioctl,
};

static int xgene_qmtm_get_u32_param(struct platform_device *pdev,
				    const char *name, u32 *param)
{
	int rc;
#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT)) {
		struct acpi_dsm_entry entry;
		int val;

		if (acpi_dsm_lookup_value(ACPI_HANDLE(&pdev->dev),
					  name, 0, &entry) || !entry.value)
			return -EINVAL;
		*param = kstrtoint(entry.value, 0, &val) ? 0 : val;
		kfree(entry.key);
		kfree(entry.value);
		return 0;
	}
#endif
	rc = of_property_read_u32(pdev->dev.of_node, name, param);
	return rc;
}

static int xgene_qmtm_get_str_param(struct platform_device *pdev,
				    const char *name, char *buf, int len)
{
	const char *param;
	int rc;

	buf[0] = '\0';
#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT)) {
		struct acpi_dsm_entry entry;

		if (acpi_dsm_lookup_value(ACPI_HANDLE(&pdev->dev),
					  name, 0, &entry) || !entry.value)
			return -EINVAL;
		strncpy(buf, entry.value, len);
		buf[len - 1] = '\0';
		kfree(entry.key);
		kfree(entry.value);
		return 0;
	}
#endif
	rc = of_property_read_string(pdev->dev.of_node, name, &param);
	if (rc == 0) {
		strncpy(buf, param, len);
		buf[len - 1] = '\0';
	}
	return rc;
}

static int xgene_qmtm_get_byte_param(struct platform_device *pdev,
				     const char *name, u8 *buf, int len)
{
	u32 *tmp;
	int rc;
	int i;
#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT)) {
		struct acpi_dsm_entry entry;
		char *value_str;
		int val;

		if (acpi_dsm_lookup_value(ACPI_HANDLE(&pdev->dev),
					  name, 0, &entry) || !entry.value)
			return -EINVAL;
		value_str = entry.value;
		for (i = 0; i < len && value_str; i++) {
			sscanf(value_str, "%d", &val);
			buf[i] = val & 0xFF;
			value_str = strchr(value_str, ' ');
			if (value_str)
				value_str++;
		}
		kfree(entry.key);
		kfree(entry.value);
		return 0;
	}
#endif
	tmp = kmalloc(sizeof(u32) * len, GFP_ATOMIC);
	if (tmp == NULL)
		return -ENOMEM;
	rc = of_property_read_u32_array(pdev->dev.of_node, name, tmp, len);
	if (rc != 0)
		goto done;
	for (i = 0; i < len; i++)
		buf[i] = tmp[i] & 0xFF;

done:
	kfree(tmp);
	return 0;
}

void xgene_qmtm_request_sab_irq(u32 qmtm_ip, u32 qid, qmtm_sab_fn_ptr entry, 
				qmtm_sab_fn_ptr exit, void *data)
{
	struct qmtm_sab_info *temp = &pdev->dev[qmtm_ip]->qmtm_sab_info_table[qid];
	unsigned int sab_proc_idx = qid / 32; 
	unsigned int val;

	/* Set sab irq handler info in table */
	temp->critical_entry = entry;
	temp->critical_exit = exit;
	temp->data = data;

	/* Unmask interrupt */
	xgene_qmtm_rd32(qmtm_ip, CSR_QM_SAB_PROC0MASK_ADDR + (sab_proc_idx * 8), &val);
	val &= ~(1 << sab_proc_idx);
	xgene_qmtm_wr32(qmtm_ip, CSR_QM_SAB_PROC0MASK_ADDR + (sab_proc_idx * 8), val);

	return;
}

void xgene_qmtm_free_sab_irq(u32 qmtm_ip, u32 qid) 
{
	struct qmtm_sab_info *temp = &pdev->dev[qmtm_ip]->qmtm_sab_info_table[qid];
	unsigned int sab_proc_idx = qid / 32; 
	unsigned int sab_proc_idx_mod = qid % 32; 
	unsigned int val;

	/* Unmask interrupt */
	xgene_qmtm_rd32(qmtm_ip, CSR_QM_SAB_PROC0MASK_ADDR + (sab_proc_idx * 8), &val);
	val |= (1 << sab_proc_idx_mod);
	xgene_qmtm_wr32(qmtm_ip, CSR_QM_SAB_PROC0MASK_ADDR + (sab_proc_idx * 8), val);

	memset(temp, 0, sizeof(struct qmtm_sab_info));

	return;

}

/* SAB interrupt handler */
static irqreturn_t xgene_qmtm_sab_intr(int irq, void *dev)
{
	u32 qmtm_ip = *(u32 *)dev;
	u32 sab_proc;
	u32 proc_sab;
	int i, j;
	static unsigned int proc_sab_saved;

	xgene_qmtm_rd32(qmtm_ip, CSR_QM_SAB_PROC0_ADDR, &sab_proc);

	for (i = 0; sab_proc > 0; i++) {
		if (sab_proc & 1) {
			unsigned int state_changed;
			xgene_qmtm_rd32(qmtm_ip, CSR_PROC_SAB0_ADDR + (i * 4), &proc_sab);

			state_changed = proc_sab ^ proc_sab_saved;
			proc_sab_saved = proc_sab;

			/* Service critical region entry interrupts */
			while ((j = ffs(state_changed))) {
				unsigned int q_idx;
				struct qmtm_sab_info *info;
				/* ffs returns exact index and zero if not found, hence decrement */
				j--;
				q_idx = i * 32 + j;
				info = &(pdev->dev[qmtm_ip]->qmtm_sab_info_table[q_idx]);

				/* check the trasition into critical area */
				if (proc_sab & (1 << j)) {
					if (info->critical_entry) {
						info->critical_entry(info->data);
					}
				} else {
					if (info->critical_exit) {
						info->critical_exit(info->data);
					}
				}
				state_changed &= ~(1 << j);
			}
		}
		sab_proc = sab_proc >> 1;               
	}

	return IRQ_HANDLED;
}

static int xgene_qmtm_probe(struct platform_device *device)
{
	u64 qmtm_csr_paddr, qmtm_fabric_paddr;
	void *qmtm_csr_vaddr, *qmtm_fabric_vaddr;
	struct clk *clk;
	struct resource *res;
	int rc = QMTM_ERR, inum = 0;
	u16 error_irq, irq, pbn, sab_irq;
	u32 qmtm_max_queues;
	struct xgene_qmtm_dev *dev;
	struct xgene_qmtm_sdev *sdev;
	char name[20];
	u8 wq_pbn_start, wq_pbn_count, fq_pbn_start, fq_pbn_count;
	u32 qmtm_ip;
	u8 info[5];

#if defined(CONFIG_ACPI)
	/* Skip the ACPI probe if booting via DTS */
	if (!efi_enabled(EFI_BOOT) && device->dev.of_node == NULL)
		return -ENODEV;
#endif
	rc = xgene_qmtm_get_u32_param(device, "devid", &qmtm_ip);
	if (rc || qmtm_ip < QMTM0 || qmtm_ip > QMTM3)
		goto _ret_probe;

	/* Retrieve QM CSR register address and size */
	res = platform_get_resource(device, IORESOURCE_MEM, 0);
	if (!res) {
		printk(KERN_ERR "Failed to get QM CSR region\n");
		rc = -ENODEV;
		goto _ret_probe;
	}
	qmtm_csr_paddr = res->start;
	qmtm_csr_vaddr = devm_ioremap_resource(&device->dev, res);

	QMTM_DEBUG_PRINT("QM CSR PADDR 0x%010llx size %ld VADDR 0x%p\n",
		qmtm_csr_paddr, (unsigned long)RES_SIZE(&res),
		qmtm_csr_vaddr);

	if (!qmtm_csr_vaddr) {
		printk(KERN_ERR "Failed to ioremap QM CSR region\n");
		rc = -ENODEV;
		goto _ret_probe;
	}

	/* Retrieve Primary Fabric address and size */
	res = platform_get_resource(device, IORESOURCE_MEM, 1);
	if (!res) {
		printk(KERN_ERR "Failed to get QM Fabric region\n");
		rc = -ENODEV;
		goto _ret_probe;
	}
	qmtm_fabric_paddr = res->start;
	qmtm_fabric_vaddr = devm_ioremap_resource(&device->dev, res);

	QMTM_DEBUG_PRINT("QM Fabric PADDR 0x%010llx size %ld VADDR 0x%p\n",
		qmtm_fabric_paddr, (unsigned long)RES_SIZE(&res),
		qmtm_fabric_vaddr);

	if (!qmtm_fabric_vaddr) {
		printk(KERN_ERR "Failed to ioremap QM Fabric region\n");
		rc = -ENODEV;
		goto _ret_probe;
	}

	xgene_qmtm_get_u32_param(device, "max_queues", &qmtm_max_queues);

#ifdef CONFIG_ACPI
	if (efi_enabled(EFI_BOOT))
		clk = NULL;
	else
#endif
	{
		clk = clk_get(&device->dev, NULL);
		if (!IS_ERR(clk)) {
			rc = clk_prepare_enable(clk);
			if (rc) {
				clk_put(clk);
				dev_err(&device->dev, "clock prepare enable failed\n");
				goto _ret_probe;
			}
		} else {
			dev_err(&device->dev, "can't get clock\n");
			rc = -ENODEV;
			goto _ret_probe;
		}
	}

	if ((rc = xgene_qmtm_dev_init(qmtm_ip, qmtm_max_queues,
			qmtm_csr_paddr, qmtm_csr_vaddr,
			qmtm_fabric_paddr, qmtm_fabric_vaddr)) != QMTM_OK) {
		printk(KERN_ERR "QMTM%d device init error %d\n", qmtm_ip, rc);
		goto _ret_probe;
	}

	dev = pdev->dev[qmtm_ip];
	dev->clk = clk;
	xgene_qmtm_get_str_param(device, "slave_name", name, sizeof(name));
	xgene_qmtm_get_byte_param(device, "slave_info", info, ARRAY_SIZE(info));
	wq_pbn_start = info[1];
	wq_pbn_count = info[2];
	fq_pbn_start = info[3];
	fq_pbn_count = info[4];

	if ((sdev = xgene_qmtm_set_sdev(name, qmtm_ip,
			wq_pbn_start, wq_pbn_count,
			fq_pbn_start, fq_pbn_count)) == NULL) {
		printk(KERN_ERR "QMTM%d Slave %s error\n", qmtm_ip, name);
		rc = -ENODEV;
		xgene_qmtm_dev_exit(qmtm_ip);
		goto _ret_probe;
	}

	error_irq = platform_get_irq(device, inum);
	/* register for SAB irq */
	if (qmtm_ip != QMTM3) {
		inum++;
		sab_irq = platform_get_irq(device, inum);

		if (sab_irq) {
			memset(dev->sab_irq_s, 0, sizeof(dev->sab_irq_s));
			snprintf(dev->sab_irq_s, sizeof(dev->sab_irq_s),
					"%s_SAB", sdev->name);

			if ((rc = request_irq(sab_irq, xgene_qmtm_sab_intr,
							0, dev->sab_irq_s,
							&dev->qmtm_ip)) != QMTM_OK) {
				printk(KERN_ERR "Could not register for IRQ %d\n", sab_irq);;
			} else {
				QMTM_DEBUG_PRINT("Registered SAB IRQ %d\n", sab_irq);
				dev->qmtm_sab_irq = sab_irq;
			}
		}
	}

	for (pbn = wq_pbn_start; pbn < (wq_pbn_start + wq_pbn_count); pbn++) {
		inum++;
		irq = platform_get_irq(device, inum);
		if (irq) {
			dev->dequeue_irq[pbn] = irq;
		} else {
			printk(KERN_ERR "Failed to map QMTM%d PBN %d IRQ\n",
				qmtm_ip, pbn);
		}
	}

#ifdef CONFIG_XGENE_QMTM_ERROR
	/* Enable error reporting */
	xgene_qmtm_enable_error(qmtm_ip, error_irq, sdev->name);
#endif
        /* Enable QM hardware interrupts */
	xgene_qmtm_irq_state(qmtm_ip, 1);

_ret_probe:
	return rc;
}

static int xgene_qmtm_remove(struct platform_device *device)
{
	void *qmtm_csr_vaddr, *qmtm_fabric_vaddr;
	int rc = QMTM_ERR;
	u32 qmtm_ip;

	xgene_qmtm_get_u32_param(device, "devid", &qmtm_ip);

	if (qmtm_ip < QMTM0 || qmtm_ip > QMTM3)
		goto _ret_remove;

#ifdef CONFIG_XGENE_QMTM_ERROR
	/* Disable error reporting */
	xgene_qmtm_disable_error(qmtm_ip);
#endif

	/* Free QMTM SAB IRQ */
	if (pdev->dev[qmtm_ip]->qmtm_sab_irq) {
		free_irq(pdev->dev[qmtm_ip]->qmtm_sab_irq, &(pdev->dev[qmtm_ip]->qmtm_ip));
		pdev->dev[qmtm_ip]->qmtm_sab_irq = 0;
	}

	qmtm_csr_vaddr = pdev->dev[qmtm_ip]->qmtm_csr_vaddr;
	qmtm_fabric_vaddr = pdev->dev[qmtm_ip]->qmtm_fabric_vaddr;

	/* Disable QPcore */
	rc = xgene_qmtm_dev_exit(qmtm_ip);

	if (rc == QMTM_OK) {
		iounmap(qmtm_csr_vaddr);
		iounmap(qmtm_fabric_vaddr);
	}

_ret_remove:
	return rc;
}

static struct of_device_id xgene_qmtm_match[] = {
	{
		.name 		= "qmtm",
		.type 		= "qmtm",
		.compatible 	= "xgene,qmtm",
 	},
	{ },
};
MODULE_DEVICE_TABLE(of, xgene_qmtm_match);

#ifdef CONFIG_ACPI
static const struct acpi_device_id xgene_qmtm_acpi_ids[] = {
        { "APMC0D04", 0 },
        { }
};
MODULE_DEVICE_TABLE(acpi, xgene_qmtm_acpi_ids);
#endif

static struct  platform_driver xgene_qmtm_driver = {
	.driver = {
		.name = "qmtm",
		.owner = THIS_MODULE,
		.of_match_table = xgene_qmtm_match,
#ifdef CONFIG_ACPI
		.acpi_match_table = ACPI_PTR(xgene_qmtm_acpi_ids),
#endif
	},
	.probe  = xgene_qmtm_probe,
	.remove = xgene_qmtm_remove,
};

static int __init xgene_qmtm_init(void)
{
	int rc = QMTM_ERR;

	printk(KERN_INFO "%s v%s\n", XGENE_QMTM_DRIVER_DESC,
		XGENE_QMTM_DRIVER_VER);

	if ((pdev = xgene_qmtm_pdev_init(STORM_QMTM)) == NULL)
		goto _ret_init;

	if (!proc_create(XGENE_QMTM_DRIVER_NAME, 0, NULL, &xgene_qmtm_proc_fops))
		goto _ret_init;

	rc = platform_driver_register(&xgene_qmtm_driver);

_ret_init:
	return rc;
}

void __exit xgene_qmtm_exit(void)
{
	MEMFREE(pdev);
	remove_proc_entry(XGENE_QMTM_DRIVER_NAME, NULL);
	platform_driver_unregister(&xgene_qmtm_driver);
	printk(KERN_NOTICE PFX "Unloaded %s...\n", XGENE_QMTM_DRIVER_DESC);
}

EXPORT_SYMBOL(xgene_qmtm_request_sab_irq);
EXPORT_SYMBOL(xgene_qmtm_free_sab_irq);
EXPORT_SYMBOL(xgene_qmtm_set_pb);
EXPORT_SYMBOL(xgene_qmtm_clr_pb);
EXPORT_SYMBOL(xgene_qmtm_set_qinfo);
EXPORT_SYMBOL(xgene_qmtm_get_qinfo);
EXPORT_SYMBOL(xgene_qmtm_clr_qinfo);
EXPORT_SYMBOL(xgene_qmtm_set_sdev);
EXPORT_SYMBOL(xgene_qmtm_clr_sdev);
EXPORT_SYMBOL(xgene_qmtm_fp_dealloc_msg);
EXPORT_SYMBOL(xgene_qmtm_push_msg);
EXPORT_SYMBOL(xgene_qmtm_pull_msg);
EXPORT_SYMBOL(xgene_qmtm_intr_coalesce);

module_init(xgene_qmtm_init);
module_exit(xgene_qmtm_exit);

MODULE_VERSION(XGENE_QMTM_DRIVER_VER);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ravi Patel <rapatel@apm.com>");
MODULE_DESCRIPTION(XGENE_QMTM_DRIVER_DESC);
