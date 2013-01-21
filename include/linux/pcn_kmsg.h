#ifndef __LINUX_PCN_KMSG_H
#define __LINUX_PCN_KMSG_H
/*
 * Header file for Popcorn inter-kernel messaging layer
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2013
 */

#include <linux/list.h>
#include <linux/multikernel.h>

/* LOCKING */


/* BOOKKEEPING */

struct pcn_kmsg_rkinfo {
	unsigned long phys_addr[POPCORN_MAX_CPUS];
	//struct pcn_kmsg_window *window;
};

/* MESSAGING */

/* Enum for message types.  Modules should add types after
   PCN_KMSG_END. */
enum pcn_kmsg_type {
	PCN_KMSG_TYPE_TEST,
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
	unsigned int from_cpu; // 4B
	enum pcn_kmsg_type type; // 4B
	enum pcn_kmsg_prio prio; // 4B
	unsigned int size; // 4B
}__attribute__((packed));

#define PCN_KMSG_PAYLOAD_SIZE 64

/* The actual messages.  The expectation is that developers will create their
   own message structs with the payload replaced with their own fields, and then
   cast them to a struct pkn_kmsg_message.  See the checkin message below for
   an example of how to do this. */
struct pcn_kmsg_message {
	struct pcn_kmsg_hdr hdr;
	unsigned char payload[PCN_KMSG_PAYLOAD_SIZE];
}__attribute__((packed));

/* List entry to copy message into and pass around in receiving kernel */
struct pcn_kmsg_container {
	struct list_head list;
	struct pcn_kmsg_hdr hdr;
	unsigned char payload[PCN_KMSG_PAYLOAD_SIZE];	
}__attribute__((packed));

/* Message struct for guest kernels to check in with each other. */
struct pcn_kmsg_checkin_message {
	struct pcn_kmsg_hdr hdr;
	unsigned long window_phys_addr;
}__attribute__((packed));

/* Message struct for testing */
struct pcn_kmsg_test_message {
	struct pcn_kmsg_hdr hdr;
	unsigned long test_val;
}__attribute__((packed));

/* WINDOW / BUFFERING */

#define PCN_KMSG_RBUF_SIZE 64

struct pcn_kmsg_window {
	unsigned long head;
	unsigned long tail;
	struct pcn_kmsg_message buffer[PCN_KMSG_RBUF_SIZE];
}__attribute__((packed));

/* Typedef for function pointer to callback functions */
typedef int (*pcn_kmsg_cbftn)(struct pcn_kmsg_message *);

/* FUNCTIONS */
int pcn_kmsg_register_callback(enum pcn_kmsg_type type, pcn_kmsg_cbftn callback);
int pcn_kmsg_unregister_callback(enum pcn_kmsg_type type);
int pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg);

#endif /* __LINUX_PCN_KMSG_H */
