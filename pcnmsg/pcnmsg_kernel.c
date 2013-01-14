/*
 * Inter-kernel messaging support for Popcorn
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2013
 */

#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/smp.h>

#include <asm/system.h>
#include <asm/apic.h>
#include <asm/hardirq.h>

void smp_popcorn_kmsg_interrupt(struct pt_regs *regs)
{
	ack_APIC_irq();

	printk("Reached Popcorn KMSG handler!\n");

	inc_irq_stat(irq_popcorn_kmsg_count);

	irq_enter();

	/* copy message to queue; handle in bottom half */

	irq_exit();
}
