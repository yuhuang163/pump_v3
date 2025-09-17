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
#ifndef __CSI2_REG_H__
#define __CSI2_REG_H__

#include "plat_types.h"

struct CSI2_REG_T {
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
    __IO uint32_t REG_30;
    __IO uint32_t REG_34;
    __IO uint32_t REG_38;
    __IO uint32_t REG_3C;
    __IO uint32_t REG_40;
    __IO uint32_t REG_44;
    __IO uint32_t REG_48;
    __IO uint32_t REG_4C;
    __IO uint32_t REG_50;
    __IO uint32_t REG_54;
    __IO uint32_t REG_58;
    __IO uint32_t REG_5C;
    __IO uint32_t REG_60;
    __IO uint32_t REG_64;
    __IO uint32_t REG_68;
    __IO uint32_t REG_6C;
    __IO uint32_t REG_70;
    __IO uint32_t REG_74;
    __IO uint32_t REG_78;
    __IO uint32_t REG_7C;
    __IO uint32_t REG_80;
    __IO uint32_t REG_84;
    __IO uint32_t REG_88;
    __IO uint32_t REG_8C;
    __IO uint32_t REG_90;
    __IO uint32_t REG_94;
    __IO uint32_t REG_98;
    __IO uint32_t REG_9C;
    __IO uint32_t REG_A0;
    __IO uint32_t REG_A4;
    __IO uint32_t REG_A8;
    __IO uint32_t REG_AC;
    __IO uint32_t REG_B0;
    __IO uint32_t REG_B4;
    __IO uint32_t REG_B8;
    __IO uint32_t REG_BC;
    __IO uint32_t REG_C0;
    __IO uint32_t REG_C4;
    __IO uint32_t REG_C8;
};
// REG00
#define CSI2_R_HS_PRPR_TIME_CK(n)                            (((n) & 0xFF) << 24)
#define CSI2_R_HS_PRPR_TIME_CK_MASK                          (0xFF << 24)
#define CSI2_R_HS_PRPR_TIME_CK_SHIFT                         (24)
#define CSI2_R_PWR_INI_DELAY(n)                              (((n) & 0xFF) << 16)
#define CSI2_R_PWR_INI_DELAY_MASK                            (0xFF << 16)
#define CSI2_R_PWR_INI_DELAY_SHIFT                           (16)
#define CSI2_R_CKLANE_HS_EN                                  (1 << 2)
#define CSI2_R_LANE_NUM(n)                                   (((n) & 0x3) << 0)
#define CSI2_R_LANE_NUM_MASK                                 (0x3 << 0)
#define CSI2_R_LANE_NUM_SHIFT                                (0)

// REG08
#define CSI2_R_DIS_LP_PRO_VIOL_ERR                           (1 << 31)
#define CSI2_R_DIS_INVALID_WC                                (1 << 29)
#define CSI2_R_DIS_DTYPE_ERR                                 (1 << 27)
#define CSI2_R_DIS_CRC_ERR                                   (1 << 26)
#define CSI2_R_DIS_MULTI_ECC_ERR                             (1 << 25)
#define CSI2_R_DIS_SINGLE_ECC_ERR                            (1 << 24)
#define CSI2_R_DIS_LP_TRANS_SYNC_ERR                         (1 << 20)
#define CSI2_R_DIS_LP_ESC_ERR                                (1 << 19)
#define CSI2_R_DIS_EOT_ERR                                   (1 << 18)
#define CSI2_R_LP_PRO_VIOL_ERR                               (1 << 15)
#define CSI2_R_INVALID_WC                                    (1 << 13)
#define CSI2_R_DTYPE_ERR                                     (1 << 11)
#define CSI2_R_CRC_ERR                                       (1 << 10)
#define CSI2_R_MULTI_ECC_ERR                                 (1 << 9)
#define CSI2_R_SINGLE_ECC_ERR                                (1 << 8)
#define CSI2_R_LP_TRANS_SYNC_ERR                             (1 << 4)
#define CSI2_R_LP_ESC_ERR                                    (1 << 3)
#define CSI2_R_EOT_ERR                                       (1 << 2)

