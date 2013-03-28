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
unsigned long ts1, ts2, ts3;

volatile int kmsg_done;

extern int my_cpu;

static int pcn_kmsg_test_send_single(struct pcn_kmsg_test_args __user *args)
{
	int rc = 0;
	struct pcn_kmsg_test_message msg;

	msg.hdr.type = PCN_KMSG_TYPE_TEST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.op = PCN_KMSG_TEST_SEND_SINGLE;

	pcn_kmsg_send(args->cpu, (struct pcn_kmsg_message *) &msg);

	return rc;
}

static int pcn_kmsg_test_send_pingpong(struct pcn_kmsg_test_args __user *args)
{
	int rc = 0;
	struct pcn_kmsg_test_message msg;
	unsigned long tsc_init;

	msg.hdr.type = PCN_KMSG_TYPE_TEST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.op = PCN_KMSG_TEST_SEND_PINGPONG;

	kmsg_done = 0;

	rdtscll(tsc_init);
	pcn_kmsg_send(args->cpu, (struct pcn_kmsg_message *) &msg);
	while (!kmsg_done) {}

	printk("Elapsed time (ticks): %lu\n", kmsg_tsc - tsc_init);

	args->ts1 = kmsg_tsc - tsc_init;

	return rc;
}

static unsigned long batch_send_start_tsc;

static int pcn_kmsg_test_send_batch(struct pcn_kmsg_test_args __user *args)
{
	int rc = 0;
	unsigned long i;
	struct pcn_kmsg_test_message msg;

	printk("Testing batch send, batch_size %lu\n", args->batch_size);

	msg.hdr.type = PCN_KMSG_TYPE_TEST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.op = PCN_KMSG_TEST_SEND_BATCH;
	msg.batch_size = args->batch_size;

	rdtscll(batch_send_start_tsc);

	kmsg_done = 0;

	/* send messages in series */
	for (i = 0; i < args->batch_size; i++) {
		msg.batch_seqnum = i;

		printk("Sending batch message, cpu %d, seqnum %lu\n", 
		       args->cpu, i);

		rc = pcn_kmsg_send(args->cpu, 
				   (struct pcn_kmsg_message *) &msg);

		if (rc) {
			printk("Error sending message!\n");
			return -1;
		}
	}

	/* wait for reply to last message */

	while (!kmsg_done) {}

	args->ts1 = ts1;
	args->ts2 = ts2;

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

	printk("POPCORN: syscall to test kernel messaging, to CPU %d\n", 
	       args->cpu);

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

		case PCN_KMSG_TEST_SEND_PINGPONG:
			rc = pcn_kmsg_test_send_pingpong(args);
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

static int handle_single_msg(struct pcn_kmsg_test_message *msg)
{
	printk("Received single test message from CPU %d!\n",
	       msg->hdr.from_cpu);
	return 0;
}

static int handle_pingpong_msg(struct pcn_kmsg_test_message *msg)
{
	int rc = 0;

	if (my_cpu) {

		struct pcn_kmsg_test_message reply_msg;

		reply_msg.hdr.type = PCN_KMSG_TYPE_TEST;
		reply_msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
		reply_msg.op = PCN_KMSG_TEST_SEND_PINGPONG;

		printk("Sending message back to CPU 0...\n");
		rc = pcn_kmsg_send(0, (struct pcn_kmsg_message *) &reply_msg);

		if (rc) {
			printk("Message send failed!\n");
			return -1;
		}
	} else {
		printk("Received ping-ping; reading end timestamp...\n");
		rdtscll(kmsg_tsc);
		kmsg_done = 1;
	}

	return 0;
}

unsigned long batch_start_tsc;

static int handle_batch_msg(struct pcn_kmsg_test_message *msg)
{
	int rc = 0;

	printk("%s: seqnum %lu size %lu\n", __func__, msg->batch_seqnum, 
	       msg->batch_size);

	if (msg->batch_seqnum == 0) {
		printk("Start of batch; taking initial timestamp!\n");
		rdtscll(batch_start_tsc);

	} else if (msg->batch_seqnum == (msg->batch_size - 1)) {
		/* send back reply */
		struct pcn_kmsg_test_message reply_msg;
		unsigned long batch_end_tsc;

		printk("End of batch; sending back reply!\n");
		rdtscll(batch_end_tsc);

		reply_msg.hdr.type = PCN_KMSG_TYPE_TEST;
		reply_msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
		reply_msg.op = PCN_KMSG_TEST_SEND_BATCH_RESULT;
		reply_msg.elapsed_time = batch_end_tsc - batch_start_tsc;

		rc = pcn_kmsg_send(0, (struct pcn_kmsg_message *) &reply_msg);

		if (rc) {
			printk("Message send failed!\n");
			return -1;
		}
	}
	return rc;
}

static int handle_batch_result_msg(struct pcn_kmsg_test_message *msg)
{
	int rc = 0;
	unsigned long batch_send_end_tsc;

	rdtscll(batch_send_end_tsc);

	printk("Batch result: sender elapsed RTT (ticks): %lu\n", 
	       batch_send_end_tsc - batch_send_start_tsc);
	printk("Batch result: receiver elapsed time (ticks): %lu\n", 
	       msg->elapsed_time);

	ts1 = batch_send_end_tsc - batch_send_start_tsc;
	ts2 = msg->elapsed_time;

	kmsg_done = 1;

	return rc;
}

static int pcn_kmsg_test_callback(struct pcn_kmsg_message *message)
{
	int rc = 0;

	struct pcn_kmsg_test_message *msg = 
		(struct pcn_kmsg_test_message *) message;

	printk("Reached %s, op %d!\n", __func__, msg->op);

	switch (msg->op) {
		case PCN_KMSG_TEST_SEND_SINGLE:
			rc = handle_single_msg(msg);
			break;

		case PCN_KMSG_TEST_SEND_PINGPONG:
			rc = handle_pingpong_msg(msg);
			break;

		case PCN_KMSG_TEST_SEND_BATCH:
			rc = handle_batch_msg(msg);
			break;

		case PCN_KMSG_TEST_SEND_BATCH_RESULT:
			rc = handle_batch_result_msg(msg);
			break;

		default:
			printk("Operation %d not supported!\n", msg->op);
	}
	
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

