#ifndef __POPCORN_PID_H__
#define __POPCORN_PID_H__

#include <asm/page.h>
#include <linux/pid_namespace.h>
#include <linux/threads.h>
#include <linux/types.h>
#include "init.h"


#define POPCORN_MAX_KERNEL_ID  15
#define GLOBAL_PID_MASK PID_MAX_LIMIT
#define PID_NODE_SHIFT POPCORN_MAX_KERNEL_ID
#define INTERNAL_PID_MASK (PID_MAX_LIMIT - 1)

#define GLOBAL_PID_NODE(pid, node) \
	(((node) << PID_NODE_SHIFT)|GLOBAL_PID_MASK|((pid) & INTERNAL_PID_MASK))

#define GLOBAL_PID(pid) GLOBAL_PID_NODE(pid,Kernel_Id)
#define GLOBAL_NPID(pid,node) GLOBAL_PID_NODE(pid,node)

#define SHORT_PID(pid) ((pid) & INTERNAL_PID_MASK)
#define ORIG_PID(pid) (SHORT_PID(pid) & ((1<<POPCORN_MAX_KERNEL_ID)-1))
#define ORIG_NODE(pid) (SHORT_PID(pid) >> PID_NODE_SHIFT)




#endif
