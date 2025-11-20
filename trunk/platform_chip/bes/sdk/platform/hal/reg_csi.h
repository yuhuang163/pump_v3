/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __CSI_REG_H__
#define __CSI_REG_H__

#include "plat_types.h"

struct CSI_REG_T {
    __IO uint32_t REG_000;
    __IO uint32_t REG_004;
    __IO uint32_t REG_008;
    __IO uint32_t REG_00C;
    __IO uint32_t REG_010;
    __IO uint32_t REG_014;
    __IO uint32_t REG_018;
    __IO uint32_t REG_01C;
    __IO uint32_t REG_020;
    __IO uint32_t REG_024;
    __IO uint32_t REG_028;
    __IO uint32_t REG_02C;
    __IO uint32_t REG_030;
    __IO uint32_t REG_034;
    __IO uint32_t REG_038;
    __IO uint32_t REG_03C;
    __IO uint32_t REG_040;
    __IO uint32_t REG_044;
    __IO uint32_t REG_048;
    __IO uint32_t REG_04C;
    __IO uint32_t REG_050;
    __IO uint32_t REG_054;
    __IO uint32_t REG_058;
    __IO uint32_t REG_05C;
    __IO uint32_t REG_060;
    __IO uint32_t REG_064;
    __IO uint32_t REG_068;
    __IO uint32_t REG_06C;
    __IO uint32_t REG_070;
    __IO uint32_t REG_074;
    __IO uint32_t REG_078;
    __IO uint32_t REG_07C;
    __IO uint32_t REG_080;
    __IO uint32_t REG_084;
    __IO uint32_t REG_088;
    __IO uint32_t REG_08C;
    __IO uint32_t REG_090;
    __IO uint32_t REG_094;
    __IO uint32_t REG_098;
    __IO uint32_t REG_09C;
    __IO uint32_t REG_0A0;
    __IO uint32_t REG_0A4;
    __IO uint32_t REG_0A8;
    __IO uint32_t REG_0AC;
    __IO uint32_t REG_0B0;
    __IO uint32_t REG_0B4;
    __IO uint32_t REG_0B8;
    __IO uint32_t REG_0BC;
    __IO uint32_t REG_0C0;
    __IO uint32_t REG_0C4;
    __IO uint32_t REG_0C8;
    __IO uint32_t REG_0CC;
};

// REG00
#define CSI_R_HS_PRPR_TIME(n)                               (((n) & 0xFF) << 24)
#define CSI_R_HS_PRPR_TIME_MASK                             (0xFF << 24)
#define CSI_R_HS_PRPR_TIME_SHIFT                            (24)
#define CSI_R_PWR_INI_DELAY(n)                              (((n) & 0xFF) << 16)
#define CSI_R_PWR_INI_DELAY_MASK                            (0xFF << 16)
#define CSI_R_PWR_INI_DELAY_SHIFT                           (16)
#define CSI_R_TIMER_LPX(n)                                  (((n) & 0xFF) << 8)
#define CSI_R_TIMER_LPX_MASK                                (0xFF << 8)
#define CSI_R_TIMER_LPX_SHIFT                               (8)
#define CSI_R_FORCE_TIME_LPX                                (1 << 7)
#define CSI_R_LPCD_CYCS                                     (1 << 6)
#define CSI_R_LPCD_DLY                                      (1 << 5)
#define CSI_R_LPCD_EN                                       (1 << 4)
#define CSI_R_DISABLE_TA_FAIL                               (1 << 3)
#define CSI_R_CKLANE_HS_EN                                  (1 << 2)
#define CSI_R_LANE_NUM(n)                                   (((n) & 0x3) << 0)
#define CSI_R_LANE_NUM_MASK                                 (0x3 << 0)
#define CSI_R_LANE_NUM_SHIFT                                (0)

