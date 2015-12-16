/**
 * xgene-pka.h - AppliedMicro Xgene PKA Driver Header file
 *
 * Copyright (c) 2013, Applied Micro Circuits Corporation
 * Author: Rameshwar Prasad Sahu <rsahu@apm.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __APM_PKA_H__
#define __APM_PKA_H__

#include <linux/interrupt.h>

#define APM_PKA_RAM_SIZE       (8*1024)
/* Reserve last 72 bytes for used by sequencer firmware and round down
   to 8-bytes align */
#define APM_PKA_RAM_FREE_SIZE	(((8*1024)-72)&~0x7)
#define APM_PKA_PROGRAM_SIZE    (2*1024)

#ifndef RC_OK
#define RC_OK                   0
#define RC_INVALID_PARM        -EINVAL
#define RC_NODEV               -ENODEV
#define RC_NO_IMPLEMENTATION   -ENOSYS
#define RC_ENOMEM              -ENOMEM
#define RC_EINPROGRESS         -EINPROGRESS
#define RC_EALREADY            -EALREADY
#define RC_EBUSY	       -EBUSY
#define RC_EIO		       -EIO

/* Error code base specify to AMCC */
#define RC_ERROR_BASE          5000
#define RC_HWERROR             -(RC_ERROR_BASE+0)
#define RC_FATAL               -(RC_ERROR_BASE+1)
#endif	/* RC_OK */

/**
 * PKA Register and bit Definitions
 *
 */

/* Register PKT_FUNCTION bit definition */
#define PKA_FUNCTION_MUL                    0x00000001
#define PKA_FUNCTION_ADDSUB                 0x00000002
#define PKA_FUNCTION_RSVD                   0x00000004
#define PKA_FUNCTION_MSONE                  0x00000008
#define PKA_FUNCTION_ADD                    0x00000010
#define PKA_FUNCTION_SUB                    0x00000020
#define PKA_FUNCTION_RSHIFT                 0x00000040
#define PKA_FUNCTION_LSHIFT                 0x00000080
#define PKA_FUNCTION_DIV                    0x00000100
#define PKA_FUNCTION_MOD                    0x00000200
#define PKA_FUNCTION_COMPARE                0x00000400
#define PKA_FUNCTION_COPY                   0x00000800
#define PKA_FUNCTION_SEQOP_MASK             (7 << 12)
#define PKA_FUNCTION_SEQOP_EXPMOD_CRT       (1 << 12)
#define PKA_FUNCTION_SEQOP_EXPMOD_ACT4      (2 << 12)
#define PKA_FUNCTION_SEQOP_ECC_ADD          (3 << 12)
#define PKA_FUNCTION_SEQOP_EXPMOD_ACT2      (4 << 12)
#define PKA_FUNCTION_SEQOP_ECC_MUL          (5 << 12)
#define PKA_FUNCTION_SEQOP_EXPMOD_VAR       (6 << 12)
#define PKA_FUNCTION_SEQOP_MODINV           (7 << 12)
#define PKA_FUNCTION_RUN                    (1 << 15)
#define PKA_FUNCTION_STALL                  (1 << 24)

/* Register PKA_COMPARE bit definition */
#define PKA_COMPARE_EQUAL                   0x00000001
#define PKA_COMPARE_LESSTHAN                0x00000002
#define PKA_COMPARE_GREATHERTHAN            0x00000004

/* Register PKA_MSW bit definition */
#define PKA_MSW_ADDR_MASK                   0x000007FF
#define PKA_MSW_RESULT_ZERO                 (1 << 15)

/* Register PKA_DIVMSW bit definition */
#define PKA_DIVMSW_ADDR_MASK                0x000007FF
#define PKA_DIVMSW_RESULT_ZERO              (1 << 15)

/* Register PKA_SEQ_CTRL bit definition */
#define PKA_SEQ_CTRL_SW_CTRL_TRIGGER_MASK   0x000000FF
#define PKA_SEQ_CTRL_SEQ_STATUS_MASK        0x0000FF00
#define PKA_SEQ_CTRL_RESET                  0x80000000

