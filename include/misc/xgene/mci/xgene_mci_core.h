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

#ifndef XGENE_MCI_H_
#define XGENE_MCI_H_

#include <linux/io.h>
#include <linux/interrupt.h>
#include <misc/xgene/qmtm/xgene_qmtm.h>

#define DMCI_DEBUG

#define XGENE_MCI_MAX_CHANNEL 2
#define XGENE_MCI_MAX_CLIENT	(4096 - 8)
#define XGENE_MCI_BUFF_LEN	0x7FFF
#define XGENE_MCI_DATA_MAX 48
#define TX_QMTM	1
#define RX_QMTM	1
#undef MCI_CONFIG_FLUSH_CACHE

#define APM_QM_MCI_RTYPE	0xF

#ifndef ARCH_MSLIM
#define QMI_CSR_BASE_ADDR			0x01f400000ULL
#define QMI_CSR_BASE_SIZE			0x10000
#define SM_QMI_SLAVE_CFGSSQMIWQASSOC__ADDR		0x90e0
#define SM_QMI_SLAVE_CFGSSQMIQMLITEWQASSOC__ADDR    	0x90f4
#define MSLIM_MEMSIZE	(256*1024*1024)
#endif

enum xgene_mci_msg_type {
	MSLIM_OK_MSG = 0,
	MSLIM_PING_MSG = 1,
	MSLIM_ERROR_MSG = 2,
	MSLIM_LOG_MSG = 3,
	MSLIM_BUF_MSG = 4,
	MSLIM_CLIENT_MSG = 5,
};

enum xgene_mci_rtype {
	MCI_OK = 0,
	MCI_CONNECTION_ESTABLISHED = -1,
	MCI_BUFFER_AVAILABLE = -2,
	MCI_CONNECTION_CLOSED = -3,
	MCI_CONNECTION_INUSE = -32,
	MCI_NO_SUPPORT_DRIVER = -21,
	MCI_EAGAIN = -22,
	MCI_OUTOFRESOURCE = -23,
};

enum xgene_mci_client_subcmd {
	MCI_CLIENT_OPEN= 0,
	MCI_CLIENT_CLOSE = 1,
	MCI_CLIENT_ACK = 2,
	MCI_CLIENT_DATA = 3,
};

enum xgene_mci_buf_msg_control_flg {
	MCI_BUF_REQ	= 0,
	MCI_BUF_AVAIL = 1,
	MCI_BUF_MSG = 2,
	MCI_BUF_FREE_POOL_Q = 3,
};

enum xgene_mci_client_data_type {
	MCI_CLIENT_DATA_LOCAL_MEM = 0,
	MCI_CLIENT_DATA_REMOTE_MEM = 1,
};

struct xgene_mci_msg16 {
	u16 Tag;
	u16 Control;

	u32 FPQNum      : 12; /* 12 + Rv(2) */
	u32 Rv2         : 2;
	u32 ELErr       : 2;
	u32 LEI         : 2;
	u32 NV          : 1;
	u32 LL          : 1;
	u32 PB          : 1;
	u32 HB          : 1;
	u32 Rv          : 1;
	u32 IN          : 1;
	u32 RType       : 4;
	u32 LErr        : 3;
	u32 HL          : 1;

	u64 DataAddr    : 42; /* 32/10 split */
	u32 Rv6         : 6;
	u32 BufDataLen  : 15;
	u32 C           : 1;
}__attribute__ ((packed));

struct xgene_mci_msg32 {
	/* First 16 bytes*/
	union {
		int FirstDataWord[4];
		struct xgene_mci_msg16 msg16;
	};
	/* Second 16 bytes : For Data Payload*/
	union {
		int DataWord[4];
		char DataByte[16];
	};
}__attribute__ ((packed));


struct xgene_mci_msg64 {
	struct xgene_mci_msg32 msg32;
	/* Ext 32 bytes: For Data Payload */
	union {
		int DataWord[8];
		char DataByte[32];
	};
} __attribute__ ((packed));

enum xgene_mci_send_flag {
	MCI_POLLING,
	MCI_ASYNC,
	MCI_FREEPOOL,
};

enum xgene_mci_msg_priority {
	MCI_HIGH_PRIORITY = 0,
	MCI_LOW_PRIORITY = 1,
};

struct xgene_mci_msg_desc {
	u32 ID;
	u32 Tag;
	u32 control;
	int param0;
	int param1;
	int param2;
	int param3;
	u64 buf;	/* Data buffer*/
	u32 buf_length; /* Data buffer length*/
	int rvalue;
} __attribute__ ((packed));

struct xgene_mci_msg_desc_ll {
	struct xgene_mci_msg_desc *msg;
	int flag;
	struct list_head node;
};

