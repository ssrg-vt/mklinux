/*
 * ipmi_raw_sm.c
 *
 * State machine for handling IPMI RAW interfaces.
 *
 * Author: MontaVista Software, Inc.
 *         Corey Minyard <minyard@mvista.com>
 *         source@mvista.com
 *
 * Copyright 2002 MontaVista Software Inc.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * This state machine is taken from the state machine in the IPMI spec,
 * pretty much verbatim.  If you have questions about the states, see
 * that document.
 */

#include <linux/kernel.h> /* For printk. */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <linux/jiffies.h>
#include <linux/ipmi_msgdefs.h>		/* for completion codes */
#include "ipmi_si_sm.h"
#include "apm_ipmi_request.h"
#include "apm_ipmi_sdr.h"
/* raw_debug is a bit-field
 *	RAW_DEBUG_ENABLE -	turned on for now
 *	RAW_DEBUG_MSG    -	commands and their responses
 *	RAW_DEBUG_STATES -	state machine
 */
#define RAW_DEBUG_STATES	4
#define RAW_DEBUG_MSG		2
#define	RAW_DEBUG_ENABLE	1

static int raw_debug;
module_param(raw_debug, int, 0644);
MODULE_PARM_DESC(raw_debug, "debug bitmask, 1=enable, 2=messages, 4=states");

static unsigned char g_state=0;
static unsigned char g_cmd;
static unsigned char g_data[IPMI_MAX_MSG_LENGTH];

/* The states the RAW driver may be in. */
enum raw_states {
	/* The RAW interface is currently doing nothing. */
	RAW_IDLE,

	/*
	 * We are starting an operation.  The data is in the output
	 * buffer, but nothing has been done to the interface yet.  This
	 * was added to the state machine in the spec to wait for the
	 * initial IBF.
	 */
	RAW_START_OP,

	/* We have written a write cmd to the interface. */
	RAW_WAIT_WRITE_START,

	/* We are writing bytes to the interface. */
	RAW_WAIT_WRITE,

	/*
	 * We have written the write end cmd to the interface, and
	 * still need to write the last byte.
	 */
	RAW_WAIT_WRITE_END,

	/* We are waiting to read data from the interface. */
	RAW_WAIT_READ,

	/*
	 * State to transition to the error handler, this was added to
	 * the state machine in the spec to be sure IBF was there.
	 */
	RAW_ERROR0,

	/*
	 * First stage error handler, wait for the interface to
	 * respond.
	 */
	RAW_ERROR1,

	/*
	 * The abort cmd has been written, wait for the interface to
	 * respond.
	 */
	RAW_ERROR2,

	/*
	 * We wrote some data to the interface, wait for it to switch
	 * to read mode.
	 */
	RAW_ERROR3,

	/* The hardware failed to follow the state machine. */
	RAW_HOSED
};

#define MAX_RAW_READ_SIZE IPMI_MAX_MSG_LENGTH
#define MAX_RAW_WRITE_SIZE IPMI_MAX_MSG_LENGTH

/* Timeouts in microseconds. */
#define IBF_RETRY_TIMEOUT 1000000
#define OBF_RETRY_TIMEOUT 1000000
#define MAX_ERROR_RETRIES 10
#define ERROR0_OBF_WAIT_JIFFIES (2*HZ)

struct si_sm_data {
	enum raw_states  state;
	struct si_sm_io *io;
	unsigned char    rx_data[MAX_RAW_WRITE_SIZE];
	int 			 rx_len;
	int              rx_pos;
	int              write_count;
	int              orig_write_count;
	unsigned char    tx_data[MAX_RAW_READ_SIZE];
	int              tx_pos;
	int	         truncated;

	unsigned int  error_retries;
	long          ibf_timeout;
	long          obf_timeout;
	unsigned long  error0_timeout;
};

static unsigned int init_raw_data(struct si_sm_data *raw,
				  struct si_sm_io *io)
{
	raw->state = RAW_IDLE;
	raw->io = io;
	raw->rx_pos = 0;
	raw->write_count = 0;
	raw->orig_write_count = 0;
	raw->tx_pos = 0;
	raw->error_retries = 0;
	raw->truncated = 0;
	raw->ibf_timeout = IBF_RETRY_TIMEOUT;
	raw->obf_timeout = OBF_RETRY_TIMEOUT;
	memset(g_data,0,MAX_RAW_READ_SIZE);
	/* Reserve 2 I/O bytes. */
	//ipmi_oem_sdr_repository_initialize();
	//ipmi_compact_sdr_repository_initialize();
	ipmi_sdr_repository_initialize();

	return 2;
}

