/*
 * ft_replication.c
 *
 * Author: Marina
 */

#include <linux/types.h>
#include <linux/ft_replication.h>
#include <linux/popcorn_namespace.h>
#include <linux/pcn_kmsg.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/dcache.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <asm/atomic.h>
#include <asm/ptrace.h>
#include <linux/sched.h>
#include <linux/spinlock.h>

#define FT_REPLICATION_VERBOSE 0
#if FT_REPLICATION_VERBOSE
#define FTPRINTK(...) printk(__VA_ARGS__)
#else
#define FTPRINTK(...) ;
#endif

struct secondary_replica_request{
	struct pcn_kmsg_hdr header;
	struct replica_id primary_replica;
	int replication_degree;
	struct ft_pop_rep_id ft_rep_id;
	int exe_path_size;
	int argc;
	int argv_size;
	int envc;
	int env_size;
	/*data is composed by:
                -exe_path
                -argv
                -env
         */

	//NOTE: do not add fields after data
	char data;
};
static int create_secondary_replica_request_msg(struct task_struct* primary_replica_task, int replication_degree, struct ft_pop_rep_id* ft_rep_id, struct secondary_replica_request** msg, int* msg_size);

struct secondary_replica_answer{
	struct pcn_kmsg_hdr header;
	struct replica_id primary_replica;
	struct replica_id secondary_replica;
	int secondary_replica_created;
};
static int create_secondary_replica_answer_msg(struct replica_id* secondary_replica_from, struct replica_id* primary_replica_to, int error, struct secondary_replica_answer** message, int *msg_size);

struct primary_replica_answer{
        struct pcn_kmsg_hdr header;
        struct replica_id primary_replica;
        struct replica_id secondary_replica;
        int start;
	int secondary_replicas;
	/*data is composed by:
                -pid
		-kernel
	 *of each secondary_replica
         */

        //NOTE: do not add fields after data
	int data;
};
static int create_primary_replica_answer_msg(struct replica_id* primary_replica_from, struct replica_id* secondary_replica_to, int start, struct list_head *replicas_list, struct primary_replica_answer** message, int *msg_size);

struct ft_work{
	struct work_struct work;
	void* data;
};

struct collect_primary_replica_answer{
        struct kref kref;
	struct list_head list_member;
	atomic_t num_answers;
	int start;
	int num_secondary_replicas;
	int* secondary_replica_list;
        struct replica_id secondary_replica;
        struct replica_id primary_replica;
        struct task_struct* waiting;
};
struct collect_primary_replica_answer collect_primary_replica_answer_head;
DEFINE_SPINLOCK(collect_primary_replica_answer_lock);

struct collect_secondary_replica_answers{
       	struct kref kref; 
	struct list_head list_member;
	atomic_t num_answers;
        atomic_t num_replicas;
	spinlock_t lock;
        struct list_head* secondary_replica_head_list;
        struct replica_id primary_replica;
        struct task_struct* waiting;
};
struct collect_secondary_replica_answers collect_secondary_replica_answers_head;
DEFINE_SPINLOCK(collect_secondary_replica_answers_lock);

static struct workqueue_struct *secondary_replica_generator_wq;
extern int _cpu;

static void release_collect_secondary_replica_answers(struct kref *kref){
	struct collect_secondary_replica_answers* collection;

	collection= container_of(kref, struct collect_secondary_replica_answers, kref);

	if(collection)
		kfree(collection);
}

static void release_collect_primary_replica_answer(struct kref *kref){
        struct collect_primary_replica_answer* collection;

        collection= container_of(kref, struct collect_primary_replica_answer, kref);

        if(collection)
                kfree(collection);
}


static void get_collect_primary_replica_answer(struct collect_primary_replica_answer *collection){
	kref_get(&collection->kref);
}

static void get_collect_secondary_replica_answers(struct collect_secondary_replica_answers *collection){
        kref_get(&collection->kref);
}

static void put_collect_primary_replica_answer(struct collect_primary_replica_answer *collection){
	kref_put(&collection->kref, release_collect_primary_replica_answer);
}

static void put_collect_secondary_replica_answers(struct collect_secondary_replica_answers *collection){
        kref_put(&collection->kref, release_collect_secondary_replica_answers);
}

static struct collect_primary_replica_answer * find_and_get_collect_primary_replica_answer(struct replica_id *primary_replica, struct replica_id *secondary_replica){
	struct collect_primary_replica_answer *collection= NULL;
	struct list_head *iter= NULL;
        struct collect_primary_replica_answer *objPtr= NULL;

	spin_lock(&collect_primary_replica_answer_lock);	

        list_for_each(iter, &collect_primary_replica_answer_head.list_member) {
                objPtr = list_entry(iter, struct collect_primary_replica_answer, list_member);
        	if(objPtr->primary_replica.kernel == primary_replica->kernel 
			&& objPtr->primary_replica.pid == primary_replica->pid 
			&& objPtr->secondary_replica.kernel == secondary_replica->kernel
                        && objPtr->secondary_replica.pid == secondary_replica->pid){
			
			collection= objPtr;
			get_collect_primary_replica_answer(collection);
			goto out;
		}        

        }
	
out:	spin_unlock(&collect_primary_replica_answer_lock);
	return collection;
}

static struct collect_secondary_replica_answers * find_and_get_collect_secondary_replica_answers(struct replica_id *primary_replica){
        struct collect_secondary_replica_answers *collection= NULL;
        struct list_head *iter= NULL;
        struct collect_secondary_replica_answers *objPtr= NULL;

        spin_lock(&collect_secondary_replica_answers_lock);

        list_for_each(iter, &collect_secondary_replica_answers_head.list_member) {
                objPtr = list_entry(iter, struct collect_secondary_replica_answers, list_member);
                if(objPtr->primary_replica.kernel == primary_replica->kernel
                        && objPtr->primary_replica.pid == primary_replica->pid){

                        collection= objPtr;
                        get_collect_secondary_replica_answers(collection);
                        goto out;
                }

        }

out:    spin_unlock(&collect_secondary_replica_answers_lock);
        return collection;
}

static void remove_collect_primary_replica_answer(struct collect_primary_replica_answer *collection){
	if(!collection)
		return;

	spin_lock(&collect_primary_replica_answer_lock);
	list_del(&collection->list_member);
	spin_unlock(&collect_primary_replica_answer_lock);
        
	return;
}

static void remove_collect_secondary_replica_answers(struct collect_secondary_replica_answers *collection){
        if(!collection)
                return;

        spin_lock(&collect_secondary_replica_answers_lock);
        list_del(&collection->list_member);
        spin_unlock(&collect_secondary_replica_answers_lock);

        return;
}

static void add_collect_primary_replica_answer(struct collect_primary_replica_answer *collection){
        if(!collection)
                return;

        spin_lock(&collect_primary_replica_answer_lock);
        list_add(&collection->list_member,&collect_primary_replica_answer_head.list_member);
        spin_unlock(&collect_primary_replica_answer_lock);

        return;
}

