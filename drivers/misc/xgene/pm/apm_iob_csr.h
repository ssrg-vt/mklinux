/**
 * apm_iob_csr.h - AppliedMicro APM88xxxx PCP Bridge Header
 *
 * Copyright (c) 2013, Applied Micro Circuits Corporation
 * Author: Loc Ho <lho@apm.com>
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
 *
 */
#ifndef _APM_IOB_CSR_H__
#define _APM_IOB_CSR_H__

/*	Global Base Address	*/
#define IOB_RBM_REGS_BASE_ADDR			0x07e000000ULL

/*    Address IOB_RBM_REGS  Registers */
#define RBCSR_ADDR                                                   0x00000000
#define RBCSR_DEFAULT                                                0x00000000
#define RBASR_ADDR                                                   0x00000004
#define RBASR_DEFAULT                                                0x00000000
#define RBEIR_ADDR                                                   0x00000008
#define RBEIR_DEFAULT                                                0x00000000
#define RBNR_ADDR                                                    0x0000000c
#define RBNR_DEFAULT                                                 0x00000000

/*	Register RBCSR	*/ 
/*	 Fields STICKYERR	 */
#define STICKYERR_WIDTH                                                       1
#define STICKYERR_SHIFT                                                       0
#define STICKYERR_MASK                                               0x00000001
#define STICKYERR_RD(src)                                (((src) & 0x00000001))
#define STICKYERR_WR(src)                           (((u32)(src)) & 0x00000001)
#define STICKYERR_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register RBASR	*/ 
/*	 Fields L3CAVAIL	 */
#define L3CAVAIL_WIDTH                                                        1
#define L3CAVAIL_SHIFT                                                       16
#define L3CAVAIL_MASK                                                0x00010000
#define L3CAVAIL_RD(src)                             (((src) & 0x00010000)>>16)
#define L3CAVAIL_WR(src)                        (((u32)(src)<<16) & 0x00010000)
#define L3CAVAIL_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields PMDAVAIL	 */
#define PMDAVAIL_WIDTH                                                        4
#define PMDAVAIL_SHIFT                                                        0
#define PMDAVAIL_MASK                                                0x0000000f
#define PMDAVAIL_RD(src)                                 (((src) & 0x0000000f))
#define PMDAVAIL_WR(src)                            (((u32)(src)) & 0x0000000f)
#define PMDAVAIL_SET(dst,src) \
                          (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register RBEIR	*/ 
/*	 Fields CPUDBGACCESSERR	 */
#define CPUDBGACCESSERR_WIDTH                                                 1
#define CPUDBGACCESSERR_SHIFT                                                31
#define CPUDBGACCESSERR_MASK                                         0x80000000
#define CPUDBGACCESSERR_RD(src)                      (((src) & 0x80000000)>>31)
#define CPUDBGACCESSERR_WR(src)                 (((u32)(src)<<31) & 0x80000000)
#define CPUDBGACCESSERR_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields AGENTOFFLINEERR	 */
#define AGENTOFFLINEERR_WIDTH                                                 1
#define AGENTOFFLINEERR_SHIFT                                                30
#define AGENTOFFLINEERR_MASK                                         0x40000000
#define AGENTOFFLINEERR_RD(src)                      (((src) & 0x40000000)>>30)
#define AGENTOFFLINEERR_WR(src)                 (((u32)(src)<<30) & 0x40000000)
#define AGENTOFFLINEERR_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields UNIMPLRBPAGEERR	 */
#define UNIMPLRBPAGEERR_WIDTH                                                 1
#define UNIMPLRBPAGEERR_SHIFT                                                29
#define UNIMPLRBPAGEERR_MASK                                         0x20000000
#define UNIMPLRBPAGEERR_RD(src)                      (((src) & 0x20000000)>>29)
#define UNIMPLRBPAGEERR_WR(src)                 (((u32)(src)<<29) & 0x20000000)
#define UNIMPLRBPAGEERR_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields WORDALIGNEDERR	 */
#define WORDALIGNEDERR_WIDTH                                                  1
#define WORDALIGNEDERR_SHIFT                                                 28
#define WORDALIGNEDERR_MASK                                          0x10000000
#define WORDALIGNEDERR_RD(src)                       (((src) & 0x10000000)>>28)
#define WORDALIGNEDERR_WR(src)                  (((u32)(src)<<28) & 0x10000000)
#define WORDALIGNEDERR_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields PAGEACCESSERR	 */
#define PAGEACCESSERR_WIDTH                                                   1
#define PAGEACCESSERR_SHIFT                                                  27
#define PAGEACCESSERR_MASK                                           0x08000000
#define PAGEACCESSERR_RD(src)                        (((src) & 0x08000000)>>27)
#define PAGEACCESSERR_WR(src)                   (((u32)(src)<<27) & 0x08000000)
#define PAGEACCESSERR_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields WRITE	 */
#define RBEIR_REGSPEC_WRITE_WIDTH                                                   1
#define RBEIR_REGSPEC_WRITE_SHIFT                                                  26
#define RBEIR_REGSPEC_WRITE_MASK                                           0x04000000
#define RBEIR_REGSPEC_WRITE_RD(src)                        (((src) & 0x04000000)>>26)
#define RBEIR_REGSPEC_WRITE_WR(src)                   (((u32)(src)<<26) & 0x04000000)
#define RBEIR_REGSPEC_WRITE_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields ADDRESS	 */
#define RBEIR_ADDRESS_WIDTH                                                        26
#define RBEIR_ADDRESS_SHIFT                                                         0
#define RBEIR_ADDRESS_MASK                                                 0x03ffffff
#define RBEIR_ADDRESS_RD(src)                                  (((src) & 0x03ffffff))
#define RBEIR_ADDRESS_WR(src)                             (((u32)(src)) & 0x03ffffff)
#define RBEIR_ADDRESS_SET(dst,src) \
                          (((dst) & ~0x03ffffff) | (((u32)(src)) & 0x03ffffff))

/*	Register RBNR	*/ 

/*	Global Base Address	*/
#define IOB_CSW_REGS_BASE_ADDR			0x07e900000ULL

/*    Address IOB_CSW_REGS  Registers */
#define CSWRCR_ADDR                                                  0x00000000
#define CSWRCR_DEFAULT                                               0x00000001
#define CSWCCR_ADDR                                                  0x00000008
#define CSWCCR_DEFAULT                                               0x00008b20

/*	Register CSWRCR	*/ 
/*	 Fields RESETCSW	 */
#define IOB_RESETCSW_WIDTH                                                        1
#define IOB_RESETCSW_SHIFT                                                        0
#define IOB_RESETCSW_MASK                                                0x00000001
#define IOB_RESETCSW_RD(src)                                 (((src) & 0x00000001))
#define IOB_RESETCSW_WR(src)                            (((u32)(src)) & 0x00000001)
#define IOB_RESETCSW_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register CSWCCR	*/ 
/*	 Fields CSWCLKMACRST	 */
#define CSWCLKMACRST_WIDTH                                                    1
#define CSWCLKMACRST_SHIFT                                                   15
#define CSWCLKMACRST_MASK                                            0x00008000
#define CSWCLKMACRST_RD(src)                         (((src) & 0x00008000)>>15)
#define CSWCLKMACRST_WR(src)                    (((u32)(src)<<15) & 0x00008000)
#define CSWCLKMACRST_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields CSWCLKMACPD	 */
#define CSWCLKMACPD_WIDTH                                                     1
#define CSWCLKMACPD_SHIFT                                                    14
#define CSWCLKMACPD_MASK                                             0x00004000
#define CSWCLKMACPD_RD(src)                          (((src) & 0x00004000)>>14)
#define CSWCLKMACPD_WR(src)                     (((u32)(src)<<14) & 0x00004000)
#define CSWCLKMACPD_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields CSWCLKEN	 */
#define CSWCLKEN_WIDTH                                                        1
#define CSWCLKEN_SHIFT                                                       11
#define CSWCLKEN_MASK                                                0x00000800
#define CSWCLKEN_RD(src)                             (((src) & 0x00000800)>>11)
#define CSWCLKEN_WR(src)                        (((u32)(src)<<11) & 0x00000800)
#define CSWCLKEN_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields CLUSTERBYPEN	 */
#define CLUSTERBYPEN_WIDTH                                                    1
#define CLUSTERBYPEN_SHIFT                                                    9
#define CLUSTERBYPEN_MASK                                            0x00000200
#define CLUSTERBYPEN_RD(src)                          (((src) & 0x00000200)>>9)
#define CLUSTERBYPEN_WR(src)                     (((u32)(src)<<9) & 0x00000200)
#define CLUSTERBYPEN_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields CLUSTERCLKEN	 */
#define CLUSTERCLKEN_WIDTH                                                    1
#define CLUSTERCLKEN_SHIFT                                                    8
#define CLUSTERCLKEN_MASK                                            0x00000100
#define CLUSTERCLKEN_RD(src)                          (((src) & 0x00000100)>>8)
#define CLUSTERCLKEN_WR(src)                     (((u32)(src)<<8) & 0x00000100)
#define CLUSTERCLKEN_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields CMLNSCORREN	 */
#define CMLNSCORREN_WIDTH                                                     1
#define CMLNSCORREN_SHIFT                                                     5
#define CMLNSCORREN_MASK                                             0x00000020
#define CMLNSCORREN_RD(src)                           (((src) & 0x00000020)>>5)
#define CMLNSCORREN_WR(src)                      (((u32)(src)<<5) & 0x00000020)
#define CMLNSCORREN_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields SKEWTUNE	 */
#define SKEWTUNE_WIDTH                                                        5
#define SKEWTUNE_SHIFT                                                        0
#define SKEWTUNE_MASK                                                0x0000001f
#define SKEWTUNE_RD(src)                                 (((src) & 0x0000001f))
#define SKEWTUNE_WR(src)                            (((u32)(src)) & 0x0000001f)
#define SKEWTUNE_SET(dst,src) \
                          (((dst) & ~0x0000001f) | (((u32)(src)) & 0x0000001f))

/*	Global Base Address	*/
#define IOB_CFG_REGS_BASE_ADDR			0x07e920000ULL

/*    Address IOB_CFG_REGS  Registers */
#define IOBPACFG_ADDR                                                0x00000000
#define IOBPACFG_DEFAULT                                             0x08040400
#define IOBVCTOKENCFG_ADDR                                           0x00000004
#define IOBVCTOKENCFG_DEFAULT                                        0x00000000
#define IOBBACFG_ADDR                                                0x00000010
#define IOBBACFG_DEFAULT                                             0x00000000
#define IOBREMAPRNG0_ADDR                                            0x00000020
#define IOBREMAPRNG0_DEFAULT                                         0x00000000
#define IOBREMAPRNG1_ADDR                                            0x00000024
#define IOBREMAPRNG1_DEFAULT                                         0x00000000
#define IOBREMAPRNG2_ADDR                                            0x00000028
#define IOBREMAPRNG2_DEFAULT                                         0x00000000
#define IOBREMAPRNG3_ADDR                                            0x0000002c
#define IOBREMAPRNG3_DEFAULT                                         0x00000000
#define IOBREMAPRNG4_ADDR                                            0x00000030
#define IOBREMAPRNG4_DEFAULT                                         0x00000000
#define IOBREMAPRNG5_ADDR                                            0x00000034
#define IOBREMAPRNG5_DEFAULT                                         0x00000000
#define IOBREMAPBAR0_ADDR                                            0x00000040
#define IOBREMAPBAR0_DEFAULT                                         0x00000000
#define IOBREMAPBAR1_ADDR                                            0x00000044
#define IOBREMAPBAR1_DEFAULT                                         0x00000000
#define IOBREMAPBAR2_ADDR                                            0x00000048
#define IOBREMAPBAR2_DEFAULT                                         0x00000000
#define IOBREMAPBAR3_ADDR                                            0x0000004c
#define IOBREMAPBAR3_DEFAULT                                         0x00000000
#define IOBREMAPBAR4_ADDR                                            0x00000050
#define IOBREMAPBAR4_DEFAULT                                         0x00000000
#define IOBREMAPBAR5_ADDR                                            0x00000054
#define IOBREMAPBAR5_DEFAULT                                         0x00000000
#define IOBREMAPTAR5_ADDR                                            0x00000074
#define IOBREMAPTAR5_DEFAULT                                         0x00000000
#define IOBTRACECFG_ADDR                                             0x00000080
#define IOBTRACECFG_DEFAULT                                          0x00000000
#define IOBMEMCFG00_ADDR                                             0x00000100
#define IOBMEMCFG00_DEFAULT                                          0x00000099
#define IOBMEMCFG01_ADDR                                             0x00000104
#define IOBMEMCFG01_DEFAULT                                          0x00000099
#define IOBMEMCFG02_ADDR                                             0x00000108
#define IOBMEMCFG02_DEFAULT                                          0x00000099
#define IOBMEMCFG03_ADDR                                             0x0000010c
#define IOBMEMCFG03_DEFAULT                                          0x00000099
#define IOBMEMCFG04_ADDR                                             0x00000110
#define IOBMEMCFG04_DEFAULT                                          0x00000099
#define IOBMEMCFG05_ADDR                                             0x00000114
#define IOBMEMCFG05_DEFAULT                                          0x00000099
#define IOBMEMCFG06_ADDR                                             0x00000118
#define IOBMEMCFG06_DEFAULT                                          0x00000099
#define IOBMEMCFG07_ADDR                                             0x0000011c
#define IOBMEMCFG07_DEFAULT                                          0x00000099
#define IOBMEMCFG08_ADDR                                             0x00000120
#define IOBMEMCFG08_DEFAULT                                          0x00000099
#define IOBMEMCFG09_ADDR                                             0x00000124
#define IOBMEMCFG09_DEFAULT                                          0x00000099
#define IOBMEMCFG0A_ADDR                                             0x00000128
#define IOBMEMCFG0A_DEFAULT                                          0x00000099
#define IOBMEMCFG0B_ADDR                                             0x0000012c
#define IOBMEMCFG0B_DEFAULT                                          0x00000099
#define IOBMEMCFG0C_ADDR                                             0x00000130
#define IOBMEMCFG0C_DEFAULT                                          0x00000099
#define IOBMEMCFG0D_ADDR                                             0x00000134
#define IOBMEMCFG0D_DEFAULT                                          0x00000099
#define IOBMEMCFG0E_ADDR                                             0x00000138
#define IOBMEMCFG0E_DEFAULT                                          0x00000099
#define IOBMEMCFG0F_ADDR                                             0x0000013c
#define IOBMEMCFG0F_DEFAULT                                          0x00000099
#define IOBMEMCFG10_ADDR                                             0x00000140
#define IOBMEMCFG10_DEFAULT                                          0x00000099
#define IOBMEMCFG11_ADDR                                             0x00000144
#define IOBMEMCFG11_DEFAULT                                          0x00000099
#define IOBMEMCFG12_ADDR                                             0x00000148
#define IOBMEMCFG12_DEFAULT                                          0x00000099
#define IOBMEMCFG13_ADDR                                             0x0000014c
#define IOBMEMCFG13_DEFAULT                                          0x00000099
#define IOBMEMCFG14_ADDR                                             0x00000150
#define IOBMEMCFG14_DEFAULT                                          0x00000099
#define IOBMEMCFG15_ADDR                                             0x00000154
#define IOBMEMCFG15_DEFAULT                                          0x00000099
#define IOBMEMCFG16_ADDR                                             0x00000158
#define IOBMEMCFG16_DEFAULT                                          0x00000099
#define IOBMEMCFG17_ADDR                                             0x0000015c
#define IOBMEMCFG17_DEFAULT                                          0x00000099
#define IOBMEMCFG18_ADDR                                             0x00000160
#define IOBMEMCFG18_DEFAULT                                          0x00000099
#define IOBMEMCFG19_ADDR                                             0x00000164
#define IOBMEMCFG19_DEFAULT                                          0x00000099
#define IOBMEMCFG1A_ADDR                                             0x00000168
#define IOBMEMCFG1A_DEFAULT                                          0x00000099
#define IOBMEMCFG1B_ADDR                                             0x0000016c
#define IOBMEMCFG1B_DEFAULT                                          0x00000099
#define IOBMEMCFG1C_ADDR                                             0x00000170
#define IOBMEMCFG1C_DEFAULT                                          0x00000099
#define IOBMEMCFG1D_ADDR                                             0x00000174
#define IOBMEMCFG1D_DEFAULT                                          0x00000099
#define IOBMEMCFG1E_ADDR                                             0x00000178
#define IOBMEMCFG1E_DEFAULT                                          0x00000099
#define IOBMEMCFG1F_ADDR                                             0x0000017c
#define IOBMEMCFG1F_DEFAULT                                          0x00000099
#define IOBMEMCFG20_ADDR                                             0x00000180
#define IOBMEMCFG20_DEFAULT                                          0x00000099
#define IOBMEMCFG21_ADDR                                             0x00000184
#define IOBMEMCFG21_DEFAULT                                          0x00000099
#define IOBMEMCFG22_ADDR                                             0x00000188
#define IOBMEMCFG22_DEFAULT                                          0x00000099
#define IOBMEMCFG23_ADDR                                             0x0000018c
#define IOBMEMCFG23_DEFAULT                                          0x00000099
#define IOBMEMCFG24_ADDR                                             0x00000190
#define IOBMEMCFG24_DEFAULT                                          0x00000099
#define IOBMEMCFG25_ADDR                                             0x00000194
#define IOBMEMCFG25_DEFAULT                                          0x00000099
#define IOBMEMCFG26_ADDR                                             0x00000198
#define IOBMEMCFG26_DEFAULT                                          0x00000099
#define IOBMEMCFG27_ADDR                                             0x0000019c
#define IOBMEMCFG27_DEFAULT                                          0x00000099
#define IOBMEMCFG28_ADDR                                             0x000001a0
#define IOBMEMCFG28_DEFAULT                                          0x00000099
#define IOBMEMCFG29_ADDR                                             0x000001a4
#define IOBMEMCFG29_DEFAULT                                          0x00000099
#define IOBMEMCFG2A_ADDR                                             0x000001a8
#define IOBMEMCFG2A_DEFAULT                                          0x00000099
#define IOBMEMCFG2B_ADDR                                             0x000001ac
#define IOBMEMCFG2B_DEFAULT                                          0x00000099
#define IOBMEMCFG2C_ADDR                                             0x000001b0
#define IOBMEMCFG2C_DEFAULT                                          0x00000099
#define IOBMEMCFG2D_ADDR                                             0x000001b4
#define IOBMEMCFG2D_DEFAULT                                          0x00000099
#define IOBMEMCFG2E_ADDR                                             0x000001b8
#define IOBMEMCFG2E_DEFAULT                                          0x00000099
#define IOBMEMCFG2F_ADDR                                             0x000001bc
#define IOBMEMCFG2F_DEFAULT                                          0x00000099
#define IOBMEMCFG30_ADDR                                             0x000001c0
#define IOBMEMCFG30_DEFAULT                                          0x00000000
#define IOBMEMCFG31_ADDR                                             0x000001c4
#define IOBMEMCFG31_DEFAULT                                          0x00000000
#define IOBMEMCFG32_ADDR                                             0x000001c8
#define IOBMEMCFG32_DEFAULT                                          0x00000000
#define IOBMEMCFG33_ADDR                                             0x000001cc
#define IOBMEMCFG33_DEFAULT                                          0x00000000
#define IOBMEMCFG34_ADDR                                             0x000001d0
#define IOBMEMCFG34_DEFAULT                                          0x00000000
#define IOBMEMCFG35_ADDR                                             0x000001d4
#define IOBMEMCFG35_DEFAULT                                          0x00000000
#define IOBMEMCFG36_ADDR                                             0x000001d8
#define IOBMEMCFG36_DEFAULT                                          0x00000000
#define IOBMEMCFG37_ADDR                                             0x000001dc
#define IOBMEMCFG37_DEFAULT                                          0x00000000
#define IOBMEMCFG38_ADDR                                             0x000001e0
#define IOBMEMCFG38_DEFAULT                                          0x00000000
#define IOBMEMCFG39_ADDR                                             0x000001e4
#define IOBMEMCFG39_DEFAULT                                          0x00000000
#define IOBMEMCFG3A_ADDR                                             0x000001e8
#define IOBMEMCFG3A_DEFAULT                                          0x00000000
#define IOBMEMCFG3B_ADDR                                             0x000001ec
#define IOBMEMCFG3B_DEFAULT                                          0x00000000
#define IOBMEMCFG3C_ADDR                                             0x000001f0
#define IOBMEMCFG3C_DEFAULT                                          0x00000000
#define IOBMEMCFG3D_ADDR                                             0x000001f4
#define IOBMEMCFG3D_DEFAULT                                          0x00000000
#define IOBMEMCFG3E_ADDR                                             0x000001f8
#define IOBMEMCFG3E_DEFAULT                                          0x00000000
#define IOBMEMCFG3F_ADDR                                             0x000001fc
#define IOBMEMCFG3F_DEFAULT                                          0x00000000
#define PCPEFUSECTRL_ADDR                                            0x00000200
#define PCPEFUSECTRL_DEFAULT                                         0x00000000
#define PCPEFUSESTAT_ADDR                                            0x00000204
#define PCPEFUSESTAT_DEFAULT                                         0x00000001

/*	Register IOBPACFG	*/ 
/*	 Fields COS2CREDITCFG	 */
#define COS2CREDITCFG_WIDTH                                                   5
#define COS2CREDITCFG_SHIFT                                                  24
#define COS2CREDITCFG_MASK                                           0x1f000000
#define COS2CREDITCFG_RD(src)                        (((src) & 0x1f000000)>>24)
#define COS2CREDITCFG_WR(src)                   (((u32)(src)<<24) & 0x1f000000)
#define COS2CREDITCFG_SET(dst,src) \
                      (((dst) & ~0x1f000000) | (((u32)(src)<<24) & 0x1f000000))
/*	 Fields COS1CREDITCFG	 */
#define COS1CREDITCFG_WIDTH                                                   5
#define COS1CREDITCFG_SHIFT                                                  16
#define COS1CREDITCFG_MASK                                           0x001f0000
#define COS1CREDITCFG_RD(src)                        (((src) & 0x001f0000)>>16)
#define COS1CREDITCFG_WR(src)                   (((u32)(src)<<16) & 0x001f0000)
#define COS1CREDITCFG_SET(dst,src) \
                      (((dst) & ~0x001f0000) | (((u32)(src)<<16) & 0x001f0000))
/*	 Fields COS0CREDITCFG	 */
#define COS0CREDITCFG_WIDTH                                                   5
#define COS0CREDITCFG_SHIFT                                                   8
#define COS0CREDITCFG_MASK                                           0x00001f00
#define COS0CREDITCFG_RD(src)                         (((src) & 0x00001f00)>>8)
#define COS0CREDITCFG_WR(src)                    (((u32)(src)<<8) & 0x00001f00)
#define COS0CREDITCFG_SET(dst,src) \
                       (((dst) & ~0x00001f00) | (((u32)(src)<<8) & 0x00001f00))
/*	 Fields MAXREQ	 */
#define MAXREQ_WIDTH                                                          3
#define MAXREQ_SHIFT                                                          0
#define MAXREQ_MASK                                                  0x00000007
#define MAXREQ_RD(src)                                   (((src) & 0x00000007))
#define MAXREQ_WR(src)                              (((u32)(src)) & 0x00000007)
#define MAXREQ_SET(dst,src) \
                          (((dst) & ~0x00000007) | (((u32)(src)) & 0x00000007))

/*	Register IOBVCTOKENCFG	*/ 
/*	 Fields VC_TOKEN_INIT	 */
#define VC_TOKEN_INIT_WIDTH                                                   1
#define VC_TOKEN_INIT_SHIFT                                                  31
#define VC_TOKEN_INIT_MASK                                           0x80000000
#define VC_TOKEN_INIT_RD(src)                        (((src) & 0x80000000)>>31)
#define VC_TOKEN_INIT_WR(src)                   (((u32)(src)<<31) & 0x80000000)
#define VC_TOKEN_INIT_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields VC2_TOKEN	 */
#define VC2_TOKEN_WIDTH                                                       9
#define VC2_TOKEN_SHIFT                                                      20
#define VC2_TOKEN_MASK                                               0x1ff00000
#define VC2_TOKEN_RD(src)                            (((src) & 0x1ff00000)>>20)
#define VC2_TOKEN_WR(src)                       (((u32)(src)<<20) & 0x1ff00000)
#define VC2_TOKEN_SET(dst,src) \
                      (((dst) & ~0x1ff00000) | (((u32)(src)<<20) & 0x1ff00000))
/*	 Fields VC1_TOKEN	 */
#define VC1_TOKEN_WIDTH                                                       9
#define VC1_TOKEN_SHIFT                                                      10
#define VC1_TOKEN_MASK                                               0x0007fc00
#define VC1_TOKEN_RD(src)                            (((src) & 0x0007fc00)>>10)
#define VC1_TOKEN_WR(src)                       (((u32)(src)<<10) & 0x0007fc00)
#define VC1_TOKEN_SET(dst,src) \
                      (((dst) & ~0x0007fc00) | (((u32)(src)<<10) & 0x0007fc00))
/*	 Fields VC0_TOKEN	 */
#define VC0_TOKEN_WIDTH                                                       9
#define VC0_TOKEN_SHIFT                                                       0
#define VC0_TOKEN_MASK                                               0x000001ff
#define VC0_TOKEN_RD(src)                                (((src) & 0x000001ff))
#define VC0_TOKEN_WR(src)                           (((u32)(src)) & 0x000001ff)
#define VC0_TOKEN_SET(dst,src) \
                          (((dst) & ~0x000001ff) | (((u32)(src)) & 0x000001ff))

/*	Register IOBBACFG	*/ 
/*	 Fields MAXREQ	 */
#define MAXREQ_F1_WIDTH                                                       2
#define MAXREQ_F1_SHIFT                                                       0
#define MAXREQ_F1_MASK                                               0x00000003
#define MAXREQ_F1_RD(src)                                (((src) & 0x00000003))
#define MAXREQ_F1_WR(src)                           (((u32)(src)) & 0x00000003)
#define MAXREQ_F1_SET(dst,src) \
                          (((dst) & ~0x00000003) | (((u32)(src)) & 0x00000003))

/*	Register IOBREMAPRNG0	*/ 
/*	 Fields BAR_RANGE	 */
#define BAR_RANGE0_WIDTH                                                     10
#define BAR_RANGE0_SHIFT                                                      1
#define BAR_RANGE0_MASK                                              0x000007fe
#define BAR_RANGE0_RD(src)                            (((src) & 0x000007fe)>>1)
#define BAR_RANGE0_WR(src)                       (((u32)(src)<<1) & 0x000007fe)
#define BAR_RANGE0_SET(dst,src) \
                       (((dst) & ~0x000007fe) | (((u32)(src)<<1) & 0x000007fe))
/*	 Fields REMAP_EN	 */
#define REMAP_EN0_WIDTH                                                       1
#define REMAP_EN0_SHIFT                                                       0
#define REMAP_EN0_MASK                                               0x00000001
#define REMAP_EN0_RD(src)                                (((src) & 0x00000001))
#define REMAP_EN0_WR(src)                           (((u32)(src)) & 0x00000001)
#define REMAP_EN0_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBREMAPRNG1	*/ 
/*	 Fields BAR_RANGE	 */
#define BAR_RANGE1_WIDTH                                                     10
#define BAR_RANGE1_SHIFT                                                      1
#define BAR_RANGE1_MASK                                              0x000007fe
#define BAR_RANGE1_RD(src)                            (((src) & 0x000007fe)>>1)
#define BAR_RANGE1_WR(src)                       (((u32)(src)<<1) & 0x000007fe)
#define BAR_RANGE1_SET(dst,src) \
                       (((dst) & ~0x000007fe) | (((u32)(src)<<1) & 0x000007fe))
/*	 Fields REMAP_EN	 */
#define REMAP_EN1_WIDTH                                                       1
#define REMAP_EN1_SHIFT                                                       0
#define REMAP_EN1_MASK                                               0x00000001
#define REMAP_EN1_RD(src)                                (((src) & 0x00000001))
#define REMAP_EN1_WR(src)                           (((u32)(src)) & 0x00000001)
#define REMAP_EN1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBREMAPRNG2	*/ 
/*	 Fields BAR_RANGE	 */
#define BAR_RANGE2_WIDTH                                                     10
#define BAR_RANGE2_SHIFT                                                      1
#define BAR_RANGE2_MASK                                              0x000007fe
#define BAR_RANGE2_RD(src)                            (((src) & 0x000007fe)>>1)
#define BAR_RANGE2_WR(src)                       (((u32)(src)<<1) & 0x000007fe)
#define BAR_RANGE2_SET(dst,src) \
                       (((dst) & ~0x000007fe) | (((u32)(src)<<1) & 0x000007fe))
/*	 Fields REMAP_EN	 */
#define REMAP_EN2_WIDTH                                                       1
#define REMAP_EN2_SHIFT                                                       0
#define REMAP_EN2_MASK                                               0x00000001
#define REMAP_EN2_RD(src)                                (((src) & 0x00000001))
#define REMAP_EN2_WR(src)                           (((u32)(src)) & 0x00000001)
#define REMAP_EN2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBREMAPRNG3	*/ 
/*	 Fields BAR_RANGE	 */
#define BAR_RANGE3_WIDTH                                                     10
#define BAR_RANGE3_SHIFT                                                      1
#define BAR_RANGE3_MASK                                              0x000007fe
#define BAR_RANGE3_RD(src)                            (((src) & 0x000007fe)>>1)
#define BAR_RANGE3_WR(src)                       (((u32)(src)<<1) & 0x000007fe)
#define BAR_RANGE3_SET(dst,src) \
                       (((dst) & ~0x000007fe) | (((u32)(src)<<1) & 0x000007fe))
/*	 Fields REMAP_EN	 */
#define REMAP_EN3_WIDTH                                                       1
#define REMAP_EN3_SHIFT                                                       0
#define REMAP_EN3_MASK                                               0x00000001
#define REMAP_EN3_RD(src)                                (((src) & 0x00000001))
#define REMAP_EN3_WR(src)                           (((u32)(src)) & 0x00000001)
#define REMAP_EN3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBREMAPRNG4	*/ 
/*	 Fields BAR_RANGE	 */
#define BAR_RANGE4_WIDTH                                                     10
#define BAR_RANGE4_SHIFT                                                      1
#define BAR_RANGE4_MASK                                              0x000007fe
#define BAR_RANGE4_RD(src)                            (((src) & 0x000007fe)>>1)
#define BAR_RANGE4_WR(src)                       (((u32)(src)<<1) & 0x000007fe)
#define BAR_RANGE4_SET(dst,src) \
                       (((dst) & ~0x000007fe) | (((u32)(src)<<1) & 0x000007fe))
/*	 Fields REMAP_EN	 */
#define REMAP_EN4_WIDTH                                                       1
#define REMAP_EN4_SHIFT                                                       0
#define REMAP_EN4_MASK                                               0x00000001
#define REMAP_EN4_RD(src)                                (((src) & 0x00000001))
#define REMAP_EN4_WR(src)                           (((u32)(src)) & 0x00000001)
#define REMAP_EN4_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBREMAPRNG5	*/ 
/*	 Fields BAR_RANGE	 */
#define BAR_RANGE5_WIDTH                                                     10
#define BAR_RANGE5_SHIFT                                                      1
#define BAR_RANGE5_MASK                                              0x000007fe
#define BAR_RANGE5_RD(src)                            (((src) & 0x000007fe)>>1)
#define BAR_RANGE5_WR(src)                       (((u32)(src)<<1) & 0x000007fe)
#define BAR_RANGE5_SET(dst,src) \
                       (((dst) & ~0x000007fe) | (((u32)(src)<<1) & 0x000007fe))
/*	 Fields REMAP_EN	 */
#define REMAP_EN5_WIDTH                                                       1
#define REMAP_EN5_SHIFT                                                       0
#define REMAP_EN5_MASK                                               0x00000001
#define REMAP_EN5_RD(src)                                (((src) & 0x00000001))
#define REMAP_EN5_WR(src)                           (((u32)(src)) & 0x00000001)
#define REMAP_EN5_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBREMAPBAR0	*/ 
/*	 Fields BASE_ADDRESS	 */
#define BASE_ADDRESS0_WIDTH                                                  22
#define BASE_ADDRESS0_SHIFT                                                   0
#define BASE_ADDRESS0_MASK                                           0x003fffff
#define BASE_ADDRESS0_RD(src)                            (((src) & 0x003fffff))
#define BASE_ADDRESS0_WR(src)                       (((u32)(src)) & 0x003fffff)
#define BASE_ADDRESS0_SET(dst,src) \
                          (((dst) & ~0x003fffff) | (((u32)(src)) & 0x003fffff))

/*	Register IOBREMAPBAR1	*/ 
/*	 Fields BASE_ADDRESS	 */
#define BASE_ADDRESS1_WIDTH                                                  22
#define BASE_ADDRESS1_SHIFT                                                   0
#define BASE_ADDRESS1_MASK                                           0x003fffff
#define BASE_ADDRESS1_RD(src)                            (((src) & 0x003fffff))
#define BASE_ADDRESS1_WR(src)                       (((u32)(src)) & 0x003fffff)
#define BASE_ADDRESS1_SET(dst,src) \
                          (((dst) & ~0x003fffff) | (((u32)(src)) & 0x003fffff))

/*	Register IOBREMAPBAR2	*/ 
/*	 Fields BASE_ADDRESS	 */
#define BASE_ADDRESS2_WIDTH                                                  22
#define BASE_ADDRESS2_SHIFT                                                   0
#define BASE_ADDRESS2_MASK                                           0x003fffff
#define BASE_ADDRESS2_RD(src)                            (((src) & 0x003fffff))
#define BASE_ADDRESS2_WR(src)                       (((u32)(src)) & 0x003fffff)
#define BASE_ADDRESS2_SET(dst,src) \
                          (((dst) & ~0x003fffff) | (((u32)(src)) & 0x003fffff))

/*	Register IOBREMAPBAR3	*/ 
/*	 Fields BASE_ADDRESS	 */
#define BASE_ADDRESS3_WIDTH                                                  22
#define BASE_ADDRESS3_SHIFT                                                   0
#define BASE_ADDRESS3_MASK                                           0x003fffff
#define BASE_ADDRESS3_RD(src)                            (((src) & 0x003fffff))
#define BASE_ADDRESS3_WR(src)                       (((u32)(src)) & 0x003fffff)
#define BASE_ADDRESS3_SET(dst,src) \
                          (((dst) & ~0x003fffff) | (((u32)(src)) & 0x003fffff))

/*	Register IOBREMAPBAR4	*/ 
/*	 Fields BASE_ADDRESS	 */
#define BASE_ADDRESS4_WIDTH                                                  22
#define BASE_ADDRESS4_SHIFT                                                   0
#define BASE_ADDRESS4_MASK                                           0x003fffff
#define BASE_ADDRESS4_RD(src)                            (((src) & 0x003fffff))
#define BASE_ADDRESS4_WR(src)                       (((u32)(src)) & 0x003fffff)
#define BASE_ADDRESS4_SET(dst,src) \
                          (((dst) & ~0x003fffff) | (((u32)(src)) & 0x003fffff))

/*	Register IOBREMAPBAR5	*/ 
/*	 Fields BASE_ADDRESS	 */
#define BASE_ADDRESS5_WIDTH                                                  22
#define BASE_ADDRESS5_SHIFT                                                   0
#define BASE_ADDRESS5_MASK                                           0x003fffff
#define BASE_ADDRESS5_RD(src)                            (((src) & 0x003fffff))
#define BASE_ADDRESS5_WR(src)                       (((u32)(src)) & 0x003fffff)
#define BASE_ADDRESS5_SET(dst,src) \
                          (((dst) & ~0x003fffff) | (((u32)(src)) & 0x003fffff))

/*	Register IOBREMAPTAR5	*/ 
/*	 Fields TRANS_ADDRESS	 */
#define TRANS_ADDRESS5_WIDTH                                                 22
#define TRANS_ADDRESS5_SHIFT                                                  0
#define TRANS_ADDRESS5_MASK                                          0x003fffff
#define TRANS_ADDRESS5_RD(src)                           (((src) & 0x003fffff))
#define TRANS_ADDRESS5_WR(src)                      (((u32)(src)) & 0x003fffff)
#define TRANS_ADDRESS5_SET(dst,src) \
                          (((dst) & ~0x003fffff) | (((u32)(src)) & 0x003fffff))

/*	Register IOBTRACECFG	*/ 
/*	 Fields PADDRDBG31	 */
#define PADDRDBG31_WIDTH                                                      1
#define PADDRDBG31_SHIFT                                                      1
#define PADDRDBG31_MASK                                              0x00000002
#define PADDRDBG31_RD(src)                            (((src) & 0x00000002)>>1)
#define PADDRDBG31_WR(src)                       (((u32)(src)<<1) & 0x00000002)
#define PADDRDBG31_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields DISABLE_ATB_BURST	 */
#define DISABLE_ATB_BURST_WIDTH                                               1
#define DISABLE_ATB_BURST_SHIFT                                               0
#define DISABLE_ATB_BURST_MASK                                       0x00000001
#define DISABLE_ATB_BURST_RD(src)                        (((src) & 0x00000001))
#define DISABLE_ATB_BURST_WR(src)                   (((u32)(src)) & 0x00000001)
#define DISABLE_ATB_BURST_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG00	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB0_WIDTH                                                    2
#define REGSPEC_RMB0_SHIFT                                                    6
#define REGSPEC_RMB0_MASK                                            0x000000c0
#define REGSPEC_RMB0_RD(src)                          (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB0_WR(src)                     (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB0_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB0_WIDTH                                                   1
#define REGSPEC_RMEB0_SHIFT                                                   4
#define REGSPEC_RMEB0_MASK                                           0x00000010
#define REGSPEC_RMEB0_RD(src)                         (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB0_WR(src)                    (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB0_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA0_WIDTH                                                    2
#define REGSPEC_RMA0_SHIFT                                                    2
#define REGSPEC_RMA0_MASK                                            0x0000000c
#define REGSPEC_RMA0_RD(src)                          (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA0_WR(src)                     (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA0_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA0_WIDTH                                                   1
#define REGSPEC_RMEA0_SHIFT                                                   0
#define REGSPEC_RMEA0_MASK                                           0x00000001
#define REGSPEC_RMEA0_RD(src)                            (((src) & 0x00000001))
#define REGSPEC_RMEA0_WR(src)                       (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA0_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG01	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB1_WIDTH                                                    2
#define REGSPEC_RMB1_SHIFT                                                    6
#define REGSPEC_RMB1_MASK                                            0x000000c0
#define REGSPEC_RMB1_RD(src)                          (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB1_WR(src)                     (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB1_WIDTH                                                   1
#define REGSPEC_RMEB1_SHIFT                                                   4
#define REGSPEC_RMEB1_MASK                                           0x00000010
#define REGSPEC_RMEB1_RD(src)                         (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB1_WR(src)                    (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA1_WIDTH                                                    2
#define REGSPEC_RMA1_SHIFT                                                    2
#define REGSPEC_RMA1_MASK                                            0x0000000c
#define REGSPEC_RMA1_RD(src)                          (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA1_WR(src)                     (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA1_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA1_WIDTH                                                   1
#define REGSPEC_RMEA1_SHIFT                                                   0
#define REGSPEC_RMEA1_MASK                                           0x00000001
#define REGSPEC_RMEA1_RD(src)                            (((src) & 0x00000001))
#define REGSPEC_RMEA1_WR(src)                       (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG02	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB2_WIDTH                                                    2
#define REGSPEC_RMB2_SHIFT                                                    6
#define REGSPEC_RMB2_MASK                                            0x000000c0
#define REGSPEC_RMB2_RD(src)                          (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB2_WR(src)                     (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB2_WIDTH                                                   1
#define REGSPEC_RMEB2_SHIFT                                                   4
#define REGSPEC_RMEB2_MASK                                           0x00000010
#define REGSPEC_RMEB2_RD(src)                         (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB2_WR(src)                    (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB2_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA2_WIDTH                                                    2
#define REGSPEC_RMA2_SHIFT                                                    2
#define REGSPEC_RMA2_MASK                                            0x0000000c
#define REGSPEC_RMA2_RD(src)                          (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA2_WR(src)                     (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA2_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA2_WIDTH                                                   1
#define REGSPEC_RMEA2_SHIFT                                                   0
#define REGSPEC_RMEA2_MASK                                           0x00000001
#define REGSPEC_RMEA2_RD(src)                            (((src) & 0x00000001))
#define REGSPEC_RMEA2_WR(src)                       (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG03	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB3_WIDTH                                                    2
#define REGSPEC_RMB3_SHIFT                                                    6
#define REGSPEC_RMB3_MASK                                            0x000000c0
#define REGSPEC_RMB3_RD(src)                          (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB3_WR(src)                     (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB3_WIDTH                                                   1
#define REGSPEC_RMEB3_SHIFT                                                   4
#define REGSPEC_RMEB3_MASK                                           0x00000010
#define REGSPEC_RMEB3_RD(src)                         (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB3_WR(src)                    (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB3_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA3_WIDTH                                                    2
#define REGSPEC_RMA3_SHIFT                                                    2
#define REGSPEC_RMA3_MASK                                            0x0000000c
#define REGSPEC_RMA3_RD(src)                          (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA3_WR(src)                     (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA3_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA3_WIDTH                                                   1
#define REGSPEC_RMEA3_SHIFT                                                   0
#define REGSPEC_RMEA3_MASK                                           0x00000001
#define REGSPEC_RMEA3_RD(src)                            (((src) & 0x00000001))
#define REGSPEC_RMEA3_WR(src)                       (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG04	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB4_WIDTH                                                    2
#define REGSPEC_RMB4_SHIFT                                                    6
#define REGSPEC_RMB4_MASK                                            0x000000c0
#define REGSPEC_RMB4_RD(src)                          (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB4_WR(src)                     (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB4_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB4_WIDTH                                                   1
#define REGSPEC_RMEB4_SHIFT                                                   4
#define REGSPEC_RMEB4_MASK                                           0x00000010
#define REGSPEC_RMEB4_RD(src)                         (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB4_WR(src)                    (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB4_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA4_WIDTH                                                    2
#define REGSPEC_RMA4_SHIFT                                                    2
#define REGSPEC_RMA4_MASK                                            0x0000000c
#define REGSPEC_RMA4_RD(src)                          (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA4_WR(src)                     (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA4_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA4_WIDTH                                                   1
#define REGSPEC_RMEA4_SHIFT                                                   0
#define REGSPEC_RMEA4_MASK                                           0x00000001
#define REGSPEC_RMEA4_RD(src)                            (((src) & 0x00000001))
#define REGSPEC_RMEA4_WR(src)                       (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA4_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG05	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB5_WIDTH                                                    2
#define REGSPEC_RMB5_SHIFT                                                    6
#define REGSPEC_RMB5_MASK                                            0x000000c0
#define REGSPEC_RMB5_RD(src)                          (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB5_WR(src)                     (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB5_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB5_WIDTH                                                   1
#define REGSPEC_RMEB5_SHIFT                                                   4
#define REGSPEC_RMEB5_MASK                                           0x00000010
#define REGSPEC_RMEB5_RD(src)                         (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB5_WR(src)                    (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB5_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA5_WIDTH                                                    2
#define REGSPEC_RMA5_SHIFT                                                    2
#define REGSPEC_RMA5_MASK                                            0x0000000c
#define REGSPEC_RMA5_RD(src)                          (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA5_WR(src)                     (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA5_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA5_WIDTH                                                   1
#define REGSPEC_RMEA5_SHIFT                                                   0
#define REGSPEC_RMEA5_MASK                                           0x00000001
#define REGSPEC_RMEA5_RD(src)                            (((src) & 0x00000001))
#define REGSPEC_RMEA5_WR(src)                       (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA5_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG06	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB6_WIDTH                                                    2
#define REGSPEC_RMB6_SHIFT                                                    6
#define REGSPEC_RMB6_MASK                                            0x000000c0
#define REGSPEC_RMB6_RD(src)                          (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB6_WR(src)                     (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB6_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB6_WIDTH                                                   1
#define REGSPEC_RMEB6_SHIFT                                                   4
#define REGSPEC_RMEB6_MASK                                           0x00000010
#define REGSPEC_RMEB6_RD(src)                         (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB6_WR(src)                    (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB6_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA6_WIDTH                                                    2
#define REGSPEC_RMA6_SHIFT                                                    2
#define REGSPEC_RMA6_MASK                                            0x0000000c
#define REGSPEC_RMA6_RD(src)                          (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA6_WR(src)                     (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA6_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA6_WIDTH                                                   1
#define REGSPEC_RMEA6_SHIFT                                                   0
#define REGSPEC_RMEA6_MASK                                           0x00000001
#define REGSPEC_RMEA6_RD(src)                            (((src) & 0x00000001))
#define REGSPEC_RMEA6_WR(src)                       (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA6_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG07	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB7_WIDTH                                                    2
#define REGSPEC_RMB7_SHIFT                                                    6
#define REGSPEC_RMB7_MASK                                            0x000000c0
#define REGSPEC_RMB7_RD(src)                          (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB7_WR(src)                     (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB7_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB7_WIDTH                                                   1
#define REGSPEC_RMEB7_SHIFT                                                   4
#define REGSPEC_RMEB7_MASK                                           0x00000010
#define REGSPEC_RMEB7_RD(src)                         (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB7_WR(src)                    (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB7_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA7_WIDTH                                                    2
#define REGSPEC_RMA7_SHIFT                                                    2
#define REGSPEC_RMA7_MASK                                            0x0000000c
#define REGSPEC_RMA7_RD(src)                          (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA7_WR(src)                     (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA7_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA7_WIDTH                                                   1
#define REGSPEC_RMEA7_SHIFT                                                   0
#define REGSPEC_RMEA7_MASK                                           0x00000001
#define REGSPEC_RMEA7_RD(src)                            (((src) & 0x00000001))
#define REGSPEC_RMEA7_WR(src)                       (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA7_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG08	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB8_WIDTH                                                    2
#define REGSPEC_RMB8_SHIFT                                                    6
#define REGSPEC_RMB8_MASK                                            0x000000c0
#define REGSPEC_RMB8_RD(src)                          (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB8_WR(src)                     (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB8_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB8_WIDTH                                                   1
#define REGSPEC_RMEB8_SHIFT                                                   4
#define REGSPEC_RMEB8_MASK                                           0x00000010
#define REGSPEC_RMEB8_RD(src)                         (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB8_WR(src)                    (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB8_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA8_WIDTH                                                    2
#define REGSPEC_RMA8_SHIFT                                                    2
#define REGSPEC_RMA8_MASK                                            0x0000000c
#define REGSPEC_RMA8_RD(src)                          (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA8_WR(src)                     (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA8_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA8_WIDTH                                                   1
#define REGSPEC_RMEA8_SHIFT                                                   0
#define REGSPEC_RMEA8_MASK                                           0x00000001
#define REGSPEC_RMEA8_RD(src)                            (((src) & 0x00000001))
#define REGSPEC_RMEA8_WR(src)                       (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA8_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG09	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB9_WIDTH                                                    2
#define REGSPEC_RMB9_SHIFT                                                    6
#define REGSPEC_RMB9_MASK                                            0x000000c0
#define REGSPEC_RMB9_RD(src)                          (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB9_WR(src)                     (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB9_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB9_WIDTH                                                   1
#define REGSPEC_RMEB9_SHIFT                                                   4
#define REGSPEC_RMEB9_MASK                                           0x00000010
#define REGSPEC_RMEB9_RD(src)                         (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB9_WR(src)                    (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB9_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA9_WIDTH                                                    2
#define REGSPEC_RMA9_SHIFT                                                    2
#define REGSPEC_RMA9_MASK                                            0x0000000c
#define REGSPEC_RMA9_RD(src)                          (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA9_WR(src)                     (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA9_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA9_WIDTH                                                   1
#define REGSPEC_RMEA9_SHIFT                                                   0
#define REGSPEC_RMEA9_MASK                                           0x00000001
#define REGSPEC_RMEA9_RD(src)                            (((src) & 0x00000001))
#define REGSPEC_RMEA9_WR(src)                       (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA9_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG0A	*/ 
/*	 Fields RMB	 */
#define IOBMEMCFG0A_REGSPEC_RMB_WIDTH                                                     2
#define IOBMEMCFG0A_REGSPEC_RMB_SHIFT                                                     6
#define IOBMEMCFG0A_REGSPEC_RMB_MASK                                             0x000000c0
#define IOBMEMCFG0A_REGSPEC_RMB_RD(src)                           (((src) & 0x000000c0)>>6)
#define IOBMEMCFG0A_REGSPEC_RMB_WR(src)                      (((u32)(src)<<6) & 0x000000c0)
#define IOBMEMCFG0A_REGSPEC_RMB_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define IOBMEMCFG0A_REGSPEC_RMEB_WIDTH                                                    1
#define IOBMEMCFG0A_REGSPEC_RMEB_SHIFT                                                    4
#define IOBMEMCFG0A_REGSPEC_RMEB_MASK                                            0x00000010
#define IOBMEMCFG0A_REGSPEC_RMEB_RD(src)                          (((src) & 0x00000010)>>4)
#define IOBMEMCFG0A_REGSPEC_RMEB_WR(src)                     (((u32)(src)<<4) & 0x00000010)
#define IOBMEMCFG0A_REGSPEC_RMEB_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define IOBMEMCFG0A_REGSPEC_RMA_WIDTH                                                     2
#define IOBMEMCFG0A_REGSPEC_RMA_SHIFT                                                     2
#define IOBMEMCFG0A_REGSPEC_RMA_MASK                                             0x0000000c
#define IOBMEMCFG0A_REGSPEC_RMA_RD(src)                           (((src) & 0x0000000c)>>2)
#define IOBMEMCFG0A_REGSPEC_RMA_WR(src)                      (((u32)(src)<<2) & 0x0000000c)
#define IOBMEMCFG0A_REGSPEC_RMA_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define IOBMEMCFG0A_REGSPEC_RMEA_WIDTH                                                    1
#define IOBMEMCFG0A_REGSPEC_RMEA_SHIFT                                                    0
#define IOBMEMCFG0A_REGSPEC_RMEA_MASK                                            0x00000001
#define IOBMEMCFG0A_REGSPEC_RMEA_RD(src)                             (((src) & 0x00000001))
#define IOBMEMCFG0A_REGSPEC_RMEA_WR(src)                        (((u32)(src)) & 0x00000001)
#define IOBMEMCFG0A_REGSPEC_RMEA_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG0B	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F1_WIDTH                                                  2
#define REGSPEC_RMB_F1_SHIFT                                                  6
#define REGSPEC_RMB_F1_MASK                                          0x000000c0
#define REGSPEC_RMB_F1_RD(src)                        (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F1_WR(src)                   (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F1_WIDTH                                                 1
#define REGSPEC_RMEB_F1_SHIFT                                                 4
#define REGSPEC_RMEB_F1_MASK                                         0x00000010
#define REGSPEC_RMEB_F1_RD(src)                       (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F1_WR(src)                  (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F1_WIDTH                                                  2
#define REGSPEC_RMA_F1_SHIFT                                                  2
#define REGSPEC_RMA_F1_MASK                                          0x0000000c
#define REGSPEC_RMA_F1_RD(src)                        (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F1_WR(src)                   (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F1_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F1_WIDTH                                                 1
#define REGSPEC_RMEA_F1_SHIFT                                                 0
#define REGSPEC_RMEA_F1_MASK                                         0x00000001
#define REGSPEC_RMEA_F1_RD(src)                          (((src) & 0x00000001))
#define REGSPEC_RMEA_F1_WR(src)                     (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG0C	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F2_WIDTH                                                  2
#define REGSPEC_RMB_F2_SHIFT                                                  6
#define REGSPEC_RMB_F2_MASK                                          0x000000c0
#define REGSPEC_RMB_F2_RD(src)                        (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F2_WR(src)                   (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F2_WIDTH                                                 1
#define REGSPEC_RMEB_F2_SHIFT                                                 4
#define REGSPEC_RMEB_F2_MASK                                         0x00000010
#define REGSPEC_RMEB_F2_RD(src)                       (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F2_WR(src)                  (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F2_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F2_WIDTH                                                  2
#define REGSPEC_RMA_F2_SHIFT                                                  2
#define REGSPEC_RMA_F2_MASK                                          0x0000000c
#define REGSPEC_RMA_F2_RD(src)                        (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F2_WR(src)                   (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F2_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F2_WIDTH                                                 1
#define REGSPEC_RMEA_F2_SHIFT                                                 0
#define REGSPEC_RMEA_F2_MASK                                         0x00000001
#define REGSPEC_RMEA_F2_RD(src)                          (((src) & 0x00000001))
#define REGSPEC_RMEA_F2_WR(src)                     (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG0D	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F3_WIDTH                                                  2
#define REGSPEC_RMB_F3_SHIFT                                                  6
#define REGSPEC_RMB_F3_MASK                                          0x000000c0
#define REGSPEC_RMB_F3_RD(src)                        (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F3_WR(src)                   (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F3_WIDTH                                                 1
#define REGSPEC_RMEB_F3_SHIFT                                                 4
#define REGSPEC_RMEB_F3_MASK                                         0x00000010
#define REGSPEC_RMEB_F3_RD(src)                       (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F3_WR(src)                  (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F3_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F3_WIDTH                                                  2
#define REGSPEC_RMA_F3_SHIFT                                                  2
#define REGSPEC_RMA_F3_MASK                                          0x0000000c
#define REGSPEC_RMA_F3_RD(src)                        (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F3_WR(src)                   (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F3_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F3_WIDTH                                                 1
#define REGSPEC_RMEA_F3_SHIFT                                                 0
#define REGSPEC_RMEA_F3_MASK                                         0x00000001
#define REGSPEC_RMEA_F3_RD(src)                          (((src) & 0x00000001))
#define REGSPEC_RMEA_F3_WR(src)                     (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG0E	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F4_WIDTH                                                  2
#define REGSPEC_RMB_F4_SHIFT                                                  6
#define REGSPEC_RMB_F4_MASK                                          0x000000c0
#define REGSPEC_RMB_F4_RD(src)                        (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F4_WR(src)                   (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F4_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F4_WIDTH                                                 1
#define REGSPEC_RMEB_F4_SHIFT                                                 4
#define REGSPEC_RMEB_F4_MASK                                         0x00000010
#define REGSPEC_RMEB_F4_RD(src)                       (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F4_WR(src)                  (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F4_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F4_WIDTH                                                  2
#define REGSPEC_RMA_F4_SHIFT                                                  2
#define REGSPEC_RMA_F4_MASK                                          0x0000000c
#define REGSPEC_RMA_F4_RD(src)                        (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F4_WR(src)                   (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F4_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F4_WIDTH                                                 1
#define REGSPEC_RMEA_F4_SHIFT                                                 0
#define REGSPEC_RMEA_F4_MASK                                         0x00000001
#define REGSPEC_RMEA_F4_RD(src)                          (((src) & 0x00000001))
#define REGSPEC_RMEA_F4_WR(src)                     (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F4_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG0F	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F5_WIDTH                                                  2
#define REGSPEC_RMB_F5_SHIFT                                                  6
#define REGSPEC_RMB_F5_MASK                                          0x000000c0
#define REGSPEC_RMB_F5_RD(src)                        (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F5_WR(src)                   (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F5_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F5_WIDTH                                                 1
#define REGSPEC_RMEB_F5_SHIFT                                                 4
#define REGSPEC_RMEB_F5_MASK                                         0x00000010
#define REGSPEC_RMEB_F5_RD(src)                       (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F5_WR(src)                  (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F5_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F5_WIDTH                                                  2
#define REGSPEC_RMA_F5_SHIFT                                                  2
#define REGSPEC_RMA_F5_MASK                                          0x0000000c
#define REGSPEC_RMA_F5_RD(src)                        (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F5_WR(src)                   (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F5_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F5_WIDTH                                                 1
#define REGSPEC_RMEA_F5_SHIFT                                                 0
#define REGSPEC_RMEA_F5_MASK                                         0x00000001
#define REGSPEC_RMEA_F5_RD(src)                          (((src) & 0x00000001))
#define REGSPEC_RMEA_F5_WR(src)                     (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F5_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG10	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB0_F1_WIDTH                                                 2
#define REGSPEC_RMB0_F1_SHIFT                                                 6
#define REGSPEC_RMB0_F1_MASK                                         0x000000c0
#define REGSPEC_RMB0_F1_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB0_F1_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB0_F1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB0_F1_WIDTH                                                1
#define REGSPEC_RMEB0_F1_SHIFT                                                4
#define REGSPEC_RMEB0_F1_MASK                                        0x00000010
#define REGSPEC_RMEB0_F1_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB0_F1_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB0_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA0_F1_WIDTH                                                 2
#define REGSPEC_RMA0_F1_SHIFT                                                 2
#define REGSPEC_RMA0_F1_MASK                                         0x0000000c
#define REGSPEC_RMA0_F1_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA0_F1_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA0_F1_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA0_F1_WIDTH                                                1
#define REGSPEC_RMEA0_F1_SHIFT                                                0
#define REGSPEC_RMEA0_F1_MASK                                        0x00000001
#define REGSPEC_RMEA0_F1_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA0_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA0_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG11	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB1_F1_WIDTH                                                 2
#define REGSPEC_RMB1_F1_SHIFT                                                 6
#define REGSPEC_RMB1_F1_MASK                                         0x000000c0
#define REGSPEC_RMB1_F1_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB1_F1_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB1_F1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB1_F1_WIDTH                                                1
#define REGSPEC_RMEB1_F1_SHIFT                                                4
#define REGSPEC_RMEB1_F1_MASK                                        0x00000010
#define REGSPEC_RMEB1_F1_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB1_F1_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB1_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA1_F1_WIDTH                                                 2
#define REGSPEC_RMA1_F1_SHIFT                                                 2
#define REGSPEC_RMA1_F1_MASK                                         0x0000000c
#define REGSPEC_RMA1_F1_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA1_F1_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA1_F1_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA1_F1_WIDTH                                                1
#define REGSPEC_RMEA1_F1_SHIFT                                                0
#define REGSPEC_RMEA1_F1_MASK                                        0x00000001
#define REGSPEC_RMEA1_F1_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA1_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA1_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG12	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB2_F1_WIDTH                                                 2
#define REGSPEC_RMB2_F1_SHIFT                                                 6
#define REGSPEC_RMB2_F1_MASK                                         0x000000c0
#define REGSPEC_RMB2_F1_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB2_F1_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB2_F1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB2_F1_WIDTH                                                1
#define REGSPEC_RMEB2_F1_SHIFT                                                4
#define REGSPEC_RMEB2_F1_MASK                                        0x00000010
#define REGSPEC_RMEB2_F1_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB2_F1_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB2_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA2_F1_WIDTH                                                 2
#define REGSPEC_RMA2_F1_SHIFT                                                 2
#define REGSPEC_RMA2_F1_MASK                                         0x0000000c
#define REGSPEC_RMA2_F1_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA2_F1_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA2_F1_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA2_F1_WIDTH                                                1
#define REGSPEC_RMEA2_F1_SHIFT                                                0
#define REGSPEC_RMEA2_F1_MASK                                        0x00000001
#define REGSPEC_RMEA2_F1_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA2_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA2_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG13	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB3_F1_WIDTH                                                 2
#define REGSPEC_RMB3_F1_SHIFT                                                 6
#define REGSPEC_RMB3_F1_MASK                                         0x000000c0
#define REGSPEC_RMB3_F1_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB3_F1_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB3_F1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB3_F1_WIDTH                                                1
#define REGSPEC_RMEB3_F1_SHIFT                                                4
#define REGSPEC_RMEB3_F1_MASK                                        0x00000010
#define REGSPEC_RMEB3_F1_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB3_F1_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB3_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA3_F1_WIDTH                                                 2
#define REGSPEC_RMA3_F1_SHIFT                                                 2
#define REGSPEC_RMA3_F1_MASK                                         0x0000000c
#define REGSPEC_RMA3_F1_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA3_F1_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA3_F1_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA3_F1_WIDTH                                                1
#define REGSPEC_RMEA3_F1_SHIFT                                                0
#define REGSPEC_RMEA3_F1_MASK                                        0x00000001
#define REGSPEC_RMEA3_F1_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA3_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA3_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG14	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB4_F1_WIDTH                                                 2
#define REGSPEC_RMB4_F1_SHIFT                                                 6
#define REGSPEC_RMB4_F1_MASK                                         0x000000c0
#define REGSPEC_RMB4_F1_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB4_F1_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB4_F1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB4_F1_WIDTH                                                1
#define REGSPEC_RMEB4_F1_SHIFT                                                4
#define REGSPEC_RMEB4_F1_MASK                                        0x00000010
#define REGSPEC_RMEB4_F1_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB4_F1_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB4_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA4_F1_WIDTH                                                 2
#define REGSPEC_RMA4_F1_SHIFT                                                 2
#define REGSPEC_RMA4_F1_MASK                                         0x0000000c
#define REGSPEC_RMA4_F1_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA4_F1_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA4_F1_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA4_F1_WIDTH                                                1
#define REGSPEC_RMEA4_F1_SHIFT                                                0
#define REGSPEC_RMEA4_F1_MASK                                        0x00000001
#define REGSPEC_RMEA4_F1_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA4_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA4_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG15	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB5_F1_WIDTH                                                 2
#define REGSPEC_RMB5_F1_SHIFT                                                 6
#define REGSPEC_RMB5_F1_MASK                                         0x000000c0
#define REGSPEC_RMB5_F1_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB5_F1_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB5_F1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB5_F1_WIDTH                                                1
#define REGSPEC_RMEB5_F1_SHIFT                                                4
#define REGSPEC_RMEB5_F1_MASK                                        0x00000010
#define REGSPEC_RMEB5_F1_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB5_F1_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB5_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA5_F1_WIDTH                                                 2
#define REGSPEC_RMA5_F1_SHIFT                                                 2
#define REGSPEC_RMA5_F1_MASK                                         0x0000000c
#define REGSPEC_RMA5_F1_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA5_F1_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA5_F1_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA5_F1_WIDTH                                                1
#define REGSPEC_RMEA5_F1_SHIFT                                                0
#define REGSPEC_RMEA5_F1_MASK                                        0x00000001
#define REGSPEC_RMEA5_F1_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA5_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA5_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG16	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB6_F1_WIDTH                                                 2
#define REGSPEC_RMB6_F1_SHIFT                                                 6
#define REGSPEC_RMB6_F1_MASK                                         0x000000c0
#define REGSPEC_RMB6_F1_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB6_F1_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB6_F1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB6_F1_WIDTH                                                1
#define REGSPEC_RMEB6_F1_SHIFT                                                4
#define REGSPEC_RMEB6_F1_MASK                                        0x00000010
#define REGSPEC_RMEB6_F1_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB6_F1_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB6_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA6_F1_WIDTH                                                 2
#define REGSPEC_RMA6_F1_SHIFT                                                 2
#define REGSPEC_RMA6_F1_MASK                                         0x0000000c
#define REGSPEC_RMA6_F1_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA6_F1_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA6_F1_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA6_F1_WIDTH                                                1
#define REGSPEC_RMEA6_F1_SHIFT                                                0
#define REGSPEC_RMEA6_F1_MASK                                        0x00000001
#define REGSPEC_RMEA6_F1_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA6_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA6_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG17	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB7_F1_WIDTH                                                 2
#define REGSPEC_RMB7_F1_SHIFT                                                 6
#define REGSPEC_RMB7_F1_MASK                                         0x000000c0
#define REGSPEC_RMB7_F1_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB7_F1_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB7_F1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB7_F1_WIDTH                                                1
#define REGSPEC_RMEB7_F1_SHIFT                                                4
#define REGSPEC_RMEB7_F1_MASK                                        0x00000010
#define REGSPEC_RMEB7_F1_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB7_F1_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB7_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA7_F1_WIDTH                                                 2
#define REGSPEC_RMA7_F1_SHIFT                                                 2
#define REGSPEC_RMA7_F1_MASK                                         0x0000000c
#define REGSPEC_RMA7_F1_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA7_F1_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA7_F1_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA7_F1_WIDTH                                                1
#define REGSPEC_RMEA7_F1_SHIFT                                                0
#define REGSPEC_RMEA7_F1_MASK                                        0x00000001
#define REGSPEC_RMEA7_F1_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA7_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA7_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG18	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB8_F1_WIDTH                                                 2
#define REGSPEC_RMB8_F1_SHIFT                                                 6
#define REGSPEC_RMB8_F1_MASK                                         0x000000c0
#define REGSPEC_RMB8_F1_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB8_F1_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB8_F1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB8_F1_WIDTH                                                1
#define REGSPEC_RMEB8_F1_SHIFT                                                4
#define REGSPEC_RMEB8_F1_MASK                                        0x00000010
#define REGSPEC_RMEB8_F1_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB8_F1_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB8_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA8_F1_WIDTH                                                 2
#define REGSPEC_RMA8_F1_SHIFT                                                 2
#define REGSPEC_RMA8_F1_MASK                                         0x0000000c
#define REGSPEC_RMA8_F1_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA8_F1_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA8_F1_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA8_F1_WIDTH                                                1
#define REGSPEC_RMEA8_F1_SHIFT                                                0
#define REGSPEC_RMEA8_F1_MASK                                        0x00000001
#define REGSPEC_RMEA8_F1_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA8_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA8_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG19	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB9_F1_WIDTH                                                 2
#define REGSPEC_RMB9_F1_SHIFT                                                 6
#define REGSPEC_RMB9_F1_MASK                                         0x000000c0
#define REGSPEC_RMB9_F1_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB9_F1_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB9_F1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB9_F1_WIDTH                                                1
#define REGSPEC_RMEB9_F1_SHIFT                                                4
#define REGSPEC_RMEB9_F1_MASK                                        0x00000010
#define REGSPEC_RMEB9_F1_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB9_F1_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB9_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA9_F1_WIDTH                                                 2
#define REGSPEC_RMA9_F1_SHIFT                                                 2
#define REGSPEC_RMA9_F1_MASK                                         0x0000000c
#define REGSPEC_RMA9_F1_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA9_F1_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA9_F1_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA9_F1_WIDTH                                                1
#define REGSPEC_RMEA9_F1_SHIFT                                                0
#define REGSPEC_RMEA9_F1_MASK                                        0x00000001
#define REGSPEC_RMEA9_F1_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA9_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA9_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG1A	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F6_WIDTH                                                  2
#define REGSPEC_RMB_F6_SHIFT                                                  6
#define REGSPEC_RMB_F6_MASK                                          0x000000c0
#define REGSPEC_RMB_F6_RD(src)                        (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F6_WR(src)                   (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F6_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F6_WIDTH                                                 1
#define REGSPEC_RMEB_F6_SHIFT                                                 4
#define REGSPEC_RMEB_F6_MASK                                         0x00000010
#define REGSPEC_RMEB_F6_RD(src)                       (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F6_WR(src)                  (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F6_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F6_WIDTH                                                  2
#define REGSPEC_RMA_F6_SHIFT                                                  2
#define REGSPEC_RMA_F6_MASK                                          0x0000000c
#define REGSPEC_RMA_F6_RD(src)                        (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F6_WR(src)                   (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F6_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F6_WIDTH                                                 1
#define REGSPEC_RMEA_F6_SHIFT                                                 0
#define REGSPEC_RMEA_F6_MASK                                         0x00000001
#define REGSPEC_RMEA_F6_RD(src)                          (((src) & 0x00000001))
#define REGSPEC_RMEA_F6_WR(src)                     (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F6_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG1B	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F7_WIDTH                                                  2
#define REGSPEC_RMB_F7_SHIFT                                                  6
#define REGSPEC_RMB_F7_MASK                                          0x000000c0
#define REGSPEC_RMB_F7_RD(src)                        (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F7_WR(src)                   (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F7_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F7_WIDTH                                                 1
#define REGSPEC_RMEB_F7_SHIFT                                                 4
#define REGSPEC_RMEB_F7_MASK                                         0x00000010
#define REGSPEC_RMEB_F7_RD(src)                       (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F7_WR(src)                  (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F7_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F7_WIDTH                                                  2
#define REGSPEC_RMA_F7_SHIFT                                                  2
#define REGSPEC_RMA_F7_MASK                                          0x0000000c
#define REGSPEC_RMA_F7_RD(src)                        (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F7_WR(src)                   (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F7_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F7_WIDTH                                                 1
#define REGSPEC_RMEA_F7_SHIFT                                                 0
#define REGSPEC_RMEA_F7_MASK                                         0x00000001
#define REGSPEC_RMEA_F7_RD(src)                          (((src) & 0x00000001))
#define REGSPEC_RMEA_F7_WR(src)                     (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F7_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG1C	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F8_WIDTH                                                  2
#define REGSPEC_RMB_F8_SHIFT                                                  6
#define REGSPEC_RMB_F8_MASK                                          0x000000c0
#define REGSPEC_RMB_F8_RD(src)                        (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F8_WR(src)                   (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F8_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F8_WIDTH                                                 1
#define REGSPEC_RMEB_F8_SHIFT                                                 4
#define REGSPEC_RMEB_F8_MASK                                         0x00000010
#define REGSPEC_RMEB_F8_RD(src)                       (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F8_WR(src)                  (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F8_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F8_WIDTH                                                  2
#define REGSPEC_RMA_F8_SHIFT                                                  2
#define REGSPEC_RMA_F8_MASK                                          0x0000000c
#define REGSPEC_RMA_F8_RD(src)                        (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F8_WR(src)                   (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F8_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F8_WIDTH                                                 1
#define REGSPEC_RMEA_F8_SHIFT                                                 0
#define REGSPEC_RMEA_F8_MASK                                         0x00000001
#define REGSPEC_RMEA_F8_RD(src)                          (((src) & 0x00000001))
#define REGSPEC_RMEA_F8_WR(src)                     (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F8_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG1D	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F9_WIDTH                                                  2
#define REGSPEC_RMB_F9_SHIFT                                                  6
#define REGSPEC_RMB_F9_MASK                                          0x000000c0
#define REGSPEC_RMB_F9_RD(src)                        (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F9_WR(src)                   (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F9_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F9_WIDTH                                                 1
#define REGSPEC_RMEB_F9_SHIFT                                                 4
#define REGSPEC_RMEB_F9_MASK                                         0x00000010
#define REGSPEC_RMEB_F9_RD(src)                       (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F9_WR(src)                  (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F9_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F9_WIDTH                                                  2
#define REGSPEC_RMA_F9_SHIFT                                                  2
#define REGSPEC_RMA_F9_MASK                                          0x0000000c
#define REGSPEC_RMA_F9_RD(src)                        (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F9_WR(src)                   (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F9_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F9_WIDTH                                                 1
#define REGSPEC_RMEA_F9_SHIFT                                                 0
#define REGSPEC_RMEA_F9_MASK                                         0x00000001
#define REGSPEC_RMEA_F9_RD(src)                          (((src) & 0x00000001))
#define REGSPEC_RMEA_F9_WR(src)                     (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F9_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG1E	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F10_WIDTH                                                 2
#define REGSPEC_RMB_F10_SHIFT                                                 6
#define REGSPEC_RMB_F10_MASK                                         0x000000c0
#define REGSPEC_RMB_F10_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F10_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F10_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F10_WIDTH                                                1
#define REGSPEC_RMEB_F10_SHIFT                                                4
#define REGSPEC_RMEB_F10_MASK                                        0x00000010
#define REGSPEC_RMEB_F10_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F10_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F10_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F10_WIDTH                                                 2
#define REGSPEC_RMA_F10_SHIFT                                                 2
#define REGSPEC_RMA_F10_MASK                                         0x0000000c
#define REGSPEC_RMA_F10_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F10_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F10_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F10_WIDTH                                                1
#define REGSPEC_RMEA_F10_SHIFT                                                0
#define REGSPEC_RMEA_F10_MASK                                        0x00000001
#define REGSPEC_RMEA_F10_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F10_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F10_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG1F	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F11_WIDTH                                                 2
#define REGSPEC_RMB_F11_SHIFT                                                 6
#define REGSPEC_RMB_F11_MASK                                         0x000000c0
#define REGSPEC_RMB_F11_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F11_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F11_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F11_WIDTH                                                1
#define REGSPEC_RMEB_F11_SHIFT                                                4
#define REGSPEC_RMEB_F11_MASK                                        0x00000010
#define REGSPEC_RMEB_F11_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F11_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F11_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F11_WIDTH                                                 2
#define REGSPEC_RMA_F11_SHIFT                                                 2
#define REGSPEC_RMA_F11_MASK                                         0x0000000c
#define REGSPEC_RMA_F11_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F11_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F11_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F11_WIDTH                                                1
#define REGSPEC_RMEA_F11_SHIFT                                                0
#define REGSPEC_RMEA_F11_MASK                                        0x00000001
#define REGSPEC_RMEA_F11_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F11_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F11_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG20	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB0_F2_WIDTH                                                 2
#define REGSPEC_RMB0_F2_SHIFT                                                 6
#define REGSPEC_RMB0_F2_MASK                                         0x000000c0
#define REGSPEC_RMB0_F2_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB0_F2_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB0_F2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB0_F2_WIDTH                                                1
#define REGSPEC_RMEB0_F2_SHIFT                                                4
#define REGSPEC_RMEB0_F2_MASK                                        0x00000010
#define REGSPEC_RMEB0_F2_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB0_F2_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB0_F2_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA0_F2_WIDTH                                                 2
#define REGSPEC_RMA0_F2_SHIFT                                                 2
#define REGSPEC_RMA0_F2_MASK                                         0x0000000c
#define REGSPEC_RMA0_F2_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA0_F2_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA0_F2_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA0_F2_WIDTH                                                1
#define REGSPEC_RMEA0_F2_SHIFT                                                0
#define REGSPEC_RMEA0_F2_MASK                                        0x00000001
#define REGSPEC_RMEA0_F2_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA0_F2_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA0_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG21	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB1_F2_WIDTH                                                 2
#define REGSPEC_RMB1_F2_SHIFT                                                 6
#define REGSPEC_RMB1_F2_MASK                                         0x000000c0
#define REGSPEC_RMB1_F2_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB1_F2_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB1_F2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB1_F2_WIDTH                                                1
#define REGSPEC_RMEB1_F2_SHIFT                                                4
#define REGSPEC_RMEB1_F2_MASK                                        0x00000010
#define REGSPEC_RMEB1_F2_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB1_F2_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB1_F2_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA1_F2_WIDTH                                                 2
#define REGSPEC_RMA1_F2_SHIFT                                                 2
#define REGSPEC_RMA1_F2_MASK                                         0x0000000c
#define REGSPEC_RMA1_F2_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA1_F2_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA1_F2_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA1_F2_WIDTH                                                1
#define REGSPEC_RMEA1_F2_SHIFT                                                0
#define REGSPEC_RMEA1_F2_MASK                                        0x00000001
#define REGSPEC_RMEA1_F2_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA1_F2_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA1_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG22	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB2_F2_WIDTH                                                 2
#define REGSPEC_RMB2_F2_SHIFT                                                 6
#define REGSPEC_RMB2_F2_MASK                                         0x000000c0
#define REGSPEC_RMB2_F2_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB2_F2_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB2_F2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB2_F2_WIDTH                                                1
#define REGSPEC_RMEB2_F2_SHIFT                                                4
#define REGSPEC_RMEB2_F2_MASK                                        0x00000010
#define REGSPEC_RMEB2_F2_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB2_F2_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB2_F2_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA2_F2_WIDTH                                                 2
#define REGSPEC_RMA2_F2_SHIFT                                                 2
#define REGSPEC_RMA2_F2_MASK                                         0x0000000c
#define REGSPEC_RMA2_F2_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA2_F2_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA2_F2_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA2_F2_WIDTH                                                1
#define REGSPEC_RMEA2_F2_SHIFT                                                0
#define REGSPEC_RMEA2_F2_MASK                                        0x00000001
#define REGSPEC_RMEA2_F2_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA2_F2_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA2_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG23	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB3_F2_WIDTH                                                 2
#define REGSPEC_RMB3_F2_SHIFT                                                 6
#define REGSPEC_RMB3_F2_MASK                                         0x000000c0
#define REGSPEC_RMB3_F2_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB3_F2_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB3_F2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB3_F2_WIDTH                                                1
#define REGSPEC_RMEB3_F2_SHIFT                                                4
#define REGSPEC_RMEB3_F2_MASK                                        0x00000010
#define REGSPEC_RMEB3_F2_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB3_F2_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB3_F2_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA3_F2_WIDTH                                                 2
#define REGSPEC_RMA3_F2_SHIFT                                                 2
#define REGSPEC_RMA3_F2_MASK                                         0x0000000c
#define REGSPEC_RMA3_F2_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA3_F2_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA3_F2_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA3_F2_WIDTH                                                1
#define REGSPEC_RMEA3_F2_SHIFT                                                0
#define REGSPEC_RMEA3_F2_MASK                                        0x00000001
#define REGSPEC_RMEA3_F2_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA3_F2_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA3_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG24	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB4_F2_WIDTH                                                 2
#define REGSPEC_RMB4_F2_SHIFT                                                 6
#define REGSPEC_RMB4_F2_MASK                                         0x000000c0
#define REGSPEC_RMB4_F2_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB4_F2_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB4_F2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB4_F2_WIDTH                                                1
#define REGSPEC_RMEB4_F2_SHIFT                                                4
#define REGSPEC_RMEB4_F2_MASK                                        0x00000010
#define REGSPEC_RMEB4_F2_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB4_F2_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB4_F2_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA4_F2_WIDTH                                                 2
#define REGSPEC_RMA4_F2_SHIFT                                                 2
#define REGSPEC_RMA4_F2_MASK                                         0x0000000c
#define REGSPEC_RMA4_F2_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA4_F2_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA4_F2_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA4_F2_WIDTH                                                1
#define REGSPEC_RMEA4_F2_SHIFT                                                0
#define REGSPEC_RMEA4_F2_MASK                                        0x00000001
#define REGSPEC_RMEA4_F2_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA4_F2_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA4_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG25	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB5_F2_WIDTH                                                 2
#define REGSPEC_RMB5_F2_SHIFT                                                 6
#define REGSPEC_RMB5_F2_MASK                                         0x000000c0
#define REGSPEC_RMB5_F2_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB5_F2_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB5_F2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB5_F2_WIDTH                                                1
#define REGSPEC_RMEB5_F2_SHIFT                                                4
#define REGSPEC_RMEB5_F2_MASK                                        0x00000010
#define REGSPEC_RMEB5_F2_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB5_F2_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB5_F2_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA5_F2_WIDTH                                                 2
#define REGSPEC_RMA5_F2_SHIFT                                                 2
#define REGSPEC_RMA5_F2_MASK                                         0x0000000c
#define REGSPEC_RMA5_F2_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA5_F2_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA5_F2_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA5_F2_WIDTH                                                1
#define REGSPEC_RMEA5_F2_SHIFT                                                0
#define REGSPEC_RMEA5_F2_MASK                                        0x00000001
#define REGSPEC_RMEA5_F2_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA5_F2_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA5_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG26	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB6_F2_WIDTH                                                 2
#define REGSPEC_RMB6_F2_SHIFT                                                 6
#define REGSPEC_RMB6_F2_MASK                                         0x000000c0
#define REGSPEC_RMB6_F2_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB6_F2_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB6_F2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB6_F2_WIDTH                                                1
#define REGSPEC_RMEB6_F2_SHIFT                                                4
#define REGSPEC_RMEB6_F2_MASK                                        0x00000010
#define REGSPEC_RMEB6_F2_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB6_F2_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB6_F2_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA6_F2_WIDTH                                                 2
#define REGSPEC_RMA6_F2_SHIFT                                                 2
#define REGSPEC_RMA6_F2_MASK                                         0x0000000c
#define REGSPEC_RMA6_F2_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA6_F2_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA6_F2_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA6_F2_WIDTH                                                1
#define REGSPEC_RMEA6_F2_SHIFT                                                0
#define REGSPEC_RMEA6_F2_MASK                                        0x00000001
#define REGSPEC_RMEA6_F2_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA6_F2_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA6_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG27	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB7_F2_WIDTH                                                 2
#define REGSPEC_RMB7_F2_SHIFT                                                 6
#define REGSPEC_RMB7_F2_MASK                                         0x000000c0
#define REGSPEC_RMB7_F2_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB7_F2_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB7_F2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB7_F2_WIDTH                                                1
#define REGSPEC_RMEB7_F2_SHIFT                                                4
#define REGSPEC_RMEB7_F2_MASK                                        0x00000010
#define REGSPEC_RMEB7_F2_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB7_F2_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB7_F2_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA7_F2_WIDTH                                                 2
#define REGSPEC_RMA7_F2_SHIFT                                                 2
#define REGSPEC_RMA7_F2_MASK                                         0x0000000c
#define REGSPEC_RMA7_F2_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA7_F2_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA7_F2_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA7_F2_WIDTH                                                1
#define REGSPEC_RMEA7_F2_SHIFT                                                0
#define REGSPEC_RMEA7_F2_MASK                                        0x00000001
#define REGSPEC_RMEA7_F2_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA7_F2_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA7_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG28	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB8_F2_WIDTH                                                 2
#define REGSPEC_RMB8_F2_SHIFT                                                 6
#define REGSPEC_RMB8_F2_MASK                                         0x000000c0
#define REGSPEC_RMB8_F2_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB8_F2_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB8_F2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB8_F2_WIDTH                                                1
#define REGSPEC_RMEB8_F2_SHIFT                                                4
#define REGSPEC_RMEB8_F2_MASK                                        0x00000010
#define REGSPEC_RMEB8_F2_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB8_F2_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB8_F2_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA8_F2_WIDTH                                                 2
#define REGSPEC_RMA8_F2_SHIFT                                                 2
#define REGSPEC_RMA8_F2_MASK                                         0x0000000c
#define REGSPEC_RMA8_F2_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA8_F2_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA8_F2_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA8_F2_WIDTH                                                1
#define REGSPEC_RMEA8_F2_SHIFT                                                0
#define REGSPEC_RMEA8_F2_MASK                                        0x00000001
#define REGSPEC_RMEA8_F2_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA8_F2_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA8_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG29	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB9_F2_WIDTH                                                 2
#define REGSPEC_RMB9_F2_SHIFT                                                 6
#define REGSPEC_RMB9_F2_MASK                                         0x000000c0
#define REGSPEC_RMB9_F2_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB9_F2_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB9_F2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB9_F2_WIDTH                                                1
#define REGSPEC_RMEB9_F2_SHIFT                                                4
#define REGSPEC_RMEB9_F2_MASK                                        0x00000010
#define REGSPEC_RMEB9_F2_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB9_F2_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB9_F2_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA9_F2_WIDTH                                                 2
#define REGSPEC_RMA9_F2_SHIFT                                                 2
#define REGSPEC_RMA9_F2_MASK                                         0x0000000c
#define REGSPEC_RMA9_F2_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA9_F2_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA9_F2_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA9_F2_WIDTH                                                1
#define REGSPEC_RMEA9_F2_SHIFT                                                0
#define REGSPEC_RMEA9_F2_MASK                                        0x00000001
#define REGSPEC_RMEA9_F2_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA9_F2_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA9_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG2A	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F12_WIDTH                                                 2
#define REGSPEC_RMB_F12_SHIFT                                                 6
#define REGSPEC_RMB_F12_MASK                                         0x000000c0
#define REGSPEC_RMB_F12_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F12_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F12_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F12_WIDTH                                                1
#define REGSPEC_RMEB_F12_SHIFT                                                4
#define REGSPEC_RMEB_F12_MASK                                        0x00000010
#define REGSPEC_RMEB_F12_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F12_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F12_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F12_WIDTH                                                 2
#define REGSPEC_RMA_F12_SHIFT                                                 2
#define REGSPEC_RMA_F12_MASK                                         0x0000000c
#define REGSPEC_RMA_F12_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F12_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F12_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F12_WIDTH                                                1
#define REGSPEC_RMEA_F12_SHIFT                                                0
#define REGSPEC_RMEA_F12_MASK                                        0x00000001
#define REGSPEC_RMEA_F12_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F12_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F12_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG2B	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F13_WIDTH                                                 2
#define REGSPEC_RMB_F13_SHIFT                                                 6
#define REGSPEC_RMB_F13_MASK                                         0x000000c0
#define REGSPEC_RMB_F13_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F13_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F13_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F13_WIDTH                                                1
#define REGSPEC_RMEB_F13_SHIFT                                                4
#define REGSPEC_RMEB_F13_MASK                                        0x00000010
#define REGSPEC_RMEB_F13_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F13_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F13_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F13_WIDTH                                                 2
#define REGSPEC_RMA_F13_SHIFT                                                 2
#define REGSPEC_RMA_F13_MASK                                         0x0000000c
#define REGSPEC_RMA_F13_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F13_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F13_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F13_WIDTH                                                1
#define REGSPEC_RMEA_F13_SHIFT                                                0
#define REGSPEC_RMEA_F13_MASK                                        0x00000001
#define REGSPEC_RMEA_F13_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F13_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F13_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG2C	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F14_WIDTH                                                 2
#define REGSPEC_RMB_F14_SHIFT                                                 6
#define REGSPEC_RMB_F14_MASK                                         0x000000c0
#define REGSPEC_RMB_F14_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F14_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F14_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F14_WIDTH                                                1
#define REGSPEC_RMEB_F14_SHIFT                                                4
#define REGSPEC_RMEB_F14_MASK                                        0x00000010
#define REGSPEC_RMEB_F14_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F14_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F14_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F14_WIDTH                                                 2
#define REGSPEC_RMA_F14_SHIFT                                                 2
#define REGSPEC_RMA_F14_MASK                                         0x0000000c
#define REGSPEC_RMA_F14_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F14_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F14_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F14_WIDTH                                                1
#define REGSPEC_RMEA_F14_SHIFT                                                0
#define REGSPEC_RMEA_F14_MASK                                        0x00000001
#define REGSPEC_RMEA_F14_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F14_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F14_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG2D	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F15_WIDTH                                                 2
#define REGSPEC_RMB_F15_SHIFT                                                 6
#define REGSPEC_RMB_F15_MASK                                         0x000000c0
#define REGSPEC_RMB_F15_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F15_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F15_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F15_WIDTH                                                1
#define REGSPEC_RMEB_F15_SHIFT                                                4
#define REGSPEC_RMEB_F15_MASK                                        0x00000010
#define REGSPEC_RMEB_F15_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F15_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F15_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F15_WIDTH                                                 2
#define REGSPEC_RMA_F15_SHIFT                                                 2
#define REGSPEC_RMA_F15_MASK                                         0x0000000c
#define REGSPEC_RMA_F15_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F15_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F15_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F15_WIDTH                                                1
#define REGSPEC_RMEA_F15_SHIFT                                                0
#define REGSPEC_RMEA_F15_MASK                                        0x00000001
#define REGSPEC_RMEA_F15_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F15_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F15_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG2E	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F16_WIDTH                                                 2
#define REGSPEC_RMB_F16_SHIFT                                                 6
#define REGSPEC_RMB_F16_MASK                                         0x000000c0
#define REGSPEC_RMB_F16_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F16_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F16_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F16_WIDTH                                                1
#define REGSPEC_RMEB_F16_SHIFT                                                4
#define REGSPEC_RMEB_F16_MASK                                        0x00000010
#define REGSPEC_RMEB_F16_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F16_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F16_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F16_WIDTH                                                 2
#define REGSPEC_RMA_F16_SHIFT                                                 2
#define REGSPEC_RMA_F16_MASK                                         0x0000000c
#define REGSPEC_RMA_F16_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F16_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F16_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F16_WIDTH                                                1
#define REGSPEC_RMEA_F16_SHIFT                                                0
#define REGSPEC_RMEA_F16_MASK                                        0x00000001
#define REGSPEC_RMEA_F16_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F16_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F16_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG2F	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F17_WIDTH                                                 2
#define REGSPEC_RMB_F17_SHIFT                                                 6
#define REGSPEC_RMB_F17_MASK                                         0x000000c0
#define REGSPEC_RMB_F17_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F17_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F17_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F17_WIDTH                                                1
#define REGSPEC_RMEB_F17_SHIFT                                                4
#define REGSPEC_RMEB_F17_MASK                                        0x00000010
#define REGSPEC_RMEB_F17_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F17_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F17_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F17_WIDTH                                                 2
#define REGSPEC_RMA_F17_SHIFT                                                 2
#define REGSPEC_RMA_F17_MASK                                         0x0000000c
#define REGSPEC_RMA_F17_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F17_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F17_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F17_WIDTH                                                1
#define REGSPEC_RMEA_F17_SHIFT                                                0
#define REGSPEC_RMEA_F17_MASK                                        0x00000001
#define REGSPEC_RMEA_F17_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F17_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F17_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG30	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB0_F3_WIDTH                                                 2
#define REGSPEC_RMB0_F3_SHIFT                                                 6
#define REGSPEC_RMB0_F3_MASK                                         0x000000c0
#define REGSPEC_RMB0_F3_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB0_F3_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB0_F3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB0_F3_WIDTH                                                1
#define REGSPEC_RMEB0_F3_SHIFT                                                4
#define REGSPEC_RMEB0_F3_MASK                                        0x00000010
#define REGSPEC_RMEB0_F3_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB0_F3_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB0_F3_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA0_F3_WIDTH                                                 2
#define REGSPEC_RMA0_F3_SHIFT                                                 2
#define REGSPEC_RMA0_F3_MASK                                         0x0000000c
#define REGSPEC_RMA0_F3_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA0_F3_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA0_F3_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA0_F3_WIDTH                                                1
#define REGSPEC_RMEA0_F3_SHIFT                                                0
#define REGSPEC_RMEA0_F3_MASK                                        0x00000001
#define REGSPEC_RMEA0_F3_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA0_F3_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA0_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG31	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB1_F3_WIDTH                                                 2
#define REGSPEC_RMB1_F3_SHIFT                                                 6
#define REGSPEC_RMB1_F3_MASK                                         0x000000c0
#define REGSPEC_RMB1_F3_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB1_F3_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB1_F3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB1_F3_WIDTH                                                1
#define REGSPEC_RMEB1_F3_SHIFT                                                4
#define REGSPEC_RMEB1_F3_MASK                                        0x00000010
#define REGSPEC_RMEB1_F3_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB1_F3_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB1_F3_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA1_F3_WIDTH                                                 2
#define REGSPEC_RMA1_F3_SHIFT                                                 2
#define REGSPEC_RMA1_F3_MASK                                         0x0000000c
#define REGSPEC_RMA1_F3_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA1_F3_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA1_F3_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA1_F3_WIDTH                                                1
#define REGSPEC_RMEA1_F3_SHIFT                                                0
#define REGSPEC_RMEA1_F3_MASK                                        0x00000001
#define REGSPEC_RMEA1_F3_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA1_F3_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA1_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG32	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB2_F3_WIDTH                                                 2
#define REGSPEC_RMB2_F3_SHIFT                                                 6
#define REGSPEC_RMB2_F3_MASK                                         0x000000c0
#define REGSPEC_RMB2_F3_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB2_F3_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB2_F3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB2_F3_WIDTH                                                1
#define REGSPEC_RMEB2_F3_SHIFT                                                4
#define REGSPEC_RMEB2_F3_MASK                                        0x00000010
#define REGSPEC_RMEB2_F3_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB2_F3_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB2_F3_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA2_F3_WIDTH                                                 2
#define REGSPEC_RMA2_F3_SHIFT                                                 2
#define REGSPEC_RMA2_F3_MASK                                         0x0000000c
#define REGSPEC_RMA2_F3_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA2_F3_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA2_F3_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA2_F3_WIDTH                                                1
#define REGSPEC_RMEA2_F3_SHIFT                                                0
#define REGSPEC_RMEA2_F3_MASK                                        0x00000001
#define REGSPEC_RMEA2_F3_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA2_F3_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA2_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG33	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB3_F3_WIDTH                                                 2
#define REGSPEC_RMB3_F3_SHIFT                                                 6
#define REGSPEC_RMB3_F3_MASK                                         0x000000c0
#define REGSPEC_RMB3_F3_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB3_F3_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB3_F3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB3_F3_WIDTH                                                1
#define REGSPEC_RMEB3_F3_SHIFT                                                4
#define REGSPEC_RMEB3_F3_MASK                                        0x00000010
#define REGSPEC_RMEB3_F3_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB3_F3_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB3_F3_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA3_F3_WIDTH                                                 2
#define REGSPEC_RMA3_F3_SHIFT                                                 2
#define REGSPEC_RMA3_F3_MASK                                         0x0000000c
#define REGSPEC_RMA3_F3_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA3_F3_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA3_F3_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA3_F3_WIDTH                                                1
#define REGSPEC_RMEA3_F3_SHIFT                                                0
#define REGSPEC_RMEA3_F3_MASK                                        0x00000001
#define REGSPEC_RMEA3_F3_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA3_F3_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA3_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG34	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB4_F3_WIDTH                                                 2
#define REGSPEC_RMB4_F3_SHIFT                                                 6
#define REGSPEC_RMB4_F3_MASK                                         0x000000c0
#define REGSPEC_RMB4_F3_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB4_F3_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB4_F3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB4_F3_WIDTH                                                1
#define REGSPEC_RMEB4_F3_SHIFT                                                4
#define REGSPEC_RMEB4_F3_MASK                                        0x00000010
#define REGSPEC_RMEB4_F3_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB4_F3_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB4_F3_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA4_F3_WIDTH                                                 2
#define REGSPEC_RMA4_F3_SHIFT                                                 2
#define REGSPEC_RMA4_F3_MASK                                         0x0000000c
#define REGSPEC_RMA4_F3_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA4_F3_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA4_F3_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA4_F3_WIDTH                                                1
#define REGSPEC_RMEA4_F3_SHIFT                                                0
#define REGSPEC_RMEA4_F3_MASK                                        0x00000001
#define REGSPEC_RMEA4_F3_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA4_F3_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA4_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG35	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB5_F3_WIDTH                                                 2
#define REGSPEC_RMB5_F3_SHIFT                                                 6
#define REGSPEC_RMB5_F3_MASK                                         0x000000c0
#define REGSPEC_RMB5_F3_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB5_F3_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB5_F3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB5_F3_WIDTH                                                1
#define REGSPEC_RMEB5_F3_SHIFT                                                4
#define REGSPEC_RMEB5_F3_MASK                                        0x00000010
#define REGSPEC_RMEB5_F3_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB5_F3_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB5_F3_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA5_F3_WIDTH                                                 2
#define REGSPEC_RMA5_F3_SHIFT                                                 2
#define REGSPEC_RMA5_F3_MASK                                         0x0000000c
#define REGSPEC_RMA5_F3_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA5_F3_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA5_F3_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA5_F3_WIDTH                                                1
#define REGSPEC_RMEA5_F3_SHIFT                                                0
#define REGSPEC_RMEA5_F3_MASK                                        0x00000001
#define REGSPEC_RMEA5_F3_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA5_F3_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA5_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG36	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB6_F3_WIDTH                                                 2
#define REGSPEC_RMB6_F3_SHIFT                                                 6
#define REGSPEC_RMB6_F3_MASK                                         0x000000c0
#define REGSPEC_RMB6_F3_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB6_F3_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB6_F3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB6_F3_WIDTH                                                1
#define REGSPEC_RMEB6_F3_SHIFT                                                4
#define REGSPEC_RMEB6_F3_MASK                                        0x00000010
#define REGSPEC_RMEB6_F3_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB6_F3_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB6_F3_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA6_F3_WIDTH                                                 2
#define REGSPEC_RMA6_F3_SHIFT                                                 2
#define REGSPEC_RMA6_F3_MASK                                         0x0000000c
#define REGSPEC_RMA6_F3_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA6_F3_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA6_F3_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA6_F3_WIDTH                                                1
#define REGSPEC_RMEA6_F3_SHIFT                                                0
#define REGSPEC_RMEA6_F3_MASK                                        0x00000001
#define REGSPEC_RMEA6_F3_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA6_F3_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA6_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG37	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB7_F3_WIDTH                                                 2
#define REGSPEC_RMB7_F3_SHIFT                                                 6
#define REGSPEC_RMB7_F3_MASK                                         0x000000c0
#define REGSPEC_RMB7_F3_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB7_F3_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB7_F3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB7_F3_WIDTH                                                1
#define REGSPEC_RMEB7_F3_SHIFT                                                4
#define REGSPEC_RMEB7_F3_MASK                                        0x00000010
#define REGSPEC_RMEB7_F3_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB7_F3_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB7_F3_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA7_F3_WIDTH                                                 2
#define REGSPEC_RMA7_F3_SHIFT                                                 2
#define REGSPEC_RMA7_F3_MASK                                         0x0000000c
#define REGSPEC_RMA7_F3_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA7_F3_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA7_F3_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA7_F3_WIDTH                                                1
#define REGSPEC_RMEA7_F3_SHIFT                                                0
#define REGSPEC_RMEA7_F3_MASK                                        0x00000001
#define REGSPEC_RMEA7_F3_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA7_F3_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA7_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG38	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB8_F3_WIDTH                                                 2
#define REGSPEC_RMB8_F3_SHIFT                                                 6
#define REGSPEC_RMB8_F3_MASK                                         0x000000c0
#define REGSPEC_RMB8_F3_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB8_F3_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB8_F3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB8_F3_WIDTH                                                1
#define REGSPEC_RMEB8_F3_SHIFT                                                4
#define REGSPEC_RMEB8_F3_MASK                                        0x00000010
#define REGSPEC_RMEB8_F3_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB8_F3_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB8_F3_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA8_F3_WIDTH                                                 2
#define REGSPEC_RMA8_F3_SHIFT                                                 2
#define REGSPEC_RMA8_F3_MASK                                         0x0000000c
#define REGSPEC_RMA8_F3_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA8_F3_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA8_F3_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA8_F3_WIDTH                                                1
#define REGSPEC_RMEA8_F3_SHIFT                                                0
#define REGSPEC_RMEA8_F3_MASK                                        0x00000001
#define REGSPEC_RMEA8_F3_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA8_F3_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA8_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG39	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB9_F3_WIDTH                                                 2
#define REGSPEC_RMB9_F3_SHIFT                                                 6
#define REGSPEC_RMB9_F3_MASK                                         0x000000c0
#define REGSPEC_RMB9_F3_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB9_F3_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB9_F3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB9_F3_WIDTH                                                1
#define REGSPEC_RMEB9_F3_SHIFT                                                4
#define REGSPEC_RMEB9_F3_MASK                                        0x00000010
#define REGSPEC_RMEB9_F3_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB9_F3_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB9_F3_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA9_F3_WIDTH                                                 2
#define REGSPEC_RMA9_F3_SHIFT                                                 2
#define REGSPEC_RMA9_F3_MASK                                         0x0000000c
#define REGSPEC_RMA9_F3_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA9_F3_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA9_F3_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA9_F3_WIDTH                                                1
#define REGSPEC_RMEA9_F3_SHIFT                                                0
#define REGSPEC_RMEA9_F3_MASK                                        0x00000001
#define REGSPEC_RMEA9_F3_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA9_F3_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA9_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG3A	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F18_WIDTH                                                 2
#define REGSPEC_RMB_F18_SHIFT                                                 6
#define REGSPEC_RMB_F18_MASK                                         0x000000c0
#define REGSPEC_RMB_F18_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F18_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F18_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F18_WIDTH                                                1
#define REGSPEC_RMEB_F18_SHIFT                                                4
#define REGSPEC_RMEB_F18_MASK                                        0x00000010
#define REGSPEC_RMEB_F18_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F18_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F18_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F18_WIDTH                                                 2
#define REGSPEC_RMA_F18_SHIFT                                                 2
#define REGSPEC_RMA_F18_MASK                                         0x0000000c
#define REGSPEC_RMA_F18_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F18_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F18_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F18_WIDTH                                                1
#define REGSPEC_RMEA_F18_SHIFT                                                0
#define REGSPEC_RMEA_F18_MASK                                        0x00000001
#define REGSPEC_RMEA_F18_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F18_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F18_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG3B	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F19_WIDTH                                                 2
#define REGSPEC_RMB_F19_SHIFT                                                 6
#define REGSPEC_RMB_F19_MASK                                         0x000000c0
#define REGSPEC_RMB_F19_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F19_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F19_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F19_WIDTH                                                1
#define REGSPEC_RMEB_F19_SHIFT                                                4
#define REGSPEC_RMEB_F19_MASK                                        0x00000010
#define REGSPEC_RMEB_F19_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F19_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F19_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F19_WIDTH                                                 2
#define REGSPEC_RMA_F19_SHIFT                                                 2
#define REGSPEC_RMA_F19_MASK                                         0x0000000c
#define REGSPEC_RMA_F19_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F19_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F19_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F19_WIDTH                                                1
#define REGSPEC_RMEA_F19_SHIFT                                                0
#define REGSPEC_RMEA_F19_MASK                                        0x00000001
#define REGSPEC_RMEA_F19_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F19_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F19_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG3C	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F20_WIDTH                                                 2
#define REGSPEC_RMB_F20_SHIFT                                                 6
#define REGSPEC_RMB_F20_MASK                                         0x000000c0
#define REGSPEC_RMB_F20_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F20_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F20_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F20_WIDTH                                                1
#define REGSPEC_RMEB_F20_SHIFT                                                4
#define REGSPEC_RMEB_F20_MASK                                        0x00000010
#define REGSPEC_RMEB_F20_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F20_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F20_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F20_WIDTH                                                 2
#define REGSPEC_RMA_F20_SHIFT                                                 2
#define REGSPEC_RMA_F20_MASK                                         0x0000000c
#define REGSPEC_RMA_F20_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F20_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F20_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F20_WIDTH                                                1
#define REGSPEC_RMEA_F20_SHIFT                                                0
#define REGSPEC_RMEA_F20_MASK                                        0x00000001
#define REGSPEC_RMEA_F20_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F20_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F20_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG3D	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F21_WIDTH                                                 2
#define REGSPEC_RMB_F21_SHIFT                                                 6
#define REGSPEC_RMB_F21_MASK                                         0x000000c0
#define REGSPEC_RMB_F21_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F21_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F21_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F21_WIDTH                                                1
#define REGSPEC_RMEB_F21_SHIFT                                                4
#define REGSPEC_RMEB_F21_MASK                                        0x00000010
#define REGSPEC_RMEB_F21_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F21_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F21_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F21_WIDTH                                                 2
#define REGSPEC_RMA_F21_SHIFT                                                 2
#define REGSPEC_RMA_F21_MASK                                         0x0000000c
#define REGSPEC_RMA_F21_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F21_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F21_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F21_WIDTH                                                1
#define REGSPEC_RMEA_F21_SHIFT                                                0
#define REGSPEC_RMEA_F21_MASK                                        0x00000001
#define REGSPEC_RMEA_F21_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F21_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F21_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG3E	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F22_WIDTH                                                 2
#define REGSPEC_RMB_F22_SHIFT                                                 6
#define REGSPEC_RMB_F22_MASK                                         0x000000c0
#define REGSPEC_RMB_F22_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F22_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F22_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F22_WIDTH                                                1
#define REGSPEC_RMEB_F22_SHIFT                                                4
#define REGSPEC_RMEB_F22_MASK                                        0x00000010
#define REGSPEC_RMEB_F22_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F22_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F22_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F22_WIDTH                                                 2
#define REGSPEC_RMA_F22_SHIFT                                                 2
#define REGSPEC_RMA_F22_MASK                                         0x0000000c
#define REGSPEC_RMA_F22_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F22_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F22_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F22_WIDTH                                                1
#define REGSPEC_RMEA_F22_SHIFT                                                0
#define REGSPEC_RMEA_F22_MASK                                        0x00000001
#define REGSPEC_RMEA_F22_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F22_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F22_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBMEMCFG3F	*/ 
/*	 Fields RMB	 */
#define REGSPEC_RMB_F23_WIDTH                                                 2
#define REGSPEC_RMB_F23_SHIFT                                                 6
#define REGSPEC_RMB_F23_MASK                                         0x000000c0
#define REGSPEC_RMB_F23_RD(src)                       (((src) & 0x000000c0)>>6)
#define REGSPEC_RMB_F23_WR(src)                  (((u32)(src)<<6) & 0x000000c0)
#define REGSPEC_RMB_F23_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields RMEB	 */
#define REGSPEC_RMEB_F23_WIDTH                                                1
#define REGSPEC_RMEB_F23_SHIFT                                                4
#define REGSPEC_RMEB_F23_MASK                                        0x00000010
#define REGSPEC_RMEB_F23_RD(src)                      (((src) & 0x00000010)>>4)
#define REGSPEC_RMEB_F23_WR(src)                 (((u32)(src)<<4) & 0x00000010)
#define REGSPEC_RMEB_F23_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RMA	 */
#define REGSPEC_RMA_F23_WIDTH                                                 2
#define REGSPEC_RMA_F23_SHIFT                                                 2
#define REGSPEC_RMA_F23_MASK                                         0x0000000c
#define REGSPEC_RMA_F23_RD(src)                       (((src) & 0x0000000c)>>2)
#define REGSPEC_RMA_F23_WR(src)                  (((u32)(src)<<2) & 0x0000000c)
#define REGSPEC_RMA_F23_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define REGSPEC_RMEA_F23_WIDTH                                                1
#define REGSPEC_RMEA_F23_SHIFT                                                0
#define REGSPEC_RMEA_F23_MASK                                        0x00000001
#define REGSPEC_RMEA_F23_RD(src)                         (((src) & 0x00000001))
#define REGSPEC_RMEA_F23_WR(src)                    (((u32)(src)) & 0x00000001)
#define REGSPEC_RMEA_F23_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register PCPEFUSECTRL	*/ 
/*	 Fields MCU_POWER_ON	 */
#define MCU_POWER_ON_WIDTH                                                    4
#define MCU_POWER_ON_SHIFT                                                    8
#define MCU_POWER_ON_MASK                                            0x00000f00
#define MCU_POWER_ON_RD(src)                          (((src) & 0x00000f00)>>8)
#define MCU_POWER_ON_WR(src)                     (((u32)(src)<<8) & 0x00000f00)
#define MCU_POWER_ON_SET(dst,src) \
                       (((dst) & ~0x00000f00) | (((u32)(src)<<8) & 0x00000f00))
/*	 Fields L3CINCR_POWER_ON	 */
#define L3CINCR_POWER_ON_WIDTH                                                4
#define L3CINCR_POWER_ON_SHIFT                                                4
#define L3CINCR_POWER_ON_MASK                                        0x000000f0
#define L3CINCR_POWER_ON_RD(src)                      (((src) & 0x000000f0)>>4)
#define L3CINCR_POWER_ON_WR(src)                 (((u32)(src)<<4) & 0x000000f0)
#define L3CINCR_POWER_ON_SET(dst,src) \
                       (((dst) & ~0x000000f0) | (((u32)(src)<<4) & 0x000000f0))
/*	 Fields L3C_POWER_ON	 */
#define L3C_POWER_ON_WIDTH                                                    1
#define L3C_POWER_ON_SHIFT                                                    3
#define L3C_POWER_ON_MASK                                            0x00000008
#define L3C_POWER_ON_RD(src)                          (((src) & 0x00000008)>>3)
#define L3C_POWER_ON_WR(src)                     (((u32)(src)<<3) & 0x00000008)
#define L3C_POWER_ON_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields PCP_EFUSE_START	 */
#define PCP_EFUSE_START_WIDTH                                                 1
#define PCP_EFUSE_START_SHIFT                                                 0
#define PCP_EFUSE_START_MASK                                         0x00000001
#define PCP_EFUSE_START_RD(src)                          (((src) & 0x00000001))
#define PCP_EFUSE_START_WR(src)                     (((u32)(src)) & 0x00000001)
#define PCP_EFUSE_START_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register PCPEFUSESTAT	*/ 
/*	 Fields MBIST_FAIL	 */
#define MBIST_FAIL_WIDTH                                                      1
#define MBIST_FAIL_SHIFT                                                      1
#define MBIST_FAIL_MASK                                              0x00000002
#define MBIST_FAIL_RD(src)                            (((src) & 0x00000002)>>1)
#define MBIST_FAIL_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields MBIST_RDY	 */
#define MBIST_RDY_WIDTH                                                       1
#define MBIST_RDY_SHIFT                                                       0
#define MBIST_RDY_MASK                                               0x00000001
#define MBIST_RDY_RD(src)                                (((src) & 0x00000001))
#define MBIST_RDY_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Global Base Address	*/
#define IOB_ERR_REGS_BASE_ADDR			0x07e930000ULL

/*    Address IOB_ERR_REGS  Registers */
#define IOBAXIS0TRANSERRINTSTS_ADDR                                  0x00000000
#define IOBAXIS0TRANSERRINTSTS_DEFAULT                               0x00000000
#define IOBAXIS0TRANSERRINTMSK_ADDR                                  0x00000004
#define IOBAXIS0TRANSERRINTMSK_DEFAULT                               0xffffffff
#define IOBAXIS0TRANSERRREQINFOL_ADDR                                0x00000008
#define IOBAXIS0TRANSERRREQINFOL_DEFAULT                             0x00000000
#define IOBAXIS0TRANSERRREQINFOH_ADDR                                0x0000000c
#define IOBAXIS0TRANSERRREQINFOH_DEFAULT                             0x00000000
#define IOBAXIS1TRANSERRINTSTS_ADDR                                  0x00000010
#define IOBAXIS1TRANSERRINTSTS_DEFAULT                               0x00000000
#define IOBAXIS1TRANSERRINTMSK_ADDR                                  0x00000014
#define IOBAXIS1TRANSERRINTMSK_DEFAULT                               0xffffffff
#define IOBAXIS1TRANSERRREQINFOL_ADDR                                0x00000018
#define IOBAXIS1TRANSERRREQINFOL_DEFAULT                             0x00000000
#define IOBAXIS1TRANSERRREQINFOH_ADDR                                0x0000001c
#define IOBAXIS1TRANSERRREQINFOH_DEFAULT                             0x00000000
#define IOBPATRANSERRINTSTS_ADDR                                     0x00000020
#define IOBPATRANSERRINTSTS_DEFAULT                                  0x00000000
#define IOBPATRANSERRINTMSK_ADDR                                     0x00000024
#define IOBPATRANSERRINTMSK_DEFAULT                                  0xffffffff
#define IOBBATRANSERRINTSTS_ADDR                                     0x00000030
#define IOBBATRANSERRINTSTS_DEFAULT                                  0x00000000
#define IOBBATRANSERRINTMSK_ADDR                                     0x00000034
#define IOBBATRANSERRINTMSK_DEFAULT                                  0xffffffff
#define IOBBATRANSERRREQINFOL_ADDR                                   0x00000038
#define IOBBATRANSERRREQINFOL_DEFAULT                                0x00000000
#define IOBBATRANSERRREQINFOH_ADDR                                   0x0000003c
#define IOBBATRANSERRREQINFOH_DEFAULT                                0x00000000
#define IOBBATRANSERRCSWREQID_ADDR                                   0x00000040
#define IOBBATRANSERRCSWREQID_DEFAULT                                0x00000000
#define XGICTRANSERRINTSTS_ADDR                                      0x00000050
#define XGICTRANSERRINTSTS_DEFAULT                                   0x00000000
#define XGICTRANSERRINTMSK_ADDR                                      0x00000054
#define XGICTRANSERRINTMSK_DEFAULT                                   0xffffffff
#define XGICTRANSERRREQINFO_ADDR                                     0x00000058
#define XGICTRANSERRREQINFO_DEFAULT                                  0x00000000
#define IOBERRRESP_ADDR                                              0x000007fc
#define IOBERRRESP_DEFAULT                                           0x00000000

/*	Register IOBAXIS0TRANSERRINTSTS	*/ 
/*	 Fields M_ILLEGAL_ACCESS	 */
#define M_ILLEGAL_ACCESS_WIDTH                                                1
#define M_ILLEGAL_ACCESS_SHIFT                                                1
#define M_ILLEGAL_ACCESS_MASK                                        0x00000002
#define M_ILLEGAL_ACCESS_RD(src)                      (((src) & 0x00000002)>>1)
#define M_ILLEGAL_ACCESS_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields ILLEGAL_ACCESS	 */
#define ILLEGAL_ACCESS_WIDTH                                                  1
#define ILLEGAL_ACCESS_SHIFT                                                  0
#define ILLEGAL_ACCESS_MASK                                          0x00000001
#define ILLEGAL_ACCESS_RD(src)                           (((src) & 0x00000001))
#define ILLEGAL_ACCESS_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBAXIS0TRANSERRINTMSK	*/ 
/*	 Fields M_ILLEGAL_ACCESS	 */
#define M_ILLEGAL_ACCESS_F1_WIDTH                                             1
#define M_ILLEGAL_ACCESS_F1_SHIFT                                             1
#define M_ILLEGAL_ACCESS_F1_MASK                                     0x00000002
#define M_ILLEGAL_ACCESS_F1_RD(src)                   (((src) & 0x00000002)>>1)
#define M_ILLEGAL_ACCESS_F1_WR(src)              (((u32)(src)<<1) & 0x00000002)
#define M_ILLEGAL_ACCESS_F1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields ILLEGAL_ACCESS	 */
#define ILLEGAL_ACCESS_F1_WIDTH                                               1
#define ILLEGAL_ACCESS_F1_SHIFT                                               0
#define ILLEGAL_ACCESS_F1_MASK                                       0x00000001
#define ILLEGAL_ACCESS_F1_RD(src)                        (((src) & 0x00000001))
#define ILLEGAL_ACCESS_F1_WR(src)                   (((u32)(src)) & 0x00000001)
#define ILLEGAL_ACCESS_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBAXIS0TRANSERRREQINFOL	*/ 
/*	 Fields ERRADDRL	 */
#define ERRADDRL_WIDTH                                                       32
#define ERRADDRL_SHIFT                                                        0
#define ERRADDRL_MASK                                                0xffffffff
#define ERRADDRL_RD(src)                                 (((src) & 0xffffffff))
#define ERRADDRL_WR(src)                            (((u32)(src)) & 0xffffffff)
#define ERRADDRL_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IOBAXIS0TRANSERRREQINFOH	*/ 
/*	 Fields ERRADDRH	 */
#define ERRADDRH_WIDTH                                                       10
#define ERRADDRH_SHIFT                                                       22
#define ERRADDRH_MASK                                                0xffc00000
#define ERRADDRH_RD(src)                             (((src) & 0xffc00000)>>22)
#define ERRADDRH_WR(src)                        (((u32)(src)<<22) & 0xffc00000)
#define ERRADDRH_SET(dst,src) \
                      (((dst) & ~0xffc00000) | (((u32)(src)<<22) & 0xffc00000))
/*	 Fields AGENTID	 */
#define AGENTID_WIDTH                                                         6
#define AGENTID_SHIFT                                                        16
#define AGENTID_MASK                                                 0x003f0000
#define AGENTID_RD(src)                              (((src) & 0x003f0000)>>16)
#define AGENTID_WR(src)                         (((u32)(src)<<16) & 0x003f0000)
#define AGENTID_SET(dst,src) \
                      (((dst) & ~0x003f0000) | (((u32)(src)<<16) & 0x003f0000))
/*	 Fields AUXINFO	 */
#define IOBAXIS0TRANSERRREQINFOH_AUXINFO_WIDTH                                                         6
#define IOBAXIS0TRANSERRREQINFOH_AUXINFO_SHIFT                                                        10
#define IOBAXIS0TRANSERRREQINFOH_AUXINFO_MASK                                                 0x0000fc00
#define IOBAXIS0TRANSERRREQINFOH_AUXINFO_RD(src)                              (((src) & 0x0000fc00)>>10)
#define IOBAXIS0TRANSERRREQINFOH_AUXINFO_WR(src)                         (((u32)(src)<<10) & 0x0000fc00)
#define IOBAXIS0TRANSERRREQINFOH_AUXINFO_SET(dst,src) \
                      (((dst) & ~0x0000fc00) | (((u32)(src)<<10) & 0x0000fc00))
/*	 Fields REQLEN	 */
#define REQLEN_WIDTH                                                          2
#define REQLEN_SHIFT                                                          4
#define REQLEN_MASK                                                  0x00000030
#define REQLEN_RD(src)                                (((src) & 0x00000030)>>4)
#define REQLEN_WR(src)                           (((u32)(src)<<4) & 0x00000030)
#define REQLEN_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*	 Fields REQSIZE	 */
#define REQSIZE_WIDTH                                                         3
#define REQSIZE_SHIFT                                                         1
#define REQSIZE_MASK                                                 0x0000000e
#define REQSIZE_RD(src)                               (((src) & 0x0000000e)>>1)
#define REQSIZE_WR(src)                          (((u32)(src)<<1) & 0x0000000e)
#define REQSIZE_SET(dst,src) \
                       (((dst) & ~0x0000000e) | (((u32)(src)<<1) & 0x0000000e))
/*	 Fields REQTYPE	 */
#define REQTYPE_WIDTH                                                         1
#define REQTYPE_SHIFT                                                         0
#define REQTYPE_MASK                                                 0x00000001
#define REQTYPE_RD(src)                                  (((src) & 0x00000001))
#define REQTYPE_WR(src)                             (((u32)(src)) & 0x00000001)
#define REQTYPE_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBAXIS1TRANSERRINTSTS	*/ 
/*	 Fields M_ILLEGAL_ACCESS	 */
#define M_ILLEGAL_ACCESS_F2_WIDTH                                             1
#define M_ILLEGAL_ACCESS_F2_SHIFT                                             1
#define M_ILLEGAL_ACCESS_F2_MASK                                     0x00000002
#define M_ILLEGAL_ACCESS_F2_RD(src)                   (((src) & 0x00000002)>>1)
#define M_ILLEGAL_ACCESS_F2_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields ILLEGAL_ACCESS	 */
#define ILLEGAL_ACCESS_F2_WIDTH                                               1
#define ILLEGAL_ACCESS_F2_SHIFT                                               0
#define ILLEGAL_ACCESS_F2_MASK                                       0x00000001
#define ILLEGAL_ACCESS_F2_RD(src)                        (((src) & 0x00000001))
#define ILLEGAL_ACCESS_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBAXIS1TRANSERRINTMSK	*/ 
/*	 Fields M_ILLEGAL_ACCESS	 */
#define M_ILLEGAL_ACCESS_F3_WIDTH                                             1
#define M_ILLEGAL_ACCESS_F3_SHIFT                                             1
#define M_ILLEGAL_ACCESS_F3_MASK                                     0x00000002
#define M_ILLEGAL_ACCESS_F3_RD(src)                   (((src) & 0x00000002)>>1)
#define M_ILLEGAL_ACCESS_F3_WR(src)              (((u32)(src)<<1) & 0x00000002)
#define M_ILLEGAL_ACCESS_F3_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields ILLEGAL_ACCESS	 */
#define ILLEGAL_ACCESS_F3_WIDTH                                               1
#define ILLEGAL_ACCESS_F3_SHIFT                                               0
#define ILLEGAL_ACCESS_F3_MASK                                       0x00000001
#define ILLEGAL_ACCESS_F3_RD(src)                        (((src) & 0x00000001))
#define ILLEGAL_ACCESS_F3_WR(src)                   (((u32)(src)) & 0x00000001)
#define ILLEGAL_ACCESS_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBAXIS1TRANSERRREQINFOL	*/ 
/*	 Fields ERRADDRL	 */
#define ERRADDRL_F1_WIDTH                                                    32
#define ERRADDRL_F1_SHIFT                                                     0
#define ERRADDRL_F1_MASK                                             0xffffffff
#define ERRADDRL_F1_RD(src)                              (((src) & 0xffffffff))
#define ERRADDRL_F1_WR(src)                         (((u32)(src)) & 0xffffffff)
#define ERRADDRL_F1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IOBAXIS1TRANSERRREQINFOH	*/ 
/*	 Fields ERRADDRH	 */
#define ERRADDRH_F1_WIDTH                                                    10
#define ERRADDRH_F1_SHIFT                                                    22
#define ERRADDRH_F1_MASK                                             0xffc00000
#define ERRADDRH_F1_RD(src)                          (((src) & 0xffc00000)>>22)
#define ERRADDRH_F1_WR(src)                     (((u32)(src)<<22) & 0xffc00000)
#define ERRADDRH_F1_SET(dst,src) \
                      (((dst) & ~0xffc00000) | (((u32)(src)<<22) & 0xffc00000))
/*	 Fields AGENTID	 */
#define AGENTID_F1_WIDTH                                                      6
#define AGENTID_F1_SHIFT                                                     16
#define AGENTID_F1_MASK                                              0x003f0000
#define AGENTID_F1_RD(src)                           (((src) & 0x003f0000)>>16)
#define AGENTID_F1_WR(src)                      (((u32)(src)<<16) & 0x003f0000)
#define AGENTID_F1_SET(dst,src) \
                      (((dst) & ~0x003f0000) | (((u32)(src)<<16) & 0x003f0000))
/*	 Fields AUXINFO	 */
#define AUXINFO_F1_WIDTH                                                      6
#define AUXINFO_F1_SHIFT                                                     10
#define AUXINFO_F1_MASK                                              0x0000fc00
#define AUXINFO_F1_RD(src)                           (((src) & 0x0000fc00)>>10)
#define AUXINFO_F1_WR(src)                      (((u32)(src)<<10) & 0x0000fc00)
#define AUXINFO_F1_SET(dst,src) \
                      (((dst) & ~0x0000fc00) | (((u32)(src)<<10) & 0x0000fc00))
/*	 Fields REQLEN	 */
#define REQLEN_F1_WIDTH                                                       2
#define REQLEN_F1_SHIFT                                                       4
#define REQLEN_F1_MASK                                               0x00000030
#define REQLEN_F1_RD(src)                             (((src) & 0x00000030)>>4)
#define REQLEN_F1_WR(src)                        (((u32)(src)<<4) & 0x00000030)
#define REQLEN_F1_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*	 Fields REQSIZE	 */
#define REQSIZE_F1_WIDTH                                                      3
#define REQSIZE_F1_SHIFT                                                      1
#define REQSIZE_F1_MASK                                              0x0000000e
#define REQSIZE_F1_RD(src)                            (((src) & 0x0000000e)>>1)
#define REQSIZE_F1_WR(src)                       (((u32)(src)<<1) & 0x0000000e)
#define REQSIZE_F1_SET(dst,src) \
                       (((dst) & ~0x0000000e) | (((u32)(src)<<1) & 0x0000000e))
/*	 Fields REQTYPE	 */
#define REQTYPE_F1_WIDTH                                                      1
#define REQTYPE_F1_SHIFT                                                      0
#define REQTYPE_F1_MASK                                              0x00000001
#define REQTYPE_F1_RD(src)                               (((src) & 0x00000001))
#define REQTYPE_F1_WR(src)                          (((u32)(src)) & 0x00000001)
#define REQTYPE_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBPATRANSERRINTSTS	*/ 
/*	 Fields M_REQIDRAM_CORRUPT	 */
#define M_REQIDRAM_CORRUPT_WIDTH                                              1
#define M_REQIDRAM_CORRUPT_SHIFT                                              7
#define M_REQIDRAM_CORRUPT_MASK                                      0x00000080
#define M_REQIDRAM_CORRUPT_RD(src)                    (((src) & 0x00000080)>>7)
#define M_REQIDRAM_CORRUPT_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields REQIDRAM_CORRUPT	 */
#define REQIDRAM_CORRUPT_WIDTH                                                1
#define REQIDRAM_CORRUPT_SHIFT                                                6
#define REQIDRAM_CORRUPT_MASK                                        0x00000040
#define REQIDRAM_CORRUPT_RD(src)                      (((src) & 0x00000040)>>6)
#define REQIDRAM_CORRUPT_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields M_TRANS_CORRUPT	 */
#define M_TRANS_CORRUPT_WIDTH                                                 1
#define M_TRANS_CORRUPT_SHIFT                                                 5
#define M_TRANS_CORRUPT_MASK                                         0x00000020
#define M_TRANS_CORRUPT_RD(src)                       (((src) & 0x00000020)>>5)
#define M_TRANS_CORRUPT_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields TRANS_CORRUPT	 */
#define TRANS_CORRUPT_WIDTH                                                   1
#define TRANS_CORRUPT_SHIFT                                                   4
#define TRANS_CORRUPT_MASK                                           0x00000010
#define TRANS_CORRUPT_RD(src)                         (((src) & 0x00000010)>>4)
#define TRANS_CORRUPT_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields M_WDATA_CORRUPT	 */
#define M_WDATA_CORRUPT_WIDTH                                                 1
#define M_WDATA_CORRUPT_SHIFT                                                 3
#define M_WDATA_CORRUPT_MASK                                         0x00000008
#define M_WDATA_CORRUPT_RD(src)                       (((src) & 0x00000008)>>3)
#define M_WDATA_CORRUPT_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields WDATA_CORRUPT	 */
#define WDATA_CORRUPT_WIDTH                                                   1
#define WDATA_CORRUPT_SHIFT                                                   2
#define WDATA_CORRUPT_MASK                                           0x00000004
#define WDATA_CORRUPT_RD(src)                         (((src) & 0x00000004)>>2)
#define WDATA_CORRUPT_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields M_RDATA_CORRUPT	 */
#define M_RDATA_CORRUPT_WIDTH                                                 1
#define M_RDATA_CORRUPT_SHIFT                                                 1
#define M_RDATA_CORRUPT_MASK                                         0x00000002
#define M_RDATA_CORRUPT_RD(src)                       (((src) & 0x00000002)>>1)
#define M_RDATA_CORRUPT_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields RDATA_CORRUPT	 */
#define RDATA_CORRUPT_WIDTH                                                   1
#define RDATA_CORRUPT_SHIFT                                                   0
#define RDATA_CORRUPT_MASK                                           0x00000001
#define RDATA_CORRUPT_RD(src)                            (((src) & 0x00000001))
#define RDATA_CORRUPT_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBPATRANSERRINTMSK	*/ 
/*	 Fields M_TRANS_CORRUPT	 */
#define M_TRANS_CORRUPT_F1_WIDTH                                              1
#define M_TRANS_CORRUPT_F1_SHIFT                                              7
#define M_TRANS_CORRUPT_F1_MASK                                      0x00000080
#define M_TRANS_CORRUPT_F1_RD(src)                    (((src) & 0x00000080)>>7)
#define M_TRANS_CORRUPT_F1_WR(src)               (((u32)(src)<<7) & 0x00000080)
#define M_TRANS_CORRUPT_F1_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields TRANS_CORRUPT	 */
#define TRANS_CORRUPT_F1_WIDTH                                                1
#define TRANS_CORRUPT_F1_SHIFT                                                6
#define TRANS_CORRUPT_F1_MASK                                        0x00000040
#define TRANS_CORRUPT_F1_RD(src)                      (((src) & 0x00000040)>>6)
#define TRANS_CORRUPT_F1_WR(src)                 (((u32)(src)<<6) & 0x00000040)
#define TRANS_CORRUPT_F1_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields M_REQIDRAM_CORRUPT	 */
#define M_REQIDRAM_CORRUPT_F1_WIDTH                                           1
#define M_REQIDRAM_CORRUPT_F1_SHIFT                                           5
#define M_REQIDRAM_CORRUPT_F1_MASK                                   0x00000020
#define M_REQIDRAM_CORRUPT_F1_RD(src)                 (((src) & 0x00000020)>>5)
#define M_REQIDRAM_CORRUPT_F1_WR(src)            (((u32)(src)<<5) & 0x00000020)
#define M_REQIDRAM_CORRUPT_F1_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields REQIDRAM_CORRUPT	 */
#define REQIDRAM_CORRUPT_F1_WIDTH                                             1
#define REQIDRAM_CORRUPT_F1_SHIFT                                             4
#define REQIDRAM_CORRUPT_F1_MASK                                     0x00000010
#define REQIDRAM_CORRUPT_F1_RD(src)                   (((src) & 0x00000010)>>4)
#define REQIDRAM_CORRUPT_F1_WR(src)              (((u32)(src)<<4) & 0x00000010)
#define REQIDRAM_CORRUPT_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields M_WDATA_CORRUPT	 */
#define M_WDATA_CORRUPT_F1_WIDTH                                              1
#define M_WDATA_CORRUPT_F1_SHIFT                                              3
#define M_WDATA_CORRUPT_F1_MASK                                      0x00000008
#define M_WDATA_CORRUPT_F1_RD(src)                    (((src) & 0x00000008)>>3)
#define M_WDATA_CORRUPT_F1_WR(src)               (((u32)(src)<<3) & 0x00000008)
#define M_WDATA_CORRUPT_F1_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields WDATA_CORRUPT	 */
#define WDATA_CORRUPT_F1_WIDTH                                                1
#define WDATA_CORRUPT_F1_SHIFT                                                2
#define WDATA_CORRUPT_F1_MASK                                        0x00000004
#define WDATA_CORRUPT_F1_RD(src)                      (((src) & 0x00000004)>>2)
#define WDATA_CORRUPT_F1_WR(src)                 (((u32)(src)<<2) & 0x00000004)
#define WDATA_CORRUPT_F1_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields M_RDATA_CORRUPT	 */
#define M_RDATA_CORRUPT_F1_WIDTH                                              1
#define M_RDATA_CORRUPT_F1_SHIFT                                              1
#define M_RDATA_CORRUPT_F1_MASK                                      0x00000002
#define M_RDATA_CORRUPT_F1_RD(src)                    (((src) & 0x00000002)>>1)
#define M_RDATA_CORRUPT_F1_WR(src)               (((u32)(src)<<1) & 0x00000002)
#define M_RDATA_CORRUPT_F1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields RDATA_CORRUPT	 */
#define RDATA_CORRUPT_F1_WIDTH                                                1
#define RDATA_CORRUPT_F1_SHIFT                                                0
#define RDATA_CORRUPT_F1_MASK                                        0x00000001
#define RDATA_CORRUPT_F1_RD(src)                         (((src) & 0x00000001))
#define RDATA_CORRUPT_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define RDATA_CORRUPT_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBBATRANSERRINTSTS	*/ 
/*	 Fields M_ILLEGAL_ACCESS	 */
#define M_ILLEGAL_ACCESS_F4_WIDTH                                             1
#define M_ILLEGAL_ACCESS_F4_SHIFT                                            15
#define M_ILLEGAL_ACCESS_F4_MASK                                     0x00008000
#define M_ILLEGAL_ACCESS_F4_RD(src)                  (((src) & 0x00008000)>>15)
#define M_ILLEGAL_ACCESS_F4_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields ILLEGAL_ACCESS	 */
#define ILLEGAL_ACCESS_F4_WIDTH                                               1
#define ILLEGAL_ACCESS_F4_SHIFT                                              14
#define ILLEGAL_ACCESS_F4_MASK                                       0x00004000
#define ILLEGAL_ACCESS_F4_RD(src)                    (((src) & 0x00004000)>>14)
#define ILLEGAL_ACCESS_F4_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields M_WIDRAM_CORRUPT	 */
#define M_WIDRAM_CORRUPT_WIDTH                                                1
#define M_WIDRAM_CORRUPT_SHIFT                                               13
#define M_WIDRAM_CORRUPT_MASK                                        0x00002000
#define M_WIDRAM_CORRUPT_RD(src)                     (((src) & 0x00002000)>>13)
#define M_WIDRAM_CORRUPT_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields WIDRAM_CORRUPT	 */
#define WIDRAM_CORRUPT_WIDTH                                                  1
#define WIDRAM_CORRUPT_SHIFT                                                 12
#define WIDRAM_CORRUPT_MASK                                          0x00001000
#define WIDRAM_CORRUPT_RD(src)                       (((src) & 0x00001000)>>12)
#define WIDRAM_CORRUPT_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields M_RIDRAM_CORRUPT	 */
#define M_RIDRAM_CORRUPT_WIDTH                                                1
#define M_RIDRAM_CORRUPT_SHIFT                                               11
#define M_RIDRAM_CORRUPT_MASK                                        0x00000800
#define M_RIDRAM_CORRUPT_RD(src)                     (((src) & 0x00000800)>>11)
#define M_RIDRAM_CORRUPT_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields RIDRAM_CORRUPT	 */
#define RIDRAM_CORRUPT_WIDTH                                                  1
#define RIDRAM_CORRUPT_SHIFT                                                 10
#define RIDRAM_CORRUPT_MASK                                          0x00000400
#define RIDRAM_CORRUPT_RD(src)                       (((src) & 0x00000400)>>10)
#define RIDRAM_CORRUPT_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields M_TRANS_CORRUPT	 */
#define M_TRANS_CORRUPT_F2_WIDTH                                              1
#define M_TRANS_CORRUPT_F2_SHIFT                                              9
#define M_TRANS_CORRUPT_F2_MASK                                      0x00000200
#define M_TRANS_CORRUPT_F2_RD(src)                    (((src) & 0x00000200)>>9)
#define M_TRANS_CORRUPT_F2_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields TRANS_CORRUPT	 */
#define TRANS_CORRUPT_F2_WIDTH                                                1
#define TRANS_CORRUPT_F2_SHIFT                                                8
#define TRANS_CORRUPT_F2_MASK                                        0x00000100
#define TRANS_CORRUPT_F2_RD(src)                      (((src) & 0x00000100)>>8)
#define TRANS_CORRUPT_F2_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields M_WDATA_CORRUPT	 */
#define M_WDATA_CORRUPT_F2_WIDTH                                              1
#define M_WDATA_CORRUPT_F2_SHIFT                                              7
#define M_WDATA_CORRUPT_F2_MASK                                      0x00000080
#define M_WDATA_CORRUPT_F2_RD(src)                    (((src) & 0x00000080)>>7)
#define M_WDATA_CORRUPT_F2_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields WDATA_CORRUPT	 */
#define WDATA_CORRUPT_F2_WIDTH                                                1
#define WDATA_CORRUPT_F2_SHIFT                                                6
#define WDATA_CORRUPT_F2_MASK                                        0x00000040
#define WDATA_CORRUPT_F2_RD(src)                      (((src) & 0x00000040)>>6)
#define WDATA_CORRUPT_F2_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields M_RBM_POISONED_REQ	 */
#define M_RBM_POISONED_REQ_WIDTH                                              1
#define M_RBM_POISONED_REQ_SHIFT                                              5
#define M_RBM_POISONED_REQ_MASK                                      0x00000020
#define M_RBM_POISONED_REQ_RD(src)                    (((src) & 0x00000020)>>5)
#define M_RBM_POISONED_REQ_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields RBM_POISONED_REQ	 */
#define RBM_POISONED_REQ_WIDTH                                                1
#define RBM_POISONED_REQ_SHIFT                                                4
#define RBM_POISONED_REQ_MASK                                        0x00000010
#define RBM_POISONED_REQ_RD(src)                      (((src) & 0x00000010)>>4)
#define RBM_POISONED_REQ_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields M_XGIC_POISONED_REQ	 */
#define M_XGIC_POISONED_REQ_WIDTH                                             1
#define M_XGIC_POISONED_REQ_SHIFT                                             3
#define M_XGIC_POISONED_REQ_MASK                                     0x00000008
#define M_XGIC_POISONED_REQ_RD(src)                   (((src) & 0x00000008)>>3)
#define M_XGIC_POISONED_REQ_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields XGIC_POISONED_REQ	 */
#define XGIC_POISONED_REQ_WIDTH                                               1
#define XGIC_POISONED_REQ_SHIFT                                               2
#define XGIC_POISONED_REQ_MASK                                       0x00000004
#define XGIC_POISONED_REQ_RD(src)                     (((src) & 0x00000004)>>2)
#define XGIC_POISONED_REQ_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields M_WRERR_RESP	 */
#define M_WRERR_RESP_WIDTH                                                    1
#define M_WRERR_RESP_SHIFT                                                    1
#define M_WRERR_RESP_MASK                                            0x00000002
#define M_WRERR_RESP_RD(src)                          (((src) & 0x00000002)>>1)
#define M_WRERR_RESP_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields WRERR_RESP	 */
#define WRERR_RESP_WIDTH                                                      1
#define WRERR_RESP_SHIFT                                                      0
#define WRERR_RESP_MASK                                              0x00000001
#define WRERR_RESP_RD(src)                               (((src) & 0x00000001))
#define WRERR_RESP_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBBATRANSERRINTMSK	*/ 
/*	 Fields M_ILLEGAL_ACCESS	 */
#define M_ILLEGAL_ACCESS_F5_WIDTH                                             1
#define M_ILLEGAL_ACCESS_F5_SHIFT                                            15
#define M_ILLEGAL_ACCESS_F5_MASK                                     0x00008000
#define M_ILLEGAL_ACCESS_F5_RD(src)                  (((src) & 0x00008000)>>15)
#define M_ILLEGAL_ACCESS_F5_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define M_ILLEGAL_ACCESS_F5_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields ILLEGAL_ACCESS	 */
#define ILLEGAL_ACCESS_F5_WIDTH                                               1
#define ILLEGAL_ACCESS_F5_SHIFT                                              14
#define ILLEGAL_ACCESS_F5_MASK                                       0x00004000
#define ILLEGAL_ACCESS_F5_RD(src)                    (((src) & 0x00004000)>>14)
#define ILLEGAL_ACCESS_F5_WR(src)               (((u32)(src)<<14) & 0x00004000)
#define ILLEGAL_ACCESS_F5_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields M_WIDRAM_CORRUPT	 */
#define M_WIDRAM_CORRUPT_F1_WIDTH                                             1
#define M_WIDRAM_CORRUPT_F1_SHIFT                                            13
#define M_WIDRAM_CORRUPT_F1_MASK                                     0x00002000
#define M_WIDRAM_CORRUPT_F1_RD(src)                  (((src) & 0x00002000)>>13)
#define M_WIDRAM_CORRUPT_F1_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define M_WIDRAM_CORRUPT_F1_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields WIDRAM_CORRUPT	 */
#define WIDRAM_CORRUPT_F1_WIDTH                                               1
#define WIDRAM_CORRUPT_F1_SHIFT                                              12
#define WIDRAM_CORRUPT_F1_MASK                                       0x00001000
#define WIDRAM_CORRUPT_F1_RD(src)                    (((src) & 0x00001000)>>12)
#define WIDRAM_CORRUPT_F1_WR(src)               (((u32)(src)<<12) & 0x00001000)
#define WIDRAM_CORRUPT_F1_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields M_RIDRAM_CORRUPT	 */
#define M_RIDRAM_CORRUPT_F1_WIDTH                                             1
#define M_RIDRAM_CORRUPT_F1_SHIFT                                            11
#define M_RIDRAM_CORRUPT_F1_MASK                                     0x00000800
#define M_RIDRAM_CORRUPT_F1_RD(src)                  (((src) & 0x00000800)>>11)
#define M_RIDRAM_CORRUPT_F1_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define M_RIDRAM_CORRUPT_F1_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields RIDRAM_CORRUPT	 */
#define RIDRAM_CORRUPT_F1_WIDTH                                               1
#define RIDRAM_CORRUPT_F1_SHIFT                                              10
#define RIDRAM_CORRUPT_F1_MASK                                       0x00000400
#define RIDRAM_CORRUPT_F1_RD(src)                    (((src) & 0x00000400)>>10)
#define RIDRAM_CORRUPT_F1_WR(src)               (((u32)(src)<<10) & 0x00000400)
#define RIDRAM_CORRUPT_F1_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields M_TRANS_CORRUPT	 */
#define M_TRANS_CORRUPT_F3_WIDTH                                              1
#define M_TRANS_CORRUPT_F3_SHIFT                                              9
#define M_TRANS_CORRUPT_F3_MASK                                      0x00000200
#define M_TRANS_CORRUPT_F3_RD(src)                    (((src) & 0x00000200)>>9)
#define M_TRANS_CORRUPT_F3_WR(src)               (((u32)(src)<<9) & 0x00000200)
#define M_TRANS_CORRUPT_F3_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields TRANS_CORRUPT	 */
#define TRANS_CORRUPT_F3_WIDTH                                                1
#define TRANS_CORRUPT_F3_SHIFT                                                8
#define TRANS_CORRUPT_F3_MASK                                        0x00000100
#define TRANS_CORRUPT_F3_RD(src)                      (((src) & 0x00000100)>>8)
#define TRANS_CORRUPT_F3_WR(src)                 (((u32)(src)<<8) & 0x00000100)
#define TRANS_CORRUPT_F3_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields M_WDATA_CORRUPT	 */
#define M_WDATA_CORRUPT_F3_WIDTH                                              1
#define M_WDATA_CORRUPT_F3_SHIFT                                              7
#define M_WDATA_CORRUPT_F3_MASK                                      0x00000080
#define M_WDATA_CORRUPT_F3_RD(src)                    (((src) & 0x00000080)>>7)
#define M_WDATA_CORRUPT_F3_WR(src)               (((u32)(src)<<7) & 0x00000080)
#define M_WDATA_CORRUPT_F3_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields WDATA_CORRUPT	 */
#define WDATA_CORRUPT_F3_WIDTH                                                1
#define WDATA_CORRUPT_F3_SHIFT                                                6
#define WDATA_CORRUPT_F3_MASK                                        0x00000040
#define WDATA_CORRUPT_F3_RD(src)                      (((src) & 0x00000040)>>6)
#define WDATA_CORRUPT_F3_WR(src)                 (((u32)(src)<<6) & 0x00000040)
#define WDATA_CORRUPT_F3_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields M_RBM_POISONED_REQ	 */
#define M_RBM_POISONED_REQ_F1_WIDTH                                           1
#define M_RBM_POISONED_REQ_F1_SHIFT                                           5
#define M_RBM_POISONED_REQ_F1_MASK                                   0x00000020
#define M_RBM_POISONED_REQ_F1_RD(src)                 (((src) & 0x00000020)>>5)
#define M_RBM_POISONED_REQ_F1_WR(src)            (((u32)(src)<<5) & 0x00000020)
#define M_RBM_POISONED_REQ_F1_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields RBM_POISONED_REQ	 */
#define RBM_POISONED_REQ_F1_WIDTH                                             1
#define RBM_POISONED_REQ_F1_SHIFT                                             4
#define RBM_POISONED_REQ_F1_MASK                                     0x00000010
#define RBM_POISONED_REQ_F1_RD(src)                   (((src) & 0x00000010)>>4)
#define RBM_POISONED_REQ_F1_WR(src)              (((u32)(src)<<4) & 0x00000010)
#define RBM_POISONED_REQ_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields M_XGIC_POISONED_REQ	 */
#define M_XGIC_POISONED_REQ_F1_WIDTH                                          1
#define M_XGIC_POISONED_REQ_F1_SHIFT                                          3
#define M_XGIC_POISONED_REQ_F1_MASK                                  0x00000008
#define M_XGIC_POISONED_REQ_F1_RD(src)                (((src) & 0x00000008)>>3)
#define M_XGIC_POISONED_REQ_F1_WR(src)           (((u32)(src)<<3) & 0x00000008)
#define M_XGIC_POISONED_REQ_F1_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields XGIC_POISONED_REQ	 */
#define XGIC_POISONED_REQ_F1_WIDTH                                            1
#define XGIC_POISONED_REQ_F1_SHIFT                                            2
#define XGIC_POISONED_REQ_F1_MASK                                    0x00000004
#define XGIC_POISONED_REQ_F1_RD(src)                  (((src) & 0x00000004)>>2)
#define XGIC_POISONED_REQ_F1_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define XGIC_POISONED_REQ_F1_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields M_WRERR_RESP	 */
#define M_WRERR_RESP_F1_WIDTH                                                 1
#define M_WRERR_RESP_F1_SHIFT                                                 1
#define M_WRERR_RESP_F1_MASK                                         0x00000002
#define M_WRERR_RESP_F1_RD(src)                       (((src) & 0x00000002)>>1)
#define M_WRERR_RESP_F1_WR(src)                  (((u32)(src)<<1) & 0x00000002)
#define M_WRERR_RESP_F1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields WRERR_RESP	 */
#define WRERR_RESP_F1_WIDTH                                                   1
#define WRERR_RESP_F1_SHIFT                                                   0
#define WRERR_RESP_F1_MASK                                           0x00000001
#define WRERR_RESP_F1_RD(src)                            (((src) & 0x00000001))
#define WRERR_RESP_F1_WR(src)                       (((u32)(src)) & 0x00000001)
#define WRERR_RESP_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBBATRANSERRREQINFOL	*/ 
/*	 Fields ERRADDRL	 */
#define ERRADDRL_F2_WIDTH                                                    32
#define ERRADDRL_F2_SHIFT                                                     0
#define ERRADDRL_F2_MASK                                             0xffffffff
#define ERRADDRL_F2_RD(src)                              (((src) & 0xffffffff))
#define ERRADDRL_F2_WR(src)                         (((u32)(src)) & 0xffffffff)
#define ERRADDRL_F2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IOBBATRANSERRREQINFOH	*/ 
/*	 Fields ERRADDRH	 */
#define ERRADDRH_F2_WIDTH                                                    10
#define ERRADDRH_F2_SHIFT                                                    22
#define ERRADDRH_F2_MASK                                             0xffc00000
#define ERRADDRH_F2_RD(src)                          (((src) & 0xffc00000)>>22)
#define ERRADDRH_F2_WR(src)                     (((u32)(src)<<22) & 0xffc00000)
#define ERRADDRH_F2_SET(dst,src) \
                      (((dst) & ~0xffc00000) | (((u32)(src)<<22) & 0xffc00000))
/*	 Fields AUXINFO	 */
#define IOB_AUXINFO_F2_WIDTH                                                      2
#define IOB_AUXINFO_F2_SHIFT                                                     14
#define IOB_AUXINFO_F2_MASK                                              0x0000c000
#define IOB_AUXINFO_F2_RD(src)                           (((src) & 0x0000c000)>>14)
#define IOB_AUXINFO_F2_WR(src)                      (((u32)(src)<<14) & 0x0000c000)
#define IOB_AUXINFO_F2_SET(dst,src) \
                      (((dst) & ~0x0000c000) | (((u32)(src)<<14) & 0x0000c000))
/*	 Fields REQSIZE	 */
#define IOBBATRANSERRREQINFOH_REQSIZE_F2_WIDTH                                                      6
#define IOBBATRANSERRREQINFOH_REQSIZE_F2_SHIFT                                                      1
#define IOBBATRANSERRREQINFOH_REQSIZE_F2_MASK                                              0x0000007e
#define IOBBATRANSERRREQINFOH_REQSIZE_F2_RD(src)                            (((src) & 0x0000007e)>>1)
#define IOBBATRANSERRREQINFOH_REQSIZE_F2_WR(src)                       (((u32)(src)<<1) & 0x0000007e)
#define IOBBATRANSERRREQINFOH_REQSIZE_F2_SET(dst,src) \
                       (((dst) & ~0x0000007e) | (((u32)(src)<<1) & 0x0000007e))
/*	 Fields REQTYPE	 */
#define REQTYPE_F2_WIDTH                                                      1
#define REQTYPE_F2_SHIFT                                                      0
#define REQTYPE_F2_MASK                                              0x00000001
#define REQTYPE_F2_RD(src)                               (((src) & 0x00000001))
#define REQTYPE_F2_WR(src)                          (((u32)(src)) & 0x00000001)
#define REQTYPE_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IOBBATRANSERRCSWREQID	*/ 
/*	 Fields CSWREQID	 */
#define CSWREQID_WIDTH                                                       16
#define CSWREQID_SHIFT                                                        0
#define CSWREQID_MASK                                                0x0000ffff
#define CSWREQID_RD(src)                                 (((src) & 0x0000ffff))
#define CSWREQID_WR(src)                            (((u32)(src)) & 0x0000ffff)
#define CSWREQID_SET(dst,src) \
                          (((dst) & ~0x0000ffff) | (((u32)(src)) & 0x0000ffff))

/*	Register XGICTRANSERRINTSTS	*/ 
/*	 Fields M_WR_ACCESS_ERR	 */
#define M_WR_ACCESS_ERR_WIDTH                                                 1
#define M_WR_ACCESS_ERR_SHIFT                                                 3
#define M_WR_ACCESS_ERR_MASK                                         0x00000008
#define M_WR_ACCESS_ERR_RD(src)                       (((src) & 0x00000008)>>3)
#define M_WR_ACCESS_ERR_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields WR_ACCESS_ERR	 */
#define WR_ACCESS_ERR_WIDTH                                                   1
#define WR_ACCESS_ERR_SHIFT                                                   2
#define WR_ACCESS_ERR_MASK                                           0x00000004
#define WR_ACCESS_ERR_RD(src)                         (((src) & 0x00000004)>>2)
#define WR_ACCESS_ERR_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields M_RD_ACCESS_ERR	 */
#define M_RD_ACCESS_ERR_WIDTH                                                 1
#define M_RD_ACCESS_ERR_SHIFT                                                 1
#define M_RD_ACCESS_ERR_MASK                                         0x00000002
#define M_RD_ACCESS_ERR_RD(src)                       (((src) & 0x00000002)>>1)
#define M_RD_ACCESS_ERR_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields RD_ACCESS_ERR	 */
#define RD_ACCESS_ERR_WIDTH                                                   1
#define RD_ACCESS_ERR_SHIFT                                                   0
#define RD_ACCESS_ERR_MASK                                           0x00000001
#define RD_ACCESS_ERR_RD(src)                            (((src) & 0x00000001))
#define RD_ACCESS_ERR_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register XGICTRANSERRINTMSK	*/ 
/*	 Fields M_WR_ACCESS_ERR	 */
#define M_WR_ACCESS_ERR_F1_WIDTH                                              1
#define M_WR_ACCESS_ERR_F1_SHIFT                                              3
#define M_WR_ACCESS_ERR_F1_MASK                                      0x00000008
#define M_WR_ACCESS_ERR_F1_RD(src)                    (((src) & 0x00000008)>>3)
#define M_WR_ACCESS_ERR_F1_WR(src)               (((u32)(src)<<3) & 0x00000008)
#define M_WR_ACCESS_ERR_F1_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields WR_ACCESS_ERR	 */
#define WR_ACCESS_ERR_F1_WIDTH                                                1
#define WR_ACCESS_ERR_F1_SHIFT                                                2
#define WR_ACCESS_ERR_F1_MASK                                        0x00000004
#define WR_ACCESS_ERR_F1_RD(src)                      (((src) & 0x00000004)>>2)
#define WR_ACCESS_ERR_F1_WR(src)                 (((u32)(src)<<2) & 0x00000004)
#define WR_ACCESS_ERR_F1_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields M_RD_ACCESS_ERR	 */
#define M_RD_ACCESS_ERR_F1_WIDTH                                              1
#define M_RD_ACCESS_ERR_F1_SHIFT                                              1
#define M_RD_ACCESS_ERR_F1_MASK                                      0x00000002
#define M_RD_ACCESS_ERR_F1_RD(src)                    (((src) & 0x00000002)>>1)
#define M_RD_ACCESS_ERR_F1_WR(src)               (((u32)(src)<<1) & 0x00000002)
#define M_RD_ACCESS_ERR_F1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields RD_ACCESS_ERR	 */
#define RD_ACCESS_ERR_F1_WIDTH                                                1
#define RD_ACCESS_ERR_F1_SHIFT                                                0
#define RD_ACCESS_ERR_F1_MASK                                        0x00000001
#define RD_ACCESS_ERR_F1_RD(src)                         (((src) & 0x00000001))
#define RD_ACCESS_ERR_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define RD_ACCESS_ERR_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register XGICTRANSERRREQINFO	*/ 
/*	 Fields REQWBYTE	 */
#define REQWBYTE_WIDTH                                                        2
#define REQWBYTE_SHIFT                                                       30
#define REQWBYTE_MASK                                                0xc0000000
#define REQWBYTE_RD(src)                             (((src) & 0xc0000000)>>30)
#define REQWBYTE_WR(src)                        (((u32)(src)<<30) & 0xc0000000)
#define REQWBYTE_SET(dst,src) \
                      (((dst) & ~0xc0000000) | (((u32)(src)<<30) & 0xc0000000))
/*	 Fields REQSIZE	 */
#define REQSIZE_F3_WIDTH                                                      3
#define REQSIZE_F3_SHIFT                                                     27
#define REQSIZE_F3_MASK                                              0x38000000
#define REQSIZE_F3_RD(src)                           (((src) & 0x38000000)>>27)
#define REQSIZE_F3_WR(src)                      (((u32)(src)<<27) & 0x38000000)
#define REQSIZE_F3_SET(dst,src) \
                      (((dst) & ~0x38000000) | (((u32)(src)<<27) & 0x38000000))
/*	 Fields REQTYPE	 */
#define REQTYPE_F3_WIDTH                                                      1
#define REQTYPE_F3_SHIFT                                                     26
#define REQTYPE_F3_MASK                                              0x04000000
#define REQTYPE_F3_RD(src)                           (((src) & 0x04000000)>>26)
#define REQTYPE_F3_WR(src)                      (((u32)(src)<<26) & 0x04000000)
#define REQTYPE_F3_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields ERRADDR	 */
#define ERRADDR_WIDTH                                                        26
#define ERRADDR_SHIFT                                                         0
#define ERRADDR_MASK                                                 0x03ffffff
#define ERRADDR_RD(src)                                  (((src) & 0x03ffffff))
#define ERRADDR_WR(src)                             (((u32)(src)) & 0x03ffffff)
#define ERRADDR_SET(dst,src) \
                          (((dst) & ~0x03ffffff) | (((u32)(src)) & 0x03ffffff))

/*	Register IOBERRRESP	*/ 

/*	Global Base Address	*/
#define GLBL_ERR_CSR_BASE_ADDR			0x07e930000ULL

/*    Address GLBL_ERR_CSR  Registers */
#define GLBL_ERR_STS_ADDR                                            0x00000800
#define GLBL_ERR_STS_DEFAULT                                         0x00000000
#define GLBL_SEC_ERRL_ADDR                                           0x00000810
#define GLBL_SEC_ERRL_DEFAULT                                        0x00000000
#define GLBL_SEC_ERRLMASK_ADDR                                       0x00000814
#define GLBL_SEC_ERRH_ADDR                                           0x00000818
#define GLBL_SEC_ERRH_DEFAULT                                        0x00000000
#define GLBL_SEC_ERRHMASK_ADDR                                       0x0000081c
#define GLBL_MSEC_ERRL_ADDR                                          0x00000820
#define GLBL_MSEC_ERRL_DEFAULT                                       0x00000000
#define GLBL_MSEC_ERRLMASK_ADDR                                      0x00000824
#define GLBL_MSEC_ERRH_ADDR                                          0x00000828
#define GLBL_MSEC_ERRH_DEFAULT                                       0x00000000
#define GLBL_MSEC_ERRHMASK_ADDR                                      0x0000082c
#define GLBL_DED_ERRL_ADDR                                           0x00000830
#define GLBL_DED_ERRL_DEFAULT                                        0x00000000
#define GLBL_DED_ERRLMASK_ADDR                                       0x00000834
#define GLBL_DED_ERRH_ADDR                                           0x00000838
#define GLBL_DED_ERRH_DEFAULT                                        0x00000000
#define GLBL_DED_ERRHMASK_ADDR                                       0x0000083c
#define GLBL_MDED_ERRL_ADDR                                          0x00000840
#define GLBL_MDED_ERRL_DEFAULT                                       0x00000000
#define GLBL_MDED_ERRLMASK_ADDR                                      0x00000844
#define GLBL_MDED_ERRH_ADDR                                          0x00000848
#define GLBL_MDED_ERRH_DEFAULT                                       0x00000000
#define GLBL_MDED_ERRHMASK_ADDR                                      0x0000084c
#define GLBL_MERR_ADDR_ADDR                                          0x00000850
#define GLBL_MERR_ADDR_DEFAULT                                       0x00000000
#define GLBL_MERR_REQINFO_ADDR                                       0x00000854
#define GLBL_MERR_REQINFO_DEFAULT                                    0x00000000
#define GLBL_TRANS_ERR_ADDR                                          0x00000860
#define GLBL_TRANS_ERR_DEFAULT                                       0x00000000
#define GLBL_TRANS_ERRMASK_ADDR                                      0x00000864
#define GLBL_WDERR_ADDR_ADDR                                         0x00000870
#define GLBL_WDERR_ADDR_DEFAULT                                      0x00000000
#define GLBL_WDERR_REQINFO_ADDR                                      0x00000874
#define GLBL_WDERR_REQINFO_DEFAULT                                   0x00000000
#define GLBL_DEVERR_ADDR_ADDR                                        0x00000878
#define GLBL_DEVERR_ADDR_DEFAULT                                     0x00000000
#define GLBL_DEVERR_REQINFO_ADDR                                     0x0000087c
#define GLBL_DEVERR_REQINFO_DEFAULT                                  0x00000000
#define GLBL_SEC_ERRL_ALS_ADDR                                       0x00000880
#define GLBL_SEC_ERRL_ALS_DEFAULT                                    0x00000000
#define GLBL_SEC_ERRH_ALS_ADDR                                       0x00000884
#define GLBL_SEC_ERRH_ALS_DEFAULT                                    0x00000000
#define GLBL_DED_ERRL_ALS_ADDR                                       0x00000888
#define GLBL_DED_ERRL_ALS_DEFAULT                                    0x00000000
#define GLBL_DED_ERRH_ALS_ADDR                                       0x0000088c
#define GLBL_DED_ERRH_ALS_DEFAULT                                    0x00000000
#define GLBL_TRANS_ERR_ALS_ADDR                                      0x00000890
#define GLBL_TRANS_ERR_ALS_DEFAULT                                   0x00000000

/*	Register GLBL_ERR_STS	*/ 
/*	 Fields SHIM_ERR	 */
#define SHIM_ERR_WIDTH                                                        1
#define SHIM_ERR_SHIFT                                                        5
#define SHIM_ERR_MASK                                                0x00000020
#define SHIM_ERR_RD(src)                              (((src) & 0x00000020)>>5)
#define SHIM_ERR_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields TRANS_ERR	 */
#define TRANS_ERR_WIDTH                                                       1
#define TRANS_ERR_SHIFT                                                       4
#define TRANS_ERR_MASK                                               0x00000010
#define TRANS_ERR_RD(src)                             (((src) & 0x00000010)>>4)
#define TRANS_ERR_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields MDED_ERR	 */
#define MDED_ERR_WIDTH                                                        1
#define MDED_ERR_SHIFT                                                        3
#define MDED_ERR_MASK                                                0x00000008
#define MDED_ERR_RD(src)                              (((src) & 0x00000008)>>3)
#define MDED_ERR_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields DED_ERR	 */
#define DED_ERR_WIDTH                                                         1
#define DED_ERR_SHIFT                                                         2
#define DED_ERR_MASK                                                 0x00000004
#define DED_ERR_RD(src)                               (((src) & 0x00000004)>>2)
#define DED_ERR_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields MSEC_ERR	 */
#define MSEC_ERR_WIDTH                                                        1
#define MSEC_ERR_SHIFT                                                        1
#define MSEC_ERR_MASK                                                0x00000002
#define MSEC_ERR_RD(src)                              (((src) & 0x00000002)>>1)
#define MSEC_ERR_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC_ERR	 */
#define SEC_ERR_WIDTH                                                         1
#define SEC_ERR_SHIFT                                                         0
#define SEC_ERR_MASK                                                 0x00000001
#define SEC_ERR_RD(src)                                  (((src) & 0x00000001))
#define SEC_ERR_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_SEC_ERRL	*/ 
/*	 Fields SEC31	 */
#define SEC31_WIDTH                                                           1
#define SEC31_SHIFT                                                          31
#define SEC31_MASK                                                   0x80000000
#define SEC31_RD(src)                                (((src) & 0x80000000)>>31)
#define SEC31_WR(src)                           (((u32)(src)<<31) & 0x80000000)
#define SEC31_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields SEC30	 */
#define SEC30_WIDTH                                                           1
#define SEC30_SHIFT                                                          30
#define SEC30_MASK                                                   0x40000000
#define SEC30_RD(src)                                (((src) & 0x40000000)>>30)
#define SEC30_WR(src)                           (((u32)(src)<<30) & 0x40000000)
#define SEC30_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields SEC29	 */
#define SEC29_WIDTH                                                           1
#define SEC29_SHIFT                                                          29
#define SEC29_MASK                                                   0x20000000
#define SEC29_RD(src)                                (((src) & 0x20000000)>>29)
#define SEC29_WR(src)                           (((u32)(src)<<29) & 0x20000000)
#define SEC29_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields SEC28	 */
#define SEC28_WIDTH                                                           1
#define SEC28_SHIFT                                                          28
#define SEC28_MASK                                                   0x10000000
#define SEC28_RD(src)                                (((src) & 0x10000000)>>28)
#define SEC28_WR(src)                           (((u32)(src)<<28) & 0x10000000)
#define SEC28_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields SEC27	 */
#define SEC27_WIDTH                                                           1
#define SEC27_SHIFT                                                          27
#define SEC27_MASK                                                   0x08000000
#define SEC27_RD(src)                                (((src) & 0x08000000)>>27)
#define SEC27_WR(src)                           (((u32)(src)<<27) & 0x08000000)
#define SEC27_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields SEC26	 */
#define SEC26_WIDTH                                                           1
#define SEC26_SHIFT                                                          26
#define SEC26_MASK                                                   0x04000000
#define SEC26_RD(src)                                (((src) & 0x04000000)>>26)
#define SEC26_WR(src)                           (((u32)(src)<<26) & 0x04000000)
#define SEC26_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields SEC25	 */
#define SEC25_WIDTH                                                           1
#define SEC25_SHIFT                                                          25
#define SEC25_MASK                                                   0x02000000
#define SEC25_RD(src)                                (((src) & 0x02000000)>>25)
#define SEC25_WR(src)                           (((u32)(src)<<25) & 0x02000000)
#define SEC25_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields SEC24	 */
#define SEC24_WIDTH                                                           1
#define SEC24_SHIFT                                                          24
#define SEC24_MASK                                                   0x01000000
#define SEC24_RD(src)                                (((src) & 0x01000000)>>24)
#define SEC24_WR(src)                           (((u32)(src)<<24) & 0x01000000)
#define SEC24_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*	 Fields SEC23	 */
#define SEC23_WIDTH                                                           1
#define SEC23_SHIFT                                                          23
#define SEC23_MASK                                                   0x00800000
#define SEC23_RD(src)                                (((src) & 0x00800000)>>23)
#define SEC23_WR(src)                           (((u32)(src)<<23) & 0x00800000)
#define SEC23_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields SEC22	 */
#define SEC22_WIDTH                                                           1
#define SEC22_SHIFT                                                          22
#define SEC22_MASK                                                   0x00400000
#define SEC22_RD(src)                                (((src) & 0x00400000)>>22)
#define SEC22_WR(src)                           (((u32)(src)<<22) & 0x00400000)
#define SEC22_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields SEC21	 */
#define SEC21_WIDTH                                                           1
#define SEC21_SHIFT                                                          21
#define SEC21_MASK                                                   0x00200000
#define SEC21_RD(src)                                (((src) & 0x00200000)>>21)
#define SEC21_WR(src)                           (((u32)(src)<<21) & 0x00200000)
#define SEC21_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields SEC20	 */
#define SEC20_WIDTH                                                           1
#define SEC20_SHIFT                                                          20
#define SEC20_MASK                                                   0x00100000
#define SEC20_RD(src)                                (((src) & 0x00100000)>>20)
#define SEC20_WR(src)                           (((u32)(src)<<20) & 0x00100000)
#define SEC20_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields SEC19	 */
#define SEC19_WIDTH                                                           1
#define SEC19_SHIFT                                                          19
#define SEC19_MASK                                                   0x00080000
#define SEC19_RD(src)                                (((src) & 0x00080000)>>19)
#define SEC19_WR(src)                           (((u32)(src)<<19) & 0x00080000)
#define SEC19_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields SEC18	 */
#define SEC18_WIDTH                                                           1
#define SEC18_SHIFT                                                          18
#define SEC18_MASK                                                   0x00040000
#define SEC18_RD(src)                                (((src) & 0x00040000)>>18)
#define SEC18_WR(src)                           (((u32)(src)<<18) & 0x00040000)
#define SEC18_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields SEC17	 */
#define SEC17_WIDTH                                                           1
#define SEC17_SHIFT                                                          17
#define SEC17_MASK                                                   0x00020000
#define SEC17_RD(src)                                (((src) & 0x00020000)>>17)
#define SEC17_WR(src)                           (((u32)(src)<<17) & 0x00020000)
#define SEC17_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields SEC16	 */
#define SEC16_WIDTH                                                           1
#define SEC16_SHIFT                                                          16
#define SEC16_MASK                                                   0x00010000
#define SEC16_RD(src)                                (((src) & 0x00010000)>>16)
#define SEC16_WR(src)                           (((u32)(src)<<16) & 0x00010000)
#define SEC16_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields SEC15	 */
#define SEC15_WIDTH                                                           1
#define SEC15_SHIFT                                                          15
#define SEC15_MASK                                                   0x00008000
#define SEC15_RD(src)                                (((src) & 0x00008000)>>15)
#define SEC15_WR(src)                           (((u32)(src)<<15) & 0x00008000)
#define SEC15_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields SEC14	 */
#define SEC14_WIDTH                                                           1
#define SEC14_SHIFT                                                          14
#define SEC14_MASK                                                   0x00004000
#define SEC14_RD(src)                                (((src) & 0x00004000)>>14)
#define SEC14_WR(src)                           (((u32)(src)<<14) & 0x00004000)
#define SEC14_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields SEC13	 */
#define SEC13_WIDTH                                                           1
#define SEC13_SHIFT                                                          13
#define SEC13_MASK                                                   0x00002000
#define SEC13_RD(src)                                (((src) & 0x00002000)>>13)
#define SEC13_WR(src)                           (((u32)(src)<<13) & 0x00002000)
#define SEC13_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields SEC12	 */
#define SEC12_WIDTH                                                           1
#define SEC12_SHIFT                                                          12
#define SEC12_MASK                                                   0x00001000
#define SEC12_RD(src)                                (((src) & 0x00001000)>>12)
#define SEC12_WR(src)                           (((u32)(src)<<12) & 0x00001000)
#define SEC12_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields SEC11	 */
#define SEC11_WIDTH                                                           1
#define SEC11_SHIFT                                                          11
#define SEC11_MASK                                                   0x00000800
#define SEC11_RD(src)                                (((src) & 0x00000800)>>11)
#define SEC11_WR(src)                           (((u32)(src)<<11) & 0x00000800)
#define SEC11_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields SEC10	 */
#define SEC10_WIDTH                                                           1
#define SEC10_SHIFT                                                          10
#define SEC10_MASK                                                   0x00000400
#define SEC10_RD(src)                                (((src) & 0x00000400)>>10)
#define SEC10_WR(src)                           (((u32)(src)<<10) & 0x00000400)
#define SEC10_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields SEC9	 */
#define SEC9_WIDTH                                                            1
#define SEC9_SHIFT                                                            9
#define SEC9_MASK                                                    0x00000200
#define SEC9_RD(src)                                  (((src) & 0x00000200)>>9)
#define SEC9_WR(src)                             (((u32)(src)<<9) & 0x00000200)
#define SEC9_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields SEC8	 */
#define SEC8_WIDTH                                                            1
#define SEC8_SHIFT                                                            8
#define SEC8_MASK                                                    0x00000100
#define SEC8_RD(src)                                  (((src) & 0x00000100)>>8)
#define SEC8_WR(src)                             (((u32)(src)<<8) & 0x00000100)
#define SEC8_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields SEC7	 */
#define SEC7_WIDTH                                                            1
#define SEC7_SHIFT                                                            7
#define SEC7_MASK                                                    0x00000080
#define SEC7_RD(src)                                  (((src) & 0x00000080)>>7)
#define SEC7_WR(src)                             (((u32)(src)<<7) & 0x00000080)
#define SEC7_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields SEC6	 */
#define SEC6_WIDTH                                                            1
#define SEC6_SHIFT                                                            6
#define SEC6_MASK                                                    0x00000040
#define SEC6_RD(src)                                  (((src) & 0x00000040)>>6)
#define SEC6_WR(src)                             (((u32)(src)<<6) & 0x00000040)
#define SEC6_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields SEC5	 */
#define SEC5_WIDTH                                                            1
#define SEC5_SHIFT                                                            5
#define SEC5_MASK                                                    0x00000020
#define SEC5_RD(src)                                  (((src) & 0x00000020)>>5)
#define SEC5_WR(src)                             (((u32)(src)<<5) & 0x00000020)
#define SEC5_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields SEC4	 */
#define SEC4_WIDTH                                                            1
#define SEC4_SHIFT                                                            4
#define SEC4_MASK                                                    0x00000010
#define SEC4_RD(src)                                  (((src) & 0x00000010)>>4)
#define SEC4_WR(src)                             (((u32)(src)<<4) & 0x00000010)
#define SEC4_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields SEC3	 */
#define SEC3_WIDTH                                                            1
#define SEC3_SHIFT                                                            3
#define SEC3_MASK                                                    0x00000008
#define SEC3_RD(src)                                  (((src) & 0x00000008)>>3)
#define SEC3_WR(src)                             (((u32)(src)<<3) & 0x00000008)
#define SEC3_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields SEC2	 */
#define SEC2_WIDTH                                                            1
#define SEC2_SHIFT                                                            2
#define SEC2_MASK                                                    0x00000004
#define SEC2_RD(src)                                  (((src) & 0x00000004)>>2)
#define SEC2_WR(src)                             (((u32)(src)<<2) & 0x00000004)
#define SEC2_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields SEC1	 */
#define SEC1_WIDTH                                                            1
#define SEC1_SHIFT                                                            1
#define SEC1_MASK                                                    0x00000002
#define SEC1_RD(src)                                  (((src) & 0x00000002)>>1)
#define SEC1_WR(src)                             (((u32)(src)<<1) & 0x00000002)
#define SEC1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC0_WIDTH                                                            1
#define SEC0_SHIFT                                                            0
#define SEC0_MASK                                                    0x00000001
#define SEC0_RD(src)                                     (((src) & 0x00000001))
#define SEC0_WR(src)                                (((u32)(src)) & 0x00000001)
#define SEC0_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_SEC_ERRLMask	*/
/*    Mask Register Fields SEC31Mask    */
#define SEC31MASK_WIDTH                                                       1
#define SEC31MASK_SHIFT                                                      31
#define SEC31MASK_MASK                                               0x80000000
#define SEC31MASK_RD(src)                            (((src) & 0x80000000)>>31)
#define SEC31MASK_WR(src)                       (((u32)(src)<<31) & 0x80000000)
#define SEC31MASK_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*    Mask Register Fields SEC30Mask    */
#define SEC30MASK_WIDTH                                                       1
#define SEC30MASK_SHIFT                                                      30
#define SEC30MASK_MASK                                               0x40000000
#define SEC30MASK_RD(src)                            (((src) & 0x40000000)>>30)
#define SEC30MASK_WR(src)                       (((u32)(src)<<30) & 0x40000000)
#define SEC30MASK_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*    Mask Register Fields SEC29Mask    */
#define SEC29MASK_WIDTH                                                       1
#define SEC29MASK_SHIFT                                                      29
#define SEC29MASK_MASK                                               0x20000000
#define SEC29MASK_RD(src)                            (((src) & 0x20000000)>>29)
#define SEC29MASK_WR(src)                       (((u32)(src)<<29) & 0x20000000)
#define SEC29MASK_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*    Mask Register Fields SEC28Mask    */
#define SEC28MASK_WIDTH                                                       1
#define SEC28MASK_SHIFT                                                      28
#define SEC28MASK_MASK                                               0x10000000
#define SEC28MASK_RD(src)                            (((src) & 0x10000000)>>28)
#define SEC28MASK_WR(src)                       (((u32)(src)<<28) & 0x10000000)
#define SEC28MASK_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*    Mask Register Fields SEC27Mask    */
#define SEC27MASK_WIDTH                                                       1
#define SEC27MASK_SHIFT                                                      27
#define SEC27MASK_MASK                                               0x08000000
#define SEC27MASK_RD(src)                            (((src) & 0x08000000)>>27)
#define SEC27MASK_WR(src)                       (((u32)(src)<<27) & 0x08000000)
#define SEC27MASK_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*    Mask Register Fields SEC26Mask    */
#define SEC26MASK_WIDTH                                                       1
#define SEC26MASK_SHIFT                                                      26
#define SEC26MASK_MASK                                               0x04000000
#define SEC26MASK_RD(src)                            (((src) & 0x04000000)>>26)
#define SEC26MASK_WR(src)                       (((u32)(src)<<26) & 0x04000000)
#define SEC26MASK_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*    Mask Register Fields SEC25Mask    */
#define SEC25MASK_WIDTH                                                       1
#define SEC25MASK_SHIFT                                                      25
#define SEC25MASK_MASK                                               0x02000000
#define SEC25MASK_RD(src)                            (((src) & 0x02000000)>>25)
#define SEC25MASK_WR(src)                       (((u32)(src)<<25) & 0x02000000)
#define SEC25MASK_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*    Mask Register Fields SEC24Mask    */
#define SEC24MASK_WIDTH                                                       1
#define SEC24MASK_SHIFT                                                      24
#define SEC24MASK_MASK                                               0x01000000
#define SEC24MASK_RD(src)                            (((src) & 0x01000000)>>24)
#define SEC24MASK_WR(src)                       (((u32)(src)<<24) & 0x01000000)
#define SEC24MASK_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*    Mask Register Fields SEC23Mask    */
#define SEC23MASK_WIDTH                                                       1
#define SEC23MASK_SHIFT                                                      23
#define SEC23MASK_MASK                                               0x00800000
#define SEC23MASK_RD(src)                            (((src) & 0x00800000)>>23)
#define SEC23MASK_WR(src)                       (((u32)(src)<<23) & 0x00800000)
#define SEC23MASK_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*    Mask Register Fields SEC22Mask    */
#define SEC22MASK_WIDTH                                                       1
#define SEC22MASK_SHIFT                                                      22
#define SEC22MASK_MASK                                               0x00400000
#define SEC22MASK_RD(src)                            (((src) & 0x00400000)>>22)
#define SEC22MASK_WR(src)                       (((u32)(src)<<22) & 0x00400000)
#define SEC22MASK_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*    Mask Register Fields SEC21Mask    */
#define SEC21MASK_WIDTH                                                       1
#define SEC21MASK_SHIFT                                                      21
#define SEC21MASK_MASK                                               0x00200000
#define SEC21MASK_RD(src)                            (((src) & 0x00200000)>>21)
#define SEC21MASK_WR(src)                       (((u32)(src)<<21) & 0x00200000)
#define SEC21MASK_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*    Mask Register Fields SEC20Mask    */
#define SEC20MASK_WIDTH                                                       1
#define SEC20MASK_SHIFT                                                      20
#define SEC20MASK_MASK                                               0x00100000
#define SEC20MASK_RD(src)                            (((src) & 0x00100000)>>20)
#define SEC20MASK_WR(src)                       (((u32)(src)<<20) & 0x00100000)
#define SEC20MASK_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*    Mask Register Fields SEC19Mask    */
#define SEC19MASK_WIDTH                                                       1
#define SEC19MASK_SHIFT                                                      19
#define SEC19MASK_MASK                                               0x00080000
#define SEC19MASK_RD(src)                            (((src) & 0x00080000)>>19)
#define SEC19MASK_WR(src)                       (((u32)(src)<<19) & 0x00080000)
#define SEC19MASK_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*    Mask Register Fields SEC18Mask    */
#define SEC18MASK_WIDTH                                                       1
#define SEC18MASK_SHIFT                                                      18
#define SEC18MASK_MASK                                               0x00040000
#define SEC18MASK_RD(src)                            (((src) & 0x00040000)>>18)
#define SEC18MASK_WR(src)                       (((u32)(src)<<18) & 0x00040000)
#define SEC18MASK_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*    Mask Register Fields SEC17Mask    */
#define SEC17MASK_WIDTH                                                       1
#define SEC17MASK_SHIFT                                                      17
#define SEC17MASK_MASK                                               0x00020000
#define SEC17MASK_RD(src)                            (((src) & 0x00020000)>>17)
#define SEC17MASK_WR(src)                       (((u32)(src)<<17) & 0x00020000)
#define SEC17MASK_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*    Mask Register Fields SEC16Mask    */
#define SEC16MASK_WIDTH                                                       1
#define SEC16MASK_SHIFT                                                      16
#define SEC16MASK_MASK                                               0x00010000
#define SEC16MASK_RD(src)                            (((src) & 0x00010000)>>16)
#define SEC16MASK_WR(src)                       (((u32)(src)<<16) & 0x00010000)
#define SEC16MASK_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*    Mask Register Fields SEC15Mask    */
#define SEC15MASK_WIDTH                                                       1
#define SEC15MASK_SHIFT                                                      15
#define SEC15MASK_MASK                                               0x00008000
#define SEC15MASK_RD(src)                            (((src) & 0x00008000)>>15)
#define SEC15MASK_WR(src)                       (((u32)(src)<<15) & 0x00008000)
#define SEC15MASK_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*    Mask Register Fields SEC14Mask    */
#define SEC14MASK_WIDTH                                                       1
#define SEC14MASK_SHIFT                                                      14
#define SEC14MASK_MASK                                               0x00004000
#define SEC14MASK_RD(src)                            (((src) & 0x00004000)>>14)
#define SEC14MASK_WR(src)                       (((u32)(src)<<14) & 0x00004000)
#define SEC14MASK_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*    Mask Register Fields SEC13Mask    */
#define SEC13MASK_WIDTH                                                       1
#define SEC13MASK_SHIFT                                                      13
#define SEC13MASK_MASK                                               0x00002000
#define SEC13MASK_RD(src)                            (((src) & 0x00002000)>>13)
#define SEC13MASK_WR(src)                       (((u32)(src)<<13) & 0x00002000)
#define SEC13MASK_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*    Mask Register Fields SEC12Mask    */
#define SEC12MASK_WIDTH                                                       1
#define SEC12MASK_SHIFT                                                      12
#define SEC12MASK_MASK                                               0x00001000
#define SEC12MASK_RD(src)                            (((src) & 0x00001000)>>12)
#define SEC12MASK_WR(src)                       (((u32)(src)<<12) & 0x00001000)
#define SEC12MASK_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*    Mask Register Fields SEC11Mask    */
#define SEC11MASK_WIDTH                                                       1
#define SEC11MASK_SHIFT                                                      11
#define SEC11MASK_MASK                                               0x00000800
#define SEC11MASK_RD(src)                            (((src) & 0x00000800)>>11)
#define SEC11MASK_WR(src)                       (((u32)(src)<<11) & 0x00000800)
#define SEC11MASK_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*    Mask Register Fields SEC10Mask    */
#define SEC10MASK_WIDTH                                                       1
#define SEC10MASK_SHIFT                                                      10
#define SEC10MASK_MASK                                               0x00000400
#define SEC10MASK_RD(src)                            (((src) & 0x00000400)>>10)
#define SEC10MASK_WR(src)                       (((u32)(src)<<10) & 0x00000400)
#define SEC10MASK_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*    Mask Register Fields SEC9Mask    */
#define SEC9MASK_WIDTH                                                        1
#define SEC9MASK_SHIFT                                                        9
#define SEC9MASK_MASK                                                0x00000200
#define SEC9MASK_RD(src)                              (((src) & 0x00000200)>>9)
#define SEC9MASK_WR(src)                         (((u32)(src)<<9) & 0x00000200)
#define SEC9MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*    Mask Register Fields SEC8Mask    */
#define SEC8MASK_WIDTH                                                        1
#define SEC8MASK_SHIFT                                                        8
#define SEC8MASK_MASK                                                0x00000100
#define SEC8MASK_RD(src)                              (((src) & 0x00000100)>>8)
#define SEC8MASK_WR(src)                         (((u32)(src)<<8) & 0x00000100)
#define SEC8MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*    Mask Register Fields SEC7Mask    */
#define SEC7MASK_WIDTH                                                        1
#define SEC7MASK_SHIFT                                                        7
#define SEC7MASK_MASK                                                0x00000080
#define SEC7MASK_RD(src)                              (((src) & 0x00000080)>>7)
#define SEC7MASK_WR(src)                         (((u32)(src)<<7) & 0x00000080)
#define SEC7MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*    Mask Register Fields SEC6Mask    */
#define SEC6MASK_WIDTH                                                        1
#define SEC6MASK_SHIFT                                                        6
#define SEC6MASK_MASK                                                0x00000040
#define SEC6MASK_RD(src)                              (((src) & 0x00000040)>>6)
#define SEC6MASK_WR(src)                         (((u32)(src)<<6) & 0x00000040)
#define SEC6MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*    Mask Register Fields SEC5Mask    */
#define SEC5MASK_WIDTH                                                        1
#define SEC5MASK_SHIFT                                                        5
#define SEC5MASK_MASK                                                0x00000020
#define SEC5MASK_RD(src)                              (((src) & 0x00000020)>>5)
#define SEC5MASK_WR(src)                         (((u32)(src)<<5) & 0x00000020)
#define SEC5MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*    Mask Register Fields SEC4Mask    */
#define SEC4MASK_WIDTH                                                        1
#define SEC4MASK_SHIFT                                                        4
#define SEC4MASK_MASK                                                0x00000010
#define SEC4MASK_RD(src)                              (((src) & 0x00000010)>>4)
#define SEC4MASK_WR(src)                         (((u32)(src)<<4) & 0x00000010)
#define SEC4MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*    Mask Register Fields SEC3Mask    */
#define SEC3MASK_WIDTH                                                        1
#define SEC3MASK_SHIFT                                                        3
#define SEC3MASK_MASK                                                0x00000008
#define SEC3MASK_RD(src)                              (((src) & 0x00000008)>>3)
#define SEC3MASK_WR(src)                         (((u32)(src)<<3) & 0x00000008)
#define SEC3MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*    Mask Register Fields SEC2Mask    */
#define SEC2MASK_WIDTH                                                        1
#define SEC2MASK_SHIFT                                                        2
#define SEC2MASK_MASK                                                0x00000004
#define SEC2MASK_RD(src)                              (((src) & 0x00000004)>>2)
#define SEC2MASK_WR(src)                         (((u32)(src)<<2) & 0x00000004)
#define SEC2MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*    Mask Register Fields SEC1Mask    */
#define SEC1MASK_WIDTH                                                        1
#define SEC1MASK_SHIFT                                                        1
#define SEC1MASK_MASK                                                0x00000002
#define SEC1MASK_RD(src)                              (((src) & 0x00000002)>>1)
#define SEC1MASK_WR(src)                         (((u32)(src)<<1) & 0x00000002)
#define SEC1MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*    Mask Register Fields SEC0Mask    */
#define SEC0MASK_WIDTH                                                        1
#define SEC0MASK_SHIFT                                                        0
#define SEC0MASK_MASK                                                0x00000001
#define SEC0MASK_RD(src)                                 (((src) & 0x00000001))
#define SEC0MASK_WR(src)                            (((u32)(src)) & 0x00000001)
#define SEC0MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_SEC_ERRH	*/ 
/*	 Fields SEC31	 */
#define SEC31_F1_WIDTH                                                        1
#define SEC31_F1_SHIFT                                                       31
#define SEC31_F1_MASK                                                0x80000000
#define SEC31_F1_RD(src)                             (((src) & 0x80000000)>>31)
#define SEC31_F1_WR(src)                        (((u32)(src)<<31) & 0x80000000)
#define SEC31_F1_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields SEC30	 */
#define SEC30_F1_WIDTH                                                        1
#define SEC30_F1_SHIFT                                                       30
#define SEC30_F1_MASK                                                0x40000000
#define SEC30_F1_RD(src)                             (((src) & 0x40000000)>>30)
#define SEC30_F1_WR(src)                        (((u32)(src)<<30) & 0x40000000)
#define SEC30_F1_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields SEC29	 */
#define SEC29_F1_WIDTH                                                        1
#define SEC29_F1_SHIFT                                                       29
#define SEC29_F1_MASK                                                0x20000000
#define SEC29_F1_RD(src)                             (((src) & 0x20000000)>>29)
#define SEC29_F1_WR(src)                        (((u32)(src)<<29) & 0x20000000)
#define SEC29_F1_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields SEC28	 */
#define SEC28_F1_WIDTH                                                        1
#define SEC28_F1_SHIFT                                                       28
#define SEC28_F1_MASK                                                0x10000000
#define SEC28_F1_RD(src)                             (((src) & 0x10000000)>>28)
#define SEC28_F1_WR(src)                        (((u32)(src)<<28) & 0x10000000)
#define SEC28_F1_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields SEC27	 */
#define SEC27_F1_WIDTH                                                        1
#define SEC27_F1_SHIFT                                                       27
#define SEC27_F1_MASK                                                0x08000000
#define SEC27_F1_RD(src)                             (((src) & 0x08000000)>>27)
#define SEC27_F1_WR(src)                        (((u32)(src)<<27) & 0x08000000)
#define SEC27_F1_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields SEC26	 */
#define SEC26_F1_WIDTH                                                        1
#define SEC26_F1_SHIFT                                                       26
#define SEC26_F1_MASK                                                0x04000000
#define SEC26_F1_RD(src)                             (((src) & 0x04000000)>>26)
#define SEC26_F1_WR(src)                        (((u32)(src)<<26) & 0x04000000)
#define SEC26_F1_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields SEC25	 */
#define SEC25_F1_WIDTH                                                        1
#define SEC25_F1_SHIFT                                                       25
#define SEC25_F1_MASK                                                0x02000000
#define SEC25_F1_RD(src)                             (((src) & 0x02000000)>>25)
#define SEC25_F1_WR(src)                        (((u32)(src)<<25) & 0x02000000)
#define SEC25_F1_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields SEC24	 */
#define SEC24_F1_WIDTH                                                        1
#define SEC24_F1_SHIFT                                                       24
#define SEC24_F1_MASK                                                0x01000000
#define SEC24_F1_RD(src)                             (((src) & 0x01000000)>>24)
#define SEC24_F1_WR(src)                        (((u32)(src)<<24) & 0x01000000)
#define SEC24_F1_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*	 Fields SEC23	 */
#define SEC23_F1_WIDTH                                                        1
#define SEC23_F1_SHIFT                                                       23
#define SEC23_F1_MASK                                                0x00800000
#define SEC23_F1_RD(src)                             (((src) & 0x00800000)>>23)
#define SEC23_F1_WR(src)                        (((u32)(src)<<23) & 0x00800000)
#define SEC23_F1_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields SEC22	 */
#define SEC22_F1_WIDTH                                                        1
#define SEC22_F1_SHIFT                                                       22
#define SEC22_F1_MASK                                                0x00400000
#define SEC22_F1_RD(src)                             (((src) & 0x00400000)>>22)
#define SEC22_F1_WR(src)                        (((u32)(src)<<22) & 0x00400000)
#define SEC22_F1_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields SEC21	 */
#define SEC21_F1_WIDTH                                                        1
#define SEC21_F1_SHIFT                                                       21
#define SEC21_F1_MASK                                                0x00200000
#define SEC21_F1_RD(src)                             (((src) & 0x00200000)>>21)
#define SEC21_F1_WR(src)                        (((u32)(src)<<21) & 0x00200000)
#define SEC21_F1_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields SEC20	 */
#define SEC20_F1_WIDTH                                                        1
#define SEC20_F1_SHIFT                                                       20
#define SEC20_F1_MASK                                                0x00100000
#define SEC20_F1_RD(src)                             (((src) & 0x00100000)>>20)
#define SEC20_F1_WR(src)                        (((u32)(src)<<20) & 0x00100000)
#define SEC20_F1_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields SEC19	 */
#define SEC19_F1_WIDTH                                                        1
#define SEC19_F1_SHIFT                                                       19
#define SEC19_F1_MASK                                                0x00080000
#define SEC19_F1_RD(src)                             (((src) & 0x00080000)>>19)
#define SEC19_F1_WR(src)                        (((u32)(src)<<19) & 0x00080000)
#define SEC19_F1_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields SEC18	 */
#define SEC18_F1_WIDTH                                                        1
#define SEC18_F1_SHIFT                                                       18
#define SEC18_F1_MASK                                                0x00040000
#define SEC18_F1_RD(src)                             (((src) & 0x00040000)>>18)
#define SEC18_F1_WR(src)                        (((u32)(src)<<18) & 0x00040000)
#define SEC18_F1_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields SEC17	 */
#define SEC17_F1_WIDTH                                                        1
#define SEC17_F1_SHIFT                                                       17
#define SEC17_F1_MASK                                                0x00020000
#define SEC17_F1_RD(src)                             (((src) & 0x00020000)>>17)
#define SEC17_F1_WR(src)                        (((u32)(src)<<17) & 0x00020000)
#define SEC17_F1_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields SEC16	 */
#define SEC16_F1_WIDTH                                                        1
#define SEC16_F1_SHIFT                                                       16
#define SEC16_F1_MASK                                                0x00010000
#define SEC16_F1_RD(src)                             (((src) & 0x00010000)>>16)
#define SEC16_F1_WR(src)                        (((u32)(src)<<16) & 0x00010000)
#define SEC16_F1_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields SEC15	 */
#define SEC15_F1_WIDTH                                                        1
#define SEC15_F1_SHIFT                                                       15
#define SEC15_F1_MASK                                                0x00008000
#define SEC15_F1_RD(src)                             (((src) & 0x00008000)>>15)
#define SEC15_F1_WR(src)                        (((u32)(src)<<15) & 0x00008000)
#define SEC15_F1_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields SEC14	 */
#define SEC14_F1_WIDTH                                                        1
#define SEC14_F1_SHIFT                                                       14
#define SEC14_F1_MASK                                                0x00004000
#define SEC14_F1_RD(src)                             (((src) & 0x00004000)>>14)
#define SEC14_F1_WR(src)                        (((u32)(src)<<14) & 0x00004000)
#define SEC14_F1_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields SEC13	 */
#define SEC13_F1_WIDTH                                                        1
#define SEC13_F1_SHIFT                                                       13
#define SEC13_F1_MASK                                                0x00002000
#define SEC13_F1_RD(src)                             (((src) & 0x00002000)>>13)
#define SEC13_F1_WR(src)                        (((u32)(src)<<13) & 0x00002000)
#define SEC13_F1_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields SEC12	 */
#define SEC12_F1_WIDTH                                                        1
#define SEC12_F1_SHIFT                                                       12
#define SEC12_F1_MASK                                                0x00001000
#define SEC12_F1_RD(src)                             (((src) & 0x00001000)>>12)
#define SEC12_F1_WR(src)                        (((u32)(src)<<12) & 0x00001000)
#define SEC12_F1_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields SEC11	 */
#define SEC11_F1_WIDTH                                                        1
#define SEC11_F1_SHIFT                                                       11
#define SEC11_F1_MASK                                                0x00000800
#define SEC11_F1_RD(src)                             (((src) & 0x00000800)>>11)
#define SEC11_F1_WR(src)                        (((u32)(src)<<11) & 0x00000800)
#define SEC11_F1_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields SEC10	 */
#define SEC10_F1_WIDTH                                                        1
#define SEC10_F1_SHIFT                                                       10
#define SEC10_F1_MASK                                                0x00000400
#define SEC10_F1_RD(src)                             (((src) & 0x00000400)>>10)
#define SEC10_F1_WR(src)                        (((u32)(src)<<10) & 0x00000400)
#define SEC10_F1_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields SEC9	 */
#define SEC9_F1_WIDTH                                                         1
#define SEC9_F1_SHIFT                                                         9
#define SEC9_F1_MASK                                                 0x00000200
#define SEC9_F1_RD(src)                               (((src) & 0x00000200)>>9)
#define SEC9_F1_WR(src)                          (((u32)(src)<<9) & 0x00000200)
#define SEC9_F1_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields SEC8	 */
#define SEC8_F1_WIDTH                                                         1
#define SEC8_F1_SHIFT                                                         8
#define SEC8_F1_MASK                                                 0x00000100
#define SEC8_F1_RD(src)                               (((src) & 0x00000100)>>8)
#define SEC8_F1_WR(src)                          (((u32)(src)<<8) & 0x00000100)
#define SEC8_F1_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields SEC7	 */
#define SEC7_F1_WIDTH                                                         1
#define SEC7_F1_SHIFT                                                         7
#define SEC7_F1_MASK                                                 0x00000080
#define SEC7_F1_RD(src)                               (((src) & 0x00000080)>>7)
#define SEC7_F1_WR(src)                          (((u32)(src)<<7) & 0x00000080)
#define SEC7_F1_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields SEC6	 */
#define SEC6_F1_WIDTH                                                         1
#define SEC6_F1_SHIFT                                                         6
#define SEC6_F1_MASK                                                 0x00000040
#define SEC6_F1_RD(src)                               (((src) & 0x00000040)>>6)
#define SEC6_F1_WR(src)                          (((u32)(src)<<6) & 0x00000040)
#define SEC6_F1_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields SEC5	 */
#define SEC5_F1_WIDTH                                                         1
#define SEC5_F1_SHIFT                                                         5
#define SEC5_F1_MASK                                                 0x00000020
#define SEC5_F1_RD(src)                               (((src) & 0x00000020)>>5)
#define SEC5_F1_WR(src)                          (((u32)(src)<<5) & 0x00000020)
#define SEC5_F1_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields SEC4	 */
#define SEC4_F1_WIDTH                                                         1
#define SEC4_F1_SHIFT                                                         4
#define SEC4_F1_MASK                                                 0x00000010
#define SEC4_F1_RD(src)                               (((src) & 0x00000010)>>4)
#define SEC4_F1_WR(src)                          (((u32)(src)<<4) & 0x00000010)
#define SEC4_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields SEC3	 */
#define SEC3_F1_WIDTH                                                         1
#define SEC3_F1_SHIFT                                                         3
#define SEC3_F1_MASK                                                 0x00000008
#define SEC3_F1_RD(src)                               (((src) & 0x00000008)>>3)
#define SEC3_F1_WR(src)                          (((u32)(src)<<3) & 0x00000008)
#define SEC3_F1_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields SEC2	 */
#define SEC2_F1_WIDTH                                                         1
#define SEC2_F1_SHIFT                                                         2
#define SEC2_F1_MASK                                                 0x00000004
#define SEC2_F1_RD(src)                               (((src) & 0x00000004)>>2)
#define SEC2_F1_WR(src)                          (((u32)(src)<<2) & 0x00000004)
#define SEC2_F1_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields SEC1	 */
#define SEC1_F1_WIDTH                                                         1
#define SEC1_F1_SHIFT                                                         1
#define SEC1_F1_MASK                                                 0x00000002
#define SEC1_F1_RD(src)                               (((src) & 0x00000002)>>1)
#define SEC1_F1_WR(src)                          (((u32)(src)<<1) & 0x00000002)
#define SEC1_F1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC0_F1_WIDTH                                                         1
#define SEC0_F1_SHIFT                                                         0
#define SEC0_F1_MASK                                                 0x00000001
#define SEC0_F1_RD(src)                                  (((src) & 0x00000001))
#define SEC0_F1_WR(src)                             (((u32)(src)) & 0x00000001)
#define SEC0_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_SEC_ERRHMask	*/
/*    Mask Register Fields SEC31Mask    */
#define SEC31MASK_F1_WIDTH                                                    1
#define SEC31MASK_F1_SHIFT                                                   31
#define SEC31MASK_F1_MASK                                            0x80000000
#define SEC31MASK_F1_RD(src)                         (((src) & 0x80000000)>>31)
#define SEC31MASK_F1_WR(src)                    (((u32)(src)<<31) & 0x80000000)
#define SEC31MASK_F1_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*    Mask Register Fields SEC30Mask    */
#define SEC30MASK_F1_WIDTH                                                    1
#define SEC30MASK_F1_SHIFT                                                   30
#define SEC30MASK_F1_MASK                                            0x40000000
#define SEC30MASK_F1_RD(src)                         (((src) & 0x40000000)>>30)
#define SEC30MASK_F1_WR(src)                    (((u32)(src)<<30) & 0x40000000)
#define SEC30MASK_F1_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*    Mask Register Fields SEC29Mask    */
#define SEC29MASK_F1_WIDTH                                                    1
#define SEC29MASK_F1_SHIFT                                                   29
#define SEC29MASK_F1_MASK                                            0x20000000
#define SEC29MASK_F1_RD(src)                         (((src) & 0x20000000)>>29)
#define SEC29MASK_F1_WR(src)                    (((u32)(src)<<29) & 0x20000000)
#define SEC29MASK_F1_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*    Mask Register Fields SEC28Mask    */
#define SEC28MASK_F1_WIDTH                                                    1
#define SEC28MASK_F1_SHIFT                                                   28
#define SEC28MASK_F1_MASK                                            0x10000000
#define SEC28MASK_F1_RD(src)                         (((src) & 0x10000000)>>28)
#define SEC28MASK_F1_WR(src)                    (((u32)(src)<<28) & 0x10000000)
#define SEC28MASK_F1_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*    Mask Register Fields SEC27Mask    */
#define SEC27MASK_F1_WIDTH                                                    1
#define SEC27MASK_F1_SHIFT                                                   27
#define SEC27MASK_F1_MASK                                            0x08000000
#define SEC27MASK_F1_RD(src)                         (((src) & 0x08000000)>>27)
#define SEC27MASK_F1_WR(src)                    (((u32)(src)<<27) & 0x08000000)
#define SEC27MASK_F1_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*    Mask Register Fields SEC26Mask    */
#define SEC26MASK_F1_WIDTH                                                    1
#define SEC26MASK_F1_SHIFT                                                   26
#define SEC26MASK_F1_MASK                                            0x04000000
#define SEC26MASK_F1_RD(src)                         (((src) & 0x04000000)>>26)
#define SEC26MASK_F1_WR(src)                    (((u32)(src)<<26) & 0x04000000)
#define SEC26MASK_F1_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*    Mask Register Fields SEC25Mask    */
#define SEC25MASK_F1_WIDTH                                                    1
#define SEC25MASK_F1_SHIFT                                                   25
#define SEC25MASK_F1_MASK                                            0x02000000
#define SEC25MASK_F1_RD(src)                         (((src) & 0x02000000)>>25)
#define SEC25MASK_F1_WR(src)                    (((u32)(src)<<25) & 0x02000000)
#define SEC25MASK_F1_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*    Mask Register Fields SEC24Mask    */
#define SEC24MASK_F1_WIDTH                                                    1
#define SEC24MASK_F1_SHIFT                                                   24
#define SEC24MASK_F1_MASK                                            0x01000000
#define SEC24MASK_F1_RD(src)                         (((src) & 0x01000000)>>24)
#define SEC24MASK_F1_WR(src)                    (((u32)(src)<<24) & 0x01000000)
#define SEC24MASK_F1_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*    Mask Register Fields SEC23Mask    */
#define SEC23MASK_F1_WIDTH                                                    1
#define SEC23MASK_F1_SHIFT                                                   23
#define SEC23MASK_F1_MASK                                            0x00800000
#define SEC23MASK_F1_RD(src)                         (((src) & 0x00800000)>>23)
#define SEC23MASK_F1_WR(src)                    (((u32)(src)<<23) & 0x00800000)
#define SEC23MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*    Mask Register Fields SEC22Mask    */
#define SEC22MASK_F1_WIDTH                                                    1
#define SEC22MASK_F1_SHIFT                                                   22
#define SEC22MASK_F1_MASK                                            0x00400000
#define SEC22MASK_F1_RD(src)                         (((src) & 0x00400000)>>22)
#define SEC22MASK_F1_WR(src)                    (((u32)(src)<<22) & 0x00400000)
#define SEC22MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*    Mask Register Fields SEC21Mask    */
#define SEC21MASK_F1_WIDTH                                                    1
#define SEC21MASK_F1_SHIFT                                                   21
#define SEC21MASK_F1_MASK                                            0x00200000
#define SEC21MASK_F1_RD(src)                         (((src) & 0x00200000)>>21)
#define SEC21MASK_F1_WR(src)                    (((u32)(src)<<21) & 0x00200000)
#define SEC21MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*    Mask Register Fields SEC20Mask    */
#define SEC20MASK_F1_WIDTH                                                    1
#define SEC20MASK_F1_SHIFT                                                   20
#define SEC20MASK_F1_MASK                                            0x00100000
#define SEC20MASK_F1_RD(src)                         (((src) & 0x00100000)>>20)
#define SEC20MASK_F1_WR(src)                    (((u32)(src)<<20) & 0x00100000)
#define SEC20MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*    Mask Register Fields SEC19Mask    */
#define SEC19MASK_F1_WIDTH                                                    1
#define SEC19MASK_F1_SHIFT                                                   19
#define SEC19MASK_F1_MASK                                            0x00080000
#define SEC19MASK_F1_RD(src)                         (((src) & 0x00080000)>>19)
#define SEC19MASK_F1_WR(src)                    (((u32)(src)<<19) & 0x00080000)
#define SEC19MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*    Mask Register Fields SEC18Mask    */
#define SEC18MASK_F1_WIDTH                                                    1
#define SEC18MASK_F1_SHIFT                                                   18
#define SEC18MASK_F1_MASK                                            0x00040000
#define SEC18MASK_F1_RD(src)                         (((src) & 0x00040000)>>18)
#define SEC18MASK_F1_WR(src)                    (((u32)(src)<<18) & 0x00040000)
#define SEC18MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*    Mask Register Fields SEC17Mask    */
#define SEC17MASK_F1_WIDTH                                                    1
#define SEC17MASK_F1_SHIFT                                                   17
#define SEC17MASK_F1_MASK                                            0x00020000
#define SEC17MASK_F1_RD(src)                         (((src) & 0x00020000)>>17)
#define SEC17MASK_F1_WR(src)                    (((u32)(src)<<17) & 0x00020000)
#define SEC17MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*    Mask Register Fields SEC16Mask    */
#define SEC16MASK_F1_WIDTH                                                    1
#define SEC16MASK_F1_SHIFT                                                   16
#define SEC16MASK_F1_MASK                                            0x00010000
#define SEC16MASK_F1_RD(src)                         (((src) & 0x00010000)>>16)
#define SEC16MASK_F1_WR(src)                    (((u32)(src)<<16) & 0x00010000)
#define SEC16MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*    Mask Register Fields SEC15Mask    */
#define SEC15MASK_F1_WIDTH                                                    1
#define SEC15MASK_F1_SHIFT                                                   15
#define SEC15MASK_F1_MASK                                            0x00008000
#define SEC15MASK_F1_RD(src)                         (((src) & 0x00008000)>>15)
#define SEC15MASK_F1_WR(src)                    (((u32)(src)<<15) & 0x00008000)
#define SEC15MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*    Mask Register Fields SEC14Mask    */
#define SEC14MASK_F1_WIDTH                                                    1
#define SEC14MASK_F1_SHIFT                                                   14
#define SEC14MASK_F1_MASK                                            0x00004000
#define SEC14MASK_F1_RD(src)                         (((src) & 0x00004000)>>14)
#define SEC14MASK_F1_WR(src)                    (((u32)(src)<<14) & 0x00004000)
#define SEC14MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*    Mask Register Fields SEC13Mask    */
#define SEC13MASK_F1_WIDTH                                                    1
#define SEC13MASK_F1_SHIFT                                                   13
#define SEC13MASK_F1_MASK                                            0x00002000
#define SEC13MASK_F1_RD(src)                         (((src) & 0x00002000)>>13)
#define SEC13MASK_F1_WR(src)                    (((u32)(src)<<13) & 0x00002000)
#define SEC13MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*    Mask Register Fields SEC12Mask    */
#define SEC12MASK_F1_WIDTH                                                    1
#define SEC12MASK_F1_SHIFT                                                   12
#define SEC12MASK_F1_MASK                                            0x00001000
#define SEC12MASK_F1_RD(src)                         (((src) & 0x00001000)>>12)
#define SEC12MASK_F1_WR(src)                    (((u32)(src)<<12) & 0x00001000)
#define SEC12MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*    Mask Register Fields SEC11Mask    */
#define SEC11MASK_F1_WIDTH                                                    1
#define SEC11MASK_F1_SHIFT                                                   11
#define SEC11MASK_F1_MASK                                            0x00000800
#define SEC11MASK_F1_RD(src)                         (((src) & 0x00000800)>>11)
#define SEC11MASK_F1_WR(src)                    (((u32)(src)<<11) & 0x00000800)
#define SEC11MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*    Mask Register Fields SEC10Mask    */
#define SEC10MASK_F1_WIDTH                                                    1
#define SEC10MASK_F1_SHIFT                                                   10
#define SEC10MASK_F1_MASK                                            0x00000400
#define SEC10MASK_F1_RD(src)                         (((src) & 0x00000400)>>10)
#define SEC10MASK_F1_WR(src)                    (((u32)(src)<<10) & 0x00000400)
#define SEC10MASK_F1_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*    Mask Register Fields SEC9Mask    */
#define SEC9MASK_F1_WIDTH                                                     1
#define SEC9MASK_F1_SHIFT                                                     9
#define SEC9MASK_F1_MASK                                             0x00000200
#define SEC9MASK_F1_RD(src)                           (((src) & 0x00000200)>>9)
#define SEC9MASK_F1_WR(src)                      (((u32)(src)<<9) & 0x00000200)
#define SEC9MASK_F1_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*    Mask Register Fields SEC8Mask    */
#define SEC8MASK_F1_WIDTH                                                     1
#define SEC8MASK_F1_SHIFT                                                     8
#define SEC8MASK_F1_MASK                                             0x00000100
#define SEC8MASK_F1_RD(src)                           (((src) & 0x00000100)>>8)
#define SEC8MASK_F1_WR(src)                      (((u32)(src)<<8) & 0x00000100)
#define SEC8MASK_F1_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*    Mask Register Fields SEC7Mask    */
#define SEC7MASK_F1_WIDTH                                                     1
#define SEC7MASK_F1_SHIFT                                                     7
#define SEC7MASK_F1_MASK                                             0x00000080
#define SEC7MASK_F1_RD(src)                           (((src) & 0x00000080)>>7)
#define SEC7MASK_F1_WR(src)                      (((u32)(src)<<7) & 0x00000080)
#define SEC7MASK_F1_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*    Mask Register Fields SEC6Mask    */
#define SEC6MASK_F1_WIDTH                                                     1
#define SEC6MASK_F1_SHIFT                                                     6
#define SEC6MASK_F1_MASK                                             0x00000040
#define SEC6MASK_F1_RD(src)                           (((src) & 0x00000040)>>6)
#define SEC6MASK_F1_WR(src)                      (((u32)(src)<<6) & 0x00000040)
#define SEC6MASK_F1_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*    Mask Register Fields SEC5Mask    */
#define SEC5MASK_F1_WIDTH                                                     1
#define SEC5MASK_F1_SHIFT                                                     5
#define SEC5MASK_F1_MASK                                             0x00000020
#define SEC5MASK_F1_RD(src)                           (((src) & 0x00000020)>>5)
#define SEC5MASK_F1_WR(src)                      (((u32)(src)<<5) & 0x00000020)
#define SEC5MASK_F1_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*    Mask Register Fields SEC4Mask    */
#define SEC4MASK_F1_WIDTH                                                     1
#define SEC4MASK_F1_SHIFT                                                     4
#define SEC4MASK_F1_MASK                                             0x00000010
#define SEC4MASK_F1_RD(src)                           (((src) & 0x00000010)>>4)
#define SEC4MASK_F1_WR(src)                      (((u32)(src)<<4) & 0x00000010)
#define SEC4MASK_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*    Mask Register Fields SEC3Mask    */
#define SEC3MASK_F1_WIDTH                                                     1
#define SEC3MASK_F1_SHIFT                                                     3
#define SEC3MASK_F1_MASK                                             0x00000008
#define SEC3MASK_F1_RD(src)                           (((src) & 0x00000008)>>3)
#define SEC3MASK_F1_WR(src)                      (((u32)(src)<<3) & 0x00000008)
#define SEC3MASK_F1_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*    Mask Register Fields SEC2Mask    */
#define SEC2MASK_F1_WIDTH                                                     1
#define SEC2MASK_F1_SHIFT                                                     2
#define SEC2MASK_F1_MASK                                             0x00000004
#define SEC2MASK_F1_RD(src)                           (((src) & 0x00000004)>>2)
#define SEC2MASK_F1_WR(src)                      (((u32)(src)<<2) & 0x00000004)
#define SEC2MASK_F1_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*    Mask Register Fields SEC1Mask    */
#define SEC1MASK_F1_WIDTH                                                     1
#define SEC1MASK_F1_SHIFT                                                     1
#define SEC1MASK_F1_MASK                                             0x00000002
#define SEC1MASK_F1_RD(src)                           (((src) & 0x00000002)>>1)
#define SEC1MASK_F1_WR(src)                      (((u32)(src)<<1) & 0x00000002)
#define SEC1MASK_F1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*    Mask Register Fields SEC0Mask    */
#define SEC0MASK_F1_WIDTH                                                     1
#define SEC0MASK_F1_SHIFT                                                     0
#define SEC0MASK_F1_MASK                                             0x00000001
#define SEC0MASK_F1_RD(src)                              (((src) & 0x00000001))
#define SEC0MASK_F1_WR(src)                         (((u32)(src)) & 0x00000001)
#define SEC0MASK_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MSEC_ERRL	*/ 
/*	 Fields MSEC31	 */
#define MSEC31_WIDTH                                                          1
#define MSEC31_SHIFT                                                         31
#define MSEC31_MASK                                                  0x80000000
#define MSEC31_RD(src)                               (((src) & 0x80000000)>>31)
#define MSEC31_WR(src)                          (((u32)(src)<<31) & 0x80000000)
#define MSEC31_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields MSEC30	 */
#define MSEC30_WIDTH                                                          1
#define MSEC30_SHIFT                                                         30
#define MSEC30_MASK                                                  0x40000000
#define MSEC30_RD(src)                               (((src) & 0x40000000)>>30)
#define MSEC30_WR(src)                          (((u32)(src)<<30) & 0x40000000)
#define MSEC30_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields MSEC29	 */
#define MSEC29_WIDTH                                                          1
#define MSEC29_SHIFT                                                         29
#define MSEC29_MASK                                                  0x20000000
#define MSEC29_RD(src)                               (((src) & 0x20000000)>>29)
#define MSEC29_WR(src)                          (((u32)(src)<<29) & 0x20000000)
#define MSEC29_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields MSEC28	 */
#define MSEC28_WIDTH                                                          1
#define MSEC28_SHIFT                                                         28
#define MSEC28_MASK                                                  0x10000000
#define MSEC28_RD(src)                               (((src) & 0x10000000)>>28)
#define MSEC28_WR(src)                          (((u32)(src)<<28) & 0x10000000)
#define MSEC28_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields MSEC27	 */
#define MSEC27_WIDTH                                                          1
#define MSEC27_SHIFT                                                         27
#define MSEC27_MASK                                                  0x08000000
#define MSEC27_RD(src)                               (((src) & 0x08000000)>>27)
#define MSEC27_WR(src)                          (((u32)(src)<<27) & 0x08000000)
#define MSEC27_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields MSEC26	 */
#define MSEC26_WIDTH                                                          1
#define MSEC26_SHIFT                                                         26
#define MSEC26_MASK                                                  0x04000000
#define MSEC26_RD(src)                               (((src) & 0x04000000)>>26)
#define MSEC26_WR(src)                          (((u32)(src)<<26) & 0x04000000)
#define MSEC26_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields MSEC25	 */
#define MSEC25_WIDTH                                                          1
#define MSEC25_SHIFT                                                         25
#define MSEC25_MASK                                                  0x02000000
#define MSEC25_RD(src)                               (((src) & 0x02000000)>>25)
#define MSEC25_WR(src)                          (((u32)(src)<<25) & 0x02000000)
#define MSEC25_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields MSEC24	 */
#define MSEC24_WIDTH                                                          1
#define MSEC24_SHIFT                                                         24
#define MSEC24_MASK                                                  0x01000000
#define MSEC24_RD(src)                               (((src) & 0x01000000)>>24)
#define MSEC24_WR(src)                          (((u32)(src)<<24) & 0x01000000)
#define MSEC24_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*	 Fields MSEC23	 */
#define MSEC23_WIDTH                                                          1
#define MSEC23_SHIFT                                                         23
#define MSEC23_MASK                                                  0x00800000
#define MSEC23_RD(src)                               (((src) & 0x00800000)>>23)
#define MSEC23_WR(src)                          (((u32)(src)<<23) & 0x00800000)
#define MSEC23_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields MSEC22	 */
#define MSEC22_WIDTH                                                          1
#define MSEC22_SHIFT                                                         22
#define MSEC22_MASK                                                  0x00400000
#define MSEC22_RD(src)                               (((src) & 0x00400000)>>22)
#define MSEC22_WR(src)                          (((u32)(src)<<22) & 0x00400000)
#define MSEC22_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields MSEC21	 */
#define MSEC21_WIDTH                                                          1
#define MSEC21_SHIFT                                                         21
#define MSEC21_MASK                                                  0x00200000
#define MSEC21_RD(src)                               (((src) & 0x00200000)>>21)
#define MSEC21_WR(src)                          (((u32)(src)<<21) & 0x00200000)
#define MSEC21_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields MSEC20	 */
#define MSEC20_WIDTH                                                          1
#define MSEC20_SHIFT                                                         20
#define MSEC20_MASK                                                  0x00100000
#define MSEC20_RD(src)                               (((src) & 0x00100000)>>20)
#define MSEC20_WR(src)                          (((u32)(src)<<20) & 0x00100000)
#define MSEC20_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields MSEC19	 */
#define MSEC19_WIDTH                                                          1
#define MSEC19_SHIFT                                                         19
#define MSEC19_MASK                                                  0x00080000
#define MSEC19_RD(src)                               (((src) & 0x00080000)>>19)
#define MSEC19_WR(src)                          (((u32)(src)<<19) & 0x00080000)
#define MSEC19_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields MSEC18	 */
#define MSEC18_WIDTH                                                          1
#define MSEC18_SHIFT                                                         18
#define MSEC18_MASK                                                  0x00040000
#define MSEC18_RD(src)                               (((src) & 0x00040000)>>18)
#define MSEC18_WR(src)                          (((u32)(src)<<18) & 0x00040000)
#define MSEC18_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields MSEC17	 */
#define MSEC17_WIDTH                                                          1
#define MSEC17_SHIFT                                                         17
#define MSEC17_MASK                                                  0x00020000
#define MSEC17_RD(src)                               (((src) & 0x00020000)>>17)
#define MSEC17_WR(src)                          (((u32)(src)<<17) & 0x00020000)
#define MSEC17_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields MSEC16	 */
#define MSEC16_WIDTH                                                          1
#define MSEC16_SHIFT                                                         16
#define MSEC16_MASK                                                  0x00010000
#define MSEC16_RD(src)                               (((src) & 0x00010000)>>16)
#define MSEC16_WR(src)                          (((u32)(src)<<16) & 0x00010000)
#define MSEC16_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields MSEC15	 */
#define MSEC15_WIDTH                                                          1
#define MSEC15_SHIFT                                                         15
#define MSEC15_MASK                                                  0x00008000
#define MSEC15_RD(src)                               (((src) & 0x00008000)>>15)
#define MSEC15_WR(src)                          (((u32)(src)<<15) & 0x00008000)
#define MSEC15_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields MSEC14	 */
#define MSEC14_WIDTH                                                          1
#define MSEC14_SHIFT                                                         14
#define MSEC14_MASK                                                  0x00004000
#define MSEC14_RD(src)                               (((src) & 0x00004000)>>14)
#define MSEC14_WR(src)                          (((u32)(src)<<14) & 0x00004000)
#define MSEC14_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields MSEC13	 */
#define MSEC13_WIDTH                                                          1
#define MSEC13_SHIFT                                                         13
#define MSEC13_MASK                                                  0x00002000
#define MSEC13_RD(src)                               (((src) & 0x00002000)>>13)
#define MSEC13_WR(src)                          (((u32)(src)<<13) & 0x00002000)
#define MSEC13_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields MSEC12	 */
#define MSEC12_WIDTH                                                          1
#define MSEC12_SHIFT                                                         12
#define MSEC12_MASK                                                  0x00001000
#define MSEC12_RD(src)                               (((src) & 0x00001000)>>12)
#define MSEC12_WR(src)                          (((u32)(src)<<12) & 0x00001000)
#define MSEC12_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields MSEC11	 */
#define MSEC11_WIDTH                                                          1
#define MSEC11_SHIFT                                                         11
#define MSEC11_MASK                                                  0x00000800
#define MSEC11_RD(src)                               (((src) & 0x00000800)>>11)
#define MSEC11_WR(src)                          (((u32)(src)<<11) & 0x00000800)
#define MSEC11_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields MSEC10	 */
#define MSEC10_WIDTH                                                          1
#define MSEC10_SHIFT                                                         10
#define MSEC10_MASK                                                  0x00000400
#define MSEC10_RD(src)                               (((src) & 0x00000400)>>10)
#define MSEC10_WR(src)                          (((u32)(src)<<10) & 0x00000400)
#define MSEC10_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields MSEC9	 */
#define MSEC9_WIDTH                                                           1
#define MSEC9_SHIFT                                                           9
#define MSEC9_MASK                                                   0x00000200
#define MSEC9_RD(src)                                 (((src) & 0x00000200)>>9)
#define MSEC9_WR(src)                            (((u32)(src)<<9) & 0x00000200)
#define MSEC9_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields MSEC8	 */
#define MSEC8_WIDTH                                                           1
#define MSEC8_SHIFT                                                           8
#define MSEC8_MASK                                                   0x00000100
#define MSEC8_RD(src)                                 (((src) & 0x00000100)>>8)
#define MSEC8_WR(src)                            (((u32)(src)<<8) & 0x00000100)
#define MSEC8_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields MSEC7	 */
#define MSEC7_WIDTH                                                           1
#define MSEC7_SHIFT                                                           7
#define MSEC7_MASK                                                   0x00000080
#define MSEC7_RD(src)                                 (((src) & 0x00000080)>>7)
#define MSEC7_WR(src)                            (((u32)(src)<<7) & 0x00000080)
#define MSEC7_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields MSEC6	 */
#define MSEC6_WIDTH                                                           1
#define MSEC6_SHIFT                                                           6
#define MSEC6_MASK                                                   0x00000040
#define MSEC6_RD(src)                                 (((src) & 0x00000040)>>6)
#define MSEC6_WR(src)                            (((u32)(src)<<6) & 0x00000040)
#define MSEC6_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields MSEC5	 */
#define MSEC5_WIDTH                                                           1
#define MSEC5_SHIFT                                                           5
#define MSEC5_MASK                                                   0x00000020
#define MSEC5_RD(src)                                 (((src) & 0x00000020)>>5)
#define MSEC5_WR(src)                            (((u32)(src)<<5) & 0x00000020)
#define MSEC5_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields MSEC4	 */
#define MSEC4_WIDTH                                                           1
#define MSEC4_SHIFT                                                           4
#define MSEC4_MASK                                                   0x00000010
#define MSEC4_RD(src)                                 (((src) & 0x00000010)>>4)
#define MSEC4_WR(src)                            (((u32)(src)<<4) & 0x00000010)
#define MSEC4_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields MSEC3	 */
#define MSEC3_WIDTH                                                           1
#define MSEC3_SHIFT                                                           3
#define MSEC3_MASK                                                   0x00000008
#define MSEC3_RD(src)                                 (((src) & 0x00000008)>>3)
#define MSEC3_WR(src)                            (((u32)(src)<<3) & 0x00000008)
#define MSEC3_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields MSEC2	 */
#define MSEC2_WIDTH                                                           1
#define MSEC2_SHIFT                                                           2
#define MSEC2_MASK                                                   0x00000004
#define MSEC2_RD(src)                                 (((src) & 0x00000004)>>2)
#define MSEC2_WR(src)                            (((u32)(src)<<2) & 0x00000004)
#define MSEC2_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields MSEC1	 */
#define MSEC1_WIDTH                                                           1
#define MSEC1_SHIFT                                                           1
#define MSEC1_MASK                                                   0x00000002
#define MSEC1_RD(src)                                 (((src) & 0x00000002)>>1)
#define MSEC1_WR(src)                            (((u32)(src)<<1) & 0x00000002)
#define MSEC1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields MSEC0	 */
#define MSEC0_WIDTH                                                           1
#define MSEC0_SHIFT                                                           0
#define MSEC0_MASK                                                   0x00000001
#define MSEC0_RD(src)                                    (((src) & 0x00000001))
#define MSEC0_WR(src)                               (((u32)(src)) & 0x00000001)
#define MSEC0_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MSEC_ERRLMask	*/
/*    Mask Register Fields MSEC31Mask    */
#define MSEC31MASK_WIDTH                                                      1
#define MSEC31MASK_SHIFT                                                     31
#define MSEC31MASK_MASK                                              0x80000000
#define MSEC31MASK_RD(src)                           (((src) & 0x80000000)>>31)
#define MSEC31MASK_WR(src)                      (((u32)(src)<<31) & 0x80000000)
#define MSEC31MASK_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*    Mask Register Fields MSEC30Mask    */
#define MSEC30MASK_WIDTH                                                      1
#define MSEC30MASK_SHIFT                                                     30
#define MSEC30MASK_MASK                                              0x40000000
#define MSEC30MASK_RD(src)                           (((src) & 0x40000000)>>30)
#define MSEC30MASK_WR(src)                      (((u32)(src)<<30) & 0x40000000)
#define MSEC30MASK_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*    Mask Register Fields MSEC29Mask    */
#define MSEC29MASK_WIDTH                                                      1
#define MSEC29MASK_SHIFT                                                     29
#define MSEC29MASK_MASK                                              0x20000000
#define MSEC29MASK_RD(src)                           (((src) & 0x20000000)>>29)
#define MSEC29MASK_WR(src)                      (((u32)(src)<<29) & 0x20000000)
#define MSEC29MASK_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*    Mask Register Fields MSEC28Mask    */
#define MSEC28MASK_WIDTH                                                      1
#define MSEC28MASK_SHIFT                                                     28
#define MSEC28MASK_MASK                                              0x10000000
#define MSEC28MASK_RD(src)                           (((src) & 0x10000000)>>28)
#define MSEC28MASK_WR(src)                      (((u32)(src)<<28) & 0x10000000)
#define MSEC28MASK_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*    Mask Register Fields MSEC27Mask    */
#define MSEC27MASK_WIDTH                                                      1
#define MSEC27MASK_SHIFT                                                     27
#define MSEC27MASK_MASK                                              0x08000000
#define MSEC27MASK_RD(src)                           (((src) & 0x08000000)>>27)
#define MSEC27MASK_WR(src)                      (((u32)(src)<<27) & 0x08000000)
#define MSEC27MASK_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*    Mask Register Fields MSEC26Mask    */
#define MSEC26MASK_WIDTH                                                      1
#define MSEC26MASK_SHIFT                                                     26
#define MSEC26MASK_MASK                                              0x04000000
#define MSEC26MASK_RD(src)                           (((src) & 0x04000000)>>26)
#define MSEC26MASK_WR(src)                      (((u32)(src)<<26) & 0x04000000)
#define MSEC26MASK_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*    Mask Register Fields MSEC25Mask    */
#define MSEC25MASK_WIDTH                                                      1
#define MSEC25MASK_SHIFT                                                     25
#define MSEC25MASK_MASK                                              0x02000000
#define MSEC25MASK_RD(src)                           (((src) & 0x02000000)>>25)
#define MSEC25MASK_WR(src)                      (((u32)(src)<<25) & 0x02000000)
#define MSEC25MASK_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*    Mask Register Fields MSEC24Mask    */
#define MSEC24MASK_WIDTH                                                      1
#define MSEC24MASK_SHIFT                                                     24
#define MSEC24MASK_MASK                                              0x01000000
#define MSEC24MASK_RD(src)                           (((src) & 0x01000000)>>24)
#define MSEC24MASK_WR(src)                      (((u32)(src)<<24) & 0x01000000)
#define MSEC24MASK_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*    Mask Register Fields MSEC23Mask    */
#define MSEC23MASK_WIDTH                                                      1
#define MSEC23MASK_SHIFT                                                     23
#define MSEC23MASK_MASK                                              0x00800000
#define MSEC23MASK_RD(src)                           (((src) & 0x00800000)>>23)
#define MSEC23MASK_WR(src)                      (((u32)(src)<<23) & 0x00800000)
#define MSEC23MASK_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*    Mask Register Fields MSEC22Mask    */
#define MSEC22MASK_WIDTH                                                      1
#define MSEC22MASK_SHIFT                                                     22
#define MSEC22MASK_MASK                                              0x00400000
#define MSEC22MASK_RD(src)                           (((src) & 0x00400000)>>22)
#define MSEC22MASK_WR(src)                      (((u32)(src)<<22) & 0x00400000)
#define MSEC22MASK_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*    Mask Register Fields MSEC21Mask    */
#define MSEC21MASK_WIDTH                                                      1
#define MSEC21MASK_SHIFT                                                     21
#define MSEC21MASK_MASK                                              0x00200000
#define MSEC21MASK_RD(src)                           (((src) & 0x00200000)>>21)
#define MSEC21MASK_WR(src)                      (((u32)(src)<<21) & 0x00200000)
#define MSEC21MASK_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*    Mask Register Fields MSEC20Mask    */
#define MSEC20MASK_WIDTH                                                      1
#define MSEC20MASK_SHIFT                                                     20
#define MSEC20MASK_MASK                                              0x00100000
#define MSEC20MASK_RD(src)                           (((src) & 0x00100000)>>20)
#define MSEC20MASK_WR(src)                      (((u32)(src)<<20) & 0x00100000)
#define MSEC20MASK_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*    Mask Register Fields MSEC19Mask    */
#define MSEC19MASK_WIDTH                                                      1
#define MSEC19MASK_SHIFT                                                     19
#define MSEC19MASK_MASK                                              0x00080000
#define MSEC19MASK_RD(src)                           (((src) & 0x00080000)>>19)
#define MSEC19MASK_WR(src)                      (((u32)(src)<<19) & 0x00080000)
#define MSEC19MASK_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*    Mask Register Fields MSEC18Mask    */
#define MSEC18MASK_WIDTH                                                      1
#define MSEC18MASK_SHIFT                                                     18
#define MSEC18MASK_MASK                                              0x00040000
#define MSEC18MASK_RD(src)                           (((src) & 0x00040000)>>18)
#define MSEC18MASK_WR(src)                      (((u32)(src)<<18) & 0x00040000)
#define MSEC18MASK_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*    Mask Register Fields MSEC17Mask    */
#define MSEC17MASK_WIDTH                                                      1
#define MSEC17MASK_SHIFT                                                     17
#define MSEC17MASK_MASK                                              0x00020000
#define MSEC17MASK_RD(src)                           (((src) & 0x00020000)>>17)
#define MSEC17MASK_WR(src)                      (((u32)(src)<<17) & 0x00020000)
#define MSEC17MASK_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*    Mask Register Fields MSEC16Mask    */
#define MSEC16MASK_WIDTH                                                      1
#define MSEC16MASK_SHIFT                                                     16
#define MSEC16MASK_MASK                                              0x00010000
#define MSEC16MASK_RD(src)                           (((src) & 0x00010000)>>16)
#define MSEC16MASK_WR(src)                      (((u32)(src)<<16) & 0x00010000)
#define MSEC16MASK_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*    Mask Register Fields MSEC15Mask    */
#define MSEC15MASK_WIDTH                                                      1
#define MSEC15MASK_SHIFT                                                     15
#define MSEC15MASK_MASK                                              0x00008000
#define MSEC15MASK_RD(src)                           (((src) & 0x00008000)>>15)
#define MSEC15MASK_WR(src)                      (((u32)(src)<<15) & 0x00008000)
#define MSEC15MASK_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*    Mask Register Fields MSEC14Mask    */
#define MSEC14MASK_WIDTH                                                      1
#define MSEC14MASK_SHIFT                                                     14
#define MSEC14MASK_MASK                                              0x00004000
#define MSEC14MASK_RD(src)                           (((src) & 0x00004000)>>14)
#define MSEC14MASK_WR(src)                      (((u32)(src)<<14) & 0x00004000)
#define MSEC14MASK_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*    Mask Register Fields MSEC13Mask    */
#define MSEC13MASK_WIDTH                                                      1
#define MSEC13MASK_SHIFT                                                     13
#define MSEC13MASK_MASK                                              0x00002000
#define MSEC13MASK_RD(src)                           (((src) & 0x00002000)>>13)
#define MSEC13MASK_WR(src)                      (((u32)(src)<<13) & 0x00002000)
#define MSEC13MASK_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*    Mask Register Fields MSEC12Mask    */
#define MSEC12MASK_WIDTH                                                      1
#define MSEC12MASK_SHIFT                                                     12
#define MSEC12MASK_MASK                                              0x00001000
#define MSEC12MASK_RD(src)                           (((src) & 0x00001000)>>12)
#define MSEC12MASK_WR(src)                      (((u32)(src)<<12) & 0x00001000)
#define MSEC12MASK_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*    Mask Register Fields MSEC11Mask    */
#define MSEC11MASK_WIDTH                                                      1
#define MSEC11MASK_SHIFT                                                     11
#define MSEC11MASK_MASK                                              0x00000800
#define MSEC11MASK_RD(src)                           (((src) & 0x00000800)>>11)
#define MSEC11MASK_WR(src)                      (((u32)(src)<<11) & 0x00000800)
#define MSEC11MASK_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*    Mask Register Fields MSEC10Mask    */
#define MSEC10MASK_WIDTH                                                      1
#define MSEC10MASK_SHIFT                                                     10
#define MSEC10MASK_MASK                                              0x00000400
#define MSEC10MASK_RD(src)                           (((src) & 0x00000400)>>10)
#define MSEC10MASK_WR(src)                      (((u32)(src)<<10) & 0x00000400)
#define MSEC10MASK_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*    Mask Register Fields MSEC9Mask    */
#define MSEC9MASK_WIDTH                                                       1
#define MSEC9MASK_SHIFT                                                       9
#define MSEC9MASK_MASK                                               0x00000200
#define MSEC9MASK_RD(src)                             (((src) & 0x00000200)>>9)
#define MSEC9MASK_WR(src)                        (((u32)(src)<<9) & 0x00000200)
#define MSEC9MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*    Mask Register Fields MSEC8Mask    */
#define MSEC8MASK_WIDTH                                                       1
#define MSEC8MASK_SHIFT                                                       8
#define MSEC8MASK_MASK                                               0x00000100
#define MSEC8MASK_RD(src)                             (((src) & 0x00000100)>>8)
#define MSEC8MASK_WR(src)                        (((u32)(src)<<8) & 0x00000100)
#define MSEC8MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*    Mask Register Fields MSEC7Mask    */
#define MSEC7MASK_WIDTH                                                       1
#define MSEC7MASK_SHIFT                                                       7
#define MSEC7MASK_MASK                                               0x00000080
#define MSEC7MASK_RD(src)                             (((src) & 0x00000080)>>7)
#define MSEC7MASK_WR(src)                        (((u32)(src)<<7) & 0x00000080)
#define MSEC7MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*    Mask Register Fields MSEC6Mask    */
#define MSEC6MASK_WIDTH                                                       1
#define MSEC6MASK_SHIFT                                                       6
#define MSEC6MASK_MASK                                               0x00000040
#define MSEC6MASK_RD(src)                             (((src) & 0x00000040)>>6)
#define MSEC6MASK_WR(src)                        (((u32)(src)<<6) & 0x00000040)
#define MSEC6MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*    Mask Register Fields MSEC5Mask    */
#define MSEC5MASK_WIDTH                                                       1
#define MSEC5MASK_SHIFT                                                       5
#define MSEC5MASK_MASK                                               0x00000020
#define MSEC5MASK_RD(src)                             (((src) & 0x00000020)>>5)
#define MSEC5MASK_WR(src)                        (((u32)(src)<<5) & 0x00000020)
#define MSEC5MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*    Mask Register Fields MSEC4Mask    */
#define MSEC4MASK_WIDTH                                                       1
#define MSEC4MASK_SHIFT                                                       4
#define MSEC4MASK_MASK                                               0x00000010
#define MSEC4MASK_RD(src)                             (((src) & 0x00000010)>>4)
#define MSEC4MASK_WR(src)                        (((u32)(src)<<4) & 0x00000010)
#define MSEC4MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*    Mask Register Fields MSEC3Mask    */
#define MSEC3MASK_WIDTH                                                       1
#define MSEC3MASK_SHIFT                                                       3
#define MSEC3MASK_MASK                                               0x00000008
#define MSEC3MASK_RD(src)                             (((src) & 0x00000008)>>3)
#define MSEC3MASK_WR(src)                        (((u32)(src)<<3) & 0x00000008)
#define MSEC3MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*    Mask Register Fields MSEC2Mask    */
#define MSEC2MASK_WIDTH                                                       1
#define MSEC2MASK_SHIFT                                                       2
#define MSEC2MASK_MASK                                               0x00000004
#define MSEC2MASK_RD(src)                             (((src) & 0x00000004)>>2)
#define MSEC2MASK_WR(src)                        (((u32)(src)<<2) & 0x00000004)
#define MSEC2MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*    Mask Register Fields MSEC1Mask    */
#define MSEC1MASK_WIDTH                                                       1
#define MSEC1MASK_SHIFT                                                       1
#define MSEC1MASK_MASK                                               0x00000002
#define MSEC1MASK_RD(src)                             (((src) & 0x00000002)>>1)
#define MSEC1MASK_WR(src)                        (((u32)(src)<<1) & 0x00000002)
#define MSEC1MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*    Mask Register Fields MSEC0Mask    */
#define MSEC0MASK_WIDTH                                                       1
#define MSEC0MASK_SHIFT                                                       0
#define MSEC0MASK_MASK                                               0x00000001
#define MSEC0MASK_RD(src)                                (((src) & 0x00000001))
#define MSEC0MASK_WR(src)                           (((u32)(src)) & 0x00000001)
#define MSEC0MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MSEC_ERRH	*/ 
/*	 Fields MSEC63	 */
#define MSEC63_WIDTH                                                          1
#define MSEC63_SHIFT                                                         31
#define MSEC63_MASK                                                  0x80000000
#define MSEC63_RD(src)                               (((src) & 0x80000000)>>31)
#define MSEC63_WR(src)                          (((u32)(src)<<31) & 0x80000000)
#define MSEC63_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields MSEC62	 */
#define MSEC62_WIDTH                                                          1
#define MSEC62_SHIFT                                                         30
#define MSEC62_MASK                                                  0x40000000
#define MSEC62_RD(src)                               (((src) & 0x40000000)>>30)
#define MSEC62_WR(src)                          (((u32)(src)<<30) & 0x40000000)
#define MSEC62_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields MSEC61	 */
#define MSEC61_WIDTH                                                          1
#define MSEC61_SHIFT                                                         29
#define MSEC61_MASK                                                  0x20000000
#define MSEC61_RD(src)                               (((src) & 0x20000000)>>29)
#define MSEC61_WR(src)                          (((u32)(src)<<29) & 0x20000000)
#define MSEC61_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields MSEC60	 */
#define MSEC60_WIDTH                                                          1
#define MSEC60_SHIFT                                                         28
#define MSEC60_MASK                                                  0x10000000
#define MSEC60_RD(src)                               (((src) & 0x10000000)>>28)
#define MSEC60_WR(src)                          (((u32)(src)<<28) & 0x10000000)
#define MSEC60_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields MSEC59	 */
#define MSEC59_WIDTH                                                          1
#define MSEC59_SHIFT                                                         27
#define MSEC59_MASK                                                  0x08000000
#define MSEC59_RD(src)                               (((src) & 0x08000000)>>27)
#define MSEC59_WR(src)                          (((u32)(src)<<27) & 0x08000000)
#define MSEC59_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields MSEC58	 */
#define MSEC58_WIDTH                                                          1
#define MSEC58_SHIFT                                                         26
#define MSEC58_MASK                                                  0x04000000
#define MSEC58_RD(src)                               (((src) & 0x04000000)>>26)
#define MSEC58_WR(src)                          (((u32)(src)<<26) & 0x04000000)
#define MSEC58_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields MSEC57	 */
#define MSEC57_WIDTH                                                          1
#define MSEC57_SHIFT                                                         25
#define MSEC57_MASK                                                  0x02000000
#define MSEC57_RD(src)                               (((src) & 0x02000000)>>25)
#define MSEC57_WR(src)                          (((u32)(src)<<25) & 0x02000000)
#define MSEC57_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields MSEC56	 */
#define MSEC56_WIDTH                                                          1
#define MSEC56_SHIFT                                                         24
#define MSEC56_MASK                                                  0x01000000
#define MSEC56_RD(src)                               (((src) & 0x01000000)>>24)
#define MSEC56_WR(src)                          (((u32)(src)<<24) & 0x01000000)
#define MSEC56_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*	 Fields MSEC55	 */
#define MSEC55_WIDTH                                                          1
#define MSEC55_SHIFT                                                         23
#define MSEC55_MASK                                                  0x00800000
#define MSEC55_RD(src)                               (((src) & 0x00800000)>>23)
#define MSEC55_WR(src)                          (((u32)(src)<<23) & 0x00800000)
#define MSEC55_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields MSEC54	 */
#define MSEC54_WIDTH                                                          1
#define MSEC54_SHIFT                                                         22
#define MSEC54_MASK                                                  0x00400000
#define MSEC54_RD(src)                               (((src) & 0x00400000)>>22)
#define MSEC54_WR(src)                          (((u32)(src)<<22) & 0x00400000)
#define MSEC54_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields MSEC53	 */
#define MSEC53_WIDTH                                                          1
#define MSEC53_SHIFT                                                         21
#define MSEC53_MASK                                                  0x00200000
#define MSEC53_RD(src)                               (((src) & 0x00200000)>>21)
#define MSEC53_WR(src)                          (((u32)(src)<<21) & 0x00200000)
#define MSEC53_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields MSEC52	 */
#define MSEC52_WIDTH                                                          1
#define MSEC52_SHIFT                                                         20
#define MSEC52_MASK                                                  0x00100000
#define MSEC52_RD(src)                               (((src) & 0x00100000)>>20)
#define MSEC52_WR(src)                          (((u32)(src)<<20) & 0x00100000)
#define MSEC52_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields MSEC51	 */
#define MSEC51_WIDTH                                                          1
#define MSEC51_SHIFT                                                         19
#define MSEC51_MASK                                                  0x00080000
#define MSEC51_RD(src)                               (((src) & 0x00080000)>>19)
#define MSEC51_WR(src)                          (((u32)(src)<<19) & 0x00080000)
#define MSEC51_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields MSEC50	 */
#define MSEC50_WIDTH                                                          1
#define MSEC50_SHIFT                                                         18
#define MSEC50_MASK                                                  0x00040000
#define MSEC50_RD(src)                               (((src) & 0x00040000)>>18)
#define MSEC50_WR(src)                          (((u32)(src)<<18) & 0x00040000)
#define MSEC50_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields MSEC49	 */
#define MSEC49_WIDTH                                                          1
#define MSEC49_SHIFT                                                         17
#define MSEC49_MASK                                                  0x00020000
#define MSEC49_RD(src)                               (((src) & 0x00020000)>>17)
#define MSEC49_WR(src)                          (((u32)(src)<<17) & 0x00020000)
#define MSEC49_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields MSEC48	 */
#define MSEC48_WIDTH                                                          1
#define MSEC48_SHIFT                                                         16
#define MSEC48_MASK                                                  0x00010000
#define MSEC48_RD(src)                               (((src) & 0x00010000)>>16)
#define MSEC48_WR(src)                          (((u32)(src)<<16) & 0x00010000)
#define MSEC48_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields MSEC47	 */
#define MSEC47_WIDTH                                                          1
#define MSEC47_SHIFT                                                         15
#define MSEC47_MASK                                                  0x00008000
#define MSEC47_RD(src)                               (((src) & 0x00008000)>>15)
#define MSEC47_WR(src)                          (((u32)(src)<<15) & 0x00008000)
#define MSEC47_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields MSEC46	 */
#define MSEC46_WIDTH                                                          1
#define MSEC46_SHIFT                                                         14
#define MSEC46_MASK                                                  0x00004000
#define MSEC46_RD(src)                               (((src) & 0x00004000)>>14)
#define MSEC46_WR(src)                          (((u32)(src)<<14) & 0x00004000)
#define MSEC46_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields MSEC45	 */
#define MSEC45_WIDTH                                                          1
#define MSEC45_SHIFT                                                         13
#define MSEC45_MASK                                                  0x00002000
#define MSEC45_RD(src)                               (((src) & 0x00002000)>>13)
#define MSEC45_WR(src)                          (((u32)(src)<<13) & 0x00002000)
#define MSEC45_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields MSEC44	 */
#define MSEC44_WIDTH                                                          1
#define MSEC44_SHIFT                                                         12
#define MSEC44_MASK                                                  0x00001000
#define MSEC44_RD(src)                               (((src) & 0x00001000)>>12)
#define MSEC44_WR(src)                          (((u32)(src)<<12) & 0x00001000)
#define MSEC44_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields MSEC43	 */
#define MSEC43_WIDTH                                                          1
#define MSEC43_SHIFT                                                         11
#define MSEC43_MASK                                                  0x00000800
#define MSEC43_RD(src)                               (((src) & 0x00000800)>>11)
#define MSEC43_WR(src)                          (((u32)(src)<<11) & 0x00000800)
#define MSEC43_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields MSEC42	 */
#define MSEC42_WIDTH                                                          1
#define MSEC42_SHIFT                                                         10
#define MSEC42_MASK                                                  0x00000400
#define MSEC42_RD(src)                               (((src) & 0x00000400)>>10)
#define MSEC42_WR(src)                          (((u32)(src)<<10) & 0x00000400)
#define MSEC42_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields MSEC41	 */
#define MSEC41_WIDTH                                                          1
#define MSEC41_SHIFT                                                          9
#define MSEC41_MASK                                                  0x00000200
#define MSEC41_RD(src)                                (((src) & 0x00000200)>>9)
#define MSEC41_WR(src)                           (((u32)(src)<<9) & 0x00000200)
#define MSEC41_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields MSEC40	 */
#define MSEC40_WIDTH                                                          1
#define MSEC40_SHIFT                                                          8
#define MSEC40_MASK                                                  0x00000100
#define MSEC40_RD(src)                                (((src) & 0x00000100)>>8)
#define MSEC40_WR(src)                           (((u32)(src)<<8) & 0x00000100)
#define MSEC40_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields MSEC39	 */
#define MSEC39_WIDTH                                                          1
#define MSEC39_SHIFT                                                          7
#define MSEC39_MASK                                                  0x00000080
#define MSEC39_RD(src)                                (((src) & 0x00000080)>>7)
#define MSEC39_WR(src)                           (((u32)(src)<<7) & 0x00000080)
#define MSEC39_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields MSEC38	 */
#define MSEC38_WIDTH                                                          1
#define MSEC38_SHIFT                                                          6
#define MSEC38_MASK                                                  0x00000040
#define MSEC38_RD(src)                                (((src) & 0x00000040)>>6)
#define MSEC38_WR(src)                           (((u32)(src)<<6) & 0x00000040)
#define MSEC38_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields MSEC37	 */
#define MSEC37_WIDTH                                                          1
#define MSEC37_SHIFT                                                          5
#define MSEC37_MASK                                                  0x00000020
#define MSEC37_RD(src)                                (((src) & 0x00000020)>>5)
#define MSEC37_WR(src)                           (((u32)(src)<<5) & 0x00000020)
#define MSEC37_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields MSEC36	 */
#define MSEC36_WIDTH                                                          1
#define MSEC36_SHIFT                                                          4
#define MSEC36_MASK                                                  0x00000010
#define MSEC36_RD(src)                                (((src) & 0x00000010)>>4)
#define MSEC36_WR(src)                           (((u32)(src)<<4) & 0x00000010)
#define MSEC36_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields MSEC35	 */
#define MSEC35_WIDTH                                                          1
#define MSEC35_SHIFT                                                          3
#define MSEC35_MASK                                                  0x00000008
#define MSEC35_RD(src)                                (((src) & 0x00000008)>>3)
#define MSEC35_WR(src)                           (((u32)(src)<<3) & 0x00000008)
#define MSEC35_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields MSEC34	 */
#define MSEC34_WIDTH                                                          1
#define MSEC34_SHIFT                                                          2
#define MSEC34_MASK                                                  0x00000004
#define MSEC34_RD(src)                                (((src) & 0x00000004)>>2)
#define MSEC34_WR(src)                           (((u32)(src)<<2) & 0x00000004)
#define MSEC34_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields MSEC33	 */
#define MSEC33_WIDTH                                                          1
#define MSEC33_SHIFT                                                          1
#define MSEC33_MASK                                                  0x00000002
#define MSEC33_RD(src)                                (((src) & 0x00000002)>>1)
#define MSEC33_WR(src)                           (((u32)(src)<<1) & 0x00000002)
#define MSEC33_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields MSEC32	 */
#define MSEC32_WIDTH                                                          1
#define MSEC32_SHIFT                                                          0
#define MSEC32_MASK                                                  0x00000001
#define MSEC32_RD(src)                                   (((src) & 0x00000001))
#define MSEC32_WR(src)                              (((u32)(src)) & 0x00000001)
#define MSEC32_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MSEC_ERRHMask	*/
/*    Mask Register Fields MSEC63Mask    */
#define MSEC63MASK_WIDTH                                                      1
#define MSEC63MASK_SHIFT                                                     31
#define MSEC63MASK_MASK                                              0x80000000
#define MSEC63MASK_RD(src)                           (((src) & 0x80000000)>>31)
#define MSEC63MASK_WR(src)                      (((u32)(src)<<31) & 0x80000000)
#define MSEC63MASK_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*    Mask Register Fields MSEC62Mask    */
#define MSEC62MASK_WIDTH                                                      1
#define MSEC62MASK_SHIFT                                                     30
#define MSEC62MASK_MASK                                              0x40000000
#define MSEC62MASK_RD(src)                           (((src) & 0x40000000)>>30)
#define MSEC62MASK_WR(src)                      (((u32)(src)<<30) & 0x40000000)
#define MSEC62MASK_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*    Mask Register Fields MSEC61Mask    */
#define MSEC61MASK_WIDTH                                                      1
#define MSEC61MASK_SHIFT                                                     29
#define MSEC61MASK_MASK                                              0x20000000
#define MSEC61MASK_RD(src)                           (((src) & 0x20000000)>>29)
#define MSEC61MASK_WR(src)                      (((u32)(src)<<29) & 0x20000000)
#define MSEC61MASK_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*    Mask Register Fields MSEC60Mask    */
#define MSEC60MASK_WIDTH                                                      1
#define MSEC60MASK_SHIFT                                                     28
#define MSEC60MASK_MASK                                              0x10000000
#define MSEC60MASK_RD(src)                           (((src) & 0x10000000)>>28)
#define MSEC60MASK_WR(src)                      (((u32)(src)<<28) & 0x10000000)
#define MSEC60MASK_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*    Mask Register Fields MSEC59Mask    */
#define MSEC59MASK_WIDTH                                                      1
#define MSEC59MASK_SHIFT                                                     27
#define MSEC59MASK_MASK                                              0x08000000
#define MSEC59MASK_RD(src)                           (((src) & 0x08000000)>>27)
#define MSEC59MASK_WR(src)                      (((u32)(src)<<27) & 0x08000000)
#define MSEC59MASK_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*    Mask Register Fields MSEC58Mask    */
#define MSEC58MASK_WIDTH                                                      1
#define MSEC58MASK_SHIFT                                                     26
#define MSEC58MASK_MASK                                              0x04000000
#define MSEC58MASK_RD(src)                           (((src) & 0x04000000)>>26)
#define MSEC58MASK_WR(src)                      (((u32)(src)<<26) & 0x04000000)
#define MSEC58MASK_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*    Mask Register Fields MSEC57Mask    */
#define MSEC57MASK_WIDTH                                                      1
#define MSEC57MASK_SHIFT                                                     25
#define MSEC57MASK_MASK                                              0x02000000
#define MSEC57MASK_RD(src)                           (((src) & 0x02000000)>>25)
#define MSEC57MASK_WR(src)                      (((u32)(src)<<25) & 0x02000000)
#define MSEC57MASK_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*    Mask Register Fields MSEC56Mask    */
#define MSEC56MASK_WIDTH                                                      1
#define MSEC56MASK_SHIFT                                                     24
#define MSEC56MASK_MASK                                              0x01000000
#define MSEC56MASK_RD(src)                           (((src) & 0x01000000)>>24)
#define MSEC56MASK_WR(src)                      (((u32)(src)<<24) & 0x01000000)
#define MSEC56MASK_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*    Mask Register Fields MSEC55Mask    */
#define MSEC55MASK_WIDTH                                                      1
#define MSEC55MASK_SHIFT                                                     23
#define MSEC55MASK_MASK                                              0x00800000
#define MSEC55MASK_RD(src)                           (((src) & 0x00800000)>>23)
#define MSEC55MASK_WR(src)                      (((u32)(src)<<23) & 0x00800000)
#define MSEC55MASK_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*    Mask Register Fields MSEC54Mask    */
#define MSEC54MASK_WIDTH                                                      1
#define MSEC54MASK_SHIFT                                                     22
#define MSEC54MASK_MASK                                              0x00400000
#define MSEC54MASK_RD(src)                           (((src) & 0x00400000)>>22)
#define MSEC54MASK_WR(src)                      (((u32)(src)<<22) & 0x00400000)
#define MSEC54MASK_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*    Mask Register Fields MSEC53Mask    */
#define MSEC53MASK_WIDTH                                                      1
#define MSEC53MASK_SHIFT                                                     21
#define MSEC53MASK_MASK                                              0x00200000
#define MSEC53MASK_RD(src)                           (((src) & 0x00200000)>>21)
#define MSEC53MASK_WR(src)                      (((u32)(src)<<21) & 0x00200000)
#define MSEC53MASK_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*    Mask Register Fields MSEC52Mask    */
#define MSEC52MASK_WIDTH                                                      1
#define MSEC52MASK_SHIFT                                                     20
#define MSEC52MASK_MASK                                              0x00100000
#define MSEC52MASK_RD(src)                           (((src) & 0x00100000)>>20)
#define MSEC52MASK_WR(src)                      (((u32)(src)<<20) & 0x00100000)
#define MSEC52MASK_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*    Mask Register Fields MSEC51Mask    */
#define MSEC51MASK_WIDTH                                                      1
#define MSEC51MASK_SHIFT                                                     19
#define MSEC51MASK_MASK                                              0x00080000
#define MSEC51MASK_RD(src)                           (((src) & 0x00080000)>>19)
#define MSEC51MASK_WR(src)                      (((u32)(src)<<19) & 0x00080000)
#define MSEC51MASK_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*    Mask Register Fields MSEC50Mask    */
#define MSEC50MASK_WIDTH                                                      1
#define MSEC50MASK_SHIFT                                                     18
#define MSEC50MASK_MASK                                              0x00040000
#define MSEC50MASK_RD(src)                           (((src) & 0x00040000)>>18)
#define MSEC50MASK_WR(src)                      (((u32)(src)<<18) & 0x00040000)
#define MSEC50MASK_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*    Mask Register Fields MSEC49Mask    */
#define MSEC49MASK_WIDTH                                                      1
#define MSEC49MASK_SHIFT                                                     17
#define MSEC49MASK_MASK                                              0x00020000
#define MSEC49MASK_RD(src)                           (((src) & 0x00020000)>>17)
#define MSEC49MASK_WR(src)                      (((u32)(src)<<17) & 0x00020000)
#define MSEC49MASK_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*    Mask Register Fields MSEC48Mask    */
#define MSEC48MASK_WIDTH                                                      1
#define MSEC48MASK_SHIFT                                                     16
#define MSEC48MASK_MASK                                              0x00010000
#define MSEC48MASK_RD(src)                           (((src) & 0x00010000)>>16)
#define MSEC48MASK_WR(src)                      (((u32)(src)<<16) & 0x00010000)
#define MSEC48MASK_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*    Mask Register Fields MSEC47Mask    */
#define MSEC47MASK_WIDTH                                                      1
#define MSEC47MASK_SHIFT                                                     15
#define MSEC47MASK_MASK                                              0x00008000
#define MSEC47MASK_RD(src)                           (((src) & 0x00008000)>>15)
#define MSEC47MASK_WR(src)                      (((u32)(src)<<15) & 0x00008000)
#define MSEC47MASK_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*    Mask Register Fields MSEC46Mask    */
#define MSEC46MASK_WIDTH                                                      1
#define MSEC46MASK_SHIFT                                                     14
#define MSEC46MASK_MASK                                              0x00004000
#define MSEC46MASK_RD(src)                           (((src) & 0x00004000)>>14)
#define MSEC46MASK_WR(src)                      (((u32)(src)<<14) & 0x00004000)
#define MSEC46MASK_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*    Mask Register Fields MSEC45Mask    */
#define MSEC45MASK_WIDTH                                                      1
#define MSEC45MASK_SHIFT                                                     13
#define MSEC45MASK_MASK                                              0x00002000
#define MSEC45MASK_RD(src)                           (((src) & 0x00002000)>>13)
#define MSEC45MASK_WR(src)                      (((u32)(src)<<13) & 0x00002000)
#define MSEC45MASK_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*    Mask Register Fields MSEC44Mask    */
#define MSEC44MASK_WIDTH                                                      1
#define MSEC44MASK_SHIFT                                                     12
#define MSEC44MASK_MASK                                              0x00001000
#define MSEC44MASK_RD(src)                           (((src) & 0x00001000)>>12)
#define MSEC44MASK_WR(src)                      (((u32)(src)<<12) & 0x00001000)
#define MSEC44MASK_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*    Mask Register Fields MSEC43Mask    */
#define MSEC43MASK_WIDTH                                                      1
#define MSEC43MASK_SHIFT                                                     11
#define MSEC43MASK_MASK                                              0x00000800
#define MSEC43MASK_RD(src)                           (((src) & 0x00000800)>>11)
#define MSEC43MASK_WR(src)                      (((u32)(src)<<11) & 0x00000800)
#define MSEC43MASK_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*    Mask Register Fields MSEC42Mask    */
#define MSEC42MASK_WIDTH                                                      1
#define MSEC42MASK_SHIFT                                                     10
#define MSEC42MASK_MASK                                              0x00000400
#define MSEC42MASK_RD(src)                           (((src) & 0x00000400)>>10)
#define MSEC42MASK_WR(src)                      (((u32)(src)<<10) & 0x00000400)
#define MSEC42MASK_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*    Mask Register Fields MSEC41Mask    */
#define MSEC41MASK_WIDTH                                                      1
#define MSEC41MASK_SHIFT                                                      9
#define MSEC41MASK_MASK                                              0x00000200
#define MSEC41MASK_RD(src)                            (((src) & 0x00000200)>>9)
#define MSEC41MASK_WR(src)                       (((u32)(src)<<9) & 0x00000200)
#define MSEC41MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*    Mask Register Fields MSEC40Mask    */
#define MSEC40MASK_WIDTH                                                      1
#define MSEC40MASK_SHIFT                                                      8
#define MSEC40MASK_MASK                                              0x00000100
#define MSEC40MASK_RD(src)                            (((src) & 0x00000100)>>8)
#define MSEC40MASK_WR(src)                       (((u32)(src)<<8) & 0x00000100)
#define MSEC40MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*    Mask Register Fields MSEC39Mask    */
#define MSEC39MASK_WIDTH                                                      1
#define MSEC39MASK_SHIFT                                                      7
#define MSEC39MASK_MASK                                              0x00000080
#define MSEC39MASK_RD(src)                            (((src) & 0x00000080)>>7)
#define MSEC39MASK_WR(src)                       (((u32)(src)<<7) & 0x00000080)
#define MSEC39MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*    Mask Register Fields MSEC38Mask    */
#define MSEC38MASK_WIDTH                                                      1
#define MSEC38MASK_SHIFT                                                      6
#define MSEC38MASK_MASK                                              0x00000040
#define MSEC38MASK_RD(src)                            (((src) & 0x00000040)>>6)
#define MSEC38MASK_WR(src)                       (((u32)(src)<<6) & 0x00000040)
#define MSEC38MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*    Mask Register Fields MSEC37Mask    */
#define MSEC37MASK_WIDTH                                                      1
#define MSEC37MASK_SHIFT                                                      5
#define MSEC37MASK_MASK                                              0x00000020
#define MSEC37MASK_RD(src)                            (((src) & 0x00000020)>>5)
#define MSEC37MASK_WR(src)                       (((u32)(src)<<5) & 0x00000020)
#define MSEC37MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*    Mask Register Fields MSEC36Mask    */
#define MSEC36MASK_WIDTH                                                      1
#define MSEC36MASK_SHIFT                                                      4
#define MSEC36MASK_MASK                                              0x00000010
#define MSEC36MASK_RD(src)                            (((src) & 0x00000010)>>4)
#define MSEC36MASK_WR(src)                       (((u32)(src)<<4) & 0x00000010)
#define MSEC36MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*    Mask Register Fields MSEC35Mask    */
#define MSEC35MASK_WIDTH                                                      1
#define MSEC35MASK_SHIFT                                                      3
#define MSEC35MASK_MASK                                              0x00000008
#define MSEC35MASK_RD(src)                            (((src) & 0x00000008)>>3)
#define MSEC35MASK_WR(src)                       (((u32)(src)<<3) & 0x00000008)
#define MSEC35MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*    Mask Register Fields MSEC34Mask    */
#define MSEC34MASK_WIDTH                                                      1
#define MSEC34MASK_SHIFT                                                      2
#define MSEC34MASK_MASK                                              0x00000004
#define MSEC34MASK_RD(src)                            (((src) & 0x00000004)>>2)
#define MSEC34MASK_WR(src)                       (((u32)(src)<<2) & 0x00000004)
#define MSEC34MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*    Mask Register Fields MSEC33Mask    */
#define MSEC33MASK_WIDTH                                                      1
#define MSEC33MASK_SHIFT                                                      1
#define MSEC33MASK_MASK                                              0x00000002
#define MSEC33MASK_RD(src)                            (((src) & 0x00000002)>>1)
#define MSEC33MASK_WR(src)                       (((u32)(src)<<1) & 0x00000002)
#define MSEC33MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*    Mask Register Fields MSEC32Mask    */
#define MSEC32MASK_WIDTH                                                      1
#define MSEC32MASK_SHIFT                                                      0
#define MSEC32MASK_MASK                                              0x00000001
#define MSEC32MASK_RD(src)                               (((src) & 0x00000001))
#define MSEC32MASK_WR(src)                          (((u32)(src)) & 0x00000001)
#define MSEC32MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_DED_ERRL	*/ 
/*	 Fields DED31	 */
#define DED31_WIDTH                                                           1
#define DED31_SHIFT                                                          31
#define DED31_MASK                                                   0x80000000
#define DED31_RD(src)                                (((src) & 0x80000000)>>31)
#define DED31_WR(src)                           (((u32)(src)<<31) & 0x80000000)
#define DED31_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields DED30	 */
#define DED30_WIDTH                                                           1
#define DED30_SHIFT                                                          30
#define DED30_MASK                                                   0x40000000
#define DED30_RD(src)                                (((src) & 0x40000000)>>30)
#define DED30_WR(src)                           (((u32)(src)<<30) & 0x40000000)
#define DED30_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields DED29	 */
#define DED29_WIDTH                                                           1
#define DED29_SHIFT                                                          29
#define DED29_MASK                                                   0x20000000
#define DED29_RD(src)                                (((src) & 0x20000000)>>29)
#define DED29_WR(src)                           (((u32)(src)<<29) & 0x20000000)
#define DED29_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields DED28	 */
#define DED28_WIDTH                                                           1
#define DED28_SHIFT                                                          28
#define DED28_MASK                                                   0x10000000
#define DED28_RD(src)                                (((src) & 0x10000000)>>28)
#define DED28_WR(src)                           (((u32)(src)<<28) & 0x10000000)
#define DED28_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields DED27	 */
#define DED27_WIDTH                                                           1
#define DED27_SHIFT                                                          27
#define DED27_MASK                                                   0x08000000
#define DED27_RD(src)                                (((src) & 0x08000000)>>27)
#define DED27_WR(src)                           (((u32)(src)<<27) & 0x08000000)
#define DED27_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields DED26	 */
#define DED26_WIDTH                                                           1
#define DED26_SHIFT                                                          26
#define DED26_MASK                                                   0x04000000
#define DED26_RD(src)                                (((src) & 0x04000000)>>26)
#define DED26_WR(src)                           (((u32)(src)<<26) & 0x04000000)
#define DED26_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields DED25	 */
#define DED25_WIDTH                                                           1
#define DED25_SHIFT                                                          25
#define DED25_MASK                                                   0x02000000
#define DED25_RD(src)                                (((src) & 0x02000000)>>25)
#define DED25_WR(src)                           (((u32)(src)<<25) & 0x02000000)
#define DED25_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields DED24	 */
#define DED24_WIDTH                                                           1
#define DED24_SHIFT                                                          24
#define DED24_MASK                                                   0x01000000
#define DED24_RD(src)                                (((src) & 0x01000000)>>24)
#define DED24_WR(src)                           (((u32)(src)<<24) & 0x01000000)
#define DED24_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*	 Fields DED23	 */
#define DED23_WIDTH                                                           1
#define DED23_SHIFT                                                          23
#define DED23_MASK                                                   0x00800000
#define DED23_RD(src)                                (((src) & 0x00800000)>>23)
#define DED23_WR(src)                           (((u32)(src)<<23) & 0x00800000)
#define DED23_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields DED22	 */
#define DED22_WIDTH                                                           1
#define DED22_SHIFT                                                          22
#define DED22_MASK                                                   0x00400000
#define DED22_RD(src)                                (((src) & 0x00400000)>>22)
#define DED22_WR(src)                           (((u32)(src)<<22) & 0x00400000)
#define DED22_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields DED21	 */
#define DED21_WIDTH                                                           1
#define DED21_SHIFT                                                          21
#define DED21_MASK                                                   0x00200000
#define DED21_RD(src)                                (((src) & 0x00200000)>>21)
#define DED21_WR(src)                           (((u32)(src)<<21) & 0x00200000)
#define DED21_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields DED20	 */
#define DED20_WIDTH                                                           1
#define DED20_SHIFT                                                          20
#define DED20_MASK                                                   0x00100000
#define DED20_RD(src)                                (((src) & 0x00100000)>>20)
#define DED20_WR(src)                           (((u32)(src)<<20) & 0x00100000)
#define DED20_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields DED19	 */
#define DED19_WIDTH                                                           1
#define DED19_SHIFT                                                          19
#define DED19_MASK                                                   0x00080000
#define DED19_RD(src)                                (((src) & 0x00080000)>>19)
#define DED19_WR(src)                           (((u32)(src)<<19) & 0x00080000)
#define DED19_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields DED18	 */
#define DED18_WIDTH                                                           1
#define DED18_SHIFT                                                          18
#define DED18_MASK                                                   0x00040000
#define DED18_RD(src)                                (((src) & 0x00040000)>>18)
#define DED18_WR(src)                           (((u32)(src)<<18) & 0x00040000)
#define DED18_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields DED17	 */
#define DED17_WIDTH                                                           1
#define DED17_SHIFT                                                          17
#define DED17_MASK                                                   0x00020000
#define DED17_RD(src)                                (((src) & 0x00020000)>>17)
#define DED17_WR(src)                           (((u32)(src)<<17) & 0x00020000)
#define DED17_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields DED16	 */
#define DED16_WIDTH                                                           1
#define DED16_SHIFT                                                          16
#define DED16_MASK                                                   0x00010000
#define DED16_RD(src)                                (((src) & 0x00010000)>>16)
#define DED16_WR(src)                           (((u32)(src)<<16) & 0x00010000)
#define DED16_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields DED15	 */
#define DED15_WIDTH                                                           1
#define DED15_SHIFT                                                          15
#define DED15_MASK                                                   0x00008000
#define DED15_RD(src)                                (((src) & 0x00008000)>>15)
#define DED15_WR(src)                           (((u32)(src)<<15) & 0x00008000)
#define DED15_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields DED14	 */
#define DED14_WIDTH                                                           1
#define DED14_SHIFT                                                          14
#define DED14_MASK                                                   0x00004000
#define DED14_RD(src)                                (((src) & 0x00004000)>>14)
#define DED14_WR(src)                           (((u32)(src)<<14) & 0x00004000)
#define DED14_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields DED13	 */
#define DED13_WIDTH                                                           1
#define DED13_SHIFT                                                          13
#define DED13_MASK                                                   0x00002000
#define DED13_RD(src)                                (((src) & 0x00002000)>>13)
#define DED13_WR(src)                           (((u32)(src)<<13) & 0x00002000)
#define DED13_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields DED12	 */
#define DED12_WIDTH                                                           1
#define DED12_SHIFT                                                          12
#define DED12_MASK                                                   0x00001000
#define DED12_RD(src)                                (((src) & 0x00001000)>>12)
#define DED12_WR(src)                           (((u32)(src)<<12) & 0x00001000)
#define DED12_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields DED11	 */
#define DED11_WIDTH                                                           1
#define DED11_SHIFT                                                          11
#define DED11_MASK                                                   0x00000800
#define DED11_RD(src)                                (((src) & 0x00000800)>>11)
#define DED11_WR(src)                           (((u32)(src)<<11) & 0x00000800)
#define DED11_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields DED10	 */
#define DED10_WIDTH                                                           1
#define DED10_SHIFT                                                          10
#define DED10_MASK                                                   0x00000400
#define DED10_RD(src)                                (((src) & 0x00000400)>>10)
#define DED10_WR(src)                           (((u32)(src)<<10) & 0x00000400)
#define DED10_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields DED9	 */
#define DED9_WIDTH                                                            1
#define DED9_SHIFT                                                            9
#define DED9_MASK                                                    0x00000200
#define DED9_RD(src)                                  (((src) & 0x00000200)>>9)
#define DED9_WR(src)                             (((u32)(src)<<9) & 0x00000200)
#define DED9_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields DED8	 */
#define DED8_WIDTH                                                            1
#define DED8_SHIFT                                                            8
#define DED8_MASK                                                    0x00000100
#define DED8_RD(src)                                  (((src) & 0x00000100)>>8)
#define DED8_WR(src)                             (((u32)(src)<<8) & 0x00000100)
#define DED8_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields DED7	 */
#define DED7_WIDTH                                                            1
#define DED7_SHIFT                                                            7
#define DED7_MASK                                                    0x00000080
#define DED7_RD(src)                                  (((src) & 0x00000080)>>7)
#define DED7_WR(src)                             (((u32)(src)<<7) & 0x00000080)
#define DED7_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields DED6	 */
#define DED6_WIDTH                                                            1
#define DED6_SHIFT                                                            6
#define DED6_MASK                                                    0x00000040
#define DED6_RD(src)                                  (((src) & 0x00000040)>>6)
#define DED6_WR(src)                             (((u32)(src)<<6) & 0x00000040)
#define DED6_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields DED5	 */
#define DED5_WIDTH                                                            1
#define DED5_SHIFT                                                            5
#define DED5_MASK                                                    0x00000020
#define DED5_RD(src)                                  (((src) & 0x00000020)>>5)
#define DED5_WR(src)                             (((u32)(src)<<5) & 0x00000020)
#define DED5_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields DED4	 */
#define DED4_WIDTH                                                            1
#define DED4_SHIFT                                                            4
#define DED4_MASK                                                    0x00000010
#define DED4_RD(src)                                  (((src) & 0x00000010)>>4)
#define DED4_WR(src)                             (((u32)(src)<<4) & 0x00000010)
#define DED4_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields DED3	 */
#define DED3_WIDTH                                                            1
#define DED3_SHIFT                                                            3
#define DED3_MASK                                                    0x00000008
#define DED3_RD(src)                                  (((src) & 0x00000008)>>3)
#define DED3_WR(src)                             (((u32)(src)<<3) & 0x00000008)
#define DED3_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields DED2	 */
#define DED2_WIDTH                                                            1
#define DED2_SHIFT                                                            2
#define DED2_MASK                                                    0x00000004
#define DED2_RD(src)                                  (((src) & 0x00000004)>>2)
#define DED2_WR(src)                             (((u32)(src)<<2) & 0x00000004)
#define DED2_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields DED1	 */
#define DED1_WIDTH                                                            1
#define DED1_SHIFT                                                            1
#define DED1_MASK                                                    0x00000002
#define DED1_RD(src)                                  (((src) & 0x00000002)>>1)
#define DED1_WR(src)                             (((u32)(src)<<1) & 0x00000002)
#define DED1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields DED0	 */
#define DED0_WIDTH                                                            1
#define DED0_SHIFT                                                            0
#define DED0_MASK                                                    0x00000001
#define DED0_RD(src)                                     (((src) & 0x00000001))
#define DED0_WR(src)                                (((u32)(src)) & 0x00000001)
#define DED0_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_DED_ERRLMask	*/
/*    Mask Register Fields DED31Mask    */
#define DED31MASK_WIDTH                                                       1
#define DED31MASK_SHIFT                                                      31
#define DED31MASK_MASK                                               0x80000000
#define DED31MASK_RD(src)                            (((src) & 0x80000000)>>31)
#define DED31MASK_WR(src)                       (((u32)(src)<<31) & 0x80000000)
#define DED31MASK_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*    Mask Register Fields DED30Mask    */
#define DED30MASK_WIDTH                                                       1
#define DED30MASK_SHIFT                                                      30
#define DED30MASK_MASK                                               0x40000000
#define DED30MASK_RD(src)                            (((src) & 0x40000000)>>30)
#define DED30MASK_WR(src)                       (((u32)(src)<<30) & 0x40000000)
#define DED30MASK_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*    Mask Register Fields DED29Mask    */
#define DED29MASK_WIDTH                                                       1
#define DED29MASK_SHIFT                                                      29
#define DED29MASK_MASK                                               0x20000000
#define DED29MASK_RD(src)                            (((src) & 0x20000000)>>29)
#define DED29MASK_WR(src)                       (((u32)(src)<<29) & 0x20000000)
#define DED29MASK_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*    Mask Register Fields DED28Mask    */
#define DED28MASK_WIDTH                                                       1
#define DED28MASK_SHIFT                                                      28
#define DED28MASK_MASK                                               0x10000000
#define DED28MASK_RD(src)                            (((src) & 0x10000000)>>28)
#define DED28MASK_WR(src)                       (((u32)(src)<<28) & 0x10000000)
#define DED28MASK_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*    Mask Register Fields DED27Mask    */
#define DED27MASK_WIDTH                                                       1
#define DED27MASK_SHIFT                                                      27
#define DED27MASK_MASK                                               0x08000000
#define DED27MASK_RD(src)                            (((src) & 0x08000000)>>27)
#define DED27MASK_WR(src)                       (((u32)(src)<<27) & 0x08000000)
#define DED27MASK_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*    Mask Register Fields DED26Mask    */
#define DED26MASK_WIDTH                                                       1
#define DED26MASK_SHIFT                                                      26
#define DED26MASK_MASK                                               0x04000000
#define DED26MASK_RD(src)                            (((src) & 0x04000000)>>26)
#define DED26MASK_WR(src)                       (((u32)(src)<<26) & 0x04000000)
#define DED26MASK_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*    Mask Register Fields DED25Mask    */
#define DED25MASK_WIDTH                                                       1
#define DED25MASK_SHIFT                                                      25
#define DED25MASK_MASK                                               0x02000000
#define DED25MASK_RD(src)                            (((src) & 0x02000000)>>25)
#define DED25MASK_WR(src)                       (((u32)(src)<<25) & 0x02000000)
#define DED25MASK_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*    Mask Register Fields DED24Mask    */
#define DED24MASK_WIDTH                                                       1
#define DED24MASK_SHIFT                                                      24
#define DED24MASK_MASK                                               0x01000000
#define DED24MASK_RD(src)                            (((src) & 0x01000000)>>24)
#define DED24MASK_WR(src)                       (((u32)(src)<<24) & 0x01000000)
#define DED24MASK_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*    Mask Register Fields DED23Mask    */
#define DED23MASK_WIDTH                                                       1
#define DED23MASK_SHIFT                                                      23
#define DED23MASK_MASK                                               0x00800000
#define DED23MASK_RD(src)                            (((src) & 0x00800000)>>23)
#define DED23MASK_WR(src)                       (((u32)(src)<<23) & 0x00800000)
#define DED23MASK_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*    Mask Register Fields DED22Mask    */
#define DED22MASK_WIDTH                                                       1
#define DED22MASK_SHIFT                                                      22
#define DED22MASK_MASK                                               0x00400000
#define DED22MASK_RD(src)                            (((src) & 0x00400000)>>22)
#define DED22MASK_WR(src)                       (((u32)(src)<<22) & 0x00400000)
#define DED22MASK_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*    Mask Register Fields DED21Mask    */
#define DED21MASK_WIDTH                                                       1
#define DED21MASK_SHIFT                                                      21
#define DED21MASK_MASK                                               0x00200000
#define DED21MASK_RD(src)                            (((src) & 0x00200000)>>21)
#define DED21MASK_WR(src)                       (((u32)(src)<<21) & 0x00200000)
#define DED21MASK_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*    Mask Register Fields DED20Mask    */
#define DED20MASK_WIDTH                                                       1
#define DED20MASK_SHIFT                                                      20
#define DED20MASK_MASK                                               0x00100000
#define DED20MASK_RD(src)                            (((src) & 0x00100000)>>20)
#define DED20MASK_WR(src)                       (((u32)(src)<<20) & 0x00100000)
#define DED20MASK_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*    Mask Register Fields DED19Mask    */
#define DED19MASK_WIDTH                                                       1
#define DED19MASK_SHIFT                                                      19
#define DED19MASK_MASK                                               0x00080000
#define DED19MASK_RD(src)                            (((src) & 0x00080000)>>19)
#define DED19MASK_WR(src)                       (((u32)(src)<<19) & 0x00080000)
#define DED19MASK_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*    Mask Register Fields DED18Mask    */
#define DED18MASK_WIDTH                                                       1
#define DED18MASK_SHIFT                                                      18
#define DED18MASK_MASK                                               0x00040000
#define DED18MASK_RD(src)                            (((src) & 0x00040000)>>18)
#define DED18MASK_WR(src)                       (((u32)(src)<<18) & 0x00040000)
#define DED18MASK_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*    Mask Register Fields DED17Mask    */
#define DED17MASK_WIDTH                                                       1
#define DED17MASK_SHIFT                                                      17
#define DED17MASK_MASK                                               0x00020000
#define DED17MASK_RD(src)                            (((src) & 0x00020000)>>17)
#define DED17MASK_WR(src)                       (((u32)(src)<<17) & 0x00020000)
#define DED17MASK_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*    Mask Register Fields DED16Mask    */
#define DED16MASK_WIDTH                                                       1
#define DED16MASK_SHIFT                                                      16
#define DED16MASK_MASK                                               0x00010000
#define DED16MASK_RD(src)                            (((src) & 0x00010000)>>16)
#define DED16MASK_WR(src)                       (((u32)(src)<<16) & 0x00010000)
#define DED16MASK_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*    Mask Register Fields DED15Mask    */
#define DED15MASK_WIDTH                                                       1
#define DED15MASK_SHIFT                                                      15
#define DED15MASK_MASK                                               0x00008000
#define DED15MASK_RD(src)                            (((src) & 0x00008000)>>15)
#define DED15MASK_WR(src)                       (((u32)(src)<<15) & 0x00008000)
#define DED15MASK_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*    Mask Register Fields DED14Mask    */
#define DED14MASK_WIDTH                                                       1
#define DED14MASK_SHIFT                                                      14
#define DED14MASK_MASK                                               0x00004000
#define DED14MASK_RD(src)                            (((src) & 0x00004000)>>14)
#define DED14MASK_WR(src)                       (((u32)(src)<<14) & 0x00004000)
#define DED14MASK_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*    Mask Register Fields DED13Mask    */
#define DED13MASK_WIDTH                                                       1
#define DED13MASK_SHIFT                                                      13
#define DED13MASK_MASK                                               0x00002000
#define DED13MASK_RD(src)                            (((src) & 0x00002000)>>13)
#define DED13MASK_WR(src)                       (((u32)(src)<<13) & 0x00002000)
#define DED13MASK_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*    Mask Register Fields DED12Mask    */
#define DED12MASK_WIDTH                                                       1
#define DED12MASK_SHIFT                                                      12
#define DED12MASK_MASK                                               0x00001000
#define DED12MASK_RD(src)                            (((src) & 0x00001000)>>12)
#define DED12MASK_WR(src)                       (((u32)(src)<<12) & 0x00001000)
#define DED12MASK_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*    Mask Register Fields DED11Mask    */
#define DED11MASK_WIDTH                                                       1
#define DED11MASK_SHIFT                                                      11
#define DED11MASK_MASK                                               0x00000800
#define DED11MASK_RD(src)                            (((src) & 0x00000800)>>11)
#define DED11MASK_WR(src)                       (((u32)(src)<<11) & 0x00000800)
#define DED11MASK_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*    Mask Register Fields DED10Mask    */
#define DED10MASK_WIDTH                                                       1
#define DED10MASK_SHIFT                                                      10
#define DED10MASK_MASK                                               0x00000400
#define DED10MASK_RD(src)                            (((src) & 0x00000400)>>10)
#define DED10MASK_WR(src)                       (((u32)(src)<<10) & 0x00000400)
#define DED10MASK_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*    Mask Register Fields DED9Mask    */
#define DED9MASK_WIDTH                                                        1
#define DED9MASK_SHIFT                                                        9
#define DED9MASK_MASK                                                0x00000200
#define DED9MASK_RD(src)                              (((src) & 0x00000200)>>9)
#define DED9MASK_WR(src)                         (((u32)(src)<<9) & 0x00000200)
#define DED9MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*    Mask Register Fields DED8Mask    */
#define DED8MASK_WIDTH                                                        1
#define DED8MASK_SHIFT                                                        8
#define DED8MASK_MASK                                                0x00000100
#define DED8MASK_RD(src)                              (((src) & 0x00000100)>>8)
#define DED8MASK_WR(src)                         (((u32)(src)<<8) & 0x00000100)
#define DED8MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*    Mask Register Fields DED7Mask    */
#define DED7MASK_WIDTH                                                        1
#define DED7MASK_SHIFT                                                        7
#define DED7MASK_MASK                                                0x00000080
#define DED7MASK_RD(src)                              (((src) & 0x00000080)>>7)
#define DED7MASK_WR(src)                         (((u32)(src)<<7) & 0x00000080)
#define DED7MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*    Mask Register Fields DED6Mask    */
#define DED6MASK_WIDTH                                                        1
#define DED6MASK_SHIFT                                                        6
#define DED6MASK_MASK                                                0x00000040
#define DED6MASK_RD(src)                              (((src) & 0x00000040)>>6)
#define DED6MASK_WR(src)                         (((u32)(src)<<6) & 0x00000040)
#define DED6MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*    Mask Register Fields DED5Mask    */
#define DED5MASK_WIDTH                                                        1
#define DED5MASK_SHIFT                                                        5
#define DED5MASK_MASK                                                0x00000020
#define DED5MASK_RD(src)                              (((src) & 0x00000020)>>5)
#define DED5MASK_WR(src)                         (((u32)(src)<<5) & 0x00000020)
#define DED5MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*    Mask Register Fields DED4Mask    */
#define DED4MASK_WIDTH                                                        1
#define DED4MASK_SHIFT                                                        4
#define DED4MASK_MASK                                                0x00000010
#define DED4MASK_RD(src)                              (((src) & 0x00000010)>>4)
#define DED4MASK_WR(src)                         (((u32)(src)<<4) & 0x00000010)
#define DED4MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*    Mask Register Fields DED3Mask    */
#define DED3MASK_WIDTH                                                        1
#define DED3MASK_SHIFT                                                        3
#define DED3MASK_MASK                                                0x00000008
#define DED3MASK_RD(src)                              (((src) & 0x00000008)>>3)
#define DED3MASK_WR(src)                         (((u32)(src)<<3) & 0x00000008)
#define DED3MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*    Mask Register Fields DED2Mask    */
#define DED2MASK_WIDTH                                                        1
#define DED2MASK_SHIFT                                                        2
#define DED2MASK_MASK                                                0x00000004
#define DED2MASK_RD(src)                              (((src) & 0x00000004)>>2)
#define DED2MASK_WR(src)                         (((u32)(src)<<2) & 0x00000004)
#define DED2MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*    Mask Register Fields DED1Mask    */
#define DED1MASK_WIDTH                                                        1
#define DED1MASK_SHIFT                                                        1
#define DED1MASK_MASK                                                0x00000002
#define DED1MASK_RD(src)                              (((src) & 0x00000002)>>1)
#define DED1MASK_WR(src)                         (((u32)(src)<<1) & 0x00000002)
#define DED1MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*    Mask Register Fields DED0Mask    */
#define DED0MASK_WIDTH                                                        1
#define DED0MASK_SHIFT                                                        0
#define DED0MASK_MASK                                                0x00000001
#define DED0MASK_RD(src)                                 (((src) & 0x00000001))
#define DED0MASK_WR(src)                            (((u32)(src)) & 0x00000001)
#define DED0MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_DED_ERRH	*/ 
/*	 Fields DED63	 */
#define DED63_WIDTH                                                           1
#define DED63_SHIFT                                                          31
#define DED63_MASK                                                   0x80000000
#define DED63_RD(src)                                (((src) & 0x80000000)>>31)
#define DED63_WR(src)                           (((u32)(src)<<31) & 0x80000000)
#define DED63_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields DED62	 */
#define DED62_WIDTH                                                           1
#define DED62_SHIFT                                                          30
#define DED62_MASK                                                   0x40000000
#define DED62_RD(src)                                (((src) & 0x40000000)>>30)
#define DED62_WR(src)                           (((u32)(src)<<30) & 0x40000000)
#define DED62_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields DED61	 */
#define DED61_WIDTH                                                           1
#define DED61_SHIFT                                                          29
#define DED61_MASK                                                   0x20000000
#define DED61_RD(src)                                (((src) & 0x20000000)>>29)
#define DED61_WR(src)                           (((u32)(src)<<29) & 0x20000000)
#define DED61_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields DED60	 */
#define DED60_WIDTH                                                           1
#define DED60_SHIFT                                                          28
#define DED60_MASK                                                   0x10000000
#define DED60_RD(src)                                (((src) & 0x10000000)>>28)
#define DED60_WR(src)                           (((u32)(src)<<28) & 0x10000000)
#define DED60_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields DED59	 */
#define DED59_WIDTH                                                           1
#define DED59_SHIFT                                                          27
#define DED59_MASK                                                   0x08000000
#define DED59_RD(src)                                (((src) & 0x08000000)>>27)
#define DED59_WR(src)                           (((u32)(src)<<27) & 0x08000000)
#define DED59_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields DED58	 */
#define DED58_WIDTH                                                           1
#define DED58_SHIFT                                                          26
#define DED58_MASK                                                   0x04000000
#define DED58_RD(src)                                (((src) & 0x04000000)>>26)
#define DED58_WR(src)                           (((u32)(src)<<26) & 0x04000000)
#define DED58_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields DED57	 */
#define DED57_WIDTH                                                           1
#define DED57_SHIFT                                                          25
#define DED57_MASK                                                   0x02000000
#define DED57_RD(src)                                (((src) & 0x02000000)>>25)
#define DED57_WR(src)                           (((u32)(src)<<25) & 0x02000000)
#define DED57_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields DED56	 */
#define DED56_WIDTH                                                           1
#define DED56_SHIFT                                                          24
#define DED56_MASK                                                   0x01000000
#define DED56_RD(src)                                (((src) & 0x01000000)>>24)
#define DED56_WR(src)                           (((u32)(src)<<24) & 0x01000000)
#define DED56_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*	 Fields DED55	 */
#define DED55_WIDTH                                                           1
#define DED55_SHIFT                                                          23
#define DED55_MASK                                                   0x00800000
#define DED55_RD(src)                                (((src) & 0x00800000)>>23)
#define DED55_WR(src)                           (((u32)(src)<<23) & 0x00800000)
#define DED55_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields DED54	 */
#define DED54_WIDTH                                                           1
#define DED54_SHIFT                                                          22
#define DED54_MASK                                                   0x00400000
#define DED54_RD(src)                                (((src) & 0x00400000)>>22)
#define DED54_WR(src)                           (((u32)(src)<<22) & 0x00400000)
#define DED54_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields DED53	 */
#define DED53_WIDTH                                                           1
#define DED53_SHIFT                                                          21
#define DED53_MASK                                                   0x00200000
#define DED53_RD(src)                                (((src) & 0x00200000)>>21)
#define DED53_WR(src)                           (((u32)(src)<<21) & 0x00200000)
#define DED53_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields DED52	 */
#define DED52_WIDTH                                                           1
#define DED52_SHIFT                                                          20
#define DED52_MASK                                                   0x00100000
#define DED52_RD(src)                                (((src) & 0x00100000)>>20)
#define DED52_WR(src)                           (((u32)(src)<<20) & 0x00100000)
#define DED52_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields DED51	 */
#define DED51_WIDTH                                                           1
#define DED51_SHIFT                                                          19
#define DED51_MASK                                                   0x00080000
#define DED51_RD(src)                                (((src) & 0x00080000)>>19)
#define DED51_WR(src)                           (((u32)(src)<<19) & 0x00080000)
#define DED51_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields DED50	 */
#define DED50_WIDTH                                                           1
#define DED50_SHIFT                                                          18
#define DED50_MASK                                                   0x00040000
#define DED50_RD(src)                                (((src) & 0x00040000)>>18)
#define DED50_WR(src)                           (((u32)(src)<<18) & 0x00040000)
#define DED50_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields DED49	 */
#define DED49_WIDTH                                                           1
#define DED49_SHIFT                                                          17
#define DED49_MASK                                                   0x00020000
#define DED49_RD(src)                                (((src) & 0x00020000)>>17)
#define DED49_WR(src)                           (((u32)(src)<<17) & 0x00020000)
#define DED49_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields DED48	 */
#define DED48_WIDTH                                                           1
#define DED48_SHIFT                                                          16
#define DED48_MASK                                                   0x00010000
#define DED48_RD(src)                                (((src) & 0x00010000)>>16)
#define DED48_WR(src)                           (((u32)(src)<<16) & 0x00010000)
#define DED48_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields DED47	 */
#define DED47_WIDTH                                                           1
#define DED47_SHIFT                                                          15
#define DED47_MASK                                                   0x00008000
#define DED47_RD(src)                                (((src) & 0x00008000)>>15)
#define DED47_WR(src)                           (((u32)(src)<<15) & 0x00008000)
#define DED47_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields DED46	 */
#define DED46_WIDTH                                                           1
#define DED46_SHIFT                                                          14
#define DED46_MASK                                                   0x00004000
#define DED46_RD(src)                                (((src) & 0x00004000)>>14)
#define DED46_WR(src)                           (((u32)(src)<<14) & 0x00004000)
#define DED46_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields DED45	 */
#define DED45_WIDTH                                                           1
#define DED45_SHIFT                                                          13
#define DED45_MASK                                                   0x00002000
#define DED45_RD(src)                                (((src) & 0x00002000)>>13)
#define DED45_WR(src)                           (((u32)(src)<<13) & 0x00002000)
#define DED45_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields DED44	 */
#define DED44_WIDTH                                                           1
#define DED44_SHIFT                                                          12
#define DED44_MASK                                                   0x00001000
#define DED44_RD(src)                                (((src) & 0x00001000)>>12)
#define DED44_WR(src)                           (((u32)(src)<<12) & 0x00001000)
#define DED44_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields DED43	 */
#define DED43_WIDTH                                                           1
#define DED43_SHIFT                                                          11
#define DED43_MASK                                                   0x00000800
#define DED43_RD(src)                                (((src) & 0x00000800)>>11)
#define DED43_WR(src)                           (((u32)(src)<<11) & 0x00000800)
#define DED43_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields DED42	 */
#define DED42_WIDTH                                                           1
#define DED42_SHIFT                                                          10
#define DED42_MASK                                                   0x00000400
#define DED42_RD(src)                                (((src) & 0x00000400)>>10)
#define DED42_WR(src)                           (((u32)(src)<<10) & 0x00000400)
#define DED42_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields DED41	 */
#define DED41_WIDTH                                                           1
#define DED41_SHIFT                                                           9
#define DED41_MASK                                                   0x00000200
#define DED41_RD(src)                                 (((src) & 0x00000200)>>9)
#define DED41_WR(src)                            (((u32)(src)<<9) & 0x00000200)
#define DED41_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields DED40	 */
#define DED40_WIDTH                                                           1
#define DED40_SHIFT                                                           8
#define DED40_MASK                                                   0x00000100
#define DED40_RD(src)                                 (((src) & 0x00000100)>>8)
#define DED40_WR(src)                            (((u32)(src)<<8) & 0x00000100)
#define DED40_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields DED39	 */
#define DED39_WIDTH                                                           1
#define DED39_SHIFT                                                           7
#define DED39_MASK                                                   0x00000080
#define DED39_RD(src)                                 (((src) & 0x00000080)>>7)
#define DED39_WR(src)                            (((u32)(src)<<7) & 0x00000080)
#define DED39_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields DED38	 */
#define DED38_WIDTH                                                           1
#define DED38_SHIFT                                                           6
#define DED38_MASK                                                   0x00000040
#define DED38_RD(src)                                 (((src) & 0x00000040)>>6)
#define DED38_WR(src)                            (((u32)(src)<<6) & 0x00000040)
#define DED38_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields DED37	 */
#define DED37_WIDTH                                                           1
#define DED37_SHIFT                                                           5
#define DED37_MASK                                                   0x00000020
#define DED37_RD(src)                                 (((src) & 0x00000020)>>5)
#define DED37_WR(src)                            (((u32)(src)<<5) & 0x00000020)
#define DED37_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields DED36	 */
#define DED36_WIDTH                                                           1
#define DED36_SHIFT                                                           4
#define DED36_MASK                                                   0x00000010
#define DED36_RD(src)                                 (((src) & 0x00000010)>>4)
#define DED36_WR(src)                            (((u32)(src)<<4) & 0x00000010)
#define DED36_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields DED35	 */
#define DED35_WIDTH                                                           1
#define DED35_SHIFT                                                           3
#define DED35_MASK                                                   0x00000008
#define DED35_RD(src)                                 (((src) & 0x00000008)>>3)
#define DED35_WR(src)                            (((u32)(src)<<3) & 0x00000008)
#define DED35_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields DED34	 */
#define DED34_WIDTH                                                           1
#define DED34_SHIFT                                                           2
#define DED34_MASK                                                   0x00000004
#define DED34_RD(src)                                 (((src) & 0x00000004)>>2)
#define DED34_WR(src)                            (((u32)(src)<<2) & 0x00000004)
#define DED34_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields DED33	 */
#define DED33_WIDTH                                                           1
#define DED33_SHIFT                                                           1
#define DED33_MASK                                                   0x00000002
#define DED33_RD(src)                                 (((src) & 0x00000002)>>1)
#define DED33_WR(src)                            (((u32)(src)<<1) & 0x00000002)
#define DED33_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields DED32	 */
#define DED32_WIDTH                                                           1
#define DED32_SHIFT                                                           0
#define DED32_MASK                                                   0x00000001
#define DED32_RD(src)                                    (((src) & 0x00000001))
#define DED32_WR(src)                               (((u32)(src)) & 0x00000001)
#define DED32_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_DED_ERRHMask	*/
/*    Mask Register Fields DED63Mask    */
#define DED63MASK_WIDTH                                                       1
#define DED63MASK_SHIFT                                                      31
#define DED63MASK_MASK                                               0x80000000
#define DED63MASK_RD(src)                            (((src) & 0x80000000)>>31)
#define DED63MASK_WR(src)                       (((u32)(src)<<31) & 0x80000000)
#define DED63MASK_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*    Mask Register Fields DED62Mask    */
#define DED62MASK_WIDTH                                                       1
#define DED62MASK_SHIFT                                                      30
#define DED62MASK_MASK                                               0x40000000
#define DED62MASK_RD(src)                            (((src) & 0x40000000)>>30)
#define DED62MASK_WR(src)                       (((u32)(src)<<30) & 0x40000000)
#define DED62MASK_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*    Mask Register Fields DED61Mask    */
#define DED61MASK_WIDTH                                                       1
#define DED61MASK_SHIFT                                                      29
#define DED61MASK_MASK                                               0x20000000
#define DED61MASK_RD(src)                            (((src) & 0x20000000)>>29)
#define DED61MASK_WR(src)                       (((u32)(src)<<29) & 0x20000000)
#define DED61MASK_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*    Mask Register Fields DED60Mask    */
#define DED60MASK_WIDTH                                                       1
#define DED60MASK_SHIFT                                                      28
#define DED60MASK_MASK                                               0x10000000
#define DED60MASK_RD(src)                            (((src) & 0x10000000)>>28)
#define DED60MASK_WR(src)                       (((u32)(src)<<28) & 0x10000000)
#define DED60MASK_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*    Mask Register Fields DED59Mask    */
#define DED59MASK_WIDTH                                                       1
#define DED59MASK_SHIFT                                                      27
#define DED59MASK_MASK                                               0x08000000
#define DED59MASK_RD(src)                            (((src) & 0x08000000)>>27)
#define DED59MASK_WR(src)                       (((u32)(src)<<27) & 0x08000000)
#define DED59MASK_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*    Mask Register Fields DED58Mask    */
#define DED58MASK_WIDTH                                                       1
#define DED58MASK_SHIFT                                                      26
#define DED58MASK_MASK                                               0x04000000
#define DED58MASK_RD(src)                            (((src) & 0x04000000)>>26)
#define DED58MASK_WR(src)                       (((u32)(src)<<26) & 0x04000000)
#define DED58MASK_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*    Mask Register Fields DED57Mask    */
#define DED57MASK_WIDTH                                                       1
#define DED57MASK_SHIFT                                                      25
#define DED57MASK_MASK                                               0x02000000
#define DED57MASK_RD(src)                            (((src) & 0x02000000)>>25)
#define DED57MASK_WR(src)                       (((u32)(src)<<25) & 0x02000000)
#define DED57MASK_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*    Mask Register Fields DED56Mask    */
#define DED56MASK_WIDTH                                                       1
#define DED56MASK_SHIFT                                                      24
#define DED56MASK_MASK                                               0x01000000
#define DED56MASK_RD(src)                            (((src) & 0x01000000)>>24)
#define DED56MASK_WR(src)                       (((u32)(src)<<24) & 0x01000000)
#define DED56MASK_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*    Mask Register Fields DED55Mask    */
#define DED55MASK_WIDTH                                                       1
#define DED55MASK_SHIFT                                                      23
#define DED55MASK_MASK                                               0x00800000
#define DED55MASK_RD(src)                            (((src) & 0x00800000)>>23)
#define DED55MASK_WR(src)                       (((u32)(src)<<23) & 0x00800000)
#define DED55MASK_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*    Mask Register Fields DED54Mask    */
#define DED54MASK_WIDTH                                                       1
#define DED54MASK_SHIFT                                                      22
#define DED54MASK_MASK                                               0x00400000
#define DED54MASK_RD(src)                            (((src) & 0x00400000)>>22)
#define DED54MASK_WR(src)                       (((u32)(src)<<22) & 0x00400000)
#define DED54MASK_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*    Mask Register Fields DED53Mask    */
#define DED53MASK_WIDTH                                                       1
#define DED53MASK_SHIFT                                                      21
#define DED53MASK_MASK                                               0x00200000
#define DED53MASK_RD(src)                            (((src) & 0x00200000)>>21)
#define DED53MASK_WR(src)                       (((u32)(src)<<21) & 0x00200000)
#define DED53MASK_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*    Mask Register Fields DED52Mask    */
#define DED52MASK_WIDTH                                                       1
#define DED52MASK_SHIFT                                                      20
#define DED52MASK_MASK                                               0x00100000
#define DED52MASK_RD(src)                            (((src) & 0x00100000)>>20)
#define DED52MASK_WR(src)                       (((u32)(src)<<20) & 0x00100000)
#define DED52MASK_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*    Mask Register Fields DED51Mask    */
#define DED51MASK_WIDTH                                                       1
#define DED51MASK_SHIFT                                                      19
#define DED51MASK_MASK                                               0x00080000
#define DED51MASK_RD(src)                            (((src) & 0x00080000)>>19)
#define DED51MASK_WR(src)                       (((u32)(src)<<19) & 0x00080000)
#define DED51MASK_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*    Mask Register Fields DED50Mask    */
#define DED50MASK_WIDTH                                                       1
#define DED50MASK_SHIFT                                                      18
#define DED50MASK_MASK                                               0x00040000
#define DED50MASK_RD(src)                            (((src) & 0x00040000)>>18)
#define DED50MASK_WR(src)                       (((u32)(src)<<18) & 0x00040000)
#define DED50MASK_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*    Mask Register Fields DED49Mask    */
#define DED49MASK_WIDTH                                                       1
#define DED49MASK_SHIFT                                                      17
#define DED49MASK_MASK                                               0x00020000
#define DED49MASK_RD(src)                            (((src) & 0x00020000)>>17)
#define DED49MASK_WR(src)                       (((u32)(src)<<17) & 0x00020000)
#define DED49MASK_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*    Mask Register Fields DED48Mask    */
#define DED48MASK_WIDTH                                                       1
#define DED48MASK_SHIFT                                                      16
#define DED48MASK_MASK                                               0x00010000
#define DED48MASK_RD(src)                            (((src) & 0x00010000)>>16)
#define DED48MASK_WR(src)                       (((u32)(src)<<16) & 0x00010000)
#define DED48MASK_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*    Mask Register Fields DED47Mask    */
#define DED47MASK_WIDTH                                                       1
#define DED47MASK_SHIFT                                                      15
#define DED47MASK_MASK                                               0x00008000
#define DED47MASK_RD(src)                            (((src) & 0x00008000)>>15)
#define DED47MASK_WR(src)                       (((u32)(src)<<15) & 0x00008000)
#define DED47MASK_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*    Mask Register Fields DED46Mask    */
#define DED46MASK_WIDTH                                                       1
#define DED46MASK_SHIFT                                                      14
#define DED46MASK_MASK                                               0x00004000
#define DED46MASK_RD(src)                            (((src) & 0x00004000)>>14)
#define DED46MASK_WR(src)                       (((u32)(src)<<14) & 0x00004000)
#define DED46MASK_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*    Mask Register Fields DED45Mask    */
#define DED45MASK_WIDTH                                                       1
#define DED45MASK_SHIFT                                                      13
#define DED45MASK_MASK                                               0x00002000
#define DED45MASK_RD(src)                            (((src) & 0x00002000)>>13)
#define DED45MASK_WR(src)                       (((u32)(src)<<13) & 0x00002000)
#define DED45MASK_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*    Mask Register Fields DED44Mask    */
#define DED44MASK_WIDTH                                                       1
#define DED44MASK_SHIFT                                                      12
#define DED44MASK_MASK                                               0x00001000
#define DED44MASK_RD(src)                            (((src) & 0x00001000)>>12)
#define DED44MASK_WR(src)                       (((u32)(src)<<12) & 0x00001000)
#define DED44MASK_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*    Mask Register Fields DED43Mask    */
#define DED43MASK_WIDTH                                                       1
#define DED43MASK_SHIFT                                                      11
#define DED43MASK_MASK                                               0x00000800
#define DED43MASK_RD(src)                            (((src) & 0x00000800)>>11)
#define DED43MASK_WR(src)                       (((u32)(src)<<11) & 0x00000800)
#define DED43MASK_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*    Mask Register Fields DED42Mask    */
#define DED42MASK_WIDTH                                                       1
#define DED42MASK_SHIFT                                                      10
#define DED42MASK_MASK                                               0x00000400
#define DED42MASK_RD(src)                            (((src) & 0x00000400)>>10)
#define DED42MASK_WR(src)                       (((u32)(src)<<10) & 0x00000400)
#define DED42MASK_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*    Mask Register Fields DED41Mask    */
#define DED41MASK_WIDTH                                                       1
#define DED41MASK_SHIFT                                                       9
#define DED41MASK_MASK                                               0x00000200
#define DED41MASK_RD(src)                             (((src) & 0x00000200)>>9)
#define DED41MASK_WR(src)                        (((u32)(src)<<9) & 0x00000200)
#define DED41MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*    Mask Register Fields DED40Mask    */
#define DED40MASK_WIDTH                                                       1
#define DED40MASK_SHIFT                                                       8
#define DED40MASK_MASK                                               0x00000100
#define DED40MASK_RD(src)                             (((src) & 0x00000100)>>8)
#define DED40MASK_WR(src)                        (((u32)(src)<<8) & 0x00000100)
#define DED40MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*    Mask Register Fields DED39Mask    */
#define DED39MASK_WIDTH                                                       1
#define DED39MASK_SHIFT                                                       7
#define DED39MASK_MASK                                               0x00000080
#define DED39MASK_RD(src)                             (((src) & 0x00000080)>>7)
#define DED39MASK_WR(src)                        (((u32)(src)<<7) & 0x00000080)
#define DED39MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*    Mask Register Fields DED38Mask    */
#define DED38MASK_WIDTH                                                       1
#define DED38MASK_SHIFT                                                       6
#define DED38MASK_MASK                                               0x00000040
#define DED38MASK_RD(src)                             (((src) & 0x00000040)>>6)
#define DED38MASK_WR(src)                        (((u32)(src)<<6) & 0x00000040)
#define DED38MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*    Mask Register Fields DED37Mask    */
#define DED37MASK_WIDTH                                                       1
#define DED37MASK_SHIFT                                                       5
#define DED37MASK_MASK                                               0x00000020
#define DED37MASK_RD(src)                             (((src) & 0x00000020)>>5)
#define DED37MASK_WR(src)                        (((u32)(src)<<5) & 0x00000020)
#define DED37MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*    Mask Register Fields DED36Mask    */
#define DED36MASK_WIDTH                                                       1
#define DED36MASK_SHIFT                                                       4
#define DED36MASK_MASK                                               0x00000010
#define DED36MASK_RD(src)                             (((src) & 0x00000010)>>4)
#define DED36MASK_WR(src)                        (((u32)(src)<<4) & 0x00000010)
#define DED36MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*    Mask Register Fields DED35Mask    */
#define DED35MASK_WIDTH                                                       1
#define DED35MASK_SHIFT                                                       3
#define DED35MASK_MASK                                               0x00000008
#define DED35MASK_RD(src)                             (((src) & 0x00000008)>>3)
#define DED35MASK_WR(src)                        (((u32)(src)<<3) & 0x00000008)
#define DED35MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*    Mask Register Fields DED34Mask    */
#define DED34MASK_WIDTH                                                       1
#define DED34MASK_SHIFT                                                       2
#define DED34MASK_MASK                                               0x00000004
#define DED34MASK_RD(src)                             (((src) & 0x00000004)>>2)
#define DED34MASK_WR(src)                        (((u32)(src)<<2) & 0x00000004)
#define DED34MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*    Mask Register Fields DED33Mask    */
#define DED33MASK_WIDTH                                                       1
#define DED33MASK_SHIFT                                                       1
#define DED33MASK_MASK                                               0x00000002
#define DED33MASK_RD(src)                             (((src) & 0x00000002)>>1)
#define DED33MASK_WR(src)                        (((u32)(src)<<1) & 0x00000002)
#define DED33MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*    Mask Register Fields DED32Mask    */
#define DED32MASK_WIDTH                                                       1
#define DED32MASK_SHIFT                                                       0
#define DED32MASK_MASK                                               0x00000001
#define DED32MASK_RD(src)                                (((src) & 0x00000001))
#define DED32MASK_WR(src)                           (((u32)(src)) & 0x00000001)
#define DED32MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MDED_ERRL	*/ 
/*	 Fields MDED31	 */
#define MDED31_WIDTH                                                          1
#define MDED31_SHIFT                                                         31
#define MDED31_MASK                                                  0x80000000
#define MDED31_RD(src)                               (((src) & 0x80000000)>>31)
#define MDED31_WR(src)                          (((u32)(src)<<31) & 0x80000000)
#define MDED31_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields MDED30	 */
#define MDED30_WIDTH                                                          1
#define MDED30_SHIFT                                                         30
#define MDED30_MASK                                                  0x40000000
#define MDED30_RD(src)                               (((src) & 0x40000000)>>30)
#define MDED30_WR(src)                          (((u32)(src)<<30) & 0x40000000)
#define MDED30_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields MDED29	 */
#define MDED29_WIDTH                                                          1
#define MDED29_SHIFT                                                         29
#define MDED29_MASK                                                  0x20000000
#define MDED29_RD(src)                               (((src) & 0x20000000)>>29)
#define MDED29_WR(src)                          (((u32)(src)<<29) & 0x20000000)
#define MDED29_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields MDED28	 */
#define MDED28_WIDTH                                                          1
#define MDED28_SHIFT                                                         28
#define MDED28_MASK                                                  0x10000000
#define MDED28_RD(src)                               (((src) & 0x10000000)>>28)
#define MDED28_WR(src)                          (((u32)(src)<<28) & 0x10000000)
#define MDED28_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields MDED27	 */
#define MDED27_WIDTH                                                          1
#define MDED27_SHIFT                                                         27
#define MDED27_MASK                                                  0x08000000
#define MDED27_RD(src)                               (((src) & 0x08000000)>>27)
#define MDED27_WR(src)                          (((u32)(src)<<27) & 0x08000000)
#define MDED27_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields MDED26	 */
#define MDED26_WIDTH                                                          1
#define MDED26_SHIFT                                                         26
#define MDED26_MASK                                                  0x04000000
#define MDED26_RD(src)                               (((src) & 0x04000000)>>26)
#define MDED26_WR(src)                          (((u32)(src)<<26) & 0x04000000)
#define MDED26_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields MDED25	 */
#define MDED25_WIDTH                                                          1
#define MDED25_SHIFT                                                         25
#define MDED25_MASK                                                  0x02000000
#define MDED25_RD(src)                               (((src) & 0x02000000)>>25)
#define MDED25_WR(src)                          (((u32)(src)<<25) & 0x02000000)
#define MDED25_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields MDED24	 */
#define MDED24_WIDTH                                                          1
#define MDED24_SHIFT                                                         24
#define MDED24_MASK                                                  0x01000000
#define MDED24_RD(src)                               (((src) & 0x01000000)>>24)
#define MDED24_WR(src)                          (((u32)(src)<<24) & 0x01000000)
#define MDED24_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*	 Fields MDED23	 */
#define MDED23_WIDTH                                                          1
#define MDED23_SHIFT                                                         23
#define MDED23_MASK                                                  0x00800000
#define MDED23_RD(src)                               (((src) & 0x00800000)>>23)
#define MDED23_WR(src)                          (((u32)(src)<<23) & 0x00800000)
#define MDED23_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields MDED22	 */
#define MDED22_WIDTH                                                          1
#define MDED22_SHIFT                                                         22
#define MDED22_MASK                                                  0x00400000
#define MDED22_RD(src)                               (((src) & 0x00400000)>>22)
#define MDED22_WR(src)                          (((u32)(src)<<22) & 0x00400000)
#define MDED22_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields MDED21	 */
#define MDED21_WIDTH                                                          1
#define MDED21_SHIFT                                                         21
#define MDED21_MASK                                                  0x00200000
#define MDED21_RD(src)                               (((src) & 0x00200000)>>21)
#define MDED21_WR(src)                          (((u32)(src)<<21) & 0x00200000)
#define MDED21_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields MDED20	 */
#define MDED20_WIDTH                                                          1
#define MDED20_SHIFT                                                         20
#define MDED20_MASK                                                  0x00100000
#define MDED20_RD(src)                               (((src) & 0x00100000)>>20)
#define MDED20_WR(src)                          (((u32)(src)<<20) & 0x00100000)
#define MDED20_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields MDED19	 */
#define MDED19_WIDTH                                                          1
#define MDED19_SHIFT                                                         19
#define MDED19_MASK                                                  0x00080000
#define MDED19_RD(src)                               (((src) & 0x00080000)>>19)
#define MDED19_WR(src)                          (((u32)(src)<<19) & 0x00080000)
#define MDED19_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields MDED18	 */
#define MDED18_WIDTH                                                          1
#define MDED18_SHIFT                                                         18
#define MDED18_MASK                                                  0x00040000
#define MDED18_RD(src)                               (((src) & 0x00040000)>>18)
#define MDED18_WR(src)                          (((u32)(src)<<18) & 0x00040000)
#define MDED18_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields MDED17	 */
#define MDED17_WIDTH                                                          1
#define MDED17_SHIFT                                                         17
#define MDED17_MASK                                                  0x00020000
#define MDED17_RD(src)                               (((src) & 0x00020000)>>17)
#define MDED17_WR(src)                          (((u32)(src)<<17) & 0x00020000)
#define MDED17_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields MDED16	 */
#define MDED16_WIDTH                                                          1
#define MDED16_SHIFT                                                         16
#define MDED16_MASK                                                  0x00010000
#define MDED16_RD(src)                               (((src) & 0x00010000)>>16)
#define MDED16_WR(src)                          (((u32)(src)<<16) & 0x00010000)
#define MDED16_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields MDED15	 */
#define MDED15_WIDTH                                                          1
#define MDED15_SHIFT                                                         15
#define MDED15_MASK                                                  0x00008000
#define MDED15_RD(src)                               (((src) & 0x00008000)>>15)
#define MDED15_WR(src)                          (((u32)(src)<<15) & 0x00008000)
#define MDED15_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields MDED14	 */
#define MDED14_WIDTH                                                          1
#define MDED14_SHIFT                                                         14
#define MDED14_MASK                                                  0x00004000
#define MDED14_RD(src)                               (((src) & 0x00004000)>>14)
#define MDED14_WR(src)                          (((u32)(src)<<14) & 0x00004000)
#define MDED14_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields MDED13	 */
#define MDED13_WIDTH                                                          1
#define MDED13_SHIFT                                                         13
#define MDED13_MASK                                                  0x00002000
#define MDED13_RD(src)                               (((src) & 0x00002000)>>13)
#define MDED13_WR(src)                          (((u32)(src)<<13) & 0x00002000)
#define MDED13_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields MDED12	 */
#define MDED12_WIDTH                                                          1
#define MDED12_SHIFT                                                         12
#define MDED12_MASK                                                  0x00001000
#define MDED12_RD(src)                               (((src) & 0x00001000)>>12)
#define MDED12_WR(src)                          (((u32)(src)<<12) & 0x00001000)
#define MDED12_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields MDED11	 */
#define MDED11_WIDTH                                                          1
#define MDED11_SHIFT                                                         11
#define MDED11_MASK                                                  0x00000800
#define MDED11_RD(src)                               (((src) & 0x00000800)>>11)
#define MDED11_WR(src)                          (((u32)(src)<<11) & 0x00000800)
#define MDED11_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields MDED10	 */
#define MDED10_WIDTH                                                          1
#define MDED10_SHIFT                                                         10
#define MDED10_MASK                                                  0x00000400
#define MDED10_RD(src)                               (((src) & 0x00000400)>>10)
#define MDED10_WR(src)                          (((u32)(src)<<10) & 0x00000400)
#define MDED10_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields MDED9	 */
#define MDED9_WIDTH                                                           1
#define MDED9_SHIFT                                                           9
#define MDED9_MASK                                                   0x00000200
#define MDED9_RD(src)                                 (((src) & 0x00000200)>>9)
#define MDED9_WR(src)                            (((u32)(src)<<9) & 0x00000200)
#define MDED9_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields MDED8	 */
#define MDED8_WIDTH                                                           1
#define MDED8_SHIFT                                                           8
#define MDED8_MASK                                                   0x00000100
#define MDED8_RD(src)                                 (((src) & 0x00000100)>>8)
#define MDED8_WR(src)                            (((u32)(src)<<8) & 0x00000100)
#define MDED8_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields MDED7	 */
#define MDED7_WIDTH                                                           1
#define MDED7_SHIFT                                                           7
#define MDED7_MASK                                                   0x00000080
#define MDED7_RD(src)                                 (((src) & 0x00000080)>>7)
#define MDED7_WR(src)                            (((u32)(src)<<7) & 0x00000080)
#define MDED7_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields MDED6	 */
#define MDED6_WIDTH                                                           1
#define MDED6_SHIFT                                                           6
#define MDED6_MASK                                                   0x00000040
#define MDED6_RD(src)                                 (((src) & 0x00000040)>>6)
#define MDED6_WR(src)                            (((u32)(src)<<6) & 0x00000040)
#define MDED6_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields MDED5	 */
#define MDED5_WIDTH                                                           1
#define MDED5_SHIFT                                                           5
#define MDED5_MASK                                                   0x00000020
#define MDED5_RD(src)                                 (((src) & 0x00000020)>>5)
#define MDED5_WR(src)                            (((u32)(src)<<5) & 0x00000020)
#define MDED5_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields MDED4	 */
#define MDED4_WIDTH                                                           1
#define MDED4_SHIFT                                                           4
#define MDED4_MASK                                                   0x00000010
#define MDED4_RD(src)                                 (((src) & 0x00000010)>>4)
#define MDED4_WR(src)                            (((u32)(src)<<4) & 0x00000010)
#define MDED4_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields MDED3	 */
#define MDED3_WIDTH                                                           1
#define MDED3_SHIFT                                                           3
#define MDED3_MASK                                                   0x00000008
#define MDED3_RD(src)                                 (((src) & 0x00000008)>>3)
#define MDED3_WR(src)                            (((u32)(src)<<3) & 0x00000008)
#define MDED3_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields MDED2	 */
#define MDED2_WIDTH                                                           1
#define MDED2_SHIFT                                                           2
#define MDED2_MASK                                                   0x00000004
#define MDED2_RD(src)                                 (((src) & 0x00000004)>>2)
#define MDED2_WR(src)                            (((u32)(src)<<2) & 0x00000004)
#define MDED2_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields MDED1	 */
#define MDED1_WIDTH                                                           1
#define MDED1_SHIFT                                                           1
#define MDED1_MASK                                                   0x00000002
#define MDED1_RD(src)                                 (((src) & 0x00000002)>>1)
#define MDED1_WR(src)                            (((u32)(src)<<1) & 0x00000002)
#define MDED1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields MDED0	 */
#define MDED0_WIDTH                                                           1
#define MDED0_SHIFT                                                           0
#define MDED0_MASK                                                   0x00000001
#define MDED0_RD(src)                                    (((src) & 0x00000001))
#define MDED0_WR(src)                               (((u32)(src)) & 0x00000001)
#define MDED0_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MDED_ERRLMask	*/
/*    Mask Register Fields MDED31Mask    */
#define MDED31MASK_WIDTH                                                      1
#define MDED31MASK_SHIFT                                                     31
#define MDED31MASK_MASK                                              0x80000000
#define MDED31MASK_RD(src)                           (((src) & 0x80000000)>>31)
#define MDED31MASK_WR(src)                      (((u32)(src)<<31) & 0x80000000)
#define MDED31MASK_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*    Mask Register Fields MDED30Mask    */
#define MDED30MASK_WIDTH                                                      1
#define MDED30MASK_SHIFT                                                     30
#define MDED30MASK_MASK                                              0x40000000
#define MDED30MASK_RD(src)                           (((src) & 0x40000000)>>30)
#define MDED30MASK_WR(src)                      (((u32)(src)<<30) & 0x40000000)
#define MDED30MASK_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*    Mask Register Fields MDED29Mask    */
#define MDED29MASK_WIDTH                                                      1
#define MDED29MASK_SHIFT                                                     29
#define MDED29MASK_MASK                                              0x20000000
#define MDED29MASK_RD(src)                           (((src) & 0x20000000)>>29)
#define MDED29MASK_WR(src)                      (((u32)(src)<<29) & 0x20000000)
#define MDED29MASK_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*    Mask Register Fields MDED28Mask    */
#define MDED28MASK_WIDTH                                                      1
#define MDED28MASK_SHIFT                                                     28
#define MDED28MASK_MASK                                              0x10000000
#define MDED28MASK_RD(src)                           (((src) & 0x10000000)>>28)
#define MDED28MASK_WR(src)                      (((u32)(src)<<28) & 0x10000000)
#define MDED28MASK_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*    Mask Register Fields MDED27Mask    */
#define MDED27MASK_WIDTH                                                      1
#define MDED27MASK_SHIFT                                                     27
#define MDED27MASK_MASK                                              0x08000000
#define MDED27MASK_RD(src)                           (((src) & 0x08000000)>>27)
#define MDED27MASK_WR(src)                      (((u32)(src)<<27) & 0x08000000)
#define MDED27MASK_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*    Mask Register Fields MDED26Mask    */
#define MDED26MASK_WIDTH                                                      1
#define MDED26MASK_SHIFT                                                     26
#define MDED26MASK_MASK                                              0x04000000
#define MDED26MASK_RD(src)                           (((src) & 0x04000000)>>26)
#define MDED26MASK_WR(src)                      (((u32)(src)<<26) & 0x04000000)
#define MDED26MASK_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*    Mask Register Fields MDED25Mask    */
#define MDED25MASK_WIDTH                                                      1
#define MDED25MASK_SHIFT                                                     25
#define MDED25MASK_MASK                                              0x02000000
#define MDED25MASK_RD(src)                           (((src) & 0x02000000)>>25)
#define MDED25MASK_WR(src)                      (((u32)(src)<<25) & 0x02000000)
#define MDED25MASK_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*    Mask Register Fields MDED24Mask    */
#define MDED24MASK_WIDTH                                                      1
#define MDED24MASK_SHIFT                                                     24
#define MDED24MASK_MASK                                              0x01000000
#define MDED24MASK_RD(src)                           (((src) & 0x01000000)>>24)
#define MDED24MASK_WR(src)                      (((u32)(src)<<24) & 0x01000000)
#define MDED24MASK_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*    Mask Register Fields MDED23Mask    */
#define MDED23MASK_WIDTH                                                      1
#define MDED23MASK_SHIFT                                                     23
#define MDED23MASK_MASK                                              0x00800000
#define MDED23MASK_RD(src)                           (((src) & 0x00800000)>>23)
#define MDED23MASK_WR(src)                      (((u32)(src)<<23) & 0x00800000)
#define MDED23MASK_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*    Mask Register Fields MDED22Mask    */
#define MDED22MASK_WIDTH                                                      1
#define MDED22MASK_SHIFT                                                     22
#define MDED22MASK_MASK                                              0x00400000
#define MDED22MASK_RD(src)                           (((src) & 0x00400000)>>22)
#define MDED22MASK_WR(src)                      (((u32)(src)<<22) & 0x00400000)
#define MDED22MASK_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*    Mask Register Fields MDED21Mask    */
#define MDED21MASK_WIDTH                                                      1
#define MDED21MASK_SHIFT                                                     21
#define MDED21MASK_MASK                                              0x00200000
#define MDED21MASK_RD(src)                           (((src) & 0x00200000)>>21)
#define MDED21MASK_WR(src)                      (((u32)(src)<<21) & 0x00200000)
#define MDED21MASK_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*    Mask Register Fields MDED20Mask    */
#define MDED20MASK_WIDTH                                                      1
#define MDED20MASK_SHIFT                                                     20
#define MDED20MASK_MASK                                              0x00100000
#define MDED20MASK_RD(src)                           (((src) & 0x00100000)>>20)
#define MDED20MASK_WR(src)                      (((u32)(src)<<20) & 0x00100000)
#define MDED20MASK_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*    Mask Register Fields MDED19Mask    */
#define MDED19MASK_WIDTH                                                      1
#define MDED19MASK_SHIFT                                                     19
#define MDED19MASK_MASK                                              0x00080000
#define MDED19MASK_RD(src)                           (((src) & 0x00080000)>>19)
#define MDED19MASK_WR(src)                      (((u32)(src)<<19) & 0x00080000)
#define MDED19MASK_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*    Mask Register Fields MDED18Mask    */
#define MDED18MASK_WIDTH                                                      1
#define MDED18MASK_SHIFT                                                     18
#define MDED18MASK_MASK                                              0x00040000
#define MDED18MASK_RD(src)                           (((src) & 0x00040000)>>18)
#define MDED18MASK_WR(src)                      (((u32)(src)<<18) & 0x00040000)
#define MDED18MASK_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*    Mask Register Fields MDED17Mask    */
#define MDED17MASK_WIDTH                                                      1
#define MDED17MASK_SHIFT                                                     17
#define MDED17MASK_MASK                                              0x00020000
#define MDED17MASK_RD(src)                           (((src) & 0x00020000)>>17)
#define MDED17MASK_WR(src)                      (((u32)(src)<<17) & 0x00020000)
#define MDED17MASK_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*    Mask Register Fields MDED16Mask    */
#define MDED16MASK_WIDTH                                                      1
#define MDED16MASK_SHIFT                                                     16
#define MDED16MASK_MASK                                              0x00010000
#define MDED16MASK_RD(src)                           (((src) & 0x00010000)>>16)
#define MDED16MASK_WR(src)                      (((u32)(src)<<16) & 0x00010000)
#define MDED16MASK_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*    Mask Register Fields MDED15Mask    */
#define MDED15MASK_WIDTH                                                      1
#define MDED15MASK_SHIFT                                                     15
#define MDED15MASK_MASK                                              0x00008000
#define MDED15MASK_RD(src)                           (((src) & 0x00008000)>>15)
#define MDED15MASK_WR(src)                      (((u32)(src)<<15) & 0x00008000)
#define MDED15MASK_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*    Mask Register Fields MDED14Mask    */
#define MDED14MASK_WIDTH                                                      1
#define MDED14MASK_SHIFT                                                     14
#define MDED14MASK_MASK                                              0x00004000
#define MDED14MASK_RD(src)                           (((src) & 0x00004000)>>14)
#define MDED14MASK_WR(src)                      (((u32)(src)<<14) & 0x00004000)
#define MDED14MASK_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*    Mask Register Fields MDED13Mask    */
#define MDED13MASK_WIDTH                                                      1
#define MDED13MASK_SHIFT                                                     13
#define MDED13MASK_MASK                                              0x00002000
#define MDED13MASK_RD(src)                           (((src) & 0x00002000)>>13)
#define MDED13MASK_WR(src)                      (((u32)(src)<<13) & 0x00002000)
#define MDED13MASK_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*    Mask Register Fields MDED12Mask    */
#define MDED12MASK_WIDTH                                                      1
#define MDED12MASK_SHIFT                                                     12
#define MDED12MASK_MASK                                              0x00001000
#define MDED12MASK_RD(src)                           (((src) & 0x00001000)>>12)
#define MDED12MASK_WR(src)                      (((u32)(src)<<12) & 0x00001000)
#define MDED12MASK_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*    Mask Register Fields MDED11Mask    */
#define MDED11MASK_WIDTH                                                      1
#define MDED11MASK_SHIFT                                                     11
#define MDED11MASK_MASK                                              0x00000800
#define MDED11MASK_RD(src)                           (((src) & 0x00000800)>>11)
#define MDED11MASK_WR(src)                      (((u32)(src)<<11) & 0x00000800)
#define MDED11MASK_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*    Mask Register Fields MDED10Mask    */
#define MDED10MASK_WIDTH                                                      1
#define MDED10MASK_SHIFT                                                     10
#define MDED10MASK_MASK                                              0x00000400
#define MDED10MASK_RD(src)                           (((src) & 0x00000400)>>10)
#define MDED10MASK_WR(src)                      (((u32)(src)<<10) & 0x00000400)
#define MDED10MASK_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*    Mask Register Fields MDED9Mask    */
#define MDED9MASK_WIDTH                                                       1
#define MDED9MASK_SHIFT                                                       9
#define MDED9MASK_MASK                                               0x00000200
#define MDED9MASK_RD(src)                             (((src) & 0x00000200)>>9)
#define MDED9MASK_WR(src)                        (((u32)(src)<<9) & 0x00000200)
#define MDED9MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*    Mask Register Fields MDED8Mask    */
#define MDED8MASK_WIDTH                                                       1
#define MDED8MASK_SHIFT                                                       8
#define MDED8MASK_MASK                                               0x00000100
#define MDED8MASK_RD(src)                             (((src) & 0x00000100)>>8)
#define MDED8MASK_WR(src)                        (((u32)(src)<<8) & 0x00000100)
#define MDED8MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*    Mask Register Fields MDED7Mask    */
#define MDED7MASK_WIDTH                                                       1
#define MDED7MASK_SHIFT                                                       7
#define MDED7MASK_MASK                                               0x00000080
#define MDED7MASK_RD(src)                             (((src) & 0x00000080)>>7)
#define MDED7MASK_WR(src)                        (((u32)(src)<<7) & 0x00000080)
#define MDED7MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*    Mask Register Fields MDED6Mask    */
#define MDED6MASK_WIDTH                                                       1
#define MDED6MASK_SHIFT                                                       6
#define MDED6MASK_MASK                                               0x00000040
#define MDED6MASK_RD(src)                             (((src) & 0x00000040)>>6)
#define MDED6MASK_WR(src)                        (((u32)(src)<<6) & 0x00000040)
#define MDED6MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*    Mask Register Fields MDED5Mask    */
#define MDED5MASK_WIDTH                                                       1
#define MDED5MASK_SHIFT                                                       5
#define MDED5MASK_MASK                                               0x00000020
#define MDED5MASK_RD(src)                             (((src) & 0x00000020)>>5)
#define MDED5MASK_WR(src)                        (((u32)(src)<<5) & 0x00000020)
#define MDED5MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*    Mask Register Fields MDED4Mask    */
#define MDED4MASK_WIDTH                                                       1
#define MDED4MASK_SHIFT                                                       4
#define MDED4MASK_MASK                                               0x00000010
#define MDED4MASK_RD(src)                             (((src) & 0x00000010)>>4)
#define MDED4MASK_WR(src)                        (((u32)(src)<<4) & 0x00000010)
#define MDED4MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*    Mask Register Fields MDED3Mask    */
#define MDED3MASK_WIDTH                                                       1
#define MDED3MASK_SHIFT                                                       3
#define MDED3MASK_MASK                                               0x00000008
#define MDED3MASK_RD(src)                             (((src) & 0x00000008)>>3)
#define MDED3MASK_WR(src)                        (((u32)(src)<<3) & 0x00000008)
#define MDED3MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*    Mask Register Fields MDED2Mask    */
#define MDED2MASK_WIDTH                                                       1
#define MDED2MASK_SHIFT                                                       2
#define MDED2MASK_MASK                                               0x00000004
#define MDED2MASK_RD(src)                             (((src) & 0x00000004)>>2)
#define MDED2MASK_WR(src)                        (((u32)(src)<<2) & 0x00000004)
#define MDED2MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*    Mask Register Fields MDED1Mask    */
#define MDED1MASK_WIDTH                                                       1
#define MDED1MASK_SHIFT                                                       1
#define MDED1MASK_MASK                                               0x00000002
#define MDED1MASK_RD(src)                             (((src) & 0x00000002)>>1)
#define MDED1MASK_WR(src)                        (((u32)(src)<<1) & 0x00000002)
#define MDED1MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*    Mask Register Fields MDED0Mask    */
#define MDED0MASK_WIDTH                                                       1
#define MDED0MASK_SHIFT                                                       0
#define MDED0MASK_MASK                                               0x00000001
#define MDED0MASK_RD(src)                                (((src) & 0x00000001))
#define MDED0MASK_WR(src)                           (((u32)(src)) & 0x00000001)
#define MDED0MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MDED_ERRH	*/ 
/*	 Fields MDED63	 */
#define MDED63_WIDTH                                                          1
#define MDED63_SHIFT                                                         31
#define MDED63_MASK                                                  0x80000000
#define MDED63_RD(src)                               (((src) & 0x80000000)>>31)
#define MDED63_WR(src)                          (((u32)(src)<<31) & 0x80000000)
#define MDED63_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields MDED62	 */
#define MDED62_WIDTH                                                          1
#define MDED62_SHIFT                                                         30
#define MDED62_MASK                                                  0x40000000
#define MDED62_RD(src)                               (((src) & 0x40000000)>>30)
#define MDED62_WR(src)                          (((u32)(src)<<30) & 0x40000000)
#define MDED62_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields MDED61	 */
#define MDED61_WIDTH                                                          1
#define MDED61_SHIFT                                                         29
#define MDED61_MASK                                                  0x20000000
#define MDED61_RD(src)                               (((src) & 0x20000000)>>29)
#define MDED61_WR(src)                          (((u32)(src)<<29) & 0x20000000)
#define MDED61_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields MDED60	 */
#define MDED60_WIDTH                                                          1
#define MDED60_SHIFT                                                         28
#define MDED60_MASK                                                  0x10000000
#define MDED60_RD(src)                               (((src) & 0x10000000)>>28)
#define MDED60_WR(src)                          (((u32)(src)<<28) & 0x10000000)
#define MDED60_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields MDED59	 */
#define MDED59_WIDTH                                                          1
#define MDED59_SHIFT                                                         27
#define MDED59_MASK                                                  0x08000000
#define MDED59_RD(src)                               (((src) & 0x08000000)>>27)
#define MDED59_WR(src)                          (((u32)(src)<<27) & 0x08000000)
#define MDED59_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields MDED58	 */
#define MDED58_WIDTH                                                          1
#define MDED58_SHIFT                                                         26
#define MDED58_MASK                                                  0x04000000
#define MDED58_RD(src)                               (((src) & 0x04000000)>>26)
#define MDED58_WR(src)                          (((u32)(src)<<26) & 0x04000000)
#define MDED58_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields MDED57	 */
#define MDED57_WIDTH                                                          1
#define MDED57_SHIFT                                                         25
#define MDED57_MASK                                                  0x02000000
#define MDED57_RD(src)                               (((src) & 0x02000000)>>25)
#define MDED57_WR(src)                          (((u32)(src)<<25) & 0x02000000)
#define MDED57_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields MDED56	 */
#define MDED56_WIDTH                                                          1
#define MDED56_SHIFT                                                         24
#define MDED56_MASK                                                  0x01000000
#define MDED56_RD(src)                               (((src) & 0x01000000)>>24)
#define MDED56_WR(src)                          (((u32)(src)<<24) & 0x01000000)
#define MDED56_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*	 Fields MDED55	 */
#define MDED55_WIDTH                                                          1
#define MDED55_SHIFT                                                         23
#define MDED55_MASK                                                  0x00800000
#define MDED55_RD(src)                               (((src) & 0x00800000)>>23)
#define MDED55_WR(src)                          (((u32)(src)<<23) & 0x00800000)
#define MDED55_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields MDED54	 */
#define MDED54_WIDTH                                                          1
#define MDED54_SHIFT                                                         22
#define MDED54_MASK                                                  0x00400000
#define MDED54_RD(src)                               (((src) & 0x00400000)>>22)
#define MDED54_WR(src)                          (((u32)(src)<<22) & 0x00400000)
#define MDED54_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields MDED53	 */
#define MDED53_WIDTH                                                          1
#define MDED53_SHIFT                                                         21
#define MDED53_MASK                                                  0x00200000
#define MDED53_RD(src)                               (((src) & 0x00200000)>>21)
#define MDED53_WR(src)                          (((u32)(src)<<21) & 0x00200000)
#define MDED53_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields MDED52	 */
#define MDED52_WIDTH                                                          1
#define MDED52_SHIFT                                                         20
#define MDED52_MASK                                                  0x00100000
#define MDED52_RD(src)                               (((src) & 0x00100000)>>20)
#define MDED52_WR(src)                          (((u32)(src)<<20) & 0x00100000)
#define MDED52_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields MDED51	 */
#define MDED51_WIDTH                                                          1
#define MDED51_SHIFT                                                         19
#define MDED51_MASK                                                  0x00080000
#define MDED51_RD(src)                               (((src) & 0x00080000)>>19)
#define MDED51_WR(src)                          (((u32)(src)<<19) & 0x00080000)
#define MDED51_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields MDED50	 */
#define MDED50_WIDTH                                                          1
#define MDED50_SHIFT                                                         18
#define MDED50_MASK                                                  0x00040000
#define MDED50_RD(src)                               (((src) & 0x00040000)>>18)
#define MDED50_WR(src)                          (((u32)(src)<<18) & 0x00040000)
#define MDED50_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields MDED49	 */
#define MDED49_WIDTH                                                          1
#define MDED49_SHIFT                                                         17
#define MDED49_MASK                                                  0x00020000
#define MDED49_RD(src)                               (((src) & 0x00020000)>>17)
#define MDED49_WR(src)                          (((u32)(src)<<17) & 0x00020000)
#define MDED49_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields MDED48	 */
#define MDED48_WIDTH                                                          1
#define MDED48_SHIFT                                                         16
#define MDED48_MASK                                                  0x00010000
#define MDED48_RD(src)                               (((src) & 0x00010000)>>16)
#define MDED48_WR(src)                          (((u32)(src)<<16) & 0x00010000)
#define MDED48_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields MDED47	 */
#define MDED47_WIDTH                                                          1
#define MDED47_SHIFT                                                         15
#define MDED47_MASK                                                  0x00008000
#define MDED47_RD(src)                               (((src) & 0x00008000)>>15)
#define MDED47_WR(src)                          (((u32)(src)<<15) & 0x00008000)
#define MDED47_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields MDED46	 */
#define MDED46_WIDTH                                                          1
#define MDED46_SHIFT                                                         14
#define MDED46_MASK                                                  0x00004000
#define MDED46_RD(src)                               (((src) & 0x00004000)>>14)
#define MDED46_WR(src)                          (((u32)(src)<<14) & 0x00004000)
#define MDED46_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields MDED45	 */
#define MDED45_WIDTH                                                          1
#define MDED45_SHIFT                                                         13
#define MDED45_MASK                                                  0x00002000
#define MDED45_RD(src)                               (((src) & 0x00002000)>>13)
#define MDED45_WR(src)                          (((u32)(src)<<13) & 0x00002000)
#define MDED45_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields MDED44	 */
#define MDED44_WIDTH                                                          1
#define MDED44_SHIFT                                                         12
#define MDED44_MASK                                                  0x00001000
#define MDED44_RD(src)                               (((src) & 0x00001000)>>12)
#define MDED44_WR(src)                          (((u32)(src)<<12) & 0x00001000)
#define MDED44_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields MDED43	 */
#define MDED43_WIDTH                                                          1
#define MDED43_SHIFT                                                         11
#define MDED43_MASK                                                  0x00000800
#define MDED43_RD(src)                               (((src) & 0x00000800)>>11)
#define MDED43_WR(src)                          (((u32)(src)<<11) & 0x00000800)
#define MDED43_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields MDED42	 */
#define MDED42_WIDTH                                                          1
#define MDED42_SHIFT                                                         10
#define MDED42_MASK                                                  0x00000400
#define MDED42_RD(src)                               (((src) & 0x00000400)>>10)
#define MDED42_WR(src)                          (((u32)(src)<<10) & 0x00000400)
#define MDED42_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields MDED41	 */
#define MDED41_WIDTH                                                          1
#define MDED41_SHIFT                                                          9
#define MDED41_MASK                                                  0x00000200
#define MDED41_RD(src)                                (((src) & 0x00000200)>>9)
#define MDED41_WR(src)                           (((u32)(src)<<9) & 0x00000200)
#define MDED41_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields MDED40	 */
#define MDED40_WIDTH                                                          1
#define MDED40_SHIFT                                                          8
#define MDED40_MASK                                                  0x00000100
#define MDED40_RD(src)                                (((src) & 0x00000100)>>8)
#define MDED40_WR(src)                           (((u32)(src)<<8) & 0x00000100)
#define MDED40_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields MDED39	 */
#define MDED39_WIDTH                                                          1
#define MDED39_SHIFT                                                          7
#define MDED39_MASK                                                  0x00000080
#define MDED39_RD(src)                                (((src) & 0x00000080)>>7)
#define MDED39_WR(src)                           (((u32)(src)<<7) & 0x00000080)
#define MDED39_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields MDED38	 */
#define MDED38_WIDTH                                                          1
#define MDED38_SHIFT                                                          6
#define MDED38_MASK                                                  0x00000040
#define MDED38_RD(src)                                (((src) & 0x00000040)>>6)
#define MDED38_WR(src)                           (((u32)(src)<<6) & 0x00000040)
#define MDED38_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields MDED37	 */
#define MDED37_WIDTH                                                          1
#define MDED37_SHIFT                                                          5
#define MDED37_MASK                                                  0x00000020
#define MDED37_RD(src)                                (((src) & 0x00000020)>>5)
#define MDED37_WR(src)                           (((u32)(src)<<5) & 0x00000020)
#define MDED37_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields MDED36	 */
#define MDED36_WIDTH                                                          1
#define MDED36_SHIFT                                                          4
#define MDED36_MASK                                                  0x00000010
#define MDED36_RD(src)                                (((src) & 0x00000010)>>4)
#define MDED36_WR(src)                           (((u32)(src)<<4) & 0x00000010)
#define MDED36_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields MDED35	 */
#define MDED35_WIDTH                                                          1
#define MDED35_SHIFT                                                          3
#define MDED35_MASK                                                  0x00000008
#define MDED35_RD(src)                                (((src) & 0x00000008)>>3)
#define MDED35_WR(src)                           (((u32)(src)<<3) & 0x00000008)
#define MDED35_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields MDED34	 */
#define MDED34_WIDTH                                                          1
#define MDED34_SHIFT                                                          2
#define MDED34_MASK                                                  0x00000004
#define MDED34_RD(src)                                (((src) & 0x00000004)>>2)
#define MDED34_WR(src)                           (((u32)(src)<<2) & 0x00000004)
#define MDED34_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields MDED33	 */
#define MDED33_WIDTH                                                          1
#define MDED33_SHIFT                                                          1
#define MDED33_MASK                                                  0x00000002
#define MDED33_RD(src)                                (((src) & 0x00000002)>>1)
#define MDED33_WR(src)                           (((u32)(src)<<1) & 0x00000002)
#define MDED33_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields MDED32	 */
#define MDED32_WIDTH                                                          1
#define MDED32_SHIFT                                                          0
#define MDED32_MASK                                                  0x00000001
#define MDED32_RD(src)                                   (((src) & 0x00000001))
#define MDED32_WR(src)                              (((u32)(src)) & 0x00000001)
#define MDED32_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MDED_ERRHMask	*/
/*    Mask Register Fields MDED63Mask    */
#define MDED63MASK_WIDTH                                                      1
#define MDED63MASK_SHIFT                                                     31
#define MDED63MASK_MASK                                              0x80000000
#define MDED63MASK_RD(src)                           (((src) & 0x80000000)>>31)
#define MDED63MASK_WR(src)                      (((u32)(src)<<31) & 0x80000000)
#define MDED63MASK_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*    Mask Register Fields MDED62Mask    */
#define MDED62MASK_WIDTH                                                      1
#define MDED62MASK_SHIFT                                                     30
#define MDED62MASK_MASK                                              0x40000000
#define MDED62MASK_RD(src)                           (((src) & 0x40000000)>>30)
#define MDED62MASK_WR(src)                      (((u32)(src)<<30) & 0x40000000)
#define MDED62MASK_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*    Mask Register Fields MDED61Mask    */
#define MDED61MASK_WIDTH                                                      1
#define MDED61MASK_SHIFT                                                     29
#define MDED61MASK_MASK                                              0x20000000
#define MDED61MASK_RD(src)                           (((src) & 0x20000000)>>29)
#define MDED61MASK_WR(src)                      (((u32)(src)<<29) & 0x20000000)
#define MDED61MASK_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*    Mask Register Fields MDED60Mask    */
#define MDED60MASK_WIDTH                                                      1
#define MDED60MASK_SHIFT                                                     28
#define MDED60MASK_MASK                                              0x10000000
#define MDED60MASK_RD(src)                           (((src) & 0x10000000)>>28)
#define MDED60MASK_WR(src)                      (((u32)(src)<<28) & 0x10000000)
#define MDED60MASK_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*    Mask Register Fields MDED59Mask    */
#define MDED59MASK_WIDTH                                                      1
#define MDED59MASK_SHIFT                                                     27
#define MDED59MASK_MASK                                              0x08000000
#define MDED59MASK_RD(src)                           (((src) & 0x08000000)>>27)
#define MDED59MASK_WR(src)                      (((u32)(src)<<27) & 0x08000000)
#define MDED59MASK_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*    Mask Register Fields MDED58Mask    */
#define MDED58MASK_WIDTH                                                      1
#define MDED58MASK_SHIFT                                                     26
#define MDED58MASK_MASK                                              0x04000000
#define MDED58MASK_RD(src)                           (((src) & 0x04000000)>>26)
#define MDED58MASK_WR(src)                      (((u32)(src)<<26) & 0x04000000)
#define MDED58MASK_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*    Mask Register Fields MDED57Mask    */
#define MDED57MASK_WIDTH                                                      1
#define MDED57MASK_SHIFT                                                     25
#define MDED57MASK_MASK                                              0x02000000
#define MDED57MASK_RD(src)                           (((src) & 0x02000000)>>25)
#define MDED57MASK_WR(src)                      (((u32)(src)<<25) & 0x02000000)
#define MDED57MASK_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*    Mask Register Fields MDED56Mask    */
#define MDED56MASK_WIDTH                                                      1
#define MDED56MASK_SHIFT                                                     24
#define MDED56MASK_MASK                                              0x01000000
#define MDED56MASK_RD(src)                           (((src) & 0x01000000)>>24)
#define MDED56MASK_WR(src)                      (((u32)(src)<<24) & 0x01000000)
#define MDED56MASK_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*    Mask Register Fields MDED55Mask    */
#define MDED55MASK_WIDTH                                                      1
#define MDED55MASK_SHIFT                                                     23
#define MDED55MASK_MASK                                              0x00800000
#define MDED55MASK_RD(src)                           (((src) & 0x00800000)>>23)
#define MDED55MASK_WR(src)                      (((u32)(src)<<23) & 0x00800000)
#define MDED55MASK_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*    Mask Register Fields MDED54Mask    */
#define MDED54MASK_WIDTH                                                      1
#define MDED54MASK_SHIFT                                                     22
#define MDED54MASK_MASK                                              0x00400000
#define MDED54MASK_RD(src)                           (((src) & 0x00400000)>>22)
#define MDED54MASK_WR(src)                      (((u32)(src)<<22) & 0x00400000)
#define MDED54MASK_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*    Mask Register Fields MDED53Mask    */
#define MDED53MASK_WIDTH                                                      1
#define MDED53MASK_SHIFT                                                     21
#define MDED53MASK_MASK                                              0x00200000
#define MDED53MASK_RD(src)                           (((src) & 0x00200000)>>21)
#define MDED53MASK_WR(src)                      (((u32)(src)<<21) & 0x00200000)
#define MDED53MASK_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*    Mask Register Fields MDED52Mask    */
#define MDED52MASK_WIDTH                                                      1
#define MDED52MASK_SHIFT                                                     20
#define MDED52MASK_MASK                                              0x00100000
#define MDED52MASK_RD(src)                           (((src) & 0x00100000)>>20)
#define MDED52MASK_WR(src)                      (((u32)(src)<<20) & 0x00100000)
#define MDED52MASK_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*    Mask Register Fields MDED51Mask    */
#define MDED51MASK_WIDTH                                                      1
#define MDED51MASK_SHIFT                                                     19
#define MDED51MASK_MASK                                              0x00080000
#define MDED51MASK_RD(src)                           (((src) & 0x00080000)>>19)
#define MDED51MASK_WR(src)                      (((u32)(src)<<19) & 0x00080000)
#define MDED51MASK_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*    Mask Register Fields MDED50Mask    */
#define MDED50MASK_WIDTH                                                      1
#define MDED50MASK_SHIFT                                                     18
#define MDED50MASK_MASK                                              0x00040000
#define MDED50MASK_RD(src)                           (((src) & 0x00040000)>>18)
#define MDED50MASK_WR(src)                      (((u32)(src)<<18) & 0x00040000)
#define MDED50MASK_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*    Mask Register Fields MDED49Mask    */
#define MDED49MASK_WIDTH                                                      1
#define MDED49MASK_SHIFT                                                     17
#define MDED49MASK_MASK                                              0x00020000
#define MDED49MASK_RD(src)                           (((src) & 0x00020000)>>17)
#define MDED49MASK_WR(src)                      (((u32)(src)<<17) & 0x00020000)
#define MDED49MASK_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*    Mask Register Fields MDED48Mask    */
#define MDED48MASK_WIDTH                                                      1
#define MDED48MASK_SHIFT                                                     16
#define MDED48MASK_MASK                                              0x00010000
#define MDED48MASK_RD(src)                           (((src) & 0x00010000)>>16)
#define MDED48MASK_WR(src)                      (((u32)(src)<<16) & 0x00010000)
#define MDED48MASK_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*    Mask Register Fields MDED47Mask    */
#define MDED47MASK_WIDTH                                                      1
#define MDED47MASK_SHIFT                                                     15
#define MDED47MASK_MASK                                              0x00008000
#define MDED47MASK_RD(src)                           (((src) & 0x00008000)>>15)
#define MDED47MASK_WR(src)                      (((u32)(src)<<15) & 0x00008000)
#define MDED47MASK_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*    Mask Register Fields MDED46Mask    */
#define MDED46MASK_WIDTH                                                      1
#define MDED46MASK_SHIFT                                                     14
#define MDED46MASK_MASK                                              0x00004000
#define MDED46MASK_RD(src)                           (((src) & 0x00004000)>>14)
#define MDED46MASK_WR(src)                      (((u32)(src)<<14) & 0x00004000)
#define MDED46MASK_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*    Mask Register Fields MDED45Mask    */
#define MDED45MASK_WIDTH                                                      1
#define MDED45MASK_SHIFT                                                     13
#define MDED45MASK_MASK                                              0x00002000
#define MDED45MASK_RD(src)                           (((src) & 0x00002000)>>13)
#define MDED45MASK_WR(src)                      (((u32)(src)<<13) & 0x00002000)
#define MDED45MASK_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*    Mask Register Fields MDED44Mask    */
#define MDED44MASK_WIDTH                                                      1
#define MDED44MASK_SHIFT                                                     12
#define MDED44MASK_MASK                                              0x00001000
#define MDED44MASK_RD(src)                           (((src) & 0x00001000)>>12)
#define MDED44MASK_WR(src)                      (((u32)(src)<<12) & 0x00001000)
#define MDED44MASK_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*    Mask Register Fields MDED43Mask    */
#define MDED43MASK_WIDTH                                                      1
#define MDED43MASK_SHIFT                                                     11
#define MDED43MASK_MASK                                              0x00000800
#define MDED43MASK_RD(src)                           (((src) & 0x00000800)>>11)
#define MDED43MASK_WR(src)                      (((u32)(src)<<11) & 0x00000800)
#define MDED43MASK_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*    Mask Register Fields MDED42Mask    */
#define MDED42MASK_WIDTH                                                      1
#define MDED42MASK_SHIFT                                                     10
#define MDED42MASK_MASK                                              0x00000400
#define MDED42MASK_RD(src)                           (((src) & 0x00000400)>>10)
#define MDED42MASK_WR(src)                      (((u32)(src)<<10) & 0x00000400)
#define MDED42MASK_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*    Mask Register Fields MDED41Mask    */
#define MDED41MASK_WIDTH                                                      1
#define MDED41MASK_SHIFT                                                      9
#define MDED41MASK_MASK                                              0x00000200
#define MDED41MASK_RD(src)                            (((src) & 0x00000200)>>9)
#define MDED41MASK_WR(src)                       (((u32)(src)<<9) & 0x00000200)
#define MDED41MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*    Mask Register Fields MDED40Mask    */
#define MDED40MASK_WIDTH                                                      1
#define MDED40MASK_SHIFT                                                      8
#define MDED40MASK_MASK                                              0x00000100
#define MDED40MASK_RD(src)                            (((src) & 0x00000100)>>8)
#define MDED40MASK_WR(src)                       (((u32)(src)<<8) & 0x00000100)
#define MDED40MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*    Mask Register Fields MDED39Mask    */
#define MDED39MASK_WIDTH                                                      1
#define MDED39MASK_SHIFT                                                      7
#define MDED39MASK_MASK                                              0x00000080
#define MDED39MASK_RD(src)                            (((src) & 0x00000080)>>7)
#define MDED39MASK_WR(src)                       (((u32)(src)<<7) & 0x00000080)
#define MDED39MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*    Mask Register Fields MDED38Mask    */
#define MDED38MASK_WIDTH                                                      1
#define MDED38MASK_SHIFT                                                      6
#define MDED38MASK_MASK                                              0x00000040
#define MDED38MASK_RD(src)                            (((src) & 0x00000040)>>6)
#define MDED38MASK_WR(src)                       (((u32)(src)<<6) & 0x00000040)
#define MDED38MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*    Mask Register Fields MDED37Mask    */
#define MDED37MASK_WIDTH                                                      1
#define MDED37MASK_SHIFT                                                      5
#define MDED37MASK_MASK                                              0x00000020
#define MDED37MASK_RD(src)                            (((src) & 0x00000020)>>5)
#define MDED37MASK_WR(src)                       (((u32)(src)<<5) & 0x00000020)
#define MDED37MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*    Mask Register Fields MDED36Mask    */
#define MDED36MASK_WIDTH                                                      1
#define MDED36MASK_SHIFT                                                      4
#define MDED36MASK_MASK                                              0x00000010
#define MDED36MASK_RD(src)                            (((src) & 0x00000010)>>4)
#define MDED36MASK_WR(src)                       (((u32)(src)<<4) & 0x00000010)
#define MDED36MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*    Mask Register Fields MDED35Mask    */
#define MDED35MASK_WIDTH                                                      1
#define MDED35MASK_SHIFT                                                      3
#define MDED35MASK_MASK                                              0x00000008
#define MDED35MASK_RD(src)                            (((src) & 0x00000008)>>3)
#define MDED35MASK_WR(src)                       (((u32)(src)<<3) & 0x00000008)
#define MDED35MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*    Mask Register Fields MDED34Mask    */
#define MDED34MASK_WIDTH                                                      1
#define MDED34MASK_SHIFT                                                      2
#define MDED34MASK_MASK                                              0x00000004
#define MDED34MASK_RD(src)                            (((src) & 0x00000004)>>2)
#define MDED34MASK_WR(src)                       (((u32)(src)<<2) & 0x00000004)
#define MDED34MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*    Mask Register Fields MDED33Mask    */
#define MDED33MASK_WIDTH                                                      1
#define MDED33MASK_SHIFT                                                      1
#define MDED33MASK_MASK                                              0x00000002
#define MDED33MASK_RD(src)                            (((src) & 0x00000002)>>1)
#define MDED33MASK_WR(src)                       (((u32)(src)<<1) & 0x00000002)
#define MDED33MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*    Mask Register Fields MDED32Mask    */
#define MDED32MASK_WIDTH                                                      1
#define MDED32MASK_SHIFT                                                      0
#define MDED32MASK_MASK                                              0x00000001
#define MDED32MASK_RD(src)                               (((src) & 0x00000001))
#define MDED32MASK_WR(src)                          (((u32)(src)) & 0x00000001)
#define MDED32MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MERR_ADDR	*/ 
/*	 Fields ERRADDRL	 */
#define ERRADDRL_F6_WIDTH                                                    32
#define ERRADDRL_F6_SHIFT                                                     0
#define ERRADDRL_F6_MASK                                             0xffffffff
#define ERRADDRL_F6_RD(src)                              (((src) & 0xffffffff))
#define ERRADDRL_F6_WR(src)                         (((u32)(src)) & 0xffffffff)
#define ERRADDRL_F6_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register GLBL_MERR_REQINFO	*/ 
/*	 Fields ERRADDRH	 */
#define ERRADDRH_F6_WIDTH                                                    10
#define ERRADDRH_F6_SHIFT                                                    22
#define ERRADDRH_F6_MASK                                             0xffc00000
#define ERRADDRH_F6_RD(src)                          (((src) & 0xffc00000)>>22)
#define ERRADDRH_F6_WR(src)                     (((u32)(src)<<22) & 0xffc00000)
#define ERRADDRH_F6_SET(dst,src) \
                      (((dst) & ~0xffc00000) | (((u32)(src)<<22) & 0xffc00000))
/*	 Fields MSTRID	 */
#define MSTRID_WIDTH                                                          6
#define MSTRID_SHIFT                                                         16
#define MSTRID_MASK                                                  0x003f0000
#define MSTRID_RD(src)                               (((src) & 0x003f0000)>>16)
#define MSTRID_WR(src)                          (((u32)(src)<<16) & 0x003f0000)
#define MSTRID_SET(dst,src) \
                      (((dst) & ~0x003f0000) | (((u32)(src)<<16) & 0x003f0000))
/*	 Fields AUXINFO	 */
#define AUXINFO_F6_WIDTH                                                      6
#define AUXINFO_F6_SHIFT                                                     10
#define AUXINFO_F6_MASK                                              0x0000fc00
#define AUXINFO_F6_RD(src)                           (((src) & 0x0000fc00)>>10)
#define AUXINFO_F6_WR(src)                      (((u32)(src)<<10) & 0x0000fc00)
#define AUXINFO_F6_SET(dst,src) \
                      (((dst) & ~0x0000fc00) | (((u32)(src)<<10) & 0x0000fc00))
/*	 Fields REQLEN	 */
#define REQLEN_F4_WIDTH                                                       2
#define REQLEN_F4_SHIFT                                                       4
#define REQLEN_F4_MASK                                               0x00000030
#define REQLEN_F4_RD(src)                             (((src) & 0x00000030)>>4)
#define REQLEN_F4_WR(src)                        (((u32)(src)<<4) & 0x00000030)
#define REQLEN_F4_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*	 Fields REQSIZE	 */
#define REQSIZE_F8_WIDTH                                                      3
#define REQSIZE_F8_SHIFT                                                      1
#define REQSIZE_F8_MASK                                              0x0000000e
#define REQSIZE_F8_RD(src)                            (((src) & 0x0000000e)>>1)
#define REQSIZE_F8_WR(src)                       (((u32)(src)<<1) & 0x0000000e)
#define REQSIZE_F8_SET(dst,src) \
                       (((dst) & ~0x0000000e) | (((u32)(src)<<1) & 0x0000000e))
/*	 Fields REQTYPE	 */
#define REQTYPE_F8_WIDTH                                                      1
#define REQTYPE_F8_SHIFT                                                      0
#define REQTYPE_F8_MASK                                              0x00000001
#define REQTYPE_F8_RD(src)                               (((src) & 0x00000001))
#define REQTYPE_F8_WR(src)                          (((u32)(src)) & 0x00000001)
#define REQTYPE_F8_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_TRANS_ERR	*/ 
/*	 Fields MSWRPOISON	 */
#define MSWRPOISON_WIDTH                                                      1
#define MSWRPOISON_SHIFT                                                     12
#define MSWRPOISON_MASK                                              0x00001000
#define MSWRPOISON_RD(src)                           (((src) & 0x00001000)>>12)
#define MSWRPOISON_WR(src)                      (((u32)(src)<<12) & 0x00001000)
#define MSWRPOISON_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields SWRPOISON	 */
#define SWRPOISON_WIDTH                                                       1
#define SWRPOISON_SHIFT                                                      11
#define SWRPOISON_MASK                                               0x00000800
#define SWRPOISON_RD(src)                            (((src) & 0x00000800)>>11)
#define SWRPOISON_WR(src)                       (((u32)(src)<<11) & 0x00000800)
#define SWRPOISON_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields SWRDYTMO	 */
#define SWRDYTMO_WIDTH                                                        1
#define SWRDYTMO_SHIFT                                                       10
#define SWRDYTMO_MASK                                                0x00000400
#define SWRDYTMO_RD(src)                             (((src) & 0x00000400)>>10)
#define SWRDYTMO_WR(src)                        (((u32)(src)<<10) & 0x00000400)
#define SWRDYTMO_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields SWRESPTMO	 */
#define SWRESPTMO_WIDTH                                                       1
#define SWRESPTMO_SHIFT                                                       9
#define SWRESPTMO_MASK                                               0x00000200
#define SWRESPTMO_RD(src)                             (((src) & 0x00000200)>>9)
#define SWRESPTMO_WR(src)                        (((u32)(src)<<9) & 0x00000200)
#define SWRESPTMO_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields MSWRERR	 */
#define MSWRERR_WIDTH                                                         1
#define MSWRERR_SHIFT                                                         8
#define MSWRERR_MASK                                                 0x00000100
#define MSWRERR_RD(src)                               (((src) & 0x00000100)>>8)
#define MSWRERR_WR(src)                          (((u32)(src)<<8) & 0x00000100)
#define MSWRERR_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields SWRERR	 */
#define SWRERR_WIDTH                                                          1
#define SWRERR_SHIFT                                                          7
#define SWRERR_MASK                                                  0x00000080
#define SWRERR_RD(src)                                (((src) & 0x00000080)>>7)
#define SWRERR_WR(src)                           (((u32)(src)<<7) & 0x00000080)
#define SWRERR_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields SRRDYTMO	 */
#define SRRDYTMO_WIDTH                                                        1
#define SRRDYTMO_SHIFT                                                        3
#define SRRDYTMO_MASK                                                0x00000008
#define SRRDYTMO_RD(src)                              (((src) & 0x00000008)>>3)
#define SRRDYTMO_WR(src)                         (((u32)(src)<<3) & 0x00000008)
#define SRRDYTMO_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields SRRESPTMO	 */
#define SRRESPTMO_WIDTH                                                       1
#define SRRESPTMO_SHIFT                                                       2
#define SRRESPTMO_MASK                                               0x00000004
#define SRRESPTMO_RD(src)                             (((src) & 0x00000004)>>2)
#define SRRESPTMO_WR(src)                        (((u32)(src)<<2) & 0x00000004)
#define SRRESPTMO_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields MSRDERR	 */
#define MSRDERR_WIDTH                                                         1
#define MSRDERR_SHIFT                                                         1
#define MSRDERR_MASK                                                 0x00000002
#define MSRDERR_RD(src)                               (((src) & 0x00000002)>>1)
#define MSRDERR_WR(src)                          (((u32)(src)<<1) & 0x00000002)
#define MSRDERR_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SRDERR	 */
#define SRDERR_WIDTH                                                          1
#define SRDERR_SHIFT                                                          0
#define SRDERR_MASK                                                  0x00000001
#define SRDERR_RD(src)                                   (((src) & 0x00000001))
#define SRDERR_WR(src)                              (((u32)(src)) & 0x00000001)
#define SRDERR_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_TRANS_ERRMask	*/
/*    Mask Register Fields MSWRPOISONMask    */
#define MSWRPOISONMASK_WIDTH                                                  1
#define MSWRPOISONMASK_SHIFT                                                 12
#define MSWRPOISONMASK_MASK                                          0x00001000
#define MSWRPOISONMASK_RD(src)                       (((src) & 0x00001000)>>12)
#define MSWRPOISONMASK_WR(src)                  (((u32)(src)<<12) & 0x00001000)
#define MSWRPOISONMASK_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*    Mask Register Fields SWRPOISONMask    */
#define SWRPOISONMASK_WIDTH                                                   1
#define SWRPOISONMASK_SHIFT                                                  11
#define SWRPOISONMASK_MASK                                           0x00000800
#define SWRPOISONMASK_RD(src)                        (((src) & 0x00000800)>>11)
#define SWRPOISONMASK_WR(src)                   (((u32)(src)<<11) & 0x00000800)
#define SWRPOISONMASK_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*    Mask Register Fields SWRDYTMOMask    */
#define SWRDYTMOMASK_WIDTH                                                    1
#define SWRDYTMOMASK_SHIFT                                                   10
#define SWRDYTMOMASK_MASK                                            0x00000400
#define SWRDYTMOMASK_RD(src)                         (((src) & 0x00000400)>>10)
#define SWRDYTMOMASK_WR(src)                    (((u32)(src)<<10) & 0x00000400)
#define SWRDYTMOMASK_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*    Mask Register Fields SWRESPTMOMask    */
#define SWRESPTMOMASK_WIDTH                                                   1
#define SWRESPTMOMASK_SHIFT                                                   9
#define SWRESPTMOMASK_MASK                                           0x00000200
#define SWRESPTMOMASK_RD(src)                         (((src) & 0x00000200)>>9)
#define SWRESPTMOMASK_WR(src)                    (((u32)(src)<<9) & 0x00000200)
#define SWRESPTMOMASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*    Mask Register Fields MSWRERRMask    */
#define MSWRERRMASK_WIDTH                                                     1
#define MSWRERRMASK_SHIFT                                                     8
#define MSWRERRMASK_MASK                                             0x00000100
#define MSWRERRMASK_RD(src)                           (((src) & 0x00000100)>>8)
#define MSWRERRMASK_WR(src)                      (((u32)(src)<<8) & 0x00000100)
#define MSWRERRMASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*    Mask Register Fields SWRERRMask    */
#define SWRERRMASK_WIDTH                                                      1
#define SWRERRMASK_SHIFT                                                      7
#define SWRERRMASK_MASK                                              0x00000080
#define SWRERRMASK_RD(src)                            (((src) & 0x00000080)>>7)
#define SWRERRMASK_WR(src)                       (((u32)(src)<<7) & 0x00000080)
#define SWRERRMASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*    Mask Register Fields SRRDYTMOMask    */
#define SRRDYTMOMASK_WIDTH                                                    1
#define SRRDYTMOMASK_SHIFT                                                    3
#define SRRDYTMOMASK_MASK                                            0x00000008
#define SRRDYTMOMASK_RD(src)                          (((src) & 0x00000008)>>3)
#define SRRDYTMOMASK_WR(src)                     (((u32)(src)<<3) & 0x00000008)
#define SRRDYTMOMASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*    Mask Register Fields SRRESPTMOMask    */
#define SRRESPTMOMASK_WIDTH                                                   1
#define SRRESPTMOMASK_SHIFT                                                   2
#define SRRESPTMOMASK_MASK                                           0x00000004
#define SRRESPTMOMASK_RD(src)                         (((src) & 0x00000004)>>2)
#define SRRESPTMOMASK_WR(src)                    (((u32)(src)<<2) & 0x00000004)
#define SRRESPTMOMASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*    Mask Register Fields MSRDERRMask    */
#define MSRDERRMASK_WIDTH                                                     1
#define MSRDERRMASK_SHIFT                                                     1
#define MSRDERRMASK_MASK                                             0x00000002
#define MSRDERRMASK_RD(src)                           (((src) & 0x00000002)>>1)
#define MSRDERRMASK_WR(src)                      (((u32)(src)<<1) & 0x00000002)
#define MSRDERRMASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*    Mask Register Fields SRDERRMask    */
#define SRDERRMASK_WIDTH                                                      1
#define SRDERRMASK_SHIFT                                                      0
#define SRDERRMASK_MASK                                              0x00000001
#define SRDERRMASK_RD(src)                               (((src) & 0x00000001))
#define SRDERRMASK_WR(src)                          (((u32)(src)) & 0x00000001)
#define SRDERRMASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_WDERR_ADDR	*/ 
/*	 Fields ERRADDRL	 */
#define ERRADDRL_F7_WIDTH                                                    32
#define ERRADDRL_F7_SHIFT                                                     0
#define ERRADDRL_F7_MASK                                             0xffffffff
#define ERRADDRL_F7_RD(src)                              (((src) & 0xffffffff))
#define ERRADDRL_F7_WR(src)                         (((u32)(src)) & 0xffffffff)
#define ERRADDRL_F7_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register GLBL_WDERR_REQINFO	*/ 
/*	 Fields ERRADDRH	 */
#define ERRADDRH_F7_WIDTH                                                    10
#define ERRADDRH_F7_SHIFT                                                    22
#define ERRADDRH_F7_MASK                                             0xffc00000
#define ERRADDRH_F7_RD(src)                          (((src) & 0xffc00000)>>22)
#define ERRADDRH_F7_WR(src)                     (((u32)(src)<<22) & 0xffc00000)
#define ERRADDRH_F7_SET(dst,src) \
                      (((dst) & ~0xffc00000) | (((u32)(src)<<22) & 0xffc00000))
/*	 Fields MSTRID	 */
#define MSTRID_F1_WIDTH                                                       6
#define MSTRID_F1_SHIFT                                                      16
#define MSTRID_F1_MASK                                               0x003f0000
#define MSTRID_F1_RD(src)                            (((src) & 0x003f0000)>>16)
#define MSTRID_F1_WR(src)                       (((u32)(src)<<16) & 0x003f0000)
#define MSTRID_F1_SET(dst,src) \
                      (((dst) & ~0x003f0000) | (((u32)(src)<<16) & 0x003f0000))
/*	 Fields AUXINFO	 */
#define AUXINFO_F7_WIDTH                                                      6
#define AUXINFO_F7_SHIFT                                                     10
#define AUXINFO_F7_MASK                                              0x0000fc00
#define AUXINFO_F7_RD(src)                           (((src) & 0x0000fc00)>>10)
#define AUXINFO_F7_WR(src)                      (((u32)(src)<<10) & 0x0000fc00)
#define AUXINFO_F7_SET(dst,src) \
                      (((dst) & ~0x0000fc00) | (((u32)(src)<<10) & 0x0000fc00))
/*	 Fields REQLEN	 */
#define REQLEN_F5_WIDTH                                                       2
#define REQLEN_F5_SHIFT                                                       4
#define REQLEN_F5_MASK                                               0x00000030
#define REQLEN_F5_RD(src)                             (((src) & 0x00000030)>>4)
#define REQLEN_F5_WR(src)                        (((u32)(src)<<4) & 0x00000030)
#define REQLEN_F5_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*	 Fields REQSIZE	 */
#define REQSIZE_F9_WIDTH                                                      3
#define REQSIZE_F9_SHIFT                                                      1
#define REQSIZE_F9_MASK                                              0x0000000e
#define REQSIZE_F9_RD(src)                            (((src) & 0x0000000e)>>1)
#define REQSIZE_F9_WR(src)                       (((u32)(src)<<1) & 0x0000000e)
#define REQSIZE_F9_SET(dst,src) \
                       (((dst) & ~0x0000000e) | (((u32)(src)<<1) & 0x0000000e))

/*	Register GLBL_DEVERR_ADDR	*/ 
/*	 Fields ERRADDRL	 */
#define ERRADDRL_F8_WIDTH                                                    32
#define ERRADDRL_F8_SHIFT                                                     0
#define ERRADDRL_F8_MASK                                             0xffffffff
#define ERRADDRL_F8_RD(src)                              (((src) & 0xffffffff))
#define ERRADDRL_F8_WR(src)                         (((u32)(src)) & 0xffffffff)
#define ERRADDRL_F8_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register GLBL_DEVERR_REQINFO	*/ 
/*	 Fields ERRADDRH	 */
#define ERRADDRH_F8_WIDTH                                                    10
#define ERRADDRH_F8_SHIFT                                                    22
#define ERRADDRH_F8_MASK                                             0xffc00000
#define ERRADDRH_F8_RD(src)                          (((src) & 0xffc00000)>>22)
#define ERRADDRH_F8_WR(src)                     (((u32)(src)<<22) & 0xffc00000)
#define ERRADDRH_F8_SET(dst,src) \
                      (((dst) & ~0xffc00000) | (((u32)(src)<<22) & 0xffc00000))
/*	 Fields MSTRID	 */
#define MSTRID_F2_WIDTH                                                       6
#define MSTRID_F2_SHIFT                                                      16
#define MSTRID_F2_MASK                                               0x003f0000
#define MSTRID_F2_RD(src)                            (((src) & 0x003f0000)>>16)
#define MSTRID_F2_WR(src)                       (((u32)(src)<<16) & 0x003f0000)
#define MSTRID_F2_SET(dst,src) \
                      (((dst) & ~0x003f0000) | (((u32)(src)<<16) & 0x003f0000))
/*	 Fields AUXINFO	 */
#define AUXINFO_F8_WIDTH                                                      6
#define AUXINFO_F8_SHIFT                                                     10
#define AUXINFO_F8_MASK                                              0x0000fc00
#define AUXINFO_F8_RD(src)                           (((src) & 0x0000fc00)>>10)
#define AUXINFO_F8_WR(src)                      (((u32)(src)<<10) & 0x0000fc00)
#define AUXINFO_F8_SET(dst,src) \
                      (((dst) & ~0x0000fc00) | (((u32)(src)<<10) & 0x0000fc00))
/*	 Fields REQLEN	 */
#define REQLEN_F6_WIDTH                                                       2
#define REQLEN_F6_SHIFT                                                       4
#define REQLEN_F6_MASK                                               0x00000030
#define REQLEN_F6_RD(src)                             (((src) & 0x00000030)>>4)
#define REQLEN_F6_WR(src)                        (((u32)(src)<<4) & 0x00000030)
#define REQLEN_F6_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*	 Fields REQSIZE	 */
#define REQSIZE_F10_WIDTH                                                     3
#define REQSIZE_F10_SHIFT                                                     1
#define REQSIZE_F10_MASK                                             0x0000000e
#define REQSIZE_F10_RD(src)                           (((src) & 0x0000000e)>>1)
#define REQSIZE_F10_WR(src)                      (((u32)(src)<<1) & 0x0000000e)
#define REQSIZE_F10_SET(dst,src) \
                       (((dst) & ~0x0000000e) | (((u32)(src)<<1) & 0x0000000e))
/*	 Fields REQTYPE	 */
#define REQTYPE_F9_WIDTH                                                      1
#define REQTYPE_F9_SHIFT                                                      0
#define REQTYPE_F9_MASK                                              0x00000001
#define REQTYPE_F9_RD(src)                               (((src) & 0x00000001))
#define REQTYPE_F9_WR(src)                          (((u32)(src)) & 0x00000001)
#define REQTYPE_F9_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_SEC_ERRL_ALS	*/ 
/*	 Fields SEC	 */
#define SEC_WIDTH                                                            32
#define SEC_SHIFT                                                             0
#define SEC_MASK                                                     0xffffffff
#define SEC_RD(src)                                      (((src) & 0xffffffff))
#define SEC_WR(src)                                 (((u32)(src)) & 0xffffffff)
#define SEC_SET(dst,src) (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register GLBL_SEC_ERRH_ALS	*/ 
/*	 Fields SEC	 */
#define SEC_F1_WIDTH                                                         32
#define SEC_F1_SHIFT                                                          0
#define SEC_F1_MASK                                                  0xffffffff
#define SEC_F1_RD(src)                                   (((src) & 0xffffffff))
#define SEC_F1_WR(src)                              (((u32)(src)) & 0xffffffff)
#define SEC_F1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register GLBL_DED_ERRL_ALS	*/ 
/*	 Fields DED	 */
#define DED_WIDTH                                                            32
#define DED_SHIFT                                                             0
#define DED_MASK                                                     0xffffffff
#define DED_RD(src)                                      (((src) & 0xffffffff))
#define DED_WR(src)                                 (((u32)(src)) & 0xffffffff)
#define DED_SET(dst,src) (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register GLBL_DED_ERRH_ALS	*/ 
/*	 Fields DED	 */
#define DED_F1_WIDTH                                                         32
#define DED_F1_SHIFT                                                          0
#define DED_F1_MASK                                                  0xffffffff
#define DED_F1_RD(src)                                   (((src) & 0xffffffff))
#define DED_F1_WR(src)                              (((u32)(src)) & 0xffffffff)
#define DED_F1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register GLBL_TRANS_ERR_ALS	*/ 
/*	 Fields SWRPOISON	 */
#define SWRPOISON_F1_WIDTH                                                    1
#define SWRPOISON_F1_SHIFT                                                   11
#define SWRPOISON_F1_MASK                                            0x00000800
#define SWRPOISON_F1_RD(src)                         (((src) & 0x00000800)>>11)
#define SWRPOISON_F1_WR(src)                    (((u32)(src)<<11) & 0x00000800)
#define SWRPOISON_F1_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields SWRDYTMO	 */
#define SWRDYTMO_F1_WIDTH                                                     1
#define SWRDYTMO_F1_SHIFT                                                    10
#define SWRDYTMO_F1_MASK                                             0x00000400
#define SWRDYTMO_F1_RD(src)                          (((src) & 0x00000400)>>10)
#define SWRDYTMO_F1_WR(src)                     (((u32)(src)<<10) & 0x00000400)
#define SWRDYTMO_F1_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields SWRESPTMO	 */
#define SWRESPTMO_F1_WIDTH                                                    1
#define SWRESPTMO_F1_SHIFT                                                    9
#define SWRESPTMO_F1_MASK                                            0x00000200
#define SWRESPTMO_F1_RD(src)                          (((src) & 0x00000200)>>9)
#define SWRESPTMO_F1_WR(src)                     (((u32)(src)<<9) & 0x00000200)
#define SWRESPTMO_F1_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields SWRERR	 */
#define SWRERR_F1_WIDTH                                                       1
#define SWRERR_F1_SHIFT                                                       7
#define SWRERR_F1_MASK                                               0x00000080
#define SWRERR_F1_RD(src)                             (((src) & 0x00000080)>>7)
#define SWRERR_F1_WR(src)                        (((u32)(src)<<7) & 0x00000080)
#define SWRERR_F1_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields SRRDYTMO	 */
#define SRRDYTMO_F1_WIDTH                                                     1
#define SRRDYTMO_F1_SHIFT                                                     3
#define SRRDYTMO_F1_MASK                                             0x00000008
#define SRRDYTMO_F1_RD(src)                           (((src) & 0x00000008)>>3)
#define SRRDYTMO_F1_WR(src)                      (((u32)(src)<<3) & 0x00000008)
#define SRRDYTMO_F1_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields SRRESPTMO	 */
#define SRRESPTMO_F1_WIDTH                                                    1
#define SRRESPTMO_F1_SHIFT                                                    2
#define SRRESPTMO_F1_MASK                                            0x00000004
#define SRRESPTMO_F1_RD(src)                          (((src) & 0x00000004)>>2)
#define SRRESPTMO_F1_WR(src)                     (((u32)(src)<<2) & 0x00000004)
#define SRRESPTMO_F1_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields SRDERR	 */
#define SRDERR_F1_WIDTH                                                       1
#define SRDERR_F1_SHIFT                                                       0
#define SRDERR_F1_MASK                                               0x00000001
#define SRDERR_F1_RD(src)                                (((src) & 0x00000001))
#define SRDERR_F1_WR(src)                           (((u32)(src)) & 0x00000001)
#define SRDERR_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Global Base Address	*/
#define IOB_PMU_REGS_BASE_ADDR			0x07e940000ULL

/*    Address IOB_PMU_REGS  Registers */
#define PMEVCNTR0_ADDR                                               0x00000000
#define PMEVCNTR0_DEFAULT                                            0x00000000
#define PMEVCNTR1_ADDR                                               0x00000004
#define PMEVCNTR1_DEFAULT                                            0x00000000
#define PMEVCNTR2_ADDR                                               0x00000008
#define PMEVCNTR2_DEFAULT                                            0x00000000
#define PMEVCNTR3_ADDR                                               0x0000000c
#define PMEVCNTR3_DEFAULT                                            0x00000000
#define PMEVTYPER0_ADDR                                              0x00000400
#define PMEVTYPER0_DEFAULT                                           0x00000000
#define PMEVTYPER1_ADDR                                              0x00000404
#define PMEVTYPER1_DEFAULT                                           0x00000000
#define PMEVTYPER2_ADDR                                              0x00000408
#define PMEVTYPER2_DEFAULT                                           0x00000000
#define PMEVTYPER3_ADDR                                              0x0000040c
#define PMEVTYPER3_DEFAULT                                           0x00000000
#define PMAMR_ADDR                                                   0x00000a00
#define PMAMR_DEFAULT                                                0x00000000
#define PMEXSELR_ADDR                                                0x00000b00
#define PMEXSELR_DEFAULT                                             0x00000000
#define PMCNTENSET_ADDR                                              0x00000c00
#define PMCNTENSET_DEFAULT                                           0x00000000
#define PMCNTENCLR_ADDR                                              0x00000c20
#define PMCNTENCLR_DEFAULT                                           0x00000000
#define PMINTENSET_ADDR                                              0x00000c40
#define PMINTENSET_DEFAULT                                           0x00000000
#define PMINTENCLR_ADDR                                              0x00000c60
#define PMINTENCLR_DEFAULT                                           0x00000000
#define PMOVSR_ADDR                                                  0x00000c80
#define PMOVSR_DEFAULT                                               0x00000000
#define PMLCSR_ADDR                                                  0x00000c84
#define PMLCSR_DEFAULT                                               0x00000003
#define PMCFGR_ADDR                                                  0x00000e00
#define PMCFGR_DEFAULT                                               0x00011f03
#define PMCR_ADDR                                                    0x00000e04
#define PMCR_DEFAULT                                                 0x00000000
#define PMCEID0_ADDR                                                 0x00000e20
#define PMCEID0_DEFAULT                                              0x005b00ff
#define PMCEID1_ADDR                                                 0x00000e24
#define PMCEID1_DEFAULT                                              0x00fc0000
#define DEVTYPE_ADDR                                                 0x00000f8c
#define DEVTYPE_DEFAULT                                              0x00000006
#define PMAUTHSTATUS_ADDR                                            0x00000fb8
#define PMAUTHSTATUS_DEFAULT                                         0x0000000a
#define DEVARCH_ADDR                                                 0x00000fbc
#define DEVARCH_DEFAULT                                              0x1e100044
#define PMPID4_ADDR                                                  0x00000fd0
#define PMPID4_DEFAULT                                               0x00000001
#define PMPID0_ADDR                                                  0x00000fe0
#define PMPID0_DEFAULT                                               0x00000040
#define PMPID1_ADDR                                                  0x00000fe4
#define PMPID1_DEFAULT                                               0x00000000
#define PMPID2_ADDR                                                  0x00000fe8
#define PMPID2_DEFAULT                                               0x0000000f
#define PMPID3_ADDR                                                  0x00000fec
#define PMPID3_DEFAULT                                               0x00000000
#define PMCID0_ADDR                                                  0x00000ff0
#define PMCID0_DEFAULT                                               0x0000000d
#define PMCID1_ADDR                                                  0x00000ff4
#define PMCID1_DEFAULT                                               0x00000090
#define PMCID2_ADDR                                                  0x00000ff8
#define PMCID2_DEFAULT                                               0x00000005
#define PMCID3_ADDR                                                  0x00000ffc
#define PMCID3_DEFAULT                                               0x000000b1

/*	Register PMEVCNTR0	*/ 
/*	 Fields PMN	 */
#define PMN0_WIDTH                                                           32
#define PMN0_SHIFT                                                            0
#define PMN0_MASK                                                    0xffffffff
#define PMN0_RD(src)                                     (((src) & 0xffffffff))
#define PMN0_WR(src)                                (((u32)(src)) & 0xffffffff)
#define PMN0_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PMEVCNTR1	*/ 
/*	 Fields PMN	 */
#define PMN1_WIDTH                                                           32
#define PMN1_SHIFT                                                            0
#define PMN1_MASK                                                    0xffffffff
#define PMN1_RD(src)                                     (((src) & 0xffffffff))
#define PMN1_WR(src)                                (((u32)(src)) & 0xffffffff)
#define PMN1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PMEVCNTR2	*/ 
/*	 Fields PMN	 */
#define PMN2_WIDTH                                                           32
#define PMN2_SHIFT                                                            0
#define PMN2_MASK                                                    0xffffffff
#define PMN2_RD(src)                                     (((src) & 0xffffffff))
#define PMN2_WR(src)                                (((u32)(src)) & 0xffffffff)
#define PMN2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PMEVCNTR3	*/ 
/*	 Fields PMN	 */
#define PMN3_WIDTH                                                           32
#define PMN3_SHIFT                                                            0
#define PMN3_MASK                                                    0xffffffff
#define PMN3_RD(src)                                     (((src) & 0xffffffff))
#define PMN3_WR(src)                                (((u32)(src)) & 0xffffffff)
#define PMN3_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PMEVTYPER0	*/ 
/*	 Fields EVTGROUP	 */
#define EVTGROUP0_WIDTH                                                       2
#define EVTGROUP0_SHIFT                                                       6
#define EVTGROUP0_MASK                                               0x000000c0
#define EVTGROUP0_RD(src)                             (((src) & 0x000000c0)>>6)
#define EVTGROUP0_WR(src)                        (((u32)(src)<<6) & 0x000000c0)
#define EVTGROUP0_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields EVTTYPE	 */
#define EVTTYPE0_WIDTH                                                        2
#define EVTTYPE0_SHIFT                                                        4
#define EVTTYPE0_MASK                                                0x00000030
#define EVTTYPE0_RD(src)                              (((src) & 0x00000030)>>4)
#define EVTTYPE0_WR(src)                         (((u32)(src)<<4) & 0x00000030)
#define EVTTYPE0_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*	 Fields EVTCOUNT	 */
#define EVTCOUNT0_WIDTH                                                       4
#define EVTCOUNT0_SHIFT                                                       0
#define EVTCOUNT0_MASK                                               0x0000000f
#define EVTCOUNT0_RD(src)                                (((src) & 0x0000000f))
#define EVTCOUNT0_WR(src)                           (((u32)(src)) & 0x0000000f)
#define EVTCOUNT0_SET(dst,src) \
                          (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register PMEVTYPER1	*/ 
/*	 Fields EVTGROUP	 */
#define EVTGROUP1_WIDTH                                                       2
#define EVTGROUP1_SHIFT                                                       6
#define EVTGROUP1_MASK                                               0x000000c0
#define EVTGROUP1_RD(src)                             (((src) & 0x000000c0)>>6)
#define EVTGROUP1_WR(src)                        (((u32)(src)<<6) & 0x000000c0)
#define EVTGROUP1_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields EVTTYPE	 */
#define EVTTYPE1_WIDTH                                                        2
#define EVTTYPE1_SHIFT                                                        4
#define EVTTYPE1_MASK                                                0x00000030
#define EVTTYPE1_RD(src)                              (((src) & 0x00000030)>>4)
#define EVTTYPE1_WR(src)                         (((u32)(src)<<4) & 0x00000030)
#define EVTTYPE1_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*	 Fields EVTCOUNT	 */
#define EVTCOUNT1_WIDTH                                                       4
#define EVTCOUNT1_SHIFT                                                       0
#define EVTCOUNT1_MASK                                               0x0000000f
#define EVTCOUNT1_RD(src)                                (((src) & 0x0000000f))
#define EVTCOUNT1_WR(src)                           (((u32)(src)) & 0x0000000f)
#define EVTCOUNT1_SET(dst,src) \
                          (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register PMEVTYPER2	*/ 
/*	 Fields EVTGROUP	 */
#define EVTGROUP2_WIDTH                                                       2
#define EVTGROUP2_SHIFT                                                       6
#define EVTGROUP2_MASK                                               0x000000c0
#define EVTGROUP2_RD(src)                             (((src) & 0x000000c0)>>6)
#define EVTGROUP2_WR(src)                        (((u32)(src)<<6) & 0x000000c0)
#define EVTGROUP2_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields EVTTYPE	 */
#define EVTTYPE2_WIDTH                                                        2
#define EVTTYPE2_SHIFT                                                        4
#define EVTTYPE2_MASK                                                0x00000030
#define EVTTYPE2_RD(src)                              (((src) & 0x00000030)>>4)
#define EVTTYPE2_WR(src)                         (((u32)(src)<<4) & 0x00000030)
#define EVTTYPE2_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*	 Fields EVTCOUNT	 */
#define EVTCOUNT2_WIDTH                                                       4
#define EVTCOUNT2_SHIFT                                                       0
#define EVTCOUNT2_MASK                                               0x0000000f
#define EVTCOUNT2_RD(src)                                (((src) & 0x0000000f))
#define EVTCOUNT2_WR(src)                           (((u32)(src)) & 0x0000000f)
#define EVTCOUNT2_SET(dst,src) \
                          (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register PMEVTYPER3	*/ 
/*	 Fields EVTGROUP	 */
#define EVTGROUP3_WIDTH                                                       2
#define EVTGROUP3_SHIFT                                                       6
#define EVTGROUP3_MASK                                               0x000000c0
#define EVTGROUP3_RD(src)                             (((src) & 0x000000c0)>>6)
#define EVTGROUP3_WR(src)                        (((u32)(src)<<6) & 0x000000c0)
#define EVTGROUP3_SET(dst,src) \
                       (((dst) & ~0x000000c0) | (((u32)(src)<<6) & 0x000000c0))
/*	 Fields EVTTYPE	 */
#define EVTTYPE3_WIDTH                                                        2
#define EVTTYPE3_SHIFT                                                        4
#define EVTTYPE3_MASK                                                0x00000030
#define EVTTYPE3_RD(src)                              (((src) & 0x00000030)>>4)
#define EVTTYPE3_WR(src)                         (((u32)(src)<<4) & 0x00000030)
#define EVTTYPE3_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*	 Fields EVTCOUNT	 */
#define EVTCOUNT3_WIDTH                                                       4
#define EVTCOUNT3_SHIFT                                                       0
#define EVTCOUNT3_MASK                                               0x0000000f
#define EVTCOUNT3_RD(src)                                (((src) & 0x0000000f))
#define EVTCOUNT3_WR(src)                           (((u32)(src)) & 0x0000000f)
#define EVTCOUNT3_SET(dst,src) \
                          (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register PMAMR	*/ 
/*	 Fields L3C	 */
#define L3C_WIDTH                                                             1
#define L3C_SHIFT                                                             9
#define L3C_MASK                                                     0x00000200
#define L3C_RD(src)                                   (((src) & 0x00000200)>>9)
#define L3C_WR(src)                              (((u32)(src)<<9) & 0x00000200)
#define L3C_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields IOBPA	 */
#define IOBPA_WIDTH                                                           1
#define IOBPA_SHIFT                                                           8
#define IOBPA_MASK                                                   0x00000100
#define IOBPA_RD(src)                                 (((src) & 0x00000100)>>8)
#define IOBPA_WR(src)                            (((u32)(src)<<8) & 0x00000100)
#define IOBPA_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields CPU31	 */
#define CPU31_WIDTH                                                           1
#define CPU31_SHIFT                                                           7
#define CPU31_MASK                                                   0x00000080
#define CPU31_RD(src)                                 (((src) & 0x00000080)>>7)
#define CPU31_WR(src)                            (((u32)(src)<<7) & 0x00000080)
#define CPU31_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields CPU30	 */
#define CPU30_WIDTH                                                           1
#define CPU30_SHIFT                                                           6
#define CPU30_MASK                                                   0x00000040
#define CPU30_RD(src)                                 (((src) & 0x00000040)>>6)
#define CPU30_WR(src)                            (((u32)(src)<<6) & 0x00000040)
#define CPU30_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields CPU21	 */
#define CPU21_WIDTH                                                           1
#define CPU21_SHIFT                                                           5
#define CPU21_MASK                                                   0x00000020
#define CPU21_RD(src)                                 (((src) & 0x00000020)>>5)
#define CPU21_WR(src)                            (((u32)(src)<<5) & 0x00000020)
#define CPU21_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields CPU20	 */
#define CPU20_WIDTH                                                           1
#define CPU20_SHIFT                                                           4
#define CPU20_MASK                                                   0x00000010
#define CPU20_RD(src)                                 (((src) & 0x00000010)>>4)
#define CPU20_WR(src)                            (((u32)(src)<<4) & 0x00000010)
#define CPU20_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields CPU11	 */
#define CPU11_WIDTH                                                           1
#define CPU11_SHIFT                                                           3
#define CPU11_MASK                                                   0x00000008
#define CPU11_RD(src)                                 (((src) & 0x00000008)>>3)
#define CPU11_WR(src)                            (((u32)(src)<<3) & 0x00000008)
#define CPU11_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields CPU10	 */
#define CPU10_WIDTH                                                           1
#define CPU10_SHIFT                                                           2
#define CPU10_MASK                                                   0x00000004
#define CPU10_RD(src)                                 (((src) & 0x00000004)>>2)
#define CPU10_WR(src)                            (((u32)(src)<<2) & 0x00000004)
#define CPU10_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields CPU01	 */
#define CPU01_WIDTH                                                           1
#define CPU01_SHIFT                                                           1
#define CPU01_MASK                                                   0x00000002
#define CPU01_RD(src)                                 (((src) & 0x00000002)>>1)
#define CPU01_WR(src)                            (((u32)(src)<<1) & 0x00000002)
#define CPU01_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields CPU00	 */
#define CPU00_WIDTH                                                           1
#define CPU00_SHIFT                                                           0
#define CPU00_MASK                                                   0x00000001
#define CPU00_RD(src)                                    (((src) & 0x00000001))
#define CPU00_WR(src)                               (((u32)(src)) & 0x00000001)
#define CPU00_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register PMEXSELR	*/ 
/*	 Fields EVNTR	 */
#define EVNTR_WIDTH                                                           2
#define EVNTR_SHIFT                                                           0
#define EVNTR_MASK                                                   0x00000003
#define EVNTR_RD(src)                                    (((src) & 0x00000003))
#define EVNTR_WR(src)                               (((u32)(src)) & 0x00000003)
#define EVNTR_SET(dst,src) \
                          (((dst) & ~0x00000003) | (((u32)(src)) & 0x00000003))

/*	Register PMCNTENSET	*/ 
/*	 Fields P3	 */
#define P3_WIDTH                                                              1
#define P3_SHIFT                                                              3
#define P3_MASK                                                      0x00000008
#define P3_RD(src)                                    (((src) & 0x00000008)>>3)
#define P3_WR(src)                               (((u32)(src)<<3) & 0x00000008)
#define P3_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields P2	 */
#define P2_WIDTH                                                              1
#define P2_SHIFT                                                              2
#define P2_MASK                                                      0x00000004
#define P2_RD(src)                                    (((src) & 0x00000004)>>2)
#define P2_WR(src)                               (((u32)(src)<<2) & 0x00000004)
#define P2_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields P1	 */
#define P1_WIDTH                                                              1
#define P1_SHIFT                                                              1
#define P1_MASK                                                      0x00000002
#define P1_RD(src)                                    (((src) & 0x00000002)>>1)
#define P1_WR(src)                               (((u32)(src)<<1) & 0x00000002)
#define P1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields P0	 */
#define P0_WIDTH                                                              1
#define P0_SHIFT                                                              0
#define P0_MASK                                                      0x00000001
#define P0_RD(src)                                       (((src) & 0x00000001))
#define P0_WR(src)                                  (((u32)(src)) & 0x00000001)
#define P0_SET(dst,src) (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register PMCNTENCLR	*/ 
/*	 Fields P3	 */
#define P3_F1_WIDTH                                                           1
#define P3_F1_SHIFT                                                           3
#define P3_F1_MASK                                                   0x00000008
#define P3_F1_RD(src)                                 (((src) & 0x00000008)>>3)
#define P3_F1_WR(src)                            (((u32)(src)<<3) & 0x00000008)
#define P3_F1_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields P2	 */
#define P2_F1_WIDTH                                                           1
#define P2_F1_SHIFT                                                           2
#define P2_F1_MASK                                                   0x00000004
#define P2_F1_RD(src)                                 (((src) & 0x00000004)>>2)
#define P2_F1_WR(src)                            (((u32)(src)<<2) & 0x00000004)
#define P2_F1_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields P1	 */
#define P1_F1_WIDTH                                                           1
#define P1_F1_SHIFT                                                           1
#define P1_F1_MASK                                                   0x00000002
#define P1_F1_RD(src)                                 (((src) & 0x00000002)>>1)
#define P1_F1_WR(src)                            (((u32)(src)<<1) & 0x00000002)
#define P1_F1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields P0	 */
#define P0_F1_WIDTH                                                           1
#define P0_F1_SHIFT                                                           0
#define P0_F1_MASK                                                   0x00000001
#define P0_F1_RD(src)                                    (((src) & 0x00000001))
#define P0_F1_WR(src)                               (((u32)(src)) & 0x00000001)
#define P0_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register PMINTENSET	*/ 
/*	 Fields P3	 */
#define P3_F2_WIDTH                                                           1
#define P3_F2_SHIFT                                                           3
#define P3_F2_MASK                                                   0x00000008
#define P3_F2_RD(src)                                 (((src) & 0x00000008)>>3)
#define P3_F2_WR(src)                            (((u32)(src)<<3) & 0x00000008)
#define P3_F2_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields P2	 */
#define P2_F2_WIDTH                                                           1
#define P2_F2_SHIFT                                                           2
#define P2_F2_MASK                                                   0x00000004
#define P2_F2_RD(src)                                 (((src) & 0x00000004)>>2)
#define P2_F2_WR(src)                            (((u32)(src)<<2) & 0x00000004)
#define P2_F2_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields P1	 */
#define P1_F2_WIDTH                                                           1
#define P1_F2_SHIFT                                                           1
#define P1_F2_MASK                                                   0x00000002
#define P1_F2_RD(src)                                 (((src) & 0x00000002)>>1)
#define P1_F2_WR(src)                            (((u32)(src)<<1) & 0x00000002)
#define P1_F2_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields P0	 */
#define P0_F2_WIDTH                                                           1
#define P0_F2_SHIFT                                                           0
#define P0_F2_MASK                                                   0x00000001
#define P0_F2_RD(src)                                    (((src) & 0x00000001))
#define P0_F2_WR(src)                               (((u32)(src)) & 0x00000001)
#define P0_F2_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register PMINTENCLR	*/ 
/*	 Fields P3	 */
#define P3_F3_WIDTH                                                           1
#define P3_F3_SHIFT                                                           3
#define P3_F3_MASK                                                   0x00000008
#define P3_F3_RD(src)                                 (((src) & 0x00000008)>>3)
#define P3_F3_WR(src)                            (((u32)(src)<<3) & 0x00000008)
#define P3_F3_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields P2	 */
#define P2_F3_WIDTH                                                           1
#define P2_F3_SHIFT                                                           2
#define P2_F3_MASK                                                   0x00000004
#define P2_F3_RD(src)                                 (((src) & 0x00000004)>>2)
#define P2_F3_WR(src)                            (((u32)(src)<<2) & 0x00000004)
#define P2_F3_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields P1	 */
#define P1_F3_WIDTH                                                           1
#define P1_F3_SHIFT                                                           1
#define P1_F3_MASK                                                   0x00000002
#define P1_F3_RD(src)                                 (((src) & 0x00000002)>>1)
#define P1_F3_WR(src)                            (((u32)(src)<<1) & 0x00000002)
#define P1_F3_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields P0	 */
#define P0_F3_WIDTH                                                           1
#define P0_F3_SHIFT                                                           0
#define P0_F3_MASK                                                   0x00000001
#define P0_F3_RD(src)                                    (((src) & 0x00000001))
#define P0_F3_WR(src)                               (((u32)(src)) & 0x00000001)
#define P0_F3_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register PMOVSR	*/ 
/*	 Fields P3	 */
#define P3_F4_WIDTH                                                           1
#define P3_F4_SHIFT                                                           3
#define P3_F4_MASK                                                   0x00000008
#define P3_F4_RD(src)                                 (((src) & 0x00000008)>>3)
#define P3_F4_WR(src)                            (((u32)(src)<<3) & 0x00000008)
#define P3_F4_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields P2	 */
#define P2_F4_WIDTH                                                           1
#define P2_F4_SHIFT                                                           2
#define P2_F4_MASK                                                   0x00000004
#define P2_F4_RD(src)                                 (((src) & 0x00000004)>>2)
#define P2_F4_WR(src)                            (((u32)(src)<<2) & 0x00000004)
#define P2_F4_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields P1	 */
#define P1_F4_WIDTH                                                           1
#define P1_F4_SHIFT                                                           1
#define P1_F4_MASK                                                   0x00000002
#define P1_F4_RD(src)                                 (((src) & 0x00000002)>>1)
#define P1_F4_WR(src)                            (((u32)(src)<<1) & 0x00000002)
#define P1_F4_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields P0	 */
#define P0_F4_WIDTH                                                           1
#define P0_F4_SHIFT                                                           0
#define P0_F4_MASK                                                   0x00000001
#define P0_F4_RD(src)                                    (((src) & 0x00000001))
#define P0_F4_WR(src)                               (((u32)(src)) & 0x00000001)
#define P0_F4_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register PMLCSR	*/ 
/*	 Fields CNT3_Done	 */
#define CNT3_DONE_WIDTH                                                       1
#define CNT3_DONE_SHIFT                                                       1
#define CNT3_DONE_MASK                                               0x00000002
#define CNT3_DONE_RD(src)                             (((src) & 0x00000002)>>1)
#define CNT3_DONE_WR(src)                        (((u32)(src)<<1) & 0x00000002)
#define CNT3_DONE_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields CNT2_Done	 */
#define CNT2_DONE_WIDTH                                                       1
#define CNT2_DONE_SHIFT                                                       0
#define CNT2_DONE_MASK                                               0x00000001
#define CNT2_DONE_RD(src)                                (((src) & 0x00000001))
#define CNT2_DONE_WR(src)                           (((u32)(src)) & 0x00000001)
#define CNT2_DONE_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register PMCFGR	*/ 
/*	 Fields UEN	 */
#define UEN_WIDTH                                                             1
#define UEN_SHIFT                                                            19
#define UEN_MASK                                                     0x00080000
#define UEN_RD(src)                                  (((src) & 0x00080000)>>19)
#define UEN_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields WT	 */
#define WT_WIDTH                                                              1
#define WT_SHIFT                                                             18
#define WT_MASK                                                      0x00040000
#define WT_RD(src)                                   (((src) & 0x00040000)>>18)
#define WT_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields NA	 */
#define NA_WIDTH                                                              1
#define NA_SHIFT                                                             17
#define NA_MASK                                                      0x00020000
#define NA_RD(src)                                   (((src) & 0x00020000)>>17)
#define NA_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields EX	 */
#define EX_WIDTH                                                              1
#define EX_SHIFT                                                             16
#define EX_MASK                                                      0x00010000
#define EX_RD(src)                                   (((src) & 0x00010000)>>16)
#define EX_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields CCD	 */
#define CCD_WIDTH                                                             1
#define CCD_SHIFT                                                            15
#define CCD_MASK                                                     0x00008000
#define CCD_RD(src)                                  (((src) & 0x00008000)>>15)
#define CCD_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields CC	 */
#define CC_WIDTH                                                              1
#define CC_SHIFT                                                             14
#define CC_MASK                                                      0x00004000
#define CC_RD(src)                                   (((src) & 0x00004000)>>14)
#define CC_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields SIZE	 */
#define REGSPEC_SIZE_WIDTH                                                    6
#define REGSPEC_SIZE_SHIFT                                                    8
#define REGSPEC_SIZE_MASK                                            0x00003f00
#define REGSPEC_SIZE_RD(src)                          (((src) & 0x00003f00)>>8)
#define REGSPEC_SIZE_SET(dst,src) \
                       (((dst) & ~0x00003f00) | (((u32)(src)<<8) & 0x00003f00))
/*	 Fields N	 */
#define N_WIDTH                                                               5
#define N_SHIFT                                                               0
#define N_MASK                                                       0x0000001f
#define N_RD(src)                                        (((src) & 0x0000001f))
#define N_SET(dst,src) (((dst) & ~0x0000001f) | (((u32)(src)) & 0x0000001f))

/*	Register PMCR	*/ 
/*	 Fields IMP	 */
#define IMP_WIDTH                                                             8
#define IMP_SHIFT                                                            24
#define IMP_MASK                                                     0xff000000
#define IMP_RD(src)                                  (((src) & 0xff000000)>>24)
#define IMP_WR(src)                             (((u32)(src)<<24) & 0xff000000)
#define IMP_SET(dst,src) \
                      (((dst) & ~0xff000000) | (((u32)(src)<<24) & 0xff000000))
/*	 Fields X	 */
#define X_WIDTH                                                               1
#define X_SHIFT                                                               4
#define X_MASK                                                       0x00000010
#define X_RD(src)                                     (((src) & 0x00000010)>>4)
#define X_WR(src)                                (((u32)(src)<<4) & 0x00000010)
#define X_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields P	 */
#define P_WIDTH                                                               1
#define P_SHIFT                                                               1
#define P_MASK                                                       0x00000002
#define P_RD(src)                                     (((src) & 0x00000002)>>1)
#define P_WR(src)                                (((u32)(src)<<1) & 0x00000002)
#define P_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields E	 */
#define E_WIDTH                                                               1
#define E_SHIFT                                                               0
#define E_MASK                                                       0x00000001
#define E_RD(src)                                        (((src) & 0x00000001))
#define E_WR(src)                                   (((u32)(src)) & 0x00000001)
#define E_SET(dst,src) (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register PMCEID0	*/ 
/*	 Fields CSWINDIRTY	 */
#define CSWINDIRTY0_WIDTH                                                     1
#define CSWINDIRTY0_SHIFT                                                    22
#define CSWINDIRTY0_MASK                                             0x00400000
#define CSWINDIRTY0_RD(src)                          (((src) & 0x00400000)>>22)
#define CSWINDIRTY0_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields AXI1WRPARTIAL	 */
#define AXI1WRPARTIAL0_WIDTH                                                  1
#define AXI1WRPARTIAL0_SHIFT                                                 20
#define AXI1WRPARTIAL0_MASK                                          0x00100000
#define AXI1WRPARTIAL0_RD(src)                       (((src) & 0x00100000)>>20)
#define AXI1WRPARTIAL0_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields AXI1WR	 */
#define AXI1WR0_WIDTH                                                         1
#define AXI1WR0_SHIFT                                                        19
#define AXI1WR0_MASK                                                 0x00080000
#define AXI1WR0_RD(src)                              (((src) & 0x00080000)>>19)
#define AXI1WR0_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields AXI0WRPARTIAL	 */
#define AXI0WRPARTIAL0_WIDTH                                                  1
#define AXI0WRPARTIAL0_SHIFT                                                 17
#define AXI0WRPARTIAL0_MASK                                          0x00020000
#define AXI0WRPARTIAL0_RD(src)                       (((src) & 0x00020000)>>17)
#define AXI0WRPARTIAL0_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields AXI0WR	 */
#define AXI0WR0_WIDTH                                                         1
#define AXI0WR0_SHIFT                                                        16
#define AXI0WR0_MASK                                                 0x00010000
#define AXI0WR0_RD(src)                              (((src) & 0x00010000)>>16)
#define AXI0WR0_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields CSWRDPARTIAL	 */
#define CSWRDPARTIAL0_WIDTH                                                   1
#define CSWRDPARTIAL0_SHIFT                                                   7
#define CSWRDPARTIAL0_MASK                                           0x00000080
#define CSWRDPARTIAL0_RD(src)                         (((src) & 0x00000080)>>7)
#define CSWRDPARTIAL0_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields CSWRDBLOCK	 */
#define CSWRDBLOCK0_WIDTH                                                     1
#define CSWRDBLOCK0_SHIFT                                                     6
#define CSWRDBLOCK0_MASK                                             0x00000040
#define CSWRDBLOCK0_RD(src)                           (((src) & 0x00000040)>>6)
#define CSWRDBLOCK0_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields AXI1RDPARTIAL	 */
#define AXI1RDPARTIAL0_WIDTH                                                  1
#define AXI1RDPARTIAL0_SHIFT                                                  5
#define AXI1RDPARTIAL0_MASK                                          0x00000020
#define AXI1RDPARTIAL0_RD(src)                        (((src) & 0x00000020)>>5)
#define AXI1RDPARTIAL0_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields AXI1RD	 */
#define AXI1RD0_WIDTH                                                         1
#define AXI1RD0_SHIFT                                                         4
#define AXI1RD0_MASK                                                 0x00000010
#define AXI1RD0_RD(src)                               (((src) & 0x00000010)>>4)
#define AXI1RD0_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields AXI0RDPARTIAL	 */
#define AXI0RDPARTIAL0_WIDTH                                                  1
#define AXI0RDPARTIAL0_SHIFT                                                  3
#define AXI0RDPARTIAL0_MASK                                          0x00000008
#define AXI0RDPARTIAL0_RD(src)                        (((src) & 0x00000008)>>3)
#define AXI0RDPARTIAL0_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields AXI0RD	 */
#define AXI0RD0_WIDTH                                                         1
#define AXI0RD0_SHIFT                                                         2
#define AXI0RD0_MASK                                                 0x00000004
#define AXI0RD0_RD(src)                               (((src) & 0x00000004)>>2)
#define AXI0RD0_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields INC64	 */
#define INC640_WIDTH                                                          1
#define INC640_SHIFT                                                          1
#define INC640_MASK                                                  0x00000002
#define INC640_RD(src)                                (((src) & 0x00000002)>>1)
#define INC640_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields INC	 */
#define INC0_WIDTH                                                            1
#define INC0_SHIFT                                                            0
#define INC0_MASK                                                    0x00000001
#define INC0_RD(src)                                     (((src) & 0x00000001))
#define INC0_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register PMCEID1	*/ 
/*	 Fields CSWRDPARTLAT	 */
#define CSWRDPARTLAT1_WIDTH                                                   1
#define CSWRDPARTLAT1_SHIFT                                                  23
#define CSWRDPARTLAT1_MASK                                           0x00800000
#define CSWRDPARTLAT1_RD(src)                        (((src) & 0x00800000)>>23)
#define CSWRDPARTLAT1_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields CSWRDBLOCKLAT	 */
#define CSWRDBLOCKLAT1_WIDTH                                                  1
#define CSWRDBLOCKLAT1_SHIFT                                                 22
#define CSWRDBLOCKLAT1_MASK                                          0x00400000
#define CSWRDBLOCKLAT1_RD(src)                       (((src) & 0x00400000)>>22)
#define CSWRDBLOCKLAT1_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields AXI1RDPARTLAT	 */
#define AXI1RDPARTLAT1_WIDTH                                                  1
#define AXI1RDPARTLAT1_SHIFT                                                 21
#define AXI1RDPARTLAT1_MASK                                          0x00200000
#define AXI1RDPARTLAT1_RD(src)                       (((src) & 0x00200000)>>21)
#define AXI1RDPARTLAT1_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields AXI1RDLAT	 */
#define AXI1RDLAT1_WIDTH                                                      1
#define AXI1RDLAT1_SHIFT                                                     20
#define AXI1RDLAT1_MASK                                              0x00100000
#define AXI1RDLAT1_RD(src)                           (((src) & 0x00100000)>>20)
#define AXI1RDLAT1_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields AXI0RDPARTLAT	 */
#define AXI0RDPARTLAT1_WIDTH                                                  1
#define AXI0RDPARTLAT1_SHIFT                                                 19
#define AXI0RDPARTLAT1_MASK                                          0x00080000
#define AXI0RDPARTLAT1_RD(src)                       (((src) & 0x00080000)>>19)
#define AXI0RDPARTLAT1_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields AXI0RDLAT	 */
#define AXI0RDLAT1_WIDTH                                                      1
#define AXI0RDLAT1_SHIFT                                                     18
#define AXI0RDLAT1_MASK                                              0x00040000
#define AXI0RDLAT1_RD(src)                           (((src) & 0x00040000)>>18)
#define AXI0RDLAT1_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))

/*	Register DEVTYPE	*/ 
/*	 Fields T	 */
#define T_WIDTH                                                               4
#define T_SHIFT                                                               4
#define T_MASK                                                       0x000000f0
#define T_RD(src)                                     (((src) & 0x000000f0)>>4)
#define T_SET(dst,src) \
                       (((dst) & ~0x000000f0) | (((u32)(src)<<4) & 0x000000f0))
/*	 Fields C	 */
#define C_WIDTH                                                               4
#define C_SHIFT                                                               0
#define C_MASK                                                       0x0000000f
#define C_RD(src)                                        (((src) & 0x0000000f))
#define C_SET(dst,src) (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register PMAUTHSTATUS	*/ 
/*	 Fields SNI	 */
#define SNI_WIDTH                                                             1
#define SNI_SHIFT                                                             7
#define SNI_MASK                                                     0x00000080
#define SNI_RD(src)                                   (((src) & 0x00000080)>>7)
#define SNI_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields SNE	 */
#define SNE_WIDTH                                                             1
#define SNE_SHIFT                                                             6
#define SNE_MASK                                                     0x00000040
#define SNE_RD(src)                                   (((src) & 0x00000040)>>6)
#define SNE_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields SI	 */
#define SI_WIDTH                                                              1
#define SI_SHIFT                                                              5
#define SI_MASK                                                      0x00000020
#define SI_RD(src)                                    (((src) & 0x00000020)>>5)
#define SI_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields SE	 */
#define SE_WIDTH                                                              1
#define SE_SHIFT                                                              4
#define SE_MASK                                                      0x00000010
#define SE_RD(src)                                    (((src) & 0x00000010)>>4)
#define SE_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields NSNI	 */
#define NSNI_WIDTH                                                            1
#define NSNI_SHIFT                                                            3
#define NSNI_MASK                                                    0x00000008
#define NSNI_RD(src)                                  (((src) & 0x00000008)>>3)
#define NSNI_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields NSNE	 */
#define NSNE_WIDTH                                                            1
#define NSNE_SHIFT                                                            2
#define NSNE_MASK                                                    0x00000004
#define NSNE_RD(src)                                  (((src) & 0x00000004)>>2)
#define NSNE_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields NSI	 */
#define NSI_WIDTH                                                             1
#define NSI_SHIFT                                                             1
#define NSI_MASK                                                     0x00000002
#define NSI_RD(src)                                   (((src) & 0x00000002)>>1)
#define NSI_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields NSE	 */
#define NSE_WIDTH                                                             1
#define NSE_SHIFT                                                             0
#define NSE_MASK                                                     0x00000001
#define NSE_RD(src)                                      (((src) & 0x00000001))
#define NSE_SET(dst,src) (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register DEVARCH	*/ 
/*	 Fields JEP106_cont	 */
#define JEP106_CONT_WIDTH                                                     4
#define JEP106_CONT_SHIFT                                                    28
#define JEP106_CONT_MASK                                             0xf0000000
#define JEP106_CONT_RD(src)                          (((src) & 0xf0000000)>>28)
#define JEP106_CONT_SET(dst,src) \
                      (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields JEP106	 */
#define JEP106_WIDTH                                                          7
#define JEP106_SHIFT                                                         21
#define JEP106_MASK                                                  0x0fe00000
#define JEP106_RD(src)                               (((src) & 0x0fe00000)>>21)
#define JEP106_SET(dst,src) \
                      (((dst) & ~0x0fe00000) | (((u32)(src)<<21) & 0x0fe00000))
/*	 Fields Revision	 */
#define REVISION_WIDTH                                                        4
#define REVISION_SHIFT                                                       16
#define REVISION_MASK                                                0x000f0000
#define REVISION_RD(src)                             (((src) & 0x000f0000)>>16)
#define REVISION_SET(dst,src) \
                      (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))
/*	 Fields ID_Version	 */
#define ID_VERSION_WIDTH                                                      4
#define ID_VERSION_SHIFT                                                     12
#define ID_VERSION_MASK                                              0x0000f000
#define ID_VERSION_RD(src)                           (((src) & 0x0000f000)>>12)
#define ID_VERSION_SET(dst,src) \
                      (((dst) & ~0x0000f000) | (((u32)(src)<<12) & 0x0000f000))
/*	 Fields ID_Part	 */
#define ID_PART_WIDTH                                                        12
#define ID_PART_SHIFT                                                         0
#define ID_PART_MASK                                                 0x00000fff
#define ID_PART_RD(src)                                  (((src) & 0x00000fff))
#define ID_PART_SET(dst,src) \
                          (((dst) & ~0x00000fff) | (((u32)(src)) & 0x00000fff))

/*	Register PMPID4	*/ 
/*	 Fields PID	 */
#define PID4_WIDTH                                                            8
#define PID4_SHIFT                                                            0
#define PID4_MASK                                                    0x000000ff
#define PID4_RD(src)                                     (((src) & 0x000000ff))
#define PID4_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Register PMPID0	*/ 
/*	 Fields PID	 */
#define PID0_WIDTH                                                            8
#define PID0_SHIFT                                                            0
#define PID0_MASK                                                    0x000000ff
#define PID0_RD(src)                                     (((src) & 0x000000ff))
#define PID0_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Register PMPID1	*/ 
/*	 Fields PID	 */
#define PID1_WIDTH                                                            8
#define PID1_SHIFT                                                            0
#define PID1_MASK                                                    0x000000ff
#define PID1_RD(src)                                     (((src) & 0x000000ff))
#define PID1_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Register PMPID2	*/ 
/*	 Fields PID	 */
#define PID2_WIDTH                                                            8
#define PID2_SHIFT                                                            0
#define PID2_MASK                                                    0x000000ff
#define PID2_RD(src)                                     (((src) & 0x000000ff))
#define PID2_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Register PMPID3	*/ 
/*	 Fields PID	 */
#define PID3_WIDTH                                                            8
#define PID3_SHIFT                                                            0
#define PID3_MASK                                                    0x000000ff
#define PID3_RD(src)                                     (((src) & 0x000000ff))
#define PID3_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Register PMCID0	*/ 
/*	 Fields CID	 */
#define CID0_WIDTH                                                            8
#define CID0_SHIFT                                                            0
#define CID0_MASK                                                    0x000000ff
#define CID0_RD(src)                                     (((src) & 0x000000ff))
#define CID0_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Register PMCID1	*/ 
/*	 Fields CID	 */
#define CID1_WIDTH                                                            8
#define CID1_SHIFT                                                            0
#define CID1_MASK                                                    0x000000ff
#define CID1_RD(src)                                     (((src) & 0x000000ff))
#define CID1_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Register PMCID2	*/ 
/*	 Fields CID	 */
#define CID2_WIDTH                                                            8
#define CID2_SHIFT                                                            0
#define CID2_MASK                                                    0x000000ff
#define CID2_RD(src)                                     (((src) & 0x000000ff))
#define CID2_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Register PMCID3	*/ 
/*	 Fields CID	 */
#define CID3_WIDTH                                                            8
#define CID3_SHIFT                                                            0
#define CID3_MASK                                                    0x000000ff
#define CID3_RD(src)                                     (((src) & 0x000000ff))
#define CID3_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Global Base Address	*/
#define IOB_DBG_REGS_BASE_ADDR			0x07e950000ULL

/*    Address IOB_DBG_REGS  Registers */
#define PACDBGFLAG00_ADDR                                            0x00000000
#define PACDBGFLAG00_DEFAULT                                         0x00000000
#define PACDBGFLAG01_ADDR                                            0x00000004
#define PACDBGFLAG01_DEFAULT                                         0x00000000
#define PACDBGFLAG02_ADDR                                            0x00000008
#define PACDBGFLAG02_DEFAULT                                         0x00000000
#define PACDBGFLAG03_ADDR                                            0x0000000c
#define PACDBGFLAG03_DEFAULT                                         0x00000000
#define PACDBGFLAG04_ADDR                                            0x00000010
#define PACDBGFLAG04_DEFAULT                                         0x00000000
#define PACDBGFLAG05_ADDR                                            0x00000014
#define PACDBGFLAG05_DEFAULT                                         0x00000000
#define PACDBGFLAG06_ADDR                                            0x00000018
#define PACDBGFLAG06_DEFAULT                                         0x00000000
#define PACDBGFLAG07_ADDR                                            0x0000001c
#define PACDBGFLAG07_DEFAULT                                         0x00000000
#define PACDBGFLAG10_ADDR                                            0x00000020
#define PACDBGFLAG10_DEFAULT                                         0x00000000
#define PACDBGFLAG11_ADDR                                            0x00000024
#define PACDBGFLAG11_DEFAULT                                         0x00000000
#define PACDBGFLAG12_ADDR                                            0x00000028
#define PACDBGFLAG12_DEFAULT                                         0x00000000
#define PACDBGFLAG13_ADDR                                            0x0000002c
#define PACDBGFLAG13_DEFAULT                                         0x00000000
#define PACDBGFLAG14_ADDR                                            0x00000030
#define PACDBGFLAG14_DEFAULT                                         0x00000000
#define PACDBGFLAG15_ADDR                                            0x00000034
#define PACDBGFLAG15_DEFAULT                                         0x00000000
#define PACDBGFLAG16_ADDR                                            0x00000038
#define PACDBGFLAG16_DEFAULT                                         0x00000000
#define PACDBGFLAG17_ADDR                                            0x0000003c
#define PACDBGFLAG17_DEFAULT                                         0x00000000
#define PACDBGFLAG20_ADDR                                            0x00000040
#define PACDBGFLAG20_DEFAULT                                         0x00000000
#define PACDBGFLAG21_ADDR                                            0x00000044
#define PACDBGFLAG21_DEFAULT                                         0x00000000
#define PACDBGFLAG22_ADDR                                            0x00000048
#define PACDBGFLAG22_DEFAULT                                         0x00000000
#define PACDBGFLAG23_ADDR                                            0x0000004c
#define PACDBGFLAG23_DEFAULT                                         0x00000000
#define PACDBGFLAG24_ADDR                                            0x00000050
#define PACDBGFLAG24_DEFAULT                                         0x00000000
#define PACDBGFLAG25_ADDR                                            0x00000054
#define PACDBGFLAG25_DEFAULT                                         0x00000000
#define PACDBGFLAG26_ADDR                                            0x00000058
#define PACDBGFLAG26_DEFAULT                                         0x00000000
#define PACDBGFLAG27_ADDR                                            0x0000005c
#define PACDBGFLAG27_DEFAULT                                         0x00000000
#define PACDBGFLAG30_ADDR                                            0x00000060
#define PACDBGFLAG30_DEFAULT                                         0x00000000
#define PACDBGFLAG31_ADDR                                            0x00000064
#define PACDBGFLAG31_DEFAULT                                         0x00000000
#define PACDBGFLAG32_ADDR                                            0x00000068
#define PACDBGFLAG32_DEFAULT                                         0x00000000
#define PACDBGFLAG33_ADDR                                            0x0000006c
#define PACDBGFLAG33_DEFAULT                                         0x00000000
#define PACDBGFLAG34_ADDR                                            0x00000070
#define PACDBGFLAG34_DEFAULT                                         0x00000000
#define PACDBGFLAG35_ADDR                                            0x00000074
#define PACDBGFLAG35_DEFAULT                                         0x00000000
#define PACDBGFLAG36_ADDR                                            0x00000078
#define PACDBGFLAG36_DEFAULT                                         0x00000000
#define PACDBGFLAG37_ADDR                                            0x0000007c
#define PACDBGFLAG37_DEFAULT                                         0x00000000
#define PACDBGFLAG40_ADDR                                            0x00000080
#define PACDBGFLAG40_DEFAULT                                         0x00000000
#define PACDBGFLAG41_ADDR                                            0x00000084
#define PACDBGFLAG41_DEFAULT                                         0x00000000
#define PACDBGFLAG42_ADDR                                            0x00000088
#define PACDBGFLAG42_DEFAULT                                         0x00000000
#define PACDBGFLAG43_ADDR                                            0x0000008c
#define PACDBGFLAG43_DEFAULT                                         0x00000000
#define PACDBGFLAG44_ADDR                                            0x00000090
#define PACDBGFLAG44_DEFAULT                                         0x00000000
#define PACDBGFLAG45_ADDR                                            0x00000094
#define PACDBGFLAG45_DEFAULT                                         0x00000000
#define PACDBGFLAG46_ADDR                                            0x00000098
#define PACDBGFLAG46_DEFAULT                                         0x00000000
#define PACDBGFLAG47_ADDR                                            0x0000009c
#define PACDBGFLAG47_DEFAULT                                         0x00000000
#define PACDBGFLAG50_ADDR                                            0x000000a0
#define PACDBGFLAG50_DEFAULT                                         0x00000000
#define PACDBGFLAG51_ADDR                                            0x000000a4
#define PACDBGFLAG51_DEFAULT                                         0x00000000
#define PACDBGFLAG52_ADDR                                            0x000000a8
#define PACDBGFLAG52_DEFAULT                                         0x00000000
#define PACDBGFLAG53_ADDR                                            0x000000ac
#define PACDBGFLAG53_DEFAULT                                         0x00000000
#define PACDBGFLAG54_ADDR                                            0x000000b0
#define PACDBGFLAG54_DEFAULT                                         0x00000000
#define PACDBGFLAG55_ADDR                                            0x000000b4
#define PACDBGFLAG55_DEFAULT                                         0x00000000
#define PACDBGFLAG56_ADDR                                            0x000000b8
#define PACDBGFLAG56_DEFAULT                                         0x00000000
#define PACDBGFLAG57_ADDR                                            0x000000bc
#define PACDBGFLAG57_DEFAULT                                         0x00000000
#define PACDBGFLAG60_ADDR                                            0x000000c0
#define PACDBGFLAG60_DEFAULT                                         0x00000000
#define PACDBGFLAG61_ADDR                                            0x000000c4
#define PACDBGFLAG61_DEFAULT                                         0x00000000
#define PACDBGFLAG62_ADDR                                            0x000000c8
#define PACDBGFLAG62_DEFAULT                                         0x00000000
#define PACDBGFLAG63_ADDR                                            0x000000cc
#define PACDBGFLAG63_DEFAULT                                         0x00000000
#define PACDBGFLAG64_ADDR                                            0x000000d0
#define PACDBGFLAG64_DEFAULT                                         0x00000000
#define PACDBGFLAG65_ADDR                                            0x000000d4
#define PACDBGFLAG65_DEFAULT                                         0x00000000
#define PACDBGFLAG66_ADDR                                            0x000000d8
#define PACDBGFLAG66_DEFAULT                                         0x00000000
#define PACDBGFLAG67_ADDR                                            0x000000dc
#define PACDBGFLAG67_DEFAULT                                         0x00000000
#define PACDBGFLAG70_ADDR                                            0x000000e0
#define PACDBGFLAG70_DEFAULT                                         0x00000000
#define PACDBGFLAG71_ADDR                                            0x000000e4
#define PACDBGFLAG71_DEFAULT                                         0x00000000
#define PACDBGFLAG72_ADDR                                            0x000000e8
#define PACDBGFLAG72_DEFAULT                                         0x00000000
#define PACDBGFLAG73_ADDR                                            0x000000ec
#define PACDBGFLAG73_DEFAULT                                         0x00000000
#define PACDBGFLAG74_ADDR                                            0x000000f0
#define PACDBGFLAG74_DEFAULT                                         0x00000000
#define PACDBGFLAG75_ADDR                                            0x000000f4
#define PACDBGFLAG75_DEFAULT                                         0x00000000
#define PACDBGFLAG76_ADDR                                            0x000000f8
#define PACDBGFLAG76_DEFAULT                                         0x00000000
#define PACDBGFLAG77_ADDR                                            0x000000fc
#define PACDBGFLAG77_DEFAULT                                         0x00000000
#define PACDBGFLAG80_ADDR                                            0x00000100
#define PACDBGFLAG80_DEFAULT                                         0x00000000
#define PACDBGFLAG81_ADDR                                            0x00000104
#define PACDBGFLAG81_DEFAULT                                         0x00000000
#define PACDBGFLAG82_ADDR                                            0x00000108
#define PACDBGFLAG82_DEFAULT                                         0x00000000
#define PACDBGFLAG83_ADDR                                            0x0000010c
#define PACDBGFLAG83_DEFAULT                                         0x00000000
#define PACDBGFLAG84_ADDR                                            0x00000110
#define PACDBGFLAG84_DEFAULT                                         0x00000000
#define PACDBGFLAG85_ADDR                                            0x00000114
#define PACDBGFLAG85_DEFAULT                                         0x00000000
#define PACDBGFLAG86_ADDR                                            0x00000118
#define PACDBGFLAG86_DEFAULT                                         0x00000000
#define PACDBGFLAG87_ADDR                                            0x0000011c
#define PACDBGFLAG87_DEFAULT                                         0x00000000
#define PACDBGFLAG90_ADDR                                            0x00000120
#define PACDBGFLAG90_DEFAULT                                         0x00000000
#define PACDBGFLAG91_ADDR                                            0x00000124
#define PACDBGFLAG91_DEFAULT                                         0x00000000
#define PACDBGFLAG92_ADDR                                            0x00000128
#define PACDBGFLAG92_DEFAULT                                         0x00000000
#define PACDBGFLAG93_ADDR                                            0x0000012c
#define PACDBGFLAG93_DEFAULT                                         0x00000000
#define PACDBGFLAG94_ADDR                                            0x00000130
#define PACDBGFLAG94_DEFAULT                                         0x00000000
#define PACDBGFLAG95_ADDR                                            0x00000134
#define PACDBGFLAG95_DEFAULT                                         0x00000000
#define PACDBGFLAG96_ADDR                                            0x00000138
#define PACDBGFLAG96_DEFAULT                                         0x00000000
#define PACDBGFLAG97_ADDR                                            0x0000013c
#define PACDBGFLAG97_DEFAULT                                         0x00000000
#define SRARDBGFLAG00_ADDR                                           0x00000200
#define SRARDBGFLAG00_DEFAULT                                        0x00000000
#define SRARDBGFLAG01_ADDR                                           0x00000204
#define SRARDBGFLAG01_DEFAULT                                        0x00000000
#define SRARDBGFLAG02_ADDR                                           0x00000208
#define SRARDBGFLAG02_DEFAULT                                        0x00000000
#define SRARDBGFLAG03_ADDR                                           0x0000020c
#define SRARDBGFLAG03_DEFAULT                                        0x00000000
#define SRARDBGFLAG04_ADDR                                           0x00000210
#define SRARDBGFLAG04_DEFAULT                                        0x00000000
#define SRARDBGFLAG05_ADDR                                           0x00000214
#define SRARDBGFLAG05_DEFAULT                                        0x00000000
#define SRARDBGFLAG06_ADDR                                           0x00000218
#define SRARDBGFLAG06_DEFAULT                                        0x00000000
#define SRARDBGFLAG07_ADDR                                           0x0000021c
#define SRARDBGFLAG07_DEFAULT                                        0x00000000
#define SRARDBGFLAG10_ADDR                                           0x00000220
#define SRARDBGFLAG10_DEFAULT                                        0x00000000
#define SRARDBGFLAG11_ADDR                                           0x00000224
#define SRARDBGFLAG11_DEFAULT                                        0x00000000
#define SRARDBGFLAG12_ADDR                                           0x00000228
#define SRARDBGFLAG12_DEFAULT                                        0x00000000
#define SRARDBGFLAG13_ADDR                                           0x0000022c
#define SRARDBGFLAG13_DEFAULT                                        0x00000000
#define SRARDBGFLAG14_ADDR                                           0x00000230
#define SRARDBGFLAG14_DEFAULT                                        0x00000000
#define SRARDBGFLAG15_ADDR                                           0x00000234
#define SRARDBGFLAG15_DEFAULT                                        0x00000000
#define SRARDBGFLAG16_ADDR                                           0x00000238
#define SRARDBGFLAG16_DEFAULT                                        0x00000000
#define SRARDBGFLAG17_ADDR                                           0x0000023c
#define SRARDBGFLAG17_DEFAULT                                        0x00000000
#define SRARDBGFLAG20_ADDR                                           0x00000240
#define SRARDBGFLAG20_DEFAULT                                        0x00000000
#define SRARDBGFLAG21_ADDR                                           0x00000244
#define SRARDBGFLAG21_DEFAULT                                        0x00000000
#define SRARDBGFLAG22_ADDR                                           0x00000248
#define SRARDBGFLAG22_DEFAULT                                        0x00000000
#define SRARDBGFLAG23_ADDR                                           0x0000024c
#define SRARDBGFLAG23_DEFAULT                                        0x00000000
#define SRARDBGFLAG24_ADDR                                           0x00000250
#define SRARDBGFLAG24_DEFAULT                                        0x00000000
#define SRARDBGFLAG25_ADDR                                           0x00000254
#define SRARDBGFLAG25_DEFAULT                                        0x00000000
#define SRARDBGFLAG26_ADDR                                           0x00000258
#define SRARDBGFLAG26_DEFAULT                                        0x00000000
#define SRARDBGFLAG27_ADDR                                           0x0000025c
#define SRARDBGFLAG27_DEFAULT                                        0x00000000
#define SRARDBGFLAG30_ADDR                                           0x00000260
#define SRARDBGFLAG30_DEFAULT                                        0x00000000
#define SRARDBGFLAG31_ADDR                                           0x00000264
#define SRARDBGFLAG31_DEFAULT                                        0x00000000
#define SRARDBGFLAG32_ADDR                                           0x00000268
#define SRARDBGFLAG32_DEFAULT                                        0x00000000
#define SRARDBGFLAG33_ADDR                                           0x0000026c
#define SRARDBGFLAG33_DEFAULT                                        0x00000000
#define SRARDBGFLAG34_ADDR                                           0x00000270
#define SRARDBGFLAG34_DEFAULT                                        0x00000000
#define SRARDBGFLAG35_ADDR                                           0x00000274
#define SRARDBGFLAG35_DEFAULT                                        0x00000000
#define SRARDBGFLAG36_ADDR                                           0x00000278
#define SRARDBGFLAG36_DEFAULT                                        0x00000000
#define SRARDBGFLAG37_ADDR                                           0x0000027c
#define SRARDBGFLAG37_DEFAULT                                        0x00000000
#define SRARDBGFLAG40_ADDR                                           0x00000280
#define SRARDBGFLAG40_DEFAULT                                        0x00000000
#define SRARDBGFLAG41_ADDR                                           0x00000284
#define SRARDBGFLAG41_DEFAULT                                        0x00000000
#define SRARDBGFLAG42_ADDR                                           0x00000288
#define SRARDBGFLAG42_DEFAULT                                        0x00000000
#define SRARDBGFLAG43_ADDR                                           0x0000028c
#define SRARDBGFLAG43_DEFAULT                                        0x00000000
#define SRARDBGFLAG44_ADDR                                           0x00000290
#define SRARDBGFLAG44_DEFAULT                                        0x00000000
#define SRARDBGFLAG45_ADDR                                           0x00000294
#define SRARDBGFLAG45_DEFAULT                                        0x00000000
#define SRARDBGFLAG46_ADDR                                           0x00000298
#define SRARDBGFLAG46_DEFAULT                                        0x00000000
#define SRARDBGFLAG47_ADDR                                           0x0000029c
#define SRARDBGFLAG47_DEFAULT                                        0x00000000
#define SRARDBGFLAG50_ADDR                                           0x000002a0
#define SRARDBGFLAG50_DEFAULT                                        0x00000000
#define SRARDBGFLAG51_ADDR                                           0x000002a4
#define SRARDBGFLAG51_DEFAULT                                        0x00000000
#define SRARDBGFLAG52_ADDR                                           0x000002a8
#define SRARDBGFLAG52_DEFAULT                                        0x00000000
#define SRARDBGFLAG53_ADDR                                           0x000002ac
#define SRARDBGFLAG53_DEFAULT                                        0x00000000
#define SRARDBGFLAG54_ADDR                                           0x000002b0
#define SRARDBGFLAG54_DEFAULT                                        0x00000000
#define SRARDBGFLAG55_ADDR                                           0x000002b4
#define SRARDBGFLAG55_DEFAULT                                        0x00000000
#define SRARDBGFLAG56_ADDR                                           0x000002b8
#define SRARDBGFLAG56_DEFAULT                                        0x00000000
#define SRARDBGFLAG57_ADDR                                           0x000002bc
#define SRARDBGFLAG57_DEFAULT                                        0x00000000
#define BACDBGFLAG00_ADDR                                            0x00000300
#define BACDBGFLAG00_DEFAULT                                         0x00000000
#define BACDBGFLAG01_ADDR                                            0x00000304
#define BACDBGFLAG01_DEFAULT                                         0x00000000
#define BACDBGFLAG10_ADDR                                            0x00000308
#define BACDBGFLAG10_DEFAULT                                         0x00000000
#define BACDBGFLAG11_ADDR                                            0x0000030c
#define BACDBGFLAG11_DEFAULT                                         0x00000000
#define BACDBGFLAG20_ADDR                                            0x00000310
#define BACDBGFLAG20_DEFAULT                                         0x00000000
#define BACDBGFLAG21_ADDR                                            0x00000314
#define BACDBGFLAG21_DEFAULT                                         0x00000000
#define BACDBGFLAG30_ADDR                                            0x00000318
#define BACDBGFLAG30_DEFAULT                                         0x00000000
#define BACDBGFLAG31_ADDR                                            0x0000031c
#define BACDBGFLAG31_DEFAULT                                         0x00000000
#define BACDBGFLAG40_ADDR                                            0x00000320
#define BACDBGFLAG40_DEFAULT                                         0x00000000
#define BACDBGFLAG41_ADDR                                            0x00000324
#define BACDBGFLAG41_DEFAULT                                         0x00000000
#define DBGFLAGLOADEN_ADDR                                           0x000003f0
#define DBGFLAGLOADEN_DEFAULT                                        0x00000000
#define DBGIOBCFG_ADDR                                               0x000003f4
#define DBGIOBCFG_DEFAULT                                            0x00000000
#define DBGIOBSTS_ADDR                                               0x000003f8
#define DBGIOBSTS_DEFAULT                                            0x0000000f
#define DBGIOBSS_ADDR                                                0x000003fc
#define DBGIOBSS_DEFAULT                                             0x00000000
#define DBGPCPPLLCCR_ADDR                                            0x00000800
#define DBGPCPPLLCCR_DEFAULT                                         0x00000000

/*	Register PACDBGFLAG00	*/ 
/*	 Fields FLAG	 */
#define FLAG0_WIDTH                                                          32
#define FLAG0_SHIFT                                                           0
#define FLAG0_MASK                                                   0xffffffff
#define FLAG0_RD(src)                                    (((src) & 0xffffffff))
#define FLAG0_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG01	*/ 
/*	 Fields FLAG	 */
#define FLAG1_WIDTH                                                          32
#define FLAG1_SHIFT                                                           0
#define FLAG1_MASK                                                   0xffffffff
#define FLAG1_RD(src)                                    (((src) & 0xffffffff))
#define FLAG1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG02	*/ 
/*	 Fields FLAG	 */
#define FLAG2_WIDTH                                                          32
#define FLAG2_SHIFT                                                           0
#define FLAG2_MASK                                                   0xffffffff
#define FLAG2_RD(src)                                    (((src) & 0xffffffff))
#define FLAG2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG03	*/ 
/*	 Fields FLAG	 */
#define FLAG3_WIDTH                                                          32
#define FLAG3_SHIFT                                                           0
#define FLAG3_MASK                                                   0xffffffff
#define FLAG3_RD(src)                                    (((src) & 0xffffffff))
#define FLAG3_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG04	*/ 
/*	 Fields FLAG	 */
#define FLAG4_WIDTH                                                          32
#define FLAG4_SHIFT                                                           0
#define FLAG4_MASK                                                   0xffffffff
#define FLAG4_RD(src)                                    (((src) & 0xffffffff))
#define FLAG4_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG05	*/ 
/*	 Fields FLAG	 */
#define FLAG5_WIDTH                                                          32
#define FLAG5_SHIFT                                                           0
#define FLAG5_MASK                                                   0xffffffff
#define FLAG5_RD(src)                                    (((src) & 0xffffffff))
#define FLAG5_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG06	*/ 
/*	 Fields FLAG	 */
#define FLAG6_WIDTH                                                          32
#define FLAG6_SHIFT                                                           0
#define FLAG6_MASK                                                   0xffffffff
#define FLAG6_RD(src)                                    (((src) & 0xffffffff))
#define FLAG6_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG07	*/ 
/*	 Fields FLAG	 */
#define FLAG7_WIDTH                                                          32
#define FLAG7_SHIFT                                                           0
#define FLAG7_MASK                                                   0xffffffff
#define FLAG7_RD(src)                                    (((src) & 0xffffffff))
#define FLAG7_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG10	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F1_WIDTH                                                       32
#define FLAG0_F1_SHIFT                                                        0
#define FLAG0_F1_MASK                                                0xffffffff
#define FLAG0_F1_RD(src)                                 (((src) & 0xffffffff))
#define FLAG0_F1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG11	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F1_WIDTH                                                       32
#define FLAG1_F1_SHIFT                                                        0
#define FLAG1_F1_MASK                                                0xffffffff
#define FLAG1_F1_RD(src)                                 (((src) & 0xffffffff))
#define FLAG1_F1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG12	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F1_WIDTH                                                       32
#define FLAG2_F1_SHIFT                                                        0
#define FLAG2_F1_MASK                                                0xffffffff
#define FLAG2_F1_RD(src)                                 (((src) & 0xffffffff))
#define FLAG2_F1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG13	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F1_WIDTH                                                       32
#define FLAG3_F1_SHIFT                                                        0
#define FLAG3_F1_MASK                                                0xffffffff
#define FLAG3_F1_RD(src)                                 (((src) & 0xffffffff))
#define FLAG3_F1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG14	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F1_WIDTH                                                       32
#define FLAG4_F1_SHIFT                                                        0
#define FLAG4_F1_MASK                                                0xffffffff
#define FLAG4_F1_RD(src)                                 (((src) & 0xffffffff))
#define FLAG4_F1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG15	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F1_WIDTH                                                       32
#define FLAG5_F1_SHIFT                                                        0
#define FLAG5_F1_MASK                                                0xffffffff
#define FLAG5_F1_RD(src)                                 (((src) & 0xffffffff))
#define FLAG5_F1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG16	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F1_WIDTH                                                       32
#define FLAG6_F1_SHIFT                                                        0
#define FLAG6_F1_MASK                                                0xffffffff
#define FLAG6_F1_RD(src)                                 (((src) & 0xffffffff))
#define FLAG6_F1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG17	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F1_WIDTH                                                       32
#define FLAG7_F1_SHIFT                                                        0
#define FLAG7_F1_MASK                                                0xffffffff
#define FLAG7_F1_RD(src)                                 (((src) & 0xffffffff))
#define FLAG7_F1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG20	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F2_WIDTH                                                       32
#define FLAG0_F2_SHIFT                                                        0
#define FLAG0_F2_MASK                                                0xffffffff
#define FLAG0_F2_RD(src)                                 (((src) & 0xffffffff))
#define FLAG0_F2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG21	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F2_WIDTH                                                       32
#define FLAG1_F2_SHIFT                                                        0
#define FLAG1_F2_MASK                                                0xffffffff
#define FLAG1_F2_RD(src)                                 (((src) & 0xffffffff))
#define FLAG1_F2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG22	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F2_WIDTH                                                       32
#define FLAG2_F2_SHIFT                                                        0
#define FLAG2_F2_MASK                                                0xffffffff
#define FLAG2_F2_RD(src)                                 (((src) & 0xffffffff))
#define FLAG2_F2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG23	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F2_WIDTH                                                       32
#define FLAG3_F2_SHIFT                                                        0
#define FLAG3_F2_MASK                                                0xffffffff
#define FLAG3_F2_RD(src)                                 (((src) & 0xffffffff))
#define FLAG3_F2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG24	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F2_WIDTH                                                       32
#define FLAG4_F2_SHIFT                                                        0
#define FLAG4_F2_MASK                                                0xffffffff
#define FLAG4_F2_RD(src)                                 (((src) & 0xffffffff))
#define FLAG4_F2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG25	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F2_WIDTH                                                       32
#define FLAG5_F2_SHIFT                                                        0
#define FLAG5_F2_MASK                                                0xffffffff
#define FLAG5_F2_RD(src)                                 (((src) & 0xffffffff))
#define FLAG5_F2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG26	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F2_WIDTH                                                       32
#define FLAG6_F2_SHIFT                                                        0
#define FLAG6_F2_MASK                                                0xffffffff
#define FLAG6_F2_RD(src)                                 (((src) & 0xffffffff))
#define FLAG6_F2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG27	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F2_WIDTH                                                       32
#define FLAG7_F2_SHIFT                                                        0
#define FLAG7_F2_MASK                                                0xffffffff
#define FLAG7_F2_RD(src)                                 (((src) & 0xffffffff))
#define FLAG7_F2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG30	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F3_WIDTH                                                       32
#define FLAG0_F3_SHIFT                                                        0
#define FLAG0_F3_MASK                                                0xffffffff
#define FLAG0_F3_RD(src)                                 (((src) & 0xffffffff))
#define FLAG0_F3_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG31	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F3_WIDTH                                                       32
#define FLAG1_F3_SHIFT                                                        0
#define FLAG1_F3_MASK                                                0xffffffff
#define FLAG1_F3_RD(src)                                 (((src) & 0xffffffff))
#define FLAG1_F3_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG32	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F3_WIDTH                                                       32
#define FLAG2_F3_SHIFT                                                        0
#define FLAG2_F3_MASK                                                0xffffffff
#define FLAG2_F3_RD(src)                                 (((src) & 0xffffffff))
#define FLAG2_F3_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG33	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F3_WIDTH                                                       32
#define FLAG3_F3_SHIFT                                                        0
#define FLAG3_F3_MASK                                                0xffffffff
#define FLAG3_F3_RD(src)                                 (((src) & 0xffffffff))
#define FLAG3_F3_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG34	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F3_WIDTH                                                       32
#define FLAG4_F3_SHIFT                                                        0
#define FLAG4_F3_MASK                                                0xffffffff
#define FLAG4_F3_RD(src)                                 (((src) & 0xffffffff))
#define FLAG4_F3_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG35	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F3_WIDTH                                                       32
#define FLAG5_F3_SHIFT                                                        0
#define FLAG5_F3_MASK                                                0xffffffff
#define FLAG5_F3_RD(src)                                 (((src) & 0xffffffff))
#define FLAG5_F3_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG36	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F3_WIDTH                                                       32
#define FLAG6_F3_SHIFT                                                        0
#define FLAG6_F3_MASK                                                0xffffffff
#define FLAG6_F3_RD(src)                                 (((src) & 0xffffffff))
#define FLAG6_F3_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG37	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F3_WIDTH                                                       32
#define FLAG7_F3_SHIFT                                                        0
#define FLAG7_F3_MASK                                                0xffffffff
#define FLAG7_F3_RD(src)                                 (((src) & 0xffffffff))
#define FLAG7_F3_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG40	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F4_WIDTH                                                       32
#define FLAG0_F4_SHIFT                                                        0
#define FLAG0_F4_MASK                                                0xffffffff
#define FLAG0_F4_RD(src)                                 (((src) & 0xffffffff))
#define FLAG0_F4_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG41	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F4_WIDTH                                                       32
#define FLAG1_F4_SHIFT                                                        0
#define FLAG1_F4_MASK                                                0xffffffff
#define FLAG1_F4_RD(src)                                 (((src) & 0xffffffff))
#define FLAG1_F4_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG42	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F4_WIDTH                                                       32
#define FLAG2_F4_SHIFT                                                        0
#define FLAG2_F4_MASK                                                0xffffffff
#define FLAG2_F4_RD(src)                                 (((src) & 0xffffffff))
#define FLAG2_F4_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG43	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F4_WIDTH                                                       32
#define FLAG3_F4_SHIFT                                                        0
#define FLAG3_F4_MASK                                                0xffffffff
#define FLAG3_F4_RD(src)                                 (((src) & 0xffffffff))
#define FLAG3_F4_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG44	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F4_WIDTH                                                       32
#define FLAG4_F4_SHIFT                                                        0
#define FLAG4_F4_MASK                                                0xffffffff
#define FLAG4_F4_RD(src)                                 (((src) & 0xffffffff))
#define FLAG4_F4_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG45	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F4_WIDTH                                                       32
#define FLAG5_F4_SHIFT                                                        0
#define FLAG5_F4_MASK                                                0xffffffff
#define FLAG5_F4_RD(src)                                 (((src) & 0xffffffff))
#define FLAG5_F4_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG46	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F4_WIDTH                                                       32
#define FLAG6_F4_SHIFT                                                        0
#define FLAG6_F4_MASK                                                0xffffffff
#define FLAG6_F4_RD(src)                                 (((src) & 0xffffffff))
#define FLAG6_F4_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG47	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F4_WIDTH                                                       32
#define FLAG7_F4_SHIFT                                                        0
#define FLAG7_F4_MASK                                                0xffffffff
#define FLAG7_F4_RD(src)                                 (((src) & 0xffffffff))
#define FLAG7_F4_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG50	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F5_WIDTH                                                       32
#define FLAG0_F5_SHIFT                                                        0
#define FLAG0_F5_MASK                                                0xffffffff
#define FLAG0_F5_RD(src)                                 (((src) & 0xffffffff))
#define FLAG0_F5_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG51	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F5_WIDTH                                                       32
#define FLAG1_F5_SHIFT                                                        0
#define FLAG1_F5_MASK                                                0xffffffff
#define FLAG1_F5_RD(src)                                 (((src) & 0xffffffff))
#define FLAG1_F5_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG52	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F5_WIDTH                                                       32
#define FLAG2_F5_SHIFT                                                        0
#define FLAG2_F5_MASK                                                0xffffffff
#define FLAG2_F5_RD(src)                                 (((src) & 0xffffffff))
#define FLAG2_F5_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG53	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F5_WIDTH                                                       32
#define FLAG3_F5_SHIFT                                                        0
#define FLAG3_F5_MASK                                                0xffffffff
#define FLAG3_F5_RD(src)                                 (((src) & 0xffffffff))
#define FLAG3_F5_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG54	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F5_WIDTH                                                       32
#define FLAG4_F5_SHIFT                                                        0
#define FLAG4_F5_MASK                                                0xffffffff
#define FLAG4_F5_RD(src)                                 (((src) & 0xffffffff))
#define FLAG4_F5_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG55	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F5_WIDTH                                                       32
#define FLAG5_F5_SHIFT                                                        0
#define FLAG5_F5_MASK                                                0xffffffff
#define FLAG5_F5_RD(src)                                 (((src) & 0xffffffff))
#define FLAG5_F5_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG56	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F5_WIDTH                                                       32
#define FLAG6_F5_SHIFT                                                        0
#define FLAG6_F5_MASK                                                0xffffffff
#define FLAG6_F5_RD(src)                                 (((src) & 0xffffffff))
#define FLAG6_F5_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG57	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F5_WIDTH                                                       32
#define FLAG7_F5_SHIFT                                                        0
#define FLAG7_F5_MASK                                                0xffffffff
#define FLAG7_F5_RD(src)                                 (((src) & 0xffffffff))
#define FLAG7_F5_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG60	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F6_WIDTH                                                       32
#define FLAG0_F6_SHIFT                                                        0
#define FLAG0_F6_MASK                                                0xffffffff
#define FLAG0_F6_RD(src)                                 (((src) & 0xffffffff))
#define FLAG0_F6_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG61	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F6_WIDTH                                                       32
#define FLAG1_F6_SHIFT                                                        0
#define FLAG1_F6_MASK                                                0xffffffff
#define FLAG1_F6_RD(src)                                 (((src) & 0xffffffff))
#define FLAG1_F6_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG62	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F6_WIDTH                                                       32
#define FLAG2_F6_SHIFT                                                        0
#define FLAG2_F6_MASK                                                0xffffffff
#define FLAG2_F6_RD(src)                                 (((src) & 0xffffffff))
#define FLAG2_F6_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG63	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F6_WIDTH                                                       32
#define FLAG3_F6_SHIFT                                                        0
#define FLAG3_F6_MASK                                                0xffffffff
#define FLAG3_F6_RD(src)                                 (((src) & 0xffffffff))
#define FLAG3_F6_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG64	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F6_WIDTH                                                       32
#define FLAG4_F6_SHIFT                                                        0
#define FLAG4_F6_MASK                                                0xffffffff
#define FLAG4_F6_RD(src)                                 (((src) & 0xffffffff))
#define FLAG4_F6_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG65	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F6_WIDTH                                                       32
#define FLAG5_F6_SHIFT                                                        0
#define FLAG5_F6_MASK                                                0xffffffff
#define FLAG5_F6_RD(src)                                 (((src) & 0xffffffff))
#define FLAG5_F6_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG66	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F6_WIDTH                                                       32
#define FLAG6_F6_SHIFT                                                        0
#define FLAG6_F6_MASK                                                0xffffffff
#define FLAG6_F6_RD(src)                                 (((src) & 0xffffffff))
#define FLAG6_F6_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG67	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F6_WIDTH                                                       32
#define FLAG7_F6_SHIFT                                                        0
#define FLAG7_F6_MASK                                                0xffffffff
#define FLAG7_F6_RD(src)                                 (((src) & 0xffffffff))
#define FLAG7_F6_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG70	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F7_WIDTH                                                       32
#define FLAG0_F7_SHIFT                                                        0
#define FLAG0_F7_MASK                                                0xffffffff
#define FLAG0_F7_RD(src)                                 (((src) & 0xffffffff))
#define FLAG0_F7_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG71	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F7_WIDTH                                                       32
#define FLAG1_F7_SHIFT                                                        0
#define FLAG1_F7_MASK                                                0xffffffff
#define FLAG1_F7_RD(src)                                 (((src) & 0xffffffff))
#define FLAG1_F7_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG72	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F7_WIDTH                                                       32
#define FLAG2_F7_SHIFT                                                        0
#define FLAG2_F7_MASK                                                0xffffffff
#define FLAG2_F7_RD(src)                                 (((src) & 0xffffffff))
#define FLAG2_F7_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG73	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F7_WIDTH                                                       32
#define FLAG3_F7_SHIFT                                                        0
#define FLAG3_F7_MASK                                                0xffffffff
#define FLAG3_F7_RD(src)                                 (((src) & 0xffffffff))
#define FLAG3_F7_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG74	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F7_WIDTH                                                       32
#define FLAG4_F7_SHIFT                                                        0
#define FLAG4_F7_MASK                                                0xffffffff
#define FLAG4_F7_RD(src)                                 (((src) & 0xffffffff))
#define FLAG4_F7_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG75	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F7_WIDTH                                                       32
#define FLAG5_F7_SHIFT                                                        0
#define FLAG5_F7_MASK                                                0xffffffff
#define FLAG5_F7_RD(src)                                 (((src) & 0xffffffff))
#define FLAG5_F7_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG76	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F7_WIDTH                                                       32
#define FLAG6_F7_SHIFT                                                        0
#define FLAG6_F7_MASK                                                0xffffffff
#define FLAG6_F7_RD(src)                                 (((src) & 0xffffffff))
#define FLAG6_F7_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG77	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F7_WIDTH                                                       32
#define FLAG7_F7_SHIFT                                                        0
#define FLAG7_F7_MASK                                                0xffffffff
#define FLAG7_F7_RD(src)                                 (((src) & 0xffffffff))
#define FLAG7_F7_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG80	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F8_WIDTH                                                       32
#define FLAG0_F8_SHIFT                                                        0
#define FLAG0_F8_MASK                                                0xffffffff
#define FLAG0_F8_RD(src)                                 (((src) & 0xffffffff))
#define FLAG0_F8_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG81	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F8_WIDTH                                                       32
#define FLAG1_F8_SHIFT                                                        0
#define FLAG1_F8_MASK                                                0xffffffff
#define FLAG1_F8_RD(src)                                 (((src) & 0xffffffff))
#define FLAG1_F8_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG82	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F8_WIDTH                                                       32
#define FLAG2_F8_SHIFT                                                        0
#define FLAG2_F8_MASK                                                0xffffffff
#define FLAG2_F8_RD(src)                                 (((src) & 0xffffffff))
#define FLAG2_F8_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG83	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F8_WIDTH                                                       32
#define FLAG3_F8_SHIFT                                                        0
#define FLAG3_F8_MASK                                                0xffffffff
#define FLAG3_F8_RD(src)                                 (((src) & 0xffffffff))
#define FLAG3_F8_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG84	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F8_WIDTH                                                       32
#define FLAG4_F8_SHIFT                                                        0
#define FLAG4_F8_MASK                                                0xffffffff
#define FLAG4_F8_RD(src)                                 (((src) & 0xffffffff))
#define FLAG4_F8_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG85	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F8_WIDTH                                                       32
#define FLAG5_F8_SHIFT                                                        0
#define FLAG5_F8_MASK                                                0xffffffff
#define FLAG5_F8_RD(src)                                 (((src) & 0xffffffff))
#define FLAG5_F8_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG86	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F8_WIDTH                                                       32
#define FLAG6_F8_SHIFT                                                        0
#define FLAG6_F8_MASK                                                0xffffffff
#define FLAG6_F8_RD(src)                                 (((src) & 0xffffffff))
#define FLAG6_F8_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG87	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F8_WIDTH                                                       32
#define FLAG7_F8_SHIFT                                                        0
#define FLAG7_F8_MASK                                                0xffffffff
#define FLAG7_F8_RD(src)                                 (((src) & 0xffffffff))
#define FLAG7_F8_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG90	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F9_WIDTH                                                       32
#define FLAG0_F9_SHIFT                                                        0
#define FLAG0_F9_MASK                                                0xffffffff
#define FLAG0_F9_RD(src)                                 (((src) & 0xffffffff))
#define FLAG0_F9_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG91	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F9_WIDTH                                                       32
#define FLAG1_F9_SHIFT                                                        0
#define FLAG1_F9_MASK                                                0xffffffff
#define FLAG1_F9_RD(src)                                 (((src) & 0xffffffff))
#define FLAG1_F9_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG92	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F9_WIDTH                                                       32
#define FLAG2_F9_SHIFT                                                        0
#define FLAG2_F9_MASK                                                0xffffffff
#define FLAG2_F9_RD(src)                                 (((src) & 0xffffffff))
#define FLAG2_F9_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG93	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F9_WIDTH                                                       32
#define FLAG3_F9_SHIFT                                                        0
#define FLAG3_F9_MASK                                                0xffffffff
#define FLAG3_F9_RD(src)                                 (((src) & 0xffffffff))
#define FLAG3_F9_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG94	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F9_WIDTH                                                       32
#define FLAG4_F9_SHIFT                                                        0
#define FLAG4_F9_MASK                                                0xffffffff
#define FLAG4_F9_RD(src)                                 (((src) & 0xffffffff))
#define FLAG4_F9_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG95	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F9_WIDTH                                                       32
#define FLAG5_F9_SHIFT                                                        0
#define FLAG5_F9_MASK                                                0xffffffff
#define FLAG5_F9_RD(src)                                 (((src) & 0xffffffff))
#define FLAG5_F9_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG96	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F9_WIDTH                                                       32
#define FLAG6_F9_SHIFT                                                        0
#define FLAG6_F9_MASK                                                0xffffffff
#define FLAG6_F9_RD(src)                                 (((src) & 0xffffffff))
#define FLAG6_F9_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register PACDBGFLAG97	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F9_WIDTH                                                       32
#define FLAG7_F9_SHIFT                                                        0
#define FLAG7_F9_MASK                                                0xffffffff
#define FLAG7_F9_RD(src)                                 (((src) & 0xffffffff))
#define FLAG7_F9_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG00	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F10_WIDTH                                                      32
#define FLAG0_F10_SHIFT                                                       0
#define FLAG0_F10_MASK                                               0xffffffff
#define FLAG0_F10_RD(src)                                (((src) & 0xffffffff))
#define FLAG0_F10_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG01	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F10_WIDTH                                                      32
#define FLAG1_F10_SHIFT                                                       0
#define FLAG1_F10_MASK                                               0xffffffff
#define FLAG1_F10_RD(src)                                (((src) & 0xffffffff))
#define FLAG1_F10_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG02	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F10_WIDTH                                                      32
#define FLAG2_F10_SHIFT                                                       0
#define FLAG2_F10_MASK                                               0xffffffff
#define FLAG2_F10_RD(src)                                (((src) & 0xffffffff))
#define FLAG2_F10_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG03	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F10_WIDTH                                                      32
#define FLAG3_F10_SHIFT                                                       0
#define FLAG3_F10_MASK                                               0xffffffff
#define FLAG3_F10_RD(src)                                (((src) & 0xffffffff))
#define FLAG3_F10_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG04	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F10_WIDTH                                                      32
#define FLAG4_F10_SHIFT                                                       0
#define FLAG4_F10_MASK                                               0xffffffff
#define FLAG4_F10_RD(src)                                (((src) & 0xffffffff))
#define FLAG4_F10_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG05	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F10_WIDTH                                                      32
#define FLAG5_F10_SHIFT                                                       0
#define FLAG5_F10_MASK                                               0xffffffff
#define FLAG5_F10_RD(src)                                (((src) & 0xffffffff))
#define FLAG5_F10_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG06	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F10_WIDTH                                                      32
#define FLAG6_F10_SHIFT                                                       0
#define FLAG6_F10_MASK                                               0xffffffff
#define FLAG6_F10_RD(src)                                (((src) & 0xffffffff))
#define FLAG6_F10_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG07	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F10_WIDTH                                                      32
#define FLAG7_F10_SHIFT                                                       0
#define FLAG7_F10_MASK                                               0xffffffff
#define FLAG7_F10_RD(src)                                (((src) & 0xffffffff))
#define FLAG7_F10_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG10	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F11_WIDTH                                                      32
#define FLAG0_F11_SHIFT                                                       0
#define FLAG0_F11_MASK                                               0xffffffff
#define FLAG0_F11_RD(src)                                (((src) & 0xffffffff))
#define FLAG0_F11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG11	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F11_WIDTH                                                      32
#define FLAG1_F11_SHIFT                                                       0
#define FLAG1_F11_MASK                                               0xffffffff
#define FLAG1_F11_RD(src)                                (((src) & 0xffffffff))
#define FLAG1_F11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG12	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F11_WIDTH                                                      32
#define FLAG2_F11_SHIFT                                                       0
#define FLAG2_F11_MASK                                               0xffffffff
#define FLAG2_F11_RD(src)                                (((src) & 0xffffffff))
#define FLAG2_F11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG13	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F11_WIDTH                                                      32
#define FLAG3_F11_SHIFT                                                       0
#define FLAG3_F11_MASK                                               0xffffffff
#define FLAG3_F11_RD(src)                                (((src) & 0xffffffff))
#define FLAG3_F11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG14	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F11_WIDTH                                                      32
#define FLAG4_F11_SHIFT                                                       0
#define FLAG4_F11_MASK                                               0xffffffff
#define FLAG4_F11_RD(src)                                (((src) & 0xffffffff))
#define FLAG4_F11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG15	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F11_WIDTH                                                      32
#define FLAG5_F11_SHIFT                                                       0
#define FLAG5_F11_MASK                                               0xffffffff
#define FLAG5_F11_RD(src)                                (((src) & 0xffffffff))
#define FLAG5_F11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG16	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F11_WIDTH                                                      32
#define FLAG6_F11_SHIFT                                                       0
#define FLAG6_F11_MASK                                               0xffffffff
#define FLAG6_F11_RD(src)                                (((src) & 0xffffffff))
#define FLAG6_F11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG17	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F11_WIDTH                                                      32
#define FLAG7_F11_SHIFT                                                       0
#define FLAG7_F11_MASK                                               0xffffffff
#define FLAG7_F11_RD(src)                                (((src) & 0xffffffff))
#define FLAG7_F11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG20	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F12_WIDTH                                                      32
#define FLAG0_F12_SHIFT                                                       0
#define FLAG0_F12_MASK                                               0xffffffff
#define FLAG0_F12_RD(src)                                (((src) & 0xffffffff))
#define FLAG0_F12_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG21	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F12_WIDTH                                                      32
#define FLAG1_F12_SHIFT                                                       0
#define FLAG1_F12_MASK                                               0xffffffff
#define FLAG1_F12_RD(src)                                (((src) & 0xffffffff))
#define FLAG1_F12_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG22	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F12_WIDTH                                                      32
#define FLAG2_F12_SHIFT                                                       0
#define FLAG2_F12_MASK                                               0xffffffff
#define FLAG2_F12_RD(src)                                (((src) & 0xffffffff))
#define FLAG2_F12_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG23	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F12_WIDTH                                                      32
#define FLAG3_F12_SHIFT                                                       0
#define FLAG3_F12_MASK                                               0xffffffff
#define FLAG3_F12_RD(src)                                (((src) & 0xffffffff))
#define FLAG3_F12_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG24	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F12_WIDTH                                                      32
#define FLAG4_F12_SHIFT                                                       0
#define FLAG4_F12_MASK                                               0xffffffff
#define FLAG4_F12_RD(src)                                (((src) & 0xffffffff))
#define FLAG4_F12_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG25	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F12_WIDTH                                                      32
#define FLAG5_F12_SHIFT                                                       0
#define FLAG5_F12_MASK                                               0xffffffff
#define FLAG5_F12_RD(src)                                (((src) & 0xffffffff))
#define FLAG5_F12_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG26	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F12_WIDTH                                                      32
#define FLAG6_F12_SHIFT                                                       0
#define FLAG6_F12_MASK                                               0xffffffff
#define FLAG6_F12_RD(src)                                (((src) & 0xffffffff))
#define FLAG6_F12_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG27	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F12_WIDTH                                                      32
#define FLAG7_F12_SHIFT                                                       0
#define FLAG7_F12_MASK                                               0xffffffff
#define FLAG7_F12_RD(src)                                (((src) & 0xffffffff))
#define FLAG7_F12_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG30	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F13_WIDTH                                                      32
#define FLAG0_F13_SHIFT                                                       0
#define FLAG0_F13_MASK                                               0xffffffff
#define FLAG0_F13_RD(src)                                (((src) & 0xffffffff))
#define FLAG0_F13_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG31	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F13_WIDTH                                                      32
#define FLAG1_F13_SHIFT                                                       0
#define FLAG1_F13_MASK                                               0xffffffff
#define FLAG1_F13_RD(src)                                (((src) & 0xffffffff))
#define FLAG1_F13_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG32	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F13_WIDTH                                                      32
#define FLAG2_F13_SHIFT                                                       0
#define FLAG2_F13_MASK                                               0xffffffff
#define FLAG2_F13_RD(src)                                (((src) & 0xffffffff))
#define FLAG2_F13_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG33	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F13_WIDTH                                                      32
#define FLAG3_F13_SHIFT                                                       0
#define FLAG3_F13_MASK                                               0xffffffff
#define FLAG3_F13_RD(src)                                (((src) & 0xffffffff))
#define FLAG3_F13_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG34	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F13_WIDTH                                                      32
#define FLAG4_F13_SHIFT                                                       0
#define FLAG4_F13_MASK                                               0xffffffff
#define FLAG4_F13_RD(src)                                (((src) & 0xffffffff))
#define FLAG4_F13_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG35	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F13_WIDTH                                                      32
#define FLAG5_F13_SHIFT                                                       0
#define FLAG5_F13_MASK                                               0xffffffff
#define FLAG5_F13_RD(src)                                (((src) & 0xffffffff))
#define FLAG5_F13_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG36	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F13_WIDTH                                                      32
#define FLAG6_F13_SHIFT                                                       0
#define FLAG6_F13_MASK                                               0xffffffff
#define FLAG6_F13_RD(src)                                (((src) & 0xffffffff))
#define FLAG6_F13_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG37	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F13_WIDTH                                                      32
#define FLAG7_F13_SHIFT                                                       0
#define FLAG7_F13_MASK                                               0xffffffff
#define FLAG7_F13_RD(src)                                (((src) & 0xffffffff))
#define FLAG7_F13_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG40	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F14_WIDTH                                                      32
#define FLAG0_F14_SHIFT                                                       0
#define FLAG0_F14_MASK                                               0xffffffff
#define FLAG0_F14_RD(src)                                (((src) & 0xffffffff))
#define FLAG0_F14_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG41	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F14_WIDTH                                                      32
#define FLAG1_F14_SHIFT                                                       0
#define FLAG1_F14_MASK                                               0xffffffff
#define FLAG1_F14_RD(src)                                (((src) & 0xffffffff))
#define FLAG1_F14_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG42	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F14_WIDTH                                                      32
#define FLAG2_F14_SHIFT                                                       0
#define FLAG2_F14_MASK                                               0xffffffff
#define FLAG2_F14_RD(src)                                (((src) & 0xffffffff))
#define FLAG2_F14_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG43	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F14_WIDTH                                                      32
#define FLAG3_F14_SHIFT                                                       0
#define FLAG3_F14_MASK                                               0xffffffff
#define FLAG3_F14_RD(src)                                (((src) & 0xffffffff))
#define FLAG3_F14_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG44	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F14_WIDTH                                                      32
#define FLAG4_F14_SHIFT                                                       0
#define FLAG4_F14_MASK                                               0xffffffff
#define FLAG4_F14_RD(src)                                (((src) & 0xffffffff))
#define FLAG4_F14_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG45	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F14_WIDTH                                                      32
#define FLAG5_F14_SHIFT                                                       0
#define FLAG5_F14_MASK                                               0xffffffff
#define FLAG5_F14_RD(src)                                (((src) & 0xffffffff))
#define FLAG5_F14_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG46	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F14_WIDTH                                                      32
#define FLAG6_F14_SHIFT                                                       0
#define FLAG6_F14_MASK                                               0xffffffff
#define FLAG6_F14_RD(src)                                (((src) & 0xffffffff))
#define FLAG6_F14_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG47	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F14_WIDTH                                                      32
#define FLAG7_F14_SHIFT                                                       0
#define FLAG7_F14_MASK                                               0xffffffff
#define FLAG7_F14_RD(src)                                (((src) & 0xffffffff))
#define FLAG7_F14_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG50	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F15_WIDTH                                                      32
#define FLAG0_F15_SHIFT                                                       0
#define FLAG0_F15_MASK                                               0xffffffff
#define FLAG0_F15_RD(src)                                (((src) & 0xffffffff))
#define FLAG0_F15_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG51	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F15_WIDTH                                                      32
#define FLAG1_F15_SHIFT                                                       0
#define FLAG1_F15_MASK                                               0xffffffff
#define FLAG1_F15_RD(src)                                (((src) & 0xffffffff))
#define FLAG1_F15_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG52	*/ 
/*	 Fields FLAG	 */
#define FLAG2_F15_WIDTH                                                      32
#define FLAG2_F15_SHIFT                                                       0
#define FLAG2_F15_MASK                                               0xffffffff
#define FLAG2_F15_RD(src)                                (((src) & 0xffffffff))
#define FLAG2_F15_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG53	*/ 
/*	 Fields FLAG	 */
#define FLAG3_F15_WIDTH                                                      32
#define FLAG3_F15_SHIFT                                                       0
#define FLAG3_F15_MASK                                               0xffffffff
#define FLAG3_F15_RD(src)                                (((src) & 0xffffffff))
#define FLAG3_F15_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG54	*/ 
/*	 Fields FLAG	 */
#define FLAG4_F15_WIDTH                                                      32
#define FLAG4_F15_SHIFT                                                       0
#define FLAG4_F15_MASK                                               0xffffffff
#define FLAG4_F15_RD(src)                                (((src) & 0xffffffff))
#define FLAG4_F15_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG55	*/ 
/*	 Fields FLAG	 */
#define FLAG5_F15_WIDTH                                                      32
#define FLAG5_F15_SHIFT                                                       0
#define FLAG5_F15_MASK                                               0xffffffff
#define FLAG5_F15_RD(src)                                (((src) & 0xffffffff))
#define FLAG5_F15_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG56	*/ 
/*	 Fields FLAG	 */
#define FLAG6_F15_WIDTH                                                      32
#define FLAG6_F15_SHIFT                                                       0
#define FLAG6_F15_MASK                                               0xffffffff
#define FLAG6_F15_RD(src)                                (((src) & 0xffffffff))
#define FLAG6_F15_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register SRARDBGFLAG57	*/ 
/*	 Fields FLAG	 */
#define FLAG7_F15_WIDTH                                                      32
#define FLAG7_F15_SHIFT                                                       0
#define FLAG7_F15_MASK                                               0xffffffff
#define FLAG7_F15_RD(src)                                (((src) & 0xffffffff))
#define FLAG7_F15_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register BACDBGFLAG00	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F16_WIDTH                                                      32
#define FLAG0_F16_SHIFT                                                       0
#define FLAG0_F16_MASK                                               0xffffffff
#define FLAG0_F16_RD(src)                                (((src) & 0xffffffff))
#define FLAG0_F16_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register BACDBGFLAG01	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F16_WIDTH                                                      32
#define FLAG1_F16_SHIFT                                                       0
#define FLAG1_F16_MASK                                               0xffffffff
#define FLAG1_F16_RD(src)                                (((src) & 0xffffffff))
#define FLAG1_F16_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register BACDBGFLAG10	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F17_WIDTH                                                      32
#define FLAG0_F17_SHIFT                                                       0
#define FLAG0_F17_MASK                                               0xffffffff
#define FLAG0_F17_RD(src)                                (((src) & 0xffffffff))
#define FLAG0_F17_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register BACDBGFLAG11	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F17_WIDTH                                                      32
#define FLAG1_F17_SHIFT                                                       0
#define FLAG1_F17_MASK                                               0xffffffff
#define FLAG1_F17_RD(src)                                (((src) & 0xffffffff))
#define FLAG1_F17_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register BACDBGFLAG20	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F18_WIDTH                                                      32
#define FLAG0_F18_SHIFT                                                       0
#define FLAG0_F18_MASK                                               0xffffffff
#define FLAG0_F18_RD(src)                                (((src) & 0xffffffff))
#define FLAG0_F18_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register BACDBGFLAG21	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F18_WIDTH                                                      32
#define FLAG1_F18_SHIFT                                                       0
#define FLAG1_F18_MASK                                               0xffffffff
#define FLAG1_F18_RD(src)                                (((src) & 0xffffffff))
#define FLAG1_F18_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register BACDBGFLAG30	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F19_WIDTH                                                      32
#define FLAG0_F19_SHIFT                                                       0
#define FLAG0_F19_MASK                                               0xffffffff
#define FLAG0_F19_RD(src)                                (((src) & 0xffffffff))
#define FLAG0_F19_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register BACDBGFLAG31	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F19_WIDTH                                                      32
#define FLAG1_F19_SHIFT                                                       0
#define FLAG1_F19_MASK                                               0xffffffff
#define FLAG1_F19_RD(src)                                (((src) & 0xffffffff))
#define FLAG1_F19_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register BACDBGFLAG40	*/ 
/*	 Fields FLAG	 */
#define FLAG0_F20_WIDTH                                                      32
#define FLAG0_F20_SHIFT                                                       0
#define FLAG0_F20_MASK                                               0xffffffff
#define FLAG0_F20_RD(src)                                (((src) & 0xffffffff))
#define FLAG0_F20_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register BACDBGFLAG41	*/ 
/*	 Fields FLAG	 */
#define FLAG1_F20_WIDTH                                                      32
#define FLAG1_F20_SHIFT                                                       0
#define FLAG1_F20_MASK                                               0xffffffff
#define FLAG1_F20_RD(src)                                (((src) & 0xffffffff))
#define FLAG1_F20_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register DBGFLAGLOADEN	*/ 
/*	 Fields BACDBGFLAGLOADEN	 */
#define BACDBGFLAGLOADEN_WIDTH                                                5
#define BACDBGFLAGLOADEN_SHIFT                                               16
#define BACDBGFLAGLOADEN_MASK                                        0x001f0000
#define BACDBGFLAGLOADEN_RD(src)                     (((src) & 0x001f0000)>>16)
#define BACDBGFLAGLOADEN_WR(src)                (((u32)(src)<<16) & 0x001f0000)
#define BACDBGFLAGLOADEN_SET(dst,src) \
                      (((dst) & ~0x001f0000) | (((u32)(src)<<16) & 0x001f0000))
/*	 Fields SRARDBGFLAGLOADEN	 */
#define SRARDBGFLAGLOADEN_WIDTH                                               6
#define SRARDBGFLAGLOADEN_SHIFT                                              10
#define SRARDBGFLAGLOADEN_MASK                                       0x0000fc00
#define SRARDBGFLAGLOADEN_RD(src)                    (((src) & 0x0000fc00)>>10)
#define SRARDBGFLAGLOADEN_WR(src)               (((u32)(src)<<10) & 0x0000fc00)
#define SRARDBGFLAGLOADEN_SET(dst,src) \
                      (((dst) & ~0x0000fc00) | (((u32)(src)<<10) & 0x0000fc00))
/*	 Fields PACDBGFLAGLOADEN	 */
#define PACDBGFLAGLOADEN_WIDTH                                               10
#define PACDBGFLAGLOADEN_SHIFT                                                0
#define PACDBGFLAGLOADEN_MASK                                        0x000003ff
#define PACDBGFLAGLOADEN_RD(src)                         (((src) & 0x000003ff))
#define PACDBGFLAGLOADEN_WR(src)                    (((u32)(src)) & 0x000003ff)
#define PACDBGFLAGLOADEN_SET(dst,src) \
                          (((dst) & ~0x000003ff) | (((u32)(src)) & 0x000003ff))

/*	Register DBGIOBCFG	*/ 
/*	 Fields BASSENCFG	 */
#define BASSENCFG_WIDTH                                                       1
#define BASSENCFG_SHIFT                                                       6
#define BASSENCFG_MASK                                               0x00000040
#define BASSENCFG_RD(src)                             (((src) & 0x00000040)>>6)
#define BASSENCFG_WR(src)                        (((u32)(src)<<6) & 0x00000040)
#define BASSENCFG_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields PASSENCFG	 */
#define PASSENCFG_WIDTH                                                       1
#define PASSENCFG_SHIFT                                                       5
#define PASSENCFG_MASK                                               0x00000020
#define PASSENCFG_RD(src)                             (((src) & 0x00000020)>>5)
#define PASSENCFG_WR(src)                        (((u32)(src)<<5) & 0x00000020)
#define PASSENCFG_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields DISABLEWRMRG	 */
#define DISABLEWRMRG_WIDTH                                                    1
#define DISABLEWRMRG_SHIFT                                                    4
#define DISABLEWRMRG_MASK                                            0x00000010
#define DISABLEWRMRG_RD(src)                          (((src) & 0x00000010)>>4)
#define DISABLEWRMRG_WR(src)                     (((u32)(src)<<4) & 0x00000010)
#define DISABLEWRMRG_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields SINGLEVCID	 */
#define SINGLEVCID_WIDTH                                                      1
#define SINGLEVCID_SHIFT                                                      3
#define SINGLEVCID_MASK                                              0x00000008
#define SINGLEVCID_RD(src)                            (((src) & 0x00000008)>>3)
#define SINGLEVCID_WR(src)                       (((u32)(src)<<3) & 0x00000008)
#define SINGLEVCID_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields AGENTORDERINGSEL	 */
#define AGENTORDERINGSEL_WIDTH                                                3
#define AGENTORDERINGSEL_SHIFT                                                0
#define AGENTORDERINGSEL_MASK                                        0x00000007
#define AGENTORDERINGSEL_RD(src)                         (((src) & 0x00000007))
#define AGENTORDERINGSEL_WR(src)                    (((u32)(src)) & 0x00000007)
#define AGENTORDERINGSEL_SET(dst,src) \
                          (((dst) & ~0x00000007) | (((u32)(src)) & 0x00000007))

/*	Register DBGIOBSTS	*/ 
/*	 Fields RESPFIFOFULL	 */
#define RESPFIFOFULL_WIDTH                                                    1
#define RESPFIFOFULL_SHIFT                                                    7
#define RESPFIFOFULL_MASK                                            0x00000080
#define RESPFIFOFULL_RD(src)                          (((src) & 0x00000080)>>7)
#define RESPFIFOFULL_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields COS2FIFOFULL	 */
#define COS2FIFOFULL_WIDTH                                                    1
#define COS2FIFOFULL_SHIFT                                                    6
#define COS2FIFOFULL_MASK                                            0x00000040
#define COS2FIFOFULL_RD(src)                          (((src) & 0x00000040)>>6)
#define COS2FIFOFULL_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields COS1FIFOFULL	 */
#define COS1FIFOFULL_WIDTH                                                    1
#define COS1FIFOFULL_SHIFT                                                    5
#define COS1FIFOFULL_MASK                                            0x00000020
#define COS1FIFOFULL_RD(src)                          (((src) & 0x00000020)>>5)
#define COS1FIFOFULL_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields COS0FIFOFULL	 */
#define COS0FIFOFULL_WIDTH                                                    1
#define COS0FIFOFULL_SHIFT                                                    4
#define COS0FIFOFULL_MASK                                            0x00000010
#define COS0FIFOFULL_RD(src)                          (((src) & 0x00000010)>>4)
#define COS0FIFOFULL_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields RESPFIFOEMPTY	 */
#define RESPFIFOEMPTY_WIDTH                                                   1
#define RESPFIFOEMPTY_SHIFT                                                   3
#define RESPFIFOEMPTY_MASK                                           0x00000008
#define RESPFIFOEMPTY_RD(src)                         (((src) & 0x00000008)>>3)
#define RESPFIFOEMPTY_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields COS2FIFOEMPTY	 */
#define COS2FIFOEMPTY_WIDTH                                                   1
#define COS2FIFOEMPTY_SHIFT                                                   2
#define COS2FIFOEMPTY_MASK                                           0x00000004
#define COS2FIFOEMPTY_RD(src)                         (((src) & 0x00000004)>>2)
#define COS2FIFOEMPTY_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields COS1FIFOEMPTY	 */
#define COS1FIFOEMPTY_WIDTH                                                   1
#define COS1FIFOEMPTY_SHIFT                                                   1
#define COS1FIFOEMPTY_MASK                                           0x00000002
#define COS1FIFOEMPTY_RD(src)                         (((src) & 0x00000002)>>1)
#define COS1FIFOEMPTY_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields COS0FIFOEMPTY	 */
#define COS0FIFOEMPTY_WIDTH                                                   1
#define COS0FIFOEMPTY_SHIFT                                                   0
#define COS0FIFOEMPTY_MASK                                           0x00000001
#define COS0FIFOEMPTY_RD(src)                            (((src) & 0x00000001))
#define COS0FIFOEMPTY_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register DBGIOBSS	*/ 
/*	 Fields BASSCFG	 */
#define BASSCFG_WIDTH                                                         1
#define BASSCFG_SHIFT                                                         1
#define BASSCFG_MASK                                                 0x00000002
#define BASSCFG_RD(src)                               (((src) & 0x00000002)>>1)
#define BASSCFG_WR(src)                          (((u32)(src)<<1) & 0x00000002)
#define BASSCFG_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields PASSCFG	 */
#define PASSCFG_WIDTH                                                         1
#define PASSCFG_SHIFT                                                         0
#define PASSCFG_MASK                                                 0x00000001
#define PASSCFG_RD(src)                                  (((src) & 0x00000001))
#define PASSCFG_WR(src)                             (((u32)(src)) & 0x00000001)
#define PASSCFG_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register DBGPCPPLLCCR	*/ 
/*	 Fields ENABLE	 */
#define DBGPCPPLLCCR_ENABLE_WIDTH                                                  1
#define DBGPCPPLLCCR_ENABLE_SHIFT                                                 15
#define DBGPCPPLLCCR_ENABLE_MASK                                          0x00008000
#define DBGPCPPLLCCR_ENABLE_RD(src)                       (((src) & 0x00008000)>>15)
#define DBGPCPPLLCCR_ENABLE_WR(src)                  (((u32)(src)<<15) & 0x00008000)
#define DBGPCPPLLCCR_ENABLE_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields DELAY	 */
#define DELAY_WIDTH                                                          12
#define DELAY_SHIFT                                                           0
#define DELAY_MASK                                                   0x00000fff
#define DELAY_RD(src)                                    (((src) & 0x00000fff))
#define DELAY_WR(src)                               (((u32)(src)) & 0x00000fff)
#define DELAY_SET(dst,src) \
                          (((dst) & ~0x00000fff) | (((u32)(src)) & 0x00000fff))
	
#endif
