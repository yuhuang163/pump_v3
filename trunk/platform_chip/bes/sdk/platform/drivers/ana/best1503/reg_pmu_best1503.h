/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

enum PMU_REG_T {
    PMU_REG_METAL_ID                = 0x00,
    PMU_REG_PWR_OFF_CNT_CFG         = 0x01,
    PMU_REG_POWER_KEY_CFG           = 0x02,
    PMU_REG_BIAS_CFG                = 0x03,
    PMU_REG_CHARGER_CFG             = 0x05,
    PMU_REG_ANA_CFG                 = 0x07,
    PMU_REG_DIG_CFG                 = 0x08,
    PMU_REG_IO_CFG                  = 0x09,
    PMU_REG_USB_CFG                 = 0x0A,
    PMU_REG_CRYSTAL_CFG             = 0x0C,
    PMU_REG_AVDD_EN                 = 0x0D,
    PMU_REG_CODEC_CFG               = 0x0E,
    PMU_REG_DCDC_ANA_VOLT           = 0x13,
    PMU_REG_DCDC_EN                 = 0x14,
    PMU_REG_BUCK_ANA_FREQ_CFG       = 0x15,
    PMU_REG_SLEEP_CFG               = 0x1D,
    PMU_REG_INT_MASK                = 0x1F,
    PMU_REG_INT_EN                  = 0x20,
    PMU_REG_RTC_DIV_1HZ             = 0x22,
    PMU_REG_RTC_LOAD_LOW            = 0x23,
    PMU_REG_RTC_LOAD_HIGH           = 0x24,
    PMU_REG_RTC_MATCH1_LOW          = 0x27,
    PMU_REG_RTC_MATCH1_HIGH         = 0x28,
    PMU_REG_MIC_BIAS_A              = 0x29,
    PMU_REG_MIC_BIAS_B              = 0x2A,
    PMU_REG_LED_CFG_IO1             = 0x2B,
    PMU_REG_PWM2_TOGGLE             = 0x2D,
    PMU_REG_PWM2_ST1                = 0x2E,
    PMU_REG_PWM2_EN                 = 0x2F,
    PMU_REG_PWM2_BR_EN              = 0x30,
    PMU_REG_IPTAT_CORE_I_CFG        = 0x34,
    PMU_REG_DIV_HW_RESET_CFG        = 0x35,
    PMU_REG_WDT_RESET_TIMER         = 0x36,
    PMU_REG_WDT_CFG                 = 0x37,
    PMU_REG_RESERVED_ANA            = 0x38,
    PMU_REG_PWMB_TOGGLE             = 0x3A,
    PMU_REG_PWMB_ST1                = 0x3B,
    PMU_REG_PWMB_EN                 = 0x3C,
    PMU_REG_PWMB_BR_EN              = 0x3D,
    PMU_REG_LED_CFG_IO2             = 0x3E,
    PMU_REG_USB_CFG_3F              = 0x3F,
    PMU_REG_USB_CFG2                = 0x40,
    PMU_REG_USB_CFG3                = 0x41,
    PMU_REG_WDT_INT_CFG             = 0x43,
    PMU_REG_CLK_SEL_CFG             = 0x44,
    PMU_REG_MIC_BIAS_C              = 0x45,
    PMU_REG_UART1_CFG               = 0x46,
    PMU_REG_UART2_CFG               = 0x47,
    PMU_REG_LED_IO_IN               = 0x48,
    PMU_REG_DCDC_DIG_VOLT           = 0x4A,
    PMU_REG_WDT_IRQ_TIMER           = 0x4D,
    PMU_REG_POWER_OFF               = 0x4F,
    PMU_REG_INT_STATUS              = 0x50,
    PMU_REG_INT_MSKED_STATUS        = 0x51,
    PMU_REG_INT_CLR                 = 0x51,
    PMU_REG_RAW_STATUS              = 0x52,
    PMU_REG_RTC_VAL_LOW             = 0x54,
    PMU_REG_RTC_VAL_HIGH            = 0x55,
    PMU_REG_CHARGER_STATUS          = 0x5E,
    PMU_REG_PWR_CLK_STATUS          = 0x5F,
    PMU_REG_NOT_RESET_61            = 0x61,
    PMU_REG_BUCK_CORE_CFG_63        = 0x63,
    PMU_REG_DCDC_DIG_SYNC           = 0x64,
    PMU_REG_MIC_PULL_DOWN           = 0x67,
    PMU_REG_MEM_CFG                 = 0x68,
    PMU_REG_PA_CFG                  = 0x69,
    PMU_REG_DCDC_RAMP_EN            = 0x6A,
    PMU_REG_SAR_EN                  = 0x8F,
    PMU_REG_SAR_CLK_CFG             = 0x90,
    PMU_REG_SAR_PU_CFG              = 0x93,
    PMU_REG_EFUSE_CTRL              = 0xB7,
    PMU_REG_EFUSE_DATA_HIGH         = 0xBD,
    PMU_REG_EFUSE_DATA_LOW          = 0xBE,
    PMU_REG_BUCK_HPPA_FREQ_CFG      = 0xBF,
    PMU_REG_BUCK_HPPA_CFG_C0        = 0xC0,
    PMU_REG_DCDC_HPPA_CAP           = 0xC1,
    PMU_REG_DCDC_HPPA_CFG_C2        = 0xC2,
    PMU_REG_DCDC_HPPA_VOLT          = 0xC3,
    PMU_REG_BUCK_ANA_CFG_C4         = 0xC4,
    PMU_REG_BUCK_ANA_CFG_C5         = 0xC5,
    PMU_REG_BUCK_CORE_CFG_C7        = 0xC7,
    PMU_REG_BUCK_CORE_CFG_C8        = 0xC8,
    PMU_REG_DCDC_HPPA_EN            = 0xCA,
    PMU_REG_PU_MEM_DR               = 0xCC,
    PMU_REG_PU_MEM_CFG              = 0xCD,
    PMU_REG_MAIN_LDO_VOLT_RC        = 0xCF,
    PMU_REG_RC_SLEEP_CFG            = 0xD0,
    PMU_REG_USB_PA_VOLT_RC          = 0xD1,
    PMU_REG_LPO_VRTC_CFG_D2         = 0xD2,
    PMU_REG_HPPA_VOLT_RC            = 0xD3,
    PMU_REG_CORE_L_VBIT             = 0xD5,
    PMU_REG_CORE_L_CFG              = 0xD6,
    PMU_REG_CORE_L_CFG2             = 0xD7,
    PMU_REG_XO_32K_CFG              = 0xD8,
    PMU_REG_XO_32K_ISEL_RAMP        = 0xD9,
    PMU_REG_PU_VSYS_DIV             = 0xDA,
    PMU_REG_GP_CFG                  = 0xDB,
    PMU_REG_SENSOR_GP_VOLT_RC       = 0xDC,
    PMU_REG_SENSOR_CFG              = 0xDD,
    PMU_REG_GP_SENS_VOLT_SLEEP      = 0xDE,
    PMU_REG_SAR_VREF_CFG_E6         = 0xE6,
    PMU_REG_BUCK_VANA_CFG_E7        = 0xE7,
    PMU_REG_BUCK_VHPPA_CFG_E8       = 0xE8,
    PMU_REG_SW_1P8_PSRAM            = 0xEA,
    PMU_REG_RCOSC2M_GOAL_CNT_MARK   = 0xED,
    PMU_REG_RCOSC2M_CALIB           = 0xEE,
    PMU_REG_DBG_RCOSC2M             = 0xF2,
    PMU_REG_RCOSC_2M_CFG            = 0xF4,
    PMU_REG_FAST_HW_RAMP_CFG        = 0xF5,
    PMU_REG_BOOT_SOURCE             = 0xF6,
    PMU_REG_RCOSC_6M_GOAL_CNT       = 0xF7,
    PMU_REG_RCOSC_6M_CFG            = 0xF8,
    PMU_REG_RCOSC_192M_CALIB        = 0xFA,
    PMU_REG_DBG_RCOSC_6M_CFG        = 0xFC,
    PMU_REG_DBG_RCOSC6M_REF_CNT     = 0xFD,
    PMU_REG_RCOSC_192M_CFG          = 0xFE,
    PMU_REG_RTC_MATCH_MIN_SEC       = 0x10A,
    PMU_REG_RTC_MATCH_HOURS         = 0x10B,
    PMU_REG_RTC_MATCH_DATAS         = 0x10C,
    PMU_REG_RTC_MATCH_YEARS         = 0x10D,
    PMU_REG_RTC_RTC_CFG_10E         = 0x10E,
    PMU_REG_RTC_LOAD_MIN_SEC_EN     = 0x10F,
    PMU_REG_RTC_LOAD_HOURS          = 0x110,
    PMU_REG_RTC_LOAD_DATAS_EN       = 0x111,
    PMU_REG_RTC_LOAD_YEARS          = 0x112,
    PMU_REG_RTC_GET_MIN_SEC         = 0x113,
    PMU_REG_RTC_GET_HOURS           = 0x114,
    PMU_REG_RTC_GET_DATAS           = 0x115,
    PMU_REG_RTC_GET_YEARS           = 0x116,
    PMU_REG_LDO_ANA_CFG             = 0x117,
    PMU_REG_LDO_ANA_VOLT            = 0x118,
    PMU_REG_LDO_CORE_CFG            = 0x119,
    PMU_REG_LDO_CORE_VOLT           = 0x11A,
    PMU_REG_LDO_CORE_L_CFG          = 0x11B,
    PMU_REG_LDO_CORE_L_VOLT         = 0x11C,
    PMU_REG_LDO_CORE_L_LP_RC_VOLT   = 0x11D,
    PMU_REG_LDO_CODEC_CFG           = 0x11E,
    PMU_REG_LDO_CODEC_VOLT          = 0x11F,
    PMU_REG_LDO_USB_CFG             = 0x120,
    PMU_REG_LDO_USB_VOLT            = 0x121,
    PMU_REG_LDO_USB_LP_RC_VOLT      = 0x122,
    PMU_REG_LDO_SENSOR_CFG          = 0x123,
    PMU_REG_LDO_SENSOR_VOLT         = 0x124,
    PMU_REG_LDO_SENSOR_LP_RC_VOLT   = 0x125,
    PMU_REG_LDO_MEM_CFG             = 0x126,
    PMU_REG_LDO_MEM_VOLT            = 0x127,
    PMU_REG_LDO_MEM_LP_RC_VOLT      = 0x128,
    PMU_REG_LDO_VRTC_CFG            = 0x129,
    PMU_REG_LDO_VRTC_VOLT           = 0x12A,
    PMU_REG_LDO_LP_BIAS_SEL_VBIT    = 0x12C,
    PMU_REG_BUCK_VANA_EN            = 0x12D,
    PMU_REG_BUCK_VANA_CFG           = 0x12E,
    PMU_REG_BUCK_VANA_LP_CFG        = 0x12F,
    PMU_REG_BUCK_VANA_RC_CFG        = 0x130,
    PMU_REG_BUCK_VCORE_EN           = 0x131,
    PMU_REG_BUCK_VCORE_CFG          = 0x132,
    PMU_REG_BUCK_VCORE_LP_CFG       = 0x133,
    PMU_REG_BUCK_VCORE_RC_CFG       = 0x134,
    PMU_REG_BUCK_VHPPA_EN           = 0x135,
    PMU_REG_BUCK_VHPPA_CFG          = 0x136,
    PMU_REG_BUCK_VHPPA_LP_CFG       = 0x137,
    PMU_REG_BUCK_VHPPA_RC_CFG       = 0x138,
    PMU_REG_BUCK_VANA_RC_CFG_140    = 0x140,
    PMU_REG_BUCK_VCORE_IS_GAIN_ULP  = 0x141,
    PMU_REG_BUCK_VCORE_RC_CFG_143   = 0x143,
    PMU_REG_BUCK_VHPPA_RC_CFG_146   = 0x146,
    PMU_REG_EFUSE_CLOCK_CTRL        = 0x147,
};

// REG_00
#define REVID_SHIFT                         0
#define REVID_MASK                          (0xF << REVID_SHIFT)
#define REVID(n)                            BITFIELD_VAL(REVID, n)
#define CHIPID_SHIFT                        4
#define CHIPID_MASK                         (0xFFF << CHIPID_SHIFT)
#define CHIPID(n)                           BITFIELD_VAL(CHIPID, n)

// REG_01
#define LPO_OFF_CNT_SHIFT                   12
#define LPO_OFF_CNT_MASK                    (0xF << LPO_OFF_CNT_SHIFT)
#define LPO_OFF_CNT(n)                      BITFIELD_VAL(LPO_OFF_CNT, n)
#define POWER_ON_DB_CNT_SHIFT               8
#define POWER_ON_DB_CNT_MASK                (0xF << POWER_ON_DB_CNT_SHIFT)
#define POWER_ON_DB_CNT(n)                  BITFIELD_VAL(POWER_ON_DB_CNT, n)
#define POWER_OFF_CNT_SHIFT                 4
#define POWER_OFF_CNT_MASK                  (0xF << POWER_OFF_CNT_SHIFT)
#define POWER_OFF_CNT(n)                    BITFIELD_VAL(POWER_OFF_CNT, n)
#define CLK_STABLE_CNT_SHIFT                0
#define CLK_STABLE_CNT_MASK                 (0xF << CLK_STABLE_CNT_SHIFT)
#define CLK_STABLE_CNT(n)                   BITFIELD_VAL(CLK_STABLE_CNT, n)

// REG_02
#define REG_PU_VBAT_DIV                     (1 << 15)
#define PU_LPO_DR                           (1 << 14)
#define PU_LPO_REG                          (1 << 13)
#define POWERKEY_WAKEUP_OSC_EN              (1 << 12)
#define RTC_POWER_ON_EN                     (1 << 11)
#define PU_ALL_REG                          (1 << 10)
#define RESERVED_ANA_16                     (1 << 9)

#define DEEPSLEEP_MODE_DIGI_DR              (1 << 7)
#define DEEPSLEEP_MODE_DIGI_REG             (1 << 6)
#define RESERVED_ANA_18_17_SHIFT            4
#define RESERVED_ANA_18_17_MASK             (0x3 << RESERVED_ANA_18_17_SHIFT)
#define RESERVED_ANA_18_17(n)               BITFIELD_VAL(RESERVED_ANA_18_17, n)
#define RESETN_ANA_DR                       (1 << 3)
#define RESETN_ANA_REG                      (1 << 2)
#define RESETN_DIG_DR                       (1 << 1)
#define RESETN_DIG_REG                      (1 << 0)

// REG_03
#define RESERVED_ANA_19                     (1 << 15)
#define PU_LP_BIAS_LDO_DSLEEP               (1 << 14)
#define PU_LP_BIAS_LDO_DR                   (1 << 13)
#define PU_LP_BIAS_LDO_REG                  (1 << 12)
#define PU_BIAS_LDO_DR                      (1 << 11)
#define PU_BIAS_LDO_REG                     (1 << 10)
#define BG_VBG_SEL_DR                       (1 << 9)
#define BG_VBG_SEL_REG                      (1 << 8)
#define BG_CONSTANT_GM_BIAS_DR              (1 << 7)
#define BG_CONSTANT_GM_BIAS_REG             (1 << 6)
#define BG_CORE_EN_DR                       (1 << 5)
#define BG_CORE_EN_REG                      (1 << 4)
#define BG_VTOI_EN_DR                       (1 << 3)
#define BG_VTOI_EN_REG                      (1 << 2)
#define BG_NOTCH_EN_DR                      (1 << 1)
#define BG_NOTCH_EN_REG                     (1 << 0)

// REG_04
#define BG_NOTCH_LPF_LOW_BW                 (1 << 15)
#define BG_OPX2                             (1 << 14)
#define BG_PTATX2                           (1 << 13)
#define BG_VBG_RES_SHIFT                    9
#define BG_VBG_RES_MASK                     (0xF << BG_VBG_RES_SHIFT)
#define BG_VBG_RES(n)                       BITFIELD_VAL(BG_VBG_RES, n)
#define BG_CONSTANT_GM_BIAS_BIT_SHIFT       7
#define BG_CONSTANT_GM_BIAS_BIT_MASK        (0x3 << BG_CONSTANT_GM_BIAS_BIT_SHIFT)
#define BG_CONSTANT_GM_BIAS_BIT(n)          BITFIELD_VAL(BG_CONSTANT_GM_BIAS_BIT, n)
#define BG_VTOI_IABS_BIT_SHIFT              2
#define BG_VTOI_IABS_BIT_MASK               (0x1F << BG_VTOI_IABS_BIT_SHIFT)
#define BG_VTOI_IABS_BIT(n)                 BITFIELD_VAL(BG_VTOI_IABS_BIT, n)
#define BG_VTOI_VCAS_BIT_SHIFT              0
#define BG_VTOI_VCAS_BIT_MASK               (0x3 << BG_VTOI_VCAS_BIT_SHIFT)
#define BG_VTOI_VCAS_BIT(n)                 BITFIELD_VAL(BG_VTOI_VCAS_BIT, n)

// REG_05
#define REG_PU_LDO_VRTC_RF_DSLEEP           (1 << 15)
#define REG_PU_LDO_VRTC_RF_DR               (1 << 14)
#define REG_PU_LDO_VRTC_RF_REG              (1 << 13)
#define REG_CHARGE_OUT_INTR_MSK             (1 << 12)
#define REG_CHARGE_IN_INTR_MSK              (1 << 11)
#define REG_AC_ON_OUT_EN                    (1 << 10)
#define REG_AC_ON_IN_EN                     (1 << 9)
#define REG_CHARGE_INTR_EN                  (1 << 8)
#define REG_AC_ON_DB_VALUE_SHIFT            0
#define REG_AC_ON_DB_VALUE_MASK             (0xFF << REG_AC_ON_DB_VALUE_SHIFT)
#define REG_AC_ON_DB_VALUE(n)               BITFIELD_VAL(REG_AC_ON_DB_VALUE, n)

// REG_06
#define REG_BYPASS_VBG_RF_BUFFER_DR         (1 << 15)
#define REG_BYPASS_VBG_RF_BUFFER_REG        (1 << 14)
#define PU_VBG_RF_BUFFER_DR                 (1 << 13)
#define PU_VBG_RF_BUFFER_REG                (1 << 12)
#define RESERVED_ANA_21_20_SHIFT            10
#define RESERVED_ANA_21_20_MASK             (0x3 << RESERVED_ANA_21_20_SHIFT)
#define RESERVED_ANA_21_20(n)               BITFIELD_VAL(RESERVED_ANA_21_20, n)
#define REG_LPO_KTRIM_SHIFT                 4
#define REG_LPO_KTRIM_MASK                  (0x3F << REG_LPO_KTRIM_SHIFT)
#define REG_LPO_KTRIM(n)                    BITFIELD_VAL(REG_LPO_KTRIM, n)
#define REG_LPO_ITRIM_SHIFT                 0
#define REG_LPO_ITRIM_MASK                  (0xF << REG_LPO_ITRIM_SHIFT)
#define REG_LPO_ITRIM(n)                    BITFIELD_VAL(REG_LPO_ITRIM, n)

// REG_07
#define REG_LDO_VANA_LIGHT_LOAD             (1 << 15)

// REG_08
#define REG_LDO_VCORE_LOOP_CTL              (1 << 15)

// REG_09
#define POWER_UP_MOD12_CNT_SHIFT            8
#define POWER_UP_MOD12_CNT_MASK             (0xFF << POWER_UP_MOD12_CNT_SHIFT)
#define POWER_UP_MOD12_CNT(n)               BITFIELD_VAL(POWER_UP_MOD12_CNT, n)
#define POWER_UP_MOD11_CNT_SHIFT            0
#define POWER_UP_MOD11_CNT_MASK             (0xFF << POWER_UP_MOD11_CNT_SHIFT)
#define POWER_UP_MOD11_CNT(n)               BITFIELD_VAL(POWER_UP_MOD11_CNT, n)

// REG_0B
#define POWER_DOWN_DB_CNT_SHIFT             4
#define POWER_DOWN_DB_CNT_MASK              (0xF << POWER_DOWN_DB_CNT_SHIFT)
#define POWER_DOWN_DB_CNT(n)                BITFIELD_VAL(POWER_DOWN_DB_CNT, n)

// REG_0C
#define REG_VCORE_SSTIME_MODE_SHIFT         14
#define REG_VCORE_SSTIME_MODE_MASK          (0x3 << REG_VCORE_SSTIME_MODE_SHIFT)
#define REG_VCORE_SSTIME_MODE(n)            BITFIELD_VAL(REG_VCORE_SSTIME_MODE, n)
#define REG_HW_WDT_COM_RST_TIME_SHIFT       0
#define REG_HW_WDT_COM_RST_TIME_MASK        (0xFFF << REG_HW_WDT_COM_RST_TIME_SHIFT)
#define REG_HW_WDT_COM_RST_TIME(n)          BITFIELD_VAL(REG_HW_WDT_COM_RST_TIME, n)

// REG_0D
#define REG_PU_AVDD25_ANA                   (1 << 1)
#define REG_LDO_VCORE_BYPASS                (1 << 0)

// REG_0E
#define POWER_UP_MOD10_CNT_SHIFT            8
#define POWER_UP_MOD10_CNT_MASK             (0xFF << POWER_UP_MOD10_CNT_SHIFT)
#define POWER_UP_MOD10_CNT(n)               BITFIELD_VAL(POWER_UP_MOD10_CNT, n)
#define POWER_UP_MOD9_CNT_SHIFT             0
#define POWER_UP_MOD9_CNT_MASK              (0xFF << POWER_UP_MOD9_CNT_SHIFT)
#define POWER_UP_MOD9_CNT(n)                BITFIELD_VAL(POWER_UP_MOD9_CNT, n)

// REG_0F
#define REG_LOW_VIO                         (1 << 15)
#define REG_UVLO_SEL_SHIFT                  12
#define REG_UVLO_SEL_MASK                   (0x3 << REG_UVLO_SEL_SHIFT)
#define REG_UVLO_SEL(n)                     BITFIELD_VAL(REG_UVLO_SEL, n)
#define POWER_UP_SOFT_CNT_SHIFT             8
#define POWER_UP_SOFT_CNT_MASK              (0xF << POWER_UP_SOFT_CNT_SHIFT)
#define POWER_UP_SOFT_CNT(n)                BITFIELD_VAL(POWER_UP_SOFT_CNT, n)

// REG_10
#define POWER_UP_MOD2_CNT_SHIFT             8
#define POWER_UP_MOD2_CNT_MASK              (0xFF << POWER_UP_MOD2_CNT_SHIFT)
#define POWER_UP_MOD2_CNT(n)                BITFIELD_VAL(POWER_UP_MOD2_CNT, n)
#define POWER_UP_MOD1_CNT_SHIFT             0
#define POWER_UP_MOD1_CNT_MASK              (0xFF << POWER_UP_MOD1_CNT_SHIFT)
#define POWER_UP_MOD1_CNT(n)                BITFIELD_VAL(POWER_UP_MOD1_CNT, n)

// REG_11
#define POWER_UP_MOD4_CNT_SHIFT             8
#define POWER_UP_MOD4_CNT_MASK              (0xFF << POWER_UP_MOD4_CNT_SHIFT)
#define POWER_UP_MOD4_CNT(n)                BITFIELD_VAL(POWER_UP_MOD4_CNT, n)
#define POWER_UP_MOD3_CNT_SHIFT             0
#define POWER_UP_MOD3_CNT_MASK              (0xFF << POWER_UP_MOD3_CNT_SHIFT)
#define POWER_UP_MOD3_CNT(n)                BITFIELD_VAL(POWER_UP_MOD3_CNT, n)

// REG_12
#define POWER_UP_MOD6_CNT_SHIFT             8
#define POWER_UP_MOD6_CNT_MASK              (0xFF << POWER_UP_MOD6_CNT_SHIFT)
#define POWER_UP_MOD6_CNT(n)                BITFIELD_VAL(POWER_UP_MOD6_CNT, n)
#define POWER_UP_MOD5_CNT_SHIFT             0
#define POWER_UP_MOD5_CNT_MASK              (0xFF << POWER_UP_MOD5_CNT_SHIFT)
#define POWER_UP_MOD5_CNT(n)                BITFIELD_VAL(POWER_UP_MOD5_CNT, n)

// REG_13
#define POWER_UP_MOD8_CNT_SHIFT             8
#define POWER_UP_MOD8_CNT_MASK              (0xFF << POWER_UP_MOD8_CNT_SHIFT)
#define POWER_UP_MOD8_CNT(n)                BITFIELD_VAL(POWER_UP_MOD8_CNT, n)
#define POWER_UP_MOD7_CNT_SHIFT             0
#define POWER_UP_MOD7_CNT_MASK              (0xFF << POWER_UP_MOD7_CNT_SHIFT)
#define POWER_UP_MOD7_CNT(n)                BITFIELD_VAL(POWER_UP_MOD7_CNT, n)

// REG_14
#define DFT_MODE_CORE                       (1 << 15)
#define POWER_UP_MOD13_CNT_SHIFT            0
#define POWER_UP_MOD13_CNT_MASK             (0xFF << POWER_UP_MOD13_CNT_SHIFT)
#define POWER_UP_MOD13_CNT(n)               BITFIELD_VAL(POWER_UP_MOD13_CNT, n)

// REG_15
#define REG_BUCK_VANA_INTERNAL_FREQUENCY_SHIFT 8
#define REG_BUCK_VANA_INTERNAL_FREQUENCY_MASK (0x7 << REG_BUCK_VANA_INTERNAL_FREQUENCY_SHIFT)
#define REG_BUCK_VANA_INTERNAL_FREQUENCY(n) BITFIELD_VAL(REG_BUCK_VANA_INTERNAL_FREQUENCY, n)
#define REG_BUCK_VANA_DT_BIT                (1 << 6)
#define REG_BUCK_VANA_PULLDOWN_EN           (1 << 5)
#define REG_BUCK_VANA_ANTI_RES_DISABLE      (1 << 4)
#define REG_BUCK_VANA_SYNC_DISABLE          (1 << 3)
#define REG_BUCK_VANA_SOFT_START_EN         (1 << 2)
#define REG_BUCK_VANA_SLOPE_DOUBLE          (1 << 0)

