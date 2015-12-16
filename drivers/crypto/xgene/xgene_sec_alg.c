/*
 * APM X-Gene SoC Crypto Driver Algorithms Implementation
 *
 * Copyright (c) 2014 Applied Micro Circuits Corporation.
 * All rights reserved. Loc Ho <lho@amcc.com>
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
 * This file implements the Linux crypto algorithms for APM X-Gene SoC
 * security driver.
 */

#include <linux/rtnetlink.h>
#include <linux/ip.h>
#include <linux/scatterlist.h>
#include <crypto/internal/hash.h>
#include <crypto/aead.h>
#include <crypto/des.h>
#include <crypto/authenc.h>
#include "xgene_sec_alg.h"
#include "xgene_sec.h"

static int xgene_sec_hash_alg_init(struct crypto_tfm *tfm, unsigned int sa_len,
				   unsigned char ha, unsigned char dt)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	int rc;

	xgene_sec_session_init(session);
	rc = xgene_sec_create_sa_tkn_pool(session, sa_len, sa_len, 0,
					  TKN_BASIC_MAX_LEN);
	if (rc != 0)
		goto err_nomem;

	crypto_ahash_set_reqsize(__crypto_ahash_cast(tfm),
				 sizeof(struct xgene_sec_req_ctx));

	dev_dbg(session->ctx->dev,
		"alg %s init sa 0x%p sa_ptr 0x%p sa len %d\n",
		tfm->__crt_alg->cra_name, session->sa, session->sa->sa_ptr,
		sa_len);

	/* Setup SA */
	session->sa->sa_ptr->hash_alg    = ha;
	session->sa->sa_ptr->digest_type = dt;
	session->sa->sa_ptr->ToP	 = SA_TOP_HASH_OUTBOUND;
	return 0;

err_nomem:
	return rc;
}

static int xgene_sec_hash_init(struct ahash_request *req)
{
	struct xgene_sec_req_ctx *rctx = ahash_request_ctx(req);

	rctx->tkn = NULL;
	rctx->sa = NULL;
	rctx->pkg_count = 0;

	return 0;
}

static int xgene_sec_hash_update(struct ahash_request *req)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = ahash_request_ctx(req);
	struct sec_tkn_input_hdr *in_tkn;
	int ds = sec_sa_compute_digest_len(session->sa->sa_ptr->hash_alg,
			SA_DIGEST_TYPE_INNER);
	unsigned char new_tkn;
	int *sa_context;
	u64 sa_hwaddr;
	int rc = 0;

	if (rctx->sa == NULL) {
		rctx->sa = xgene_sec_sa_get(session);
		if (rctx->sa == NULL) {
			rc = -ENOMEM;
			goto err_nomem;
		}
		memcpy(rctx->sa->sa_ptr, session->sa->sa_ptr, session->sa_len);
		/*
		 * Now, reserve a token for this request and update fields as
		 * required.
		 */
		rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
		if (rctx->tkn == NULL) {
			rc = -ENOMEM;
			goto err_nomem;
		}
		rctx->tkn->flags = (u32)(TKN_FLAG_PKG_FIRST | TKN_FLAG_CONTINOUS);
	}

	if (rctx->tkn == NULL) {
		rc = -ENOMEM;
		goto err_nomem;
	}

	if (req->nbytes % (crypto_tfm_alg_blocksize(req->base.tfm))) {
		APMSEC_TXLOG("Input plaintext size not multiple of %dB",
				(crypto_tfm_alg_blocksize(req->base.tfm)));
		rctx->tkn->flags |= TKN_FLAG_PKG_FINAL;
	}

	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	sa_context = (u32 *) session->sa->sa_ptr;

	/* First package*/
	if (rctx->tkn->flags & (u32)TKN_FLAG_PKG_FIRST) {
		rctx->tkn->flags &= ~(u32)TKN_FLAG_PKG_FIRST;
		rctx->pkg_count = 0;
		session->sa->sa_ptr->pkt_based_options = SA_PBO_NOFINISH_HASH;
	} else { /* Update package*/
		session->sa->sa_ptr->digest_type = SA_DIGEST_TYPE_INNER;
		session->sa->sa_ptr->pkt_based_options = SA_PBO_NOFINISH_HASH;
		session->sa->sa_ptr->context_length = ds/4 + 1;
		session->sa->sa_ptr->digest_cnt = 1;
		rctx->tkn->flags |= (u32)TKN_FLAG_PKG_MIDDLE;
		sec_sa_set32le(&sa_context[ds/4 + 2], rctx->pkg_count);
	}

	/* Final package*/
	if (rctx->tkn->flags & (u32)TKN_FLAG_PKG_FINAL) {
		rctx->tkn->flags &= ~(u32)TKN_FLAG_CONTINOUS;
		session->sa->sa_ptr->pkt_based_options = 0;
	}

	if (req->result == NULL) {
		req->result = rctx->result_buffer;
		rctx->result_ptr_unavailable = 1;
	} else {
		rctx->result_ptr_unavailable = 0;
	}

	rctx->pkg_count += req->nbytes/64;

	rctx->tkn->sa = session->sa;
	rctx->tkn->context = &req->base;

	sa_hwaddr = (session->sa->sa_ptr->context_length == 0) ? 0 :
			SA_PTR_HW(xgene_sec_encode2hwaddr(
			SA_CONTEXT_PTR(session->sa->sa_hwptr)));

	if (session->sa->sa_ptr->hash_alg == SA_HASH_ALG_KASUMI_F9)
		rctx->tkn->input_tkn_len = sec_tkn_set_kasumi_f9(
			(u32 *) in_tkn,	req->nbytes,
			(u32 *) session->sa->sa_ptr, sa_hwaddr,
			crypto_ahash_digestsize(
				__crypto_ahash_cast(req->base.tfm)), 0);
	else
		rctx->tkn->input_tkn_len = sec_tkn_set_hash(
			(u32 *) in_tkn,	req->nbytes,
			(u32 *) session->sa->sa_ptr, sa_hwaddr,
			crypto_ahash_digestsize(
				__crypto_ahash_cast(req->base.tfm)), 0);

	BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);

	rc = xgene_sec_setup_crypto(session->ctx, &req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;
	else if (rc != 0)
		dev_err(session->ctx->dev, "hash digest failed error %d", rc);
	return 0;

err_nomem:
	if (rctx->tkn) {
		xgene_sec_tkn_free(session, rctx->tkn);
		rctx->tkn = NULL;
	}
	if (rctx->sa) {
		xgene_sec_sa_free(session, rctx->sa);
		rctx->sa = NULL;
	}
	return rc;
}

static int xgene_sec_hash_final(struct ahash_request *req)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = ahash_request_ctx(req);
	struct sec_tkn_input_hdr *in_tkn;
	int rc;
	int *sa_context;
	int ctx_length = 0;
	int ds = sec_sa_compute_digest_len(session->sa->sa_ptr->hash_alg,
			SA_DIGEST_TYPE_INNER);
	struct scatterlist sg;
	int tmp;
	u64 sa_hwaddr;

	if (rctx->tkn != NULL )
		/* Stop SEC engine and get the result */
		if ((rctx->tkn->flags & (u32)TKN_FLAG_PKG_FINAL) == 0) {
			sg_init_one(&sg, &tmp, 4);
			ctx_length = ds / 4 + 1;

			session->sa->sa_ptr->context_length = ctx_length;
			session->sa->sa_ptr->digest_cnt = 1;

			sa_context = (u32 *) session->sa->sa_ptr;
			sec_sa_set32le(&sa_context[ctx_length + 1], rctx->pkg_count);

			session->sa->sa_ptr->digest_type = SA_DIGEST_TYPE_INNER;
			session->sa->sa_ptr->pkt_based_options = 0;
			rctx->result_ptr_unavailable = 0;

			in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);

			rctx->tkn->sa = session->sa;
			rctx->tkn->context = &req->base;
			rctx->tkn->flags &= ~(u32)TKN_FLAG_CONTINOUS;
			req->src = &sg;
			req->nbytes = 4;

			sa_hwaddr = (session->sa->sa_ptr->context_length == 0) ? 0 :
					SA_PTR_HW(xgene_sec_encode2hwaddr(
					SA_CONTEXT_PTR(session->sa->sa_hwptr)));
			if (session->sa->sa_ptr->hash_alg == SA_HASH_ALG_KASUMI_F9)
				rctx->tkn->input_tkn_len = sec_tkn_set_kasumi_f9(
					(u32 *) in_tkn,	req->nbytes,
					(u32 *) session->sa->sa_ptr, sa_hwaddr,
					crypto_ahash_digestsize(
						__crypto_ahash_cast(
							req->base.tfm)), 1);
			else
				rctx->tkn->input_tkn_len = sec_tkn_set_hash(
					(u32 *) in_tkn,	req->nbytes,
					(u32 *) session->sa->sa_ptr, sa_hwaddr,
					crypto_ahash_digestsize(
						__crypto_ahash_cast(
							req->base.tfm)), 1);
			BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);

			rc = xgene_sec_setup_crypto(session->ctx, &req->base);
			if (rc == -EINPROGRESS || rc == -EAGAIN)
				return rc;
			else if (rc != 0)
				dev_err(session->ctx->dev, "hash digest failed error %d", rc);
		}

	if (rctx->result_ptr_unavailable ) {
		memcpy(req->result, rctx->result_buffer,
				crypto_ahash_digestsize(__crypto_ahash_cast(req->base.tfm)));
	}

	return 0;
}

static int xgene_sec_hash_digest(struct ahash_request *req)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = ahash_request_ctx(req);
	struct sec_tkn_input_hdr *in_tkn;
	unsigned char new_tkn;
	int rc;
	int nbytes;
	u64 sa_hwaddr;

	APMSEC_TXLOG("hash digest %s nbytes %d\n",
		     req->base.tfm->__crt_alg->cra_name, req->nbytes);

	rctx->sa  = NULL; /* No private SA */
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	rctx->tkn->flags = 0;
	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("alg %s tkn 0x%p outtkn 0x%p intkn 0x%p\n",
		req->base.tfm->__crt_alg->cra_name, rctx->tkn,
		TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	nbytes = req->nbytes ? req->nbytes :
			       crypto_tfm_alg_blocksize(req->base.tfm);
	if (new_tkn) {
		sa_hwaddr = (session->sa->sa_ptr->context_length == 0) ? 0 :
				SA_PTR_HW(xgene_sec_encode2hwaddr(
				SA_CONTEXT_PTR(session->sa->sa_hwptr)));
		if (session->sa->sa_ptr->hash_alg == SA_HASH_ALG_KASUMI_F9)
			rctx->tkn->input_tkn_len = sec_tkn_set_kasumi_f9(
				(u32 *) in_tkn,	req->nbytes,
				(u32 *) session->sa->sa_ptr, sa_hwaddr,
				crypto_ahash_digestsize(
					__crypto_ahash_cast(req->base.tfm)), 0);
		else
			rctx->tkn->input_tkn_len = sec_tkn_set_hash(
				(u32 *) in_tkn,	req->nbytes,
				(u32 *) session->sa->sa_ptr, sa_hwaddr,
				crypto_ahash_digestsize(
					__crypto_ahash_cast(req->base.tfm)), 0);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
	} else {
		if (session->sa->sa_ptr->hash_alg == SA_HASH_ALG_KASUMI_F9)
			sec_tkn_update_kasumi_f9((u32 *) in_tkn, nbytes);
		else
			sec_tkn_update_hash((u32 *) in_tkn, nbytes);
	}
	rctx->tkn->sa = session->sa;
	rctx->tkn->context = &req->base;

	/*
	 * NOTE: The source and destination addresses are transferred to
	 *       the QM message right before sending to QM hardware.
	 */
	rc = xgene_sec_setup_crypto(session->ctx, &req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		dev_err(session->ctx->dev, "hash digest failed error %d\n",
			rc);
	xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}

static int xgene_sec_sha1_alg_init(struct crypto_tfm *tfm)
{
	return xgene_sec_hash_alg_init(tfm, SA_SHA1_LEN(1),
				       SA_HASH_ALG_SHA1, SA_DIGEST_TYPE_INIT);
}

static int xgene_sec_sha2_alg_init(struct crypto_tfm *tfm)
{
        int ds = crypto_ahash_digestsize(__crypto_ahash_cast(tfm));
        unsigned char ha = 0;

        switch (ds) {
        default:
        case 256/8:
                ha = SA_HASH_ALG_SHA256;
                break;
        case 224/8:
                ha = SA_HASH_ALG_SHA224;
                break;
        case 512/8:
                ha = SA_HASH_ALG_SHA512;
                break;
        case 384/8:
                ha = SA_HASH_ALG_SHA384;
                break;
        }
        return xgene_sec_hash_alg_init(tfm, SA_SHA2_LEN(ds, 1),
                                       ha, SA_DIGEST_TYPE_INIT);
}

static int xgene_sec_compute_ipad_opad(struct crypto_tfm *tfm,
					const unsigned char *key,
					unsigned int keylen,
					unsigned char *ipad_opad)
{
	struct crypto_shash 	*child_hash = NULL;
	struct crypto_tfm 	*child_tfm;
	unsigned char 	*input = NULL;
	unsigned char	*opad;
	unsigned char	*ipad;
	const char 	*alg_name;
	char 	*child_name;
	int 	bs;
	int 	ctx_size;
	int	dl;
	int	rc;
	int	i;
	struct {
		struct shash_desc shash;
		char ctx[256];
	} sdesc;

	alg_name = crypto_tfm_alg_name(tfm);
	if (strstr(alg_name, "md5") != 0) {
		child_name = "md5";
		dl = 16;
	} else if (strstr(alg_name, "sha1") != 0) {
		child_name = "sha1";
		dl = 20;
	} else if (strstr(alg_name, "sha224") != 0) {
		child_name = "sha224";
		dl = 32;
	} else if (strstr(alg_name, "sha256") != 0) {
		child_name = "sha256";
		dl = 32;
	} else if (strstr(alg_name, "sha384") != 0) {
		child_name = "sha384";
		dl = 64;
	} else if (strstr(alg_name, "sha512") != 0) {
		child_name = "sha512";
		dl = 64;
	} else {
		dev_err(((struct xgene_sec_session_ctx *)crypto_tfm_ctx(tfm))->ctx->dev,
			"unsupported algorithm %s", alg_name);
		return -EINVAL;
	}

	child_hash = crypto_alloc_shash(child_name, 0, 0);
	if (IS_ERR(child_hash)) {
		rc = PTR_ERR(child_hash);
		dev_err(((struct xgene_sec_session_ctx *)crypto_tfm_ctx(tfm))->ctx->dev, 
			"failed to load transform for %s error %d",
			child_name, rc);
		goto err_alg;
	}

	child_tfm = crypto_shash_tfm(child_hash);
	bs = crypto_tfm_alg_blocksize(child_tfm);
	ctx_size = crypto_shash_descsize(child_hash);
	/* Allocate buffer for ipad/opad/key buffer */
	input = kzalloc(ctx_size * 2, GFP_KERNEL);
	if (input == NULL) {
		rc = -ENOMEM;
		goto err_alg_hash_key;
	}
	ipad = input;
	opad = input + ctx_size;

	sdesc.shash.tfm = child_hash;
	sdesc.shash.flags = 0;

	if (keylen > bs) {
		rc = crypto_shash_init(&sdesc.shash);
		if (rc < 0)
			goto err_alg_hash_key;
		rc = crypto_shash_update(&sdesc.shash, key, keylen);
		if (rc < 0)
			goto err_alg_hash_key;
		rc = crypto_shash_final(&sdesc.shash, ipad);
		keylen = crypto_shash_digestsize(child_hash);
	} else {
		memcpy(ipad, key, keylen);
	}
	
	memset(ipad + keylen, 0, ctx_size - keylen);
	memcpy(opad, ipad, ctx_size);
	for (i = 0; i < bs; i++) {
		ipad[i] ^= 0x36;
		opad[i] ^= 0x5c;
	}

	crypto_shash_init(&sdesc.shash);
	crypto_shash_update(&sdesc.shash, ipad, bs);
 	crypto_shash_export(&sdesc.shash, ipad);
 	/* 
	 * We load the digest in byte stream array 
 	 * SHA is big endian, MD5 is little endian
 	 */
	if (dl < 64) {
		if (strstr(child_name, "md5") != 0)
			sec_sa_setle((u32 *) ipad_opad, ipad, dl);
		else
			sec_sa_setbe((u32 *) ipad_opad, ipad + 8, dl);
	} else {
		ipad += 16;
		sec_sa_setbe((u32 *) ipad_opad, ipad, dl);
	}

	crypto_shash_init(&sdesc.shash);
 	crypto_shash_update(&sdesc.shash, opad, bs);
	crypto_shash_export(&sdesc.shash, opad);
 	/* 
	 * We load the digest in byte stream array 
 	 * SHA is big endian, MD5 is little endian
 	 */
	if (dl < 64) {
		if (strstr(child_name, "md5") != 0)
			sec_sa_setle((u32 *) (ipad_opad + dl), opad, dl);
		else
			sec_sa_setbe((u32 *) (ipad_opad + dl), opad + 8, dl);
	} else {
		opad += 16;
		sec_sa_setbe((u32 *) (ipad_opad + dl), opad, dl);
	}

	rc = dl * 2;

err_alg_hash_key:
	crypto_free_shash(child_hash);

err_alg:
	if (input)
		kfree(input);

	return rc;
}

static int xgene_sec_hash_hmac_setkey(struct crypto_ahash *hash,
				       const unsigned char *key,
				       unsigned int keylen,
				       unsigned int sa_len,
				       unsigned char ha,
				       unsigned char dt)
{
	struct crypto_tfm *tfm = crypto_ahash_tfm(hash);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	int	rc;

	/* Re-create the SA/token pool */
	rc = xgene_sec_create_sa_tkn_pool(session, sa_len, sa_len, 0,
					  TKN_BASIC_MAX_LEN);
	if (rc != 0)
		goto err_nomem;
	/* Setup SA */
	session->sa->sa_ptr->hash_alg    = ha;
	session->sa->sa_ptr->digest_type = dt;
	session->sa->sa_ptr->ToP	 = SA_TOP_HASH_OUTBOUND;
	session->sa->sa_ptr->context_length = (session->sa_len >> 2) - 2;

	/* Load pre-computed inner and outer digest into SA */
	rc = xgene_sec_compute_ipad_opad(tfm, key, keylen,
					 (u8 *) SA_DIGEST(session->sa->sa_ptr));

	if (rc < 0)
		goto err_nomem;

	dev_dbg(session->ctx->dev, "alg %s setkey", crypto_tfm_alg_name(tfm));
	return 0;

err_nomem:
	return rc;
}

static int xgene_sec_sha1_hmac_setkey(struct crypto_ahash *hash,
				       const unsigned char *key,
				       unsigned int keylen)
{
	return xgene_sec_hash_hmac_setkey(hash, key, keylen,
					  SA_SHA1_LEN(1), SA_HASH_ALG_SHA1,
					  SA_DIGEST_TYPE_HMAC);
}

static int xgene_sec_sha2_hmac_setkey(struct crypto_ahash *hash,
				      const unsigned char *key,
				      unsigned int keylen)
{
	int ds = crypto_ahash_digestsize(hash);
	unsigned char ha = 0;

	switch (ds) {
	default:
	case 256/8:
		ha = SA_HASH_ALG_SHA256;
		break;
	case 224/8:
		ha = SA_HASH_ALG_SHA224;
		ds = 32;
		break;
	case 512/8:
		ha = SA_HASH_ALG_SHA512;
		break;
	case 384/8:
		ha = SA_HASH_ALG_SHA384;
		ds = 64;
		break;
	}
	return xgene_sec_hash_hmac_setkey(hash, key, keylen,
					  SA_SHA2_LEN(ds, 1), ha,
					  SA_DIGEST_TYPE_HMAC);
}

static int xgene_sec_md5_alg_init(struct crypto_tfm *tfm)
{
	return xgene_sec_hash_alg_init(tfm, SA_MD5_LEN(0),
				       SA_HASH_ALG_MD5, SA_DIGEST_TYPE_INIT);
}

static int xgene_sec_md5_hmac_setkey(struct crypto_ahash *hash,
				const unsigned char *key,
				unsigned int keylen)
{
	return xgene_sec_hash_hmac_setkey(hash, key, keylen, SA_MD5_LEN(1),
	 				  SA_HASH_ALG_MD5, SA_DIGEST_TYPE_HMAC);
}

/*
 * XCBC Key Generation Rotuine
 */
int xgene_sec_xcbc_digest(struct crypto_tfm *tfm, const unsigned char *key,
			  unsigned int keylen, u32 *ipad_opad)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	struct crypto_blkcipher *blk_tfm;
	struct blkcipher_desc blk_desc;
	struct scatterlist sg[1];
	unsigned char *block128;
	int j;
	int len;
	int rc;

	/* Load K2, K3, and K1 digest */
	blk_tfm = crypto_alloc_blkcipher("ecb(aes)", 0, 0);
	if (IS_ERR(blk_tfm)) {
		rc = -PTR_ERR(blk_tfm);
		dev_err(session->ctx->dev,
			"failed to load helper transform for ecb(aes) error %d",
			rc);
		return rc;
	}
	blk_desc.tfm	= blk_tfm;
	blk_desc.flags	= 0;
	rc		= crypto_blkcipher_setkey(blk_tfm, key, keylen);
	if (rc != 0) {
		dev_err(session->ctx->dev, "failed to set helper transform key "
			"for ecb(aes) error %d", rc);
		rc = -EINVAL;
		goto err;
	}

	block128 = kzalloc(16, GFP_ATOMIC);
	if (block128 == NULL) {
                rc = -ENOMEM;
                goto err;
        }
	for (j = 2; j < 5; j++) {
		memset(block128, j == 4 ? 0x01 : j, 16);
		sg_init_one(&sg[0], block128, 128/8);
		rc = crypto_blkcipher_encrypt(&blk_desc, sg, sg, 128/8);
		if (rc != 0) {
			dev_err(session->ctx->dev, "failed to encrypt hash key"
				"for xcbc key error %d", rc);
			goto err;
		}
		/* Encrypt result is BE, swap to LE to fill in Context */
		for (len = 16; len > 0; block128 += 4, len -= 4)
                	*ipad_opad++ = swab32(*(unsigned int *)block128);
	}
err:
	if (blk_tfm)
		crypto_free_blkcipher(blk_tfm);
	return rc;

}

static int xgene_sec_sha2_xcbc_setkey(struct crypto_ahash *hash,
				      const unsigned char *key,
				      unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_ahash_tfm(hash);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	int rc;

	if (keylen != 128/8) {
		crypto_ahash_set_flags(hash, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -1;
	}

	/* Re-create the SA/token pool */
	rc = xgene_sec_create_sa_tkn_pool(session, SA_AES_XCBC_LEN,
					  SA_AES_XCBC_LEN, 0,
					  TKN_BASIC_MAX_LEN);
	if (rc != 0)
		return -ENOMEM;

	session->sa->sa_ptr->hash_alg	    = SA_HASH_ALG_XCBC128;
	session->sa->sa_ptr->digest_type    = SA_DIGEST_TYPE_HASH;
	session->sa->sa_ptr->ToP	    = SA_TOP_HASH_OUTBOUND;
	session->sa->sa_ptr->context_length = (session->sa_len >> 2) - 2;

	/* Load pre-computed key value into SA */
	rc = xgene_sec_xcbc_digest(tfm, key, keylen,	
				   SA_DIGEST(session->sa->sa_ptr));

	return rc;
}

/* 
 * Kasumi F9 - Hash Algorithms
 */
static int xgene_sec_kasumi_f9_setkey(struct crypto_ahash *hash,
				      const unsigned char *key,
				      unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_ahash_tfm(hash);
	struct xgene_sec_session_ctx *ctx = crypto_tfm_ctx(tfm);
	int rc;
	int direction;
	unsigned char *count, *fresh;
	char i;

	/* 16 byte key, 4 byte count, 4 byte fresh, 1 byte direction */
	if (keylen != 16 + 4 + 4 + 1) {
		crypto_ahash_set_flags(hash, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -1;
	}
	keylen -= 9;
	count	= kzalloc(4, GFP_ATOMIC);
	fresh	= kzalloc(4, GFP_ATOMIC);
	for (i = 0; i < 4; i++) {
		count[i] = key[keylen + i];
		fresh[i] = key[keylen + 4 + i];
	}
	direction = key[24] ? 1 : 0;

	/* Re-create the SA/token pool */
	rc = xgene_sec_create_sa_tkn_pool(ctx, SA_KASUMI_F9_LEN,
					  SA_KASUMI_F9_LEN, 0,
					  TKN_BASIC_MAX_LEN);
	if (rc != 0)
		return rc;

	ctx->sa->sa_ptr->ToP		= SA_TOP_HASH_OUTBOUND;
	ctx->sa->sa_ptr->hash_alg	= SA_HASH_ALG_KASUMI_F9;
	ctx->sa->sa_ptr->digest_type	= SA_DIGEST_TYPE_HASH;
	ctx->sa->sa_ptr->hash_store	= 0;
	ctx->sa->sa_ptr->context_length	= (ctx->sa_len >> 2) - 2;
	ctx->sa->sa_ptr->spi		= 1;
	ctx->sa->sa_ptr->seq		= 1;
	ctx->sa->sa_ptr->kasumi_f9_direction = direction;
	ctx->sa->sa_ptr->cryptomode_feedback = 0;

	/* Load hash key */
	memcpy(SA_DIGEST(ctx->sa->sa_ptr), key, keylen);
	/* Load Count. Swab to prepare filling into Hash engine */
	*SA_SEQ(ctx->sa->sa_ptr) = swab32(*(unsigned int *)count);
	/* Load Fresh. Swab to prepare filling into Hash engine */
	*SA_SPI(ctx->sa->sa_ptr) = swab32(*(unsigned int *)fresh);
	
	if (count)
		kfree(count);
	if (fresh)
		kfree(fresh);
	return 0;
}

static inline int xgene_sec_encrypt(struct ablkcipher_request *req)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = ablkcipher_request_ctx(req);
	struct sec_tkn_input_hdr *in_tkn;
	int		rc, i;
	unsigned char	new_tkn, *buf;
	unsigned short	iv_pos = 10, ivs = 16;
	unsigned int	*sa_ptr;

	APMSEC_TXLOG("encrypt %s nbytes %d",
		req->base.tfm->__crt_alg->cra_name, req->nbytes);

#ifdef APM_SEC_DEBUG
	if (req->info == NULL &&
	    rctx->sa->sa_ptr->cryptomode_feedback ==
			SA_CRYPTO_MODE_AES_3DES_CBC)
	    return -EINVAL;
#endif
	rctx->sa  = NULL;	/* No private SA */
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	rctx->tkn->flags = 0;
	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("alg %s tkn 0x%p outtkn 0x%p intkn 0x%p",
		req->base.tfm->__crt_alg->cra_name, rctx->tkn,
		TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);

	session->sa->sa_ptr->ToP = SA_TOP_CRYPTO_OUTBOUND;

	switch (session->sa->sa_ptr->crypto_alg) {
	case SA_CRYPTO_ALG_DES:
		iv_pos = 4;
		break;
	case SA_CRYPTO_ALG_AES128:
	case SA_CRYPTO_ALG_ARC4:
	case SA_CRYPTO_ALG_KASUMI_F8:
		iv_pos = 6;
		break;
	case SA_CRYPTO_ALG_AES192:
	case SA_CRYPTO_ALG_3DES:
		iv_pos = 8;
		break;
	}
	if ((session->sa->sa_ptr->cryptomode_feedback == SA_CRYPTO_MODE_AES_3DES_CBC) ||
	    (session->sa->sa_ptr->cryptomode_feedback == SA_CRYPTO_MODE_AES_3DES_OFB) ||
	    (session->sa->sa_ptr->cryptomode_feedback == SA_CRYPTO_MODE_AES_3DES_CFB))
		ivs = session->sa->sa_ptr->crypto_alg == SA_CRYPTO_ALG_DES ||
		      session->sa->sa_ptr->crypto_alg == SA_CRYPTO_ALG_3DES ||
		      session->sa->sa_ptr->crypto_alg == SA_CRYPTO_ALG_KASUMI_F8 ? 8 : 16;
	/* AES CTR has 4 byte Nonce and then 8 byte IV */
	if ((session->sa->sa_ptr->cryptomode_feedback == SA_CRYPTO_MODE_AES_3DES_CTR) ||
	    (session->sa->sa_ptr->cryptomode_feedback == SA_CRYPTO_MODE_AES_3DES_CTR_LOAD_REUSE)) {
		iv_pos++ ;
		ivs = 8;
	}
	sa_ptr = (unsigned int *) session->sa->sa_ptr;
	buf = req->info;
	/* 
	 * Fill IV value to IV field in Context.
	 * For AES ICM, only fill Nonce, not IV value in IV field.
	 */
	if ((session->sa->sa_ptr->cryptomode_feedback != SA_CRYPTO_MODE_AES_3DES_ICM) &&
	    (session->sa->sa_ptr->cryptomode_feedback != SA_CRYPTO_MODE_AES_3DES_ICM_LOAD_REUSE)) {
		for (i = iv_pos; ivs > 0; buf += 4, ivs -= 4, i++)
			sa_ptr[i] = *(unsigned int *) buf;
	}

	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_crypt(
				(u32 *) in_tkn,
				req->nbytes,
				SA_PTR_HW(xgene_sec_encode2hwaddr(
					SA_CONTEXT_PTR(session->sa->sa_hwptr))),
				(u32 *) session->sa->sa_ptr,
				session->sa->sa_ptr->crypto_alg, 1);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
	} else {
		sec_tkn_update_crypt((u32 *) in_tkn, req->nbytes,
				(u32 *) session->sa->sa_ptr,
				session->sa->sa_ptr->crypto_alg, 1);
	}
	rctx->tkn->sa = session->sa;
	rctx->tkn->context	= &req->base;
	/* NOTE: The source and destination addresses are transferred to
	         the QM message right before sending to QM hardware via
	         function xgene_sec_loadbuffer2qmsg. */

	rc = xgene_sec_setup_crypto(session->ctx, &req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("encrypt failed error %d", rc);

	xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}

static inline int xgene_sec_decrypt(struct ablkcipher_request *req)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = ablkcipher_request_ctx(req);
	struct sec_tkn_input_hdr *in_tkn;
	int		rc, i;
	unsigned char	new_tkn, *buf;
	unsigned short	iv_pos = 10, ivs = 16;
	unsigned int	*sa_ptr;

	APMSEC_TXLOG("decrypt %s nbytes %d",
		req->base.tfm->__crt_alg->cra_name, req->nbytes);
#ifdef APM_SEC_DEBUG
	if (req->info == NULL &&
	    rctx->sa->sa_ptr->cryptomode_feedback ==
	    				SA_CRYPTO_MODE_AES_3DES_CBC)
		return -EINVAL;
#endif
	rctx->sa  = NULL;	/* No private SA */
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	rctx->tkn->flags = 0;
	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("alg %s tkn 0x%p outtkn 0x%p intkn 0x%p",
		req->base.tfm->__crt_alg->cra_name, rctx->tkn,
		TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);

	session->sa->sa_ptr->ToP = SA_TOP_CRYPTO_INBOUND;

	switch (session->sa->sa_ptr->crypto_alg) {
	case SA_CRYPTO_ALG_DES:
		iv_pos = 4;
		break;
	case SA_CRYPTO_ALG_AES128:
	case SA_CRYPTO_ALG_ARC4:
	case SA_CRYPTO_ALG_KASUMI_F8:
		iv_pos = 6;
		break;
	case SA_CRYPTO_ALG_AES192:
	case SA_CRYPTO_ALG_3DES:
		iv_pos = 8;
		break;
	}
	if ((session->sa->sa_ptr->cryptomode_feedback == SA_CRYPTO_MODE_AES_3DES_CBC) ||
	    (session->sa->sa_ptr->cryptomode_feedback == SA_CRYPTO_MODE_AES_3DES_OFB) ||
	    (session->sa->sa_ptr->cryptomode_feedback == SA_CRYPTO_MODE_AES_3DES_CFB))
		ivs = session->sa->sa_ptr->crypto_alg == SA_CRYPTO_ALG_DES ||
		      session->sa->sa_ptr->crypto_alg == SA_CRYPTO_ALG_3DES ||
		      session->sa->sa_ptr->crypto_alg == SA_CRYPTO_ALG_KASUMI_F8 ? 8 : 16;
	/* AES CTR has 4 byte Nonce and then 8 byte IV */
	if ((session->sa->sa_ptr->cryptomode_feedback == SA_CRYPTO_MODE_AES_3DES_CTR) ||
	    (session->sa->sa_ptr->cryptomode_feedback == SA_CRYPTO_MODE_AES_3DES_CTR_LOAD_REUSE)) {
		iv_pos++ ;
		ivs = 8;
	}
	sa_ptr = (unsigned int *) session->sa->sa_ptr;
	buf = req->info;
	/* 
	 * Fill IV value to IV field in Context.
	 * For AES ICM, only fill Nonce, not IV value in IV field.
	 */
	if ((session->sa->sa_ptr->cryptomode_feedback != SA_CRYPTO_MODE_AES_3DES_ICM) &&
	    (session->sa->sa_ptr->cryptomode_feedback != SA_CRYPTO_MODE_AES_3DES_ICM_LOAD_REUSE)) {
		for (i = iv_pos; ivs > 0; buf += 4, ivs -= 4, i++)
			sa_ptr[i] = *(unsigned int *) buf;
	}

	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_crypt(
				(u32 *) in_tkn,
				req->nbytes,
				SA_PTR_HW (xgene_sec_encode2hwaddr(
					SA_CONTEXT_PTR(session->sa->sa_hwptr))),
				(u32 *) session->sa->sa_ptr,
				session->sa->sa_ptr->crypto_alg, 0);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
	} else {
		sec_tkn_update_crypt((u32 *) in_tkn, req->nbytes,
				(u32 *) session->sa->sa_ptr,
				session->sa->sa_ptr->crypto_alg, 0);
	}
	rctx->tkn->sa = session->sa;
	rctx->tkn->context     = &req->base;
	/* NOTE: The source and destination addresses are transferred to
	         the QM message right before sending to QM hardware via
	         function xgene_sec_loadbuffer2qmsg. */

	rc = xgene_sec_setup_crypto(session->ctx, &req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("decrypt failed error %d", rc);

	xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}

