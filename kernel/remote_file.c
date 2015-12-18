/*
 * remote_file.c
 *
 *  Created on: Jan 19, 2014
 *      Author: saif
 */

#include <linux/kthread.h>
#include <linux/export.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/sched.h>
#include <linux/threads.h> // NR_CPUS
#include <linux/kmod.h>
#include <linux/path.h>
#include <linux/mount.h>
#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/slab.h>
#include <linux/process_server.h>
#include <linux/mm.h>
#include <linux/io.h> // ioremap
#include <linux/mman.h> // MAP_ANONYMOUS
#include <linux/pcn_kmsg.h> // Messaging
#include <linux/string.h>
#include <linux/pid.h>

#include <asm/pgtable.h>
#include <asm/atomic.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <linux/fdtable.h> //saif
#include <popcorn/pid.h>  //saif for orig_pid
#include <linux/semaphore.h>//saif for semaphore stuff
#include <asm/mmu_context.h>
#include <popcorn/remote_file.h>

/* Locks */
DEFINE_SPINLOCK(_file_list_lock);					//Lock for file_start
DEFINE_SPINLOCK(_file_op_request_id_lock); 			//Lock for request_op_number

DEFINE_SPINLOCK(_wait_q_lock); 						//Lock for file_descriptor
DEFINE_SPINLOCK(_wait_q_file_status_lock); 			//Lock for file_status_wait
DEFINE_SPINLOCK(_wait_q_file_offset_lock); 			//Lock for file_offset_wait
DEFINE_SPINLOCK(_wait_q_file_offset_confirm_lock); 	//Lock for file_offset_confirm
DEFINE_SPINLOCK(_remote_file_info_lock);			//Lock on remote file info list// will add fine grain lock later saif



/* Extern fucntions */
extern long saif_open(char *filename, int flags, int mode, int fd,
		pid_t actual_owner);
extern long sys_lseek(unsigned int, off_t, unsigned int);
extern long sys_close(unsigned int);
extern long saif_close(int fd, struct task_struct* task);
extern long remote_thread_open(const char *filename, int flags, int mode,
		pid_t owner_pid, struct task_struct* task);

extern int _file_cpu;

static fd_wait wait_q;
static file_status_wait wait_q_file_status;
static file_offset_wait wait_q_file_offset;
static file_offset_wait wait_q_file_offset_confirm;

/* Init functions */
void file_wait_q(void)
{
	INIT_LIST_HEAD(&wait_q.list);
	INIT_LIST_HEAD(&wait_q_file_status.list);
	INIT_LIST_HEAD(&wait_q_file_offset.list);
	INIT_LIST_HEAD(&wait_q_file_offset_confirm.list);
}

long long get_filesystem_reqno(void)
{
	long long retval;
	spin_lock(&_file_op_request_id_lock);
	retval = _file_op_request_id++ % 0xFFFFFFFFFFFFFFF;
	spin_unlock(&_file_op_request_id_lock);
	return retval;
}

static void register_for_file_status(file_status_wait *strc)
{
	spin_lock(&_wait_q_file_status_lock);
	INIT_LIST_HEAD(&(strc->list));
	list_add_tail(&(strc->list), &(wait_q_file_status.list));
	spin_unlock(&_wait_q_file_status_lock);
}

/* Wait functions */
static void wait_for_file_status(file_status_wait *strc)
{
	sema_init(&(strc->file_sem), 0);
	down_interruptible(&(strc->file_sem));
}

static void register_for_file_offset_confirm(file_offset_confirm_wait *strc)
{
	spin_lock(&_wait_q_file_offset_confirm_lock);
	INIT_LIST_HEAD(&(strc->list));
	list_add_tail(&(strc->list), &(wait_q_file_offset_confirm.list));
	spin_unlock(&_wait_q_file_offset_confirm_lock);
}

static void wait_for_file_offset_confirm(file_offset_confirm_wait *strc)
{
	sema_init(&(strc->file_sem), 0);
	down_interruptible(&(strc->file_sem));
}

