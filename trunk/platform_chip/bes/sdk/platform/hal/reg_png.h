/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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
#ifndef __PNG_REG_H__
#define __PNG_REG_H__

#include "plat_types.h"

struct PNG_DEC_REG_T {
    __IO uint32_t REG_00;
    __IO uint32_t REG_04;
    __IO uint32_t REG_08;
    __IO uint32_t REG_0C;
    __IO uint32_t REG_10;
    __IO uint32_t REG_14;
    __IO uint32_t REG_18;
    __IO uint32_t REG_1C;
    __IO uint32_t REG_20;
    __IO uint32_t REG_24;
    __IO uint32_t REG_28;
    __IO uint32_t REG_2C;
};


// REG_00
#define PNG_DEC_START                       (1 << 0)
#define PNG_DEC_BURST_0TO3(n)               (((n) & 0x3) << 1) /* 0: 64, 1: 128 */
#define PNG_DEC_BURST_0TO3_MASK             (0x3 << 1)
#define PNG_DEC_BURST_0TO3_SHIFT            (1)

// REG_04
#define PNG_DEC_SRC_DATA_ADDR(n)            (((n) & 0xffffffff))
#define PNG_DEC_SRC_DATA_ADDR_MASK          (0xffffffff)
#define PNG_DEC_SRC_DATA_ADDR_SHIFT         (0)

// REG_08
#define PNG_DEC_DMA_ENA                     (1 << 0)
#define PNG_DEC_DMA_PIX_RGB565              (1 << 1)
#define PNG_DEC_DMA_PIX_RGBA565             (1 << 2)
#define PNG_DEC_DMA_PIX_ARGB565             (1 << 3)
#define PNG_DEC_DMA_PIX_FMT(n)              (((n) & 0x3) << 4) /* 0: 16bpp, RGB565,
                                                                * 1: 24bpp, RGB888/RGBA565/ARGB565,
                                                                * 2: 32bpp, ARGB888,
                                                                * 3: 32bpp, RGBA888,
                                                                */
#define PNG_DEC_DMA_PIX_FMT_MASK            (0x3 << 4)
#define PNG_DEC_DMA_PIX_FMT_SHIFT           (4)
#define PNG_DEC_DMA_BURST_LEN(n)            (((n) & 0x1f) << 8) /* 8: 128, 4: 64 */
#define PNG_DEC_DMA_BURST_LEN_MASK          (0x1f << 8)
#define PNG_DEC_DMA_BURST_LEN_SHIFT         (8)
#define PNG_DEC_DMA_IMG_PITCH(n)            (((n) & 0xffff) << 16)
#define PNG_DEC_DMA_IMG_PITCH_MASK          (0xffff << 16)
#define PNG_DEC_DMA_IMG_PITCH_SHIFT         (16)

// REG_0C
#define PNG_DEC_PIX_DATA_ADDR(n)            (((n) & 0xffffffff))
#define PNG_DEC_PIX_DATA_ADDR_MASK          (0xffffffff)
#define PNG_DEC_PIX_DATA_ADDR_SHIFT         (0)

// REG_10
#define PNG_DEC_USER_ID(n)                  (((n) & 0xf) << 0)
#define PNG_DEC_USER_ID_MASK                (0xf << 0)
#define PNG_DEC_USER_ID_SHIFT               (0)
#define PNG_DEC_AWQOS(n)                    (((n) & 0xf) << 4)
#define PNG_DEC_AWQOS_MASK                  (0xf << 4)
#define PNG_DEC_AWQOS_SHIFT                 (4)
#define PNG_DEC_ARQOS(n)                    (((n) & 0xf) << 8)
#define PNG_DEC_ARQOS_MASK                  (0xf << 8)
#define PNG_DEC_ARQOS_SHIFT                 (8)
#define PNG_DEC_MAX_REQ_NUM(n)              (((n) & 0xf) << 12)
#define PNG_DEC_MAX_REQ_NUM_MASK            (0xf << 12)
#define PNG_DEC_MAX_REQ_NUM_SHIFT           (12)
#define PNG_DEC_POSTWR_EN                   (1 << 15)
#define PNG_DEC_AXI_SEC                     (1 << 16)
#define PNG_DEC_WR_INT_CLR                  (1 << 17)
#define PNG_DEC_RST_REQ                     (1 << 18)
#define PNG_DEC_RST_N_PWR                   (1 << 19)
#define PNG_DEC_GLOBAL_ALPHA_EN             (1 << 23)
#define PNG_DEC_GLOBAL_ALPHA(n)             (((n) & 0xff) << 24)
#define PNG_DEC_GLOBAL_ALPHA_MASK           (0xff << 24)
#define PNG_DEC_GLOBAL_ALPHA_SHIFT          (24)

