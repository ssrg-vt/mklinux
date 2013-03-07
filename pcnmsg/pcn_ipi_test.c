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

#define NUM_TRIALS 1000

/* Ping-pong interrupt handler */
void smp_popcorn_ipi_latency_interrupt(struct pt_regs *regs)
{
	ack_APIC_irq();

	//printk("Ping-pong IPI received!\n");
#if 0
	if (my_cpu) {
		//printk("Sending IPI back to CPU 0...\n");
		// send IPI back to sender
		apic->send_IPI_mask(cpumask_of(0), POPCORN_IPI_LATENCY_VECTOR);
	} else {
		//printk("Received ping-pong; reading end timestamp...\n");
		rdtscll(tsc);
		done = 1;
	}
#endif

	return;
}

static unsigned long calculate_tsc_overhead(void)
{
	unsigned long t0, t1, overhead = ~0UL;
	int i;

	for (i = 0; i < 1000; i++) {
		rdtscll(t0);
		asm volatile("");
		rdtscll(t1);
		if (t1 - t0 < overhead)
			overhead = t1 - t0;
	}

	printk("tsc overhead is %ld\n", overhead);

	return overhead;
}

unsigned long test_ipi_pingpong(int cpu)
{
	unsigned long tsc_init, tsc_final;

	done = 0;
	rdtscll(tsc_init);
	apic->send_IPI_mask(cpumask_of(cpu), POPCORN_IPI_LATENCY_VECTOR);
	while (!done) {}
	rdtscll(tsc_final);

	return tsc_final - tsc_init;
}

unsigned long test_ipi_send_time(int cpu)
{
	unsigned long tsc_init, tsc_final;

	rdtscll(tsc_init);
	apic->send_IPI_mask(cpumask_of(cpu), POPCORN_IPI_LATENCY_VECTOR);
	rdtscll(tsc_final);

	return tsc_final - tsc_init;

}

/* Syscall for testing all this stuff */
SYSCALL_DEFINE1(popcorn_test_ipi_latency, int, cpu)
{
	int rc = 0, i;
	unsigned long result, result_min = ~0UL, result_max = 0;
	unsigned long overhead = calculate_tsc_overhead();

	printk("Reached IPI latency syscall, sending to CPU %d\n", cpu);

	done = 0;

	for (i = 0; i < NUM_TRIALS; i++) {
		result = test_ipi_send_time(cpu) - overhead;

		if (result < result_min) {
			result_min = result;
		}

		if (result > result_max) {
			result_max = result;
		}
	}

	printk("Performed %d trials, min time %ld, max time %ld\n",
			NUM_TRIALS, result_min, result_max);

	return rc;
}
