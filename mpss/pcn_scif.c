/*
 * pcn_scif.c - Kernel Module for Popcorn Messaging Layer over Intel SCIF
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/pcn_kmsg.h>
#include "./scif.h"

#define FALSE 0
#define TRUE 1
#define PORT_DATA_IN 10
#define PORT_DATA_OUT 11
#define START_OFFSET 0x80000
#define BACKLOG 5 // Length of incoming message queue for control messages
#define _NO_DMA_
#define DMA_THRESH 2048
#define DMA_DONE 1

int connection_handler(void *arg0);
int send_thread(void* arg0);
int executer_thread(void* arg0);
int test_thread(void* arg0);

int msg_count;
int err=0,control_msg=1;

int is_connection_done=PCN_CONN_WATING;

extern scif_epd_t scif_open(void);

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




static int __init initialize(void);
static void __exit unload(void);

unsigned int my_cpu;

pcn_kmsg_cbftn callbacks[PCN_KMSG_TYPE_MAX];
struct task_struct *handler;
struct task_struct *sender_handler;
struct task_struct *exect_handler;
struct task_struct *test_handler;

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
			vfree(msg);
		}else{
			ftn = callbacks[msg->hdr.type];
			if(ftn != NULL){
				ftn(msg);
			}else{
				printk(KERN_INFO "Recieved message type %d size %d has no registered callback!\n", msg->hdr.type,msg->hdr.size,msg_count++);
				vfree(msg);
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
	char *dma_send_buffer;
	off_t buffer;
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
	dma_send_buffer=vmalloc(1024*PAGE_SIZE);
	buffer = scif_register(epd, dma_send_buffer, 1024*PAGE_SIZE, 0x80000, SCIF_PROT_WRITE | SCIF_PROT_READ, SCIF_MAP_KERNEL | SCIF_MAP_FIXED);
	BARRIER(epd,"Register Send");
	struct pcn_kmsg_long_message *lmsg;
	int use_dma=0;
	is_connection_done=PCN_CONN_CONNECTED;
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
			//printk("%s:Through DMA\n",__func__);
			//printk("%s\n",dma_send_buffer);
			err=scif_writeto(epd, buffer, lmsg->hdr.size-sizeof(struct pcn_kmsg_message), 0x80000, SCIF_RMA_SYNC);
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
		
		dma_rcv_buffer=vmalloc(1024*PAGE_SIZE);
		buffer = scif_register(newepd, dma_rcv_buffer, 1024*PAGE_SIZE, 0x80000, SCIF_PROT_WRITE | SCIF_PROT_READ, SCIF_MAP_KERNEL | SCIF_MAP_FIXED);
		int sts_msg=DMA_DONE;
		int msg_size;
		
		BARRIER(newepd,"Register RCV");
		up(&rcv_connDone);
		
while(TRUE){		

		if(kthread_should_stop()){
				scif_close(dataepd);
				return 0;
		}
	
		msg = (struct pcn_kmsg_message*)vmalloc(sizeof(struct pcn_kmsg_long_message));
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
			else if(msg_size>DMA_THRESH)
			{
				//printk("Must be DMA \n");
				if(msg_size>sizeof(struct pcn_kmsg_long_message))
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
				memcpy(curr_addr+dflt_size,dma_rcv_buffer,msg_size-dflt_size);
				//printk("%s\n",msg);
				scif_send(newepd, &sts_msg, sizeof(int), SCIF_SEND_BLOCK);
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
	
	
	/*if( pcn_connection_staus()==PCN_CONN_WATING)
		return -1;
	send_wait wait_ptr;
	sema_init(&wait_ptr._sem,0);
	msg->hdr.from_cpu = my_cpu;
	msg->hdr.is_lg_msg = 0;
	wait_ptr.msg=msg;
	wait_ptr.dst_cpu=dest_cpu;
	msg->hdr.size=sizeof(struct pcn_kmsg_message);
	enq_send(&wait_ptr);
	down(&wait_ptr._sem);
	return (wait_ptr.error<0) ? -1 : wait_ptr.error;*/
	pcn_kmsg_send_long(dest_cpu,(struct pcn_kmsg_long_message *) msg, 64 - sizeof(struct pcn_kmsg_hdr));
}

int pcn_kmsg_send_long(unsigned int dest_cpu, struct pcn_kmsg_long_message *lmsg, unsigned int payload_size){
	if( pcn_connection_staus()==PCN_CONN_WATING)
		return -1;
	send_wait wait_ptr;
	sema_init(&wait_ptr._sem,0);
	lmsg->hdr.from_cpu = my_cpu;
	lmsg->hdr.is_lg_msg = 1;
	wait_ptr.msg=lmsg;
	wait_ptr.dst_cpu=dest_cpu;
	lmsg->hdr.size=sizeof(struct pcn_kmsg_hdr) + payload_size;
	enq_send(&wait_ptr);
	down(&wait_ptr._sem);
	return (wait_ptr.error<0) ? -1 : wait_ptr.error;
}

inline void pcn_kmsg_free_msg(void *msg){
	vfree(msg);
}


EXPORT_SYMBOL(pcn_kmsg_free_msg);
EXPORT_SYMBOL(pcn_kmsg_send_long);
EXPORT_SYMBOL(pcn_kmsg_send);
EXPORT_SYMBOL(pcn_kmsg_unregister_callback);
EXPORT_SYMBOL(pcn_kmsg_register_callback);






 
 
