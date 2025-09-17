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

#ifndef __REG_PSRAM_PHY_V6_H__
#define __REG_PSRAM_PHY_V6_H__

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
    __IO uint32_t REG_0D0;
    __IO uint32_t REG_0D4;
    __IO uint32_t REG_0D8;
    __IO uint32_t REG_0DC;
    __IO uint32_t REG_0E0;
    __IO uint32_t REG_0E4;
    __IO uint32_t REG_0E8;
    __IO uint32_t REG_0EC;
    __IO uint32_t REG_0F0;
};

extern struct PSRAM_PHY_T  *const psram_phy[];

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

// reg_028
#define PSRAM_PHY_REG_REF_DDP_EN                            (1 << 0)
#define PSRAM_PHY_REG_REF_DDP_CMD(n)                        (((n) & 0xFF) << 1)
#define PSRAM_PHY_REG_REF_DDP_CMD_MASK                      (0xFF << 1)
#define PSRAM_PHY_REG_REF_DDP_CMD_SHIFT                     (1)

// reg_03c
#define PSRAM_PHY_REG_PHY_PWR_DONE_DR                       (1 << 0)
#define PSRAM_PHY_REG_PHY_PWR_DONE                          (1 << 1)

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
#define PSRAM_PHY_REG_T_TRANS(n)                            (((n) & 0xFFFF) << 0)
#define PSRAM_PHY_REG_T_TRANS_MASK                          (0xFFFF << 0)
#define PSRAM_PHY_REG_T_TRANS_SHIFT                         (0)
#define PSRAM_PHY_REG_T_PRECHARGE(n)                        (((n) & 0xFFFF) << 16)
#define PSRAM_PHY_REG_T_PRECHARGE_MASK                      (0xFFFF << 16)
#define PSRAM_PHY_REG_T_PRECHARGE_SHIFT                     (16)

// reg_04c
#define PSRAM_PHY_REG_T_PU_DVDD(n)                          (((n) & 0xFFFF) << 0)
#define PSRAM_PHY_REG_T_PU_DVDD_MASK                        (0xFFFF << 0)
#define PSRAM_PHY_REG_T_PU_DVDD_SHIFT                       (0)
#define PSRAM_PHY_REG_T_PU_AVDDQ(n)                         (((n) & 0xFFFF) << 16)
#define PSRAM_PHY_REG_T_PU_AVDDQ_MASK                       (0xFFFF << 16)
#define PSRAM_PHY_REG_T_PU_AVDDQ_SHIFT                      (16)

// reg_050
#define PSRAM_PHY_REG_T_LOCK(n)                             (((n) & 0xFFFF) << 0)
#define PSRAM_PHY_REG_T_LOCK_MASK                           (0xFFFF << 0)
#define PSRAM_PHY_REG_T_LOCK_SHIFT                          (0)
#define PSRAM_PHY_REG_T_BYPASS_TA_WAIT(n)                   (((n) & 0x3F) << 16)
#define PSRAM_PHY_REG_T_BYPASS_TA_WAIT_MASK                 (0x3F << 16)
#define PSRAM_PHY_REG_T_BYPASS_TA_WAIT_SHIFT                (16)
#define PSRAM_PHY_REG_PSRAM_LDO_PU_DR                       (1 << 22)
#define PSRAM_PHY_REG_PSRAM_LDO_PRECHARGE_DR                (1 << 23)
#define PSRAM_PHY_REG_PSRAM_PU_DVDD_DR                      (1 << 24)
#define PSRAM_PHY_REG_PSRAM_PU_DVDD2_DR                     (1 << 25)
#define PSRAM_PHY_REG_PSRAM_PU_AVDDQ_DR                     (1 << 26)
#define PSRAM_PHY_REG_PSRAM_PU_AVDDQ2_DR                    (1 << 27)
#define PSRAM_PHY_REG_PSRAM_DLL_RESETB_DR                   (1 << 28)
#define PSRAM_PHY_REG_PSRAM_DLL_CK_RDY_DR                   (1 << 29)
#define PSRAM_PHY_REG_PSRAM_PHY_BYPASS_DR                   (1 << 30)
#define PSRAM_PHY_REG_PHY_PD_TRANS_MASK                     (1 << 31)

