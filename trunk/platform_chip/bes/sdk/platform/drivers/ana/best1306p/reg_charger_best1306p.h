/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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
#ifndef __REG_CHARGER_BEST1306P_H__
#define __REG_CHARGER_BEST1306P_H__

#include "plat_types.h"

enum CHG_REG_T {
    CHG_REG_01 = 0x1,
    CHG_REG_02 = 0x2,
    CHG_REG_03 = 0x3,
    CHG_REG_04 = 0x4,
    CHG_REG_05 = 0x5,
    CHG_REG_06 = 0x6,
    CHG_REG_07 = 0x7,
    CHG_REG_08 = 0x8,
    CHG_REG_09 = 0x9,
    CHG_REG_0A = 0xA,
    CHG_REG_0B = 0xB,
    CHG_REG_0C = 0xC,
    CHG_REG_0E = 0xE,
    CHG_REG_21 = 0x21,
    CHG_REG_23 = 0x23,
    CHG_REG_24 = 0x24,
    CHG_REG_26 = 0x26,
    CHG_REG_28 = 0x28,
    CHG_REG_2A = 0x2A,
    CHG_REG_2B = 0x2B,
    CHG_REG_30 = 0x30,
    CHG_REG_31 = 0x31,
    CHG_REG_33 = 0x33,
    CHG_REG_34 = 0x34,
    CHG_REG_40 = 0x40,
    CHG_REG_41 = 0x41,
    CHG_REG_42 = 0x42,
};

// REG_01
#define REG_SYS_DET_EN                                  (1 << 2)
#define REG_SYS_DET_EN_DR                               (1 << 3)
#define REG_CHARGER_RECHARGE_EN                         (1 << 12)
#define REG_CHARGER_RECHARGE_EN_DR                      (1 << 13)

// REG_02
#define REG_BG_CORE_EN                                  (1 << 0)
#define REG_BG_CORE_EN_DR                               (1 << 1)
#define CHARGER_SS_PUP_EN                               (1 << 11)

// REG_03
#define REG_CHARGER_CHARGE_EN                           (1 << 0)
#define REG_CHARGER_CHARGE_EN_DR                        (1 << 1)
#define REG_CHARGER_VOREG_BIT_SHIFT                     10
#define REG_CHARGER_VOREG_BIT_MASK                      (0x3F << REG_CHARGER_VOREG_BIT_SHIFT)
#define REG_CHARGER_VOREG_BIT(n)                        BITFIELD_VAL(REG_CHARGER_VOREG_BIT, n)

// REG_04
#define REG_CHARGER_IPRE_SEL_SHIFT                      0
#define REG_CHARGER_IPRE_SEL_MASK                       (0x7F << REG_CHARGER_IPRE_SEL_SHIFT)
#define REG_CHARGER_IPRE_SEL(n)                         BITFIELD_VAL(REG_CHARGER_IPRE_SEL, n)
#define REG_CHARGER_ICC_SEL_SHIFT                       7
#define REG_CHARGER_ICC_SEL_MASK                        (0x7F << REG_CHARGER_ICC_SEL_SHIFT)
#define REG_CHARGER_ICC_SEL(n)                          BITFIELD_VAL(REG_CHARGER_ICC_SEL, n)
#define REG_CHARGER_ICC_SEL_DR                          (1 << 14)

// REG_05
#define REG_CHARGER_BAT_OVP_DET_EN                      (1 << 6)
#define REG_CHARGER_BAT_OVP_DET_EN_DR                   (1 << 7)

// REG_06
#define REG_PP_CL_EN                                    (1 << 0)
#define REG_PP_CL_EN_DR                                 (1 << 1)
#define REG_PP_CS_EN                                    (1 << 2)
#define REG_PP_CS_EN_DR                                 (1 << 3)
#define REG_PP_ID_MODE_EN                               (1 << 4)
#define REG_PP_SS_PUP_EN                                (1 << 6)
#define REG_PP_SS_PUP_EN_DR                             (1 << 7)
#define REG_PP_VDIV_EN                                  (1 << 8)
#define REG_PP_VDIV_EN_DR                               (1 << 9)
#define REG_PP_VIN_DPM_EN                               (1 << 10)
#define REG_PP_VIN_DPM_EN_DR                            (1 << 11)
#define REG_PP_VIN_FR_EN                                (1 << 12)
#define REG_PP_CV_MODE_EN                               (1 << 14)
#define REG_PP_CV_MODE_EN_DR                            (1 << 15)