static void register_for_fd_ret(fd_wait *strc){
	spin_lock(&_wait_q_lock);
	INIT_LIST_HEAD(&(strc->list));
	list_add_tail(&(strc->list), &(wait_q.list));
	spin_unlock(&_wait_q_lock);
}

static void wait_for_fd_ret(fd_wait *strc)
{
	sema_init(&(strc->file_sem), 0);
	down_interruptible(&(strc->file_sem));
}

static void register_for_file_offset(file_offset_wait *strc)
{
	spin_lock(&_wait_q_file_offset_lock);
	INIT_LIST_HEAD(&(strc->list));
	list_add_tail(&(strc->list), &(wait_q_file_offset.list));
	spin_unlock(&_wait_q_file_offset_lock);
}

static void wait_for_file_offset(file_offset_wait *strc)
{
	sema_init(&(strc->file_sem), 0);
	down_interruptible(&(strc->file_sem));
}

void tell_remote_offset(int fd, struct file* file, loff_t pos,
		offset_update_type type)
{
	int tx_ret;

	file_offset_confirm *waitPtr = kmalloc(sizeof(file_offset_confirm), GFP_KERNEL);
	file_offset_update *uptPtrOwner;

	if ((current->origin_pid != file->owner_pid)
			&& (file->owner_pid != current->tgid) && (file->owner_pid != 0)
			&& current->tgroup_distributed != 0) {

		printk("Tell the owner pid %d\n",current->pid);

		uptPtrOwner = kmalloc(sizeof(file_offset_update), GFP_KERNEL);

		/* header */
		uptPtrOwner->header.from_cpu = _file_cpu;
		uptPtrOwner->header.type = PCN_KMSG_TYPE_FILE_OFFSET_UPDATE;
		uptPtrOwner->header.prio = PCN_KMSG_PRIO_HIGH;

		/* data */
		uptPtrOwner->fd = fd;
		uptPtrOwner->offset = pos;
		uptPtrOwner->owner_pid = file->owner_pid;
		uptPtrOwner->reqno = get_filesystem_reqno();
		uptPtrOwner->type = type;
		uptPtrOwner->target = FOR_OWNER;

		waitPtr->reqno = uptPtrOwner->reqno;

		tx_ret = pcn_kmsg_send(ORIG_NODE(file->owner_pid),
				(struct pcn_kmsg_long_message*) uptPtrOwner);

		kfree(waitPtr);
		kfree(uptPtrOwner);
	}
}

loff_t ask_remote_offset(int fd, struct file *file)
{
	int tx_ret;
	loff_t offset = -1;

	file_offset_req *reqPtr = kmalloc(sizeof(file_offset_req), GFP_KERNEL);
	file_offset_wait* reqWaitPtr = kmalloc(sizeof(file_offset_wait), GFP_KERNEL);

	printk("%s Pid %d\n",__func__,current->pid);

	/*header*/
	reqPtr->header.from_cpu = _file_cpu;
	reqPtr->header.type = PCN_KMSG_TYPE_FILE_OFFSET_REQUEST;
	reqPtr->header.prio = PCN_KMSG_PRIO_HIGH;
	/*data*/
	reqPtr->fd = fd;
	reqPtr->owner_pid = file->owner_pid;
	reqPtr->reqno = get_filesystem_reqno();

	reqWaitPtr->req_no = reqPtr->reqno;
	register_for_file_offset(reqWaitPtr);

	tx_ret = pcn_kmsg_send(ORIG_NODE(file->owner_pid),
			(struct pcn_kmsg_long_message*) reqPtr);

	if (tx_ret < 0)
		return -1;

	kfree(reqPtr);

	wait_for_file_offset(reqWaitPtr);

	if (reqWaitPtr->owner_pid > 0)
		offset = reqWaitPtr->offset;

	kfree(reqWaitPtr);

	return offset;
}

struct file* ask_orgin_file(int fd, pid_t orgin_pid)
{
	struct file *f = NULL;
	int tx_ret;

