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

/* COMMON STATE */

/* table of callback functions for handling each message type */
pcn_kmsg_cbftn callback_table[PCN_KMSG_TYPE_SIZE];

/* number of current kernel */
int my_cpu = 0;

/* table with phys/virt addresses for remote kernels*/
struct pcn_kmsg_rkinfo rkinfo[POPCORN_MAX_CPUS];

/* lists of messages to be processed for each prio */
struct list_head msglist_hiprio, msglist_normprio;

/* INITIALIZATION */

int pcn_kmsg_checkin_callback(struct pcn_kmsg_message *message) {
	printk("Called Popcorn callback for processing check-in messages\n");

	printk("Type %d, size %d, contents 0x%lx\n", message->hdr.type, message->hdr.size, 
		*((unsigned long *) message->payload));	
	return 0;
}

unsigned long master_kernel_phys_addr;

int pcn_kmsg_window_init(struct pcn_kmsg_window *window)
{

	return 0;
}

extern unsigned long orig_boot_params;

void __init pcn_kmsg_init(void)
{
	int rc;
	unsigned long page_addr, phys_addr;
	void * master_kernel_mapped_addr;
	struct boot_params * boot_params_va;

	printk("Entered setup_popcorn_kmsg\n");

	my_cpu = raw_smp_processor_id();

	/* Initialize list heads */
	INIT_LIST_HEAD(&msglist_hiprio);
	INIT_LIST_HEAD(&msglist_normprio);

	/* Clear callback table and register default callback functions */
	memset(&callback_table, 0, PCN_KMSG_TYPE_SIZE * sizeof(pcn_kmsg_cbftn));
	rc = pcn_kmsg_register_callback(PCN_KMSG_TYPE_CHECKIN, &pcn_kmsg_checkin_callback);
	if (rc) {
		printk("POPCORN: Failed to register initial kmsg recv callback!\n");
	}

	/* Malloc our own receive buffer and set it up */
	page_addr = __get_free_pages(GFP_KERNEL, 2);
	printk("Allocated 4 pages, virt addr 0x%lx\n", page_addr);
	rkinfo[my_cpu].window = (struct pcn_kmsg_window *) page_addr;
	phys_addr = virt_to_phys((void *) page_addr);
	printk("Physical address: 0x%lx\n", phys_addr);

	rc = pcn_kmsg_window_init(rkinfo[my_cpu].window);
	if (rc) {
		printk("POPCORN: Failed to initialize kmsg recv window!\n");
	}

	/* If we're not the master kernel, we need to map the master kernel's
	   messaging window and check in */
	if (mklinux_boot) {
		printk("Master kernel phys addr: 0x%lx\n", boot_params.pcn_kmsg_master_window);

		rkinfo[0].phys_addr = boot_params.pcn_kmsg_master_window;

		rkinfo[0].window = ioremap_cache(rkinfo[0].phys_addr, sizeof(struct pcn_kmsg_window));
		printk("Master kernel virt addr: 0x%lx\n", rkinfo[0].window);
		//printk("First eight bytes: 0x%lx\n", *((unsigned long *) master_kernel_mapped_addr));
	} else {
		printk("We're the master; setting boot_params...\n");
		/* Otherwise, we need to set the boot_params to show the rest
		   of the kernels where the master kernel's messaging window is. */
		boot_params_va = (struct boot_params *) (0xffffffff80000000 + orig_boot_params);
		printk("Boot params virtual address: 0x%p\n", boot_params_va);
		printk("Test: kernel alignment %d\n", boot_params_va->hdr.kernel_alignment);
		boot_params_va->pcn_kmsg_master_window = phys_addr;
	}

	return;
}

/* Register a callback function when a kernel module is loaded */
int pcn_kmsg_register_callback(enum pcn_kmsg_type type, pcn_kmsg_cbftn callback)
{
	if (type >= PCN_KMSG_TYPE_SIZE) {
		printk("POPCORN: Attempted to register callback with bad type %d\n", type);
		return -1;
	}

	callback_table[type] = callback;

	return 0;
}