// REG_14/18/1C
#define PNG_DEC_IRQ_IHDR_READ_DONE          (1 << 8)
#define PNG_DEC_IRQ_DECODE_DONE             (1 << 9)
#define PNG_DEC_IRQ_TRIG_LINE               (1 << 10)
#define PNG_DEC_IRQ_DMA_WR_ERR              (1 << 11)
#define PNG_DEC_IRQ_COMP_MODE_ERR           (1 << 12)
#define PNG_DEC_IRQ_DECODE_ERR              (1 << 13)

// REG_20
#define PNG_DEC_IMG_HEIGHT(n)               (((n) & 0x1ff) << 0)
#define PNG_DEC_IMG_HEIGHT_MASK             (0x1ff << 0)
#define PNG_DEC_IMG_HEIGHT_SHIFT            (0)
#define PNG_DEC_IMG_WIDTH(n)                (((n) & 0x1ff) << 9)
#define PNG_DEC_IMG_WIDTH_MASK              (0x1ff << 9)
#define PNG_DEC_IMG_WIDTH_SHIFT             (9)
#define PNG_DEC_IMG_COLOR_TYPE(n)           (((n) & 0x7) << 18) /* 0 - Gray,
                                                                 * 2 - RGB888,
                                                                 * 3 - Index,
                                                                 * 4 - Gray+Alpha,
                                                                 * 6 - RGBA8888
                                                                 */
#define PNG_DEC_IMG_COLOR_TYPE_MASK         (0x7 << 18)
#define PNG_DEC_IMG_COLOR_TYPE_SHIFT        (18)
#define PNG_DEC_IMG_COLOR_DEPTH(n)          (((n) & 0xff) << 21) /* Valid set:
                                                                  * Gray:         1/2/4/8/16,
                                                                  * RGB888:       8/16,
                                                                  * Index:        1/2/4/8,
                                                                  * Gray+Alpha:   8/16,
                                                                  * RGBA8888:     8/16,
                                                                  */
#define PNG_DEC_IMG_COLOR_DEPTH_MASK        (0xff << 21)
#define PNG_DEC_IMG_COLOR_DEPTH_SHIFT       (21)
#define PNG_DEC_IMG_COMP_MODE(n)            (((n) & 0x7) << 29) /* 0 - deflate */
#define PNG_DEC_IMG_COMP_MODE_MASK          (0x7 << 29)
#define PNG_DEC_IMG_COMP_MODE_SHIFT         (29)

// REG_24
#define PNG_DEC_TRIG_LINES(n)               (((n) & 0x1ff) << 0)
#define PNG_DEC_TRIG_LINES_MASK             (0x1ff << 0)
#define PNG_DEC_TRIG_LINES_SHIFT            (0)

// REG_28
#define PNG_DEC_SRC_DATA_SIZE(n)            (((n) & 0xffffffff) << 0)
#define PNG_DEC_SRC_DATA_SIZE_MASK          (0xffffffff << 0)
#define PNG_DEC_SRC_DATA_SIZE_SHIFT         (0)

// REG_2C
#define PNG_DEC_PIX_SIZE_SET                (1 << 0)
#define PNG_DEC_PIX_WIDTH(n)                (((n) & 0x1ff) << 4)
#define PNG_DEC_PIX_WIDTH_MASK              (0x1ff << 4)
#define PNG_DEC_PIX_WIDTH_SHIFT             (4)
#define PNG_DEC_PIX_HEIGHT(n)               (((n) & 0x1ff) << 20)
#define PNG_DEC_PIX_HEIGHT_MASK             (0x1ff << 20)
#define PNG_DEC_PIX_HEIGHT_SHIFT            (20)

#endif /* __PNG_REG_H__ */
