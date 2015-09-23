/*
 * snull.c --  the Simple Network Utility
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
 *
 * $Id: snull.c,v 1.21 2004/11/05 02:36:03 rubini Exp $
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

#include <linux/sched.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/interrupt.h> /* mark_bh */

#include <linux/in.h>
#include <linux/netdevice.h>   /* struct device, and other headers */
#include <linux/etherdevice.h> /* eth_type_trans */
#include <linux/ip.h>          /* struct iphdr */
#include <linux/tcp.h>         /* struct tcphdr */
#include <linux/skbuff.h>

#include "snull.h"

#include <linux/in6.h>
#include <asm/checksum.h>

#include <linux/types.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/in.h>
#include <linux/interrupt.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/sctp.h>
#include <linux/pkt_sched.h>
#include <linux/ipv6.h>
#include <linux/slab.h>
#include <net/checksum.h>
#include <net/ip6_checksum.h>
#include <linux/ethtool.h>
#include <linux/if.h>
#include <linux/if_vlan.h>
#include <linux/prefetch.h>
#include <scsi/fc/fc_fcoe.h>
#include <linux/semaphore.h>
#include <linux/pcn_kmsg.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/net.h>
#include <net/sock.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/udp.h>
#include <net/tcp.h>
#include <linux/tcp.h>
#include <net/route.h>
#include <net/checksum.h>




#include "ixgbe.h"
#include "ixgbe_common.h"
#include "ixgbe_dcb_82599.h"
#include "ixgbe_sriov.h"
#include "ixgbe_type.h"


MODULE_AUTHOR("B M Saif Ansary");
MODULE_LICENSE("Dual BSD/GPL");
struct semaphore wait_config;
struct semaphore wait_red_confrim;
struct semaphore wait_load_update;


struct remote_eth_dev_response config;
static struct task_struct *load_balancer_task;
static struct task_struct *show_cpu_load;
extern flow_info* flow_info_data[MAX_FLOW_COUNT];

extern void inc_num_of_hash_buc();
extern void dec_num_of_hash_buc();
extern int read_num_of_hash_buc();
extern void set_number_ideal(int val);
extern int get_number_ideal();
extern int compare_ideal_curr();
extern unsigned long weighted_cpuload(const int cpu);
extern unsigned long source_load(int cpu, int type);
/*
 * Transmitter lockup simulation, normally disabled.
 */
 
extern void snull_clean_rx_ring(struct ixgbe_ring *rx_ring,int count);
extern int check_if_primary();
extern void wait_for_balance_req(void);
extern void clear_request_to_balance();

extern void print_tx_ring_desc(int q_no);
struct ixgbe_adapter * get_mapping_info_ixgbe_dev();
extern struct ixgbe_adapter * get_ixgbe_adapter();
void print_tx_ring(struct ixgbe_ring *tx_ring); 
extern void ixgbe_configure_rx_ring(struct ixgbe_adapter *adapter,
			     struct ixgbe_ring *ring);
extern void ixgbe_set_itr(struct ixgbe_q_vector *q_vector);
extern u64 get_sync_count(get_sync_count);
extern void clear_sync_count();
extern unsigned int read_request_to_balance();
//extern int pop_msi_set_affinity(unsigned int irq,unsigned int dest_cpu);

extern int pop_msi_set_affinity(unsigned int irq,unsigned int dest_cpu,unsigned int vector);
extern void ixgbe_alloc_rx_buffers(struct ixgbe_ring *rx_ring, u16 cleaned_count);
extern void snull_move_flow(struct ixgbe_hw *hw, flow_info * info, u8 queue);

static void * pci_remapped;
static struct ixgbe_ring * tx_ring_remapped;
static struct ixgbe_ring * rx_ring_remapped;

static struct ixgbe_q_vector *q_vector_remapped;
struct ixgbe_adapter  local_adapter;

 
static int lockup = 0;
module_param(lockup, int, 0);

static int timeout = SNULL_TIMEOUT;
module_param(timeout, int, 0);

/*
 * Do we run in NAPI mode?
 */
 
static int queue=0; 
static int use_napi = 0;
module_param(use_napi, int, 0);
struct semaphore *config_wait_sem;

static unsigned long pci_address=0xde080000;
static unsigned int pci_size=524288;
static unsigned long tx_ring_desc_add=0x0;
static unsigned long rx_ring_desc_add=0x0;

static unsigned int ring_desc_size=512;
static unsigned long tx_buffer_info_add=0x0;


module_param(pci_address, long, 0);
module_param(pci_size, int, 0);
module_param(tx_ring_desc_add,long, 0);
module_param(ring_desc_size, int, 0);
module_param(tx_buffer_info_add, long, 0);


netdev_tx_t snull_xmit_frame_ring(struct sk_buff *skb, struct ixgbe_adapter *adapter,  struct ixgbe_ring *tx_ring);
static bool ixgbe_clean_tx_irq(struct ixgbe_q_vector *q_vector, struct ixgbe_ring *tx_ring);
static bool ixgbe_clean_rx_irq(struct ixgbe_q_vector *q_vector, struct ixgbe_ring *rx_ring,int budget,struct napi_struct *napi);
int get_vector(int irq);
int get_irq(int q_no);
int get_irq_from_vector(int vector,int cpu);
void * get_adapter_phy();
void get_configuration_from_primary();

static int handle_remote_eth_dev_info_response(struct pcn_kmsg_message* inc_msg);
static int handle_remote_eth_dev_info_request(struct pcn_kmsg_message* inc_msg);
static int handle_remote_flow_red_response(struct pcn_kmsg_message* inc_msg);
static int handle_remote_flow_red_request(struct pcn_kmsg_message* inc_msg);
static int handle_remote_loadinfo_response(struct pcn_kmsg_message* inc_msg);
static int handle_remote_loadinfo_request(struct pcn_kmsg_message* inc_msg);
extern int handle_rx_copy(struct pcn_kmsg_message* inc_msg);


int get_max_kernel();
static struct proc_dir_entry* snull_proc_file;


struct remote_cpu_load_info remote_cpu_info[MAX_KERNEL];



/*
 * A structure representing an in-flight packet.
 */
struct snull_packet {
	struct snull_packet *next;
	struct net_device *dev;
	int	datalen;
	u8 data[ETH_DATA_LEN];
};