static void add_collect_secondary_replica_answers(struct collect_secondary_replica_answers *collection){
        if(!collection)
                return;

        spin_lock(&collect_secondary_replica_answers_lock);
        list_add(&collection->list_member,&collect_secondary_replica_answers_head.list_member);
        spin_unlock(&collect_secondary_replica_answers_lock);

        return;
}

atomic_t ft_pop_id_generator= ATOMIC_INIT(0);
static int get_new_ft_pop_id(void){
	return	atomic_inc_return(&ft_pop_id_generator);
}

static void release_ft_pop_rep(struct kref *kref){
        struct ft_pop_rep* ft_pop;

        ft_pop= container_of(kref, struct ft_pop_rep, kref);

        if(ft_pop)
                kfree(ft_pop);
}

void get_ft_pop_rep(struct ft_pop_rep* ft_pop){
        kref_get(&ft_pop->kref);
}

void put_ft_pop_rep(struct ft_pop_rep* ft_pop){
        kref_put(&ft_pop->kref, release_ft_pop_rep);
}

/* Creates a new struct ft_pop_rep.
 *
 * If new_id is not 0, a new identificative is created, otherwise is used
 * the one provided in id.
 *
 * NOTE: each struct ft_pop_rep is identified by the fields {id, kernel}. 
 * Correspondig struct ft_pop_rep in other kernels (the ones created by the replicas)
 * will have the same identificative.
 */
static struct ft_pop_rep* create_ft_pop_rep(int replication_degree, int new_id, struct ft_pop_rep_id* id){
	struct ft_pop_rep *new_ft_pop= NULL;

	new_ft_pop= kmalloc(sizeof(*new_ft_pop), GFP_KERNEL);
	if(!new_ft_pop){	
		return ERR_PTR(-ENOMEM);
	}

	atomic_set(&new_ft_pop->kref.refcount, 1);

	if(new_id){
		new_ft_pop->id.id = get_new_ft_pop_id();
		new_ft_pop->id.kernel = _cpu; 
	}
	else{
		new_ft_pop->id.id = id->id;
                new_ft_pop->id.kernel = id->kernel;
	}
	new_ft_pop->replication_degree= replication_degree;

	INIT_LIST_HEAD(&new_ft_pop->secondary_replicas_head.replica_list_member);
	
	return new_ft_pop;
}

/* Creates and populates a struct secondary_replica_request message with information taken from primary_replica_task.
 *
 * In case of success 0 is returned and a pointer to the message is saved in msg and the corresponding size is set in msg_size.
 * Remember to kfree the message allocated eventually!
 *
 * Message size is variable because the path, argv end env of primary_replica_task vary from task to task. 
 */
static int create_secondary_replica_request_msg(struct task_struct* primary_replica_task, int replication_degree, struct ft_pop_rep_id* ft_rep_id, struct secondary_replica_request** msg, int* msg_size){	
	int argv_size= 0;
	int env_size= 0;
	int total_msg_size= 0;
	struct secondary_replica_request* message;
	char *path, *rpath, *data;
	int max_path_size= 256;
	int ret,i,argc,envc;

	if(!primary_replica_task || !primary_replica_task->mm)
		return -EFAULT;

	argv_size= primary_replica_task->mm->arg_end - primary_replica_task->mm->arg_start;
	env_size= primary_replica_task->mm->env_end - primary_replica_task->mm->env_start;
	
path_again:

	path= kmalloc(sizeof(char)*max_path_size, GFP_KERNEL);
	if(!path){
		return -ENOMEM;
	}

    	rpath= d_path(&primary_replica_task->mm->exe_file->f_path, path, max_path_size);
	if(IS_ERR(rpath)){
		kfree(path);
		if(rpath == ERR_PTR(-ENAMETOOLONG)){
			max_path_size= max_path_size*2;
			goto path_again;			
		}
		else{
			return PTR_ERR(rpath);
		}
	}

	total_msg_size= sizeof(*message)+ argv_size+ env_size+ max_path_size;

	message= kmalloc(total_msg_size, GFP_KERNEL);
	if(!message){
		ret= -ENOMEM;
		goto out;
	}

	/*message->data is composed by:
		-exe_path
		-argv
		-env
	*/
	data= (char *) &message->data;
	strncpy(data, rpath, max_path_size);
	message->exe_path_size= max_path_size;
	
	FTPRINTK("%s: replicating %s\n", __func__, data);
	
	data= data+ max_path_size;
	if (copy_from_user(data, (const void __user *)primary_replica_task->mm->arg_start, argv_size)) {
        	ret= -EFAULT;
		goto out_msg;	
        }

	argc= 0;
	for(i = 0; i < argv_size; i++){
        	if (data[i] == 0){
			argc++;
		}
	}

	message->argv_size= argv_size;
	message->argc= argc;
	
	data= data+ argv_size;
	if (copy_from_user(data, (const void __user *)primary_replica_task->mm->env_start, env_size)) {
                ret= -EFAULT;
                goto out_msg;
        }

	envc= 0;
	for(i = 0; i < env_size; i++){
                if (data[i] == 0){
			envc++;
		}
        }
        
	message->env_size= env_size;
	message->envc= envc;

	message->primary_replica.kernel= _cpu;
	message->primary_replica.pid= primary_replica_task->pid;
	message->replication_degree= replication_degree;
	message->ft_rep_id= *ft_rep_id;	
		
	message->header.type= PCN_KMSG_TYPE_FT_SECONDARY_REPLICA_REQUEST;
	message->header.prio= PCN_KMSG_PRIO_NORMAL;
	
	*msg= message;
	*msg_size= total_msg_size;
	ret= 0;
	goto out;
	
out_msg:
	kfree(message);
out:	
	kfree(path);
	return ret;

}

/* Creates and populates a struct secondary_replica_answer message.
 *
 * In case of success 0 is returned and a pointer to the message is saved in message and the corresponding size is set in msg_size.
 * Remember to kfree the message allocated eventually!
 *      
 */
static int create_secondary_replica_answer_msg(struct replica_id* secondary_replica_from, struct replica_id* primary_replica_to, int error, struct secondary_replica_answer** message, int *msg_size){
	struct secondary_replica_answer* msg;

	if(!secondary_replica_from||!primary_replica_to)
		return -EFAULT;

        msg= kmalloc(sizeof(*msg),GFP_KERNEL);
        if(!msg)
                return -ENOMEM;

        msg->primary_replica= *primary_replica_to;
        msg->secondary_replica= *secondary_replica_from;
	if(error)
        	msg->secondary_replica_created= 0;
	else
		msg->secondary_replica_created= 1;

        msg->header.type= PCN_KMSG_TYPE_FT_SECONDARY_REPLICA_ANSWER;
        msg->header.prio= PCN_KMSG_PRIO_NORMAL;

	*message= msg;
	*msg_size= sizeof(*msg);

	return 0;
}

/* Creates and populates a struct primary_replica_answer message.
 *
 * In case of success 0 is returned and a pointer to the message is saved in message and the corresponding size is set in msg_size.
 * Remember to kfree the message allocated eventually!
 *
 * Message size is variable because a list of the secondary replicas copied from replicas_list is appended to it. 
 */
