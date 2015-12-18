/*
 * remote_file.h
 *
 *  Created on: Jan 19, 2014
 *      Author: saif
 */


#include <linux/types.h>
#include <linux/pcn_kmsg.h>

#ifndef REMOTE_FILE_H_
#define REMOTE_FILE_H_

/* Data structures */

typedef struct _file_offset_req {
	struct pcn_kmsg_hdr header;
	unsigned int owner_pid;
	long long reqno;
	int fd;
} file_offset_req;

typedef struct _file_offset_wait {
	struct list_head list;
	struct semaphore file_sem;
	unsigned int owner_pid;
	long long req_no;
	loff_t offset;
} file_offset_wait;

typedef struct _file_offset_reply {
	struct pcn_kmsg_hdr header;
	unsigned int owner_pid;
	long long reqno;
	loff_t offset;
} file_offset_reply;

typedef struct _file_offset_update {
	struct pcn_kmsg_hdr header;
	unsigned int owner_pid;
	int fd;
	long long reqno;
	loff_t offset;
	offset_update_type type;
	for_who target;
} file_offset_update;

typedef enum _offset_confirm_sts {
	OFFSET_UPDATE_SUCC,
	OFFSET_UPDATE_FAIL,
	OFFSET_UPDATE_IGN,
	OFFSET_UPDATE_OWNER_MIG
} offset_confirm_sts;

typedef struct _file_offset_confirm {
	struct pcn_kmsg_hdr header;
	offset_confirm_sts status;
	unsigned int new_node;
	long long reqno;
} file_offset_confirm;

typedef struct _file_offset_confirm_wait {
	struct list_head list;
	struct semaphore file_sem;
	offset_confirm_sts status;
	unsigned int new_node;
	long long reqno;
} file_offset_confirm_wait;

typedef struct _file_data {
	fmode_t mode;
	int flags;
	char file_name[256];
} file_data;

typedef struct _fd_wait {
	struct list_head list;
	struct semaphore file_sem;
	long long req_no;
	int fd;
} fd_wait;

typedef struct _file_status_wait {
	struct list_head list;
	struct semaphore file_sem;
	long long req_no;
	char name[256];
	unsigned int flags;
	fmode_t mode;
	pid_t owner;
} file_status_wait;

typedef struct _file_req {
	struct pcn_kmsg_hdr header;
	int clone_request_id;
	char name[256];
	int fd;
	unsigned int flags;
	fmode_t mode;
	off_t pos;
} file_info_t_req;

typedef struct _file_info_reply {
	struct pcn_kmsg_hdr header;
	int fd;
	int request_id;
} file_info_t_reply;

typedef struct _file_info {
	struct pcn_kmsg_hdr header;
	int requesting_cpu;
	int clone_request_id;
	char name[256];
	int fd;
	unsigned int flags;
	fmode_t mode;
	off_t pos;
} file_info_t;

typedef struct _file_list_node {
	struct _file_list_node* next;
	struct _file_list_node* prev;
	file_info_t data;
} file_node;

typedef struct _file_open_req {
	struct pcn_kmsg_hdr header;
	unsigned int original_pid;
	char file_name[256];
	fmode_t mode;
	pid_t actual_owner;
	long long reqno;
	int flags;
} file_open_req;

typedef struct _file_status_reply {
	struct pcn_kmsg_hdr header;
	file_data filedata;
	pid_t owner;
	long long reqno;
} file_status_reply;

typedef struct _file_status_req {
	struct pcn_kmsg_hdr header;
	unsigned int original_pid;
	long long reqno;
	int fd;
} file_status_req;

typedef struct _file_open_work {
	struct work_struct my_work;
	unsigned int original_pid;
	char file_name[256];
	fmode_t mode;
	int flags;
} file_open_work;

typedef struct _file_close_req {
	struct pcn_kmsg_hdr header;
	unsigned int original_pid;
	int fd;
} file_close_req;

typedef struct _file_close_work {
	struct work_struct my_work;
	unsigned int original_pid;
	int fd;
} file_close_work;

typedef struct _pos_update_req {
	struct pcn_kmsg_hdr header;
	unsigned int original_pid;
	off_t pos;
} file_pos_update;

typedef struct _pos_update_work {
	struct work_struct my_work;
	unsigned int original_pid;
	off_t pos;
} file_pos_update_work;

static int _file_op_request_id = 0;

/* Fucntion prototypes */

struct file* ask_orgin_file(int fd, pid_t orgin_pid);
loff_t ask_remote_offset(int fd, struct file *file);
void tell_remote_offset(int fd, struct file *file, loff_t pos, offset_update_type type);
long long get_filesystem_reqno(void);
int pcn_get_fd_from_home(char *tmp, int flags, fmode_t mode);
static char* get_filename_file(struct file *file, file_info_t_req *fileinfo);
static char* get_filename(struct file *file, file_data *fileinfo);

/* Handler functions */
/* Reply handlers */
int handle_file_open_reply(struct pcn_kmsg_message *inc_msg);
int handle_file_status_reply(struct pcn_kmsg_message *inc_msg);
int handle_file_offset_reply(struct pcn_kmsg_message *inc_msg);
int handle_file_pos_confirm(struct pcn_kmsg_message *inc_msg);
/* Request handlers */
int handle_file_open_request(struct pcn_kmsg_message *inc_msg);
int handle_file_status_request(struct pcn_kmsg_message *inc_msg);
int handle_file_offset_request(struct pcn_kmsg_message *inc_msg);
int handle_file_close_notification(struct pcn_kmsg_message *inc_msg);
int handle_file_pos_update(struct pcn_kmsg_message *inc_msg);
int change_dir_remote(const char *path, struct task_struct *task);

/* Init function */
void file_wait_q(void);

#endif /* REMOTE_FILE_H_ */
