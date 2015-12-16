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
 * This file defines the secure token messages format. All token are in
 * little endian format.
 *
 */
#ifndef __XGENE_SEC_TKN_H__
#define __XGENE_SEC_TKN_H__

struct sec_tkn_result_hdr {
#ifdef CONFIG_CPU_BIG_ENDIAN
	u32 EXX:15;		/* E0 - E14 */
	u32 pkt_len:17;

	u32 L:1;
	u32 N:1;
	u32 C:1;
	u32 B:1;
	u32 hash_bytes:6;
	u32 H:1;
	u32 rsvd1:7;
	u32 num_dell_buf:9;
	u32 E15:1;
	u32 bypass_data:4;
#else
	u32 pkt_len:17;
	u32 EXX:15;		/* E0 - E14 */

	u32 bypass_data:4;
	u32 E15:1;
	u32 num_dell_buf:9;
	u32 rsvd1:7;
	u32 H:1;
	u32 hash_bytes:6;
	u32 B:1;
	u32 C:1;
	u32 N:1;
	u32 L:1;
#endif
	u32 out_pkt_ptr;	/* Copied from input token */
#ifdef CONFIG_CPU_BIG_ENDIAN
	u16 rsvd2;
	u8 pad_length;
	u8 next_hdr_field;
#else
	u8 next_hdr_field;
	u8 pad_length;
	u16 rsvd2;
#endif
	u32 bypass_tkn_data[4];	/* Optional bypass data 0 - 4 */
} __attribute__ ((packed));

#define TKN_ALIGN16(x)		(((unsigned long) (x) + 15) & ~0xF)
#define TKN_RESULT_HDR_MAX_LEN	TKN_ALIGN16(sizeof(struct sec_tkn_result_hdr))
#define TKN_RESULT_HDR_LEN	(sizeof(struct sec_tkn_result_hdr) - 4 * 4)

#define TKN_RESULT_E7		0x0001
#define TKN_RESULT_E8		0x0002
#define TKN_RESULT_E9		0x0004
#define TKN_RESULT_E10		0x0008
#define TKN_RESULT_E11		0x0010
#define TKN_RESULT_E12		0x0020
#define TKN_RESULT_E13		0x0040
#define TKN_RESULT_E14		0x0080
#define TKN_RESULT_E1		0x0100
#define TKN_RESULT_E2		0x0200
#define TKN_RESULT_E3		0x0400
#define TKN_RESULT_E4		0x0800
#define TKN_RESULT_E5		0x1000
#define TKN_RESULT_E6		0x2000

/*
 * Variouse Index into token , use in token update
 */
#define sec_tkn_sa_ptr_idx	3	/* Tkn Index for SA pointer */
#define sec_tkn_ctrl0_idx	4	/* Tkn Index for SA Control WORD0 */
#define sec_tkn_ctrl1_idx	5	/* Tkn Index for SA Control WORD1 */
#define sec_tkn_1st_instr_idx	6	/* Tkn Index for 1st instr with SA ctrl WORD0/1 */

struct sec_tkn_input_hdr {
	/*
	 * All field is in little endian. We don't actual use this structure.
	 * It is more for reference purpose.
	 */
	u32 pkt_len:17;
	u32 rsvd1:1;
	u32 CT:2;		/* Context Type - Set TKN_CT_XXX */
	u32 RC:2;		/* Re-use context - See TKN_RC_XXX */
	u32 ToO:3;		/* Type of output - See TKN_TOO_XXX */
	u32 C:1;		/* Context Control words present if set to 1 */
	u32 IV:3;		/* IV usage and selection */
	u32 U:1;		/* Checksum present if set to 1 */
	u32 rsvd2:2;

	u32 tkn_length:14;
	u32 rsvd3:1;
	u32 num_ctx_updates:4;
	u32 rsvd4:13;

	u32 ctx_ptr_msb:6;
	u32 rsvd5:26;

	u32 ctx_ptr_lsb:32;
} __attribute__ ((packed));

#define _TKN_INPUT_HDR_LEN      (sizeof(struct sec_tkn_input_hdpr))

#define TKN_FLAG_CONTINOUS	0x00000001	/* Multiple op single request ctx */
#define TKN_FLAG_PKG_FIRST	0x00000002	/* First package of request */
#define TKN_FLAG_PKG_MIDDLE	0x00000004	/* Middle package of request */
#define TKN_FLAG_PKG_FINAL	0x00000008	/* Final package of request */