static int create_primary_replica_answer_msg(struct replica_id* primary_replica_from, struct replica_id* secondary_replica_to, int start, struct list_head *replicas_list, struct primary_replica_answer** message, int *msg_size){
	struct primary_replica_answer* msg;
	int secondary_replicas= 0;
	int message_size;
	struct list_head *iter= NULL;
	struct list_head *n= NULL;
	struct replica_id_list *objPtr= NULL;
	struct replica_id secondary_replica;
	int* datap;

	if(!primary_replica_from || !secondary_replica_to)
                return -EFAULT;

	if(start){
		if(!replicas_list){
                        return -EFAULT;
                }

		list_for_each_safe(iter, n, replicas_list) {
	                secondary_replicas++;
                }
		
		message_size= sizeof(*msg)+ 2*sizeof(int)*secondary_replicas;
	}
	else{
		message_size= sizeof(*msg);
	}

	msg= kmalloc(message_size,GFP_KERNEL);
        if(!msg)
                return -ENOMEM;

        msg->primary_replica= *primary_replica_from;
        msg->secondary_replica= *secondary_replica_to;
        if(start){
                msg->start= 1;
		msg->secondary_replicas= secondary_replicas;
		iter= NULL;
		n= NULL;
		datap= &msg->data;
		list_for_each_safe(iter, n, replicas_list) {
                	objPtr = list_entry(iter, struct replica_id_list, replica_list_member);
                	secondary_replica= objPtr->replica;
			*datap++= secondary_replica.pid;
			*datap++= secondary_replica.kernel;
		}
        }
        else{
                msg->start= 0;
	}

        msg->header.type= PCN_KMSG_TYPE_FT_PRIMARY_REPLICA_ANSWER;
        msg->header.prio= PCN_KMSG_PRIO_NORMAL;

        *message= msg;
        *msg_size= message_size;

        return 0;

}

/* send msg to all secondary replica listed in ft_popcorn.
 *
 */
void send_to_all_secondary_replicas(struct ft_pop_rep* ft_popcorn, struct pcn_kmsg_long_message* msg, int msg_size){
	struct list_head *iter= NULL;
        struct replica_id secondary_replica;
        struct replica_id_list* objPtr;

	list_for_each(iter, &ft_popcorn->secondary_replicas_head.replica_list_member) {
                objPtr = list_entry(iter, struct replica_id_list, replica_list_member);
                secondary_replica= objPtr->replica;

                pcn_kmsg_send_long(secondary_replica.kernel, msg, msg_size-sizeof(msg->hdr));

        }

}

/* Returns 1 in case task is a secondary replica or secondary replica descendant.
 *
 */
int ft_is_secondary_replica(struct task_struct *task){
        struct task_struct *ancestor;

        ancestor= find_task_by_vpid(task->tgid);

        if(ancestor->replica_type == SECONDARY_REPLICA
                || ancestor->replica_type == NEW_SECONDARY_REPLICA_DESCENDANT){

                return 1;
        }

        return 0;
}

/* Returns 1 in case task is an primary replica or primary replica descendant.
 *
 */
int ft_is_primary_replica(struct task_struct *task){
	struct task_struct *ancestor;

        ancestor= find_task_by_vpid(task->tgid);

        if(ancestor->replica_type == PRIMARY_REPLICA
                || ancestor->replica_type == NEW_PRIMARY_REPLICA_DESCENDANT){
                
                return 1;
        }

        return 0;
}

/* Returns 1 in cast task is ft replicated, 0 otherwise.
 *
 */
int ft_is_replicated(struct task_struct *task){

	if(task->replica_type == PRIMARY_REPLICA
                || task->replica_type == SECONDARY_REPLICA
                || task->replica_type == NEW_PRIMARY_REPLICA_DESCENDANT
                || task->replica_type == NEW_SECONDARY_REPLICA_DESCENDANT
                || task->replica_type == REPLICA_DESCENDANT){	
		
		return 1;
	}

	return 0;
}

/* Checks if two struct ft_pid are equals.
 *
 * Returns 1 if they are equals, 0 otherwise.
 */
int are_ft_pid_equals(struct ft_pid* first, struct ft_pid* second){
	int ret= 0;
	int i;

	/*Same ft_pop_rep_id??*/
	if(first->ft_pop_id.kernel != second->ft_pop_id.kernel
		|| first->ft_pop_id.id != second->ft_pop_id.id)
		goto out;

	/*Same level??*/
	if(first->level != second->level)
		goto out;
	
	if(first->level==0){
		ret= 1;
		goto out;
	}

	/*Same ancestors??*/
	for(i=0;i<first->level;i++){
		if(first->id_array[i]!=second->id_array[i])
			goto out;
	}

	ret= 1;
		
out:	return ret;
}

/* Converts a ft_pid struct in a string.
 *
 * Remember to kfree the returned string eventually.
 */
char* print_ft_pid(struct ft_pid* pid){
	char *string;
	const int size= 1024;
	int i,pos;
	
	if(!pid)
		return NULL;

	string= kmalloc(size, GFP_ATOMIC);
	if(!string)
		return NULL;

	pos= snprintf(string, size,"{ ft_pop_rep_id: {kernel %d, id %d}, level: %d", pid->ft_pop_id.kernel, pid->ft_pop_id.id, pid->level);
	if(pos>=size)
		goto out_clean;
	
	if(pid->level){
		pos= pos+ snprintf(&string[pos], size-pos, ", id_array: [");
		if(pos>=size)
	                goto out_clean;

		for(i=0;i<pid->level-1;i++){
        		pos= pos+ snprintf(&string[pos], size-pos, " %d,",pid->id_array[i]);
			if(pos>=size)
                        	goto out_clean;
		}

		pos= pos+ snprintf(&string[pos], size-pos, " %d]}", pid->id_array[pid->level-1]);
		if(pos>=size)
                	goto out_clean;
	}
	else{
		pos= pos+ snprintf(&string[pos], size-pos, "}");
		if(pos>=size)
                	goto out_clean;
	}

	return string;

out_clean:
	kfree(string);
	printk("%s: buffer size too small\n", __func__);
	return NULL;
	
}

/* Set ft-popcorn fields of struct task_struct;
 * If popcorn_namespace is active, replica_type and ft_pid are
 * defined here.
 */
