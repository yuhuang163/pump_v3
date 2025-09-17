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
#ifndef __REG_CRC_H__
#define __REG_CRC_H__

#include "plat_types.h"

struct CRC_T {
    __IO uint32_t DMA_REQ; //0x00
    __IO uint32_t DMA_ADDR;//0x04
    __IO uint32_t DMA_LEN_BST;//0x08
    __IO uint32_t DMA_CFG;//0x0c
    __IO uint32_t CRC_CFG;//0x10
    __IO uint32_t CRC_INIT_VAL;//0x14
    __IO uint32_t CRC_POLY;//0x18
    __IO uint32_t RSVED;//0x1c
    __IO uint32_t CRC_RES;//0x20
    __I  uint32_t DMA_ADDR_ERR;//0X24;
    __I  uint32_t DMA_STATUS;//0x28;
};

// reg_000
#define CRC_DMA_EN                                          (1 << 0)

// reg_004
#define CRC_SRC_ADDR(n)                                     (((n) & 0xFFFFFFFF) << 0)
#define CRC_SRC_ADDR_MASK                                   (0xFFFFFFFF << 0)
#define CRC_SRC_ADDR_SHIFT                                  (0)

// reg_008
#define CRC_DMA_LEN(n)                                      (((n) & 0x1FFF) << 0)
#define CRC_DMA_LEN_MASK                                    (0x1FFF << 0)
#define CRC_DMA_LEN_SHIFT                                   (0)
#define CRC_DMA_BST(n)                                      (((n) & 0xF) << 13)
#define CRC_DMA_BST_MASK                                    (0xF << 13)
#define CRC_DMA_BST_SHIFT                                   (13)

// reg_00c
#define CRC_DMA_BIGD                                        (1 << 0)
#define CRC_DMA_BST_SINGLE                                  (1 << 1)
#define CRC_DMA_RD                                          (1 << 2)
#define CRC_DMA_RST                                         (1 << 3)

// reg_010
#define CRC_INTR_MASK                                       (1 << 0)
#define CRC_INTR_CLEAR                                      (1 << 1)
#define CRC_CRC_INI                                         (1 << 2)
#define CRC_CRC16_32_SHIFT                                  (3)
#define CRC_CRC16_32_MASK                                   (1 << 3)
#define CRC_DMA_TRANS                                       (1 << 4)
#define CRC_CRC_REFOUT_SHIFT                                (5)
#define CRC_CRC_REFOUT_MASK                                 (1 << 5)
#define CRC_CRC_REFIN_SHIFT                                 (6)
#define CRC_CRC_REFIN_MASK                                  (1 << 6)
#define CRC_CRC_RESULT_XOR_SHIFT                            (7)
#define CRC_CRC_RESULT_XOR_MASK                             (1 << 7)

// reg_014
#define CRC_CRC_INIVAL(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define CRC_CRC_INIVAL_MASK                                 (0xFFFFFFFF << 0)
#define CRC_CRC_INIVAL_SHIFT                                (0)

// reg_018
#define CRC_CRC_POLY(n)                                     (((n) & 0xFFFFFFFF) << 0)
#define CRC_CRC_POLY_MASK                                   (0xFFFFFFFF << 0)
#define CRC_CRC_POLY_SHIFT                                  (0)

// reg_020
#define CRC_CRC_RES(n)                                      (((n) & 0xFFFFFFFF) << 0)
#define CRC_CRC_RES_MASK                                    (0xFFFFFFFF << 0)
#define CRC_CRC_RES_SHIFT                                   (0)

// reg_024
#define CRC_DMA_ERR_ADDR(n)                                 (((n) & 0xFFFFFFFF) << 0)
#define CRC_DMA_ERR_ADDR_MASK                               (0xFFFFFFFF << 0)
#define CRC_DMA_ERR_ADDR_SHIFT                              (0)

// reg_028
#define CRC_DMA_TRANS_DONE                                  (1 << 0)
#define CRC_DMA_NOWORK                                      (1 << 1)
#define CRC_DMA_TRANS_ERR                                   (1 << 2)
#define CRC_DMA_DONE_INTR                                   (1 << 3)
#define CRC_CRC_BUSY                                        (1 << 4)
#define CRC_CRC_WORKING                                     (1 << 5)
#define CRC_CRC_REQING                                      (1 << 6)
#endif