// REG08
#define CSI_R_DIS_LP_PRO_VIOL_ERR                           (1 << 31)
#define CSI_R_DIS_INVALID_WC                                (1 << 29)
#define CSI_R_DIS_DTYPE_ERR                                 (1 << 27)
#define CSI_R_DIS_CRC_ERR                                   (1 << 26)
#define CSI_R_DIS_MULTI_ECC_ERR                             (1 << 25)
#define CSI_R_DIS_SINGLE_ECC_ERR                            (1 << 24)
#define CSI_R_DIS_LP_TRANS_SYNC_ERR                         (1 << 20)
#define CSI_R_DIS_LP_ESC_ERR                                (1 << 19)
#define CSI_R_DIS_EOT_ERR                                   (1 << 18)
#define CSI_R_LP_PRO_VIOL_ERR                               (1 << 15)
#define CSI_R_INVALID_WC                                    (1 << 13)
#define CSI_R_DTYPE_ERR                                     (1 << 11)
#define CSI_R_CRC_ERR                                       (1 << 10)
#define CSI_R_MULTI_ECC_ERR                                 (1 << 9)
#define CSI_R_SINGLE_ECC_ERR                                (1 << 8)
#define CSI_R_LP_TRANS_SYNC_ERR                             (1 << 4)
#define CSI_R_LP_ESC_ERR                                    (1 << 3)
#define CSI_R_EOT_ERR                                       (1 << 2)

// REG0C
#define CSI_R_USER_DTYPE_BN3(n)                                   (((n) & 0x3) << 30)
#define CSI_R_USER_DTYPE_BN3_MASK                                 (0x3 << 30)
#define CSI_R_USER_DTYPE_BN3_SHIFT                                (30)
#define CSI_R_USER_DTYPE_BN2(n)                                   (((n) & 0x3) << 28)
#define CSI_R_USER_DTYPE_BN2_MASK                                 (0x3 << 28)
#define CSI_R_USER_DTYPE_BN2_SHIFT                                (28)
#define CSI_R_USER_DTYPE_BN1(n)                                   (((n) & 0x3) << 26)
#define CSI_R_USER_DTYPE_BN1_MASK                                 (0x3 << 26)
#define CSI_R_USER_DTYPE_BN1_SHIFT                                (26)
#define CSI_R_USER_DTYPE_BN0(n)                                   (((n) & 0x3) << 24)
#define CSI_R_USER_DTYPE_BN0_MASK                                 (0x3 << 24)
#define CSI_R_USER_DTYPE_BN0_SHIFT                                (24)
#define CSI_R_HDLY1(n)                                   (((n) & 0xFFF) << 12)
#define CSI_R_HDLY1_MASK                                 (0xFFF << 12)
#define CSI_R_HDLY1_SHIFT                                (12)
#define CSI_R_HDLY0(n)                                   (((n) & 0xFFF) << 0)
#define CSI_R_HDLY0_MASK                                 (0xFFF << 0)
#define CSI_R_HDLY0_SHIFT                                (0)

// REG10
#define CSI_R_HS_PRPR_TIME_DL(n)                         (((n) & 0xFF) << 24)
#define CSI_R_HS_PRPR_TIME_DL_MASK                       (0xFF << 24)
#define CSI_R_HS_PRPR_TIME_DL_SHIFT                      (24)
#define CSI_R_HDLY3(n)                                   (((n) & 0xFFF) << 12)
#define CSI_R_HDLY3_MASK                                 (0xFFF << 12)
#define CSI_R_HDLY3_SHIFT                                (12)
#define CSI_R_HDLY2(n)                                   (((n) & 0xFFF) << 0)
#define CSI_R_HDLY2_MASK                                 (0xFFF << 0)
#define CSI_R_HDLY2_SHIFT                                (0)

// REG14
#define CSI_LP_ESC_FIFO_RODATA(n)                        (((n) & 0xFFFFFF) << 0)
#define CSI_LP_ESC_FIFO_RODATA_MASK                      (0xFFFFFF << 0)
#define CSI_LP_ESC_FIFO_RODATA_SHIFT                     (0)

// REG28
#define CSI_I2C_CMD(n)                                   (((n) & 0x3FF) << 0)
#define CSI_I2C_CMD_MASK                                 (0x3FF << 0)
#define CSI_I2C_CMD_SHIFT                                (0)

// REG2C
#define CSI_I2C_DATA(n)                                  (((n) & 0X1FF) << 0)
#define CSI_I2C_DATA_MASK                                (0x1FF << 0)
#define CSI_I2C_DATA_SHIFT                               (0)

