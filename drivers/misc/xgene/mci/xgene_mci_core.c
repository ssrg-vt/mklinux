/*
 * AppliedMicro X-Gene MCI Interface
 *
 * Copyright (c) 2014 Applied Micro Circuits Corporation.
 * Author: 	Hoan Tran <hotran@apm.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/io.h>
#include <linux/export.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <asm/io.h>
#include <linux/string.h>
#include <misc/xgene/mci/xgene_mci_sysfs.h>
#include <misc/xgene/mci/xgene_mci_core.h>

int xgene_mci_send_msg(struct xgene_mci_channel *chan, int cmd, int ctl, int param, void *buffer,
		int len);

struct xgene_mci_client* xgene_mci_get_client(struct xgene_mci *mci, int port)
{
	if (port >= XGENE_MCI_MAX_CLIENT)
		return NULL ;
	return mci->clients[port];
}

static int xgene_mci_client_add(struct xgene_mci *mci, struct xgene_mci_client *mci_client)
{
	int i;

	for (i = 0; i < XGENE_MCI_MAX_CLIENT; i++) {
		if (mci->clients[i] == NULL ) {
			mci->clients[i] = mci_client;
			mci->clients[i]->port = i;
			return i; /* Return Port number*/
		}
	}

	return MCI_OUTOFRESOURCE;
}

static int xgene_mci_client_remove(struct xgene_mci *mci, int port)
{
	if (port > XGENE_MCI_MAX_CLIENT)
		return MCI_OUTOFRESOURCE;

	mci->clients[port] = NULL;

	return MCI_OK;
}

int xgene_mci_open(char name[8], void* context, int flags,
		int (*mci_status_cb)(int port, void *ctx, int status),
		int (*mci_rcv_notify_cb)(int port, void *ctx, void *buffer, int len))
{
	struct xgene_mci_client *mci_client;
	struct xgene_mci *mci;
	int rc = MCI_OK;

	mci = xgene_mci_get_dev();
	if (mci->enable == 0) {
		return MCI_CONNECTION_CLOSED;
	}

	spin_lock(&mci->lock);
	mci_client = devm_kzalloc(mci->dev, sizeof(struct xgene_mci_client), GFP_ATOMIC );
	BUG_ON(mci_client == NULL);

	mci_client->port = -1;
	memcpy(mci_client->name, name, 8);
	mci_client->context = context;
	mci_client->flag = flags;
	mci_client->mci_client_rcv_cb_fn = mci_rcv_notify_cb;
	mci_client->mci_client_status_cb_fn = mci_status_cb;


	if (mci->client_ops->open != NULL ) {
		rc = mci->client_ops->open(mci_client);
	} else {
		rc = xgene_mci_client_add(mci, mci_client);
		if (rc < 0)
			goto err1;

		mci_client->port = rc;
		dev_dbg(mci->dev, "Opening client %s, port %d\r\n", name, mci_client->port);

		/* Notify to MSLIM */
		rc = xgene_mci_send_msg(&mci->channels[MCI_HIGH_PRIORITY],
				(MSLIM_CLIENT_MSG + mci_client->port), MCI_CLIENT_OPEN, 0,
				(void *) name, 8);

		if (rc != MCI_CONNECTION_ESTABLISHED) {
			goto err1;
		}

		rc = mci_client->port;
	}

	spin_unlock(&mci->lock);
	return rc;
err1:
	spin_unlock(&mci->lock);
	devm_kfree(mci->dev, mci_client);
	xgene_mci_client_remove(mci, mci_client->port);
	return rc;
}

int xgene_mci_close(int port)
{
	int rc = MCI_OK;
	struct xgene_mci *mci;

	mci = xgene_mci_get_dev();
	if (mci->enable == 0) {
		return MCI_CONNECTION_CLOSED;
	}

	spin_lock(&mci->lock);

	if (port > XGENE_MCI_MAX_CLIENT) {
		rc = MCI_OUTOFRESOURCE;
		goto ret;
	}

	rc = xgene_mci_send_msg(&mci->channels[MCI_HIGH_PRIORITY], (MSLIM_CLIENT_MSG + port),
			MCI_CLIENT_CLOSE, 0, (void *) NULL, 0);

	if (rc != MCI_CONNECTION_CLOSED) {
		goto ret;
	}

	devm_kfree(mci->dev, mci->clients[port]);
	xgene_mci_client_remove(mci, port);
ret:
	spin_unlock(&mci->lock);
	return rc;
}

