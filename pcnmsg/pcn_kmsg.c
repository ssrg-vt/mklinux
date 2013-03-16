/*
 * Inter-kernel messaging support for Popcorn
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2013
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

#include <asm/system.h>
#include <asm/apic.h>
#include <asm/hardirq.h>
#include <asm/setup.h>
#include <asm/bootparam.h>
#include <asm/errno.h>
#include <asm/atomic.h>

/* COMMON STATE */

/* table of callback functions for handling each message type */
pcn_kmsg_cbftn callback_table[PCN_KMSG_TYPE_MAX];

/* number of current kernel */
int my_cpu = 0;

/* pointer to table with phys addresses for remote kernels' windows,
 * owned by kernel 0 */
struct pcn_kmsg_rkinfo *rkinfo;

/* table with virtual (mapped) addresses for remote kernels' windows,
   one per kernel */
struct pcn_kmsg_window * rkvirt[POPCORN_MAX_CPUS];

/* Same thing, but for mcast windows */
struct pcn_kmsg_mcast_local mcastlocal[POPCORN_MAX_MCAST_CHANNELS];

/* lists of messages to be processed for each prio */
struct list_head msglist_hiprio, msglist_normprio;

/* array to hold pointers to large messages received */
struct pcn_kmsg_long_message * lg_buf[POPCORN_MAX_CPUS];

/* action for bottom half */
static void pcn_kmsg_action(struct softirq_action *h);

/* workqueue for operations that can sleep */
static struct workqueue_struct *kmsg_wq;

/* RING BUFFER */

#define RB_SHIFT 6
#define RB_SIZE (1 << RB_SHIFT)
#define RB_MASK ((1 << RB_SHIFT) - 1)

#define PCN_DEBUG(...) ;
//#define PCN_WARN(...) printk(__VA_ARGS__)
#define PCN_WARN(...) ;
#define PCN_ERROR(...) printk(__VA_ARGS__)

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

static inline int win_put(struct pcn_kmsg_window *win, 
			  struct pcn_kmsg_message *msg) 
{
	unsigned long ticket;

	/* if the queue is already really long, return EAGAIN */
	if (win_inuse(win) >= RB_SIZE) {
		printk("Window full, caller should try again...\n");
		return -EAGAIN;
	}

	/* grab ticket */
	ticket = fetch_and_add(&win->head, 1);
	PCN_DEBUG(KERN_ERR "%s: ticket = %lu, head = %lu, tail = %lu\n", 
		 __func__, ticket, win->head, win->tail);

	/* spin until there's a spot free for me */
	while (win_inuse(win) >= RB_SIZE) {}

	/* insert item */
	memcpy(&win->buffer[ticket & RB_MASK], msg, 
	       sizeof(struct pcn_kmsg_message));

	pcn_barrier();

	/* set completed flag */
	win->buffer[ticket & RB_MASK].hdr.ready = 1;

	return 0;
}

static inline int win_get(struct pcn_kmsg_window *win, 
			  struct pcn_kmsg_message **msg) 
{
	struct pcn_kmsg_message *rcvd;

	if (!win_inuse(win)) {
		PCN_WARN(KERN_ERR "%s: Nothing in buffer, returning...\n", __func__);
		return -1;
	}

	PCN_DEBUG(KERN_ERR "%s: reached win_get, head %lu, tail %lu\n", 
	       __func__, win->head, win->tail);

	/* spin until entry.ready at end of cache line is set */
	rcvd = &(win->buffer[win->tail & RB_MASK]);
	//printk(KERN_ERR "Ready bit: %u\n", rcvd->hdr.ready);
	while (!rcvd->hdr.ready) {
		pcn_cpu_relax();
	}

	// barrier here?
	pcn_barrier();

	rcvd->hdr.ready = 0;

	*msg = rcvd;	

	return 0;
}

static inline void win_advance_tail(struct pcn_kmsg_window *win) 
{
	win->tail++;
}

#define MCASTWIN(_id_) (mcastlocal[(_id_)].mcastvirt)
#define LOCAL_TAIL(_id_) (mcastlocal[(_id_)].local_tail)

/* MULTICAST RING BUFFER */
static inline unsigned long mcastwin_inuse(pcn_kmsg_mcast_id id)
{
	        return MCASTWIN(id)->head - MCASTWIN(id)->tail;
}

static inline int mcastwin_put(pcn_kmsg_mcast_id id,
			  struct pcn_kmsg_message *msg)
{
	unsigned long ticket;

	/* if the queue is already really long, return EAGAIN */
	if (mcastwin_inuse(id) >= RB_SIZE) {
		PCN_WARN(KERN_ERR"%s: Window full, caller should try again...\n", __func__);
		return -EAGAIN;
	}

	/* grab ticket */
	ticket = fetch_and_add(&MCASTWIN(id)->head, 1);
	PCN_DEBUG(KERN_ERR "%s: ticket = %lu, head = %lu, tail = %lu\n", __func__,
	       ticket, MCASTWIN(id)->head, MCASTWIN(id)->tail);

	/* spin until there's a spot free for me */
	while (mcastwin_inuse(id) >= RB_SIZE) {}

	/* insert item */
	memcpy(&MCASTWIN(id)->buffer[ticket & RB_MASK], msg,
	       sizeof(struct pcn_kmsg_message));

