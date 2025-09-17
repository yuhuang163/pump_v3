/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef __DMA2D_REG_H__
#define __DMA2D_REG_H__

#include "plat_types.h"

struct DMA2D_REG_T {
    __IO uint32_t REG_RES_000_030[13];
    __IO uint32_t REG_034;
    __IO uint32_t REG_038;
    __IO uint32_t REG_03C;
    __IO uint32_t REG_040;
    __IO uint32_t REG_044;
    __IO uint32_t REG_048;
    __IO uint32_t REG_04C;
    __IO uint32_t REG_RES_050_07C[12];
    __IO uint32_t REG_080;
    __IO uint32_t REG_084;
    __IO uint32_t REG_RES_088_0BC[14];
    __IO uint32_t REG_0C0;
    __IO uint32_t REG_RES_0C4_0DC[7];
    __IO uint32_t REG_0E0;
    __IO uint32_t REG_0E4;
    __IO uint32_t REG_0E8;
    __IO uint32_t REG_0EC;
    __IO uint32_t REG_0F0;
    __IO uint32_t REG_0F4;
    __IO uint32_t REG_0F8;
    __IO uint32_t REG_0FC;
    __IO uint32_t REG_100;
    __IO uint32_t REG_104;
    __IO uint32_t REG_108;
    __IO uint32_t REG_10C;
    __IO uint32_t REG_110;
    __IO uint32_t REG_114;
    __IO uint32_t REG_118;
    __IO uint32_t REG_11C;
    __IO uint32_t REG_120;
    __IO uint32_t REG_124;
    __IO uint32_t REG_128;
    __IO uint32_t REG_12C;
    __IO uint32_t REG_RES_130_15C[12];
    __IO uint32_t REG_160;
    __IO uint32_t REG_164;
    __IO uint32_t REG_168;
    __IO uint32_t REG_16C;
    __IO uint32_t REG_RES_170_184[6];
    __IO uint32_t REG_188;
    __IO uint32_t REG_18C;
    __IO uint32_t REG_190;
    __IO uint32_t REG_194;
    __IO uint32_t REG_198;
    __IO uint32_t REG_19C;
    __IO uint32_t REG_1A0;
    __IO uint32_t REG_1A4;
    __IO uint32_t REG_1A8;
    __IO uint32_t REG_1AC;
    __IO uint32_t REG_RES_1B0_1BC[4];
    __IO uint32_t REG_1C0;
    __IO uint32_t REG_1C4;
    __IO uint32_t REG_1C8;
    __IO uint32_t REG_1CC;
    __IO uint32_t REG_1D0;
    __IO uint32_t REG_1D4;
    __IO uint32_t REG_1D8;
    __IO uint32_t REG_1DC;
    __IO uint32_t REG_1E0;
    __IO uint32_t REG_1E4;
    __IO uint32_t REG_1E8;
    __IO uint32_t REG_1EC;
    __IO uint32_t REG_1F0;
    __IO uint32_t REG_1F4;
    __IO uint32_t REG_1F8;
    __IO uint32_t REG_1FC;
    __IO uint32_t REG_200;
    __IO uint32_t REG_204;
    __IO uint32_t REG_208;
    __IO uint32_t REG_20C;
    __IO uint32_t REG_210;
    __IO uint32_t REG_214;
    __IO uint32_t REG_218;
    __IO uint32_t REG_21C;
    __IO uint32_t REG_220;
    __IO uint32_t REG_224;
    __IO uint32_t REG_228;
    __IO uint32_t REG_22C;
    __IO uint32_t REG_RES_230_25C[12];
    __IO uint32_t REG_260;
    __IO uint32_t REG_264;
};

// REG_034
#define DMA2D_LAYER_TVG_ADDR(n)                     (((n) & 0xffffffff) << 0)
#define DMA2D_LAYER_TVG_ADDR_MASK                   (0xffffffff << 0)
#define DMA2D_LAYER_TVG_ADDR_SHIFT                  (0)

