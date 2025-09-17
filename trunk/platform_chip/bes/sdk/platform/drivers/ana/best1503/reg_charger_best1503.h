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
#ifndef __REG_PMU_BEST1503_H__
#define __REG_PMU_BEST1503_H__

#include "plat_types.h"

enum CHG_REG_T {
    CHG_REG_METAL_ID                    = 0x00,
    CHG_REG_VBAT_DIV_CFG                = 0x01,
    CHG_REG_AC_IN_PP_CFG                = 0x03,
    CHG_REG_VRECHG_ITERM_PRECHG_V_CFG   = 0x05,
    CHG_REG_CV_CFG_07                   = 0x07,
    CHG_REG_CHG_FUNC_CFG                = 0x08,
    CHG_REG_CC_PRE_CV_CFG               = 0x09,
    CHG_REG_TRIM_VREF_CFG               = 0x13,
    CHG_REG_EFUSE_CFG_20                = 0x20,
    CHG_REG_EFUSE_CFG_22                = 0x22,
    CHG_REG_EFUSE_CFG_23                = 0x23,
    CHG_REG_EFUSE_INIT_DONE_CFG         = 0x24,
    CHG_PATTERN0_RST_CHARGER_CFG        = 0x30,
    CHG_PATTERN1_SHIPMODE_ON_CFG        = 0x32,
    CHG_PATTERN2_SHIPMODE_OFF_CFG       = 0x34,
    CHG_PATTERN3_SINGLE_UART_ON_CFG     = 0x36,
    CHG_PATTERN4_SINGLE_UART_OFF_CFG    = 0x38,
    CHG_REG_AC_ON_DET_INTR_CFG          = 0x40,
    CHG_REG_EFUSE_INIT_INTR_CFG         = 0x50,
    CHG_REG_ICC_HALF_INTF_PMU           = 0x58,
    CHG_REG_SHIP_MODE_VRTC_CFG          = 0x59,
    CHG_REG_STANDBY_SHIP_MODE           = 0x5A,
    CHG_REG_FAST_TRI_TIMEOUT_INTR_CFG   = 0x60,
    CHG_REG_TEMP_STATUS_INTR_CFG        = 0x63,
    CHG_REG_CHG_DONE_INTR_CFG           = 0x64,
    CHG_REG_OTP_ACIN_VSYS_SC_INTR_CFG   = 0x65,
    CHG_REG_VSYS_LOW_INTR_CFG           = 0x66,
    CHG_REG_ITERM_POS_DB_CNT_CFG        = 0x6D,
    CHG_REG_ITERM_POS_DB_CNT2_CFG       = 0x6E,
    CHG_REG_CHG_STATUS1                 = 0x7B,
    CHG_REG_CHG_STATUS2                 = 0x7C,
    CHG_REG_CHG_STATUS3                 = 0x7D,
    CHG_REG_CHG_INTR1                   = 0x80,
    CHG_REG_CHG_INTR2                   = 0x81,
    CHG_REG_MAIN_PU_CHG_STATE_INTR_EN   = 0x82,
    CHG_REG_MAIN_PU_CHG_STATE_INTR_MASK = 0x83,
    CHG_REG_MAIN_PU_CHG_STATE_INTR_RAW  = 0x84,
    CHG_REG_MAIN_PU_CHG_STATE_INTR_CLR  = 0x85,
};

// REG_82
#define MAIN_STATE_INTR_EN_SHIFT                        0
#define MAIN_STATE_INTR_EN_MASK                         (0xF << MAIN_STATE_INTR_EN_SHIFT)
#define MAIN_STATE_INTR_EN(n)                           BITFIELD_VAL(MAIN_STATE_INTR_EN, n)
#define PU_STATE_INTR_EN_SHIFT                          4
#define PU_STATE_INTR_EN_MASK                           (0x3F << PU_STATE_INTR_EN_SHIFT)
#define PU_STATE_INTR_EN(n)                             BITFIELD_VAL(PU_STATE_INTR_EN, n)
#define CHRG_STATE_INTR_EN_SHIFT                        10
#define CHRG_STATE_INTR_EN_MASK                         (0x3F << CHRG_STATE_INTR_EN_SHIFT)
#define CHRG_STATE_INTR_EN(n)                           BITFIELD_VAL(CHRG_STATE_INTR_EN, n)

// REG_00
#define REVID_CHARGER_SHIFT                             0
#define REVID_CHARGER_MASK                              (0xF << REVID_CHARGER_SHIFT)
#define REVID_CHARGER(n)                                BITFIELD_VAL(REVID_CHARGER, n)

// REG_01
#define LPO_FTRIM_SHIFT                                 0
#define LPO_FTRIM_MASK                                  (0xF << LPO_FTRIM_SHIFT)
#define LPO_FTRIM(n)                                    BITFIELD_VAL(LPO_FTRIM, n)
#define LPO_KTRIM_SHIFT                                 4
#define LPO_KTRIM_MASK                                  (0x7 << LPO_KTRIM_SHIFT)
#define LPO_KTRIM(n)                                    BITFIELD_VAL(LPO_KTRIM, n)

// REG_03
#define REG_EN_ACIN_OV_DET                              (1 << 6)
#define REG_EN_ACIN_OV_DET_DR                           (1 << 7)
#define REG_CHARGER_RECHARGE_EN                         (1 << 13)
#define REG_CHARGER_RECHARGE_EN_DR                      (1 << 14)

// REG_04
#define REG_BG_CORE_EN                                  (1 << 0)
#define REG_BG_CORE_EN_DR                               (1 << 1)
#define REG_BG_REF_GEN_EN                               (1 << 2)
#define REG_BG_REF_GEN_EN_DR                            (1 << 3)
#define REG_BG_EN_OTP                                   (1 << 4)
#define REG_BG_EN_OTP_DR                                (1 << 5)
#define BG_OTP_SEL_SHIFT                                6
#define BG_OTP_SEL_MASK                                 (0x3 << BG_OTP_SEL_SHIFT)
#define BG_OTP_SEL(n)                                   BITFIELD_VAL(BG_OTP_SEL, n)

