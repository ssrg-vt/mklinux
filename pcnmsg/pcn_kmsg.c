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

/* lists of messages to be processed for each prio */
struct list_head msglist_hiprio, msglist_normprio;

/* array to hold pointers to large messages received */
struct pcn_kmsg_long_message * lg_buf[POPCORN_MAX_CPUS];

/* action for bottom half */
void pcn_kmsg_action(struct softirq_action *h);

/* workqueue for operations that can sleep */
static struct workqueue_struct *kmsg_wq;

/* RING BUFFER */

#define RB_SHIFT 6
#define RB_SIZE (1 << RB_SHIFT)
#define RB_MASK ((1 << RB_SHIFT) - 1)

/* From Wikipedia page "Fetch and add", modified to work for u64 */
inline unsigned long fetch_and_add(volatile unsigned long * variable, 
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
	printk(KERN_ERR "ticket = %lu, head = %lu, tail = %lu\n", 
	       ticket, win->head, win->tail);

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
		printk(KERN_ERR "Nothing in buffer, returning...\n");
		return -1;
	}

	printk(KERN_ERR "reached win_get, head %lu, tail %lu\n", 
	       win->head, win->tail);

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


/* INITIALIZATION */

int pcn_kmsg_mcast_callback(struct pcn_kmsg_message *message);



typedef struct {
	struct work_struct work;
	int cpu;
} pcn_kmsg_work_t;

/* bottom half for workqueue */
void process_kmsg_wq_item(struct work_struct * work)
{
	int cpu;

	pcn_kmsg_work_t *w = (pcn_kmsg_work_t *) work;
	cpu = w->cpu;

	printk("Workqueue bottom half running, CPU %d\n", cpu);

	rkvirt[cpu] = ioremap_cache(rkinfo->phys_addr[cpu],
				    sizeof(struct pcn_kmsg_rkinfo));
	if (rkvirt[cpu]) {
		printk("POPCORN: ioremapped window, virt addr 0x%p\n", 
		       rkvirt[cpu]);
	} else {
		printk("POPCORN: Failed to ioremap CPU %d's window at phys addr 0x%lx\n",
		       cpu, rkinfo->phys_addr[cpu]);
	}

}

int pcn_kmsg_checkin_callback(struct pcn_kmsg_message *message) 
{
	struct pcn_kmsg_checkin_message *msg = (struct pcn_kmsg_checkin_message *) message;
	int from_cpu = msg->hdr.from_cpu;
	pcn_kmsg_work_t *kmsg_work = NULL;

	printk("Called Popcorn callback for processing check-in messages\n");

	printk("From CPU %d, type %d, window phys addr 0x%lx\n", 
	       msg->hdr.from_cpu, msg->hdr.type, 
	       msg->window_phys_addr);

	if (from_cpu >= POPCORN_MAX_CPUS) {
		printk("Invalid source CPU %d\n", msg->hdr.from_cpu);
		return -1;
	}

	if (!msg->window_phys_addr) {
		printk("Window physical address from CPU %d is NULL!\n", 
		       from_cpu);
		return -1;
	}

	/* Note that we're not allowed to ioremap anything from a bottom half,
	   so we'll add it to a workqueue and do it in a kernel thread. */
	printk("Testing passing work off to the work queue...\n");
	kmsg_work = kmalloc(sizeof(pcn_kmsg_work_t),GFP_ATOMIC);
	if (kmsg_work) {
		INIT_WORK((struct work_struct *) kmsg_work, process_kmsg_wq_item);
		kmsg_work->cpu = from_cpu;
		queue_work(kmsg_wq, (struct work_struct *) kmsg_work);
	} else {
		printk("Failed to malloc work structure; this is VERY BAD!\n");
	}

	kfree(message);

	return 0;
}

int pcn_kmsg_test_callback(struct pcn_kmsg_message *message)
{
	struct pcn_kmsg_long_message *lmsg = message;

	printk("Received test long message, payload: %s\n", &lmsg->payload);

	return 0;
}

inline int pcn_kmsg_window_init(struct pcn_kmsg_window *window)
{
	window->head = 0;
	window->tail = 0;
	memset(&window->buffer, 0, PCN_KMSG_RBUF_SIZE * sizeof(struct pcn_kmsg_message));
	return 0;
}

extern unsigned long orig_boot_params;

