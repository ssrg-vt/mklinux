#ifndef _PCN_SCIF_PARALLEL_
#define _PCN_SCIF_PARALLEL_
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/file.h>
#include <linux/pcn_kmsg.h>
#include <linux/fdtable.h>
#include "scif.h"

struct buffer_desc {
  long* buffer;
  int*  phead;
  int*  ptail;
  int   elements;
};
typedef struct _rcv_wait{
	struct list_head list;
	void * msg;
}rcv_wait;

typedef struct _dq_info
{
	rcv_wait rcv_wait_q;
	spinlock_t rcv_q_mutex;
	struct semaphore rcv_q_empty;
}dq_info;

typedef struct _send_conn_desc
{
	unsigned long ticket;
	scif_epd_t send_epd;
	struct scif_range *pages;
	char * remote_free_list;
	struct scif_portID portID;
	struct buffer_desc remote_buffer_desc;
	struct buffer_desc dma_buffer_decs;
	char *dma_send_buffer;
	off_t send_buffer;
	spinlock_t dma_q_free;
	spinlock_t send_lock;
}send_conn_desc; 


typedef struct _rcv_conn_desc
{
	scif_epd_t rcv_epd;
	char * dma_rcv_buffer;
	off_t buffer;
	struct buffer_desc local_buffer_desc;
	dq_info q_info;
	
}rcv_conn_desc;



typedef struct _conn_thread_data
{
	int portID;
	int conn_no;
	dq_info q_info;			
}conn_thread_data;


struct personal_desc {
  struct buffer_desc mybuf;
  int lock;
};

typedef struct _send_wait{
	struct list_head list;
	struct semaphore _sem;
	void * msg;
	int error;
	int dst_cpu;
}send_wait;






//function prototypes
static int connection_handler(void *arg0);
static int send_thread(void* arg0);
static int executer_thread(void* arg0);
static int test_thread(void* arg0);

 
#endif
 
