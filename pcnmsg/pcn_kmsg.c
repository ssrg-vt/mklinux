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

#include <asm/system.h>
#include <asm/apic.h>
#include <asm/hardirq.h>

/* COMMON STATE */

/* table with phys/virt addresses for remote kernels*/
struct pcn_kmsg_rkinfo rkinfo[POPCORN_MAX_CPUS];

/* lists of messages to be processed for each prio */
struct list_head msglist_hiprio, msglist_normprio;

/* INITIALIZATION */

unsigned long master_kernel_phys_addr;

void __init pcn_kmsg_init(void)
{
	unsigned long page_addr, phys_addr;
	void * master_kernel_mapped_addr;

	printk("Entered setup_popcorn_kmsg\n");

	/* Initialize list heads */
	INIT_LIST_HEAD(&msglist_hiprio);
	INIT_LIST_HEAD(&msglist_normprio);

	page_addr = __get_free_pages(GFP_KERNEL | __GFP_ZERO, 2);

	printk("Allocated 4 pages, addr 0x%lx\n", page_addr); 

	phys_addr = virt_to_phys(page_addr);

	printk("Physical address: 0x%lx\n", phys_addr);

	memset(page_addr, 0xbc, 4096);

	/* If we're not the master kernel, we need to map the master kernel's
	   messaging window and check in */
	if (mklinux_boot) {
		printk("Master kernel phys addr: 0x%lx\n", master_kernel_phys_addr);

		master_kernel_mapped_addr = ioremap_cache(master_kernel_phys_addr, 4096);

		printk("Master kernel virt addr: 0x%lx\n", master_kernel_mapped_addr);

		printk("First eight bytes: 0x%lx\n", *((unsigned long *) master_kernel_mapped_addr));
	}

	return;
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
	printk("Tasklet handler called...\n");

	return;
}

void smp_popcorn_kmsg_interrupt(struct pt_regs *regs)
{
	struct pcn_kmsg_container *incoming;

	ack_APIC_irq();

	printk("Reached Popcorn KMSG handler!\n");

	inc_irq_stat(irq_popcorn_kmsg_count);

	irq_enter();

	/* memcpy message from rbuf */

	/* add container to appropriate list */	

	/* schedule bottom half */
	tasklet_schedule(&pcn_kmsg_tasklet);

	irq_exit();

	return;
}

/* Syscall for testing all this stuff out */
SYSCALL_DEFINE1(popcorn_test_kmsg, int, cpu)
{
	printk("POPCORN: syscall to test kernel messaging, to CPU %d\n", cpu);

	apic->send_IPI_mask(cpumask_of(cpu), POPCORN_KMSG_VECTOR);

	return 0;
}