	file_status_req *reqPtr = kmalloc(sizeof(file_status_req), GFP_KERNEL);
	file_status_wait *reqWaitPtr = kmalloc(sizeof(file_status_wait), GFP_KERNEL);

	printk("%s\n", __func__);

	reqPtr->header.from_cpu = _file_cpu;
	reqPtr->header.type = PCN_KMSG_TYPE_FILE_STATUS_REQUEST;
	reqPtr->header.prio = PCN_KMSG_PRIO_HIGH;

	/*data*/
	reqPtr->fd = fd;
	reqPtr->original_pid = orgin_pid;
	reqPtr->reqno = get_filesystem_reqno();

	reqWaitPtr->req_no = reqPtr->reqno;

	register_for_file_status(reqWaitPtr);

	tx_ret = pcn_kmsg_send(ORIG_NODE(orgin_pid), (struct pcn_kmsg_long_message*) reqPtr);

	if (tx_ret < 0)
		return NULL;

	kfree(reqPtr);

	/* wait for it */
	wait_for_file_status(reqWaitPtr);

	/* open the file...write another open function... have to save actual
 	 * owner information */
	/* saif_open(char * filename,int flags,int mode,int fd,pid_t actual_owner) */
	if (reqWaitPtr->owner > 0) {
		f = saif_open(reqWaitPtr->name, reqWaitPtr->flags, reqWaitPtr->mode, fd,
				reqWaitPtr->owner);
	}

	kfree(reqWaitPtr);

	return f;
}

static char *get_filename_file(struct file *file, file_info_t_req *fileinfo)
{
	struct path path;
	char buffer[256];
	char *pathname;

	path = file->f_path;

	pathname = d_path(&path, buffer, sizeof(buffer));

	if (!IS_ERR(pathname)) {
		fileinfo->pos = file->f_pos;
		fileinfo->mode = file->f_omode;
		fileinfo->flags = file->f_flags;
		strcpy(fileinfo->name, pathname);
	}

	return pathname;
}

static char *get_filename(struct file *file, file_data *fileinfo)
{
	struct path path;
	char *pathname;
	char buffer[256];

	path = file->f_path;
	pathname = d_path(&path, buffer, sizeof(buffer));

	printk("%s path %s \n",__func__,pathname);

	if (!IS_ERR(pathname)) {
		fileinfo->mode = file->f_omode;
		fileinfo->flags = file->f_flags;
		strcpy(fileinfo->file_name, pathname);
	}

	return pathname;
}

/* Get the proper fd from home kernel */
int pcn_get_fd_from_home(char *tmp, int flags, fmode_t mode)
{
	int fd, tx_ret;


	file_open_req* request = kmalloc(sizeof(file_open_req), GFP_KERNEL);
	fd_wait *wait_for_fd = kmalloc(sizeof(fd_wait), GFP_KERNEL);

	printk("%s: tmp _%s_\n", __func__, tmp);

	request->header.from_cpu = _file_cpu;
	request->header.is_lg_msg = 0;
	request->header.type = PCN_KMSG_TYPE_FILE_OPEN_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_HIGH;

	request->flags = flags;
	request->mode = mode;
	request->reqno = get_filesystem_reqno();
	request->original_pid = current->tgroup_home_id;
	request->actual_owner = current->tgid;
	strcpy(request->file_name, tmp);

	/*
	 *                       USER VIEW
	 <-- PID 43 --> <----------------- PID 42 ----------------->
	 +---------+
	 | process |
	 _| pid=42  |_
	 _/ | tgid=42 | \_ (new thread) _
	 _ (fork) _/   +---------+                  \
      /                                        +---------+
	 +---------+                                    | process |
	 | process |                                    | pid=44  |
	 | pid=43  |                                    | tgid=42 |
	 | tgid=43 |                                    +---------+
	 +---------+
	 <-- PID 43 --> <--------- PID 42 --------> <--- PID 44 --->
	 KERNEL VIEW
	 *
	 * */

	wait_for_fd->req_no = request->reqno;
	register_for_fd_ret(wait_for_fd);
	tx_ret = pcn_kmsg_send_long(ORIG_NODE(current->origin_pid),
			(struct pcn_kmsg_long_message*) request,
			sizeof(file_open_req) - sizeof(request->header));

	kfree(request);
	wait_for_fd_ret(wait_for_fd);
	fd = wait_for_fd->fd;
	kfree(wait_for_fd);

	return fd;
}