// REG_07
#define PP_VSYS_MIN_VBIT_SHIFT                          7
#define PP_VSYS_MIN_VBIT_MASK                           (0xF << PP_VSYS_MIN_VBIT_SHIFT)
#define PP_VSYS_MIN_VBIT(n)                             BITFIELD_VAL(PP_VSYS_MIN_VBIT, n)
#define PP_VSYS_CV_VBIT_SHIFT                           11
#define PP_VSYS_CV_VBIT_MASK                            (0x7 << PP_VSYS_CV_VBIT_SHIFT)
#define PP_VSYS_CV_VBIT(n)                              BITFIELD_VAL(PP_VSYS_CV_VBIT, n)

// REG_08
#define PP_VIN_DPM_VBIT_SHIFT                           0
#define PP_VIN_DPM_VBIT_MASK                            (0xF << PP_VIN_DPM_VBIT_SHIFT)
#define PP_VIN_DPM_VBIT(n)                              BITFIELD_VAL(PP_VIN_DPM_VBIT, n)
#define PP_ILIMIT_SEL_SHIFT                             10
#define PP_ILIMIT_SEL_MASK                              (0x1F << PP_ILIMIT_SEL_SHIFT)
#define PP_ILIMIT_SEL(n)                                BITFIELD_VAL(PP_ILIMIT_SEL, n)

// REG_09
#define PP_FR_DELTA_VBIT_SHIFT                          9
#define PP_FR_DELTA_VBIT_MASK                           (0x7 << PP_FR_DELTA_VBIT_SHIFT)
#define PP_FR_DELTA_VBIT(n)                             BITFIELD_VAL(PP_FR_DELTA_VBIT, n)

// REG_0A
#define CHARGER_TRIM_VOREG_SHIFT                        6
#define CHARGER_TRIM_VOREG_MASK                         (0x1F << CHARGER_TRIM_VOREG_SHIFT)
#define CHARGER_TRIM_VOREG(n)                           BITFIELD_VAL(CHARGER_TRIM_VOREG, n)
#define CHARGER_TRIM_PRE_SHIFT                          11
#define CHARGER_TRIM_PRE_MASK                           (0x1F << CHARGER_TRIM_PRE_SHIFT)
#define CHARGER_TRIM_PRE(n)                             BITFIELD_VAL(CHARGER_TRIM_PRE, n)

// REG_0B
#define REG_CHARGER_TRIM_ITERM_SHIFT                    0
#define REG_CHARGER_TRIM_ITERM_MASK                     (0x1F << REG_CHARGER_TRIM_ITERM_SHIFT)
#define REG_CHARGER_TRIM_ITERM(n)                       BITFIELD_VAL(REG_CHARGER_TRIM_ITERM, n)
#define REG_CHARGER_TRIM_CC_SHIFT                       5
#define REG_CHARGER_TRIM_CC_MASK                        (0x1F << REG_CHARGER_TRIM_CC_SHIFT)
#define REG_CHARGER_TRIM_CC(n)                          BITFIELD_VAL(REG_CHARGER_TRIM_CC, n)
#define REG_CHARGER_PRE2CC_VBIT_SHIFT                   10
#define REG_CHARGER_PRE2CC_VBIT_MASK                    (0x3 << REG_CHARGER_PRE2CC_VBIT_SHIFT)
#define REG_CHARGER_PRE2CC_VBIT(n)                      BITFIELD_VAL(REG_CHARGER_PRE2CC_VBIT, n)

// REG_0C
#define CHARGER_ITERM_SEL_SHIFT                         0
#define CHARGER_ITERM_SEL_MASK                          (0x7F << CHARGER_ITERM_SEL_SHIFT)
#define CHARGER_ITERM_SEL(n)                            BITFIELD_VAL(CHARGER_ITERM_SEL, n)

// REG_0E
#define BG_TRIM_SHIFT                                   0
#define BG_TRIM_MASK                                    (0x3F << BG_TRIM_SHIFT)
#define BG_TRIM(n)                                      BITFIELD_VAL(BG_TRIM, n)
#define BG_TEMP_TRIM_SHIFT                              6
#define BG_TEMP_TRIM_MASK                               (0xF << BG_TEMP_TRIM_SHIFT)
#define BG_TEMP_TRIM(n)                                 BITFIELD_VAL(BG_TEMP_TRIM, n)

// REG_21
#define FET_RESET                                       (1 << 1)
#define REG_SW_CHARGE_EN                                (1 << 2)
#define REG_LPO_ON                                      (1 << 4)

