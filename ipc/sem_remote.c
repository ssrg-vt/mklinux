#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/pcn_kmsg.h>
#include <linux/string.h>
#include <linux/wait.h>

#include <linux/ipc.h>
#include "sem_remote.h"
#include "util.h"




static int _cpu=-1;
static int wait=-1;
static int errnum=0;

static DECLARE_WAIT_QUEUE_HEAD(wq);

/*
 ******************************************************** common functions*******************************************************************
 */


int reset_wait()
{
	wait=-1;
	return 0;
}

/*
 ******************************************************** semget message functions*******************************************************************
 */


struct _remote_ipc_semget_request {
    struct pcn_kmsg_hdr header;
    struct ipc_params _param;
    char pad_string[44];
} __attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_ipc_semget_request _remote_ipc_semget_request_t;

struct _remote_ipc_semget_response {
    struct pcn_kmsg_hdr header;
    int errno;
    char pad_string[56];
   } __attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_ipc_semget_response _remote_ipc_semget_response_t;


struct kern_ipc_perm *remote_ipc_findkey(struct ipc_ids *ids, key_t key)
{
	struct kern_ipc_perm *ipc;
	int next_id;
	int total;

	for (total = 0, next_id = 0; total < ids->in_use; next_id++) {
		ipc = idr_find(&ids->ipcs_idr, next_id);

		if (ipc == NULL)
			continue;

		if (ipc->key != key) {
			total++;
			continue;
		}
		return ipc;
	}

	return NULL;
}

int remote_semget( struct ipc_params *params)
{
		struct ipc_namespace *ns;
		struct ipc_ops sem_ops;

		struct kern_ipc_perm *perm;
		int err = 0;

		ns = current->nsproxy->ipc_ns;

		sem_ops.getnew = newary;
		sem_ops.associate = sem_security;
		sem_ops.more_checks = sem_more_checks;

		perm=remote_ipc_findkey(&sem_ids(ns),params->key);
		if(perm!=NULL)
		{
			err=sem_more_checks(perm,params);
			if (!err)
				err = ipc_check_perms(ns, perm, &sem_ops, params);

		return GLOBAL_IPCID(err);
		}
		return 0;
}