	/* set counter to (# in group - self) */
	MCASTWIN(id)->read_counter[ticket & RB_MASK] = 
		rkinfo->mcast_wininfo[id].num_members - 1;

	pcn_barrier();

	/* set completed flag */
	MCASTWIN(id)->buffer[ticket & RB_MASK].hdr.ready = 1;

	return 0;
}

static inline int mcastwin_get(pcn_kmsg_mcast_id id,
			  struct pcn_kmsg_message **msg)
{
	struct pcn_kmsg_message *rcvd;

	if (!mcastwin_inuse(id)) {
		PCN_WARN(KERN_ERR"%s: Nothing in buffer, returning...\n", __func__);
		return -1;
	}

	PCN_DEBUG(KERN_ERR "%s: reached mcastwin_get, head %lu, tail %lu\n", __func__,
	       MCASTWIN(id)->head, MCASTWIN(id)->tail);

	/* spin until entry.ready at end of cache line is set */
	rcvd = &(MCASTWIN(id)->buffer[MCASTWIN(id)->tail & RB_MASK]);
	//printk(KERN_ERR "Ready bit: %u\n", rcvd->hdr.ready);
	while (!rcvd->hdr.ready) {
		pcn_cpu_relax();
	}

	// barrier here?
	pcn_barrier();

	rcvd->hdr.ready = 0;

	*msg = rcvd;

	return 0;
}

static inline void mcastwin_advance_tail(pcn_kmsg_mcast_id id)
{
	unsigned long slot = LOCAL_TAIL(id) & RB_MASK;

	PCN_DEBUG(KERN_ERR "%s: Advancing tail; local tail currently on slot %lu\n", __func__, LOCAL_TAIL(id));

	if (atomic_dec_and_test((atomic_t *) &MCASTWIN(id)->read_counter[slot])) {
		PCN_DEBUG(KERN_ERR"%s: We're the last reader to go; advancing global tail\n", __func__);
		atomic64_inc((atomic64_t *) &MCASTWIN(id)->tail);
	}

	LOCAL_TAIL(id)++;
}

/* INITIALIZATION */

static int pcn_kmsg_mcast_callback(struct pcn_kmsg_message *message);

/* bottom half for workqueue */
static void process_kmsg_wq_item(struct work_struct * work)
{
	int cpu;
	pcn_kmsg_mcast_id id;
	pcn_kmsg_work_t *w = (pcn_kmsg_work_t *) work;

	PCN_DEBUG(KERN_ERR "%s: Processing kmsg wq item, op %d\n", __func__, w->op);

	switch (w->op) {
		case PCN_KMSG_WQ_OP_MAP_MSG_WIN:
			cpu = w->cpu_to_add;

			if (cpu < 0 || cpu >= POPCORN_MAX_CPUS) {
				PCN_ERROR(KERN_ERR"%s: Invalid CPU %d specified!\n", __func__, cpu);
				return;
			}

			rkvirt[cpu] = ioremap_cache(rkinfo->phys_addr[cpu],
						    ((sizeof(struct pcn_kmsg_window) >> PAGE_SHIFT) +1)<< PAGE_SHIFT);
PCN_DEBUG("%s: ioremap %lx [%d] %ld\n", 
	  __func__, rkinfo->phys_addr[cpu], cpu, ((sizeof(struct pcn_kmsg_window) >> PAGE_SHIFT) +1) << PAGE_SHIFT);

			if (rkvirt[cpu]) {
				PCN_DEBUG("%s: ioremapped window, virt addr 0x%p\n", __func__,
				       rkvirt[cpu]);
			} else {
				PCN_DEBUG("%s: Failed to ioremap CPU %d's window at phys addr 0x%lx\n",
				       __func__, cpu, rkinfo->phys_addr[cpu]);
			}
			break;

		case PCN_KMSG_WQ_OP_UNMAP_MSG_WIN:
			PCN_DEBUG("%s: UNMAP_MSG_WIN not yet implemented!\n", __func__);
			break;

		case PCN_KMSG_WQ_OP_MAP_MCAST_WIN:
			id = w->id_to_join;

			/* map window */
			if (id < 0 || id > POPCORN_MAX_MCAST_CHANNELS) {
				PCN_ERROR("%s: Invalid mcast channel id %lu specified!\n", __func__, id);
				return;
			}

			MCASTWIN(id) = ioremap_cache(rkinfo->mcast_wininfo[id].phys_addr,
						    sizeof(struct pcn_kmsg_mcast_window));
			if (MCASTWIN(id)) {
				PCN_WARN("%s: ioremapped mcast window, virt addr 0x%p\n", __func__,
				       MCASTWIN(id));
			} else {
				PCN_ERROR("%s: Failed to ioremap mcast window %lu at phys addr 0x%lx\n", __func__,
				       id, rkinfo->mcast_wininfo[id].phys_addr);
			}


			break;

		case PCN_KMSG_WQ_OP_UNMAP_MCAST_WIN:
			PCN_ERROR("%s: UNMAP_MCAST_WIN not yet implemented!\n", __func__);
			break;

		default:
			PCN_ERROR("%s: Invalid work queue operation %d\n", __func__, w->op);

	}

	kfree(work);
}