// reg_054
#define PSRAM_PHY_PSRAM_DLL_DLY_IN(n)                       (((n) & 0x3F) << 0)
#define PSRAM_PHY_PSRAM_DLL_DLY_IN_MASK                     (0x3F << 0)
#define PSRAM_PHY_PSRAM_DLL_DLY_IN_SHIFT                    (0)
#define PSRAM_PHY_PSRAM_DLL_LOCK                            (1 << 6)
#define PSRAM_PHY_PSRAM_DLL_ALL_ZERO                        (1 << 7)
#define PSRAM_PHY_PSRAM_DLL_ALL_ONE                         (1 << 8)

// reg_058
#define PSRAM_PHY_REG_PSRAM_DLL(n)                          (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG_PSRAM_DLL_MASK                        (0xFF << 0)
#define PSRAM_PHY_REG_PSRAM_DLL_SHIFT                       (0)
#define PSRAM_PHY_REG_PSRAM_DLL_DLY_INI(n)                  (((n) & 0xFF) << 8)
#define PSRAM_PHY_REG_PSRAM_DLL_DLY_INI_MASK                (0xFF << 8)
#define PSRAM_PHY_REG_PSRAM_DLL_DLY_INI_SHIFT               (8)
#define PSRAM_PHY_REG_PSRAM_DLL_CK_RDY                      (1 << 16)
#define PSRAM_PHY_REG_PSRAM_DLL_RANGE(n)                    (((n) & 0x3) << 17)
#define PSRAM_PHY_REG_PSRAM_DLL_RANGE_MASK                  (0x3 << 17)
#define PSRAM_PHY_REG_PSRAM_DLL_RANGE_SHIFT                 (17)
#define PSRAM_PHY_REG_PSRAM_DLL_RESETB                      (1 << 19)
#define PSRAM_PHY_REG_PSRAM_LDO_PRECHARGE                   (1 << 20)
#define PSRAM_PHY_REG_PSRAM_LDO_PU                          (1 << 21)
#define PSRAM_PHY_REG_PSRAM_LDO_RES(n)                      (((n) & 0xF) << 22)
#define PSRAM_PHY_REG_PSRAM_LDO_RES_MASK                    (0xF << 22)
#define PSRAM_PHY_REG_PSRAM_LDO_RES_SHIFT                   (22)
#define PSRAM_PHY_REG_PSRAM_PU_AVDDQ                        (1 << 26)
#define PSRAM_PHY_REG_PSRAM_PU_CEB                          (1 << 27)
#define PSRAM_PHY_REG_PSRAM_PU_DVDD                         (1 << 28)
#define PSRAM_PHY_REG_PSRAM_PU_AVDDQ2                       (1 << 29)
#define PSRAM_PHY_REG_PSRAM_PU_CEB2                         (1 << 30)
#define PSRAM_PHY_REG_PSRAM_PU_DVDD2                        (1 << 31)

// reg_05c
#define PSRAM_PHY_REG0_PSRAM_PSRAM_ITUNE(n)                 (((n) & 0x7) << 0)
#define PSRAM_PHY_REG0_PSRAM_PSRAM_ITUNE_MASK               (0x7 << 0)
#define PSRAM_PHY_REG0_PSRAM_PSRAM_ITUNE_SHIFT              (0)
#define PSRAM_PHY_REG0_PSRAM_ODT(n)                         (((n) & 0x3) << 3)
#define PSRAM_PHY_REG0_PSRAM_ODT_MASK                       (0x3 << 3)
#define PSRAM_PHY_REG0_PSRAM_ODT_SHIFT                      (3)
#define PSRAM_PHY_REG0_PSRAM_PLDN                           (1 << 5)
#define PSRAM_PHY_REG0_PSRAM_PLDN_CEB                       (1 << 6)
#define PSRAM_PHY_REG0_PSRAM_PLDN_DQS(n)                    (((n) & 0x3) << 7)
#define PSRAM_PHY_REG0_PSRAM_PLDN_DQS_MASK                  (0x3 << 7)
#define PSRAM_PHY_REG0_PSRAM_PLDN_DQS_SHIFT                 (7)
#define PSRAM_PHY_REG0_PSRAM_PLDN_OFF                       (1 << 9)
#define PSRAM_PHY_REG0_PSRAM_PLDN_RESET                     (1 << 10)
#define PSRAM_PHY_REG0_PSRAM_PLUP                           (1 << 11)
#define PSRAM_PHY_REG0_PSRAM_PLUP_CEB                       (1 << 12)
#define PSRAM_PHY_REG0_PSRAM_PLUP_DQS(n)                    (((n) & 0x3) << 13)
#define PSRAM_PHY_REG0_PSRAM_PLUP_DQS_MASK                  (0x3 << 13)
#define PSRAM_PHY_REG0_PSRAM_PLUP_DQS_SHIFT                 (13)
#define PSRAM_PHY_REG0_PSRAM_PLUP_RESET                     (1 << 15)
#define PSRAM_PHY_REG0_PSRAM_PSEL_DQS                       (1 << 16)
#define PSRAM_PHY_REG0_PSRAM_PU_RCV                         (1 << 17)
#define PSRAM_PHY_REG0_PSRAM_PU_RESET                       (1 << 18)
#define PSRAM_PHY_REG0_PSRAM_PU_SCHMMITT                    (1 << 19)
#define PSRAM_PHY_REG0_PSRAM_PAD_EN_SEL                     (1 << 20)
#define PSRAM_PHY_REG0_PSRAM_RSTB_SEL                       (1 << 21)
#define PSRAM_PHY_REG0_PSRAM_WR_PUSH_SEL                    (1 << 22)
#define PSRAM_PHY_REG0_PSRAM_PLDN_CLK(n)                    (((n) & 0x3) << 23)
#define PSRAM_PHY_REG0_PSRAM_PLDN_CLK_MASK                  (0x3 << 23)
#define PSRAM_PHY_REG0_PSRAM_PLDN_CLK_SHIFT                 (23)
#define PSRAM_PHY_REG0_PSRAM_PLUP_CLK(n)                    (((n) & 0x3) << 25)
#define PSRAM_PHY_REG0_PSRAM_PLUP_CLK_MASK                  (0x3 << 25)
#define PSRAM_PHY_REG0_PSRAM_PLUP_CLK_SHIFT                 (25)
#define PSRAM_PHY_REG0_PSRAM_DATA_RDY_SEL                   (1 << 27)

