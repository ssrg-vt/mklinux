/*
 * pcn_scif.c - Kernel Module for Popcorn Messaging Layer over Intel SCIF
 */

#include <linux/circ_buf.h>
#include <linux/proc_fs.h>

#include "pcn_scif_parallel.h"

extern 	int __scif_flush(ep);

#define min_val(x,y) ((x)<=(y)?(x):(y))
#define FALSE 0
#define TRUE 1
#define MAX_CONNEC 8
#define PORT_BASE    20
#define PORT_DATA_IN PORT_BASE
#define PORT_DATA_OUT PORT_BASE + MAX_CONNEC
#define START_OFFSET 0x80000
#define BACKLOG 5 // Length of incoming message queue for control messages
#define _NO_DMA_
#define DMA_THRESH 512
#define DMA_DONE 1
#define NO_PAGES_MAPPED 400
#define DMA_MSG_OFFSET 2*PAGE_SIZE
#define DEFAULT_BUFFER_SIZE 200
#define MAX_LOOPS 12345
#define MAX_LOOPS_JIFFIES (MAX_SCHEDULE_TIMEOUT)
#define MAX_ASYNC_BUFFER 128 

#define _PAGE_MSG_GET 1


static scif_epd_t send_epd;
static int msg_count;
static int err=0,control_msg=1;


volatile unsigned long ticket;
#define fetch_and_add xadd_sync

int is_connection_done[MAX_CONNEC];
int is_send_connection_done;

extern scif_epd_t scif_open(void);

static int dma_rcv_thresh,dma_send_thresh;

//struct buffer_desc dma_buffer_decs[MAX_CONNEC];
send_conn_desc  conn_descriptors[MAX_CONNEC];
rcv_conn_desc  rcv_descriptors[MAX_CONNEC];
dq_info * excution_qs[MAX_CONNEC];
//spinlock_t dma_q_free[MAX_CONNEC];

#define BARRIER(newepd, string) { \
	printk("%s\n", string); \
	scif_send(newepd, &control_msg, sizeof(control_msg), 1); \
	scif_recv(newepd, &control_msg, sizeof(control_msg), 1); \
	printk("==============================================================\n");\
}

static struct semaphore send_connDone[MAX_CONNEC];
static struct semaphore rcv_connDone[MAX_CONNEC];

DEFINE_SPINLOCK(send_q_mutex_p); 
static send_wait send_wait_q;

DEFINE_SPINLOCK(send_lock_pp); 

DEFINE_SPINLOCK(rcv_q_mutex_p); 
//static rcv_wait rcv_wait_q_array[MAX_CONNEC];

//DEFINE_SPINLOCK(send_lock_p); 

//DEFINE_SPINLOCK(dma_q_free_p); 


DEFINE_SPINLOCK(sent_msg_mutex);
DEFINE_SPINLOCK(rcvd_msg_mutex);

static int __init initialize(void);
static void __exit unload(void);
static void __free_msg(void *msg);

unsigned int my_cpu;

static pcn_kmsg_cbftn callbacks[PCN_KMSG_TYPE_MAX];
static struct task_struct *handler;
static struct task_struct *sender_handler;
static struct task_struct *exect_handler;
static struct task_struct *test_handler;

//static char *dma_send_buffer;
//static off_t send_buffer;

static char * freeList;
static char * remote_free_list;
unsigned long  pages_send=0;
unsigned long  pages_rcv=0;


static struct pcn_kmsg_buf * send_buf[MAX_CONNEC];

//static struct buffer_desc dma_buffer_decs,remote_buffer_desc;


/*
int buffer_init(struct buffer_desc * desc, int elem)
{
  void * addr;
  long size = (sizeof(desc->buffer[0]) * elem);

  if ((elem < 1) || (desc == 0))
    return -1;
  addr = malloc(size + sizeof(int) + sizeof(int)); //TODO this must be changed in pages
  if (!addr)
    return -1;
  desc->phead = addr;
  desc->ptail = addr + sizeof(int);
  desc->buffer = addr + sizeof(int) + sizeof(int);
  desc->elements = elem;
  memset(desc->buffer, 0, size);
  return 0;
}
*/


static int read_pages_send(char *page, char **start, off_t off, int count, int *eof, void *data)
{
        int len;

        len=sprintf(page,"%lu",pages_send);

        return len;

}
static int write_pages_send (struct file *file, const char __user *buffer, unsigned long count, void *data)
{
        int i;

        kstrtol_from_user(buffer, count, 0, &pages_send);
      //  printk("%s: migrating_threads %d\n",__func__,migrating_threads);
        return count;
}