/*
 * Security Token Context
 *
 * Due the the natural of security token variable length fields, it is better
 * to operate on an array of bytes. Input token musts be 16-bytes align. Output
 * token musts be 16 bytes align.
 *
 * Format of Input and Output Token:
 * ------------------
 * - Software core  - software core variable - 16 bytes address aligned
 * - variables      -
 * ------------------
 * - Output Token   - see sec_tkn_result_hdr - 16 bytes address aligned
 * ------------------
 * - Input Token    - see sec_tkn_input_hdr - 16 bytes address aligned
 * -                - This is variable length. The begin of input token
 * -                - is a configurable value of X DWORD. At this moment, it
 * -                - is 32 bytes.
 * ------------------
 */
struct sec_tkn_ctx {
	struct list_head next;	/* Next link for list chain purpose */
	u32 flags;		/* See TKN_FLAG_XXXX */
	u64 tkn_paddr;		/* PHY address of this structure */
	void *context;		/* A pointer context */
	u8 sa_len_dw;		/* # of SA DWORD */
	u16 input_tkn_len;	/* Input token length in bytes */
	void *src_addr_link;	/* Extended QM source link list */
	u64 src_addr_link_paddr;
	void *dst_addr_link;	/* Extended QM destination link list */
	u64 dst_addr_link_paddr;
	int dest_nbytes;	/* FIXME - Do we need this? */
#if 0
	/* FIXME */
	void *dest_mem;
	unsigned char esp_src_len;	/* calculated len to do the flsuh the esp pkt(for aes) */
#endif
	/* For AEAD request */
	struct scatterlist *src_sg;
	u64 src_sg_paddr;	/* PHY address */
	int src_sg_nents;	/* Num of entries */
	int src_sg_nbytes;	/* Num of bytes */

	struct sec_sa_item *sa;	/* SA pointer */
	u64 result_tkn_hwptr;	/* PHY address of result_tkn_ptr */
	void *result_tkn_ptr;	/* Point to output tkn (16-bytes aligned) */

	/* See note above
	 * struct       sec_tkn_result_hdr result_tkn;
	 * struct       sec_tkn_input_hdr  in_tkn;
	 */
} __attribute__ ((aligned(16)));

#define TKN_CTX_RESULT_TKN_COMPUTE(t)	\
		((void *) TKN_ALIGN16((void *) (t) + \
		sizeof(struct sec_tkn_ctx)))
#define TKN_CTX_SIZE(in_tkn_size)   	\
		(TKN_ALIGN16(sizeof(struct sec_tkn_ctx)) + \
		 TKN_RESULT_HDR_MAX_LEN + TKN_ALIGN16(in_tkn_size))
#define TKN_CTX_RESULT_TKN(t)           \
		((struct sec_tkn_result_hdr *) (t)->result_tkn_ptr)
#define TKN_CTX_INPUT_TKN(t)    	\
		((struct sec_tkn_input_hdr *) \
		 ((char *) (t)->result_tkn_ptr + TKN_RESULT_HDR_MAX_LEN))
#define TKN_CTX_RESULT_TKN_OFFSET(t)    \
		((unsigned long) (t)->result_tkn_ptr - (unsigned long) (t))
#define TKN_CTX_INPUT_TKN_OFFSET(t)	\
		(TKN_CTX_TKN_RESULT_TKN_OFFSET(t) + TKN_RESULT_HDR_MAX_LEN)
#define TKN_CTX_HWADDR2TKN(x)		(x - sizeof(struct sec_tkn_ctx))

#ifdef APM_SEC_SATKNDEBUG
void sec_tkn_dump(struct sec_tkn_ctx *tkn);
#endif

/*
 * Create token header (first 4 DWORD's)
 * @param   t       A pointer to input token
 * @param   tl      Variable to store token length counter
 * @param   u       U field of token header. If set, checksum is supplied
 *			from token
 * @param   iv      IV field of token header. See SafeNet User Manual
 *			7.2.1.6 IV:Usage and Selection
 * @param   c       C field of token header. Context control WORD0,1 from
 *			token
 * @param   too     ToO field of token header. See IPE_TKN_TOO_XXX or
 *			SafeNet User Manual
 * @param   rc      RC field of token header. See IPE_TKN_RC_XXX or SafeNet
 *			User Manual
 * @param   ct      CT field of token header. 0 for single IPE. 1 for shared
 *			context.
 * @param   pl      Input packet length in bytes
 * @param   sa      SA address in 32-bit physical address format. See
 * 			function ipe_sa_va2indirectmap.
 * @note  Token header consists of:
 *          Token Control DWORD             - 1st DWORD
 *          Token input packet address      - 2nd DWORD which is always 0
 *          Token output packet address     - 3rd DWORD which is always 0
 *          Token context pointer address   - 4th DWORD which is physical
 *						address
 */