// REG60
#define CSI_CFG_WR_FAST_ENA                                (1 << 31)
#define CSI_CFG_SPLIT_WLAST_MASK                           (1 << 30)
#define CSI_CFG_WLAST_MARK                                 (1 << 29)
#define CSI_BURST_LEN(n)                                   (((n) & 0x1F) << 24)
#define CSI_BURST_LEN_MASK                                 (0x1F << 24)
#define CSI_BURST_LEN_SHIFT                                (24)
#define CSI_IMG_PITCH(n)                                   (((n) & 0x7FFF) << 8)
#define CSI_IMG_PITCH_MASK                                 (0x7FFF << 8)
#define CSI_IMG_PITCH_SHIFT                                (8)
#define CSI_LOOP_TEST_MODE                                 (1 << 5)
#define CSI_ARST_SYNC_BYPASS_EN                            (1 << 4)
#define CSI_PIX_FMT(n)                                   (((n) & 0x3) << 2)
#define CSI_PIX_FMT_MASK                                 (0x3 << 2)
#define CSI_PIX_FMT_SHIFT                                (2)
#define CSI_WDMA_ADDR_SEL                               (1 << 1)
#define CSI_DMA_ENA                                     (1 << 0)

// REG64
#define CSI_IMG_WIDTH(n)                                   (((n) & 0x1FFF) << 12)
#define CSI_IMG_WIDTH_MASK                                 (0x1FFF << 12)
#define CSI_IMG_WIDTH_SHIFT                                (12)
#define CSI_IMG_HEIGHT(n)                                   (((n) & 0xFFF) << 0)
#define CSI_IMG_HEIGHT_MASK                                 (0xFFF << 0)
#define CSI_IMG_HEIGHT_SHIFT                                (0)

// REG68
#define CSI_DMA_BASE_ADDR(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define CSI_DMA_BASE_ADDR_MASK                                 (0xFFFFFFFF << 0)
#define CSI_DMA_BASE_ADDR_SHIFT                                (0)

// REG6C
#define CSI_REG_DMA_ENA_SYNC_FCLK                               (1 << 21)
#define CSI_REG_DMA_ENA_SYNC_ACLK                               (1 << 20)
#define CSI_DMAC_RST_N_PWR                               (1 << 19)
#define CSI_DMAC_RST_REQ                               (1 << 18)
#define CSI_DMAC_WR_INT_CLK                               (1 << 17)
#define CSI_DMAC_AXI_SEC                               (1 << 16)
#define CSI_DMAC_POSTWR_EN                               (1 << 15)
#define CSI_DMAC_MAX_REQ_NUM(n)                                   (((n) & 0x7) << 12)
#define CSI_DMAC_MAX_REQ_NUM_MASK                                 (0x7 << 12)
#define CSI_DMAC_MAX_REQ_NUM_SHIFT                                (12)
#define CSI_DMAC_ARQOS(n)                                   (((n) & 0xF) << 8)
#define CSI_DMAC_ARQOS_MASK                                 (0xF << 8)
#define CSI_DMAC_ARQOS_SHIFT                                (8)
#define CSI_DMAC_AWQOS(n)                                   (((n) & 0xF) << 4)
#define CSI_DMAC_AWQOS_MASK                                 (0xF << 4)
#define CSI_DMAC_AWQOS_SHIFT                                (4)
#define CSI_DMAC_USER_ID(n)                                   (((n) & 0xF) << 0)
#define CSI_DMAC_USER_ID_MASK                                 (0xF << 0)
#define CSI_DMAC_USER_ID_SHIFT                                (0)

// REG70
#define CSI_IRQ_RSR(n)                                  (((n) & 0XFFFFFF) << 0)
#define CSI_IRQ_RSR_MASK                                (0xFFFFFF << 0)
#define CSI_IRQ_RSR_SHIFT                               (0)

// REG74
#ifdef CHIP_BEST2003
#define CSI_DMA_IRQ_EN_UNDFLOW                              (1 << 5)
#define CSI_DMA_IRQ_EN_DONE                                 (1 << 6)
#define CSI_DMA_IRQ_EN_OVERFLOW                             (1 << 7)
#endif