int pool_size = 8;
module_param(pool_size, int, 0);
int q_no=1;
module_param(q_no, int, 0);

u8 *mapping_table;

static int  snull_show(struct seq_file *m, void *v)
{
	int port,temp;
	for(port=0;port< 1<<12;port++)
	{
		temp = htons(port);
		if(flow_info_data[temp]!=NULL)
		{
			seq_printf(m, "port %x q %d ref %d\n",flow_info_data[temp]->port,flow_info_data[temp]->queue,atomic_read(&flow_info_data[temp]->ref));
		}
		
     }
     return 0;
}



static int snull_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, snull_show, NULL);
}



static const struct file_operations snull_file_fops = {
     .owner	= THIS_MODULE,
     .open	= snull_proc_open,
     .read	= seq_read,
     .llseek	= seq_lseek,
     .release	= single_release,
 };



/*
 * This structure is private to each device. It is used to pass
 * packets in and out, so there is place for a packet
 */

struct snull_priv {
	struct net_device_stats stats;
	int status;
	struct snull_packet *ppool;
	struct snull_packet *rx_queue;  /* List of incoming packets */
	int rx_int_enabled;
	int tx_packetlen;
	u8 *tx_packetdata;
	struct sk_buff *skb;
	spinlock_t lock;
	struct net_device *dev;
	struct napi_struct napi;
};

static void snull_tx_timeout(struct net_device *dev);
static void (*snull_interrupt)(int, void *);

static irqreturn_t irq_handler(int irq, void *data)
{
	//printk("%s:caught irq %d\n",__func__,irq);
	struct net_device *dev = (struct net_device *) data;
	struct snull_priv *priv = netdev_priv(dev);
	napi_schedule(&priv->napi);
	
	
	return IRQ_HANDLED;
}

/*
 * Set up a device's packet pool.
 */
void snull_setup_pool(struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	int i;
	struct snull_packet *pkt;

	priv->ppool = NULL;
	for (i = 0; i < pool_size; i++) {
		pkt = kmalloc (sizeof (struct snull_packet), GFP_KERNEL);
		if (pkt == NULL) {
			printk (KERN_NOTICE "Ran out of memory allocating packet pool\n");
			return;
		}
		pkt->dev = dev;
		pkt->next = priv->ppool;
		priv->ppool = pkt;
	}
}

void snull_teardown_pool(struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	struct snull_packet *pkt;
    
	while ((pkt = priv->ppool)) {
		priv->ppool = pkt->next;
		kfree (pkt);
		/* FIXME - in-flight packets ? */
	}
}    

/*
 * Buffer/pool management.
 */
struct snull_packet *snull_get_tx_buffer(struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	unsigned long flags;
	struct snull_packet *pkt;
    
	spin_lock_irqsave(&priv->lock, flags);
	pkt = priv->ppool;
	priv->ppool = pkt->next;
	if (priv->ppool == NULL) {
		printk (KERN_INFO "Pool empty\n");
		netif_stop_queue(dev);
	}
	spin_unlock_irqrestore(&priv->lock, flags);
	return pkt;
}


void snull_release_buffer(struct snull_packet *pkt)
{
	unsigned long flags;
	struct snull_priv *priv = netdev_priv(pkt->dev);
	
	spin_lock_irqsave(&priv->lock, flags);
	pkt->next = priv->ppool;
	priv->ppool = pkt;
	spin_unlock_irqrestore(&priv->lock, flags);
	if (netif_queue_stopped(pkt->dev) && pkt->next == NULL)
		netif_wake_queue(pkt->dev);
}

void snull_enqueue_buf(struct net_device *dev, struct snull_packet *pkt)
{
	unsigned long flags;
	struct snull_priv *priv = netdev_priv(dev);

	spin_lock_irqsave(&priv->lock, flags);
	pkt->next = priv->rx_queue;  /* FIXME - misorders packets */
	priv->rx_queue = pkt;
	spin_unlock_irqrestore(&priv->lock, flags);
}

struct snull_packet *snull_dequeue_buf(struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	struct snull_packet *pkt;
	unsigned long flags;

	spin_lock_irqsave(&priv->lock, flags);
	pkt = priv->rx_queue;
	if (pkt != NULL)
		priv->rx_queue = pkt->next;
	spin_unlock_irqrestore(&priv->lock, flags);
	return pkt;
}

/*
 * Enable and disable receive interrupts.
 */
static void snull_rx_ints(struct net_device *dev, int enable)
{
	struct snull_priv *priv = netdev_priv(dev);
	priv->rx_int_enabled = enable;
}
static inline void snull_irq_enable_queues(struct ixgbe_adapter *adapter,
					u64 qmask)
{
	u32 mask;
	struct ixgbe_hw *hw = &adapter->hw;

	switch (hw->mac.type) {
	case ixgbe_mac_82598EB:
		mask = (IXGBE_EIMS_RTX_QUEUE & qmask);
		IXGBE_WRITE_REG(hw, IXGBE_EIMS, mask);
		break;
	case ixgbe_mac_82599EB:
	case ixgbe_mac_X540:
		mask = (qmask & 0xFFFFFFFF);
		if (mask)
			IXGBE_WRITE_REG(hw, IXGBE_EIMS_EX(0), mask);
		mask = (qmask >> 32);
		if (mask)
			IXGBE_WRITE_REG(hw, IXGBE_EIMS_EX(1), mask);
		break;
	default:
		break;
	}
	/* skip the flush */
}

    
/*
 * Open and close
 */

