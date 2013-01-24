/*
    *  SHMTUN - Popcorn shared-memory network tunnel.
     *  Copyright (C) 2012-2013 Ben Shelton <beshelto@vt.edu>
      */


#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#define DRV_NAME	"shmtun"
#define DRV_VERSION	"1.0"
#define DRV_DESCRIPTION	"Popcorn shared memory network tunnel device driver"
#define DRV_COPYRIGHT	"(C) 2011-2012 Ben Shelton <beshelto@vt.edu>"

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/miscdevice.h>
#include <linux/ethtool.h>
#include <linux/rtnetlink.h>
#include <linux/compat.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_shmtun.h>
#include <linux/crc32.h>
#include <linux/nsproxy.h>
#include <linux/virtio_net.h>
#include <linux/rcupdate.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <net/rtnetlink.h>
#include <net/sock.h>

#include <asm/system.h>
#include <asm/uaccess.h>

/* Uncomment to enable debugging */
/* #define TUN_DEBUG 1 */

#ifdef TUN_DEBUG
static int debug;

#define tun_debug(level, tun, fmt, args...)			\
do {								\
	if (tun->debug)						\
		netdev_printk(level, tun->dev, fmt, ##args);	\
} while (0)
#define DBG1(level, fmt, args...)				\
do {								\
	if (debug == 2)						\
		printk(level fmt, ##args);			\
} while (0)
#else
#define tun_debug(level, tun, fmt, args...)			\
do {								\
	if (0)							\
		netdev_printk(level, tun->dev, fmt, ##args);	\
} while (0)
#define DBG1(level, fmt, args...)				\
do {								\
	if (0)							\
		printk(level fmt, ##args);			\
} while (0)
#endif

struct tun_file {
	atomic_t count;
	struct tun_struct *tun;
	struct net *net;
};

struct tun_sock;

struct tun_struct {
	struct tun_file		*tfile;
	unsigned int 		flags;
	uid_t			owner;
	gid_t			group;

	struct net_device	*dev;
	u32			set_features;
#define TUN_USER_FEATURES (NETIF_F_HW_CSUM|NETIF_F_TSO_ECN|NETIF_F_TSO| \
			  NETIF_F_TSO6|NETIF_F_UFO)
	struct fasync_struct	*fasync;

	int			vnet_hdr_sz;

#ifdef TUN_DEBUG
	int debug;
#endif
};

struct tun_sock {
	struct sock		sk;
	struct tun_struct	*tun;
};

static inline struct tun_sock *tun_sk(struct sock *sk)
{
	return container_of(sk, struct tun_sock, sk);
}

#define RB_SHIFT 8
#define RB_SIZE (1 << RB_SHIFT)
#define RB_MASK ((1 << RB_SHIFT) - 1)

typedef struct shmem_pkt {
	int	pkt_len;
	char	data[ETH_DATA_LEN];
} shmem_pkt_t;

typedef struct rb {
	shmem_pkt_t buffer[RB_SIZE];
	unsigned long head;
	unsigned long tail;
} rb_t;

typedef struct shmem_tun {
	rb_t	to_host;
	rb_t	to_guest;
	int	int_enabled;
} shmem_tun_t;

static inline unsigned long rb_inuse(rb_t *rbuf) {
	return rbuf->head - rbuf->tail;
}

static inline int rb_put(rb_t *rbuf, char *data, int len) {
	if (rb_inuse(rbuf) != RB_SIZE) {
		rbuf->buffer[rbuf->head & RB_MASK].pkt_len = len;
		memcpy(&(rbuf->buffer[(rbuf->head & RB_MASK)].data), data, len);
		rbuf->head++;
		printk("RBUF PUT: size %lu, head %lu, tail %lu\n", rbuf->head - rbuf->tail, rbuf->head, rbuf->tail);
		return 0;
	} else {
		return -1;
	}
}

static inline int rb_get(rb_t *rbuf, shmem_pkt_t **pkt) {
	if (rb_inuse(rbuf) != 0) {
		*pkt = &(rbuf->buffer[rbuf->tail & RB_MASK]);
		printk("RBUF GET: size %lu, head %lu, tail %lu\n", rbuf->head - rbuf->tail, rbuf->head, rbuf->tail);
		return 0;
	} else {
		return -1;
	}
}

static inline void rb_advance_tail(rb_t *rbuf) {
	rbuf->tail++;	
}

#define SHMTUN_MAX_CPUS 4
#define SHMEM_SIZE 0x2000000

unsigned long long global_shmtun_phys_addr = 0x0;

static int __init _setup_shmtun_offset(char *str)
{
	global_shmtun_phys_addr = simple_strtoull(str, 0, 16);
	printk(KERN_ALERT "SHMTUN offset %llx\n", global_shmtun_phys_addr);
	return 0;
}
early_param("shmtun_offset", _setup_shmtun_offset);

static shmem_tun_t *shmem_window;

static struct sk_buff * shmtun_get_pkt_from_rbuf(rb_t *rbuf);
static ssize_t shmtun_put_shmem(struct tun_struct *tun,
		                struct sk_buff *skb);
static int shmtun_napi_handler(struct napi_struct *napi, int budget);

static struct tun_struct *global_tun = NULL;
static int global_cpu = 0;
static struct napi_struct global_napi;

#define SHMTUN_IS_SERVER (global_cpu == 0)

void smp_popcorn_net_interrupt(struct pt_regs *regs)
{
	ack_APIC_irq();
	printk("Interrupt received!\n");

	inc_irq_stat(irq_popcorn_net_count);

	irq_enter();
	
	/* NAPI stuff */

	if (likely(napi_schedule_prep(&global_napi))) {
		/* Disable RX interrupt */
		printk("Turning off RX interrupt...\n");
		shmem_window[global_cpu].int_enabled = 0;	
		
		printk("Calling __napi_schedule...\n");
		/* Schedule NAPI processing */
		__napi_schedule(&global_napi);
	}

	irq_exit();

	return;
}

static struct sk_buff * shmtun_rx_next_pkt(void)
{
	struct sk_buff *skb;
	static int cur_cpu = 0; 
	
	int start_cpu;

	if (SHMTUN_IS_SERVER) {

		start_cpu = cur_cpu;

		/* need to go through all the buffers round-robin */
		do {
			if ((skb = shmtun_get_pkt_from_rbuf(&shmem_window[cur_cpu % SHMTUN_MAX_CPUS].to_host))) {
				break;
			}
			cur_cpu = (cur_cpu + 1) % SHMTUN_MAX_CPUS;
		} while (cur_cpu != start_cpu);
	} else {
		/* only need to check my own buffer */
		skb = shmtun_get_pkt_from_rbuf(&(shmem_window[global_cpu].to_guest));
	}

	return skb;
}

static int shmtun_napi_handler(struct napi_struct *napi, int budget)
{
	struct sk_buff *skb;
	int work_done = 0;

	printk("Called shmtun_napi_handler\n");

	/* go through ring buffer and get packets, up to budget */

	while ((skb = shmtun_rx_next_pkt()) && work_done < budget) {
		napi_gro_receive(napi, skb);
		work_done++;
	}

	printk("Total work done: %d\n", work_done);

	if (work_done < budget) {

		printk("Going back to interrupt mode!\n");

		napi_gro_flush(napi);
		__napi_complete(napi);
		/* turn interrupts back on */
		shmem_window[global_cpu].int_enabled = 1;
	}

	return work_done;
}

static int shmtun_attach(struct tun_struct *tun, struct file *file)
{
	struct tun_file *tfile = file->private_data;
	int err;

	printk("Called shmtun_attach!\n");

	global_tun = tun;

	ASSERT_RTNL();

	netif_tx_lock_bh(tun->dev);

	err = -EINVAL;
	if (tfile->tun)
		goto out;

	err = -EBUSY;
	if (tun->tfile)
		goto out;

	err = 0;
	tfile->tun = tun;
	tun->tfile = tfile;
	netif_carrier_on(tun->dev);
	dev_hold(tun->dev);
	atomic_inc(&tfile->count);

out:
	netif_tx_unlock_bh(tun->dev);
	return err;
}

static void __shmtun_detach(struct tun_struct *tun)
{
	/* Detach from net device */
	netif_tx_lock_bh(tun->dev);
	netif_carrier_off(tun->dev);
	tun->tfile = NULL;
	netif_tx_unlock_bh(tun->dev);

	/* Drop the extra count on the net device */
	dev_put(tun->dev);
}

static void shmtun_detach(struct tun_struct *tun)
{
	rtnl_lock();
	__shmtun_detach(tun);
	rtnl_unlock();
}

static struct tun_struct *__shmtun_get(struct tun_file *tfile)
{
	struct tun_struct *tun = NULL;

	if (atomic_inc_not_zero(&tfile->count))
		tun = tfile->tun;

	return tun;
}

static struct tun_struct *shmtun_get(struct file *file)
{
	return __shmtun_get(file->private_data);
}

static void shmtun_put(struct tun_struct *tun)
{
	struct tun_file *tfile = tun->tfile;

	if (atomic_dec_and_test(&tfile->count))
		shmtun_detach(tfile->tun);
}

/* Network device part of the driver */

static const struct ethtool_ops shmtun_ethtool_ops;

/* Net device detach from fd. */
static void shmtun_net_uninit(struct net_device *dev)
{
	struct tun_struct *tun = netdev_priv(dev);
	struct tun_file *tfile = tun->tfile;

	/* Inform the methods they need to stop using the dev.
	 */
	if (tfile) {
		if (atomic_dec_and_test(&tfile->count))
			__shmtun_detach(tun);
	}
}

static void shmtun_free_netdev(struct net_device *dev)
{
	return;
}

/* Net device open. */
static int shmtun_net_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

/* Net device close. */
static int shmtun_net_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

/* Net device start xmit */
static netdev_tx_t shmtun_net_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct tun_struct *tun = netdev_priv(dev);
	int rc;
	//printk("Called tun_net_xmit, length %d\n", skb->len);

	tun_debug(KERN_INFO, tun, "tun_net_xmit %d\n", skb->len);

	/* Drop packet if interface is not attached */
	if (!tun->tfile)
		goto drop;

	rc = shmtun_put_shmem(tun, skb);

	//printk("Returning NETDEV_TX_OK, rc = %d\n", rc);

	kfree_skb(skb);

	return NETDEV_TX_OK;

drop:
	printk("Dropping packet!\n");
	dev->stats.tx_dropped++;
	kfree_skb(skb);
	return NETDEV_TX_OK;
}

#define MIN_MTU 68
#define MAX_MTU 65535

static int
shmtun_net_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu < MIN_MTU || new_mtu + dev->hard_header_len > MAX_MTU)
		return -EINVAL;
	dev->mtu = new_mtu;
	return 0;
}

