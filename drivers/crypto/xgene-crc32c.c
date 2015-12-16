/*
 * APM X-Gene SoC PktDma crc32c HW acceleration
 *
 * Copyright (c) 2014, Applied Micro Circuits Corporation
 * Author: Rameshwar Prasad Sahu <rsahu@apm.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <crypto/internal/hash.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <misc/xgene/xgene-pktdma.h>

#define CHKSUM_DIGEST_SIZE      	4
#define CHKSUM_BLOCK_SIZE       	1

struct xgene_crc_reqctx {
	u32 crc_seed;
};

struct xgene_crc_ctx {
	u32 key;
};

void xgene_crc32c_cb(void *ctx, u32 result)
{
	struct ahash_request * req = ctx;

	*(__le32 *)req->result = cpu_to_le32(result);
	if (req->base.complete)
		req->base.complete(&req->base, 0);
}

int xgene_crc32c_handle_req(struct ahash_request *req)
{
	struct xgene_crc_reqctx *rctx = ahash_request_ctx(req);
	
	return xgene_flyby_op(xgene_crc32c_cb, req, req->src,
				req->nbytes, rctx->crc_seed, FBY_ISCSI_CRC32C);
}

static int xgene_crc32c_init(struct ahash_request *req)
{
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
	struct xgene_crc_ctx *ctx = crypto_ahash_ctx(tfm);

	*(__le32 *) req->result = cpu_to_le32(ctx->key);

	return 0;
}

static int xgene_crc32c_update(struct ahash_request *req)
{
	struct xgene_crc_reqctx *rctx = ahash_request_ctx(req);

	rctx->crc_seed = le32_to_cpu(*(__le32 *) req->result);

	return xgene_crc32c_handle_req(req);
}

static int xgene_crc32c_final(struct ahash_request *req)
{
	return 0;
}

static int xgene_crc32c_finup(struct ahash_request *req)
{
	struct xgene_crc_reqctx *rctx = ahash_request_ctx(req);

	rctx->crc_seed = le32_to_cpu(*(__le32 *) req->result);

	return xgene_crc32c_handle_req(req);
}

static int xgene_crc32c_digest(struct ahash_request *req)
{
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
	struct xgene_crc_ctx *ctx = crypto_ahash_ctx(tfm);
	struct xgene_crc_reqctx *rctx = ahash_request_ctx(req);

	rctx->crc_seed = ctx->key;

	return xgene_crc32c_handle_req(req);
}

static int xgene_crc32c_setkey(struct crypto_ahash *tfm, const u8 *key,
			       unsigned int keylen)
{
	struct xgene_crc_ctx *ctx = crypto_ahash_ctx(tfm);

	if (keylen != CHKSUM_DIGEST_SIZE) {
		crypto_ahash_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	ctx->key = le32_to_cpu(*(__le32 *) key);

	return 0;
}

static int xgene_crc32c_cra_init(struct crypto_tfm *tfm)
{
	struct xgene_crc_ctx *ctx = crypto_tfm_ctx(tfm);
	
	crypto_ahash_set_reqsize(__crypto_ahash_cast(tfm),
				 sizeof(struct xgene_crc_reqctx));
	ctx->key = ~0;

	return xgene_flyby_alloc_resources();
}

static void xgene_crc32c_cra_exit(struct crypto_tfm *tfm)
{
	xgene_flyby_free_resources();
}

static struct ahash_alg xgene_crc32c_alg = {
	.init		= xgene_crc32c_init,
	.update		= xgene_crc32c_update,
	.final		= xgene_crc32c_final,
	.finup		= xgene_crc32c_finup,
	.digest		= xgene_crc32c_digest,
	.setkey		= xgene_crc32c_setkey,
	.halg.digestsize = CHKSUM_DIGEST_SIZE,
	.halg.base	= {
		.cra_name		= "xgene(crc32c)",
		.cra_driver_name	= "xgene-crc32c",
		.cra_priority		= 100,
		.cra_flags		= CRYPTO_ALG_TYPE_AHASH |
					  CRYPTO_ALG_ASYNC,
		.cra_blocksize		= CHKSUM_BLOCK_SIZE,
		.cra_ctxsize		= sizeof(struct xgene_crc_ctx),
		.cra_alignmask		= 0,
		.cra_module		= THIS_MODULE,
		.cra_init		= xgene_crc32c_cra_init,
		.cra_exit		= xgene_crc32c_cra_exit,
	},
};

static int __init xgene_crc32c_mod_init(void)
{
	return crypto_register_ahash(&xgene_crc32c_alg);
}
late_initcall(xgene_crc32c_mod_init);

static void __exit xgene_crc32c_mod_exit(void)
{
	crypto_unregister_ahash(&xgene_crc32c_alg);
}
module_exit(xgene_crc32c_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rameshwar Prasad Sahu <rsahu@apm.com>");
MODULE_DESCRIPTION("APM X-Gene SoC CRC32c hw accelerator");