int copy_replication(unsigned long flags, struct task_struct *tsk){
	struct popcorn_namespace *pop;
	struct task_struct* ancestor;

	pop= tsk->nsproxy->pop_ns;
	if(is_popcorn_namespace_active(pop)){
		if(current->pid == pop->root && current->replica_type == ROOT_POT_PRIMARY_REPLICA){
			tsk->replica_type= POTENTIAL_PRIMARY_REPLICA;
			tsk->ft_popcorn= NULL;
			tsk->ft_pid.level= 0;
			tsk->next_pid_to_use= 0;
			tsk->next_id_resources= 0;
			tsk->id_syscall= 0;
                	tsk->ft_pid.id_array= NULL;
			tsk->useful= NULL;

			return 0;
		}
		else{
		
			if(!current->ft_popcorn){
				/* assosiated to an active popcorn namespace (=>with a root)
				 * but it is not the root that is forking me, and any of my ancestors
				 * exeve on a new program (otherwise ft_popcorn would be != NULL)...
				 * not sure what to do about them.
				 * we can let them be not_replicated to avoid explosion of replica, but this can be changed
				 */	
	
				tsk->ft_pid.level= 0;
		                tsk->ft_pid.id_array= NULL;
                		tsk->next_pid_to_use= 0;
                		tsk->next_id_resources= 0;
                		tsk->id_syscall= 0;
				tsk->replica_type= NOT_REPLICATED;
                		tsk->ft_popcorn= NULL;
				tsk->useful= NULL;

				return 0;
			}

			/* All forked threads should have the same ft_popcorn because all have as ancestor 
                         * the same primary/secondary replica.
                         * The same shuould apply for new processes. However now the initial primary/secondary 
                         * replica can exit indipendently by this new process. Should we recompute all the 
                         * replica ids for this new process???
                         */
	
			tsk->ft_popcorn= current->ft_popcorn; 
			if(!(flags & CLONE_THREAD)){
				get_ft_pop_rep(tsk->ft_popcorn);
				
				ancestor= find_task_by_vpid(current->tgid);
				if(ancestor->replica_type == PRIMARY_REPLICA
					|| ancestor->replica_type == NEW_PRIMARY_REPLICA_DESCENDANT){
					
					tsk->replica_type= NEW_PRIMARY_REPLICA_DESCENDANT;
					printk("%s: created NEW_PRIMARY_REPLICA_DESCENDANT from (pid %d tgid %d)\n",__func__, tsk->pid, tsk->tgid);
				}
				else{
					if(ancestor->replica_type == SECONDARY_REPLICA
                                        	|| ancestor->replica_type == NEW_SECONDARY_REPLICA_DESCENDANT){
                                        
						tsk->replica_type= NEW_SECONDARY_REPLICA_DESCENDANT;
						printk("%s: created NEW_SECONDARY_REPLICA_DESCENDANT from (pid %d tgid %d)\n", __func__, tsk->pid, tsk->tgid);
                                	}
					else{
						//BUG();
						printk("%s: ERROR ancestor (pid %d) replica type is %d",__func__, ancestor->pid, ancestor->replica_type);
						return -1;
					}
				}
			}
			else{
				tsk->replica_type= REPLICA_DESCENDANT;
				printk("%s: created REPLICA_DESCENDANT from (pid %d tgid %d)\n", __func__, tsk->pid, tsk->tgid);
			}

			/* Compute the ft_pid.
			 * Note that same replicas in different kernel will end up with the same ft_pid.
			 */
			tsk->ft_pid.level= current->ft_pid.level+1;
			tsk->ft_pid.ft_pop_id= tsk->ft_popcorn->id;
			tsk->next_pid_to_use= 0;
			tsk->ft_pid.id_array= kmalloc(sizeof(int)*(tsk->ft_pid.level),GFP_KERNEL);
			if(!tsk->ft_pid.id_array){
				return -ENOMEM;
			}
			
			if(current->ft_pid.id_array){
				memcpy(tsk->ft_pid.id_array,current->ft_pid.id_array,sizeof(int)*current->ft_pid.level);
			}
			
			tsk->ft_pid.id_array[current->ft_pid.level]= current->next_pid_to_use++;

			tsk->next_id_resources= 0;
			tsk->id_syscall= 0;
			tsk->useful= NULL;
		}
	}
	else{
		tsk->ft_pid.level= 0;
		tsk->ft_pid.id_array= NULL;	
		tsk->next_pid_to_use= 0;
		tsk->next_id_resources= 0;
		tsk->replica_type= NOT_REPLICATED;
		tsk->ft_popcorn= NULL;
		tsk->useful= NULL;
	}
	
	return 0;
}

static int _send_update_to_primary_replica(struct replica_id* primary_replica_to, struct replica_id* secondary_replica_from, int error){
        struct secondary_replica_answer* msg;
        int msg_size;
        int ret;
        
        ret= create_secondary_replica_answer_msg(secondary_replica_from, primary_replica_to, error, &msg, &msg_size);
        if(ret)
        	return ret;

        ret= pcn_kmsg_send_long(primary_replica_to->kernel,(struct pcn_kmsg_long_message*) msg, msg_size-sizeof(msg->header));
	if(!ret){
		FTPRINTK("%s: sent update to primary replica pid %d in kernel %d with %s\n",__func__, primary_replica_to->pid, primary_replica_to->kernel, ((error)?"error":"no error"));
	}

        kfree(msg);

	return ret;
}

static int send_ack_to_primary_replica(struct replica_id* primary_replica_to, struct replica_id* secondary_replica_from){

	return _send_update_to_primary_replica(primary_replica_to, secondary_replica_from, 0);

}

static void _send_error_to_primary_replica_from_work(struct work_struct* work){
        struct ft_work* my_work= (struct ft_work*) work;
	struct replica_id* primary_replica= (struct replica_id*) my_work->data;
	struct replica_id secondary_replica;
        secondary_replica.kernel= _cpu;

	_send_update_to_primary_replica(primary_replica, &secondary_replica, 1);

	kfree(primary_replica);
	kfree(work);
	return;		
}

static void send_error_to_primary_replica(struct replica_id* primary_replica, int from_same_thread){
	struct ft_work *work;
	struct replica_id* primary_rep_copy;
	struct replica_id secondary_rep;
	
	if(from_same_thread){
		secondary_rep.kernel= _cpu;
		secondary_rep.pid= current->pid;
		_send_update_to_primary_replica(primary_replica, &secondary_rep, 1);
	}
	else{
        	work= kmalloc(sizeof(*work), GFP_ATOMIC);
        	if(!work){
                	return;
        	}
        	
		primary_rep_copy= kmalloc(sizeof(*primary_rep_copy), GFP_ATOMIC);
		if(!primary_rep_copy){
			kfree(work);
			return;
		}

		INIT_WORK( (struct work_struct*)work, _send_error_to_primary_replica_from_work);
		*primary_rep_copy= *primary_replica;
		
		work->data= primary_rep_copy;

		queue_work(secondary_replica_generator_wq, (struct work_struct*)work);

	}
                  
}

/* see ____call_usermodehelper of kernel/kmod.h
 * 
 * Associates the current thread to Popcorn namespace, set the replica_type to POTENTIAL_SECONDARY_REPLICA
 * and tries to execve the current thread with exec path, argv and env of the primary replica stored in data.
 * 
 * In case of error it sends an "error" message to the primary replica.
 */