// REG_05
#define CHARGER_ITERM_SEL_SHIFT                         1
#define CHARGER_ITERM_SEL_MASK                          (0xF << CHARGER_ITERM_SEL_SHIFT)
#define CHARGER_ITERM_SEL(n)                            BITFIELD_VAL(CHARGER_ITERM_SEL, n)
#define CHARGER_VRECHARGE_SEL_SHIFT                     5
#define CHARGER_VRECHARGE_SEL_MASK                      (0x3 << CHARGER_VRECHARGE_SEL_SHIFT)
#define CHARGER_VRECHARGE_SEL(n)                        BITFIELD_VAL(CHARGER_VRECHARGE_SEL, n)
#define CHARGER_VREF_PRECHARGE_SEL_SHIFT                7
#define CHARGER_VREF_PRECHARGE_SEL_MASK                 (0x3 << CHARGER_VREF_PRECHARGE_SEL_SHIFT)
#define CHARGER_VREF_PRECHARGE_SEL(n)                   BITFIELD_VAL(CHARGER_VREF_PRECHARGE_SEL, n)

// REG_06
#define CHARGER_CC_COMP_CAP_SHIFT                       0
#define CHARGER_CC_COMP_CAP_MASK                        (0x7 << CHARGER_CC_COMP_CAP_SHIFT)
#define CHARGER_CC_COMP_CAP(n)                          BITFIELD_VAL(CHARGER_CC_COMP_CAP, n)
#define CHARGER_CC_COMP_RES_SHIFT                       3
#define CHARGER_CC_COMP_RES_MASK                        (0x7 << CHARGER_CC_COMP_RES_SHIFT)
#define CHARGER_CC_COMP_RES(n)                          BITFIELD_VAL(CHARGER_CC_COMP_RES, n)
#define CHARGER_CC_GM_SEL_SHIFT                         6
#define CHARGER_CC_GM_SEL_MASK                          (0x7 << CHARGER_CC_GM_SEL_SHIFT)
#define CHARGER_CC_GM_SEL(n)                            BITFIELD_VAL(CHARGER_CC_GM_SEL, n)

// REG_07
#define CHARGER_CV_COMP_CAP_SHIFT                       0
#define CHARGER_CV_COMP_CAP_MASK                        (0x7 << CHARGER_CV_COMP_CAP_SHIFT)
#define CHARGER_CV_COMP_CAP(n)                          BITFIELD_VAL(CHARGER_CV_COMP_CAP, n)
#define CHARGER_CV_COMP_RES_SHIFT                       3
#define CHARGER_CV_COMP_RES_MASK                        (0x7 << CHARGER_CV_COMP_RES_SHIFT)
#define CHARGER_CV_COMP_RES(n)                          BITFIELD_VAL(CHARGER_CV_COMP_RES, n)
#define CHARGER_CV_GM_SEL_SHIFT                         6
#define CHARGER_CV_GM_SEL_MASK                          (0x7 << CHARGER_CV_GM_SEL_SHIFT)
#define CHARGER_CV_GM_SEL(n)                            BITFIELD_VAL(CHARGER_CV_GM_SEL, n)

// REG_08
#define REG_CHARGER_CHARGE_EN                           (1 << 0)
#define REG_CHARGER_CHARGE_EN_DR                        (1 << 1)
#define REG_CHARGER_TRICKLE_CHARGE_EN                   (1 << 2)
#define REG_CHARGER_TRICKLE_CHARGE_EN_DR                (1 << 3)
#define REG_CHARGER_CC_CV_CHARGE_EN                     (1 << 4)
#define REG_CHARGER_CC_CV_CHARGE_EN_DR                  (1 << 5)
#define REG_CHARGER_PRECHARGE_EN                        (1 << 8)
#define REG_CHARGER_PRECHARGE_EN_DR                     (1 << 9)
#define REG_CHARGER_ICHARGE_RAMP_CNT_SHIFT              10
#define REG_CHARGER_ICHARGE_RAMP_CNT_MASK               (0x3F << REG_CHARGER_ICHARGE_RAMP_CNT_SHIFT)
#define REG_CHARGER_ICHARGE_RAMP_CNT(n)                 BITFIELD_VAL(REG_CHARGER_ICHARGE_RAMP_CNT, n)

// REG_09
#define REG_CHARGER_VOREG_BIT_SHIFT                     0
#define REG_CHARGER_VOREG_BIT_MASK                      (0x1F << REG_CHARGER_VOREG_BIT_SHIFT)
#define REG_CHARGER_VOREG_BIT(n)                        BITFIELD_VAL(REG_CHARGER_VOREG_BIT, n)
#define REG_CHARGER_IPRE_SEL_SHIFT                      6
#define REG_CHARGER_IPRE_SEL_MASK                       (0xF << REG_CHARGER_IPRE_SEL_SHIFT)
#define REG_CHARGER_IPRE_SEL(n)                         BITFIELD_VAL(REG_CHARGER_IPRE_SEL, n)
#define REG_CHARGER_ICC_SEL_SHIFT                       10
#define REG_CHARGER_ICC_SEL_MASK                        (0xF << REG_CHARGER_ICC_SEL_SHIFT)
#define REG_CHARGER_ICC_SEL(n)                          BITFIELD_VAL(REG_CHARGER_ICC_SEL, n)
#define REG_CHARGER_ICC_SEL_DR                          (1 << 14)

// REG_0A
#define REG_CLK_32K_SEL                                 (1 << 0)
#define REG_CLK_32K_SEL_DR                              (1 << 1)
#define CHARGER_TEST_SEL_SHIFT                          2
#define CHARGER_TEST_SEL_MASK                           (0x7 << CHARGER_TEST_SEL_SHIFT)
#define CHARGER_TEST_SEL(n)                             BITFIELD_VAL(CHARGER_TEST_SEL, n)
#define TEST_MODE                                       (1 << 5)

// REG_10
#define REG_TRIM_BGTEMP_SHIFT                           0
#define REG_TRIM_BGTEMP_MASK                            (0xF << REG_TRIM_BGTEMP_SHIFT)
#define REG_TRIM_BGTEMP(n)                              BITFIELD_VAL(REG_TRIM_BGTEMP, n)
#define REG_TRIM_BGTEMP_DR                              (1 << 4)
#define REG_TRIM_VBG_SHIFT                              5
#define REG_TRIM_VBG_MASK                               (0x3F << REG_TRIM_VBG_SHIFT)
#define REG_TRIM_VBG(n)                                 BITFIELD_VAL(REG_TRIM_VBG, n)
#define REG_TRIM_VBG_DR                                 (1 << 11)