int xgene_mci_send(int port, void *buffer, int len)
{
	int rc = MCI_OK;
	int i = 0;
	int type;
	struct xgene_mci *mci;

	mci = xgene_mci_get_dev();
	if (mci->enable == 0) {
		return MCI_CONNECTION_CLOSED;
	}

	spin_lock(&mci->lock);

	if (port > XGENE_MCI_MAX_CLIENT) {
		rc = MCI_OUTOFRESOURCE;
		goto ret;
	}
	if (mci->clients[port] == NULL ) {
		rc = MCI_NO_SUPPORT_DRIVER;
		goto ret;
	}
	if (len > XGENE_MCI_BUFF_LEN) {
		rc = MCI_OUTOFRESOURCE;
		goto ret;
	}

	switch (mci->clients[port]->flag) {
	case MCI_POLLING:
		type = MCI_HIGH_PRIORITY;
		break;
	case MCI_ASYNC:
		type = MCI_LOW_PRIORITY;
		break;
	case MCI_FREEPOOL:
	default:
		rc = MCI_EAGAIN;
		goto ret;
	}

	rc = xgene_mci_send_msg(&mci->channels[type], (MSLIM_CLIENT_MSG + port),
			MCI_CLIENT_DATA, 0, (void *) (buffer + XGENE_MCI_DATA_MAX * i),
			len);
ret:
	spin_unlock(&mci->lock);
	return rc;
}

EXPORT_SYMBOL(xgene_mci_open);
EXPORT_SYMBOL(xgene_mci_close);
EXPORT_SYMBOL(xgene_mci_send);

struct xgene_mci_msg_desc *xgene_mci_receive_msg(
		struct xgene_mci_channel *chan,
		struct xgene_mci_msg64 *msg)
{
	struct xgene_mci_msg_desc *mci_msg;
	struct xgene_mci *mci = container_of(chan,
			struct xgene_mci, channels[chan->ID]);
	u32 *buf;

	mci_msg = kzalloc(sizeof(struct xgene_mci_msg_desc), GFP_ATOMIC);
	BUG_ON (mci_msg == NULL);

	/* Check message error */
	if (msg->msg32.msg16.LErr || msg->msg32.msg16.ELErr) {
		/*TODO: Return Msg Error Code */
		return NULL;
	}

	/* Parse message */
	mci_msg->Tag = msg->msg32.msg16.Tag;
	mci_msg->control = msg->msg32.msg16.Control;
	mci_msg->param0 = msg->msg32.DataWord[0];
	mci_msg->param1 = msg->msg32.DataWord[1];
	mci_msg->param2 = msg->msg32.DataWord[2];
	mci_msg->param3 = msg->msg32.DataWord[3];
	/* If buffer transfer, store PHY address */
	if (msg->msg32.msg16.DataAddr) {
		mci_msg->buf = msg->msg32.msg16.DataAddr;
	}
	mci_msg->buf_length = msg->msg32.msg16.BufDataLen;

	/* If Msg payload transfer, copy DATA into client buffer */
	if ((mci_msg->buf_length != 0) && (mci_msg->buf == 0 )) {
		buf = kzalloc(mci_msg->buf_length, GFP_ATOMIC);
		memcpy(buf, &msg->msg32.DataWord[0], mci_msg->buf_length);
		mci_msg->buf = (u64)buf;
	}

	dev_dbg(mci->dev,"Tag: 0x%x\n", mci_msg->Tag);
	dev_dbg(mci->dev,"Control: 0x%x\n", mci_msg->control);
	dev_dbg(mci->dev,"Param0: 0x%x\n", mci_msg->param0);
	dev_dbg(mci->dev,"Param1: 0x%x\n", mci_msg->param1);
	dev_dbg(mci->dev,"Param2: 0x%x\n", mci_msg->param2);
	dev_dbg(mci->dev,"Param3: 0x%x\n", mci_msg->param3);
	dev_dbg(mci->dev,"Buf: 0x%p\n", (void *)mci_msg->buf);
	dev_dbg(mci->dev,"Length: 0x%x\n", mci_msg->buf_length);