static u32 shmtun_net_fix_features(struct net_device *dev, u32 features)
{
	struct tun_struct *tun = netdev_priv(dev);

	return (features & tun->set_features) | (features & ~TUN_USER_FEATURES);
}
#ifdef CONFIG_NET_POLL_CONTROLLER
static void shmtun_poll_controller(struct net_device *dev)
{

	printk("Called tun_poll_controller...\n");

	/*
	 * Tun only receives frames when:
	 * 1) the char device endpoint gets data from user space
	 * 2) the tun socket gets a sendmsg call from user space
	 * Since both of those are syncronous operations, we are guaranteed
	 * never to have pending data when we poll for it
	 * so theres nothing to do here but return.
	 * We need this though so netpoll recognizes us as an interface that
	 * supports polling, which enables bridge devices in virt setups to
	 * still use netconsole
	 */
	return;
}
#endif
static const struct net_device_ops shmtun_netdev_ops = {
	.ndo_uninit		= shmtun_net_uninit,
	.ndo_open		= shmtun_net_open,
	.ndo_stop		= shmtun_net_close,
	.ndo_start_xmit		= shmtun_net_xmit,
	.ndo_change_mtu		= shmtun_net_change_mtu,
	.ndo_fix_features	= shmtun_net_fix_features,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= shmtun_poll_controller,
#endif
};

