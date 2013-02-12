#ifndef __LINUX_PCN_KMSG_H
#define __LINUX_PCN_KMSG_H
/*
 * Header file for Popcorn inter-kernel messaging layer
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2013
 */

#include <linux/list.h>
#include <linux/multikernel.h>

/* LOCKING / SYNCHRONIZATION */
#define pcn_cpu_relax() __asm__ ("pause":::"memory")
#define pcn_barrier() __asm__ __volatile__("":::"memory")

/* BOOKKEEPING */

struct pcn_kmsg_rkinfo {
	unsigned long phys_addr[POPCORN_MAX_CPUS];
	//struct pcn_kmsg_window *window;
};

/* MESSAGING */

typedef unsigned long pcn_kmsg_mcast_id;

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
	unsigned int from_cpu	:8; // b0
	enum pcn_kmsg_type type	:8; // b1
	enum pcn_kmsg_prio prio	:8; // b2
	unsigned int spare 	:7; // b3
	unsigned int ready	:1; 
}__attribute__((packed));

#define PCN_KMSG_PAYLOAD_SIZE 60

/* The actual messages.  The expectation is that developers will create their
   own message structs with the payload replaced with their own fields, and then
   cast them to a struct pkn_kmsg_message.  See the checkin message below for
   an example of how to do this. */
struct pcn_kmsg_message {
	unsigned char payload[PCN_KMSG_PAYLOAD_SIZE];
	struct pcn_kmsg_hdr hdr;
}__attribute__((packed)) __attribute__((aligned(64)));

/* List entry to copy message into and pass around in receiving kernel */
struct pcn_kmsg_container {
	struct pcn_kmsg_message msg;
	struct list_head list;
}__attribute__((packed));

/* Message struct for guest kernels to check in with each other. */
struct pcn_kmsg_checkin_message {
	unsigned long window_phys_addr;
	char pad[52];
	struct pcn_kmsg_hdr hdr;
}__attribute__((packed)) __attribute__((aligned(64)));

/* Message struct for testing */
struct pcn_kmsg_test_message {
	unsigned long test_val;
	char pad[52];
	struct pcn_kmsg_hdr hdr;
}__attribute__((packed)) __attribute__((aligned(64)));

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

/* SETUP */

/* Register a callback function to handle a new message type.  Intended to
   be called when a kernel module is loaded. */
int pcn_kmsg_register_callback(enum pcn_kmsg_type type, pcn_kmsg_cbftn callback);

/* Unregister a callback function for a message type.  Intended to
   be called when a kernel module is unloaded. */
int pcn_kmsg_unregister_callback(enum pcn_kmsg_type type);

/* MESSAGING */

/* Send a message to the specified destination CPU. */
int pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg);

/* MULTICAST GROUPS */

/* Open a multicast group containing the CPUs specified in the mask. */
int pcn_kmsg_mcast_open(pcn_kmsg_mcast_id *id, unsigned long mask);

/* Add new members to a multicast group. */
int pcn_kmsg_mcast_add_members(unsigned long mask);

/* Remove existing members from a multicast group. */
int pcn_kmsg_mcast_delete_members(unsigned long mask);

/* Close a multicast group. */
int pcn_kmsg_mcast_close(pcn_kmsg_mcast_id id);

/* Send a message to the specified multicast group. */
int pcn_kmsg_mcast_send(pcn_kmsg_mcast_id id, struct pcn_kmsg_message *msg);

#endif /* __LINUX_PCN_KMSG_H */
