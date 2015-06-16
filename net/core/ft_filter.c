/* pckt_id
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
#include <linux/net.h>

struct cold_replica_answer{
        struct pcn_kmsg_hdr header;
	struct ft_pid creator;
	int filter_id;
        int pckt_id;
};
static int create_tx_notify_msg(int filter_id, int pckt_id, struct ft_pid* creator, struct cold_replica_answer** msg, int* msg_size);

struct tx_notify_work{
        struct work_struct work;
        struct net_filter_info *filter;
	int pckt_id;
};

static struct workqueue_struct *tx_notify_wq;
extern int _cpu;
struct net_filter_info filter_list_head;
DEFINE_SPINLOCK(filter_list_lock);

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

		fake_filter->type= FT_FILTER_DISABLE;

		spin_unlock(&fake_filter->lock);
	}

	list_add(&filter->list_member,&filter_list_head.list_member);
	spin_unlock(&filter_list_lock);
        
	if(fake_filter)
		put_ft_filter(fake_filter);
	
	return ;

}

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


static int create_fake_filter(struct ft_pid *creator, int filter_id){
	struct net_filter_info* filter;
		
	filter= kmalloc(sizeof(*filter),GFP_ATOMIC);
	if(!filter)
		return -ENOMEM;

	INIT_LIST_HEAD(&filter->list_member);
	atomic_set(&filter->kref.refcount,1);
	filter->creator= *creator;
	filter->ft_popcorn= NULL;
	spin_lock_init(&filter->lock);
		
	filter->type= FT_FILTER_ENABLE;
	filter->id= filter_id;

        filter->local_tx= 0;
        filter->hot_tx= 0;
        filter->local_rx= 0;
        filter->hot_rx= 0;

	add_filter_with_check(filter);

	return 0;
}

int create_filter_accept(struct socket *newsock,struct socket *sock){
	int ret= 0;
	
	if(newsock->filter)
		put_ft_filter(newsock->filter);

	if(sock->filter_type == FT_FILTER_ENABLE){
		newsock->filter_type= sock->filter_type;
		get_ft_filter(sock->filter);
		newsock->filter= sock->filter;
	}
	else{
		newsock->filter_type= FT_FILTER_DISABLE;
                newsock->filter= NULL;
	}

	return ret;
}

int create_filter(struct task_struct *task, struct socket *sock){
	struct net_filter_info* filter;
	struct task_struct *ancestor;

	if(task->replica_type == HOT_REPLICA 
		|| task->replica_type == COLD_REPLICA
		|| task->replica_type == NEW_HOT_REPLICA_DESCENDANT
		|| task->replica_type == NEW_COLD_REPLICA_DESCENDANT
		|| task->replica_type == REPLICA_DESCENDANT){
		
		filter= kmalloc(sizeof(*filter),GFP_KERNEL);
		if(!filter)
			return -ENOMEM;

		INIT_LIST_HEAD(&filter->list_member);
		atomic_set(&filter->kref.refcount, 1);
		filter->creator= task->ft_pid;
		get_ft_pop_rep(task->ft_popcorn);
		filter->ft_popcorn= task->ft_popcorn;
		spin_lock_init(&filter->lock);
		init_waitqueue_head(&filter->wait_queue);
		
		/* NOTE: target applications are deterministic, so all replicas will do the same actions 
                 * on the same order.
                 * Because of this, all replicas will and up creating this socket, and giving it the same id.
                 */

		filter->id= task->next_id_resources++;

                filter->local_tx= 0;
                filter->hot_tx= 0;
                filter->local_rx= 0;
                filter->hot_rx= 0;

			
		ancestor= find_task_by_vpid(task->tgid);

		if(ancestor->replica_type == HOT_REPLICA || ancestor->replica_type == NEW_HOT_REPLICA_DESCENDANT){
			filter->type= FT_FILTER_HOT_REPLICA;
			add_filter(filter);
		}
		else{
			if(ancestor->replica_type == COLD_REPLICA || ancestor->replica_type == NEW_COLD_REPLICA_DESCENDANT){
				filter->type= FT_FILTER_COLD_REPLICA;
				/*maybe the hot replica alredy sent me some notifications or msg*/
				add_filter_coping_pending(filter);
			}
			else{
				//BUG();
				printk("%s: ERROR ancestor pid %d has replica type %d \n",__func__, ancestor->pid, ancestor->replica_type);
				kfree(filter);
				return -1;
			}
			
		}		

		sock->filter_type= FT_FILTER_ENABLE;
		sock->filter= filter;
		
	}else{
		sock->filter_type= FT_FILTER_DISABLE;
		sock->filter= NULL;
	}	

	return 0;	
}

