/* 
 * ft_filter.c
 *
 * Author: Marina
 */

#include <linux/ft_replication.h>
#include <linux/popcorn_namespace.h>
#include <linux/pcn_kmsg.h>
#include <linux/slab.h>
#include <asm/atomic.h>
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

#define FT_FILTER_VERBOSE 1
#if FT_FILTER_VERBOSE
#define FTPRINTK(...) printk(__VA_ARGS__)
#else
#define FTPRINTK(...) ;
#endif

struct rx_copy_msg{
	struct pcn_kmsg_hdr header;
        struct ft_pid creator;
        int filter_id;
        unsigned long long pckt_id;
	unsigned long long local_tx;

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
static int create_rx_skb_copy_msg(struct ft_pid *filter_creator, int filter_id, unsigned long long pckt_id, unsigned long long local_tx, struct sk_buff *skb, struct rx_copy_msg **msg, int *msg_size);

struct tx_notify_msg{
        struct pcn_kmsg_hdr header;
	struct ft_pid creator;
	int filter_id;
        unsigned long long pckt_id;
};
static int create_tx_notify_msg(int filter_id, unsigned long long pckt_id, struct ft_pid* creator, struct tx_notify_msg** msg, int* msg_size);

struct tcp_init_param_msg{
        struct pcn_kmsg_hdr header;
        struct ft_pid creator;
        int filter_id;
        int connect_id;
	struct tcp_init_param tcp_param;
};

struct tx_notify_work{
        struct work_struct work;
        struct net_filter_info *filter;
	unsigned long long pckt_id;
};

struct rx_copy_work{
        struct work_struct work;
        struct net_filter_info* filter;
	void* data;
};

struct tcp_param_work{
	struct delayed_work work;
        struct net_filter_info* filter;
	int connect_id;
	struct tcp_init_param tcp_param;
};

static struct workqueue_struct *tx_notify_wq;
extern int _cpu;
struct net_filter_info filter_list_head;
DEFINE_SPINLOCK(filter_list_lock);

static int get_iphdr(struct sk_buff *skb, struct iphdr** ip_header,int *iphdrlen);
static void put_iphdr(struct sk_buff *skb, int iphdrlen);


/* NOTE: filter lock must be already aquired
 */
static void add_ft_buff_entry(struct ft_sk_buff_list* list_head, struct ft_sk_buff_list* entry){
	struct ft_sk_buff_list* next= NULL;
	struct ft_sk_buff_list* prev= NULL;
	struct list_head *iter= NULL;

	list_for_each_prev(iter, &list_head->list_member) {
		prev = list_entry(iter, struct ft_sk_buff_list, list_member);
		if(prev->pckt_id < entry->pckt_id)
			goto out;
		next= prev;
	}

out:
	if(prev == next){
		list_add(&entry->list_member, &list_head->list_member);
		return;
	}

	if(prev && next){
		__list_add(&entry->list_member, &prev->list_member, &next->list_member);
		return;
	}

	
	list_add_tail(&entry->list_member, &list_head->list_member);
	return;
	
}

/* NOTE: filter lock must be already aquired
 */
static void remove_ft_buff_entry(struct ft_sk_buff_list* list_head, unsigned long long pckt_id){
        struct ft_sk_buff_list* objPtr= NULL;
	struct list_head *iter= NULL;
	struct ft_sk_buff_list* entry= NULL;

        list_for_each(iter, &list_head->list_member) {
                objPtr = list_entry(iter, struct ft_sk_buff_list, list_member);
                if(objPtr->pckt_id == pckt_id){
			entry= objPtr;
			goto out;
		}
		if(objPtr->pckt_id > pckt_id)
                        goto out;
        }

out:
	if(entry){	
		list_del(&entry->list_member);
		kfree_skb(entry->skbuff);
                kfree(entry); 
	}
}

static void add_filter(struct net_filter_info* filter){
	if(!filter)
        	return;

        spin_lock(&filter_list_lock);
        list_add(&filter->list_member,&filter_list_head.list_member);
        spin_unlock(&filter_list_lock);

}

static void remove_filter(struct net_filter_info* filter){
        if(!filter)
                return;

        spin_lock(&filter_list_lock);
        list_del(&filter->list_member);
        spin_unlock(&filter_list_lock);

}

static void release_filter(struct kref *kref){
	struct net_filter_info* filter;

	filter= container_of(kref, struct net_filter_info, kref);
	if (filter){
		remove_filter(filter);

		if(filter->ft_popcorn)
			put_ft_pop_rep(filter->ft_popcorn);
		if(filter->wait_queue)
			kfree(filter->wait_queue);
		if(filter->rx_copy_wq)
			destroy_workqueue(filter->rx_copy_wq);

		kfree(filter);
	}
}

void get_ft_filter(struct net_filter_info* filter){
	kref_get(&filter->kref);
}

/*Note: put_ft_filter may call release_filter that acquires filter_list_lock
 *Never call this function while holding that lock.
 */
void put_ft_filter(struct net_filter_info* filter){
	kref_put(&filter->kref, release_filter);
}

static struct net_filter_info* find_and_get_filter(struct ft_pid *creator, int filter_id){

	struct net_filter_info* filter= NULL;
        struct list_head *iter= NULL;
        struct net_filter_info *objPtr= NULL;

        spin_lock(&filter_list_lock);

