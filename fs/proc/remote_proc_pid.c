#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/pcn_kmsg.h>
#include <linux/delay.h>
#include <linux/string.h>

#include <popcorn/remote_proc_pid.h>
//#include <popcorn/pid.h>
#define POPCORN_MAX_KERNEL_ID  15
#define GLOBAL_PID_MASK PID_MAX_LIMIT
#define PID_NODE_SHIFT POPCORN_MAX_KERNEL_ID
#define INTERNAL_PID_MASK (PID_MAX_LIMIT - 1)

#define GLOBAL_PID_NODE(pid, node) \
	(((node) << PID_NODE_SHIFT)|GLOBAL_PID_MASK|((pid) & INTERNAL_PID_MASK))

#define SHORT_PID(pid) ((pid) & INTERNAL_PID_MASK)
#define ORIG_PID(pid) (SHORT_PID(pid) & ((1<<POPCORN_MAX_KERNEL_ID)-1))
#define ORIG_NODE(pid) (SHORT_PID(pid) >> PID_NODE_SHIFT)

#define SIZE 250

static int _cpu=-1;
static int wait=-1;

// Exec list
static struct workqueue_struct *clone_wq;
static struct workqueue_struct *exit_wq;

unsigned long pid_arr[SIZE];
int pid_count=0;


#define PROC_MAXPIDS 100


struct _remote_pid_request {
    struct pcn_kmsg_hdr header;
    char pad_string[60];
} __attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_pid_request _remote_pid_request_t;

struct _remote_pid_response {
    struct pcn_kmsg_hdr header;
    unsigned long remote_pid[SIZE];
    int count;
   } __attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_pid_response _remote_pid_response_t;