/* Unregister a callback function when a kernel module is unloaded */
int pcn_kmsg_unregister_callback(enum pcn_kmsg_type type)
{
	if (type >= PCN_KMSG_TYPE_SIZE) {
		printk("POPCORN: Attempted to register callback with bad type %d\n", type);
		return -1;
	}

	callback_table[type] = NULL;

	return 0;
}

/* SENDING / MARSHALING */

int pcn_kmsg_send(int dest_cpu)
{

	/* grab lock */

	/* place message in rbuf */

	/* unlock */

	/* send IPI */
	apic->send_IPI_mask(cpumask_of(dest_cpu), POPCORN_KMSG_VECTOR);

	return 0;
}

/* RECEIVING / UNMARSHALING */

void pcn_kmsg_do_tasklet(unsigned long);
DECLARE_TASKLET(pcn_kmsg_tasklet, pcn_kmsg_do_tasklet, 0);

void pcn_kmsg_do_tasklet(unsigned long unused)
{
	int rc;
	struct pcn_kmsg_container *pos = NULL;

	printk("Tasklet handler called...\n");

	/* Process high-priority queue first */
	list_for_each_entry(pos, &msglist_hiprio, list) {
		printk("Item in high-prio list, type %d,  processing it...\n", pos->hdr.type);

		if (pos->hdr.type >= PCN_KMSG_TYPE_SIZE) {
			printk("Invalid type; continuing!\n");
			continue;
		}

		rc = callback_table[pos->hdr.type]((struct pcn_kmsg_message *) &pos->hdr);
	}

	/* Then process normal-priority queue */
	list_for_each_entry(pos, &msglist_normprio, list) {
		printk("Item in norm-prio list; processing it...\n");
		printk("Size %d, data 0x%lx\n", pos->hdr.size, *((unsigned long *) pos->payload));
	}

	return;
}

void smp_popcorn_kmsg_interrupt(struct pt_regs *regs)
{
	struct pcn_kmsg_container *incoming;

	ack_APIC_irq();

	printk("Reached Popcorn KMSG handler!\n");

	inc_irq_stat(irq_popcorn_kmsg_count);
	irq_enter();

	/* malloc some memory (don't sleep!) */
	incoming = kmalloc(sizeof(struct pcn_kmsg_container), GFP_ATOMIC);
	if (!incoming) {
		printk("Unable to kmalloc buffer for incoming message!  THIS IS BAD!\n");
		goto out;
	}

	/* memcpy message from rbuf */

	/* TEST -- set it up for testing!!! */
	incoming->hdr.type = PCN_KMSG_TYPE_CHECKIN;
	incoming->hdr.prio = PCN_KMSG_PRIO_HIGH;
	incoming->hdr.size = 8;
	memset(&(incoming->payload), 0xab, 8);

	/* add container to appropriate list */
	switch (incoming->hdr.prio) {
		case PCN_KMSG_PRIO_HIGH:
			printk("Adding to high-priority list...\n");
			list_add_tail(&(incoming->list), &msglist_hiprio);
			break;

		case PCN_KMSG_PRIO_NORMAL:
			printk("Adding to normal-priority list...\n");
			list_add_tail(&(incoming->list), &msglist_normprio);
			break;

		default:
			printk("Priority value %d unknown -- THIS IS BAD!\n", incoming->hdr.prio);
			goto out;
	}

	/* schedule bottom half */
	tasklet_schedule(&pcn_kmsg_tasklet);
out:
	irq_exit();
	return;
}

/* Syscall for testing all this stuff */
SYSCALL_DEFINE1(popcorn_test_kmsg, int, cpu)
{
	int rc;

	printk("POPCORN: syscall to test kernel messaging, to CPU %d\n", cpu);

	rc = pcn_kmsg_send(cpu);
	if (rc) {
		printk("POPCORN: syscall screwed up!!!\n");
	}

	return rc;
}