	return mci_msg;
}

static int __xgene_mci_send_msg(struct xgene_mci_channel *chan, struct xgene_mci_msg_desc *mci_msg)
{
	struct xgene_mci *mci = container_of(chan,
			struct xgene_mci, channels[chan->ID]);
	struct xgene_mci_msg64 *msg;
	struct xgene_mci_msg32 *msg32;
	int rc = 0;
	int client_port;

	/* Assign message into current channel*/
	chan->msg_desc = mci_msg;

	/* Prepare a QM message */
	msg = kzalloc(sizeof(struct xgene_mci_msg64), GFP_ATOMIC);
	BUG_ON(msg == NULL);

	msg32 = &msg->msg32;

	msg32->msg16.C = 1; /* Coherent IO */
	msg32->msg16.RType = APM_QM_MCI_RTYPE;
	msg32->msg16.LEI = 0;
	msg32->msg16.LErr = 0;
	msg32->msg16.ELErr = 0;
	msg32->msg16.NV = 0;    // 32-byte message

	msg32->msg16.Tag = mci_msg->Tag;
	switch (mci_msg->Tag) {
	case MSLIM_OK_MSG:
		dev_dbg(mci->dev,"MCI%d sends OK_MSG\n", chan->ID);
		/* Count for Message */
		msg32->DataWord[0] = mci_msg->param0;
		break;
	case MSLIM_PING_MSG:
		dev_dbg(mci->dev,"MCI%d sends MSLIM_PING_MSG\n", chan->ID);
		/* Optional */
		msg32->msg16.Control = mci_msg->control;
		/* High Priority Rx Queue*/
		msg32->DataWord[0] = mci->channels[0].rx_q_desc.qid;
		/* Low Priority Rx Queue*/
		msg32->DataWord[1] = mci->channels[1].rx_q_desc.qid;
		/* High Priority Tx Queue*/
		msg32->DataWord[2] = mci->channels[0].tx_q_desc.pbn;
		/* Low Priority Tx Queue*/
		msg32->DataWord[3] = mci->channels[1].tx_q_desc.pbn;
		break;
	case MSLIM_ERROR_MSG:
		dev_dbg(mci->dev,"MCI%d sends MSLIM_ERROR_MSG\n", chan->ID);
		msg32->DataWord[0] = mci_msg->param0;
		break;
	case MSLIM_LOG_MSG:
		dev_dbg(mci->dev,"MCI%d sends MSLIM_LOG_MSG\n", chan->ID);
		msg32->msg16.BufDataLen =
				(mci_msg->buf_length > 16) ? 16 : mci_msg->buf_length;
		/* Max Log Message = 16 bytes */
		memcpy(&msg32->DataWord[0], (void *)mci_msg->buf,
				(mci_msg->buf_length > 16) ? 16 : mci_msg->buf_length);
		break;
	case MSLIM_BUF_MSG:
		dev_dbg(mci->dev,"MCI%d sends MSLIM_BUF_MSG\n", chan->ID);
		msg32->msg16.Control = mci_msg->control;
		break;
	case MSLIM_CLIENT_MSG ... (MSLIM_CLIENT_MSG + XGENE_MCI_MAX_CLIENT):
		client_port = mci_msg->Tag - MSLIM_CLIENT_MSG;
		dev_dbg(mci->dev,"MCI%d sends msg at port %d\n", chan->ID, client_port);
		msg32->msg16.BufDataLen = mci_msg->buf_length;
		switch (mci_msg->control) {
		case MCI_CLIENT_ACK:
			if (mci->clients[client_port]) {
				mci->clients[client_port]->tx_data_ack_msg_count++;
			}
			msg32->msg16.Control = mci_msg->control;
			msg32->DataWord[0] = mci_msg->param0;
			break;
		case MCI_CLIENT_CLOSE:
			msg32->msg16.Control = mci_msg->control;
			break;
		case MCI_CLIENT_OPEN:
			msg32->msg16.Control = mci_msg->control;
			memcpy(&msg32->DataWord[0], (void *)mci_msg->buf, mci_msg->buf_length);
			break;
		case MCI_CLIENT_DATA:
			if (mci->clients[client_port]) {
				mci->clients[client_port]->tx_data_msg_count++;
			}
			msg32->msg16.Control = mci_msg->control;
			if (mci_msg->buf_length <= 16) {
				memcpy(&msg32->DataWord[0], (void *)mci_msg->buf, mci_msg->buf_length);
			} else if (mci_msg->buf_length <= XGENE_MCI_DATA_MAX) {
				msg32->msg16.NV = 1;    // 64-byte message
				memcpy(&msg32->DataWord[0], (void *)mci_msg->buf, 16);
				memcpy(&msg->DataWord[0], (void *)(mci_msg->buf + 16 / 4),
						(mci_msg->buf_length - 16));
			} else {
				/* Buffer command with DMA */
				msg32->msg16.DataAddr =
						mci->ops->virt_to_phys( chan,
								(void *)mci_msg->buf,
								(size_t)mci_msg->buf_length);
			}
			break;
		default:
			rc = -1;
			goto err;
		}
		break;
	default:
		rc = -1;
		goto err;
	}

	/* Send message */
	mci->ops->tx_msg(chan, (struct xgene_qmtm_msg64 *)msg);

err:
	kfree(msg);
	return rc;
}

