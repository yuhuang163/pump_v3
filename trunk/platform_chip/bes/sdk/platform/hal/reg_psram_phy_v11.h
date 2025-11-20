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
#ifndef __REG_PSRAM_PHY_V11_H__
#define __REG_PSRAM_PHY_V11_H__

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
};

// reg_000
#define PSRAM_ULP_PHY_CHIP_TYPE                             (1 << 0)
#define PSRAM_ULP_PHY_CHIP_BIT                              (1 << 1)
#define PSRAM_ULP_PHY_MEMORY_WIDTH(n)                       (((n) & 0x3) << 2)
#define PSRAM_ULP_PHY_MEMORY_WIDTH_MASK                     (0x3 << 2)
#define PSRAM_ULP_PHY_MEMORY_WIDTH_SHIFT                    (2)
#define PSRAM_ULP_PHY_FRE_RATIO(n)                          (((n) & 0x3) << 4)
#define PSRAM_ULP_PHY_FRE_RATIO_MASK                        (0x3 << 4)
#define PSRAM_ULP_PHY_FRE_RATIO_SHIFT                       (4)

// reg_004
#define PSRAM_ULP_PHY_CTRL_DELAY(n)                         (((n) & 0x3) << 0)
#define PSRAM_ULP_PHY_CTRL_DELAY_MASK                       (0x3 << 0)
#define PSRAM_ULP_PHY_CTRL_DELAY_SHIFT                      (0)
#define PSRAM_ULP_PHY_RX_DLY_EN                             (1 << 2)
#define PSRAM_ULP_PHY_ALIGN_BYPASS                          (1 << 3)
#define PSRAM_ULP_PHY_PHY_LOOPBACK_EN                       (1 << 4)
#define PSRAM_ULP_PHY_PHY_DUMMY_CYC_EN                      (1 << 5)
#define PSRAM_ULP_PHY_PHY_DLY_AUTO_EN                       (1 << 6)
#define PSRAM_ULP_PHY_SQPI_SAMPLE_SEL(n)                    (((n) & 0xF) << 7)
#define PSRAM_ULP_PHY_SQPI_SAMPLE_SEL_MASK                  (0xF << 7)
#define PSRAM_ULP_PHY_SQPI_SAMPLE_SEL_SHIFT                 (7)
#define PSRAM_ULP_PHY_IDLE_DQ_OEN                           (1 << 11)
#define PSRAM_ULP_PHY_PHY_TX_BYPASS                         (1 << 12)
#define PSRAM_ULP_PHY_ADDR_4BYTES_EN                        (1 << 13)
#define PSRAM_ULP_PHY_IDLE_DQS_OEN                          (1 << 14)
#define PSRAM_ULP_PHY_DQS_DM_MERGE_EN                       (1 << 15)
#define PSRAM_ULP_PHY_REG_WR_WO_DM                          (1 << 16)

// reg_008
#define PSRAM_ULP_PHY_T_WPST(n)                             (((n) & 0x7) << 0)
#define PSRAM_ULP_PHY_T_WPST_MASK                           (0x7 << 0)
#define PSRAM_ULP_PHY_T_WPST_SHIFT                          (0)

// reg_00c
#define PSRAM_ULP_PHY_T_EXPANDRD(n)                         (((n) & 0x3F) << 0)
#define PSRAM_ULP_PHY_T_EXPANDRD_MASK                       (0x3F << 0)
#define PSRAM_ULP_PHY_T_EXPANDRD_SHIFT                      (0)

// reg_010
#define PSRAM_ULP_PHY_CMD_CONFLICT_CLR                      (1 << 0)

// reg_014
#define PSRAM_ULP_PHY_SQPI_CMD_CONFLICT_CLR                 (1 << 0)

// reg_018
#define PSRAM_ULP_PHY_RX_TIMEOUT_WORKAROUND_EN              (1 << 0)
#define PSRAM_ULP_PHY_RX_TIMEOUT_CNT(n)                     (((n) & 0x3FF) << 1)
#define PSRAM_ULP_PHY_RX_TIMEOUT_CNT_MASK                   (0x3FF << 1)
#define PSRAM_ULP_PHY_RX_TIMEOUT_CNT_SHIFT                  (1)