// reg_060
#define PSRAM_PHY_REG0_PSRAM_RANGE_CK(n)                    (((n) & 0x3) << 0)
#define PSRAM_PHY_REG0_PSRAM_RANGE_CK_MASK                  (0x3 << 0)
#define PSRAM_PHY_REG0_PSRAM_RANGE_CK_SHIFT                 (0)
#define PSRAM_PHY_REG0_PSRAM_RX_BIAS(n)                     (((n) & 0x3) << 2)
#define PSRAM_PHY_REG0_PSRAM_RX_BIAS_MASK                   (0x3 << 2)
#define PSRAM_PHY_REG0_PSRAM_RX_BIAS_SHIFT                  (2)
#define PSRAM_PHY_REG0_PSRAM_RX_VREF_TUNE(n)                (((n) & 0x1F) << 4)
#define PSRAM_PHY_REG0_PSRAM_RX_VREF_TUNE_MASK              (0x1F << 4)
#define PSRAM_PHY_REG0_PSRAM_RX_VREF_TUNE_SHIFT             (4)
#define PSRAM_PHY_REG0_PSRAM_SWRC(n)                        (((n) & 0x3) << 9)
#define PSRAM_PHY_REG0_PSRAM_SWRC_MASK                      (0x3 << 9)
#define PSRAM_PHY_REG0_PSRAM_SWRC_SHIFT                     (9)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_CK_DLY(n)                (((n) & 0x7) << 11)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_CK_DLY_MASK              (0x7 << 11)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_CK_DLY_SHIFT             (11)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_CK_PSEL                  (1 << 14)
#define PSRAM_PHY_REG0_PSRAM_TX_WR_CK_PSEL                  (1 << 15)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_POINTER_DLY(n)           (((n) & 0x7) << 16)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_POINTER_DLY_MASK         (0x7 << 16)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_POINTER_DLY_SHIFT        (16)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_POINTER_PSEL             (1 << 19)
#define PSRAM_PHY_REG0_PSRAM_TXDRV(n)                       (((n) & 0x7) << 20)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_MASK                     (0x7 << 20)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_SHIFT                    (20)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_CLK(n)                   (((n) & 0x7) << 23)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_CLK_MASK                 (0x7 << 23)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_CLK_SHIFT                (23)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_DQS(n)                   (((n) & 0x7) << 26)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_DQS_MASK                 (0x7 << 26)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_DQS_SHIFT                (26)