static int pcn_kmsg_checkin_callback(struct pcn_kmsg_message *message) 
{
	struct pcn_kmsg_checkin_message *msg = 
		(struct pcn_kmsg_checkin_message *) message;
	int from_cpu = msg->hdr.from_cpu;
	pcn_kmsg_work_t *kmsg_work = NULL;

	PCN_DEBUG("%s: Called Popcorn callback for processing check-in messages\n", __func__);

	PCN_DEBUG("%s: From CPU %d, type %d, window phys addr 0x%lx\n", __func__,
	       msg->hdr.from_cpu, msg->hdr.type, 
	       msg->window_phys_addr);

	if (from_cpu >= POPCORN_MAX_CPUS) {
		PCN_ERROR("%s: Invalid source CPU %d\n", __func__, msg->hdr.from_cpu);
		return -1;
	}

	if (!msg->window_phys_addr) {
		PCN_ERROR("%s: Window physical address from CPU %d is NULL!\n", __func__,
		       from_cpu);
		return -1;
	}

	/* Note that we're not allowed to ioremap anything from a bottom half,
	   so we'll add it to a workqueue and do it in a kernel thread. */
	kmsg_work = kmalloc(sizeof(pcn_kmsg_work_t), GFP_ATOMIC);
	if (kmsg_work) {
		INIT_WORK((struct work_struct *) kmsg_work, 
			  process_kmsg_wq_item);
		kmsg_work->op = PCN_KMSG_WQ_OP_MAP_MSG_WIN;
		kmsg_work->from_cpu = msg->hdr.from_cpu;
		kmsg_work->cpu_to_add = msg->cpu_to_add;
		queue_work(kmsg_wq, (struct work_struct *) kmsg_work);
	} else {
		PCN_ERROR("%s: Failed to malloc work structure; this is VERY BAD!\n", __func__);
	}

	kfree(message);

	return 0;
}

static int pcn_kmsg_test_callback(struct pcn_kmsg_message *message)
{
	struct pcn_kmsg_long_message *lmsg = 
		(struct pcn_kmsg_long_message *) message;

	PCN_DEBUG("%s: Received test long message, payload: %s\n", __func__,
	       (char *) &lmsg->payload);

	return 0;
}

static inline int pcn_kmsg_window_init(struct pcn_kmsg_window *window)
{
	window->head = 0;
	window->tail = 0;
	memset(&window->buffer, 0, 
	       PCN_KMSG_RBUF_SIZE * sizeof(struct pcn_kmsg_message));
	return 0;
}

static inline int pcn_kmsg_mcast_window_init(struct pcn_kmsg_mcast_window *window)
{
	window->head = 0;
	window->tail = 0;
	memset(&window->read_counter, 0, 
	       PCN_KMSG_RBUF_SIZE * sizeof(int));
	memset(&window->buffer, 0,
	       PCN_KMSG_RBUF_SIZE * sizeof(struct pcn_kmsg_message));
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
		PCN_WARN("%s: Failed to send checkin message, rc = %d\n", __func__, rc);
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
						  ((sizeof(struct pcn_kmsg_window) >> PAGE_SHIFT ) +1) << PAGE_SHIFT);
			if (rkvirt[i]) {
				PCN_WARN("%s: ioremapped CPU %d's window, virt addr 0x%p size %ld(%ld)\n", __func__,
				       i, rkvirt[i], ((sizeof(struct pcn_kmsg_window) >> PAGE_SHIFT) +1) << PAGE_SHIFT , sizeof(struct pcn_kmsg_window));
			} else {
				PCN_ERROR("%s: Failed to ioremap CPU %d's window at phys addr 0x%lx\n", __func__,
				       i, rkinfo->phys_addr[i]);
				return -1;
			}

			PCN_DEBUG("%s: Sending checkin message to kernel %d\n", __func__, i);			
			rc = send_checkin_msg(my_cpu, i);
			if (rc) {
				PCN_ERROR("%s: POPCORN: Checkin failed for CPU %d!\n", __func__, i);
				return rc;
			}
		}
	}

	return rc;
}

