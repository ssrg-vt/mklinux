/*
 * Copyright (C) 2014 Linaro Ltd.
 * Author: Jassi Brar <jassisinghbrar@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MAILBOX_CLIENT_H
#define __MAILBOX_CLIENT_H

#include <linux/mailbox.h>

struct mbox_chan;

/**
 * struct mbox_client - User of a mailbox
 * @chan_name:		The "controller:channel" this client wants
 * @rx_callback:	Atomic callback to provide client the data received
 * @tx_done:		Atomic callback to tell client of data transmission
 * @tx_block:		If the mbox_send_message should block until data is
 *			transmitted.
 * @tx_tout:		Max block period in ms before TX is assumed failure
 * @knows_txdone:	if the client could run the TX state machine. Usually
 *			if the client receives some ACK packet for transmission.
 *			Unused if the controller already has TX_Done/RTR IRQ.
 * @link_data:		Optional controller specific parameters during channel
 *			request.
 */
struct mbox_client {
	char *chan_name;
	void (*rx_callback)(struct mbox_client *cl, void *mssg);
	void (*tx_done)(struct mbox_client *cl, void *mssg, enum mbox_result r);
	bool tx_block;
	unsigned long tx_tout;
	bool knows_txdone;
	void *link_data;
};

struct mbox_chan *mbox_request_channel(struct mbox_client *cl);
int mbox_send_message(struct mbox_chan *chan, void *mssg);
void mbox_client_txdone(struct mbox_chan *chan, enum mbox_result r);
void mbox_free_channel(struct mbox_chan *chan);
int mbox_notify_chan_register(const char *name, struct notifier_block *nb);
void mbox_notify_chan_unregister(const char *name, struct notifier_block *nb);

#endif /* __MAILBOX_CLIENT_H */
