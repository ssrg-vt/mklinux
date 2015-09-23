

#include <linux/kernel.h>
#include <linux/list.h>



#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/timex.h>
#include <linux/timer.h>
#include <linux/pcn_kmsg.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/jhash.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/net.h>
#include <linux/types.h>
#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/interrupt.h>
#include <net/sock.h>
#include "../igb.h"
#include <linux/pci.h>
#include <net/route.h>
 

//#include <net/net-sysfs.h>
#define PRINT_MESSAGES 0
#if PRINT_MESSAGES
#define PRINTK(...) printk(__VA_ARGS__)
#else
#define PRINTK(...) ;
#endif

#define NR_CPUS 2

#define MAJ 3
#define MIN 2
#define BUILD 10
#define DRV_VERSION __stringify(MAJ) "." __stringify(MIN) "." \
__stringify(BUILD) "-k"
char igb_d_name[] = "igb";
char igb_d_version[] = DRV_VERSION;
static const char igb_driver_string[] =
                                "Intel(R) Gigabit Ethernet Network Driver AKSHAY";
static const char igb_copyright[] = "Copyright (c) 2007-2011 Intel Corporation.";


//TODO: This is needed if ww share pci and can control the device directly
static DEFINE_PCI_DEVICE_TABLE(igb_p_pci_tbl) = {
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I350_COPPER), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I350_FIBER), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I350_SERDES), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_I350_SGMII), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82580_COPPER), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82580_FIBER), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82580_QUAD_FIBER), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82580_SERDES), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82580_SGMII), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82580_COPPER_DUAL), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_DH89XXCC_SGMII), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_DH89XXCC_SERDES), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_DH89XXCC_BACKPLANE), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_DH89XXCC_SFP), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_NS), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_NS_SERDES), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_FIBER), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_SERDES), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_SERDES_QUAD), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_QUAD_COPPER_ET2), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82576_QUAD_COPPER), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82575EB_COPPER), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82575EB_FIBER_SERDES), board_82575 },
	{ PCI_VDEVICE(INTEL, E1000_DEV_ID_82575GB_QUAD_COPPER), board_82575 },
	/* required last entry */
	{0, }
};
MODULE_DEVICE_TABLE(pci, igb_p_pci_tbl);

static struct pci_dev *pd=NULL;
int __devinit igb_p_probe(struct pci_dev *pdev,
                               const struct pci_device_id *ent){
	pd=pdev;
	printk(KERN_ALERT"probe custom made pdev{%p} ent{%d} \n",pdev,ent->device);
	return 0;
}

int igb_p_suspend(struct pci_dev *pdev, pm_message_t state){
	return 0;
}

int __devexit igb_p_remove(struct pci_dev *pdev){
	return 0;
}
int  igb_p_resume(struct pci_dev *pdev){
	return 0;
}

int  igb_p_shutdown(struct pci_dev *pdev){
	return 0;
}

struct pci_driver igb_p_driver = {
        .name     = igb_d_name,
        .id_table = igb_p_pci_tbl,
        .probe    = igb_p_probe,
        .remove   = __devexit_p(igb_p_remove),
#ifdef CONFIG_PM
        /* Power Management Hooks */
        .suspend  = igb_p_suspend,
        .resume   = igb_p_resume,
#endif
        .shutdown = igb_p_shutdown,

};


static DECLARE_WAIT_QUEUE_HEAD( wq_dev);
static int wait_dev = -1;
static struct net_device  *dev;
static int _cpu=0;
extern int __list_netdevice(struct net_device *dev);
extern int netdev_register_kobject(struct net_device *);
extern struct in_ifaddr *inet_alloc_ifa(void);
extern int __inet_set_ifa(struct net_device *dev, struct in_ifaddr *ifa);
extern void igb_set_ethtool_ops(struct net_device *);
extern struct in_device *__inetdev_init(struct net_device *dev);
extern int __ip_rt_ioctl(struct net *net, unsigned int cmd, void  *arg);
extern struct rtable * rt_get_iface(char *name);

struct _device_info{
	char  name[IFNAMSIZ];
	unsigned char perm_addr[MAX_ADDR_LEN]; /* permanent hw address */
	//TODO: Add mac layer details
	//unsigned char           addr_assign_type; /* hw address assignment type */
	//unsigned char           addr_len;       /* hardware address length      */
	//unsigned short          dev_id;         /* for shared network cards */
	__be32 addr;
	__be32 mask;
	__be32 broadcast;
};