static int __init pcn_kmsg_init(void)
{
	int rc;
	unsigned long win_virt_addr, win_phys_addr, rkinfo_phys_addr;
	struct boot_params * boot_params_va;

	my_cpu = raw_smp_processor_id();
	
	printk("%s: Entered pcn_kmsg_init raw: %d id: %d\n", __func__, my_cpu, smp_processor_id());

	/* Initialize list heads */
	INIT_LIST_HEAD(&msglist_hiprio);
	INIT_LIST_HEAD(&msglist_normprio);

	/* Clear out large-message receive buffers */
	memset(&lg_buf, 0, POPCORN_MAX_CPUS * sizeof(unsigned char *));

	/* Clear callback table and register default callback functions */
	PCN_DEBUG("%s: Registering initial callbacks...\n", __func__);
	memset(&callback_table, 0, PCN_KMSG_TYPE_MAX * sizeof(pcn_kmsg_cbftn));
	rc = pcn_kmsg_register_callback(PCN_KMSG_TYPE_CHECKIN, 
					&pcn_kmsg_checkin_callback);
	if (rc) {
		printk("POPCORN: Failed to register initial kmsg checkin callback!\n");
	}

	rc = pcn_kmsg_register_callback(PCN_KMSG_TYPE_MCAST, 
					&pcn_kmsg_mcast_callback);
	if (rc) {
		printk("POPCORN: Failed to register initial kmsg multicast callback!\n");
	}

	rc = pcn_kmsg_register_callback(PCN_KMSG_TYPE_TEST, 
					&pcn_kmsg_test_callback);
	if (rc) {
		printk("POPCORN: Failed to register initial kmsg test callback!\n");
	}

	/* Register softirq handler */
	PCN_DEBUG("%s: Registering softirq handler...\n", __func__);
	open_softirq(PCN_KMSG_SOFTIRQ, pcn_kmsg_action);

	/* Initialize work queue */
	PCN_DEBUG("%s: Initializing workqueue...\n", __func__);
	kmsg_wq = create_singlethread_workqueue("kmsg_wq");

	/* If we're the master kernel, malloc and map the rkinfo structure and 
	   put its physical address in boot_params; otherwise, get it from the 
	   boot_params and map it */
	if (!mklinux_boot) {
		PCN_DEBUG("%s: We're the master; mallocing rkinfo...\n", __func__);
		rkinfo = kmalloc(sizeof(struct pcn_kmsg_rkinfo), GFP_KERNEL);

		if (!rkinfo) {
			PCN_ERROR("%s: Failed to malloc rkinfo structure -- this is very bad!\n", __func__);
			return -1;
		}

		rkinfo_phys_addr = virt_to_phys(rkinfo);

		PCN_DEBUG("%s: rkinfo virt addr 0x%p, phys addr 0x%lx\n", __func__,
		       rkinfo, rkinfo_phys_addr);

		memset(rkinfo, 0x0, sizeof(struct pcn_kmsg_rkinfo));

		PCN_DEBUG("%s: Setting boot_params...\n", __func__);
		/* Otherwise, we need to set the boot_params to show the rest
		   of the kernels where the master kernel's messaging window is. */
		boot_params_va = (struct boot_params *) 
			(0xffffffff80000000 + orig_boot_params);
		PCN_DEBUG("%s: Boot params virtual address: 0x%p\n", __func__, boot_params_va);
		boot_params_va->pcn_kmsg_master_window = rkinfo_phys_addr;
	} else {
		PCN_DEBUG("%s: Master kernel rkinfo phys addr: 0x%lx\n", __func__, 
		       (unsigned long) boot_params.pcn_kmsg_master_window);

		rkinfo_phys_addr = boot_params.pcn_kmsg_master_window;
		rkinfo = ioremap_cache(rkinfo_phys_addr, 
				       sizeof(struct pcn_kmsg_rkinfo));

		if (!rkinfo) {
			PCN_ERROR("%s: Failed to ioremap rkinfo struct from master kernel! Continuing..\n", __func__);
		}

		PCN_DEBUG("%s: rkinfo virt addr: 0x%p\n", __func__, rkinfo);
	}

	/* Malloc our own receive buffer and set it up */
	win_virt_addr = kmalloc(sizeof(struct pcn_kmsg_window), GFP_KERNEL);
	PCN_DEBUG("%s: Allocated %ld bytes for my window, virt addr 0x%lx\n", __func__, 
	       sizeof(struct pcn_kmsg_window), win_virt_addr);
	rkvirt[my_cpu] = (struct pcn_kmsg_window *) win_virt_addr;
	win_phys_addr = virt_to_phys((void *) win_virt_addr);
	PCN_DEBUG("%s: Physical address: 0x%lx\n", __func__, win_phys_addr);
	rkinfo->phys_addr[my_cpu] = win_phys_addr;

	rc = pcn_kmsg_window_init(rkvirt[my_cpu]);
	if (rc) {
		printk("POPCORN: Failed to initialize kmsg recv window! Continuing..\n");
	}

	/* If we're not the master kernel, we need to check in */
	if (mklinux_boot) {
		rc = do_checkin();

		if (rc) { 
			printk("POPCORN: Failed to check in!\n");
			return -1;
		}
	} 

	return 0;
}

subsys_initcall(pcn_kmsg_init);

/* Register a callback function when a kernel module is loaded */
int pcn_kmsg_register_callback(enum pcn_kmsg_type type, pcn_kmsg_cbftn callback)
{
	PCN_WARN("%s: registering callback for type %d, ptr 0x%p\n", __func__, type, callback);

	if (type >= PCN_KMSG_TYPE_MAX) {
		PCN_ERROR("%s: Attempted to register callback with bad type %d\n", __func__, type);
		return -1;
	}

	callback_table[type] = callback;

	return 0;
}

/* Unregister a callback function when a kernel module is unloaded */
int pcn_kmsg_unregister_callback(enum pcn_kmsg_type type)
{
	if (type >= PCN_KMSG_TYPE_MAX) {
		PCN_ERROR("%s: Attempted to register callback with bad type %d\n", __func__, type);
		return -1;
	}

	callback_table[type] = NULL;

	return 0;
}

/* SENDING / MARSHALING */

