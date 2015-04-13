/*
 * pcn_scif.c - Kernel Module for Popcorn Messaging Layer over Intel SCIF
 * 
 * Initial Fail safe Masseging layer
 * B M Saif Ansary <bmsaif86@vt.edu> 2014
 */
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
extern 	int __scif_flush(ep);

#define min_val(x,y) ((x)<=(y)?(x):(y))
#define FALSE 0
#define TRUE 1
#define PORT_DATA_IN 10
#define PORT_DATA_OUT 11
#define START_OFFSET 0x80000
#define BACKLOG 5 // Length of incoming message queue for control messages
#define _NO_DMA_
#define DMA_THRESH 512
#define DMA_DONE 1
#define NO_PAGES_MAPPED 400
#define DMA_MSG_OFFSET 2*PAGE_SIZE
#define DEFAULT_BUFFER_SIZE 200

int connection_handler(void *arg0);
int send_thread(void* arg0);
int executer_thread(void* arg0);
int test_thread(void* arg0);


scif_epd_t send_epd;

int msg_count;
int err=0,control_msg=1;

int is_connection_done=PCN_CONN_WATING;

extern scif_epd_t scif_open(void);

static int dma_rcv_thresh,dma_send_thresh;


int pcn_connection_staus()
{
	return is_connection_done;
}

#define BARRIER(newepd, string) { \
	printk("%s\n", string); \
	scif_send(newepd, &control_msg, sizeof(control_msg), 1); \
	scif_recv(newepd, &control_msg, sizeof(control_msg), 1); \
	printk("==============================================================\n");\
}

struct buffer_desc {
  long* buffer;
  int*  phead;
  int*  ptail;
  int   elements;
};



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

typedef struct _rcv_wait{
	struct list_head list;
	void * msg;
}rcv_wait;

//struct semaphore send_q_mutex;
struct semaphore send_q_empty;
struct semaphore rcv_q_empty;

struct semaphore send_connDone;
struct semaphore rcv_connDone;

DEFINE_SPINLOCK(send_q_mutex); 
static send_wait send_wait_q;

DEFINE_SPINLOCK(rcv_q_mutex); 
static rcv_wait rcv_wait_q;

DEFINE_SPINLOCK(send_lock); 

DEFINE_SPINLOCK(dma_q_free); 


static int __init initialize(void);
static void __exit unload(void);

unsigned int my_cpu;

pcn_kmsg_cbftn callbacks[PCN_KMSG_TYPE_MAX];
struct task_struct *handler;
struct task_struct *sender_handler;
struct task_struct *exect_handler;
struct task_struct *test_handler;
char *dma_send_buffer;
off_t send_buffer;
volatile char * freeList;
volatile char * remote_free_list;

struct buffer_desc dma_buffer_decs,remote_buffer_desc;


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



static inline int buffer_empty(struct buffer_desc * desc)
{
	return ((*desc->phead) == (*desc->ptail));
}

static inline int buffer_full(struct buffer_desc * desc)
{
	return ((((*desc->phead) +1) % desc->elements) == (*desc->ptail));
}


int buffer_get(struct buffer_desc * desc, long * value) 
{
  if (!value || !desc)
    return -1;
  if (buffer_empty(desc))
    return 0;
  *value = desc->buffer[*desc->ptail];
  *desc->ptail = (*desc->ptail +1) % desc->elements;
  return 1;
}

int buffer_put(struct buffer_desc * desc, long value)
{
  if (!desc)
    return -1;
  int local_head=*desc->phead;
  //if (buffer_full(desc))
  //  return 0;
  desc->buffer[local_head] = value;
  *desc->phead = (local_head +1) % desc->elements;
  return 1;
}




scif_epd_t test_func()
{
	return send_epd;
	
}





void enq_send(send_wait *strc)
{
	spin_lock(&send_q_mutex);
	INIT_LIST_HEAD(&(strc->list));
	list_add_tail(&(strc->list), &(send_wait_q.list));
	up(&send_q_empty);
	spin_unlock(&send_q_mutex);
}

