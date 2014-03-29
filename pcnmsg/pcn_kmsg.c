/*
 * Inter-kernel messaging support for Popcorn
 *
 * Current ver: Antonio Barbalace, Phil Wilshire 2013
 * First ver: Ben Shelton <beshelto@vt.edu> 2013
 */

#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/smp.h>
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/multikernel.h>
#include <linux/pcn_kmsg.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>

#include <asm/system.h>
#include <asm/apic.h>
#include <asm/hardirq.h>
#include <asm/setup.h>
#include <asm/bootparam.h>
#include <asm/errno.h>
#include <asm/atomic.h>
#include <linux/delay.h>

#define LOGLEN 4
#define LOGCALL 32

#define KMSG_VERBOSE 0

#if KMSG_VERBOSE
#define KMSG_PRINTK(fmt, args...) printk("%s: " fmt, __func__, ##args)
#else
#define KMSG_PRINTK(...) ;
#endif


#define MCAST_VERBOSE 0

#if MCAST_VERBOSE
#define MCAST_PRINTK(fmt, args...) printk("%s: " fmt, __func__, ##args)
#else
#define MCAST_PRINTK(...) ;
#endif

#define KMSG_INIT(fmt, args...) printk("KMSG INIT: %s: " fmt, __func__, ##args)

#define KMSG_ERR(fmt, args...) printk("%s: ERROR: " fmt, __func__, ##args)

#define ROUND_PAGES(size) ((size/PAGE_SIZE) + ((size%PAGE_SIZE)? 1:0))
#define ROUND_PAGE_SIZE(size) (ROUND_PAGES(size)*PAGE_SIZE)

/* COMMON STATE */

/* table of callback functions for handling each message type */
pcn_kmsg_cbftn callback_table[PCN_KMSG_TYPE_MAX];

/* number of current kernel */
int my_cpu = 0; // NOT CORRECT FOR CLUSTERING!!! STILL WE HAVE TO DECIDE HOW TO IMPLEMENT CLUSTERING

/* pointer to table with phys addresses for remote kernels' windows,
 * owned by kernel 0 */
struct pcn_kmsg_rkinfo *rkinfo;

/* table with virtual (mapped) addresses for remote kernels' windows,
   one per kernel */
struct pcn_kmsg_window * rkvirt[POPCORN_MAX_CPUS];


/* lists of messages to be processed for each prio */
struct list_head msglist_hiprio, msglist_normprio;

/* array to hold pointers to large messages received */
//struct pcn_kmsg_container * lg_buf[POPCORN_MAX_CPUS];
struct list_head lg_buf[POPCORN_MAX_CPUS];
volatile unsigned long long_id;
int who_is_writing=-1;

/* action for bottom half */
static void pcn_kmsg_action(/*struct softirq_action *h*/struct work_struct* work);

/* workqueue for operations that can sleep */
struct workqueue_struct *kmsg_wq;
struct workqueue_struct *messaging_wq;

/* RING BUFFER */

#define RB_SHIFT 6
#define RB_SIZE (1 << RB_SHIFT)
#define RB_MASK ((1 << RB_SHIFT) - 1)

#define PCN_DEBUG(...) ;
//#define PCN_WARN(...) printk(__VA_ARGS__)
#define PCN_WARN(...) ;
#define PCN_ERROR(...) printk(__VA_ARGS__)

struct pcn_kmsg_hdr log_receive[LOGLEN];
struct pcn_kmsg_hdr log_send[LOGLEN];
int log_r_index=0;
int log_s_index=0;

void * log_function_called[LOGCALL];
int log_f_index=0;
int log_f_sendindex=0;
void * log_function_send[LOGCALL];

/* From Wikipedia page "Fetch and add", modified to work for u64 */
static inline unsigned long fetch_and_add(volatile unsigned long * variable, 
					  unsigned long value)
{
	asm volatile( 
		     "lock; xaddq %%rax, %2;"
		     :"=a" (value)                   //Output
		     : "a" (value), "m" (*variable)  //Input
		     :"memory" );
	return value;
}