// REG74
#ifdef CHIP_BEST2009
#define CSI_ERR_COLOR_FLAG_EN                               (1 << 0)
#define CSI_TRIG_LING_FLAG_EN                               (1 << 1)
#define CSI_CRC_ERR_IRQ_EN                                  (1 << 2)
#define CSI_ECC_ERR_IRQ_EN                                  (1 << 3)
#define CSI_PKT_END_IRQ_EN                                  (1 << 4)
#define CSI_CSIRX_FIFO_FULL_EN                              (1 << 5)
#define CSI_DMA_IRQ_EN_DONE                                 (1 << 6)
#define CSI_BAD_FRAME_IRQ_EN                                (1 << 7)
#define CSI_BAD_FRAME_IRQ_EN0                               (1 << 8)
#define CSI_DMA_IRQ_EN_UNDFLOW1                             (1 << 18)
#define CSI_DMA_IRQ_EN_UNDFLOW                              (1 << 19)
#define CSI_DMA_IRQ_EN_OVERFLOW1                            (1 << 20)
#define CSI_DMA_IRQ_EN_OVERFLOW                             (1 << 21)
#define CSI_DMAC_WR_ERR_EN1                                 (1 << 22)
#define CSI_DMAC_WR_ERR_EN                                  (1 << 23)
#endif

// REG78
#ifdef CHIP_BEST2003
#define CSI_DMA_IRQ_MASK_UNDFLOW                            (1 << 5)
#define CSI_DMA_IRQ_MASK_DONE                               (1 << 6)
#define CSI_DMA_IRQ_MASK_OVERFLOW                           (1 << 7)
#endif

// REG78
#ifdef CHIP_BEST2009
#define CSI_ERR_COLOR_FLAG                                  (1 << 0)
#define CSI_TRIG_LING_FLAG                                  (1 << 1)
#define CSI_CRC_ERR_IRQ                                     (1 << 2)
#define CSI_ECC_ERR_IRQ                                     (1 << 3)
#define CSI_PKT_END_IRQ                                     (1 << 4)
#define CSI_CSIRX_FIFO_FULL                                 (1 << 5)
#define CSI_DMA_IRQ_MASK_DONE                               (1 << 6)//CSI_WDMA_DONE
#define CSI_BAD_FRAME_IRQ                                   (1 << 7)
#define CSI_BAD_FRAME_IRQ0                                  (1 << 8)
#define CSI_DMA_IRQ_MASK_UNDFLOW1                           (1 << 18)//CSI_FIFO_UNDERRUN_WDMA0
#define CSI_DMA_IRQ_MASK_UNDFLOW                            (1 << 19)//CSI_FIFO_UNDERRUN_WDMA
#define CSI_DMA_IRQ_MASK_OVERFLOW1                          (1 << 20)//CSI_FIFO_OVERRUN_WDMA0
#define CSI_DMA_IRQ_MASK_OVERFLOW                           (1 << 21)//CSI_FIFO_OVERRUN_WDMA
#define CSI_DMAC_WR_ERR1                                    (1 << 22)
#define CSI_DMAC_WR_ERR                                     (1 << 23)
#endif

// REG80
#define CSI_DMA_BASE_ADDR1(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define CSI_DMA_BASE_ADDR1_MASK                                 (0xFFFFFFFF << 0)
#define CSI_DMA_BASE_ADDR1_SHIFT                                (0)

// REG94
#define CSI_HEAD_TYPE(n)                                        (((n) & 0xFFFFFFFF) << 0)
#define CSI_HEAD_TYPE_MASK                                      (0xFFFFFFFF << 0)
#define CSI_HEAD_TYPE_SHIFT                                     (0)

// REG98
#define CSI_WEIGHT_HEIGHT_CNT(n)                                (((n) & 0xFFFFF) << 12)
#define CSI_WEIGHT_HEIGHT_CNT_MASK                              (0xFFFFF << 12)
#define CSI_WEIGHT_HEIGHT_CNT_SHIFT                             (12)
#define CSI_REG_TRIG_LINE(n)                                    (((n) & 0xFFF) << 0)
#define CSI_REG_TRIG_LINE_MASK                                  (0xFFF << 0)
#define CSI_REG_TRIG_LINE_SHIFT                                 (0)