static int exec_to_secondary_replica(void *data){
	struct secondary_replica_request* msg = data;
        struct cred *new;
        int retval;
	char *exe_pathp,*argvp,*envp;
	char **copy_argv,**copy_env;
	int next_is_pointer,i,nr_pointers;

        spin_lock_irq(&current->sighand->siglock);
        flush_signal_handlers(current, 1);
        spin_unlock_irq(&current->sighand->siglock);

        set_cpus_allowed_ptr(current, cpu_all_mask);

        set_user_nice(current, 0);

        retval = -ENOMEM;
        new = prepare_kernel_cred(current);
        if (!new)
                goto fail;

	new->cap_bset = CAP_FULL_SET;
    	new->cap_inheritable = CAP_FULL_SET;
        commit_creds(new);
	
	retval= associate_to_popcorn_ns(current, msg->replication_degree);
	if(retval){
		goto fail;
	}

	current->replica_type= POTENTIAL_SECONDARY_REPLICA;
	current->useful= msg;

	exe_pathp= (char *) &msg->data;
	argvp= exe_pathp+ msg->exe_path_size;
	envp= argvp+ msg->argv_size;

	copy_argv= kmalloc((msg->argc+1)*sizeof(char*), GFP_KERNEL);
	if(!copy_argv){
		retval= -ENOMEM;	
		goto fail;
	}
	copy_env= kmalloc((msg->envc+1)*sizeof(char*), GFP_KERNEL);
	if(!copy_env){
		retval= -ENOMEM;
		kfree(copy_argv);
		goto fail;
	}
		
	next_is_pointer= 1;
	nr_pointers= 0;
	for(i = 0; i < msg->argv_size; i++){
                if(next_is_pointer == 1){
			if(nr_pointers == msg->argc){
				nr_pointers++;
				break;
			}
			copy_argv[nr_pointers++]= &argvp[i]; 
			next_is_pointer= 0;
		}
		if (argvp[i] == 0){
                        next_is_pointer= 1;
                }
        }
        if(nr_pointers != msg->argc){
		printk("ERROR at %s: nr_pointers is %d argc is %d\n", __func__, nr_pointers, msg->argc);
		//BUG();
		retval= -EFAULT;
		goto out;
	}
	copy_argv[nr_pointers]= NULL;
	
	
	next_is_pointer= 1;
	nr_pointers= 0;
	for(i = 0; i < msg->env_size; i++){
                if(next_is_pointer == 1){
			if(nr_pointers == msg->envc){
                                nr_pointers++;
                                break;
                        }
                        copy_env[nr_pointers++]= &envp[i];
                        next_is_pointer= 0;
                }
                if (envp[i] == 0){
                        next_is_pointer= 1;
                }
        }
        if(nr_pointers != msg->envc){
                printk("ERROR at %s: nr_pointers is %d envc is %d\n", __func__, nr_pointers, msg->envc);
        	//BUG();
		retval= -EFAULT;
		goto out;
	}
	copy_env[nr_pointers]= NULL;

	FTPRINTK("%s: pid %d going to execve to %s for beeing a replica of pid %d in kernel %d\n",__func__, current->pid, exe_pathp, msg->primary_replica.pid, msg->primary_replica.kernel);

	retval = kernel_execve(exe_pathp,
                               (const char *const *)copy_argv,
                               (const char *const *)copy_env);

	/* Ok, if the kernel_execve succeded and only maybe_create_replicas failed,
	 * the stack maybe a little mess up. 
	 * I saw that parameters are not available any more => data is NULL.
	 * Just try to retrive them again? Avoiding using them? Be careful there migth be more errors...
	 */

	kfree(copy_argv);
        kfree(copy_env);

        printk("%s failed retval{%d}\n",__func__, retval);

        if(retval != -ENOFTREP){
                send_error_to_primary_replica(&((struct secondary_replica_request*) current->useful)->primary_replica, 1);
        }

        pcn_kmsg_free_msg(current->useful);

        do_exit(0);

	
out:	kfree(copy_argv);
	kfree(copy_env);

fail:

    	printk("%s failed retval{%d}\n",__func__, retval);
	
	if(retval != -ENOFTREP){
		send_error_to_primary_replica(&msg->primary_replica, 1);
	}

	pcn_kmsg_free_msg(msg);

	do_exit(0);
}

/* Forks a new thread in a new Popcorn namespace that will execve to a secondary replica.
 * 
 * In case of error during the fork, it sends and "error" message to the primary replica.
 */
static void create_thread_secondary_replica(struct work_struct* work){
	long ret;
	struct secondary_replica_request* msg;
	struct ft_work* my_work;

	my_work= (struct ft_work*) work;
	msg= (struct secondary_replica_request*) my_work->data;	
	
	FTPRINTK("%s: received new replica request from pid %d in kernel %d\n", __func__, msg->primary_replica.pid, msg->primary_replica.kernel);

	ret= kernel_thread( exec_to_secondary_replica, my_work->data,
                                    CLONE_VFORK | SIGCHLD | CLONE_NEWPOPCORN);

	if(IS_ERR_VALUE(ret)){
		send_error_to_primary_replica(&msg->primary_replica, 1);
                pcn_kmsg_free_msg(msg);
	}

	kfree(work);
}

static int handle_secondary_replica_request(struct pcn_kmsg_message* inc_msg) {
	struct ft_work *work;

	work= kmalloc(sizeof(*work),GFP_ATOMIC);
	if(!work){
		pcn_kmsg_free_msg(inc_msg);
        	return -ENOMEM;
	}

        INIT_WORK( (struct work_struct*)work, create_thread_secondary_replica);
        work->data= inc_msg;
        queue_work(secondary_replica_generator_wq, (struct work_struct*)work);

	return 0;
}

static int send_update_to_secondary_replica(struct replica_id* primary_replica_from, struct replica_id* secondary_replica_to, int start, struct list_head *secondary_replicas_list){
        struct primary_replica_answer* msg;
        int msg_size;
        int ret;

	ret= create_primary_replica_answer_msg(primary_replica_from, secondary_replica_to, start, secondary_replicas_list, &msg, &msg_size);
        if(ret)
                return ret;
	
	ret= pcn_kmsg_send_long(secondary_replica_to->kernel,(struct pcn_kmsg_long_message*) msg, msg_size-sizeof(msg->header));

        kfree(msg);
	return ret;
}

static void send_error_to_secondary_replica_from_work(struct work_struct* work){
        struct ft_work* my_work= (struct ft_work*) work;
	struct secondary_replica_answer* msg= (struct secondary_replica_answer*) my_work->data;

        send_update_to_secondary_replica(&msg->primary_replica, &msg->secondary_replica, 0, NULL);
	
	pcn_kmsg_free_msg(msg);
	kfree(work);
}

/* Sends an "error" message to the first replica_number secondary replicas stored in answer_collection and removes them from the list.
 *
 */