static int read_pages_rcv(char *page, char **start, off_t off, int count, int *eof, void *data)
{
        int len;

        len=sprintf(page,"%lu",pages_rcv);

        return len;
        
}
static int write_pages_rcv (struct file *file, const char __user *buffer, unsigned long count, void *data)
{
        int i;

        kstrtol_from_user(buffer, count, 0, &pages_rcv);
      //  printk("%s: migrating_threads %d\n",__func__,migrating_threads);
        return count;
}
      



int pcn_connection_status(int conn_no)
{
	return is_connection_done[conn_no];
}

static inline int buffer_empty(struct buffer_desc * desc)
{
	return ((*desc->phead) == (*desc->ptail));
}

static inline int buffer_full(struct buffer_desc * desc)
{
	return ((((*desc->phead) +1) % desc->elements) == (*desc->ptail));
}


static int buffer_get(struct buffer_desc * desc, long * value) 
{
	
 // printk("%s: desc->elements %d \n ",__func__,desc->elements); 	
  if (!value || !desc){
    printk("returning -1\n");
	return -1;
	}
  if (buffer_empty(desc)){
//	printk("buffer empty: head %d tail %d returning 0\n",*desc->phead,*desc->ptail);
    return 0;
	}
  *value = desc->buffer[*desc->ptail];
 // printk("%s: buffer_no %d\n",*value);
  *desc->ptail = (*desc->ptail +1) % desc->elements;
 //  printk("%s: newTail %d\n",*desc->ptail);
  return 1;
}

static int buffer_put(struct buffer_desc * desc, long value)
{
	
 
  if (!desc)
    return -1;
  int local_head=*desc->phead;
 //  printk("%s: desc->elements %d head %d\n ",__func__,desc->elements);	
  //if (buffer_full(desc))
  //  return 0;
  desc->buffer[local_head] = value;
  *desc->phead = (local_head +1) % desc->elements;
 // printk("%s: newHead %d\n",*desc->phead);
  return 1;
}

static void enq_send(struct pcn_kmsg_buf * buf, struct pcn_kmsg_message *msg, unsigned int dest_cpu, unsigned int payload_size)
{
	down_interruptible(&(buf->snd_q_full));
	spin_lock(&(buf->enq_buf_mutex));
	unsigned long head = buf->head;
	unsigned long tail = ACCESS_ONCE(buf->tail);

	buf->rbuf[head].msg = msg;
	buf->rbuf[head].msg->hdr.flag = PCN_KMSG_ASYNC;
	buf->rbuf[head].dest_cpu = dest_cpu;
	buf->rbuf[head].payload_size = payload_size;
	smp_wmb();
	buf->head = (head + 1) & (MAX_ASYNC_BUFFER - 1);
	up(&(buf->snd_q_empty));
	spin_unlock(&(buf->enq_buf_mutex));
}

static void deq_send(struct pcn_kmsg_buf * buf, int conn_no)
{
	struct pcn_kmsg_buf_item msg;

	down_interruptible(&(buf->snd_q_empty));
	spin_lock(&(buf->deq_buf_mutex));
	unsigned long head = ACCESS_ONCE(buf->head);
	unsigned long tail = buf->tail;

	smp_read_barrier_depends();
	msg = buf->rbuf[tail];
	smp_mb();
	buf->tail = (tail + 1) & (MAX_ASYNC_BUFFER - 1);
	up(&(buf->snd_q_full));
	spin_unlock(&(buf->deq_buf_mutex));

	__pcn_do_send(msg.dest_cpu, msg.msg, msg.payload_size, conn_no);
	__free_msg(msg.msg);
}

static void enq_rcv(dq_info * q_info,rcv_wait *strc)
{
	
    spin_lock(&q_info->rcv_q_mutex);
	INIT_LIST_HEAD(&(strc->list));
	list_add_tail(&(strc->list), &(q_info->rcv_wait_q.list));
	up(&q_info->rcv_q_empty);
	spin_unlock(&q_info->rcv_q_mutex);

}