#if 0
int snull_open(struct net_device *dev)
{
	pci_remapped=ioremap(pci_address,pci_size); 
	
	tx_ring_remapped=(struct ixgbe_ring *)ioremap(ring_desc_add,ring_desc_size);
	
	
	tx_ring_remapped->tx_buffer_info = (struct ixgbe_tx_buffer *)ioremap(tx_ring_remapped->tx_rx_buffer_info_phys,sizeof(struct ixgbe_tx_buffer));
	tx_ring_remapped->desc = ioremap(tx_ring_remapped->dma,tx_ring_remapped->size);
	tx_ring_remapped->pci_base_adress=pci_remapped;
	
	tx_ring_remapped->tail= pci_remapped + IXGBE_TDT(tx_ring_remapped->reg_idx);
	
	printk("%s:tx_ring_remapped->reg_idx %d TDT %d\n",tx_ring_remapped->reg_idx,IXGBE_TDT(tx_ring_remapped->reg_idx));
	
	
	printk("%s: pci_remapped %p phys tx_buffer_info %x tx_remapped %p\n ",__func__,tx_ring_remapped,tx_ring_remapped->tx_rx_buffer_info_phys,tx_ring_remapped->tx_buffer_info);  
	print_tx_ring(tx_ring_remapped);
	 
	memcpy(dev->dev_addr, "\0SNUL0", ETH_ALEN);
	if (dev == snull_devs[1])
		dev->dev_addr[ETH_ALEN-1]++; /* \0SNUL1 */
	netif_start_queue(dev);
	return 0;
}
#endif
int snull_open(struct net_device *dev)
{
	
	unsigned char mac_add[14]={0};
	mac_add[0]=0x00;
	mac_add[1]=0x1b;
	mac_add[2]=0x21;
	mac_add[3]=0xa6;
	mac_add[4]=0x15;
	mac_add[5]=0xa0;
	
	struct snull_priv *priv = netdev_priv(dev);
	memcpy(dev->dev_addr, mac_add, ETH_ALEN);
	
	
	//memcpy(dev->dev_addr, "\0SNUL0", ETH_ALEN);
	printk("%s: weight %d\n",__func__,priv->napi.weight);
	
	//ixgbe_setup_rx_resources(rx_ring_remapped);
	//ixgbe_configure_rx_ring(&local_adapter, rx_ring_remapped);
	snull_clean_rx_ring(rx_ring_remapped,511);
	
	ixgbe_alloc_rx_buffers(rx_ring_remapped,511);
	//q_vector_remapped
	
	napi_enable(&priv->napi);
	
	
	netif_start_queue(dev);
	return 0;
}



int snull_release(struct net_device *dev)
{
    /* release ports, irq and such -- like fops->close */

	netif_stop_queue(dev); /* can't transmit any more */
	return 0;
}

/*
 * Configuration changes (passed on by ifconfig)
 */
int snull_config(struct net_device *dev, struct ifmap *map)
{
	if (dev->flags & IFF_UP) /* can't act on a running interface */
		return -EBUSY;

	/* Don't allow changing the I/O address */
	if (map->base_addr != dev->base_addr) {
		printk(KERN_WARNING "snull: Can't change I/O address\n");
		return -EOPNOTSUPP;
	}

	/* Allow changing the IRQ */
	if (map->irq != dev->irq) {
		dev->irq = map->irq;
        	/* request_irq() is delayed to open-time */
	}

	/* ignore other fields */
	return 0;
}

/*
 * Receive a packet: retrieve, encapsulate and pass over to upper levels
 */
void snull_rx(struct net_device *dev, struct snull_packet *pkt)
{
	struct sk_buff *skb;
	struct snull_priv *priv = netdev_priv(dev);

	/*
	 * The packet has been retrieved from the transmission
	 * medium. Build an skb around it, so upper layers can handle it
	 */
	skb = dev_alloc_skb(pkt->datalen + 2);
	if (!skb) {
		if (printk_ratelimit())
			printk(KERN_NOTICE "snull rx: low on mem - packet dropped\n");
		priv->stats.rx_dropped++;
		goto out;
	}
	skb_reserve(skb, 2); /* align IP on 16B boundary */  
	memcpy(skb_put(skb, pkt->datalen), pkt->data, pkt->datalen);

	/* Write metadata, and then pass to the receive level */
	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
	priv->stats.rx_packets++;
	priv->stats.rx_bytes += pkt->datalen;
	netif_rx(skb);
  out:
	return;
}
    
#if 0
/*
 * The poll implementation.
 */
static int snull_poll(struct napi_struct *napi, int budget)
{
	int npackets = 0;
	struct sk_buff *skb;
	struct snull_priv *priv = container_of(napi, struct snull_priv, napi);
	struct net_device *dev = priv->dev;
	struct snull_packet *pkt;
    
	while (npackets < budget && priv->rx_queue) {
		pkt = snull_dequeue_buf(dev);
		skb = dev_alloc_skb(pkt->datalen + 2);
		if (! skb) {
			if (printk_ratelimit())
				printk(KERN_NOTICE "snull: packet dropped\n");
			priv->stats.rx_dropped++;
			snull_release_buffer(pkt);
			continue;
		}
		skb_reserve(skb, 2); /* align IP on 16B boundary */  
		memcpy(skb_put(skb, pkt->datalen), pkt->data, pkt->datalen);
		skb->dev = dev;
		skb->protocol = eth_type_trans(skb, dev);
		skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
		netif_receive_skb(skb);
		
        	/* Maintain stats */
		npackets++;
		priv->stats.rx_packets++;
		priv->stats.rx_bytes += pkt->datalen;
		snull_release_buffer(pkt);
	}
	/* If we processed all packets, we're done; tell the kernel and reenable ints */
	if (! priv->rx_queue) {
		napi_complete(napi);
		snull_rx_ints(dev, 1);
		return 0;
	}
	/* We couldn't process everything. */
	return npackets;
}
#endif	    

static int snull_poll(struct napi_struct *napi, int budget)
{
	
	//printk("%s: called budget %d \n",__func__,budget);
	bool clean_complete = true;
	struct ixgbe_adapter *adapter = &local_adapter;
	struct ixgbe_q_vector *q_vector = q_vector_remapped;
	int per_ring_budget=0;
	clean_complete &= !!ixgbe_clean_tx_irq(q_vector_remapped,tx_ring_remapped);
	
	if (q_vector->rx.count > 1)
		per_ring_budget = max(budget/q_vector->rx.count, 1);
	else
		per_ring_budget = budget;
	
	
	clean_complete &=ixgbe_clean_rx_irq(q_vector_remapped,rx_ring_remapped,per_ring_budget,napi);
	
	if (!clean_complete)
		return budget;
	
	napi_complete(napi);

	if (adapter->rx_itr_setting & 1)
		ixgbe_set_itr(q_vector);
	if (!test_bit(__IXGBE_DOWN, &adapter->state))
		snull_irq_enable_queues(adapter, ((u64)1 << q_vector->v_idx));

	
	
//	__snull_poll();
}
      