// REG_17
#define REG_SS_VCORE_EN                     (1 << 15)
#define RESERVED_ANA_23                     (1 << 14)
#define REG_GPADC_RESETN_DR                 (1 << 11)
#define REG_GPADC_RESETN                    (1 << 10)
#define SAR_PWR_BIT_SHIFT                   8
#define SAR_PWR_BIT_MASK                    (0x3 << SAR_PWR_BIT_SHIFT)
#define SAR_PWR_BIT(n)                      BITFIELD_VAL(SAR_PWR_BIT, n)
#define SAR_OP_IBIT_SHIFT                   5
#define SAR_OP_IBIT_MASK                    (0x7 << SAR_OP_IBIT_SHIFT)
#define SAR_OP_IBIT(n)                      BITFIELD_VAL(SAR_OP_IBIT, n)
#define SAR_THERM_GAIN_SHIFT                3
#define SAR_THERM_GAIN_MASK                 (0x3 << SAR_THERM_GAIN_SHIFT)
#define SAR_THERM_GAIN(n)                   BITFIELD_VAL(SAR_THERM_GAIN, n)
#define SAR_VREF_BIT_SHIFT                  0
#define SAR_VREF_BIT_MASK                   (0x7 << SAR_VREF_BIT_SHIFT)
#define SAR_VREF_BIT(n)                     BITFIELD_VAL(SAR_VREF_BIT, n)

// REG_18
#define SAR_MODE_SEL                        (1 << 15)
#define KEY_DB_DSB                          (1 << 14)
#define KEY_INTERVAL_MODE                   (1 << 13)
#define GPADC_INTERVAL_MODE                 (1 << 12)
#define STATE_RESET                         (1 << 11)
#define SAR_ADC_MODE                        (1 << 10)
#define DELAY_BEFORE_SAMP_SHIFT             8
#define DELAY_BEFORE_SAMP_MASK              (0x3 << DELAY_BEFORE_SAMP_SHIFT)
#define DELAY_BEFORE_SAMP(n)                BITFIELD_VAL(DELAY_BEFORE_SAMP, n)
#define CONV_CLK_INV                        (1 << 7)
#define SAMP_CLK_INV                        (1 << 6)
#define TIME_SAMP_NEG                       (1 << 5)
#define TIME_SAMP_POS                       (1 << 4)
#define SAR_OUT_POLARITY                    (1 << 3)
#define TIMER_SAR_STABLE_SEL_SHIFT          0
#define TIMER_SAR_STABLE_SEL_MASK           (0x7 << TIMER_SAR_STABLE_SEL_SHIFT)
#define TIMER_SAR_STABLE_SEL(n)             BITFIELD_VAL(TIMER_SAR_STABLE_SEL, n)

// REG_19
#define REG_CLK_GPADC_DIV_VALUE_SHIFT       11
#define REG_CLK_GPADC_DIV_VALUE_MASK        (0x1F << REG_CLK_GPADC_DIV_VALUE_SHIFT)
#define REG_CLK_GPADC_DIV_VALUE(n)          BITFIELD_VAL(REG_CLK_GPADC_DIV_VALUE, n)
#define REG_KEY_IN_DR                       (1 << 10)
#define REG_KEY_IN_REG                      (1 << 9)
#define REG_GPADC_EN_DR                     (1 << 8)
#define REG_GPADC_EN_REG_SHIFT              0
#define REG_GPADC_EN_REG_MASK               (0xFF << REG_GPADC_EN_REG_SHIFT)
#define REG_GPADC_EN_REG(n)                 BITFIELD_VAL(REG_GPADC_EN_REG, n)

// REG_1A
#define REG_PMU_VSEL1_SHIFT                 13
#define REG_PMU_VSEL1_MASK                  (0x7 << REG_PMU_VSEL1_SHIFT)
#define REG_PMU_VSEL1(n)                    BITFIELD_VAL(REG_PMU_VSEL1, n)
#define REG_POWER_SEL_CNT_SHIFT             8
#define REG_POWER_SEL_CNT_MASK              (0x1F << REG_POWER_SEL_CNT_SHIFT)
#define REG_POWER_SEL_CNT(n)                BITFIELD_VAL(REG_POWER_SEL_CNT, n)
#define REG_32K_GATE_EN                     (1 << 7)
#define REG_PWR_SEL                         (1 << 6)
#define CLK_BG_EN                           (1 << 5)
#define CLK_BG_DIV_VALUE_SHIFT              0
#define CLK_BG_DIV_VALUE_MASK               (0x1F << CLK_BG_DIV_VALUE_SHIFT)
#define CLK_BG_DIV_VALUE(n)                 BITFIELD_VAL(CLK_BG_DIV_VALUE, n)

// REG_1B
#define KEY_INTERVAL_SHIFT                  0
#define KEY_INTERVAL_MASK                   (0xFFFF << KEY_INTERVAL_SHIFT)
#define KEY_INTERVAL(n)                     BITFIELD_VAL(KEY_INTERVAL, n)

// REG_1C
#define GPADC_INTERVAL_SHIFT                0
#define GPADC_INTERVAL_MASK                 (0xFFFF << GPADC_INTERVAL_SHIFT)
#define GPADC_INTERVAL(n)                   BITFIELD_VAL(GPADC_INTERVAL, n)

// REG_1D
#define SLEEP_ALLOW                         (1 << 15)
#define CHIP_ADDR_I2C_SHIFT                 8
#define CHIP_ADDR_I2C_MASK                  (0x7F << CHIP_ADDR_I2C_SHIFT)
#define CHIP_ADDR_I2C(n)                    BITFIELD_VAL(CHIP_ADDR_I2C, n)
#define CHAN_EN_REG_SHIFT                   0
#define CHAN_EN_REG_MASK                    (0xFF << CHAN_EN_REG_SHIFT)
#define CHAN_EN_REG(n)                      BITFIELD_VAL(CHAN_EN_REG, n)

// REG_1E
#define RESERVED_ANA_24                     (1 << 15)
#define REG_SAR_REF_MODE_SEL                (1 << 14)
#define REG_CLK_GPADC_EN                    (1 << 13)
#define REG_DR_PU_SAR                       (1 << 12)
#define REG_PU_SAR                          (1 << 11)
#define REG_DR_TSC_SAR_BIT                  (1 << 10)
#define REG_TSC_SAR_BIT_SHIFT               0
#define REG_TSC_SAR_BIT_MASK                (0x3FF << REG_TSC_SAR_BIT_SHIFT)
#define REG_TSC_SAR_BIT(n)                  BITFIELD_VAL(REG_TSC_SAR_BIT, n)

// REG_1F
#define USB_INSERT_INTR_MSK                 (1 << 15)
#define RTC_INT1_MSK                        (1 << 14)
#define RTC_INT0_MSK                        (1 << 13)
#define KEY_ERR1_INTR_MSK                   (1 << 12)
#define KEY_ERR0_INTR_MSK                   (1 << 11)
#define KEY_PRESS_INTR_MSK                  (1 << 10)
#define KEY_RELEASE_INTR_MSK                (1 << 9)
#define SAMPLE_DONE_INTR_MSK                (1 << 8)
#define CHAN_DATA_INTR_MSK_SHIFT            0
#define CHAN_DATA_INTR_MSK_MASK             (0xFF << CHAN_DATA_INTR_MSK_SHIFT)
#define CHAN_DATA_INTR_MSK(n)               BITFIELD_VAL(CHAN_DATA_INTR_MSK, n)


// REG_20
#define RTC_INT_EN_1                        (1 << 14)
#define RTC_INT_EN_0                        (1 << 13)
#define KEY_ERR1_INTR_EN                    (1 << 12)
#define KEY_ERR0_INTR_EN                    (1 << 11)
#define KEY_PRESS_INTR_EN                   (1 << 10)
#define KEY_RELEASE_INTR_EN                 (1 << 9)
#define SAMPLE_DONE_INTR_EN                 (1 << 8)
#define CHAN_DATA_INTR_EN_SHIFT             0
#define CHAN_DATA_INTR_EN_MASK              (0xFF << CHAN_DATA_INTR_EN_SHIFT)
#define CHAN_DATA_INTR_EN(n)                BITFIELD_VAL(CHAN_DATA_INTR_EN, n)

// REG_21
#define RESERVED_ANA_26_25_SHIFT            14
#define RESERVED_ANA_26_25_MASK             (0x3 << RESERVED_ANA_26_25_SHIFT)
#define RESERVED_ANA_26_25(n)               BITFIELD_VAL(RESERVED_ANA_26_25, n)


// REG_22
#define CFG_DIV_RTC_1HZ_SHIFT               0
#define CFG_DIV_RTC_1HZ_MASK                (0xFFFF << CFG_DIV_RTC_1HZ_SHIFT)
#define CFG_DIV_RTC_1HZ(n)                  BITFIELD_VAL(CFG_DIV_RTC_1HZ, n)

// REG_29
#define REG_MIC_BIASA_CHANSEL_SHIFT         14
#define REG_MIC_BIASA_CHANSEL_MASK          (0x3 << REG_MIC_BIASA_CHANSEL_SHIFT)
#define REG_MIC_BIASA_CHANSEL(n)            BITFIELD_VAL(REG_MIC_BIASA_CHANSEL, n)
#define REG_MIC_BIASA_EN                    (1 << 13)
#define REG_MIC_BIASA_ENLPF                 (1 << 12)
#define RESERVED_ANA_22                     (1 << 11)
#define REG_MIC_BIASA_VSEL_SHIFT            5
#define REG_MIC_BIASA_VSEL_MASK             (0x3F << REG_MIC_BIASA_VSEL_SHIFT)
#define REG_MIC_BIASA_VSEL(n)               BITFIELD_VAL(REG_MIC_BIASA_VSEL, n)
#define REG_MIC_LDO_RES_SHIFT               1
#define REG_MIC_LDO_RES_MASK                (0xF << REG_MIC_LDO_RES_SHIFT)
#define REG_MIC_LDO_RES(n)                  BITFIELD_VAL(REG_MIC_LDO_RES, n)
#define REG_MIC_LDO_LOOPCTRL                (1 << 0)

// REG_2A
#define REG_MIC_BIASB_CHANSEL_SHIFT         14
#define REG_MIC_BIASB_CHANSEL_MASK          (0x3 << REG_MIC_BIASB_CHANSEL_SHIFT)
#define REG_MIC_BIASB_CHANSEL(n)            BITFIELD_VAL(REG_MIC_BIASB_CHANSEL, n)
#define REG_MIC_BIASB_EN                    (1 << 13)
#define REG_MIC_BIASB_ENLPF                 (1 << 12)

#define REG_MIC_BIASB_VSEL_SHIFT            5
#define REG_MIC_BIASB_VSEL_MASK             (0x3F << REG_MIC_BIASB_VSEL_SHIFT)
#define REG_MIC_BIASB_VSEL(n)               BITFIELD_VAL(REG_MIC_BIASB_VSEL, n)
#define REG_MIC_LDO_EN                      (1 << 4)
#define REG_MIC_LDO_PULLDOWN                (1 << 3)

// REG_2B
#define REG_LED_IO1_IBIT_SHIFT              14
#define REG_LED_IO1_IBIT_MASK               (0x3 << REG_LED_IO1_IBIT_SHIFT)
#define REG_LED_IO1_IBIT(n)                 BITFIELD_VAL(REG_LED_IO1_IBIT, n)
#define REG_LED_IO1_OENB_PRE                (1 << 13)
#define REG_LED_IO1_PDEN                    (1 << 12)
#define REG_LED_IO1_PU                      (1 << 11)
#define REG_LED_IO1_PUEN                    (1 << 10)
#define REG_LED_IO1_SEL_SHIFT               8
#define REG_LED_IO1_SEL_MASK                (0x3 << REG_LED_IO1_SEL_SHIFT)
#define REG_LED_IO1_SEL(n)                  BITFIELD_VAL(REG_LED_IO1_SEL, n)
#define REG_LED_IO1_RX_EN                   (1 << 7)
#define REG_LED_IO1_AIO_EN                  (1 << 6)
#define RESERVED_ANA_27                     (1 << 5)

// REG_2C
#define RESERVED_ANA_28                     (1 << 13)

// REG_2D
#define R_PWM2_TOGGLE_SHIFT                 0
#define R_PWM2_TOGGLE_MASK                  (0xFFFF << R_PWM2_TOGGLE_SHIFT)
#define R_PWM2_TOGGLE(n)                    BITFIELD_VAL(R_PWM2_TOGGLE, n)

// REG_2E
#define REG_PWM2_ST1_SHIFT                  0
#define REG_PWM2_ST1_MASK                   (0xFFFF << REG_PWM2_ST1_SHIFT)
#define REG_PWM2_ST1(n)                     BITFIELD_VAL(REG_PWM2_ST1, n)

// REG_2F
#define SUBCNT_DATA2_SHIFT                  8
#define SUBCNT_DATA2_MASK                   (0xFF << SUBCNT_DATA2_SHIFT)
#define SUBCNT_DATA2(n)                     BITFIELD_VAL(SUBCNT_DATA2, n)
#define TG_SUBCNT_D2_ST_SHIFT               1
#define TG_SUBCNT_D2_ST_MASK                (0x7F << TG_SUBCNT_D2_ST_SHIFT)
#define TG_SUBCNT_D2_ST(n)                  BITFIELD_VAL(TG_SUBCNT_D2_ST, n)
#define REG_LED0_OUT                        (1 << 0)

// REG_30
#define RESERVED_ANA_29                     (1 << 15)
#define RESETN_DB_NUMBER_SHIFT              11
#define RESETN_DB_NUMBER_MASK               (0xF << RESETN_DB_NUMBER_SHIFT)
#define RESETN_DB_NUMBER(n)                 BITFIELD_VAL(RESETN_DB_NUMBER, n)
#define RESETN_DB_EN                        (1 << 10)
#define REG_PWM_CLK_EN                      (1 << 9)
#define REG_CLK_PWM_DIV_SHIFT               4
#define REG_CLK_PWM_DIV_MASK                (0x1F << REG_CLK_PWM_DIV_SHIFT)
#define REG_CLK_PWM_DIV(n)                  BITFIELD_VAL(REG_CLK_PWM_DIV, n)
#define REG_PWM2_BR_EN                      (1 << 3)
#define PWM_SELECT_EN                       (1 << 2)
#define PWM_SELECT_INV                      (1 << 1)
#define LED_GPIO_SELECT                     (1 << 0)

// REG_34
#define RESERVED_ANA_30                     (1 << 15)
#define CLK_32K_COUNT_NUM_SHIFT             11
#define CLK_32K_COUNT_NUM_MASK              (0xF << CLK_32K_COUNT_NUM_SHIFT)
#define CLK_32K_COUNT_NUM(n)                BITFIELD_VAL(CLK_32K_COUNT_NUM, n)
#define DIG_IPTAT_CORE_EN                   (1 << 4)
#define CLK_32K_COUNT_EN                    (1 << 0)

// REG_35
#define REG_DIV_HW_RESET_SHIFT              0
#define REG_DIV_HW_RESET_MASK               (0xFFFF << REG_DIV_HW_RESET_SHIFT)
#define REG_DIV_HW_RESET(n)                 BITFIELD_VAL(REG_DIV_HW_RESET, n)

// REG_36
#define REG_WDT_TIMER_SHIFT                 0
#define REG_WDT_TIMER_MASK                  (0xFFFF << REG_WDT_TIMER_SHIFT)
#define REG_WDT_TIMER(n)                    BITFIELD_VAL(REG_WDT_TIMER, n)

// REG_37
#define RESERVED_ANA_33_31_SHIFT            13
#define RESERVED_ANA_33_31_MASK             (0x7 << RESERVED_ANA_33_31_SHIFT)
#define RESERVED_ANA_33_31(n)               BITFIELD_VAL(RESERVED_ANA_33_31, n)
#define CLK_32K_COUNT_EN_TRIGGER            (1 << 12)
#define CLK_32K_COUNT_CLOCK_EN              (1 << 11)
#define POWERON_DETECT_EN                   (1 << 10)
#define MERGE_INTR                          (1 << 9)
#define REG_WDT_EN                          (1 << 8)
#define REG_WDT_RESET_EN                    (1 << 7)
#define REG_HW_RESET_TIME_SHIFT             1
#define REG_HW_RESET_TIME_MASK              (0x3F << REG_HW_RESET_TIME_SHIFT)
#define REG_HW_RESET_TIME(n)                BITFIELD_VAL(REG_HW_RESET_TIME, n)
#define REG_HW_RESET_EN                     (1 << 0)

// REG_38
#define RESERVED_ANA_EN                     (1 << 15)
#define RESERVED_ANA_01                     (1 << 1)

// REG_39
#define RESERVED_DIG_15_0_SHIFT             0
#define RESERVED_DIG_15_0_MASK              (0xFFFF << RESERVED_DIG_15_0_SHIFT)
#define RESERVED_DIG_15_0(n)                BITFIELD_VAL(RESERVED_DIG_15_0, n)

// REG_3A
#define R_PWMB_TOGGLE_SHIFT                 0
#define R_PWMB_TOGGLE_MASK                  (0xFFFF << R_PWMB_TOGGLE_SHIFT)
#define R_PWMB_TOGGLE(n)                    BITFIELD_VAL(R_PWMB_TOGGLE, n)

// REG_3B
#define REG_PWMB_ST1_SHIFT                  0
#define REG_PWMB_ST1_MASK                   (0xFFFF << REG_PWMB_ST1_SHIFT)
#define REG_PWMB_ST1(n)                     BITFIELD_VAL(REG_PWMB_ST1, n)

// REG_3C
#define SUBCNT_DATAB_SHIFT                  8
#define SUBCNT_DATAB_MASK                   (0xFF << SUBCNT_DATAB_SHIFT)
#define SUBCNT_DATAB(n)                     BITFIELD_VAL(SUBCNT_DATAB, n)
#define TG_SUBCNT_DB_ST_SHIFT               1
#define TG_SUBCNT_DB_ST_MASK                (0x7F << TG_SUBCNT_DB_ST_SHIFT)
#define TG_SUBCNT_DB_ST(n)                  BITFIELD_VAL(TG_SUBCNT_DB_ST, n)
#define REG_LEDB_OUT                        (1 << 0)

// REG_3D
#define RESERVED_PWMB_SHIFT                 10
#define RESERVED_PWMB_MASK                  (0x3F << RESERVED_PWMB_SHIFT)
#define RESERVED_PWMB(n)                    BITFIELD_VAL(RESERVED_PWMB, n)
#define REG_PWMB_CLK_EN                     (1 << 9)
#define REG_CLK_PWMB_DIV_SHIFT              4
#define REG_CLK_PWMB_DIV_MASK               (0x1F << REG_CLK_PWMB_DIV_SHIFT)
#define REG_CLK_PWMB_DIV(n)                 BITFIELD_VAL(REG_CLK_PWMB_DIV, n)
#define REG_PWMB_BR_EN                      (1 << 3)
#define PWMB_SELECT_EN                      (1 << 2)
#define PWMB_SELECT_INV                     (1 << 1)
#define LEDB_GPIO_SELECT                    (1 << 0)

// REG_3E
#define REG_LED_IO2_IBIT_SHIFT              14
#define REG_LED_IO2_IBIT_MASK               (0x3 << REG_LED_IO2_IBIT_SHIFT)
#define REG_LED_IO2_IBIT(n)                 BITFIELD_VAL(REG_LED_IO2_IBIT, n)
#define REG_LED_IO2_OENB_PRE                (1 << 13)
#define REG_LED_IO2_PDEN                    (1 << 12)
#define REG_LED_IO2_PU                      (1 << 11)
#define REG_LED_IO2_PUEN                    (1 << 10)
#define REG_LED_IO2_SEL_SHIFT               8
#define REG_LED_IO2_SEL_MASK                (0x3 << REG_LED_IO2_SEL_SHIFT)
#define REG_LED_IO2_SEL(n)                  BITFIELD_VAL(REG_LED_IO2_SEL, n)
#define REG_LED_IO2_RX_EN                   (1 << 7)
#define REG_LED_IO2_AIO_EN                  (1 << 6)
#define RESERVED_3E_SHIFT                   0
#define RESERVED_3E_MASK                    (0x3F << RESERVED_3E_SHIFT)
#define RESERVED_3E(n)                      BITFIELD_VAL(RESERVED_3E, n)

// REG_3F
#define AC_OUT_LDO_ON_EN                    (1 << 15)
#define REG_LED_DBG_32K_SEL                 (1 << 14)

// REG_40
#define RESERVED_40                         (1 << 15)
#define RTC_INTR_TMP_MERGED_MSK             (1 << 14)
#define GPADC_INTR_MERGED_MSK               (1 << 13)
#define CHARGE_INTR_MERGED_MSK              (1 << 12)

// REG_41
#define USB_INTR_MERGED_MSK                 (1 << 15)
#define POWER_ON_INTR_MERGED_MSK            (1 << 14)
#define PMU_GPIO_INTR_MSKED1_MERGED_MSK     (1 << 13)
#define PMU_GPIO_INTR_MSKED2_MERGED_MSK     (1 << 12)
#define WDT_INTR_MSKED_MERGED_MSK           (1 << 11)

// REG_42
#define RESERVED_42_SHIFT                   10
#define RESERVED_42_MASK                    (0x3F << RESERVED_42_SHIFT)
#define RESERVED_42(n)                      BITFIELD_VAL(RESERVED_42, n)
#define REG_BUCK_VANA_I_DELTAV_X2           (1 << 7)

// REG_43
#define REG_LOW_LEVEL_INTR_SEL1             (1 << 15)
#define REG_WDT_INTR_EN                     (1 << 14)
#define REG_WDT_INTR_MSK                    (1 << 13)

// REG_44
#define REG_BUCK_VANA_EDGE_CON_SHIFT        12
#define REG_BUCK_VANA_EDGE_CON_MASK         (0xF << REG_BUCK_VANA_EDGE_CON_SHIFT)
#define REG_BUCK_VANA_EDGE_CON(n)           BITFIELD_VAL(REG_BUCK_VANA_EDGE_CON, n)
#define REG_BUCK_VANA_HALF_BIAS             (1 << 11)
#define CLK_32K_SEL_SHIFT                   6
#define CLK_32K_SEL_MASK                    (0x3 << CLK_32K_SEL_SHIFT)
#define CLK_32K_SEL(n)                      BITFIELD_VAL(CLK_32K_SEL, n)
#define RESETN_RF_DR                        (1 << 5)
#define RESETN_RF_REG                       (1 << 4)
#define RESETN_MOD1_CNT_SHIFT               0
#define RESETN_MOD1_CNT_MASK                (0xF << RESETN_MOD1_CNT_SHIFT)
#define RESETN_MOD1_CNT(n)                  BITFIELD_VAL(RESETN_MOD1_CNT, n)

// REG_45
#define REG_EDGE_INTR_SEL2                  (1 << 14)
#define REG_POS_INTR_SEL2                   (1 << 13)
#define REG_EDGE_INTR_SEL1                  (1 << 12)
#define REG_POS_INTR_SEL1                   (1 << 11)

// REG_46
#define REG_GPIO_I_SEL                      (1 << 15)
#define PMU_DB_BYPASS1                      (1 << 14)
#define PMU_DB_TARGET1_SHIFT                6
#define PMU_DB_TARGET1_MASK                 (0xFF << PMU_DB_TARGET1_SHIFT)
#define PMU_DB_TARGET1(n)                   BITFIELD_VAL(PMU_DB_TARGET1, n)
#define REG_PMU_UART_DR1                    (1 << 5)
#define REG_PMU_UART_TX1                    (1 << 4)
#define REG_PMU_UART_OENB1                  (1 << 3)
#define REG_UART_LEDA_SEL                   (1 << 2)
#define REG_PMU_GPIO_INTR_MSK1              (1 << 1)
#define REG_PMU_GPIO_INTR_EN1               (1 << 0)

// REG_47
#define REG_LOW_LEVEL_INTR_SEL2             (1 << 15)
#define PMU_DB_BYPASS2                      (1 << 14)
#define PMU_DB_TARGET2_SHIFT                6
#define PMU_DB_TARGET2_MASK                 (0xFF << PMU_DB_TARGET2_SHIFT)
#define PMU_DB_TARGET2(n)                   BITFIELD_VAL(PMU_DB_TARGET2, n)
#define REG_PMU_UART_DR2                    (1 << 5)
#define REG_PMU_UART_TX2                    (1 << 4)
#define REG_PMU_UART_OENB2                  (1 << 3)
#define REG_UART_LEDB_SEL                   (1 << 2)
#define REG_PMU_GPIO_INTR_MSK2              (1 << 1)
#define REG_PMU_GPIO_INTR_EN2               (1 << 0)

// REG_48
#define PMU_GPIO_INTR_MSKED1                (1 << 15)
#define PMU_GPIO_INTR_MSKED2                (1 << 14)
#define LED_IO1_IN_DB                       (1 << 13)
#define LED_IO2_IN_DB                       (1 << 12)
#define WDT_INTR_MSKED                      (1 << 11)
#define POWER_DOWN_INTR_MSKED               (1 << 10)
#define BUCK_VCORE_BURST_MODE_OUT           (1 << 9)
#define BUCK_VANA_BURST_MODE_OUT            (1 << 8)
#define BUCK_VHPPA_BURST_MODE_OUT           (1 << 7)
#define REG_PMU_GPIO_INTR_CLR1              (1 << 15)
#define REG_PMU_GPIO_INTR_CLR2              (1 << 14)
#define REG_WDT_INTR_CLR                    (1 << 13)
#define REG_POWER_DOWN_INTR_CLR             (1 << 10)

// REG_49
#define REG_BUCK_VANA_LP_VCOMP              (1 << 15)
#define REG_BUCK_VANA_EN_ZCD_CAL            (1 << 7)
#define REG_BUCK_VANA_ZCD_CAP_BIT_SHIFT     0
#define REG_BUCK_VANA_ZCD_CAP_BIT_MASK      (0xF << REG_BUCK_VANA_ZCD_CAP_BIT_SHIFT)
#define REG_BUCK_VANA_ZCD_CAP_BIT(n)        BITFIELD_VAL(REG_BUCK_VANA_ZCD_CAP_BIT, n)

// REG_4B
#define DATA_CHAN_MSB_SHIFT                 0
#define DATA_CHAN_MSB_MASK                  (0xFF << DATA_CHAN_MSB_SHIFT)
#define DATA_CHAN_MSB(n)                    BITFIELD_VAL(DATA_CHAN_MSB, n)

// REG_4C
#define REG_VCORE_ON_DELAY_DR               (1 << 15)
#define REG_VCORE_ON_DELAY                  (1 << 14)