/* PKA Registers definitions */
#if !defined(PKA_APTR_ADDR)
#define PKA_APTR_ADDR                       0x0000
#define PKA_BPTR_ADDR                       0x0004
#define PKA_CPTR_ADDR                       0x0008
#define PKA_DPTR_ADDR                       0x000C
#define PKA_ALENGTH_ADDR                    0x0010
#define PKA_BLENGTH_ADDR                    0x0014
#define PKA_SHIFT_ADDR                      0x0018
#define PKA_FUNCTION_ADDR                   0x001C
#define PKA_COMPARE_ADDR                    0x0020
#define PKA_MSW_ADDR                        0x0024
#define PKA_DIVMSW_ADDR                     0x0028
#define PKA_SEQ_CTRL_ADDR                   0x00C8
#define PKA_OPTIONS_ADDR                    0x00f4
#define PKA_SW_REV_ADDR                     0x00f8
#define PKA_REVISION_ADDR                   0x000fc
#endif
#define PKA_RAM_ADDR                        0x2000
/* PKA Program area is shared with PKA_RAM. To access, put the Sequencer
   into reset. See PKA_SEQ_CTRL. */
#define PKA_PROGRAM_ADDR                    0x2000

/* # of operation can be queued */
#define APM_PKA_PENDING_OP_MAX		    256
#define MAX_FREE_DESC_AVAIL_TRY		    10

typedef void (*apm_pka_cb)(void *ctx, int status);

/** Structure definitions used for asynchronous PKA operations */
struct apm_pka_async_pkcp_op {
	u32 vecA_cnt;
	u32 *vecA;
	u32 vecB_cnt;
	u32 *vecB;
	u32 *vec_addsub_C;
	u8  shift_val;
};

struct apm_pka_async_expmod_op {
	u32 *base;
	u32 *exp;
	u32 *modulus;
	u32 exp_cnt;
	u32 base_mod_cnt;
};

struct apm_pka_async_expmod_crt_op {
	u32 *expP;
	u32 *expQ;
	u32 exp_len;
	u32 *modP;
	u32 *modQ;
	u32 mod_inverse_len;
	u32 *inverseQ;
	u32 *input;
};

struct apm_pka_async_mod_inv_op {
        u32 num_cnt;
        u32 *num;
        u32 mod_cnt;
        u32 *mod;
};

struct apm_pka_async_ecc_add_op {
       u32 vect_cnt;
       u32 *vectA_x;
       u32 *vectA_y;
       u32 *curve_p;
       u32 *curve_a;
       u32 *vectC_x;
       u32 *vectC_y;
};

struct apm_pka_async_ecc_mul_op {
       u32 vectA_cnt;
       u32 *scalar_k;
       u32 vectB_cnt;
       u32 *curve_p;
       u32 *curve_a;
       u32 *curve_b;
       u32 *vectC_x;
       u32 *vectC_y;
};

#define async_pkcp		async_op.pkcp_op
#define async_expmod		async_op.expmod_op
#define async_expmod_crt	async_op.expmod_crt_op
#define async_mod_inv           async_op.mod_inv_op
#define async_ecc_add           async_op.ecc_add_op
#define async_ecc_mul           async_op.ecc_mul_op

