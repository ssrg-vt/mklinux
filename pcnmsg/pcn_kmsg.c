/*
 * pcn_kmesg.c - Kernel Module for Popcorn Messaging Layer over Socket
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/file.h>
#include <linux/pcn_kmsg.h>

#include <linux/fdtable.h>

#include <linux/net.h>
#include <net/sock.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <asm/uaccess.h>
#include <linux/socket.h>

#include <linux/delay.h>
#include <linux/time.h>

extern int _init_RemoteCPUMask(void);

pcn_kmsg_cbftn callbacks[PCN_KMSG_TYPE_MAX];
EXPORT_SYMBOL(callbacks);

send_cbftn send_callback;
EXPORT_SYMBOL(send_callback);

unsigned int my_cpu = 0;

/* Initialize callback table to null, set up control and data channels */
int __init initialize(void)
{
	printk("In messaging layer init\n");

#if defined(CONFIG_ARM64)
	my_cpu = 0;
#elif defined(CONFIG_X86_64)
	my_cpu = 1;
#else
	printk(" In msg layer: unkown architecture detected\n");
	my_cpu = 0;
#endif

	send_callback = NULL;

	printk("Popcorn Messaging Wrapper Layer Initialized\n");
	return 0;
}

late_initcall(initialize);

int pcn_kmsg_register_callback(enum pcn_kmsg_type type, pcn_kmsg_cbftn callback)
{
	if (type >= PCN_KMSG_TYPE_MAX)
		return -ENODEV; /* invalid type */

	printk("%s: registering %d \n",__func__, type);
	callbacks[type] = callback;
	return 0;
}

int pcn_kmsg_unregister_callback(enum pcn_kmsg_type type)
{
	if (type >= PCN_KMSG_TYPE_MAX)
		return -1;

	printk("Unregistering callback %d\n", type);
	callbacks[type] = NULL;
	return 0;
}

int pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg)
{
	return pcn_kmsg_send_long(dest_cpu, (struct pcn_kmsg_long_message *)msg,
				  sizeof(struct pcn_kmsg_message)-sizeof(struct pcn_kmsg_hdr));
}

int pcn_kmsg_send_long(unsigned int dest_cpu, struct pcn_kmsg_long_message *lmsg, unsigned int payload_size)
{
	int ret = 0;

	if (send_callback == NULL) {
		msleep(100);
		//printk("Waiting for call back function to be registered\n");
		return 0;
	}

	ret = send_callback(dest_cpu, (struct pcn_kmsg_message *)lmsg,
			    payload_size);

	return ret;
}

void pcn_kmsg_free_msg(void *msg){
	vfree(msg);
}

/* TODO */
inline int pcn_kmsg_get_node_ids(uint16_t *nodes, int len, uint16_t *self)
{
#if defined(CONFIG_ARM64)
	*self = 0;
#elif defined(CONFIG_X86_64)
	*self = 1;
#else
	printk(" Unkown architecture detected\n");
	*self = 0;
#endif

	return 0;
}

EXPORT_SYMBOL(pcn_kmsg_free_msg);
EXPORT_SYMBOL(pcn_kmsg_send_long);
EXPORT_SYMBOL(pcn_kmsg_send);
EXPORT_SYMBOL(pcn_kmsg_unregister_callback);
EXPORT_SYMBOL(pcn_kmsg_register_callback);
EXPORT_SYMBOL(pcn_kmsg_get_node_ids);