int handle_file_open_reply(struct pcn_kmsg_message* inc_msg)
{
	struct list_head *pos, *q;
	fd_wait *tmp = NULL;
	file_info_t_reply *r_ptr = (file_info_t_reply*) inc_msg;

	if (r_ptr == NULL) {
		printk("%s inc_msg NULL :S \n", __func__);
	}

	if (list_empty(&wait_q.list)) {
		printk("%s Q not initialized\n", __func__);
	}

	list_for_each_safe(pos, q, &(wait_q.list)) {
		tmp = list_entry(pos, fd_wait, list);
		if (tmp != NULL) {
			printk("handle_file_open_reply File_O_R Fd %d Req %d tmp->Req %d\n",
				r_ptr->fd, r_ptr->request_id, tmp->req_no);

			if (tmp->req_no == r_ptr->request_id) {
				printk("Fd %d Req %d\n", r_ptr->fd, r_ptr->request_id);
				tmp->fd = r_ptr->fd;
				/* Reshuffle */
				list_del(pos);
				pcn_kmsg_free_msg(inc_msg);
				up(&tmp->file_sem);
				return 0;
			}
		} else{
			printk("%s List entry is NULL\n",__func__);
		}
	}

	pcn_kmsg_free_msg(inc_msg);

	return -1;
}

int handle_file_status_reply(struct pcn_kmsg_message* inc_msg)
{
	struct list_head *pos, *q;
	file_status_wait *tmp = NULL;
	file_status_reply *r_ptr = (file_status_reply*) inc_msg;

	list_for_each(pos,&(wait_q_file_status.list)) {
		tmp = list_entry(pos, file_status_wait, list);

		printk("Handle Sts reply  Req %d tmp->Req %d\n", r_ptr->reqno, tmp->req_no);

		if (tmp->req_no == r_ptr->reqno) {
			printk("FName %s flag %d mode %d\n", r_ptr->filedata.file_name,
					r_ptr->filedata.flags, r_ptr->filedata.mode);

			tmp->mode = r_ptr->filedata.mode;
			tmp->flags = r_ptr->filedata.flags;
			tmp->owner = r_ptr->owner;
			strcpy(tmp->name, r_ptr->filedata.file_name);
			list_del(pos);
			pcn_kmsg_free_msg(inc_msg);
			up(&tmp->file_sem);
			return 0;
		}
	}

	pcn_kmsg_free_msg(inc_msg);

	return -1;
}

int handle_file_offset_reply(struct pcn_kmsg_message* inc_msg)
{
	struct list_head *pos, *q;
	file_offset_wait *tmp = NULL;
	file_offset_reply *r_ptr = (file_offset_reply*) inc_msg;

	printk("Handle offet reply Req %d\n", r_ptr->reqno);

	list_for_each_safe(pos,q,&(wait_q_file_offset.list)) {
		tmp = list_entry(pos,file_offset_wait,list);

		printk("Handle Offset reply Req %d tmp->Req %d\n", r_ptr->reqno, tmp->req_no);

		if (tmp->req_no == r_ptr->reqno) {
			printk("Reply Offset %d \n", r_ptr->offset);
			tmp->offset = r_ptr->offset;
			tmp->owner_pid = r_ptr->owner_pid;

//			strcpy(tmp->name,r_ptr->filedata.file_name);

			list_del(pos);
			pcn_kmsg_free_msg(inc_msg);
			up(&tmp->file_sem);
			return 0;
		}
	}

	pcn_kmsg_free_msg(inc_msg);

	return -1;
}