int xgene_mci_send_msg(struct xgene_mci_channel *chan, int cmd,
		int ctl, int param, void *buffer, int len)
{
	struct xgene_mci_msg_desc *mci_msg;
	struct xgene_mci *mci = container_of(chan, struct xgene_mci,channels[chan->ID]);
	int rc = MCI_OK;
	unsigned long flags;

	spin_lock_irqsave(&chan->lock, flags);
	mci_msg = kzalloc(sizeof(struct xgene_mci_msg_desc), GFP_ATOMIC);
	BUG_ON (mci_msg == NULL );

	mci_msg->ID = chan->ID;
	mci_msg->Tag = cmd;
	mci_msg->control = ctl;
	mci_msg->param0 = param;
	mci_msg->buf = (u64)buffer;
	mci_msg->buf_length = len;

	rc = __xgene_mci_send_msg(chan, mci_msg);
	/* High Priority: Wait for Message's reply */
	if(chan->ID == MCI_HIGH_PRIORITY) {
		if(mci->ops->wait_for_msg_complete != NULL) {
			spin_unlock_irqrestore(&chan->lock, flags);
			rc = mci->ops->wait_for_msg_complete(chan);
			spin_lock_irqsave(&chan->lock, flags);
		}
	}

	kfree(mci_msg);
	spin_unlock_irqrestore(&chan->lock, flags);
	return rc;
}

static int xgene_mci_process_msg_client(struct xgene_mci_channel *chan,
		struct xgene_mci_msg_desc *mci_msg)
{
	struct xgene_mci *mci = container_of(chan,
			struct xgene_mci, channels[chan->ID]);
	int client_port = mci_msg->Tag - MSLIM_CLIENT_MSG;
	unsigned long flags;
	int data_type = MCI_CLIENT_DATA_LOCAL_MEM;