typedef struct _device_info device_info_t;

struct _remote_proxy_info_request {
	struct pcn_kmsg_hdr header;
	int cpu;
	int irq;
	char pad[56];
}__attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_proxy_info_request _remote_proxy_info_request_t;

struct _remote_proxy_info_response {
	struct pcn_kmsg_hdr header;
	int count;
	device_info_t data;//TODO: should be dynamic
}__attribute__((packed)) ;

typedef struct _remote_proxy_info_response _remote_proxy_info_response_t;

static void register_dummy_device(){
	int ret =0;
        struct igb_adapter *adapter = NULL;
	 u16 eeprom_data = 0;
        s32 ret_val;

	ret = pci_register_driver(&igb_p_driver);
	if(ret){
        	printk(KERN_ALERT"%s: iret {%d} isnull{%d} \n",__func__,ret,(pd==NULL)?0:1);
		goto out;
	}
        
	dev = alloc_etherdev_mq(sizeof(struct igb_adapter),
                        IGB_MAX_TX_QUEUES);
        
	adapter = netdev_priv(dev);
        adapter->netdev = dev;
        dev->reg_state = NETREG_DUMMY;
	//TODO: Set Ethtool operations
        //      dev->netdev_ops = &igb_netdev_ops;
        //      igb_set_ethtool_ops(dev);
        dev->watchdog_timeo = 5 * HZ;


        INIT_LIST_HEAD(&dev->napi_list);
        set_bit(__LINK_STATE_PRESENT, &dev->state);
        set_bit(__LINK_STATE_START, &dev->state);

        __list_netdevice(dev);

	ret = netdev_register_kobject(dev);	
	
	PRINTK(KERN_ALERT"%s: netdev_register_kobject {%d}\n",__func__,ret);

	__inetdev_init(dev);
out:
	printk(KERN_ALERT"ERROR\n");
}

static void copy_inet_address(char * name, __be32 addr, __be32 mask, __be32 broadcast){
	//allocate ifa
	//TODO: based on count do the below repeatedly
        struct in_ifaddr *ifa = NULL;
	int err = 0;

	ifa = inet_alloc_ifa();

	INIT_HLIST_NODE(&ifa->hash);
	memcpy(ifa->ifa_label, name, IFNAMSIZ);
	printk("ifa label %s\n",ifa->ifa_label);
	ifa->ifa_broadcast = 0;
	ifa->ifa_scope = 0;
	ifa->remote = 1;

	//set address
	ifa->ifa_address = ifa->ifa_local = in_aton("10.1.1.42");//addr;
	ifa->ifa_prefixlen = 32;
	//set mask
	ifa->ifa_mask = mask;
	//set broadcast
	ifa->ifa_broadcast = broadcast;

	err = __inet_set_ifa(dev, ifa);

	printk(KERN_ALERT"%s: local addr %d inet set ifa error {%d}\n",__func__,ifa->ifa_local,err);
	char *add;
	add = &ifa->ifa_local;
	printk(KERN_ALERT"local %d . %d. %d. %d:\n",add[0],add[1],add[2],add[3]);
	add = &ifa->ifa_mask;
	printk(KERN_ALERT"mask %d . %d. %d. %d:\n",add[0],add[1],add[2],add[3]);
	add = &ifa->ifa_broadcast;
	printk(KERN_ALERT"dev name %d . %d. %d. %d:\n",add[0],add[1],add[2],add[3]);
	
}