// REG_4D
#define REG_WDT_TIMER_INTR_SHIFT            0
#define REG_WDT_TIMER_INTR_MASK             (0xFFFF << REG_WDT_TIMER_INTR_SHIFT)
#define REG_WDT_TIMER_INTR(n)               BITFIELD_VAL(REG_WDT_TIMER_INTR, n)

// REG_4E
#define CLK_32K_COUNTER_26M_SHIFT           0
#define CLK_32K_COUNTER_26M_MASK            (0x7FFF << CLK_32K_COUNTER_26M_SHIFT)
#define CLK_32K_COUNTER_26M(n)              BITFIELD_VAL(CLK_32K_COUNTER_26M, n)
#define CLK_32K_COUNTER_26M_READY           (1 << 15)

// REG_4F
#define GPADC_START                         (1 << 5)
#define KEY_START                           (1 << 4)
#define AC_ON_EN                            (1 << 2)
#define HARDWARE_POWER_OFF_EN               (1 << 1)
#define SOFT_POWER_OFF                      (1 << 0)

// REG_50
#define USBINSERT_INTR2CPU                  (1 << 15)
#define RTC_INT_1                           (1 << 14)
#define RTC_INT_0                           (1 << 13)
#define KEY_ERR1_INTR                       (1 << 12)
#define KEY_ERR0_INTR                       (1 << 11)
#define KEY_PRESS_INTR                      (1 << 10)
#define KEY_RELEASE_INTR                    (1 << 9)
#define SAMPLE_PERIOD_DONE_INTR             (1 << 8)
#define CHAN_DATA_VALID_INTR_SHIFT          0
#define CHAN_DATA_VALID_INTR_MASK           (0xFF << CHAN_DATA_VALID_INTR_SHIFT)
#define CHAN_DATA_VALID_INTR(n)             BITFIELD_VAL(CHAN_DATA_VALID_INTR, n)

// REG_51
#define USB_INSERT_INTR_MSKED               (1 << 15)
#define RTC_INT1_MSKED                      (1 << 14)
#define RTC_INT0_MSKED                      (1 << 13)
#define KEY_ERR1_INTR_MSKED                 (1 << 12)
#define KEY_ERR0_INTR_MSKED                 (1 << 11)
#define KEY_PRESS_INTR_MSKED                (1 << 10)
#define KEY_RELEASE_INTR_MSKED              (1 << 9)
#define SAMPLE_DONE_INTR_MSKED              (1 << 8)
#define CHAN_DATA_INTR_MSKED_SHIFT          0
#define CHAN_DATA_INTR_MSKED_MASK           (0xFF << CHAN_DATA_INTR_MSKED_SHIFT)
#define CHAN_DATA_INTR_MSKED(n)             BITFIELD_VAL(CHAN_DATA_INTR_MSKED, n)
#define RTC_INT_CLR_1                       (1 << 14)
#define RTC_INT_CLR_0                       (1 << 13)
#define KEY_ERR1_INTR_CLR                   (1 << 12)
#define KEY_ERR0_INTR_CLR                   (1 << 11)
#define KEY_PRESS_INTR_CLR                  (1 << 10)
#define KEY_RELEASE_INTR_CLR                (1 << 9)
#define SAMPLE_DONE_INTR_CLR                (1 << 8)
#define CHAN_DATA_INTR_CLR_SHIFT            0
#define CHAN_DATA_INTR_CLR_MASK             (0xFF << CHAN_DATA_INTR_CLR_SHIFT)
#define CHAN_DATA_INTR_CLR(n)               BITFIELD_VAL(CHAN_DATA_INTR_CLR, n)

// REG_52
#define WDT_LOAD_INTR                       (1 << 9)
#define PMU_GPIO_INTR2                      (1 << 8)
#define PMU_GPIO_INTR1                      (1 << 7)
#define R_WDT_INTR                          (1 << 6)
#define USB_DTESTO_TX                       (1 << 5)
#define VCORE_ON_DELAY                      (1 << 4)
#define WDT_LOAD                            (1 << 2)
#define CORE_GPIO_OUT1                      (1 << 1)
#define POWER_ON                            (1 << 0)

// REG_56
#define DATA_CHAN0_SHIFT                    0
#define DATA_CHAN0_MASK                     (0xFFFF << DATA_CHAN0_SHIFT)
#define DATA_CHAN0(n)                       BITFIELD_VAL(DATA_CHAN0, n)

// REG_57
#define DATA_CHAN1_SHIFT                    0
#define DATA_CHAN1_MASK                     (0xFFFF << DATA_CHAN1_SHIFT)
#define DATA_CHAN1(n)                       BITFIELD_VAL(DATA_CHAN1, n)

// REG_58
#define DATA_CHAN2_SHIFT                    0
#define DATA_CHAN2_MASK                     (0xFFFF << DATA_CHAN2_SHIFT)
#define DATA_CHAN2(n)                       BITFIELD_VAL(DATA_CHAN2, n)

// REG_59
#define DATA_CHAN3_SHIFT                    0
#define DATA_CHAN3_MASK                     (0xFFFF << DATA_CHAN3_SHIFT)
#define DATA_CHAN3(n)                       BITFIELD_VAL(DATA_CHAN3, n)

// REG_5A
#define DATA_CHAN4_SHIFT                    0
#define DATA_CHAN4_MASK                     (0xFFFF << DATA_CHAN4_SHIFT)
#define DATA_CHAN4(n)                       BITFIELD_VAL(DATA_CHAN4, n)

// REG_5B
#define DATA_CHAN5_SHIFT                    0
#define DATA_CHAN5_MASK                     (0xFFFF << DATA_CHAN5_SHIFT)
#define DATA_CHAN5(n)                       BITFIELD_VAL(DATA_CHAN5, n)

// REG_5C
#define DATA_CHAN6_SHIFT                    0
#define DATA_CHAN6_MASK                     (0xFFFF << DATA_CHAN6_SHIFT)
#define DATA_CHAN6(n)                       BITFIELD_VAL(DATA_CHAN6, n)

// REG_5D
#define DATA_CHAN7_SHIFT                    0
#define DATA_CHAN7_MASK                     (0xFFFF << DATA_CHAN7_SHIFT)
#define DATA_CHAN7(n)                       BITFIELD_VAL(DATA_CHAN7, n)

// REG_5E
#define POWER_ON_RELEASE                    (1 << 15)
#define REG_POWER_ON_RELEASE_CLR            (1 << 15)
#define POWER_ON_PRESS                      (1 << 14)
#define REG_POWER_ON_PRESS_CLR              (1 << 14)
#define DIG_LDO_VSENSOR_EN                  (1 << 11)
#define DIG_LDO_VUSB_EN                     (1 << 10)

#define DIG_PU_VRTC_RF                      (1 << 7)
#define DIG_PU_BIAS_LDO                     (1 << 6)
#define DIG_PU_LP_BIAS_LDO                  (1 << 5)
#define INTR_MSKED_CHARGE_SHIFT             3
#define INTR_MSKED_CHARGE_MASK              (0x3 << INTR_MSKED_CHARGE_SHIFT)
#define INTR_MSKED_CHARGE(n)                BITFIELD_VAL(INTR_MSKED_CHARGE, n)
#define AC_ON                               (1 << 2)
#define AC_ON_DET_OUT                       (1 << 1)
#define REG_CHARGE_OUT_INTR_CLR             (1 << 1)
#define AC_ON_DET_IN                        (1 << 0)
#define REG_CHARGE_IN_INTR_CLR              (1 << 0)

// REG_5F
#define RC_SLEEP_MODE                       (1 << 15)
#define DIG_PU_SAR                          (1 << 14)
#define DIG_PU_DCDC_VANA                    (1 << 13)
#define DIG_PU_DCDC_VCORE                   (1 << 12)
#define DIG_PU_DCDC_VHPPA                   (1 << 11)
#define DIG_LDO_VCORE_EN                    (1 << 10)
#define DIG_LDO_VANA_EN                     (1 << 9)
#define DIG_LDO_VMEM_EN                     (1 << 8)
#define DIG_LDO_VCODEC_EN                   (1 << 7)
#define DEEPSLEEP_MODE                      (1 << 6)
#define UVLO_LV                             (1 << 5)
#define DIG_PU_LPO                          (1 << 4)
#define PMU_LDO_ON_EN                       (1 << 0)

// REG_60
#define REG_WD_RESETN_INFOR_SHIFT           0
#define REG_WD_RESETN_INFOR_MASK            (0xFFFF << REG_WD_RESETN_INFOR_SHIFT)
#define REG_WD_RESETN_INFOR(n)              BITFIELD_VAL(REG_WD_RESETN_INFOR, n)

// REG_61
#define REG_COMMAND_RST_DIS                 (1 << 15)
#define REG_NOT_RESET_SHIFT                 2
#define REG_NOT_RESET_MASK                  (0x1FFF << REG_NOT_RESET_SHIFT)
#define REG_NOT_RESET(n)                    BITFIELD_VAL(REG_NOT_RESET, n)
#define REG_NOT_RESET_BIT1                  (1 << 1)
#define REG_NOT_RESET_BIT0                  (1 << 0)

#define REG_NOT_RESET_RTC_ENABLED           REG_NOT_RESET_BIT1
#define REG_NOT_RESET_CHIP_PWR_ON           REG_NOT_RESET_BIT0

// REG_62
#define RESETN_A7PLL_REG                    (1 << 11)
#define RESETN_A7PLL_DR                     (1 << 10)
#define RESETN_PSRAM1_REG                   (1 << 9)
#define RESETN_PSRAM1_DR                    (1 << 8)
#define RESETN_PSRAM0_REG                   (1 << 7)
#define RESETN_PSRAM0_DR                    (1 << 6)
#define RESETN_PSRAMPLL_REG                 (1 << 5)
#define RESETN_PSRAMPLL_DR                  (1 << 4)
#define RESETN_BT_REG                       (1 << 3)
#define RESETN_BT_DR                        (1 << 2)
#define RESETN_WIFI_REG                     (1 << 1)
#define RESETN_WIFI_DR                      (1 << 0)

// REG_63
#define REG_BUCK_VANA_DIV_RFB_RES_X2        (1 << 15)
#define REG_BUCK_VANA_KICK_PD_SLEEP_SEL     (1 << 14)

// REG_64
#define REG_BUCK_VCORE_ICOMP_DC_HIGH        (1 << 15)
#define REG_BUCK_VCORE_DIV_RFB_RES_X2       (1 << 14)
#define REG_BUCK_VCORE_KICK_PD_SLEEP_SEL    (1 << 13)
#define REG_BUCK_VCORE_INTERNAL_FREQUENCY_SHIFT 10
#define REG_BUCK_VCORE_INTERNAL_FREQUENCY_MASK (0x7 << REG_BUCK_VCORE_INTERNAL_FREQUENCY_SHIFT)
#define REG_BUCK_VCORE_INTERNAL_FREQUENCY(n) BITFIELD_VAL(REG_BUCK_VCORE_INTERNAL_FREQUENCY, n)
#define REG_BUCK_VCORE_DT_BIT               (1 << 8)
#define REG_BUCK_VCORE_PULLDOWN_EN          (1 << 7)
#define REG_BUCK_VCORE_ANTI_RES_DISABLE     (1 << 6)
#define REG_BUCK_VCORE_SYNC_DISABLE         (1 << 3)
#define REG_BUCK_VCORE_SOFT_START_EN        (1 << 2)
#define REG_BUCK_VCORE_SLOPE_DOUBLE         (1 << 0)

// REG_65
#define REG_BUCK_VCORE_I_DELTAV_X2          (1 << 14)
#define REG_BUCK_VCORE_ZCD_CAP_BIT_SHIFT    4
#define REG_BUCK_VCORE_ZCD_CAP_BIT_MASK     (0xF << REG_BUCK_VCORE_ZCD_CAP_BIT_SHIFT)
#define REG_BUCK_VCORE_ZCD_CAP_BIT(n)       BITFIELD_VAL(REG_BUCK_VCORE_ZCD_CAP_BIT, n)
#define REG_BUCK_VCORE_EN_ZCD_CAL           (1 << 3)

// REG_66
#define REG_COMMAND_RST_BIT_WIDTH_TOLERANCE_SHIFT 8
#define REG_COMMAND_RST_BIT_WIDTH_TOLERANCE_MASK (0xFF << REG_COMMAND_RST_BIT_WIDTH_TOLERANCE_SHIFT)
#define REG_COMMAND_RST_BIT_WIDTH_TOLERANCE(n) BITFIELD_VAL(REG_COMMAND_RST_BIT_WIDTH_TOLERANCE, n)

#define REG_BUCK_VCORE_LP_VCOMP             (1 << 5)
#define REG_BUCK_VCORE_HALF_BIAS            (1 << 4)
#define REG_BUCK_VCORE_EDGE_CON_SHIFT       0
#define REG_BUCK_VCORE_EDGE_CON_MASK        (0xF << REG_BUCK_VCORE_EDGE_CON_SHIFT)
#define REG_BUCK_VCORE_EDGE_CON(n)          BITFIELD_VAL(REG_BUCK_VCORE_EDGE_CON, n)

// REG_67
#define REG_MIC_BIASB_PULLDOWN              (1 << 2)
#define REG_MIC_BIASA_PULLDOWN              (1 << 1)
#define REG_MIC_LP_ENABLE                   (1 << 0)

// REG_6A
#define REG_BIAS_SEL_LDO                    (1 << 7)
#define LP_MODE_RTC_DR                      (1 << 6)
#define LP_MODE_RTC_REG                     (1 << 5)

// REG_68
#define REG_RMAP_MODE_SEL                   (1 << 6)

// REG_6D
#define REG_SAR_RESULT_SEL                  (1 << 15)
#define REG_SAR_ADC_ON                      (1 << 14)
#define SAR_BIT00_WEIGHT_SHIFT              0
#define SAR_BIT00_WEIGHT_MASK               (0x3FFF << SAR_BIT00_WEIGHT_SHIFT)
#define SAR_BIT00_WEIGHT(n)                 BITFIELD_VAL(SAR_BIT00_WEIGHT, n)

// REG_6E
#define SAR_BIT01_WEIGHT_SHIFT              0
#define SAR_BIT01_WEIGHT_MASK               (0x3FFF << SAR_BIT01_WEIGHT_SHIFT)
#define SAR_BIT01_WEIGHT(n)                 BITFIELD_VAL(SAR_BIT01_WEIGHT, n)

// REG_6F
#define SAR_BIT02_WEIGHT_SHIFT              0
#define SAR_BIT02_WEIGHT_MASK               (0x3FFF << SAR_BIT02_WEIGHT_SHIFT)
#define SAR_BIT02_WEIGHT(n)                 BITFIELD_VAL(SAR_BIT02_WEIGHT, n)

// REG_70
#define SAR_BIT03_WEIGHT_SHIFT              0
#define SAR_BIT03_WEIGHT_MASK               (0x3FFF << SAR_BIT03_WEIGHT_SHIFT)
#define SAR_BIT03_WEIGHT(n)                 BITFIELD_VAL(SAR_BIT03_WEIGHT, n)

// REG_71
#define SAR_BIT04_WEIGHT_SHIFT              0
#define SAR_BIT04_WEIGHT_MASK               (0x3FFF << SAR_BIT04_WEIGHT_SHIFT)
#define SAR_BIT04_WEIGHT(n)                 BITFIELD_VAL(SAR_BIT04_WEIGHT, n)

// REG_72
#define SAR_P_BIT05_WEIGHT_SHIFT            0
#define SAR_P_BIT05_WEIGHT_MASK             (0x3FFF << SAR_P_BIT05_WEIGHT_SHIFT)
#define SAR_P_BIT05_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT05_WEIGHT, n)

// REG_73
#define SAR_P_BIT06_WEIGHT_SHIFT            0
#define SAR_P_BIT06_WEIGHT_MASK             (0x3FFF << SAR_P_BIT06_WEIGHT_SHIFT)
#define SAR_P_BIT06_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT06_WEIGHT, n)

// REG_74
#define SAR_P_BIT07_WEIGHT_SHIFT            0
#define SAR_P_BIT07_WEIGHT_MASK             (0x3FFF << SAR_P_BIT07_WEIGHT_SHIFT)
#define SAR_P_BIT07_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT07_WEIGHT, n)

// REG_75
#define SAR_P_BIT08_WEIGHT_SHIFT            0
#define SAR_P_BIT08_WEIGHT_MASK             (0x3FFF << SAR_P_BIT08_WEIGHT_SHIFT)
#define SAR_P_BIT08_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT08_WEIGHT, n)

// REG_76
#define SAR_P_BIT09_WEIGHT_SHIFT            0
#define SAR_P_BIT09_WEIGHT_MASK             (0x3FFF << SAR_P_BIT09_WEIGHT_SHIFT)
#define SAR_P_BIT09_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT09_WEIGHT, n)

// REG_77
#define SAR_P_BIT10_WEIGHT_SHIFT            0
#define SAR_P_BIT10_WEIGHT_MASK             (0x3FFF << SAR_P_BIT10_WEIGHT_SHIFT)
#define SAR_P_BIT10_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT10_WEIGHT, n)

// REG_78
#define SAR_P_BIT11_WEIGHT_SHIFT            0
#define SAR_P_BIT11_WEIGHT_MASK             (0x3FFF << SAR_P_BIT11_WEIGHT_SHIFT)
#define SAR_P_BIT11_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT11_WEIGHT, n)

// REG_79
#define SAR_P_BIT12_WEIGHT_SHIFT            0
#define SAR_P_BIT12_WEIGHT_MASK             (0x3FFF << SAR_P_BIT12_WEIGHT_SHIFT)
#define SAR_P_BIT12_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT12_WEIGHT, n)

// REG_7A
#define SAR_P_BIT13_WEIGHT_SHIFT            0
#define SAR_P_BIT13_WEIGHT_MASK             (0x3FFF << SAR_P_BIT13_WEIGHT_SHIFT)
#define SAR_P_BIT13_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT13_WEIGHT, n)

// REG_7B
#define SAR_P_BIT14_WEIGHT_SHIFT            0
#define SAR_P_BIT14_WEIGHT_MASK             (0x3FFF << SAR_P_BIT14_WEIGHT_SHIFT)
#define SAR_P_BIT14_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT14_WEIGHT, n)

// REG_7C
#define SAR_P_BIT15_WEIGHT_SHIFT            0
#define SAR_P_BIT15_WEIGHT_MASK             (0x3FFF << SAR_P_BIT15_WEIGHT_SHIFT)
#define SAR_P_BIT15_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT15_WEIGHT, n)

// REG_7D
#define SAR_P_BIT16_WEIGHT_SHIFT            0
#define SAR_P_BIT16_WEIGHT_MASK             (0x3FFF << SAR_P_BIT16_WEIGHT_SHIFT)
#define SAR_P_BIT16_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT16_WEIGHT, n)

// REG_7E
#define SAR_P_BIT17_WEIGHT_SHIFT            0
#define SAR_P_BIT17_WEIGHT_MASK             (0x3FFF << SAR_P_BIT17_WEIGHT_SHIFT)
#define SAR_P_BIT17_WEIGHT(n)               BITFIELD_VAL(SAR_P_BIT17_WEIGHT, n)

// REG_7F
#define SAR_N_BIT05_WEIGHT_SHIFT            0
#define SAR_N_BIT05_WEIGHT_MASK             (0x3FFF << SAR_N_BIT05_WEIGHT_SHIFT)
#define SAR_N_BIT05_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT05_WEIGHT, n)

// REG_80
#define SAR_N_BIT06_WEIGHT_SHIFT            0
#define SAR_N_BIT06_WEIGHT_MASK             (0x3FFF << SAR_N_BIT06_WEIGHT_SHIFT)
#define SAR_N_BIT06_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT06_WEIGHT, n)

// REG_81
#define SAR_N_BIT07_WEIGHT_SHIFT            0
#define SAR_N_BIT07_WEIGHT_MASK             (0x3FFF << SAR_N_BIT07_WEIGHT_SHIFT)
#define SAR_N_BIT07_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT07_WEIGHT, n)

// REG_82
#define SAR_N_BIT08_WEIGHT_SHIFT            0
#define SAR_N_BIT08_WEIGHT_MASK             (0x3FFF << SAR_N_BIT08_WEIGHT_SHIFT)
#define SAR_N_BIT08_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT08_WEIGHT, n)

// REG_83
#define SAR_N_BIT09_WEIGHT_SHIFT            0
#define SAR_N_BIT09_WEIGHT_MASK             (0x3FFF << SAR_N_BIT09_WEIGHT_SHIFT)
#define SAR_N_BIT09_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT09_WEIGHT, n)

// REG_84
#define SAR_N_BIT10_WEIGHT_SHIFT            0
#define SAR_N_BIT10_WEIGHT_MASK             (0x3FFF << SAR_N_BIT10_WEIGHT_SHIFT)
#define SAR_N_BIT10_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT10_WEIGHT, n)

// REG_85
#define SAR_N_BIT11_WEIGHT_SHIFT            0
#define SAR_N_BIT11_WEIGHT_MASK             (0x3FFF << SAR_N_BIT11_WEIGHT_SHIFT)
#define SAR_N_BIT11_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT11_WEIGHT, n)

// REG_86
#define SAR_N_BIT12_WEIGHT_SHIFT            0
#define SAR_N_BIT12_WEIGHT_MASK             (0x3FFF << SAR_N_BIT12_WEIGHT_SHIFT)
#define SAR_N_BIT12_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT12_WEIGHT, n)

// REG_87
#define SAR_N_BIT13_WEIGHT_SHIFT            0
#define SAR_N_BIT13_WEIGHT_MASK             (0x3FFF << SAR_N_BIT13_WEIGHT_SHIFT)
#define SAR_N_BIT13_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT13_WEIGHT, n)

// REG_88
#define SAR_N_BIT14_WEIGHT_SHIFT            0
#define SAR_N_BIT14_WEIGHT_MASK             (0x3FFF << SAR_N_BIT14_WEIGHT_SHIFT)
#define SAR_N_BIT14_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT14_WEIGHT, n)

// REG_89
#define SAR_N_BIT15_WEIGHT_SHIFT            0
#define SAR_N_BIT15_WEIGHT_MASK             (0x3FFF << SAR_N_BIT15_WEIGHT_SHIFT)
#define SAR_N_BIT15_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT15_WEIGHT, n)

// REG_8A
#define SAR_N_BIT16_WEIGHT_SHIFT            0
#define SAR_N_BIT16_WEIGHT_MASK             (0x3FFF << SAR_N_BIT16_WEIGHT_SHIFT)
#define SAR_N_BIT16_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT16_WEIGHT, n)

// REG_8B
#define SAR_N_BIT17_WEIGHT_SHIFT            0
#define SAR_N_BIT17_WEIGHT_MASK             (0x3FFF << SAR_N_BIT17_WEIGHT_SHIFT)
#define SAR_N_BIT17_WEIGHT(n)               BITFIELD_VAL(SAR_N_BIT17_WEIGHT, n)

// REG_8C
#define CLK_SARADC_CODEC_INV                (1 << 14)
#define REG_SAR_ADC_OFFSET_DR               (1 << 13)
#define REG_SAR_OFFSET_DR                   (1 << 12)
#define REG_SAR_WEIGHT_DR                   (1 << 11)
#define REG_SAR_CALI_LSB_SCRN               (1 << 10)
#define REG_SAR_CALI_CNT_SHIFT              6
#define REG_SAR_CALI_CNT_MASK               (0xF << REG_SAR_CALI_CNT_SHIFT)
#define REG_SAR_CALI_CNT(n)                 BITFIELD_VAL(REG_SAR_CALI_CNT, n)
#define REG_SAR_CALI                        (1 << 5)
#define REG_SAR_INIT_CALI_BIT_SHIFT         0
#define REG_SAR_INIT_CALI_BIT_MASK          (0x1F << REG_SAR_INIT_CALI_BIT_SHIFT)
#define REG_SAR_INIT_CALI_BIT(n)            BITFIELD_VAL(REG_SAR_INIT_CALI_BIT, n)

// REG_8D
#define REG_SAR_SAMPLE_IBIT_SHIFT           10
#define REG_SAR_SAMPLE_IBIT_MASK            (0x7 << REG_SAR_SAMPLE_IBIT_SHIFT)
#define REG_SAR_SAMPLE_IBIT(n)              BITFIELD_VAL(REG_SAR_SAMPLE_IBIT, n)
#define REG_SAR_OFFSET_N_SHIFT              5
#define REG_SAR_OFFSET_N_MASK               (0x1F << REG_SAR_OFFSET_N_SHIFT)
#define REG_SAR_OFFSET_N(n)                 BITFIELD_VAL(REG_SAR_OFFSET_N, n)
#define REG_SAR_OFFSET_P_SHIFT              0
#define REG_SAR_OFFSET_P_MASK               (0x1F << REG_SAR_OFFSET_P_SHIFT)
#define REG_SAR_OFFSET_P(n)                 BITFIELD_VAL(REG_SAR_OFFSET_P, n)

// REG_8E
#define REG_SAR_ADC_OFFSET_SHIFT            0
#define REG_SAR_ADC_OFFSET_MASK             (0xFFFF << REG_SAR_ADC_OFFSET_SHIFT)
#define REG_SAR_ADC_OFFSET(n)               BITFIELD_VAL(REG_SAR_ADC_OFFSET, n)

// REG_8F
#define REG_SAR_VCM_CORE_SEL_SHIFT          11
#define REG_SAR_VCM_CORE_SEL_MASK           (0x7 << REG_SAR_VCM_CORE_SEL_SHIFT)
#define REG_SAR_VCM_CORE_SEL(n)             BITFIELD_VAL(REG_SAR_VCM_CORE_SEL, n)
#define REG_SAR_DIFF_EN_IN_REG              (1 << 10)
#define REG_SAR_DIFF_EN_IN_DR               (1 << 9)
#define REG_SAR_SE2DIFF_EN                  (1 << 8)
#define REG_SAR_BUF_EN                      (1 << 7)
#define SAR_CONV_DONE_INV                   (1 << 6)
#define SAR_VOUT_CALIB_INV                  (1 << 5)
#define REG_SAR_CLK_OUT_SEL                 (1 << 4)
#define REG_SAR_OFFSET_CALI_CNT_SHIFT       1
#define REG_SAR_OFFSET_CALI_CNT_MASK        (0x7 << REG_SAR_OFFSET_CALI_CNT_SHIFT)
#define REG_SAR_OFFSET_CALI_CNT(n)          BITFIELD_VAL(REG_SAR_OFFSET_CALI_CNT, n)
#define REG_SAR_OFFSET_CALI                 (1 << 0)