#define sec_tkn_set_hdr(t, tl, u, iv, c, too, rc, ct, pl, cu, tknlen, sa) \
{ \
	sec_tkn_set32le(&t[0], \
		(pl) | (ct) << 18 | (rc) << 20 | (too) << 22 | \
		(c) << 25 | (iv) << 26 | (u) << 29); \
	sec_tkn_set32le(&t[1], (tknlen) | (cu) << 15); \
	/* SA (security association address */ \
	sec_tkn_set32le(&t[2], (u32) ((sa) >> 32)); \
	sec_tkn_set32le(&t[3], (u32) (sa)); \
	tl = 4; \
}

#define sec_tkn_update_hdr(t, u, iv, c, too, rc, ct, pl) 	\
	sec_tkn_set32le((t), \
		(pl) | (ct) << 18 | (rc) << 20 | (too) << 22 | \
		(c) << 25 | (iv) << 26 | (u) << 29)

#define sec_tkn_update_hdr_len(t, u, iv, c, too, rc, ct, pl, cu, tknlen) 	\
{\
	sec_tkn_set32le(&t[0], \
		(pl) | (ct) << 18 | (rc) << 20 | (too) << 22 | \
		(c) << 25 | (iv) << 26 | (u) << 29); \
	sec_tkn_set32le(&t[1], (tknlen) | (cu) << 15); \
}
/*
 * Max # of byte in SA for basic encrypt/decrypt and hashing algorithms
 *
 * Crypto: Tkn Header, Input pointer, Output pointer, Context pointer,
 *         control W0, control W1, IV03 (CBC and CTR only),
 *	   two token instructions
 * Hash:   Tkn Header, Input pointer, Output pointer, Context pointer,
 *         control W0, control W1, 3 token instructionss
 * AES GCM: Tkn Header, Context pointer, control W0, control W1, 
 * 	   6 token instructions, 2KByte max AAD data
 */
#define TKN_BASIC_MAX_LEN		(6*4+6*4)
#define TKN_AES_GCM_MAX			(12*4+512*4)
#define TKN_AES_CCM_MAX			(4*25)

#define IPE_INTKN_AUTHENC_MAX_SIZE	((4+2+6)<<2)
#define IPE_TKN_ESP_TRANSPORT_MAXSIZE	((4+2+8) << 2)
#define IPE_TKN_ESP_TRANSPORT_GCM_MAXSIZE	((4+2+11) << 2)

/*
 * MACSec Token Templates
 */
#define IPE_INTKN_MACSEC_MAX_SIZE	((4+2+12)<<2)
#define IPE_INTKN_AES_CCM		1
#define IPE_INTKN_AES_GMAC		2
#define IPE_INTKN_AES_GCM		3

static inline void sec_tkn_set32le(unsigned int *tkn, unsigned int v)
{
	*tkn = cpu_to_le32(v);
}

static inline void sec_tkn_set32be(unsigned int *tkn, unsigned int v)
{
	*tkn = cpu_to_be32(v);
}

static inline void sec_tkn_setle(unsigned int *tkn, const unsigned char *buf,
				 int len)
{
	for (; len > 0; buf += 4, len -= 4)
		*tkn++ = cpu_to_le32(*(unsigned int *)buf);
}

static inline void sec_tkn_setbe2le(unsigned int *tkn,
				    const unsigned char *buf, int len)
{
	/*
	 * Set network byte order (big endian) to little endian
	 * We must swap regardless of system
	 */
	for (; len > 0; buf += 4, len -= 4)
		*tkn++ = cpu_to_be32(*(unsigned int *)buf);
}

static inline void sec_kn_seth2h(unsigned int *tkn,
				 const unsigned char *buf, int len)
{
	memcpy(tkn, buf, len);
}

static inline unsigned int sec_tkn_getle32h(unsigned int *tkn)
{
	return le32_to_cpu(*tkn);
}

static inline void sec_tkn_setbe(unsigned int *tkn,
				 const unsigned char *buf, int len)
{
	for (; len > 0; buf += 4, len -= 4)
		*tkn++ = cpu_to_be32(*(unsigned int *)buf);
}

static inline void sec_tkn_setle32h(unsigned int *in, unsigned int len,
				    unsigned int *out)
{
	for (; len > 0; len -= 4)
		*out++ = le32_to_cpu(*(unsigned int *)in++);
}

/*
 * Create input token for basic hashing operation
 * @param t	A pointer to input token
 * @param pl	Payload byte length to operation
 * @param ctrl  Two control control WORD
 * @param sa	38 bits physical address format. See function
 *			apm_sa_va2hw.
 * @param ds	Digest size in bytes
 * @return Return total number token byte required
 */
u16 sec_tkn_set_hash(u32 * t, u32 pl, u32 * ctrl, u64 sa, u16 ds, u32 out);

/*
 * Update input token for basic hashing operation
 * @param t	A pointer to input token
 * @param pl	Payload byte length to encrypt
 *
 */
void sec_tkn_update_hash(u32 * t, u32 pl);