static void set_routing_table(__be32 address, __be32 mask, __be32 broadcast){
	int err = 0;
	//set routes
	struct socket *res = NULL;
	struct rtentry route;
	struct sockaddr_in *addr,*ad2,*ad3;
	char *add;

	memset(&route, 0, sizeof(route));
	
	addr = (struct sockaddr_in*) &route.rt_gateway;
  	addr->sin_family = AF_INET;
  	addr->sin_addr.s_addr = in_aton("0.0.0.0");;//ifa->ifa_local;

	
	
	ad2 = (struct sockaddr_in*) &route.rt_genmask;
	ad2->sin_family = AF_INET;
  	ad2->sin_addr.s_addr = in_aton("255.255.255.0");//inet_make_mask(ifa->ifa_prefixlen);
	
	ad3 = (struct sockaddr_in*) &route.rt_dst;
 	ad3->sin_family = AF_INET;
  	ad3->sin_addr.s_addr = in_aton("10.1.1.0");;//0x0001010A;//ifa->ifa_local;
	
	route.rt_flags = RTF_UP;// | RTF_GATEWAY;

	add = &addr->sin_addr.s_addr;
	printk(KERN_ALERT"local %d . %d. %d. %d:\n",add[0],add[1],add[2],add[3]);
	add = &ad2->sin_addr.s_addr ;
	printk(KERN_ALERT"mask %d . %d. %d. %d:\n",add[0],add[1],add[2],add[3]);
	add = &ad3->sin_addr.s_addr;
	printk(KERN_ALERT"dev name %d . %d. %d. %d:\n",add[0],add[1],add[2],add[3]);

	err = __sock_create(&init_net, AF_INET, SOCK_STREAM, IPPROTO_IP, &res,0);
	
	printk(KERN_ALERT"%s: __sock_create {%d}\n",__func__,err);

	
	struct sock *sk = res->sk;
        struct net *net = sock_net(sk);

	err = __ip_rt_ioctl(net, SIOCADDRT, &route);

	printk(KERN_ALERT"%s: ip_rt_ioctl {%d}\n",__func__,err);


}
static int handle_remote_proxy_dev_info_response(
		struct pcn_kmsg_message* inc_msg) {
	
	_remote_proxy_info_response_t* msg = (_remote_proxy_info_response_t*) inc_msg;
	int ret =0;
	PRINTK(KERN_ALERT"%s: response ---- wait_cpu_list{%d} \n", __func__, wait_dev);
	int err =0;
       
	register_dummy_device();

  	if(!dev)
	   goto end;

	dev->flags |= IFF_UP;
	
	memcpy(dev->perm_addr, msg->data.perm_addr, dev->addr_len);
	strcpy(dev->name, msg->data.name);
	printk(KERN_ALERT"dev name %s: \n",dev->name);
	//err = register_netdev(dev);
	//if(err)
	//	goto end;

	copy_inet_address(dev->name,msg->data.addr,msg->data.mask,msg->data.broadcast);

	set_routing_table(msg->data.addr,msg->data.mask,msg->data.broadcast);

	wait_dev = 1;
	wake_up_interruptible(&wq_dev);	

	pcn_kmsg_free_msg(inc_msg);
	return 0;

end:
	free_netdev(dev);

}
static int set_remote_irte(struct net_device * netd, int remote_irq){

	struct igb_adapter *adapter = netdev_priv(netd);
	int err = 0;
	if(adapter == NULL){
		err = 100;
		goto out;
	}
	((struct pci_dev *)adapter->pdev)->_master = netd->_master;
	((struct pci_dev *)adapter->pdev)->_for_cpu = netd->_for_cpu;
	int i = 1;

	if(adapter->msix_entries == NULL){
		err = 20;
		goto out;
	}

	adapter->msix_entries[i].entry = i;
	//TODO: This is not working. As pci device is already enabled.
	//Need to set IRTE through some other mechanism
        //err = pci_enable_msix(adapter->pdev,
          //                     adapter->msix_entries,
            //                   1);
        // Get the free IRQ and pass it over Seconadary
out:	
	printk(KERN_ALERT"%s: err %d \n",__func__,err);	

}