// reg_064
#define PSRAM_PHY_REG0_PSRAM_TX_CEB_DLY(n)                  (((n) & 0x1F) << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CEB_DLY_MASK                (0x1F << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CEB_DLY_SHIFT               (0)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DLY(n)                  (((n) & 0x1F) << 5)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DLY_MASK                (0x1F << 5)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DLY_SHIFT               (5)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DLY(n)                  (((n) & 0x1F) << 10)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DLY_MASK                (0x1F << 10)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DLY_SHIFT               (10)
#define PSRAM_PHY_REG0_PSRAM_TX_DM_DLY(n)                   (((n) & 0x1F) << 15)
#define PSRAM_PHY_REG0_PSRAM_TX_DM_DLY_MASK                 (0x1F << 15)
#define PSRAM_PHY_REG0_PSRAM_TX_DM_DLY_SHIFT                (15)
#define PSRAM_PHY_REG0_PSRAM_RX_DQS_DLY(n)                  (((n) & 0x1F) << 20)
#define PSRAM_PHY_REG0_PSRAM_RX_DQS_DLY_MASK                (0x1F << 20)
#define PSRAM_PHY_REG0_PSRAM_RX_DQS_DLY_SHIFT               (20)
#define PSRAM_PHY_REG0_PSRAM_RX_CLK_DLY(n)                  (((n) & 0x1F) << 25)
#define PSRAM_PHY_REG0_PSRAM_RX_CLK_DLY_MASK                (0x1F << 25)
#define PSRAM_PHY_REG0_PSRAM_RX_CLK_DLY_SHIFT               (25)

// reg_068
#define PSRAM_PHY_REG0_PSRAM_TX_DQ_DLY_0_31(n)              (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_DQ_DLY_0_31_MASK            (0xFFFFFFFF << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_DQ_DLY_0_31_SHIFT           (0)

// reg_06c
#define PSRAM_PHY_REG0_PSRAM_TX_DQ_DLY_32_39(n)             (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_DQ_DLY_32_39_MASK           (0xFF << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_DQ_DLY_32_39_SHIFT          (0)

// reg_070
#define PSRAM_PHY_REG0_PSRAM_TX_CMD_DLY_0_31(n)             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CMD_DLY_0_31_MASK           (0xFFFFFFFF << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CMD_DLY_0_31_SHIFT          (0)

// reg_074
#define PSRAM_PHY_REG0_PSRAM_TX_CMD_DLY_32_39(n)            (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CMD_DLY_32_39_MASK          (0xFF << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CMD_DLY_32_39_SHIFT         (0)

// reg_078
#define PSRAM_PHY_REG0_PSRAM_RX_ADQ_DLY_0_31(n)             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_PHY_REG0_PSRAM_RX_ADQ_DLY_0_31_MASK           (0xFFFFFFFF << 0)
#define PSRAM_PHY_REG0_PSRAM_RX_ADQ_DLY_0_31_SHIFT          (0)

// reg_07c
#define PSRAM_PHY_REG0_PSRAM_RX_ADQ_DLY_32_39(n)            (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG0_PSRAM_RX_ADQ_DLY_32_39_MASK          (0xFF << 0)
#define PSRAM_PHY_REG0_PSRAM_RX_ADQ_DLY_32_39_SHIFT         (0)