/*
 * AES Functions
 *
 */
static int xgene_sec_setkey_aes(struct crypto_ablkcipher *cipher,
				const unsigned char *key, unsigned int keylen,
				unsigned char cm)
{
	struct crypto_tfm *tfm = crypto_ablkcipher_tfm(cipher);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	const unsigned char  *nonce = NULL;
	int 	rc;

	if (cm == SA_CRYPTO_MODE_AES_3DES_CTR ||
	    cm == SA_CRYPTO_MODE_AES_3DES_CTR_LOAD_REUSE) {
		/* AES CTR has 4 byte nonce */
		if (keylen != 256/8+4 && keylen != 128/8+4 &&
		    keylen != 192/8+4) {
			crypto_ablkcipher_set_flags(cipher,
					CRYPTO_TFM_RES_BAD_KEY_LEN);
			return -1;
		}
		keylen -= 4;
		nonce = key + keylen;
	} else if (cm == SA_CRYPTO_MODE_AES_3DES_ICM ||
		   cm == SA_CRYPTO_MODE_AES_3DES_ICM_LOAD_REUSE) {
		/* AES ICM has 14 byte nonce */
		if (keylen != 256/8+14 && keylen != 128/8+14 &&
		    keylen != 192/8+14) {
			crypto_ablkcipher_set_flags(cipher,
						    CRYPTO_TFM_RES_BAD_KEY_LEN);
			return -1;
		}
		keylen -= 14;
		nonce = key + keylen;
	} else if (keylen != 256/8 && keylen != 128/8 && keylen != 192/8) {
		crypto_ablkcipher_set_flags(cipher,
					CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -1;
	}
	/* Now, create SA, token and cache again */
	rc = xgene_sec_create_sa_tkn_pool(session, SA_AES_MAX_LEN(cm),
			SA_AES_LEN(keylen, cm), 0, TKN_BASIC_MAX_LEN);
	if (rc != 0)
		goto err_nomem;

	/* Setup SA  */
	if (keylen >= 256 / 8)
		session->sa->sa_ptr->crypto_alg = SA_CRYPTO_ALG_AES256;
	else if (keylen >= 192 / 8)
		session->sa->sa_ptr->crypto_alg = SA_CRYPTO_ALG_AES192;
	else
		session->sa->sa_ptr->crypto_alg = SA_CRYPTO_ALG_AES128;
	session->sa->sa_ptr->cryptomode_feedback = cm;
	session->sa->sa_ptr->key		= 1;
	session->sa->sa_ptr->context_length	= (session->sa_len >> 2) - 2;
	memcpy(SA_KEY(session->sa->sa_ptr), key, keylen);
	/* session->sa->sa_ptr->iv_format	= 0; */
	if (cm == SA_CRYPTO_MODE_AES_3DES_CTR ||
	    cm == SA_CRYPTO_MODE_AES_3DES_CTR_LOAD_REUSE) {
		memcpy(SA_IV(session->sa->sa_ptr), nonce, 4);
		sec_sa_set32le(SA_IV(session->sa->sa_ptr) + 3, 1);
		/* IV0,1,2,3 in SA but will over written by token */
		session->sa->sa_ptr->iv03 = 0xF;
	} else if (cm == SA_CRYPTO_MODE_AES_3DES_ICM ||
		   cm == SA_CRYPTO_MODE_AES_3DES_ICM_LOAD_REUSE) {
		memcpy(SA_IV(session->sa->sa_ptr), nonce, 14);
		sec_sa_set32le(SA_IV(session->sa->sa_ptr) + 13, 1);
		/* IV0,1,2,3 in SA but will over written by token */
		session->sa->sa_ptr->iv03 = 0xF;
	} else if (cm == SA_CRYPTO_MODE_AES_3DES_CBC ||
	    	   cm == SA_CRYPTO_MODE_AES_3DES_OFB ||
		   cm == SA_CRYPTO_MODE_AES_3DES_CFB) {
		/* IV0,1,2,3 in SA but will over written by token */
		session->sa->sa_ptr->iv03 = 0xF;
	}

	dev_dbg(session->ctx->dev, "alg %s setkey\n", crypto_tfm_alg_name(tfm));
	return 0;

err_nomem:
	return rc;
}

static inline int xgene_sec_setkey_aes_cbc(struct crypto_ablkcipher *cipher,
                                           const unsigned char *key,
                                           unsigned int keylen)
{
        return xgene_sec_setkey_aes(cipher, key, keylen,
                                    SA_CRYPTO_MODE_AES_3DES_CBC);
}

static inline int xgene_sec_setkey_aes_ecb(struct crypto_ablkcipher *cipher,
					   const unsigned char *key,
					   unsigned int keylen)
{
	return xgene_sec_setkey_aes(cipher, key, keylen,
				    SA_CRYPTO_MODE_AES_3DES_ECB);
}

static inline int xgene_sec_setkey_aes_ctr(struct crypto_ablkcipher *cipher,
					   const unsigned char *key,
					   unsigned int keylen)
{
	return xgene_sec_setkey_aes(cipher, key, keylen,
				    SA_CRYPTO_MODE_AES_3DES_CTR);
}

static inline int xgene_sec_setkey_aes_icm(struct crypto_ablkcipher *cipher,
					   const unsigned char *key,
					   unsigned int keylen)
{
	return xgene_sec_setkey_aes(cipher, key, keylen,
				    SA_CRYPTO_MODE_AES_3DES_ICM);
}

static inline int xgene_sec_setkey_aes_ofb(struct crypto_ablkcipher *cipher,
					   const unsigned char *key,
					   unsigned int keylen)
{
	return xgene_sec_setkey_aes(cipher, key, keylen,
				    SA_CRYPTO_MODE_AES_3DES_OFB);
}

/*
 * 3DES/DES Functions
 *
 */
static int xgene_sec_setkey_3des(struct crypto_ablkcipher *cipher,
				 const unsigned char *key,
				 unsigned int keylen,
				 unsigned char cm)
{
	struct	crypto_tfm *tfm = crypto_ablkcipher_tfm(cipher);
	struct	xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	int	rc = 0;

	if (keylen != 8 && keylen != 24) {
		crypto_ablkcipher_set_flags(cipher,
					    CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	if (keylen == 8) {
		u32 tmp[32];
		rc = des_ekey(tmp, key);
		if (unlikely(rc == 0) &&
		    (tfm->crt_flags & CRYPTO_TFM_REQ_WEAK_KEY)) {
			crypto_ablkcipher_set_flags(cipher,
						    CRYPTO_TFM_RES_WEAK_KEY);
			return -EINVAL;
		}
	}

	/* Re-create SA/token pool */
	rc = xgene_sec_create_sa_tkn_pool(session,
			keylen == 8 ? SA_DES_MAX_LEN : SA_3DES_MAX_LEN,
			keylen == 8 ? SA_DES_LEN(cm) : SA_3DES_LEN(cm),
			1, TKN_BASIC_MAX_LEN);
	if (rc != 0)
		goto err_nomem;

	/* Setup SA */
	session->sa->sa_ptr->crypto_alg = keylen == 8 ? SA_CRYPTO_ALG_DES :
					                SA_CRYPTO_ALG_3DES;
	session->sa->sa_ptr->cryptomode_feedback = cm;
	session->sa->sa_ptr->key            	 = 1;
	session->sa->sa_ptr->context_length 	 = (session->sa_len >> 2) - 2;
	/* session->sa->sa_ptr->iv_format = 0; */
        memcpy(SA_KEY(session->sa->sa_ptr), key, keylen);

	if (cm == SA_CRYPTO_MODE_AES_3DES_CBC)
		session->sa->sa_ptr->iv03 = 0x3; /* Overwrite via PRNG/Tkn */
	dev_dbg(session->ctx->dev, "alg %s setkey\n", crypto_tfm_alg_name(tfm));
	return 0;

err_nomem:
	return rc;
}

static inline int xgene_sec_setkey_3des_cbc(struct crypto_ablkcipher *cipher,
					    const unsigned char *key,
					    unsigned int keylen)
{
	return xgene_sec_setkey_3des(cipher, key, keylen,
				     SA_CRYPTO_MODE_AES_3DES_CBC);
}

static inline int xgene_sec_setkey_3des_ecb(struct crypto_ablkcipher *cipher,
					    const unsigned char *key,
					    unsigned int keylen)
{
	return xgene_sec_setkey_3des(cipher, key, keylen,
				     SA_CRYPTO_MODE_AES_3DES_ECB);
}

static inline int xgene_sec_setkey_3des_cfb(struct crypto_ablkcipher *cipher,
					    const unsigned char *key,
					    unsigned int keylen)
{
	return xgene_sec_setkey_3des(cipher, key, keylen,
				     SA_CRYPTO_MODE_AES_3DES_CFB);
}

static inline int xgene_sec_setkey_3des_ofb(struct crypto_ablkcipher *cipher,
					    const unsigned char *key,
					    unsigned int keylen)
{
	return xgene_sec_setkey_3des(cipher, key, keylen,
				     SA_CRYPTO_MODE_AES_3DES_OFB);
}

/* ARC4 Functions */
static int xgene_sec_setkey_arc4(struct crypto_ablkcipher *cipher,
				 const unsigned char *key, unsigned int keylen,
				 unsigned char cm)
{
	struct crypto_tfm *tfm = crypto_ablkcipher_tfm(cipher);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	int rc;

	if (keylen < 5 || keylen > 16) {
		crypto_ablkcipher_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -1;
	}
	/* Re-create SA and token cache */
	rc = xgene_sec_create_sa_tkn_pool(session, SA_ARC4_LEN(cm),
					  SA_ARC4_LEN(cm) +
					  (cm == SA_ARC4_STATEFULL ? 256 : 0),
					  0, TKN_BASIC_MAX_LEN);
	if (rc != 0)
		return rc;

	memcpy(SA_KEY(session->sa->sa_ptr), key, keylen);
	session->sa->sa_ptr->crypto_alg		 = SA_CRYPTO_ALG_ARC4;
	session->sa->sa_ptr->cryptomode_feedback = keylen;
	session->sa->sa_ptr->state_selection	 = cm;
	session->sa->sa_ptr->ij_pointer		 = cm;
	session->sa->sa_ptr->crypto_store	 = 1;
	session->sa->sa_ptr->key		 = 1;
	session->sa->sa_ptr->pkt_based_options   = 0;
	session->sa->sa_ptr->context_length	 = (session->sa_len >> 2) - 2;
	if (cm == SA_ARC4_STATEFULL) {
		int i;
		u32 j = 0;
		u32 k = 0;
		u64 *arc4_state = (u64 *) ((u8 *) session->sa->sa_ptr +
					   SA_ARC4_LEN(SA_ARC4_STATEFULL));
		sec_sa_set32le(SA_ARC4(session->sa->sa_ptr),
			       xgene_sec_encode2hwaddr(*arc4_state));
		sec_sa_set32le(SA_ARC4(session->sa->sa_ptr) + 1, 0);
		/* Initialize ARC4 state record */
		for (i = 0; i < 256; i++)
			arc4_state[i] = i;
		for (i = 0; i < 256; i++) {
			u32 temp = arc4_state[i];
			j = (j + temp + key[k]) & 0xff;
			arc4_state[i] = arc4_state[j];
			arc4_state[j] = temp;
			if (++k >= keylen)
				k = 0;
		}
	}

	return 0;
}

static int xgene_sec_setkey_arc4_stateless(struct crypto_ablkcipher *cipher,
					   const unsigned char * key,
					   unsigned int keylen)
{
	return xgene_sec_setkey_arc4(cipher, key, keylen, SA_ARC4_STATELESS);
}

static int xgene_sec_setkey_arc4_statefull(struct crypto_ablkcipher *cipher,
					   const unsigned char * key,
					   unsigned int keylen)
{
	return xgene_sec_setkey_arc4(cipher, key, keylen, SA_ARC4_STATEFULL);
}

static inline int xgene_sec_arc4_encrypt(struct ablkcipher_request *req)
{
	struct xgene_sec_session_ctx *session  = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = ablkcipher_request_ctx(req);
	struct sec_tkn_input_hdr *in_tkn;
	unsigned char new_tkn;
	int rc;

	rctx->sa	= NULL;
	rctx->tkn	= xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	in_tkn		= TKN_CTX_INPUT_TKN(rctx->tkn);
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_arc4_crypt(
				(u32 *) in_tkn,	req->nbytes,
				SA_PTR_HW(xgene_sec_encode2hwaddr(
					SA_CONTEXT_PTR(session->sa->sa_hwptr))),
				(u32 *) session->sa->sa_ptr, 1);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
	} else {
		sec_tkn_update_arc4((u32 *) in_tkn, req->nbytes,
				    (u32 *) session->sa->sa_ptr);
	}
	rctx->tkn->sa		= session->sa;
	rctx->tkn->context	= &req->base;

	rc = xgene_sec_setup_crypto(session->ctx, &req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("encrypt failed error %d", rc);

	xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}

static inline int xgene_sec_arc4_decrypt(struct ablkcipher_request *req)
{
	struct xgene_sec_session_ctx *session  = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = ablkcipher_request_ctx(req);
	struct sec_tkn_input_hdr *in_tkn;
	unsigned char new_tkn;
	int rc;

	rctx->sa	= NULL;
	rctx->tkn	= xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	in_tkn		= TKN_CTX_INPUT_TKN(rctx->tkn);
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_arc4_crypt(
				(u32 *) in_tkn,	req->nbytes,
				SA_PTR_HW(xgene_sec_encode2hwaddr(
					SA_CONTEXT_PTR(session->sa->sa_hwptr))),
				(u32 *) session->sa->sa_ptr, 0);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
	} else {
		sec_tkn_update_arc4((u32 *) in_tkn, req->nbytes, 
				    (u32 *) session->sa->sa_ptr);
	}
	rctx->tkn->sa		= session->sa;
	rctx->tkn->context	= &req->base;

	rc = xgene_sec_setup_crypto(session->ctx, &req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("encrypt failed error %d", rc);

	xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}

/*
 * Kasumi Functions
 */
static inline int xgene_sec_setkey_kasumi(struct crypto_ablkcipher *cipher,
					  const unsigned char *key,
					  unsigned int keylen, int mode)
{
	struct crypto_tfm *tfm = crypto_ablkcipher_tfm(cipher);
	struct xgene_sec_session_ctx *ctx = crypto_tfm_ctx(tfm);
	int rc;

	if (keylen != 16) {
		crypto_ablkcipher_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -1;
	}

	rc = xgene_sec_create_sa_tkn_pool(ctx, SA_KASUMI_F8_LEN(mode),
					  SA_KASUMI_F8_LEN(mode),
					  0, TKN_BASIC_MAX_LEN);
	if (rc != 0)
		return rc;

	memcpy(SA_KEY(ctx->sa->sa_ptr), key, keylen);
	ctx->sa->sa_ptr->crypto_alg 	     = SA_CRYPTO_ALG_KASUMI_F8;
	ctx->sa->sa_ptr->cryptomode_feedback = mode;
	ctx->sa->sa_ptr->key		     = 1;
	ctx->sa->sa_ptr->context_length      = (ctx->sa_len >> 2) - 2;

	if (mode == SA_CRYPTO_MODE_KASUMI_F8)
		ctx->sa->sa_ptr->iv03 = 0x3; 
	return 0;
}

static inline int xgene_sec_setkey_kasumi_p(struct crypto_ablkcipher *cipher,
					    const unsigned char *key,
					    unsigned int keylen)
{
	return xgene_sec_setkey_kasumi(cipher, key, keylen,
				       SA_CRYPTO_MODE_KASUMI);
}

static inline int xgene_sec_setkey_kasumi_f8(struct crypto_ablkcipher *cipher,
					     const unsigned char *key,
					     unsigned int keylen)
{
	return xgene_sec_setkey_kasumi(cipher, key, keylen,
				       SA_CRYPTO_MODE_KASUMI_F8);
}

/*
 * Authenc(cbc(aes),hmac(sha1)) IPSec Functions
 *
 */
static int xgene_sec_setkey_cbc_hmac_authenc(struct crypto_aead *cipher,
			const unsigned char *key,
			unsigned int keylen,
			unsigned char ca,
			unsigned char ha)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	struct crypto_authenc_key_param *param;
	struct rtattr *rta = (void *) key;
	unsigned int enckeylen;
	unsigned int authkeylen;
	unsigned int sa_len;
	unsigned int sa_max_len;
	int rc = -EINVAL;

	if (!RTA_OK(rta, keylen))
		goto badkey;

	param = RTA_DATA(rta);
	enckeylen = be32_to_cpu(param->enckeylen);

	key += RTA_ALIGN(rta->rta_len);
	keylen -= RTA_ALIGN(rta->rta_len);

	authkeylen = keylen - enckeylen;

	if (keylen < enckeylen)
		goto badkey;
	if (enckeylen != 128/8 && enckeylen != 192/8
		&& enckeylen != 256/8 && enckeylen != 64/8)
		goto badkey;
	if (ca == SA_CRYPTO_ALG_RSVD) {
		switch (enckeylen) {
		case 128/8:
			ca = SA_CRYPTO_ALG_AES128;
			break;
		case 192/8:
			ca = SA_CRYPTO_ALG_AES192;
			break;
		case 256/8:
		default:
			ca = SA_CRYPTO_ALG_AES256;
			break;
		}
	}
	if (ca == SA_CRYPTO_ALG_DES) {
		sa_len = SA_AUTHENC_DES_LEN(
					1,
					ha, SA_DIGEST_TYPE_HMAC);
		sa_max_len = SA_AUTHENC_DES_MAX_LEN(
					1,
					ha, SA_DIGEST_TYPE_HMAC);
	} else if (ca == SA_CRYPTO_ALG_3DES) {
		sa_len = SA_AUTHENC_3DES_LEN(
					1,
					ha, SA_DIGEST_TYPE_HMAC);
		sa_max_len = SA_AUTHENC_3DES_MAX_LEN(
					1,
					ha, SA_DIGEST_TYPE_HMAC);
	} else {
		sa_len = SA_AUTHENC_AES_LEN(enckeylen,
					1,
					ha, SA_DIGEST_TYPE_HMAC);
		sa_max_len = SA_AUTHENC_AES_MAX_LEN(enckeylen,
					1,
					ha, SA_DIGEST_TYPE_HMAC);
	}

	rc = xgene_sec_create_sa_tkn_pool(session, sa_max_len, sa_len, 0,
			IPE_INTKN_AUTHENC_MAX_SIZE);
	if (rc != 0)
		goto err_nomem;

	/* Setup SA */
	session->sa->sa_ptr->crypto_alg = ca;
	session->sa->sa_ptr->hash_alg = ha;
	session->sa->sa_ptr->digest_type = SA_DIGEST_TYPE_HMAC;
	session->sa->sa_ptr->key = 1;
	session->sa->sa_ptr->cryptomode_feedback = SA_CRYPTO_MODE_AES_3DES_CBC;
	session->sa->sa_ptr->iv03 = 0xF;
	session->sa->sa_ptr->context_length = (sa_len >> 2) - 2;
	memcpy(SA_KEY(session->sa->sa_ptr), key + authkeylen, enckeylen);

	/* Pre-compute inner and outer digest using auth key */
	rc = xgene_sec_compute_ipad_opad(tfm, key, authkeylen,
					 (u8 *) SA_DIGEST(session->sa->sa_ptr));
	return 0;

err_nomem:
	return rc;

badkey:
	crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return rc;
}

static int xgene_sec_setkey_aes_md5_authenc(struct crypto_aead *cipher,
					    const unsigned char *key,
					    unsigned int keylen)
{
	return xgene_sec_setkey_cbc_hmac_authenc(cipher, key, keylen,
						 SA_CRYPTO_ALG_RSVD,
						 SA_HASH_ALG_MD5);
}

static int xgene_sec_setkey_aes_sha1_authenc(struct crypto_aead *cipher,
			const unsigned char *key,
			unsigned int keylen)
{
	return xgene_sec_setkey_cbc_hmac_authenc(cipher, key, keylen,
			SA_CRYPTO_ALG_RSVD, SA_HASH_ALG_SHA1);
}

static int xgene_sec_setkey_aes_sha224_authenc(struct crypto_aead *cipher,
					       const unsigned char *key,
					       unsigned int keylen)
{
	return xgene_sec_setkey_cbc_hmac_authenc(cipher, key, keylen,
						 SA_CRYPTO_ALG_RSVD,
						 SA_HASH_ALG_SHA224);
}

static int xgene_sec_setkey_aes_sha256_authenc(struct crypto_aead *cipher,
					       const unsigned char *key,
					       unsigned int keylen)
{
	return xgene_sec_setkey_cbc_hmac_authenc(cipher, key, keylen,
						 SA_CRYPTO_ALG_RSVD, 
						 SA_HASH_ALG_SHA256);
}

static int xgene_sec_setkey_3des_md5_authenc(struct crypto_aead *cipher,
					     const unsigned char *key,
					     unsigned int keylen)
{
	return xgene_sec_setkey_cbc_hmac_authenc(cipher, key, keylen,
						 SA_CRYPTO_ALG_3DES, 
						 SA_HASH_ALG_MD5);
}

static int xgene_sec_setkey_3des_sha1_authenc(struct crypto_aead *cipher,
					      const unsigned char *key,
					      unsigned int keylen)
{
	return xgene_sec_setkey_cbc_hmac_authenc(cipher, key, keylen,
						 SA_CRYPTO_ALG_3DES, 
						 SA_HASH_ALG_SHA1);
}

static int xgene_sec_setkey_3des_sha224_authenc(struct crypto_aead *cipher,
						const unsigned char *key,
						unsigned int keylen)
{
	return xgene_sec_setkey_cbc_hmac_authenc(cipher, key, keylen,
						 SA_CRYPTO_ALG_3DES, 
						 SA_HASH_ALG_SHA224);
}

static int xgene_sec_setkey_3des_sha256_authenc(struct crypto_aead *cipher,
						const unsigned char *key,
						unsigned int keylen)
{
	return xgene_sec_setkey_cbc_hmac_authenc(cipher, key, keylen,
						 SA_CRYPTO_ALG_3DES, 
						 SA_HASH_ALG_SHA256);
}

/*
 * SetAuthSize Functions
 *
 */
static int xgene_sec_setauthsize_aes(struct crypto_aead *ciper,
				     unsigned int authsize)
{
	struct aead_tfm *tfm = crypto_aead_crt(ciper);

	tfm->authsize = authsize;
	return 0;
}

static int xgene_sec_setauthsize_aes_gcm(struct crypto_aead *ciper,
					 unsigned int authsize)
{
	struct aead_tfm *tfm = crypto_aead_crt(ciper);

	switch (authsize) {
	case 4:
	case 8:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
		break;
	default:
		return -EINVAL;
	}

	tfm->authsize = authsize;
	return 0;
}

/*
 * Count the number of scatterlist entries in a scatterlist
 */
static int sg_count(struct scatterlist *sg_list, int nbytes)
{
	struct scatterlist *sg = sg_list;
	int sg_nents = 0;

	while (nbytes > 0) {
		++sg_nents;
		nbytes -= sg->length;
		sg = sg_next(sg);
	}

	return sg_nents;
}

/*
 * Make new source scatterlist includes
 * Assoc + IV + Data Payload
 */
static int xgene_aead_make_sg(struct aead_request *req, u8 *iv)
{
	unsigned int ivsize = crypto_aead_ivsize(crypto_aead_reqtfm(req));
	struct xgene_sec_session_ctx *session;
	struct xgene_sec_req_ctx *rctx;
	struct scatterlist *cur, *src_tmp;
	int i, src_ents, assoc_ents;

	session = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);

	/* Get scatterlist entries of source */
	src_ents = sg_count(req->src, req->cryptlen);
	/* Get scatterlist entries of assoc */
	assoc_ents = sg_count(req->assoc, req->assoclen);
	/* Update num of entries */
	rctx->tkn->src_sg_nents = src_ents + assoc_ents + 1;

	src_tmp = (struct scatterlist *) dma_alloc_coherent(session->ctx->dev,
			sizeof(struct scatterlist) * rctx->tkn->src_sg_nents,
			&rctx->tkn->src_sg_paddr, GFP_ATOMIC);
	if (!src_tmp)
		return -ENOMEM;
	rctx->tkn->src_sg = src_tmp;

	sg_init_table(src_tmp, rctx->tkn->src_sg_nents);

	/* Adding Assoc into source sg*/
	for_each_sg(req->assoc, cur, assoc_ents, i) {
		memcpy(src_tmp++, cur, sizeof(struct scatterlist));
		if (sg_is_last(cur))
			sg_unmark_end(src_tmp - 1);
	}

	/* Adding IV into source sg*/
	sg_set_buf(src_tmp++, iv, ivsize);
	/* Now map in the payload for the source */
	for_each_sg(req->src, cur, src_ents, i) {
		memcpy(src_tmp++, cur, sizeof(struct scatterlist));
	}
	/* Mark the last entry here */
	sg_mark_end(src_tmp - 1);

	/* Update new source length */
	rctx->tkn->src_sg_nbytes = req->assoclen + req->cryptlen + ivsize;

	return 0;
}

static int __xgene_sec_encrypt_cbc_authenc(
		struct aead_givcrypt_request *givreq,
		struct aead_request *req)
{
	struct xgene_sec_session_ctx *session;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead *aead;
	unsigned char new_tkn;
	int icvsize;
	int rc;
	u8 *iv;

	if (givreq) {
		req = &givreq->areq;
		iv = givreq->giv ? givreq->giv : req->iv;
	} else {
		iv = req->iv;
	}

	session = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);
	aead = crypto_aead_reqtfm(req);

	rctx->sa = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	rctx->tkn->flags = 0;
	icvsize = crypto_aead_authsize(aead);
	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_TXLOG("tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	session->sa->sa_ptr->ToP = SA_TOP_ENCRYPT_HASH_OUTBOUND;

	/* Loading IV */
	memcpy(SA_IV(session->sa->sa_ptr), iv, 16);

	/* Prepare source scatterlist*/
	rc = xgene_aead_make_sg(req, iv);
	if (rc)
		goto err_nomem;

	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_encrypt_cbc_authenc(
				(u32 *) in_tkn,
				rctx->tkn->src_sg_nbytes,
				SA_PTR_HW(xgene_sec_encode2hwaddr(
				SA_CONTEXT_PTR(session->sa->sa_hwptr))),
				(u32 *) session->sa->sa_ptr,
				session->sa->sa_ptr->crypto_alg,
				req->cryptlen,
				icvsize);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
	} else {
		sec_tkn_update_encrypt_cbc_authenc((u32 *) in_tkn,
				rctx->tkn->src_sg_nbytes, (u32 *) session->sa->sa_ptr,
				session->sa->sa_ptr->crypto_alg,
				req->cryptlen, icvsize);
	}

	rctx->tkn->sa = session->sa;
	rctx->tkn->context = &req->base;
	rctx->tkn->dest_nbytes = req->cryptlen + icvsize;
	/* NOTE: The source and destination addresses are transferred to
			the QM message right before sending to QM hardware via
			function xgene_sec_loadbuffer2qmsg. */
	rc = xgene_sec_setup_crypto(session->ctx, &req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		dev_dbg(session->ctx->dev, "Encrypt authenc failed error %d", rc);

err_nomem:
	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}


static int xgene_sec_encrypt_cbc_authenc(struct aead_request *req)
{
	return __xgene_sec_encrypt_cbc_authenc(NULL, req);
}

static int __xgene_sec_decrypt_cbc_authenc(
		struct aead_givcrypt_request *givreq,
		struct aead_request *req)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct sec_tkn_input_hdr *in_tkn;
	unsigned char new_tkn;
	int icvsize;
	int rc;
	u8 *iv;

	if (givreq) {
		req = &givreq->areq;
		iv = givreq->giv ? givreq->giv : req->iv;
	} else {
		iv = req->iv;
	}

	rctx->sa = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	rctx->tkn->flags = 0;
	icvsize = crypto_aead_authsize(aead);
	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	session->sa->sa_ptr->ToP = SA_TOP_HASH_DECRYPT_INBOUND;

	/* Loading IV */
	memcpy(SA_IV(session->sa->sa_ptr), iv, 16);

	/* Prepare source scatterlist*/
	rc = xgene_aead_make_sg(req, iv);
	if (rc)
		goto err_nomem;

	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_decrypt_cbc_authenc(
						(u32 *) in_tkn,
						rctx->tkn->src_sg_nbytes,
						SA_PTR_HW(xgene_sec_encode2hwaddr(
							SA_CONTEXT_PTR(session->sa->sa_hwptr))),
						(u32 *) session->sa->sa_ptr,
						session->sa->sa_ptr->crypto_alg,
						req->cryptlen,
						icvsize);
	} else {
		sec_tkn_update_decrypt_cbc_authenc((u32 *) in_tkn,
				rctx->tkn->src_sg_nbytes, (u32 *) session->sa->sa_ptr,
				session->sa->sa_ptr->crypto_alg,
				req->cryptlen, icvsize);
	}
	rctx->tkn->sa = session->sa;
	rctx->tkn->context = &req->base;
	rctx->tkn->dest_nbytes = req->cryptlen - icvsize;
	/* NOTE: The source and destination addresses are transferred to
			the QM message right before sending to QM hardware via
			function xgene_sec_loadbuffer2qmsg. */
	rc = xgene_sec_setup_crypto(session->ctx, &req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		dev_dbg(session->ctx->dev, "Decrypt authenc failed error %d", rc);

err_nomem:
	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}

