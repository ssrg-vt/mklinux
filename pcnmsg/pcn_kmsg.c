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

void pcn_kmsg_action(struct softirq_action *h);

/* RING BUFFER */

#define RB_SHIFT 6
#define RB_SIZE (1 << RB_SHIFT)
#define RB_MASK ((1 << RB_SHIFT) - 1)

/* From Wikipedia page "Fetch and add", modified to work for u64 */
inline unsigned long fetch_and_add( unsigned long * variable, unsigned long value )
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

static inline int win_put(struct pcn_kmsg_window *win, struct pcn_kmsg_message *msg) 
{
	unsigned long ticket;

	/* if the queue is already really long, return EAGAIN */
	if (win_inuse(win) >= RB_SIZE) {
		printk("Window full, caller should try again...\n");
		return -EAGAIN;
	}

	/* grab ticket */
	ticket = fetch_and_add(&win->head, 1);
	printk("ticket = %lu, head = %lu\n", ticket, win->head);

	/* spin until there's a spot free for me */
	while (win_inuse(win) >= RB_SIZE) {}

	/* insert item */
	memcpy(&win->buffer[ticket & RB_MASK], msg, sizeof(struct pcn_kmsg_message));

	pcn_barrier();

	/* set completed flag */
	win->buffer[ticket & RB_MASK].hdr.ready = 1;

	return 0;
}

