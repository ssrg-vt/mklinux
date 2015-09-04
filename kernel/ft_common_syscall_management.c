/*
 * ft_common_syscall_management.c
 *
 * Author: Marina
 * 
 */

#include <linux/ft_replication.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/pcn_kmsg.h>

#define FT_CSYSC_VERBOSE 0
#if FT_CSYSC_VERBOSE
#define FTPRINTK(...) printk(__VA_ARGS__)
#else
#define FTPRINTK(...) ;
#endif

typedef struct _list_entry{
        struct list_head list;
        char *string; //key
        void *obj; //pointer to the object to store
}list_entry_t;

typedef struct _hash_table{
        int size;
        spinlock_t spinlock; //used to lock the whole hash table when adding/removing/looking, not fine grain but effective!
        list_entry_t **table;
}hash_table_t;

/* Create an hash table with size @size.
 *
 */
static hash_table_t* create_hashtable(int size){
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

static unsigned int hash(hash_table_t *hashtable, char *str)
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

/* Return the object stored in @hashtable in the entry with key @key  
 * if any, NULL otherwise.
 */
static void* hash_lookup(hash_table_t *hashtable, char *key){
        unsigned int hashval;
        list_entry_t *head, *entry;
        void* obj= NULL;

        hashval= hash(hashtable, key);
        spin_lock(&hashtable->spinlock);

        head= hashtable->table[hashval];
        if(head){
                list_for_each_entry(entry, &head->list, list){
                        if((strcmp(entry->string,key)==0)){
                                obj= entry->obj;
                                goto out;
                        }

                }
        }

out:    spin_unlock(&hashtable->spinlock);
        return obj;
}

/* Add a new object in @hashtable with key @key and object @obj.
 * 
 * If an entry with the same key is already present, the object of that entry 
 * is returned and the one passed as paramenter is NOT inserted ( => remember to free both @key and @obj) 
 *
 * If no entry with the same key are found, NULL is returned and the entry inserted will use both @key and @obj
 * pointers so do not free them while not removed from the hashtable.
 */
static void* hash_add(hash_table_t *hashtable, char *key, void* obj){
        unsigned int hashval;
        void* entry= NULL;
        list_entry_t *new, *head, *app;

        new= kmalloc(sizeof(list_entry_t), GFP_ATOMIC);
        if(!new)
                return ERR_PTR(-ENOMEM);

        INIT_LIST_HEAD(&new->list);
        new->string= key;
        new->obj= obj;

        hashval= hash(hashtable, key);

        spin_lock(&hashtable->spinlock);

        head= hashtable->table[hashval];

        if(head){
                list_for_each_entry(app, &head->list, list){
                        if((strcmp(app->string, key)==0)){
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

/* Remove an entry from the hash table @hashtable with key @key.
 *
 * If a corresponding entry to @key is found, the object stored by that entry 
 * is returned, NULL otherwise. 
 *
 * NOTE: remember to free @key and the object returned eventually.
 */
static void* hash_remove(hash_table_t *hashtable, char *key){
        unsigned int hashval;
        list_entry_t *head, *app;
        list_entry_t *entry= NULL;
        void *obj= NULL;

        hashval= hash(hashtable, key);

        spin_lock(&hashtable->spinlock);
        head= hashtable->table[hashval];
        if(head){
                list_for_each_entry(app, &head->list, list){
                        if((strcmp(app->string, key)==0)){
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

/* syscall_hash is an hash table used to store info about syscalls that need to be synchronized between replicas.
 *
 * The inital idea is that the primary replica performs the syscall and sends meaningfull info for that syscall to the secondary replicas.
 * Those info can be stored in the hash table while the secondary replica reaches the same syscall, or the secondary replica can create an "empty"
 * entry in the hash and sleeps while the primary send the info over.
 * 
 * The key of the hash table should be computed with get_key from ft_pid entries and id_syscall of the thread.
 * The object stored is a void* that can be used differently by each syscall.
 */
hash_table_t* syscall_hash;

/* Remove an entry from the syscall_hash  with key @key.
 *
 * If a corresponding entry to @key is found, the object stored by that entry 
 * is returned, NULL otherwise. 
 *
 * NOTE: remember to free @key and the object returned eventually.
 */
void* ft_syscall_hash_remove(char *key){
	return hash_remove(syscall_hash, key);
}

/* Add a new object in sycall_hash with key @key and object @obj.
 * 
 * If an entry with the same key is already present, the object of that entry 
 * is returned and the one passed as paramenter is NOT inserted ( => remember to free both @key and @obj) 
 *
 * If no entry with the same key are found, NULL is returned and the entry inserted will use both @key and @obj
 * pointers so do not free them while not removed from the hashtable.
 */
void* ft_syscall_hash_add(char *key, void* obj){
	return hash_add(syscall_hash, key, obj);
}

/* Return the object stored in syscall_hash in the entry with key @key  
 * if any, NULL otherwise.
 */
void* ft_syscall_hash_lookup(char *key){
	return hash_lookup(syscall_hash, key);
}

/* Return a string that is the concatenation of ft_pop_id fields, level, id_array and id_syscall.
 * This uniquely identify each syscall for each ft_pid replica.
 *
 */
char* ft_syscall_get_key(struct ft_pop_rep_id* ft_pop_id, int level, int* id_array, int id_syscall){
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

        pos= snprintf(&string[pos], size-pos,"%d%c", id_syscall,'\0');
        if(pos>=size)
                goto out_clean;

        return string;

out_clean:
        kfree(string);
        printk("%s: buffer size too small\n", __func__);
        return NULL;

}

/* Return a string that is the concatenation of ft_pop_id fields, level, id_array and id_syscall.
 * This uniquely identify each syscall for each ft_pid replica.
 *
 */
char* ft_syscall_get_key_from_ft_pid(struct ft_pid *ft_pid, int id_syscall){
	return ft_syscall_get_key(&ft_pid->ft_pop_id, ft_pid->level, ft_pid->id_array, id_syscall);
}


static struct workqueue_struct *ft_syscall_info_wq;

struct wait_syscall{
        struct task_struct *task;
        int populated;
	void *private;
};

struct send_syscall_work{
        struct work_struct work;
        struct ft_pop_rep *replica_group; //to know secondary replicas to whom send the msg
        /* ft_pid fields to id the replica*/
        struct ft_pop_rep_id ft_pop_id;
        int level;
	int* id_array;
	int syscall_id; //syscall id for that ft_pid replica
	unsigned int private_data_size; //size of the private data of the syscall
	char* private;
};

struct syscall_msg{
        struct pcn_kmsg_hdr header;
        /*the following is pid_t linearized*/
        struct ft_pop_rep_id ft_pop_id;
        int level;
	//int* id_array; this is size variable so included in data field
        int syscall_id;
	unsigned int syscall_info_size;
	/*this must be the last field of the struct*/
        char data; /*contains id_array followed by syscall_info*/
};

static int create_syscall_msg(struct ft_pop_rep_id* primary_ft_pop_id, int primary_level, int* primary_id_array, int syscall_id, char* syscall_info, unsigned int syscall_info_size, struct syscall_msg** message, int *msg_size){

	struct syscall_msg* msg;
        int size;
	char* variable_data;

        size= sizeof(*msg) + primary_level*sizeof(int) + syscall_info_size;
        msg= kmalloc(size, GFP_KERNEL);
        if(!msg)
                return -ENOMEM;

	msg->header.type= PCN_KMSG_TYPE_FT_SYSCALL_INFO;
        msg->header.prio= PCN_KMSG_PRIO_NORMAL;

        msg->ft_pop_id= *primary_ft_pop_id;
        msg->level= primary_level;
	
	msg->syscall_id= syscall_id;
	msg->syscall_info_size= syscall_info_size;

	variable_data= &msg->data;
	if(primary_level){
		memcpy(variable_data, primary_id_array, primary_level*sizeof(int));
		variable_data+= primary_level*sizeof(int);
	}
	
	if(syscall_info_size){
		memcpy(variable_data, syscall_info, syscall_info_size);
	}

        *message= msg;
        *msg_size= size;

        return 0;
}

static void send_syscall_info_to_secondary_replicas(struct ft_pop_rep *replica_group, struct ft_pop_rep_id* primary_ft_pop_id, int primary_level, int* primary_id_array, int syscall_id, char* syscall_info, unsigned int syscall_info_size){
        struct syscall_msg* msg;
        int msg_size;
        int ret;

        ret= create_syscall_msg(primary_ft_pop_id, primary_level, primary_id_array, syscall_id, syscall_info, syscall_info_size, &msg, &msg_size);
        if(ret)
                return;

        send_to_all_secondary_replicas(replica_group, (struct pcn_kmsg_long_message*) msg, msg_size);

        kfree(msg);
}

static void send_syscall_info_to_secondary_replicas_from_work(struct work_struct* work){
        struct send_syscall_work *my_work= (struct send_syscall_work*) work;

        send_syscall_info_to_secondary_replicas(my_work->replica_group, &my_work->ft_pop_id, my_work->level, (int*) &my_work->id_array, my_work->syscall_id, my_work->private, my_work->private_data_size);

        put_ft_pop_rep(my_work->replica_group);
	
	kfree(my_work->id_array);
	kfree(my_work->private);
        kfree(my_work);

}

/* Supposed to be called by a primary replica to send syscall info to its secondary replicas.
 * Data sent is stored in @syscall_info and it is of @syscall_info_size bytes.
 * A copy is made so data can be free after the call.
 * The current thread will be used to send the data.
 */
void ft_send_syscall_info(struct ft_pop_rep *replica_group, struct ft_pid *primary_pid, int syscall_id, char* syscall_info, unsigned int syscall_info_size){
	
	send_syscall_info_to_secondary_replicas(replica_group, &primary_pid->ft_pop_id, primary_pid->level, primary_pid->id_array, syscall_id, syscall_info, syscall_info_size);
}

/* As for ft_send_syscall_info, but a worker thread will be used to send the data.
 * Also in this case a copy of the data will be made, so it is possible to free @syscall_info
 * after the call.
 */
void ft_send_syscall_info_from_work(struct ft_pop_rep *replica_group, struct ft_pid *primary_pid, int syscall_id, char* syscall_info, unsigned int syscall_info_size){
	struct send_syscall_work *work;

	FTPRINTK("%s called from pid %s\n", __func__, current->pid);

	work= kmalloc( sizeof(*work), GFP_KERNEL);
	if(!work)
		return;

	get_ft_pop_rep(replica_group);
	work->replica_group= replica_group;

	/* Do a copy of ft_pid because potentially it may exit on the mean while
	 * and the pointer could not be valid anymore...
	 */
	work->ft_pop_id= primary_pid->ft_pop_id;
	work->level= primary_pid->level;
	if(primary_pid->level){
		work->id_array= kmalloc(primary_pid->level*sizeof(int), GFP_KERNEL);
		if(!work->id_array){
			kfree(work);
			return;
		}
                memcpy(work->id_array, primary_pid->id_array, primary_pid->level*sizeof(int));
        }
	
	/* Do a copy of syscall_info */
	work->private_data_size= syscall_info_size;
	if(syscall_info_size){
		work->private= kmalloc(syscall_info_size, GFP_KERNEL);
		if(!work->private){
			kfree(work->id_array);
			kfree(work);
			return;
		}
		memcpy(work->private, syscall_info, syscall_info_size);
	}
	work->syscall_id= syscall_id;
		
	INIT_WORK( (struct work_struct*)work, send_syscall_info_to_secondary_replicas_from_work);

	queue_work(ft_syscall_info_wq, (struct work_struct*)work);

	FTPRINTK("%s work queued\n", __func__);

	return;
	
}

/* Supposed to be called by secondary replicas to wait for syscall data sent by the primary replica.
 * The data returned is the one identified by the ft_pid of the replica and the syscall_id.
 * It may put the current thread to sleep.
 * NOTE: do not try to put more than one thread to sleep for the same data, it won't work. This is
 * designed to allow only the secondary replica itself to sleep while waiting the data from its primary. 
 */
void* ft_wait_for_syscall_info(struct ft_pid *secondary, int id_syscall){
	struct wait_syscall* wait_info;
        struct wait_syscall* present_info= NULL;
	char* key;
        int free_key= 0;
	void* ret= NULL;

	FTPRINTK("%s called from pid %s\n", __func__, current->pid);

	key= ft_syscall_get_key_from_ft_pid(secondary, id_syscall);
        if(!key)
                return ERR_PTR(-ENOMEM);

        wait_info= kmalloc(sizeof(*wait_info), GFP_ATOMIC);
        if(!wait_info)
                return ERR_PTR(-ENOMEM);

        wait_info->task= current;
        wait_info->populated= 0;

        if((present_info= ((struct wait_syscall*) ft_syscall_hash_add(key, (void*) wait_info)))){
		FTPRINTK("%s data present, no need to wait\n", __func__);

                kfree(wait_info);
                free_key= 1;
                goto copy;
        }
        else{
		FTPRINTK("%s: pid %d going to wait for data\n", __func__, current->pid);

                present_info= wait_info;
                while(present_info->populated==0){
                        set_current_state(TASK_UNINTERRUPTIBLE);
                        if(present_info->populated==0);
                                schedule();
                        set_current_state(TASK_RUNNING);
                }
		
		FTPRINTK("%s: data arrived for pid %d \n", __func__, current->pid);
        }


copy:   if(present_info->populated != 1){
                printk("%s ERROR, entry present in syscall hash but not populated\n", __func__);
                ret= ERR_PTR(-EFAULT);
		goto out;
        }

	ret= present_info->private;

out:
	ft_syscall_hash_remove(key);
        if(free_key)
                kfree(key);
        kfree(present_info);

        return ret;


}

static int handle_syscall_info_msg(struct pcn_kmsg_message* inc_msg){
        struct syscall_msg* msg = (struct syscall_msg*) inc_msg;
        struct wait_syscall* wait_info;
        struct wait_syscall* present_info= NULL;
        char* key;
	int* id_array;
	char* private;

	/* retrive variable data length fields (id_array and syscall_info)*/
	id_array= (int*) &msg->data;
	if(msg->level){
		private= &msg->data+ msg->level*sizeof(int);
	}	
	else{
		private= &msg->data;
	}

	/* retrive key for this syscall in hash_table*/
        key= ft_syscall_get_key(&msg->ft_pop_id, msg->level, id_array, msg->syscall_id);
        if(!key)
                return -ENOMEM;

	/* create a wait_syscall struct.
	 * if nobody was already waiting for this syscall, this struct will be added
	 * on the hash table, otherwise the private field will be copied on the wait_syscall
	 * present on the hash table and this one will be discarded.
	 */
        wait_info= kmalloc(sizeof(*wait_info), GFP_ATOMIC);
        if(!wait_info)
                return -ENOMEM;
	
	if(msg->syscall_info_size){
		wait_info->private= kmalloc(msg->syscall_info_size, GFP_ATOMIC);
		if(!wait_info->private){
			kfree(wait_info);
			return -ENOMEM;
		}
		memcpy(wait_info->private, private, msg->syscall_info_size);
	}
	else
		wait_info->private= NULL;

        wait_info->task= NULL;
        wait_info->populated=1;

        if((present_info= ((struct wait_syscall*) ft_syscall_hash_add(key, (void*) wait_info)))){
                present_info->private= wait_info->private;
                present_info->populated= 1;
                wake_up_process(present_info->task);
		kfree(key);
		kfree(wait_info);
        }

        pcn_kmsg_free_msg(msg);

        return 0;

}

long syscall_hook_enter(struct pt_regs *regs)
{
        // System call number is in orig_ax
        if(ft_is_replicated(current)){
                printk("%s in system call [%ld] [%ld]\n", current->comm, regs->orig_ax, regs->ax);
                current->id_syscall++;
        }
        return regs->orig_ax;
}

void syscall_hook_exit(struct pt_regs *regs)
{
        // System call number is in ax
        if(ft_is_replicated(current)){
                printk("%s out[%ld] [%ld]\n", current->comm, regs->orig_ax, regs->ax);
        }
}

static int __init ft_syscall_common_management_init(void) {
	ft_syscall_info_wq= create_singlethread_workqueue("ft_syscall_info_wq");
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_SYSCALL_INFO, handle_syscall_info_msg);
        syscall_hash= create_hashtable(50);
        return 0;
}

late_initcall(ft_syscall_common_management_init);