// REG_90
#define REG_SAR_DELAY_CMP                   (1 << 15)
#define REG_SAR_INPUT_BUF_EN                (1 << 14)
#define REG_SAR_EN_PREAMP                   (1 << 13)
#define REG_SAR_DELAY_BIT_SHIFT             11
#define REG_SAR_DELAY_BIT_MASK              (0x3 << REG_SAR_DELAY_BIT_SHIFT)
#define REG_SAR_DELAY_BIT(n)                BITFIELD_VAL(REG_SAR_DELAY_BIT, n)
#define REG_SAR_CLK_MODE                    (1 << 10)
#define REG_SAR_CLK_TRIM_SHIFT              8
#define REG_SAR_CLK_TRIM_MASK               (0x3 << REG_SAR_CLK_TRIM_SHIFT)
#define REG_SAR_CLK_TRIM(n)                 BITFIELD_VAL(REG_SAR_CLK_TRIM, n)
#define REG_SAR_CH_SEL_IN_SHIFT             0
#define REG_SAR_CH_SEL_IN_MASK              (0xFF << REG_SAR_CH_SEL_IN_SHIFT)
#define REG_SAR_CH_SEL_IN(n)                BITFIELD_VAL(REG_SAR_CH_SEL_IN, n)

// REG_91
#define REG_SAR_HIGH_PW_CMP                 (1 << 11)
#define REG_SAR_HIGH_DVDD_EN                (1 << 10)
#define REG_SAR_VREG_OUTCAP_MODE            (1 << 9)
#define REG_SAR_VREG_IBIT_SHIFT             7
#define REG_SAR_VREG_IBIT_MASK              (0x3 << REG_SAR_VREG_IBIT_SHIFT)
#define REG_SAR_VREG_IBIT(n)                BITFIELD_VAL(REG_SAR_VREG_IBIT, n)
#define REG_SAR_THERM_GAIN_IN_SHIFT         5
#define REG_SAR_THERM_GAIN_IN_MASK          (0x3 << REG_SAR_THERM_GAIN_IN_SHIFT)
#define REG_SAR_THERM_GAIN_IN(n)            BITFIELD_VAL(REG_SAR_THERM_GAIN_IN, n)
#define REG_SAR_SE_MODE                     (1 << 4)
#define REG_SAR_PU_NIN_BIAS                 (1 << 3)
#define REG_SAR_PU_NIN_BIAS_DR              (1 << 2)
#define REG_SAR_PU_PIN_BIAS                 (1 << 1)
#define REG_SAR_PU_PIN_BIAS_DR              (1 << 0)

// REG_92
#define REG_SAR_RESET                       (1 << 9)
#define REG_SAR_ADC_RESET                   (1 << 8)
#define REG_SAR_CLK_OUT_DIV_SHIFT           0
#define REG_SAR_CLK_OUT_DIV_MASK            (0xFF << REG_SAR_CLK_OUT_DIV_SHIFT)
#define REG_SAR_CLK_OUT_DIV(n)              BITFIELD_VAL(REG_SAR_CLK_OUT_DIV, n)

// REG_93
#define REG_SAR_PU_START_DLY_CNT_SHIFT      9
#define REG_SAR_PU_START_DLY_CNT_MASK       (0x7F << REG_SAR_PU_START_DLY_CNT_SHIFT)
#define REG_SAR_PU_START_DLY_CNT(n)         BITFIELD_VAL(REG_SAR_PU_START_DLY_CNT, n)
#define REG_SAR_PU_RST_REG                  (1 << 8)
#define REG_SAR_PU_RST_DR                   (1 << 7)
#define REG_SAR_PU_PRECHARGE_REG            (1 << 6)
#define REG_SAR_PU_PRECHARGE_DR             (1 << 5)
#define REG_SAR_PU_REG                      (1 << 4)
#define REG_SAR_PU_DR                       (1 << 3)
#define REG_SAR_PU_VREF_IN                  (1 << 2)
#define REG_SAR_PU_VREF_REG                 (1 << 1)
#define REG_SAR_PU_VREF_DR                  (1 << 0)

// REG_94
#define REG_SAR_PU_RST_START_DLY_CNT_SHIFT  7
#define REG_SAR_PU_RST_START_DLY_CNT_MASK   (0x7F << REG_SAR_PU_RST_START_DLY_CNT_SHIFT)
#define REG_SAR_PU_RST_START_DLY_CNT(n)     BITFIELD_VAL(REG_SAR_PU_RST_START_DLY_CNT, n)
#define REG_SAR_PU_PRECHARGE_START_DLY_CNT_SHIFT 0
#define REG_SAR_PU_PRECHARGE_START_DLY_CNT_MASK (0x7F << REG_SAR_PU_PRECHARGE_START_DLY_CNT_SHIFT)
#define REG_SAR_PU_PRECHARGE_START_DLY_CNT(n) BITFIELD_VAL(REG_SAR_PU_PRECHARGE_START_DLY_CNT, n)

// REG_95
#define REG_SAR_PU_PRECHARGE_LAST_CNT_SHIFT 7
#define REG_SAR_PU_PRECHARGE_LAST_CNT_MASK  (0x1FF << REG_SAR_PU_PRECHARGE_LAST_CNT_SHIFT)
#define REG_SAR_PU_PRECHARGE_LAST_CNT(n)    BITFIELD_VAL(REG_SAR_PU_PRECHARGE_LAST_CNT, n)
#define REG_SAR_PU_RST_LAST_CNT_SHIFT       0
#define REG_SAR_PU_RST_LAST_CNT_MASK        (0x7F << REG_SAR_PU_RST_LAST_CNT_SHIFT)
#define REG_SAR_PU_RST_LAST_CNT(n)          BITFIELD_VAL(REG_SAR_PU_RST_LAST_CNT, n)


// REG_96
#define SAR_P_BIT05_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT05_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT05_WEIGHT_IN_SHIFT)
#define SAR_P_BIT05_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT05_WEIGHT_IN, n)

// REG_97
#define SAR_P_BIT06_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT06_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT06_WEIGHT_IN_SHIFT)
#define SAR_P_BIT06_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT06_WEIGHT_IN, n)

// REG_98
#define SAR_P_BIT07_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT07_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT07_WEIGHT_IN_SHIFT)
#define SAR_P_BIT07_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT07_WEIGHT_IN, n)

// REG_99
#define SAR_P_BIT08_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT08_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT08_WEIGHT_IN_SHIFT)
#define SAR_P_BIT08_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT08_WEIGHT_IN, n)

// REG_9A
#define SAR_P_BIT09_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT09_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT09_WEIGHT_IN_SHIFT)
#define SAR_P_BIT09_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT09_WEIGHT_IN, n)

// REG_9B
#define SAR_P_BIT10_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT10_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT10_WEIGHT_IN_SHIFT)
#define SAR_P_BIT10_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT10_WEIGHT_IN, n)

// REG_9C
#define SAR_P_BIT11_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT11_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT11_WEIGHT_IN_SHIFT)
#define SAR_P_BIT11_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT11_WEIGHT_IN, n)

// REG_9D
#define SAR_P_BIT12_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT12_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT12_WEIGHT_IN_SHIFT)
#define SAR_P_BIT12_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT12_WEIGHT_IN, n)

// REG_9E
#define SAR_P_BIT13_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT13_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT13_WEIGHT_IN_SHIFT)
#define SAR_P_BIT13_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT13_WEIGHT_IN, n)

// REG_9F
#define SAR_P_BIT14_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT14_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT14_WEIGHT_IN_SHIFT)
#define SAR_P_BIT14_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT14_WEIGHT_IN, n)

// REG_A0
#define SAR_P_BIT15_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT15_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT15_WEIGHT_IN_SHIFT)
#define SAR_P_BIT15_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT15_WEIGHT_IN, n)

// REG_A1
#define SAR_P_BIT16_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT16_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT16_WEIGHT_IN_SHIFT)
#define SAR_P_BIT16_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT16_WEIGHT_IN, n)

// REG_A2
#define SAR_P_BIT17_WEIGHT_IN_SHIFT         0
#define SAR_P_BIT17_WEIGHT_IN_MASK          (0x3FFF << SAR_P_BIT17_WEIGHT_IN_SHIFT)
#define SAR_P_BIT17_WEIGHT_IN(n)            BITFIELD_VAL(SAR_P_BIT17_WEIGHT_IN, n)

// REG_A3
#define SAR_N_BIT05_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT05_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT05_WEIGHT_IN_SHIFT)
#define SAR_N_BIT05_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT05_WEIGHT_IN, n)

// REG_A4
#define SAR_N_BIT06_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT06_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT06_WEIGHT_IN_SHIFT)
#define SAR_N_BIT06_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT06_WEIGHT_IN, n)

// REG_A5
#define SAR_N_BIT07_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT07_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT07_WEIGHT_IN_SHIFT)
#define SAR_N_BIT07_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT07_WEIGHT_IN, n)

// REG_A6
#define SAR_N_BIT08_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT08_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT08_WEIGHT_IN_SHIFT)
#define SAR_N_BIT08_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT08_WEIGHT_IN, n)

// REG_A7
#define SAR_N_BIT09_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT09_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT09_WEIGHT_IN_SHIFT)
#define SAR_N_BIT09_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT09_WEIGHT_IN, n)

// REG_A8
#define SAR_N_BIT10_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT10_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT10_WEIGHT_IN_SHIFT)
#define SAR_N_BIT10_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT10_WEIGHT_IN, n)

// REG_A9
#define SAR_N_BIT11_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT11_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT11_WEIGHT_IN_SHIFT)
#define SAR_N_BIT11_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT11_WEIGHT_IN, n)

// REG_AA
#define SAR_N_BIT12_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT12_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT12_WEIGHT_IN_SHIFT)
#define SAR_N_BIT12_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT12_WEIGHT_IN, n)

// REG_AB
#define SAR_N_BIT13_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT13_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT13_WEIGHT_IN_SHIFT)
#define SAR_N_BIT13_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT13_WEIGHT_IN, n)

// REG_AC
#define SAR_N_BIT14_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT14_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT14_WEIGHT_IN_SHIFT)
#define SAR_N_BIT14_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT14_WEIGHT_IN, n)

// REG_AD
#define SAR_N_BIT15_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT15_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT15_WEIGHT_IN_SHIFT)
#define SAR_N_BIT15_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT15_WEIGHT_IN, n)

// REG_AE
#define SAR_N_BIT16_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT16_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT16_WEIGHT_IN_SHIFT)
#define SAR_N_BIT16_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT16_WEIGHT_IN, n)

// REG_AF
#define SAR_N_BIT17_WEIGHT_IN_SHIFT         0
#define SAR_N_BIT17_WEIGHT_IN_MASK          (0x3FFF << SAR_N_BIT17_WEIGHT_IN_SHIFT)
#define SAR_N_BIT17_WEIGHT_IN(n)            BITFIELD_VAL(SAR_N_BIT17_WEIGHT_IN, n)

// REG_B0
#define REG_SAR_OFFSET_N_IN_SHIFT           5
#define REG_SAR_OFFSET_N_IN_MASK            (0x1F << REG_SAR_OFFSET_N_IN_SHIFT)
#define REG_SAR_OFFSET_N_IN(n)              BITFIELD_VAL(REG_SAR_OFFSET_N_IN, n)
#define REG_SAR_OFFSET_P_IN_SHIFT           0
#define REG_SAR_OFFSET_P_IN_MASK            (0x1F << REG_SAR_OFFSET_P_IN_SHIFT)
#define REG_SAR_OFFSET_P_IN(n)              BITFIELD_VAL(REG_SAR_OFFSET_P_IN, n)

// REG_B1
#define REG_SAR_ADC_OFFSET_IN_SHIFT         0
#define REG_SAR_ADC_OFFSET_IN_MASK          (0xFFFF << REG_SAR_ADC_OFFSET_IN_SHIFT)
#define REG_SAR_ADC_OFFSET_IN(n)            BITFIELD_VAL(REG_SAR_ADC_OFFSET_IN, n)

// REG_B2
#define REG_EFUSE_REDUNDANCY_DATA_OUT_SHIFT 2
#define REG_EFUSE_REDUNDANCY_DATA_OUT_MASK  (0xF << REG_EFUSE_REDUNDANCY_DATA_OUT_SHIFT)
#define REG_EFUSE_REDUNDANCY_DATA_OUT(n)    BITFIELD_VAL(REG_EFUSE_REDUNDANCY_DATA_OUT, n)
#define REG_EFUSE_REDUNDANCY_DATA_OUT_DR    (1 << 1)
#define REG_EFUSE_DATA_OUT_DR               (1 << 0)

// REG_B3
#define REG_EFUSE_DATA_OUT_HI_SHIFT         0
#define REG_EFUSE_DATA_OUT_HI_MASK          (0xFFFF << REG_EFUSE_DATA_OUT_HI_SHIFT)
#define REG_EFUSE_DATA_OUT_HI(n)            BITFIELD_VAL(REG_EFUSE_DATA_OUT_HI, n)

// REG_B4
#define REG_EFUSE_DATA_OUT_LO_SHIFT         0
#define REG_EFUSE_DATA_OUT_LO_MASK          (0xFFFF << REG_EFUSE_DATA_OUT_LO_SHIFT)
#define REG_EFUSE_DATA_OUT_LO(n)            BITFIELD_VAL(REG_EFUSE_DATA_OUT_LO, n)

// REG_B5
#define EFUSE_REDUNDANCY_DATA_OUT_SHIFT     0
#define EFUSE_REDUNDANCY_DATA_OUT_MASK      (0xF << EFUSE_REDUNDANCY_DATA_OUT_SHIFT)
#define EFUSE_REDUNDANCY_DATA_OUT(n)        BITFIELD_VAL(EFUSE_REDUNDANCY_DATA_OUT, n)

// REG_B6
#define EFUSE_CHIP_SEL_ENB_DR               (1 << 15)
#define EFUSE_CHIP_SEL_ENB_REG              (1 << 14)
#define EFUSE_STROBE_DR                     (1 << 13)
#define EFUSE_STROBE_REG                    (1 << 12)
#define EFUSE_LOAD_DR                       (1 << 11)
#define EFUSE_LOAD_REG                      (1 << 10)
#define EFUSE_PGM_ENB_DR                    (1 << 9)
#define EFUSE_PGM_ENB_REG                   (1 << 8)
#define EFUSE_PGM_SEL_DR                    (1 << 7)
#define EFUSE_PGM_SEL_REG                   (1 << 6)
#define EFUSE_READ_MODE_DR                  (1 << 5)
#define EFUSE_READ_MODE_REG                 (1 << 4)
#define EFUSE_PWR_DN_ENB_DR                 (1 << 3)
#define EFUSE_PWR_DN_ENB_REG                (1 << 2)
#define EFUSE_REDUNDANCY_EN_DR              (1 << 1)
#define EFUSE_REDUNDANCY_EN_REG             (1 << 0)

// REG_B7
#define REG_EFUSE_ADDRESS_SHIFT             6
#define REG_EFUSE_ADDRESS_MASK              (0x3FF << REG_EFUSE_ADDRESS_SHIFT)
#define REG_EFUSE_ADDRESS(n)                BITFIELD_VAL(REG_EFUSE_ADDRESS, n)
#define REG_EFUSE_STROBE_TRIGGER            (1 << 5)
#define REG_EFUSE_TURN_ON                   (1 << 4)
#define REG_EFUSE_CLK_EN                    (1 << 3)
#define REG_EFUSE_READ_MODE                 (1 << 2)
#define REG_EFUSE_PGM_MODE                  (1 << 1)
#define REG_EFUSE_PGM_READ_SEL              (1 << 0)

// REG_B8
#define REG_EFUSE_TIME_CSB_ADDR_VALUE_SHIFT 10
#define REG_EFUSE_TIME_CSB_ADDR_VALUE_MASK  (0xF << REG_EFUSE_TIME_CSB_ADDR_VALUE_SHIFT)
#define REG_EFUSE_TIME_CSB_ADDR_VALUE(n)    BITFIELD_VAL(REG_EFUSE_TIME_CSB_ADDR_VALUE, n)
#define REG_EFUSE_TIME_PS_CSB_VALUE_SHIFT   6
#define REG_EFUSE_TIME_PS_CSB_VALUE_MASK    (0xF << REG_EFUSE_TIME_PS_CSB_VALUE_SHIFT)
#define REG_EFUSE_TIME_PS_CSB_VALUE(n)      BITFIELD_VAL(REG_EFUSE_TIME_PS_CSB_VALUE, n)
#define REG_EFUSE_TIME_PD_PS_VALUE_SHIFT    0
#define REG_EFUSE_TIME_PD_PS_VALUE_MASK     (0x3F << REG_EFUSE_TIME_PD_PS_VALUE_SHIFT)
#define REG_EFUSE_TIME_PD_PS_VALUE(n)       BITFIELD_VAL(REG_EFUSE_TIME_PD_PS_VALUE, n)

// REG_B9
#define REG_EFUSE_TIME_PGM_STROBING_VALUE_SHIFT 4
#define REG_EFUSE_TIME_PGM_STROBING_VALUE_MASK (0x1FF << REG_EFUSE_TIME_PGM_STROBING_VALUE_SHIFT)
#define REG_EFUSE_TIME_PGM_STROBING_VALUE(n) BITFIELD_VAL(REG_EFUSE_TIME_PGM_STROBING_VALUE, n)
#define REG_EFUSE_TIME_ADDR_STROBE_VALUE_SHIFT 0
#define REG_EFUSE_TIME_ADDR_STROBE_VALUE_MASK (0xF << REG_EFUSE_TIME_ADDR_STROBE_VALUE_SHIFT)
#define REG_EFUSE_TIME_ADDR_STROBE_VALUE(n) BITFIELD_VAL(REG_EFUSE_TIME_ADDR_STROBE_VALUE, n)

// REG_BA
#define REG_EFUSE_TIME_READ_STROBING_VALUE_SHIFT 0
#define REG_EFUSE_TIME_READ_STROBING_VALUE_MASK (0x1FF << REG_EFUSE_TIME_READ_STROBING_VALUE_SHIFT)
#define REG_EFUSE_TIME_READ_STROBING_VALUE(n) BITFIELD_VAL(REG_EFUSE_TIME_READ_STROBING_VALUE, n)

// REG_BB
#define REG_EFUSE_TIME_PS_PD_VALUE_SHIFT    10
#define REG_EFUSE_TIME_PS_PD_VALUE_MASK     (0x3F << REG_EFUSE_TIME_PS_PD_VALUE_SHIFT)
#define REG_EFUSE_TIME_PS_PD_VALUE(n)       BITFIELD_VAL(REG_EFUSE_TIME_PS_PD_VALUE, n)
#define REG_EFUSE_TIME_CSB_PS_VALUE_SHIFT   6
#define REG_EFUSE_TIME_CSB_PS_VALUE_MASK    (0xF << REG_EFUSE_TIME_CSB_PS_VALUE_SHIFT)
#define REG_EFUSE_TIME_CSB_PS_VALUE(n)      BITFIELD_VAL(REG_EFUSE_TIME_CSB_PS_VALUE, n)
#define REG_EFUSE_TIME_STROBE_CSB_VALUE_SHIFT 0
#define REG_EFUSE_TIME_STROBE_CSB_VALUE_MASK (0x3F << REG_EFUSE_TIME_STROBE_CSB_VALUE_SHIFT)
#define REG_EFUSE_TIME_STROBE_CSB_VALUE(n)  BITFIELD_VAL(REG_EFUSE_TIME_STROBE_CSB_VALUE, n)

// REG_BC
#define EFUSE_REDUNDANCY_INFO_ROW_SEL_DR    (1 << 7)
#define EFUSE_REDUNDANCY_INFO_ROW_SEL_REG   (1 << 6)
#define REG_EFUSE_TIME_PD_OFF_VALUE_SHIFT   0
#define REG_EFUSE_TIME_PD_OFF_VALUE_MASK    (0x3F << REG_EFUSE_TIME_PD_OFF_VALUE_SHIFT)
#define REG_EFUSE_TIME_PD_OFF_VALUE(n)      BITFIELD_VAL(REG_EFUSE_TIME_PD_OFF_VALUE, n)

// REG_BD
#define EFUSE_DATA_OUT_HI_SHIFT             0
#define EFUSE_DATA_OUT_HI_MASK              (0xFFFF << EFUSE_DATA_OUT_HI_SHIFT)
#define EFUSE_DATA_OUT_HI(n)                BITFIELD_VAL(EFUSE_DATA_OUT_HI, n)

// REG_BE
#define EFUSE_DATA_OUT_LO_SHIFT             0
#define EFUSE_DATA_OUT_LO_MASK              (0xFFFF << EFUSE_DATA_OUT_LO_SHIFT)
#define EFUSE_DATA_OUT_LO(n)                BITFIELD_VAL(EFUSE_DATA_OUT_LO, n)


// REG_BF
#define REG_BUCK_VHPPA_INTERNAL_FREQUENCY_SHIFT 10
#define REG_BUCK_VHPPA_INTERNAL_FREQUENCY_MASK (0x7 << REG_BUCK_VHPPA_INTERNAL_FREQUENCY_SHIFT)
#define REG_BUCK_VHPPA_INTERNAL_FREQUENCY(n) BITFIELD_VAL(REG_BUCK_VHPPA_INTERNAL_FREQUENCY, n)
#define REG_BUCK_VHPPA_PULLDOWN_EN          (1 << 9)
#define REG_BUCK_VHPPA_ANTI_RES_DISABLE     (1 << 8)
#define REG_BUCK_VHPPA_BURST_THRESHOLD_SHIFT 3
#define REG_BUCK_VHPPA_BURST_THRESHOLD_MASK (0x1F << REG_BUCK_VHPPA_BURST_THRESHOLD_SHIFT)
#define REG_BUCK_VHPPA_BURST_THRESHOLD(n)   BITFIELD_VAL(REG_BUCK_VHPPA_BURST_THRESHOLD, n)
#define REG_BUCK_VHPPA_SYNC_DISABLE         (1 << 2)
#define REG_BUCK_VHPPA_SOFT_START_EN        (1 << 1)
#define REG_BUCK_VHPPA_SLOPE_DOUBLE         (1 << 0)

// REG_C0
#define REG_BUCK_VHPPA_DIV_RFB_RES_X2       (1 << 12)
#define REG_BUCK_VHPPA_KICK_PD_SLEEP_SEL    (1 << 11)
#define REG_BUCK_VHPPA_TEST_POWERMOS        (1 << 10)
#define REG_BUCK_VHPPA_EN_ZCD_CAL           (1 << 9)
#define REG_BUCK_VHPPA_TEST_PMOS            (1 << 8)
#define REG_BUCK_VHPPA_HALF_BIAS            (1 << 7)
#define REG_BUCK_VHPPA_SLOPE_EN_BURST       (1 << 6)
#define REG_BUCK_VHPPA_OFFSET_CURRENT_EN    (1 << 5)
#define REG_BUCK_VHPPA_I_DELTAV_X2          (1 << 4)
#define REG_BUCK_VHPPA_LOGIC_IR_CLK_EN      (1 << 3)
#define REG_BUCK_VHPPA_ENB_PU_DELAY         (1 << 2)
#define REG_BUCK_VHPPA_ICOMP_DC_HIGH        (1 << 1)
#define REG_BUCK_VHPPA_LP_VCOMP             (1 << 0)

// REG_C1
#define REG_BUCK_VHPPA_ZCD_CAP_BIT_SHIFT    12
#define REG_BUCK_VHPPA_ZCD_CAP_BIT_MASK     (0xF << REG_BUCK_VHPPA_ZCD_CAP_BIT_SHIFT)
#define REG_BUCK_VHPPA_ZCD_CAP_BIT(n)       BITFIELD_VAL(REG_BUCK_VHPPA_ZCD_CAP_BIT, n)
#define REG_BUCK_VHPPA_CAP_BIT_SHIFT        8
#define REG_BUCK_VHPPA_CAP_BIT_MASK         (0xF << REG_BUCK_VHPPA_CAP_BIT_SHIFT)
#define REG_BUCK_VHPPA_CAP_BIT(n)           BITFIELD_VAL(REG_BUCK_VHPPA_CAP_BIT, n)
#define REG_BUCK_VHPPA_DT_BIT               (1 << 6)
#define REG_BUCK_VHPPA_CAL_DELTAV_SHIFT     0
#define REG_BUCK_VHPPA_CAL_DELTAV_MASK      (0x3F << REG_BUCK_VHPPA_CAL_DELTAV_SHIFT)
#define REG_BUCK_VHPPA_CAL_DELTAV(n)        BITFIELD_VAL(REG_BUCK_VHPPA_CAL_DELTAV, n)

// REG_C2
#define REG_BUCK_VHPPA_EDGE_CON_SHIFT       12
#define REG_BUCK_VHPPA_EDGE_CON_MASK        (0xF << REG_BUCK_VHPPA_EDGE_CON_SHIFT)
#define REG_BUCK_VHPPA_EDGE_CON(n)          BITFIELD_VAL(REG_BUCK_VHPPA_EDGE_CON, n)
#define REG_BUCK_VHPPA_OFFSET_BIT_SHIFT     7
#define REG_BUCK_VHPPA_OFFSET_BIT_MASK      (0x1F << REG_BUCK_VHPPA_OFFSET_BIT_SHIFT)
#define REG_BUCK_VHPPA_OFFSET_BIT(n)        BITFIELD_VAL(REG_BUCK_VHPPA_OFFSET_BIT, n)
#define REG_BUCK_VHPPA_REFRESH_BYPASS_B     (1 << 4)
#define REG_BUCK_VHPPA_COUNTER_SEL_SHIFT    2
#define REG_BUCK_VHPPA_COUNTER_SEL_MASK     (0x3 << REG_BUCK_VHPPA_COUNTER_SEL_SHIFT)
#define REG_BUCK_VHPPA_COUNTER_SEL(n)       BITFIELD_VAL(REG_BUCK_VHPPA_COUNTER_SEL, n)