// REG_11
#define REG_TRIM_IREF_RES_SHIFT                         5
#define REG_TRIM_IREF_RES_MASK                          (0xF << REG_TRIM_IREF_RES_SHIFT)
#define REG_TRIM_IREF_RES(n)                            BITFIELD_VAL(REG_TRIM_IREF_RES, n)
#define REG_TRIM_CSOS_SHIFT                             9
#define REG_TRIM_CSOS_MASK                              (0x1F << REG_TRIM_CSOS_SHIFT)
#define REG_TRIM_CSOS(n)                                BITFIELD_VAL(REG_TRIM_CSOS, n)

// REG_12
#define REG_TRIM_ICHARGE_RES_SHIFT                      0
#define REG_TRIM_ICHARGE_RES_MASK                       (0x7 << REG_TRIM_ICHARGE_RES_SHIFT)
#define REG_TRIM_ICHARGE_RES(n)                         BITFIELD_VAL(REG_TRIM_ICHARGE_RES, n)
#define REG_TRIM_ICHARGE_RES_DR                         (1 << 3)
#define REG_TRIM_ITERM_RES_SHIFT                        4
#define REG_TRIM_ITERM_RES_MASK                         (0x7 << REG_TRIM_ITERM_RES_SHIFT)
#define REG_TRIM_ITERM_RES(n)                           BITFIELD_VAL(REG_TRIM_ITERM_RES, n)
#define REG_TRIM_ITERM_RES_DR                           (1 << 7)

// REG_13
#define REG_TRIM_VREF_PRE_SHIFT                         0
#define REG_TRIM_VREF_PRE_MASK                          (0x7 << REG_TRIM_VREF_PRE_SHIFT)
#define REG_TRIM_VREF_PRE(n)                            BITFIELD_VAL(REG_TRIM_VREF_PRE, n)
#define REG_TRIM_VREF_PRE_DR                            (1 << 3)
#define REG_TRIM_VREF_CC_SHIFT                          4
#define REG_TRIM_VREF_CC_MASK                           (0x7 << REG_TRIM_VREF_CC_SHIFT)
#define REG_TRIM_VREF_CC(n)                             BITFIELD_VAL(REG_TRIM_VREF_CC, n)
#define REG_TRIM_VREF_CC_DR                             (1 << 7)
#define REG_TRIM_VOREG_SHIFT                            8
#define REG_TRIM_VOREG_MASK                             (0x1F << REG_TRIM_VOREG_SHIFT)
#define REG_TRIM_VOREG(n)                               BITFIELD_VAL(REG_TRIM_VOREG, n)
#define REG_TRIM_VOREG_DR                               (1 << 13)

// REG_20
#define EFUSE_TRIM_BGTEMP_SHIFT                         0
#define EFUSE_TRIM_BGTEMP_MASK                          (0xF << EFUSE_TRIM_BGTEMP_SHIFT)
#define EFUSE_TRIM_BGTEMP(n)                            BITFIELD_VAL(EFUSE_TRIM_BGTEMP, n)
#define EFUSE_TRIM_BGTEMP_DR                            (1 << 4)
#define EFUSE_TRIM_VBG_SHIFT                            5
#define EFUSE_TRIM_VBG_MASK                             (0x3F << EFUSE_TRIM_VBG_SHIFT)
#define EFUSE_TRIM_VBG(n)                               BITFIELD_VAL(EFUSE_TRIM_VBG, n)
#define EFUSE_TRIM_VBG_DR                               (1 << 11)

// REG_22
#define EFUSE_TRIM_ICHARGE_RES_SHIFT                    0
#define EFUSE_TRIM_ICHARGE_RES_MASK                     (0x7 << EFUSE_TRIM_ICHARGE_RES_SHIFT)
#define EFUSE_TRIM_ICHARGE_RES(n)                       BITFIELD_VAL(EFUSE_TRIM_ICHARGE_RES, n)
#define EFUSE_TRIM_ICHARGE_RES_DR                       (1 << 3)
#define EFUSE_TRIM_ITERM_RES_SHIFT                      4
#define EFUSE_TRIM_ITERM_RES_MASK                       (0x7 << EFUSE_TRIM_ITERM_RES_SHIFT)
#define EFUSE_TRIM_ITERM_RES(n)                         BITFIELD_VAL(EFUSE_TRIM_ITERM_RES, n)
#define EFUSE_TRIM_ITERM_RES_DR                         (1 << 7)

// REG_23
#define EFUSE_TRIM_VREF_PRE_SHIFT                       0
#define EFUSE_TRIM_VREF_PRE_MASK                        (0x7 << EFUSE_TRIM_VREF_PRE_SHIFT)
#define EFUSE_TRIM_VREF_PRE(n)                          BITFIELD_VAL(EFUSE_TRIM_VREF_PRE, n)
#define EFUSE_TRIM_VREF_PRE_DR                          (1 << 3)
#define EFUSE_TRIM_VREF_CC_SHIFT                        4
#define EFUSE_TRIM_VREF_CC_MASK                         (0x7 << EFUSE_TRIM_VREF_CC_SHIFT)
#define EFUSE_TRIM_VREF_CC(n)                           BITFIELD_VAL(EFUSE_TRIM_VREF_CC, n)
#define EFUSE_TRIM_VREF_CC_DR                           (1 << 7)
#define EFUSE_TRIM_VOREG_SHIFT                          8
#define EFUSE_TRIM_VOREG_MASK                           (0x1F << EFUSE_TRIM_VOREG_SHIFT)
#define EFUSE_TRIM_VOREG(n)                             BITFIELD_VAL(EFUSE_TRIM_VOREG, n)
#define EFUSE_TRIM_VOREG_DR                             (1 << 13)