/* Initialize net device. */
static void shmtun_net_init(struct net_device *dev)
{
	//struct tun_struct *tun = netdev_priv(dev);
	//int cpu;

	printk("Called shmtun_net_init!\n");

	shmem_window = ioremap_cache(global_shmtun_phys_addr, SHMEM_SIZE);

	printk("Mapped shared memory window, virt addr 0x%p\n", shmem_window);


	global_cpu = raw_smp_processor_id();

	printk("Raw SMP processor ID: %d\n", global_cpu);

	if (global_cpu == 0) {
		printk("We're the server...\n");
	} else {
		printk("We're the client...\n");
	}

	shmem_window[global_cpu].int_enabled = 1;

	netif_napi_add(dev, &global_napi, shmtun_napi_handler, 64);

	napi_enable(&global_napi);

	dev->netdev_ops = &shmtun_netdev_ops;

	/* Point-to-Point TUN Device */
	dev->hard_header_len = 0;
	dev->addr_len = 0;
	dev->mtu = 1500;

	/* Zero header length */
	dev->type = ARPHRD_NONE;
	dev->flags = IFF_POINTOPOINT | IFF_NOARP | IFF_MULTICAST;
	dev->tx_queue_len = TUN_READQ_SIZE;  /* We prefer our own queue length */
}

/* Character device part */

static struct sk_buff * shmtun_get_pkt_from_rbuf(rb_t *rbuf)
{
	struct sk_buff *skb;
	struct shmem_pkt *pkt;
	int rc;

