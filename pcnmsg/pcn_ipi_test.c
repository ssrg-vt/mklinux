/*
 * IPI latency measurement for Popcorn
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2013
 */

#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/smp.h>
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/multikernel.h>
#include <linux/list.h>
#include <linux/slab.h>

#include <asm/system.h>
#include <asm/apic.h>
#include <asm/hardirq.h>
#include <asm/setup.h>
#include <asm/bootparam.h>
#include <asm/errno.h>

volatile unsigned long tsc;
volatile int done;

extern int my_cpu;

/*
static inline unsigned long rdtscll(void)
{
	unsigned int a, d;
	asm volatile ("rdtsc" : "=a" (a), "=d" (d));
	return ((unsigned long) a) | (((unsigned long) d) << 32);
}
*/


/* Ping-pong interrupt handler */
void smp_popcorn_ipi_latency_interrupt(struct pt_regs *regs)
{
	ack_APIC_irq();

	//printk("Ping-pong IPI received!\n");

	if (my_cpu) {
		//printk("Sending IPI back to CPU 0...\n");
		// send IPI back to sender
		apic->send_IPI_mask(cpumask_of(0), POPCORN_IPI_LATENCY_VECTOR);
	} else {
		//printk("Received ping-pong; reading end timestamp...\n");
		rdtscll(tsc);
		done = 1;
	}

	return;
}

/* Syscall for testing all this stuff */
SYSCALL_DEFINE1(popcorn_test_ipi_latency, int, cpu)
{
	int rc = 0;
	unsigned long tsc_init;

	printk("Reached IPI latency syscall, sending to CPU %d\n", cpu);

	done = 0;

	rdtscll(tsc_init);

	apic->send_IPI_mask(cpumask_of(cpu), POPCORN_IPI_LATENCY_VECTOR);
	
	while (!done) {}

	printk("initial tsc - final tsc: %lu\n", tsc - tsc_init);

	return rc;
}
