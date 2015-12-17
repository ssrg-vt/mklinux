/**
 * AppliedMicro X-Gene SOC Queue Manager Traffic Manager Error Source file
 *
 * Copyright (c) 2013 Applied Micro Circuits Corporation.
 * Author: Ravi Patel <rapatel@apm.com>
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
 * @file xgene_qmtm_error.c
 *
 */

#include <linux/interrupt.h>
#include "xgene_qmtm_core.h"
#include "xgene_qmtm_csr.h"
#include <misc/xgene/qmtm/xgene_qmtm.h>

/* QMTM Error Reporting */
/* LErr(3b) Decoding */
enum xgene_qmtm_lerr {
	QMTM_NO_ERR,
	QMTM_MSG_SIZE_ERR,
	QMTM_HOP_COUNT_ERR,
	QMTM_VQ_ENQ_ERR,
	QMTM_DISABLEDQ_ENQ_ERR,
	QMTM_Q_OVERFLOW_ERR,
	QMTM_ENQUEUE_ERR,
	QMTM_DEQUEUE_ERR,
};

/* Userinfo encodings for LERR code 6 */
/* err[2:0] Encoding */
#define QMTM_AXI_READ_ERR          0	/* AXI error on read from CPU mailbox */
#define QMTM_AXI_ENQ_VQ_ERR        3	/* Alt. Enq cmd to a VQ */
#define QMTM_AXI_ENQ_DQ_ERR        4	/* Alt. Enq cmd to a Disabled Q */
#define QMTM_AXI_ENQ_OVERFLOWQ_ERR 5	/* Alt. Enq cmd overfills a Q  */

/* cmd_acr_enq_err[1:0] Encoding  */
enum xgene_qmtm_enq_err {
	QMTM_NO_AXI_ERR,
	QMTM_AXI_SLAVE_ERR,	/* AXI slave error on CPU mailbox read */
	QMTM_AXI_DECODE_ERR,	/* AXI decode error on CPU mailbox read */
};

/* Userinfo encodings for LERR code 7 */
#define QMTM_CHILD_VQ_ERR	6	/* VQ was assigned as a child of another VQ */
#define QMTM_DEQUEUE_DQ	7	/* dequeue was requested from a disabled PQ */

/*
 * Parse the exact for the Error Message received on Error Queue
 *
 */
static void xgene_qmtm_error_msg(u16 queue_id, struct xgene_qmtm_msg32 *msg)
{
	struct xgene_qmtm_msg16 *msg16 = &msg->msg16;
#ifdef QMTM_ERROR_PRINT_ENABLE
	u8 err = 0, cmd_acr_enq_err = 0, cmd_acr_enq_qid = 0, deq_slot_num = 0;
	u16 deq_slvid_pbn = 0;
#endif

	QMTM_ERROR_PRINT("QMTM Error LErr[%d] for Qid[%d] \n",
		msg16->LErr, queue_id);

	switch(msg16->LErr) {
	case QMTM_MSG_SIZE_ERR:
		QMTM_ERROR_PRINT("Msg Size Error for Enqueue on Queue %d \n",
			queue_id);
		break;
	case QMTM_HOP_COUNT_ERR:
		QMTM_ERROR_PRINT("Hop count error. QMTM received a message with"
			"a hop count of 3 for Queue %d \n",
			queue_id);
		break;
	case QMTM_VQ_ENQ_ERR:
		QMTM_ERROR_PRINT("Enqueue on Virtual Queue %d \n",
			queue_id);
		break;
	case QMTM_DISABLEDQ_ENQ_ERR:
		QMTM_ERROR_PRINT("Enqueue on disabled Queue %d \n",
			queue_id);
		break;
	case QMTM_Q_OVERFLOW_ERR:
		QMTM_ERROR_PRINT("Queue %d overflow, message send to"
			"Error Queue \n",
			queue_id);
		break;
#ifdef QMTM_ERROR_PRINT_ENABLE
	case QMTM_ENQUEUE_ERR:
		cmd_acr_enq_qid = (msg16->UserInfo & QMTM_QID_MASK);
		cmd_acr_enq_err = ((msg16->UserInfo >> 22) & 0X2);
		err = ((msg16->UserInfo >> 29) & 0X7);
		QMTM_ERROR_PRINT("Enqueue Error on Qid[%d]\n", cmd_acr_enq_qid);

		switch(err) {
		case QMTM_AXI_READ_ERR:
			QMTM_ERROR_PRINT("AXI error on read from CPU "
				"mailbox: Qid[%d]\n",
				cmd_acr_enq_qid);
			break;
		case QMTM_AXI_ENQ_VQ_ERR:
			QMTM_ERROR_PRINT("Alternate Enqueue Command to a"
				"Virtual Queue: Qid[%d]\n",
				cmd_acr_enq_qid);
			break;
		case QMTM_AXI_ENQ_DQ_ERR:
			QMTM_ERROR_PRINT("Alternate Enqueue Command to a"
				"Disabled Queue: Qid[%d]\n",
				cmd_acr_enq_qid);
			break;
		case QMTM_AXI_ENQ_OVERFLOWQ_ERR:
			QMTM_ERROR_PRINT("Alternate Enqueue Command "
				"overfills Queue: Qid[%d]\n",
				cmd_acr_enq_qid);
			break;
		}

		if (cmd_acr_enq_err == QMTM_AXI_SLAVE_ERR)
			QMTM_ERROR_PRINT("AXI slave error on CPU mailbox"
				"read: Qid[%d]\n",
				cmd_acr_enq_qid);
		else if (cmd_acr_enq_err == QMTM_AXI_SLAVE_ERR)
			QMTM_ERROR_PRINT("AXI decode error on CPU mailbox"
				"read: Qid[%d]\n",
				cmd_acr_enq_qid);
		break;

	case QMTM_DEQUEUE_ERR:
		err = ((msg16->UserInfo >> 29) & 0X7);
		deq_slvid_pbn = ((msg16->UserInfo >> 3) & 0XFF3);
		deq_slot_num = (msg16->UserInfo & 0x7);
		QMTM_ERROR_PRINT("Dequeue Error for deq_slot_num :%d and \n"
			"deq_slvid_pbn: %d",
			deq_slot_num, deq_slvid_pbn);

		if (err ==  QMTM_CHILD_VQ_ERR)
			QMTM_ERROR_PRINT("VQ was assigned as a child of another"
				"VQ, deq_slot_num :%d and \n"
				"deq_slvid_pbn: %d",
				deq_slot_num, deq_slvid_pbn);
		else if (err == QMTM_DEQUEUE_DQ)
			QMTM_ERROR_PRINT("A dequeue was requested from a"
				"disabled PQ, deq_slot_num :%d and \n"
				"deq_slvid_pbn: %d",
				deq_slot_num, deq_slvid_pbn);
		break;
#else
	case QMTM_ENQUEUE_ERR:
	case QMTM_DEQUEUE_ERR:
		break;
#endif
	default:
		QMTM_ERROR_PRINT("Unknown Error \n");
		break;
	}
}