/*
 * Create input token for Kasumi F9 operation
 * @param t	A pointer to input token
 * @param pl	Payload byte length to operation
 * @param ctrl  Two control control WORD
 * @param sa	38 bits physical address format. See function
 *			apm_sa_va2hw.
 * @param ds	Digest size in bytes
 * @return Return total number token byte required
 */
u16 sec_tkn_set_kasumi_f9(u32 * t, u32 pl, u32 * ctrl, u64 sa, u16 ds, u32 out);

/*
 * Update input token for Kasumi F9 operation
 * @param t	A pointer to input token
 * @param pl	Payload byte length to encrypt
 *
 */
void sec_tkn_update_kasumi_f9(u32 * t, u32 pl);

/*
 * Create input token for basic encrypt/decrypt operation
 * @param t	A pointer to input token
 * @param pl	Payload byte length to encrypt
 * @param sa	MSB 32-bit of a 36-bits physical address format. See function
 *			apm_sa_va2hw.
 * @param ca	Crypto algorithm to use. See SA_CRYPTO_ALG_XXX
 * @param cm	Crypto mode to use. See SA_CRYPTO_MODE_XXX
 * @param nonce Nonce value in SA format for CTR mode
 * @param iv	Pointer to IV byte stream (16 bytes) for CBC and CTR mode
 * @param ctrl  Two control control WORD
 * @return Return total number token byte required
 * @note For ECB, OFB, and CFB, there is no nonce or iv.
 *       For CBC, IV0 and IV1 is loaded from SA and override by token IV0
 *	 and IV1.
 *       For CTR, IV0 - IV3 is loaded from SA and override by token IV0 -
 *	 IV3.
 */
u16 sec_tkn_set_crypt(u32 * t, u32 pl, u64 sa, u32 * ctrl, u8 ca, int enc);

/*
 * Update input token for basic encrypt/decrypt operation
 * @param t	A pointer to input token
 * @param pl	Payload byte length to encrypt
 * @param iv	For CBC mode, pointer to IV byte stream (16 bytes)
 * @note For ECB, OFB, and CFB, there is no nonce or iv.
 *       For CBC, IV0 and IV1 is loaded from SA and override by token IV0
 *	 and IV1.
 *       For CTR, IV0 - IV3 is loaded from SA and override by token IV0 -
 *	 IV3.
 */
void sec_tkn_update_crypt(u32 * t, u32 pl, u32 * ctrl, u8 ca, int enc);

/*
 * Create input token for gcm encrypt/decrypt operation
 * @param t	A pointer to input token
 * @param pl	Payload byte length to encrypt
 * @param sa	MSB 32-bit of a 36-bits physical address format. See function
 *			apm_sa_va2hw.
 * @param ca	Crypto algorithm to use. See SA_CRYPTO_ALG_XXX
 * @param cm	Crypto mode to use. See SA_CRYPTO_MODE_XXX
 * @param nonce Nonce value in SA format for CTR mode
 * @param iv	Pointer to IV byte stream (16 bytes) for CBC and CTR mode
 * @param ctrl  Two control control WORD
 * @return Return total number token byte required
 */
unsigned short sec_tkn_set_crypt_aes_gcm(unsigned int *t,
					 unsigned int pl, unsigned long long sa,
					 unsigned int *ctrl,
					 void *adata,
					 unsigned char assoclen,
					 unsigned char icv_len, int enc);

/*
 * Update input token for  gcm encrypt/decrypt operation
 * @param t	A pointer to input token
 * @param pl	Payload byte length to encrypt
 * @param iv	For CBC mode, pointer to IV byte stream (16 bytes)
 */
unsigned short sec_tkn_update_crypt_aes_gcm(unsigned int *t,
					    unsigned int pl,
					    unsigned long long sa,
					    unsigned int *ctrl,
					    void *adata,
					    unsigned char assoclen,
					    unsigned char icv_len, int enc);

/*
 * Create input token for ccm encrypt/decrypt operation
 * @param t	A pointer to input token
 * @param pl	Payload byte length to encrypt
 * @param sa	MSB 32-bit of a 36-bits physical address format. See function
 *			apm_sa_va2hw.
 * @param ca	Crypto algorithm to use. See SA_CRYPTO_ALG_XXX
 * @param cm	Crypto mode to use. See SA_CRYPTO_MODE_XXX
 * @param nonce Nonce value in SA format for CTR mode
 * @param iv	Pointer to IV byte stream (16 bytes) for CBC and CTR mode
 * @param ctrl  Two control control WORD
 * @return Return total number token byte required
 */
unsigned short sec_tkn_set_crypt_aes_ccm(unsigned int *t, unsigned int pl,
					 unsigned long long sa,
					 unsigned int *ctrl,
					 unsigned int icv_size,
					 unsigned int *iv_B0,
					 unsigned char *adata_length,
					 unsigned int adata_length_length,
					 unsigned char *adata,
					 unsigned char assoclen, int enc);