static inline unsigned long win_inuse(struct pcn_kmsg_window *win) 
{
	return win->head - win->tail;
}
static long unsigned int msg_put=0;
static inline int win_put(struct pcn_kmsg_window *win, 
			  struct pcn_kmsg_message *msg,
			  int no_block) 
{
	unsigned long ticket;

	/* if we can't block and the queue is already really long, 
	   return EAGAIN */
	if (no_block && (win_inuse(win) >= RB_SIZE)) {
		KMSG_PRINTK("window full, caller should try again...\n");
		return -EAGAIN;
	}

	/* grab ticket */
	ticket = fetch_and_add(&win->head, 1);
	PCN_DEBUG(KERN_ERR "%s: ticket = %lu, head = %lu, tail = %lu\n", 
		 __func__, ticket, win->head, win->tail);

	KMSG_PRINTK("%s: ticket = %lu, head = %lu, tail = %lu\n",
			 __func__, ticket, win->head, win->tail);

	who_is_writing= ticket;
	/* spin until there's a spot free for me */
	//while (win_inuse(win) >= RB_SIZE) {}
	//if(ticket>=PCN_KMSG_RBUF_SIZE){
		while((win->buffer[ticket%PCN_KMSG_RBUF_SIZE].last_ticket != ticket-PCN_KMSG_RBUF_SIZE)) {
			//pcn_cpu_relax();
			msleep(1);
		}
		while(	win->buffer[ticket%PCN_KMSG_RBUF_SIZE].ready!=0){
			//pcn_cpu_relax();
			msleep(1);
		}
	//}
	/* insert item */
	memcpy(&win->buffer[ticket%PCN_KMSG_RBUF_SIZE].payload,
	       &msg->payload, PCN_KMSG_PAYLOAD_SIZE);

	memcpy((void*)&(win->buffer[ticket%PCN_KMSG_RBUF_SIZE].hdr),
	       (void*)&(msg->hdr), sizeof(struct pcn_kmsg_hdr));

	//log_send[log_s_index%LOGLEN]= win->buffer[ticket & RB_MASK].hdr;
	memcpy(&(log_send[log_s_index%LOGLEN]),
		(void*)&(win->buffer[ticket%PCN_KMSG_RBUF_SIZE].hdr),
		sizeof(struct pcn_kmsg_hdr));
	log_s_index++;

	win->second_buffer[ticket%PCN_KMSG_RBUF_SIZE]++;

	/* set completed flag */
	win->buffer[ticket%PCN_KMSG_RBUF_SIZE].ready = 1;
	wmb();
	win->buffer[ticket%PCN_KMSG_RBUF_SIZE].last_ticket = ticket;

	who_is_writing=-1;

msg_put++;

	return 0;
}

static long unsigned msg_get=0;
static inline int win_get(struct pcn_kmsg_window *win, 
			  struct pcn_kmsg_reverse_message **msg) 
{
	struct pcn_kmsg_reverse_message *rcvd;

	if (!win_inuse(win)) {

		KMSG_PRINTK("nothing in buffer, returning...\n");
		return -1;
	}

	KMSG_PRINTK("reached win_get, head %lu, tail %lu\n", 
		    win->head, win->tail);	

	/* spin until entry.ready at end of cache line is set */
	rcvd =(struct pcn_kmsg_reverse_message*) &(win->buffer[win->tail % PCN_KMSG_RBUF_SIZE]);
	//KMSG_PRINTK("%s: Ready bit: %u\n", __func__, rcvd->hdr.ready);


	while (!rcvd->ready) {

		//pcn_cpu_relax();
		msleep(1);

	}

	// barrier here?
	pcn_barrier();

	//log_receive[log_r_index%LOGLEN]=rcvd->hdr;
	memcpy(&(log_receive[log_r_index%LOGLEN]),&(rcvd->hdr),sizeof(struct pcn_kmsg_hdr));
	log_r_index++;

	//rcvd->hdr.ready = 0;

	*msg = rcvd;	
msg_get++;



	return 0;
}

static inline void win_advance_tail(struct pcn_kmsg_window *win) 
{
	win->tail++;
}

/* win_enable_int
 * win_disable_int
 * win_int_enabled
 *
 * These functions will inhibit senders to send a message while
 * the receiver is processing IPI from any sender.
 */
static inline void win_enable_int(struct pcn_kmsg_window *win) {
	        win->int_enabled = 1;
	        wmb(); // enforce ordering
}
static inline void win_disable_int(struct pcn_kmsg_window *win) {
	        win->int_enabled = 0;
	        wmb(); // enforce ordering
}
static inline unsigned char win_int_enabled(struct pcn_kmsg_window *win) {
    		rmb(); //not sure this is required (Antonio)
	        return win->int_enabled;
}

static inline int atomic_add_return_sync(int i, atomic_t *v)
{
	return i + xadd_sync(&v->counter, i);
}

static inline int atomic_dec_and_test_sync(atomic_t *v)
{
	unsigned char c;

	asm volatile("lock; decl %0; sete %1"
		     : "+m" (v->counter), "=qm" (c)
		     : : "memory");
	return c != 0;
}



/* INITIALIZATION */
#ifdef PCN_SUPPORT_MULTICAST
static int pcn_kmsg_mcast_callback(struct pcn_kmsg_message *message);
#endif /* PCN_SUPPORT_MULTICAST */

static void map_msg_win(pcn_kmsg_work_t *w)
{
	int cpu = w->cpu_to_add;

	if (cpu < 0 || cpu >= POPCORN_MAX_CPUS) {
		KMSG_ERR("invalid CPU %d specified!\n", cpu);
		return;
	}

	rkvirt[cpu] = ioremap_cache(rkinfo->phys_addr[cpu],
				  ROUND_PAGE_SIZE(sizeof(struct pcn_kmsg_window)));

	if (rkvirt[cpu]) {
		KMSG_INIT("ioremapped window, virt addr 0x%p\n", 
			  rkvirt[cpu]);
	} else {
		KMSG_ERR("failed to map CPU %d's window at phys addr 0x%lx\n",
			 cpu, rkinfo->phys_addr[cpu]);
	}
}

