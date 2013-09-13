#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/pcn_kmsg.h>
#include <linux/string.h>
#include <linux/wait.h>

#include <linux/shm.h>
#include "shm_remote.h"
#include "util.h"
#include <popcorn/pid.h>
#include "remote_ipc_function.h"

//to be removed
#include <linux/file.h>
#include <linux/mman.h>
#include <linux/shmem_fs.h>
#include <linux/security.h>
#include <linux/hugetlb.h>

static int _cpu = -1;
static int wait = -1;
static int errnum = 0;
static unsigned long resptr;

static DECLARE_WAIT_QUEUE_HEAD( wq);

/*
 ******************************************************** common functions*******************************************************************
 */

int reset_wait_shm() {
	wait = -1;
	resptr = 0;
	return 0;
}

/*
 ******************************************************** semget message functions*******************************************************************
 */

struct _remote_ipc_shmget_request {
	struct pcn_kmsg_hdr header;
	struct ipc_params _param;
	char pad_string[44];
}__attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_ipc_shmget_request _remote_ipc_shmget_request_t;

struct _remote_ipc_shmget_response {
	struct pcn_kmsg_hdr header;
	int errno;
	char pad_string[56];
}__attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_ipc_shmget_response _remote_ipc_shmget_response_t;
/*
struct kern_ipc_perm *remote_ipc_findkey(struct ipc_ids *ids, key_t key) {
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
*/
int remote_shmget(struct ipc_params *params) {
	struct ipc_namespace *ns;
	struct ipc_ops shm_ops;

	struct kern_ipc_perm *perm;
	int err = 0;

	ns = current->nsproxy->ipc_ns;

	shm_ops.getnew = newseg;
	shm_ops.associate = shm_security;
	shm_ops.more_checks = shm_more_checks;

	perm = remote_ipc_findkey(&shm_ids(ns), params->key);
	if (perm != NULL) {
		err = shm_more_checks(perm, params);
		if (!err)
			err = ipc_check_perms(ns, perm, &shm_ops, params);

		return GLOBAL_IPCID(err);
	}
	return 0;
}

