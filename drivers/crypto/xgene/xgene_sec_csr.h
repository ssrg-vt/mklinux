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
 * This file defines header for the Xgene Crypto Engine.
 *
 * This is an auto-generated C header file for register definitions
 * PLEASE DON'T MANUALLY MODIFY IN THIS FILE AS CHANGES WILL BE LOST
 */
#ifndef _APM_SEC_CSR_H__
#define _APM_SEC_CSR_H__

/*	Global Base Address	*/
#define SEC_GLBL_CTRL_CSR_BASE_ADDR			0x1f250000

/*    Address SEC_GLBL_CTRL_CSR  Registers */
/*	Configuration register	*/
#define CSR_AXI_RD_WRR_CFG_ADDR                                      0x00000004
#define CSR_AXI_RD_WRR_CFG_DEFAULT                                   0x88800000
#define CSR_AXI_WR_WRR_CFG_ADDR                                      0x00000008
#define CSR_AXI_WR_WRR_CFG_DEFAULT                                   0x88800000
#define CSR_GLB_SEC_INT_TEST_ADDR                                    0x00000014
#define CSR_GLB_SEC_INT_TEST_DEFAULT                                 0x00000000
#define CSR_GLB_SEC_VIRTUAL_CHANNEL_ADDR			     0x00000018
#define CSR_GLB_SEC_VIRTUAL_CHANNEL_DEFAULT			     0x80000000
#define CSR_GLB_SEC_WQ_EIP96_EIP62_ADDR				     0x0000001c
#define CSR_GLB_SEC_WQ_EIP96_EIP62_DEFAULT			     0x00000000

/*	Interrupt and Status register	*/
#define CSR_GLB_SEC_INT_STS_ADDR				     0x0000000c
#define CSR_GLB_SEC_INT_STS_DEFAULT				     0x00000000
#define CSR_GLB_SEC_INT_STSMASK_ADDR				     0x00000010
#define CSR_GLB_SEC_INT_STSMASK_DEFAULT				     0xffffffff

/*	Status read only register	*/
#define CSR_ID_ADDR                                                  0x00000000
#define CSR_ID_DEFAULT                                               0x000005a0

/*	Register csr_id	*/
/*	 Fields rev_no	 */
#define REV_NO_WIDTH                                                          2
#define REV_NO_SHIFT                                                         14
#define REV_NO_MASK                                                  0x0000C000
#define REV_NO_RD(src)			  (((src) & REV_NO_MASK)>>REV_NO_SHIFT)
#define REV_NO_SET(dst,src) \
          (((dst) & ~REV_NO_MASK) | (((u32)(src)<<REV_NO_SHIFT) & REV_NO_MASK))
/*	 Fields bus_id	 */
#define BUS_ID_WIDTH                                                          2
#define BUS_ID_SHIFT                                                         12
#define BUS_ID_MASK                                                  0x00003000
#define BUS_ID_RD(src)                                (((src) & BUS_ID_MASK)>>BUS_ID_SHIFT)
#define BUS_ID_SET(dst,src) \
                       (((dst) & ~BUS_ID_MASK) | (((u32)(src)<<BUS_ID_SHIFT) & BUS_ID_MASK))
/*	 Fields device_id	 */
#define DEVICE_ID_WIDTH                                                      12
#define DEVICE_ID_SHIFT                                                       0
#define DEVICE_ID_MASK                                               0x00000fff
#define DEVICE_ID_RD(src)	(((src) & DEVICE_ID_MASK)>>DEVICE_ID_SHIFT)
#define DEVICE_ID_SET(dst,src) \
             (((dst) & ~DEVICE_ID_MASK) | (((u32)(src)<<DEVICE_ID_SHIFT) & DEVICE_ID_MASK))

/*	Register csr_axi_rd_wrr_cfg	*/
/*	 Fields qmi_rd	 */
#define QMI_RD_WIDTH                                                          4
#define QMI_RD_SHIFT                                                         28
#define QMI_RD_MASK                                                  0xf0000000
#define QMI_RD_RD(src)                               (((src) & 0xf0000000)>>28)
#define QMI_RD_WR(src)                          (((u32)(src)<<28) & 0xf0000000)
#define QMI_RD_SET(dst,src) \
                      (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields eip96_rd	 */
#define EIP96_RD_WIDTH                                                        4
#define EIP96_RD_SHIFT                                                       24
#define EIP96_RD_MASK                                                0x0f000000
#define EIP96_RD_RD(src)                             (((src) & 0x0f000000)>>24)
#define EIP96_RD_WR(src)                        (((u32)(src)<<24) & 0x0f000000)
#define EIP96_RD_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields xts_rd	 */
#define XTS_RD_WIDTH                                                          4
#define XTS_RD_SHIFT                                                         20
#define XTS_RD_MASK                                                  0x00f00000
#define XTS_RD_RD(src)                               (((src) & 0x00f00000)>>20)
#define XTS_RD_WR(src)                          (((u32)(src)<<20) & 0x00f00000)
#define XTS_RD_SET(dst,src) \
                      (((dst) & ~0x00f00000) | (((u32)(src)<<20) & 0x00f00000))
/*       Fields eip62_rd   */
#define EIP62_RD_WIDTH                                                        4
#define EIP62_RD_SHIFT                                                         16
#define EIP62_RD_MASK                                                  0x000f0000
#define EIP62_RD_RD(src)                               (((src) & 0x000f0000)>>16)
#define EIP62_RD_WR(src)                          (((u32)(src)<<16) & 0x000f0000)
#define EIP62_RD_SET(dst,src) \
                      (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))

/*	Register csr_axi_wr_wrr_cfg	*/
/*	 Fields qmi_wr	 */
#define QMI_WR_WIDTH                                                          4
#define QMI_WR_SHIFT                                                         28
#define QMI_WR_MASK                                                  0xf0000000
#define QMI_WR_RD(src)                               (((src) & 0xf0000000)>>28)
#define QMI_WR_WR(src)                          (((u32)(src)<<28) & 0xf0000000)
#define QMI_WR_SET(dst,src) \
                      (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields eip96_wr	 */
#define EIP96_WR_WIDTH                                                        4
#define EIP96_WR_SHIFT                                                       24
#define EIP96_WR_MASK                                                0x0f000000
#define EIP96_WR_RD(src)                             (((src) & 0x0f000000)>>24)
#define EIP96_WR_WR(src)                        (((u32)(src)<<24) & 0x0f000000)
#define EIP96_WR_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields xts_wr	 */
#define XTS_WR_WIDTH                                                          4
#define XTS_WR_SHIFT                                                         20
#define XTS_WR_MASK                                                  0x00f00000
#define XTS_WR_RD(src)                               (((src) & 0x00f00000)>>20)
#define XTS_WR_WR(src)                          (((u32)(src)<<20) & 0x00f00000)
#define XTS_WR_SET(dst,src) \
                      (((dst) & ~0x00f00000) | (((u32)(src)<<20) & 0x00f00000))
/*       Fields eip62_wr   */
#define EIP62_WR_WIDTH                                                        4
#define EIP62_WR_SHIFT                                                         16
#define EIP62_WR_MASK                                                  0x000f0000
#define EIP62_WR_RD(src)                               (((src) & 0x000f0000)>>16)
#define EIP62_WR_WR(src)                          (((u32)(src)<<16) & 0x000f0000)
#define EIP62_WR_SET(dst,src) \
                      (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))

/*	Register csr_glb_sec_int_sts	*/
/*	 Fields qmi	 */
#define QMI_WIDTH                                                             1
#define QMI_SHIFT                                                            31
#define QMI_MASK                                                     0x80000000
#define QMI_RD(src)                                  (((src) & 0x80000000)>>31)
#define QMI_WR(src)                             (((u32)(src)<<31) & 0x80000000)
#define QMI_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields eip96	 */
#define EIP96_WIDTH                                                           1
#define EIP96_SHIFT                                                          30
#define EIP96_MASK                                                   0x40000000
#define EIP96_RD(src)                                (((src) & 0x40000000)>>30)
#define EIP96_WR(src)                           (((u32)(src)<<30) & 0x40000000)
#define EIP96_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields xts	 */
#define XTS_WIDTH                                                             1
#define XTS_SHIFT                                                            29
#define XTS_MASK                                                     0x20000000
#define XTS_RD(src)                                  (((src) & 0x20000000)>>29)
#define XTS_WR(src)                             (((u32)(src)<<29) & 0x20000000)
#define XTS_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields eip62    */
#define EIP62_WIDTH                                                           1
#define EIP62_SHIFT                                                          28
#define EIP62_MASK                                                   0x10000000
#define EIP62_RD(src)                                (((src) & 0x10000000)>>28)
#define EIP62_WR(src)                           (((u32)(src)<<28) & 0x10000000)
#define EIP62_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))

/*	Register csr_glb_sec_int_stsMask	*/
/*    Mask Register Fields qmiMask    */
#define QMIMASK_WIDTH                                                         1
#define QMIMASK_SHIFT                                                        31
#define QMIMASK_MASK                                                 0x80000000
#define QMIMASK_RD(src)                              (((src) & 0x80000000)>>31)
#define QMIMASK_WR(src)                         (((u32)(src)<<31) & 0x80000000)
#define QMIMASK_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*    Mask Register Fields eip96Mask    */
#define EIP96MASK_WIDTH                                                       1
#define EIP96MASK_SHIFT                                                      30
#define EIP96MASK_MASK                                               0x40000000
#define EIP96MASK_RD(src)                            (((src) & 0x40000000)>>30)
#define EIP96MASK_WR(src)                       (((u32)(src)<<30) & 0x40000000)
#define EIP96MASK_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*    Mask Register Fields xtsMask    */
#define XTSMASK_WIDTH                                                         1
#define XTSMASK_SHIFT                                                        29
#define XTSMASK_MASK                                                 0x20000000
#define XTSMASK_RD(src)                              (((src) & 0x20000000)>>29)
#define XTSMASK_WR(src)                         (((u32)(src)<<29) & 0x20000000)
#define XTSMASK_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*    Mask Register Fields eip62Mask    */
#define EIP62MASK_WIDTH                                                       1
#define EIP62MASK_SHIFT                                                      28
#define EIP62MASK_MASK                                               0x10000000
#define EIP62MASK_RD(src)                            (((src) & 0x10000000)>>28)
#define EIP62MASK_WR(src)                       (((u32)(src)<<28) & 0x10000000)
#define EIP62MASK_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))

/*	Register csr_glb_sec_int_test	*/
/*	 Fields force_sec_int	 */
#define FORCE_SEC_INT_WIDTH                                                   1
#define FORCE_SEC_INT_SHIFT                                                  31
#define FORCE_SEC_INT_MASK                                           0x80000000
#define FORCE_SEC_INT_RD(src)                        (((src) & 0x80000000)>>31)
#define FORCE_SEC_INT_WR(src)                   (((u32)(src)<<31) & 0x80000000)
#define FORCE_SEC_INT_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))

/*	Register csr_glb_sec_virtual_channel	*/
/*	 Feilds vc	*/
#define VC_WIDTH								2
#define VC_SHIFT								30
#define VC_MASK									0xc0000000
#define VC_RD(src)			(((src) & VC_MASK)>>VC_SHIFT)
#define VC_WR(src)			(((u32)(src)<<VC_SHIFT) & VC_MASK)
#define VC_SET(dst,src) \
		(((dst) & ~VC_MASK) | (((u32)(src)<<VC_SHIFT) & VC_MASK))

/*      Register csr_glb_sec_wq_eip96_eip62    */
/*       Feilds sel      */
#define SEL_WIDTH                                                                1
#define SEL_SHIFT                                                                31
#define SEL_MASK                                                                 0x80000000
#define SEL_RD(src)                      (((src) & SEL_MASK)>>SEL_SHIFT)
#define SEL_WR(src)                      (((u32)(src)<<SEL_SHIFT) & SEL_MASK)
#define SEL_SET(dst,src) \
                (((dst) & ~SEL_MASK) | (((u32)(src)<<SEL_SHIFT) & SEL_MASK))

/*	Global Base Address	*/
#define SEC_QMI_SLAVE_BASE_ADDR			0x1f259000

/*    Address QMI_SLAVE  Registers */
/*	Configuration Register	*/
#define SEC_CFGSSQMI0_ADDR                                           0x00000000
#define SEC_CFGSSQMI0_DEFAULT                                        0x804c4041
#define SEC_CFGSSQMI1_ADDR                                           0x00000004
#define SEC_CFGSSQMI1_DEFAULT                                        0x2de8b7a2
#define SEC_CFGSSQMIQM0_ADDR                                    0x00000008
#define SEC_CFGSSQMIQM0_DEFAULT                                 0x00000060
#define SEC_CFGSSQMIQM1_ADDR                                    0x0000000c
#define SEC_CFGSSQMIQM1_DEFAULT                                 0x0000006c
#define SEC_CFGSSQMIFPDISABLE_ADDR                                   0x00000010
#define SEC_CFGSSQMIFPDISABLE_DEFAULT                                0x00000000
#define SEC_CFGSSQMIFPRESET_ADDR                                   0x00000014
#define SEC_CFGSSQMIFPRESET_DEFAULT                                0x00000000
#define SEC_CFGSSQMIWQDISABLE_ADDR                                   0x00000018
#define SEC_CFGSSQMIWQDISABLE_DEFAULT                                0x00000000
#define SEC_CFGSSQMIWQRESET_ADDR                                   0x0000001c
#define SEC_CFGSSQMIWQRESET_DEFAULT                                0x00000000
#define SEC_CFGSSQMISABENABLE_ADDR                                   0x00000070
#define SEC_CFGSSQMISABENABLE_DEFAULT                                0x00000000
#define SEC_CFGSSQMISAB0_ADDR                                   0x00000074
#define SEC_CFGSSQMISAB0_DEFAULT                                0x00000000
#define SEC_CFGSSQMISAB1_ADDR                                   0x00000078
#define SEC_CFGSSQMISAB1_DEFAULT                                0x00000000
#define SEC_CFGSSQMISAB2_ADDR                                   0x0000007c
#define SEC_CFGSSQMISAB2_DEFAULT                                0x00000000
#define SEC_CFGSSQMISAB3_ADDR                                   0x00000080
#define SEC_CFGSSQMISAB3_DEFAULT                                0x00000000
#define SEC_CFGSSQMISAB4_ADDR                                   0x00000084
#define SEC_CFGSSQMISAB4_DEFAULT                                0x00000000
#define SEC_CFGSSQMISAB5_ADDR                                   0x00000088
#define SEC_CFGSSQMISAB5_DEFAULT                                0x00000000
#define SEC_CFGSSQMISAB6_ADDR                                   0x0000008c
#define SEC_CFGSSQMISAB6_DEFAULT                                0x00000000
#define SEC_CFGSSQMISAB7_ADDR                                   0x00000090
#define SEC_CFGSSQMISAB7_DEFAULT                                0x00000000
#define SEC_CFGSSQMISAB8_ADDR                                   0x00000094
#define SEC_CFGSSQMISAB8_DEFAULT                                0x00000000
#define SEC_CFGSSQMISAB9_ADDR                                   0x00000098
#define SEC_CFGSSQMISAB9_DEFAULT                                0x00000000
#define SEC_CFGSSQMIDBGCTRL_ADDR                                   0x000000c4
#define SEC_CFGSSQMIDBGCTRL_DEFAULT                                0x00000000
#define SEC_CFGSSQMIDBGDATA0_ADDR                                   0x000000c8
#define SEC_CFGSSQMIDBGDATA0_DEFAULT                                0x00000000
#define SEC_CFGSSQMIDBGDATA1_ADDR                                   0x000000cc
#define SEC_CFGSSQMIDBGDATA1_DEFAULT                                0x00000000
#define SEC_CFGSSQMIDBGDATA2_ADDR                                   0x000000d0
#define SEC_CFGSSQMIDBGDATA2_DEFAULT                                0x00000000
#define SEC_CFGSSQMIDBGDATA3_ADDR                                   0x000000d4
#define SEC_CFGSSQMIDBGDATA3_DEFAULT                                0x00000000
#define SEC_CFGSSQMIFPQASSOC_ADDR                                   0x000000dc
#define SEC_CFGSSQMIFPQASSOC_DEFAULT                                0x00000000
#define SEC_CFGSSQMIWQASSOC_ADDR                                   0x000000e0
#define SEC_CFGSSQMIWQASSOC_DEFAULT                                0x00000000
#define SEC_CFGSSQMIMEMORY_ADDR                                   0x000000e4
#define SEC_CFGSSQMIMEMORY_DEFAULT                                0x00000000
#define SEC_CFGSSQMIQMLITE_ADDR                                   0x000000ec
#define SEC_CFGSSQMIQMLITE_DEFAULT                                0x00000040
#define SEC_CFGSSQMIQMLITEFPQASSOC_ADDR                                   0x000000f0
#define SEC_CFGSSQMIQMLITEFPQASSOC_DEFAULT                                0x00000000
#define SEC_CFGSSQMIQMLITEWQASSOC_ADDR                                   0x000000f4
#define SEC_CFGSSQMIQMLITEWQASSOC_DEFAULT                                0x00000000
#define SEC_CFGSSQMIQMHOLD_ADDR                                   0x000000f8
#define SEC_CFGSSQMIQMHOLD_DEFAULT                                0x80000003
#define SEC_CFGSSQMIFPQVCASSOC0_ADDR                                   0x00000100
#define SEC_CFGSSQMIFPQVCASSOC0_DEFAULT                                0x00000000
#define SEC_CFGSSQMIFPQVCASSOC1_ADDR                                   0x00000104
#define SEC_CFGSSQMIFPQVCASSOC1_DEFAULT                                0x00000000
#define SEC_CFGSSQMIWQVCASSOC0_ADDR                                   0x00000108
#define SEC_CFGSSQMIWQVCASSOC0_DEFAULT                                0x00000000
#define SEC_CFGSSQMIWQVCASSOC1_ADDR                                   0x0000010c
#define SEC_CFGSSQMIWQVCASSOC1_DEFAULT                                0x00000000
#define SEC_CFGSSQMIQM2_ADDR                                   0x00000110
#define SEC_CFGSSQMIQM2_DEFAULT                                0x00000078

/*	Interrupt and Status register	*/
#define SEC_STSSSQMIINT0_ADDR                                   0x0000009c
#define SEC_STSSSQMIINT0_DEFAULT                                0x00000000
#define SEC_STSSSQMIINT0MASK_ADDR                                   0x000000a0
#define SEC_STSSSQMIINT0MASK_DEFAULT                                0xffffffff
#define SEC_STSSSQMIINT1_ADDR                                   0x000000a4
#define SEC_STSSSQMIINT1_DEFAULT                                0x00000000
#define SEC_STSSSQMIINT1MASK_ADDR                                   0x000000a8
#define SEC_STSSSQMIINT1MASK_DEFAULT                                0xffffffff
#define SEC_STSSSQMIINT2_ADDR                                   0x000000ac
#define SEC_STSSSQMIINT2_DEFAULT                                0x00000000
#define SEC_STSSSQMIINT2MASK_ADDR                                   0x000000b0
#define SEC_STSSSQMIINT2MASK_DEFAULT                                0xffffffff
#define SEC_STSSSQMIINT3_ADDR                                   0x000000b4
#define SEC_STSSSQMIINT3_DEFAULT                                0x00000000
#define SEC_STSSSQMIINT3MASK_ADDR                                   0x000000b8
#define SEC_STSSSQMIINT3MASK_DEFAULT                                0xffffffff
#define SEC_STSSSQMIINT4_ADDR                                   0x000000bc
#define SEC_STSSSQMIINT4_DEFAULT                                0x00000000
#define SEC_STSSSQMIINT4MASK_ADDR                                   0x000000c0
#define SEC_STSSSQMIINT4MASK_DEFAULT                                0xffffffff
/*	Status Read only Register	*/
#define SEC_STSSSQMIFPPTR0_ADDR                                   0x00000020
#define SEC_STSSSQMIFPPTR0_DEFAULT                                0x00000000
#define SEC_STSSSQMIFPPTR1_ADDR                                   0x00000024
#define SEC_STSSSQMIFPPTR1_DEFAULT                                0x00000000
#define SEC_STSSSQMIFPPTR2_ADDR                                   0x00000028
#define SEC_STSSSQMIFPPTR2_DEFAULT                                0x00000000
#define SEC_STSSSQMIFPPTR3_ADDR                                   0x0000002c
#define SEC_STSSSQMIFPPTR3_DEFAULT                                0x00000000
#define SEC_STSSSQMIFPNUMENTRIES0_ADDR                                   0x00000030
#define SEC_STSSSQMIFPNUMENTRIES0_DEFAULT                                0x00000000
#define SEC_STSSSQMIFPNUMENTRIES1_ADDR                                   0x00000034
#define SEC_STSSSQMIFPNUMENTRIES1_DEFAULT                                0x00000000
#define SEC_STSSSQMIFPNUMENTRIES2_ADDR                                   0x00000038
#define SEC_STSSSQMIFPNUMENTRIES2_DEFAULT                                0x00000000
#define SEC_STSSSQMIFPNUMENTRIES3_ADDR                                   0x0000003c
#define SEC_STSSSQMIFPNUMENTRIES3_DEFAULT                                0x00000000
#define SEC_STSSSQMIWQPTR0_ADDR                                   0x00000040
#define SEC_STSSSQMIWQPTR0_DEFAULT                                0x00000000
#define SEC_STSSSQMIWQPTR1_ADDR                                   0x00000044
#define SEC_STSSSQMIWQPTR1_DEFAULT                                0x00000000
#define SEC_STSSSQMIWQPTR2_ADDR                                   0x00000048
#define SEC_STSSSQMIWQPTR2_DEFAULT                                0x00000000
#define SEC_STSSSQMIWQPTR3_ADDR                                   0x0000004c
#define SEC_STSSSQMIWQPTR3_DEFAULT                                0x00000000
#define SEC_STSSSQMIWQNUMENTRIES0_ADDR                                   0x00000050
#define SEC_STSSSQMIWQNUMENTRIES0_DEFAULT                                0x00000000
#define SEC_STSSSQMIWQNUMENTRIES1_ADDR                                   0x00000054
#define SEC_STSSSQMIWQNUMENTRIES1_DEFAULT                                0x00000000
#define SEC_STSSSQMIWQNUMENTRIES2_ADDR                                   0x00000058
#define SEC_STSSSQMIWQNUMENTRIES2_DEFAULT                                0x00000000
#define SEC_STSSSQMIWQNUMENTRIES3_ADDR                                   0x0000005c
#define SEC_STSSSQMIWQNUMENTRIES3_DEFAULT                                0x00000000
#define SEC_STSSSQMIWQNUMENTRIES4_ADDR                                   0x00000060
#define SEC_STSSSQMIWQNUMENTRIES4_DEFAULT                                0x00000000
#define SEC_STSSSQMIWQNUMENTRIES5_ADDR                                   0x00000064
#define SEC_STSSSQMIWQNUMENTRIES5_DEFAULT                                0x00000000
#define SEC_STSSSQMIWQNUMENTRIES6_ADDR                                   0x00000068
#define SEC_STSSSQMIWQNUMENTRIES6_DEFAULT                                0x00000000
#define SEC_STSSSQMIWQNUMENTRIES7_ADDR                                   0x0000006c
#define SEC_STSSSQMIWQNUMENTRIES7_DEFAULT                                0x00000000
#define SEC_STSSSQMIDBGDATA_ADDR                                   0x000000d8
#define SEC_STSSSQMIDBGDATA_DEFAULT                                0x00000000
#define SEC_STSSSQMIFIFO_ADDR                                   0x000000e8
#define SEC_STSSSQMIFIFO_DEFAULT                                0x07ffffff
#define SEC_STSSSQMIQMHOLD_ADDR                                   0x000000fc
#define SEC_STSSSQMIQMHOLD_DEFAULT                                0x00000000

/*	Register CfgSsQmi0	*/
/*	 Fields WQBavailFMWait	 */
#define SEC_WQBAVAILFMWAIT0_WIDTH                                             1
#define SEC_WQBAVAILFMWAIT0_SHIFT                                            31
#define SEC_WQBAVAILFMWAIT0_MASK                                     0x80000000
#define SEC_WQBAVAILFMWAIT0_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_WQBAVAILFMWAIT0_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_WQBAVAILFMWAIT0_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields FPDecDiffThreshold	 */
#define SEC_FPDECDIFFTHRESHOLD_WIDTH                                                4
#define SEC_FPDECDIFFTHRESHOLD_SHIFT                                               27
#define SEC_FPDECDIFFTHRESHOLD_MASK                                        0x78000000
#define SEC_FPDECDIFFTHRESHOLD_RD(src)                     (((src) & 0x78000000)>>27)
#define SEC_FPDECDIFFTHRESHOLD_WR(src)                (((u32)(src)<<27) & 0x78000000)
#define SEC_FPDECDIFFTHRESHOLD_SET(dst,src) \
                      (((dst) & ~0x78000000) | (((u32)(src)<<27) & 0x78000000))
/*	 Fields WQDecDiffThreshold	 */
#define SEC_WQDECDIFFTHRESHOLD_WIDTH                                                  5
#define SEC_WQDECDIFFTHRESHOLD_SHIFT                                                 22
#define SEC_WQDECDIFFTHRESHOLD_MASK                                          0x07c00000
#define SEC_WQDECDIFFTHRESHOLD_RD(src)                       (((src) & 0x07c00000)>>22)
#define SEC_WQDECDIFFTHRESHOLD_WR(src)                  (((u32)(src)<<22) & 0x07c00000)
#define SEC_WQDECDIFFTHRESHOLD_SET(dst,src) \
                      (((dst) & ~0x07c00000) | (((u32)(src)<<22) & 0x07c00000))
/*	 Fields DeallocThreshold	 */
#define SEC_DEALLOCTHRESHOLD_WIDTH                                                4
#define SEC_DEALLOCTHRESHOLD_SHIFT                                               18
#define SEC_DEALLOCTHRESHOLD_MASK                                        0x003c0000
#define SEC_DEALLOCTHRESHOLD_RD(src)                     (((src) & 0x003c0000)>>18)
#define SEC_DEALLOCTHRESHOLD_WR(src)                (((u32)(src)<<18) & 0x003c0000)
#define SEC_DEALLOCTHRESHOLD_SET(dst,src) \
                      (((dst) & ~0x003c0000) | (((u32)(src)<<18) & 0x003c0000))
/*	 Fields FPDecThreshold	 */
#define SEC_FPDECTHRESHOLD_WIDTH                                                4
#define SEC_FPDECTHRESHOLD_SHIFT                                               14
#define SEC_FPDECTHRESHOLD_MASK                                        0x0003c000
#define SEC_FPDECTHRESHOLD_RD(src)                     (((src) & 0x0003c000)>>14)
#define SEC_FPDECTHRESHOLD_WR(src)                (((u32)(src)<<14) & 0x0003c000)
#define SEC_FPDECTHRESHOLD_SET(dst,src) \
                      (((dst) & ~0x0003c000) | (((u32)(src)<<14) & 0x0003c000))
/*	 Fields FPBAvlThreshold	 */
#define SEC_FPBAVLTHRESHOLD_WIDTH                                                  4
#define SEC_FPBAVLTHRESHOLD_SHIFT                                                 10
#define SEC_FPBAVLTHRESHOLD_MASK                                          0x00003c00
#define SEC_FPBAVLTHRESHOLD_RD(src)                       (((src) & 0x00003c00)>>10)
#define SEC_FPBAVLTHRESHOLD_WR(src)                  (((u32)(src)<<10) & 0x00003c00)
#define SEC_FPBAVLTHRESHOLD_SET(dst,src) \
                      (((dst) & ~0x00003c00) | (((u32)(src)<<10) & 0x00003c00))
/*	 Fields WQDecThreshold	 */
#define SEC_WQDECTHRESHOLD_WIDTH                                                  5
#define SEC_WQDECTHRESHOLD_SHIFT                                                  5
#define SEC_WQDECTHRESHOLD_MASK                                          0x000003e0
#define SEC_WQDECTHRESHOLD_RD(src)                       (((src) & 0x000003e0)>>5)
#define SEC_WQDECTHRESHOLD_WR(src)                  (((u32)(src)<<5) & 0x000003e0)
#define SEC_WQDECTHRESHOLD_SET(dst,src) \
                      (((dst) & ~0x000003e0) | (((u32)(src)<<5) & 0x000003e0))
/*	 Fields WQBAvlThreshold	 */
#define SEC_WQBAVLTHRESHOLD_WIDTH                                         5
#define SEC_WQBAVLTHRESHOLD_SHIFT                                         0
#define SEC_WQBAVLTHRESHOLD_MASK                                 0x0000001f
#define SEC_WQBAVLTHRESHOLD_RD(src)              (((src) & 0x0000001f))
#define SEC_WQBAVLTHRESHOLD_WR(src)         (((u32)(src)) & 0x0000001f)
#define SEC_WQBAVLTHRESHOLD_SET(dst,src) \
                      (((dst) & ~0x0000001f) | (((u32)(src)) & 0x0000001f))

/*	Register CfgSsQmi1	*/
/*	 Fields CmOverrideLLFields	 */
#define SEC_CMOVERRIDELLFIELDS1_WIDTH                                         1
#define SEC_CMOVERRIDELLFIELDS1_SHIFT                                        15
#define SEC_CMOVERRIDELLFIELDS1_MASK                                 0x00008000
#define SEC_CMOVERRIDELLFIELDS1_RD(src)              (((src) & 0x00008000)>>15)
#define SEC_CMOVERRIDELLFIELDS1_WR(src)         (((u32)(src)<<15) & 0x00008000)
#define SEC_CMOVERRIDELLFIELDS1_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields CmCtrlbuffThreshold	 */
#define SEC_CMCTRLBUFFTHRESHOLD1_WIDTH                                        3
#define SEC_CMCTRLBUFFTHRESHOLD1_SHIFT                                       12
#define SEC_CMCTRLBUFFTHRESHOLD1_MASK                                0x00007000
#define SEC_CMCTRLBUFFTHRESHOLD1_RD(src)             (((src) & 0x00007000)>>12)
#define SEC_CMCTRLBUFFTHRESHOLD1_WR(src)        (((u32)(src)<<12) & 0x00007000)
#define SEC_CMCTRLBUFFTHRESHOLD1_SET(dst,src) \
                      (((dst) & ~0x00007000) | (((u32)(src)<<12) & 0x00007000))
/*	 Fields CmDatabuffThreshold	 */
#define SEC_CMDATABUFFTHRESHOLD1_WIDTH                                        5
#define SEC_CMDATABUFFTHRESHOLD1_SHIFT                                        7
#define SEC_CMDATABUFFTHRESHOLD1_MASK                                0x00000f80
#define SEC_CMDATABUFFTHRESHOLD1_RD(src)             (((src) & 0x00000f80)>>7)
#define SEC_CMDATABUFFTHRESHOLD1_WR(src)        (((u32)(src)<<7) & 0x00000f80)
#define SEC_CMDATABUFFTHRESHOLD1_SET(dst,src) \
                      (((dst) & ~0x00000f80) | (((u32)(src)<<7) & 0x00000f80))
/*	 Fields CmMsgfThreshold	 */
#define SEC_CMMSGFTHRESHOLD1_WIDTH                                            4
#define SEC_CMMSGFTHRESHOLD1_SHIFT                                            3
#define SEC_CMMSGFTHRESHOLD1_MASK                                    0x00000078
#define SEC_CMMSGFTHRESHOLD1_RD(src)                 (((src) & 0x00000078)>>3)
#define SEC_CMMSGFTHRESHOLD1_WR(src)            (((u32)(src)<<3) & 0x00000078)
#define SEC_CMMSGFTHRESHOLD1_SET(dst,src) \
                      (((dst) & ~0x00000078) | (((u32)(src)<<3) & 0x00000078))
/*	 Fields CmRegfThreshold	 */
#define SEC_CMREGFTHRESHOLD1_WIDTH                                            3
#define SEC_CMREGFTHRESHOLD1_SHIFT                                            0
#define SEC_CMREGFTHRESHOLD1_MASK                                    0x00000007
#define SEC_CMREGFTHRESHOLD1_RD(src)                 (((src) & 0x00000007)>>0)
#define SEC_CMREGFTHRESHOLD1_WR(src)            (((u32)(src)<<0) & 0x00000007)
#define SEC_CMREGFTHRESHOLD1_SET(dst,src) \
                      (((dst) & ~0x00000007) | (((u32)(src)<<0) & 0x00000007))

/*	Register CfgSsQmiQM0 	*/
/*	 Fields Address	 */
#define SEC_CFGSSQMIQM0_ADDRESS_WIDTH                                                    20
#define SEC_CFGSSQMIQM0_ADDRESS_SHIFT                                                     0
#define SEC_CFGSSQMIQM0_ADDRESS_MASK                                             0x000fffff
#define SEC_CFGSSQMIQM0_ADDRESS_RD(src)                          (((src) & 0x000fffff)>>0)
#define SEC_CFGSSQMIQM0_ADDRESS_WR(src)                     (((u32)(src)<<0) & 0x000fffff)
#define SEC_CFGSSQMIQM0_ADDRESS_SET(dst,src) \
                      (((dst) & ~0x000fffff) | (((u32)(src)<<0) & 0x000fffff))

/*      Register CfgSsQmiQM1    */
/*       Fields Address  */
#define SEC_CFGSSQMIQM1_ADDRESS_WIDTH                                                    20
#define SEC_CFGSSQMIQM1_ADDRESS_SHIFT                                                     0
#define SEC_CFGSSQMIQM1_ADDRESS_MASK                                             0x000fffff
#define SEC_CFGSSQMIQM1_ADDRESS_RD(src)                          (((src) & 0x000fffff)>>0)
#define SEC_CFGSSQMIQM1_ADDRESS_WR(src)                     (((u32)(src)<<0) & 0x000fffff)
#define SEC_CFGSSQMIQM1_ADDRESS_SET(dst,src) \
                      (((dst) & ~0x000fffff) | (((u32)(src)<<0) & 0x000fffff))

/*	Register CfgSsQmiFPDisable	*/
/*	 Fields Disable	 */
#define SEC_CFGSSQMIFPDISABLE_DISABLE_WIDTH                                                 32
#define SEC_CFGSSQMIFPDISABLE_DISABLE_SHIFT                                                 0
#define SEC_CFGSSQMIFPDISABLE_DISABLE_MASK                                          0xffffffff
#define SEC_CFGSSQMIFPDISABLE_DISABLE_RD(src)                       (((src) & 0xffffffff))
#define SEC_CFGSSQMIFPDISABLE_DISABLE_WR(src)                  (((u32)(src)) & 0xffffffff)
#define SEC_CFGSSQMIFPDISABLE_DISABLE_SET(dst,src) \
                      (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	 Register CfgSsQmiFPReset 	*/
/*	 Fields Reset	 */
#define SEC_CFGSSQMIFPRESET_RESET_WIDTH                                                   32
#define SEC_CFGSSQMIFPRESET_RESET_SHIFT                                                    0
#define SEC_CFGSSQMIFPRESET_RESET_MASK                                            0xffffffff
#define SEC_CFGSSQMIFPRESET_RESET_RD(src)                             (((src) & 0xffffffff))
#define SEC_CFGSSQMIFPRESET_RESET_WR(src)                        (((u32)(src)) & 0xffffffff)
#define SEC_CFGSSQMIFPRESET_RESET_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register CfgSsQmiWQDisable      */
/*      Fields Disable  */
#define SEC_CFGSSQMIWQDISABLE_DISABLE_WIDTH                                                 32
#define SEC_CFGSSQMIWQDISABLE_DISABLE_SHIFT                                                 0
#define SEC_CFGSSQMIWQDISABLE_DISABLE_MASK                                          0xffffffff
#define SEC_CFGSSQMIWQDISABLE_DISABLE_RD(src)                       (((src) & 0xffffffff))
#define SEC_CFGSSQMIWQDISABLE_DISABLE_WR(src)                  (((u32)(src)) & 0xffffffff)
#define SEC_CFGSSQMIWQDISABLE_DISABLE_SET(dst,src) \
                      (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*       Register CfgSsQmiWQReset       */
/*       Fields Reset    */
#define SEC_CFGSSQMIWQRESET_RESET_WIDTH                                                   32
#define SEC_CFGSSQMIWQRESET_RESET_SHIFT                                                    0
#define SEC_CFGSSQMIWQRESET_RESET_MASK                                            0xffffffff
#define SEC_CFGSSQMIWQRESET_RESET_RD(src)                             (((src) & 0xffffffff))
#define SEC_CFGSSQMIWQRESET_RESET_WR(src)                        (((u32)(src)) & 0xffffffff)
#define SEC_CFGSSQMIWQRESET_RESET_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register StsSsQmiFPPtr0	*/
/*	 Fields FP 7 */
#define SEC_FP7_WIDTH                                               3
#define SEC_FP7_SHIFT                                              28
#define SEC_FP7_MASK                                       0x70000000
#define SEC_FP7_RD(src)                        (((src) & 0x70000000)>>28)
#define SEC_FP7_SET(dst,src) \
                          (((dst) & ~0x70000000) | (((u32)(src)<<28) & 0x70000000))
/*	 Fields FP 6	 */
#define SEC_FP6_WIDTH                                               3
#define SEC_FP6_SHIFT                                              24
#define SEC_FP6_MASK                                       0x07000000
#define SEC_FP6_RD(src)                    (((src) & 0x07000000)>>24)
#define SEC_FP6_SET(dst,src) \
                      (((dst) & ~0x07000000) | (((u32)(src)<<24) & 0x07000000))
/*	 Fields FP 5	 */
#define SEC_FP5_WIDTH                                               3
#define SEC_FP5_SHIFT                                              20
#define SEC_FP5_MASK                                       0x00700000
#define SEC_FP5_RD(src)                    (((src) & 0x00700000)>>20)
#define SEC_FP5_SET(dst,src) \
                      (((dst) & ~0x00700000) | (((u32)(src)<<20) & 0x00700000))
/*	 Fields FP 4	 */
#define SEC_FP4_WIDTH                                               3
#define SEC_FP4_SHIFT                                              16
#define SEC_FP4_MASK                                       0x00070000
#define SEC_FP4_RD(src)                    (((src) & 0x00070000)>>16)
#define SEC_FP4_SET(dst,src) \
                      (((dst) & ~0x00070000) | (((u32)(src)<<16) & 0x00070000))
/*       Fields FP 3   */
#define SEC_FP3_WIDTH                                               3
#define SEC_FP3_SHIFT                                              12
#define SEC_FP3_MASK                                       0x00007000
#define SEC_FP3_RD(src)                    (((src) & 0x00007000)>>12)
#define SEC_FP3_SET(dst,src) \
                      (((dst) & ~0x00007000) | (((u32)(src)<<12) & 0x00007000))
/*       Fields FP 2   */
#define SEC_FP2_WIDTH                                               3
#define SEC_FP2_SHIFT                                              8
#define SEC_FP2_MASK                                       0x00000700
#define SEC_FP2_RD(src)                    (((src) & 0x00000700)>>8)
#define SEC_FP2_SET(dst,src) \
                      (((dst) & ~0x00000700) | (((u32)(src)<<8) & 0x00000700))
/*       Fields FP 1   */
#define SEC_FP1_WIDTH                                               3
#define SEC_FP1_SHIFT                                              4
#define SEC_FP1_MASK                                       0x00000070
#define SEC_FP1_RD(src)                    (((src) & 0x00000070)>>4)
#define SEC_FP1_SET(dst,src) \
                      (((dst) & ~0x00000070) | (((u32)(src)<<4) & 0x00000070))
/*       Fields FP 0   */
#define SEC_FP0_WIDTH                                               3
#define SEC_FP0_SHIFT                                              0
#define SEC_FP0_MASK                                       0x00000007
#define SEC_FP0_RD(src)                    (((src) & 0x00000007))
#define SEC_FP0_SET(dst,src) \
                      (((dst) & ~0x00000007) | (((u32)(src)) & 0x00000007))

/*	Register StsSsQmiFPPtr1	*/
/*	 Fields FP 15 */
#define SEC_FP15_WIDTH                                               3
#define SEC_FP15_SHIFT                                              28
#define SEC_FP15_MASK                                       0x70000000
#define SEC_FP15_RD(src)                        (((src) & 0x70000000)>>28)
#define SEC_FP15_SET(dst,src) \
                          (((dst) & ~0x70000000) | (((u32)(src)<<28) & 0x70000000))
/*	 Fields FP 14	 */
#define SEC_FP14_WIDTH                                               3
#define SEC_FP14_SHIFT                                              24
#define SEC_FP14_MASK                                       0x07000000
#define SEC_FP14_RD(src)                    (((src) & 0x07000000)>>24)
#define SEC_FP14_SET(dst,src) \
                      (((dst) & ~0x07000000) | (((u32)(src)<<24) & 0x07000000))
/*	 Fields FP 13	 */
#define SEC_FP13_WIDTH                                               3
#define SEC_FP13_SHIFT                                              20
#define SEC_FP13_MASK                                       0x00700000
#define SEC_FP13_RD(src)                    (((src) & 0x00700000)>>20)
#define SEC_FP13_SET(dst,src) \
                      (((dst) & ~0x00700000) | (((u32)(src)<<20) & 0x00700000))
/*	 Fields FP 12	 */
#define SEC_FP12_WIDTH                                               3
#define SEC_FP12_SHIFT                                              16
#define SEC_FP12_MASK                                       0x00070000
#define SEC_FP12_RD(src)                    (((src) & 0x00070000)>>16)
#define SEC_FP12_SET(dst,src) \
                      (((dst) & ~0x00070000) | (((u32)(src)<<16) & 0x00070000))
/*       Fields FP 11   */
#define SEC_FP11_WIDTH                                               3
#define SEC_FP11_SHIFT                                              12
#define SEC_FP11_MASK                                       0x00007000
#define SEC_FP11_RD(src)                    (((src) & 0x00007000)>>12)
#define SEC_FP11_SET(dst,src) \
                      (((dst) & ~0x00007000) | (((u32)(src)<<12) & 0x00007000))
/*       Fields FP 10   */
#define SEC_FP10_WIDTH                                               3
#define SEC_FP10_SHIFT                                              8
#define SEC_FP10_MASK                                       0x00000700
#define SEC_FP10_RD(src)                    (((src) & 0x00000700)>>8)
#define SEC_FP10_SET(dst,src) \
                      (((dst) & ~0x00000700) | (((u32)(src)<<8) & 0x00000700))
/*       Fields FP 9   */
#define SEC_FP9_WIDTH                                               3
#define SEC_FP9_SHIFT                                              4
#define SEC_FP9_MASK                                       0x00000070
#define SEC_FP9_RD(src)                    (((src) & 0x00000070)>>4)
#define SEC_FP9_SET(dst,src) \
                      (((dst) & ~0x00000070) | (((u32)(src)<<4) & 0x00000070))
/*       Fields FP 8   */
#define SEC_FP8_WIDTH                                               3
#define SEC_FP8_SHIFT                                              0
#define SEC_FP8_MASK                                       0x00000007
#define SEC_FP8_RD(src)                    (((src) & 0x00000007))
#define SEC_FP8_SET(dst,src) \
                      (((dst) & ~0x00000007) | (((u32)(src)) & 0x00000007))

/*	Register StsSsQmiFPPtr2	*/
/*	 Fields FP 23 */
#define SEC_FP23_WIDTH                                               3
#define SEC_FP23_SHIFT                                              28
#define SEC_FP23_MASK                                       0x70000000
#define SEC_FP23_RD(src)                        (((src) & 0x70000000)>>28)
#define SEC_FP23_SET(dst,src) \
                          (((dst) & ~0x70000000) | (((u32)(src)<<28) & 0x70000000))
/*	 Fields FP 22	 */
#define SEC_FP22_WIDTH                                               3
#define SEC_FP22_SHIFT                                              24
#define SEC_FP22_MASK                                       0x07000000
#define SEC_FP22_RD(src)                    (((src) & 0x07000000)>>24)
#define SEC_FP22_SET(dst,src) \
                      (((dst) & ~0x07000000) | (((u32)(src)<<24) & 0x07000000))
/*	 Fields FP 21	 */
#define SEC_FP21_WIDTH                                               3
#define SEC_FP21_SHIFT                                              20
#define SEC_FP21_MASK                                       0x00700000
#define SEC_FP21_RD(src)                    (((src) & 0x00700000)>>20)
#define SEC_FP21_SET(dst,src) \
                      (((dst) & ~0x00700000) | (((u32)(src)<<20) & 0x00700000))
/*	 Fields FP 20	 */
#define SEC_FP20_WIDTH                                               3
#define SEC_FP20_SHIFT                                              16
#define SEC_FP20_MASK                                       0x00070000
#define SEC_FP20_RD(src)                    (((src) & 0x00070000)>>16)
#define SEC_FP20_SET(dst,src) \
                      (((dst) & ~0x00070000) | (((u32)(src)<<16) & 0x00070000))
/*       Fields FP 19   */
#define SEC_FP19_WIDTH                                               3
#define SEC_FP19_SHIFT                                              12
#define SEC_FP19_MASK                                       0x00007000
#define SEC_FP19_RD(src)                    (((src) & 0x00007000)>>12)
#define SEC_FP19_SET(dst,src) \
                      (((dst) & ~0x00007000) | (((u32)(src)<<12) & 0x00007000))
/*       Fields FP 18   */
#define SEC_FP18_WIDTH                                               3
#define SEC_FP18_SHIFT                                              8
#define SEC_FP18_MASK                                       0x00000700
#define SEC_FP18_RD(src)                    (((src) & 0x00000700)>>8)
#define SEC_FP18_SET(dst,src) \
                      (((dst) & ~0x00000700) | (((u32)(src)<<8) & 0x00000700))
/*       Fields FP 17   */
#define SEC_FP17_WIDTH                                               3
#define SEC_FP17_SHIFT                                              4
#define SEC_FP17_MASK                                       0x00000070
#define SEC_FP17_RD(src)                    (((src) & 0x00000070)>>4)
#define SEC_FP17_SET(dst,src) \
                      (((dst) & ~0x00000070) | (((u32)(src)<<4) & 0x00000070))
/*       Fields FP 16   */
#define SEC_FP16_WIDTH                                               3
#define SEC_FP16_SHIFT                                              0
#define SEC_FP16_MASK                                       0x00000007
#define SEC_FP16_RD(src)                    (((src) & 0x00000007))
#define SEC_FP16_SET(dst,src) \
                      (((dst) & ~0x00000007) | (((u32)(src)) & 0x00000007))

/*	Register StsSsQmiFPPtr3	*/
/*	 Fields FP 31 */
#define SEC_FP31_WIDTH                                               3
#define SEC_FP31_SHIFT                                              28
#define SEC_FP31_MASK                                       0x70000000
#define SEC_FP31_RD(src)                        (((src) & 0x70000000)>>28)
#define SEC_FP31_SET(dst,src) \
                          (((dst) & ~0x70000000) | (((u32)(src)<<28) & 0x70000000))
/*	 Fields FP 30	 */
#define SEC_FP30_WIDTH                                               3
#define SEC_FP30_SHIFT                                              24
#define SEC_FP30_MASK                                       0x07000000
#define SEC_FP30_RD(src)                    (((src) & 0x07000000)>>24)
#define SEC_FP30_SET(dst,src) \
                      (((dst) & ~0x07000000) | (((u32)(src)<<24) & 0x07000000))
/*	 Fields FP 29	 */
#define SEC_FP29_WIDTH                                               3
#define SEC_FP29_SHIFT                                              20
#define SEC_FP29_MASK                                       0x00700000
#define SEC_FP29_RD(src)                    (((src) & 0x00700000)>>20)
#define SEC_FP29_SET(dst,src) \
                      (((dst) & ~0x00700000) | (((u32)(src)<<20) & 0x00700000))
/*	 Fields FP 28	 */
#define SEC_FP28_WIDTH                                               3
#define SEC_FP28_SHIFT                                              16
#define SEC_FP28_MASK                                       0x00070000
#define SEC_FP28_RD(src)                    (((src) & 0x00070000)>>16)
#define SEC_FP28_SET(dst,src) \
                      (((dst) & ~0x00070000) | (((u32)(src)<<16) & 0x00070000))
/*       Fields FP 27   */
#define SEC_FP27_WIDTH                                               3
#define SEC_FP27_SHIFT                                              12
#define SEC_FP27_MASK                                       0x00007000
#define SEC_FP27_RD(src)                    (((src) & 0x00007000)>>12)
#define SEC_FP27_SET(dst,src) \
                      (((dst) & ~0x00007000) | (((u32)(src)<<12) & 0x00007000))
/*       Fields FP 26   */
#define SEC_FP26_WIDTH                                               3
#define SEC_FP26_SHIFT                                              8
#define SEC_FP26_MASK                                       0x00000700
#define SEC_FP26_RD(src)                    (((src) & 0x00000700)>>8)
#define SEC_FP26_SET(dst,src) \
                      (((dst) & ~0x00000700) | (((u32)(src)<<8) & 0x00000700))
/*       Fields FP 25   */
#define SEC_FP25_WIDTH                                               3
#define SEC_FP25_SHIFT                                              4
#define SEC_FP25_MASK                                       0x00000070
#define SEC_FP25_RD(src)                    (((src) & 0x00000070)>>4)
#define SEC_FP25_SET(dst,src) \
                      (((dst) & ~0x00000070) | (((u32)(src)<<4) & 0x00000070))
/*       Fields FP 24   */
#define SEC_FP24_WIDTH                                               3
#define SEC_FP24_SHIFT                                              0
#define SEC_FP24_MASK                                       0x00000007
#define SEC_FP24_RD(src)                    (((src) & 0x00000007))
#define SEC_FP24_SET(dst,src) \
                      (((dst) & ~0x00000007) | (((u32)(src)) & 0x00000007))

/*	Register StsSsQmiFPNumEntries0	*/
/*	 Fields FP 7 */
#define SEC_ENTRIESFP7_WIDTH                                               4
#define SEC_ENTRIESFP7_SHIFT                                              28
#define SEC_ENTRIESFP7_MASK                                       0xf0000000
#define SEC_ENTRIESFP7_RD(src)                        (((src) & 0xf0000000)>>28)
#define SEC_ENTRIESFP7_SET(dst,src) \
                          (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields FP 6	 */
#define SEC_ENTRIESFP6_WIDTH                                               4
#define SEC_ENTRIESFP6_SHIFT                                              24
#define SEC_ENTRIESFP6_MASK                                       0x0f000000
#define SEC_ENTRIESFP6_RD(src)                    (((src) & 0x0f000000)>>24)
#define SEC_ENTRIESFP6_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields FP 5	 */
#define SEC_ENTRIESFP5_WIDTH                                               4
#define SEC_ENTRIESFP5_SHIFT                                              20
#define SEC_ENTRIESFP5_MASK                                       0x00f00000
#define SEC_ENTRIESFP5_RD(src)                    (((src) & 0x00f00000)>>20)
#define SEC_ENTRIESFP5_SET(dst,src) \
                      (((dst) & ~0x00f00000) | (((u32)(src)<<20) & 0x00f00000))
/*	 Fields FP 4	 */
#define SEC_ENTRIESFP4_WIDTH                                               4
#define SEC_ENTRIESFP4_SHIFT                                              16
#define SEC_ENTRIESFP4_MASK                                       0x000f0000
#define SEC_ENTRIESFP4_RD(src)                    (((src) & 0x000f0000)>>16)
#define SEC_ENTRIESFP4_SET(dst,src) \
                      (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))
/*       Fields FP 4   */
#define SEC_ENTRIESFP3_WIDTH                                               4
#define SEC_ENTRIESFP3_SHIFT                                              12
#define SEC_ENTRIESFP3_MASK                                       0x0000f000
#define SEC_ENTRIESFP3_RD(src)                    (((src) & 0x0000f000)>>12)
#define SEC_ENTRIESFP3_SET(dst,src) \
                      (((dst) & ~0x0000f000) | (((u32)(src)<<12) & 0x0000f000))
/*       Fields FP 2   */
#define SEC_ENTRIESFP2_WIDTH                                               4
#define SEC_ENTRIESFP2_SHIFT                                              8
#define SEC_ENTRIESFP2_MASK                                       0x00000f00
#define SEC_ENTRIESFP2_RD(src)                    (((src) & 0x00000f00)>>8)
#define SEC_ENTRIESFP2_SET(dst,src) \
                      (((dst) & ~0x00000f00) | (((u32)(src)<<8) & 0x00000f00))
/*       Fields FP 1   */
#define SEC_ENTRIESFP1_WIDTH                                               4
#define SEC_ENTRIESFP1_SHIFT                                              4
#define SEC_ENTRIESFP1_MASK                                       0x000000f0
#define SEC_ENTRIESFP1_RD(src)                    (((src) & 0x000000f0)>>4)
#define SEC_ENTRIESFP1_SET(dst,src) \
                      (((dst) & ~0x000000f0) | (((u32)(src)<<4) & 0x000000f0))
/*       Fields FP 0   */
#define SEC_ENTRIESFP0_WIDTH                                               4
#define SEC_ENTRIESFP0_SHIFT                                              0
#define SEC_ENTRIESFP0_MASK                                       0x0000000f
#define SEC_ENTRIESFP0_RD(src)                    (((src) & 0x0000000f))
#define SEC_ENTRIESFP0_SET(dst,src) \
                      (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register StsSsQmiFPNumEntries1	*/
/*	 Fields FP 15 */
#define SEC_ENTRIESFP15_WIDTH                                               4
#define SEC_ENTRIESFP15_SHIFT                                              28
#define SEC_ENTRIESFP15_MASK                                       0xf0000000
#define SEC_ENTRIESFP15_RD(src)                        (((src) & 0xf0000000)>>28)
#define SEC_ENTRIESFP15_SET(dst,src) \
                          (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields FP 14	 */
#define SEC_ENTRIESFP14_WIDTH                                               4
#define SEC_ENTRIESFP14_SHIFT                                              24
#define SEC_ENTRIESFP14_MASK                                       0x0f000000
#define SEC_ENTRIESFP14_RD(src)                    (((src) & 0x0f000000)>>24)
#define SEC_ENTRIESFP14_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields FP 13	 */
#define SEC_ENTRIESFP13_WIDTH                                               4
#define SEC_ENTRIESFP13_SHIFT                                              20
#define SEC_ENTRIESFP13_MASK                                       0x00f00000
#define SEC_ENTRIESFP13_RD(src)                    (((src) & 0x00f00000)>>20)
#define SEC_ENTRIESFP13_SET(dst,src) \
                      (((dst) & ~0x00f00000) | (((u32)(src)<<20) & 0x00f00000))
/*	 Fields FP 12	 */
#define SEC_ENTRIESFP12_WIDTH                                               4
#define SEC_ENTRIESFP12_SHIFT                                              16
#define SEC_ENTRIESFP12_MASK                                       0x000f0000
#define SEC_ENTRIESFP12_RD(src)                    (((src) & 0x000f0000)>>16)
#define SEC_ENTRIESFP12_SET(dst,src) \
                      (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))
/*       Fields FP 11   */
#define SEC_ENTRIESFP11_WIDTH                                               4
#define SEC_ENTRIESFP11_SHIFT                                              12
#define SEC_ENTRIESFP11_MASK                                       0x0000f000
#define SEC_ENTRIESFP11_RD(src)                    (((src) & 0x0000f000)>>12)
#define SEC_ENTRIESFP11_SET(dst,src) \
                      (((dst) & ~0x0000f000) | (((u32)(src)<<12) & 0x0000f000))
/*       Fields FP 10   */
#define SEC_ENTRIESFP10_WIDTH                                               4
#define SEC_ENTRIESFP10_SHIFT                                              8
#define SEC_ENTRIESFP10_MASK                                       0x00000f00
#define SEC_ENTRIESFP10_RD(src)                    (((src) & 0x00000f00)>>8)
#define SEC_ENTRIESFP10_SET(dst,src) \
                      (((dst) & ~0x00000f00) | (((u32)(src)<<8) & 0x00000f00))
/*       Fields FP 9   */
#define SEC_ENTRIESFP9_WIDTH                                               4
#define SEC_ENTRIESFP9_SHIFT                                              4
#define SEC_ENTRIESFP9_MASK                                       0x000000f0
#define SEC_ENTRIESFP9_RD(src)                    (((src) & 0x000000f0)>>4)
#define SEC_ENTRIESFP9_SET(dst,src) \
                      (((dst) & ~0x000000f0) | (((u32)(src)<<4) & 0x000000f0))
/*       Fields FP 8   */
#define SEC_ENTRIESFP8_WIDTH                                               4
#define SEC_ENTRIESFP8_SHIFT                                              0
#define SEC_ENTRIESFP8_MASK                                       0x0000000f
#define SEC_ENTRIESFP8_RD(src)                    (((src) & 0x0000000f))
#define SEC_ENTRIESFP8_SET(dst,src) \
                      (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register StsSsQmiFPNumEntries2	*/
/*	 Fields FP 23 */
#define SEC_ENTRIESFP23_WIDTH                                               4
#define SEC_ENTRIESFP23_SHIFT                                              28
#define SEC_ENTRIESFP23_MASK                                       0xf0000000
#define SEC_ENTRIESFP23_RD(src)                        (((src) & 0xf0000000)>>28)
#define SEC_ENTRIESFP23_SET(dst,src) \
                          (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields FP 22	 */
#define SEC_ENTRIESFP22_WIDTH                                               4
#define SEC_ENTRIESFP22_SHIFT                                              24
#define SEC_ENTRIESFP22_MASK                                       0x0f000000
#define SEC_ENTRIESFP22_RD(src)                    (((src) & 0x0f000000)>>24)
#define SEC_ENTRIESFP22_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields FP 21	 */
#define SEC_ENTRIESFP21_WIDTH                                               4
#define SEC_ENTRIESFP21_SHIFT                                              20
#define SEC_ENTRIESFP21_MASK                                       0x00f00000
#define SEC_ENTRIESFP21_RD(src)                    (((src) & 0x00f00000)>>20)
#define SEC_ENTRIESFP21_SET(dst,src) \
                      (((dst) & ~0x00f00000) | (((u32)(src)<<20) & 0x00f00000))
/*	 Fields FP 20	 */
#define SEC_ENTRIESFP20_WIDTH                                               4
#define SEC_ENTRIESFP20_SHIFT                                              16
#define SEC_ENTRIESFP20_MASK                                       0x000f0000
#define SEC_ENTRIESFP20_RD(src)                    (((src) & 0x000f0000)>>16)
#define SEC_ENTRIESFP20_SET(dst,src) \
                      (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))
/*       Fields FP 19   */
#define SEC_ENTRIESFP19_WIDTH                                               4
#define SEC_ENTRIESFP19_SHIFT                                              12
#define SEC_ENTRIESFP19_MASK                                       0x0000f000
#define SEC_ENTRIESFP19_RD(src)                    (((src) & 0x0000f000)>>12)
#define SEC_ENTRIESFP19_SET(dst,src) \
                      (((dst) & ~0x0000f000) | (((u32)(src)<<12) & 0x0000f000))
/*       Fields FP 18   */
#define SEC_ENTRIESFP18_WIDTH                                               4
#define SEC_ENTRIESFP18_SHIFT                                              8
#define SEC_ENTRIESFP18_MASK                                       0x00000f00
#define SEC_ENTRIESFP18_RD(src)                    (((src) & 0x00000f00)>>8)
#define SEC_ENTRIESFP18_SET(dst,src) \
                      (((dst) & ~0x00000f00) | (((u32)(src)<<8) & 0x00000f00))
/*       Fields FP 17   */
#define SEC_ENTRIESFP17_WIDTH                                               4
#define SEC_ENTRIESFP17_SHIFT                                              4
#define SEC_ENTRIESFP17_MASK                                       0x000000f0
#define SEC_ENTRIESFP17_RD(src)                    (((src) & 0x000000f0)>>4)
#define SEC_ENTRIESFP17_SET(dst,src) \
                      (((dst) & ~0x000000f0) | (((u32)(src)<<4) & 0x000000f0))
/*       Fields FP 16   */
#define SEC_ENTRIESFP16_WIDTH                                               4
#define SEC_ENTRIESFP16_SHIFT                                              0
#define SEC_ENTRIESFP16_MASK                                       0x0000000f
#define SEC_ENTRIESFP16_RD(src)                    (((src) & 0x0000000f))
#define SEC_ENTRIESFP16_SET(dst,src) \
                      (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register StsSsQmiFPNumEntries3	*/
/*	 Fields FP 31 */
#define SEC_ENTRIESFP31_WIDTH                                               4
#define SEC_ENTRIESFP31_SHIFT                                              28
#define SEC_ENTRIESFP31_MASK                                       0xf0000000
#define SEC_ENTRIESFP31_RD(src)                        (((src) & 0xf0000000)>>28)
#define SEC_ENTRIESFP31_SET(dst,src) \
                          (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields FP 30	 */
#define SEC_ENTRIESFP30_WIDTH                                               4
#define SEC_ENTRIESFP30_SHIFT                                              24
#define SEC_ENTRIESFP30_MASK                                       0x0f000000
#define SEC_ENTRIESFP30_RD(src)                    (((src) & 0x0f000000)>>24)
#define SEC_ENTRIESFP30_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields FP 29	 */
#define SEC_ENTRIESFP29_WIDTH                                               4
#define SEC_ENTRIESFP29_SHIFT                                              20
#define SEC_ENTRIESFP29_MASK                                       0x00f00000
#define SEC_ENTRIESFP29_RD(src)                    (((src) & 0x00f00000)>>20)
#define SEC_ENTRIESFP29_SET(dst,src) \
                      (((dst) & ~0x00f00000) | (((u32)(src)<<20) & 0x00f00000))
/*	 Fields FP 28	 */
#define SEC_ENTRIESFP28_WIDTH                                               4
#define SEC_ENTRIESFP28_SHIFT                                              16
#define SEC_ENTRIESFP28_MASK                                       0x000f0000
#define SEC_ENTRIESFP28_RD(src)                    (((src) & 0x000f0000)>>16)
#define SEC_ENTRIESFP28_SET(dst,src) \
                      (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))
/*       Fields FP 27   */
#define SEC_ENTRIESFP27_WIDTH                                               4
#define SEC_ENTRIESFP27_SHIFT                                              12
#define SEC_ENTRIESFP27_MASK                                       0x0000f000
#define SEC_ENTRIESFP27_RD(src)                    (((src) & 0x0000f000)>>12)
#define SEC_ENTRIESFP27_SET(dst,src) \
                      (((dst) & ~0x0000f000) | (((u32)(src)<<12) & 0x0000f000))
/*       Fields FP 26   */
#define SEC_ENTRIESFP26_WIDTH                                               4
#define SEC_ENTRIESFP26_SHIFT                                              8
#define SEC_ENTRIESFP26_MASK                                       0x00000f00
#define SEC_ENTRIESFP26_RD(src)                    (((src) & 0x00000f00)>>8)
#define SEC_ENTRIESFP26_SET(dst,src) \
                      (((dst) & ~0x00000f00) | (((u32)(src)<<8) & 0x00000f00))
/*       Fields FP 25   */
#define SEC_ENTRIESFP25_WIDTH                                               4
#define SEC_ENTRIESFP25_SHIFT                                              4
#define SEC_ENTRIESFP25_MASK                                       0x000000f0
#define SEC_ENTRIESFP25_RD(src)                    (((src) & 0x000000f0)>>4)
#define SEC_ENTRIESFP25_SET(dst,src) \
                      (((dst) & ~0x000000f0) | (((u32)(src)<<4) & 0x000000f0))
/*       Fields FP 24   */
#define SEC_ENTRIESFP24_WIDTH                                               4
#define SEC_ENTRIESFP24_SHIFT                                              0
#define SEC_ENTRIESFP24_MASK                                       0x0000000f
#define SEC_ENTRIESFP24_RD(src)                    (((src) & 0x0000000f))
#define SEC_ENTRIESFP24_SET(dst,src) \
                      (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register StsSsQmiWQPtr0	*/
/*	 Fields WQ 7 */
#define SEC_WQ7_WIDTH                                               4
#define SEC_WQ7_SHIFT                                              28
#define SEC_WQ7_MASK                                       0xf0000000
#define SEC_WQ7_RD(src)                        (((src) & 0xf0000000)>>28)
#define SEC_WQ7_SET(dst,src) \
                          (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields WQ 6	 */
#define SEC_WQ6_WIDTH                                               4
#define SEC_WQ6_SHIFT                                              24
#define SEC_WQ6_MASK                                       0x0f000000
#define SEC_WQ6_RD(src)                    (((src) & 0x0f000000)>>24)
#define SEC_WQ6_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields WQ 5	 */
#define SEC_WQ5_WIDTH                                               4
#define SEC_WQ5_SHIFT                                              20
#define SEC_WQ5_MASK                                       0x00f00000
#define SEC_WQ5_RD(src)                    (((src) & 0x00f00000)>>20)
#define SEC_WQ5_SET(dst,src) \
                      (((dst) & ~0x00f00000) | (((u32)(src)<<20) & 0x00f00000))
/*	 Fields WQ 4	 */
#define SEC_WQ4_WIDTH                                               4
#define SEC_WQ4_SHIFT                                              16
#define SEC_WQ4_MASK                                       0x000f0000
#define SEC_WQ4_RD(src)                    (((src) & 0x000f0000)>>16)
#define SEC_WQ4_SET(dst,src) \
                      (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))
/*       Fields WQ 4   */
#define SEC_WQ3_WIDTH                                               4
#define SEC_WQ3_SHIFT                                              12
#define SEC_WQ3_MASK                                       0x0000f000
#define SEC_WQ3_RD(src)                    (((src) & 0x0000f000)>>12)
#define SEC_WQ3_SET(dst,src) \
                      (((dst) & ~0x0000f000) | (((u32)(src)<<12) & 0x0000f000))
/*       Fields WQ 2   */
#define SEC_WQ2_WIDTH                                               4
#define SEC_WQ2_SHIFT                                              8
#define SEC_WQ2_MASK                                       0x00000f00
#define SEC_WQ2_RD(src)                    (((src) & 0x00000f00)>>8)
#define SEC_WQ2_SET(dst,src) \
                      (((dst) & ~0x00000f00) | (((u32)(src)<<8) & 0x00000f00))
/*       Fields WQ 1   */
#define SEC_WQ1_WIDTH                                               4
#define SEC_WQ1_SHIFT                                              4
#define SEC_WQ1_MASK                                       0x000000f0
#define SEC_WQ1_RD(src)                    (((src) & 0x000000f0)>>4)
#define SEC_WQ1_SET(dst,src) \
                      (((dst) & ~0x000000f0) | (((u32)(src)<<4) & 0x000000f0))
/*       Fields WQ 0   */
#define SEC_WQ0_WIDTH                                               4
#define SEC_WQ0_SHIFT                                              0
#define SEC_WQ0_MASK                                       0x0000000f
#define SEC_WQ0_RD(src)                    (((src) & 0x0000000f))
#define SEC_WQ0_SET(dst,src) \
                      (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register StsSsQmiWQPtr1	*/
/*	 Fields WQ 15 */
#define SEC_WQ15_WIDTH                                               4
#define SEC_WQ15_SHIFT                                              28
#define SEC_WQ15_MASK                                       0xf0000000
#define SEC_WQ15_RD(src)                        (((src) & 0xf0000000)>>28)
#define SEC_WQ15_SET(dst,src) \
                          (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields WQ 14	 */
#define SEC_WQ14_WIDTH                                               4
#define SEC_WQ14_SHIFT                                              24
#define SEC_WQ14_MASK                                       0x0f000000
#define SEC_WQ14_RD(src)                    (((src) & 0x0f000000)>>24)
#define SEC_WQ14_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields WQ 13	 */
#define SEC_WQ13_WIDTH                                               4
#define SEC_WQ13_SHIFT                                              20
#define SEC_WQ13_MASK                                       0x00f00000
#define SEC_WQ13_RD(src)                    (((src) & 0x00f00000)>>20)
#define SEC_WQ13_SET(dst,src) \
                      (((dst) & ~0x00f00000) | (((u32)(src)<<20) & 0x00f00000))
/*	 Fields WQ 12	 */
#define SEC_WQ12_WIDTH                                               4
#define SEC_WQ12_SHIFT                                              16
#define SEC_WQ12_MASK                                       0x000f0000
#define SEC_WQ12_RD(src)                    (((src) & 0x000f0000)>>16)
#define SEC_WQ12_SET(dst,src) \
                      (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))
/*       Fields WQ 11   */
#define SEC_WQ11_WIDTH                                               4
#define SEC_WQ11_SHIFT                                              12
#define SEC_WQ11_MASK                                       0x0000f000
#define SEC_WQ11_RD(src)                    (((src) & 0x0000f000)>>12)
#define SEC_WQ11_SET(dst,src) \
                      (((dst) & ~0x0000f000) | (((u32)(src)<<12) & 0x0000f000))
/*       Fields WQ 10   */
#define SEC_WQ10_WIDTH                                               4
#define SEC_WQ10_SHIFT                                              8
#define SEC_WQ10_MASK                                       0x00000f00
#define SEC_WQ10_RD(src)                    (((src) & 0x00000f00)>>8)
#define SEC_WQ10_SET(dst,src) \
                      (((dst) & ~0x00000f00) | (((u32)(src)<<8) & 0x00000f00))
/*       Fields WQ 9   */
#define SEC_WQ9_WIDTH                                               4
#define SEC_WQ9_SHIFT                                              4
#define SEC_WQ9_MASK                                       0x000000f0
#define SEC_WQ9_RD(src)                    (((src) & 0x000000f0)>>4)
#define SEC_WQ9_SET(dst,src) \
                      (((dst) & ~0x000000f0) | (((u32)(src)<<4) & 0x000000f0))
/*       Fields WQ 8   */
#define SEC_WQ8_WIDTH                                               4
#define SEC_WQ8_SHIFT                                              0
#define SEC_WQ8_MASK                                       0x0000000f
#define SEC_WQ8_RD(src)                    (((src) & 0x0000000f))
#define SEC_WQ8_SET(dst,src) \
                      (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register StsSsQmiWQPtr2	*/
/*	 Fields WQ 23 */
#define SEC_WQ23_WIDTH                                               4
#define SEC_WQ23_SHIFT                                              28
#define SEC_WQ23_MASK                                       0xf0000000
#define SEC_WQ23_RD(src)                        (((src) & 0xf0000000)>>28)
#define SEC_WQ23_SET(dst,src) \
                          (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields WQ 22	 */
#define SEC_WQ22_WIDTH                                               4
#define SEC_WQ22_SHIFT                                              24
#define SEC_WQ22_MASK                                       0x0f000000
#define SEC_WQ22_RD(src)                    (((src) & 0x0f000000)>>24)
#define SEC_WQ22_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields WQ 21	 */
#define SEC_WQ21_WIDTH                                               4
#define SEC_WQ21_SHIFT                                              20
#define SEC_WQ21_MASK                                       0x00f00000
#define SEC_WQ21_RD(src)                    (((src) & 0x00f00000)>>20)
#define SEC_WQ21_SET(dst,src) \
                      (((dst) & ~0x00f00000) | (((u32)(src)<<20) & 0x00f00000))
/*	 Fields WQ 20	 */
#define SEC_WQ20_WIDTH                                               4
#define SEC_WQ20_SHIFT                                              16
#define SEC_WQ20_MASK                                       0x000f0000
#define SEC_WQ20_RD(src)                    (((src) & 0x000f0000)>>16)
#define SEC_WQ20_SET(dst,src) \
                      (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))
/*       Fields WQ 19   */
#define SEC_WQ19_WIDTH                                               4
#define SEC_WQ19_SHIFT                                              12
#define SEC_WQ19_MASK                                       0x0000f000
#define SEC_WQ19_RD(src)                    (((src) & 0x0000f000)>>12)
#define SEC_WQ19_SET(dst,src) \
                      (((dst) & ~0x0000f000) | (((u32)(src)<<12) & 0x0000f000))
/*       Fields WQ 18   */
#define SEC_WQ18_WIDTH                                               4
#define SEC_WQ18_SHIFT                                              8
#define SEC_WQ18_MASK                                       0x00000f00
#define SEC_WQ18_RD(src)                    (((src) & 0x00000f00)>>8)
#define SEC_WQ18_SET(dst,src) \
                      (((dst) & ~0x00000f00) | (((u32)(src)<<8) & 0x00000f00))
/*       Fields WQ 17   */
#define SEC_WQ17_WIDTH                                               4
#define SEC_WQ17_SHIFT                                              4
#define SEC_WQ17_MASK                                       0x000000f0
#define SEC_WQ17_RD(src)                    (((src) & 0x000000f0)>>4)
#define SEC_WQ17_SET(dst,src) \
                      (((dst) & ~0x000000f0) | (((u32)(src)<<4) & 0x000000f0))
/*       Fields WQ 16   */
#define SEC_WQ16_WIDTH                                               4
#define SEC_WQ16_SHIFT                                              0
#define SEC_WQ16_MASK                                       0x0000000f
#define SEC_WQ16_RD(src)                    (((src) & 0x0000000f))
#define SEC_WQ16_SET(dst,src) \
                      (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register StsSsQmiWQPtr3	*/
/*	 Fields WQ 31 */
#define SEC_WQ31_WIDTH                                               4
#define SEC_WQ31_SHIFT                                              28
#define SEC_WQ31_MASK                                       0xf0000000
#define SEC_WQ31_RD(src)                        (((src) & 0xf0000000)>>28)
#define SEC_WQ31_SET(dst,src) \
                          (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields WQ 30	 */
#define SEC_WQ30_WIDTH                                               4
#define SEC_WQ30_SHIFT                                              24
#define SEC_WQ30_MASK                                       0x0f000000
#define SEC_WQ30_RD(src)                    (((src) & 0x0f000000)>>24)
#define SEC_WQ30_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields WQ 29	 */
#define SEC_WQ29_WIDTH                                               4
#define SEC_WQ29_SHIFT                                              20
#define SEC_WQ29_MASK                                       0x00f00000
#define SEC_WQ29_RD(src)                    (((src) & 0x00f00000)>>20)
#define SEC_WQ29_SET(dst,src) \
                      (((dst) & ~0x00f00000) | (((u32)(src)<<20) & 0x00f00000))
/*	 Fields WQ 28	 */
#define SEC_WQ28_WIDTH                                               4
#define SEC_WQ28_SHIFT                                              16
#define SEC_WQ28_MASK                                       0x000f0000
#define SEC_WQ28_RD(src)                    (((src) & 0x000f0000)>>16)
#define SEC_WQ28_SET(dst,src) \
                      (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))
/*       Fields WQ 27   */
#define SEC_WQ27_WIDTH                                               4
#define SEC_WQ27_SHIFT                                              12
#define SEC_WQ27_MASK                                       0x0000f000
#define SEC_WQ27_RD(src)                    (((src) & 0x0000f000)>>12)
#define SEC_WQ27_SET(dst,src) \
                      (((dst) & ~0x0000f000) | (((u32)(src)<<12) & 0x0000f000))
/*       Fields WQ 26   */
#define SEC_WQ26_WIDTH                                               4
#define SEC_WQ26_SHIFT                                              8
#define SEC_WQ26_MASK                                       0x00000f00
#define SEC_WQ26_RD(src)                    (((src) & 0x00000f00)>>8)
#define SEC_WQ26_SET(dst,src) \
                      (((dst) & ~0x00000f00) | (((u32)(src)<<8) & 0x00000f00))
/*       Fields WQ 25   */
#define SEC_WQ25_WIDTH                                               4
#define SEC_WQ25_SHIFT                                              4
#define SEC_WQ25_MASK                                       0x000000f0
#define SEC_WQ25_RD(src)                    (((src) & 0x000000f0)>>4)
#define SEC_WQ25_SET(dst,src) \
                      (((dst) & ~0x000000f0) | (((u32)(src)<<4) & 0x000000f0))
/*       Fields WQ 24   */
#define SEC_WQ24_WIDTH                                               4
#define SEC_WQ24_SHIFT                                              0
#define SEC_WQ24_MASK                                       0x0000000f
#define SEC_WQ24_RD(src)                    (((src) & 0x0000000f))
#define SEC_WQ24_SET(dst,src) \
                      (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register StsSsQmiWQNumEntries0	*/
/*	 Fields WQ 3 */
#define SEC_ENTRIESWQ3_WIDTH                                               5
#define SEC_ENTRIESWQ3_SHIFT                                              24
#define SEC_ENTRIESWQ3_MASK                                       0x1f000000
#define SEC_ENTRIESWQ3_RD(src)                        (((src) & 0x1f000000)>>24)
#define SEC_ENTRIESWQ3_SET(dst,src) \
                          (((dst) & ~0x1f000000) | (((u32)(src)<<24) & 0x1f000000))
/*       Fields WQ 2 */
#define SEC_ENTRIESWQ2_WIDTH                                               5
#define SEC_ENTRIESWQ2_SHIFT                                              16
#define SEC_ENTRIESWQ2_MASK                                       0x001f0000
#define SEC_ENTRIESWQ2_RD(src)                        (((src) & 0x001f0000)>>16)
#define SEC_ENTRIESWQ2_SET(dst,src) \
                          (((dst) & ~0x001f0000) | (((u32)(src)<<16) & 0x001f0000))
/*       Fields WQ 1 */
#define SEC_ENTRIESWQ1_WIDTH                                               5
#define SEC_ENTRIESWQ1_SHIFT                                              8
#define SEC_ENTRIESWQ1_MASK                                       0x00001f00
#define SEC_ENTRIESWQ1_RD(src)                        (((src) & 0x00001f00)>>8)
#define SEC_ENTRIESWQ1_SET(dst,src) \
                          (((dst) & ~0x00001f00) | (((u32)(src)<<8) & 0x00001f00))
/*       Fields WQ 0 */
#define SEC_ENTRIESWQ0_WIDTH                                               5
#define SEC_ENTRIESWQ0_SHIFT                                              0
#define SEC_ENTRIESWQ0_MASK                                       0x0000001f
#define SEC_ENTRIESWQ0_RD(src)                        (((src) & 0x0000001f))
#define SEC_ENTRIESWQ0_SET(dst,src) \
                          (((dst) & ~0x0000001f) | (((u32)(src)) & 0x0000001f))

/*	Register StsSsQmiWQNumEntries1	*/
/*	 Fields WQ 7 */
#define SEC_ENTRIESWQ7_WIDTH                                               5
#define SEC_ENTRIESWQ7_SHIFT                                              24
#define SEC_ENTRIESWQ7_MASK                                       0x1f000000
#define SEC_ENTRIESWQ7_RD(src)                        (((src) & 0x1f000000)>>24)
#define SEC_ENTRIESWQ7_SET(dst,src) \
                          (((dst) & ~0x1f000000) | (((u32)(src)<<24) & 0x1f000000))
/*       Fields WQ 6 */
#define SEC_ENTRIESWQ6_WIDTH                                               5
#define SEC_ENTRIESWQ6_SHIFT                                              16
#define SEC_ENTRIESWQ6_MASK                                       0x001f0000
#define SEC_ENTRIESWQ6_RD(src)                        (((src) & 0x001f0000)>>16)
#define SEC_ENTRIESWQ6_SET(dst,src) \
                          (((dst) & ~0x001f0000) | (((u32)(src)<<16) & 0x001f0000))
/*       Fields WQ 5 */
#define SEC_ENTRIESWQ5_WIDTH                                               5
#define SEC_ENTRIESWQ5_SHIFT                                              8
#define SEC_ENTRIESWQ5_MASK                                       0x00001f00
#define SEC_ENTRIESWQ5_RD(src)                        (((src) & 0x00001f00)>>8)
#define SEC_ENTRIESWQ5_SET(dst,src) \
                          (((dst) & ~0x00001f00) | (((u32)(src)<<8) & 0x00001f00))
/*       Fields WQ 4 */
#define SEC_ENTRIESWQ4_WIDTH                                               5
#define SEC_ENTRIESWQ4_SHIFT                                              0
#define SEC_ENTRIESWQ4_MASK                                       0x0000001f
#define SEC_ENTRIESWQ4_RD(src)                        (((src) & 0x0000001f))
#define SEC_ENTRIESWQ4_SET(dst,src) \
                          (((dst) & ~0x0000001f) | (((u32)(src)) & 0x0000001f))

/*      Register StsSsQmiWQNumEntries2  */
/*       Fields WQ 11 */
#define SEC_ENTRIESWQ11_WIDTH                                               5
#define SEC_ENTRIESWQ11_SHIFT                                              24
#define SEC_ENTRIESWQ11_MASK                                       0x1f000000
#define SEC_ENTRIESWQ11_RD(src)                        (((src) & 0x1f000000)>>24)
#define SEC_ENTRIESWQ11_SET(dst,src) \
                          (((dst) & ~0x1f000000) | (((u32)(src)<<24) & 0x1f000000))
/*       Fields WQ 10 */
#define SEC_ENTRIESWQ10_WIDTH                                               5
#define SEC_ENTRIESWQ10_SHIFT                                              16
#define SEC_ENTRIESWQ10_MASK                                       0x001f0000
#define SEC_ENTRIESWQ10_RD(src)                        (((src) & 0x001f0000)>>16)
#define SEC_ENTRIESWQ10_SET(dst,src) \
                          (((dst) & ~0x001f0000) | (((u32)(src)<<16) & 0x001f0000))
/*       Fields WQ 9 */
#define SEC_ENTRIESWQ9_WIDTH                                               5
#define SEC_ENTRIESWQ9_SHIFT                                              8
#define SEC_ENTRIESWQ9_MASK                                       0x00001f00
#define SEC_ENTRIESWQ9_RD(src)                        (((src) & 0x00001f00)>>8)
#define SEC_ENTRIESWQ9_SET(dst,src) \
                          (((dst) & ~0x00001f00) | (((u32)(src)<<8) & 0x00001f00))
/*       Fields WQ 8 */
#define SEC_ENTRIESWQ8_WIDTH                                               5
#define SEC_ENTRIESWQ8_SHIFT                                              0
#define SEC_ENTRIESWQ8_MASK                                       0x0000001f
#define SEC_ENTRIESWQ8_RD(src)                        (((src) & 0x0000001f))
#define SEC_ENTRIESWQ8_SET(dst,src) \
                          (((dst) & ~0x0000001f) | (((u32)(src)) & 0x0000001f))

/*      Register StsSsQmiWQNumEntries3  */
/*       Fields WQ 15 */
#define SEC_ENTRIESWQ15_WIDTH                                               5
#define SEC_ENTRIESWQ15_SHIFT                                              24
#define SEC_ENTRIESWQ15_MASK                                       0x1f000000
#define SEC_ENTRIESWQ15_RD(src)                        (((src) & 0x1f000000)>>24)
#define SEC_ENTRIESWQ15_SET(dst,src) \
                          (((dst) & ~0x1f000000) | (((u32)(src)<<24) & 0x1f000000))
/*       Fields WQ 14 */
#define SEC_ENTRIESWQ14_WIDTH                                               5
#define SEC_ENTRIESWQ14_SHIFT                                              16
#define SEC_ENTRIESWQ14_MASK                                       0x001f0000
#define SEC_ENTRIESWQ14_RD(src)                        (((src) & 0x001f0000)>>16)
#define SEC_ENTRIESWQ14_SET(dst,src) \
                          (((dst) & ~0x001f0000) | (((u32)(src)<<16) & 0x001f0000))
/*       Fields WQ 13 */
#define SEC_ENTRIESWQ13_WIDTH                                               5
#define SEC_ENTRIESWQ13_SHIFT                                              8
#define SEC_ENTRIESWQ13_MASK                                       0x00001f00
#define SEC_ENTRIESWQ13_RD(src)                        (((src) & 0x00001f00)>>8)
#define SEC_ENTRIESWQ13_SET(dst,src) \
                          (((dst) & ~0x00001f00) | (((u32)(src)<<8) & 0x00001f00))
/*       Fields WQ 12 */
#define SEC_ENTRIESWQ12_WIDTH                                               5
#define SEC_ENTRIESWQ12_SHIFT                                              0
#define SEC_ENTRIESWQ12_MASK                                       0x0000001f
#define SEC_ENTRIESWQ12_RD(src)                        (((src) & 0x0000001f))
#define SEC_ENTRIESWQ12_SET(dst,src) \
                          (((dst) & ~0x0000001f) | (((u32)(src)) & 0x0000001f))

/*      Register StsSsQmiWQNumEntries4  */
/*       Fields WQ 19 */
#define SEC_ENTRIESWQ19_WIDTH                                               5
#define SEC_ENTRIESWQ19_SHIFT                                              24
#define SEC_ENTRIESWQ19_MASK                                       0x1f000000
#define SEC_ENTRIESWQ19_RD(src)                        (((src) & 0x1f000000)>>24)
#define SEC_ENTRIESWQ19_SET(dst,src) \
                          (((dst) & ~0x1f000000) | (((u32)(src)<<24) & 0x1f000000))
/*       Fields WQ 18 */
#define SEC_ENTRIESWQ18_WIDTH                                               5
#define SEC_ENTRIESWQ18_SHIFT                                              16
#define SEC_ENTRIESWQ18_MASK                                       0x001f0000
#define SEC_ENTRIESWQ18_RD(src)                        (((src) & 0x001f0000)>>16)
#define SEC_ENTRIESWQ18_SET(dst,src) \
                          (((dst) & ~0x001f0000) | (((u32)(src)<<16) & 0x001f0000))
/*       Fields WQ 17 */
#define SEC_ENTRIESWQ17_WIDTH                                               5
#define SEC_ENTRIESWQ17_SHIFT                                              8
#define SEC_ENTRIESWQ17_MASK                                       0x00001f00
#define SEC_ENTRIESWQ17_RD(src)                        (((src) & 0x00001f00)>>8)
#define SEC_ENTRIESWQ17_SET(dst,src) \
                          (((dst) & ~0x00001f00) | (((u32)(src)<<8) & 0x00001f00))
/*       Fields WQ 16 */
#define SEC_ENTRIESWQ16_WIDTH                                               5
#define SEC_ENTRIESWQ16_SHIFT                                              0
#define SEC_ENTRIESWQ16_MASK                                       0x0000001f
#define SEC_ENTRIESWQ16_RD(src)                        (((src) & 0x0000001f))
#define SEC_ENTRIESWQ16_SET(dst,src) \
                          (((dst) & ~0x0000001f) | (((u32)(src)) & 0x0000001f))

/*      Register StsSsQmiWQNumEntries5  */
/*       Fields WQ 23 */
#define SEC_ENTRIESWQ23_WIDTH                                               5
#define SEC_ENTRIESWQ23_SHIFT                                              24
#define SEC_ENTRIESWQ23_MASK                                       0x1f000000
#define SEC_ENTRIESWQ23_RD(src)                        (((src) & 0x1f000000)>>24)
#define SEC_ENTRIESWQ23_SET(dst,src) \
                          (((dst) & ~0x1f000000) | (((u32)(src)<<24) & 0x1f000000))
/*       Fields WQ 22 */
#define SEC_ENTRIESWQ22_WIDTH                                               5
#define SEC_ENTRIESWQ22_SHIFT                                              16
#define SEC_ENTRIESWQ22_MASK                                       0x001f0000
#define SEC_ENTRIESWQ22_RD(src)                        (((src) & 0x001f0000)>>16)
#define SEC_ENTRIESWQ22_SET(dst,src) \
                          (((dst) & ~0x001f0000) | (((u32)(src)<<16) & 0x001f0000))
/*       Fields WQ 21 */
#define SEC_ENTRIESWQ21_WIDTH                                               5
#define SEC_ENTRIESWQ21_SHIFT                                              8
#define SEC_ENTRIESWQ21_MASK                                       0x00001f00
#define SEC_ENTRIESWQ21_RD(src)                        (((src) & 0x00001f00)>>8)
#define SEC_ENTRIESWQ21_SET(dst,src) \
                          (((dst) & ~0x00001f00) | (((u32)(src)<<8) & 0x00001f00))
/*       Fields WQ 20 */
#define SEC_ENTRIESWQ20_WIDTH                                               5
#define SEC_ENTRIESWQ20_SHIFT                                              0
#define SEC_ENTRIESWQ20_MASK                                       0x0000001f
#define SEC_ENTRIESWQ20_RD(src)                        (((src) & 0x0000001f))
#define SEC_ENTRIESWQ20_SET(dst,src) \
                          (((dst) & ~0x0000001f) | (((u32)(src)) & 0x0000001f))

/*      Register StsSsQmiWQNumEntries6  */
/*       Fields WQ 27 */
#define SEC_ENTRIESWQ27_WIDTH                                               5
#define SEC_ENTRIESWQ27_SHIFT                                              24
#define SEC_ENTRIESWQ27_MASK                                       0x1f000000
#define SEC_ENTRIESWQ27_RD(src)                        (((src) & 0x1f000000)>>24)
#define SEC_ENTRIESWQ27_SET(dst,src) \
                          (((dst) & ~0x1f000000) | (((u32)(src)<<24) & 0x1f000000))
/*       Fields WQ 26 */
#define SEC_ENTRIESWQ26_WIDTH                                               5
#define SEC_ENTRIESWQ26_SHIFT                                              16
#define SEC_ENTRIESWQ26_MASK                                       0x001f0000
#define SEC_ENTRIESWQ26_RD(src)                        (((src) & 0x001f0000)>>16)
#define SEC_ENTRIESWQ26_SET(dst,src) \
                          (((dst) & ~0x001f0000) | (((u32)(src)<<16) & 0x001f0000))
/*       Fields WQ 25 */
#define SEC_ENTRIESWQ25_WIDTH                                               5
#define SEC_ENTRIESWQ25_SHIFT                                              8
#define SEC_ENTRIESWQ25_MASK                                       0x00001f00
#define SEC_ENTRIESWQ25_RD(src)                        (((src) & 0x00001f00)>>8)
#define SEC_ENTRIESWQ25_SET(dst,src) \
                          (((dst) & ~0x00001f00) | (((u32)(src)<<8) & 0x00001f00))
/*       Fields WQ 24 */
#define SEC_ENTRIESWQ24_WIDTH                                               5
#define SEC_ENTRIESWQ24_SHIFT                                              0
#define SEC_ENTRIESWQ24_MASK                                       0x0000001f
#define SEC_ENTRIESWQ24_RD(src)                        (((src) & 0x0000001f))
#define SEC_ENTRIESWQ24_SET(dst,src) \
                          (((dst) & ~0x0000001f) | (((u32)(src)) & 0x0000001f))

/*      Register StsSsQmiWQNumEntries7  */
/*       Fields WQ 31 */
#define SEC_ENTRIESWQ31_WIDTH                                               5
#define SEC_ENTRIESWQ31_SHIFT                                              24
#define SEC_ENTRIESWQ31_MASK                                       0x1f000000
#define SEC_ENTRIESWQ31_RD(src)                        (((src) & 0x1f000000)>>24)
#define SEC_ENTRIESWQ31_SET(dst,src) \
                          (((dst) & ~0x1f000000) | (((u32)(src)<<24) & 0x1f000000))
/*       Fields WQ 30 */
#define SEC_ENTRIESWQ30_WIDTH                                               5
#define SEC_ENTRIESWQ30_SHIFT                                              16
#define SEC_ENTRIESWQ30_MASK                                       0x001f0000
#define SEC_ENTRIESWQ30_RD(src)                        (((src) & 0x001f0000)>>16)
#define SEC_ENTRIESWQ30_SET(dst,src) \
                          (((dst) & ~0x001f0000) | (((u32)(src)<<16) & 0x001f0000))
/*       Fields WQ 29 */
#define SEC_ENTRIESWQ29_WIDTH                                               5
#define SEC_ENTRIESWQ29_SHIFT                                              8
#define SEC_ENTRIESWQ29_MASK                                       0x00001f00
#define SEC_ENTRIESWQ29_RD(src)                        (((src) & 0x00001f00)>>8)
#define SEC_ENTRIESWQ29_SET(dst,src) \
                          (((dst) & ~0x00001f00) | (((u32)(src)<<8) & 0x00001f00))
/*       Fields WQ 28 */
#define SEC_ENTRIESWQ28_WIDTH                                               5
#define SEC_ENTRIESWQ28_SHIFT                                              0
#define SEC_ENTRIESWQ28_MASK                                       0x0000001f
#define SEC_ENTRIESWQ28_RD(src)                        (((src) & 0x0000001f))
#define SEC_ENTRIESWQ28_SET(dst,src) \
                          (((dst) & ~0x0000001f) | (((u32)(src)) & 0x0000001f))

/*	Register CfgSsQmiSab	*/
/*	 Fields EnableMonitoring	 */
#define SEC_ENABLEMONITORING_WIDTH                                           20
#define SEC_ENABLEMONITORING_SHIFT                                            0
#define SEC_ENABLEMONITORING_MASK                                    0x000fffff
#define SEC_ENABLEMONITORING_RD(src)                     (((src) & 0x000fffff))
#define SEC_ENABLEMONITORING_WR(src)                (((u32)(src)) & 0x000fffff)
#define SEC_ENABLEMONITORING_SET(dst,src) \
                          (((dst) & ~0x000fffff) | (((u32)(src)) & 0x000fffff))

/*	Register CfgSsQmiSab0	*/
/*	 Fields QID1	 */
#define SEC_QID1_WIDTH                                                       12
#define SEC_QID1_SHIFT                                                       16
#define SEC_QID1_MASK                                                0x0fff0000
#define SEC_QID1_RD(src)                                 (((src) & 0x0fff0000)>>16)
#define SEC_QID1_WR(src)                            (((u32)(src)<<16) & 0x0fff0000)
#define SEC_QID1_SET(dst,src) \
                          (((dst) & ~0x0fff0000) | (((u32)(src)<<16) & 0x0fff0000))
/*	 Fields QID0	 */
#define SEC_QID0_WIDTH                                                       12
#define SEC_QID0_SHIFT                                                        0
#define SEC_QID0_MASK                                                0x00000fff
#define SEC_QID0_RD(src)                                 (((src) & 0x00000fff))
#define SEC_QID0_WR(src)                            (((u32)(src)) & 0x00000fff)
#define SEC_QID0_SET(dst,src) \
                          (((dst) & ~0x00000fff) | (((u32)(src)) & 0x00000fff))

/*      Register CfgSsQmiSab1   */
/*       Fields QID3     */
#define SEC_QID3_WIDTH                                                       12
#define SEC_QID3_SHIFT                                                       16
#define SEC_QID3_MASK                                                0x0fff0000
#define SEC_QID3_RD(src)                                 (((src) & 0x0fff0000)>>16)
#define SEC_QID3_WR(src)                            (((u32)(src)<<16) & 0x0fff0000)
#define SEC_QID3_SET(dst,src) \
                          (((dst) & ~0x0fff0000) | (((u32)(src)<<16) & 0x0fff0000))
/*       Fields QID2     */
#define SEC_QID2_WIDTH                                                       12
#define SEC_QID2_SHIFT                                                        0
#define SEC_QID2_MASK                                                0x00000fff
#define SEC_QID2_RD(src)                                 (((src) & 0x00000fff))
#define SEC_QID2_WR(src)                            (((u32)(src)) & 0x00000fff)
#define SEC_QID2_SET(dst,src) \
                          (((dst) & ~0x00000fff) | (((u32)(src)) & 0x00000fff))

/*      Register CfgSsQmiSab2   */
/*       Fields QID5     */
#define SEC_QID5_WIDTH                                                       12
#define SEC_QID5_SHIFT                                                       16
#define SEC_QID5_MASK                                                0x0fff0000
#define SEC_QID5_RD(src)                                 (((src) & 0x0fff0000)>>16)
#define SEC_QID5_WR(src)                            (((u32)(src)<<16) & 0x0fff0000)
#define SEC_QID5_SET(dst,src) \
                          (((dst) & ~0x0fff0000) | (((u32)(src)<<16) & 0x0fff0000))
/*       Fields QID4     */
#define SEC_QID4_WIDTH                                                       12
#define SEC_QID4_SHIFT                                                        0
#define SEC_QID4_MASK                                                0x00000fff
#define SEC_QID4_RD(src)                                 (((src) & 0x00000fff))
#define SEC_QID4_WR(src)                            (((u32)(src)) & 0x00000fff)
#define SEC_QID4_SET(dst,src) \
                          (((dst) & ~0x00000fff) | (((u32)(src)) & 0x00000fff))

/*      Register CfgSsQmiSab3   */
/*       Fields QID7     */
#define SEC_QID7_WIDTH                                                       12
#define SEC_QID7_SHIFT                                                       16
#define SEC_QID7_MASK                                                0x0fff0000
#define SEC_QID7_RD(src)                                 (((src) & 0x0fff0000)>>16)
#define SEC_QID7_WR(src)                            (((u32)(src)<<16) & 0x0fff0000)
#define SEC_QID7_SET(dst,src) \
                          (((dst) & ~0x0fff0000) | (((u32)(src)<<16) & 0x0fff0000))
/*       Fields QID6     */
#define SEC_QID6_WIDTH                                                       12
#define SEC_QID6_SHIFT                                                        0
#define SEC_QID6_MASK                                                0x00000fff
#define SEC_QID6_RD(src)                                 (((src) & 0x00000fff))
#define SEC_QID6_WR(src)                            (((u32)(src)) & 0x00000fff)
#define SEC_QID6_SET(dst,src) \
                          (((dst) & ~0x00000fff) | (((u32)(src)) & 0x00000fff))

/*      Register CfgSsQmiSab4   */
/*       Fields QID9     */
#define SEC_QID9_WIDTH                                                       12
#define SEC_QID9_SHIFT                                                       16
#define SEC_QID9_MASK                                                0x0fff0000
#define SEC_QID9_RD(src)                                 (((src) & 0x0fff0000)>>16)
#define SEC_QID9_WR(src)                            (((u32)(src)<<16) & 0x0fff0000)
#define SEC_QID9_SET(dst,src) \
                          (((dst) & ~0x0fff0000) | (((u32)(src)<<16) & 0x0fff0000))
/*       Fields QID8     */
#define SEC_QID8_WIDTH                                                       12
#define SEC_QID8_SHIFT                                                        0
#define SEC_QID8_MASK                                                0x00000fff
#define SEC_QID8_RD(src)                                 (((src) & 0x00000fff))
#define SEC_QID8_WR(src)                            (((u32)(src)) & 0x00000fff)
#define SEC_QID8_SET(dst,src) \
                          (((dst) & ~0x00000fff) | (((u32)(src)) & 0x00000fff))

/*      Register CfgSsQmiSab5   */
/*       Fields QID11     */
#define SEC_QID11_WIDTH                                                       12
#define SEC_QID11_SHIFT                                                       16
#define SEC_QID11_MASK                                                0x0fff0000
#define SEC_QID11_RD(src)                                 (((src) & 0x0fff0000)>>16)
#define SEC_QID11_WR(src)                            (((u32)(src)<<16) & 0x0fff0000)
#define SEC_QID11_SET(dst,src) \
                          (((dst) & ~0x0fff0000) | (((u32)(src)<<16) & 0x0fff0000))
/*       Fields QID10     */
#define SEC_QID10_WIDTH                                                       12
#define SEC_QID10_SHIFT                                                        0
#define SEC_QID10_MASK                                                0x00000fff
#define SEC_QID10_RD(src)                                 (((src) & 0x00000fff))
#define SEC_QID10_WR(src)                            (((u32)(src)) & 0x00000fff)
#define SEC_QID10_SET(dst,src) \
                          (((dst) & ~0x00000fff) | (((u32)(src)) & 0x00000fff))

/*      Register CfgSsQmiSab6   */
/*       Fields QID13     */
#define SEC_QID13_WIDTH                                                       12
#define SEC_QID13_SHIFT                                                       16
#define SEC_QID13_MASK                                                0x0fff0000
#define SEC_QID13_RD(src)                                 (((src) & 0x0fff0000)>>16)
#define SEC_QID13_WR(src)                            (((u32)(src)<<16) & 0x0fff0000)
#define SEC_QID13_SET(dst,src) \
                          (((dst) & ~0x0fff0000) | (((u32)(src)<<16) & 0x0fff0000))
/*       Fields QID12     */
#define SEC_QID12_WIDTH                                                       12
#define SEC_QID12_SHIFT                                                        0
#define SEC_QID12_MASK                                                0x00000fff
#define SEC_QID12_RD(src)                                 (((src) & 0x00000fff))
#define SEC_QID12_WR(src)                            (((u32)(src)) & 0x00000fff)
#define SEC_QID12_SET(dst,src) \
                          (((dst) & ~0x00000fff) | (((u32)(src)) & 0x00000fff))

/*      Register CfgSsQmiSab7   */
/*       Fields QID15     */
#define SEC_QID15_WIDTH                                                       12
#define SEC_QID15_SHIFT                                                       16
#define SEC_QID15_MASK                                                0x0fff0000
#define SEC_QID15_RD(src)                                 (((src) & 0x0fff0000)>>16)
#define SEC_QID15_WR(src)                            (((u32)(src)<<16) & 0x0fff0000)
#define SEC_QID15_SET(dst,src) \
                          (((dst) & ~0x0fff0000) | (((u32)(src)<<16) & 0x0fff0000))
/*       Fields QID14     */
#define SEC_QID14_WIDTH                                                       12
#define SEC_QID14_SHIFT                                                        0
#define SEC_QID14_MASK                                                0x00000fff
#define SEC_QID14_RD(src)                                 (((src) & 0x00000fff))
#define SEC_QID14_WR(src)                            (((u32)(src)) & 0x00000fff)
#define SEC_QID14_SET(dst,src) \
                          (((dst) & ~0x00000fff) | (((u32)(src)) & 0x00000fff))

/*      Register CfgSsQmiSab8   */
/*       Fields QID17     */
#define SEC_QID17_WIDTH                                                       12
#define SEC_QID17_SHIFT                                                       16
#define SEC_QID17_MASK                                                0x0fff0000
#define SEC_QID17_RD(src)                                 (((src) & 0x0fff0000)>>16)
#define SEC_QID17_WR(src)                            (((u32)(src)<<16) & 0x0fff0000)
#define SEC_QID17_SET(dst,src) \
                          (((dst) & ~0x0fff0000) | (((u32)(src)<<16) & 0x0fff0000))
/*       Fields QID16     */
#define SEC_QID16_WIDTH                                                       12
#define SEC_QID16_SHIFT                                                        0
#define SEC_QID16_MASK                                                0x00000fff
#define SEC_QID16_RD(src)                                 (((src) & 0x00000fff))
#define SEC_QID16_WR(src)                            (((u32)(src)) & 0x00000fff)
#define SEC_QID16_SET(dst,src) \
                          (((dst) & ~0x00000fff) | (((u32)(src)) & 0x00000fff))

/*      Register CfgSsQmiSab9   */
/*       Fields QID19     */
#define SEC_QID19_WIDTH                                                       12
#define SEC_QID19_SHIFT                                                       16
#define SEC_QID19_MASK                                                0x0fff0000
#define SEC_QID19_RD(src)                                 (((src) & 0x0fff0000)>>16)
#define SEC_QID19_WR(src)                            (((u32)(src)<<16) & 0x0fff0000)
#define SEC_QID19_SET(dst,src) \
                          (((dst) & ~0x0fff0000) | (((u32)(src)<<16) & 0x0fff0000))
/*       Fields QID18     */
#define SEC_QID18_WIDTH                                                       12
#define SEC_QID18_SHIFT                                                        0
#define SEC_QID18_MASK                                                0x00000fff
#define SEC_QID18_RD(src)                                 (((src) & 0x00000fff))
#define SEC_QID18_WR(src)                            (((u32)(src)) & 0x00000fff)
#define SEC_QID18_SET(dst,src) \
                          (((dst) & ~0x00000fff) | (((u32)(src)) & 0x00000fff))

/*	Register StsSsQmiInt0	*/
/*	 Fields FPOverflow	 */
#define SEC_FPOVERFLOW0_WIDTH                                                32
#define SEC_FPOVERFLOW0_SHIFT                                                 0
#define SEC_FPOVERFLOW0_MASK                                         0xffffffff
#define SEC_FPOVERFLOW0_RD(src)                          (((src) & 0xffffffff))
#define SEC_FPOVERFLOW0_WR(src)                     (((u32)(src)) & 0xffffffff)
#define SEC_FPOVERFLOW0_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register StsSsQmiInt0Mask	*/
/*    Mask Register Fields FPOverflowMask    */
#define SEC_FPOVERFLOWMASK_WIDTH                                             32
#define SEC_FPOVERFLOWMASK_SHIFT                                              0
#define SEC_FPOVERFLOWMASK_MASK                                      0xffffffff
#define SEC_FPOVERFLOWMASK_RD(src)                       (((src) & 0xffffffff))
#define SEC_FPOVERFLOWMASK_WR(src)                  (((u32)(src)) & 0xffffffff)
#define SEC_FPOVERFLOWMASK_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register StsSsQmiInt1	*/
/*	 Fields WQOverflow	 */
#define SEC_WQOVERFLOW1_WIDTH                                                32
#define SEC_WQOVERFLOW1_SHIFT                                                 0
#define SEC_WQOVERFLOW1_MASK                                         0xffffffff
#define SEC_WQOVERFLOW1_RD(src)                          (((src) & 0xffffffff))
#define SEC_WQOVERFLOW1_WR(src)                     (((u32)(src)) & 0xffffffff)
#define SEC_WQOVERFLOW1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register StsSsQmiInt1Mask	*/
/*    Mask Register Fields WQOverflowMask    */
#define SEC_WQOVERFLOWMASK_WIDTH                                             32
#define SEC_WQOVERFLOWMASK_SHIFT                                              0
#define SEC_WQOVERFLOWMASK_MASK                                      0xffffffff
#define SEC_WQOVERFLOWMASK_RD(src)                       (((src) & 0xffffffff))
#define SEC_WQOVERFLOWMASK_WR(src)                  (((u32)(src)) & 0xffffffff)
#define SEC_WQOVERFLOWMASK_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register StsSsQmiInt2	*/
/*	 Fields FPUnderRun	 */
#define SEC_FPUNDERRUN2_WIDTH                                                32
#define SEC_FPUNDERRUN2_SHIFT                                                 0
#define SEC_FPUNDERRUN2_MASK                                         0xffffffff
#define SEC_FPUNDERRUN2_RD(src)                          (((src) & 0xffffffff))
#define SEC_FPUNDERRUN2_WR(src)                     (((u32)(src)) & 0xffffffff)
#define SEC_FPUNDERRUN2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register StsSsQmiInt2Mask	*/
/*    Mask Register Fields FPUnderRunMask    */
#define SEC_FPUNDERRUNMASK_WIDTH                                             32
#define SEC_FPUNDERRUNMASK_SHIFT                                              0
#define SEC_FPUNDERRUNMASK_MASK                                      0xffffffff
#define SEC_FPUNDERRUNMASK_RD(src)                       (((src) & 0xffffffff))
#define SEC_FPUNDERRUNMASK_WR(src)                  (((u32)(src)) & 0xffffffff)
#define SEC_FPUNDERRUNMASK_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register StsSsQmiInt3	*/
/*	 Fields WQUnderRun	 */
#define SEC_WQUNDERRUN3_WIDTH                                                32
#define SEC_WQUNDERRUN3_SHIFT                                                 0
#define SEC_WQUNDERRUN3_MASK                                         0xffffffff
#define SEC_WQUNDERRUN3_RD(src)                          (((src) & 0xffffffff))
#define SEC_WQUNDERRUN3_WR(src)                     (((u32)(src)) & 0xffffffff)
#define SEC_WQUNDERRUN3_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register StsSsQmiInt3Mask	*/
/*    Mask Register Fields WQUnderRunMask    */
#define SEC_WQUNDERRUNMASK_WIDTH                                             32
#define SEC_WQUNDERRUNMASK_SHIFT                                              0
#define SEC_WQUNDERRUNMASK_MASK                                      0xffffffff
#define SEC_WQUNDERRUNMASK_RD(src)                       (((src) & 0xffffffff))
#define SEC_WQUNDERRUNMASK_WR(src)                  (((u32)(src)) & 0xffffffff)
#define SEC_WQUNDERRUNMASK_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register StsSsQmiInt4	*/
/*	 Fields axiwcmr_slverr	 */
#define SEC_AXIWCMR_SLVERR4_WIDTH                                             1
#define SEC_AXIWCMR_SLVERR4_SHIFT                                             1
#define SEC_AXIWCMR_SLVERR4_MASK                                     0x00000002
#define SEC_AXIWCMR_SLVERR4_RD(src)                   (((src) & 0x00000002)>>1)
#define SEC_AXIWCMR_SLVERR4_WR(src)              (((u32)(src)<<1) & 0x00000002)
#define SEC_AXIWCMR_SLVERR4_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields axiwcmr_decerr	 */
#define SEC_AXIWCMR_DECERR4_WIDTH                                             1
#define SEC_AXIWCMR_DECERR4_SHIFT                                             0
#define SEC_AXIWCMR_DECERR4_MASK                                     0x00000001
#define SEC_AXIWCMR_DECERR4_RD(src)                      (((src) & 0x00000001))
#define SEC_AXIWCMR_DECERR4_WR(src)                 (((u32)(src)) & 0x00000001)
#define SEC_AXIWCMR_DECERR4_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register StsSsQmiInt4Mask	*/
/*    Mask Register Fields axiwcmr_slverrMask    */
#define SEC_AXIWCMR_SLVERRMASK_WIDTH                                          1
#define SEC_AXIWCMR_SLVERRMASK_SHIFT                                          1
#define SEC_AXIWCMR_SLVERRMASK_MASK                                  0x00000002
#define SEC_AXIWCMR_SLVERRMASK_RD(src)                (((src) & 0x00000002)>>1)
#define SEC_AXIWCMR_SLVERRMASK_WR(src)           (((u32)(src)<<1) & 0x00000002)
#define SEC_AXIWCMR_SLVERRMASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*    Mask Register Fields axiwcmr_decerrMask    */
#define SEC_AXIWCMR_DECERRMASK_WIDTH                                          1
#define SEC_AXIWCMR_DECERRMASK_SHIFT                                          0
#define SEC_AXIWCMR_DECERRMASK_MASK                                  0x00000001
#define SEC_AXIWCMR_DECERRMASK_RD(src)                   (((src) & 0x00000001))
#define SEC_AXIWCMR_DECERRMASK_WR(src)              (((u32)(src)) & 0x00000001)
#define SEC_AXIWCMR_DECERRMASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register CfgSsQmiDbgCtrl	*/
/*	 Fields PBID	 */
#define SEC_PBID_WIDTH                                                        6
#define SEC_PBID_SHIFT                                                       12
#define SEC_PBID_MASK                                                0x0003f000
#define SEC_PBID_RD(src)                             (((src) & 0x0003f000)>>12)
#define SEC_PBID_WR(src)                        (((u32)(src)<<12) & 0x0003f000)
#define SEC_PBID_SET(dst,src) \
                      (((dst) & ~0x0003f000) | (((u32)(src)<<12) & 0x0003f000))
/*	 Fields nAck	 */
#define SEC_NACK_WIDTH                                                        1
#define SEC_NACK_SHIFT                                                       11
#define SEC_NACK_MASK                                                0x00000800
#define SEC_NACK_RD(src)                             (((src) & 0x00000800)>>11)
#define SEC_NACK_WR(src)                        (((u32)(src)<<11) & 0x00000800)
#define SEC_NACK_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields last	 */
#define SEC_LAST_WIDTH                                                        1
#define SEC_LAST_SHIFT                                                        10
#define SEC_LAST_MASK                                                0x00000400
#define SEC_LAST_RD(src)                              (((src) & 0x00000400)>>10)
#define SEC_LAST_WR(src)                         (((u32)(src)<<10) & 0x00000400)
#define SEC_LAST_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields Push	 */
#define SEC_PUSH_WIDTH                                                        1
#define SEC_PUSH_SHIFT                                                        9
#define SEC_PUSH_MASK                                                0x00000200
#define SEC_PUSH_RD(src)                              (((src) & 0x00000200)>>9)
#define SEC_PUSH_WR(src)                         (((u32)(src)<<9) & 0x00000200)
#define SEC_PUSH_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields Write	 */
#define SEC_WRITE_WIDTH                                                       1
#define SEC_WRITE_SHIFT                                                       8
#define SEC_WRITE_MASK                                               0x00000100
#define SEC_WRITE_RD(src)                             (((src) & 0x00000100)>>8)
#define SEC_WRITE_WR(src)                        (((u32)(src)<<8) & 0x00000100)
#define SEC_WRITE_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields Pop	 */
#define SEC_POP_WIDTH                                                         1
#define SEC_POP_SHIFT                                                         7
#define SEC_POP_MASK                                                 0x00000080
#define SEC_POP_RD(src)                               (((src) & 0x00000080)>>7)
#define SEC_POP_WR(src)                          (((u32)(src)<<7) & 0x00000080)
#define SEC_POP_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields Read	 */
#define SEC_READ_WIDTH                                                        1
#define SEC_READ_SHIFT                                                        6
#define SEC_READ_MASK                                                0x00000040
#define SEC_READ_RD(src)                              (((src) & 0x00000040)>>6)
#define SEC_READ_WR(src)                         (((u32)(src)<<6) & 0x00000040)
#define SEC_READ_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields BufferAddr	 */
#define SEC_BUFFERADDR_WIDTH                                                  6
#define SEC_BUFFERADDR_SHIFT                                                  0
#define SEC_BUFFERADDR_MASK                                          0x0000003f
#define SEC_BUFFERADDR_RD(src)                           (((src) & 0x0000003f))
#define SEC_BUFFERADDR_WR(src)                      (((u32)(src)) & 0x0000003f)
#define SEC_BUFFERADDR_SET(dst,src) \
                          (((dst) & ~0x0000003f) | (((u32)(src)) & 0x0000003f))

/*	Register CfgSsQmiDbgData0	*/
/*	 Fields Data	 */
#define SEC_DATA0_WIDTH                                                      32
#define SEC_DATA0_SHIFT                                                       0
#define SEC_DATA0_MASK                                               0xffffffff
#define SEC_DATA0_RD(src)                                (((src) & 0xffffffff))
#define SEC_DATA0_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_DATA0_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CfgSsQmiDbgData1	*/
/*	 Fields Data	 */
#define SEC_DATA1_WIDTH                                                      32
#define SEC_DATA1_SHIFT                                                       0
#define SEC_DATA1_MASK                                               0xffffffff
#define SEC_DATA1_RD(src)                                (((src) & 0xffffffff))
#define SEC_DATA1_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_DATA1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CfgSsQmiDbgData2	*/
/*	 Fields Data	 */
#define SEC_DATA2_WIDTH                                                      32
#define SEC_DATA2_SHIFT                                                       0
#define SEC_DATA2_MASK                                               0xffffffff
#define SEC_DATA2_RD(src)                                (((src) & 0xffffffff))
#define SEC_DATA2_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_DATA2_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CfgSsQmiDbgData3	*/
/*	 Fields Data	 */
#define SEC_DATA3_WIDTH                                                      32
#define SEC_DATA3_SHIFT                                                       0
#define SEC_DATA3_MASK                                               0xffffffff
#define SEC_DATA3_RD(src)                                (((src) & 0xffffffff))
#define SEC_DATA3_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_DATA3_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register StsSsQmiDbgData	*/
/*	 Fields Data	 */
#define SEC_DATA_WIDTH                                                       32
#define SEC_DATA_SHIFT                                                        0
#define SEC_DATA_MASK                                                0xffffffff
#define SEC_DATA_RD(src)                                 (((src) & 0xffffffff))
#define SEC_DATA_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CfgSsQmiFPQAssoc	*/
/*	 Fields Association	 */
#define SEC_FPQASSOCIATION_WIDTH                                                       32
#define SEC_FPQASSOCIATION_SHIFT                                                        0
#define SEC_FPQASSOCIATION_MASK                                                0xffffffff
#define SEC_FPQASSOCIATION_RD(src)                                 (((src) & 0xffffffff))
#define SEC_FPQASSOCIATION_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CfgSsQmiWQAssoc	*/
/*	 Fields Association	 */
#define SEC_WQASSOCIATION_WIDTH                                                       32
#define SEC_WQASSOCIATION_SHIFT                                                        0
#define SEC_WQASSOCIATION_MASK                                                0xffffffff
#define SEC_WQASSOCIATION_RD(src)                                 (((src) & 0xffffffff))
#define SEC_WQASSOCIATION_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CfgSsQmiLite	*/
/*	 Fields QMLiteDeviceAddress	 */
#define SEC_QMLITEDEVICEADDRESS_WIDTH                                        20
#define SEC_QMLITEDEVICEADDRESS_SHIFT                                         0
#define SEC_QMLITEDEVICEADDRESS_MASK                                 0x000fffff
#define SEC_QMLITEDEVICEADDRESS_RD(src)                  (((src) & 0x000fffff))
#define SEC_QMLITEDEVICEADDRESS_WR(src)             (((u32)(src)) & 0x000fffff)
#define SEC_QMLITEDEVICEADDRESS_SET(dst,src) \
                          (((dst) & ~0x000fffff) | (((u32)(src)) & 0x000fffff))

/*	Register CfgSsQmiMemory	*/
/*	 Fields RMA	 */
#define SEC_RMA_WIDTH                                                         2
#define SEC_RMA_SHIFT                                                         4
#define SEC_RMA_MASK                                                 0x00000030
#define SEC_RMA_RD(src)                               (((src) & 0x00000030)>>4)
#define SEC_RMA_WR(src)                          (((u32)(src)<<4) & 0x00000030)
#define SEC_RMA_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*	 Fields RMB	 */
#define SEC_RMB_WIDTH                                                         2
#define SEC_RMB_SHIFT                                                         2
#define SEC_RMB_MASK                                                 0x0000000c
#define SEC_RMB_RD(src)                               (((src) & 0x0000000c)>>2)
#define SEC_RMB_WR(src)                          (((u32)(src)<<2) & 0x0000000c)
#define SEC_RMB_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields RMEA	 */
#define SEC_RMEA_WIDTH                                                        1
#define SEC_RMEA_SHIFT                                                        1
#define SEC_RMEA_MASK                                                0x00000002
#define SEC_RMEA_RD(src)                              (((src) & 0x00000002)>>1)
#define SEC_RMEA_WR(src)                         (((u32)(src)<<1) & 0x00000002)
#define SEC_RMEA_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields RMEB	 */
#define SEC_RMEB_WIDTH                                                        1
#define SEC_RMEB_SHIFT                                                        0
#define SEC_RMEB_MASK                                                0x00000001
#define SEC_RMEB_RD(src)                                 (((src) & 0x00000001))
#define SEC_RMEB_WR(src)                            (((u32)(src)) & 0x00000001)
#define SEC_RMEB_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register StsSsQmiFifo	*/
/*	 Fields empty	 */
#define SEC_EMPTY_WIDTH                                                      27
#define SEC_EMPTY_SHIFT                                                       0
#define SEC_EMPTY_MASK                                               0x07ffffff
#define SEC_EMPTY_RD(src)                                (((src) & 0x07ffffff))
#define SEC_EMPTY_SET(dst,src) \
                          (((dst) & ~0x07ffffff) | (((u32)(src)) & 0x07ffffff))

/*      Register CfgSsQmiQMLiteFPQAssoc        */
/*       Fields Association     */
#define SEC_QMLITEFPQASSOC_WIDTH                                                       32
#define SEC_QMLITEFPQASSOC_SHIFT                                                        0
#define SEC_QMLITEFPQASSOC_MASK                                                0xffffffff
#define SEC_QMLITEFPQASSOC_RD(src)                                 (((src) & 0xffffffff))
#define SEC_QMLITEFPQASSOC_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CfgSsQmiQMLiteWQAssoc	*/
/*	 Fields Association	 */
#define SEC_QMLITEWQASSOC_WIDTH                                                       32
#define SEC_QMLITEWQASSOC_SHIFT                                                        0
#define SEC_QMLITEWQASSOC_MASK                                                0xffffffff
#define SEC_QMLITEWQASSOC_RD(src)                                 (((src) & 0xffffffff))
#define SEC_QMLITEWQASSOC_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CfgSsQmiQMHold	*/
/*	 Fields QMLite_hold_en	 */
#define SEC_QMLITEHOLDEN_WIDTH                                                         1
#define SEC_QMLITEHOLDEN_SHIFT                                                         31
#define SEC_QMLITEHOLDEN_MASK                                                 0x80000000
#define SEC_QMLITEHOLDEN_RD(src)                               (((src) & 0x80000000)>>31)
#define SEC_QMLITEHOLDEN_WR(src)                          (((u32)(src)<<31) & 0x80000000)
#define SEC_QMLITEHOLDEN_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields QM2_hold_en	 */
#define SEC_QM2HOLDEN_WIDTH                                                         1
#define SEC_QM2HOLDEN_SHIFT                                                         2
#define SEC_QM2HOLDEN_MASK                                                 0x00000004
#define SEC_QM2HOLDEN_RD(src)                               (((src) & 0x00000004)>>2)
#define SEC_QM2HOLDEN_WR(src)                          (((u32)(src)<<2) & 0x00000004)
#define SEC_QM2HOLDEN_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields QM1HOLDEN	 */
#define SEC_QM1HOLDEN_WIDTH                                                        1
#define SEC_QM1HOLDEN_SHIFT                                                        1
#define SEC_QM1HOLDEN_MASK                                                0x00000002
#define SEC_QM1HOLDEN_RD(src)                              (((src) & 0x00000002)>>1)
#define SEC_QM1HOLDEN_WR(src)                         (((u32)(src)<<1) & 0x00000002)
#define SEC_QM1HOLDEN_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields QM0HOLDEN	 */
#define SEC_QM0HOLDEN_WIDTH                                                        1
#define SEC_QM0HOLDEN_SHIFT                                                        0
#define SEC_QM0HOLDEN_MASK                                                0x00000001
#define SEC_QM0HOLDEN_RD(src)                                 (((src) & 0x00000001))
#define SEC_QM0HOLDEN_WR(src)                            (((u32)(src)) & 0x00000001)
#define SEC_QM0HOLDEN_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register StsSsQmiQMHold	*/
/*	 Fields QMLite_hold_en	 */
#define SEC_STSQMLITEHOLDEN_WIDTH                                                         1
#define SEC_STSQMLITEHOLDEN_SHIFT                                                         31
#define SEC_STSQMLITEHOLDEN_MASK                                                 0x80000000
#define SEC_STSQMLITEHOLDEN_RD(src)                               (((src) & 0x80000000)>>31)
#define SEC_STSQMLITEHOLDEN_WR(src)                          (((u32)(src)<<31) & 0x80000000)
#define SEC_STSQMLITEHOLDEN_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields QM2_hold_en	 */
#define SEC_STSQM2HOLDEN_WIDTH                                                         1
#define SEC_STSQM2HOLDEN_SHIFT                                                         2
#define SEC_STSQM2HOLDEN_MASK                                                 0x00000004
#define SEC_STSQM2HOLDEN_RD(src)                               (((src) & 0x00000004)>>2)
#define SEC_STSQM2HOLDEN_WR(src)                          (((u32)(src)<<2) & 0x00000004)
#define SEC_STSQM2HOLDEN_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields QM1HOLDEN	 */
#define SEC_STSQM1HOLDEN_WIDTH                                                        1
#define SEC_STSQM1HOLDEN_SHIFT                                                        1
#define SEC_STSQM1HOLDEN_MASK                                                0x00000002
#define SEC_STSQM1HOLDEN_RD(src)                              (((src) & 0x00000002)>>1)
#define SEC_STSQM1HOLDEN_WR(src)                         (((u32)(src)<<1) & 0x00000002)
#define SEC_STSQM1HOLDEN_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields QM0HOLDEN	 */
#define SEC_STSQM0HOLDEN_WIDTH                                                        1
#define SEC_STSQM0HOLDEN_SHIFT                                                        0
#define SEC_STSQM0HOLDEN_MASK                                                0x00000001
#define SEC_STSQM0HOLDEN_RD(src)                                 (((src) & 0x00000001))
#define SEC_STSQM0HOLDEN_WR(src)                            (((u32)(src)) & 0x00000001)
#define SEC_STSQM0HOLDEN_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*      Register CfgSsQmiFPQVCAssoc0       */
/*       Fields Association     */
#define SEC_FPQVCASSOC0_WIDTH                                                      32
#define SEC_FPQVCASSOC0_SHIFT                                                       0
#define SEC_FPQVCASSOC0_MASK                                               0xffffffff
#define SEC_FPQVCASSOC0_RD(src)                                (((src) & 0xffffffff))
#define SEC_FPQVCASSOC0_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_FPQVCASSOC0_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register CfgSsQmiFPQVCAssoc1       */
/*       Fields Association     */
#define SEC_FPQVCASSOC1_WIDTH                                                      32
#define SEC_FPQVCASSOC1_SHIFT                                                       0
#define SEC_FPQVCASSOC1_MASK                                               0xffffffff
#define SEC_FPQVCASSOC1_RD(src)                                (((src) & 0xffffffff))
#define SEC_FPQVCASSOC1_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_FPQVCASSOC1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register CfgSsQmiWQVCAssoc0       */
/*       Fields Association     */
#define SEC_WQVCASSOC0_WIDTH                                                      32
#define SEC_WQVCASSOC0_SHIFT                                                       0
#define SEC_WQVCASSOC0_MASK                                               0xffffffff
#define SEC_WQVCASSOC0_RD(src)                                (((src) & 0xffffffff))
#define SEC_WQVCASSOC0_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_WQVCASSOC0_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register CfgSsQmiWQVCAssoc1       */
/*       Fields Association     */
#define SEC_WQVCASSOC1_WIDTH                                                      32
#define SEC_WQVCASSOC1_SHIFT                                                       0
#define SEC_WQVCASSOC1_MASK                                               0xffffffff
#define SEC_WQVCASSOC1_RD(src)                                (((src) & 0xffffffff))
#define SEC_WQVCASSOC1_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_WQVCASSOC1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register CfgSsQmiQM2       */
/*       Fields Address     */
#define SEC_QMIQM2ADDRESS_WIDTH                                                      20
#define SEC_QMIQM2ADDRESS_SHIFT                                                       0
#define SEC_QMIQM2ADDRESS_MASK                                               0x000fffff
#define SEC_QMIQM2ADDRESS_RD(src)                                (((src) & 0x000fffff))
#define SEC_QMIQM2ADDRESS_WR(src)                           (((u32)(src)) & 0x000fffff)
#define SEC_QMIQM2ADDRESS_SET(dst,src) \
                          (((dst) & ~0x000fffff) | (((u32)(src)) & 0x000fffff))

/*	Global Base Address	*/
#define SEC_EIP96_AXI_CSR_BASE_ADDR			0x1f252800
#define SEC_XTS_AXI_CSR_BASE_ADDR			0x1f251000
#define	SEC_EIP62_AXI_CSR_BASE_ADDR			0x1f254000

/*    Address SEC_XTS_AXI_CSR  Registers */
#define CSR_SEC_CFG_ADDR                                             0x00000000
#define CSR_SEC_CFG_DEFAULT                                          0x00000000
#define CSR_AXI_RD_MAX_BRST_CFG_ADDR                                 0x00000004
#define CSR_AXI_RD_MAX_BRST_CFG_DEFAULT                              0x00000000
#define CSR_AXI_WR_MAX_BRST_CFG_ADDR                                 0x00000008
#define CSR_AXI_WR_MAX_BRST_CFG_DEFAULT                              0x00000000
#define CSR_AXI_RD_MAX_OUTSTANDING_CFG_ADDR                          0x0000000c
#define CSR_AXI_RD_MAX_OUTSTANDING_CFG_DEFAULT                       0x11110000
#define CSR_AXI_WR_MAX_OUTSTANDING_CFG_ADDR                          0x00000010
#define CSR_AXI_WR_MAX_OUTSTANDING_CFG_DEFAULT                       0x11100000
#define CSR_SEC_INT_STS_ADDR                                         0x00000014
#define CSR_SEC_INT_STS_DEFAULT                                      0x00000000
#define CSR_SEC_INT_STSMASK_ADDR                                     0x00000018
#define CSR_SEC_INT_STSMASK_DEFAULT                                  0xffffffff
#define CSR_AXI_SEC_FIFO_STS_ADDR                                    0x0000001c
#define CSR_AXI_SEC_FIFO_STS_DEFAULT                                 0xffc00000
#define CSR_MAX_PKT_TIMEOUT_CTRL_ADDR                                0x00000020
#define CSR_MAX_PKT_TIMEOUT_CTRL_DEFAULT                             0x7fffffff
#define CSR_MAX_FPBAVL_TIMEOUT_CTRL_ADDR                             0x00000024
#define CSR_MAX_FPBAVL_TIMEOUT_CTRL_DEFAULT                          0x7fffffff
#define CSR_MISC_CTRL_ADDR                                           0x00000028
#define CSR_MISC_CTRL_DEFAULT                                        0x80000000

/*	Register csr_sec_cfg	*/
/*	 Fields go	 */
#define GO_WIDTH                                                              1
#define GO_SHIFT                                                             31
#define GO_MASK                                                      0x80000000
#define GO_RD(src)                                   (((src) & 0x80000000)>>31)
#define GO_WR(src)                              (((u32)(src)<<31) & 0x80000000)
#define GO_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))

/*	Register csr_axi_rd_max_brst_cfg	*/
/*	 Fields tkn_rd	 */
#define TKN_RD_WIDTH                                                          2
#define TKN_RD_SHIFT                                                         30
#define TKN_RD_MASK                                                  0xc0000000
#define TKN_RD_RD(src)                               (((src) & 0xc0000000)>>30)
#define TKN_RD_WR(src)                          (((u32)(src)<<30) & 0xc0000000)
#define TKN_RD_SET(dst,src) \
                      (((dst) & ~0xc0000000) | (((u32)(src)<<30) & 0xc0000000))
/*	 Fields ctx_rd	 */
#define CTX_RD_WIDTH                                                          2
#define CTX_RD_SHIFT                                                         28
#define CTX_RD_MASK                                                  0x30000000
#define CTX_RD_RD(src)                               (((src) & 0x30000000)>>28)
#define CTX_RD_WR(src)                          (((u32)(src)<<28) & 0x30000000)
#define CTX_RD_SET(dst,src) \
                      (((dst) & ~0x30000000) | (((u32)(src)<<28) & 0x30000000))
/*	 Fields data_rd	 */
#define DATA_RD_WIDTH                                                         2
#define DATA_RD_SHIFT                                                        26
#define DATA_RD_MASK                                                 0x0c000000
#define DATA_RD_RD(src)                              (((src) & 0x0c000000)>>26)
#define DATA_RD_WR(src)                         (((u32)(src)<<26) & 0x0c000000)
#define DATA_RD_SET(dst,src) \
                      (((dst) & ~0x0c000000) | (((u32)(src)<<26) & 0x0c000000))
/*	 Fields dstll_rd	 */
#define DSTLL_RD_WIDTH                                                        2
#define DSTLL_RD_SHIFT                                                       24
#define DSTLL_RD_MASK                                                0x03000000
#define DSTLL_RD_RD(src)                             (((src) & 0x03000000)>>24)
#define DSTLL_RD_WR(src)                        (((u32)(src)<<24) & 0x03000000)
#define DSTLL_RD_SET(dst,src) \
                      (((dst) & ~0x03000000) | (((u32)(src)<<24) & 0x03000000))

/*	Register csr_axi_wr_max_brst_cfg	*/
/*	 Fields tkn_wr	 */
#define TKN_WR_WIDTH                                                          2
#define TKN_WR_SHIFT                                                         30
#define TKN_WR_MASK                                                  0xc0000000
#define TKN_WR_RD(src)                               (((src) & 0xc0000000)>>30)
#define TKN_WR_WR(src)                          (((u32)(src)<<30) & 0xc0000000)
#define TKN_WR_SET(dst,src) \
                      (((dst) & ~0xc0000000) | (((u32)(src)<<30) & 0xc0000000))
/*	 Fields ctx_wr	 */
#define CTX_WR_WIDTH                                                          2
#define CTX_WR_SHIFT                                                         28
#define CTX_WR_MASK                                                  0x30000000
#define CTX_WR_RD(src)                               (((src) & 0x30000000)>>28)
#define CTX_WR_WR(src)                          (((u32)(src)<<28) & 0x30000000)
#define CTX_WR_SET(dst,src) \
                      (((dst) & ~0x30000000) | (((u32)(src)<<28) & 0x30000000))
/*	 Fields data_wr	 */
#define DATA_WR_WIDTH                                                         2
#define DATA_WR_SHIFT                                                        26
#define DATA_WR_MASK                                                 0x0c000000
#define DATA_WR_RD(src)                              (((src) & 0x0c000000)>>26)
#define DATA_WR_WR(src)                         (((u32)(src)<<26) & 0x0c000000)
#define DATA_WR_SET(dst,src) \
                      (((dst) & ~0x0c000000) | (((u32)(src)<<26) & 0x0c000000))

/*	Register csr_axi_rd_max_outstanding_cfg	*/
/*	 Fields tkn_rd	 */
#define TKN_RD_F1_WIDTH                                                       4
#define TKN_RD_F1_SHIFT                                                      28
#define TKN_RD_F1_MASK                                               0xf0000000
#define TKN_RD_F1_RD(src)                            (((src) & 0xf0000000)>>28)
#define TKN_RD_F1_WR(src)                       (((u32)(src)<<28) & 0xf0000000)
#define TKN_RD_F1_SET(dst,src) \
                      (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields ctx_rd	 */
#define CTX_RD_F1_WIDTH                                                       4
#define CTX_RD_F1_SHIFT                                                      24
#define CTX_RD_F1_MASK                                               0x0f000000
#define CTX_RD_F1_RD(src)                            (((src) & 0x0f000000)>>24)
#define CTX_RD_F1_WR(src)                       (((u32)(src)<<24) & 0x0f000000)
#define CTX_RD_F1_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields data_rd	 */
#define DATA_RD_F1_WIDTH                                                      4
#define DATA_RD_F1_SHIFT                                                     20
#define DATA_RD_F1_MASK                                              0x00f00000
#define DATA_RD_F1_RD(src)                           (((src) & 0x00f00000)>>20)
#define DATA_RD_F1_WR(src)                      (((u32)(src)<<20) & 0x00f00000)
#define DATA_RD_F1_SET(dst,src) \
                      (((dst) & ~0x00f00000) | (((u32)(src)<<20) & 0x00f00000))
/*	 Fields dstll_rd	 */
#define DSTLL_RD_F1_WIDTH                                                     4
#define DSTLL_RD_F1_SHIFT                                                    16
#define DSTLL_RD_F1_MASK                                             0x000f0000
#define DSTLL_RD_F1_RD(src)                          (((src) & 0x000f0000)>>16)
#define DSTLL_RD_F1_WR(src)                     (((u32)(src)<<16) & 0x000f0000)
#define DSTLL_RD_F1_SET(dst,src) \
                      (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))

/*	Register csr_axi_wr_max_outstanding_cfg	*/
/*	 Fields tkn_wr	 */
#define TKN_WR_F1_WIDTH                                                       4
#define TKN_WR_F1_SHIFT                                                      28
#define TKN_WR_F1_MASK                                               0xf0000000
#define TKN_WR_F1_RD(src)                            (((src) & 0xf0000000)>>28)
#define TKN_WR_F1_WR(src)                       (((u32)(src)<<28) & 0xf0000000)
#define TKN_WR_F1_SET(dst,src) \
                      (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields ctx_wr	 */
#define CTX_WR_F1_WIDTH                                                       4
#define CTX_WR_F1_SHIFT                                                      24
#define CTX_WR_F1_MASK                                               0x0f000000
#define CTX_WR_F1_RD(src)                            (((src) & 0x0f000000)>>24)
#define CTX_WR_F1_WR(src)                       (((u32)(src)<<24) & 0x0f000000)
#define CTX_WR_F1_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields data_wr	 */
#define DATA_WR_F1_WIDTH                                                      4
#define DATA_WR_F1_SHIFT                                                     20
#define DATA_WR_F1_MASK                                              0x00f00000
#define DATA_WR_F1_RD(src)                           (((src) & 0x00f00000)>>20)
#define DATA_WR_F1_WR(src)                      (((u32)(src)<<20) & 0x00f00000)
#define DATA_WR_F1_SET(dst,src) \
                      (((dst) & ~0x00f00000) | (((u32)(src)<<20) & 0x00f00000))

/*	Register csr_sec_int_sts	*/
/*	 Fields eip96_core	 */
#define EIP96_CORE_WIDTH                                                      1
#define EIP96_CORE_SHIFT                                                     31
#define EIP96_CORE_MASK                                              0x80000000
#define EIP96_CORE_RD(src)                           (((src) & 0x80000000)>>31)
#define EIP96_CORE_WR(src)                      (((u32)(src)<<31) & 0x80000000)
#define EIP96_CORE_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields tkn_rd	 */
#define TKN_RD_F2_WIDTH                                                       1
#define TKN_RD_F2_SHIFT                                                      30
#define TKN_RD_F2_MASK                                               0x40000000
#define TKN_RD_F2_RD(src)                            (((src) & 0x40000000)>>30)
#define TKN_RD_F2_WR(src)                       (((u32)(src)<<30) & 0x40000000)
#define TKN_RD_F2_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields ctx_rd	 */
#define CTX_RD_F2_WIDTH                                                       1
#define CTX_RD_F2_SHIFT                                                      29
#define CTX_RD_F2_MASK                                               0x20000000
#define CTX_RD_F2_RD(src)                            (((src) & 0x20000000)>>29)
#define CTX_RD_F2_WR(src)                       (((u32)(src)<<29) & 0x20000000)
#define CTX_RD_F2_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields data_rd	 */
#define DATA_RD_F2_WIDTH                                                      1
#define DATA_RD_F2_SHIFT                                                     28
#define DATA_RD_F2_MASK                                              0x10000000
#define DATA_RD_F2_RD(src)                           (((src) & 0x10000000)>>28)
#define DATA_RD_F2_WR(src)                      (((u32)(src)<<28) & 0x10000000)
#define DATA_RD_F2_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields dstll_rd	 */
#define DSTLL_RD_F2_WIDTH                                                     1
#define DSTLL_RD_F2_SHIFT                                                    27
#define DSTLL_RD_F2_MASK                                             0x08000000
#define DSTLL_RD_F2_RD(src)                          (((src) & 0x08000000)>>27)
#define DSTLL_RD_F2_WR(src)                     (((u32)(src)<<27) & 0x08000000)
#define DSTLL_RD_F2_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields tkn_wr	 */
#define TKN_WR_F2_WIDTH                                                       1
#define TKN_WR_F2_SHIFT                                                      26
#define TKN_WR_F2_MASK                                               0x04000000
#define TKN_WR_F2_RD(src)                            (((src) & 0x04000000)>>26)
#define TKN_WR_F2_WR(src)                       (((u32)(src)<<26) & 0x04000000)
#define TKN_WR_F2_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields ctx_wr	 */
#define CTX_WR_F2_WIDTH                                                       1
#define CTX_WR_F2_SHIFT                                                      25
#define CTX_WR_F2_MASK                                               0x02000000
#define CTX_WR_F2_RD(src)                            (((src) & 0x02000000)>>25)
#define CTX_WR_F2_WR(src)                       (((u32)(src)<<25) & 0x02000000)
#define CTX_WR_F2_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields data_wr	 */
#define DATA_WR_F2_WIDTH                                                      1
#define DATA_WR_F2_SHIFT                                                     24
#define DATA_WR_F2_MASK                                              0x01000000
#define DATA_WR_F2_RD(src)                           (((src) & 0x01000000)>>24)
#define DATA_WR_F2_WR(src)                      (((u32)(src)<<24) & 0x01000000)
#define DATA_WR_F2_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*	 Fields lerr	 */
#define LERR_WIDTH                                                            1
#define LERR_SHIFT                                                           23
#define LERR_MASK                                                    0x00800000
#define LERR_RD(src)                                 (((src) & 0x00800000)>>23)
#define LERR_WR(src)                            (((u32)(src)<<23) & 0x00800000)
#define LERR_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))

/*	Register csr_sec_int_stsMask	*/
/*    Mask Register Fields eip96_coreMask    */
#define EIP96_COREMASK_WIDTH                                                  1
#define EIP96_COREMASK_SHIFT                                                 31
#define EIP96_COREMASK_MASK                                          0x80000000
#define EIP96_COREMASK_RD(src)                       (((src) & 0x80000000)>>31)
#define EIP96_COREMASK_WR(src)                  (((u32)(src)<<31) & 0x80000000)
#define EIP96_COREMASK_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*    Mask Register Fields tkn_rdMask    */
#define TKN_RDMASK_WIDTH                                                      1
#define TKN_RDMASK_SHIFT                                                     30
#define TKN_RDMASK_MASK                                              0x40000000
#define TKN_RDMASK_RD(src)                           (((src) & 0x40000000)>>30)
#define TKN_RDMASK_WR(src)                      (((u32)(src)<<30) & 0x40000000)
#define TKN_RDMASK_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*    Mask Register Fields ctx_rdMask    */
#define CTX_RDMASK_WIDTH                                                      1
#define CTX_RDMASK_SHIFT                                                     29
#define CTX_RDMASK_MASK                                              0x20000000
#define CTX_RDMASK_RD(src)                           (((src) & 0x20000000)>>29)
#define CTX_RDMASK_WR(src)                      (((u32)(src)<<29) & 0x20000000)
#define CTX_RDMASK_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*    Mask Register Fields data_rdMask    */
#define DATA_RDMASK_WIDTH                                                     1
#define DATA_RDMASK_SHIFT                                                    28
#define DATA_RDMASK_MASK                                             0x10000000
#define DATA_RDMASK_RD(src)                          (((src) & 0x10000000)>>28)
#define DATA_RDMASK_WR(src)                     (((u32)(src)<<28) & 0x10000000)
#define DATA_RDMASK_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*    Mask Register Fields dstll_rdMask    */
#define DSTLL_RDMASK_WIDTH                                                    1
#define DSTLL_RDMASK_SHIFT                                                   27
#define DSTLL_RDMASK_MASK                                            0x08000000
#define DSTLL_RDMASK_RD(src)                         (((src) & 0x08000000)>>27)
#define DSTLL_RDMASK_WR(src)                    (((u32)(src)<<27) & 0x08000000)
#define DSTLL_RDMASK_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*    Mask Register Fields tkn_wrMask    */
#define TKN_WRMASK_WIDTH                                                      1
#define TKN_WRMASK_SHIFT                                                     26
#define TKN_WRMASK_MASK                                              0x04000000
#define TKN_WRMASK_RD(src)                           (((src) & 0x04000000)>>26)
#define TKN_WRMASK_WR(src)                      (((u32)(src)<<26) & 0x04000000)
#define TKN_WRMASK_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*    Mask Register Fields ctx_wrMask    */
#define CTX_WRMASK_WIDTH                                                      1
#define CTX_WRMASK_SHIFT                                                     25
#define CTX_WRMASK_MASK                                              0x02000000
#define CTX_WRMASK_RD(src)                           (((src) & 0x02000000)>>25)
#define CTX_WRMASK_WR(src)                      (((u32)(src)<<25) & 0x02000000)
#define CTX_WRMASK_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*    Mask Register Fields data_wrMask    */
#define DATA_WRMASK_WIDTH                                                     1
#define DATA_WRMASK_SHIFT                                                    24
#define DATA_WRMASK_MASK                                             0x01000000
#define DATA_WRMASK_RD(src)                          (((src) & 0x01000000)>>24)
#define DATA_WRMASK_WR(src)                     (((u32)(src)<<24) & 0x01000000)
#define DATA_WRMASK_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*    Mask Register Fields lerrMask    */
#define LERRMASK_WIDTH                                                        1
#define LERRMASK_SHIFT                                                       23
#define LERRMASK_MASK                                                0x00800000
#define LERRMASK_RD(src)                             (((src) & 0x00800000)>>23)
#define LERRMASK_WR(src)                        (((u32)(src)<<23) & 0x00800000)
#define LERRMASK_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))

/*	Register csr_sec_fifo_sts	*/
/*	 Fields crcf_empty	 */
#define CRCF_EMPTY_WIDTH                                                      1
#define CRCF_EMPTY_SHIFT                                                     31
#define CRCF_EMPTY_MASK                                              0x80000000
#define CRCF_EMPTY_RD(src)                           (((src) & 0x80000000)>>31)
#define CRCF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields cwcf_empty	 */
#define CWCF_EMPTY_WIDTH                                                      1
#define CWCF_EMPTY_SHIFT                                                     30
#define CWCF_EMPTY_MASK                                              0x40000000
#define CWCF_EMPTY_RD(src)                           (((src) & 0x40000000)>>30)
#define CWCF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields cwf_empty	 */
#define CWF_EMPTY_WIDTH                                                       1
#define CWF_EMPTY_SHIFT                                                      29
#define CWF_EMPTY_MASK                                               0x20000000
#define CWF_EMPTY_RD(src)                            (((src) & 0x20000000)>>29)
#define CWF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields trcf_empty	 */
#define TRCF_EMPTY_WIDTH                                                      1
#define TRCF_EMPTY_SHIFT                                                     28
#define TRCF_EMPTY_MASK                                              0x10000000
#define TRCF_EMPTY_RD(src)                           (((src) & 0x10000000)>>28)
#define TRCF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields twcf_empty	 */
#define TWCF_EMPTY_WIDTH                                                      1
#define TWCF_EMPTY_SHIFT                                                     27
#define TWCF_EMPTY_MASK                                              0x08000000
#define TWCF_EMPTY_RD(src)                           (((src) & 0x08000000)>>27)
#define TWCF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields twdf_empty	 */
#define TWDF_EMPTY_WIDTH                                                      1
#define TWDF_EMPTY_SHIFT                                                     26
#define TWDF_EMPTY_MASK                                              0x04000000
#define TWDF_EMPTY_RD(src)                           (((src) & 0x04000000)>>26)
#define TWDF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields dwszf_empty	 */
#define DWSZF_EMPTY_WIDTH                                                     1
#define DWSZF_EMPTY_SHIFT                                                    25
#define DWSZF_EMPTY_MASK                                             0x02000000
#define DWSZF_EMPTY_RD(src)                          (((src) & 0x02000000)>>25)
#define DWSZF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields drcf_empty	 */
#define DRCF_EMPTY_WIDTH                                                      1
#define DRCF_EMPTY_SHIFT                                                     24
#define DRCF_EMPTY_MASK                                              0x01000000
#define DRCF_EMPTY_RD(src)                           (((src) & 0x01000000)>>24)
#define DRCF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*	 Fields dwcf_empty	 */
#define DWCF_EMPTY_WIDTH                                                      1
#define DWCF_EMPTY_SHIFT                                                     23
#define DWCF_EMPTY_MASK                                              0x00800000
#define DWCF_EMPTY_RD(src)                           (((src) & 0x00800000)>>23)
#define DWCF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields dwf_empty	 */
#define DWF_EMPTY_WIDTH                                                       1
#define DWF_EMPTY_SHIFT                                                      22
#define DWF_EMPTY_MASK                                               0x00400000
#define DWF_EMPTY_RD(src)                            (((src) & 0x00400000)>>22)
#define DWF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))

/*	Register csr_max_pkt_timeout_ctrl	*/
/*	 Fields enable	 */
#define SEC_ENABLE_WIDTH                                                      1
#define SEC_ENABLE_SHIFT                                                     31
#define SEC_ENABLE_MASK                                              0x80000000
#define SEC_ENABLE_RD(src)                           (((src) & 0x80000000)>>31)
#define SEC_ENABLE_WR(src)                      (((u32)(src)<<31) & 0x80000000)
#define SEC_ENABLE_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields cnt	 */
#define SEC_CNT_WIDTH                                                        31
#define SEC_CNT_SHIFT                                                         0
#define SEC_CNT_MASK                                                 0x7fffffff
#define SEC_CNT_RD(src)                                  (((src) & 0x7fffffff))
#define SEC_CNT_WR(src)                             (((u32)(src)) & 0x7fffffff)
#define SEC_CNT_SET(dst,src) \
                          (((dst) & ~0x7fffffff) | (((u32)(src)) & 0x7fffffff))

/*	Register csr_max_fpbavl_timeout_ctrl	*/
/*	 Fields enable	 */
#define SEC_ENABLE_F1_WIDTH                                                   1
#define SEC_ENABLE_F1_SHIFT                                                  31
#define SEC_ENABLE_F1_MASK                                           0x80000000
#define SEC_ENABLE_F1_RD(src)                        (((src) & 0x80000000)>>31)
#define SEC_ENABLE_F1_WR(src)                   (((u32)(src)<<31) & 0x80000000)
#define SEC_ENABLE_F1_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields cnt	 */
#define SEC_CNT_F1_WIDTH                                                     31
#define SEC_CNT_F1_SHIFT                                                      0
#define SEC_CNT_F1_MASK                                              0x7fffffff
#define SEC_CNT_F1_RD(src)                               (((src) & 0x7fffffff))
#define SEC_CNT_F1_WR(src)                          (((u32)(src)) & 0x7fffffff)
#define SEC_CNT_F1_SET(dst,src) \
                          (((dst) & ~0x7fffffff) | (((u32)(src)) & 0x7fffffff))

/*	Register csr_misc_ctrl	*/
/*	 Fields en_dealloc_buf_cnt	 */
#define EN_DEALLOC_BUF_CNT_WIDTH                                              1
#define EN_DEALLOC_BUF_CNT_SHIFT                                             31
#define EN_DEALLOC_BUF_CNT_MASK                                      0x80000000
#define EN_DEALLOC_BUF_CNT_RD(src)                   (((src) & 0x80000000)>>31)
#define EN_DEALLOC_BUF_CNT_WR(src)              (((u32)(src)<<31) & 0x80000000)
#define EN_DEALLOC_BUF_CNT_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))

/*	Global Base Address	*/
#define SEC_EIP96_CRYPTO_CSR_BASE_ADDR			0x1f253000
#define SEC_XTS_CRYPTO_CSR_BASE_ADDR			0x1f251800
#define	SEC_EIP62_CRYPTO_CSR_BASE_ADDR			0x1f254800

/*    Address SEC_XTS_CRYPTO_CSR  Registers */
#define CSR_SEC_CRYPTO_CFG_0_ADDR                                    0x00000000
#define CSR_SEC_CRYPTO_CFG_0_DEFAULT                                 0x40000000
#define CSR_SEC_CRYPTO_CFG_1_ADDR                                    0x00000004
#define CSR_SEC_CRYPTO_CFG_1_DEFAULT                                 0xf0000000
#define CSR_SEC_CRYPTO_CFG_2_ADDR                                    0x00000008
#define CSR_SEC_CRYPTO_CFG_2_DEFAULT                                 0x00000000
#define CSR_SEC_FIFO_STS_ADDR                                        0x0000000c
#define CSR_SEC_FIFO_STS_DEFAULT                                     0xf0000000
#define CSR_SEC_EIP96_ERR_MASK_ADDR                                  0x00000010
#define CSR_SEC_EIP96_ERR_MASK_DEFAULT                               0xfffe0000
#define CSR_SEC_EIP62_CTX_CTRL_ADDR				     0x00000014
#define CSR_SEC_EIP62_CTX_CTRL_DEFAULT				     0x00000006

/*	Register csr_sec_crypto_cfg_0	*/
/*	 Fields tkn_rd_prefetch_size	 */
#define TKN_RD_PREFETCH_SIZE0_WIDTH                                           4
#define TKN_RD_PREFETCH_SIZE0_SHIFT                                          28
#define TKN_RD_PREFETCH_SIZE0_MASK                                   0xf0000000
#define TKN_RD_PREFETCH_SIZE0_RD(src)                (((src) & 0xf0000000)>>28)
#define TKN_RD_PREFETCH_SIZE0_WR(src)           (((u32)(src)<<28) & 0xf0000000)
#define TKN_RD_PREFETCH_SIZE0_SET(dst,src) \
                      (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields tkn_rd_offset_size	 */
#define TKN_RD_OFFSET_SIZE0_WIDTH                                             4
#define TKN_RD_OFFSET_SIZE0_SHIFT                                            24
#define TKN_RD_OFFSET_SIZE0_MASK                                     0x0f000000
#define TKN_RD_OFFSET_SIZE0_RD(src)                  (((src) & 0x0f000000)>>24)
#define TKN_RD_OFFSET_SIZE0_WR(src)             (((u32)(src)<<24) & 0x0f000000)
#define TKN_RD_OFFSET_SIZE0_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))

/*	Register csr_sec_crypto_cfg_1	*/
/*	 Fields dis_wait_tkn_wr_done	 */
#define DIS_WAIT_TKN_WR_DONE1_WIDTH                                           1
#define DIS_WAIT_TKN_WR_DONE1_SHIFT                                          31
#define DIS_WAIT_TKN_WR_DONE1_MASK                                   0x80000000
#define DIS_WAIT_TKN_WR_DONE1_RD(src)                (((src) & 0x80000000)>>31)
#define DIS_WAIT_TKN_WR_DONE1_WR(src)           (((u32)(src)<<31) & 0x80000000)
#define DIS_WAIT_TKN_WR_DONE1_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields dis_ctx_interlock	 */
#define DIS_CTX_INTERLOCK1_WIDTH                                              1
#define DIS_CTX_INTERLOCK1_SHIFT                                             30
#define DIS_CTX_INTERLOCK1_MASK                                      0x40000000
#define DIS_CTX_INTERLOCK1_RD(src)                   (((src) & 0x40000000)>>30)
#define DIS_CTX_INTERLOCK1_WR(src)              (((u32)(src)<<30) & 0x40000000)
#define DIS_CTX_INTERLOCK1_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields dis_zero_length_hndl	 */
#define DIS_ZERO_LENGTH_HNDL1_WIDTH                                           1
#define DIS_ZERO_LENGTH_HNDL1_SHIFT                                          29
#define DIS_ZERO_LENGTH_HNDL1_MASK                                   0x20000000
#define DIS_ZERO_LENGTH_HNDL1_RD(src)                (((src) & 0x20000000)>>29)
#define DIS_ZERO_LENGTH_HNDL1_WR(src)           (((u32)(src)<<29) & 0x20000000)
#define DIS_ZERO_LENGTH_HNDL1_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields en_adv_dma_in_err_hndl	 */
#define EN_ADV_DMA_IN_ERR_HNDL1_WIDTH                                         1
#define EN_ADV_DMA_IN_ERR_HNDL1_SHIFT                                        28
#define EN_ADV_DMA_IN_ERR_HNDL1_MASK                                 0x10000000
#define EN_ADV_DMA_IN_ERR_HNDL1_RD(src)              (((src) & 0x10000000)>>28)
#define EN_ADV_DMA_IN_ERR_HNDL1_WR(src)         (((u32)(src)<<28) & 0x10000000)
#define EN_ADV_DMA_IN_ERR_HNDL1_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))

/*	Register csr_sec_crypto_cfg_2	*/
/*	 Fields en_ib_rma	 */
#define EN_IB_RMA2_WIDTH                                                      1
#define EN_IB_RMA2_SHIFT                                                     31
#define EN_IB_RMA2_MASK                                              0x80000000
#define EN_IB_RMA2_RD(src)                           (((src) & 0x80000000)>>31)
#define EN_IB_RMA2_WR(src)                      (((u32)(src)<<31) & 0x80000000)
#define EN_IB_RMA2_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields ib_rma_ctrl	 */
#define IB_RMA_CTRL2_WIDTH                                                    2
#define IB_RMA_CTRL2_SHIFT                                                   29
#define IB_RMA_CTRL2_MASK                                            0x60000000
#define IB_RMA_CTRL2_RD(src)                         (((src) & 0x60000000)>>29)
#define IB_RMA_CTRL2_WR(src)                    (((u32)(src)<<29) & 0x60000000)
#define IB_RMA_CTRL2_SET(dst,src) \
                      (((dst) & ~0x60000000) | (((u32)(src)<<29) & 0x60000000))
/*	 Fields en_ib_rmb	 */
#define EN_IB_RMB2_WIDTH                                                      1
#define EN_IB_RMB2_SHIFT                                                     28
#define EN_IB_RMB2_MASK                                              0x10000000
#define EN_IB_RMB2_RD(src)                           (((src) & 0x10000000)>>28)
#define EN_IB_RMB2_WR(src)                      (((u32)(src)<<28) & 0x10000000)
#define EN_IB_RMB2_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields ib_rmb_ctrl	 */
#define IB_RMB_CTRL2_WIDTH                                                    2
#define IB_RMB_CTRL2_SHIFT                                                   26
#define IB_RMB_CTRL2_MASK                                            0x0c000000
#define IB_RMB_CTRL2_RD(src)                         (((src) & 0x0c000000)>>26)
#define IB_RMB_CTRL2_WR(src)                    (((u32)(src)<<26) & 0x0c000000)
#define IB_RMB_CTRL2_SET(dst,src) \
                      (((dst) & ~0x0c000000) | (((u32)(src)<<26) & 0x0c000000))
/*	 Fields en_ob_rma	 */
#define EN_OB_RMA2_WIDTH                                                      1
#define EN_OB_RMA2_SHIFT                                                     25
#define EN_OB_RMA2_MASK                                              0x02000000
#define EN_OB_RMA2_RD(src)                           (((src) & 0x02000000)>>25)
#define EN_OB_RMA2_WR(src)                      (((u32)(src)<<25) & 0x02000000)
#define EN_OB_RMA2_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields ob_rma_ctrl	 */
#define OB_RMA_CTRL2_WIDTH                                                    2
#define OB_RMA_CTRL2_SHIFT                                                   23
#define OB_RMA_CTRL2_MASK                                            0x01800000
#define OB_RMA_CTRL2_RD(src)                         (((src) & 0x01800000)>>23)
#define OB_RMA_CTRL2_WR(src)                    (((u32)(src)<<23) & 0x01800000)
#define OB_RMA_CTRL2_SET(dst,src) \
                      (((dst) & ~0x01800000) | (((u32)(src)<<23) & 0x01800000))
/*	 Fields en_ob_rmb	 */
#define EN_OB_RMB2_WIDTH                                                      1
#define EN_OB_RMB2_SHIFT                                                     22
#define EN_OB_RMB2_MASK                                              0x00400000
#define EN_OB_RMB2_RD(src)                           (((src) & 0x00400000)>>22)
#define EN_OB_RMB2_WR(src)                      (((u32)(src)<<22) & 0x00400000)
#define EN_OB_RMB2_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields ob_rmb_ctrl	 */
#define OB_RMB_CTRL2_WIDTH                                                    2
#define OB_RMB_CTRL2_SHIFT                                                   20
#define OB_RMB_CTRL2_MASK                                            0x00300000
#define OB_RMB_CTRL2_RD(src)                         (((src) & 0x00300000)>>20)
#define OB_RMB_CTRL2_WR(src)                    (((u32)(src)<<20) & 0x00300000)
#define OB_RMB_CTRL2_SET(dst,src) \
                      (((dst) & ~0x00300000) | (((u32)(src)<<20) & 0x00300000))

/*	Register csr_sec_fifo_sts	*/
/*	 Fields crf_empty	 */
#define CRF_EMPTY_WIDTH                                                       1
#define CRF_EMPTY_SHIFT                                                      31
#define CRF_EMPTY_MASK                                               0x80000000
#define CRF_EMPTY_RD(src)                            (((src) & 0x80000000)>>31)
#define CRF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields trdf_empty	 */
#define TRDF_EMPTY_WIDTH                                                      1
#define TRDF_EMPTY_SHIFT                                                     30
#define TRDF_EMPTY_MASK                                              0x40000000
#define TRDF_EMPTY_RD(src)                           (((src) & 0x40000000)>>30)
#define TRDF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields drf_empty	 */
#define DRF_EMPTY_WIDTH                                                       1
#define DRF_EMPTY_SHIFT                                                      29
#define DRF_EMPTY_MASK                                               0x20000000
#define DRF_EMPTY_RD(src)                            (((src) & 0x20000000)>>29)
#define DRF_EMPTY_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields qmi_wq_empty	 */
#define QMI_WQ_EMPTY_WIDTH                                                    1
#define QMI_WQ_EMPTY_SHIFT                                                   28
#define QMI_WQ_EMPTY_MASK                                            0x10000000
#define QMI_WQ_EMPTY_RD(src)                         (((src) & 0x10000000)>>28)
#define QMI_WQ_EMPTY_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))

/*	Register csr_sec_eip96_err_mask	*/
/*	 Fields E0_E15	 */
#define E0_E15_WIDTH                                                         16
#define E0_E15_SHIFT                                                         16
#define E0_E15_MASK                                                  0xffff0000
#define E0_E15_RD(src)                               (((src) & 0xffff0000)>>16)
#define E0_E15_WR(src)                          (((u32)(src)<<16) & 0xffff0000)
#define E0_E15_SET(dst,src) \
                      (((dst) & ~0xffff0000) | (((u32)(src)<<16) & 0xffff0000))

/*	Register csr_sec_eip62_ctx_ctrl	*/
/*	 Fields CTX_PREFETCH_NUM	 */
#define CTX_PREFETCH_NUM_WIDTH                                                         5
#define CTX_PREFETCH_NUM_SHIFT                                                         0
#define CTX_PREFETCH_NUM_MASK                                                  0x0000001f
#define CTX_PREFETCH_NUM_RD(src)                               (((src) & 0x0000001f))
#define CTX_PREFETCH_NUM_WR(src)                          (((u32)(src)) & 0x0000001f)
#define CTX_PREFETCH_NUM_SET(dst,src) \
                      (((dst) & ~0x0000001f) | (((u32)(src)) & 0x0000001f))

/*	Global Base Address	*/
#define SEC_XTS_CORE_CSR_BASE_ADDR			0x1f252000

/*    Address SEC_XTS_CORE_CSR  Registers */
#define CSR_XTS_CONTROL_ADDR                                         0x00000000
#define CSR_XTS_CONTROL_DEFAULT                                      0x00000000
#define CSR_XTS_CONFIGURATION_ADDR                                   0x00000004
#define CSR_XTS_CONFIGURATION_DEFAULT                                0x00000000
#define CSR_XTS_STATUS_ADDR                                          0x00000008
#define CSR_XTS_STATUS_DEFAULT                                       0x00600000
#define CSR_XTS_TOKEN_CONTROL_WORD_ADDR                              0x0000000c
#define CSR_XTS_TOKEN_CONTROL_WORD_DEFAULT                           0x00000000
#define CSR_XTS_TOKEN_CONTEXT_POINTER_ADDR                           0x00000010
#define CSR_XTS_TOKEN_CONTEXT_POINTER_DEFAULT                        0x00000000
#define CSR_XTS_TOKEN_DMA_LENGTH_ADDR                                0x00000014
#define CSR_XTS_TOKEN_DMA_LENGTH_DEFAULT                             0x00000000
#define CSR_XTS_TOKEN_IV_DWORD0_ADDR                                 0x00000018
#define CSR_XTS_TOKEN_IV_DWORD0_DEFAULT                              0x00000000
#define CSR_XTS_TOKEN_IV_DWORD1_ADDR                                 0x0000001c
#define CSR_XTS_TOKEN_IV_DWORD1_DEFAULT                              0x00000000
#define CSR_XTS_TOKEN_IV_DWORD2_ADDR                                 0x00000020
#define CSR_XTS_TOKEN_IV_DWORD2_DEFAULT                              0x00000000
#define CSR_XTS_TOKEN_IV_DWORD3_ADDR                                 0x00000024
#define CSR_XTS_TOKEN_IV_DWORD3_DEFAULT                              0x00000000
#define CSR_XTS_CONTEXT_KEY1_DWORD0_ADDR                             0x00000028
#define CSR_XTS_CONTEXT_KEY1_DWORD0_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY1_DWORD1_ADDR                             0x0000002c
#define CSR_XTS_CONTEXT_KEY1_DWORD1_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY1_DWORD2_ADDR                             0x00000030
#define CSR_XTS_CONTEXT_KEY1_DWORD2_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY1_DWORD3_ADDR                             0x00000034
#define CSR_XTS_CONTEXT_KEY1_DWORD3_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY1_DWORD4_ADDR                             0x00000038
#define CSR_XTS_CONTEXT_KEY1_DWORD4_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY1_DWORD5_ADDR                             0x0000003c
#define CSR_XTS_CONTEXT_KEY1_DWORD5_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY1_DWORD6_ADDR                             0x00000040
#define CSR_XTS_CONTEXT_KEY1_DWORD6_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY1_DWORD7_ADDR                             0x00000044
#define CSR_XTS_CONTEXT_KEY1_DWORD7_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY2_DWORD0_ADDR                             0x00000048
#define CSR_XTS_CONTEXT_KEY2_DWORD0_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY2_DWORD1_ADDR                             0x0000004c
#define CSR_XTS_CONTEXT_KEY2_DWORD1_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY2_DWORD2_ADDR                             0x00000050
#define CSR_XTS_CONTEXT_KEY2_DWORD2_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY2_DWORD3_ADDR                             0x00000054
#define CSR_XTS_CONTEXT_KEY2_DWORD3_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY2_DWORD4_ADDR                             0x00000058
#define CSR_XTS_CONTEXT_KEY2_DWORD4_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY2_DWORD5_ADDR                             0x0000005c
#define CSR_XTS_CONTEXT_KEY2_DWORD5_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY2_DWORD6_ADDR                             0x00000060
#define CSR_XTS_CONTEXT_KEY2_DWORD6_DEFAULT                          0x00000000
#define CSR_XTS_CONTEXT_KEY2_DWORD7_ADDR                             0x00000064
#define CSR_XTS_CONTEXT_KEY2_DWORD7_DEFAULT                          0x00000000
#define CSR_XTS_DIN_XFR_COUNT_ADDR                                   0x00000068
#define CSR_XTS_DIN_XFR_COUNT_DEFAULT                                0x00000000
#define CSR_XTS_DOUT_XFR_COUNT_ADDR                                  0x0000006c
#define CSR_XTS_DOUT_XFR_COUNT_DEFAULT                               0x00000000
#define CSR_XTS_DEBUG_TRACE_ADDR                                     0x00000070
#define CSR_XTS_DEBUG_TRACE_DEFAULT                                  0x844c4000

/*	Register CSR_XTS_CONTROL	*/
/*	 Fields DEBUG_TRACE_SEL_RBIT	 */
#define DEBUG_TRACE_SEL_RBIT_WIDTH                                            2
#define DEBUG_TRACE_SEL_RBIT_SHIFT                                           16
#define DEBUG_TRACE_SEL_RBIT_MASK                                    0x00030000
#define DEBUG_TRACE_SEL_RBIT_RD(src)                 (((src) & 0x00030000)>>16)
#define DEBUG_TRACE_SEL_RBIT_WR(src)            (((u32)(src)<<16) & 0x00030000)
#define DEBUG_TRACE_SEL_RBIT_SET(dst,src) \
                      (((dst) & ~0x00030000) | (((u32)(src)<<16) & 0x00030000))
/*	 Fields HALT_REQ_RBIT	 */
#define HALT_REQ_RBIT_WIDTH                                                   1
#define HALT_REQ_RBIT_SHIFT                                                  10
#define HALT_REQ_RBIT_MASK                                           0x00000400
#define HALT_REQ_RBIT_RD(src)                        (((src) & 0x00000400)>>10)
#define HALT_REQ_RBIT_WR(src)                   (((u32)(src)<<10) & 0x00000400)
#define HALT_REQ_RBIT_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields PAUSE_ON_DONE_REQ_RBIT	 */
#define PAUSE_ON_DONE_REQ_RBIT_WIDTH                                          1
#define PAUSE_ON_DONE_REQ_RBIT_SHIFT                                          9
#define PAUSE_ON_DONE_REQ_RBIT_MASK                                  0x00000200
#define PAUSE_ON_DONE_REQ_RBIT_RD(src)                (((src) & 0x00000200)>>9)
#define PAUSE_ON_DONE_REQ_RBIT_WR(src)           (((u32)(src)<<9) & 0x00000200)
#define PAUSE_ON_DONE_REQ_RBIT_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields XTS_ERROR_CLR_RBIT	 */
#define XTS_ERROR_CLR_RBIT_WIDTH                                              1
#define XTS_ERROR_CLR_RBIT_SHIFT                                              1
#define XTS_ERROR_CLR_RBIT_MASK                                      0x00000002
#define XTS_ERROR_CLR_RBIT_RD(src)                    (((src) & 0x00000002)>>1)
#define XTS_ERROR_CLR_RBIT_WR(src)               (((u32)(src)<<1) & 0x00000002)
#define XTS_ERROR_CLR_RBIT_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields XTS_CORE_RST_RBIT	 */
#define XTS_CORE_RST_RBIT_WIDTH                                               1
#define XTS_CORE_RST_RBIT_SHIFT                                               0
#define XTS_CORE_RST_RBIT_MASK                                       0x00000001
#define XTS_CORE_RST_RBIT_RD(src)                        (((src) & 0x00000001))
#define XTS_CORE_RST_RBIT_WR(src)                   (((u32)(src)) & 0x00000001)
#define XTS_CORE_RST_RBIT_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register CSR_XTS_CONFIGURATION	*/
/*	 Fields NO_RESIDUAL_DATA_POST_DIN_ERROR_RBIT	 */
#define NO_RESIDUAL_DATA_POST_DIN_ERROR_RBIT_WIDTH                            1
#define NO_RESIDUAL_DATA_POST_DIN_ERROR_RBIT_SHIFT                            3
#define NO_RESIDUAL_DATA_POST_DIN_ERROR_RBIT_MASK                    0x00000008
#define NO_RESIDUAL_DATA_POST_DIN_ERROR_RBIT_RD(src)  (((src) & 0x00000008)>>3)
#define NO_RESIDUAL_DATA_POST_DIN_ERROR_RBIT_WR(src) \
                                                 (((u32)(src)<<3) & 0x00000008)
#define NO_RESIDUAL_DATA_POST_DIN_ERROR_RBIT_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields HALT_ON_ERROR_RBIT	 */
#define HALT_ON_ERROR_RBIT_WIDTH                                              1
#define HALT_ON_ERROR_RBIT_SHIFT                                              2
#define HALT_ON_ERROR_RBIT_MASK                                      0x00000004
#define HALT_ON_ERROR_RBIT_RD(src)                    (((src) & 0x00000004)>>2)
#define HALT_ON_ERROR_RBIT_WR(src)               (((u32)(src)<<2) & 0x00000004)
#define HALT_ON_ERROR_RBIT_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields ERROR_INTERRUPT_MASK_RBIT	 */
#define ERROR_INTERRUPT_MASK_RBIT_WIDTH                                       1
#define ERROR_INTERRUPT_MASK_RBIT_SHIFT                                       1
#define ERROR_INTERRUPT_MASK_RBIT_MASK                               0x00000002
#define ERROR_INTERRUPT_MASK_RBIT_RD(src)             (((src) & 0x00000002)>>1)
#define ERROR_INTERRUPT_MASK_RBIT_WR(src)        (((u32)(src)<<1) & 0x00000002)
#define ERROR_INTERRUPT_MASK_RBIT_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields CTS_MODE_ENABLE_RBIT	 */
#define SEC_CTS_MODE_ENABLE_RBIT_WIDTH                                        1
#define SEC_CTS_MODE_ENABLE_RBIT_SHIFT                                        0
#define SEC_CTS_MODE_ENABLE_RBIT_MASK                                0x00000001
#define SEC_CTS_MODE_ENABLE_RBIT_RD(src)                 (((src) & 0x00000001))
#define SEC_CTS_MODE_ENABLE_RBIT_WR(src)            (((u32)(src)) & 0x00000001)
#define SEC_CTS_MODE_ENABLE_RBIT_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register CSR_XTS_STATUS	*/
/*	 Fields XTS_IV_J_VALUE	 */
#define XTS_IV_J_VALUE_WIDTH                                                  8
#define XTS_IV_J_VALUE_SHIFT                                                 24
#define XTS_IV_J_VALUE_MASK                                          0xff000000
#define XTS_IV_J_VALUE_RD(src)                       (((src) & 0xff000000)>>24)
#define XTS_IV_J_VALUE_SET(dst,src) \
                      (((dst) & ~0xff000000) | (((u32)(src)<<24) & 0xff000000))
/*	 Fields DOUT_XFR_DONE	 */
#define DOUT_XFR_DONE_WIDTH                                                   1
#define DOUT_XFR_DONE_SHIFT                                                  22
#define DOUT_XFR_DONE_MASK                                           0x00400000
#define DOUT_XFR_DONE_RD(src)                        (((src) & 0x00400000)>>22)
#define DOUT_XFR_DONE_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields DIN_XFR_DONE	 */
#define DIN_XFR_DONE_WIDTH                                                    1
#define DIN_XFR_DONE_SHIFT                                                   21
#define DIN_XFR_DONE_MASK                                            0x00200000
#define DIN_XFR_DONE_RD(src)                         (((src) & 0x00200000)>>21)
#define DIN_XFR_DONE_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields CRYPTO_DATA_COUNT	 */
#define CRYPTO_DATA_COUNT_WIDTH                                               5
#define CRYPTO_DATA_COUNT_SHIFT                                              16
#define CRYPTO_DATA_COUNT_MASK                                       0x001f0000
#define CRYPTO_DATA_COUNT_RD(src)                    (((src) & 0x001f0000)>>16)
#define CRYPTO_DATA_COUNT_SET(dst,src) \
                      (((dst) & ~0x001f0000) | (((u32)(src)<<16) & 0x001f0000))
/*	 Fields HALT_ACK_RBIT	 */
#define HALT_ACK_RBIT_WIDTH                                                   1
#define HALT_ACK_RBIT_SHIFT                                                   9
#define HALT_ACK_RBIT_MASK                                           0x00000200
#define HALT_ACK_RBIT_RD(src)                         (((src) & 0x00000200)>>9)
#define HALT_ACK_RBIT_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields PAUSE_ON_DONE_ACK_RBIT	 */
#define PAUSE_ON_DONE_ACK_RBIT_WIDTH                                          1
#define PAUSE_ON_DONE_ACK_RBIT_SHIFT                                          8
#define PAUSE_ON_DONE_ACK_RBIT_MASK                                  0x00000100
#define PAUSE_ON_DONE_ACK_RBIT_RD(src)                (((src) & 0x00000100)>>8)
#define PAUSE_ON_DONE_ACK_RBIT_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields TKN_UNDERFLOW_ERROR	 */
#define TKN_UNDERFLOW_ERROR_WIDTH                                             1
#define TKN_UNDERFLOW_ERROR_SHIFT                                             5
#define TKN_UNDERFLOW_ERROR_MASK                                     0x00000020
#define TKN_UNDERFLOW_ERROR_RD(src)                   (((src) & 0x00000020)>>5)
#define TKN_UNDERFLOW_ERROR_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields CTX_OVERFLOW_ERROR	 */
#define CTX_OVERFLOW_ERROR_WIDTH                                              1
#define CTX_OVERFLOW_ERROR_SHIFT                                              4
#define CTX_OVERFLOW_ERROR_MASK                                      0x00000010
#define CTX_OVERFLOW_ERROR_RD(src)                    (((src) & 0x00000010)>>4)
#define CTX_OVERFLOW_ERROR_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields DIN_OVERFLOW_ERROR	 */
#define DIN_OVERFLOW_ERROR_WIDTH                                              1
#define DIN_OVERFLOW_ERROR_SHIFT                                              3
#define DIN_OVERFLOW_ERROR_MASK                                      0x00000008
#define DIN_OVERFLOW_ERROR_RD(src)                    (((src) & 0x00000008)>>3)
#define DIN_OVERFLOW_ERROR_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields DOUT_OVERFLOW_ERROR	 */
#define DOUT_OVERFLOW_ERROR_WIDTH                                             1
#define DOUT_OVERFLOW_ERROR_SHIFT                                             2
#define DOUT_OVERFLOW_ERROR_MASK                                     0x00000004
#define DOUT_OVERFLOW_ERROR_RD(src)                   (((src) & 0x00000004)>>2)
#define DOUT_OVERFLOW_ERROR_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields DIN_DMA_ERROR	 */
#define DIN_DMA_ERROR_WIDTH                                                   1
#define DIN_DMA_ERROR_SHIFT                                                   1
#define DIN_DMA_ERROR_MASK                                           0x00000002
#define DIN_DMA_ERROR_RD(src)                         (((src) & 0x00000002)>>1)
#define DIN_DMA_ERROR_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields DOUT_DMA_ERROR	 */
#define DOUT_DMA_ERROR_WIDTH                                                  1
#define DOUT_DMA_ERROR_SHIFT                                                  0
#define DOUT_DMA_ERROR_MASK                                          0x00000001
#define DOUT_DMA_ERROR_RD(src)                           (((src) & 0x00000001))
#define DOUT_DMA_ERROR_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register CSR_XTS_TOKEN_CONTROL_WORD	*/
/*	 Fields TKN_DWORD0	 */
#define TKN_DWORD0_WIDTH                                                     32
#define TKN_DWORD0_SHIFT                                                      0
#define TKN_DWORD0_MASK                                              0xffffffff
#define TKN_DWORD0_RD(src)                               (((src) & 0xffffffff))
#define TKN_DWORD0_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_TOKEN_CONTEXT_POINTER	*/
/*	 Fields TKN_CONTEXT_POINTER	 */
#define TKN_CONTEXT_POINTER_WIDTH                                            32
#define TKN_CONTEXT_POINTER_SHIFT                                             0
#define TKN_CONTEXT_POINTER_MASK                                     0xffffffff
#define TKN_CONTEXT_POINTER_RD(src)                      (((src) & 0xffffffff))
#define TKN_CONTEXT_POINTER_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_TOKEN_DMA_LENGTH	*/
/*	 Fields TKN_DMA_LENGTH	 */
#define TKN_DMA_LENGTH_WIDTH                                                 32
#define TKN_DMA_LENGTH_SHIFT                                                  0
#define TKN_DMA_LENGTH_MASK                                          0xffffffff
#define TKN_DMA_LENGTH_RD(src)                           (((src) & 0xffffffff))
#define TKN_DMA_LENGTH_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_TOKEN_IV_DWORD0	*/
/*	 Fields TKN_IV_DWORD0	 */
#define TKN_IV_DWORD00_WIDTH                                                 32
#define TKN_IV_DWORD00_SHIFT                                                  0
#define TKN_IV_DWORD00_MASK                                          0xffffffff
#define TKN_IV_DWORD00_RD(src)                           (((src) & 0xffffffff))
#define TKN_IV_DWORD00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_TOKEN_IV_DWORD1	*/
/*	 Fields TKN_IV_DWORD1	 */
#define TKN_IV_DWORD11_WIDTH                                                 32
#define TKN_IV_DWORD11_SHIFT                                                  0
#define TKN_IV_DWORD11_MASK                                          0xffffffff
#define TKN_IV_DWORD11_RD(src)                           (((src) & 0xffffffff))
#define TKN_IV_DWORD11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_TOKEN_IV_DWORD2	*/
/*	 Fields TKN_IV_DWORD2	 */
#define TKN_IV_DWORD22_WIDTH                                                 32
#define TKN_IV_DWORD22_SHIFT                                                  0
#define TKN_IV_DWORD22_MASK                                          0xffffffff
#define TKN_IV_DWORD22_RD(src)                           (((src) & 0xffffffff))
#define TKN_IV_DWORD22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_TOKEN_IV_DWORD3	*/
/*	 Fields TKN_IV_DWORD3	 */
#define TKN_IV_DWORD33_WIDTH                                                 32
#define TKN_IV_DWORD33_SHIFT                                                  0
#define TKN_IV_DWORD33_MASK                                          0xffffffff
#define TKN_IV_DWORD33_RD(src)                           (((src) & 0xffffffff))
#define TKN_IV_DWORD33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY1_DWORD0	*/
/*	 Fields CTX_KEY1_DWORD0	 */
#define CTX_KEY1_DWORD00_WIDTH                                               32
#define CTX_KEY1_DWORD00_SHIFT                                                0
#define CTX_KEY1_DWORD00_MASK                                        0xffffffff
#define CTX_KEY1_DWORD00_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY1_DWORD00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY1_DWORD1	*/
/*	 Fields CTX_KEY1_DWORD1	 */
#define CTX_KEY1_DWORD11_WIDTH                                               32
#define CTX_KEY1_DWORD11_SHIFT                                                0
#define CTX_KEY1_DWORD11_MASK                                        0xffffffff
#define CTX_KEY1_DWORD11_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY1_DWORD11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY1_DWORD2	*/
/*	 Fields CTX_KEY1_DWORD2	 */
#define CTX_KEY1_DWORD22_WIDTH                                               32
#define CTX_KEY1_DWORD22_SHIFT                                                0
#define CTX_KEY1_DWORD22_MASK                                        0xffffffff
#define CTX_KEY1_DWORD22_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY1_DWORD22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY1_DWORD3	*/
/*	 Fields CTX_KEY1_DWORD3	 */
#define CTX_KEY1_DWORD33_WIDTH                                               32
#define CTX_KEY1_DWORD33_SHIFT                                                0
#define CTX_KEY1_DWORD33_MASK                                        0xffffffff
#define CTX_KEY1_DWORD33_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY1_DWORD33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY1_DWORD4	*/
/*	 Fields CTX_KEY1_DWORD4	 */
#define CTX_KEY1_DWORD44_WIDTH                                               32
#define CTX_KEY1_DWORD44_SHIFT                                                0
#define CTX_KEY1_DWORD44_MASK                                        0xffffffff
#define CTX_KEY1_DWORD44_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY1_DWORD44_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY1_DWORD5	*/
/*	 Fields CTX_KEY1_DWORD5	 */
#define CTX_KEY1_DWORD55_WIDTH                                               32
#define CTX_KEY1_DWORD55_SHIFT                                                0
#define CTX_KEY1_DWORD55_MASK                                        0xffffffff
#define CTX_KEY1_DWORD55_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY1_DWORD55_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY1_DWORD6	*/
/*	 Fields CTX_KEY1_DWORD6	 */
#define CTX_KEY1_DWORD66_WIDTH                                               32
#define CTX_KEY1_DWORD66_SHIFT                                                0
#define CTX_KEY1_DWORD66_MASK                                        0xffffffff
#define CTX_KEY1_DWORD66_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY1_DWORD66_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY1_DWORD7	*/
/*	 Fields CTX_KEY1_DWORD7	 */
#define CTX_KEY1_DWORD77_WIDTH                                               32
#define CTX_KEY1_DWORD77_SHIFT                                                0
#define CTX_KEY1_DWORD77_MASK                                        0xffffffff
#define CTX_KEY1_DWORD77_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY1_DWORD77_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY2_DWORD0	*/
/*	 Fields CTX_KEY2_DWORD0	 */
#define CTX_KEY2_DWORD00_WIDTH                                               32
#define CTX_KEY2_DWORD00_SHIFT                                                0
#define CTX_KEY2_DWORD00_MASK                                        0xffffffff
#define CTX_KEY2_DWORD00_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY2_DWORD00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY2_DWORD1	*/
/*	 Fields CTX_KEY2_DWORD1	 */
#define CTX_KEY2_DWORD11_WIDTH                                               32
#define CTX_KEY2_DWORD11_SHIFT                                                0
#define CTX_KEY2_DWORD11_MASK                                        0xffffffff
#define CTX_KEY2_DWORD11_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY2_DWORD11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY2_DWORD2	*/
/*	 Fields CTX_KEY2_DWORD2	 */
#define CTX_KEY2_DWORD22_WIDTH                                               32
#define CTX_KEY2_DWORD22_SHIFT                                                0
#define CTX_KEY2_DWORD22_MASK                                        0xffffffff
#define CTX_KEY2_DWORD22_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY2_DWORD22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY2_DWORD3	*/
/*	 Fields CTX_KEY2_DWORD3	 */
#define CTX_KEY2_DWORD33_WIDTH                                               32
#define CTX_KEY2_DWORD33_SHIFT                                                0
#define CTX_KEY2_DWORD33_MASK                                        0xffffffff
#define CTX_KEY2_DWORD33_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY2_DWORD33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY2_DWORD4	*/
/*	 Fields CTX_KEY2_DWORD4	 */
#define CTX_KEY2_DWORD44_WIDTH                                               32
#define CTX_KEY2_DWORD44_SHIFT                                                0
#define CTX_KEY2_DWORD44_MASK                                        0xffffffff
#define CTX_KEY2_DWORD44_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY2_DWORD44_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY2_DWORD5	*/
/*	 Fields CTX_KEY2_DWORD5	 */
#define CTX_KEY2_DWORD55_WIDTH                                               32
#define CTX_KEY2_DWORD55_SHIFT                                                0
#define CTX_KEY2_DWORD55_MASK                                        0xffffffff
#define CTX_KEY2_DWORD55_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY2_DWORD55_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY2_DWORD6	*/
/*	 Fields CTX_KEY2_DWORD6	 */
#define CTX_KEY2_DWORD66_WIDTH                                               32
#define CTX_KEY2_DWORD66_SHIFT                                                0
#define CTX_KEY2_DWORD66_MASK                                        0xffffffff
#define CTX_KEY2_DWORD66_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY2_DWORD66_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_CONTEXT_KEY2_DWORD7	*/
/*	 Fields CTX_KEY2_DWORD7	 */
#define CTX_KEY2_DWORD77_WIDTH                                               32
#define CTX_KEY2_DWORD77_SHIFT                                                0
#define CTX_KEY2_DWORD77_MASK                                        0xffffffff
#define CTX_KEY2_DWORD77_RD(src)                         (((src) & 0xffffffff))
#define CTX_KEY2_DWORD77_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_DIN_XFR_COUNT	*/
/*	 Fields DIN_XFR_COUNT	 */
#define DIN_XFR_COUNT_WIDTH                                                  32
#define DIN_XFR_COUNT_SHIFT                                                   0
#define DIN_XFR_COUNT_MASK                                           0xffffffff
#define DIN_XFR_COUNT_RD(src)                            (((src) & 0xffffffff))
#define DIN_XFR_COUNT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_DOUT_XFR_COUNT	*/
/*	 Fields DOUT_XFR_COUNT	 */
#define DOUT_XFR_COUNT_WIDTH                                                 32
#define DOUT_XFR_COUNT_SHIFT                                                  0
#define DOUT_XFR_COUNT_MASK                                          0xffffffff
#define DOUT_XFR_COUNT_RD(src)                           (((src) & 0xffffffff))
#define DOUT_XFR_COUNT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CSR_XTS_DEBUG_TRACE	*/
/*	 Fields DEBUG_TRACE	 */
#define DEBUG_TRACE_WIDTH                                                    32
#define DEBUG_TRACE_SHIFT                                                     0
#define DEBUG_TRACE_MASK                                             0xffffffff
#define DEBUG_TRACE_RD(src)                              (((src) & 0xffffffff))
#define DEBUG_TRACE_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*    Address SEC_EIP96_AXI_CSR  Registers */

/*    Address SEC_EIP96_CRYPTO_CSR  Registers */

/*	Global Base Address	*/
#define SEC_EIP96_CORE_CSR_BASE_ADDR			0x1f253800
#define SEC_EIP62_CORE_CSR_BASE_ADDR			0x1f255000

/*    Address SEC_EIP96_CORE_CSR  Registers */
#define IPE_TKN_CTRL_STAT_ADDR                                       0x00000000
#define IPE_TKN_CTRL_STAT_DEFAULT                                    0x00004004
#define IPE_PRC_ALG_EN_ADDR                                          0x00000004
#define IPE_PRC_ALG_EN_DEFAULT                                       0xffffffff
#define IPE_CTX_CTRL_ADDR                                            0x00000008
#define IPE_CTX_CTRL_DEFAULT                                         0x00000235
#define IPE_CTX_STAT_ADDR                                            0x0000000c
#define IPE_CTX_STAT_DEFAULT                                         0x00000000
#define IPE_INT_CTRL_STAT_ADDR                                       0x00000010
#define IPE_INT_CTRL_STAT_DEFAULT                                    0xc00f0000
/*	EIP96_CORE Register	*/
#define IPE_RX_CTRL_STAT_ADDR                                        0x00000014
#define IPE_RX_CTRL_STAT_DEFAULT                                     0xf88008ff
#define IPE_TX_CTRL_STAT_ADDR                                        0x00000018
#define IPE_TX_CTRL_STAT_DEFAULT                                     0xf88008ff
#define IPE_DEV_INFO_ADDR                                            0x0000003c
#define IPE_DEV_INFO_DEFAULT                                         0xdfddf312
#define IPE_PRNG_STAT_ADDR                                           0x00000040
#define IPE_PRNG_STAT_DEFAULT                                        0x00000000
#define IPE_PRNG_CTRL_ADDR                                           0x00000044
#define IPE_PRNG_CTRL_DEFAULT                                        0x00000000
#define IPE_PRNG_SEED_L_ADDR                                         0x00000048
#define IPE_PRNG_SEED_L_DEFAULT                                      0x00000000
#define IPE_PRNG_SEED_H_ADDR                                         0x0000004c
#define IPE_PRNG_SEED_H_DEFAULT                                      0x00000000
#define IPE_PRNG_KEY_0_L_ADDR                                        0x00000050
#define IPE_PRNG_KEY_0_L_DEFAULT                                     0x00000000
#define IPE_PRNG_KEY_0_H_ADDR                                        0x00000054
#define IPE_PRNG_KEY_0_H_DEFAULT                                     0x00000000
#define IPE_PRNG_KEY_1_L_ADDR                                        0x00000058
#define IPE_PRNG_KEY_1_L_DEFAULT                                     0x00000000
#define IPE_PRNG_KEY_1_H_ADDR                                        0x0000005c
#define IPE_PRNG_KEY_1_H_DEFAULT                                     0x00000000
#define IPE_PRNG_RES0_L_ADDR                                         0x00000060
#define IPE_PRNG_RES0_L_DEFAULT                                      0x00000000
#define IPE_PRNG_RES0_H_ADDR                                         0x00000064
#define IPE_PRNG_RES0_H_DEFAULT                                      0x00000000
#define IPE_PRNG_RES1_L_ADDR                                         0x00000068
#define IPE_PRNG_RES1_L_DEFAULT                                      0x00000000
#define IPE_PRNG_RES1_H_ADDR                                         0x0000006c
#define IPE_PRNG_RES1_H_DEFAULT                                      0x00000000
#define IPE_PRNG_LFSR_L_ADDR                                         0x00000070
#define IPE_PRNG_LFSR_L_DEFAULT                                      0x00000000
#define IPE_PRNG_LFSR_H_ADDR                                         0x00000074
#define IPE_PRNG_LFSR_H_DEFAULT                                      0x00000000
#define IPE_ACT_TKN_W0_ADDR                                          0x00000080
#define IPE_ACT_TKN_W0_DEFAULT                                       0x00000000
#define IPE_ACT_TKN_W1_ADDR                                          0x00000084
#define IPE_ACT_TKN_W1_DEFAULT                                       0x00000000
#define IPE_ACT_TKN_W2_ADDR                                          0x00000088
#define IPE_ACT_TKN_W2_DEFAULT                                       0x00000000
#define IPE_ACT_TKN_W3_ADDR                                          0x0000008c
#define IPE_ACT_TKN_W3_DEFAULT                                       0x00000000
#define IPE_ACT_TKN_W4_ADDR                                          0x00000090
#define IPE_ACT_TKN_W4_DEFAULT                                       0x00000000
#define IPE_ACT_TKN_W5_ADDR                                          0x00000094
#define IPE_ACT_TKN_W5_DEFAULT                                       0x00000000
#define IPE_ACT_TKN_W6_ADDR                                          0x00000098
#define IPE_ACT_TKN_W6_DEFAULT                                       0x00000000
#define IPE_ACT_TKN_W7_ADDR                                          0x0000009c
#define IPE_ACT_TKN_W7_DEFAULT                                       0x00000000
#define IPE_ACT_TKN_W8_ADDR                                          0x000000a0
#define IPE_ACT_TKN_W8_DEFAULT                                       0x00000000
#define IPE_ACT_TKN_W9_ADDR                                          0x000000a4
#define IPE_ACT_TKN_W9_DEFAULT                                       0x00000000
#define IPE_ACT_TKN_W10_ADDR                                         0x000000a8
#define IPE_ACT_TKN_W10_DEFAULT                                      0x00000000
#define IPE_ACT_TKN_W11_ADDR                                         0x000000ac
#define IPE_ACT_TKN_W11_DEFAULT                                      0x00000000
#define IPE_ACT_TKN_W12_ADDR                                         0x000000b0
#define IPE_ACT_TKN_W12_DEFAULT                                      0x00000000
#define IPE_ACT_TKN_W13_ADDR                                         0x000000b4
#define IPE_ACT_TKN_W13_DEFAULT                                      0x00000000
#define IPE_ACT_TKN_W14_ADDR                                         0x000000b8
#define IPE_ACT_TKN_W14_DEFAULT                                      0x00000000
#define IPE_ACT_TKN_W15_ADDR                                         0x000000bc
#define IPE_ACT_TKN_W15_DEFAULT                                      0x00000000
#define IPE_NXT_TKN_W0_ACT_W16_ADDR                                  0x000000c0
#define IPE_NXT_TKN_W0_ACT_W16_DEFAULT                               0x00000000
#define IPE_NXT_TKN_W1_ACT_W17_ADDR                                  0x000000c4
#define IPE_NXT_TKN_W1_ACT_W17_DEFAULT                               0x00000000
#define IPE_NXT_TKN_W2_ACT_W18_ADDR                                  0x000000c8
#define IPE_NXT_TKN_W2_ACT_W18_DEFAULT                               0x00000000
#define IPE_NXT_TKN_W3_ACT_W19_ADDR                                  0x000000cc
#define IPE_NXT_TKN_W3_ACT_W19_DEFAULT                               0x00000000
#define IPE_NXT_TKN_W4_ACT_W20_ADDR                                  0x000000d0
#define IPE_NXT_TKN_W4_ACT_W20_DEFAULT                               0x00000000
#define IPE_NXT_TKN_W5_ACT_W21_ADDR                                  0x000000d4
#define IPE_NXT_TKN_W5_ACT_W21_DEFAULT                               0x00000000
#define IPE_NXT_TKN_W6_ACT_W22_ADDR                                  0x000000d8
#define IPE_NXT_TKN_W6_ACT_W22_DEFAULT                               0x00000000
#define IPE_NXT_TKN_W7_ACT_W23_ADDR                                  0x000000dc
#define IPE_NXT_TKN_W7_ACT_W23_DEFAULT                               0x00000000
#define IPE_NXT_TKN_W8_ACT_W24_ADDR                                  0x000000e0
#define IPE_NXT_TKN_W8_ACT_W24_DEFAULT                               0x00000000
#define IPE_NXT_TKN_W9_ACT_W25_ADDR                                  0x000000e4
#define IPE_NXT_TKN_W9_ACT_W25_DEFAULT                               0x00000000
#define IPE_NXT_TKN_W10_ACT_W26_ADDR                                 0x000000e8
#define IPE_NXT_TKN_W10_ACT_W26_DEFAULT                              0x00000000
#define IPE_NXT_TKN_W11_ACT_W27_ADDR                                 0x000000ec
#define IPE_NXT_TKN_W11_ACT_W27_DEFAULT                              0x00000000
#define IPE_NXT_TKN_W12_ACT_W28_ADDR                                 0x000000f0
#define IPE_NXT_TKN_W12_ACT_W28_DEFAULT                              0x00000000
#define IPE_NXT_TKN_W13_ACT_W29_ADDR                                 0x000000f4
#define IPE_NXT_TKN_W13_ACT_W29_DEFAULT                              0x00000000
#define IPE_NXT_TKN_W14_ACT_W30_ADDR                                 0x000000f8
#define IPE_NXT_TKN_W14_ACT_W30_DEFAULT                              0x00000000
#define IPE_NXT_TKN_W15_ACT_W31_ADDR                                 0x000000fc
#define IPE_NXT_TKN_W15_ACT_W31_DEFAULT                              0x00000000
#define IPE_RES_TKN_W0_ADDR                                          0x00000100
#define IPE_RES_TKN_W0_DEFAULT                                       0x00000000
#define IPE_RES_TKN_W1_ADDR                                          0x00000104
#define IPE_RES_TKN_W1_DEFAULT                                       0x00000000
#define IPE_RES_TKN_W2_ADDR                                          0x00000108
#define IPE_RES_TKN_W2_DEFAULT                                       0x00000000
#define IPE_RES_TKN_W3_ADDR                                          0x0000010c
#define IPE_RES_TKN_W3_DEFAULT                                       0x00000000
#define IPE_RES_TKN_W4_ADDR                                          0x00000110
#define IPE_RES_TKN_W4_DEFAULT                                       0x00000000
#define IPE_RES_TKN_W5_ADDR                                          0x00000114
#define IPE_RES_TKN_W5_DEFAULT                                       0x00000000
#define IPE_RES_TKN_W6_ADDR                                          0x00000118
#define IPE_RES_TKN_W6_DEFAULT                                       0x00000000
#define IPE_RES_TKN_W7_ADDR                                          0x0000011c
#define IPE_RES_TKN_W7_DEFAULT                                       0x00000000
#define IPE_NXT_CTX_CMD0_ADDR                                        0x00000140
#define IPE_NXT_CTX_CMD0_DEFAULT                                     0x00000000
#define IPE_NXT_CTX_CMD1_ADDR                                        0x00000144
#define IPE_NXT_CTX_CMD1_DEFAULT                                     0x00000000
#define IPE_NXT_GPR0_ADDR                                            0x00000148
#define IPE_NXT_GPR0_DEFAULT                                         0x00000000
#define IPE_NXT_GPR1_ADDR                                            0x0000014c
#define IPE_NXT_GPR1_DEFAULT                                         0x00000000
#define IPE_NXT_IV0_ADDR                                             0x00000150
#define IPE_NXT_IV0_DEFAULT                                          0x00000000
#define IPE_NXT_IV1_ADDR                                             0x00000154
#define IPE_NXT_IV1_DEFAULT                                          0x00000000
#define IPE_NXT_IV2_ADDR                                             0x00000158
#define IPE_NXT_IV2_DEFAULT                                          0x00000000
#define IPE_NXT_IV3_ADDR                                             0x0000015c
#define IPE_NXT_IV3_DEFAULT                                          0x00000000
#define IPE_NXT_KEY0_ADDR                                            0x00000160
#define IPE_NXT_KEY0_DEFAULT                                         0x00000000
#define IPE_NXT_KEY1_ADDR                                            0x00000164
#define IPE_NXT_KEY1_DEFAULT                                         0x00000000
#define IPE_NXT_KEY2_ADDR                                            0x00000168
#define IPE_NXT_KEY2_DEFAULT                                         0x00000000
#define IPE_NXT_KEY3_ADDR                                            0x0000016c
#define IPE_NXT_KEY3_DEFAULT                                         0x00000000
#define IPE_NXT_KEY4_ADDR                                            0x00000170
#define IPE_NXT_KEY4_DEFAULT                                         0x00000000
#define IPE_NXT_KEY5_ADDR                                            0x00000174
#define IPE_NXT_KEY5_DEFAULT                                         0x00000000
#define IPE_NXT_KEY6_ADDR                                            0x00000178
#define IPE_NXT_KEY6_DEFAULT                                         0x00000000
#define IPE_NXT_KEY7_ADDR                                            0x0000017c
#define IPE_NXT_KEY7_DEFAULT                                         0x00000000
#define IPE_NXT_IN_DGST0_ADDR                                        0x00000180
#define IPE_NXT_IN_DGST0_DEFAULT                                     0x00000000
#define IPE_NXT_IN_DGST1_ADDR                                        0x00000184
#define IPE_NXT_IN_DGST1_DEFAULT                                     0x00000000
#define IPE_NXT_IN_DGST2_ADDR                                        0x00000188
#define IPE_NXT_IN_DGST2_DEFAULT                                     0x00000000
#define IPE_NXT_IN_DGST3_ADDR                                        0x0000018c
#define IPE_NXT_IN_DGST3_DEFAULT                                     0x00000000
#define IPE_NXT_IN_DGST4_ADDR                                        0x00000190
#define IPE_NXT_IN_DGST4_DEFAULT                                     0x00000000
#define IPE_NXT_IN_DGST5_ADDR                                        0x00000194
#define IPE_NXT_IN_DGST5_DEFAULT                                     0x00000000
#define IPE_NXT_IN_DGST6_ADDR                                        0x00000198
#define IPE_NXT_IN_DGST6_DEFAULT                                     0x00000000
#define IPE_NXT_IN_DGST7_ADDR                                        0x0000019c
#define IPE_NXT_IN_DGST7_DEFAULT                                     0x00000000
#define IPE_NXT_OUT_DGST0_ADDR                                       0x000001a0
#define IPE_NXT_OUT_DGST0_DEFAULT                                    0x00000000
#define IPE_NXT_OUT_DGST1_ADDR                                       0x000001a4
#define IPE_NXT_OUT_DGST1_DEFAULT                                    0x00000000
#define IPE_NXT_OUT_DGST2_ADDR                                       0x000001a8
#define IPE_NXT_OUT_DGST2_DEFAULT                                    0x00000000
#define IPE_NXT_OUT_DGST3_ADDR                                       0x000001ac
#define IPE_NXT_OUT_DGST3_DEFAULT                                    0x00000000
#define IPE_NXT_OUT_DGST4_ADDR                                       0x000001b0
#define IPE_NXT_OUT_DGST4_DEFAULT                                    0x00000000
#define IPE_NXT_OUT_DGST5_ADDR                                       0x000001b4
#define IPE_NXT_OUT_DGST5_DEFAULT                                    0x00000000
#define IPE_NXT_OUT_DGST6_ADDR                                       0x000001b8
#define IPE_NXT_OUT_DGST6_DEFAULT                                    0x00000000
#define IPE_NXT_OUT_DGST7_ADDR                                       0x000001bc
#define IPE_NXT_OUT_DGST7_DEFAULT                                    0x00000000
#define IPE_NXT_DGST_CNT_ADDR                                        0x000001c0
#define IPE_NXT_DGST_CNT_DEFAULT                                     0x00000000
#define IPE_NXT_SPI_SSRC_ADDR                                        0x000001c4
#define IPE_NXT_SPI_SSRC_DEFAULT                                     0x00000000
#define IPE_NXT_SN_ADDR                                              0x000001c8
#define IPE_NXT_SN_DEFAULT                                           0x00000000
#define IPE_NXT_ESN_ADDR                                             0x000001cc
#define IPE_NXT_ESN_DEFAULT                                          0x00000000
#define IPE_NXT_SN_M0_ADDR                                           0x000001d0
#define IPE_NXT_SN_M0_DEFAULT                                        0x00000000
#define IPE_NXT_SN_M1_ADDR                                           0x000001d4
#define IPE_NXT_SN_M1_DEFAULT                                        0x00000000
#define IPE_NXT_SN_M2_ADDR                                           0x000001d8
#define IPE_NXT_SN_M2_DEFAULT                                        0x00000000
#define IPE_NXT_SN_M3_ADDR                                           0x000001dc
#define IPE_NXT_SN_M3_DEFAULT                                        0x00000000
#define IPE_NXT_CS_ADDR                                              0x000001e4
#define IPE_NXT_CS_DEFAULT                                           0x00000000
#define IPE_NXT_UP_PKT_LEN_ADDR                                      0x000001e8
#define IPE_NXT_UP_PKT_LEN_DEFAULT                                   0x00000000
#define IPE_ACT_CTX_CMD0_ADDR                                        0x00000200
#define IPE_ACT_CTX_CMD0_DEFAULT                                     0x00000000
#define IPE_ACT_CTX_CMD1_ADDR                                        0x00000204
#define IPE_ACT_CTX_CMD1_DEFAULT                                     0x00000000
#define IPE_ACT_GPR0_ADDR                                            0x00000208
#define IPE_ACT_GPR0_DEFAULT                                         0x00000000
#define IPE_ACT_GPR1_ADDR                                            0x0000020c
#define IPE_ACT_GPR1_DEFAULT                                         0x00000000
#define IPE_IV0_ADDR                                                 0x00000210
#define IPE_IV0_DEFAULT                                              0x00000000
#define IPE_IV1_ADDR                                                 0x00000214
#define IPE_IV1_DEFAULT                                              0x00000000
#define IPE_IV2_ADDR                                                 0x00000218
#define IPE_IV2_DEFAULT                                              0x00000000
#define IPE_IV3_ADDR                                                 0x0000021c
#define IPE_IV3_DEFAULT                                              0x00000000
#define IPE_KEY0_ADDR                                                0x00000220
#define IPE_KEY0_DEFAULT                                             0x00000000
#define IPE_KEY1_ADDR                                                0x00000224
#define IPE_KEY1_DEFAULT                                             0x00000000
#define IPE_KEY2_ADDR                                                0x00000228
#define IPE_KEY2_DEFAULT                                             0x00000000
#define IPE_KEY3_ADDR                                                0x0000022c
#define IPE_KEY3_DEFAULT                                             0x00000000
#define IPE_KEY4_ADDR                                                0x00000230
#define IPE_KEY4_DEFAULT                                             0x00000000
#define IPE_KEY5_ADDR                                                0x00000234
#define IPE_KEY5_DEFAULT                                             0x00000000
#define IPE_KEY6_ADDR                                                0x00000238
#define IPE_KEY6_DEFAULT                                             0x00000000
#define IPE_KEY7_ADDR                                                0x0000023c
#define IPE_KEY7_DEFAULT                                             0x00000000
#define IPE_IN_DGST0_ADDR                                            0x00000240
#define IPE_IN_DGST0_DEFAULT                                         0x00000000
#define IPE_IN_DGST1_ADDR                                            0x00000244
#define IPE_IN_DGST1_DEFAULT                                         0x00000000
#define IPE_IN_DGST2_ADDR                                            0x00000248
#define IPE_IN_DGST2_DEFAULT                                         0x00000000
#define IPE_IN_DGST3_ADDR                                            0x0000024c
#define IPE_IN_DGST3_DEFAULT                                         0x00000000
#define IPE_IN_DGST4_ADDR                                            0x00000250
#define IPE_IN_DGST4_DEFAULT                                         0x00000000
#define IPE_IN_DGST5_ADDR                                            0x00000254
#define IPE_IN_DGST5_DEFAULT                                         0x00000000
#define IPE_IN_DGST6_ADDR                                            0x00000258
#define IPE_IN_DGST6_DEFAULT                                         0x00000000
#define IPE_IN_DGST7_ADDR                                            0x0000025c
#define IPE_IN_DGST7_DEFAULT                                         0x00000000
#define IPE_OUT_DGST0_ADDR                                           0x00000260
#define IPE_OUT_DGST0_DEFAULT                                        0x00000000
#define IPE_OUT_DGST1_ADDR                                           0x00000264
#define IPE_OUT_DGST1_DEFAULT                                        0x00000000
#define IPE_OUT_DGST2_ADDR                                           0x00000268
#define IPE_OUT_DGST2_DEFAULT                                        0x00000000
#define IPE_OUT_DGST3_ADDR                                           0x0000026c
#define IPE_OUT_DGST3_DEFAULT                                        0x00000000
#define IPE_OUT_DGST4_ADDR                                           0x00000270
#define IPE_OUT_DGST4_DEFAULT                                        0x00000000
#define IPE_OUT_DGST5_ADDR                                           0x00000274
#define IPE_OUT_DGST5_DEFAULT                                        0x00000000
#define IPE_OUT_DGST6_ADDR                                           0x00000278
#define IPE_OUT_DGST6_DEFAULT                                        0x00000000
#define IPE_OUT_DGST7_ADDR                                           0x0000027c
#define IPE_OUT_DGST7_DEFAULT                                        0x00000000
#define IPE_DGST_CNT_ADDR                                            0x00000280
#define IPE_DGST_CNT_DEFAULT                                         0x00000000
#define IPE_SPI_SSRC_ADDR                                            0x00000284
#define IPE_SPI_SSRC_DEFAULT                                         0x00000000
#define IPE_SN_ADDR                                                  0x00000288
#define IPE_SN_DEFAULT                                               0x00000000
#define IPE_ESN_ADDR                                                 0x0000028c
#define IPE_ESN_DEFAULT                                              0x00000000
#define IPE_SN_M0_ADDR                                               0x00000290
#define IPE_SN_M0_DEFAULT                                            0x00000000
#define IPE_SN_M1_ADDR                                               0x00000294
#define IPE_SN_M1_DEFAULT                                            0x00000000
#define IPE_SN_M2_ADDR                                               0x00000298
#define IPE_SN_M2_DEFAULT                                            0x00000000
#define IPE_SN_M3_ADDR                                               0x0000029c
#define IPE_SN_M3_DEFAULT                                            0x00000000
#define IPE_CS_ADDR                                                  0x000002a4
#define IPE_CS_DEFAULT                                               0x00000000
#define IPE_UP_PKT_LEN_ADDR                                          0x000002a8
#define IPE_UP_PKT_LEN_DEFAULT                                       0x00000000
#define IPE_HASH_RES0_ADDR                                           0x000002c0
#define IPE_HASH_RES0_DEFAULT                                        0x00000000
#define IPE_HASH_RES1_ADDR                                           0x000002c4
#define IPE_HASH_RES1_DEFAULT                                        0x00000000
#define IPE_HASH_RES2_ADDR                                           0x000002c8
#define IPE_HASH_RES2_DEFAULT                                        0x00000000
#define IPE_HASH_RES3_ADDR                                           0x000002cc
#define IPE_HASH_RES3_DEFAULT                                        0x00000000
#define IPE_HASH_RES4_ADDR                                           0x000002d0
#define IPE_HASH_RES4_DEFAULT                                        0x00000000
#define IPE_HASH_RES5_ADDR                                           0x000002d4
#define IPE_HASH_RES5_DEFAULT                                        0x00000000
#define IPE_HASH_RES6_ADDR                                           0x000002d8
#define IPE_HASH_RES6_DEFAULT                                        0x00000000
#define IPE_HASH_RES7_ADDR                                           0x000002dc
#define IPE_HASH_RES7_DEFAULT                                        0x00000000
#define IPE_DGST_CNT_RES_ADDR                                        0x000002e0
#define IPE_DGST_CNT_RES_DEFAULT                                     0x00000000
#define IPE_SPI_RTVD_ADDR                                            0x000002e4
#define IPE_SPI_RTVD_DEFAULT                                         0x00000000
#define IPE_SN_RTVD_ADDR                                             0x000002e8
#define IPE_SN_RTVD_DEFAULT                                          0x00000000
#define IPE_ESN_CALC_ADDR                                            0x000002ec
#define IPE_ESN_CALC_DEFAULT                                         0x00000000
#define IPE_CS_RES_ADDR                                              0x000002f0
#define IPE_CS_RES_DEFAULT                                           0x00000000
#define IPE_NXT_IN_DGST8_ADDR                                        0x00000300
#define IPE_NXT_IN_DGST8_DEFAULT                                     0x00000000
#define IPE_NXT_IN_DGST9_ADDR                                        0x00000304
#define IPE_NXT_IN_DGST9_DEFAULT                                     0x00000000
#define IPE_NXT_IN_DGST10_ADDR                                       0x00000308
#define IPE_NXT_IN_DGST10_DEFAULT                                    0x00000000
#define IPE_NXT_IN_DGST11_ADDR                                       0x0000030c
#define IPE_NXT_IN_DGST11_DEFAULT                                    0x00000000
#define IPE_NXT_IN_DGST12_ADDR                                       0x00000310
#define IPE_NXT_IN_DGST12_DEFAULT                                    0x00000000
#define IPE_NXT_IN_DGST13_ADDR                                       0x00000314
#define IPE_NXT_IN_DGST13_DEFAULT                                    0x00000000
#define IPE_NXT_IN_DGST14_ADDR                                       0x00000318
#define IPE_NXT_IN_DGST14_DEFAULT                                    0x00000000
#define IPE_NXT_IN_DGST15_ADDR                                       0x0000031c
#define IPE_NXT_IN_DGST15_DEFAULT                                    0x00000000
#define IPE_NXT_OUT_DGST8_ADDR                                       0x00000320
#define IPE_NXT_OUT_DGST8_DEFAULT                                    0x00000000
#define IPE_NXT_OUT_DGST9_ADDR                                       0x00000324
#define IPE_NXT_OUT_DGST9_DEFAULT                                    0x00000000
#define IPE_NXT_OUT_DGST10_ADDR                                      0x00000328
#define IPE_NXT_OUT_DGST10_DEFAULT                                   0x00000000
#define IPE_NXT_OUT_DGST11_ADDR                                      0x0000032c
#define IPE_NXT_OUT_DGST11_DEFAULT                                   0x00000000
#define IPE_NXT_OUT_DGST12_ADDR                                      0x00000330
#define IPE_NXT_OUT_DGST12_DEFAULT                                   0x00000000
#define IPE_NXT_OUT_DGST13_ADDR                                      0x00000334
#define IPE_NXT_OUT_DGST13_DEFAULT                                   0x00000000
#define IPE_NXT_OUT_DGST14_ADDR                                      0x00000338
#define IPE_NXT_OUT_DGST14_DEFAULT                                   0x00000000
#define IPE_NXT_OUT_DGST15_ADDR                                      0x0000033c
#define IPE_NXT_OUT_DGST15_DEFAULT                                   0x00000000
#define IPE_IN_DGST8_ADDR                                            0x00000340
#define IPE_IN_DGST8_DEFAULT                                         0x00000000
#define IPE_IN_DGST9_ADDR                                            0x00000344
#define IPE_IN_DGST9_DEFAULT                                         0x00000000
#define IPE_IN_DGST10_ADDR                                           0x00000348
#define IPE_IN_DGST10_DEFAULT                                        0x00000000
#define IPE_IN_DGST11_ADDR                                           0x0000034c
#define IPE_IN_DGST11_DEFAULT                                        0x00000000
#define IPE_IN_DGST12_ADDR                                           0x00000350
#define IPE_IN_DGST12_DEFAULT                                        0x00000000
#define IPE_IN_DGST13_ADDR                                           0x00000354
#define IPE_IN_DGST13_DEFAULT                                        0x00000000
#define IPE_IN_DGST14_ADDR                                           0x00000358
#define IPE_IN_DGST14_DEFAULT                                        0x00000000
#define IPE_IN_DGST15_ADDR                                           0x0000035c
#define IPE_IN_DGST15_DEFAULT                                        0x00000000
#define IPE_OUT_DGST8_ADDR                                           0x00000360
#define IPE_OUT_DGST8_DEFAULT                                        0x00000000
#define IPE_OUT_DGST9_ADDR                                           0x00000364
#define IPE_OUT_DGST9_DEFAULT                                        0x00000000
#define IPE_OUT_DGST10_ADDR                                          0x00000368
#define IPE_OUT_DGST10_DEFAULT                                       0x00000000
#define IPE_OUT_DGST11_ADDR                                          0x0000036c
#define IPE_OUT_DGST11_DEFAULT                                       0x00000000
#define IPE_OUT_DGST12_ADDR                                          0x00000370
#define IPE_OUT_DGST12_DEFAULT                                       0x00000000
#define IPE_OUT_DGST13_ADDR                                          0x00000374
#define IPE_OUT_DGST13_DEFAULT                                       0x00000000
#define IPE_OUT_DGST14_ADDR                                          0x00000378
#define IPE_OUT_DGST14_DEFAULT                                       0x00000000
#define IPE_OUT_DGST15_ADDR                                          0x0000037c
#define IPE_OUT_DGST15_DEFAULT                                       0x00000000
#define IPE_HASH_RES8_ADDR                                           0x00000380
#define IPE_HASH_RES8_DEFAULT                                        0x00000000
#define IPE_HASH_RES9_ADDR                                           0x00000384
#define IPE_HASH_RES9_DEFAULT                                        0x00000000
#define IPE_HASH_RES10_ADDR                                          0x00000388
#define IPE_HASH_RES10_DEFAULT                                       0x00000000
#define IPE_HASH_RES11_ADDR                                          0x0000038c
#define IPE_HASH_RES11_DEFAULT                                       0x00000000
#define IPE_HASH_RES12_ADDR                                          0x00000390
#define IPE_HASH_RES12_DEFAULT                                       0x00000000
#define IPE_HASH_RES13_ADDR                                          0x00000394
#define IPE_HASH_RES13_DEFAULT                                       0x00000000
#define IPE_HASH_RES14_ADDR                                          0x00000398
#define IPE_HASH_RES14_DEFAULT                                       0x00000000
#define IPE_HASH_RES15_ADDR                                          0x0000039c
#define IPE_HASH_RES15_DEFAULT                                       0x00000000

/*	EIP62_CORE Register	*/
#define SEC_IPE_SW_INT_ADDR					     0x00000014
#define SEC_IPE_SW_INT_DEFAULT					     0x00000000
#define SEC_IPE_SN_THRESHOLD_ADDR				     0x00000020
#define SEC_IPE_SN_THRESHOLD_DEFAULT				     0x00000000
#define SEC_IPE_BLK_CTX_UPDT_ADDR				     0x00000030
#define SEC_IPE_BLK_CTX_UPDT_DEFAULT				     0x00000000
#define SEC_IPE_DEV_INFO_TYPE_ADDR				     0x000003f8
#define SEC_IPE_DEV_INFO_TYPE_DEFAULT				     0x422dda14
#define SEC_IPE_DEV_INFO_VERSION_ADDR				     0x000003fc
#define SEC_IPE_DEV_INFO_VERSION_DEFAULT			     0x0143c13e

/*	Register IPE_TKN_CTRL_STAT	*/
/*	 Fields Hold_processing	 */
#define HOLD_PROCESSING_WIDTH                                                 1
#define HOLD_PROCESSING_SHIFT                                                31
#define HOLD_PROCESSING_MASK                                         0x80000000
#define HOLD_PROCESSING_RD(src)                      (((src) & 0x80000000)>>31)
#define HOLD_PROCESSING_WR(src)                 (((u32)(src)<<31) & 0x80000000)
#define HOLD_PROCESSING_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields rsvd2	 */
#define RSVD2_WIDTH                                                           1
#define RSVD2_SHIFT                                                          30
#define RSVD2_MASK                                                   0x40000000
#define RSVD2_RD(src)                                (((src) & 0x40000000)>>30)
#define RSVD2_WR(src)                           (((u32)(src)<<30) & 0x40000000)
#define RSVD2_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields Process_N_packets	 */
#define PROCESS_N_PACKETS_WIDTH                                               6
#define PROCESS_N_PACKETS_SHIFT                                              24
#define PROCESS_N_PACKETS_MASK                                       0x3f000000
#define PROCESS_N_PACKETS_RD(src)                    (((src) & 0x3f000000)>>24)
#define PROCESS_N_PACKETS_WR(src)               (((u32)(src)<<24) & 0x3f000000)
#define PROCESS_N_PACKETS_SET(dst,src) \
                      (((dst) & ~0x3f000000) | (((u32)(src)<<24) & 0x3f000000))
/*	 Fields Debug_mode	 */
#define SEC_DEBUG_MODE_WIDTH                                                  1
#define SEC_DEBUG_MODE_SHIFT                                                 23
#define SEC_DEBUG_MODE_MASK                                          0x00800000
#define SEC_DEBUG_MODE_RD(src)                       (((src) & 0x00800000)>>23)
#define SEC_DEBUG_MODE_WR(src)                  (((u32)(src)<<23) & 0x00800000)
#define SEC_DEBUG_MODE_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields Time_out_counter_enable	 */
#define TIME_OUT_COUNTER_ENABLE_WIDTH                                         1
#define TIME_OUT_COUNTER_ENABLE_SHIFT                                        22
#define TIME_OUT_COUNTER_ENABLE_MASK                                 0x00400000
#define TIME_OUT_COUNTER_ENABLE_RD(src)              (((src) & 0x00400000)>>22)
#define TIME_OUT_COUNTER_ENABLE_WR(src)         (((u32)(src)<<22) & 0x00400000)
#define TIME_OUT_COUNTER_ENABLE_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields rsvd1	 */
#define RSVD1_WIDTH                                                           4
#define RSVD1_SHIFT                                                          18
#define RSVD1_MASK                                                   0x003c0000
#define RSVD1_RD(src)                                (((src) & 0x003c0000)>>18)
#define RSVD1_WR(src)                           (((u32)(src)<<18) & 0x003c0000)
#define RSVD1_SET(dst,src) \
                      (((dst) & ~0x003c0000) | (((u32)(src)<<18) & 0x003c0000))
/*	 Fields Interrupt_pulse_or_level	 */
#define INTERRUPT_PULSE_OR_LEVEL_WIDTH                                        1
#define INTERRUPT_PULSE_OR_LEVEL_SHIFT                                       17
#define INTERRUPT_PULSE_OR_LEVEL_MASK                                0x00020000
#define INTERRUPT_PULSE_OR_LEVEL_RD(src)             (((src) & 0x00020000)>>17)
#define INTERRUPT_PULSE_OR_LEVEL_WR(src)        (((u32)(src)<<17) & 0x00020000)
#define INTERRUPT_PULSE_OR_LEVEL_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields Optimal_context_updates	 */
#define OPTIMAL_CONTEXT_UPDATES_WIDTH                                         1
#define OPTIMAL_CONTEXT_UPDATES_SHIFT                                        16
#define OPTIMAL_CONTEXT_UPDATES_MASK                                 0x00010000
#define OPTIMAL_CONTEXT_UPDATES_RD(src)              (((src) & 0x00010000)>>16)
#define OPTIMAL_CONTEXT_UPDATES_WR(src)         (((u32)(src)<<16) & 0x00010000)
#define OPTIMAL_CONTEXT_UPDATES_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields Busy	 */
#define EIP96_BUSY_WIDTH                                                            1
#define EIP96_BUSY_SHIFT                                                           15
#define EIP96_BUSY_MASK                                                    0x00008000
#define EIP96_BUSY_RD(src)                                 (((src) & 0x00008000)>>15)
#define EIP96_BUSY_WR(src)                            (((u32)(src)<<15) & 0x00008000)
#define EIP96_BUSY_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields Processing_held_IDLE	 */
#define PROCESSING_HELD_IDLE_WIDTH                                            1
#define PROCESSING_HELD_IDLE_SHIFT                                           14
#define PROCESSING_HELD_IDLE_MASK                                    0x00004000
#define PROCESSING_HELD_IDLE_RD(src)                 (((src) & 0x00004000)>>14)
#define PROCESSING_HELD_IDLE_WR(src)            (((u32)(src)<<14) & 0x00004000)
#define PROCESSING_HELD_IDLE_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields Packets_to_be_processed	 */
#define PACKETS_TO_BE_PROCESSED_WIDTH                                         6
#define PACKETS_TO_BE_PROCESSED_SHIFT                                         8
#define PACKETS_TO_BE_PROCESSED_MASK                                 0x00003f00
#define PACKETS_TO_BE_PROCESSED_RD(src)               (((src) & 0x00003f00)>>8)
#define PACKETS_TO_BE_PROCESSED_WR(src)          (((u32)(src)<<8) & 0x00003f00)
#define PACKETS_TO_BE_PROCESSED_SET(dst,src) \
                       (((dst) & ~0x00003f00) | (((u32)(src)<<8) & 0x00003f00))
/*	 Fields Result_context	 */
#define RESULT_CONTEXT_WIDTH                                                  1
#define RESULT_CONTEXT_SHIFT                                                  7
#define RESULT_CONTEXT_MASK                                          0x00000080
#define RESULT_CONTEXT_RD(src)                        (((src) & 0x00000080)>>7)
#define RESULT_CONTEXT_WR(src)                   (((u32)(src)<<7) & 0x00000080)
#define RESULT_CONTEXT_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields Context_fetch	 */
#define CONTEXT_FETCH_WIDTH                                                   1
#define CONTEXT_FETCH_SHIFT                                                   6
#define CONTEXT_FETCH_MASK                                           0x00000040
#define CONTEXT_FETCH_RD(src)                         (((src) & 0x00000040)>>6)
#define CONTEXT_FETCH_WR(src)                    (((u32)(src)<<6) & 0x00000040)
#define CONTEXT_FETCH_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields Context_cache_avctive	 */
#define CONTEXT_CACHE_AVCTIVE_WIDTH                                           1
#define CONTEXT_CACHE_AVCTIVE_SHIFT                                           5
#define CONTEXT_CACHE_AVCTIVE_MASK                                   0x00000020
#define CONTEXT_CACHE_AVCTIVE_RD(src)                 (((src) & 0x00000020)>>5)
#define CONTEXT_CACHE_AVCTIVE_WR(src)            (((u32)(src)<<5) & 0x00000020)
#define CONTEXT_CACHE_AVCTIVE_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields Token_read_active	 */
#define TOKEN_READ_ACTIVE_WIDTH                                               1
#define TOKEN_READ_ACTIVE_SHIFT                                               4
#define TOKEN_READ_ACTIVE_MASK                                       0x00000010
#define TOKEN_READ_ACTIVE_RD(src)                     (((src) & 0x00000010)>>4)
#define TOKEN_READ_ACTIVE_WR(src)                (((u32)(src)<<4) & 0x00000010)
#define TOKEN_READ_ACTIVE_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields Result_token_available	 */
#define RESULT_TOKEN_AVAILABLE_WIDTH                                          1
#define RESULT_TOKEN_AVAILABLE_SHIFT                                          3
#define RESULT_TOKEN_AVAILABLE_MASK                                  0x00000008
#define RESULT_TOKEN_AVAILABLE_RD(src)                (((src) & 0x00000008)>>3)
#define RESULT_TOKEN_AVAILABLE_WR(src)           (((u32)(src)<<3) & 0x00000008)
#define RESULT_TOKEN_AVAILABLE_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields Token_location_available	 */
#define TOKEN_LOCATION_AVAILABLE_WIDTH                                        1
#define TOKEN_LOCATION_AVAILABLE_SHIFT                                        2
#define TOKEN_LOCATION_AVAILABLE_MASK                                0x00000004
#define TOKEN_LOCATION_AVAILABLE_RD(src)              (((src) & 0x00000004)>>2)
#define TOKEN_LOCATION_AVAILABLE_WR(src)         (((u32)(src)<<2) & 0x00000004)
#define TOKEN_LOCATION_AVAILABLE_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields Active_tokens	 */
#define ACTIVE_TOKENS_WIDTH                                                   2
#define ACTIVE_TOKENS_SHIFT                                                   0
#define ACTIVE_TOKENS_MASK                                           0x00000003
#define ACTIVE_TOKENS_RD(src)                            (((src) & 0x00000003))
#define ACTIVE_TOKENS_WR(src)                       (((u32)(src)) & 0x00000003)
#define ACTIVE_TOKENS_SET(dst,src) \
                          (((dst) & ~0x00000003) | (((u32)(src)) & 0x00000003))

/*	Register IPE_PRC_ALG_EN	*/
/*	 Fields GCM_HASH	 */
#define GCM_HASH_WIDTH                                                        1
#define GCM_HASH_SHIFT                                                       31
#define GCM_HASH_MASK                                                0x80000000
#define GCM_HASH_RD(src)                             (((src) & 0x80000000)>>31)
#define GCM_HASH_WR(src)                        (((u32)(src)<<31) & 0x80000000)
#define GCM_HASH_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields AES_XCBC_MAC	 */
#define AES_XCBC_MAC_WIDTH                                                    1
#define AES_XCBC_MAC_SHIFT                                                   30
#define AES_XCBC_MAC_MASK                                            0x40000000
#define AES_XCBC_MAC_RD(src)                         (((src) & 0x40000000)>>30)
#define AES_XCBC_MAC_WR(src)                    (((u32)(src)<<30) & 0x40000000)
#define AES_XCBC_MAC_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields HMAC_SHA2	 */
#define HMAC_SHA2_WIDTH                                                       1
#define HMAC_SHA2_SHIFT                                                      29
#define HMAC_SHA2_MASK                                               0x20000000
#define HMAC_SHA2_RD(src)                            (((src) & 0x20000000)>>29)
#define HMAC_SHA2_WR(src)                       (((u32)(src)<<29) & 0x20000000)
#define HMAC_SHA2_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields basic_SHA2	 */
#define BASIC_SHA2_WIDTH                                                      1
#define BASIC_SHA2_SHIFT                                                     28
#define BASIC_SHA2_MASK                                              0x10000000
#define BASIC_SHA2_RD(src)                           (((src) & 0x10000000)>>28)
#define BASIC_SHA2_WR(src)                      (((u32)(src)<<28) & 0x10000000)
#define BASIC_SHA2_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields HMAC_SHA1	 */
#define HMAC_SHA1_WIDTH                                                       1
#define HMAC_SHA1_SHIFT                                                      27
#define HMAC_SHA1_MASK                                               0x08000000
#define HMAC_SHA1_RD(src)                            (((src) & 0x08000000)>>27)
#define HMAC_SHA1_WR(src)                       (((u32)(src)<<27) & 0x08000000)
#define HMAC_SHA1_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields basic_SHA1	 */
#define BASIC_SHA1_WIDTH                                                      1
#define BASIC_SHA1_SHIFT                                                     26
#define BASIC_SHA1_MASK                                              0x04000000
#define BASIC_SHA1_RD(src)                           (((src) & 0x04000000)>>26)
#define BASIC_SHA1_WR(src)                      (((u32)(src)<<26) & 0x04000000)
#define BASIC_SHA1_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields HMAC_MD5	 */
#define HMAC_MD5_WIDTH                                                        1
#define HMAC_MD5_SHIFT                                                       25
#define HMAC_MD5_MASK                                                0x02000000
#define HMAC_MD5_RD(src)                             (((src) & 0x02000000)>>25)
#define HMAC_MD5_WR(src)                        (((u32)(src)<<25) & 0x02000000)
#define HMAC_MD5_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields basic_MD5	 */
#define BASIC_MD5_WIDTH                                                       1
#define BASIC_MD5_SHIFT                                                      24
#define BASIC_MD5_MASK                                               0x01000000
#define BASIC_MD5_RD(src)                            (((src) & 0x01000000)>>24)
#define BASIC_MD5_WR(src)                       (((u32)(src)<<24) & 0x01000000)
#define BASIC_MD5_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*	 Fields Kasumi_f9	 */
#define KASUMI_F9_WIDTH                                                       1
#define KASUMI_F9_SHIFT                                                      23
#define KASUMI_F9_MASK                                               0x00800000
#define KASUMI_F9_RD(src)                            (((src) & 0x00800000)>>23)
#define KASUMI_F9_WR(src)                       (((u32)(src)<<23) & 0x00800000)
#define KASUMI_F9_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields DES3_CFB	 */
#define DES3_CFB_WIDTH                                                        1
#define DES3_CFB_SHIFT                                                       22
#define DES3_CFB_MASK                                                0x00400000
#define DES3_CFB_RD(src)                             (((src) & 0x00400000)>>22)
#define DES3_CFB_WR(src)                        (((u32)(src)<<22) & 0x00400000)
#define DES3_CFB_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields DES3_OFB	 */
#define DES3_OFB_WIDTH                                                        1
#define DES3_OFB_SHIFT                                                       21
#define DES3_OFB_MASK                                                0x00200000
#define DES3_OFB_RD(src)                             (((src) & 0x00200000)>>21)
#define DES3_OFB_WR(src)                        (((u32)(src)<<21) & 0x00200000)
#define DES3_OFB_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields reserved2	 */
#define RESERVED2_WIDTH                                                       1
#define RESERVED2_SHIFT                                                      20
#define RESERVED2_MASK                                               0x00100000
#define RESERVED2_RD(src)                            (((src) & 0x00100000)>>20)
#define RESERVED2_WR(src)                       (((u32)(src)<<20) & 0x00100000)
#define RESERVED2_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields DES3_CBC	 */
#define DES3_CBC_WIDTH                                                        1
#define DES3_CBC_SHIFT                                                       19
#define DES3_CBC_MASK                                                0x00080000
#define DES3_CBC_RD(src)                             (((src) & 0x00080000)>>19)
#define DES3_CBC_WR(src)                        (((u32)(src)<<19) & 0x00080000)
#define DES3_CBC_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields DES3_ECB	 */
#define DES3_ECB_WIDTH                                                        1
#define DES3_ECB_SHIFT                                                       18
#define DES3_ECB_MASK                                                0x00040000
#define DES3_ECB_RD(src)                             (((src) & 0x00040000)>>18)
#define DES3_ECB_WR(src)                        (((u32)(src)<<18) & 0x00040000)
#define DES3_ECB_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields DES_CFB	 */
#define DES_CFB_WIDTH                                                         1
#define DES_CFB_SHIFT                                                        17
#define DES_CFB_MASK                                                 0x00020000
#define DES_CFB_RD(src)                              (((src) & 0x00020000)>>17)
#define DES_CFB_WR(src)                         (((u32)(src)<<17) & 0x00020000)
#define DES_CFB_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields DES_OFB	 */
#define DES_OFB_WIDTH                                                         1
#define DES_OFB_SHIFT                                                        16
#define DES_OFB_MASK                                                 0x00010000
#define DES_OFB_RD(src)                              (((src) & 0x00010000)>>16)
#define DES_OFB_WR(src)                         (((u32)(src)<<16) & 0x00010000)
#define DES_OFB_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields reserved1	 */
#define RESERVED1_WIDTH                                                       1
#define RESERVED1_SHIFT                                                      15
#define RESERVED1_MASK                                               0x00008000
#define RESERVED1_RD(src)                            (((src) & 0x00008000)>>15)
#define RESERVED1_WR(src)                       (((u32)(src)<<15) & 0x00008000)
#define RESERVED1_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields DES_CBC	 */
#define DES_CBC_WIDTH                                                         1
#define DES_CBC_SHIFT                                                        14
#define DES_CBC_MASK                                                 0x00004000
#define DES_CBC_RD(src)                              (((src) & 0x00004000)>>14)
#define DES_CBC_WR(src)                         (((u32)(src)<<14) & 0x00004000)
#define DES_CBC_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields DES_ECB	 */
#define DES_ECB_WIDTH                                                         1
#define DES_ECB_SHIFT                                                        13
#define DES_ECB_MASK                                                 0x00002000
#define DES_ECB_RD(src)                              (((src) & 0x00002000)>>13)
#define DES_ECB_WR(src)                         (((u32)(src)<<13) & 0x00002000)
#define DES_ECB_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields AES_CFB	 */
#define AES_CFB_WIDTH                                                         1
#define AES_CFB_SHIFT                                                        12
#define AES_CFB_MASK                                                 0x00001000
#define AES_CFB_RD(src)                              (((src) & 0x00001000)>>12)
#define AES_CFB_WR(src)                         (((u32)(src)<<12) & 0x00001000)
#define AES_CFB_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields AES_OFB	 */
#define AES_OFB_WIDTH                                                         1
#define AES_OFB_SHIFT                                                        11
#define AES_OFB_MASK                                                 0x00000800
#define AES_OFB_RD(src)                              (((src) & 0x00000800)>>11)
#define AES_OFB_WR(src)                         (((u32)(src)<<11) & 0x00000800)
#define AES_OFB_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields AES_CTR_ICM	 */
#define AES_CTR_ICM_WIDTH                                                     1
#define AES_CTR_ICM_SHIFT                                                    10
#define AES_CTR_ICM_MASK                                             0x00000400
#define AES_CTR_ICM_RD(src)                          (((src) & 0x00000400)>>10)
#define AES_CTR_ICM_WR(src)                     (((u32)(src)<<10) & 0x00000400)
#define AES_CTR_ICM_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields AES_CBC	 */
#define AES_CBC_WIDTH                                                         1
#define AES_CBC_SHIFT                                                         9
#define AES_CBC_MASK                                                 0x00000200
#define AES_CBC_RD(src)                               (((src) & 0x00000200)>>9)
#define AES_CBC_WR(src)                          (((u32)(src)<<9) & 0x00000200)
#define AES_CBC_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields AES_EBC	 */
#define AES_EBC_WIDTH                                                         1
#define AES_EBC_SHIFT                                                         8
#define AES_EBC_MASK                                                 0x00000100
#define AES_EBC_RD(src)                               (((src) & 0x00000100)>>8)
#define AES_EBC_WR(src)                          (((u32)(src)<<8) & 0x00000100)
#define AES_EBC_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields ARC4	 */
#define ARC4_WIDTH                                                            1
#define ARC4_SHIFT                                                            7
#define ARC4_MASK                                                    0x00000080
#define ARC4_RD(src)                                  (((src) & 0x00000080)>>7)
#define ARC4_WR(src)                             (((u32)(src)<<7) & 0x00000080)
#define ARC4_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields Kasumi_Kasumi_f8	 */
#define KASUMI_KASUMI_F8_WIDTH                                                1
#define KASUMI_KASUMI_F8_SHIFT                                                6
#define KASUMI_KASUMI_F8_MASK                                        0x00000040
#define KASUMI_KASUMI_F8_RD(src)                      (((src) & 0x00000040)>>6)
#define KASUMI_KASUMI_F8_WR(src)                 (((u32)(src)<<6) & 0x00000040)
#define KASUMI_KASUMI_F8_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields decrypt_hash	 */
#define DECRYPT_HASH_WIDTH                                                    1
#define DECRYPT_HASH_SHIFT                                                    5
#define DECRYPT_HASH_MASK                                            0x00000020
#define DECRYPT_HASH_RD(src)                          (((src) & 0x00000020)>>5)
#define DECRYPT_HASH_WR(src)                     (((u32)(src)<<5) & 0x00000020)
#define DECRYPT_HASH_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields encrypt_hash	 */
#define ENCRYPT_HASH_WIDTH                                                    1
#define ENCRYPT_HASH_SHIFT                                                    4
#define ENCRYPT_HASH_MASK                                            0x00000010
#define ENCRYPT_HASH_RD(src)                          (((src) & 0x00000010)>>4)
#define ENCRYPT_HASH_WR(src)                     (((u32)(src)<<4) & 0x00000010)
#define ENCRYPT_HASH_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields hash_decrypt	 */
#define HASH_DECRYPT_WIDTH                                                    1
#define HASH_DECRYPT_SHIFT                                                    3
#define HASH_DECRYPT_MASK                                            0x00000008
#define HASH_DECRYPT_RD(src)                          (((src) & 0x00000008)>>3)
#define HASH_DECRYPT_WR(src)                     (((u32)(src)<<3) & 0x00000008)
#define HASH_DECRYPT_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields hash_encrypt	 */
#define HASH_ENCRYPT_WIDTH                                                    1
#define HASH_ENCRYPT_SHIFT                                                    2
#define HASH_ENCRYPT_MASK                                            0x00000004
#define HASH_ENCRYPT_RD(src)                          (((src) & 0x00000004)>>2)
#define HASH_ENCRYPT_WR(src)                     (((u32)(src)<<2) & 0x00000004)
#define HASH_ENCRYPT_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields encrypt_only	 */
#define ENCRYPT_ONLY_WIDTH                                                    1
#define ENCRYPT_ONLY_SHIFT                                                    1
#define ENCRYPT_ONLY_MASK                                            0x00000002
#define ENCRYPT_ONLY_RD(src)                          (((src) & 0x00000002)>>1)
#define ENCRYPT_ONLY_WR(src)                     (((u32)(src)<<1) & 0x00000002)
#define ENCRYPT_ONLY_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields hash_only_null	 */
#define HASH_ONLY_NULL_WIDTH                                                  1
#define HASH_ONLY_NULL_SHIFT                                                  0
#define HASH_ONLY_NULL_MASK                                          0x00000001
#define HASH_ONLY_NULL_RD(src)                           (((src) & 0x00000001))
#define HASH_ONLY_NULL_WR(src)                      (((u32)(src)) & 0x00000001)
#define HASH_ONLY_NULL_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IPE_CTX_CTRL	*/
/*	 Fields rsvd	 */
#define RSVD_WIDTH                                                           22
#define RSVD_SHIFT                                                           10
#define RSVD_MASK                                                    0xfffffc00
#define RSVD_RD(src)                                 (((src) & 0xfffffc00)>>10)
#define RSVD_WR(src)                            (((u32)(src)<<10) & 0xfffffc00)
#define RSVD_SET(dst,src) \
                      (((dst) & ~0xfffffc00) | (((u32)(src)<<10) & 0xfffffc00))
/*	 Fields Control_mode	 */
#define SEC_CONTROL_MODE_WIDTH                                                1
#define SEC_CONTROL_MODE_SHIFT                                                9
#define SEC_CONTROL_MODE_MASK                                        0x00000200
#define SEC_CONTROL_MODE_RD(src)                      (((src) & 0x00000200)>>9)
#define SEC_CONTROL_MODE_WR(src)                 (((u32)(src)<<9) & 0x00000200)
#define SEC_CONTROL_MODE_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields Address_mode	 */
#define SEC_ADDRESS_MODE_WIDTH                                                1
#define SEC_ADDRESS_MODE_SHIFT                                                8
#define SEC_ADDRESS_MODE_MASK                                        0x00000100
#define SEC_ADDRESS_MODE_RD(src)                      (((src) & 0x00000100)>>8)
#define SEC_ADDRESS_MODE_WR(src)                 (((u32)(src)<<8) & 0x00000100)
#define SEC_ADDRESS_MODE_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields Context_size	 */
#define CONTEXT_SIZE_WIDTH                                                    8
#define CONTEXT_SIZE_SHIFT                                                    0
#define CONTEXT_SIZE_MASK                                            0x000000ff
#define CONTEXT_SIZE_RD(src)                             (((src) & 0x000000ff))
#define CONTEXT_SIZE_WR(src)                        (((u32)(src)) & 0x000000ff)
#define CONTEXT_SIZE_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Register IPE_CTX_STAT	*/
/*	 Fields Next_Packet_Current_State	 */
#define NEXT_PACKET_CURRENT_STATE_WIDTH                                       4
#define NEXT_PACKET_CURRENT_STATE_SHIFT                                      28
#define NEXT_PACKET_CURRENT_STATE_MASK                               0xf0000000
#define NEXT_PACKET_CURRENT_STATE_RD(src)            (((src) & 0xf0000000)>>28)
#define NEXT_PACKET_CURRENT_STATE_WR(src)       (((u32)(src)<<28) & 0xf0000000)
#define NEXT_PACKET_CURRENT_STATE_SET(dst,src) \
                      (((dst) & ~0xf0000000) | (((u32)(src)<<28) & 0xf0000000))
/*	 Fields Active_Packet_Processing_Current_State	 */
#define ACTIVE_PACKET_PROCESSING_CURRENT_STATE_WIDTH                          4
#define ACTIVE_PACKET_PROCESSING_CURRENT_STATE_SHIFT                         24
#define ACTIVE_PACKET_PROCESSING_CURRENT_STATE_MASK                  0x0f000000
#define ACTIVE_PACKET_PROCESSING_CURRENT_STATE_RD(src) \
                                                    (((src) & 0x0f000000)>>24)
#define ACTIVE_PACKET_PROCESSING_CURRENT_STATE_WR(src) \
                                                (((u32)(src)<<24) & 0x0f000000)
#define ACTIVE_PACKET_PROCESSING_CURRENT_STATE_SET(dst,src) \
                      (((dst) & ~0x0f000000) | (((u32)(src)<<24) & 0x0f000000))
/*	 Fields rsvd2	 */
#define RSVD2_F1_WIDTH                                                        2
#define RSVD2_F1_SHIFT                                                       22
#define RSVD2_F1_MASK                                                0x00c00000
#define RSVD2_F1_RD(src)                             (((src) & 0x00c00000)>>22)
#define RSVD2_F1_WR(src)                        (((u32)(src)<<22) & 0x00c00000)
#define RSVD2_F1_SET(dst,src) \
                      (((dst) & ~0x00c00000) | (((u32)(src)<<22) & 0x00c00000))
/*	 Fields Error_recovery	 */
#define ERROR_RECOVERY_WIDTH                                                  1
#define ERROR_RECOVERY_SHIFT                                                 21
#define ERROR_RECOVERY_MASK                                          0x00200000
#define ERROR_RECOVERY_RD(src)                       (((src) & 0x00200000)>>21)
#define ERROR_RECOVERY_WR(src)                  (((u32)(src)<<21) & 0x00200000)
#define ERROR_RECOVERY_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields Result_Context	 */
#define RESULT_CONTEXT_F1_WIDTH                                               1
#define RESULT_CONTEXT_F1_SHIFT                                              20
#define RESULT_CONTEXT_F1_MASK                                       0x00100000
#define RESULT_CONTEXT_F1_RD(src)                    (((src) & 0x00100000)>>20)
#define RESULT_CONTEXT_F1_WR(src)               (((u32)(src)<<20) & 0x00100000)
#define RESULT_CONTEXT_F1_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields Next_Context	 */
#define NEXT_CONTEXT_WIDTH                                                    1
#define NEXT_CONTEXT_SHIFT                                                   19
#define NEXT_CONTEXT_MASK                                            0x00080000
#define NEXT_CONTEXT_RD(src)                         (((src) & 0x00080000)>>19)
#define NEXT_CONTEXT_WR(src)                    (((u32)(src)<<19) & 0x00080000)
#define NEXT_CONTEXT_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields Active_Context	 */
#define ACTIVE_CONTEXT_WIDTH                                                  1
#define ACTIVE_CONTEXT_SHIFT                                                 18
#define ACTIVE_CONTEXT_MASK                                          0x00040000
#define ACTIVE_CONTEXT_RD(src)                       (((src) & 0x00040000)>>18)
#define ACTIVE_CONTEXT_WR(src)                  (((u32)(src)<<18) & 0x00040000)
#define ACTIVE_CONTEXT_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields No_Available_Tokens	 */
#define NO_AVAILABLE_TOKENS_WIDTH                                             2
#define NO_AVAILABLE_TOKENS_SHIFT                                            16
#define NO_AVAILABLE_TOKENS_MASK                                     0x00030000
#define NO_AVAILABLE_TOKENS_RD(src)                  (((src) & 0x00030000)>>16)
#define NO_AVAILABLE_TOKENS_WR(src)             (((u32)(src)<<16) & 0x00030000)
#define NO_AVAILABLE_TOKENS_SET(dst,src) \
                      (((dst) & ~0x00030000) | (((u32)(src)<<16) & 0x00030000))
/*	 Fields rsvd1	 */
#define RSVD1_F1_WIDTH                                                        1
#define RSVD1_F1_SHIFT                                                       15
#define RSVD1_F1_MASK                                                0x00008000
#define RSVD1_F1_RD(src)                             (((src) & 0x00008000)>>15)
#define RSVD1_F1_WR(src)                        (((u32)(src)<<15) & 0x00008000)
#define RSVD1_F1_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields E14	 */
#define E14_WIDTH                                                             1
#define E14_SHIFT                                                            14
#define E14_MASK                                                     0x00004000
#define E14_RD(src)                                  (((src) & 0x00004000)>>14)
#define E14_WR(src)                             (((u32)(src)<<14) & 0x00004000)
#define E14_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields E13	 */
#define E13_WIDTH                                                             1
#define E13_SHIFT                                                            13
#define E13_MASK                                                     0x00002000
#define E13_RD(src)                                  (((src) & 0x00002000)>>13)
#define E13_WR(src)                             (((u32)(src)<<13) & 0x00002000)
#define E13_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields E12	 */
#define E12_WIDTH                                                             1
#define E12_SHIFT                                                            12
#define E12_MASK                                                     0x00001000
#define E12_RD(src)                                  (((src) & 0x00001000)>>12)
#define E12_WR(src)                             (((u32)(src)<<12) & 0x00001000)
#define E12_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields E11	 */
#define E11_WIDTH                                                             1
#define E11_SHIFT                                                            11
#define E11_MASK                                                     0x00000800
#define E11_RD(src)                                  (((src) & 0x00000800)>>11)
#define E11_WR(src)                             (((u32)(src)<<11) & 0x00000800)
#define E11_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields E10	 */
#define E10_WIDTH                                                             1
#define E10_SHIFT                                                            10
#define E10_MASK                                                     0x00000400
#define E10_RD(src)                                  (((src) & 0x00000400)>>10)
#define E10_WR(src)                             (((u32)(src)<<10) & 0x00000400)
#define E10_SET(dst,src) \
                      (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*	 Fields E9	 */
#define E9_WIDTH                                                              1
#define E9_SHIFT                                                              9
#define E9_MASK                                                      0x00000200
#define E9_RD(src)                                    (((src) & 0x00000200)>>9)
#define E9_WR(src)                               (((u32)(src)<<9) & 0x00000200)
#define E9_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*	 Fields E8	 */
#define E8_WIDTH                                                              1
#define E8_SHIFT                                                              8
#define E8_MASK                                                      0x00000100
#define E8_RD(src)                                    (((src) & 0x00000100)>>8)
#define E8_WR(src)                               (((u32)(src)<<8) & 0x00000100)
#define E8_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*	 Fields E7	 */
#define E7_WIDTH                                                              1
#define E7_SHIFT                                                              7
#define E7_MASK                                                      0x00000080
#define E7_RD(src)                                    (((src) & 0x00000080)>>7)
#define E7_WR(src)                               (((u32)(src)<<7) & 0x00000080)
#define E7_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*	 Fields E6	 */
#define E6_WIDTH                                                              1
#define E6_SHIFT                                                              6
#define E6_MASK                                                      0x00000040
#define E6_RD(src)                                    (((src) & 0x00000040)>>6)
#define E6_WR(src)                               (((u32)(src)<<6) & 0x00000040)
#define E6_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*	 Fields E5	 */
#define E5_WIDTH                                                              1
#define E5_SHIFT                                                              5
#define E5_MASK                                                      0x00000020
#define E5_RD(src)                                    (((src) & 0x00000020)>>5)
#define E5_WR(src)                               (((u32)(src)<<5) & 0x00000020)
#define E5_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields E4	 */
#define E4_WIDTH                                                              1
#define E4_SHIFT                                                              4
#define E4_MASK                                                      0x00000010
#define E4_RD(src)                                    (((src) & 0x00000010)>>4)
#define E4_WR(src)                               (((u32)(src)<<4) & 0x00000010)
#define E4_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields E3	 */
#define E3_WIDTH                                                              1
#define E3_SHIFT                                                              3
#define E3_MASK                                                      0x00000008
#define E3_RD(src)                                    (((src) & 0x00000008)>>3)
#define E3_WR(src)                               (((u32)(src)<<3) & 0x00000008)
#define E3_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields E2	 */
#define E2_WIDTH                                                              1
#define E2_SHIFT                                                              2
#define E2_MASK                                                      0x00000004
#define E2_RD(src)                                    (((src) & 0x00000004)>>2)
#define E2_WR(src)                               (((u32)(src)<<2) & 0x00000004)
#define E2_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields E1	 */
#define E1_WIDTH                                                              1
#define E1_SHIFT                                                              1
#define E1_MASK                                                      0x00000002
#define E1_RD(src)                                    (((src) & 0x00000002)>>1)
#define E1_WR(src)                               (((u32)(src)<<1) & 0x00000002)
#define E1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields E0	 */
#define E0_WIDTH                                                              1
#define E0_SHIFT                                                              0
#define E0_MASK                                                      0x00000001
#define E0_RD(src)                                       (((src) & 0x00000001))
#define E0_WR(src)                                  (((u32)(src)) & 0x00000001)
#define E0_SET(dst,src) (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IPE_INT_CTRL_STAT	*/
/*	 Fields Interrupt_output_pin_enable	 */
#define INTERRUPT_OUTPUT_PIN_ENABLE_WIDTH                                     1
#define INTERRUPT_OUTPUT_PIN_ENABLE_SHIFT                                    31
#define INTERRUPT_OUTPUT_PIN_ENABLE_MASK                             0x80000000
#define INTERRUPT_OUTPUT_PIN_ENABLE_RD(src)          (((src) & 0x80000000)>>31)
#define INTERRUPT_OUTPUT_PIN_ENABLE_WR(src)     (((u32)(src)<<31) & 0x80000000)
#define INTERRUPT_OUTPUT_PIN_ENABLE_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields Fatal_error_enable	 */
#define FATAL_ERROR_ENABLE_WIDTH                                              1
#define FATAL_ERROR_ENABLE_SHIFT                                             30
#define FATAL_ERROR_ENABLE_MASK                                      0x40000000
#define FATAL_ERROR_ENABLE_RD(src)                   (((src) & 0x40000000)>>30)
#define FATAL_ERROR_ENABLE_WR(src)              (((u32)(src)<<30) & 0x40000000)
#define FATAL_ERROR_ENABLE_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields rsvd2	 */
#define RSVD2_F2_WIDTH                                                       10
#define RSVD2_F2_SHIFT                                                       20
#define RSVD2_F2_MASK                                                0x3ff00000
#define RSVD2_F2_RD(src)                             (((src) & 0x3ff00000)>>20)
#define RSVD2_F2_WR(src)                        (((u32)(src)<<20) & 0x3ff00000)
#define RSVD2_F2_SET(dst,src) \
                      (((dst) & ~0x3ff00000) | (((u32)(src)<<20) & 0x3ff00000))
/*	 Fields Packet_timeout_enable	 */
#define PACKET_TIMEOUT_ENABLE_WIDTH                                           1
#define PACKET_TIMEOUT_ENABLE_SHIFT                                          19
#define PACKET_TIMEOUT_ENABLE_MASK                                   0x00080000
#define PACKET_TIMEOUT_ENABLE_RD(src)                (((src) & 0x00080000)>>19)
#define PACKET_TIMEOUT_ENABLE_WR(src)           (((u32)(src)<<19) & 0x00080000)
#define PACKET_TIMEOUT_ENABLE_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields Packet_processing_enable	 */
#define PACKET_PROCESSING_ENABLE_WIDTH                                        1
#define PACKET_PROCESSING_ENABLE_SHIFT                                       18
#define PACKET_PROCESSING_ENABLE_MASK                                0x00040000
#define PACKET_PROCESSING_ENABLE_RD(src)             (((src) & 0x00040000)>>18)
#define PACKET_PROCESSING_ENABLE_WR(src)        (((u32)(src)<<18) & 0x00040000)
#define PACKET_PROCESSING_ENABLE_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields Output_DMA_error_enable	 */
#define OUTPUT_DMA_ERROR_ENABLE_WIDTH                                         1
#define OUTPUT_DMA_ERROR_ENABLE_SHIFT                                        17
#define OUTPUT_DMA_ERROR_ENABLE_MASK                                 0x00020000
#define OUTPUT_DMA_ERROR_ENABLE_RD(src)              (((src) & 0x00020000)>>17)
#define OUTPUT_DMA_ERROR_ENABLE_WR(src)         (((u32)(src)<<17) & 0x00020000)
#define OUTPUT_DMA_ERROR_ENABLE_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields Input_DMA_error_enable	 */
#define INPUT_DMA_ERROR_ENABLE_WIDTH                                          1
#define INPUT_DMA_ERROR_ENABLE_SHIFT                                         16
#define INPUT_DMA_ERROR_ENABLE_MASK                                  0x00010000
#define INPUT_DMA_ERROR_ENABLE_RD(src)               (((src) & 0x00010000)>>16)
#define INPUT_DMA_ERROR_ENABLE_WR(src)          (((u32)(src)<<16) & 0x00010000)
#define INPUT_DMA_ERROR_ENABLE_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields Interrupt_output_pin_status	 */
#define INTERRUPT_OUTPUT_PIN_STATUS_WIDTH                                     1
#define INTERRUPT_OUTPUT_PIN_STATUS_SHIFT                                    15
#define INTERRUPT_OUTPUT_PIN_STATUS_MASK                             0x00008000
#define INTERRUPT_OUTPUT_PIN_STATUS_RD(src)          (((src) & 0x00008000)>>15)
#define INTERRUPT_OUTPUT_PIN_STATUS_WR(src)     (((u32)(src)<<15) & 0x00008000)
#define INTERRUPT_OUTPUT_PIN_STATUS_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields Fatal_error_status	 */
#define FATAL_ERROR_STATUS_WIDTH                                              1
#define FATAL_ERROR_STATUS_SHIFT                                             14
#define FATAL_ERROR_STATUS_MASK                                      0x00004000
#define FATAL_ERROR_STATUS_RD(src)                   (((src) & 0x00004000)>>14)
#define FATAL_ERROR_STATUS_WR(src)              (((u32)(src)<<14) & 0x00004000)
#define FATAL_ERROR_STATUS_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields rsvd1	 */
#define RSVD1_F2_WIDTH                                                       10
#define RSVD1_F2_SHIFT                                                        4
#define RSVD1_F2_MASK                                                0x00003ff0
#define RSVD1_F2_RD(src)                              (((src) & 0x00003ff0)>>4)
#define RSVD1_F2_WR(src)                         (((u32)(src)<<4) & 0x00003ff0)
#define RSVD1_F2_SET(dst,src) \
                       (((dst) & ~0x00003ff0) | (((u32)(src)<<4) & 0x00003ff0))
/*	 Fields Packet_timeout_status	 */
#define PACKET_TIMEOUT_STATUS_WIDTH                                           1
#define PACKET_TIMEOUT_STATUS_SHIFT                                           3
#define PACKET_TIMEOUT_STATUS_MASK                                   0x00000008
#define PACKET_TIMEOUT_STATUS_RD(src)                 (((src) & 0x00000008)>>3)
#define PACKET_TIMEOUT_STATUS_WR(src)            (((u32)(src)<<3) & 0x00000008)
#define PACKET_TIMEOUT_STATUS_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields Packet_processing_status	 */
#define PACKET_PROCESSING_STATUS_WIDTH                                        1
#define PACKET_PROCESSING_STATUS_SHIFT                                        2
#define PACKET_PROCESSING_STATUS_MASK                                0x00000004
#define PACKET_PROCESSING_STATUS_RD(src)              (((src) & 0x00000004)>>2)
#define PACKET_PROCESSING_STATUS_WR(src)         (((u32)(src)<<2) & 0x00000004)
#define PACKET_PROCESSING_STATUS_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields Output_DMA_error_status	 */
#define OUTPUT_DMA_ERROR_STATUS_WIDTH                                         1
#define OUTPUT_DMA_ERROR_STATUS_SHIFT                                         1
#define OUTPUT_DMA_ERROR_STATUS_MASK                                 0x00000002
#define OUTPUT_DMA_ERROR_STATUS_RD(src)               (((src) & 0x00000002)>>1)
#define OUTPUT_DMA_ERROR_STATUS_WR(src)          (((u32)(src)<<1) & 0x00000002)
#define OUTPUT_DMA_ERROR_STATUS_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields Input_DMA_error_status	 */
#define INPUT_DMA_ERROR_STATUS_WIDTH                                          1
#define INPUT_DMA_ERROR_STATUS_SHIFT                                          0
#define INPUT_DMA_ERROR_STATUS_MASK                                  0x00000001
#define INPUT_DMA_ERROR_STATUS_RD(src)                   (((src) & 0x00000001))
#define INPUT_DMA_ERROR_STATUS_WR(src)              (((u32)(src)) & 0x00000001)
#define INPUT_DMA_ERROR_STATUS_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IPE_RX_CTRL_STAT	*/
/*	 Fields Transfer_size_mask	 */
#define TRANSFER_SIZE_MASK_WIDTH                                              8
#define TRANSFER_SIZE_MASK_SHIFT                                             24
#define TRANSFER_SIZE_MASK_MASK                                      0xff000000
#define TRANSFER_SIZE_MASK_RD(src)                   (((src) & 0xff000000)>>24)
#define TRANSFER_SIZE_MASK_WR(src)              (((u32)(src)<<24) & 0xff000000)
#define TRANSFER_SIZE_MASK_SET(dst,src) \
                      (((dst) & ~0xff000000) | (((u32)(src)<<24) & 0xff000000))
/*	 Fields Max_transfer_size	 */
#define MAX_TRANSFER_SIZE_WIDTH                                               8
#define MAX_TRANSFER_SIZE_SHIFT                                              16
#define MAX_TRANSFER_SIZE_MASK                                       0x00ff0000
#define MAX_TRANSFER_SIZE_RD(src)                    (((src) & 0x00ff0000)>>16)
#define MAX_TRANSFER_SIZE_WR(src)               (((u32)(src)<<16) & 0x00ff0000)
#define MAX_TRANSFER_SIZE_SET(dst,src) \
                      (((dst) & ~0x00ff0000) | (((u32)(src)<<16) & 0x00ff0000))
/*	 Fields Min_transfer_size	 */
#define MIN_TRANSFER_SIZE_WIDTH                                               8
#define MIN_TRANSFER_SIZE_SHIFT                                               8
#define MIN_TRANSFER_SIZE_MASK                                       0x0000ff00
#define MIN_TRANSFER_SIZE_RD(src)                     (((src) & 0x0000ff00)>>8)
#define MIN_TRANSFER_SIZE_WR(src)                (((u32)(src)<<8) & 0x0000ff00)
#define MIN_TRANSFER_SIZE_SET(dst,src) \
                       (((dst) & ~0x0000ff00) | (((u32)(src)<<8) & 0x0000ff00))
/*	 Fields Available_dwords	 */
#define AVAILABLE_DWORDS_WIDTH                                                8
#define AVAILABLE_DWORDS_SHIFT                                                0
#define AVAILABLE_DWORDS_MASK                                        0x000000ff
#define AVAILABLE_DWORDS_RD(src)                         (((src) & 0x000000ff))
#define AVAILABLE_DWORDS_WR(src)                    (((u32)(src)) & 0x000000ff)
#define AVAILABLE_DWORDS_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Register IPE_TX_CTRL_STAT	*/
/*	 Fields Transfer_size_mask	 */
#define TRANSFER_SIZE_MASK_F1_WIDTH                                           8
#define TRANSFER_SIZE_MASK_F1_SHIFT                                          24
#define TRANSFER_SIZE_MASK_F1_MASK                                   0xff000000
#define TRANSFER_SIZE_MASK_F1_RD(src)                (((src) & 0xff000000)>>24)
#define TRANSFER_SIZE_MASK_F1_WR(src)           (((u32)(src)<<24) & 0xff000000)
#define TRANSFER_SIZE_MASK_F1_SET(dst,src) \
                      (((dst) & ~0xff000000) | (((u32)(src)<<24) & 0xff000000))
/*	 Fields Max_transfer_size	 */
#define MAX_TRANSFER_SIZE_F1_WIDTH                                            8
#define MAX_TRANSFER_SIZE_F1_SHIFT                                           16
#define MAX_TRANSFER_SIZE_F1_MASK                                    0x00ff0000
#define MAX_TRANSFER_SIZE_F1_RD(src)                 (((src) & 0x00ff0000)>>16)
#define MAX_TRANSFER_SIZE_F1_WR(src)            (((u32)(src)<<16) & 0x00ff0000)
#define MAX_TRANSFER_SIZE_F1_SET(dst,src) \
                      (((dst) & ~0x00ff0000) | (((u32)(src)<<16) & 0x00ff0000))
/*	 Fields Min_transfer_size	 */
#define MIN_TRANSFER_SIZE_F1_WIDTH                                            8
#define MIN_TRANSFER_SIZE_F1_SHIFT                                            8
#define MIN_TRANSFER_SIZE_F1_MASK                                    0x0000ff00
#define MIN_TRANSFER_SIZE_F1_RD(src)                  (((src) & 0x0000ff00)>>8)
#define MIN_TRANSFER_SIZE_F1_WR(src)             (((u32)(src)<<8) & 0x0000ff00)
#define MIN_TRANSFER_SIZE_F1_SET(dst,src) \
                       (((dst) & ~0x0000ff00) | (((u32)(src)<<8) & 0x0000ff00))
/*	 Fields Available_dwords	 */
#define AVAILABLE_DWORDS_F1_WIDTH                                             8
#define AVAILABLE_DWORDS_F1_SHIFT                                             0
#define AVAILABLE_DWORDS_F1_MASK                                     0x000000ff
#define AVAILABLE_DWORDS_F1_RD(src)                      (((src) & 0x000000ff))
#define AVAILABLE_DWORDS_F1_WR(src)                 (((u32)(src)) & 0x000000ff)
#define AVAILABLE_DWORDS_F1_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Register IPE_DEV_INFO	*/
/*	 Fields Kasumi_f9	 */
#define KASUMI_F9_F1_WIDTH                                                    1
#define KASUMI_F9_F1_SHIFT                                                   31
#define KASUMI_F9_F1_MASK                                            0x80000000
#define KASUMI_F9_F1_RD(src)                         (((src) & 0x80000000)>>31)
#define KASUMI_F9_F1_WR(src)                    (((u32)(src)<<31) & 0x80000000)
#define KASUMI_F9_F1_SET(dst,src) \
                      (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields GHASH	 */
#define GHASH_WIDTH                                                           1
#define GHASH_SHIFT                                                          30
#define GHASH_MASK                                                   0x40000000
#define GHASH_RD(src)                                (((src) & 0x40000000)>>30)
#define GHASH_WR(src)                           (((u32)(src)<<30) & 0x40000000)
#define GHASH_SET(dst,src) \
                      (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields CBC_MAC_key_lengths	 */
#define CBC_MAC_KEY_LENGTHS_WIDTH                                             1
#define CBC_MAC_KEY_LENGTHS_SHIFT                                            29
#define CBC_MAC_KEY_LENGTHS_MASK                                     0x20000000
#define CBC_MAC_KEY_LENGTHS_RD(src)                  (((src) & 0x20000000)>>29)
#define CBC_MAC_KEY_LENGTHS_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define CBC_MAC_KEY_LENGTHS_SET(dst,src) \
                      (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*	 Fields CBC_MAC_Speed	 */
#define CBC_MAC_SPEED_WIDTH                                                   1
#define CBC_MAC_SPEED_SHIFT                                                  28
#define CBC_MAC_SPEED_MASK                                           0x10000000
#define CBC_MAC_SPEED_RD(src)                        (((src) & 0x10000000)>>28)
#define CBC_MAC_SPEED_WR(src)                   (((u32)(src)<<28) & 0x10000000)
#define CBC_MAC_SPEED_SET(dst,src) \
                      (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*	 Fields XCBC_MAC	 */
#define XCBC_MAC_WIDTH                                                        1
#define XCBC_MAC_SHIFT                                                       27
#define XCBC_MAC_MASK                                                0x08000000
#define XCBC_MAC_RD(src)                             (((src) & 0x08000000)>>27)
#define XCBC_MAC_WR(src)                        (((u32)(src)<<27) & 0x08000000)
#define XCBC_MAC_SET(dst,src) \
                      (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*	 Fields SHA_384_512	 */
#define SHA_384_512_WIDTH                                                     1
#define SHA_384_512_SHIFT                                                    26
#define SHA_384_512_MASK                                             0x04000000
#define SHA_384_512_RD(src)                          (((src) & 0x04000000)>>26)
#define SHA_384_512_WR(src)                     (((u32)(src)<<26) & 0x04000000)
#define SHA_384_512_SET(dst,src) \
                      (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*	 Fields SHA_224_256	 */
#define SHA_224_256_WIDTH                                                     1
#define SHA_224_256_SHIFT                                                    25
#define SHA_224_256_MASK                                             0x02000000
#define SHA_224_256_RD(src)                          (((src) & 0x02000000)>>25)
#define SHA_224_256_WR(src)                     (((u32)(src)<<25) & 0x02000000)
#define SHA_224_256_SET(dst,src) \
                      (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*	 Fields SHA_1_Speed	 */
#define SHA_1_SPEED_WIDTH                                                     1
#define SHA_1_SPEED_SHIFT                                                    24
#define SHA_1_SPEED_MASK                                             0x01000000
#define SHA_1_SPEED_RD(src)                          (((src) & 0x01000000)>>24)
#define SHA_1_SPEED_WR(src)                     (((u32)(src)<<24) & 0x01000000)
#define SHA_1_SPEED_SET(dst,src) \
                      (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*	 Fields SHA_1	 */
#define SHA_1_WIDTH                                                           1
#define SHA_1_SHIFT                                                          23
#define SHA_1_MASK                                                   0x00800000
#define SHA_1_RD(src)                                (((src) & 0x00800000)>>23)
#define SHA_1_WR(src)                           (((u32)(src)<<23) & 0x00800000)
#define SHA_1_SET(dst,src) \
                      (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*	 Fields MD5	 */
#define MD5_WIDTH                                                             1
#define MD5_SHIFT                                                            22
#define MD5_MASK                                                     0x00400000
#define MD5_RD(src)                                  (((src) & 0x00400000)>>22)
#define MD5_WR(src)                             (((u32)(src)<<22) & 0x00400000)
#define MD5_SET(dst,src) \
                      (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*	 Fields Kasumi_Speed	 */
#define KASUMI_SPEED_WIDTH                                                    1
#define KASUMI_SPEED_SHIFT                                                   21
#define KASUMI_SPEED_MASK                                            0x00200000
#define KASUMI_SPEED_RD(src)                         (((src) & 0x00200000)>>21)
#define KASUMI_SPEED_WR(src)                    (((u32)(src)<<21) & 0x00200000)
#define KASUMI_SPEED_SET(dst,src) \
                      (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*	 Fields Kasumi	 */
#define KASUMI_WIDTH                                                          1
#define KASUMI_SHIFT                                                         20
#define KASUMI_MASK                                                  0x00100000
#define KASUMI_RD(src)                               (((src) & 0x00100000)>>20)
#define KASUMI_WR(src)                          (((u32)(src)<<20) & 0x00100000)
#define KASUMI_SET(dst,src) \
                      (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*	 Fields ARC4_Speed	 */
#define ARC4_SPEED_WIDTH                                                      1
#define ARC4_SPEED_SHIFT                                                     19
#define ARC4_SPEED_MASK                                              0x00080000
#define ARC4_SPEED_RD(src)                           (((src) & 0x00080000)>>19)
#define ARC4_SPEED_WR(src)                      (((u32)(src)<<19) & 0x00080000)
#define ARC4_SPEED_SET(dst,src) \
                      (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*	 Fields ARC4	 */
#define ARC4_F1_WIDTH                                                         1
#define ARC4_F1_SHIFT                                                        18
#define ARC4_F1_MASK                                                 0x00040000
#define ARC4_F1_RD(src)                              (((src) & 0x00040000)>>18)
#define ARC4_F1_WR(src)                         (((u32)(src)<<18) & 0x00040000)
#define ARC4_F1_SET(dst,src) \
                      (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*	 Fields DES_Speed	 */
#define DES_SPEED_WIDTH                                                       1
#define DES_SPEED_SHIFT                                                      17
#define DES_SPEED_MASK                                               0x00020000
#define DES_SPEED_RD(src)                            (((src) & 0x00020000)>>17)
#define DES_SPEED_WR(src)                       (((u32)(src)<<17) & 0x00020000)
#define DES_SPEED_SET(dst,src) \
                      (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*	 Fields DES_FB	 */
#define DES_FB_WIDTH                                                          1
#define DES_FB_SHIFT                                                         16
#define DES_FB_MASK                                                  0x00010000
#define DES_FB_RD(src)                               (((src) & 0x00010000)>>16)
#define DES_FB_WR(src)                          (((u32)(src)<<16) & 0x00010000)
#define DES_FB_SET(dst,src) \
                      (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields DES	 */
#define DES_WIDTH                                                             1
#define DES_SHIFT                                                            15
#define DES_MASK                                                     0x00008000
#define DES_RD(src)                                  (((src) & 0x00008000)>>15)
#define DES_WR(src)                             (((u32)(src)<<15) & 0x00008000)
#define DES_SET(dst,src) \
                      (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*	 Fields AES_Speed	 */
#define AES_SPEED_WIDTH                                                       1
#define AES_SPEED_SHIFT                                                      14
#define AES_SPEED_MASK                                               0x00004000
#define AES_SPEED_RD(src)                            (((src) & 0x00004000)>>14)
#define AES_SPEED_WR(src)                       (((u32)(src)<<14) & 0x00004000)
#define AES_SPEED_SET(dst,src) \
                      (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*	 Fields AES_FB	 */
#define AES_FB_WIDTH                                                          1
#define AES_FB_SHIFT                                                         13
#define AES_FB_MASK                                                  0x00002000
#define AES_FB_RD(src)                               (((src) & 0x00002000)>>13)
#define AES_FB_WR(src)                          (((u32)(src)<<13) & 0x00002000)
#define AES_FB_SET(dst,src) \
                      (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*	 Fields AES	 */
#define AES_WIDTH                                                             1
#define AES_SHIFT                                                            12
#define AES_MASK                                                     0x00001000
#define AES_RD(src)                                  (((src) & 0x00001000)>>12)
#define AES_WR(src)                             (((u32)(src)<<12) & 0x00001000)
#define AES_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields IPE_DEV_INFO_reserved	 */
#define IPE_DEV_INFO_RESERVED_WIDTH                                           4
#define IPE_DEV_INFO_RESERVED_SHIFT                                           8
#define IPE_DEV_INFO_RESERVED_MASK                                   0x00000f00
#define IPE_DEV_INFO_RESERVED_RD(src)                 (((src) & 0x00000f00)>>8)
#define IPE_DEV_INFO_RESERVED_WR(src)            (((u32)(src)<<8) & 0x00000f00)
#define IPE_DEV_INFO_RESERVED_SET(dst,src) \
                       (((dst) & ~0x00000f00) | (((u32)(src)<<8) & 0x00000f00))
/*	 Fields Major_revision_number	 */
#define MAJOR_REVISION_NUMBER_WIDTH                                           4
#define MAJOR_REVISION_NUMBER_SHIFT                                           4
#define MAJOR_REVISION_NUMBER_MASK                                   0x000000f0
#define MAJOR_REVISION_NUMBER_RD(src)                 (((src) & 0x000000f0)>>4)
#define MAJOR_REVISION_NUMBER_WR(src)            (((u32)(src)<<4) & 0x000000f0)
#define MAJOR_REVISION_NUMBER_SET(dst,src) \
                       (((dst) & ~0x000000f0) | (((u32)(src)<<4) & 0x000000f0))
/*	 Fields Minor_revision_number	 */
#define MINOR_REVISION_NUMBER_WIDTH                                           4
#define MINOR_REVISION_NUMBER_SHIFT                                           0
#define MINOR_REVISION_NUMBER_MASK                                   0x0000000f
#define MINOR_REVISION_NUMBER_RD(src)                    (((src) & 0x0000000f))
#define MINOR_REVISION_NUMBER_WR(src)               (((u32)(src)) & 0x0000000f)
#define MINOR_REVISION_NUMBER_SET(dst,src) \
                          (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register IPE_PRNG_STAT	*/
/*	 Fields rsvd	 */
#define RSVD_F1_WIDTH                                                        30
#define RSVD_F1_SHIFT                                                         2
#define RSVD_F1_MASK                                                 0xfffffffc
#define RSVD_F1_RD(src)                               (((src) & 0xfffffffc)>>2)
#define RSVD_F1_WR(src)                          (((u32)(src)<<2) & 0xfffffffc)
#define RSVD_F1_SET(dst,src) \
                       (((dst) & ~0xfffffffc) | (((u32)(src)<<2) & 0xfffffffc))
/*	 Fields Result_Ready	 */
#define RESULT_READY_WIDTH                                                    1
#define RESULT_READY_SHIFT                                                    1
#define RESULT_READY_MASK                                            0x00000002
#define RESULT_READY_RD(src)                          (((src) & 0x00000002)>>1)
#define RESULT_READY_WR(src)                     (((u32)(src)<<1) & 0x00000002)
#define RESULT_READY_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields Busy	 */
#define BUSY_F1_WIDTH                                                         1
#define BUSY_F1_SHIFT                                                         0
#define BUSY_F1_MASK                                                 0x00000001
#define BUSY_F1_RD(src)                                  (((src) & 0x00000001))
#define BUSY_F1_WR(src)                             (((u32)(src)) & 0x00000001)
#define BUSY_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IPE_PRNG_CTRL	*/
/*	 Fields rsvd	 */
#define RSVD_F2_WIDTH                                                        29
#define RSVD_F2_SHIFT                                                         3
#define RSVD_F2_MASK                                                 0xfffffff8
#define RSVD_F2_RD(src)                               (((src) & 0xfffffff8)>>3)
#define RSVD_F2_WR(src)                          (((u32)(src)<<3) & 0xfffffff8)
#define RSVD_F2_SET(dst,src) \
                       (((dst) & ~0xfffffff8) | (((u32)(src)<<3) & 0xfffffff8))
/*	 Fields Result_128	 */
#define RESULT_128_WIDTH                                                      1
#define RESULT_128_SHIFT                                                      2
#define RESULT_128_MASK                                              0x00000004
#define RESULT_128_RD(src)                            (((src) & 0x00000004)>>2)
#define RESULT_128_WR(src)                       (((u32)(src)<<2) & 0x00000004)
#define RESULT_128_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields Auto	 */
#define AUTO_WIDTH                                                            1
#define AUTO_SHIFT                                                            1
#define AUTO_MASK                                                    0x00000002
#define AUTO_RD(src)                                  (((src) & 0x00000002)>>1)
#define AUTO_WR(src)                             (((u32)(src)<<1) & 0x00000002)
#define AUTO_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields Enable	 */
#define SEC_ENABLE_F8_WIDTH                                                   1
#define SEC_ENABLE_F8_SHIFT                                                   0
#define SEC_ENABLE_F8_MASK                                           0x00000001
#define SEC_ENABLE_F8_RD(src)                            (((src) & 0x00000001))
#define SEC_ENABLE_F8_WR(src)                       (((u32)(src)) & 0x00000001)
#define SEC_ENABLE_F8_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register IPE_PRNG_SEED_L	*/
/*	 Fields PRNG_Seed_Low	 */
#define PRNG_SEED_LOW_WIDTH                                                  32
#define PRNG_SEED_LOW_SHIFT                                                   0
#define PRNG_SEED_LOW_MASK                                           0xffffffff
#define PRNG_SEED_LOW_RD(src)                            (((src) & 0xffffffff))
#define PRNG_SEED_LOW_WR(src)                       (((u32)(src)) & 0xffffffff)
#define PRNG_SEED_LOW_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_PRNG_SEED_H	*/
/*	 Fields PRNG_Seed_High	 */
#define PRNG_SEED_HIGH_WIDTH                                                 32
#define PRNG_SEED_HIGH_SHIFT                                                  0
#define PRNG_SEED_HIGH_MASK                                          0xffffffff
#define PRNG_SEED_HIGH_RD(src)                           (((src) & 0xffffffff))
#define PRNG_SEED_HIGH_WR(src)                      (((u32)(src)) & 0xffffffff)
#define PRNG_SEED_HIGH_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_PRNG_KEY_0_L	*/
/*	 Fields PRNG_Key0_Low	 */
#define PRNG_KEY0_LOW_WIDTH                                                  32
#define PRNG_KEY0_LOW_SHIFT                                                   0
#define PRNG_KEY0_LOW_MASK                                           0xffffffff
#define PRNG_KEY0_LOW_RD(src)                            (((src) & 0xffffffff))
#define PRNG_KEY0_LOW_WR(src)                       (((u32)(src)) & 0xffffffff)
#define PRNG_KEY0_LOW_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_PRNG_KEY_0_H	*/
/*	 Fields PRNG_Key0_High	 */
#define PRNG_KEY0_HIGH_WIDTH                                                 32
#define PRNG_KEY0_HIGH_SHIFT                                                  0
#define PRNG_KEY0_HIGH_MASK                                          0xffffffff
#define PRNG_KEY0_HIGH_RD(src)                           (((src) & 0xffffffff))
#define PRNG_KEY0_HIGH_WR(src)                      (((u32)(src)) & 0xffffffff)
#define PRNG_KEY0_HIGH_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_PRNG_KEY_1_L	*/
/*	 Fields PRNG_Key1_Low	 */
#define PRNG_KEY1_LOW_WIDTH                                                  32
#define PRNG_KEY1_LOW_SHIFT                                                   0
#define PRNG_KEY1_LOW_MASK                                           0xffffffff
#define PRNG_KEY1_LOW_RD(src)                            (((src) & 0xffffffff))
#define PRNG_KEY1_LOW_WR(src)                       (((u32)(src)) & 0xffffffff)
#define PRNG_KEY1_LOW_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_PRNG_KEY_1_H	*/
/*	 Fields PRNG_Key1_High	 */
#define PRNG_KEY1_HIGH_WIDTH                                                 32
#define PRNG_KEY1_HIGH_SHIFT                                                  0
#define PRNG_KEY1_HIGH_MASK                                          0xffffffff
#define PRNG_KEY1_HIGH_RD(src)                           (((src) & 0xffffffff))
#define PRNG_KEY1_HIGH_WR(src)                      (((u32)(src)) & 0xffffffff)
#define PRNG_KEY1_HIGH_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_PRNG_RES0_L	*/
/*	 Fields PRNG_Res0_Low	 */
#define PRNG_RES0_LOW_WIDTH                                                  32
#define PRNG_RES0_LOW_SHIFT                                                   0
#define PRNG_RES0_LOW_MASK                                           0xffffffff
#define PRNG_RES0_LOW_RD(src)                            (((src) & 0xffffffff))
#define PRNG_RES0_LOW_WR(src)                       (((u32)(src)) & 0xffffffff)
#define PRNG_RES0_LOW_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_PRNG_RES0_H	*/
/*	 Fields PRNG_Res0_High	 */
#define PRNG_RES0_HIGH_WIDTH                                                 32
#define PRNG_RES0_HIGH_SHIFT                                                  0
#define PRNG_RES0_HIGH_MASK                                          0xffffffff
#define PRNG_RES0_HIGH_RD(src)                           (((src) & 0xffffffff))
#define PRNG_RES0_HIGH_WR(src)                      (((u32)(src)) & 0xffffffff)
#define PRNG_RES0_HIGH_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_PRNG_RES1_L	*/
/*	 Fields PRNG_Res1_Low	 */
#define PRNG_RES1_LOW_WIDTH                                                  32
#define PRNG_RES1_LOW_SHIFT                                                   0
#define PRNG_RES1_LOW_MASK                                           0xffffffff
#define PRNG_RES1_LOW_RD(src)                            (((src) & 0xffffffff))
#define PRNG_RES1_LOW_WR(src)                       (((u32)(src)) & 0xffffffff)
#define PRNG_RES1_LOW_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_PRNG_RES1_H	*/
/*	 Fields PRNG_Res1_High	 */
#define PRNG_RES1_HIGH_WIDTH                                                 32
#define PRNG_RES1_HIGH_SHIFT                                                  0
#define PRNG_RES1_HIGH_MASK                                          0xffffffff
#define PRNG_RES1_HIGH_RD(src)                           (((src) & 0xffffffff))
#define PRNG_RES1_HIGH_WR(src)                      (((u32)(src)) & 0xffffffff)
#define PRNG_RES1_HIGH_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_PRNG_LFSR_L	*/
/*	 Fields PRNG_LFSR_Low	 */
#define PRNG_LFSR_LOW_WIDTH                                                  32
#define PRNG_LFSR_LOW_SHIFT                                                   0
#define PRNG_LFSR_LOW_MASK                                           0xffffffff
#define PRNG_LFSR_LOW_RD(src)                            (((src) & 0xffffffff))
#define PRNG_LFSR_LOW_WR(src)                       (((u32)(src)) & 0xffffffff)
#define PRNG_LFSR_LOW_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_PRNG_LFSR_H	*/
/*	 Fields PRNG_LFSR_High	 */
#define PRNG_LFSR_HIGH_WIDTH                                                 32
#define PRNG_LFSR_HIGH_SHIFT                                                  0
#define PRNG_LFSR_HIGH_MASK                                          0xffffffff
#define PRNG_LFSR_HIGH_RD(src)                           (((src) & 0xffffffff))
#define PRNG_LFSR_HIGH_WR(src)                      (((u32)(src)) & 0xffffffff)
#define PRNG_LFSR_HIGH_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W0	*/
/*	 Fields IPE_ACT_TKN_W0	 */
#define IPE_ACT_TKN_W00_WIDTH                                                32
#define IPE_ACT_TKN_W00_SHIFT                                                 0
#define IPE_ACT_TKN_W00_MASK                                         0xffffffff
#define IPE_ACT_TKN_W00_RD(src)                          (((src) & 0xffffffff))
#define IPE_ACT_TKN_W00_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W1	*/
/*	 Fields IPE_ACT_TKN_W1	 */
#define IPE_ACT_TKN_W11_WIDTH                                                32
#define IPE_ACT_TKN_W11_SHIFT                                                 0
#define IPE_ACT_TKN_W11_MASK                                         0xffffffff
#define IPE_ACT_TKN_W11_RD(src)                          (((src) & 0xffffffff))
#define IPE_ACT_TKN_W11_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W2	*/
/*	 Fields IPE_ACT_TKN_W2	 */
#define IPE_ACT_TKN_W22_WIDTH                                                32
#define IPE_ACT_TKN_W22_SHIFT                                                 0
#define IPE_ACT_TKN_W22_MASK                                         0xffffffff
#define IPE_ACT_TKN_W22_RD(src)                          (((src) & 0xffffffff))
#define IPE_ACT_TKN_W22_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W3	*/
/*	 Fields IPE_ACT_TKN_W3	 */
#define IPE_ACT_TKN_W33_WIDTH                                                32
#define IPE_ACT_TKN_W33_SHIFT                                                 0
#define IPE_ACT_TKN_W33_MASK                                         0xffffffff
#define IPE_ACT_TKN_W33_RD(src)                          (((src) & 0xffffffff))
#define IPE_ACT_TKN_W33_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W4	*/
/*	 Fields IPE_ACT_TKN_W4	 */
#define IPE_ACT_TKN_W44_WIDTH                                                32
#define IPE_ACT_TKN_W44_SHIFT                                                 0
#define IPE_ACT_TKN_W44_MASK                                         0xffffffff
#define IPE_ACT_TKN_W44_RD(src)                          (((src) & 0xffffffff))
#define IPE_ACT_TKN_W44_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W44_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W5	*/
/*	 Fields IPE_ACT_TKN_W5	 */
#define IPE_ACT_TKN_W55_WIDTH                                                32
#define IPE_ACT_TKN_W55_SHIFT                                                 0
#define IPE_ACT_TKN_W55_MASK                                         0xffffffff
#define IPE_ACT_TKN_W55_RD(src)                          (((src) & 0xffffffff))
#define IPE_ACT_TKN_W55_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W55_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W6	*/
/*	 Fields IPE_ACT_TKN_W6	 */
#define IPE_ACT_TKN_W66_WIDTH                                                32
#define IPE_ACT_TKN_W66_SHIFT                                                 0
#define IPE_ACT_TKN_W66_MASK                                         0xffffffff
#define IPE_ACT_TKN_W66_RD(src)                          (((src) & 0xffffffff))
#define IPE_ACT_TKN_W66_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W66_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W7	*/
/*	 Fields IPE_ACT_TKN_W7	 */
#define IPE_ACT_TKN_W77_WIDTH                                                32
#define IPE_ACT_TKN_W77_SHIFT                                                 0
#define IPE_ACT_TKN_W77_MASK                                         0xffffffff
#define IPE_ACT_TKN_W77_RD(src)                          (((src) & 0xffffffff))
#define IPE_ACT_TKN_W77_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W77_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W8	*/
/*	 Fields IPE_ACT_TKN_W8	 */
#define IPE_ACT_TKN_W88_WIDTH                                                32
#define IPE_ACT_TKN_W88_SHIFT                                                 0
#define IPE_ACT_TKN_W88_MASK                                         0xffffffff
#define IPE_ACT_TKN_W88_RD(src)                          (((src) & 0xffffffff))
#define IPE_ACT_TKN_W88_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W88_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W9	*/
/*	 Fields IPE_ACT_TKN_W9	 */
#define IPE_ACT_TKN_W99_WIDTH                                                32
#define IPE_ACT_TKN_W99_SHIFT                                                 0
#define IPE_ACT_TKN_W99_MASK                                         0xffffffff
#define IPE_ACT_TKN_W99_RD(src)                          (((src) & 0xffffffff))
#define IPE_ACT_TKN_W99_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W99_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W10	*/
/*	 Fields IPE_ACT_TKN_W10	 */
#define IPE_ACT_TKN_W100_WIDTH                                               32
#define IPE_ACT_TKN_W100_SHIFT                                                0
#define IPE_ACT_TKN_W100_MASK                                        0xffffffff
#define IPE_ACT_TKN_W100_RD(src)                         (((src) & 0xffffffff))
#define IPE_ACT_TKN_W100_WR(src)                    (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W100_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W11	*/
/*	 Fields IPE_ACT_TKN_W11	 */
#define IPE_ACT_TKN_W111_WIDTH                                               32
#define IPE_ACT_TKN_W111_SHIFT                                                0
#define IPE_ACT_TKN_W111_MASK                                        0xffffffff
#define IPE_ACT_TKN_W111_RD(src)                         (((src) & 0xffffffff))
#define IPE_ACT_TKN_W111_WR(src)                    (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W111_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W12	*/
/*	 Fields IPE_ACT_TKN_W12	 */
#define IPE_ACT_TKN_W122_WIDTH                                               32
#define IPE_ACT_TKN_W122_SHIFT                                                0
#define IPE_ACT_TKN_W122_MASK                                        0xffffffff
#define IPE_ACT_TKN_W122_RD(src)                         (((src) & 0xffffffff))
#define IPE_ACT_TKN_W122_WR(src)                    (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W122_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W13	*/
/*	 Fields IPE_ACT_TKN_W13	 */
#define IPE_ACT_TKN_W133_WIDTH                                               32
#define IPE_ACT_TKN_W133_SHIFT                                                0
#define IPE_ACT_TKN_W133_MASK                                        0xffffffff
#define IPE_ACT_TKN_W133_RD(src)                         (((src) & 0xffffffff))
#define IPE_ACT_TKN_W133_WR(src)                    (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W133_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W14	*/
/*	 Fields IPE_ACT_TKN_W14	 */
#define IPE_ACT_TKN_W144_WIDTH                                               32
#define IPE_ACT_TKN_W144_SHIFT                                                0
#define IPE_ACT_TKN_W144_MASK                                        0xffffffff
#define IPE_ACT_TKN_W144_RD(src)                         (((src) & 0xffffffff))
#define IPE_ACT_TKN_W144_WR(src)                    (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W144_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_TKN_W15	*/
/*	 Fields IPE_ACT_TKN_W15	 */
#define IPE_ACT_TKN_W155_WIDTH                                               32
#define IPE_ACT_TKN_W155_SHIFT                                                0
#define IPE_ACT_TKN_W155_MASK                                        0xffffffff
#define IPE_ACT_TKN_W155_RD(src)                         (((src) & 0xffffffff))
#define IPE_ACT_TKN_W155_WR(src)                    (((u32)(src)) & 0xffffffff)
#define IPE_ACT_TKN_W155_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W0_ACT_W16	*/
/*	 Fields IPE_NXT_TKN_W0_ACT_W16	 */
#define IPE_NXT_TKN_W0_ACT_W166_WIDTH                                        32
#define IPE_NXT_TKN_W0_ACT_W166_SHIFT                                         0
#define IPE_NXT_TKN_W0_ACT_W166_MASK                                 0xffffffff
#define IPE_NXT_TKN_W0_ACT_W166_RD(src)                  (((src) & 0xffffffff))
#define IPE_NXT_TKN_W0_ACT_W166_WR(src)             (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W0_ACT_W166_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W1_ACT_W17	*/
/*	 Fields IPE_NXT_TKN_W1_ACT_W17	 */
#define IPE_NXT_TKN_W1_ACT_W177_WIDTH                                        32
#define IPE_NXT_TKN_W1_ACT_W177_SHIFT                                         0
#define IPE_NXT_TKN_W1_ACT_W177_MASK                                 0xffffffff
#define IPE_NXT_TKN_W1_ACT_W177_RD(src)                  (((src) & 0xffffffff))
#define IPE_NXT_TKN_W1_ACT_W177_WR(src)             (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W1_ACT_W177_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W2_ACT_W18	*/
/*	 Fields IPE_NXT_TKN_W2_ACT_W18	 */
#define IPE_NXT_TKN_W2_ACT_W188_WIDTH                                        32
#define IPE_NXT_TKN_W2_ACT_W188_SHIFT                                         0
#define IPE_NXT_TKN_W2_ACT_W188_MASK                                 0xffffffff
#define IPE_NXT_TKN_W2_ACT_W188_RD(src)                  (((src) & 0xffffffff))
#define IPE_NXT_TKN_W2_ACT_W188_WR(src)             (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W2_ACT_W188_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W3_ACT_W19	*/
/*	 Fields IPE_NXT_TKN_W3_ACT_W19	 */
#define IPE_NXT_TKN_W3_ACT_W199_WIDTH                                        32
#define IPE_NXT_TKN_W3_ACT_W199_SHIFT                                         0
#define IPE_NXT_TKN_W3_ACT_W199_MASK                                 0xffffffff
#define IPE_NXT_TKN_W3_ACT_W199_RD(src)                  (((src) & 0xffffffff))
#define IPE_NXT_TKN_W3_ACT_W199_WR(src)             (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W3_ACT_W199_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W4_ACT_W20	*/
/*	 Fields IPE_NXT_TKN_W4_ACT_W20	 */
#define IPE_NXT_TKN_W4_ACT_W200_WIDTH                                        32
#define IPE_NXT_TKN_W4_ACT_W200_SHIFT                                         0
#define IPE_NXT_TKN_W4_ACT_W200_MASK                                 0xffffffff
#define IPE_NXT_TKN_W4_ACT_W200_RD(src)                  (((src) & 0xffffffff))
#define IPE_NXT_TKN_W4_ACT_W200_WR(src)             (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W4_ACT_W200_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W5_ACT_W21	*/
/*	 Fields IPE_NXT_TKN_W5_ACT_W21	 */
#define IPE_NXT_TKN_W5_ACT_W211_WIDTH                                        32
#define IPE_NXT_TKN_W5_ACT_W211_SHIFT                                         0
#define IPE_NXT_TKN_W5_ACT_W211_MASK                                 0xffffffff
#define IPE_NXT_TKN_W5_ACT_W211_RD(src)                  (((src) & 0xffffffff))
#define IPE_NXT_TKN_W5_ACT_W211_WR(src)             (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W5_ACT_W211_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W6_ACT_W22	*/
/*	 Fields IPE_NXT_TKN_W6_ACT_W22	 */
#define IPE_NXT_TKN_W6_ACT_W222_WIDTH                                        32
#define IPE_NXT_TKN_W6_ACT_W222_SHIFT                                         0
#define IPE_NXT_TKN_W6_ACT_W222_MASK                                 0xffffffff
#define IPE_NXT_TKN_W6_ACT_W222_RD(src)                  (((src) & 0xffffffff))
#define IPE_NXT_TKN_W6_ACT_W222_WR(src)             (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W6_ACT_W222_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W7_ACT_W23	*/
/*	 Fields IPE_NXT_TKN_W7_ACT_W23	 */
#define IPE_NXT_TKN_W7_ACT_W233_WIDTH                                        32
#define IPE_NXT_TKN_W7_ACT_W233_SHIFT                                         0
#define IPE_NXT_TKN_W7_ACT_W233_MASK                                 0xffffffff
#define IPE_NXT_TKN_W7_ACT_W233_RD(src)                  (((src) & 0xffffffff))
#define IPE_NXT_TKN_W7_ACT_W233_WR(src)             (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W7_ACT_W233_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W8_ACT_W24	*/
/*	 Fields IPE_NXT_TKN_W8_ACT_W24	 */
#define IPE_NXT_TKN_W8_ACT_W244_WIDTH                                        32
#define IPE_NXT_TKN_W8_ACT_W244_SHIFT                                         0
#define IPE_NXT_TKN_W8_ACT_W244_MASK                                 0xffffffff
#define IPE_NXT_TKN_W8_ACT_W244_RD(src)                  (((src) & 0xffffffff))
#define IPE_NXT_TKN_W8_ACT_W244_WR(src)             (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W8_ACT_W244_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W9_ACT_W25	*/
/*	 Fields IPE_NXT_TKN_W9_ACT_W25	 */
#define IPE_NXT_TKN_W9_ACT_W255_WIDTH                                        32
#define IPE_NXT_TKN_W9_ACT_W255_SHIFT                                         0
#define IPE_NXT_TKN_W9_ACT_W255_MASK                                 0xffffffff
#define IPE_NXT_TKN_W9_ACT_W255_RD(src)                  (((src) & 0xffffffff))
#define IPE_NXT_TKN_W9_ACT_W255_WR(src)             (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W9_ACT_W255_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W10_ACT_W26	*/
/*	 Fields IPE_NXT_TKN_W10_ACT_W26	 */
#define IPE_NXT_TKN_W10_ACT_W266_WIDTH                                       32
#define IPE_NXT_TKN_W10_ACT_W266_SHIFT                                        0
#define IPE_NXT_TKN_W10_ACT_W266_MASK                                0xffffffff
#define IPE_NXT_TKN_W10_ACT_W266_RD(src)                 (((src) & 0xffffffff))
#define IPE_NXT_TKN_W10_ACT_W266_WR(src)            (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W10_ACT_W266_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W11_ACT_W27	*/
/*	 Fields IPE_NXT_TKN_W11_ACT_W27	 */
#define IPE_NXT_TKN_W11_ACT_W277_WIDTH                                       32
#define IPE_NXT_TKN_W11_ACT_W277_SHIFT                                        0
#define IPE_NXT_TKN_W11_ACT_W277_MASK                                0xffffffff
#define IPE_NXT_TKN_W11_ACT_W277_RD(src)                 (((src) & 0xffffffff))
#define IPE_NXT_TKN_W11_ACT_W277_WR(src)            (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W11_ACT_W277_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W12_ACT_W28	*/
/*	 Fields IPE_NXT_TKN_W12_ACT_W28	 */
#define IPE_NXT_TKN_W12_ACT_W288_WIDTH                                       32
#define IPE_NXT_TKN_W12_ACT_W288_SHIFT                                        0
#define IPE_NXT_TKN_W12_ACT_W288_MASK                                0xffffffff
#define IPE_NXT_TKN_W12_ACT_W288_RD(src)                 (((src) & 0xffffffff))
#define IPE_NXT_TKN_W12_ACT_W288_WR(src)            (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W12_ACT_W288_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W13_ACT_W29	*/
/*	 Fields IPE_NXT_TKN_W13_ACT_W29	 */
#define IPE_NXT_TKN_W13_ACT_W299_WIDTH                                       32
#define IPE_NXT_TKN_W13_ACT_W299_SHIFT                                        0
#define IPE_NXT_TKN_W13_ACT_W299_MASK                                0xffffffff
#define IPE_NXT_TKN_W13_ACT_W299_RD(src)                 (((src) & 0xffffffff))
#define IPE_NXT_TKN_W13_ACT_W299_WR(src)            (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W13_ACT_W299_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W14_ACT_W30	*/
/*	 Fields IPE_NXT_TKN_W14_ACT_W30	 */
#define IPE_NXT_TKN_W14_ACT_W300_WIDTH                                       32
#define IPE_NXT_TKN_W14_ACT_W300_SHIFT                                        0
#define IPE_NXT_TKN_W14_ACT_W300_MASK                                0xffffffff
#define IPE_NXT_TKN_W14_ACT_W300_RD(src)                 (((src) & 0xffffffff))
#define IPE_NXT_TKN_W14_ACT_W300_WR(src)            (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W14_ACT_W300_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_TKN_W15_ACT_W31	*/
/*	 Fields IPE_NXT_TKN_W15_ACT_W31	 */
#define IPE_NXT_TKN_W15_ACT_W311_WIDTH                                       32
#define IPE_NXT_TKN_W15_ACT_W311_SHIFT                                        0
#define IPE_NXT_TKN_W15_ACT_W311_MASK                                0xffffffff
#define IPE_NXT_TKN_W15_ACT_W311_RD(src)                 (((src) & 0xffffffff))
#define IPE_NXT_TKN_W15_ACT_W311_WR(src)            (((u32)(src)) & 0xffffffff)
#define IPE_NXT_TKN_W15_ACT_W311_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_RES_TKN_W0	*/
/*	 Fields IPE_RES_TKN_W0	 */
#define IPE_RES_TKN_W00_WIDTH                                                32
#define IPE_RES_TKN_W00_SHIFT                                                 0
#define IPE_RES_TKN_W00_MASK                                         0xffffffff
#define IPE_RES_TKN_W00_RD(src)                          (((src) & 0xffffffff))
#define IPE_RES_TKN_W00_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_RES_TKN_W00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_RES_TKN_W1	*/
/*	 Fields IPE_RES_TKN_W1	 */
#define IPE_RES_TKN_W11_WIDTH                                                32
#define IPE_RES_TKN_W11_SHIFT                                                 0
#define IPE_RES_TKN_W11_MASK                                         0xffffffff
#define IPE_RES_TKN_W11_RD(src)                          (((src) & 0xffffffff))
#define IPE_RES_TKN_W11_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_RES_TKN_W11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_RES_TKN_W2	*/
/*	 Fields IPE_RES_TKN_W2	 */
#define IPE_RES_TKN_W22_WIDTH                                                32
#define IPE_RES_TKN_W22_SHIFT                                                 0
#define IPE_RES_TKN_W22_MASK                                         0xffffffff
#define IPE_RES_TKN_W22_RD(src)                          (((src) & 0xffffffff))
#define IPE_RES_TKN_W22_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_RES_TKN_W22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_RES_TKN_W3	*/
/*	 Fields IPE_RES_TKN_W3	 */
#define IPE_RES_TKN_W33_WIDTH                                                32
#define IPE_RES_TKN_W33_SHIFT                                                 0
#define IPE_RES_TKN_W33_MASK                                         0xffffffff
#define IPE_RES_TKN_W33_RD(src)                          (((src) & 0xffffffff))
#define IPE_RES_TKN_W33_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_RES_TKN_W33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_RES_TKN_W4	*/
/*	 Fields IPE_RES_TKN_W4	 */
#define IPE_RES_TKN_W44_WIDTH                                                32
#define IPE_RES_TKN_W44_SHIFT                                                 0
#define IPE_RES_TKN_W44_MASK                                         0xffffffff
#define IPE_RES_TKN_W44_RD(src)                          (((src) & 0xffffffff))
#define IPE_RES_TKN_W44_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_RES_TKN_W44_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_RES_TKN_W5	*/
/*	 Fields IPE_RES_TKN_W5	 */
#define IPE_RES_TKN_W55_WIDTH                                                32
#define IPE_RES_TKN_W55_SHIFT                                                 0
#define IPE_RES_TKN_W55_MASK                                         0xffffffff
#define IPE_RES_TKN_W55_RD(src)                          (((src) & 0xffffffff))
#define IPE_RES_TKN_W55_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_RES_TKN_W55_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_RES_TKN_W6	*/
/*	 Fields IPE_RES_TKN_W6	 */
#define IPE_RES_TKN_W66_WIDTH                                                32
#define IPE_RES_TKN_W66_SHIFT                                                 0
#define IPE_RES_TKN_W66_MASK                                         0xffffffff
#define IPE_RES_TKN_W66_RD(src)                          (((src) & 0xffffffff))
#define IPE_RES_TKN_W66_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_RES_TKN_W66_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_RES_TKN_W7	*/
/*	 Fields IPE_RES_TKN_W7	 */
#define IPE_RES_TKN_W77_WIDTH                                                32
#define IPE_RES_TKN_W77_SHIFT                                                 0
#define IPE_RES_TKN_W77_MASK                                         0xffffffff
#define IPE_RES_TKN_W77_RD(src)                          (((src) & 0xffffffff))
#define IPE_RES_TKN_W77_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_RES_TKN_W77_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_CTX_CMD0	*/
/*	 Fields IPE_NXT_CTX_CMD0	 */
#define IPE_NXT_CTX_CMD00_WIDTH                                              32
#define IPE_NXT_CTX_CMD00_SHIFT                                               0
#define IPE_NXT_CTX_CMD00_MASK                                       0xffffffff
#define IPE_NXT_CTX_CMD00_RD(src)                        (((src) & 0xffffffff))
#define IPE_NXT_CTX_CMD00_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_NXT_CTX_CMD00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_CTX_CMD1	*/
/*	 Fields IPE_NXT_CTX_CMD1	 */
#define IPE_NXT_CTX_CMD11_WIDTH                                              32
#define IPE_NXT_CTX_CMD11_SHIFT                                               0
#define IPE_NXT_CTX_CMD11_MASK                                       0xffffffff
#define IPE_NXT_CTX_CMD11_RD(src)                        (((src) & 0xffffffff))
#define IPE_NXT_CTX_CMD11_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_NXT_CTX_CMD11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_GPR0	*/
/*	 Fields IPE_NXT_GPR0	 */
#define IPE_NXT_GPR00_WIDTH                                                  32
#define IPE_NXT_GPR00_SHIFT                                                   0
#define IPE_NXT_GPR00_MASK                                           0xffffffff
#define IPE_NXT_GPR00_RD(src)                            (((src) & 0xffffffff))
#define IPE_NXT_GPR00_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_NXT_GPR00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_GPR1	*/
/*	 Fields IPE_NXT_GPR1	 */
#define IPE_NXT_GPR11_WIDTH                                                  32
#define IPE_NXT_GPR11_SHIFT                                                   0
#define IPE_NXT_GPR11_MASK                                           0xffffffff
#define IPE_NXT_GPR11_RD(src)                            (((src) & 0xffffffff))
#define IPE_NXT_GPR11_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_NXT_GPR11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IV0	*/
/*	 Fields IPE_NXT_IV0	 */
#define IPE_NXT_IV00_WIDTH                                                   32
#define IPE_NXT_IV00_SHIFT                                                    0
#define IPE_NXT_IV00_MASK                                            0xffffffff
#define IPE_NXT_IV00_RD(src)                             (((src) & 0xffffffff))
#define IPE_NXT_IV00_WR(src)                        (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IV00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IV1	*/
/*	 Fields IPE_NXT_IV1	 */
#define IPE_NXT_IV11_WIDTH                                                   32
#define IPE_NXT_IV11_SHIFT                                                    0
#define IPE_NXT_IV11_MASK                                            0xffffffff
#define IPE_NXT_IV11_RD(src)                             (((src) & 0xffffffff))
#define IPE_NXT_IV11_WR(src)                        (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IV11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IV2	*/
/*	 Fields IPE_NXT_IV2	 */
#define IPE_NXT_IV22_WIDTH                                                   32
#define IPE_NXT_IV22_SHIFT                                                    0
#define IPE_NXT_IV22_MASK                                            0xffffffff
#define IPE_NXT_IV22_RD(src)                             (((src) & 0xffffffff))
#define IPE_NXT_IV22_WR(src)                        (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IV22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IV3	*/
/*	 Fields IPE_NXT_IV3	 */
#define IPE_NXT_IV33_WIDTH                                                   32
#define IPE_NXT_IV33_SHIFT                                                    0
#define IPE_NXT_IV33_MASK                                            0xffffffff
#define IPE_NXT_IV33_RD(src)                             (((src) & 0xffffffff))
#define IPE_NXT_IV33_WR(src)                        (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IV33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_KEY0	*/
/*	 Fields IPE_NXT_KEY0	 */
#define IPE_NXT_KEY00_WIDTH                                                  32
#define IPE_NXT_KEY00_SHIFT                                                   0
#define IPE_NXT_KEY00_MASK                                           0xffffffff
#define IPE_NXT_KEY00_RD(src)                            (((src) & 0xffffffff))
#define IPE_NXT_KEY00_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_NXT_KEY00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_KEY1	*/
/*	 Fields IPE_NXT_KEY1	 */
#define IPE_NXT_KEY11_WIDTH                                                  32
#define IPE_NXT_KEY11_SHIFT                                                   0
#define IPE_NXT_KEY11_MASK                                           0xffffffff
#define IPE_NXT_KEY11_RD(src)                            (((src) & 0xffffffff))
#define IPE_NXT_KEY11_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_NXT_KEY11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_KEY2	*/
/*	 Fields IPE_NXT_KEY2	 */
#define IPE_NXT_KEY22_WIDTH                                                  32
#define IPE_NXT_KEY22_SHIFT                                                   0
#define IPE_NXT_KEY22_MASK                                           0xffffffff
#define IPE_NXT_KEY22_RD(src)                            (((src) & 0xffffffff))
#define IPE_NXT_KEY22_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_NXT_KEY22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_KEY3	*/
/*	 Fields IPE_NXT_KEY3	 */
#define IPE_NXT_KEY33_WIDTH                                                  32
#define IPE_NXT_KEY33_SHIFT                                                   0
#define IPE_NXT_KEY33_MASK                                           0xffffffff
#define IPE_NXT_KEY33_RD(src)                            (((src) & 0xffffffff))
#define IPE_NXT_KEY33_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_NXT_KEY33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_KEY4	*/
/*	 Fields IPE_NXT_KEY4	 */
#define IPE_NXT_KEY44_WIDTH                                                  32
#define IPE_NXT_KEY44_SHIFT                                                   0
#define IPE_NXT_KEY44_MASK                                           0xffffffff
#define IPE_NXT_KEY44_RD(src)                            (((src) & 0xffffffff))
#define IPE_NXT_KEY44_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_NXT_KEY44_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_KEY5	*/
/*	 Fields IPE_NXT_KEY5	 */
#define IPE_NXT_KEY55_WIDTH                                                  32
#define IPE_NXT_KEY55_SHIFT                                                   0
#define IPE_NXT_KEY55_MASK                                           0xffffffff
#define IPE_NXT_KEY55_RD(src)                            (((src) & 0xffffffff))
#define IPE_NXT_KEY55_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_NXT_KEY55_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_KEY6	*/
/*	 Fields IPE_NXT_KEY6	 */
#define IPE_NXT_KEY66_WIDTH                                                  32
#define IPE_NXT_KEY66_SHIFT                                                   0
#define IPE_NXT_KEY66_MASK                                           0xffffffff
#define IPE_NXT_KEY66_RD(src)                            (((src) & 0xffffffff))
#define IPE_NXT_KEY66_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_NXT_KEY66_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_KEY7	*/
/*	 Fields IPE_NXT_KEY7	 */
#define IPE_NXT_KEY77_WIDTH                                                  32
#define IPE_NXT_KEY77_SHIFT                                                   0
#define IPE_NXT_KEY77_MASK                                           0xffffffff
#define IPE_NXT_KEY77_RD(src)                            (((src) & 0xffffffff))
#define IPE_NXT_KEY77_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_NXT_KEY77_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST0	*/
/*	 Fields IPE_NXT_IN_DGST0	 */
#define IPE_NXT_IN_DGST00_WIDTH                                              32
#define IPE_NXT_IN_DGST00_SHIFT                                               0
#define IPE_NXT_IN_DGST00_MASK                                       0xffffffff
#define IPE_NXT_IN_DGST00_RD(src)                        (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST00_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST1	*/
/*	 Fields IPE_NXT_IN_DGST1	 */
#define IPE_NXT_IN_DGST11_WIDTH                                              32
#define IPE_NXT_IN_DGST11_SHIFT                                               0
#define IPE_NXT_IN_DGST11_MASK                                       0xffffffff
#define IPE_NXT_IN_DGST11_RD(src)                        (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST11_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST2	*/
/*	 Fields IPE_NXT_IN_DGST2	 */
#define IPE_NXT_IN_DGST22_WIDTH                                              32
#define IPE_NXT_IN_DGST22_SHIFT                                               0
#define IPE_NXT_IN_DGST22_MASK                                       0xffffffff
#define IPE_NXT_IN_DGST22_RD(src)                        (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST22_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST3	*/
/*	 Fields IPE_NXT_IN_DGST3	 */
#define IPE_NXT_IN_DGST33_WIDTH                                              32
#define IPE_NXT_IN_DGST33_SHIFT                                               0
#define IPE_NXT_IN_DGST33_MASK                                       0xffffffff
#define IPE_NXT_IN_DGST33_RD(src)                        (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST33_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST4	*/
/*	 Fields IPE_NXT_IN_DGST4	 */
#define IPE_NXT_IN_DGST44_WIDTH                                              32
#define IPE_NXT_IN_DGST44_SHIFT                                               0
#define IPE_NXT_IN_DGST44_MASK                                       0xffffffff
#define IPE_NXT_IN_DGST44_RD(src)                        (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST44_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST44_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST5	*/
/*	 Fields IPE_NXT_IN_DGST5	 */
#define IPE_NXT_IN_DGST55_WIDTH                                              32
#define IPE_NXT_IN_DGST55_SHIFT                                               0
#define IPE_NXT_IN_DGST55_MASK                                       0xffffffff
#define IPE_NXT_IN_DGST55_RD(src)                        (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST55_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST55_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST6	*/
/*	 Fields IPE_NXT_IN_DGST6	 */
#define IPE_NXT_IN_DGST66_WIDTH                                              32
#define IPE_NXT_IN_DGST66_SHIFT                                               0
#define IPE_NXT_IN_DGST66_MASK                                       0xffffffff
#define IPE_NXT_IN_DGST66_RD(src)                        (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST66_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST66_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST7	*/
/*	 Fields IPE_NXT_IN_DGST7	 */
#define IPE_NXT_IN_DGST77_WIDTH                                              32
#define IPE_NXT_IN_DGST77_SHIFT                                               0
#define IPE_NXT_IN_DGST77_MASK                                       0xffffffff
#define IPE_NXT_IN_DGST77_RD(src)                        (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST77_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST77_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST0	*/
/*	 Fields IPE_NXT_OUT_DGST0	 */
#define IPE_NXT_OUT_DGST00_WIDTH                                             32
#define IPE_NXT_OUT_DGST00_SHIFT                                              0
#define IPE_NXT_OUT_DGST00_MASK                                      0xffffffff
#define IPE_NXT_OUT_DGST00_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST00_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST1	*/
/*	 Fields IPE_NXT_OUT_DGST1	 */
#define IPE_NXT_OUT_DGST11_WIDTH                                             32
#define IPE_NXT_OUT_DGST11_SHIFT                                              0
#define IPE_NXT_OUT_DGST11_MASK                                      0xffffffff
#define IPE_NXT_OUT_DGST11_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST11_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST2	*/
/*	 Fields IPE_NXT_OUT_DGST2	 */
#define IPE_NXT_OUT_DGST22_WIDTH                                             32
#define IPE_NXT_OUT_DGST22_SHIFT                                              0
#define IPE_NXT_OUT_DGST22_MASK                                      0xffffffff
#define IPE_NXT_OUT_DGST22_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST22_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST3	*/
/*	 Fields IPE_NXT_OUT_DGST3	 */
#define IPE_NXT_OUT_DGST33_WIDTH                                             32
#define IPE_NXT_OUT_DGST33_SHIFT                                              0
#define IPE_NXT_OUT_DGST33_MASK                                      0xffffffff
#define IPE_NXT_OUT_DGST33_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST33_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST4	*/
/*	 Fields IPE_NXT_OUT_DGST4	 */
#define IPE_NXT_OUT_DGST44_WIDTH                                             32
#define IPE_NXT_OUT_DGST44_SHIFT                                              0
#define IPE_NXT_OUT_DGST44_MASK                                      0xffffffff
#define IPE_NXT_OUT_DGST44_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST44_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST44_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST5	*/
/*	 Fields IPE_NXT_OUT_DGST5	 */
#define IPE_NXT_OUT_DGST55_WIDTH                                             32
#define IPE_NXT_OUT_DGST55_SHIFT                                              0
#define IPE_NXT_OUT_DGST55_MASK                                      0xffffffff
#define IPE_NXT_OUT_DGST55_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST55_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST55_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST6	*/
/*	 Fields IPE_NXT_OUT_DGST6	 */
#define IPE_NXT_OUT_DGST66_WIDTH                                             32
#define IPE_NXT_OUT_DGST66_SHIFT                                              0
#define IPE_NXT_OUT_DGST66_MASK                                      0xffffffff
#define IPE_NXT_OUT_DGST66_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST66_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST66_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST7	*/
/*	 Fields IPE_NXT_OUT_DGST7	 */
#define IPE_NXT_OUT_DGST77_WIDTH                                             32
#define IPE_NXT_OUT_DGST77_SHIFT                                              0
#define IPE_NXT_OUT_DGST77_MASK                                      0xffffffff
#define IPE_NXT_OUT_DGST77_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST77_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST77_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_DGST_CNT	*/
/*	 Fields IPE_NXT_DGST_CNT	 */
#define IPE_NXT_DGST_CNT_WIDTH                                               32
#define IPE_NXT_DGST_CNT_SHIFT                                                0
#define IPE_NXT_DGST_CNT_MASK                                        0xffffffff
#define IPE_NXT_DGST_CNT_RD(src)                         (((src) & 0xffffffff))
#define IPE_NXT_DGST_CNT_WR(src)                    (((u32)(src)) & 0xffffffff)
#define IPE_NXT_DGST_CNT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_SPI_SSRC	*/
/*	 Fields IPE_NXT_SPI_SSRC	 */
#define IPE_NXT_SPI_SSRC_WIDTH                                               32
#define IPE_NXT_SPI_SSRC_SHIFT                                                0
#define IPE_NXT_SPI_SSRC_MASK                                        0xffffffff
#define IPE_NXT_SPI_SSRC_RD(src)                         (((src) & 0xffffffff))
#define IPE_NXT_SPI_SSRC_WR(src)                    (((u32)(src)) & 0xffffffff)
#define IPE_NXT_SPI_SSRC_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_SN	*/
/*	 Fields IPE_NXT_SN	 */
#define IPE_NXT_SN_WIDTH                                                     32
#define IPE_NXT_SN_SHIFT                                                      0
#define IPE_NXT_SN_MASK                                              0xffffffff
#define IPE_NXT_SN_RD(src)                               (((src) & 0xffffffff))
#define IPE_NXT_SN_WR(src)                          (((u32)(src)) & 0xffffffff)
#define IPE_NXT_SN_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_ESN	*/
/*	 Fields IPE_NXT_ESN	 */
#define IPE_NXT_ESN_WIDTH                                                    32
#define IPE_NXT_ESN_SHIFT                                                     0
#define IPE_NXT_ESN_MASK                                             0xffffffff
#define IPE_NXT_ESN_RD(src)                              (((src) & 0xffffffff))
#define IPE_NXT_ESN_WR(src)                         (((u32)(src)) & 0xffffffff)
#define IPE_NXT_ESN_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_SN_M0	*/
/*	 Fields IPE_NXT_SN_M0	 */
#define IPE_NXT_SN_M00_WIDTH                                                 32
#define IPE_NXT_SN_M00_SHIFT                                                  0
#define IPE_NXT_SN_M00_MASK                                          0xffffffff
#define IPE_NXT_SN_M00_RD(src)                           (((src) & 0xffffffff))
#define IPE_NXT_SN_M00_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_NXT_SN_M00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_SN_M1	*/
/*	 Fields IPE_NXT_SN_M1	 */
#define IPE_NXT_SN_M11_WIDTH                                                 32
#define IPE_NXT_SN_M11_SHIFT                                                  0
#define IPE_NXT_SN_M11_MASK                                          0xffffffff
#define IPE_NXT_SN_M11_RD(src)                           (((src) & 0xffffffff))
#define IPE_NXT_SN_M11_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_NXT_SN_M11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_SN_M2	*/
/*	 Fields IPE_NXT_SN_M2	 */
#define IPE_NXT_SN_M22_WIDTH                                                 32
#define IPE_NXT_SN_M22_SHIFT                                                  0
#define IPE_NXT_SN_M22_MASK                                          0xffffffff
#define IPE_NXT_SN_M22_RD(src)                           (((src) & 0xffffffff))
#define IPE_NXT_SN_M22_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_NXT_SN_M22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_SN_M3	*/
/*	 Fields IPE_NXT_SN_M3	 */
#define IPE_NXT_SN_M33_WIDTH                                                 32
#define IPE_NXT_SN_M33_SHIFT                                                  0
#define IPE_NXT_SN_M33_MASK                                          0xffffffff
#define IPE_NXT_SN_M33_RD(src)                           (((src) & 0xffffffff))
#define IPE_NXT_SN_M33_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_NXT_SN_M33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_CS	*/
/*	 Fields IPE_NXT_CS	 */
#define IPE_NXT_CS_WIDTH                                                     32
#define IPE_NXT_CS_SHIFT                                                      0
#define IPE_NXT_CS_MASK                                              0xffffffff
#define IPE_NXT_CS_RD(src)                               (((src) & 0xffffffff))
#define IPE_NXT_CS_WR(src)                          (((u32)(src)) & 0xffffffff)
#define IPE_NXT_CS_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_UP_PKT_LEN	*/
/*	 Fields IPE_NXT_UP_PKT_LEN	 */
#define IPE_NXT_UP_PKT_LEN_WIDTH                                             32
#define IPE_NXT_UP_PKT_LEN_SHIFT                                              0
#define IPE_NXT_UP_PKT_LEN_MASK                                      0xffffffff
#define IPE_NXT_UP_PKT_LEN_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_UP_PKT_LEN_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_UP_PKT_LEN_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_CTX_CMD0	*/
/*	 Fields IPE_ACT_CTX_CMD0	 */
#define IPE_ACT_CTX_CMD00_WIDTH                                              32
#define IPE_ACT_CTX_CMD00_SHIFT                                               0
#define IPE_ACT_CTX_CMD00_MASK                                       0xffffffff
#define IPE_ACT_CTX_CMD00_RD(src)                        (((src) & 0xffffffff))
#define IPE_ACT_CTX_CMD00_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_ACT_CTX_CMD00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_CTX_CMD1	*/
/*	 Fields IPE_ACT_CTX_CMD1	 */
#define IPE_ACT_CTX_CMD11_WIDTH                                              32
#define IPE_ACT_CTX_CMD11_SHIFT                                               0
#define IPE_ACT_CTX_CMD11_MASK                                       0xffffffff
#define IPE_ACT_CTX_CMD11_RD(src)                        (((src) & 0xffffffff))
#define IPE_ACT_CTX_CMD11_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_ACT_CTX_CMD11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_GPR0	*/
/*	 Fields IPE_ACT_GPR0	 */
#define IPE_ACT_GPR00_WIDTH                                                  32
#define IPE_ACT_GPR00_SHIFT                                                   0
#define IPE_ACT_GPR00_MASK                                           0xffffffff
#define IPE_ACT_GPR00_RD(src)                            (((src) & 0xffffffff))
#define IPE_ACT_GPR00_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_ACT_GPR00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ACT_GPR1	*/
/*	 Fields IPE_ACT_GPR1	 */
#define IPE_ACT_GPR11_WIDTH                                                  32
#define IPE_ACT_GPR11_SHIFT                                                   0
#define IPE_ACT_GPR11_MASK                                           0xffffffff
#define IPE_ACT_GPR11_RD(src)                            (((src) & 0xffffffff))
#define IPE_ACT_GPR11_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_ACT_GPR11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IV0	*/
/*	 Fields IPE_IV0	 */
#define IPE_IV00_WIDTH                                                       32
#define IPE_IV00_SHIFT                                                        0
#define IPE_IV00_MASK                                                0xffffffff
#define IPE_IV00_RD(src)                                 (((src) & 0xffffffff))
#define IPE_IV00_WR(src)                            (((u32)(src)) & 0xffffffff)
#define IPE_IV00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IV1	*/
/*	 Fields IPE_IV1	 */
#define IPE_IV11_WIDTH                                                       32
#define IPE_IV11_SHIFT                                                        0
#define IPE_IV11_MASK                                                0xffffffff
#define IPE_IV11_RD(src)                                 (((src) & 0xffffffff))
#define IPE_IV11_WR(src)                            (((u32)(src)) & 0xffffffff)
#define IPE_IV11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IV2	*/
/*	 Fields IPE_IV2	 */
#define IPE_IV22_WIDTH                                                       32
#define IPE_IV22_SHIFT                                                        0
#define IPE_IV22_MASK                                                0xffffffff
#define IPE_IV22_RD(src)                                 (((src) & 0xffffffff))
#define IPE_IV22_WR(src)                            (((u32)(src)) & 0xffffffff)
#define IPE_IV22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IV3	*/
/*	 Fields IPE_IV3	 */
#define IPE_IV33_WIDTH                                                       32
#define IPE_IV33_SHIFT                                                        0
#define IPE_IV33_MASK                                                0xffffffff
#define IPE_IV33_RD(src)                                 (((src) & 0xffffffff))
#define IPE_IV33_WR(src)                            (((u32)(src)) & 0xffffffff)
#define IPE_IV33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_KEY0	*/
/*	 Fields IPE_KEY0	 */
#define IPE_KEY00_WIDTH                                                      32
#define IPE_KEY00_SHIFT                                                       0
#define IPE_KEY00_MASK                                               0xffffffff
#define IPE_KEY00_RD(src)                                (((src) & 0xffffffff))
#define IPE_KEY00_WR(src)                           (((u32)(src)) & 0xffffffff)
#define IPE_KEY00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_KEY1	*/
/*	 Fields IPE_KEY1	 */
#define IPE_KEY11_WIDTH                                                      32
#define IPE_KEY11_SHIFT                                                       0
#define IPE_KEY11_MASK                                               0xffffffff
#define IPE_KEY11_RD(src)                                (((src) & 0xffffffff))
#define IPE_KEY11_WR(src)                           (((u32)(src)) & 0xffffffff)
#define IPE_KEY11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_KEY2	*/
/*	 Fields IPE_KEY2	 */
#define IPE_KEY22_WIDTH                                                      32
#define IPE_KEY22_SHIFT                                                       0
#define IPE_KEY22_MASK                                               0xffffffff
#define IPE_KEY22_RD(src)                                (((src) & 0xffffffff))
#define IPE_KEY22_WR(src)                           (((u32)(src)) & 0xffffffff)
#define IPE_KEY22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_KEY3	*/
/*	 Fields IPE_KEY3	 */
#define IPE_KEY33_WIDTH                                                      32
#define IPE_KEY33_SHIFT                                                       0
#define IPE_KEY33_MASK                                               0xffffffff
#define IPE_KEY33_RD(src)                                (((src) & 0xffffffff))
#define IPE_KEY33_WR(src)                           (((u32)(src)) & 0xffffffff)
#define IPE_KEY33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_KEY4	*/
/*	 Fields IPE_KEY4	 */
#define IPE_KEY44_WIDTH                                                      32
#define IPE_KEY44_SHIFT                                                       0
#define IPE_KEY44_MASK                                               0xffffffff
#define IPE_KEY44_RD(src)                                (((src) & 0xffffffff))
#define IPE_KEY44_WR(src)                           (((u32)(src)) & 0xffffffff)
#define IPE_KEY44_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_KEY5	*/
/*	 Fields IPE_KEY5	 */
#define IPE_KEY55_WIDTH                                                      32
#define IPE_KEY55_SHIFT                                                       0
#define IPE_KEY55_MASK                                               0xffffffff
#define IPE_KEY55_RD(src)                                (((src) & 0xffffffff))
#define IPE_KEY55_WR(src)                           (((u32)(src)) & 0xffffffff)
#define IPE_KEY55_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_KEY6	*/
/*	 Fields IPE_KEY6	 */
#define IPE_KEY66_WIDTH                                                      32
#define IPE_KEY66_SHIFT                                                       0
#define IPE_KEY66_MASK                                               0xffffffff
#define IPE_KEY66_RD(src)                                (((src) & 0xffffffff))
#define IPE_KEY66_WR(src)                           (((u32)(src)) & 0xffffffff)
#define IPE_KEY66_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_KEY7	*/
/*	 Fields IPE_KEY7	 */
#define IPE_KEY77_WIDTH                                                      32
#define IPE_KEY77_SHIFT                                                       0
#define IPE_KEY77_MASK                                               0xffffffff
#define IPE_KEY77_RD(src)                                (((src) & 0xffffffff))
#define IPE_KEY77_WR(src)                           (((u32)(src)) & 0xffffffff)
#define IPE_KEY77_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST0	*/
/*	 Fields IPE_IN_DGST0	 */
#define IPE_IN_DGST00_WIDTH                                                  32
#define IPE_IN_DGST00_SHIFT                                                   0
#define IPE_IN_DGST00_MASK                                           0xffffffff
#define IPE_IN_DGST00_RD(src)                            (((src) & 0xffffffff))
#define IPE_IN_DGST00_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST1	*/
/*	 Fields IPE_IN_DGST1	 */
#define IPE_IN_DGST11_WIDTH                                                  32
#define IPE_IN_DGST11_SHIFT                                                   0
#define IPE_IN_DGST11_MASK                                           0xffffffff
#define IPE_IN_DGST11_RD(src)                            (((src) & 0xffffffff))
#define IPE_IN_DGST11_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST2	*/
/*	 Fields IPE_IN_DGST2	 */
#define IPE_IN_DGST22_WIDTH                                                  32
#define IPE_IN_DGST22_SHIFT                                                   0
#define IPE_IN_DGST22_MASK                                           0xffffffff
#define IPE_IN_DGST22_RD(src)                            (((src) & 0xffffffff))
#define IPE_IN_DGST22_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST3	*/
/*	 Fields IPE_IN_DGST3	 */
#define IPE_IN_DGST33_WIDTH                                                  32
#define IPE_IN_DGST33_SHIFT                                                   0
#define IPE_IN_DGST33_MASK                                           0xffffffff
#define IPE_IN_DGST33_RD(src)                            (((src) & 0xffffffff))
#define IPE_IN_DGST33_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST4	*/
/*	 Fields IPE_IN_DGST4	 */
#define IPE_IN_DGST44_WIDTH                                                  32
#define IPE_IN_DGST44_SHIFT                                                   0
#define IPE_IN_DGST44_MASK                                           0xffffffff
#define IPE_IN_DGST44_RD(src)                            (((src) & 0xffffffff))
#define IPE_IN_DGST44_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST44_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST5	*/
/*	 Fields IPE_IN_DGST5	 */
#define IPE_IN_DGST55_WIDTH                                                  32
#define IPE_IN_DGST55_SHIFT                                                   0
#define IPE_IN_DGST55_MASK                                           0xffffffff
#define IPE_IN_DGST55_RD(src)                            (((src) & 0xffffffff))
#define IPE_IN_DGST55_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST55_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST6	*/
/*	 Fields IPE_IN_DGST6	 */
#define IPE_IN_DGST66_WIDTH                                                  32
#define IPE_IN_DGST66_SHIFT                                                   0
#define IPE_IN_DGST66_MASK                                           0xffffffff
#define IPE_IN_DGST66_RD(src)                            (((src) & 0xffffffff))
#define IPE_IN_DGST66_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST66_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST7	*/
/*	 Fields IPE_IN_DGST7	 */
#define IPE_IN_DGST77_WIDTH                                                  32
#define IPE_IN_DGST77_SHIFT                                                   0
#define IPE_IN_DGST77_MASK                                           0xffffffff
#define IPE_IN_DGST77_RD(src)                            (((src) & 0xffffffff))
#define IPE_IN_DGST77_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST77_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST0	*/
/*	 Fields IPE_OUT_DGST0	 */
#define IPE_OUT_DGST00_WIDTH                                                 32
#define IPE_OUT_DGST00_SHIFT                                                  0
#define IPE_OUT_DGST00_MASK                                          0xffffffff
#define IPE_OUT_DGST00_RD(src)                           (((src) & 0xffffffff))
#define IPE_OUT_DGST00_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST1	*/
/*	 Fields IPE_OUT_DGST1	 */
#define IPE_OUT_DGST11_WIDTH                                                 32
#define IPE_OUT_DGST11_SHIFT                                                  0
#define IPE_OUT_DGST11_MASK                                          0xffffffff
#define IPE_OUT_DGST11_RD(src)                           (((src) & 0xffffffff))
#define IPE_OUT_DGST11_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST2	*/
/*	 Fields IPE_OUT_DGST2	 */
#define IPE_OUT_DGST22_WIDTH                                                 32
#define IPE_OUT_DGST22_SHIFT                                                  0
#define IPE_OUT_DGST22_MASK                                          0xffffffff
#define IPE_OUT_DGST22_RD(src)                           (((src) & 0xffffffff))
#define IPE_OUT_DGST22_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST3	*/
/*	 Fields IPE_OUT_DGST3	 */
#define IPE_OUT_DGST33_WIDTH                                                 32
#define IPE_OUT_DGST33_SHIFT                                                  0
#define IPE_OUT_DGST33_MASK                                          0xffffffff
#define IPE_OUT_DGST33_RD(src)                           (((src) & 0xffffffff))
#define IPE_OUT_DGST33_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST4	*/
/*	 Fields IPE_OUT_DGST4	 */
#define IPE_OUT_DGST44_WIDTH                                                 32
#define IPE_OUT_DGST44_SHIFT                                                  0
#define IPE_OUT_DGST44_MASK                                          0xffffffff
#define IPE_OUT_DGST44_RD(src)                           (((src) & 0xffffffff))
#define IPE_OUT_DGST44_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST44_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST5	*/
/*	 Fields IPE_OUT_DGST5	 */
#define IPE_OUT_DGST55_WIDTH                                                 32
#define IPE_OUT_DGST55_SHIFT                                                  0
#define IPE_OUT_DGST55_MASK                                          0xffffffff
#define IPE_OUT_DGST55_RD(src)                           (((src) & 0xffffffff))
#define IPE_OUT_DGST55_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST55_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST6	*/
/*	 Fields IPE_OUT_DGST6	 */
#define IPE_OUT_DGST66_WIDTH                                                 32
#define IPE_OUT_DGST66_SHIFT                                                  0
#define IPE_OUT_DGST66_MASK                                          0xffffffff
#define IPE_OUT_DGST66_RD(src)                           (((src) & 0xffffffff))
#define IPE_OUT_DGST66_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST66_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST7	*/
/*	 Fields IPE_OUT_DGST7	 */
#define IPE_OUT_DGST77_WIDTH                                                 32
#define IPE_OUT_DGST77_SHIFT                                                  0
#define IPE_OUT_DGST77_MASK                                          0xffffffff
#define IPE_OUT_DGST77_RD(src)                           (((src) & 0xffffffff))
#define IPE_OUT_DGST77_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST77_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_DGST_CNT	*/
/*	 Fields IPE_DGST_CNT	 */
#define IPE_DGST_CNT_WIDTH                                                   32
#define IPE_DGST_CNT_SHIFT                                                    0
#define IPE_DGST_CNT_MASK                                            0xffffffff
#define IPE_DGST_CNT_RD(src)                             (((src) & 0xffffffff))
#define IPE_DGST_CNT_WR(src)                        (((u32)(src)) & 0xffffffff)
#define IPE_DGST_CNT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_SPI_SSRC	*/
/*	 Fields IPE_SPI_SSRC	 */
#define IPE_SPI_SSRC_WIDTH                                                   32
#define IPE_SPI_SSRC_SHIFT                                                    0
#define IPE_SPI_SSRC_MASK                                            0xffffffff
#define IPE_SPI_SSRC_RD(src)                             (((src) & 0xffffffff))
#define IPE_SPI_SSRC_WR(src)                        (((u32)(src)) & 0xffffffff)
#define IPE_SPI_SSRC_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_SN	*/
/*	 Fields IPE_SN	 */
#define IPE_SN_WIDTH                                                         32
#define IPE_SN_SHIFT                                                          0
#define IPE_SN_MASK                                                  0xffffffff
#define IPE_SN_RD(src)                                   (((src) & 0xffffffff))
#define IPE_SN_WR(src)                              (((u32)(src)) & 0xffffffff)
#define IPE_SN_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ESN	*/
/*	 Fields IPE_ESN	 */
#define IPE_ESN_WIDTH                                                        32
#define IPE_ESN_SHIFT                                                         0
#define IPE_ESN_MASK                                                 0xffffffff
#define IPE_ESN_RD(src)                                  (((src) & 0xffffffff))
#define IPE_ESN_WR(src)                             (((u32)(src)) & 0xffffffff)
#define IPE_ESN_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_SN_M0	*/
/*	 Fields IPE_SN_M0	 */
#define IPE_SN_M00_WIDTH                                                     32
#define IPE_SN_M00_SHIFT                                                      0
#define IPE_SN_M00_MASK                                              0xffffffff
#define IPE_SN_M00_RD(src)                               (((src) & 0xffffffff))
#define IPE_SN_M00_WR(src)                          (((u32)(src)) & 0xffffffff)
#define IPE_SN_M00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_SN_M1	*/
/*	 Fields IPE_SN_M1	 */
#define IPE_SN_M11_WIDTH                                                     32
#define IPE_SN_M11_SHIFT                                                      0
#define IPE_SN_M11_MASK                                              0xffffffff
#define IPE_SN_M11_RD(src)                               (((src) & 0xffffffff))
#define IPE_SN_M11_WR(src)                          (((u32)(src)) & 0xffffffff)
#define IPE_SN_M11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_SN_M2	*/
/*	 Fields IPE_SN_M2	 */
#define IPE_SN_M22_WIDTH                                                     32
#define IPE_SN_M22_SHIFT                                                      0
#define IPE_SN_M22_MASK                                              0xffffffff
#define IPE_SN_M22_RD(src)                               (((src) & 0xffffffff))
#define IPE_SN_M22_WR(src)                          (((u32)(src)) & 0xffffffff)
#define IPE_SN_M22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_SN_M3	*/
/*	 Fields IPE_SN_M3	 */
#define IPE_SN_M33_WIDTH                                                     32
#define IPE_SN_M33_SHIFT                                                      0
#define IPE_SN_M33_MASK                                              0xffffffff
#define IPE_SN_M33_RD(src)                               (((src) & 0xffffffff))
#define IPE_SN_M33_WR(src)                          (((u32)(src)) & 0xffffffff)
#define IPE_SN_M33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_CS	*/
/*	 Fields IPE_CS	 */
#define IPE_CS_WIDTH                                                         32
#define IPE_CS_SHIFT                                                          0
#define IPE_CS_MASK                                                  0xffffffff
#define IPE_CS_RD(src)                                   (((src) & 0xffffffff))
#define IPE_CS_WR(src)                              (((u32)(src)) & 0xffffffff)
#define IPE_CS_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_UP_PKT_LEN	*/
/*	 Fields IPE_UP_PKT_LEN	 */
#define IPE_UP_PKT_LEN_WIDTH                                                 32
#define IPE_UP_PKT_LEN_SHIFT                                                  0
#define IPE_UP_PKT_LEN_MASK                                          0xffffffff
#define IPE_UP_PKT_LEN_RD(src)                           (((src) & 0xffffffff))
#define IPE_UP_PKT_LEN_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_UP_PKT_LEN_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES0	*/
/*	 Fields IPE_HASH_RES0	 */
#define IPE_HASH_RES00_WIDTH                                                 32
#define IPE_HASH_RES00_SHIFT                                                  0
#define IPE_HASH_RES00_MASK                                          0xffffffff
#define IPE_HASH_RES00_RD(src)                           (((src) & 0xffffffff))
#define IPE_HASH_RES00_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES00_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES1	*/
/*	 Fields IPE_HASH_RES1	 */
#define IPE_HASH_RES11_WIDTH                                                 32
#define IPE_HASH_RES11_SHIFT                                                  0
#define IPE_HASH_RES11_MASK                                          0xffffffff
#define IPE_HASH_RES11_RD(src)                           (((src) & 0xffffffff))
#define IPE_HASH_RES11_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES11_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES2	*/
/*	 Fields IPE_HASH_RES2	 */
#define IPE_HASH_RES22_WIDTH                                                 32
#define IPE_HASH_RES22_SHIFT                                                  0
#define IPE_HASH_RES22_MASK                                          0xffffffff
#define IPE_HASH_RES22_RD(src)                           (((src) & 0xffffffff))
#define IPE_HASH_RES22_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES22_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES3	*/
/*	 Fields IPE_HASH_RES3	 */
#define IPE_HASH_RES33_WIDTH                                                 32
#define IPE_HASH_RES33_SHIFT                                                  0
#define IPE_HASH_RES33_MASK                                          0xffffffff
#define IPE_HASH_RES33_RD(src)                           (((src) & 0xffffffff))
#define IPE_HASH_RES33_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES33_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES4	*/
/*	 Fields IPE_HASH_RES4	 */
#define IPE_HASH_RES44_WIDTH                                                 32
#define IPE_HASH_RES44_SHIFT                                                  0
#define IPE_HASH_RES44_MASK                                          0xffffffff
#define IPE_HASH_RES44_RD(src)                           (((src) & 0xffffffff))
#define IPE_HASH_RES44_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES44_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES5	*/
/*	 Fields IPE_HASH_RES5	 */
#define IPE_HASH_RES55_WIDTH                                                 32
#define IPE_HASH_RES55_SHIFT                                                  0
#define IPE_HASH_RES55_MASK                                          0xffffffff
#define IPE_HASH_RES55_RD(src)                           (((src) & 0xffffffff))
#define IPE_HASH_RES55_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES55_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES6	*/
/*	 Fields IPE_HASH_RES6	 */
#define IPE_HASH_RES66_WIDTH                                                 32
#define IPE_HASH_RES66_SHIFT                                                  0
#define IPE_HASH_RES66_MASK                                          0xffffffff
#define IPE_HASH_RES66_RD(src)                           (((src) & 0xffffffff))
#define IPE_HASH_RES66_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES66_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES7	*/
/*	 Fields IPE_HASH_RES7	 */
#define IPE_HASH_RES77_WIDTH                                                 32
#define IPE_HASH_RES77_SHIFT                                                  0
#define IPE_HASH_RES77_MASK                                          0xffffffff
#define IPE_HASH_RES77_RD(src)                           (((src) & 0xffffffff))
#define IPE_HASH_RES77_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES77_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_DGST_CNT_RES	*/
/*	 Fields IPE_DGST_CNT_RES	 */
#define IPE_DGST_CNT_RES_WIDTH                                               32
#define IPE_DGST_CNT_RES_SHIFT                                                0
#define IPE_DGST_CNT_RES_MASK                                        0xffffffff
#define IPE_DGST_CNT_RES_RD(src)                         (((src) & 0xffffffff))
#define IPE_DGST_CNT_RES_WR(src)                    (((u32)(src)) & 0xffffffff)
#define IPE_DGST_CNT_RES_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_SPI_RTVD	*/
/*	 Fields IPE_SPI_RTVD	 */
#define IPE_SPI_RTVD_WIDTH                                                   32
#define IPE_SPI_RTVD_SHIFT                                                    0
#define IPE_SPI_RTVD_MASK                                            0xffffffff
#define IPE_SPI_RTVD_RD(src)                             (((src) & 0xffffffff))
#define IPE_SPI_RTVD_WR(src)                        (((u32)(src)) & 0xffffffff)
#define IPE_SPI_RTVD_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_SN_RTVD	*/
/*	 Fields IPE_SN_RTVD	 */
#define IPE_SN_RTVD_WIDTH                                                    32
#define IPE_SN_RTVD_SHIFT                                                     0
#define IPE_SN_RTVD_MASK                                             0xffffffff
#define IPE_SN_RTVD_RD(src)                              (((src) & 0xffffffff))
#define IPE_SN_RTVD_WR(src)                         (((u32)(src)) & 0xffffffff)
#define IPE_SN_RTVD_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_ESN_CALC	*/
/*	 Fields IPE_ESN_CALC	 */
#define IPE_ESN_CALC_WIDTH                                                   32
#define IPE_ESN_CALC_SHIFT                                                    0
#define IPE_ESN_CALC_MASK                                            0xffffffff
#define IPE_ESN_CALC_RD(src)                             (((src) & 0xffffffff))
#define IPE_ESN_CALC_WR(src)                        (((u32)(src)) & 0xffffffff)
#define IPE_ESN_CALC_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_CS_RES	*/
/*	 Fields IPE_CS_RES	 */
#define IPE_CS_RES_WIDTH                                                     32
#define IPE_CS_RES_SHIFT                                                      0
#define IPE_CS_RES_MASK                                              0xffffffff
#define IPE_CS_RES_RD(src)                               (((src) & 0xffffffff))
#define IPE_CS_RES_WR(src)                          (((u32)(src)) & 0xffffffff)
#define IPE_CS_RES_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST8	*/
/*	 Fields IPE_NXT_IN_DGST8	 */
#define IPE_NXT_IN_DGST88_WIDTH                                              32
#define IPE_NXT_IN_DGST88_SHIFT                                               0
#define IPE_NXT_IN_DGST88_MASK                                       0xffffffff
#define IPE_NXT_IN_DGST88_RD(src)                        (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST88_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST88_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST9	*/
/*	 Fields IPE_NXT_IN_DGST9	 */
#define IPE_NXT_IN_DGST99_WIDTH                                              32
#define IPE_NXT_IN_DGST99_SHIFT                                               0
#define IPE_NXT_IN_DGST99_MASK                                       0xffffffff
#define IPE_NXT_IN_DGST99_RD(src)                        (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST99_WR(src)                   (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST99_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST10	*/
/*	 Fields IPE_NXT_IN_DGST10	 */
#define IPE_NXT_IN_DGST100_WIDTH                                             32
#define IPE_NXT_IN_DGST100_SHIFT                                              0
#define IPE_NXT_IN_DGST100_MASK                                      0xffffffff
#define IPE_NXT_IN_DGST100_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST100_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST100_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST11	*/
/*	 Fields IPE_NXT_IN_DGST11	 */
#define IPE_NXT_IN_DGST111_WIDTH                                             32
#define IPE_NXT_IN_DGST111_SHIFT                                              0
#define IPE_NXT_IN_DGST111_MASK                                      0xffffffff
#define IPE_NXT_IN_DGST111_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST111_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST111_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST12	*/
/*	 Fields IPE_NXT_IN_DGST12	 */
#define IPE_NXT_IN_DGST122_WIDTH                                             32
#define IPE_NXT_IN_DGST122_SHIFT                                              0
#define IPE_NXT_IN_DGST122_MASK                                      0xffffffff
#define IPE_NXT_IN_DGST122_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST122_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST122_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST13	*/
/*	 Fields IPE_NXT_IN_DGST13	 */
#define IPE_NXT_IN_DGST133_WIDTH                                             32
#define IPE_NXT_IN_DGST133_SHIFT                                              0
#define IPE_NXT_IN_DGST133_MASK                                      0xffffffff
#define IPE_NXT_IN_DGST133_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST133_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST133_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST14	*/
/*	 Fields IPE_NXT_IN_DGST14	 */
#define IPE_NXT_IN_DGST144_WIDTH                                             32
#define IPE_NXT_IN_DGST144_SHIFT                                              0
#define IPE_NXT_IN_DGST144_MASK                                      0xffffffff
#define IPE_NXT_IN_DGST144_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST144_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST144_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_IN_DGST15	*/
/*	 Fields IPE_NXT_IN_DGST15	 */
#define IPE_NXT_IN_DGST155_WIDTH                                             32
#define IPE_NXT_IN_DGST155_SHIFT                                              0
#define IPE_NXT_IN_DGST155_MASK                                      0xffffffff
#define IPE_NXT_IN_DGST155_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_IN_DGST155_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_IN_DGST155_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST8	*/
/*	 Fields IPE_NXT_OUT_DGST8	 */
#define IPE_NXT_OUT_DGST88_WIDTH                                             32
#define IPE_NXT_OUT_DGST88_SHIFT                                              0
#define IPE_NXT_OUT_DGST88_MASK                                      0xffffffff
#define IPE_NXT_OUT_DGST88_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST88_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST88_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST9	*/
/*	 Fields IPE_NXT_OUT_DGST9	 */
#define IPE_NXT_OUT_DGST99_WIDTH                                             32
#define IPE_NXT_OUT_DGST99_SHIFT                                              0
#define IPE_NXT_OUT_DGST99_MASK                                      0xffffffff
#define IPE_NXT_OUT_DGST99_RD(src)                       (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST99_WR(src)                  (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST99_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST10	*/
/*	 Fields IPE_NXT_OUT_DGST10	 */
#define IPE_NXT_OUT_DGST100_WIDTH                                            32
#define IPE_NXT_OUT_DGST100_SHIFT                                             0
#define IPE_NXT_OUT_DGST100_MASK                                     0xffffffff
#define IPE_NXT_OUT_DGST100_RD(src)                      (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST100_WR(src)                 (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST100_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST11	*/
/*	 Fields IPE_NXT_OUT_DGST11	 */
#define IPE_NXT_OUT_DGST111_WIDTH                                            32
#define IPE_NXT_OUT_DGST111_SHIFT                                             0
#define IPE_NXT_OUT_DGST111_MASK                                     0xffffffff
#define IPE_NXT_OUT_DGST111_RD(src)                      (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST111_WR(src)                 (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST111_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST12	*/
/*	 Fields IPE_NXT_OUT_DGST12	 */
#define IPE_NXT_OUT_DGST122_WIDTH                                            32
#define IPE_NXT_OUT_DGST122_SHIFT                                             0
#define IPE_NXT_OUT_DGST122_MASK                                     0xffffffff
#define IPE_NXT_OUT_DGST122_RD(src)                      (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST122_WR(src)                 (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST122_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST13	*/
/*	 Fields IPE_NXT_OUT_DGST13	 */
#define IPE_NXT_OUT_DGST133_WIDTH                                            32
#define IPE_NXT_OUT_DGST133_SHIFT                                             0
#define IPE_NXT_OUT_DGST133_MASK                                     0xffffffff
#define IPE_NXT_OUT_DGST133_RD(src)                      (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST133_WR(src)                 (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST133_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST14	*/
/*	 Fields IPE_NXT_OUT_DGST14	 */
#define IPE_NXT_OUT_DGST144_WIDTH                                            32
#define IPE_NXT_OUT_DGST144_SHIFT                                             0
#define IPE_NXT_OUT_DGST144_MASK                                     0xffffffff
#define IPE_NXT_OUT_DGST144_RD(src)                      (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST144_WR(src)                 (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST144_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_NXT_OUT_DGST15	*/
/*	 Fields IPE_NXT_OUT_DGST15	 */
#define IPE_NXT_OUT_DGST155_WIDTH                                            32
#define IPE_NXT_OUT_DGST155_SHIFT                                             0
#define IPE_NXT_OUT_DGST155_MASK                                     0xffffffff
#define IPE_NXT_OUT_DGST155_RD(src)                      (((src) & 0xffffffff))
#define IPE_NXT_OUT_DGST155_WR(src)                 (((u32)(src)) & 0xffffffff)
#define IPE_NXT_OUT_DGST155_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST8	*/
/*	 Fields IPE_IN_DGST8	 */
#define IPE_IN_DGST88_WIDTH                                                  32
#define IPE_IN_DGST88_SHIFT                                                   0
#define IPE_IN_DGST88_MASK                                           0xffffffff
#define IPE_IN_DGST88_RD(src)                            (((src) & 0xffffffff))
#define IPE_IN_DGST88_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST88_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST9	*/
/*	 Fields IPE_IN_DGST9	 */
#define IPE_IN_DGST99_WIDTH                                                  32
#define IPE_IN_DGST99_SHIFT                                                   0
#define IPE_IN_DGST99_MASK                                           0xffffffff
#define IPE_IN_DGST99_RD(src)                            (((src) & 0xffffffff))
#define IPE_IN_DGST99_WR(src)                       (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST99_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST10	*/
/*	 Fields IPE_IN_DGST10	 */
#define IPE_IN_DGST100_WIDTH                                                 32
#define IPE_IN_DGST100_SHIFT                                                  0
#define IPE_IN_DGST100_MASK                                          0xffffffff
#define IPE_IN_DGST100_RD(src)                           (((src) & 0xffffffff))
#define IPE_IN_DGST100_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST100_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST11	*/
/*	 Fields IPE_IN_DGST11	 */
#define IPE_IN_DGST111_WIDTH                                                 32
#define IPE_IN_DGST111_SHIFT                                                  0
#define IPE_IN_DGST111_MASK                                          0xffffffff
#define IPE_IN_DGST111_RD(src)                           (((src) & 0xffffffff))
#define IPE_IN_DGST111_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST111_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST12	*/
/*	 Fields IPE_IN_DGST12	 */
#define IPE_IN_DGST122_WIDTH                                                 32
#define IPE_IN_DGST122_SHIFT                                                  0
#define IPE_IN_DGST122_MASK                                          0xffffffff
#define IPE_IN_DGST122_RD(src)                           (((src) & 0xffffffff))
#define IPE_IN_DGST122_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST122_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST13	*/
/*	 Fields IPE_IN_DGST13	 */
#define IPE_IN_DGST133_WIDTH                                                 32
#define IPE_IN_DGST133_SHIFT                                                  0
#define IPE_IN_DGST133_MASK                                          0xffffffff
#define IPE_IN_DGST133_RD(src)                           (((src) & 0xffffffff))
#define IPE_IN_DGST133_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST133_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST14	*/
/*	 Fields IPE_IN_DGST14	 */
#define IPE_IN_DGST144_WIDTH                                                 32
#define IPE_IN_DGST144_SHIFT                                                  0
#define IPE_IN_DGST144_MASK                                          0xffffffff
#define IPE_IN_DGST144_RD(src)                           (((src) & 0xffffffff))
#define IPE_IN_DGST144_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST144_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_IN_DGST15	*/
/*	 Fields IPE_IN_DGST15	 */
#define IPE_IN_DGST155_WIDTH                                                 32
#define IPE_IN_DGST155_SHIFT                                                  0
#define IPE_IN_DGST155_MASK                                          0xffffffff
#define IPE_IN_DGST155_RD(src)                           (((src) & 0xffffffff))
#define IPE_IN_DGST155_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_IN_DGST155_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST8	*/
/*	 Fields IPE_OUT_DGST8	 */
#define IPE_OUT_DGST88_WIDTH                                                 32
#define IPE_OUT_DGST88_SHIFT                                                  0
#define IPE_OUT_DGST88_MASK                                          0xffffffff
#define IPE_OUT_DGST88_RD(src)                           (((src) & 0xffffffff))
#define IPE_OUT_DGST88_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST88_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST9	*/
/*	 Fields IPE_OUT_DGST9	 */
#define IPE_OUT_DGST99_WIDTH                                                 32
#define IPE_OUT_DGST99_SHIFT                                                  0
#define IPE_OUT_DGST99_MASK                                          0xffffffff
#define IPE_OUT_DGST99_RD(src)                           (((src) & 0xffffffff))
#define IPE_OUT_DGST99_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST99_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST10	*/
/*	 Fields IPE_OUT_DGST10	 */
#define IPE_OUT_DGST100_WIDTH                                                32
#define IPE_OUT_DGST100_SHIFT                                                 0
#define IPE_OUT_DGST100_MASK                                         0xffffffff
#define IPE_OUT_DGST100_RD(src)                          (((src) & 0xffffffff))
#define IPE_OUT_DGST100_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST100_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST11	*/
/*	 Fields IPE_OUT_DGST11	 */
#define IPE_OUT_DGST111_WIDTH                                                32
#define IPE_OUT_DGST111_SHIFT                                                 0
#define IPE_OUT_DGST111_MASK                                         0xffffffff
#define IPE_OUT_DGST111_RD(src)                          (((src) & 0xffffffff))
#define IPE_OUT_DGST111_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST111_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST12	*/
/*	 Fields IPE_OUT_DGST12	 */
#define IPE_OUT_DGST122_WIDTH                                                32
#define IPE_OUT_DGST122_SHIFT                                                 0
#define IPE_OUT_DGST122_MASK                                         0xffffffff
#define IPE_OUT_DGST122_RD(src)                          (((src) & 0xffffffff))
#define IPE_OUT_DGST122_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST122_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST13	*/
/*	 Fields IPE_OUT_DGST13	 */
#define IPE_OUT_DGST133_WIDTH                                                32
#define IPE_OUT_DGST133_SHIFT                                                 0
#define IPE_OUT_DGST133_MASK                                         0xffffffff
#define IPE_OUT_DGST133_RD(src)                          (((src) & 0xffffffff))
#define IPE_OUT_DGST133_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST133_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST14	*/
/*	 Fields IPE_OUT_DGST14	 */
#define IPE_OUT_DGST144_WIDTH                                                32
#define IPE_OUT_DGST144_SHIFT                                                 0
#define IPE_OUT_DGST144_MASK                                         0xffffffff
#define IPE_OUT_DGST144_RD(src)                          (((src) & 0xffffffff))
#define IPE_OUT_DGST144_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST144_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_OUT_DGST15	*/
/*	 Fields IPE_OUT_DGST15	 */
#define IPE_OUT_DGST155_WIDTH                                                32
#define IPE_OUT_DGST155_SHIFT                                                 0
#define IPE_OUT_DGST155_MASK                                         0xffffffff
#define IPE_OUT_DGST155_RD(src)                          (((src) & 0xffffffff))
#define IPE_OUT_DGST155_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_OUT_DGST155_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES8	*/
/*	 Fields IPE_HASH_RES8	 */
#define IPE_HASH_RES88_WIDTH                                                 32
#define IPE_HASH_RES88_SHIFT                                                  0
#define IPE_HASH_RES88_MASK                                          0xffffffff
#define IPE_HASH_RES88_RD(src)                           (((src) & 0xffffffff))
#define IPE_HASH_RES88_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES88_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES9	*/
/*	 Fields IPE_HASH_RES9	 */
#define IPE_HASH_RES99_WIDTH                                                 32
#define IPE_HASH_RES99_SHIFT                                                  0
#define IPE_HASH_RES99_MASK                                          0xffffffff
#define IPE_HASH_RES99_RD(src)                           (((src) & 0xffffffff))
#define IPE_HASH_RES99_WR(src)                      (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES99_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES10	*/
/*	 Fields IPE_HASH_RES10	 */
#define IPE_HASH_RES100_WIDTH                                                32
#define IPE_HASH_RES100_SHIFT                                                 0
#define IPE_HASH_RES100_MASK                                         0xffffffff
#define IPE_HASH_RES100_RD(src)                          (((src) & 0xffffffff))
#define IPE_HASH_RES100_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES100_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES11	*/
/*	 Fields IPE_HASH_RES11	 */
#define IPE_HASH_RES111_WIDTH                                                32
#define IPE_HASH_RES111_SHIFT                                                 0
#define IPE_HASH_RES111_MASK                                         0xffffffff
#define IPE_HASH_RES111_RD(src)                          (((src) & 0xffffffff))
#define IPE_HASH_RES111_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES111_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES12	*/
/*	 Fields IPE_HASH_RES12	 */
#define IPE_HASH_RES122_WIDTH                                                32
#define IPE_HASH_RES122_SHIFT                                                 0
#define IPE_HASH_RES122_MASK                                         0xffffffff
#define IPE_HASH_RES122_RD(src)                          (((src) & 0xffffffff))
#define IPE_HASH_RES122_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES122_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES13	*/
/*	 Fields IPE_HASH_RES13	 */
#define IPE_HASH_RES133_WIDTH                                                32
#define IPE_HASH_RES133_SHIFT                                                 0
#define IPE_HASH_RES133_MASK                                         0xffffffff
#define IPE_HASH_RES133_RD(src)                          (((src) & 0xffffffff))
#define IPE_HASH_RES133_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES133_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES14	*/
/*	 Fields IPE_HASH_RES14	 */
#define IPE_HASH_RES144_WIDTH                                                32
#define IPE_HASH_RES144_SHIFT                                                 0
#define IPE_HASH_RES144_MASK                                         0xffffffff
#define IPE_HASH_RES144_RD(src)                          (((src) & 0xffffffff))
#define IPE_HASH_RES144_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES144_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register IPE_HASH_RES15	*/
/*	 Fields IPE_HASH_RES15	 */
#define IPE_HASH_RES155_WIDTH                                                32
#define IPE_HASH_RES155_SHIFT                                                 0
#define IPE_HASH_RES155_MASK                                         0xffffffff
#define IPE_HASH_RES155_RD(src)                          (((src) & 0xffffffff))
#define IPE_HASH_RES155_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_HASH_RES155_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register IPE_SW_INT */
/*       Fields IPE_SW_INT   */
#define IPE_SW_INT_WIDTH                                                32
#define IPE_SW_INT_SHIFT                                                 0
#define IPE_SW_INT_MASK                                         0xffffffff
#define IPE_SW_INT_RD(src)                          (((src) & 0xffffffff))
#define IPE_SW_INT_WR(src)                     (((u32)(src)) & 0xffffffff)
#define IPE_SW_INT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register IPE_SN_THRESHOLD */
/*       Fields IPE_SN_THRESHOLD   */
#define SEQ_NUM_THRESHOLD_WIDTH                                                32
#define SEQ_NUM_THRESHOLD_SHIFT                                                 0
#define SEQ_NUM_THRESHOLD_MASK                                         0xffffffff
#define SEQ_NUM_THRESHOLD_RD(src)                          (((src) & 0xffffffff))
#define SEQ_NUM_THRESHOLD_WR(src)                     (((u32)(src)) & 0xffffffff)
#define SEQ_NUM_THRESHOLD_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register IPE_BLK_CTX_UPDT */
/*       Fields IPE_BLK_CTX_UPDT   */
#define BLK_CTX_UPDT_WIDTH                                                2
#define BLK_CTX_UPDT_SHIFT                                                 0
#define BLK_CTX_UPDT_MASK                                         0x00000003
#define BLK_CTX_UPDT_RD(src)                          (((src) & 0x00000003))
#define BLK_CTX_UPDT_WR(src)                     (((u32)(src)) & 0x00000003)
#define BLK_CTX_UPDT_SET(dst,src) \
                          (((dst) & ~0x00000003) | (((u32)(src)) & 0x00000003))

/*	Register IPE_DEV_INFO_TYPE	*/
/*	 Fields GHASH_speed	 */
#define GHASH_SPEED_WIDTH                                                        1
#define GHASH_SPEED_SHIFT                                                         31
#define GHASH_SPEED_MASK                                                 0x80000000
#define GHASH_SPEED_RD(src)                               (((src) & 0x80000000)>>31)
#define GHASH_SPEED_WR(src)                          (((u32)(src)<<31) & 0x80000000)
#define GHASH_SPEED_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*	 Fields GHASH	 */
#define GHASH_WIDTH                                                    1
#define GHASH_SHIFT                                                    30
#define GHASH_MASK                                            0x40000000
#define GHASH_RD(src)                          (((src) & 0x40000000)>>30)
#define GHASH_WR(src)                     (((u32)(src)<<30) & 0x40000000)
#define GHASH_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*	 Fields IPsec	 */
#define IPSEC_WIDTH                                                         1
#define IPSEC_SHIFT                                                         25
#define IPSEC_MASK                                                 0x02000000
#define IPSEC_RD(src)                                  (((src) & 0x02000000)>>25)
#define IPSEC_WR(src)                             (((u32)(src)<<25) & 0x02000000)
#define IPSEC_SET(dst,src) \
                          (((dst) & ~0x02000000) | (((u32)(src))<<25 & 0x02000000))
/*	 Fields Para_pipe_depth	 */
#define PARA_PIPE_DEPTH_WIDTH                                                         2
#define PARA_PIPE_DEPTH_SHIFT                                                         20
#define PARA_PIPE_DEPTH_MASK                                                 0x01f00000
#define PARA_PIPE_DEPTH_RD(src)                                  (((src) & 0x01f00000)>>20)
#define PARA_PIPE_DEPTH_WR(src)                             (((u32)(src)<<20) & 0x01f00000)
#define PARA_PIPE_DEPTH_SET(dst,src) \
                          (((dst) & ~0x01f00000) | (((u32)(src))<<20 & 0x01f00000))
/*	 Fields AES_186_AES_192	 */
#define AES_186_AES_192_WIDTH                                                         2
#define AES_186_AES_192_SHIFT                                                         18
#define AES_186_AES_192_MASK                                                 0x000c0000
#define AES_186_AES_192_RD(src)                                  (((src) & 0x000c0000)>>18)
#define AES_186_AES_192_WR(src)                             (((u32)(src)<<18) & 0x000c0000)
#define AES_186_AES_192_SET(dst,src) \
                          (((dst) & ~0x000c0000) | (((u32)(src))<<18 & 0x000c0000))
/*       Fields AES_SPEED    */
#define AES_SPEED_F1_WIDTH                                                         4
#define AES_SPEED_F1_SHIFT                                                         14
#define AES_SPEED_F1_MASK                                                 0x0003c000
#define AES_SPEED_F1_RD(src)                                  (((src) & 0x0003c000)>>14)
#define AES_SPEED_F1_WR(src)                             (((u32)(src)<<14) & 0x0003c000)
#define AES_SPEED_F1_SET(dst,src) \
                          (((dst) & ~0x0003c000) | (((u32)(src))<<14 & 0x0003c000))
/*       Fields AES_FB    */
#define AES_FB_F1_WIDTH                                                         1
#define AES_FB_F1_SHIFT                                                         13
#define AES_FB_F1_MASK                                                 0x00002000
#define AES_FB_F1_RD(src)                                  (((src) & 0x00002000)>>13)
#define AES_FB_F1_WR(src)                             (((u32)(src)<<13) & 0x00002000)
#define AES_FB_F1_SET(dst,src) \
                          (((dst) & ~0x00002000) | (((u32)(src))<<13 & 0x00002000))
/*       Fields AES    */
#define AES_F1_WIDTH                                                         1
#define AES_F1_SHIFT                                                         12
#define AES_F1_MASK                                                 0x00001000
#define AES_F1_RD(src)                                  (((src) & 0x00001000)>>12)
#define AES_F1_WR(src)                             (((u32)(src)<<12) & 0x00001000)
#define AES_F1_SET(dst,src) \
                          (((dst) & ~0x00001000) | (((u32)(src))<<12 & 0x00001000))
/*       Fields AES_ONLY_CORE    */
#define AES_ONLY_CORE_WIDTH                                                         1
#define AES_ONLY_CORE_SHIFT                                                         11
#define AES_ONLY_CORE_MASK                                                 0x00000800
#define AES_ONLY_CORE_RD(src)                                  (((src) & 0x00000800)>>11)
#define AES_ONLY_CORE_WR(src)                             (((u32)(src)<<11) & 0x00000800)
#define AES_ONLY_CORE_SET(dst,src) \
                          (((dst) & ~0x00000800) | (((u32)(src))<<11 & 0x00000800))
/*       Fields ASIC_AES_LOOK_UP    */
#define ASIC_AES_LOOK_UP_WIDTH                                                         1
#define ASIC_AES_LOOK_UP_SHIFT                                                         10
#define ASIC_AES_LOOK_UP_MASK                                                 0x00000400
#define ASIC_AES_LOOK_UP_RD(src)                                  (((src) & 0x00000400)>>10)
#define ASIC_AES_LOOK_UP_WR(src)                             (((u32)(src)<<10) & 0x00000400)
#define ASIC_AES_LOOK_UP_SET(dst,src) \
                          (((dst) & ~0x00000400) | (((u32)(src))<<10 & 0x00000400))
/*       Fields ASIC_AES_GF    */
#define ASIC_AES_GF_WIDTH                                                         1
#define ASIC_AES_GF_SHIFT                                                         9
#define ASIC_AES_GF_MASK                                                 0x00000200
#define ASIC_AES_GF_RD(src)                                  (((src) & 0x00000200)>>9)
#define ASIC_AES_GF_WR(src)                             (((u32)(src)<<9) & 0x00000200)
#define ASIC_AES_GF_SET(dst,src) \
                          (((dst) & ~0x00000200) | (((u32)(src))<<9 & 0x00000200))
/*       Fields FPGA_SOLUTION    */
#define FPGA_SOLUTION_WIDTH                                                         1
#define FPGA_SOLUTION_SHIFT                                                         8
#define FPGA_SOLUTION_MASK                                                 0x00000100
#define FPGA_SOLUTION_RD(src)                                  (((src) & 0x00000100)>>8)
#define FPGA_SOLUTION_WR(src)                             (((u32)(src)<<8) & 0x00000100)
#define FPGA_SOLUTION_SET(dst,src) \
                          (((dst) & ~0x00000100) | (((u32)(src))<<8 & 0x00000100))
/*       Fields MAJOR_REV_NUM    */
#define MAJOR_REV_NUM_WIDTH                                                         4
#define MAJOR_REV_NUM_SHIFT                                                         4
#define MAJOR_REV_NUM_MASK                                                 0x000000f0
#define MAJOR_REV_NUM_RD(src)                                  (((src) & 0x000000f0)>>4)
#define MAJOR_REV_NUM_WR(src)                             (((u32)(src)<<4) & 0x000000f0)
#define MAJOR_REV_NUM_SET(dst,src) \
                          (((dst) & ~0x000000f0) | (((u32)(src))<<4 & 0x000000f0))
/*	 Fields MINOR_REV_NUM	 */
#define MINOR_REV_NUM_WIDTH                                                         4
#define MINOR_REV_NUM_SHIFT                                                         0
#define MINOR_REV_NUM_MASK                                                 0x0000000f
#define MINOR_REV_NUM_RD(src)                                  (((src) & 0x0000000f))
#define MINOR_REV_NUM_WR(src)                             (((u32)(src)) & 0x0000000f)
#define MINOR_REV_NUM_SET(dst,src) \
                          (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*	Register IPE_DEV_INFO_VERSION	*/
/*	 Fields Major_rev_num	 */
#define MAJOR_REV_NUM_F1_WIDTH                                                        4
#define MAJOR_REV_NUM_F1_SHIFT                                                         24
#define MAJOR_REV_NUM_F1_MASK                                                 0x0f000000
#define MAJOR_REV_NUM_F1_RD(src)                               (((src) & 0x0f000000)>>24)
#define MAJOR_REV_NUM_F1_WR(src)                          (((u242)(src)<<24) & 0x0f000000)
#define MAJOR_REV_NUM_F1_SET(dst,src) \
                       (((dst) & ~0x0f000000) | (((u242)(src)<<24) & 0x0f000000))
/*	 Fields Minor_rev_num	 */
#define MINOR_REV_NUM_F1_WIDTH                                                      4
#define MINOR_REV_NUM_F1_SHIFT                                                      20
#define MINOR_REV_NUM_F1_MASK                                              0x00f00000
#define MINOR_REV_NUM_F1_RD(src)                            (((src) & 0x00f00000)>>20)
#define MINOR_REV_NUM_F1_WR(src)                       (((u320)(src)<<20) & 0x00f00000)
#define MINOR_REV_NUM_F1_SET(dst,src) \
                       (((dst) & ~0x00f00000) | (((u320)(src)<<20) & 0x00f00000))
/*	 Fields Patch_level	 */
#define PATCH_LEVEL_WIDTH                                                            4
#define PATCH_LEVEL_SHIFT                                                            16
#define PATCH_LEVEL_MASK                                                    0x000f0000
#define PATCH_LEVEL_RD(src)                                  (((src) & 0x000f0000)>>16)
#define PATCH_LEVEL_WR(src)                             (((u32)(src)<<16) & 0x000f0000)
#define PATCH_LEVEL_SET(dst,src) \
                       (((dst) & ~0x000f0000) | (((u32)(src)<<16) & 0x000f0000))
/*	 Fields Com_EIP_number 	 */
#define COM_EIP_NUMBER_WIDTH                                                            8
#define COM_EIP_NUMBER_SHIFT                                                            8
#define COM_EIP_NUMBER_MASK                                                    0x0000ff00
#define COM_EIP_NUMBER_RD(src)                                  (((src) & 0x0000ff00)>>8)
#define COM_EIP_NUMBER_WR(src)                             (((u32)(src)<<8) & 0x0000ff00)
#define COM_EIP_NUMBER_SET(dst,src) \
                       (((dst) & ~0x0000ff00) | (((u32)(src)<<8) & 0x0000ff00))
/*	 Fields EIP_number 	 */
#define EIP_NUMBER_WIDTH                                                   8
#define EIP_NUMBER_SHIFT                                                   0
#define EIP_NUMBER_MASK                                           0x000000ff
#define EIP_NUMBER_RD(src)                            (((src) & 0x000000ff))
#define EIP_NUMBER_WR(src)                       (((u32)(src)) & 0x000000ff)
#define EIP_NUMBER_SET(dst,src) \
                          (((dst) & ~0x000000ff) | (((u32)(src)) & 0x000000ff))

/*	Global Base Address	*/
#define SEC_GLBL_DIAG_CSR_BASE_ADDR			0x1f25d000

/*    Address GLBL_DIAG_CSR  Registers */
#define SEC_CFG_DIAG_SEL_ADDR                                        0x00000000
#define SEC_CFG_DIAG_SEL_DEFAULT                                     0x00000000
#define SEC_CFG_READ_BW_LAT_ADDR_MASK_ADDR                           0x00000004
#define SEC_CFG_READ_BW_LAT_ADDR_MASK_DEFAULT                        0x00000000
#define SEC_CFG_READ_BW_LAT_ADDR_PAT_ADDR                            0x00000008
#define SEC_CFG_READ_BW_LAT_ADDR_PAT_DEFAULT                         0xffffffff
#define SEC_CFG_WRITE_BW_LAT_ADDR_MASK_ADDR                          0x0000000c
#define SEC_CFG_WRITE_BW_LAT_ADDR_MASK_DEFAULT                       0x00000000
#define SEC_CFG_WRITE_BW_LAT_ADDR_PAT_ADDR                           0x00000010
#define SEC_CFG_WRITE_BW_LAT_ADDR_PAT_DEFAULT                        0xffffffff
#define SEC_CFG_DIAG_START_STOP_ADDR                                 0x00000014
#define SEC_CFG_DIAG_START_STOP_DEFAULT                              0x000003ff
#define SEC_CFG_BW_MSTR_STOP_CNT_ADDR                                0x00000018
#define SEC_CFG_BW_MSTR_STOP_CNT_DEFAULT                             0x00040004
#define SEC_CFG_BW_SLV_STOP_CNT_ADDR                                 0x0000001c
#define SEC_CFG_BW_SLV_STOP_CNT_DEFAULT                              0x00040004
#define SEC_STS_READ_LATENCY_OUTPUT_ADDR                             0x00000020
#define SEC_STS_READ_LATENCY_OUTPUT_DEFAULT                          0x00000000
#define SEC_STS_AXI_MRD_BW_CLK_CNT_ADDR                              0x00000024
#define SEC_STS_AXI_MRD_BW_CLK_CNT_DEFAULT                           0x00000000
#define SEC_STS_AXI_MRD_BW_BYTE_CNT_ADDR                             0x00000028
#define SEC_STS_AXI_MRD_BW_BYTE_CNT_DEFAULT                          0x00000000
#define SEC_STS_AXI_MWR_BW_CLK_CNT_ADDR                              0x0000002c
#define SEC_STS_AXI_MWR_BW_CLK_CNT_DEFAULT                           0x00000000
#define SEC_STS_AXI_MWR_BW_BYTE_CNT_ADDR                             0x00000030
#define SEC_STS_AXI_MWR_BW_BYTE_CNT_DEFAULT                          0x00000000
#define SEC_STS_AXI_SRD_BW_CLK_CNT_ADDR                              0x00000034
#define SEC_STS_AXI_SRD_BW_CLK_CNT_DEFAULT                           0x00000000
#define SEC_STS_AXI_SRD_BW_BYTE_CNT_ADDR                             0x00000038
#define SEC_STS_AXI_SRD_BW_BYTE_CNT_DEFAULT                          0x00000000
#define SEC_STS_AXI_SWR_BW_CLK_CNT_ADDR                              0x0000003c
#define SEC_STS_AXI_SWR_BW_CLK_CNT_DEFAULT                           0x00000000
#define SEC_STS_AXI_SWR_BW_BYTE_CNT_ADDR                             0x00000040
#define SEC_STS_AXI_SWR_BW_BYTE_CNT_DEFAULT                          0x00000000
#define SEC_CFG_DBG_TRIG_CTRL_ADDR                                   0x00000044
#define SEC_CFG_DBG_TRIG_CTRL_DEFAULT                                0x00000000
#define SEC_CFG_DBG_PAT_REG_0_ADDR                                   0x00000048
#define SEC_CFG_DBG_PAT_REG_0_DEFAULT                                0x00000000
#define SEC_CFG_DBG_PAT_MASK_REG_0_ADDR                              0x0000004c
#define SEC_CFG_DBG_PAT_MASK_REG_0_DEFAULT                           0x00000000
#define SEC_CFG_DBG_PAT_REG_1_ADDR                                   0x00000050
#define SEC_CFG_DBG_PAT_REG_1_DEFAULT                                0x00000000
#define SEC_CFG_DBG_PAT_MASK_REG_1_ADDR                              0x00000054
#define SEC_CFG_DBG_PAT_MASK_REG_1_DEFAULT                           0x00000000
#define SEC_DBG_TRIG_OUT_ADDR                                        0x00000058
#define SEC_DBG_TRIG_OUT_DEFAULT                                     0x00000000
#define SEC_DBG_TRIG_INT_ADDR                                     0x0000005c
#define SEC_DBG_TRIG_INT_DEFAULT                                  0x00000000
#define SEC_DBG_TRIG_INTMASK_ADDR                                 0x00000060
#define SEC_DBG_TRIG_INTMASK_DEFAULT					0xffffffff
#define SEC_INTR_STS_ADDR                              0x00000064
#define SEC_INTR_STS_DEFAULT                           0x00000000
#define SEC_CFG_MEM_ECC_BYPASS_ADDR                                  0x00000068
#define SEC_CFG_MEM_ECC_BYPASS_DEFAULT                               0x00000000
#define SEC_CFG_MEM_PWRDN_DIS_ADDR                                   0x0000006c
#define SEC_CFG_MEM_PWRDN_DIS_DEFAULT                                0x00000000
#define SEC_CFG_MEM_RAM_SHUTDOWN_ADDR                                0x00000070
#define SEC_CFG_MEM_RAM_SHUTDOWN_DEFAULT                             0xffffffff
#define SEC_BLOCK_MEM_RDY_ADDR                                       0x00000074
#define SEC_BLOCK_MEM_RDY_DEFAULT                                    0xffffffff
#define SEC_CFG_AXI_SLV_RD_TMO_ADDR                                  0x00000078
#define SEC_CFG_AXI_SLV_RD_TMO_DEFAULT                               0x83ff83ff
#define SEC_CFG_AXI_SLV_WR_TMO_ADDR                                  0x0000007c
#define SEC_CFG_AXI_SLV_WR_TMO_DEFAULT                               0x83ff83ff
#define SEC_STS_AXI_SLV_TMO_ADDR                                     0x00000080
#define SEC_STS_AXI_SLV_TMO_DEFAULT                                  0x00000000
#define SEC_CFG_AXI_SS_CSR_TMO_ADDR                                  0x00000084
#define SEC_CFG_AXI_SS_CSR_TMO_DEFAULT                               0x02008000
#define SEC_STS_READ_LATENCY_TOT_READ_REQS_ADDR                      0x0000008c
#define SEC_STS_READ_LATENCY_TOT_READ_REQS_DEFAULT                   0x00000000
#define SEC_CFG_LT_MSTR_STOP_CNT_ADDR				     0x00000090
#define SEC_CFG_LT_MSTR_STOP_CNT_DEFAULT			     0x00040000
#define SEC_CFG_BW_SRD_TRIG_CAP_ADDR                              0x000000a0
#define SEC_CFG_BW_SRD_TRIG_CAP_DEFAULT                           0x00000000
#define SEC_CFG_BW_SWR_TRIG_CAP_ADDR                              0x000000a4
#define SEC_CFG_BW_SWR_TRIG_CAP_DEFAULT                           0x00000000
#define SEC_CFG_BW_MRD_TRIG_CAP_ADDR                              0x000000a8
#define SEC_CFG_BW_MRD_TRIG_CAP_DEFAULT                           0x00000000
#define SEC_CFG_BW_MWR_TRIG_CAP_ADDR                              0x000000ac
#define SEC_CFG_BW_MWR_TRIG_CAP_DEFAULT                           0x00000000
#define SEC_CFG_LT_MRD_TRIG_CAP_ADDR                              0x000000b0
#define SEC_CFG_LT_MRD_TRIG_CAP_DEFAULT                           0x00000000
#define SEC_DBG_BLOCK_AXI_ADDR                              0x000000b4
#define SEC_DBG_BLOCK_AXI_DEFAULT                           0x00000000
#define SEC_DBG_BLOCK_NON_AXI_ADDR                              0x000000b8
#define SEC_DBG_BLOCK_NON_AXI_DEFAULT                           0x00000000
#define SEC_DBG_AXI_SHIM_OUT_ADDR                              0x000000bc
#define SEC_DBG_AXI_SHIM_OUT_DEFAULT                           0x00000000

/*	Register CFG_DIAG_SEL	*/
/*	 Fields CFG_SHIM_BLK_DBUS_MUX_SELECT	 */
#define SEC_CFG_SHIM_BLK_DBUS_MUX_SELECT_WIDTH                                  1
#define SEC_CFG_SHIM_BLK_DBUS_MUX_SELECT_SHIFT                                 12
#define SEC_CFG_SHIM_BLK_DBUS_MUX_SELECT_MASK                          0x00001000
#define SEC_CFG_SHIM_BLK_DBUS_MUX_SELECT_RD(src)       (((src) & 0x00001000)>>12)
#define SEC_CFG_SHIM_BLK_DBUS_MUX_SELECT_WR(src)  (((u32)(src)<<12) & 0x00001000)
#define SEC_CFG_SHIM_BLK_DBUS_MUX_SELECT_SET(dst,src) \
                      (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*	 Fields CFG_AXI_NON_AXI_MUX_SELECT	 */
#define SEC_CFG_AXI_NON_AXI_MUX_SELECT_WIDTH                                  1
#define SEC_CFG_AXI_NON_AXI_MUX_SELECT_SHIFT                                 11
#define SEC_CFG_AXI_NON_AXI_MUX_SELECT_MASK                          0x00000800
#define SEC_CFG_AXI_NON_AXI_MUX_SELECT_RD(src)       (((src) & 0x00000800)>>11)
#define SEC_CFG_AXI_NON_AXI_MUX_SELECT_WR(src)  (((u32)(src)<<11) & 0x00000800)
#define SEC_CFG_AXI_NON_AXI_MUX_SELECT_SET(dst,src) \
                      (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*	 Fields CFG_MUX_SELECTOR	 */
#define SEC_CFG_MUX_SELECTOR_WIDTH                                           11
#define SEC_CFG_MUX_SELECTOR_SHIFT                                            0
#define SEC_CFG_MUX_SELECTOR_MASK                                    0x000007ff
#define SEC_CFG_MUX_SELECTOR_RD(src)                     (((src) & 0x000007ff))
#define SEC_CFG_MUX_SELECTOR_WR(src)                (((u32)(src)) & 0x000007ff)
#define SEC_CFG_MUX_SELECTOR_SET(dst,src) \
                          (((dst) & ~0x000007ff) | (((u32)(src)) & 0x000007ff))

/*	Register CFG_READ_BW_LAT_ADDR_MASK	*/
/*	 Fields READ_ADDR_MASK	 */
#define SEC_READ_ADDR_MASK_WIDTH                                             32
#define SEC_READ_ADDR_MASK_SHIFT                                              0
#define SEC_READ_ADDR_MASK_MASK                                      0xffffffff
#define SEC_READ_ADDR_MASK_RD(src)                       (((src) & 0xffffffff))
#define SEC_READ_ADDR_MASK_WR(src)                  (((u32)(src)) & 0xffffffff)
#define SEC_READ_ADDR_MASK_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CFG_READ_BW_LAT_ADDR_PAT	*/
/*	 Fields READ_ADDR_PAT	 */
#define SEC_READ_ADDR_PAT_WIDTH                                              32
#define SEC_READ_ADDR_PAT_SHIFT                                               0
#define SEC_READ_ADDR_PAT_MASK                                       0xffffffff
#define SEC_READ_ADDR_PAT_RD(src)                        (((src) & 0xffffffff))
#define SEC_READ_ADDR_PAT_WR(src)                   (((u32)(src)) & 0xffffffff)
#define SEC_READ_ADDR_PAT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CFG_WRITE_BW_LAT_ADDR_MASK	*/
/*	 Fields WRITE_ADDR_MASK	 */
#define SEC_WRITE_ADDR_MASK_WIDTH                                            32
#define SEC_WRITE_ADDR_MASK_SHIFT                                             0
#define SEC_WRITE_ADDR_MASK_MASK                                     0xffffffff
#define SEC_WRITE_ADDR_MASK_RD(src)                      (((src) & 0xffffffff))
#define SEC_WRITE_ADDR_MASK_WR(src)                 (((u32)(src)) & 0xffffffff)
#define SEC_WRITE_ADDR_MASK_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CFG_WRITE_BW_LAT_ADDR_PAT	*/
/*	 Fields WRITE_ADDR_PAT	 */
#define SEC_WRITE_ADDR_PAT_WIDTH                                             32
#define SEC_WRITE_ADDR_PAT_SHIFT                                              0
#define SEC_WRITE_ADDR_PAT_MASK                                      0xffffffff
#define SEC_WRITE_ADDR_PAT_RD(src)                       (((src) & 0xffffffff))
#define SEC_WRITE_ADDR_PAT_WR(src)                  (((u32)(src)) & 0xffffffff)
#define SEC_WRITE_ADDR_PAT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CFG_DIAG_START_STOP	*/
/*	 Fields START_RD_LATENCY	 */
#define SEC_START_RD_LATENCY_WIDTH                                            1
#define SEC_START_RD_LATENCY_SHIFT                                            4
#define SEC_START_RD_LATENCY_MASK                                    0x00000010
#define SEC_START_RD_LATENCY_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_START_RD_LATENCY_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_START_RD_LATENCY_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields START_SRD_BW	 */
#define SEC_START_SRD_BW_WIDTH                                                1
#define SEC_START_SRD_BW_SHIFT                                                3
#define SEC_START_SRD_BW_MASK                                        0x00000008
#define SEC_START_SRD_BW_RD(src)                      (((src) & 0x00000008)>>3)
#define SEC_START_SRD_BW_WR(src)                 (((u32)(src)<<3) & 0x00000008)
#define SEC_START_SRD_BW_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields START_MRD_BW	 */
#define SEC_START_MRD_BW_WIDTH                                                1
#define SEC_START_MRD_BW_SHIFT                                                2
#define SEC_START_MRD_BW_MASK                                        0x00000004
#define SEC_START_MRD_BW_RD(src)                      (((src) & 0x00000004)>>2)
#define SEC_START_MRD_BW_WR(src)                 (((u32)(src)<<2) & 0x00000004)
#define SEC_START_MRD_BW_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*	 Fields START_SWR_BW	 */
#define SEC_START_SWR_BW_WIDTH                                                1
#define SEC_START_SWR_BW_SHIFT                                                1
#define SEC_START_SWR_BW_MASK                                        0x00000002
#define SEC_START_SWR_BW_RD(src)                      (((src) & 0x00000002)>>1)
#define SEC_START_SWR_BW_WR(src)                 (((u32)(src)<<1) & 0x00000002)
#define SEC_START_SWR_BW_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields START_MWR_BW	 */
#define SEC_START_MWR_BW_WIDTH                                                1
#define SEC_START_MWR_BW_SHIFT                                                0
#define SEC_START_MWR_BW_MASK                                        0x00000001
#define SEC_START_MWR_BW_RD(src)                         (((src) & 0x00000001))
#define SEC_START_MWR_BW_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_START_MWR_BW_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register CFG_BW_MSTR_STOP_CNT	*/
/*	 Fields MSTR_STOP_RD_CNT	 */
#define SEC_MSTR_STOP_RD_CNT_WIDTH                                           16
#define SEC_MSTR_STOP_RD_CNT_SHIFT                                           16
#define SEC_MSTR_STOP_RD_CNT_MASK                                    0xffff0000
#define SEC_MSTR_STOP_RD_CNT_RD(src)                 (((src) & 0xffff0000)>>16)
#define SEC_MSTR_STOP_RD_CNT_WR(src)            (((u32)(src)<<16) & 0xffff0000)
#define SEC_MSTR_STOP_RD_CNT_SET(dst,src) \
                      (((dst) & ~0xffff0000) | (((u32)(src)<<16) & 0xffff0000))
/*	 Fields MSTR_STOP_WR_CNT	 */
#define SEC_MSTR_STOP_WR_CNT_WIDTH                                           16
#define SEC_MSTR_STOP_WR_CNT_SHIFT                                            0
#define SEC_MSTR_STOP_WR_CNT_MASK                                    0x0000ffff
#define SEC_MSTR_STOP_WR_CNT_RD(src)                     (((src) & 0x0000ffff))
#define SEC_MSTR_STOP_WR_CNT_WR(src)                (((u32)(src)) & 0x0000ffff)
#define SEC_MSTR_STOP_WR_CNT_SET(dst,src) \
                          (((dst) & ~0x0000ffff) | (((u32)(src)) & 0x0000ffff))

/*	Register CFG_BW_SLV_STOP_CNT	*/
/*	 Fields SLV_STOP_RD_CNT	 */
#define SEC_SLV_STOP_RD_CNT_WIDTH                                            16
#define SEC_SLV_STOP_RD_CNT_SHIFT                                            16
#define SEC_SLV_STOP_RD_CNT_MASK                                     0xffff0000
#define SEC_SLV_STOP_RD_CNT_RD(src)                  (((src) & 0xffff0000)>>16)
#define SEC_SLV_STOP_RD_CNT_WR(src)             (((u32)(src)<<16) & 0xffff0000)
#define SEC_SLV_STOP_RD_CNT_SET(dst,src) \
                      (((dst) & ~0xffff0000) | (((u32)(src)<<16) & 0xffff0000))
/*	 Fields SLV_STOP_WR_CNT	 */
#define SEC_SLV_STOP_WR_CNT_WIDTH                                            16
#define SEC_SLV_STOP_WR_CNT_SHIFT                                             0
#define SEC_SLV_STOP_WR_CNT_MASK                                     0x0000ffff
#define SEC_SLV_STOP_WR_CNT_RD(src)                      (((src) & 0x0000ffff))
#define SEC_SLV_STOP_WR_CNT_WR(src)                 (((u32)(src)) & 0x0000ffff)
#define SEC_SLV_STOP_WR_CNT_SET(dst,src) \
                          (((dst) & ~0x0000ffff) | (((u32)(src)) & 0x0000ffff))

/*	Register STS_READ_LATENCY_OUTPUT	*/
/*	 Fields READ_MAX	 */
#define SEC_READ_MAX_WIDTH                                                   10
#define SEC_READ_MAX_SHIFT                                                   22
#define SEC_READ_MAX_MASK                                            0xffc00000
#define SEC_READ_MAX_RD(src)                         (((src) & 0xffc00000)>>22)
#define SEC_READ_MAX_SET(dst,src) \
                      (((dst) & ~0xffc00000) | (((u32)(src)<<22) & 0xffc00000))
/*	 Fields READ_TOT	 */
#define SEC_READ_TOT_WIDTH                                                   22
#define SEC_READ_TOT_SHIFT                                                    0
#define SEC_READ_TOT_MASK                                            0x003fffff
#define SEC_READ_TOT_RD(src)                             (((src) & 0x003fffff))
#define SEC_READ_TOT_SET(dst,src) \
                          (((dst) & ~0x003fffff) | (((u32)(src)) & 0x003fffff))

/*	Register STS_AXI_MRD_BW_CLK_CNT	*/
/*	 Fields MSTR_READ_BW_CLK_CNT	 */
#define SEC_MSTR_READ_BW_CLK_CNT_WIDTH                                       32
#define SEC_MSTR_READ_BW_CLK_CNT_SHIFT                                        0
#define SEC_MSTR_READ_BW_CLK_CNT_MASK                                0xffffffff
#define SEC_MSTR_READ_BW_CLK_CNT_RD(src)                 (((src) & 0xffffffff))
#define SEC_MSTR_READ_BW_CLK_CNT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register STS_AXI_MRD_BW_BYTE_CNT	*/
/*	 Fields MSTR_READ_BW_BYTE_CNT	 */
#define SEC_MSTR_READ_BW_BYTE_CNT_WIDTH                                      32
#define SEC_MSTR_READ_BW_BYTE_CNT_SHIFT                                       0
#define SEC_MSTR_READ_BW_BYTE_CNT_MASK                               0xffffffff
#define SEC_MSTR_READ_BW_BYTE_CNT_RD(src)                (((src) & 0xffffffff))
#define SEC_MSTR_READ_BW_BYTE_CNT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register STS_AXI_MWR_BW_CLK_CNT	*/
/*	 Fields MSTR_WRITE_BW_CLK_CNT	 */
#define SEC_MSTR_WRITE_BW_CLK_CNT_WIDTH                                      32
#define SEC_MSTR_WRITE_BW_CLK_CNT_SHIFT                                       0
#define SEC_MSTR_WRITE_BW_CLK_CNT_MASK                               0xffffffff
#define SEC_MSTR_WRITE_BW_CLK_CNT_RD(src)                (((src) & 0xffffffff))
#define SEC_MSTR_WRITE_BW_CLK_CNT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register STS_AXI_MWR_BW_BYTE_CNT	*/
/*	 Fields MSTR_WRITE_BW_BYTE_CNT	 */
#define SEC_MSTR_WRITE_BW_BYTE_CNT_WIDTH                                     32
#define SEC_MSTR_WRITE_BW_BYTE_CNT_SHIFT                                      0
#define SEC_MSTR_WRITE_BW_BYTE_CNT_MASK                              0xffffffff
#define SEC_MSTR_WRITE_BW_BYTE_CNT_RD(src)               (((src) & 0xffffffff))
#define SEC_MSTR_WRITE_BW_BYTE_CNT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register STS_AXI_SRD_BW_CLK_CNT	*/
/*	 Fields SLV_READ_BW_CLK_CNT	 */
#define SEC_SLV_READ_BW_CLK_CNT_WIDTH                                        32
#define SEC_SLV_READ_BW_CLK_CNT_SHIFT                                         0
#define SEC_SLV_READ_BW_CLK_CNT_MASK                                 0xffffffff
#define SEC_SLV_READ_BW_CLK_CNT_RD(src)                  (((src) & 0xffffffff))
#define SEC_SLV_READ_BW_CLK_CNT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register STS_AXI_SRD_BW_BYTE_CNT	*/
/*	 Fields SLV_READ_BW_BYTE_CNT	 */
#define SEC_SLV_READ_BW_BYTE_CNT_WIDTH                                       32
#define SEC_SLV_READ_BW_BYTE_CNT_SHIFT                                        0
#define SEC_SLV_READ_BW_BYTE_CNT_MASK                                0xffffffff
#define SEC_SLV_READ_BW_BYTE_CNT_RD(src)                 (((src) & 0xffffffff))
#define SEC_SLV_READ_BW_BYTE_CNT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register STS_AXI_SWR_BW_CLK_CNT	*/
/*	 Fields SLV_WRITE_BW_CLK_CNT	 */
#define SEC_SLV_WRITE_BW_CLK_CNT_WIDTH                                       32
#define SEC_SLV_WRITE_BW_CLK_CNT_SHIFT                                        0
#define SEC_SLV_WRITE_BW_CLK_CNT_MASK                                0xffffffff
#define SEC_SLV_WRITE_BW_CLK_CNT_RD(src)                 (((src) & 0xffffffff))
#define SEC_SLV_WRITE_BW_CLK_CNT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register STS_AXI_SWR_BW_BYTE_CNT	*/
/*	 Fields SLV_WRITE_BW_BYTE_CNT	 */
#define SEC_SLV_WRITE_BW_BYTE_CNT_WIDTH                                      32
#define SEC_SLV_WRITE_BW_BYTE_CNT_SHIFT                                       0
#define SEC_SLV_WRITE_BW_BYTE_CNT_MASK                               0xffffffff
#define SEC_SLV_WRITE_BW_BYTE_CNT_RD(src)                (((src) & 0xffffffff))
#define SEC_SLV_WRITE_BW_BYTE_CNT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CFG_DBG_TRIG_CTRL	*/
/*	 Fields TRIG_EN_OUT_CTRL	 */
#define SEC_TRIG_EN_OUT_CTRL_WIDTH                                                     1
#define SEC_TRIG_EN_OUT_CTRL_SHIFT                                                     5
#define SEC_TRIG_EN_OUT_CTRL_MASK                                             0x00000020
#define SEC_TRIG_EN_OUT_CTRL_RD(src)                           (((src) & 0x00000020)>>5)
#define SEC_TRIG_EN_OUT_CTRL_WR(src)                      (((u32)(src)<<5) & 0x00000020)
#define SEC_TRIG_EN_OUT_CTRL_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields TRIG_EN	 */
#define SEC_TRIG_EN_WIDTH                                                     1
#define SEC_TRIG_EN_SHIFT                                                     4
#define SEC_TRIG_EN_MASK                                             0x00000010
#define SEC_TRIG_EN_RD(src)                           (((src) & 0x00000010)>>4)
#define SEC_TRIG_EN_WR(src)                      (((u32)(src)<<4) & 0x00000010)
#define SEC_TRIG_EN_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields AND_E	 */
#define SEC_AND_E_WIDTH                                                       2
#define SEC_AND_E_SHIFT                                                       2
#define SEC_AND_E_MASK                                               0x0000000c
#define SEC_AND_E_RD(src)                             (((src) & 0x0000000c)>>2)
#define SEC_AND_E_WR(src)                        (((u32)(src)<<2) & 0x0000000c)
#define SEC_AND_E_SET(dst,src) \
                       (((dst) & ~0x0000000c) | (((u32)(src)<<2) & 0x0000000c))
/*	 Fields OR_E	 */
#define SEC_OR_E_WIDTH                                                        2
#define SEC_OR_E_SHIFT                                                        0
#define SEC_OR_E_MASK                                                0x00000003
#define SEC_OR_E_RD(src)                                 (((src) & 0x00000003))
#define SEC_OR_E_WR(src)                            (((u32)(src)) & 0x00000003)
#define SEC_OR_E_SET(dst,src) \
                          (((dst) & ~0x00000003) | (((u32)(src)) & 0x00000003))

/*	Register CFG_DBG_PAT_REG_0	*/
/*	 Fields PATTERN	 */
#define SEC_PATTERN0_WIDTH                                                   32
#define SEC_PATTERN0_SHIFT                                                    0
#define SEC_PATTERN0_MASK                                            0xffffffff
#define SEC_PATTERN0_RD(src)                             (((src) & 0xffffffff))
#define SEC_PATTERN0_WR(src)                        (((u32)(src)) & 0xffffffff)
#define SEC_PATTERN0_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CFG_DBG_PAT_MASK_REG_0	*/
/*	 Fields PAT_MASK	 */
#define SEC_PAT_MASK0_WIDTH                                                  32
#define SEC_PAT_MASK0_SHIFT                                                   0
#define SEC_PAT_MASK0_MASK                                           0xffffffff
#define SEC_PAT_MASK0_RD(src)                            (((src) & 0xffffffff))
#define SEC_PAT_MASK0_WR(src)                       (((u32)(src)) & 0xffffffff)
#define SEC_PAT_MASK0_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CFG_DBG_PAT_REG_1	*/
/*	 Fields PATTERN	 */
#define SEC_PATTERN1_WIDTH                                                   32
#define SEC_PATTERN1_SHIFT                                                    0
#define SEC_PATTERN1_MASK                                            0xffffffff
#define SEC_PATTERN1_RD(src)                             (((src) & 0xffffffff))
#define SEC_PATTERN1_WR(src)                        (((u32)(src)) & 0xffffffff)
#define SEC_PATTERN1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CFG_DBG_PAT_MASK_REG_1	*/
/*	 Fields PAT_MASK	 */
#define SEC_PAT_MASK1_WIDTH                                                  32
#define SEC_PAT_MASK1_SHIFT                                                   0
#define SEC_PAT_MASK1_MASK                                           0xffffffff
#define SEC_PAT_MASK1_RD(src)                            (((src) & 0xffffffff))
#define SEC_PAT_MASK1_WR(src)                       (((u32)(src)) & 0xffffffff)
#define SEC_PAT_MASK1_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register DBG_TRIG_OUT	*/
/*	 Fields DBG_OUT	 */
#define SEC_DBG_OUT_WIDTH                                                    32
#define SEC_DBG_OUT_SHIFT                                                     0
#define SEC_DBG_OUT_MASK                                             0xffffffff
#define SEC_DBG_OUT_RD(src)                              (((src) & 0xffffffff))
#define SEC_DBG_OUT_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register DBG_TRIG_INT	*/
/*	 Fields DBG_INT	 */
#define SEC_DBG_INT_WIDTH                                                     1
#define SEC_DBG_INT_SHIFT                                                     0
#define SEC_DBG_INT_MASK                                             0x00000001
#define SEC_DBG_INT_RD(src)                              (((src) & 0x00000001))
#define SEC_DBG_INT_WR(src)                         (((u32)(src)) & 0x00000001)
#define SEC_DBG_INT_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register DBG_TRIG_INTMask	*/
/*	 Fields DBG_INTMASK	 */
#define SEC_DBG_INTMASK_WIDTH                                                     1
#define SEC_DBG_INTMASK_SHIFT                                                     0
#define SEC_DBG_INTMASK_MASK                                             0x00000001
#define SEC_DBG_INTMASK_RD(src)                              (((src) & 0x00000001))
#define SEC_DBG_INTMASK_WR(src)                         (((u32)(src)) & 0x00000001)
#define SEC_DBG_INTMASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register INTR_STS	*/
/*	 Fields DIAGMOD_INT	 */
#define SEC_DIAGMOD_INT_WIDTH                                          1
#define SEC_DIAGMOD_INT_SHIFT                                          1
#define SEC_DIAGMOD_INT_MASK                                   0x00000002
#define SEC_DIAGMOD_INT_RD(src)                (((src) & 0x00000002)>>1)
#define SEC_DIAGMOD_INT_WR(src)           (((u32)(src)<<1) & 0x00000002)
#define SEC_DIAGMOD_INT_SET(dst,src) \
                      (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields ERRMOD_INT	 */
#define SEC_ERRMOD_INT_WIDTH                                              1
#define SEC_ERRMOD_INT_SHIFT                                              0
#define SEC_ERRMOD_INT_MASK                                      0x00000001
#define SEC_ERRMOD_INT_RD(src)                       (((src) & 0x00000001))
#define SEC_ERRMOD_INT_WR(src)                  (((u32)(src)) & 0x00000001)
#define SEC_ERRMOD_INT_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register CFG_MEM_ECC_BYPASS	*/
/*	 Fields CFG_ECC_BYPASS	 */
#define SEC_CFG_ECC_BYPASS_WIDTH                                             16
#define SEC_CFG_ECC_BYPASS_SHIFT                                              0
#define SEC_CFG_ECC_BYPASS_MASK                                      0x0000ffff
#define SEC_CFG_ECC_BYPASS_RD(src)                       (((src) & 0x0000ffff))
#define SEC_CFG_ECC_BYPASS_WR(src)                  (((u32)(src)) & 0x0000ffff)
#define SEC_CFG_ECC_BYPASS_SET(dst,src) \
                          (((dst) & ~0x0000ffff) | (((u32)(src)) & 0x0000ffff))

/*	Register CFG_MEM_PWRDN_DIS	*/
/*	 Fields CFG_ECC_PWRDN_DIS	 */
#define SEC_CFG_ECC_PWRDN_DIS_WIDTH                                          16
#define SEC_CFG_ECC_PWRDN_DIS_SHIFT                                          16
#define SEC_CFG_ECC_PWRDN_DIS_MASK                                   0xffff0000
#define SEC_CFG_ECC_PWRDN_DIS_RD(src)                (((src) & 0xffff0000)>>16)
#define SEC_CFG_ECC_PWRDN_DIS_WR(src)           (((u32)(src)<<16) & 0xffff0000)
#define SEC_CFG_ECC_PWRDN_DIS_SET(dst,src) \
                      (((dst) & ~0xffff0000) | (((u32)(src)<<16) & 0xffff0000))
/*	 Fields CFG_PWRDN_DIS	 */
#define SEC_CFG_PWRDN_DIS_WIDTH                                              16
#define SEC_CFG_PWRDN_DIS_SHIFT                                               0
#define SEC_CFG_PWRDN_DIS_MASK                                       0x0000ffff
#define SEC_CFG_PWRDN_DIS_RD(src)                        (((src) & 0x0000ffff))
#define SEC_CFG_PWRDN_DIS_WR(src)                   (((u32)(src)) & 0x0000ffff)
#define SEC_CFG_PWRDN_DIS_SET(dst,src) \
                          (((dst) & ~0x0000ffff) | (((u32)(src)) & 0x0000ffff))

/*	Register CFG_MEM_RAM_SHUTDOWN	*/
/*	 Fields CFG_RAM_SHUTDOWN_EN	 */
#define SEC_CFG_RAM_SHUTDOWN_EN_WIDTH                                        32
#define SEC_CFG_RAM_SHUTDOWN_EN_SHIFT                                         0
#define SEC_CFG_RAM_SHUTDOWN_EN_MASK                                 0xffffffff
#define SEC_CFG_RAM_SHUTDOWN_EN_RD(src)                  (((src) & 0xffffffff))
#define SEC_CFG_RAM_SHUTDOWN_EN_WR(src)             (((u32)(src)) & 0xffffffff)
#define SEC_CFG_RAM_SHUTDOWN_EN_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register BLOCK_MEM_RDY	*/
/*	 Fields MEM_RDY	 */
#define SEC_MEM_RDY_WIDTH                                                    32
#define SEC_MEM_RDY_SHIFT                                                     0
#define SEC_MEM_RDY_MASK                                             0xffffffff
#define SEC_MEM_RDY_RD(src)                              (((src) & 0xffffffff))
#define SEC_MEM_RDY_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register STS_READ_LATENCY_TOT_READ_REQS	*/
/*	 Fields TOT_READS	 */
#define SEC_TOT_READS_WIDTH                                                  16
#define SEC_TOT_READS_SHIFT                                                  16
#define SEC_TOT_READS_MASK                                           0xffff0000
#define SEC_TOT_READS_RD(src)                            (((src) & 0xffff0000)>>16)
#define SEC_TOT_READS_SET(dst,src) \
                          (((dst) & ~0xffff0000) | (((u32)(src))<<16 & 0xffff0000))

/*	Register CFG_LT_MSTR_STOP_CNT	*/
/*	 Fields MSTR_LT_STOP_CNT	 */
#define SEC_MSTR_LT_STOP_CNT_WIDTH                                                  16
#define SEC_MSTR_LT_STOP_CNT_SHIFT                                                  16
#define SEC_MSTR_LT_STOP_CNT_MASK                                           0xffff0000
#define SEC_MSTR_LT_STOP_CNT_RD(src)                            (((src) & 0xffff0000)>>16)
#define SEC_MSTR_LT_STOP_CNT_SET(dst,src) \
                          (((dst) & ~0xffff0000) | (((u32)(src))<<16 & 0xffff0000))

/*	Register CFG_BW_SRD_TRIG_CAP	*/
/*	 Fields CAP_ADD_BWSRD	 */
#define SEC_CAP_ADD_BWSRD_WIDTH                                                    32
#define SEC_CAP_ADD_BWSRD_SHIFT                                                     0
#define SEC_CAP_ADD_BWSRD_MASK                                             0xffffffff
#define SEC_CAP_ADD_BWSRD_RD(src)                              (((src) & 0xffffffff))
#define SEC_CAP_ADD_BWSRD_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register CFG_BW_SWR_TRIG_CAP	*/
/*	 Fields CAP_ADD_BWSWR	 */
#define SEC_CAP_ADD_BWSWR_WIDTH                                                    32
#define SEC_CAP_ADD_BWSWR_SHIFT                                                     0
#define SEC_CAP_ADD_BWSWR_MASK                                             0xffffffff
#define SEC_CAP_ADD_BWSWR_RD(src)                              (((src) & 0xffffffff))
#define SEC_CAP_ADD_BWSWR_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register CFG_BW_MRD_TRIG_CAP    */
/*       Fields CAP_ADD_BWMRD    */
#define SEC_CAP_ADD_BWMRD_WIDTH                                                    32
#define SEC_CAP_ADD_BWMRD_SHIFT                                                     0
#define SEC_CAP_ADD_BWMRD_MASK                                             0xffffffff
#define SEC_CAP_ADD_BWMRD_RD(src)                              (((src) & 0xffffffff))
#define SEC_CAP_ADD_BWMRD_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register CFG_BW_MWR_TRIG_CAP    */
/*       Fields CAP_ADD_BWMWR    */
#define SEC_CAP_ADD_BWMWR_WIDTH                                                    32
#define SEC_CAP_ADD_BWMWR_SHIFT                                                     0
#define SEC_CAP_ADD_BWMWR_MASK                                             0xffffffff
#define SEC_CAP_ADD_BWMWR_RD(src)                              (((src) & 0xffffffff))
#define SEC_CAP_ADD_BWMWR_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register CFG_BW_MWR_TRIG_CAP    */
/*       Fields CAP_ADD_BWMWR    */
#define SEC_CAP_ADD_BWMWR_WIDTH                                                    32
#define SEC_CAP_ADD_BWMWR_SHIFT                                                     0
#define SEC_CAP_ADD_BWMWR_MASK                                             0xffffffff
#define SEC_CAP_ADD_BWMWR_RD(src)                              (((src) & 0xffffffff))
#define SEC_CAP_ADD_BWMWR_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register DBG_BLOCK_AXI    */
/*       Fields DBG_BUS_BLOCK_AXI    */
#define SEC_DBG_BUS_BLOCK_AXI_WIDTH                                                    32
#define SEC_DBG_BUS_BLOCK_AXI_SHIFT                                                     0
#define SEC_DBG_BUS_BLOCK_AXI_MASK                                             0xffffffff
#define SEC_DBG_BUS_BLOCK_AXI_RD(src)                              (((src) & 0xffffffff))
#define SEC_DBG_BUS_BLOCK_AXI_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register DBG_BLOCK_NON_AXI    */
/*       Fields DBG_BUS_BLOCK_NON_AXI    */
#define SEC_DBG_BUS_BLOCK_NON_AXI_WIDTH                                                    32
#define SEC_DBG_BUS_BLOCK_NON_AXI_SHIFT                                                     0
#define SEC_DBG_BUS_BLOCK_NON_AXI_MASK                                             0xffffffff
#define SEC_DBG_BUS_BLOCK_NON_AXI_RD(src)                              (((src) & 0xffffffff))
#define SEC_DBG_BUS_BLOCK_NON_AXI_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register DBG_AXI_SHIM_OUT    */
/*       Fields DBG_BUS_SHIM    */
#define SEC_DBG_BUS_SHIM_WIDTH                                                    32
#define SEC_DBG_BUS_SHIM_SHIFT                                                     0
#define SEC_DBG_BUS_SHIM_MASK                                             0xffffffff
#define SEC_DBG_BUS_SHIM_RD(src)                              (((src) & 0xffffffff))
#define SEC_DBG_BUS_SHIM_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Global Base Address     */
#define SEC_GLBL_ERR_CSR_BASE_ADDR                     0x1f25d800

/*    Address GLBL_DIAG_CSR  Registers */
#define SEC_GLBL_MERR_ADDR_ADDR				0x00000050
#define SEC_GLBL_MERR_ADDR_DEFAULT                      0x00000000
#define SEC_GLBL_MERR_REQINFO_ADDR			0x00000054
#define SEC_GLBL_MERR_REQINFO_DEFAULT                   0x00000000
#define SEC_GLBL_WDERR_ADDR_ADDR			0x00000070
#define SEC_GLBL_WDERR_ADDR_DEFAULT                     0x00000000
#define SEC_GLBL_WDERR_REQINFO_ADDR			0x00000074
#define SEC_GLBL_WDERR_REQINFO_DEFAULT                  0x00000000
#define SEC_GLBL_DEVERR_ADDR_ADDR			0x00000078
#define SEC_GLBL_DEVERR_ADDR_DEFAULT                    0x00000000
#define SEC_GLBL_DEVERR_REQINFO_ADDR			0x0000007c
#define SEC_GLBL_DEVERR_REQINFO_DEFAULT                 0x00000000
#define SEC_GLBL_SEC_ERRL_ALS_ADDR			0x00000080
#define SEC_GLBL_SEC_ERRL_ALS_DEFAULT                   0x00000000
#define SEC_GLBL_SEC_ERRH_ALS_ADDR			0x00000084
#define SEC_GLBL_SEC_ERRH_ALS_DEFAULT                   0x00000000
#define SEC_GLBL_DED_ERRL_ALS_ADDR			0x00000088
#define SEC_GLBL_DED_ERRL_ALS_DEFAULT                   0x00000000
#define SEC_GLBL_DED_ERRH_ALS_ADDR			0x0000008c
#define SEC_GLBL_DED_ERRH_ALS_DEFAULT                   0x00000000
#define SEC_GLBL_TRANS_ERR_ALS_ADDR			0x00000090
#define SEC_GLBL_TRANS_ERR_ALS_DEFAULT                  0x00000000
#define SEC_GLBL_SEC_ERRL_ADDR				0x00000010
#define SEC_GLBL_SEC_ERRL_DEFAULT                       0x00000000
#define SEC_GLBL_SEC_ERRLMASK_ADDR			0x00000014
#define SEC_GLBL_SEC_ERRLMASK_DEFAULT                   0xffffffff
#define SEC_GLBL_SEC_ERRH_ADDR				0x00000018
#define SEC_GLBL_SEC_ERRH_DEFAULT                       0x00000000
#define SEC_GLBL_SEC_ERRHMASK_ADDR			0x0000001c
#define SEC_GLBL_SEC_ERRHMASK_DEFAULT                   0xffffffff
#define SEC_GLBL_MSEC_ERRL_ADDR				0x00000020
#define SEC_GLBL_MSEC_ERRL_DEFAULT                      0x00000000
#define SEC_GLBL_MSEC_ERRLMASK_ADDR			0x00000024
#define SEC_GLBL_MSEC_ERRLMASK_DEFAULT                  0xffffffff
#define SEC_GLBL_MSEC_ERRH_ADDR				0x00000028
#define SEC_GLBL_MSEC_ERRH_DEFAULT                      0x00000000
#define SEC_GLBL_MSEC_ERRHMASK_ADDR			0x0000002c
#define SEC_GLBL_MSEC_ERRHMASK_DEFAULT                  0xffffffff
#define SEC_GLBL_DED_ERRL_ADDR				0x00000030
#define SEC_GLBL_DED_ERRL_DEFAULT                       0x00000000
#define SEC_GLBL_DED_ERRLMASK_ADDR			0x00000034
#define SEC_GLBL_DED_ERRLMASK_DEFAULT                   0xffffffff
#define SEC_GLBL_DED_ERRH_ADDR				0x00000038
#define SEC_GLBL_DED_ERRH_DEFAULT                       0x00000000
#define SEC_GLBL_DED_ERRHMASK_ADDR			0x0000003c
#define SEC_GLBL_DED_ERRHMASK_DEFAULT                   0xffffffff
#define SEC_GLBL_MDED_ERRL_ADDR				0x00000040
#define SEC_GLBL_MDED_ERRL_DEFAULT                      0x00000000
#define SEC_GLBL_MDED_ERRLMASK_ADDR			0x00000044
#define SEC_GLBL_MDED_ERRLMASK_DEFAULT                  0xffffffff
#define SEC_GLBL_MDED_ERRH_ADDR				0x00000048
#define SEC_GLBL_MDED_ERRH_DEFAULT                      0x00000000
#define SEC_GLBL_MDED_ERRHMASK_ADDR			0x0000004c
#define SEC_GLBL_MDED_ERRHMASK_DEFAULT                  0xffffffff
#define SEC_GLBL_TRANS_ERR_ADDR				0x00000060
#define SEC_GLBL_TRANS_ERR_DEFAULT                      0x00000000
#define SEC_GLBL_TRANS_ERRMASK_ADDR			0x00000064
#define SEC_GLBL_TRANS_ERRMASK_DEFAULT                  0xffffffff
#define SEC_GLBL_ERR_STS_ADDR				0x00000000
#define SEC_GLBL_ERR_STS_DEFAULT                        0x00000000

/*	Register GLBL_ERR_STS	*/
/*	 Fields SHIM_ERR	 */
#define SHIM_ERR_WIDTH                                                        1
#define SHIM_ERR_SHIFT                                                         5
#define SHIM_ERR_MASK                                                 0x00000020
#define SHIM_ERR_RD(src)                               (((src) & 0x00000020)>>5)
#define SHIM_ERR_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*	 Fields RANS_ERR	 */
#define TRANS_ERR_WIDTH                                                      1
#define TRANS_ERR_SHIFT                                                      4
#define TRANS_ERR_MASK                                              0x00000010
#define TRANS_ERR_RD(src)                            (((src) & 0x00000010)>>4)
#define TRANS_ERR_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*	 Fields MDED_ERR	 */
#define MDED_ERR_WIDTH                                                            1
#define MDED_ERR_SHIFT                                                            3
#define MDED_ERR_MASK                                                    0x00000008
#define MDED_ERR_RD(src)                                  (((src) & 0x00000008)>>3)
#define MDED_ERR_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*	 Fields DED_ERR 	 */
#define DED_ERR_WIDTH                                                            1
#define DED_ERR_SHIFT                                                            2
#define DED_ERR_MASK                                                    0x00000004
#define DED_ERR_RD(src)                                  (((src) & 0x00000004)>>2)
#define DED_ERR_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields MSEC_ERR          */
#define MSEC_ERR_WIDTH                                                            1
#define MSEC_ERR_SHIFT                                                            1
#define MSEC_ERR_MASK                                                    0x00000002
#define MSEC_ERR_RD(src)                                  (((src) & 0x00000002)>>1)
#define MSEC_ERR_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC_ERR 	 */
#define SEC_ERR_WIDTH                                                   1
#define SEC_ERR_SHIFT                                                   0
#define SEC_ERR_MASK                                           0x00000001
#define SEC_ERR_RD(src)                            (((src) & 0x00000001))
#define SEC_ERR_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_SEC_ERRL	*/
/*	 Fields SEC31	 */
#define SEC_SEC31_WIDTH                                            1
#define SEC_SEC31_SHIFT                                            31
#define SEC_SEC31_MASK                                    0x80000000
#define SEC_SEC31_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_SEC31_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_SEC31_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30    */
#define SEC_SEC30_WIDTH                                            1
#define SEC_SEC30_SHIFT                                            30
#define SEC_SEC30_MASK                                    0x40000000
#define SEC_SEC30_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_SEC30_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_SEC30_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_SEC29_WIDTH                                            1
#define SEC_SEC29_SHIFT                                            29
#define SEC_SEC29_MASK                                    0x20000000
#define SEC_SEC29_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_SEC29_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_SEC29_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_SEC28_WIDTH                                            1
#define SEC_SEC28_SHIFT                                            28
#define SEC_SEC28_MASK                                    0x10000000
#define SEC_SEC28_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_SEC28_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_SEC28_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_SEC27_WIDTH                                            1
#define SEC_SEC27_SHIFT                                            27
#define SEC_SEC27_MASK                                    0x08000000
#define SEC_SEC27_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_SEC27_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_SEC27_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_SEC26_WIDTH                                            1
#define SEC_SEC26_SHIFT                                            26
#define SEC_SEC26_MASK                                    0x04000000
#define SEC_SEC26_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_SEC26_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_SEC26_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_SEC25_WIDTH                                            1
#define SEC_SEC25_SHIFT                                            25
#define SEC_SEC25_MASK                                    0x02000000
#define SEC_SEC25_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_SEC25_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_SEC25_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_SEC24_WIDTH                                            1
#define SEC_SEC24_SHIFT                                            24
#define SEC_SEC24_MASK                                    0x01000000
#define SEC_SEC24_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_SEC24_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_SEC24_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_SEC23_WIDTH                                            1
#define SEC_SEC23_SHIFT                                            23
#define SEC_SEC23_MASK                                    0x00800000
#define SEC_SEC23_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_SEC23_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_SEC23_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_SEC22_WIDTH                                            1
#define SEC_SEC22_SHIFT                                            22
#define SEC_SEC22_MASK                                    0x00400000
#define SEC_SEC22_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_SEC22_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_SEC22_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_SEC21_WIDTH                                            1
#define SEC_SEC21_SHIFT                                            21
#define SEC_SEC21_MASK                                    0x00200000
#define SEC_SEC21_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_SEC21_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_SEC21_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_SEC20_WIDTH                                            1
#define SEC_SEC20_SHIFT                                            20
#define SEC_SEC20_MASK                                    0x00100000
#define SEC_SEC20_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_SEC20_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_SEC20_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_SEC19_WIDTH                                            1
#define SEC_SEC19_SHIFT                                            19
#define SEC_SEC19_MASK                                    0x00080000
#define SEC_SEC19_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_SEC19_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_SEC19_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_SEC18_WIDTH                                            1
#define SEC_SEC18_SHIFT                                            18
#define SEC_SEC18_MASK                                    0x00040000
#define SEC_SEC18_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_SEC18_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_SEC18_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_SEC17_WIDTH                                            1
#define SEC_SEC17_SHIFT                                            17
#define SEC_SEC17_MASK                                    0x00020000
#define SEC_SEC17_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_SEC17_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_SEC17_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_SEC16_WIDTH                                            1
#define SEC_SEC16_SHIFT                                            16
#define SEC_SEC16_MASK                                    0x00010000
#define SEC_SEC16_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_SEC16_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_SEC16_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_SEC15_WIDTH                                            1
#define SEC_SEC15_SHIFT                                            15
#define SEC_SEC15_MASK                                    0x00008000
#define SEC_SEC15_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_SEC15_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_SEC15_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_SEC14_WIDTH                                            1
#define SEC_SEC14_SHIFT                                            14
#define SEC_SEC14_MASK                                    0x00004000
#define SEC_SEC14_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_SEC14_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_SEC14_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_SEC13_WIDTH                                            1
#define SEC_SEC13_SHIFT                                            13
#define SEC_SEC13_MASK                                    0x00002000
#define SEC_SEC13_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_SEC13_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_SEC13_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_SEC12_WIDTH                                            1
#define SEC_SEC12_SHIFT                                            12
#define SEC_SEC12_MASK                                    0x00001000
#define SEC_SEC12_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_SEC12_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_SEC12_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_SEC11_WIDTH                                            1
#define SEC_SEC11_SHIFT                                            11
#define SEC_SEC11_MASK                                    0x00000800
#define SEC_SEC11_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_SEC11_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_SEC11_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_SEC10_WIDTH                                            1
#define SEC_SEC10_SHIFT                                            10
#define SEC_SEC10_MASK                                    0x00000400
#define SEC_SEC10_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_SEC10_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_SEC10_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_SEC9_WIDTH                                            1
#define SEC_SEC9_SHIFT                                            9
#define SEC_SEC9_MASK                                    0x00000200
#define SEC_SEC9_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_SEC9_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_SEC9_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_SEC8_WIDTH                                            1
#define SEC_SEC8_SHIFT                                            8
#define SEC_SEC8_MASK                                    0x00000100
#define SEC_SEC8_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_SEC8_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_SEC8_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_SEC7_WIDTH                                            1
#define SEC_SEC7_SHIFT                                            7
#define SEC_SEC7_MASK                                    0x00000080
#define SEC_SEC7_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_SEC7_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_SEC7_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_SEC6_WIDTH                                            1
#define SEC_SEC6_SHIFT                                            6
#define SEC_SEC6_MASK                                    0x00000040
#define SEC_SEC6_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_SEC6_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_SEC6_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_SEC5_WIDTH                                            1
#define SEC_SEC5_SHIFT                                            5
#define SEC_SEC5_MASK                                    0x00000020
#define SEC_SEC5_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_SEC5_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_SEC5_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_SEC4_WIDTH                                            1
#define SEC_SEC4_SHIFT                                            4
#define SEC_SEC4_MASK                                    0x00000010
#define SEC_SEC4_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_SEC4_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_SEC4_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_SEC3_WIDTH                                            1
#define SEC_SEC3_SHIFT                                            3
#define SEC_SEC3_MASK                                    0x00000008
#define SEC_SEC3_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_SEC3_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_SEC3_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_SEC2_WIDTH                                            1
#define SEC_SEC2_SHIFT                                            2
#define SEC_SEC2_MASK                                    0x00000004
#define SEC_SEC2_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_SEC2_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_SEC2_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_SEC1_WIDTH                                            1
#define SEC_SEC1_SHIFT                                            1
#define SEC_SEC1_MASK                                    0x00000002
#define SEC_SEC1_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_SEC1_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_SEC1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_SEC0_WIDTH                                                1
#define SEC_SEC0_SHIFT                                                0
#define SEC_SEC0_MASK                                        0x00000001
#define SEC_SEC0_RD(src)                         (((src) & 0x00000001))
#define SEC_SEC0_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_SEC0_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_SEC_ERRLMASK	*/
/*	 Fields SEC31_MASK	 */
#define SEC_SEC31_MASK_WIDTH                                            1
#define SEC_SEC31_MASK_SHIFT                                            31
#define SEC_SEC31_MASK_MASK                                    0x80000000
#define SEC_SEC31_MASK_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_SEC31_MASK_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_SEC31_MASK_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30_MASK    */
#define SEC_SEC30_MASK_WIDTH                                            1
#define SEC_SEC30_MASK_SHIFT                                            30
#define SEC_SEC30_MASK_MASK                                    0x40000000
#define SEC_SEC30_MASK_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_SEC30_MASK_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_SEC30_MASK_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_SEC29_MASK_WIDTH                                            1
#define SEC_SEC29_MASK_SHIFT                                            29
#define SEC_SEC29_MASK_MASK                                    0x20000000
#define SEC_SEC29_MASK_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_SEC29_MASK_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_SEC29_MASK_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_SEC28_MASK_WIDTH                                            1
#define SEC_SEC28_MASK_SHIFT                                            28
#define SEC_SEC28_MASK_MASK                                    0x10000000
#define SEC_SEC28_MASK_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_SEC28_MASK_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_SEC28_MASK_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_SEC27_MASK_WIDTH                                            1
#define SEC_SEC27_MASK_SHIFT                                            27
#define SEC_SEC27_MASK_MASK                                    0x08000000
#define SEC_SEC27_MASK_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_SEC27_MASK_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_SEC27_MASK_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_SEC26_MASK_WIDTH                                            1
#define SEC_SEC26_MASK_SHIFT                                            26
#define SEC_SEC26_MASK_MASK                                    0x04000000
#define SEC_SEC26_MASK_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_SEC26_MASK_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_SEC26_MASK_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_SEC25_MASK_WIDTH                                            1
#define SEC_SEC25_MASK_SHIFT                                            25
#define SEC_SEC25_MASK_MASK                                    0x02000000
#define SEC_SEC25_MASK_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_SEC25_MASK_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_SEC25_MASK_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_SEC24_MASK_WIDTH                                            1
#define SEC_SEC24_MASK_SHIFT                                            24
#define SEC_SEC24_MASK_MASK                                    0x01000000
#define SEC_SEC24_MASK_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_SEC24_MASK_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_SEC24_MASK_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_SEC23_MASK_WIDTH                                            1
#define SEC_SEC23_MASK_SHIFT                                            23
#define SEC_SEC23_MASK_MASK                                    0x00800000
#define SEC_SEC23_MASK_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_SEC23_MASK_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_SEC23_MASK_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_SEC22_MASK_WIDTH                                            1
#define SEC_SEC22_MASK_SHIFT                                            22
#define SEC_SEC22_MASK_MASK                                    0x00400000
#define SEC_SEC22_MASK_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_SEC22_MASK_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_SEC22_MASK_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_SEC21_MASK_WIDTH                                            1
#define SEC_SEC21_MASK_SHIFT                                            21
#define SEC_SEC21_MASK_MASK                                    0x00200000
#define SEC_SEC21_MASK_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_SEC21_MASK_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_SEC21_MASK_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_SEC20_MASK_WIDTH                                            1
#define SEC_SEC20_MASK_SHIFT                                            20
#define SEC_SEC20_MASK_MASK                                    0x00100000
#define SEC_SEC20_MASK_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_SEC20_MASK_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_SEC20_MASK_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_SEC19_MASK_WIDTH                                            1
#define SEC_SEC19_MASK_SHIFT                                            19
#define SEC_SEC19_MASK_MASK                                    0x00080000
#define SEC_SEC19_MASK_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_SEC19_MASK_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_SEC19_MASK_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_SEC18_MASK_MASK_WIDTH                                            1
#define SEC_SEC18_MASK_MASK_SHIFT                                            18
#define SEC_SEC18_MASK_MASK_MASK                                    0x00040000
#define SEC_SEC18_MASK_MASK_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_SEC18_MASK_MASK_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_SEC18_MASK_MASK_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_SEC17_MASK_WIDTH                                            1
#define SEC_SEC17_MASK_SHIFT                                            17
#define SEC_SEC17_MASK_MASK                                    0x00020000
#define SEC_SEC17_MASK_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_SEC17_MASK_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_SEC17_MASK_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_SEC16_MASK_WIDTH                                            1
#define SEC_SEC16_MASK_SHIFT                                            16
#define SEC_SEC16_MASK_MASK                                    0x00010000
#define SEC_SEC16_MASK_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_SEC16_MASK_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_SEC16_MASK_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_SEC15_MASK_WIDTH                                            1
#define SEC_SEC15_MASK_SHIFT                                            15
#define SEC_SEC15_MASK_MASK                                    0x00008000
#define SEC_SEC15_MASK_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_SEC15_MASK_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_SEC15_MASK_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_SEC14_MASK_WIDTH                                            1
#define SEC_SEC14_MASK_SHIFT                                            14
#define SEC_SEC14_MASK_MASK                                    0x00004000
#define SEC_SEC14_MASK_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_SEC14_MASK_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_SEC14_MASK_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_SEC13_MASK_WIDTH                                            1
#define SEC_SEC13_MASK_SHIFT                                            13
#define SEC_SEC13_MASK_MASK                                    0x00002000
#define SEC_SEC13_MASK_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_SEC13_MASK_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_SEC13_MASK_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_SEC12_MASK_WIDTH                                            1
#define SEC_SEC12_MASK_SHIFT                                            12
#define SEC_SEC12_MASK_MASK                                    0x00001000
#define SEC_SEC12_MASK_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_SEC12_MASK_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_SEC12_MASK_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_SEC11_MASK_WIDTH                                            1
#define SEC_SEC11_MASK_SHIFT                                            11
#define SEC_SEC11_MASK_MASK                                    0x00000800
#define SEC_SEC11_MASK_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_SEC11_MASK_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_SEC11_MASK_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_SEC10_MASK_WIDTH                                            1
#define SEC_SEC10_MASK_SHIFT                                            10
#define SEC_SEC10_MASK_MASK                                    0x00000400
#define SEC_SEC10_MASK_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_SEC10_MASK_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_SEC10_MASK_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_SEC9_MASK_WIDTH                                            1
#define SEC_SEC9_MASK_SHIFT                                            9
#define SEC_SEC9_MASK_MASK                                    0x00000200
#define SEC_SEC9_MASK_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_SEC9_MASK_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_SEC9_MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_SEC8_MASK_WIDTH                                            1
#define SEC_SEC8_MASK_SHIFT                                            8
#define SEC_SEC8_MASK_MASK                                    0x00000100
#define SEC_SEC8_MASK_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_SEC8_MASK_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_SEC8_MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_SEC7_MASK_WIDTH                                            1
#define SEC_SEC7_MASK_SHIFT                                            7
#define SEC_SEC7_MASK_MASK                                    0x00000080
#define SEC_SEC7_MASK_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_SEC7_MASK_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_SEC7_MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_SEC6_MASK_WIDTH                                            1
#define SEC_SEC6_MASK_SHIFT                                            6
#define SEC_SEC6_MASK_MASK                                    0x00000040
#define SEC_SEC6_MASK_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_SEC6_MASK_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_SEC6_MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_SEC5_MASK_WIDTH                                            1
#define SEC_SEC5_MASK_SHIFT                                            5
#define SEC_SEC5_MASK_MASK                                    0x00000020
#define SEC_SEC5_MASK_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_SEC5_MASK_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_SEC5_MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_SEC4_MASK_WIDTH                                            1
#define SEC_SEC4_MASK_SHIFT                                            4
#define SEC_SEC4_MASK_MASK                                    0x00000010
#define SEC_SEC4_MASK_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_SEC4_MASK_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_SEC4_MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_SEC3_MASK_WIDTH                                            1
#define SEC_SEC3_MASK_SHIFT                                            3
#define SEC_SEC3_MASK_MASK                                    0x00000008
#define SEC_SEC3_MASK_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_SEC3_MASK_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_SEC3_MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_SEC2_MASK_WIDTH                                            1
#define SEC_SEC2_MASK_SHIFT                                            2
#define SEC_SEC2_MASK_MASK                                    0x00000004
#define SEC_SEC2_MASK_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_SEC2_MASK_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_SEC2_MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_SEC1_MASK_WIDTH                                            1
#define SEC_SEC1_MASK_SHIFT                                            1
#define SEC_SEC1_MASK_MASK                                    0x00000002
#define SEC_SEC1_MASK_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_SEC1_MASK_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_SEC1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_SEC0_MASK_WIDTH                                                1
#define SEC_SEC0_MASK_SHIFT                                                0
#define SEC_SEC0_MASK_MASK                                        0x00000001
#define SEC_SEC0_MASK_RD(src)                         (((src) & 0x00000001))
#define SEC_SEC0_MASK_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_SEC0_MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_SEC_ERRH	*/
/*	 Fields SEC31	 */
#define SEC_SEC31_F1_WIDTH                                            1
#define SEC_SEC31_F1_SHIFT                                            31
#define SEC_SEC31_F1_MASK                                    0x80000000
#define SEC_SEC31_F1_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_SEC31_F1_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_SEC31_F1_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30    */
#define SEC_SEC30_F1_WIDTH                                            1
#define SEC_SEC30_F1_SHIFT                                            30
#define SEC_SEC30_F1_MASK                                    0x40000000
#define SEC_SEC30_F1_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_SEC30_F1_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_SEC30_F1_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_SEC29_F1_WIDTH                                            1
#define SEC_SEC29_F1_SHIFT                                            29
#define SEC_SEC29_F1_MASK                                    0x20000000
#define SEC_SEC29_F1_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_SEC29_F1_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_SEC29_F1_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_SEC28_F1_WIDTH                                            1
#define SEC_SEC28_F1_SHIFT                                            28
#define SEC_SEC28_F1_MASK                                    0x10000000
#define SEC_SEC28_F1_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_SEC28_F1_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_SEC28_F1_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_SEC27_F1_WIDTH                                            1
#define SEC_SEC27_F1_SHIFT                                            27
#define SEC_SEC27_F1_MASK                                    0x08000000
#define SEC_SEC27_F1_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_SEC27_F1_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_SEC27_F1_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_SEC26_F1_WIDTH                                            1
#define SEC_SEC26_F1_SHIFT                                            26
#define SEC_SEC26_F1_MASK                                    0x04000000
#define SEC_SEC26_F1_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_SEC26_F1_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_SEC26_F1_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_SEC25_F1_WIDTH                                            1
#define SEC_SEC25_F1_SHIFT                                            25
#define SEC_SEC25_F1_MASK                                    0x02000000
#define SEC_SEC25_F1_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_SEC25_F1_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_SEC25_F1_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_SEC24_F1_WIDTH                                            1
#define SEC_SEC24_F1_SHIFT                                            24
#define SEC_SEC24_F1_MASK                                    0x01000000
#define SEC_SEC24_F1_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_SEC24_F1_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_SEC24_F1_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_SEC23_F1_WIDTH                                            1
#define SEC_SEC23_F1_SHIFT                                            23
#define SEC_SEC23_F1_MASK                                    0x00800000
#define SEC_SEC23_F1_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_SEC23_F1_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_SEC23_F1_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_SEC22_F1_WIDTH                                            1
#define SEC_SEC22_F1_SHIFT                                            22
#define SEC_SEC22_F1_MASK                                    0x00400000
#define SEC_SEC22_F1_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_SEC22_F1_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_SEC22_F1_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_SEC21_F1_WIDTH                                            1
#define SEC_SEC21_F1_SHIFT                                            21
#define SEC_SEC21_F1_MASK                                    0x00200000
#define SEC_SEC21_F1_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_SEC21_F1_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_SEC21_F1_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_SEC20_F1_WIDTH                                            1
#define SEC_SEC20_F1_SHIFT                                            20
#define SEC_SEC20_F1_MASK                                    0x00100000
#define SEC_SEC20_F1_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_SEC20_F1_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_SEC20_F1_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_SEC19_F1_WIDTH                                            1
#define SEC_SEC19_F1_SHIFT                                            19
#define SEC_SEC19_F1_MASK                                    0x00080000
#define SEC_SEC19_F1_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_SEC19_F1_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_SEC19_F1_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_SEC18_F1_WIDTH                                            1
#define SEC_SEC18_F1_SHIFT                                            18
#define SEC_SEC18_F1_MASK                                    0x00040000
#define SEC_SEC18_F1_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_SEC18_F1_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_SEC18_F1_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_SEC17_F1_WIDTH                                            1
#define SEC_SEC17_F1_SHIFT                                            17
#define SEC_SEC17_F1_MASK                                    0x00020000
#define SEC_SEC17_F1_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_SEC17_F1_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_SEC17_F1_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_SEC16_F1_WIDTH                                            1
#define SEC_SEC16_F1_SHIFT                                            16
#define SEC_SEC16_F1_MASK                                    0x00010000
#define SEC_SEC16_F1_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_SEC16_F1_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_SEC16_F1_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_SEC15_F1_WIDTH                                            1
#define SEC_SEC15_F1_SHIFT                                            15
#define SEC_SEC15_F1_MASK                                    0x00008000
#define SEC_SEC15_F1_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_SEC15_F1_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_SEC15_F1_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_SEC14_F1_WIDTH                                            1
#define SEC_SEC14_F1_SHIFT                                            14
#define SEC_SEC14_F1_MASK                                    0x00004000
#define SEC_SEC14_F1_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_SEC14_F1_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_SEC14_F1_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_SEC13_F1_WIDTH                                            1
#define SEC_SEC13_F1_SHIFT                                            13
#define SEC_SEC13_F1_MASK                                    0x00002000
#define SEC_SEC13_F1_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_SEC13_F1_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_SEC13_F1_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_SEC12_F1_WIDTH                                            1
#define SEC_SEC12_F1_SHIFT                                            12
#define SEC_SEC12_F1_MASK                                    0x00001000
#define SEC_SEC12_F1_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_SEC12_F1_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_SEC12_F1_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_SEC11_F1_WIDTH                                            1
#define SEC_SEC11_F1_SHIFT                                            11
#define SEC_SEC11_F1_MASK                                    0x00000800
#define SEC_SEC11_F1_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_SEC11_F1_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_SEC11_F1_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_SEC10_F1_WIDTH                                            1
#define SEC_SEC10_F1_SHIFT                                            10
#define SEC_SEC10_F1_MASK                                    0x00000400
#define SEC_SEC10_F1_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_SEC10_F1_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_SEC10_F1_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_SEC9_F1_WIDTH                                            1
#define SEC_SEC9_F1_SHIFT                                            9
#define SEC_SEC9_F1_MASK                                    0x00000200
#define SEC_SEC9_F1_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_SEC9_F1_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_SEC9_F1_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_SEC8_F1_WIDTH                                            1
#define SEC_SEC8_F1_SHIFT                                            8
#define SEC_SEC8_F1_MASK                                    0x00000100
#define SEC_SEC8_F1_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_SEC8_F1_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_SEC8_F1_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_SEC7_F1_WIDTH                                            1
#define SEC_SEC7_F1_SHIFT                                            7
#define SEC_SEC7_F1_MASK                                    0x00000080
#define SEC_SEC7_F1_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_SEC7_F1_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_SEC7_F1_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_SEC6_F1_WIDTH                                            1
#define SEC_SEC6_F1_SHIFT                                            6
#define SEC_SEC6_F1_MASK                                    0x00000040
#define SEC_SEC6_F1_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_SEC6_F1_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_SEC6_F1_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_SEC5_F1_WIDTH                                            1
#define SEC_SEC5_F1_SHIFT                                            5
#define SEC_SEC5_F1_MASK                                    0x00000020
#define SEC_SEC5_F1_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_SEC5_F1_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_SEC5_F1_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_SEC4_F1_WIDTH                                            1
#define SEC_SEC4_F1_SHIFT                                            4
#define SEC_SEC4_F1_MASK                                    0x00000010
#define SEC_SEC4_F1_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_SEC4_F1_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_SEC4_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_SEC3_F1_WIDTH                                            1
#define SEC_SEC3_F1_SHIFT                                            3
#define SEC_SEC3_F1_MASK                                    0x00000008
#define SEC_SEC3_F1_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_SEC3_F1_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_SEC3_F1_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_SEC2_F1_WIDTH                                            1
#define SEC_SEC2_F1_SHIFT                                            2
#define SEC_SEC2_F1_MASK                                    0x00000004
#define SEC_SEC2_F1_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_SEC2_F1_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_SEC2_F1_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_SEC1_F1_WIDTH                                            1
#define SEC_SEC1_F1_SHIFT                                            1
#define SEC_SEC1_F1_MASK                                    0x00000002
#define SEC_SEC1_F1_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_SEC1_F1_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_SEC1_F1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_SEC0_F1_WIDTH                                                1
#define SEC_SEC0_F1_SHIFT                                                0
#define SEC_SEC0_F1_MASK                                        0x00000001
#define SEC_SEC0_F1_RD(src)                         (((src) & 0x00000001))
#define SEC_SEC0_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_SEC0_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_SEC_ERRHMASK	*/
/*	 Fields SEC31_F1_MASK	 */
#define SEC_SEC31_F1_MASK_WIDTH                                            1
#define SEC_SEC31_F1_MASK_SHIFT                                            31
#define SEC_SEC31_F1_MASK_MASK                                    0x80000000
#define SEC_SEC31_F1_MASK_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_SEC31_F1_MASK_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_SEC31_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30_F1_MASK    */
#define SEC_SEC30_F1_MASK_WIDTH                                            1
#define SEC_SEC30_F1_MASK_SHIFT                                            30
#define SEC_SEC30_F1_MASK_MASK                                    0x40000000
#define SEC_SEC30_F1_MASK_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_SEC30_F1_MASK_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_SEC30_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_SEC29_F1_MASK_WIDTH                                            1
#define SEC_SEC29_F1_MASK_SHIFT                                            29
#define SEC_SEC29_F1_MASK                                    0x20000000
#define SEC_SEC29_F1_MASK_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_SEC29_F1_MASK_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_SEC29_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_SEC28_F1_MASK_WIDTH                                            1
#define SEC_SEC28_F1_MASK_SHIFT                                            28
#define SEC_SEC28_F1_MASK_MASK                                    0x10000000
#define SEC_SEC28_F1_MASK_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_SEC28_F1_MASK_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_SEC28_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_SEC27_F1_MASK_WIDTH                                            1
#define SEC_SEC27_F1_MASK_SHIFT                                            27
#define SEC_SEC27_F1_MASK_MASK                                    0x08000000
#define SEC_SEC27_F1_MASK_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_SEC27_F1_MASK_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_SEC27_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_SEC26_F1_MASK_WIDTH                                            1
#define SEC_SEC26_F1_MASK_SHIFT                                            26
#define SEC_SEC26_F1_MASK_MASK                                    0x04000000
#define SEC_SEC26_F1_MASK_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_SEC26_F1_MASK_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_SEC26_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_SEC25_F1_MASK_WIDTH                                            1
#define SEC_SEC25_F1_MASK_SHIFT                                            25
#define SEC_SEC25_F1_MASK_MASK                                    0x02000000
#define SEC_SEC25_F1_MASK_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_SEC25_F1_MASK_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_SEC25_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_SEC24_F1_MASK_WIDTH                                            1
#define SEC_SEC24_F1_MASK_SHIFT                                            24
#define SEC_SEC24_F1_MASK_MASK                                    0x01000000
#define SEC_SEC24_F1_MASK_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_SEC24_F1_MASK_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_SEC24_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_SEC23_F1_MASK_WIDTH                                            1
#define SEC_SEC23_F1_MASK_SHIFT                                            23
#define SEC_SEC23_F1_MASK_MASK                                    0x00800000
#define SEC_SEC23_F1_MASK_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_SEC23_F1_MASK_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_SEC23_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_SEC22_F1_MASK_WIDTH                                            1
#define SEC_SEC22_F1_MASK_SHIFT                                            22
#define SEC_SEC22_F1_MASK_MASK                                    0x00400000
#define SEC_SEC22_F1_MASK_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_SEC22_F1_MASK_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_SEC22_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_SEC21_F1_MASK_WIDTH                                            1
#define SEC_SEC21_F1_MASK_SHIFT                                            21
#define SEC_SEC21_F1_MASK_MASK                                    0x00200000
#define SEC_SEC21_F1_MASK_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_SEC21_F1_MASK_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_SEC21_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_SEC20_F1_MASK_WIDTH                                            1
#define SEC_SEC20_F1_MASK_SHIFT                                            20
#define SEC_SEC20_F1_MASK_MASK                                    0x00100000
#define SEC_SEC20_F1_MASK_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_SEC20_F1_MASK_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_SEC20_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_SEC19_F1_MASK_WIDTH                                            1
#define SEC_SEC19_F1_MASK_SHIFT                                            19
#define SEC_SEC19_F1_MASK_MASK                                    0x00080000
#define SEC_SEC19_F1_MASK_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_SEC19_F1_MASK_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_SEC19_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_SEC18_F1_MASK_WIDTH                                            1
#define SEC_SEC18_F1_MASK_SHIFT                                            18
#define SEC_SEC18_F1_MASK_MASK                                    0x00040000
#define SEC_SEC18_F1_MASK_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_SEC18_F1_MASK_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_SEC18_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_SEC17_F1_MASK_WIDTH                                            1
#define SEC_SEC17_F1_MASK_SHIFT                                            17
#define SEC_SEC17_F1_MASK_MASK                                    0x00020000
#define SEC_SEC17_F1_MASK_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_SEC17_F1_MASK_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_SEC17_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_SEC16_F1_MASK_WIDTH                                            1
#define SEC_SEC16_F1_MASK_SHIFT                                            16
#define SEC_SEC16_F1_MASK_MASK                                    0x00010000
#define SEC_SEC16_F1_MASK_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_SEC16_F1_MASK_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_SEC16_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_SEC15_F1_MASK_WIDTH                                            1
#define SEC_SEC15_F1_MASK_SHIFT                                            15
#define SEC_SEC15_F1_MASK_MASK                                    0x00008000
#define SEC_SEC15_F1_MASK_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_SEC15_F1_MASK_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_SEC15_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_SEC14_F1_MASK_WIDTH                                            1
#define SEC_SEC14_F1_MASK_SHIFT                                            14
#define SEC_SEC14_F1_MASK_MASK                                    0x00004000
#define SEC_SEC14_F1_MASK_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_SEC14_F1_MASK_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_SEC14_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_SEC13_F1_MASK_WIDTH                                            1
#define SEC_SEC13_F1_MASK_SHIFT                                            13
#define SEC_SEC13_F1_MASK_MASK                                    0x00002000
#define SEC_SEC13_F1_MASK_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_SEC13_F1_MASK_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_SEC13_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_SEC12_F1_MASK_WIDTH                                            1
#define SEC_SEC12_F1_MASK_SHIFT                                            12
#define SEC_SEC12_F1_MASK_MASK                                    0x00001000
#define SEC_SEC12_F1_MASK_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_SEC12_F1_MASK_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_SEC12_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_SEC11_F1_MASK_WIDTH                                            1
#define SEC_SEC11_F1_MASK_SHIFT                                            11
#define SEC_SEC11_F1_MASK_MASK                                    0x00000800
#define SEC_SEC11_F1_MASK_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_SEC11_F1_MASK_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_SEC11_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_SEC10_F1_MASK_WIDTH                                            1
#define SEC_SEC10_F1_MASK_SHIFT                                            10
#define SEC_SEC10_F1_MASK_MASK                                    0x00000400
#define SEC_SEC10_F1_MASK_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_SEC10_F1_MASK_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_SEC10_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_SEC9_F1_MASK_WIDTH                                            1
#define SEC_SEC9_F1_MASK_SHIFT                                            9
#define SEC_SEC9_F1_MASK_MASK                                    0x00000200
#define SEC_SEC9_F1_MASK_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_SEC9_F1_MASK_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_SEC9_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_SEC8_F1_MASK_WIDTH                                            1
#define SEC_SEC8_F1_MASK_SHIFT                                            8
#define SEC_SEC8_F1_MASK_MASK                                    0x00000100
#define SEC_SEC8_F1_MASK_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_SEC8_F1_MASK_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_SEC8_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_SEC7_F1_MASK_WIDTH                                            1
#define SEC_SEC7_F1_MASK_SHIFT                                            7
#define SEC_SEC7_F1_MASK_MASK                                    0x00000080
#define SEC_SEC7_F1_MASK_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_SEC7_F1_MASK_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_SEC7_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_SEC6_F1_MASK_WIDTH                                            1
#define SEC_SEC6_F1_MASK_SHIFT                                            6
#define SEC_SEC6_F1_MASK_MASK                                    0x00000040
#define SEC_SEC6_F1_MASK_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_SEC6_F1_MASK_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_SEC6_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_SEC5_F1_MASK_WIDTH                                            1
#define SEC_SEC5_F1_MASK_SHIFT                                            5
#define SEC_SEC5_F1_MASK_MASK                                    0x00000020
#define SEC_SEC5_F1_MASK_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_SEC5_F1_MASK_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_SEC5_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_SEC4_F1_MASK_WIDTH                                            1
#define SEC_SEC4_F1_MASK_SHIFT                                            4
#define SEC_SEC4_F1_MASK_MASK                                    0x00000010
#define SEC_SEC4_F1_MASK_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_SEC4_F1_MASK_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_SEC4_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_SEC3_F1_MASK_WIDTH                                            1
#define SEC_SEC3_F1_MASK_SHIFT                                            3
#define SEC_SEC3_F1_MASK_MASK                                    0x00000008
#define SEC_SEC3_F1_MASK_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_SEC3_F1_MASK_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_SEC3_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_SEC2_F1_MASK_WIDTH                                            1
#define SEC_SEC2_F1_MASK_SHIFT                                            2
#define SEC_SEC2_F1_MASK_MASK                                    0x00000004
#define SEC_SEC2_F1_MASK_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_SEC2_F1_MASK_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_SEC2_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_SEC1_F1_MASK_WIDTH                                            1
#define SEC_SEC1_F1_MASK_SHIFT                                            1
#define SEC_SEC1_F1_MASK_MASK                                    0x00000002
#define SEC_SEC1_F1_MASK_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_SEC1_F1_MASK_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_SEC1_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_SEC0_F1_MASK_WIDTH                                                1
#define SEC_SEC0_F1_MASK_SHIFT                                                0
#define SEC_SEC0_F1_MASK_MASK                                        0x00000001
#define SEC_SEC0_F1_MASK_RD(src)                         (((src) & 0x00000001))
#define SEC_SEC0_F1_MASK_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_SEC0_F1_MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MSEC_ERRL	*/
/*	 Fields SEC31	 */
#define SEC_MSEC31_WIDTH                                            1
#define SEC_MSEC31_SHIFT                                            31
#define SEC_MSEC31_MASK                                    0x80000000
#define SEC_MSEC31_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_MSEC31_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_MSEC31_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30    */
#define SEC_MSEC30_WIDTH                                            1
#define SEC_MSEC30_SHIFT                                            30
#define SEC_MSEC30_MASK                                    0x40000000
#define SEC_MSEC30_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_MSEC30_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_MSEC30_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_MSEC29_WIDTH                                            1
#define SEC_MSEC29_SHIFT                                            29
#define SEC_MSEC29_MASK                                    0x20000000
#define SEC_MSEC29_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_MSEC29_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_MSEC29_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_MSEC28_WIDTH                                            1
#define SEC_MSEC28_SHIFT                                            28
#define SEC_MSEC28_MASK                                    0x10000000
#define SEC_MSEC28_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_MSEC28_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_MSEC28_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_MSEC27_WIDTH                                            1
#define SEC_MSEC27_SHIFT                                            27
#define SEC_MSEC27_MASK                                    0x08000000
#define SEC_MSEC27_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_MSEC27_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_MSEC27_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_MSEC26_WIDTH                                            1
#define SEC_MSEC26_SHIFT                                            26
#define SEC_MSEC26_MASK                                    0x04000000
#define SEC_MSEC26_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_MSEC26_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_MSEC26_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_MSEC25_WIDTH                                            1
#define SEC_MSEC25_SHIFT                                            25
#define SEC_MSEC25_MASK                                    0x02000000
#define SEC_MSEC25_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_MSEC25_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_MSEC25_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_MSEC24_WIDTH                                            1
#define SEC_MSEC24_SHIFT                                            24
#define SEC_MSEC24_MASK                                    0x01000000
#define SEC_MSEC24_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_MSEC24_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_MSEC24_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_MSEC23_WIDTH                                            1
#define SEC_MSEC23_SHIFT                                            23
#define SEC_MSEC23_MASK                                    0x00800000
#define SEC_MSEC23_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_MSEC23_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_MSEC23_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_MSEC22_WIDTH                                            1
#define SEC_MSEC22_SHIFT                                            22
#define SEC_MSEC22_MASK                                    0x00400000
#define SEC_MSEC22_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_MSEC22_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_MSEC22_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_MSEC21_WIDTH                                            1
#define SEC_MSEC21_SHIFT                                            21
#define SEC_MSEC21_MASK                                    0x00200000
#define SEC_MSEC21_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_MSEC21_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_MSEC21_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_MSEC20_WIDTH                                            1
#define SEC_MSEC20_SHIFT                                            20
#define SEC_MSEC20_MASK                                    0x00100000
#define SEC_MSEC20_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_MSEC20_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_MSEC20_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_MSEC19_WIDTH                                            1
#define SEC_MSEC19_SHIFT                                            19
#define SEC_MSEC19_MASK                                    0x00080000
#define SEC_MSEC19_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_MSEC19_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_MSEC19_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_MSEC18_WIDTH                                            1
#define SEC_MSEC18_SHIFT                                            18
#define SEC_MSEC18_MASK                                    0x00040000
#define SEC_MSEC18_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_MSEC18_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_MSEC18_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_MSEC17_WIDTH                                            1
#define SEC_MSEC17_SHIFT                                            17
#define SEC_MSEC17_MASK                                    0x00020000
#define SEC_MSEC17_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_MSEC17_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_MSEC17_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_MSEC16_WIDTH                                            1
#define SEC_MSEC16_SHIFT                                            16
#define SEC_MSEC16_MASK                                    0x00010000
#define SEC_MSEC16_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_MSEC16_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_MSEC16_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_MSEC15_WIDTH                                            1
#define SEC_MSEC15_SHIFT                                            15
#define SEC_MSEC15_MASK                                    0x00008000
#define SEC_MSEC15_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_MSEC15_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_MSEC15_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_MSEC14_WIDTH                                            1
#define SEC_MSEC14_SHIFT                                            14
#define SEC_MSEC14_MASK                                    0x00004000
#define SEC_MSEC14_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_MSEC14_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_MSEC14_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_MSEC13_WIDTH                                            1
#define SEC_MSEC13_SHIFT                                            13
#define SEC_MSEC13_MASK                                    0x00002000
#define SEC_MSEC13_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_MSEC13_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_MSEC13_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_MSEC12_WIDTH                                            1
#define SEC_MSEC12_SHIFT                                            12
#define SEC_MSEC12_MASK                                    0x00001000
#define SEC_MSEC12_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_MSEC12_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_MSEC12_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_MSEC11_WIDTH                                            1
#define SEC_MSEC11_SHIFT                                            11
#define SEC_MSEC11_MASK                                    0x00000800
#define SEC_MSEC11_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_MSEC11_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_MSEC11_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_MSEC10_WIDTH                                            1
#define SEC_MSEC10_SHIFT                                            10
#define SEC_MSEC10_MASK                                    0x00000400
#define SEC_MSEC10_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_MSEC10_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_MSEC10_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_MSEC9_WIDTH                                            1
#define SEC_MSEC9_SHIFT                                            9
#define SEC_MSEC9_MASK                                    0x00000200
#define SEC_MSEC9_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_MSEC9_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_MSEC9_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_MSEC8_WIDTH                                            1
#define SEC_MSEC8_SHIFT                                            8
#define SEC_MSEC8_MASK                                    0x00000100
#define SEC_MSEC8_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_MSEC8_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_MSEC8_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_MSEC7_WIDTH                                            1
#define SEC_MSEC7_SHIFT                                            7
#define SEC_MSEC7_MASK                                    0x00000080
#define SEC_MSEC7_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_MSEC7_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_MSEC7_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_MSEC6_WIDTH                                            1
#define SEC_MSEC6_SHIFT                                            6
#define SEC_MSEC6_MASK                                    0x00000040
#define SEC_MSEC6_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_MSEC6_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_MSEC6_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_MSEC5_WIDTH                                            1
#define SEC_MSEC5_SHIFT                                            5
#define SEC_MSEC5_MASK                                    0x00000020
#define SEC_MSEC5_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_MSEC5_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_MSEC5_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_MSEC4_WIDTH                                            1
#define SEC_MSEC4_SHIFT                                            4
#define SEC_MSEC4_MASK                                    0x00000010
#define SEC_MSEC4_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_MSEC4_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_MSEC4_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_MSEC3_WIDTH                                            1
#define SEC_MSEC3_SHIFT                                            3
#define SEC_MSEC3_MASK                                    0x00000008
#define SEC_MSEC3_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_MSEC3_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_MSEC3_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_MSEC2_WIDTH                                            1
#define SEC_MSEC2_SHIFT                                            2
#define SEC_MSEC2_MASK                                    0x00000004
#define SEC_MSEC2_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_MSEC2_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_MSEC2_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_MSEC1_WIDTH                                            1
#define SEC_MSEC1_SHIFT                                            1
#define SEC_MSEC1_MASK                                    0x00000002
#define SEC_MSEC1_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_MSEC1_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_MSEC1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_MSEC0_WIDTH                                                1
#define SEC_MSEC0_SHIFT                                                0
#define SEC_MSEC0_MASK                                        0x00000001
#define SEC_MSEC0_RD(src)                         (((src) & 0x00000001))
#define SEC_MSEC0_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_MSEC0_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MSEC_ERRLMASK	*/
/*	 Fields SEC31_MASK	 */
#define SEC_MSEC31_MASK_WIDTH                                            1
#define SEC_MSEC31_MASK_SHIFT                                            31
#define SEC_MSEC31_MASK_MASK                                    0x80000000
#define SEC_MSEC31_MASK_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_MSEC31_MASK_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_MSEC31_MASK_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30_MASK    */
#define SEC_MSEC30_MASK_WIDTH                                            1
#define SEC_MSEC30_MASK_SHIFT                                            30
#define SEC_MSEC30_MASK_MASK                                    0x40000000
#define SEC_MSEC30_MASK_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_MSEC30_MASK_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_MSEC30_MASK_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_MSEC29_MASK_WIDTH                                            1
#define SEC_MSEC29_MASK_SHIFT                                            29
#define SEC_MSEC29_MASK_MASK                                    0x20000000
#define SEC_MSEC29_MASK_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_MSEC29_MASK_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_MSEC29_MASK_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_MSEC28_MASK_WIDTH                                            1
#define SEC_MSEC28_MASK_SHIFT                                            28
#define SEC_MSEC28_MASK_MASK                                    0x10000000
#define SEC_MSEC28_MASK_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_MSEC28_MASK_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_MSEC28_MASK_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_MSEC27_MASK_WIDTH                                            1
#define SEC_MSEC27_MASK_SHIFT                                            27
#define SEC_MSEC27_MASK_MASK                                    0x08000000
#define SEC_MSEC27_MASK_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_MSEC27_MASK_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_MSEC27_MASK_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_MSEC26_MASK_WIDTH                                            1
#define SEC_MSEC26_MASK_SHIFT                                            26
#define SEC_MSEC26_MASK_MASK                                    0x04000000
#define SEC_MSEC26_MASK_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_MSEC26_MASK_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_MSEC26_MASK_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_MSEC25_MASK_WIDTH                                            1
#define SEC_MSEC25_MASK_SHIFT                                            25
#define SEC_MSEC25_MASK_MASK                                    0x02000000
#define SEC_MSEC25_MASK_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_MSEC25_MASK_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_MSEC25_MASK_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_MSEC24_MASK_WIDTH                                            1
#define SEC_MSEC24_MASK_SHIFT                                            24
#define SEC_MSEC24_MASK_MASK                                    0x01000000
#define SEC_MSEC24_MASK_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_MSEC24_MASK_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_MSEC24_MASK_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_MSEC23_MASK_WIDTH                                            1
#define SEC_MSEC23_MASK_SHIFT                                            23
#define SEC_MSEC23_MASK_MASK                                    0x00800000
#define SEC_MSEC23_MASK_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_MSEC23_MASK_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_MSEC23_MASK_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_MSEC22_MASK_WIDTH                                            1
#define SEC_MSEC22_MASK_SHIFT                                            22
#define SEC_MSEC22_MASK_MASK                                    0x00400000
#define SEC_MSEC22_MASK_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_MSEC22_MASK_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_MSEC22_MASK_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_MSEC21_MASK_WIDTH                                            1
#define SEC_MSEC21_MASK_SHIFT                                            21
#define SEC_MSEC21_MASK_MASK                                    0x00200000
#define SEC_MSEC21_MASK_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_MSEC21_MASK_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_MSEC21_MASK_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_MSEC20_MASK_WIDTH                                            1
#define SEC_MSEC20_MASK_SHIFT                                            20
#define SEC_MSEC20_MASK_MASK                                    0x00100000
#define SEC_MSEC20_MASK_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_MSEC20_MASK_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_MSEC20_MASK_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_MSEC19_MASK_WIDTH                                            1
#define SEC_MSEC19_MASK_SHIFT                                            19
#define SEC_MSEC19_MASK_MASK                                    0x00080000
#define SEC_MSEC19_MASK_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_MSEC19_MASK_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_MSEC19_MASK_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_MSEC18_MASK_MASK_WIDTH                                            1
#define SEC_MSEC18_MASK_MASK_SHIFT                                            18
#define SEC_MSEC18_MASK_MASK_MASK                                    0x00040000
#define SEC_MSEC18_MASK_MASK_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_MSEC18_MASK_MASK_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_MSEC18_MASK_MASK_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_MSEC17_MASK_WIDTH                                            1
#define SEC_MSEC17_MASK_SHIFT                                            17
#define SEC_MSEC17_MASK_MASK                                    0x00020000
#define SEC_MSEC17_MASK_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_MSEC17_MASK_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_MSEC17_MASK_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_MSEC16_MASK_WIDTH                                            1
#define SEC_MSEC16_MASK_SHIFT                                            16
#define SEC_MSEC16_MASK_MASK                                    0x00010000
#define SEC_MSEC16_MASK_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_MSEC16_MASK_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_MSEC16_MASK_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_MSEC15_MASK_WIDTH                                            1
#define SEC_MSEC15_MASK_SHIFT                                            15
#define SEC_MSEC15_MASK_MASK                                    0x00008000
#define SEC_MSEC15_MASK_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_MSEC15_MASK_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_MSEC15_MASK_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_MSEC14_MASK_WIDTH                                            1
#define SEC_MSEC14_MASK_SHIFT                                            14
#define SEC_MSEC14_MASK_MASK                                    0x00004000
#define SEC_MSEC14_MASK_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_MSEC14_MASK_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_MSEC14_MASK_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_MSEC13_MASK_WIDTH                                            1
#define SEC_MSEC13_MASK_SHIFT                                            13
#define SEC_MSEC13_MASK_MASK                                    0x00002000
#define SEC_MSEC13_MASK_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_MSEC13_MASK_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_MSEC13_MASK_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_MSEC12_MASK_WIDTH                                            1
#define SEC_MSEC12_MASK_SHIFT                                            12
#define SEC_MSEC12_MASK_MASK                                    0x00001000
#define SEC_MSEC12_MASK_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_MSEC12_MASK_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_MSEC12_MASK_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_MSEC11_MASK_WIDTH                                            1
#define SEC_MSEC11_MASK_SHIFT                                            11
#define SEC_MSEC11_MASK_MASK                                    0x00000800
#define SEC_MSEC11_MASK_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_MSEC11_MASK_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_MSEC11_MASK_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_MSEC10_MASK_WIDTH                                            1
#define SEC_MSEC10_MASK_SHIFT                                            10
#define SEC_MSEC10_MASK_MASK                                    0x00000400
#define SEC_MSEC10_MASK_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_MSEC10_MASK_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_MSEC10_MASK_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_MSEC9_MASK_WIDTH                                            1
#define SEC_MSEC9_MASK_SHIFT                                            9
#define SEC_MSEC9_MASK_MASK                                    0x00000200
#define SEC_MSEC9_MASK_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_MSEC9_MASK_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_MSEC9_MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_MSEC8_MASK_WIDTH                                            1
#define SEC_MSEC8_MASK_SHIFT                                            8
#define SEC_MSEC8_MASK_MASK                                    0x00000100
#define SEC_MSEC8_MASK_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_MSEC8_MASK_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_MSEC8_MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_MSEC7_MASK_WIDTH                                            1
#define SEC_MSEC7_MASK_SHIFT                                            7
#define SEC_MSEC7_MASK_MASK                                    0x00000080
#define SEC_MSEC7_MASK_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_MSEC7_MASK_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_MSEC7_MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_MSEC6_MASK_WIDTH                                            1
#define SEC_MSEC6_MASK_SHIFT                                            6
#define SEC_MSEC6_MASK_MASK                                    0x00000040
#define SEC_MSEC6_MASK_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_MSEC6_MASK_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_MSEC6_MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_MSEC5_MASK_WIDTH                                            1
#define SEC_MSEC5_MASK_SHIFT                                            5
#define SEC_MSEC5_MASK_MASK                                    0x00000020
#define SEC_MSEC5_MASK_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_MSEC5_MASK_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_MSEC5_MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_MSEC4_MASK_WIDTH                                            1
#define SEC_MSEC4_MASK_SHIFT                                            4
#define SEC_MSEC4_MASK_MASK                                    0x00000010
#define SEC_MSEC4_MASK_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_MSEC4_MASK_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_MSEC4_MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_MSEC3_MASK_WIDTH                                            1
#define SEC_MSEC3_MASK_SHIFT                                            3
#define SEC_MSEC3_MASK_MASK                                    0x00000008
#define SEC_MSEC3_MASK_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_MSEC3_MASK_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_MSEC3_MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_MSEC2_MASK_WIDTH                                            1
#define SEC_MSEC2_MASK_SHIFT                                            2
#define SEC_MSEC2_MASK_MASK                                    0x00000004
#define SEC_MSEC2_MASK_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_MSEC2_MASK_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_MSEC2_MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_MSEC1_MASK_WIDTH                                            1
#define SEC_MSEC1_MASK_SHIFT                                            1
#define SEC_MSEC1_MASK_MASK                                    0x00000002
#define SEC_MSEC1_MASK_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_MSEC1_MASK_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_MSEC1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_MSEC0_MASK_WIDTH                                                1
#define SEC_MSEC0_MASK_SHIFT                                                0
#define SEC_MSEC0_MASK_MASK                                        0x00000001
#define SEC_MSEC0_MASK_RD(src)                         (((src) & 0x00000001))
#define SEC_MSEC0_MASK_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_MSEC0_MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MSEC_ERRH	*/
/*	 Fields SEC31	 */
#define SEC_MSEC31_F1_WIDTH                                            1
#define SEC_MSEC31_F1_SHIFT                                            31
#define SEC_MSEC31_F1_MASK                                    0x80000000
#define SEC_MSEC31_F1_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_MSEC31_F1_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_MSEC31_F1_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30    */
#define SEC_MSEC30_F1_WIDTH                                            1
#define SEC_MSEC30_F1_SHIFT                                            30
#define SEC_MSEC30_F1_MASK                                    0x40000000
#define SEC_MSEC30_F1_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_MSEC30_F1_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_MSEC30_F1_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_MSEC29_F1_WIDTH                                            1
#define SEC_MSEC29_F1_SHIFT                                            29
#define SEC_MSEC29_F1_MASK                                    0x20000000
#define SEC_MSEC29_F1_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_MSEC29_F1_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_MSEC29_F1_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_MSEC28_F1_WIDTH                                            1
#define SEC_MSEC28_F1_SHIFT                                            28
#define SEC_MSEC28_F1_MASK                                    0x10000000
#define SEC_MSEC28_F1_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_MSEC28_F1_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_MSEC28_F1_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_MSEC27_F1_WIDTH                                            1
#define SEC_MSEC27_F1_SHIFT                                            27
#define SEC_MSEC27_F1_MASK                                    0x08000000
#define SEC_MSEC27_F1_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_MSEC27_F1_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_MSEC27_F1_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_MSEC26_F1_WIDTH                                            1
#define SEC_MSEC26_F1_SHIFT                                            26
#define SEC_MSEC26_F1_MASK                                    0x04000000
#define SEC_MSEC26_F1_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_MSEC26_F1_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_MSEC26_F1_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_MSEC25_F1_WIDTH                                            1
#define SEC_MSEC25_F1_SHIFT                                            25
#define SEC_MSEC25_F1_MASK                                    0x02000000
#define SEC_MSEC25_F1_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_MSEC25_F1_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_MSEC25_F1_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_MSEC24_F1_WIDTH                                            1
#define SEC_MSEC24_F1_SHIFT                                            24
#define SEC_MSEC24_F1_MASK                                    0x01000000
#define SEC_MSEC24_F1_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_MSEC24_F1_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_MSEC24_F1_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_MSEC23_F1_WIDTH                                            1
#define SEC_MSEC23_F1_SHIFT                                            23
#define SEC_MSEC23_F1_MASK                                    0x00800000
#define SEC_MSEC23_F1_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_MSEC23_F1_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_MSEC23_F1_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_MSEC22_F1_WIDTH                                            1
#define SEC_MSEC22_F1_SHIFT                                            22
#define SEC_MSEC22_F1_MASK                                    0x00400000
#define SEC_MSEC22_F1_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_MSEC22_F1_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_MSEC22_F1_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_MSEC21_F1_WIDTH                                            1
#define SEC_MSEC21_F1_SHIFT                                            21
#define SEC_MSEC21_F1_MASK                                    0x00200000
#define SEC_MSEC21_F1_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_MSEC21_F1_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_MSEC21_F1_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_MSEC20_F1_WIDTH                                            1
#define SEC_MSEC20_F1_SHIFT                                            20
#define SEC_MSEC20_F1_MASK                                    0x00100000
#define SEC_MSEC20_F1_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_MSEC20_F1_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_MSEC20_F1_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_MSEC19_F1_WIDTH                                            1
#define SEC_MSEC19_F1_SHIFT                                            19
#define SEC_MSEC19_F1_MASK                                    0x00080000
#define SEC_MSEC19_F1_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_MSEC19_F1_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_MSEC19_F1_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_MSEC18_F1_WIDTH                                            1
#define SEC_MSEC18_F1_SHIFT                                            18
#define SEC_MSEC18_F1_MASK                                    0x00040000
#define SEC_MSEC18_F1_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_MSEC18_F1_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_MSEC18_F1_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_MSEC17_F1_WIDTH                                            1
#define SEC_MSEC17_F1_SHIFT                                            17
#define SEC_MSEC17_F1_MASK                                    0x00020000
#define SEC_MSEC17_F1_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_MSEC17_F1_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_MSEC17_F1_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_MSEC16_F1_WIDTH                                            1
#define SEC_MSEC16_F1_SHIFT                                            16
#define SEC_MSEC16_F1_MASK                                    0x00010000
#define SEC_MSEC16_F1_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_MSEC16_F1_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_MSEC16_F1_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_MSEC15_F1_WIDTH                                            1
#define SEC_MSEC15_F1_SHIFT                                            15
#define SEC_MSEC15_F1_MASK                                    0x00008000
#define SEC_MSEC15_F1_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_MSEC15_F1_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_MSEC15_F1_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_MSEC14_F1_WIDTH                                            1
#define SEC_MSEC14_F1_SHIFT                                            14
#define SEC_MSEC14_F1_MASK                                    0x00004000
#define SEC_MSEC14_F1_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_MSEC14_F1_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_MSEC14_F1_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_MSEC13_F1_WIDTH                                            1
#define SEC_MSEC13_F1_SHIFT                                            13
#define SEC_MSEC13_F1_MASK                                    0x00002000
#define SEC_MSEC13_F1_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_MSEC13_F1_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_MSEC13_F1_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_MSEC12_F1_WIDTH                                            1
#define SEC_MSEC12_F1_SHIFT                                            12
#define SEC_MSEC12_F1_MASK                                    0x00001000
#define SEC_MSEC12_F1_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_MSEC12_F1_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_MSEC12_F1_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_MSEC11_F1_WIDTH                                            1
#define SEC_MSEC11_F1_SHIFT                                            11
#define SEC_MSEC11_F1_MASK                                    0x00000800
#define SEC_MSEC11_F1_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_MSEC11_F1_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_MSEC11_F1_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_MSEC10_F1_WIDTH                                            1
#define SEC_MSEC10_F1_SHIFT                                            10
#define SEC_MSEC10_F1_MASK                                    0x00000400
#define SEC_MSEC10_F1_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_MSEC10_F1_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_MSEC10_F1_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_MSEC9_F1_WIDTH                                            1
#define SEC_MSEC9_F1_SHIFT                                            9
#define SEC_MSEC9_F1_MASK                                    0x00000200
#define SEC_MSEC9_F1_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_MSEC9_F1_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_MSEC9_F1_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_MSEC8_F1_WIDTH                                            1
#define SEC_MSEC8_F1_SHIFT                                            8
#define SEC_MSEC8_F1_MASK                                    0x00000100
#define SEC_MSEC8_F1_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_MSEC8_F1_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_MSEC8_F1_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_MSEC7_F1_WIDTH                                            1
#define SEC_MSEC7_F1_SHIFT                                            7
#define SEC_MSEC7_F1_MASK                                    0x00000080
#define SEC_MSEC7_F1_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_MSEC7_F1_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_MSEC7_F1_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_MSEC6_F1_WIDTH                                            1
#define SEC_MSEC6_F1_SHIFT                                            6
#define SEC_MSEC6_F1_MASK                                    0x00000040
#define SEC_MSEC6_F1_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_MSEC6_F1_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_MSEC6_F1_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_MSEC5_F1_WIDTH                                            1
#define SEC_MSEC5_F1_SHIFT                                            5
#define SEC_MSEC5_F1_MASK                                    0x00000020
#define SEC_MSEC5_F1_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_MSEC5_F1_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_MSEC5_F1_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_MSEC4_F1_WIDTH                                            1
#define SEC_MSEC4_F1_SHIFT                                            4
#define SEC_MSEC4_F1_MASK                                    0x00000010
#define SEC_MSEC4_F1_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_MSEC4_F1_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_MSEC4_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_MSEC3_F1_WIDTH                                            1
#define SEC_MSEC3_F1_SHIFT                                            3
#define SEC_MSEC3_F1_MASK                                    0x00000008
#define SEC_MSEC3_F1_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_MSEC3_F1_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_MSEC3_F1_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_MSEC2_F1_WIDTH                                            1
#define SEC_MSEC2_F1_SHIFT                                            2
#define SEC_MSEC2_F1_MASK                                    0x00000004
#define SEC_MSEC2_F1_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_MSEC2_F1_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_MSEC2_F1_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_MSEC1_F1_WIDTH                                            1
#define SEC_MSEC1_F1_SHIFT                                            1
#define SEC_MSEC1_F1_MASK                                    0x00000002
#define SEC_MSEC1_F1_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_MSEC1_F1_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_MSEC1_F1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_MSEC0_F1_WIDTH                                                1
#define SEC_MSEC0_F1_SHIFT                                                0
#define SEC_MSEC0_F1_MASK                                        0x00000001
#define SEC_MSEC0_F1_RD(src)                         (((src) & 0x00000001))
#define SEC_MSEC0_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_MSEC0_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MSEC_ERRHMASK	*/
/*	 Fields SEC31_F1_MASK	 */
#define SEC_MSEC31_F1_MASK_WIDTH                                            1
#define SEC_MSEC31_F1_MASK_SHIFT                                            31
#define SEC_MSEC31_F1_MASK_MASK                                    0x80000000
#define SEC_MSEC31_F1_MASK_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_MSEC31_F1_MASK_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_MSEC31_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30_F1_MASK    */
#define SEC_MSEC30_F1_MASK_WIDTH                                            1
#define SEC_MSEC30_F1_MASK_SHIFT                                            30
#define SEC_MSEC30_F1_MASK_MASK                                    0x40000000
#define SEC_MSEC30_F1_MASK_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_MSEC30_F1_MASK_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_MSEC30_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_MSEC29_F1_MASK_WIDTH                                            1
#define SEC_MSEC29_F1_MASK_SHIFT                                            29
#define SEC_MSEC29_F1_MASK                                    0x20000000
#define SEC_MSEC29_F1_MASK_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_MSEC29_F1_MASK_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_MSEC29_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_MSEC28_F1_MASK_WIDTH                                            1
#define SEC_MSEC28_F1_MASK_SHIFT                                            28
#define SEC_MSEC28_F1_MASK_MASK                                    0x10000000
#define SEC_MSEC28_F1_MASK_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_MSEC28_F1_MASK_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_MSEC28_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_MSEC27_F1_MASK_WIDTH                                            1
#define SEC_MSEC27_F1_MASK_SHIFT                                            27
#define SEC_MSEC27_F1_MASK_MASK                                    0x08000000
#define SEC_MSEC27_F1_MASK_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_MSEC27_F1_MASK_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_MSEC27_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_MSEC26_F1_MASK_WIDTH                                            1
#define SEC_MSEC26_F1_MASK_SHIFT                                            26
#define SEC_MSEC26_F1_MASK_MASK                                    0x04000000
#define SEC_MSEC26_F1_MASK_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_MSEC26_F1_MASK_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_MSEC26_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_MSEC25_F1_MASK_WIDTH                                            1
#define SEC_MSEC25_F1_MASK_SHIFT                                            25
#define SEC_MSEC25_F1_MASK_MASK                                    0x02000000
#define SEC_MSEC25_F1_MASK_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_MSEC25_F1_MASK_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_MSEC25_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_MSEC24_F1_MASK_WIDTH                                            1
#define SEC_MSEC24_F1_MASK_SHIFT                                            24
#define SEC_MSEC24_F1_MASK_MASK                                    0x01000000
#define SEC_MSEC24_F1_MASK_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_MSEC24_F1_MASK_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_MSEC24_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_MSEC23_F1_MASK_WIDTH                                            1
#define SEC_MSEC23_F1_MASK_SHIFT                                            23
#define SEC_MSEC23_F1_MASK_MASK                                    0x00800000
#define SEC_MSEC23_F1_MASK_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_MSEC23_F1_MASK_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_MSEC23_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_MSEC22_F1_MASK_WIDTH                                            1
#define SEC_MSEC22_F1_MASK_SHIFT                                            22
#define SEC_MSEC22_F1_MASK_MASK                                    0x00400000
#define SEC_MSEC22_F1_MASK_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_MSEC22_F1_MASK_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_MSEC22_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_MSEC21_F1_MASK_WIDTH                                            1
#define SEC_MSEC21_F1_MASK_SHIFT                                            21
#define SEC_MSEC21_F1_MASK_MASK                                    0x00200000
#define SEC_MSEC21_F1_MASK_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_MSEC21_F1_MASK_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_MSEC21_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_MSEC20_F1_MASK_WIDTH                                            1
#define SEC_MSEC20_F1_MASK_SHIFT                                            20
#define SEC_MSEC20_F1_MASK_MASK                                    0x00100000
#define SEC_MSEC20_F1_MASK_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_MSEC20_F1_MASK_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_MSEC20_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_MSEC19_F1_MASK_WIDTH                                            1
#define SEC_MSEC19_F1_MASK_SHIFT                                            19
#define SEC_MSEC19_F1_MASK_MASK                                    0x00080000
#define SEC_MSEC19_F1_MASK_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_MSEC19_F1_MASK_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_MSEC19_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_MSEC18_F1_MASK_WIDTH                                            1
#define SEC_MSEC18_F1_MASK_SHIFT                                            18
#define SEC_MSEC18_F1_MASK_MASK                                    0x00040000
#define SEC_MSEC18_F1_MASK_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_MSEC18_F1_MASK_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_MSEC18_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_MSEC17_F1_MASK_WIDTH                                            1
#define SEC_MSEC17_F1_MASK_SHIFT                                            17
#define SEC_MSEC17_F1_MASK_MASK                                    0x00020000
#define SEC_MSEC17_F1_MASK_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_MSEC17_F1_MASK_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_MSEC17_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_MSEC16_F1_MASK_WIDTH                                            1
#define SEC_MSEC16_F1_MASK_SHIFT                                            16
#define SEC_MSEC16_F1_MASK_MASK                                    0x00010000
#define SEC_MSEC16_F1_MASK_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_MSEC16_F1_MASK_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_MSEC16_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_MSEC15_F1_MASK_WIDTH                                            1
#define SEC_MSEC15_F1_MASK_SHIFT                                            15
#define SEC_MSEC15_F1_MASK_MASK                                    0x00008000
#define SEC_MSEC15_F1_MASK_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_MSEC15_F1_MASK_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_MSEC15_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_MSEC14_F1_MASK_WIDTH                                            1
#define SEC_MSEC14_F1_MASK_SHIFT                                            14
#define SEC_MSEC14_F1_MASK_MASK                                    0x00004000
#define SEC_MSEC14_F1_MASK_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_MSEC14_F1_MASK_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_MSEC14_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_MSEC13_F1_MASK_WIDTH                                            1
#define SEC_MSEC13_F1_MASK_SHIFT                                            13
#define SEC_MSEC13_F1_MASK_MASK                                    0x00002000
#define SEC_MSEC13_F1_MASK_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_MSEC13_F1_MASK_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_MSEC13_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_MSEC12_F1_MASK_WIDTH                                            1
#define SEC_MSEC12_F1_MASK_SHIFT                                            12
#define SEC_MSEC12_F1_MASK_MASK                                    0x00001000
#define SEC_MSEC12_F1_MASK_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_MSEC12_F1_MASK_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_MSEC12_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_MSEC11_F1_MASK_WIDTH                                            1
#define SEC_MSEC11_F1_MASK_SHIFT                                            11
#define SEC_MSEC11_F1_MASK_MASK                                    0x00000800
#define SEC_MSEC11_F1_MASK_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_MSEC11_F1_MASK_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_MSEC11_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_MSEC10_F1_MASK_WIDTH                                            1
#define SEC_MSEC10_F1_MASK_SHIFT                                            10
#define SEC_MSEC10_F1_MASK_MASK                                    0x00000400
#define SEC_MSEC10_F1_MASK_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_MSEC10_F1_MASK_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_MSEC10_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_MSEC9_F1_MASK_WIDTH                                            1
#define SEC_MSEC9_F1_MASK_SHIFT                                            9
#define SEC_MSEC9_F1_MASK_MASK                                    0x00000200
#define SEC_MSEC9_F1_MASK_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_MSEC9_F1_MASK_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_MSEC9_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_MSEC8_F1_MASK_WIDTH                                            1
#define SEC_MSEC8_F1_MASK_SHIFT                                            8
#define SEC_MSEC8_F1_MASK_MASK                                    0x00000100
#define SEC_MSEC8_F1_MASK_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_MSEC8_F1_MASK_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_MSEC8_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_MSEC7_F1_MASK_WIDTH                                            1
#define SEC_MSEC7_F1_MASK_SHIFT                                            7
#define SEC_MSEC7_F1_MASK_MASK                                    0x00000080
#define SEC_MSEC7_F1_MASK_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_MSEC7_F1_MASK_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_MSEC7_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_MSEC6_F1_MASK_WIDTH                                            1
#define SEC_MSEC6_F1_MASK_SHIFT                                            6
#define SEC_MSEC6_F1_MASK_MASK                                    0x00000040
#define SEC_MSEC6_F1_MASK_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_MSEC6_F1_MASK_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_MSEC6_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_MSEC5_F1_MASK_WIDTH                                            1
#define SEC_MSEC5_F1_MASK_SHIFT                                            5
#define SEC_MSEC5_F1_MASK_MASK                                    0x00000020
#define SEC_MSEC5_F1_MASK_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_MSEC5_F1_MASK_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_MSEC5_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_MSEC4_F1_MASK_WIDTH                                            1
#define SEC_MSEC4_F1_MASK_SHIFT                                            4
#define SEC_MSEC4_F1_MASK_MASK                                    0x00000010
#define SEC_MSEC4_F1_MASK_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_MSEC4_F1_MASK_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_MSEC4_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_MSEC3_F1_MASK_WIDTH                                            1
#define SEC_MSEC3_F1_MASK_SHIFT                                            3
#define SEC_MSEC3_F1_MASK_MASK                                    0x00000008
#define SEC_MSEC3_F1_MASK_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_MSEC3_F1_MASK_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_MSEC3_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_MSEC2_F1_MASK_WIDTH                                            1
#define SEC_MSEC2_F1_MASK_SHIFT                                            2
#define SEC_MSEC2_F1_MASK_MASK                                    0x00000004
#define SEC_MSEC2_F1_MASK_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_MSEC2_F1_MASK_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_MSEC2_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_MSEC1_F1_MASK_WIDTH                                            1
#define SEC_MSEC1_F1_MASK_SHIFT                                            1
#define SEC_MSEC1_F1_MASK_MASK                                    0x00000002
#define SEC_MSEC1_F1_MASK_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_MSEC1_F1_MASK_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_MSEC1_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_MSEC0_F1_MASK_WIDTH                                                1
#define SEC_MSEC0_F1_MASK_SHIFT                                                0
#define SEC_MSEC0_F1_MASK_MASK                                        0x00000001
#define SEC_MSEC0_F1_MASK_RD(src)                         (((src) & 0x00000001))
#define SEC_MSEC0_F1_MASK_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_MSEC0_F1_MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_DED_ERRL	*/
/*	 Fields SEC31	 */
#define SEC_DED31_WIDTH                                            1
#define SEC_DED31_SHIFT                                            31
#define SEC_DED31_MASK                                    0x80000000
#define SEC_DED31_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_DED31_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_DED31_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30    */
#define SEC_DED30_WIDTH                                            1
#define SEC_DED30_SHIFT                                            30
#define SEC_DED30_MASK                                    0x40000000
#define SEC_DED30_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_DED30_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_DED30_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_DED29_WIDTH                                            1
#define SEC_DED29_SHIFT                                            29
#define SEC_DED29_MASK                                    0x20000000
#define SEC_DED29_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_DED29_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_DED29_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_DED28_WIDTH                                            1
#define SEC_DED28_SHIFT                                            28
#define SEC_DED28_MASK                                    0x10000000
#define SEC_DED28_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_DED28_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_DED28_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_DED27_WIDTH                                            1
#define SEC_DED27_SHIFT                                            27
#define SEC_DED27_MASK                                    0x08000000
#define SEC_DED27_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_DED27_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_DED27_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_DED26_WIDTH                                            1
#define SEC_DED26_SHIFT                                            26
#define SEC_DED26_MASK                                    0x04000000
#define SEC_DED26_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_DED26_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_DED26_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_DED25_WIDTH                                            1
#define SEC_DED25_SHIFT                                            25
#define SEC_DED25_MASK                                    0x02000000
#define SEC_DED25_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_DED25_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_DED25_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_DED24_WIDTH                                            1
#define SEC_DED24_SHIFT                                            24
#define SEC_DED24_MASK                                    0x01000000
#define SEC_DED24_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_DED24_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_DED24_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_DED23_WIDTH                                            1
#define SEC_DED23_SHIFT                                            23
#define SEC_DED23_MASK                                    0x00800000
#define SEC_DED23_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_DED23_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_DED23_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_DED22_WIDTH                                            1
#define SEC_DED22_SHIFT                                            22
#define SEC_DED22_MASK                                    0x00400000
#define SEC_DED22_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_DED22_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_DED22_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_DED21_WIDTH                                            1
#define SEC_DED21_SHIFT                                            21
#define SEC_DED21_MASK                                    0x00200000
#define SEC_DED21_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_DED21_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_DED21_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_DED20_WIDTH                                            1
#define SEC_DED20_SHIFT                                            20
#define SEC_DED20_MASK                                    0x00100000
#define SEC_DED20_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_DED20_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_DED20_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_DED19_WIDTH                                            1
#define SEC_DED19_SHIFT                                            19
#define SEC_DED19_MASK                                    0x00080000
#define SEC_DED19_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_DED19_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_DED19_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_DED18_WIDTH                                            1
#define SEC_DED18_SHIFT                                            18
#define SEC_DED18_MASK                                    0x00040000
#define SEC_DED18_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_DED18_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_DED18_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_DED17_WIDTH                                            1
#define SEC_DED17_SHIFT                                            17
#define SEC_DED17_MASK                                    0x00020000
#define SEC_DED17_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_DED17_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_DED17_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_DED16_WIDTH                                            1
#define SEC_DED16_SHIFT                                            16
#define SEC_DED16_MASK                                    0x00010000
#define SEC_DED16_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_DED16_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_DED16_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_DED15_WIDTH                                            1
#define SEC_DED15_SHIFT                                            15
#define SEC_DED15_MASK                                    0x00008000
#define SEC_DED15_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_DED15_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_DED15_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_DED14_WIDTH                                            1
#define SEC_DED14_SHIFT                                            14
#define SEC_DED14_MASK                                    0x00004000
#define SEC_DED14_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_DED14_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_DED14_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_DED13_WIDTH                                            1
#define SEC_DED13_SHIFT                                            13
#define SEC_DED13_MASK                                    0x00002000
#define SEC_DED13_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_DED13_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_DED13_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_DED12_WIDTH                                            1
#define SEC_DED12_SHIFT                                            12
#define SEC_DED12_MASK                                    0x00001000
#define SEC_DED12_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_DED12_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_DED12_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_DED11_WIDTH                                            1
#define SEC_DED11_SHIFT                                            11
#define SEC_DED11_MASK                                    0x00000800
#define SEC_DED11_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_DED11_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_DED11_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_DED10_WIDTH                                            1
#define SEC_DED10_SHIFT                                            10
#define SEC_DED10_MASK                                    0x00000400
#define SEC_DED10_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_DED10_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_DED10_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_DED9_WIDTH                                            1
#define SEC_DED9_SHIFT                                            9
#define SEC_DED9_MASK                                    0x00000200
#define SEC_DED9_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_DED9_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_DED9_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_DED8_WIDTH                                            1
#define SEC_DED8_SHIFT                                            8
#define SEC_DED8_MASK                                    0x00000100
#define SEC_DED8_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_DED8_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_DED8_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_DED7_WIDTH                                            1
#define SEC_DED7_SHIFT                                            7
#define SEC_DED7_MASK                                    0x00000080
#define SEC_DED7_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_DED7_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_DED7_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_DED6_WIDTH                                            1
#define SEC_DED6_SHIFT                                            6
#define SEC_DED6_MASK                                    0x00000040
#define SEC_DED6_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_DED6_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_DED6_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_DED5_WIDTH                                            1
#define SEC_DED5_SHIFT                                            5
#define SEC_DED5_MASK                                    0x00000020
#define SEC_DED5_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_DED5_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_DED5_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_DED4_WIDTH                                            1
#define SEC_DED4_SHIFT                                            4
#define SEC_DED4_MASK                                    0x00000010
#define SEC_DED4_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_DED4_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_DED4_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_DED3_WIDTH                                            1
#define SEC_DED3_SHIFT                                            3
#define SEC_DED3_MASK                                    0x00000008
#define SEC_DED3_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_DED3_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_DED3_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_DED2_WIDTH                                            1
#define SEC_DED2_SHIFT                                            2
#define SEC_DED2_MASK                                    0x00000004
#define SEC_DED2_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_DED2_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_DED2_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_DED1_WIDTH                                            1
#define SEC_DED1_SHIFT                                            1
#define SEC_DED1_MASK                                    0x00000002
#define SEC_DED1_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_DED1_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_DED1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_DED0_WIDTH                                                1
#define SEC_DED0_SHIFT                                                0
#define SEC_DED0_MASK                                        0x00000001
#define SEC_DED0_RD(src)                         (((src) & 0x00000001))
#define SEC_DED0_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_DED0_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_DED_ERRLMASK	*/
/*	 Fields SEC31_MASK	 */
#define SEC_DED31_MASK_WIDTH                                            1
#define SEC_DED31_MASK_SHIFT                                            31
#define SEC_DED31_MASK_MASK                                    0x80000000
#define SEC_DED31_MASK_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_DED31_MASK_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_DED31_MASK_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30_MASK    */
#define SEC_DED30_MASK_WIDTH                                            1
#define SEC_DED30_MASK_SHIFT                                            30
#define SEC_DED30_MASK_MASK                                    0x40000000
#define SEC_DED30_MASK_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_DED30_MASK_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_DED30_MASK_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_DED29_MASK_WIDTH                                            1
#define SEC_DED29_MASK_SHIFT                                            29
#define SEC_DED29_MASK_MASK                                    0x20000000
#define SEC_DED29_MASK_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_DED29_MASK_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_DED29_MASK_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_DED28_MASK_WIDTH                                            1
#define SEC_DED28_MASK_SHIFT                                            28
#define SEC_DED28_MASK_MASK                                    0x10000000
#define SEC_DED28_MASK_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_DED28_MASK_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_DED28_MASK_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_DED27_MASK_WIDTH                                            1
#define SEC_DED27_MASK_SHIFT                                            27
#define SEC_DED27_MASK_MASK                                    0x08000000
#define SEC_DED27_MASK_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_DED27_MASK_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_DED27_MASK_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_DED26_MASK_WIDTH                                            1
#define SEC_DED26_MASK_SHIFT                                            26
#define SEC_DED26_MASK_MASK                                    0x04000000
#define SEC_DED26_MASK_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_DED26_MASK_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_DED26_MASK_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_DED25_MASK_WIDTH                                            1
#define SEC_DED25_MASK_SHIFT                                            25
#define SEC_DED25_MASK_MASK                                    0x02000000
#define SEC_DED25_MASK_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_DED25_MASK_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_DED25_MASK_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_DED24_MASK_WIDTH                                            1
#define SEC_DED24_MASK_SHIFT                                            24
#define SEC_DED24_MASK_MASK                                    0x01000000
#define SEC_DED24_MASK_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_DED24_MASK_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_DED24_MASK_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_DED23_MASK_WIDTH                                            1
#define SEC_DED23_MASK_SHIFT                                            23
#define SEC_DED23_MASK_MASK                                    0x00800000
#define SEC_DED23_MASK_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_DED23_MASK_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_DED23_MASK_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_DED22_MASK_WIDTH                                            1
#define SEC_DED22_MASK_SHIFT                                            22
#define SEC_DED22_MASK_MASK                                    0x00400000
#define SEC_DED22_MASK_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_DED22_MASK_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_DED22_MASK_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_DED21_MASK_WIDTH                                            1
#define SEC_DED21_MASK_SHIFT                                            21
#define SEC_DED21_MASK_MASK                                    0x00200000
#define SEC_DED21_MASK_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_DED21_MASK_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_DED21_MASK_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_DED20_MASK_WIDTH                                            1
#define SEC_DED20_MASK_SHIFT                                            20
#define SEC_DED20_MASK_MASK                                    0x00100000
#define SEC_DED20_MASK_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_DED20_MASK_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_DED20_MASK_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_DED19_MASK_WIDTH                                            1
#define SEC_DED19_MASK_SHIFT                                            19
#define SEC_DED19_MASK_MASK                                    0x00080000
#define SEC_DED19_MASK_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_DED19_MASK_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_DED19_MASK_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_DED18_MASK_MASK_WIDTH                                            1
#define SEC_DED18_MASK_MASK_SHIFT                                            18
#define SEC_DED18_MASK_MASK_MASK                                    0x00040000
#define SEC_DED18_MASK_MASK_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_DED18_MASK_MASK_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_DED18_MASK_MASK_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_DED17_MASK_WIDTH                                            1
#define SEC_DED17_MASK_SHIFT                                            17
#define SEC_DED17_MASK_MASK                                    0x00020000
#define SEC_DED17_MASK_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_DED17_MASK_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_DED17_MASK_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_DED16_MASK_WIDTH                                            1
#define SEC_DED16_MASK_SHIFT                                            16
#define SEC_DED16_MASK_MASK                                    0x00010000
#define SEC_DED16_MASK_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_DED16_MASK_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_DED16_MASK_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_DED15_MASK_WIDTH                                            1
#define SEC_DED15_MASK_SHIFT                                            15
#define SEC_DED15_MASK_MASK                                    0x00008000
#define SEC_DED15_MASK_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_DED15_MASK_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_DED15_MASK_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_DED14_MASK_WIDTH                                            1
#define SEC_DED14_MASK_SHIFT                                            14
#define SEC_DED14_MASK_MASK                                    0x00004000
#define SEC_DED14_MASK_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_DED14_MASK_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_DED14_MASK_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_DED13_MASK_WIDTH                                            1
#define SEC_DED13_MASK_SHIFT                                            13
#define SEC_DED13_MASK_MASK                                    0x00002000
#define SEC_DED13_MASK_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_DED13_MASK_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_DED13_MASK_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_DED12_MASK_WIDTH                                            1
#define SEC_DED12_MASK_SHIFT                                            12
#define SEC_DED12_MASK_MASK                                    0x00001000
#define SEC_DED12_MASK_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_DED12_MASK_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_DED12_MASK_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_DED11_MASK_WIDTH                                            1
#define SEC_DED11_MASK_SHIFT                                            11
#define SEC_DED11_MASK_MASK                                    0x00000800
#define SEC_DED11_MASK_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_DED11_MASK_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_DED11_MASK_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_DED10_MASK_WIDTH                                            1
#define SEC_DED10_MASK_SHIFT                                            10
#define SEC_DED10_MASK_MASK                                    0x00000400
#define SEC_DED10_MASK_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_DED10_MASK_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_DED10_MASK_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_DED9_MASK_WIDTH                                            1
#define SEC_DED9_MASK_SHIFT                                            9
#define SEC_DED9_MASK_MASK                                    0x00000200
#define SEC_DED9_MASK_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_DED9_MASK_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_DED9_MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_DED8_MASK_WIDTH                                            1
#define SEC_DED8_MASK_SHIFT                                            8
#define SEC_DED8_MASK_MASK                                    0x00000100
#define SEC_DED8_MASK_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_DED8_MASK_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_DED8_MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_DED7_MASK_WIDTH                                            1
#define SEC_DED7_MASK_SHIFT                                            7
#define SEC_DED7_MASK_MASK                                    0x00000080
#define SEC_DED7_MASK_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_DED7_MASK_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_DED7_MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_DED6_MASK_WIDTH                                            1
#define SEC_DED6_MASK_SHIFT                                            6
#define SEC_DED6_MASK_MASK                                    0x00000040
#define SEC_DED6_MASK_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_DED6_MASK_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_DED6_MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_DED5_MASK_WIDTH                                            1
#define SEC_DED5_MASK_SHIFT                                            5
#define SEC_DED5_MASK_MASK                                    0x00000020
#define SEC_DED5_MASK_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_DED5_MASK_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_DED5_MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_DED4_MASK_WIDTH                                            1
#define SEC_DED4_MASK_SHIFT                                            4
#define SEC_DED4_MASK_MASK                                    0x00000010
#define SEC_DED4_MASK_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_DED4_MASK_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_DED4_MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_DED3_MASK_WIDTH                                            1
#define SEC_DED3_MASK_SHIFT                                            3
#define SEC_DED3_MASK_MASK                                    0x00000008
#define SEC_DED3_MASK_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_DED3_MASK_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_DED3_MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_DED2_MASK_WIDTH                                            1
#define SEC_DED2_MASK_SHIFT                                            2
#define SEC_DED2_MASK_MASK                                    0x00000004
#define SEC_DED2_MASK_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_DED2_MASK_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_DED2_MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_DED1_MASK_WIDTH                                            1
#define SEC_DED1_MASK_SHIFT                                            1
#define SEC_DED1_MASK_MASK                                    0x00000002
#define SEC_DED1_MASK_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_DED1_MASK_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_DED1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_DED0_MASK_WIDTH                                                1
#define SEC_DED0_MASK_SHIFT                                                0
#define SEC_DED0_MASK_MASK                                        0x00000001
#define SEC_DED0_MASK_RD(src)                         (((src) & 0x00000001))
#define SEC_DED0_MASK_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_DED0_MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_DED_ERRH	*/
/*	 Fields SEC31	 */
#define SEC_DED31_F1_WIDTH                                            1
#define SEC_DED31_F1_SHIFT                                            31
#define SEC_DED31_F1_MASK                                    0x80000000
#define SEC_DED31_F1_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_DED31_F1_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_DED31_F1_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30    */
#define SEC_DED30_F1_WIDTH                                            1
#define SEC_DED30_F1_SHIFT                                            30
#define SEC_DED30_F1_MASK                                    0x40000000
#define SEC_DED30_F1_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_DED30_F1_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_DED30_F1_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_DED29_F1_WIDTH                                            1
#define SEC_DED29_F1_SHIFT                                            29
#define SEC_DED29_F1_MASK                                    0x20000000
#define SEC_DED29_F1_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_DED29_F1_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_DED29_F1_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_DED28_F1_WIDTH                                            1
#define SEC_DED28_F1_SHIFT                                            28
#define SEC_DED28_F1_MASK                                    0x10000000
#define SEC_DED28_F1_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_DED28_F1_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_DED28_F1_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_DED27_F1_WIDTH                                            1
#define SEC_DED27_F1_SHIFT                                            27
#define SEC_DED27_F1_MASK                                    0x08000000
#define SEC_DED27_F1_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_DED27_F1_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_DED27_F1_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_DED26_F1_WIDTH                                            1
#define SEC_DED26_F1_SHIFT                                            26
#define SEC_DED26_F1_MASK                                    0x04000000
#define SEC_DED26_F1_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_DED26_F1_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_DED26_F1_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_DED25_F1_WIDTH                                            1
#define SEC_DED25_F1_SHIFT                                            25
#define SEC_DED25_F1_MASK                                    0x02000000
#define SEC_DED25_F1_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_DED25_F1_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_DED25_F1_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_DED24_F1_WIDTH                                            1
#define SEC_DED24_F1_SHIFT                                            24
#define SEC_DED24_F1_MASK                                    0x01000000
#define SEC_DED24_F1_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_DED24_F1_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_DED24_F1_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_DED23_F1_WIDTH                                            1
#define SEC_DED23_F1_SHIFT                                            23
#define SEC_DED23_F1_MASK                                    0x00800000
#define SEC_DED23_F1_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_DED23_F1_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_DED23_F1_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_DED22_F1_WIDTH                                            1
#define SEC_DED22_F1_SHIFT                                            22
#define SEC_DED22_F1_MASK                                    0x00400000
#define SEC_DED22_F1_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_DED22_F1_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_DED22_F1_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_DED21_F1_WIDTH                                            1
#define SEC_DED21_F1_SHIFT                                            21
#define SEC_DED21_F1_MASK                                    0x00200000
#define SEC_DED21_F1_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_DED21_F1_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_DED21_F1_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_DED20_F1_WIDTH                                            1
#define SEC_DED20_F1_SHIFT                                            20
#define SEC_DED20_F1_MASK                                    0x00100000
#define SEC_DED20_F1_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_DED20_F1_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_DED20_F1_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_DED19_F1_WIDTH                                            1
#define SEC_DED19_F1_SHIFT                                            19
#define SEC_DED19_F1_MASK                                    0x00080000
#define SEC_DED19_F1_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_DED19_F1_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_DED19_F1_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_DED18_F1_WIDTH                                            1
#define SEC_DED18_F1_SHIFT                                            18
#define SEC_DED18_F1_MASK                                    0x00040000
#define SEC_DED18_F1_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_DED18_F1_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_DED18_F1_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_DED17_F1_WIDTH                                            1
#define SEC_DED17_F1_SHIFT                                            17
#define SEC_DED17_F1_MASK                                    0x00020000
#define SEC_DED17_F1_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_DED17_F1_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_DED17_F1_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_DED16_F1_WIDTH                                            1
#define SEC_DED16_F1_SHIFT                                            16
#define SEC_DED16_F1_MASK                                    0x00010000
#define SEC_DED16_F1_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_DED16_F1_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_DED16_F1_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_DED15_F1_WIDTH                                            1
#define SEC_DED15_F1_SHIFT                                            15
#define SEC_DED15_F1_MASK                                    0x00008000
#define SEC_DED15_F1_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_DED15_F1_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_DED15_F1_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_DED14_F1_WIDTH                                            1
#define SEC_DED14_F1_SHIFT                                            14
#define SEC_DED14_F1_MASK                                    0x00004000
#define SEC_DED14_F1_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_DED14_F1_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_DED14_F1_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_DED13_F1_WIDTH                                            1
#define SEC_DED13_F1_SHIFT                                            13
#define SEC_DED13_F1_MASK                                    0x00002000
#define SEC_DED13_F1_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_DED13_F1_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_DED13_F1_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_DED12_F1_WIDTH                                            1
#define SEC_DED12_F1_SHIFT                                            12
#define SEC_DED12_F1_MASK                                    0x00001000
#define SEC_DED12_F1_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_DED12_F1_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_DED12_F1_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_DED11_F1_WIDTH                                            1
#define SEC_DED11_F1_SHIFT                                            11
#define SEC_DED11_F1_MASK                                    0x00000800
#define SEC_DED11_F1_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_DED11_F1_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_DED11_F1_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_DED10_F1_WIDTH                                            1
#define SEC_DED10_F1_SHIFT                                            10
#define SEC_DED10_F1_MASK                                    0x00000400
#define SEC_DED10_F1_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_DED10_F1_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_DED10_F1_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_DED9_F1_WIDTH                                            1
#define SEC_DED9_F1_SHIFT                                            9
#define SEC_DED9_F1_MASK                                    0x00000200
#define SEC_DED9_F1_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_DED9_F1_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_DED9_F1_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_DED8_F1_WIDTH                                            1
#define SEC_DED8_F1_SHIFT                                            8
#define SEC_DED8_F1_MASK                                    0x00000100
#define SEC_DED8_F1_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_DED8_F1_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_DED8_F1_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_DED7_F1_WIDTH                                            1
#define SEC_DED7_F1_SHIFT                                            7
#define SEC_DED7_F1_MASK                                    0x00000080
#define SEC_DED7_F1_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_DED7_F1_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_DED7_F1_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_DED6_F1_WIDTH                                            1
#define SEC_DED6_F1_SHIFT                                            6
#define SEC_DED6_F1_MASK                                    0x00000040
#define SEC_DED6_F1_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_DED6_F1_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_DED6_F1_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_DED5_F1_WIDTH                                            1
#define SEC_DED5_F1_SHIFT                                            5
#define SEC_DED5_F1_MASK                                    0x00000020
#define SEC_DED5_F1_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_DED5_F1_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_DED5_F1_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_DED4_F1_WIDTH                                            1
#define SEC_DED4_F1_SHIFT                                            4
#define SEC_DED4_F1_MASK                                    0x00000010
#define SEC_DED4_F1_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_DED4_F1_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_DED4_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_DED3_F1_WIDTH                                            1
#define SEC_DED3_F1_SHIFT                                            3
#define SEC_DED3_F1_MASK                                    0x00000008
#define SEC_DED3_F1_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_DED3_F1_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_DED3_F1_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_DED2_F1_WIDTH                                            1
#define SEC_DED2_F1_SHIFT                                            2
#define SEC_DED2_F1_MASK                                    0x00000004
#define SEC_DED2_F1_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_DED2_F1_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_DED2_F1_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_DED1_F1_WIDTH                                            1
#define SEC_DED1_F1_SHIFT                                            1
#define SEC_DED1_F1_MASK                                    0x00000002
#define SEC_DED1_F1_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_DED1_F1_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_DED1_F1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_DED0_F1_WIDTH                                                1
#define SEC_DED0_F1_SHIFT                                                0
#define SEC_DED0_F1_MASK                                        0x00000001
#define SEC_DED0_F1_RD(src)                         (((src) & 0x00000001))
#define SEC_DED0_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_DED0_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_DED_ERRHMASK	*/
/*	 Fields SEC31_F1_MASK	 */
#define SEC_DED31_F1_MASK_WIDTH                                            1
#define SEC_DED31_F1_MASK_SHIFT                                            31
#define SEC_DED31_F1_MASK_MASK                                    0x80000000
#define SEC_DED31_F1_MASK_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_DED31_F1_MASK_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_DED31_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30_F1_MASK    */
#define SEC_DED30_F1_MASK_WIDTH                                            1
#define SEC_DED30_F1_MASK_SHIFT                                            30
#define SEC_DED30_F1_MASK_MASK                                    0x40000000
#define SEC_DED30_F1_MASK_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_DED30_F1_MASK_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_DED30_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_DED29_F1_MASK_WIDTH                                            1
#define SEC_DED29_F1_MASK_SHIFT                                            29
#define SEC_DED29_F1_MASK                                    0x20000000
#define SEC_DED29_F1_MASK_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_DED29_F1_MASK_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_DED29_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_DED28_F1_MASK_WIDTH                                            1
#define SEC_DED28_F1_MASK_SHIFT                                            28
#define SEC_DED28_F1_MASK_MASK                                    0x10000000
#define SEC_DED28_F1_MASK_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_DED28_F1_MASK_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_DED28_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_DED27_F1_MASK_WIDTH                                            1
#define SEC_DED27_F1_MASK_SHIFT                                            27
#define SEC_DED27_F1_MASK_MASK                                    0x08000000
#define SEC_DED27_F1_MASK_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_DED27_F1_MASK_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_DED27_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_DED26_F1_MASK_WIDTH                                            1
#define SEC_DED26_F1_MASK_SHIFT                                            26
#define SEC_DED26_F1_MASK_MASK                                    0x04000000
#define SEC_DED26_F1_MASK_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_DED26_F1_MASK_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_DED26_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_DED25_F1_MASK_WIDTH                                            1
#define SEC_DED25_F1_MASK_SHIFT                                            25
#define SEC_DED25_F1_MASK_MASK                                    0x02000000
#define SEC_DED25_F1_MASK_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_DED25_F1_MASK_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_DED25_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_DED24_F1_MASK_WIDTH                                            1
#define SEC_DED24_F1_MASK_SHIFT                                            24
#define SEC_DED24_F1_MASK_MASK                                    0x01000000
#define SEC_DED24_F1_MASK_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_DED24_F1_MASK_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_DED24_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_DED23_F1_MASK_WIDTH                                            1
#define SEC_DED23_F1_MASK_SHIFT                                            23
#define SEC_DED23_F1_MASK_MASK                                    0x00800000
#define SEC_DED23_F1_MASK_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_DED23_F1_MASK_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_DED23_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_DED22_F1_MASK_WIDTH                                            1
#define SEC_DED22_F1_MASK_SHIFT                                            22
#define SEC_DED22_F1_MASK_MASK                                    0x00400000
#define SEC_DED22_F1_MASK_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_DED22_F1_MASK_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_DED22_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_DED21_F1_MASK_WIDTH                                            1
#define SEC_DED21_F1_MASK_SHIFT                                            21
#define SEC_DED21_F1_MASK_MASK                                    0x00200000
#define SEC_DED21_F1_MASK_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_DED21_F1_MASK_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_DED21_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_DED20_F1_MASK_WIDTH                                            1
#define SEC_DED20_F1_MASK_SHIFT                                            20
#define SEC_DED20_F1_MASK_MASK                                    0x00100000
#define SEC_DED20_F1_MASK_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_DED20_F1_MASK_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_DED20_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_DED19_F1_MASK_WIDTH                                            1
#define SEC_DED19_F1_MASK_SHIFT                                            19
#define SEC_DED19_F1_MASK_MASK                                    0x00080000
#define SEC_DED19_F1_MASK_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_DED19_F1_MASK_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_DED19_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_DED18_F1_MASK_WIDTH                                            1
#define SEC_DED18_F1_MASK_SHIFT                                            18
#define SEC_DED18_F1_MASK_MASK                                    0x00040000
#define SEC_DED18_F1_MASK_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_DED18_F1_MASK_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_DED18_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_DED17_F1_MASK_WIDTH                                            1
#define SEC_DED17_F1_MASK_SHIFT                                            17
#define SEC_DED17_F1_MASK_MASK                                    0x00020000
#define SEC_DED17_F1_MASK_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_DED17_F1_MASK_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_DED17_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_DED16_F1_MASK_WIDTH                                            1
#define SEC_DED16_F1_MASK_SHIFT                                            16
#define SEC_DED16_F1_MASK_MASK                                    0x00010000
#define SEC_DED16_F1_MASK_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_DED16_F1_MASK_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_DED16_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_DED15_F1_MASK_WIDTH                                            1
#define SEC_DED15_F1_MASK_SHIFT                                            15
#define SEC_DED15_F1_MASK_MASK                                    0x00008000
#define SEC_DED15_F1_MASK_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_DED15_F1_MASK_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_DED15_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_DED14_F1_MASK_WIDTH                                            1
#define SEC_DED14_F1_MASK_SHIFT                                            14
#define SEC_DED14_F1_MASK_MASK                                    0x00004000
#define SEC_DED14_F1_MASK_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_DED14_F1_MASK_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_DED14_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_DED13_F1_MASK_WIDTH                                            1
#define SEC_DED13_F1_MASK_SHIFT                                            13
#define SEC_DED13_F1_MASK_MASK                                    0x00002000
#define SEC_DED13_F1_MASK_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_DED13_F1_MASK_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_DED13_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_DED12_F1_MASK_WIDTH                                            1
#define SEC_DED12_F1_MASK_SHIFT                                            12
#define SEC_DED12_F1_MASK_MASK                                    0x00001000
#define SEC_DED12_F1_MASK_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_DED12_F1_MASK_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_DED12_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_DED11_F1_MASK_WIDTH                                            1
#define SEC_DED11_F1_MASK_SHIFT                                            11
#define SEC_DED11_F1_MASK_MASK                                    0x00000800
#define SEC_DED11_F1_MASK_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_DED11_F1_MASK_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_DED11_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_DED10_F1_MASK_WIDTH                                            1
#define SEC_DED10_F1_MASK_SHIFT                                            10
#define SEC_DED10_F1_MASK_MASK                                    0x00000400
#define SEC_DED10_F1_MASK_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_DED10_F1_MASK_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_DED10_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_DED9_F1_MASK_WIDTH                                            1
#define SEC_DED9_F1_MASK_SHIFT                                            9
#define SEC_DED9_F1_MASK_MASK                                    0x00000200
#define SEC_DED9_F1_MASK_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_DED9_F1_MASK_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_DED9_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_DED8_F1_MASK_WIDTH                                            1
#define SEC_DED8_F1_MASK_SHIFT                                            8
#define SEC_DED8_F1_MASK_MASK                                    0x00000100
#define SEC_DED8_F1_MASK_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_DED8_F1_MASK_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_DED8_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_DED7_F1_MASK_WIDTH                                            1
#define SEC_DED7_F1_MASK_SHIFT                                            7
#define SEC_DED7_F1_MASK_MASK                                    0x00000080
#define SEC_DED7_F1_MASK_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_DED7_F1_MASK_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_DED7_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_DED6_F1_MASK_WIDTH                                            1
#define SEC_DED6_F1_MASK_SHIFT                                            6
#define SEC_DED6_F1_MASK_MASK                                    0x00000040
#define SEC_DED6_F1_MASK_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_DED6_F1_MASK_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_DED6_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_DED5_F1_MASK_WIDTH                                            1
#define SEC_DED5_F1_MASK_SHIFT                                            5
#define SEC_DED5_F1_MASK_MASK                                    0x00000020
#define SEC_DED5_F1_MASK_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_DED5_F1_MASK_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_DED5_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_DED4_F1_MASK_WIDTH                                            1
#define SEC_DED4_F1_MASK_SHIFT                                            4
#define SEC_DED4_F1_MASK_MASK                                    0x00000010
#define SEC_DED4_F1_MASK_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_DED4_F1_MASK_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_DED4_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_DED3_F1_MASK_WIDTH                                            1
#define SEC_DED3_F1_MASK_SHIFT                                            3
#define SEC_DED3_F1_MASK_MASK                                    0x00000008
#define SEC_DED3_F1_MASK_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_DED3_F1_MASK_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_DED3_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_DED2_F1_MASK_WIDTH                                            1
#define SEC_DED2_F1_MASK_SHIFT                                            2
#define SEC_DED2_F1_MASK_MASK                                    0x00000004
#define SEC_DED2_F1_MASK_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_DED2_F1_MASK_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_DED2_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_DED1_F1_MASK_WIDTH                                            1
#define SEC_DED1_F1_MASK_SHIFT                                            1
#define SEC_DED1_F1_MASK_MASK                                    0x00000002
#define SEC_DED1_F1_MASK_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_DED1_F1_MASK_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_DED1_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_DED0_F1_MASK_WIDTH                                                1
#define SEC_DED0_F1_MASK_SHIFT                                                0
#define SEC_DED0_F1_MASK_MASK                                        0x00000001
#define SEC_DED0_F1_MASK_RD(src)                         (((src) & 0x00000001))
#define SEC_DED0_F1_MASK_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_DED0_F1_MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MDED_ERRL	*/
/*	 Fields SEC31	 */
#define SEC_MDED31_WIDTH                                            1
#define SEC_MDED31_SHIFT                                            31
#define SEC_MDED31_MASK                                    0x80000000
#define SEC_MDED31_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_MDED31_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_MDED31_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30    */
#define SEC_MDED30_WIDTH                                            1
#define SEC_MDED30_SHIFT                                            30
#define SEC_MDED30_MASK                                    0x40000000
#define SEC_MDED30_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_MDED30_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_MDED30_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_MDED29_WIDTH                                            1
#define SEC_MDED29_SHIFT                                            29
#define SEC_MDED29_MASK                                    0x20000000
#define SEC_MDED29_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_MDED29_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_MDED29_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_MDED28_WIDTH                                            1
#define SEC_MDED28_SHIFT                                            28
#define SEC_MDED28_MASK                                    0x10000000
#define SEC_MDED28_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_MDED28_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_MDED28_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_MDED27_WIDTH                                            1
#define SEC_MDED27_SHIFT                                            27
#define SEC_MDED27_MASK                                    0x08000000
#define SEC_MDED27_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_MDED27_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_MDED27_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_MDED26_WIDTH                                            1
#define SEC_MDED26_SHIFT                                            26
#define SEC_MDED26_MASK                                    0x04000000
#define SEC_MDED26_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_MDED26_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_MDED26_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_MDED25_WIDTH                                            1
#define SEC_MDED25_SHIFT                                            25
#define SEC_MDED25_MASK                                    0x02000000
#define SEC_MDED25_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_MDED25_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_MDED25_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_MDED24_WIDTH                                            1
#define SEC_MDED24_SHIFT                                            24
#define SEC_MDED24_MASK                                    0x01000000
#define SEC_MDED24_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_MDED24_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_MDED24_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_MDED23_WIDTH                                            1
#define SEC_MDED23_SHIFT                                            23
#define SEC_MDED23_MASK                                    0x00800000
#define SEC_MDED23_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_MDED23_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_MDED23_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_MDED22_WIDTH                                            1
#define SEC_MDED22_SHIFT                                            22
#define SEC_MDED22_MASK                                    0x00400000
#define SEC_MDED22_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_MDED22_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_MDED22_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_MDED21_WIDTH                                            1
#define SEC_MDED21_SHIFT                                            21
#define SEC_MDED21_MASK                                    0x00200000
#define SEC_MDED21_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_MDED21_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_MDED21_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_MDED20_WIDTH                                            1
#define SEC_MDED20_SHIFT                                            20
#define SEC_MDED20_MASK                                    0x00100000
#define SEC_MDED20_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_MDED20_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_MDED20_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_MDED19_WIDTH                                            1
#define SEC_MDED19_SHIFT                                            19
#define SEC_MDED19_MASK                                    0x00080000
#define SEC_MDED19_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_MDED19_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_MDED19_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_MDED18_WIDTH                                            1
#define SEC_MDED18_SHIFT                                            18
#define SEC_MDED18_MASK                                    0x00040000
#define SEC_MDED18_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_MDED18_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_MDED18_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_MDED17_WIDTH                                            1
#define SEC_MDED17_SHIFT                                            17
#define SEC_MDED17_MASK                                    0x00020000
#define SEC_MDED17_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_MDED17_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_MDED17_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_MDED16_WIDTH                                            1
#define SEC_MDED16_SHIFT                                            16
#define SEC_MDED16_MASK                                    0x00010000
#define SEC_MDED16_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_MDED16_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_MDED16_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_MDED15_WIDTH                                            1
#define SEC_MDED15_SHIFT                                            15
#define SEC_MDED15_MASK                                    0x00008000
#define SEC_MDED15_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_MDED15_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_MDED15_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_MDED14_WIDTH                                            1
#define SEC_MDED14_SHIFT                                            14
#define SEC_MDED14_MASK                                    0x00004000
#define SEC_MDED14_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_MDED14_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_MDED14_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_MDED13_WIDTH                                            1
#define SEC_MDED13_SHIFT                                            13
#define SEC_MDED13_MASK                                    0x00002000
#define SEC_MDED13_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_MDED13_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_MDED13_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_MDED12_WIDTH                                            1
#define SEC_MDED12_SHIFT                                            12
#define SEC_MDED12_MASK                                    0x00001000
#define SEC_MDED12_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_MDED12_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_MDED12_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_MDED11_WIDTH                                            1
#define SEC_MDED11_SHIFT                                            11
#define SEC_MDED11_MASK                                    0x00000800
#define SEC_MDED11_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_MDED11_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_MDED11_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_MDED10_WIDTH                                            1
#define SEC_MDED10_SHIFT                                            10
#define SEC_MDED10_MASK                                    0x00000400
#define SEC_MDED10_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_MDED10_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_MDED10_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_MDED9_WIDTH                                            1
#define SEC_MDED9_SHIFT                                            9
#define SEC_MDED9_MASK                                    0x00000200
#define SEC_MDED9_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_MDED9_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_MDED9_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_MDED8_WIDTH                                            1
#define SEC_MDED8_SHIFT                                            8
#define SEC_MDED8_MASK                                    0x00000100
#define SEC_MDED8_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_MDED8_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_MDED8_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_MDED7_WIDTH                                            1
#define SEC_MDED7_SHIFT                                            7
#define SEC_MDED7_MASK                                    0x00000080
#define SEC_MDED7_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_MDED7_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_MDED7_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_MDED6_WIDTH                                            1
#define SEC_MDED6_SHIFT                                            6
#define SEC_MDED6_MASK                                    0x00000040
#define SEC_MDED6_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_MDED6_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_MDED6_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_MDED5_WIDTH                                            1
#define SEC_MDED5_SHIFT                                            5
#define SEC_MDED5_MASK                                    0x00000020
#define SEC_MDED5_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_MDED5_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_MDED5_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_MDED4_WIDTH                                            1
#define SEC_MDED4_SHIFT                                            4
#define SEC_MDED4_MASK                                    0x00000010
#define SEC_MDED4_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_MDED4_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_MDED4_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_MDED3_WIDTH                                            1
#define SEC_MDED3_SHIFT                                            3
#define SEC_MDED3_MASK                                    0x00000008
#define SEC_MDED3_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_MDED3_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_MDED3_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_MDED2_WIDTH                                            1
#define SEC_MDED2_SHIFT                                            2
#define SEC_MDED2_MASK                                    0x00000004
#define SEC_MDED2_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_MDED2_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_MDED2_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_MDED1_WIDTH                                            1
#define SEC_MDED1_SHIFT                                            1
#define SEC_MDED1_MASK                                    0x00000002
#define SEC_MDED1_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_MDED1_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_MDED1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_MDED0_WIDTH                                                1
#define SEC_MDED0_SHIFT                                                0
#define SEC_MDED0_MASK                                        0x00000001
#define SEC_MDED0_RD(src)                         (((src) & 0x00000001))
#define SEC_MDED0_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_MDED0_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MDED_ERRLMASK	*/
/*	 Fields SEC31_MASK	 */
#define SEC_MDED31_MASK_WIDTH                                            1
#define SEC_MDED31_MASK_SHIFT                                            31
#define SEC_MDED31_MASK_MASK                                    0x80000000
#define SEC_MDED31_MASK_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_MDED31_MASK_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_MDED31_MASK_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30_MASK    */
#define SEC_MDED30_MASK_WIDTH                                            1
#define SEC_MDED30_MASK_SHIFT                                            30
#define SEC_MDED30_MASK_MASK                                    0x40000000
#define SEC_MDED30_MASK_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_MDED30_MASK_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_MDED30_MASK_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_MDED29_MASK_WIDTH                                            1
#define SEC_MDED29_MASK_SHIFT                                            29
#define SEC_MDED29_MASK_MASK                                    0x20000000
#define SEC_MDED29_MASK_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_MDED29_MASK_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_MDED29_MASK_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_MDED28_MASK_WIDTH                                            1
#define SEC_MDED28_MASK_SHIFT                                            28
#define SEC_MDED28_MASK_MASK                                    0x10000000
#define SEC_MDED28_MASK_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_MDED28_MASK_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_MDED28_MASK_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_MDED27_MASK_WIDTH                                            1
#define SEC_MDED27_MASK_SHIFT                                            27
#define SEC_MDED27_MASK_MASK                                    0x08000000
#define SEC_MDED27_MASK_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_MDED27_MASK_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_MDED27_MASK_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_MDED26_MASK_WIDTH                                            1
#define SEC_MDED26_MASK_SHIFT                                            26
#define SEC_MDED26_MASK_MASK                                    0x04000000
#define SEC_MDED26_MASK_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_MDED26_MASK_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_MDED26_MASK_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_MDED25_MASK_WIDTH                                            1
#define SEC_MDED25_MASK_SHIFT                                            25
#define SEC_MDED25_MASK_MASK                                    0x02000000
#define SEC_MDED25_MASK_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_MDED25_MASK_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_MDED25_MASK_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_MDED24_MASK_WIDTH                                            1
#define SEC_MDED24_MASK_SHIFT                                            24
#define SEC_MDED24_MASK_MASK                                    0x01000000
#define SEC_MDED24_MASK_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_MDED24_MASK_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_MDED24_MASK_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_MDED23_MASK_WIDTH                                            1
#define SEC_MDED23_MASK_SHIFT                                            23
#define SEC_MDED23_MASK_MASK                                    0x00800000
#define SEC_MDED23_MASK_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_MDED23_MASK_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_MDED23_MASK_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_MDED22_MASK_WIDTH                                            1
#define SEC_MDED22_MASK_SHIFT                                            22
#define SEC_MDED22_MASK_MASK                                    0x00400000
#define SEC_MDED22_MASK_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_MDED22_MASK_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_MDED22_MASK_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_MDED21_MASK_WIDTH                                            1
#define SEC_MDED21_MASK_SHIFT                                            21
#define SEC_MDED21_MASK_MASK                                    0x00200000
#define SEC_MDED21_MASK_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_MDED21_MASK_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_MDED21_MASK_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_MDED20_MASK_WIDTH                                            1
#define SEC_MDED20_MASK_SHIFT                                            20
#define SEC_MDED20_MASK_MASK                                    0x00100000
#define SEC_MDED20_MASK_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_MDED20_MASK_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_MDED20_MASK_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_MDED19_MASK_WIDTH                                            1
#define SEC_MDED19_MASK_SHIFT                                            19
#define SEC_MDED19_MASK_MASK                                    0x00080000
#define SEC_MDED19_MASK_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_MDED19_MASK_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_MDED19_MASK_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_MDED18_MASK_MASK_WIDTH                                            1
#define SEC_MDED18_MASK_MASK_SHIFT                                            18
#define SEC_MDED18_MASK_MASK_MASK                                    0x00040000
#define SEC_MDED18_MASK_MASK_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_MDED18_MASK_MASK_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_MDED18_MASK_MASK_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_MDED17_MASK_WIDTH                                            1
#define SEC_MDED17_MASK_SHIFT                                            17
#define SEC_MDED17_MASK_MASK                                    0x00020000
#define SEC_MDED17_MASK_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_MDED17_MASK_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_MDED17_MASK_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_MDED16_MASK_WIDTH                                            1
#define SEC_MDED16_MASK_SHIFT                                            16
#define SEC_MDED16_MASK_MASK                                    0x00010000
#define SEC_MDED16_MASK_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_MDED16_MASK_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_MDED16_MASK_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_MDED15_MASK_WIDTH                                            1
#define SEC_MDED15_MASK_SHIFT                                            15
#define SEC_MDED15_MASK_MASK                                    0x00008000
#define SEC_MDED15_MASK_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_MDED15_MASK_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_MDED15_MASK_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_MDED14_MASK_WIDTH                                            1
#define SEC_MDED14_MASK_SHIFT                                            14
#define SEC_MDED14_MASK_MASK                                    0x00004000
#define SEC_MDED14_MASK_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_MDED14_MASK_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_MDED14_MASK_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_MDED13_MASK_WIDTH                                            1
#define SEC_MDED13_MASK_SHIFT                                            13
#define SEC_MDED13_MASK_MASK                                    0x00002000
#define SEC_MDED13_MASK_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_MDED13_MASK_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_MDED13_MASK_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_MDED12_MASK_WIDTH                                            1
#define SEC_MDED12_MASK_SHIFT                                            12
#define SEC_MDED12_MASK_MASK                                    0x00001000
#define SEC_MDED12_MASK_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_MDED12_MASK_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_MDED12_MASK_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_MDED11_MASK_WIDTH                                            1
#define SEC_MDED11_MASK_SHIFT                                            11
#define SEC_MDED11_MASK_MASK                                    0x00000800
#define SEC_MDED11_MASK_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_MDED11_MASK_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_MDED11_MASK_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_MDED10_MASK_WIDTH                                            1
#define SEC_MDED10_MASK_SHIFT                                            10
#define SEC_MDED10_MASK_MASK                                    0x00000400
#define SEC_MDED10_MASK_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_MDED10_MASK_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_MDED10_MASK_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_MDED9_MASK_WIDTH                                            1
#define SEC_MDED9_MASK_SHIFT                                            9
#define SEC_MDED9_MASK_MASK                                    0x00000200
#define SEC_MDED9_MASK_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_MDED9_MASK_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_MDED9_MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_MDED8_MASK_WIDTH                                            1
#define SEC_MDED8_MASK_SHIFT                                            8
#define SEC_MDED8_MASK_MASK                                    0x00000100
#define SEC_MDED8_MASK_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_MDED8_MASK_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_MDED8_MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_MDED7_MASK_WIDTH                                            1
#define SEC_MDED7_MASK_SHIFT                                            7
#define SEC_MDED7_MASK_MASK                                    0x00000080
#define SEC_MDED7_MASK_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_MDED7_MASK_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_MDED7_MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_MDED6_MASK_WIDTH                                            1
#define SEC_MDED6_MASK_SHIFT                                            6
#define SEC_MDED6_MASK_MASK                                    0x00000040
#define SEC_MDED6_MASK_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_MDED6_MASK_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_MDED6_MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_MDED5_MASK_WIDTH                                            1
#define SEC_MDED5_MASK_SHIFT                                            5
#define SEC_MDED5_MASK_MASK                                    0x00000020
#define SEC_MDED5_MASK_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_MDED5_MASK_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_MDED5_MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_MDED4_MASK_WIDTH                                            1
#define SEC_MDED4_MASK_SHIFT                                            4
#define SEC_MDED4_MASK_MASK                                    0x00000010
#define SEC_MDED4_MASK_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_MDED4_MASK_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_MDED4_MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_MDED3_MASK_WIDTH                                            1
#define SEC_MDED3_MASK_SHIFT                                            3
#define SEC_MDED3_MASK_MASK                                    0x00000008
#define SEC_MDED3_MASK_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_MDED3_MASK_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_MDED3_MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_MDED2_MASK_WIDTH                                            1
#define SEC_MDED2_MASK_SHIFT                                            2
#define SEC_MDED2_MASK_MASK                                    0x00000004
#define SEC_MDED2_MASK_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_MDED2_MASK_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_MDED2_MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_MDED1_MASK_WIDTH                                            1
#define SEC_MDED1_MASK_SHIFT                                            1
#define SEC_MDED1_MASK_MASK                                    0x00000002
#define SEC_MDED1_MASK_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_MDED1_MASK_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_MDED1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_MDED0_MASK_WIDTH                                                1
#define SEC_MDED0_MASK_SHIFT                                                0
#define SEC_MDED0_MASK_MASK                                        0x00000001
#define SEC_MDED0_MASK_RD(src)                         (((src) & 0x00000001))
#define SEC_MDED0_MASK_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_MDED0_MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MDED_ERRH	*/
/*	 Fields SEC31	 */
#define SEC_MDED31_F1_WIDTH                                            1
#define SEC_MDED31_F1_SHIFT                                            31
#define SEC_MDED31_F1_MASK                                    0x80000000
#define SEC_MDED31_F1_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_MDED31_F1_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_MDED31_F1_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30    */
#define SEC_MDED30_F1_WIDTH                                            1
#define SEC_MDED30_F1_SHIFT                                            30
#define SEC_MDED30_F1_MASK                                    0x40000000
#define SEC_MDED30_F1_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_MDED30_F1_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_MDED30_F1_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_MDED29_F1_WIDTH                                            1
#define SEC_MDED29_F1_SHIFT                                            29
#define SEC_MDED29_F1_MASK                                    0x20000000
#define SEC_MDED29_F1_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_MDED29_F1_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_MDED29_F1_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_MDED28_F1_WIDTH                                            1
#define SEC_MDED28_F1_SHIFT                                            28
#define SEC_MDED28_F1_MASK                                    0x10000000
#define SEC_MDED28_F1_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_MDED28_F1_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_MDED28_F1_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_MDED27_F1_WIDTH                                            1
#define SEC_MDED27_F1_SHIFT                                            27
#define SEC_MDED27_F1_MASK                                    0x08000000
#define SEC_MDED27_F1_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_MDED27_F1_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_MDED27_F1_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_MDED26_F1_WIDTH                                            1
#define SEC_MDED26_F1_SHIFT                                            26
#define SEC_MDED26_F1_MASK                                    0x04000000
#define SEC_MDED26_F1_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_MDED26_F1_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_MDED26_F1_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_MDED25_F1_WIDTH                                            1
#define SEC_MDED25_F1_SHIFT                                            25
#define SEC_MDED25_F1_MASK                                    0x02000000
#define SEC_MDED25_F1_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_MDED25_F1_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_MDED25_F1_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_MDED24_F1_WIDTH                                            1
#define SEC_MDED24_F1_SHIFT                                            24
#define SEC_MDED24_F1_MASK                                    0x01000000
#define SEC_MDED24_F1_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_MDED24_F1_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_MDED24_F1_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_MDED23_F1_WIDTH                                            1
#define SEC_MDED23_F1_SHIFT                                            23
#define SEC_MDED23_F1_MASK                                    0x00800000
#define SEC_MDED23_F1_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_MDED23_F1_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_MDED23_F1_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_MDED22_F1_WIDTH                                            1
#define SEC_MDED22_F1_SHIFT                                            22
#define SEC_MDED22_F1_MASK                                    0x00400000
#define SEC_MDED22_F1_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_MDED22_F1_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_MDED22_F1_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_MDED21_F1_WIDTH                                            1
#define SEC_MDED21_F1_SHIFT                                            21
#define SEC_MDED21_F1_MASK                                    0x00200000
#define SEC_MDED21_F1_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_MDED21_F1_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_MDED21_F1_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_MDED20_F1_WIDTH                                            1
#define SEC_MDED20_F1_SHIFT                                            20
#define SEC_MDED20_F1_MASK                                    0x00100000
#define SEC_MDED20_F1_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_MDED20_F1_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_MDED20_F1_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_MDED19_F1_WIDTH                                            1
#define SEC_MDED19_F1_SHIFT                                            19
#define SEC_MDED19_F1_MASK                                    0x00080000
#define SEC_MDED19_F1_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_MDED19_F1_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_MDED19_F1_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_MDED18_F1_WIDTH                                            1
#define SEC_MDED18_F1_SHIFT                                            18
#define SEC_MDED18_F1_MASK                                    0x00040000
#define SEC_MDED18_F1_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_MDED18_F1_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_MDED18_F1_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_MDED17_F1_WIDTH                                            1
#define SEC_MDED17_F1_SHIFT                                            17
#define SEC_MDED17_F1_MASK                                    0x00020000
#define SEC_MDED17_F1_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_MDED17_F1_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_MDED17_F1_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_MDED16_F1_WIDTH                                            1
#define SEC_MDED16_F1_SHIFT                                            16
#define SEC_MDED16_F1_MASK                                    0x00010000
#define SEC_MDED16_F1_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_MDED16_F1_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_MDED16_F1_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_MDED15_F1_WIDTH                                            1
#define SEC_MDED15_F1_SHIFT                                            15
#define SEC_MDED15_F1_MASK                                    0x00008000
#define SEC_MDED15_F1_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_MDED15_F1_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_MDED15_F1_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_MDED14_F1_WIDTH                                            1
#define SEC_MDED14_F1_SHIFT                                            14
#define SEC_MDED14_F1_MASK                                    0x00004000
#define SEC_MDED14_F1_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_MDED14_F1_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_MDED14_F1_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_MDED13_F1_WIDTH                                            1
#define SEC_MDED13_F1_SHIFT                                            13
#define SEC_MDED13_F1_MASK                                    0x00002000
#define SEC_MDED13_F1_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_MDED13_F1_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_MDED13_F1_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_MDED12_F1_WIDTH                                            1
#define SEC_MDED12_F1_SHIFT                                            12
#define SEC_MDED12_F1_MASK                                    0x00001000
#define SEC_MDED12_F1_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_MDED12_F1_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_MDED12_F1_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_MDED11_F1_WIDTH                                            1
#define SEC_MDED11_F1_SHIFT                                            11
#define SEC_MDED11_F1_MASK                                    0x00000800
#define SEC_MDED11_F1_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_MDED11_F1_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_MDED11_F1_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_MDED10_F1_WIDTH                                            1
#define SEC_MDED10_F1_SHIFT                                            10
#define SEC_MDED10_F1_MASK                                    0x00000400
#define SEC_MDED10_F1_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_MDED10_F1_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_MDED10_F1_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_MDED9_F1_WIDTH                                            1
#define SEC_MDED9_F1_SHIFT                                            9
#define SEC_MDED9_F1_MASK                                    0x00000200
#define SEC_MDED9_F1_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_MDED9_F1_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_MDED9_F1_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_MDED8_F1_WIDTH                                            1
#define SEC_MDED8_F1_SHIFT                                            8
#define SEC_MDED8_F1_MASK                                    0x00000100
#define SEC_MDED8_F1_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_MDED8_F1_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_MDED8_F1_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_MDED7_F1_WIDTH                                            1
#define SEC_MDED7_F1_SHIFT                                            7
#define SEC_MDED7_F1_MASK                                    0x00000080
#define SEC_MDED7_F1_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_MDED7_F1_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_MDED7_F1_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_MDED6_F1_WIDTH                                            1
#define SEC_MDED6_F1_SHIFT                                            6
#define SEC_MDED6_F1_MASK                                    0x00000040
#define SEC_MDED6_F1_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_MDED6_F1_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_MDED6_F1_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_MDED5_F1_WIDTH                                            1
#define SEC_MDED5_F1_SHIFT                                            5
#define SEC_MDED5_F1_MASK                                    0x00000020
#define SEC_MDED5_F1_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_MDED5_F1_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_MDED5_F1_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_MDED4_F1_WIDTH                                            1
#define SEC_MDED4_F1_SHIFT                                            4
#define SEC_MDED4_F1_MASK                                    0x00000010
#define SEC_MDED4_F1_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_MDED4_F1_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_MDED4_F1_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_MDED3_F1_WIDTH                                            1
#define SEC_MDED3_F1_SHIFT                                            3
#define SEC_MDED3_F1_MASK                                    0x00000008
#define SEC_MDED3_F1_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_MDED3_F1_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_MDED3_F1_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_MDED2_F1_WIDTH                                            1
#define SEC_MDED2_F1_SHIFT                                            2
#define SEC_MDED2_F1_MASK                                    0x00000004
#define SEC_MDED2_F1_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_MDED2_F1_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_MDED2_F1_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_MDED1_F1_WIDTH                                            1
#define SEC_MDED1_F1_SHIFT                                            1
#define SEC_MDED1_F1_MASK                                    0x00000002
#define SEC_MDED1_F1_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_MDED1_F1_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_MDED1_F1_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_MDED0_F1_WIDTH                                                1
#define SEC_MDED0_F1_SHIFT                                                0
#define SEC_MDED0_F1_MASK                                        0x00000001
#define SEC_MDED0_F1_RD(src)                         (((src) & 0x00000001))
#define SEC_MDED0_F1_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_MDED0_F1_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MDED_ERRHMASK	*/
/*	 Fields SEC31_F1_MASK	 */
#define SEC_MDED31_F1_MASK_WIDTH                                            1
#define SEC_MDED31_F1_MASK_SHIFT                                            31
#define SEC_MDED31_F1_MASK_MASK                                    0x80000000
#define SEC_MDED31_F1_MASK_RD(src)                  (((src) & 0x80000000)>>31)
#define SEC_MDED31_F1_MASK_WR(src)             (((u32)(src)<<31) & 0x80000000)
#define SEC_MDED31_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x80000000) | (((u32)(src)<<31) & 0x80000000))
/*       Fields SEC30_F1_MASK    */
#define SEC_MDED30_F1_MASK_WIDTH                                            1
#define SEC_MDED30_F1_MASK_SHIFT                                            30
#define SEC_MDED30_F1_MASK_MASK                                    0x40000000
#define SEC_MDED30_F1_MASK_RD(src)                  (((src) & 0x40000000)>>30)
#define SEC_MDED30_F1_MASK_WR(src)             (((u32)(src)<<30) & 0x40000000)
#define SEC_MDED30_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x40000000) | (((u32)(src)<<30) & 0x40000000))
/*       Fields SEC29    */
#define SEC_MDED29_F1_MASK_WIDTH                                            1
#define SEC_MDED29_F1_MASK_SHIFT                                            29
#define SEC_MDED29_F1_MASK                                    0x20000000
#define SEC_MDED29_F1_MASK_RD(src)                  (((src) & 0x20000000)>>29)
#define SEC_MDED29_F1_MASK_WR(src)             (((u32)(src)<<29) & 0x20000000)
#define SEC_MDED29_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x20000000) | (((u32)(src)<<29) & 0x20000000))
/*       Fields SEC28    */
#define SEC_MDED28_F1_MASK_WIDTH                                            1
#define SEC_MDED28_F1_MASK_SHIFT                                            28
#define SEC_MDED28_F1_MASK_MASK                                    0x10000000
#define SEC_MDED28_F1_MASK_RD(src)                  (((src) & 0x10000000)>>28)
#define SEC_MDED28_F1_MASK_WR(src)             (((u32)(src)<<28) & 0x10000000)
#define SEC_MDED28_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x10000000) | (((u32)(src)<<28) & 0x10000000))
/*       Fields SEC27    */
#define SEC_MDED27_F1_MASK_WIDTH                                            1
#define SEC_MDED27_F1_MASK_SHIFT                                            27
#define SEC_MDED27_F1_MASK_MASK                                    0x08000000
#define SEC_MDED27_F1_MASK_RD(src)                  (((src) & 0x08000000)>>27)
#define SEC_MDED27_F1_MASK_WR(src)             (((u32)(src)<<27) & 0x08000000)
#define SEC_MDED27_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x08000000) | (((u32)(src)<<27) & 0x08000000))
/*       Fields SEC26    */
#define SEC_MDED26_F1_MASK_WIDTH                                            1
#define SEC_MDED26_F1_MASK_SHIFT                                            26
#define SEC_MDED26_F1_MASK_MASK                                    0x04000000
#define SEC_MDED26_F1_MASK_RD(src)                  (((src) & 0x04000000)>>26)
#define SEC_MDED26_F1_MASK_WR(src)             (((u32)(src)<<26) & 0x04000000)
#define SEC_MDED26_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x04000000) | (((u32)(src)<<26) & 0x04000000))
/*       Fields SEC25    */
#define SEC_MDED25_F1_MASK_WIDTH                                            1
#define SEC_MDED25_F1_MASK_SHIFT                                            25
#define SEC_MDED25_F1_MASK_MASK                                    0x02000000
#define SEC_MDED25_F1_MASK_RD(src)                  (((src) & 0x02000000)>>25)
#define SEC_MDED25_F1_MASK_WR(src)             (((u32)(src)<<25) & 0x02000000)
#define SEC_MDED25_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x02000000) | (((u32)(src)<<25) & 0x02000000))
/*       Fields SEC24    */
#define SEC_MDED24_F1_MASK_WIDTH                                            1
#define SEC_MDED24_F1_MASK_SHIFT                                            24
#define SEC_MDED24_F1_MASK_MASK                                    0x01000000
#define SEC_MDED24_F1_MASK_RD(src)                  (((src) & 0x01000000)>>24)
#define SEC_MDED24_F1_MASK_WR(src)             (((u32)(src)<<24) & 0x01000000)
#define SEC_MDED24_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x01000000) | (((u32)(src)<<24) & 0x01000000))
/*       Fields SEC23    */
#define SEC_MDED23_F1_MASK_WIDTH                                            1
#define SEC_MDED23_F1_MASK_SHIFT                                            23
#define SEC_MDED23_F1_MASK_MASK                                    0x00800000
#define SEC_MDED23_F1_MASK_RD(src)                  (((src) & 0x00800000)>>23)
#define SEC_MDED23_F1_MASK_WR(src)             (((u32)(src)<<23) & 0x00800000)
#define SEC_MDED23_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00800000) | (((u32)(src)<<23) & 0x00800000))
/*       Fields SEC22    */
#define SEC_MDED22_F1_MASK_WIDTH                                            1
#define SEC_MDED22_F1_MASK_SHIFT                                            22
#define SEC_MDED22_F1_MASK_MASK                                    0x00400000
#define SEC_MDED22_F1_MASK_RD(src)                  (((src) & 0x00400000)>>22)
#define SEC_MDED22_F1_MASK_WR(src)             (((u32)(src)<<22) & 0x00400000)
#define SEC_MDED22_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00400000) | (((u32)(src)<<22) & 0x00400000))
/*       Fields SEC21    */
#define SEC_MDED21_F1_MASK_WIDTH                                            1
#define SEC_MDED21_F1_MASK_SHIFT                                            21
#define SEC_MDED21_F1_MASK_MASK                                    0x00200000
#define SEC_MDED21_F1_MASK_RD(src)                  (((src) & 0x00200000)>>21)
#define SEC_MDED21_F1_MASK_WR(src)             (((u32)(src)<<21) & 0x00200000)
#define SEC_MDED21_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00200000) | (((u32)(src)<<21) & 0x00200000))
/*       Fields SEC20    */
#define SEC_MDED20_F1_MASK_WIDTH                                            1
#define SEC_MDED20_F1_MASK_SHIFT                                            20
#define SEC_MDED20_F1_MASK_MASK                                    0x00100000
#define SEC_MDED20_F1_MASK_RD(src)                  (((src) & 0x00100000)>>20)
#define SEC_MDED20_F1_MASK_WR(src)             (((u32)(src)<<20) & 0x00100000)
#define SEC_MDED20_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00100000) | (((u32)(src)<<20) & 0x00100000))
/*       Fields SEC19    */
#define SEC_MDED19_F1_MASK_WIDTH                                            1
#define SEC_MDED19_F1_MASK_SHIFT                                            19
#define SEC_MDED19_F1_MASK_MASK                                    0x00080000
#define SEC_MDED19_F1_MASK_RD(src)                  (((src) & 0x00080000)>>19)
#define SEC_MDED19_F1_MASK_WR(src)             (((u32)(src)<<19) & 0x00080000)
#define SEC_MDED19_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00080000) | (((u32)(src)<<19) & 0x00080000))
/*       Fields SEC18    */
#define SEC_MDED18_F1_MASK_WIDTH                                            1
#define SEC_MDED18_F1_MASK_SHIFT                                            18
#define SEC_MDED18_F1_MASK_MASK                                    0x00040000
#define SEC_MDED18_F1_MASK_RD(src)                  (((src) & 0x00040000)>>18)
#define SEC_MDED18_F1_MASK_WR(src)             (((u32)(src)<<18) & 0x00040000)
#define SEC_MDED18_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00040000) | (((u32)(src)<<18) & 0x00040000))
/*       Fields SEC17    */
#define SEC_MDED17_F1_MASK_WIDTH                                            1
#define SEC_MDED17_F1_MASK_SHIFT                                            17
#define SEC_MDED17_F1_MASK_MASK                                    0x00020000
#define SEC_MDED17_F1_MASK_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_MDED17_F1_MASK_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_MDED17_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields SEC16    */
#define SEC_MDED16_F1_MASK_WIDTH                                            1
#define SEC_MDED16_F1_MASK_SHIFT                                            16
#define SEC_MDED16_F1_MASK_MASK                                    0x00010000
#define SEC_MDED16_F1_MASK_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_MDED16_F1_MASK_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_MDED16_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*       Fields SEC15    */
#define SEC_MDED15_F1_MASK_WIDTH                                            1
#define SEC_MDED15_F1_MASK_SHIFT                                            15
#define SEC_MDED15_F1_MASK_MASK                                    0x00008000
#define SEC_MDED15_F1_MASK_RD(src)                  (((src) & 0x00008000)>>15)
#define SEC_MDED15_F1_MASK_WR(src)             (((u32)(src)<<15) & 0x00008000)
#define SEC_MDED15_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00008000) | (((u32)(src)<<15) & 0x00008000))
/*       Fields SEC14    */
#define SEC_MDED14_F1_MASK_WIDTH                                            1
#define SEC_MDED14_F1_MASK_SHIFT                                            14
#define SEC_MDED14_F1_MASK_MASK                                    0x00004000
#define SEC_MDED14_F1_MASK_RD(src)                  (((src) & 0x00004000)>>14)
#define SEC_MDED14_F1_MASK_WR(src)             (((u32)(src)<<14) & 0x00004000)
#define SEC_MDED14_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00004000) | (((u32)(src)<<14) & 0x00004000))
/*       Fields SEC13    */
#define SEC_MDED13_F1_MASK_WIDTH                                            1
#define SEC_MDED13_F1_MASK_SHIFT                                            13
#define SEC_MDED13_F1_MASK_MASK                                    0x00002000
#define SEC_MDED13_F1_MASK_RD(src)                  (((src) & 0x00002000)>>13)
#define SEC_MDED13_F1_MASK_WR(src)             (((u32)(src)<<13) & 0x00002000)
#define SEC_MDED13_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00002000) | (((u32)(src)<<13) & 0x00002000))
/*       Fields SEC12    */
#define SEC_MDED12_F1_MASK_WIDTH                                            1
#define SEC_MDED12_F1_MASK_SHIFT                                            12
#define SEC_MDED12_F1_MASK_MASK                                    0x00001000
#define SEC_MDED12_F1_MASK_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_MDED12_F1_MASK_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_MDED12_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SEC11    */
#define SEC_MDED11_F1_MASK_WIDTH                                            1
#define SEC_MDED11_F1_MASK_SHIFT                                            11
#define SEC_MDED11_F1_MASK_MASK                                    0x00000800
#define SEC_MDED11_F1_MASK_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_MDED11_F1_MASK_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_MDED11_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SEC10    */
#define SEC_MDED10_F1_MASK_WIDTH                                            1
#define SEC_MDED10_F1_MASK_SHIFT                                            10
#define SEC_MDED10_F1_MASK_MASK                                    0x00000400
#define SEC_MDED10_F1_MASK_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_MDED10_F1_MASK_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_MDED10_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SEC9    */
#define SEC_MDED9_F1_MASK_WIDTH                                            1
#define SEC_MDED9_F1_MASK_SHIFT                                            9
#define SEC_MDED9_F1_MASK_MASK                                    0x00000200
#define SEC_MDED9_F1_MASK_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_MDED9_F1_MASK_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_MDED9_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields SEC8    */
#define SEC_MDED8_F1_MASK_WIDTH                                            1
#define SEC_MDED8_F1_MASK_SHIFT                                            8
#define SEC_MDED8_F1_MASK_MASK                                    0x00000100
#define SEC_MDED8_F1_MASK_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_MDED8_F1_MASK_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_MDED8_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SEC7    */
#define SEC_MDED7_F1_MASK_WIDTH                                            1
#define SEC_MDED7_F1_MASK_SHIFT                                            7
#define SEC_MDED7_F1_MASK_MASK                                    0x00000080
#define SEC_MDED7_F1_MASK_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_MDED7_F1_MASK_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_MDED7_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SEC6    */
#define SEC_MDED6_F1_MASK_WIDTH                                            1
#define SEC_MDED6_F1_MASK_SHIFT                                            6
#define SEC_MDED6_F1_MASK_MASK                                    0x00000040
#define SEC_MDED6_F1_MASK_RD(src)                  (((src) & 0x00000040)>>6)
#define SEC_MDED6_F1_MASK_WR(src)             (((u32)(src)<<6) & 0x00000040)
#define SEC_MDED6_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000040) | (((u32)(src)<<6) & 0x00000040))
/*       Fields SEC5    */
#define SEC_MDED5_F1_MASK_WIDTH                                            1
#define SEC_MDED5_F1_MASK_SHIFT                                            5
#define SEC_MDED5_F1_MASK_MASK                                    0x00000020
#define SEC_MDED5_F1_MASK_RD(src)                  (((src) & 0x00000020)>>5)
#define SEC_MDED5_F1_MASK_WR(src)             (((u32)(src)<<5) & 0x00000020)
#define SEC_MDED5_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000020) | (((u32)(src)<<5) & 0x00000020))
/*       Fields SEC4    */
#define SEC_MDED4_F1_MASK_WIDTH                                            1
#define SEC_MDED4_F1_MASK_SHIFT                                            4
#define SEC_MDED4_F1_MASK_MASK                                    0x00000010
#define SEC_MDED4_F1_MASK_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_MDED4_F1_MASK_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_MDED4_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields SEC3    */
#define SEC_MDED3_F1_MASK_WIDTH                                            1
#define SEC_MDED3_F1_MASK_SHIFT                                            3
#define SEC_MDED3_F1_MASK_MASK                                    0x00000008
#define SEC_MDED3_F1_MASK_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_MDED3_F1_MASK_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_MDED3_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SEC2    */
#define SEC_MDED2_F1_MASK_WIDTH                                            1
#define SEC_MDED2_F1_MASK_SHIFT                                            2
#define SEC_MDED2_F1_MASK_MASK                                    0x00000004
#define SEC_MDED2_F1_MASK_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_MDED2_F1_MASK_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_MDED2_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields SEC1    */
#define SEC_MDED1_F1_MASK_WIDTH                                            1
#define SEC_MDED1_F1_MASK_SHIFT                                            1
#define SEC_MDED1_F1_MASK_MASK                                    0x00000002
#define SEC_MDED1_F1_MASK_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_MDED1_F1_MASK_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_MDED1_F1_MASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SEC0	 */
#define SEC_MDED0_F1_MASK_WIDTH                                                1
#define SEC_MDED0_F1_MASK_SHIFT                                                0
#define SEC_MDED0_F1_MASK_MASK                                        0x00000001
#define SEC_MDED0_F1_MASK_RD(src)                         (((src) & 0x00000001))
#define SEC_MDED0_F1_MASK_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_MDED0_F1_MASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_MERR_ADDR	*/
/*	Field ERRADDRL	*/
#define SEC_ERRADDRL_WIDTH                                                       32
#define SEC_ERRADDRL_SHIFT                                                        0
#define SEC_ERRADDRL_MASK                                                0xffffffff
#define SEC_ERRADDRL_RD(src)                                 (((src) & 0xffffffff))
#define SEC_ERRADDRL_WR(src)                            (((u32)(src)) & 0xffffffff)
#define SEC_ERRADDRL_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register GLBL_MERR_REQINFO        */
/*       Fields ERRADDRH       */
#define SEC_ERRADDRH_WIDTH                                                10
#define SEC_ERRADDRH_SHIFT                                                 22
#define SEC_ERRADDRH_MASK                                         0xffc00000
#define SEC_ERRADDRH_RD(src)                       (((src) & 0xffc00000)>>22)
#define SEC_ERRADDRH_WR(src)                  (((u32)(src)<<22) & 0xffc00000)
#define SEC_ERRADDRH_SET(dst,src) \
                       (((dst) & ~0xffc00000) | (((u32)(src)<<22) & 0xffc00000))
/*       Fields MSTRID      */
#define SEC_MSTRID_WIDTH                                                6
#define SEC_MSTRID_SHIFT                                                16
#define SEC_MSTRID_MASK                                        0x003f0000
#define SEC_MSTRID_RD(src)                      (((src) & 0x003f0000)>>16)
#define SEC_MSTRID_WR(src)                 (((u32)(src)<<16) & 0x003f0000)
#define SEC_MSTRID_SET(dst,src) \
                       (((dst) & ~0x003f0000) | (((u32)(src)<<16) & 0x003f0000))
/*       Fields AUXINFO  */
#define SEC_AUXINFO_WIDTH                                            6
#define SEC_AUXINFO_SHIFT                                            10
#define SEC_AUXINFO_MASK                                    0x0000fc00
#define SEC_AUXINFO_RD(src)                  (((src) & 0x0000fc00)>>10)
#define SEC_AUXINFO_WR(src)             (((u32)(src)<<10) & 0x0000fc00)
#define SEC_AUXINFO_SET(dst,src) \
                       (((dst) & ~0x0000fc00) | (((u32)(src)<<10) & 0x0000fc00))
/*       Fields REQLEN  */
#define SEC_REQLEN_WIDTH                                            2
#define SEC_REQLEN_SHIFT                                            4
#define SEC_REQLEN_MASK                                    0x00000030
#define SEC_REQLEN_RD(src)                  (((src) & 0x00000030)>>4)
#define SEC_REQLEN_WR(src)             (((u32)(src)<<4) & 0x00000030)
#define SEC_REQLEN_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*       Fields REQSIZE  */
#define SEC_REQSIZE_WIDTH                                            3
#define SEC_REQSIZE_SHIFT                                            1
#define SEC_REQSIZE_MASK                                    0x0000000e
#define SEC_REQSIZE_RD(src)                  (((src) & 0x0000000e)>>1)
#define SEC_REQSIZE_WR(src)             (((u32)(src)<<1) & 0x0000000e)
#define SEC_REQSIZE_SET(dst,src) \
                       (((dst) & ~0x0000000e) | (((u32)(src)<<1) & 0x0000000e))
/*       Fields REQTYPE       */
#define SEC_REQTYPE_WIDTH                                                 1
#define SEC_REQTYPE_SHIFT                                                 0
#define SEC_REQTYPE_MASK                                         0x00000001
#define SEC_REQTYPE_RD(src)                          (((src) & 0x00000001))
#define SEC_REQTYPE_WR(src)                     (((u32)(src)) & 0x00000001)
#define SEC_REQTYPE_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_TRANS_ERR	*/
/*       Fields MSWRPOISON    */
#define SEC_MSWRPOISON_WIDTH                                            1
#define SEC_MSWRPOISON_SHIFT                                            12
#define SEC_MSWRPOISON_MASK                                    0x00001000
#define SEC_MSWRPOISON_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_MSWRPOISON_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_MSWRPOISON_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SWRPOISON    */
#define SEC_SWRPOISON_WIDTH                                            1
#define SEC_SWRPOISON_SHIFT                                            11
#define SEC_SWRPOISON_MASK                                    0x00000800
#define SEC_SWRPOISON_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_SWRPOISON_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_SWRPOISON_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SWRDYTMO    */
#define SEC_SWRDYTMO_WIDTH                                            1
#define SEC_SWRDYTMO_SHIFT                                            10
#define SEC_SWRDYTMO_MASK                                    0x00000400
#define SEC_SWRDYTMO_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_SWRDYTMO_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_SWRDYTMO_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SWRESPTMO    */
#define SEC_SWRESPTMO_WIDTH                                            1
#define SEC_SWRESPTMO_SHIFT                                            9
#define SEC_SWRESPTMO_MASK                                    0x00000200
#define SEC_SWRESPTMO_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_SWRESPTMO_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_SWRESPTMO_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields MSWRERR    */
#define SEC_MSWRERR_WIDTH                                            1
#define SEC_MSWRERR_SHIFT                                            8
#define SEC_MSWRERR_MASK                                    0x00000100
#define SEC_MSWRERR_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_MSWRERR_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_MSWRERR_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SWRERR    */
#define SEC_SWRERR_WIDTH                                            1
#define SEC_SWRERR_SHIFT                                            7
#define SEC_SWRERR_MASK                                    0x00000080
#define SEC_SWRERR_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_SWRERR_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_SWRERR_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SRRDYTMO    */
#define SEC_SRRDYTMO_WIDTH                                            1
#define SEC_SRRDYTMO_SHIFT                                            3
#define SEC_SRRDYTMO_MASK                                    0x00000008
#define SEC_SRRDYTMO_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_SRRDYTMO_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_SRRDYTMO_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SRRESPTMO    */
#define SEC_SRRESPTMO_WIDTH                                            1
#define SEC_SRRESPTMO_SHIFT                                            2
#define SEC_SRRESPTMO_MASK                                    0x00000004
#define SEC_SRRESPTMO_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_SRRESPTMO_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_SRRESPTMO_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields MSRDERR    */
#define SEC_MSRDERR_WIDTH                                            1
#define SEC_MSRDERR_SHIFT                                            1
#define SEC_MSRDERR_MASK                                    0x00000002
#define SEC_MSRDERR_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_MSRDERR_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_MSRDERR_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SRDERR	 */
#define SEC_SRDERR_WIDTH                                                1
#define SEC_SRDERR_SHIFT                                                0
#define SEC_SRDERR_MASK                                        0x00000001
#define SEC_SRDERR_RD(src)                         (((src) & 0x00000001))
#define SEC_SRDERR_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_SRDERR_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_TRANS_ERRMASK	*/
/*       Fields MSWRPOISONMASK    */
#define SEC_MSWRPOISONMASK_WIDTH                                            1
#define SEC_MSWRPOISONMASK_SHIFT                                            12
#define SEC_MSWRPOISONMASK_MASK                                    0x00001000
#define SEC_MSWRPOISONMASK_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_MSWRPOISONMASK_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_MSWRPOISONMASK_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SWRPOISONMASK    */
#define SEC_SWRPOISONMASK_WIDTH                                            1
#define SEC_SWRPOISONMASK_SHIFT                                            11
#define SEC_SWRPOISONMASK_MASK                                    0x00000800
#define SEC_SWRPOISONMASK_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_SWRPOISONMASK_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_SWRPOISONMASK_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SWRDYTMOMASK    */
#define SEC_SWRDYTMOMASK_WIDTH                                            1
#define SEC_SWRDYTMOMASK_SHIFT                                            10
#define SEC_SWRDYTMOMASK_MASK                                    0x00000400
#define SEC_SWRDYTMOMASK_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_SWRDYTMOMASK_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_SWRDYTMOMASK_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SWRESPTMOMASK    */
#define SEC_SWRESPTMOMASK_WIDTH                                            1
#define SEC_SWRESPTMOMASK_SHIFT                                            9
#define SEC_SWRESPTMOMASK_MASK                                    0x00000200
#define SEC_SWRESPTMOMASK_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_SWRESPTMOMASK_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_SWRESPTMOMASK_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields MSWRERRMASK    */
#define SEC_MSWRERRMASK_WIDTH                                            1
#define SEC_MSWRERRMASK_SHIFT                                            8
#define SEC_MSWRERRMASK_MASK                                    0x00000100
#define SEC_MSWRERRMASK_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_MSWRERRMASK_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_MSWRERRMASK_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SWRERRMASK    */
#define SEC_SWRERRMASK_WIDTH                                            1
#define SEC_SWRERRMASK_SHIFT                                            7
#define SEC_SWRERRMASK_MASK                                    0x00000080
#define SEC_SWRERRMASK_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_SWRERRMASK_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_SWRERRMASK_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SRRDYTMOMASK    */
#define SEC_SRRDYTMOMASK_WIDTH                                            1
#define SEC_SRRDYTMOMASK_SHIFT                                            3
#define SEC_SRRDYTMOMASK_MASK                                    0x00000008
#define SEC_SRRDYTMOMASK_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_SRRDYTMOMASK_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_SRRDYTMOMASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SRRESPTMOMASK    */
#define SEC_SRRESPTMOMASK_WIDTH                                            1
#define SEC_SRRESPTMOMASK_SHIFT                                            2
#define SEC_SRRESPTMOMASK_MASK                                    0x00000004
#define SEC_SRRESPTMOMASK_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_SRRESPTMOMASK_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_SRRESPTMOMASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields MSRDERRMASK    */
#define SEC_MSRDERRMASK_WIDTH                                            1
#define SEC_MSRDERRMASK_SHIFT                                            1
#define SEC_MSRDERRMASK_MASK                                    0x00000002
#define SEC_MSRDERRMASK_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_MSRDERRMASK_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_MSRDERRMASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SRDERRMASK	 */
#define SEC_SRDERRMASK_WIDTH                                                1
#define SEC_SRDERRMASK_SHIFT                                                0
#define SEC_SRDERRMASK_MASK                                        0x00000001
#define SEC_SRDERRMASK_RD(src)                         (((src) & 0x00000001))
#define SEC_SRDERRMASK_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_SRDERRMASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_WDERR_ADDR	*/
/*	Field WDERR_ERRADDRL	*/
#define SEC_WDERR_ERRADDRL_WIDTH                                                       32
#define SEC_WDERR_ERRADDRL_SHIFT                                                        0
#define SEC_WDERR_ERRADDRL_MASK                                                0xffffffff
#define SEC_WDERR_ERRADDRL_RD(src)                                 (((src) & 0xffffffff))
#define SEC_WDERR_ERRADDRL_WR(src)                            (((u32)(src)) & 0xffffffff)
#define SEC_WDERR_ERRADDRL_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register GLBL_WDERR_REQINFO        */
/*       Fields ERRADDRH       */
#define SEC_WDERR_ERRADDRH_WIDTH                                                10
#define SEC_WDERR_ERRADDRH_SHIFT                                                 22
#define SEC_WDERR_ERRADDRH_MASK                                         0xffc00000
#define SEC_WDERR_ERRADDRH_RD(src)                       (((src) & 0xffc00000)>>22)
#define SEC_WDERR_ERRADDRH_WR(src)                  (((u32)(src)<<22) & 0xffc00000)
#define SEC_WDERR_ERRADDRH_SET(dst,src) \
                       (((dst) & ~0xffc00000) | (((u32)(src)<<22) & 0xffc00000))
/*       Fields MSTRID      */
#define SEC_WDERR_MSTRID_WIDTH                                                6
#define SEC_WDERR_MSTRID_SHIFT                                                16
#define SEC_WDERR_MSTRID_MASK                                        0x003f0000
#define SEC_WDERR_MSTRID_RD(src)                      (((src) & 0x003f0000)>>16)
#define SEC_WDERR_MSTRID_WR(src)                 (((u32)(src)<<16) & 0x003f0000)
#define SEC_WDERR_MSTRID_SET(dst,src) \
                       (((dst) & ~0x003f0000) | (((u32)(src)<<16) & 0x003f0000))
/*       Fields AUXINFO  */
#define SEC_WDERR_AUXINFO_WIDTH                                            6
#define SEC_WDERR_AUXINFO_SHIFT                                            10
#define SEC_WDERR_AUXINFO_MASK                                    0x0000fc00
#define SEC_WDERR_AUXINFO_RD(src)                  (((src) & 0x0000fc00)>>10)
#define SEC_WDERR_AUXINFO_WR(src)             (((u32)(src)<<10) & 0x0000fc00)
#define SEC_WDERR_AUXINFO_SET(dst,src) \
                       (((dst) & ~0x0000fc00) | (((u32)(src)<<10) & 0x0000fc00))
/*       Fields REQLEN  */
#define SEC_WDERR_REQLEN_WIDTH                                            2
#define SEC_WDERR_REQLEN_SHIFT                                            4
#define SEC_WDERR_REQLEN_MASK                                    0x00000030
#define SEC_WDERR_REQLEN_RD(src)                  (((src) & 0x00000030)>>4)
#define SEC_WDERR_REQLEN_WR(src)             (((u32)(src)<<4) & 0x00000030)
#define SEC_WDERR_REQLEN_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*       Fields REQSIZE  */
#define SEC_WDERR_REQSIZE_WIDTH                                            3
#define SEC_WDERR_REQSIZE_SHIFT                                            1
#define SEC_WDERR_REQSIZE_MASK                                    0x0000000e
#define SEC_WDERR_REQSIZE_RD(src)                  (((src) & 0x0000000e)>>1)
#define SEC_WDERR_REQSIZE_WR(src)             (((u32)(src)<<1) & 0x0000000e)
#define SEC_WDERR_REQSIZE_SET(dst,src) \
                       (((dst) & ~0x0000000e) | (((u32)(src)<<1) & 0x0000000e))
/*       Fields REQTYPE       */
#define SEC_WDERR_REQTYPE_WIDTH                                                 1
#define SEC_WDERR_REQTYPE_SHIFT                                                 0
#define SEC_WDERR_REQTYPE_MASK                                         0x00000001
#define SEC_WDERR_REQTYPE_RD(src)                          (((src) & 0x00000001))
#define SEC_WDERR_REQTYPE_WR(src)                     (((u32)(src)) & 0x00000001)
#define SEC_WDERR_REQTYPE_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register GLBL_DEVERR_ADDR	*/
/*	Field DEVERR_ERRADDRL	*/
#define SEC_DEVERR_ERRADDRL_WIDTH                                                       32
#define SEC_DEVERR_ERRADDRL_SHIFT                                                        0
#define SEC_DEVERR_ERRADDRL_MASK                                                0xffffffff
#define SEC_DEVERR_ERRADDRL_RD(src)                                 (((src) & 0xffffffff))
#define SEC_DEVERR_ERRADDRL_WR(src)                            (((u32)(src)) & 0xffffffff)
#define SEC_DEVERR_ERRADDRL_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register GLBL_DEVERR_REQINFO        */
/*       Fields ERRADDRH       */
#define SEC_DEVERR_ERRADDRH_WIDTH                                                10
#define SEC_DEVERR_ERRADDRH_SHIFT                                                 22
#define SEC_DEVERR_ERRADDRH_MASK                                         0xffc00000
#define SEC_DEVERR_ERRADDRH_RD(src)                       (((src) & 0xffc00000)>>22)
#define SEC_DEVERR_ERRADDRH_WR(src)                  (((u32)(src)<<22) & 0xffc00000)
#define SEC_DEVERR_ERRADDRH_SET(dst,src) \
                       (((dst) & ~0xffc00000) | (((u32)(src)<<22) & 0xffc00000))
/*       Fields MSTRID      */
#define SEC_DEVERR_MSTRID_WIDTH                                                6
#define SEC_DEVERR_MSTRID_SHIFT                                                16
#define SEC_DEVERR_MSTRID_MASK                                        0x003f0000
#define SEC_DEVERR_MSTRID_RD(src)                      (((src) & 0x003f0000)>>16)
#define SEC_DEVERR_MSTRID_WR(src)                 (((u32)(src)<<16) & 0x003f0000)
#define SEC_DEVERR_MSTRID_SET(dst,src) \
                       (((dst) & ~0x003f0000) | (((u32)(src)<<16) & 0x003f0000))
/*       Fields AUXINFO  */
#define SEC_DEVERR_AUXINFO_WIDTH                                            6
#define SEC_DEVERR_AUXINFO_SHIFT                                            10
#define SEC_DEVERR_AUXINFO_MASK                                    0x0000fc00
#define SEC_DEVERR_AUXINFO_RD(src)                  (((src) & 0x0000fc00)>>10)
#define SEC_DEVERR_AUXINFO_WR(src)             (((u32)(src)<<10) & 0x0000fc00)
#define SEC_DEVERR_AUXINFO_SET(dst,src) \
                       (((dst) & ~0x0000fc00) | (((u32)(src)<<10) & 0x0000fc00))
/*       Fields REQLEN  */
#define SEC_DEVERR_REQLEN_WIDTH                                            2
#define SEC_DEVERR_REQLEN_SHIFT                                            4
#define SEC_DEVERR_REQLEN_MASK                                    0x00000030
#define SEC_DEVERR_REQLEN_RD(src)                  (((src) & 0x00000030)>>4)
#define SEC_DEVERR_REQLEN_WR(src)             (((u32)(src)<<4) & 0x00000030)
#define SEC_DEVERR_REQLEN_SET(dst,src) \
                       (((dst) & ~0x00000030) | (((u32)(src)<<4) & 0x00000030))
/*       Fields REQSIZE  */
#define SEC_DEVERR_REQSIZE_WIDTH                                            3
#define SEC_DEVERR_REQSIZE_SHIFT                                            1
#define SEC_DEVERR_REQSIZE_MASK                                    0x0000000e
#define SEC_DEVERR_REQSIZE_RD(src)                  (((src) & 0x0000000e)>>1)
#define SEC_DEVERR_REQSIZE_WR(src)             (((u32)(src)<<1) & 0x0000000e)
#define SEC_DEVERR_REQSIZE_SET(dst,src) \
                       (((dst) & ~0x0000000e) | (((u32)(src)<<1) & 0x0000000e))
/*       Fields REQTYPE       */
#define SEC_DEVERR_REQTYPE_WIDTH                                                 1
#define SEC_DEVERR_REQTYPE_SHIFT                                                 0
#define SEC_DEVERR_REQTYPE_MASK                                         0x00000001
#define SEC_DEVERR_REQTYPE_RD(src)                          (((src) & 0x00000001))
#define SEC_DEVERR_REQTYPE_WR(src)                     (((u32)(src)) & 0x00000001)
#define SEC_DEVERR_REQTYPE_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*      Register GLBL_SEC_ERRL_ALS       */
/*       Fields SEC     */
#define SEC_SECL_WIDTH                                                      32
#define SEC_SECL_SHIFT                                                       0
#define SEC_SECL_MASK                                               0xffffffff
#define SEC_SECL_RD(src)                                (((src) & 0xffffffff))
#define SEC_SECL_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_SECL_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register GLBL_SEC_ERRH_ALS       */
/*       Fields SEC     */
#define SEC_SECH_WIDTH                                                      32
#define SEC_SECH_SHIFT                                                       0
#define SEC_SECH_MASK                                               0xffffffff
#define SEC_SECH_RD(src)                                (((src) & 0xffffffff))
#define SEC_SECH_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_SECH_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register GLBL_DED_ERRL_ALS       */
/*       Fields SEC     */
#define SEC_DEDL_WIDTH                                                      32
#define SEC_DEDL_SHIFT                                                       0
#define SEC_DEDL_MASK                                               0xffffffff
#define SEC_DEDL_RD(src)                                (((src) & 0xffffffff))
#define SEC_DEDL_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_DEDL_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register GLBL_DED_ERRH_ALS       */
/*       Fields SEC     */
#define SEC_DEDH_WIDTH                                                      32
#define SEC_DEDH_SHIFT                                                       0
#define SEC_DEDH_MASK                                               0xffffffff
#define SEC_DEDH_RD(src)                                (((src) & 0xffffffff))
#define SEC_DEDH_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_DEDH_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register GLBL_TRANS_ERR_ALS	*/
/*       Fields MSWRPOISON    */
#define SEC_ALS_MSWRPOISON_WIDTH                                            1
#define SEC_ALS_MSWRPOISON_SHIFT                                            12
#define SEC_ALS_MSWRPOISON_MASK                                    0x00001000
#define SEC_ALS_MSWRPOISON_RD(src)                  (((src) & 0x00001000)>>12)
#define SEC_ALS_MSWRPOISON_WR(src)             (((u32)(src)<<12) & 0x00001000)
#define SEC_ALS_MSWRPOISON_SET(dst,src) \
                       (((dst) & ~0x00001000) | (((u32)(src)<<12) & 0x00001000))
/*       Fields SWRPOISON    */
#define SEC_ALS_SWRPOISON_WIDTH                                            1
#define SEC_ALS_SWRPOISON_SHIFT                                            11
#define SEC_ALS_SWRPOISON_MASK                                    0x00000800
#define SEC_ALS_SWRPOISON_RD(src)                  (((src) & 0x00000800)>>11)
#define SEC_ALS_SWRPOISON_WR(src)             (((u32)(src)<<11) & 0x00000800)
#define SEC_ALS_SWRPOISON_SET(dst,src) \
                       (((dst) & ~0x00000800) | (((u32)(src)<<11) & 0x00000800))
/*       Fields SWRDYTMO    */
#define SEC_ALS_SWRDYTMO_WIDTH                                            1
#define SEC_ALS_SWRDYTMO_SHIFT                                            10
#define SEC_ALS_SWRDYTMO_MASK                                    0x00000400
#define SEC_ALS_SWRDYTMO_RD(src)                  (((src) & 0x00000400)>>10)
#define SEC_ALS_SWRDYTMO_WR(src)             (((u32)(src)<<10) & 0x00000400)
#define SEC_ALS_SWRDYTMO_SET(dst,src) \
                       (((dst) & ~0x00000400) | (((u32)(src)<<10) & 0x00000400))
/*       Fields SWRESPTMO    */
#define SEC_ALS_SWRESPTMO_WIDTH                                            1
#define SEC_ALS_SWRESPTMO_SHIFT                                            9
#define SEC_ALS_SWRESPTMO_MASK                                    0x00000200
#define SEC_ALS_SWRESPTMO_RD(src)                  (((src) & 0x00000200)>>9)
#define SEC_ALS_SWRESPTMO_WR(src)             (((u32)(src)<<9) & 0x00000200)
#define SEC_ALS_SWRESPTMO_SET(dst,src) \
                       (((dst) & ~0x00000200) | (((u32)(src)<<9) & 0x00000200))
/*       Fields MSWRERR    */
#define SEC_ALS_MSWRERR_WIDTH                                            1
#define SEC_ALS_MSWRERR_SHIFT                                            8
#define SEC_ALS_MSWRERR_MASK                                    0x00000100
#define SEC_ALS_MSWRERR_RD(src)                  (((src) & 0x00000100)>>8)
#define SEC_ALS_MSWRERR_WR(src)             (((u32)(src)<<8) & 0x00000100)
#define SEC_ALS_MSWRERR_SET(dst,src) \
                       (((dst) & ~0x00000100) | (((u32)(src)<<8) & 0x00000100))
/*       Fields SWRERR    */
#define SEC_ALS_SWRERR_WIDTH                                            1
#define SEC_ALS_SWRERR_SHIFT                                            7
#define SEC_ALS_SWRERR_MASK                                    0x00000080
#define SEC_ALS_SWRERR_RD(src)                  (((src) & 0x00000080)>>7)
#define SEC_ALS_SWRERR_WR(src)             (((u32)(src)<<7) & 0x00000080)
#define SEC_ALS_SWRERR_SET(dst,src) \
                       (((dst) & ~0x00000080) | (((u32)(src)<<7) & 0x00000080))
/*       Fields SRRDYTMO    */
#define SEC_ALS_SRRDYTMO_WIDTH                                            1
#define SEC_ALS_SRRDYTMO_SHIFT                                            3
#define SEC_ALS_SRRDYTMO_MASK                                    0x00000008
#define SEC_ALS_SRRDYTMO_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_ALS_SRRDYTMO_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_ALS_SRRDYTMO_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields SRRESPTMO    */
#define SEC_ALS_SRRESPTMO_WIDTH                                            1
#define SEC_ALS_SRRESPTMO_SHIFT                                            2
#define SEC_ALS_SRRESPTMO_MASK                                    0x00000004
#define SEC_ALS_SRRESPTMO_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_ALS_SRRESPTMO_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_ALS_SRRESPTMO_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields MSRDERR    */
#define SEC_ALS_MSRDERR_WIDTH                                            1
#define SEC_ALS_MSRDERR_SHIFT                                            1
#define SEC_ALS_MSRDERR_MASK                                    0x00000002
#define SEC_ALS_MSRDERR_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_ALS_MSRDERR_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_ALS_MSRDERR_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields SRDERR	 */
#define SEC_ALS_SRDERR_WIDTH                                                1
#define SEC_ALS_SRDERR_SHIFT                                                0
#define SEC_ALS_SRDERR_MASK                                        0x00000001
#define SEC_ALS_SRDERR_RD(src)                         (((src) & 0x00000001))
#define SEC_ALS_SRDERR_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_ALS_SRDERR_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*      Global Base Address     */
#define SEC_CLK_RST_CSR_BASE_ADDR		0x1f25c000

/*    Address SEC_CLK_RST_CSR	Registers */
#define	CSR_SEC_SRST_ADDR				0x00000000
#define CSR_SEC_SRST_DEFAULT				0x0000001f
#define CSR_SEC_CLKEN_ADDR                               0x00000008
#define CSR_SEC_CLKEN_DEFAULT                            0x00000000

/*	Register CSR_SEC_SRST	*/
/*       Fields EIP62_RESET    */
#define SEC_EIP62_RESET_WIDTH                                            1
#define SEC_EIP62_RESET_SHIFT                                            4
#define SEC_EIP62_RESET_MASK                                    0x00000010
#define SEC_EIP62_RESET_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_EIP62_RESET_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_EIP62_RESET_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields XTS_RESET    */
#define SEC_XTS_RESET_WIDTH                                            1
#define SEC_XTS_RESET_SHIFT                                            3
#define SEC_XTS_RESET_MASK                                    0x00000008
#define SEC_XTS_RESET_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_XTS_RESET_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_XTS_RESET_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields EIP96_RESET    */
#define SEC_EIP96_RESET_WIDTH                                            1
#define SEC_EIP96_RESET_SHIFT                                            2
#define SEC_EIP96_RESET_MASK                                    0x00000004
#define SEC_EIP96_RESET_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_EIP96_RESET_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_EIP96_RESET_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields AXI_RESET    */
#define SEC_AXI_RESET_WIDTH                                            1
#define SEC_AXI_RESET_SHIFT                                            1
#define SEC_AXI_RESET_MASK                                    0x00000002
#define SEC_AXI_RESET_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_AXI_RESET_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_AXI_RESET_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields CSR_RESET	 */
#define SEC_CSR_RESET_WIDTH                                                1
#define SEC_CSR_RESET_SHIFT                                                0
#define SEC_CSR_RESET_MASK                                        0x00000001
#define SEC_CSR_RESET_RD(src)                         (((src) & 0x00000001))
#define SEC_CSR_RESET_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_CSR_RESET_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register CSR_SEC_CLKEN	*/
/*       Fields EIP62_CLKEN    */
#define SEC_EIP62_CLKEN_WIDTH                                            1
#define SEC_EIP62_CLKEN_SHIFT                                            4
#define SEC_EIP62_CLKEN_MASK                                    0x00000010
#define SEC_EIP62_CLKEN_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_EIP62_CLKEN_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_EIP62_CLKEN_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields XTS_CLKEN    */
#define SEC_XTS_CLKEN_WIDTH                                            1
#define SEC_XTS_CLKEN_SHIFT                                            3
#define SEC_XTS_CLKEN_MASK                                    0x00000008
#define SEC_XTS_CLKEN_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_XTS_CLKEN_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_XTS_CLKEN_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields EIP96_CLKEN    */
#define SEC_EIP96_CLKEN_WIDTH                                            1
#define SEC_EIP96_CLKEN_SHIFT                                            2
#define SEC_EIP96_CLKEN_MASK                                    0x00000004
#define SEC_EIP96_CLKEN_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_EIP96_CLKEN_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_EIP96_CLKEN_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields AXI_CLKEN    */
#define SEC_AXI_CLKEN_WIDTH                                            1
#define SEC_AXI_CLKEN_SHIFT                                            1
#define SEC_AXI_CLKEN_MASK                                    0x00000002
#define SEC_AXI_CLKEN_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_AXI_CLKEN_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_AXI_CLKEN_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields CSR_CLKEN	 */
#define SEC_CSR_CLKEN_WIDTH                                                1
#define SEC_CSR_CLKEN_SHIFT                                                0
#define SEC_CSR_CLKEN_MASK                                        0x00000001
#define SEC_CSR_CLKEN_RD(src)                         (((src) & 0x00000001))
#define SEC_CSR_CLKEN_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_CSR_CLKEN_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*      Global Base Address     */
#define SEC_GLBL_MASTER_SHIM_CSR_BASE_ADDR			0x1f25f000

/*    Address SEC_GLBL_MASTER_SHIM_CSR  Registers */
#define SEC_CFG_MST_IOB_SEL_ADDR			0x00000004
#define SEC_CFG_MST_IOB_SEL_DEFAULT                     0x00000002
#define SEC_CFG_VC0_PREFETCH_ADDR			0x00000008
#define SEC_CFG_VC0_PREFETCH_DEFAULT                    0x00000004
#define SEC_CFG_VC1_PREFETCH_ADDR			0x0000000c
#define SEC_CFG_VC1_PREFETCH_DEFAULT                    0x00000004
#define SEC_CFG_VC2_PREFETCH_ADDR			0x00000010
#define SEC_CFG_VC2_PREFETCH_DEFAULT                    0x00000004
#define SEC_VC0_TOKEN_USED_ADDR				0x00000014
#define SEC_VC0_TOKEN_USED_DEFAULT                      0x00000000
#define SEC_VC0_TOKEN_USED_DEFAULT                      0x00000000
#define SEC_VC1_TOKEN_USED_ADDR				0x00000018
#define SEC_VC1_TOKEN_USED_DEFAULT                      0x00000000
#define SEC_VC2_TOKEN_USED_ADDR				0x0000001c
#define SEC_VC2_TOKEN_USED_DEFAULT                      0x00000000
#define SEC_VC0_TOKEN_REQ_ADDR				0x00000020
#define SEC_VC0_TOKEN_REQ_DEFAULT                       0x00000000
#define SEC_VC1_TOKEN_REQ_ADDR				0x00000024
#define SEC_VC1_TOKEN_REQ_DEFAULT                       0x00000000
#define SEC_VC2_TOKEN_REQ_ADDR				0x00000028
#define SEC_VC2_TOKEN_REQ_DEFAULT                       0x00000000

/*	Register CFG_MST_IOB_SEL	*/
/*       Fields CFG_VC_BYPASS    */
#define SEC_CFG_VC_BYPASS_WIDTH                                            1
#define SEC_CFG_VC_BYPASS_SHIFT                                            1
#define SEC_CFG_VC_BYPASS_MASK                                    0x00000002
#define SEC_CFG_VC_BYPASS_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_CFG_VC_BYPASS_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_CFG_VC_BYPASS_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields CFG_MST_IOB_SEL	 */
#define SEC_CFG_MST_IOB_SEL_WIDTH                                                1
#define SEC_CFG_MST_IOB_SEL_SHIFT                                                0
#define SEC_CFG_MST_IOB_SEL_MASK                                        0x00000001
#define SEC_CFG_MST_IOB_SEL_RD(src)                         (((src) & 0x00000001))
#define SEC_CFG_MST_IOB_SEL_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_CFG_MST_IOB_SEL_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register CFG_VC0_PREFETCH	*/
/*	 Fields CFG_VC0_PREFETCH_CNT	 */
#define SEC_CFG_VC0_PREFETCH_CNT_WIDTH                                                4
#define SEC_CFG_VC0_PREFETCH_CNT_SHIFT                                                0
#define SEC_CFG_VC0_PREFETCH_CNT_MASK                                        0x0000000f
#define SEC_CFG_VC0_PREFETCH_CNT_RD(src)                         (((src) & 0x0000000f))
#define SEC_CFG_VC0_PREFETCH_CNT_WR(src)                    (((u32)(src)) & 0x0000000f)
#define SEC_CFG_VC0_PREFETCH_CNT_SET(dst,src) \
                          (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*      Register CFG_VC1_PREFETCH       */
/*       Fields CFG_VC1_PREFETCH_CNT     */
#define SEC_CFG_VC1_PREFETCH_CNT_WIDTH                                                4
#define SEC_CFG_VC1_PREFETCH_CNT_SHIFT                                                0
#define SEC_CFG_VC1_PREFETCH_CNT_MASK                                        0x0000000f
#define SEC_CFG_VC1_PREFETCH_CNT_RD(src)                         (((src) & 0x0000000f))
#define SEC_CFG_VC1_PREFETCH_CNT_WR(src)                    (((u32)(src)) & 0x0000000f)
#define SEC_CFG_VC1_PREFETCH_CNT_SET(dst,src) \
                          (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*      Register CFG_VC2_PREFETCH       */
/*       Fields CFG_VC2_PREFETCH_CNT     */
#define SEC_CFG_VC2_PREFETCH_CNT_WIDTH                                                4
#define SEC_CFG_VC2_PREFETCH_CNT_SHIFT                                                0
#define SEC_CFG_VC2_PREFETCH_CNT_MASK                                        0x0000000f
#define SEC_CFG_VC2_PREFETCH_CNT_RD(src)                         (((src) & 0x0000000f))
#define SEC_CFG_VC2_PREFETCH_CNT_WR(src)                    (((u32)(src)) & 0x0000000f)
#define SEC_CFG_VC2_PREFETCH_CNT_SET(dst,src) \
                          (((dst) & ~0x0000000f) | (((u32)(src)) & 0x0000000f))

/*      Register VC0_TOKEN_USED       */
/*       Fields VC0_TOKEN_USED     */
#define SEC_VC0_TOKEN_USED_WIDTH                                                      32
#define SEC_VC0_TOKEN_USED_SHIFT                                                       0
#define SEC_VC0_TOKEN_USED_MASK                                               0xffffffff
#define SEC_VC0_TOKEN_USED_RD(src)                                (((src) & 0xffffffff))
#define SEC_VC0_TOKEN_USED_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register VC1_TOKEN_USED       */
/*       Fields VC1_TOKEN_USED     */
#define SEC_VC1_TOKEN_USED_WIDTH                                                      32
#define SEC_VC1_TOKEN_USED_SHIFT                                                       0
#define SEC_VC1_TOKEN_USED_MASK                                               0xffffffff
#define SEC_VC1_TOKEN_USED_RD(src)                                (((src) & 0xffffffff))
#define SEC_VC1_TOKEN_USED_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register VC2_TOKEN_USED       */
/*       Fields VC2_TOKEN_USED     */
#define SEC_VC2_TOKEN_USED_WIDTH                                                      32
#define SEC_VC2_TOKEN_USED_SHIFT                                                       0
#define SEC_VC2_TOKEN_USED_MASK                                               0xffffffff
#define SEC_VC2_TOKEN_USED_RD(src)                                (((src) & 0xffffffff))
#define SEC_VC2_TOKEN_USED_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register VC0_TOKEN_REQ       */
/*       Fields VC0_TOKEN_REQ     */
#define SEC_VC0_TOKEN_REQ_WIDTH                                                      32
#define SEC_VC0_TOKEN_REQ_SHIFT                                                       0
#define SEC_VC0_TOKEN_REQ_MASK                                               0xffffffff
#define SEC_VC0_TOKEN_REQ_RD(src)                                (((src) & 0xffffffff))
#define SEC_VC0_TOKEN_REQ_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register VC1_TOKEN_REQ       */
/*       Fields VC1_TOKEN_REQ     */
#define SEC_VC1_TOKEN_REQ_WIDTH                                                      32
#define SEC_VC1_TOKEN_REQ_SHIFT                                                       0
#define SEC_VC1_TOKEN_REQ_MASK                                               0xffffffff
#define SEC_VC1_TOKEN_REQ_RD(src)                                (((src) & 0xffffffff))
#define SEC_VC1_TOKEN_REQ_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Register VC2_TOKEN_REQ       */
/*       Fields VC2_TOKEN_REQ     */
#define SEC_VC2_TOKEN_REQ_WIDTH                                                      32
#define SEC_VC2_TOKEN_REQ_SHIFT                                                       0
#define SEC_VC2_TOKEN_REQ_MASK                                               0xffffffff
#define SEC_VC2_TOKEN_REQ_RD(src)                                (((src) & 0xffffffff))
#define SEC_VC2_TOKEN_REQ_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*      Global Base Address     */
#define SEC_GLBL_SLAVE_SHIM_CSR_BASE_ADDR			0x1f25e000

/*    Address SEC_GLBL_SLAVE_SHIM_CSR  Registers */
#define SEC_CFG_SLV_RESP_TMO_CNTR_ADDR				0x00000004
#define SEC_CFG_SLV_RESP_TMO_CNTR_DEFAULT                       0x0000ffff
#define SEC_CFG_SLV_READY_TMO_CNTR_ADDR				0x00000008
#define SEC_CFG_SLV_READY_TMO_CNTR_DEFAULT                      0x0004ffff
#define SEC_CFG_AMA_MODE_ADDR					0x00000014
#define SEC_CFG_AMA_MODE_DEFAULT                                0x00000000
#define SEC_CFG_SLV_CSR_TMO_CNTR_ADDR				0x00000018
#define SEC_CFG_SLV_CSR_TMO_CNTR_DEFAULT                        0x0000ffff
#define SEC_CFG_MASK_DEV_ERR_RESP_ADDR				0x0000001c
#define SEC_CFG_MASK_DEV_ERR_RESP_DEFAULT                       0x00000000
#define SEC_INT_SLV_TMO_ADDR					0x0000000c
#define SEC_INT_SLV_TMO_DEFAULT                                 0x00000000
#define SEC_INT_SLV_TMOMASK_ADDR				0x00000010
#define SEC_INT_SLV_TMOMASK_DEFAULT                             0xffffffff

/*	Register CFG_SLV_RESP_TMO_CNTR	*/
/*       Fields CFG_CSR_POISON    */
#define SEC_CFG_CSR_POISON_WIDTH                                            1
#define SEC_CFG_CSR_POISON_SHIFT                                            17
#define SEC_CFG_CSR_POISON_MASK                                    0x00020000
#define SEC_CFG_CSR_POISON_RD(src)                  (((src) & 0x00020000)>>17)
#define SEC_CFG_CSR_POISON_WR(src)             (((u32)(src)<<17) & 0x00020000)
#define SEC_CFG_CSR_POISON_SET(dst,src) \
                       (((dst) & ~0x00020000) | (((u32)(src)<<17) & 0x00020000))
/*       Fields CSR_ERR_RESP_EN    */
#define SEC_CSR_ERR_RESP_EN_WIDTH                                            1
#define SEC_CSR_ERR_RESP_EN_SHIFT                                            16
#define SEC_CSR_ERR_RESP_EN_MASK                                    0x00010000
#define SEC_CSR_ERR_RESP_EN_RD(src)                  (((src) & 0x00010000)>>16)
#define SEC_CSR_ERR_RESP_EN_WR(src)             (((u32)(src)<<16) & 0x00010000)
#define SEC_CSR_ERR_RESP_EN_SET(dst,src) \
                       (((dst) & ~0x00010000) | (((u32)(src)<<16) & 0x00010000))
/*	 Fields CFG_CSR_RESP_TMO	 */
#define SEC_CFG_CSR_RESP_TMO_WIDTH                                                16
#define SEC_CFG_CSR_RESP_TMO_SHIFT                                                0
#define SEC_CFG_CSR_RESP_TMO_MASK                                        0x0000ffff
#define SEC_CFG_CSR_RESP_TMO_RD(src)                         (((src) & 0x0000ffff))
#define SEC_CFG_CSR_RESP_TMO_WR(src)                    (((u32)(src)) & 0x0000ffff)
#define SEC_CFG_CSR_RESP_TMO_SET(dst,src) \
                          (((dst) & ~0x0000ffff) | (((u32)(src)) & 0x0000ffff))

/*      Register CFG_SLV_READY_TMO_CNTR       */
/*       Fields CFG_CSR_READY_TMO     */
#define SEC_CFG_CSR_READY_TMO_WIDTH                                                      32
#define SEC_CFG_CSR_READY_TMO_SHIFT                                                       0
#define SEC_CFG_CSR_READY_TMO_MASK                                               0xffffffff
#define SEC_CFG_CSR_READY_TMO_RD(src)                                (((src) & 0xffffffff))
#define SEC_CFG_CSR_READY_TMO_WR(src)                           (((u32)(src)) & 0xffffffff)
#define SEC_CFG_CSR_READY_TMO_SET(dst,src) \
                          (((dst) & ~0xffffffff) | (((u32)(src)) & 0xffffffff))

/*	Register INT_SLV_TMO 	*/
/*       Fields STS_CSR_TMO    */
#define SEC_STS_CSR_TMO_WIDTH                                            1
#define SEC_STS_CSR_TMO_SHIFT                                            4
#define SEC_STS_CSR_TMO_MASK                                    0x00000010
#define SEC_STS_CSR_TMO_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_STS_CSR_TMO_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_STS_CSR_TMO_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields STS_ARREADY_TMO    */
#define SEC_STS_ARREADY_TMO_WIDTH                                            1
#define SEC_STS_ARREADY_TMO_SHIFT                                            3
#define SEC_STS_ARREADY_TMO_MASK                                    0x00000008
#define SEC_STS_ARREADY_TMO_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_STS_ARREADY_TMO_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_STS_ARREADY_TMO_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields STS_RVALID_TMO    */
#define SEC_STS_RVALID_TMO_WIDTH                                            1
#define SEC_STS_RVALID_TMO_SHIFT                                            2
#define SEC_STS_RVALID_TMO_MASK                                    0x00000004
#define SEC_STS_RVALID_TMO_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_STS_RVALID_TMO_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_STS_RVALID_TMO_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields STS_AWREADY_TMO    */
#define SEC_STS_AWREADY_TMO_WIDTH                                            1
#define SEC_STS_AWREADY_TMO_SHIFT                                            1
#define SEC_STS_AWREADY_TMO_MASK                                    0x00000002
#define SEC_STS_AWREADY_TMO_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_STS_AWREADY_TMO_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_STS_AWREADY_TMO_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields STS_BVALID_TMO	 */
#define SEC_STS_BVALID_TMO_WIDTH                                                1
#define SEC_STS_BVALID_TMO_SHIFT                                                0
#define SEC_STS_BVALID_TMO_MASK                                        0x00000001
#define SEC_STS_BVALID_TMO_RD(src)                         (((src) & 0x00000001))
#define SEC_STS_BVALID_TMO_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_STS_BVALID_TMO_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	Register INT_SLV_TMOMASK 	*/
/*       Fields STS_CSR_TMOMASK    */
#define SEC_STS_CSR_TMOMASK_WIDTH                                            1
#define SEC_STS_CSR_TMOMASK_SHIFT                                            4
#define SEC_STS_CSR_TMOMASK_MASK                                    0x00000010
#define SEC_STS_CSR_TMOMASK_RD(src)                  (((src) & 0x00000010)>>4)
#define SEC_STS_CSR_TMOMASK_WR(src)             (((u32)(src)<<4) & 0x00000010)
#define SEC_STS_CSR_TMOMASK_SET(dst,src) \
                       (((dst) & ~0x00000010) | (((u32)(src)<<4) & 0x00000010))
/*       Fields STS_ARREADY_TMOMASK    */
#define SEC_STS_ARREADY_TMOMASK_WIDTH                                            1
#define SEC_STS_ARREADY_TMOMASK_SHIFT                                            3
#define SEC_STS_ARREADY_TMOMASK_MASK                                    0x00000008
#define SEC_STS_ARREADY_TMOMASK_RD(src)                  (((src) & 0x00000008)>>3)
#define SEC_STS_ARREADY_TMOMASK_WR(src)             (((u32)(src)<<3) & 0x00000008)
#define SEC_STS_ARREADY_TMOMASK_SET(dst,src) \
                       (((dst) & ~0x00000008) | (((u32)(src)<<3) & 0x00000008))
/*       Fields STS_RVALID_TMOMASK    */
#define SEC_STS_RVALID_TMOMASK_WIDTH                                            1
#define SEC_STS_RVALID_TMOMASK_SHIFT                                            2
#define SEC_STS_RVALID_TMOMASK_MASK                                    0x00000004
#define SEC_STS_RVALID_TMOMASK_RD(src)                  (((src) & 0x00000004)>>2)
#define SEC_STS_RVALID_TMOMASK_WR(src)             (((u32)(src)<<2) & 0x00000004)
#define SEC_STS_RVALID_TMOMASK_SET(dst,src) \
                       (((dst) & ~0x00000004) | (((u32)(src)<<2) & 0x00000004))
/*       Fields STS_AWREADY_TMOMASK    */
#define SEC_STS_AWREADY_TMOMASK_WIDTH                                            1
#define SEC_STS_AWREADY_TMOMASK_SHIFT                                            1
#define SEC_STS_AWREADY_TMOMASK_MASK                                    0x00000002
#define SEC_STS_AWREADY_TMOMASK_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_STS_AWREADY_TMOMASK_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_STS_AWREADY_TMOMASK_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields STS_BVALID_TMOMASK	 */
#define SEC_STS_BVALID_TMOMASK_WIDTH                                                1
#define SEC_STS_BVALID_TMOMASK_SHIFT                                                0
#define SEC_STS_BVALID_TMOMASK_MASK                                        0x00000001
#define SEC_STS_BVALID_TMOMASK_RD(src)                         (((src) & 0x00000001))
#define SEC_STS_BVALID_TMOMASK_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_STS_BVALID_TMOMASK_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	register CFG_AMA_MODE	*/
/*       Fields CFG_RD2WR_EN    */
#define SEC_CFG_RD2WR_EN_WIDTH                                            1
#define SEC_CFG_RD2WR_EN_SHIFT                                            1
#define SEC_CFG_RD2WR_EN_MASK                                    0x00000002
#define SEC_CFG_RD2WR_EN_RD(src)                  (((src) & 0x00000002)>>1)
#define SEC_CFG_RD2WR_EN_WR(src)             (((u32)(src)<<1) & 0x00000002)
#define SEC_CFG_RD2WR_EN_SET(dst,src) \
                       (((dst) & ~0x00000002) | (((u32)(src)<<1) & 0x00000002))
/*	 Fields CFG_AMA_MODE	 */
#define SEC_CFG_AMA_MODE_WIDTH                                                1
#define SEC_CFG_AMA_MODE_SHIFT                                                0
#define SEC_CFG_AMA_MODE_MASK                                        0x00000001
#define SEC_CFG_AMA_MODE_RD(src)                         (((src) & 0x00000001))
#define SEC_CFG_AMA_MODE_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_CFG_AMA_MODE_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

/*	register CFG_SLV_CSR_TMO_CNTR	*/
/*	Field CFG_CSR_TMO	*/
#define SEC_CFG_CSR_TMO_WIDTH                                                16
#define SEC_CFG_CSR_TMO_SHIFT                                                0
#define SEC_CFG_CSR_TMO_MASK                                        0x0000ffff
#define SEC_CFG_CSR_TMO_RD(src)                         (((src) & 0x0000ffff))
#define SEC_CFG_CSR_TMO_WR(src)                    (((u32)(src)) & 0x0000ffff)
#define SEC_CFG_CSR_TMO_SET(dst,src) \
                          (((dst) & ~0x0000ffff) | (((u32)(src)) & 0x0000ffff))

/*	Register CFG_MASK_DEV_ERR_RESP	*/
/*	 Fields MASK_DEV_ERR_RESP	 */
#define SEC_MASK_DEV_ERR_RESP_WIDTH                                                1
#define SEC_MASK_DEV_ERR_RESP_SHIFT                                                0
#define SEC_MASK_DEV_ERR_RESP_MASK                                        0x00000001
#define SEC_MASK_DEV_ERR_RESP_RD(src)                         (((src) & 0x00000001))
#define SEC_MASK_DEV_ERR_RESP_WR(src)                    (((u32)(src)) & 0x00000001)
#define SEC_MASK_DEV_ERR_RESP_SET(dst,src) \
                          (((dst) & ~0x00000001) | (((u32)(src)) & 0x00000001))

#endif /*_APM_SEC_CSR_H__*/