// reg_080
#define PSRAM_PHY_REG0_RESERVED(n)                          (((n) & 0x7) << 0)
#define PSRAM_PHY_REG0_RESERVED_MASK                        (0x7 << 0)
#define PSRAM_PHY_REG0_RESERVED_SHIFT                       (0)
#define PSRAM_PHY_REG0_PSRAM_ODT2(n)                        (((n) & 0x3) << 3)
#define PSRAM_PHY_REG0_PSRAM_ODT2_MASK                      (0x3 << 3)
#define PSRAM_PHY_REG0_PSRAM_ODT2_SHIFT                     (3)
#define PSRAM_PHY_REG0_PSRAM_PLDN2                          (1 << 5)
#define PSRAM_PHY_REG0_PSRAM_PLDN_CEB2                      (1 << 6)
#define PSRAM_PHY_REG0_PSRAM_PLDN_DQS2(n)                   (((n) & 0x3) << 7)
#define PSRAM_PHY_REG0_PSRAM_PLDN_DQS2_MASK                 (0x3 << 7)
#define PSRAM_PHY_REG0_PSRAM_PLDN_DQS2_SHIFT                (7)
#define PSRAM_PHY_REG0_PSRAM_PLDN_OFF2                      (1 << 9)
#define PSRAM_PHY_REG0_PSRAM_PLDN_RESET2                    (1 << 10)
#define PSRAM_PHY_REG0_PSRAM_PLUP2                          (1 << 11)
#define PSRAM_PHY_REG0_PSRAM_PLUP_CEB2                      (1 << 12)
#define PSRAM_PHY_REG0_PSRAM_PLUP_DQS2(n)                   (((n) & 0x3) << 13)
#define PSRAM_PHY_REG0_PSRAM_PLUP_DQS2_MASK                 (0x3 << 13)
#define PSRAM_PHY_REG0_PSRAM_PLUP_DQS2_SHIFT                (13)
#define PSRAM_PHY_REG0_PSRAM_PLUP_RESET2                    (1 << 15)
#define PSRAM_PHY_REG0_PSRAM_PSEL_DQS2                      (1 << 16)
#define PSRAM_PHY_REG0_PSRAM_PU_RCV2                        (1 << 17)
#define PSRAM_PHY_REG0_PSRAM_PU_RESET2                      (1 << 18)
#define PSRAM_PHY_REG0_PSRAM_PU_SCHMMITT2                   (1 << 19)
#define PSRAM_PHY_REG0_PSRAM_PAD_EN_SEL2                    (1 << 20)
#define PSRAM_PHY_REG0_PSRAM_RSTB_SEL2                      (1 << 21)
#define PSRAM_PHY_REG0_PSRAM_WR_PUSH_SEL2                   (1 << 22)
#define PSRAM_PHY_REG0_PSRAM_PLDN_CLK2(n)                   (((n) & 0x3) << 23)
#define PSRAM_PHY_REG0_PSRAM_PLDN_CLK2_MASK                 (0x3 << 23)
#define PSRAM_PHY_REG0_PSRAM_PLDN_CLK2_SHIFT                (23)
#define PSRAM_PHY_REG0_PSRAM_PLUP_CLK2(n)                   (((n) & 0x3) << 25)
#define PSRAM_PHY_REG0_PSRAM_PLUP_CLK2_MASK                 (0x3 << 25)
#define PSRAM_PHY_REG0_PSRAM_PLUP_CLK2_SHIFT                (25)
#define PSRAM_PHY_REG0_PSRAM_DATA_RDY_SEL2                  (1 << 27)

// reg_084
#define PSRAM_PHY_REG0_PSRAM_RANGE_CK2(n)                   (((n) & 0x3) << 0)
#define PSRAM_PHY_REG0_PSRAM_RANGE_CK2_MASK                 (0x3 << 0)
#define PSRAM_PHY_REG0_PSRAM_RANGE_CK2_SHIFT                (0)
#define PSRAM_PHY_REG0_PSRAM_RX_BIAS2(n)                    (((n) & 0x3) << 2)
#define PSRAM_PHY_REG0_PSRAM_RX_BIAS2_MASK                  (0x3 << 2)
#define PSRAM_PHY_REG0_PSRAM_RX_BIAS2_SHIFT                 (2)
#define PSRAM_PHY_REG0_PSRAM_RX_VREF_TUNE2(n)               (((n) & 0x1F) << 4)
#define PSRAM_PHY_REG0_PSRAM_RX_VREF_TUNE2_MASK             (0x1F << 4)
#define PSRAM_PHY_REG0_PSRAM_RX_VREF_TUNE2_SHIFT            (4)
#define PSRAM_PHY_REG0_PSRAM_SWRC2(n)                       (((n) & 0x3) << 9)
#define PSRAM_PHY_REG0_PSRAM_SWRC2_MASK                     (0x3 << 9)
#define PSRAM_PHY_REG0_PSRAM_SWRC2_SHIFT                    (9)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_CK_DLY2(n)               (((n) & 0x7) << 11)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_CK_DLY2_MASK             (0x7 << 11)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_CK_DLY2_SHIFT            (11)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_CK_PSEL2                 (1 << 14)
#define PSRAM_PHY_REG0_PSRAM_TX_WR_CK_PSEL2                 (1 << 15)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_POINTER_DLY2(n)          (((n) & 0x7) << 16)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_POINTER_DLY2_MASK        (0x7 << 16)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_POINTER_DLY2_SHIFT       (16)
#define PSRAM_PHY_REG0_PSRAM_TX_RD_POINTER_PSEL2            (1 << 19)
#define PSRAM_PHY_REG0_PSRAM_TXDRV2(n)                      (((n) & 0x7) << 20)
#define PSRAM_PHY_REG0_PSRAM_TXDRV2_MASK                    (0x7 << 20)
#define PSRAM_PHY_REG0_PSRAM_TXDRV2_SHIFT                   (20)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_CLK2(n)                  (((n) & 0x7) << 23)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_CLK2_MASK                (0x7 << 23)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_CLK2_SHIFT               (23)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_DQS2(n)                  (((n) & 0x7) << 26)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_DQS2_MASK                (0x7 << 26)
#define PSRAM_PHY_REG0_PSRAM_TXDRV_DQS2_SHIFT               (26)