	rc = rb_get(rbuf, &pkt);

	if (rc) {
		//printk("No packet in ring buffer, returning...\n");
		return NULL;
	}

	printk("Received packet of pkt_len %d\n", pkt->pkt_len);

	skb = dev_alloc_skb(pkt->pkt_len + 2);

	if (!skb) {
		printk("Unable to allocate skb for received packet; dropping it!\n");
		return NULL;
	}

	skb_reserve(skb, 2);  /* align IP on 16B boundary */

	memcpy(skb_put(skb, pkt->pkt_len), pkt->data, pkt->pkt_len);

	/* POPCORN -- we're safe to move the tail pointer at this point */
	rb_advance_tail(rbuf);

	skb->protocol = htons(ETH_P_IP);
	skb_reset_mac_header(skb);
	skb->dev = global_tun->dev;
	skb->ip_summed = CHECKSUM_UNNECESSARY;

	return skb;
}

/* Put packet to the shared memory buffer */
static ssize_t shmtun_put_shmem(struct tun_struct *tun,
		struct sk_buff *skb)
{
	int rc, len, sendto_cpu = 0;
	char *data, shortpkt[ETH_ZLEN];
	ssize_t total = 0;
	rb_t *send_buf;

	printk("Called tun_put_shmem, len = %d\n", skb->len);

	/* code from LDD3 example */
	data = skb->data;
	len = skb->len;
	if (len < ETH_ZLEN) {
		memset(shortpkt, 0, ETH_ZLEN);
		memcpy(shortpkt, skb->data, skb->len);
		len = ETH_ZLEN;
		data = shortpkt;
	}
	/*
	printk("SEND from %d.%d.%d.%d to %d.%d.%d.%d\n",
			(int) data[12], (int) data[13], (int) data[14], (int) data[15],
			(int) data[16], (int) data[17], (int) data[18], (int) data[19]
	      );
	*/
	if (0) { /* enable this conditional to look at the data */
		int i;
		printk("len is %i\n" KERN_DEBUG "data:",len);
		for (i=14 ; i<len; i++)
			printk(" %02x",data[i]&0xff);
		printk("\n");
	}

	

	if (SHMTUN_IS_SERVER) {
		sendto_cpu = data[19] - 1;
		send_buf = &(shmem_window[sendto_cpu].to_guest);
	} else {
		sendto_cpu = 0;
		/* client sends to server and server forwards */
		send_buf = &(shmem_window[data[15] - 1].to_host);
	}

	/* POPCORN -- copy to shared memory buffer */
	rc = rb_put(send_buf, data, len);

	if (rc) {
		printk("No space in ring buffer, dropping packet...\n");
		tun->dev->stats.tx_dropped++;
		return 0;
	}

	/* POPCORN -- if RX interrupts are enabled, send IPI to receiver */

	if (shmem_window[sendto_cpu].int_enabled) {

		printk("Interrupts enabled for CPU %d, sending IPI...\n", sendto_cpu);

		if (SHMTUN_IS_SERVER) {
			printk("Sending IPI to CPU 2...\n");
			apic->send_IPI_single(sendto_cpu, POPCORN_NET_VECTOR);
			//apic->send_IPI_mask(cpumask_of(sendto_cpu), POPCORN_NET_VECTOR);
		} else {
			printk("Sending IPI to CPU 0...\n");
			apic->send_IPI_single(0, POPCORN_NET_VECTOR);
			//apic->send_IPI_mask(cpumask_of(0), POPCORN_NET_VECTOR);
		}
	}

	total += skb->len;

	tun->dev->stats.tx_packets++;
	tun->dev->stats.tx_bytes += len;

	return total;
}

static void shmtun_setup(struct net_device *dev)
{
	struct tun_struct *tun = netdev_priv(dev);

	tun->owner = -1;
	tun->group = -1;

	dev->ethtool_ops = &shmtun_ethtool_ops;
	dev->destructor = shmtun_free_netdev;
}

/* Trivial set of netlink ops to allow deleting tun or tap
 * device with netlink.
 */
static int shmtun_validate(struct nlattr *tb[], struct nlattr *data[])
{
	return -EINVAL;
}

static struct rtnl_link_ops tun_link_ops __read_mostly = {
	.kind		= DRV_NAME,
	.priv_size	= sizeof(struct tun_struct),
	.setup		= shmtun_setup,
	.validate	= shmtun_validate,
};

