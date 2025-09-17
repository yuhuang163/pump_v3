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
#ifndef __JPEG_REG_H__
#define __JPEG_REG_H__

#include "plat_types.h"

struct JPEG_REG_T {
    __IO uint32_t REG_RES_00[1];
    __IO uint32_t REG_04;                         //Control
    __IO uint32_t REG_08;                         //Pixel transfer start
    __IO uint32_t REG_0C;                         //ECS transfer start
    __IO uint32_t REG_10;                         //Pixel data length
    __IO uint32_t REG_14;                         //ECS data length, only valid in decoding
    __IO uint32_t REG_18;                         //Total length of incoming data
    __IO uint32_t REG_1C;                         //ECS output data length, only valid in encoding
    __IO uint32_t REG_20;                         //Dummy bytes
    __IO uint32_t REG_RES_24_3C[7];
    __IO uint32_t REG_40;                         //Interrupt status
    __IO uint32_t REG_44;                         //Interrupt mask
    __IO uint32_t REG_RES_46_5C[6];
    __IO uint32_t REG_60;                         //Scaling & fake mode setting
    __IO uint32_t REG_RES_64_6C[3];
    __IO uint32_t REG_70;                         //Picture decoding width & height
    __IO uint32_t REG_74;                         //Picture encoding width & height
    __IO uint32_t REG_RES_78_80[3];
    __IO uint32_t REG_84;                         //Threshold
    __IO uint32_t REG_RES_88_0FFC[990];
    __IO uint32_t REG_1000;                       //Codec reg_0
    __IO uint32_t REG_1004;                       //Codec reg_1
    __IO uint32_t REG_1008;                       //Codec reg_2
    __IO uint32_t REG_100C;                       //Codec reg_3
    __IO uint32_t REG_1010;                       //Codec reg_4
    __IO uint32_t REG_1014;                       //Codec reg_5
    __IO uint32_t REG_1018;                       //Codec reg_6
    __IO uint32_t REG_101C;                       //Codec reg_7
};


// REG_04
#define JPEG_DEC_MODE                               (1 << 0)
#define JPEG_ENC_MODE                               (0 << 0)
#define JPEG_DEC_FORMAT(n)                          (((n) & 0x3) << 1) /* 0: YUV422, 1: YUV420, 2: YUV444, 3: YUV411 */
#define JPEG_DEC_FORMAT_MASK                        (0x3 << 1)
#define JPEG_DEC_FORMAT_SHIFT                       (1)
#define JPEG_DEC_SWAP_YU                            (1 << 3)
#define JPEG_DEC_SWAP_UV                            (1 << 4)
#define JPEG_SW_RESET                               (1 << 5)
#define JPEG_ENC_SWAP_YU                            (1 << 6)
#define JPEG_ENC_SWAP_UV                            (1 << 7)

// REG_08
#define JPEG_PIX_XFER_START                         (1 << 0)

// REG_0C
#define JPEG_ECS_XFER_START                         (1 << 0)

// REG_10
#define JPEG_PIX_XFER_LEN(n)                        (((n) & 0xffffffff) << 0)
#define JPEG_PIX_XFER_LEN_MASK                      (0xffffffff << 0)
#define JPEG_PIX_XFER_LEN_SHIFT                     (0)

// REG_14
#define JPEG_ECS_XFER_LEN(n)                        (((n) & 0xffffffff) << 0)
#define JPEG_ECS_XFER_LEN_MASK                      (0xffffffff << 0)
#define JPEG_ECS_XFER_LEN_SHIFT                     (0)

// REG_18
#define JPEG_TOTAL_INCOM_LEN(n)                     (((n) & 0xffffffff) << 0)
#define JPEG_TOTAL_INCOM_LEN_MASK                   (0xffffffff << 0)
#define JPEG_TOTAL_INCOM_LEN_SHIFT                  (0)

// REG_1C
#define JPEG_ECS_OUTPUT_LEN(n)                      (((n) & 0xffffff) << 0)
#define JPEG_ECS_OUTPUT_LEN_MASK                    (0xffffff << 0)
#define JPEG_ECS_OUTPUT_LEN_SHIFT                   (0)