// REG_30
#define REG_COMMAND_0_ENABLE                            (1 << 0)
#define REG_COMMAND_0_BIT_WIDTH_TOLERANCE_SHIFT         1
#define REG_COMMAND_0_BIT_WIDTH_TOLERANCE_MASK          (0xFF << REG_COMMAND_0_BIT_WIDTH_TOLERANCE_SHIFT)
#define REG_COMMAND_0_BIT_WIDTH_TOLERANCE(n)            BITFIELD_VAL(REG_COMMAND_0_BIT_WIDTH_TOLERANCE, n)

// REG_31
#define REG_COMMAND_0_TIME_SHIFT                        0
#define REG_COMMAND_0_TIME_MASK                         0xFFF << REG_COMMAND_0_TIME_SHIFT)
#define REG_COMMAND_0_TIME(n)                           BITFIELD_VAL(REG_COMMAND_0_TIME, n)

// REG_36
#define REG_COMMAND_3_ENABLE                            (1 << 0)
#define REG_COMMAND_3_BIT_WIDTH_TOLERANCE_SHIFT         1
#define REG_COMMAND_3_BIT_WIDTH_TOLERANCE_MASK          (0xFF << REG_COMMAND_3_BIT_WIDTH_TOLERANCE_SHIFT)
#define REG_COMMAND_3_BIT_WIDTH_TOLERANCE(n)            BITFIELD_VAL(REG_COMMAND_3_BIT_WIDTH_TOLERANCE, n)
#define REG_COMMAND_3_TIME_SHIFT                        9
#define REG_COMMAND_3_TIME_MASK                         (0x3F << REG_COMMAND_3_TIME_SHIFT)
#define REG_COMMAND_3_TIME(n)                           BITFIELD_VAL(REG_COMMAND_3_TIME, n)

// REG_38
#define REG_COMMAND_4_ENABLE                            (1 << 0)
#define REG_COMMAND_4_BIT_WIDTH_TOLERANCE_SHIFT         1
#define REG_COMMAND_4_BIT_WIDTH_TOLERANCE_MASK          (0xFF << REG_COMMAND_4_BIT_WIDTH_TOLERANCE_SHIFT)
#define REG_COMMAND_4_BIT_WIDTH_TOLERANCE(n)            BITFIELD_VAL(REG_COMMAND_4_BIT_WIDTH_TOLERANCE, n)
#define REG_COMMAND_4_TIME_SHIFT                        9
#define REG_COMMAND_4_TIME_MASK                         (0x3F << REG_COMMAND_4_TIME_SHIFT)
#define REG_COMMAND_4_TIME(n)                           BITFIELD_VAL(REG_COMMAND_4_TIME, n)

// REG_3A
#define REG_RESETN_CHIP_DB_CNT_SHIFT                    0
#define REG_RESETN_CHIP_DB_CNT_MASK                     (0xF << REG_RESETN_CHIP_DB_CNT_SHIFT)
#define REG_RESETN_CHIP_DB_CNT(n)                       BITFIELD_VAL(REG_RESETN_CHIP_DB_CNT, n)
#define REG_RESETN_CHIP_DB_EN                           (1 << 4)

// REG_3B
#define REG_CLK_LPO_STABLE_CNT_SHIFT                    0
#define REG_CLK_LPO_STABLE_CNT_MASK                     (0xF << REG_CLK_LPO_STABLE_CNT_SHIFT)
#define REG_CLK_LPO_STABLE_CNT(n)                       BITFIELD_VAL(REG_CLK_LPO_STABLE_CNT, n)
#define REG_CLK_32K_PMU_STABLE_CNT_SHIFT                4
#define REG_CLK_32K_PMU_STABLE_CNT_MASK                 (0xF << REG_CLK_32K_PMU_STABLE_CNT_SHIFT)
#define REG_CLK_32K_PMU_STABLE_CNT(n)                   BITFIELD_VAL(REG_CLK_32K_PMU_STABLE_CNT, n)

// REG_40
#define REG_AC_ON_DET_IN_INTR_RAW                       (1 << 0)
#define REG_AC_ON_DET_IN_INTR_EN                        (1 << 1)
#define REG_AC_ON_DET_IN_INTR_MASK                      (1 << 2)
#define REG_AC_ON_DET_IN_INTR_CLR                       (1 << 3)
#define REG_AC_ON_DET_OUT_INTR_RAW                      (1 << 8)
#define REG_AC_ON_DET_OUT_INTR_EN                       (1 << 9)
#define REG_AC_ON_DET_OUT_INTR_MASK                     (1 << 10)
#define REG_AC_ON_DET_OUT_INTR_CLR                      (1 << 11)

#define REG_CHARGE_AC_ON_DET_IN_INTR_RAW                REG_AC_ON_DET_IN_INTR_RAW
#define REG_CHARGE_AC_ON_DET_IN_INTR_EN                 REG_AC_ON_DET_IN_INTR_EN
#define REG_CHARGE_AC_ON_DET_IN_INTR_MASK               REG_AC_ON_DET_IN_INTR_MASK
#define REG_CHARGE_AC_ON_DET_IN_INTR_CLR                REG_AC_ON_DET_IN_INTR_CLR
#define REG_CHARGE_AC_ON_DET_OUT_INTR_RAW               REG_AC_ON_DET_OUT_INTR_RAW
#define REG_CHARGE_AC_ON_DET_OUT_INTR_EN                REG_AC_ON_DET_OUT_INTR_EN
#define REG_CHARGE_AC_ON_DET_OUT_INTR_MASK              REG_AC_ON_DET_OUT_INTR_MASK
#define REG_CHARGE_AC_ON_DET_OUT_INTR_CLR               REG_AC_ON_DET_OUT_INTR_CLR

// REG_41
#define REG_AC_ON_EN                                    (1 << 0)
#define REG_AC_ON_POWEROFF_EN                           (1 << 1)

// REG_42
#define REG_AC_ON_POS_DB_CNT_SHIFT                      0
#define REG_AC_ON_POS_DB_CNT_MASK                       (0x7F << REG_AC_ON_POS_DB_CNT_SHIFT)
#define REG_AC_ON_POS_DB_CNT(n)                         BITFIELD_VAL(REG_AC_ON_POS_DB_CNT, n)
#define REG_AC_ON_NEG_DB_CNT_SHIFT                      7
#define REG_AC_ON_NEG_DB_CNT_MASK                       (0x7F << REG_AC_ON_NEG_DB_CNT_SHIFT)
#define REG_AC_ON_NEG_DB_CNT(n)                         BITFIELD_VAL(REG_AC_ON_NEG_DB_CNT, n)

