#ifndef __LINUX_PCN_KMSG_H
#define __LINUX_PCN_KMSG_H
/*
 * Header file for Popcorn inter-kernel messaging layer
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2013
 */

#include <linux/list.h>
#include <linux/multikernel.h>
#include <linux/types.h>

/* LOCKING / SYNCHRONIZATION */
#define pcn_cpu_relax() __asm__ ("pause":::"memory")
#define pcn_barrier() __asm__ __volatile__("":::"memory")

/* BOOKKEEPING */

#define POPCORN_MAX_MCAST_CHANNELS 32

struct pcn_kmsg_mcast_wininfo {
	volatile unsigned char lock;
	unsigned char owner_cpu;
	volatile unsigned char is_closing;
	unsigned long mask;
	unsigned int num_members;
	unsigned long phys_addr;
};

struct pcn_kmsg_rkinfo {
	unsigned long phys_addr[POPCORN_MAX_CPUS];
	struct pcn_kmsg_mcast_wininfo mcast_wininfo[POPCORN_MAX_MCAST_CHANNELS];
};

enum pcn_kmsg_wq_ops {
	PCN_KMSG_WQ_OP_MAP_MSG_WIN,
	PCN_KMSG_WQ_OP_UNMAP_MSG_WIN,
	PCN_KMSG_WQ_OP_MAP_MCAST_WIN,
	PCN_KMSG_WQ_OP_UNMAP_MCAST_WIN
};

typedef unsigned long pcn_kmsg_mcast_id;

typedef struct {
	struct work_struct work;
	enum pcn_kmsg_wq_ops op;
	int from_cpu;
	int cpu_to_add;
	pcn_kmsg_mcast_id id_to_join;
} pcn_kmsg_work_t;

/* MESSAGING */

/* Enum for message types.  Modules should add types after
   PCN_KMSG_END. */
enum pcn_kmsg_type {
	PCN_KMSG_TYPE_TEST,
	PCN_KMSG_TYPE_TEST_LONG,
	PCN_KMSG_TYPE_CHECKIN,
	PCN_KMSG_TYPE_MCAST,
    PCN_KMSG_TYPE_PROC_SRV_CLONE_REQUEST,
    PCN_KMSG_TYPE_PROC_SRV_CREATE_PROCESS_PAIRING,
    PCN_KMSG_TYPE_PROC_SRV_EXIT_PROCESS,
    PCN_KMSG_TYPE_PROC_SRV_VMA_TRANSFER,
    PCN_KMSG_TYPE_PROC_SRV_PTE_TRANSFER,
    PCN_KMSG_TYPE_PROC_SRV_MAPPING_QUERY,
    PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE,
	PCN_KMSG_TYPE_MCAST_CLOSE,
	PCN_KMSG_TYPE_SHMTUN,
	PCN_KMSG_TYPE_MAX
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

	enum pcn_kmsg_prio prio	:5; // b2
	unsigned int is_lg_msg  :1;
	unsigned int lg_start   :1;
	unsigned int lg_end     :1;

	unsigned int lg_seqnum 	:7; // b3
	unsigned int ready	:1; 
}__attribute__((packed));

#define PCN_KMSG_PAYLOAD_SIZE 60

/* The actual messages.  The expectation is that developers will create their
   own message structs with the payload replaced with their own fields, and then
   cast them to a struct pcn_kmsg_message.  See the checkin message below for
   an example of how to do this. */

/* Struct for the actual messages.  Note that hdr and payload are flipped
   when this actually goes out, so the receiver can poll on the ready bit
   in the header. */
struct pcn_kmsg_message {
	struct pcn_kmsg_hdr hdr;
	unsigned char payload[PCN_KMSG_PAYLOAD_SIZE];
}__attribute__((packed)) __attribute__((aligned(64)));

/* Struct for sending long messages (>60 bytes payload) */
struct pcn_kmsg_long_message {
	struct pcn_kmsg_hdr hdr;
	unsigned char payload[512];
}__attribute__((packed));

/* List entry to copy message into and pass around in receiving kernel */
struct pcn_kmsg_container {
	struct list_head list;
	struct pcn_kmsg_message msg;
}__attribute__((packed));