static rcv_wait * dq_rcv(dq_info *q_info)
{
	rcv_wait *tmp;							//I have to change this
	down_interruptible(&q_info->rcv_q_empty);
	spin_lock(&q_info->rcv_q_mutex);
	if(list_empty(&q_info->rcv_wait_q.list)){
		printk("List is empty...\n");
		spin_unlock(&q_info->rcv_q_mutex);
		return NULL;	
	}
	else{
		tmp = list_first_entry (&q_info->rcv_wait_q.list, rcv_wait, list);
		list_del(q_info->rcv_wait_q.list.next);
		spin_unlock(&q_info->rcv_q_mutex);
		return tmp;
	}
	
}



// Initialize callback table to null, set up control and data channels
static int __init initialize(){
	uint16_t fromcpu;
	int i;

	memset(is_connection_done, 0, sizeof(is_connection_done));

	struct proc_dir_entry *res_send;
	res_send = create_proc_entry("pages_send", S_IRUGO, NULL);
	if (!res_send) {
        	printk(KERN_ALERT"%s: create_proc_entry failed (%p)\n", __func__, res_send);
	        return -ENOMEM;
	}
	
	res_send->read_proc = read_pages_send;
	res_send->write_proc = write_pages_send;
	
	struct proc_dir_entry *res_rcv;
        res_rcv = create_proc_entry("pages_rcv", S_IRUGO, NULL);
        if (!res_rcv) {
                printk(KERN_ALERT"%s: create_proc_entry failed (%p)\n", __func__, res_rcv);
                return -ENOMEM;
        }

	res_rcv->read_proc = read_pages_rcv;
	res_rcv->write_proc =write_pages_rcv;


	printk("In pcn new messaging layer init\n");	
	for(i=0;i<MAX_CONNEC;i++)
	{
		sema_init(&send_connDone[i],0);
		sema_init(&rcv_connDone[i],0);
	}
	
	if(scif_get_nodeIDs(NULL, 0, &fromcpu) == -1){
		printk(KERN_INFO "scif_get_nodeIDs failed! Messaging layer not initialized\n");
		return -1;
	}
	my_cpu = fromcpu;
	
	if(my_cpu==0)
	{
		dma_rcv_thresh=512;
		dma_send_thresh=8192;
	}
	else
	{
		dma_rcv_thresh=8192;
		dma_send_thresh=512;
	}
	//for(i = 0; i < PCN_KMSG_TYPE_MAX; i++) callbacks[i] = NULL;
	
	
	for(i=0;i<MAX_CONNEC;i++)
	{
		conn_thread_data * conn_data = (conn_thread_data*)kmalloc(sizeof(conn_thread_data),GFP_KERNEL);
		BUG_ON(!conn_data);

		conn_data->portID=PORT_DATA_IN+i;
		conn_data->conn_no=i;

		handler = kthread_run(connection_handler, conn_data, "pcn_connd");
		if(handler < 0){
			printk(KERN_INFO "kthread_run failed! Messaging Layer not initialized\n");
			return (long long int)handler;
		}
		if(my_cpu!=0)
		{
			down_interruptible(&rcv_connDone[i]);
		}
	}

	for(i=0;i<MAX_CONNEC;i++)
	{
		send_thread_data * send_data = (send_thread_data*)kmalloc(sizeof(send_thread_data),GFP_KERNEL);
		BUG_ON(!send_data);

		send_data->portID=PORT_DATA_IN+i;
		send_data->conn_no=i;
		send_data->buf = (struct pcn_kmsg_buf *) kmalloc(sizeof(struct pcn_kmsg_buf), GFP_KERNEL);
		BUG_ON(!(send_data->buf));
		send_data->buf->rbuf = (struct pcn_kmsg_buf_item *) vmalloc(sizeof(struct pcn_kmsg_buf_item) * MAX_ASYNC_BUFFER);

		send_data->buf->head = 0;
		send_data->buf->tail = 0;

		sema_init(&(send_data->buf->snd_q_empty), 0);
		sema_init(&(send_data->buf->snd_q_full), MAX_ASYNC_BUFFER);
		send_buf[i] = send_data->buf;
		spin_lock_init(&(send_data->buf->enq_buf_mutex)); 
		spin_lock_init(&(send_data->buf->deq_buf_mutex)); 

		handler = kthread_run(send_thread, send_data, "pcn_sendd_parallel");
		if(handler < 0){
			printk(KERN_INFO "kthread_run failed! Messaging Layer not initialized\n");
			return (long long int)handler;
		}
		if(my_cpu!=0)
		{
			down_interruptible(&send_connDone[i]);
		}
		printk("Send thread %d done\n", i);
	}
/*	
    exect_handler = kthread_run(executer_thread, NULL, "pcnscif_execD");
	if(exect_handler < 0){
		printk(KERN_INFO "kthread_run failed! Messaging Layer not initialized\n");
		return (long long int)exect_handler;
	}
*/ 
#ifdef __TEST__
	test_handler = kthread_run(test_thread, NULL, "pcnscif_testD");
	if(test_handler < 0){
		printk(KERN_INFO "kthread_run failed! Messaging Layer not initialized\n");
		return (long long int)exect_handler;
	}
#endif	
	printk(KERN_INFO "Popcorn SCIF Messaging Layer Initialized\n");
	
	return 0;
}