// REG_03C
#define DMA2D_LAYER_TVG_PITCH(n)                    (((n) & 0xffff) << 0)
#define DMA2D_LAYER_TVG_PITCH_MASK                  (0xffff << 0)
#define DMA2D_LAYER_TVG_PITCH_SHIFT                 (0)

// REG_040
#define DMA2D_LAYER_TVG_YPOS(n)                     (((n) & 0xfff) << 0)
#define DMA2D_LAYER_TVG_YPOS_MASK                   (0xfff << 0)
#define DMA2D_LAYER_TVG_YPOS_SHIFT                  (0)
#define DMA2D_LAYER_TVG_XPOS(n)                     (((n) & 0xfff) << 16)
#define DMA2D_LAYER_TVG_XPOS_MASK                   (0xfff << 16)
#define DMA2D_LAYER_TVG_XPOS_SHIFT                  (16)

// REG_044
#define DMA2D_LAYER_TVG_WIDTH(n)                    (((n) & 0xfff) << 0)
#define DMA2D_LAYER_TVG_WIDTH_MASK                  (0xfff << 0)
#define DMA2D_LAYER_TVG_WIDTH_SHIFT                 (0)
#define DMA2D_LAYER_TVG_HEIGHT(n)                   (((n) & 0xfff) << 16)
#define DMA2D_LAYER_TVG_HEIGHT_MASK                 (0xfff << 16)
#define DMA2D_LAYER_TVG_HEIGHT_SHIFT                (16)

// REG_048
#define DMA2D_LAYER_TVG_ZOOM_WIDTH(n)               (((n) & 0xfff) << 0)
#define DMA2D_LAYER_TVG_ZOOM_WIDTH_MASK             (0xfff << 0)
#define DMA2D_LAYER_TVG_ZOOM_WIDTH_SHIFT            (0)
#define DMA2D_LAYER_TVG_ZOOM_HEIGHT(n)              (((n) & 0xfff) << 16)
#define DMA2D_LAYER_TVG_ZOOM_HEIGHT_MASK            (0xfff << 16)
#define DMA2D_LAYER_TVG_ZOOM_HEIGHT_SHIFT           (16)

// REG_080
#define DMA2D_LAYER_TVG_ENABLE                      (1 << 0)
#define DMA2D_LAYER_TVG_SWAP_RB                     (1 << 4)
#define DMA2D_LAYER_TVG_COLOR_MODE(n)               (((n) & 0xf) << 8) /* 0x0: RGB565;
                                                                        * 0x1: RGB1555;
                                                                        * 0x2: RGB888 packed;
                                                                        * 0x3: RGB888 unpacked;
                                                                        * 0x4: RGBA8888;
                                                                        * 0x9: Palette color 4-bit;
                                                                        * 0xA: Palette color 8-bit;
                                                                        * 0xB: ARGB8888;
                                                                        */
#define DMA2D_LAYER_TVG_COLOR_MODE_MASK             (0xf << 8)
#define DMA2D_LAYER_TVG_COLOR_MODE_SHIFT            (8)

// REG_084
#define DMA2D_BLEND_TV_ALPHA_VAL(n)                 (((n) & 0xff) << 8)
#define DMA2D_BLEND_TV_ALPHA_VAL_MASK               (0xff << 8)
#define DMA2D_BLEND_TV_ALPHA_VAL_SHIFT              (8)

// REG_0C0
#define DMA2D_LAYER_DMA_ADDR(n)                     (((n) & 0xffffffff) << 0)
#define DMA2D_LAYER_DMA_ADDR_MASK                   (0xffffffff << 0)
#define DMA2D_LAYER_DMA_ADDR_SHIFT                  (0)

// REG_0E0
#define DMA2D_LAYER_DMA_PITCH(n)                    (((n) & 0xffff) << 0)
#define DMA2D_LAYER_DMA_PITCH_MASK                  (0xffff << 0)
#define DMA2D_LAYER_DMA_PITCH_SHIFT                 (0)