/* bottom half for workqueue */
static void process_kmsg_wq_item(struct work_struct * work)
{
	pcn_kmsg_work_t *w = (pcn_kmsg_work_t *) work;

	KMSG_PRINTK("called with op %d\n", w->op);

	switch (w->op) {
		case PCN_KMSG_WQ_OP_MAP_MSG_WIN:
			map_msg_win(w);
			break;

		case PCN_KMSG_WQ_OP_UNMAP_MSG_WIN:
			KMSG_ERR("%s: UNMAP_MSG_WIN not yet implemented!\n",
				 __func__);
			break;

#ifdef PCN_SUPPORT_MULTICAST
		case PCN_KMSG_WQ_OP_MAP_MCAST_WIN:
			map_mcast_win(w);
			break;

		case PCN_KMSG_WQ_OP_UNMAP_MCAST_WIN:
			KMSG_ERR("UNMAP_MCAST_WIN not yet implemented!\n");
			break;
#endif /* PCN_SUPPORT_MULTICAST */

		default:
			KMSG_ERR("Invalid work queue operation %d\n", w->op);

	}

	kfree(work);
}

inline void pcn_kmsg_free_msg(void * msg)
{
	kfree(msg - sizeof(struct list_head));
}

static int pcn_kmsg_checkin_callback(struct pcn_kmsg_message *message) 
{
	struct pcn_kmsg_checkin_message *msg = 
		(struct pcn_kmsg_checkin_message *) message;
	int from_cpu = msg->hdr.from_cpu;
	pcn_kmsg_work_t *kmsg_work = NULL;


	KMSG_INIT("From CPU %d, type %d, window phys addr 0x%lx\n", 
		  msg->hdr.from_cpu, msg->hdr.type, 
		  msg->window_phys_addr);

	if (from_cpu >= POPCORN_MAX_CPUS) {
		KMSG_ERR("Invalid source CPU %d\n", msg->hdr.from_cpu);
		return -1;
	}

	if (!msg->window_phys_addr) {

		KMSG_ERR("Window physical address from CPU %d is NULL!\n", 
			 from_cpu);
		return -1;
	}

	/* Note that we're not allowed to ioremap anything from a bottom half,
	   so we'll add it to a workqueue and do it in a kernel thread. */
	kmsg_work = kmalloc(sizeof(pcn_kmsg_work_t), GFP_ATOMIC);
	if (likely(kmsg_work)) {
		INIT_WORK((struct work_struct *) kmsg_work, 
			  process_kmsg_wq_item);
		kmsg_work->op = PCN_KMSG_WQ_OP_MAP_MSG_WIN;
		kmsg_work->from_cpu = msg->hdr.from_cpu;
		kmsg_work->cpu_to_add = msg->cpu_to_add;
		queue_work(kmsg_wq, (struct work_struct *) kmsg_work);
	} else {
		KMSG_ERR("Failed to malloc work structure!\n");
	}

	pcn_kmsg_free_msg(message);

	return 0;
}

static inline int pcn_kmsg_window_init(struct pcn_kmsg_window *window)
{
	window->head = 0;
	window->tail = 0;
	//memset(&window->buffer, 0,
	     //  PCN_KMSG_RBUF_SIZE * sizeof(struct pcn_kmsg_reverse_message));
	int i;
	for(i=0;i<PCN_KMSG_RBUF_SIZE;i++){
		window->buffer[i].last_ticket=i-PCN_KMSG_RBUF_SIZE;
		window->buffer[i].ready=0;
	}
	memset(&window->second_buffer, 0,
		       PCN_KMSG_RBUF_SIZE * sizeof(int));

	window->int_enabled = 1;
	return 0;
}

extern unsigned long orig_boot_params;

static int send_checkin_msg(unsigned int cpu_to_add, unsigned int to_cpu)
{
	int rc;
	struct pcn_kmsg_checkin_message msg;

	msg.hdr.type = PCN_KMSG_TYPE_CHECKIN;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.window_phys_addr = rkinfo->phys_addr[my_cpu];
	msg.cpu_to_add = cpu_to_add;

	rc = pcn_kmsg_send(to_cpu, (struct pcn_kmsg_message *) &msg);

	if (rc) {
		KMSG_ERR("Failed to send checkin message, rc = %d\n", rc);
		return rc;
	}

	return 0;
}

static int do_checkin(void)
{
	int rc = 0;
	int i;

	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (i == my_cpu) {
			continue;
		}

		if (rkinfo->phys_addr[i]) {
			rkvirt[i] = ioremap_cache(rkinfo->phys_addr[i],
						  ROUND_PAGE_SIZE(sizeof(struct pcn_kmsg_window)));

			if (rkvirt[i]) {
				KMSG_INIT("ioremapped CPU %d's window, virt addr 0x%p\n", 
					  i, rkvirt[i]);
			} else {
				KMSG_ERR("Failed to ioremap CPU %d's window at phys addr 0x%lx\n",
					 i, rkinfo->phys_addr[i]);
				return -1;
			}

			KMSG_INIT("Sending checkin message to kernel %d\n", i);			
			rc = send_checkin_msg(my_cpu, i);
			if (rc) {
				KMSG_ERR("POPCORN: Checkin failed for CPU %d!\n", i);
				return rc;
			}
		}
	}

	return rc;
}

