/**
 * xgene-pka.c - AppliedMicro Xgene  PKA Driver
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include "xgene-pka.h"
#include "xgene-pka-firmware.h"

#define APM_PKA_VER_STR	  "0.2"
#define APM_PKA_HDR	  "APM_PKA: "

#undef ENABLE_PKA_DBG
#undef ENABLE_PKA_CSRDBG

#if defined(ENABLE_PKA_DBG)
#define DEBUG(fmt...) do { printk(KERN_INFO "APM PKA: " fmt); } while (0)
#else
#define DEBUG(fmt...)
#endif

#if defined(ENABLE_PKA_CSRDBG)
#define CSRDEBUG(fmt...) do { printk(KERN_INFO "APM PKA CSR: " fmt); } while (0)
#else
#define CSRDEBUG(fmt...)
#endif

struct apm_pka_dev {
	struct apm_pka_ctx	ctx;
	struct resource 	csr_res;
	int			pwredoff;
};

static struct apm_pka_dev	pka_dev = {
	.ctx = {
		0,
		{ { 1, APM_PKA_RAM_FREE_SIZE/4, 0 },
		  { 0, 0, 0 } },
		  0, 0
	}
};

static int apm_pka_readl(u32 reg_addr, u32 *data_val)
{
	int ret = 0;
	*data_val = readl(pka_dev.ctx.csr + reg_addr);
        CSRDEBUG("RD: ADDR  0x%p  Val  0x%08X\n", pka_dev.ctx.csr + reg_addr, *data_val);
        return ret;
}

static int apm_pka_writel(u32 reg_addr, u32 data_val)
{
        int ret = 0;
	writel(data_val, pka_dev.ctx.csr + reg_addr);
        CSRDEBUG("WR: ADDR  0x%p  Val  0x%08X\n", pka_dev.ctx.csr + reg_addr, data_val);
        return ret;
}

static int pka_clk_init(struct device *dev)
{
        struct clk *clk;
        int ret = 0;

        /* Enable APM PKA IP clock */
	clk = clk_get(dev, NULL);
	if (IS_ERR(clk)) {
		dev_err(dev, "PKA Error: No clock interface\n");
		ret = -EPERM;
	}
	ret = clk_prepare_enable(clk);
	if (ret) {
		clk_put(clk);
        	dev_err(dev, "PKA Error: clock prepare enable failed");
		ret = -EPERM;
	}

        return ret;
}

/**
 * PKA Functions
 *
 */
/* # of time to poll for synchronous operation */
#define APM_PKA_POLL_DONE_MAX_CNT	5000

#define APM_PKA_CSR_WRITE_RETURN(a, v)     \
	do { \
		rc = apm_pka_writel((a), (v)); \
		if (rc != 0) \
			return rc; \
	} while(0);

#define APM_PKA_CSR_READ_RETURN(a, v)     \
	do { \
		rc = apm_pka_readl((a), (v)); \
		if (rc != 0) \
			return rc; \
	} while(0);


#define PKA_ALIGN(x, a)		do {	\
					(x) += ((a)-1); \
					(x) &= ~((a)-1); \
				} while(0);
#define PKA_ALIGN_RVAL(x, a)	(((x) + ((a)-1)) & (~((a)-1)))

static u32 pkt_firmware_sizedw		= PKA_FIRMWARE_1_3_SIZEDW;
static const u32 *pka_firmware		= pka_firmware_1_3;