static inline int win_get(struct pcn_kmsg_window *win, struct pcn_kmsg_message **msg) 
{
	struct pcn_kmsg_message *rcvd;

	if (!win_inuse(win)) {
		printk("Nothing in buffer, returning...\n");
		return -1;
	}

	printk("reached win_get, head %lu, tail %lu\n", win->head, win->tail);

	/* spin until entry.ready at end of cache line is set */
	rcvd = &(win->buffer[win->tail & RB_MASK]);
	printk("Ready bit: %u\n", rcvd->hdr.ready);
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

int pcn_kmsg_checkin_callback(struct pcn_kmsg_message *message) 
{
	struct pcn_kmsg_checkin_message *msg = (struct pcn_kmsg_checkin_message *) message;
	int from_cpu = msg->hdr.from_cpu;

	printk("Called Popcorn callback for processing check-in messages\n");

	printk("From CPU %d, type %d, window phys addr 0x%lx\n", 
			msg->hdr.from_cpu, msg->hdr.type, 
			msg->window_phys_addr);

	if (from_cpu >= POPCORN_MAX_CPUS) {
		printk("Invalid source CPU %d\n", msg->hdr.from_cpu);
		return -1;
	}

	if (!msg->window_phys_addr) {
		printk("Window physical address from CPU %d is NULL!\n", from_cpu);
		return -1;
	}

	//rkinfo->phys_addr[from_cpu] = msg->window_phys_addr;

	/* Note that we're not allowed to ioremap anything from a bottom half,
	   so we'll do it the first time this kernel tries to send a message
	   to the remote kernel. */

	kfree(message);

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

void __init pcn_kmsg_init(void)
{
	int rc;
	unsigned long win_virt_addr, win_phys_addr, rkinfo_phys_addr;
	struct boot_params * boot_params_va;

	printk("Entered pcn_kmsg_init\n");

	printk("ALIGNMENT INFO for pcn_kmsg_container: size %lu, list %lu, msg %lu\n",
			sizeof(struct pcn_kmsg_container),
			offsetof(struct pcn_kmsg_container, list),
			offsetof(struct pcn_kmsg_container, msg));

	my_cpu = raw_smp_processor_id();

	/* Initialize list heads */
	INIT_LIST_HEAD(&msglist_hiprio);
	INIT_LIST_HEAD(&msglist_normprio);

	/* Clear callback table and register default callback functions */
	memset(&callback_table, 0, PCN_KMSG_TYPE_MAX * sizeof(pcn_kmsg_cbftn));
	rc = pcn_kmsg_register_callback(PCN_KMSG_TYPE_CHECKIN, &pcn_kmsg_checkin_callback);
	if (rc) {
		printk("POPCORN: Failed to register initial kmsg checkin callback!\n");
	}

	rc = pcn_kmsg_register_callback(PCN_KMSG_TYPE_MCAST, &pcn_kmsg_mcast_callback);
	if (rc) {
		printk("POPCORN: Failed to register initial kmsg multicast callback!\n");
	}

	/* Register softirq handler */
	open_softirq(PCN_KMSG_SOFTIRQ, pcn_kmsg_action);

	/* If we're the master kernel, malloc and map the rkinfo structure and put
	   its physical address in boot_params; otherwise, get it from the boot_params 
	   and map it */
	if (!mklinux_boot) {
		printk("We're the master; mallocing rkinfo...\n");
		rkinfo = kmalloc(sizeof(struct pcn_kmsg_rkinfo), GFP_KERNEL);

		if (!rkinfo) {
			printk("Failed to malloc rkinfo structure -- this is very bad!\n");
			return;
		}

		rkinfo_phys_addr = virt_to_phys(rkinfo);

		printk("rkinfo virt addr 0x%p, phys addr 0x%lx\n", rkinfo, rkinfo_phys_addr);

		memset(rkinfo, 0x0, sizeof(struct pcn_kmsg_rkinfo));

		printk("Setting boot_params...\n");
		/* Otherwise, we need to set the boot_params to show the rest
		   of the kernels where the master kernel's messaging window is. */
		boot_params_va = (struct boot_params *) (0xffffffff80000000 + orig_boot_params);
		printk("Boot params virtual address: 0x%p\n", boot_params_va);
		printk("Test: kernel alignment %d\n", boot_params_va->hdr.kernel_alignment);
		boot_params_va->pcn_kmsg_master_window = rkinfo_phys_addr;
	} else {
		printk("Master kernel rkinfo phys addr: 0x%lx\n", (unsigned long) boot_params.pcn_kmsg_master_window);

		rkinfo_phys_addr = boot_params.pcn_kmsg_master_window;

		rkinfo = ioremap_cache(rkinfo_phys_addr, sizeof(struct pcn_kmsg_rkinfo));

		if (!rkinfo) {
			printk("Failed to ioremap rkinfo struct from master kernel!\n");
		}

		printk("rkinfo virt addr: 0x%p\n", rkinfo);
	}


	/* Malloc our own receive buffer and set it up */
	win_virt_addr = __get_free_pages(GFP_KERNEL, 2);
	printk("Allocated 4 pages for my window, virt addr 0x%lx\n", win_virt_addr);
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
		rc = send_checkin_msg(0);
		if (rc) { 
			printk("POPCORN: Failed to check in!\n");
		}
	} 

	return;
}

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

int pcn_kmsg_send(unsigned int dest_cpu, struct pcn_kmsg_message *msg)
{
	int rc;
	struct pcn_kmsg_window *dest_window;

	if (dest_cpu >= POPCORN_MAX_CPUS) {
		printk("POPCORN: Invalid destination CPU %d\n", dest_cpu);
		return -1;
	}

	dest_window = rkvirt[dest_cpu];

	if (!dest_window) {
		printk("POPCORN: Destination window for CPU %d not mapped!\n", dest_cpu);
		/* check if phys addr exists, and if so, map it */
		if (rkinfo->phys_addr[dest_cpu]) {
			rkvirt[dest_cpu] = ioremap_cache(rkinfo->phys_addr[dest_cpu], 
					sizeof(struct pcn_kmsg_rkinfo));
			if (rkvirt[dest_cpu]) {
				dest_window = rkvirt[dest_cpu];
			} else {
				printk("POPCORN: Failed to ioremap CPU %d's window at phys addr 0x%lx\n",
						dest_cpu, rkinfo->phys_addr[dest_cpu]);
			}

		} else {
			printk("POPCORN: No physical address known for CPU %d's window!\n", dest_cpu);
			return -1;
		}
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

/* RECEIVING / UNMARSHALING */

int process_message_list(struct list_head *head) 
{
	int rc, rc_overall = 0;
	struct pcn_kmsg_container *pos = NULL, *n = NULL;
	struct pcn_kmsg_message *msg;

	list_for_each_entry_safe(pos, n, head, list) {
		msg = &pos->msg;

		printk("Item in list, type %d,  processing it...\n", msg->hdr.type);

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

	irq_exit();
	return;
}

/* bottom half */
void pcn_kmsg_action(struct softirq_action *h)
{
	int rc;
	struct pcn_kmsg_message *msg;
	struct pcn_kmsg_container *incoming;

	printk("Popcorn kmsg softirq handler called...\n");

	/* Get messages out of the buffer first */

	while (!win_get(rkvirt[my_cpu], &msg)) {
		printk("Got a message!\n");

		/* malloc some memory (don't sleep!) */
		incoming = kmalloc(sizeof(struct pcn_kmsg_container), GFP_ATOMIC);
		if (!incoming) {
			printk("Unable to kmalloc buffer for incoming message!  THIS IS BAD!\n");
			goto out;
		}

		/* memcpy message from rbuf */
		memcpy(&incoming->msg, msg, sizeof(struct pcn_kmsg_message));
		win_advance_tail(rkvirt[my_cpu]);

		printk("Received message, type %d, prio %d\n", incoming->msg.hdr.type, incoming->msg.hdr.prio);

		/* add container to appropriate list */
		switch (incoming->msg.hdr.prio) {
			case PCN_KMSG_PRIO_HIGH:
				printk("Adding to high-priority list...\n");
				list_add_tail(&(incoming->list), &msglist_hiprio);
				break;

			case PCN_KMSG_PRIO_NORMAL:
				printk("Adding to normal-priority list...\n");
				list_add_tail(&(incoming->list), &msglist_normprio);
				break;

			default:
				printk("Priority value %d unknown -- THIS IS BAD!\n", incoming->msg.hdr.prio);
				goto out;
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
	int rc;
	/* test mask includes specified CPU and CPU 0 */
	unsigned long mask = (1 << cpu) | 1;
	pcn_kmsg_mcast_id test_id = -1;

	printk("POPCORN: syscall to test kernel messaging, to CPU %d\n", cpu);

	rc = pcn_kmsg_mcast_open(&test_id, mask);
	if (rc) {
		printk("POPCORN: pcn_kmsg_mcast_open returned %d, test_id %lu\n", rc, test_id);
	}

	return rc;
}

/* MULTICAST */

struct pcn_kmsg_mcast_map {
	unsigned char lock;
	unsigned long mask;
	unsigned int num_members;
};

struct pcn_kmsg_mcast_map mcast_map[POPCORN_MAX_MCAST_CHANNELS];

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

	printk("ACTIVE MCAST GROUPS ON CPU %d\n:", my_cpu);

	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (mcast_map[i].mask) {
			printk("group %d, mask 0x%lx, num_members %d\n", 
					i, mcast_map[i].mask, mcast_map[i].num_members);
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
		if (!mcast_map[i].num_members) {
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

	mcast_map[found_id].mask = mask;
	mcast_map[found_id].num_members = count_members(mask);

	printk("Found %d members\n", mcast_map[found_id].num_members);

	msg.hdr.type = PCN_KMSG_TYPE_MCAST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.type = PCN_KMSG_MCAST_OPEN;
	msg.id = found_id;
	msg.mask = mask;
	msg.num_members = mcast_map[found_id].num_members;

	/* send message to each member except self.  Can't use mcast yet because
	   group is not yet established, so unicast to each CPU in mask. */
	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if ((mcast_map[found_id].mask & (1ULL << i)) && (my_cpu != i)) {
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

	mcast_map[id].mask |= mask; 

	/* TODO -- unlock! */

	return 0;
}

/* Remove existing members from a multicast group. */
int pcn_kmsg_mcast_delete_members(pcn_kmsg_mcast_id id, unsigned long mask)
{
	/* TODO -- lock! */

	mcast_map[id].mask &= !mask;

	/* TODO -- unlock! */


	return 0;
}

inline int __pcn_kmsg_mcast_close(pcn_kmsg_mcast_id id)
{
	/* TODO -- lock! */

	mcast_map[id].mask = 0;
	mcast_map[id].num_members = 0;

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

	return 0;
}

/* Send a message to the specified multicast group. */
int pcn_kmsg_mcast_send(pcn_kmsg_mcast_id id, struct pcn_kmsg_message *msg)
{
	int i, rc;

	/* quick hack for testing for now; loop through mask and send individual messages */
	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (mcast_map[id].mask & (0x1 << i)) {
			rc = pcn_kmsg_send(i, msg);

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
			mcast_map[msg->id].mask = msg->mask;
			mcast_map[msg->id].num_members = msg->num_members;
			break;

		case PCN_KMSG_MCAST_ADD_MEMBERS:
			printk("Processing mcast add members message...\n");
			mcast_map[msg->id].mask |= msg->mask;
			mcast_map[msg->id].num_members = count_members(msg->mask);
			break;

		case PCN_KMSG_MCAST_DEL_MEMBERS:
			printk("Processing mcast del members message...\n");
			mcast_map[msg->id].mask &= !(msg->mask);
			mcast_map[msg->id].num_members = count_members(msg->mask);
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