// REG_0E8
#define DMA2D_LAYER_DMA_YPOS(n)                     (((n) & 0xfff) << 0)
#define DMA2D_LAYER_DMA_YPOS_MASK                   (0xfff << 0)
#define DMA2D_LAYER_DMA_YPOS_SHIFT                  (0)
#define DMA2D_LAYER_DMA_XPOS(n)                     (((n) & 0xfff) << 16)
#define DMA2D_LAYER_DMA_XPOS_MASK                   (0xfff << 16)
#define DMA2D_LAYER_DMA_XPOS_SHIFT                  (16)

// REG_0EC
#define DMA2D_LAYER_DMA_WIDTH(n)                    (((n) & 0xfff) << 0)
#define DMA2D_LAYER_DMA_WIDTH_MASK                  (0xfff << 0)
#define DMA2D_LAYER_DMA_WIDTH_SHIFT                 (0)
#define DMA2D_LAYER_DMA_HEIGHT(n)                   (((n) & 0xfff) << 16)
#define DMA2D_LAYER_DMA_HEIGHT_MASK                 (0xfff << 16)
#define DMA2D_LAYER_DMA_HEIGHT_SHIFT                (16)

// REG_0F0
#define DMA2D_LAYER_DMA_ZOOM_WIDTH(n)               (((n) & 0xfff) << 0)
#define DMA2D_LAYER_DMA_ZOOM_WIDTH_MASK             (0xfff << 0)
#define DMA2D_LAYER_DMA_ZOOM_WIDTH_SHIFT            (0)
#define DMA2D_LAYER_DMA_ZOOM_HEIGHT(n)              (((n) & 0xfff) << 16)
#define DMA2D_LAYER_DMA_ZOOM_HEIGHT_MASK            (0xfff << 16)
#define DMA2D_LAYER_DMA_ZOOM_HEIGHT_SHIFT           (16)

// REG_0F4
#define DMA2D_LAYER_GRA_ADDR(n)                     (((n) & 0xffffffff) << 0)
#define DMA2D_LAYER_GRA_ADDR_MASK                   (0xffffffff << 0)
#define DMA2D_LAYER_GRA_ADDR_SHIFT                  (0)

// REG_0FC
#define DMA2D_LAYER_GRA_PITCH(n)                    (((n) & 0xffff) << 0)
#define DMA2D_LAYER_GRA_PITCH_MASK                  (0xffff << 0)
#define DMA2D_LAYER_GRA_PITCH_SHIFT                 (0)

// REG_100
#define DMA2D_LAYER_GRA_YPOS(n)                     (((n) & 0xfff) << 0)
#define DMA2D_LAYER_GRA_YPOS_MASK                   (0xfff << 0)
#define DMA2D_LAYER_GRA_YPOS_SHIFT                  (0)
#define DMA2D_LAYER_GRA_XPOS(n)                     (((n) & 0xfff) << 16)
#define DMA2D_LAYER_GRA_XPOS_MASK                   (0xfff << 16)
#define DMA2D_LAYER_GRA_XPOS_SHIFT                  (16)

// REG_104
#define DMA2D_LAYER_GRA_WIDTH(n)                    (((n) & 0xfff) << 0)
#define DMA2D_LAYER_GRA_WIDTH_MASK                  (0xfff << 0)
#define DMA2D_LAYER_GRA_WIDTH_SHIFT                 (0)
#define DMA2D_LAYER_GRA_HEIGHT(n)                   (((n) & 0xfff) << 16)
#define DMA2D_LAYER_GRA_HEIGHT_MASK                 (0xfff << 16)
#define DMA2D_LAYER_GRA_HEIGHT_SHIFT                (16)