// reg_088
#define PSRAM_PHY_REG0_PSRAM_TX_CEB_DLY2(n)                 (((n) & 0x1F) << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CEB_DLY2_MASK               (0x1F << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CEB_DLY2_SHIFT              (0)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DLY2(n)                 (((n) & 0x1F) << 5)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DLY2_MASK               (0x1F << 5)
#define PSRAM_PHY_REG0_PSRAM_TX_CLK_DLY2_SHIFT              (5)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DLY2(n)                 (((n) & 0x1F) << 10)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DLY2_MASK               (0x1F << 10)
#define PSRAM_PHY_REG0_PSRAM_TX_DQS_DLY2_SHIFT              (10)
#define PSRAM_PHY_REG0_PSRAM_TX_DM_DLY2(n)                  (((n) & 0x1F) << 15)
#define PSRAM_PHY_REG0_PSRAM_TX_DM_DLY2_MASK                (0x1F << 15)
#define PSRAM_PHY_REG0_PSRAM_TX_DM_DLY2_SHIFT               (15)
#define PSRAM_PHY_REG0_PSRAM_RX_DQS_DLY2(n)                 (((n) & 0x1F) << 20)
#define PSRAM_PHY_REG0_PSRAM_RX_DQS_DLY2_MASK               (0x1F << 20)
#define PSRAM_PHY_REG0_PSRAM_RX_DQS_DLY2_SHIFT              (20)
#define PSRAM_PHY_REG0_PSRAM_RX_CLK_DLY2(n)                 (((n) & 0x1F) << 25)
#define PSRAM_PHY_REG0_PSRAM_RX_CLK_DLY2_MASK               (0x1F << 25)
#define PSRAM_PHY_REG0_PSRAM_RX_CLK_DLY2_SHIFT              (25)

// reg_08c
#define PSRAM_PHY_REG0_PSRAM_TX_DQ_DLY2_0_31(n)             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_DQ_DLY2_0_31_MASK           (0xFFFFFFFF << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_DQ_DLY2_0_31_SHIFT          (0)

// reg_090
#define PSRAM_PHY_REG0_PSRAM_TX_DQ_DLY2_32_39(n)            (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_DQ_DLY2_32_39_MASK          (0xFF << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_DQ_DLY2_32_39_SHIFT         (0)

// reg_094
#define PSRAM_PHY_REG0_PSRAM_TX_CMD_DLY2_0_31(n)            (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CMD_DLY2_0_31_MASK          (0xFFFFFFFF << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CMD_DLY2_0_31_SHIFT         (0)

// reg_098
#define PSRAM_PHY_REG0_PSRAM_TX_CMD_DLY2_32_39(n)           (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CMD_DLY2_32_39_MASK         (0xFF << 0)
#define PSRAM_PHY_REG0_PSRAM_TX_CMD_DLY2_32_39_SHIFT        (0)

// reg_09c
#define PSRAM_PHY_REG0_PSRAM_RX_ADQ_DLY2_0_31(n)            (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_PHY_REG0_PSRAM_RX_ADQ_DLY2_0_31_MASK          (0xFFFFFFFF << 0)
#define PSRAM_PHY_REG0_PSRAM_RX_ADQ_DLY2_0_31_SHIFT         (0)

// reg_0a0
#define PSRAM_PHY_REG0_PSRAM_RX_ADQ_DLY2_32_39(n)           (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG0_PSRAM_RX_ADQ_DLY2_32_39_MASK         (0xFF << 0)
#define PSRAM_PHY_REG0_PSRAM_RX_ADQ_DLY2_32_39_SHIFT        (0)