/*
 * Update input token for  gcm encrypt/decrypt operation
 * @param t	A pointer to input token
 * @param pl	Payload byte length to encrypt
 * @param iv	For CBC mode, pointer to IV byte stream (16 bytes)
 */
unsigned short sec_tkn_update_crypt_aes_ccm(unsigned int *t,
					    unsigned int pl,
					    unsigned long long sa,
					    unsigned int *ctrl,
					    unsigned int icv_size,
					    unsigned int *iv_B0,
					    unsigned char *adata_length,
					    unsigned int adata_length_length,
					    unsigned char *adata,
					    unsigned char assoclen, int enc);

/*
 * Create input token for  arc4 encrypt/decrypt operation
 * @param t	A pointer to input token
 * @param pl	Payload byte length to encrypt
 * @param sa	MSB 32-bit of a 36-bits physical address format. See function
 *			apm_sa_va2hw.
 * @param ctrl  Two control control WORD
 * @return Return total number token byte required
 */
unsigned short sec_tkn_set_arc4_crypt(unsigned int *t, unsigned int pl,
				      unsigned long long sa, unsigned int *ctrl,
				      int enc);

/*
 * Update input token for  gcm encrypt/decrypt operation
 * @param t	A pointer to input token
 * @param pl	Payload byte length to encrypt
 * @param iv	For CBC mode, pointer to IV byte stream (16 bytes)
 */
void sec_tkn_update_arc4(unsigned int *t, unsigned int pl, unsigned int *ctrl);

/*
 * Create ESP outbound token for tunnel mode
 * @param   t       Token pointer (DWORD pointer)
 * @param   tl      Variable to store token length counter
 * @param   pl      Packet length in bytes
 * @param   sa      Security context association physical address (32-bits)
 * @param   ipv6    For IPv6, set to 1. For IPv4, set to 0
 * @param   ethl    Ethernet header length in bytes
 * @param   iphl    IP header field length in bytes
 * @param   oiphl   Outer IP header length in bytes
 * @param   oiph    Outer IP header array of byte to insert
 * @param   ca      Crypto alg (IPE_SA_CRYPTO_ALG_XXX)
 * @param   cm      Crypto mode (IPE_SA_CRYPTO_MODE_AES_XXX)
 * @param   ha      Hash alg (IPE_SA_HASH_ALG_XXX - no XCBC, no GHASH, no
 *		     KASUMI)
 * @param   esn     Enable extend seq num if 1 - 64-bit
 * @param   pdl     Padding length in bytes - IPSec padding length
 * @param   iv_prng Use PRNG for IV if 1
 * @param   gcm_icv_len For GCM mode, set ICV length to non-zero (valid=8,
 *		        12, 16)
 * @return Token length in byte
 *
 * @note Security Context (SA) Format:
 *      0xCTRLW0    - Control Word 0
 *      0xCTRLW1    - Control Word 1
 *      0xKEY_0-7   - Crypto Key 128, 192, and 256
 *      0xDIGEST0-1 - Hash Digest 0, 1 - depend on hash alg
 *      0xSPI       - SPI
 *      0xSEQ0-1    - Seq number and extend seq (See seq01)
 *      0xIV0-3     - IV 0,1,2,3 - depend on crypto alg
 */
unsigned short sec_tkn_set_espout_tunnel(unsigned int *t,
					 unsigned int pl, unsigned long long sa,
					 unsigned int *ctrl, unsigned char ca,
					 unsigned char cm, unsigned char pdl,
					 unsigned char gcm_flag,
					 unsigned char seq_offset);

/*
 * Update ESP outbound token for tunnel mode
 * @param   t       Token pointer (DWORD pointer)
 * @param   pl      Packet length in bytes
 * @param   ipv6    For IPv6, set to 1. For IPv4, set to 0
 * @param   ethl    Ethernet header length in bytes
 * @param   iphl    IP header field length in bytes
 * @param   oiphl   Outer IP header length in bytes
 * @param   oiph    Outer IP header array of byte to insert
 * @param   pdl     Padding length in bytes - IPSec padding length
 * @param   gcm_icv_len For GCM mode, set ICV length to non-zero (valid=8, 12,
 *			16)
 */
void sec_tkn_update_espout_tunnel(unsigned int *t, unsigned int pl,
				  unsigned char cm, unsigned int icv_len,
				  unsigned char pad_len,
				  unsigned char gcm_flag);

