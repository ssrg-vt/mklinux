/*
 * Tests/benchmarks for Popcorn inter-kernel messaging layer
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2013
 */

#include <linux/syscalls.h>

#include <linux/multikernel.h>
#include <linux/pcn_kmsg.h>
#include <linux/pcn_kmsg_test.h>

#define KMSG_TEST_VERBOSE 1

#ifdef KMSG_TEST_VERBOSE
#define TEST_PRINTK(fmt, args...) printk("%s: " fmt, __func__, ##args)
#else
#define TEST_PRINTK(...) ;
#endif

#define TEST_ERR(fmt, args...) printk("%s: ERROR: " fmt, __func__, ##args)

volatile unsigned long kmsg_tsc;
volatile int kmsg_done;

extern int my_cpu;

static int pcn_kmsg_test_send_single(struct pcn_kmsg_test_args __user *args)
{
	int rc = 0;
	struct pcn_kmsg_test_message msg;
	unsigned long tsc_init;

	msg.hdr.type = PCN_KMSG_TYPE_TEST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;

	kmsg_done = 0;

	rdtscll(tsc_init);
	pcn_kmsg_send(args->cpu, (struct pcn_kmsg_message *) &msg);
	while (!kmsg_done) {}
	
	printk("Elapsed time (ticks): %lu\n", kmsg_tsc - tsc_init);

	return rc;
}


static int pcn_kmsg_test_send_batch(struct pcn_kmsg_test_args __user *args)
{
	int rc = 0;

	return rc;
}


static int pcn_kmsg_test_long_msg(struct pcn_kmsg_test_args __user *args)
{
	int rc;
	struct pcn_kmsg_long_message lmsg;
	char *str = "This is a very long test message.  Don't be surprised if it gets corrupted; it probably will.  If it does, you're in for a lot more work, and may not get home to see your wife this weekend.  You should knock on wood before running this test.";

	lmsg.hdr.type = PCN_KMSG_TYPE_TEST_LONG;
	lmsg.hdr.prio = PCN_KMSG_PRIO_NORMAL;

	strcpy((char *) &lmsg.payload, str);

	printk("Message to send: %s\n", (char *) &lmsg.payload);

	printk("POPCORN: syscall to test kernel messaging, to CPU %d\n", args->cpu);

	rc = pcn_kmsg_send_long(args->cpu, &lmsg, strlen(str) + 5);

	printk("POPCORN: pcn_kmsg_send_long returned %d\n", rc);

	return rc;
}

static int pcn_kmsg_test_mcast_open(struct pcn_kmsg_test_args __user *args)
{
	int rc;
	pcn_kmsg_mcast_id test_id = -1;

	/* open */
	printk("%s: open\n", __func__);
	rc = pcn_kmsg_mcast_open(&test_id, args->mask);

	printk("POPCORN: pcn_kmsg_mcast_open returned %d, test_id %lu\n",
		       rc, test_id);

	return rc;
}

static int pcn_kmsg_test_mcast_send(struct pcn_kmsg_test_args __user *args)
{
	int rc;
	struct pcn_kmsg_test_message msg;

	/* send */
	printk("%s: send\n", __func__);
	msg.hdr.type = PCN_KMSG_TYPE_TEST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;

	rc = pcn_kmsg_mcast_send(args->mcast_id,
				 (struct pcn_kmsg_message *) &msg);
		printk("%s: failed to send mcast message to group %lu!\n",
		       __func__, args->mcast_id);
	return rc;
}

static int pcn_kmsg_test_mcast_close(struct pcn_kmsg_test_args __user *args)
{
	int rc;

	/* close */
	printk("%s: close\n", __func__);

	rc = pcn_kmsg_mcast_close(args->mcast_id);

	printk("%s: mcast close returned %d\n", __func__, rc);

	return rc;
}

/* Syscall for testing all this stuff */
SYSCALL_DEFINE2(popcorn_test_kmsg, enum pcn_kmsg_test_op, op,
		struct pcn_kmsg_test_args __user *, args)
{
	int rc = 0;

	printk("Reached test kmsg syscall, op %d, cpu %d\n",
	       op, args->cpu);

	switch (op) {
		case PCN_KMSG_TEST_SEND_SINGLE:
			rc = pcn_kmsg_test_send_single(args);
			break;

		case PCN_KMSG_TEST_SEND_BATCH:
			rc = pcn_kmsg_test_send_batch(args);
			break;

		case PCN_KMSG_TEST_SEND_LONG:
			rc = pcn_kmsg_test_long_msg(args);
			break;

		case PCN_KMSG_TEST_OP_MCAST_OPEN:
			rc = pcn_kmsg_test_mcast_open(args);
			break;

		case PCN_KMSG_TEST_OP_MCAST_SEND:
			rc = pcn_kmsg_test_mcast_send(args);
			break;

		case PCN_KMSG_TEST_OP_MCAST_CLOSE:
			rc = pcn_kmsg_test_mcast_close(args);
			break;
		
		default:
			TEST_ERR("invalid option %d\n", op);
			return -1;
	}

	return rc;
}


/* CALLBACKS */

static int pcn_kmsg_test_callback(struct pcn_kmsg_message *message)
{
	int rc = 0;

	printk("Reached %s!\n", __func__);

	if (my_cpu) {

		struct pcn_kmsg_test_message reply_msg;

		reply_msg.hdr.type = PCN_KMSG_TYPE_TEST;
		reply_msg.hdr.prio = PCN_KMSG_PRIO_HIGH;

		printk("Sending message back to CPU 0...\n");
		rc = pcn_kmsg_send(0, (struct pcn_kmsg_message *) &reply_msg);

		if (rc) {
			printk("Message send failed!\n");
			goto out;
		}
	} else {
		printk("Received ping-ping; reading end timestamp...\n");
		rdtscll(kmsg_tsc);
		kmsg_done = 1;
	}

out:
	pcn_kmsg_free_msg(message);

	return rc;
}

static int pcn_kmsg_test_long_callback(struct pcn_kmsg_message *message)
{
	struct pcn_kmsg_long_message *lmsg =
		(struct pcn_kmsg_long_message *) message;

	printk("Received test long message, payload: %s\n",
	       (char *) &lmsg->payload);

	pcn_kmsg_free_msg(message);

	return 0;
}


static int __init pcn_kmsg_test_init(void)
{
	int rc;

	TEST_PRINTK("Registering test callbacks!\n");

	rc = pcn_kmsg_register_callback(PCN_KMSG_TYPE_TEST,
					&pcn_kmsg_test_callback);
	if (rc) {
		TEST_ERR("Failed to register initial kmsg test callback!\n");
	}

	rc = pcn_kmsg_register_callback(PCN_KMSG_TYPE_TEST_LONG,
					&pcn_kmsg_test_long_callback);
	if (rc) {
		TEST_ERR("Failed to register initial kmsg_test_long callback!\n");
	}

	return rc;
}

late_initcall(pcn_kmsg_test_init);