// REG_108
#define DMA2D_LAYER_GRA_ZOOM_WIDTH(n)               (((n) & 0xfff) << 0)
#define DMA2D_LAYER_GRA_ZOOM_WIDTH_MASK             (0xfff << 0)
#define DMA2D_LAYER_GRA_ZOOM_WIDTH_SHIFT            (0)
#define DMA2D_LAYER_GRA_ZOOM_HEIGHT(n)              (((n) & 0xfff) << 16)
#define DMA2D_LAYER_GRA_ZOOM_HEIGHT_MASK            (0xfff << 16)
#define DMA2D_LAYER_GRA_ZOOM_HEIGHT_SHIFT           (16)

// REG_114
#define DMA2D_SPU_HTOTAL(n)                         (((n) & 0xfff) << 0)
#define DMA2D_SPU_HTOTAL_MASK                       (0xfff << 0)
#define DMA2D_SPU_HTOTAL_SHIFT                      (0)
#define DMA2D_SPU_VTOTAL(n)                         (((n) & 0xfff) << 16)
#define DMA2D_SPU_VTOTAL_MASK                       (0xfff << 16)
#define DMA2D_SPU_VTOTAL_SHIFT                      (16)

// REG_118
#define DMA2D_SPU_WIDTH(n)                          (((n) & 0xfff) << 0)
#define DMA2D_SPU_WIDTH_MASK                        (0xfff << 0)
#define DMA2D_SPU_WIDTH_SHIFT                       (0)
#define DMA2D_SPU_HEIGHT(n)                         (((n) & 0xfff) << 16)
#define DMA2D_SPU_HEIGHT_MASK                       (0xfff << 16)
#define DMA2D_SPU_HEIGHT_SHIFT                      (16)

// REG_11C
#define DMA2D_SPU_HFPORCH(n)                        (((n) & 0xfff) << 0)
#define DMA2D_SPU_HFPORCH_MASK                      (0xfff << 0)
#define DMA2D_SPU_HFPORCH_SHIFT                     (0)
#define DMA2D_SPU_HBPORCH(n)                        (((n) & 0xfff) << 16)
#define DMA2D_SPU_HBPORCH_MASK                      (0xfff << 16)
#define DMA2D_SPU_HBPORCH_SHIFT                     (16)

// REG_120
#define DMA2D_SPU_VFPORCH(n)                        (((n) & 0xfff) << 0)
#define DMA2D_SPU_VFPORCH_MASK                      (0xfff << 0)
#define DMA2D_SPU_VFPORCH_SHIFT                     (0)
#define DMA2D_SPU_VBPORCH(n)                        (((n) & 0xfff) << 16)
#define DMA2D_SPU_VBPORCH_MASK                      (0xfff << 16)
#define DMA2D_SPU_VBPORCH_SHIFT                     (16)

// REG_124
#define DMA2D_SPU_BGCOLOR(n)                        (((n) & 0xffffffff) << 0)
#define DMA2D_SPU_BGCOLOR_MASK                      (0xffffffff << 0)
#define DMA2D_SPU_BGCOLOR_SHIFT                     (0)

// REG_128
#define DMA2D_SPU_APLHA_COLOR1(n)                   (((n) & 0xffffffff) << 0)
#define DMA2D_SPU_APLHA_COLOR1_MASK                 (0xffffffff << 0)
#define DMA2D_SPU_APLHA_COLOR1_SHIFT                (0)

// REG_12C
#define DMA2D_SPU_APLHA_COLOR2(n)                   (((n) & 0xffffffff) << 0)
#define DMA2D_SPU_APLHA_COLOR2_MASK                 (0xffffffff << 0)
#define DMA2D_SPU_APLHA_COLOR2_SHIFT                (0)

// REG_160
#define DMA2D_PN_LAYERS_DISABLE                     (1 << 3)
#define DMA2D_TV_LAYERS_DISABLE                     (1 << 4)

// REG_164
#define DMA2D_PN_LAYERS_ENABLE                      (1 << 3)
#define DMA2D_TV_LAYERS_ENABLE                      (1 << 4)

