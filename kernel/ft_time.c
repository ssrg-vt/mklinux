/*
 * ft_time.c
 *
 * Author: Marina
 */

#include <linux/ft_replication.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/pcn_kmsg.h>
#include <asm/uaccess.h>
#include <linux/time.h>
#include <linux/sched.h>

#define FT_TIME_VERBOSE 0
#if FT_TIME_VERBOSE
#define FTPRINTK(...) printk(__VA_ARGS__)
#else
#define FTPRINTK(...) ;
#endif

typedef struct _list_entry{
	struct list_head list;
	char *string;
	void *obj;
}list_entry_t;

typedef struct _hash_table{
	int size;
	spinlock_t spinlock;
	list_entry_t **table;	
}hash_table_t;

hash_table_t* create_hashtable(int size){
	hash_table_t *ret;
	list_entry_t **table;
	int i;

	if(size<1)
		return ERR_PTR(-EFAULT);

	ret= kmalloc(sizeof(*ret), GFP_KERNEL);
	if(!ret)
		return ERR_PTR(-ENOMEM);
	
	table= kmalloc(sizeof(*table)*size, GFP_KERNEL);
	if(!table){
		kfree(ret);
		return ERR_PTR(-ENOMEM);
	}

	for(i=0;i<size;i++){
		table[i]= NULL;
	}

	ret->size= size;
	ret->table= table;
	spin_lock_init(&ret->spinlock);
	
	return ret;	
}

unsigned int hash(hash_table_t *hashtable, char *str)
{
    unsigned int hashval;
    
    /* we start our hash out at 0 */
    hashval = 0;

    /* for each character, we multiply the old hash by 31 and add the current
     * character.  Remember that shifting a number left is equivalent to 
     * multiplying it by 2 raised to the number of places shifted.  So we 
     * are in effect multiplying hashval by 32 and then subtracting hashval.  
     * Why do we do this?  Because shifting and subtraction are much more 
     * efficient operations than multiplication.
     */
    for(; *str != '\0'; str++) hashval = *str + (hashval << 5) - hashval;

    /* we then return the hash value mod the hashtable size so that it will
     * fit into the necessary range
     */
    return hashval % hashtable->size;
}

void* hash_lookup(hash_table_t *hashtable, char *str){
	unsigned int hashval;
	list_entry_t *head, *entry;
	void* obj= NULL;
	
	hashval= hash(hashtable, str);
	spin_lock(&hashtable->spinlock);

	head= hashtable->table[hashval];
        if(head){
                list_for_each_entry(entry, &head->list, list){
                        if((strcmp(entry->string,str)==0)){
                                obj= entry->obj;
                                goto out;
                        }

                }
        }

out:	spin_unlock(&hashtable->spinlock);
	return obj;
}

void* hash_add(hash_table_t *hashtable, char *str, void* obj){
	unsigned int hashval;
        void* entry= NULL;
	list_entry_t *new, *head, *app;

	new= kmalloc(sizeof(list_entry_t), GFP_ATOMIC);
        if(!new)
                return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&new->list);
        new->string= str;
	new->obj= obj;

	hashval= hash(hashtable, str);

	spin_lock(&hashtable->spinlock);

	head= hashtable->table[hashval];
        
	if(head){
		list_for_each_entry(app, &head->list, list){
			if((strcmp(app->string, str)==0)){
                                entry= app->obj;
				spin_unlock(&hashtable->spinlock);
				kfree(new);
                                return entry;
                        }

                }
        }
	else{
		hashtable->table[hashval]= kmalloc(sizeof(list_entry_t), GFP_ATOMIC);
	        if(!hashtable->table[hashval]){
        		spin_unlock(&hashtable->spinlock);
		 	kfree(new); 
		       	return ERR_PTR(-ENOMEM);
		}
		head= hashtable->table[hashval];
		INIT_LIST_HEAD(&head->list);
	}

	list_add(&new->list, &head->list);

	spin_unlock(&hashtable->spinlock);
        
	return NULL;
}

void* hash_remove(hash_table_t *hashtable, char *str){
        unsigned int hashval;
        list_entry_t *head, *app;
	list_entry_t *entry= NULL;
	void *obj= NULL;

	hashval= hash(hashtable, str);

	spin_lock(&hashtable->spinlock);
        head= hashtable->table[hashval];
	if(head){
		list_for_each_entry(app, &head->list, list){
			if((strcmp(app->string,str)==0)){
				entry= app;
				list_del(&app->list);
				goto out;
			}
				
		}
	}
out:
	spin_unlock(&hashtable->spinlock);
	if(entry){
		obj= entry->obj;
		kfree(entry->string);
		kfree(entry);
	}

	return obj;
}



hash_table_t* gettimeofday_hash;
static struct workqueue_struct *ft_time_wq;

struct get_time_info{
	struct timeval tv;
        struct timezone tz;
};

struct wait_time_info{
	struct get_time_info info;
	struct task_struct *task;
	int populated;
};