static inline unsigned char read_status(struct si_sm_data *raw)
{
	//printk("%s: addr=0x%x value=0x%x\n",__func__,
	//		&(raw->io->addr)+1,g_state);
	return g_state;
	/*
	 * return raw->io->inputb(raw->io, 1);
	 */
}

static inline unsigned char tx_data(struct si_sm_data *raw)
{

	//printk("%s: addr=0x%x value=0x%x\n",__func__,
	//		&(raw->io->addr),g_data);
	int len;
	memcpy(raw->tx_data,g_data,MAX_RAW_READ_SIZE);
	memset(g_data,0,MAX_RAW_READ_SIZE);
	len = raw->tx_pos;
	raw->tx_pos = 0;
	return len;
	/*return raw->io->inputb(raw->io, 0);*/
}

static inline void write_cmd(struct si_sm_data *raw, unsigned char data)
{

	//printk("%s: addr=0x%x,data=0x%X\n",__func__,(raw->io->addr)+1,data);
	g_cmd = data;
	/*raw->io->outputb(raw->io, 1, data);*/
}
static inline void rx_data(struct si_sm_data *raw, unsigned char data)
{

	//printk("%s: data=0x%X\n",__func__,data);
	//g_data[raw->rx_pos]=data;
	/*raw->io->outputb(raw->io, 0, data);*/
}

/* Control codes. */
#define RAW_GET_STATUS_ABORT	0x60
#define RAW_WRITE_START		0x61
#define RAW_WRITE_END		0x62
#define RAW_READ_BYTE		0x68

/* Status bits. */
#define GET_STATUS_STATE(status) (((status) >> 6) & 0x03)
#define RAW_IDLE_STATE	0
#define RAW_READ_STATE	1
#define RAW_WRITE_STATE	2
#define RAW_ERROR_STATE	3
#define GET_STATUS_ATN(status) ((status) & 0x04)
#define GET_STATUS_IBF(status) ((status) & 0x02)
#define GET_STATUS_OBF(status) ((status) & 0x01)


static inline void write_next_byte(struct si_sm_data *raw)
{
	rx_data(raw, raw->rx_data[raw->rx_pos]);
	(raw->rx_pos)++;
	(raw->write_count)--;
}

static inline void start_error_recovery(struct si_sm_data *raw, char *reason)
{
	(raw->error_retries)++;
	if (raw->error_retries > MAX_ERROR_RETRIES) {
		if (raw_debug & RAW_DEBUG_ENABLE)
			printk(KERN_DEBUG "ipmi_raw_sm: raw hosed: %s\n",
			       reason);
		raw->state = RAW_HOSED;
	} else {
		raw->error0_timeout = jiffies + ERROR0_OBF_WAIT_JIFFIES;
		raw->state = RAW_ERROR0;
	}
}

static inline void read_next_byte(struct si_sm_data *raw)
{
	if (raw->tx_pos >= MAX_RAW_READ_SIZE) {
		/* Throw the data away and mark it truncated. */
		tx_data(raw);
		raw->truncated = 1;
	} else {
		raw->tx_data[raw->tx_pos] = tx_data(raw);
		(raw->tx_pos)++;
	}
	rx_data(raw, RAW_READ_BYTE);
}

static inline int check_ibf(struct si_sm_data *raw, unsigned char status,
			    long time)
{
	if (GET_STATUS_IBF(status)) {
		raw->ibf_timeout -= time;
		if (raw->ibf_timeout < 0) {
			start_error_recovery(raw, "IBF not ready in time");
			raw->ibf_timeout = IBF_RETRY_TIMEOUT;
			return 1;
		}
		return 0;
	}
	raw->ibf_timeout = IBF_RETRY_TIMEOUT;
	return 1;
}

static inline int check_obf(struct si_sm_data *raw, unsigned char status,
			    long time)
{
	if (!GET_STATUS_OBF(status)) {
		raw->obf_timeout -= time;
		if (raw->obf_timeout < 0) {
		    start_error_recovery(raw, "OBF not ready in time");
		    return 1;
		}
		return 0;
	}
	raw->obf_timeout = OBF_RETRY_TIMEOUT;
	return 1;
}

static void clear_obf(struct si_sm_data *raw, unsigned char status)
{
	if (GET_STATUS_OBF(status))
		tx_data(raw);
}

static void restart_raw_transaction(struct si_sm_data *raw)
{
	raw->write_count = raw->orig_write_count;
	raw->rx_pos = 0;
	raw->tx_pos = 0;
	raw->state = RAW_WAIT_WRITE_START;
	raw->ibf_timeout = IBF_RETRY_TIMEOUT;
	raw->obf_timeout = OBF_RETRY_TIMEOUT;
	write_cmd(raw, RAW_WRITE_START);
}