	spin_lock_irqsave(&chan->lock, flags);
	/* Check sub commands*/
	switch (mci_msg->control) {
	case MCI_CLIENT_OPEN:
		dev_dbg(mci->dev," MCI_CLIENT_OPEN\n");
		spin_unlock_irqrestore(&chan->lock, flags);
		if(mci->client_ops->rx_open_cb != NULL)
			mci->client_ops->rx_open_cb(chan, client_port, mci_msg);
		break;
	case MCI_CLIENT_CLOSE:
		dev_dbg(mci->dev," MCI_CLIENT_CLOSE\n");
		if (mci->clients[client_port]) {
			if (mci->clients[client_port]->mci_client_status_cb_fn) {
				mci->clients[client_port]->mci_client_status_cb_fn(
						client_port, NULL, MCI_CONNECTION_CLOSED);
			}
		}
		xgene_mci_client_remove(mci, client_port);
		spin_unlock_irqrestore(&chan->lock, flags);
		if(mci->client_ops->rx_close_cb != NULL)
			mci->client_ops->rx_close_cb(chan, client_port, mci_msg);
		break;
	case MCI_CLIENT_ACK:
		dev_dbg(mci->dev," MCI_CLIENT_ACK\n");
		spin_unlock_irqrestore(&chan->lock, flags);
		if (mci->clients[client_port]) {
			mci->clients[client_port]->rx_data_ack_msg_count++;
			if (mci->clients[client_port]->mci_client_status_cb_fn) {
				mci->clients[client_port]->mci_client_status_cb_fn(
						client_port, NULL, MSLIM_OK_MSG);
			}
		}
		if (chan->msg_desc != NULL )
			chan->msg_desc->rvalue = mci_msg->param0;
		break;
	case MCI_CLIENT_DATA:
		dev_dbg(mci->dev," MCI_CLIENT_DATA\n");
		/* Convert PHY to VIRT address */
		if ((mci_msg->buf != 0)
				&& (mci_msg->buf_length > XGENE_MCI_DATA_MAX)) {
			mci_msg->buf = mci->ops->phys_to_virt(chan,
					(void *)mci_msg->buf,
					(size_t)mci_msg->buf_length);
			data_type = MCI_CLIENT_DATA_REMOTE_MEM;
		}
		if (mci->clients[client_port]) {
			if (mci->clients[client_port]->mci_client_rcv_cb_fn) {
				mci->clients[client_port]->mci_client_rcv_cb_fn(client_port,
						&data_type, (void *)mci_msg->buf, mci_msg->buf_length);
			}
			mci->clients[client_port]->rx_data_msg_count++;
			/* Send ACK message */
			spin_unlock_irqrestore(&chan->lock, flags);
			xgene_mci_send_msg(chan, MSLIM_CLIENT_MSG + client_port,
					MCI_CLIENT_ACK, MCI_OK, (void *) NULL, 0);
		} else {
			/* Send ACK message */
			spin_unlock_irqrestore(&chan->lock, flags);
			xgene_mci_send_msg(chan, MSLIM_CLIENT_MSG + client_port,
					MCI_NO_SUPPORT_DRIVER, MCI_OK, (void *) NULL, 0);
		}
		break;
	default:
		spin_unlock_irqrestore(&chan->lock, flags);
		return -1;
	}

	return MCI_OK;
}

int xgene_mci_process_msg(struct xgene_mci_channel *chan,
		struct xgene_mci_msg_desc *mci_msg)
{
	struct xgene_mci *mci = container_of(chan,
			struct xgene_mci, channels[chan->ID]);
	struct xgene_mci_msg_desc_ll *node;
	int client_port;
	int rc = 0;

	/* Decode message */
	switch (mci_msg->Tag) {
	case MSLIM_OK_MSG:
		dev_dbg(mci->dev,"MCI%d receives OK_MSG\n", chan->ID);
		if ((chan->msg_desc != NULL ) && (chan->ID == MCI_HIGH_PRIORITY)){
			chan->msg_desc->rvalue = MCI_OK;
		}
		break;
	case MSLIM_PING_MSG:
		dev_dbg(mci->dev,"MCI%d receives MSLIM_PING_MSG\n", chan->ID);
		if(mci->ops->rx_ping_msg_cb != NULL)
			mci->ops->rx_ping_msg_cb(chan, mci_msg);
		break;
	case MSLIM_ERROR_MSG:
		dev_dbg(mci->dev,"MCI%d receives MSLIM_ERROR_MSG\n", chan->ID);
		if(chan->msg_desc == NULL) {
			rc = -1;
			break;
		} else
			chan->msg_desc->rvalue = mci_msg->param0;
		break;
	case MSLIM_LOG_MSG:
		dev_dbg(mci->dev,"MCI%d receives MSLIM_LOG_MSG\n", chan->ID);
		break;
	case MSLIM_BUF_MSG:
		dev_dbg(mci->dev,"MCI%d receives MSLIM_BUF_MSG\n", chan->ID);
		/* Check sub commands*/
		switch (mci_msg->control) {
		case MCI_BUF_REQ:
			dev_dbg(mci->dev," MCI_BUF_REQ\n");
			break;
		case MCI_BUF_AVAIL:
			dev_dbg(mci->dev," MCI_BUF_AVAIL\n");
			break;
		case MCI_BUF_MSG:
			dev_dbg(mci->dev," MCI_BUF_MSG\n");
			break;
		case MCI_BUF_FREE_POOL_Q:
			dev_dbg(mci->dev," MCI_BUF_FREE_POOL_Q\n");
			break;
		default:
			rc = -1;
			break;
		}
		break;
	case MSLIM_CLIENT_MSG ... (MSLIM_CLIENT_MSG + XGENE_MCI_MAX_CLIENT):
		client_port = mci_msg->Tag - MSLIM_CLIENT_MSG;
		dev_dbg(mci->dev,"MCI%d receives msg at port %d\n", chan->ID, client_port);
		if (chan->ID == MCI_LOW_PRIORITY) {
			/* Only ASYNC message can jump into here*/
			node = kzalloc (sizeof(struct xgene_mci_msg_desc_ll), GFP_ATOMIC);
			BUG_ON(node == NULL);
			node->msg = mci_msg;
			node->flag = 0;
			list_add_tail(&node->node, &chan->rx_q_desc.msg_ll.node);
			cancel_delayed_work(&chan->rxwork);
			schedule_delayed_work(&chan->rxwork, 10);
			goto ret;
		} else {
			rc = xgene_mci_process_msg_client(chan, mci_msg);
		}
		break;
	default:
		break;
	}