// REG_188
#define DMA2D_SPU_ENABLE                            (1 << 0)
#define DMA2D_SPU_SMPN_MODE(n)                      (((n) & 0xf) << 8) /* 0: RGB888, 3 cycles/pix;
                                                                        * 1: RGB666, 3 cycles/pix;
                                                                        * 2: RGB565, 3 cycles/pix;
                                                                        * 3: RGB888, 1 cycles/pix;
                                                                        * 4: RGB666, 1 cycles/pix;
                                                                        * 5: RGB565, 1 cycles/pix;
                                                                        */
#define DMA2D_SPU_SMPN_MODE_MASK                    (0xf << 8)
#define DMA2D_SPU_SMPN_MODE_SHIFT                   (8)

// REG_190
#define DMA2D_LAYER_DMA_ENABLE                      (1 << 0)
#define DMA2D_LAYER_DMA_YUV2RGB                     (1 << 1)
#define DMA2D_LAYER_DMA_SWAP_RB                     (1 << 4)
#define DMA2D_LAYER_DMA_COLOR_MODE(n)               (((n) & 0xf) << 8) /* 0x0: RGB565;
                                                                        * 0x1: RGB1555;
                                                                        * 0x2: RGB888 packed;
                                                                        * 0x3: RGB888 unpacked;
                                                                        * 0x4: RGBA8888;
                                                                        * 0x5: YUV422 packed;
                                                                        * 0x6: YUV422 planar;
                                                                        * 0x7: YUV420 planar;
                                                                        * 0x9: Palette color 4-bit;
                                                                        * 0xA: Palette color 8-bit;
                                                                        * 0xB: ARGB8888;
                                                                        */
#define DMA2D_LAYER_DMA_COLOR_MODE_MASK             (0xf << 8)
#define DMA2D_LAYER_DMA_COLOR_MODE_SHIFT            (8)

// REG_194
#define DMA2D_BLEND_PN_ALPHA_VAL(n)                 (((n) & 0xff) << 8)
#define DMA2D_BLEND_PN_ALPHA_VAL_MASK               (0xff << 8)
#define DMA2D_BLEND_PN_ALPHA_VAL_SHIFT              (8)
#define DMA2D_BLEND_PN_CONST_MODE                   (0 << 16)
#define DMA2D_BLEND_PN_PIXEL_MODE                   (1 << 16)
#define DMA2D_BLEND_TV_CONST_MODE                   (0 << 17)
#define DMA2D_BLEND_TV_PIXEL_MODE                   (1 << 17)
#define DMA2D_BLEND_DMA_TRANS_TRIGGER               (1 << 31)

// REG_198
#define DMA2D_CLUT_COLOR_INDEX(n)                   (((n) & 0xff) << 0)
#define DMA2D_CLUT_COLOR_INDEX_MASK                 (0xff << 0)
#define DMA2D_CLUT_COLOR_INDEX_SHIFT                (0)
#define DMA2D_CLUT_COLOR_INDEX_LCDID(n)             (((n) & 0xf) << 8)
#define DMA2D_CLUT_COLOR_INDEX_LCDID_MASK           (0xf << 8)
#define DMA2D_CLUT_COLOR_INDEX_LCDID_SHIFT          (8)
#define DMA2D_CLUT_COLOR_READ_EN                    (1 << 14)
#define DMA2D_CLUT_COLOR_WRITE_EN                   (1 << 15)

// REG_19C
#define DMA2D_CLUT_COLOR_DATA(n)                    (((n) & 0xffffffff) << 0)
#define DMA2D_CLUT_COLOR_DATA_MASK                  (0xffffffff << 0)
#define DMA2D_CLUT_COLOR_DATA_SHIFT                 (0)