// reg_01c
#define PSRAM_ULP_PHY_OPI_RX_DQS_VLD_L                      (1 << 0)
#define PSRAM_ULP_PHY_OPI_RX_DQS_VLD_H                      (1 << 1)
#define PSRAM_ULP_PHY_RX_TIMEOUT_ERR                        (1 << 2)

// reg_020
#define PSRAM_ULP_PHY_RX_TIMEOUT_ERR_CLR                    (1 << 0)

// reg_024
#define PSRAM_ULP_PHY_REG_TR_CLK_EN                         (1 << 0)
#define PSRAM_ULP_PHY_REG_TR_DQS_EN                         (1 << 1)

// reg_040
#define PSRAM_ULP_PHY_PHY_RX_BYPASS                         (1 << 0)
#define PSRAM_ULP_PHY_PHY_SAMP_WITH_CLK                     (1 << 1)
#define PSRAM_ULP_PHY_ANA_PHY_BYPASS                        (1 << 2)
#define PSRAM_ULP_PHY_ANA_PHY_LP_CTRL_DR(n)                 (((n) & 0x3F) << 3)
#define PSRAM_ULP_PHY_ANA_PHY_LP_CTRL_DR_MASK               (0x3F << 3)
#define PSRAM_ULP_PHY_ANA_PHY_LP_CTRL_DR_SHIFT              (3)
#define PSRAM_ULP_PHY_ANA_PHY_PWRUP_REQ                     (1 << 9)

// reg_044
#define PSRAM_ULP_PHY_CMD_CONFLICT_STS                      (1 << 0)
#define PSRAM_ULP_PHY_PHY_FSM_STATE(n)                      (((n) & 0xF) << 1)
#define PSRAM_ULP_PHY_PHY_FSM_STATE_MASK                    (0xF << 1)
#define PSRAM_ULP_PHY_PHY_FSM_STATE_SHIFT                   (1)
#define PSRAM_ULP_PHY_SQPI_CMD_CONFLICT_STS                 (1 << 5)
#define PSRAM_ULP_PHY_SQPI_PHY_FSM_STATE(n)                 (((n) & 0xF) << 6)
#define PSRAM_ULP_PHY_SQPI_PHY_FSM_STATE_MASK               (0xF << 6)
#define PSRAM_ULP_PHY_SQPI_PHY_FSM_STATE_SHIFT              (6)
#define PSRAM_ULP_PHY_ANA_PHY_BYPASS_STS                    (1 << 10)

// reg_048
#define PSRAM_ULP_PHY_REG_PSRAM_PU                          (1 << 0)
#define PSRAM_ULP_PHY_REG_PSRAM_S2P_RESETB                  (1 << 1)
#define PSRAM_ULP_PHY_REG_PSRAM_SLEEP_EN                    (1 << 2)

// reg_04c
#define PSRAM_ULP_PHY_REG_PSRAM_DIV2_RESETB                 (1 << 0)
#define PSRAM_ULP_PHY_REG_PSRAM_CAL_CLK_EN                  (1 << 1)
#define PSRAM_ULP_PHY_REG_PSRAM_TXDRV(n)                    (((n) & 0x7) << 2)
#define PSRAM_ULP_PHY_REG_PSRAM_TXDRV_MASK                  (0x7 << 2)
#define PSRAM_ULP_PHY_REG_PSRAM_TXDRV_SHIFT                 (2)
#define PSRAM_ULP_PHY_REG_PSRAM_LOOPBACK_EN                 (1 << 5)