	kfree(mci_msg);
ret:
	return rc;
}

int xgene_mci_send_utils(int index, int sub)
{
	char buffer[] = "MCI driver testing \r\n";
	struct xgene_mci *mci;

	mci = xgene_mci_get_dev();
	if (mci->enable == 0) {
		return MCI_CONNECTION_CLOSED;
	}

	switch (index) {
	case MSLIM_OK_MSG:
		xgene_mci_send_msg(&mci->channels[0],
				MSLIM_OK_MSG, 0, 0, (void *) NULL, 0);
		dev_dbg(mci->dev,"MCI sends MSLIM_OK_MSG\n");
		break;
	case MSLIM_PING_MSG:
		xgene_mci_send_msg(&mci->channels[0],
				MSLIM_PING_MSG, 0, 0, (void *) NULL, 0);
		dev_dbg(mci->dev,"MCI sends MSLIM_PING_MSG\n");
		break;
	case MSLIM_ERROR_MSG:
		xgene_mci_send_msg(&mci->channels[0],
				MSLIM_ERROR_MSG, 0, MCI_CONNECTION_INUSE, (void *) NULL, 0);
		dev_dbg(mci->dev,"MCI sends MSLIM_ERROR_MSG\n");
		break;
	case MSLIM_LOG_MSG:
		xgene_mci_send_msg(&mci->channels[0],
				MSLIM_LOG_MSG, 0, 0, (void *) buffer, sizeof(buffer));
		dev_dbg(mci->dev,"MCI sends MSLIM_LOG_MSG\n");
		break;
	case MSLIM_CLIENT_MSG:
		xgene_mci_send_msg(&mci->channels[0],
				MSLIM_CLIENT_MSG, sub, 0, (void *) buffer, sizeof(buffer));
		dev_dbg(mci->dev,"MCI sends MSLIM_CLIENT_MSG\n");
		break;
	default:
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(xgene_mci_send_utils);

void xgene_mci_rx_work(struct work_struct *work)
{
	struct delayed_work *rxwork = container_of(work, struct delayed_work,
			work);
    struct xgene_mci_channel *chan = container_of(rxwork,
    		struct xgene_mci_channel, rxwork);
	struct xgene_mci_msg_desc_ll *node;
	struct list_head *pos, *q;
	unsigned long flags;

	spin_lock_irqsave(&chan->lock, flags);
	list_for_each_safe(pos, q, &chan->rx_q_desc.msg_ll.node) {
		node = list_entry(pos, struct xgene_mci_msg_desc_ll, node);
		list_del(pos);
		spin_unlock_irqrestore(&chan->lock, flags);
		xgene_mci_process_msg_client(chan, node->msg);
		spin_lock_irqsave(&chan->lock, flags);
		kfree(node->msg);
		kfree(node);
	}
	spin_unlock_irqrestore(&chan->lock, flags);
}
