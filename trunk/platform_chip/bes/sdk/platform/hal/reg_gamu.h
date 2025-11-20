/***************************************************************************
 *
 * Copyright 2015-2023 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#ifndef __REG_GAMU_H__
#define __REG_GAMU_H__

#include "plat_types.h"

struct GAMU_REG_T {
    __IO uint32_t REG_LUT[512];             /* LUT entry 0 ~ 511 */
    __IO uint32_t REG_PADDR[4];             /* Physical address for buffers */
    __IO uint32_t REG_810;
    __IO uint32_t REG_814;
    __IO uint32_t REG_PADDR_WR[4];          /* Physical address for buffers */
    __IO uint32_t REG_828;
    __IO uint32_t REG_82C;
    __IO uint32_t REG_830;
    __IO uint32_t REG_834;
    __IO uint32_t REG_838;
    __IO uint32_t REG_83C;
};

// REG_000 ~ 7FC
#define GAMU_LUT_BLOCK_START(n)                     (((n) & 0x7f) << 0)
#define GAMU_LUT_BLOCK_START_MASK                   (0x7f)
#define GAMU_LUT_BLOCK_START_SHIFT                  (0)
#define GAMU_LUT_BLOCK_END(n)                       (((n) & 0x7f) << 7)
#define GAMU_LUT_BLOCK_END_MASK                     (0x7f << 7)
#define GAMU_LUT_BLOCK_END_SHIFT                    (7)
#define GAMU_LUT_LINE_ACTIVE                        (1 << 15)
#define GAMU_LUT_BLOCK_OFFSET(n)                    (((n) & 0xffff) << 16)
#define GAMU_LUT_BLOCK_OFFSET_MASK                  (0xffff << 16)
#define GAMU_LUT_BLOCK_OFFSET_SHIFT                 (16)

// REG_800 ~ 80C
#define GAMU_BUF_PHY_ADDR(n)                        (((n) & 0xffffffff) << 0)
#define GAMU_BUF_PHY_ADDR_MASK                      (0xffffffff)
#define GAMU_BUF_PHY_ADDR_SHIFT                     (0)

// REG_810
#define GAMU_PHY_SEL(n)                             (((n) & 0x3) << 0)
#define GAMU_PHY_SEL_MASK                           (0x3 << 0)
#define GAMU_PHY_SEL_SHIFT                          (0)
#define GAMU_PHY_SEL_EN                             (1 << 2)
#define GAMU_RGB_INPUT_FORMAT(n)                    (((n) & 0x3) << 3) /* 0: RGBA8888, 32bits;
                                                                        * 3: RGB888, 24bits;
                                                                        * 2: RGB565, 16bites;
                                                                        */
#define GAMU_RGB_INPUT_FORMAT_MASK                  (0x3 << 3)
#define GAMU_RGB_INPUT_FORMAT_SHIFT                 (3)
#define GAMU_VVM_EN                                 (1 << 5)    /* always 1 */
#define GAMU_CROSS_1K_EN                            (1 << 6)    /* always 1 */
#define GAMU_SPLIT_1TH_RLEN(n)                      (((n) & 0xff) << 8)
#define GAMU_SPLIT_1TH_RLEN_MASK                    (0xff << 8)
#define GAMU_SPLIT_1TH_RLEN_SHIFT                   (8)
#define GAMU_FIX_SPLIT_RLEN                         (1 << 16)

// REG_814
#define GAMU_UNVALID_HADDR(n)                       (((n) & 0xffffffff) << 0)
#define GAMU_UNVALID_HADDR_MASK                     (0xffffffff)
#define GAMU_UNVALID_HADDR_SHIFT                    (0)

// REG_818 ~ 824
#define GAMU_BUF_PHY_ADDR_WR(n)                     (((n) & 0xffffffff) << 0)
#define GAMU_BUF_PHY_ADDR_WR_MASK                   (0xffffffff)
#define GAMU_BUF_PHY_ADDR_WR_SHIFT                  (0)

// REG_828
#define GAMU_PHY_SEL_WR(n)                          (((n) & 0x3) << 0)
#define GAMU_PHY_SEL_WR_MASK                        (0x3 << 0)
#define GAMU_PHY_SEL_WR_SHIFT                       (0)
#define GAMU_PHY_SEL_WR_EN                          (1 << 2)
#define GAMU_RGB_INPUT_FORMAT_WR(n)                 (((n) & 0x3) << 3)
#define GAMU_RGB_INPUT_FORMAT_WR_MASK               (0x3 << 3)
#define GAMU_RGB_INPUT_FORMAT_WR_SHIFT              (3)
#define GAMU_VVM_EN_WR                              (1 << 5)
#define GAMU_VMM_MEM_CE_SEL                         (1 << 6)
#define GAMU_LINE_ACTIVE_MASK                       (1 << 7)
#define GAMU_SPLIT_1TH_WLEN(n)                      (((n) & 0xff) << 8)
#define GAMU_SPLIT_1TH_WLEN_MASK                    (0xff << 8)
#define GAMU_SPLIT_1TH_WLEN_SHIFT                   (8)
#define GAMU_FIX_SPLIT_WLEN                         (1 << 16)

// REG_82C
#define GAMU_RDATA_FAKE(n)                          (((n) & 0xffffffff) << 0)
#define GAMU_RDATA_FAKE_MASK                        (0xffffffff)
#define GAMU_RDATA_FAKE_SHIFT                       (0)

// REG_830
#define GAMU_UNVALID_ADDR_WR(n)                     (((n) & 0xffffffff) << 0)
#define GAMU_UNVALID_ADDR_WR_MASK                   (0xffffffff)
#define GAMU_UNVALID_ADDR_WR_SHIFT                  (0)

// REG_834
#define GAMU_MMU_ADD_ADDR_MODE                      (1 << 0)
#define GAMU_MMU_CHOOSE_ADDR_0                      (1 << 1)
#define GAMU_MMU_CHOOSE_ADDR_1                      (1 << 2)

// REG_838
#define GAMU_MMU_ADD_ADDR_0(n)                      (((n) & 0xffffffff) << 0)
#define GAMU_MMU_ADD_ADDR_0_MASK                    (0xffffffff)
#define GAMU_MMU_ADD_ADDR_0_SHIFT                   (0)

// REG_83C
#define GAMU_MMU_ADD_ADDR_1(n)                      (((n) & 0xffffffff) << 0)
#define GAMU_MMU_ADD_ADDR_1_MASK                    (0xffffffff)
#define GAMU_MMU_ADD_ADDR_1_SHIFT                   (0)

#endif /* __REG_GAMU_H__ */
