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
#ifndef __REG_PSRAM_MC_V7_H__
#define __REG_PSRAM_MC_V7_H__

#include "plat_types.h"

struct PSRAM_PHY_T {
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
};

// reg_000
#define PSRAM_PHY_CHIP_TYPE                                 (1 << 0)
#define PSRAM_PHY_CHIP_BIT                                  (1 << 1)
#define PSRAM_PHY_MEMORY_WIDTH(n)                           (((n) & 0x3) << 2)
#define PSRAM_PHY_MEMORY_WIDTH_MASK                         (0x3 << 2)
#define PSRAM_PHY_MEMORY_WIDTH_SHIFT                        (2)
#define PSRAM_PHY_FRE_RATIO(n)                              (((n) & 0x3) << 4)
#define PSRAM_PHY_FRE_RATIO_MASK                            (0x3 << 4)
#define PSRAM_PHY_FRE_RATIO_SHIFT                           (4)

// reg_004
#define PSRAM_PHY_PHY_DUMMY_CYC_EN                          (1 << 0)
#define PSRAM_PHY_SQPI_SAMPLE_SEL(n)                        (((n) & 0xF) << 1)
#define PSRAM_PHY_SQPI_SAMPLE_SEL_MASK                      (0xF << 1)
#define PSRAM_PHY_SQPI_SAMPLE_SEL_SHIFT                     (1)
#define PSRAM_PHY_IDLE_DQ_OEN                               (1 << 5)
#define PSRAM_PHY_SQPI_PHY_TX_BYPASS                        (1 << 6)
#define PSRAM_PHY_IDLE_DQS_OEN                              (1 << 7)
#define PSRAM_PHY_DQS_DM_MERGE_EN                           (1 << 8)
#define PSRAM_PHY_MRW_WO_DM                                 (1 << 9)
#define PSRAM_PHY_CS_POL                                    (1 << 10)
#define PSRAM_PHY_PHY_DUMMY_CYC_STOP_DQS                    (1 << 11)

// reg_008
#define PSRAM_PHY_T_WPST(n)                                 (((n) & 0x7) << 0)
#define PSRAM_PHY_T_WPST_MASK                               (0x7 << 0)
#define PSRAM_PHY_T_WPST_SHIFT                              (0)

// reg_00c
#define PSRAM_PHY_T_EXPANDRD(n)                             (((n) & 0x3F) << 0)
#define PSRAM_PHY_T_EXPANDRD_MASK                           (0x3F << 0)
#define PSRAM_PHY_T_EXPANDRD_SHIFT                          (0)

// reg_010
#define PSRAM_PHY_CMD_CONFLICT_CLR                          (1 << 0)

// reg_014
#define PSRAM_PHY_SQPI_CMD_CONFLICT_CLR                     (1 << 0)

// reg_018
#define PSRAM_PHY_T_PHYDATA_DLY(n)                          (((n) & 0x3) << 0)
#define PSRAM_PHY_T_PHYDATA_DLY_MASK                        (0x3 << 0)
#define PSRAM_PHY_T_PHYDATA_DLY_SHIFT                       (0)

// reg_01c
#define PSRAM_PHY_CA_TR_EN                                  (1 << 0)
#define PSRAM_PHY_DQS_RDY_0                                 (1 << 1)
#define PSRAM_PHY_DQS_STS_0                                 (1 << 2)
#define PSRAM_PHY_DQS_RDY_1                                 (1 << 3)
#define PSRAM_PHY_DQS_STS_1                                 (1 << 4)

// reg_020
#define PSRAM_PHY_SEL_PHY_CLK1X                             (1 << 0)
#define PSRAM_PHY_CA_SEQ(n)                                 (((n) & 0x7) << 1)
#define PSRAM_PHY_CA_SEQ_MASK                               (0x7 << 1)
#define PSRAM_PHY_CA_SEQ_SHIFT                              (1)
#define PSRAM_PHY_RX_SAMP_POS_SEL                           (1 << 4)
#define PSRAM_PHY_PAD_RSTN                                  (1 << 5)
#define PSRAM_PHY_HYPERBUS_SAMPLE_LAT_EDGE_SEL              (1 << 6)
#define PSRAM_PHY_PAD_EN                                    (1 << 7)
#define PSRAM_PHY_HYPERBUS_VAR_LAT                          (1 << 8)
#define PSRAM_PHY_HYPERBUS_CMD_DLY_CNT(n)                   (((n) & 0x7) << 9)
#define PSRAM_PHY_HYPERBUS_CMD_DLY_CNT_MASK                 (0x7 << 9)
#define PSRAM_PHY_HYPERBUS_CMD_DLY_CNT_SHIFT                (9)
#define PSRAM_PHY_REG_RX_DLY_OF_DESIGN(n)                   (((n) & 0xF) << 12)
#define PSRAM_PHY_REG_RX_DLY_OF_DESIGN_MASK                 (0xF << 12)
#define PSRAM_PHY_REG_RX_DLY_OF_DESIGN_SHIFT                (12)
#define PSRAM_PHY_CFG_RX_DQS_SEL(n)                         (((n) & 0x3) << 16)
#define PSRAM_PHY_CFG_RX_DQS_SEL_MASK                       (0x3 << 16)
#define PSRAM_PHY_CFG_RX_DQS_SEL_SHIFT                      (16)
#define PSRAM_PHY_REG_CA_DQ_LAT_SW_EN                       (1 << 18)
#define PSRAM_PHY_T_LAT_SWITCH_CYC(n)                       (((n) & 0xF) << 19)
#define PSRAM_PHY_T_LAT_SWITCH_CYC_MASK                     (0xF << 19)
#define PSRAM_PHY_T_LAT_SWITCH_CYC_SHIFT                    (19)
#define PSRAM_PHY_MAN_TX_WR_PUSH_EN                         (1 << 23)
#define PSRAM_PHY_AUTO_PWR_TX_WR_PUSH_EN                    (1 << 24)
#define PSRAM_PHY_CLK_PREAMPLE_EN                           (1 << 25)
#define PSRAM_PHY_CA_PREAMPLE_EN                            (1 << 26)
#define PSRAM_PHY_HYPERBUS_TCSS(n)                          (((n) & 0x3) << 27)
#define PSRAM_PHY_HYPERBUS_TCSS_MASK                        (0x3 << 27)
#define PSRAM_PHY_HYPERBUS_TCSS_SHIFT                       (27)