late_initcall(initialize);

static int handle_selfie_test(struct pcn_kmsg_message* inc_msg)
{
	printk("%s:%s",__func__,inc_msg->payload);
	
}
#ifdef __TEST__
int test_thread(void* arg0)
{
	int i;
	printk("%s:called\n",__func__);
	pcn_kmsg_register_callback(PCN_KMSG_TYPE_SELFIE_TEST,
		handle_selfie_test);
	for (i=0;i<10;i++)
	{
		struct pcn_kmsg_message *msg = (struct pcn_kmsg_message *)pcn_kmsg_alloc_msg(sizeof(struct pcn_kmsg_message));
		msg->hdr.type= PCN_KMSG_TYPE_SELFIE_TEST;
		memset(msg->payload,'a',PCN_KMSG_PAYLOAD_SIZE);
		pcn_kmsg_send(my_cpu, msg);
		pcn_kmsg_free_msg(msg);
	}
}
#endif

static int executer_thread(void* arg0)
{
	rcv_wait * wait_data;
	pcn_kmsg_cbftn ftn;
	
	dq_info* q_info = (dq_info*)arg0;
	while(1)
	{
		wait_data=dq_rcv(q_info);
		
		struct pcn_kmsg_message *msg = wait_data->msg;
		//printk("Executer Thread\n type %d size %d\n",msg->hdr.type,msg->hdr.size);
		if(msg->hdr.type < 0 || msg->hdr.type >= PCN_KMSG_TYPE_MAX){
			printk(KERN_INFO "Received invalid message type %d\n", msg->hdr.type);
			__free_msg(msg);
		}else{
			ftn = callbacks[msg->hdr.type];
			if(ftn != NULL){
				ftn(msg);
			}else{
				printk(KERN_INFO "Recieved message type %d size %d has no registered callback!\n", msg->hdr.type,msg->hdr.size,msg_count++);
				__free_msg(msg);
			}
		}
		kfree(wait_data);
	
	}
	
}

