

#include <linux/ipc_namespace.h>
#include <linux/ipc.h>
#include <linux/idr.h>

static inline struct kern_ipc_perm *remote_ipc_findkey(struct ipc_ids *ids, key_t key)
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