// reg_024
#define PSRAM_PHY_AUTO_PUSH_EXT_COUNT(n)                    (((n) & 0xF) << 0)
#define PSRAM_PHY_AUTO_PUSH_EXT_COUNT_MASK                  (0xF << 0)
#define PSRAM_PHY_AUTO_PUSH_EXT_COUNT_SHIFT                 (0)
#define PSRAM_PHY_REG_TR_CLK_EN                             (1 << 4)
#define PSRAM_PHY_REG_TR_DQS_EN                             (1 << 5)


// reg_028
#define PSRAM_PHY_REG_REF_DDP_EN                            (1 << 0)
#define PSRAM_PHY_REG_REF_DDP_CMD(n)                        (((n) & 0xFF) << 1)
#define PSRAM_PHY_REG_REF_DDP_CMD_MASK                      (0xFF << 1)
#define PSRAM_PHY_REG_REF_DDP_CMD_SHIFT                     (1)

// reg_03c
#define PSRAM_PHY_REG_PHY_PWR_DONE_DR                       (1 << 0)
#define PSRAM_PHY_REG_PHY_PWR_DONE                          (1 << 1)
#define PSRAM_PHY_REG_PHY_PD_TRANS_MASK                     (1 << 2)

// reg_040
#define PSRAM_PHY_PHY_RX_BYPASS                             (1 << 0)
#define PSRAM_PHY_PHY_SAMP_WITH_CLK                         (1 << 1)
#define PSRAM_PHY_PHY_RX_BYPASS2                            (1 << 2)
#define PSRAM_PHY_PHY_SAMP_WITH_CLK2                        (1 << 3)
#define PSRAM_PHY_REG_PSRAM_LOOPBACK_EN                     (1 << 4)
#define PSRAM_PHY_REG_PSRAM_LOOPBACK_EN2                    (1 << 5)
#define PSRAM_PHY_PHY_P2S_BYPASS                            (1 << 6)
#define PSRAM_PHY_PHY_ANA_BYPASS                            (1 << 7)
#define PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL(n)               (((n) & 0x7) << 8)
#define PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL_MASK             (0x7 << 8)
#define PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL_SHIFT            (8)
#define PSRAM_PHY_PHY_BYPASS_AUTO_LP_EN                     (1 << 11)
#define PSRAM_PHY_MC_IDLE_AUTO_LP_EN                        (1 << 12)
#define PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL1(n)              (((n) & 0x7) << 13)
#define PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL1_MASK            (0x7 << 13)
#define PSRAM_PHY_BYPASS_ANA_RX_SAMPLE_SEL1_SHIFT           (13)