static int send_thread(void* arg0)
{
	uint16_t fromcpu;
	scif_epd_t epd;
	int rc;
	int err;
	int dest_cpu;
	send_thread_data *thread_data = arg0;
	int conn_no = thread_data->conn_no;
	
	pcn_kmsg_cbftn ftn;
//	off_t offset,remote_offset;
	struct scif_portID portID;
	int curr_size, no_bytes;
	struct pcn_kmsg_buf_item *item;
	
	if(scif_get_nodeIDs(NULL, 0, &fromcpu) == -1){
		printk(KERN_INFO "scif_get_nodeIDs failed! Messaging layer not initialized\n");
		return -1;
	}
	
	if(fromcpu==0)
		dest_cpu=1;
	else
		dest_cpu=0;
	printk("Dest_CPU %d\n",dest_cpu);
	
	if((conn_descriptors[conn_no].send_epd = scif_open()) == NULL){
		printk(KERN_INFO "scif_open failed! Could not send message.\n");
		return (long long int)epd;
	}
	if((rc = scif_bind(conn_descriptors[conn_no].send_epd, 0)) < 0){
		printk(KERN_INFO "Send Long: scif_bind failed with error %d. Could not send message.\n", rc);
		return -1;
	}
	conn_descriptors[conn_no].ticket = conn_no;
	conn_descriptors[conn_no].portID.node = dest_cpu;
	conn_descriptors[conn_no].portID.port = PORT_DATA_IN+conn_no;
	spin_lock_init(&conn_descriptors[conn_no].send_lock);
	while((rc = scif_connect(conn_descriptors[conn_no].send_epd, &conn_descriptors[conn_no].portID)) < 0){
		msleep(1000);
	}
	if(rc<0)
		printk(KERN_INFO "scif_connect failed with error %d! Could not send message\n", rc);
	
	printk("send_para Before DMA stuff\n");	
	conn_descriptors[conn_no].dma_send_buffer=vmalloc((NO_PAGES_MAPPED+1)*PAGE_SIZE);
	conn_descriptors[conn_no].send_buffer = scif_register(conn_descriptors[conn_no].send_epd, conn_descriptors[conn_no].dma_send_buffer, (NO_PAGES_MAPPED+1)*PAGE_SIZE, 0x80000, 
														SCIF_PROT_WRITE | SCIF_PROT_READ, SCIF_MAP_KERNEL | SCIF_MAP_FIXED);
	
	spin_lock_init(&conn_descriptors[conn_no].dma_q_free);
	epd=conn_descriptors[conn_no].send_epd;
	printk("send_para Before Bar stuff\n");
	BARRIER(epd,"Register Send_PP");
	struct pcn_kmsg_long_message *lmsg;
	
	
	struct scif_range *pages;
	
	err=scif_get_pages(conn_descriptors[conn_no].send_epd,0x80000+(NO_PAGES_MAPPED*PAGE_SIZE),PAGE_SIZE,&conn_descriptors[conn_no].pages);
	if(err<0)
	{
		printk("Could not get remote freelist\n");
	}
	printk("%s: nPages %d err %d phy_addr %x\n",__func__,conn_descriptors[conn_no].pages->nr_pages,err,conn_descriptors[conn_no].pages->phys_addr);
	
	conn_descriptors[conn_no].remote_free_list=ioremap(conn_descriptors[conn_no].pages->phys_addr[0],PAGE_SIZE);	
	conn_descriptors[conn_no].remote_buffer_desc.phead= (int*)(conn_descriptors[conn_no].remote_free_list);
	conn_descriptors[conn_no].remote_buffer_desc.ptail=(int*)(conn_descriptors[conn_no].remote_buffer_desc.phead+1);
	conn_descriptors[conn_no].remote_buffer_desc.buffer=(long*)(conn_descriptors[conn_no].remote_buffer_desc.ptail+1);
	conn_descriptors[conn_no].remote_buffer_desc.elements=DEFAULT_BUFFER_SIZE;
	
	
	printk("Ptail %d Phead %d\n",*conn_descriptors[conn_no].remote_buffer_desc.ptail,*conn_descriptors[conn_no].remote_buffer_desc.phead);
	int i;
	BARRIER(epd,"DMA mapping Done_PP");

	is_connection_done[conn_no] = PCN_CONN_CONNECTED;

	if(my_cpu!=0)
	{
		printk("Waiting on send_thread no %d\n", conn_no);
		printk("Waiting on send_thread no %d\n", conn_no);
		up(&send_connDone[conn_no]);
	}
	printk("Connection %d Done...PCN_SEND Thread\n", conn_no);
	
	while(1)
	{
		deq_send(thread_data->buf, conn_no);
	}
}