struct xgene_mci_q_desc {
	int slot_cnt;		/* Total number of 32B slots */
	int slot_cur;   	/* Current slot */
	volatile u32 *nummsgs;
	struct xgene_qmtm_msg32 *base_addr; /* Base address of the descriptor */
	struct xgene_mci_msg_desc_ll msg_ll;
	int qid;		/* Queue ID */
	int mbid;		/* Mailbox ID */
	int pbn;		/* PBN number */
	int irq;		/* IRQ for inbound to CPU */
	char irq_name[8];
};

/* MCI callback function type */
typedef int (*apm_mci_msg_fn_ptr) (struct xgene_mci_msg_desc *msg_desc);

struct xgene_mci_channel {
	u32 ID;
	int cmd_done;
	struct xgene_mci_msg64 *qm_msg; //qm_msg_desc;
	struct xgene_mci_msg_desc *msg_desc;
	wait_queue_head_t waitq;
	spinlock_t lock;
	int ok_count;

	struct tasklet_struct rx_tasklet;
	struct delayed_work rxwork;
 	apm_mci_msg_fn_ptr mci_cb_fn;
	struct xgene_mci_q_desc tx_q_desc;
	struct xgene_mci_q_desc rx_q_desc;

	int irq_count;
};

/* MCI Client callback functions*/
typedef int (* mci_status_cb)(int port, void *ctx, int status);
typedef int (* mci_rcv_notify_cb)(int port, void *ctx, void *buffer, int len);

struct xgene_mci_client {
	int port;	/* client port*/
	char name[8]; /* Client context ID*/
	void *context;
	int flag;	/* Transfer types: Polling, Async or Freepool*/
	mci_status_cb mci_client_status_cb_fn;
	mci_rcv_notify_cb mci_client_rcv_cb_fn;

	/*for debug*/
	int rx_data_msg_count;
	int rx_data_ack_msg_count;
	int tx_data_msg_count;
	int tx_data_ack_msg_count;
};

struct xgene_mci_client_operations {
	int (*rx_open_cb)(struct xgene_mci_channel *chan, int client_port, struct xgene_mci_msg_desc *mci_msg);
	int (*rx_close_cb)(struct xgene_mci_channel *chan, int client_port, struct xgene_mci_msg_desc *mci_msg);
	int (*rx_ack_cb)(struct xgene_mci_channel *chan, int client_port, struct xgene_mci_msg_desc *mci_msg);
	int (*rx_data_cb)(struct xgene_mci_channel *chan, int client_port, struct xgene_mci_msg_desc *mci_msg);
	int (*open)(struct xgene_mci_client *mci_client);
};

struct xgene_mci_operations {
	int (*tx_msg)(struct xgene_mci_channel *chan, struct xgene_qmtm_msg64 * msg);
	void (*rx_ping_msg_cb)(struct xgene_mci_channel *chan, struct xgene_mci_msg_desc * msg);
	int (*wait_for_msg_complete)(struct xgene_mci_channel *chan);
	void (*flush_dcache_area)(void *addr, size_t len);
	unsigned long long (*virt_to_phys)(struct xgene_mci_channel *chan, void *x, size_t len);
	unsigned long long (*phys_to_virt)(struct xgene_mci_channel *chan, void *x, size_t len);
};

struct xgene_mci {
	struct device *dev;

	spinlock_t lock;
	int enable;
	unsigned long long qmi_csr_paddr;
	void __iomem *qmi_csr_vaddr;
	unsigned long long mslim_mem_paddr;
	void __iomem *mslim_mem_vaddr;
	struct xgene_qmtm_sdev *sdev;
	struct xgene_mci_channel channels[XGENE_MCI_MAX_CHANNEL];
	struct xgene_mci_client* clients[XGENE_MCI_MAX_CLIENT];
	struct xgene_mci_client* unregistered_clients[XGENE_MCI_MAX_CLIENT];
	struct xgene_mci_operations *ops;
	struct xgene_mci_client_operations *client_ops;
};

struct xgene_mci* xgene_mci_get_dev(void);
int xgene_mci_open(char name[8], void* context, int flags,
		int (* mci_status_cb)(int port, void *ctx, int status),
		int (* mci_rcv_notify_cb)(int port, void *ctx, void *buffer, int len));
int xgene_mci_close(int port);
int xgene_mci_send(int port, void *buffer, int len);
struct xgene_mci_client* xgene_mci_get_client(struct xgene_mci *mci, int port);
struct xgene_mci_channel* xgene_mci_get_chan(struct xgene_mci *mci, int chan);
int xgene_mci_send_utils(int index, int sub);

void xgene_mci_rx_work(struct work_struct *work);
struct xgene_mci_msg_desc *xgene_mci_receive_msg(
		struct xgene_mci_channel *chan, struct xgene_mci_msg64 *msg);
int xgene_mci_process_msg(struct xgene_mci_channel *chan,
		struct xgene_mci_msg_desc *mci_msg);
int xgene_mci_send_msg(struct xgene_mci_channel *chan, int cmd, int ctl, int param, void *buffer,
		int len);

#endif /* XGENE_MCI_H_ */