static int pcn_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char *p= page;
    int len, i, idx;

	p += sprintf(p, "messages get: %ld\n", msg_get);
        p += sprintf(p, "messages put: %ld\n", msg_put);

    idx = log_r_index;
    for (i =0; i>-LOGLEN; i--)
    	p +=sprintf (p,"r%d: from%d type%d %1d:%1d:%1d seq%d\n",
    			(idx+i),(int) log_receive[(idx+i)%LOGLEN].from_cpu, (int)log_receive[(idx+i)%LOGLEN].type,
    			(int) log_receive[(idx+i)%LOGLEN].is_lg_msg, (int)log_receive[(idx+i)%LOGLEN].lg_start,
    			(int) log_receive[(idx+i)%LOGLEN].lg_end, (int) log_receive[(idx+i)%LOGLEN].lg_seqnum );

    idx = log_s_index;
    for (i =0; i>-LOGLEN; i--)
    	p +=sprintf (p,"s%d: from%d type%d %1d:%1d:%1d seq%d\n",
    			(idx+i),(int) log_send[(idx+i)%LOGLEN].from_cpu, (int)log_send[(idx+i)%LOGLEN].type,
    			(int) log_send[(idx+i)%LOGLEN].is_lg_msg, (int)log_send[(idx+i)%LOGLEN].lg_start,
    			(int) log_send[(idx+i)%LOGLEN].lg_end, (int) log_send[(idx+i)%LOGLEN].lg_seqnum );

    idx = log_f_index;
        for (i =0; i>-LOGCALL; i--)
        	p +=sprintf (p,"f%d: %pB\n",
        			(idx+i),(void*) log_function_called[(idx+i)%LOGCALL] );

    idx = log_f_sendindex;
    	for (i =0; i>-LOGCALL; i--)
           	p +=sprintf (p,"[s%d]->: %pB\n",
           			(idx+i),(void*) log_function_send[(idx+i)%LOGCALL] );

        for(i=0; i<PCN_KMSG_RBUF_SIZE; i++)
        	p +=sprintf (p,"second_buffer[%i]=%i\n",i,rkvirt[my_cpu]->second_buffer[i]);


	len = (p -page) - off;
	if (len < 0)
		len = 0;
	*eof = (len <= count) ? 1 : 0;
	*start = page + off;
	return len;
}