/*
 * Create ESP inbound token for tunnel mode
 * @param   t       Token pointer (DWORD pointer)
 * @param   tl      Variable to store token length counter
 * @param   pl      Packet length in bytes
 * @param   sa      Security context association physical address (32-bits)
 * @param   ipv6    For IPv6, set to 1. For IPv4, set to 0
 * @param   ca      Crypto alg (IPE_SA_CRYPTO_ALG_XXX)
 * @param   cm      Crypto mode (IPE_SA_CRYPTO_MODE_AES_XXX)
 * @param   ha      Hash alg (IPE_SA_HASH_ALG_XXX - no XCBC, no GHASH, no
 *		     KASUMI)
 * @param   esn     Enable extend seq num if 1 - 64-bit
 * @param   mask01  SEQ number mask bit of SA CTRL WORD 0 (00=no mask;
 *		     01/10=Mask0,1; 11=Mask0,1,2,3)
 * @param   ethl    Ethernet header length in bytes
 * @param   oiphl   Outer IP header length in bytes
 * @param   pdl     Padding length in bytes - IPSec padding length
 * @param   iv_prng Use PRNG for IV if 1
 * @param   gcm_icv_len For GCM mode, set ICV length to non-zero (valid=8, 12,
 *			16)
 * @return Token length in byte
 *
 * @note Security Context (SA) Format:
 *      0xCTRLW0    - Control Word 0
 *      0xCTRLW1    - Control Word 1
 *      0xKEY_0-7   - Crypto Key 128, 192, and 256
 *      0xDIGEST0-1 - Hash Digest 0, 1 - depend on hash alg
 *      0xSPI       - SPI
 *      0xSEQ0-1    - Seq number and extend seq (See seq01)
 *      0xMASK0-3   - Seq number mask (see mask01)
 *      0xIV0-3     - IV 0,1,2,3 - depend on crypto alg
 */
unsigned short sec_tkn_set_espin_tunnel(unsigned int *t,
					unsigned int pl, unsigned long long sa,
					unsigned int *ctrl, unsigned char ca,
					unsigned char cm,
					unsigned char gcm_flag,
					unsigned int ihl, unsigned char icv_len,
					unsigned char seq_offset);

/*
 * Update ESP outbound token for tunnel mode
 * @param   t       Token pointer (DWORD pointer)
 * @param   pl      Packet length in bytes
 * @param   ipv6    For IPv6, set to 1. For IPv4, set to 0
 * @param   ethl    Ethernet header length in bytes
 * @param   oiphl   Outer IP header length in bytes
 * @param   gcm_icv_len For GCM mode, set ICV length to non-zero (valid=8, 12,
 *			16)
 */
void sec_tkn_update_espin_tunnel(unsigned int *t, unsigned int pl,
				 unsigned char cm, unsigned char ca,
				 unsigned int icvsize, unsigned int gcm_flag);

/*
 * Create ESP outbound token for transport mode AES/DES/DES3/AES(CTR)
 * @param   t       Token pointer (DWORD pointer)
 * @param   tl      Variable to store token length counter
 * @param   pl      Packet length in bytes
 * @param   sa      Security context association physical address (32-bits)
 * @param   ca      Crypto alg (IPE_SA_CRYPTO_ALG_XXX)
 * @param   pdl     Padding length in bytes - IPSec padding length
 * @param   icvsize Authentication length calculated for payload + esp data
 * @param   nh      Next Header value to be placed in the ESP trailer
 * @param   seq_offset Offset in the Control word where the upadted
 *			sequence is to be placed
 * @return Token length in byte
 *
 * @note Security Context (SA) Format:
 *      0xCTRLW0    - Control Word 0
 *      0xCTRLW1    - Control Word 1
 *      0xKEY_0-7   - Crypto Key 128, 192, and 256
 *      0xDIGEST0-1 - Hash Digest 0, 1 - depend on hash alg
 *      0xSPI       - SPI
 *      0xSEQ0-1    - Seq number and extend seq (See seq01)
 *      0xIV0-3     - IV 0,1,2,3 - depend on crypto alg
 *	This token set routine works for AES/DES/DES3/AES(CTR) Algorithms
 */
unsigned short sec_tkn_set_espout_transport(unsigned int *t,
					    unsigned int pl,
					    unsigned long long sa,
					    unsigned int *ctrl,
					    unsigned char ca, unsigned char pdl,
					    unsigned char icvsize,
					    unsigned char nh,
					    unsigned char iphl,
					    unsigned char seq_offset,
					    unsigned char cm, int gcm_flag);

/*
 * Update ESP outbound token for transport mode AES/DES/DES3/AES(CTR)
 * @param   t       Token pointer (DWORD pointer)
 * @param   tl      Variable to store token length counter
 * @param   pl      Packet length in bytes
 * @param   ca      Crypto alg (IPE_SA_CRYPTO_ALG_XXX)
 * @param   pdl     Padding length in bytes - IPSec padding length
 * @param   icvsize Authentication length calculated for payload + esp data
 * @param   nh      Next Header value to be placed in the ESP trailer
 */