static int __pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg)
{
	int rc;
	struct pcn_kmsg_window *dest_window;

	if (dest_cpu >= POPCORN_MAX_CPUS) {
		PCN_ERROR("%s: Invalid destination CPU %d\n", __func__, dest_cpu);
		return -1;
	}

	dest_window = rkvirt[dest_cpu];

	if (!rkvirt[dest_cpu]) {
		PCN_ERROR("%s: Destination window for CPU %d not mapped -- this is VERY BAD!\n", __func__, dest_cpu);
		/* check if phys addr exists, and if so, map it */
		return -1;
	}

	if (!msg) {
		PCN_ERROR("%s: Passed in a null pointer to msg!\n", __func__);
		return -1;
	}

	/* set source CPU */
	msg->hdr.from_cpu = my_cpu;

	/* place message in rbuf */
	rc = win_put(dest_window, msg);		

	if (rc) {
		PCN_WARN("%s: Failed to place message in destination window -- maybe it's full?\n", __func__);
		return -1;
	}

	/* send IPI */
	apic->send_IPI_mask(cpumask_of(dest_cpu), POPCORN_KMSG_VECTOR);

	return 0;
}

int pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg)
{
	msg->hdr.is_lg_msg = 0;
	msg->hdr.lg_start = 0;
	msg->hdr.lg_end = 0;
	msg->hdr.lg_seqnum = 0;

	return __pcn_kmsg_send(dest_cpu, msg);
}

int pcn_kmsg_send_long(unsigned int dest_cpu, 
		       struct pcn_kmsg_long_message *lmsg, 
		       unsigned int payload_size)
{
	int i;
	int num_chunks = payload_size / PCN_KMSG_PAYLOAD_SIZE;
	struct pcn_kmsg_message this_chunk;
	//char test_buf[15];

	if (payload_size % PCN_KMSG_PAYLOAD_SIZE) {
		num_chunks++;
	}

	PCN_DEBUG("%s: Sending large message to CPU %d, type %d, payload size %d bytes, %d chunks\n", __func__, dest_cpu, lmsg->hdr.type, payload_size, num_chunks);

	this_chunk.hdr.type = lmsg->hdr.type;
	this_chunk.hdr.prio = lmsg->hdr.prio;
	this_chunk.hdr.is_lg_msg = 1;

	for (i = 0; i < num_chunks; i++) {
		PCN_DEBUG("%s: Sending chunk %d\n", __func__, i);

		this_chunk.hdr.lg_start = (i == 0) ? 1 : 0;
		this_chunk.hdr.lg_end = (i == num_chunks - 1) ? 1 : 0;
		this_chunk.hdr.lg_seqnum = (i == 0) ? num_chunks : i;

		memcpy(&this_chunk.payload, 
		       ((unsigned char *) &lmsg->payload) + 
		       i * PCN_KMSG_PAYLOAD_SIZE, 
		       PCN_KMSG_PAYLOAD_SIZE);

		//memcpy(test_buf, &this_chunk.payload, 10);
		//test_buf[11] = '\0';

		//printk("First 10 characters: %s\n", test_buf);

		__pcn_kmsg_send(dest_cpu, &this_chunk);
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

		PCN_DEBUG("%s: Item in list, type %d,  processing it...\n", __func__,
		       msg->hdr.type);

		list_del(&pos->list);

		if (msg->hdr.type >= PCN_KMSG_TYPE_MAX || 
		    !callback_table[msg->hdr.type]) {
			PCN_WARN("%s: Invalid type %d; continuing!\n", __func__, msg->hdr.type);
			continue;
		}

		rc = callback_table[msg->hdr.type](msg);
		if (!rc_overall) {
			rc_overall = rc;
		}

		/* NOTE: callback function is responsible for freeing memory
		   that was kmalloced! */
	}

	return rc_overall;
}

//void pcn_kmsg_do_tasklet(unsigned long);
//DECLARE_TASKLET(pcn_kmsg_tasklet, pcn_kmsg_do_tasklet, 0);

/* top half */
void smp_popcorn_kmsg_interrupt(struct pt_regs *regs)
{
	ack_APIC_irq();

	PCN_DEBUG("%s: Reached Popcorn KMSG interrupt handler!\n", __func__);

	inc_irq_stat(irq_popcorn_kmsg_count);
	irq_enter();

	/* We do as little work as possible in here (decoupling notification 
	   from messaging) */

	/* schedule bottom half */
	__raise_softirq_irqoff(PCN_KMSG_SOFTIRQ);
	//tasklet_schedule(&pcn_kmsg_tasklet);

	irq_exit();
	return;
}