// REG_C4
#define REG_BUCK_VANA_ICOMP_DC_HIGH         (1 << 15)
#define REG_BUCK_VANA_CAP_BIT_SHIFT         11
#define REG_BUCK_VANA_CAP_BIT_MASK          (0xF << REG_BUCK_VANA_CAP_BIT_SHIFT)
#define REG_BUCK_VANA_CAP_BIT(n)            BITFIELD_VAL(REG_BUCK_VANA_CAP_BIT, n)
#define REG_BUCK_VANA_CAL_DELTAV_SHIFT      5
#define REG_BUCK_VANA_CAL_DELTAV_MASK       (0x3F << REG_BUCK_VANA_CAL_DELTAV_SHIFT)
#define REG_BUCK_VANA_CAL_DELTAV(n)         BITFIELD_VAL(REG_BUCK_VANA_CAL_DELTAV, n)
#define REG_BUCK_VANA_BURST_THRESHOLD_SHIFT 0
#define REG_BUCK_VANA_BURST_THRESHOLD_MASK  (0x1F << REG_BUCK_VANA_BURST_THRESHOLD_SHIFT)
#define REG_BUCK_VANA_BURST_THRESHOLD(n)    BITFIELD_VAL(REG_BUCK_VANA_BURST_THRESHOLD, n)

// REG_C5
#define REG_BUCK_VANA_REFRESH_BYPASS_B      (1 << 13)
#define REG_BUCK_VANA_COUNTER_SEL_SHIFT     11
#define REG_BUCK_VANA_COUNTER_SEL_MASK      (0x3 << REG_BUCK_VANA_COUNTER_SEL_SHIFT)
#define REG_BUCK_VANA_COUNTER_SEL(n)        BITFIELD_VAL(REG_BUCK_VANA_COUNTER_SEL, n)
#define REG_BUCK_VANA_TEST_MODE_EN          (1 << 10)
#define REG_BUCK_VANA_RESERVED_SHIFT        6
#define REG_BUCK_VANA_RESERVED_MASK         (0xF << REG_BUCK_VANA_RESERVED_SHIFT)
#define REG_BUCK_VANA_RESERVED(n)           BITFIELD_VAL(REG_BUCK_VANA_RESERVED, n)
#define REG_BUCK_VANA_TEST_POWERMOS         (1 << 5)
#define REG_BUCK_VANA_TEST_PMOS             (1 << 4)
#define REG_BUCK_VANA_SLOPE_EN_BURST        (1 << 3)
#define REG_BUCK_VANA_OFFSET_CURRENT_EN     (1 << 2)
#define REG_BUCK_VANA_LOGIC_IR_CLK_EN       (1 << 1)
#define REG_BUCK_VANA_ENB_PU_DELAY          (1 << 0)

// REG_C6
#define REG_BUCK_VHPPA_TEST_MODE_EN         (1 << 5)
#define REG_BUCK_VANA_OFFSET_BIT_SHIFT      0
#define REG_BUCK_VANA_OFFSET_BIT_MASK       (0x1F << REG_BUCK_VANA_OFFSET_BIT_SHIFT)
#define REG_BUCK_VANA_OFFSET_BIT(n)         BITFIELD_VAL(REG_BUCK_VANA_OFFSET_BIT, n)

// REG_C7
#define REG_BUCK_VCORE_CAP_BIT_SHIFT        11
#define REG_BUCK_VCORE_CAP_BIT_MASK         (0xF << REG_BUCK_VCORE_CAP_BIT_SHIFT)
#define REG_BUCK_VCORE_CAP_BIT(n)           BITFIELD_VAL(REG_BUCK_VCORE_CAP_BIT, n)
#define REG_BUCK_VCORE_CAL_DELTAV_SHIFT     5
#define REG_BUCK_VCORE_CAL_DELTAV_MASK      (0x3F << REG_BUCK_VCORE_CAL_DELTAV_SHIFT)
#define REG_BUCK_VCORE_CAL_DELTAV(n)        BITFIELD_VAL(REG_BUCK_VCORE_CAL_DELTAV, n)
#define REG_BUCK_VCORE_BURST_THRESHOLD_SHIFT 0
#define REG_BUCK_VCORE_BURST_THRESHOLD_MASK (0x1F << REG_BUCK_VCORE_BURST_THRESHOLD_SHIFT)
#define REG_BUCK_VCORE_BURST_THRESHOLD(n)   BITFIELD_VAL(REG_BUCK_VCORE_BURST_THRESHOLD, n)

// REG_C8
#define REG_BUCK_VCORE_RESERVED_SHIFT       12
#define REG_BUCK_VCORE_RESERVED_MASK        (0xF << REG_BUCK_VCORE_RESERVED_SHIFT)
#define REG_BUCK_VCORE_RESERVED(n)          BITFIELD_VAL(REG_BUCK_VCORE_RESERVED, n)
#define REG_BUCK_VCORE_REFRESH_BYPASS_B     (1 << 11)
#define REG_BUCK_VCORE_COUNTER_SEL_SHIFT    9
#define REG_BUCK_VCORE_COUNTER_SEL_MASK     (0x3 << REG_BUCK_VCORE_COUNTER_SEL_SHIFT)
#define REG_BUCK_VCORE_COUNTER_SEL(n)       BITFIELD_VAL(REG_BUCK_VCORE_COUNTER_SEL, n)
#define REG_BUCK_VCORE_TEST_MODE_EN         (1 << 8)
#define REG_BUCK_VCORE_TEST_POWERMOS        (1 << 5)
#define REG_BUCK_VCORE_TEST_PMOS            (1 << 4)
#define REG_BUCK_VCORE_SLOPE_EN_BURST       (1 << 3)
#define REG_BUCK_VCORE_OFFSET_CURRENT_EN    (1 << 2)
#define REG_BUCK_VCORE_LOGIC_IR_CLK_EN      (1 << 1)
#define REG_BUCK_VCORE_ENB_PU_DELAY         (1 << 0)

// REG_C9
#define REG_BUCK_VCORE_OFFSET_BIT_SHIFT     0
#define REG_BUCK_VCORE_OFFSET_BIT_MASK      (0x1F << REG_BUCK_VCORE_OFFSET_BIT_SHIFT)
#define REG_BUCK_VCORE_OFFSET_BIT(n)        BITFIELD_VAL(REG_BUCK_VCORE_OFFSET_BIT, n)

// REG_CB
#define REG_ISO_ANA2CORE                    (1 << 13)
#define ISO_ANA2CORE_DR                     (1 << 12)
#define PU_INTERFACE_ANA                    (1 << 11)
#define PU_INTERFACE_RF                     (1 << 10)
#define PU_INTERFACE_RESERVED1              (1 << 9)
#define PU_INTERFACE_RESERVED2              (1 << 8)
#define REG_ISO_INTERFACE_ANA               (1 << 7)
#define REG_ISO_INTERFACE_RF                (1 << 6)
#define REG_ISO_INTERFACE_CHRG              (1 << 5)
#define REG_ISO_INTERFACE_USB               (1 << 4)
#define ISO_INTERFACE_ANA_DR                (1 << 3)
#define ISO_INTERFACE_RF_DR                 (1 << 2)
#define ISO_INTERFACE_CHRG_DR               (1 << 1)
#define ISO_INTERFACE_USB_DR                (1 << 0)

// REG_CC
#define REG_PU_MCU_LOGIC_DR                 (1 << 0)

// REG_CD
#define REG_PU_MCU_RETMEM_CE_DR             (1 << 15)
#define REG_PU_MCU_RETMEM_PE_DR             (1 << 14)
#define REG_PU_MCU_MEM_DR                   (1 << 13)
#define REG_PU_BT_LOGIC_DR                  (1 << 12)
#define REG_PU_BT_RETMEM_CE_DR              (1 << 11)
#define REG_PU_BT_MEM_DR                    (1 << 10)
#define REG_PU_CODEC_LOGIC_DR               (1 << 9)
#define REG_PU_CODEC_MEM_DR                 (1 << 8)
#define REG_PU_DISPLAY_LOGIC_DR             (1 << 7)
#define REG_PU_DISPLAY_MEM_DR               (1 << 6)
#define REG_PU_SHARE_MEM4_PE_DR             (1 << 5)
#define REG_PU_SHARE_MEM4_CE_DR             (1 << 4)
#define REG_PU_SHARE_MEM5_PE_DR             (1 << 3)
#define REG_PU_SHARE_MEM5_CE_DR             (1 << 2)
#define REG_PU_SHARE_MEM6_PE_DR             (1 << 1)
#define REG_PU_SHARE_MEM6_CE_DR             (1 << 0)

// REG_CE
#define REG_PU_MCU_LOGIC                    (1 << 0)

// REG_CF
#define REG_PU_MCU_RETMEM_CE                (1 << 15)
#define REG_PU_MCU_RETMEM_PE                (1 << 14)
#define REG_PU_MCU_MEM                      (1 << 13)
#define REG_PU_BT_LOGIC                     (1 << 12)
#define REG_PU_BT_RETMEM_CE                 (1 << 11)
#define REG_PU_BT_MEM                       (1 << 10)
#define REG_PU_CODEC_LOGIC                  (1 << 9)
#define REG_PU_CODEC_MEM                    (1 << 8)
#define REG_PU_DISPLAY_LOGIC                (1 << 7)
#define REG_PU_DISPLAY_MEM                  (1 << 6)
#define REG_PU_SHARE_MEM4_PE                (1 << 5)
#define REG_PU_SHARE_MEM4_CE                (1 << 4)
#define REG_PU_SHARE_MEM5_PE                (1 << 3)
#define REG_PU_SHARE_MEM5_CE                (1 << 2)
#define REG_PU_SHARE_MEM6_PE                (1 << 1)
#define REG_PU_SHARE_MEM6_CE                (1 << 0)

// REG_D0
#define REG_RC_SLEEP_ALLOW                  (1 << 7)
#define REG_RC_MERGE_MODE                   (1 << 6)
#define REG_RC_SLEEP_MODE_DR                (1 << 5)
#define REG_RC_SLEEP                        (1 << 4)

// REG_D1
#define REG_VBAT_SUB3V_ISEL_SHIFT           6
#define REG_VBAT_SUB3V_ISEL_MASK            (0x3 << REG_VBAT_SUB3V_ISEL_SHIFT)
#define REG_VBAT_SUB3V_ISEL(n)              BITFIELD_VAL(REG_VBAT_SUB3V_ISEL, n)
#define REG_LPO_ISEL_COMP_SHIFT             2
#define REG_LPO_ISEL_COMP_MASK              (0xF << REG_LPO_ISEL_COMP_SHIFT)
#define REG_LPO_ISEL_COMP(n)                BITFIELD_VAL(REG_LPO_ISEL_COMP, n)
#define REG_LPO_ISEL_LDO_SHIFT              0
#define REG_LPO_ISEL_LDO_MASK               (0x3 << REG_LPO_ISEL_LDO_SHIFT)
#define REG_LPO_ISEL_LDO(n)                 BITFIELD_VAL(REG_LPO_ISEL_LDO, n)

// REG_D2
#define REG_LPO_ISEL_OSC_SHIFT              12
#define REG_LPO_ISEL_OSC_MASK               (0x7 << REG_LPO_ISEL_OSC_SHIFT)
#define REG_LPO_ISEL_OSC(n)                 BITFIELD_VAL(REG_LPO_ISEL_OSC, n)
#define REG_LPO_CAP_BIT_SHIFT               6
#define REG_LPO_CAP_BIT_MASK                (0x3F << REG_LPO_CAP_BIT_SHIFT)
#define REG_LPO_CAP_BIT(n)                  BITFIELD_VAL(REG_LPO_CAP_BIT, n)
#define REG_LPO_RES_BIT_SHIFT               4
#define REG_LPO_RES_BIT_MASK                (0x3 << REG_LPO_RES_BIT_SHIFT)
#define REG_LPO_RES_BIT(n)                  BITFIELD_VAL(REG_LPO_RES_BIT, n)
#define REG_BG_ISEL_SHIFT                   2
#define REG_BG_ISEL_MASK                    (0x3 << REG_BG_ISEL_SHIFT)
#define REG_BG_ISEL(n)                      BITFIELD_VAL(REG_BG_ISEL, n)
#define REG_VRTC_OVERRIDE                   (1 << 1)
#define REG_RTC_LDO_TRAN_ENHANCE            (1 << 0)

// REG_D4
#define REG_BUCK_VCORE_EDGE_CON_RX_SHIFT    8
#define REG_BUCK_VCORE_EDGE_CON_RX_MASK     (0xF << REG_BUCK_VCORE_EDGE_CON_RX_SHIFT)
#define REG_BUCK_VCORE_EDGE_CON_RX(n)       BITFIELD_VAL(REG_BUCK_VCORE_EDGE_CON_RX, n)
#define REG_BUCK_VANA_EDGE_CON_RX_SHIFT     4
#define REG_BUCK_VANA_EDGE_CON_RX_MASK      (0xF << REG_BUCK_VANA_EDGE_CON_RX_SHIFT)
#define REG_BUCK_VANA_EDGE_CON_RX(n)        BITFIELD_VAL(REG_BUCK_VANA_EDGE_CON_RX, n)
#define REG_BUCK_VHPPA_EDGE_CON_RX_SHIFT    0
#define REG_BUCK_VHPPA_EDGE_CON_RX_MASK     (0xF << REG_BUCK_VHPPA_EDGE_CON_RX_SHIFT)
#define REG_BUCK_VHPPA_EDGE_CON_RX(n)       BITFIELD_VAL(REG_BUCK_VHPPA_EDGE_CON_RX, n)

// REG_D7
#define REG_LDO_VCORE_L_VOUT_HV             (1 << 6)
#define REG_LDO_VCORE_L_BYPASS              (1 << 5)

// REG_D8
#define REG_XO_32K_ISEL_VALUE_SHIFT         8
#define REG_XO_32K_ISEL_VALUE_MASK          (0x1F << REG_XO_32K_ISEL_VALUE_SHIFT)
#define REG_XO_32K_ISEL_VALUE(n)            BITFIELD_VAL(REG_XO_32K_ISEL_VALUE, n)
#define REG_XO_32K_ISEL_DR                  (1 << 7)
#define REG_XO_32K_PU_LDO_DR                (1 << 6)
#define REG_XO_32K_PU_LDO                   (1 << 5)
#define REG_XO_32K_LDO_PRECHARGE_VALUE_SHIFT 2
#define REG_XO_32K_LDO_PRECHARGE_VALUE_MASK (0x7 << REG_XO_32K_LDO_PRECHARGE_VALUE_SHIFT)
#define REG_XO_32K_LDO_PRECHARGE_VALUE(n)   BITFIELD_VAL(REG_XO_32K_LDO_PRECHARGE_VALUE, n)
#define REG_XO_32K_LDO_PRECHARGE_DR         (1 << 1)
#define REG_XO_32K_LDO_PRECHARGE            (1 << 0)

// REG_D9
#define REG_XO_32K_ISEL_RAMP_START_DLY_SHIFT 5
#define REG_XO_32K_ISEL_RAMP_START_DLY_MASK (0x1FF << REG_XO_32K_ISEL_RAMP_START_DLY_SHIFT)
#define REG_XO_32K_ISEL_RAMP_START_DLY(n)   BITFIELD_VAL(REG_XO_32K_ISEL_RAMP_START_DLY, n)
#define REG_XO_32K_ISEL_RAMP_T_STEP_SHIFT   0
#define REG_XO_32K_ISEL_RAMP_T_STEP_MASK    (0x1F << REG_XO_32K_ISEL_RAMP_T_STEP_SHIFT)
#define REG_XO_32K_ISEL_RAMP_T_STEP(n)      BITFIELD_VAL(REG_XO_32K_ISEL_RAMP_T_STEP, n)

// REG_DA
#define REG_CLK_32K_EXT1_EN                 (1 << 12)
#define REG_CLK_32K_EXT2_EN                 (1 << 11)
#define REG_CLK_32K_EXT3_EN                 (1 << 10)
#define REG_PU_VSYS_DIV                     (1 << 9)
#define REG_XO_32K_CAPBIT_SHIFT             0
#define REG_XO_32K_CAPBIT_MASK              (0x1FF << REG_XO_32K_CAPBIT_SHIFT)
#define REG_XO_32K_CAPBIT(n)                BITFIELD_VAL(REG_XO_32K_CAPBIT, n)

// REG_DD
#define REG_PULL_DOWN_VSENSOR               (1 << 15)

// REG_DF
#define REG_GUARD_MODE_SEL                  (1 << 15)
#define POWER_DOWN_INTR_MERGED_MSK          (1 << 14)
#define POWER_DOWN_INTR_MSK                 (1 << 13)
#define HARD_POWER_OFF_INTR_EN              (1 << 12)

#define REG_GUARD_RELEASE_SHIFT             4
#define REG_GUARD_RELEASE_MASK              (0xF << REG_GUARD_RELEASE_SHIFT)
#define REG_GUARD_RELEASE(n)                BITFIELD_VAL(REG_GUARD_RELEASE, n)
#define REG_GUARD_HIGH_SHIFT                0
#define REG_GUARD_HIGH_MASK                 (0xF << REG_GUARD_HIGH_SHIFT)
#define REG_GUARD_HIGH(n)                   BITFIELD_VAL(REG_GUARD_HIGH, n)

// REG_E0
#define REG_OSC_NPULSE_EXT_EN               (1 << 14)
#define REG_OSC_GUARD_TIME_SHIFT            8
#define REG_OSC_GUARD_TIME_MASK             (0x3F << REG_OSC_GUARD_TIME_SHIFT)
#define REG_OSC_GUARD_TIME(n)               BITFIELD_VAL(REG_OSC_GUARD_TIME, n)
#define REG_POWER_DOWN_DB_VALUE_SHIFT       0
#define REG_POWER_DOWN_DB_VALUE_MASK        (0xFF << REG_POWER_DOWN_DB_VALUE_SHIFT)
#define REG_POWER_DOWN_DB_VALUE(n)          BITFIELD_VAL(REG_POWER_DOWN_DB_VALUE, n)

// REG_E1
#define REG_BIASB_LPFSEL_DLY_SHIFT          5
#define REG_BIASB_LPFSEL_DLY_MASK           (0x1F << REG_BIASB_LPFSEL_DLY_SHIFT)
#define REG_BIASB_LPFSEL_DLY(n)             BITFIELD_VAL(REG_BIASB_LPFSEL_DLY, n)
#define REG_BIASA_LPFSEL_DLY_SHIFT          0
#define REG_BIASA_LPFSEL_DLY_MASK           (0x1F << REG_BIASA_LPFSEL_DLY_SHIFT)
#define REG_BIASA_LPFSEL_DLY(n)             BITFIELD_VAL(REG_BIASA_LPFSEL_DLY, n)

// REG_E2
#define REG_RC_ASENSOR_EN                   (1 << 14)
#define REG_MIC_BIASB_EN_DR                 (1 << 13)
#define REG_MIC_LDO_EN_DR                   (1 << 12)
#define REG_MIC_BIASB_LPFSEL_SHIFT          9
#define REG_MIC_BIASB_LPFSEL_MASK           (0x7 << REG_MIC_BIASB_LPFSEL_SHIFT)
#define REG_MIC_BIASB_LPFSEL(n)             BITFIELD_VAL(REG_MIC_BIASB_LPFSEL, n)
#define REG_MIC_BIASB_LPFSEL_CHANGE_SHIFT   6
#define REG_MIC_BIASB_LPFSEL_CHANGE_MASK    (0x7 << REG_MIC_BIASB_LPFSEL_CHANGE_SHIFT)
#define REG_MIC_BIASB_LPFSEL_CHANGE(n)      BITFIELD_VAL(REG_MIC_BIASB_LPFSEL_CHANGE, n)
#define REG_MIC_BIASA_LPFSEL_SHIFT          3
#define REG_MIC_BIASA_LPFSEL_MASK           (0x7 << REG_MIC_BIASA_LPFSEL_SHIFT)
#define REG_MIC_BIASA_LPFSEL(n)             BITFIELD_VAL(REG_MIC_BIASA_LPFSEL, n)
#define REG_MIC_BIASA_LPFSEL_CHANGE_SHIFT   0
#define REG_MIC_BIASA_LPFSEL_CHANGE_MASK    (0x7 << REG_MIC_BIASA_LPFSEL_CHANGE_SHIFT)
#define REG_MIC_BIASA_LPFSEL_CHANGE(n)      BITFIELD_VAL(REG_MIC_BIASA_LPFSEL_CHANGE, n)

// REG_E3
#define REG_REFRESH_MODE_EN                 (1 << 15)
#define REG_BG_EN_REFRESH_DR                (1 << 13)
#define REG_BG_EN_REFRESH                   (1 << 12)
#define REG_REFRESH_CYCLE_CNT_SHIFT         0
#define REG_REFRESH_CYCLE_CNT_MASK          (0xFFF << REG_REFRESH_CYCLE_CNT_SHIFT)
#define REG_REFRESH_CYCLE_CNT(n)            BITFIELD_VAL(REG_REFRESH_CYCLE_CNT, n)

// REG_E4
#define REG_VBG_SEL_DLY_TIME_SHIFT          12
#define REG_VBG_SEL_DLY_TIME_MASK           (0xF << REG_VBG_SEL_DLY_TIME_SHIFT)
#define REG_VBG_SEL_DLY_TIME(n)             BITFIELD_VAL(REG_VBG_SEL_DLY_TIME, n)
#define REG_REFRESH_OFF_DLY_CNT_SHIFT       8
#define REG_REFRESH_OFF_DLY_CNT_MASK        (0x3 << REG_REFRESH_OFF_DLY_CNT_SHIFT)
#define REG_REFRESH_OFF_DLY_CNT(n)          BITFIELD_VAL(REG_REFRESH_OFF_DLY_CNT, n)
#define REG_REFRESH_ON_DLY_CNT_SHIFT        5
#define REG_REFRESH_ON_DLY_CNT_MASK         (0x7 << REG_REFRESH_ON_DLY_CNT_SHIFT)
#define REG_REFRESH_ON_DLY_CNT(n)           BITFIELD_VAL(REG_REFRESH_ON_DLY_CNT, n)
#define REG_REFRESH_ON_CNT_SHIFT            0
#define REG_REFRESH_ON_CNT_MASK             (0x1F << REG_REFRESH_ON_CNT_SHIFT)
#define REG_REFRESH_ON_CNT(n)               BITFIELD_VAL(REG_REFRESH_ON_CNT, n)

// REG_E6
#define REG_SAR_VREG_SEL_SHIFT              8
#define REG_SAR_VREG_SEL_MASK               (0x7F << REG_SAR_VREG_SEL_SHIFT)
#define REG_SAR_VREG_SEL(n)                 BITFIELD_VAL(REG_SAR_VREG_SEL, n)
#define REG_SAR_EN_VREF_CALIB               (1 << 6)
#define REG_SAR_PD_VREF_ADC_SW              (1 << 5)

// REG_E7
#define REG_SAR_OFFSET_BIT_VREF_SHIFT       10
#define REG_SAR_OFFSET_BIT_VREF_MASK        (0x3F << REG_SAR_OFFSET_BIT_VREF_SHIFT)
#define REG_SAR_OFFSET_BIT_VREF(n)          BITFIELD_VAL(REG_SAR_OFFSET_BIT_VREF, n)
#define REG_CLK_32K_BUCK_VANA_EN            (1 << 8)
#define REG_BUCK_VANA_VBAT_3V_EXTRA         (1 << 7)
#define REG_BUCK_VANA_ULP_FRESH             (1 << 6)
#define REG_BUCK_VANA_I_HALVE               (1 << 5)
#define REG_BUCK_VANA_REFRESH_FREQ_SEL_SHIFT 2
#define REG_BUCK_VANA_REFRESH_FREQ_SEL_MASK (0x7 << REG_BUCK_VANA_REFRESH_FREQ_SEL_SHIFT)
#define REG_BUCK_VANA_REFRESH_FREQ_SEL(n)   BITFIELD_VAL(REG_BUCK_VANA_REFRESH_FREQ_SEL, n)
#define REG_BUCK_VANA_CLK_LPO_PULSE_SHIFT   0
#define REG_BUCK_VANA_CLK_LPO_PULSE_MASK    (0x3 << REG_BUCK_VANA_CLK_LPO_PULSE_SHIFT)
#define REG_BUCK_VANA_CLK_LPO_PULSE(n)      BITFIELD_VAL(REG_BUCK_VANA_CLK_LPO_PULSE, n)

// REG_E8
#define REG_BUCK_VHPPA_RESERVED_SHIFT       9
#define REG_BUCK_VHPPA_RESERVED_MASK        (0xF << REG_BUCK_VHPPA_RESERVED_SHIFT)
#define REG_BUCK_VHPPA_RESERVED(n)          BITFIELD_VAL(REG_BUCK_VHPPA_RESERVED, n)
#define REG_CLK_32K_BUCK_VHPPA_EN           (1 << 8)
#define REG_BUCK_VHPPA_VBAT_3V_EXTRA        (1 << 7)
#define REG_BUCK_VHPPA_ULP_FRESH            (1 << 6)
#define REG_BUCK_VHPPA_I_HALVE              (1 << 5)
#define REG_BUCK_VHPPA_REFRESH_FREQ_SEL_SHIFT 2
#define REG_BUCK_VHPPA_REFRESH_FREQ_SEL_MASK (0x7 << REG_BUCK_VHPPA_REFRESH_FREQ_SEL_SHIFT)
#define REG_BUCK_VHPPA_REFRESH_FREQ_SEL(n)  BITFIELD_VAL(REG_BUCK_VHPPA_REFRESH_FREQ_SEL, n)
#define REG_BUCK_VHPPA_CLK_LPO_PULSE_SHIFT  0
#define REG_BUCK_VHPPA_CLK_LPO_PULSE_MASK   (0x3 << REG_BUCK_VHPPA_CLK_LPO_PULSE_SHIFT)
#define REG_BUCK_VHPPA_CLK_LPO_PULSE(n)     BITFIELD_VAL(REG_BUCK_VHPPA_CLK_LPO_PULSE, n)

// REG_E9
#define REG_CLK_32K_BUCK_VCORE_EN           (1 << 8)
#define REG_BUCK_VCORE_VBAT_3V_EXTRA        (1 << 7)
#define REG_BUCK_VCORE_ULP_FRESH            (1 << 6)
#define REG_BUCK_VCORE_I_HALVE              (1 << 5)
#define REG_BUCK_VCORE_REFRESH_FREQ_SEL_SHIFT 2
#define REG_BUCK_VCORE_REFRESH_FREQ_SEL_MASK (0x7 << REG_BUCK_VCORE_REFRESH_FREQ_SEL_SHIFT)
#define REG_BUCK_VCORE_REFRESH_FREQ_SEL(n)  BITFIELD_VAL(REG_BUCK_VCORE_REFRESH_FREQ_SEL, n)
#define REG_BUCK_VCORE_CLK_LPO_PULSE_SHIFT  0
#define REG_BUCK_VCORE_CLK_LPO_PULSE_MASK   (0x3 << REG_BUCK_VCORE_CLK_LPO_PULSE_SHIFT)
#define REG_BUCK_VCORE_CLK_LPO_PULSE(n)     BITFIELD_VAL(REG_BUCK_VCORE_CLK_LPO_PULSE, n)