void sec_tkn_update_espout_transport(unsigned int *t, unsigned int pl,
				     unsigned char ca, unsigned char pdl,
				     unsigned char nh, unsigned char iphl,
				     unsigned char icvsize, unsigned char cm,
				     int gcm_flag);

/*
 * Create ESP inbound token for transport mode AES/DES/DES3/AES(CTR)
 * @param   t       Token pointer (DWORD pointer)
 * @param   tl      Variable to store token length counter
 * @param   pl      Packet length in bytes
 * @param   sa      Security context association physical address (32-bits)
 * @param   ca      Crypto alg (IPE_SA_CRYPTO_ALG_XXX)
 * @param   pdl     Padding length in bytes - IPSec padding length
 * @param   icvsize Authentication length verified for payload + esp data
 * @param   seq_offset Offset in the Control word where the upadted
 *			sequence is to be placed
 * @return Token length in byte
 *
 * @note Security Context (SA) Format:
 *      0xCTRLW0    - Control Word 0
 *      0xCTRLW1    - Control Word 1
 *      0xKEY_0-7   - Crypto Key 128, 192, and 256
 *      0xDIGEST0-1 - Hash Digest 0, 1 - depend on hash alg
 *      0xSPI       - SPI
 *      0xSEQ0-1    - Seq number and extend seq (See seq01)
 *      0xIV0-3     - IV 0,1,2,3 - depend on crypto alg
 *
 *	This token works for AES/DES/DES3/AES(CTR) Algorithms
 */
unsigned short sec_tkn_set_espin_transport(unsigned int *t, unsigned int pl,
					   unsigned long long sa,
					   unsigned int *ctrl, unsigned char ca,
					   unsigned int ihl,
					   unsigned char icv_len,
					   unsigned char seq_offset,
					   unsigned char cm, int gcm_flag);

/*
 * Update ESP inbound token for transport mode AES/DES/DES3/AES(CTR)
 * @param   t       Token pointer (DWORD pointer)
 * @param   tl      Variable to store token length counter
 * @param   pl      Packet length in bytes
 * @param   ca      Crypto alg (IPE_SA_CRYPTO_ALG_XXX)
 * @param   icvsize Authentication length verified for payload + esp data
 */
void sec_tkn_update_espin_transport(unsigned int *t,
				    unsigned int pl, unsigned char ca,
				    unsigned short ihl, unsigned char icv_len,
				    unsigned char cm, int gcm_flag);

/*
 * Outbound ESP Transport Create/Update Token for CCM
 */
unsigned short sec_tkn_set_espout_transport_ccm(unsigned int *t,
						unsigned int pl,
						unsigned long long sa,
						unsigned int *ctrl,
						unsigned char pdl,
						unsigned char icvsize,
						unsigned char nh,
						unsigned char iphl,
						unsigned char *iv_B0,
						unsigned char *adata_length,
						unsigned char seq_offset);
unsigned short sec_tkn_update_espout_transport_ccm(unsigned int *t,
						   unsigned int pl,
						   unsigned long long sa,
						   unsigned char pdl,
						   unsigned char icvsize,
						   unsigned char nh,
						   unsigned char iphl,
						   unsigned char *iv_B0,
						   unsigned char *adata_length);
unsigned short sec_tkn_set_espin_transport_ccm(unsigned int *t, unsigned int pl,
					       unsigned long long sa,
					       unsigned int *ctrl,
					       unsigned char pdl,
					       unsigned char icvsize,
					       unsigned char nh,
					       unsigned char *iv_B0,
					       unsigned char *adata_length,
					       unsigned char seq_offset);

/*
 * Sample SSL/TLS/DTLS Token Templates
 */
#define IPE_TKN_SSL_TLS_OUT_MAXSIZE     ((4+2+15)<<2)
#define IPE_TKN_SSL_TLS_IN_MAXSIZE      ((4+2+15)<<2)
#define IPE_TKN_DTLS_OUT_MAXSIZE     	((4+2+15)<<2)

#define ipe_tkn_ssl_tls_vSSL3_0(v)      (((v) == 0x0300)?1:0)
#define ipe_tkn_ssl_tls_vTLS1_0(v)      (((v) == 0x0301)?1:0)
#define ipe_tkn_ssl_tls_vTLS1_1(v)      (((v) == 0x0302)?1:0)

/*
 * Compute the required padding length
 *
 * @param   pad_var     Variable to save to
 * @param   pyl         Payload of the IP packet in bytes
 * @param   pdl         Desire pad length in bytes
 *
 * @note    Minimum padding is 1 block size (pdl). If pad length is 1 bytes,
 *          one extra block size is added.
 */