int handle_file_open_request(struct pcn_kmsg_message* inc_msg)
{
//	file_open_work *workPtr=kmalloc(sizeof(file_open_work), GFP_ATOMIC);
	file_open_req *reqPtr = (file_open_req*) inc_msg;
	file_info_t_reply* replyPtr = kmalloc(sizeof(file_info_t_reply),GFP_ATOMIC);
	int tx_ret = 0;
	int fd = -1;
	struct task_struct *task, *parent;
	/*if(inc_msg==NULL)
	 {
	 return -1;
	 }*/
	printk("handl_file_o_req %d n %s\n", reqPtr->original_pid,
			reqPtr->file_name);

	task = pid_task(find_vpid(reqPtr->original_pid), PIDTYPE_PID);

	if (task != NULL) {
		fd = remote_thread_open(reqPtr->file_name, reqPtr->flags, reqPtr->mode,
				reqPtr->actual_owner, task);

	} else {
		printk("F_O_REQ Task %d is NULL\n", reqPtr->original_pid);
	}
	/*	if(fd<0)
	 {
	 return -1;
	 }
	 */
//	printk("In Handle File Open File %s pid %d\n",reqPtr->file_name,reqPtr->original_pid);
//	printk("In Handle re->pid %d found->pid %d found_par %d effP %d\n",reqPtr->original_pid,task->pid,task->real_parent->pid,task->parent->pid);
	replyPtr->header.from_cpu = _file_cpu;
	replyPtr->header.is_lg_msg = 0;
	replyPtr->header.prio = PCN_KMSG_PRIO_HIGH;
	replyPtr->header.type = PCN_KMSG_TYPE_FILE_OPEN_REPLY;
	replyPtr->fd = fd;
	replyPtr->request_id = reqPtr->reqno;

	tx_ret = pcn_kmsg_send(reqPtr->header.from_cpu,
			(struct pcn_kmsg_long_message*) replyPtr);

	pcn_kmsg_free_msg(inc_msg);

	/*	if(workPtr)
	 {
	 INIT_WORK((struct work_struct* )workPtr, file_open_request);
	 workPtr->flags=reqPtr->flags;
	 workPtr->mode=reqPtr->mode;
	 workPtr->original_pid=reqPtr->original_pid;
	 queue_work( file_open_wq, (struct work_struct *)workPtr);
	 }
	 */

}

int handle_file_status_request(struct pcn_kmsg_message* inc_msg)
{
//	file_open_work *workPtr=kmalloc(sizeof(file_open_work), GFP_ATOMIC);
	file_status_req *reqPtr = (file_open_req*) inc_msg;
	file_status_reply replyPtr;
	struct files_struct * tsk_ftable = NULL;
	struct file * filePtr = NULL;
	int tx_ret = 0;
	int fd;
	struct task_struct *task, *parent;
//	if(inc_msg==NULL)
//	{
//		return -1;
//	}
	task = pid_task(find_vpid(reqPtr->original_pid), PIDTYPE_PID);
	printk("Handle Sts request o_pid %d \n", reqPtr->original_pid);
	if (task != NULL) {
		tsk_ftable = task->files;
		filePtr = fcheck_files(tsk_ftable, reqPtr->fd);
		if (filePtr != NULL) {
			get_filename(filePtr, &(replyPtr.filedata));
			replyPtr.owner = filePtr->owner_pid;
			printk("H_STS_REQ % owner %d\n", filePtr->owner_pid);

		} else if (task->fake_file_table[reqPtr->fd] != NULL) {
			strcpy(replyPtr.filedata.file_name,
					task->fake_file_table[reqPtr->fd]->filename);
			replyPtr.filedata.flags = task->fake_file_table[reqPtr->fd]->flags;
			replyPtr.filedata.mode = task->fake_file_table[reqPtr->fd]->mode;
			replyPtr.owner = task->fake_file_table[reqPtr->fd]->owner_pid;
			printk("H_STS_REQ % owner %d\n", filePtr->owner_pid);
		} else {

		}
		//	printk("In Handle re->pid %d found->pid %d found_par %d effP %d\n",reqPtr->original_pid,task->pid,task->real_parent->pid,task->parent->pid);
	} else {
		printk("F_STS_REQ Task %d is NULL\n", reqPtr->original_pid);
	}
	replyPtr.header.from_cpu = _file_cpu;
	replyPtr.header.is_lg_msg = 1;
	replyPtr.header.prio = PCN_KMSG_PRIO_HIGH;
	replyPtr.header.type = PCN_KMSG_TYPE_FILE_STATUS_REPLY;
	replyPtr.reqno = reqPtr->reqno;

//	 pcn_kmsg_send(reqPtr->header.from_cpu,
//					(struct pcn_kmsg_long_message*) replyPtr);
	tx_ret = pcn_kmsg_send_long(reqPtr->header.from_cpu,
			(struct pcn_kmsg_long_message*) &replyPtr,
			sizeof(file_status_reply) - sizeof(struct pcn_kmsg_hdr));

	pcn_kmsg_free_msg(inc_msg);

	/*	if(workPtr)
	 {
	 INIT_WORK((struct work_struct* )workPtr, file_open_request);
	 workPtr->flags=reqPtr->flags;
	 workPtr->mode=reqPtr->mode;
	 workPtr->original_pid=reqPtr->original_pid;
	 queue_work( file_open_wq, (struct work_struct *)workPtr);
	 }
	 */

}