static void xgene_qmtm_error(u8 qmtm_ip)
{
	u32 qstate[8];
	u32 status;
	u32 pbm_err;
	u32 msgrd_err;

	xgene_qmtm_rd32(qmtm_ip, QM_INTERRUPT_ADDR, &status);
	QMTM_ERROR_PRINT("QMTM error interrupt status 0x%08X\n", status);

	xgene_qmtm_rd32(qmtm_ip, CSR_PBM_ERRINF_ADDR, &pbm_err);
	QMTM_ERROR_PRINT("QMTM CSR PBM ERRINF (0x%X) value 0x%08X\n",
		CSR_PBM_ERRINF_ADDR, pbm_err);

	xgene_qmtm_rd32(qmtm_ip, CSR_MSGRD_ERRINF_ADDR, &msgrd_err);
	QMTM_ERROR_PRINT("QMTM CSR MSGRD ERRINF (0x%X) value 0x%08X\n",
		CSR_MSGRD_ERRINF_ADDR, msgrd_err);

	QMTM_ERROR_PRINT("DEQ QID %d\n", QID_RD(msgrd_err));
	memset(qstate, 0, sizeof(qstate));
	xgene_qmtm_read_qstate(qmtm_ip, QID_RD(msgrd_err), qstate);
	print_hex_dump(KERN_INFO, "DEQSTATE ", DUMP_PREFIX_ADDRESS, 16, 4,
		qstate, sizeof(qstate), 1);

	QMTM_ERROR_PRINT("ENQ QID %d\n", ACR_QID_RD(msgrd_err));
	memset(qstate, 0, sizeof(qstate));
	xgene_qmtm_read_qstate(qmtm_ip, ACR_QID_RD(msgrd_err), qstate);
	print_hex_dump(KERN_INFO, "ENQSTATE ", DUMP_PREFIX_ADDRESS, 16, 4,
		qstate, sizeof(qstate), 1);

	xgene_qmtm_wr32(qmtm_ip, QM_INTERRUPT_ADDR, status);
}

static irqreturn_t xgene_qmtm_error_intr(int irq, void *dev)
{
	xgene_qmtm_error((u8)(*(u32 *)dev));

	return IRQ_HANDLED;
}

static irqreturn_t xgene_qmtm_error_queue_intr(int irq, void *dev)
{
	struct xgene_qmtm_msg64 msg;
	struct xgene_qmtm_qinfo *qinfo = (struct xgene_qmtm_qinfo *)dev;
	u16 queue_id = qinfo->queue_id;
	u8 qmtm_ip = qinfo->qmtm_ip;
	int rc;

	QMTM_DEBUG_PRINT("QMTM%d PBN %d IRQ %d\n", qmtm_ip, qinfo->pbn, irq);

	rc = xgene_qmtm_pull_msg(qmtm_ip, queue_id, &msg);

	if (rc == QMTM_ERR) {
		/* Return if invalid interrupt */
		QMTM_ERROR_PRINT("QMTM%d QID %d PBN %d IRQ %d spurious\n",
			qmtm_ip, queue_id, qinfo->pbn, irq);
		return IRQ_HANDLED;
	}

	xgene_qmtm_error(qmtm_ip);
	QMTM_PRINT("QMTM%d Error: QID %d\n", qmtm_ip, queue_id);
	print_hex_dump(KERN_INFO, "Err q MSG: ", DUMP_PREFIX_ADDRESS,
			16, 4, &msg, msg.msg32_1.msg16.NV ? 64 : 32, 1);
	xgene_qmtm_error_msg(queue_id, &msg.msg32_1);

	return IRQ_HANDLED;
}