struct apm_pka_op
{
	u8	id;
	u32	opcode;		     /* Encoded as PKA_FUNCTION register */
	apm_pka_cb	cb;	     /* Callback function */
	void 	*ctx;		     /* Context for callback function */
	u16     resultC_cnt;	     /* # of DWORD for result C */
	u16     resultD_cnt;	     /* # of DWORD for result D */
	u32 	*resultC_addr;	     /* Address for result C */
	u32 	*resultD_addr;	     /* Address for result D */
        u32     *result_ecc_x_addr;  /* Address for ECC result x co-ordinate */
        u32     *result_ecc_y_addr;  /* Address for ECC result y co-ordinate */
	u32     ramC_addr;	     /* PKA RAM Address for result C in DWORD unit */
	u32     ramD_addr;	     /* PKA RAM Address for result D in DWORD unit */
	u32     ram_addr_start;	     /* PKA RAM Address start */
	u16     ram_addr_size;	     /* PKA RAM Address size */
	struct  list_head next;
	int     status;
	union   {
		struct apm_pka_async_expmod_crt_op expmod_crt_op;
		struct apm_pka_async_expmod_op expmod_op;
		struct apm_pka_async_pkcp_op pkcp_op;
                struct apm_pka_async_mod_inv_op mod_inv_op;
                struct apm_pka_async_ecc_add_op ecc_add_op;
                struct apm_pka_async_ecc_mul_op ecc_mul_op;
	} async_op;
};

/** Context for async operations */
struct apm_pka_ctx
{
	s16	ram_blk2use;
	struct {
		u8  usable;
		s16 sizeDW;
		u32 addrDW;
	} ram_blk[2];
	u16	op_head;
	u16	op_tail;
	struct apm_pka_op op[APM_PKA_PENDING_OP_MAX];
	u32 op_cnt;
	spinlock_t op_lock;
	u32 csr_paddr;
	u32 pka_ram_paddr;
	volatile void __iomem *csr;
	int irq;
	struct list_head completed_event_queue;
	spinlock_t 	lock;
	struct tasklet_struct	tasklet;
};

/**
 * @brief   IRQ Handler for PKA Interrupts
 * @return  Success/Failure
 */
irqreturn_t apm_pka_irq_handler(int irq, void *id);

/**
 * @brief   Tasklet for post-processing of result of PKA operations
 */
void apm_pka_tasklet_cb (unsigned long data);

/**
 * @brief   Process completed events
 */
void apm_pka_process_completed_event (struct apm_pka_op *op);

/**
 * @brief Initiaize PKA hardware
 * @return  0 or relevant error code
 *
 */
int apm_pka_hw_init(void);

/**
 * @brief De-Initiaize PKA hardware
 * @return  0
 *
 */
int apm_pka_hw_deinit(void);

/**< PKA input vector max count */
#define APM_PKA_VECTOR_MAXSIZE         256
#define APM_PKA_ECC_VECTOR_MAXSIZE     24

/**
 * @brief Performs large vector multiplication (A * B = C)
 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complex operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param multiplicand_len Length of multiplicand in DWORD unit (<=
 *			    APM_PKA_VECTOR_MAXSIZE)
 * @param multiplicand  Multiplicand vector pointer of the operation - input A
 * @param multiplier_len   Length of multiplier in DWORD unit (<=
 *			    APM_PKA_VECTOR_MAXSIZE)
 * @param multiplier    Multiplier vector pointer of the operation - input B
 * @param product       Result product vector - output C. Length is
 *			 multiplicand_len * multiplier_len in DWORD unit
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		- operation completed successfully (= 0)
 *
 */
int apm_pka_mul(apm_pka_cb cb,
		void *ctx,
		u32  *op_id,
		u32  multiplicand_len,
		u32  *multiplicand,
		u32  multiplier_len,
		u32  *multiplier,
		u32  *product);

/**
 * @brief Performs vector division (A / B = D and A % B = C)
 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complex operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param dividend_len  Length of dividend in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param dividend      Dividend vector pointer of the operation - input A
 * @param divisor_len   Length of divisor in DWORD unit (<= dividend_len <=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param divisor       Divisor vector pointer of the operation - input B -
 *			 where most significant 32 bits != 0
 * @param remainder     Result reminder vector pointer of the operation -
 *			 output C. Length is divisor_len + 1 in DWORD unit
 * @param quotient      Result integer vector pointer of the operation -
 *			 output D. Length is dividend_len - divisor_len + 1
 *			 in DWORD unit
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		- operation completed successfully (= 0)
 *
 */