// REG_43
#define REG_SW_POWER_OFF                                (1 << 0)
#define REG_SW_POWER_OFF_CNT_SHIFT                      1
#define REG_SW_POWER_OFF_CNT_MASK                       (0xF << REG_SW_POWER_OFF_CNT_SHIFT)
#define REG_SW_POWER_OFF_CNT(n)                         BITFIELD_VAL(REG_SW_POWER_OFF_CNT, n)
#define REG_LPO_OFF_CNT_SHIFT                           5
#define REG_LPO_OFF_CNT_MASK                            (0xFF << REG_LPO_OFF_CNT_SHIFT)
#define REG_LPO_OFF_CNT(n)                              BITFIELD_VAL(REG_LPO_OFF_CNT, n)
#define REG_PU_LPO                                      (1 << 13)
#define REG_PU_LPO_DR                                   (1 << 14)

// REG_47
#define REG_PU_CORE_CNT_SHIFT                           0
#define REG_PU_CORE_CNT_MASK                            (0x1FFF << REG_PU_CORE_CNT_SHIFT)
#define REG_PU_CORE_CNT(n)                              BITFIELD_VAL(REG_PU_CORE_CNT, n)

// REG_48
#define REG_PU_EN_DET_CNT_SHIFT                         0
#define REG_PU_EN_DET_CNT_MASK                          (0x1FFF << REG_PU_EN_DET_CNT_SHIFT)
#define REG_PU_EN_DET_CNT(n)                            BITFIELD_VAL(REG_PU_EN_DET_CNT, n)

// REG_49
#define REG_PU_VDET_CNT_SHIFT                           0
#define REG_PU_VDET_CNT_MASK                            (0x1FFF << REG_PU_VDET_CNT_SHIFT)
#define REG_PU_VDET_CNT(n)                              BITFIELD_VAL(REG_PU_VDET_CNT, n)

// REG_4D
#define REG_PU_CHARGE_CNT_SHIFT                         0
#define REG_PU_CHARGE_CNT_MASK                          (0x1FFF << REG_PU_CHARGE_CNT_SHIFT)
#define REG_PU_CHARGE_CNT(n)                               BITFIELD_VAL(REG_PU_CHARGE_CNT, n)

// REG_4F
#define REG_OTP_EN                                      (1 << 0)
#define REG_MAX_SUB3V_BUF_EN                            (1 << 5)
#define REG_MAX_SUB3V_BUF_EN_DR                         (1 << 6)
#define REG_SUB3V_CNT_SHIFT                             7
#define REG_SUB3V_CNT_MASK                              (0x3F << REG_SUB3V_CNT_SHIFT)
#define REG_SUB3V_CNT(n)                                BITFIELD_VAL(REG_SUB3V_CNT, n)
#define REG_ACIN_OV_EN                                  (1 << 14)

// REG_58
#define REG_EN_BAT_ISINK250U                            (1 << 0)
#define REG_EN_BAT_ISINK250U_DR                         (1 << 1)
#define REG_NTC_HALF_ICC                                (1 << 4)

// REG_5A
#define REG_STANDBY_SHIP_MODE_EN                        (1 << 0)
#define REG_STANDBY_SHIP_MODE                           (1 << 1)
#define REG_UART_STANDBY_MODE_EN                        (1 << 2)
#define REG_UART_STANDBY_MODE                           (1 << 3)
#define REG_DONE_SW_RECHARGE_EN                         (1 << 4)
#define REG_DONE_SW_RECHARGE                            (1 << 5)
#define REG_FAULT_SW_RECHARGE_EN                        (1 << 6)
#define REG_FAULT_SW_RECHARGE                           (1 << 7)
#define REG_FAULT2DONE_EN                               (1 << 8)
#define REG_FAULT2DONE                                  (1 << 9)

// REG_5D
#define REG_CHARGE_OFF_CNT_SHIFT                        0
#define REG_CHARGE_OFF_CNT_MASK                         (0xFF << REG_CHARGE_OFF_CNT_SHIFT)
#define REG_CHARGE_OFF_CNT(n)                           BITFIELD_VAL(REG_CHARGE_OFF_CNT, n)

// REG_60
#define REG_CHARGE_TRIPRE_TIMEOUT_INTR_RAW              (1 << 0)
#define REG_CHARGE_TRIPRE_TIMEOUT_INTR_EN               (1 << 1)
#define REG_CHARGE_TRIPRE_TIMEOUT_INTR_MASK             (1 << 2)
#define REG_CHARGE_TRIPRE_TIMEOUT_INTR_CLR              (1 << 3)
#define REG_CHARGE_FAST_TIMEOUT_INTR_RAW                (1 << 4)
#define REG_CHARGE_FAST_TIMEOUT_INTR_EN                 (1 << 5)
#define REG_CHARGE_FAST_TIMEOUT_INTR_MASK               (1 << 6)
#define REG_CHARGE_FAST_TIMEOUT_INTR_CLR                (1 << 7)

// REG_61
#define REG_CHARGE_TRIPRE_TIMEOUT_CNT_SHIFT             0
#define REG_CHARGE_TRIPRE_TIMEOUT_CNT_MASK              (0x3FFF << REG_CHARGE_TRIPRE_TIMEOUT_CNT_SHIFT)
#define REG_CHARGE_TRIPRE_TIMEOUT_CNT(n)                BITFIELD_VAL(REG_CHARGE_TRIPRE_TIMEOUT_CNT, n)

// REG_62
#define REG_CHARGE_FAST_TIMEOUT_CNT_SHIFT               0
#define REG_CHARGE_FAST_TIMEOUT_CNT_MASK                (0x3FFF << REG_CHARGE_FAST_TIMEOUT_CNT_SHIFT)
#define REG_CHARGE_FAST_TIMEOUT_CNT(n)                  BITFIELD_VAL(REG_CHARGE_FAST_TIMEOUT_CNT, n)