static int tun_flags(struct tun_struct *tun)
{
	int flags = 0;

	if (tun->flags & TUN_TUN_DEV)
		flags |= IFF_TUN;
	else
		flags |= IFF_TAP;

	if (tun->flags & TUN_NO_PI)
		flags |= IFF_NO_PI;

	if (tun->flags & TUN_ONE_QUEUE)
		flags |= IFF_ONE_QUEUE;

	if (tun->flags & TUN_VNET_HDR)
		flags |= IFF_VNET_HDR;

	return flags;
}

static ssize_t tun_show_flags(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct tun_struct *tun = netdev_priv(to_net_dev(dev));
	return sprintf(buf, "0x%x\n", tun_flags(tun));
}

static ssize_t tun_show_owner(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct tun_struct *tun = netdev_priv(to_net_dev(dev));
	return sprintf(buf, "%d\n", tun->owner);
}

static ssize_t tun_show_group(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct tun_struct *tun = netdev_priv(to_net_dev(dev));
	return sprintf(buf, "%d\n", tun->group);
}

static DEVICE_ATTR(tun_flags, 0444, tun_show_flags, NULL);
static DEVICE_ATTR(owner, 0444, tun_show_owner, NULL);
static DEVICE_ATTR(group, 0444, tun_show_group, NULL);

static int shmtun_set_iff(struct net *net, struct file *file, struct ifreq *ifr)
{
	struct tun_struct *tun;
	struct net_device *dev;
	int err;

	printk("Reached shmtun_set_iff!\n");

	dev = __dev_get_by_name(net, ifr->ifr_name);
	if (dev) {
		const struct cred *cred = current_cred();

		if (ifr->ifr_flags & IFF_TUN_EXCL)
			return -EBUSY;
		if ((ifr->ifr_flags & IFF_TUN) && dev->netdev_ops == &shmtun_netdev_ops)
			tun = netdev_priv(dev);
		else
			return -EINVAL;

		if (((tun->owner != -1 && cred->euid != tun->owner) ||
		     (tun->group != -1 && !in_egroup_p(tun->group))) &&
		    !capable(CAP_NET_ADMIN))
			return -EPERM;

		err = shmtun_attach(tun, file);
		if (err < 0)
			return err;
	}
	else {
		char *name;
		unsigned long flags = 0;

		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		err = security_tun_dev_create();
		if (err < 0)
			return err;

		/* Set dev type */
		if (ifr->ifr_flags & IFF_TUN) {
			/* SHMTUN device */
			flags |= TUN_TUN_DEV;
			name = "shmtun%d";
		} else if (ifr->ifr_flags & IFF_TAP) {
			printk("TAP DEVICE -- SHOULD NOT GET HERE!\n");
			/* TAP device */
			flags |= TUN_TAP_DEV;
			name = "tap%d";
		} else
			return -EINVAL;

		if (*ifr->ifr_name)
			name = ifr->ifr_name;

		dev = alloc_netdev(sizeof(struct tun_struct), name,
				   shmtun_setup);
		if (!dev)
			return -ENOMEM;

		dev_net_set(dev, net);
		dev->rtnl_link_ops = &tun_link_ops;

		tun = netdev_priv(dev);
		tun->dev = dev;
		tun->flags = flags;
		tun->vnet_hdr_sz = sizeof(struct virtio_net_hdr);

		shmtun_net_init(dev);

		dev->hw_features = NETIF_F_SG | NETIF_F_FRAGLIST |
			TUN_USER_FEATURES;
		dev->features = dev->hw_features;

		err = register_netdevice(tun->dev);
		if (err < 0)
			goto err_free_sk;

		if (device_create_file(&tun->dev->dev, &dev_attr_tun_flags) ||
		    device_create_file(&tun->dev->dev, &dev_attr_owner) ||
		    device_create_file(&tun->dev->dev, &dev_attr_group))
			pr_err("Failed to create tun sysfs files\n");

		err = shmtun_attach(tun, file);
		if (err < 0)
			goto failed;
	}

	tun_debug(KERN_INFO, tun, "shmtun_set_iff\n");

	if (ifr->ifr_flags & IFF_NO_PI)
		tun->flags |= TUN_NO_PI;
	else
		tun->flags &= ~TUN_NO_PI;

	if (ifr->ifr_flags & IFF_ONE_QUEUE)
		tun->flags |= TUN_ONE_QUEUE;
	else
		tun->flags &= ~TUN_ONE_QUEUE;

	if (ifr->ifr_flags & IFF_VNET_HDR)
		tun->flags |= TUN_VNET_HDR;
	else
		tun->flags &= ~TUN_VNET_HDR;

	/* Make sure persistent devices do not get stuck in
	 * xoff state.
	 */
	if (netif_running(tun->dev))
		netif_wake_queue(tun->dev);

	strcpy(ifr->ifr_name, tun->dev->name);

	printk("shmtun_set_iff returned 0!\n");
	
	return 0;

 err_free_sk:
	free_netdev(dev);
 failed:
	printk("shmtun_set_iff returned error %d\n", err);

	return err;
}