#define sec_tkn_compute_pad_len(pad_var, pyl, pdl) { \
	/* IPSec padding - 1 extra block if already multiple */ \
	if ((pdl) == 0) \
		(pad_var) = 0; \
	else \
		(pad_var) = (pdl) - ((pyl) % (pdl)); \
	/* If padding is 1 bytes, add one more full pad length as we */ \
	/*  require 2 bytes */ \
	if ((pad_var) == 1) \
		(pad_var) += (pdl); \
}

/*
 * Create token for SSL/TLS outbound
 * @param   t       Token pointer (DWORD pointer)
 * @param   pl      Packet length in bytes
 * @param   sa      Security context association physical address (32-bits)
 * @param   ha      Hash alg (IPE_SA_HASH_ALG_MD5, IPE_SA_HASH_ALG_SHA1_SSL_MAC,
 *                      IPE_SA_HASH_ALG_SHA1)
 * @param   ca      Crypto alg (IPE_SA_CRYPTO_ALG_NULL, IPE_SA_CRYPTO_ALG_DES,
 *                      IPE_SA_CRYPTO_ALG_3DES, IPE_SA_CRYPTO_ALG_ARC4,
 *                      IPE_SA_CRYPTO_ALG_AESXXX in CBC mode)
 * @param   cm      Crypto mode. NOTE: for AES,
 *			cm musts be IPE_SA_CRYPTO_MODE_AES_3DES_CBC.
 *                      For ARC4, ARC4 key length.
 * @param   ijptr           I-J ptr field of ARC4
 * @param   arc4_statefull  ARC4 state full field of ARC4
 * @param   iv_prng         For IV, use PRNG if 1
 * @param   ssl_tls_ver     Version of SSL/TLS
 *
 * @note  Security Context:
 *          0xCTRL0
 *          0xCTRL1
 *          0xKEY       - Crypto Key
 *          0xHASH      - Hash digest
 *          0xSPI       - SSL/TLS type[31:24] + version major[23-16] +
 *			version minor[15-8] + 0
 *          0xIV0-3     - Depend on crypto alg
 *          0xARC4      - ARC4 state if IPE_SA_CRYPTO_ALG_ARC4
 *          0xIJPTR     - ARC4 i-j ptr if IPE_SA_CRYPTO_ALG_ARC4
 *        For MD5, first inner hash block musts be pre-calculated =
 *					(MAC_KEY + pad_1).
 *        First outer hash block musts be pre-calculated = (MACK_KEY + pad_2).
 *        For SHA1, only MAC_KEY is provided in
 *				context record (inner digest field).
 *        The rest is calculated internally.
 */
unsigned short sec_tkn_set_ssl_tls_out(unsigned int *t,
				       unsigned int pl, unsigned long long sa,
				       unsigned int *ctrl, unsigned int padlen,
				       unsigned char seq_offset, int icvsize,
				       int arc4, int enc, int arc4_state_ptr,
				       int arc4_ij_ptr, int opcode, int ivsize);

unsigned short sec_tkn_set_ssl_tls_in(unsigned int *t,
				      unsigned int frag_len, unsigned int pyl,
				      unsigned long long sa, unsigned int *ctrl,
				      unsigned int padlen,
				      unsigned char seq_offset, int icvsize,
				      int ha, int enc, int opcode, int ivsize);

/*
 * DTLS Token Templates
 */
unsigned short sec_tkn_set_dtls_out(unsigned int *t,
				    unsigned int pl, unsigned long long sa,
				    unsigned int *ctrl, unsigned int padlen,
				    unsigned char seq_offset, int icvsize,
				    int frag_len, int ivsize, int enc);
unsigned short sec_tkn_set_dtls_in(unsigned int *t, unsigned int frag_len,
				   unsigned int pyl, unsigned long long sa,
				   unsigned int *ctrl, unsigned int padlen,
				   unsigned char seq_offset, int icvsize,
				   int ha, int ivsize, int enc);

unsigned short sec_tkn_set_encrypt_cbc_authenc(unsigned int *t, unsigned int pl,
					       unsigned long long sa,
					       unsigned int *ctrl,
					       unsigned char ca,
					       unsigned int pdl,
					       unsigned char icvsize);
void sec_tkn_update_encrypt_cbc_authenc(unsigned int *t, unsigned int pl,
					unsigned int *ctrl, unsigned char ca,
					unsigned int pdl,
					unsigned char icvsize);
unsigned short sec_tkn_set_decrypt_cbc_authenc(unsigned int *t, unsigned int pl,
					       unsigned long long sa,
					       unsigned int *ctrl,
					       unsigned char ca,
					       unsigned int pdl,
					       unsigned char icvsize);
void sec_tkn_update_decrypt_cbc_authenc(unsigned int *t, unsigned int pl,
					unsigned int *ctrl, unsigned char ca,
					unsigned int pdl,
					unsigned char icvsize);
#endif
