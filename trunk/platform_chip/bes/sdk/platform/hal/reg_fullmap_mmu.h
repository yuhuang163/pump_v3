/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __REG_FULLMAP_MMU_H__
#define __REG_FULLMAP_MMU_H__

#include "plat_types.h"

struct FULLMAP_MMU_T {
    __IO uint32_t CONTROL;                  //0x00
    __I  uint32_t IRQ_STATE;                //0x04
    __IO uint32_t IRQ_CLEAR;                //0x08
    __IO uint32_t IRQ_ENABLE;               //0x0C
    __I  uint32_t IRQ_INFO1_WRITE;          //0x10
    __I  uint32_t IRQ_INFO2_WRITE;          //0x14
    __I  uint32_t IRQ_INFO1_READ;           //0x18
    __I  uint32_t IRQ_INFO2_READ;           //0x1C
    __IO uint32_t IRQ_SET;                  //0x20
    __I  uint32_t TRAN_CNT_WRITE;           //0x24
    __I  uint32_t TRAN_CNT_READ;            //0x28
    __IO uint32_t READ_ACCESS_BIT_CLEAR;    //0x2C
    __I  uint32_t RESERVED0[500];           //0x30~0x7FC
    __I  uint32_t READ_ACCESS_BIT[512];     //0x800~0xFFC
    __I  uint32_t RESERVED1[7168];          //0x1000~0x7FFC
    __IO uint16_t LOOK_UP_TABLE[16384];     //0x8000~0xFFFC
};

// reg_000
#define MMU_LUT_LOWER_INIT_DONE                             (1 << 0)
#define MMU_LUT_UPPER_INIT_DONE                             (1 << 1)
#define MMU_LUT_OVERPROVISION                               (1 << 2)
#define MMU_LUT_UPPER_HALF                                  (1 << 3)
#define MMU_RESP_CFG                                        (1 << 4)
#define MMU_RESERVED_00_5(n)                                (((n) & 0x7) << 5)
#define MMU_RESERVED_00_5_MASK                              (0x7 << 5)
#define MMU_RESERVED_00_5_SHIFT                             (5)
#define MMU_ALL_CHECK_BYPASS                                (1 << 8)
#define MMU_WR_CHECK_BYPASS                                 (1 << 9)
#define MMU_RD_CHECK_BYPASS                                 (1 << 10)
#define MMU_KEEP_FIRST_INFO                                 (1 << 11)
#define MMU_RESERVED_00_12(n)                               (((n) & 0xF) << 12)
#define MMU_RESERVED_00_12_MASK                             (0xF << 12)
#define MMU_RESERVED_00_12_SHIFT                            (12)
#define MMU_MMU_CFG_LOCK                                    (1 << 16)
#define MMU_RESERVED_00_17(n)                               (((n) & 0x3FF) << 17)
#define MMU_RESERVED_00_17_MASK                             (0x3FF << 17)
#define MMU_RESERVED_00_17_SHIFT                            (17)
#define MMU_MODE_HCLK_ACCESS                                (1 << 27)
#define MMU_BYPASS_LUT_CG                                   (1 << 28)
#define MMU_RESERVED_00_29(n)                               (((n) & 0x7) << 29)
#define MMU_RESERVED_00_29_MASK                             (0x7 << 29)
#define MMU_RESERVED_00_29_SHIFT                            (29)

// reg_004
#define MMU_IRQ_W                                           (1 << 0)
#define MMU_IRQ_R                                           (1 << 1)

// reg_008
#define MMU_CLEAR_IRQ_W                                     (1 << 0)
#define MMU_CLEAR_IRQ_R                                     (1 << 1)

// reg_00c
#define MMU_ENABLE_IRQ_W                                    (1 << 0)
#define MMU_ENABLE_IRQ_R                                    (1 << 1)

// reg_010
#define MMU_IRQ_W_INFO_ADDR(n)                              (((n) & 0x3FFFFFF) << 0)
#define MMU_IRQ_W_INFO_ADDR_MASK                            (0x3FFFFFF << 0)
#define MMU_IRQ_W_INFO_ADDR_SHIFT                           (0)

// reg_014
#define MMU_IRQ_W_INFO_ID(n)                                (((n) & 0x7FFF) << 0)
#define MMU_IRQ_W_INFO_ID_MASK                              (0x7FFF << 0)
#define MMU_IRQ_W_INFO_ID_SHIFT                             (0)
#define MMU_IRQ_W_INFO_W                                    (1 << 15)

// reg_018
#define MMU_IRQ_R_INFO_ADDR(n)                              (((n) & 0x3FFFFFF) << 0)
#define MMU_IRQ_R_INFO_ADDR_MASK                            (0x3FFFFFF << 0)
#define MMU_IRQ_R_INFO_ADDR_SHIFT                           (0)

// reg_01c
#define MMU_IRQ_R_INFO_ID(n)                                (((n) & 0x7FFF) << 0)
#define MMU_IRQ_R_INFO_ID_MASK                              (0x7FFF << 0)
#define MMU_IRQ_R_INFO_ID_SHIFT                             (0)
#define MMU_IRQ_R_INFO_R                                    (1 << 15)

// reg_020
#define MMU_IRQ_SET                                         (1 << 0)

// reg_024
#define MMU_AW_CNTR(n)                                      (((n) & 0x1F) << 0)
#define MMU_AW_CNTR_MASK                                    (0x1F << 0)
#define MMU_AW_CNTR_SHIFT                                   (0)
#define MMU_W_CNTR(n)                                       (((n) & 0x1F) << 5)
#define MMU_W_CNTR_MASK                                     (0x1F << 5)
#define MMU_W_CNTR_SHIFT                                    (5)

// reg_028
#define MMU_AR_CNTR(n)                                      (((n) & 0x1F) << 0)
#define MMU_AR_CNTR_MASK                                    (0x1F << 0)
#define MMU_AR_CNTR_SHIFT                                   (0)

// reg_02c
#define MMU_READ_ACC_GLOBAL_CLEAR                           (1 << 0)

// reg_800
#define MMU_READ_ACCESS(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define MMU_READ_ACCESS_MASK                                (0xFFFFFFFF << 0)
#define MMU_READ_ACCESS_SHIFT                               (0)

// reg_8000
#define MMU_PHYSICAL_PAGE_NUM_0(n)                          (((n) & 0x3FFF) << 0)
#define MMU_PHYSICAL_PAGE_NUM_0_MASK                        (0x3FFF << 0)
#define MMU_PHYSICAL_PAGE_NUM_0_SHIFT                       (0)
#define MMU_RD_VALID_0                                      (1 << 14)
#define MMU_WR_VALID_0                                      (1 << 15)
#define MMU_PHYSICAL_PAGE_NUM_1(n)                          (((n) & 0x3FFF) << 16)
#define MMU_PHYSICAL_PAGE_NUM_1_MASK                        (0x3FFF << 16)
#define MMU_PHYSICAL_PAGE_NUM_1_SHIFT                       (16)
#define MMU_RD_VALID_1                                      (1 << 30)
#define MMU_WR_VALID_1                                      (1 << 31)

#endif