static int process_large_message(struct pcn_kmsg_message *msg)
{
	int rc = 0;
	int recv_buf_size;

	
	// THERE IS NO CHECK FOR AN EXISTENT MESSAGE IN THE BUFFER!!! :-(
	
	
	PCN_DEBUG( "%s: Got a large message fragment, type %u, from_cpu %u, start %u, end %u, seqnum %u!\n", __func__,
	       msg->hdr.type, msg->hdr.from_cpu,
	       msg->hdr.lg_start, msg->hdr.lg_end,
	       msg->hdr.lg_seqnum);

	if (msg->hdr.lg_start) {
		PCN_DEBUG("%s: Processing initial message fragment...\n", __func__);

		recv_buf_size = sizeof(struct pcn_kmsg_hdr) + 
			msg->hdr.lg_seqnum * PCN_KMSG_PAYLOAD_SIZE;

		lg_buf[msg->hdr.from_cpu] = kmalloc(recv_buf_size, GFP_ATOMIC);

		if (!lg_buf[msg->hdr.from_cpu]) {
			PCN_ERROR("%s: Unable to kmalloc buffer for incoming message!  THIS IS BAD!\n", __func__);
			goto out;
		}

		/* copy header first */
		memcpy((unsigned char *)lg_buf[msg->hdr.from_cpu], 
		       &msg->hdr, sizeof(struct pcn_kmsg_hdr));

		/* copy first chunk of message */
		memcpy((unsigned char *)lg_buf[msg->hdr.from_cpu] + 
		       sizeof(struct pcn_kmsg_hdr),
		       &msg->payload, PCN_KMSG_PAYLOAD_SIZE);

		if (msg->hdr.lg_end) {
			PCN_WARN("%s: NOTE: Long message of length 1 received; this isn't efficient!\n", __func__);
			
			if (unlikely(!callback_table[msg->hdr.type])) {
				PCN_WARN("%s: Callback function for message type %d not registered!\n", __func__,
						msg->hdr.type);
				goto out;
			}
			
			rc = callback_table[msg->hdr.type]((struct pcn_kmsg_message *)lg_buf[msg->hdr.from_cpu]);

			if (rc) {
				PCN_ERROR("%s: Large message callback failed!\n", __func__);
			}
		}
	} else {
		
		PCN_DEBUG("%s: Processing subsequent message fragment...\n", __func__);

		memcpy((unsigned char *)lg_buf[msg->hdr.from_cpu] + 
		       sizeof(struct pcn_kmsg_hdr) + 
		       PCN_KMSG_PAYLOAD_SIZE * msg->hdr.lg_seqnum,
		       &msg->payload, PCN_KMSG_PAYLOAD_SIZE);

		if (msg->hdr.lg_end) {
			PCN_DEBUG("%s: Last fragment in series...\n", __func__);

			PCN_DEBUG("%s: from_cpu %d, type %d, prio %d\n", __func__,
			       lg_buf[msg->hdr.from_cpu]->hdr.from_cpu,
			       lg_buf[msg->hdr.from_cpu]->hdr.type,
			       lg_buf[msg->hdr.from_cpu]->hdr.prio);

			if (unlikely(!callback_table[msg->hdr.type])) {
				PCN_WARN("%s: Callback function for message type %d not registered!\n",
						__func__, msg->hdr.type);
				goto out;
			}

			rc = callback_table[msg->hdr.type]((struct pcn_kmsg_message *)lg_buf[msg->hdr.from_cpu]);

			if (rc) {
				PCN_ERROR("%s: Large message callback failed!\n", __func__);
			}
		}
	}

out:

	win_advance_tail(rkvirt[my_cpu]);

	return rc;
}

static int process_small_message(struct pcn_kmsg_message *msg)
{
	int rc;
	struct pcn_kmsg_container *incoming;

	/* malloc some memory (don't sleep!) */
	incoming = kmalloc(sizeof(struct pcn_kmsg_container), GFP_ATOMIC);
	if (!incoming) {
		PCN_ERROR("%s: Unable to kmalloc buffer for incoming message!  THIS IS BAD!\n", __func__);
		win_advance_tail(rkvirt[my_cpu]);
		return -1;
	}

	/* memcpy message from rbuf */
	memcpy(&incoming->msg, msg,
	       sizeof(struct pcn_kmsg_message));
	win_advance_tail(rkvirt[my_cpu]);

	PCN_DEBUG("%s: Received message, type %d, prio %d\n", __func__,
	       incoming->msg.hdr.type, incoming->msg.hdr.prio);

	/* add container to appropriate list */
	switch (incoming->msg.hdr.prio) {
		case PCN_KMSG_PRIO_HIGH:
			PCN_DEBUG("%s: Adding to high-priority list...\n", __func__);
			list_add_tail(&(incoming->list),
				      &msglist_hiprio);
			break;

		case PCN_KMSG_PRIO_NORMAL:
			PCN_DEBUG("%s: Adding to normal-priority list...\n", __func__);
			list_add_tail(&(incoming->list),
				      &msglist_normprio);
			break;

		default:
			PCN_ERROR("%s: Priority value %d unknown -- THIS IS BAD!\n", __func__,
			       incoming->msg.hdr.prio);
	}

	return rc;
}

static void process_mcast_queue(pcn_kmsg_mcast_id id)
{
	struct pcn_kmsg_message *msg;
	while (!mcastwin_get(id, &msg)) {
		PCN_DEBUG("%s: Got an mcast message!\n", __func__);

		mcastwin_advance_tail(id);
	}

}

/* bottom half */
static void pcn_kmsg_action(struct softirq_action *h)
{
	int rc;
	struct pcn_kmsg_message *msg;
	int i;

	PCN_DEBUG("%s: Popcorn kmsg softirq handler called...\n", __func__);

	/* Get messages out of the buffer first */

	while (!win_get(rkvirt[my_cpu], &msg)) {
		PCN_DEBUG("%s: Got a message!\n", __func__);
		/* Special processing for large messages */
		if (msg->hdr.is_lg_msg) {
			PCN_DEBUG("%s: Message is a large message!\n", __func__);
			rc = process_large_message(msg);
		} else {
			PCN_DEBUG("%s: Message is a small message!\n",  __func__);
			rc = process_small_message(msg);
		}

	}

	PCN_DEBUG("%s: No more messages in ring buffer; checking multicast queues...\n", __func__);

	for (i = 0; i < POPCORN_MAX_MCAST_CHANNELS; i++) {
		if (MCASTWIN(i)) {
			PCN_DEBUG("%s: mcast window %d mapped, processing it...\n", __func__, i);
			process_mcast_queue(i);
		}
	}

	PCN_DEBUG("%s: Done checking multicast queues; processing messages...\n", __func__);

	/* Process high-priority queue first */
	rc = process_message_list(&msglist_hiprio);

	if (list_empty(&msglist_hiprio)) {
		PCN_WARN("%s: High-priority queue is empty!\n", __func__);
	}

	/* Then process normal-priority queue */
	rc = process_message_list(&msglist_normprio);

	return;
}