int send_checkin_msg(unsigned int to_cpu)
{
	int rc;
	struct pcn_kmsg_checkin_message msg;

	msg.hdr.type = PCN_KMSG_TYPE_CHECKIN;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.window_phys_addr = rkinfo->phys_addr[my_cpu];

	rc = pcn_kmsg_send(to_cpu, (struct pcn_kmsg_message *) &msg);

	if (rc) {
		printk("Failed to send checkin message, rc = %d\n", rc);
		return rc;
	}

	return 0;
}

static int __init pcn_kmsg_init(void)
{
	int rc;
	unsigned long win_virt_addr, win_phys_addr, rkinfo_phys_addr;
	struct boot_params * boot_params_va;

	printk("Entered pcn_kmsg_init\n");

	my_cpu = raw_smp_processor_id();

	/* Initialize list heads */
	INIT_LIST_HEAD(&msglist_hiprio);
	INIT_LIST_HEAD(&msglist_normprio);

	/* Clear out large-message receive buffers */
	memset(&lg_buf, 0, POPCORN_MAX_CPUS * sizeof(unsigned char *));

	/* Clear callback table and register default callback functions */
	printk("Registering initial callbacks...\n");
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
	printk("Registering softirq handler...\n");
	open_softirq(PCN_KMSG_SOFTIRQ, pcn_kmsg_action);

	/* Initialize work queue */
	printk("Initializing workqueue...\n");
	kmsg_wq = create_singlethread_workqueue("kmsg_wq");

	/* If we're the master kernel, malloc and map the rkinfo structure and 
	   put its physical address in boot_params; otherwise, get it from the 
	   boot_params and map it */
	if (!mklinux_boot) {
		printk("We're the master; mallocing rkinfo...\n");
		rkinfo = kmalloc(sizeof(struct pcn_kmsg_rkinfo), GFP_KERNEL);

		if (!rkinfo) {
			printk("Failed to malloc rkinfo structure -- this is very bad!\n");
			return -1;
		}

		rkinfo_phys_addr = virt_to_phys(rkinfo);

		printk("rkinfo virt addr 0x%p, phys addr 0x%lx\n", 
		       rkinfo, rkinfo_phys_addr);

		memset(rkinfo, 0x0, sizeof(struct pcn_kmsg_rkinfo));

		printk("Setting boot_params...\n");
		/* Otherwise, we need to set the boot_params to show the rest
		   of the kernels where the master kernel's messaging window is. */
		boot_params_va = (struct boot_params *) (0xffffffff80000000 + orig_boot_params);
		printk("Boot params virtual address: 0x%p\n", boot_params_va);
		boot_params_va->pcn_kmsg_master_window = rkinfo_phys_addr;
	} else {
		printk("Master kernel rkinfo phys addr: 0x%lx\n", 
		       (unsigned long) boot_params.pcn_kmsg_master_window);

		rkinfo_phys_addr = boot_params.pcn_kmsg_master_window;
		rkinfo = ioremap_cache(rkinfo_phys_addr, 
				       sizeof(struct pcn_kmsg_rkinfo));

		if (!rkinfo) {
			printk("Failed to ioremap rkinfo struct from master kernel!\n");
		}

		printk("rkinfo virt addr: 0x%p\n", rkinfo);
	}

	/* Malloc our own receive buffer and set it up */
	win_virt_addr = __get_free_pages(GFP_KERNEL, 2);
	printk("Allocated 4 pages for my window, virt addr 0x%lx\n", 
	       win_virt_addr);
	rkvirt[my_cpu] = (struct pcn_kmsg_window *) win_virt_addr;
	win_phys_addr = virt_to_phys((void *) win_virt_addr);
	printk("Physical address: 0x%lx\n", win_phys_addr);
	rkinfo->phys_addr[my_cpu] = win_phys_addr;

	rc = pcn_kmsg_window_init(rkvirt[my_cpu]);
	if (rc) {
		printk("POPCORN: Failed to initialize kmsg recv window!\n");
	}

	/* If we're not the master kernel, we need to check in */
	if (mklinux_boot) {
		/* Need to map kernel 0's receive window */
		rkvirt[0] = ioremap_cache(rkinfo->phys_addr[0],
					  sizeof(struct pcn_kmsg_rkinfo));
		if (rkvirt[0]) {
			printk("POPCORN: ioremapped host CPU's window, virt addr 0x%p\n", rkvirt[0]);
		} else {
			printk("POPCORN: Failed to ioremap host CPU's window at phys addr 0x%lx\n",
			       rkinfo->phys_addr[0]);
			return -1;
		}

		rc = send_checkin_msg(0);
		if (rc) { 
			printk("POPCORN: Failed to check in!\n");
		}
	} 

	return 0;
}

