/*
 * Tests/benchmarks for Popcorn inter-kernel messaging layer
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2013
 */

#include <linux/syscalls.h>

#include <linux/multikernel.h>
#include <linux/pcn_kmsg.h>
#include <linux/pcn_kmsg_test.h>

#define KMSG_TEST_VERBOSE 0

#if KMSG_TEST_VERBOSE
#define TEST_PRINTK(fmt, args...) printk("%s: " fmt, __func__, ##args)
#else
#define TEST_PRINTK(...) ;
#endif

#define TEST_ERR(fmt, args...) printk("%s: ERROR: " fmt, __func__, ##args)

volatile unsigned long kmsg_tsc;
unsigned long ts1, ts2, ts3, ts4, ts5;

volatile int kmsg_done;

extern int my_cpu;

extern volatile unsigned long isr_ts, isr_ts_2, bh_ts, bh_ts_2;

static int pcn_kmsg_test_send_single(struct pcn_kmsg_test_args __user *args)
{
	int rc = 0;
	struct pcn_kmsg_test_message msg;
	unsigned long ts_start, ts_end;

	msg.hdr.type = PCN_KMSG_TYPE_TEST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.op = PCN_KMSG_TEST_SEND_SINGLE;

	rdtscll(ts_start);

	pcn_kmsg_send(args->cpu, (struct pcn_kmsg_message *) &msg);

	rdtscll(ts_end);

	args->send_ts = ts_end - ts_start;

	return rc;
}

extern unsigned long int_ts;

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

	TEST_PRINTK("Elapsed time (ticks): %lu\n", kmsg_tsc - tsc_init);

	args->send_ts = tsc_init;
	args->ts0 = int_ts;
	args->ts1 = ts1;
	args->ts2 = ts2;
	args->ts3 = ts3;
	args->ts4 = ts4;
	args->ts5 = ts5;
	args->rtt = kmsg_tsc;

	return rc;
}

static int pcn_kmsg_test_send_batch(struct pcn_kmsg_test_args __user *args)
{
	int rc = 0;
	unsigned long i;
	struct pcn_kmsg_test_message msg;
	unsigned long batch_send_start_tsc, batch_send_end_tsc;

	TEST_PRINTK("Testing batch send, batch_size %lu\n", args->batch_size);

	msg.hdr.type = PCN_KMSG_TYPE_TEST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.op = PCN_KMSG_TEST_SEND_BATCH;
	msg.batch_size = args->batch_size;

	rdtscll(batch_send_start_tsc);

	kmsg_done = 0;

	/* send messages in series */
	for (i = 0; i < args->batch_size; i++) {
		msg.batch_seqnum = i;

		TEST_PRINTK("Sending batch message, cpu %d, seqnum %lu\n", 
			    args->cpu, i);

		rc = pcn_kmsg_send(args->cpu, 
				   (struct pcn_kmsg_message *) &msg);

		if (rc) {
			TEST_ERR("Error sending message!\n");
			return -1;
		}
	}

	rdtscll(batch_send_end_tsc);

	/* wait for reply to last message */

	while (!kmsg_done) {}

	args->send_ts = batch_send_start_tsc;
	args->ts0 = batch_send_end_tsc;
	args->ts1 = ts1;
	args->ts2 = ts2;
	args->ts3 = ts3;
	args->rtt = kmsg_tsc;

	return rc;
}


static int pcn_kmsg_test_long_msg(struct pcn_kmsg_test_args __user *args)
{
	int rc;
	unsigned long start_ts, end_ts;
	struct pcn_kmsg_long_message lmsg;
	char *str = "This is a very long test message.  Don't be surprised if it gets corrupted; it probably will.  If it does, you're in for a lot more work, and may not get home to see your wife this weekend.  You should knock on wood before running this test.";

	lmsg.hdr.type = PCN_KMSG_TYPE_TEST_LONG;
	lmsg.hdr.prio = PCN_KMSG_PRIO_NORMAL;

	strcpy((char *) &lmsg.payload, str);

	TEST_PRINTK("Message to send: %s\n", (char *) &lmsg.payload);

	TEST_PRINTK("syscall to test kernel messaging, to CPU %d\n", 
		    args->cpu);

	rdtscll(start_ts);

	rc = pcn_kmsg_send_long(args->cpu, &lmsg, strlen(str) + 5);

	rdtscll(end_ts);

	args->send_ts = end_ts - start_ts;

	TEST_PRINTK("POPCORN: pcn_kmsg_send_long returned %d\n", rc);

	return rc;
}

static int pcn_kmsg_test_mcast_open(struct pcn_kmsg_test_args __user *args)
{
	int rc;
	pcn_kmsg_mcast_id test_id = -1;

	/* open */
	TEST_PRINTK("open\n");
	rc = pcn_kmsg_mcast_open(&test_id, args->mask);

	TEST_PRINTK("pcn_kmsg_mcast_open returned %d, test_id %lu\n",
		    rc, test_id);

	args->mcast_id = test_id;

	return rc;
}

extern unsigned long mcast_ipi_ts;