/*
 * The typical interrupt entry point
 */
static void snull_regular_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	int statusword;
	struct snull_priv *priv;
	struct snull_packet *pkt = NULL;
	/*
	 * As usual, check the "device" pointer to be sure it is
	 * really interrupting.
	 * Then assign "struct device *dev"
	 */
	struct net_device *dev = (struct net_device *)dev_id;
	/* ... and check with hw if it's really ours */

	/* paranoid */
	if (!dev)
		return;

	/* Lock the device */
	priv = netdev_priv(dev);
	spin_lock(&priv->lock);

	/* retrieve statusword: real netdevices use I/O instructions */
	statusword = priv->status;
	priv->status = 0;
	if (statusword & SNULL_RX_INTR) {
		/* send it to snull_rx for handling */
		pkt = priv->rx_queue;
		if (pkt) {
			priv->rx_queue = pkt->next;
			snull_rx(dev, pkt);
		}
	}
	if (statusword & SNULL_TX_INTR) {
		/* a transmission is over: free the skb */
		priv->stats.tx_packets++;
		priv->stats.tx_bytes += priv->tx_packetlen;
		dev_kfree_skb(priv->skb);
	}

	/* Unlock the device and we are done */
	spin_unlock(&priv->lock);
	if (pkt) snull_release_buffer(pkt); /* Do this outside the lock! */
	return;
}

/*
 * A NAPI interrupt handler.
 */
static void snull_napi_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	int statusword;
	struct snull_priv *priv;

	/*
	 * As usual, check the "device" pointer for shared handlers.
	 * Then assign "struct device *dev"
	 */
	struct net_device *dev = (struct net_device *)dev_id;
	/* ... and check with hw if it's really ours */

	/* paranoid */
	if (!dev)
		return;

	/* Lock the device */
	priv = netdev_priv(dev);
	spin_lock(&priv->lock);

	/* retrieve statusword: real netdevices use I/O instructions */
	statusword = priv->status;
	priv->status = 0;
	if (statusword & SNULL_RX_INTR) {
		snull_rx_ints(dev, 0);  /* Disable further interrupts */
		napi_schedule(&priv->napi);
	}
	if (statusword & SNULL_TX_INTR) {
        	/* a transmission is over: free the skb */
		priv->stats.tx_packets++;
		priv->stats.tx_bytes += priv->tx_packetlen;
		dev_kfree_skb(priv->skb);
	}

	/* Unlock the device and we are done */
	spin_unlock(&priv->lock);
	return;
}



/*
 * Transmit a packet (low level interface)
 */



/*
 * Transmit a packet (called by the kernel)
 */
int snull_tx(struct sk_buff *skb, struct net_device *dev)
{
	
//	printk("%s: called\n",__func__);
	
	snull_xmit_frame_ring(skb,dev,tx_ring_remapped);
	
/*	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	struct ixgbe_ring *tx_ring;

	tx_ring = adapter->tx_ring[skb->queue_mapping];
	return snull_xmit_frame_ring(skb, adapter, tx_ring);
*/	

	return 0; /* Our simple device can not fail */
}

/*
 * Deal with a transmit timeout.
 */
void snull_tx_timeout (struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);

	PDEBUG("Transmit timeout at %ld, latency %ld\n", jiffies,
			jiffies - dev->trans_start);
        /* Simulate a transmission interrupt to get things moving */
	priv->status = SNULL_TX_INTR;
	snull_interrupt(0, dev);
	priv->stats.tx_errors++;
	netif_wake_queue(dev);
	return;
}



/*
 * Ioctl commands 
 */
int snull_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	PDEBUG("ioctl\n");
	return 0;
}

/*
 * Return statistics to the caller
 */
struct net_device_stats *snull_stats(struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	return &priv->stats;
}

/*
 * This function is called to fill up an eth header, since arp is not
 * available on the interface
 */
int snull_rebuild_header(struct sk_buff *skb)
{
	struct ethhdr *eth = (struct ethhdr *) skb->data;
	struct net_device *dev = skb->dev;
    unsigned char mac_add[14]={0};
	mac_add[0]=0x00;
	mac_add[1]=0x1b;
	mac_add[2]=0x21;
	mac_add[3]=0x73;
	mac_add[4]=0xea;
	mac_add[5]=0x85;
    
    
	memcpy(eth->h_source, dev->dev_addr, dev->addr_len);
	memcpy(eth->h_dest, mac_add, dev->addr_len);
//	eth->h_dest[ETH_ALEN-1]   ^= 0x01;   /* dest is us xor 1 */                      //for now hardcode the mac address for destination
	return 0;
}


int snull_header(struct sk_buff *skb, struct net_device *dev,
                unsigned short type, const void *daddr, const void *saddr,
                unsigned len)
{
	struct ethhdr *eth = (struct ethhdr *)skb_push(skb,ETH_HLEN);
	unsigned char mac_add[14]={0};
	mac_add[0]=0x00;
	mac_add[1]=0x1b;
	mac_add[2]=0x21;
	mac_add[3]=0x73;
	mac_add[4]=0xea;
	mac_add[5]=0x85;



	eth->h_proto = htons(type);
	memcpy(eth->h_source, saddr ? saddr : dev->dev_addr, dev->addr_len);
//	memcpy(eth->h_dest, mac_add, dev->addr_len);
	memcpy(eth->h_dest, mac_add, ETH_ALEN);
//	eth->h_dest[ETH_ALEN-1]   ^= 0x01;   /* dest is us xor 1 */
	return (dev->hard_header_len);
}


/*
 * The "change_mtu" method is usually not needed.
 * If you need it, it must be like this.
 */
int snull_change_mtu(struct net_device *dev, int new_mtu)
{
	unsigned long flags;
	struct snull_priv *priv = netdev_priv(dev);
	spinlock_t *lock = &priv->lock;
    
	/* check ranges */
	if ((new_mtu < 68) || (new_mtu > 1500))
		return -EINVAL;
	/*
	 * Do anything you need, and the accept the value
	 */
	spin_lock_irqsave(lock, flags);
	dev->mtu = new_mtu;
	spin_unlock_irqrestore(lock, flags);
	return 0; /* success */
}