int handle_file_offset_request(struct pcn_kmsg_message* inc_msg)
{
//	file_open_work *workPtr=kmalloc(sizeof(file_open_work), GFP_ATOMIC);
	file_offset_req *reqPtr = (file_offset_req*) inc_msg;
	file_offset_reply replyPtr;
	struct files_struct * tsk_ftable = NULL;
	struct file * filePtr = NULL;
	int tx_ret = 0;
	int fd;
	struct task_struct *task, *parent;
//	printk("Handle offset request \n");
//	if(inc_msg==NULL)
//	{
//		return -1;
//	}
	task = pid_task(find_vpid(reqPtr->owner_pid), PIDTYPE_PID);
	printk("Handle offset request o_pid %d ", reqPtr->owner_pid);
	if (task != NULL) {
		tsk_ftable = task->files;
		filePtr = fcheck_files(tsk_ftable, reqPtr->fd);
		if (filePtr != NULL) {
			replyPtr.offset = filePtr->f_pos;
			replyPtr.owner_pid = filePtr->owner_pid;
		} else {
			replyPtr.offset = -1;
			replyPtr.owner_pid = -1;
		}
	} else {
		printk("F_OFF_REQ Task %d is NULL\n", reqPtr->owner_pid);
	}
	replyPtr.header.from_cpu = _file_cpu;
	replyPtr.header.prio = PCN_KMSG_PRIO_HIGH;
	replyPtr.header.type = PCN_KMSG_TYPE_FILE_OFFSET_REPLY;
	replyPtr.reqno = reqPtr->reqno;
	printk("handle_file_offset_request Dest CPU %d \n",
			reqPtr->header.from_cpu);
//	 pcn_kmsg_send(reqPtr->header.from_cpu,
//					(struct pcn_kmsg_long_message*) replyPtr);
	tx_ret = pcn_kmsg_send_long(reqPtr->header.from_cpu,
			(struct pcn_kmsg_long_message*) &replyPtr,
			sizeof(file_offset_reply) - sizeof(struct pcn_kmsg_hdr));

	pcn_kmsg_free_msg(inc_msg);

	/*	if(workPtr)
	 {
	 INIT_WORK((struct work_struct* )workPtr, file_open_request);
	 workPtr->flags=reqPtr->flags;
	 workPtr->mode=reqPtr->mode;
	 workPtr->original_pid=reqPtr->original_pid;
	 queue_work( file_open_wq, (struct work_struct *)workPtr);
	 }
	 */

}