// REG9C
// 2009 CSI0/1_REG_9C 31-21bit
#define CSI_DTYPE_RAW10                                         (1 << 29)
#define CSI_DTYPE_RAW8                                          (1 << 28)
#define CSI_NEED_CSIRX                                          (1 << 27)
#define CSI_CFG_DP_MUL_SEL                                      (1 << 26)
#define CSI_CFG_ISP_VCYNC_INV                                   (1 << 25)
#define CSI_REG_MUX_LP_PRO_VIOL_ERR                             (1 << 24)
#define CSI_TOF_EN                                              (1 << 23)
#define CSI_REG_DATA_SWAP                                       (1 << 22)
#define CSI_DOL_HDR_ON                                          (1 << 21)

// 2009 CSI2_REG_9C 31-21bit
#define CSI2_TOF_TAP2_PARAL                                     (1 << 31)
#define CSI2_TOF_MARK(n)                                        (((n) & 0x3) << 29)
#define CSI2_TOF_MARK_MASK                                      (0x3 << 29)
#define CSI2_TOF_MARK_SHIFT                                     (29)
#define CSI2_REG_MUX_LP_PRO_VIOL_ERR                            (1 << 28)
#define CSI2_REG_CSI_CHCK1_CKX_SEL                              (1 << 27)
#define CSI2_REG_DATA_SWAP                                      (1 << 26)
#define CSI2_TOF_TAP_MODE(n)                                    (((n) & 0x3) << 24)
#define CSI2_TOF_TAP_MODE_MASK                                  (0x3 << 24)
#define CSI2_TOF_TAP_MODE_SHIFT                                 (24)
#define CSI2_TOF_EN                                             (1 << 23)
#define CSI2_CFG_ISP_VCYNC_INV                                  (1 << 21)

#define CSI_MONO_APP_ON                                         (1 << 20)
#define CSI_SEENSOR_1OR_2                                       (1 << 19)
#define CSI_HDR_ON                                              (1 << 18)
#define CSI_CHECK_CRC_MODE                                      (1 << 17)
#define CSI_CHECK_COLOR_MODE                                    (1 << 16)
#define CSI_REG_ERR_COLOR_PETTE(n)                              (((n) & 0xFFFF) << 0)
#define CSI_REG_ERR_COLOR_PETT_MASK                             (0xFFFF << 0)
#define CSI_REG_ERR_COLOR_PETT_SHIFT                            (0)//WRITE IS REG_COLOR_PETT,READ IS ERR_COLOR_PETT

// REGA0
#define CSI_REG_CRC_REF_GEN(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define CSI_REG_CRC_REF_GEN_MASK                                (0xFFFFFFFF << 0)
#define CSI_REG_CRC_REF_GEN_SHIFT                               (0)

// REGA4
#define CSI_DMA_BASE_ADDR2(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI_DMA_BASE_ADDR2_MASK                             (0xFFFFFFFF << 0)
#define CSI_DMA_BASE_ADDR2_SHIFT                            (0)

// REGA8
#define CSI_DMA_BASE_ADDR3(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI_DMA_BASE_ADDR3_MASK                             (0xFFFFFFFF << 0)
#define CSI_DMA_BASE_ADDR3_SHIFT                            (0)

// REGAC
#define CSI_DMA_BASE_ADDR4(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI_DMA_BASE_ADDR4_MASK                             (0xFFFFFFFF << 0)
#define CSI_DMA_BASE_ADDR4_SHIFT                            (0)

// REGB0
#define CSI_DMA_BASE_ADDR5(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI_DMA_BASE_ADDR5_MASK                             (0xFFFFFFFF << 0)
#define CSI_DMA_BASE_ADDR5_SHIFT                            (0)

// REGB4
#define CSI_DMA_BASE_ADDR6(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI_DMA_BASE_ADDR6_MASK                             (0xFFFFFFFF << 0)
#define CSI_DMA_BASE_ADDR6_SHIFT                            (0)

// REGB8
#define CSI_DMA_BASE_ADDR7(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI_DMA_BASE_ADDR7_MASK                             (0xFFFFFFFF << 0)
#define CSI_DMA_BASE_ADDR7_SHIFT                            (0)

