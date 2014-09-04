/*
 * pcn_scif.c - Kernel Module for Popcorn Messaging Layer over Intel SCIF
 */


#include "pcn_scif_parallel.h"

extern 	int __scif_flush(ep);

#define min_val(x,y) ((x)<=(y)?(x):(y))
#define FALSE 0
#define TRUE 1
#define MAX_CONNEC 10
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

static scif_epd_t send_epd;
static int msg_count;
static int err=0,control_msg=1;


volatile unsigned long ticket;
#define fetch_and_add xadd_sync

int is_connection_done=PCN_CONN_WATING;

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

//struct semaphore send_q_mutex;
static struct semaphore send_q_empty;
//static struct semaphore rcv_q_empty;

static struct semaphore send_connDone;
static struct semaphore rcv_connDone[MAX_CONNEC];

DEFINE_SPINLOCK(send_q_mutex_p); 
static send_wait send_wait_q;

DEFINE_SPINLOCK(send_lock_pp); 

DEFINE_SPINLOCK(rcv_q_mutex_p); 
//static rcv_wait rcv_wait_q_array[MAX_CONNEC];

//DEFINE_SPINLOCK(send_lock_p); 

//DEFINE_SPINLOCK(dma_q_free_p); 


static int __init initialize(void);
static void __exit unload(void);

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

int pcn_connection_staus()
{
	return is_connection_done;
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
	
  // printk("%s: desc->elements %d\n ",__func__,desc->elements); 	
  if (!value || !desc)
    return -1;
  if (buffer_empty(desc))
    return 0;
  *value = desc->buffer[*desc->ptail];
  *desc->ptail = (*desc->ptail +1) % desc->elements;
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
  return 1;
}


static void enq_send(send_wait *strc)
{
	spin_lock(&send_q_mutex_p);
	INIT_LIST_HEAD(&(strc->list));
	list_add_tail(&(strc->list), &(send_wait_q.list));
	up(&send_q_empty);
	spin_unlock(&send_q_mutex_p);
}

static send_wait * dq_send(void)
{
	send_wait *tmp;
	down_interruptible(&send_q_empty);
	spin_lock(&send_q_mutex_p);
	if(list_empty(&send_wait_q.list)){
		printk("List is empty...\n");
		spin_unlock(&send_q_mutex_p);
		return NULL;	
	}
	else{
		tmp = list_first_entry (&send_wait_q.list, send_wait, list);
		list_del(send_wait_q.list.next);
		spin_unlock(&send_q_mutex_p);
		return tmp;
	}
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

	printk("In pcn new messaging layer init\n");	
	//sema_init(&(send_q_mutex), 0);
	sema_init(&(send_q_empty), 0);
	sema_init(&send_connDone,0);
	for(i=0;i<MAX_CONNEC;i++)
	{
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
		conn_thread_data * data = (conn_thread_data*)kmalloc(sizeof(conn_thread_data),GFP_KERNEL);
		data->portID=PORT_DATA_IN+i;
		data->conn_no=i;
		handler = kthread_run(connection_handler, data, "pcn_connd");
		if(handler < 0){
			printk(KERN_INFO "kthread_run failed! Messaging Layer not initialized\n");
			return (long long int)handler;
		}
		if(my_cpu!=0)
		{
			down_interruptible(&rcv_connDone[i]);
		}
		
	}
	sender_handler = kthread_run(send_thread, NULL, "pcnscif_sendD_parallel");
	if(sender_handler < 0){
		printk(KERN_INFO "kthread_run failed! Messaging Layer not initialized\n");
		return (long long int)sender_handler;
	}
/*	
    exect_handler = kthread_run(executer_thread, NULL, "pcnscif_execD");
	if(exect_handler < 0){
		printk(KERN_INFO "kthread_run failed! Messaging Layer not initialized\n");
		return (long long int)exect_handler;
	}
*/ 
	if(my_cpu!=0)
	{
		down_interruptible(&send_connDone);
		
		//down_interruptible(&rcv_connDone);
	}
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
		struct pcn_kmsg_message msg;
		msg.hdr.type= PCN_KMSG_TYPE_SELFIE_TEST;
		memset(msg.payload,'a',PCN_KMSG_PAYLOAD_SIZE);
		pcn_kmsg_send(my_cpu,&msg);
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
			pcn_kmsg_free_msg(msg);
		}else{
			ftn = callbacks[msg->hdr.type];
			if(ftn != NULL){
				ftn(msg);
			}else{
				printk(KERN_INFO "Recieved message type %d size %d has no registered callback!\n", msg->hdr.type,msg->hdr.size,msg_count++);
				pcn_kmsg_free_msg(msg);
			}
		}
		kfree(wait_data);
	
	}
	
}

