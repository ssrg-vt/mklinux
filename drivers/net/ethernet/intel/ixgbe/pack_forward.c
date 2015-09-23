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

struct net_device *snull_devs[2];
EXPORT_SYMBOL(snull_devs);

LIST_HEAD(fliter_list);



void process_rx_copy_msg(struct work_struct* work);
int handle_rx_copy(struct pcn_kmsg_message* inc_msg);
static struct sk_buff* create_skb_from_rx_copy_msg(struct rx_copy_msg *msg);


static int create_rx_skb_copy_msg(struct sk_buff *skb, struct rx_copy_msg **msg, int *msg_size){
	 struct rx_copy_msg *message;
     int headerlen;
     int head_data_len;
     int message_size;
     headerlen = skb_headroom(skb);
     head_data_len= headerlen + skb->len;
     message_size= head_data_len+ sizeof(*message);
     message= kmalloc(message_size, GFP_ATOMIC);
     if(!message)
          return -ENOMEM;
/*     message->creator= filter->creator;
     message->filter_id= filter->id;
     message->is_child= filter->type & FT_FILTER_CHILD;
     if(message->is_child){
            message->daddr= filter->tcp_param.daddr;
            message->dport= filter->tcp_param.dport;
     }
     message->pckt_id= pckt_id;
     message->local_tx= local_tx;
*/ 
     message->headerlen= headerlen;
     message->datalen= skb->len;
     message->taillen= skb_end_pointer(skb) - skb_tail_pointer(skb);

        //this should copy both header and data
     if (skb_copy_bits(skb, -headerlen, &message->data, head_data_len))
              BUG();
 
         /* Code copied from __copy_skb_header 
          *
          */
 
     message->tstamp             = skb->tstamp;
         /*new->dev                  = old->dev;*/
#ifdef NET_SKBUFF_DATA_USES_OFFSET
     message->transport_header_off   = skb->transport_header- (skb->data-skb->head);
     message->network_header_off     = skb->network_header- (skb->data-skb->head);
     message->mac_header_off         = skb->mac_header- (skb->data-skb->head);
#else
     message->transport_header_off   = skb->transport_header- (skb->data);
     message->network_header_off     = skb->network_header- (skb->data);
     message->mac_header_off         = skb->mac_header- (skb->data);

#endif
        //skb_dst_copy(new, old);
     message->rxhash             = skb->rxhash;
     message->ooo_okay           = skb->ooo_okay;
     message->l4_rxhash          = skb->l4_rxhash;
         /*#ifdef CONFIG_XFRM
        new->sp                 = secpath_get(old->sp);
		#endif*/
     memcpy(message->cb, skb->cb, sizeof(message->cb));
     message->csum               = skb->csum;
     message->local_df           = skb->local_df;
     message->pkt_type           = skb->pkt_type;
     message->ip_summed          = skb->ip_summed;
     /*skb_copy_queue_mapping(new, old);*/
     message->priority          = skb->priority;
#if defined(CONFIG_IP_VS) || defined(CONFIG_IP_VS_MODULE)
     message->ipvs_property      = skb->ipvs_property;
#endif
     message->protocol           = skb->protocol;
     message->mark               = skb->mark;
     message->skb_iif            = skb->skb_iif;
         /*__nf_copy(new, old);*/
#if defined(CONFIG_NETFILTER_XT_TARGET_TRACE) || \
    defined(CONFIG_NETFILTER_XT_TARGET_TRACE_MODULE)
     message->nf_trace           = skb->nf_trace;
#endif
#ifdef CONFIG_NET_SCHED
     message->tc_index           = skb->tc_index;
#ifdef CONFIG_NET_CLS_ACT
     message->tc_verd            = skb->tc_verd;
#endif
#endif
     message->vlan_tci           = skb->vlan_tci;
     message->secmark = skb->secmark;
     message->header.type= PCN_KMSG_TYPE_FT_RX_COPY;
     message->header.prio= PCN_KMSG_PRIO_NORMAL;
     *msg= message;          
     *msg_size= message_size;
     return 0;
  
}

void send_skb_copy(struct sk_buff *skb,int kernel)
{
	 struct rx_copy_msg* msg;
	 int msg_size;
	 int ret;
	 ret= create_rx_skb_copy_msg(skb, &msg, &msg_size);
	 if(ret)
			 return;
 	 pcn_kmsg_send_long(kernel, (struct pcn_kmsg_long_message*) msg, msg_size-sizeof(msg->header));

	 kfree(msg);
}
EXPORT_SYMBOL(send_skb_copy);