// reg_050
#define PSRAM_ULP_PHY_REG_PSRAM_CEB_DELAY_RANGE(n)          (((n) & 0x3) << 0)
#define PSRAM_ULP_PHY_REG_PSRAM_CEB_DELAY_RANGE_MASK        (0x3 << 0)
#define PSRAM_ULP_PHY_REG_PSRAM_CEB_DELAY_RANGE_SHIFT       (0)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CEB1_DLY(n)              (((n) & 0x7) << 2)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CEB1_DLY_MASK            (0x7 << 2)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CEB1_DLY_SHIFT           (2)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CEB2_DLY(n)              (((n) & 0x7) << 5)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CEB2_DLY_MASK            (0x7 << 5)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CEB2_DLY_SHIFT           (5)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DELAY_RANGE(n)       (((n) & 0x3) << 8)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DELAY_RANGE_MASK     (0x3 << 8)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DELAY_RANGE_SHIFT    (8)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY(n)               (((n) & 0x7) << 10)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY_MASK             (0x7 << 10)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_CLK_DLY_SHIFT            (10)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DELAY_RANGE(n)       (((n) & 0x3) << 13)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DELAY_RANGE_MASK     (0x3 << 13)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DELAY_RANGE_SHIFT    (13)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY(n)               (((n) & 0x7) << 15)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_MASK             (0x7 << 15)
#define PSRAM_ULP_PHY_REG_PSRAM_TX_DQS_DLY_SHIFT            (15)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_DELAY_RANGE(n)           (((n) & 0x3) << 18)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_DELAY_RANGE_MASK         (0x3 << 18)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_DELAY_RANGE_SHIFT        (18)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_DELAY_CTRL(n)            (((n) & 0xF) << 20)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_DELAY_CTRL_MASK          (0xF << 20)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_DELAY_CTRL_SHIFT         (20)

// reg_054
#define PSRAM_ULP_PHY_REG_PSRAM_S1_DELAY_RANGE(n)           (((n) & 0x3) << 0)
#define PSRAM_ULP_PHY_REG_PSRAM_S1_DELAY_RANGE_MASK         (0x3 << 0)
#define PSRAM_ULP_PHY_REG_PSRAM_S1_DELAY_RANGE_SHIFT        (0)
#define PSRAM_ULP_PHY_REG_PSRAM_S1_DLY(n)                   (((n) & 0x7) << 2)
#define PSRAM_ULP_PHY_REG_PSRAM_S1_DLY_MASK                 (0x7 << 2)
#define PSRAM_ULP_PHY_REG_PSRAM_S1_DLY_SHIFT                (2)
#define PSRAM_ULP_PHY_REG_PSRAM_S2_DELAY_RANGE(n)           (((n) & 0x3) << 5)
#define PSRAM_ULP_PHY_REG_PSRAM_S2_DELAY_RANGE_MASK         (0x3 << 5)
#define PSRAM_ULP_PHY_REG_PSRAM_S2_DELAY_RANGE_SHIFT        (5)
#define PSRAM_ULP_PHY_REG_PSRAM_S2_DLY(n)                   (((n) & 0x7) << 7)
#define PSRAM_ULP_PHY_REG_PSRAM_S2_DLY_MASK                 (0x7 << 7)
#define PSRAM_ULP_PHY_REG_PSRAM_S2_DLY_SHIFT                (7)

// reg_058
#define PSRAM_ULP_PHY_PSRAM_TX_CLK_PHASE_CHECK(n)           (((n) & 0xF) << 0)
#define PSRAM_ULP_PHY_PSRAM_TX_CLK_PHASE_CHECK_MASK         (0xF << 0)
#define PSRAM_ULP_PHY_PSRAM_TX_CLK_PHASE_CHECK_SHIFT        (0)
#define PSRAM_ULP_PHY_PSRAM_TX_DQS_PHASE_CHECK(n)           (((n) & 0xF) << 4)
#define PSRAM_ULP_PHY_PSRAM_TX_DQS_PHASE_CHECK_MASK         (0xF << 4)
#define PSRAM_ULP_PHY_PSRAM_TX_DQS_PHASE_CHECK_SHIFT        (4)

// reg_05c
#define PSRAM_ULP_PHY_REG_PSRAM_RESERVED(n)                 (((n) & 0xFF) << 0)
#define PSRAM_ULP_PHY_REG_PSRAM_RESERVED_MASK               (0xFF << 0)
#define PSRAM_ULP_PHY_REG_PSRAM_RESERVED_SHIFT              (0)