struct gettime_work{
	struct work_struct work;
	int get_time_id;
	struct ft_pop_rep *ft_popcorn;
	struct get_time_info info;
	 /*the following is pid_t linearized*/
        struct ft_pop_rep_id ft_pop_id;
        int level;
        /*this must be the last field of the struct*/
        int* id_array;

};

struct gettime_msg{
	struct pcn_kmsg_hdr header;
	struct get_time_info info;
	int get_time_id;
	/*the following is pid_t linearized*/
	struct ft_pop_rep_id ft_pop_id;
	int level;
	/*this must be the last field of the struct*/
	int* id_array;	
};

static int create_gettime_msg(struct get_time_info *info, int level, struct ft_pop_rep_id* ft_pop_id, int* id_array, int id, struct gettime_msg **message, int* msg_size){
	struct gettime_msg* msg;
	int size;
	
	size= sizeof(*msg) + level*sizeof(int);
	msg= kmalloc(size, GFP_KERNEL);
	if(!msg)
		return -ENOMEM;
		
	msg->info= *info;
	msg->get_time_id= id;
	
	msg->ft_pop_id= *ft_pop_id;
	msg->level= level;
	memcpy(&msg->id_array, id_array, level*sizeof(int));
	msg->header.type= PCN_KMSG_TYPE_FT_GETTIMEOFDAY;
        msg->header.prio= PCN_KMSG_PRIO_NORMAL;

        *message= msg;
        *msg_size= size;

	return 0;
}

static void send_gettimeofday_to_cold_replicas( struct get_time_info *info, struct ft_pop_rep *ft_popcorn, int level, struct ft_pop_rep_id* ft_pop_id, int* id_array, int id){
	struct gettime_msg* msg;
	int msg_size;
	int ret;

	FTPRINTK("%s sending data\n", __func__);

	ret= create_gettime_msg(info, level, ft_pop_id, id_array, id, &msg, &msg_size);
	if(ret)
		return;

	send_to_all_cold_replicas(ft_popcorn, (struct pcn_kmsg_long_message*) msg, msg_size);

	kfree(msg);
}

static void send_gettimeofday_to_cold_replicas_from_work(struct work_struct* work){
	struct gettime_work* my_work= (struct gettime_work*) work;

	send_gettimeofday_to_cold_replicas(&my_work->info, my_work->ft_popcorn, my_work->level, &my_work->ft_pop_id, (int*) &my_work->id_array, my_work->get_time_id);

	put_ft_pop_rep(my_work->ft_popcorn);
	kfree(work);

}

static long ft_gettimeofday_hot(struct timeval __user * tv, struct timezone __user * tz){
	long ret= 0;
	struct timeval *ktv= NULL;
	struct timezone *ktz= NULL;
	struct gettime_work *my_work;

	printk("%s called form pid %d\n", __func__, current->pid);
	
	if (likely(tv != NULL)) {
                ktv= kmalloc(sizeof(*ktv), GFP_KERNEL);
		if(!ktv){
			ret= -ENOMEM;
			goto out;
		}
                do_gettimeofday(ktv);
                if (copy_to_user(tv, ktv, sizeof(*ktv))){
                        ret= -EFAULT;
			goto out;
		}
        }
        if (unlikely(tz != NULL)) {
		ktz= kmalloc(sizeof(*ktz), GFP_KERNEL);
		if(!ktz){
			ret= -ENOMEM;
			goto out;
		}
		memcpy(ktz, &sys_tz, sizeof(*ktz));
                if (copy_to_user(tz, ktz, sizeof(*ktz))){
                        ret= -EFAULT;
                        goto out;
                }
        }

	my_work= kmalloc(sizeof(*my_work)+ current->ft_pid.level*sizeof(int), GFP_KERNEL);
        if(!my_work){
        	ret= -ENOMEM;
		goto out;
        }

	if(ktv)
		my_work->info.tv= *ktv;
	if(ktz)
		my_work->info.tz= *ktz;

	my_work->get_time_id= current->next_id_kernel_requests++;
		
        my_work->ft_pop_id= current->ft_pid.ft_pop_id;
        my_work->level= current->ft_pid.level;
        if(current->ft_pid.level)
		memcpy(&my_work->id_array, current->ft_pid.id_array, current->ft_pid.level*sizeof(int));


	my_work->ft_popcorn= current->ft_popcorn;
	get_ft_pop_rep(current->ft_popcorn);
	
	INIT_WORK( (struct work_struct*)my_work, send_gettimeofday_to_cold_replicas_from_work);

	queue_work(ft_time_wq, (struct work_struct*)my_work);

out:	
	if(ktv)
        	kfree(ktv);
	if(ktz)
                kfree(ktz);                
        return ret;


}