u32 apm_pka_pkcp_set_vec(u32 vecA_cnt,
			 u32 *vecA,
			 u32 vecB_cnt,
			 u32 *vecB)
{
	u32 addr;
	int rc, i;
	u32 val32;

	addr  = PKA_RAM_ADDR;
	/* Set PKA RAM address for vectA and load input A */
	APM_PKA_CSR_WRITE_RETURN(PKA_ALENGTH_ADDR, vecA_cnt);
	APM_PKA_CSR_WRITE_RETURN(PKA_APTR_ADDR, addr >> 2);
	for (i = 0; i < vecA_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, vecA[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  vectA Val  0x%08X\n", addr, val32);
	}
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for vectA and load input B */
	APM_PKA_CSR_WRITE_RETURN(PKA_BLENGTH_ADDR, vecB_cnt);
	APM_PKA_CSR_WRITE_RETURN(PKA_BPTR_ADDR, addr >> 2);
	for (i = 0; i < vecB_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, vecB[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  vectB Val  0x%08X\n", addr, val32);
	}
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for Output C */
	APM_PKA_CSR_WRITE_RETURN(PKA_CPTR_ADDR, addr >> 2);

	return addr;
}

u32 apm_pka_addsub_set_vec(u32 input_cnt,
			   u32 *addendA,
			   u32 *subtrahend,
			   u32 *addendC)
{
	u32 addr;
	int rc, i;
        u32 val32;

	addr  = PKA_RAM_ADDR;
	/* Set PKA RAM address for vectA and load input A - addendA */
	APM_PKA_CSR_WRITE_RETURN(PKA_ALENGTH_ADDR, input_cnt);
	APM_PKA_CSR_WRITE_RETURN(PKA_APTR_ADDR, addr >> 2);
	for (i = 0; i < input_cnt; i++, addr += 4)
        {
		APM_PKA_CSR_WRITE_RETURN(addr, addendA[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  AddendA Val  0x%08X\n", addr, val32);
        }
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for vectB and load input B - subtrahend */
	APM_PKA_CSR_WRITE_RETURN(PKA_BPTR_ADDR, addr >> 2);
	for(i = 0; i < input_cnt; i++, addr += 4)
        {
		APM_PKA_CSR_WRITE_RETURN(addr, subtrahend[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  Subtrahend Val  0x%08X\n", addr, val32);
        }
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for vectC and load input C - addendC */
	APM_PKA_CSR_WRITE_RETURN(PKA_CPTR_ADDR, addr >> 2);
	for (i = 0; i < input_cnt; i++, addr += 4)
        {
		APM_PKA_CSR_WRITE_RETURN(addr, addendC[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  AddendC Val  0x%08X\n", addr, val32);
        }
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for output D - result */
	APM_PKA_CSR_WRITE_RETURN(PKA_DPTR_ADDR, addr >> 2);

	return addr;
}

u32 apm_pka_shift_set_vec(u32 input_cnt,
			  u32 *input,
			  u8  shift)
{
	u32 addr;
	int rc, i;
        u32 val32;

	addr  = PKA_RAM_ADDR;
	/* Set PKA RAM address for vectA and load input A - input */
	APM_PKA_CSR_WRITE_RETURN(PKA_ALENGTH_ADDR, input_cnt);
	APM_PKA_CSR_WRITE_RETURN(PKA_APTR_ADDR, addr >> 2);
	for (i = 0; i < input_cnt; i++, addr += 4)
        {
		APM_PKA_CSR_WRITE_RETURN(addr, input[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  Shift Input Val  0x%08X\n", addr, val32);
        }
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set shift value */
	APM_PKA_CSR_WRITE_RETURN(PKA_SHIFT_ADDR, shift);
        DEBUG("Shif Count  0x%08X\n", shift);
	/* Set PKA RAM address for output - result */
	APM_PKA_CSR_WRITE_RETURN(PKA_CPTR_ADDR, addr >> 2);

	return addr;
}

u32 apm_pka_expmod_crt_set_vec(u32 exp_len,
			       u32 *expP,
			       u32 *expQ,
			       u32 mod_inverse_len,
			       u32 *modP,
			       u32 *modQ,
			       u32 *inverseQ,
			       u32 *input)
{
	u32 addr;
	u32 Daddr;
	int i, rc;
	u32 val32;

	addr  = PKA_RAM_ADDR;

        /* Set PKA RAM Address for vectA and load input A - expP */
	APM_PKA_CSR_WRITE_RETURN(PKA_ALENGTH_ADDR, exp_len);
	APM_PKA_CSR_WRITE_RETURN(PKA_APTR_ADDR, addr >> 2);
	for (i = 0; i < exp_len; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, expP[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X  expP Val  0x%08X\n",addr, val32);
	}
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

        /* Load input A - expQ */
	for (i = 0; i < exp_len; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, expQ[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  expQ Val 0x%08X\n",addr, val32);
	}
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for vectB and load input B - modQ */
	APM_PKA_CSR_WRITE_RETURN(PKA_BLENGTH_ADDR, mod_inverse_len);
	APM_PKA_CSR_WRITE_RETURN(PKA_BPTR_ADDR, addr >> 2);
	for(i = 0; i < mod_inverse_len; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, modP[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X  modP Val 0x%08X\n", addr, val32);
	}
	addr += 8;/*mm */	/* Require 1 extra DWORD */
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

        /* Load input B - modQ */
	for(i = 0; i < mod_inverse_len; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, modQ[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X  modQ Val  0x%08X\n", addr, val32);
	}
	addr += 4;		/* Require 1 extra DWORD */
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for vectC and load input C - inverseQ */
	APM_PKA_CSR_WRITE_RETURN(PKA_CPTR_ADDR, addr >> 2);
	for(i = 0; i < mod_inverse_len; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, inverseQ[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  invQ Val 0x%08X\n", addr, val32);
	}
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for output D - result */
	APM_PKA_CSR_WRITE_RETURN(PKA_DPTR_ADDR, addr >> 2);
	Daddr = addr;
	for(i = 0; i < (mod_inverse_len<<1); i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, input[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  Expmod Input Val  0x%08X\n", addr, val32);
	}

	return Daddr;
}

u32  apm_pka_expmod_set_vec(u32 base_mod_cnt,
			    u32 *base,
			    u32 *modulus,
			    u32 exponent_cnt,
			    u32 *exponent)
{
	u32 addr;
	u32 val32;
	int rc, i;

	addr  = PKA_RAM_ADDR;

	/* Set PKA RAM address for vectA and load input A - exponent */
	APM_PKA_CSR_WRITE_RETURN(PKA_ALENGTH_ADDR, exponent_cnt);
	APM_PKA_CSR_WRITE_RETURN(PKA_APTR_ADDR, addr >> 2);
	for(i = 0; i < exponent_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, exponent[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  Exp Val  0x%08X\n", addr, val32);
	}
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for vectB and load input B - modulus */
	APM_PKA_CSR_WRITE_RETURN(PKA_BLENGTH_ADDR, base_mod_cnt);
	APM_PKA_CSR_WRITE_RETURN(PKA_BPTR_ADDR, addr >> 2);
	for(i = 0; i < base_mod_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, modulus[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  Mod Val  0x%08X\n", addr, val32);
	}
	addr += 4;		/* Require 1 extra DWORD */
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for vectC and load input C - base */
	APM_PKA_CSR_WRITE_RETURN(PKA_CPTR_ADDR, addr >> 2);
	for(i = 0; i < base_mod_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, base[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  Base Val  0x%08X\n", addr, val32);
	}
	addr += 4;		/* Require 1 extra DWORD */
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for output D - result */
	APM_PKA_CSR_WRITE_RETURN(PKA_DPTR_ADDR, addr >> 2);

	return addr;
}

u32 apm_pka_copy_set_vec(u32 vectA_cnt,
			 u32* vectA)
{
	u32 addr;
	int rc, i;
	u32 val32;

	addr  = PKA_RAM_ADDR;
	/* Set PKA RAM address for vectA and load input A - input */
	APM_PKA_CSR_WRITE_RETURN(PKA_ALENGTH_ADDR, vectA_cnt);
	APM_PKA_CSR_WRITE_RETURN(PKA_APTR_ADDR, addr >> 2);

	for (i = 0; i < vectA_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, vectA[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  Copy Input Val  0x%08X\n", addr, val32);
	}
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for output C - result */
	APM_PKA_CSR_WRITE_RETURN(PKA_CPTR_ADDR, addr >> 2);

	return addr;
}

u32 apm_pka_mod_inv_set_vec(u32 num_cnt,
			    u32 *num ,
			    u32 mod_cnt,
			    u32 *mod)
{
	u32 addr;
	u32 i, rc;
	u32 val32;

	addr  = PKA_RAM_ADDR;

	/* Set PKA RAM address for vectA and load input A - Number to invert */
	APM_PKA_CSR_WRITE_RETURN(PKA_ALENGTH_ADDR, num_cnt);
	APM_PKA_CSR_WRITE_RETURN(PKA_APTR_ADDR, addr >> 2);
	for (i = 0; i < num_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, num[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X  Num Val  0x%08X\n", addr, val32);
	}
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for vectB and load input B - modulus */
	APM_PKA_CSR_WRITE_RETURN(PKA_BLENGTH_ADDR, mod_cnt);
	APM_PKA_CSR_WRITE_RETURN(PKA_BPTR_ADDR, addr >> 2);
	for(i = 0; i < mod_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, mod[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X  Mod Val  0x%08X\n", addr, val32);
	}
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for output D - result */
	APM_PKA_CSR_WRITE_RETURN(PKA_DPTR_ADDR, addr >> 2);

	return addr;
}

u32 apm_pka_ecc_add_set_vec(u32 vect_cnt,
			    u32 *vectA_x,
			    u32 *vectA_y,
			    u32 *curve_p,
			    u32 *curve_a,
			    u32 *vectC_x,
			    u32 *vectC_y)
{
	u32 addr;
	int i, rc;
	u32 val32;
        u32 extra_dword;

        if (vect_cnt % 2)
		extra_dword = 3;
        else
		extra_dword = 2;

	addr  = PKA_RAM_ADDR;

	/* Set PKA RAM address for vectA and load input A - vectA_x co-ordinate */
	APM_PKA_CSR_WRITE_RETURN(PKA_APTR_ADDR, addr >> 2);
	for (i = 0; i < vect_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, vectA_x[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X  vectA_X Val  0x%08X\n", addr, val32);
	}
	addr += (extra_dword * 4); 	/* Require extra DWORD */
	PKA_ALIGN(addr, 8);      	/* Align 8-byte */

	/* Load input A - vectA_y co-ordinate */
	for (i = 0; i < vect_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, vectA_y[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X  vectA_Y Val  0x%08X\n", addr, val32);
	}
	addr += (extra_dword * 4); 	/* Require extra DWORD */
	PKA_ALIGN(addr, 8);		/* Align 8-byte */

	/* Set PKA RAM address for vectB and load input B - curve P value */
	APM_PKA_CSR_WRITE_RETURN(PKA_BLENGTH_ADDR, vect_cnt);
	APM_PKA_CSR_WRITE_RETURN(PKA_BPTR_ADDR, addr >> 2);
	for(i = 0; i < vect_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, curve_p[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X  Curve_p Val  0x%08X\n", addr, val32);
	}
	addr += (extra_dword * 4); 	/* Require extra DWORD */
	PKA_ALIGN(addr, 8);		/* Align 8-byte */

	/* Load input B - curve a value */
	for(i = 0; i < vect_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, curve_a[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X Curve_a Val  0x%08X\n", addr, val32);
	}
	addr += (extra_dword * 4); 	/* Require extra DWORD */
	PKA_ALIGN(addr, 8);		/* Align 8-byte */

	/* Set PKA RAM address for vectC and load input C - vectC_x co-ordinate value */
	APM_PKA_CSR_WRITE_RETURN(PKA_CPTR_ADDR, addr >> 2);
	for(i = 0; i < vect_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, vectC_x[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X  vectC_X Val  0x%08X\n", addr, val32);
	}
	addr += (extra_dword * 4); 	/* Require extra DWORD */
	PKA_ALIGN(addr, 8);		/* Align 8-byte */

        /* Load input C - vectC_y co-ordinate value */
        for(i = 0; i < vect_cnt; i++, addr += 4) {
            APM_PKA_CSR_WRITE_RETURN(addr, vectC_y[i]);
            APM_PKA_CSR_READ_RETURN(addr, &val32);
            DEBUG("ADDR  0x%08X  vectC_Y Val  0x%08X\n", addr, val32);
        }
        addr += (extra_dword * 4); 	/* Require extra DWORD */
        PKA_ALIGN(addr, 8); 		/* Align 8-byte */

	/* Set PKA RAM address for output D - result */
	APM_PKA_CSR_WRITE_RETURN(PKA_DPTR_ADDR, addr >> 2);

	return addr;
}

u32 apm_pka_ecc_mul_set_vec(u32 vectA_cnt,
			    u32 *scalar_k,
			    u32 vectB_cnt,
			    u32 *curve_p,
			    u32 *curve_a,
			    u32 *curve_b,
			    u32 *vectC_x,
			    u32 *vectC_y)
{
	u32 addr;
	u32 i, rc;
	u32 val32;
        u32 extra_dword;

        if (vectB_cnt % 2)
		extra_dword = 3;
        else
		extra_dword = 2;

	addr = PKA_RAM_ADDR;

	/* Set PKA RAM address for vectA and load input A - scalar k value */
	APM_PKA_CSR_WRITE_RETURN(PKA_ALENGTH_ADDR, vectA_cnt);
	APM_PKA_CSR_WRITE_RETURN(PKA_APTR_ADDR, addr >> 2);
	for (i = 0; i < vectA_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, scalar_k[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X  Scalar_k Val 0x%08X\n", addr, val32);
	}
        if (vectB_cnt % 2)
             addr += 4;		/* require 1 extra word if vectB len is odd */
	PKA_ALIGN(addr, 8);	/* Align 8-byte */

	/* Set PKA RAM address for vectB and load input B - curve p value */
	APM_PKA_CSR_WRITE_RETURN(PKA_BLENGTH_ADDR, vectB_cnt);
	APM_PKA_CSR_WRITE_RETURN(PKA_BPTR_ADDR, addr >> 2);
	for(i = 0; i < vectB_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, curve_p[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X  Curve_p Val  0x%08X\n", addr, val32);
	}
	addr += (extra_dword * 4); 	/* Require extra DWORD */
	PKA_ALIGN(addr, 8);		/* Align 8-byte */

	/*Load input B - curve a value */
	for(i = 0; i < vectB_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, curve_a[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X  Curve_a Val  0x%08X\n", addr, val32);
	}
	addr += (extra_dword * 4); 	/* Require extra DWORD */
	PKA_ALIGN(addr, 8);		/* Align 8-byte */

	/*Load input B - curve b value */
	for(i = 0; i < vectB_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, curve_b[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
                DEBUG("ADDR  0x%08X  Curve_b Val  0x%08X\n", addr, val32);
	}
	addr += (extra_dword * 4); 	/* Require extra DWORD */
	PKA_ALIGN(addr, 8);		/* Align 8-byte */


	/* Set PKA RAM address for vectC and load input C -vectC_x co-ordinate value */
	APM_PKA_CSR_WRITE_RETURN(PKA_CPTR_ADDR, addr >> 2);
	for(i = 0; i < vectB_cnt; i++, addr += 4) {
		APM_PKA_CSR_WRITE_RETURN(addr, vectC_x[i]);
		APM_PKA_CSR_READ_RETURN(addr, &val32);
		DEBUG("ADDR  0x%08X vectC_X Val  0x%08X\n", addr, val32);
	}
	addr += (extra_dword * 4); 	/* Require extra DWORD */
	PKA_ALIGN(addr, 8);		/* Align 8-byte */

        /* Load input C -vectC_y co-ordinate value */
        for(i = 0; i < vectB_cnt; i++, addr += 4) {
            APM_PKA_CSR_WRITE_RETURN(addr, vectC_y[i]);
            APM_PKA_CSR_READ_RETURN(addr, &val32);
            DEBUG("ADDR  0x%08X vectC_Y Val  0x%08X\n", addr, val32);
        }
        addr += (extra_dword * 4);  	/* Require extra DWORD */
        PKA_ALIGN(addr, 8); 		/* Align 8-byte */

	/* Set PKA RAM address for output D - result */
	APM_PKA_CSR_WRITE_RETURN(PKA_DPTR_ADDR, addr >> 2);

	return addr;
}


void apm_pka_process_completed_event(struct apm_pka_op *op)
{
	int status = 0;
	apm_pka_cb callback = NULL;

	callback = op->cb;
	op->cb = NULL;
	if (callback)
		(*callback)(op->ctx, status);
}

void apm_pka_tasklet_cb(unsigned long data)
{
	struct list_head *pos;
	struct list_head *tmp;

	unsigned long flags;

	spin_lock_irqsave(&pka_dev.ctx.lock, flags);

	list_for_each_safe(pos, tmp, &pka_dev.ctx.completed_event_queue) {
		struct apm_pka_op *item;
		item = list_entry(pos, struct apm_pka_op, next);
		list_del(pos);
		spin_unlock_irqrestore(&pka_dev.ctx.lock,flags);
		apm_pka_process_completed_event(item);
		spin_lock_irqsave(&pka_dev.ctx.lock, flags);
	}

	spin_unlock_irqrestore(&pka_dev.ctx.lock,flags);
}

static u8 apm_pka_pending_op(void)
{
	u32 retval;
	struct apm_pka_ctx *ctx;
	unsigned long flags;

	ctx = &pka_dev.ctx;

	spin_lock_irqsave(&ctx->op_lock, flags);
	retval = pka_dev.ctx.op_head != pka_dev.ctx.op_tail;
	spin_unlock_irqrestore(&ctx->op_lock, flags);

	return retval;
}

static struct apm_pka_op * apm_pka_get_op_item(void)
{
	u32 tail;
	unsigned long flags;
	struct apm_pka_op *op;

	spin_lock_irqsave(&pka_dev.ctx.op_lock, flags);

	if (pka_dev.ctx.op_tail == APM_PKA_PENDING_OP_MAX-1)
		tail = 0;
	else
		tail = pka_dev.ctx.op_tail + 1;

	if (tail == pka_dev.ctx.op_head) {
		printk(KERN_ERR "No free descriptor available for operation "
				 "queuing\n");
		return NULL;
	}

	op = &pka_dev.ctx.op[pka_dev.ctx.op_tail];

	spin_unlock_irqrestore(&pka_dev.ctx.op_lock, flags);

	return op;
}

static int apm_pka_start_op(struct apm_pka_op *op, int interrupt_mode)
{
	int	rc;
	u8 	restart = 0;
	u32     Daddr;
	u32     Caddr, addr;
	u32     val32;
	unsigned long flags;

	spin_lock_irqsave(&pka_dev.ctx.op_lock, flags);
	if (!interrupt_mode) {
		/* Not calling from interrupt context, check if pending op */
		restart = !(pka_dev.ctx.op_head != pka_dev.ctx.op_tail);

		if (pka_dev.ctx.op_tail == (u16) (APM_PKA_PENDING_OP_MAX-1))
			pka_dev.ctx.op_tail = 0;
		else
			pka_dev.ctx.op_tail++;
	}
	spin_unlock_irqrestore(&pka_dev.ctx.op_lock, flags);

	if (restart || interrupt_mode) {
		switch(op->opcode) {
		case 0:	/* Canceled */
			return 0;
		case PKA_FUNCTION_DIV:
			/* Passing to apm_pka_div_set_vec the order of
			 * dividend_cnt, dividend, divisor_cnt, divisor
			 */
			DEBUG("Starting async Div PKA operation\n");
			Caddr = apm_pka_pkcp_set_vec(op->async_pkcp.vecA_cnt,
						     op->async_pkcp.vecA,
						     op->async_pkcp.vecB_cnt,
						     op->async_pkcp.vecB);
			op->ramC_addr = Caddr;
			addr = Caddr;
			addr += (op->async_pkcp.vecB_cnt + 1) * 4;
			PKA_ALIGN(addr, 8);	/* Align 8-byte */
			/* Select PKA RAM address for output D - quotient */
			APM_PKA_CSR_WRITE_RETURN(PKA_DPTR_ADDR, addr >> 2);
			APM_PKA_CSR_READ_RETURN(PKA_DPTR_ADDR, &val32);
			op->ramD_addr = addr;
			break;
		case PKA_FUNCTION_MUL:
		case PKA_FUNCTION_MOD:
		case PKA_FUNCTION_ADD:
		case PKA_FUNCTION_SUB:
		case PKA_FUNCTION_COMPARE:
			Caddr = apm_pka_pkcp_set_vec(op->async_pkcp.vecA_cnt,
						     op->async_pkcp.vecA,
						     op->async_pkcp.vecB_cnt,
						     op->async_pkcp.vecB);
			op->ramC_addr = Caddr;
			break;
		case PKA_FUNCTION_ADDSUB:
			DEBUG("Starting async ADDSUB PKA operation\n");
			Daddr = apm_pka_addsub_set_vec(op->async_pkcp.vecA_cnt,
						       op->async_pkcp.vecA,
						       op->async_pkcp.vecB,
						       op->async_pkcp.vec_addsub_C);
			op->ramD_addr = Daddr;
			break;
		case PKA_FUNCTION_RSHIFT:
		case PKA_FUNCTION_LSHIFT:
			Caddr = apm_pka_shift_set_vec(op->async_pkcp.vecA_cnt,
						      op->async_pkcp.vecA,
						      op->async_pkcp.shift_val);
			op->ramC_addr = Caddr;
			break;
		case PKA_FUNCTION_SEQOP_EXPMOD_ACT2:
		case PKA_FUNCTION_SEQOP_EXPMOD_ACT4:
		case PKA_FUNCTION_SEQOP_EXPMOD_VAR:
			Daddr = apm_pka_expmod_set_vec(op->async_expmod.base_mod_cnt,
						       op->async_expmod.base,
						       op->async_expmod.modulus,
						       op->async_expmod.exp_cnt,
						       op->async_expmod.exp);
			op->ramD_addr = Daddr;
			break;
		case PKA_FUNCTION_SEQOP_EXPMOD_CRT:
			/* No pending operation before adding this operation
			* id restart = 1
			*/
			Daddr = apm_pka_expmod_crt_set_vec(op->async_expmod_crt.exp_len,
							   op->async_expmod_crt.expP,
   							   op->async_expmod_crt.expQ,
							   op->async_expmod_crt.mod_inverse_len,
							   op->async_expmod_crt.modP,
							   op->async_expmod_crt.modQ,
							   op->async_expmod_crt.inverseQ,
							   op->async_expmod_crt.input);
			op->ramD_addr = Daddr;
			break;
		case PKA_FUNCTION_COPY:
			Caddr = apm_pka_copy_set_vec(op->async_pkcp.vecA_cnt,
						     op->async_pkcp.vecA);
			op->ramC_addr = Caddr;
			break;
		case PKA_FUNCTION_SEQOP_MODINV:
			Daddr = apm_pka_mod_inv_set_vec(op->async_mod_inv.num_cnt,
							op->async_mod_inv.num,
							op->async_mod_inv.mod_cnt,
							op->async_mod_inv.mod);
			op->ramD_addr = Daddr;
			break;
		case PKA_FUNCTION_SEQOP_ECC_ADD:
			Daddr = apm_pka_ecc_add_set_vec(op->async_ecc_add.vect_cnt,
							op->async_ecc_add.vectA_x,
							op->async_ecc_add.vectA_y,
							op->async_ecc_add.curve_p,
							op->async_ecc_add.curve_a,
							op->async_ecc_add.vectC_x,
							op->async_ecc_add.vectC_y);
			op->ramD_addr = Daddr;
			break;
		case PKA_FUNCTION_SEQOP_ECC_MUL:
			Daddr = apm_pka_ecc_mul_set_vec(op->async_ecc_mul.vectA_cnt,
							op->async_ecc_mul.scalar_k,
							op->async_ecc_mul.vectB_cnt,
							op->async_ecc_mul.curve_p,
							op->async_ecc_mul.curve_a,
							op->async_ecc_mul.curve_b,
							op->async_ecc_mul.vectC_x,
							op->async_ecc_mul.vectC_y);
			op->ramD_addr = Daddr;
			break;
		default:
			printk(KERN_ERR "No operation in async mode\n");
			return 0;
		}
		if (op->opcode == PKA_FUNCTION_SEQOP_EXPMOD_VAR ||
			op->opcode == PKA_FUNCTION_SEQOP_EXPMOD_CRT) {
			APM_PKA_CSR_WRITE_RETURN(PKA_SHIFT_ADDR, op->resultC_cnt);
		}
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
					PKA_FUNCTION_RUN | op->opcode);
	}
	return 0;
}

irqreturn_t apm_pka_irq_handler(int irq, void * id)
{
	int	rc;
	u32     i;
	u32	val;
	struct apm_pka_op *op;
	struct apm_pka_op *next_op;
	unsigned long flags;

	if (!apm_pka_pending_op()) {
		DEBUG("No pending op in apm_pka_irq_handler !!\n");
		return 0;
	}

	spin_lock_irqsave(&pka_dev.ctx.op_lock, flags);
	op = &pka_dev.ctx.op[pka_dev.ctx.op_head];
	spin_unlock_irqrestore(&pka_dev.ctx.op_lock, flags);
	switch(op->opcode) {
	case 0:	/* Canceled */
		op->cb = NULL;
		break;
	case PKA_FUNCTION_COMPARE:
		APM_PKA_CSR_READ_RETURN(PKA_COMPARE_ADDR, &val);
		if (val & PKA_COMPARE_EQUAL)
			*op->resultC_addr = 0;
		else if (val & PKA_COMPARE_LESSTHAN)
			*op->resultC_addr = -1;
		else
			*op->resultC_addr = 1;
		break;
	case PKA_FUNCTION_SEQOP_EXPMOD_ACT2:
	case PKA_FUNCTION_SEQOP_EXPMOD_ACT4:
	case PKA_FUNCTION_SEQOP_EXPMOD_VAR:
	case PKA_FUNCTION_SEQOP_EXPMOD_CRT:
        case PKA_FUNCTION_SEQOP_MODINV:
        case PKA_FUNCTION_ADDSUB:
		for (i = 0; i < op->resultD_cnt; op->ramD_addr += 4, i++)
			apm_pka_readl(op->ramD_addr,
					&op->resultD_addr[i]);
		break;
	case PKA_FUNCTION_DIV:
		for (i = 0; i < op->resultC_cnt; op->ramC_addr += 4)
			apm_pka_readl(op->ramC_addr,
					&op->resultC_addr[i++]);
		for (i = 0; i < op->resultD_cnt; op->ramD_addr += 4)
			apm_pka_readl(op->ramD_addr,
					&op->resultD_addr[i++]);
		break;
	case PKA_FUNCTION_SEQOP_ECC_ADD:
        case PKA_FUNCTION_SEQOP_ECC_MUL:
		for (i = 0; i < op->resultD_cnt; op->ramD_addr += 4, i++)
			apm_pka_readl(op->ramD_addr,
					&op->result_ecc_x_addr[i]);
                op->ramD_addr += ((op->resultD_cnt % 2) ? 3:2) * 4;
                for (i = 0; i < op->resultD_cnt; op->ramD_addr += 4, i++)
			apm_pka_readl(op->ramD_addr,
					&op->result_ecc_y_addr[i]);
		break;
	default:
		for (i = 0; i < op->resultC_cnt; op->ramC_addr += 4)
		apm_pka_readl(op->ramC_addr,
					&op->resultC_addr[i++]);
		break;
	}

	spin_lock_irqsave(&pka_dev.ctx.op_lock, flags);
	/* Add complete operation to completed event queue */
	list_add_tail(&op->next, &pka_dev.ctx.completed_event_queue);

	/* Get next operation */
	if (pka_dev.ctx.op_head == APM_PKA_PENDING_OP_MAX - 1)
		pka_dev.ctx.op_head = 0;
	else
		pka_dev.ctx.op_head = (pka_dev.ctx.op_head + 1) %
						APM_PKA_PENDING_OP_MAX;
	next_op = &pka_dev.ctx.op[pka_dev.ctx.op_head];
	spin_unlock_irqrestore(&pka_dev.ctx.lock,flags);

	/* Check if there is actual next operation */
	if (!apm_pka_pending_op()) {
		DEBUG("No pending op in apm_pka_irq_handler\n");
		tasklet_schedule(&pka_dev.ctx.tasklet);
		return IRQ_HANDLED;
	}
	/* Start next operation */
	apm_pka_start_op(next_op, 1);

	/* Schedule tasklet to process completed operation */
	tasklet_schedule(&pka_dev.ctx.tasklet);

	return IRQ_HANDLED;
}

static int apm_pka_wait2complete(void)
{
	int rc;
	u32 val;
	u32 tried = 0;

	do {
		udelay(1);
		APM_PKA_CSR_READ_RETURN(PKA_FUNCTION_ADDR, &val);
		if (!(val & PKA_FUNCTION_RUN)) {
			return 0;
		}
		tried++;
	} while (tried < APM_PKA_POLL_DONE_MAX_CNT);

	DEBUG("Returning busy after tried count = %d\n", tried);
	return -EBUSY;
}

int  apm_pka_mul(apm_pka_cb cb, void *ctx, u32 *op_id,
		 u32 multiplicand_cnt, u32 *multiplicand,
		 u32 multiplier_cnt, u32 *multiplier,
		 u32 *product)
{
	int  rc;
	u32 addr;		/* Address of PKA RAM */
	struct apm_pka_op *pka_op;
	u32 i;

	if (multiplicand_cnt > APM_PKA_VECTOR_MAXSIZE ||
	    multiplier_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	/* Save callback for asynchronous operation */
	if (!cb) {
		addr = apm_pka_pkcp_set_vec(multiplicand_cnt, multiplicand,
					    multiplier_cnt, multiplier);
		/* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
					PKA_FUNCTION_RUN | PKA_FUNCTION_MUL);
		rc = apm_pka_wait2complete();

		if (rc)
			return rc;
		multiplicand_cnt += multiplier_cnt;
		for (i = 0; i < multiplicand_cnt; i++) {
			APM_PKA_CSR_READ_RETURN(addr, &product[i]);
			DEBUG("Result ADDR  0x%08X  Product Val  0x%08X\n",
				addr, product[i]);
			addr += 4;
		}
		return 0;
	}
	/* Asynchronous operation */
	pka_op->opcode	       	    = PKA_FUNCTION_MUL;
	pka_op->cb             	    = cb;
	pka_op->ctx            	    = ctx;
	pka_op->resultC_cnt    	    = multiplicand_cnt + multiplier_cnt;
	pka_op->resultC_addr   	    = product;
	pka_op->async_pkcp.vecA_cnt = multiplicand_cnt;
	pka_op->async_pkcp.vecA     = multiplicand;
	pka_op->async_pkcp.vecB_cnt = multiplier_cnt;
	pka_op->async_pkcp.vecB     = multiplier;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);
	return -EINPROGRESS;
}

int  apm_pka_div(apm_pka_cb cb, void *ctx, u32 *op_id,
		 u32 dividend_cnt, u32 *dividend,
		 u32 divisor_cnt, u32 *divisor,
		 u32 *remainder, u32 *quotient)
{
	int  rc;
	u32 addr;		/* Address of PKA RAM */
	struct apm_pka_op *pka_op;
	u32 resultC_addr;
	u32 resultD_addr;
	u32 i;

	if (dividend_cnt > APM_PKA_VECTOR_MAXSIZE ||
	    divisor_cnt > APM_PKA_VECTOR_MAXSIZE ||
	    divisor_cnt > dividend_cnt)
		return -EINVAL;
	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	/* Save callback for asynchronous operation */
	if (!cb) {
		resultC_addr = apm_pka_pkcp_set_vec(dividend_cnt, dividend,
						    divisor_cnt, divisor);
		addr = resultC_addr;
		addr += (divisor_cnt + 1) * 4;
		PKA_ALIGN(addr, 8);	/* Align 8-byte */
		/* Select PKA RAM address for output D - quotient */
		APM_PKA_CSR_WRITE_RETURN(PKA_DPTR_ADDR, addr >> 2);
		resultD_addr = addr;

		/* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
					PKA_FUNCTION_RUN | PKA_FUNCTION_DIV);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;
		for (i = 0; i < divisor_cnt; i++) {
			APM_PKA_CSR_READ_RETURN(resultC_addr, &remainder[i]);
                        DEBUG("Result ADDR  0x%08X  Remainder Val  0x%08X\n",
				resultC_addr, remainder[i]);
			resultC_addr += 4;
		}
		dividend_cnt -= divisor_cnt;
		for (i = 0; i <= dividend_cnt /* Use = for + 1 */; i++ ) {
			APM_PKA_CSR_READ_RETURN(resultD_addr,
					&quotient[i]);
                        DEBUG("Result ADDR  0x%08X  Quotient Val  0x%08X\n",
				resultD_addr, quotient[i]);
			resultD_addr += 4;
		}

		return 0;
	}
	/* Setting params for Asynchronous operation */
	pka_op->opcode	       	    = PKA_FUNCTION_DIV;
	pka_op->cb            	    = cb;
	pka_op->ctx           	    = ctx;
	pka_op->resultC_cnt  	    = divisor_cnt;
	pka_op->resultD_cnt  	    = dividend_cnt - divisor_cnt + 1;
	pka_op->resultC_addr  	    = remainder;
	pka_op->resultD_addr   	    = quotient;
	pka_op->async_pkcp.vecA_cnt = dividend_cnt;
	pka_op->async_pkcp.vecA     = dividend;
	pka_op->async_pkcp.vecB_cnt = divisor_cnt;
	pka_op->async_pkcp.vecB     = divisor;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);
	return -EINPROGRESS;
}

int  apm_pka_mod(apm_pka_cb cb, void *ctx, u32 *op_id,
		 u32 dividend_cnt, u32 *dividend,
		 u32 divisor_cnt, u32 *divisor,
		 u32 *remainder)
{
	int  rc;
	u32 addr;		/* Address of PKA RAM */
	struct apm_pka_op *pka_op;
	u32 i;

	if (dividend_cnt > APM_PKA_VECTOR_MAXSIZE ||
	    divisor_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	/* Save callback for asynchronous operation */
	if (!cb) {
		addr = apm_pka_pkcp_set_vec(dividend_cnt, dividend,
					    divisor_cnt, divisor);
		/* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
					PKA_FUNCTION_RUN | PKA_FUNCTION_MOD);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;
		for (i = 0; i < divisor_cnt; i++ ) {
			APM_PKA_CSR_READ_RETURN(addr, &remainder[i]);
                        DEBUG("Result ADDR  0x%08X  Remainder Val  0x%08X\n",
				addr, remainder[i]);
			addr += 4;
		}
		return 0;
	}
	/* Asynchronous operation */
	pka_op->opcode       	    = PKA_FUNCTION_MOD;
	pka_op->cb           	    = cb;
	pka_op->ctx           	    = ctx;
	pka_op->resultC_cnt    	    = divisor_cnt;
	pka_op->resultC_addr  	    = remainder;
	pka_op->async_pkcp.vecA_cnt = dividend_cnt;
	pka_op->async_pkcp.vecA     = dividend;
	pka_op->async_pkcp.vecB_cnt = divisor_cnt;
	pka_op->async_pkcp.vecB     = divisor;
	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);
	return -EINPROGRESS;
}

int  apm_pka_add(apm_pka_cb cb, void *ctx, u32 *op_id,
		 u32 addendA_cnt, u32 *addendA,
		 u32 addendB_cnt, u32 *addendB, u32 *sum)
{
	int  rc;
	u32 addr;		/* Address of PKA RAM */
	struct apm_pka_op *pka_op;
	u32 result_len;
	u32 i;

	if (addendA_cnt > APM_PKA_VECTOR_MAXSIZE ||
	    addendB_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	result_len   = addendA_cnt > addendB_cnt ? (addendA_cnt+1) :
						   (addendB_cnt+1);
	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	/* Save callback for asynchronous operation */
	if (!cb) {
		addr = apm_pka_pkcp_set_vec(addendA_cnt, addendA,
					    addendB_cnt, addendB);
		/* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
					PKA_FUNCTION_ADD | PKA_FUNCTION_RUN);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;
		for (i = 0; i < result_len; i++) {
			APM_PKA_CSR_READ_RETURN(addr, &sum[i]);
                        DEBUG("Result ADDR  0x%08X  Sum Val  0x%08X\n",
				addr, sum[i]);
			addr += 4;
		}
		return 0;
	}
	/* Asynchronous operation */
	pka_op->opcode       	    = PKA_FUNCTION_ADD;
	pka_op->cb             	    = cb;
	pka_op->ctx            	    = ctx;
	pka_op->resultC_cnt    	    = result_len;
	pka_op->resultC_addr   	    = sum;
	pka_op->async_pkcp.vecA_cnt = addendA_cnt;
	pka_op->async_pkcp.vecA     = addendA;
	pka_op->async_pkcp.vecB_cnt = addendB_cnt;
	pka_op->async_pkcp.vecB     = addendB;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);
	return -EINPROGRESS;
}

int  apm_pka_sub(apm_pka_cb cb, void *ctx, u32 *op_id,
		 u32 minuend_cnt, u32 *minuend,
		 u32 subtrahend_cnt, u32 *subtrahend,
		 u32 *difference)
{
	int  rc;
	u32 addr;		/* Address of PKA RAM */
	struct apm_pka_op *pka_op;
	u32 result_len;
	u32 i;

	if (minuend_cnt > APM_PKA_VECTOR_MAXSIZE ||
	    subtrahend_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	result_len   = minuend_cnt > subtrahend_cnt ? minuend_cnt :
						      subtrahend_cnt;
	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	/* Save callback for asynchronous operation */
	if (!cb) {
		addr = apm_pka_pkcp_set_vec(minuend_cnt, minuend,
					    subtrahend_cnt, subtrahend);
		/* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
					PKA_FUNCTION_SUB | PKA_FUNCTION_RUN);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;
		for (i = 0; i < result_len; i++) {
			APM_PKA_CSR_READ_RETURN(addr, &difference[i]);
                        DEBUG("Result ADDR  0x%08X  Difference Val  0x%08X\n",
				addr, difference[i]);
			addr += 4;
		}
		return 0;
	}
	/* Asynchronous operation */
	pka_op->opcode         	    = PKA_FUNCTION_SUB;
	pka_op->cb             	    = cb;
	pka_op->ctx            	    = ctx;
	pka_op->resultC_cnt   	    = result_len;
	pka_op->resultC_addr   	    = difference;
	pka_op->async_pkcp.vecA_cnt = minuend_cnt;
	pka_op->async_pkcp.vecA     = minuend;
	pka_op->async_pkcp.vecB_cnt = subtrahend_cnt;
	pka_op->async_pkcp.vecB     = subtrahend;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);

	return -EINPROGRESS;
}

int  apm_pka_addsub(apm_pka_cb cb, void *ctx, u32 *op_id,
		    u32 input_cnt, u32 *addendA,
		    u32 *addendC, u32 *subtrahend,
		    u32 *result)
{
	int  rc;
	u32 addr;		/* Address of PKA RAM */
	struct apm_pka_op * pka_op;
	u32 i;

	if (input_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	/* Save callback for asynchronous operation */
	if (!cb) {
		addr = apm_pka_addsub_set_vec(input_cnt, addendA,
					      subtrahend, addendC);
		/* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
					PKA_FUNCTION_ADDSUB | PKA_FUNCTION_RUN);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;
		for (i = 0; i <= input_cnt /* Use = for + 1 */; i++) {
			APM_PKA_CSR_READ_RETURN(addr, &result[i]);
                        DEBUG("Result ADDR  0x%08X  AddSub Val  0x%08X\n",
				addr, result[i]);
			addr += 4;
		}
		return 0;
	}
	/* Asynchronous operation */
	pka_op->opcode         		= PKA_FUNCTION_ADDSUB;
	pka_op->cb             		= cb;
	pka_op->ctx            		= ctx;
	pka_op->resultD_cnt    		= input_cnt+1;
	pka_op->resultD_addr  		= result;
	pka_op->async_pkcp.vecA_cnt 	= input_cnt;
	pka_op->async_pkcp.vecA 	= addendA;
	pka_op->async_pkcp.vecB_cnt 	= 0;
	pka_op->async_pkcp.vecB 	= subtrahend;
	pka_op->async_pkcp.vec_addsub_C = addendC;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);

	return -EINPROGRESS;
}

int  apm_pka_rshift(apm_pka_cb cb, void *ctx, u32 *op_id,
		    u32 input_cnt, u32 *input,
		    u8  shift, u32 *result)
{
	int  rc;
	u32 addr;		/* Address of PKA RAM */
	struct apm_pka_op *pka_op;
	u32 i;

	if (input_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	/* Save callback for asynchronous operation */
	if (!cb) {
		addr = apm_pka_shift_set_vec(input_cnt, input, shift);
		/* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
					PKA_FUNCTION_RSHIFT | PKA_FUNCTION_RUN);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;
		for (i = 0; i < input_cnt; i++) {
			APM_PKA_CSR_READ_RETURN(addr, &result[i]);
                        DEBUG("Result ADDR  0x%08X  RShift Val  0x%08X\n",
				addr, result[i]);
			addr += 4;
		}
		return 0;
	}
	/* Asynchronous operation */
	pka_op->opcode      	     = PKA_FUNCTION_RSHIFT;
	pka_op->cb           	     = cb;
	pka_op->ctx           	     = ctx;
	pka_op->resultC_cnt    	     = input_cnt;
	pka_op->resultC_addr  	     = result;
	pka_op->async_pkcp.vecA_cnt  = input_cnt;
	pka_op->async_pkcp.vecA	     = input;
	pka_op->async_pkcp.shift_val = shift;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);

	return -EINPROGRESS;
}

int  apm_pka_lshift(apm_pka_cb cb, void *ctx, u32 *op_id,
		    u32 input_cnt, u32 *input,
		    u8  shift, u32 *result)
{
	int  rc;
	u32 addr;		/* Address of PKA RAM */
	struct apm_pka_op *pka_op;
	u32 result_len;
	u32 i;

	if (input_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	result_len = shift == 0 ? input_cnt : (input_cnt + 1);
	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else  {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	/* Save callback for asynchronous operation */
	if (!cb) {
		addr = apm_pka_shift_set_vec(input_cnt, input, shift);
		/* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
					PKA_FUNCTION_LSHIFT | PKA_FUNCTION_RUN);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;
		for (i = 0; i < result_len; i++) {
			APM_PKA_CSR_READ_RETURN(addr, &result[i]);
                        DEBUG("Result ADDR  0x%08X  LShift Val  0x%08X\n",
				addr, result[i]);
			addr += 4;
		}
		return 0;
	}
	/* Asynchronous operation */
	pka_op->opcode        		= PKA_FUNCTION_LSHIFT;
	pka_op->cb            		= cb;
	pka_op->ctx            		= ctx;
	pka_op->resultC_cnt   		= result_len;
	pka_op->resultC_addr   		= result;
	pka_op->async_pkcp.vecA_cnt	= input_cnt;
	pka_op->async_pkcp.vecA 	= input;
	pka_op->async_pkcp.shift_val 	= shift;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);

	return -EINPROGRESS;
}

int  apm_pka_compare(apm_pka_cb cb, void *ctx, u32 *op_id,
		     u32 input1_cnt, u32 *input1,
		     u32 input2_cnt, u32 *input2,
		     int *result)
{
	int  rc;
	struct apm_pka_op *pka_op;
	u32 val;

	if (input1_cnt > APM_PKA_VECTOR_MAXSIZE ||
	    input2_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	/* Save callback for asynchronous operation */
	if (!cb) {
		apm_pka_pkcp_set_vec(input1_cnt, input1, input2_cnt,
				     input2);
		/* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
				PKA_FUNCTION_COMPARE | PKA_FUNCTION_RUN);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;
		APM_PKA_CSR_READ_RETURN(PKA_COMPARE_ADDR, &val);
		if (val & PKA_COMPARE_EQUAL) {
			*result = 0;
                        DEBUG("Compare Result input1 == input2\n");
                }
		else if (val & PKA_COMPARE_LESSTHAN) {
			*result = -1;
                        DEBUG("Compare Result input1 < input2\n");
                }
		else {
			*result = 1;
                        DEBUG("Compare Result input1 > input2\n");
                }
		return 0;
	}
	/* Asynchronous operation */
	pka_op->opcode        	    = PKA_FUNCTION_COMPARE;
	pka_op->cb             	    = cb;
	pka_op->ctx            	    = ctx;
	pka_op->resultC_cnt    	    = 1;
	pka_op->resultC_addr   	    = (u32 *)result;
	pka_op->async_pkcp.vecA_cnt = input1_cnt;
	pka_op->async_pkcp.vecA     = input1;
	pka_op->async_pkcp.vecB_cnt = input2_cnt;
	pka_op->async_pkcp.vecB     = input2;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);

	return -EINPROGRESS;
}

int  apm_pka_expmod(apm_pka_cb cb, void *ctx, u32 *op_id,
		    u8  odd_pwr_cnt, u32 base_mod_cnt,
		    u32 *base, u32 *modulus,
		    u32 exponent_cnt, u32 *exponent,
		    u32 *result)
{
	int  rc;
	u32 addr;		/* Address of PKA RAM */
	struct apm_pka_op * pka_op;
	u32 cmd;
	u32 i;

	if (odd_pwr_cnt > 16 || odd_pwr_cnt == 0 ||
	    base_mod_cnt > APM_PKA_VECTOR_MAXSIZE ||
	    exponent_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	/* Start the operation */
	if (odd_pwr_cnt == 2) {
		cmd = PKA_FUNCTION_SEQOP_EXPMOD_ACT2;
	} else if (odd_pwr_cnt == 8) {
		cmd = PKA_FUNCTION_SEQOP_EXPMOD_ACT4;
	} else {
		APM_PKA_CSR_WRITE_RETURN(PKA_SHIFT_ADDR, odd_pwr_cnt);
		cmd = PKA_FUNCTION_SEQOP_EXPMOD_VAR;
	}
	/* Save callback for asynchronous operation */
	if (!cb) {
		addr = apm_pka_expmod_set_vec(base_mod_cnt, base, modulus,
					      exponent_cnt, exponent);
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
				     cmd | PKA_FUNCTION_RUN);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;
		for (i = 0; i < base_mod_cnt; i++) {
			APM_PKA_CSR_READ_RETURN(addr, &result[i]);
			DEBUG("Result ADDR  0x%08X  ExpMod Val  0x%08X\n",
				addr, result[i]);
			addr += 4;
		}
		return 0;

	}
	/* Asynchronous operation */
	pka_op->opcode        		  = cmd;
	pka_op->cb             		  = cb;
	pka_op->ctx                       = ctx;
	pka_op->resultC_cnt               = odd_pwr_cnt; /* Save odd power cnt in here */
	pka_op->resultD_cnt    		  = base_mod_cnt;
	pka_op->resultC_addr              = NULL;
	pka_op->resultD_addr  		  = result;
	pka_op->async_expmod.base 	  = base;
	pka_op->async_expmod.exp 	  = exponent;
	pka_op->async_expmod.modulus 	  = modulus;
	pka_op->async_expmod.base_mod_cnt = base_mod_cnt;
	pka_op->async_expmod.exp_cnt 	  = exponent_cnt;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);

	return -EINPROGRESS;
}

int apm_pka_expmod_crt(apm_pka_cb cb, void *ctx, u32 *op_id,
		       u8  odd_pwr_cnt, u32 exp_len,
		       u32 *expP, u32 *expQ, u32 mod_inverse_len,
		       u32 *modP, u32 *modQ, u32 *inverseQ,
		       u32 *input, u32 *result)
{
	int  rc;
	struct apm_pka_op *pka_op;
	u32 i;
        u32 Daddr;

	if (exp_len > APM_PKA_VECTOR_MAXSIZE ||
	    mod_inverse_len > APM_PKA_VECTOR_MAXSIZE ||
	    odd_pwr_cnt > 16)
		return -EINVAL;

	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	if (!cb) {
		Daddr = apm_pka_expmod_crt_set_vec(exp_len, expP, expQ,
						   mod_inverse_len,
						   modP, modQ,
						   inverseQ, input);
	} else {
		/* Asynchronous operation */
		pka_op->opcode         			 = PKA_FUNCTION_SEQOP_EXPMOD_CRT;
		pka_op->cb             			 = cb;
		pka_op->ctx            			 = ctx;
		pka_op->resultD_cnt    			 = mod_inverse_len << 1;
		pka_op->resultC_cnt    			 = odd_pwr_cnt; /* Use result C cnt for pwr cnt */
		pka_op->resultD_addr   			 = result;
		pka_op->async_expmod_crt.expP		 = expP;
		pka_op->async_expmod_crt.expQ 		 = expQ;
		pka_op->async_expmod_crt.modP 		 = modP;
		pka_op->async_expmod_crt.modQ		 = modQ;
		pka_op->async_expmod_crt.inverseQ 	 = inverseQ;
		pka_op->async_expmod_crt.exp_len 	 = exp_len;
		pka_op->async_expmod_crt.mod_inverse_len = mod_inverse_len;
		pka_op->async_expmod_crt.input 		 = input;
	}

	/* Save callback for asynchronous operation */
	if (!cb) {
		/* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_SHIFT_ADDR, odd_pwr_cnt);
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
			PKA_FUNCTION_SEQOP_EXPMOD_CRT | PKA_FUNCTION_RUN);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;
		for (i = 0; i < (mod_inverse_len << 1); i++) {
			APM_PKA_CSR_READ_RETURN(Daddr, &result[i]);
                        DEBUG("Result ADDR  0x%08X  ExpMod CRT Val  0x%08X\n",
				Daddr, result[i]);
			Daddr += 4;
		}
		return 0;
	}

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);

	return -EINPROGRESS;
}

int  apm_pka_copy(apm_pka_cb cb, void *ctx, u32 *op_id,
		  u32 input_cnt, u32 *input, u32 *result)
{
	int  rc;
	u32 addr;		/* Address of PKA RAM */
	struct apm_pka_op *pka_op;
	u32 i;

	if (input_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	/* Save callback for asynchronous operation */
	if (!cb) {
		addr = apm_pka_copy_set_vec(input_cnt, input);
		/* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
			PKA_FUNCTION_COPY | PKA_FUNCTION_RUN);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;
		for (i = 0; i < input_cnt; i++) {
			APM_PKA_CSR_READ_RETURN(addr, &result[i]);
                        DEBUG("Result ADDR  0x%08X  Copy Val  0x%08X\n",
				addr, result[i]);
                        addr += 4;
		}
		return 0;
	}
	/* Asynchronous operation */
	pka_op->opcode              = PKA_FUNCTION_COPY;
	pka_op->cb                  = cb;
	pka_op->ctx                 = ctx;
	pka_op->resultC_cnt         = input_cnt;
	pka_op->resultC_addr        = result;
	pka_op->async_pkcp.vecA     = input;
	pka_op->async_pkcp.vecA_cnt = input_cnt;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);
	return -EINPROGRESS;
}

int apm_pka_mod_inv(apm_pka_cb cb, void *ctx, u32 *op_id,
		    u32 num_cnt, u32 *num, u32 mod_cnt,
		    u32 *mod, u32 *result)
{
	int  rc;
        u32 val32;
	struct apm_pka_op *pka_op;
	u32 i;
        u32 Daddr;

	if (num_cnt > APM_PKA_VECTOR_MAXSIZE ||
	    mod_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	if (!cb) {
		Daddr = apm_pka_mod_inv_set_vec(num_cnt, num, mod_cnt, mod);

                /* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
			PKA_FUNCTION_SEQOP_MODINV | PKA_FUNCTION_RUN);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;

                /* check the PKA_SHIFT register for return info on operation's result */
                APM_PKA_CSR_READ_RETURN(PKA_SHIFT_ADDR, &val32);
                if(val32)
                     return -EINVAL;
                else {
		      for (i = 0; i < mod_cnt; i++) {
			         APM_PKA_CSR_READ_RETURN(Daddr, &result[i]);
                                 DEBUG("Result ADDR  0x%08X  ModInv Val  0x%08X\n",
					Daddr, result[i]);
			         Daddr += 4;
		      }
		      return 0;
                }
        }
	/* Asynchronous operation */
	pka_op->opcode         	      = PKA_FUNCTION_SEQOP_MODINV;
	pka_op->cb             	      = cb;
	pka_op->ctx           	      = ctx;
	pka_op->resultD_cnt    	      = mod_cnt;
	pka_op->resultD_addr   	      = result;
	pka_op->async_mod_inv.num     = num;
	pka_op->async_mod_inv.num_cnt = num_cnt;
	pka_op->async_mod_inv.mod     = mod;
	pka_op->async_mod_inv.mod_cnt = mod_cnt;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);

	return -EINPROGRESS;
}

int apm_pka_ecc_add(apm_pka_cb cb, void *ctx, u32 *op_id,
		    u32 vect_cnt, u32 *vectA_x, u32 *vectA_y,
		    u32 *curve_p, u32 *curve_a, u32 *vectC_x,
		    u32 *vectC_y, u32 *result_vect_x,
		    u32 *result_vect_y)
{
	int  rc;
        u32 val32;
	struct apm_pka_op *pka_op;
	u32 i;
        u32 Daddr;

	if (vect_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	if (!cb) {
		Daddr = apm_pka_ecc_add_set_vec(vect_cnt, vectA_x, vectA_y,
					        curve_p, curve_a, vectC_x,
					        vectC_y);

                /* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
			 PKA_FUNCTION_SEQOP_ECC_MUL | PKA_FUNCTION_RUN);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;

                /* check the PKA_SHIFT register for return info on operation's result */
                APM_PKA_CSR_READ_RETURN(PKA_SHIFT_ADDR, &val32);
                if(val32)
			return -EINVAL;
                else {
			for (i = 0; i < vect_cnt; i++) {
				APM_PKA_CSR_READ_RETURN(Daddr, &result_vect_x[i]);
				DEBUG("Result ADDR  0x%08X  ECC ADD Result X Val  0x%08X\n",
					Daddr, result_vect_x[i]);
			        Daddr += 4;
		        }
                        Daddr += ((vect_cnt % 2) ? 3 :2) * 4;
                        for (i = 0; i < vect_cnt; i++) {
			        APM_PKA_CSR_READ_RETURN(Daddr, &result_vect_y[i]);
                                DEBUG("Result ADDR  0x%08X  ECC ADD Result Y Val  0x%08X\n",
					Daddr, result_vect_y[i]);
			        Daddr += 4;
		        }
		        return 0;
                }
        }
	/* Asynchronous operation */
	pka_op->opcode                 = PKA_FUNCTION_SEQOP_ECC_MUL;
	pka_op->cb                     = cb;
	pka_op->ctx                    = ctx;
	pka_op->resultD_cnt    	       = vect_cnt;
	pka_op->result_ecc_x_addr      = result_vect_x;
	pka_op->result_ecc_y_addr      = result_vect_y;
	pka_op->async_ecc_add.vectA_x  = vectA_x;
	pka_op->async_ecc_add.vectA_y  = vectA_y;
	pka_op->async_ecc_add.curve_p  = curve_p;
	pka_op->async_ecc_add.curve_a  = curve_a;
	pka_op->async_ecc_add.vect_cnt = vect_cnt;
	pka_op->async_ecc_add.vectC_x  = vectC_x;
	pka_op->async_ecc_add.vectC_y  = vectC_y;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);

	return -EINPROGRESS;
}

int apm_pka_ecc_mul(apm_pka_cb cb, void *ctx, u32 *op_id,
		    u32 vectA_cnt, u32 *scalar_k,
		    u32 vectB_cnt, u32 *curve_p,
		    u32 *curve_a, u32 *curve_b,
		    u32 *vectC_x, u32 *vectC_y,
		    u32 *result_vect_x,
		    u32 *result_vect_y)
{
	int  rc;
        u32 val32;
	struct apm_pka_op *pka_op;
	u32 i;
        u32 Daddr;

	if (vectA_cnt > APM_PKA_ECC_VECTOR_MAXSIZE ||
	    vectB_cnt > APM_PKA_VECTOR_MAXSIZE)
		return -EINVAL;

	if (cb == NULL) {
		if (apm_pka_pending_op())
			return -EBUSY;
		pka_op 	     = NULL;
	} else {
		pka_op = apm_pka_get_op_item();
		if (pka_op == NULL)
			return -EBUSY;
	}

	if (!cb) {
		Daddr = apm_pka_ecc_mul_set_vec(vectA_cnt, scalar_k,
					        vectB_cnt, curve_p, curve_a,
					        curve_b, vectC_x, vectC_y);
                /* Start the operation */
		APM_PKA_CSR_WRITE_RETURN(PKA_FUNCTION_ADDR,
			PKA_FUNCTION_SEQOP_ECC_MUL | PKA_FUNCTION_RUN);
		rc = apm_pka_wait2complete();
		if (rc)
			return rc;

               /* check the PKA_SHIFT register for return info on operation's result */
               APM_PKA_CSR_READ_RETURN(PKA_SHIFT_ADDR, &val32);
               if(val32)
                    return -EINVAL;
               else {
			for (i = 0; i < vectB_cnt; i++) {
			     APM_PKA_CSR_READ_RETURN(Daddr, &result_vect_x[i]);
                             DEBUG("Result ADDR  0x%08X  ECC MUL Result X Val  0x%08X\n",
					Daddr, result_vect_x[i]);
			     Daddr += 4;
		        }
                        Daddr += ((vectB_cnt % 2) ? 3 : 2) * 4;

                        for (i = 0; i < vectB_cnt; i++) {
			     APM_PKA_CSR_READ_RETURN(Daddr, &result_vect_y[i]);
                             DEBUG("Result ADDR  0x%08X  ECC MUL Result Y Val  0x%08X\n",
					Daddr, result_vect_y[i]);
			     Daddr += 4;
		        }
		        return 0;
               }
	}
	/* Asynchronous operation */
	pka_op->opcode                  = PKA_FUNCTION_SEQOP_ECC_MUL;
	pka_op->cb                      = cb;
	pka_op->ctx                     = ctx;
	pka_op->resultD_cnt   	        = vectB_cnt;
	pka_op->result_ecc_x_addr       = result_vect_x;
	pka_op->result_ecc_y_addr       = result_vect_y;
	pka_op->async_ecc_mul.scalar_k  = scalar_k;
	pka_op->async_ecc_mul.vectA_cnt = vectA_cnt;
	pka_op->async_ecc_mul.curve_p   = curve_p;
	pka_op->async_ecc_mul.curve_a   = curve_a;
	pka_op->async_ecc_mul.curve_b   = curve_b;
	pka_op->async_ecc_mul.vectB_cnt = vectB_cnt;
	pka_op->async_ecc_mul.vectC_x   = vectC_x;
	pka_op->async_ecc_mul.vectC_y   = vectC_y;

	if (op_id)
		*op_id = pka_op->id;
	apm_pka_start_op(pka_op, 0);

	return -EINPROGRESS;
}

int apm_pka_hw_init(void)
{
	int rc;
	u32 i;
	u32 prog_addr;

	DEBUG("Initializing PKA...\n");

	/* Initialize context variable */
	for (i = 0; i < APM_PKA_PENDING_OP_MAX; i++) {
		pka_dev.ctx.op[i].id     = i+1;
		pka_dev.ctx.op[i].opcode = 0;
	}

	INIT_LIST_HEAD(&pka_dev.ctx.completed_event_queue);

	/* Load PKA firmware */
	DEBUG("Loading PKA firmware PKA RAM Addr: 0x%08X size "
			"(DW): %d...\n", pka_dev.ctx.csr_paddr,
			pkt_firmware_sizedw);

	/* Put PKA Sequencer into reset to access firmware area */
	rc = apm_pka_writel(PKA_SEQ_CTRL_ADDR, PKA_SEQ_CTRL_RESET);
	if (rc != 0) {
		DEBUG("Failed to put PKA Sequencer into reset error 0x%08X\n", rc);
		return rc;
	}

	/* Now, load the firmware */
	prog_addr = PKA_PROGRAM_ADDR;
	for (i = 0; i < pkt_firmware_sizedw; i++, prog_addr += 4) {
		rc = apm_pka_writel(prog_addr, pka_firmware[i]);

		if (rc != 0) {
			DEBUG("Failed to load PKA firmware error 0x%08X\n", rc);
			return rc;
		}
	}

	/* Put PKA Sequencer into normal operation */
	rc = apm_pka_writel(PKA_SEQ_CTRL_ADDR, 0);
	if (rc != 0) {
		DEBUG("Failed to put PKA Sequencer into reset error 0x%08X\n",
			rc);
		return rc;
	}

	spin_lock_init(&pka_dev.ctx.op_lock);
	pka_dev.ctx.op_cnt = 0;

	/* Register for interrupt */
	tasklet_init(&pka_dev.ctx.tasklet,
		      apm_pka_tasklet_cb, (unsigned long) pka_dev.ctx.op);
	return 0;
}

int apm_pka_hw_deinit(void)
{
	if (pka_dev.ctx.irq != 0) {
		disable_irq(pka_dev.ctx.irq);
		free_irq(pka_dev.ctx.irq, NULL);
	}
	return 0;
}

#ifdef CONFIG_PM
static int apm_pka_suspend(struct platform_device * ofdev, pm_message_t state)
{
	if (state.event & PM_EVENT_FREEZE) {
		/* To hibernate */
		pka_dev.pwredoff = 1;
	} else if (state.event & PM_EVENT_SUSPEND) {
		/* To suspend */
	} else if (state.event & PM_EVENT_RESUME) {
		/* To resume */
	} else if (state.event & PM_EVENT_RECOVER) {
		/* To recover from enter suspend failure */
	}
	return 0;
}

static int apm_pka_resume(struct platform_device* ofdev)
{
	int rc = 0;

	if (pka_dev.pwredoff) {
		rc = pka_clk_init(&ofdev->dev);	/* Re-enable clock */
             if (rc)
                goto err;
		rc = apm_pka_hw_init();
             if (rc)
                goto err;
	     if (rc == 0)
		pka_dev.pwredoff = 0;
	}
err:
	return rc;
}
#else
#define apm_pka_suspend NULL
#define apm_pka_resume NULL
#endif /* CONFIG_PM */

/**
 * Setup Driver with platform registration
 */
static int apm_pka_probe(struct platform_device *ofdev)
{
	struct device_node *pka_np = ofdev->dev.of_node;
	int rc;

	rc = of_address_to_resource(pka_np, 0, &pka_dev.csr_res);
	if (rc) {
		dev_err(&ofdev->dev, "PKA Error: failed to get address to resource\n");
		return -ENODEV;
	}
	pka_dev.ctx.csr_paddr = pka_dev.csr_res.start;
	pka_dev.ctx.csr = ioremap_nocache(pka_dev.csr_res.start,
				pka_dev.csr_res.end - pka_dev.csr_res.start+1);
	if (pka_dev.ctx.csr == NULL) {
		dev_err(&ofdev->dev, "PKA Error: failed to ioremap 0x%02X_%08X size %d\n",
			(u32) (pka_dev.csr_res.start >> 32),
			(u32) pka_dev.csr_res.start,
			(u32) (pka_dev.csr_res.end - pka_dev.csr_res.start+1));
		return -ENOMEM;
	}

	pka_dev.ctx.irq = irq_of_parse_and_map(pka_np, 0);
	if (pka_dev.ctx.irq) {
		rc = request_irq(pka_dev.ctx.irq, apm_pka_irq_handler,
				 0, "PKA", NULL);
		if (rc) {
                	dev_err(&ofdev->dev, "PKA Error: failed to register PKA Interrupt Handler\n");
			goto err;
		}
		irq_set_irq_type(pka_dev.ctx.irq, IRQ_TYPE_EDGE_RISING);
	}

	pka_dev.pwredoff = 0;

	DEBUG("Applied Micro PKA v%s @0x%02X_%08X size %d IRQ %d\n",
		APM_PKA_VER_STR,
		(u32) (pka_dev.csr_res.start >> 32),
		(u32) pka_dev.csr_res.start,
		(u32) (pka_dev.csr_res.end - pka_dev.csr_res.start + 1),
                pka_dev.ctx.irq);

	rc = pka_clk_init(&ofdev->dev);
        if (rc)
                goto err;

	rc = apm_pka_hw_init();
	if (rc < 0) {
		dev_err(&ofdev->dev, "PKA Error: Failed to initialize PKA HW\n");
		goto err;
	}
	printk("Applied Micro PKA Driver Successfully Initialized\n");
	return rc;

err:
	iounmap(pka_dev.ctx.csr);
	return rc;
}

static int apm_pka_remove(struct platform_device *ofdev)
{
	apm_pka_hw_deinit();
	iounmap(pka_dev.ctx.csr);
	return 0;
}

static struct of_device_id apm_pka_match[] = {
	{ .compatible	= "apm,xgene-pka",},
	{ },
};
MODULE_DEVICE_TABLE(of, apm_pka_match);

static struct platform_driver apm_pka_driver = {
	.probe		= apm_pka_probe,
	.remove		= apm_pka_remove,
	.suspend	= apm_pka_suspend,
	.resume		= apm_pka_resume,
	.driver = {
		.name		= "xgene-pka",
		.owner 		= THIS_MODULE,
		.of_match_table = apm_pka_match,
	},
};

module_platform_driver(apm_pka_driver);

MODULE_DESCRIPTION("Applied Micro Public Key Accelerator");
MODULE_AUTHOR("Rameshwar Prasad Sahu <rsahu@apm.com>");
MODULE_LICENSE("GPL");

EXPORT_SYMBOL_GPL(apm_pka_mul);
EXPORT_SYMBOL_GPL(apm_pka_div);
EXPORT_SYMBOL_GPL(apm_pka_mod);
EXPORT_SYMBOL_GPL(apm_pka_add);
EXPORT_SYMBOL_GPL(apm_pka_sub);
EXPORT_SYMBOL_GPL(apm_pka_addsub);
EXPORT_SYMBOL_GPL(apm_pka_rshift);
EXPORT_SYMBOL_GPL(apm_pka_lshift);
EXPORT_SYMBOL_GPL(apm_pka_compare);
EXPORT_SYMBOL_GPL(apm_pka_copy);
EXPORT_SYMBOL_GPL(apm_pka_expmod);
EXPORT_SYMBOL_GPL(apm_pka_expmod_crt);
EXPORT_SYMBOL_GPL(apm_pka_mod_inv);
EXPORT_SYMBOL_GPL(apm_pka_ecc_add);
EXPORT_SYMBOL_GPL(apm_pka_ecc_mul);