static const struct header_ops snull_header_ops = {
        .create  = snull_header,
	.rebuild = snull_rebuild_header
};

static const struct net_device_ops snull_netdev_ops = {
	.ndo_open            = snull_open,
	.ndo_stop            = snull_release,
	.ndo_start_xmit      = snull_tx,
	.ndo_do_ioctl        = snull_ioctl,
	.ndo_set_config      = snull_config,
	.ndo_get_stats       = snull_stats,
	.ndo_change_mtu      = snull_change_mtu,
	.ndo_tx_timeout      = snull_tx_timeout
};

//just to check


/*
 * The init function (sometimes called probe).
 * It is invoked by register_netdev()
 */
void snull_init(struct net_device *dev)
{
	struct snull_priv *priv;
	int i, err, pci_using_dac;
	
	
	
  //  int irq = get_irq_from_vector(config.msi_vector,smp_processor_id()); 
    int status;
    
 //   printk("%s: irq %d \n",__func__,irq);
 //   status = request_irq(irq,&irq_handler,0,"Popcorn Net",NULL);
//    request_irq(entry->vector, &ixgbe_msix_clean_rings, 0,
//				  q_vector->name, q_vector);
	
	
#if 0
    	/*
	 * Make the usual checks: check_region(), probe irq, ...  -ENODEV
	 * should be returned if no device found.  No resource should be
	 * grabbed: this is done on open(). 
	 */
#endif

    	/* 
	 * Then, assign other fields in dev, using ether_setup() and some
	 * hand assignments
	 */
	ether_setup(dev); /* assign some of the fields */
	dev->watchdog_timeo = timeout;
	dev->netdev_ops = &snull_netdev_ops;
//	dev->header_ops = &snull_header_ops;
	/* keep the default flags, just add NOARP */
//	dev->flags           |= IFF_NOARP;
	dev->features        |= NETIF_F_HW_CSUM;
	
	

	/*
	 * Then, initialize the priv field. This encloses the statistics
	 * and a few private fields.
	 */
	 
	 
	priv = netdev_priv(dev);
//	if (use_napi) {
	
//	}
	memset(priv, 0, sizeof(struct snull_priv));
	spin_lock_init(&priv->lock);
	snull_rx_ints(dev, 1);		/* enable receive interrupts */
	snull_setup_pool(dev);
	
	netif_napi_add(dev, &priv->napi, snull_poll,64);
}

/*
 * The devices
 */

extern struct net_device *snull_devs[2];



/*
 * Finally, the module stuff
 */

void snull_cleanup(void)
{
	int i;
    
	for (i = 0; i < 2;  i++) {
		if (snull_devs[i]) {
			unregister_netdev(snull_devs[i]);
			snull_teardown_pool(snull_devs[i]);
			free_netdev(snull_devs[i]);
		}
	}
	return;
}

extern void print_tx_ring_desc(int q_no);

int init_flow_info(flow_info* temp)
{
	temp = vmalloc(sizeof(flow_info));
	if(temp==NULL)
		return -1;
	atomic_set(&temp->ref,0);
	atomic_set(&temp->state,FLOW_ACTIVE);
	
	return 0;
}


void snull_create_flow_info(int max_kernels)
{
	
	int kernel_id=smp_processor_id();
	int port,queue,temp;
	//printk("%s: %d max_kernel %d\n",__func__,kernel_id,max_kernels);
	int ideal_num_hash_buc = (1 << 12) / max_kernels;
	set_number_ideal(ideal_num_hash_buc);
	
	for (port = 0; port < (1 << 12); port++) 
	{
		queue = port % max_kernels;
//		queue = 0;
//		printk("%s: q %d port %x\n",__func__,queue,port);
		if(queue==kernel_id)
		{
//			num_of_hash_buc++;
			inc_num_of_hash_buc();
		}
			temp = htons(port);
			if(flow_info_data[temp] == NULL)
				flow_info_data[temp] = (flow_info*)vmalloc(sizeof(flow_info));		
			atomic_set(&flow_info_data[temp]->ref,0);
			atomic_set(&flow_info_data[temp]->state,FLOW_ACTIVE);
			flow_info_data[temp]->port = port;
			flow_info_data[temp]->queue = queue;
	
//			flow_info_data[temp]->queue = 0;
	//	}
	}
	printk("%s: %lu %lu\n",__func__,get_number_ideal(),read_num_of_hash_buc());
}


void send_flow_redirection_info(int port,int target)
{
	struct remote_flow_redirect_req request;
	request.header.from_cpu = smp_processor_id();
	request.header.prio=PCN_KMSG_PRIO_NORMAL;
	request.header.type=PCN_KMSG_TYPE_FLW_RED_REQ;
	request.port=port;
	request.kernel_id = request.header.from_cpu;
	pcn_kmsg_send_long(target,(struct pcn_kmsg_long_message*)&request,sizeof(struct remote_eth_dev_req )-sizeof(request.header));
	down(&wait_red_confrim);
	
}



void get_remote_load(int kernel)
{
	struct remote_loadinfo_req request;
	request.header.from_cpu = smp_processor_id();
	request.header.prio=PCN_KMSG_PRIO_NORMAL;
	request.header.type=PCN_KMSG_TYPE_LDINF_REQ; 
	request.kernel_id = request.header.from_cpu;
	pcn_kmsg_send_long(kernel,(struct pcn_kmsg_long_message*)&request,sizeof(struct remote_eth_dev_req )-sizeof(request.header));
	down(&wait_load_update);
	
}

int update_remote_cpu_info(struct remote_cpu_load_info *kernels)
{
	int kernel,count=0;
	int my_id = smp_processor_id();
	int n= get_max_kernel();
	for(kernel=0;kernel<n;kernel++)
	{
		if(kernel==my_id)
		{
			remote_cpu_info[kernel].avg_load=0xFFFFFFFF;
			remote_cpu_info[kernel].syn_fin_bal=0xFFFFFFFF;
			remote_cpu_info[kernel].kernel_id=kernel;
			continue;
		}
		get_remote_load(kernel);
	}
	for(kernel=0;kernel<n;kernel++)
	{
		if(remote_cpu_info[kernel].avg_load!=0xFFFFFFFF)
		{
			kernels[count] = remote_cpu_info[kernel];
			count++;
		}
	}
	return count;
/*	
 * for (c = 0;c<(n-1);c++)
	{
		for (d=0;d<n-c-1;d++)
		{
			  if (kernels[d] > array[d+1])
			  {
				swap = array[d];
				array[d]   = array[d+1];
				array[d+1] = swap;
			  }
		}
    }
*/
}