static int shmtun_get_iff(struct net *net, struct tun_struct *tun,
		       struct ifreq *ifr)
{
	tun_debug(KERN_INFO, tun, "shmtun_get_iff\n");

	strcpy(ifr->ifr_name, tun->dev->name);

	ifr->ifr_flags = tun_flags(tun);

	return 0;
}

/* This is like a cut-down ethtool ops, except done via tun fd so no
 * privs required. */
static int set_offload(struct tun_struct *tun, unsigned long arg)
{
	u32 features = 0;

	if (arg & TUN_F_CSUM) {
		features |= NETIF_F_HW_CSUM;
		arg &= ~TUN_F_CSUM;

		if (arg & (TUN_F_TSO4|TUN_F_TSO6)) {
			if (arg & TUN_F_TSO_ECN) {
				features |= NETIF_F_TSO_ECN;
				arg &= ~TUN_F_TSO_ECN;
			}
			if (arg & TUN_F_TSO4)
				features |= NETIF_F_TSO;
			if (arg & TUN_F_TSO6)
				features |= NETIF_F_TSO6;
			arg &= ~(TUN_F_TSO4|TUN_F_TSO6);
		}

		if (arg & TUN_F_UFO) {
			features |= NETIF_F_UFO;
			arg &= ~TUN_F_UFO;
		}
	}

	/* This gives the user a way to test for new features in future by
	 * trying to set them. */
	if (arg)
		return -EINVAL;

	tun->set_features = features;
	netdev_update_features(tun->dev);

	return 0;
}

