/*
 * APM X-Gene SoC Security Driver
 *
 * Copyright (c) 2014 Applied Micro Circuits Corporation.
 * All rights reserved. Loc Ho <lho@apm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This file defines header for the algorithms to use with Linux CryptoAPI.
 *
 */
#ifndef __XGENE_SEC_ALG_H__
#define __XGENE_SEC_ALG_H__

#include <linux/types.h>
#include <linux/crypto.h>
#include <crypto/hash.h>
#include "xgene_sec_sa.h"
#include "xgene_sec_tkn.h"

#define XGENE_SEC_CRYPTO_PRIORITY		300
#define XGENE_SEC_CRYPTO_PRIORITY_IPSEC		350

#define crypto_alg_to_xgene_sec_alg(x)  	\
		container_of(x, struct xgene_sec_alg, alg)

struct xgene_sec_req_ctx {
	struct sec_tkn_ctx *tkn;
	struct sec_sa_item *sa;

	/* Continuous Hashing */
	char result_buffer[64];
	int pkg_count;		/* Multi of 64 Bytes */
	int result_ptr_unavailable;
};

struct xgene_sec_alg {
	struct list_head entry;
	u32 type;
	union {
		struct crypto_alg cipher;
		struct ahash_alg hash;
	} u;
};

extern struct xgene_sec_alg xgene_sec_alg_tlb[];

#endif
