#ifndef __LINUX_PCN_KMSG_H
#define __LINUX_PCN_KMSG_H
/*
 * Header file for Popcorn inter-kernel messaging layer
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2013
 */

#include <linux/list.h>
#include <linux/multikernel.h>

/* BOOKKEEPING */

struct pcn_kmsg_rkinfo {
	unsigned long phys_addr;
	struct pcn_kmsg_window *window;
};

/* MESSAGING */

/* Enum for message types.  Modules should add types after
   PCN_KMSG_END. */
enum pcn_kmsg_type {
	PCN_KMSG_TYPE_CHECKIN,
	PCN_KMSG_TYPE_SIZE
};

/* Enum for message priority. */
enum pcn_kmsg_prio {
	PCN_KMSG_PRIO_HIGH,
	PCN_KMSG_PRIO_NORMAL
};

/* Message header */
struct pcn_kmsg_hdr {
	enum pcn_kmsg_type type;
	enum pcn_kmsg_prio prio;
	unsigned int size;
};

#define PCN_KMSG_PAYLOAD_SIZE 64

struct pcn_kmsg_message {
	struct pcn_kmsg_hdr hdr;
	char payload[PCN_KMSG_PAYLOAD_SIZE];
};

/* List entry to copy message into and pass around in receiving kernel */
struct pcn_kmsg_container {
	struct list_head list;
	struct pcn_kmsg_hdr hdr;
	char payload[PCN_KMSG_PAYLOAD_SIZE];	
};

/* WINDOW / BUFFERING */

#define PCN_KMSG_RBUF_SIZE 64

struct pcn_kmsg_window {
	int lock;
	unsigned long head;
	unsigned long tail;
	struct pcn_kmsg_message buffer[PCN_KMSG_RBUF_SIZE];
};

/* Typedef for function pointer to callback functions */
typedef int (*pcn_kmsg_cbftn)(struct pcn_kmsg_message *);

/* FUNCTIONS */
int pcn_kmsg_register_callback(enum pcn_kmsg_type type, pcn_kmsg_cbftn callback);
int pcn_kmsg_unregister_callback(enum pcn_kmsg_type type);
int pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg);

#endif /* __LINUX_PCN_KMSG_H */