static int __init pcn_kmsg_init(void)
{
	int rc,i;
	unsigned long win_phys_addr, rkinfo_phys_addr;
	struct pcn_kmsg_window *win_virt_addr;
	struct boot_params *boot_params_va;

	KMSG_INIT("entered\n");

	my_cpu = raw_smp_processor_id();
	
	printk("%s: THIS VERSION DOES NOT SUPPORT CACHE ALIGNED BUFFERS\n",
	       __func__);
	printk("%s: Entered pcn_kmsg_init raw: %d id: %d\n",
		__func__, my_cpu, smp_processor_id());

	/* Initialize list heads */
	INIT_LIST_HEAD(&msglist_hiprio);
	INIT_LIST_HEAD(&msglist_normprio);

	/* Clear out large-message receive buffers */
	//memset(&lg_buf, 0, POPCORN_MAX_CPUS * sizeof(unsigned char *));
	for(i=0; i<POPCORN_MAX_CPUS; i++) {
		INIT_LIST_HEAD(&(lg_buf[i]));
	}
	long_id=0;


	/* Clear callback table and register default callback functions */
	KMSG_INIT("Registering initial callbacks...\n");
	memset(&callback_table, 0, PCN_KMSG_TYPE_MAX * sizeof(pcn_kmsg_cbftn));
	rc = pcn_kmsg_register_callback(PCN_KMSG_TYPE_CHECKIN, 
					&pcn_kmsg_checkin_callback);
	if (rc) {
		printk(KERN_ALERT"Failed to register initial kmsg checkin callback!\n");
	}

#ifdef PCN_SUPPORT_MULTICAST
	rc = pcn_kmsg_register_callback(PCN_KMSG_TYPE_MCAST, 
					&pcn_kmsg_mcast_callback);
	if (rc) {
		printk(KERN_ALERT"Failed to register initial kmsg mcast callback!\n");
	}
#endif /* PCN_SUPPORT_MULTICAST */ 	

	/* Register softirq handler now kworker */
	KMSG_INIT("Registering softirq handler...\n");
	//open_softirq(PCN_KMSG_SOFTIRQ, pcn_kmsg_action);
	messaging_wq= create_workqueue("messaging_wq");
	if (!messaging_wq) 
		printk("%s: create_workqueue(messaging_wq) ret 0x%lx ERROR\n",
			__func__, (unsigned long)messaging_wq);

	/* Initialize work queue */
	KMSG_INIT("Initializing workqueue...\n");
	kmsg_wq = create_workqueue("kmsg_wq");
	if (!kmsg_wq)
		printk("%s: create_workqueue(kmsg_wq) ret 0x%lx ERROR\n",
			__func__, (unsigned long)kmsg_wq);

		
	/* If we're the master kernel, malloc and map the rkinfo structure and 
	   put its physical address in boot_params; otherwise, get it from the 
	   boot_params and map it */
	if (!mklinux_boot) {
		/* rkinfo must be multiple of a page, because the granularity of
		 * foreings mapping is per page. The following didn't worked,
		 * the returned address is on the form 0xffff88000000, ioremap
		 * on the remote fails. 
		int order = get_order(sizeof(struct pcn_kmsg_rkinfo));
		rkinfo = __get_free_pages(GFP_KERNEL, order);
		*/
		KMSG_INIT("Primary kernel, mallocing rkinfo size:%d rounded:%d\n",
		       sizeof(struct pcn_kmsg_rkinfo), ROUND_PAGE_SIZE(sizeof(struct pcn_kmsg_rkinfo)));
		rkinfo = kmalloc(ROUND_PAGE_SIZE(sizeof(struct pcn_kmsg_rkinfo)), GFP_KERNEL);
		if (!rkinfo) {
			KMSG_ERR("Failed to malloc rkinfo structure!\n");
			return -1;
		}
		memset(rkinfo, 0x0, sizeof(struct pcn_kmsg_rkinfo));
		rkinfo_phys_addr = virt_to_phys(rkinfo);
		KMSG_INIT("rkinfo virt %p, phys 0x%lx MAX_CPUS %d\n", 
			  rkinfo, rkinfo_phys_addr, POPCORN_MAX_CPUS);

		/* Otherwise, we need to set the boot_params to show the rest
		   of the kernels where the master kernel's messaging window 
		   is. */
		KMSG_INIT("Setting boot_params...\n");
		boot_params_va = (struct boot_params *) 
			(0xffffffff80000000 + orig_boot_params);
		boot_params_va->pcn_kmsg_master_window = rkinfo_phys_addr;
		KMSG_INIT("boot_params virt %p phys %p\n",
			boot_params_va, orig_boot_params);
	}
	else {
		KMSG_INIT("Primary kernel rkinfo phys addr: 0x%lx\n", 
			  (unsigned long) boot_params.pcn_kmsg_master_window);
		rkinfo_phys_addr = boot_params.pcn_kmsg_master_window;
		
		rkinfo = ioremap_cache(rkinfo_phys_addr, ROUND_PAGE_SIZE(sizeof(struct pcn_kmsg_rkinfo)));
		if (!rkinfo) {
			KMSG_ERR("Failed to map rkinfo from master kernel!\n");
		}
		KMSG_INIT("rkinfo virt addr: 0x%p\n", rkinfo);
	}

	/* Malloc our own receive buffer and set it up */
	win_virt_addr = kmalloc(ROUND_PAGE_SIZE(sizeof(struct pcn_kmsg_window)), GFP_KERNEL);
	if (win_virt_addr) {
		KMSG_INIT("Allocated %ld(%ld) bytes for my win, virt addr 0x%p\n", 
			  ROUND_PAGE_SIZE(sizeof(struct pcn_kmsg_window)),
			  sizeof(struct pcn_kmsg_window), win_virt_addr);
	} else {
		KMSG_ERR("%s: Failed to kmalloc kmsg recv window!\n", __func__);
		return -1;
	}

	rkvirt[my_cpu] = win_virt_addr;
	win_phys_addr = virt_to_phys((void *) win_virt_addr);
	KMSG_INIT("cpu %d physical address: 0x%lx\n", my_cpu, win_phys_addr);
	rkinfo->phys_addr[my_cpu] = win_phys_addr;

	rc = pcn_kmsg_window_init(rkvirt[my_cpu]);
	if (rc) {
		KMSG_ERR("Failed to initialize kmsg recv window!\n");
		return -1;
	}

	/* If we're not the master kernel, we need to check in */
	if (mklinux_boot) {
		rc = do_checkin();

		if (rc) { 
			KMSG_ERR("Failed to check in!\n");
			return -1;
		}
	} 

	memset(log_receive,0,sizeof(struct pcn_kmsg_hdr)*LOGLEN);
	memset(log_send,0,sizeof(struct pcn_kmsg_hdr)*LOGLEN);
	memset(log_function_called,0,sizeof(void*)*LOGCALL);
	memset(log_function_send,0,sizeof(void*)*LOGCALL);
	/* if everything is ok create a proc interface */
	struct proc_dir_entry *res;
	res = create_proc_entry("pcnmsg", S_IRUGO, NULL);
	if (!res) {
		printk(KERN_ALERT"%s: create_proc_entry failed (%p)\n", __func__, res);
		return -ENOMEM;
	}
	res->read_proc = pcn_read_proc;

	return 0;
}

subsys_initcall(pcn_kmsg_init);

/* Register a callback function when a kernel module is loaded */
int pcn_kmsg_register_callback(enum pcn_kmsg_type type, pcn_kmsg_cbftn callback)
{
	PCN_WARN("%s: registering callback for type %d, ptr 0x%p\n", __func__, type, callback);

	if (type >= PCN_KMSG_TYPE_MAX) {
		printk(KERN_ALERT"Attempted to register callback with bad type %d\n", 
			 type);
		return -1;
	}

	callback_table[type] = callback;

	return 0;
}

/* Unregister a callback function when a kernel module is unloaded */
int pcn_kmsg_unregister_callback(enum pcn_kmsg_type type)
{
	if (type >= PCN_KMSG_TYPE_MAX) {
		KMSG_ERR("Attempted to register callback with bad type %d\n", 
			 type);
		return -1;
	}

	callback_table[type] = NULL;

	return 0;
}

/* SENDING / MARSHALING */

unsigned long int_ts;