// reg_044
#define PSRAM_PHY_CMD_CONFLICT_STS_0                        (1 << 0)
#define PSRAM_PHY_PHY_FSM_STATE_0(n)                        (((n) & 0xF) << 1)
#define PSRAM_PHY_PHY_FSM_STATE_0_MASK                      (0xF << 1)
#define PSRAM_PHY_PHY_FSM_STATE_0_SHIFT                     (1)
#define PSRAM_PHY_DBG_RX_EMPTY_0                            (1 << 5)
#define PSRAM_PHY_DBG_RXFIFO_COUNT_0(n)                     (((n) & 0x7) << 6)
#define PSRAM_PHY_DBG_RXFIFO_COUNT_0_MASK                   (0x7 << 6)
#define PSRAM_PHY_DBG_RXFIFO_COUNT_0_SHIFT                  (6)
#define PSRAM_PHY_DBG_RX_DQS_FLAG_0                         (1 << 9)
#define PSRAM_PHY_CMD_CONFLICT_STS_1                        (1 << 10)
#define PSRAM_PHY_PHY_FSM_STATE_1(n)                        (((n) & 0xF) << 11)
#define PSRAM_PHY_PHY_FSM_STATE_1_MASK                      (0xF << 11)
#define PSRAM_PHY_PHY_FSM_STATE_1_SHIFT                     (11)
#define PSRAM_PHY_DBG_RX_EMPTY_1                            (1 << 15)
#define PSRAM_PHY_DBG_RXFIFO_COUNT_1(n)                     (((n) & 0x7) << 16)
#define PSRAM_PHY_DBG_RXFIFO_COUNT_1_MASK                   (0x7 << 16)
#define PSRAM_PHY_DBG_RXFIFO_COUNT_1_SHIFT                  (16)
#define PSRAM_PHY_DBG_RX_DQS_FLAG_1                         (1 << 19)
#define PSRAM_PHY_SQPI_CMD_CONFLICT_STS                     (1 << 20)
#define PSRAM_PHY_SQPI_PHY_FSM_STATE(n)                     (((n) & 0xF) << 21)
#define PSRAM_PHY_SQPI_PHY_FSM_STATE_MASK                   (0xF << 21)
#define PSRAM_PHY_SQPI_PHY_FSM_STATE_SHIFT                  (21)
#define PSRAM_PHY_PHY_AUTO_LP_STATE(n)                      (((n) & 0xF) << 25)
#define PSRAM_PHY_PHY_AUTO_LP_STATE_MASK                    (0xF << 25)
#define PSRAM_PHY_PHY_AUTO_LP_STATE_SHIFT                   (25)

// reg_048
#define PSRAM_PHY_REG_T_INIT0(n)                            (((n) & 0x3F) << 0)
#define PSRAM_PHY_REG_T_INIT0_MASK                          (0x3F << 0)
#define PSRAM_PHY_REG_T_INIT0_SHIFT                         (0)
#define PSRAM_PHY_REG_T_INIT1(n)                            (((n) & 0x3F) << 6)
#define PSRAM_PHY_REG_T_INIT1_MASK                          (0x3F << 6)
#define PSRAM_PHY_REG_T_INIT1_SHIFT                         (6)
#define PSRAM_PHY_REG_T_INIT2(n)                            (((n) & 0x3F) << 12)
#define PSRAM_PHY_REG_T_INIT2_MASK                          (0x3F << 12)
#define PSRAM_PHY_REG_T_INIT2_SHIFT                         (12)
#define PSRAM_PHY_REG_T_INIT3(n)                            (((n) & 0x3F) << 18)
#define PSRAM_PHY_REG_T_INIT3_MASK                          (0x3F << 18)
#define PSRAM_PHY_REG_T_INIT3_SHIFT                         (18)

// reg_04c
#define PSRAM_PHY_REG_PSRAM_PU_DR                           (1 << 0)
#define PSRAM_PHY_REG_PSRAM_PU2_DR                          (1 << 1)
#define PSRAM_PHY_REG_PSRAM_SLEEP_EN_DR                     (1 << 2)
#define PSRAM_PHY_REG_PSRAM_SLEEP_EN2_DR                    (1 << 3)
#define PSRAM_PHY_REG_PSRAM_CEB1_PU_DR                      (1 << 4)
#define PSRAM_PHY_REG_PSRAM_CEB2_PU_DR                      (1 << 5)

// reg_050
#define PSRAM_PHY_REG_PSRAM_PU                              (1 << 0)
#define PSRAM_PHY_REG_PSRAM_PU2                             (1 << 1)
#define PSRAM_PHY_REG_PSRAM_SLEEP_EN                        (1 << 2)
#define PSRAM_PHY_REG_PSRAM_SLEEP_EN2                       (1 << 3)
#define PSRAM_PHY_REG_PSRAM_CEB1_PU                         (1 << 4)
#define PSRAM_PHY_REG_PSRAM_CEB2_PU                         (1 << 5)

// reg_054
#define PSRAM_PHY_PSRAM_TX_CLK_PHASE_CHECK(n)               (((n) & 0xF) << 0)
#define PSRAM_PHY_PSRAM_TX_CLK_PHASE_CHECK_MASK             (0xF << 0)
#define PSRAM_PHY_PSRAM_TX_CLK_PHASE_CHECK_SHIFT            (0)
#define PSRAM_PHY_PSRAM_TX_DQS_PHASE_CHECK(n)               (((n) & 0xF) << 4)
#define PSRAM_PHY_PSRAM_TX_DQS_PHASE_CHECK_MASK             (0xF << 1)
#define PSRAM_PHY_PSRAM_TX_DQS_PHASE_CHECK_SHIFT            (4)