// REG_EA
#define REG_EN_SW1P8_PSRAM_DR               (1 << 13)
#define REG_EN_SW1P8_PSRAM_DLY              (1 << 12)
#define REG_EN_SW1P8_PSRAM                  (1 << 11)
#define REG_SW_DLY_CNT_DR                   (1 << 10)
#define REG_EN_SW_DLY_SEL_SHIFT             7
#define REG_EN_SW_DLY_SEL_MASK              (0x7 << REG_EN_SW_DLY_SEL_SHIFT)
#define REG_EN_SW_DLY_SEL(n)                BITFIELD_VAL(REG_EN_SW_DLY_SEL, n)
#define REG_EN_SW_DLY_CNT_SHIFT             0
#define REG_EN_SW_DLY_CNT_MASK              (0x7F << REG_EN_SW_DLY_CNT_SHIFT)
#define REG_EN_SW_DLY_CNT(n)                BITFIELD_VAL(REG_EN_SW_DLY_CNT, n)

// REG_EB
#define REG_EN_SW1P2_PSRAM_DR               (1 << 14)
#define REG_EN_SW1P2_PSRAM_DLY              (1 << 13)
#define REG_EN_SW1P2_PSRAM                  (1 << 12)
#define REG_SW1P2_PSRAM_PULL_DOWN_SHIFT     10
#define REG_SW1P2_PSRAM_PULL_DOWN_MASK      (0x3 << REG_SW1P2_PSRAM_PULL_DOWN_SHIFT)
#define REG_SW1P2_PSRAM_PULL_DOWN(n)        BITFIELD_VAL(REG_SW1P2_PSRAM_PULL_DOWN, n)
#define REG_SW1P2_PSRAM_ISEL_SHIFT          6
#define REG_SW1P2_PSRAM_ISEL_MASK           (0xF << REG_SW1P2_PSRAM_ISEL_SHIFT)
#define REG_SW1P2_PSRAM_ISEL(n)             BITFIELD_VAL(REG_SW1P2_PSRAM_ISEL, n)
#define REG_SW1P8_PSRAM_PULL_DOWN_SHIFT     4
#define REG_SW1P8_PSRAM_PULL_DOWN_MASK      (0x3 << REG_SW1P8_PSRAM_PULL_DOWN_SHIFT)
#define REG_SW1P8_PSRAM_PULL_DOWN(n)        BITFIELD_VAL(REG_SW1P8_PSRAM_PULL_DOWN, n)
#define REG_SW1P8_PSRAM_ISEL_SHIFT          0
#define REG_SW1P8_PSRAM_ISEL_MASK           (0xF << REG_SW1P8_PSRAM_ISEL_SHIFT)
#define REG_SW1P8_PSRAM_ISEL(n)             BITFIELD_VAL(REG_SW1P8_PSRAM_ISEL, n)

// REG_ED
#define REG_RCOSC2M_GOAL_CNT_MARK_15_0_SHIFT 0
#define REG_RCOSC2M_GOAL_CNT_MARK_15_0_MASK (0x7FFF << REG_RCOSC2M_GOAL_CNT_MARK_15_0_SHIFT)
#define REG_RCOSC2M_GOAL_CNT_MARK_15_0(n)   BITFIELD_VAL(REG_RCOSC2M_GOAL_CNT_MARK_15_0, n)

// REG_EE
#define REG_RCOSC2M_GOAL_CNT_MARK_16        (1 << 15)
#define REG_RCOSC2M_SFT_RSTN                (1 << 14)
#define REG_RCOSC2M_REF_DIV_NUM_SHIFT       4
#define REG_RCOSC2M_REF_DIV_NUM_MASK        (0x3FF << REG_RCOSC2M_REF_DIV_NUM_SHIFT)
#define REG_RCOSC2M_REF_DIV_NUM(n)          BITFIELD_VAL(REG_RCOSC2M_REF_DIV_NUM, n)
#define REG_RCOSC2M_CALIB_EN                (1 << 3)
#define REG_RCOSC2M_TUN_CODE_DR             (1 << 2)
#define REG_RCOSC2M_TUN_CODE_FINE_DR        (1 << 1)
#define REG_RCOSC2M_POLAR_SEL               (1 << 0)

// REG_EF
#define REG_RCOSC2M_TUN_CODE_SHIFT          5
#define REG_RCOSC2M_TUN_CODE_MASK           (0x3F << REG_RCOSC2M_TUN_CODE_SHIFT)
#define REG_RCOSC2M_TUN_CODE(n)             BITFIELD_VAL(REG_RCOSC2M_TUN_CODE, n)
#define REG_RCOSC2M_TUN_CODE_FINE_SHIFT     0
#define REG_RCOSC2M_TUN_CODE_FINE_MASK      (0x1F << REG_RCOSC2M_TUN_CODE_FINE_SHIFT)
#define REG_RCOSC2M_TUN_CODE_FINE(n)        BITFIELD_VAL(REG_RCOSC2M_TUN_CODE_FINE, n)

// REG_F0
#define DBG_RCOSC2M_GOAL_CNT_16             (1 << 6)
#define REG_REF_DIV_SHIFT                   0
#define REG_REF_DIV_MASK                    (0x3F << REG_REF_DIV_SHIFT)
#define REG_REF_DIV(n)                      BITFIELD_VAL(REG_REF_DIV, n)

// REG_F1
#define DBG_RCOSC2M_GOAL_CNT_15_0_SHIFT     0
#define DBG_RCOSC2M_GOAL_CNT_15_0_MASK      (0xFFFF << DBG_RCOSC2M_GOAL_CNT_15_0_SHIFT)
#define DBG_RCOSC2M_GOAL_CNT_15_0(n)        BITFIELD_VAL(DBG_RCOSC2M_GOAL_CNT_15_0, n)

// REG_F2
#define DBG_RCOSC2M_STATE_SHIFT             12
#define DBG_RCOSC2M_STATE_MASK              (0x3 << DBG_RCOSC2M_STATE_SHIFT)
#define DBG_RCOSC2M_STATE(n)                BITFIELD_VAL(DBG_RCOSC2M_STATE, n)
#define DBG_RCOSC2M_TUN_DONE                (1 << 11)
#define DBG_RCOSC2M_FRIM_SHIFT              5
#define DBG_RCOSC2M_FRIM_MASK               (0x3F << DBG_RCOSC2M_FRIM_SHIFT)
#define DBG_RCOSC2M_FRIM(n)                 BITFIELD_VAL(DBG_RCOSC2M_FRIM, n)
#define DBG_RCOSC2M_FRIM_FINE_SHIFT         0
#define DBG_RCOSC2M_FRIM_FINE_MASK          (0x1F << DBG_RCOSC2M_FRIM_FINE_SHIFT)
#define DBG_RCOSC2M_FRIM_FINE(n)            BITFIELD_VAL(DBG_RCOSC2M_FRIM_FINE, n)

// REG_F3
#define REG_RCOSC_2M_VOUT_SEL_SHIFT         13
#define REG_RCOSC_2M_VOUT_SEL_MASK          (0x7 << REG_RCOSC_2M_VOUT_SEL_SHIFT)
#define REG_RCOSC_2M_VOUT_SEL(n)            BITFIELD_VAL(REG_RCOSC_2M_VOUT_SEL, n)

#define DBG_RCOSC2M_REF_CNT_SHIFT           0
#define DBG_RCOSC2M_REF_CNT_MASK            (0x3FF << DBG_RCOSC2M_REF_CNT_SHIFT)
#define DBG_RCOSC2M_REF_CNT(n)              BITFIELD_VAL(DBG_RCOSC2M_REF_CNT, n)

// REG_F4
#define REG_RCOSC_2M_LP_LDO_SHIFT           13
#define REG_RCOSC_2M_LP_LDO_MASK            (0x3 << REG_RCOSC_2M_LP_LDO_SHIFT)
#define REG_RCOSC_2M_LP_LDO(n)              BITFIELD_VAL(REG_RCOSC_2M_LP_LDO, n)
#define REG_RCOSC_2M_CALIB_GATE             (1 << 12)
#define REG_RCOSC_2M_EN_CLK_DIG             (1 << 11)
#define REG_RCOSC_2M_EN_DR                  (1 << 10)
#define REG_RCOSC_2M_EN                     (1 << 9)
#define REG_RCOSC_2M_PU_LDO_DR              (1 << 8)
#define REG_RCOSC_2M_PU_LDO                 (1 << 7)
#define REG_RCOSC_2M_EN_AGPIO_TEST          (1 << 6)
#define REG_RCOSC_2M_EN_CLK_PMU_DIG         (1 << 5)
#define REG_RCOSC_2M_EN_CLK_DLATCH          (1 << 4)
#define REG_RCOSC_2M_OSC_FR_SHIFT           2
#define REG_RCOSC_2M_OSC_FR_MASK            (0x3 << REG_RCOSC_2M_OSC_FR_SHIFT)
#define REG_RCOSC_2M_OSC_FR(n)              BITFIELD_VAL(REG_RCOSC_2M_OSC_FR, n)
#define REG_RCOSC_2M_PWR_BIT_SHIFT          0
#define REG_RCOSC_2M_PWR_BIT_MASK           (0x3 << REG_RCOSC_2M_PWR_BIT_SHIFT)
#define REG_RCOSC_2M_PWR_BIT(n)             BITFIELD_VAL(REG_RCOSC_2M_PWR_BIT, n)

// REG_F5
#define REG_RAMP_DIV_BYPASS                 (1 << 14)
#define REG_RAMP_BIAS_DIV_BYPASS            (1 << 13)
#define REG_CLK_RAMP_DIV_SHIFT              8
#define REG_CLK_RAMP_DIV_MASK               (0x1F << REG_CLK_RAMP_DIV_SHIFT)
#define REG_CLK_RAMP_DIV(n)                 BITFIELD_VAL(REG_CLK_RAMP_DIV, n)
#define REG_RAMP_BIAS_FAST_MSK              (1 << 7)
#define REG_CLK_RAMP_RC_GATE_EN             (1 << 6)
#define REG_RAMP_FAST_EN                    (1 << 5)
#define REG_RAMP_FAST_EN_DR                 (1 << 4)
#define REG_RAMP_CLK_TIME_SHIFT             0
#define REG_RAMP_CLK_TIME_MASK              (0xF << REG_RAMP_CLK_TIME_SHIFT)
#define REG_RAMP_CLK_TIME(n)                BITFIELD_VAL(REG_RAMP_CLK_TIME, n)

// REG_F6
#define REG_CLK_RAMP_BIAS_DIV_SHIFT         8
#define REG_CLK_RAMP_BIAS_DIV_MASK          (0x1F << REG_CLK_RAMP_BIAS_DIV_SHIFT)
#define REG_CLK_RAMP_BIAS_DIV(n)            BITFIELD_VAL(REG_CLK_RAMP_BIAS_DIV, n)
#define ABORT_SOURCE_CLR                    (1 << 7)
#define PMU_LDO_ON_SOURCE_CLR               (1 << 6)
#define ABORT_SOURCE_SHIFT                  3
#define ABORT_SOURCE_MASK                   (0x7 << ABORT_SOURCE_SHIFT)
#define ABORT_SOURCE(n)                     BITFIELD_VAL(ABORT_SOURCE, n)
#define PMU_LDO_ON_SOURCE_SHIFT             0
#define PMU_LDO_ON_SOURCE_MASK              (0x7 << PMU_LDO_ON_SOURCE_SHIFT)
#define PMU_LDO_ON_SOURCE(n)                BITFIELD_VAL(PMU_LDO_ON_SOURCE, n)

// REG_F7
#define REG_RCOSC6M_GOAL_CNT_MARK_15_0_SHIFT 0
#define REG_RCOSC6M_GOAL_CNT_MARK_15_0_MASK (0xFFFF << REG_RCOSC6M_GOAL_CNT_MARK_15_0_SHIFT)
#define REG_RCOSC6M_GOAL_CNT_MARK_15_0(n)   BITFIELD_VAL(REG_RCOSC6M_GOAL_CNT_MARK_15_0, n)

// REG_F8
#define REG_RCOSC6M_GOAL_CNT_MARK_16        (1 << 15)
#define REG_RCOSC6M_SFT_RSTN                (1 << 14)
#define REG_RCOSC6M_REF_DIV_NUM_SHIFT       4
#define REG_RCOSC6M_REF_DIV_NUM_MASK        (0x3FF << REG_RCOSC6M_REF_DIV_NUM_SHIFT)
#define REG_RCOSC6M_REF_DIV_NUM(n)          BITFIELD_VAL(REG_RCOSC6M_REF_DIV_NUM, n)
#define REG_RCOSC6M_CALIB_EN                (1 << 3)
#define REG_RCOSC6M_TUN_CODE_DR             (1 << 2)
#define REG_RCOSC6M_TUN_CODE_FINE_DR        (1 << 1)
#define REG_RCOSC6M_POLAR_SEL               (1 << 0)

// REG_F9
#define REG_RCOSC6M_TUN_CODE_SHIFT          6
#define REG_RCOSC6M_TUN_CODE_MASK           (0x1F << REG_RCOSC6M_TUN_CODE_SHIFT)
#define REG_RCOSC6M_TUN_CODE(n)             BITFIELD_VAL(REG_RCOSC6M_TUN_CODE, n)
#define REG_RCOSC6M_TUN_CODE_FINE_SHIFT     0
#define REG_RCOSC6M_TUN_CODE_FINE_MASK      (0x3F << REG_RCOSC6M_TUN_CODE_FINE_SHIFT)
#define REG_RCOSC6M_TUN_CODE_FINE(n)        BITFIELD_VAL(REG_RCOSC6M_TUN_CODE_FINE, n)

// REG_FA
#define REG_RCOSC_192M_CALIB_GATE           (1 << 15)
#define REG_RCOSC_192M_EN_CLK_CODEC_DIG     (1 << 14)
#define DBG_RCOSC6M_GOAL_CNT_16             (1 << 13)
#define REG_RCOSC_192M_OSC_FR_SHIFT         10
#define REG_RCOSC_192M_OSC_FR_MASK          (0x7 << REG_RCOSC_192M_OSC_FR_SHIFT)
#define REG_RCOSC_192M_OSC_FR(n)            BITFIELD_VAL(REG_RCOSC_192M_OSC_FR, n)
#define REG_RCOSC_192M_DIG_SEL_SHIFT        8
#define REG_RCOSC_192M_DIG_SEL_MASK         (0x3 << REG_RCOSC_192M_DIG_SEL_SHIFT)
#define REG_RCOSC_192M_DIG_SEL(n)           BITFIELD_VAL(REG_RCOSC_192M_DIG_SEL, n)
#define REG_RCOSC_192M_CODEC_SEL_SHIFT      6
#define REG_RCOSC_192M_CODEC_SEL_MASK       (0x3 << REG_RCOSC_192M_CODEC_SEL_SHIFT)
#define REG_RCOSC_192M_CODEC_SEL(n)         BITFIELD_VAL(REG_RCOSC_192M_CODEC_SEL, n)
#define REG_6M_REF_DIV_SHIFT                0
#define REG_6M_REF_DIV_MASK                 (0x3F << REG_6M_REF_DIV_SHIFT)
#define REG_6M_REF_DIV(n)                   BITFIELD_VAL(REG_6M_REF_DIV, n)

// REG_FB
#define DBG_RCOSC6M_GOAL_CNT_15_0_SHIFT     0
#define DBG_RCOSC6M_GOAL_CNT_15_0_MASK      (0xFFFF << DBG_RCOSC6M_GOAL_CNT_15_0_SHIFT)
#define DBG_RCOSC6M_GOAL_CNT_15_0(n)        BITFIELD_VAL(DBG_RCOSC6M_GOAL_CNT_15_0, n)

// REG_FC
#define DBG_RCOSC6M_STATE_SHIFT             12
#define DBG_RCOSC6M_STATE_MASK              (0x3 << DBG_RCOSC6M_STATE_SHIFT)
#define DBG_RCOSC6M_STATE(n)                BITFIELD_VAL(DBG_RCOSC6M_STATE, n)
#define DBG_RCOSC6M_TUN_DONE                (1 << 11)
#define DBG_RCOSC6M_FRIM_SHIFT              6
#define DBG_RCOSC6M_FRIM_MASK               (0x1F << DBG_RCOSC6M_FRIM_SHIFT)
#define DBG_RCOSC6M_FRIM(n)                 BITFIELD_VAL(DBG_RCOSC6M_FRIM, n)
#define DBG_RCOSC6M_FRIM_FINE_SHIFT         0
#define DBG_RCOSC6M_FRIM_FINE_MASK          (0x3F << DBG_RCOSC6M_FRIM_FINE_SHIFT)
#define DBG_RCOSC6M_FRIM_FINE(n)            BITFIELD_VAL(DBG_RCOSC6M_FRIM_FINE, n)

// REG_FD
#define REG_RCOSC_192M_VOUT_SEL_SHIFT       13
#define REG_RCOSC_192M_VOUT_SEL_MASK        (0x7 << REG_RCOSC_192M_VOUT_SEL_SHIFT)
#define REG_RCOSC_192M_VOUT_SEL(n)          BITFIELD_VAL(REG_RCOSC_192M_VOUT_SEL, n)
#define REG_PU_RC_CAP_MSK                   (1 << 10)
#define DBG_RCOSC6M_REF_CNT_SHIFT           0
#define DBG_RCOSC6M_REF_CNT_MASK            (0x3FF << DBG_RCOSC6M_REF_CNT_SHIFT)
#define DBG_RCOSC6M_REF_CNT(n)              BITFIELD_VAL(DBG_RCOSC6M_REF_CNT, n)

// REG_FE
#define REG_RCOSC_192M_FTRIM_ADJ_SHIFT      13
#define REG_RCOSC_192M_FTRIM_ADJ_MASK       (0x7 << REG_RCOSC_192M_FTRIM_ADJ_SHIFT)
#define REG_RCOSC_192M_FTRIM_ADJ(n)         BITFIELD_VAL(REG_RCOSC_192M_FTRIM_ADJ, n)
#define REG_RCOSC_192M_EN_AGPIO_TEST_SEL    (1 << 12)
#define REG_RCOSC_192M_EN_CLK_DIG           (1 << 11)
#define REG_RCOSC_192M_EN_DR                (1 << 10)
#define REG_RCOSC_192M_EN                   (1 << 9)
#define REG_RCOSC_192M_PU_LDO_DR            (1 << 8)
#define REG_RCOSC_192M_PU_LDO               (1 << 7)
#define REG_RCOSC_192M_EN_AGPIO_TEST        (1 << 6)
#define REG_RCOSC_192M_EN_CLK_ADC_DIG       (1 << 5)
#define REG_RCOSC_192M_EN_CLK_DLATCH        (1 << 4)
#define REG_RCOSC_192M_LP_LDO_SHIFT         2
#define REG_RCOSC_192M_LP_LDO_MASK          (0x3 << REG_RCOSC_192M_LP_LDO_SHIFT)
#define REG_RCOSC_192M_LP_LDO(n)            BITFIELD_VAL(REG_RCOSC_192M_LP_LDO, n)
#define REG_RCOSC_192M_PWR_BIT_SHIFT        0
#define REG_RCOSC_192M_PWR_BIT_MASK         (0x3 << REG_RCOSC_192M_PWR_BIT_SHIFT)
#define REG_RCOSC_192M_PWR_BIT(n)           BITFIELD_VAL(REG_RCOSC_192M_PWR_BIT, n)

// REG_10A
#define RTC_MATCH_TIME_MIN_TENS_SHIFT       12
#define RTC_MATCH_TIME_MIN_TENS_MASK        (0x7 << RTC_MATCH_TIME_MIN_TENS_SHIFT)
#define RTC_MATCH_TIME_MIN_TENS(n)          BITFIELD_VAL(RTC_MATCH_TIME_MIN_TENS, n)
#define RTC_MATCH_TIME_MIN_UNITS_SHIFT      8
#define RTC_MATCH_TIME_MIN_UNITS_MASK       (0xF << RTC_MATCH_TIME_MIN_UNITS_SHIFT)
#define RTC_MATCH_TIME_MIN_UNITS(n)         BITFIELD_VAL(RTC_MATCH_TIME_MIN_UNITS, n)
#define RTC_MATCH_TIME_SEC_TENS_SHIFT       4
#define RTC_MATCH_TIME_SEC_TENS_MASK        (0x7 << RTC_MATCH_TIME_SEC_TENS_SHIFT)
#define RTC_MATCH_TIME_SEC_TENS(n)          BITFIELD_VAL(RTC_MATCH_TIME_SEC_TENS, n)
#define RTC_MATCH_TIME_SEC_UNITS_SHIFT      0
#define RTC_MATCH_TIME_SEC_UNITS_MASK       (0xF << RTC_MATCH_TIME_SEC_UNITS_SHIFT)
#define RTC_MATCH_TIME_SEC_UNITS(n)         BITFIELD_VAL(RTC_MATCH_TIME_SEC_UNITS, n)

// REG_10B
#define RTC_MATCH_HOURS_TENS_SHIFT          4
#define RTC_MATCH_HOURS_TENS_MASK           (0x3 << RTC_MATCH_HOURS_TENS_SHIFT)
#define RTC_MATCH_HOURS_TENS(n)             BITFIELD_VAL(RTC_MATCH_HOURS_TENS, n)
#define RTC_MATCH_HOURS_UNITS_SHIFT         0
#define RTC_MATCH_HOURS_UNITS_MASK          (0xF << RTC_MATCH_HOURS_UNITS_SHIFT)
#define RTC_MATCH_HOURS_UNITS(n)            BITFIELD_VAL(RTC_MATCH_HOURS_UNITS, n)

// REG_10C
#define RTC_MATCH_DATAS_WEEK_SHIFT          13
#define RTC_MATCH_DATAS_WEEK_MASK           (0x7 << RTC_MATCH_DATAS_WEEK_SHIFT)
#define RTC_MATCH_DATAS_WEEK(n)             BITFIELD_VAL(RTC_MATCH_DATAS_WEEK, n)
#define RTC_MATCH_DATAS_MONTH_TENS          (1 << 12)
#define RTC_MATCH_DATAS_MONTH_UNITS_SHIFT   8
#define RTC_MATCH_DATAS_MONTH_UNITS_MASK    (0xF << RTC_MATCH_DATAS_MONTH_UNITS_SHIFT)
#define RTC_MATCH_DATAS_MONTH_UNITS(n)      BITFIELD_VAL(RTC_MATCH_DATAS_MONTH_UNITS, n)
#define RTC_MATCH_DATAS_DAY_TENS_SHIFT      4
#define RTC_MATCH_DATAS_DAY_TENS_MASK       (0x3 << RTC_MATCH_DATAS_DAY_TENS_SHIFT)
#define RTC_MATCH_DATAS_DAY_TENS(n)         BITFIELD_VAL(RTC_MATCH_DATAS_DAY_TENS, n)
#define RTC_MATCH_DATAS_DAY_UNITS_SHIFT     0
#define RTC_MATCH_DATAS_DAY_UNITS_MASK      (0xF << RTC_MATCH_DATAS_DAY_UNITS_SHIFT)
#define RTC_MATCH_DATAS_DAY_UNITS(n)        BITFIELD_VAL(RTC_MATCH_DATAS_DAY_UNITS, n)

// REG_10D
#define RTC_MATCH_YEARS_TENS_SHIFT          4
#define RTC_MATCH_YEARS_TENS_MASK           (0xF << RTC_MATCH_YEARS_TENS_SHIFT)
#define RTC_MATCH_YEARS_TENS(n)             BITFIELD_VAL(RTC_MATCH_YEARS_TENS, n)
#define RTC_MATCH_YEARS_UNITS_SHIFT         0
#define RTC_MATCH_YEARS_UNITS_MASK          (0xF << RTC_MATCH_YEARS_UNITS_SHIFT)
#define RTC_MATCH_YEARS_UNITS(n)            BITFIELD_VAL(RTC_MATCH_YEARS_UNITS, n)

// REG_10E
#define REG_CLK_RTC_SEL_XO                  (1 << 1)
#define RTC_TIME_LOAD                       (1 << 0)

// REG_10F
#define RTC_LOAD_TIME_MIN_TENS_SHIFT        12
#define RTC_LOAD_TIME_MIN_TENS_MASK         (0x7 << RTC_LOAD_TIME_MIN_TENS_SHIFT)
#define RTC_LOAD_TIME_MIN_TENS(n)           BITFIELD_VAL(RTC_LOAD_TIME_MIN_TENS, n)
#define RTC_LOAD_TIME_MIN_UNITS_SHIFT       8
#define RTC_LOAD_TIME_MIN_UNITS_MASK        (0xF << RTC_LOAD_TIME_MIN_UNITS_SHIFT)
#define RTC_LOAD_TIME_MIN_UNITS(n)          BITFIELD_VAL(RTC_LOAD_TIME_MIN_UNITS, n)
#define RTC_LOAD_TIME_SEC_TENS_SHIFT        4
#define RTC_LOAD_TIME_SEC_TENS_MASK         (0x7 << RTC_LOAD_TIME_SEC_TENS_SHIFT)
#define RTC_LOAD_TIME_SEC_TENS(n)           BITFIELD_VAL(RTC_LOAD_TIME_SEC_TENS, n)
#define RTC_LOAD_TIME_SEC_UNITS_SHIFT       0
#define RTC_LOAD_TIME_SEC_UNITS_MASK        (0xF << RTC_LOAD_TIME_SEC_UNITS_SHIFT)
#define RTC_LOAD_TIME_SEC_UNITS(n)          BITFIELD_VAL(RTC_LOAD_TIME_SEC_UNITS, n)

// REG_110
#define RTC_LOAD_HOURS_TENS_SHIFT           4
#define RTC_LOAD_HOURS_TENS_MASK            (0x3 << RTC_LOAD_HOURS_TENS_SHIFT)
#define RTC_LOAD_HOURS_TENS(n)              BITFIELD_VAL(RTC_LOAD_HOURS_TENS, n)
#define RTC_LOAD_HOURS_UNITS_SHIFT          0
#define RTC_LOAD_HOURS_UNITS_MASK           (0xF << RTC_LOAD_HOURS_UNITS_SHIFT)
#define RTC_LOAD_HOURS_UNITS(n)             BITFIELD_VAL(RTC_LOAD_HOURS_UNITS, n)