static int connection_handler(void *arg0){
	int rc;
	int cmd;
	scif_epd_t newepd;
	int dflt_size;
	struct pcn_kmsg_message *msg,*tmp,*msg_del;
	static struct task_struct * exect_handler_local;
	conn_thread_data * conn_desc=(conn_thread_data *)arg0;
	
	
	dq_info * q_info = kmalloc(sizeof(dq_info),GFP_KERNEL);

	//struct buffer_desc dma_buffer_decs;
	
	INIT_LIST_HEAD(&q_info->rcv_wait_q.list);
	spin_lock_init(&q_info->rcv_q_mutex); 
	sema_init(&(q_info->rcv_q_empty), 0);
	//spin_lock_init(&dma_q_free[conn_desc->conn_no]);
	excution_qs[conn_desc->conn_no]=q_info;

	exect_handler_local = kthread_run(executer_thread, q_info, "pcnscif_execD_pp");
	if(exect_handler < 0){
		printk(KERN_INFO "kthread_run failed! Messaging Layer not initialized\n");
		return (long long int)exect_handler;
	}
	
	
	struct scif_portID portID;
	int no_bytes, curr_size;
	char *curr_addr;

	rcv_wait *wait_data;

	scif_epd_t dataepd = scif_open();
	if(dataepd == SCIF_OPEN_FAILED){
		printk(KERN_INFO "scif_open failed! Messaging layer not initialized\n");
		return (long long int)NULL;
	}
	char *dma_rcv_buffer;
	off_t buffer;
	
	printk("%s_p:port id %d\n",__func__,conn_desc->portID);
	
	int dma_rcv_index=-1;
	rc = scif_bind(dataepd, conn_desc->portID);
	if(rc != conn_desc->portID){
		printk(KERN_INFO "Connection Handler: scif_bind failed with error code %d! Messaging layer not initialized\n", rc);
		scif_close(dataepd);
		return (long long int)NULL;
	}
	rc = scif_listen(dataepd,1);
	if(rc != 0){
		printk(KERN_INFO "scif_listen failed with error code %d! Messaging layer not initialized\n", rc);
		scif_close(dataepd);
		return (long long int)NULL;
	}
	
		rc = -EAGAIN;
		while(rc == -EAGAIN){
			rc = scif_accept(dataepd, &portID, &newepd, (long long int)NULL);
			if((rc != -EAGAIN) && (rc != 0)){
				printk(KERN_INFO "scif_accept failed with error code %d! Messaging layer not initialized\n", rc);
				scif_close(dataepd);
				return (long long int)NULL;
			}
			if(kthread_should_stop()){
				scif_close(dataepd);
				return 0;
			}
			msleep(1000);
		}
	//	dma_rcv_buffer=kmalloc(2*PAGE_SIZE, GFP_KERNEL);
		scif_close(dataepd);
		
		dma_rcv_buffer=vmalloc((NO_PAGES_MAPPED+1)*PAGE_SIZE);
		buffer = scif_register(newepd, dma_rcv_buffer, (NO_PAGES_MAPPED+1)*PAGE_SIZE, 0x80000, SCIF_PROT_WRITE | SCIF_PROT_READ, SCIF_MAP_KERNEL | SCIF_MAP_FIXED);
		int sts_msg=DMA_DONE;
		int msg_size;
			
		conn_descriptors[conn_desc->conn_no].dma_buffer_decs.phead = (int*)(dma_rcv_buffer+(NO_PAGES_MAPPED)*PAGE_SIZE);
		conn_descriptors[conn_desc->conn_no].dma_buffer_decs.ptail= (int*)(conn_descriptors[conn_desc->conn_no].dma_buffer_decs.phead + 1);
		conn_descriptors[conn_desc->conn_no].dma_buffer_decs.buffer= (long*)(conn_descriptors[conn_desc->conn_no].dma_buffer_decs.ptail + 1);
		conn_descriptors[conn_desc->conn_no].dma_buffer_decs.elements=DEFAULT_BUFFER_SIZE;
		int index;
		for(index=0;index<DEFAULT_BUFFER_SIZE;index++)
		{
			conn_descriptors[conn_desc->conn_no].dma_buffer_decs.buffer[index]=index;
		}
		*(conn_descriptors[conn_desc->conn_no].dma_buffer_decs.phead)=DEFAULT_BUFFER_SIZE-1;
		*(conn_descriptors[conn_desc->conn_no].dma_buffer_decs.ptail)=0;
	//	memset(freeList,0x0,PAGE_SIZE);
	//	printk("%s: phy_addr %x\n",__func__,virt_to_phys(freeList));
		BARRIER(newepd,"Register RCV_PP");
		BARRIER(newepd,"DMA Mapping RCV_PP");
		printk("%s_p:Calling a up on %d\n",__func__,conn_desc->conn_no);
		up(&rcv_connDone[conn_desc->conn_no]);
		
		tmp = (struct pcn_kmsg_message*)vmalloc(2*PAGE_SIZE);

while(TRUE){		

		if(kthread_should_stop()){
				scif_close(dataepd);
				return 0;
		}
	
		dflt_size = sizeof(struct pcn_kmsg_hdr);
		curr_addr = (char*) tmp;
		
		if((no_bytes = scif_recv(newepd, curr_addr, dflt_size, SCIF_RECV_BLOCK)))
		{				
			if(no_bytes==-ECONNRESET)
			{
				printk("%s: Peer lost Terminating PCN_Messaging....\n",__func__);
				return;
				
			}
			if(no_bytes<0)
			{
				continue;
				
			}
			msg_size=tmp->hdr.size;
			msg = (struct pcn_kmsg_message *) pcn_kmsg_alloc_msg(msg_size);
		
#if _PAGE_MSG_GET
 	       if( msg_size>PAGE_SIZE)
        	{
                	spin_lock(&rcvd_msg_mutex);
                	pages_rcv++;
                	spin_unlock(&rcvd_msg_mutex);
        	}
#endif	
	
			//printk("Size %d NoRcv %d\n",tmp->hdr.size,no_bytes);
			/*if(msg_size<=sizeof(struct pcn_kmsg_message))
			{
				//printk("Must be NoNDMA Small\n");
			}
			*/
			if(msg_size>dma_rcv_thresh)
			{
				dma_rcv_index=tmp->hdr.slot;
			//	printk("Must be DMA \n");
			
//				vfree(msg);
				//printk("Offset %d + index %d message address %x \n",DMA_MSG_OFFSET,dma_rcv_index,dma_rcv_buffer+DMA_MSG_OFFSET*dma_rcv_index);
				memcpy(msg,dma_rcv_buffer+DMA_MSG_OFFSET*dma_rcv_index,msg_size);		
				buffer_put(&conn_descriptors[msg->hdr.conn_no].remote_buffer_desc,msg->hdr.slot);
			}
			else
			{
				//printk("Must be NoN-DMA Large %d\n",tmp->hdr.size);
				memcpy(msg, tmp, dflt_size);
				curr_size = msg_size - dflt_size;	
				curr_addr = (char*) msg + dflt_size;
				while((no_bytes = scif_recv(newepd, curr_addr, curr_size, SCIF_RECV_BLOCK)) >= 0)
				{
					curr_addr += no_bytes;
					curr_size -= no_bytes;
					if(curr_size == 0)
						break;
				}
			}	
		}

_process:
			wait_data=kmalloc(sizeof(rcv_wait),GFP_KERNEL);
			wait_data->msg = msg;

			enq_rcv(q_info,wait_data); //must be changed

			

			
	}
	kfree(q_info);
	scif_close(newepd);
	return (long long int)NULL;
}



