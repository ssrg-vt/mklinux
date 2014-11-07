

#include <linux/pcn_kmsg.h>


struct _remote_wakeup_request {
	struct pcn_kmsg_hdr header;
	int nr_wake;			// 4
	int nr_wake2;			// 4
	u32 bitset;				// 4
	int cmpval;				// 4
	int tghid;				// 4
	int rflag;				// 4
	int pid;				// 4
	unsigned int flags;		// 4
	int fn_flag;			// 4
	unsigned int ticket;	// 4
	unsigned long uaddr;	// 8
	unsigned long uaddr2;	// 8
	unsigned int ops ;	// 4
	char pad[64];
}__attribute__((packed));// __attribute__((aligned(64)));


typedef struct _remote_wakeup_request _remote_wakeup_request_t;


struct _remote_wakeup_response {
	struct pcn_kmsg_hdr header;
	int errno;				// 4
	int request_id;			// 4
	pid_t rem_pid;			// 4
	unsigned long uaddr;	// 8
	char pad_string[64];
}__attribute__((packed));// __attribute__((aligned(64)));


typedef struct _remote_wakeup_response _remote_wakeup_response_t;



struct _remote_key_request {
	struct pcn_kmsg_hdr header;
	unsigned long uaddr;	// 8
	unsigned int flags;		// 4
	unsigned int fn_flags;	// 4
	u32 bitset;
	int rw;					// 4
	int pid;				// 4
	int val;				// 4
	int tghid;				// 4
	unsigned int ticket;	// 4
	unsigned int ops ;	// 4
	char pad_string[64];
}__attribute__((packed));// __attribute__((aligned(64)));

typedef struct _remote_key_request _remote_key_request_t;

struct _remote_key_response {
	struct pcn_kmsg_hdr header;
	int errno;				// 4
	int request_id;			// 4
	pid_t rem_pid;			// 4
	unsigned long uaddr;	// 8
	char pad_string[64];
}__attribute__((packed));// __attribute__((aligned(64)));


typedef struct _remote_key_response _remote_key_response_t;