// reg_060
#define PSRAM_ULP_PHY_REG_T_INIT0(n)                        (((n) & 0xF) << 0)
#define PSRAM_ULP_PHY_REG_T_INIT0_MASK                      (0xF << 0)
#define PSRAM_ULP_PHY_REG_T_INIT0_SHIFT                     (0)
#define PSRAM_ULP_PHY_REG_T_INIT1(n)                        (((n) & 0xF) << 4)
#define PSRAM_ULP_PHY_REG_T_INIT1_MASK                      (0xF << 4)
#define PSRAM_ULP_PHY_REG_T_INIT1_SHIFT                     (4)
#define PSRAM_ULP_PHY_REG_T_INIT2(n)                        (((n) & 0xF) << 8)
#define PSRAM_ULP_PHY_REG_T_INIT2_MASK                      (0xF << 8)
#define PSRAM_ULP_PHY_REG_T_INIT2_SHIFT                     (8)
#define PSRAM_ULP_PHY_REG_T_INIT3(n)                        (((n) & 0xFF) << 12)
#define PSRAM_ULP_PHY_REG_T_INIT3_MASK                      (0xFF << 12)
#define PSRAM_ULP_PHY_REG_T_INIT3_SHIFT                     (12)
#define PSRAM_ULP_PHY_REG_T_TRANS(n)                        (((n) & 0xF) << 20)
#define PSRAM_ULP_PHY_REG_T_TRANS_MASK                      (0xF << 20)
#define PSRAM_ULP_PHY_REG_T_TRANS_SHIFT                     (20)

// reg_064
#define PSRAM_ULP_PHY_ANA_PHY_CTRL_DR(n)                    (((n) & 0x7) << 0)
#define PSRAM_ULP_PHY_ANA_PHY_CTRL_DR_MASK                  (0x7 << 0)
#define PSRAM_ULP_PHY_ANA_PHY_CTRL_DR_SHIFT                 (0)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_SAMP_CLK_SEL             (1 << 3)
#define PSRAM_ULP_PHY_REG_PSRAM_TXCLK_TO_RXDQS_EN           (1 << 4)
#define PSRAM_ULP_PHY_REG_PSRAM_RX_S2P_EN                   (1 << 5)

// reg_068
#define PSRAM_ULP_PHY_REG_PSRAM_CEB1_PU                     (1 << 0)
#define PSRAM_ULP_PHY_REG_PSRAM_CEB2_PU                     (1 << 1)

// reg_06c
#define PSRAM_ULP_PHY_HYPERBUS_HACK_BIT39                   (1 << 0)
#define PSRAM_ULP_PHY_CS_PREAMPLE_EN                        (1 << 1)
#define PSRAM_ULP_PHY_DQ_IDLE_CMD(n)                        (((n) & 0xFF) << 2)
#define PSRAM_ULP_PHY_DQ_IDLE_CMD_MASK                      (0xFF << 2)
#define PSRAM_ULP_PHY_DQ_IDLE_CMD_SHIFT                     (2)
#define PSRAM_ULP_PHY_HYPERBUS_LAT_CMD_CNT(n)               (((n) & 0x3) << 10)
#define PSRAM_ULP_PHY_HYPERBUS_LAT_CMD_CNT_MASK             (0x3 << 10)
#define PSRAM_ULP_PHY_HYPERBUS_LAT_CMD_CNT_SHIFT            (10)
#define PSRAM_ULP_PHY_HYPERBUS_ADD_LATENCY_EDGE             (1 << 12)
#define PSRAM_ULP_PHY_HYPERBUS_CMD_WITH_LATENCY_EN(n)       (((n) & 0xF) << 13)
#define PSRAM_ULP_PHY_HYPERBUS_CMD_WITH_LATENCY_EN_MASK     (0xF << 13)
#define PSRAM_ULP_PHY_HYPERBUS_CMD_WITH_LATENCY_EN_SHIFT    (13)