static void send_error_to_secondary_replicas_and_remove_from_collection(struct collect_secondary_replica_answers* answer_collection, int replica_number){
	struct list_head *iter= NULL;
	struct list_head *n= NULL;
        struct replica_id_list *objPtr= NULL;
       	struct replica_id secondary_replica;
	int count= 0;

	if(replica_number==0)	
		return;

	spin_lock(&answer_collection->lock);

	list_for_each_safe(iter, n, answer_collection->secondary_replica_head_list) {
                objPtr = list_entry(iter, struct replica_id_list, replica_list_member);
                secondary_replica= objPtr->replica;
		
		send_update_to_secondary_replica(&answer_collection->primary_replica, &secondary_replica, 0, NULL);
		
		list_del(&objPtr->replica_list_member);
		kfree(objPtr);
		
		atomic_dec(&answer_collection->num_replicas);

		count++;

		if(count==replica_number)
			goto out;
	}
	
out:	spin_unlock(&answer_collection->lock);
}

/* Sends a "start" message to all secondary replicas stored in answer_collection.
 * 
 * Returns 0 in case of success.
 */
static int send_ack_to_secondary_replicas(struct collect_secondary_replica_answers* answer_collection){
	struct list_head *iter= NULL;
        struct list_head *n= NULL;
        struct replica_id_list *objPtr= NULL;
        struct replica_id secondary_replica;
	int ret= 0;

	spin_lock(&answer_collection->lock);

        list_for_each_safe(iter, n, answer_collection->secondary_replica_head_list) {
                objPtr = list_entry(iter, struct replica_id_list, replica_list_member);
                secondary_replica= objPtr->replica;

                ret= send_update_to_secondary_replica(&answer_collection->primary_replica, &secondary_replica, 1, answer_collection->secondary_replica_head_list);
		if(ret)
			goto out;
        }
	
out:    spin_unlock(&answer_collection->lock);
	
	return ret;
}

static int handle_primary_replica_answer(struct pcn_kmsg_message* inc_msg){
	struct primary_replica_answer* msg= (struct primary_replica_answer*) inc_msg;
	struct collect_primary_replica_answer* collection;
	int ret= 0;

	FTPRINTK("%s: received msg from primary replica %d pid in kernel %d to pid %d in kernel %d. Secondary replica %s allowed to start\n", __func__, msg->primary_replica.pid, msg->primary_replica.kernel, msg->secondary_replica.pid, msg->secondary_replica.kernel, ((msg->start==1)?"":"not"));

	collection= find_and_get_collect_primary_replica_answer(&msg->primary_replica, &msg->secondary_replica);
	if(collection){
        	collection->start= msg->start;
		if(msg->start){
			collection->num_secondary_replicas= msg->secondary_replicas;
			collection->secondary_replica_list= kmalloc(2*sizeof(int)*msg->secondary_replicas, GFP_ATOMIC);
			if(!collection->secondary_replica_list){
				ret= -ENOMEM;
				goto out;
			}
			memcpy(collection->secondary_replica_list, &msg->data, 2*sizeof(int)*msg->secondary_replicas);
		}
                atomic_inc(&collection->num_answers);
                wake_up_process(collection->waiting);
		put_collect_primary_replica_answer(collection);
	}
	else{
		FTPRINTK("%s: received msg from primary replica %d pid in kernel %d to pid %d in kernel %d but nobody waiting for this msg...\n", __func__, msg->primary_replica.pid, msg->primary_replica.kernel, msg->secondary_replica.pid, msg->secondary_replica.kernel);
	}

out:	pcn_kmsg_free_msg(msg);
        return ret;

}

static int handle_secondary_replica_answer(struct pcn_kmsg_message* inc_msg){
	struct secondary_replica_answer* msg= (struct secondary_replica_answer*) inc_msg;
	struct ft_work* work;
	struct collect_secondary_replica_answers* collection;
	struct replica_id_list *entry;
	int ret= 0;

	collection= find_and_get_collect_secondary_replica_answers(&msg->primary_replica);

	if(!collection){
		
		FTPRINTK("%s: received msg for primary replica %d pid in kernel %d from pid %d in kernel %d but nobody is waiting\n", __func__, msg->primary_replica.pid, msg->primary_replica.kernel, msg->secondary_replica.pid, msg->secondary_replica.kernel);

		if(msg->secondary_replica_created == 1){
			//send error
			work= kmalloc(sizeof(*work), GFP_ATOMIC);
                	if(!work){
                        	ret= -ENOMEM;
				goto out;
                	}

                	INIT_WORK( (struct work_struct*)work, send_error_to_secondary_replica_from_work);
                	work->data= msg;

                	queue_work(secondary_replica_generator_wq, (struct work_struct*)work);
			ret= 0;
			goto out;		
		}
		else{
			ret= 0;
			goto out_msg;
		}
	}
	else{
		
		FTPRINTK("%s: received msg for primary replica %d pid in kernel %d from pid %d in kernel %d. Replica was %s created\n", __func__, msg->primary_replica.pid, msg->primary_replica.kernel, msg->secondary_replica.pid, msg->secondary_replica.kernel, ((msg->secondary_replica_created==1)?"":"not"));

		if(msg->secondary_replica_created == 1){
			entry= kmalloc(sizeof(*entry), GFP_ATOMIC);
			if(!entry){
                                ret= -ENOMEM;
				goto out_put;
			}
			
			entry->replica= msg->secondary_replica;
			INIT_LIST_HEAD(&entry->replica_list_member);
			
			spin_lock(&collection->lock);
   			
			list_add(&entry->replica_list_member, collection->secondary_replica_head_list);
			atomic_inc(&collection->num_replicas);
			
			spin_unlock(&collection->lock);			
		}		

	}	
	
	atomic_inc(&collection->num_answers);
	wake_up_process(collection->waiting);	
out_put:	
	put_collect_secondary_replica_answers(collection);
out_msg:	
	pcn_kmsg_free_msg(msg);
out:		
	return ret;
}

static int wait_for_primary_replica_answer(struct collect_primary_replica_answer * answer_collection){
	unsigned long timeout = msecs_to_jiffies(WAIT_ANSWER_TIMEOUT_SECOND*1000*NR_CPUS) + 1;

        while(!atomic_read(&answer_collection->num_answers)){
            	timeout = schedule_timeout_interruptible(timeout);
		if(!atomic_read(&answer_collection->num_answers)){
                        if(!timeout){
				FTPRINTK("%s: timeout with %d answers\n", __func__, atomic_read(&answer_collection->num_answers));
                                return -ETIME;
                        }
                }
        }

        return 0;
}

static int wait_for_secondary_replica_answers(struct collect_secondary_replica_answers* answer_collection, int secondary_copies_requested){
	unsigned long timeout = msecs_to_jiffies(WAIT_ANSWER_TIMEOUT_SECOND*1000) + 1;

	if(secondary_copies_requested == 0){
		return 0;
	}

	while(secondary_copies_requested != atomic_read(&answer_collection->num_answers)){
		timeout = schedule_timeout_interruptible(timeout);
		if(secondary_copies_requested != atomic_read(&answer_collection->num_answers)){
			if(!timeout){
				FTPRINTK("%s: timeout with %d answers\n",__func__,atomic_read(&answer_collection->num_answers));
				return -ETIME;
			}
		}
	}

	return 0;
}