static int send_thread(void* arg0)
{
	uint16_t fromcpu;
	scif_epd_t epd;
	int rc,number_of_conn=0;
	int err;
	int dest_cpu;
	
	pcn_kmsg_cbftn ftn;
//	off_t offset,remote_offset;
	struct scif_portID portID;
	int curr_size, no_bytes;
	send_wait * wait_data;
	
	if(scif_get_nodeIDs(NULL, 0, &fromcpu) == -1){
		printk(KERN_INFO "scif_get_nodeIDs failed! Messaging layer not initialized\n");
		return -1;
	}
	
	if(fromcpu==0)
		dest_cpu=1;
	else
		dest_cpu=0;
	printk("Dest_CPU %d\n",dest_cpu);
//crete parallel connections 
	
	for(number_of_conn=0;number_of_conn<MAX_CONNEC;number_of_conn++)
	{	
		
		//conn_descriptors[number_of_conn]=kmalloc(sizeof(send_conn_desc),GFP_KERNEL);
		if((conn_descriptors[number_of_conn].send_epd = scif_open()) == NULL){
			printk(KERN_INFO "scif_open failed! Could not send message.\n");
			return (long long int)epd;
		}
		if((rc = scif_bind(conn_descriptors[number_of_conn].send_epd, 0)) < 0){
			printk(KERN_INFO "Send Long: scif_bind failed with error %d. Could not send message.\n", rc);
			return -1;
		}
		conn_descriptors[number_of_conn].ticket = number_of_conn;
		conn_descriptors[number_of_conn].portID.node = dest_cpu;
		conn_descriptors[number_of_conn].portID.port = PORT_DATA_IN+number_of_conn;
		spin_lock_init(&conn_descriptors[number_of_conn].send_lock);
		while((rc = scif_connect(conn_descriptors[number_of_conn].send_epd, &conn_descriptors[number_of_conn].portID)) < 0){
			msleep(1000);
			//printk(KERN_INFO "scif_connect failed with error %d! Could not send message\n", rc);
			//return rc;
		}
		if(rc<0)
			printk(KERN_INFO "scif_connect failed with error %d! Could not send message\n", rc);
		
		printk("send_para Before DMA stuff\n");	
		conn_descriptors[number_of_conn].dma_send_buffer=vmalloc((NO_PAGES_MAPPED+1)*PAGE_SIZE);
		conn_descriptors[number_of_conn].send_buffer = scif_register(conn_descriptors[number_of_conn].send_epd, conn_descriptors[number_of_conn].dma_send_buffer, (NO_PAGES_MAPPED+1)*PAGE_SIZE, 0x80000, 
															SCIF_PROT_WRITE | SCIF_PROT_READ, SCIF_MAP_KERNEL | SCIF_MAP_FIXED);
		
		spin_lock_init(&conn_descriptors[number_of_conn].dma_q_free);
		epd=conn_descriptors[number_of_conn].send_epd;
		printk("send_para Before Bar stuff\n");
		BARRIER(epd,"Register Send_PP");
		struct pcn_kmsg_long_message *lmsg;
	//	int use_dma=0;
		
		
	//	send_epd = epd;
		//printk("%s: epd %d send_epd %d\n",__func__,epd,send_epd);
		struct scif_range *pages;
		
		err=scif_get_pages(conn_descriptors[number_of_conn].send_epd,0x80000+(NO_PAGES_MAPPED*PAGE_SIZE),PAGE_SIZE,&conn_descriptors[number_of_conn].pages);
		if(err<0)
		{
			printk("Could not get remote freelist\n");
		}
		
		
		printk("%s: nPages %d err %d phy_addr %x\n",__func__,conn_descriptors[number_of_conn].pages->nr_pages,err,conn_descriptors[number_of_conn].pages->phys_addr);
		
		conn_descriptors[number_of_conn].remote_free_list=ioremap(conn_descriptors[number_of_conn].pages->phys_addr[0],PAGE_SIZE);	
		conn_descriptors[number_of_conn].remote_buffer_desc.phead= (int*)(conn_descriptors[number_of_conn].remote_free_list);
		conn_descriptors[number_of_conn].remote_buffer_desc.ptail=(int*)(conn_descriptors[number_of_conn].remote_buffer_desc.phead+1);
		conn_descriptors[number_of_conn].remote_buffer_desc.buffer=(long*)(conn_descriptors[number_of_conn].remote_buffer_desc.ptail+1);
		conn_descriptors[number_of_conn].remote_buffer_desc.elements=DEFAULT_BUFFER_SIZE;
		
		
		printk("Ptail %d Phead %d\n",*conn_descriptors[number_of_conn].remote_buffer_desc.ptail,*conn_descriptors[number_of_conn].remote_buffer_desc.phead);
		int i;
		BARRIER(epd,"DMA mapping Done_PP");
		printk("Connection Done %d\n",number_of_conn);

	}
	is_connection_done=PCN_CONN_CONNECTED;
	up(&send_connDone);
	printk("Connection Done...PCN_SEND Thread\n");
	
	while(1)
	{
		wait_data=dq_send();
	
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
		
while(TRUE){		

		if(kthread_should_stop()){
				scif_close(dataepd);
				return 0;
		}
	
		msg = (struct pcn_kmsg_message*)vmalloc(2*PAGE_SIZE);
		dflt_size = sizeof(struct pcn_kmsg_message);
		curr_addr = (char*) msg;
		
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
			tmp=(struct pcn_kmsg_message*)curr_addr;
			msg_size=tmp->hdr.size;
			
			//printk("Size %d NoRcv %d\n",tmp->hdr.size,no_bytes);
			if(msg_size<=sizeof(struct pcn_kmsg_message))
			{
				//printk("Must be NoNDMA Small\n");
			}
			else if(msg_size>dma_rcv_thresh)
			{
				dma_rcv_index=tmp->hdr.slot;
			//	printk("Must be DMA \n");
			
				vfree(msg);
				//printk("Offset %d + index %d message address %x \n",DMA_MSG_OFFSET,dma_rcv_index,dma_rcv_buffer+DMA_MSG_OFFSET*dma_rcv_index);
				msg=dma_rcv_buffer+DMA_MSG_OFFSET*dma_rcv_index;
			}
			else
			{
				//printk("Must be NoN-DMA Large %d\n",tmp->hdr.size);
				curr_size = tmp->hdr.size - dflt_size;	
				curr_addr = (char*) msg+no_bytes;
				while((no_bytes = scif_recv(newepd, curr_addr, curr_size, SCIF_RECV_BLOCK)) >= 0)
				{
					curr_addr = curr_addr + no_bytes;
					curr_size = curr_size - no_bytes;
					if(curr_size == 0)
						break ;
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

int pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg){
	
	
	if( pcn_connection_staus()==PCN_CONN_WATING)
		return -1;
	return pcn_kmsg_send_long(dest_cpu,msg,sizeof(struct pcn_kmsg_message)-sizeof(struct pcn_kmsg_hdr));
}

int pcn_kmsg_send_long(unsigned int dest_cpu, struct pcn_kmsg_long_message *lmsg, unsigned int payload_size){


	
	if( pcn_connection_staus()==PCN_CONN_WATING)
		return -1;
	
	int free_index;		
	int no_bytes,data_send=0;
	char * curr_addr;
	int my_ticket;
	my_ticket=fetch_and_add(&ticket, 1);
	int conn_no=my_ticket%MAX_CONNEC;
	
	int curr_size;
	lmsg->hdr.size = payload_size+sizeof(struct pcn_kmsg_hdr);	
	lmsg->hdr.from_cpu = my_cpu;
	lmsg->hdr.conn_no= conn_no;
	
	//lmsg->hdr.is_lg_msg = 1;
	//lmsg->hdr.size=curr_size;
	unsigned long start_time[10], finish_time[10];

	int i;
	

	//printk("%s:My Channel is %d current ticket %d conn_ticket %d\n",__func__,conn_no,my_ticket,conn_descriptors[conn_no].ticket);
	
	
	//printk("%s: is called\n",__func__);
	if(dest_cpu==my_cpu)
	{
		rcv_wait *exec_data = NULL;
		while(exec_data==NULL)
		{
				exec_data = kmalloc(sizeof(rcv_wait),GFP_KERNEL);
		}
		exec_data->msg=vmalloc(lmsg->hdr.size);
		memcpy(exec_data->msg,lmsg,lmsg->hdr.size);
		//exec_data->msg=wait_data->msg;
		
		enq_rcv(excution_qs[conn_no],exec_data);		
	//	conn_descriptors[conn_no].ticket+=MAX_CONNEC;
		//printk("%s: Next ticket %d\n",__func__,conn_descriptors[conn_no].ticket);
		//printk("%s: This is a selfie...\n",__func__);
		return lmsg->hdr.size;
	}
	
	
/*	
	int loop=0;
	while(my_ticket!=conn_descriptors[conn_no].ticket);
	{
		if ( !(++loop % MAX_LOOPS) )
		{
			//schedule_timeout(MAX_LOOPS_JIFFIES);
			udelay(10);
		}
			
	}
*/
	//spin_lock(&send_lock_pp);	
	curr_size = lmsg->hdr.size;
	if(lmsg->hdr.size>dma_send_thresh)
	{
		spin_lock(&conn_descriptors[conn_no].send_lock);		
		while(buffer_get(&conn_descriptors[conn_no].dma_buffer_decs,&free_index)!=1)
		{
		
		}
		spin_unlock(&conn_descriptors[conn_no].send_lock);	
found:
			//printk("%s: slot %d \n",__func__,free_index);
			lmsg->hdr.slot=free_index;
			curr_addr = (char*) lmsg;
			int sts_from_peer=-1;
		//	printk("%s:Through DMA MSG size %d\n",__func__,lmsg->hdr.size);
			memcpy(conn_descriptors[conn_no].dma_send_buffer+free_index*DMA_MSG_OFFSET,curr_addr,lmsg->hdr.size);
		//	printk("%s:Through DMA MSG size %d\n",__func__,lmsg->hdr.size);
		//	printk("%s\n",dma_send_buffer);
			err=scif_writeto(conn_descriptors[conn_no].send_epd, conn_descriptors[conn_no].send_buffer+free_index*DMA_MSG_OFFSET, lmsg->hdr.size, 0x80000+free_index*DMA_MSG_OFFSET, SCIF_RMA_SYNC);
			if(err<0)
			{
				printk("error in DMA Transfer %d",err);
				spin_unlock(&conn_descriptors[conn_no].send_lock);
				goto _out;
			}
			if((no_bytes = scif_send(conn_descriptors[conn_no].send_epd, curr_addr, sizeof(struct pcn_kmsg_message), SCIF_SEND_BLOCK)<0))
			{
				//printk("%s:Through NON-DMA\n",__func__);
				if(no_bytes==-ENODEV)
				{
					printk("%s:Terminatind pcn_sendD\n",__func__);
					//up(&wait_data->_sem);
					data_send=no_bytes;
					is_connection_done = PCN_CONN_WATING;
					spin_unlock(&conn_descriptors[conn_no].send_lock);
					return;
				}
				printk("Some thing went wrong Send Failed");
				data_send=no_bytes;
				//spin_unlock(&conn_descriptors[conn_no].send_lock);
				goto _out;
			}
	/*		scif_recv(send_epd,&sts_from_peer,sizeof(int),SCIF_RECV_BLOCK);
			if(sts_from_peer!=DMA_DONE)
			{
			printk("DMA send failed 2\n");
			/goto _out;
			}
	*/ 
			data_send=lmsg->hdr.size;
	//		goto _out;
		
	}
	else
	{
		spin_lock(&conn_descriptors[conn_no].send_lock);
		//printk("Non_dma Send\n");
		curr_addr = (char*) lmsg;
		curr_size = lmsg->hdr.size;
		while((no_bytes = scif_send(conn_descriptors[conn_no].send_epd, curr_addr, curr_size, 0)) >= 0){
					
					
					if(no_bytes==-ENODEV)
					{
						printk("%s:Terminatind pcn_sendD\n",__func__);
						//up(&wait_data->_sem);
						data_send=no_bytes;
						is_connection_done = PCN_CONN_WATING;
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
					//printk("%s: total %d nobytes %d\n",__func__,lmsg->hdr.size,no_bytes);
		}	
		spin_unlock(&conn_descriptors[conn_no].send_lock);
		
	}
_out:
//conn_descriptors[conn_no].ticket+=MAX_CONNEC;
//printk("%s: Next ticket %d\n",__func__,conn_descriptors[conn_no].ticket);
//spin_unlock(&send_lock_pp);
//printk("%s: %d",__func__,data_send);
return (data_send<0?-1:data_send);
	
}

void pcn_kmsg_free_msg(void *msg){
	
	struct pcn_kmsg_message *msgPtr = (struct pcn_kmsg_message*)msg;
	if(msgPtr->hdr.size>dma_rcv_thresh)
	{
	//	spin_lock(&conn_descriptors[msgPtr->hdr.conn_no].dma_q_free);
	//	printk("%s: conn_no %d slot %d\n",__func__,msgPtr->hdr.conn_no,msgPtr->hdr.slot);
		buffer_put(&conn_descriptors[msgPtr->hdr.conn_no].remote_buffer_desc,msgPtr->hdr.slot);
		//freeList[msgPtr->hdr.slot]=BUFFER_FREE;
	//	spin_unlock(&conn_descriptors[msgPtr->hdr.conn_no].dma_q_free);
	}
	else
		vfree(msg);
}


EXPORT_SYMBOL(pcn_kmsg_free_msg);
EXPORT_SYMBOL(pcn_kmsg_send_long);
EXPORT_SYMBOL(pcn_kmsg_send);
EXPORT_SYMBOL(pcn_kmsg_unregister_callback);
EXPORT_SYMBOL(pcn_kmsg_register_callback);
//EXPORT_SYMBOL(epd_fd);
//EXPORT_SYMBOL(test_func);






 
 