static int xgene_sec_decrypt_cbc_authenc(struct aead_request *req)
{
	return __xgene_sec_decrypt_cbc_authenc(NULL, req);
}

static int xgene_sec_givencrypt_cbc_authenc(struct aead_givcrypt_request
						*givreq)
{
	return __xgene_sec_encrypt_cbc_authenc(givreq, NULL);
}

static int xgene_sec_givendecrypt_cbc_authenc(struct aead_givcrypt_request
						*givreq)
{
	return __xgene_sec_decrypt_cbc_authenc(givreq, NULL);
}

/*
 * GHASH Calculation Routine
 *
 */
int xgene_compute_gcm_hash_key(struct crypto_tfm *tfm,
			       const u8 * key, unsigned int keylen)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	struct scatterlist 	sg[1];
	struct crypto_blkcipher *aes_tfm = NULL;
	struct blkcipher_desc 	desc;
	int bs = crypto_tfm_alg_blocksize(tfm);
	unsigned char *digest;
	u32 *sa_hash;
	int rc;
	int len;

	/* Load pre-computed key value into SA */
	aes_tfm = crypto_alloc_blkcipher("ecb(aes)", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(aes_tfm)) {
		rc = PTR_ERR(aes_tfm);
		dev_err(session->ctx->dev,
			"failed to load transform for ecb(aes) error %d", rc);
		return rc;
	}
	desc.tfm    = aes_tfm;
	desc.flags  = 0;

	rc = crypto_blkcipher_setkey(desc.tfm, key, keylen);
	if (rc) {
		dev_err(session->ctx->dev,
			"failed to load gcm key error %d", rc);
		goto err_alg;
	}

	digest = kzalloc(16, GFP_ATOMIC);
	if (digest == NULL) {
		rc = -ENOMEM;
		goto err_alg;
	}
	memset(digest, 0x00, 16);
	sg_init_one(&sg[0], digest, 16);
	rc = crypto_blkcipher_encrypt(&desc, sg, sg, bs);
	if (rc < 0) {
		dev_err(session->ctx->dev, 
			"failed to hash gcm key error %d", rc);
		goto err_alg;
	}
	/* Encrypt result is BE, swap to LE to fill in Context */
	sa_hash = SA_DIGEST(session->sa->sa_ptr);
	for (len = 16; len > 0; digest += 4, len -=4) 
		*sa_hash++ = swab32(*(unsigned int *)digest);

err_alg:
	if (aes_tfm)
		crypto_free_blkcipher(aes_tfm);

	return rc;
}

/*
 * Basic Algorithm AES-GCM Functions
 *
 */
static inline int xgene_sec_setkey_aes_gcm(struct crypto_aead *cipher,
					   const unsigned char *key,
					   unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	int 	rc = 0;

	if (keylen != 128/8 && keylen != 192/8 && keylen != 256/8) {
		crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	/* Create the Token pool */
	rc = xgene_sec_create_sa_tkn_pool(session, SA_AES_GCM_MAX_LEN,
					  SA_AES_GCM_LEN(keylen),
					  0, TKN_AES_GCM_MAX);
	if (rc != 0)
		goto err;

	/* Setup SA */
	session->sa->sa_ptr->hash_alg    = SA_HASH_ALG_GHASH;
	session->sa->sa_ptr->digest_type = SA_DIGEST_TYPE_HASH;
	session->sa->sa_ptr->key 	 = 1;
	switch(keylen) {
	case 128/8:
		session->sa->sa_ptr->crypto_alg = SA_CRYPTO_ALG_AES128;
		break;
	case 192/8:
		session->sa->sa_ptr->crypto_alg = SA_CRYPTO_ALG_AES192;
		break;
	case 256/8:
		session->sa->sa_ptr->crypto_alg = SA_CRYPTO_ALG_AES256;
		break;
	}
	session->sa->sa_ptr->cryptomode_feedback = SA_CRYPTO_MODE_AES_3DES_CTR;

	session->sa->sa_ptr->mask	= SA_MASK_NONE;
	session->sa->sa_ptr->iv_format	= SA_IV_FORMAT_COUNTER_MODE;
	session->sa->sa_ptr->iv03	= 0x7;
	session->sa->sa_ptr->enc_hash_result = 1;
	session->sa->sa_ptr->context_length  = (session->sa_len >> 2) - 2;

	memcpy(SA_KEY(session->sa->sa_ptr), key, keylen);
	rc = xgene_compute_gcm_hash_key(tfm, key, keylen);
	if (rc < 0)
		goto err;

	dev_dbg(session->ctx->dev, "alg %s setkey", crypto_tfm_alg_name(tfm));
	return rc;

err:
	dev_dbg(session->ctx->dev, "alg %s setkey failed rc %d",
			crypto_tfm_alg_name(tfm), rc);
	return rc;
}

static int xgene_sec_encrypt_aes_gcm(struct aead_request *req)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct sec_tkn_input_hdr *in_tkn;
	void *adata;
	unsigned char new_tkn;
	int icvsize;
	int rc;
	u64 sa_hwaddr;
	int nents;

	/* Allocate token and SA for this request */
	rctx->sa  = NULL;       /* No private SA */	

	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	rctx->tkn->flags = 0;
	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("gcm encrypt tkn 0x%p outtkn 0x%p intkn 0x%p"
			"nbytes = %d",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn, req->cryptlen);

	session->sa->sa_ptr->ToP = SA_TOP_ENCRYPT_HASH_OUTBOUND;
	memcpy(SA_IV(session->sa->sa_ptr), req->iv, 12);

	adata = kmalloc(req->assoclen, GFP_ATOMIC);
	if (adata == NULL) {
		rc = -ENOMEM;
		goto err_enc_gcm;
	}
	nents = sg_nents(req->assoc);
	sg_copy_to_buffer(req->assoc, nents, adata, req->assoclen);

	icvsize = crypto_aead_authsize(aead);
	sa_hwaddr = SA_PTR_HW(xgene_sec_encode2hwaddr(
			      SA_CONTEXT_PTR(session->sa->sa_hwptr)));	
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_crypt_aes_gcm(
				(u32 *) in_tkn,
				req->cryptlen,
				sa_hwaddr,
				(u32 *) session->sa->sa_ptr,
				adata, req->assoclen, icvsize, 1);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
	} else {
		rctx->tkn->input_tkn_len = sec_tkn_update_crypt_aes_gcm(
				(u32 *) in_tkn,
				req->cryptlen,
				sa_hwaddr,
				(u32 *) session->sa->sa_ptr,
				adata, req->assoclen, icvsize, 1);
	}
	rctx->tkn->sa		= session->sa;
	rctx->tkn->context      = &req->base;
	rctx->tkn->dest_nbytes	= req->cryptlen + icvsize;

	rc = xgene_sec_setup_crypto(session->ctx, &req->base);

	kfree(adata);
 	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("encrypt gcm failed error %d",
				rc);

err_enc_gcm:
	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}

static int xgene_sec_decrypt_aes_gcm(struct aead_request *req)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	void *adata;
	unsigned char new_tkn;
	unsigned char icvsize;
	int rc;
	u64 sa_hwaddr;	
	int nents;

	/* Allocate token and SA for this request */
	rctx->sa  = NULL;

	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	rctx->tkn->flags = 0;	
	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("gcm decrypt tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);

	session->sa->sa_ptr->ToP = SA_TOP_HASH_DECRYPT_INBOUND;
	/* Form SA - SPI, SEQ, and Nonce */
	memcpy(SA_IV(session->sa->sa_ptr), req->iv, 12);

	adata = kmalloc(req->assoclen, GFP_ATOMIC);
	if (adata == NULL) {
		rc = -ENOMEM;
		goto err_dec_gcm;
	}
	nents = sg_nents(req->assoc);
	sg_copy_to_buffer(req->assoc, nents, adata, req->assoclen);

	icvsize = crypto_aead_authsize(aead);

	sa_hwaddr = SA_PTR_HW(xgene_sec_encode2hwaddr(
			      SA_CONTEXT_PTR(session->sa->sa_hwptr)));
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_crypt_aes_gcm(
				(u32 *) in_tkn,
				req->cryptlen,
				sa_hwaddr,
				(u32 *) session->sa->sa_ptr,
				adata, req->assoclen, icvsize, 0);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
	} else {
		rctx->tkn->input_tkn_len = sec_tkn_update_crypt_aes_gcm(
				(u32 *) in_tkn,
				req->cryptlen,
				sa_hwaddr,
				(u32 *) session->sa->sa_ptr,
				adata, req->assoclen, icvsize, 0);
	}
	rctx->tkn->sa		= session->sa;
	rctx->tkn->context	= &req->base;
	rctx->tkn->dest_nbytes	= req->cryptlen - icvsize;

	rc = xgene_sec_setup_crypto(session->ctx, &req->base);
	kfree(adata);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("decrypt gcm failed error %d", rc);

err_dec_gcm:
	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}

static int xgene_sec_givencrypt_aes_gcm(struct aead_givcrypt_request *req)
{
	return -ENOSYS;
}

static int xgene_sec_givdecrypt_aes_gcm(struct aead_givcrypt_request *req)
{
	return -ENOSYS;
}

/*
 * Supported Crypto Algorithms
 */
struct xgene_sec_alg xgene_sec_alg_tlb[] = {
	/* Basic Hash Alg */
	{.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
         .halg.base      	= {
 	 .cra_name 		= "md5",
	 .cra_driver_name 	= "md5-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* MD5 block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ahash_type,
	 .cra_init 		= xgene_sec_md5_alg_init },
	 .halg.digestsize 	= 16,	/* Digest is 128-bits */
	 .init 			= xgene_sec_hash_init,
	 .update 		= xgene_sec_hash_update,
	 .final 		= xgene_sec_hash_final,
	 .digest 		= xgene_sec_hash_digest,
	 }},

	{.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
         .halg.base      	= {
	 .cra_name 		= "sha1",
	 .cra_driver_name	= "sha1-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* 512-bit SHA1-HMAC block size */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0,	/* HW requires 16 bytes aligned */
	 .cra_type   		= &crypto_ahash_type,
	 .cra_init 		= xgene_sec_sha1_alg_init },
	 .halg.digestsize 	= 20,	/* 160-bits disgest */
	 .init   		= xgene_sec_hash_init,
	 .update 		= xgene_sec_hash_update,
	 .final  		= xgene_sec_hash_final,
	 .digest 		= xgene_sec_hash_digest,
	 }},

	{.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
         .halg.base      	= {
	 .cra_name 		= "sha224",
	 .cra_driver_name 	= "sha224-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* SHA224-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ahash_type,
	 .cra_init 		= xgene_sec_sha2_alg_init },
	 .halg.digestsize 	= 28,	/* Disgest is 224-bits */
	 .init   		= xgene_sec_hash_init,
	 .update 		= xgene_sec_hash_update,
	 .final  		= xgene_sec_hash_final,
	 .digest 		= xgene_sec_hash_digest,
	 }},

        {.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
         .halg.base             = {
         .cra_name              = "sha256",
         .cra_driver_name       = "sha256-xgene",
         .cra_flags             = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
         .cra_blocksize         = 64,   /* SHA256-HMAC block size is 512-bits */
         .cra_ctxsize           = sizeof(struct xgene_sec_session_ctx),
         .cra_alignmask         = 0,    /* Hardware requires 16 bytes aligned */
         .cra_type              = &crypto_ahash_type,
         .cra_init              = xgene_sec_sha2_alg_init },
         .halg.digestsize       = 32,   /* Disgest is 256-bits */
         .init                  = xgene_sec_hash_init,
         .update                = xgene_sec_hash_update,
         .final                 = xgene_sec_hash_final,
         .digest                = xgene_sec_hash_digest,
         }},

	{.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
         .halg.base      	= {
	 .cra_name 		= "sha384",
	 .cra_driver_name 	= "sha384-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 128,	/* SHA384-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ahash_type,
	 .cra_init 		= xgene_sec_sha2_alg_init },
	 .halg.digestsize 	= 48,	/* Disgest is 384-bits */
	 .init   		= xgene_sec_hash_init,
	 .update 		= xgene_sec_hash_update,
	 .final  		= xgene_sec_hash_final,
	 .digest 		= xgene_sec_hash_digest,
	 }},

	{.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
         .halg.base      	= {
	 .cra_name 		= "sha512",
	 .cra_driver_name 	= "sha512-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 128,	/* SHA512-HMAC block size is 1024-bits*/
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ahash_type,
	 .cra_init 		= xgene_sec_sha2_alg_init },
	 .halg.digestsize 	= 64,	/* Disgest is 512-bits */
	 .init   		= xgene_sec_hash_init,
	 .update 		= xgene_sec_hash_update,
	 .final  		= xgene_sec_hash_final,
	 .digest 		= xgene_sec_hash_digest,
	 }},

	{.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
         .halg.base      	= {
	 .cra_name 		= "hmac(md5)",
	 .cra_driver_name 	= "hmac-md5-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* MD5-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ahash_type },
	 .halg.digestsize 	= 16,	/* Digest is 128-bits */
	 .init 			= xgene_sec_hash_init,
	 .update 		= xgene_sec_hash_update,
	 .final 		= xgene_sec_hash_final,
	 .digest 		= xgene_sec_hash_digest,
	 .setkey 		= xgene_sec_md5_hmac_setkey,
	 }},

       {.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
        .halg.base            = {
        .cra_name              = "hmac(sha1)",
        .cra_driver_name       = "hmac-sha1-xgene",
        .cra_flags             = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
        .cra_blocksize         = 64,   /* SHA1-HMAC block size is 512-bits */
        .cra_ctxsize           = sizeof(struct xgene_sec_session_ctx),
        .cra_alignmask         = 0,    /* Hardware requires 16 bytes aligned */
        .cra_type              = &crypto_ahash_type },
        .halg.digestsize       = 20,   /* Disgest is 160-bits */
        .init                  = xgene_sec_hash_init,
        .update                = xgene_sec_hash_update,
        .final                 = xgene_sec_hash_final,
        .digest                = xgene_sec_hash_digest,
        .setkey                = xgene_sec_sha1_hmac_setkey,
        }},

	{.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
         .halg.base      	= {
	 .cra_name 		= "hmac(sha224)",
	 .cra_driver_name 	= "hmac-sha224-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* SHA224-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ahash_type },
	 .halg.digestsize 	= 28,	/* Disgest is 224-bits */
	 .init   		= xgene_sec_hash_init,
	 .update 		= xgene_sec_hash_update,
	 .final  		= xgene_sec_hash_final,
	 .digest 		= xgene_sec_hash_digest,
	 .setkey 		= xgene_sec_sha2_hmac_setkey,
	 }},

       {.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
	.halg.base      	= {
	.cra_name 		= "hmac(sha256)",
	.cra_driver_name 	= "hmac-sha256-xgene",
	.cra_flags 		= CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
	.cra_blocksize 		= 64,	/* SHA256-HMAC block size is 512-bits */
	.cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	.cra_alignmask 		= 0,	/* Hardware requires 16 bytes aligned */
	.cra_type 		= &crypto_ahash_type },
	.halg.digestsize 	= 32,	/* Disgest is 256-bits */
	.init   		= xgene_sec_hash_init,
	.update 		= xgene_sec_hash_update,
	.final  		= xgene_sec_hash_final,
	.digest 		= xgene_sec_hash_digest,
	.setkey 		= xgene_sec_sha2_hmac_setkey,
	}},

	{.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
         .halg.base      	= {
	 .cra_name 		= "xcbc(aes)",
	 .cra_driver_name	= "xcbc-aes-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* XCBC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ahash_type },
	 .halg.digestsize 	= 16,	/* MAC (message auth code) is 128-bits*/
	 .init   		= xgene_sec_hash_init,
	 .update 		= xgene_sec_hash_update,
	 .final  		= xgene_sec_hash_final,
	 .digest 		= xgene_sec_hash_digest,
	 .setkey 		= xgene_sec_sha2_xcbc_setkey,
	 }},

	/* Basic Crypto Alg */
	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
	 .cra_name		= "ecb(aes)",
	 .cra_driver_name	= "ecb-aes-xgene",
	 .cra_flags		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize		= 16,   /* 128-bits block */
	 .cra_ctxsize		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask		= 0xF,  /* Hardware requires 16 bytes aligned */
	 .cra_type		= &crypto_ablkcipher_type,
	 .cra_u			= {.ablkcipher = {
	 .min_keysize		= 16,   /* AES min key size is 128-bits */
	 .max_keysize		= 32,   /* AES max key size is 256-bits */
	 .setkey		= xgene_sec_setkey_aes_ecb,
	 .encrypt		= xgene_sec_encrypt,
	 .decrypt		= xgene_sec_decrypt,
	 }}}},

	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
         .cra_name 		= "ecb(des)",
	 .cra_driver_name 	= "ecb-des-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 64-bit block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 8,	/* DES key is 64-bits */
	 .max_keysize 		= 8,	/* DES key is 64-bits */
	 .setkey 		= xgene_sec_setkey_3des_ecb,
	 .encrypt 		= xgene_sec_encrypt,
	 .decrypt 		= xgene_sec_decrypt,
	 }}}},

	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
         .cra_name 		= "ecb(des3_ede)",
	 .cra_driver_name 	= "ecb-des3-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 64-bit block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 24,	/* 3DES key is 192-bits */
	 .max_keysize 		= 24,	/* 3DES key is 192-bits */
	 .setkey 		= xgene_sec_setkey_3des_ecb,
	 .encrypt 		= xgene_sec_encrypt,
	 .decrypt 		= xgene_sec_decrypt,
	 }}}},

	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
	 .cra_name		= "cbc(aes)",
	 .cra_driver_name	= "cbc-aes-xgene",
	 .cra_flags		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize		= 16,   /* 128-bits block */
	 .cra_ctxsize		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask		= 0xF,  /* Hardware requires 16 bytes aligned */
	 .cra_type		= &crypto_ablkcipher_type,
	 .cra_u			= {.ablkcipher = {
	 .min_keysize		= 16,   /* AES min key size is 128-bits */
	 .max_keysize		= 32,   /* AES max key size is 256-bits */
	 .ivsize		= 16,
	 .setkey		= xgene_sec_setkey_aes_cbc,
	 .encrypt		= xgene_sec_encrypt,
	 .decrypt		= xgene_sec_decrypt,
	 }}}},

       {.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
        .cra_name 		= "cbc(des3_ede)",
	.cra_driver_name 	= "cbc-des3-xgene",
	.cra_flags		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize		= 8,	/* 64-bit block */
	.cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	.cra_alignmask		= 0xF,	/* Hardware requires 16 bytes aligned */
	.cra_type 		= &crypto_ablkcipher_type,
	.cra_u			= {.ablkcipher = {
	.min_keysize 		= 24,	/* 3DES key is 192-bits */
	.max_keysize 		= 24,	/* 3DES key is 192-bits */
	.ivsize 		= 8,	/* IV size is 8 bytes */
	.setkey 		= xgene_sec_setkey_3des_cbc,
	.encrypt 		= xgene_sec_encrypt,
	.decrypt 		= xgene_sec_decrypt,
	}}}},

	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
         .cra_name		= "cfb(des)",
	 .cra_driver_name	= "cfb-des-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize		= 8,	/* 64-bit block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 8,	/* DES key is 64-bits */
	 .max_keysize 		= 8,	/* DES key is 64-bits */
	 .ivsize 		= 8,	/* IV size is 8 bytes */
	 .setkey 		= xgene_sec_setkey_3des_cfb,
	 .encrypt 		= xgene_sec_encrypt,
	 .decrypt 		= xgene_sec_decrypt,
	 }}}},

	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
         .cra_name		= "cfb(des3_ede)",
	 .cra_driver_name	= "cfb-des3-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize		= 8,	/* 64-bit block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 24,	/* 3DES key is 192-bits */
	 .max_keysize 		= 24,	/* 3DES key is 192-bits */
	 .ivsize 		= 8,	/* IV size is 8 bytes */
	 .setkey 		= xgene_sec_setkey_3des_cfb,
	 .encrypt 		= xgene_sec_encrypt,
	 .decrypt 		= xgene_sec_decrypt,
	 }}}},

	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
         .cra_name 		= "ofb(aes)",
	 .cra_driver_name 	= "ofb-aes-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bit block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 16,	/* AES min key is 128-bits */
	 .max_keysize 		= 32,	/* AES max key is 256-bits */
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .setkey 		= xgene_sec_setkey_aes_ofb,
	 .encrypt 		= xgene_sec_encrypt,
	 .decrypt 		= xgene_sec_decrypt,
	 }}}},

	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
         .cra_name 		= "ofb(des)",
	 .cra_driver_name 	= "ofb-des-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 64-bit block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 8,	/* DES key is 64-bits */
	 .max_keysize 		= 8,	/* DES key is 64-bits */
	 .ivsize 		= 8,	/* IV size is 8 bytes */
	 .setkey 		= xgene_sec_setkey_3des_ofb,
	 .encrypt 		= xgene_sec_encrypt,
	 .decrypt 		= xgene_sec_decrypt,
	 }}}},

	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
         .cra_name 		= "ofb(des3_ede)",
	 .cra_driver_name 	= "ofb-des3-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize		= 8,	/* 64-bit block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 24,	/* 3DES key is 192-bits */
	 .max_keysize 		= 24,	/* 3DES key is 192-bits */
	 .ivsize 		= 8,	/* IV size is 8 bytes */
	 .setkey 		= xgene_sec_setkey_3des_ofb,
	 .encrypt 		= xgene_sec_encrypt,
	 .decrypt 		= xgene_sec_decrypt,
	 }}}},

	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
         .cra_name 		= "rfc3686(ctr(aes))",
	 .cra_driver_name 	= "ctr-aes-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 16,	/* AES min key size is 128-bits */
	 .max_keysize 		= 32,	/* AES max key size is 256-bits */
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .setkey 		= xgene_sec_setkey_aes_ctr,
	 .encrypt 		= xgene_sec_encrypt,
	 .decrypt 		= xgene_sec_decrypt,
	 }}}},

	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
	 .cra_name 		= "icm(aes)",
	 .cra_driver_name 	= "icm-aes-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 16,	/* AES min key size is 128-bits */
	 .max_keysize 		= 32,	/* AES max key size is 256-bits */
	 .setkey 		= xgene_sec_setkey_aes_icm,
	 .encrypt 		= xgene_sec_encrypt,
	 .decrypt 		= xgene_sec_decrypt,
	 }}}},

	/* Kasumi */
	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
	 .cra_name 		= "kasumi",
	 .cra_driver_name 	= "kasumi-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 64-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 byte aligned */
	 .cra_type 		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 16,	/* Kasumi  min key size is 128-bits */
	 .max_keysize 		= 16,	/* Kasumi  max key size is 128-bits */
	 .setkey 		= xgene_sec_setkey_kasumi_p,
	 .encrypt 		= xgene_sec_encrypt,
	 .decrypt 		= xgene_sec_decrypt,
	 }}}},

	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
         .cra_name 		= "f8(kasumi)",
	 .cra_driver_name 	= "f8-kasumi-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 64-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 byte aligned */
	 .cra_type 		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 16,	/* Kasumi  min key size is 128-bits */
	 .max_keysize 		= 16,	/* Kasumi  max key size is 128-bits */
	 .ivsize 		= 8,	/* IV size is 8 bytes */
	 .setkey 		= xgene_sec_setkey_kasumi_f8,
	 .encrypt 		= xgene_sec_encrypt,
	 .decrypt 		= xgene_sec_decrypt,
	 }}}},

	{.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
	 .halg.base 		= {
	 .cra_name 		= "f9(kasumi)",
	 .cra_driver_name 	= "f9-kasumi-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* F9 Kasumi block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask		= 0,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ahash_type },
	 .halg.digestsize 	= 4,	/* MAC (message auth code) is 32-bits*/
	 .init   		= xgene_sec_hash_init,
	 .update 		= xgene_sec_hash_update,
	 .final  		= xgene_sec_hash_final,
	 .digest 		= xgene_sec_hash_digest,
	 .setkey 		= xgene_sec_kasumi_f9_setkey,
	 }},

	/* Crypto ARC4 - stateless */
	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
         .cra_name 		= "ecb(arc4)",
	 .cra_driver_name 	= "arc4-stateless-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 1,	/* 8-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 5,	/* ARC4 min key size is 40-bits */
	 .max_keysize 		= 16,	/* ARC4 max key size is 128-bits */
	 .setkey 		= xgene_sec_setkey_arc4_stateless,
	 .encrypt 		= xgene_sec_arc4_encrypt,
	 .decrypt 		= xgene_sec_arc4_decrypt,
	 }}}},

	/* Crypto ARC4 - statefull */
	{.type = CRYPTO_ALG_TYPE_ABLKCIPHER, .u.cipher = {
         .cra_name 		= "cbc(arc4)",
	 .cra_driver_name 	= "arc4-statefull-xgene",
	 .cra_flags 		= CRYPTO_ALG_TYPE_BLKCIPHER | CRYPTO_ALG_ASYNC,
	 .cra_blocksize		= 1,	/* 8-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type		= &crypto_ablkcipher_type,
	 .cra_u 		= {.ablkcipher = {
	 .min_keysize 		= 5,	/* ARC4 min key size is 40-bits */
	 .max_keysize 		= 16,	/* ARC4 max key size is 128-bits */
	 .setkey 		= xgene_sec_setkey_arc4_statefull,
	 .encrypt 		= xgene_sec_arc4_encrypt,
	 .decrypt 		= xgene_sec_arc4_decrypt,
	 }}}},

	/* IPSec combined hash and crypto Algorithms */
	{.type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	.cra_name		= "authenc(hmac(md5),cbc(aes))",
	.cra_driver_name 	= "authenc-hmac-md5-cbc-aes-xgene",
	.cra_flags		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	.cra_blocksize		= 16,	/* 128-bits block */
	.cra_ctxsize		= sizeof(struct xgene_sec_session_ctx),
	.cra_alignmask		= 0xF,	/* Hardware requires 16 bytes aligned */
	.cra_type		= &crypto_aead_type,
	.cra_u			= {
	.aead			= {
	.ivsize			= 16,	/* IV size is 16 bytes */
	.maxauthsize		= 16,	/* Max auth data size in bytes */
	.setkey			= xgene_sec_setkey_aes_md5_authenc,
	.setauthsize		= xgene_sec_setauthsize_aes,
	.encrypt		= xgene_sec_encrypt_cbc_authenc,
	.decrypt		= xgene_sec_decrypt_cbc_authenc,
	.givencrypt		= xgene_sec_givencrypt_cbc_authenc,
	.givdecrypt		= xgene_sec_givendecrypt_cbc_authenc,
	}}}},

	{.type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	.cra_name		= "authenc(hmac(sha1),cbc(aes))",
	.cra_driver_name 	= "authenc-hmac-sha1-cbc-aes-xgene",
	.cra_flags		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	.cra_blocksize	= 16,	/* 128-bits block */
	.cra_ctxsize	= sizeof(struct xgene_sec_session_ctx),
	.cra_alignmask	= 0xF,	/* Hardware requires 16 bytes aligned */
	.cra_type		= &crypto_aead_type,
	.cra_u			= {
	.aead			= {
	.ivsize			= 16,	/* IV size is 16 bytes */
	.maxauthsize	= 20,	/* Max auth data size in bytes */
	.setkey			= xgene_sec_setkey_aes_sha1_authenc,
	.setauthsize	= xgene_sec_setauthsize_aes,
	.encrypt		= xgene_sec_encrypt_cbc_authenc,
	.decrypt		= xgene_sec_decrypt_cbc_authenc,
	.givencrypt		= xgene_sec_givencrypt_cbc_authenc,
	.givdecrypt		= xgene_sec_givendecrypt_cbc_authenc,
	}}}},

	{.type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	.cra_name		= "authenc(hmac(sha224),cbc(aes))",
	.cra_driver_name 	= "authenc-hmac-sha224-cbc-aes-xgene",
	.cra_flags		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	.cra_blocksize		= 16,	/* 128-bits block */
	.cra_ctxsize		= sizeof(struct xgene_sec_session_ctx),
	.cra_alignmask		= 0xF,	/* Hardware requires 16 bytes aligned */
	.cra_type		= &crypto_aead_type,
	.cra_u			= {
	.aead			= {
	.ivsize			= 16,	/* IV size is 16 bytes */
	.maxauthsize		= 28,	/* Max auth data size in bytes */
	.setkey			= xgene_sec_setkey_aes_sha224_authenc,
	.setauthsize		= xgene_sec_setauthsize_aes,
	.encrypt		= xgene_sec_encrypt_cbc_authenc,
	.decrypt		= xgene_sec_decrypt_cbc_authenc,
	.givencrypt		= xgene_sec_givencrypt_cbc_authenc,
	.givdecrypt		= xgene_sec_givendecrypt_cbc_authenc,
	}}}},

	{.type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	.cra_name		= "authenc(hmac(sha256),cbc(aes))",
	.cra_driver_name	= "authenc-hmac-sha256-cbc-aes-xgene",
	.cra_flags		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	.cra_blocksize		= 16,	/* 128-bits block */
	.cra_ctxsize		= sizeof(struct xgene_sec_session_ctx),
	.cra_alignmask		= 0xF,	/* Hardware requires 16 bytes aligned */
	.cra_type		= &crypto_aead_type,
	.cra_u			= {.aead = {
	.ivsize			= 16,	/* IV size is 16 bytes */
	.maxauthsize		= 32,	/* Max auth data size in bytes */
	.setkey			= xgene_sec_setkey_aes_sha256_authenc,
	.setauthsize		= xgene_sec_setauthsize_aes,
	.encrypt		= xgene_sec_encrypt_cbc_authenc,
	.decrypt		= xgene_sec_decrypt_cbc_authenc,
	.givencrypt		= xgene_sec_givencrypt_cbc_authenc,
	.givdecrypt		= xgene_sec_givendecrypt_cbc_authenc,
	}}}},

	{.type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	.cra_name		= "authenc(hmac(md5),cbc(des3_ede))",
	.cra_driver_name	= "authenc-hmac-md5-cbc-des3-xgene",
	.cra_flags		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	.cra_blocksize		= 16,	/* 128-bits block */
	.cra_ctxsize		= sizeof(struct xgene_sec_session_ctx),
	.cra_alignmask		= 0xF,	/* Hardware requires 16 bytes aligned */
	.cra_type		= &crypto_aead_type,
	.cra_u			= {.aead = {
	.ivsize			= 8,	/* IV size is 8 bytes */
	.maxauthsize		= 16,	/* Max auth data size in bytes */
	.setkey			= xgene_sec_setkey_3des_md5_authenc,
	.setauthsize		= xgene_sec_setauthsize_aes,
	.encrypt		= xgene_sec_encrypt_cbc_authenc,
	.decrypt		= xgene_sec_decrypt_cbc_authenc,
	.givencrypt		= xgene_sec_givencrypt_cbc_authenc,
	.givdecrypt		= xgene_sec_givendecrypt_cbc_authenc,
	}}}},

	{.type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	.cra_name		= "authenc(hmac(sha1),cbc(des3_ede))",
	.cra_driver_name	= "authenc-hmac-sha1-cbc-des3-xgene",
	.cra_flags		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	.cra_blocksize		= 16,	/* 128-bits block */
	.cra_ctxsize		= sizeof(struct xgene_sec_session_ctx),
	.cra_alignmask		= 0xF,	/* Hardware requires 16 bytes aligned */
	.cra_type		= &crypto_aead_type,
	.cra_u			= {.aead = {
	.ivsize			= 8,	/* IV size is 8 bytes */
	.maxauthsize		= 20,	/* Max auth data size in bytes */
	.setkey			= xgene_sec_setkey_3des_sha1_authenc,
	.setauthsize		= xgene_sec_setauthsize_aes,
	.encrypt		= xgene_sec_encrypt_cbc_authenc,
	.decrypt		= xgene_sec_decrypt_cbc_authenc,
	.givencrypt		= xgene_sec_givencrypt_cbc_authenc,
	.givdecrypt		= xgene_sec_givendecrypt_cbc_authenc,
	}}}},

	{.type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	.cra_name		= "authenc(hmac(sha224),cbc(des3_ede))",
	.cra_driver_name	= "authenc-hmac-sha224-cbc-des3-xgene",
	.cra_flags		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	.cra_blocksize		= 16,	/* 128-bits block */
	.cra_ctxsize		= sizeof(struct xgene_sec_session_ctx),
	.cra_alignmask		= 0xF,	/* Hardware requires 16 bytes aligned */
	.cra_type		= &crypto_aead_type,
	.cra_u			= {.aead = {
	.ivsize			= 8,	/* IV size is 8 bytes */
	.maxauthsize		= 28,	/* Max auth data size in bytes */
	.setkey			= xgene_sec_setkey_3des_sha224_authenc,
	.setauthsize		= xgene_sec_setauthsize_aes,
	.encrypt		= xgene_sec_encrypt_cbc_authenc,
	.decrypt		= xgene_sec_decrypt_cbc_authenc,
	.givencrypt		= xgene_sec_givencrypt_cbc_authenc,
	.givdecrypt		= xgene_sec_givendecrypt_cbc_authenc,
	}}}},

	{.type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	.cra_name		= "authenc(hmac(sha256),cbc(des3_ede))",
	.cra_driver_name	= "authenc-hmac-sha256-cbc-des3-xgene",
	.cra_flags		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	.cra_blocksize		= 16,	/* 128-bits block */
	.cra_ctxsize		= sizeof(struct xgene_sec_session_ctx),
	.cra_alignmask		= 0xF,	/* Hardware requires 16 bytes aligned */
	.cra_type		= &crypto_aead_type,
	.cra_u			= {.aead = {
	.ivsize			= 8,	/* IV size is 8 bytes */
	.maxauthsize		= 32,	/* Max auth data size in bytes */
	.setkey			= xgene_sec_setkey_3des_sha256_authenc,
	.setauthsize		= xgene_sec_setauthsize_aes,
	.encrypt		= xgene_sec_encrypt_cbc_authenc,
	.decrypt		= xgene_sec_decrypt_cbc_authenc,
	.givencrypt		= xgene_sec_givencrypt_cbc_authenc,
	.givdecrypt		= xgene_sec_givendecrypt_cbc_authenc,
	}}}},

	/* GCM Alg Decryption icv Failing  FIXME*/
	{.type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	.cra_name		= "gcm(aes)",
	.cra_driver_name	= "gcm-aes-xgene",
	.cra_flags		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	.cra_blocksize		= 16,	/* 128-bits block */
	.cra_ctxsize		= sizeof(struct xgene_sec_session_ctx),
	.cra_alignmask		= 0xF,	/* Hardware requires 16 bytes aligned */
	.cra_type		= &crypto_aead_type,
	.cra_u			= {.aead = {
	.ivsize			= 12,	/* IV size is 16 bytes */
	.maxauthsize		= 16,	/* Max auth data size in bytes */
	.setkey			= xgene_sec_setkey_aes_gcm,
	.setauthsize		= xgene_sec_setauthsize_aes_gcm,
	.encrypt		= xgene_sec_encrypt_aes_gcm,
	.decrypt		= xgene_sec_decrypt_aes_gcm,
	.givencrypt		= xgene_sec_givencrypt_aes_gcm,
	.givdecrypt		= xgene_sec_givdecrypt_aes_gcm,
	}}}},
	/* Terminator */
	{.type = 0 }
};