static void 
do_process_ipmi_request(struct si_sm_data *raw)
{
	REQ_MSG msg;

	memset(&msg,0,sizeof(REQ_MSG));
	msg.net_fn = raw->rx_data[0] >> 2;
	msg.cmd_code = raw->rx_data[1];
	msg.data_len = raw->rx_len - 2;

	if (msg.data_len > 0) {
		memcpy(((unsigned char *)msg.data),((unsigned char *)(raw->rx_data) + 2),msg.data_len);
	}

	if (raw_debug & RAW_DEBUG_MSG) {
		printk("%s,netfn=0x%x,cmd=0x%x,req_len=%d\n",__func__,
			msg.net_fn,msg.cmd_code,raw->rx_len);	
	}

	raw->tx_pos = ipmi_request_dispatcher(&msg,raw->tx_data);
}
static int start_raw_transaction(struct si_sm_data *raw, unsigned char *data,
				 unsigned int size)
{
	unsigned int i;

	if (size < 2)
		return IPMI_REQ_LEN_INVALID_ERR;
	if (size > MAX_RAW_WRITE_SIZE)
		return IPMI_REQ_LEN_EXCEEDED_ERR;

	if ((raw->state != RAW_IDLE) && (raw->state != RAW_HOSED))
		return IPMI_NOT_IN_MY_STATE_ERR;

	if (raw_debug & RAW_DEBUG_MSG) {
		printk(KERN_DEBUG "start_raw_transaction -");
		for (i = 0; i < size; i++)
			printk(" %02x", (unsigned char) (data [i]));
		printk(" size=%d\n",size);
	}
	memset(raw->rx_data,0,MAX_RAW_WRITE_SIZE);
	raw->error_retries = 0;
	memcpy(raw->rx_data, data, size);
	raw->write_count = size;
	raw->orig_write_count = size;
	raw->rx_len = size;
	raw->rx_pos = 0;
	raw->tx_pos = 0;
	raw->state = RAW_START_OP;
	raw->ibf_timeout = IBF_RETRY_TIMEOUT;
	raw->obf_timeout = OBF_RETRY_TIMEOUT;
	g_state = RAW_IDLE_STATE;
	return 0;
}

static int get_raw_result(struct si_sm_data *raw, unsigned char *data,
			  unsigned int length)
{

	if (length < raw->tx_pos) {
		raw->tx_pos = length;
		raw->truncated = 1;
	}

	memcpy(data, raw->tx_data, raw->tx_pos);

	if ((length >= 3) && (raw->tx_pos < 3)) {
		/* Guarantee that we return at least 3 bytes, with an
		   error in the third byte if it is too short. */
		data[2] = IPMI_ERR_UNSPECIFIED;
		raw->tx_pos = 3;
	}
	if (raw->truncated) {
		/*
		 * Report a truncated error.  We might overwrite
		 * another error, but that's too bad, the user needs
		 * to know it was truncated.
		 */
		data[2] = IPMI_ERR_MSG_TRUNCATED;
		raw->truncated = 0;
	}

	//printk("%s:data_length=%d\n",__func__,raw->tx_pos);
	return raw->tx_pos;
}

/*
 * This implements the state machine defined in the IPMI manual, see
 * that for details on how this works.  Divide that flowchart into
 * sections delimited by "Wait for IBF" and this will become clear.
 */
static enum si_sm_result raw_event(struct si_sm_data *raw, long time)
{
	unsigned char status;
	unsigned char state;

	status = read_status(raw);

	/*if (raw_debug & RAW_DEBUG_STATES)*/
	if(raw->state != RAW_IDLE) {
		//printk("RAW: State = %d, %x\n", raw->state, status);
	}