// REG0C
#define CSI2_R_USER_DTYPE_BN3(n)                             (((n) & 0x3) << 30)
#define CSI2_R_USER_DTYPE_BN3_MASK                           (0x3 << 30)
#define CSI2_R_USER_DTYPE_BN3_SHIFT                          (30)
#define CSI2_R_USER_DTYPE_BN2(n)                             (((n) & 0x3) << 28)
#define CSI2_R_USER_DTYPE_BN2_MASK                           (0x3 << 28)
#define CSI2_R_USER_DTYPE_BN2_SHIFT                          (28)
#define CSI2_R_USER_DTYPE_BN1(n)                             (((n) & 0x3) << 26)
#define CSI2_R_USER_DTYPE_BN1_MASK                           (0x3 << 26)
#define CSI2_R_USER_DTYPE_BN1_SHIFT                          (26)
#define CSI2_R_USER_DTYPE_BN0(n)                             (((n) & 0x3) << 24)
#define CSI2_R_USER_DTYPE_BN0_MASK                           (0x3 << 24)
#define CSI2_R_USER_DTYPE_BN0_SHIFT                          (24)
#define CSI2_R_HDLY1(n)                                      (((n) & 0xFFF) << 12)
#define CSI2_R_HDLY1_MASK                                    (0xFFF << 12)
#define CSI2_R_HDLY1_SHIFT                                   (12)
#define CSI2_R_HDLY0(n)                                      (((n) & 0xFFF) << 0)
#define CSI2_R_HDLY0_MASK                                    (0xFFF << 0)
#define CSI2_R_HDLY0_SHIFT                                   (0)

// REG10
#define CSI2_R_HS_PRPR_TIME_DR(n)                            (((n) & 0xFF) << 24)
#define CSI2_R_HS_PRPR_TIME_DR_MASK                          (0xFF << 24)
#define CSI2_R_HS_PRPR_TIME_DR_SHIFT                         (24)
#define CSI2_R_HDLY3(n)                                      (((n) & 0xFFF) << 12)
#define CSI2_R_HDLY3_MASK                                    (0xFFF << 12)
#define CSI2_R_HDLY3_SHIFT                                   (12)
#define CSI2_R_HDLY2(n)                                      (((n) & 0xFFF) << 0)
#define CSI2_R_HDLY2_MASK                                    (0xFFF << 0)
#define CSI2_R_HDLY2_SHIFT                                   (0)

// REG14
#define CSI2_LP_ESC_FIFO_RODATA(n)                           (((n) & 0xFFFFFF) << 0)
#define CSI2_LP_ESC_FIFO_RODATA_MASK                         (0xFFFFFF << 0)
#define CSI2_LP_ESC_FIFO_RODATA_SHIFT                        (0)

// REG28
#define CSI2_I2C_CMD(n)                                      (((n) & 0x3FF) << 0)
#define CSI2_I2C_CMD_MASK                                    (0x3FF << 0)
#define CSI2_I2C_CMD_SHIFT                                   (0)

// REG2C
#define CSI2_I2C_DATA(n)                                     (((n) & 0X1FF) << 0)
#define CSI2_I2C_DATA_MASK                                   (0x1FF << 0)
#define CSI2_I2C_DATA_SHIFT                                  (0)

// REG60
#define CSI2_CFG_WR_FAST_ENA                                 (1 << 31)
#define CSI2_BURST_LEN(n)                                    (((n) & 0x1F) << 24)
#define CSI2_BURST_LEN_MASK                                  (0x1F << 24)
#define CSI2_BURST_LEN_SHIFT                                 (24)
#define CSI2_IMG_PITCH(n)                                    (((n) & 0x7FFF) << 8)
#define CSI2_IMG_PITCH_MASK                                  (0x7FFF << 8)
#define CSI2_IMG_PITCH_SHIFT                                 (8)
#define CSI2_ARST_SYNC_BYPASS_EN                             (1 << 4)
#define CSI2_PIX_FMT(n)                                      (((n) & 0x3) << 2)
#define CSI2_PIX_FMT_MASK                                    (0x3 << 2)
#define CSI2_PIX_FMT_SHIFT                                   (2)
#define CSI2_WDMA_ADDR_SEL                                   (1 << 1)
#define CSI2_DMA_ENA                                         (1 << 0)