/* Syscall for testing all this stuff */
SYSCALL_DEFINE1(popcorn_test_kmsg, int, cpu)
{
	int rc = 0;

#if 1
	/* test mask includes specified CPU and CPU 0 */
	unsigned long mask = (1 << cpu) | 1;
	pcn_kmsg_mcast_id test_id = -1;

	rc = pcn_kmsg_mcast_open(&test_id, mask);
	if (rc) {
		printk("POPCORN: pcn_kmsg_mcast_open returned %d, test_id %lu\n", 
		       rc, test_id);
	}

#else

	struct pcn_kmsg_long_message lmsg;
	char *str = "This is a very long test message.  Don't be surprised if it gets corrupted; it probably will.  If it does, you're in for a lot more work, and may not get home to see your wife this weekend.  You should knock on wood before running this test.";


	lmsg.hdr.type = PCN_KMSG_TYPE_TEST;
	lmsg.hdr.prio = PCN_KMSG_PRIO_NORMAL;

	strcpy(&lmsg.payload, str); 

	printk("Message to send: %s\n", &lmsg.payload);

	printk("POPCORN: syscall to test kernel messaging, to CPU %d\n", cpu);

	rc = pcn_kmsg_send_long(cpu, &lmsg, strlen(str) + 5);

	if (rc) {
		printk("POPCORN: error: pcn_kmsg_send_long returned %d\n", rc);
	}

#endif

	return rc;
}

/* MULTICAST */

inline int count_members(unsigned long mask)
{
	int i, count = 0;

	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (mask & (1ULL << i)) {
			count++;
		}
	}

	return count;
}

void print_mcast_map(void)
{
	int i;

	printk("ACTIVE MCAST GROUPS:\n");

	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (rkinfo->mcast_wininfo[i].mask) {
			printk("group %d, mask 0x%lx, num_members %d\n", 
			       i, rkinfo->mcast_wininfo[i].mask, 
			       rkinfo->mcast_wininfo[i].num_members);
		}
	}
	return;
}

/* Open a multicast group containing the CPUs specified in the mask. */
int pcn_kmsg_mcast_open(pcn_kmsg_mcast_id *id, unsigned long mask)
{
	int rc, i, found_id = -1;
	struct pcn_kmsg_mcast_message msg;
	struct pcn_kmsg_mcast_wininfo *slot;
	struct pcn_kmsg_mcast_window * new_win;

	PCN_DEBUG("%s: Reached pcn_kmsg_mcast_open, mask 0x%lx\n", __func__, mask);

	if (!(mask & (1 << my_cpu))) {
		PCN_ERROR("%s: This CPU is not a member of the mcast group to be created, cpu %d, mask 0x%lx\n", __func__,
		       my_cpu, mask);
		return -1;
	}

	for (i = 0; i < POPCORN_MAX_MCAST_CHANNELS; i++) {
		if (!rkinfo->mcast_wininfo[i].num_members) {
			found_id = i;
			break;
		}
	}

	PCN_DEBUG("%s: Found channel ID %d\n", __func__, found_id);

	if (found_id == -1) {
		PCN_ERROR("%s: No free multicast channels!\n", __func__);
		return -1;
	}

	/* TODO -- lock and check if channel is still unused; 
	   otherwise, try again */

	slot = &rkinfo->mcast_wininfo[found_id];

	slot->mask = mask;
	slot->num_members = count_members(mask);
	slot->owner_cpu = my_cpu;

	PCN_DEBUG("%s: Found %d members\n", __func__, slot->num_members);

	new_win = kmalloc(sizeof(struct pcn_kmsg_mcast_window), GFP_ATOMIC);

	if (!new_win) {
		PCN_ERROR("%s: Failed to kmalloc mcast buffer!\n", __func__);
		goto out;
	}

	MCASTWIN(found_id) = new_win;
	slot->phys_addr = virt_to_phys(new_win);
	PCN_DEBUG("%s: Malloced mcast receive window %d at phys addr 0x%lx\n", __func__,
	       found_id, slot->phys_addr);

	msg.hdr.type = PCN_KMSG_TYPE_MCAST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.type = PCN_KMSG_MCAST_OPEN;
	msg.id = found_id;
	msg.mask = mask;
	msg.num_members = slot->num_members;

	/* send message to each member except self.  Can't use mcast yet because
	   group is not yet established, so unicast to each CPU in mask. */
	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if ((slot->mask & (1ULL << i)) && 
		    (my_cpu != i)) {
			PCN_DEBUG("%s: Sending message to CPU %d\n", __func__, i);

			rc = pcn_kmsg_send(i, (struct pcn_kmsg_message *) &msg);

			if (rc) {
				PCN_ERROR("%s: Message send failed!\n", __func__);
			}
		}
	}

	*id = found_id;