/*OK so we would try to load balance. Idea is to find few
 * fileter enties that are not used and redirect them to other 
 * Qs*/


					

int avg_load;	

static unsigned long get_avg_load()
{
	return avg_load;
}	
#define ALPHA 0.15			
int snull_print_cpu_load()
{
	
	int cpud_id=smp_processor_id();
	int base = 1024;
	unsigned long current_load,type1;
	while(1)
	{
		current_load = weighted_cpuload(cpud_id);
		avg_load = (ALPHA * current_load) + (1-ALPHA)*avg_load;
//		type1 = source_load(cpud_id,1);
		msleep(200);
//		printk("%s: current %lu avg %lu\n",__func__,current_load,avg_load);
		
	}
}					



int snull_load_balance(void* arg0)
{
	int port,temp;
	struct ixgbe_adapter * adapter = (struct ixgbe_adapter *)arg0;
	struct ixgbe_hw* hw=&adapter->hw;
	int count=0,kernel_to=0,kernel_idx;
	int status,willing_kernels;
	//printk("%s: n_buc %lu i_buc %lu sync %lu\n",__func__,read_num_of_hash_buc(),get_number_ideal(),get_sync_count());
	int curr_kernel = smp_processor_id();
	//return 0;	
	
	while(1)
	{
		count=0;
		kernel_to=0;
		kernel_idx=0;
		
		struct remote_cpu_load_info kernels[MAX_KERNEL]={0};
		
/*		if((get_sync_count() <= LOAD_THRESH)||num_of_hash_buc<=ideal_num_hash_buc)
		{
//			printk("%s: n_buc %lu i_buc %lu sync %lu\n",__func__,num_of_hash_buc,ideal_num_hash_buc,get_sync_count());
			udelay(1000);
			continue;
		}	
*/		
		wait_for_balance_req();
//		printk("%s:woke up \n",__func__);
		if(read_request_to_balance()==0||compare_ideal_curr()>=0)
		{
//				printk("%s:nothing to do \n",__func__);
				continue;
		}		
	//	printk("%s:inside \n",__func__);

	willing_kernels=update_remote_cpu_info(kernels);
	//printk("%s:inside after willing kernels %d\n",__func__,willing_kernels);
	if(willing_kernels==0)
		continue;

//	while(count<ONE_TIME_THRES)
	{		
		for(port=0;port< 1<<12;port++)
		{
			
			temp = htons(port);
			int a;
			if(flow_info_data[temp]!=NULL)
			{
				a=atomic_read(&flow_info_data[temp]->ref);
				status = atomic_read(&flow_info_data[temp]->state);
				if(a==0&&status==FLOW_ACTIVE&&flow_info_data[temp]->queue==curr_kernel)
				{
					kernel_idx++;
					kernel_idx = kernel_idx % willing_kernels ;
					kernel_to = kernels[kernel_idx].kernel_id;
	//				printk("%s: moving entry %x port %x kernel_to %d curr_kernel %d kernel_idx %d\n",__func__,temp,flow_info_data[temp]->port,kernel_to,curr_kernel,kernel_idx);
					atomic_set(&flow_info_data[temp]->state,FLOW_TRANSITING);
					send_flow_redirection_info(flow_info_data[temp]->port,kernel_to);
					snull_move_flow(hw,flow_info_data[temp], kernel_to);
					atomic_set(&flow_info_data[temp]->state,FLOW_ROUTED);
	//				if(check_if_primary()==0)
	//				printk("%s: moved entry %x port %x kernel_to %d\n",__func__,temp,flow_info_data[temp]->port,kernel_to);
//					num_of_hash_buc--;
					dec_num_of_hash_buc();
					count++;
					if(compare_ideal_curr()>=0)
					{
						printk("%s: reached\n",__func__);
						break;
					}
 
				}
/*				else if(a<5&&status==FLOW_ACTIVE)
				{
					atomic_set(&flow_info_data[temp]->state,FLOW_TRANSITING);
				}
*/
				
			}
			
		 }
		 clear_request_to_balance();
		 msleep(1);

	 }
//		 clear_sync_count();
		 
	}
}



int __init snull_init_module(void)
{
	int result, i, ret = -ENOMEM;
	int status,irq;
	snull_interrupt = use_napi ? snull_napi_interrupt : snull_regular_interrupt;
	sema_init(&wait_config, 0);
	sema_init(&wait_red_confrim,0);
	sema_init(&wait_load_update,0);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_ETH_MAP_REQ,handle_remote_eth_dev_info_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_ETH_MAP_RES,handle_remote_eth_dev_info_response);
	
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FLW_RED_REQ,handle_remote_flow_red_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FLW_RED_RES,handle_remote_flow_red_response);
	
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_LDINF_REQ,handle_remote_loadinfo_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_LDINF_RES,handle_remote_loadinfo_response);
	
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_RX_COPY, handle_rx_copy);
	u8 queue;
//	mapping_table = vmalloc(0x10000);
	int port;
	
	
/*	for (port = 0; port < (1 << 12); port++) 
	{
	//	u8 queue = ;
//		queue = port % get_max_kernel();
		//mapping_table[htons(port)] = queue;
		
	//	printk("%s: called port %x acttual %x q %d\n",__func__,port,htons(port),queue);
	//	printk("%s: called port %x acttual %x q %d\n",__func__,port,htons(port),queue);
	//	snull_atr_setup_spread_load_balance_one(hw, port, queue);
	}
*/

	
//	snull_proc_file = proc_create("snull_proc_stats", 0, NULL, &snull_file_fops);

	if(check_if_primary())
	{
		printk("%s: primary\n",__func__);	
		snull_create_flow_info(get_max_kernel());
		load_balancer_task = kthread_run(snull_load_balance, (void*)get_ixgbe_adapter(), "Load_balancer");
		show_cpu_load = kthread_run(snull_print_cpu_load, NULL, "stats");
		return 0;
	}
	else
	{

		printk("%s: secondary\n",__func__);
	}	
	
	