// REG64
#define CSI2_IMG_WIDTH(n)                                    (((n) & 0x1FFF) << 12)
#define CSI2_IMG_WIDTH_MASK                                  (0x1FFF << 12)
#define CSI2_IMG_WIDTH_SHIFT                                 (12)
#define CSI2_IMG_HEIGHT(n)                                   (((n) & 0xFFF) << 0)
#define CSI2_IMG_HEIGHT_MASK                                 (0xFFF << 0)
#define CSI2_IMG_HEIGHT_SHIFT                                (0)

// REG68
#define CSI2_DMA_BASE_ADDR(n)                                (((n) & 0xFFFFFFFF) << 0)
#define CSI2_DMA_BASE_ADDR_MASK                              (0xFFFFFFFF << 0)
#define CSI2_DMA_BASE_ADDR_SHIFT                             (0)

// REG6C
#define CSI2_REG_DMA_ENA_SYNC_FCLK                           (1 << 21)
#define CSI2_REG_DMA_ENA_SYNC_ACLK                           (1 << 20)
#define CSI2_DMAC_RST_N_PWR                                  (1 << 19)
#define CSI2_DMAC_RST_REQ                                    (1 << 18)
#define CSI2_DMAC_WR_INT_CLR                                 (1 << 17)
#define CSI2_DMAC_AXI_SEC                                    (1 << 16)
#define CSI2_DMAC_POSTWR_EN                                  (1 << 15)
#define CSI2_DMAC_MAX_REQ_NUM(n)                             (((n) & 0x7) << 12)
#define CSI2_DMAC_MAX_REQ_NUM_MASK                           (0x7 << 12)
#define CSI2_DMAC_MAX_REQ_NUM_SHIFT                          (12)
#define CSI2_DMAC_ARQOS(n)                                   (((n) & 0xF) << 8)
#define CSI2_DMAC_ARQOS_MASK                                 (0xF << 8)
#define CSI2_DMAC_ARQOS_SHIFT                                (8)
#define CSI2_DMAC_AWQOS(n)                                   (((n) & 0xF) << 4)
#define CSI2_DMAC_AWQOS_MASK                                 (0xF << 4)
#define CSI2_DMAC_AWQOS_SHIFT                                (4)
#define CSI2_DMAC_USER_ID(n)                                 (((n) & 0xF) << 0)
#define CSI2_DMAC_USER_ID_MASK                               (0xF << 0)
#define CSI2_DMAC_USER_ID_SHIFT                              (0)

// REG70
#define CSI2_IRQ_RSR(n)                                      (((n) & 0XFFFFFF) << 0)
#define CSI2_IRQ_RSR_MASK                                    (0xFFFFFF << 0)
#define CSI2_IRQ_RSR_SHIFT                                   (0)

// REG74
#define CSI2_IRQ_ENA(n)                                      (((n) & 0XFFFFFF) << 0)
#define CSI2_IRQ_ENA_MASK                                    (0xFFFFFF << 0)
#define CSI2_IRQ_ENA_SHIFT                                   (0)

// REG78
#define CSI2_ERR_COLOR_FLAG                                  (1 << 0)
#define CSI2_TRIG_LING_FLAG                                  (1 << 1)
#define CSI2_CRC_ERR_IRQ                                     (1 << 2)
#define CSI2_ECC_ERR_IRQ                                     (1 << 3)
#define CSI2_PKT_END_IRQ                                     (1 << 4)
#define CSI2_CSIRX_FIFO_FULL                                 (1 << 5)
#define CSI2_WDMA_DONE                                       (1 << 6)
#define CSI2_BAD_FRAME_IRQ0                                  (1 << 7)
#define CSI2_BAD_FRAME_IRQ1                                  (1 << 8)
#define CSI2_FIFO_UNDERRUN_WDMA1                             (1 << 18)
#define CSI2_FIFO_UNDERRUN_WDMA0                             (1 << 19)
#define CSI2_FIFO_OVERRUN_WDMA1                              (1 << 20)
#define CSI2_FIFO_OVERRUN_WDMA0                              (1 << 21)
#define CSI2_DMAC_WR_ERR_1                                   (1 << 22)
#define CSI2_DMAC_WR_ERR_0                                   (1 << 23)

