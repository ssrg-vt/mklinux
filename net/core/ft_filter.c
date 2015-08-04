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
#include <net/checksum.h>

#define FT_FILTER_VERBOSE 0 
#if FT_FILTER_VERBOSE
#define FTPRINTK(...) printk(__VA_ARGS__)
#else
#define FTPRINTK(...) ;
#endif

struct rx_copy_msg{
	struct pcn_kmsg_hdr header;
        struct ft_pid creator;
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

struct ft_sk_buff_tcp_list{
	struct ft_sk_buff_list ft_sk_buff_list_common;
	__u32 seq;
	__u32 seq_ack;
	__u16 syn:1;
	__u16 ack:1;
	__u16 fin:1;
};

struct tx_notify_msg{
        struct pcn_kmsg_hdr header;
	struct ft_pid creator;
	int filter_id;
	int is_child;
        __be16 dport;
        __be32 daddr;
        long long pckt_id;
	__wsum csum;
	__u32 seq;
        __u32 seq_ack;
        __u16 syn:1;
        __u16 ack:1;
	__u16 fin:1;
};

struct tcp_init_param_msg{
        struct pcn_kmsg_hdr header;
        struct ft_pid creator;
        int filter_id;
	int is_child;
        __be16 dport;
        __be32 daddr;
        int connect_id;
	int accept_id;
	struct tcp_init_param tcp_param;
};

struct tx_notify_work{
        struct work_struct work;
        struct net_filter_info *filter;
	long long pckt_id;
	__wsum csum;
	struct sk_buff* skb;
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
	int accept_id;
	struct tcp_init_param tcp_param;
};

struct wq_member{
	struct list_head entry;
	struct workqueue_struct *wq;
};

struct stack{
	int size;
	spinlock_t stack_lock;
	struct list_head stack_head;
};

static struct workqueue_struct *workqueues_creator_wq;
static struct workqueue_struct *tx_notify_wq;
extern int _cpu;
struct net_filter_info filter_list_head;
DEFINE_SPINLOCK(filter_list_lock);

#define MAX_WQ_POOL	50
#define THRESHOLD_WQ_POOL	(MAX_WQ_POOL/2)
struct stack wq_stack;

static int get_iphdr(struct sk_buff *skb, struct iphdr** ip_header,int *iphdrlen);
static void put_iphdr(struct sk_buff *skb, int iphdrlen);

static struct workqueue_struct * add_wq_to_pool(struct workqueue_struct *wq, int force_add){
        struct wq_member *new_entry;
        struct workqueue_struct *ret= wq;
        
        if(!wq)
                return ret;
        
        new_entry= kmalloc(sizeof(*new_entry), GFP_ATOMIC);
        if(!new_entry) 
                return ret;
                
        INIT_LIST_HEAD(&new_entry->entry);
        new_entry->wq= wq; 
                
        spin_lock(&wq_stack.stack_lock);
        if(force_add || wq_stack.size < MAX_WQ_POOL){
                list_add(&new_entry->entry, &wq_stack.stack_head);
                wq_stack.size++;
                ret= NULL;
        }
        spin_unlock(&wq_stack.stack_lock);

        if(ret){
                kfree(new_entry);
        }

        return ret;

}

static void create_more_working_queues(struct work_struct* work){
	static atomic_t id= ATOMIC_INIT(0);
	static const int NAME_SIZE= 200;
	char name[NAME_SIZE];
	struct workqueue_struct *wq;
	int ret;

	if(in_interrupt())
		return;

	do{
		ret= snprintf(name, NAME_SIZE, "ft_wq_%d", atomic_inc_return(&id));
		if(ret==NAME_SIZE){
			printk("%s ERROR: name field too small\n", __func__);
			return;
		}
		
		wq= create_singlethread_workqueue(name); 
		if(wq)
			wq= add_wq_to_pool(wq, 0);
	}
	while(wq==NULL);

	destroy_workqueue(wq);
}

static struct workqueue_struct * remove_wq_from_pool(void){
 	struct workqueue_struct *ret= NULL;
        struct wq_member *entry= NULL;
	int create_more= 0;
	struct work_struct *work;

	spin_lock(&wq_stack.stack_lock);
        if(wq_stack.size > 0){
		entry= (struct wq_member *) list_first_entry(&wq_stack.stack_head, struct wq_member, entry);
		list_del(&entry->entry);
		wq_stack.size--;
	}
	if(wq_stack.size < THRESHOLD_WQ_POOL){
		create_more= 1;
	}
        spin_unlock(&wq_stack.stack_lock);

	if(entry){
		ret= entry->wq;
		kfree(entry);
	}

	if(create_more){
		work= kmalloc(sizeof(*work), GFP_ATOMIC);
		if(work){
			INIT_WORK(work, create_more_working_queues);
		        queue_work(workqueues_creator_wq, (struct work_struct*)work);
		}
	}

	return ret;
}

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
static struct ft_sk_buff_list* remove_ft_buff_entry(struct ft_sk_buff_list* list_head, long long pckt_id){
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
	}

