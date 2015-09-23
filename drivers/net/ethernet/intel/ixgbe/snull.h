
/*
 * snull.h -- definitions for the network module
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 */

/*
 * Macros to help debugging
 */


#include <linux/pcn_kmsg.h>
#undef PDEBUG             /* undef it, just in case */
#ifdef SNULL_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "snull: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */


/* These are the flags in the statusword */
#define SNULL_RX_INTR 0x0001
#define SNULL_TX_INTR 0x0002

/* Default timeout period */
#define SNULL_TIMEOUT 5   /* In jiffies */
#define MAX_PORTS 65535
#define LOAD_THRESH 5000
#define ONE_TIME_THRES 10
#define BASE_LOAD 1024
#define CPU_THRESH 80

#define MAX_KERNEL 24
#include <net/inet_common.h>
extern struct net_device *snull_devs[];

struct rx_copy_msg{
         struct pcn_kmsg_hdr header;

         int filter_id;
         int is_child;
         __be16 dport;
         __be32 daddr;
         long long pckt_id;
         long long local_tx;
         ktime_t tstamp; 
         char cb[48];
         union {
                 __wsum          csum;
                 struct {
                         __u16   csum_start;
                         __u16   csum_offset;
                 };
         };
         __u32 priority;
         kmemcheck_bitfield_begin(flags1);
        __u8 local_df:1,
              cloned:1,
       ip_summed:2,
           nohdr:1,
        nfctinfo:3;
   __u8 pkt_type:3,
          fclone:2,
          ipvs_property:1,
           peeked:1,
           nf_trace:1;
           kmemcheck_bitfield_end(flags1);
          __be16 protocol;
          __u32 rxhash;
          int skb_iif;
  #ifdef CONFIG_NET_SCHED
          __u16 tc_index;       /* traffic control index */
  #ifdef CONFIG_NET_CLS_ACT
          __u16 tc_verd;        /* traffic control verdict */
  #endif
  #endif
          kmemcheck_bitfield_begin(flags2);
  #ifdef CONFIG_IPV6_NDISC_NODETYPE
          __u8 ndisc_nodetype:2;
  #endif
          __u8 ooo_okay:1;
          __u8 l4_rxhash:1;
          kmemcheck_bitfield_end(flags2);
  #ifdef CONFIG_NETWORK_SECMARK
          __u32 secmark;
  #endif
           union {
                  __u32           mark;
                  __u32           dropcount;
          };
           __u16 vlan_tci;
           int transport_header_off;
           int network_header_off;
           int mac_header_off;
   
          int headerlen;
          int datalen;
          int taillen;
           //NOTE: data must be the last field;
           char data;
};

struct rx_copy_work{
      struct work_struct work;
      struct net_filter_info* filter;
      void* data;
};

struct remote_eth_dev_response {
	struct pcn_kmsg_hdr header;
	unsigned long pci_base_adress;
	unsigned int pci_size;
	unsigned long eth_dev_base_add;
	unsigned int msi_vector;
	int irq;
	unsigned int max_kernels;
	char pad[56];
}__attribute__((packed)) __attribute__((aligned(64)));
struct remote_eth_dev_req{
	struct pcn_kmsg_hdr header;
	int kernel_id;
	int vector;

};

struct remote_flow_redirect_req{
	struct pcn_kmsg_hdr header;
	int kernel_id;
	int port;
}__attribute__((packed)) __attribute__((aligned(64)));;

struct remote_flow_redirect_res{
	struct pcn_kmsg_hdr header;
	int status;
}__attribute__((packed)) __attribute__((aligned(64)));;


struct remote_loadinfo_req{
	struct pcn_kmsg_hdr header;
	int kernel_id;
}__attribute__((packed)) __attribute__((aligned(64)));;

struct remote_loadinfo_res{
	struct pcn_kmsg_hdr header;
	unsigned long avg_load;
	unsigned long syn_fin_bal;
}__attribute__((packed)) __attribute__((aligned(64)));;


struct remote_cpu_load_info{
	unsigned int kernel_id;
	unsigned long avg_load;
	unsigned long syn_fin_bal;
};


struct remote_filter{
	struct list_head mylist ;
	unsigned int ip;
	unsigned int port;
	unsigned int kernel;
};