static int handle_remote_pid_response(struct pcn_kmsg_message* inc_msg) {
	_remote_pid_response_t* msg = (_remote_pid_response_t*) inc_msg;

	printk("%s: response --- pid stored - pid{%d} \n","handle_remote_pid_response", msg->count);

	pid_count = msg->count;
	//pid_arr = kmalloc(sizeof(long) * SIZE, GFP_KERNEL);

	memcpy(&pid_arr, &msg->remote_pid, SIZE*sizeof(long));

	wait = 1;

	printk("%s: response --- pid stored - wait{%d} \n",
			"handle_remote_pid_response", wait);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

static int handle_remote_pid_request(struct pcn_kmsg_message* inc_msg) {

	_remote_pid_request_t* msg = (_remote_pid_request_t*) inc_msg;
	_remote_pid_response_t response;
	unsigned long *pids;
	int count = 0;

	printk("%s: request -- entered \n", "handle_remote_pid_request");

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_PID_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	response.count = iterate_process(&pid_arr);
	memcpy(&response.remote_pid, &pid_arr, SIZE*sizeof(long));


	printk("%s: request --remote:pid count : %d \n",
			"handle_remote_pid_request", response.count);
	// Send response
	pcn_kmsg_send_long(msg->header.from_cpu, (struct pcn_kmsg_message*) (&response),
			sizeof(_remote_pid_response_t) - sizeof(struct pcn_kmsg_hdr));

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}


struct remote_pid_entry {
	char *name;
	int len;
	mode_t mode;
	const struct inode_operations *iop;
	const struct file_operations *fop;
	union proc_remote_op op;
};

static
unsigned int remote_pid_entry_count_dirs(const struct remote_pid_entry *entries,
				      unsigned int n)
{
	unsigned int i;
	unsigned int count;

	count = 0;
	for (i = 0; i < n; ++i) {
		if (S_ISDIR(entries[i].mode))
			++count;
	}

	return count;
}

struct proc_remote_pid_info *get_remote_proc_task(struct inode *inode)
{
	return &PROC_I(inode)->remote_proc;
}

static struct inode *remote_proc_pid_make_inode(struct super_block *sb,
					     struct proc_remote_pid_info *task)
{
	struct inode *inode;
	struct proc_remote_pid_info *ei;

	/* We need a new inode */

	inode = new_inode(sb);
	if (!inode)
		goto out;

	/* Common stuff */
	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
	inode->i_op = &proc_def_inode_operations;

	ei = get_remote_proc_task(inode);
	//krg_task_get(task->task_obj);
	*ei = *task;

	inode->i_uid = 0;
	inode->i_gid = 0;

out:
	return inode;
}

static struct remote_pid_entry remote_tgid_base_stuff[] = {

};
typedef struct dentry *instantiate_t(struct inode *, struct dentry *,
				     struct proc_remote_pid_info *,
				     const void *);



static int remote_pid_revalidate(struct dentry *dentry, struct nameidata *nd)
{
	/* We need to check that the pid still exists in the system */
	/*struct inode *inode = dentry->d_inode;
	struct proc_distant_pid_info *ei = get_krg_proc_task(inode);
	struct task_kddm_object *obj;
	long state = EXIT_DEAD;
*/
	/*
	 * Optimization: avoid doing krg_task_readlock() when it is obviously
	 * useless.
	 */
/*	if (!krg_task_alive(ei->task_obj))
		goto drop;*/
	/* If pid is reused in between, the former task_obj field is dead. */
	/*obj = krg_task_readlock(ei->pid);
	if (!krg_task_alive(ei->task_obj) || !obj)
		goto unlock;

	BUG_ON(obj != ei->task_obj);
	state = obj->exit_state;
	if (obj->node == kerrighed_node_id)*/
		/*
		 * The task is probably not dead, but we want the dentry
		 * to be regenerated with vanilla procfs operations.
		 */
	/*	state = EXIT_DEAD;
	if (state != EXIT_DEAD) {
		ei->dumpable = obj->dumpable;
		if ((inode->i_mode == (S_IFDIR|S_IRUGO|S_IXUGO)) ||
		    obj->dumpable) {
			inode->i_uid = obj->euid;
			inode->i_gid = obj->egid;
		} else {
			inode->i_uid = 0;
			inode->i_gid = 0;
		}
		inode->i_mode &= ~(S_ISUID | S_ISGID);
	}

unlock:
	krg_task_unlock(ei->pid);

	if (state != EXIT_DEAD)
		return 1;
drop:
	d_drop(dentry);*/
	return 0;
}

static int remote_pid_delete_dentry(struct dentry *dentry)
{
	//struct proc_distant_pid_info *ei = get_krg_proc_task(dentry->d_inode);

	/*
	 * If the task is local, we want the dentry to be regenerated with
	 * vanilla procfs operations.
	 */
	/*if (!krg_task_alive(ei->task_obj)
	    || ei->task_obj->node == kerrighed_node_id)
		return 1;*/

	return 0;
}
static struct dentry_operations remote_pid_dentry_operations = {
	.d_revalidate = remote_pid_revalidate,
	.d_delete = remote_pid_delete_dentry,
};



static int remote_proc_fill_cache(struct file *filp,
			       void *dirent, filldir_t filldir,
			       char *name, int len,
			       instantiate_t instantiate,
			       struct proc_remote_pid_info *task,
			       const void *ptr)
{
	struct dentry *child, *dir = filp->f_path.dentry;
	struct inode *inode;
	struct qstr qname;
	ino_t ino = 0;
	unsigned type = DT_UNKNOWN;

	qname.name = name;
	qname.len  = len;
	qname.hash = full_name_hash(name, len);

	child = d_lookup(dir, &qname);
	if (!child) {
		struct dentry *new;
		new = d_alloc(dir, &qname);
		if (new) {
			child = instantiate(dir->d_inode, new, task, ptr);
			if (child)
				dput(new);
			else
				child = new;
		}
	}
	if (!child || IS_ERR(child) || !child->d_inode)
		goto end_instantiate;
	inode = child->d_inode;
	if (inode) {
		ino = inode->i_ino;
		type = inode->i_mode >> 12;
	}
	dput(child);
end_instantiate:
	if (!ino)
		ino = find_inode_number(dir, &qname);
	if (!ino)
		ino = 1;
	return filldir(dirent, name, len, filp->f_pos, ino, type);
}


static int remote_proc_pident_readdir(struct file *filp,
				   void *dirent, filldir_t filldir,
				   const struct remote_pid_entry *ents,
				   unsigned int nents)
{
return 0;
}

static int remote_proc_tgid_base_readdir(struct file *filp,
				      void *dirent, filldir_t filldir)
{
	return remote_proc_pident_readdir(filp, dirent, filldir,
				       remote_tgid_base_stuff,
				       ARRAY_SIZE(remote_tgid_base_stuff));
}

static struct file_operations remote_proc_tgid_base_operations = {
	.read		= generic_read_dir,
	.readdir	= remote_proc_tgid_base_readdir,
};



struct dentry *remote_proc_pident_lookup(struct inode *dir,
				      struct dentry *dentry,
				      const struct remote_pid_entry *ents,
				      unsigned int nents)
{
	return 0;
}
static struct dentry *remote_proc_tgid_base_lookup(struct inode *dir,
						struct dentry *dentry,
						struct nameidata *nd)
{
	return remote_proc_pident_lookup(dir, dentry,
				      remote_tgid_base_stuff,
				      ARRAY_SIZE(remote_tgid_base_stuff));
}

static int remote_pid_getattr(struct vfsmount *mnt, struct dentry *dentry,
			   struct kstat *stat)
{
	struct inode *inode = dentry->d_inode;
//	struct proc_distant_pid_info *task;

	generic_fillattr(inode, stat);

	stat->uid = 0;
	stat->gid = 0;
	/*task = get_proc_task(inode);
	if (task_alive(task->task_obj)) {
		if ((inode->i_mode == (S_IFDIR|S_IRUGO|S_IXUGO)) ||
		    task->dumpable) {
			stat->uid = task->euid;
			stat->gid = task->egid;
		}
	}*/

	return 0;
}

static struct inode_operations remote_proc_tgid_base_inode_operations = {
	.lookup = remote_proc_tgid_base_lookup,
	.getattr = remote_pid_getattr,
	.setattr = proc_setattr,
};


static
struct dentry *remote_proc_pid_instantiate(struct inode *dir,
					struct dentry *dentry,
					struct proc_remote_pid_info *task,
					const void *ptr)
{
	struct dentry *error = ERR_PTR(-ENOENT);
	struct inode *inode;

	inode =	remote_proc_pid_make_inode(dir->i_sb, task);
	if (!inode)
		goto out;

	inode->i_mode = S_IFDIR|S_IRUGO|S_IXUGO;
	inode->i_op = &remote_proc_tgid_base_inode_operations;
	inode->i_fop = &remote_proc_tgid_base_operations;
	inode->i_flags |= S_IMMUTABLE;

	//inode->i_nlink = 2 + remote_pid_entry_count_dirs(remote_tgid_base_stuff,
	//					      ARRAY_SIZE(remote_tgid_base_stuff));
	//inode->i_nlink=0;
	dentry->d_op = &remote_pid_dentry_operations;

	d_add(dentry, inode);

	error = NULL;
out:
	return error;
}


struct dentry *remote_proc_pid_lookup(struct inode *dir,
				   struct dentry *dentry, pid_t tgid)
{
	/* try and locate pid in the cluster */
	struct dentry *result = ERR_PTR(-ENOENT);
	struct proc_remote_pid_info task;


	task.pid = tgid;
	task.euid = 0;
	task.egid = 0;
	task.Kernel_Num = 3;

	result = remote_proc_pid_instantiate(dir, dentry, &task, NULL);

	return result;
}


static int remote_proc_pid_fill_cache(int Kernel_id,struct file *filp,
				   void *dirent, filldir_t filldir,
				   struct tgid_iter iter)
{
	char name[PROC_NUMBUF];
	int len = snprintf(name, sizeof(name), "%d", iter.tgid);
	struct proc_remote_pid_info proc_task;

	int retval = 0;

		proc_task.pid = iter.tgid;
		proc_task.Kernel_Num = Kernel_id;
		proc_task.euid = 0;
		proc_task.egid = 0;

		retval = remote_proc_fill_cache(filp, dirent, filldir, name, len,
					     remote_proc_pid_instantiate,
					     &proc_task, NULL);


	return retval;
}

int send_request_to_remote(int KernelId) {

	_remote_pid_request_t* request = kmalloc(sizeof(_remote_pid_request_t),
	GFP_KERNEL);
	// Build request
	request->header.type = PCN_KMSG_TYPE_REMOTE_PID_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;

	// Send response
	pcn_kmsg_send(KernelId, (struct pcn_kmsg_message*) (request));
	return 0;
}

int iterate_process(unsigned long *pids) {
	struct task_struct *p, *g;
	int count = 0;
	if (_cpu == 3) {
		//pids = kmalloc(sizeof(long) * SIZE, GFP_KERNEL);
		read_lock(&tasklist_lock);
		do_each_thread(g, p)
		{
			pids[count] = p->pid;
			count++;
		}
		while_each_thread(g, p);
		read_unlock(&tasklist_lock);
	}

	return count;

}

int fill_next_remote_tgids(int Kernel_id, struct file *filp,
		void *dirent, filldir_t filldir, loff_t offset) {
	struct tgid_iter iter;

	int result = 0;
	int i;
	int retval;

	result = send_request_to_remote(Kernel_id);

	while (wait == -1) {
		msleep(2);
		printk("fill_next_remote_tgids: waiting for the response");
	}
	wait = -1;


	for (i = 0; i < pid_count; i++) {
		iter.tgid = pid_arr[i];
		filp->f_pos = iter.tgid  + offset;
		iter.task = NULL;
		retval = remote_proc_pid_fill_cache(Kernel_id,filp, dirent, filldir, iter);

		if (retval < 0) {
			retval = -EAGAIN;
			return retval;
		}
	}
	retval = pid_count+1 ;//< ARRAY_SIZE(pid_arr) ? 0 : pid_count;

	return retval;

}

int fill_next_tgids(int Kernel_id, struct file *filp, void *dirent,
		filldir_t filldir, loff_t offset) {
	pid_t tgid;
	int retval;

	//do {
		retval = fill_next_remote_tgids(Kernel_id, filp, dirent, filldir,offset);
		if (retval > 0) {
			tgid = filp->f_pos - offset;
			if ((tgid & INTERNAL_PID_MASK) >= PID_MAX_LIMIT - 1) {
				retval = 0;
				//break;
			}
			filp->f_pos++;
		}
	//} while (retval > 0);

	return retval;
}

int remote_proc_pid_readdir(struct file *filp, void *dirent, filldir_t filldir,
		loff_t offset) {
	pid_t tgid;
	int node;
	int retval = 0;

	tgid = filp->f_pos - offset;
	int global = (tgid & GLOBAL_PID_MASK);

	if (!global) {
		tgid = GLOBAL_PID_NODE(0, 0);
		filp->f_pos = tgid + offset;
	}
	node = ORIG_NODE(tgid);
	printk("remote_proc_pid : Kernel ID: %d\n", node);
//	for (; node <Kernel_Id ; have to query the entire active kernels
//	     node++,
	filp->f_pos = GLOBAL_PID_NODE(0, 3) + offset;

	retval = fill_next_tgids(3, filp, dirent, filldir, offset);

	return retval;

}
static int __init pid_handler_init(void)
{


    _cpu = smp_processor_id();

	 clone_wq = create_workqueue("clone_wq");
	 exit_wq  = create_workqueue("exit_wq");

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_PID_REQUEST,
				    		handle_remote_pid_request);


	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_PID_RESPONSE,
				    		handle_remote_pid_response);
	return 0;
}
/**
 * Register process server init function as
 * module initialization function.
 */
late_initcall(pid_handler_init);