static int __pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg,
			   int no_block)
{
	int rc;
	struct pcn_kmsg_window *dest_window;

	if (unlikely(dest_cpu >= POPCORN_MAX_CPUS)) {
		KMSG_ERR("Invalid destination CPU %d\n", dest_cpu);
		return -1;
	}

	dest_window = rkvirt[dest_cpu];

	if (unlikely(!rkvirt[dest_cpu])) {
		//KMSG_ERR("Dest win for CPU %d not mapped!\n", dest_cpu);
		return -1;
	}

	if (unlikely(!msg)) {
		KMSG_ERR("Passed in a null pointer to msg!\n");
		return -1;
	}

	/* set source CPU */
	msg->hdr.from_cpu = my_cpu;

	rc = win_put(dest_window, msg, no_block);

	if (rc) {
		if (no_block && (rc == EAGAIN)) {
			return rc;
		}
		KMSG_ERR("Failed to place message in dest win!\n");
		return -1;
	}


	/* send IPI */
	if (win_int_enabled(dest_window)) {
		KMSG_PRINTK("Interrupts enabled; sending IPI...\n");
		rdtscll(int_ts);
		apic->send_IPI_single(dest_cpu, POPCORN_KMSG_VECTOR);
	} else {
		KMSG_PRINTK("Interrupts not enabled; not sending IPI...\n");
	}


	return 0;
}

int pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg)
{
	unsigned long bp;
	get_bp(bp);
	log_function_send[log_f_sendindex%LOGCALL]= callback_table[msg->hdr.type];
	log_f_sendindex++;
	msg->hdr.is_lg_msg = 0;
	msg->hdr.lg_start = 0;
	msg->hdr.lg_end = 0;
	msg->hdr.lg_seqnum = 0;
	msg->hdr.long_number= 0;

	return __pcn_kmsg_send(dest_cpu, msg, 0);
}

int pcn_kmsg_send_noblock(unsigned int dest_cpu, struct pcn_kmsg_message *msg)
{

	msg->hdr.is_lg_msg = 0;
	msg->hdr.lg_start = 0;
	msg->hdr.lg_end = 0;
	msg->hdr.lg_seqnum = 0;
	msg->hdr.long_number= 0;

	return __pcn_kmsg_send(dest_cpu, msg, 1);
}

int pcn_kmsg_send_long(unsigned int dest_cpu, 
		       struct pcn_kmsg_long_message *lmsg, 
		       unsigned int payload_size)
{
	int i, ret =0;
	int num_chunks = payload_size / PCN_KMSG_PAYLOAD_SIZE;
	struct pcn_kmsg_message this_chunk;

	if (payload_size % PCN_KMSG_PAYLOAD_SIZE) {
		num_chunks++;
	}

	 if ( num_chunks >= MAX_CHUNKS ){
		 KMSG_PRINTK("Message too long (size:%d, chunks:%d, max:%d) can not be transferred\n",
	                payload_size, num_chunks, MAX_CHUNKS);
	        return -1;
	 }

	KMSG_PRINTK("Sending large message to CPU %d, type %d, payload size %d bytes, %d chunks\n", 
		    dest_cpu, lmsg->hdr.type, payload_size, num_chunks);

	this_chunk.hdr.type = lmsg->hdr.type;
	this_chunk.hdr.prio = lmsg->hdr.prio;
	this_chunk.hdr.is_lg_msg = 1;
	this_chunk.hdr.long_number= fetch_and_add(&long_id,1);

	for (i = 0; i < num_chunks; i++) {
		KMSG_PRINTK("Sending chunk %d\n", i);

		this_chunk.hdr.lg_start = (i == 0) ? 1 : 0;
		this_chunk.hdr.lg_end = (i == num_chunks - 1) ? 1 : 0;
		this_chunk.hdr.lg_seqnum = (i == 0) ? num_chunks : i;

		memcpy(&this_chunk.payload, 
		       ((unsigned char *) &lmsg->payload) + 
		       i * PCN_KMSG_PAYLOAD_SIZE, 
		       PCN_KMSG_PAYLOAD_SIZE);

		ret=__pcn_kmsg_send(dest_cpu, &this_chunk, 0);
		if(ret!=0)
			return ret;
	}

	return 0;
}

/* RECEIVING / UNMARSHALING */

static int process_message_list(struct list_head *head) 
{
	int rc, rc_overall = 0;
	struct pcn_kmsg_container *pos = NULL, *n = NULL;
	struct pcn_kmsg_message *msg;

	list_for_each_entry_safe(pos, n, head, list) {
		msg = &pos->msg;

		KMSG_PRINTK("Item in list, type %d,  processing it...\n", 
			    msg->hdr.type);

		list_del(&pos->list);

		if (unlikely(msg->hdr.type >= PCN_KMSG_TYPE_MAX || 
			     !callback_table[msg->hdr.type])) {
			KMSG_ERR("Invalid type %d; continuing!\n", 
				 msg->hdr.type);
			continue;
		}

		rc = callback_table[msg->hdr.type](msg);
		if (!rc_overall) {
			rc_overall = rc;
		}
		//log_function_called[log_f_index%LOGLEN]= callback_table[msg->hdr.type];
		//memcpy(&(log_function_called[log_f_index%LOGCALL]),&(callback_table[msg->hdr.type]),sizeof(void*));
		log_function_called[log_f_index%LOGCALL]= callback_table[msg->hdr.type];
		log_f_index++;
		/* NOTE: callback function is responsible for freeing memory
		   that was kmalloced! */
	}

	return rc_overall;
}

//void pcn_kmsg_do_tasklet(unsigned long);
//DECLARE_TASKLET(pcn_kmsg_tasklet, pcn_kmsg_do_tasklet, 0);