// REG_20
#define JPEG_DUMMY_MODE(n)                          (((n) & 0x3) << 0)
#define JPEG_DUMMY_MODE_MASK                        (0x3 << 0)
#define JPEG_DUMMY_MODE_SHIFT                       (0)

// REG_40
#define JPEG_INTR_CODEC_DONE                        (1 << 0)
#define JPEG_INTR_LINE_BUF_ACCESS                   (1 << 1)
#define JPEG_INTR_PIX_XFER_DONE                     (1 << 2)
#define JPEG_INTR_ECS_XFER_DONE                     (1 << 3)

// REG_44
#define JPEG_INTR_CODEC_DONE_MASK                   (1 << 0)
#define JPEG_INTR_LINE_BUF_ACCESS_MASK              (1 << 1)
#define JPEG_INTR_PIX_XFER_DONE_MASK                (1 << 2)
#define JPEG_INTR_ECS_XFER_DONE_MASK                (1 << 3)

// REG_60
#define JPEG_SCALE_MODE_1_8                         (1 << 0) // Scaling 1/8
#define JPEG_SCALE_MODE_1_4                         (0 << 0) // Scaling 1/4
#define JPEG_SCALE_MODE_EN                          (1 << 1)
#define JPEG_FAKE_MODE_EN                           (1 << 2)
#define JPEG_FIX_BUG_EN                             (1 << 3)

// REG_70
#define JPEG_DEC_PIC_WIDTH(n)                       (((n) & 0xfff) << 0)
#define JPEG_DEC_PIC_WIDTH_MASK                     (0xfff << 0)
#define JPEG_DEC_PIC_WIDTH_SHIFT                    (0)
#define JPEG_DEC_PIC_HEIGHT(n)                      (((n) & 0xfff) << 12)
#define JPEG_DEC_PIC_HEIGHT_MASK                    (0xfff << 12)
#define JPEG_DEC_PIC_HEIGHT_SHIFT                   (12)

// REG_74
#define JPEG_ENC_PIC_WIDTH(n)                       (((n) & 0xfff) << 0)
#define JPEG_ENC_PIC_WIDTH_MASK                     (0xfff << 0)
#define JPEG_ENC_PIC_WIDTH_SHIFT                    (0)
#define JPEG_ENC_PIC_HEIGHT(n)                      (((n) & 0xfff) << 12)
#define JPEG_ENC_PIC_HEIGHT_MASK                    (0xfff << 12)
#define JPEG_ENC_PIC_HEIGHT_SHIFT                   (12)
#define JPEG_ENC_XMODE_EN                           (1 << 24)

// REG_84
#define JPEG_PIX_REQ_THRESHOLD(n)                   (((n) & 0xf) << 0)
#define JPEG_PIX_REQ_THRESHOLD_MASK                 (0xf << 0)
#define JPEG_PIX_REQ_THRESHOLD_SHIFT                (0)
#define JPEG_ECS_REQ_THRESHOLD(n)                   (((n) & 0xf) << 4)
#define JPEG_ECS_REQ_THRESHOLD_MASK                 (0xf << 4)
#define JPEG_ECS_REQ_THRESHOLD_SHIFT                (4)

/*****************************************************************************/

// REG_1000
#define JPEG_CODEC_START                            (1 << 0)

// REG_1004
#define JPEG_CODEC_NCOL(n)                          (((n) & 0x3) << 0) // Color num - 1, RGB/YUV:2
#define JPEG_CODEC_NCOL_MASK                        (0x3 << 0)
#define JPEG_CODEC_NCOL_SHIFT                       (0)
#define JPEG_CODEC_RESTART_MARKER_EN                (1 << 2)
#define JPEG_CODEC_DEC_MODE                         (1 << 3)
#define JPEG_CODEC_ENC_MODE                         (0 << 3)

// REG_1008
#define JPEG_CODEC_NMCU(n)                          (((n) & 0x3ffffff) << 0)
#define JPEG_CODEC_NMCU_MASK                        (0x3ffffff << 0)
#define JPEG_CODEC_NMCU_SHIFT                       (0)