//	print_tx_ring_desc(q_no);
	//print_tx_ring()
	/* Allocate the devices */
	snull_devs[0] = alloc_netdev(sizeof(struct snull_priv), "sn%d",snull_init);
	//snull_devs[1] = alloc_netdev(sizeof(struct snull_priv), "sn%d",
	//		snull_init);
//	if (snull_devs[0] == NULL || snull_devs[1] == NULL)
	if (snull_devs[0] == NULL)
		goto out;

	ret = -ENODEV;
//	for (i = 0; i < 2;  i++)
		if ((result = register_netdev(snull_devs[0])))
			printk("snull: error %i registering device \"%s\"\n",
					result, snull_devs[0]->name);
		else
			ret = 0;
			
//	irq=create_irq_nr(0,0);
	irq=17;
	status=request_irq(irq,&irq_handler,0,"Popcorn Net",snull_devs[0]);
	int vector = get_vector(irq);
	
	
	printk("%s: sending irq %d config request vector %d\n",__func__,irq,vector);
	get_configuration_from_primary(vector);
	printk("%s:after config reqeust pci base address %x size %d\n",__func__,config.pci_base_adress,config.pci_size);
	
	pci_remapped=ioremap(config.pci_base_adress,config.pci_size); 
	struct ixgbe_adapter * adapter = ioremap_cache(config.eth_dev_base_add,sizeof(struct ixgbe_adapter));
	tx_ring_desc_add = adapter->tx_ring_phys[smp_processor_id()];
	rx_ring_desc_add = adapter->rx_ring_phys[smp_processor_id()];
	
	memcpy(&local_adapter,adapter,sizeof(struct ixgbe_adapter));
	
	
	
	local_adapter.hw.hw_addr=pci_remapped;
	
	printk("%s: ring_desc_add : %x CPU ID %d \n",__func__,tx_ring_desc_add,smp_processor_id());
	
	tx_ring_remapped=(struct ixgbe_ring *)ioremap_cache(tx_ring_desc_add,sizeof(struct ixgbe_ring));
	rx_ring_remapped=(struct ixgbe_ring *)ioremap_cache(rx_ring_desc_add,sizeof(struct ixgbe_ring));
	
	q_vector_remapped = (struct ixgbe_q_vector *)ioremap_cache(adapter->q_vec_phy[smp_processor_id()],sizeof(struct ixgbe_q_vector));
	q_vector_remapped->adapter=&local_adapter;
	
	
	
	rx_ring_remapped->netdev=snull_devs[0]; 
	rx_ring_remapped->rx_buffer_info = (struct ixgbe_tx_buffer *)ioremap_cache(rx_ring_remapped->tx_rx_buffer_info_phys,sizeof(struct ixgbe_tx_buffer)*rx_ring_remapped->count);
	rx_ring_remapped->desc = ioremap_cache(rx_ring_remapped->dma,rx_ring_remapped->size);
	rx_ring_remapped->pci_base_adress=pci_remapped;
	rx_ring_remapped->tail= pci_remapped +IXGBE_RDT(rx_ring_remapped->reg_idx);
	
	tx_ring_remapped->tx_buffer_info = (struct ixgbe_tx_buffer *)ioremap_cache(tx_ring_remapped->tx_rx_buffer_info_phys,sizeof(struct ixgbe_tx_buffer)*tx_ring_remapped->count);
	tx_ring_remapped->desc = ioremap_cache(tx_ring_remapped->dma,tx_ring_remapped->size);
	tx_ring_remapped->pci_base_adress=pci_remapped;
	tx_ring_remapped->tail= pci_remapped + IXGBE_TDT(tx_ring_remapped->reg_idx);
	
	union ixgbe_adv_rx_desc *rx_desc;
	rx_desc = IXGBE_RX_DESC_ADV(rx_ring_remapped, i);
	 
	snull_create_flow_info(config.max_kernels);
	
	load_balancer_task = kthread_run(snull_load_balance, (void*)&local_adapter, "Load_balancer");
	show_cpu_load = kthread_run(snull_print_cpu_load, NULL, "stats");

	
	int staterr1,staterr2;
	unsigned long long start_1,end_1,start_2,end_2;
	start_1= native_read_tsc();
	
	staterr1 = le32_to_cpu(rx_desc->wb.upper.status_error);
	
	end_1=native_read_tsc();
	
	start_2= native_read_tsc();
	
	staterr2 = le32_to_cpu(rx_desc->wb.upper.status_error);
	
	end_2=native_read_tsc();
	
	
//	writel(0xFF, pci_remapped+IXGBE_FDIRHASH);
	printk("%s: %p time 1 %llu time 2 %llu\n",__func__,(pci_remapped+IXGBE_FDIRHASH),(end_1-start_1),(end_2-start_2));
	
	
	printk("%s:tx_ring_remapped->reg_idx %d TDT %x tail %p\n",__func__,tx_ring_remapped->reg_idx,IXGBE_TDT(tx_ring_remapped->reg_idx),tx_ring_remapped->tail);
	
	
	printk("%s: pci_remapped %p rx_ring_remapped->pci_base_adress %p phys tx_buffer_info %x tx_remapped %p\n ",__func__,pci_remapped,tx_ring_remapped->pci_base_adress,tx_ring_remapped->tx_rx_buffer_info_phys,tx_ring_remapped->tx_buffer_info);  
//	print_tx_ring(tx_ring_remapped);

   out:
	if (ret) 
		snull_cleanup();
	return ret;
}
/*popcorn stuff*/

/*we are assuming 1 to 1 mapping of Qs to vectors*/

void get_configuration_from_primary(int vector)
{
	struct remote_eth_dev_req request;
	request.header.from_cpu = smp_processor_id();
	request.header.prio=PCN_KMSG_PRIO_NORMAL;
	request.header.type=PCN_KMSG_TYPE_ETH_MAP_REQ;
	
	request.kernel_id = request.header.from_cpu; 
	request.vector=vector;
	pcn_kmsg_send_long(0,(struct pcn_kmsg_long_message*)&request,sizeof(struct remote_eth_dev_req )-sizeof(request.header));
	down(&wait_config);
	
}