static int handle_remote_ipc_shmget_response(struct pcn_kmsg_message* inc_msg) {
	_remote_ipc_shmget_response_t* msg =
			(_remote_ipc_shmget_response_t*) inc_msg;

	printk("%s: response --- errno{%d} \n", "handle_remote_ipc_shmget_response",
			msg->errno);

	wait = 1;
	errnum = msg->errno;
	wake_up_interruptible(&wq);
	printk("%s: response --- pid stored - wait{%d} \n",
			"handle_remote_pid_response", wait);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

static int handle_remote_ipc_shmget_request(struct pcn_kmsg_message* inc_msg) {

	_remote_ipc_shmget_request_t* msg = (_remote_ipc_shmget_request_t*) inc_msg;
	_remote_ipc_shmget_response_t response;

	int errno = 0;
	printk("%s: request -- entered \n", "handle_remote_ipc_shmget_request");

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_IPC_SHMGET_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	errno = remote_shmget(&(msg->_param));

	response.errno = errno;

	printk("%s: request -- errno %d \n", "handle_remote_ipc_shmget_request",
			response.errno);
	// Send response
	pcn_kmsg_send(msg->header.from_cpu, (struct pcn_kmsg_message*) (&response));

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

int send_shmget_req_to_remote(int KernelId, struct ipc_params *params) {

	int res = 0;
	_remote_ipc_shmget_request_t* request = kmalloc(
			sizeof(_remote_ipc_shmget_request_t),
			GFP_KERNEL);
	// Build request
	request->header.type = PCN_KMSG_TYPE_REMOTE_IPC_SHMGET_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
	request->_param = *params;
	// Send response
	res = pcn_kmsg_send(KernelId, (struct pcn_kmsg_message*) (request));
	return res;
}

int remote_ipc_shm_getid(struct ipc_ids *ids, struct ipc_params *params) {

	reset_wait_shm();
	int ret = 0, i;
	/*
	 * have to send message to remote kernels and fetch if the key is present
	 */
	long *key_index;
	int id = -1;

	key_index = 1;

	for (i = 0; i < NR_CPUS; i++) {

		reset_wait_shm();
		// Skip the current cpu
		if (i == _cpu)
			continue;
		id = send_shmget_req_to_remote(i, params);

		if (!id) {
			printk("remote_ipc_shm_getid: go to sleep for kernel {%d}!!!!",i);
			wait_event_interruptible(wq, wait != -1);
			ret = errnum;
			if (ret != 0)
				break;
		}
	}

	return ret;
}

/*
 ******************************************************** shmat message functions*******************************************************************
 */

struct _remote_ipc_shmat_request {
	struct pcn_kmsg_hdr header;
	int _shmid;
	char *_shmaddr;
	int _shmflg;
}__attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_ipc_shmat_request _remote_ipc_shmat_request_t;

struct _remote_ipc_shmat_response {
	struct pcn_kmsg_hdr header;
	int errno;
	ulong _raddr;
	char pad_string[48];
}__attribute__((packed)) __attribute__((aligned(64)));

typedef struct _remote_ipc_shmat_response _remote_ipc_shmat_response_t;

int remote_shmat(int shmid, char *shmaddr, int shmflg, ulong *raddr) {

	//return do_shmat(shmid,shmaddr,shmflg,raddr);

	struct shmid_kernel *shp;
	unsigned long addr;
	unsigned long size;
	struct file * file;
	int    err;
	unsigned long flags;
	unsigned long prot;
	int acc_mode;
	unsigned long user_addr;
	struct ipc_namespace *ns;
	struct shm_file_data *sfd;
	struct path path;
	fmode_t f_mode;

	err = -EINVAL;
	if (shmid < 0)
		goto out;
	else if ((addr = (ulong)shmaddr)) {
		if (addr & (SHMLBA-1)) {
			if (shmflg & SHM_RND)
				addr &= ~(SHMLBA-1);	   /* round down */
			else
#ifndef __ARCH_FORCE_SHMLBA
				if (addr & ~PAGE_MASK)
#endif
					goto out;
		}
		flags = MAP_SHARED | MAP_FIXED;
	} else {
		if ((shmflg & SHM_REMAP))
			goto out;

		flags = MAP_SHARED;
	}

	if (shmflg & SHM_RDONLY) {
		prot = PROT_READ;
		acc_mode = S_IRUGO;
		f_mode = FMODE_READ;
	} else {
		prot = PROT_READ | PROT_WRITE;
		acc_mode = S_IRUGO | S_IWUGO;
		f_mode = FMODE_READ | FMODE_WRITE;
	}
	if (shmflg & SHM_EXEC) {
		prot |= PROT_EXEC;
		acc_mode |= S_IXUGO;
	}

	/*
	 * We cannot rely on the fs check since SYSV IPC does have an
	 * additional creator id...
	 */
	ns = current->nsproxy->ipc_ns;
	shp = shm_lock_check(ns, shmid);
	if (IS_ERR(shp)) {
		err = PTR_ERR(shp);
		goto out;
	}

	err = -EACCES;
	if (ipcperms(ns, &shp->shm_perm, acc_mode))
		goto out_unlock;

	err = security_shm_shmat(shp, shmaddr, shmflg);
	if (err)
		goto out_unlock;

	path = shp->shm_file->f_path;
	path_get(&path);
	shp->shm_nattch++;
	size = i_size_read(path.dentry->d_inode);
	shm_unlock(shp);

	err = -ENOMEM;
	sfd = kzalloc(sizeof(*sfd), GFP_KERNEL);
	if (!sfd)
		goto out_put_dentry;

	file = alloc_file(&path, f_mode,
			  is_file_hugepages(shp->shm_file) ?
				&shm_file_operations_huge :
				&shm_file_operations);
	if (!file)
		goto out_free;

	file->private_data = sfd;
	file->f_mapping = shp->shm_file->f_mapping;
	sfd->id = shp->shm_perm.id;
	sfd->ns = get_ipc_ns(ns);
	sfd->file = shp->shm_file;
	sfd->vm_ops = NULL;

	//down_write(&current->mm->mmap_sem);
	if (addr && !(shmflg & SHM_REMAP)) {
		err = -EINVAL;
		if (find_vma_intersection(current->mm, addr, addr + size))
			goto invalid;
		/*
		 * If shm segment goes below stack, make sure there is some
		 * space left for the stack to grow (at least 4 pages).
		 */
		if (addr < current->mm->start_stack &&
		    addr > current->mm->start_stack - size - PAGE_SIZE * 5)
			goto invalid;
	}

	user_addr = do_mmap (file, addr, size, prot, flags, 0);
	*raddr = user_addr;
	err = 0;
	if (IS_ERR_VALUE(user_addr))
		err = (long)user_addr;
invalid:
	//up_write(&current->mm->mmap_sem);

	fput(file);

out_nattch:
	down_write(&shm_ids(ns).rw_mutex);
	shp = shm_lock(ns, shmid);
	BUG_ON(IS_ERR(shp));
	shp->shm_nattch--;
	if (shm_may_destroy(ns, shp))
		shm_destroy(ns, shp);
	else
		shm_unlock(shp);
	up_write(&shm_ids(ns).rw_mutex);

out:
	return err;

out_unlock:
	shm_unlock(shp);
	goto out;

out_free:
	kfree(sfd);
out_put_dentry:
	path_put(&path);
	goto out_nattch;
}

static int handle_remote_ipc_shmat_response(struct pcn_kmsg_message* inc_msg) {
	_remote_ipc_shmat_response_t* msg = (_remote_ipc_shmat_response_t*) inc_msg;

	printk("%s: response --- errno{%d} \n", "handle_remote_ipc_semget_response",
			msg->errno);

	wait = 1;
	errnum = msg->errno;
	resptr = msg->_raddr;
	wake_up_interruptible(&wq);
	printk("%s: response --- pid stored - wait{%d} \n",
			"handle_remote_pid_response", wait);

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

static int handle_remote_ipc_shmat_request(struct pcn_kmsg_message* inc_msg) {

	_remote_ipc_shmat_request_t* msg = (_remote_ipc_shmat_request_t*) inc_msg;
	_remote_ipc_shmat_response_t response;

	int errno = 0;
	ulong raddr;
	printk("%s: request -- entered \n", "handle_remote_ipc_semget_request");

	// Finish constructing response
	response.header.type = PCN_KMSG_TYPE_REMOTE_IPC_SHMAT_RESPONSE;
	response.header.prio = PCN_KMSG_PRIO_NORMAL;

	errno = remote_shmat(msg->_shmid, msg->_shmaddr, msg->_shmflg, &raddr);

	response.errno = errno;
	response._raddr = raddr;

	printk("%s: request -- errno %d \n", "handle_remote_ipc_semget_request",
			response.errno);
	// Send response
	pcn_kmsg_send(msg->header.from_cpu, (struct pcn_kmsg_message*) (&response));

	pcn_kmsg_free_msg(inc_msg);

	return 0;
}

int send_shmat_req_to_remote(int KernelId, int shmid, char *shmaddr, int shmflg) {

	int res = 0;
	_remote_ipc_shmat_request_t* request = kmalloc(
			sizeof(_remote_ipc_shmat_request_t),
			GFP_KERNEL);
	// Build request
	request->header.type = PCN_KMSG_TYPE_REMOTE_IPC_SHMAT_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
	request->_shmid = shmid;
	request->_shmaddr = shmaddr;
	request->_shmflg = shmflg;
	// Send response
	res = pcn_kmsg_send_long(KernelId, (struct pcn_kmsg_message*) (request),
			sizeof(_remote_ipc_shmat_request_t) - sizeof(struct pcn_kmsg_hdr));
	return res;
}

int remote_ipc_shm_shmat(int shmid, char *shmaddr, int shmflg, ulong *raddr) {

	reset_wait_shm();
	int ret = 0, i;

	long *key_index;
	int id = -1;

	key_index = 1;

	id = send_shmat_req_to_remote(ORIG_IPC_NODE(shmid), shmid, shmaddr, shmflg);

		if (!id) {
			printk("remote_ipc_shm_shmat: go to sleep for kernel {%d}!!!!",ORIG_IPC_NODE(shmid));
			wait_event_interruptible(wq, wait != -1);
			ret = errnum;
			raddr = &resptr;
		}

	return ret;
}

static int __init ipc_shm_handler_init(void)
{


    _cpu = smp_processor_id();


	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_SHMGET_REQUEST,
				    		handle_remote_ipc_shmget_request);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_SHMGET_RESPONSE,
				    		handle_remote_ipc_shmget_response);

	pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_SHMAT_REQUEST,
			handle_remote_ipc_shmat_request);
		pcn_kmsg_register_callback(PCN_KMSG_TYPE_REMOTE_IPC_SHMAT_RESPONSE,
				handle_remote_ipc_shmat_response);

	return 0;
}
/**
 * Register process server init function as
 * module initialization function.
 */
late_initcall(ipc_shm_handler_init);
