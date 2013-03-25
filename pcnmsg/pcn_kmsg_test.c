/*
 * Tests/benchmarks for Popcorn inter-kernel messaging layer
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2013
 */

#include <linux/syscalls.h>

#include <linux/multikernel.h>
#include <linux/pcn_kmsg.h>

static int pcn_kmsg_test_long_msg(struct pcn_kmsg_test_args __user *args)
{
	int rc;
	struct pcn_kmsg_long_message lmsg;
	char *str = "This is a very long test message.  Don't be surprised if it gets corrupted; it probably will.  If it does, you're in for a lot more work, and may not get home to see your wife this weekend.  You should knock on wood before running this test.";

	lmsg.hdr.type = PCN_KMSG_TYPE_TEST;
	lmsg.hdr.prio = PCN_KMSG_PRIO_NORMAL;

	strcpy(&lmsg.payload, str);

	printk("Message to send: %s\n", &lmsg.payload);

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
			break;

		case PCN_KMSG_TEST_SEND_BATCH:
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
			printk("%s: invalid option %d\n", __func__, op);
			return -1;
	}

	return rc;
}