static int handle_remote_ipc_semget_response(struct pcn_kmsg_message* inc_msg) {
	_remote_ipc_semget_response_t* msg = (_remote_ipc_semget_response_t*) inc_msg;

	printk("%s: response --- errno{%d} \n","handle_remote_ipc_semget_response", msg->errno);

	wait = 1;
	errnum= msg->errno;
	wake_up_interruptible(&wq);
	printk("%s: response --- pid stored - wait{%d} \n",
			"handle_remote_pid_response", wait);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

static int handle_remote_ipc_semget_request(struct pcn_kmsg_message* inc_msg) {

	_remote_ipc_semget_request_t* msg = (_remote_ipc_semget_request_t*) inc_msg;
	_remote_ipc_semget_response_t response;

	int errno=0;
	printk("%s: request -- entered \n", "handle_remote_ipc_semget_request");

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_IPC_SEMGET_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	errno = remote_semget(&(msg->_param));

	response.errno=errno;

	printk("%s: request -- errno %d \n", "handle_remote_ipc_semget_request",response.errno);
	// Send response
	pcn_kmsg_send(msg->header.from_cpu, (struct pcn_kmsg_message*) (&response));

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}


int send_semget_req_to_remote(int KernelId,  struct ipc_params *params) {

	int res=0;
	_remote_ipc_semget_request_t* request = kmalloc(sizeof(_remote_ipc_semget_request_t),
	GFP_KERNEL);
	// Build request
	request->header.type = PCN_KMSG_TYPE_REMOTE_IPC_SEMGET_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
	request->_param = *params;
	//request->nsems=params->u.nsems;
	// Send response
	res=pcn_kmsg_send(KernelId, (struct pcn_kmsg_message*) (request));
	return res;
}


int remote_ipc_sem_getid(struct ipc_ids *ids, struct ipc_params *params)
{

	reset_wait();
	int ret=0;
	/*
	 * have to send message to remote kernels and fetch if the key is present
	 */
	long *key_index;
	int id = -1;

	key_index = 1;

	if(_cpu==0)
	id = send_semget_req_to_remote(3,params);
	else
	id = send_semget_req_to_remote(0,params);

	printk("remote_ipc_sem_findkey: go to sleep!!!!");
	wait_event_interruptible(wq, wait != -1);

	ret=errnum;

	return ret;
}


/*
 ******************************************************** semgctl message functions*******************************************************************
 */

struct _remote_ipc_semctl_request {
    struct pcn_kmsg_hdr header;
    int _semnum;
    int _semid;
    int _cmd;
    int _version;
    union semun _arg;
} __attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_ipc_semctl_request _remote_ipc_semctl_request_t;

struct _remote_ipc_semctl_response {
    struct pcn_kmsg_hdr header;
    int errno;
    char pad_string[56];
   } __attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_ipc_semctl_response _remote_ipc_semctl_response_t;




int remote_semctl(int semnum, int semid, int cmd, int version, union semun arg)
{
		struct ipc_namespace *ns;
		int err = -EINVAL;

		ns = current->nsproxy->ipc_ns;

		switch(cmd) {
			case IPC_INFO:
			case SEM_INFO:
			case IPC_STAT:
			case SEM_STAT:
				err = semctl_nolock(ns, semid, cmd, version, arg);
				return err;
			case GETALL:
			case GETVAL:
			case GETPID:
			case GETNCNT:
			case GETZCNT:
			case SETVAL:
			case SETALL:
				err = semctl_main(ns,semid,semnum,cmd,version,arg);
				return err;
			case IPC_RMID:
			case IPC_SET:
				err = semctl_down(ns, semid, cmd, version, arg);
				return err;
			default:
				return -EINVAL;
			}

}


static int handle_remote_ipc_semctl_response(struct pcn_kmsg_message* inc_msg) {
	_remote_ipc_semctl_response_t* msg = (_remote_ipc_semctl_response_t*) inc_msg;

	printk("%s: response --- errno{%d} \n","handle_remote_ipc_semget_response", msg->errno);

	wait = 1;
	errnum= msg->errno;
	wake_up_interruptible(&wq);
	printk("%s: response --- pid stored - wait{%d} \n",
			"handle_remote_pid_response", wait);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

static int handle_remote_ipc_semctl_request(struct pcn_kmsg_message* inc_msg) {

	_remote_ipc_semctl_request_t* msg = (_remote_ipc_semctl_request_t*) inc_msg;
	_remote_ipc_semctl_response_t response;

	int errno=0;
	printk("%s: request -- entered \n", "handle_remote_ipc_semget_request");

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_IPC_SEMCTL_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	errno = remote_semctl(msg->_semnum,msg->_semid,msg->_cmd,msg->_version,msg->_arg);

	response.errno=errno;

	printk("%s: request -- errno %d \n", "handle_remote_ipc_semget_request",response.errno);
	// Send response
	pcn_kmsg_send(msg->header.from_cpu, (struct pcn_kmsg_message*) (&response));

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}


int send_semctl_req_to_remote(int KernelId, int semnum, int semid, int cmd, int version, union semun arg) {

	int res=0;
	_remote_ipc_semctl_request_t* request = kmalloc(sizeof(_remote_ipc_semctl_request_t),
	GFP_KERNEL);
	// Build request
	request->header.type = PCN_KMSG_TYPE_REMOTE_IPC_SEMCTL_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
	request->_semnum = semnum;
	request->_arg = arg;
	request->_cmd = cmd;
	request->_semid = semid;
	request->_version = version;
	// Send response
	res=pcn_kmsg_send_long(KernelId, (struct pcn_kmsg_message*) (request),
			sizeof(_remote_ipc_semctl_request_t) - sizeof(struct pcn_kmsg_hdr));
	return res;
}


int remote_ipc_sem_semctl(int semnum, int semid, int cmd, int version, union semun arg)
{

	reset_wait();
	int ret=0;

	long *key_index;
	int id = -1;

	key_index = 1;

	if(_cpu==0)
	id = send_semctl_req_to_remote(3,semnum,semid,cmd,version,arg);
	else
	id = send_semctl_req_to_remote(0,semnum,semid,cmd,version,arg);

	printk("remote_ipc_sem_findkey: go to sleep!!!!");
	wait_event_interruptible(wq, wait != -1);

	ret=errnum;

	return ret;
}

static int __init ipc_handler_init(void)
{


    _cpu = smp_processor_id();


	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_SEMGET_REQUEST,
				    		handle_remote_ipc_semget_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_SEMGET_RESPONSE,
				    		handle_remote_ipc_semget_response);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_SEMCTL_REQUEST,
						    		handle_remote_ipc_semctl_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_SEMCTL_RESPONSE,
						    		handle_remote_ipc_semctl_response);
	return 0;
}
/**
 * Register process server init function as
 * module initialization function.
 */
late_initcall(ipc_handler_init);