int handle_file_close_notification(struct pcn_kmsg_message* inc_msg)
{
	/*	file_open_work *workPtr=kmalloc(sizeof(file_open_work), GFP_ATOMIC);
	 file_open_req *reqPtr=(file_open_req*)inc_msg;
	 if(inc_msg==NULL)
	 {
	 return -1;
	 }
	 if(workPtr)
	 {
	 INIT_WORK((struct work_struct* )workPtr, file_open_notification);
	 workPtr->flags=reqPtr->flags;
	 workPtr->mode=reqPtr->mode;
	 workPtr->original_pid=reqPtr->original_pid;
	 queue_work( file_open_wq, (struct work_struct *)workPtr);
	 }
	 */
}
int handle_file_pos_confirm(struct pcn_kmsg_message* inc_msg)
{
	struct list_head *pos, *q;
	file_offset_confirm_wait *tmp = NULL;
	file_offset_confirm *r_ptr = (file_offset_reply*) inc_msg;
	printk("handle_file_pos_confirm Req %d\n", r_ptr->reqno); //somthing is wrong with printk it works :(
	list_for_each_safe(pos,q,&(wait_q_file_offset_confirm.list))
	{
		tmp = list_entry(pos,file_offset_confirm_wait,list);
		//printk("Handle Sts reply \n");
		printk("Handle Offset reply Req %d tmp->Req %d\n", r_ptr->reqno,
				tmp->reqno);
		if (tmp->reqno == r_ptr->reqno) {
			tmp->new_node = r_ptr->new_node;
			tmp->reqno = r_ptr->reqno;
			tmp->status = r_ptr->status;
			//			strcpy(tmp->name,r_ptr->filedata.file_name);
			list_del(pos);
			pcn_kmsg_free_msg(inc_msg);
			up(&tmp->file_sem);
			return 0;
		}
	}
	pcn_kmsg_free_msg(inc_msg);
	return -1;

}

int handle_file_pos_update(struct pcn_kmsg_message* inc_msg)
{
	file_offset_update *uptPtr = (file_offset_update*) inc_msg;
	file_offset_confirm confPtr;
	offset_confirm_sts status = OFFSET_UPDATE_SUCC;
	struct task_struct *task;
	struct files_struct * tsk_ftable = NULL;
	struct file * filePtr = NULL;
	int tx_ret;
	task = pid_task(find_vpid(uptPtr->owner_pid), PIDTYPE_PID);

	printk("Handle FILE_POS update o_pid %d t_pid %d", uptPtr->owner_pid);
	if (task != NULL) {
		if (task->fake_file_table[uptPtr->fd] != NULL) {
			if (uptPtr->offset < task->fake_file_table[uptPtr->fd]->pos) {
				if (uptPtr->type == LSEEK_UPDATE) {
					task->fake_file_table[uptPtr->fd]->pos = uptPtr->offset;
				} else {
					status = OFFSET_UPDATE_IGN;
				}
			} else {
				task->fake_file_table[uptPtr->fd]->pos = uptPtr->offset;
			}
		} else {
			tsk_ftable = task->files;

			filePtr = fcheck_files(tsk_ftable, uptPtr->fd);
			if (filePtr != NULL) {

				if (uptPtr->offset < filePtr->f_pos) {
					if (uptPtr->type == LSEEK_UPDATE) {
						filePtr->f_pos = uptPtr->offset;
					} else {
						status = OFFSET_UPDATE_IGN;
					}
				} else {
					filePtr->f_pos = uptPtr->offset;
				}
			}
		}
	} else {
		printk("FILE_POS Task is NULL\n");
		status = OFFSET_UPDATE_FAIL;
	}
	confPtr.header.from_cpu = _file_cpu;
	confPtr.header.prio = PCN_KMSG_PRIO_HIGH;
	confPtr.header.type = PCN_KMSG_TYPE_FILE_OFFSET_CONFIRM;
	confPtr.reqno = uptPtr->reqno;
	confPtr.status = status;
	confPtr.new_node = 0;
	tx_ret = pcn_kmsg_send_long(uptPtr->header.from_cpu,
			(struct pcn_kmsg_long_message*) &confPtr,
			sizeof(file_offset_confirm) - sizeof(struct pcn_kmsg_hdr));
	pcn_kmsg_free_msg(inc_msg);
	return 0;
}