static int handle_tx_notify(struct pcn_kmsg_message* inc_msg){
	struct cold_replica_answer *msg= (struct cold_replica_answer *) inc_msg;
	struct net_filter_info *filter;
	int err;
	int removing_fake= 0;
	int queue_active= 1;


again:	filter= find_and_get_filter(&msg->creator, msg->filter_id);
	if(filter){
		spin_lock(&filter->lock);
		if(filter->type != FT_FILTER_DISABLE){
			
			if(filter->hot_tx < msg->pckt_id)
				filter->hot_tx= msg->pckt_id;
			
			if(filter->type == FT_FILTER_ENABLE)	
				queue_active= 0;
		}
		else{
			removing_fake= 1;
		}
        	spin_unlock(&filter->lock);
		
		if(queue_active)
			wake_up(&filter->wait_queue);

		put_ft_filter(filter);

		if(removing_fake){
			removing_fake= 0;
			queue_active= 1;
			goto again;
		}
	}
	else{
		err= create_fake_filter(&msg->creator, msg->filter_id);
		if(!err)
			goto again;
	}
	
	kfree(msg);
	
	return 0;
}

static int create_tx_notify_msg(int filter_id, int pckt_id, struct ft_pid* creator, struct cold_replica_answer** msg, int* msg_size){
	struct cold_replica_answer* message;
	
	message= kmalloc(sizeof(*message), GFP_KERNEL);
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
	struct cold_replica_answer* msg;
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

static int tx_filter_hot(struct net_filter_info *filter){
        unsigned long long pckt_id;
	int ret= 0;
	struct tx_notify_work *work;

        spin_lock(&filter->lock);

        pckt_id= ++filter->local_tx;

        spin_unlock(&filter->lock);


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

out:        return ret;

}


static int tx_filter_cold(struct net_filter_info *filter){
	unsigned long long pckt_id;

	spin_lock(&filter->lock);
	
	pckt_id= ++filter->local_tx;
	
	spin_unlock(&filter->lock);

	while (filter->hot_tx < pckt_id){
		
		DEFINE_WAIT(wait);
		prepare_to_wait(&filter->wait_queue, &wait, TASK_UNINTERRUPTIBLE);
		
		if (filter->hot_tx < pckt_id) {
			schedule();
		}

		finish_wait(&filter->wait_queue, &wait);

	}

	return 1;

}

int net_ft_tx_filter(struct socket* socket){
	int ret= 0;
	struct net_filter_info *filter;
	
	if(!socket){
		//BUG();
		printk("%s: ERROR socket is null\n",__func__);
		goto out;
	}

	if(socket->filter_type == FT_FILTER_DISABLE){
		goto out;
	}

	filter= socket->filter;

	if(!filter){
        	//BUG();
                printk("%s: ERROR filter is null\n",__func__);
                goto out;
	}

	if(filter->type == FT_FILTER_COLD_REPLICA){
		return tx_filter_cold(filter);
	}

	if(filter->type == FT_FILTER_HOT_REPLICA){
		return tx_filter_hot(filter);
	}	

out:	return ret;
}

static int __init ft_filter_init(void){

	tx_notify_wq= create_singlethread_workqueue("tx_notify_wq");

	INIT_LIST_HEAD(&filter_list_head.list_member);	
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_TX_NOTIFY, handle_tx_notify);

	return 0;
}

late_initcall(ft_filter_init);