// REGBC
#define CSI_IMG_PITCH1(n)                                  (((n) & 0x7FFF) << 8)
#define CSI_IMG_PITCH1_MASK                                (0x7FFF << 8)
#define CSI_IMG_PITCH1_SHIFT                               (8)
#define CSI_PIX_FMT1(n)                                    (((n) & 0x3) << 2)
#define CSI_PIX_FMT1_MASK                                  (0x3 << 2)
#define CSI_PIX_FMT1_SHIFT                                 (2)
#define CSI_WDMA_ADDR_SEL1                                 (1 << 1)
#define CSI_DMA_ENA1                                       (1 << 0)

// REGC0
#define CSI_IMG_WIDTH_1(n)                                 (((n) & 0xFFF) << 12)
#define CSI_IMG_WIDTH_1_MASK                               (0xFFF << 12)
#define CSI_IMG_WIDTH_1_SHIFT                              (12)
#define CSI_IMG_HEIGHT_1(n)                                (((n) & 0xFFF) << 0)
#define CSI_IMG_HEIGHT_1_MASK                              (0xFFF << 0)
#define CSI_IMG_HEIGHT_1_SHIFT                             (0)

// REGC4
#define CSI_CFG_TEST_HVALID_CNT(n)                         (((n) & 0x7) << 29)
#define CSI_CFG_TEST_HVALID_CNT_MASK                       (0x7 << 29)
#define CSI_CFG_TEST_HVALID_CNT_SHIFT                      (29)
#define CSI_CFG_TEST_DHSYNC_CNT(n)                         (((n) & 0x3F) << 23)
#define CSI_CFG_TEST_DHSYNC_CNT_MASK                       (0x3F << 23)
#define CSI_CFG_TEST_DHSYNC_CNT_SHIFT                      (23)
#define CSI_CFG_TEST_VHSYNC_CNT(n)                         (((n) & 0x3F) << 17)
#define CSI_CFG_TEST_VHSYNC_CNT_MASK                       (0x3F << 17)
#define CSI_CFG_TEST_VHSYNC_CNT_SHIFT                      (17)
#define CSI_CFG_ISP_TEST_PXLCNT(n)                         (((n) & 0xFFF) << 5)
#define CSI_CFG_ISP_TEST_PXLCNT_MASK                       (0xFFF << 5)
#define CSI_CFG_ISP_TEST_PXLCNT_SHIFT                      (5)
#define CSI_CFG_ISP_TEST2                                  (1 << 4)
#define CSI_CFG_ISP_TEST1                                  (1 << 3)
#define CSI_CFG_ISP_TEST0                                  (1 << 2)
#define CSI_CFG_ISP_TEST_DONE                              (1 << 1)
#define CSI_CFG_ISP_TEST                                   (1 << 0)

// REGC8
#define CSI_DP_MUX_SEL                                     (1 << 30)
#define CSI_DTYPE_RAW10_1                                  (1 << 29)
#define CSI_DTYPE_RAW8_1                                   (1 << 28)
#define CSI_CFG_PXL_CNT(n)                                 (((n) & 0xF) << 0)
#define CSI_CFG_PXL_CNT_MASK                               (0xF << 0)
#define CSI_CFG_PXL_CNT_SHIFT                              (24)
#define CSI_CFG_TEST_LINE_CNT(n)                           (((n) & 0xFFF) << 0)
#define CSI_CFG_TEST_LINE_CNT_MASK                         (0xFFF << 0)
#define CSI_CFG_TEST_LINE_CNT_SHIFT                        (12)
#define CSI_CFG_TEST_EOF_CNT(n)                            (((n) & 0xFFF) << 0)
#define CSI_CFG_TEST_EOF_CNT_MASK                          (0xFFF << 0)
#define CSI_CFG_TEST_EOF_CNT_SHIFT                         (0)

// REGCC
#define CSI_CFG_WC(n)                                      (((n) & 0xFFFF) << 0)
#define CSI_CFG_WC_MASK                                    (0xFFFF << 0)
#define CSI_CFG_WC_SHIFT                                   (16)
#define CSI_CFG_DTYPE(n)                                   (((n) & 0x3F) << 0)
#define CSI_CFG_DTYPE_MASK                                 (0x3F << 0)
#define CSI_CFG_DTYPE_SHIFT                                (1)
#define CSI_CFG_PROTECT                                    (1 << 0)

#endif /* __CSI_REG_H__ */