        list_for_each(iter, &filter_list_head.list_member) {
                objPtr = list_entry(iter, struct net_filter_info, list_member);
                if( are_ft_pid_equals(&objPtr->creator, creator)
			&& objPtr->id == filter_id){

                        filter= objPtr;
                        get_ft_filter(filter);
                        goto out;
                }

        }

out: 	spin_unlock(&filter_list_lock);
	return filter;

}

/* Add struct net_filter_info filter in filter_list_head.
 * If a fake_filter is found with the same id of filter, fake_filter's counters are copied
 * on filter before adding it.
 * Fake_filter is then removed from the list.
 */
static void add_filter_coping_pending(struct net_filter_info* filter){
	struct net_filter_info* fake_filter= NULL;
        struct list_head *iter= NULL;
        struct net_filter_info *objPtr= NULL;

        spin_lock(&filter_list_lock);

        list_for_each(iter, &filter_list_head.list_member) {
                objPtr = list_entry(iter, struct net_filter_info, list_member);
                if( are_ft_pid_equals(&objPtr->creator, &filter->creator)
                        && objPtr->id == filter->id){

                        fake_filter= objPtr;
			list_del(&filter->list_member);
                        goto next;
                }

        }

next:	if(fake_filter){
		spin_lock(&fake_filter->lock);
		
		filter->local_tx= fake_filter->local_tx;
		filter->hot_tx= fake_filter->hot_tx;
		filter->local_rx= fake_filter->local_rx;
		filter->hot_rx= fake_filter->hot_rx;
	
		filter->hot_connect_id= fake_filter->hot_connect_id;
        	filter->local_connect_id= fake_filter->local_connect_id;
		filter->tcp_param= fake_filter->tcp_param;
		
		kfree(filter->wait_queue);	
		filter->wait_queue= fake_filter->wait_queue;	
		fake_filter->wait_queue= NULL;

		destroy_workqueue(filter->rx_copy_wq);
		filter->rx_copy_wq= fake_filter->rx_copy_wq;
		fake_filter->rx_copy_wq= NULL;

		fake_filter->type &= ~FT_FILTER_ENABLE;

		spin_unlock(&fake_filter->lock);
	
		wake_up(filter->wait_queue);
	}

	list_add(&filter->list_member,&filter_list_head.list_member);
	spin_unlock(&filter_list_lock);
        
	if(fake_filter)
		put_ft_filter(fake_filter);
	
	return ;

}

/* Adds a struct net_filter_info filter in filter_list_head.
 * If a real_filter is found with the same id of filter, filter is not 
 * inserted in the list and its reference is dropped. 
 */
static void add_filter_with_check(struct net_filter_info* filter){
        struct net_filter_info* real_filter= NULL;
        struct list_head *iter= NULL;
        struct net_filter_info *objPtr= NULL;

        spin_lock(&filter_list_lock);

        list_for_each(iter, &filter_list_head.list_member) {
                objPtr = list_entry(iter, struct net_filter_info, list_member);
                if( are_ft_pid_equals(&objPtr->creator, &filter->creator)
                        && objPtr->id == filter->id){

                        real_filter= objPtr;
                        goto next;
                }

        }

next:   if(!real_filter){
		list_add(&filter->list_member,&filter_list_head.list_member);
	}

	spin_unlock(&filter_list_lock);

	if(real_filter)
		put_ft_filter(filter);

        return ;

}

/* Prints the filter identificative on string.
 *
 * Remember to kfree the returned string eventually.
 */
char* print_filter_id(struct net_filter_info *filter){
        char *string;
	char *creator_printed;
        const int size= 1024*2;
	int ret;

        if(!filter)
                return NULL;

	creator_printed= print_ft_pid(&filter->creator);
	if(!creator_printed)
		return NULL;

        string= kmalloc(size,GFP_ATOMIC);
        if(!string)
                return NULL;
	
        ret= snprintf(string, size, "{ creator: %s, id %d}", creator_printed, filter->id);
	if (ret>= size)
		goto out_clean;

        kfree(creator_printed);

	return string;

out_clean:
	kfree(creator_printed);
	kfree(string);
	printk("%s: buff size too small\n", __func__);
        return NULL;
}

/* Creates a struct net_filter_info* fake_filter and adds it in filter_list_head
 * if a real one does not already exists.
 * 
 * A fake filter is used as "temporary" struct net_filter_info to store hot replica's
 * notifications while the cold one reaches the create_filter call.
 */
static int create_fake_filter(struct ft_pid *creator, int filter_id){
	struct net_filter_info* filter;
	char* filter_id_printed;
	
	filter= kmalloc(sizeof(*filter),GFP_ATOMIC);
	if(!filter)
		return -ENOMEM;

	filter->wait_queue= kmalloc(sizeof(*filter->wait_queue),GFP_ATOMIC);
        if(!filter->wait_queue){
        	kfree(filter); 
	       	return -ENOMEM;
	}

	INIT_LIST_HEAD(&filter->list_member);
	atomic_set(&filter->kref.refcount,1);
	filter->creator= *creator;
	filter->ft_popcorn= NULL;
	filter->ft_socket= NULL;
	filter->ft_sock= NULL;
	spin_lock_init(&filter->lock);
	filter_id_printed= print_filter_id(filter);
        filter->rx_copy_wq= create_singlethread_workqueue(filter_id_printed);
	init_waitqueue_head(filter->wait_queue);
	INIT_LIST_HEAD(&filter->skbuff_list.list_member);
	
	filter->type= FT_FILTER_ENABLE; 
	filter->type|= FT_FILTER_FAKE;

	filter->id= filter_id;

        filter->local_tx= 0;
        filter->hot_tx= 0;
        filter->local_rx= 0;
        filter->hot_rx= 0;

	filter->hot_connect_id= 0;
	filter->local_connect_id= 0;

	add_filter_with_check(filter);

	return 0;
}

/* Creates a filter for newsock and checks the compatibility between sock and newsock
 * filters.
 * Returns 0 in case of success.
 */
int create_filter_accept(struct task_struct *task, struct socket *newsock,struct socket *sock){
	int ret= 0;
	
	if(!newsock->filter){
		ret= create_filter(task, newsock);
		if(ret)
			goto out;
	}

	if(sock->filter_type & FT_FILTER_ENABLE){
		if(!(newsock->filter_type & FT_FILTER_ENABLE)){
			printk("%s: ERROR accepting from a ft-socket but not in a ft-application\n", __func__);
			ret= -EFAULT;
			goto out_clean;
		}

		if( (newsock->filter_type & FT_FILTER_COLD_REPLICA) && ! (sock->filter_type & FT_FILTER_COLD_REPLICA) ){
			printk("%s: ERROR new sock filter is cold but sock filter is not\n", __func__);
			ret= -EFAULT;
                        goto out_clean;
		}

		if( (newsock->filter_type & FT_FILTER_HOT_REPLICA) && ! (sock->filter_type & FT_FILTER_HOT_REPLICA) ){
                        printk("%s: ERROR new sock filter is hot but sock filter is not\n", __func__);
                        ret= -EFAULT;
                        goto out_clean;
                }

	}

	return ret;

out_clean: 
	put_ft_filter(newsock->filter);
	newsock->filter= NULL;
	newsock->filter_type= FT_FILTER_DISABLE;
out:	return ret;
}

/* Create a struct net_filter_info* real_filter and add it in filter_list_head.
 * The filter created will be associated with the struct ft_pop_rep ft_popcorn of task.
 * 
 * Returns 0 in case of success.
 */
int create_filter(struct task_struct *task, struct socket *sock){
	struct net_filter_info* filter;
	struct task_struct *ancestor;
        char* filter_id_printed;

	if(task->replica_type == HOT_REPLICA 
		|| task->replica_type == COLD_REPLICA
		|| task->replica_type == NEW_HOT_REPLICA_DESCENDANT
		|| task->replica_type == NEW_COLD_REPLICA_DESCENDANT
		|| task->replica_type == REPLICA_DESCENDANT){
		
		filter= kmalloc(sizeof(*filter),GFP_ATOMIC);
		if(!filter)
			return -ENOMEM;
		
		filter->wait_queue= kmalloc(sizeof(*filter->wait_queue),GFP_ATOMIC);
	        if(!filter->wait_queue){
        	        kfree(filter);
                	return -ENOMEM;
        	}

		INIT_LIST_HEAD(&filter->list_member);
		atomic_set(&filter->kref.refcount, 1);
		filter->creator= task->ft_pid;
		get_ft_pop_rep(task->ft_popcorn);
		filter->ft_popcorn= task->ft_popcorn;
		filter->ft_socket= sock;
		filter->ft_sock= sock->sk;
		spin_lock_init(&filter->lock);
		init_waitqueue_head(filter->wait_queue);
		INIT_LIST_HEAD(&filter->skbuff_list.list_member);

		/* NOTE: target applications are deterministic, so all replicas will do the same actions 
                 * on the same order.
                 * Because of this, all replicas will and up creating this socket, and giving it the same id.
                 */

		filter->id= task->next_id_resources++;

		filter_id_printed= print_filter_id(filter);
                filter->rx_copy_wq= create_singlethread_workqueue(filter_id_printed);

                filter->local_tx= 0;
                filter->hot_tx= 0;
                filter->local_rx= 0;
                filter->hot_rx= 0;

		filter->hot_connect_id= 0;
	        filter->local_connect_id= 0;
			
		ancestor= find_task_by_vpid(task->tgid);

		if(ancestor->replica_type == HOT_REPLICA || ancestor->replica_type == NEW_HOT_REPLICA_DESCENDANT){
			filter->type= FT_FILTER_ENABLE;
			filter->type |= FT_FILTER_HOT_REPLICA;
			add_filter(filter);
		}
		else{
			if(ancestor->replica_type == COLD_REPLICA || ancestor->replica_type == NEW_COLD_REPLICA_DESCENDANT){
                        	filter->type= FT_FILTER_ENABLE; 
                        	filter->type |= FT_FILTER_COLD_REPLICA;
				/*maybe the hot replica alredy sent me some notifications or msg*/
				add_filter_coping_pending(filter);
			}
			else{
				//BUG();
				printk("%s: ERROR ancestor pid %d has replica type %d \n",__func__, ancestor->pid, ancestor->replica_type);
				put_ft_pop_rep(filter->ft_popcorn);
				kfree(filter->wait_queue);
				kfree(filter);
				return -EFAULT;
			}
			
		}		

		sock->filter_type= FT_FILTER_ENABLE;
		sock->filter= filter;

#if FT_FILTER_VERBOSE
        	FTPRINTK("%s: pid %d created new filter %s\n\n", __func__, current->pid, filter_id_printed);
	    	if(filter_id_printed)
                	kfree(filter_id_printed);
#endif
		
	}else{
		sock->filter_type= FT_FILTER_DISABLE;
		sock->filter= NULL;
	}	

	return 0;	
}

/* Stores hot_replica notifications on the proper struct net_filter_info filter.
 * 
 * If a filter is not found, a fake one is temporarily added in the list for storing
 * incoming notifications.
 */
static int handle_tx_notify(struct pcn_kmsg_message* inc_msg){
	struct tx_notify_msg *msg= (struct tx_notify_msg *) inc_msg;
	struct net_filter_info *filter;
	int err;
	int removing_fake= 0;
#if FT_FILTER_VERBOSE
        char* ft_pid_printed;
#endif


again:	filter= find_and_get_filter(&msg->creator, msg->filter_id);
	if(filter){
		spin_lock(&filter->lock);
		if(filter->type & FT_FILTER_ENABLE){
			
			if(filter->hot_tx < msg->pckt_id)
				filter->hot_tx= msg->pckt_id;

			wake_up(filter->wait_queue);		
			remove_ft_buff_entry(&filter->skbuff_list, msg->pckt_id);
		}
		else{
			removing_fake= 1;
		}
        	spin_unlock(&filter->lock);
		
		put_ft_filter(filter);

		if(removing_fake){
			removing_fake= 0;
			goto again;
		}
	}
	else{
#if FT_FILTER_VERBOSE
        	ft_pid_printed= print_ft_pid(&msg->creator);
        	FTPRINTK("%s: creating fake filter for ft_pid %s id %d\n\n", __func__, ft_pid_printed, msg->filter_id);
        	if(ft_pid_printed)
                	kfree(ft_pid_printed);
#endif

		err= create_fake_filter(&msg->creator, msg->filter_id);
		if(!err)
			goto again;
	}
	
	pcn_kmsg_free_msg(msg);
	
	return 0;
}

/* Creates a struct tx_notify_msg message.
 * In case of success 0 is returned and msg and msg_size are properly populated.
 *
 * Remember to kfree the message eventually.
 */
static int create_tx_notify_msg(int filter_id, unsigned long long pckt_id, struct ft_pid* creator, struct tx_notify_msg** msg, int* msg_size){
	struct tx_notify_msg* message;
	
	message= kmalloc(sizeof(*message), GFP_ATOMIC);
	if(!message)
		return -ENOMEM;

	message->creator= *creator;
	message->filter_id= filter_id;
	message->pckt_id= pckt_id;

	message->header.type= PCN_KMSG_TYPE_FT_TX_NOTIFY;
	message->header.prio= PCN_KMSG_PRIO_NORMAL;

	*msg_size= sizeof(*message);
	*msg= message;

	return 0;
}


static void send_tx_notification(struct work_struct* work){
	struct tx_notify_work *tx_n_work= (struct tx_notify_work*) work;
	struct net_filter_info *filter= tx_n_work->filter;
	struct tx_notify_msg* msg;
	int msg_size;
	int ret;
	struct list_head *iter= NULL;
	struct replica_id cold_replica;
	struct replica_id_list* objPtr;

	ret= create_tx_notify_msg(filter->id, tx_n_work->pckt_id, &filter->creator, &msg, &msg_size);
	if(ret)
		goto out;

        list_for_each(iter, &filter->ft_popcorn->cold_replicas_head.replica_list_member) {
                objPtr = list_entry(iter, struct replica_id_list, replica_list_member);
                cold_replica= objPtr->replica;

		pcn_kmsg_send_long(cold_replica.kernel, (struct pcn_kmsg_long_message*) msg, msg_size-sizeof(msg->header));
	
        }
	
	kfree(msg);

out:	kfree(work);	
	put_ft_filter(filter);
}

/* Notifies all cold replicas that a new packet is beeing transmitted
 * on this socket.
 *
 * Note: messages are sent from a working queue.
 *
 * In case of error a value < 0 is returned, FT_TX_OK
 * is returned.
 */
static int tx_filter_hot(struct net_filter_info *filter){
        unsigned long long pckt_id;
	int ret= FT_TX_OK;
	struct tx_notify_work *work;
#if FT_FILTER_VERBOSE
        char* ft_pid_printed;
        char* filter_id_printed;
#endif

        spin_lock(&filter->lock);

        pckt_id= ++filter->local_tx;

        spin_unlock(&filter->lock);

#if FT_FILTER_VERBOSE
        ft_pid_printed= print_ft_pid(&current->ft_pid);
        filter_id_printed= print_filter_id(filter);
        FTPRINTK("%s: pid %d ft_pid %s reached send of packet %llu in filter %s\n\n", __func__, current->pid, ft_pid_printed, pckt_id, filter_id_printed);
        if(ft_pid_printed)
                kfree(ft_pid_printed);
        if(filter_id_printed)
                kfree(filter_id_printed);
#endif

	work= kmalloc(sizeof(*work), GFP_ATOMIC);
	if(!work){
        	ret= -ENOMEM;
                goto out;
        }

	get_ft_filter(filter);

        INIT_WORK( (struct work_struct*)work, send_tx_notification);
        work->filter= filter;
	work->pckt_id= pckt_id;

        queue_work(tx_notify_wq, (struct work_struct*)work);

out:        
	return ret;

}

static int is_pckt_to_filter(struct sk_buff *skb){
	struct iphdr *network_header;

	if(skb->protocol == cpu_to_be16(ETH_P_IP)){
		network_header= (struct iphdr *)skb_network_header(skb);

		if(network_header->protocol == IPPROTO_UDP
			|| network_header->protocol == IPPROTO_TCP){
			return 1;
		}

	}
	
	return 0;
}

/* Waits that the hot replica transmits the same packet.
 *
 * In case of error a value < 0 is returned, FT_TX_DROP
 * is returned.
 */
static int tx_filter_cold(struct net_filter_info *filter, struct sk_buff *skb){
	unsigned long long pckt_id;
	struct ft_sk_buff_list* buff_entry;
	unsigned long timeout = msecs_to_jiffies(WAIT_PCKT_MAX) + 1;
#if FT_FILTER_VERBOSE
	char* ft_pid_printed;
	char* filter_id_printed;
#endif

	spin_lock(&filter->lock);
	
	pckt_id= ++filter->local_tx;
	
	spin_unlock(&filter->lock);

	wake_up(filter->wait_queue);

#if FT_FILTER_VERBOSE
	ft_pid_printed= print_ft_pid(&current->ft_pid);
	filter_id_printed= print_filter_id(filter);
	FTPRINTK("%s: pid %d ft_pid %s waiting for packet %llu in filter %s\n\n", __func__, current->pid, ft_pid_printed, pckt_id, filter_id_printed);
	if(ft_pid_printed)
		kfree(ft_pid_printed);
	if(filter_id_printed)
		kfree(filter_id_printed);
#endif

	if ( !in_atomic_preempt_off() && filter->hot_tx < pckt_id){
		
		wait_event_timeout(*filter->wait_queue, filter->hot_tx >= pckt_id, timeout);

	}

	if(filter->hot_tx < pckt_id){
		buff_entry= kmalloc(sizeof(*buff_entry),GFP_ATOMIC);
		if(!buff_entry){
			return -ENOMEM;
		}
		skb_get(skb);
		buff_entry->skbuff= skb;
		buff_entry->pckt_id= pckt_id;

		spin_lock(&filter->lock);
		if(filter->hot_tx < pckt_id){
			add_ft_buff_entry(&filter->skbuff_list, buff_entry);
			FTPRINTK("%s: pid %d saved packet %llu \n\n", __func__, current->pid, pckt_id);
		}
		else{
			kfree_skb(skb);
			kfree(buff_entry);
		}
		spin_unlock(&filter->lock);
	}
	
	FTPRINTK("%s: pid %d is going to drop the packet %llu\n\n", __func__, current->pid, pckt_id);

	return FT_TX_DROP;

}

/* Packet filter for ft-popcorn, activated only if socket is associated
 * with a replicated thread.
 *
 * In case the socket was created by an hot replica associated thread, 
 * a notification message is sent to all cold replicas.
 *
 * In case the socket was created by a cold replica associated thread,
 * the execution is stopped while the corresponding packet notification
 * is received from the hot replica.
 * 
 * In case of error a value < 0 is returned, otherwise FT_TX_OK or FT_TX_DROP
 * is returned.
 */
int net_ft_tx_filter(struct sock* sk, struct sk_buff *skb){
	int ret= FT_TX_OK;
	struct net_filter_info *filter;
	struct socket* socket;

	if(!is_pckt_to_filter(skb))
		goto out;		
	
	if(!sk){
		printk("%s: WARNING sock is null for pid %d\n",__func__, current->pid);
                goto out;
	}

	socket= sk->sk_socket;
	if(!socket){
		//BUG();
		printk("%s: WARNING socket is null for pid %d\n",__func__, current->pid);
		goto out;
	}

	if(!(socket->filter_type & FT_FILTER_ENABLE)){
		goto out;
	}

	filter= socket->filter;

	if(!filter){
        	//BUG();
                printk("%s: ERROR filter is null\n",__func__);
                goto out;
	}


	if(filter->type & FT_FILTER_COLD_REPLICA){
		return tx_filter_cold(filter, skb);
	}

	if(filter->type & FT_FILTER_HOT_REPLICA){
		return tx_filter_hot(filter);
	}	

out:	return ret;
}

static void fake_parameters(struct sk_buff *skb, struct net_filter_info *filter){
	struct inet_sock *inet;
	int res, iphdrlen, datalen, msg_changed;
        struct iphdr *network_header; 
	struct tcphdr *tcp_header= NULL;     // tcp header struct
        struct udphdr *udp_header= NULL;     // udp header struct

	/* I need to fake the receive of this skbuff from a device.
         * I use dummy net driver for that.
         */
        skb->dev= dev_get_by_name(&init_net, DUMMY_DRIVER);

	/* The local IP or port may be different, 
	 * hack the message with the correct ones.
	 */
	inet = inet_sk(filter->ft_sock);
	if(!inet){
		printk("%s, ERROR impossible to retrive inet socket\n",__func__);
		return;
	}

	res= get_iphdr(skb, &network_header, &iphdrlen);
	if(res){
		return;
	}

	msg_changed= 0;

	/* inet_saddr is the local IP
	 * because for the socket is the rcv end of the stream?!
	 */
	if(network_header->daddr != inet->inet_saddr){
		network_header->daddr= inet->inet_saddr;
		msg_changed= 1;
	}

	if (network_header->protocol == IPPROTO_UDP){
		udp_header= (struct udphdr *) ((char*)network_header+ network_header->ihl*4);
		datalen= skb->len - ip_hdrlen(skb);
		// udp_header->source
		if(udp_header->dest != inet->inet_sport){
			udp_header->dest= inet->inet_sport;
			msg_changed= 1 ;
		} 
		//inet_iif(skb)
		
		if(msg_changed){
	                 udp_header->check = csum_tcpudp_magic(network_header->saddr, network_header->daddr,
 	                                   datalen, network_header->protocol,
 	                                   csum_partial((char *)udp_header, datalen, 0));
 	                 ip_send_check(network_header);
 	         }

	}
	else{
		if (skb->pkt_type != PACKET_HOST)
			goto out_put;

		if (!pskb_may_pull(skb, sizeof(struct tcphdr)))
			goto out_put;

		tcp_header= tcp_hdr(skb);

		if (tcp_header->doff < sizeof(struct tcphdr) / 4)
			goto out_put;

		if (!pskb_may_pull(skb, tcp_header->doff * 4))
			goto out_put;

		//tcp_header->source
		if(tcp_header->dest != inet->inet_sport){     
                        tcp_header->dest= inet->inet_sport; 
                        msg_changed= 1 ;
                }

		if(msg_changed){
			 tcp_v4_send_check(filter->ft_sock, skb);
                         /*tcp_header->check = csum_tcpudp_magic(network_header->saddr, network_header->daddr,
                                           datalen, network_header->protocol,
                                           csum_partial((char *)tcp_header, datalen, 0));*/
                         ip_send_check(network_header);
                 }

		//inet_iif(skb)
	}

out_put:	put_iphdr(skb, iphdrlen);
		
}

static struct sk_buff* create_skb_from_rx_copy_msg(struct rx_copy_msg *msg, struct net_filter_info *filter){
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
        skb->tstamp		= msg->tstamp;
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
        skb->secmark 		= msg->secmark;
	