// REG_65
#define REG_ACIN_OV_INTR_RAW                            (1 << 4)
#define REG_ACIN_OV_INTR_EN                             (1 << 5)
#define REG_ACIN_OV_INTR_MASK                           (1 << 6)
#define REG_ACIN_OV_INTR_CLR                            (1 << 7)
#define REG_OTP_INTR_RAW                                (1 << 12)
#define REG_OTP_INTR_EN                                 (1 << 13)
#define REG_OTP_INTR_MASK                               (1 << 14)
#define REG_OTP_INTR_CLR                                (1 << 15)

#define REG_CHARGE_ACIN_OV_INTR_RAW                     REG_ACIN_OV_INTR_RAW
#define REG_CHARGE_ACIN_OV_INTR_EN                      REG_ACIN_OV_INTR_EN
#define REG_CHARGE_ACIN_OV_INTR_MASK                    REG_ACIN_OV_INTR_MASK
#define REG_CHARGE_ACIN_OV_INTR_CLR                     REG_ACIN_OV_INTR_CLR
#define REG_CHARGE_OTP_INTR_RAW                         REG_OTP_INTR_RAW
#define REG_CHARGE_OTP_INTR_EN                          REG_OTP_INTR_EN
#define REG_CHARGE_OTP_INTR_MASK                        REG_OTP_INTR_MASK
#define REG_CHARGE_OTP_INTR_CLR                         REG_OTP_INTR_CLR

// REG_6A
#define REG_VBAT_LOW_POS_DB_CNT_SHIFT                   0
#define REG_VBAT_LOW_POS_DB_CNT_MASK                    (0x7F << REG_VBAT_LOW_POS_DB_CNT_SHIFT)
#define REG_VBAT_LOW_POS_DB_CNT(n)                      BITFIELD_VAL(REG_VBAT_LOW_POS_DB_CNT, n)
#define REG_VBAT_LOW_NEG_DB_CNT_SHIFT                   7
#define REG_VBAT_LOW_NEG_DB_CNT_MASK                    (0xF << REG_VBAT_LOW_NEG_DB_CNT_SHIFT)
#define REG_VBAT_LOW_NEG_DB_CNT(n)                      BITFIELD_VAL(REG_VBAT_LOW_NEG_DB_CNT, n)

// REG_6B
#define REG_PRE2CC_POS_DB_CNT_SHIFT                     0
#define REG_PRE2CC_POS_DB_CNT_MASK                      (0xF << REG_PRE2CC_POS_DB_CNT_SHIFT)
#define REG_PRE2CC_POS_DB_CNT(n)                        BITFIELD_VAL(REG_PRE2CC_POS_DB_CNT, n)
#define REG_PRE2CC_NEG_DB_CNT_SHIFT                     4
#define REG_PRE2CC_NEG_DB_CNT_MASK                      (0x7F << REG_PRE2CC_NEG_DB_CNT_SHIFT)
#define REG_PRE2CC_NEG_DB_CNT(n)                        BITFIELD_VAL(REG_PRE2CC_NEG_DB_CNT, n)

// REG_6C
#define REG_RECHARGE_POS_DB_CNT_SHIFT                   0
#define REG_RECHARGE_POS_DB_CNT_MASK                    (0x3FF << REG_RECHARGE_POS_DB_CNT_SHIFT)
#define REG_RECHARGE_POS_DB_CNT(n)                      BITFIELD_VAL(REG_RECHARGE_POS_DB_CNT, n)
#define REG_RECHARGE_NEG_DB_CNT                         (1 << 10)

// REG_6D
#define REG_ITERM_POS_DB_CNT_15_0_SHIFT                 0
#define REG_ITERM_POS_DB_CNT_15_0_MASK                  (0xFFFF << REG_ITERM_POS_DB_CNT_15_0_SHIFT)
#define REG_ITERM_POS_DB_CNT_15_0(n)                    BITFIELD_VAL(REG_ITERM_POS_DB_CNT_15_0, n)

// REG_6E
#define REG_ITERM_POS_DB_CNT_20_16_SHIFT                0
#define REG_ITERM_POS_DB_CNT_20_16_MASK                 (0x1F << REG_ITERM_POS_DB_CNT_20_16_SHIFT)
#define REG_ITERM_POS_DB_CNT_20_16(n)                   BITFIELD_VAL(REG_ITERM_POS_DB_CNT_20_16, n)

// REG_72
#define REG_CV_MODE_POS_DB_CNT_SHIFT                    0
#define REG_CV_MODE_POS_DB_CNT_MASK                     (0x7F << REG_CV_MODE_POS_DB_CNT_SHIFT)
#define REG_CV_MODE_POS_DB_CNT(n)                       BITFIELD_VAL(REG_CV_MODE_POS_DB_CNT, n)
#define REG_CV_MODE_NEG_DB_CNT_SHIFT                    7
#define REG_CV_MODE_NEG_DB_CNT_MASK                     (0x7F << REG_CV_MODE_NEG_DB_CNT_SHIFT)
#define REG_CV_MODE_NEG_DB_CNT(n)                       BITFIELD_VAL(REG_CV_MODE_NEG_DB_CNT, n)

// REG_73
#define REG_OTP_POS_DB_CNT_SHIFT                        0
#define REG_OTP_POS_DB_CNT_MASK                         (0x7F << REG_OTP_POS_DB_CNT_SHIFT)
#define REG_OTP_POS_DB_CNT(n)                           BITFIELD_VAL(REG_OTP_POS_DB_CNT, n)
#define REG_OTP_NEG_DB_CNT                              (1 << 7)

// REG_74
#define REG_ACIN_OV_POS_DB_CNT                          (1 << 0)
#define REG_ACIN_OV_NEG_DB_CNT_SHIFT                    1
#define REG_ACIN_OV_NEG_DB_CNT_MASK                     (0x1F << REG_ACIN_OV_NEG_DB_CNT_SHIFT)
#define REG_ACIN_OV_NEG_DB_CNT(n)                       BITFIELD_VAL(REG_ACIN_OV_NEG_DB_CNT, n)