static long __shmtun_chr_ioctl(struct file *file, unsigned int cmd,
			    unsigned long arg, int ifreq_len)
{
	struct tun_file *tfile = file->private_data;
	struct tun_struct *tun;
	void __user* argp = (void __user*)arg;
	struct ifreq ifr;
	int vnet_hdr_sz;
	int ret;

	printk("Called __shmtun_chr_ioctl!\n");

	if (cmd == TUNSETIFF || _IOC_TYPE(cmd) == 0x89)
		if (copy_from_user(&ifr, argp, ifreq_len))
			return -EFAULT;

	if (cmd == TUNGETFEATURES) {
		/* Currently this just means: "what IFF flags are valid?".
		 * This is needed because we never checked for invalid flags on
		 * TUNSETIFF. */
		return put_user(IFF_TUN | IFF_TAP | IFF_NO_PI | IFF_ONE_QUEUE |
				IFF_VNET_HDR,
				(unsigned int __user*)argp);
	}

	rtnl_lock();

	tun = __shmtun_get(tfile);
	if (cmd == TUNSETIFF && !tun) {

		ifr.ifr_name[IFNAMSIZ-1] = '\0';

		ret = shmtun_set_iff(tfile->net, file, &ifr);

		if (ret)
			goto unlock;

		if (copy_to_user(argp, &ifr, ifreq_len))
			ret = -EFAULT;
		goto unlock;
	}

	ret = -EBADFD;
	if (!tun)
		goto unlock;

	tun_debug(KERN_INFO, tun, "tun_chr_ioctl cmd %d\n", cmd);

	ret = 0;
	switch (cmd) {
	case TUNGETIFF:
		ret = shmtun_get_iff(current->nsproxy->net_ns, tun, &ifr);
		if (ret)
			break;

		if (copy_to_user(argp, &ifr, ifreq_len))
			ret = -EFAULT;
		break;

	case TUNSETNOCSUM:
		/* Disable/Enable checksum */

		/* [unimplemented] */
		tun_debug(KERN_INFO, tun, "ignored: set checksum %s\n",
			  arg ? "disabled" : "enabled");
		break;

	case TUNSETPERSIST:
		/* Disable/Enable persist mode */
		if (arg)
			tun->flags |= TUN_PERSIST;
		else
			tun->flags &= ~TUN_PERSIST;

		tun_debug(KERN_INFO, tun, "persist %s\n",
			  arg ? "enabled" : "disabled");
		break;

	case TUNSETOWNER:
		/* Set owner of the device */
		tun->owner = (uid_t) arg;

		tun_debug(KERN_INFO, tun, "owner set to %d\n", tun->owner);
		break;

	case TUNSETGROUP:
		/* Set group of the device */
		tun->group= (gid_t) arg;

		tun_debug(KERN_INFO, tun, "group set to %d\n", tun->group);
		break;

	case TUNSETLINK:
		/* Only allow setting the type when the interface is down */
		if (tun->dev->flags & IFF_UP) {
			tun_debug(KERN_INFO, tun,
				  "Linktype set failed because interface is up\n");
			ret = -EBUSY;
		} else {
			tun->dev->type = (int) arg;
			tun_debug(KERN_INFO, tun, "linktype set to %d\n",
				  tun->dev->type);
			ret = 0;
		}
		break;

#ifdef TUN_DEBUG
	case TUNSETDEBUG:
		tun->debug = arg;
		break;
#endif
	case TUNSETOFFLOAD:
		ret = set_offload(tun, arg);
		break;

	case TUNSETTXFILTER:
		/* Can be set only for TAPs */
		ret = -EINVAL;
		break;

	case SIOCGIFHWADDR:
		/* Get hw address */
		memcpy(ifr.ifr_hwaddr.sa_data, tun->dev->dev_addr, ETH_ALEN);
		ifr.ifr_hwaddr.sa_family = tun->dev->type;
		if (copy_to_user(argp, &ifr, ifreq_len))
			ret = -EFAULT;
		break;

	case SIOCSIFHWADDR:
		/* Set hw address */
		tun_debug(KERN_DEBUG, tun, "set hw address: %pM\n",
			  ifr.ifr_hwaddr.sa_data);

		ret = dev_set_mac_address(tun->dev, &ifr.ifr_hwaddr);
		break;

	case TUNGETSNDBUF:
		break;

	case TUNSETSNDBUF:
		break;

	case TUNGETVNETHDRSZ:
		vnet_hdr_sz = tun->vnet_hdr_sz;
		if (copy_to_user(argp, &vnet_hdr_sz, sizeof(vnet_hdr_sz)))
			ret = -EFAULT;
		break;

	case TUNSETVNETHDRSZ:
		if (copy_from_user(&vnet_hdr_sz, argp, sizeof(vnet_hdr_sz))) {
			ret = -EFAULT;
			break;
		}
		if (vnet_hdr_sz < (int)sizeof(struct virtio_net_hdr)) {
			ret = -EINVAL;
			break;
		}

		tun->vnet_hdr_sz = vnet_hdr_sz;
		break;

	case TUNATTACHFILTER:
		/* Can be set only for TAPs */
		break;

	case TUNDETACHFILTER:
		/* Can be set only for TAPs */
		break;

	default:
		ret = -EINVAL;
		break;
	}

unlock:
	rtnl_unlock();
	if (tun)
		shmtun_put(tun);
	return ret;
}

static long shmtun_chr_ioctl(struct file *file,
			  unsigned int cmd, unsigned long arg)
{
	return __shmtun_chr_ioctl(file, cmd, arg, sizeof (struct ifreq));
}

#ifdef CONFIG_COMPAT
static long shmtun_chr_compat_ioctl(struct file *file,
			 unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case TUNSETIFF:
	case TUNGETIFF:
	case TUNSETTXFILTER:
	case TUNGETSNDBUF:
	case TUNSETSNDBUF:
	case SIOCGIFHWADDR:
	case SIOCSIFHWADDR:
		arg = (unsigned long)compat_ptr(arg);
		break;
	default:
		arg = (compat_ulong_t)arg;
		break;
	}

	/*
	 * compat_ifreq is shorter than ifreq, so we must not access beyond
	 * the end of that structure. All fields that are used in this
	 * driver are compatible though, we don't need to convert the
	 * contents.
	 */
	return __shmtun_chr_ioctl(file, cmd, arg, sizeof(struct compat_ifreq));
}
#endif /* CONFIG_COMPAT */

static int shmtun_chr_fasync(int fd, struct file *file, int on)
{
	struct tun_struct *tun = shmtun_get(file);
	int ret;

	if (!tun)
		return -EBADFD;

	tun_debug(KERN_INFO, tun, "tun_chr_fasync %d\n", on);

	if ((ret = fasync_helper(fd, file, on, &tun->fasync)) < 0)
		goto out;

	if (on) {
		ret = __f_setown(file, task_pid(current), PIDTYPE_PID, 0);
		if (ret)
			goto out;
		tun->flags |= TUN_FASYNC;
	} else
		tun->flags &= ~TUN_FASYNC;
	ret = 0;
out:
	shmtun_put(tun);
	return ret;
}

