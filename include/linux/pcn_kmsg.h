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
	PCN_KMSG_TYPE_END
};

/* Enum for message priority. */
enum pcn_kmsg_prio {
	PCN_KMSG_PRIO_HIGH,
	PCN_KMSG_PRIO_NORMAL
};

/* Message header */
struct pcn_kmsg_header {
	enum pcn_kmsg_type type;
	enum pcn_kmsg_prio prio;
};

struct pcn_kmsg_message {
	struct pcn_kmsg_header hdr;
	char payload[64];
};

/* List entry to copy message into and pass around in receiving kernel */
struct pcn_kmsg_container {
	struct list_head list;
	struct pcn_kmsg_message msg;
	
};

#endif /* __LINUX_PCN_KMSG_H */