// REG_1C0 (IRQ Enable)
// REG_1C4 (IRQ Status)
// REG_1C8 (IRQ_Clear)
#define DMA2D_IRQ_LAYER_TVG_FIFO_EMPTY              (1 << 0)
#define DMA2D_IRQ_LAYER_TVD_FIFO_EMPTY              (1 << 1)
#define DMA2D_IRQ_LAYER_DMA_FIFO_EMPTY              (1 << 2)
#define DMA2D_IRQ_LAYER_GRA_FIFO_EMPTY              (1 << 3)
#define DMA2D_IRQ_LAYER_TVG_FRAME_CNT               (1 << 4)
#define DMA2D_IRQ_LAYER_TVD_FRAME_CNT               (1 << 5)
#define DMA2D_IRQ_LAYER_GRA_FRAME_CNT               (1 << 6)
#define DMA2D_IRQ_LAYER_DMA_FRAME_CNT               (1 << 7)
#define DMA2D_IRQ_FRAME_HALT                        (1 << 8)
#define DMA2D_IRQ_LAYER_TVG_UNDERFLOW               (1 << 9)
#define DMA2D_IRQ_LAYER_TVG_FRAMEDONE1              (1 << 10)
#define DMA2D_IRQ_LAYER_TVG_FRAMEDONE0              (1 << 11)
#define DMA2D_IRQ_LAYER_TV_VSYNC                    (1 << 12)
#define DMA2D_IRQ_LAYER_WB_UNDERRUN                 (1 << 13)
#define DMA2D_IRQ_LAYER_WB_FRAMEDONE                (1 << 14)
#define DMA2D_IRQ_LAYER_WB_OVERRUN                  (1 << 15)
#define DMA2D_IRQ_ERROR                             (1 << 16)
#define DMA2D_IRQ_POWER_DOWN_REQ                    (1 << 17)
#define DMA2D_IRQ_SPI                               (1 << 18)
#define DMA2D_IRQ_SLV                               (1 << 19)
#define DMA2D_IRQ_SPI_FRAMEDONE                     (1 << 20)
#define DMA2D_IRQ_TXC_FRAMEDONE                     (1 << 21)
#define DMA2D_IRQ_TXC_WATERMARK                     (1 << 22)
#define DMA2D_IRQ_PN_VSYNC                          (1 << 23)
#define DMA2D_IRQ_TVC_FRAMEDONE                     (1 << 24)
#define DMA2D_IRQ_LAYER_GRA_UNDERFLOW               (1 << 25)
#define DMA2D_IRQ_LAYER_GRA_FRAMEDONE1              (1 << 26)
#define DMA2D_IRQ_LAYER_GRA_FRAMEDONE0              (1 << 27)
#define DMA2D_IRQ_M2A                               (1 << 28)
#define DMA2D_IRQ_LAYER_DMA_UNDERFLOW               (1 << 29)
#define DMA2D_IRQ_LAYER_DMA_FRAMEDONE1              (1 << 30)
#define DMA2D_IRQ_LAYER_DMA_FRAMEDONE0              (1 << 31)

// REG_1DC
#define DMA2D_LAYER_DMA_BURST_LEN(n)                (((n) & 0x3) << 8)
#define DMA2D_LAYER_DMA_BURST_LEN_MASK              (0x3 << 8)
#define DMA2D_LAYER_DMA_BURST_LEN_SHIFT             (8)
#define DMA2D_LAYER_GRA_BURST_LEN(n)                (((n) & 0x3) << 10)
#define DMA2D_LAYER_GRA_BURST_LEN_MASK              (0x3 << 10)
#define DMA2D_LAYER_GRA_BURST_LEN_SHIFT             (10)
#define DMA2D_VIDEO_PATH_AUTO_SET                   (0 << 16) /* Auto detect processing path */
#define DMA2D_VIDEO_PATH_MANUAL_SET                 (1 << 16) /* Select path by bit17 */
#define DMA2D_VIDEO_PATH_SEL(n)                     (((n) & 0x1) << 17) /* 0: Layer DMA, 1: Layer GRA */
#define DMA2D_VIDEO_PATH_SEL_MASK                   (1 << 17)
#define DMA2D_VIDEO_PATH_SEL_SHIFT                  (17)