late_initcall(pcn_kmsg_init);

/* Register a callback function when a kernel module is loaded */
int pcn_kmsg_register_callback(enum pcn_kmsg_type type, pcn_kmsg_cbftn callback)
{
	if (type >= PCN_KMSG_TYPE_MAX) {
		printk("POPCORN: Attempted to register callback with bad type %d\n", type);
		return -1;
	}

	callback_table[type] = callback;

	return 0;
}

/* Unregister a callback function when a kernel module is unloaded */
int pcn_kmsg_unregister_callback(enum pcn_kmsg_type type)
{
	if (type >= PCN_KMSG_TYPE_MAX) {
		printk("POPCORN: Attempted to register callback with bad type %d\n", type);
		return -1;
	}

	callback_table[type] = NULL;

	return 0;
}

/* SENDING / MARSHALING */

int __pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg)
{
	int rc;
	struct pcn_kmsg_window *dest_window;

	if (dest_cpu >= POPCORN_MAX_CPUS) {
		printk("POPCORN: Invalid destination CPU %d\n", dest_cpu);
		return -1;
	}

	dest_window = rkvirt[dest_cpu];

	if (!rkvirt[dest_cpu]) {
		printk("POPCORN: Destination window for CPU %d not mapped -- this is VERY BAD!\n", dest_cpu);
		/* check if phys addr exists, and if so, map it */
		return -1;
	}

	if (!msg) {
		printk("POPCORN: Passed in a null pointer to msg!\n");
		return -1;
	}

	/* set source CPU */
	msg->hdr.from_cpu = my_cpu;

	/* place message in rbuf */
	rc = win_put(dest_window, msg);		

	if (rc) {
		printk("POPCORN: Failed to place message in destination window -- maybe it's full?\n");
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

	printk("Sending large message to CPU %d, type %d, payload size %d bytes, %d chunks\n", 
	       dest_cpu, lmsg->hdr.type, payload_size, num_chunks);

	this_chunk.hdr.type = lmsg->hdr.type;
	this_chunk.hdr.prio = lmsg->hdr.prio;
	this_chunk.hdr.is_lg_msg = 1;

	for (i = 0; i < num_chunks; i++) {
		printk("Sending chunk %d\n", i);

		this_chunk.hdr.lg_start = (i == 0) ? 1 : 0;
		this_chunk.hdr.lg_end = (i == num_chunks - 1) ? 1 : 0;
		this_chunk.hdr.lg_seqnum = (i == 0) ? num_chunks : i;

		memcpy(&this_chunk.payload, ((unsigned char *) &lmsg->payload) + i * PCN_KMSG_PAYLOAD_SIZE, PCN_KMSG_PAYLOAD_SIZE);

		//memcpy(test_buf, &this_chunk.payload, 10);
		//test_buf[11] = '\0';

		//printk("First 10 characters: %s\n", test_buf);

		__pcn_kmsg_send(dest_cpu, &this_chunk);
	}

	return 0;
}

/* RECEIVING / UNMARSHALING */

int process_message_list(struct list_head *head) 
{
	int rc, rc_overall = 0;
	struct pcn_kmsg_container *pos = NULL, *n = NULL;
	struct pcn_kmsg_message *msg;

	list_for_each_entry_safe(pos, n, head, list) {
		msg = &pos->msg;

		printk("Item in list, type %d,  processing it...\n", 
		       msg->hdr.type);

		list_del(&pos->list);

		if (msg->hdr.type >= PCN_KMSG_TYPE_MAX || !callback_table[msg->hdr.type]) {
			printk("Invalid type %d; continuing!\n", msg->hdr.type);
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

	printk("Reached Popcorn KMSG interrupt handler!\n");

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

/* bottom half */
void pcn_kmsg_action(struct softirq_action *h)
{
	int rc;
	struct pcn_kmsg_message *msg;
	struct pcn_kmsg_container *incoming;

	printk(KERN_ERR "Popcorn kmsg softirq handler called...\n");

	/* Get messages out of the buffer first */

	while (!win_get(rkvirt[my_cpu], &msg)) {
		printk(KERN_ERR "Got a message!\n");

		/* Special processing for large messages */
		if (msg->hdr.is_lg_msg) {
			printk(KERN_ERR "Got a large message fragment, type %u, from_cpu %u, start %u, end %u, seqnum %u!\n",
			       msg->hdr.type, msg->hdr.from_cpu, 
			       msg->hdr.lg_start, msg->hdr.lg_end, 
			       msg->hdr.lg_seqnum);

			if (msg->hdr.lg_start) {
				printk("Processing initial message fragment...\n");

				lg_buf[msg->hdr.from_cpu] = kmalloc(sizeof(struct pcn_kmsg_hdr) + msg->hdr.lg_seqnum * PCN_KMSG_PAYLOAD_SIZE,
								    GFP_ATOMIC);

				if (!lg_buf[msg->hdr.from_cpu]) {
					printk("Unable to kmalloc buffer for incoming message!  THIS IS BAD!\n");
					goto out;
				}

				memcpy((unsigned char *)lg_buf[msg->hdr.from_cpu], &msg->hdr, sizeof(struct pcn_kmsg_hdr));

				memcpy((unsigned char *)lg_buf[msg->hdr.from_cpu] + sizeof(struct pcn_kmsg_hdr), 
				       &msg->payload, PCN_KMSG_PAYLOAD_SIZE);

				if (msg->hdr.lg_end) {
					printk("NOTE: Long message of length 1 received; this isn't efficient!\n");
					rc = callback_table[msg->hdr.type](lg_buf[msg->hdr.from_cpu]);

					if (rc) {
						printk("Large message callback failed!\n");
					}
				}
			} else {
				printk("Processing subsequent message fragment...\n");

				memcpy((unsigned char *)lg_buf[msg->hdr.from_cpu] + sizeof(struct pcn_kmsg_hdr) + PCN_KMSG_PAYLOAD_SIZE * msg->hdr.lg_seqnum,
				       &msg->payload, PCN_KMSG_PAYLOAD_SIZE);

				if (msg->hdr.lg_end) {
					printk("Last fragment in series...\n");

					printk("from_cpu %d, type %d, prio %d\n", 
					       lg_buf[msg->hdr.from_cpu]->hdr.from_cpu,
					       lg_buf[msg->hdr.from_cpu]->hdr.type,
					       lg_buf[msg->hdr.from_cpu]->hdr.prio);


					rc = callback_table[msg->hdr.type](lg_buf[msg->hdr.from_cpu]);

					if (rc) {
						printk("Large message callback failed!\n");
					}
				}
			}

			win_advance_tail(rkvirt[my_cpu]);
		} else {
			printk(KERN_ERR "Message is a small message!\n");

			/* malloc some memory (don't sleep!) */
			incoming = kmalloc(sizeof(struct pcn_kmsg_container), GFP_ATOMIC);
			if (!incoming) {
				printk("Unable to kmalloc buffer for incoming message!  THIS IS BAD!\n");
				goto out;
			}

			/* memcpy message from rbuf */
			memcpy(&incoming->msg, msg, 
			       sizeof(struct pcn_kmsg_message));
			win_advance_tail(rkvirt[my_cpu]);

			printk("Received message, type %d, prio %d\n", 
			       incoming->msg.hdr.type, incoming->msg.hdr.prio);

			/* add container to appropriate list */
			switch (incoming->msg.hdr.prio) {
				case PCN_KMSG_PRIO_HIGH:
					printk("Adding to high-priority list...\n");
					list_add_tail(&(incoming->list), 
						      &msglist_hiprio);
					break;

				case PCN_KMSG_PRIO_NORMAL:
					printk("Adding to normal-priority list...\n");
					list_add_tail(&(incoming->list), 
						      &msglist_normprio);
					break;

				default:
					printk("Priority value %d unknown -- THIS IS BAD!\n", 
					       incoming->msg.hdr.prio);
					goto out;
			}
		}

	}

	printk("No more messages in ring buffer; done polling\n");

	/* Process high-priority queue first */
	rc = process_message_list(&msglist_hiprio);

	if (list_empty(&msglist_hiprio)) {
		printk("High-priority queue is empty!\n");
	}

	/* Then process normal-priority queue */
	rc = process_message_list(&msglist_normprio);

out:
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
		if (rkinfo->mcast_window[i].mask) {
			printk("group %d, mask 0x%lx, num_members %d\n", 
			       i, rkinfo->mcast_window[i].mask, 
			       rkinfo->mcast_window[i].num_members);
		}
	}
	return;
}

/* Open a multicast group containing the CPUs specified in the mask. */
int pcn_kmsg_mcast_open(pcn_kmsg_mcast_id *id, unsigned long mask)
{
	int rc, i, found_id = -1;
	struct pcn_kmsg_mcast_message msg;

	printk("Reached pcn_kmsg_mcast_open, mask 0x%lx\n", mask);

	for (i = 0; i < POPCORN_MAX_MCAST_CHANNELS; i++) {
		if (!rkinfo->mcast_window[i].num_members) {
			found_id = i;
			break;
		}
	}

	printk("Found channel ID %d\n", found_id);

	if (found_id == -1) {
		printk("No free multicast channels!\n");
		return -1;
	}

	/* TODO -- lock and check if channel is still unused; otherwise, try again */

	rkinfo->mcast_window[found_id].mask = mask;
	rkinfo->mcast_window[found_id].num_members = count_members(mask);

	printk("Found %d members\n", 
	       rkinfo->mcast_window[found_id].num_members);

	msg.hdr.type = PCN_KMSG_TYPE_MCAST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.type = PCN_KMSG_MCAST_OPEN;
	msg.id = found_id;
	msg.mask = mask;
	msg.num_members = rkinfo->mcast_window[found_id].num_members;

	/* send message to each member except self.  Can't use mcast yet because
	   group is not yet established, so unicast to each CPU in mask. */
	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if ((rkinfo->mcast_window[found_id].mask & (1ULL << i)) && (my_cpu != i)) {
			printk("Sending message to CPU %d\n", i);

			rc = pcn_kmsg_send(i, (struct pcn_kmsg_message *) &msg);

			if (rc) {
				printk("Message send failed!\n");
			}
		}
	}

	*id = found_id;

	return 0;
}

/* Add new members to a multicast group. */
int pcn_kmsg_mcast_add_members(pcn_kmsg_mcast_id id, unsigned long mask)
{
	/* TODO -- lock! */

	rkinfo->mcast_window[id].mask |= mask; 

	/* TODO -- unlock! */

	return 0;
}

/* Remove existing members from a multicast group. */
int pcn_kmsg_mcast_delete_members(pcn_kmsg_mcast_id id, unsigned long mask)
{
	/* TODO -- lock! */

	rkinfo->mcast_window[id].mask &= !mask;

	/* TODO -- unlock! */


	return 0;
}

inline int __pcn_kmsg_mcast_close(pcn_kmsg_mcast_id id)
{
	/* TODO -- lock! */

	printk("Closing multicast channel %d on CPU %d\n", id, my_cpu);

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
		printk("POPCORN: failed to send mcast close message!\n");
		return -1;
	}

	/* close window locally */
	__pcn_kmsg_mcast_close(id);

	rkinfo->mcast_window[id].mask = 0;
	rkinfo->mcast_window[id].num_members = 0;

	return 0;
}

/* Send a message to the specified multicast group. */
int pcn_kmsg_mcast_send(pcn_kmsg_mcast_id id, struct pcn_kmsg_message *msg)
{
	int i, rc;

	printk("Sending mcast message, id %lu\n", id);

	/* quick hack for testing for now; 
	   loop through mask and send individual messages */
	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (rkinfo->mcast_window[id].mask & (0x1 << i)) {
			rc = pcn_kmsg_send(i, msg);

			if (rc) {
				printk("Batch send failed to CPU %d\n", i);
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

	printk("Sending long mcast message, id %lu, size %u\n", id, payload_size);

	/* quick hack for testing for now; 
	   loop through mask and send individual messages */
	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (rkinfo->mcast_window[id].mask & (0x1 << i)) {
			rc = pcn_kmsg_send_long(i, msg, payload_size);

			if (rc) {
				printk("Batch send failed to CPU %d\n", i);
				return -1;
			}
		}
	}

	return 0;
}


int pcn_kmsg_mcast_callback(struct pcn_kmsg_message *message) 
{
	int rc = 0;
	struct pcn_kmsg_mcast_message *msg = (struct pcn_kmsg_mcast_message *) message;

	printk("Received mcast message, type %d\n", msg->type);

	switch (msg->type) {
		case PCN_KMSG_MCAST_OPEN:
			printk("Processing mcast open message...\n");
			break;

		case PCN_KMSG_MCAST_ADD_MEMBERS:
			printk("Processing mcast add members message...\n");
			break;

		case PCN_KMSG_MCAST_DEL_MEMBERS:
			printk("Processing mcast del members message...\n");
			break;

		case PCN_KMSG_MCAST_CLOSE:
			printk("Processing mcast close message...\n");
			__pcn_kmsg_mcast_close(msg->id);
			break;

		default:
			printk("Invalid multicast message type %d\n", msg->type);
			rc = -1;
			goto out;
	}

	print_mcast_map();

out:
	kfree(message);
	return rc;
}