int pcn_kmsg_register_callback(enum pcn_kmsg_type type, pcn_kmsg_cbftn callback){
	if(type >= PCN_KMSG_TYPE_MAX) return -1; //invalid type
	//printk("%s: registering %d \n",type);
	callbacks[type] = callback;
	return 0;
}

int pcn_kmsg_unregister_callback(enum pcn_kmsg_type type){
	if(type >= PCN_KMSG_TYPE_MAX) return -1;
	callbacks[type] = NULL;
	return 0;
}

static int __pcn_do_send(unsigned int dest_cpu, struct pcn_kmsg_long_message *lmsg, unsigned int payload_size, int conn_no) {
	int free_index;		
	int no_bytes,data_send=0;
	char * curr_addr;
	int my_ticket;
	my_ticket=fetch_and_add(&ticket, 1);
	if (conn_no == -1) {
		conn_no=my_ticket%(MAX_CONNEC);
	}
	
//	if((my_cpu!=0))
//		conn_no=conn_no+MAX_CONNEC/2;
	
	int curr_size;
	lmsg->hdr.size = payload_size+sizeof(struct pcn_kmsg_hdr);
	lmsg->hdr.from_cpu = my_cpu;
	lmsg->hdr.conn_no = conn_no;

	unsigned long start_time[10], finish_time[10];

	int i;

	if(dest_cpu==my_cpu)
	{
		rcv_wait *exec_data = NULL;
		while(exec_data==NULL)
		{
				exec_data = kmalloc(sizeof(rcv_wait),GFP_KERNEL);
		}
		exec_data->msg=pcn_kmsg_alloc_msg(lmsg->hdr.size);
		memcpy(exec_data->msg,lmsg,lmsg->hdr.size);
		
		enq_rcv(excution_qs[conn_no],exec_data);		
		return lmsg->hdr.size;
	}

	curr_size = lmsg->hdr.size;
#if _PAGE_MSG_GET
	if( lmsg->hdr.size>PAGE_SIZE)
	{
		spin_lock(&sent_msg_mutex);
		pages_send++;
		spin_unlock(&sent_msg_mutex);
		
	}
#endif
	
	if(lmsg->hdr.size>dma_send_thresh)
	{
		spin_lock(&conn_descriptors[conn_no].send_lock);
		unsigned long bla=0;		
		while(buffer_get(&conn_descriptors[conn_no].dma_buffer_decs,&free_index) != 1)
		{
		}
		spin_unlock(&conn_descriptors[conn_no].send_lock);
found:
		lmsg->hdr.slot=free_index;
		curr_addr = (char*) lmsg;
		int sts_from_peer=-1;
		memcpy(conn_descriptors[conn_no].dma_send_buffer+free_index*DMA_MSG_OFFSET,curr_addr,lmsg->hdr.size);
		err=scif_writeto(conn_descriptors[conn_no].send_epd, conn_descriptors[conn_no].send_buffer+free_index*DMA_MSG_OFFSET, lmsg->hdr.size, 0x80000+free_index*DMA_MSG_OFFSET, SCIF_RMA_SYNC);
		if(err<0)
		{
			printk("error in DMA Transfer %d",err);
			goto _out;
		}
		if((no_bytes = scif_send(conn_descriptors[conn_no].send_epd, curr_addr, sizeof(struct pcn_kmsg_hdr), SCIF_SEND_BLOCK)<0))
		{
			//printk("%s:Through NON-DMA\n",__func__);
			if(no_bytes==-ENODEV)
			{
				printk("%s:Terminatind pcn_sendD\n",__func__);
				//up(&wait_data->_sem);
				data_send=no_bytes;
				is_connection_done[conn_no] = PCN_CONN_WATING;
				return;
			}
			printk("Some thing went wrong Send Failed");
			data_send=no_bytes;
			goto _out;
		}
		data_send=lmsg->hdr.size;
	}
	else
	{
		//spin_lock(&conn_descriptors[conn_no].send_lock);
		//printk("Non_dma Send\n");
		curr_addr = (char*) lmsg;
		curr_size = lmsg->hdr.size;
		while((no_bytes = scif_send(conn_descriptors[conn_no].send_epd, curr_addr, curr_size, 0)) >= 0) {
			if(no_bytes==-ENODEV)
			{
				printk("%s:Terminatind pcn_sendD\n",__func__);
				//up(&wait_data->_sem);
				data_send=no_bytes;
				is_connection_done[conn_no] = PCN_CONN_WATING;
				return;
			}

			if(no_bytes<0)
			{
				printk("Some thing went wrong Send Failed\n");
			}
			curr_addr = curr_addr + no_bytes;
			curr_size = curr_size - no_bytes;
			data_send=data_send+no_bytes;
			if(curr_size == 0)
				break;
		}
	}
_out:
	return (data_send<0?-1:data_send);
}