#if 0
#include <crypto/algapi.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mod_devicetable.h>
#include <linux/spinlock_types.h>
#include <linux/highmem.h>
#include <linux/scatterlist.h>
#include <linux/interrupt.h>
#include <linux/crypto.h>
#include <linux/rtnetlink.h>
#include <linux/ip.h>
#include <crypto/aead.h>
#include <crypto/des.h>
#include <crypto/authenc.h>

static int xgene_sec_setauthsize_aes_ccm(struct crypto_aead *ciper,
					 unsigned int authsize)
{
	struct aead_tfm *tfm = crypto_aead_crt(ciper);

	switch (authsize) {
	case 4:
	case 6:
	case 8:
	case 10:
	case 12:
	case 14:
	case 16:
		break;
	default:
		return -EINVAL;
	}

	tfm->authsize = authsize;
	return 0;
}

static int xgene_sec_setauthsize_aes_gcm_rfc4106(struct crypto_aead *ciper,
						 unsigned int authsize)
{
	struct aead_tfm *tfm = crypto_aead_crt(ciper);

	switch (authsize) {
	case 8:
	case 12:
	case 16:
		break;
	default:
		return -EINVAL;
	}

	tfm->authsize = authsize;
	return 0;
}
int ccm_count = 0;

/**
 * Basic CCM Algorithm Routines
 *
 */