// REG80
#define CSI2_DMA_BASE_ADDR1(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI2_DMA_BASE_ADDR1_MASK                             (0xFFFFFFFF << 0)
#define CSI2_DMA_BASE_ADDR1_SHIFT                            (0)

// REG94
#define CSI2_HEAD_TYPE(n)                                    (((n) & 0xFFFFFFFF) << 0)
#define CSI2_HEAD_TYPE_MASK                                  (0xFFFFFFFF << 0)
#define CSI2_HEAD_TYPE_SHIFT                                 (0)

// REG98
#define CSI2_TRIG_LINE_WEIGHT_HEIGHT_CNT(n)                  (((n) & 0xFFFFF) << 12)
#define CSI2_RTRIG_LINE_WEIGHT_HEIGHT_CNT_MASK               (0xFFFFF << 12)
#define CSI2_TRIG_LINE_WEIGHT_HEIGHT_CNT_SHIFT               (12)
#define CSI2_REG_TRIG_LINE(n)                                (((n) & 0xFFF) << 0)
#define CSI2_REG_TRIG_LINE_MASK                              (0xFFF << 0)
#define CSI2_REG_TRIG_LINE_SHIFT                             (0)

// REG9C
#define CSI2_TOF_TAP2_PARAL                                  (1 << 31)
#define CSI2_TOF_MARK(n)                                     (((n) & 0x3) << 29)
#define CSI2_TOF_MARK_MASK                                   (0x3 << 29)
#define CSI2_TOF_MARK_SHIFT                                  (29)
#define CSI2_REG_MUX_LP_PRO_VIOL_ERR                         (1 << 28)
#define CSI2_REG_CSI2_CHCK1_CKX_SEL                           (1 << 27)
#define CSI2_REG_DATA_SWAP                                   (1 << 26)
#define CSI2_TOF_TAP_MODE(n)                                 (((n) & 0x3) << 24)
#define CSI2_TOF_TAP_MODE_MASK                               (0x3 << 24)
#define CSI2_TOF_TAP_MODE_SHIFT                              (24)
#define CSI2_TOF_EN                                          (1 << 23)
#define CSI2_LLOPBACK_SUCC(n)                                (((n) & 0x3) << 21)
#define CSI2_LLOPBACK_SUCC_MASK                              (0x3 << 21)
#define CSI2_LLOPBACK_SUCC_SHIFT                             (21)
#define CSI2_MONO_APP_ON                                     (1 << 20)
#define CSI2_SEENSOR_1OR_2                                   (1 << 19)
#define CSI2_HDR_ON                                          (1 << 18)
#define CSI2_CHECK_CRC_MODE                                  (1 << 17)
#define CSI2_CHECK_COLOR_MODE                                (1 << 16)
#define CSI2_REG_ERR_COLOR_PETTE(n)                          (((n) & 0xFFFF) << 0)
#define CSI2_REG_ERR_COLOR_PETT_MASK                         (0xFFFF << 0)
#define CSI2_REG_ERR_COLOR_PETT_SHIFT                        (0)//WRITE IS REG_COLOR_PETT,READ IS ERR_COLOR_PETT

// REGA0
#define CSI2_REG_CRC_REF_GEN(n)                              (((n) & 0xFFFFFFFF) << 0)
#define CSI2_REG_CRC_REF_GEN_MASK                            (0xFFFFFFFF << 0)
#define CSI2_REG_CRC_REF_GEN_SHIFT                           (0)

// REGA4
#define CSI2_DMA_BASE_ADDR2(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI2_DMA_BASE_ADDR2_MASK                             (0xFFFFFFFF << 0)
#define CSI2_DMA_BASE_ADDR2_SHIFT                            (0)

// REGA8
#define CSI2_DMA_BASE_ADDR3(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI2_DMA_BASE_ADDR3_MASK                             (0xFFFFFFFF << 0)
#define CSI2_DMA_BASE_ADDR3_SHIFT                            (0)