static int pcn_kmsg_test_mcast_send(struct pcn_kmsg_test_args __user *args)
{
	int rc;
	struct pcn_kmsg_test_message msg;
	unsigned long ts_start, ts_end;

	/* send */
	TEST_PRINTK("send\n");
	msg.hdr.type = PCN_KMSG_TYPE_TEST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.op = PCN_KMSG_TEST_SEND_SINGLE;

	rdtscll(ts_start);

	rc = pcn_kmsg_mcast_send(args->mcast_id,
				 (struct pcn_kmsg_message *) &msg);

	rdtscll(ts_end);

	if (rc) {
		TEST_ERR("failed to send mcast message to group %lu!\n",
			 args->mcast_id);
	}

	args->send_ts = ts_start;
	args->ts0 = mcast_ipi_ts;
	args->ts1 = ts_end;

	return rc;
}

static int pcn_kmsg_test_mcast_close(struct pcn_kmsg_test_args __user *args)
{
	int rc;

	/* close */
	TEST_PRINTK("close\n");

	rc = pcn_kmsg_mcast_close(args->mcast_id);

	TEST_PRINTK("mcast close returned %d\n", rc);

	return rc;
}

/* Syscall for testing all this stuff */
SYSCALL_DEFINE2(popcorn_test_kmsg, enum pcn_kmsg_test_op, op,
		struct pcn_kmsg_test_args __user *, args)
{
	int rc = 0;

	TEST_PRINTK("Reached test kmsg syscall, op %d, cpu %d\n",
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
	TEST_PRINTK("Received single test message from CPU %d!\n",
		    msg->hdr.from_cpu);
	return 0;
}

static int handle_pingpong_msg(struct pcn_kmsg_test_message *msg)
{
	int rc = 0;
	unsigned long handler_ts;

	rdtscll(handler_ts);

	if (my_cpu) {

		struct pcn_kmsg_test_message reply_msg;

		reply_msg.hdr.type = PCN_KMSG_TYPE_TEST;
		reply_msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
		reply_msg.op = PCN_KMSG_TEST_SEND_PINGPONG;
		reply_msg.ts1 = isr_ts;
		reply_msg.ts2 = isr_ts_2;
		reply_msg.ts3 = bh_ts;
		reply_msg.ts4 = bh_ts_2;
		reply_msg.ts5 = handler_ts;

		TEST_PRINTK("Sending message back to CPU 0...\n");
		rc = pcn_kmsg_send(0, (struct pcn_kmsg_message *) &reply_msg);

		if (rc) {
			TEST_ERR("Message send failed!\n");
			return -1;
		}

		isr_ts = isr_ts_2 = bh_ts = bh_ts_2 = 0;
	} else {
		TEST_PRINTK("Received ping-pong; reading end timestamp...\n");
		rdtscll(kmsg_tsc);
		ts1 = msg->ts1;
		ts2 = msg->ts2;
		ts3 = msg->ts3;
		ts4 = msg->ts4;
		ts5 = msg->ts5;
		kmsg_done = 1;
	}

	return 0;
}

unsigned long batch_start_tsc;

static int handle_batch_msg(struct pcn_kmsg_test_message *msg)
{
	int rc = 0;

	TEST_PRINTK("seqnum %lu size %lu\n", msg->batch_seqnum, 
		    msg->batch_size);

	if (msg->batch_seqnum == 0) {
		TEST_PRINTK("Start of batch; taking initial timestamp!\n");
		rdtscll(batch_start_tsc);

	}
	
	if (msg->batch_seqnum == (msg->batch_size - 1)) {
		/* send back reply */
		struct pcn_kmsg_test_message reply_msg;
		unsigned long batch_end_tsc;

		TEST_PRINTK("End of batch; sending back reply!\n");
		rdtscll(batch_end_tsc);

		reply_msg.hdr.type = PCN_KMSG_TYPE_TEST;
		reply_msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
		reply_msg.op = PCN_KMSG_TEST_SEND_BATCH_RESULT;
		reply_msg.ts1 = bh_ts;
		reply_msg.ts2 = bh_ts_2;
		reply_msg.ts3 = batch_end_tsc;

		isr_ts = isr_ts_2 = bh_ts = bh_ts_2 = 0;

		rc = pcn_kmsg_send(0, (struct pcn_kmsg_message *) &reply_msg);

		if (rc) {
			TEST_ERR("Message send failed!\n");
			return -1;
		}
	}
	return rc;
}

static int handle_batch_result_msg(struct pcn_kmsg_test_message *msg)
{
	int rc = 0;

	rdtscll(kmsg_tsc);

	ts1 = msg->ts1;
	ts2 = msg->ts2;
	ts3 = msg->ts3;

	kmsg_done = 1;

	return rc;
}

static int pcn_kmsg_test_callback(struct pcn_kmsg_message *message)
{
	int rc = 0;

	struct pcn_kmsg_test_message *msg = 
		(struct pcn_kmsg_test_message *) message;

	TEST_PRINTK("Reached %s, op %d!\n", __func__, msg->op);

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
			TEST_ERR("Operation %d not supported!\n", msg->op);
	}

	pcn_kmsg_free_msg(message);

	return rc;
}

static int pcn_kmsg_test_long_callback(struct pcn_kmsg_message *message)
{
	struct pcn_kmsg_long_message *lmsg =
		(struct pcn_kmsg_long_message *) message;

	TEST_PRINTK("Received test long message, payload: %s\n",
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