int apm_pka_div(apm_pka_cb cb,
		void *ctx,
		u32  *op_id,
		u32  dividend_len,
		u32  *dividend,
		u32  divisor_len,
		u32  *divisor,
		u32  *remainder,
		u32  *quotient);

/**
 * @brief Performs vector modulo (A % B = C) where B != 0
 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complex operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param dividend_len  Length of dividend in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param dividend      Dividend vector pointer of the operation - input A
 * @param divisor_len   Length of divisor in DWORD unit (<= dividend_len <=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param divisor       Divisor vector pointer of the operation - input B
 * @param remainder     Result reminder vector pointer of the operation -
 *			 output C. Length is divisor_len + 1 in DWORD unit
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		- operation completed successfully (= 0)
 *
 */
int apm_pka_mod(apm_pka_cb cb,
		void *ctx,
		u32 *op_id,
		u32 dividend_len,
		u32 *dividend,
		u32 divisor_len,
		u32 *divisor,
		u32 *remainder);

/**
 * @brief Performs vector addition (A + B = C)
 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complex operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param addendA_len   Length of addendA in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param addendA       Addend vector pointer of the operation - input A
 * @param addendB_len   Length of addendB in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param addendB       Addend vector pointer of the operation - input B
 * @param sum           Result sum vector pointer of the operation - output C.
 *			 Length is MAX(addendA_len, addendB_len) + 1 in DWORD
 *			 unit
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		- operation completed successfully (= 0)
 *
 */
int apm_pka_add(apm_pka_cb cb,
		void *ctx,
		u32 *op_id,
		u32 addendA_len,
		u32 *addendA,
		u32 addendB_len,
		u32 *addendB,
		u32 *sum);

/**
 * @brief Performs vector substraction (A - B = C) where A >= B
 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complex operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param minuend_len   Length of minuend in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param minuend       Minuendnd vector pointer of the operation - input A
 * @param subtrahend_len Length of minuend in DWORD unit (<=
 *			  APM_PKA_VECTOR_MAXSIZE)
 * @param subtrahend    Subtrahend vector pointer of the operation - input B
 * @param difference    Result difference vector pointer of the operation -
 *			 output C. Length is MAX(minuend_len, subtrahend_len)
 *			 in DWORD unit.
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		- operation completed successfully (= 0)
 *
 */
int apm_pka_sub(apm_pka_cb cb,
		void *ctx,
		u32 *op_id,
		u32 minuend_len,
		u32 *minuend,
		u32 subtrahend_len,
		u32 *subtrahend,
		u32 *difference);

/**
 * @brief Performs vector addition and then substraction (A + C - B = D) where
 *        A + C >= B.
 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complex operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param input_cnt     Length of all inputs in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param addendA       Addend vector pointer of the operation - input A
 * @param addendC       Addend vector pointer of the operation - input C
 * @param subtrahend    Subtrahend vector pointer of the operation - input B
 * @param result        Result vector pointer of the operation - output D.
 *                       Length is input_cnt + 1 in DWORD unit.
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		- operation completed successfully (= 0)
 *
 */
int apm_pka_addsub(apm_pka_cb cb,
		   void *ctx,
		   u32 *op_id,
		   u32 input_cnt,
		   u32 *addendA,
		   u32 *addendC,
		   u32 *subtrahend,
		   u32 *result);

/**
 * @brief Performs vector right shift (A >> X = C)
 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complex operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param input_cnt     Length of input in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param input         Input vector pointer of the operation - input A
 * @param shift         Shift value of the operation - input X. 0 <= shift <= 31
 * @param result        Result vector pointer of the operation - output C
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		- operation completed successfully (= 0)
 *
 */
int apm_pka_rshift(apm_pka_cb cb,
		   void *ctx,
		   u32 *op_id,
		   u32 input_cnt,
		   u32 *input,
		   u8 shift,
		   u32 *result);