	return entry;
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
//#if FT_FILTER_VERBOSE
	char* filter_printed;
//#endif
	filter= container_of(kref, struct net_filter_info, kref);
	if (filter){
		if(!(filter->type & FT_FILTER_FAKE)){
			remove_filter(filter);
		}
/*#if FT_FILTER_VERBOSE
                filter_printed= print_filter_id(filter);
                FTPRINTK("%s: deleting %s filter %s\n", __func__, (filter->type & FT_FILTER_FAKE)?"fake":"", filter_printed);
                if(filter_printed)
                        kfree(filter_printed);
#endif*/
                filter_printed= print_filter_id(filter);
                printk("%s: deleting %s filter %s pckt rcv %d pckt snt %d\n", __func__, (filter->type & FT_FILTER_FAKE)?"fake":"", filter_printed, filter->local_rx, filter->local_tx);
                if(filter_printed)
                        kfree(filter_printed);
		if(filter->ft_popcorn)
			put_ft_pop_rep(filter->ft_popcorn);
		if(filter->wait_queue)
			kfree(filter->wait_queue);
		if(filter->rx_copy_wq){
			add_wq_to_pool(filter->rx_copy_wq, 1);
		}

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

static struct net_filter_info* find_and_get_filter(struct ft_pid *creator, int filter_id, int is_child, __be32 daddr, __be16 dport){

	struct net_filter_info* filter= NULL;
        struct list_head *iter= NULL;
        struct net_filter_info *objPtr= NULL;

        spin_lock(&filter_list_lock);

        list_for_each(iter, &filter_list_head.list_member) {
                objPtr = list_entry(iter, struct net_filter_info, list_member);
                if( are_ft_pid_equals(&objPtr->creator, creator)
			&& objPtr->id == filter_id){
			
			if( !is_child && !(objPtr->type & FT_FILTER_CHILD) ){
                        	filter= objPtr;
                        	get_ft_filter(filter);
                        	goto out;
			}
			
			if( is_child && (objPtr->type & FT_FILTER_CHILD) &&
				daddr == objPtr->tcp_param.daddr &&
				dport == objPtr->tcp_param.dport ){

                                filter= objPtr;
                                get_ft_filter(filter);
                                goto out;
                        }

			
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
	int is_child= (filter->type & FT_FILTER_CHILD);
	struct workqueue_struct *filter_wq;

        spin_lock(&filter_list_lock);

        list_for_each(iter, &filter_list_head.list_member) {
                objPtr = list_entry(iter, struct net_filter_info, list_member);
                if( are_ft_pid_equals(&objPtr->creator, &filter->creator)
                        && objPtr->id == filter->id){

			if( !is_child && !(objPtr->type & FT_FILTER_CHILD) ){
                        	fake_filter= objPtr;
                        	goto next;
			}

                        if( is_child && (objPtr->type & FT_FILTER_CHILD) &&
                                filter->tcp_param.daddr == objPtr->tcp_param.daddr &&
                                filter->tcp_param.dport == objPtr->tcp_param.dport ){
                                
                                fake_filter= objPtr;
				goto next;
                        }

                }

        }

next:	if(fake_filter){

		if(!(fake_filter->type & FT_FILTER_FAKE))
			printk("ERROR %s: substituting a real filter\n",__func__);
		
		kfree(filter->wait_queue);
		filter_wq= filter->rx_copy_wq;
		
		spin_lock(&fake_filter->lock);
		
		filter->local_tx= fake_filter->local_tx;
		filter->hot_tx= fake_filter->hot_tx;
		filter->local_rx= fake_filter->local_rx;
		filter->hot_rx= fake_filter->hot_rx;
	
		filter->hot_connect_id= fake_filter->hot_connect_id;
        	filter->local_connect_id= fake_filter->local_connect_id;
		filter->hot_accept_id= fake_filter->hot_accept_id;
                filter->local_accept_id= fake_filter->local_accept_id;

		filter->tcp_param= fake_filter->tcp_param;
		
		filter->wait_queue= fake_filter->wait_queue;	
		fake_filter->wait_queue= NULL;

		filter->rx_copy_wq= fake_filter->rx_copy_wq;
		fake_filter->rx_copy_wq= NULL;

		fake_filter->type &= ~FT_FILTER_ENABLE;

		list_del(&fake_filter->list_member);

		spin_unlock(&fake_filter->lock);
	
	}

	list_add(&filter->list_member,&filter_list_head.list_member);
	spin_unlock(&filter_list_lock);
        
	if(fake_filter){
		add_wq_to_pool(filter_wq, 1);
		wake_up(filter->wait_queue);
		put_ft_filter(fake_filter);
	}

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
	int is_child= (filter->type & FT_FILTER_CHILD);

        spin_lock(&filter_list_lock);

        list_for_each(iter, &filter_list_head.list_member) {
                objPtr = list_entry(iter, struct net_filter_info, list_member);
                if( are_ft_pid_equals(&objPtr->creator, &filter->creator)
                        && objPtr->id == filter->id){

			if( !is_child && !(objPtr->type & FT_FILTER_CHILD) ){
                                real_filter= objPtr;
                                goto next;
                        }

                        if( is_child && (objPtr->type & FT_FILTER_CHILD) &&
                                filter->tcp_param.daddr == objPtr->tcp_param.daddr &&
                                filter->tcp_param.dport == objPtr->tcp_param.dport ){

                                real_filter= objPtr;
                                goto next;
                        }

                }

        }

next:   if(!real_filter){
		list_add(&filter->list_member,&filter_list_head.list_member);
	}

	spin_unlock(&filter_list_lock);

	if(real_filter){
		put_ft_filter(filter);
	}
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
	int rsize,ret, is_child;

        if(!filter)
                return NULL;

	is_child= (filter->type & FT_FILTER_CHILD);
	creator_printed= print_ft_pid(&filter->creator);
	if(!creator_printed)
		return NULL;

        string= kmalloc(size,GFP_ATOMIC);
        if(!string)
                return NULL;
	
	rsize= size;
        ret= snprintf(string, rsize, "{ creator: %s, id %d", creator_printed, filter->id);
	if (ret>= rsize)
		goto out_clean;
	
	rsize= rsize-ret;
	if(is_child){
		ret= snprintf(&string[ret], rsize, ", daddr: %i, dport: %i}", filter->tcp_param.daddr, filter->tcp_param.dport);
                if(ret>=rsize)
                        goto out_clean;

	}
	else{
		ret= snprintf(&string[ret], rsize, "}");
		if(ret>=rsize)
			goto out_clean;
	}
	
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
static int create_fake_filter(struct ft_pid *creator, int filter_id, int is_child, __be32 daddr, __be16 dport){
	struct net_filter_info* filter;
#if FT_FILTER_VERBOSE
	char* filter_id_printed;
#endif

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
	filter->ft_sock= NULL;
	filter->ft_req= NULL;
	spin_lock_init(&filter->lock);
	filter->rx_copy_wq= remove_wq_from_pool();
	if(filter->rx_copy_wq == NULL){
		printk("%s not enougth wq available\n", __func__);
		kfree(filter->wait_queue);
		kfree(filter);
		return -EFAULT;
	}
	init_waitqueue_head(filter->wait_queue);
	INIT_LIST_HEAD(&filter->skbuff_list.list_member);
	
	filter->type= FT_FILTER_ENABLE; 
	filter->type|= FT_FILTER_FAKE;

	memset(&filter->tcp_param,0,sizeof(filter->tcp_param));

	filter->id= filter_id;

        filter->local_tx= 0;
        filter->hot_tx= 0;
        filter->local_rx= 0;
        filter->hot_rx= 0;

	filter->hot_connect_id= 0;
	filter->local_connect_id= 0;

	filter->hot_accept_id= 0;
        filter->local_accept_id= 0;

	if(is_child){
                filter->type|= FT_FILTER_CHILD;
                filter->tcp_param.daddr= daddr;
                filter->tcp_param.dport= dport;
                filter->local_tx= -1;
        }

#if FT_FILTER_VERBOSE
        filter_id_printed= print_filter_id(filter);
	FTPRINTK("%s: pid %d created new filter %s\n\n", __func__, current->pid, filter_id_printed);
        if(filter_id_printed)
  	      kfree(filter_id_printed);
#endif

	add_filter_with_check(filter);

	return 0;
}


void ft_grown_mini_filter(struct sock* sk, struct request_sock *req){
	if(req->ft_filter){
		get_ft_filter(req->ft_filter);
		req->ft_filter->ft_sock= sk;
		req->ft_filter->ft_req= NULL;
		sk->ft_filter= req->ft_filter;
	}
}

int ft_create_mini_filter(struct request_sock *req, struct sock *sk, struct sk_buff * skb){
	struct net_filter_info* parent_filter= sk->ft_filter;
        struct net_filter_info* filter;
	__be16 dport = tcp_hdr(skb)->source;
        __be32 daddr = ip_hdr(skb)->saddr;
#if FT_FILTER_VERBOSE
	char* filter_id_printed;
#endif

	if(parent_filter){
		filter= kmalloc(sizeof(*filter), GFP_ATOMIC);
                if(!filter)
                        return -ENOMEM;

                filter->wait_queue= kmalloc(sizeof(*filter->wait_queue), GFP_ATOMIC);
                if(!filter->wait_queue){
                        kfree(filter);
                        return -ENOMEM;
                }

                INIT_LIST_HEAD(&filter->list_member);
                atomic_set(&filter->kref.refcount, 1);
                filter->creator= parent_filter->creator;
		get_ft_pop_rep(parent_filter->ft_popcorn);
                filter->ft_popcorn= parent_filter->ft_popcorn;
		filter->ft_sock= NULL;
		filter->ft_req= req;
                spin_lock_init(&filter->lock);
                init_waitqueue_head(filter->wait_queue);
                INIT_LIST_HEAD(&filter->skbuff_list.list_member);
		
		filter->type= parent_filter->type | FT_FILTER_CHILD;
                filter->id= parent_filter->id;

		memset(&filter->tcp_param,0,sizeof(filter->tcp_param));

                filter->tcp_param.daddr= daddr;
                filter->tcp_param.dport= dport;
                
		filter->rx_copy_wq= remove_wq_from_pool();
	        if(filter->rx_copy_wq == NULL){
        	        printk("%s not enougth wq available\n", __func__);
                	kfree(filter->wait_queue);
                	kfree(filter);
                	return -EFAULT;
        	}

                filter->local_tx= -1;
                filter->hot_tx= 0;
                filter->local_rx= 0;
                filter->hot_rx= 0;

                filter->hot_connect_id= 0;
                filter->local_connect_id= 0;

                filter->hot_accept_id= 0;
                filter->local_accept_id= 0;


		if(filter->type & FT_FILTER_COLD_REPLICA){
                	add_filter_coping_pending(filter);
		}
		else{
			add_filter(filter);
		}

		req->ft_filter= filter;

#if FT_FILTER_VERBOSE
                filter_id_printed= print_filter_id(filter);
		FTPRINTK("%s: pid %d created new filter %s\n\n", __func__, current->pid, filter_id_printed);
		if(filter_id_printed)
			kfree(filter_id_printed);		
#endif

	}
	else{
		req->ft_filter= NULL;
	}

	return 0;
}

/* Create a struct net_filter_info* real_filter and add it in filter_list_head.
 * The filter created will be associated with the struct ft_pop_rep ft_popcorn of task.
 * 
 * Returns 0 in case of success.
 */
int create_filter(struct task_struct *task, struct sock *sk, gfp_t priority){
	struct net_filter_info* filter;
	struct task_struct *ancestor;
#if FT_FILTER_VERBOSE
        char* filter_id_printed;
#endif
	if(in_interrupt())
		return 0;

	if(task->replica_type == HOT_REPLICA 
		|| task->replica_type == COLD_REPLICA
		|| task->replica_type == NEW_HOT_REPLICA_DESCENDANT
		|| task->replica_type == NEW_COLD_REPLICA_DESCENDANT
		|| task->replica_type == REPLICA_DESCENDANT){
		
		filter= kmalloc(sizeof(*filter), priority);
		if(!filter)
			return -ENOMEM;
		
		filter->wait_queue= kmalloc(sizeof(*filter->wait_queue), priority);
	        if(!filter->wait_queue){
        	        kfree(filter);
                	return -ENOMEM;
        	}

		INIT_LIST_HEAD(&filter->list_member);
		atomic_set(&filter->kref.refcount, 1);
		filter->creator= task->ft_pid;
		get_ft_pop_rep(task->ft_popcorn);
		filter->ft_popcorn= task->ft_popcorn;
		filter->ft_sock= sk;
		filter->ft_req= NULL;
		spin_lock_init(&filter->lock);
		init_waitqueue_head(filter->wait_queue);
		INIT_LIST_HEAD(&filter->skbuff_list.list_member);

		/* NOTE: target applications are deterministic, so all replicas will do the same actions 
                 * on the same order.
                 * Because of this, all replicas will and up creating this socket, and giving it the same id.
                 */

		filter->id= task->next_id_resources++;

		filter->rx_copy_wq= remove_wq_from_pool();
	        if(filter->rx_copy_wq == NULL){
        	        printk("%s not enougth wq available\n", __func__);
                	kfree(filter->wait_queue);
                	kfree(filter);
                	return -EFAULT;
        	}

                filter->local_tx= 0;
                filter->hot_tx= 0;
                filter->local_rx= 0;
                filter->hot_rx= 0;

		filter->hot_connect_id= 0;
	        filter->local_connect_id= 0;
		
		filter->hot_accept_id= 0;
	        filter->local_accept_id= 0;

		memset(&filter->tcp_param,0,sizeof(filter->tcp_param));
	
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
				printk("%s: ERROR ancestor pid %d tgid %d has replica type %d (current pid %d tgid %d)\n",__func__, ancestor->pid, ancestor->tgid, ancestor->replica_type, task->pid, task->tgid);
				put_ft_pop_rep(filter->ft_popcorn);
				kfree(filter->wait_queue);
				kfree(filter);
				return -EFAULT;
			}
			
		}		

		sk->ft_filter= filter;

#if FT_FILTER_VERBOSE
		filter_id_printed= print_filter_id(filter);
        	FTPRINTK("%s: pid %d created new filter %s\n\n", __func__, current->pid, filter_id_printed);

	    	if(filter_id_printed)
                	kfree(filter_id_printed);
#endif
		
	}else{
		sk->ft_filter= NULL;
	}	

	return 0;	
}

/* Compute a checksum of the application data of an skb.
 * For now, it assumes network prot IP and transport TCP/UDP.
 * It uses headers API, so call it only after net and trans stack
 * have been called in tx. 
 * So call it before going to link layer but after network!
 */
static __wsum compute_user_checksum(struct sk_buff* skb){
	unsigned char *app;
	struct iphdr* network_header;
	struct tcphdr *tcp_header= NULL;     // tcp header struct
        struct udphdr *udp_header= NULL;
	unsigned int head_len= 0, size;	
	__wsum res;

        network_header= (struct iphdr *)skb_network_header(skb);
	head_len= ip_hdrlen(skb);
        if(network_header->protocol == IPPROTO_UDP){
		udp_header= udp_hdr(skb);	
		head_len+= sizeof(*udp_header);
        }else{
		tcp_header= tcp_hdr(skb);
		head_len+= tcp_hdrlen(skb);
	}

	size= ntohs(network_header->tot_len)- head_len;
	app= kmalloc(size, GFP_ATOMIC);
	if(!app)
		return 0;

	skb_copy_bits(skb, head_len,(void*) app, size);
	
	res= csum_partial(app, size, 0);
	
	//FTPRINTK("%s len %u head_len %u data len %d len-head_len-data_len %u size %u skb->csum %u seq %u seq_end %u h_seq %u fin %u syn %u csum %u\n", __func__, skb->len, head_len, skb->data_len, skb->len-skb->data_len-head_len, size, skb->csum,TCP_SKB_CB(skb)->seq,TCP_SKB_CB(skb)->end_seq,tcp_hdr(skb)->seq,tcp_hdr(skb)->fin,tcp_hdr(skb)->syn, res);
	
	kfree(app);
		
	return res;
}

static int check_msg(struct ft_sk_buff_list *copy, struct ft_sk_buff_list *copy2, struct net_filter_info *filter, int is_tcp){
        char* ft_filter_printed;
        int ret= 0;
        struct ft_sk_buff_tcp_list *tcp_copy= NULL;
	struct ft_sk_buff_tcp_list *tcp_copy2= NULL;

        if(is_tcp){
		
		tcp_copy= (struct ft_sk_buff_tcp_list *)copy;
	        tcp_copy2= (struct ft_sk_buff_tcp_list *)copy2;

                if(tcp_copy->syn != tcp_copy2->syn 
			|| tcp_copy->fin != tcp_copy2->fin
			|| tcp_copy->ack != tcp_copy2->ack){
                        printk("%s ERROR syn1 %u ack1 %u fin1 %u syn2 %u ack2 %u fin2 %u\n", __func__, tcp_copy->syn, tcp_copy->ack, tcp_copy->fin, tcp_copy2->syn, tcp_copy2->ack, tcp_copy2->fin);
                        ret= -EFAULT;
                        goto out;
                }

                if(tcp_copy->seq != tcp_copy2->seq){
                        printk("%s ERROR svd seq %u rcv seq %u\n", __func__, ntohl(tcp_copy->seq), ntohl(tcp_copy2->seq));
                        ret= -EFAULT;
                        goto out;

                }

           	if(tcp_copy->syn || tcp_copy->fin){
			//syn and synack have one byte of fake payload, do not check it.
			goto out;
		}
			
		if(tcp_copy->seq_ack != tcp_copy2->seq_ack){
                        printk("%s ERROR svd ack %u rcv ack %u\n", __func__, ntohl(tcp_copy->seq_ack), ntohl(tcp_copy2->seq_ack));
                        ret= -EFAULT;
                        goto out;

                }

        }

        /* This is a check on only the application data,
         * not transport/network protol headers.
         */
        if(copy->csum != copy2->csum){
                ret= -EFAULT;
                goto out;

        }

out:
        if(ret){
                ft_filter_printed= print_filter_id(filter);	
		if(!is_tcp)
                	printk("%s ERROR in filter %s: csum of pckt id %lld does not match (%u %u)\n", __func__, ft_filter_printed, copy->pckt_id, copy->csum, copy2->csum);
                else
			printk("%s ERROR in filter %s: csum of pckt id %lld does not match (%u %u) (syn %u ack %u fin %u seq %u ack_seq %u)\n", __func__, ft_filter_printed, copy->pckt_id, copy->csum, copy2->csum, tcp_copy->syn, tcp_copy->ack, tcp_copy->fin, ntohl(tcp_copy->seq), ntohl(tcp_copy->seq_ack));
		if(ft_filter_printed)
                        kfree(ft_filter_printed);

        }
        return ret;
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
	struct ft_sk_buff_list *entry= NULL, *new_entry;
	struct ft_sk_buff_tcp_list *new_entry_tcp;
	wait_queue_head_t *filter_wait_queue= NULL;
	int removing_fake= 0;
	int is_tcp= 0;
#if FT_FILTER_VERBOSE
        //char* ft_filter_printed;
	char* ft_pid_printed;
#endif


again:	filter= find_and_get_filter(&msg->creator, msg->filter_id, msg->is_child, msg->daddr, msg->dport);
	if(filter){
		spin_lock(&filter->lock);
		if(filter->type & FT_FILTER_ENABLE){ 
			if( (filter->ft_sock && filter->ft_sock->sk_protocol == IPPROTO_TCP) ||
				(filter->ft_req!=NULL) ){
				is_tcp= 1;
				new_entry= kmalloc(sizeof(struct ft_sk_buff_tcp_list), GFP_ATOMIC);
                        	if(!new_entry){
					spin_unlock(&filter->lock);
                                	put_ft_filter(filter);
                                	goto out;
                        	}

				new_entry_tcp= (struct ft_sk_buff_tcp_list *) new_entry;
				new_entry_tcp->seq= msg->seq;
				new_entry_tcp->seq_ack= msg->seq_ack;
				new_entry_tcp->syn= msg->syn;
				new_entry_tcp->ack= msg->ack;
				new_entry_tcp->fin= msg->fin;

				}
			else{
				new_entry= kmalloc(sizeof(*new_entry), GFP_ATOMIC);
				if(!new_entry){
					spin_unlock(&filter->lock);
					put_ft_filter(filter);
					goto out;
				}

			}
			new_entry->csum= msg->csum;
			new_entry->pckt_id= msg->pckt_id;
			new_entry->skbuff= NULL;

			
			if(filter->hot_tx < msg->pckt_id)
				filter->hot_tx= msg->pckt_id;

			filter_wait_queue= filter->wait_queue;
			
			entry= remove_ft_buff_entry(&filter->skbuff_list, msg->pckt_id);
			if(!entry){
				FTPRINTK("%s adding packt in list\n", __func__);
				add_ft_buff_entry(&filter->skbuff_list, new_entry);
			}
		}
		else{
			removing_fake= 1;
		}
        	spin_unlock(&filter->lock);
		
		if(removing_fake){
			put_ft_filter(filter);
			removing_fake= 0;
			goto again;
		}
		
		if(entry){
			
			check_msg(entry, new_entry, filter, is_tcp);	
		
			kfree(new_entry);
			kfree_skb(entry->skbuff);
                	kfree(entry);
			entry= NULL;
			//who added entry should have got filter for me...
                        put_ft_filter(filter);

		}

		wake_up(filter_wait_queue);
		
		put_ft_filter(filter);
	}
	else{
#if FT_FILTER_VERBOSE
        	ft_pid_printed= print_ft_pid(&msg->creator);
        	FTPRINTK("%s: creating fake filter for ft_pid %s id %d\n\n", __func__, ft_pid_printed, msg->filter_id);
        	if(ft_pid_printed)
                	kfree(ft_pid_printed);
#endif

		err= create_fake_filter(&msg->creator, msg->filter_id, msg->is_child, msg->daddr, msg->dport);
		if(!err)
			goto again;
	}
	
out:
	pcn_kmsg_free_msg(msg);
	
	return 0;
}

/* Creates a struct tx_notify_msg message.
 * In case of success 0 is returned and msg and msg_size are properly populated.
 *
 * Remember to kfree the message eventually.
 */
static int create_tx_notify_msg(struct net_filter_info *filter, long long pckt_id, __wsum csum, struct sk_buff* skb, struct tx_notify_msg** msg, int* msg_size){
	struct tx_notify_msg* message;
	
	message= kmalloc(sizeof(*message), GFP_ATOMIC);
	if(!message)
		return -ENOMEM;

	message->creator= filter->creator;
	message->filter_id= filter->id;
	message->is_child= filter->type & FT_FILTER_CHILD;
	if(message->is_child){
		message->daddr= filter->tcp_param.daddr;
		message->dport= filter->tcp_param.dport;
	}
		
	message->pckt_id= pckt_id;
	message->csum= csum;
	if(filter->ft_sock->sk_protocol == IPPROTO_TCP){
		message->seq= tcp_hdr(skb)->seq;
		message->seq_ack= tcp_hdr(skb)->ack_seq;
		message->syn= tcp_hdr(skb)->syn;
		message->ack= tcp_hdr(skb)->ack;
		message->fin= tcp_hdr(skb)->fin;
	}
	
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
#if FT_FILTER_VERBOSE
	char *filter_id_printed;
#endif

	ret= create_tx_notify_msg(filter, tx_n_work->pckt_id, tx_n_work->csum, tx_n_work->skb, &msg, &msg_size);
	if(ret)
		goto out;

#if FT_FILTER_VERBOSE
        filter_id_printed= print_filter_id(filter);
        FTPRINTK("%s: reached send of packet %llu in filter %s (syn %u ack %u fin %u seq %u ack_seq %u csum %u) \n\n", __func__, tx_n_work->pckt_id, filter_id_printed, msg->syn, msg->ack, msg->fin, msg->seq, msg->seq_ack, tx_n_work->csum);
        if(filter_id_printed)
                kfree(filter_id_printed);

#endif

	/*char *filter_id_printed;
	filter_id_printed= print_filter_id(filter);
	printk("%s: reached send of packet %llu in filter %s \n", __func__, tx_n_work->pckt_id, filter_id_printed);
	if(filter_id_printed)
                kfree(filter_id_printed);
	*/

        list_for_each(iter, &filter->ft_popcorn->cold_replicas_head.replica_list_member) {
                objPtr = list_entry(iter, struct replica_id_list, replica_list_member);
                cold_replica= objPtr->replica;

		pcn_kmsg_send_long(cold_replica.kernel, (struct pcn_kmsg_long_message*) msg, msg_size-sizeof(msg->header));
	
        }
	
	kfree(msg);

out:	kfree_skb(tx_n_work->skb);
	kfree(work);	
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
static int tx_filter_hot(struct net_filter_info *filter, struct sk_buff* skb){
        long long pckt_id;
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

        INIT_WORK( (struct work_struct*)work, send_tx_notification);
        get_ft_filter(filter);
	work->filter= filter;
	work->pckt_id= pckt_id;

	/* compute it here bacause the structure of the skb may change after when pushing 
	 * it on the link layer.
	 */ 
	work->csum= compute_user_checksum(skb);
	skb_get(skb);
	work->skb= skb; 

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
	long long pckt_id;
	struct ft_sk_buff_list *buff_entry, *old_buff_entry= NULL;
	struct ft_sk_buff_tcp_list *buff_entry_tcp;
	int sk_buff_added= 0;
	__wsum csum;
	char* filter_id_printed;
#if FT_FILTER_VERBOSE
	char* ft_pid_printed;
#endif

	if(filter->ft_sock->sk_protocol == IPPROTO_TCP){
		buff_entry= kmalloc(sizeof(struct ft_sk_buff_tcp_list), GFP_ATOMIC);
		if(!buff_entry){
			return -ENOMEM;
		}

		buff_entry_tcp= (struct ft_sk_buff_tcp_list *)buff_entry;
		buff_entry_tcp->seq= tcp_hdr(skb)->seq;
		buff_entry_tcp->seq_ack= tcp_hdr(skb)->ack_seq;
		buff_entry_tcp->syn= tcp_hdr(skb)->syn;
		buff_entry_tcp->ack= tcp_hdr(skb)->ack;
		buff_entry_tcp->fin= tcp_hdr(skb)->fin;
	}
	else{

		buff_entry= kmalloc(sizeof(*buff_entry),GFP_ATOMIC);
		if(!buff_entry){
			return -ENOMEM;
		}
	}
	skb_get(skb);
	
	csum= compute_user_checksum(skb);
	buff_entry->csum= csum;

	spin_lock(&filter->lock);
	
	pckt_id= ++filter->local_tx;
	buff_entry->pckt_id= pckt_id;

	if(filter->hot_tx < pckt_id){
		//increment kref of filter to let it active for the handler of tx_notify
		get_ft_filter(filter);
	
		add_ft_buff_entry(&filter->skbuff_list, buff_entry);
		sk_buff_added= 1;
		FTPRINTK("%s: pid %d saved packet %llu \n\n", __func__, current->pid, pckt_id);
	}
	else{
		old_buff_entry= remove_ft_buff_entry(&filter->skbuff_list, pckt_id);
	}
	spin_unlock(&filter->lock);
	
	if(sk_buff_added == 0){
		if(old_buff_entry){
			check_msg(buff_entry, old_buff_entry, filter, filter->ft_sock->sk_protocol == IPPROTO_TCP);
			kfree(old_buff_entry);
		}
		else{
			filter_id_printed= print_filter_id(filter);
			printk("%s ERROR in filter %s: no pack entry id %lld for checking csum \n",__func__, filter_id_printed, pckt_id);
			if(filter_id_printed)
				kfree(filter_id_printed);
		}
		kfree_skb(skb);
		kfree(buff_entry);
	}

	

#if FT_FILTER_VERBOSE
        ft_pid_printed= print_ft_pid(&current->ft_pid);
        filter_id_printed= print_filter_id(filter);
        FTPRINTK("%s: pid %d ft_pid %s tx packet %llu csum %d in filter %s\n", __func__, current->pid, ft_pid_printed, pckt_id, csum, filter_id_printed);
        if(filter->ft_sock->sk_protocol == IPPROTO_TCP)
		FTPRINTK(" syn %u ack %u fin %u seq %u ack_seq %u\n", tcp_hdr(skb)->syn, tcp_hdr(skb)->ack, tcp_hdr(skb)->fin, tcp_hdr(skb)->seq, tcp_hdr(skb)->ack_seq);
	if(ft_pid_printed)
                kfree(ft_pid_printed);
        if(filter_id_printed)
                kfree(filter_id_printed);
#endif

	wake_up(filter->wait_queue);
	return FT_TX_DROP;

}

/* Packet filter for ft-popcorn, activated only if sk is associated
 * with a replicated thread.
 *
 * In case the sock was created by an hot replica associated thread, 
 * a notification message is sent to all cold replicas.
 *
 * In case the socket was created by a cold replica associated thread,
 * the execution is delayed while the corresponding packet notification
 * is received from the hot replica.
 * 
 * In case of error a value < 0 is returned, otherwise FT_TX_OK or FT_TX_DROP
 * is returned.
 */
int net_ft_tx_filter(struct sock* sk, struct sk_buff *skb){
	int ret= FT_TX_OK;
	struct net_filter_info *filter;

	if(!is_pckt_to_filter(skb))
		goto out;		
	
	if(!sk){
		printk("%s: WARNING sock is null for pid %d\n", __func__, current->pid);
                goto out;
	}

	if(!sk->ft_filter)
		goto out;

	filter= sk->ft_filter;

	if(!filter){
        	//BUG();
                printk("%s: ERROR filter is null\n",__func__);
                goto out;
	}


	if(filter->type & FT_FILTER_COLD_REPLICA){
		return tx_filter_cold(filter, skb);
	}

	if(filter->type & FT_FILTER_HOT_REPLICA){
		return tx_filter_hot(filter, skb);
	}	

out:	return ret;
}

static void fake_parameters(struct sk_buff *skb, struct net_filter_info *filter){
	struct inet_sock *inet;
	struct inet_request_sock *ireq;
	int res, iphdrlen, datalen, msg_changed;
        struct iphdr *network_header; 
	struct tcphdr *tcp_header= NULL;     // tcp header struct
        struct udphdr *udp_header= NULL;     // udp header struct
	__be16 sport;
	__be32 saddr;

	/* I need to fake the receive of this skbuff from a device.
         * I use dummy net driver for that.
         */
        skb->dev= dev_get_by_name(&init_net, DUMMY_DRIVER);

	/* The local IP or port may be different, 
	 * hack the message with the correct ones.
	 */
	inet = inet_sk(filter->ft_sock);
	if(!inet){
		ireq= inet_rsk(filter->ft_req);
		if(ireq){
			sport= ireq->loc_port;
                	saddr= ireq->loc_addr;
		}
		else{
			printk("%s, ERROR impossible to retrive inet socket\n",__func__);
			return;
		}
	}
	else{
		sport= inet->inet_sport;
		saddr= inet->inet_saddr;	
	}

	res= get_iphdr(skb, &network_header, &iphdrlen);
	if(res){
		return;
	}

	msg_changed= 0;

	/* saddr is the local IP
	 * watch out, saddr=0 means any address so do not change it
	 * in the packet.
	 */
	if(saddr && network_header->daddr != saddr){
		network_header->daddr= saddr;
		msg_changed= 1;
	}

	if (network_header->protocol == IPPROTO_UDP){
		udp_header= (struct udphdr *) ((char*)network_header+ network_header->ihl*4);
		datalen= skb->len - ip_hdrlen(skb);
		
		if(udp_header->dest != sport){
			udp_header->dest= sport;
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

		if(tcp_header->dest != sport){     
                        tcp_header->dest= sport; 
                        msg_changed= 1 ;
                }

		if(msg_changed){
			 tcp_v4_send_check(filter->ft_sock, skb);
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
	unsigned long timeout;

#if FT_FILTER_VERBOSE
	//char* filter_id_printed;
#endif
	char* filter_id_printed;
again:	spin_lock(&filter->lock);
	if(filter->type & FT_FILTER_ENABLE){

	        //TODO: I should save a copy on a list
		// beacuse maybe the hot replica died before sending this pckt to other 
		// kernels => save a copy to give to them.
		if(msg->pckt_id != filter->hot_rx+1){
			printk("%s: ERROR out of order delivery pckt id %llu hot_rx %llu \n", __func__, msg->pckt_id, filter->hot_rx);
			goto out_err;
		}
		
		filter->hot_rx= msg->pckt_id;
		filter_id_printed= print_filter_id(filter);
        	if(filter_id_printed)
                	kfree(filter_id_printed);

		//FTPRINTK("%s: pid %d is going to wait for delivering packet %llu\n\n", __func__, current->pid, msg->pckt_id);

		/* Wait to be aligned with the hot replica for the delivery of the packet.
		 * => wait to reach the same number of sent pckts.
		 */
		while( (filter->type & FT_FILTER_FAKE) || (filter->local_tx < msg->local_tx)){
			timeout = msecs_to_jiffies(WAIT_CROSS_FILTER_MAX) + 1;
			where_to_wait= filter->wait_queue;
			spin_unlock(&filter->lock);

			wait_event_timeout(*where_to_wait, !(filter->type & FT_FILTER_ENABLE) || ( !(filter->type & FT_FILTER_FAKE) && (filter->local_tx >= msg->local_tx)), timeout);

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

                		filter= find_and_get_filter(&msg->creator, msg->filter_id, msg->is_child, msg->daddr, msg->dport);;
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

		filter= find_and_get_filter(&msg->creator, msg->filter_id, msg->is_child, msg->daddr, msg->dport);
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

#if FT_FILTER_VERBOSE
	filter_id_printed= print_filter_id(filter);
	FTPRINTK("%s: pid %d is going to deliver the packet %llu in filter %s\n\n", __func__, current->pid, msg->pckt_id, filter_id_printed);
	if(filter_id_printed)
		kfree(filter_id_printed);
#endif
        
        printk("%s: pid %d is going to deliver the packet %llu in filter %s\n\n", __func__, current->pid, msg->pckt_id, filter_id_printed);
        if(filter_id_printed)
                kfree(filter_id_printed);

	/* the network stack rx path is thougth to be executed in softirq
	 * context...
	 */
	
	local_bh_disable();	
	netif_receive_skb(skb);
	local_bh_enable();

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
	struct workqueue_struct *rx_copy_wq;
#if FT_FILTER_VERBOSE
        char* ft_pid_printed;
#endif

again:  filter= find_and_get_filter(&msg->creator, msg->filter_id, msg->is_child, msg->daddr, msg->dport);
        if(filter){
                spin_lock(&filter->lock);
                if(filter->type & FT_FILTER_ENABLE){
			rx_copy_wq= filter->rx_copy_wq;
			spin_unlock(&filter->lock);	
		
			work= kmalloc(sizeof(*work), GFP_ATOMIC);
		        if(!work){
                		ret= -ENOMEM;
                		goto out_err;
        		}

        		INIT_WORK( (struct work_struct*)work, process_rx_copy_msg);
        		work->data= inc_msg;
			work->filter= filter;
        		queue_work(rx_copy_wq, (struct work_struct*)work);
			
                }
                else{

			if(!(filter->type & FT_FILTER_FAKE)){
	                	spin_unlock(&filter->lock);
			        printk("%s: ERROR filter is disable but not fake\n",__func__);
				ret= -EFAULT;
				goto out_err;
			}

                        spin_unlock(&filter->lock);
			put_ft_filter(filter);
                        goto again;

                }

        }
        else{
#if FT_FILTER_VERBOSE
                ft_pid_printed= print_ft_pid(&msg->creator);
                FTPRINTK("%s: creating fake filter ft_pid %s id %d child %i\n\n", __func__, ft_pid_printed, msg->filter_id, msg->is_child);
                if(ft_pid_printed)
                        kfree(ft_pid_printed);
#endif

                ret= create_fake_filter(&msg->creator, msg->filter_id, msg->is_child, msg->daddr, msg->dport);
                if(!ret)
                        goto again;
        }

out:
	return ret;
out_err:
	put_ft_filter(filter);
	pcn_kmsg_free_msg(msg);
	goto out;
}

/*
 * For coping skb check net/core/skb.c 
 */
static int create_rx_skb_copy_msg(struct net_filter_info *filter, long long pckt_id, long long local_tx, struct sk_buff *skb, struct rx_copy_msg **msg, int *msg_size){
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

	message->creator= filter->creator;
	message->filter_id= filter->id;
	message->is_child= filter->type & FT_FILTER_CHILD;
        if(message->is_child){
                message->daddr= filter->tcp_param.daddr;
                message->dport= filter->tcp_param.dport;
        }
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

static void send_skb_copy(struct net_filter_info *filter, long long pckt_id, long long local_tx, struct sk_buff *skb){
        struct rx_copy_msg* msg;
        int msg_size;
        int ret;
        struct list_head *iter= NULL;
        struct replica_id cold_replica;
        struct replica_id_list* objPtr;

        ret= create_rx_skb_copy_msg(filter, pckt_id, local_tx, skb, &msg, &msg_size);
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
	long long pckt_id;
	long long local_tx;
#if FT_FILTER_VERBOSE
	char* filter_id_printed;
#endif

        spin_lock(&filter->lock);
 	pckt_id= ++filter->local_rx;
	local_tx= filter->local_tx;

#if FT_FILTER_VERBOSE
        filter_id_printed= print_filter_id(filter);
        FTPRINTK("%s: pid %d broadcasting packet %llu in filter %s\n\n", __func__, current->pid, pckt_id, filter_id_printed);
        if(filter_id_printed)
                kfree(filter_id_printed);
#endif
	/*char* filter_id_printed;
	filter_id_printed= print_filter_id(filter);
        printk("%s: pid %d broadcasting packet %llu in filter %s\n\n", __func__, current->pid, pckt_id, filter_id_printed);
        if(filter_id_printed)
                kfree(filter_id_printed);*/

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
	long long pckt_id;
	long long hot_rx;
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

static void update_tcp_init_param(struct net_filter_info *filter, struct tcp_init_param *tcp_param){

        filter->tcp_param= *tcp_param;

}

static int handle_tcp_init_param(struct pcn_kmsg_message* inc_msg){
	struct tcp_init_param_msg* msg= (struct tcp_init_param_msg*) inc_msg;
	struct net_filter_info *filter;
        int err= 0;
        int removing_fake= 0;
#if FT_FILTER_VERBOSE
        char* ft_pid_printed;
#endif

again:  filter= find_and_get_filter(&msg->creator, msg->filter_id, msg->is_child, msg->daddr, msg->dport);
        if(filter){

                spin_lock(&filter->lock);
                if(filter->type & FT_FILTER_ENABLE){
			if(msg->connect_id != -1){
				filter->hot_connect_id++;

			}
			else{
           			filter->hot_accept_id++;
			}
			update_tcp_init_param(filter, &msg->tcp_param);
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

                err= create_fake_filter(&msg->creator, msg->filter_id, msg->is_child, msg->daddr, msg->dport);
                if(!err)
                        goto again;
        }

        pcn_kmsg_free_msg(msg);
	return err;
}

static int create_tcp_init_param_msg(struct net_filter_info* filter, int connect_id, int accept_id, struct tcp_init_param* tcp_param, struct tcp_init_param_msg** msg, int* msg_leng ){
	struct tcp_init_param_msg* message;

	message= kmalloc(sizeof(*message), GFP_ATOMIC);
	if(!message)
		return -ENOMEM;

	message->header.type= PCN_KMSG_TYPE_FT_TCP_INIT_PARAM;
        message->header.prio= PCN_KMSG_PRIO_NORMAL;

	message->is_child= filter->type & FT_FILTER_CHILD;
        message->creator= filter->creator;
        message->filter_id= filter->id;
	message->daddr= filter->tcp_param.daddr;
	message->dport= filter->tcp_param.dport;
        message->connect_id= connect_id;
	message->accept_id= accept_id;
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

	ret= create_tcp_init_param_msg(filter, my_work->connect_id, my_work->accept_id, &my_work->tcp_param, &msg, &msg_size);
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

static void send_tcp_init_param_accept(struct net_filter_info* filter, struct request_sock *req){
	struct tcp_param_work* work;
	int accept;
	struct inet_request_sock *ireq;

	spin_lock(&filter->lock);
        accept= ++filter->local_accept_id;    
        spin_unlock(&filter->lock);
        
        work= kmalloc(sizeof(*work), GFP_ATOMIC);
        if(!work)
                return;

        INIT_WORK( (struct work_struct*)work, send_tcp_init_parameters_from_work);
        work->filter= filter;
        work->connect_id= -1;
	work->accept_id= accept;
	ireq = inet_rsk(req);
	work->tcp_param.saddr= ireq->loc_addr;
        work->tcp_param.sport= ireq->loc_port;
	work->tcp_param.daddr= ireq->rmt_addr;
	work->tcp_param.dport= ireq->rmt_port;
       	work->tcp_param.snt_isn= tcp_rsk(req)->snt_isn; 
        work->tcp_param.snt_synack= tcp_rsk(req)->snt_synack;
	get_ft_filter(filter);

        queue_work(tx_notify_wq, (struct work_struct*)work);

}

static void send_tcp_init_param_connect(struct net_filter_info* filter, struct sock* sk){
	struct inet_sock *inet = inet_sk(sk);
        struct tcp_sock *tp = tcp_sk(sk);
	struct tcp_param_work* work;
	int connect;

	spin_lock(&filter->lock);
	connect= ++filter->local_connect_id;	
	spin_unlock(&filter->lock);
	
	work= kmalloc(sizeof(*work), GFP_ATOMIC);
	if(!work)
		return;

	INIT_WORK( (struct work_struct*)work, send_tcp_init_parameters_from_work);
        work->filter= filter;
	work->accept_id= -1;
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

static void send_tcp_init_param(struct net_filter_info* filter, struct sock* sk, struct request_sock *req){
	
	if (req) {
		send_tcp_init_param_accept(filter, req);
		return;
	}

	if (sk){
		send_tcp_init_param_connect(filter, sk);
		return;
	}
}

void ft_change_tcp_init_connect(struct sock* sk){
	struct inet_sock *inet = inet_sk(sk);
	struct tcp_sock *tp = tcp_sk(sk);

	tp->write_seq= inet->inet_daddr + inet->inet_dport;
}

void ft_change_tcp_init_accept(struct request_sock *req){
        
	tcp_rsk(req)->snt_isn= inet_rsk(req)->rmt_addr + inet_rsk(req)->rmt_port;
        tcp_rsk(req)->snt_synack= tcp_rsk(req)->snt_isn*10;
}

/* Remove randomly generated sequence numbers to align hot/cold replicas.
 * If replica is HOT, also send init connection information, like real port
 * and address to COLD replicas.
 */
void ft_check_tcp_init_param(struct net_filter_info* filter, struct sock* sk, struct request_sock *req){

	if(filter){
		if(!req)
			ft_change_tcp_init_connect(sk);
		else
			ft_change_tcp_init_accept(req);
	
		if(filter->type & FT_FILTER_HOT_REPLICA){
			send_tcp_init_param(filter, sk, req);
		}
	}

}

void ft_activate_grown_filter(struct net_filter_info* filter){
	if(filter){
		spin_lock(&filter->lock);
		filter->local_tx++;
		spin_unlock(&filter->lock);
	}
}

/* Note: call put_iphdr after using get_iphdr in case 
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

	/*if(skb_shared(skb))
		printk("%s: WARNING skb shared\n", __func__);*/

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
	int iphdrlen;
	struct iphdr *network_header;  // ip header struct
	struct tcphdr *tcp_header= NULL;     // tcp header struct
	struct udphdr *udp_header= NULL;     // udp header struct 
	struct sock* sk;

	type= skb->protocol;

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
				
			ret= sk->ft_filter;

			sock_put(sk);
			

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
	if(sk->ft_filter){
		if(sk->ft_filter->type & FT_FILTER_COLD_REPLICA){
			return 0;
		}

	}
	
	return 1;
}

static int __init ft_filter_init(void){

	spin_lock_init(&wq_stack.stack_lock);
	INIT_LIST_HEAD(&wq_stack.stack_head);
	wq_stack.size= 0;
	create_more_working_queues(NULL);
	workqueues_creator_wq= create_workqueue("workqueues_creator_wq");

	INIT_LIST_HEAD(&filter_list_head.list_member);

	tx_notify_wq= create_singlethread_workqueue("tx_notify_wq");
	
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_TX_NOTIFY, handle_tx_notify);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_RX_COPY, handle_rx_copy);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_TCP_INIT_PARAM, handle_tcp_init_param);

	print_log_buf_info();

	printk("%s:  TCP_DELACK_MAX %d\n", __func__, TCP_DELACK_MAX);
	printk("%s:  TCP_DELACK_MIN %d\n", __func__, TCP_DELACK_MIN);
	printk("%s:  TCP_TIMEOUT_INIT %d\n", __func__, TCP_TIMEOUT_INIT);
	printk("%s:  TCP_SYNQ_INTERVAL %d\n",__func__, TCP_SYNQ_INTERVAL);
	return 0;
}

late_initcall(ft_filter_init);