// reg_0a4
#define PSRAM_PHY_REG1_PSRAM_DLL_RANGE(n)                   (((n) & 0x3) << 0)
#define PSRAM_PHY_REG1_PSRAM_DLL_RANGE_MASK                 (0x3 << 0)
#define PSRAM_PHY_REG1_PSRAM_DLL_RANGE_SHIFT                (0)
#define PSRAM_PHY_REG1_PSRAM_RANGE_CK(n)                    (((n) & 0x3) << 2)
#define PSRAM_PHY_REG1_PSRAM_RANGE_CK_MASK                  (0x3 << 2)
#define PSRAM_PHY_REG1_PSRAM_RANGE_CK_SHIFT                 (2)
#define PSRAM_PHY_REG1_PSRAM_RANGE_CK2(n)                   (((n) & 0x3) << 4)
#define PSRAM_PHY_REG1_PSRAM_RANGE_CK2_MASK                 (0x3 << 4)
#define PSRAM_PHY_REG1_PSRAM_RANGE_CK2_SHIFT                (4)
#define PSRAM_PHY_REG1_PSRAM_PU_SCHMMITT                    (1 << 6)
#define PSRAM_PHY_REG1_PSRAM_PU_SCHMMITT2                   (1 << 7)
#define PSRAM_PHY_REG1_PSRAM_PU_RCV                         (1 << 8)
#define PSRAM_PHY_REG1_PSRAM_PU_RCV2                        (1 << 9)


// reg_0ac
#define PSRAM_PHY_REG1_PSRAM_TX_CEB_DLY(n)                  (((n) & 0x1F) << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CEB_DLY_MASK                (0x1F << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CEB_DLY_SHIFT               (0)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DLY(n)                  (((n) & 0x1F) << 5)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DLY_MASK                (0x1F << 5)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DLY_SHIFT               (5)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DLY(n)                  (((n) & 0x1F) << 10)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DLY_MASK                (0x1F << 10)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DLY_SHIFT               (10)
#define PSRAM_PHY_REG1_PSRAM_TX_DM_DLY(n)                   (((n) & 0x1F) << 15)
#define PSRAM_PHY_REG1_PSRAM_TX_DM_DLY_MASK                 (0x1F << 15)
#define PSRAM_PHY_REG1_PSRAM_TX_DM_DLY_SHIFT                (15)
#define PSRAM_PHY_REG1_PSRAM_RX_DQS_DLY(n)                  (((n) & 0x1F) << 20)
#define PSRAM_PHY_REG1_PSRAM_RX_DQS_DLY_MASK                (0x1F << 20)
#define PSRAM_PHY_REG1_PSRAM_RX_DQS_DLY_SHIFT               (20)
#define PSRAM_PHY_REG1_PSRAM_RX_CLK_DLY(n)                  (((n) & 0x1F) << 25)
#define PSRAM_PHY_REG1_PSRAM_RX_CLK_DLY_MASK                (0x1F << 25)
#define PSRAM_PHY_REG1_PSRAM_RX_CLK_DLY_SHIFT               (25)

// reg_0b0
#define PSRAM_PHY_REG1_PSRAM_TX_DQ_DLY_0_31(n)              (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_DQ_DLY_0_31_MASK            (0xFFFFFFFF << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_DQ_DLY_0_31_SHIFT           (0)

// reg_0b4
#define PSRAM_PHY_REG1_PSRAM_TX_DQ_DLY_32_39(n)             (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_DQ_DLY_32_39_MASK           (0xFF << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_DQ_DLY_32_39_SHIFT          (0)

// reg_0b8
#define PSRAM_PHY_REG1_PSRAM_TX_CMD_DLY_0_31(n)             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CMD_DLY_0_31_MASK           (0xFFFFFFFF << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CMD_DLY_0_31_SHIFT          (0)

// reg_0bc
#define PSRAM_PHY_REG1_PSRAM_TX_CMD_DLY_32_39(n)            (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CMD_DLY_32_39_MASK          (0xFF << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CMD_DLY_32_39_SHIFT         (0)

// reg_0c0
#define PSRAM_PHY_REG1_PSRAM_RX_ADQ_DLY_0_31(n)             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_PHY_REG1_PSRAM_RX_ADQ_DLY_0_31_MASK           (0xFFFFFFFF << 0)
#define PSRAM_PHY_REG1_PSRAM_RX_ADQ_DLY_0_31_SHIFT          (0)

// reg_0c4
#define PSRAM_PHY_REG1_PSRAM_RX_ADQ_DLY_32_39(n)            (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG1_PSRAM_RX_ADQ_DLY_32_39_MASK          (0xFF << 0)
#define PSRAM_PHY_REG1_PSRAM_RX_ADQ_DLY_32_39_SHIFT         (0)