int xgene_qmtm_enable_error(u8 qmtm_ip, u16 irq, char *name)
{
	int rc = QMTM_OK;
	struct xgene_qmtm_dev *dev = xgene_qmtm_get_dev(qmtm_ip);
	struct xgene_qmtm_qinfo qinfo;
	u32 val;

	if (irq) {
#ifdef CONFIG_ARCH_MSLIM
		u32 flags = IRQF_SHARED;
#else
		u32 flags = 0;
#endif
		memset(dev->error_irq_s, 0, sizeof(dev->error_irq_s));
		snprintf(dev->error_irq_s, sizeof(dev->error_irq_s),
			"%s_Err", name);

		if ((rc = request_irq(irq, xgene_qmtm_error_intr,
				flags, dev->error_irq_s,
				&dev->qmtm_ip)) != QMTM_OK) {
			printk(KERN_ERR "Could not register for IRQ %d\n", irq);
			goto _ret_enable_error;
		}

		QMTM_DEBUG_PRINT("Registered error IRQ %d\n", irq);
		dev->qmtm_error_irq = irq;
	}

	if (qmtm_ip == QMTM3)
		goto _ret_enable_error;

	memset(&qinfo, 0, sizeof(qinfo));
#ifdef CONFIG_ARCH_MSLIM
	qinfo.slave = SLAVE_MSLIM(qmtm_ip);
	qinfo.qaccess = QACCESS_QMI;
#else
	qinfo.slave = SLAVE_CPU(qmtm_ip);
	qinfo.qaccess = QACCESS_ALT;
#endif
	qinfo.qtype = QTYPE_PQ;
	qinfo.qsize = QSIZE_2KB;
	qinfo.flags = XGENE_SLAVE_DEFAULT_FLAGS;

	/* create error queue */
	QMTM_DEBUG_PRINT("Configure QMTM error queue\n");

	if ((rc = xgene_qmtm_set_qinfo(&qinfo)) != QMTM_OK) {
		QMTM_PRINT("QMTM %d unable to configure error queue\n",
			qmtm_ip);
		goto _free_irq;
	}

	dev->error_qinfo = dev->qinfo[qinfo.queue_id];
	memset(dev->error_queue_irq_s, 0, sizeof(dev->error_queue_irq_s));
	snprintf(dev->error_queue_irq_s, sizeof(dev->error_queue_irq_s),
		"%s_ErQ", name);

	if ((rc = request_irq(qinfo.irq, xgene_qmtm_error_queue_intr,
			0, dev->error_queue_irq_s,
			dev->error_qinfo)) != QMTM_OK) {
		QMTM_PRINT("QMTM %d unable to reuest_irq %d\n",
			qmtm_ip, qinfo.irq);
		goto _clr_qinfo;
	}

	val = 0;
	val = UNEXPECTED_EN_SET(val, 1);
	val = UNEXPECTED_QID_SET(val, qinfo.queue_id);
	val = EXPECTED_EN_SET(val, 1);
	val = EXPECTED_QID_SET(val, qinfo.queue_id);
	return xgene_qmtm_wr32(qmtm_ip, CSR_ERRQ_ADDR, val);

_clr_qinfo:
	xgene_qmtm_clr_qinfo(&qinfo);
	dev->error_qinfo = NULL;

_free_irq:
	if (irq)
		free_irq(irq, &dev->qmtm_ip);

_ret_enable_error:
	return rc;
}

void xgene_qmtm_disable_error(u8 qmtm_ip)
{
	struct xgene_qmtm_dev *dev = xgene_qmtm_get_dev(qmtm_ip);
	struct xgene_qmtm_qinfo *error_qinfo = dev->error_qinfo;

	/* Free QMTM Error IRQ */
	if (dev->qmtm_error_irq) {
		free_irq(dev->qmtm_error_irq, &dev->qmtm_ip);
		dev->qmtm_error_irq = 0;
	}

	if (error_qinfo) {
		struct xgene_qmtm_qinfo qinfo;

		/* Free QMTM Error Queue IRQ */
		free_irq(error_qinfo->irq, error_qinfo);

		/* Delete error queue */
		qinfo.qmtm_ip = error_qinfo->qmtm_ip;
		qinfo.queue_id = error_qinfo->queue_id;
		QMTM_DEBUG_PRINT("Unconfigure QMTM error queue\n");
		xgene_qmtm_clr_qinfo(&qinfo);
		dev->error_qinfo = NULL;

		/* Unassign error queue */
		xgene_qmtm_wr32(qmtm_ip, CSR_QM_CONFIG_ADDR, 0);
		xgene_qmtm_wr32(qmtm_ip, CSR_ERRQ_ADDR, 0);
	}
}