/* Tries to send msg (on behalf of primary_replica_from) to number_of_replicas different kernels.
 * 
 * Target kernels are selected sequentially from id 0 to NR_CPUS (excluding the current kernel id)
 * while number_of_replicas correct secondary replicas have been created.
 *
 * When is possible to communicate with a target kernel, an update from that kernel is waited to know if
 * the secondary replica was correctly created. If the target kernel does not answer within a timeout, that 
 * replica is discarded.
 *
 * In case number_of_replicas replicas were successfully created, a message to all the secondary replicas is sent to 
 * allow them to start their execution. 
 *
 * In case the replicas created are less then number_of_replicas requested, an error message is sent to the created
 * ones to discard them.
 *
 * In case of success it returns 0 and populate secondary_replica_head with a list of the secondary replicas created, a value <0 otherwise.
 * If repication requirements are not met -ENOFTREP is returned.
 */
static int send_secondary_replica_requests(struct secondary_replica_request* msg, int msg_size, struct replica_id* primary_replica_from, int number_of_replicas, struct list_head* secondary_replica_head){
	int i,ret= 0;
	int secondary_copies_requested= 0;	
	int secondary_copy_collected= 0;
	int sent_to[NR_CPUS]= {0};
	struct collect_secondary_replica_answers* answer_collection;

	if(!msg || !primary_replica_from)
		return -EFAULT;

	if(number_of_replicas == 0)
		return 0;
		
	answer_collection= kmalloc(sizeof(*answer_collection),GFP_KERNEL);
	if(!answer_collection)
		return -ENOMEM;

	kref_init(&answer_collection->kref);
	INIT_LIST_HEAD(&answer_collection->list_member);
	answer_collection->primary_replica= *primary_replica_from;
	answer_collection->waiting= current;
	atomic_set(&answer_collection->num_replicas,0);
	atomic_set(&answer_collection->num_answers,0);
	answer_collection->secondary_replica_head_list= secondary_replica_head;
	spin_lock_init(&answer_collection->lock);
	
	add_collect_secondary_replica_answers(answer_collection);

	FTPRINTK("%s: starting request for %d replicas\n",__func__,number_of_replicas);
again:
	secondary_copies_requested= 0;
	atomic_set(&answer_collection->num_answers,0);

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
		if(sent_to[i] == 0){
			// Send the request to cpu i.
			ret= pcn_kmsg_send_long(i,(struct pcn_kmsg_long_message*) msg, msg_size-sizeof(msg->header));
			if(!ret) {
				FTPRINTK("%s: replica request sent to kernel %d\n",__func__,i);
				secondary_copies_requested++;
				sent_to[i]= 1;
				if(secondary_copies_requested + secondary_copy_collected == number_of_replicas){
					goto next;
				}
			}
		}
    	}	

next:	if(secondary_copies_requested + secondary_copy_collected != number_of_replicas){
		//not enougth kernels available
		FTPRINTK("%s: not enougth kernels available for creating %d replicas\n", __func__, number_of_replicas);
						
		wait_for_secondary_replica_answers(answer_collection, secondary_copies_requested);

		remove_collect_secondary_replica_answers(answer_collection);

		if(atomic_read(&answer_collection->num_replicas) > 0){
			//somebody timouted maybe answered...
			if(atomic_read(&answer_collection->num_replicas) >= number_of_replicas){
				goto collected;
			}
			else{
				send_error_to_secondary_replicas_and_remove_from_collection(answer_collection, atomic_read(&answer_collection->num_replicas));
			}			
		}
		
		
		ret= -ENOFTREP;
		goto out;
	}
	else{
		wait_for_secondary_replica_answers(answer_collection, secondary_copies_requested);
		
		if(atomic_read(&answer_collection->num_replicas) != number_of_replicas){
			secondary_copy_collected= atomic_read(&answer_collection->num_replicas);
			goto again;
		}
		
		remove_collect_secondary_replica_answers(answer_collection);
	}


collected:
	//if somebody timouted answered, there can be more replicas than necessary	
	if(atomic_read(&answer_collection->num_replicas)>number_of_replicas){
		send_error_to_secondary_replicas_and_remove_from_collection(answer_collection, atomic_read(&answer_collection->num_replicas)-number_of_replicas);
	}

	ret= send_ack_to_secondary_replicas(answer_collection);

out:	put_collect_secondary_replica_answers(answer_collection);
	
	return ret;

}

/* Creates replication_degree-1 secondary replicas of primary_replica_task.
 * 
 * In case of success 0 is returned and secondary_replica_head is populated with 
 * a list of struct replica_id_list of the secondary replicas created.
 *
 * Replicas will start in newly forked thread and will be forced to execve
 * to the same exec, with the same path, env and args of primary_replica_task.
 *
 */
static int create_replicas(struct task_struct* primary_replica_task, int replication_degree, struct ft_pop_rep_id *ft_rep_id, struct list_head* secondary_replica_head){
	struct secondary_replica_request* msg= NULL;
	int msg_size= 0;
	int ret= 0;
	struct replica_id primary_replica;

	FTPRINTK("%s: thread %d requested %d replicas\n", __func__, primary_replica_task->pid, replication_degree-1);

	ret= create_secondary_replica_request_msg(primary_replica_task, replication_degree, ft_rep_id, &msg, &msg_size);
	if(ret)
		return ret;
	
	primary_replica.kernel= _cpu;
	primary_replica.pid= primary_replica_task->pid;

	/*replication_degree includes the primary one*/
	ret= send_secondary_replica_requests(msg, msg_size, &primary_replica, replication_degree-1, secondary_replica_head);
	
	kfree(msg);

	return ret;
}

/* Notify primary_replica_to that secondary_replica_from has been created.
 * 
 * It waits for an update message from primary_replica_to for a timeout.
 * The update message can be "start" type or "discard". 
 *
 * In case a "start" message is received list_secondary_replicas is populated with 
 * the received list of secondary replicas.
 * 
 * Return 0 if the secondary copy can start, a value <0 otherwise.
 * When replication requirements are not met,-ENOFTREP is returned.
 */