// reg_058
#define PSRAM_PHY_REG0_PSRAM_CEB_SEL                        (1 << 0)
#define PSRAM_PHY_REG0_PSRAM_CEB_SEL2                       (1 << 1)
#define PSRAM_PHY_REG0_PSRAM_DIV2_RESETB                    (1 << 2)
#define PSRAM_PHY_REG0_PSRAM_CAL_CLK_EN                     (1 << 3)
#define PSRAM_PHY_REG0_PSRAM_LOOPBACK_EN                    (1 << 4)
#define PSRAM_PHY_REG0_PSRAM_LOOPBACK_EN2                   (1 << 5)
#define PSRAM_PHY_REG0_PSRAM_TXCLK_TO_RXDQS_EN              (1 << 6)
#define PSRAM_PHY_REG0_PSRAM_TXCLK_TO_RXDQS_EN2             (1 << 7)
#define PSRAM_PHY_REG1_PSRAM_CEB_SEL                        (1 << 8)
#define PSRAM_PHY_REG1_PSRAM_CEB_SEL2                       (1 << 9)
#define PSRAM_PHY_REG1_PSRAM_DIV2_RESETB                    (1 << 10)
#define PSRAM_PHY_REG1_PSRAM_CAL_CLK_EN                     (1 << 11)
#define PSRAM_PHY_REG1_PSRAM_LOOPBACK_EN                    (1 << 12)
#define PSRAM_PHY_REG1_PSRAM_LOOPBACK_EN2                   (1 << 13)
#define PSRAM_PHY_REG1_PSRAM_TXCLK_TO_RXDQS_EN              (1 << 14)
#define PSRAM_PHY_REG1_PSRAM_TXCLK_TO_RXDQS_EN2             (1 << 15)