	/* All states wait for ibf, so just do it here. */
	/*
	if (!check_ibf(raw, status, time))
		return SI_SM_CALL_WITH_DELAY;
	*/
	/* Just about everything looks at the RAW state, so grab that, too. */
	/*state = GET_STATUS_STATE(status);*/
	state = status;
	switch (raw->state) {
	case RAW_IDLE:
		/* If there's and interrupt source, turn it off. */
		clear_obf(raw, status);

		if (GET_STATUS_ATN(status))
			return SI_SM_ATTN;
		else
			return SI_SM_IDLE;

	case RAW_START_OP:
		if (state != RAW_IDLE) {
			start_error_recovery(raw,
					     "State machine not idle at start");
			break;
		}

		clear_obf(raw, status);
		write_cmd(raw, RAW_WRITE_START);
		raw->state = RAW_WAIT_WRITE_START;
		g_state = RAW_WRITE_STATE;
		break;

	case RAW_WAIT_WRITE_START:
		if (state != RAW_WRITE_STATE) {
			start_error_recovery(
				raw,
				"Not in write state at write start");
			break;
		}
		tx_data(raw);
		if (raw->write_count == 1) {
			write_cmd(raw, RAW_WRITE_END);
			raw->state = RAW_WAIT_WRITE_END;
		} else {
			write_next_byte(raw);
			raw->state = RAW_WAIT_WRITE;
		}
		break;

	case RAW_WAIT_WRITE:
		if (state != RAW_WRITE_STATE) {
			start_error_recovery(raw,
					     "Not in write state for write");
			break;
		}
		clear_obf(raw, status);
		if (raw->write_count == 1) {
			write_cmd(raw, RAW_WRITE_END);
			raw->state = RAW_WAIT_WRITE_END;
		} else {
			write_next_byte(raw);
		}
		break;

	case RAW_WAIT_WRITE_END:
		if (state != RAW_WRITE_STATE) {
			start_error_recovery(raw,
					     "Not in write state"
					     " for write end");
			break;
		}
		clear_obf(raw, status);
		write_next_byte(raw);
		raw->state = RAW_WAIT_READ;
		g_state = RAW_IDLE_STATE;

		/*Starting to handle IPMI request*/
		do_process_ipmi_request(raw);
		break;

	case RAW_WAIT_READ:
		if ((state != RAW_READ_STATE) && (state != RAW_IDLE_STATE)) {
			start_error_recovery(
				raw,
				"Not in read or idle in read state");
			break;
		}

		if (state == RAW_READ_STATE) {
			if (!check_obf(raw, status, time))
				return SI_SM_CALL_WITH_DELAY;
			read_next_byte(raw);
		} else {
			/*
			 * We don't implement this exactly like the state
			 * machine in the spec.  Some broken hardware
			 * does not write the final dummy byte to the
			 * read register.  Thus obf will never go high
			 * here.  We just go straight to idle, and we
			 * handle clearing out obf in idle state if it
			 * happens to come in.
			 */
			clear_obf(raw, status);
			raw->orig_write_count = 0;
			raw->state = RAW_IDLE;
			return SI_SM_TRANSACTION_COMPLETE;
		}
		break;

	case RAW_ERROR0:
		clear_obf(raw, status);
		status = read_status(raw);
		if (GET_STATUS_OBF(status))
			/* controller isn't responding */
			if (time_before(jiffies, raw->error0_timeout))
				return SI_SM_CALL_WITH_TICK_DELAY;
		write_cmd(raw, RAW_GET_STATUS_ABORT);
		raw->state = RAW_ERROR1;
		break;

	case RAW_ERROR1:
		clear_obf(raw, status);
		rx_data(raw, 0);
		raw->state = RAW_ERROR2;
		break;

	case RAW_ERROR2:
		if (state != RAW_READ_STATE) {
			start_error_recovery(raw,
					     "Not in read state for error2");
			break;
		}
		if (!check_obf(raw, status, time))
			return SI_SM_CALL_WITH_DELAY;

		clear_obf(raw, status);
		rx_data(raw, RAW_READ_BYTE);
		raw->state = RAW_ERROR3;
		break;

	case RAW_ERROR3:
		if (state != RAW_IDLE_STATE) {
			start_error_recovery(raw,
					     "Not in idle state for error3");
			break;
		}

		if (!check_obf(raw, status, time))
			return SI_SM_CALL_WITH_DELAY;

		clear_obf(raw, status);
		if (raw->orig_write_count) {
			restart_raw_transaction(raw);
		} else {
			raw->state = RAW_IDLE;
			return SI_SM_TRANSACTION_COMPLETE;
		}
		break;

	case RAW_HOSED:
		break;
	}

	if (raw->state == RAW_HOSED) {
		init_raw_data(raw, raw->io);
		return SI_SM_HOSED;
	}

	return SI_SM_CALL_WITHOUT_DELAY;
}

static int raw_size(void)
{
	return sizeof(struct si_sm_data);
}

static int raw_detect(struct si_sm_data *raw)
{
	/*
	 * It's impossible for the RAW status register to be all 1's,
	 * (assuming a properly functioning, self-initialized BMC)
	 * but that's what you get from reading a bogus address, so we
	 * test that first.
	 */
	//if (read_status(raw) == 0xff)
	//	return 1;
	//printk("%s\n",__func__);
	return 0;
}

static void raw_cleanup(struct si_sm_data *raw)
{
	ipmi_sdr_repository_cleanup();
}

struct si_sm_handlers raw_smi_handlers = {
	.init_data         = init_raw_data,
	.start_transaction = start_raw_transaction,
	.get_result        = get_raw_result,
	.event             = raw_event,
	.detect            = raw_detect,
	.cleanup           = raw_cleanup,
	.size              = raw_size,
};
