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
#ifndef __REG_CHK_H__
#define __REG_CHK_H__

#include "plat_types.h"

struct CHK_T {
    __IO uint32_t DMA_REQ;              //0x00
    __IO uint32_t DMA_ADDR;             //0x04
    __IO uint32_t DMA_LEN_BST;          //0x08
    __IO uint32_t DMA_CFG;              //0x0c
    __IO uint32_t CHK_CFG;              //0x10
    __IO uint32_t CHK_HEADER_LEN_PRO;   //0x14
    __IO uint32_t CHK_HEADER_SRC;       //0x18
    __IO uint32_t CHK_HEADER_DST;       //0x1c
    __IO uint32_t CHK_RES;              //0x20
    __I  uint32_t DMA_ERR;              //0X24
    __I  uint32_t DMA_STATUS;           //0x28
};

// reg_000
#define CHK_DMA_EN                                          (1 << 0)

// reg_004
#define CHK_SRC_ADDR(n)                                     (((n) & 0xFFFFFFFF) << 0)
#define CHK_SRC_ADDR_MASK                                   (0xFFFFFFFF << 0)
#define CHK_SRC_ADDR_SHIFT                                  (0)

// reg_008
#define CHK_DMA_LEN(n)                                      (((n) & 0x1FFF) << 0)
#define CHK_DMA_LEN_MASK                                    (0x1FFF << 0)
#define CHK_DMA_LEN_SHIFT                                   (0)
#define CHK_DMA_BST(n)                                      (((n) & 0xF) << 13)
#define CHK_DMA_BST_MASK                                    (0xF << 13)
#define CHK_DMA_BST_SHIFT                                   (13)

// reg_00c
#define CHK_DMA_BIGD                                        (1 << 0)
#define CHK_DMA_BST_SINGLE                                  (1 << 1)
#define CHK_DMA_RD                                          (1 << 2)
#define CHK_DMA_RST                                         (1 << 3)

// reg_010
#define CHK_INTR_MASK                                       (1 << 0)
#define CHK_INTR_CLEAR                                      (1 << 1)
#define CHK_CHK_CLEAR                                       (1 << 2)
#define CHK_CHK_BIGD                                        (1 << 3)
#define CHK_CHK_MODE                                        (1 << 4)
#define CHK_CHK_HEADER_BIGD                                 (1 << 5)

// reg_014
#define CHK_HEADER_LENGTH(n)                                (((n) & 0xFFFF) << 0)
#define CHK_HEADER_LENGTH_MASK                              (0xFFFF << 0)
#define CHK_HEADER_LENGTH_SHIFT                             (0)
#define CHK_HEADER_PROTOCOL(n)                              (((n) & 0xFFFF) << 16)
#define CHK_HEADER_PROTOCOL_MASK                            (0xFFFF << 16)
#define CHK_HEADER_PROTOCOL_SHIFT                           (16)

// reg_018
#define CHK_HEADER_SRC(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define CHK_HEADER_SRC_MASK                                 (0xFFFFFFFF << 0)
#define CHK_HEADER_SRC_SHIFT                                (0)

// reg_01c
#define CHK_HEADER_DST(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define CHK_HEADER_DST_MASK                                 (0xFFFFFFFF << 0)
#define CHK_HEADER_DST_SHIFT                                (0)

// reg_020
#define CHK_CHK_RES(n)                                      (((n) & 0xFFFFFFFF) << 0)
#define CHK_CHK_RES_MASK                                    (0xFFFFFFFF << 0)
#define CHK_CHK_RES_SHIFT                                   (0)

// reg_024
#define CHK_DMA_ERR_ADDR(n)                                 (((n) & 0xFFFFFFFF) << 0)
#define CHK_DMA_ERR_ADDR_MASK                               (0xFFFFFFFF << 0)
#define CHK_DMA_ERR_ADDR_SHIFT                              (0)

// reg_028
#define CHK_DMA_TRANS_DONE                                  (1 << 0)
#define CHK_DMA_NOWORK                                      (1 << 1)
#define CHK_DMA_TRANS_ERR                                   (1 << 2)
#define CHK_DMA_DONE_INTR                                   (1 << 3)
#define CHK_DMA_BUSY                                        (1 << 4)
#define CHK_CHK_WORKING                                     (1 << 5)
#define CHK_CHK_REQING                                      (1 << 6)

#endif