	fake_parameters(skb, filter);
	
	return skb;
}

static void process_rx_copy_msg(struct work_struct* work){
        struct rx_copy_work *my_work= (struct rx_copy_work *) work;
        struct rx_copy_msg *msg= (struct rx_copy_msg *) my_work->data;
	struct net_filter_info *filter= ( struct net_filter_info *) my_work->filter;
	struct sk_buff *skb;
	wait_queue_head_t* where_to_wait;

again:	spin_lock(&filter->lock);
	if(filter->type & FT_FILTER_ENABLE){

	        //TODO: I should save a copy on a list
		// beacuse maybe the hot replica died before sending this pckt to other 
		// kernels => save a copy to give to them.
		if(msg->pckt_id != filter->hot_rx+1){
			printk("%s: ERROR out of order delivery\n", __func__);
			goto out_err;
		}
		filter->hot_rx= msg->pckt_id;
	
		FTPRINTK("%s: pid %d is going to wait for delivering packet %llu\n\n", __func__, current->pid, msg->pckt_id);

		/* Wait to be aligned with the hot replica for the delivery of the packet.
		 * => wait to reach the same number of sent pckts.
		 */
		while( (filter->type & FT_FILTER_FAKE) || (filter->local_tx < msg->local_tx)){
			where_to_wait= filter->wait_queue;
			spin_unlock(&filter->lock);

			wait_event(*where_to_wait, !(filter->type & FT_FILTER_ENABLE) || ( !(filter->type & FT_FILTER_FAKE) && (filter->local_tx >= msg->local_tx)));
			//wait_event(*where_to_wait, !((filter->type & FT_FILTER_ENABLE) && (filter->local_tx < msg->local_tx)));

			spin_lock(&filter->lock);
            
			if ( !(filter->type & FT_FILTER_FAKE) && (filter->local_tx >= msg->local_tx) )
				goto done;

			if(!(filter->type & FT_FILTER_ENABLE)){
				if(!(filter->type & FT_FILTER_FAKE)){
		                        printk("%s: ERROR filter is disable but not fake\n",__func__);
                		        goto out_err;
                		}

                		spin_unlock(&filter->lock);
                		put_ft_filter(filter);

                		filter= find_and_get_filter(&msg->creator, msg->filter_id);
                		if(!filter){
                        		printk("%s: ERROR no filter\n",__func__);
                        		goto out;
                		}

				spin_lock(&filter->lock);

			}
		}
	}
	else{

		if(!(filter->type & FT_FILTER_FAKE)){
			printk("%s: ERROR filter is disable but not fake\n",__func__);
			goto out_err;
		}

		spin_unlock(&filter->lock);
		put_ft_filter(filter);

		filter= find_and_get_filter(&msg->creator, msg->filter_id);
		if(!filter){
			printk("%s: ERROR no filter\n",__func__);
			goto out;
		}
		else
			goto again;
	}
done:	spin_unlock(&filter->lock);

	if(filter->type & FT_FILTER_FAKE){
		printk("%s: ERROR trying to delivery pckt to fake filter\n", __func__);
		put_ft_filter(filter);
		goto out;
	}

	skb= create_skb_from_rx_copy_msg(msg, filter);
        if(IS_ERR(skb)){
                put_ft_filter(filter);
                goto out;
        }

	FTPRINTK("%s: pid %d is going to deliver the packet %llu\n\n", __func__, current->pid, msg->pckt_id);

	netif_receive_skb(skb);
	
	put_ft_filter(filter);

out:	pcn_kmsg_free_msg(msg);
	kfree(work);
	return;
out_err:
	spin_unlock(&filter->lock);
        put_ft_filter(filter);
	goto out;
}

static int handle_rx_copy(struct pcn_kmsg_message* inc_msg){
	struct rx_copy_msg *msg= (struct rx_copy_msg *) inc_msg;
	struct rx_copy_work *work;
	int ret= 0;
	struct net_filter_info* filter;
	int removing_fake= 0;
#if FT_FILTER_VERBOSE
        char* ft_pid_printed;
#endif

again:  filter= find_and_get_filter(&msg->creator, msg->filter_id);
        if(filter){
                spin_lock(&filter->lock);
                if(filter->type & FT_FILTER_ENABLE){

			work= kmalloc(sizeof(*work), GFP_ATOMIC);
		        if(!work){
                		ret= -ENOMEM;
                		goto out_err;
        		}

        		INIT_WORK( (struct work_struct*)work, process_rx_copy_msg);
        		work->data= inc_msg;
			work->filter= filter;
        		queue_work(filter->rx_copy_wq, (struct work_struct*)work);

                }
                else{
			if(!(filter->type & FT_FILTER_FAKE)){
	                        printk("%s: ERROR filter is disable but not fake\n",__func__);
				ret= -EFAULT;
				goto out_err;
			}

                        removing_fake= 1;
                }
                spin_unlock(&filter->lock);

                if(removing_fake){
                        put_ft_filter(filter);
			removing_fake= 0;
                        goto again;
                }
        }
        else{
#if FT_FILTER_VERBOSE
                ft_pid_printed= print_ft_pid(&msg->creator);
                FTPRINTK("%s: creating fake filter for ft_pid %s id %d\n\n", __func__, ft_pid_printed, msg->filter_id);
                if(ft_pid_printed)
                        kfree(ft_pid_printed);
#endif

                ret= create_fake_filter(&msg->creator, msg->filter_id);
                if(!ret)
                        goto again;
        }

out:
	return ret;
out_err:
	spin_unlock(&filter->lock);
	put_ft_filter(filter);
	pcn_kmsg_free_msg(msg);
	goto out;
}

/*
 * For coping skb check net/core/skb.c 
 */
static int create_rx_skb_copy_msg(struct ft_pid *filter_creator, int filter_id, unsigned long long pckt_id, unsigned long long local_tx, struct sk_buff *skb, struct rx_copy_msg **msg, int *msg_size){
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

	message->creator= *filter_creator;
	message->filter_id= filter_id;
	message->pckt_id= pckt_id;
	message->local_tx= local_tx;

	message->headerlen= headerlen;
	message->datalen= skb->len;
	message->taillen= skb_end_pointer(skb) - skb_tail_pointer(skb);
	
	//this should copy both header and data
	if (skb_copy_bits(skb, -headerlen, &message->data, head_data_len))
               BUG();

	/* Code copied from __copy_skb_header 
	 *
	 */

	message->tstamp		    = skb->tstamp;
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

static void send_skb_copy(struct net_filter_info *filter, unsigned long long pckt_id, unsigned long long local_tx, struct sk_buff *skb){
        struct rx_copy_msg* msg;
        int msg_size;
        int ret;
        struct list_head *iter= NULL;
        struct replica_id cold_replica;
        struct replica_id_list* objPtr;

        ret= create_rx_skb_copy_msg(&filter->creator,filter->id, pckt_id, local_tx, skb, &msg, &msg_size);
        if(ret)
                return;

        list_for_each(iter, &filter->ft_popcorn->cold_replicas_head.replica_list_member) {
                objPtr = list_entry(iter, struct replica_id_list, replica_list_member);
                cold_replica= objPtr->replica;

                pcn_kmsg_send_long(cold_replica.kernel, (struct pcn_kmsg_long_message*) msg, msg_size-sizeof(msg->header));

        }

        kfree(msg);
}

static int rx_filter_hot(struct net_filter_info *filter, struct sk_buff *skb){
	unsigned long long pckt_id;
	unsigned long long local_tx;
#if FT_FILTER_VERBOSE
	char* ft_pid_printed;
	char* filter_id_printed;
#endif

        spin_lock(&filter->lock);
 	pckt_id= ++filter->local_rx;
	local_tx= filter->local_tx;

#if FT_FILTER_VERBOSE
        ft_pid_printed= print_ft_pid(&current->ft_pid);
        filter_id_printed= print_filter_id(filter);
        FTPRINTK("%s: pid %d ft_pid %s broadcasting packet %llu in filter %s\n\n", __func__, current->pid, ft_pid_printed, pckt_id, filter_id_printed);
        if(ft_pid_printed)
                kfree(ft_pid_printed);
        if(filter_id_printed)
                kfree(filter_id_printed);
#endif

	send_skb_copy(filter, pckt_id, local_tx, skb);

	/* Do not know if it is correct to send msgs while holding this lock,
	 * but this should prevent deliver out of order of pckts to cold replicas
	 * if the working queues rx_copy_wq are single thread.
	 * (assuming that msg layer is FIFO)
 	 */
	spin_unlock(&filter->lock);

        return 0;
}

static int rx_filter_cold(struct net_filter_info *filter){
	unsigned long long pckt_id;
	unsigned long long hot_rx;
	char* filter_id_printed;
#if FT_FILTER_VERBOSE
        char* ft_pid_printed;
#endif
        spin_lock(&filter->lock);
        pckt_id= ++filter->local_rx;	
	hot_rx= filter->hot_rx;
	spin_unlock(&filter->lock);

#if FT_FILTER_VERBOSE
        ft_pid_printed= print_ft_pid(&current->ft_pid);
        filter_id_printed= print_filter_id(filter);
        FTPRINTK("%s: pid %d ft_pid %s received pckt %llu in filter %s\n\n", __func__, current->pid, ft_pid_printed, pckt_id, filter_id_printed);
        if(ft_pid_printed)
                kfree(ft_pid_printed);
        if(filter_id_printed){
                kfree(filter_id_printed);
		filter_id_printed= NULL;
	}
#endif

	if(pckt_id > hot_rx){
		filter_id_printed= print_filter_id(filter);
		printk("%s: ERROR pckt id is %llu hot rx is %llu in filter %s\n", __func__, pckt_id, hot_rx, filter_id_printed);
		if(filter_id_printed)
                	kfree(filter_id_printed);
	}
	return 0;
}

static void update_init_parameters_from_work(struct work_struct* work){
	struct tcp_param_work* my_work= (struct tcp_param_work*) work;
	struct net_filter_info *filter= my_work->filter;
	struct ft_pid creator;
	int filter_id;

again:	spin_lock(&filter->lock);
	if(filter->type & FT_FILTER_ENABLE){
		if(filter->local_connect_id == my_work->connect_id-1
                                && filter->hot_connect_id == my_work->connect_id-1){
                                filter->tcp_param= my_work->tcp_param;
                                wake_up(filter->wait_queue);
                }
                else{
			INIT_DELAYED_WORK( (struct delayed_work*)work, update_init_parameters_from_work);
                        queue_delayed_work(tx_notify_wq, (struct delayed_work*) work, 10);
			spin_unlock(&filter->lock);
			return;
		}
	}
	else{
		spin_unlock(&filter->lock);
		if(!(filter->type & FT_FILTER_FAKE)){
			printk("%s: ERROR filter is disabled but not fake\n", __func__);
			goto out;
		}
		creator= filter->creator;
		filter_id= filter->id;

		put_ft_filter(filter);
		filter= find_and_get_filter(&creator, filter_id);
		if(!filter || !(filter->type & FT_FILTER_ENABLE)){
                        printk("%s: ERROR filter not available\n", __func__);
        		if(filter)
				put_ft_filter(filter);

	                goto out;
                }

		goto again;

	}
	spin_unlock(&filter->lock);
	
	put_ft_filter(filter);

out:	
	kfree(work);
}

static int handle_tcp_init_param(struct pcn_kmsg_message* inc_msg){
	struct tcp_init_param_msg* msg= (struct tcp_init_param_msg*) inc_msg;
	struct net_filter_info *filter;
        int err= 0;
        int removing_fake= 0;
#if FT_FILTER_VERBOSE
        char* ft_pid_printed;
#endif
	struct tcp_param_work* work;

again:  filter= find_and_get_filter(&msg->creator, msg->filter_id);
        if(filter){
                spin_lock(&filter->lock);
                if(filter->type & FT_FILTER_ENABLE){
			if(filter->local_connect_id == msg->connect_id-1
				&& filter->hot_connect_id == msg->connect_id-1){
				filter->tcp_param= msg->tcp_param;
				filter->hot_connect_id++;
				wake_up(filter->wait_queue);
			}
			else{
				work= kmalloc(sizeof(*work), GFP_KERNEL);
        			if(!work){
                			err= -ENOMEM;
					spin_unlock(&filter->lock);
			                put_ft_filter(filter);
					goto out;
				}
        			INIT_WORK( (struct work_struct*)work, update_init_parameters_from_work);
        			work->filter= filter;
        			work->connect_id= msg->connect_id;
				work->tcp_param= msg->tcp_param;
				
				/* NOTE, I cannot use the rx_copy_msg
				 * because if the handler of rx_copy queues a work before this,
				 * everything will hang.
				 */
				queue_work(tx_notify_wq, (struct work_struct*)work);

			}
                }
                else{
                        removing_fake= 1;
                }
                spin_unlock(&filter->lock);

                put_ft_filter(filter);

                if(removing_fake){
                        removing_fake= 0;
                        goto again;
                }
        }
        else{
#if FT_FILTER_VERBOSE
                ft_pid_printed= print_ft_pid(&msg->creator);
                FTPRINTK("%s: creating fake filter for ft_pid %s id %d\n\n", __func__, ft_pid_printed, msg->filter_id);
                if(ft_pid_printed)
                        kfree(ft_pid_printed);
#endif

                err= create_fake_filter(&msg->creator, msg->filter_id);
                if(!err)
                        goto again;
        }

out:
        pcn_kmsg_free_msg(msg);
	return err;
}

static int create_tcp_init_param_msg(struct net_filter_info* filter, int connect_id, struct tcp_init_param* tcp_param, struct tcp_init_param_msg** msg, int* msg_leng ){
	struct tcp_init_param_msg* message;

	message= kmalloc(sizeof(*message), GFP_KERNEL);
	if(!message)
		return -ENOMEM;

	message->header.type= PCN_KMSG_TYPE_FT_TCP_INIT_PARAM;
        message->header.prio= PCN_KMSG_PRIO_NORMAL;

        message->creator= filter->creator;
        message->filter_id= filter->id;
        message->connect_id= connect_id;
	message->tcp_param= *tcp_param;

	*msg_leng= sizeof(*message);
	*msg= message;

	return 0; 
}

static void send_tcp_init_parameters_from_work(struct work_struct* work){
	struct tcp_param_work* my_work= (struct tcp_param_work*)work;
	struct net_filter_info* filter= my_work->filter;
	struct tcp_init_param_msg* msg;
	int msg_size,ret;
	struct list_head *iter= NULL;
        struct replica_id cold_replica;
        struct replica_id_list* objPtr;

	ret= create_tcp_init_param_msg(filter, my_work->connect_id, &my_work->tcp_param, &msg, &msg_size);
	if(ret)
		goto out;
	
	list_for_each(iter, &filter->ft_popcorn->cold_replicas_head.replica_list_member) {
                objPtr = list_entry(iter, struct replica_id_list, replica_list_member);
                cold_replica= objPtr->replica;

                pcn_kmsg_send_long(cold_replica.kernel, (struct pcn_kmsg_long_message*) msg, msg_size-sizeof(msg->header));

        }

	kfree(msg);
out:
	kfree(work);
	put_ft_filter(filter); 
	
}

static void send_tcp_init_param(struct socket* socket){
	struct net_filter_info* filter= socket->filter;
	struct inet_sock *inet = inet_sk(socket->sk);
        struct tcp_sock *tp = tcp_sk(socket->sk);
	struct tcp_param_work* work;
	int connect;

	spin_lock(&filter->lock);
	connect= ++filter->local_connect_id;	
	spin_unlock(&filter->lock);
	
	work= kmalloc(sizeof(*work), GFP_KERNEL);
	if(!work)
		return;

	INIT_WORK( (struct work_struct*)work, send_tcp_init_parameters_from_work);
        work->filter= filter;
	work->connect_id= connect;
        work->tcp_param.write_seq= tp->write_seq;
	work->tcp_param.inet_id= inet->inet_id;
	work->tcp_param.sport= inet->inet_sport;
	work->tcp_param.dport= inet->inet_dport;
	work->tcp_param.daddr= inet->inet_daddr;
	work->tcp_param.saddr= inet->inet_saddr;
	work->tcp_param.rcv_saddr= inet->inet_rcv_saddr;
	
	get_ft_filter(filter);

        queue_work(tx_notify_wq, (struct work_struct*)work);
}

void wait_tcp_init_param(struct net_filter_info* filter){
	struct inet_sock *inet = inet_sk(filter->ft_sock);
	struct tcp_sock *tp = tcp_sk(filter->ft_sock);

again:	spin_lock(&filter->lock);

	if(filter->hot_connect_id== filter->local_connect_id+1){
		tp->write_seq= filter->tcp_param.write_seq;
		inet->inet_id= filter->tcp_param.inet_id;
		filter->local_connect_id++;
	} 
	else{
		spin_unlock(&filter->lock);
		wait_event(*filter->wait_queue, filter->hot_connect_id== filter->local_connect_id+1);
		goto again;
	}

	spin_unlock(&filter->lock);
}

void ft_check_tcp_init_param(struct socket* socket){
	struct net_filter_info* filter;

	if(socket->filter_type & FT_FILTER_ENABLE){
		filter= socket->filter;
		if(filter->type & FT_FILTER_HOT_REPLICA){
			send_tcp_init_param(socket);
		}
		else{
			wait_tcp_init_param(filter);
		}
	}

}

/* Note call put_iphdr after using the iphdr in case 
 * of no errors.
 */
static int get_iphdr(struct sk_buff *skb, struct iphdr** ip_header,int *iphdrlen){
	int res= -EFAULT;
	struct iphdr* network_header= NULL;
	int len;

	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);
	skb_reset_mac_len(skb);

	if (skb->pkt_type == PACKET_OTHERHOST)
		goto out;

	if (!pskb_may_pull(skb, sizeof(struct iphdr)))
		goto out;

	if(skb_shared(skb))
		printk("%s: WARNING skb shared\n", __func__);

	network_header= ip_hdr(skb);

	if (network_header->ihl < 5 || network_header->version != 4)
		goto out;

	if (!pskb_may_pull(skb, network_header->ihl*4))
		goto out;

	network_header= ip_hdr(skb);

	if (unlikely(ip_fast_csum((u8 *)network_header, network_header->ihl)))
		goto out;

	len = ntohs(network_header->tot_len);
	if (skb->len < len || len < network_header->ihl*4)
		goto out;

	if (pskb_trim_rcsum(skb, len))
		goto out;

	/* Remove any debris in the socket control block */
	memset(IPCB(skb), 0, sizeof(struct inet_skb_parm));
	skb_orphan(skb);

	*iphdrlen= ip_hdrlen(skb);
	__skb_pull(skb, *iphdrlen);
	skb_reset_transport_header(skb);

	*ip_header= ip_hdr(skb);

	res= 0;

out:
	return res;
}

static void put_iphdr(struct sk_buff *skb, int iphdrlen){
	__skb_push(skb, iphdrlen);
}

static struct net_filter_info* try_get_ft_filter(struct sk_buff *skb){
	struct net_filter_info* ret= NULL;
	__be16 type;
	//int proto;
	int iphdrlen;
	struct iphdr *network_header;  // ip header struct
	struct tcphdr *tcp_header= NULL;     // tcp header struct
	struct udphdr *udp_header= NULL;     // udp header struct 
	struct sock* sk;
	struct socket* socket; 