/**
 * @brief Performs vector left shift (A << X = C)
 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complex operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param input_cnt     Length of input in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param input         Input vector pointer of the operation - input A
 * @param shift         Shift value of the operation - input X. 0 <= shift <= 31
 * @param result        Result vector pointer of the operation - output C
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		- operation completed successfully (= 0)
 *
 */
int apm_pka_lshift(apm_pka_cb cb,
		   void *ctx,
		   u32 *op_id,
		   u32 input_cnt,
		   u32 *input,
		   u8 shift,
		   u32 *result);

/**
 * @brief Performs vector comparsion (A < B, A == B, or A > B)
 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complex operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param input1_cnt    Length of input1 in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param input1        Input vector pointer of the operation - input A
 * @param input2_cnt    Length of input2 in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param input2        Input vector pointer of the operation - input B
 * @param result        Single result pointer to an unsigned char to store
 *			the result. 0 = A=B; -1 = A<B; 1 = A>B.
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		- operation completed successfully (= 0)
 *
 */
int apm_pka_compare(apm_pka_cb cb,
		    void *ctx,
		    u32 *op_id,
		    u32 input1_cnt,
		    u32 *input1,
		    u32 input2_cnt,
		    u32 *input2,
		    int *result);

/**
 * @brief Performs vector modular exponentiation operation - C^A % B = D where B
 *        is odd integer and > 2^32 and C < B
 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complext operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param odd_pwr_cnt	# of pre-calculated 'odd power'. For value of:
 *			 2 = ExpMod-ACT2
 *			 8 = ExpMod-ACT4
 *			 1,3,4,5,6,7,9..16 = ExpMod-variable
 *			 17...MAX  = invalid parameter
 * @param base_mod_cnt   Length of base and modulus in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param base          Base vector pointer of the operation - input C
 * @param modulus       Modulus vector pointer of the operation - input B
 * @param exponent_cnt  Length of exponent in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param exponent      Exponent vector pointer of the operation - input A
 * @param result        Result vector pointer of the operation. Length is
 *			 base_mod_cnt in DWORD unit.
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		- operation completed successfully (= 0)
 *
 * @note Use of odd power > 8 is not advisable as the speed advantage for
 *       each extra odd power decreases rapidly (and even become negative for
 *       short exponent vector lengths due to the extra pre-processsing
 *       required). More odd power requires more PKA RAM which will decrease
 *       queue (setup) of pending operation for asynchronous operation.
 *
 */
int apm_pka_expmod(apm_pka_cb cb,
		   void *ctx,
		   u32 *op_id,
		   u8  odd_pwr_cnt,
		   u32 base_mod_cnt,
		   u32 *base,
		   u32 *modulus,
		   u32 exponent_cnt,
		   u32 *exponent,
		   u32 *result);

/**
 * @brief Performs vector modular exponentiation with CRT operation (RSA-CRT)
 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complex operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param odd_pwr_cnt	# of pre-calculated 'odd power' (1-16)
 * @param exp_len    	Length of exponent(s) in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param expP          Exponent P vector pointer of the operation - input expP
 * @param expQ          Exponent Q vector pointer of the operation - input expQ
 * @param mod_inverse_len Length of modulus(es), inverse in DWORD unit (<=
 *			   APM_PKA_VECTOR_MAXSIZE)
 * @param modP          Modulus P vector pointer of the operation - input modP
 * @param modQ          Modulus Q vector pointer of the operation - input modQ
 * @param inverseQ      Inverse Q vector pointer of the operation - input
 *			 inverseQ
 * @param input         Input vector pointer of the operation - input
 *			 'Input'. Input is 2*mod_inverse_len.
 * @param result        Result vector of the operation. Result is
 *			 2*mod_inverse_len.
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		- operation completed successfully (= 0)
 *
 * @note CRT Formula Rewritten:
 *
 *		X = (Input % modP)^expP % modP
 *              Y = (Input % modQ)^expQ % modP
 *              Z = (((X-Y) % modP) * inverseQ % modP) * modQ
 *              R = Y + Z
 *        	inverseQ = modQ^-1 % modP
 *		expP = d^-1 % (modP - 1)
 *              expQ = d^-1 % (modQ - 1)
 *
 *        where modP and modQ is odd and co-prime (GCD=1), modP > 2^32,
 *	  modQ > 2^32, 0 < expP < modP - 1, 0 < expQ < modQ - 1,
 *        inverseQ * modP = 1, Input < (modP * modQ), and d is decrypt
 *        exponent. d can be calculated by d = e^-1 % (p - 1)(q - 1) where
 *        e is the encrypt exponent chosen such that it is prime and
 *        GCD(p - 1, q - 1) != e.
 *
 */