send_wait * dq_send(void)
{
	send_wait *tmp;
	down_interruptible(&send_q_empty);
	spin_lock(&send_q_mutex);
	if(list_empty(&send_wait_q.list)){
		printk("List is empty...\n");
		spin_unlock(&send_q_mutex);
		return NULL;	
	}
	else{
		tmp = list_first_entry (&send_wait_q.list, send_wait, list);
		list_del(send_wait_q.list.next);
		spin_unlock(&send_q_mutex);
		return tmp;
	}
}



void enq_rcv(rcv_wait *strc)
{
	spin_lock(&rcv_q_mutex);
	INIT_LIST_HEAD(&(strc->list));
	list_add_tail(&(strc->list), &(rcv_wait_q.list));
	up(&rcv_q_empty);
	spin_unlock(&rcv_q_mutex);
}

rcv_wait * dq_rcv(void)
{
	rcv_wait *tmp;
	down_interruptible(&rcv_q_empty);
	spin_lock(&rcv_q_mutex);
	if(list_empty(&rcv_wait_q.list)){
		printk("List is empty...\n");
		spin_unlock(&rcv_q_mutex);
		return NULL;	
	}
	else{
		tmp = list_first_entry (&rcv_wait_q.list, rcv_wait, list);
		list_del(rcv_wait_q.list.next);
		spin_unlock(&rcv_q_mutex);
		return tmp;
	}
}