static int __pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_long_message *msg, unsigned int payload_size) {
	int my_ticket;
	my_ticket=fetch_and_add(&ticket, 1);
	int conn_no=my_ticket%(MAX_CONNEC);
	int ret;

	if(pcn_connection_status(conn_no) == PCN_CONN_WATING) {
		printk("Connection %d is not ready\n", conn_no);
		return -1;
	}

	if (msg->hdr.flag & PCN_KMSG_SYNC) {
		ret = __pcn_do_send(dest_cpu, msg, payload_size, -1);
	} else {
		// Put it into the queue and return immediately
		enq_send(send_buf[conn_no], msg, dest_cpu, payload_size);
		ret = payload_size;
	}

	return ret;
}

int pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg) {
	return __pcn_kmsg_send(dest_cpu,msg,sizeof(struct pcn_kmsg_message)-sizeof(struct pcn_kmsg_hdr));
}

int pcn_kmsg_send_long(unsigned int dest_cpu, struct pcn_kmsg_long_message *lmsg, unsigned int payload_size) {
	return __pcn_kmsg_send(dest_cpu, lmsg, payload_size);
}

void *pcn_kmsg_alloc_msg(size_t size) {
	struct pcn_kmsg_message *msg;

	msg = (struct pcn_kmsg_message *) kmalloc(size, GFP_ATOMIC);
	BUG_ON(!msg);

	msg->hdr.size = size;
	msg->hdr.flag = PCN_KMSG_ASYNC;
	return (void *) msg;
}

static void __free_msg(void *msg) {
	struct pcn_kmsg_message *pmsg = (struct pcn_kmsg_message *)msg;
	kfree(msg);
}

void pcn_kmsg_free_msg_now(void *msg) {
	__free_msg(msg);
}

void pcn_kmsg_free_msg(void *msg) {
/*
 *    struct pcn_kmsg_message *pmsg = (struct pcn_kmsg_message *)msg;
 *
 *    if (pmsg->hdr.flag & PCN_KMSG_SYNC) {
 *        __free_msg(msg);
 *    }
 */
}


EXPORT_SYMBOL(pcn_kmsg_alloc_msg);
EXPORT_SYMBOL(pcn_kmsg_free_msg);
EXPORT_SYMBOL(pcn_kmsg_free_msg_now);
EXPORT_SYMBOL(pcn_kmsg_send_long);
EXPORT_SYMBOL(pcn_kmsg_send);
EXPORT_SYMBOL(pcn_kmsg_unregister_callback);
EXPORT_SYMBOL(pcn_kmsg_register_callback);