int apm_pka_expmod_crt(apm_pka_cb cb,
		       void *ctx,
		       u32 *op_id,
		       u8 odd_pwr_cnt,
		       u32 exp_len,
		       u32 *expP,
		       u32 *expQ,
		       u32 mod_inverse_len,
		       u32 *modP,
		       u32 *modQ,
		       u32 *inverseQ,
		       u32 *input,
		       u32 *result);

/**
 * @brief Performs large vector Copy operation (Copy A to C)
 * @param cb        Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complex operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx	     A void context pointer passed in cb function
 * @param op_id	     An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param input_cnt  Length of input vector in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param input      input vector pointer of the operation - input A
 * @param result     Result vector of the operation. Result is same as input  A.
 *
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		 - operation completed successfully (= 0)
 *
 */
int apm_pka_copy(apm_pka_cb cb,
		 void *ctx,
		 u32 *op_id,
		 u32 input_cnt,
		 u32* input,
		 u32 *result);

/**
 * @brief Performs vector modular inversion operation - A^(-1) % B = D where B
 *                      is odd integer and may not have 1.
 * @param cb            Callback function of asynchronous operation. If this A^(-1)
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complext operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param num_cnt       Length of Number to invert in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param num          Number to invert  vector pointer of the operation ,
 *                      length is num_cnt long - input A
 * @param mod_cnt      Length of modulus in DWORD unit (<=
 *			 APM_PKA_VECTOR_MAXSIZE)
 * @param mod          Modulus vector pointer of the operation, length is mod_cnt
 long  - input B
 * @param result       Result vector pointer of the operation. Length is
 *			 mod_cnt long in DWORD unit.
 * @return -EINVAL -   any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		 - operation completed successfully (= 0)
 *
 */
int apm_pka_mod_inv(apm_pka_cb cb,
		    void *ctx,
		    u32 *op_id,
		    u32 num_cnt,
		    u32 *num,
		    u32 mod_cnt,
		    u32 *mod,
		    u32*result);

/**
 * @brief Performs ECC ADDITION operation - Point addition/doubling on elliptic curve:
 *          y^2 = x^3 + a.x + b (mod p)
 *          pntA + pntC = pntD where
 *          Modulus p must be a prime > 2^63
 *          Effective modulus size (in bits) must be a multiple of 32
 *          a < p and b < p
 *          pntA and pntC must be on the curve (this is not checked)
 *          Neither pntA nor pntC can be the "point-at-infinity", although ECC-ADD can
 *          return this point as a result.
 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complext operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param vect_cnt      Length of all parameter in this function in DWORD unit (<=
 *			 APM_PKA_ECC_VECTOR_MAXSIZE), vectA len is not used
 * @param vectA_x       vectA x co-ordinate on the curve  pointer of the
 *                       operation, length is vect_cnt long - input A
 * @param vectA_y       vectA y co-ordinate on the curve  pointer of the
 *                       operation, length is vect_cnt long - input A
 * @param curve_p       modulus p pointer of the operation, length is
 *                       vect_cnt long - input B
 * @param curve_a       value of a pointer of the operation, length is
 *                       vect_cnt long  - input B ( b is not used)
 * @param vectC_x       vectC x co-ordinate on the curve pointer of the operation, length is
 *                       vect_cnt long  - input C
 * @param vectC_y       vectC y co-ordinate on the curve pointer of the operation, length is
 *                       vect_cnt long  - input C
 * @param result_vect_x  x co-ordinate  Result vector pointer of the operation. Length is
 *			  vect_cnt long in DWORD unit.
 * @param result_vect_y  y co-ordinate  Result vector pointer of the operation. Length is
 *			  vect_cnt long in DWORD unit.
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		 - operation completed successfully (= 0)
 *
 */