// reg_070
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ0(n)                    (((n) & 0x7) << 0)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ0_MASK                  (0x7 << 0)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ0_SHIFT                 (0)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ1(n)                    (((n) & 0x7) << 3)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ1_MASK                  (0x7 << 3)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ1_SHIFT                 (3)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ2(n)                    (((n) & 0x7) << 6)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ2_MASK                  (0x7 << 6)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ2_SHIFT                 (6)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ3(n)                    (((n) & 0x7) << 9)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ3_MASK                  (0x7 << 9)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ3_SHIFT                 (9)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ4(n)                    (((n) & 0x7) << 12)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ4_MASK                  (0x7 << 12)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ4_SHIFT                 (12)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ5(n)                    (((n) & 0x7) << 15)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ5_MASK                  (0x7 << 15)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ5_SHIFT                 (15)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ6(n)                    (((n) & 0x7) << 18)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ6_MASK                  (0x7 << 18)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ6_SHIFT                 (18)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ7(n)                    (((n) & 0x7) << 21)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ7_MASK                  (0x7 << 21)
#define PSRAM_ULP_PHY_PIN_MUX0_TX_DQ7_SHIFT                 (21)

// reg_074
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ0(n)                    (((n) & 0x7) << 0)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ0_MASK                  (0x7 << 0)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ0_SHIFT                 (0)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ1(n)                    (((n) & 0x7) << 3)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ1_MASK                  (0x7 << 3)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ1_SHIFT                 (3)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ2(n)                    (((n) & 0x7) << 6)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ2_MASK                  (0x7 << 6)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ2_SHIFT                 (6)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ3(n)                    (((n) & 0x7) << 9)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ3_MASK                  (0x7 << 9)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ3_SHIFT                 (9)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ4(n)                    (((n) & 0x7) << 12)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ4_MASK                  (0x7 << 12)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ4_SHIFT                 (12)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ5(n)                    (((n) & 0x7) << 15)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ5_MASK                  (0x7 << 15)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ5_SHIFT                 (15)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ6(n)                    (((n) & 0x7) << 18)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ6_MASK                  (0x7 << 18)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ6_SHIFT                 (18)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ7(n)                    (((n) & 0x7) << 21)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ7_MASK                  (0x7 << 21)
#define PSRAM_ULP_PHY_PIN_MUX1_TX_DQ7_SHIFT                 (21)

// reg_078
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ0(n)                    (((n) & 0x7) << 0)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ0_MASK                  (0x7 << 0)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ0_SHIFT                 (0)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ1(n)                    (((n) & 0x7) << 3)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ1_MASK                  (0x7 << 3)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ1_SHIFT                 (3)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ2(n)                    (((n) & 0x7) << 6)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ2_MASK                  (0x7 << 6)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ2_SHIFT                 (6)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ3(n)                    (((n) & 0x7) << 9)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ3_MASK                  (0x7 << 9)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ3_SHIFT                 (9)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ4(n)                    (((n) & 0x7) << 12)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ4_MASK                  (0x7 << 12)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ4_SHIFT                 (12)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ5(n)                    (((n) & 0x7) << 15)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ5_MASK                  (0x7 << 15)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ5_SHIFT                 (15)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ6(n)                    (((n) & 0x7) << 18)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ6_MASK                  (0x7 << 18)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ6_SHIFT                 (18)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ7(n)                    (((n) & 0x7) << 21)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ7_MASK                  (0x7 << 21)
#define PSRAM_ULP_PHY_PIN_MUX0_RX_DQ7_SHIFT                 (21)

// reg_07c
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ0(n)                    (((n) & 0x7) << 0)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ0_MASK                  (0x7 << 0)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ0_SHIFT                 (0)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ1(n)                    (((n) & 0x7) << 3)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ1_MASK                  (0x7 << 3)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ1_SHIFT                 (3)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ2(n)                    (((n) & 0x7) << 6)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ2_MASK                  (0x7 << 6)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ2_SHIFT                 (6)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ3(n)                    (((n) & 0x7) << 9)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ3_MASK                  (0x7 << 9)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ3_SHIFT                 (9)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ4(n)                    (((n) & 0x7) << 12)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ4_MASK                  (0x7 << 12)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ4_SHIFT                 (12)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ5(n)                    (((n) & 0x7) << 15)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ5_MASK                  (0x7 << 15)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ5_SHIFT                 (15)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ6(n)                    (((n) & 0x7) << 18)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ6_MASK                  (0x7 << 18)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ6_SHIFT                 (18)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ7(n)                    (((n) & 0x7) << 21)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ7_MASK                  (0x7 << 21)
#define PSRAM_ULP_PHY_PIN_MUX1_RX_DQ7_SHIFT                 (21)

#endif