static char* get_bare_string_ft_pid(struct ft_pop_rep_id* ft_pop_id, int level, int* id_array, int get_time_id){
	char* string;
	const int size= 1024;
	int pos,i;

	string= kmalloc(size, GFP_KERNEL);
	if(!string)
		return NULL;

	pos= snprintf(string, size,"%d%d%d", ft_pop_id->kernel, ft_pop_id->id, level);
        if(pos>=size)
                goto out_clean;

        if(level){
                for(i=0;i<level;i++){
                        pos= pos+ snprintf(&string[pos], size-pos, "%d", id_array[i]);
                        if(pos>=size)
                                goto out_clean;
                }
        }

	pos= snprintf(&string[pos], size-pos,"%d%c", get_time_id,'\0');
        if(pos>=size)
                goto out_clean;


//	FTPRINTK("%s string is %s\n", __func__, string);

        return string;

out_clean:
        kfree(string);
        printk("%s: buffer size too small\n", __func__);
        return NULL;

}

static long ft_gettimeofday_cold(struct timeval __user * tv, struct timezone __user * tz){
        struct wait_time_info* wait_info;
        struct wait_time_info* present_info= NULL;
        char* key;
	long ret= 0;
	int free_key= 0;

        FTPRINTK("%s called from pid %d\n", __func__, current->pid);

	key= get_bare_string_ft_pid(&current->ft_pid.ft_pop_id, current->ft_pid.level, current->ft_pid.id_array, current->next_id_kernel_requests++);
        if(!key)
                return -ENOMEM;

        wait_info= kmalloc(sizeof(*wait_info), GFP_ATOMIC);
        if(!wait_info)
                return -ENOMEM;

        wait_info->task= current;
        wait_info->populated= 0;

        if((present_info= ((struct wait_time_info*) hash_add(gettimeofday_hash, key, (void*) wait_info)))){
                kfree(wait_info);
		free_key= 1;
		goto copy;
        }
	else{
		FTPRINTK("%s going to sleep pid %d\n", __func__, current->pid);

		present_info= wait_info;
		while(present_info->populated==0){
			set_current_state(TASK_UNINTERRUPTIBLE);
			if(present_info->populated==0);
				schedule();
			set_current_state(TASK_RUNNING);
		}		

	}

	FTPRINTK("%s going to copy data\n", __func__);

copy: 	if(present_info->populated != 1){
		printk("%s ERROR, data in cash but not populated\n", __func__);
		ret= -EFAULT;
		goto out;
	}

	if (likely(tv != NULL)) {
		if (copy_to_user(tv, (void*) &present_info->info.tv, sizeof(tv))){
			ret= -EFAULT;
			goto out;
		}
	}

	if (unlikely(tz != NULL)) {
		if (copy_to_user(tz, (void*) &present_info->info.tz, sizeof(sys_tz))){
			ret= -EFAULT;
			goto out;
		}
	}


out:	hash_remove(gettimeofday_hash, key);
	if(free_key)
		kfree(key);	
	kfree(present_info);

        return ret;

}

long ft_gettimeofday(struct timeval __user * tv, struct timezone __user * tz){
	struct task_struct *ancestor;

	//FTPRINTK("%s called from pid %d\n", __func__, current->pid);

	ancestor= find_task_by_vpid(current->tgid);

	if(ancestor->replica_type == HOT_REPLICA || ancestor->replica_type == NEW_HOT_REPLICA_DESCENDANT){
		return ft_gettimeofday_hot(tv, tz);	
	}
	else{
		if(ancestor->replica_type == COLD_REPLICA || ancestor->replica_type == NEW_COLD_REPLICA_DESCENDANT){
			return ft_gettimeofday_cold(tv, tz);
		}
		else{
			//BUG();
			printk("%s: ERROR ancestor pid %d tgid %d has replica type %d (current pid %d tgid %d)\n", __func__, ancestor->pid, ancestor->tgid, ancestor->replica_type, current->pid, current->tgid);
			return -EFAULT;
		}

	}

        return 0;

}

static int handle_gettimeofday_msg(struct pcn_kmsg_message* inc_msg){
	struct gettime_msg* msg = (struct gettime_msg*) inc_msg;
	struct wait_time_info* wait_info;
	struct wait_time_info* present_info= NULL;
	char* key;

	//FTPRINTK("%s received data\n", __func__);

	key= get_bare_string_ft_pid(&msg->ft_pop_id, msg->level,(int*) &msg->id_array, msg->get_time_id);
	if(!key)
		return -ENOMEM;

	wait_info= kmalloc(sizeof(*wait_info), GFP_ATOMIC);
	if(!wait_info)
		return -ENOMEM;
	
	wait_info->info= msg->info;
	wait_info->task= NULL;
	wait_info->populated=1;
	
	if((present_info= ((struct wait_time_info*) hash_add(gettimeofday_hash, key, (void*) wait_info)))){
		kfree(key);
		kfree(wait_info);

		//FTPRINTK("%s waking up sleeper\n", __func__);
		present_info->info.tv= msg->info.tv;
		present_info->info.tz= msg->info.tz;
		present_info->populated= 1;
		wake_up_process(present_info->task);
	}

	pcn_kmsg_free_msg(msg);

	return 0;
		
}

static int __init ft_time_init(void) {
	ft_time_wq= create_singlethread_workqueue("ft_time_wq");
	gettimeofday_hash= create_hashtable(50);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_GETTIMEOFDAY, handle_gettimeofday_msg);
	
	return 0;
}

late_initcall(ft_time_init);