int apm_pka_ecc_add(apm_pka_cb cb,
		    void *ctx,
		    u32 *op_id,
		    u32 vect_cnt,
		    u32 *vectA_x,
		    u32 *vectA_y,
		    u32 *curve_p,
		    u32 *curve_a,
		    u32 *vectC_x,
		    u32 *vectC_y,
		    u32 *result_vect_x,
		    u32 *result_vect_y);

/**
 * @brief Performs ECC MULTIPLICATION operation - Point multiplication on elliptic curve:
 *      y^2 = x^3 + a.x + b (mod p)
 *      k * pntC = pntD where
 *      Modulus p must be a prime > 2^63
 *      Effective modulus size (in bits) must be a multiple of 32
 *      a < p and b < p
 *     pntC must be on the curve (this is not checked)
 *     pntC can be the "point-at-infinity", although ECC-ADD can
 *     return this point as a result.

 * @param cb            Callback function of asynchronous operation. If this
 *                       parameter is NULL, then it will block until operation
 *                       completed. As this function is called from interrupt
 *                       context, it should defer complext operation and
 *                       return as soon as possible. When there is pending
 *			 operation, this parameter can not be NULL.
 * @param ctx		A void context pointer passed in cb function
 * @param op_id		An integer identifier pointer for asynchronous
 *			 operation. It can be used to cancel pending operation.
 *			 Can be NULL.
 * @param vectA_cnt     Length of scalar k in DWORD unit (<=
 *			 APM_PKA_ECC_VECTOR_MAXSIZE)
 * @param scalar_k      k to multiply with curve point  pointer of the
 *                       operation, , length is  vectA_cnt long  - input A
 * @param vectB_cnt     Length of vectB and  vectC in DWORD unit (<=
 *			 APM_PKA_ECC_VECTOR_MAXSIZE),
 * @param curve_p       modulus p pointer of the operation, length is  vectB_cnt long  - input B
 * @param curve_a       value of a pointer of the operation, length is
 *                       vectB_cnt long  - input B ( b is not used)
 * @param curve_b       value of b pointer of the operation, length is
 *                       vectB_cnt long  - input B ( b is not used)
 * @param vectC_x       vectC x co-ordinate on the curve pointer of the operation, length is
 *                       vectB_cnt long  - input C
 * @param vectC_y       vectC y co-ordinate on the curve pointer of the operation, length is
 *                       vectB_cnt long  - input C
 * @param result_vect_x  x co-ordinate  Result vector pointer of the operation. Length is
 *			  vectB_cnt long in DWORD unit.
 * @param result_vect_y   y co-ordinate  Result vector pointer of the operation. Length is
 *			   vectB_cnt long in DWORD unit.
 * @return -EINVAL - any invalid parameters (< 0)
 *         -EBUSY        - If cb ia NULL and there is pending operation
 *				  (< 0). Or, hardware buffer is full.
 *	   -EINPROGRESS  - operation is pending (< 0)
 *         0 		 - operation completed successfully (= 0)
 *
 */
int apm_pka_ecc_mul(apm_pka_cb cb,
		    void *ctx,
		    u32 *op_id,
		    u32 vectA_cnt,
		    u32 *scalar_k,
		    u32 vectB_cnt,
		    u32 *curve_p,
		    u32 *curve_a,
		    u32 *curve_b,
		    u32 *vectC_x,
		    u32 *vectC_y,
		    u32 *result_vect_x,
		    u32 *result_vect_y);
#endif