// REG_23
#define REG_PU_LPO                                      (1 << 5)
#define REG_PU_LPO_DR                                   (1 << 6)

// REG_24
#define REG_STANDBY_SHIP_MODE                           (1 << 0)
#define REG_SHIP_MODE_EN                                (1 << 7)

// REG_26
#define REG_VIN_OV_INTR_EN                              (1 << 5)
#define REG_VIN_OV_INTR_CLR                             (1 << 7)
#define REG_VIN_OC_INTR_EN                              (1 << 9)
#define REG_VIN_OC_INTR_CLR                             (1 << 11)

// REG_28
#define REG_CHARGE_DONE_INTR_EN                         (1 << 13)
#define REG_CHARGE_DONE_INTR_CLR                        (1 << 15)

// REG_2B
#define CHIP_ADDR_I2C_SHIFT                             0
#define CHIP_ADDR_I2C_MASK                              (0x7F << CHIP_ADDR_I2C_SHIFT)
#define CHIP_ADDR_I2C(n)                                BITFIELD_VAL(CHIP_ADDR_I2C, n)

// REG_30
#define RESERVED_ANA_8_0_SHIFT                          0
#define RESERVED_ANA_8_0_MASK                           (0x1FF << RESERVED_ANA_8_0_SHIFT)
#define RESERVED_ANA_8_0(n)                             BITFIELD_VAL(RESERVED_ANA_8_0, n)

// Metal id > 0
#define RESERVED_ANA_8_0_BIT_0                          (1 << 0)
#define RESERVED_ANA_8_0_BIT_3_1_SHIFT                  1
#define RESERVED_ANA_8_0_BIT_3_1_MASK                   (0x7 << RESERVED_ANA_8_0_BIT_3_1_SHIFT)
#define RESERVED_ANA_8_0_BIT_3_1(n)                     BITFIELD_VAL(RESERVED_ANA_8_0_BIT_3_1, n)

// REG_31
#define RESERVED_ANA_17_9_SHIFT                         0
#define RESERVED_ANA_17_9_MASK                          (0x1FF << RESERVED_ANA_17_9_SHIFT)
#define RESERVED_ANA_17_9(n)                            BITFIELD_VAL(RESERVED_ANA_17_9, n)

// Metal id > 0
#define RESERVED_ANA_17_9_BIT_9                         (1 << 0)
#define RESERVED_ANA_17_9_BIT_11                        (1 << 2)

// REG_33
#define CHARGE_STATE_SHIFT                              12
#define CHARGE_STATE_MASK                               (0x7 << CHARGE_STATE_SHIFT)
#define CHARGE_STATE(n)                                 BITFIELD_VAL(CHARGE_STATE, n)

// REG_34
#define CHARGE_DONE_INTR                                (1 << 5)
#define VIN_OV_INTR                                     (1 << 7)
#define VIN_OC_INTR                                     (1 << 8)

// REG_40
#define REG_EFUSE_PGM_EN                                (1 << 0)
#define REG_EFUSE_READ_EN                               (1 << 1)
#define EFUSE_STATE_SHIFT                               3
#define EFUSE_STATE_MASK                                (0x7 << EFUSE_STATE_SHIFT)
#define EFUSE_STATE(n)                                  BITFIELD_VAL(EFUSE_STATE, n)
#define REG_EFUSE_PGM_CNT_SHIFT                         6
#define REG_EFUSE_PGM_CNT_MASK                          (0x1F << REG_EFUSE_PGM_CNT_SHIFT)
#define REG_EFUSE_PGM_CNT(n)                            BITFIELD_VAL(REG_EFUSE_PGM_CNT, n)
#define REG_EFUSE_CLOCK_EN                              (1 << 11)

// cl_vos_bit[3:0]
#define CL_VOS_BIT_3                                    RESERVED_ANA_17_9_BIT_11
#define CL_VOS_BIT_2_0_SHIFT                            RESERVED_ANA_8_0_BIT_3_1_SHIFT
#define CL_VOS_BIT_2_0_MASK                             RESERVED_ANA_8_0_BIT_3_1_MASK
#define CL_VOS_BIT_2_0(n)                               RESERVED_ANA_8_0_BIT_3_1(n)

// bat_cl_ibit[2:0]
#define BAT_CL_IBIT_2_0_BIT_2                           RESERVED_ANA_17_9_BIT_9
#define BAT_CL_IBIT_2_0_BIT_1                           RESERVED_ANA_8_0_BIT_0
#define BAT_CL_IBIT_2_0_BIT_0                           CHARGER_SS_PUP_EN

#endif
