
#include <linux/sem.h>
#include <linux/ipc_namespace.h>

struct ipc_params;
union semun;
struct ipc_ids;
struct kern_ipc_perm;
struct ipc_ops;

//external definition for functions in sem.c
extern int newary(struct ipc_namespace *, struct ipc_params *);
extern void freeary(struct ipc_namespace *, struct kern_ipc_perm *);
extern int sem_security(struct kern_ipc_perm *ipcp, int semflg);
extern int sem_more_checks(struct kern_ipc_perm *ipcp,
				struct ipc_params *params);
extern int semctl_down(struct ipc_namespace *ns, int semid,
		       int cmd, int version, union semun arg);
extern int semctl_nolock(struct ipc_namespace *ns, int semid,
			 int cmd, int version, union semun arg);
extern int semctl_main(struct ipc_namespace *ns, int semid, int semnum,
		int cmd, int version, union semun arg);

//external definition for functions in util.c
extern struct kern_ipc_perm *ipc_findkey(struct ipc_ids *ids, key_t key);
extern int ipc_check_perms(struct ipc_namespace *ns,
 			   struct kern_ipc_perm *ipcp,
 			   struct ipc_ops *ops,
 			   struct ipc_params *params);


//remote semaphore operation functions
int remote_ipc_sem_getid(struct ipc_ids *, struct ipc_params *params);
int remote_ipc_sem_semctl(int semnum, int semid, int cmd, int version, union semun arg);

struct rem_sem_op_result
{
	int errnum;
};
