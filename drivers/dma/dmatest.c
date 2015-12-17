/*
 * DMA Engine test module
 *
 * Copyright (C) 2007 Atmel Corporation
 * Copyright (C) 2013 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/freezer.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/ctype.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <misc/xgene/xgene-pktdma.h>

static unsigned int test_buf_size = 16384;
module_param(test_buf_size, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(test_buf_size, "Size of the memcpy test buffer");

/*
 * 0 FOR MEMCPY
 * 1 FOR XOR
 * 2 FOR PQ
 * 3 FOR SG
 * 4 FOR iSCSI CRC32c
 */

#define CRC32C_TYPE		4
#define MAX_CRC_BUFSIZE		(32 * 1024 - 1)
#define DEFAULT_CRC32C_SEED	0xFFFFFFFF

static unsigned int test_type = DMA_MEMCPY;
module_param(test_type, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(test_type, "type of test to run (default: MEMCPY");

static char test_channel[20];
module_param_string(channel, test_channel, sizeof(test_channel),
		S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(channel, "Bus ID of the channel to test (default: any)");

static char test_device[20];
module_param_string(device, test_device, sizeof(test_device),
		S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(device, "Bus ID of the DMA Engine to test (default: any)");

static unsigned int threads_per_chan = 1;
module_param(threads_per_chan, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(threads_per_chan,
		"Number of threads to start per channel (default: 1)");

static unsigned int max_channels = 1;
module_param(max_channels, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(max_channels,
		"Maximum number of channels to use (default: 1)");

static unsigned int iterations = 100000;
module_param(iterations, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(iterations,
		"Iterations before stopping test (default: 10000)");

#define MAX_OUTSTANDING			1024
static unsigned int outstanding = 256;
module_param(outstanding, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(outstanding,
		"Outstanding submitted operation (default: 256)");

static unsigned int xor_sources = 3;
module_param(xor_sources, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(xor_sources,
		"Number of xor source buffers (default: 3)");

static unsigned int pq_sources = 3;
module_param(pq_sources, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(pq_sources,
		"Number of p+q source buffers (default: 3)");

static int timeout = 3000;
module_param(timeout, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(timeout, "Transfer Timeout in msec (default: 3000), "
		 "Pass -1 for infinite timeout");

/* Maximum amount of mismatched bytes in buffer to print */
#define MAX_ERROR_COUNT		32

/*
 * Initialization patterns. All bytes in the source buffer has bit 7
 * set, all bytes in the destination buffer has bit 7 cleared.
 *
 * Bit 6 is set for all bytes which are to be copied by the DMA
 * engine. Bit 5 is set for all bytes which are to be overwritten by
 * the DMA engine.
 *
 * The remaining bits are the inverse of a counter which increments by
 * one for each byte address.
 */
#define PATTERN_SRC		0x80
#define PATTERN_DST		0x00
#define PATTERN_COPY		0x40
#define PATTERN_OVERWRITE	0x20
#define PATTERN_COUNT_MASK	0x1f

enum dmatest_error_type {
	DMATEST_ET_OK,
	DMATEST_ET_MAP_SRC,
	DMATEST_ET_MAP_DST,
	DMATEST_ET_PREP,
	DMATEST_ET_SUBMIT,
	DMATEST_ET_TIMEOUT,
	DMATEST_ET_DMA_ERROR,
	DMATEST_ET_DMA_IN_PROGRESS,
	DMATEST_ET_VERIFY,
	DMATEST_ET_VERIFY_BUF,
};

struct dmatest_verify_buffer {
	unsigned int	index;
	u8		expected;
	u8		actual;
};

struct dmatest_verify_result {
	unsigned int			error_count;
	struct dmatest_verify_buffer	data[MAX_ERROR_COUNT];
	u8				pattern;
	bool				is_srcbuf;
};

struct dmatest_thread_result {
	struct list_head	node;
	unsigned int		n;
	unsigned int		src_off;
	unsigned int		dst_off;
	unsigned int		len;
	enum dmatest_error_type	type;
	union {
		unsigned long			data;
		dma_cookie_t			cookie;
		enum dma_status			status;
		int				error;
		struct dmatest_verify_result	*vr;
	};
};

struct dmatest_result {
	struct list_head	node;
	char			*name;
	struct list_head	results;
};

struct dmatest_info;

#define SRC_DST_COUNT			10

struct dmatest_thread {
	struct list_head	node;
	struct dmatest_info	*info;
	struct task_struct	*task;
	struct dma_chan		*chan;
	u8			*srcs[MAX_OUTSTANDING][SRC_DST_COUNT];
	u8			*dsts[MAX_OUTSTANDING][SRC_DST_COUNT];
	dma_addr_t 		dma_srcs[MAX_OUTSTANDING][SRC_DST_COUNT];
	dma_addr_t 		dma_dsts[MAX_OUTSTANDING][SRC_DST_COUNT];
	dma_addr_t		dma_pq[MAX_OUTSTANDING][SRC_DST_COUNT];
	enum dma_transaction_type type;
	bool			done;
};

struct dmatest_chan {
	struct list_head	node;
	struct dma_chan		*chan;
	struct list_head	threads;
};

/**
 * struct dmatest_params - test parameters.
 * @buf_size:		size of the memcpy test buffer
 * @channel:		bus ID of the channel to test
 * @device:		bus ID of the DMA Engine to test
 * @threads_per_chan:	number of threads to start per channel
 * @max_channels:	maximum number of channels to use
 * @iterations:		iterations before stopping test
 * @xor_sources:	number of xor source buffers
 * @pq_sources:		number of p+q source buffers
 * @timeout:		transfer timeout in msec, -1 for infinite timeout
 */
struct dmatest_params {
	unsigned int	buf_size;
	enum dma_transaction_type test_type;
	char		channel[20];
	char		device[20];
	unsigned int	threads_per_chan;
	unsigned int	max_channels;
	unsigned int	iterations;
	unsigned int	xor_sources;
	unsigned int	pq_sources;
	int		timeout;
	unsigned int	outstanding;
};

/**
 * struct dmatest_info - test information.
 * @params:		test parameters
 * @lock:		access protection to the fields of this structure
 */
struct dmatest_info {
	/* Test parameters */
	struct dmatest_params	params;

	/* Internal state */
	struct list_head	channels;
	unsigned int		nr_channels;
	struct mutex		lock;

	/* debugfs related stuff */
	struct dentry		*root;

	/* Test results */
	struct list_head	results;
	struct mutex		results_lock;
};

static struct dmatest_info test_info;

static bool dmatest_match_channel(struct dmatest_params *params,
		struct dma_chan *chan)
{
	if (params->channel[0] == '\0')
		return true;
	return strcmp(dma_chan_name(chan), params->channel) == 0;
}

static bool dmatest_match_device(struct dmatest_params *params,
		struct dma_device *device)
{
	if (params->device[0] == '\0')
		return true;
	return strcmp(dev_name(device->dev), params->device) == 0;
}

static unsigned long dmatest_random(void)
{
	unsigned long buf;

	get_random_bytes(&buf, sizeof(buf));
	return buf;
}

static void dmatest_init_srcs(u8 **bufs, unsigned int start, unsigned int len,
		unsigned int buf_size)
{
	unsigned int i;
	u8 *buf;

	for (; (buf = *bufs); bufs++) {
		for (i = 0; i < start; i++)
			buf[i] = PATTERN_SRC | (~i & PATTERN_COUNT_MASK);
		for ( ; i < start + len; i++)
			buf[i] = PATTERN_SRC | PATTERN_COPY
				| (~i & PATTERN_COUNT_MASK);
		for ( ; i < buf_size; i++)
			buf[i] = PATTERN_SRC | (~i & PATTERN_COUNT_MASK);
		buf++;
	}
}

static void dmatest_init_dsts(u8 **bufs, unsigned int start, unsigned int len,
		unsigned int buf_size)
{
	unsigned int i;
	u8 *buf;

	for (; (buf = *bufs); bufs++) {
		for (i = 0; i < start; i++)
			buf[i] = PATTERN_DST | (~i & PATTERN_COUNT_MASK);
		for ( ; i < start + len; i++)
			buf[i] = PATTERN_DST | PATTERN_OVERWRITE
				| (~i & PATTERN_COUNT_MASK);
		for ( ; i < buf_size; i++)
			buf[i] = PATTERN_DST | (~i & PATTERN_COUNT_MASK);
	}
}

static unsigned int dmatest_verify(struct dmatest_verify_result *vr, u8 **bufs,
		unsigned int start, unsigned int end, unsigned int counter,
		u8 pattern, bool is_srcbuf)
{
	unsigned int i;
	unsigned int error_count = 0;
	u8 actual;
	u8 expected;
	u8 *buf;
	unsigned int counter_orig = counter;
	struct dmatest_verify_buffer *vb;

	for (; (buf = *bufs); bufs++) {
		counter = counter_orig;
		for (i = start; i < end; i++) {
			actual = buf[i];
			expected = pattern | (~counter & PATTERN_COUNT_MASK);
			if (actual != expected) {
				if (error_count < MAX_ERROR_COUNT && vr) {
					vb = &vr->data[error_count];
					vb->index = i;
					vb->expected = expected;
					vb->actual = actual;
				}
				error_count++;
			}
			counter++;
		}
	}

	if (error_count > MAX_ERROR_COUNT)
		pr_warning("%s: %u errors suppressed\n",
			current->comm, error_count - MAX_ERROR_COUNT);

	return error_count;
}

/* poor man's completion - we want to use wait_event_freezable() on it */
struct dmatest_done {
	bool			done;
	wait_queue_head_t	*wait;
	unsigned int 		*counter;
};

static void dmatest_callback(void *arg)
{
	struct dmatest_done *done = arg;

	if (done->counter)
		(*done->counter)++;

	done->done = true;
	wake_up_all(done->wait);
}

static inline void unmap_src(struct device *dev, dma_addr_t *addr, size_t len,
			     unsigned int count)
{
	while (count--)
		dma_unmap_single(dev, addr[count], len, DMA_TO_DEVICE);
}

static inline void unmap_dst(struct device *dev, dma_addr_t *addr, size_t len,
			     unsigned int count)
{
	while (count--)
		dma_unmap_single(dev, addr[count], len, DMA_BIDIRECTIONAL);
}

static unsigned int min_odd(unsigned int x, unsigned int y)
{
	unsigned int val = min(x, y);

	return val % 2 ? val : val - 1;
}

static char *verify_result_get_one(struct dmatest_verify_result *vr,
		unsigned int i)
{
	struct dmatest_verify_buffer *vb = &vr->data[i];
	u8 diff = vb->actual ^ vr->pattern;
	static char buf[512];
	char *msg;

	if (vr->is_srcbuf)
		msg = "srcbuf overwritten!";
	else if ((vr->pattern & PATTERN_COPY)
			&& (diff & (PATTERN_COPY | PATTERN_OVERWRITE)))
		msg = "dstbuf not copied!";
	else if (diff & PATTERN_SRC)
		msg = "dstbuf was copied!";
	else
		msg = "dstbuf mismatch!";

	snprintf(buf, sizeof(buf) - 1, "%s [0x%x] Expected %02x, got %02x", msg,
		 vb->index, vb->expected, vb->actual);

	return buf;
}

static char *thread_result_get(const char *name,
		struct dmatest_thread_result *tr)
{
	static const char * const messages[] = {
		[DMATEST_ET_OK]			= "No errors",
		[DMATEST_ET_MAP_SRC]		= "src mapping error",
		[DMATEST_ET_MAP_DST]		= "dst mapping error",
		[DMATEST_ET_PREP]		= "prep error",
		[DMATEST_ET_SUBMIT]		= "submit error",
		[DMATEST_ET_TIMEOUT]		= "test timed out",
		[DMATEST_ET_DMA_ERROR]		=
			"got completion callback (DMA_ERROR)",
		[DMATEST_ET_DMA_IN_PROGRESS]	=
			"got completion callback (DMA_IN_PROGRESS)",
		[DMATEST_ET_VERIFY]		= "errors",
		[DMATEST_ET_VERIFY_BUF]		= "verify errors",
	};
	static char buf[512];

	snprintf(buf, sizeof(buf) - 1,
		 "%s: #%u: %s with src_off=0x%x ""dst_off=0x%x len=0x%x (%lu)",
		 name, tr->n, messages[tr->type], tr->src_off, tr->dst_off,
		 tr->len, tr->data);

	return buf;
}

static int thread_result_add(struct dmatest_info *info,
		struct dmatest_result *r, enum dmatest_error_type type,
		unsigned int n, unsigned int src_off, unsigned int dst_off,
		unsigned int len, unsigned long data)
{
	struct dmatest_thread_result *tr;

	tr = kzalloc(sizeof(*tr), GFP_KERNEL);
	if (!tr)
		return -ENOMEM;

	tr->type = type;
	tr->n = n;
	tr->src_off = src_off;
	tr->dst_off = dst_off;
	tr->len = len;
	tr->data = data;

	mutex_lock(&info->results_lock);
	list_add_tail(&tr->node, &r->results);
	mutex_unlock(&info->results_lock);

	if (tr->type == DMATEST_ET_OK)
		pr_debug("%s\n", thread_result_get(r->name, tr));
	else
		pr_warn("%s\n", thread_result_get(r->name, tr));

	return 0;
}

static unsigned int verify_result_add(struct dmatest_info *info,
		struct dmatest_result *r, unsigned int n,
		unsigned int src_off, unsigned int dst_off, unsigned int len,
		u8 **bufs, int whence, unsigned int counter, u8 pattern,
		bool is_srcbuf)
{
	struct dmatest_verify_result *vr;
	unsigned int error_count;
	unsigned int buf_off = is_srcbuf ? src_off : dst_off;
	unsigned int start, end;

	if (whence < 0) {
		start = 0;
		end = buf_off;
	} else if (whence > 0) {
		start = buf_off + len;
		end = info->params.buf_size;
	} else {
		start = buf_off;
		end = buf_off + len;
	}

	vr = kmalloc(sizeof(*vr), GFP_KERNEL);
	if (!vr) {
		pr_warn("dmatest: No memory to store verify result\n");
		return dmatest_verify(NULL, bufs, start, end, counter, pattern,
				      is_srcbuf);
	}

	vr->pattern = pattern;
	vr->is_srcbuf = is_srcbuf;

	error_count = dmatest_verify(vr, bufs, start, end, counter, pattern,
				     is_srcbuf);
	if (error_count) {
		vr->error_count = error_count;
		thread_result_add(info, r, DMATEST_ET_VERIFY_BUF, n, src_off,
				  dst_off, len, (unsigned long)vr);
		return error_count;
	}

	kfree(vr);
	return 0;
}

static void result_free(struct dmatest_info *info, const char *name)
{
	struct dmatest_result *r, *_r;

	mutex_lock(&info->results_lock);
	list_for_each_entry_safe(r, _r, &info->results, node) {
		struct dmatest_thread_result *tr, *_tr;

		if (name && strcmp(r->name, name))
			continue;

		list_for_each_entry_safe(tr, _tr, &r->results, node) {
			if (tr->type == DMATEST_ET_VERIFY_BUF)
				kfree(tr->vr);
			list_del(&tr->node);
			kfree(tr);
		}

		kfree(r->name);
		list_del(&r->node);
		kfree(r);
	}

	mutex_unlock(&info->results_lock);
}

static struct dmatest_result *result_init(struct dmatest_info *info,
		const char *name)
{
	struct dmatest_result *r;

	r = kzalloc(sizeof(*r), GFP_KERNEL);
	if (r) {
		r->name = kstrdup(name, GFP_KERNEL);
		INIT_LIST_HEAD(&r->results);
		mutex_lock(&info->results_lock);
		list_add_tail(&r->node, &info->results);
		mutex_unlock(&info->results_lock);
	}
	return r;
}

int crc_passed_tests;
int crc_failed_tests;

struct crc_test_params {
	void *srcs[MAX_OUTSTANDING];
	struct scatterlist *src_sg[MAX_OUTSTANDING];
};

void xgene_crc32c_speed_test_cb(void *ctx, u32 result)
{
	crc_passed_tests++;
	
	if ((crc_passed_tests + crc_failed_tests) % outstanding == 0)
		complete(ctx);
}

int xgene_crc32c_speed_test(void *data)
{
	struct dmatest_params *params = data;
	struct completion completion;
	struct crc_test_params *test;
	struct timeval ts, te;
	unsigned long throughput;
	unsigned long elapse_nsec;
	int loop_count;
	int ret;
	int i;
	int j;
	
	printk("%s: Starting APM X-Gene iSCSI CRC32c Speed Test\n", __func__);

	loop_count = (params->iterations / params->outstanding) + 
			((params->iterations % params->outstanding) ? 1 : 0);

	test = kzalloc(sizeof(struct crc_test_params), GFP_ATOMIC);
	if (!test) {
		printk("%s: Couldn't allocate memory for test parameters\n", __func__);
		return -ENOMEM;
	}

	for (i = 0; i < params->outstanding; i++) {
		test->srcs[i] = kmalloc(params->buf_size, GFP_ATOMIC);
		if (!test->srcs[i]) {
			printk("%s: Couldn't allocate memory for srcs %d\n", __func__, i);
			ret = -ENOMEM;
			goto test_params_free;
		}
	}
	
	for (i = 0; i < params->outstanding; i++) {
		test->src_sg[i] = kmalloc(sizeof(struct scatterlist), GFP_ATOMIC);
		if (!test->src_sg[i]) { 
			printk("%s: Couldn't allocate memory for source sg %d\n", __func__, i);
			ret = -ENOMEM;
			goto srcs_free;
		}
	}

	for (i = 0; i < params->outstanding; i++)
		sg_init_one(test->src_sg[i], test->srcs[i], params->buf_size);

	ret = xgene_flyby_alloc_resources();
	if (ret) {
		printk("%s: Couldn't allocate resource\n", __func__);
		goto src_sg_free;
	}

	crc_passed_tests = 0;
	crc_failed_tests = 0;

	do_gettimeofday(&ts);

	for (j = 0; j < loop_count; j++) {
		init_completion(&completion);
		for (i = 0; i < params->outstanding; i++) {	
			ret = xgene_flyby_op(xgene_crc32c_speed_test_cb, &completion, test->src_sg[i], 
						params->buf_size, DEFAULT_CRC32C_SEED, FBY_ISCSI_CRC32C);
			if (ret != -EINPROGRESS) {
				printk("%s: Iteration %d failed\n", __func__, i);
				crc_failed_tests++;
			}
		}
		wait_for_completion_interruptible(&completion);
	}
	
	do_gettimeofday(&te);

	elapse_nsec = (te.tv_sec - ts.tv_sec) * 1000000000 +
			(te.tv_usec - ts.tv_usec) * 1000;
	/*
	* Throughput calculation by using this formula :
	* Throughput = (Total Data Size * 10 ^ 9) / nano_seconds Bytes/Seconds
	*            = (Total Data Size * 10 ^ 3) / nano_seconds MBytes/Seconds
	*/

	throughput = ((unsigned long) params->buf_size * (unsigned long) crc_passed_tests * 1000) / elapse_nsec;
	printk("APM X-Gene  iSCSI CRC32c Performance:\n");
	printk("Total No of Iterations %d\n", loop_count * params->outstanding);
	printk("Time Start %ld.%ld\n", ts.tv_sec, ts.tv_usec);
	printk("Time End %ld.%ld\n", te.tv_sec, te.tv_usec);
	printk("Total Data Size %ld KB\n", ((long) params->buf_size * (long) crc_passed_tests) / 1024);
	printk("Elapse Time %ld ns\n", elapse_nsec);
	printk("\nThroughput %ld MB/s\n\n", throughput);

	xgene_flyby_free_resources();
	ret = 0;
	printk("dma2chan0-crc32c: terminating after %u tests, %u failures (status %d)\n",
			crc_passed_tests, crc_failed_tests, ret);
	printk("Exiting APM X-Gene iSCSI CRC32c Speed Test\n");

src_sg_free:
	for (i = 0; i < params->outstanding; i++) 
		kfree(test->src_sg[i]);
srcs_free:	
	for (i = 0; i < params->outstanding; i++) 
		kfree(test->srcs[i]);	
test_params_free:
		kfree(test);

	return ret;
}

/*
 * This function repeatedly tests DMA transfers of various lengths and
 * offsets for a given operation type until it is told to exit by
 * kthread_stop(). There may be multiple threads running this function
 * in parallel for a single channel, and there may be multiple channels
 * being tested in parallel.
 *
 * Before each test, the source and destination buffer is initialized
 * with a known pattern. This pattern is different depending on
 * whether it's in an area which is supposed to be copied or
 * overwritten, and different in the source and destination buffers.
 * So if the DMA engine doesn't copy exactly what we tell it to copy,
 * we'll notice.
 */
static int dmatest_func(void *data)
{
	DECLARE_WAIT_QUEUE_HEAD_ONSTACK(done_wait);
	struct dmatest_thread	*thread = data;
	struct dmatest_done	*done = NULL;
	struct dmatest_info	*info;
	struct dmatest_params	*params;
	struct dma_chan		*chan;
	struct dma_device	*dev;
	const char		*thread_name;
	unsigned int		src_off = 0, dst_off = 0, len = 0;
	unsigned int		error_count;
	unsigned int		failed_tests = 0;
	unsigned int		total_tests = 0;
	dma_cookie_t		cookie = 0;
	enum dma_status		status;
	enum dma_ctrl_flags 	flags;
	u8			*pq_coefs = NULL;
	int			ret;
	int			src_cnt;
	int			dst_cnt;
	int			i;
	struct dmatest_result	*result;
	int			j;
	u8 			align = 0;
	struct timeval		ts;
	struct timeval		te;
	int 			outstanding_idx;
	int 			prev_outstanding_idx;
	struct dma_async_tx_descriptor **tx = NULL;

	thread_name = current->comm;
	set_freezable();
	
	smp_rmb();
	info = thread->info;
	params = &info->params;
	chan = thread->chan;
	dev = chan->device;

	if (params->test_type == CRC32C_TYPE) {
		ret = xgene_crc32c_speed_test(params);
		if (ret)
			printk("APM X-Gene iSCSI CRC32C Speed Test Failed %d\n", ret);

		thread->done = true;

		if (params->iterations > 0)
			while (!kthread_should_stop()) {
				DECLARE_WAIT_QUEUE_HEAD_ONSTACK(wait_dmatest_exit);
				interruptible_sleep_on(&wait_dmatest_exit);
			}

		return ret;
	}

	ret = -ENOMEM;

	tx = kzalloc(sizeof(struct dma_async_tx_descriptor *) *
			params->outstanding, GFP_KERNEL);
	if (!tx) {
		printk(KERN_ERR "Out of memory\n");
		return 0;
	}
	done = kzalloc(sizeof(*done) * params->outstanding, GFP_KERNEL);
	if (!done) {
		kfree(tx);
		printk(KERN_ERR "Out of memory\n");
		return 0;
	}

	if (thread->type == DMA_MEMCPY)
		src_cnt = dst_cnt = 1;
	else if (thread->type == DMA_XOR) {
		/* force odd to ensure dst = src */
		if (params->outstanding <= 1)
			src_cnt = min_odd(params->xor_sources | 1, dev->max_xor);
		else 
			src_cnt = min(params->xor_sources, (unsigned int) dev->max_xor);
		dst_cnt = 1;
	} else if (thread->type == DMA_PQ) {
		/* force odd to ensure dst = src */
		if (params->outstanding <= 1)
			src_cnt = min_odd(params->pq_sources | 1, dma_maxpq(dev, 0));
		else 
			src_cnt = min(params->pq_sources, (unsigned int) dma_maxpq(dev, 0));
		dst_cnt = 2;

		pq_coefs = kmalloc(params->pq_sources+1, GFP_KERNEL);
		if (!pq_coefs)
			goto err_thread_type;

		for (i = 0; i < src_cnt; i++)
			pq_coefs[i] = 1;
	} else
		goto err_thread_type;

	result = result_init(info, thread_name);
	if (!result)
		goto err_srcs;

	for (j = 0; j < params->outstanding; j++) {
		for (i = 0; i < src_cnt; i++) {
			thread->dma_srcs[j][i] = 0;
			thread->srcs[j][i] = kmalloc(params->buf_size, GFP_KERNEL);
			if (!thread->srcs[j][i])
				goto err_srcbuf;
		}
		thread->srcs[j][i] = NULL;
		thread->dma_srcs[j][i] = 0;

		for (i = 0; i < dst_cnt; i++) {
			thread->dma_dsts[j][i] = 0;
			thread->dma_pq[j][i] = 0;
			thread->dsts[j][i] = kmalloc(params->buf_size, GFP_KERNEL);
			if (!thread->dsts[j][i])
				goto err_dstbuf;
		}
		thread->dsts[j][i] = NULL;
		thread->dma_dsts[j][i] = 0;
		thread->dma_pq[j][i] = 0;
	}

	set_user_nice(current, 10);

	/*
	 * src buffers are freed by the DMAEngine code with dma_unmap_single()
	 * dst buffers are freed by ourselves below
	 */
	flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT
	      | DMA_COMPL_SKIP_DEST_UNMAP | DMA_COMPL_SRC_UNMAP_SINGLE;

	/* honor alignment restrictions */
	if (thread->type == DMA_MEMCPY)
		align = dev->copy_align;
	else if (thread->type == DMA_XOR)
		align = dev->xor_align;
	else if (thread->type == DMA_PQ)
		align = dev->pq_align;

	if (1 << align > params->buf_size) {
		pr_err("%u-byte buffer too small for %d-byte alignment\n",
			params->buf_size, 1 << align);
		goto err_align;
	}

	if (params->outstanding > 1) {
		len = params->buf_size;
		len = (len >> align) << align;
		if (!len)
			len = 1 << align;
		src_off = params->buf_size - len;
		dst_off = params->buf_size - len;

		src_off = (src_off >> align) << align;
		dst_off = (dst_off >> align) << align;

		for (j = 0; j < params->outstanding; j++) {
			dmatest_init_srcs(thread->srcs[j], src_off, len, params->buf_size);
			dmatest_init_dsts(thread->dsts[j], dst_off, len, params->buf_size);

			for (i = 0; i < src_cnt; i++) {
				u8 *buf = thread->srcs[j][i] + src_off;

				thread->dma_srcs[j][i] = dma_map_single(dev->dev, buf, len,
									DMA_TO_DEVICE);
				ret = dma_mapping_error(dev->dev, thread->dma_srcs[j][i]);
				if (ret) {
					thread_result_add(info, result,
							DMATEST_ET_MAP_SRC,
							total_tests, src_off, dst_off,
							len, ret);
					goto err;
				}
			}
			/* map with DMA_BIDIRECTIONAL to force writeback/invalidate */
			for (i = 0; i < dst_cnt; i++) {
				thread->dma_dsts[j][i] = dma_map_single(dev->dev,
									thread->dsts[j][i],
									params->buf_size,
									DMA_BIDIRECTIONAL);
				ret = dma_mapping_error(dev->dev, thread->dma_dsts[j][i]);
				if (ret) {
					thread_result_add(info, result,
							DMATEST_ET_MAP_DST,
							total_tests, src_off, dst_off,
							len, ret);
					goto err;
				}
			}

			done[j].done = true;
			done[j].wait = &done_wait;
			done[j].counter = &total_tests;

		}
	} else {
		done[0].done = true;
		done[0].wait = &done_wait;
	}

	do_gettimeofday(&ts);

	prev_outstanding_idx = outstanding_idx = 0;
	while (!kthread_should_stop()
		&& !(params->iterations && total_tests >= params->iterations)) {

		if (params->outstanding <= 1) {
			total_tests++;

			len = dmatest_random() % params->buf_size + 1;
			len = (len >> align) << align;
			if (!len)
				len = 1 << align;
			src_off = dmatest_random() % (params->buf_size - len + 1);
			dst_off = dmatest_random() % (params->buf_size - len + 1);

			src_off = (src_off >> align) << align;
			dst_off = (dst_off >> align) << align;

			dmatest_init_srcs(thread->srcs[0], src_off, len, params->buf_size);
			dmatest_init_dsts(thread->dsts[0], dst_off, len, params->buf_size);

			for (i = 0; i < src_cnt; i++) {
				u8 *buf = thread->srcs[0][i] + src_off;

				thread->dma_srcs[0][i] = dma_map_single(dev->dev, buf, len,
									DMA_TO_DEVICE);
				ret = dma_mapping_error(dev->dev, thread->dma_srcs[0][i]);
				if (ret) {
					unmap_src(dev->dev, thread->dma_srcs[0], len, i);
					thread_result_add(info, result,
							DMATEST_ET_MAP_SRC,
							total_tests, src_off, dst_off,
							len, ret);
					failed_tests++;
					continue;
				}
			}
			/* map with DMA_BIDIRECTIONAL to force writeback/invalidate */
			for (i = 0; i < dst_cnt; i++) {
				thread->dma_dsts[0][i] = dma_map_single(dev->dev, thread->dsts[0][i],
							params->buf_size,
							DMA_BIDIRECTIONAL);
				ret = dma_mapping_error(dev->dev, thread->dma_dsts[0][i]);
				if (ret) {
					unmap_src(dev->dev, thread->dma_srcs[0], len, src_cnt);
					unmap_dst(dev->dev, thread->dma_dsts[0], params->buf_size,
						i);
					thread_result_add(info, result,
							DMATEST_ET_MAP_DST,
							total_tests, src_off, dst_off,
							len, ret);
					failed_tests++;
					continue;
				}
			}
			done[0].done = true;
		}

		for (j = 0; j < params->outstanding; j++) {
			if (!done[outstanding_idx].done)
				break;
			if (thread->type == DMA_MEMCPY)
				tx[outstanding_idx] = dev->device_prep_dma_memcpy(chan,
						thread->dma_dsts[outstanding_idx][0] + dst_off,
						thread->dma_srcs[outstanding_idx][0], len,
						flags);
			else if (thread->type == DMA_XOR)
				tx[outstanding_idx] = dev->device_prep_dma_xor(chan,
						thread->dma_dsts[outstanding_idx][0] + dst_off,
						thread->dma_srcs[outstanding_idx], src_cnt,
						len, flags);
			else if (thread->type == DMA_PQ) {
				for (i = 0; i < dst_cnt; i++)
					thread->dma_pq[outstanding_idx][i] = thread->dma_dsts[outstanding_idx][i] + dst_off;
				tx[outstanding_idx] = dev->device_prep_dma_pq(chan, thread->dma_pq[j],
						thread->dma_srcs[outstanding_idx],
						src_cnt, pq_coefs,
						len, flags);
			}

			if (!tx[outstanding_idx]) {
				if (params->outstanding <= 1) {
					unmap_src(dev->dev, thread->dma_srcs[0], len, src_cnt);
					unmap_dst(dev->dev, thread->dma_dsts[0], params->buf_size,
						  dst_cnt);
				} else {
					total_tests++;
				}
				thread_result_add(info, result, DMATEST_ET_PREP,
						total_tests, src_off, dst_off,
						len, 0);
				msleep(100);
				failed_tests++;
				continue;
			}

			done[outstanding_idx].done = false;
			tx[outstanding_idx]->callback = dmatest_callback;
			tx[outstanding_idx]->callback_param = &done[outstanding_idx];
			cookie = tx[outstanding_idx]->tx_submit(tx[outstanding_idx]);

			if (dma_submit_error(cookie)) {
				thread_result_add(info, result, DMATEST_ET_SUBMIT,
						total_tests, src_off, dst_off,
						len, cookie);
				msleep(100);
				if (params->outstanding > 1)
					total_tests++;
				failed_tests++;
				continue;
			}
			prev_outstanding_idx = outstanding_idx;
			if (++outstanding_idx >= params->outstanding)
				outstanding_idx = 0;
		}

		dma_async_issue_pending(chan);

		wait_event_freezable_timeout(done_wait, done[prev_outstanding_idx].done,
					     msecs_to_jiffies(params->timeout));

		if (params->outstanding <= 1) {
			status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
			if (!done[prev_outstanding_idx].done) {
				/*
				* We're leaving the timed out dma operation with
				* dangling pointer to done_wait.  To make this
				* correct, we'll need to allocate wait_done for
				* each test iteration and perform "who's gonna
				* free it this time?" dancing.  For now, just
				* leave it dangling.
				*/
				thread_result_add(info, result, DMATEST_ET_TIMEOUT,
						total_tests, src_off, dst_off,
						len, 0);
				failed_tests++;
				continue;
			} else if (status != DMA_SUCCESS) {
				enum dmatest_error_type type = (status == DMA_ERROR) ?
					DMATEST_ET_DMA_ERROR : DMATEST_ET_DMA_IN_PROGRESS;
				thread_result_add(info, result, type,
						total_tests, src_off, dst_off,
						len, status);
				failed_tests++;
				continue;
			}

			/* Unmap by myself (see DMA_COMPL_SKIP_DEST_UNMAP above) */
			unmap_dst(dev->dev, thread->dma_dsts[0], params->buf_size, dst_cnt);

			error_count = 0;

			pr_debug("%s: verifying source buffer...\n", thread_name);
			error_count += verify_result_add(info, result, total_tests,
					src_off, dst_off, len, thread->srcs[0], -1,
					0, PATTERN_SRC, true);
			error_count += verify_result_add(info, result, total_tests,
					src_off, dst_off, len, thread->srcs[0], 0,
					src_off, PATTERN_SRC | PATTERN_COPY, true);
			error_count += verify_result_add(info, result, total_tests,
					src_off, dst_off, len, thread->srcs[0], 1,
					src_off + len, PATTERN_SRC, true);

			pr_debug("%s: verifying dest buffer...\n", thread_name);
			error_count += verify_result_add(info, result, total_tests,
					src_off, dst_off, len, thread->dsts[0], -1,
					0, PATTERN_DST, false);
			error_count += verify_result_add(info, result, total_tests,
					src_off, dst_off, len, thread->dsts[0], 0,
					src_off, PATTERN_SRC | PATTERN_COPY, false);
			error_count += verify_result_add(info, result, total_tests,
					src_off, dst_off, len, thread->dsts[0], 1,
					dst_off + len, PATTERN_DST, false);

			if (error_count) {
				thread_result_add(info, result, DMATEST_ET_VERIFY,
						total_tests, src_off, dst_off,
						len, error_count);
				failed_tests++;
			} else {
				thread_result_add(info, result, DMATEST_ET_OK,
						total_tests, src_off, dst_off,
						len, 0);
			}
		}
	}

	if (params->outstanding > 1) {
		unsigned long throughput;
		unsigned long elapse_nsec;

		do_gettimeofday(&te);
		elapse_nsec = (te.tv_sec - ts.tv_sec) * 1000000000 +
			      (te.tv_usec - ts.tv_usec) * 1000;
		/*
		* Throughput calculation by using this formula :
		* Throughput = (Total Data Size * 10 ^ 9) / nano_seconds Bytes/Seconds
		*            = (Total Data Size * 10 ^ 3) / nano_seconds MBytes/Seconds
		*/

		throughput = ((unsigned long) len *
			      (unsigned long) total_tests * 1000) /
			     elapse_nsec;
		if (thread->type == DMA_MEMCPY)
			printk("%s: Memory Copy Performance:\n",
				dma_chan_name(chan));
		else if (thread->type == DMA_XOR)
			printk("%s: XOR Performance:\n", dma_chan_name(chan));
		else
			printk("%s: PQ Performance:\n", dma_chan_name(chan));
		printk("%s: Time Start %ld.%ld\n",
			dma_chan_name(chan), ts.tv_sec, ts.tv_usec);
		printk("%s:   Time End %ld.%ld\n",
			dma_chan_name(chan), te.tv_sec, te.tv_usec);
		printk("%s: Total Data Size %ld KB\n", dma_chan_name(chan),
			((long) len * (long) total_tests) / 1024);
		printk("%s: Elapse Time %ld ns\n", dma_chan_name(chan),
			elapse_nsec);
		printk("%s: Throughput %ld MB/s\n", dma_chan_name(chan),
			throughput);
	}

err:
err_align:
err_dstbuf:
err_srcbuf:
err_srcs:
	if (tx)
		kfree(tx);
	if (done)
		kfree(done);
	if (pq_coefs)
		kfree(pq_coefs);

	for (j = 0; j < params->outstanding; j++) {
		if (params->outstanding > 1) {
			unmap_dst(dev->dev, thread->dma_dsts[j], params->buf_size, dst_cnt);
			unmap_src(dev->dev, thread->dma_srcs[j], params->buf_size, src_cnt);
		}
		for (i = 0; thread->dsts[j][i]; i++)
			kfree(thread->dsts[j][i]);
		for (i = 0; thread->srcs[j][i]; i++)
			kfree(thread->srcs[j][i]);
	}

err_thread_type:
	pr_notice("%s: terminating after %u tests, %u failures (status %d)\n",
			thread_name, total_tests, failed_tests, ret);

	/* terminate all transfers on specified channels */
	if (ret)
		dmaengine_terminate_all(chan);

	thread->done = true;

	if (params->iterations > 0)
		while (!kthread_should_stop()) {
			DECLARE_WAIT_QUEUE_HEAD_ONSTACK(wait_dmatest_exit);
			interruptible_sleep_on(&wait_dmatest_exit);
		}

	return ret;
}

static void dmatest_cleanup_channel(struct dmatest_chan *dtc)
{
	struct dmatest_thread	*thread;
	struct dmatest_thread	*_thread;
	int			ret;

	list_for_each_entry_safe(thread, _thread, &dtc->threads, node) {
		ret = kthread_stop(thread->task);
		pr_debug("dmatest: thread %s exited with status %d\n",
				thread->task->comm, ret);
		list_del(&thread->node);
		kfree(thread);
	}

	/* terminate all transfers on specified channels */
	dmaengine_terminate_all(dtc->chan);

	kfree(dtc);
}

static int dmatest_add_threads(struct dmatest_info *info,
		struct dmatest_chan *dtc, enum dma_transaction_type type)
{
	struct dmatest_params *params = &info->params;
	struct dmatest_thread *thread;
	struct dma_chan *chan = dtc->chan;
	char *op;
	unsigned int i;

	if (type == DMA_MEMCPY)
		op = "copy";
	else if (type == DMA_XOR)
		op = "xor";
	else if (type == DMA_PQ)
		op = "pq";
	else
		return -EINVAL;

	for (i = 0; i < params->threads_per_chan; i++) {
		thread = kzalloc(sizeof(struct dmatest_thread), GFP_KERNEL);
		if (!thread) {
			pr_warning("dmatest: No memory for %s-%s%u\n",
				   dma_chan_name(chan), op, i);

			break;
		}
		thread->info = info;
		thread->chan = dtc->chan;
		thread->type = type;
		smp_wmb();
		thread->task = kthread_run(dmatest_func, thread, "%s-%s%u",
				dma_chan_name(chan), op, i);
		if (IS_ERR(thread->task)) {
			pr_warning("dmatest: Failed to run thread %s-%s%u\n",
					dma_chan_name(chan), op, i);
			kfree(thread);
			break;
		}

		/* srcbuf and dstbuf are allocated by the thread itself */

		list_add_tail(&thread->node, &dtc->threads);
	}

	return i;
}

static int dmatest_add_channel(struct dmatest_info *info,
		struct dma_chan *chan)
{
	struct dmatest_chan	*dtc;
	struct dma_device	*dma_dev = chan->device;
	unsigned int		thread_count = 0;
	int cnt;
	struct dmatest_params *params = &info->params;

	dtc = kmalloc(sizeof(struct dmatest_chan), GFP_KERNEL);
	if (!dtc) {
		pr_warning("dmatest: No memory for %s\n", dma_chan_name(chan));
		return -ENOMEM;
	}

	dtc->chan = chan;
	INIT_LIST_HEAD(&dtc->threads);

	if ((params->test_type == DMA_MEMCPY || params->test_type == CRC32C_TYPE) && dma_has_cap(DMA_MEMCPY, dma_dev->cap_mask)) {
		cnt = dmatest_add_threads(info, dtc, DMA_MEMCPY);
		thread_count += cnt > 0 ? cnt : 0;
	}
	if (params->test_type == DMA_XOR && dma_has_cap(DMA_XOR, dma_dev->cap_mask)) {
		cnt = dmatest_add_threads(info, dtc, DMA_XOR);
		thread_count += cnt > 0 ? cnt : 0;
	}
	if (params->test_type == DMA_PQ && dma_has_cap(DMA_PQ, dma_dev->cap_mask)) {
		cnt = dmatest_add_threads(info, dtc, DMA_PQ);
		thread_count += cnt > 0 ? cnt : 0;
	}

	pr_info("dmatest: Started %u threads using %s\n",
		thread_count, dma_chan_name(chan));

	list_add_tail(&dtc->node, &info->channels);
	info->nr_channels++;

	return 0;
}

static bool filter(struct dma_chan *chan, void *param)
{
	struct dmatest_params *params = param;

	if (!dmatest_match_channel(params, chan) ||
	    !dmatest_match_device(params, chan->device))
		return false;
	else
		return true;
}

static int __run_threaded_test(struct dmatest_info *info)
{
	dma_cap_mask_t mask;
	struct dma_chan *chan;
	struct dmatest_params *params = &info->params;
	int err = 0;

	dma_cap_zero(mask);
	if (params->test_type == CRC32C_TYPE)
		dma_cap_set(DMA_MEMCPY, mask);
	else 
		dma_cap_set(params->test_type, mask);
	for (;;) {
		chan = dma_request_channel(mask, filter, params);
		if (chan) {
			err = dmatest_add_channel(info, chan);
			if (err) {
				dma_release_channel(chan);
				break; /* add_channel failed, punt */
			}
		} else
			break; /* no more channels available */
		if (params->max_channels &&
		    info->nr_channels >= params->max_channels)
			break; /* we have all we need */
	}
	return err;
}

#ifndef MODULE
static int run_threaded_test(struct dmatest_info *info)
{
	int ret;

	mutex_lock(&info->lock);
	ret = __run_threaded_test(info);
	mutex_unlock(&info->lock);
	return ret;
}
#endif

static void __stop_threaded_test(struct dmatest_info *info)
{
	struct dmatest_chan *dtc, *_dtc;
	struct dma_chan *chan;

	list_for_each_entry_safe(dtc, _dtc, &info->channels, node) {
		list_del(&dtc->node);
		chan = dtc->chan;
		dmatest_cleanup_channel(dtc);
		pr_debug("dmatest: dropped channel %s\n", dma_chan_name(chan));
		dma_release_channel(chan);
	}

	info->nr_channels = 0;
}

static void stop_threaded_test(struct dmatest_info *info)
{
	mutex_lock(&info->lock);
	__stop_threaded_test(info);
	mutex_unlock(&info->lock);
}

static int __restart_threaded_test(struct dmatest_info *info, bool run)
{
	struct dmatest_params *params = &info->params;

	/* Stop any running test first */
	__stop_threaded_test(info);

	if (run == false)
		return 0;

	/* Clear results from previous run */
	result_free(info, NULL);

	outstanding = (outstanding > MAX_OUTSTANDING) ? MAX_OUTSTANDING :
							outstanding;
	if (test_type == CRC32C_TYPE) {
		test_buf_size = (test_buf_size > MAX_CRC_BUFSIZE) ? 
					MAX_CRC_BUFSIZE : test_buf_size;
	}
	/* Copy test parameters */
	params->buf_size = test_buf_size;
	params->test_type = test_type;
	strlcpy(params->channel, strim(test_channel), sizeof(params->channel));
	strlcpy(params->device, strim(test_device), sizeof(params->device));
	params->threads_per_chan = threads_per_chan;
	params->max_channels = max_channels;
	params->iterations = iterations;
	params->xor_sources = xor_sources;
	params->pq_sources = pq_sources;
	params->timeout = timeout;
	params->outstanding = outstanding;

	/* Run test with new parameters */
	return __run_threaded_test(info);
}

static bool __is_threaded_test_run(struct dmatest_info *info)
{
	struct dmatest_chan *dtc;

	list_for_each_entry(dtc, &info->channels, node) {
		struct dmatest_thread *thread;

		list_for_each_entry(thread, &dtc->threads, node) {
			if (!thread->done)
				return true;
		}
	}

	return false;
}

static ssize_t dtf_read_run(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct dmatest_info *info = file->private_data;
	char buf[3];

	mutex_lock(&info->lock);

	if (__is_threaded_test_run(info)) {
		buf[0] = 'Y';
	} else {
		__stop_threaded_test(info);
		buf[0] = 'N';
	}

	mutex_unlock(&info->lock);
	buf[1] = '\n';
	buf[2] = 0x00;
	return simple_read_from_buffer(user_buf, count, ppos, buf, 2);
}

static ssize_t dtf_write_run(struct file *file, const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct dmatest_info *info = file->private_data;
	char buf[16];
	bool bv;
	int ret = 0;

	if (copy_from_user(buf, user_buf, min(count, (sizeof(buf) - 1))))
		return -EFAULT;

	if (strtobool(buf, &bv) == 0) {
		mutex_lock(&info->lock);

		if (__is_threaded_test_run(info))
			ret = -EBUSY;
		else
			ret = __restart_threaded_test(info, bv);

		mutex_unlock(&info->lock);
	}

	return ret ? ret : count;
}

static const struct file_operations dtf_run_fops = {
	.read	= dtf_read_run,
	.write	= dtf_write_run,
	.open	= simple_open,
	.llseek	= default_llseek,
};

static int dtf_results_show(struct seq_file *sf, void *data)
{
	struct dmatest_info *info = sf->private;
	struct dmatest_result *result;
	struct dmatest_thread_result *tr;
	unsigned int i;

	mutex_lock(&info->results_lock);
	list_for_each_entry(result, &info->results, node) {
		list_for_each_entry(tr, &result->results, node) {
			seq_printf(sf, "%s\n",
				thread_result_get(result->name, tr));
			if (tr->type == DMATEST_ET_VERIFY_BUF) {
				for (i = 0; i < tr->vr->error_count; i++) {
					seq_printf(sf, "\t%s\n",
						verify_result_get_one(tr->vr, i));
				}
			}
		}
	}

	mutex_unlock(&info->results_lock);
	return 0;
}

static int dtf_results_open(struct inode *inode, struct file *file)
{
	return single_open(file, dtf_results_show, inode->i_private);
}

static const struct file_operations dtf_results_fops = {
	.open		= dtf_results_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int dmatest_register_dbgfs(struct dmatest_info *info)
{
	struct dentry *d;

	d = debugfs_create_dir("dmatest", NULL);
	if (IS_ERR(d))
		return PTR_ERR(d);
	if (!d)
		goto err_root;

	info->root = d;

	/* Run or stop threaded test */
	debugfs_create_file("run", S_IWUSR | S_IRUGO, info->root, info,
			    &dtf_run_fops);

	/* Results of test in progress */
	debugfs_create_file("results", S_IRUGO, info->root, info,
			    &dtf_results_fops);

	return 0;

err_root:
	pr_err("dmatest: Failed to initialize debugfs\n");
	return -ENOMEM;
}

static int __init dmatest_init(void)
{
	struct dmatest_info *info = &test_info;
	int ret;

	memset(info, 0, sizeof(*info));

	mutex_init(&info->lock);
	INIT_LIST_HEAD(&info->channels);

	mutex_init(&info->results_lock);
	INIT_LIST_HEAD(&info->results);

	ret = dmatest_register_dbgfs(info);
	if (ret)
		return ret;

#ifdef MODULE
	return 0;
#else
	return run_threaded_test(info);
#endif
}
/* when compiled-in wait for drivers to load first */
late_initcall(dmatest_init);

static void __exit dmatest_exit(void)
{
	struct dmatest_info *info = &test_info;

	debugfs_remove_recursive(info->root);
	stop_threaded_test(info);
	result_free(info, NULL);
}
module_exit(dmatest_exit);

MODULE_AUTHOR("Haavard Skinnemoen (Atmel)");
MODULE_LICENSE("GPL v2");