static int shmtun_chr_open(struct inode *inode, struct file * file)
{
	struct tun_file *tfile;

	DBG1(KERN_INFO, "tunX: shmtun_chr_open\n");

	tfile = kmalloc(sizeof(*tfile), GFP_KERNEL);
	if (!tfile)
		return -ENOMEM;
	atomic_set(&tfile->count, 0);
	tfile->tun = NULL;
	tfile->net = get_net(current->nsproxy->net_ns);
	file->private_data = tfile;
	return 0;
}

static int shmtun_chr_close(struct inode *inode, struct file *file)
{
	struct tun_file *tfile = file->private_data;
	struct tun_struct *tun;

	tun = __shmtun_get(tfile);
	if (tun) {
		struct net_device *dev = tun->dev;

		tun_debug(KERN_INFO, tun, "shmtun_chr_close\n");

		__shmtun_detach(tun);

		/* If desirable, unregister the netdevice. */
		if (!(tun->flags & TUN_PERSIST)) {
			rtnl_lock();
			if (dev->reg_state == NETREG_REGISTERED)
				unregister_netdevice(dev);
			rtnl_unlock();
		}
	}

	put_net(tfile->net);
	kfree(tfile);

	return 0;
}

static const struct file_operations tun_fops = {
	.owner	= THIS_MODULE,
	.llseek = no_llseek,
	.unlocked_ioctl	= shmtun_chr_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = shmtun_chr_compat_ioctl,
#endif
	.open	= shmtun_chr_open,
	.release = shmtun_chr_close,
	.fasync = shmtun_chr_fasync
};

static struct miscdevice shmtun_miscdev = {
	.minor = SHMTUN_MINOR,
	.name = "shmtun",
	.nodename = "net/shmtun",
	.fops = &tun_fops,
};

/* ethtool interface */

static int shmtun_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	cmd->supported		= 0;
	cmd->advertising	= 0;
	ethtool_cmd_speed_set(cmd, SPEED_10);
	cmd->duplex		= DUPLEX_FULL;
	cmd->port		= PORT_TP;
	cmd->phy_address	= 0;
	cmd->transceiver	= XCVR_INTERNAL;
	cmd->autoneg		= AUTONEG_DISABLE;
	cmd->maxtxpkt		= 0;
	cmd->maxrxpkt		= 0;
	return 0;
}

static void shmtun_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct tun_struct *tun = netdev_priv(dev);

	strcpy(info->driver, DRV_NAME);
	strcpy(info->version, DRV_VERSION);
	strcpy(info->fw_version, "N/A");

	switch (tun->flags & TUN_TYPE_MASK) {
	case TUN_TUN_DEV:
		strcpy(info->bus_info, "tun");
		break;
	case TUN_TAP_DEV:
		strcpy(info->bus_info, "tap");
		break;
	}
}

static u32 shmtun_get_msglevel(struct net_device *dev)
{
#ifdef TUN_DEBUG
	struct tun_struct *tun = netdev_priv(dev);
	return tun->debug;
#else
	return -EOPNOTSUPP;
#endif
}

static void shmtun_set_msglevel(struct net_device *dev, u32 value)
{
#ifdef TUN_DEBUG
	struct tun_struct *tun = netdev_priv(dev);
	tun->debug = value;
#endif
}

static const struct ethtool_ops shmtun_ethtool_ops = {
	.get_settings	= shmtun_get_settings,
	.get_drvinfo	= shmtun_get_drvinfo,
	.get_msglevel	= shmtun_get_msglevel,
	.set_msglevel	= shmtun_set_msglevel,
	.get_link	= ethtool_op_get_link,
};


static int __init tun_init(void)
{
	int ret = 0;

	pr_info("%s, %s\n", DRV_DESCRIPTION, DRV_VERSION);
	pr_info("%s\n", DRV_COPYRIGHT);

	ret = rtnl_link_register(&tun_link_ops);
	if (ret) {
		pr_err("Can't register link_ops\n");
		goto err_linkops;
	}

	ret = misc_register(&shmtun_miscdev);
	if (ret) {
		pr_err("Can't register misc device %d\n", SHMTUN_MINOR);
		goto err_misc;
	}
	return  0;
err_misc:
	rtnl_link_unregister(&tun_link_ops);
err_linkops:
	return ret;
}

static void tun_cleanup(void)
{
	misc_deregister(&shmtun_miscdev);
	rtnl_link_unregister(&tun_link_ops);
}

module_init(tun_init);
module_exit(tun_cleanup);
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR(DRV_COPYRIGHT);
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(SHMTUN_MINOR);
MODULE_ALIAS("devname:net/shmtun");