unsigned volatile long isr_ts = 0, isr_ts_2 = 0;

/* top half */
void smp_popcorn_kmsg_interrupt(struct pt_regs *regs)
{
	//if (!isr_ts) {
		rdtscll(isr_ts);
	//}

	ack_APIC_irq();

	KMSG_PRINTK("Reached Popcorn KMSG interrupt handler!\n");

	inc_irq_stat(irq_popcorn_kmsg_count);
	irq_enter();

	/* We do as little work as possible in here (decoupling notification 
	   from messaging) */

	/* disable further interrupts for now */
	win_disable_int(rkvirt[my_cpu]);

	//if (!isr_ts_2) {
	rdtscll(isr_ts_2);
	//}

	/* schedule bottom half */
	//__raise_softirq_irqoff(PCN_KMSG_SOFTIRQ);
	struct work_struct* kmsg_work = kmalloc(sizeof(struct work_struct), GFP_ATOMIC);
	if (kmsg_work) {
		INIT_WORK(kmsg_work,pcn_kmsg_action);
		queue_work(messaging_wq, kmsg_work);
	} else {
		KMSG_ERR("Failed to kmalloc work structure!\n");
	}
	//tasklet_schedule(&pcn_kmsg_tasklet);

	irq_exit();
	return;
}

static int msg_add_list(struct pcn_kmsg_container *ctr)
{
	int rc = 0;

	switch (ctr->msg.hdr.prio) {
		case PCN_KMSG_PRIO_HIGH:
			KMSG_PRINTK("%s: Adding to high-priority list...\n", __func__);
			list_add_tail(&(ctr->list),
				      &msglist_hiprio);
			break;

		case PCN_KMSG_PRIO_NORMAL:
			KMSG_PRINTK("%s: Adding to normal-priority list...\n", __func__);
			list_add_tail(&(ctr->list),
				      &msglist_normprio);
			break;

		default:
			KMSG_ERR("%s: Priority value %d unknown -- THIS IS BAD!\n", __func__,
				  ctr->msg.hdr.prio);
			rc = -1;
	}

	return rc;
}

static int process_large_message(struct pcn_kmsg_reverse_message *msg)
{
	int rc = 0;
	int recv_buf_size;
	struct pcn_kmsg_long_message *lmsg;
	int work_done = 0;
	struct pcn_kmsg_container* container_long=NULL, *n=NULL;

	KMSG_PRINTK("Got a large message fragment, type %u, from_cpu %u, start %u, end %u, seqnum %u!\n",
		    msg->hdr.type, msg->hdr.from_cpu,
		    msg->hdr.lg_start, msg->hdr.lg_end,
		    msg->hdr.lg_seqnum);

	if (msg->hdr.lg_start) {
		KMSG_PRINTK("Processing initial message fragment...\n");

		if (!msg->hdr.lg_seqnum)
		  printk(KERN_ALERT"%s: ERROR lg_seqnum is zero:%d long_number:%ld\n",
		      __func__, (int)msg->hdr.lg_seqnum, (long)msg->hdr.long_number);
		  
		// calculate the size of the holding buffer
		recv_buf_size = sizeof(struct list_head) + 
			sizeof(struct pcn_kmsg_hdr) + 
			msg->hdr.lg_seqnum * PCN_KMSG_PAYLOAD_SIZE;
#undef BEN_VERSION
#ifdef BEN_VERSION		
		lg_buf[msg->hdr.from_cpu] = kmalloc(recv_buf_size, GFP_ATOMIC);
		if (!lg_buf[msg->hdr.from_cpu]) {
					KMSG_ERR("Unable to kmalloc buffer for incoming message!\n");
					goto out;
				}
		lmsg = (struct pcn_kmsg_long_message *) &lg_buf[msg->hdr.from_cpu]->msg;
#else /* BEN_VERSION */
		container_long= kmalloc(recv_buf_size, GFP_ATOMIC);
		if (!container_long) {
			KMSG_ERR("Unable to kmalloc buffer for incoming message!\n");
			goto out;
		}
		lmsg = (struct pcn_kmsg_long_message *) &container_long->msg; //TODO wrong cast!
#endif /* !BEN_VERSION */

		/* copy header first */
		memcpy((unsigned char *) &lmsg->hdr, 
		       &msg->hdr, sizeof(struct pcn_kmsg_hdr));
		/* copy first chunk of message */
		memcpy((unsigned char *) &lmsg->payload,
		       &msg->payload, PCN_KMSG_PAYLOAD_SIZE);

		if (msg->hdr.lg_end) {
			KMSG_PRINTK("NOTE: Long message of length 1 received; this isn't efficient!\n");

			/* add to appropriate list */
#ifdef BEN_VERSION			
			rc = msg_add_list(lg_buf[msg->hdr.from_cpu]);
#else /* BEN_VERSION */
			rc = msg_add_list(container_long);
#endif /* !BEN_VERSION */
			if (rc)
				KMSG_ERR("Failed to add large message to list!\n");
			work_done = 1;
		}
#ifndef BEN_VERSION		
		else
		  // add the message in the lg_buf
		  list_add_tail(&container_long->list, &lg_buf[msg->hdr.from_cpu]);
#endif /* !BEN_VERSION */
	}
	else {
		KMSG_PRINTK("Processing subsequent message fragment...\n");

		//It should not be needed safe
		list_for_each_entry_safe(container_long, n, &lg_buf[msg->hdr.from_cpu], list) {
			if ( (container_long != NULL) &&
			  (container_long->msg.hdr.long_number == msg->hdr.long_number) )
				// found!
				goto next;
		}

		KMSG_ERR("Failed to find long message %lu in the list of cpu %i!\n",
			 msg->hdr.long_number, msg->hdr.from_cpu);
		goto out;

next:		
		lmsg = (struct pcn_kmsg_long_message *) &container_long->msg;
		memcpy((unsigned char *) ((void*)&lmsg->payload) + (PCN_KMSG_PAYLOAD_SIZE * msg->hdr.lg_seqnum),
		       &msg->payload, PCN_KMSG_PAYLOAD_SIZE);

		if (msg->hdr.lg_end) {
			KMSG_PRINTK("Last fragment in series...\n");
			KMSG_PRINTK("from_cpu %d, type %d, prio %d\n",
				    lmsg->hdr.from_cpu, lmsg->hdr.type, lmsg->hdr.prio);
			/* add to appropriate list */
#ifdef BEN_VERSION
			rc = msg_add_list(lg_buf[msg->hdr.from_cpu]);
#else /* BEN_VERSION */
			list_del(&container_long->list);
			rc = msg_add_list(container_long);
#endif /* !BEN_VERSION */			
			if (rc)
				KMSG_ERR("Failed to add large message to list!\n");
			work_done = 1;
		}
	}

out:
	return work_done;
}

