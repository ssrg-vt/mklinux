/*
 * AppliedMicro X-Gene SoC PktDma Driver
 *
 * Copyright (c) 2014, Applied Micro Circuits Corporation
 * Author: Loc Ho <lho@apm.com>
 *         Rameshwar Prasad Sahu <rsahu@apm.com>
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
#ifndef __XGENE_PKTDMA_ACCESS_H__
#define __XGENE_PKTDMA_ACCESS_H__

#define FBY_CRC16			0x1
#define FBY_ISCSI_CRC32C		0x2
#define FBY_CRC32			0x3
#define FBY_CHECKSUM			0x4

int xgene_flyby_op(void (*cb_func)(void *ctx, u32 result), 
			void *ctx, struct scatterlist *src_sg, 
			u32 nbytes, u32 seed, u8 opcode);

int xgene_flyby_alloc_resources(void);

void xgene_flyby_free_resources(void);

#endif /* __XGENE_PKTDMA_ACCESS_H__ */