	type= skb->protocol;
	/*proto= ntohs(eth_hdr(skb)->h_proto);
	
	if( type != cpu_to_be16(proto))
		printk("%s: WARNING type is %d proto is %d\n", __func__, type, proto);
	*/
	if( type == cpu_to_be16(ETH_P_IP)
		//|| type == cpu_to_be16(ETH_P_IPV6)
		){

		if(get_iphdr(skb, &network_header, &iphdrlen))
			goto out;

		if(network_header->protocol == IPPROTO_UDP
			|| network_header->protocol == IPPROTO_TCP){

			if (skb_dst(skb) == NULL) {
                        	int err = ip_route_input_noref(skb, network_header->daddr, network_header->saddr, network_header->tos, skb->dev);
                                if (unlikely(err)) {
                                	goto out_push;
                                }
                        }
	
	
			if (network_header->protocol == IPPROTO_UDP){
				udp_header= (struct udphdr *) ((char*)network_header+ network_header->ihl*4);

				sk = udp4_lib_lookup(dev_net(skb_dst(skb)->dev), network_header->saddr, udp_header->source,
				     network_header->daddr, udp_header->dest, inet_iif(skb));
				if(!sk)
					goto out_push;

			}
			else{
				if (skb->pkt_type != PACKET_HOST)
					goto out_push;

				if (!pskb_may_pull(skb, sizeof(struct tcphdr)))
					goto out_push;

				//tcp_header= (struct tcphdr *) ((char*)network_header+ network_header->ihl*4);
				tcp_header= tcp_hdr(skb);
				
				if (tcp_header->doff < sizeof(struct tcphdr) / 4)
					goto out_push;
				
				if (!pskb_may_pull(skb, tcp_header->doff * 4))
					goto out_push;

				tcp_header = tcp_hdr(skb);
				network_header = ip_hdr(skb);
				TCP_SKB_CB(skb)->seq = ntohl(tcp_header->seq);
			        TCP_SKB_CB(skb)->end_seq = (TCP_SKB_CB(skb)->seq + tcp_header->syn + tcp_header->fin +
	                                     skb->len - tcp_header->doff * 4);
			        TCP_SKB_CB(skb)->ack_seq = ntohl(tcp_header->ack_seq);
				TCP_SKB_CB(skb)->when    = 0;
				TCP_SKB_CB(skb)->ip_dsfield = ipv4_get_dsfield(network_header);
				TCP_SKB_CB(skb)->sacked  = 0;

				sk = find_tcp_sock(skb, tcp_header);
				if(!sk)
					goto out_push;

			}
				
			socket= sk->sk_socket;
                                if(!socket)
                                        goto out_push;

                        if(socket->filter_type & FT_FILTER_ENABLE){
	                        ret= socket->filter;
				FTPRINTK("%s: saddr %d daddr %d sport %d dport %d\n", __func__, network_header->saddr, network_header->daddr,  (tcp_header!=NULL)?( tcp_header->source):(udp_header->source),(tcp_header!=NULL)?(tcp_header->dest):(udp_header->dest));
			}

		}	
		put_iphdr(skb, iphdrlen);
	}

out:
	return ret;

out_push:
	put_iphdr(skb, iphdrlen);
	goto out;
}

int net_ft_rx_filter(struct sk_buff *skb){
	int ret= 0;
	struct net_filter_info * filter;

	filter= try_get_ft_filter(skb);
	if(!filter)
		goto out;

	if(IS_ERR(filter)){
		ret= PTR_ERR(filter);
		goto out;
	}

	if(filter->type & FT_FILTER_COLD_REPLICA){
		ret= rx_filter_cold(filter);
		goto out;
	}

        if(filter->type & FT_FILTER_HOT_REPLICA){
               	ret= rx_filter_hot(filter, skb);
		goto out;
        }

out:
	return ret;
}

/* ARGH... no clues on what to do with the timestemps...
 *
 */
int ft_check_tcp_timestamp(struct sock* sk){
	struct socket* socket= sk->sk_socket;
	
	if(socket->filter_type & FT_FILTER_ENABLE){
		if(socket->filter->type & FT_FILTER_COLD_REPLICA){
			return 0;
		}

	}
	
	return 1;
}

static int __init ft_filter_init(void){

	tx_notify_wq= create_singlethread_workqueue("tx_notify_wq");

	INIT_LIST_HEAD(&filter_list_head.list_member);	
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_TX_NOTIFY, handle_tx_notify);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_RX_COPY, handle_rx_copy);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_TCP_INIT_PARAM, handle_tcp_init_param);

	return 0;
}

late_initcall(ft_filter_init);
