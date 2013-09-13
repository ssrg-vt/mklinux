
#include <linux/ipc_namespace.h>

struct ipc_params;
struct ipc_ids;
struct kern_ipc_perm;
struct ipc_ops;

//external definition for functions in shm.c
extern int newseg(struct ipc_namespace *ns, struct ipc_params *params);
extern int shm_security(struct kern_ipc_perm *ipcp, int shmflg);
extern int shm_more_checks(struct kern_ipc_perm *ipcp,
				struct ipc_params *params);
//extern long do_shmat(int shmid, char  *shmaddr, int shmflg, ulong *raddr);

//to be removed
extern const struct file_operations shm_file_operations_huge;
extern struct shmid_kernel *shm_lock_check(struct ipc_namespace *ns,
		int id);
extern struct shmid_kernel *shm_lock(struct ipc_namespace *ns, int id);
extern int is_file_shm_hugepages(struct file *file);
extern const struct file_operations shm_file_operations;
extern void shm_destroy(struct ipc_namespace *ns, struct shmid_kernel *shp);
extern bool shm_may_destroy(struct ipc_namespace *ns, struct shmid_kernel *shp);

//to be moved back to shm.c
struct shm_file_data {
	int id;
	struct ipc_namespace *ns;
	struct file *file;
	const struct vm_operations_struct *vm_ops;
};

//external definition for functions in util.c
extern struct kern_ipc_perm *ipc_findkey(struct ipc_ids *ids, key_t key);
extern int ipc_check_perms(struct ipc_namespace *ns,
 			   struct kern_ipc_perm *ipcp,
 			   struct ipc_ops *ops,
 			   struct ipc_params *params);


//remote shared memory operation functions
int remote_ipc_shm_getid(struct ipc_ids *, struct ipc_params *params);
int remote_ipc_shm_shmat(int shmid, char *shmaddr, int shmflg, ulong *raddr);

struct rem_shm_op_result
{
	int errnum;
};