static int process_small_message(struct pcn_kmsg_reverse_message *msg)
{
	int rc = 0, work_done = 1;
	struct pcn_kmsg_container *incoming;

	/* malloc some memory (don't sleep!) */
	incoming = kmalloc(sizeof(struct pcn_kmsg_container), GFP_ATOMIC);
	if (unlikely(!incoming)) {
		KMSG_ERR("Unable to kmalloc buffer for incoming message!\n");
		return 0;
	}

	/* memcpy message from rbuf */
	memcpy(&incoming->msg.hdr, &msg->hdr,
	       sizeof(struct pcn_kmsg_hdr));

	memcpy(&incoming->msg.payload, &msg->payload,
	       PCN_KMSG_PAYLOAD_SIZE);

	KMSG_PRINTK("Received message, type %d, prio %d\n",
		    incoming->msg.hdr.type, incoming->msg.hdr.prio);

	/* add container to appropriate list */
	rc = msg_add_list(incoming);

	return work_done;
}

static int pcn_kmsg_poll_handler(void)
{
	struct pcn_kmsg_reverse_message *msg;
	struct pcn_kmsg_window *win = rkvirt[my_cpu]; // TODO this will not work for clustering
	int work_done = 0;

	KMSG_PRINTK("called\n");

pull_msg:
	/* Get messages out of the buffer first */
//#define PCN_KMSG_BUDGET 128
	//while ((work_done < PCN_KMSG_BUDGET) && (!win_get(rkvirt[my_cpu], &msg))) {
	while (! win_get(win, &msg) ) {
		KMSG_PRINTK("got a message!\n");

		/* Special processing for large messages */
		if (msg->hdr.is_lg_msg) {
			KMSG_PRINTK("message is a large message!\n");
			work_done += process_large_message(msg);
		} else {
			KMSG_PRINTK("message is a small message!\n");
			work_done += process_small_message(msg);
		}
		pcn_barrier();
		msg->ready = 0;
		//win_advance_tail(win);
		fetch_and_add(&win->tail, 1);
	}

	win_enable_int(win);
	if ( win_inuse(win) ) {
		win_disable_int(win);
		goto pull_msg;
	}

	return work_done;
}

unsigned volatile long bh_ts = 0, bh_ts_2 = 0;

// NOTE the following was declared as a bottom half
//static void pcn_kmsg_action(struct softirq_action *h)
static void pcn_kmsg_action(struct work_struct* work)
{
	int rc;
	int i;
	int work_done = 0;

	//if (!bh_ts) {
		rdtscll(bh_ts);
	//}
	KMSG_PRINTK("called\n");

	work_done = pcn_kmsg_poll_handler();
	KMSG_PRINTK("Handler did %d units of work!\n", work_done);

#ifdef PCN_SUPPORT_MULTICAST	
	for (i = 0; i < POPCORN_MAX_MCAST_CHANNELS; i++) {
		if (MCASTWIN(i)) {
			KMSG_PRINTK("mcast win %d mapped, processing it\n", i);
			process_mcast_queue(i);
		}
	}
	KMSG_PRINTK("Done checking mcast queues; processing messages\n");
#endif /* PCN_SUPPORT_MULTICAST */

	//if (!bh_ts_2) {
		rdtscll(bh_ts_2);
	//}

	/* Process high-priority queue first */
	rc = process_message_list(&msglist_hiprio);

	if (list_empty(&msglist_hiprio)) {
		KMSG_PRINTK("High-priority queue is empty!\n");
	}

	/* Then process normal-priority queue */
	rc = process_message_list(&msglist_normprio);

	kfree(work);

	return;
}

#ifdef PCN_SUPPORT_MULTICAST
# include "pcn_kmsg_mcast.h"
#endif /* PCN_SUPPORT_MULTICAST */