// REG_100C
#define JPEG_CODEC_NRST(n)                          (((n) & 0xffff) << 0)
#define JPEG_CODEC_NRST_MASK                        (0xffff << 0)
#define JPEG_CODEC_NRST_SHIFT                       (0)

// REG_1010
#define JPEG_CODEC_HD0(n)                           (((n) & 0x1) << 0)
#define JPEG_CODEC_HD0_MASK                         (0x1 << 0)
#define JPEG_CODEC_HD0_SHIFT                        (0)
#define JPEG_CODEC_HA0(n)                           (((n) & 0x1) << 1)
#define JPEG_CODEC_HA0_MASK                         (0x1 << 1)
#define JPEG_CODEC_HA0_SHIFT                        (1)
#define JPEG_CODEC_QT0(n)                           (((n) & 0x3) << 2)
#define JPEG_CODEC_QT0_MASK                         (0x3 << 2)
#define JPEG_CODEC_QT0_SHIFT                        (2)
#define JPEG_CODEC_NBLOCK0(n)                       (((n) & 0xf) << 4)
#define JPEG_CODEC_NBLOCK0_MASK                     (0xf << 4)
#define JPEG_CODEC_NBLOCK0_SHIFT                    (4)

// REG_1014
#define JPEG_CODEC_HD1(n)                           (((n) & 0x1) << 0)
#define JPEG_CODEC_HD1_MASK                         (0x1 << 0)
#define JPEG_CODEC_HD1_SHIFT                        (0)
#define JPEG_CODEC_HA1(n)                           (((n) & 0x1) << 1)
#define JPEG_CODEC_HA1_MASK                         (0x1 << 1)
#define JPEG_CODEC_HA1_SHIFT                        (1)
#define JPEG_CODEC_QT1(n)                           (((n) & 0x3) << 2)
#define JPEG_CODEC_QT1_MASK                         (0x3 << 2)
#define JPEG_CODEC_QT1_SHIFT                        (2)
#define JPEG_CODEC_NBLOCK1(n)                       (((n) & 0xf) << 4)
#define JPEG_CODEC_NBLOCK1_MASK                     (0xf << 4)
#define JPEG_CODEC_NBLOCK1_SHIFT                    (4)

// REG_1018
#define JPEG_CODEC_HD2(n)                           (((n) & 0x1) << 0)
#define JPEG_CODEC_HD2_MASK                         (0x1 << 0)
#define JPEG_CODEC_HD2_SHIFT                        (0)
#define JPEG_CODEC_HA2(n)                           (((n) & 0x1) << 1)
#define JPEG_CODEC_HA2_MASK                         (0x1 << 1)
#define JPEG_CODEC_HA2_SHIFT                        (1)
#define JPEG_CODEC_QT2(n)                           (((n) & 0x3) << 2)
#define JPEG_CODEC_QT2_MASK                         (0x3 << 2)
#define JPEG_CODEC_QT2_SHIFT                        (2)
#define JPEG_CODEC_NBLOCK2(n)                       (((n) & 0xf) << 4)
#define JPEG_CODEC_NBLOCK2_MASK                     (0xf << 4)
#define JPEG_CODEC_NBLOCK2_SHIFT                    (4)

// REG_101C
#define JPEG_CODEC_HD3(n)                           (((n) & 0x1) << 0)
#define JPEG_CODEC_HD3_MASK                         (0x1 << 0)
#define JPEG_CODEC_HD3_SHIFT                        (0)
#define JPEG_CODEC_HA3(n)                           (((n) & 0x1) << 1)
#define JPEG_CODEC_HA3_MASK                         (0x1 << 1)
#define JPEG_CODEC_HA3_SHIFT                        (1)
#define JPEG_CODEC_QT3(n)                           (((n) & 0x3) << 2)
#define JPEG_CODEC_QT3_MASK                         (0x3 << 2)
#define JPEG_CODEC_QT3_SHIFT                        (2)
#define JPEG_CODEC_NBLOCK3(n)                       (((n) & 0xf) << 4)
#define JPEG_CODEC_NBLOCK3_MASK                     (0xf << 4)
#define JPEG_CODEC_NBLOCK3_SHIFT                    (4)


#endif /* __JPEG_REG_H__ */