// REG_111
#define RTC_LOAD_DATAS_WEEK_SHIFT           13
#define RTC_LOAD_DATAS_WEEK_MASK            (0x7 << RTC_LOAD_DATAS_WEEK_SHIFT)
#define RTC_LOAD_DATAS_WEEK(n)              BITFIELD_VAL(RTC_LOAD_DATAS_WEEK, n)
#define RTC_LOAD_DATAS_MONTH_TENS           (1 << 12)
#define RTC_LOAD_DATAS_MONTH_UNITS_SHIFT    8
#define RTC_LOAD_DATAS_MONTH_UNITS_MASK     (0xF << RTC_LOAD_DATAS_MONTH_UNITS_SHIFT)
#define RTC_LOAD_DATAS_MONTH_UNITS(n)       BITFIELD_VAL(RTC_LOAD_DATAS_MONTH_UNITS, n)
#define RTC_LOAD_DATAS_DAY_TENS_SHIFT       4
#define RTC_LOAD_DATAS_DAY_TENS_MASK        (0x3 << RTC_LOAD_DATAS_DAY_TENS_SHIFT)
#define RTC_LOAD_DATAS_DAY_TENS(n)          BITFIELD_VAL(RTC_LOAD_DATAS_DAY_TENS, n)
#define RTC_LOAD_DATAS_DAY_UNITS_SHIFT      0
#define RTC_LOAD_DATAS_DAY_UNITS_MASK       (0xF << RTC_LOAD_DATAS_DAY_UNITS_SHIFT)
#define RTC_LOAD_DATAS_DAY_UNITS(n)         BITFIELD_VAL(RTC_LOAD_DATAS_DAY_UNITS, n)

// REG_112
#define RTC_LOAD_YEARS_TENS_SHIFT           4
#define RTC_LOAD_YEARS_TENS_MASK            (0xF << RTC_LOAD_YEARS_TENS_SHIFT)
#define RTC_LOAD_YEARS_TENS(n)              BITFIELD_VAL(RTC_LOAD_YEARS_TENS, n)
#define RTC_LOAD_YEARS_UNITS_SHIFT          0
#define RTC_LOAD_YEARS_UNITS_MASK           (0xF << RTC_LOAD_YEARS_UNITS_SHIFT)
#define RTC_LOAD_YEARS_UNITS(n)             BITFIELD_VAL(RTC_LOAD_YEARS_UNITS, n)

// REG_113
#define RTC_GET_TIME_MIN_TENS_SHIFT         12
#define RTC_GET_TIME_MIN_TENS_MASK          (0x7 << RTC_GET_TIME_MIN_TENS_SHIFT)
#define RTC_GET_TIME_MIN_TENS(n)            BITFIELD_VAL(RTC_GET_TIME_MIN_TENS, n)
#define RTC_GET_TIME_MIN_UNITS_SHIFT        8
#define RTC_GET_TIME_MIN_UNITS_MASK         (0xF << RTC_GET_TIME_MIN_UNITS_SHIFT)
#define RTC_GET_TIME_MIN_UNITS(n)           BITFIELD_VAL(RTC_GET_TIME_MIN_UNITS, n)
#define RTC_GET_TIME_SEC_TENS_SHIFT         4
#define RTC_GET_TIME_SEC_TENS_MASK          (0x7 << RTC_GET_TIME_SEC_TENS_SHIFT)
#define RTC_GET_TIME_SEC_TENS(n)            BITFIELD_VAL(RTC_GET_TIME_SEC_TENS, n)
#define RTC_GET_TIME_SEC_UNITS_SHIFT        0
#define RTC_GET_TIME_SEC_UNITS_MASK         (0xF << RTC_GET_TIME_SEC_UNITS_SHIFT)
#define RTC_GET_TIME_SEC_UNITS(n)           BITFIELD_VAL(RTC_GET_TIME_SEC_UNITS, n)

// REG_114
#define RTC_GET_HOURS_TENS_SHIFT            4
#define RTC_GET_HOURS_TENS_MASK             (0x3 << RTC_GET_HOURS_TENS_SHIFT)
#define RTC_GET_HOURS_TENS(n)               BITFIELD_VAL(RTC_GET_HOURS_TENS, n)
#define RTC_GET_HOURS_UNITS_SHIFT           0
#define RTC_GET_HOURS_UNITS_MASK            (0xF << RTC_GET_HOURS_UNITS_SHIFT)
#define RTC_GET_HOURS_UNITS(n)              BITFIELD_VAL(RTC_GET_HOURS_UNITS, n)

// REG_115
#define RTC_GET_DATAS_WEEK_SHIFT            13
#define RTC_GET_DATAS_WEEK_MASK             (0x7 << RTC_GET_DATAS_WEEK_SHIFT)
#define RTC_GET_DATAS_WEEK(n)               BITFIELD_VAL(RTC_GET_DATAS_WEEK, n)
#define RTC_GET_DATAS_MONTH_TENS            (1 << 12)
#define RTC_GET_DATAS_MONTH_UNITS_SHIFT     8
#define RTC_GET_DATAS_MONTH_UNITS_MASK      (0xF << RTC_GET_DATAS_MONTH_UNITS_SHIFT)
#define RTC_GET_DATAS_MONTH_UNITS(n)        BITFIELD_VAL(RTC_GET_DATAS_MONTH_UNITS, n)
#define RTC_GET_DATAS_DAY_TENS_SHIFT        4
#define RTC_GET_DATAS_DAY_TENS_MASK         (0x3 << RTC_GET_DATAS_DAY_TENS_SHIFT)
#define RTC_GET_DATAS_DAY_TENS(n)           BITFIELD_VAL(RTC_GET_DATAS_DAY_TENS, n)
#define RTC_GET_DATAS_DAY_UNITS_SHIFT       0
#define RTC_GET_DATAS_DAY_UNITS_MASK        (0xF << RTC_GET_DATAS_DAY_UNITS_SHIFT)
#define RTC_GET_DATAS_DAY_UNITS(n)          BITFIELD_VAL(RTC_GET_DATAS_DAY_UNITS, n)

// REG_116
#define RTC_GET_YEARS_TENS_SHIFT            4
#define RTC_GET_YEARS_TENS_MASK             (0xF << RTC_GET_YEARS_TENS_SHIFT)
#define RTC_GET_YEARS_TENS(n)               BITFIELD_VAL(RTC_GET_YEARS_TENS, n)
#define RTC_GET_YEARS_UNITS_SHIFT           0
#define RTC_GET_YEARS_UNITS_MASK            (0xF << RTC_GET_YEARS_UNITS_SHIFT)
#define RTC_GET_YEARS_UNITS(n)              BITFIELD_VAL(RTC_GET_YEARS_UNITS, n)

// REG_117
#define REG_LDO_VANA_PULLDOWN               (1 << 12)
#define REG_TRAN_ENHANCE_LDO_VANA           (1 << 11)
#define REG_TRAN_ENHANCE_LDO_VANA_DR        (1 << 10)
#define REG_LDO_VANA_RAMP_STEP_SHIFT        7
#define REG_LDO_VANA_RAMP_STEP_MASK         (0x7 << REG_LDO_VANA_RAMP_STEP_SHIFT)
#define REG_LDO_VANA_RAMP_STEP(n)           BITFIELD_VAL(REG_LDO_VANA_RAMP_STEP, n)
#define REG_LDO_VANA_RAMP_EN                (1 << 6)
#define REG_PU_LDO_VANA_DSLEEP              (1 << 5)
#define REG_PU_LDO_VANA_DR                  (1 << 4)
#define REG_PU_LDO_VANA                     (1 << 3)
#define REG_LP_EN_LDO_VANA_DSLEEP           (1 << 2)
#define REG_LP_EN_LDO_VANA_DR               (1 << 1)
#define REG_LP_EN_LDO_VANA                  (1 << 0)

#define REG_PU_LDO_VANA_REG                 REG_PU_LDO_VANA
#define REG_LP_EN_LDO_VANA_REG              REG_LP_EN_LDO_VANA

// REG_118
#define REG_LDO_VANA_VBIT_NORMAL_SHIFT      8
#define REG_LDO_VANA_VBIT_NORMAL_MASK       (0xF << REG_LDO_VANA_VBIT_NORMAL_SHIFT)
#define REG_LDO_VANA_VBIT_NORMAL(n)         BITFIELD_VAL(REG_LDO_VANA_VBIT_NORMAL, n)
#define REG_LDO_VANA_VBIT_DSLEEP_SHIFT      4
#define REG_LDO_VANA_VBIT_DSLEEP_MASK       (0xF << REG_LDO_VANA_VBIT_DSLEEP_SHIFT)
#define REG_LDO_VANA_VBIT_DSLEEP(n)         BITFIELD_VAL(REG_LDO_VANA_VBIT_DSLEEP, n)
#define REG_LDO_VANA_VBIT_RC_SHIFT          0
#define REG_LDO_VANA_VBIT_RC_MASK           (0xF << REG_LDO_VANA_VBIT_RC_SHIFT)
#define REG_LDO_VANA_VBIT_RC(n)             BITFIELD_VAL(REG_LDO_VANA_VBIT_RC, n)

// REG_119
#define REG_LDO_VCORE_PULLDOWN              (1 << 10)
#define REG_LDO_VCORE_RAMP_STEP_SHIFT       7
#define REG_LDO_VCORE_RAMP_STEP_MASK        (0x7 << REG_LDO_VCORE_RAMP_STEP_SHIFT)
#define REG_LDO_VCORE_RAMP_STEP(n)          BITFIELD_VAL(REG_LDO_VCORE_RAMP_STEP, n)
#define REG_LDO_VCORE_RAMP_EN               (1 << 6)
#define REG_PU_LDO_VCORE_DSLEEP             (1 << 5)
#define REG_PU_LDO_VCORE_DR                 (1 << 4)
#define REG_PU_LDO_VCORE                    (1 << 3)
#define REG_LP_EN_LDO_VCORE_DSLEEP          (1 << 2)
#define REG_LP_EN_LDO_VCORE_DR              (1 << 1)
#define REG_LP_EN_LDO_VCORE                 (1 << 0)

#define REG_PU_LDO_VCORE_REG                REG_PU_LDO_VCORE
#define REG_LP_EN_LDO_VCORE_REG             REG_LP_EN_LDO_VCORE

// REG_11A
#define REG_LDO_VCORE_VBIT_NORMAL_SHIFT     8
#define REG_LDO_VCORE_VBIT_NORMAL_MASK      (0xF << REG_LDO_VCORE_VBIT_NORMAL_SHIFT)
#define REG_LDO_VCORE_VBIT_NORMAL(n)        BITFIELD_VAL(REG_LDO_VCORE_VBIT_NORMAL, n)
#define REG_LDO_VCORE_VBIT_DSLEEP_SHIFT     4
#define REG_LDO_VCORE_VBIT_DSLEEP_MASK      (0xF << REG_LDO_VCORE_VBIT_DSLEEP_SHIFT)
#define REG_LDO_VCORE_VBIT_DSLEEP(n)        BITFIELD_VAL(REG_LDO_VCORE_VBIT_DSLEEP, n)
#define REG_LDO_VCORE_VBIT_RC_SHIFT         0
#define REG_LDO_VCORE_VBIT_RC_MASK          (0xF << REG_LDO_VCORE_VBIT_RC_SHIFT)
#define REG_LDO_VCORE_VBIT_RC(n)            BITFIELD_VAL(REG_LDO_VCORE_VBIT_RC, n)

// REG_11B
#define REG_LDO_VCORE_L_PULLDOWN            (1 << 12)
#define REG_LDO_VCORE_L_RAMP_STEP_SHIFT     7
#define REG_LDO_VCORE_L_RAMP_STEP_MASK      (0x1F << REG_LDO_VCORE_L_RAMP_STEP_SHIFT)
#define REG_LDO_VCORE_L_RAMP_STEP(n)        BITFIELD_VAL(REG_LDO_VCORE_L_RAMP_STEP, n)
#define REG_LDO_VCORE_L_RAMP_EN             (1 << 6)
#define REG_PU_LDO_VCORE_L_DSLEEP           (1 << 5)
#define REG_PU_LDO_VCORE_L_DR               (1 << 4)
#define REG_PU_LDO_VCORE_L                  (1 << 3)
#define REG_LP_EN_LDO_VCORE_L_DSLEEP        (1 << 2)
#define REG_LP_EN_LDO_VCORE_L_DR            (1 << 1)
#define REG_LP_EN_LDO_VCORE_L               (1 << 0)

#define REG_PU_LDO_VCORE_L_REG              REG_PU_LDO_VCORE_L
#define REG_LP_EN_LDO_VCORE_L_REG           REG_LP_EN_LDO_VCORE_L

// REG_11C
#define REG_LDO_VCORE_L_VBIT_NORMAL_SHIFT   0
#define REG_LDO_VCORE_L_VBIT_NORMAL_MASK    (0xFF << REG_LDO_VCORE_L_VBIT_NORMAL_SHIFT)
#define REG_LDO_VCORE_L_VBIT_NORMAL(n)      BITFIELD_VAL(REG_LDO_VCORE_L_VBIT_NORMAL, n)

// REG_11D
#define REG_LDO_VCORE_L_VBIT_DSLEEP_SHIFT   8
#define REG_LDO_VCORE_L_VBIT_DSLEEP_MASK    (0xFF << REG_LDO_VCORE_L_VBIT_DSLEEP_SHIFT)
#define REG_LDO_VCORE_L_VBIT_DSLEEP(n)      BITFIELD_VAL(REG_LDO_VCORE_L_VBIT_DSLEEP, n)
#define REG_LDO_VCORE_L_VBIT_RC_SHIFT       0
#define REG_LDO_VCORE_L_VBIT_RC_MASK        (0xFF << REG_LDO_VCORE_L_VBIT_RC_SHIFT)
#define REG_LDO_VCORE_L_VBIT_RC(n)          BITFIELD_VAL(REG_LDO_VCORE_L_VBIT_RC, n)

// REG_11E
#define REG_LDO_VCODEC_PULLDOWN             (1 << 13)
#define REG_TRAN_ENHANCE_LDO_VCODEC         (1 << 12)
#define REG_TRAN_ENHANCE_LDO_VCODEC_DR      (1 << 11)
#define REG_LDO_VCODEC_RAMP_STEP_SHIFT      7
#define REG_LDO_VCODEC_RAMP_STEP_MASK       (0xF << REG_LDO_VCODEC_RAMP_STEP_SHIFT)
#define REG_LDO_VCODEC_RAMP_STEP(n)         BITFIELD_VAL(REG_LDO_VCODEC_RAMP_STEP, n)
#define REG_LDO_VCODEC_RAMP_EN              (1 << 6)
#define REG_PU_LDO_VCODEC_DSLEEP            (1 << 5)
#define REG_PU_LDO_VCODEC_DR                (1 << 4)
#define REG_PU_LDO_VCODEC                   (1 << 3)
#define REG_LP_EN_LDO_VCODEC_DSLEEP         (1 << 2)
#define REG_LP_EN_LDO_VCODEC_DR             (1 << 1)
#define REG_LP_EN_LDO_VCODEC                (1 << 0)

#define REG_PU_LDO_VCODEC_REG               REG_PU_LDO_VCODEC
#define REG_LP_EN_LDO_VCODEC_REG            REG_LP_EN_LDO_VCODEC

// REG_11F
#define REG_LDO_VCODEC_VBIT_NORMAL_SHIFT    10
#define REG_LDO_VCODEC_VBIT_NORMAL_MASK     (0x1F << REG_LDO_VCODEC_VBIT_NORMAL_SHIFT)
#define REG_LDO_VCODEC_VBIT_NORMAL(n)       BITFIELD_VAL(REG_LDO_VCODEC_VBIT_NORMAL, n)
#define REG_LDO_VCODEC_VBIT_DSLEEP_SHIFT    5
#define REG_LDO_VCODEC_VBIT_DSLEEP_MASK     (0x1F << REG_LDO_VCODEC_VBIT_DSLEEP_SHIFT)
#define REG_LDO_VCODEC_VBIT_DSLEEP(n)       BITFIELD_VAL(REG_LDO_VCODEC_VBIT_DSLEEP, n)
#define REG_LDO_VCODEC_VBIT_RC_SHIFT        0
#define REG_LDO_VCODEC_VBIT_RC_MASK         (0x1F << REG_LDO_VCODEC_VBIT_RC_SHIFT)
#define REG_LDO_VCODEC_VBIT_RC(n)           BITFIELD_VAL(REG_LDO_VCODEC_VBIT_RC, n)

// REG_120
#define REG_LDO_VUSB_PULLDOWN               (1 << 11)
#define REG_LDO_VUSB_RAMP_STEP_SHIFT        7
#define REG_LDO_VUSB_RAMP_STEP_MASK         (0xF << REG_LDO_VUSB_RAMP_STEP_SHIFT)
#define REG_LDO_VUSB_RAMP_STEP(n)           BITFIELD_VAL(REG_LDO_VUSB_RAMP_STEP, n)
#define REG_LDO_VUSB_RAMP_EN                (1 << 6)
#define REG_PU_LDO_VUSB_DSLEEP              (1 << 5)
#define REG_PU_LDO_VUSB_DR                  (1 << 4)
#define REG_PU_LDO_VUSB                     (1 << 3)
#define REG_LP_EN_LDO_VUSB_DSLEEP           (1 << 2)
#define REG_LP_EN_LDO_VUSB_DR               (1 << 1)
#define REG_LP_EN_LDO_VUSB                  (1 << 0)

#define REG_PU_LDO_VUSB_REG                 REG_PU_LDO_VUSB
#define REG_LP_EN_LDO_VUSB_REG              REG_LP_EN_LDO_VUSB

// REG_121
#define REG_LDO_VUSB_VBIT_NORMAL_SHIFT      0
#define REG_LDO_VUSB_VBIT_NORMAL_MASK       (0x3F << REG_LDO_VUSB_VBIT_NORMAL_SHIFT)
#define REG_LDO_VUSB_VBIT_NORMAL(n)         BITFIELD_VAL(REG_LDO_VUSB_VBIT_NORMAL, n)

// REG_122
#define REG_LDO_VUSB_VBIT_DSLEEP_SHIFT      6
#define REG_LDO_VUSB_VBIT_DSLEEP_MASK       (0x3F << REG_LDO_VUSB_VBIT_DSLEEP_SHIFT)
#define REG_LDO_VUSB_VBIT_DSLEEP(n)         BITFIELD_VAL(REG_LDO_VUSB_VBIT_DSLEEP, n)
#define REG_LDO_VUSB_VBIT_RC_SHIFT          0
#define REG_LDO_VUSB_VBIT_RC_MASK           (0x3F << REG_LDO_VUSB_VBIT_RC_SHIFT)
#define REG_LDO_VUSB_VBIT_RC(n)             BITFIELD_VAL(REG_LDO_VUSB_VBIT_RC, n)

// REG_123
#define REG_LDO_VSENSOR_PULLDOWN            (1 << 11)
#define REG_LDO_VSENSOR_RAMP_STEP_SHIFT     7
#define REG_LDO_VSENSOR_RAMP_STEP_MASK      (0xF << REG_LDO_VSENSOR_RAMP_STEP_SHIFT)
#define REG_LDO_VSENSOR_RAMP_STEP(n)        BITFIELD_VAL(REG_LDO_VSENSOR_RAMP_STEP, n)
#define REG_LDO_VSENSOR_RAMP_EN             (1 << 6)
#define REG_PU_LDO_VSENSOR_DSLEEP           (1 << 5)
#define REG_PU_LDO_VSENSOR_DR               (1 << 4)
#define REG_PU_LDO_VSENSOR                  (1 << 3)
#define REG_LP_EN_LDO_VSENSOR_DSLEEP        (1 << 2)
#define REG_LP_EN_LDO_VSENSOR_DR            (1 << 1)
#define REG_LP_EN_LDO_VSENSOR               (1 << 0)

#define REG_PU_LDO_VSENSOR_REG              REG_PU_LDO_VSENSOR
#define REG_LP_EN_LDO_VSENSOR_REG           REG_LP_EN_LDO_VSENSOR

// REG_124
#define REG_LDO_VSENSOR_VBIT_NORMAL_SHIFT   0
#define REG_LDO_VSENSOR_VBIT_NORMAL_MASK    (0x3F << REG_LDO_VSENSOR_VBIT_NORMAL_SHIFT)
#define REG_LDO_VSENSOR_VBIT_NORMAL(n)      BITFIELD_VAL(REG_LDO_VSENSOR_VBIT_NORMAL, n)

// REG_125
#define REG_LDO_VSENSOR_VBIT_DSLEEP_SHIFT   6
#define REG_LDO_VSENSOR_VBIT_DSLEEP_MASK    (0x3F << REG_LDO_VSENSOR_VBIT_DSLEEP_SHIFT)
#define REG_LDO_VSENSOR_VBIT_DSLEEP(n)      BITFIELD_VAL(REG_LDO_VSENSOR_VBIT_DSLEEP, n)
#define REG_LDO_VSENSOR_VBIT_RC_SHIFT       0
#define REG_LDO_VSENSOR_VBIT_RC_MASK        (0x3F << REG_LDO_VSENSOR_VBIT_RC_SHIFT)
#define REG_LDO_VSENSOR_VBIT_RC(n)          BITFIELD_VAL(REG_LDO_VSENSOR_VBIT_RC, n)

// REG_126
#define REG_LDO_VMEM_PULLDOWN               (1 << 11)
#define REG_LDO_VMEM_RAMP_STEP_SHIFT        7
#define REG_LDO_VMEM_RAMP_STEP_MASK         (0xF << REG_LDO_VMEM_RAMP_STEP_SHIFT)
#define REG_LDO_VMEM_RAMP_STEP(n)           BITFIELD_VAL(REG_LDO_VMEM_RAMP_STEP, n)
#define REG_LDO_VMEM_RAMP_EN                (1 << 6)
#define REG_PU_LDO_VMEM_DSLEEP              (1 << 5)
#define REG_PU_LDO_VMEM_DR                  (1 << 4)
#define REG_PU_LDO_VMEM                     (1 << 3)
#define REG_LP_EN_LDO_VMEM_DSLEEP           (1 << 2)
#define REG_LP_EN_LDO_VMEM_DR               (1 << 1)
#define REG_LP_EN_LDO_VMEM                  (1 << 0)

#define REG_PU_LDO_VMEM_REG                 REG_PU_LDO_VMEM
#define REG_LP_EN_LDO_VMEM_REG              REG_LP_EN_LDO_VMEM

// REG_127
#define REG_LDO_VMEM_VBIT_NORMAL_SHIFT      0
#define REG_LDO_VMEM_VBIT_NORMAL_MASK       (0x3F << REG_LDO_VMEM_VBIT_NORMAL_SHIFT)
#define REG_LDO_VMEM_VBIT_NORMAL(n)         BITFIELD_VAL(REG_LDO_VMEM_VBIT_NORMAL, n)

// REG_128
#define REG_LDO_VMEM_VBIT_DSLEEP_SHIFT      6
#define REG_LDO_VMEM_VBIT_DSLEEP_MASK       (0x3F << REG_LDO_VMEM_VBIT_DSLEEP_SHIFT)
#define REG_LDO_VMEM_VBIT_DSLEEP(n)         BITFIELD_VAL(REG_LDO_VMEM_VBIT_DSLEEP, n)
#define REG_LDO_VMEM_VBIT_RC_SHIFT          0
#define REG_LDO_VMEM_VBIT_RC_MASK           (0x3F << REG_LDO_VMEM_VBIT_RC_SHIFT)
#define REG_LDO_VMEM_VBIT_RC(n)             BITFIELD_VAL(REG_LDO_VMEM_VBIT_RC, n)

// REG_129
#define REG_LDO_VRTC_RAMP_STEP_SHIFT        1
#define REG_LDO_VRTC_RAMP_STEP_MASK         (0x7 << REG_LDO_VRTC_RAMP_STEP_SHIFT)
#define REG_LDO_VRTC_RAMP_STEP(n)           BITFIELD_VAL(REG_LDO_VRTC_RAMP_STEP, n)
#define REG_LDO_VRTC_RAMP_EN                (1 << 0)

// REG_12A
#define REG_LDO_VRTC_VBIT_PWR_DOWN_SHIFT    12
#define REG_LDO_VRTC_VBIT_PWR_DOWN_MASK     (0xF << REG_LDO_VRTC_VBIT_PWR_DOWN_SHIFT)
#define REG_LDO_VRTC_VBIT_PWR_DOWN(n)       BITFIELD_VAL(REG_LDO_VRTC_VBIT_PWR_DOWN, n)
#define REG_LDO_VRTC_VBIT_NORMAL_SHIFT      8
#define REG_LDO_VRTC_VBIT_NORMAL_MASK       (0xF << REG_LDO_VRTC_VBIT_NORMAL_SHIFT)
#define REG_LDO_VRTC_VBIT_NORMAL(n)         BITFIELD_VAL(REG_LDO_VRTC_VBIT_NORMAL, n)
#define REG_LDO_VRTC_VBIT_DSLEEP_SHIFT      4
#define REG_LDO_VRTC_VBIT_DSLEEP_MASK       (0xF << REG_LDO_VRTC_VBIT_DSLEEP_SHIFT)
#define REG_LDO_VRTC_VBIT_DSLEEP(n)         BITFIELD_VAL(REG_LDO_VRTC_VBIT_DSLEEP, n)
#define REG_LDO_VRTC_VBIT_RC_SHIFT          0
#define REG_LDO_VRTC_VBIT_RC_MASK           (0xF << REG_LDO_VRTC_VBIT_RC_SHIFT)
#define REG_LDO_VRTC_VBIT_RC(n)             BITFIELD_VAL(REG_LDO_VRTC_VBIT_RC, n)

// REG_12B
#define REG_LDO_LP_BIAS_RAMP_STEP_SHIFT     2
#define REG_LDO_LP_BIAS_RAMP_STEP_MASK      (0x7 << REG_LDO_LP_BIAS_RAMP_STEP_SHIFT)
#define REG_LDO_LP_BIAS_RAMP_STEP(n)        BITFIELD_VAL(REG_LDO_LP_BIAS_RAMP_STEP, n)
#define REG_LDO_LP_BIAS_RAMP_EN             (1 << 1)
#define REG_LP_BIAS_SEL_LDO_DR              (1 << 0)