// REG_1E8
#define DMA2D_BLEND_MODE(n)                         (((n) & 0x3) << 0) /* 0: (1-a) * dma-pix + a * gra-pix;
                                                                        * 1: (1-a) * dma-pix + 1 * gra-pix;
                                                                        * 2: a * dma-pix + 1 * gra-pix;
                                                                        */
#define DMA2D_BLEND_MODE_MASK                       (0x3 << 0)
#define DMA2D_BLEND_MODE_SHIFT                      (0)

// REG_1EC
#define DMA2D_DITHER_PN_LAYERS_EN                   (1 << 0)
#define DMA2D_DITHER_PN_LAYERS_4X8                  (1 << 1)
#define DMA2D_DITHER_PN_LAYERS_MODE(n)              (((n) & 0x7) << 4)
#define DMA2D_DITHER_PN_LAYERS_MODE_MASK            (0x7 << 4)
#define DMA2D_DITHER_PN_LAYERS_MODE_SHIFT           (4)
#define DMA2D_DITHER_TV_LAYERS_EN                   (1 << 8)
#define DMA2D_DITHER_TV_LAYERS_4X8                  (1 << 9)
#define DMA2D_DITHER_TV_LAYERS_MODE(n)              (((n) & 0x7) << 12)
#define DMA2D_DITHER_TV_LAYERS_MODE_MASK            (0x7 << 12)
#define DMA2D_DITHER_TV_LAYERS_MODE_SHIFT           (12)
#define DMA2D_DITHER_TABLE_INDEX(n)                 (((n) & 0x3) << 16)
#define DMA2D_DITHER_TABLE_INDEX_MASK               (0x3 << 16)
#define DMA2D_DITHER_TABLE_INDEX_SHIFT              (16)

// REG_1F0
#define DMA2D_DITHER_TABLE_DATA(n)                  (((n) & 0xffffffff) << 0)
#define DMA2D_DITHER_TABLE_DATA_MASK                (0xffffffff << 0)
#define DMA2D_DITHER_TABLE_DATA_SHIFT               (0)

// REG_200
#define DMA2D_LAYER_WB_ENABLE                       (1 << 0)
#define DMA2D_LAYER_WB_COLOR_MODE(n)                (((n) & 0x3) << 4) /* 0x0: RGB565;
                                                                        * 0x1: RGB888;
                                                                        * 0x2: RGBA8888;
                                                                        * 0x3: ARGB8888;
                                                                        */
#define DMA2D_LAYER_WB_COLOR_MODE_MASK              (0x3 << 4)
#define DMA2D_LAYER_WB_COLOR_MODE_SHIFT             (4)
#define DMA2D_LAYER_WB_BURST_LEN(n)                 (((n) & 0x1f) << 8)
#define DMA2D_LAYER_WB_BURST_LEN_MASK               (0x1f << 8)
#define DMA2D_LAYER_WB_BURST_LEN_SHIFT              (8)
#define DMA2D_LAYER_WB_PITCH(n)                     (((n) & 0xffff) << 16)
#define DMA2D_LAYER_WB_PITCH_MASK                   (0xffff << 16)
#define DMA2D_LAYER_WB_PITCH_SHIFT                  (16)

// REG_204
#define DMA2D_LAYER_WB_WIDTH(n)                     (((n) & 0xfff) << 0)
#define DMA2D_LAYER_WB_WIDTH_MASK                   (0xfff << 0)
#define DMA2D_LAYER_WB_WIDTH_SHIFT                  (0)
#define DMA2D_LAYER_WB_HEIGHT(n)                    (((n) & 0xfff) << 16)
#define DMA2D_LAYER_WB_HEIGHT_MASK                  (0xfff << 16)
#define DMA2D_LAYER_WB_HEIGHT_SHIFT                 (16)
#define DMA2D_LAYER_WB_FRAME_PER_TOGGLE(n)          (((n) & 0xf) << 28)
#define DMA2D_LAYER_WB_FRAME_PER_TOGGLE_MASK        (0xf << 28)
#define DMA2D_LAYER_WB_FRAME_PER_TOGGLE_SHIFT       (28)