// REG_75
#define REG_ACIN_OK_POS_DB_CNT_SHIFT                    0
#define REG_ACIN_OK_POS_DB_CNT_MASK                     (0x7F << REG_ACIN_OK_POS_DB_CNT_SHIFT)
#define REG_ACIN_OK_POS_DB_CNT(n)                       BITFIELD_VAL(REG_ACIN_OK_POS_DB_CNT, n)
#define REG_ACIN_OK_NEG_DB_CNT_SHIFT                    7
#define REG_ACIN_OK_NEG_DB_CNT_MASK                     (0x7F << REG_ACIN_OK_NEG_DB_CNT_SHIFT)
#define REG_ACIN_OK_NEG_DB_CNT(n)                       BITFIELD_VAL(REG_ACIN_OK_NEG_DB_CNT, n)

// REG_76
#define RESERVED_DIG_15_0_SHIFT                         0
#define RESERVED_DIG_15_0_MASK                          (0xFFFF << RESERVED_DIG_15_0_SHIFT)
#define RESERVED_DIG_15_0(n)                            BITFIELD_VAL(RESERVED_DIG_15_0, n)

// REG_77
#define RESERVED_DIG_31_16_SHIFT                        0
#define RESERVED_DIG_31_16_MASK                         (0xFFFF << RESERVED_DIG_31_16_SHIFT)
#define RESERVED_DIG_31_16(n)                           BITFIELD_VAL(RESERVED_DIG_31_16, n)

// REG_78
#define RESERVED_ANA_8_0_SHIFT                          0
#define RESERVED_ANA_8_0_MASK                           (0x1FF << RESERVED_ANA_8_0_SHIFT)
#define RESERVED_ANA_8_0(n)                             BITFIELD_VAL(RESERVED_ANA_8_0, n)

// REG_79
#define RESERVED_ANA_17_9_SHIFT                         0
#define RESERVED_ANA_17_9_MASK                          (0x1FF << RESERVED_ANA_17_9_SHIFT)
#define RESERVED_ANA_17_9(n)                            BITFIELD_VAL(RESERVED_ANA_17_9, n)

// REG_7A
#define CHIP_ADDR_I2C_SHIFT                             0
#define CHIP_ADDR_I2C_MASK                              (0x7F << CHIP_ADDR_I2C_SHIFT)
#define CHIP_ADDR_I2C(n)                                BITFIELD_VAL(CHIP_ADDR_I2C, n)

// REG_7B
#define AC_ON                                           (1 << 0)
#define AC_ON_DB                                        (1 << 1)
#define PRE2CC                                          (1 << 2)
#define PRE2CC_DB                                       (1 << 3)
#define ITERM                                           (1 << 4)
#define ITERM_DB                                        (1 << 5)
#define RECHARGE                                        (1 << 6)
#define RECHARGE_DB                                     (1 << 7)
#define CV_MODE                                         (1 << 8)
#define CV_MODE_DB                                      (1 << 9)

// REG_7C
#define ACIN_OK                                         (1 << 2)
#define ACIN_OK_DB                                      (1 << 3)
#define ACIN_OV                                         (1 << 6)
#define ACIN_OV_DB                                      (1 << 7)
#define VBAT_LOW                                        (1 << 8)
#define VBAT_LOW_DB                                     (1 << 9)
#define OTP                                             (1 << 10)
#define OTP_DB                                          (1 << 11)

// REG_7D
#define ACINGTVBAT                                      (1 << 0)
#define POWER_UP_STATE_SHIFT                            5
#define POWER_UP_STATE_MASK                             (0x7 << POWER_UP_STATE_SHIFT)
#define POWER_UP_STATE(n)                               BITFIELD_VAL(POWER_UP_STATE, n)
#define MAIN_STATE_SHIFT                                8
#define MAIN_STATE_MASK                                 (0x3 << MAIN_STATE_SHIFT)
#define MAIN_STATE(n)                                   BITFIELD_VAL(MAIN_STATE, n)
#define CHARGE_STATE_SHIFT                              10
#define CHARGE_STATE_MASK                               (0x7 << CHARGE_STATE_SHIFT)
#define CHARGE_STATE(n)                                 BITFIELD_VAL(CHARGE_STATE, n)

// REG_7E
#define REG_CLK_LPO                                     (1 << 0)
#define REG_CLK_32K_PMU                                 (1 << 1)
#define REG_CLK_LPO_BB                                  (1 << 2)
#define REG_CLK_LPO_STABLE_GATE                         (1 << 3)

// REG_80
#define CHRG_INTR                                       (1 << 0)
#define AC_ON_DET_IN_INTR                               (1 << 1)
#define AC_ON_DET_OUT_INTR                              (1 << 2)
#define CHARGE_TRIPRE_TIMEOUT_INTR                      (1 << 5)
#define CHARGE_FAST_TIMEOUT_INTR                        (1 << 6)
#define ACIN_OV_INTR                                    (1 << 12)
#define OTP_INTR                                        (1 << 14)

#define IRQ1_STATUS_SHIFT                               0
#define IRQ1_STATUS_MASK                                (0xFFFF << IRQ1_STATUS_SHIFT)
#define IRQ1_STATUS(n)                                  BITFIELD_VAL(IRQ1_STATUS, n)
#define IRQ1_NUM                                        16

// REG_81
#define MAIN_STANDBY_INTR                               (1 << 0)
#define MAIN_UART_INTR                                  (1 << 1)
#define MAIN_SHIP_INTR                                  (1 << 2)
#define MAIN_CHARGE_INTR                                (1 << 3)
#define PU_INIT_DONE_INTR                               (1 << 4)
#define PU_CORE_INTR                                    (1 << 5)
#define PU_EN_DET_INTR                                  (1 << 6)
#define PU_VDET_INTR                                    (1 << 7)
#define PU_PP_PRE_INTR                                  (1 << 8)
#define PU_PP_INTR                                      (1 << 9)
#define CHRG_TRICKLE_INTR                               (1 << 10)
#define CHRG_PRE_INTR                                   (1 << 11)
#define CHRG_FAST_INTR                                  (1 << 12)
#define CHRG_OFF_INTR                                   (1 << 13)
#define CHRG_DONE_INTR                                  (1 << 14)
#define CHRG_FAULT_INTR                                 (1 << 15)

#define IRQ2_STATUS_SHIFT                               0
#define IRQ2_STATUS_MASK                                (0xFFFF << IRQ2_STATUS_SHIFT)
#define IRQ2_STATUS(n)                                  BITFIELD_VAL(IRQ2_STATUS, n)