struct pcn_kmsg_reverse_message {
	unsigned char payload[PCN_KMSG_PAYLOAD_SIZE];
	struct pcn_kmsg_hdr hdr;
}__attribute__((packed)) __attribute__((aligned(64)));


/* TYPES OF MESSAGES */

/* Message struct for guest kernels to check in with each other. */
struct pcn_kmsg_checkin_message {
	struct pcn_kmsg_hdr hdr;
	unsigned long window_phys_addr;
	unsigned char cpu_to_add;
	char pad[51];
}__attribute__((packed)) __attribute__((aligned(64)));



/* WINDOW / BUFFERING */

#define PCN_KMSG_RBUF_SIZE 64

struct pcn_kmsg_window {
	volatile unsigned long head;
	volatile unsigned long tail;
	volatile unsigned char int_enabled;
	volatile struct pcn_kmsg_reverse_message buffer[PCN_KMSG_RBUF_SIZE];
}__attribute__((packed));

/* Typedef for function pointer to callback functions */
typedef int (*pcn_kmsg_cbftn)(struct pcn_kmsg_message *);

/* FUNCTIONS */

/* SETUP */

/* Register a callback function to handle a new message type.  Intended to
   be called when a kernel module is loaded. */
int pcn_kmsg_register_callback(enum pcn_kmsg_type type, 
			       pcn_kmsg_cbftn callback);

/* Unregister a callback function for a message type.  Intended to
   be called when a kernel module is unloaded. */
int pcn_kmsg_unregister_callback(enum pcn_kmsg_type type);

/* MESSAGING */

/* Send a message to the specified destination CPU. */
int pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg);

/* Send a long message to the specified destination CPU. */
int pcn_kmsg_send_long(unsigned int dest_cpu, 
		       struct pcn_kmsg_long_message *lmsg, 
		       unsigned int payload_size);

/* Free a received message (called at the end of the callback function) */
inline void pcn_kmsg_free_msg(void *msg);

/* MULTICAST GROUPS */

/* Enum for mcast message type. */
enum pcn_kmsg_mcast_type {
	PCN_KMSG_MCAST_OPEN,
	PCN_KMSG_MCAST_ADD_MEMBERS,
	PCN_KMSG_MCAST_DEL_MEMBERS,
	PCN_KMSG_MCAST_CLOSE,
	PCN_KMSG_MCAST_MAX
};

/* Message struct for guest kernels to check in with each other. */
struct pcn_kmsg_mcast_message {
	struct pcn_kmsg_hdr hdr;
	enum pcn_kmsg_mcast_type type :32; 
	pcn_kmsg_mcast_id id;	
	unsigned long mask;
	unsigned int num_members;
	unsigned long window_phys_addr;
	char pad[28];
}__attribute__((packed)) __attribute__((aligned(64)));

struct pcn_kmsg_mcast_window {
	volatile unsigned long head;
	volatile unsigned long tail;
	atomic_t read_counter[PCN_KMSG_RBUF_SIZE];
	volatile struct pcn_kmsg_reverse_message buffer[PCN_KMSG_RBUF_SIZE];
}__attribute__((packed));

struct pcn_kmsg_mcast_local {
	struct pcn_kmsg_mcast_window * mcastvirt;
	unsigned long local_tail;
};

/* Open a multicast group containing the CPUs specified in the mask. */
int pcn_kmsg_mcast_open(pcn_kmsg_mcast_id *id, unsigned long mask);

/* Add new members to a multicast group. */
int pcn_kmsg_mcast_add_members(pcn_kmsg_mcast_id id, unsigned long mask);

/* Remove existing members from a multicast group. */
int pcn_kmsg_mcast_delete_members(pcn_kmsg_mcast_id id, unsigned long mask);

/* Close a multicast group. */
int pcn_kmsg_mcast_close(pcn_kmsg_mcast_id id);

/* Send a message to the specified multicast group. */
int pcn_kmsg_mcast_send(pcn_kmsg_mcast_id id, struct pcn_kmsg_message *msg);

/* Send a long message to the specified multicast group. */
int pcn_kmsg_mcast_send_long(pcn_kmsg_mcast_id id, 
			     struct pcn_kmsg_long_message *msg,
			     unsigned int payload_size);

#endif /* __LINUX_PCN_KMSG_H */