// REGAC
#define CSI2_DMA_BASE_ADDR4(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI2_DMA_BASE_ADDR4_MASK                             (0xFFFFFFFF << 0)
#define CSI2_DMA_BASE_ADDR4_SHIFT                            (0)

// REGB0
#define CSI2_DMA_BASE_ADDR5(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI2_DMA_BASE_ADDR5_MASK                             (0xFFFFFFFF << 0)
#define CSI2_DMA_BASE_ADDR5_SHIFT                            (0)

// REGB4
#define CSI2_DMA_BASE_ADDR6(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI2_DMA_BASE_ADDR6_MASK                             (0xFFFFFFFF << 0)
#define CSI2_DMA_BASE_ADDR6_SHIFT                            (0)

// REGB8
#define CSI2_DMA_BASE_ADDR7(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI2_DMA_BASE_ADDR7_MASK                             (0xFFFFFFFF << 0)
#define CSI2_DMA_BASE_ADDR7_SHIFT                            (0)

// REGBC
#define CSI2_DMA_BASE_ADDR2(n)                               (((n) & 0xFFFFFFFF) << 0)
#define CSI2_DMA_BASE_ADDR2_MASK                             (0xFFFFFFFF << 0)
#define CSI2_DMA_BASE_ADDR2_SHIFT                            (0)

// REGC0
#define CSI2_IMG_WIDTH_1(n)                                 (((n) & 0xFFF) << 12)
#define CSI2_IMG_WIDTH_1_MASK                               (0xFFF << 12)
#define CSI2_IMG_WIDTH_1_SHIFT                              (12)
#define CSI2_IMG_HEIGHT_1(n)                                (((n) & 0xFFF) << 0)
#define CSI2_IMG_HEIGHT_1_MASK                              (0xFFF << 0)
#define CSI2_IMG_HEIGHT_1_SHIFT                             (0)

// REGC4
#define CSI2_CFG_TEST_HVALID_CNT(n)                         (((n) & 0x7) << 29)
#define CSI2_CFG_TEST_HVALID_CNT_MASK                       (0x7 << 29)
#define CSI2_CFG_TEST_HVALID_CNT_SHIFT                      (29)
#define CSI2_CFG_TEST_DHSYNC_CNT(n)                         (((n) & 0x3F) << 23)
#define CSI2_CFG_TEST_DHSYNC_CNT_MASK                       (0x3F << 23)
#define CSI2_CFG_TEST_DHSYNC_CNT_SHIFT                      (23)
#define CSI2_CFG_TEST_VHSYNC_CNT(n)                         (((n) & 0x3F) << 17)
#define CSI2_CFG_TEST_VHSYNC_CNT_MASK                       (0x3F << 17)
#define CSI2_CFG_TEST_VHSYNC_CNT_SHIFT                      (17)
#define CSI2_CFG_ISP_TEST_PXLCNT(n)                         (((n) & 0xFFF) << 5)
#define CSI2_CFG_ISP_TEST_PXLCNT_MASK                       (0xFFF << 5)
#define CSI2_CFG_ISP_TEST_PXLCNT_SHIFT                      (5)
#define CSI2_CFG_ISP_TEST2                                  (1 << 4)
#define CSI2_CFG_ISP_TEST1                                  (1 << 3)
#define CSI2_CFG_ISP_TEST0                                  (1 << 2)
#define CSI2_CFG_ISP_TEST_DONE                              (1 << 1)
#define CSI2_CFG_ISP_TEST                                   (1 << 0)

// REGC8
#define CSI2_CFG_TEST_LINE_CNT(n)                           (((n) & 0xFFF) << 0)
#define CSI2_CFG_TEST_LINE_CNT_MASK                         (0xFFF << 0)
#define CSI2_CFG_TEST_LINE_CNT_SHIFT                        (6)
#define CSI2_CFG_TEST_EOF_CNT(n)                            (((n) & 0x3F) << 0)
#define CSI2_CFG_TEST_EOF_CNT_MASK                          (0x3F << 0)
#define CSI2_CFG_TEST_EOF_CNT_SHIFT                         (0)

#endif /* __CSI2_REG_H__ */

