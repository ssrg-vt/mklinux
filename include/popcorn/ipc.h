
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/init.h>
#include <linux/rcupdate.h>

#include "init.h"

#define KERNELBITS 6 // Bits to represent Kernel ids

//Maximum semid is (1<<29)-1=536870911

#define IPC_MAX_ID (1UL << 35)  //35 bits  (29 bits for integer representation + 6 bits for kernel representation)
#define IPC_MAX_ID_MASK (IPC_MAX_ID - 1)
#define IPC_SHIFT_BITS (35-KERNELBITS)

#define GLOBAL_IPC_NODE(id, node) \
        (((node) << IPC_SHIFT_BITS)|IPC_MAX_ID|((id) & IPC_MAX_ID_MASK))

#define GLOBAL_IPCID(id) GLOBAL_IPC_NODE(id,Kernel_Id)

#define SHORT_IPCID(id) ((id) & IPC_MAX_ID_MASK)
#define ORIG_IPCID(id) (SHORT_IPCID(id) & ((1UL<<IPC_SHIFT_BITS)-1))
#define ORIG_IPC_NODE(id) (SHORT_IPCID(id) >> IPC_SHIFT_BITS)

static inline int isLocalKernel(int id)
{
	if(ORIG_IPC_NODE(id)==Kernel_Id)
		return 1;

	return 0;
}



typedef long global_ipc_id_t;


#define GLOBAL_KEY_NODE(key, node) \
	(((node) << IPC_SHIFT_BITS)|IPC_MAX_ID|((key) & IPC_MAX_ID_MASK))

#define GLOBAL_KEY(key) GLOBAL_KEY_NODE(key,Kernel_Id)

#define SHORT_KEY(key) ((key) & IPC_MAX_ID_MASK)
#define ORIG_KEY(key) (SHORT_KEY(key) & ((1<<IPC_SHIFT_BITS)-1))
#define ORIG_KEY_NODE(key) (SHORT_KEY(key) >> IPC_SHIFT_BITS)


typedef long global_key_t;