// REG_12C
#define REG_LP_BIAS_SEL_LDO_REG_SHIFT       12
#define REG_LP_BIAS_SEL_LDO_REG_MASK        (0xF << REG_LP_BIAS_SEL_LDO_REG_SHIFT)
#define REG_LP_BIAS_SEL_LDO_REG(n)          BITFIELD_VAL(REG_LP_BIAS_SEL_LDO_REG, n)
#define REG_LDO_LP_BIAS_VBIT_NORMAL_SHIFT   8
#define REG_LDO_LP_BIAS_VBIT_NORMAL_MASK    (0xF << REG_LDO_LP_BIAS_VBIT_NORMAL_SHIFT)
#define REG_LDO_LP_BIAS_VBIT_NORMAL(n)      BITFIELD_VAL(REG_LDO_LP_BIAS_VBIT_NORMAL, n)
#define REG_LDO_LP_BIAS_VBIT_DSLEEP_SHIFT   4
#define REG_LDO_LP_BIAS_VBIT_DSLEEP_MASK    (0xF << REG_LDO_LP_BIAS_VBIT_DSLEEP_SHIFT)
#define REG_LDO_LP_BIAS_VBIT_DSLEEP(n)      BITFIELD_VAL(REG_LDO_LP_BIAS_VBIT_DSLEEP, n)
#define REG_LDO_LP_BIAS_VBIT_RC_SHIFT       0
#define REG_LDO_LP_BIAS_VBIT_RC_MASK        (0xF << REG_LDO_LP_BIAS_VBIT_RC_SHIFT)
#define REG_LDO_LP_BIAS_VBIT_RC(n)          BITFIELD_VAL(REG_LDO_LP_BIAS_VBIT_RC, n)

// REG_12D
#define REG_BUCK_VANA_RAMP_STEP_SHIFT       4
#define REG_BUCK_VANA_RAMP_STEP_MASK        (0x1F << REG_BUCK_VANA_RAMP_STEP_SHIFT)
#define REG_BUCK_VANA_RAMP_STEP(n)          BITFIELD_VAL(REG_BUCK_VANA_RAMP_STEP, n)
#define REG_BUCK_VANA_RAMP_EN               (1 << 3)
#define REG_BUCK_VANA_DSLEEP_MSK            (1 << 2)
#define REG_PU_BUCK_VANA_DR                 (1 << 1)
#define REG_PU_BUCK_VANA                    (1 << 0)

// REG_12E
#define REG_BUCK_VANA_VREF_SEL_NORMAL_SHIFT 14
#define REG_BUCK_VANA_VREF_SEL_NORMAL_MASK  (0x3 << REG_BUCK_VANA_VREF_SEL_NORMAL_SHIFT)
#define REG_BUCK_VANA_VREF_SEL_NORMAL(n)    BITFIELD_VAL(REG_BUCK_VANA_VREF_SEL_NORMAL, n)
#define REG_BUCK_VANA_IX2_ERR_NORMAL        (1 << 13)
#define REG_BUCK_VANA_BURST_MODE_SEL_NORMAL (1 << 12)
#define REG_BUCK_VANA_COUNTER_SEL_ULP_NORMAL (1 << 10)
#define REG_BUCK_VANA_HYSTERESIS_BIT_NORMAL (1 << 9)
#define REG_BUCK_VANA_VBIT_NORMAL_SHIFT     0
#define REG_BUCK_VANA_VBIT_NORMAL_MASK      (0x1FF << REG_BUCK_VANA_VBIT_NORMAL_SHIFT)
#define REG_BUCK_VANA_VBIT_NORMAL(n)        BITFIELD_VAL(REG_BUCK_VANA_VBIT_NORMAL, n)

// REG_12F
#define REG_BUCK_VANA_VREF_SEL_DSLEEP_SHIFT 14
#define REG_BUCK_VANA_VREF_SEL_DSLEEP_MASK  (0x3 << REG_BUCK_VANA_VREF_SEL_DSLEEP_SHIFT)
#define REG_BUCK_VANA_VREF_SEL_DSLEEP(n)    BITFIELD_VAL(REG_BUCK_VANA_VREF_SEL_DSLEEP, n)
#define REG_BUCK_VANA_IX2_ERR_DSLEEP        (1 << 13)
#define REG_BUCK_VANA_BURST_MODE_SEL_DSLEEP (1 << 12)
#define REG_BUCK_VANA_COUNTER_SEL_ULP_DSLEEP (1 << 10)
#define REG_BUCK_VANA_HYSTERESIS_BIT_DSLEEP (1 << 9)
#define REG_BUCK_VANA_VBIT_DSLEEP_SHIFT     0
#define REG_BUCK_VANA_VBIT_DSLEEP_MASK      (0x1FF << REG_BUCK_VANA_VBIT_DSLEEP_SHIFT)
#define REG_BUCK_VANA_VBIT_DSLEEP(n)        BITFIELD_VAL(REG_BUCK_VANA_VBIT_DSLEEP, n)

// REG_130
#define REG_BUCK_VANA_VREF_SEL_RC_SHIFT     14
#define REG_BUCK_VANA_VREF_SEL_RC_MASK      (0x3 << REG_BUCK_VANA_VREF_SEL_RC_SHIFT)
#define REG_BUCK_VANA_VREF_SEL_RC(n)        BITFIELD_VAL(REG_BUCK_VANA_VREF_SEL_RC, n)
#define REG_BUCK_VANA_IX2_ERR_RC            (1 << 13)
#define REG_BUCK_VANA_BURST_MODE_SEL_RC     (1 << 12)
#define REG_BUCK_VANA_COUNTER_SEL_ULP_RC    (1 << 10)
#define REG_BUCK_VANA_HYSTERESIS_BIT_RC     (1 << 9)
#define REG_BUCK_VANA_VBIT_RC_SHIFT         0
#define REG_BUCK_VANA_VBIT_RC_MASK          (0x1FF << REG_BUCK_VANA_VBIT_RC_SHIFT)
#define REG_BUCK_VANA_VBIT_RC(n)            BITFIELD_VAL(REG_BUCK_VANA_VBIT_RC, n)

// REG_131
#define REG_BUCK_VCORE_RAMP_STEP_SHIFT      4
#define REG_BUCK_VCORE_RAMP_STEP_MASK       (0x1F << REG_BUCK_VCORE_RAMP_STEP_SHIFT)
#define REG_BUCK_VCORE_RAMP_STEP(n)         BITFIELD_VAL(REG_BUCK_VCORE_RAMP_STEP, n)
#define REG_BUCK_VCORE_RAMP_EN              (1 << 3)
#define REG_BUCK_VCORE_DSLEEP_MSK           (1 << 2)
#define REG_PU_BUCK_VCORE_DR                (1 << 1)
#define REG_PU_BUCK_VCORE                   (1 << 0)

// REG_132
#define REG_BUCK_VCORE_VREF_SEL_NORMAL_SHIFT 14
#define REG_BUCK_VCORE_VREF_SEL_NORMAL_MASK  0x3 << REG_BUCK_VCORE_VREF_SEL_NORMAL_SHIFT)
#define REG_BUCK_VCORE_VREF_SEL_NORMAL(n)    BITFIELD_VAL(REG_BUCK_VCORE_VREF_SEL_NORMAL, n)
#define REG_BUCK_VCORE_IX2_ERR_NORMAL        (1 << 13)
#define REG_BUCK_VCORE_BURST_MODE_SEL_NORMAL (1 << 12)
#define REG_BUCK_VCORE_COUNTER_SEL_ULP_NORMAL (1 << 10)
#define REG_BUCK_VCORE_HYSTERESIS_BIT_NORMAL (1 << 9)
#define REG_BUCK_VCORE_VBIT_NORMAL_SHIFT     0
#define REG_BUCK_VCORE_VBIT_NORMAL_MASK      (0x1FF << REG_BUCK_VCORE_VBIT_NORMAL_SHIFT)
#define REG_BUCK_VCORE_VBIT_NORMAL(n)        BITFIELD_VAL(REG_BUCK_VCORE_VBIT_NORMAL, n)

// REG_133
#define REG_BUCK_VCORE_VREF_SEL_DSLEEP_SHIFT 14
#define REG_BUCK_VCORE_VREF_SEL_DSLEEP_MASK  (0x3 << REG_BUCK_VCORE_VREF_SEL_DSLEEP_SHIFT)
#define REG_BUCK_VCORE_VREF_SEL_DSLEEP(n)    BITFIELD_VAL(REG_BUCK_VCORE_VREF_SEL_DSLEEP, n)
#define REG_BUCK_VCORE_IX2_ERR_DSLEEP        (1 << 13)
#define REG_BUCK_VCORE_BURST_MODE_SEL_DSLEEP (1 << 12)
#define REG_BUCK_VCORE_COUNTER_SEL_ULP_DSLEEP (1 << 10)
#define REG_BUCK_VCORE_HYSTERESIS_BIT_DSLEEP (1 << 9)
#define REG_BUCK_VCORE_VBIT_DSLEEP_SHIFT     0
#define REG_BUCK_VCORE_VBIT_DSLEEP_MASK      (0x1FF << REG_BUCK_VCORE_VBIT_DSLEEP_SHIFT)
#define REG_BUCK_VCORE_VBIT_DSLEEP(n)        BITFIELD_VAL(REG_BUCK_VCORE_VBIT_DSLEEP, n)

// REG_134
#define REG_BUCK_VCORE_VREF_SEL_RC_SHIFT    14
#define REG_BUCK_VCORE_VREF_SEL_RC_MASK     (0x3 << REG_BUCK_VCORE_VREF_SEL_RC_SHIFT)
#define REG_BUCK_VCORE_VREF_SEL_RC(n)       BITFIELD_VAL(REG_BUCK_VCORE_VREF_SEL_RC, n)
#define REG_BUCK_VCORE_IX2_ERR_RC           (1 << 13)
#define REG_BUCK_VCORE_BURST_MODE_SEL_RC    (1 << 12)
#define REG_BUCK_VCORE_COUNTER_SEL_ULP_RC   (1 << 10)
#define REG_BUCK_VCORE_HYSTERESIS_BIT_RC    (1 << 9)
#define REG_BUCK_VCORE_VBIT_RC_SHIFT        0
#define REG_BUCK_VCORE_VBIT_RC_MASK         (0x1FF << REG_BUCK_VCORE_VBIT_RC_SHIFT)
#define REG_BUCK_VCORE_VBIT_RC(n)           BITFIELD_VAL(REG_BUCK_VCORE_VBIT_RC, n)

// REG_135
#define REG_BUCK_VHPPA_RAMP_STEP_SHIFT      4
#define REG_BUCK_VHPPA_RAMP_STEP_MASK       (0x1F << REG_BUCK_VHPPA_RAMP_STEP_SHIFT)
#define REG_BUCK_VHPPA_RAMP_STEP(n)         BITFIELD_VAL(REG_BUCK_VHPPA_RAMP_STEP, n)
#define REG_BUCK_VHPPA_RAMP_EN              (1 << 3)
#define REG_BUCK_VHPPA_DSLEEP_MSK           (1 << 2)
#define REG_PU_BUCK_VHPPA_DR                (1 << 1)
#define REG_PU_BUCK_VHPPA                   (1 << 0)

// REG_136
#define REG_BUCK_VHPPA_VREF_SEL_NORMAL_SHIFT 14
#define REG_BUCK_VHPPA_VREF_SEL_NORMAL_MASK  (0x3 << REG_BUCK_VHPPA_VREF_SEL_NORMAL_SHIFT)
#define REG_BUCK_VHPPA_VREF_SEL_NORMAL(n)    BITFIELD_VAL(REG_BUCK_VHPPA_VREF_SEL_NORMAL, n)
#define REG_BUCK_VHPPA_IX2_ERR_NORMAL        (1 << 13)
#define REG_BUCK_VHPPA_BURST_MODE_SEL_NORMAL (1 << 12)
#define REG_BUCK_VHPPA_COUNTER_SEL_ULP_NORMAL (1 << 10)
#define REG_BUCK_VHPPA_HYSTERESIS_BIT_NORMAL (1 << 9)
#define REG_BUCK_VHPPA_VBIT_NORMAL_SHIFT     0
#define REG_BUCK_VHPPA_VBIT_NORMAL_MASK      (0x1FF << REG_BUCK_VHPPA_VBIT_NORMAL_SHIFT)
#define REG_BUCK_VHPPA_VBIT_NORMAL(n)        BITFIELD_VAL(REG_BUCK_VHPPA_VBIT_NORMAL, n)

// REG_137
#define REG_BUCK_VHPPA_VREF_SEL_DSLEEP_SHIFT 14
#define REG_BUCK_VHPPA_VREF_SEL_DSLEEP_MASK  (0x3 << REG_BUCK_VHPPA_VREF_SEL_DSLEEP_SHIFT)
#define REG_BUCK_VHPPA_VREF_SEL_DSLEEP(n)    BITFIELD_VAL(REG_BUCK_VHPPA_VREF_SEL_DSLEEP, n)
#define REG_BUCK_VHPPA_IX2_ERR_DSLEEP        (1 << 13)
#define REG_BUCK_VHPPA_BURST_MODE_SEL_DSLEEP (1 << 12)
#define REG_BUCK_VHPPA_COUNTER_SEL_ULP_DSLEEP (1 << 10)
#define REG_BUCK_VHPPA_HYSTERESIS_BIT_DSLEEP (1 << 9)
#define REG_BUCK_VHPPA_VBIT_DSLEEP_SHIFT     0
#define REG_BUCK_VHPPA_VBIT_DSLEEP_MASK      (0x1FF << REG_BUCK_VHPPA_VBIT_DSLEEP_SHIFT)
#define REG_BUCK_VHPPA_VBIT_DSLEEP(n)        BITFIELD_VAL(REG_BUCK_VHPPA_VBIT_DSLEEP, n)

// REG_138
#define REG_BUCK_VHPPA_VREF_SEL_RC_SHIFT    14
#define REG_BUCK_VHPPA_VREF_SEL_RC_MASK     (0x3 << REG_BUCK_VHPPA_VREF_SEL_RC_SHIFT)
#define REG_BUCK_VHPPA_VREF_SEL_RC(n)       BITFIELD_VAL(REG_BUCK_VHPPA_VREF_SEL_RC, n)
#define REG_BUCK_VHPPA_IX2_ERR_RC           (1 << 13)
#define REG_BUCK_VHPPA_BURST_MODE_SEL_RC    (1 << 12)
#define REG_BUCK_VHPPA_COUNTER_SEL_ULP_RC   (1 << 10)
#define REG_BUCK_VHPPA_HYSTERESIS_BIT_RC    (1 << 9)
#define REG_BUCK_VHPPA_VBIT_RC_SHIFT        0
#define REG_BUCK_VHPPA_VBIT_RC_MASK         (0x1FF << REG_BUCK_VHPPA_VBIT_RC_SHIFT)
#define REG_BUCK_VHPPA_VBIT_RC(n)           BITFIELD_VAL(REG_BUCK_VHPPA_VBIT_RC, n)

// Swap buck_xxx_ix2_err/counter_sel/hysteresis rc and dsleep

// REG_139
#define REG_BUCK_VANA_IS_GAIN_NORMAL_SHIFT  1
#define REG_BUCK_VANA_IS_GAIN_NORMAL_MASK   (0x7 << REG_BUCK_VANA_IS_GAIN_NORMAL_SHIFT)
#define REG_BUCK_VANA_IS_GAIN_NORMAL(n)     BITFIELD_VAL(REG_BUCK_VANA_IS_GAIN_NORMAL, n)
#define REG_BUCK_VANA_ULP_MODE_NORMAL       (1 << 0)

// REG_13A
#define REG_BUCK_VANA_IS_GAIN_DSLEEP_SHIFT  1
#define REG_BUCK_VANA_IS_GAIN_DSLEEP_MASK   (0x7 << REG_BUCK_VANA_IS_GAIN_DSLEEP_SHIFT)
#define REG_BUCK_VANA_IS_GAIN_DSLEEP(n)     BITFIELD_VAL(REG_BUCK_VANA_IS_GAIN_DSLEEP, n)
#define REG_BUCK_VANA_ULP_MODE_DSLEEP       (1 << 0)

// REG_140
#define REG_BUCK_VANA_IS_GAIN_RC_SHIFT      1
#define REG_BUCK_VANA_IS_GAIN_RC_MASK       (0x7 << REG_BUCK_VANA_IS_GAIN_RC_SHIFT)
#define REG_BUCK_VANA_IS_GAIN_RC(n)         BITFIELD_VAL(REG_BUCK_VANA_IS_GAIN_RC, n)
#define REG_BUCK_VANA_ULP_MODE_RC           (1 << 0)

// REG_141
#define REG_BUCK_VCORE_IS_GAIN_NORMAL_SHIFT 1
#define REG_BUCK_VCORE_IS_GAIN_NORMAL_MASK  (0x7 << REG_BUCK_VCORE_IS_GAIN_NORMAL_SHIFT)
#define REG_BUCK_VCORE_IS_GAIN_NORMAL(n)    BITFIELD_VAL(REG_BUCK_VCORE_IS_GAIN_NORMAL, n)
#define REG_BUCK_VCORE_ULP_MODE_NORMAL      (1 << 0)

// REG_142
#define REG_BUCK_VCORE_IS_GAIN_DSLEEP_SHIFT 1
#define REG_BUCK_VCORE_IS_GAIN_DSLEEP_MASK  (0x7 << REG_BUCK_VCORE_IS_GAIN_DSLEEP_SHIFT)
#define REG_BUCK_VCORE_IS_GAIN_DSLEEP(n)    BITFIELD_VAL(REG_BUCK_VCORE_IS_GAIN_DSLEEP, n)
#define REG_BUCK_VCORE_ULP_MODE_DSLEEP      (1 << 0)

// REG_143
#define REG_BUCK_VCORE_IS_GAIN_RC_SHIFT     1
#define REG_BUCK_VCORE_IS_GAIN_RC_MASK      (0x7 << REG_BUCK_VCORE_IS_GAIN_RC_SHIFT)
#define REG_BUCK_VCORE_IS_GAIN_RC(n)        BITFIELD_VAL(REG_BUCK_VCORE_IS_GAIN_RC, n)
#define REG_BUCK_VCORE_ULP_MODE_RC          (1 << 0)

// REG_144
#define REG_BUCK_VHPPA_IS_GAIN_NORMAL_SHIFT 1
#define REG_BUCK_VHPPA_IS_GAIN_NORMAL_MASK  (0x7 << REG_BUCK_VHPPA_IS_GAIN_NORMAL_SHIFT)
#define REG_BUCK_VHPPA_IS_GAIN_NORMAL(n)    BITFIELD_VAL(REG_BUCK_VHPPA_IS_GAIN_NORMAL, n)
#define REG_BUCK_VHPPA_ULP_MODE_NORMAL      (1 << 0)

// REG_145
#define REG_BUCK_VHPPA_IS_GAIN_DSLEEP_SHIFT 1
#define REG_BUCK_VHPPA_IS_GAIN_DSLEEP_MASK  (0x7 << REG_BUCK_VHPPA_IS_GAIN_DSLEEP_SHIFT)
#define REG_BUCK_VHPPA_IS_GAIN_DSLEEP(n)    BITFIELD_VAL(REG_BUCK_VHPPA_IS_GAIN_DSLEEP, n)
#define REG_BUCK_VHPPA_ULP_MODE_DSLEEP      (1 << 0)

// REG_146
#define REG_BUCK_VHPPA_IS_GAIN_RC_SHIFT     1
#define REG_BUCK_VHPPA_IS_GAIN_RC_MASK      (0x7 << REG_BUCK_VHPPA_IS_GAIN_RC_SHIFT)
#define REG_BUCK_VHPPA_IS_GAIN_RC(n)        BITFIELD_VAL(REG_BUCK_VHPPA_IS_GAIN_RC, n)
#define REG_BUCK_VHPPA_ULP_MODE_RC          (1 << 0)

// REG_147
#define REG_EFUSE_CLK_OSC_SEL               (1 << 14)
#define REG_EFUSE_AUTO_READ                 (1 << 13)
#define REG_BASE_ADDR_SHIFT                 8
#define REG_BASE_ADDR_MASK                  (0x1F << REG_BASE_ADDR_SHIFT)
#define REG_BASE_ADDR(n)                    BITFIELD_VAL(REG_BASE_ADDR, n)
#define REG_ADDR_INC_SHIFT                  4
#define REG_ADDR_INC_MASK                   (0xF << REG_ADDR_INC_SHIFT)
#define REG_ADDR_INC(n)                     BITFIELD_VAL(REG_ADDR_INC, n)
#define REG_SQ_TIME_SHIFT                   0
#define REG_SQ_TIME_MASK                    (0xF << REG_SQ_TIME_SHIFT)
#define REG_SQ_TIME(n)                      BITFIELD_VAL(REG_SQ_TIME, n)

// REG_148
#define AUTO_ADDR0_DATA_SHIFT               0
#define AUTO_ADDR0_DATA_MASK                (0xFFFF << AUTO_ADDR0_DATA_SHIFT)
#define AUTO_ADDR0_DATA(n)                  BITFIELD_VAL(AUTO_ADDR0_DATA, n)

// REG_149
#define AUTO_ADDR1_DATA_SHIFT               0
#define AUTO_ADDR1_DATA_MASK                (0xFFFF << AUTO_ADDR1_DATA_SHIFT)
#define AUTO_ADDR1_DATA(n)                  BITFIELD_VAL(AUTO_ADDR1_DATA, n)

// REG_14A
#define AUTO_ADDR2_DATA_SHIFT               0
#define AUTO_ADDR2_DATA_MASK                (0xFFFF << AUTO_ADDR2_DATA_SHIFT)
#define AUTO_ADDR2_DATA(n)                  BITFIELD_VAL(AUTO_ADDR2_DATA, n)

// REG_14B
#define AUTO_ADDR3_DATA_SHIFT               0
#define AUTO_ADDR3_DATA_MASK                (0xFFFF << AUTO_ADDR3_DATA_SHIFT)
#define AUTO_ADDR3_DATA(n)                  BITFIELD_VAL(AUTO_ADDR3_DATA, n)

// REG_14C
#define AUTO_ADDR4_DATA_SHIFT               0
#define AUTO_ADDR4_DATA_MASK                (0xFFFF << AUTO_ADDR4_DATA_SHIFT)
#define AUTO_ADDR4_DATA(n)                  BITFIELD_VAL(AUTO_ADDR4_DATA, n)

// REG_14D
#define AUTO_ADDR5_DATA_SHIFT               0
#define AUTO_ADDR5_DATA_MASK                (0xFFFF << AUTO_ADDR5_DATA_SHIFT)
#define AUTO_ADDR5_DATA(n)                  BITFIELD_VAL(AUTO_ADDR5_DATA, n)

// REG_14E
#define AUTO_ADDR6_DATA_SHIFT               0
#define AUTO_ADDR6_DATA_MASK                (0xFFFF << AUTO_ADDR6_DATA_SHIFT)
#define AUTO_ADDR6_DATA(n)                  BITFIELD_VAL(AUTO_ADDR6_DATA, n)

// REG_14F
#define AUTO_ADDR7_DATA_SHIFT               0
#define AUTO_ADDR7_DATA_MASK                (0xFFFF << AUTO_ADDR7_DATA_SHIFT)
#define AUTO_ADDR7_DATA(n)                  BITFIELD_VAL(AUTO_ADDR7_DATA, n)

// REG_150
#define AUTO_ADDR8_DATA_SHIFT               0
#define AUTO_ADDR8_DATA_MASK                (0xFFFF << AUTO_ADDR8_DATA_SHIFT)
#define AUTO_ADDR8_DATA(n)                  BITFIELD_VAL(AUTO_ADDR8_DATA, n)

// REG_151
#define AUTO_ADDR9_DATA_SHIFT               0
#define AUTO_ADDR9_DATA_MASK                (0xFFFF << AUTO_ADDR9_DATA_SHIFT)
#define AUTO_ADDR9_DATA(n)                  BITFIELD_VAL(AUTO_ADDR9_DATA, n)

// REG_152
#define AUTO_ADDRA_DATA_SHIFT               0
#define AUTO_ADDRA_DATA_MASK                (0xFFFF << AUTO_ADDRA_DATA_SHIFT)
#define AUTO_ADDRA_DATA(n)                  BITFIELD_VAL(AUTO_ADDRA_DATA, n)

// REG_153
#define AUTO_ADDRB_DATA_SHIFT               0
#define AUTO_ADDRB_DATA_MASK                (0xFFFF << AUTO_ADDRB_DATA_SHIFT)
#define AUTO_ADDRB_DATA(n)                  BITFIELD_VAL(AUTO_ADDRB_DATA, n)

// REG_154
#define AUTO_ADDRC_DATA_SHIFT               0
#define AUTO_ADDRC_DATA_MASK                (0xFFFF << AUTO_ADDRC_DATA_SHIFT)
#define AUTO_ADDRC_DATA(n)                  BITFIELD_VAL(AUTO_ADDRC_DATA, n)

// REG_155
#define AUTO_ADDRD_DATA_SHIFT               0
#define AUTO_ADDRD_DATA_MASK                (0xFFFF << AUTO_ADDRD_DATA_SHIFT)
#define AUTO_ADDRD_DATA(n)                  BITFIELD_VAL(AUTO_ADDRD_DATA, n)

// REG_156
#define AUTO_ADDRE_DATA_SHIFT               0
#define AUTO_ADDRE_DATA_MASK                (0xFFFF << AUTO_ADDRE_DATA_SHIFT)
#define AUTO_ADDRE_DATA(n)                  BITFIELD_VAL(AUTO_ADDRE_DATA, n)

// REG_157
#define AUTO_ADDRF_DATA_SHIFT               0
#define AUTO_ADDRF_DATA_MASK                (0xFFFF << AUTO_ADDRF_DATA_SHIFT)
#define AUTO_ADDRF_DATA(n)                  BITFIELD_VAL(AUTO_ADDRF_DATA, n)

// REG_158
#define EFUSE_FSM_SHIFT                     0
#define EFUSE_FSM_MASK                      (0xF << EFUSE_FSM_SHIFT)
#define EFUSE_FSM(n)                        BITFIELD_VAL(EFUSE_FSM, n)

#endif