static int handle_remote_eth_dev_info_response(struct pcn_kmsg_message* inc_msg) 
{
	printk("%s: called\n",__func__);
	memcpy(&config,inc_msg,sizeof(struct remote_eth_dev_response));
	pcn_kmsg_free_msg(inc_msg);
	printk("%s: pci_base %x pci_size %d \n",__func__,config.pci_base_adress,config.pci_size);
	up(&wait_config);
}

static int handle_remote_eth_dev_info_request(struct pcn_kmsg_message* inc_msg) 
{
	
	printk("%s: called \n",__func__);
	struct remote_eth_dev_req* req_ptr = (struct remote_eth_dev_req*)inc_msg;
	int kernel_id = req_ptr->kernel_id;
	
	struct ixgbe_adapter * dev_ptr=get_mapping_info_ixgbe_dev();
	struct remote_eth_dev_response response;	
	int irq;
	int vector;
	response.header.from_cpu=smp_processor_id();
	response.header.is_lg_msg=1;
	response.header.prio=PCN_KMSG_PRIO_NORMAL;
	response.header.type=PCN_KMSG_TYPE_ETH_MAP_RES;
	
	
	irq = get_irq(kernel_id); // we assume one Q per secondary kernel. so 1 irq. we need 1 to 1 mapping between Q and irq
//	vector = get_vector(irq);

	pop_msi_set_affinity(irq,kernel_id,req_ptr->vector);
	
	response.irq = irq;
	response.msi_vector = vector;
	response.pci_base_adress = pci_resource_start(dev_ptr->pdev, 0);
	response.pci_size = pci_resource_len(dev_ptr->pdev, 0);
	response.eth_dev_base_add = get_adapter_phy();
	response.max_kernels = get_max_kernel();
	dev_ptr->q_vec_phy[kernel_id]=virt_to_phys(dev_ptr->q_vector[kernel_id]);
	dev_ptr->tx_ring_phys[kernel_id]=virt_to_phys(dev_ptr->tx_ring[kernel_id]);
	dev_ptr->rx_ring_phys[kernel_id]=virt_to_phys(dev_ptr->rx_ring[kernel_id]);
	
	printk("%s: kernel_id %d dev_ptr->tx_ring_phys[kernel_id] %p \n",__func__,kernel_id,dev_ptr->tx_ring_phys[kernel_id]);
	print_tx_ring_desc(kernel_id);
	pcn_kmsg_send_long(kernel_id, 
                       (struct pcn_kmsg_long_message*)&response, 
                       sizeof(struct remote_eth_dev_response) - sizeof(response.header));
    pcn_kmsg_free_msg(inc_msg);
	
}

static int handle_remote_flow_red_request(struct pcn_kmsg_message* inc_msg)
{
	struct remote_flow_redirect_req * req_ptr = (struct remote_flow_redirect_req*)inc_msg;
	struct remote_flow_redirect_res response;
	int port = req_ptr->port;
	int temp;
	int kernel_id = req_ptr->kernel_id;
	temp = htons(port);
	if(flow_info_data[temp]==NULL)
	{
		flow_info_data[temp] = (flow_info*)vmalloc(sizeof(flow_info));
		atomic_set(&flow_info_data[temp]->ref,0);
		atomic_set(&flow_info_data[temp]->state,FLOW_ACTIVE);
		flow_info_data[temp]->port = port;
		flow_info_data[temp]->queue = smp_processor_id();
	}
	else 
	{
		atomic_set(&flow_info_data[temp]->ref,0);
		atomic_set(&flow_info_data[temp]->state,FLOW_ACTIVE);
		flow_info_data[temp]->port = port;
		flow_info_data[temp]->queue = smp_processor_id();
	}
	response.header.from_cpu = smp_processor_id();
	response.header.prio=PCN_KMSG_PRIO_NORMAL;
	response.header.type=PCN_KMSG_TYPE_FLW_RED_RES;
	response.status=1;
	
	pcn_kmsg_send_long(kernel_id, 
                       (struct pcn_kmsg_long_message*)&response, 
                       sizeof(struct remote_flow_redirect_res) - sizeof(response.header));
	
	 pcn_kmsg_free_msg(inc_msg);
	
	
}
static int handle_remote_flow_red_response(struct pcn_kmsg_message* inc_msg) 
{
	pcn_kmsg_free_msg(inc_msg);
	up(&wait_red_confrim);
}
static int handle_remote_loadinfo_request(struct pcn_kmsg_message* inc_msg) 
{
	struct remote_loadinfo_req* request = (struct remote_loadinfo_req* ) inc_msg;
	struct remote_loadinfo_res response; 
	int who_asked  = request->kernel_id;
	
	response.header.from_cpu = smp_processor_id();
	response.header.prio=PCN_KMSG_PRIO_NORMAL;
	response.header.type=PCN_KMSG_TYPE_LDINF_RES;
	
	response.avg_load= avg_load;
	response.syn_fin_bal = get_sync_count();
	
	pcn_kmsg_send_long(who_asked, 
                       (struct pcn_kmsg_long_message*)&response, 
                       sizeof(struct remote_loadinfo_res) - sizeof(response.header));
                       
     pcn_kmsg_free_msg(inc_msg);             
		
}

static int handle_remote_loadinfo_response(struct pcn_kmsg_message* inc_msg) 
{
	struct remote_loadinfo_res * response = (struct remote_loadinfo_res*)inc_msg;
	int kernel_id = response->header.from_cpu;
	if(((response->avg_load)/BASE_LOAD)>=CPU_THRESH)
	{
		remote_cpu_info[kernel_id].avg_load=0xFFFFFFFF;
		remote_cpu_info[kernel_id].syn_fin_bal=0xFFFFFFFF;
		remote_cpu_info[kernel_id].kernel_id=kernel_id;
	}
	else
	{
		remote_cpu_info[kernel_id].avg_load=response->avg_load;
		remote_cpu_info[kernel_id].syn_fin_bal=response->syn_fin_bal;
		remote_cpu_info[kernel_id].kernel_id=kernel_id;
	}
	pcn_kmsg_free_msg(inc_msg);
	up(&wait_load_update);
}





/*packet forwarding*/


module_init(snull_init_module);
module_exit(snull_cleanup);