static int handle_remote_proxy_dev_info_request(struct pcn_kmsg_message* inc_msg) {

	int i;

	_remote_proxy_info_request_t* msg = (_remote_proxy_info_request_t*) inc_msg;
	_remote_proxy_info_response_t response;

	printk(KERN_ALERT"%s: Entered remote request \n", "handle_remote_proxy_info_request");

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_PROXY_DEV_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;
	struct socket *res = NULL;
	struct net *net = NULL;
	struct in_device *in_dev;
	struct in_ifaddr **ifap = NULL;
	struct in_ifaddr *ifa = NULL;
	struct net_device *dev;
	char *name ="eth0";
	char *add;


	__sock_create(&init_net, PF_INET, SOCK_STREAM, IPPROTO_IP, &res,0);
	printk(KERN_ALERT"after sock create \n");
	struct sock *sk = res->sk;
	net = (struct net *) sock_net(sk);

	dev_load(net, name);
	dev = __dev_get_by_name(net, name);

	printk(KERN_ALERT"master %d for cpu %d \n",dev->_master,dev->_for_cpu);

	dev->_master = 1;
	dev->_for_cpu = msg->cpu;

	set_remote_irte(dev,msg->irq);

	in_dev = __in_dev_get_rtnl(dev);
	for (ifap = &in_dev->ifa_list; (ifa = *ifap) != NULL;
			ifap = &ifa->ifa_next) {
		if (!strcmp(name, ifa->ifa_label)) {
			break; /* found */
		}
	}

	if(!ifa){
		memcpy(&response.data.addr,&ifa->ifa_local,sizeof(__be32));
		memcpy(&response.data.mask,&ifa->ifa_mask,sizeof(__be32));
		memcpy(&response.data.broadcast,&ifa->ifa_broadcast,sizeof(__be32));
		response.data.addr = htonl(ifa->ifa_local);
		response.data.mask = htonl(ifa->ifa_mask);
		response.data.broadcast = htonl(ifa->ifa_broadcast);
	}

	response.count = 1;
	add = &response.data.addr;
	memcpy(response.data.perm_addr, dev->perm_addr, dev->addr_len);
	strncpy(response.data.name, name,IFNAMSIZ);
	printk(KERN_ALERT"dev name %s:\n",ifa->ifa_label);
	printk(KERN_ALERT"dev name %d . %d. %d. %d:\n",add[0],add[1],add[2],add[3]);
	printk(KERN_ALERT"dev name %s:\n",response.data.name);
	add = &ifa->ifa_local;
	printk(KERN_ALERT"local %d . %d. %d. %d:\n",add[0],add[1],add[2],add[3]);
	add = &ifa->ifa_mask;
	printk(KERN_ALERT"mask %d . %d. %d. %d:\n",add[0],add[1],add[2],add[3]);
	add = &ifa->ifa_broadcast;
	printk(KERN_ALERT"dev name %d . %d. %d. %d:\n",add[0],add[1],add[2],add[3]);
	//rt_get_iface(name);
	// Send response
	pcn_kmsg_send_long(msg->header.from_cpu,
			(struct pcn_kmsg_message*) (&response),
			sizeof(_remote_proxy_info_response_t) - sizeof(struct pcn_kmsg_hdr));


	//#endif
	sock_release(res);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

int send_proxy_request(int KernelId) {

	int res = 0;
	_remote_proxy_info_request_t * request = kmalloc(
			sizeof(_remote_proxy_info_request_t),
			GFP_KERNEL);
	// Build request
	request->header.type = PCN_KMSG_TYPE_REMOTE_PROXY_DEV_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
	unsigned int irq = 0, irq_want;
	int node = 0;
        //All secondary kernels should generate its own IRQ
	if(_cpu != 0) { 
		irq_want = NR_IRQS_LEGACY;
		irq = create_irq_nr(irq_want, node);
	}
	request->cpu = smp_processor_id();
	request->irq = irq;
	// Send response
	res = pcn_kmsg_send_long(KernelId, (struct pcn_kmsg_message*) (request),
			sizeof(_remote_proxy_info_request_t) - sizeof(struct pcn_kmsg_hdr));

	//kfree(request);
	return res;
}

/*
 * ************************************* Function (hook) to be called from other file ********************
 */
static void start_remote_probe(){
	int i = 0;

	printk(KERN_ALERT"%s",__func__);
	int result = 0;
	int retval;

	for (i = 0; i < NR_CPUS; i++) {

		// Skip the current cpu
		if (i == _cpu)
			continue;
		result = send_proxy_request(i);

		if (!result) {

			PRINTK("%s : go to sleep!!!!", __func__);
			wait_event_interruptible(wq_dev, wait_dev != -1);
		}
	}
}
static void __exit proxy_device_exit(void)
{
	printk ("Unloading my module.\n");
	return;
}


static irqreturn_t irq_handler(int irq, void *dummy, struct pt_regs * regs)
{
	printk("Caught an interrupt");
}

static int __init proxy_device_init(void)
{
	_cpu = smp_processor_id();
	int status;
	printk(KERN_ALERT"%s: Saif's modified",__func__);
	int ret =0;
	ret = pci_register_driver(&igb_p_driver);
	status = request_irq(17, irq_handler,0,"igbTXRX1", NULL);
	
	
	//#ifdef CONFIG_POPCORN_KMSG
//	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_PROXY_DEV_REQUEST,
//			handle_remote_proxy_dev_info_request);
//	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_PROXY_DEV_RESPONSE,
//			handle_remote_proxy_dev_info_response);
	//start remote probe
//	start_remote_probe();
	//#endif
	
	
	return 0;
}
/**
 * Register  init function as
 * module initialization function.
 */


module_init(proxy_device_init);
module_exit(proxy_device_exit);

MODULE_LICENSE("GPL");