// reg_0d0
#define PSRAM_PHY_REG1_PSRAM_TX_CEB_DLY2(n)                 (((n) & 0x1F) << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CEB_DLY2_MASK               (0x1F << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CEB_DLY2_SHIFT              (0)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DLY2(n)                 (((n) & 0x1F) << 5)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DLY2_MASK               (0x1F << 5)
#define PSRAM_PHY_REG1_PSRAM_TX_CLK_DLY2_SHIFT              (5)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DLY2(n)                 (((n) & 0x1F) << 10)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DLY2_MASK               (0x1F << 10)
#define PSRAM_PHY_REG1_PSRAM_TX_DQS_DLY2_SHIFT              (10)
#define PSRAM_PHY_REG1_PSRAM_TX_DM_DLY2(n)                  (((n) & 0x1F) << 15)
#define PSRAM_PHY_REG1_PSRAM_TX_DM_DLY2_MASK                (0x1F << 15)
#define PSRAM_PHY_REG1_PSRAM_TX_DM_DLY2_SHIFT               (15)
#define PSRAM_PHY_REG1_PSRAM_RX_DQS_DLY2(n)                 (((n) & 0x1F) << 20)
#define PSRAM_PHY_REG1_PSRAM_RX_DQS_DLY2_MASK               (0x1F << 20)
#define PSRAM_PHY_REG1_PSRAM_RX_DQS_DLY2_SHIFT              (20)
#define PSRAM_PHY_REG1_PSRAM_RX_CLK_DLY2(n)                 (((n) & 0x1F) << 25)
#define PSRAM_PHY_REG1_PSRAM_RX_CLK_DLY2_MASK               (0x1F << 25)
#define PSRAM_PHY_REG1_PSRAM_RX_CLK_DLY2_SHIFT              (25)

// reg_0d4
#define PSRAM_PHY_REG1_PSRAM_TX_DQ_DLY2_0_31(n)             (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_DQ_DLY2_0_31_MASK           (0xFFFFFFFF << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_DQ_DLY2_0_31_SHIFT          (0)

// reg_0d8
#define PSRAM_PHY_REG1_PSRAM_TX_DQ_DLY2_32_39(n)            (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_DQ_DLY2_32_39_MASK          (0xFF << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_DQ_DLY2_32_39_SHIFT         (0)

// reg_0dc
#define PSRAM_PHY_REG1_PSRAM_TX_CMD_DLY2_0_31(n)            (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CMD_DLY2_0_31_MASK          (0xFFFFFFFF << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CMD_DLY2_0_31_SHIFT         (0)

// reg_0e0
#define PSRAM_PHY_REG1_PSRAM_TX_CMD_DLY2_32_39(n)           (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CMD_DLY2_32_39_MASK         (0xFF << 0)
#define PSRAM_PHY_REG1_PSRAM_TX_CMD_DLY2_32_39_SHIFT        (0)

// reg_0e4
#define PSRAM_PHY_REG1_PSRAM_RX_ADQ_DLY2_0_31(n)            (((n) & 0xFFFFFFFF) << 0)
#define PSRAM_PHY_REG1_PSRAM_RX_ADQ_DLY2_0_31_MASK          (0xFFFFFFFF << 0)
#define PSRAM_PHY_REG1_PSRAM_RX_ADQ_DLY2_0_31_SHIFT         (0)

// reg_0e8
#define PSRAM_PHY_REG1_PSRAM_RX_ADQ_DLY2_32_39(n)           (((n) & 0xFF) << 0)
#define PSRAM_PHY_REG1_PSRAM_RX_ADQ_DLY2_32_39_MASK         (0xFF << 0)
#define PSRAM_PHY_REG1_PSRAM_RX_ADQ_DLY2_32_39_SHIFT        (0)

// reg_0ec
#define PSRAM_PHY_REG_PSRAM_PU_VCORE_DR                     (1 << 0)
#define PSRAM_PHY_REG_PSRAM_PU_VCORE2_DR                    (1 << 1)
#define PSRAM_PHY_REG_PSRAM_PU_VCORE                        (1 << 2)
#define PSRAM_PHY_REG_PSRAM_PU_VCORE2                       (1 << 3)

// reg_0f0
#define PSRAM_PHY_REG_PSRAM_PU_CEB_DR                       (1 << 0)
#define PSRAM_PHY_REG_PSRAM_PU_CEB2_DR                      (1 << 1)
#define PSRAM_PHY_REG_PSRAM_PAD_EN_SEL_DR                   (1 << 2)
#define PSRAM_PHY_REG_PSRAM_PAD_EN_SEL2_DR                  (1 << 3)

#endif /* __REG_PSRAM_PHY_V6_H__ */