out:
	/* TODO -- unlock */

	return 0;
}

/* Add new members to a multicast group. */
int pcn_kmsg_mcast_add_members(pcn_kmsg_mcast_id id, unsigned long mask)
{
	/* TODO -- lock! */

	rkinfo->mcast_wininfo[id].mask |= mask; 

	/* TODO -- unlock! */

	return 0;
}

/* Remove existing members from a multicast group. */
int pcn_kmsg_mcast_delete_members(pcn_kmsg_mcast_id id, unsigned long mask)
{
	/* TODO -- lock! */

	rkinfo->mcast_wininfo[id].mask &= !mask;

	/* TODO -- unlock! */


	return 0;
}

inline int __pcn_kmsg_mcast_close(pcn_kmsg_mcast_id id)
{
	/* TODO -- lock! */

	PCN_DEBUG("%s: Closing multicast channel %lu on CPU %d\n", __func__, id, my_cpu);

	/* TODO --unlock! */

	return 0;
}

/* Close a multicast group. */
int pcn_kmsg_mcast_close(pcn_kmsg_mcast_id id)
{
	int rc;
	struct pcn_kmsg_mcast_message msg;

	/* broadcast message to close window globally */
	msg.hdr.type = PCN_KMSG_TYPE_MCAST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.type = PCN_KMSG_MCAST_CLOSE;
	msg.id = id;

	rc = pcn_kmsg_mcast_send(id, (struct pcn_kmsg_message *) &msg);
	if (rc) {
		PCN_ERROR("%s: POPCORN: failed to send mcast close message!\n", __func__);
		return -1;
	}

	/* close window locally */
	__pcn_kmsg_mcast_close(id);

	rkinfo->mcast_wininfo[id].mask = 0;
	rkinfo->mcast_wininfo[id].num_members = 0;

	return 0;
}

/* Send a message to the specified multicast group. */
int pcn_kmsg_mcast_send(pcn_kmsg_mcast_id id, struct pcn_kmsg_message *msg)
{
	int i, rc;

	PCN_DEBUG("%s: Sending mcast message, id %lu\n", __func__, id);

	/* quick hack for testing for now; 
	   loop through mask and send individual messages */
	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (rkinfo->mcast_wininfo[id].mask & (0x1 << i)) {
			rc = pcn_kmsg_send(i, msg);

			if (rc) {
				PCN_ERROR("%s: Batch send failed to CPU %d\n", __func__,  i);
				return -1;
			}
		}
	}

	return 0;
}

/* Send a message to the specified multicast group. */
int pcn_kmsg_mcast_send_long(pcn_kmsg_mcast_id id, 
			     struct pcn_kmsg_long_message *msg, 
			     unsigned int payload_size)
{
	int i, rc;

	PCN_DEBUG("%s: Sending long mcast message, id %lu, size %u\n", 
	       __func__, id, payload_size);

	/* quick hack for testing for now; 
	   loop through mask and send individual messages */
	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (rkinfo->mcast_wininfo[id].mask & (0x1 << i)) {
			rc = pcn_kmsg_send_long(i, msg, payload_size);

			if (rc) {
				PCN_ERROR("%s: Batch send failed to CPU %d\n", __func__, i);
				return -1;
			}
		}
	}

	return 0;
}


static int pcn_kmsg_mcast_callback(struct pcn_kmsg_message *message) 
{
	int rc = 0;
	struct pcn_kmsg_mcast_message *msg = 
		(struct pcn_kmsg_mcast_message *) message;
	pcn_kmsg_work_t *kmsg_work;

	printk("Received mcast message, type %d\n", msg->type);

	switch (msg->type) {
		case PCN_KMSG_MCAST_OPEN:
			PCN_DEBUG("%s: Processing mcast open message...\n", __func__);

			/* Need to queue work to remap the window in a kernel
			   thread; it can't happen here */
			kmsg_work = kmalloc(sizeof(pcn_kmsg_work_t), GFP_ATOMIC);
			if (kmsg_work) {
				INIT_WORK((struct work_struct *) kmsg_work,
					  process_kmsg_wq_item);
				kmsg_work->op = PCN_KMSG_WQ_OP_MAP_MCAST_WIN;
				kmsg_work->from_cpu = msg->hdr.from_cpu;
				kmsg_work->id_to_join = msg->id;
				queue_work(kmsg_wq, (struct work_struct *) kmsg_work);
			} else {
				PCN_ERROR("%s: Failed to kmalloc work structure; this is VERY BAD!\n", __func__);
			}

			break;

		case PCN_KMSG_MCAST_ADD_MEMBERS:
			PCN_DEBUG("%s: Processing mcast add members message...\n", __func__);
			break;

		case PCN_KMSG_MCAST_DEL_MEMBERS:
			PCN_DEBUG("%s: Processing mcast del members message...\n", __func__);
			break;

		case PCN_KMSG_MCAST_CLOSE:
			PCN_DEBUG("%s: Processing mcast close message...\n", __func__);
			__pcn_kmsg_mcast_close(msg->id);
			break;

		default:
			PCN_ERROR("%s: Invalid multicast message type %d\n", __func__,
			       msg->type);
			rc = -1;
			goto out;
	}

	print_mcast_map();

out:
	kfree(message);
	return rc;
}