static int notify_primary_replica(struct replica_id* primary_replica_to, struct replica_id* secondary_replica_from, struct list_head* list_secondary_replicas){
        struct collect_primary_replica_answer* answer_collection;
	int i,ret= 0;
	struct replica_id_list *entry;

	FTPRINTK("%s: thread pid %d successfully execve to a replica, going to notify primary replica pid %d in kernel %d\n", __func__, current->pid, primary_replica_to->pid, primary_replica_to->kernel);

	answer_collection= kmalloc(sizeof(*answer_collection), GFP_KERNEL);
	if(!answer_collection)
		return -ENOMEM;

	kref_init(&answer_collection->kref);
	INIT_LIST_HEAD(&answer_collection->list_member);
	answer_collection->secondary_replica= *secondary_replica_from;
	answer_collection->primary_replica= *primary_replica_to;
	answer_collection->waiting= current;
	atomic_set(&answer_collection->num_answers,0);
	answer_collection->start= 0;
	answer_collection->num_secondary_replicas= 0;
	answer_collection->secondary_replica_list= NULL;

	add_collect_primary_replica_answer(answer_collection);	

	ret= send_ack_to_primary_replica(primary_replica_to,secondary_replica_from);
	if(ret)
		goto out;

	/* From this point I already sent an ack message to the primary_replica, so if an error occurs that makes me fail the exec, mask it with -ENOFTREP,
	 * such that the caller of the do_execve will not send another update message.
	 */	

	ret= wait_for_primary_replica_answer(answer_collection);
	
	remove_collect_primary_replica_answer(answer_collection);

	if(ret){
		if(!atomic_read(&answer_collection->num_answers)){
			ret= -ENOFTREP;
			FTPRINTK("%s: thread pid %d will not start as a secondary replica\n",__func__, current->pid);
                	goto out;
		}
	}

	if(answer_collection->start == 0){
		ret= -ENOFTREP;
		FTPRINTK("%s: thread pid %d will not start as a secondary replica\n",__func__, current->pid);
		goto out;
	}
	
	for(i=0;i<answer_collection->num_secondary_replicas;i++){
		entry= kmalloc(sizeof(*entry), GFP_ATOMIC);
		if(!entry){
			//ret= -ENOMEM;
			ret= -ENOFTREP;
			goto out;
		}

		entry->replica.pid= answer_collection->secondary_replica_list[i*2];
		entry->replica.kernel= answer_collection->secondary_replica_list[i*2+1];
		INIT_LIST_HEAD(&entry->replica_list_member);

		list_add(&entry->replica_list_member,list_secondary_replicas);
	}
	
	
out:	if(answer_collection->secondary_replica_list)
		kfree(answer_collection->secondary_replica_list);

	put_collect_primary_replica_answer(answer_collection);
        return ret;
}


/* Checks if the current newly execve thread needs to be replicated. 
 * Nothing will be done if not in an active Popcorn namespace.
 *
 * Returns 0 in case no errors occured, a value < 0 otherwise.
 * When replication requirements are not met,-ENOFTREP is returned.
 * 
 * If replica_type is POTENTIAL_PRIMARY_REPLICA, it tries to create n-1 secondary replicas in other kernels, 
 * where n is the replication_degree of the namespace, and set the replica_type to PRIMARY_REPLICA in
 * case of success.    
 *
 * If replica_type is POTENTIAL_SECONDARY_REPLICA, it notifies the correlated primary replica that a secondary copy
 * was succesfully created, and set the replica_type to SECONDARY_REPLICA in case of success.
 * 
 * In both above cases, in case of success, the current's ft_popcorn field is allocated and populated 
 * with a list of all secondary replicas and the current primary replica.
 */
int maybe_create_replicas(void){
	struct popcorn_namespace *pop;
	int ret= 0;
	struct replica_id secondary;
	struct secondary_replica_request* msg;
	struct list_head *iter= NULL;
        struct replica_id_list *objPtr= NULL;
	struct ft_pop_rep* ft_popcorn;

	pop= current->nsproxy->pop_ns;

	if(is_popcorn_namespace_active(pop)){
		if(current->replica_type == POTENTIAL_PRIMARY_REPLICA){

			ft_popcorn= create_ft_pop_rep(pop->replication_degree, 1, NULL);	
				
			if(IS_ERR(ft_popcorn)){
				return PTR_ERR(ft_popcorn);
			}

			ret= create_replicas(current,pop->replication_degree, &ft_popcorn->id, &ft_popcorn->secondary_replicas_head.replica_list_member);			
			if(ret == 0){
				current->replica_type= PRIMARY_REPLICA;	
				ft_popcorn->primary_replica.pid= current->pid;
				ft_popcorn->primary_replica.kernel= _cpu;
				current->ft_popcorn= ft_popcorn;
				current->ft_pid.ft_pop_id= ft_popcorn->id;
				
				printk("%s: Replica list of %s pid %d\n", __func__, current->comm, current->pid);
				printk("primary: {pid: %d, kernel %d}, secondary: ", ft_popcorn->primary_replica.pid, ft_popcorn->primary_replica.kernel);
				list_for_each(iter, &ft_popcorn->secondary_replicas_head.replica_list_member) {
                			objPtr = list_entry(iter, struct replica_id_list, replica_list_member);
					printk("{pid: %d, kernel %d} ", objPtr->replica.pid, objPtr->replica.kernel);
				}
				printk("\n");
				
			}
			else{
				put_ft_pop_rep(ft_popcorn);
			}
			
		}
		else{
			if(current->replica_type == POTENTIAL_SECONDARY_REPLICA){
				secondary.pid= current->pid;
				secondary.kernel= _cpu;
			
				msg= (struct secondary_replica_request*) current->useful;
				current->useful= NULL;

				ft_popcorn= create_ft_pop_rep(pop->replication_degree, 0, &msg->ft_rep_id);

	                        if(IS_ERR(ft_popcorn)){
        	                        return PTR_ERR(ft_popcorn);
                	        }

				ret= notify_primary_replica(&msg->primary_replica, &secondary, &ft_popcorn->secondary_replicas_head.replica_list_member);
	                        if(ret == 0){
        	                        current->replica_type= SECONDARY_REPLICA;
					ft_popcorn->primary_replica.pid= msg->primary_replica.pid;
	                                ft_popcorn->primary_replica.kernel= msg->primary_replica.kernel;
					current->ft_popcorn= ft_popcorn;
					current->ft_pid.ft_pop_id= ft_popcorn->id;

					printk("%s: Replica list of %s pid %d\n", __func__, current->comm, current->pid);
                                	printk("primary: {pid: %d, kernel %d}, secondary: ", ft_popcorn->primary_replica.pid, ft_popcorn->primary_replica.kernel);
					list_for_each(iter, &ft_popcorn->secondary_replicas_head.replica_list_member) {
                                        	objPtr = list_entry(iter, struct replica_id_list, replica_list_member);
                                        	printk("{pid: %d, kernel %d} ", objPtr->replica.pid, objPtr->replica.kernel);
                                	}       
                                	printk("\n");
		
					pcn_kmsg_free_msg(msg);
                	        }
				else{
        	                        put_ft_pop_rep(ft_popcorn);
	                        }

		
			}
		}		
	}	

	if(current->replica_type != NOT_REPLICATED)
		printk("comm: %s pid %d\n", current->comm, current->pid);
	return ret;
}

static int __init ft_replication_init(void) {

	secondary_replica_generator_wq= create_singlethread_workqueue("secondary_replica_generator_wq");

	INIT_LIST_HEAD(&collect_primary_replica_answer_head.list_member);
	INIT_LIST_HEAD(&collect_secondary_replica_answers_head.list_member);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_SECONDARY_REPLICA_REQUEST, handle_secondary_replica_request);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_SECONDARY_REPLICA_ANSWER, handle_secondary_replica_answer);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_FT_PRIMARY_REPLICA_ANSWER, handle_primary_replica_answer);
			
	return 0;
}

late_initcall(ft_replication_init);