// REG_82
#define MAIN_STATE_INTR_EN_SHIFT                        0
#define MAIN_STATE_INTR_EN_MASK                         (0xF << MAIN_STATE_INTR_EN_SHIFT)
#define MAIN_STATE_INTR_EN(n)                           BITFIELD_VAL(MAIN_STATE_INTR_EN, n)
#define PU_STATE_INTR_EN_SHIFT                          4
#define PU_STATE_INTR_EN_MASK                           (0x3F << PU_STATE_INTR_EN_SHIFT)
#define PU_STATE_INTR_EN(n)                             BITFIELD_VAL(PU_STATE_INTR_EN, n)
#define CHRG_STATE_INTR_EN_SHIFT                        10
#define CHRG_STATE_INTR_EN_MASK                         (0x3F << CHRG_STATE_INTR_EN_SHIFT)
#define CHRG_STATE_INTR_EN(n)                           BITFIELD_VAL(CHRG_STATE_INTR_EN, n)

#define REG_CHARGE_CHRG_TRICKLE_INTR_EN                 (1 << 10)
#define REG_CHARGE_CHRG_PRE_INTR_EN                     (1 << 11)
#define REG_CHARGE_CHRG_FAST_INTR_EN                    (1 << 12)
#define REG_CHARGE_CHRG_OFF_INTR_EN                     (1 << 13)
#define REG_CHARGE_CHRG_DONE_INTR_EN                    (1 << 14)

// REG_83
#define MAIN_STATE_INTR_MASK_SHIFT                      0
#define MAIN_STATE_INTR_MASK_MASK                       (0xF << MAIN_STATE_INTR_MASK_SHIFT)
#define MAIN_STATE_INTR_MASK(n)                         BITFIELD_VAL(MAIN_STATE_INTR_MASK, n)
#define PU_STATE_INTR_MASK_SHIFT                        4
#define PU_STATE_INTR_MASK_MASK                         (0x3F << PU_STATE_INTR_MASK_SHIFT)
#define PU_STATE_INTR_MASK(n)                           BITFIELD_VAL(PU_STATE_INTR_MASK, n)
#define CHRG_STATE_INTR_MASK_SHIFT                      10
#define CHRG_STATE_INTR_MASK_MASK                       (0x3F << CHRG_STATE_INTR_MASK_SHIFT)
#define CHRG_STATE_INTR_MASK(n)                         BITFIELD_VAL(CHRG_STATE_INTR_MASK, n)

#define REG_CHARGE_CHRG_TRICKLE_INTR_MASK               (1 << 10)
#define REG_CHARGE_CHRG_PRE_INTR_MASK                   (1 << 11)
#define REG_CHARGE_CHRG_FAST_INTR_MASK                  (1 << 12)
#define REG_CHARGE_CHRG_OFF_INTR_MASK                   (1 << 13)
#define REG_CHARGE_CHRG_DONE_INTR_MASK                  (1 << 14)

// REG_84
#define MAIN_STATE_INTR_RAW_SHIFT                       0
#define MAIN_STATE_INTR_RAW_MASK                        (0xF << MAIN_STATE_INTR_RAW_SHIFT)
#define MAIN_STATE_INTR_RAW(n)                          BITFIELD_VAL(MAIN_STATE_INTR_RAW, n)
#define PU_STATE_INTR_RAW_SHIFT                         4
#define PU_STATE_INTR_RAW_MASK                          (0x3F << PU_STATE_INTR_RAW_SHIFT)
#define PU_STATE_INTR_RAW(n)                            BITFIELD_VAL(PU_STATE_INTR_RAW, n)
#define CHRG_STATE_INTR_RAW_SHIFT                       10
#define CHRG_STATE_INTR_RAW_MASK                        (0x3F << CHRG_STATE_INTR_RAW_SHIFT)
#define CHRG_STATE_INTR_RAW(n)                          BITFIELD_VAL(CHRG_STATE_INTR_RAW, n)

#define REG_CHARGE_CHRG_TRICKLE_INTR_RAW                (1 << 10)
#define REG_CHARGE_CHRG_PRE_INTR_RAW                    (1 << 11)
#define REG_CHARGE_CHRG_FAST_INTR_RAW                   (1 << 12)
#define REG_CHARGE_CHRG_OFF_INTR_RAW                    (1 << 13)
#define REG_CHARGE_CHRG_DONE_INTR_RAW                   (1 << 14)

// REG_85
#define MAIN_STATE_INTR_CLR_SHIFT                       0
#define MAIN_STATE_INTR_CLR_MASK                        (0xF << MAIN_STATE_INTR_CLR_SHIFT)
#define MAIN_STATE_INTR_CLR(n)                          BITFIELD_VAL(MAIN_STATE_INTR_CLR, n)
#define PU_STATE_INTR_CLR_SHIFT                         4
#define PU_STATE_INTR_CLR_MASK                          (0x3F << PU_STATE_INTR_CLR_SHIFT)
#define PU_STATE_INTR_CLR(n)                            BITFIELD_VAL(PU_STATE_INTR_CLR, n)
#define CHRG_STATE_INTR_CLR_SHIFT                       10
#define CHRG_STATE_INTR_CLR_MASK                        (0x3F << CHRG_STATE_INTR_CLR_SHIFT)
#define CHRG_STATE_INTR_CLR(n)                          BITFIELD_VAL(CHRG_STATE_INTR_CLR, n)

#define REG_CHARGE_CHRG_TRICKLE_INTR_CLR                (1 << 10)
#define REG_CHARGE_CHRG_PRE_INTR_CLR                    (1 << 11)
#define REG_CHARGE_CHRG_FAST_INTR_CLR                   (1 << 12)
#define REG_CHARGE_CHRG_OFF_INTR_CLR                    (1 << 13)
#define REG_CHARGE_CHRG_DONE_INTR_CLR                   (1 << 14)

#define REG_CHARGE_INTR_RAW                             0
#define REG_CHARGE_INTR_EN                              0
#define REG_CHARGE_INTR_MASK                            0
#define REG_CHARGE_INTR_CLR                             0

#endif
