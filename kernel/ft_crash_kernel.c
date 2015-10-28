/*
 * ft_crash_kernel.c
 *
 * Author: Marina
 */


#include <linux/kernel.h>
#include <linux/ft_replication.h>
#include <linux/sched.h>
#include <linux/pcn_kmsg.h>
#include <linux/pci.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/if.h>
#include <asm/uaccess.h>

struct crash_kernel_notification_msg{
        struct pcn_kmsg_hdr header;
};

static struct workqueue_struct *crash_wq;

extern int _cpu;
extern int pci_dev_list_remove(int compatible, char *vendor, char *model,
               char* slot, char *strflags, int flags);

unsigned int inet_addr(char *str)
{
    int a, b, c, d;
    char arr[4];
    sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d);
    arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d;
    return *(unsigned int *)arr;
}

static void process_crash_kernel_notification(struct work_struct *work){
	struct pci_dev *dev;
	struct pci_dev *prev;
	int found, fd, offset;
	struct pci_bus *bus;
	mm_segment_t fs;
	struct ifreq ifr;
	struct socket *sock;
	unsigned int *addr;
	
	printk("%s called\n", __func__);
	kfree(work);

	 //reenable device
        pci_dev_list_remove(0,"0x8086","0x10c9","0.0","", 0);

        bus = NULL;
        while ((bus = pci_find_next_bus(bus)) != NULL)
                         pci_rescan_bus(bus);

	//scan the buses to activate the device
        dev= NULL;
        prev= NULL;
        found= 0;
        do{
                dev= pci_get_device(0x8086, 0x10c9, prev);
                if( dev && (PCI_SLOT(dev->devfn)== 0 && (PCI_FUNC(dev->devfn)== 0)))
                        found= 1;
                prev= dev;

        }while(dev!= NULL && !found);

        if(!dev){
                printk("ERROR: %s device not found\n", __func__);
                return;
        }

        if(!dev->driver){
                printk("ERROR: %s driver not found\n", __func__);
                return;
        }

	if(flush_pending_pckt_in_filters()){
		printk("ERROR: %s impossible to flush filters\n", __func__);
                return;
	}	

	printk("filters flushed\n");
	
	if(trim_stable_buffer_in_filters()){
		printk("ERROR: %s impossible to trim filters\n", __func__);
                return;
	}	
	printk("stable buffer trimmed\n");

	if(flush_send_buffer_in_filters()){
                printk("ERROR: %s impossible to flush send buffers\n", __func__);
                return;
        }
        printk("send buffer flushed\n");
	
	//set the net device up
	//the idea is to emulate what ifconfig does
	//ifconfig eth1 up
	//ifconfig eth1 10.1.1.40

	//NOTE for now net dev name (eth1) and desired address (10.1.1.48) are hardcoded
	//TODO extract dev name from net_dev

	sock= NULL;
	fd= sock_create_kern( PF_INET, SOCK_DGRAM, IPPROTO_IP, &sock);
	if(!sock || !sock->ops ||  !sock->ops->ioctl){
		printk("ERROR: %s impossible create socket\n", __func__);
		return;
	}

	//fs needs to be changed to be able to call ioctl from kernel space
	// (it is supposed to be called througth a system_call)

        fs = get_fs();     /* save previous value */
        set_fs (get_ds()); /* use kernel limit */

	memset(&ifr,0,sizeof(ifr));
        
        memcpy(ifr.ifr_name, "eth1", sizeof("eth1"));
        ifr.ifr_addr.sa_family= (sa_family_t) AF_INET;

        ifr.ifr_flags= IFF_UP|IFF_BROADCAST|IFF_RUNNING|IFF_MULTICAST;

        sock->ops->ioctl(sock,  SIOCSIFFLAGS, (long unsigned int)&ifr);

	memset(&ifr,0,sizeof(ifr));
        
	memcpy(ifr.ifr_name, "eth1", sizeof("eth1"));
        ifr.ifr_addr.sa_family= (sa_family_t) AF_INET;
	//the first unsigned short of sa_data is supposed to be the port
	offset= sizeof(unsigned short);
	addr= (unsigned int*) (ifr.ifr_addr.sa_data+offset);
	*addr= inet_addr("10.1.1.48");

	sock->ops->ioctl(sock, SIOCSIFADDR, (long unsigned int)&ifr);	

  
	set_fs(fs); /* restore before returning to user space */	

	printk("network up\n");

	update_replica_type_after_failure();
	printk("replica type updated\n");

	fs = get_fs();     /* save previous value */
        set_fs (get_ds()); /* use kernel limit */

        memset(&ifr,0,sizeof(ifr));

        memcpy(ifr.ifr_name, DUMMY_DRIVER, sizeof(DUMMY_DRIVER));
        ifr.ifr_addr.sa_family= (sa_family_t) AF_INET;

        ifr.ifr_flags= IFF_BROADCAST|IFF_RUNNING|IFF_MULTICAST;

        sock->ops->ioctl(sock,  SIOCSIFFLAGS, (long unsigned int)&ifr);
        
        set_fs(fs); /* restore before returning to user space */

	printk("dummy_driver down\n");

	flush_syscall_info();
	printk("syscall info updated\n");

	return;
}

static int handle_crash_kernel_notification(struct pcn_kmsg_message* inc_msg){
	struct work_struct* work;
	
	work= kmalloc(sizeof(*work), GFP_ATOMIC);
	if(!work)
		return -1;

	INIT_WORK(work, process_crash_kernel_notification);
        queue_work(crash_wq, work);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

static void send_crash_kernel_msg(void){
	int i;
	struct crash_kernel_notification_msg *msg;

	msg= kmalloc(sizeof(*msg), GFP_KERNEL);
	if(!msg)
		return;
	
	msg->header.type= PCN_KMGS_TYPE_FT_CRASH_KERNEL;
	msg->header.prio= PCN_KMSG_PRIO_NORMAL;
	
#ifndef SUPPORT_FOR_CLUSTERING
        for(i = 0; i < NR_CPUS; i++) {

                if(i == _cpu) continue;
#else
        // the list does not include the current processor group descirptor (TODO)
        struct list_head *iter= NULL;
        _remote_cpu_info_list_t *objPtr= NULL;
        extern struct list_head rlist_head;
        list_for_each(iter, &rlist_head) {
                objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
                i = objPtr->_data._processor;
#endif
		
		pcn_kmsg_send(i,(struct pcn_kmsg_message*) msg);
	}

	kfree(msg);

}

static void hang_cpu(void){
	asm volatile("cli": : :"memory");
 	asm volatile("hlt": : :"memory"); 
}

asmlinkage long sys_ft_crash_kernel(void)
{
       	if(ft_is_replicated(current)){
		/*if(ft_is_primary_replica(current)){
			printk("%s called\n", __func__);
			//local_bh_disable();
			//send message to all kernel to notify them that this one is crashing
			//this should be automatically detected from other kernels using heartbeat
			send_crash_kernel_msg();
			//send_zero_window_in_filters();
			//hang the cpu (for now I am assuming the kernel is running on a single core
			hang_cpu();

			//if here something went wrong	
			printk("ERROR: %s out from hang cpu\n", __func__);						
		}*/
	}

	return 0;
}

static int __init ft_crash_kernel_init(void) {

        pcn_kmsg_register_callback(PCN_KMGS_TYPE_FT_CRASH_KERNEL, handle_crash_kernel_notification);
	crash_wq= create_singlethread_workqueue("crash_wq");

        return 0;
}

late_initcall(ft_crash_kernel_init);