int handle_rx_copy(struct pcn_kmsg_message* inc_msg)
{
     struct rx_copy_msg *msg= (struct rx_copy_msg *) inc_msg;
     int ret= 0;
     struct sk_buff *skb;
     printk("%s: called \n",__func__);              
     skb= create_skb_from_rx_copy_msg(msg);
     if(IS_ERR(skb)){
        //put_ft_filter(filter);
         goto out;
     }
        
     local_bh_disable();     
     netif_receive_skb(skb);
     local_bh_enable();
        
        
out:
	pcn_kmsg_free_msg(msg);
       return ret;
}
EXPORT_SYMBOL(handle_rx_copy);

static struct sk_buff* create_skb_from_rx_copy_msg(struct rx_copy_msg *msg){
         struct sk_buff *skb;
 
         skb= dev_alloc_skb(msg->datalen+ msg->headerlen+ msg->taillen);
         if(!skb)
                 return ERR_PTR(-ENOMEM);
 
 
         /* Set the data pointer */
         skb_reserve(skb, msg->headerlen);
         /* Set the tail pointer and length */
         skb_put(skb, msg->datalen);     
         
         skb_copy_to_linear_data_offset(skb, -msg->headerlen, &msg->data, msg->headerlen+ msg->datalen);
 
         /* Code copied from __copy_skb_header 
         *
         */
         skb->tstamp             = msg->tstamp;
/*new->dev              = old->dev;*/
         skb_set_transport_header(skb,msg->transport_header_off);
         skb_set_network_header(skb,msg->network_header_off);
         skb_set_mac_header(skb,msg->mac_header_off);
 
/*skb_dst_copy(new, old);*/
         skb->rxhash             = msg->rxhash;
         skb->ooo_okay           = msg->ooo_okay;
         skb->l4_rxhash          = msg->l4_rxhash;
/*#ifdef CONFIG_XFRM
         new->sp                 = secpath_get(old->sp);
#endif*/
         memcpy(skb->cb, msg->cb, sizeof(skb->cb));
         skb->csum               = msg->csum;
         skb->local_df           = msg->local_df;
         skb->pkt_type           = msg->pkt_type;
         skb->ip_summed          = msg->ip_summed;
/*skb_copy_queue_mapping(new, old);*/
         skb->priority          = msg->priority;
#if defined(CONFIG_IP_VS) || defined(CONFIG_IP_VS_MODULE)
         skb->ipvs_property      = msg->ipvs_property;
#endif
         skb->protocol           = msg->protocol;
         skb->mark               = msg->mark;
         skb->skb_iif            = msg->skb_iif;
/*__nf_copy(new, old);*/
#if defined(CONFIG_NETFILTER_XT_TARGET_TRACE) || \
     defined(CONFIG_NETFILTER_XT_TARGET_TRACE_MODULE)
         skb->nf_trace           = msg->nf_trace;
#endif
#ifdef CONFIG_NET_SCHED
         skb->tc_index           = msg->tc_index;
#ifdef CONFIG_NET_CLS_ACT
         skb->tc_verd            = msg->tc_verd;
#endif
#endif
        skb->vlan_tci           = msg->vlan_tci;
        skb->secmark            = msg->secmark;   
        skb->dev				= snull_devs[0];
        
 //       fake_parameters(skb, filter);
         
        return skb;
}

void add_filter(unsigned int src_ip,unsigned int port,int kernel)
{
	printk("%s: ip %x port %x\n",__func__,src_ip,port);
	struct remote_filter * filter_entry = kmalloc(sizeof(struct remote_filter),GFP_ATOMIC);
	filter_entry->ip = src_ip;
	filter_entry->port = port;
	filter_entry->kernel = kernel;
	
	list_add_tail(&filter_entry->mylist,&fliter_list);
	
	
}

void remove_filter(unsigned int src_ip,unsigned int port)
{
}


int check_and_get_kernel(unsigned int src_ip,unsigned int port)
{
	
}

void process_rx_copy_msg(struct work_struct* work){
         struct rx_copy_work *my_work= (struct rx_copy_work *) work;
         struct rx_copy_msg *msg= (struct rx_copy_msg *) my_work->data;
         struct net_filter_info *filter= ( struct net_filter_info *) my_work->filter;
         struct sk_buff *skb;
         wait_queue_head_t* where_to_wait;
         unsigned long timeout;
 
#if FT_FILTER_VERBOSE
         char* filter_id_printed;
#endif
 
        //TODO: I should save a copy on a list
        // beacuse maybe the hot replica died before sending this pckt to other 
        // kernels => save a copy to give to them.
         skb= create_skb_from_rx_copy_msg(msg);
         if(IS_ERR(skb)){
                 //put_ft_filter(filter);
                goto out;
         }
         /* the network stack rx path is thougth to be executed in softirq
          * context...
          */
         
         local_bh_disable();     
         netif_receive_skb(skb);
         local_bh_enable();
 
out:    pcn_kmsg_free_msg(msg);
         kfree(work);
         return;
out_err:
        // spin_unlock(&filter->lock);
       //  put_ft_filter(filter);
         goto out;
}