// reg_05c
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DLY(n)                  (((n) & 0xF) << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DLY_MASK                (0xF << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DLY_SHIFT               (0)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK2_DLY(n)                 (((n) & 0xF) << 4)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK2_DLY_MASK               (0xF << 4)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK2_DLY_SHIFT              (4)
#define PSRAM_PHY_REG0_PSRAM_TX_CEB1_DLY(n)                 (((n) & 0xF) << 8)
#define PSRAM_PHY_REG0_PSRAM_TX_CEB1_DLY_MASK               (0xF << 8)
#define PSRAM_PHY_REG0_PSRAM_TX_CEB1_DLY_SHIFT              (8)
#define PSRAM_PHY_REG0_PSRAM_TX_CEB2_DLY(n)                 (((n) & 0xF) << 12)
#define PSRAM_PHY_REG0_PSRAM_TX_CEB2_DLY_MASK               (0xF << 12)
#define PSRAM_PHY_REG0_PSRAM_TX_CEB2_DLY_SHIFT              (12)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DLY(n)                  (((n) & 0xF) << 16)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DLY_MASK                (0xF << 16)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DLY_SHIFT               (16)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS2_DLY(n)                 (((n) & 0xF) << 20)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS2_DLY_MASK               (0xF << 20)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS2_DLY_SHIFT              (20)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DELAY_RANGE(n)          (((n) & 0x3) << 24)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DELAY_RANGE_MASK        (0x3 << 24)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DELAY_RANGE_SHIFT       (24)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DELAY_RANGE2(n)         (((n) & 0x3) << 26)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DELAY_RANGE2_MASK       (0x3 << 26)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DELAY_RANGE2_SHIFT      (26)

// reg_060
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DLY(n)                  (((n) & 0xF) << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DLY_MASK                (0xF << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DLY_SHIFT               (0)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK2_DLY(n)                 (((n) & 0xF) << 4)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK2_DLY_MASK               (0xF << 4)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK2_DLY_SHIFT              (4)
#define PSRAM_PHY_REG1_PSRAM_TX_CEB1_DLY(n)                 (((n) & 0xF) << 8)
#define PSRAM_PHY_REG1_PSRAM_TX_CEB1_DLY_MASK               (0xF << 8)
#define PSRAM_PHY_REG1_PSRAM_TX_CEB1_DLY_SHIFT              (8)
#define PSRAM_PHY_REG1_PSRAM_TX_CEB2_DLY(n)                 (((n) & 0xF) << 12)
#define PSRAM_PHY_REG1_PSRAM_TX_CEB2_DLY_MASK               (0xF << 12)
#define PSRAM_PHY_REG1_PSRAM_TX_CEB2_DLY_SHIFT              (12)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DLY(n)                  (((n) & 0xF) << 16)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DLY_MASK                (0xF << 16)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DLY_SHIFT               (16)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS2_DLY(n)                 (((n) & 0xF) << 20)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS2_DLY_MASK               (0xF << 20)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS2_DLY_SHIFT              (20)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DELAY_RANGE(n)          (((n) & 0x3) << 24)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DELAY_RANGE_MASK        (0x3 << 24)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DELAY_RANGE_SHIFT       (24)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DELAY_RANGE2(n)         (((n) & 0x3) << 26)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DELAY_RANGE2_MASK       (0x3 << 26)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DELAY_RANGE2_SHIFT      (26)

// reg_064
#define PSRAM_PHY_REG0_PSRAM_RX_DELAY_CTRL(n)               (((n) & 0xF) << 0)
#define PSRAM_PHY_REG0_PSRAM_RX_DELAY_CTRL_MASK             (0xF << 0)
#define PSRAM_PHY_REG0_PSRAM_RX_DELAY_CTRL_SHIFT            (0)
#define PSRAM_PHY_REG0_PSRAM_RX_DELAY2_CTRL(n)              (((n) & 0xF) << 4)
#define PSRAM_PHY_REG0_PSRAM_RX_DELAY2_CTRL_MASK            (0xF << 4)
#define PSRAM_PHY_REG0_PSRAM_RX_DELAY2_CTRL_SHIFT           (4)
#define PSRAM_PHY_REG0_PSRAM_RX_DELAY_RANGE(n)              (((n) & 0x3) << 8)
#define PSRAM_PHY_REG0_PSRAM_RX_DELAY_RANGE_MASK            (0x3 << 8)
#define PSRAM_PHY_REG0_PSRAM_RX_DELAY_RANGE_SHIFT           (8)
#define PSRAM_PHY_REG0_PSRAM_RX_DELAY_RANGE2(n)             (((n) & 0x3) << 10)
#define PSRAM_PHY_REG0_PSRAM_RX_DELAY_RANGE2_MASK           (0x3 << 10)
#define PSRAM_PHY_REG0_PSRAM_RX_DELAY_RANGE2_SHIFT          (10)
#define PSRAM_PHY_REG0_PSRAM_TXDRV(n)                       (((n) & 0x7) << 12)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_MASK                     (0x7 << 12)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_SHIFT                    (12)
#define PSRAM_PHY_REG0_PSRAM_TXDRV2(n)                      (((n) & 0x7) << 15)
#define PSRAM_PHY_REG0_PSRAM_TXDRV2_MASK                    (0x7 << 15)
#define PSRAM_PHY_REG0_PSRAM_TXDRV2_SHIFT                   (15)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DELAY_RANGE(n)          (((n) & 0x3) << 18)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DELAY_RANGE_MASK        (0x3 << 18)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DELAY_RANGE_SHIFT       (18)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DELAY_RANGE2(n)         (((n) & 0x3) << 20)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DELAY_RANGE2_MASK       (0x3 << 20)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DELAY_RANGE2_SHIFT      (20)

// reg_068
#define PSRAM_PHY_REG1_PSRAM_RX_DELAY_CTRL(n)               (((n) & 0xF) << 0)
#define PSRAM_PHY_REG1_PSRAM_RX_DELAY_CTRL_MASK             (0xF << 0)
#define PSRAM_PHY_REG1_PSRAM_RX_DELAY_CTRL_SHIFT            (0)
#define PSRAM_PHY_REG1_PSRAM_RX_DELAY2_CTRL(n)              (((n) & 0xF) << 4)
#define PSRAM_PHY_REG1_PSRAM_RX_DELAY2_CTRL_MASK            (0xF << 4)
#define PSRAM_PHY_REG1_PSRAM_RX_DELAY2_CTRL_SHIFT           (4)
#define PSRAM_PHY_REG1_PSRAM_RX_DELAY_RANGE(n)              (((n) & 0x3) << 8)
#define PSRAM_PHY_REG1_PSRAM_RX_DELAY_RANGE_MASK            (0x3 << 8)
#define PSRAM_PHY_REG1_PSRAM_RX_DELAY_RANGE_SHIFT           (8)
#define PSRAM_PHY_REG1_PSRAM_RX_DELAY_RANGE2(n)             (((n) & 0x3) << 10)
#define PSRAM_PHY_REG1_PSRAM_RX_DELAY_RANGE2_MASK           (0x3 << 10)
#define PSRAM_PHY_REG1_PSRAM_RX_DELAY_RANGE2_SHIFT          (10)
#define PSRAM_PHY_REG1_PSRAM_TXDRV(n)                       (((n) & 0x7) << 12)
#define PSRAM_PHY_REG1_PSRAM_TXDRV_MASK                     (0x7 << 12)
#define PSRAM_PHY_REG1_PSRAM_TXDRV_SHIFT                    (12)
#define PSRAM_PHY_REG1_PSRAM_TXDRV2(n)                      (((n) & 0x7) << 15)
#define PSRAM_PHY_REG1_PSRAM_TXDRV2_MASK                    (0x7 << 15)
#define PSRAM_PHY_REG1_PSRAM_TXDRV2_SHIFT                   (15)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DELAY_RANGE(n)          (((n) & 0x3) << 18)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DELAY_RANGE_MASK        (0x3 << 18)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DELAY_RANGE_SHIFT       (18)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DELAY_RANGE2(n)         (((n) & 0x3) << 20)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DELAY_RANGE2_MASK       (0x3 << 20)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DELAY_RANGE2_SHIFT      (20)

// reg_07c
#define PSRAM_PHY_REG0_PSRAM_CEB_DELAY_RANGE(n)             (((n) & 0x3) << 0)
#define PSRAM_PHY_REG0_PSRAM_CEB_DELAY_RANGE_MASK           (0x3 << 0)
#define PSRAM_PHY_REG0_PSRAM_CEB_DELAY_RANGE_SHIFT          (0)
#define PSRAM_PHY_REG0_PSRAM_CEB_DELAY_RANGE2(n)            (((n) & 0x3) << 2)
#define PSRAM_PHY_REG0_PSRAM_CEB_DELAY_RANGE2_MASK          (0x3 << 2)
#define PSRAM_PHY_REG0_PSRAM_CEB_DELAY_RANGE2_SHIFT         (2)
#define PSRAM_PHY_REG0_PSRAM_S1_DLY(n)                      (((n) & 0xF) << 4)
#define PSRAM_PHY_REG0_PSRAM_S1_DLY_MASK                    (0xF << 4)
#define PSRAM_PHY_REG0_PSRAM_S1_DLY_SHIFT                   (4)
#define PSRAM_PHY_REG0_PSRAM_S1_DELAY_RANGE(n)              (((n) & 0x3) << 8)
#define PSRAM_PHY_REG0_PSRAM_S1_DELAY_RANGE_MASK            (0x3 << 8)
#define PSRAM_PHY_REG0_PSRAM_S1_DELAY_RANGE_SHIFT           (8)
#define PSRAM_PHY_REG0_PSRAM_S2_DLY(n)                      (((n) & 0xF) << 10)
#define PSRAM_PHY_REG0_PSRAM_S2_DLY_MASK                    (0xF << 10)
#define PSRAM_PHY_REG0_PSRAM_S2_DLY_SHIFT                   (10)
#define PSRAM_PHY_REG0_PSRAM_S2_DELAY_RANGE(n)              (((n) & 0x3) << 14)
#define PSRAM_PHY_REG0_PSRAM_S2_DELAY_RANGE_MASK            (0x3 << 14)
#define PSRAM_PHY_REG0_PSRAM_S2_DELAY_RANGE_SHIFT           (14)

// reg_080
#define PSRAM_PHY_REG1_PSRAM_CEB_DELAY_RANGE(n)             (((n) & 0x3) << 0)
#define PSRAM_PHY_REG1_PSRAM_CEB_DELAY_RANGE_MASK           (0x3 << 0)
#define PSRAM_PHY_REG1_PSRAM_CEB_DELAY_RANGE_SHIFT          (0)
#define PSRAM_PHY_REG1_PSRAM_CEB_DELAY_RANGE2(n)            (((n) & 0x3) << 2)
#define PSRAM_PHY_REG1_PSRAM_CEB_DELAY_RANGE2_MASK          (0x3 << 2)
#define PSRAM_PHY_REG1_PSRAM_CEB_DELAY_RANGE2_SHIFT         (2)
#define PSRAM_PHY_REG1_PSRAM_S1_DLY(n)                      (((n) & 0xF) << 4)
#define PSRAM_PHY_REG1_PSRAM_S1_DLY_MASK                    (0xF << 4)
#define PSRAM_PHY_REG1_PSRAM_S1_DLY_SHIFT                   (4)
#define PSRAM_PHY_REG1_PSRAM_S1_DELAY_RANGE(n)              (((n) & 0x3) << 8)
#define PSRAM_PHY_REG1_PSRAM_S1_DELAY_RANGE_MASK            (0x3 << 8)
#define PSRAM_PHY_REG1_PSRAM_S1_DELAY_RANGE_SHIFT           (8)
#define PSRAM_PHY_REG1_PSRAM_S2_DLY(n)                      (((n) & 0xF) << 10)
#define PSRAM_PHY_REG1_PSRAM_S2_DLY_MASK                    (0xF << 10)
#define PSRAM_PHY_REG1_PSRAM_S2_DLY_SHIFT                   (10)
#define PSRAM_PHY_REG1_PSRAM_S2_DELAY_RANGE(n)              (((n) & 0x3) << 14)
#define PSRAM_PHY_REG1_PSRAM_S2_DELAY_RANGE_MASK            (0x3 << 14)
#define PSRAM_PHY_REG1_PSRAM_S2_DELAY_RANGE_SHIFT           (14)
#define PSRAM_PHY_REG_PSRAM_RESERVED(n)                     (((n) & 0xFF) << 16)
#define PSRAM_PHY_REG_PSRAM_RESERVED_MASK                   (0xFF << 16)
#define PSRAM_PHY_REG_PSRAM_RESERVED_SHIFT                  (16)

// reg_084
#define PSRAM_PHY_PIN_MUX0_TX_DQ0(n)                        (((n) & 0x7) << 0)
#define PSRAM_PHY_PIN_MUX0_TX_DQ0_MASK                      (0x7 << 0)
#define PSRAM_PHY_PIN_MUX0_TX_DQ0_SHIFT                     (0)
#define PSRAM_PHY_PIN_MUX0_TX_DQ1(n)                        (((n) & 0x7) << 3)
#define PSRAM_PHY_PIN_MUX0_TX_DQ1_MASK                      (0x7 << 3)
#define PSRAM_PHY_PIN_MUX0_TX_DQ1_SHIFT                     (3)
#define PSRAM_PHY_PIN_MUX0_TX_DQ2(n)                        (((n) & 0x7) << 6)
#define PSRAM_PHY_PIN_MUX0_TX_DQ2_MASK                      (0x7 << 6)
#define PSRAM_PHY_PIN_MUX0_TX_DQ2_SHIFT                     (6)
#define PSRAM_PHY_PIN_MUX0_TX_DQ3(n)                        (((n) & 0x7) << 9)
#define PSRAM_PHY_PIN_MUX0_TX_DQ3_MASK                      (0x7 << 9)
#define PSRAM_PHY_PIN_MUX0_TX_DQ3_SHIFT                     (9)
#define PSRAM_PHY_PIN_MUX0_TX_DQ4(n)                        (((n) & 0x7) << 12)
#define PSRAM_PHY_PIN_MUX0_TX_DQ4_MASK                      (0x7 << 12)
#define PSRAM_PHY_PIN_MUX0_TX_DQ4_SHIFT                     (12)
#define PSRAM_PHY_PIN_MUX0_TX_DQ5(n)                        (((n) & 0x7) << 15)
#define PSRAM_PHY_PIN_MUX0_TX_DQ5_MASK                      (0x7 << 15)
#define PSRAM_PHY_PIN_MUX0_TX_DQ5_SHIFT                     (15)
#define PSRAM_PHY_PIN_MUX0_TX_DQ6(n)                        (((n) & 0x7) << 18)
#define PSRAM_PHY_PIN_MUX0_TX_DQ6_MASK                      (0x7 << 18)
#define PSRAM_PHY_PIN_MUX0_TX_DQ6_SHIFT                     (18)
#define PSRAM_PHY_PIN_MUX0_TX_DQ7(n)                        (((n) & 0x7) << 21)
#define PSRAM_PHY_PIN_MUX0_TX_DQ7_MASK                      (0x7 << 21)
#define PSRAM_PHY_PIN_MUX0_TX_DQ7_SHIFT                     (21)

// reg_088
#define PSRAM_PHY_PIN_MUX1_TX_DQ0(n)                        (((n) & 0x7) << 0)
#define PSRAM_PHY_PIN_MUX1_TX_DQ0_MASK                      (0x7 << 0)
#define PSRAM_PHY_PIN_MUX1_TX_DQ0_SHIFT                     (0)
#define PSRAM_PHY_PIN_MUX1_TX_DQ1(n)                        (((n) & 0x7) << 3)
#define PSRAM_PHY_PIN_MUX1_TX_DQ1_MASK                      (0x7 << 3)
#define PSRAM_PHY_PIN_MUX1_TX_DQ1_SHIFT                     (3)
#define PSRAM_PHY_PIN_MUX1_TX_DQ2(n)                        (((n) & 0x7) << 6)
#define PSRAM_PHY_PIN_MUX1_TX_DQ2_MASK                      (0x7 << 6)
#define PSRAM_PHY_PIN_MUX1_TX_DQ2_SHIFT                     (6)
#define PSRAM_PHY_PIN_MUX1_TX_DQ3(n)                        (((n) & 0x7) << 9)
#define PSRAM_PHY_PIN_MUX1_TX_DQ3_MASK                      (0x7 << 9)
#define PSRAM_PHY_PIN_MUX1_TX_DQ3_SHIFT                     (9)
#define PSRAM_PHY_PIN_MUX1_TX_DQ4(n)                        (((n) & 0x7) << 12)
#define PSRAM_PHY_PIN_MUX1_TX_DQ4_MASK                      (0x7 << 12)
#define PSRAM_PHY_PIN_MUX1_TX_DQ4_SHIFT                     (12)
#define PSRAM_PHY_PIN_MUX1_TX_DQ5(n)                        (((n) & 0x7) << 15)
#define PSRAM_PHY_PIN_MUX1_TX_DQ5_MASK                      (0x7 << 15)
#define PSRAM_PHY_PIN_MUX1_TX_DQ5_SHIFT                     (15)
#define PSRAM_PHY_PIN_MUX1_TX_DQ6(n)                        (((n) & 0x7) << 18)
#define PSRAM_PHY_PIN_MUX1_TX_DQ6_MASK                      (0x7 << 18)
#define PSRAM_PHY_PIN_MUX1_TX_DQ6_SHIFT                     (18)
#define PSRAM_PHY_PIN_MUX1_TX_DQ7(n)                        (((n) & 0x7) << 21)
#define PSRAM_PHY_PIN_MUX1_TX_DQ7_MASK                      (0x7 << 21)
#define PSRAM_PHY_PIN_MUX1_TX_DQ7_SHIFT                     (21)

// reg_08c
#define PSRAM_PHY_PIN_MUX0_RX_DQ0(n)                        (((n) & 0x7) << 0)
#define PSRAM_PHY_PIN_MUX0_RX_DQ0_MASK                      (0x7 << 0)
#define PSRAM_PHY_PIN_MUX0_RX_DQ0_SHIFT                     (0)
#define PSRAM_PHY_PIN_MUX0_RX_DQ1(n)                        (((n) & 0x7) << 3)
#define PSRAM_PHY_PIN_MUX0_RX_DQ1_MASK                      (0x7 << 3)
#define PSRAM_PHY_PIN_MUX0_RX_DQ1_SHIFT                     (3)
#define PSRAM_PHY_PIN_MUX0_RX_DQ2(n)                        (((n) & 0x7) << 6)
#define PSRAM_PHY_PIN_MUX0_RX_DQ2_MASK                      (0x7 << 6)
#define PSRAM_PHY_PIN_MUX0_RX_DQ2_SHIFT                     (6)
#define PSRAM_PHY_PIN_MUX0_RX_DQ3(n)                        (((n) & 0x7) << 9)
#define PSRAM_PHY_PIN_MUX0_RX_DQ3_MASK                      (0x7 << 9)
#define PSRAM_PHY_PIN_MUX0_RX_DQ3_SHIFT                     (9)
#define PSRAM_PHY_PIN_MUX0_RX_DQ4(n)                        (((n) & 0x7) << 12)
#define PSRAM_PHY_PIN_MUX0_RX_DQ4_MASK                      (0x7 << 12)
#define PSRAM_PHY_PIN_MUX0_RX_DQ4_SHIFT                     (12)
#define PSRAM_PHY_PIN_MUX0_RX_DQ5(n)                        (((n) & 0x7) << 15)
#define PSRAM_PHY_PIN_MUX0_RX_DQ5_MASK                      (0x7 << 15)
#define PSRAM_PHY_PIN_MUX0_RX_DQ5_SHIFT                     (15)
#define PSRAM_PHY_PIN_MUX0_RX_DQ6(n)                        (((n) & 0x7) << 18)
#define PSRAM_PHY_PIN_MUX0_RX_DQ6_MASK                      (0x7 << 18)
#define PSRAM_PHY_PIN_MUX0_RX_DQ6_SHIFT                     (18)
#define PSRAM_PHY_PIN_MUX0_RX_DQ7(n)                        (((n) & 0x7) << 21)
#define PSRAM_PHY_PIN_MUX0_RX_DQ7_MASK                      (0x7 << 21)
#define PSRAM_PHY_PIN_MUX0_RX_DQ7_SHIFT                     (21)

// reg_090
#define PSRAM_PHY_PIN_MUX1_RX_DQ0(n)                        (((n) & 0x7) << 0)
#define PSRAM_PHY_PIN_MUX1_RX_DQ0_MASK                      (0x7 << 0)
#define PSRAM_PHY_PIN_MUX1_RX_DQ0_SHIFT                     (0)
#define PSRAM_PHY_PIN_MUX1_RX_DQ1(n)                        (((n) & 0x7) << 3)
#define PSRAM_PHY_PIN_MUX1_RX_DQ1_MASK                      (0x7 << 3)
#define PSRAM_PHY_PIN_MUX1_RX_DQ1_SHIFT                     (3)
#define PSRAM_PHY_PIN_MUX1_RX_DQ2(n)                        (((n) & 0x7) << 6)
#define PSRAM_PHY_PIN_MUX1_RX_DQ2_MASK                      (0x7 << 6)
#define PSRAM_PHY_PIN_MUX1_RX_DQ2_SHIFT                     (6)
#define PSRAM_PHY_PIN_MUX1_RX_DQ3(n)                        (((n) & 0x7) << 9)
#define PSRAM_PHY_PIN_MUX1_RX_DQ3_MASK                      (0x7 << 9)
#define PSRAM_PHY_PIN_MUX1_RX_DQ3_SHIFT                     (9)
#define PSRAM_PHY_PIN_MUX1_RX_DQ4(n)                        (((n) & 0x7) << 12)
#define PSRAM_PHY_PIN_MUX1_RX_DQ4_MASK                      (0x7 << 12)
#define PSRAM_PHY_PIN_MUX1_RX_DQ4_SHIFT                     (12)
#define PSRAM_PHY_PIN_MUX1_RX_DQ5(n)                        (((n) & 0x7) << 15)
#define PSRAM_PHY_PIN_MUX1_RX_DQ5_MASK                      (0x7 << 15)
#define PSRAM_PHY_PIN_MUX1_RX_DQ5_SHIFT                     (15)
#define PSRAM_PHY_PIN_MUX1_RX_DQ6(n)                        (((n) & 0x7) << 18)
#define PSRAM_PHY_PIN_MUX1_RX_DQ6_MASK                      (0x7 << 18)
#define PSRAM_PHY_PIN_MUX1_RX_DQ6_SHIFT                     (18)
#define PSRAM_PHY_PIN_MUX1_RX_DQ7(n)                        (((n) & 0x7) << 21)
#define PSRAM_PHY_PIN_MUX1_RX_DQ7_MASK                      (0x7 << 21)
#define PSRAM_PHY_PIN_MUX1_RX_DQ7_SHIFT                     (21)

#endif