// REG_208
#define DMA2D_LAYER_WB_ADDR(n)                      (((n) & 0xffffffff) << 0)
#define DMA2D_LAYER_WB_ADDR_MASK                    (0xffffffff << 0)
#define DMA2D_LAYER_WB_ADDR_SHIFT                   (0)

// REG_210
#define DMA2D_DISP_SVSYNC_TRIG_DISABLE              (1 << 0)
#define DMA2D_DISP_AVSYNC_TRIG_DISABLE              (1 << 1)
#define DMA2D_DISP_MEM_ENABLE                       (1 << 30)

// REG_218
#define DMA2D_VSYNC_STANDBY_LEN(n)                  (((n) & 0xfff) << 0)
#define DMA2D_VSYNC_STANDBY_LEN_MASK                (0xfff << 0)
#define DMA2D_VSYNC_STANDBY_LEN_SHIFT               (0)
#define DMA2D_VSYNC_START_LINE(n)                   (((n) & 0xff) << 12)
#define DMA2D_VSYNC_START_LINE_MASK                 (0xff << 12)
#define DMA2D_VSYNC_START_LINE_SHIFT                (12)
#define DMA2D_VSYNC_END_LINE(n)                     (((n) & 0xff) << 20)
#define DMA2D_VSYNC_END_LINE_MASK                   (0xff << 20)
#define DMA2D_VSYNC_END_LINE_SHIFT                  (20)

// REG_21C
#define DMA2D_SMPN_HFPORCH(n)                       (((n) & 0xfff) << 0)
#define DMA2D_SMPN_HFPORCH_MASK                     (0xfff << 0)
#define DMA2D_SMPN_HFPORCH_SHIFT                    (0)
#define DMA2D_SMPN_HBPORCH(n)                       (((n) & 0xfff) << 12)
#define DMA2D_SMPN_HBPORCH_MASK                     (0xfff << 12)
#define DMA2D_SMPN_HBPORCH_SHIFT                    (12)
#define DMA2D_VSYNC_LINE_NUM(n)                     (((n) & 0xff) << 24)
#define DMA2D_VSYNC_LINE_NUM_MASK                   (0xff << 24)
#define DMA2D_VSYNC_LINE_NUM_SHIFT                  (24)

// REG_220
#define DMA2D_VSYNC_START_PIX(n)                    (((n) & 0xffff) << 0)
#define DMA2D_VSYNC_START_PIX_MASK                  (0xffff << 0)
#define DMA2D_VSYNC_START_PIX_SHIFT                 (0)
#define DMA2D_VSYNC_END_PIX(n)                      (((n) & 0xffff) << 16)
#define DMA2D_VSYNC_END_PIX_MASK                    (0xffff << 16)
#define DMA2D_VSYNC_END_PIX_SHIFT                   (16)

// REG_224
#define DMA2D_FRAME_START                           (1 << 0)


// REG_260
#define DMA2D_AXIBUS_FAST_MODE                      (1 << 27)

// REG_264
#define DMA2D_LAYER_GRA_ENABLE                      (1 << 0)
#define DMA2D_LAYER_GRA_SWAP_RB                     (1 << 4)
#define DMA2D_LAYER_GRA_COLOR_MODE(n)               (((n) & 0xf) << 8) /* 0x0: RGB565;
                                                                        * 0x1: RGB1555;
                                                                        * 0x2: RGB888 packed;
                                                                        * 0x3: RGB888 unpacked;
                                                                        * 0x4: RGBA8888;
                                                                        * 0x9: Palette color 4-bit;
                                                                        * 0xA: Palette color 8-bit;
                                                                        * 0xB: ARGB8888;
                                                                        */
#define DMA2D_LAYER_GRA_COLOR_MODE_MASK             (0xf << 8)
#define DMA2D_LAYER_GRA_COLOR_MODE_SHIFT            (8)

#endif /* __DMA2D_REG_H__ */