static int xgene_sec_setkey_aes_ccm(struct crypto_aead *cipher,
				const unsigned char *key,
				unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	int rc = 0;

	if (keylen != 128/8 && keylen != 192/8 && keylen != 256/8) {
		crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	/* Key changed, musts release all pre-allocate token and sa */
	xgene_sec_free_sa_tkn_pool(session);
	/* Re-create token and SA pool */
	rc = xgene_sec_create_sa_tkn_pool(session,
			SA_AES_CCM_MAX_LEN, SA_AES_CCM_LEN(keylen),
			0, TKN_AES_CCM_MAX);
	if (rc != 0)
		goto err_nomem;

	session->sa->sa_ptr->hash_alg   	= SA_HASH_ALG_XCBC128;
	session->sa->sa_ptr->crypto_alg 	= SA_CRYPTO_ALG_AES128;
	session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HASH;
	session->sa->sa_ptr->key 		= 1;
	session->sa->sa_ptr->cryptomode_feedback =
				SA_CRYPTO_MODE_AES_3DES_CTR_LOAD_REUSE;
	session->sa->sa_ptr->spi		= 0;
	session->sa->sa_ptr->seq		= SA_SEQ_NO_COUNTER;
	session->sa->sa_ptr->mask		= SA_MASK_NONE;
	session->sa->sa_ptr->iv03		= 0xF;
	session->sa->sa_ptr->enc_hash_result= 1;
	session->sa->sa_ptr->context_length = (session->sa_len >> 2) - 2;

	sec_sa_setbe(SA_KEY(session->sa->sa_ptr), key, keylen);
	memset(SA_DIGEST(session->sa->sa_ptr), 0, 16 * 2);
	sec_sa_setle(SA_DIGEST(session->sa->sa_ptr) + 8, key, keylen);
	/* clear crypto block A0 */
	memset(SA_IV(session->sa->sa_ptr), 0, 16);

	//printk("alg setkey %d\n", session->sa_len);
	//ccm_count++;
	//if ((ccm_count == 8))
	//	printk(".");
	//if ((ccm_count == 2))
	//	printk("/");

	return rc;

err_nomem:
	xgene_sec_free_sa_tkn_pool(session);
	return rc;
}

static int xgene_sec_encrypt_aes_ccm(struct aead_request *req)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct crypto_aead 	 *aead = crypto_aead_reqtfm(req);
	struct sec_tkn_input_hdr *in_tkn;
	unsigned char	iv_B0[16];
	unsigned char	adata_length[8];
	u32		adata_length_length 	= 0;
	unsigned char	*adata 			= NULL;
	unsigned char	new_tkn;
	int		rc;
	void *sa_align;
#if defined(APM_SEC_TXDEBUG) || defined(APM_SEC_SATKNDEBUG)
	struct crypto_alg 	*alg = req->base.tfm->__crt_alg;
#endif
	APMSEC_TXLOG("encrypt ccm %s nbytes %d", alg->cra_name, req->cryptlen);

	/* Support only AAD data size of 0, 8, 12, and 16 */
#ifdef APM_SEC_DEBUG
	if (req->assoclen != 0 && req->assoclen != 8 &&
	    req->assoclen != 12 && req->assoclen != 16) {
		APMSEC_TXLOG("Algorithm AES-CCM unsupported adata length %d",
			req->assoclen);
		return -EINVAL;
	}
	/* Support only counter field length of 2 and 4 bytes */
	if (req->iv[0] != 1 && req->iv[0] != 3) {
		APMSEC_TXLOG("algorithm AES-CCM unsupported "
				"counter length %d",
			req->iv[0] & 0x7);
		return -EINVAL;
	}
#endif
	/* Allocate SA and token for this request */
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	rctx->sa  = xgene_sec_sa_get(session);
	if (rctx->sa == NULL) {
		rc = -ENOMEM;
		goto err_enc_aes_ccm;
	}
	memcpy(rctx->sa->sa_ptr, session->sa->sa_ptr, session->sa_len);

	/* Form IV B0 salt and flags for hashing */
	memcpy(iv_B0, req->iv, 16);
	iv_B0[0] |= (8 * ((crypto_aead_authsize(aead) - 2) / 2));
	if (req->assoclen)
		iv_B0[0] |= 64;
	rc = sec_sa_set_aes_ccm_msg_len(iv_B0 + 16 - (req->iv[0] + 1),
					req->cryptlen, req->iv[0] + 1);
	if (rc != 0) {
		rc = -EOVERFLOW;
		goto err_enc_aes_ccm;
	}

	/* Form adata length field and its length */
	if (req->assoclen)
		adata_length_length = sec_sa_set_aes_ccm_adata(adata_length,
							       req->assoclen);
	/* Note: rfc 3610 and NIST 800-38C require counter of
	 * zero to encrypt auth tag.
	 */
	memset(req->iv + 15 - req->iv[0], 0, req->iv[0] + 1);
	sec_sa_setbe(SA_IV(rctx->sa->sa_ptr), req->iv, 16);

	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	if (req->assoclen) {
		adata = kmap_atomic(sg_page(&req->assoc[0])) +
						req->assoc[0].offset;
	}
	APMSEC_SATKNLOG("alg %s tkn 0x%p outtkn 0x%p intkn 0x%p",
			alg->cra_name, rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	sa_align = SA_PTR_ALIGN(rctx->sa->sa_ptr);
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_crypt_aes_ccm(
					(u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr(sa_align),
					(u32 *) rctx->sa->sa_ptr,
					crypto_aead_authsize(aead),
					(u32 *) iv_B0,
					adata_length, adata_length_length,
					adata, req->assoclen, 1);
	} else {
		sec_tkn_update_crypt_aes_ccm((u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr(sa_align),
					(u32 *) rctx->sa->sa_ptr,
					crypto_aead_authsize(aead),
					(u32 *) iv_B0,
					adata_length, adata_length_length,
					adata, req->assoclen, 1);
	}
	if (adata)
		kunmap_atomic(adata);

	BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
	session->sa_flush_done = 0;
	rctx->tkn->sa		= rctx->sa;
	rctx->tkn->context      = &req->base;
	rctx->tkn->dest_nbytes = req->cryptlen + crypto_aead_authsize(aead);
	/* NOTE: The source and destination addresses are transferred to
	         the QM message right before sending to QM hardware via
	         function xgene_sec_loadbuffer2qmsg. */

	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("encrypt ccm failed error %d", rc);

err_enc_aes_ccm:
	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	if (rctx->sa)
		xgene_sec_sa_free(session, rctx->sa);
	return rc;
}

static int xgene_sec_decrypt_aes_ccm(struct aead_request *req)
{
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct sec_tkn_input_hdr *in_tkn;
	unsigned char	iv_B0[16];
	unsigned char	adata_length[8];
	u32		adata_length_length 	= 0;
	unsigned char	*adata 			= NULL;
	unsigned char	new_tkn;
	int		rc;
	void *sa_align;

#if defined(APM_SEC_SATKNDEBUG)
	struct crypto_alg 	*alg = req->base.tfm->__crt_alg;
#endif
	APMSEC_TXLOG("decrypt ccm %s nbytes %d",
			req->base.tfm->__crt_alg->cra_name, req->cryptlen);

#ifdef APM_SEC_DEBUG
	/* Support only AAD data size of 0, 8, 12, and 16 */
	if (req->assoclen != 0 && req->assoclen != 8 &&
	    req->assoclen != 12 && req->assoclen != 16) {
		APMSEC_RXLOG("algorithm AES-CCM unsupported adata "
				"length %d", req->assoclen);
		return -EINVAL;
	}
	/* Support only counter field length of 2 and 4 bytes */
	if (req->iv[0] != 1 && req->iv[0] != 3) {
		APMSEC_RXLOG("algorithm AES-CCM unsupported counter "
				"length %d", req->iv[0] & 0x7);
		return -EINVAL;
	}
#endif
	/* Allocate SA and token for this request */
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	rctx->sa  = xgene_sec_sa_get(session);
	if (rctx->sa == NULL) {
		rc = -ENOMEM;
		goto err_enc_aes_ccm;
	}
	memcpy(rctx->sa->sa_ptr, session->sa->sa_ptr, session->sa_len);

	/* Form IV B0 salt and flags for hashing */
	memcpy(iv_B0, req->iv, 16);
	/* Note: rfc 3610 and NIST 800-38C require counter of
	 * zero to encrypt auth tag.
	 */
	memset(req->iv + 15 - req->iv[0], 0, req->iv[0] + 1);
	sec_sa_setbe(SA_IV(rctx->sa->sa_ptr), req->iv, 16);

	/* Decrypt format control info per RFC 3610 and NIST Special Publication
	   800-38C */
	iv_B0[0] |= (8 * ((crypto_aead_authsize(aead) - 2) / 2));
	if (req->assoclen)
		iv_B0[0] |= 64;
	rc = sec_sa_set_aes_ccm_msg_len(iv_B0 + 16 - (req->iv[0] + 1),
				req->cryptlen - crypto_aead_authsize(aead),
				req->iv[0] + 1);
	if (rc != 0) {
		rc = -EOVERFLOW;
		goto err_enc_aes_ccm;
	}
	/* Form adata length field and its length */
	if (req->assoclen)
		adata_length_length = sec_sa_set_aes_ccm_adata(adata_length,
							req->assoclen);

	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	if (req->assoclen) {
		adata = kmap_atomic(sg_page(&req->assoc[0])) +
				req->assoc[0].offset;
	}
	APMSEC_SATKNLOG("alg %s tkn 0x%p outtkn 0x%p intkn 0x%p",
			alg->cra_name, rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	sa_align = SA_PTR_ALIGN(rctx->sa->sa_ptr);
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_crypt_aes_ccm(
					(u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr(sa_align),
					(u32 *) rctx->sa->sa_ptr,
					crypto_aead_authsize(aead),
					(u32 *) iv_B0,
					adata_length, adata_length_length,
					adata, req->assoclen, 0);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
	} else {
		sec_tkn_update_crypt_aes_ccm((u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr(sa_align),
					(u32 *) rctx->sa->sa_ptr,
					crypto_aead_authsize(aead),
					(u32 *) iv_B0,
					adata_length, adata_length_length,
					adata, req->assoclen, 0);
	}
	if (adata)
		kunmap_atomic(adata);//);
	session->sa_flush_done = 0;
	rctx->tkn->sa	       = rctx->sa;
	rctx->tkn->context     = &req->base;
	rctx->tkn->dest_nbytes  = req->cryptlen;

	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG(" decrypt ccm failed error %d",
				rc);

err_enc_aes_ccm:
	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	if (rctx->sa)
		xgene_sec_sa_free(session, rctx->sa);
	return rc;
}

static int xgene_sec_givencrypt_aes_ccm(struct aead_givcrypt_request *req)
{	/* TBD */
	return -ENOSYS;
}

static int xgene_sec_givdecrypt_aes_ccm(struct aead_givcrypt_request *req)
{	/* TBD */
	return -ENOSYS;
}


/**
* Setkey Routines for Transport/Tunnel GCM,/AES(CTR) Algorithms
*
*/

static int __xgene_sec_setkey_esp_ctr(struct crypto_aead *cipher,
				const u8 *key,
				unsigned int keylen, unsigned char ca,
				unsigned char ha,
				unsigned char gcm_flag)
{
	struct crypto_tfm    *tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	struct rtattr 	     *rta = (void *) key;
	struct esp_authenc_param {
		__be32 spi;
		__be32 seq;
		__be16 pad_block_size;
		__be16 encap_uhl;
		struct crypto_authenc_key_param authenc_param;
	} *param;
	unsigned int enckeylen;
	unsigned int authkeylen;
	unsigned char dt;
	int rc = -EINVAL;

	if (!RTA_OK(rta, keylen))
		goto badkey;

	param     = RTA_DATA(rta);
	enckeylen = be32_to_cpu(param->authenc_param.enckeylen);

	key    += RTA_ALIGN(rta->rta_len);
	keylen -= RTA_ALIGN(rta->rta_len);
	if (gcm_flag)
		enckeylen = keylen;
	authkeylen = keylen - enckeylen;

	if (keylen < enckeylen)
		goto badkey;

	if (enckeylen != 128/8 + 4 && enckeylen != 192/8 + 4 &&
		enckeylen != 256/8 + 4) {
		crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	enckeylen -= 4;	/* salt (nonce) is at last 4 bytes */

	switch (enckeylen) {
	case 128/8:
		ca = SA_CRYPTO_ALG_AES128;
		break;
	case 192/8:
		ca = SA_CRYPTO_ALG_AES192;
		break;
	case 256/8:
	default:
		ca = SA_CRYPTO_ALG_AES256;
		break;
	}
	/* As key length can change, we need to flush the cache sa/token */
	xgene_sec_free_sa_tkn_pool(session);
	if (gcm_flag) {
		rc = xgene_sec_create_sa_tkn_pool(session,
			SA_AES_GCM_MAX_LEN, SA_AES_GCM_LEN(enckeylen),
			1, IPE_TKN_ESP_TRANSPORT_GCM_MAXSIZE);
	} else {
		dt = SA_DIGEST_TYPE_HMAC;
		rc = xgene_sec_create_sa_tkn_pool(session,
			SA_AES_CTR_MAX_LEN,
			SA_AES_CTR_LEN(enckeylen, ha, dt),
			1, IPE_TKN_ESP_TRANSPORT_GCM_MAXSIZE);
	}

	if (rc != 0)
		goto err_nomem;

	memset(session->sa->sa_ptr, 0, session->sa_len);
	session->sa->sa_ptr->crypto_alg 	= ca;
	session->sa->sa_ptr->hash_alg   	= ha;
	if (gcm_flag)
		session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HASH;
	else
		session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HMAC;
	session->sa->sa_ptr->key 		= 1;
	session->sa->sa_ptr->cryptomode_feedback = SA_CRYPTO_MODE_AES_3DES_CTR;
	session->sa->sa_ptr->ToP		= SA_TOP_ENCRYPT_HASH_OUTBOUND;
	session->sa->sa_ptr->spi		= 1;
	session->sa->sa_ptr->seq		= SA_SEQ_32BITS;
	session->sa->sa_ptr->iv_format	= SA_IV_FORMAT_COUNTER_MODE;
	session->sa->sa_ptr->iv03		= 0x1;
	if (gcm_flag)
		session->sa->sa_ptr->enc_hash_result= 1;

	session->sa->sa_ptr->pad_type	= SA_PAD_TYPE_IPSEC;
	session->sa->sa_ptr->context_length = (session->sa_len >> 2) - 2;
#if 0
	sec_sa_setbe(SA_SPI(session->sa->sa_ptr), param->spi);
	sec_sa_setbe(SA_SEQ(session->sa->sa_ptr), param->seq);
#endif
	sec_sa_set32le(SA_SPI(session->sa->sa_ptr), param->spi);
	sec_sa_set32le(SA_SEQ(session->sa->sa_ptr), param->seq);

	session->pad_block_size = be16_to_cpu(param->pad_block_size);
	session->encap_uhl      = be16_to_cpu(param->encap_uhl);

	APMSEC_DEBUG_DUMP("enc key: ", key + authkeylen, enckeylen);
	APMSEC_DEBUG_DUMP("auth key: ", key, authkeylen);
	if (gcm_flag) {
		sec_sa_setbe(SA_KEY(session->sa->sa_ptr), key, enckeylen);
		/* Save salt into IV0; IV1 & IV2 will be filled later
		 * from request
		 */
		sec_sa_setbe(SA_IV(session->sa->sa_ptr), key + enckeylen, 4);
	} else {
		sec_sa_setbe(SA_KEY(session->sa->sa_ptr), key + authkeylen,
							enckeylen);
		/* Save salt into IV0; IV1 & IV2 will be filled
		 * later from request
		 */
		sec_sa_setbe(SA_IV(session->sa->sa_ptr), key + authkeylen
							+ enckeylen, 4);
	}
	APMSEC_DEBUG("esp sa 0x%p sa_ptr 0x%p sa_ib 0x%p sa_ib_ptr 0x%p",
		session->sa, session->sa->sa_ptr, session->sa_ib, session->sa_ib->sa_ptr);
	if (gcm_flag)
		rc = xgene_compute_gcm_hash_key(tfm,
					(unsigned char *) key, enckeylen);
	else
		rc = xgene_sec_compute_ipad_opad(tfm, key, authkeylen,
					(u8 *) SA_DIGEST(session->sa->sa_ptr));
	if (rc < 0) {
		APMSEC_ERR("returned err in digest calc %d", rc)
		goto err_nomem;
	}

	APMSEC_DEBUGX_DUMP("sa: ", session->sa->sa_ptr, session->sa_len);
	/* Now copy the same one for inbound */
	memcpy(session->sa_ib->sa_ptr, session->sa->sa_ptr, session->sa_len);
	session->sa_ib->sa_ptr->ToP = SA_TOP_HASH_DECRYPT_INBOUND;
	APMSEC_DEBUGX_DUMP("sa_ib: ", session->sa_ib->sa_ptr, session->sa_len);

	APMSEC_DEBUG("alg %s setkey",
			crypto_tfm_alg_name(tfm));
	return 0;

err_nomem:
	xgene_sec_free_sa_tkn_pool(session);
	return rc;

badkey:
	crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return rc;
}

static unsigned char xgene_sec_set_esp_ha(struct crypto_aead *cipher)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(cipher);
	struct crypto_alg *alg  = tfm->__crt_alg;
	struct aead_alg aalg = alg->cra_aead;
	unsigned char ha;

	switch (aalg.maxauthsize) {
		default:
		case 128/8:
			ha = SA_HASH_ALG_MD5;
			break;
		case 160/8:
			ha = SA_HASH_ALG_SHA1;
			break;
		case 256/8:
			ha = SA_HASH_ALG_SHA256;
			break;
		case 224/8:
			ha = SA_HASH_ALG_SHA224;
			break;
		case 512/8:
			ha = SA_HASH_ALG_SHA512;
			break;
		case 384/8:
			ha = SA_HASH_ALG_SHA384;
			break;

	}
	return ha;
}
static int __xgene_sec_setkey_esp(struct crypto_aead *cipher, const u8 *key,
				  unsigned int keylen, unsigned char ca,
				  unsigned char ha)
{
	struct crypto_tfm    *tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	struct rtattr 	     *rta = (void *) key;
	struct esp_authenc_param {
		__be32 spi;
		__be32 seq;
		__be16 pad_block_size;
		__be16 encap_uhl;
		struct crypto_authenc_key_param authenc_param;
	} *param;
	unsigned int enckeylen;
	unsigned int authkeylen;
	unsigned int sa_len;
	unsigned int sa_max_len;
	int rc = -EINVAL;

	if (!RTA_OK(rta, keylen))
		goto badkey;
	if (rta->rta_type != CRYPTO_AUTHENC_KEYA_PARAM)
		goto badkey;
	if (RTA_PAYLOAD(rta) < sizeof(*param))
		goto badkey;

	param     = RTA_DATA(rta);
	enckeylen = be32_to_cpu(param->authenc_param.enckeylen);

	key    += RTA_ALIGN(rta->rta_len);
	keylen -= RTA_ALIGN(rta->rta_len);

	authkeylen = keylen - enckeylen;

	if (keylen < enckeylen)
		goto badkey;
	if (enckeylen != 128/8 && enckeylen != 192/8 && enckeylen != 256/8 &&
	    enckeylen != 64/8)
		goto badkey;
	if (ca == SA_CRYPTO_ALG_RSVD) {
		switch (enckeylen) {
		case 128/8:
			ca = SA_CRYPTO_ALG_AES128;
			break;
		case 192/8:
			ca = SA_CRYPTO_ALG_AES192;
			break;
		case 256/8:
		default:
			ca = SA_CRYPTO_ALG_AES256;
			break;
		}
	}
	if (ca == SA_CRYPTO_ALG_DES) {
		sa_len = SA_AUTHENC_DES_LEN(0 /* IV using PRNG */,
					ha, SA_DIGEST_TYPE_HMAC);
		sa_max_len = SA_AUTHENC_DES_MAX_LEN(
					0 /* IV using PRNG */,
					ha, SA_DIGEST_TYPE_HMAC);
	} else if (ca == SA_CRYPTO_ALG_3DES) {
		sa_len = SA_AUTHENC_3DES_LEN(
					0 /* IV using PRNG */,
					ha, SA_DIGEST_TYPE_HMAC);
		sa_max_len = SA_AUTHENC_3DES_MAX_LEN(
					0 /* IV using PRNG */,
					ha, SA_DIGEST_TYPE_HMAC);
	} else {
		sa_len = SA_AUTHENC_AES_LEN(enckeylen,
					0 /* IV using PRNG */,
					ha, SA_DIGEST_TYPE_HMAC);
		sa_max_len = SA_AUTHENC_AES_MAX_LEN(enckeylen,
					0 /* IV using PRNG */,
					ha, SA_DIGEST_TYPE_HMAC);
	}

	/* As key length can change, we need to flush the cache sa/token */
	xgene_sec_free_sa_tkn_pool(session);
	/* Re-create the SA/token pool */
	/*FIXME*/
	/*  For now this size should work for both transport and Tunnel input and output since
	we do not pass the Outer IP header processing to the Engine */
	rc = xgene_sec_create_sa_tkn_pool(session, sa_max_len, sa_len, 1,
					IPE_TKN_ESP_TRANSPORT_MAXSIZE);

	if (rc != 0)
		goto err_nomem;

	APMSEC_DEBUG("authenc keylen %d authkeylen %d "
		"ca %d ha %d sa_len %d icvsize %d",
		enckeylen, authkeylen, ca, ha, session->sa_len,
		crypto_aead_authsize(cipher));
	APMSEC_DEBUGX_DUMP("authenc key: ", key + authkeylen, enckeylen);
	APMSEC_DEBUGX_DUMP("authenc auth key: ", key, authkeylen);
	/* Setup SA - NOTE: Control word is in host endian to allow easy
	 * variable reference. Control word in token is in little endian.
	 * Even though we setup both SA (inbound and outbound) only one
	 * can be used.
	 */
	memset(session->sa->sa_ptr, 0, session->sa_len);
	memset(session->sa_ib->sa_ptr, 0, session->sa_len);
	session->sa->sa_ptr->crypto_alg 	= ca;
	session->sa->sa_ptr->hash_alg   	= ha;
	session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HMAC;
	session->sa->sa_ptr->key 		= 1;
	session->sa->sa_ptr->cryptomode_feedback = SA_CRYPTO_MODE_AES_3DES_CBC;
	session->sa->sa_ptr->ToP		= SA_TOP_ENCRYPT_HASH_OUTBOUND;
	session->sa->sa_ptr->spi		= 1;
	session->sa->sa_ptr->seq		= SA_SEQ_32BITS;
	session->sa->sa_ptr->iv03		= 0;	/* Use PRNG */
	session->sa->sa_ptr->pad_type	= SA_PAD_TYPE_IPSEC;
	session->sa->sa_ptr->context_length = (sa_len >> 2) - 2;
	sec_sa_setbe(SA_KEY(session->sa->sa_ptr), key + authkeylen, enckeylen);
	sec_sa_set32le(SA_SPI(session->sa->sa_ptr), param->spi);
	sec_sa_set32le(SA_SEQ(session->sa->sa_ptr), param->seq);

	session->pad_block_size = be16_to_cpu(param->pad_block_size);
	session->encap_uhl      = be16_to_cpu(param->encap_uhl);

	/* Pre-compute inner and outer digest using auth key */
	APMSEC_DEBUG("SA 0x%p sa_ptr 0x%p sa_ib 0x%p sa_ib_ptr 0x%p",
		session->sa, session->sa->sa_ptr, session->sa_ib, session->sa_ib->sa_ptr);
	rc = xgene_sec_compute_ipad_opad(tfm, key, authkeylen,
			(unsigned char *) SA_DIGEST(session->sa->sa_ptr));
	if (rc < 0)
		goto err_nomem;
	APMSEC_DEBUGX_DUMP("sa: ", session->sa->sa_ptr, session->sa_len);
	/* Now copy the same one for inbound */
	memcpy(session->sa_ib->sa_ptr, session->sa->sa_ptr, session->sa_len);
	session->sa_ib->sa_ptr->ToP = SA_TOP_HASH_DECRYPT_INBOUND;
	APMSEC_DEBUGX_DUMP("sa_ib: ", session->sa_ib->sa_ptr, session->sa_len);

	APMSEC_DEBUG("alg %s setkey",
			crypto_tfm_alg_name(tfm));
	return 0;

err_nomem:
	xgene_sec_free_sa_tkn_pool(session);
	return rc;

badkey:
	crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return rc;
}

u64 xgene_sec_v2hwaddr_arc4(void *vaddr)
{
//#if !defined(CONFIG_APM86xxx_IOCOHERENT)
//	return (u32) (PADDR(vaddr) >> 4);
//#else
	u64 offset;
	offset = (u64)vaddr - (u64)xgene_sec_dma_addr();
	return (u64) ((xgene_sec_dma_paddr() + offset));
//#endif
}

/** Setkey for SSL/TLS ARC4 alg */
static int _xgene_sec_setkey_ssl_tls_arc4(struct crypto_aead *cipher,
				   const u8 *key,
				   unsigned int keylen, unsigned char ca,
				   unsigned char ha)
{
	struct crypto_tfm	*tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	struct rtattr		*rta = (void *) key;
	struct offload_param {
		__be32 spi;
		__be32 seq_h;
		__be32 seq_l;
		struct crypto_authenc_key_param authenc_param;
	} *param;
	unsigned int enckeylen;
	unsigned int authkeylen;
	unsigned int sa_len;
	unsigned int sa_max_len;
	int i;
	u32 j = 0;
	u32 k = 0;
	u8 *arc4_state;
	int rc = -EINVAL;

	if (!RTA_OK(rta, keylen))
		goto badkey;
	if (rta->rta_type != CRYPTO_AUTHENC_KEYA_PARAM)
		goto badkey;
	if (RTA_PAYLOAD(rta) < sizeof(*param))
		goto badkey;

	param     = RTA_DATA(rta);
	APMSEC_DEBUGX_DUMP("param data: ", param, 64);
	enckeylen = be32_to_cpu(param->authenc_param.enckeylen);
	key    += RTA_ALIGN(rta->rta_len);
	keylen -= RTA_ALIGN(rta->rta_len);
	authkeylen = keylen - enckeylen;

	if (keylen < enckeylen)
		goto badkey;

	sa_len = SA_SSL_TLS_ARC4_LEN(enckeylen);
	sa_max_len = SA_MAX_SSL_TLS_ARC4_LEN;

	/* As key length can change, we need to flush the cache sa/token */
	xgene_sec_free_sa_tkn_pool(session);
	/* Re-create the SA/token pool */
	/*FIXME*/
	/*  For now this size should work for both transport and Tunnel input and output since
	we do not pass the Outer IP header processing to the Engine */
	rc = xgene_sec_create_sa_tkn_pool(session, sa_max_len, sa_len, 1,
					IPE_TKN_SSL_TLS_OUT_MAXSIZE);

	if (rc != 0)
		goto err_nomem;

	APMSEC_DEBUG("authenc keylen %d authkeylen %d "
			"ca %d ha %d sa_len %d icvsize %d",
	enckeylen, authkeylen, ca, ha, session->sa_len,
	crypto_aead_authsize(cipher));
	APMSEC_DEBUGX_DUMP("authenc key: ", key + authkeylen, enckeylen);
	APMSEC_DEBUGX_DUMP("authenc auth key: ", key, authkeylen);

	/* Setup SA - NOTE: Control word is in host endian to allow easy
	* variable reference. Control word in token is in little endian.
	* Even though we setup both SA (inbound and outbound) only one
	* can be used.
	*/

	memset(session->sa->sa_ptr, 0, session->sa_len);
	session->sa->sa_ptr->crypto_alg 	= ca;
	session->sa->sa_ptr->hash_alg   	= ha;
	session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HMAC;
	session->sa->sa_ptr->key 		= 1;
	session->sa->sa_ptr->cryptomode_feedback = enckeylen;
	session->sa->sa_ptr->ToP		= SA_TOP_HASH_ENCRYPT_OUTBOUND;
	session->sa->sa_ptr->spi		= 1;
	session->sa->sa_ptr->seq		= SA_SEQ_64BITS;
	session->sa->sa_ptr->pad_type	= 0;
	session->sa->sa_ptr->iv03 = 0;
	session->sa->sa_ptr->hash_store  = 1;
	session->sa->sa_ptr->crypto_store  = 1;
	session->sa->sa_ptr->context_length = (sa_len >> 2) - 2;
	sec_sa_setbe(SA_KEY(session->sa->sa_ptr), key + authkeylen, enckeylen);
	sec_sa_setbe(SA_DIGEST(session->sa->sa_ptr), key, authkeylen);

	sec_sa_set32le(SA_SPI(session->sa->sa_ptr), param->spi);
	sec_sa_set32le(SA_SEQ(session->sa->sa_ptr), param->seq_l);
	sec_sa_set32le(SA_SEQ(session->sa->sa_ptr) + 1, param->seq_h);
	arc4_state = (u8 *)((u8 *) session->sa->sa_ptr +
			SA_SSL_TLS_ARC4_OFFSET(enckeylen));
	printk("arc4_state  = %p\n", arc4_state);
	sec_sa_set32le(SA_ARC4(session->sa->sa_ptr), xgene_sec_v2hwaddr_arc4(arc4_state));
	printk("SA_ARC4(session->sa->sa_ptr) = %p\n", SA_ARC4(session->sa->sa_ptr));
	sec_sa_set32le(SA_ARC4(session->sa->sa_ptr) + 1, 0);
	printk("setting the arc4 state \n");
		/* Initialize ARC4 state record */
		for (i = 0; i < 256; i++) {
			arc4_state[i] = i;
			printk("arc4 value %d = %d, %p\n", i, arc4_state[i], &arc4_state[i]);
		}
		for (i = 0; i < 256; i++) {
			u8 temp = arc4_state[i];
			j = (j + temp + (key + authkeylen)[k]) & 0xff;
			arc4_state[i] = arc4_state[j];
			arc4_state[j] = temp;
			printk("j = %d; arc4_value = %d, arc4_state %p\n", j, arc4_state[j], &arc4_state[j]);
			if (++k >= enckeylen)
				k = 0;
		}
	/* Pre-compute inner and outer digest using auth key */
	APMSEC_DEBUG("SA 0x%p sa_ptr 0x%p sa_ib 0x%p sa_ib_ptr 0x%p",
		     session->sa, session->sa->sa_ptr, session->sa_ib, session->sa_ib->sa_ptr);
	APMSEC_DEBUGX_DUMP("sa: ", session->sa->sa_ptr, session->sa_len);
	/* Now copy the same one for inbound */
	memcpy(session->sa_ib->sa_ptr, session->sa->sa_ptr, session->sa_len);
	session->sa_ib->sa_ptr->ToP = SA_TOP_DECRYPT_HASH_INBOUND;
	APMSEC_DEBUGX_DUMP("sa_ib: ", session->sa_ib->sa_ptr, session->sa_len);
	//sec_sa_dump((u32 *)session->sa->sa_ptr, session->sa_len);
	APMSEC_DEBUG("alg %s setkey",
		     crypto_tfm_alg_name(tfm));
	return 0;

err_nomem:
		xgene_sec_free_sa_tkn_pool(session);
	return rc;

badkey:
		crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return rc;
}

static int _xgene_sec_setkey_dtls(struct crypto_aead *cipher,
				   const u8 *key,
				   unsigned int keylen, unsigned char ca,
				   unsigned char ha)
{
	struct crypto_tfm	*tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	struct rtattr		*rta = (void *) key;
	struct offload_param {
		__be32 spi;
		__be32 seq_h;
		__be32 seq_l;
		struct crypto_authenc_key_param authenc_param;
	} *param;
	unsigned int enckeylen;
	unsigned int authkeylen;
	unsigned int sa_len;
	unsigned int sa_max_len;
	int rc = -EINVAL;

	if (!RTA_OK(rta, keylen))
		goto badkey;
	if (rta->rta_type != CRYPTO_AUTHENC_KEYA_PARAM)
		goto badkey;
	if (RTA_PAYLOAD(rta) < sizeof(*param))
		goto badkey;

	param     = RTA_DATA(rta);
	APMSEC_DEBUGX_DUMP("param data: ", param, 64);
	enckeylen = be32_to_cpu(param->authenc_param.enckeylen);
	key    += RTA_ALIGN(rta->rta_len);
	keylen -= RTA_ALIGN(rta->rta_len);
	authkeylen = keylen - enckeylen;

	if (keylen < enckeylen)
		goto badkey;
	if (enckeylen != 128/8 && enckeylen != 192/8 && enckeylen != 256/8 &&
		   enckeylen != 64/8)
		goto badkey;
	if (ca == SA_CRYPTO_ALG_RSVD) {
		switch (enckeylen) {
			case 128/8:
				ca = SA_CRYPTO_ALG_AES128;
				break;
			case 192/8:
				ca = SA_CRYPTO_ALG_AES192;
				break;
			case 256/8:
			default:
				ca = SA_CRYPTO_ALG_AES256;
				break;
		}
	}
	if (ca == SA_CRYPTO_ALG_DES) {
		sa_len = SA_DTLS_DES_LEN(ha, SA_DIGEST_TYPE_HMAC);
		sa_max_len = SA_DTLS_DES_MAX_LEN(ha, SA_DIGEST_TYPE_HMAC);
	} else if (ca == SA_CRYPTO_ALG_3DES) {
		sa_len = SA_DTLS_3DES_LEN(ha, SA_DIGEST_TYPE_HMAC);
		sa_max_len = SA_DTLS_3DES_MAX_LEN(ha, SA_DIGEST_TYPE_HMAC);
	} else {
		sa_len = SA_DTLS_AES_LEN(enckeylen,
			SA_CRYPTO_MODE_AES_3DES_CBC /* IV using Context */,
			ha, SA_DIGEST_TYPE_HMAC);
		sa_max_len = SA_DTLS_AES_MAX_LEN(enckeylen,
			SA_CRYPTO_MODE_AES_3DES_CBC,
			ha, SA_DIGEST_TYPE_HMAC);
	}
#if 0
	sa_len = SA_DTLS_LEN(enckeylen);
	sa_max_len = SA_MAX_DTLS_LEN;
#endif
	/* As key length can change, we need to flush the cache sa/token */
	xgene_sec_free_sa_tkn_pool(session);
	/* Re-create the SA/token pool */
	/*FIXME*/
	/*  For now this size should work for both transport and Tunnel input and output since
	we do not pass the Outer IP header processing to the Engine */
	rc = xgene_sec_create_sa_tkn_pool(session, sa_max_len, sa_len, 1,
					IPE_TKN_DTLS_OUT_MAXSIZE);

	if (rc != 0)
		goto err_nomem;

	APMSEC_DEBUG("authenc keylen %d authkeylen %d "
			"ca %d ha %d sa_len %d icvsize %SA_DTLS_3DES_LENd",
	enckeylen, authkeylen, ca, ha, session->sa_len,
	crypto_aead_authsize(cipher));
	APMSEC_DEBUGX_DUMP("authenc key: ", key + authkeylen, enckeylen);
	APMSEC_DEBUGX_DUMP("authenc auth key: ", key, authkeylen);

		/* Setup SA - NOTE: Control word is in host endian to allow easy
	* variable reference. Control word in token is in little endian.
	* Even though we setup both SA (inbound and outbound) only one
	* can be used.
		*/
	memset(session->sa->sa_ptr, 0, session->sa_len);
	session->sa->sa_ptr->crypto_alg 	= ca;
	session->sa->sa_ptr->hash_alg   	= ha;
	session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HMAC;
	session->sa->sa_ptr->key 		= 1;
	session->sa->sa_ptr->cryptomode_feedback = SA_CRYPTO_MODE_AES_3DES_CBC;
	session->sa->sa_ptr->ToP		= SA_TOP_HASH_ENCRYPT_OUTBOUND;
	session->sa->sa_ptr->spi		= 1;
	session->sa->sa_ptr->seq		= SA_SEQ_48BITS;
	session->sa->sa_ptr->iv03		= 0xF;	/* Use SA */
	session->sa->sa_ptr->pad_type	= SA_PAD_TYPE_SSL;
	session->sa->sa_ptr->hash_store  = 1;
	session->sa->sa_ptr->crypto_store  = 1;
	session->sa->sa_ptr->context_length = (sa_len >> 2) - 2;
	sec_sa_setbe(SA_KEY(session->sa->sa_ptr), key + authkeylen, enckeylen);

	/* Load pre-computed inner and outer digest into SA */
	rc = xgene_sec_compute_ipad_opad(tfm, key, keylen,
				       (u8 *) SA_DIGEST(session->sa->sa_ptr));


	sec_sa_set32le(SA_SPI(session->sa->sa_ptr), param->spi);
	sec_sa_set32le(SA_SEQ(session->sa->sa_ptr), param->seq_l);
	sec_sa_set32le(SA_SEQ(session->sa->sa_ptr) + 1, param->seq_h);
	/* Pre-compute inner and outer digest using auth key */
	APMSEC_DEBUG("SA 0x%p sa_ptr 0x%p sa_ib 0x%p sa_ib_ptr 0x%p",
		     session->sa, session->sa->sa_ptr, session->sa_ib, session->sa_ib->sa_ptr);
	APMSEC_DEBUGX_DUMP("sa: ", session->sa->sa_ptr, session->sa_len);
	/* Now copy the same one for inbound */
	memcpy(session->sa_ib->sa_ptr, session->sa->sa_ptr, session->sa_len);
	session->sa_ib->sa_ptr->ToP = SA_TOP_DECRYPT_HASH_INBOUND;
	APMSEC_DEBUGX_DUMP("sa_ib: ", session->sa_ib->sa_ptr, session->sa_len);
	//sec_sa_dump((u32 *)session->sa->sa_ptr, session->sa_len);
	APMSEC_DEBUG("alg %s setkey",
		     crypto_tfm_alg_name(tfm));
	return 0;

err_nomem:
		xgene_sec_free_sa_tkn_pool(session);
	return rc;

badkey:
		crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return rc;
}

static int _xgene_sec_setkey_ssl_tls(struct crypto_aead *cipher,
				  const u8 *key,
				  unsigned int keylen, unsigned char ca,
				  unsigned char ha, int opcode)
{
	struct crypto_tfm	*tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	struct rtattr		*rta = (void *) key;
	struct offload_param {
		__be32 spi;
		__be32 seq_h;
		__be32 seq_l;
		struct crypto_authenc_key_param authenc_param;
	} *param;
	unsigned int enckeylen;
	unsigned int authkeylen;
	unsigned int sa_len;
	unsigned int sa_max_len;
	int rc = -EINVAL;

	if (!RTA_OK(rta, keylen))
		goto badkey;
	if (rta->rta_type != CRYPTO_AUTHENC_KEYA_PARAM)
		goto badkey;
	if (RTA_PAYLOAD(rta) < sizeof(*param))
		goto badkey;

	param     = RTA_DATA(rta);
	APMSEC_DEBUGX_DUMP("param data: ", param, 64);
	enckeylen = be32_to_cpu(param->authenc_param.enckeylen);
	key    += RTA_ALIGN(rta->rta_len);
	keylen -= RTA_ALIGN(rta->rta_len);
	authkeylen = keylen - enckeylen;

	if (keylen < enckeylen)
		goto badkey;
	if (enckeylen != 128/8 && enckeylen != 192/8 && enckeylen != 256/8 &&
		   enckeylen != 64/8)
		goto badkey;
	if (ca == SA_CRYPTO_ALG_RSVD) {
		switch (enckeylen) {
			case 128/8:
				ca = SA_CRYPTO_ALG_AES128;
				break;
			case 192/8:
				ca = SA_CRYPTO_ALG_AES192;
				break;
			case 256/8:
			default:
				ca = SA_CRYPTO_ALG_AES256;
				break;
		}
	}

	sa_len = SA_SSL_TLS_LEN(enckeylen);
	sa_max_len = SA_MAX_SSL_TLS_LEN;

	/* As key length can change, we need to flush the cache sa/token */
	xgene_sec_free_sa_tkn_pool(session);
	/* Re-create the SA/token pool */
	/*FIXME*/
	/*  For now this size should work for both transport and Tunnel input and output since
	we do not pass the Outer IP header processing to the Engine */
	rc = xgene_sec_create_sa_tkn_pool(session, sa_max_len, sa_len, 1,
					IPE_TKN_SSL_TLS_OUT_MAXSIZE);

	if (rc != 0)
		goto err_nomem;

	APMSEC_DEBUG("authenc keylen %d authkeylen %d "
			"ca %d ha %d sa_len %d icvsize %d",
	enckeylen, authkeylen, ca, ha, session->sa_len,
	crypto_aead_authsize(cipher));
	APMSEC_DEBUGX_DUMP("authenc key: ", key + authkeylen, enckeylen);
	APMSEC_DEBUGX_DUMP("authenc auth key: ", key, authkeylen);

		/* Setup SA - NOTE: Control word is in host endian to allow easy
	* variable reference. Control word in token is in little endian.
	* Even though we setup both SA (inbound and outbound) only one
	* can be used.
		*/
	memset(session->sa->sa_ptr, 0, session->sa_len);
	session->sa->sa_ptr->crypto_alg 	= ca;
	session->sa->sa_ptr->hash_alg   	= ha;
	session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HMAC;
	session->sa->sa_ptr->key 		= 1;
	session->sa->sa_ptr->cryptomode_feedback = SA_CRYPTO_MODE_AES_3DES_CBC;
	session->sa->sa_ptr->ToP		= SA_TOP_HASH_ENCRYPT_OUTBOUND;
	session->sa->sa_ptr->spi		= 1;
	session->sa->sa_ptr->seq		= SA_SEQ_64BITS;
	session->sa->sa_ptr->iv03		= 0xF;	/* Use SA */
	session->sa->sa_ptr->pad_type	= SA_PAD_TYPE_SSL;
	session->sa->sa_ptr->hash_store  = 1;
	session->sa->sa_ptr->crypto_store  = 1;
	session->sa->sa_ptr->context_length = (sa_len >> 2) - 2;
	sec_sa_setbe(SA_KEY(session->sa->sa_ptr), key + authkeylen, enckeylen);
	if (opcode == SA_OPCODE_TLS) {
		/* Load pre-computed inner and outer digest into SA */
		rc = xgene_sec_compute_ipad_opad(tfm, key, keylen,
			       (u8 *) SA_DIGEST(session->sa->sa_ptr));
	} else if (opcode == SA_OPCODE_SSL) {
		if (ha == SA_HASH_ALG_MD5) {
			/* Load pre-computed inner and outer digest into SA */
			rc = xgene_sec_compute_ipad_opad(tfm, key, keylen,
				(u8 *) SA_DIGEST(session->sa->sa_ptr));
		} else {
			sec_sa_setbe(SA_DIGEST(session->sa->sa_ptr), key, authkeylen);
		}
	}

	sec_sa_set32le(SA_SPI(session->sa->sa_ptr), param->spi);
	sec_sa_set32le(SA_SEQ(session->sa->sa_ptr), param->seq_l);
	sec_sa_set32le(SA_SEQ(session->sa->sa_ptr) + 1, param->seq_h);
	/* Pre-compute inner and outer digest using auth key */
	APMSEC_DEBUG("SA 0x%p sa_ptr 0x%p sa_ib 0x%p sa_ib_ptr 0x%p",
		     session->sa, session->sa->sa_ptr, session->sa_ib, session->sa_ib->sa_ptr);
	APMSEC_DEBUGX_DUMP("sa: ", session->sa->sa_ptr, session->sa_len);
	/* Now copy the same one for inbound */
	memcpy(session->sa_ib->sa_ptr, session->sa->sa_ptr, session->sa_len);
	session->sa_ib->sa_ptr->ToP = SA_TOP_DECRYPT_HASH_INBOUND;
	APMSEC_DEBUGX_DUMP("sa_ib: ", session->sa_ib->sa_ptr, session->sa_len);
	//sec_sa_dump((u32 *)session->sa->sa_ptr, session->sa_len);
	APMSEC_DEBUG("alg %s setkey",
		     crypto_tfm_alg_name(tfm));
	return 0;

err_nomem:
		xgene_sec_free_sa_tkn_pool(session);
	return rc;

badkey:
		crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return rc;
}

static int _xgene_sec_encrypt_ssl_tls_arc4(struct aead_request *req)
{
	struct xgene_sec_session_ctx     *session;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead 	 *aead;
	struct crypto_alg 	 *alg;
	unsigned char 	new_tkn;
	int		rc;
	int		icvsize;
	int		clen;
	void		*sa_align;
	int ha;

	session  = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);
	aead = crypto_aead_reqtfm(req);
	alg  = req->base.tfm->__crt_alg;

#if defined(APM_SEC_TXDEBUG)
	APMSEC_TXLOG("encrypt  ssl %s nbytes %d",
		     alg->cra_name, req->cryptlen);
#endif
	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	ha = session->sa->sa_ptr->hash_alg;
	icvsize = sec_sa_compute_hash_rlen(ha);
	in_tkn  = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("SSL tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);

	clen = req->cryptlen;
	sa_align = SA_PTR_ALIGN(session->sa->sa_ptr);
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_ssl_tls_out(
				(u32 *) in_tkn,
		req->cryptlen,
		xgene_sec_v2hwaddr(sa_align),
		(u32 *) session->sa->sa_ptr,
		0, SA_SEQ_OFFSET(session->sa->sa_ptr), icvsize, 1, 1,
		SA_ARC4_OFFSET(session->sa->sa_ptr),
		(SA_ARC4_OFFSET(session->sa->sa_ptr) + 1), 0, 0);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
		rctx->tkn->sa	        = session->sa;
	}
	rctx->tkn->context     = &req->base;
	 /* 5 is the size of the SSL Header Type(1) + Version(2) + Length(2) + 1(Pad len field)*/;
	rctx->tkn->dest_nbytes = req->cryptlen + icvsize + 5;


	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("encrypt esp tunnel failed error %d",
			     rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;

}

/**
 * ESP Tunnel Encrypt/Decrypt Routine for all AES/DES/DES3/GCM/AES(CTR) Algs
 *
 */
static int _xgene_sec_encrypt_ssl_tls(struct aead_request *req, int opcode)
{
	struct xgene_sec_session_ctx     *session;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead 	 *aead;
	struct crypto_alg 	 *alg;
	unsigned char 	new_tkn;
	int		rc;
	int		icvsize;
	int		clen;
	void		*sa_align;
	int 		pdl;
	int ha;

	session  = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);
	aead = crypto_aead_reqtfm(req);
	alg  = req->base.tfm->__crt_alg;

#if defined(APM_SEC_TXDEBUG)
	APMSEC_TXLOG("encrypt  ssl %s nbytes %d",
		     alg->cra_name, req->cryptlen);
#endif
	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	ha = session->sa->sa_ptr->hash_alg;
	icvsize = sec_sa_compute_hash_rlen(ha);
	in_tkn  = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("SSL tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);

	clen = req->cryptlen;

	pdl = ALIGN((clen + icvsize + 1), ALIGN(crypto_aead_blocksize(aead), 4));
	pdl -= (clen + icvsize + 1);
	sa_align = SA_PTR_ALIGN(session->sa->sa_ptr);
	sec_sa_setbe(SA_IV(session->sa->sa_ptr), req->iv, 16);

	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_ssl_tls_out(
				(u32 *) in_tkn,
		req->cryptlen,
		xgene_sec_v2hwaddr(sa_align),
		(u32 *) session->sa->sa_ptr,
		pdl, SA_SEQ_OFFSET(session->sa->sa_ptr),
		icvsize, 0, 1, 0, 0, opcode, alg->cra_aead.ivsize);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
		rctx->tkn->sa	        = session->sa;
	}
	rctx->tkn->context     = &req->base;
	 /* 5 is the size of the SSL Header Type(1) + Version(2) + Length(2) + 1(Pad len field)*/
	if (opcode == SA_OPCODE_SSL)
		rctx->tkn->dest_nbytes = req->cryptlen + pdl + icvsize + 5 + 1;
	else /* For TLS add IV to the Output packet */
		rctx->tkn->dest_nbytes = req->cryptlen + pdl + icvsize + 5 + 1 +
				alg->cra_aead.ivsize;


	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("encrypt esp tunnel failed error %d",
			     rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;

}

int xgene_sec_ssl_decrypt_pad_len(const unsigned char *key,
			unsigned int keylen, int payload_len,
			unsigned char *enc_pad, char *iv)

{
	struct crypto_blkcipher *blk_tfm = NULL;
	struct blkcipher_desc   blk_desc;
	struct scatterlist 	sg[1];
	int rc = 0;

	/* Load K2, K3, and K1 digest */
	blk_tfm = crypto_alloc_blkcipher("cbc(aes)", 0, 0);
	if (IS_ERR(blk_tfm)) {
		APMSEC_ERR("failed to load helper transform for cbc(aes) "
				"error %ld", PTR_ERR(blk_tfm));
		return -EINVAL;
	}
	blk_desc.tfm   = blk_tfm;
	blk_desc.flags = 0;
	APMSEC_DEBUGX_DUMP("Payload: ", enc_pad, payload_len);
	APMSEC_DEBUGX_DUMP("dec key: ", key, 16);
	APMSEC_DEBUGX_DUMP("IV: ", iv, 16);

	rc = crypto_blkcipher_setkey(blk_tfm, key, keylen);
	if (rc != 0) {
		APMSEC_ERR("failed to set helper transform key for ecb(aes) "
				"error %d", rc);
		rc = -EINVAL;
		goto err;
	}

	sg_init_one(sg, enc_pad, payload_len);
	crypto_blkcipher_set_iv(blk_tfm, iv, 16);
	rc = crypto_blkcipher_decrypt(&blk_desc, sg, sg, payload_len);
	if (rc != 0) {
		APMSEC_ERR("failed to decrypt hash key for "
				"xcbc key error %d", rc);
		rc = -ENOMEM;
		goto err;
	}
	APMSEC_DEBUGX_DUMP("Output: ", enc_pad, payload_len);

err:
	if (blk_tfm)
	crypto_free_blkcipher(blk_tfm);
	return rc;

}

static int _xgene_sec_decrypt_ssl_tls(struct aead_request *req, int opcode)
{
	struct xgene_sec_session_ctx     *session;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead 	 *aead;
	struct crypto_alg 	 *alg;
	unsigned char 	new_tkn;
	void		*sa_align;
	int ha, pyl, rc, icvsize, pdl;
	void *spage;
	char padlen[32];
	int blocklen = 32;

	session  = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);
	aead = crypto_aead_reqtfm(req);
	alg  = req->base.tfm->__crt_alg;

#if defined(APM_SEC_TXDEBUG)
	APMSEC_TXLOG("decrypt  ssl %s nbytes %d",
		     alg->cra_name, req->cryptlen);
#endif
	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	ha = session->sa_ib->sa_ptr->hash_alg;
	icvsize = sec_sa_compute_hash_rlen(ha);
	in_tkn  = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("SSL tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);

	sec_sa_setbe(SA_IV(session->sa_ib->sa_ptr), req->iv, 16);

	spage = kmap_atomic(sg_page(req->src));//);
	memcpy(padlen, ((spage + req->cryptlen) - blocklen), blocklen);
	kunmap_atomic(spage);

	xgene_sec_ssl_decrypt_pad_len((char *)SA_KEY(session->sa_ib->sa_ptr),
				     16, blocklen,
				     padlen, req->iv);

	pdl = padlen[31] + 1;
	if (opcode == SA_OPCODE_SSL)
		pyl = req->cryptlen - pdl - icvsize - 5;
	else {
		pyl = req->cryptlen - pdl - icvsize - 5 -alg->cra_aead.ivsize ;
	}
	sa_align = SA_PTR_ALIGN(session->sa_ib->sa_ptr);

	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_ssl_tls_in(
		(u32 *) in_tkn,
		req->cryptlen, pyl,
		xgene_sec_v2hwaddr(sa_align),
		(u32 *) session->sa_ib->sa_ptr,
		pdl, SA_SEQ_OFFSET(session->sa_ib->sa_ptr),
		icvsize,
		session->sa_ib->sa_ptr->hash_alg,
		0, opcode, alg->cra_aead.ivsize);
		//BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
		rctx->tkn->sa	        = session->sa_ib;
	}
	rctx->tkn->context     = &req->base;
	 /* 5 is the size of the SSL Header Type(1) + Version(2) + Length(2) + 1(Pad len field)*/;
	rctx->tkn->dest_nbytes = req->cryptlen;

	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("encrypt esp tunnel failed error %d",
			     rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;

}

/**
 * DTLS Encrypt/Decrypt Routine for all AES/DES/DES3 ALgs
 *
 */
static int _xgene_sec_encrypt_dtls(struct aead_request *req)
{
	struct xgene_sec_session_ctx     *session;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead 	 *aead;
	struct crypto_alg 	 *alg;
	unsigned char 	new_tkn;
	int		rc;
	int		icvsize;
	int		clen;
	void		*sa_align;
	int 		pdl;
	int ha, frag_len;

	session  = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);
	aead = crypto_aead_reqtfm(req);
	alg  = req->base.tfm->__crt_alg;

#if defined(APM_SEC_TXDEBUG)
	APMSEC_TXLOG("encrypt dtls %s nbytes %d",
		     alg->cra_name, req->cryptlen);
#endif
	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	ha = session->sa->sa_ptr->hash_alg;
	icvsize = sec_sa_compute_hash_rlen(ha);
	in_tkn  = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("DTLS tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);

	clen = req->cryptlen;

	pdl = ALIGN((clen + icvsize + 1), ALIGN(crypto_aead_blocksize(aead), 4));
	pdl -= (clen + icvsize + 1);
	frag_len = req->cryptlen + pdl + icvsize + alg->cra_aead.ivsize + 1;
	sa_align = SA_PTR_ALIGN(session->sa->sa_ptr);
	sec_sa_setbe(SA_IV(session->sa->sa_ptr), req->iv, 16);

	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_dtls_out(
				(u32 *) in_tkn,
		req->cryptlen,
		xgene_sec_v2hwaddr(sa_align),
		(u32 *) session->sa->sa_ptr,
		pdl, SA_SEQ_OFFSET(session->sa->sa_ptr), icvsize,
		frag_len, alg->cra_aead.ivsize, 1);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
		rctx->tkn->sa	        = session->sa;
	}
	rctx->tkn->context     = &req->base;
	 /* 5 is the size of the SSL Header Type(1) + Version(2) + Length(2) + 1(Pad len field)*/;
	rctx->tkn->dest_nbytes = req->cryptlen + pdl + icvsize + 13 + 1 + alg->cra_aead.ivsize;


	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("encrypt esp tunnel failed error %d",
			     rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;

}

static int _xgene_sec_decrypt_dtls(struct aead_request *req)
{
	struct xgene_sec_session_ctx     *session;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead 	 *aead;
	struct crypto_alg 	 *alg;
	unsigned char 	new_tkn;
	void		*sa_align;
	int ha, pyl, rc, icvsize, pdl;
	void *spage;
	char padlen[32];
	int blocklen = 32;

	session  = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);
	aead = crypto_aead_reqtfm(req);
	alg  = req->base.tfm->__crt_alg;

#if defined(APM_SEC_TXDEBUG)
	APMSEC_TXLOG("decrypt dtls %s nbytes %d",
		     alg->cra_name, req->cryptlen);
#endif
	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	ha = session->sa_ib->sa_ptr->hash_alg;
	icvsize = sec_sa_compute_hash_rlen(ha);
	in_tkn  = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("DTLS tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);

	sec_sa_setbe(SA_IV(session->sa_ib->sa_ptr), req->iv, 16);

	spage = kmap_atomic(sg_page(req->src));//);
	/* Frag len is located at Packet's 11t byte and 12th bye */
	memcpy(padlen, ((spage + req->cryptlen) - blocklen), blocklen);
	kunmap_atomic(spage);

	xgene_sec_ssl_decrypt_pad_len((char *)SA_KEY(session->sa_ib->sa_ptr),
				     16, blocklen,
				     padlen, req->iv);
	pdl = padlen[31] + 1;
	pyl = req->cryptlen - pdl - icvsize - alg->cra_aead.ivsize - 13;
	/* 13 - DTLS Header */;
	sa_align = SA_PTR_ALIGN(session->sa_ib->sa_ptr);

	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_dtls_in(
		(u32 *) in_tkn,
		req->cryptlen, pyl,
		xgene_sec_v2hwaddr(sa_align),
		(u32 *) session->sa_ib->sa_ptr,
		pdl, SA_SEQ_OFFSET(session->sa_ib->sa_ptr),
		icvsize,
		session->sa_ib->sa_ptr->hash_alg, alg->cra_aead.ivsize,
		0);
		rctx->tkn->sa	        = session->sa_ib;
	}
	rctx->tkn->context     = &req->base;
	rctx->tkn->dest_nbytes = req->cryptlen;

	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("encrypt esp tunnel failed error %d",
			     rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;

}

static int xgene_sec_setkey_ssl_aes_sha1(struct crypto_aead *cipher,
				  const u8 *key,
				  unsigned int keylen)
{
	unsigned char ha;

	ha = SA_HASH_ALG_SHA1_SSL_MAC;
	return _xgene_sec_setkey_ssl_tls(cipher, key, keylen,
				       SA_CRYPTO_ALG_RSVD,
				       ha, SA_OPCODE_SSL);
}


static int xgene_sec_setkey_ssl_aes_md5(struct crypto_aead *cipher,
				       const u8 *key,
				       unsigned int keylen)
{
	unsigned char ha;

	ha = SA_HASH_ALG_MD5;
	return _xgene_sec_setkey_ssl_tls(cipher, key, keylen,
				       SA_CRYPTO_ALG_RSVD,
				       ha, SA_OPCODE_SSL);
}

static int xgene_sec_setkey_tls_aes_sha1(struct crypto_aead *cipher,
				       const u8 *key,
				       unsigned int keylen)
{
	unsigned char ha;

	ha = SA_HASH_ALG_SHA1;
	return _xgene_sec_setkey_ssl_tls(cipher, key, keylen,
				       SA_CRYPTO_ALG_RSVD,
				       ha, SA_OPCODE_TLS);
}

static int xgene_sec_setkey_dtls_aes_sha1(struct crypto_aead *cipher,
				  const u8 *key,
				  unsigned int keylen)
{
	unsigned char ha;

	ha = SA_HASH_ALG_SHA1;
	return _xgene_sec_setkey_dtls(cipher, key, keylen,
					SA_CRYPTO_ALG_RSVD,
					ha);
}

static int xgene_sec_setkey_dtls_3des_sha1(struct crypto_aead *cipher,
					const u8 *key,
					unsigned int keylen)
{
	unsigned char ha;

	ha = SA_HASH_ALG_SHA1;
	return _xgene_sec_setkey_dtls(cipher, key, keylen,
				    SA_CRYPTO_ALG_3DES,
				    ha);
}

static int xgene_sec_setkey_dtls_aes_md5(struct crypto_aead *cipher,
					const u8 *key,
					unsigned int keylen)
{
	unsigned char ha;

	ha = SA_HASH_ALG_MD5;
	return _xgene_sec_setkey_dtls(cipher, key, keylen,
				    SA_CRYPTO_ALG_RSVD,
				    ha);
}

static int xgene_sec_setkey_dtls_3des_md5(struct crypto_aead *cipher,
				       const u8 *key,
				       unsigned int keylen)
{
	unsigned char ha;

	ha = SA_HASH_ALG_MD5;
	return _xgene_sec_setkey_dtls(cipher, key, keylen,
				    SA_CRYPTO_ALG_3DES,
				    ha);
}

static int xgene_sec_setkey_ssl_arc4(struct crypto_aead *cipher,
				       const u8 *key,
				       unsigned int keylen)
{
	unsigned char ha;

	ha = SA_HASH_ALG_SHA1_SSL_MAC;
	return _xgene_sec_setkey_ssl_tls_arc4(cipher, key, keylen,
					    SA_CRYPTO_ALG_ARC4,
					    ha);
}

static int xgene_sec_setkey_esp(struct crypto_aead *cipher,
			      const u8 *key,
			      unsigned int keylen)
{
	unsigned char ha;

	ha = xgene_sec_set_esp_ha(cipher);
	return __xgene_sec_setkey_esp(cipher, key, keylen,
				    SA_CRYPTO_ALG_RSVD,
				    ha);
}
int glb_seq = 0;

/**
 * ESP Tunnel Encrypt/Decrypt Routine for all AES/DES/DES3/GCM/AES(CTR) Algs
 *
 */
static int _xgene_sec_encrypt_esp_tunnel(struct aead_request *req, int gcm_flag)
{
	struct xgene_sec_session_ctx     *session;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead 	 *aead;
	struct crypto_alg 	 *alg;
	struct iphdr	*iph;
	unsigned char 	new_tkn;
	int		rc;
	int		icvsize;
	void 		*page;
	int		pdl;
	unsigned char	nh;
	int		clen;
	void *sa_align;

	session  = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);
	aead = crypto_aead_reqtfm(req);
	alg  = req->base.tfm->__crt_alg;

#if defined(APM_SEC_TXDEBUG)
	APMSEC_TXLOG("encrypt esp tunnel %s nbytes %d",
		     alg->cra_name, req->cryptlen);
#endif
	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	icvsize = crypto_aead_authsize(aead);
	in_tkn  = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("ESP Tunnel tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	/* Compute pad len and Next header byte */
	page = kmap_atomic(sg_page(&req->src[0]));//);
	iph   = ((struct iphdr *) (page + req->src[0].offset));
	nh = iph->protocol;
	clen = req->cryptlen;
	kunmap_atomic(page);

	pdl = ALIGN(clen + 2, ALIGN(crypto_aead_blocksize(aead), 4));
	if (session->pad_block_size)
		pdl = ALIGN(pdl, session->pad_block_size);
	pdl -= clen;
	sa_align = SA_PTR_ALIGN(session->sa->sa_ptr);
	/* sec_sa_set32le(SA_SEQ(session->sa->sa_ptr), (++glb_seq)); */
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_espout_tunnel(
				(u32 *) in_tkn,
		req->cryptlen,
		xgene_sec_v2hwaddr(sa_align),
		(u32 *) session->sa->sa_ptr,
		session->sa->sa_ptr->crypto_alg,
		session->sa->sa_ptr->cryptomode_feedback,
		pdl, gcm_flag, SA_SEQ_OFFSET(session->sa->sa_ptr));
	BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
#if 0
		rctx->tkn->devid	= session->dev->dev_id;
		rctx->tkn->priority 	= xgene_sec_COS_DEF;
#endif
		rctx->tkn->sa	        = session->sa;
	} else {
		sec_tkn_update_espout_tunnel((u32 *) in_tkn,
					      req->cryptlen,
					      session->sa->sa_ptr->cryptomode_feedback,
					      icvsize, pdl, gcm_flag);
	}

	rctx->tkn->context     = &req->base;
	rctx->tkn->dest_nbytes = sizeof(struct ip_esp_hdr) +
			alg->cra_aead.ivsize + pdl +
			req->cryptlen
			+ icvsize;
	/* For AES to work need to flush the entire ESP packet */
			/* 44 = Len of IP Header(20) +
			Len of SPI + Len of Seq + Len of IV(16) */
	rctx->tkn->esp_src_len = 20 /* Size of IP Header*/ +
			sizeof(struct ip_esp_hdr) +
			alg->cra_aead.ivsize;
#if 0
	printk("rctx->tkn->dest_nbytes = %d\n", rctx->tkn->dest_nbytes);
	printk("pdl = %d\n", pdl);
	printk("icvsize = %d\n", icvsize);
#endif
	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("encrypt esp tunnel failed error %d",
			     rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;

}

static int _xgene_sec_decrypt_esp_tunnel(struct aead_request *req, int gcm_flag)
{
	struct xgene_sec_session_ctx     *session  = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct crypto_aead 	 *aead = crypto_aead_reqtfm(req);
#ifdef APM_SEC_TXDEBUG
	struct crypto_alg 	 *alg  = req->base.tfm->__crt_alg;
#endif
	struct sec_tkn_input_hdr *in_tkn;
	int		icv_len = crypto_aead_authsize(aead);
	unsigned char 	new_tkn;
	int		rc;
	int		ihl;
	void 		*page;
	void *sa_align;
	int ivsize;

	APMSEC_TXLOG("decrypt esp tunnel %s nbytes %d",
		     alg->cra_name, req->cryptlen);

	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	/* Extract IHL length */
	page = kmap_atomic(sg_page(&req->src[0]));//);
	ihl  = ((struct iphdr *) (page + req->src[0].offset))->ihl << 2;
	kunmap_atomic(page);
	sa_align = SA_PTR_ALIGN(session->sa_ib->sa_ptr);
	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_espin_tunnel(
				(u32 *) in_tkn,
		req->cryptlen,
		xgene_sec_v2hwaddr(sa_align),
		(u32 *) session->sa_ib->sa_ptr,
		session->sa_ib->sa_ptr->crypto_alg,
		session->sa_ib->sa_ptr->cryptomode_feedback, gcm_flag,
		ihl, icv_len, SA_SEQ_OFFSET(session->sa_ib->sa_ptr));
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
#if 0
		rctx->tkn->devid	= session->dev->dev_id;
		rctx->tkn->priority 	= xgene_sec_COS_DEF;
#endif
		rctx->tkn->sa	        = session->sa_ib;
	} else {
		sec_tkn_update_espin_tunnel((u32 *) in_tkn,
					     req->cryptlen,
					     session->sa_ib->sa_ptr->cryptomode_feedback,
					     session->sa_ib->sa_ptr->crypto_alg,
					     icv_len, gcm_flag);
	}
	rctx->tkn->context     = &req->base;
	ivsize = (session->sa_ib->sa_ptr->crypto_alg == SA_CRYPTO_ALG_DES ||
			session->sa_ib->sa_ptr->crypto_alg == SA_CRYPTO_ALG_3DES)
			? 8 : 16;
	if (gcm_flag)
		ivsize = 8;
	rctx->tkn->dest_nbytes = req->cryptlen /* -  icv_len */ - 8 - ivsize;

	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("decrypt ccm failed error %d",
			     rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}
static int xgene_sec_decrypt_esp_tunnel(struct aead_request *req)
{
	return _xgene_sec_decrypt_esp_tunnel(req, 0);
}

static int xgene_sec_encrypt_esp_tunnel(struct aead_request *req)
{
	return _xgene_sec_encrypt_esp_tunnel(req, 0);
}

static int xgene_sec_decrypt_esp_tunnel_gcm(struct aead_request *req)
{
	return _xgene_sec_decrypt_esp_tunnel(req, 1);
}

static int xgene_sec_encrypt_esp_tunnel_gcm(struct aead_request *req)
{
	return _xgene_sec_encrypt_esp_tunnel(req, 1);
}

static int xgene_sec_encrypt_ssl(struct aead_request *req)
{
	int opcode = SA_OPCODE_SSL;
	return _xgene_sec_encrypt_ssl_tls(req, opcode);
}

static int xgene_sec_decrypt_ssl(struct aead_request *req)
{
	int opcode = SA_OPCODE_SSL;
	return _xgene_sec_decrypt_ssl_tls(req, opcode);
}

static int xgene_sec_encrypt_tls1_1(struct aead_request *req)
{
	int opcode = SA_OPCODE_TLS;
	return _xgene_sec_encrypt_ssl_tls(req, opcode);
}

static int xgene_sec_decrypt_tls1_1(struct aead_request *req)
{
	int opcode = SA_OPCODE_TLS;
	return _xgene_sec_decrypt_ssl_tls(req, opcode);
}

static int xgene_sec_encrypt_ssl_tls_arc4(struct aead_request *req)
{
	return _xgene_sec_encrypt_ssl_tls_arc4(req);
}

static int xgene_sec_encrypt_dtls(struct aead_request *req)
{
	return _xgene_sec_encrypt_dtls(req);
}

static int xgene_sec_decrypt_dtls(struct aead_request *req)
{
	return _xgene_sec_decrypt_dtls(req);
}

static int xgene_sec_setkey_esp_gcm(struct crypto_aead *cipher,
				const u8 *key,
				unsigned int keylen)
{
	return __xgene_sec_setkey_esp_ctr(cipher, key, keylen,
					  SA_CRYPTO_ALG_RSVD,
					  SA_HASH_ALG_GHASH, 1);
}

static int xgene_sec_setkey_esp_aes_ctr(struct crypto_aead *cipher,
				const u8 *key,
				unsigned int keylen)
{
	unsigned char ha;
	ha = xgene_sec_set_esp_ha(cipher);
	return __xgene_sec_setkey_esp_ctr(cipher, key, keylen,
					SA_CRYPTO_ALG_RSVD,
					ha, 0);
}

static int xgene_sec_setkey_esp_des3(struct crypto_aead *cipher,
				const unsigned char *key,
				unsigned int keylen)
{
	unsigned char ha;
	ha = xgene_sec_set_esp_ha(cipher);
	return __xgene_sec_setkey_esp(cipher, key, keylen,
					SA_CRYPTO_ALG_3DES,
					ha);

}

static int xgene_sec_setkey_esp_des(struct crypto_aead *cipher,
				const u8 *key,
				unsigned int keylen)
{
	unsigned char ha;
	ha = xgene_sec_set_esp_ha(cipher);

	return __xgene_sec_setkey_esp(cipher, key, keylen,
				SA_CRYPTO_ALG_DES,
				ha);
}


/**
 * ESP Transport Encrypt/Decrypt Routine for AES/DES/DES3/AES(CTR) Algs
 *
 */
static int _xgene_sec_encrypt_esp_transport(struct aead_request *req, int gcm_flag)
{
	struct xgene_sec_session_ctx     *session;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead 	 *aead;
	struct crypto_alg 	 *alg;
	struct iphdr	*iph;
	unsigned char 	new_tkn;
	int		rc;
	int		icvsize;
	void 		*page;
	int		pdl;
	unsigned char	nh;
	unsigned char	iphl;
	int		clen;
	void *sa_align;

	session  = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);
	aead = crypto_aead_reqtfm(req);
	alg  = req->base.tfm->__crt_alg;

#if defined(APM_SEC_TXDEBUG)
	APMSEC_TXLOG("IPE encrypt esp %s nbytes %d",
			alg->cra_name, req->cryptlen);
#endif
	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	icvsize = crypto_aead_authsize(aead);
	in_tkn  = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("IPE tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	/* Compute pad len and Next header byte */
	page = kmap_atomic(sg_page(&req->src[0]));//);
	iph   = ((struct iphdr *) (page + req->src[0].offset));
	nh = iph->protocol;
	iphl = iph->ihl << 2;
	APMSEC_DEBUG("icv_len = %d\n", icvsize);
	APMSEC_DEBUG("nh = %d\n", nh);
	clen = req->cryptlen - iphl;
	kunmap_atomic(page);
	sa_align = SA_PTR_ALIGN(session->sa->sa_ptr);
	pdl = ALIGN(clen + 2, ALIGN(crypto_aead_blocksize(aead), 4));
	if (session->pad_block_size)
		pdl = ALIGN(pdl, session->pad_block_size);
	pdl -= clen;
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_espout_transport(
					(u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr(sa_align),
					(u32 *) session->sa->sa_ptr,
					session->sa->sa_ptr->crypto_alg,
					pdl,
					icvsize,
					nh, iphl,
					SA_SEQ_OFFSET(session->sa->sa_ptr),
					session->sa->sa_ptr->cryptomode_feedback,
					gcm_flag);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
		rctx->tkn->sa	        = session->sa;
	} else {
		sec_tkn_update_espout_transport((u32 *) in_tkn,
					req->cryptlen,
					session->sa_ib->sa_ptr->crypto_alg,
					pdl, nh, iphl, icvsize,
					session->sa->sa_ptr->cryptomode_feedback,
					gcm_flag);
	}

	rctx->tkn->context     = &req->base;
	rctx->tkn->dest_nbytes = sizeof(struct ip_esp_hdr) +
			alg->cra_aead.ivsize + pdl +
			req->cryptlen + icvsize;

	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;
	if (rc != 0)
		APMSEC_TXLOG("IPE encrypt transport esp failed error %d",
				rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}

static int _xgene_sec_decrypt_esp_transport(struct aead_request *req, int gcm_flag)
{
	struct xgene_sec_session_ctx     *session  = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct crypto_aead 	 *aead = crypto_aead_reqtfm(req);
#ifdef APM_SEC_TXDEBUG
	struct crypto_alg 	 *alg  = req->base.tfm->__crt_alg;
#endif
	struct sec_tkn_input_hdr *in_tkn;
	int		icv_len = crypto_aead_authsize(aead);
	unsigned char 	new_tkn;
	int		rc;
	int		ihl;
	void 		*page;
	int ivsize;
	void *sa_align;

	APMSEC_TXLOG("IPE decrypt esp %s nbytes %d",
			alg->cra_name, req->cryptlen);

	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	/* Extract IHL length */
	page = kmap_atomic(sg_page(&req->src[0]));
	ihl  = ((struct iphdr *) (page + req->src[0].offset))->ihl << 2;
	kunmap_atomic(page);
	sa_align = SA_PTR_ALIGN(session->sa_ib->sa_ptr);
	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("IPE tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_espin_transport(
				(u32 *) in_tkn,
				req->cryptlen,
				xgene_sec_v2hwaddr(sa_align),
				(u32 *) session->sa_ib->sa_ptr,
				session->sa_ib->sa_ptr->crypto_alg,
				ihl, icv_len,
				SA_SEQ_OFFSET(session->sa_ib->sa_ptr),
				session->sa_ib->sa_ptr->cryptomode_feedback,
				gcm_flag);
		BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
		rctx->tkn->sa	        = session->sa_ib;
	} else {
		sec_tkn_update_espin_transport((u32 *) in_tkn,
				req->cryptlen,
				session->sa_ib->sa_ptr->crypto_alg,
				ihl, icv_len,
				session->sa_ib->sa_ptr->cryptomode_feedback,
				gcm_flag);
	}
	rctx->tkn->context     = &req->base;
	ivsize = (session->sa_ib->sa_ptr->crypto_alg == SA_CRYPTO_ALG_DES ||
			session->sa_ib->sa_ptr->crypto_alg ==
			SA_CRYPTO_ALG_3DES
			|| session->sa_ib->sa_ptr->cryptomode_feedback ==
			SA_CRYPTO_MODE_AES_3DES_CTR) ? 8 : 16;
	rctx->tkn->dest_nbytes = req->cryptlen - icv_len - 8 - ivsize;

	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;
	if (rc != 0)
		APMSEC_TXLOG("IPE decrypt failed error %d",
				rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}

static int xgene_sec_encrypt_esp_transport_gcm(struct aead_request *req)
{
	return _xgene_sec_encrypt_esp_transport(req, 1);
}

static int xgene_sec_encrypt_esp_transport(struct aead_request *req)
{
	return _xgene_sec_encrypt_esp_transport(req, 0);
}

static int xgene_sec_decrypt_esp_transport_gcm(struct aead_request *req)
{
	return _xgene_sec_decrypt_esp_transport(req, 1);
}

static int xgene_sec_decrypt_esp_transport(struct aead_request *req)
{
	return _xgene_sec_decrypt_esp_transport(req, 0);
}

/**
 * ESP Transport Encrypt/Decrypt Routine for GCM Algorithm
 *
 */
static int xgene_sec_encrypt_esp_transport_gcm(struct aead_request *req)
{
	struct xgene_sec_session_ctx     *ctx;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead 	 *aead;
	struct crypto_alg 	 *alg;
	struct iphdr	*iph;
	unsigned char 	new_tkn;
	int		rc;
	int		icvsize;
	void 		*page;
	int		pdl;
	unsigned char	nh;
	int		clen;

	ctx  = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);
	aead = crypto_aead_reqtfm(req);
	alg  = req->base.tfm->__crt_alg;

#if defined(APM_SEC_TXDEBUG)
	APMSEC_TXLOG("IPE%d encrypt esp %s nbytes %d",
			alg->cra_name, req->cryptlen);
#endif
	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(ctx, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	icvsize = crypto_aead_authsize(aead);
	in_tkn  = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("IPE%d tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	/* Compute pad len and Next header byte */
	page = kmap_atomic(sg_page(&req->src[0]));
	iph   = ((struct iphdr *) (page + req->src[0].offset));
	nh = iph->protocol;
	clen = req->cryptlen - (iph->ihl << 2);
	kunmap_atomic(page);

	pdl = ALIGN(clen + 2, ALIGN(crypto_aead_blocksize(aead), 4));
	if (ctx->pad_block_size)
		pdl = ALIGN(pdl, ctx->pad_block_size);
	pdl -= clen;
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_espout_transport_gcm(
					(u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr(session->sa->sa_ptr),
					(u32 *) session->sa->sa_ptr,
					pdl,
					icvsize,
					nh,
					SA_SEQ_OFFSET(session->sa->sa_ptr));
		BUG_ON(rctx->tkn->input_tkn_len > ctx->tkn_input_len);
		rctx->tkn->devid 	= ctx->dev->dev_id;
		rctx->tkn->priority 	= xgene_sec_COS_DEF;
		rctx->tkn->sa	        = session->sa;
	} else {
		sec_tkn_update_espout_transport_gcm((u32 *) in_tkn,
					req->cryptlen,
					session->sa_ib->sa_ptr->crypto_alg,
					pdl, nh, icvsize);
	}

	rctx->tkn->context     = &req->base;
	rctx->tkn->src	       = NULL;
	if (xgene_sec_sg_scattered(req->cryptlen, req->dst)) {
		rctx->tkn->result_type     = IPE_TKN_RESULT_AMEM;
		rctx->tkn->amem_result.len = sizeof(struct ip_esp_hdr) +
						alg->cra_aead.ivsize + pdl +
						req->cryptlen + icvsize;
		rctx->tkn->amem_result.offset = 0;
		rctx->tkn->amem_result.buf = kmalloc(rctx->tkn->mem_result.len,
							GFP_KERNEL);
		if (rctx->tkn->amem_result.buf == NULL) {
			rc = -ENOMEM;
			goto out_chk;
		}
		rc = xgene_sec_setup_crypto(&req->base);
	} else {
		void *dpage;
		rctx->tkn->result_type 	  = IPE_TKN_RESULT_MMEM;
		rctx->tkn->mem_result.len = sizeof(struct ip_esp_hdr) +
						alg->cra_aead.ivsize + pdl +
						req->cryptlen + icvsize;
		rctx->tkn->mem_result.offset = 0;
		dpage = kmap_atomic(sg_page(&req->dst[0]));
		rctx->tkn->mem_result.buf = dpage + req->dst[0].offset;
		rc = xgene_sec_setup_crypto(&req->base);
		kunmap_atomic(dpage);
	}
out_chk:
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("IPE%d encrypt gcm esp failed error %d",
				rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(ctx, rctx->tkn);
	return rc;
}

static int xgene_sec_decrypt_esp_transport_gcm(struct aead_request *req)
{
	struct xgene_sec_session_ctx     *ctx  = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct crypto_aead 	 *aead = crypto_aead_reqtfm(req);
	struct crypto_alg 	 *alg  = req->base.tfm->__crt_alg;

	struct sec_tkn_input_hdr *in_tkn;
	int icvsize = crypto_aead_authsize(aead);
	unsigned char 	new_tkn;
	int		rc;
	int		ihl;
	void 		*page;
	int result_len;
	int ivsize;

	APMSEC_TXLOG("IPE%d decrypt esp %s nbytes %d",
			alg->cra_name, req->cryptlen);

	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(ctx, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	/* Extract IHL length */
	page = kmap_atomic(sg_page(&req->src[0]));
	ihl  = ((struct iphdr *) (page + req->src[0].offset))->ihl << 2;
	kunmap_atomic(page);

	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("IPE%d tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_espin_transport_gcm(
					(u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr
					(session->sa_ib->sa_ptr),
					(u32 *) session->sa_ib->sa_ptr,
					session->sa_ib->sa_ptr->crypto_alg,
					ihl,
					icvsize,
					SA_SEQ_OFFSET(session->sa->sa_ptr));
		BUG_ON(rctx->tkn->input_tkn_len > ctx->tkn_input_len);
		rctx->tkn->devid	= ctx->dev->dev_id;
		rctx->tkn->priority 	= xgene_sec_COS_DEF;
		rctx->tkn->sa	        = session->sa_ib;
	} else {
		sec_tkn_update_espin_transport_gcm((u32 *) in_tkn,
					req->cryptlen,
					session->sa_ib->sa_ptr->crypto_alg,
					ihl, icvsize);
	}

	rctx->tkn->context     = &req->base;
	rctx->tkn->src	       = NULL;
	ivsize = alg->cra_aead.ivsize;

	result_len = req->cryptlen - icvsize - 8 - ivsize;

	if (xgene_sec_sg_scattered(result_len, req->dst)) {
		rctx->tkn->result_type     = IPE_TKN_RESULT_AMEM;
		rctx->tkn->amem_result.len = result_len;
		rctx->tkn->amem_result.buf = kmalloc(result_len, GFP_KERNEL);
		if (rctx->tkn->amem_result.buf == NULL) {
			rc = -ENOMEM;
			goto out_chk;
		}
		rctx->tkn->amem_result.offset = 0;
		rc = xgene_sec_setup_crypto(&req->base);
	} else {
		void *dpage;
		rctx->tkn->result_type 	     = IPE_TKN_RESULT_MMEM;
		rctx->tkn->mem_result.len    = result_len;
		rctx->tkn->mem_result.offset = 0;
		dpage = kmap_atomic(sg_page(&req->dst[0]));
		rctx->tkn->mem_result.buf = dpage + req->dst[0].offset;
		rc = xgene_sec_setup_crypto(&req->base);
		kunmap_atomic(dpage);
	}
out_chk:
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("IPE%d decrypt ccm failed error %d",
				rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(ctx, rctx->tkn);
	return rc;
}

/**
 * AH Transport/Tunnel SetKey Routines
 *
 */

static int __xgene_sec_setkey_ah(struct crypto_aead *cipher, const u8 *key,
				  	unsigned int keylen, unsigned char ha,
					unsigned char xcbc_flag)
{
	struct crypto_tfm    *tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *ctx = crypto_tfm_ctx(tfm);
	struct rtattr 	     *rta = (void *) key;
	struct ah_param {
		__be32 spi;
		__be32 seq;
	} *param;
	unsigned int authkeylen;
	unsigned int sa_len;
	unsigned int sa_max_len;
	int rc = -EINVAL;

	if (!RTA_OK(rta, keylen))
		goto badkey;
	if (rta->rta_type != CRYPTO_AUTHENC_KEYA_PARAM)
		goto badkey;
	if (RTA_PAYLOAD(rta) < sizeof(*param))
		goto badkey;

	param     = RTA_DATA(rta);

	key    += RTA_ALIGN(rta->rta_len);
	keylen -= RTA_ALIGN(rta->rta_len);

	authkeylen = keylen;

	if (xcbc_flag) {
		sa_len  = 8 + SA_compute_digest_len((ha),
			SA_DIGEST_TYPE_HASH) + 4 /* SPI */+
			SA_compute_seq_len
				(SA_SEQ_32BITS /* Assuming 32bit seq*/);

		sa_max_len  = 8 + 512/8 + SA_compute_digest_len((ha),
			SA_DIGEST_TYPE_HASH) + 4 /* SPI */+
			SA_compute_seq_len
				(SA_SEQ_32BITS /* Assuming 32bit seq*/);
	}
	else {
		sa_len  = 8 + SA_compute_digest_len((ha),
			SA_DIGEST_TYPE_HMAC) + 4 /* SPI */+
			SA_compute_seq_len
				(SA_SEQ_32BITS /* Assuming 32bit seq*/);

		sa_max_len  = 8 + 512/8 + SA_compute_digest_len((ha),
			SA_DIGEST_TYPE_HMAC) + 4 /* SPI */+
			SA_compute_seq_len
				(SA_SEQ_32BITS /* Assuming 32bit seq*/);
	}
	/* As key length can change, we need to flush the cache sa/token */
	xgene_sec_free_sa_tkn_pool(ctx);
	/* Re-create the SA/token pool */
	rc = xgene_sec_create_sa_tkn_pool(ctx, sa_max_len, sa_len, 1,
			IPE_TKN_AHOUT_TUNNEL_MAXSIZE);
	if (rc != 0)
		goto err_nomem;

	APMSEC_DEBUG("authenc authkeylen %d ha %d sa_len %d icvsize %d",
		authkeylen, ha, session->sa_len, crypto_aead_authsize(cipher));
	APMSEC_DEBUGX_DUMP("authenc auth key: ", key, authkeylen);
	/* Setup SA - NOTE: Control word is in host endian to allow easy
	 * variable reference. Control word in token is in little endian.
	 * Even though we setup both SA (inbound and outbound) only one
	 * can be used.
	 */
	memset(session->sa->sa_ptr, 0, session->sa_len);
	session->sa->sa_ptr->hash_alg   	= ha;
	if (xcbc_flag)
		session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HASH;
	else
		session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HMAC;
	session->sa->sa_ptr->ToP		= SA_TOP_HASH_OUTBOUND;
	session->sa->sa_ptr->spi		= 1;
	session->sa->sa_ptr->seq		= SA_SEQ_32BITS;
	session->sa->sa_ptr->iv03		= 0;	/* Use PRNG */
	session->sa->sa_ptr->context_length = (sa_len >> 2) - 2;

	sec_sa_setbe(SA_SPI(session->sa->sa_ptr), param->spi);
	sec_sa_setbe(SA_SEQ(session->sa->sa_ptr), param->seq);

	/* Pre-compute inner and outer digest using auth key */
	APMSEC_DEBUG("transport sa 0x%p sa_ptr 0x%p sa_ib 0x%p sa_ib_ptr 0x%p",
		session->sa, session->sa->sa_ptr, session->sa_ib, session->sa_ib->sa_ptr);
	if (xcbc_flag)
		rc = xgene_sec_xcbc_digest(key, authkeylen,
			(unsigned char *) SA_DIGEST(session->sa->sa_ptr));
	else
		rc = xgene_sec_compute_ipad_opad(tfm, key, authkeylen,
			(unsigned char *) SA_DIGEST(session->sa->sa_ptr));
	if (rc)
		goto err_nomem;
	APMSEC_DEBUGX_DUMP("sa: ", session->sa->sa_ptr, session->sa_len);
	/* Now copy the same one for inbound */
	memcpy(session->sa_ib->sa_ptr, session->sa->sa_ptr, session->sa_len);
	session->sa_ib->sa_ptr->ToP = SA_TOP_HASH_INBOUND;
	APMSEC_DEBUGX_DUMP("sa_ib: ", session->sa_ib->sa_ptr, session->sa_len);

	APMSEC_DEBUG("alg %s setkey",
			crypto_tfm_alg_name(tfm));
	return rc;

err_nomem:
	xgene_sec_free_sa_tkn_pool(ctx);
	return rc;

badkey:
	crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return rc;
}

static int xgene_sec_setkey_ah_md5(struct crypto_aead *cipher, const u8 *key,
				   unsigned int keylen)
{
	return __xgene_sec_setkey_ah(cipher, key, keylen,
					SA_HASH_ALG_MD5, 0);
}

static int xgene_sec_setkey_ah_sha1(struct crypto_aead *cipher, const u8 *key,
				   unsigned int keylen)
{
	return __xgene_sec_setkey_ah(cipher, key, keylen,
					SA_HASH_ALG_SHA1, 0);
}

static int xgene_sec_setkey_ah_xcbc(struct crypto_aead *cipher, const u8 *key,
				    unsigned int keylen)
{
	return __xgene_sec_setkey_ah(cipher, key, keylen,
					SA_HASH_ALG_XCBC128, 1);
}

static int xgene_sec_setkey_ah_sha2(struct crypto_aead *cipher,
				const unsigned char *key,
				unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(cipher);
	struct crypto_alg *alg  = tfm->__crt_alg;
	struct aead_alg aalg = alg->cra_aead;

	unsigned char ha = 0;

	switch (aalg.maxauthsize) {
	default:
	case 256/8:
		ha = SA_HASH_ALG_SHA256;
		break;
	case 224/8:
		ha = SA_HASH_ALG_SHA224;
		break;
	case 512/8:
		ha = SA_HASH_ALG_SHA512;
		break;
	case 384/8:
		ha = SA_HASH_ALG_SHA384;
		break;
	}
	return __xgene_sec_setkey_ah(cipher, key, keylen, ha, 0);
}

/**
 * AH Transport/Tunnel Encrypt Routines
 *
 */
static int xgene_sec_encrypt_ah_transport(struct aead_request *req)
{
	struct xgene_sec_session_ctx     *ctx;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead 	 *aead;
	struct crypto_alg 	 *alg;
	struct iphdr	*iph;
	unsigned char 	new_tkn;
	int		rc;
	int		icvsize;
	void 		*page;
	unsigned char	nh;
	unsigned short  rid;
	unsigned short iphl;

	ctx  = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);
	aead = crypto_aead_reqtfm(req);
	alg  = req->base.tfm->__crt_alg;

#if defined(APM_SEC_TXDEBUG)
	APMSEC_TXLOG("IPE%d auth ah transport %s nbytes %d",
			alg->cra_name, req->cryptlen);
#endif
	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(ctx, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	icvsize = crypto_aead_authsize(aead);
	in_tkn  = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("IPE%d tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	/* Compute pad len and Next header byte */
	page = kmap_atomic(sg_page(&req->src[0]));
	iph   = ((struct iphdr *) (page + req->src[0].offset));
	iphl = iph->ihl << 2;
	nh = iph->protocol;
	rid = iph->id;
	kunmap_atomic(page);

	if (new_tkn) {
		rctx->tkn->input_tkn_len = ipe_tkn_set_ahout_transport(
					(u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr(session->sa->sa_ptr),
					(u32 *) session->sa->sa_ptr, 0, 0,
					iphl, 0, nh, rid, 0, 0,
					icvsize,
					SA_SEQ_OFFSET(session->sa->sa_ptr));
		BUG_ON(rctx->tkn->input_tkn_len > ctx->tkn_input_len);
		rctx->tkn->devid 	= ctx->dev->dev_id;
		rctx->tkn->priority 	= xgene_sec_COS_DEF;
		rctx->tkn->sa	        = session->sa;
	} else {
		ipe_tkn_update_ahout_transport((u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr
					(session->sa->sa_ptr),
					0, rid, iphl, 0, nh, 0,
					0, icvsize);
	}

	rctx->tkn->context     = &req->base;
	rctx->tkn->src	       = NULL;
	if (xgene_sec_sg_scattered(req->cryptlen, req->dst)) {
		rctx->tkn->result_type     = IPE_TKN_RESULT_AMEM;
		rctx->tkn->amem_result.len = sizeof(struct ip_auth_hdr) +
						req->cryptlen + icvsize;
		rctx->tkn->amem_result.offset = 0;
		rctx->tkn->amem_result.buf = kmalloc(rctx->tkn->mem_result.len,
							GFP_KERNEL);
		if (rctx->tkn->amem_result.buf == NULL) {
			rc = -ENOMEM;
			goto out_chk;
		}
		rc = xgene_sec_setup_crypto(&req->base);
	} else {
		void *dpage;
		rctx->tkn->result_type 	  = IPE_TKN_RESULT_MMEM;
		rctx->tkn->mem_result.len = sizeof(struct ip_auth_hdr) +
						req->cryptlen + icvsize;
		rctx->tkn->mem_result.offset = 0;
		dpage = kmap_atomic(sg_page(&req->dst[0]));
		rctx->tkn->mem_result.buf = dpage + req->dst[0].offset;
		rc = xgene_sec_setup_crypto(&req->base);
		kunmap_atomic(dpage);
	}
out_chk:
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("IPE%d auth ah failed error %d",
				rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(ctx, rctx->tkn);
	return rc;
}

static int xgene_sec_encrypt_ah_tunnel(struct aead_request *req)
{
	struct xgene_sec_session_ctx     *ctx;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead 	 *aead;
	struct crypto_alg 	 *alg;
	struct iphdr	*iph;
	unsigned char 	new_tkn;
	int		rc;
	int		icvsize;
	void 		*page;
	unsigned char	nh;
	unsigned short  rid;
	unsigned short iphl;
	unsigned int saddr;
	unsigned int daddr;
	unsigned char tos;
	unsigned int result_len;
	unsigned short fragoff;
	ctx  = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);
	aead = crypto_aead_reqtfm(req);
	alg  = req->base.tfm->__crt_alg;

	APMSEC_TXLOG("IPE%d auth ah tunnel %s nbytes %d",
			alg->cra_name, req->cryptlen);
	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(ctx, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	icvsize = crypto_aead_authsize(aead);
	in_tkn  = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("IPE%d tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	/* Compute Outer Header values to be sent to Hash Engine for ICV */
	page = kmap_atomic(sg_page(&req->dst[0]));
	iph   = ((struct iphdr *) (page + req->dst[0].offset));
	iphl = iph->ihl << 2;
	nh = iph->protocol;
	rid = iph->id;
	saddr = iph->saddr;
	daddr = iph->daddr;
	tos = iph->tos;
	fragoff = iph->frag_off;
	kunmap_atomic(page);

	if (new_tkn) {
		rctx->tkn->input_tkn_len = ipe_tkn_set_ahout_tunnel(
					(u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr
					(session->sa->sa_ptr),
					(u32 *) session->sa->sa_ptr, 0, 0,
					iphl, nh, tos, saddr, daddr, rid,
					0, fragoff, icvsize,
					SA_SEQ_OFFSET(session->sa->sa_ptr));
		BUG_ON(rctx->tkn->input_tkn_len > ctx->tkn_input_len);
		rctx->tkn->devid 	= ctx->dev->dev_id;
		rctx->tkn->priority 	= xgene_sec_COS_DEF;
		rctx->tkn->sa	        = session->sa;
	} else {
		ipe_tkn_update_ahout_tunnel((u32 *) in_tkn,
					req->cryptlen,
					0, 0, iphl, nh, tos,
					saddr, daddr, rid,
					fragoff, 0);
	}

	rctx->tkn->context     = &req->base;
	rctx->tkn->src	       = NULL;
	result_len = req->cryptlen + iphl + sizeof(struct ip_auth_hdr)
					+ icvsize;
	if (xgene_sec_sg_scattered(req->cryptlen, req->dst)) {
		rctx->tkn->result_type     = IPE_TKN_RESULT_AMEM;
		rctx->tkn->amem_result.len = result_len;
		rctx->tkn->amem_result.offset = 0;
		rctx->tkn->amem_result.buf = kmalloc(rctx->tkn->mem_result.len,
							GFP_KERNEL);
		if (rctx->tkn->amem_result.buf == NULL) {
			rc = -ENOMEM;
			goto out_chk;
		}
		rc = xgene_sec_setup_crypto(&req->base);
	} else {
		void *dpage;
		rctx->tkn->result_type 	  = IPE_TKN_RESULT_MMEM;
		rctx->tkn->mem_result.len = result_len;
		rctx->tkn->mem_result.offset = 0;
		dpage = kmap_atomic(sg_page(&req->dst[0]));
		rctx->tkn->mem_result.buf = dpage + req->dst[0].offset;
		rc = xgene_sec_setup_crypto(&req->base);
		kunmap_atomic(dpage);
	}
out_chk:
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("IPE%d auth ah tunnel failed error %d",
				rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(ctx, rctx->tkn);
	return rc;
}

/**
 * AH Transport/Tunnel Decrypt Routines
 *
 */
static int xgene_sec_decrypt_ah_transport(struct aead_request *req)
{
	struct xgene_sec_session_ctx     *ctx  = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct crypto_aead 	 *aead = crypto_aead_reqtfm(req);
#ifdef APM_SEC_TXDEBUG
	struct crypto_alg 	 *alg  = req->base.tfm->__crt_alg;
#endif
	struct sec_tkn_input_hdr *in_tkn;
	unsigned char 	new_tkn;
	int		rc;
	void 		*page;
	struct iphdr	*iph;
	struct ip_auth_hdr  *ah;
	int 		result_len;
	int 		icvsize;
	unsigned char	nh;
	unsigned short  rid;
	unsigned short  iphl;

	APMSEC_TXLOG("IPE%d decrypt ah transport %s nbytes %d",
			alg->cra_name, req->cryptlen);

	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(ctx, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	/* Extract IHL length */
	icvsize = crypto_aead_authsize(aead);
	page = kmap_atomic(sg_page(&req->src[0]));
	iph  = ((struct iphdr *) (page + req->src[0].offset));
	iphl = iph->ihl << 2;
	rid = iph->id;
	ah	= (struct ip_auth_hdr *) (page + req->src[0].offset + iphl);
	nh = ah->nexthdr;
	kunmap_atomic(page);

	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("IPE%d tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	if (new_tkn) {
		rctx->tkn->input_tkn_len = ipe_tkn_set_ahin_transport(
					(u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr
					(session->sa_ib->sa_ptr),
					(u32 *) session->sa_ib->sa_ptr, 0,
					iphl, 0, nh, rid, 0, 0,
					icvsize,
					SA_SEQ_OFFSET(session->sa_ib->sa_ptr));
		BUG_ON(rctx->tkn->input_tkn_len > ctx->tkn_input_len);
		rctx->tkn->devid	= ctx->dev->dev_id;
		rctx->tkn->priority 	= xgene_sec_COS_DEF;
		rctx->tkn->sa	        = session->sa_ib;
	} else {
		ipe_tkn_update_ahin_transport((u32 *) in_tkn,
					req->cryptlen, 0, rid, iphl, 0, nh, 0,
					0, icvsize);
	}
	rctx->tkn->context     = &req->base;
	rctx->tkn->src	       = NULL;
	result_len = req->cryptlen - sizeof(struct ip_auth_hdr)
					- icvsize;
	if (xgene_sec_sg_scattered(result_len, req->dst)) {
		rctx->tkn->result_type     = IPE_TKN_RESULT_AMEM;
		rctx->tkn->amem_result.len = result_len;
		rctx->tkn->amem_result.buf = kmalloc(result_len, GFP_KERNEL);
		if (rctx->tkn->amem_result.buf == NULL) {
			rc = -ENOMEM;
			goto out_chk;
		}
		rctx->tkn->amem_result.offset = 0;
		rc = xgene_sec_setup_crypto(&req->base);
	} else {
		void *dpage;
		rctx->tkn->result_type 	     = IPE_TKN_RESULT_MMEM;
		rctx->tkn->mem_result.len    = result_len;
		rctx->tkn->mem_result.offset = 0;
		dpage = kmap_atomic(sg_page(&req->dst[0]));
		rctx->tkn->mem_result.buf = dpage + req->dst[0].offset;
		rc = xgene_sec_setup_crypto(&req->base);
		kunmap_atomic(dpage);
	}
out_chk:
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("IPE%d decrypt ah transport failed error %d",
				rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(ctx, rctx->tkn);
	return rc;
}

static int xgene_sec_decrypt_ah_tunnel(struct aead_request *req)
{
	struct xgene_sec_session_ctx     *ctx  = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct crypto_aead 	 *aead = crypto_aead_reqtfm(req);
#ifdef APM_SEC_TXDEBUG
	struct crypto_alg 	 *alg  = req->base.tfm->__crt_alg;
#endif
	struct sec_tkn_input_hdr *in_tkn;
	unsigned char 	new_tkn;
	int		rc;
	void 		*page;
	struct iphdr	*iph;
	int 		result_len;
	int 		icvsize;
	unsigned short  rid;
	unsigned short iphl;

	APMSEC_TXLOG("IPE%d decrypt ah tunnel %s nbytes %d",
			alg->cra_name, req->cryptlen);

	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(ctx, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	/* Extract IHL length */
	icvsize = crypto_aead_authsize(aead);
	page = kmap_atomic(sg_page(&req->src[0]));
	iph  = ((struct iphdr *) (page + req->src[0].offset));
	iphl = iph->ihl << 2;
	rid = iph->id;
	kunmap_atomic(page);

	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("IPE%d tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	if (new_tkn) {
		rctx->tkn->input_tkn_len = ipe_tkn_set_ahin_tunnel(
					(u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr(session->sa_ib->sa_ptr),
					(u32 *) session->sa_ib->sa_ptr, 0,
					0, iphl, rid, icvsize,
					SA_SEQ_OFFSET(session->sa_ib->sa_ptr));
		BUG_ON(rctx->tkn->input_tkn_len > ctx->tkn_input_len);
		rctx->tkn->devid	= ctx->dev->dev_id;
		rctx->tkn->priority 	= xgene_sec_COS_DEF;
		rctx->tkn->sa	        = session->sa_ib;
	} else {
		ipe_tkn_update_ahin_tunnel((u32 *) in_tkn,
					req->cryptlen, 0, 0, iphl, rid);
	}
	rctx->tkn->context     = &req->base;
	rctx->tkn->src	       = NULL;
	result_len = req->cryptlen - sizeof(struct ip_auth_hdr)
			 		- icvsize - iphl;

	if (xgene_sec_sg_scattered(result_len, req->dst)) {
		rctx->tkn->result_type     = IPE_TKN_RESULT_AMEM;
		rctx->tkn->amem_result.len = result_len;
		rctx->tkn->amem_result.buf = kmalloc(result_len, GFP_KERNEL);
		if (rctx->tkn->amem_result.buf == NULL) {
			rc = -ENOMEM;
			goto out_chk;
		}
		rctx->tkn->amem_result.offset = 0;
		rc = xgene_sec_setup_crypto(&req->base);
	} else {
		void *dpage;
		rctx->tkn->result_type 	     = IPE_TKN_RESULT_MMEM;
		rctx->tkn->mem_result.len    = result_len;
		rctx->tkn->mem_result.offset = 0;
		dpage = kmap_atomic(sg_page(&req->dst[0]));
		rctx->tkn->mem_result.buf = dpage + req->dst[0].offset;
		rc = xgene_sec_setup_crypto(&req->base);
		kunmap_atomic(dpage);
	}
out_chk:
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("IPE%d decrypt ah tunnel failed error %d",
				rc);

	if (rctx->tkn)
		xgene_sec_tkn_free(ctx, rctx->tkn);
	return rc;
}

/*
 * AES-GCM (RFC4106) IPSec Authenc Routines
 *
 */
static int xgene_sec_setkey_aes_gcm_rfc4106(struct crypto_aead *cipher,
					const unsigned char *key,
					unsigned int keylen)
{

	struct crypto_tfm    *tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *ctx = crypto_tfm_ctx(tfm);
	int 	rc;

	if (keylen != 128/8 + 4 && keylen != 192/8 + 4 &&
	    keylen != 256/8 + 4) {
		crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	keylen -= 4;	/* salt (nonce) is at last 4 bytes */

	rc = xgene_sec_create_sa_tkn_pool(ctx,
			SA_AES_GCM_MAX_LEN, SA_AES_GCM_LEN(keylen),
			0, TKN_BASIC_MAX_LEN);
	if (rc != 0)
		goto err_nomem;

	/* Setup SA - Fill SA with proper value. For AES-GCM, we will
	   have a SA for each request */
	memset(session->sa, 0, session->sa_len);
	session->sa->sa_ptr->hash_alg   	= SA_HASH_ALG_GHASH;
	session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HASH;
	session->sa->sa_ptr->key 		= 1;
	switch(keylen) {
	case 128/8:
		session->sa->sa_ptr->crypto_alg 	= SA_CRYPTO_ALG_AES128;
		break;
	case 192/8:
		session->sa->sa_ptr->crypto_alg 	= SA_CRYPTO_ALG_AES192;
		break;
	case 256/8:
		session->sa->sa_ptr->crypto_alg 	= SA_CRYPTO_ALG_AES256;
		break;
	}
	session->sa->sa_ptr->cryptomode_feedback    =
			SA_CRYPTO_MODE_AES_3DES_CTR;

	session->sa->sa_ptr->spi		= 1;
	session->sa->sa_ptr->seq		= SA_SEQ_32BITS;
					/* assume 32-bits seq */
	session->sa->sa_ptr->iv_format	= SA_IV_FORMAT_COUNTER_MODE;
	session->sa->sa_ptr->iv03		= 0x7;
	session->sa->sa_ptr->mask		= SA_MASK_NONE;
	session->sa->sa_ptr->context_length = (session->sa_len >> 2) - 2;

	sec_sa_setle(SA_KEY(session->sa), key, keylen);
	/* Save salt into IV0; IV1 & IV2 will be filled later from request */
	sec_sa_setle(SA_IV(session->sa), key + keylen, 4);

	memcpy(session->sa_ib, session->sa, session->sa_len);
	session->sa_ib->sa_ptr->mask = SA_MASK_64;    /* assum 64-bit mask */

	APMSEC_DEBUG("alg %s setkey",
			crypto_tfm_alg_name(tfm));
	return 0;

err_nomem:
	xgene_sec_free_sa_tkn_pool(ctx);

	return rc;
}

/**
 * AES-CCM (RFC4309) IPSec Functions
 *
 */
static int xgene_sec_setkey_esp_ccm(struct crypto_aead *cipher,
					const unsigned char *key,
					unsigned int keylen)
{
	struct crypto_tfm    *tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *session = crypto_tfm_ctx(tfm);
	struct rtattr 	     *rta = (void *) key;
	struct esp_authenc_param {
		__be32 spi;
		__be32 seq;
		__be16 pad_block_size;
		__be16 encap_uhl;
		struct crypto_authenc_key_param authenc_param;
	} *param;
	int 	rc = -EINVAL;

	if (!RTA_OK(rta, keylen))
		goto badkey;

	param     = RTA_DATA(rta);

	key    += RTA_ALIGN(rta->rta_len);
	keylen -= RTA_ALIGN(rta->rta_len);

	if (keylen != 128/8 + 3 && keylen != 192/8 + 3 &&
			keylen != 256/8 + 3)
		goto badkey;

	keylen -= 3;	/* salt value is 3 bytes */

	xgene_sec_free_sa_tkn_pool(session);
	rc = xgene_sec_create_sa_tkn_pool(session, SA_AES_ESP_CCM_MAX_LEN,
					  SA_AES_ESP_CCM_LEN(keylen),
					  1, TKN_AES_CCM_MAX);
	if (rc != 0)
		goto err_nomem;

	memset(session->sa->sa_ptr, 0, session->sa_len);
	switch(keylen) {
	case 128/8:
		session->sa->sa_ptr->crypto_alg 	= SA_CRYPTO_ALG_AES128;
		session->sa->sa_ptr->hash_alg   	= SA_HASH_ALG_XCBC128;
		break;
	case 192/8:
		session->sa->sa_ptr->crypto_alg 	= SA_CRYPTO_ALG_AES192;
		session->sa->sa_ptr->hash_alg   	= SA_HASH_ALG_XCBC192;
		break;
	case 256/8:
		session->sa->sa_ptr->crypto_alg 	= SA_CRYPTO_ALG_AES256;
		session->sa->sa_ptr->hash_alg   	= SA_HASH_ALG_XCBC256;
		break;
	}
	session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HASH;
	session->sa->sa_ptr->key 		= 1;
	session->sa->sa_ptr->cryptomode_feedback =
				SA_CRYPTO_MODE_AES_3DES_CTR_LOAD_REUSE;
	session->sa->sa_ptr->iv03		= 0xF;

	session->sa->sa_ptr->ToP		= SA_TOP_HASH_ENCRYPT_OUTBOUND;
	session->sa->sa_ptr->spi		= 1;
	session->sa->sa_ptr->seq		= SA_SEQ_32BITS;
	session->sa->sa_ptr->enc_hash_result= 1;

	session->sa->sa_ptr->pad_type	= SA_PAD_TYPE_IPSEC;
	session->sa->sa_ptr->context_length = (session->sa_len >> 2) - 2;

	sec_sa_set32le(SA_SPI(session->sa->sa_ptr), param->spi);
	sec_sa_set32le(SA_SEQ(session->sa->sa_ptr), param->seq);

	session->pad_block_size = be16_to_cpu(param->pad_block_size);
	session->encap_uhl      = be16_to_cpu(param->encap_uhl);

	sec_sa_setbe(SA_KEY(session->sa->sa_ptr), key, keylen);
	/* Load K2, K3, and K1 digest */
	rc = xgene_sec_xcbc_digest(key, keylen,
				(unsigned char *)
				SA_DIGEST(session->sa->sa_ptr));
	if (rc)
		goto err_nomem;
	/* Load salt (nonce) into IV0 */
	memcpy(SA_IV(session->sa->sa_ptr), key + keylen, 3);
	*((unsigned char *) SA_IV(session->sa->sa_ptr) + 3) = 0;
#if 0
	memcpy(SA_IV(session->sa->sa_ptr) + 1, 0xA1A2A3A4, 4);
	memcpy(SA_IV(session->sa->sa_ptr) + 2, 0xA5A6A7A8, 4);
#endif
	/* Now copy the same one for inbound */
	memcpy(session->sa_ib->sa_ptr, session->sa->sa_ptr, session->sa_len);
	session->sa_ib->sa_ptr->iv03	= 0x7;
	session->sa_ib->sa_ptr->ToP     = SA_TOP_DECRYPT_HASH_INBOUND;
	APMSEC_DEBUG("authenc sa 0x%p sa_ptr 0x%p sa_ib 0x%p sa_ib_ptr 0x%p",
		session->sa, session->sa->sa_ptr, session->sa_ib, session->sa_ib->sa_ptr);
	APMSEC_DEBUGX_DUMP("sa: ", session->sa->sa_ptr, session->sa_len);

	session->sa_ib->sa_ptr->ToP = SA_TOP_DECRYPT_HASH_INBOUND;
	APMSEC_DEBUGX_DUMP("sa_ib: ", session->sa_ib->sa_ptr, session->sa_len);

	APMSEC_DEBUG("alg %s setkey",
			crypto_tfm_alg_name(tfm));
	return rc;

err_nomem:
	xgene_sec_free_sa_tkn_pool(session);
	return rc;
badkey:
	crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return rc;
}

static int xgene_sec_decrypt_esp_transport_ccm(struct aead_request *req)
{

	struct xgene_sec_session_ctx     *session = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead 	 *aead = crypto_aead_reqtfm(req);
	struct crypto_alg 	 *alg  = req->base.tfm->__crt_alg;
	struct iphdr	*iph;
	unsigned char iv_B0[16];
	unsigned char adata_length[6];
	unsigned char 	new_tkn;
	int		rc;
	int		icvsize;
	int 		ivsize;
	void 		*page;
	int		pdl;
	unsigned char	nh;
	int		clen;
	void *sa_align;

	APMSEC_TXLOG("IPE decrypt esp %s nbytes %d",
			alg->cra_name, req->cryptlen);
	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	icvsize = crypto_aead_authsize(aead);
	in_tkn  = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("IPE tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	sa_align = SA_PTR_ALIGN(session->sa_ib->sa_ptr);
	/* Compute pad len and Next header byte */
	page = kmap_atomic(sg_page(&req->src[0]));
	iph   = ((struct iphdr *) (page + req->src[0].offset));
	nh = iph->protocol;
	clen = req->cryptlen - (iph->ihl << 2);
	kunmap_atomic(page);

	pdl = ALIGN(clen + 2, ALIGN(crypto_aead_blocksize(aead), 4));
	if (session->pad_block_size)
		pdl = ALIGN(pdl, session->pad_block_size);
	pdl -= clen;

	/* Form IV B0 salt and flags for hashing */
	//sec_tkn_setle(iv_B0, SA_IV(session->sa_ib->sa_ptr), 4);
	sec_tkn_setle((unsigned int *)iv_B0, (unsigned char *)
			SA_IV(session->sa_ib->sa_ptr), 4);
	memset(iv_B0 + 12, 0, 4);
	memset(adata_length, 0, 6);
	/* format control info per RFC 3610 and NIST Special Publication
	800-38C */
	iv_B0[0] |= 3;
	iv_B0[0] |= (8 * ((crypto_aead_authsize(aead) - 2) / 2));
	iv_B0[0] |= 64;
	/* Taking L length as 2 bytes*/
	ivsize = alg->cra_aead.ivsize;
	rctx->tkn->dest_nbytes = req->cryptlen - icvsize - 8 - ivsize;
	rc = sec_sa_set_aes_ccm_msg_len(iv_B0 + (16 - 4),
					84, 4);
	if (rc != 0) {
		rc = -EOVERFLOW;
		goto err_dec_aes_ccm;
	}
	sec_sa_set_aes_ccm_adata(adata_length, 8 /* SPI + Seq*/);
	rctx->tkn->input_tkn_len = sec_tkn_set_espin_transport_ccm(
					(u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr(sa_align),
					(u32 *) session->sa_ib->sa_ptr,
					pdl,
					icvsize,
					nh, iv_B0, adata_length,
					SA_SEQ_OFFSET(session->sa_ib->sa_ptr));
	BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
	rctx->tkn->sa	        = session->sa_ib;
	rctx->tkn->context     = &req->base;
	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;
err_dec_aes_ccm:
	if (rc != 0)
		APMSEC_TXLOG("IPE ccm decrypt esp failed error %d",
			     rc);
	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}

static int xgene_sec_encrypt_esp_transport_ccm(struct aead_request *req)
{
	struct xgene_sec_session_ctx     *session;
	struct xgene_sec_req_ctx *rctx;
	struct sec_tkn_input_hdr *in_tkn;
	struct crypto_aead 	 *aead;
	struct crypto_alg 	 *alg;
	struct iphdr	*iph;
	unsigned char iv_B0[16];
	unsigned char adata_length[6];
	unsigned char 	new_tkn;
	int		rc;
	int		icvsize;
	void 		*page;
	int		pdl;
	unsigned char	nh;
	unsigned char	iphl;
	int		clen;
	void *sa_align;

	session  = crypto_tfm_ctx(req->base.tfm);
	rctx = aead_request_ctx(req);
	aead = crypto_aead_reqtfm(req);
	alg  = req->base.tfm->__crt_alg;

	APMSEC_TXLOG("IPE encrypt esp %s nbytes %d",
			alg->cra_name, req->cryptlen);
	rctx->sa  = NULL;
	rctx->tkn = xgene_sec_tkn_get(session, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;
	icvsize = crypto_aead_authsize(aead);
	in_tkn  = TKN_CTX_INPUT_TKN(rctx->tkn);
	APMSEC_SATKNLOG("IPE tkn 0x%p outtkn 0x%p intkn 0x%p",
			rctx->tkn,
			TKN_CTX_RESULT_TKN(rctx->tkn), in_tkn);
	/* Compute pad len and Next header byte */
	page = kmap_atomic(sg_page(&req->src[0]));
	iph   = ((struct iphdr *) (page + req->src[0].offset));
	nh = iph->protocol;
	iphl = (iph->ihl << 2);
	clen = req->cryptlen - iphl;
	kunmap_atomic(page);

	pdl = ALIGN(clen + 2, ALIGN(crypto_aead_blocksize(aead), 4));
	if (session->pad_block_size)
		pdl = ALIGN(pdl, session->pad_block_size);
	pdl -= clen;
	sa_align = SA_PTR_ALIGN(session->sa->sa_ptr);
	/* Form IV B0 salt and flags for hashing */
	sec_tkn_setle((unsigned int *)iv_B0, (unsigned char *)
			SA_IV(session->sa->sa_ptr), 4);
	memset(iv_B0 + 12, 0, 4);
	memset(adata_length, 0, 6);
	/* format control info per RFC 3610 and NIST Special Publication
	800-38C */
	iv_B0[0] |= 3;
	iv_B0[0] |= (8 * ((crypto_aead_authsize(aead) - 2) / 2));
	iv_B0[0] |= 64;
	/* Taking L length as 2 bytes*/
	rc = sec_sa_set_aes_ccm_msg_len(iv_B0 + (16 - 4), req->cryptlen, 4);
	if (rc != 0) {
		rc = -EOVERFLOW;
		goto err_enc_aes_ccm;
	}
	sec_sa_set_aes_ccm_adata(adata_length, 8 /* SPI + Seq*/);


	rctx->tkn->input_tkn_len = sec_tkn_set_espout_transport_ccm(
				(u32 *) in_tkn,
				req->cryptlen,
				xgene_sec_v2hwaddr(sa_align),
				(u32 *) session->sa->sa_ptr,
				pdl,
				icvsize,
				nh, iphl, iv_B0, adata_length,
				SA_SEQ_OFFSET(session->sa->sa_ptr));
	BUG_ON(rctx->tkn->input_tkn_len > session->tkn_input_len);
	rctx->tkn->sa	        = session->sa;

	rctx->tkn->context     = &req->base;
	rctx->tkn->dest_nbytes = sizeof(struct ip_esp_hdr) +
			alg->cra_aead.ivsize + pdl +
			req->cryptlen + icvsize;

	rc = xgene_sec_setup_crypto(&req->base);
	if (rc == -EINPROGRESS || rc == -EAGAIN)
		return rc;

	if (rc != 0)
		APMSEC_TXLOG("IPE  encrypt esp failed error %d",
				rc);
err_enc_aes_ccm:
	if (rctx->tkn)
		xgene_sec_tkn_free(session, rctx->tkn);
	return rc;
}

static int xgene_sec_setkey_aes_xcbc_ipsec(struct crypto_aead *cipher,
				 	   const unsigned char 	*key,
				 	   unsigned int	keylen)
{
	struct crypto_tfm    *tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *ctx = crypto_tfm_ctx(tfm);
	struct rtattr 	     *rta = (void *) key;
	struct crypto_authenc_key_param *param;
	struct crypto_blkcipher *blk_tfm = NULL;
	struct blkcipher_desc   blk_desc;
	struct scatterlist 	sg[1];
	unsigned int enckeylen;
	unsigned int authkeylen;
	unsigned int j;
	unsigned int *digest;
	char 	block128[16];
	int 	rc = -EINVAL;

	if (!RTA_OK(rta, keylen))
		goto badkey;
	if (rta->rta_type != CRYPTO_AUTHENC_KEYA_PARAM)
		goto badkey;
	if (RTA_PAYLOAD(rta) < sizeof(*param))
		goto badkey;

	param     = RTA_DATA(rta);
	enckeylen = be32_to_cpu(param->enckeylen);

	key    += RTA_ALIGN(rta->rta_len);
	keylen -= RTA_ALIGN(rta->rta_len);

	authkeylen = keylen - enckeylen;

	if (keylen < enckeylen)
		goto badkey;
	if (keylen != 128/8 && keylen != 192/8 && keylen != 256/8)
		goto badkey;

	rc = xgene_sec_create_sa_tkn_pool(ctx,
			SA_AES_MAX_LEN(SA_CRYPTO_MODE_AES_3DES_CBC),
			SA_AES_LEN(keylen, SA_CRYPTO_MODE_AES_3DES_CBC),
			1, sec_tkn_IPSEC_MAX_SIZE);
	if (rc != 0)
		goto err_nomem;

	/* Setup SA */
	memset(session->sa->sa_ptr, 0, session->sa_len);
	session->sa->sa_ptr->crypto_alg 	= SA_keylen2aes_mode(keylen);
	session->sa->sa_ptr->hash_alg   	= SA_HASH_ALG_XCBC128;
	session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HASH;
	session->sa->sa_ptr->key 		= 1;
	session->sa->sa_ptr->cryptomode_feedback = SA_CRYPTO_MODE_AES_3DES_CBC;
	session->sa->sa_ptr->iv03		= 0xF;
	session->sa->sa_ptr->context_length = (session->sa_len >> 2) - 2;
	sec_sa_setbe2le(SA_KEY(session->sa->sa_ptr), key, keylen);

	/* Load K2, K3, and K1 digest */
	blk_tfm = crypto_alloc_blkcipher("ecb(aes)", 0, 0);
	if (IS_ERR(blk_tfm)) {
		APMSEC_ERR("failed to load helper transform for ecb(aes) "
				"error %ld", PTR_ERR(blk_tfm));
		goto err_nomem;
	}
	blk_desc.tfm   = blk_tfm;
	blk_desc.flags = 0;
	rc = crypto_blkcipher_setkey(blk_tfm, key, keylen);
	if (rc != 0) {
		APMSEC_ERR("failed to set helper transform key for ecb(aes) "
				"error %d", rc);
		goto err_nomem;
	}
	digest = SA_DIGEST(session->sa->sa_ptr);
	for (j = 2; j < 5; j++) {
		int 	i;
		for (i = 0; i < 128/8; i++)
			block128[i] = j == 4 ? 0x01 : j;
		sg_init_one(sg, block128, 128/8);
		rc = crypto_blkcipher_encrypt(&blk_desc, sg, sg, 128/8);
		if (rc != 0) {
			APMSEC_ERR("failed to encrypt hash key error %d",
					rc);
			goto err_nomem;
		}
		sec_sa_setbe2le(digest, block128, 128/8);
		digest += 4;
	}

	/* Load salt (nonce) into IV0 */
	memcpy(SA_IV(session->sa->sa_ptr), key + keylen, 3);
	*((unsigned char *) SA_IV(session->sa->sa_ptr) + 3) = 0;

	memcpy(session->sa_ib->sa_ptr, session->sa->sa_ptr, session->sa_len);

	APMSEC_DEBUG("alg %s setkey", alg->cra_name);
	return rc;

err_nomem:
	xgene_sec_free_sa_tkn_pool(ctx);
	return rc;

badkey:
	crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return rc;
}

/*
 * MACSec Algorithms
 *
 *
 */
static inline int xgene_sec_setkey_macsec(struct crypto_aead *cipher,
				const unsigned char *key,
				unsigned int keylen)
{
	struct crypto_tfm    *tfm = crypto_aead_tfm(cipher);
	struct xgene_sec_session_ctx *ctx = crypto_tfm_ctx(tfm);
	int 	rc = -EINVAL;

	/* Key length musts be 16-byte crypto and 16 bytes hash */
	if (keylen != 16 + 16) {
		crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	rc = xgene_sec_create_sa_tkn_pool(ctx, SA_MACSEC_LEN,
			SA_MACSEC_LEN, 1, sec_tkn_MACSEC_MAX_SIZE);
	if (rc != 0)
		goto err_nomem;

	/* Setup SA - Fill SA with proper values */
	session->sa->sa_ptr->hash_alg   	= SA_HASH_ALG_GHASH;
	session->sa->sa_ptr->digest_type 	= SA_DIGEST_TYPE_HASH;
	session->sa->sa_ptr->key 		= 1;
	session->sa->sa_ptr->crypto_alg 	= SA_CRYPTO_ALG_AES128;
	session->sa->sa_ptr->cryptomode_feedback = SA_CRYPTO_MODE_AES_3DES_CTR;
	session->sa->sa_ptr->seq		= SA_SEQ_32BITS;
	session->sa->sa_ptr->mask		= SA_MASK_NONE;
	session->sa->sa_ptr->iv_format	= SA_IV_FORMAT_COUNTER_MODE;
	session->sa->sa_ptr->iv03		= 0x7;
	session->sa->sa_ptr->context_length = (session->sa_len >> 2) - 2;
	sec_sa_setbe2le(SA_KEY(session->sa->sa_ptr), key, 16);
	sec_sa_setle(SA_DIGEST(session->sa->sa_ptr), key + 16, 16);
	SA_set32(SA_SEQ(session->sa->sa_ptr), 1);

	memcpy(session->sa_ib->sa_ptr, session->sa->sa_ptr, session->sa_len);
	session->sa_ib->sa_ptr->mask = SA_MASK_64;    /* assume 64-bit mask */

	APMSEC_DEBUG("alg %s setkey", alg->cra_name);
	return 0;

err_nomem:
	xgene_sec_free_sa_tkn_pool(ctx);

	return rc;
}

static int xgene_sec_setauthsize_macsec(struct crypto_aead *ciper,
				        unsigned int authsize)
{
	struct aead_tfm *tfm = crypto_aead_crt(ciper);

	if (authsize != 16)
		return -EINVAL;

	tfm->authsize = authsize;
	return 0;
}

static int xgene_sec_encrypt_macsec(struct aead_request *req,
					unsigned int aes_mode)
{
	struct xgene_sec_session_ctx     *ctx  = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct sec_tkn_input_hdr	*in_tkn;
	unsigned char 	*adata = NULL;
	unsigned char	new_tkn;

	/* MACSec STI (4) and SCI (8) is stored in associate data field */
	if (req->assoclen != 12) {
		APMSEC_ERR("algorithm MACSec unsupported AAD length %d",
				req->assoclen);
		return -EINVAL;
	}

	/* Allocate token for this request */
	rctx->tkn = xgene_sec_tkn_get(ctx, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;

	if (req->assoclen)
		adata = kmap_atomic(sg_page(req->assoc));
	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_out_macsec(
					(u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr(rctx->sa),
					aes_mode,
					*(u32 *) adata,
					adata + 4);
		rctx->tkn->devid 	= ctx->dev->dev_id;
		rctx->tkn->priority 	= xgene_sec_COS_DEF;
		rctx->tkn->sa_len_dw 	= session->sa_len >> 2;
	} else {
		rctx->tkn->input_tkn_len = sec_tkn_update_out_macsec(
					(u32 *) in_tkn,
					req->cryptlen,
					*(u32 *) adata,
					adata + 4);
	}
	if (adata)
		kunmap_atomic(adata);

	return xgene_sec_setup_crypto(&req->base);
}

static int xgene_sec_decrypt_macsec(struct aead_request *req,
					unsigned int aes_mode)
{
	struct xgene_sec_session_ctx     *ctx  = crypto_tfm_ctx(req->base.tfm);
	struct xgene_sec_req_ctx *rctx = aead_request_ctx(req);
	struct sec_tkn_input_hdr	*in_tkn;
	unsigned char	new_tkn;

	/* Allocate token for this request */
	rctx->tkn = xgene_sec_tkn_get(ctx, &new_tkn);
	if (rctx->tkn == NULL)
		return -ENOMEM;

	in_tkn = TKN_CTX_INPUT_TKN(rctx->tkn);
	if (new_tkn) {
		rctx->tkn->input_tkn_len = sec_tkn_set_in_macsec(
					(u32 *) in_tkn,
					req->cryptlen,
					xgene_sec_v2hwaddr(rctx->sa->sa_ptr),
					rctx->sa->sa_ptr->mask,
					aes_mode);
		rctx->tkn->devid 	= ctx->dev->dev_id;
		rctx->tkn->priority 	= xgene_sec_COS_DEF;
		rctx->tkn->sa_len_dw 	= session->sa_len >> 2;
	} else {
		rctx->tkn->input_tkn_len = sec_tkn_update_in_macsec(
						(u32 *) in_tkn,
						req->cryptlen);
	}

	return xgene_sec_setup_crypto(&req->base);
}

static int xgene_sec_encrypt_macsec_gcm(struct aead_request *req)
{
	return xgene_sec_encrypt_macsec(req, sec_tkn_AES_GCM);
}
static int xgene_sec_decrypt_macsec_gcm(struct aead_request *req)
{
	return xgene_sec_decrypt_macsec(req, sec_tkn_AES_GCM);
}

static int xgene_sec_encrypt_macsec_gmac(struct aead_request *req)
{
	return xgene_sec_encrypt_macsec(req, sec_tkn_AES_GMAC);
}
static int xgene_sec_decrypt_macsec_gmac(struct aead_request *req)
{
	return xgene_sec_decrypt_macsec(req, sec_tkn_AES_GMAC);
}

/*
 * For xcbc(aes), no support for init, update, and final hash functions
 *
 */
static int xgene_sec_hash_noinit(struct ahash_request *req)
{       /* TBD */
        return -ENOSYS;
}

static int xgene_sec_hash_noupdate(struct ahash_request *req)
{       /* TBD */
        return -ENOSYS;
}

static int xgene_sec_hash_nofinal(struct ahash_request *req)
{       /* TBD */
        return -ENOSYS;
}

struct xgene_sec_alg xgene_sec_alg_tlb[] = {
	/* Basic Hash Alg */

	{.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
         .halg.base      	= {
	 .cra_name 		= "hmac(sha384)",
	 .cra_driver_name 	= "xgene-hmac-sha384",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 128,	/* SHA384-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ahash_type },
	 .halg.digestsize 	= 48,	/* Disgest is 384-bits */
	 .init   		= xgene_sec_hash_init,
	 .update 		= xgene_sec_hash_update,
	 .final  		= xgene_sec_hash_final,
	 .digest 		= xgene_sec_hash_digest,
	 .setkey 		= xgene_sec_sha2_hmac_setkey,
	 }},

	{.type = CRYPTO_ALG_TYPE_AHASH, .u.hash = {
         .halg.base      	= {
	 .cra_name 		= "hmac(sha512)",
	 .cra_driver_name 	= "xgene-hmac-sha512",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 128,	/* SHA512-HMAC block size is 1024-bits*/
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_ahash_type },
	 .halg.digestsize 	= 64,	/* Disgest is 512-bits */
	 .init   		= xgene_sec_hash_init,
	 .update 		= xgene_sec_hash_update,
	 .final  		= xgene_sec_hash_final,
	 .digest 		= xgene_sec_hash_digest,
	 .setkey 		= xgene_sec_sha2_hmac_setkey,
	 }},

	/* Crypto AES ECB, CBC, CTR, GCM, CCM, and GMAC modes */
	{.type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
         .cra_name 		= "ccm(aes)",
	 .cra_driver_name 	= "xgene-ccm-aes",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_aes_ccm,
	 .setauthsize		= xgene_sec_setauthsize_aes_ccm,
	 .encrypt 		= xgene_sec_encrypt_aes_ccm,
	 .decrypt 		= xgene_sec_decrypt_aes_ccm,
	 .givencrypt		= xgene_sec_givencrypt_aes_ccm,
	 .givdecrypt		= xgene_sec_givdecrypt_aes_ccm,
	 }}}},

	 /* IPSec ESP Transport Mode Algorithms */
	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(md5),cbc(aes))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-md5-cbc-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size for crypto */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(sha1),cbc(aes))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-sha1-cbc-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size for crypto */
	 .maxauthsize		= 20,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(sha256),cbc(aes))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-sha256-cbc-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size for crypto */
	 .maxauthsize		= 32,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(sha224),cbc(aes))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-sha224-cbc-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size for crypto */
	 .maxauthsize		= 28,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(sha512),cbc(aes))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-sha512-cbc-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size for crypto */
	 .maxauthsize		= 32,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(md5),cbc(des))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-md5-cbc-des",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size for crypto */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(sha1),cbc(des))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-sha1-cbc-des",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size for crypto */
	 .maxauthsize		= 20,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(sha224),cbc(des))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-sha224-cbc-des",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 64-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size for crypto */
	 .maxauthsize		= 28,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(sha256),cbc(des))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-sha256-cbc-des",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size for crypto */
	 .maxauthsize		= 32,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(md5),cbc(des3_ede))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-md5-cbc-des3",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size for crypto */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des3,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(sha1),cbc(des3_ede))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-sha1-cbc-des3",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size for crypto */
	 .maxauthsize		= 20,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des3,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(sha256),cbc(des3_ede))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-sha256-cbc-des3",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size for crypto */
	 .maxauthsize		= 32,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des3,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(md5),rfc3686(ctr(aes)))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-md5-ctr-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_aes_ctr,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(authenc(hmac(sha1),rfc3686(ctr(aes)))))",
	 .cra_driver_name 	= "xgene-transport-esp-hmac-sha1-ctr-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 20,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_aes_ctr,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(rfc4106(gcm(aes))))",
	 .cra_driver_name 	= "xgene-transport-esp-rfc4106-gcm-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size for crypto */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_gcm,
	 .setauthsize		= xgene_sec_setauthsize_aes_gcm_rfc4106,
	 .encrypt 		= xgene_sec_encrypt_esp_transport_gcm,
	 .decrypt 		= xgene_sec_decrypt_esp_transport_gcm,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "transport(esp(rfc4309(ccm(aes))))",
	 .cra_driver_name 	= "xgene-transport-esp-rfc4309-ccm-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_module 		= THIS_MODULE,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size for crypto */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_ccm,
	 .setauthsize		= xgene_sec_setauthsize_aes_ccm,
	 .encrypt 		= xgene_sec_encrypt_esp_transport_ccm,
	 .decrypt 		= xgene_sec_decrypt_esp_transport_ccm,
	 }}}},
	/* IPsec Tunnel Mode ESP Algorithms*/
	{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(md5),cbc(aes))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-md5-cbc-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha1),cbc(aes))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha1-cbc-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .maxauthsize		= 20,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha224),cbc(aes))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha224-cbc-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .maxauthsize		= 28,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha256),cbc(aes))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha256-cbc-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .maxauthsize		= 32,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha384),cbc(aes))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha384-cbc-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .maxauthsize		= 48,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha512),cbc(aes))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha512-cbc-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .maxauthsize		= 64,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(md5),cbc(des3_ede))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-md5-cbc-des3",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des3,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha1),cbc(des3_ede))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha1-cbc-des3",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 20,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des3,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha224),cbc(des3_ede))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha224-cbc-des3",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 28,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des3,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha256),cbc(des3_ede))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha256-cbc-des3",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 32,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des3,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha384),cbc(des3_ede))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha384-cbc-des3",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 48,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des3,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha512),cbc(des3_ede))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha512-cbc-des3",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 64,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des3,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(md5),cbc(des))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-md5-cbc-des",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha1),cbc(des))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha1-cbc-des",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 20,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha224),cbc(des))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha224-cbc-des",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 28,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha256),cbc(des))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha256-cbc-des",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 32,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha384),cbc(des))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha384-cbc-des",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 48,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha512),cbc(des))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha512-cbc-des",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 8,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 32,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_des,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 /* IPSec Algorithms */
	 {.type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "rfc4106(gcm(aes))",
	 .cra_driver_name 	= "xgene-aes-ipsec",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,		/* IV size is 16 bytes */
	 .setkey 		= xgene_sec_setkey_esp_gcm,
	 .setauthsize		= xgene_sec_setauthsize_aes_gcm_rfc4106,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(rfc4106(gcm(aes))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-rfc4106-gcm-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_gcm,
	 .setauthsize		= xgene_sec_setauthsize_aes_gcm_rfc4106,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel_gcm,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel_gcm,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(md5),rfc3686(ctr(aes)))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-md5-ctr-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_aes_ctr,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha1),rfc3686(ctr(aes)))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha1-ctr-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 20,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_aes_ctr,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha224),rfc3686(ctr(aes)))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha224-ctr-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 28,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_aes_ctr,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha256),rfc3686(ctr(aes)))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha256-ctr-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 32,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_aes_ctr,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha384),rfc3686(ctr(aes)))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha384-ctr-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 48,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_aes_ctr,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	 { .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
	 .cra_name 		= "tunnel(esp(authenc(hmac(sha512),rfc3686(ctr(aes)))))",
	 .cra_driver_name 	= "xgene-tunnel-esp-hmac-sha512-ctr-aes",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 8,	/* IV size is 16 bytes */
	 .maxauthsize		= 64,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_aes_ctr,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_tunnel,
	 .decrypt 		= xgene_sec_decrypt_esp_tunnel,
	 }}}},

	{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
		.cra_name 		= "tls1_1(aes-sha1)",
		.cra_driver_name 	= "xgene-ssl-aes-sha1",
		.cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
		.cra_blocksize 		= 16,	/* 64 bits  ... 128-bits block */
		.cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
		.cra_alignmask 		= 0xF,
		.cra_type 		= &crypto_aead_type,
		.cra_module 		= THIS_MODULE,
		.cra_u 		= {.aead = {
		.ivsize 		= 16,	/* IV size is 16 bytes */
		.maxauthsize		= 53,	/* Max auth data size in bytes */
		.setkey 		= xgene_sec_setkey_tls_aes_sha1,
		.setauthsize		= NULL,
		.encrypt 		= xgene_sec_encrypt_tls1_1,
		.decrypt 		= xgene_sec_decrypt_tls1_1,
		.givencrypt		= NULL,
		.givdecrypt		= NULL,
	}}}},

	{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
		.cra_name 		= "ssl(aes-sha1)",
		.cra_driver_name 	= "xgene-ssl-aes-sha1",
		.cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
		.cra_blocksize 		= 16,	/* 64 bits  ... 128-bits block */
		.cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
		.cra_alignmask 		= 0xF,
		.cra_type 		= &crypto_aead_type,
		.cra_module 		= THIS_MODULE,
		.cra_u 		= {.aead = {
		.ivsize 		= 16,	/* IV size is 16 bytes */
		.maxauthsize		= 35,	/* Max auth data size in bytes */
		.setkey 		= xgene_sec_setkey_ssl_aes_sha1,
		.setauthsize		= NULL,
		.encrypt 		= xgene_sec_encrypt_ssl,
		.decrypt 		= xgene_sec_decrypt_ssl,
		.givencrypt		= NULL,
		.givdecrypt		= NULL,
	}}}},

	{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
		.cra_name 		= "ssl(aes-md5)",
		.cra_driver_name 	= "xgene-ssl-aes-md5",
		.cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
		.cra_blocksize 		= 16,	/* 64 bits  ... 128-bits block */
		.cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
		.cra_alignmask 		= 0xF,
		.cra_type 		= &crypto_aead_type,
		.cra_module 		= THIS_MODULE,
		.cra_u 		= {.aead = {
		.ivsize 		= 16,	/* IV size is 16 bytes */
		.maxauthsize		= 35,	/* Max auth data size in bytes */
		.setkey 		= xgene_sec_setkey_ssl_aes_md5,
		.setauthsize		= NULL,
		.encrypt 		= xgene_sec_encrypt_ssl,
		.decrypt 		= xgene_sec_decrypt_ssl,
		.givencrypt		= NULL,
		.givdecrypt		= NULL,
	}}}},
	{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
		.cra_name 		= "ssl(arc4-sha1)",
		.cra_driver_name 	= "xgene-ssl-arc4-sha1",
		.cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
		.cra_blocksize 		= 16,	/* 64 bits  ... 128-bits block */
		.cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
		.cra_alignmask 		= 0xF,
		.cra_type 		= &crypto_aead_type,
		.cra_module 		= THIS_MODULE,
		.cra_u 		= {.aead = {
		.ivsize 		= 16,	/* IV size is 16 bytes */
		.maxauthsize		= 35,	/* Max auth data size in bytes */
		.setkey 		= xgene_sec_setkey_ssl_arc4,
		.setauthsize		= NULL,
		.encrypt 		= xgene_sec_encrypt_ssl_tls_arc4,
		.decrypt 		= xgene_sec_decrypt_ssl,
		.givencrypt		= NULL,
		.givdecrypt		= NULL,
	}}}},
	{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
		.cra_name 		= "dtls(aes-sha1)",
		.cra_driver_name 	= "xgene-dtls-aes-sha1",
		.cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
		.cra_blocksize 		= 16,	/* 64 bits  ... 128-bits block */
		.cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
		.cra_alignmask 		= 0xF,
		.cra_type 		= &crypto_aead_type,
		.cra_module 		= THIS_MODULE,
		.cra_u 		= {.aead = {
		.ivsize 		= 16,	/* IV size is 16 bytes */
		.maxauthsize		= 59,	/* Max auth data size in bytes */
		.setkey 		= xgene_sec_setkey_dtls_aes_sha1,
		.setauthsize		= NULL,
		.encrypt 		= xgene_sec_encrypt_dtls,
		.decrypt 		= xgene_sec_decrypt_dtls,
		.givencrypt		= NULL,
		.givdecrypt		= NULL,
	}}}},
		{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
		.cra_name 		= "dtls(3des-sha1)",
		.cra_driver_name 	= "xgene-dtls-3des-sha1",
		.cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
		.cra_blocksize 		= 8,	/* 64 bits  ... 128-bits block */
		.cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
		.cra_alignmask 		= 0xF,
		.cra_type 		= &crypto_aead_type,
		.cra_module 		= THIS_MODULE,
		.cra_u 		= {.aead = {
		.ivsize 		= 8,	/* IV size is 16 bytes */
		.maxauthsize		= 59,	/* Max auth data size in bytes */
		.setkey 		= xgene_sec_setkey_dtls_3des_sha1,
		.setauthsize		= NULL,
		.encrypt 		= xgene_sec_encrypt_dtls,
		.decrypt 		= xgene_sec_decrypt_dtls,
		.givencrypt		= NULL,
		.givdecrypt		= NULL,
	}}}},
		{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
		.cra_name 		= "dtls(aes-md5)",
		.cra_driver_name 	= "xgene-dtls-aes-md5",
		.cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
		.cra_blocksize 		= 16,	/* 64 bits  ... 128-bits block */
		.cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
		.cra_alignmask 		= 0xF,
		.cra_type 		= &crypto_aead_type,
		.cra_module 		= THIS_MODULE,
		.cra_u 		= {.aead = {
		.ivsize 		= 16,	/* IV size is 16 bytes */
		.maxauthsize		= 59,	/* Max auth data size in bytes */
		.setkey 		= xgene_sec_setkey_dtls_aes_md5,
		.setauthsize		= NULL,
		.encrypt 		= xgene_sec_encrypt_dtls,
		.decrypt 		= xgene_sec_decrypt_dtls,
		.givencrypt		= NULL,
		.givdecrypt		= NULL,
	}}}},
	{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
		.cra_name 		= "dtls(3des-md5)",
		.cra_driver_name 	= "xgene-dtls-3des-md5",
		.cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
		.cra_blocksize 		= 8,	/* 64 bits  ... 128-bits block */
		.cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
		.cra_alignmask 		= 0xF,
		.cra_type 		= &crypto_aead_type,
		.cra_module 		= THIS_MODULE,
		.cra_u 		= {.aead = {
		.ivsize 		= 8,	/* IV size is 16 bytes */
		.maxauthsize		= 59,	/* Max auth data size in bytes */
		.setkey 		= xgene_sec_setkey_dtls_3des_md5,
		.setauthsize		= NULL,
		.encrypt 		= xgene_sec_encrypt_dtls,
		.decrypt 		= xgene_sec_decrypt_dtls,
		.givencrypt		= NULL,
		.givdecrypt		= NULL,
	}}}},

	 /* Linux provides init alg testing for all the algorithms above */

	{ .type = CRYPTO_ALG_TYPE_AEAD, .u.cipher = {
		.cra_name 		= "rfc4309(ccm(aes))",
		.cra_driver_name 	= "xgene-aes-ipsec",
		.cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
		.cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
		.cra_blocksize 	= 16,	/* 128-bits block */
		.cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
		.cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
		.cra_type 		= &crypto_aead_type,
		.cra_u 		= {.aead = {
		.ivsize 		= 16,	/* IV size is 16 bytes */
		.maxauthsize		= 16,	/* Max auth data size in bytes */
		.setkey 		= xgene_sec_setkey_esp_ccm,
		.setauthsize		= xgene_sec_setauthsize_aes_ccm,
		.encrypt 		= xgene_sec_encrypt_esp_transport_ccm,
		.decrypt 		= xgene_sec_encrypt_esp_transport_ccm,
	}}}},

	 /* IPsec Transport Mode AH Algorithms*/
	 {.cra_name 		= "transport(ah(hmac(md5)))",
	 .cra_driver_name 	= "xgene-transport-ah-hmac-md5",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* MD5-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 0,	/* IV size for crypto */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ah_md5,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ah_transport,
	 .decrypt 		= xgene_sec_decrypt_ah_transport,
	 }}},
	 {.cra_name 		= "transport(ah(hmac(sha1)))",
	 .cra_driver_name 	= "xgene-transport-ah-hmac-sha1",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* MD5-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 0,	/* IV size for crypto */
	 .maxauthsize		= 20,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ah_sha1,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ah_transport,
	 .decrypt 		= xgene_sec_decrypt_ah_transport,
	 }}},
	 {.cra_name 		= "transport(ah(hmac(sha224)))",
	 .cra_driver_name 	= "xgene-transport-ah-hmac-sha224",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* MD5-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 0,	/* IV size for crypto */
	 .maxauthsize		= 28,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ah_sha2,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ah_transport,
	 .decrypt 		= xgene_sec_decrypt_ah_transport,
	 }}},
	 {.cra_name 		= "transport(ah(hmac(sha256)))",
	 .cra_driver_name 	= "xgene-transport-ah-hmac-sha224",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* MD5-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 0,	/* IV size for crypto */
	 .maxauthsize		= 32,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ah_sha2,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ah_transport,
	 .decrypt 		= xgene_sec_decrypt_ah_transport,
	 }}},
	 /* IPsec Tunnel Mode AH Algorithms*/
	 {.cra_name 		= "tunnel(ah(hmac(md5)))",
	 .cra_driver_name 	= "xgene-tunnel-ah-hmac-md5",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* MD5-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	.ivsize 		= 0,	/* IV size for crypto */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ah_md5,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ah_tunnel,
	 .decrypt 		= xgene_sec_decrypt_ah_tunnel,
	 }}},
	 {.cra_name 		= "tunnel(ah(hmac(sha224)))",
	 .cra_driver_name 	= "xgene-tunnel-ah-hmac-sha224",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* MD5-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 0,	/* IV size for crypto */
	 .maxauthsize		= 28,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ah_sha2,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ah_tunnel,
	 .decrypt 		= xgene_sec_decrypt_ah_tunnel,
	 }}},
	 {.cra_name 		= "tunnel(ah(hmac(sha256)))",
	 .cra_driver_name 	= "xgene-tunnel-ah-hmac-sha256",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* MD5-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 0,	/* IV size for crypto */
	 .maxauthsize		= 32,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ah_sha2,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ah_tunnel,
	 .decrypt 		= xgene_sec_decrypt_ah_tunnel,
	 }}},
	 {.cra_name 		= "tunnel(ah(hmac(sha384)))",
	 .cra_driver_name 	= "xgene-tunnel-ah-hmac-sha384",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* MD5-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 0,	/* IV size for crypto */
	 .maxauthsize		= 48,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ah_sha2,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ah_tunnel,
	 .decrypt 		= xgene_sec_decrypt_ah_tunnel,
	 }}},
	 {.cra_name 		= "tunnel(ah(hmac(sha512)))",
	 .cra_driver_name 	= "xgene-tunnel-ah-hmac-sha512",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* SHA512-HMAC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 0,	/* IV size for crypto */
	 .maxauthsize		= 64,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ah_sha2,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ah_tunnel,
	 .decrypt 		= xgene_sec_decrypt_ah_tunnel,
	 }}},
	 {.cra_name 		= "tunnel(ah(xcbc(aes)))",
	 .cra_driver_name 	= "xgene-tunnel-ah-xcbc",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 64,	/* XCBC block size is 512-bits */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 0,	/* IV size for crypto */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ah_xcbc,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ah_tunnel,
	 .decrypt 		= xgene_sec_decrypt_ah_tunnel,
	 }}},
	{.cra_name 		= "authenc(rfc3686(ctr(aes)),hmac(md5))",
	 .cra_driver_name 	= "xgene-aes-md5-ipsec",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_esp_aes_ctr,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_esp_transport,
	 .decrypt 		= xgene_sec_decrypt_esp_transport,
	 }}},


	{.cra_name 		= "authenc(rfc3686(ctr(aes)),hmac(sha1))",
	 .cra_driver_name 	= "xgene-aes-md5-ipsec",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ctr_aes_sha1_ipsec,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ctr_ipsec,
	 .decrypt 		= xgene_sec_decrypt_ctr_ipsec,
	 .givencrypt		= xgene_sec_givencrypt_ctr_ipsec,
	 }}},

	{.cra_name 		= "authenc(rfc3686(ctr(aes)),hmac(sha256))",
	 .cra_driver_name 	= "xgene-aes-md5-ipsec",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ctr_aes_sha256_ipsec,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ctr_ipsec,
	 .decrypt 		= xgene_sec_decrypt_ctr_ipsec,
	 .givencrypt		= xgene_sec_givencrypt_ctr_ipsec,
	 }}},

	{.cra_name 		= "authenc(rfc3686(ctr(aes)),xcbc(aes))",
	 .cra_driver_name 	= "xgene-aes-md5-ipsec",
	 .cra_priority 		= APM_SEC_CRYPTO_PRIORITY_IPSEC,
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,	/* IV size is 16 bytes */
	 .maxauthsize		= 16,	/* Max auth data size in bytes */
	 .setkey 		= xgene_sec_setkey_ctr_aes_xcbc_ipsec,
	 .setauthsize		= xgene_sec_setauthsize_aes,
	 .encrypt 		= xgene_sec_encrypt_ctr_ipsec,
	 .decrypt 		= xgene_sec_decrypt_ctr_ipsec,
	 .givencrypt		= xgene_sec_givencrypt_ctr_ipsec,
	 }}},

	{.cra_name 		= "macsec(gcm(aes))",
	 .cra_driver_name 	= "xgene-macsec",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,		/* IV size is 16 bytes */
	 .setkey 		= xgene_sec_setkey_macsec,
	 .setauthsize		= xgene_sec_setauthsize_macsec,
	 .encrypt 		= xgene_sec_encrypt_macsec_gcm,
	 .decrypt 		= xgene_sec_decrypt_macsec_gcm,
	 }}},
	{.cra_name 		= "macsec(gmac(aes))",
	 .cra_driver_name 	= "xgene-macsec",
	 .cra_flags 		= CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	 .cra_blocksize 	= 16,	/* 128-bits block */
	 .cra_ctxsize 		= sizeof(struct xgene_sec_session_ctx),
	 .cra_alignmask 	= 0xF,	/* Hardware requires 16 bytes aligned */
	 .cra_type 		= &crypto_aead_type,
	 .cra_u 		= {.aead = {
	 .ivsize 		= 16,		/* IV size is 16 bytes */
	 .setkey 		= xgene_sec_setkey_macsec,
	 .setauthsize		= xgene_sec_setauthsize_macsec,
	 .encrypt 		= xgene_sec_encrypt_macsec_gmac,
	 .decrypt 		= xgene_sec_decrypt_macsec_gmac,
	 }}},

	/* Terminator */
	{.type = 0 }
};
#endif