// Initialize callback table to null, set up control and data channels
static int __init initialize(){
	uint16_t fromcpu;
	int i;
	INIT_LIST_HEAD(&send_wait_q.list);
	INIT_LIST_HEAD(&rcv_wait_q.list);
	
	printk("In pcn new messaging layer init\n");	
	//sema_init(&(send_q_mutex), 0);
	sema_init(&(send_q_empty), 0);
	sema_init(&(rcv_q_empty), 0);
	
		
	sema_init(&send_connDone,0);
	sema_init(&rcv_connDone,0);
	
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
	handler = kthread_run(connection_handler, &callbacks, "pcnscifd");
	if(handler < 0){
		printk(KERN_INFO "kthread_run failed! Messaging Layer not initialized\n");
		return (long long int)handler;
	}
	
	sender_handler = kthread_run(send_thread, NULL, "pcnscif_sendD");
	if(sender_handler < 0){
		printk(KERN_INFO "kthread_run failed! Messaging Layer not initialized\n");
		return (long long int)sender_handler;
	}
	exect_handler = kthread_run(executer_thread, NULL, "pcnscif_execD");
	if(exect_handler < 0){
		printk(KERN_INFO "kthread_run failed! Messaging Layer not initialized\n");
		return (long long int)exect_handler;
	}
	if(my_cpu!=0)
	{
		down_interruptible(&send_connDone);
		down_interruptible(&rcv_connDone);
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

int handle_selfie_test(struct pcn_kmsg_message* inc_msg)
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

int executer_thread(void* arg0)
{
	rcv_wait * wait_data;
	pcn_kmsg_cbftn ftn;
	while(1)
	{
		wait_data=dq_rcv();
		
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

int send_thread(void* arg0)
{
	uint16_t fromcpu;
	scif_epd_t epd;
	int rc;
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
	
	
	if((epd = scif_open()) == NULL){
		printk(KERN_INFO "scif_open failed! Could not send message.\n");
		return (long long int)epd;
	}
	if((rc = scif_bind(epd, 0)) < 0){
		printk(KERN_INFO "Send Long: scif_bind failed with error %d. Could not send message.\n", rc);
		return -1;
	}
	portID.node = dest_cpu;
	portID.port = PORT_DATA_IN;
	while((rc = scif_connect(epd, &portID)) < 0){
		msleep(1000);
		//printk(KERN_INFO "scif_connect failed with error %d! Could not send message\n", rc);
		//return rc;
	}
	dma_send_buffer=vmalloc((NO_PAGES_MAPPED+1)*PAGE_SIZE);
	send_buffer = scif_register(epd, dma_send_buffer, (NO_PAGES_MAPPED+1)*PAGE_SIZE, 0x80000, SCIF_PROT_WRITE | SCIF_PROT_READ, SCIF_MAP_KERNEL | SCIF_MAP_FIXED);
	BARRIER(epd,"Register Send");
	struct pcn_kmsg_long_message *lmsg;
	int use_dma=0;
	is_connection_done=PCN_CONN_CONNECTED;
	
	send_epd = epd;
	//printk("%s: epd %d send_epd %d\n",__func__,epd,send_epd);
	struct scif_range *pages;
	
	err=scif_get_pages(epd,0x80000+(NO_PAGES_MAPPED*PAGE_SIZE),PAGE_SIZE,&pages);
	
	printk("%s: nPages %d err %d phy_addr %x\n",__func__,pages->nr_pages,err,pages->phys_addr);
	
	remote_free_list=ioremap(pages->phys_addr[0],PAGE_SIZE);
	
	
	remote_buffer_desc.phead= (int*)(remote_free_list);
	remote_buffer_desc.ptail=(int*)(remote_buffer_desc.phead+1);
	remote_buffer_desc.buffer=(long*)(remote_buffer_desc.ptail+1);
	remote_buffer_desc.elements=DEFAULT_BUFFER_SIZE;
	
	
	printk("Ptail %d Phead %d\n",*remote_buffer_desc.ptail,*remote_buffer_desc.phead);
	int i;
/*	for(i=0;i<DEFAULT_BUFFER_SIZE;i++)
	{
		printk("%d ",(remote_buffer_desc.buffer[i]));
		if((i%20)==0)
			printk("\n");
	}
*/	
	
	up(&send_connDone);
	printk("Connection Done...PCN_SEND Thread\n");
	
	while(1){
		int data_send=-1;
		wait_data=dq_send();
		
		if(wait_data!=NULL)
			;
	//		printk("Got data ");
		else
		{
		//	printk("NULL data ");
			return 0;
		}
		lmsg=wait_data->msg;
/*		
		if(lmsg->hdr.type==PCN_KMSG_TERMINATE)
		{
			printk("%s:Terminatind pcn_sendD\n",__func__);
			up(&wait_data->_sem);
			is_connection_done = PCN_CONN_WATING;
			return;
		}
		
*/	
		char *curr_addr = (char*) lmsg;
		lmsg->hdr.from_cpu = my_cpu;
		//lmsg->hdr.is_lg_msg = 1;
		curr_size = lmsg->hdr.size;
		int sts_from_peer=0;
		int err;
		if(wait_data->dst_cpu==my_cpu)
		{
			rcv_wait *exec_data = NULL;
			while(exec_data==NULL)
			{
				exec_data = kmalloc(sizeof(rcv_wait),GFP_KERNEL);
			}
			exec_data->msg=vmalloc(lmsg->hdr.size);
			memcpy(exec_data->msg,lmsg,lmsg->hdr.size);
			//exec_data->msg=wait_data->msg;
			enq_rcv(exec_data);
			
			//printk("%s: This is a selfie...\n",__func__);
			goto _out;
		}
		
		
		if(lmsg->hdr.size>DMA_THRESH)
		{
			sts_from_peer=-1;
			memcpy(dma_send_buffer,curr_addr+sizeof(struct pcn_kmsg_message),lmsg->hdr.size-sizeof(struct pcn_kmsg_message));
		//	printk("%s:Through DMA\n",__func__);
			//printk("%s\n",dma_send_buffer);
			err=scif_writeto(epd, send_buffer, lmsg->hdr.size-sizeof(struct pcn_kmsg_message), 0x80000, SCIF_RMA_SYNC);
			if(err<0)
			{
				printk("error in DMA Transfer %d",err);
				goto _out;
			}
			if((no_bytes = scif_send(epd, curr_addr, sizeof(struct pcn_kmsg_message), SCIF_SEND_BLOCK)<0))
			{
				//printk("%s:Through NON-DMA\n",__func__);
				if(no_bytes==-ENODEV)
				{
					printk("%s:Terminatind pcn_sendD\n",__func__);
					up(&wait_data->_sem);
					data_send=no_bytes;
					is_connection_done = PCN_CONN_WATING;
					return;
				}
				printk("Some thing went wrong Send Failed");
				data_send=no_bytes;
				goto _out;
			}
			scif_recv(epd,&sts_from_peer,sizeof(int),SCIF_RECV_BLOCK);
			if(sts_from_peer!=DMA_DONE)
			{
				printk("DMA send failed 2\n");
				goto _out;
			}
			data_send=lmsg->hdr.size;
		}
		//printk("Got data sending...type %d size %d\n",lmsg->hdr.is_lg_msg,curr_size);
		
		else{
			
			while((no_bytes = scif_send(epd, curr_addr, curr_size, SCIF_SEND_BLOCK)) >= 0){
				
				
				if(no_bytes==-ENODEV)
				{
					printk("%s:Terminatind pcn_sendD\n",__func__);
					up(&wait_data->_sem);
					data_send=no_bytes;
					is_connection_done = PCN_CONN_WATING;
					return;
				}
				
				if(no_bytes<0)
				{
					printk("Some thing went wrong Send Failed");
				}
				curr_addr = curr_addr + no_bytes;
				curr_size = curr_size - no_bytes;
				data_send=data_send+no_bytes;
				if(curr_size == 0)
					break;
			}
		}
_out:		
		wait_data->error=data_send;
		up(&wait_data->_sem);
		

		if(kthread_should_stop()){
			scif_close(epd);
			return 0;
		}
	}
}

int connection_handler(void *arg0){
	int rc;
	int cmd;
	scif_epd_t newepd;
	int dflt_size;
	struct pcn_kmsg_message *msg,*tmp,*msg_del;
//	pcn_kmsg_cbftn ftn;
	struct scif_portID portID;
	int no_bytes, curr_size;
	char *curr_addr;
	rcv_wait *wait_data;
//	pcn_kmsg_cbftn* cbtbl = (pcn_kmsg_cbftn*)arg0;
	scif_epd_t dataepd = scif_open();
	if(dataepd == SCIF_OPEN_FAILED){
		printk(KERN_INFO "scif_open failed! Messaging layer not initialized\n");
		return (long long int)NULL;
	}
	char *dma_rcv_buffer;
	off_t buffer;
	
	int dma_rcv_index=-1;
	rc = scif_bind(dataepd, PORT_DATA_IN);
	if(rc != PORT_DATA_IN){
		printk(KERN_INFO "Connection Handler: scif_bind failed with error code %d! Messaging layer not initialized\n", rc);
		scif_close(dataepd);
		return (long long int)NULL;
	}
	rc = scif_listen(dataepd, BACKLOG);
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
		
		dma_rcv_buffer=vmalloc((NO_PAGES_MAPPED+1)*PAGE_SIZE);
		buffer = scif_register(newepd, dma_rcv_buffer, (NO_PAGES_MAPPED+1)*PAGE_SIZE, 0x80000, SCIF_PROT_WRITE | SCIF_PROT_READ, SCIF_MAP_KERNEL | SCIF_MAP_FIXED);
		int sts_msg=DMA_DONE;
		int msg_size;
			
		dma_buffer_decs.phead = (int*)(dma_rcv_buffer+(NO_PAGES_MAPPED)*PAGE_SIZE);
		dma_buffer_decs.ptail= (int*)(dma_buffer_decs.phead + 1);
		dma_buffer_decs.buffer= (long*)(dma_buffer_decs.ptail +1);
		dma_buffer_decs.elements=DEFAULT_BUFFER_SIZE;
		int index;
		for(index=0;index<DEFAULT_BUFFER_SIZE;index++)
		{
			dma_buffer_decs.buffer[index]=index;
		}
		*(dma_buffer_decs.phead)=DEFAULT_BUFFER_SIZE-1;
		*(dma_buffer_decs.ptail)=0;
	//	memset(freeList,0x0,PAGE_SIZE);
	//	printk("%s: phy_addr %x\n",__func__,virt_to_phys(freeList));
		BARRIER(newepd,"Register RCV");
		up(&rcv_connDone);
		
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
			
	//		printk("Size %d NoRcv %d\n",tmp->hdr.size,no_bytes);
			if(msg_size<=sizeof(struct pcn_kmsg_message))
			{
				//printk("Must be NoNDMA Small\n");
			}
			else if(msg_size>dma_rcv_thresh)
			{
				dma_rcv_index=tmp->hdr.slot;
			//	printk("Must be DMA \n");
				
/*				if(msg_size>sizeof(struct pcn_kmsg_long_message))
				{
					//printk("DMA Huge Message size %d\n",tmp->hdr.size);
					msg_del=msg;
					msg=vmalloc(msg_size);
					if(msg==NULL)
					{
						printk("Can't Vmalloc..\n");
						continue;
					}
					memcpy(msg,msg_del,sizeof(struct pcn_kmsg_message));
					curr_addr=(char*)msg;
					vfree(msg_del);
				}
*/ 
				vfree(msg);
		//		printk("Offset %d + index %d message address %x \n",DMA_MSG_OFFSET,dma_rcv_index,dma_rcv_buffer+DMA_MSG_OFFSET*dma_rcv_index);
				msg=dma_rcv_buffer+DMA_MSG_OFFSET*dma_rcv_index;
				//memcpy(curr_addr+dflt_size,dma_rcv_buffer,msg_size-dflt_size);
				//printk("%s\n",msg);
				//scif_send(newepd, &sts_msg, sizeof(int), SCIF_SEND_BLOCK);
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
			enq_rcv(wait_data);
			
	}
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
	int curr_size;
	lmsg->hdr.size = payload_size+sizeof(struct pcn_kmsg_hdr);	
	lmsg->hdr.from_cpu = my_cpu;
	//lmsg->hdr.is_lg_msg = 1;
	//lmsg->hdr.size=curr_size;
	unsigned long start_time[10], finish_time[10];

	int i;
	//printk("%s: noc %d\n",__func__,num_chunks);
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
		enq_rcv(exec_data);		
		//printk("%s: This is a selfie...\n",__func__);
		return lmsg->hdr.size;
	}
	
	spin_lock(&send_lock);	
	curr_size = lmsg->hdr.size;
	if(lmsg->hdr.size>dma_send_thresh)
	{
			
		while(buffer_get(&dma_buffer_decs,&free_index)!=1)
		{
			printk("%s in while pid %d\n",__func__,current->pid);
		}

found:
			//printk("%s: slot %d \n",__func__,free_index);
			lmsg->hdr.slot=free_index;
			curr_addr = (char*) lmsg;
			int sts_from_peer=-1;
		//	printk("%s:Through DMA MSG size %d\n",__func__,lmsg->hdr.size);
			memcpy(dma_send_buffer+free_index*DMA_MSG_OFFSET,curr_addr,lmsg->hdr.size);
			//printk("%s:Through DMA MSG size %d\n",__func__,lmsg->hdr.size);
			//printk("%s\n",dma_send_buffer);
			err=scif_writeto(send_epd, send_buffer+free_index*DMA_MSG_OFFSET, lmsg->hdr.size, 0x80000+free_index*DMA_MSG_OFFSET, SCIF_RMA_SYNC);
			if(err<0)
			{
				printk("error in DMA Transfer %d",err);
				goto _out;
			}
			if((no_bytes = scif_send(send_epd, curr_addr, sizeof(struct pcn_kmsg_message), SCIF_SEND_BLOCK)<0))
			{
				//printk("%s:Through NON-DMA\n",__func__);
				if(no_bytes==-ENODEV)
				{
					printk("%s:Terminatind pcn_sendD\n",__func__);
					//up(&wait_data->_sem);
					data_send=no_bytes;
					is_connection_done = PCN_CONN_WATING;
					return;
				}
				printk("Some thing went wrong Send Failed");
				data_send=no_bytes;
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
			goto _out;
	}
	else
	{
		//printk("Heloooooo\n");
		curr_addr = (char*) lmsg;
		curr_size = lmsg->hdr.size;
		while((no_bytes = scif_send(send_epd, curr_addr, curr_size, 0)) >= 0){
					
					
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
	}
	
/*	for(i=0;i<num_chunks;i++)
	{
		printk("%s: Chunck %d send %llu\n",__func__,i,finish_time[i]-start_time[i]);
	}
*/	
//__scif_flush(send_epd);	
_out:
spin_unlock(&send_lock);
//printk("%s: %d",__func__,data_send);
return (data_send<0?-1:data_send);
	
}

inline void pcn_kmsg_free_msg(void *msg){
	
	struct pcn_kmsg_message *msgPtr = (struct pcn_kmsg_message*)msg;
	if(msgPtr->hdr.size>dma_rcv_thresh)
	{
		spin_lock(&dma_q_free);
		buffer_put(&remote_buffer_desc,msgPtr->hdr.slot);
		//freeList[msgPtr->hdr.slot]=BUFFER_FREE;
		spin_unlock(&dma_q_free);
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
EXPORT_SYMBOL(test_func);






 
 
