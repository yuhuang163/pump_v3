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
#include "pmu.h"
#include CHIP_SPECIFIC_HDR(reg_pmu)
#include CHIP_SPECIFIC_HDR(rf_xtal)
#include CHIP_SPECIFIC_HDR(charger)
#include "analog.h"
#include "cmsis.h"
#include "cmsis_nvic.h"
#include "hal_aud.h"
#include "hal_bootmode.h"
#include "hal_cache.h"
#include "hal_chipid.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "patch.h"
#include "tgt_hardware.h"

#define PMU_EFUSE_INIT_IN_BOOT
#define VRTC_SWITCH_TO_VCORE

#ifdef RC_FAST_WAKEUP
#ifndef RC_CLK_ENABLE
#error "Invalid config, RC_FAST_WAKEUP need define RC_CLK_ENABLE!"
#endif
// interface need 3us, analog circuit need 7us
#define FAST_HW_RAMP
#endif

#ifdef RC_CLK_ENABLE
#ifndef PMU_FORCE_LP_MODE
#error "Invalid config, RC_CLK_ENABLE need define PMU_FORCE_LP_MODE!"
#endif
#else
#define DCDC_ULP_RC_ON
#endif

#if !defined(DIG_DCDC_MODE) && !defined(PMU_LDO_VCORE_CALIB)
#define PMU_LDO_VCORE_CALIB
#endif

#if defined(PSRAM_ENABLE) || defined(PMU_ALREADY_INIT)
#define PMU_OPEN_BSS_LOC                BOOT_BSS_LOC
#else
#define PMU_OPEN_BSS_LOC
#endif

#ifdef PMU_IRQ_UNIFIED
#define PMU_IRQ_HDLR_PARAM              uint16_t irq_status
#else
#define PMU_IRQ_HDLR_PARAM              void
#endif

//#define PMU_VCORE_RC_MODE_FIXED

#define ana_read(reg,val)               hal_analogif_reg_read(ANA_REG(reg),val)
#define ana_write(reg,val)              hal_analogif_reg_write(ANA_REG(reg),val)
#define rf_read(reg,val)                hal_analogif_reg_read(RF_REG(reg),val)
#define rf_write(reg,val)               hal_analogif_reg_write(RF_REG(reg),val)
#define chg_read(reg,val)               hal_analogif_reg_read(CHG_REG(reg),val)
#define chg_write(reg,val)              hal_analogif_reg_write(CHG_REG(reg),val)

// LDO soft start interval is about 1000 us
#define PMU_LDO_PU_STABLE_TIME_US       1800
#define PMU_LDO_PULLDOWN_STABLE_TIME_US 1000
#define PMU_LDO_VRTC_STABLE_TIME_US     1000
#define PMU_DCDC_PU_STABLE_TIME_US      100
#define PMU_DCDC_HW_RAMP_1_CLK_TIME_US  40
#define PMU_HW_RAMP_STEP_PER_CYCLE      16
#define PMU_HW_RAMP_STEP_HALF_PER_CYCLE (PMU_HW_RAMP_STEP_PER_CYCLE / 2)
#ifdef FAST_HW_RAMP
#define PMU_F_HW_RAMP_STEP_PER_CYCLE    2
#endif
#define PMU_VANA_STABLE_TIME_US         10
#define PMU_VCORE_STABLE_TIME_US        10
#define PMU_BIG_BG_STABLE_TIME_US       200

// External 32.768KHz stable time(ms)
#define PMU_EXT_32K_STABLE_TIME         300
#define PMU_EXT_32K_CLK_OUT_STABLE_TIME 1
#define PMU_CLK_SWITCH_STABLE_TIME      1
#define PMU_EXT_32K_CAPBIT_DEFAULT      0x141

// 2mV
#define PMU_DCDC_DIG_1_1V               0x16A
#define PMU_DCDC_DIG_1_05V              0x151
#define PMU_DCDC_DIG_1_0V               0x138
#define PMU_DCDC_DIG_0_95V              0x11F
#define PMU_DCDC_DIG_0_9V               0x106
#define PMU_DCDC_DIG_0_85V              0xED
#define PMU_DCDC_DIG_0_84V              0xE8
#define PMU_DCDC_DIG_0_8V               0xD4
#define PMU_DCDC_DIG_0_75V              0xBB
#define PMU_DCDC_DIG_0_74V              0xB6
#define PMU_DCDC_DIG_0_7V               0xA2
#define PMU_DCDC_DIG_0_65V              0x89
#define PMU_DCDC_DIG_0_6V               0x70
#define PMU_DCDC_DIG_0_55V              0x57
#define PMU_DCDC_DIG_0_5V               0x3E
#define PMU_DCDC_DIG_50_MV              (PMU_DCDC_DIG_0_85V - PMU_DCDC_DIG_0_8V)
#define PMU_DCDC_DIG_DEFAULT            0x140

// 4mV
#define PMU_DCDC_ANA_1_4V               0xBC
#define PMU_DCDC_ANA_1_3V               0xA3
#define PMU_DCDC_ANA_1_25V              0x97
#define PMU_DCDC_ANA_1_2V               0x8A
#define PMU_DCDC_ANA_1_1V               0x74
#define PMU_DCDC_ANA_1_0V               0x58

// 4mV
#define PMU_DCDC_HPPA_1_9V              0x138
#define PMU_DCDC_HPPA_1_8V              0x11F
#define PMU_DCDC_HPPA_1_75V             0x113
#define PMU_DCDC_HPPA_1_7V              0x106
#define PMU_DCDC_HPPA_1_65V             0xFA
#define PMU_DCDC_HPPA_1_6V              0xEE
#define PMU_DCDC_HPPA_1_55V             0xE1
#define PMU_DCDC_HPPA_1_5V              0xD5
#define PMU_DCDC_HPPA_1_45V             0xC8
#define PMU_DCDC_HPPA_1_4V              0xBC

#define PMU_VDIG_1_1V                   0x9
#define PMU_VDIG_1_05V                  0x8
#define PMU_VDIG_1_0V                   0x7
#define PMU_VDIG_0_95V                  0x6
#define PMU_VDIG_0_9V                   0x5
#define PMU_VDIG_0_85V                  0x4
#define PMU_VDIG_0_8V                   0x3
#define PMU_VDIG_0_75V                  0x2
#define PMU_VDIG_0_7V                   0x1
#define PMU_VDIG_0_65V                  0x0
#define PMU_VDIG_DEFAULT                0x4 /* 0.85V */

#define PMU_VDIG_L_1_05V                0xEF
#define PMU_VDIG_L_1_0V                 0xDE
#define PMU_VDIG_L_0_95V                0xCD
#define PMU_VDIG_L_0_9V                 0xBC
#define PMU_VDIG_L_0_85V                0xAB
#define PMU_VDIG_L_0_8V                 0x9A
#define PMU_VDIG_L_0_75V                0x89
#define PMU_VDIG_L_0_7V                 0x78
#define PMU_VDIG_L_0_65V                0x67
#define PMU_VDIG_L_0_6V                 0x56
#define PMU_VDIG_L_0_55V                0x45
#define PMU_VDIG_L_0_5V                 0x34
#define PMU_VDIG_L_0_45V                0x23
#define PMU_VDIG_L_DEFAULT              0x64

#define PMU_LDO_SENSOR_3_3V             0x28
#define PMU_LDO_SENSOR_3_2V             0x26
#define PMU_LDO_SENSOR_3_1V             0x24
#define PMU_LDO_SENSOR_3_0V             0x22
#define PMU_LDO_SENSOR_2_9V             0x20
#define PMU_LDO_SENSOR_2_8V             0x1E
#define PMU_LDO_SENSOR_2_7V             0x1C
#define PMU_LDO_SENSOR_2_6V             0x1B
#define PMU_LDO_SENSOR_2_5V             0x19
#define PMU_LDO_SENSOR_2_4V             0x17
#define PMU_LDO_SENSOR_2_3V             0x15
#define PMU_LDO_SENSOR_2_2V             0x13
#define PMU_LDO_SENSOR_2_1V             0x11
#define PMU_LDO_SENSOR_2_0V             0xF
#define PMU_LDO_SENSOR_1_9V             0xD
#define PMU_LDO_SENSOR_1_8V             0xB

#define PMU_LDO_VRTC_0_825V             0xF
#define PMU_LDO_VRTC_0_8V               0xE
#define PMU_LDO_VRTC_0_775V             0xD
#define PMU_LDO_VRTC_0_75V              0xC
#define PMU_LDO_VRTC_0_725V             0xB
#define PMU_LDO_VRTC_0_7V               0xA
#define PMU_LDO_VRTC_0_675V             0x9
#define PMU_LDO_VRTC_0_65V              0x8
#define PMU_LDO_VRTC_0_625V             0x7
#define PMU_LDO_VRTC_0_6V               0x6
#define PMU_LDO_VRTC_0_575V             0x5
#define PMU_LDO_VRTC_0_55V              0x4
#define PMU_LDO_VRTC_0_525V             0x3
#define PMU_LDO_VRTC_0_5V               0x2
#define PMU_LDO_VRTC_0_475V             0x1
#define PMU_LDO_VRTC_0_45V              0x0
#define PMU_LDO_VRTC_DEFAULT_ACT        0xE
#define PMU_LDO_VRTC_DEFAULT_LP         0x8

#define PMU_VMIC_2_8V                   0x33
#define PMU_VMIC_2_7V                   0x30
#define PMU_VMIC_2_6V                   0x2D
#define PMU_VMIC_1_6V                   0x13

#define PMU_VMIC_RES_3_3V               0xF
#define PMU_VMIC_RES_2_8V               0xA

#define PMU_CODEC_1_9V                  0x7
#define PMU_CODEC_1_8V                  0x6
#define PMU_CODEC_1_7V                  0x5
#define PMU_CODEC_1_6V                  0x4
#define PMU_CODEC_DEFAULT_V             0x6

#define PMU_VMEM_1_8V                   0xC
#define PMU_VMEM_1_7V                   0xA
#define PMU_VMEM_1_6V                   0x8
#define PMU_VMEM_DEFAULT_V              0xC

#define PMU_USB_3_3V                    0xC
#define PMU_USB_3_2V                    0xB
#define PMU_USB_3_1V                    0xA
#define PMU_USB_3_0V                    0x9
#define PMU_USB_2_9V                    0x8
#define PMU_USB_2_8V                    0x7
#define PMU_USB_2_7V                    0x6
#define PMU_USB_2_6V                    0x5
#define PMU_USB_2_5V                    0x4
#define PMU_USB_2_4V                    0x3

#define MAX_BUCK_VANA_BIT_VAL                   (REG_BUCK_VANA_VBIT_NORMAL_MASK >> REG_BUCK_VANA_VBIT_NORMAL_SHIFT)
#define MAX_BUCK_VCORE_BIT_VAL                  (REG_BUCK_VCORE_VBIT_NORMAL_MASK >> REG_BUCK_VCORE_VBIT_NORMAL_SHIFT)
#define MAX_BUCK_VHPPA_BIT_VAL                  (REG_BUCK_VHPPA_VBIT_NORMAL_MASK >> REG_BUCK_VHPPA_VBIT_NORMAL_SHIFT)
#define MAX_LDO_VCORE_L_BIT_VAL                 (REG_LDO_VCORE_L_VBIT_NORMAL_MASK >> REG_LDO_VCORE_L_VBIT_NORMAL_SHIFT)

#define INTR_MSKED_CHARGE_OUT                   INTR_MSKED_CHARGE(1 << 1)
#define INTR_MSKED_CHARGE_IN                    INTR_MSKED_CHARGE(1 << 0)

#define REG_DCDC1_RAMP_EN                       (1 << 1)
#define REG_DCDC2_RAMP_EN                       (1 << 2)
#define REG_DCDC3_RAMP_EN                       (1 << 3)
#define REG_VCORE_L_RAMP_EN                     (1 << 4)

#define CHIP_REV_ID_SHIFT                       0
#define CHIP_REV_ID_MASK                        (0xF << CHIP_REV_ID_SHIFT)
#define CHIP_REV_ID(n)                          BITFIELD_VAL(CHIP_REV_ID, n)

// PMU_VMIN_SEARCH
#define PMU_VMIN_SET_REG_ADDR                   PMU_REG_RTC_LOAD_MIN_SEC_EN
#define PMU_VMIN_VCORE_SET_DONE                 (1 << 15)
#define PMU_VMIN_VCORE_SET_PLUS                 (1 << 14)
#define PMU_VMIN_VCORE_SET_OFFSET_SHIFT         0
#define PMU_VMIN_VCORE_SET_OFFSET_MASK          (0x1F << PMU_VMIN_VCORE_SET_OFFSET_SHIFT)
#define PMU_VMIN_VCORE_SET_OFFSET(n)            BITFIELD_VAL(PMU_VMIN_VCORE_SET_OFFSET, n)

// PMU_EFUSE_PAGE_DCDC1_VOLT
#define PMU_EFUSE_DCDC1_VOLT_COMP_SHIFT         0
#define PMU_EFUSE_DCDC1_VOLT_COMP_MASK          (0x7F << PMU_EFUSE_DCDC1_VOLT_COMP_SHIFT)
#define PMU_EFUSE_DCDC1_VOLT_COMP(n)            BITFIELD_VAL(PMU_EFUSE_DCDC1_VOLT_COMP, n)
#define PMU_EFUSE_DCDC1_VOLT_COMP_SIGN          (1 << 7)
#define PMU_EFUSE_DCDC1_VOLT_LP_COMP_SHIFT      8
#define PMU_EFUSE_DCDC1_VOLT_LP_COMP_MASK       (0x7F << PMU_EFUSE_DCDC1_VOLT_LP_COMP_SHIFT)
#define PMU_EFUSE_DCDC1_VOLT_LP_COMP(n)         BITFIELD_VAL(PMU_EFUSE_DCDC1_VOLT_LP_COMP, n)
#define PMU_EFUSE_DCDC1_VOLT_LP_COMP_SIGN       (1 << 15)

// PMU_EFUSE_PAGE_DCDC1_VOLT_NEW
#define PMU_EFUSE_DCDC1_VOLT_CAL_NEW            (1 << 8)

// PMU_EFUSE_PAGE_DCDC2_VOLT
#define PMU_EFUSE_DCDC2_VOLT_COMP_SHIFT         0
#define PMU_EFUSE_DCDC2_VOLT_COMP_MASK          (0x3F << PMU_EFUSE_DCDC2_VOLT_COMP_SHIFT)
#define PMU_EFUSE_DCDC2_VOLT_COMP(n)            BITFIELD_VAL(PMU_EFUSE_DCDC2_VOLT_COMP, n)
#define PMU_EFUSE_DCDC2_VOLT_COMP_SIGN          (1 << 6)
#define PMU_EFUSE_DCDC2_VOLT_LP_COMP_SHIFT      7
#define PMU_EFUSE_DCDC2_VOLT_LP_COMP_MASK       (0x3F << PMU_EFUSE_DCDC2_VOLT_LP_COMP_SHIFT)
#define PMU_EFUSE_DCDC2_VOLT_LP_COMP(n)         BITFIELD_VAL(PMU_EFUSE_DCDC2_VOLT_LP_COMP, n)
#define PMU_EFUSE_DCDC2_VOLT_LP_COMP_SIGN       (1 << 13)

// PMU_EFUSE_PAGE_DCDC3_VOLT
#define PMU_EFUSE_DCDC3_VOLT_COMP_SHIFT         0
#define PMU_EFUSE_DCDC3_VOLT_COMP_MASK          (0x3F << PMU_EFUSE_DCDC3_VOLT_COMP_SHIFT)
#define PMU_EFUSE_DCDC3_VOLT_COMP(n)            BITFIELD_VAL(PMU_EFUSE_DCDC3_VOLT_COMP, n)
#define PMU_EFUSE_DCDC3_VOLT_COMP_SIGN          (1 << 6)
#define PMU_EFUSE_DCDC3_VOLT_LP_COMP_SHIFT      7
#define PMU_EFUSE_DCDC3_VOLT_LP_COMP_MASK       (0x3F << PMU_EFUSE_DCDC3_VOLT_LP_COMP_SHIFT)
#define PMU_EFUSE_DCDC3_VOLT_LP_COMP(n)         BITFIELD_VAL(PMU_EFUSE_DCDC3_VOLT_LP_COMP, n)
#define PMU_EFUSE_DCDC3_VOLT_LP_COMP_SIGN       (1 << 13)

// PMU_EFUSE_PAGE_LDO_CORE_L_VOLT
#define PMU_EFUSE_LDO_CORE_L_VOLT_COMP_SHIFT    0
#define PMU_EFUSE_LDO_CORE_L_VOLT_COMP_MASK     (0x3F << PMU_EFUSE_LDO_CORE_L_VOLT_COMP_SHIFT)
#define PMU_EFUSE_LDO_CORE_L_VOLT_COMP(n)       BITFIELD_VAL(PMU_EFUSE_LDO_CORE_L_VOLT_COMP, n)
#define PMU_EFUSE_LDO_CORE_L_VOLT_COMP_SIGN     (1 << 6)
#define PMU_EFUSE_LDO_CORE_L_VOLT_LP_COMP_SHIFT 7
#define PMU_EFUSE_LDO_CORE_L_VOLT_LP_COMP_MASK  (0x3F << PMU_EFUSE_LDO_CORE_L_VOLT_LP_COMP_SHIFT)
#define PMU_EFUSE_LDO_CORE_L_VOLT_LP_COMP(n)    BITFIELD_VAL(PMU_EFUSE_LDO_CORE_L_VOLT_LP_COMP, n)
#define PMU_EFUSE_LDO_CORE_L_VOLT_LP_COMP_SIGN  (1 << 13)

// REG_CP_WAFER_CTX_XY
#define PMU_EFUSE_CP_WAFER_CTX_X_SHIFT          0
#define PMU_EFUSE_CP_WAFER_CTX_X_MASK           (0x7F << PMU_EFUSE_CP_WAFER_CTX_X_SHIFT)
#define PMU_EFUSE_CP_WAFER_CTX_X(n)             BITFIELD_VAL(PMU_EFUSE_CP_WAFER_CTX_X, n)
#define PMU_EFUSE_CP_WAFER_CTX_Y_SHIFT          7
#define PMU_EFUSE_CP_WAFER_CTX_Y_MASK           (0x7F << PMU_EFUSE_CP_WAFER_CTX_Y_SHIFT)
#define PMU_EFUSE_CP_WAFER_CTX_Y(n)             BITFIELD_VAL(PMU_EFUSE_CP_WAFER_CTX_Y, n)

// REG_CP_WAFER_CTX_ID
#define PMU_EFUSE_CP_WAFER_CTX_ID_SHIFT         0
#define PMU_EFUSE_CP_WAFER_CTX_ID_MASK          (0x1F << PMU_EFUSE_CP_WAFER_CTX_ID_SHIFT)
#define PMU_EFUSE_CP_WAFER_CTX_ID(n)            BITFIELD_VAL(PMU_EFUSE_CP_WAFER_CTX_ID, n)

// PMU_EFUSE_PAGE_RESERVED_0X14
#define PMU_EFUSE_SAR_ADC_VREF_COMP_SHIFT       0
#define PMU_EFUSE_SAR_ADC_VREF_COMP_MASK        (0x1F << PMU_EFUSE_SAR_ADC_VREF_COMP_SHIFT)
#define PMU_EFUSE_SAR_ADC_VREF_COMP(n)          BITFIELD_VAL(PMU_EFUSE_SAR_ADC_VREF_COMP, n)
#define PMU_EFUSE_SAR_ADC_VREF_COMP_SIGN        (1 << 5)

// RF_REG_F5
#define REG_BT_BBPLL_DIVN_CODEC_SHIFT           0
#define REG_BT_BBPLL_DIVN_CODEC_MASK            (0x1F << REG_BT_BBPLL_DIVN_CODEC_SHIFT)
#define REG_BT_BBPLL_DIVN_CODEC(n)              BITFIELD_VAL(REG_BT_BBPLL_DIVN_CODEC, n)
#define REG_RXCALI_REVERSE                      (1 << 5)
#define REG_SIG_LOSS_SHIFT                      6
#define REG_SIG_LOSS_MASK                       (0xF << REG_SIG_LOSS_SHIFT)
#define REG_SIG_LOSS(n)                         BITFIELD_VAL(REG_SIG_LOSS, n)
#define REG_BT_IPTAT_ISEL_SHIFT                 10
#define REG_BT_IPTAT_ISEL_MASK                  (0x3F << REG_BT_IPTAT_ISEL_SHIFT)
#define REG_BT_IPTAT_ISEL(n)                    BITFIELD_VAL(REG_BT_IPTAT_ISEL, n)

// REG_52D
#define REG_LP_BBPLL_RSTN_DR                    (1 << 0)
#define REG_LP_BBPLL_RSTN                       (1 << 1)
#define REG_LP_BBPLL_CLK_FBC_EDGE               (1 << 2)
#define REG_LP_BBPLL_INT_DEC_SEL_SHIFT          3
#define REG_LP_BBPLL_INT_DEC_SEL_MASK           (0x7 << REG_LP_BBPLL_INT_DEC_SEL_SHIFT)
#define REG_LP_BBPLL_INT_DEC_SEL(n)             BITFIELD_VAL(REG_LP_BBPLL_INT_DEC_SEL, n)
#define REG_LP_BBPLL_DITHER_BYPASS              (1 << 6)
#define REG_LP_BBPLL_PRESCALER_DEL_SEL_SHIFT     7
#define REG_LP_BBPLL_PRESCALER_DEL_SEL_MASK     (0xF << REG_LP_BBPLL_PRESCALER_DEL_SEL_SHIFT)
#define REG_LP_BBPLL_PRESCALER_DEL_SEL(n)       BITFIELD_VAL(REG_LP_BBPLL_PRESCALER_DEL_SEL, n)
#define REG_LP_BBPLL_FREQ_EN_COMB               (1 << 11)
#define REG_LP_BBPLL_TRI_STEP_SEL_SHIFT         12
#define REG_LP_BBPLL_TRI_STEP_SEL_MASK          (0x3 << REG_LP_BBPLL_TRI_STEP_SEL_SHIFT)
#define REG_LP_BBPLL_TRI_STEP_SEL(n)            BITFIELD_VAL(REG_LP_BBPLL_TRI_STEP_SEL, n)
#define REG_LP_BBPLL_TRI_FREQ_SEL_SHIFT         14
#define REG_LP_BBPLL_TRI_FREQ_SEL_MASK          (0x3 << REG_LP_BBPLL_TRI_FREQ_SEL_SHIFT)
#define REG_LP_BBPLL_TRI_FREQ_SEL(n)            BITFIELD_VAL(REG_LP_BBPLL_TRI_FREQ_SEL, n)

// REG_52F
#define REG_LP_BBPLL_SDM_FREQWORD_15_0_SHIFT    0
#define REG_LP_BBPLL_SDM_FREQWORD_15_0_MASK     (0xFFFF << REG_LP_BBPLL_SDM_FREQWORD_15_0_SHIFT)
#define REG_LP_BBPLL_SDM_FREQWORD_15_0(n)       BITFIELD_VAL(REG_LP_BBPLL_SDM_FREQWORD_15_0, n)

// REG_530
#define REG_LP_BBPLL_SDM_FREQWORD_31_16_SHIFT   0
#define REG_LP_BBPLL_SDM_FREQWORD_31_16_MASK    (0xFFFF << REG_LP_BBPLL_SDM_FREQWORD_31_16_SHIFT)
#define REG_LP_BBPLL_SDM_FREQWORD_31_16(n)      BITFIELD_VAL(REG_LP_BBPLL_SDM_FREQWORD_31_16, n)

// REG_531
#define REG_LP_BBPLL_SDM_FREQWORD_34_32_SHIFT   0
#define REG_LP_BBPLL_SDM_FREQWORD_34_32_MASK    (0x7 << REG_LP_BBPLL_SDM_FREQWORD_34_32_SHIFT)
#define REG_LP_BBPLL_SDM_FREQWORD_34_32(n)      BITFIELD_VAL(REG_LP_BBPLL_SDM_FREQWORD_34_32, n)

// REG_534
#define REG_LP_BBPLL_PFD_RST_DR                 1
#define REG_LP_BBPLL_PFD_RST                    2
#define REG_LP_BBPLL_PRECHARGE_DR               14
#define REG_LP_BBPLL_PRECHARGE                  15

// REG_535
#define REG_LP_BBPLL_VCO_SWRC_SHIFT             3
#define REG_LP_BBPLL_VCO_SWRC_MASK              (0x3 << REG_LP_BBPLL_VCO_SWRC_SHIFT)
#define REG_LP_BBPLL_VCO_SWRC(n)                BITFIELD_VAL(REG_LP_BBPLL_VCO_SWRC, n)
#define REG_LP_BBPLL_VCO_SPD_SHIFT              5
#define REG_LP_BBPLL_VCO_SPD_MASK               (0x7 << REG_LP_BBPLL_VCO_SPD_SHIFT)
#define REG_LP_BBPLL_VCO_SPD(n)                 BITFIELD_VAL(REG_LP_BBPLL_VCO_SPD, n)
#define REG_LP_BBPLL_DIG_SWRC_SHIFT             10
#define REG_LP_BBPLL_DIG_SWRC_MASK              (0x7 << REG_LP_BBPLL_DIG_SWRC_SHIFT)
#define REG_LP_BBPLL_DIG_SWRC(n)                BITFIELD_VAL(REG_LP_BBPLL_DIG_SWRC, n)

// REG_537
#define REG_LP_BBPLL_LK_RSTB                    (1 << 0)
#define REG_LP_BBPLL_CLKGEN_RST                 (1 << 1)
#define REG_LP_BBPLL_EN_CLK_USB                 (1 << 2)
#define REG_LP_BBPLL_EN_CLK_CODEC               (1 << 3)
#define REG_LP_BBPLL_DIVN_AD_SHIFT              4
#define REG_LP_BBPLL_DIVN_AD_MASK               (0x1F << REG_LP_BBPLL_DIVN_AD_SHIFT)
#define REG_LP_BBPLL_DIVN_AD(n)                 BITFIELD_VAL(REG_LP_BBPLL_DIVN_AD_SHIFT, n)
#define REG_LP_BBPLL_DIG_CLK_FREQ_SEL           (1 << 9)
#define REG_LP_BBPLL_EN_CLK_DIG                 (1 << 10)
#define REG_LP_BBPLL_EN_CLK_BBADC               (1 << 11)
#define REG_LP_BBPLL_CLKGEN_RST_DR              (1 << 12)
#define REG_LP_BBPLL_SDM_CLK_SEL                (1 << 14)

// ANALOG REG_16D
#define REG_ANALOG_PU_OSC                       (1 << 6)

// ANALOG REG_1AD
#define REG_ANALOG_CLK_SEL_DSI                  (1 << 0)
#define REG_ANALOG_EN_CLK_DSI                   (1 << 1)

#define PMU_VCORE_RC_MODE_FIXED_VDIG            PMU_VDIG_0_7V
#define PMU_VCORE_RC_MODE_FIXED_DCDC_DIG        PMU_DCDC_DIG_0_7V
#define PMU_VCORE_RC_MODE_FIXED_VDIG_L          PMU_VDIG_L_0_65V

#define PMU_EFUSE_TEMPSENSOR_DEFAULT            24948

#define REG_CP_WAFER_CTX_XY                     PMU_EFUSE_PAGE_CP_WAFER_XY_CTX
#define REG_CP_WAFER_CTX_ID                     PMU_EFUSE_PAGE_CP_WAFER_ID

enum RF_ANA_CHG_REG_T {
    CHG_REG_METAL_ID            = 0x00,

    ANA_REG_60                  = 0x60,

    RF_REG_00                   = 0x00,
    RF_REG_F5                   = 0xF5,
    RF_REG_111                  = 0x111,
    RF_REG_113                  = 0x113,
    RF_REG_114                  = 0x114,
    RF_REG_115                  = 0x115,
    RF_REG_11F                  = 0X11F,
    RF_REG_120                  = 0x120,
    RF_REG_122                  = 0x122,
    RF_REG_1F4                  = 0x1F4,
    RF_REG_52D                  = 0x52D,
    RF_REG_52F                  = 0x52F,
    RF_REG_530                  = 0x530,
    RF_REG_531                  = 0x531,
    RF_REG_534                  = 0x534,
    RF_REG_535                  = 0x535,
    RF_REG_537                  = 0x537,
};

enum PMU_VCORE_REQ_T {
    PMU_VCORE_FLASH_WRITE_ENABLED   = (1 << 0),
    PMU_VCORE_FLASH_FREQ_HIGH       = (1 << 1),
    PMU_VCORE_PSRAM_FREQ_HIGH       = (1 << 2),
    PMU_VCORE_PSRAM_FREQ_MEDIUM     = (1 << 3),
    PMU_VCORE_PSRAM_FREQ_MEDIUM_LOW = (1 << 4),
    PMU_VCORE_USB_HS_ENABLED        = (1 << 5),
    PMU_VCORE_SYS_FREQ_MEDIUM_LOW   = (1 << 6),
    PMU_VCORE_SYS_FREQ_MEDIUM       = (1 << 7),
    PMU_VCORE_SYS_FREQ_HIGH         = (1 << 8),
    PMU_VCORE_RS_FREQ_HIGH          = (1 << 9),
    PMU_VCORE_RS_ADC_FREQ_HIGH      = (1 << 10),
    PMU_VCORE_IIR_FREQ_HIGH         = (1 << 11),
    PMU_VCORE_IIR_EQ_FREQ_HIGH      = (1 << 12),
    PMU_VCORE_FIR_FREQ_HIGH         = (1 << 13),
    PMU_VCORE_PMU_RESET             = (1 << 14),
};

union BOOT_SETTINGS_T {
    struct {
        unsigned short usb_dld_dis      :1;
        unsigned short uart_dld_en      :1;
        unsigned short uart_trace_en    :1;
        unsigned short pll_dis          :1;
        unsigned short uart_baud_div2   :1;
        unsigned short sec_freq_div2    :1;
        unsigned short crystal_freq     :2;
        unsigned short timeout_div      :2;
        unsigned short uart_connected   :1;
        unsigned short read_quit_crm    :1;
        unsigned short chksum           :4;
    };
    unsigned short reg;
};

// Before adding a new module, it is necessary to check if pmu_module_set(get)_volt() is compatible.
enum PMU_MODULE_T {
    PMU_ANA,
    PMU_CORE,
    PMU_CODEC,
    PMU_CORE_L,
    PMU_USB,
    PMU_SENSOR,
    PMU_MEM,

    PMU_MODULE_QTY,
};

struct PMU_MODULE_CFG_T {
    unsigned short manual_bit;
    unsigned short ldo_en;
    unsigned short lp_en_dr;
    unsigned short lp_en;
    unsigned short dsleep_mode;
    unsigned short dsleep_v;
    unsigned short dsleep_v_shift;
    unsigned short normal_v;
    unsigned short normal_v_shift;
    unsigned short rc_v;
    unsigned short rc_v_shift;
};

#define PMU_MOD_CFG_VAL(m)              { \
    REG_PU_LDO_V##m##_DR, REG_PU_LDO_V##m##_REG, \
    REG_LP_EN_LDO_V##m##_DR, REG_LP_EN_LDO_V##m##_REG, \
    REG_PU_LDO_V##m##_DSLEEP, \
    REG_LDO_V##m##_VBIT_DSLEEP_MASK, REG_LDO_V##m##_VBIT_DSLEEP_SHIFT,\
    REG_LDO_V##m##_VBIT_NORMAL_MASK, REG_LDO_V##m##_VBIT_NORMAL_SHIFT,\
    REG_LDO_V##m##_VBIT_RC_MASK, REG_LDO_V##m##_VBIT_RC_SHIFT }

static const struct PMU_MODULE_CFG_T pmu_module_cfg[] = {
    PMU_MOD_CFG_VAL(ANA),
    PMU_MOD_CFG_VAL(CORE),
    PMU_MOD_CFG_VAL(CODEC),
    PMU_MOD_CFG_VAL(CORE_L),
    PMU_MOD_CFG_VAL(USB),
    PMU_MOD_CFG_VAL(SENSOR),
    PMU_MOD_CFG_VAL(MEM),
};

#define OPT_TYPE                        const

static OPT_TYPE uint16_t ana_act_dcdc =
    PMU_DCDC_ANA_1_2V;
static OPT_TYPE uint16_t ana_lp_dcdc = ana_act_dcdc;

static OPT_TYPE POSSIBLY_UNUSED uint16_t vcodec_mv = (uint16_t)(VCODEC_VOLT * 1000);
static OPT_TYPE POSSIBLY_UNUSED uint16_t vhppa_mv = (uint16_t)(VHPPA_VOLT * 1000);
#ifdef PMU_VMEM_PWR_SUPPLY_IO
static OPT_TYPE POSSIBLY_UNUSED uint16_t vmem_mv = (uint16_t)(VMEM_VOLT * 1000);
#endif

static OPT_TYPE uint16_t dig_lp_ldo = PMU_VDIG_0_65V;
static OPT_TYPE uint16_t dig_lp_dcdc = PMU_DCDC_DIG_0_6V;
#ifdef SPLIT_CORE_M_CORE_L
static OPT_TYPE uint16_t dig_lp_logic = PMU_VDIG_L_0_55V;
#endif

// TODO: Use the value in BL as the default value
static enum PMU_POWER_MODE_T BOOT_DATA_LOC pmu_power_mode =
#ifdef PMU_ALREADY_INIT
    PMU_POWER_MODE_DIG_DCDC;
#else
    PMU_POWER_MODE_NONE;
#endif

// TODO: Use the value in BL as the default value
static enum PMU_VCORE_REQ_T BOOT_BSS_LOC pmu_vcore_req;

static PMU_CHARGER_IRQ_HANDLER_T charger_irq_handler;

#ifdef PMU_IRQ_UNIFIED
static bool gpio_irq_en[2];
static HAL_GPIO_PIN_IRQ_HANDLER gpio_irq_handler[2];

static PMU_WDT_IRQ_HANDLER_T wdt_irq_handler;

static PMU_IRQ_UNIFIED_HANDLER_T pmu_irq_hdlrs[PMU_IRQ_TYPE_QTY];
#endif

// Move all the data/bss invovled in pmu_open() to .sram_data/.sram_bss,
// so that pmu_open() can be called at the end of BootInit(),
// for data/bss is initialized after BootInit().
static uint8_t BOOT_DATA_LOC bbpll_codec_div = 32;

static uint32_t BOOT_BSS_LOC pmu_metal_id;

static uint16_t wdt_irq_timer;
static uint16_t wdt_reset_timer;

static const bool dcdc_ramp_en =
#if defined(DCDC_VOLT_HW_RAMP_ONLY)
    true;
#else
    false;
#endif

static uint16_t SRAM_BSS_DEF(dcdc_ramp_map);

static uint16_t ntc_raw = 0;
static int ntc_temperature = 0;
static bool ntc_irq_busy = false;
static PMU_NTC_IRQ_HANDLER_T pmu_ntc_cb;

#if defined(MCU_HIGH_PERFORMANCE_MODE)
static const uint16_t high_perf_freq_mhz =
#if defined(MTEST_ENABLED) && defined(MTEST_CLK_MHZ)
    MTEST_CLK_MHZ;
#else
    320;
#endif
PMU_OPEN_BSS_LOC
static bool high_perf_on;
#endif

static enum PMU_BOOT_CAUSE_T BOOT_BSS_LOC pmu_boot_reason = PMU_BOOT_CAUSE_NULL;

static bool BOOT_BSS_LOC pmu_boot_first_pwr_up = false;

static enum PMU_CLK_SEL_RC_USER_T rc_clk_user_map = 0;

#ifdef PMU_FORCE_LP_MODE
static enum PMU_BIG_BANDGAP_USER_T big_bandgap_user_map;
#endif

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#ifndef HPPA_LDO_ON
static void pmu_hppa_dcdc_to_ldo(void);
#endif
static void pmu_module_ramp_volt(unsigned char module, unsigned short sleep_v, unsigned short normal_v);
#ifndef PMU_VMEM_PWR_SUPPLY_IO
void pmu_module_config(enum PMU_MODULE_T module,unsigned short is_manual,unsigned short ldo_on,unsigned short lp_mode,unsigned short dpmode);
#endif
#ifdef PMU_FORCE_LP_MODE
static void pmu_bandgap_cfg_reset(void);
#endif
#if !defined(ROM_BUILD) && defined(PMU_EFUSE_INIT_IN_BOOT)
static bool BOOT_BSS_LOC efuse_buf_valid;
static unsigned short pmu_efuse_page_arrays[PMU_EFUSE_PAGE_QTY];
#endif
#endif

#if defined(PMU_CLK_USE_EXT_CRYSTAL) || defined(RTC_CLK_USE_EXT_CRYSTAL)
#ifdef PMU_CLK_USE_EXT_CRYSTAL

enum XTAL_32K_CALIB_STATE_T {
    XTAL_32K_CALIB_STATE_NULL,
    XTAL_32K_CALIB_STATE_CALIB,
    XTAL_32K_CALIB_STATE_SET,
    XTAL_32K_CALIB_STATE_ROLLBACK,
};

static enum XTAL_32K_CALIB_STATE_T xtal_32k_calib_state = XTAL_32K_CALIB_STATE_NULL;
static uint32_t xtal_32k_calib_time;
static float xtal_32k_calib_prev_hz;
static uint16_t xtal_32k_calib_prev_capbit;
static bool xtal_32k_calib_prev_valid = false;
static const uint32_t xtal_32k_calib_intvl_ms = 200;
static const uint32_t xtal_32k_calib_stable_ms = 300;
#endif

static uint32_t BOOT_BSS_LOC pmu_ext_crystal_init_stime;
static uint8_t pmu_ext_crystal_clkout_usr_map;

static void pmu_external_crystal_init(void);
static void pmu_external_crystal_clkout_after_pwroff(void);
#endif

#if defined(_AUTO_TEST_)
static bool at_skip_shutdown = false;

void pmu_at_skip_shutdown(bool enable)
{
    at_skip_shutdown = enable;
}
#endif

#ifdef RTC_CALENDAR
#define RTC_REG_VAL2CALENDAR(TENS, UNITS)                     (TENS * 10 + UNITS)
#define RTC_CALENDAR2REG_VAL_TENS(RTC_CALENDAR)               (RTC_CALENDAR / 10 % 10)
#define RTC_CALENDAR2REG_VAL_UNITS(RTC_CALENDAR)              (RTC_CALENDAR / 1 % 10)

#define EXTERNAL_CRYSTAL_FREQ                                 (32768)
#define RTC_STATUS_STABLE_TIME_MS                             (1000)

#define RTC_MONTH_MAX                                         (12)
#define RTC_MONTH_MIN                                         (1)
#define RTC_DAY_MIN                                           (1)
#define RTC_WEEK_MAX                                          (7)
#define RTC_WEEK_MIN                                          (1)
#define RTC_HOUR_MAX                                          (23)
#define RTC_HOUR_MIN                                          (0)
#define RTC_MINUTE_MAX                                        (59)
#define RTC_MINUTE_MIN                                        (0)
#define RTC_SECOND_MAX                                        (59)
#define RTC_SECOND_MIN                                        (0)

struct RTC_REG_VAL_FORMAT_T {
    uint8_t year_tens;
    uint8_t year_units;
    uint8_t data_month_tens;
    uint8_t data_month_units;
    uint8_t data_day_tens;
    uint8_t data_day_units;
    uint8_t data_week;
    uint8_t hour_tens;
    uint8_t hour_units;
    uint8_t min_tens;
    uint8_t min_units;
    uint8_t sec_tens;
    uint8_t sec_units;
};

static PMU_RTC_CALENDAR_IRQ_HANDLER_T rtc_cal_irq_handler;

static void pmu_rtc_calendar_alarm_intr_enable(void)
{
    uint16_t val;
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_INT_EN, &val);
    val |= RTC_INT_EN_0;
    pmu_write(PMU_REG_INT_EN, val);
    int_unlock(lock);
}

static void pmu_rtc_calendar_init(void)
{
    uint16_t POSSIBLY_UNUSED val;

#ifdef RTC_CLK_USE_EXT_CRYSTAL
    pmu_read(PMU_REG_RTC_RTC_CFG_10E, &val);
    val |= REG_CLK_RTC_SEL_XO;
    pmu_write(PMU_REG_RTC_RTC_CFG_10E, val);
#elif !defined(PMU_CLK_USE_EXT_CRYSTAL)
    pmu_read(PMU_REG_POWER_KEY_CFG, &val);
    val |= PU_LPO_DR | PU_LPO_REG;
    pmu_write(PMU_REG_POWER_KEY_CFG, val);
#endif

    pmu_rtc_calendar_alarm_intr_enable();

#ifdef SIMU
    val = 32 - 2;
#elif defined(RTC_CLK_USE_EXT_CRYSTAL)
    val = 32768 - 2;
#else
    val = CONFIG_SYSTICK_HZ * 2 - 2;
#endif
    pmu_write(PMU_REG_RTC_DIV_1HZ, val);
}

static void pmu_rtc_calendar_data_valid_check(struct RTC_CALENDAR_FORMAT_T *calendar)
{
    char month_buf[RTC_MONTH_MAX] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (calendar->week > RTC_WEEK_MAX || calendar->week < RTC_WEEK_MIN) {
        ASSERT(0, "invalid week! week:%d", calendar->week);
    }

    if ((calendar->hour > RTC_HOUR_MAX || calendar->minute > RTC_MINUTE_MAX || calendar->second > RTC_SECOND_MAX)) {
        ASSERT(0, "invalid time! hour/min/sec:%d/%d/%d", calendar->hour, calendar->minute, calendar->second);
    }

    if (calendar->month == 2) {
        // leap year
        if (((calendar->year % 4 == 0) && (calendar->year % 100 != 0)) || (calendar->year % 400 == 0)) {
            month_buf[1] += 1;
        }
    }

    if (calendar->month > RTC_MONTH_MAX || calendar->month < RTC_MONTH_MIN ||
            calendar->day > month_buf[calendar->month - 1] || calendar->day < RTC_DAY_MIN) {
        ASSERT(0, "invalid data! month/day:%d/%d", calendar->month, calendar->day);
    }
}

static void _pmu_rtc_calendar_alarm_irq_handler(uint16_t irq_status)
{
    struct RTC_CALENDAR_FORMAT_T alarm_cal = {0, };

    if (irq_status & RTC_INT0_MSKED) {
        pmu_write(PMU_REG_INT_STATUS, RTC_INT_0);
        if (rtc_cal_irq_handler) {
            pmu_rtc_calendar_get(&alarm_cal);
            rtc_cal_irq_handler(&alarm_cal);
        }
    }
}

int pmu_rtc_calendar_alarm_irq_handler_set(PMU_RTC_CALENDAR_IRQ_HANDLER_T handler)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    rtc_cal_irq_handler = handler;

    pmu_read(PMU_REG_INT_MASK, &val);
    if (handler) {
        val |= RTC_INT0_MSK;
    } else {
        val &= ~RTC_INT0_MSK;
    }
    pmu_write(PMU_REG_INT_MASK, val);
    pmu_set_irq_unified_handler(PMU_IRQ_TYPE_RTC, handler ? _pmu_rtc_calendar_alarm_irq_handler : NULL);
    int_unlock(lock);

    return 0;
}

int pmu_rtc_calendar_pwron_enable(bool en)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    pmu_read(PMU_REG_POWER_KEY_CFG, &val);
    if (en) {
        val |= RTC_POWER_ON_EN;
    } else {
        val &= ~RTC_POWER_ON_EN;
    }
    pmu_write(PMU_REG_POWER_KEY_CFG, val);
    int_unlock(lock);

    return 0;
}

int pmu_rtc_calendar_alarm_clear(void)
{
    uint16_t val;
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_INT_EN, &val);
    val &= ~RTC_INT_EN_0;
    pmu_write(PMU_REG_INT_EN, val);
    int_unlock(lock);

    return 0;
}

int pmu_rtc_calendar_alarm_set(struct RTC_CALENDAR_FORMAT_T *alarm_set)
{
    uint32_t lock;
    uint16_t val;
    struct RTC_REG_VAL_FORMAT_T rtc_reg_fmt = {0, };

    lock = int_lock();
    pmu_rtc_calendar_data_valid_check(alarm_set);

    rtc_reg_fmt.year_tens = RTC_CALENDAR2REG_VAL_TENS(alarm_set->year);
    rtc_reg_fmt.year_units = RTC_CALENDAR2REG_VAL_UNITS(alarm_set->year);

    rtc_reg_fmt.data_month_tens = RTC_CALENDAR2REG_VAL_TENS(alarm_set->month);
    rtc_reg_fmt.data_month_units = RTC_CALENDAR2REG_VAL_UNITS(alarm_set->month);

    rtc_reg_fmt.data_day_tens = RTC_CALENDAR2REG_VAL_TENS(alarm_set->day);
    rtc_reg_fmt.data_day_units = RTC_CALENDAR2REG_VAL_UNITS(alarm_set->day);

    rtc_reg_fmt.data_week = alarm_set->week;

    rtc_reg_fmt.hour_tens = RTC_CALENDAR2REG_VAL_TENS(alarm_set->hour);
    rtc_reg_fmt.hour_units = RTC_CALENDAR2REG_VAL_UNITS(alarm_set->hour);

    rtc_reg_fmt.min_tens = RTC_CALENDAR2REG_VAL_TENS(alarm_set->minute);
    rtc_reg_fmt.min_units = RTC_CALENDAR2REG_VAL_UNITS(alarm_set->minute);

    rtc_reg_fmt.sec_tens = RTC_CALENDAR2REG_VAL_TENS(alarm_set->second);
    rtc_reg_fmt.sec_units = RTC_CALENDAR2REG_VAL_UNITS(alarm_set->second);

    // year
    pmu_read(PMU_REG_RTC_MATCH_YEARS, &val);
    val = SET_BITFIELD(val, RTC_MATCH_YEARS_TENS, rtc_reg_fmt.year_tens);
    val = SET_BITFIELD(val, RTC_MATCH_YEARS_UNITS, rtc_reg_fmt.year_units);
    pmu_write(PMU_REG_RTC_MATCH_YEARS, val);

    // data
    pmu_read(PMU_REG_RTC_MATCH_DATAS, &val);
    if (rtc_reg_fmt.data_month_tens) {
        val |= RTC_MATCH_DATAS_MONTH_TENS;
    } else {
        val &= ~RTC_MATCH_DATAS_MONTH_TENS;
    }
    val = SET_BITFIELD(val, RTC_MATCH_DATAS_MONTH_UNITS, rtc_reg_fmt.data_month_units);
    val = SET_BITFIELD(val, RTC_MATCH_DATAS_DAY_TENS, rtc_reg_fmt.data_day_tens);
    val = SET_BITFIELD(val, RTC_MATCH_DATAS_DAY_UNITS, rtc_reg_fmt.data_day_units);
    val = SET_BITFIELD(val, RTC_MATCH_DATAS_WEEK, rtc_reg_fmt.data_week);
    pmu_write(PMU_REG_RTC_MATCH_DATAS, val);

    // hour
    pmu_read(PMU_REG_RTC_MATCH_HOURS, &val);
    val = SET_BITFIELD(val, RTC_MATCH_HOURS_TENS, rtc_reg_fmt.hour_tens);
    val = SET_BITFIELD(val, RTC_MATCH_HOURS_UNITS, rtc_reg_fmt.hour_units);
    pmu_write(PMU_REG_RTC_MATCH_HOURS, val);

    // minute_second
    pmu_read(PMU_REG_RTC_MATCH_MIN_SEC, &val);
    val = SET_BITFIELD(val, RTC_MATCH_TIME_MIN_TENS, rtc_reg_fmt.min_tens);
    val = SET_BITFIELD(val, RTC_MATCH_TIME_MIN_UNITS, rtc_reg_fmt.min_units);
    val = SET_BITFIELD(val, RTC_MATCH_TIME_SEC_TENS, rtc_reg_fmt.sec_tens);
    val = SET_BITFIELD(val, RTC_MATCH_TIME_SEC_UNITS, rtc_reg_fmt.sec_units);
    pmu_write(PMU_REG_RTC_MATCH_MIN_SEC, val);

    pmu_rtc_calendar_alarm_intr_enable();

    int_unlock(lock);

    return 0;
}

int pmu_rtc_calendar_alarm_get(struct RTC_CALENDAR_FORMAT_T *alarm_get)
{
    uint32_t lock;
    uint16_t val;
    struct RTC_REG_VAL_FORMAT_T rtc_reg_fmt = {0, };

    lock = int_lock();
    // year
    pmu_read(PMU_REG_RTC_MATCH_YEARS, &val);
    rtc_reg_fmt.year_tens = GET_BITFIELD(val, RTC_GET_YEARS_TENS);
    rtc_reg_fmt.year_units = GET_BITFIELD(val, RTC_GET_YEARS_UNITS);
    alarm_get->year = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.year_tens, rtc_reg_fmt.year_units) + 2000;

    // data
    pmu_read(PMU_REG_RTC_MATCH_DATAS, &val);
    if (val & RTC_GET_DATAS_MONTH_TENS) {
        rtc_reg_fmt.data_month_tens = 1;
    } else {
        rtc_reg_fmt.data_month_tens = 0;
    }
    rtc_reg_fmt.data_month_units = GET_BITFIELD(val, RTC_GET_DATAS_MONTH_UNITS);
    alarm_get->month = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.data_month_tens, rtc_reg_fmt.data_month_units);

    rtc_reg_fmt.data_day_tens = GET_BITFIELD(val, RTC_GET_DATAS_DAY_TENS);
    rtc_reg_fmt.data_day_units = GET_BITFIELD(val, RTC_GET_DATAS_DAY_UNITS);
    alarm_get->day = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.data_day_tens, rtc_reg_fmt.data_day_units);

    rtc_reg_fmt.data_week = GET_BITFIELD(val, RTC_GET_DATAS_WEEK);
    alarm_get->week = rtc_reg_fmt.data_week;

    // hour
    pmu_read(PMU_REG_RTC_MATCH_HOURS, &val);
    rtc_reg_fmt.hour_tens = GET_BITFIELD(val, RTC_GET_HOURS_TENS);
    rtc_reg_fmt.hour_units = GET_BITFIELD(val, RTC_GET_HOURS_UNITS);
    alarm_get->hour = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.hour_tens, rtc_reg_fmt.hour_units);

    // minute_second
    pmu_read(PMU_REG_RTC_MATCH_MIN_SEC, &val);
    rtc_reg_fmt.min_tens = GET_BITFIELD(val, RTC_GET_TIME_MIN_TENS);
    rtc_reg_fmt.min_units = GET_BITFIELD(val, RTC_GET_TIME_MIN_UNITS);
    alarm_get->minute = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.min_tens, rtc_reg_fmt.min_units);

    rtc_reg_fmt.sec_tens = GET_BITFIELD(val, RTC_GET_TIME_SEC_TENS);
    rtc_reg_fmt.sec_units = GET_BITFIELD(val, RTC_GET_TIME_SEC_UNITS);
    alarm_get->second = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.sec_tens, rtc_reg_fmt.sec_units);

    int_unlock(lock);

    return 0;
}

int pmu_rtc_calendar_set(struct RTC_CALENDAR_FORMAT_T *calendar_set)
{
    uint32_t lock;
    uint16_t val;
    struct RTC_REG_VAL_FORMAT_T rtc_reg_fmt = {0, };

    lock = int_lock();
    pmu_rtc_calendar_data_valid_check(calendar_set);

    rtc_reg_fmt.year_tens = RTC_CALENDAR2REG_VAL_TENS(calendar_set->year);
    rtc_reg_fmt.year_units = RTC_CALENDAR2REG_VAL_UNITS(calendar_set->year);

    if (RTC_CALENDAR2REG_VAL_TENS(calendar_set->month) != 0) {
        rtc_reg_fmt.data_month_tens = 1;
    } else {
        rtc_reg_fmt.data_month_tens = 0;
    }
    rtc_reg_fmt.data_month_units = RTC_CALENDAR2REG_VAL_UNITS(calendar_set->month);

    rtc_reg_fmt.data_day_tens = RTC_CALENDAR2REG_VAL_TENS(calendar_set->day);
    rtc_reg_fmt.data_day_units = RTC_CALENDAR2REG_VAL_UNITS(calendar_set->day);

    rtc_reg_fmt.data_week = calendar_set->week;

    rtc_reg_fmt.hour_tens = RTC_CALENDAR2REG_VAL_TENS(calendar_set->hour);
    rtc_reg_fmt.hour_units = RTC_CALENDAR2REG_VAL_UNITS(calendar_set->hour);

    rtc_reg_fmt.min_tens = RTC_CALENDAR2REG_VAL_TENS(calendar_set->minute);
    rtc_reg_fmt.min_units = RTC_CALENDAR2REG_VAL_UNITS(calendar_set->minute);

    rtc_reg_fmt.sec_tens = RTC_CALENDAR2REG_VAL_TENS(calendar_set->second);
    rtc_reg_fmt.sec_units = RTC_CALENDAR2REG_VAL_UNITS(calendar_set->second);

    // year
    pmu_read(PMU_REG_RTC_LOAD_YEARS, &val);
    val = SET_BITFIELD(val, RTC_LOAD_YEARS_TENS, rtc_reg_fmt.year_tens);
    val = SET_BITFIELD(val, RTC_LOAD_YEARS_UNITS, rtc_reg_fmt.year_units);
    pmu_write(PMU_REG_RTC_LOAD_YEARS, val);

    // data
    pmu_read(PMU_REG_RTC_LOAD_DATAS_EN, &val);
    if (rtc_reg_fmt.data_month_tens) {
        val |= RTC_LOAD_DATAS_MONTH_TENS;
    } else {
        val &= ~RTC_LOAD_DATAS_MONTH_TENS;
    }
    val = SET_BITFIELD(val, RTC_LOAD_DATAS_MONTH_UNITS, rtc_reg_fmt.data_month_units);
    val = SET_BITFIELD(val, RTC_LOAD_DATAS_DAY_TENS, rtc_reg_fmt.data_day_tens);
    val = SET_BITFIELD(val, RTC_LOAD_DATAS_DAY_UNITS, rtc_reg_fmt.data_day_units);
    val = SET_BITFIELD(val, RTC_LOAD_DATAS_WEEK, rtc_reg_fmt.data_week);
    pmu_write(PMU_REG_RTC_LOAD_DATAS_EN, val);

    // hour
    pmu_read(PMU_REG_RTC_LOAD_HOURS, &val);
    val = SET_BITFIELD(val, RTC_LOAD_HOURS_TENS, rtc_reg_fmt.hour_tens);
    val = SET_BITFIELD(val, RTC_LOAD_HOURS_UNITS, rtc_reg_fmt.hour_units);
    pmu_write(PMU_REG_RTC_LOAD_HOURS, val);

    // minute_second
    pmu_read(PMU_REG_RTC_LOAD_MIN_SEC_EN, &val);
    val = SET_BITFIELD(val, RTC_LOAD_TIME_MIN_TENS, rtc_reg_fmt.min_tens);
    val = SET_BITFIELD(val, RTC_LOAD_TIME_MIN_UNITS, rtc_reg_fmt.min_units);
    val = SET_BITFIELD(val, RTC_LOAD_TIME_SEC_TENS, rtc_reg_fmt.sec_tens);
    val = SET_BITFIELD(val, RTC_LOAD_TIME_SEC_UNITS, rtc_reg_fmt.sec_units);
    pmu_write(PMU_REG_RTC_LOAD_MIN_SEC_EN, val);

    pmu_read(PMU_REG_RTC_RTC_CFG_10E, &val);
    val |= RTC_TIME_LOAD;
    pmu_write(PMU_REG_RTC_RTC_CFG_10E, val);

    int_unlock(lock);

    return 0;
}

int pmu_rtc_calendar_get(struct RTC_CALENDAR_FORMAT_T *calendar_get)
{
    uint32_t lock;
    uint16_t val;
    struct RTC_REG_VAL_FORMAT_T rtc_reg_fmt = {0, };

    lock = int_lock();
    // year
    pmu_read(PMU_REG_RTC_GET_YEARS, &val);
    rtc_reg_fmt.year_tens = GET_BITFIELD(val, RTC_GET_YEARS_TENS);
    rtc_reg_fmt.year_units = GET_BITFIELD(val, RTC_GET_YEARS_UNITS);
    calendar_get->year = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.year_tens, rtc_reg_fmt.year_units) + 2000;

    // data
    pmu_read(PMU_REG_RTC_GET_DATAS, &val);
    if (val & RTC_GET_DATAS_MONTH_TENS) {
        rtc_reg_fmt.data_month_tens = 1;
    } else {
        rtc_reg_fmt.data_month_tens = 0;
    }
    rtc_reg_fmt.data_month_units = GET_BITFIELD(val, RTC_GET_DATAS_MONTH_UNITS);
    calendar_get->month = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.data_month_tens, rtc_reg_fmt.data_month_units);

    rtc_reg_fmt.data_day_tens = GET_BITFIELD(val, RTC_GET_DATAS_DAY_TENS);
    rtc_reg_fmt.data_day_units = GET_BITFIELD(val, RTC_GET_DATAS_DAY_UNITS);
    calendar_get->day = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.data_day_tens, rtc_reg_fmt.data_day_units);

    rtc_reg_fmt.data_week = GET_BITFIELD(val, RTC_GET_DATAS_WEEK);
    calendar_get->week = rtc_reg_fmt.data_week;

    // hour
    pmu_read(PMU_REG_RTC_GET_HOURS, &val);
    rtc_reg_fmt.hour_tens = GET_BITFIELD(val, RTC_GET_HOURS_TENS);
    rtc_reg_fmt.hour_units = GET_BITFIELD(val, RTC_GET_HOURS_UNITS);
    calendar_get->hour = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.hour_tens, rtc_reg_fmt.hour_units);

    // minute_second
    pmu_read(PMU_REG_RTC_GET_MIN_SEC, &val);
    rtc_reg_fmt.min_tens = GET_BITFIELD(val, RTC_GET_TIME_MIN_TENS);
    rtc_reg_fmt.min_units = GET_BITFIELD(val, RTC_GET_TIME_MIN_UNITS);
    calendar_get->minute = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.min_tens, rtc_reg_fmt.min_units);

    rtc_reg_fmt.sec_tens = GET_BITFIELD(val, RTC_GET_TIME_SEC_TENS);
    rtc_reg_fmt.sec_units = GET_BITFIELD(val, RTC_GET_TIME_SEC_UNITS);
    calendar_get->second = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.sec_tens, rtc_reg_fmt.sec_units);

    int_unlock(lock);

    return 0;
}
#endif

#ifdef PMU_LDO_VCORE_CALIB
static int8_t PMU_OPEN_BSS_LOC pmu_ldo_dig_l_comp = 0;
static int8_t PMU_OPEN_BSS_LOC pmu_ldo_lp_dig_l_comp = 0;

#ifdef PMU_ALREADY_INIT
BOOT_TEXT_FLASH_LOC
#endif
void pmu_get_ldo_dig_calib_value(void)
{
    uint16_t val;
    bool no_calib = false;

    pmu_get_efuse(PMU_EFUSE_PAGE_LDO_CORE_L_VOLT, &val);
    pmu_ldo_dig_l_comp = GET_BITFIELD(val, PMU_EFUSE_LDO_CORE_L_VOLT_COMP);
    if (val & PMU_EFUSE_LDO_CORE_L_VOLT_COMP_SIGN) {
        pmu_ldo_dig_l_comp = -pmu_ldo_dig_l_comp;
    }
    pmu_ldo_lp_dig_l_comp = GET_BITFIELD(val, PMU_EFUSE_LDO_CORE_L_VOLT_LP_COMP);
    if (val & PMU_EFUSE_LDO_CORE_L_VOLT_LP_COMP_SIGN) {
        pmu_ldo_lp_dig_l_comp = -pmu_ldo_lp_dig_l_comp;
    } else {
        if (pmu_ldo_lp_dig_l_comp == 0) {
            no_calib = true;
        }
    }

    if (no_calib) {
        pmu_ldo_lp_dig_l_comp += 9;
    }

#ifdef FORCE_BIG_BANDGAP
    pmu_ldo_lp_dig_l_comp  = pmu_ldo_dig_l_comp;
#elif defined(PMU_FORCE_LP_MODE)
    pmu_ldo_dig_l_comp  = pmu_ldo_lp_dig_l_comp;
#endif
}
#endif

#ifdef PMU_DCDC_CALIB
static int16_t PMU_OPEN_BSS_LOC pmu_dcdc_dig_comp = 0;
static int8_t PMU_OPEN_BSS_LOC pmu_dcdc_ana_comp = 0;
static int8_t PMU_OPEN_BSS_LOC pmu_dcdc_hppa_comp = 0;

static int16_t PMU_OPEN_BSS_LOC pmu_dig_lp_comp  = 0;
static int8_t PMU_OPEN_BSS_LOC pmu_ana_lp_comp  = 0;
static int8_t PMU_OPEN_BSS_LOC pmu_hppa_lp_comp = 0;

#ifdef PMU_ALREADY_INIT
BOOT_TEXT_FLASH_LOC
#endif
static POSSIBLY_UNUSED void pmu_get_dcdc_calib_value(void)
{
    uint16_t val;
    bool no_calib = false;
    bool new_calib = false;

    pmu_get_efuse(PMU_EFUSE_PAGE_DCDC1_VOLT_NEW, &val);
    new_calib = !!(val & PMU_EFUSE_DCDC1_VOLT_CAL_NEW);
    if (new_calib) {
        pmu_dig_lp_comp = GET_BITFIELD(val, PMU_EFUSE_DCDC1_VOLT_COMP);
        if (val & PMU_EFUSE_DCDC1_VOLT_COMP_SIGN) {
            pmu_dig_lp_comp = -pmu_dig_lp_comp;
        }
        pmu_dcdc_dig_comp = pmu_dig_lp_comp;
    } else {
        pmu_get_efuse(PMU_EFUSE_PAGE_DCDC1_VOLT, &val);
        pmu_dcdc_dig_comp = GET_BITFIELD(val, PMU_EFUSE_DCDC1_VOLT_COMP);
        if (val & PMU_EFUSE_DCDC1_VOLT_COMP_SIGN) {
            pmu_dcdc_dig_comp = -pmu_dcdc_dig_comp;
        }
        pmu_dig_lp_comp = GET_BITFIELD(val, PMU_EFUSE_DCDC1_VOLT_LP_COMP);
        if (val & PMU_EFUSE_DCDC1_VOLT_LP_COMP_SIGN) {
            pmu_dig_lp_comp = -pmu_dig_lp_comp;
        }
    }

    pmu_get_efuse(PMU_EFUSE_PAGE_DCDC2_VOLT, &val);
    pmu_dcdc_ana_comp = GET_BITFIELD(val, PMU_EFUSE_DCDC2_VOLT_COMP);
    if (val & PMU_EFUSE_DCDC2_VOLT_COMP_SIGN) {
        pmu_dcdc_ana_comp = -pmu_dcdc_ana_comp;
    }
    pmu_ana_lp_comp = GET_BITFIELD(val, PMU_EFUSE_DCDC2_VOLT_LP_COMP);
    if (val & PMU_EFUSE_DCDC2_VOLT_LP_COMP_SIGN) {
        pmu_ana_lp_comp = -pmu_ana_lp_comp;
    }

    pmu_get_efuse(PMU_EFUSE_PAGE_DCDC3_VOLT, &val);
    pmu_dcdc_hppa_comp = GET_BITFIELD(val, PMU_EFUSE_DCDC3_VOLT_COMP);
    if (val & PMU_EFUSE_DCDC3_VOLT_COMP_SIGN) {
        pmu_dcdc_hppa_comp = -pmu_dcdc_hppa_comp;
    }
    pmu_hppa_lp_comp = GET_BITFIELD(val, PMU_EFUSE_DCDC3_VOLT_LP_COMP);
    if (val & PMU_EFUSE_DCDC3_VOLT_LP_COMP_SIGN) {
        pmu_hppa_lp_comp = -pmu_hppa_lp_comp;
    } else {
        if (pmu_hppa_lp_comp == 0) {
            no_calib = true;
        }
    }

    if (no_calib) {
        pmu_dig_lp_comp += 37;
        pmu_ana_lp_comp += 10;
        pmu_hppa_lp_comp += 16;
    }

#ifdef FORCE_BIG_BANDGAP
    pmu_dig_lp_comp  = pmu_dcdc_dig_comp;
    pmu_ana_lp_comp  = pmu_dcdc_ana_comp;
    pmu_hppa_lp_comp = pmu_dcdc_hppa_comp;
#elif defined(PMU_FORCE_LP_MODE)
    pmu_dcdc_dig_comp  = pmu_dig_lp_comp;
    pmu_dcdc_ana_comp  = pmu_ana_lp_comp;
    pmu_dcdc_hppa_comp = pmu_hppa_lp_comp;
#endif
}

static POSSIBLY_UNUSED unsigned short pmu_reg_val_add(unsigned short val, int delta, unsigned short max)
{
    int result = val + delta;

    if (result > max) {
        result = max;
    } else if (result < 0) {
        result = 0;
    }

    return (unsigned short)result;
}
#endif

uint32_t BOOT_TEXT_FLASH_LOC read_hw_metal_id(void)
{
    uint16_t val;
    uint32_t metal_id;

    pmu_read(PMU_REG_METAL_ID, &val);
    pmu_metal_id = GET_BITFIELD(val, REVID);

    metal_id = hal_cmu_get_aon_revision_id();
    metal_id = GET_BITFIELD(metal_id, CHIP_REV_ID);
    if (metal_id == 0x4) {
        metal_id = HAL_CHIP_METAL_ID_1;
    } else if (metal_id == 0x8) {
        metal_id = HAL_CHIP_METAL_ID_2;
    } else if (metal_id == 0xC) {
        metal_id = HAL_CHIP_METAL_ID_3;
    } else if (metal_id == 0xD) {
        metal_id = HAL_CHIP_METAL_ID_4;
    } else if (metal_id == 0xA) {
        metal_id = HAL_CHIP_METAL_ID_6;
    }

    return metal_id;
}

static void BOOT_TEXT_FLASH_LOC pmu_boot_cause_init(void)
{
    uint16_t val = 0;
    uint16_t val_reason = 0;
    union HAL_HW_BOOTMODE_T hw_bm;

    hw_bm = hal_hw_bootmode_get();

    if (hw_bm.watchdog) {
        pmu_boot_reason = PMU_BOOT_CAUSE_DIG_WDT;
    } else if (hw_bm.global) {
        pmu_boot_reason = PMU_BOOT_CAUSE_DIG_REBOOT;
    } else {
        pmu_read(PMU_REG_BOOT_SOURCE, &val);
        val_reason = GET_BITFIELD(val, ABORT_SOURCE);
        if (val_reason) {
            if (val_reason == 1) {
                pmu_boot_reason = PMU_BOOT_CAUSE_PMU_WDT;
            } else if (val_reason == 2) {
                pmu_boot_reason = PMU_BOOT_CAUSE_LONG_PRESS;
            } else if (val_reason == 3) {
                pmu_boot_reason = PMU_BOOT_CAUSE_VCHG_COMMAND;
            } else if (val_reason == 4) {
                pmu_boot_reason = PMU_BOOT_CAUSE_RESET_KEY;
            }
        } else {
            val_reason = GET_BITFIELD(val, PMU_LDO_ON_SOURCE);
            if (val_reason == 1) {
                pmu_boot_reason = PMU_BOOT_CAUSE_POWER_KEY;
            } else if (val_reason == 2) {
                pmu_boot_reason = PMU_BOOT_CAUSE_RTC;
            } else if (val_reason == 3) {
                pmu_boot_reason = PMU_BOOT_CAUSE_AC_IN;
            } else if (val_reason == 4) {
                pmu_boot_reason = PMU_BOOT_CAUSE_AC_OUT;
            }
        }
        if (pmu_boot_reason != PMU_BOOT_CAUSE_NULL) {
            val = ABORT_SOURCE_CLR | PMU_LDO_ON_SOURCE_CLR;
            pmu_write(PMU_REG_BOOT_SOURCE, val);
            // One lpo clock
            hal_sys_timer_delay_us(30);
            pmu_write(PMU_REG_BOOT_SOURCE, val);
        }

    }

    pmu_read(PMU_REG_NOT_RESET_61, &val);
    if ((val & REG_NOT_RESET_CHIP_PWR_ON) == 0) {
        pmu_boot_first_pwr_up = true;
    }
#if !(defined(OTA_BOOT_IMAGE) || defined(ARM_CMSE))
    val |= REG_NOT_RESET_CHIP_PWR_ON;
    pmu_write(PMU_REG_NOT_RESET_61, val);
#endif
}

enum PMU_BOOT_CAUSE_T pmu_boot_cause_get(void)
{
    return pmu_boot_reason;
}

bool pmu_boot_first_power_up(void)
{
    return pmu_boot_first_pwr_up;
}

void BOOT_TEXT_FLASH_LOC pmu_boot_init(void)
{
    uint16_t val;

    pmu_boot_cause_init();

#ifdef __WATCHER_DOG_RESET__
    pmu_wdt_save_context();
#endif

#if !defined(FPGA) && !defined(PROGRAMMER)
    pmu_charger_save_context();
#endif

#ifdef PMU_CLK_USE_EXT_CRYSTAL
    uint16_t val_32k_sel;

    // PMU clock sel LPO
    pmu_read(PMU_REG_CLK_SEL_CFG, &val_32k_sel);
    val_32k_sel = SET_BITFIELD(val_32k_sel, CLK_32K_SEL, 0x0);
    pmu_write(PMU_REG_CLK_SEL_CFG, val_32k_sel);
    hal_sys_timer_delay(MS_TO_TICKS(PMU_CLK_SWITCH_STABLE_TIME));
#endif

#ifndef PMU_ALREADY_INIT
    // Reset PMU (to recover from a possible insane state, e.g., ESD reset)
    pmu_write(PMU_REG_METAL_ID, 0xCAFE);
    pmu_write(PMU_REG_METAL_ID, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));
#endif

    pmu_read(PMU_REG_CHARGER_CFG, &val);
    val |= REG_CHARGE_INTR_EN;
    pmu_write(PMU_REG_CHARGER_CFG, val);

    // Disable LDO module hw ramp_en by default part1 (vcore/vana/vmem/vcodec)
    pmu_read(PMU_REG_LDO_CORE_CFG, &val);
    val &= ~REG_LDO_VCORE_RAMP_EN;
    pmu_write(PMU_REG_LDO_CORE_CFG, val);

    pmu_read(PMU_REG_LDO_ANA_CFG, &val);
    val &= ~REG_LDO_VANA_RAMP_EN;
    pmu_write(PMU_REG_LDO_ANA_CFG, val);

    pmu_read(PMU_REG_LDO_CODEC_CFG, &val);
    val &= ~REG_LDO_VCODEC_RAMP_EN;
    pmu_write(PMU_REG_LDO_CODEC_CFG, val);

    pmu_read(PMU_REG_LDO_MEM_CFG, &val);
    val &= ~REG_LDO_VMEM_RAMP_EN;
    pmu_write(PMU_REG_LDO_MEM_CFG, val);

    chg_write(CHG_REG_METAL_ID, 0xCAFE);
    chg_write(CHG_REG_METAL_ID, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));

#if defined(PMU_CLK_USE_EXT_CRYSTAL) || defined(RTC_CLK_USE_EXT_CRYSTAL)
    pmu_external_crystal_init();
#else
    pmu_write(PMU_REG_XO_32K_CFG, REG_XO_32K_PU_LDO_DR);
#endif

#ifdef __WATCHER_DOG_RESET__
    pmu_wdt_restore_context();
#else
    pmu_wdt_stop();
#endif

#if defined(PMU_DCDC_CALIB) && defined(PMU_ALREADY_INIT)
    pmu_get_dcdc_calib_value();
#endif

#if defined(PMU_LDO_VCORE_CALIB) && defined(PMU_ALREADY_INIT)
    pmu_get_ldo_dig_calib_value();
#endif

    pmu_rf_ana_init();
}

void pmu_pll_freq_reg_set(uint16_t low, uint16_t high, uint16_t high2)
{
    int ret;
    uint16_t val;

    val = low;
    ret = rf_write(RF_REG_52F, val);
    if (ret) {
        return;
    }

    val = high;
    ret = rf_write(RF_REG_530, val);
    if (ret) {
        return;
    }

    rf_read(RF_REG_531, &val);
    val = SET_BITFIELD(val, REG_LP_BBPLL_SDM_FREQWORD_34_32, high2);
    ret = rf_write(RF_REG_531, val);
    if (ret) {
        return;
    }

    ret = rf_read(RF_REG_52D, &val);
    if (ret) {
        return;
    }
    val |= REG_LP_BBPLL_FREQ_EN_COMB;
    ret = rf_write(RF_REG_52D, val);
    if (ret) {
        return;
    }

    // Delay at least for 7us
    hal_sys_timer_delay(US_TO_TICKS(100));
}

void BOOT_TEXT_FLASH_LOC bbpll_freq_pll_config(uint32_t freq)
{
    uint64_t PLL_cfg_val;
    uint16_t v[3];
    uint32_t crystal = hal_cmu_get_crystal_freq();

    PLL_cfg_val = ((uint64_t)(1 << 28) * freq + crystal / 2) / crystal;
    v[0] = PLL_cfg_val & 0xFFFF;
    v[1] = (PLL_cfg_val >> 16) & 0xFFFF;
    v[2] = (PLL_cfg_val >> 32) & 0xFFFF;

    pmu_pll_freq_reg_set(v[0], v[1], v[2]);
}

void bbpll_codec_clock_enable(bool en)
{
    uint16_t val;

    rf_read(RF_REG_537, &val);
    if (en) {
        val |= REG_LP_BBPLL_EN_CLK_CODEC;
    } else {
        val &= ~(REG_LP_BBPLL_EN_CLK_CODEC);
    }
    rf_write(RF_REG_537, val);
}

void BOOT_TEXT_FLASH_LOC bbpll_pll_update(uint64_t pll_cfg_val, bool calib)
{
    uint16_t v[3];
    uint16_t val;
    //bool sys_is_bbpll = false;
    //bool flash_is_bbpll = false;
    //uint32_t lock = 0;

    v[0] = pll_cfg_val & 0xFFFF;
    v[1] = (pll_cfg_val >> 16) & 0xFFFF;
    v[2] = (pll_cfg_val >> 32) & 0xFFFF;

    val = v[0];
    rf_write(RF_REG_52F, val);

    val = v[1];
    rf_write(RF_REG_530, val);

    rf_read(RF_REG_531, &val);
    val = SET_BITFIELD(val, REG_LP_BBPLL_SDM_FREQWORD_34_32, v[2]);
    rf_write(RF_REG_531, val);

    rf_read(RF_REG_52D, &val);
    val |= REG_LP_BBPLL_FREQ_EN_COMB;
    rf_write(RF_REG_52D, val);
}

void bbpll_set_codec_div(uint32_t div)
{
    uint16_t val;

    rf_read(RF_REG_537, &val);
    val = SET_BITFIELD(val, REG_LP_BBPLL_DIVN_AD, div);
    rf_write(RF_REG_537, val);
}

void bbpll_set_spd(uint32_t spd)
{
    uint16_t val;

    rf_read(RF_REG_535, &val);
    val = SET_BITFIELD(val, REG_LP_BBPLL_VCO_SPD, spd);
    rf_write(RF_REG_535, val);
}

void bbpll_auto_calib_spd(uint32_t codec_freq, uint32_t div)
{
    const uint32_t sys_freq_div = 4;
    const uint32_t f_med = codec_freq * div / sys_freq_div
#ifndef AUD_PLL_DOUBLE
        / 2
#endif
        ;
    const uint32_t calc_ms = 50;
    const float deviation = (float)f_med / hal_cmu_get_crystal_freq() * (1.0f / CONFIG_FAST_SYSTICK_HZ) / (calc_ms / 1000.0f);
    const uint32_t f_min = f_med - (uint32_t)(f_med * deviation * 1.6f);
    const uint32_t f_max = f_med + (uint32_t)(f_med * deviation * 1.2f);
    uint32_t f;
    uint32_t k = 0;
    uint32_t valid_cnt = 0;
    uint32_t valid_start = 0;
    uint32_t valid_end = 0;
    uint32_t cur_cnt = 0;
    uint32_t sel;
    uint32_t lock;

    lock = int_lock();

    const enum HAL_CMU_FREQ_T org_freq = hal_sysfreq_get_hw_freq();
    // Switch to 52M to avoid using PLL
    hal_cmu_sys_set_freq(HAL_CMU_FREQ_52M);
    // Enable PLL
#ifdef ULTRA_LOW_POWER
    hal_cmu_low_freq_mode_disable(org_freq, HAL_CMU_FREQ_208M);
#endif
    // Inc vcore voltage
    pmu_sys_freq_config(HAL_CMU_FREQ_208M);

    analog_aud_freq_pll_config(codec_freq, div);
    TRACE(0,"codec_freq = %d, div = %d", codec_freq, div);
    TRACE(0,"f_med = %d, deviation = %f", f_med, (double)deviation);
    hal_cmu_force_sys_pll_div(sys_freq_div);

    for (k = 0; k < 8; k++) {
        // Switch to 52M to avoid using PLL
        hal_cmu_sys_set_freq(HAL_CMU_FREQ_52M);

        bbpll_set_spd(k);

        // Switch to PLL freq
        hal_cmu_sys_set_freq(HAL_CMU_FREQ_208M);

        f = hal_sys_timer_calc_cpu_freq(calc_ms, 1);
        if (f_min < f && f < f_max) {
            cur_cnt++;
        } else {
            if (cur_cnt) {
                if (cur_cnt > valid_cnt) {
                    valid_cnt = cur_cnt;
                    valid_end = k - 1;
                }
                cur_cnt = 0;
            }
        }
        TRACE(0, "[%c%2u] f=%u min=%u max=%u", cur_cnt ? '+' : '-', k, f, f_min, f_max);
    }

    if (cur_cnt) {
        if (cur_cnt > valid_cnt) {
            valid_cnt = cur_cnt;
            valid_end = k - 1;
        }
    }
    if (valid_cnt) {
        valid_start = valid_end + 1 - valid_cnt;
        sel = (valid_start + valid_end) / 2;
        TRACE(0, "Valid spd=[%2u, %2u] cnt=%2u sel=%2u/0x%02X", valid_start, valid_end, valid_cnt, sel, sel);
    } else {
        sel = 0x0;
        TRACE(0, "BAD: no valid spd");
    }
    bbpll_set_spd(sel);
    // Restore the sys freq
    hal_cmu_force_sys_pll_div(0);
    hal_cmu_sys_set_freq(org_freq);
    // Restore PLL status
#ifdef ULTRA_LOW_POWER
    hal_cmu_low_freq_mode_enable(HAL_CMU_FREQ_208M, org_freq);
#endif
    // Restore vcore voltage
    pmu_sys_freq_config(org_freq);

    int_unlock(lock);
}

int pmu_rc_2m_calib(void)
{
    uint16_t val;

    pmu_read(PMU_REG_RCOSC2M_GOAL_CNT_MARK, &val);
    val = SET_BITFIELD(val, REG_RCOSC2M_GOAL_CNT_MARK_15_0, 0x2FFF);
    pmu_write(PMU_REG_RCOSC2M_GOAL_CNT_MARK, val);

    pmu_read(PMU_REG_RCOSC2M_CALIB, &val);
    val = SET_BITFIELD(val, REG_RCOSC2M_REF_DIV_NUM, 0x1FF);
    pmu_write(PMU_REG_RCOSC2M_CALIB, val);

    pmu_read(PMU_REG_RCOSC_2M_CFG, &val);
    val |= REG_RCOSC_2M_EN_CLK_PMU_DIG;
    pmu_write(PMU_REG_RCOSC_2M_CFG, val);

    pmu_read(PMU_REG_RESERVED_ANA, &val);
    val &= ~RESERVED_ANA_01;
    pmu_write(PMU_REG_RESERVED_ANA, val);

    pmu_read(PMU_REG_RCOSC_2M_CFG, &val);
    val |= REG_RCOSC_2M_CALIB_GATE;
    pmu_write(PMU_REG_RCOSC_2M_CFG, val);

    pmu_read(PMU_REG_RCOSC2M_CALIB, &val);
    val |= REG_RCOSC2M_SFT_RSTN | REG_RCOSC2M_CALIB_EN;
    pmu_write(PMU_REG_RCOSC2M_CALIB, val);

    hal_sys_timer_delay(MS_TO_TICKS(10));
    pmu_read(PMU_REG_DBG_RCOSC2M, &val);
    if ((val & DBG_RCOSC2M_TUN_DONE) == 0) {
        return 1;
    }

    pmu_read(PMU_REG_RCOSC_2M_CFG, &val);
    val &= ~REG_RCOSC_2M_CALIB_GATE;
    pmu_write(PMU_REG_RCOSC_2M_CFG, val);

    return 0;
}

void BOOT_TEXT_FLASH_LOC pmu_rc_2m_enable(void)
{
    POSSIBLY_UNUSED int ret;
    uint16_t val;

    pmu_read(PMU_REG_RCOSC_2M_CFG, &val);
    val |= REG_RCOSC_2M_PU_LDO | REG_RCOSC_2M_PU_LDO_DR | REG_RCOSC_2M_EN | REG_RCOSC_2M_EN_DR;
    val |= REG_RCOSC_2M_EN_CLK_DIG;
    pmu_write(PMU_REG_RCOSC_2M_CFG, val);

#ifndef SIMU
    ret = pmu_rc_2m_calib();
    if (ret) {
        PMU_INFO_TRACE_IMM(0, "\nError: rc_2m_calib failed! \n");
    }
#ifdef CALIB_SLOW_TIMER
    // Calib slow timer again
    hal_sys_timer_calib();
#endif
#endif
}

#define CODEC_FREQ                  (48000 * 512)
#define PLL_DIV                     (16)
#define CRYSTAL                     (24000000)
void BOOT_TEXT_FLASH_LOC pmu_rf_ana_init(void)
{
    uint64_t PLL_cfg_val;

#if defined(PMU_FULL_INIT) || (!defined(FPGA) && !defined(PROGRAMMER) && !defined(PMU_ALREADY_INIT))
    // Reset RF
    rf_write(RF_REG_00, 0xCAFE);
    rf_write(RF_REG_00, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));

    // Reset ANA
    ana_write(ANA_REG_60, 0xCAFE);
    ana_write(ANA_REG_60, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));
#endif

    // init pll 393MHz
    // CODEC_FREQ is likely 24.576M (48K series) or 22.5792M (44.1K series)
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_6){
        PLL_cfg_val = ((uint64_t)(1 << 25) * CODEC_FREQ * PLL_DIV + CRYSTAL / 2) / CRYSTAL;
    }else{
        PLL_cfg_val = ((uint64_t)(1 << 28) * CODEC_FREQ * PLL_DIV + CRYSTAL / 2) / CRYSTAL;
    }
    bbpll_pll_update(PLL_cfg_val, true);

#if defined(RC_CLK_ENABLE) || (defined(PSRAM_ENABLE) && (PSRAM_SPEED >= 200))
    uint16_t val;
    rf_read(RF_REG_537, &val);
    val &= ~REG_LP_BBPLL_DIG_CLK_FREQ_SEL;
    rf_write(RF_REG_537, val);
#endif
}

void BOOT_TEXT_SRAM_LOC pmu_pll_div_reset_set(enum HAL_CMU_PLL_T pll)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    if (pll == HAL_CMU_PLL_AUD) {
        rf_read(RF_REG_F5, &val);
        val |= REG_BT_BBPLL_DIVN_CODEC_MASK;
        rf_write(RF_REG_F5, val);
    }
    int_unlock(lock);
}

void BOOT_TEXT_SRAM_LOC pmu_pll_div_reset_clear(enum HAL_CMU_PLL_T pll)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    if (pll == HAL_CMU_PLL_AUD) {
        rf_read(RF_REG_F5, &val);
        val = SET_BITFIELD(val, REG_BT_BBPLL_DIVN_CODEC, bbpll_codec_div);
        rf_write(RF_REG_F5, val);
    }
    int_unlock(lock);
}

void pmu_pll_div_set(enum HAL_CMU_PLL_T pll, enum PMU_PLL_DIV_TYPE_T type, uint32_t div)
{
    uint32_t lock;
    uint16_t val;

    if (type != PMU_PLL_DIV_CODEC) {
        return;
    }

    lock = int_lock();
    if (pll == HAL_CMU_PLL_AUD) {
        if (div != bbpll_codec_div) {
            bbpll_codec_div = div;
            rf_read(RF_REG_F5, &val);
            val |= REG_BT_BBPLL_DIVN_CODEC_MASK;
            rf_write(RF_REG_F5, val);
            if (div != (REG_BT_BBPLL_DIVN_CODEC_MASK >> REG_BT_BBPLL_DIVN_CODEC_SHIFT)) {
                val = SET_BITFIELD(val, REG_BT_BBPLL_DIVN_CODEC, bbpll_codec_div);
                rf_write(RF_REG_F5, val);
            }
        }
    }
    int_unlock(lock);
}

int pmu_get_security_value(union SECURITY_VALUE_T *val)
{
    int ret;

    val->reg = 0;

    ret = pmu_get_efuse(PMU_EFUSE_PAGE_SECURITY, &val->reg);
    if (ret) {
        // Error
        return ret;
    }

    return 0;
}

enum HAL_FLASH_ID_T pmu_get_boot_flash_ctrl_id(void)
{
#ifdef FLASH1_CTRL_BASE
    union SECURITY_VALUE_T security;

    pmu_get_security_value(&security);

    return (security.root.flash_id) ? HAL_FLASH_ID_1 : HAL_FLASH_ID_0;
#else
    return HAL_FLASH_ID_0;
#endif
}

static int BOOT_TEXT_SRAM_LOC pmu_get_efuse_phy(enum PMU_EFUSE_PAGE_T page, unsigned short *efuse)
{
    int ret;
    unsigned short val;
    unsigned short tmp[2];

#ifdef EFUSE_READ_WORKAROUND
    *efuse = 0;
    return 0;
#endif

    // Sel fast clk
    pmu_read(PMU_REG_EFUSE_CLOCK_CTRL, &val);
    if ((val & REG_EFUSE_CLK_OSC_SEL) == 0) {
        val = REG_EFUSE_CLK_OSC_SEL;
        ret = pmu_write(PMU_REG_EFUSE_CLOCK_CTRL, val);
        if (ret) {
            goto _exit;
        }
        hal_sys_timer_delay_us(100);
    }

    // Enable CLK_EN
    val = REG_EFUSE_CLK_EN;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // Enable TURN_ON
    val |= REG_EFUSE_TURN_ON;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // Write Address
#ifdef PMU_EFUSE_NO_REDUNDANCY
    val |= REG_EFUSE_ADDRESS(page / 2);
#else
    val |= REG_EFUSE_ADDRESS(page); //redundancy
#endif
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // Set Strobe Trigger = 1
    val |= REG_EFUSE_STROBE_TRIGGER;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // set Strobe Trigger = 0
    val &= ~REG_EFUSE_STROBE_TRIGGER;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);
    if (ret) {
        goto _exit;
    }

    // Read Efuse High 16 bits
    ret = pmu_read(PMU_REG_EFUSE_DATA_LOW, &tmp[0]);
    if (ret) {
        goto _exit;
    }

    // Read Efuse Low 16 bits
    ret = pmu_read(PMU_REG_EFUSE_DATA_HIGH, &tmp[1]);
    if (ret) {
        goto _exit;
    }
#ifdef PMU_EFUSE_NO_REDUNDANCY
    *efuse = tmp[page % 2];
#else
    *efuse = (tmp[0] | tmp[1]); //redundancy
#endif

    _exit:
    // Disable TURN_ON
    val &= ~(REG_EFUSE_TURN_ON | REG_EFUSE_ADDRESS_MASK);
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);

    // Disable CLK_EN
    val &= ~REG_EFUSE_CLK_EN;
    ret = pmu_write(PMU_REG_EFUSE_CTRL, val);

    return ret;
}

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#if !defined(ROM_BUILD) && defined(PMU_EFUSE_INIT_IN_BOOT)
static void BOOT_TEXT_SRAM_LOC pmu_efuse_init(void)
{
    uint32_t i;

    for (i = 0; i < PMU_EFUSE_PAGE_QTY; i++) {
        pmu_get_efuse_phy(i, &pmu_efuse_page_arrays[i]);
    }

    efuse_buf_valid = true;
}
#endif
#endif

int BOOT_TEXT_SRAM_LOC pmu_get_efuse(enum PMU_EFUSE_PAGE_T page, unsigned short *efuse)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#if !defined(ROM_BUILD) && defined(PMU_EFUSE_INIT_IN_BOOT)
    if (efuse_buf_valid) {
        *efuse = pmu_efuse_page_arrays[page];
        return 0;
    }
#endif
#endif
    return pmu_get_efuse_phy(page, efuse);
}

static void pmu_sys_ctrl(bool shutdown)
{
    uint16_t val;
    uint32_t lock = int_lock();

    PMU_INFO_TRACE_IMM(0, "Start pmu %s", shutdown ? "shutdown" : "reboot");

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#ifdef RC_CLK_ENABLE
#ifdef RC_FAST_WAKEUP
    hal_cmu_rc_2m_clock_disable();
#endif
    if (pmu_rc_clock_enabled()) {
        hal_sysfreq_select_clock(HAL_SYSFREQ_SELECT_CLOCK_OSC);
        if (pmu_rc_clock_enabled()) {
            PMU_INFO_TRACE_IMM(0, "\nError: Siwtch to OSC clock fail! \n");
        }
    }
#endif

#if defined(MCU_HIGH_PERFORMANCE_MODE)
    // Default vcore might not be high enough to support high performance mode
    pmu_high_performance_mode_enable(false);
#endif
    hal_cmu_sys_set_freq(HAL_CMU_FREQ_26M);
    pmu_vcore_req |= PMU_VCORE_PMU_RESET;

#ifdef SPLIT_CORE_M_CORE_L
    pmu_read(PMU_REG_CORE_L_CFG2, &val);
    val |= REG_LDO_VCORE_L_BYPASS;
    pmu_write(PMU_REG_CORE_L_CFG2, val);
    hal_sys_timer_delay(MS_TO_TICKS(1));
#endif

#ifdef PMU_FORCE_LP_MODE
    pmu_bandgap_cfg_reset();
#endif

    pmu_module_ramp_volt(PMU_MEM, PMU_VMEM_DEFAULT_V, PMU_VMEM_DEFAULT_V);
#ifndef PMU_VMEM_PWR_SUPPLY_IO
    pmu_module_config(PMU_MEM,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);
    hal_sys_timer_delay_us(PMU_LDO_PU_STABLE_TIME_US);
#endif

#ifndef HPPA_LDO_ON
    pmu_hppa_dcdc_to_ldo();
#endif
    pmu_mode_change(PMU_POWER_MODE_LDO);
    hal_sys_timer_delay(MS_TO_TICKS(1));

#ifdef VRTC_SWITCH_TO_VCORE
    pmu_read(PMU_REG_LDO_VRTC_CFG, &val);
    val &= ~REG_LDO_VRTC_RAMP_EN;
    pmu_write(PMU_REG_LDO_VRTC_CFG, val);

    // VRTC power resume
    pmu_read(PMU_REG_LDO_VRTC_VOLT, &val);
    val = SET_BITFIELD(val, REG_LDO_VRTC_VBIT_NORMAL, PMU_LDO_VRTC_DEFAULT_ACT);
    val = SET_BITFIELD(val, REG_LDO_VRTC_VBIT_RC, PMU_LDO_VRTC_DEFAULT_ACT);
    val = SET_BITFIELD(val, REG_LDO_VRTC_VBIT_DSLEEP, PMU_LDO_VRTC_DEFAULT_LP);
    pmu_write(PMU_REG_LDO_VRTC_VOLT, val);
    hal_sys_timer_delay_us(PMU_LDO_VRTC_STABLE_TIME_US);

    pmu_read(PMU_REG_LPO_VRTC_CFG_D2, &val);
    val &= ~REG_VRTC_OVERRIDE;
    pmu_write(PMU_REG_LPO_VRTC_CFG_D2, val);
#endif
#endif

#ifdef PMU_CLK_USE_EXT_CRYSTAL
    uint16_t val_32k_sel;

    // PMU clock sel LPO
    pmu_read(PMU_REG_CLK_SEL_CFG, &val_32k_sel);
    val_32k_sel = SET_BITFIELD(val_32k_sel, CLK_32K_SEL, 0x0);
    pmu_write(PMU_REG_CLK_SEL_CFG, val_32k_sel);
    hal_sys_timer_delay(MS_TO_TICKS(PMU_CLK_SWITCH_STABLE_TIME));
#endif

    // Reset PMU
    pmu_write(PMU_REG_METAL_ID, 0xCAFE);
    pmu_write(PMU_REG_METAL_ID, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));

#ifdef PMU_CLK_USE_EXT_CRYSTAL
    // PMU clock sel EXT32K
    val_32k_sel = SET_BITFIELD(val_32k_sel, CLK_32K_SEL, 0x2);
    pmu_write(PMU_REG_CLK_SEL_CFG, val_32k_sel);
    hal_sys_timer_delay(MS_TO_TICKS(PMU_CLK_SWITCH_STABLE_TIME));
#endif

    pmu_read(PMU_REG_CHARGER_CFG, &val);
    val |= REG_CHARGE_INTR_EN;
    pmu_write(PMU_REG_CHARGER_CFG, val);

#ifdef RTC_CALENDAR
    pmu_rtc_calendar_init();
#endif

    if (shutdown) {
#if defined(_AUTO_TEST_)
        if (at_skip_shutdown) {
            hal_cmu_sys_reboot();
            return;
        }
#endif

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
        if (
#if defined(CHG_OUT_PWRON) || defined(RTC_CALENDAR) || defined(PMU_CLK_USE_EXT_CRYSTAL) || defined(RTC_CLK_USE_EXT_CRYSTAL)
            // ac_on will en lpo
            true ||
#endif
            false) {
            pmu_wdt_stop();
        } else {
            pmu_wdt_config(3*1000, 3*1000);
            pmu_wdt_start();
        }
#endif

#if defined(PMU_CLK_USE_EXT_CRYSTAL) || defined(RTC_CLK_USE_EXT_CRYSTAL)
        if (pmu_ext_crystal_clkout_usr_map) {
            pmu_external_crystal_clkout_after_pwroff();
        }
#endif

        pmu_read(PMU_REG_USB_CFG_3F, &val);
#ifdef CHG_OUT_PWRON
        val |= AC_OUT_LDO_ON_EN;
#else
        val &= ~AC_OUT_LDO_ON_EN;
#endif
        pmu_write(PMU_REG_USB_CFG_3F, val);

#ifdef ACIN_PATTERN_DISABLE
        charger_charge_pattern_disable();
#endif

        uint16_t val_temp;

        // Power off
        pmu_read(PMU_REG_POWER_OFF,&val);
        pmu_read(PMU_REG_PWR_OFF_CNT_CFG, &val_temp);
        if (val & SOFT_POWER_OFF) {
            val_temp = SET_BITFIELD(val_temp, POWER_OFF_CNT, 0x0);
            PMU_INFO_TRACE_IMM(0, "\nError: SOFT_POWER_OFF=1 \n");
        } else {
            val_temp = SET_BITFIELD(val_temp, POWER_OFF_CNT, 0xF);
        }
        pmu_write(PMU_REG_PWR_OFF_CNT_CFG, val_temp);

        val |= SOFT_POWER_OFF;
        for (int i = 0; i < 100; i++) {
            pmu_write(PMU_REG_POWER_OFF,val);
            hal_sys_timer_delay(MS_TO_TICKS(5));
        }

        hal_sys_timer_delay(MS_TO_TICKS(50));

        //can't reach here
        PMU_INFO_TRACE_IMM(0, "\nError: pmu shutdown failed!\n");
        hal_sys_timer_delay(MS_TO_TICKS(5));
    } else {
#if defined(PMU_FULL_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
        // CAUTION:
        // 1) Never reset RF because system or flash might be using X2/X4, which are off by default
        // 2) Never reset RF/ANA because system or flash might be using PLL, and the reset might cause clock glitch
        // TODO:
        // Restore BBPLL settings in RF
#endif
    }

    hal_cmu_sys_reboot();

    hal_sys_timer_delay(MS_TO_TICKS(5));
    hal_cmu_sys_reboot();
    for (int i = 0; i < 10; i++) {
        hal_sys_timer_delay(MS_TO_TICKS(1));
    }

    // Use pmu wdt reset instead of global reset, bootmode will be cleared
    pmu_wdt_reboot();
    while (1) {}

    int_unlock(lock);
}

void pmu_shutdown(void)
{
    pmu_sys_ctrl(true);
}

void pmu_reboot(void)
{
    pmu_sys_ctrl(false);
}

static inline uint16_t pmu_get_module_addr(enum PMU_MODULE_T module)
{
    if (module == PMU_ANA) {
        return PMU_REG_LDO_ANA_CFG;
    } else if (module == PMU_CORE) {
        return PMU_REG_LDO_CORE_CFG;
    } else if (module == PMU_CORE_L) {
        return PMU_REG_LDO_CORE_L_CFG;
    } else if (module == PMU_CODEC) {
        return PMU_REG_LDO_CODEC_CFG;
    } else if (module == PMU_USB) {
        return PMU_REG_LDO_USB_CFG;
    } else if (module == PMU_SENSOR) {
        return PMU_REG_LDO_SENSOR_CFG;
    } else {
        return PMU_REG_LDO_MEM_CFG;
    }
}

static inline uint16_t pmu_get_module_volt_addr(enum PMU_MODULE_T module)
{
    if (module == PMU_ANA) {
        return PMU_REG_LDO_ANA_VOLT;
    } else if (module == PMU_CORE) {
        return PMU_REG_LDO_CORE_VOLT;
    } else if (module == PMU_CORE_L) {
        return PMU_REG_LDO_CORE_L_VOLT;
    } else if (module == PMU_CODEC) {
        return PMU_REG_LDO_CODEC_VOLT;
    } else if (module == PMU_USB) {
        return PMU_REG_LDO_USB_VOLT;
    } else if (module == PMU_SENSOR) {
        return PMU_REG_LDO_SENSOR_VOLT;
    } else {
        return PMU_REG_LDO_MEM_VOLT;
    }
}

void pmu_module_config(enum PMU_MODULE_T module,unsigned short is_manual,unsigned short ldo_on,unsigned short lp_mode,unsigned short dpmode)
{
    unsigned short val;
    unsigned short module_address;
    const struct PMU_MODULE_CFG_T *module_cfg_p = &pmu_module_cfg[module];

    module_address = pmu_get_module_addr(module);

    pmu_read(module_address, &val);
    if (is_manual) {
        val |= module_cfg_p->manual_bit;
    } else {
        val &= ~module_cfg_p->manual_bit;
    }
    if (ldo_on) {
        val |= module_cfg_p->ldo_en;
    } else {
        val &= ~module_cfg_p->ldo_en;
    }

#ifdef PMU_FORCE_LP_MODE
    if (pmu_power_mode != PMU_POWER_MODE_NONE) {
        val |= module_cfg_p->lp_en_dr;
        if (ldo_on) {
            val |= module_cfg_p->lp_en;
        } else {
            val &= ~module_cfg_p->lp_en;
        }
    }
#else
    if (lp_mode) {
        val &= ~module_cfg_p->lp_en_dr;
    } else {
        val = (val & ~module_cfg_p->lp_en) | module_cfg_p->lp_en_dr;
    }
#endif

    // Avoid pulldown reg is enabled by pmu_ldo_vsensor_off_pull_down() and not call pmu_ldo_vsensor_on_lp_dr() to resume pulldown reg.
    if (module == PMU_SENSOR && ldo_on) {
        val &= ~REG_LDO_VSENSOR_PULLDOWN;
    }

    if (dpmode) {
        val |= module_cfg_p->dsleep_mode;
    } else {
        val &= ~module_cfg_p->dsleep_mode;
    }
    pmu_write(module_address, val);
}

#ifdef PMU_FORCE_LP_MODE
void pmu_module_force_lp_config(void)
{
    uint16_t val;
    uint16_t module_address;
    const struct PMU_MODULE_CFG_T *module_cfg_p;
    enum PMU_MODULE_T module;
    bool ldo_on;

    for (module = 0; module < PMU_MODULE_QTY; module++) {
        module_cfg_p = &pmu_module_cfg[module];
        module_address = pmu_get_module_addr(module);

        pmu_read(module_address, &val);
        ldo_on = !!(val & module_cfg_p->ldo_en);
        val |= module_cfg_p->lp_en_dr;
        if (ldo_on) {
            val |= module_cfg_p->lp_en;
        } else {
            val &= ~module_cfg_p->lp_en;
        }
        pmu_write(module_address, val);
    }
}
#endif

void pmu_module_set_volt(unsigned char module, unsigned short sleep_v, unsigned short normal_v)
{
    unsigned short val;
    unsigned short module_volt_address;
    unsigned short rc_v = normal_v;
    const struct PMU_MODULE_CFG_T *module_cfg_p = &pmu_module_cfg[module];

#ifdef PMU_LDO_VCORE_CALIB
    if (module == PMU_CORE_L) {
        normal_v = pmu_reg_val_add(normal_v, pmu_ldo_dig_l_comp, MAX_LDO_VCORE_L_BIT_VAL);
        sleep_v = pmu_reg_val_add(sleep_v, pmu_ldo_lp_dig_l_comp, MAX_LDO_VCORE_L_BIT_VAL);
#if !defined(PMU_FORCE_LP_MODE) && !defined(FORCE_BIG_BANDGAP)
        rc_v = pmu_reg_val_add(rc_v, pmu_ldo_lp_dig_l_comp, MAX_LDO_VCORE_L_BIT_VAL);
#else
        rc_v = normal_v;
#endif
    }
#endif

    module_volt_address = pmu_get_module_volt_addr(module);

    pmu_read(module_volt_address, &val);
    val &= ~module_cfg_p->normal_v;
    val |= (normal_v << module_cfg_p->normal_v_shift) & module_cfg_p->normal_v;
    if (module <= PMU_CODEC) {
        val &= ~module_cfg_p->dsleep_v;
        val |= (sleep_v << module_cfg_p->dsleep_v_shift) & module_cfg_p->dsleep_v;
#ifdef PMU_VCORE_RC_MODE_FIXED
        if (module != PMU_CORE)
#endif
        {
            val &= ~module_cfg_p->rc_v;
            val |= (rc_v << module_cfg_p->rc_v_shift) & module_cfg_p->rc_v;
        }
    }
    pmu_write(module_volt_address, val);
    if (module > PMU_CODEC) {
        module_volt_address += 1;

        pmu_read(module_volt_address, &val);
        val &= ~module_cfg_p->dsleep_v;
        val |= (sleep_v << module_cfg_p->dsleep_v_shift) & module_cfg_p->dsleep_v;
#ifdef PMU_VCORE_RC_MODE_FIXED
        if (module != PMU_CORE_L)
#endif
        {
            val &= ~module_cfg_p->rc_v;
            val |= (rc_v << module_cfg_p->rc_v_shift) & module_cfg_p->rc_v;
        }
        pmu_write(module_volt_address, val);
    }

#ifdef SPLIT_CORE_M_CORE_L
    if (module == PMU_CORE_L) {
        if (!dcdc_ramp_en) {
            if (normal_v > sleep_v || rc_v > sleep_v) {
                dcdc_ramp_map |= REG_VCORE_L_RAMP_EN;
            } else {
                dcdc_ramp_map &= ~REG_VCORE_L_RAMP_EN;
            }
        }
    }
#endif
}

int pmu_module_get_volt(unsigned char module, unsigned short *sleep_vp, unsigned short *normal_vp)
{
    unsigned short val;
    unsigned short module_volt_address;
    const struct PMU_MODULE_CFG_T *module_cfg_p = &pmu_module_cfg[module];

    module_volt_address = pmu_get_module_volt_addr(module);

    pmu_read(module_volt_address, &val);
    if (normal_vp) {
        *normal_vp = (val & module_cfg_p->normal_v) >> module_cfg_p->normal_v_shift;
    }

    if (module <= PMU_CODEC) {
        if (sleep_vp) {
            *sleep_vp = (val & module_cfg_p->dsleep_v) >> module_cfg_p->dsleep_v_shift;
        }
    } else {
        module_volt_address += 1;
        pmu_read(module_volt_address, &val);
        if (sleep_vp) {
            *sleep_vp = (val & module_cfg_p->dsleep_v) >> module_cfg_p->dsleep_v_shift;
        }
    }

#ifdef PMU_LDO_VCORE_CALIB
    if (module == PMU_CORE_L) {
        if (normal_vp) {
            *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_ldo_dig_l_comp, MAX_LDO_VCORE_L_BIT_VAL);
        }
        if (sleep_vp) {
            *sleep_vp = pmu_reg_val_add(*sleep_vp, -pmu_ldo_lp_dig_l_comp, MAX_LDO_VCORE_L_BIT_VAL);
        }
    }
#endif

    return 0;
}

static void pmu_module_ramp_volt(unsigned char module, unsigned short sleep_v, unsigned short normal_v)
{
    uint16_t old_normal_v;
    uint16_t old_sleep_v;

    pmu_module_get_volt(module, &old_sleep_v, &old_normal_v);

    if (old_normal_v < normal_v) {
        while (old_normal_v < normal_v) {
            if (old_normal_v + 2 < normal_v) {
                old_normal_v += 2;
            } else {
                old_normal_v = normal_v;
            }
            pmu_module_set_volt(module, sleep_v, old_normal_v);
        }
#ifndef DIG_DCDC_MODE
    } else if (old_normal_v > normal_v && module == PMU_CORE) {
        while (old_normal_v > normal_v) {
            if (old_normal_v > normal_v + 2) {
                old_normal_v -= 2;
            } else {
                old_normal_v = normal_v;
            }
            pmu_module_set_volt(module, sleep_v, old_normal_v);
            hal_sys_timer_delay_us(100);
        }
#endif
    } else if (old_normal_v != normal_v || old_sleep_v != sleep_v) {
        pmu_module_set_volt(module, sleep_v, normal_v);
    }
}

static void pmu_dcdc_ana_get_volt(unsigned short *normal_vp, unsigned short *dsleep_vp)
{
    unsigned short val;

    pmu_read(PMU_REG_BUCK_VANA_CFG, &val);
    if (normal_vp) {
        *normal_vp = GET_BITFIELD(val, REG_BUCK_VANA_VBIT_NORMAL);
    }

    pmu_read(PMU_REG_BUCK_VANA_LP_CFG, &val);
    if (dsleep_vp) {
        *dsleep_vp = GET_BITFIELD(val, REG_BUCK_VANA_VBIT_DSLEEP);
    }

#ifdef PMU_DCDC_CALIB
    if (normal_vp) {
        *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_dcdc_ana_comp, MAX_BUCK_VANA_BIT_VAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = pmu_reg_val_add(*dsleep_vp, -pmu_ana_lp_comp, MAX_BUCK_VANA_BIT_VAL);
    }
#endif
}

static void pmu_dcdc_ana_set_volt(unsigned short normal_v,unsigned short dsleep_v)
{
    unsigned short val;
    unsigned short rc_v = normal_v;

#ifdef PMU_DCDC_CALIB
    normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_ana_comp, MAX_BUCK_VANA_BIT_VAL);
    dsleep_v = pmu_reg_val_add(dsleep_v, pmu_ana_lp_comp, MAX_BUCK_VANA_BIT_VAL);
#if !defined(PMU_FORCE_LP_MODE) && !defined(FORCE_BIG_BANDGAP)
    rc_v = pmu_reg_val_add(rc_v, pmu_ana_lp_comp, MAX_BUCK_VANA_BIT_VAL);
#else
    rc_v = normal_v;
#endif
#endif

    pmu_read(PMU_REG_BUCK_VANA_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_VANA_VBIT_NORMAL, normal_v);
    pmu_write(PMU_REG_BUCK_VANA_CFG, val);

    pmu_read(PMU_REG_BUCK_VANA_LP_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_VANA_VBIT_DSLEEP, dsleep_v);
    pmu_write(PMU_REG_BUCK_VANA_LP_CFG, val);

    pmu_read(PMU_REG_BUCK_VANA_RC_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_VANA_VBIT_RC, rc_v);
    pmu_write(PMU_REG_BUCK_VANA_RC_CFG, val);

    if (!dcdc_ramp_en) {
        if (normal_v > dsleep_v || rc_v > dsleep_v) {
            dcdc_ramp_map |= REG_DCDC2_RAMP_EN;
        } else {
            dcdc_ramp_map &= ~REG_DCDC2_RAMP_EN;
        }
    }
}

static void pmu_ana_set_volt(enum PMU_POWER_MODE_T mode)
{
    uint32_t delay_us;
    uint16_t old_act_dcdc;
    uint16_t old_lp_dcdc;
    uint16_t new_act_dcdc;

    new_act_dcdc = ana_act_dcdc;

    if (mode == PMU_POWER_MODE_ANA_DCDC || mode == PMU_POWER_MODE_DIG_DCDC) {
        pmu_dcdc_ana_get_volt(&old_act_dcdc, &old_lp_dcdc);
        if (old_act_dcdc < new_act_dcdc) {
            if (dcdc_ramp_en) {
                pmu_dcdc_ana_set_volt(new_act_dcdc, ana_lp_dcdc);
            } else {
                while (old_act_dcdc++ < new_act_dcdc) {
                    pmu_dcdc_ana_set_volt(old_act_dcdc, ana_lp_dcdc);
                }
            }
            if (dcdc_ramp_en) {
                delay_us = (new_act_dcdc - old_act_dcdc) / PMU_HW_RAMP_STEP_HALF_PER_CYCLE * PMU_DCDC_HW_RAMP_1_CLK_TIME_US + 100;
                hal_sys_timer_delay_us(delay_us);
            } else {
                hal_sys_timer_delay_us(PMU_VANA_STABLE_TIME_US);
            }
        } else if (old_act_dcdc != new_act_dcdc || old_lp_dcdc != ana_lp_dcdc) {
            pmu_dcdc_ana_set_volt(new_act_dcdc, ana_lp_dcdc);
        }
    } else {
        pmu_dcdc_ana_set_volt(new_act_dcdc, ana_lp_dcdc);
    }
}

static void pmu_dcdc_dig_get_volt(unsigned short *normal_vp, unsigned short *dsleep_vp)
{
    unsigned short val;

    pmu_read(PMU_REG_BUCK_VCORE_CFG, &val);
    if (normal_vp) {
        *normal_vp = GET_BITFIELD(val, REG_BUCK_VCORE_VBIT_NORMAL);
    }

    pmu_read(PMU_REG_BUCK_VCORE_LP_CFG, &val);
    if (dsleep_vp) {
        *dsleep_vp = GET_BITFIELD(val, REG_BUCK_VCORE_VBIT_DSLEEP);
    }

#ifdef PMU_DCDC_CALIB
    if (normal_vp) {
        *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = pmu_reg_val_add(*dsleep_vp, -pmu_dig_lp_comp, MAX_BUCK_VCORE_BIT_VAL);
    }
#endif
}

static void pmu_dcdc_dig_set_volt(unsigned short normal_v,unsigned short dsleep_v)
{
    unsigned short val;
    unsigned short rc_v =
#ifdef PMU_VCORE_RC_MODE_FIXED
        PMU_VCORE_RC_MODE_FIXED_DCDC_DIG;
#else
        normal_v;
#endif

#ifdef PMU_DCDC_CALIB
    normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
    dsleep_v = pmu_reg_val_add(dsleep_v, pmu_dig_lp_comp, MAX_BUCK_VCORE_BIT_VAL);
#if !defined(PMU_FORCE_LP_MODE) && !defined(FORCE_BIG_BANDGAP)
    rc_v = pmu_reg_val_add(rc_v, pmu_dig_lp_comp, MAX_BUCK_VCORE_BIT_VAL);
#else
    rc_v = normal_v;
#endif
#endif

    pmu_read(PMU_REG_BUCK_VCORE_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_VCORE_VBIT_NORMAL, normal_v);
    pmu_write(PMU_REG_BUCK_VCORE_CFG, val);

    pmu_read(PMU_REG_BUCK_VCORE_LP_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_VCORE_VBIT_DSLEEP, dsleep_v);
    pmu_write(PMU_REG_BUCK_VCORE_LP_CFG, val);

#ifndef PMU_VCORE_RC_MODE_FIXED
    pmu_read(PMU_REG_BUCK_VCORE_RC_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_VCORE_VBIT_RC, rc_v);
    pmu_write(PMU_REG_BUCK_VCORE_RC_CFG, val);
#endif

    if (!dcdc_ramp_en) {
        if (normal_v > dsleep_v || rc_v > dsleep_v) {
            dcdc_ramp_map |= REG_DCDC1_RAMP_EN;
        } else {
            dcdc_ramp_map &= ~REG_DCDC1_RAMP_EN;
        }
    }
}

#ifdef MTEST_ENABLED
bool pmu_dig_set_volt_value(uint16_t dcdc_volt_mv, uint16_t logic_volt_mv, uint16_t ldo_volt_mv)
{
    uint16_t dcdc_volt, old_dcdc_volt;
    uint16_t POSSIBLY_UNUSED logic_volt;
    uint32_t i, find = 0;
    bool success = false;

    const static uint16_t dcdc_volt_list[][2] = {
        {1100, PMU_DCDC_DIG_1_1V },
        {1050, PMU_DCDC_DIG_1_05V},
        {1000, PMU_DCDC_DIG_1_0V },
        {950,  PMU_DCDC_DIG_0_95V},
        {900,  PMU_DCDC_DIG_0_9V },
        {850,  PMU_DCDC_DIG_0_85V},
        {840,  PMU_DCDC_DIG_0_84V},
        {800,  PMU_DCDC_DIG_0_8V },
        {750,  PMU_DCDC_DIG_0_75V},
        {740,  PMU_DCDC_DIG_0_74V},
        {700,  PMU_DCDC_DIG_0_7V },
        {650,  PMU_DCDC_DIG_0_65V},
        {600,  PMU_DCDC_DIG_0_6V },
        {550,  PMU_DCDC_DIG_0_55V},
        {500,  PMU_DCDC_DIG_0_5V },
    };

    const static uint16_t logic_volt_list[][2] = {
        {1050, PMU_VDIG_L_1_05V},
        {1000, PMU_VDIG_L_1_0V },
        {950,  PMU_VDIG_L_0_95V},
        {900,  PMU_VDIG_L_0_9V },
        {850,  PMU_VDIG_L_0_85V},
        {800,  PMU_VDIG_L_0_8V },
        {750,  PMU_VDIG_L_0_75V},
        {700,  PMU_VDIG_L_0_7V },
        {650,  PMU_VDIG_L_0_65V},
        {600,  PMU_VDIG_L_0_6V },
        {550,  PMU_VDIG_L_0_55V},
        {500,  PMU_VDIG_L_0_5V },
        {450,  PMU_VDIG_L_0_45V},
    };

    for (i = 0; i < ARRAY_SIZE(dcdc_volt_list); i++) {
        if (dcdc_volt_mv == dcdc_volt_list[i][0]) {
            dcdc_volt = dcdc_volt_list[i][1];
            find |= 0x1;
            break;
        }
    }
    for (i = 0; i < ARRAY_SIZE(logic_volt_list); i++) {
        if (logic_volt_mv == logic_volt_list[i][0]) {
            logic_volt = logic_volt_list[i][1];
            find |= 0x2;
            break;
        }
    }
    if (find == 0x3) {
        pmu_dcdc_dig_get_volt(&old_dcdc_volt, NULL);
        if (old_dcdc_volt < dcdc_volt) {
            while (old_dcdc_volt++ < dcdc_volt) {
                pmu_dcdc_dig_set_volt(old_dcdc_volt, dig_lp_dcdc);
            }
#ifdef SPLIT_CORE_M_CORE_L
            pmu_module_set_volt(PMU_CORE_L, dig_lp_logic, logic_volt);
#endif
        } else if (old_dcdc_volt != dcdc_volt) {
#ifdef SPLIT_CORE_M_CORE_L
            pmu_module_set_volt(PMU_CORE_L, dig_lp_logic, logic_volt);
#endif
            pmu_dcdc_dig_set_volt(dcdc_volt, dig_lp_dcdc);
        }
        success = true;
    }
    return success;
}
#endif /* MTEST_ENABLED */

static void BOOT_TEXT_SRAM_LOC pmu_dig_get_target_volt(uint16_t *ldo, uint16_t *dcdc, uint16_t *logic)
{
    uint16_t ldo_volt;
    uint16_t dcdc_volt;
    uint16_t logic_volt;

    if (0) {
    } else if (pmu_vcore_req & PMU_VCORE_PMU_RESET) {
        ldo_volt = PMU_VDIG_DEFAULT;
        dcdc_volt = PMU_DCDC_DIG_0_85V - PMU_DCDC_DIG_50_MV;
        logic_volt = PMU_VDIG_L_DEFAULT;
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    } else if (pmu_vcore_req & (PMU_VCORE_SYS_FREQ_HIGH)) {
        if (high_perf_freq_mhz <= 260) {
            ldo_volt = PMU_VDIG_1_05V;
            dcdc_volt = PMU_DCDC_DIG_1_05V;
            logic_volt = PMU_VDIG_L_1_0V;
        } else {
            ldo_volt = PMU_VDIG_1_1V;
            dcdc_volt = PMU_DCDC_DIG_1_1V;
            logic_volt = PMU_VDIG_L_1_05V;
        }
#endif
    } else if (pmu_vcore_req & PMU_VCORE_PSRAM_FREQ_HIGH) {
        ldo_volt = PMU_VDIG_0_9V;
        dcdc_volt = PMU_DCDC_DIG_0_9V;
        logic_volt = PMU_VDIG_L_0_85V;
    } else if (pmu_vcore_req & (PMU_VCORE_RS_FREQ_HIGH | PMU_VCORE_SYS_FREQ_MEDIUM)) {
        ldo_volt = PMU_VDIG_0_85V;
        dcdc_volt = PMU_DCDC_DIG_0_85V;
        logic_volt = PMU_VDIG_L_0_8V;
    } else if (pmu_vcore_req & PMU_VCORE_PSRAM_FREQ_MEDIUM) {
        ldo_volt = PMU_VDIG_0_8V;
        dcdc_volt = PMU_DCDC_DIG_0_8V;
        logic_volt = PMU_VDIG_L_0_75V;
    } else if (pmu_vcore_req & (PMU_VCORE_FLASH_FREQ_HIGH | PMU_VCORE_FLASH_WRITE_ENABLED | PMU_VCORE_PSRAM_FREQ_MEDIUM_LOW)) {
        ldo_volt = PMU_VDIG_0_75V;
        dcdc_volt = PMU_DCDC_DIG_0_75V;
        logic_volt = PMU_VDIG_L_0_7V;
    } else {
        // Common cases
        ldo_volt = PMU_VDIG_0_7V;
        dcdc_volt = PMU_DCDC_DIG_0_7V;
        logic_volt = PMU_VDIG_L_0_65V;
    }

#ifdef PMU_VCORE_RISE_MV
    ldo_volt += (PMU_VDIG_0_8V - PMU_VDIG_0_75V) * (PMU_VCORE_RISE_MV / 25) / 2;
    dcdc_volt += (PMU_DCDC_DIG_0_8V - PMU_DCDC_DIG_0_75V) * (PMU_VCORE_RISE_MV / 25) / 2;
    logic_volt += (PMU_VDIG_L_0_8V - PMU_VDIG_L_0_75V) * (PMU_VCORE_RISE_MV / 25) / 2;
#endif

#if (defined(PROGRAMMER) || defined(__BES_OTA_MODE__)) && !defined(PMU_FULL_INIT)
    // Try to keep the same vcore voltage as ROM (hardware default)
    if (ldo_volt < PMU_VDIG_DEFAULT) {
        ldo_volt = PMU_VDIG_DEFAULT;
    }
    if (dcdc_volt < PMU_DCDC_DIG_DEFAULT) {
        dcdc_volt = PMU_DCDC_DIG_DEFAULT;
    }
    if (logic_volt < PMU_VDIG_L_DEFAULT) {
        logic_volt = PMU_VDIG_L_DEFAULT;
    }
#endif

#ifdef DIG_DCDC_MODE
    if (ldo_volt < PMU_VDIG_DEFAULT) {
        ldo_volt = PMU_VDIG_DEFAULT;
    }
#endif

    if (ldo) {
        *ldo = ldo_volt;
    }
    if (dcdc) {
        *dcdc = dcdc_volt;
    }
    if (logic) {
        *logic = logic_volt;
    }
}

static void pmu_dig_set_volt(enum PMU_POWER_MODE_T mode)
{
    uint32_t lock;
    uint32_t delay_us;
    uint32_t delta_raw;
    uint16_t dcdc_volt, old_act_dcdc, old_lp_dcdc;
    uint16_t ldo_volt, old_act_ldo, old_lp_ldo;
    uint16_t logic_volt;
    bool volt_inc = false;
#ifdef SPLIT_CORE_M_CORE_L
    uint16_t old_act_logic, old_lp_logic;
#endif
    const uint8_t ramp_step = 24; // spi 1m

    lock = int_lock();

    if (mode == PMU_POWER_MODE_NONE) {
        mode = pmu_power_mode;
    }

    pmu_dig_get_target_volt(&ldo_volt, &dcdc_volt, &logic_volt);

    pmu_module_get_volt(PMU_CORE, &old_lp_ldo, &old_act_ldo);
    pmu_dcdc_dig_get_volt(&old_act_dcdc, &old_lp_dcdc);
#ifdef SPLIT_CORE_M_CORE_L
    pmu_module_get_volt(PMU_CORE_L, &old_lp_logic, &old_act_logic);
#endif

    if (mode == PMU_POWER_MODE_DIG_DCDC) {
        if (old_act_dcdc < dcdc_volt) {
            volt_inc = true;
            if (dcdc_ramp_en) {
                pmu_dcdc_dig_set_volt(dcdc_volt, dig_lp_dcdc);
            } else {
                while (old_act_dcdc < dcdc_volt) {
                    if (old_act_dcdc + ramp_step < dcdc_volt) {
                        old_act_dcdc += ramp_step;
                    } else {
                        old_act_dcdc = dcdc_volt;
                    }
                    pmu_dcdc_dig_set_volt(old_act_dcdc, dig_lp_dcdc);
                }
            }
        } else if (old_act_dcdc != dcdc_volt || old_lp_dcdc != dig_lp_dcdc) {
#ifdef SPLIT_CORE_M_CORE_L
            pmu_module_set_volt(PMU_CORE_L, dig_lp_logic, logic_volt);
#endif
            pmu_dcdc_dig_set_volt(dcdc_volt, dig_lp_dcdc);
        }
        // Update the voltage of the other mode
        pmu_module_set_volt(PMU_CORE, dig_lp_ldo, ldo_volt);
    } else {
        if (old_act_ldo < ldo_volt) {
            volt_inc = true;
        }
#ifdef SPLIT_CORE_M_CORE_L
        if (!volt_inc) {
            pmu_module_set_volt(PMU_CORE_L, dig_lp_logic, logic_volt);
        }
#endif
        pmu_module_ramp_volt(PMU_CORE, dig_lp_ldo, ldo_volt);
        // Update the voltage of the other mode
        pmu_dcdc_dig_set_volt(dcdc_volt, dig_lp_dcdc);
    }

    if (volt_inc) {
        delta_raw = dcdc_volt - old_act_dcdc;
#ifdef SPLIT_CORE_M_CORE_L
        if (dcdc_ramp_en) {
            uint32_t delta_raw_logic;

            pmu_module_set_volt(PMU_CORE_L, dig_lp_logic, logic_volt);
            delta_raw_logic = logic_volt - old_act_logic;
            delta_raw = (delta_raw > delta_raw_logic) ? delta_raw : delta_raw_logic;
        } else {
            pmu_module_ramp_volt(PMU_CORE_L, dig_lp_logic, logic_volt);
        }
#endif
        if (dcdc_ramp_en) {
            delay_us = delta_raw / PMU_HW_RAMP_STEP_PER_CYCLE * PMU_DCDC_HW_RAMP_1_CLK_TIME_US + 100;
            hal_sys_timer_delay_us(delay_us);
        } else {
            hal_sys_timer_delay_us(PMU_VCORE_STABLE_TIME_US);
        }
    }

    int_unlock(lock);
}

static void pmu_ana_reserved_en(bool en)
{
    uint16_t val;

    pmu_read(PMU_REG_RESERVED_ANA, &val);
    if (en) {
        val |= RESERVED_ANA_EN;
    } else {
        val &= ~RESERVED_ANA_EN;
    }
    pmu_write(PMU_REG_RESERVED_ANA, val);
}

static void pmu_ldo_mode_en(void)
{
    unsigned short val;

    // Enable vana ldo
    pmu_module_config(PMU_ANA,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);
    // Enable vcore ldo
    pmu_module_config(PMU_CORE,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);

    hal_sys_timer_delay_us(PMU_LDO_PU_STABLE_TIME_US);

    // The load under ana_ldo should be turned on after ana_ldo enabled.
    pmu_ana_reserved_en(true);

    // Disable vana dcdc
    pmu_read(PMU_REG_BUCK_VANA_EN, &val);
    val = (val & ~REG_PU_BUCK_VANA) | REG_PU_BUCK_VANA_DR;
    pmu_write(PMU_REG_BUCK_VANA_EN, val);

    // Disable vcore dcdc
    pmu_read(PMU_REG_BUCK_VCORE_EN, &val);
    val = (val & ~REG_PU_BUCK_VCORE) | REG_PU_BUCK_VCORE_DR;
    pmu_write(PMU_REG_BUCK_VCORE_EN, val);
}

static void pmu_dcdc_ana_mode_en(void)
{
    unsigned short val;

    if (pmu_power_mode == PMU_POWER_MODE_DIG_DCDC) {
        // Enable vcore ldo
        pmu_module_config(PMU_CORE,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);

        hal_sys_timer_delay_us(PMU_LDO_PU_STABLE_TIME_US);

        // Disable vcore dcdc
        pmu_read(PMU_REG_BUCK_VCORE_EN, &val);
        val = (val & ~REG_PU_BUCK_VCORE) | REG_PU_BUCK_VCORE_DR;
        pmu_write(PMU_REG_BUCK_VCORE_EN, val);
    } else {
        pmu_read(PMU_REG_BUCK_VANA_CFG, &val);
#ifdef PMU_DCDC_PWM
        val &= ~REG_BUCK_VANA_BURST_MODE_SEL_NORMAL;
#else
        val |= REG_BUCK_VANA_BURST_MODE_SEL_NORMAL;
#endif
        pmu_write(PMU_REG_BUCK_VANA_CFG, val);

#ifndef DCDC_ULP_RC_ON
        pmu_read(PMU_REG_BUCK_VANA_RC_CFG_140, &val);
        val &= ~REG_BUCK_VANA_ULP_MODE_RC;
        pmu_write(PMU_REG_BUCK_VANA_RC_CFG_140, val);
#endif

        // Enable vana dcdc
        pmu_read(PMU_REG_BUCK_VANA_EN, &val);
        val |= REG_PU_BUCK_VANA_DR | REG_PU_BUCK_VANA;
        pmu_write(PMU_REG_BUCK_VANA_EN, val);

        hal_sys_timer_delay_us(PMU_DCDC_PU_STABLE_TIME_US);

        // Disable vana ldo
        pmu_module_config(PMU_ANA,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);

        // The load under ana_ldo should be turned off after ana_ldo disabled.
        pmu_ana_reserved_en(false);
    }
}

static void pmu_dcdc_dual_mode_en(void)
{
    unsigned short val;

    pmu_read(PMU_REG_BUCK_VANA_CFG, &val);
#ifdef PMU_DCDC_PWM
    val &= ~REG_BUCK_VANA_BURST_MODE_SEL_NORMAL;
#else
    val |= REG_BUCK_VANA_BURST_MODE_SEL_NORMAL;
#endif
    pmu_write(PMU_REG_BUCK_VANA_CFG, val);

#ifndef DCDC_ULP_RC_ON
    pmu_read(PMU_REG_BUCK_VANA_RC_CFG_140, &val);
    val &= ~REG_BUCK_VANA_ULP_MODE_RC;
    pmu_write(PMU_REG_BUCK_VANA_RC_CFG_140, val);
#endif

    pmu_read(PMU_REG_BUCK_VCORE_CFG, &val);
#ifdef PMU_DCDC_PWM
    val &= ~REG_BUCK_VCORE_BURST_MODE_SEL_NORMAL;
#else
    val |= REG_BUCK_VCORE_BURST_MODE_SEL_NORMAL;
#endif
    pmu_write(PMU_REG_BUCK_VCORE_CFG, val);

#ifndef DCDC_ULP_RC_ON
    pmu_read(PMU_REG_BUCK_VCORE_RC_CFG_143, &val);
    val &= ~REG_BUCK_VCORE_ULP_MODE_RC;
    pmu_write(PMU_REG_BUCK_VCORE_RC_CFG_143, val);
#endif

    // Enable vana dcdc
    pmu_read(PMU_REG_BUCK_VANA_EN, &val);
    val |= REG_PU_BUCK_VANA_DR | REG_PU_BUCK_VANA;
    pmu_write(PMU_REG_BUCK_VANA_EN, val);

    // Enable vcore dcdc
    pmu_read(PMU_REG_BUCK_VCORE_EN, &val);
    val |= REG_PU_BUCK_VCORE_DR | REG_PU_BUCK_VCORE;
    pmu_write(PMU_REG_BUCK_VCORE_EN, val);

    hal_sys_timer_delay_us(PMU_DCDC_PU_STABLE_TIME_US);

    // Disable vana ldo
    pmu_module_config(PMU_ANA,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
    // Disable vcore ldo
    pmu_module_config(PMU_CORE,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);

    // The load under ana_ldo should be turned off after ana_ldo disabled.
    pmu_ana_reserved_en(false);
}

void pmu_mode_change(enum PMU_POWER_MODE_T mode)
{
    uint32_t lock;

#ifndef PMU_ALREADY_INIT
    if (pmu_power_mode == mode || mode == PMU_POWER_MODE_NONE) {
        return;
    }
#endif

    lock = int_lock();

    pmu_ana_set_volt(pmu_power_mode);
    pmu_dig_set_volt(pmu_power_mode);

    if (mode == PMU_POWER_MODE_ANA_DCDC) {
        pmu_dcdc_ana_mode_en();
    } else if (mode == PMU_POWER_MODE_DIG_DCDC) {
        pmu_dcdc_dual_mode_en();
    } else if (mode == PMU_POWER_MODE_LDO) {
        pmu_ldo_mode_en();
    }

    pmu_power_mode = mode;

    int_unlock(lock);
}

void pmu_sleep_en(unsigned char sleep_en)
{
    unsigned short val;

    pmu_read(PMU_REG_SLEEP_CFG, &val);
    if(sleep_en) {
        val |= SLEEP_ALLOW;
    } else {
        val &= ~SLEEP_ALLOW;
    }
    pmu_write(PMU_REG_SLEEP_CFG, val);

    pmu_read(PMU_REG_RC_SLEEP_CFG, &val);
    if (sleep_en) {
        val |= REG_RC_SLEEP_ALLOW;
    } else {
        val &= ~REG_RC_SLEEP_ALLOW;
    }
    pmu_write(PMU_REG_RC_SLEEP_CFG, val);
}

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#ifndef HPPA_LDO_ON
static uint32_t pmu_dcdc_hppa_mv_to_val(uint16_t mv)
{
    uint32_t val;

    if (mv == 1600) {
        val = PMU_DCDC_HPPA_1_6V;
    } else if (mv == 1650) {
        val = PMU_DCDC_HPPA_1_65V;
    } else if (mv == 1700) {
        val = PMU_DCDC_HPPA_1_7V;
    } else if (mv == 1750) {
        val = PMU_DCDC_HPPA_1_75V;
    } else if (mv == 1900) {
        val = PMU_DCDC_HPPA_1_9V;
    } else {
        val = PMU_DCDC_HPPA_1_8V;
    }

    return val;
}

POSSIBLY_UNUSED
static void pmu_dcdc_hppa_get_volt(unsigned short *normal_vp, unsigned short *dsleep_vp)
{
    unsigned short val;

    pmu_read(PMU_REG_BUCK_VHPPA_CFG, &val);
    if (normal_vp) {
        *normal_vp = GET_BITFIELD(val, REG_BUCK_VHPPA_VBIT_NORMAL);
    }

    pmu_read(PMU_REG_BUCK_VHPPA_LP_CFG, &val);
    if (dsleep_vp) {
        *dsleep_vp = GET_BITFIELD(val, REG_BUCK_VHPPA_VBIT_DSLEEP);
    }

#ifdef PMU_DCDC_CALIB
    if (normal_vp) {
        *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_dcdc_hppa_comp, MAX_BUCK_VHPPA_BIT_VAL);
    }
    if (dsleep_vp) {
        *dsleep_vp = pmu_reg_val_add(*dsleep_vp, -pmu_hppa_lp_comp, MAX_BUCK_VHPPA_BIT_VAL);
    }
#endif
}

static void pmu_dcdc_hppa_set_volt(unsigned short normal_v,unsigned short dsleep_v)
{
    unsigned short val;
    unsigned short rc_v = normal_v;

#ifdef PMU_DCDC_CALIB
    normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_hppa_comp, MAX_BUCK_VHPPA_BIT_VAL);
    dsleep_v = pmu_reg_val_add(dsleep_v, pmu_hppa_lp_comp, MAX_BUCK_VHPPA_BIT_VAL);
#if !defined(PMU_FORCE_LP_MODE) && !defined(FORCE_BIG_BANDGAP)
    rc_v = pmu_reg_val_add(rc_v, pmu_hppa_lp_comp, MAX_BUCK_VHPPA_BIT_VAL);
#else
    rc_v = normal_v;
#endif
#endif

    pmu_read(PMU_REG_BUCK_VHPPA_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_VHPPA_VBIT_NORMAL, normal_v);
    pmu_write(PMU_REG_BUCK_VHPPA_CFG, val);

    pmu_read(PMU_REG_BUCK_VHPPA_LP_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_VHPPA_VBIT_DSLEEP, dsleep_v);
    pmu_write(PMU_REG_BUCK_VHPPA_LP_CFG, val);

    pmu_read(PMU_REG_BUCK_VHPPA_RC_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_VHPPA_VBIT_RC, rc_v);
    pmu_write(PMU_REG_BUCK_VHPPA_RC_CFG, val);

    if (!dcdc_ramp_en) {
        if (normal_v > dsleep_v || rc_v > dsleep_v) {
            dcdc_ramp_map |= REG_DCDC3_RAMP_EN;
        } else {
            dcdc_ramp_map &= ~REG_DCDC3_RAMP_EN;
        }
    }
}

static void pmu_dcdc_hppa_en(int enable)
{
    uint16_t val;

    if (enable) {
        pmu_read(PMU_REG_BUCK_VHPPA_CFG, &val);
#ifdef PMU_DCDC_PWM
        val &= ~REG_BUCK_VHPPA_BURST_MODE_SEL_NORMAL;
#else
        val |= REG_BUCK_VHPPA_BURST_MODE_SEL_NORMAL;
#endif
        pmu_write(PMU_REG_BUCK_VHPPA_CFG, val);

#ifndef DCDC_ULP_RC_ON
        pmu_read(PMU_REG_BUCK_VHPPA_RC_CFG_146, &val);
        val &= ~REG_BUCK_VHPPA_ULP_MODE_RC;
        pmu_write(PMU_REG_BUCK_VHPPA_RC_CFG_146, val);
#endif
    }

    pmu_read(PMU_REG_BUCK_VHPPA_EN, &val);
    val |= REG_PU_BUCK_VHPPA_DR;
    if (enable) {
        val |= REG_PU_BUCK_VHPPA;
    } else {
        val &= ~REG_PU_BUCK_VHPPA;
    }
    pmu_write(PMU_REG_BUCK_VHPPA_EN, val);
}

static void pmu_hppa_dcdc_to_ldo(void)
{
    pmu_module_set_volt(PMU_CODEC, PMU_CODEC_DEFAULT_V, PMU_CODEC_DEFAULT_V);
    pmu_module_config(PMU_CODEC,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_OFF,PMU_DSLEEP_MODE_OFF);
    hal_sys_timer_delay_us(PMU_LDO_PU_STABLE_TIME_US);
    pmu_dcdc_hppa_en(false);
}

#ifdef PMU_VMEM_PWR_SUPPLY_IO
static uint32_t pmu_ldo_vmem_mv_to_val(uint16_t mv)
{
    uint32_t val;

    if (mv == 1600) {
        val = PMU_VMEM_1_6V;
    } else if (mv == 1700) {
        val = PMU_VMEM_1_7V;
    } else {
        val = PMU_VMEM_1_8V;
    }

    return val;
}
#endif
#endif

int pmu_codec_volt_ramp_up(void)
{
    return 0;
}

int pmu_codec_volt_ramp_down(void)
{
    return 0;
}

static void BOOT_TEXT_FLASH_LOC pmu_dig_init_volt(void)
{
    uint16_t ldo_volt;
    uint16_t val;

    pmu_dig_get_target_volt(&ldo_volt, NULL, NULL);

    pmu_read(PMU_REG_LDO_CORE_VOLT, &val);
    if (GET_BITFIELD(val, REG_LDO_VCORE_VBIT_NORMAL) < ldo_volt) {
        val = SET_BITFIELD(val, REG_LDO_VCORE_VBIT_NORMAL, ldo_volt);
        pmu_write(PMU_REG_LDO_CORE_VOLT, val);
    }
}

#ifdef PMU_REG_CFG_DUMP
static void pmu_reg_val_print(void)
{
    uint16_t val;

    TRACE(0, "%s start", __func__);
    for (uint16_t reg = 0x00; reg <= 0xFE; reg++) {
        pmu_read(reg, &val);
        TRACE_IMM(0, "reg=val 0x%04X=0x%04X", reg, val);
    }

    for (uint16_t reg = 0x10A; reg <= 0x158; reg++) {
        pmu_read(reg, &val);
        TRACE_IMM(0, "reg=val 0x%04X=0x%04X", reg, val);
    }

    for (uint16_t reg = PMU_EFUSE_PAGE_SECURITY; reg <= PMU_EFUSE_PAGE_RESERVED_0X1F; reg++) {
        pmu_get_efuse(reg, &val);
        TRACE_IMM(0, "efuse=val 0x%04X=0x%04X", reg, val);
    }
    TRACE(0, "%s done", __func__);
}
#endif

#ifdef PMU_VCORE_RC_MODE_FIXED
// Don't need ramp because MCU don't use RC clock.
static void pmu_vcore_rc_mode_init(void)
{
    uint16_t rc_v;
    uint16_t val;

    // VCORE-M-DCDC-RC
    rc_v = PMU_VCORE_RC_MODE_FIXED_DCDC_DIG;

#ifdef PMU_DCDC_CALIB
#if !defined(PMU_FORCE_LP_MODE) && !defined(FORCE_BIG_BANDGAP)
    rc_v = pmu_reg_val_add(rc_v, pmu_dig_lp_comp, MAX_BUCK_VCORE_BIT_VAL);
#else
    rc_v = pmu_reg_val_add(rc_v, pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
#endif
#endif

    pmu_read(PMU_REG_BUCK_VCORE_RC_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_VCORE_VBIT_RC, rc_v);
    pmu_write(PMU_REG_BUCK_VCORE_RC_CFG, val);

    // VCORE-M-LDO-RC
    pmu_read(PMU_REG_LDO_CORE_VOLT, &val);
    val = SET_BITFIELD(val, REG_LDO_VCORE_VBIT_RC, PMU_VCORE_RC_MODE_FIXED_VDIG);
    pmu_write(PMU_REG_LDO_CORE_VOLT, val);

#ifdef SPLIT_CORE_M_CORE_L
    // VCORE-L-RC
    rc_v = PMU_VCORE_RC_MODE_FIXED_VDIG_L;

#ifdef PMU_LDO_VCORE_CALIB
#if !defined(PMU_FORCE_LP_MODE) && !defined(FORCE_BIG_BANDGAP)
    rc_v = pmu_reg_val_add(rc_v, pmu_ldo_lp_dig_l_comp, MAX_LDO_VCORE_L_BIT_VAL);
#else
    rc_v = pmu_reg_val_add(rc_v, pmu_ldo_dig_l_comp, MAX_LDO_VCORE_L_BIT_VAL);
#endif
#endif

    pmu_read(PMU_REG_LDO_CORE_L_LP_RC_VOLT, &val);
    val = SET_BITFIELD(val, REG_LDO_VCORE_L_VBIT_RC, rc_v);
    pmu_write(PMU_REG_LDO_CORE_L_LP_RC_VOLT, val);
#endif
}
#endif

#ifdef PMU_VMIN_SEARCH
#ifdef RTC_CALENDAR
#error "PMU_VMIN_SEARCH will use RTC reg, please check it."
#endif

static void pmu_vmin_search_enable(void)
{
    uint16_t val;
    int8_t vmin_offset;

    TRACE(0, "%s reg_addr=0x%x", __func__, PMU_VMIN_SET_REG_ADDR);

    pmu_write(PMU_VMIN_SET_REG_ADDR, 0);

    while (1) {
        pmu_read(PMU_VMIN_SET_REG_ADDR, &val);
        if (val & PMU_VMIN_VCORE_SET_DONE) {
            break;
        }
    }

    if (val & PMU_VMIN_VCORE_SET_PLUS) {
        vmin_offset = GET_BITFIELD(val, PMU_VMIN_VCORE_SET_OFFSET);
    } else {
        vmin_offset = -GET_BITFIELD(val, PMU_VMIN_VCORE_SET_OFFSET);
    }

    pmu_dcdc_dig_comp += vmin_offset;

    TRACE(0, "%s vmin_offset%d pmu_dcdc_dig_comp=%d", __func__, vmin_offset, pmu_dcdc_dig_comp);
}
#endif

// Target voltage 1.8V
static void pmu_set_sar_adc_vref(void)
{
    int8_t vref_comp;
    uint16_t val_efuse;
    uint16_t val_sar_vreg_sel = 0x17;
    uint16_t val;

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_0X14, &val_efuse);
    vref_comp = GET_BITFIELD(val_efuse, PMU_EFUSE_SAR_ADC_VREF_COMP);
    if (val_efuse & PMU_EFUSE_SAR_ADC_VREF_COMP_SIGN) {
        vref_comp = -vref_comp;
    }

    pmu_read(PMU_REG_SAR_VREF_CFG_E6, &val);
    val_sar_vreg_sel += vref_comp;
    val = SET_BITFIELD(val, REG_SAR_VREG_SEL, val_sar_vreg_sel);
    pmu_write(PMU_REG_SAR_VREF_CFG_E6, val);

    TRACE(0, "%s Calibration done! vref_comp=%d", __func__, vref_comp);
}

#ifdef PMU_FORCE_LP_MODE
static void pmu_bandgap_cfg_reset(void)
{
    uint16_t val;

    pmu_read(PMU_REG_BIAS_CFG, &val);
    val |= PU_BIAS_LDO_DR | BG_CONSTANT_GM_BIAS_DR | BG_CORE_EN_DR | BG_VTOI_EN_DR | BG_NOTCH_EN_DR | BG_VBG_SEL_DR |
        PU_BIAS_LDO_REG | BG_CONSTANT_GM_BIAS_REG | BG_CORE_EN_REG | BG_VTOI_EN_REG | BG_NOTCH_EN_REG;
    pmu_write(PMU_REG_BIAS_CFG, val);
    hal_sys_timer_delay_us(PMU_BIG_BG_STABLE_TIME_US);

    val |= BG_VBG_SEL_REG;
    pmu_write(PMU_REG_BIAS_CFG, val);
}
#endif
#endif

#ifdef FAST_HW_RAMP
static void pmu_buck_volt_hw_ramp_step_set(bool sleep, uint8_t step)
{
    uint16_t val;

    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_3) {
        pmu_read(PMU_REG_MEM_CFG, &val);
        if (sleep) {
            // Use rc2m to fast ramp voltage for fast wakeup
            val |= REG_RMAP_MODE_SEL;
        } else {
            // Use 32k to ramp voltage for normal mode
            val &= ~REG_RMAP_MODE_SEL;
        }
        pmu_write(PMU_REG_MEM_CFG, val);

        pmu_read(PMU_REG_BUCK_VCORE_EN, &val);
        val = SET_BITFIELD(val, REG_BUCK_VCORE_RAMP_STEP, step);
        pmu_write(PMU_REG_BUCK_VCORE_EN, val);

        step = (step / 2) > 0 ? (step / 2) : 1;

        pmu_read(PMU_REG_BUCK_VANA_EN, &val);
        val = SET_BITFIELD(val, REG_BUCK_VANA_RAMP_STEP, step);
        pmu_write(PMU_REG_BUCK_VANA_EN, val);

        pmu_read(PMU_REG_BUCK_VHPPA_EN, &val);
        val = SET_BITFIELD(val, REG_BUCK_VHPPA_RAMP_STEP, step);
        pmu_write(PMU_REG_BUCK_VHPPA_EN, val);

#ifdef SPLIT_CORE_M_CORE_L
        pmu_read(PMU_REG_LDO_CORE_L_CFG, &val);
        val = SET_BITFIELD(val, REG_LDO_VCORE_L_RAMP_STEP, step);
        pmu_write(PMU_REG_LDO_CORE_L_CFG, val);
#endif
    }
}
#endif

#ifndef DCDC_VOLT_HW_RAMP_ONLY
static void pmu_buck_volt_hw_ramp_enable(uint8_t map)
{
    uint16_t val;

    if (dcdc_ramp_map & REG_DCDC1_RAMP_EN) {
        pmu_read(PMU_REG_BUCK_VCORE_EN, &val);
        val |= REG_BUCK_VCORE_RAMP_EN;
        pmu_write(PMU_REG_BUCK_VCORE_EN, val);
    }

    if (dcdc_ramp_map & REG_DCDC2_RAMP_EN) {
        pmu_read(PMU_REG_BUCK_VANA_EN, &val);
        val |= REG_BUCK_VANA_RAMP_EN;
        pmu_write(PMU_REG_BUCK_VANA_EN, val);
    }

    if (dcdc_ramp_map & REG_DCDC3_RAMP_EN) {
        pmu_read(PMU_REG_BUCK_VHPPA_EN, &val);
        val |= REG_BUCK_VHPPA_RAMP_EN;
        pmu_write(PMU_REG_BUCK_VHPPA_EN, val);
    }

#ifdef SPLIT_CORE_M_CORE_L
    if (dcdc_ramp_map & REG_VCORE_L_RAMP_EN) {
        pmu_read(PMU_REG_LDO_CORE_L_CFG, &val);
        val |= REG_LDO_VCORE_L_RAMP_EN;
        pmu_write(PMU_REG_LDO_CORE_L_CFG, val);
    }
#endif
}

static void pmu_buck_volt_hw_ramp_disable(uint8_t map)
{
    uint16_t val;

    if (dcdc_ramp_map & REG_DCDC1_RAMP_EN) {
        pmu_read(PMU_REG_BUCK_VCORE_EN, &val);
        val &= ~REG_BUCK_VCORE_RAMP_EN;
        pmu_write(PMU_REG_BUCK_VCORE_EN, val);
    }

    if (dcdc_ramp_map & REG_DCDC2_RAMP_EN) {
        pmu_read(PMU_REG_BUCK_VANA_EN, &val);
        val &= ~REG_BUCK_VANA_RAMP_EN;
        pmu_write(PMU_REG_BUCK_VANA_EN, val);
    }

    if (dcdc_ramp_map & REG_DCDC3_RAMP_EN) {
        pmu_read(PMU_REG_BUCK_VHPPA_EN, &val);
        val &= ~REG_BUCK_VHPPA_RAMP_EN;
        pmu_write(PMU_REG_BUCK_VHPPA_EN, val);
    }

#ifdef SPLIT_CORE_M_CORE_L
    if (dcdc_ramp_map & REG_VCORE_L_RAMP_EN) {
        pmu_read(PMU_REG_LDO_CORE_L_CFG, &val);
        val &= ~REG_LDO_VCORE_L_RAMP_EN;
        pmu_write(PMU_REG_LDO_CORE_L_CFG, val);
    }
#endif
}
#endif

static void pmu_sar_adc_vref_sw_pu(bool pu)
{
    uint16_t val;

    pmu_read(PMU_REG_SAR_VREF_CFG_E6, &val);
    if (pu) {
        val &= ~REG_SAR_PD_VREF_ADC_SW;
    } else {
        val |= REG_SAR_PD_VREF_ADC_SW;
    }
    pmu_write(PMU_REG_SAR_VREF_CFG_E6, val);
}

POSSIBLY_UNUSED static BOOT_BSS_LOC bool pmu_opened = false;

int BOOT_TEXT_FLASH_LOC pmu_open(void)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))

    uint16_t val;
    enum PMU_POWER_MODE_T mode;

    if (pmu_opened) {
        return 0;
    }

    pmu_opened = true;

    ASSERT(vcodec_mv == 1400 || vcodec_mv == 1450 || vcodec_mv == 1500 || vcodec_mv == 1550 || vcodec_mv == 1600 || vcodec_mv == 1650 ||
        vcodec_mv == 1700 || vcodec_mv == 1750 || vcodec_mv == 1800 || vcodec_mv == 1900,
        "Invalid vcodec cfg: vcodec_mv=%u", vcodec_mv);
    ASSERT(vhppa_mv == 1400 || vhppa_mv == 1450 || vhppa_mv == 1500 || vhppa_mv == 1550 || vhppa_mv == 1600 || vhppa_mv == 1650 ||
        vhppa_mv == 1700 || vhppa_mv == 1750 || vhppa_mv == 1800 || vhppa_mv == 1900,
        "Invalid vhppa cfg: vhppa_mv=%u", vhppa_mv);
    TRACE(0, "%s vcodec_mv:%d vhppa_mv:%d", __func__, vcodec_mv, vhppa_mv);

#ifdef PMU_VMEM_PWR_SUPPLY_IO
    ASSERT(vmem_mv == 1600 || vmem_mv == 1700 || vmem_mv == 1800, "Invalid vmem cfg: vhppa_mv=%u", vmem_mv);
    TRACE(0, "%s vmem_mv:%d", __func__, vmem_mv);
#endif

#if !defined(ROM_BUILD) && defined(PMU_EFUSE_INIT_IN_BOOT)
    pmu_efuse_init();
#endif

#ifdef PMU_WAFER_PRINT
    pmu_get_efuse(REG_CP_WAFER_CTX_ID, &val);
    TRACE(0, "wafer_ctx_print: wafer_ID=%d", GET_BITFIELD(val, PMU_EFUSE_CP_WAFER_CTX_ID));

    pmu_get_efuse(REG_CP_WAFER_CTX_XY, &val);
    TRACE(0, "wafer_ctx_print: wafer_X=%d, wafer_Y=%d",
        GET_BITFIELD(val, PMU_EFUSE_CP_WAFER_CTX_X), GET_BITFIELD(val, PMU_EFUSE_CP_WAFER_CTX_Y));
#endif

    TRACE(0, "%s boot_cause=0x%04X first_pwron:%d noapp_test", __func__, pmu_boot_reason, pmu_boot_first_pwr_up);

    // Disable and clear all PMU irqs by default
    pmu_write(PMU_REG_INT_MASK, 0);
    pmu_write(PMU_REG_INT_EN, 0);
    // PMU irqs cannot be cleared by PMU soft reset
    pmu_read(PMU_REG_CHARGER_STATUS, &val);
    pmu_write(PMU_REG_CHARGER_STATUS, val);
    pmu_read(PMU_REG_INT_STATUS, &val);
    pmu_write(PMU_REG_INT_CLR, val);

    // Allow PMU to sleep when power key is pressed
    pmu_read(PMU_REG_POWER_KEY_CFG, &val);
    val &= ~POWERKEY_WAKEUP_OSC_EN;
    pmu_write(PMU_REG_POWER_KEY_CFG, val);

#ifndef PMU_FORCE_LP_MODE
    pmu_read(PMU_REG_BIAS_CFG, &val);
#ifdef FORCE_BIG_BANDGAP
    val |= PU_BIAS_LDO_DR | BG_VBG_SEL_DR | BG_CONSTANT_GM_BIAS_DR | BG_CORE_EN_DR | BG_VTOI_EN_DR;
#else
    // Allow low power bandgap
    val &= ~BG_VBG_SEL_DR;
#endif
    pmu_write(PMU_REG_BIAS_CFG, val);
#endif

    // Disable sar vref output
    pmu_sar_adc_vref_sw_pu(false);
    pmu_set_sar_adc_vref();

#ifdef PMU_IRQ_UNIFIED
    pmu_read(PMU_REG_WDT_CFG, &val);
    val |= POWERON_DETECT_EN | MERGE_INTR;
    pmu_write(PMU_REG_WDT_CFG, val);
#endif

#if defined(PMU_DCDC_CALIB) && !defined(PMU_ALREADY_INIT)
    pmu_get_dcdc_calib_value();
    TRACE(0, "normal/lp vcore-m=%d/%d vana=%d/%d vcodec=%d/%d",
            pmu_dcdc_dig_comp, pmu_dig_lp_comp, pmu_dcdc_ana_comp, pmu_ana_lp_comp, pmu_dcdc_hppa_comp, pmu_hppa_lp_comp);
#endif

#if defined(PMU_LDO_VCORE_CALIB) && !defined(PMU_ALREADY_INIT)
    pmu_get_ldo_dig_calib_value();
    TRACE(0, "normal/lp vcore-l=%d/%d", pmu_ldo_dig_l_comp, pmu_ldo_lp_dig_l_comp);
#endif

#ifdef PMU_VMIN_SEARCH
    pmu_vmin_search_enable();
#endif

#ifndef NO_SLEEP
    pmu_sleep_en(1);  //enable sleep
#endif

    // Disable LDO module hw ramp_en by default part2 (others LDO module)
    pmu_read(PMU_REG_LDO_USB_CFG, &val);
    val &= ~REG_LDO_VUSB_RAMP_EN;
    pmu_write(PMU_REG_LDO_USB_CFG, val);

    pmu_read(PMU_REG_LDO_SENSOR_CFG, &val);
    val &= ~REG_LDO_VSENSOR_RAMP_EN;
    pmu_write(PMU_REG_LDO_SENSOR_CFG, val);

    pmu_read(PMU_REG_BUCK_VCORE_EN, &val);
#ifdef DCDC_VOLT_HW_RAMP_ONLY
    val |= REG_BUCK_VCORE_RAMP_EN;
#endif
    val = SET_BITFIELD(val, REG_BUCK_VCORE_RAMP_STEP, PMU_HW_RAMP_STEP_PER_CYCLE);
    pmu_write(PMU_REG_BUCK_VCORE_EN, val);

    pmu_read(PMU_REG_BUCK_VANA_EN, &val);
#ifdef DCDC_VOLT_HW_RAMP_ONLY
    val |= REG_BUCK_VANA_RAMP_EN;
#endif
    val = SET_BITFIELD(val, REG_BUCK_VANA_RAMP_STEP, PMU_HW_RAMP_STEP_HALF_PER_CYCLE);
    pmu_write(PMU_REG_BUCK_VANA_EN, val);

    pmu_read(PMU_REG_BUCK_VHPPA_EN, &val);
#ifdef DCDC_VOLT_HW_RAMP_ONLY
    val |= REG_BUCK_VHPPA_RAMP_EN;
#endif
    val = SET_BITFIELD(val, REG_BUCK_VHPPA_RAMP_STEP, PMU_HW_RAMP_STEP_HALF_PER_CYCLE);
    pmu_write(PMU_REG_BUCK_VHPPA_EN, val);

#ifdef SPLIT_CORE_M_CORE_L
    pmu_read(PMU_REG_LDO_CORE_L_CFG, &val);
#ifndef DCDC_VOLT_HW_RAMP_ONLY
    val &= ~REG_LDO_VCORE_L_RAMP_EN;
#endif
    // vcore-m 2mv/step, vcore-l 3mv/step. PMU_HW_RAMP_STEP_PER_CYCLE * 2 / 3
    val = SET_BITFIELD(val, REG_LDO_VCORE_L_RAMP_STEP, 0xA);
    pmu_write(PMU_REG_LDO_CORE_L_CFG, val);
#endif

    // fast fast voltage ramp up(2m) only used for chip wakeup
    // switch ramp mode sel in pmu_sleep() and pmu_wakeup()
    pmu_read(PMU_REG_FAST_HW_RAMP_CFG, &val);
    val = (val & ~REG_RAMP_FAST_EN) | REG_RAMP_FAST_EN_DR;
    pmu_write(PMU_REG_FAST_HW_RAMP_CFG, val);

    pmu_read(PMU_REG_LDO_LP_BIAS_SEL_VBIT, &val);
    val = SET_BITFIELD(val, REG_LDO_LP_BIAS_VBIT_RC, 0x1);
#ifdef SPLIT_CORE_M_CORE_L
    val = SET_BITFIELD(val, REG_LDO_LP_BIAS_VBIT_DSLEEP, 0x1);
#endif
    pmu_write(PMU_REG_LDO_LP_BIAS_SEL_VBIT, val);

#ifndef HPPA_LDO_ON
    pmu_read(PMU_REG_BUCK_VHPPA_CFG_E8, &val);
    val = SET_BITFIELD(val, REG_BUCK_VHPPA_CLK_LPO_PULSE, 0x0);
    pmu_write(PMU_REG_BUCK_VHPPA_CFG_E8, val);

    pmu_read(PMU_REG_BUCK_HPPA_FREQ_CFG, &val);
    val |= REG_BUCK_VHPPA_SYNC_DISABLE;
    val = SET_BITFIELD(val, REG_BUCK_VHPPA_INTERNAL_FREQUENCY, 0x7);
    pmu_write(PMU_REG_BUCK_HPPA_FREQ_CFG, val);

    pmu_read(PMU_REG_BUCK_VHPPA_LP_CFG, &val);
    val = (val & ~REG_BUCK_VHPPA_COUNTER_SEL_ULP_DSLEEP) | REG_BUCK_VHPPA_IX2_ERR_DSLEEP;
    pmu_write(PMU_REG_BUCK_VHPPA_LP_CFG, val);

    pmu_read(PMU_REG_BUCK_VHPPA_RC_CFG, &val);
    val = (val & ~REG_BUCK_VHPPA_COUNTER_SEL_ULP_RC) | REG_BUCK_VHPPA_IX2_ERR_RC;
    pmu_write(PMU_REG_BUCK_VHPPA_RC_CFG, val);
#endif

#ifndef LDO_MODE
    pmu_read(PMU_REG_BUCK_VCORE_IS_GAIN_ULP, &val);
    val = SET_BITFIELD(val, REG_BUCK_VCORE_IS_GAIN_NORMAL, 0x2);
    pmu_write(PMU_REG_BUCK_VCORE_IS_GAIN_ULP, val);

    pmu_read(PMU_REG_DCDC_DIG_SYNC, &val);
    val &= ~REG_BUCK_VCORE_KICK_PD_SLEEP_SEL;
    val |= REG_BUCK_VCORE_SYNC_DISABLE;
    val = SET_BITFIELD(val, REG_BUCK_VCORE_INTERNAL_FREQUENCY, 0x7);
    pmu_write(PMU_REG_DCDC_DIG_SYNC, val);

    pmu_read(PMU_REG_BUCK_CORE_CFG_C8, &val);
    val |= REG_BUCK_VCORE_SLOPE_EN_BURST;
    val = SET_BITFIELD(val, REG_BUCK_VCORE_COUNTER_SEL, 0x0);
    pmu_write(PMU_REG_BUCK_CORE_CFG_C8, val);

    pmu_read(PMU_REG_BUCK_VANA_CFG_E7, &val);
    val = SET_BITFIELD(val, REG_BUCK_VANA_CLK_LPO_PULSE, 0x0);
    pmu_write(PMU_REG_BUCK_VANA_CFG_E7, val);

    pmu_read(PMU_REG_BUCK_ANA_CFG_C4, &val);
    val = SET_BITFIELD(val, REG_BUCK_VANA_BURST_THRESHOLD, 0x19);
    pmu_write(PMU_REG_BUCK_ANA_CFG_C4, val);

    pmu_read(PMU_REG_BUCK_ANA_FREQ_CFG, &val);
    val |= REG_BUCK_VANA_SYNC_DISABLE;
    val = SET_BITFIELD(val, REG_BUCK_VANA_INTERNAL_FREQUENCY, 0x7);
    pmu_write(PMU_REG_BUCK_ANA_FREQ_CFG, val);
#endif

#ifndef HPPA_LDO_ON
    // enable vcodec dcdc
    val = pmu_dcdc_hppa_mv_to_val(vhppa_mv);
    pmu_dcdc_hppa_set_volt(val, val);
    pmu_dcdc_hppa_en(true);
    hal_sys_timer_delay_us(PMU_DCDC_PU_STABLE_TIME_US);
    //disable vcodec ldo
    pmu_module_config(PMU_CODEC,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
#endif

#ifdef PMU_VMEM_PWR_SUPPLY_IO
    val = pmu_ldo_vmem_mv_to_val(vmem_mv);
    pmu_module_ramp_volt(PMU_MEM, val, val);
    pmu_module_config(PMU_MEM,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);
#else
    pmu_module_config(PMU_MEM,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
#endif

    pmu_module_config(PMU_USB,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
#ifdef PMU_VSENSOR_ON
    pmu_module_config(PMU_SENSOR,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
#else
    pmu_module_config(PMU_SENSOR,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);
#endif

    pmu_codec_mic_bias_set_volt(AUD_VMIC_MAP_VMIC1 | AUD_VMIC_MAP_VMIC2, 2200);

#ifdef VRTC_SWITCH_TO_VCORE
    uint16_t val_vrtc_cfg;

    pmu_read(PMU_REG_LDO_VRTC_CFG, &val_vrtc_cfg);
    val_vrtc_cfg &= ~REG_LDO_VRTC_RAMP_EN;
    pmu_write(PMU_REG_LDO_VRTC_CFG, val_vrtc_cfg);

    pmu_read(PMU_REG_LDO_VRTC_VOLT, &val);
    val = SET_BITFIELD(val, REG_LDO_VRTC_VBIT_NORMAL, PMU_LDO_VRTC_0_75V);
    val = SET_BITFIELD(val, REG_LDO_VRTC_VBIT_RC, PMU_LDO_VRTC_0_75V);
    val = SET_BITFIELD(val, REG_LDO_VRTC_VBIT_DSLEEP, PMU_LDO_VRTC_0_55V);
    pmu_write(PMU_REG_LDO_VRTC_VOLT, val);

    pmu_read(PMU_REG_LPO_VRTC_CFG_D2, &val);
    val |= REG_VRTC_OVERRIDE;
    pmu_write(PMU_REG_LPO_VRTC_CFG_D2, val);
    hal_sys_timer_delay_us(PMU_LDO_VRTC_STABLE_TIME_US);

    pmu_read(PMU_REG_LDO_VRTC_VOLT, &val);
    val = SET_BITFIELD(val, REG_LDO_VRTC_VBIT_NORMAL, PMU_LDO_VRTC_0_65V);
    val = SET_BITFIELD(val, REG_LDO_VRTC_VBIT_RC, PMU_LDO_VRTC_0_65V);
    val = SET_BITFIELD(val, REG_LDO_VRTC_VBIT_DSLEEP, PMU_LDO_VRTC_0_55V);
    pmu_write(PMU_REG_LDO_VRTC_VOLT, val);
    hal_sys_timer_delay_us(PMU_LDO_VRTC_STABLE_TIME_US);

    val_vrtc_cfg |= REG_LDO_VRTC_RAMP_EN;
    pmu_write(PMU_REG_LDO_VRTC_CFG, val_vrtc_cfg);
#endif

#ifdef DIG_DCDC_MODE
    mode = PMU_POWER_MODE_DIG_DCDC;
#elif defined(ANA_DCDC_MODE)
    mode = PMU_POWER_MODE_ANA_DCDC;
#else // LDO_MODE
    mode = PMU_POWER_MODE_LDO;
#endif

    pmu_mode_change(mode);

#ifdef PMU_VCORE_RC_MODE_FIXED
    pmu_vcore_rc_mode_init();
#endif

#ifndef PSRAM_ENABLE
    pmu_read(PMU_REG_SW_1P8_PSRAM, &val);
    val &= ~REG_EN_SW1P8_PSRAM;
    pmu_write(PMU_REG_SW_1P8_PSRAM, val);
#endif

#ifdef SPLIT_CORE_M_CORE_L
    pmu_module_config(PMU_CORE_L,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);
    hal_sys_timer_delay_us(PMU_LDO_PU_STABLE_TIME_US);

    pmu_read(PMU_REG_CORE_L_CFG2, &val);
    val &= ~REG_LDO_VCORE_L_BYPASS;
    pmu_write(PMU_REG_CORE_L_CFG2, val);
    hal_sys_timer_delay(MS_TO_TICKS(5));
#else
    pmu_module_config(PMU_CORE_L,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);
#endif

#ifdef PMU_FORCE_LP_MODE
    // Reduce LDO voltage ripple
    // TODO: lp bias

    // Force LP mode
    pmu_module_force_lp_config();
    pmu_codec_mic_bias_lowpower_mode(AUD_VMIC_MAP_VMIC1 | AUD_VMIC_MAP_VMIC2, true);
    hal_sys_timer_delay(MS_TO_TICKS(1));

    // Switch to little bandgap
    pmu_read(PMU_REG_BIAS_CFG, &val);
    val = (val & ~BG_VBG_SEL_REG) | BG_VBG_SEL_DR;
    pmu_write(PMU_REG_BIAS_CFG, val);
    hal_sys_timer_delay(MS_TO_TICKS(1));

    if (big_bandgap_user_map == 0) {
        // Disable big bandgap
        val = (val & ~(PU_BIAS_LDO_REG | BG_CONSTANT_GM_BIAS_REG | BG_CORE_EN_REG | BG_VTOI_EN_REG | BG_NOTCH_EN_REG)) |
            PU_BIAS_LDO_DR | BG_CONSTANT_GM_BIAS_DR | BG_CORE_EN_DR | BG_VTOI_EN_DR | BG_NOTCH_EN_DR;
        pmu_write(PMU_REG_BIAS_CFG, val);
    }
#endif

#ifdef BT_RCOSC_CAL
    rf_xtal_rcosc_cal();
#endif

#ifdef PMU_VIO_3V3_ENABLE
    pmu_vio_3v3(true);
#endif

#if defined(MCU_HIGH_PERFORMANCE_MODE)
#error "Not support MCU_HIGH_PERFORMANCE_MODE!"
    pmu_high_performance_mode_enable(true);
#endif

    charger_charge_open();

#ifdef ACIN_PATTERN_DISABLE
    pmu_charger_pattern_disable();
#endif

#ifdef RC_CLK_ENABLE
#ifdef RC_FAST_WAKEUP
    pmu_rc_2m_enable();
    hal_cmu_rc_2m_clock_enable();
#endif
#ifdef SYS_RC_ENABLE
    pmu_rc_clk_enable(PMU_CLK_SEL_RC_USER_SYS);
#endif
#endif

#ifdef CAPSENSOR_ENABLE
    pmu_rc_clk_enable(PMU_CLK_SEL_RC_USER_CAP);
#endif

#endif // PMU_INIT || (!FPGA && !PROGRAMMER)

#ifdef PMU_CLK_USE_EXT_CRYSTAL
    pmu_external_crystal_enable();
#endif

#ifdef RTC_CALENDAR
    pmu_rtc_calendar_init();
#endif

#ifdef PMU_REG_CFG_DUMP
    pmu_reg_val_print();
#endif

    return 0;
}

void pmu_big_bandgap_enable(enum PMU_BIG_BANDGAP_USER_T user, int enable)
{
#ifdef PMU_FORCE_LP_MODE
    uint16_t val;
    bool update = false;

    if (enable) {
        if (big_bandgap_user_map == 0) {
            update = true;
        }
        big_bandgap_user_map |= user;
    } else {
        if (big_bandgap_user_map & user) {
            big_bandgap_user_map &= ~user;
            if (big_bandgap_user_map == 0) {
                update = true;
            }
        }
    }

    if (!update) {
        return;
    }

    pmu_read(PMU_REG_BIAS_CFG, &val);
    val |= BG_CONSTANT_GM_BIAS_DR | BG_CORE_EN_DR | BG_VTOI_EN_DR;
    if (enable) {
        val |= BG_CONSTANT_GM_BIAS_REG | BG_CORE_EN_REG | BG_VTOI_EN_REG;
    } else {
        val &= ~(BG_CONSTANT_GM_BIAS_REG | BG_CORE_EN_REG | BG_VTOI_EN_REG);
    }
    pmu_write(PMU_REG_BIAS_CFG, val);
    if (enable) {
        hal_sys_timer_delay_us(PMU_BIG_BG_STABLE_TIME_US);
    }
#endif
}

void pmu_sleep(void)
{
#ifdef FAST_HW_RAMP
    pmu_buck_volt_hw_ramp_step_set(true, PMU_F_HW_RAMP_STEP_PER_CYCLE);
#endif

#ifndef DCDC_VOLT_HW_RAMP_ONLY
    if (!dcdc_ramp_en) {
        // Enable DCDC ramp
        pmu_buck_volt_hw_ramp_enable(dcdc_ramp_map);
    }
#endif
}

void pmu_wakeup(void)
{
#ifdef FAST_HW_RAMP
    pmu_buck_volt_hw_ramp_step_set(false, PMU_HW_RAMP_STEP_PER_CYCLE);
#endif

#ifndef DCDC_VOLT_HW_RAMP_ONLY
    if (!dcdc_ramp_en) {
        pmu_buck_volt_hw_ramp_disable(dcdc_ramp_map);
    }
#endif
}

void pmu_codec_mic_bias_set_volt(uint32_t map, uint32_t mv)
{
    uint16_t val;
    int i;
    enum PMU_REG_T bias_reg;
    uint8_t volt = 0;
    uint8_t res = 0;
    uint8_t step = 0;

    step = 40;

    if (mv <= 1600) {
        volt = PMU_VMIC_1_6V;
    } else {
        volt = PMU_VMIC_1_6V + (mv - 1600) / step;
        if (volt > PMU_VMIC_2_8V) {
            volt = PMU_VMIC_2_8V;
        }
    }

    if (vcodec_mv < 1800) {
        volt += 2;
    }

    res = PMU_VMIC_RES_2_8V;

    pmu_read(PMU_REG_MIC_BIAS_A, &val);
    val = (val & ~(REG_MIC_LDO_RES_MASK)) | REG_MIC_LDO_RES(res);
    pmu_write(PMU_REG_MIC_BIAS_A, val);

    for (i = 0; i < MAX_VMIC_CH_NUM; i++) {
        if ((map & (AUD_VMIC_MAP_VMIC1 << i)) == 0) {
            continue;
        }
        bias_reg = PMU_REG_MIC_BIAS_A + i;
        pmu_read(bias_reg, &val);
        val = (val & ~(REG_MIC_BIASA_VSEL_MASK)) | REG_MIC_BIASA_VSEL(volt);
        pmu_write(bias_reg, val);
    }
}

void pmu_codec_mic_bias_enable(uint32_t map, int enable)
{
    int i;
    enum PMU_REG_T bias_reg;
    uint16_t val;
    uint16_t val_temp = 0;
    static bool reg_mic_ldo_en = false;
    static int need_ldo_on_cnt = 0;

    for (i = 0; i < MAX_VMIC_CH_NUM; i++) {
        if ((map & (AUD_VMIC_MAP_VMIC1 << i)) == 0) {
            continue;
        }
        bias_reg = PMU_REG_MIC_BIAS_A + i;
        pmu_read(bias_reg, &val);
        if (enable){
            val |= REG_MIC_BIASA_EN;
            need_ldo_on_cnt++;
        } else {
            val &= ~REG_MIC_BIASA_EN;
            need_ldo_on_cnt--;
        }
        pmu_write(bias_reg, val);
    }

    if (!reg_mic_ldo_en && need_ldo_on_cnt) {
        pmu_read(PMU_REG_MIC_BIAS_B, &val);
        val |= REG_MIC_LDO_EN;
        pmu_write(PMU_REG_MIC_BIAS_B, val);
        reg_mic_ldo_en = true;
    } else if (need_ldo_on_cnt == 0) {
        pmu_read(PMU_REG_MIC_BIAS_B, &val);
        val &= ~REG_MIC_LDO_EN;
        pmu_write(PMU_REG_MIC_BIAS_B, val);
        reg_mic_ldo_en = false;
    }

    if (!enable) {
        pmu_read(PMU_REG_MIC_PULL_DOWN, &val);
        val_temp = val;
        if (map & AUD_VMIC_MAP_VMIC1) {
            val_temp |= REG_MIC_BIASA_PULLDOWN;
        }
        if (map & AUD_VMIC_MAP_VMIC2) {
            val_temp |= REG_MIC_BIASB_PULLDOWN;
        }
        pmu_write(PMU_REG_MIC_PULL_DOWN, val_temp);
        hal_sys_timer_delay_us(PMU_LDO_PULLDOWN_STABLE_TIME_US);
        // need clear pull_down
        pmu_write(PMU_REG_MIC_PULL_DOWN, val);
    }
}

void pmu_codec_mic_bias_lowpower_mode(uint32_t map, int enable)
{
    uint16_t val;

    if (map & (AUD_VMIC_MAP_VMIC1 | AUD_VMIC_MAP_VMIC2)) {
        pmu_read(PMU_REG_MIC_PULL_DOWN, &val);
        if (enable) {
            val |= REG_MIC_LP_ENABLE;
        } else {
            val &= ~REG_MIC_LP_ENABLE;
        }
        pmu_write(PMU_REG_MIC_PULL_DOWN, val);
    }
}

SRAM_TEXT_LOC void pmu_flash_write_config(void)
{
#ifdef FLASH_WRITE_AT_HIGH_VCORE
    uint32_t lock;

    if (pmu_vcore_req & PMU_VCORE_FLASH_WRITE_ENABLED) {
        return;
    }

    lock = int_lock();
    pmu_vcore_req |= PMU_VCORE_FLASH_WRITE_ENABLED;
    int_unlock(lock);

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
#endif
}

SRAM_TEXT_LOC void pmu_flash_read_config(void)
{
#ifdef FLASH_WRITE_AT_HIGH_VCORE
    uint32_t lock;

    if ((pmu_vcore_req & PMU_VCORE_FLASH_WRITE_ENABLED) == 0) {
        return;
    }

    lock = int_lock();
    pmu_vcore_req &= ~PMU_VCORE_FLASH_WRITE_ENABLED;
    int_unlock(lock);

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
#endif
}

void BOOT_TEXT_FLASH_LOC pmu_flash_freq_config(uint32_t freq)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
    uint32_t lock;

    lock = int_lock();
    if (freq > 52000000) {
        // The real max freq is 120M
        pmu_vcore_req |= PMU_VCORE_FLASH_FREQ_HIGH;
    } else {
        pmu_vcore_req &= ~PMU_VCORE_FLASH_FREQ_HIGH;
    }
    int_unlock(lock);

    if (pmu_power_mode == PMU_POWER_MODE_NONE) {
        // PMU not init yet
        pmu_dig_init_volt();
        return;
    }

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
#endif
}

void BOOT_TEXT_FLASH_LOC pmu_psram_freq_config(uint32_t freq)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
    uint32_t lock;

    lock = int_lock();
    pmu_vcore_req &= ~(PMU_VCORE_PSRAM_FREQ_HIGH | PMU_VCORE_PSRAM_FREQ_MEDIUM | PMU_VCORE_PSRAM_FREQ_MEDIUM_LOW);
    if (freq >= 200000000) {
        pmu_vcore_req |= PMU_VCORE_PSRAM_FREQ_HIGH;
    } else if (freq >= 160000000) {
        pmu_vcore_req |= PMU_VCORE_PSRAM_FREQ_MEDIUM;
    } else if (freq >= 100000000) {
        pmu_vcore_req |= PMU_VCORE_PSRAM_FREQ_MEDIUM_LOW;
    }
    int_unlock(lock);

    if (pmu_power_mode == PMU_POWER_MODE_NONE) {
        // PMU not init yet
        pmu_dig_init_volt();
    } else {
        pmu_dig_set_volt(PMU_POWER_MODE_NONE);
    }
#endif
}

void pmu_anc_config(int enable)
{
}

void pmu_fir_high_speed_config(int enable)
{
}

void pmu_fir_freq_config(uint32_t freq)
{
    uint32_t lock;

    lock = int_lock();
    if (freq >= 96000000) {
        pmu_vcore_req |= PMU_VCORE_FIR_FREQ_HIGH;
    } else {
        pmu_vcore_req &= ~PMU_VCORE_FIR_FREQ_HIGH;
    }
    int_unlock(lock);

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
}

void pmu_iir_freq_config(uint32_t freq)
{
}

void pmu_iir_eq_freq_config(uint32_t freq)
{
    uint32_t lock;

    lock = int_lock();
    if (freq >= 96000000) {
        pmu_vcore_req |= PMU_VCORE_IIR_EQ_FREQ_HIGH;
    } else {
        pmu_vcore_req &= ~PMU_VCORE_IIR_EQ_FREQ_HIGH;
    }
    int_unlock(lock);

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
}

void pmu_rs_freq_config(uint32_t freq)
{
    uint32_t lock;

    lock = int_lock();
    if (freq >= 60000000) {
        pmu_vcore_req |= PMU_VCORE_RS_FREQ_HIGH;
    } else {
        pmu_vcore_req &= ~PMU_VCORE_RS_FREQ_HIGH;
    }
    int_unlock(lock);

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
}

void pmu_rs_adc_freq_config(uint32_t freq)
{
    uint32_t lock;

    lock = int_lock();
    if (freq >= 96000000) {
        pmu_vcore_req |= PMU_VCORE_RS_ADC_FREQ_HIGH;
    } else {
        pmu_vcore_req &= ~PMU_VCORE_RS_ADC_FREQ_HIGH;
    }
    int_unlock(lock);

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
}


void BOOT_TEXT_SRAM_LOC pmu_sys_freq_config(enum HAL_CMU_FREQ_T freq)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#if defined(MCU_HIGH_PERFORMANCE_MODE) || defined(ULTRA_LOW_POWER) || !defined(OSC_26M_X4_AUD2BB)
    uint32_t lock;
    enum PMU_VCORE_REQ_T old_req;
    bool update = false;

    lock = int_lock();
    old_req = pmu_vcore_req;
    pmu_vcore_req &= ~(PMU_VCORE_SYS_FREQ_HIGH | PMU_VCORE_SYS_FREQ_MEDIUM);
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    if (freq > HAL_CMU_FREQ_104M) {
        if (high_perf_on) {
            // The real freq is 350M
            pmu_vcore_req |= PMU_VCORE_SYS_FREQ_HIGH;
        } else {
            pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM;
        }
    } else {
#ifndef OSC_26M_X4_AUD2BB
        if (freq == HAL_CMU_FREQ_104M) {
            // The real freq is 200M
            pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM;
        }
#endif
    }
#else
    if (freq > HAL_CMU_FREQ_104M) {
        pmu_vcore_req |= PMU_VCORE_SYS_FREQ_MEDIUM;
    }
#endif
    if (old_req != pmu_vcore_req) {
        update = true;
    }
    int_unlock(lock);

    if (!update) {
        // Nothing changes
        return;
    }

    if (pmu_power_mode == PMU_POWER_MODE_NONE) {
        // PMU not init yet
        pmu_dig_init_volt();
        return;
    }

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
#endif
#endif
}

void pmu_high_performance_mode_enable(bool enable)
{
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    uint16_t val;

    if (high_perf_on == enable) {
        return;
    }
    high_perf_on = enable;

    if (!enable) {
        if (high_perf_freq_mhz > 300) {
            // Switch to 52M to avoid using PLL
            hal_cmu_sys_set_freq(HAL_CMU_FREQ_52M);
            // Restore the default div
            rf_read(RF_REG_537, &val);
            val |= (1 << 9); // div=2 (bit9=1)
            rf_write(RF_REG_537, val);
            // Restore the sys freq
            hal_cmu_sys_set_freq(hal_sysfreq_get_hw_freq());
        }
        // Restore the default PLL freq (384M)
        bbpll_freq_pll_config(192 * 1000000 * 2);
    }

    pmu_sys_freq_config(hal_sysfreq_get_hw_freq());

    if (enable) {
        uint32_t pll_freq;

        // Change freq first, and then change divider.
        // Otherwise there will be an instant very high freq sent to digital domain.

        if (high_perf_freq_mhz <= 300) {
            pll_freq = high_perf_freq_mhz * 1000000 * 2;
        } else {
            pll_freq = high_perf_freq_mhz * 1000000;
        }
        bbpll_freq_pll_config(pll_freq);

        if (high_perf_freq_mhz > 300) {
            // Switch to 52M to avoid using PLL
            hal_cmu_sys_set_freq(HAL_CMU_FREQ_52M);
            rf_read(RF_REG_537, &val);
            val &= ~(1 << 9); // div=1 (bit9=0)
            rf_write(RF_REG_537, val);
            // Restore the sys freq
            hal_cmu_sys_set_freq(hal_sysfreq_get_hw_freq());
        }
    }
#endif
}

void pmu_usb_config(enum PMU_USB_CONFIG_TYPE_T type)
{

}

#if !defined(FPGA) && !defined(PROGRAMMER)
struct PMU_CHG_CTX_T {
    uint16_t pmu_chg_status;
};

struct PMU_CHG_CTX_T BOOT_BSS_LOC pmu_chg_ctx;

void BOOT_TEXT_SRAM_LOC pmu_charger_save_context(void)
{
    pmu_read(PMU_REG_CHARGER_STATUS, &pmu_chg_ctx.pmu_chg_status);
}

enum PMU_POWER_ON_CAUSE_T pmu_charger_poweron_status(void)
{
    enum PMU_POWER_ON_CAUSE_T pmu_power_on_cause = PMU_POWER_ON_CAUSE_NONE;

    if (pmu_chg_ctx.pmu_chg_status & AC_ON_DET_OUT){
        pmu_power_on_cause = PMU_POWER_ON_CAUSE_CHARGER_ACOFF;
    }else if (pmu_chg_ctx.pmu_chg_status & AC_ON){
        pmu_power_on_cause = PMU_POWER_ON_CAUSE_CHARGER_ACON;
    }
    return pmu_power_on_cause;
}
#endif

void pmu_charger_init(void)
{
    unsigned short readval_cfg;
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_CFG, &readval_cfg);
    readval_cfg &= ~(REG_CHARGE_OUT_INTR_MSK | REG_CHARGE_IN_INTR_MSK |
        REG_AC_ON_OUT_EN | REG_AC_ON_IN_EN);
    pmu_write(PMU_REG_CHARGER_CFG ,readval_cfg);
    int_unlock(lock);

    hal_sys_timer_delay(MS_TO_TICKS(1));

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_CFG, &readval_cfg);
    readval_cfg |= REG_AC_ON_OUT_EN | REG_AC_ON_IN_EN;
    readval_cfg = SET_BITFIELD(readval_cfg, REG_AC_ON_DB_VALUE, 8);
    pmu_write(PMU_REG_CHARGER_CFG ,readval_cfg);
    int_unlock(lock);
}

static void pmu_charger_irq_handler(PMU_IRQ_HDLR_PARAM)
{
    enum PMU_CHARGER_STATUS_T status = PMU_CHARGER_UNKNOWN;
    unsigned short readval;

#ifdef PMU_IRQ_UNIFIED
    readval = irq_status;
#else
    uint32_t lock;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_STATUS, &readval);
    pmu_write(PMU_REG_CHARGER_STATUS, readval);
    int_unlock(lock);
#endif
    PMU_DEBUG_TRACE(3,"%s REG_%02X=0x%04X", __func__, PMU_REG_CHARGER_STATUS, readval);

    if ((readval & (INTR_MSKED_CHARGE_IN | INTR_MSKED_CHARGE_OUT)) == 0){
        PMU_DEBUG_TRACE(1,"%s SKIP", __func__);
        return;
    } else if ((readval & (INTR_MSKED_CHARGE_IN | INTR_MSKED_CHARGE_OUT)) ==
            (INTR_MSKED_CHARGE_IN | INTR_MSKED_CHARGE_OUT)) {
        PMU_DEBUG_TRACE(1,"%s DITHERING", __func__);
        hal_sys_timer_delay(2);
    } else {
        PMU_DEBUG_TRACE(1,"%s NORMAL", __func__);
    }

    status = pmu_charger_get_status();

    if (charger_irq_handler) {
        charger_irq_handler(status);
    }
}

void pmu_charger_set_irq_handler(PMU_CHARGER_IRQ_HANDLER_T handler)
{
    uint32_t lock;
    uint16_t val;

    charger_irq_handler = handler;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_CFG, &val);
    if (handler) {
        val |= REG_CHARGE_IN_INTR_MSK | REG_CHARGE_OUT_INTR_MSK;
    } else {
        val &= ~(REG_CHARGE_IN_INTR_MSK | REG_CHARGE_OUT_INTR_MSK);
    }
    pmu_write(PMU_REG_CHARGER_CFG, val);

#ifdef PMU_IRQ_UNIFIED
   pmu_set_irq_unified_handler(PMU_IRQ_TYPE_CHARGER, handler ? pmu_charger_irq_handler : NULL);
#else
    if (handler) {
        NVIC_SetVector(CHARGER_IRQn, (uint32_t)pmu_charger_irq_handler);
        NVIC_SetPriority(CHARGER_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(CHARGER_IRQn);
        NVIC_EnableIRQ(CHARGER_IRQn);
    } else {
        NVIC_DisableIRQ(CHARGER_IRQn);
    }
#endif
    int_unlock(lock);
}

void pmu_charger_plugin_config(void)
{
}

void pmu_charger_plugout_config(void)
{
}

enum PMU_CHARGER_STATUS_T pmu_charger_get_status(void)
{
    unsigned short readval;
    enum PMU_CHARGER_STATUS_T status;

    pmu_read(PMU_REG_CHARGER_STATUS, &readval);
    if (readval & AC_ON)
        status = PMU_CHARGER_PLUGIN;
    else
        status = PMU_CHARGER_PLUGOUT;

    return status;
}

#ifdef PMU_IRQ_UNIFIED
static void pmu_general_irq_handler(void)
{
    uint32_t lock;
    uint16_t val;
    bool pwrkey, charger, gpadc, rtc, gpio, wdt;

    pwrkey = false;
    charger = false;
    gpadc = false;
    rtc = false;
    gpio = false;
    wdt = false;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_STATUS, &val);
    if (val & (POWER_ON_RELEASE | POWER_ON_PRESS)) {
        pwrkey = true;
    }
    if (val & (INTR_MSKED_CHARGE_IN | INTR_MSKED_CHARGE_OUT)) {
        charger = true;
    }
    if (pwrkey || charger) {
        pmu_write(PMU_REG_CHARGER_STATUS, val);
    }
    int_unlock(lock);

    if (pwrkey) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_PWRKEY]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_PWRKEY](val);
        }
    }
    if (charger) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_CHARGER]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_CHARGER](val);
        }
    }

    lock = int_lock();
    pmu_read(PMU_REG_INT_MSKED_STATUS, &val);
    if (val & (KEY_ERR1_INTR_MSKED | KEY_ERR0_INTR_MSKED | KEY_PRESS_INTR_MSKED | KEY_RELEASE_INTR_MSKED |
            SAMPLE_DONE_INTR_MSKED | CHAN_DATA_INTR_MSKED_MASK)) {
        gpadc = true;
    }
    if (val & (RTC_INT1_MSKED | RTC_INT0_MSKED)) {
        rtc = true;
    }
    if (gpadc || rtc) {
        pmu_write(PMU_REG_INT_CLR, val);
    }
    int_unlock(lock);

    if (gpadc) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_GPADC]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_GPADC](val);
        }
    }
    if (rtc) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_RTC]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_RTC](val);
        }
    }

    lock = int_lock();
    pmu_read(PMU_REG_LED_IO_IN, &val);
    if (val & (PMU_GPIO_INTR_MSKED1 | PMU_GPIO_INTR_MSKED2)) {
        gpio = true;
    }
    if (val & WDT_INTR_MSKED) {
        wdt = true;
    }
    if (gpio) {
        pmu_write(PMU_REG_LED_IO_IN, val);
    }
    if (wdt) {
        // No need to clear
    }
    int_unlock(lock);

    if (gpio) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_GPIO]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_GPIO](val);
        }
    }
    if (wdt) {
        if (pmu_irq_hdlrs[PMU_IRQ_TYPE_WDT]) {
            pmu_irq_hdlrs[PMU_IRQ_TYPE_WDT](val);
        }
    }
}

int pmu_set_irq_unified_handler(enum PMU_IRQ_TYPE_T type, PMU_IRQ_UNIFIED_HANDLER_T hdlr)
{
    bool update;
    uint32_t lock;
    int i;

    if (type >= PMU_IRQ_TYPE_QTY) {
        return 1;
    }

    enum PMU_REG_T reg;
    uint16_t val;
    uint16_t mask;

    if (type == PMU_IRQ_TYPE_GPADC) {
        reg = PMU_REG_USB_CFG2;
        mask = GPADC_INTR_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_RTC) {
        reg = PMU_REG_USB_CFG2;
        mask = RTC_INTR_TMP_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_CHARGER) {
        reg = PMU_REG_USB_CFG2;
        mask = CHARGE_INTR_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_GPIO) {
        reg = PMU_REG_USB_CFG3;
        mask = PMU_GPIO_INTR_MSKED1_MERGED_MSK | PMU_GPIO_INTR_MSKED2_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_WDT) {
        reg = PMU_REG_USB_CFG3;
        mask = WDT_INTR_MSKED_MERGED_MSK;
    } else if (type == PMU_IRQ_TYPE_PWRKEY) {
        reg = PMU_REG_USB_CFG3;
        mask = POWER_ON_INTR_MERGED_MSK;
    } else {
        return 2;
    }

    update = false;

    lock = int_lock();

    for (i = 0; i < PMU_IRQ_TYPE_QTY; i++) {
        if (pmu_irq_hdlrs[i]) {
            break;
        }
    }

    pmu_irq_hdlrs[type] = hdlr;

    pmu_read(reg, &val);
    if (hdlr) {
        val |= mask;
    } else {
        val &= ~mask;
    }
    pmu_write(reg, val);

    if (hdlr) {
        update = (i >= PMU_IRQ_TYPE_QTY);
    } else {
        if (i == type) {
            for (; i < PMU_IRQ_TYPE_QTY; i++) {
                if (pmu_irq_hdlrs[i]) {
                    break;
                }
            }
            update = (i >= PMU_IRQ_TYPE_QTY);
        }
    }

    if (update) {
        if (hdlr) {
            NVIC_SetVector(RTC_IRQn, (uint32_t)pmu_general_irq_handler);
            NVIC_SetPriority(RTC_IRQn, IRQ_PRIORITY_NORMAL);
            NVIC_ClearPendingIRQ(RTC_IRQn);
            NVIC_EnableIRQ(RTC_IRQn);
        } else {
            NVIC_DisableIRQ(RTC_IRQn);
        }
    }

    int_unlock(lock);

    return 0;
}
#endif

enum HAL_PWRKEY_IRQ_T pmu_pwrkey_irq_value_to_state(uint16_t irq_status)
{
    enum HAL_PWRKEY_IRQ_T state = HAL_PWRKEY_IRQ_NONE;

    if (irq_status & POWER_ON_PRESS) {
        state |= HAL_PWRKEY_IRQ_FALLING_EDGE;
    }

    if (irq_status & POWER_ON_RELEASE) {
        state |= HAL_PWRKEY_IRQ_RISING_EDGE;
    }

    return state;
}

int pmu_debug_config_ana(uint16_t volt)
{
    return 0;
}

int pmu_debug_config_codec(uint16_t volt)
{
    return 0;
}

int pmu_debug_config_vcrystal(bool on)
{
    return 0;
}

int pmu_debug_config_audio_output(bool diff)
{
    return 0;
}

void pmu_debug_reliability_test(int stage)
{
    uint16_t volt;

    if (stage == 0) {
        volt = PMU_DCDC_ANA_1_2V;
    } else {
        volt = PMU_DCDC_ANA_1_3V;
    }
    pmu_dcdc_ana_set_volt(volt, ana_lp_dcdc);
}

void pmu_led_set_hiz(enum HAL_GPIO_PIN_T pin)
{
    uint16_t val;
    uint32_t lock;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_GPIO_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val = (val | REG_LED_IO1_PU | REG_LED_IO1_OENB_PRE) & ~(REG_LED_IO1_PUEN | REG_LED_IO1_PDEN | REG_LED_IO1_RX_EN);
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val = (val | REG_LED_IO2_PU | REG_LED_IO2_OENB_PRE) & ~(REG_LED_IO2_PUEN | REG_LED_IO2_PDEN | REG_LED_IO2_RX_EN);
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }
}

void pmu_led_set_direction(enum HAL_GPIO_PIN_T pin, enum HAL_GPIO_DIR_T dir)
{
    uint16_t val;
    uint32_t lock;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_GPIO_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val |= REG_LED_IO1_PU;
            if (dir == HAL_GPIO_DIR_IN) {
                val |= REG_LED_IO1_OENB_PRE | REG_LED_IO1_RX_EN;
            } else {
                val &= ~(REG_LED_IO1_OENB_PRE | REG_LED_IO1_RX_EN);
            }
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val |= REG_LED_IO2_PU;
            if (dir == HAL_GPIO_DIR_IN) {
                val |= REG_LED_IO2_OENB_PRE | REG_LED_IO2_RX_EN;
            } else {
                val &= ~(REG_LED_IO2_OENB_PRE | REG_LED_IO2_RX_EN);
            }
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }
}

enum HAL_GPIO_DIR_T pmu_led_get_direction(enum HAL_GPIO_PIN_T pin)
{
    uint16_t val;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        if (pin == HAL_GPIO_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            return (val & REG_LED_IO1_OENB_PRE) ? HAL_GPIO_DIR_IN : HAL_GPIO_DIR_OUT;
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            return (val & REG_LED_IO2_OENB_PRE) ? HAL_GPIO_DIR_IN : HAL_GPIO_DIR_OUT;
        }
    } else {
        return HAL_GPIO_DIR_IN;
    }
}

void pmu_led_set_voltage_domains(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_VOLTAGE_DOMAINS_T volt)
{
    enum PMU_LED_VOLT_T {
        PMU_LED_VOLT_VBAT,
        PMU_LED_VOLT_VMEM,
        PMU_LED_VOLT_VIO,
    };
    enum PMU_LED_VOLT_T sel;
    uint16_t val;
    uint32_t lock;

    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        if (volt == HAL_IOMUX_PIN_VOLTAGE_VIO) {
            sel = PMU_LED_VOLT_VIO;
        } else if (volt == HAL_IOMUX_PIN_VOLTAGE_MEM) {
            sel = PMU_LED_VOLT_VMEM;
        } else {
            sel = PMU_LED_VOLT_VBAT;
        }

        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val = SET_BITFIELD(val, REG_LED_IO1_SEL, sel);
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val = SET_BITFIELD(val, REG_LED_IO2_SEL, sel);
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }
}

void pmu_led_set_pull_select(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_PULL_SELECT_T pull_sel)
{
    uint16_t val;
    uint32_t lock;

    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_read(PMU_REG_LED_CFG_IO1, &val);
            val &= ~(REG_LED_IO1_PDEN | REG_LED_IO1_PUEN);
            if (pull_sel == HAL_IOMUX_PIN_PULLUP_ENABLE) {
                val |= REG_LED_IO1_PUEN;
            } else if (pull_sel == HAL_IOMUX_PIN_PULLDOWN_ENABLE) {
                val |= REG_LED_IO1_PDEN;
            }
            pmu_write(PMU_REG_LED_CFG_IO1, val);
        } else {
            pmu_read(PMU_REG_LED_CFG_IO2, &val);
            val &= ~(REG_LED_IO2_PDEN | REG_LED_IO2_PUEN);
            if (pull_sel == HAL_IOMUX_PIN_PULLUP_ENABLE) {
                val |= REG_LED_IO2_PUEN;
            } else if (pull_sel == HAL_IOMUX_PIN_PULLDOWN_ENABLE) {
                val |= REG_LED_IO2_PDEN;
            }
            pmu_write(PMU_REG_LED_CFG_IO2, val);
        }
        int_unlock(lock);
    }
}

void pmu_led_set_value(enum HAL_GPIO_PIN_T pin, int data)
{
    uint32_t lock;
    uint16_t val;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_GPIO_PIN_LED1) {
#ifdef PMU_LED_VIA_PWM
            pmu_write(PMU_REG_PWM2_TOGGLE, 0xFFFF);
            pmu_read(PMU_REG_PWM2_BR_EN, &val);
            val &= ~REG_PWM2_BR_EN;
            val |= PWM_SELECT_EN;
            if (data) {
                val &= ~PWM_SELECT_INV;
            } else {
                val |= PWM_SELECT_INV;
            }
            pmu_write(PMU_REG_PWM2_BR_EN, val);
#else
            pmu_read(PMU_REG_PWM2_EN, &val);
            if (data) {
                val |= REG_LED0_OUT;
            } else {
                val &= ~REG_LED0_OUT;
            }
            pmu_write(PMU_REG_PWM2_EN, val);
#endif
        }else {
#ifdef PMU_LED_VIA_PWM
            pmu_write(PMU_REG_PWMB_TOGGLE, 0xFFFF);
            pmu_read(PMU_REG_PWMB_BR_EN, &val);
            val &= ~REG_PWMB_BR_EN;
            val |= PWMB_SELECT_EN;
            if (data) {
                val &= ~PWMB_SELECT_INV;
            } else {
                val |= PWMB_SELECT_INV;
            }
            pmu_write(PMU_REG_PWMB_BR_EN, val);
#else
            pmu_read(PMU_REG_PWMB_EN, &val);
            if (data) {
                val |= REG_LEDB_OUT;
            } else {
                val &= ~REG_LEDB_OUT;
            }
            pmu_write(PMU_REG_PWMB_EN, val);
#endif
        }
        int_unlock(lock);
    }
}

int pmu_led_get_value(enum HAL_GPIO_PIN_T pin)
{
    uint16_t val;
    int data = 0;

    if (pin == HAL_GPIO_PIN_LED1 || pin == HAL_GPIO_PIN_LED2) {
        pmu_read(PMU_REG_LED_IO_IN, &val);
        if (pin == HAL_GPIO_PIN_LED1) {
            data = LED_IO1_IN_DB;
        } else {
            data = LED_IO2_IN_DB;
        }
        data &= val;
    }

    return !!data;
}

void pmu_led_uart_enable(enum HAL_IOMUX_PIN_T pin)
{
    uint16_t val;
    if (pin == HAL_IOMUX_PIN_LED1) {
        pmu_read(PMU_REG_UART1_CFG, &val);
        val &= ~(REG_UART_LEDA_SEL | REG_PMU_UART_DR1);
        val |=  REG_GPIO_I_SEL;
        pmu_write(PMU_REG_UART1_CFG, val);

        pmu_read(PMU_REG_UART2_CFG, &val);
        val |= REG_PMU_UART_DR2;
        pmu_write(PMU_REG_UART2_CFG, val);

        pmu_read(PMU_REG_LED_CFG_IO1, &val);
        val |= REG_LED_IO1_PUEN;
        pmu_write(PMU_REG_LED_CFG_IO1, val);
    } else {
        pmu_read(PMU_REG_UART1_CFG, &val);
        val &= ~ REG_GPIO_I_SEL;
        val |= REG_PMU_UART_DR1;
        pmu_write(PMU_REG_UART1_CFG, val);

        pmu_read(PMU_REG_UART2_CFG, &val);
        val &= ~(REG_UART_LEDB_SEL | REG_PMU_UART_DR2);
        pmu_write(PMU_REG_UART2_CFG, val);

        pmu_read(PMU_REG_LED_CFG_IO2, &val);
        val |= REG_LED_IO2_PUEN;
        pmu_write(PMU_REG_LED_CFG_IO2, val);
    }
}

void pmu_led_uart_disable(enum HAL_IOMUX_PIN_T pin)
{
    uint16_t val;
    if (pin == HAL_IOMUX_PIN_LED1) {
        pmu_read(PMU_REG_UART1_CFG, &val);
        val |= (REG_UART_LEDA_SEL | REG_PMU_UART_DR1);
        val &=  ~REG_GPIO_I_SEL;
        pmu_write(PMU_REG_UART1_CFG, val);

        pmu_read(PMU_REG_UART2_CFG, &val);
        val &= ~REG_PMU_UART_DR2;
        pmu_write(PMU_REG_UART2_CFG, val);

        pmu_read(PMU_REG_LED_CFG_IO1, &val);
        val &= ~REG_LED_IO1_PUEN;
        pmu_write(PMU_REG_LED_CFG_IO1, val);
    } else {
        pmu_read(PMU_REG_UART1_CFG, &val);
        val |= REG_GPIO_I_SEL;
        val &= ~REG_PMU_UART_DR1;
        pmu_write(PMU_REG_UART1_CFG, val);

        pmu_read(PMU_REG_UART2_CFG, &val);
        val |= (REG_UART_LEDB_SEL | REG_PMU_UART_DR2);
        pmu_write(PMU_REG_UART2_CFG, val);

        pmu_read(PMU_REG_LED_CFG_IO2, &val);
        val &= ~REG_LED_IO2_PUEN;
        pmu_write(PMU_REG_LED_CFG_IO2, val);
    }
}

void pmu_led_breathing_enable(enum HAL_IOMUX_PIN_T pin, const struct PMU_LED_BR_CFG_T *cfg)
{
    uint32_t st1;
    uint32_t st2;
    uint32_t subcnt_data;
    uint8_t tg;
    uint16_t val;
    uint32_t lock;
    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        st1 = MS_TO_TICKS(cfg->off_time_ms);
        if (st1 > 0xFFFF) {
            st1 = 0xFFFF;
        }
        st2 = MS_TO_TICKS(cfg->on_time_ms);
        if (st2 > 0xFFFF) {
            st2 = 0xFFFF;
        }
        subcnt_data = MS_TO_TICKS(cfg->fade_time_ms);
        subcnt_data = integer_sqrt_nearest(subcnt_data);
        if (subcnt_data > (SUBCNT_DATA2_MASK >> SUBCNT_DATA2_SHIFT)) {
            subcnt_data = (SUBCNT_DATA2_MASK >> SUBCNT_DATA2_SHIFT);
        }
        // TODO: Keep compatible with digital PWM module (can be removed after 2500)
        if (subcnt_data > 0xFE) {
            subcnt_data = 0xFE;
        }
        tg = 1;

        pmu_led_set_direction((enum HAL_GPIO_PIN_T)pin, HAL_GPIO_DIR_OUT);
        pmu_led_set_pull_select(pin, HAL_IOMUX_PIN_NOPULL);
        pmu_led_set_voltage_domains(pin, HAL_IOMUX_PIN_VOLTAGE_VBAT);

        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_write(PMU_REG_PWM2_TOGGLE, st2);
            pmu_write(PMU_REG_PWM2_ST1, st1);
            val = SUBCNT_DATA2(subcnt_data) | TG_SUBCNT_D2_ST(tg);
            pmu_write(PMU_REG_PWM2_EN, val);
            pmu_read(PMU_REG_PWM2_BR_EN, &val);
            val = (val & ~REG_CLK_PWM_DIV_MASK) | REG_CLK_PWM_DIV(0) |
                REG_PWM_CLK_EN | REG_PWM2_BR_EN | PWM_SELECT_EN;
            pmu_write(PMU_REG_PWM2_BR_EN, val);
        } else {
            pmu_write(PMU_REG_PWMB_TOGGLE, st2);
            pmu_write(PMU_REG_PWMB_ST1, st1);
            val = SUBCNT_DATAB(subcnt_data) | TG_SUBCNT_DB_ST(tg);
            pmu_write(PMU_REG_PWMB_EN, val);
            pmu_read(PMU_REG_PWMB_BR_EN, &val);
            val = (val & ~REG_CLK_PWMB_DIV_MASK) | REG_CLK_PWMB_DIV(0) |
                REG_PWMB_CLK_EN | REG_PWMB_BR_EN | PWMB_SELECT_EN;
            pmu_write(PMU_REG_PWMB_BR_EN, val);
        }
        int_unlock(lock);
    }
}

void pmu_led_breathing_disable(enum HAL_IOMUX_PIN_T pin)
{
    uint16_t val;
    uint32_t lock;
    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        lock = int_lock();
        if (pin == HAL_IOMUX_PIN_LED1) {
            pmu_read(PMU_REG_PWM2_BR_EN, &val);
            val &= ~(REG_PWM_CLK_EN | REG_PWM2_BR_EN | PWM_SELECT_EN);
            pmu_write(PMU_REG_PWM2_BR_EN, val);
        } else {
            pmu_read(PMU_REG_PWMB_BR_EN, &val);
            val &= ~(REG_PWMB_CLK_EN | REG_PWMB_BR_EN | PWMB_SELECT_EN);
            pmu_write(PMU_REG_PWMB_BR_EN, val);
        }
        int_unlock(lock);
        pmu_led_set_direction((enum HAL_GPIO_PIN_T)pin, HAL_GPIO_DIR_IN);
        pmu_led_set_pull_select(pin, HAL_IOMUX_PIN_PULLUP_ENABLE);
    }
}

#ifdef PMU_IRQ_UNIFIED
static void pmu_gpio_irq_handler(uint16_t irq_status)
{
    if (irq_status & PMU_GPIO_INTR_MSKED1) {
        if (gpio_irq_handler[0]) {
            gpio_irq_handler[0](HAL_GPIO_PIN_LED1);
        }
    }
    if (irq_status & PMU_GPIO_INTR_MSKED2) {
        if (gpio_irq_handler[1]) {
            gpio_irq_handler[1](HAL_GPIO_PIN_LED2);
        }
    }
}

uint8_t pmu_gpio_setup_irq(enum HAL_GPIO_PIN_T pin, const struct HAL_GPIO_IRQ_CFG_T *cfg)
{
    uint32_t lock;
    uint16_t val;
    bool old_en;

    if (pin != HAL_GPIO_PIN_LED1 && pin != HAL_GPIO_PIN_LED2) {
        return 1;
    }

    lock = int_lock();

    old_en = (gpio_irq_en[0] || gpio_irq_en[1]);

    if (pin == HAL_GPIO_PIN_LED1) {
        gpio_irq_en[0] = cfg->irq_enable;
        gpio_irq_handler[0] = cfg->irq_handler;

        if (cfg->irq_enable) {
        }

        pmu_read(PMU_REG_UART1_CFG, &val);
        if (cfg->irq_enable) {
            val |= REG_PMU_GPIO_INTR_MSK1 | REG_PMU_GPIO_INTR_EN1;
            if (cfg->irq_debounce) {
                val &= ~PMU_DB_BYPASS1;
            } else {
                val |= PMU_DB_BYPASS1;
            }
        } else {
            val &= ~(REG_PMU_GPIO_INTR_MSK1 | REG_PMU_GPIO_INTR_EN1);
        }
        pmu_write(PMU_REG_UART1_CFG, val);
    } else {
        gpio_irq_en[1] = cfg->irq_enable;
        gpio_irq_handler[1] = cfg->irq_handler;

        pmu_read(PMU_REG_UART2_CFG, &val);
        if (cfg->irq_enable) {
            val |= REG_PMU_GPIO_INTR_MSK2 | REG_PMU_GPIO_INTR_EN2;
            if (cfg->irq_debounce) {
                val &= ~PMU_DB_BYPASS2;
            } else {
                val |= PMU_DB_BYPASS2;
            }
        } else {
            val &= ~(REG_PMU_GPIO_INTR_MSK1 | REG_PMU_GPIO_INTR_EN1);
        }
        pmu_write(PMU_REG_UART2_CFG, val);
    }

    if (cfg->irq_enable) {
        uint16_t type;
        uint16_t pol;

        type = (pin == HAL_GPIO_PIN_LED1) ? REG_EDGE_INTR_SEL1 : REG_EDGE_INTR_SEL2;
        pol = (pin == HAL_GPIO_PIN_LED1) ? REG_POS_INTR_SEL1 : REG_POS_INTR_SEL2;
        pmu_read(PMU_REG_MIC_BIAS_C, &val);
        if (cfg->irq_type == HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE) {
            val |= type;
            if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_LOW_FALLING) {
                val &= ~pol;
            } else {
                val |= pol;
            }
        } else {
            val &= ~type;
        }
        pmu_write(PMU_REG_MIC_BIAS_C, val);

        if (cfg->irq_type != HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE) {
            if (pin == HAL_GPIO_PIN_LED1) {
                pmu_read(PMU_REG_WDT_INT_CFG, &val);
                if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_LOW_FALLING) {
                    val |= REG_LOW_LEVEL_INTR_SEL1;
                } else {
                    val &= ~REG_LOW_LEVEL_INTR_SEL1;
                }
                pmu_write(PMU_REG_WDT_INT_CFG, val);
            } else {
                pmu_read(PMU_REG_UART2_CFG, &val);
                if (cfg->irq_polarity == HAL_GPIO_IRQ_POLARITY_LOW_FALLING) {
                    val |= REG_LOW_LEVEL_INTR_SEL2;
                } else {
                    val &= ~REG_LOW_LEVEL_INTR_SEL2;
                }
                pmu_write(PMU_REG_UART2_CFG, val);
            }
        }
    }

    if (old_en != cfg->irq_enable) {
        pmu_set_irq_unified_handler(PMU_IRQ_TYPE_GPIO, cfg->irq_enable ? pmu_gpio_irq_handler : NULL);
    }

    int_unlock(lock);

    return 0;
}

static void pmu_wdt_irq_handler(uint16_t irq_status)
{
    if (wdt_irq_handler) {
        wdt_irq_handler();
    }
}

void pmu_wdt_set_irq_handler(PMU_WDT_IRQ_HANDLER_T handler)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();

    val = REG_WDT_INTR_CLR;
    pmu_write(PMU_REG_LED_IO_IN, val);

    wdt_irq_handler = handler;

    pmu_read(PMU_REG_WDT_INT_CFG, &val);
    if (handler) {
        val |= REG_WDT_INTR_EN | REG_WDT_INTR_MSK;
    } else {
        val &= ~(REG_WDT_INTR_EN | REG_WDT_INTR_MSK);
    }
    pmu_write(PMU_REG_WDT_INT_CFG, val);

    pmu_set_irq_unified_handler(PMU_IRQ_TYPE_WDT, handler ? pmu_wdt_irq_handler : NULL);

    int_unlock(lock);
}
#else
uint8_t pmu_gpio_setup_irq(enum HAL_GPIO_PIN_T pin, const struct HAL_GPIO_IRQ_CFG_T *cfg)
{
    ASSERT(false, "PMU_IRQ_UNIFIED must defined to use PMU GPIO IRQ");
    return 1;
}

void pmu_wdt_set_irq_handler(PMU_WDT_IRQ_HANDLER_T handler)
{
}
#endif

#ifdef __WATCHER_DOG_RESET__
struct PMU_WDT_CTX_T {
    bool enabled;
    uint16_t wdt_irq_timer;
    uint16_t wdt_reset_timer;
    uint16_t wdt_cfg;
};

static struct PMU_WDT_CTX_T BOOT_BSS_LOC wdt_ctx;

void BOOT_TEXT_SRAM_LOC pmu_wdt_save_context(void)
{
    uint16_t wdt_cfg = 0, wdt_timer = 0;
    pmu_read(PMU_REG_WDT_CFG, &wdt_cfg);
    if (wdt_cfg & (REG_WDT_RESET_EN | REG_WDT_EN)){
        wdt_ctx.enabled = true;
        wdt_ctx.wdt_cfg = wdt_cfg;
        pmu_read(PMU_REG_WDT_IRQ_TIMER, &wdt_timer);
        wdt_ctx.wdt_irq_timer = wdt_timer;
        pmu_read(PMU_REG_WDT_RESET_TIMER, &wdt_timer);
        wdt_ctx.wdt_reset_timer = wdt_timer;
    }
}

void BOOT_TEXT_SRAM_LOC pmu_wdt_restore_context(void)
{
    if (wdt_ctx.enabled) {
        pmu_write(PMU_REG_WDT_IRQ_TIMER, wdt_ctx.wdt_irq_timer);
        pmu_write(PMU_REG_WDT_RESET_TIMER, wdt_ctx.wdt_reset_timer);
        pmu_write(PMU_REG_WDT_CFG, wdt_ctx.wdt_cfg);
    }
}
#endif

int pmu_wdt_config(uint32_t irq_ms, uint32_t reset_ms)
{
    if (irq_ms > 0xFFFF) {
        return 1;
    }
    if (reset_ms > 0xFFFF) {
        return 1;
    }
    wdt_irq_timer = irq_ms;
    wdt_reset_timer = reset_ms;

    pmu_write(PMU_REG_WDT_IRQ_TIMER, wdt_irq_timer);
    pmu_write(PMU_REG_WDT_RESET_TIMER, wdt_reset_timer);

    return 0;
}

void pmu_wdt_start(void)
{
    uint16_t val;

    if (wdt_irq_timer == 0 && wdt_reset_timer == 0) {
        return;
    }

    pmu_read(PMU_REG_WDT_CFG, &val);
    val |= (REG_WDT_RESET_EN | REG_WDT_EN);
    pmu_write(PMU_REG_WDT_CFG, val);
}

#ifndef __WATCHER_DOG_RESET__
BOOT_TEXT_SRAM_LOC
#endif
void pmu_wdt_stop(void)
{
    uint16_t val;

    pmu_read(PMU_REG_WDT_CFG, &val);
    val &= ~(REG_WDT_RESET_EN | REG_WDT_EN);
    pmu_write(PMU_REG_WDT_CFG, val);

    val = REG_WDT_INTR_CLR;
    pmu_write(PMU_REG_LED_IO_IN, val);
}

void pmu_wdt_feed(void)
{
    if (wdt_irq_timer == 0 && wdt_reset_timer == 0) {
        return;
    }

    pmu_write(PMU_REG_WDT_IRQ_TIMER, wdt_irq_timer);
    pmu_write(PMU_REG_WDT_RESET_TIMER, wdt_reset_timer);
}

int pmu_wdt_reboot(void)
{
    uint32_t lock;
    uint16_t val;
    uint16_t val_wdt_cfg;
    const uint16_t wdt_time_ms = 4;

    lock = int_lock();

    // Disable wdt by defalut
    pmu_read(PMU_REG_WDT_CFG, &val_wdt_cfg);
    val_wdt_cfg &= ~(REG_WDT_RESET_EN | REG_WDT_EN);
    pmu_write(PMU_REG_WDT_CFG, val_wdt_cfg);

    // Clear intr by default
    val = REG_WDT_INTR_CLR;
    pmu_write(PMU_REG_LED_IO_IN, val);

    pmu_write(PMU_REG_WDT_IRQ_TIMER, wdt_time_ms / 2);
    pmu_write(PMU_REG_WDT_RESET_TIMER, wdt_time_ms / 2);

    pmu_read(PMU_REG_WDT_INT_CFG, &val);
    val |= REG_WDT_INTR_EN | REG_WDT_INTR_MSK;
    pmu_write(PMU_REG_WDT_INT_CFG, val);

    val_wdt_cfg |= (REG_WDT_RESET_EN | REG_WDT_EN);
    pmu_write(PMU_REG_WDT_CFG, val_wdt_cfg);
    hal_sys_timer_delay(MS_TO_TICKS(wdt_time_ms << 1));

    PMU_INFO_TRACE_IMM(0, "\nError: pmu wdt reboot failed!\n");

    int_unlock(lock);

    return 0;
}

int pmu_wdt_load_test(void)
{
    uint16_t val0, val1;

    pmu_read(PMU_REG_RAW_STATUS, &val0);

    pmu_write(PMU_REG_WDT_RESET_TIMER, 3000);

    pmu_read(PMU_REG_RAW_STATUS, &val1);

    TRACE(0, "wdt_load: %u -> %u", !!(val0 & WDT_LOAD), !!(val1 & WDT_LOAD));

    if ((val0 & WDT_LOAD) == (val1 & WDT_LOAD)) {
        TRACE(0, "BAD: wdt load not changed");
        return 1;
    } else {
        TRACE(0, "GOOD: wdt load changed");
        return 0;
    }
}

void pmu_ntc_capture_enable(void)
{
    uint16_t val;

    pmu_read(PMU_REG_AVDD_EN, &val);
    val |= REG_PU_AVDD25_ANA;
    pmu_write(PMU_REG_AVDD_EN, val);

    pmu_read(PMU_REG_SAR_CLK_CFG, &val);
    val |= REG_SAR_INPUT_BUF_EN;
    pmu_write(PMU_REG_SAR_CLK_CFG, val);

    pmu_read(PMU_REG_SAR_PU_CFG, &val);
    val |= REG_SAR_PU_VREF_IN;
    pmu_write(PMU_REG_SAR_PU_CFG, val);

    pmu_read(PMU_REG_IPTAT_CORE_I_CFG, &val);
    val |= DIG_IPTAT_CORE_EN;
    pmu_write(PMU_REG_IPTAT_CORE_I_CFG, val);
}

void pmu_ntc_capture_disable(void)
{
    uint16_t val;

    pmu_read(PMU_REG_AVDD_EN, &val);
    val &= ~REG_PU_AVDD25_ANA;
    pmu_write(PMU_REG_AVDD_EN, val);

    pmu_read(PMU_REG_SAR_CLK_CFG, &val);
    val &= ~REG_SAR_INPUT_BUF_EN;
    pmu_write(PMU_REG_SAR_CLK_CFG, val);

    pmu_read(PMU_REG_SAR_PU_CFG, &val);
    val &= ~REG_SAR_PU_VREF_IN;
    pmu_write(PMU_REG_SAR_PU_CFG, val);

    pmu_read(PMU_REG_IPTAT_CORE_I_CFG, &val);
    val &= ~DIG_IPTAT_CORE_EN;
    pmu_write(PMU_REG_IPTAT_CORE_I_CFG, val);
}

void pmu_bt_reconn(bool en)
{
    uint16_t pmu_val = 0;

    pmu_read(0x38, &pmu_val);
    //TRACE(1,"origin: PMU_38=0x%x", pmu_val);
    pmu_val &= 0xfff0;      //clear [3:0]

    if(en){
        pmu_val |= 0xf;
    }
    pmu_write(0x38, pmu_val);

    //pmu_read(0x38, &pmu_val);
    //TRACE(1,"final: PMU_38=0x%x", pmu_val);
}

void pmu_bt_2v5pa(bool en)
{
}

void pmu_vio_3v3(bool en)
{
    uint32_t lock;

    lock = int_lock();
    if (en) {
        pmu_module_config(PMU_SENSOR, PMU_MANUAL_MODE, PMU_LDO_ON, PMU_LP_MODE_ON, PMU_DSLEEP_MODE_ON);
        pmu_module_ramp_volt(PMU_SENSOR, PMU_LDO_SENSOR_3_3V, PMU_LDO_SENSOR_3_3V);
    } else {
        pmu_module_config(PMU_SENSOR, PMU_MANUAL_MODE, PMU_LDO_OFF, PMU_LP_MODE_ON, PMU_DSLEEP_MODE_OFF);
    }
    int_unlock(lock);
}

void pmu_sar_adc_vref_enable(void)
{
    uint16_t val;

#ifdef PMU_FORCE_LP_MODE
    pmu_big_bandgap_enable(PMU_BIG_BANDGAP_USER_SAR_VREF_OUTPUT, true);
#endif

    // force enable
    pmu_read(PMU_REG_AVDD_EN, &val);
    val |= REG_PU_AVDD25_ANA;
    pmu_write(PMU_REG_AVDD_EN, val);

    pmu_read(PMU_REG_SAR_PU_CFG, &val);
    val |= REG_SAR_PU_VREF_REG | REG_SAR_PU_VREF_DR | REG_SAR_PU_REG | REG_SAR_PU_DR;
    pmu_write(PMU_REG_SAR_PU_CFG, val);

    // sar vref output
    pmu_sar_adc_vref_sw_pu(true);
}

void pmu_sar_adc_vref_disable(void)
{
    uint16_t val;

#ifdef PMU_FORCE_LP_MODE
    pmu_big_bandgap_enable(PMU_BIG_BANDGAP_USER_SAR_VREF_OUTPUT, false);
#endif

    pmu_sar_adc_vref_sw_pu(false);

    pmu_read(PMU_REG_SAR_PU_CFG, &val);
    val &= ~(REG_SAR_PU_VREF_REG | REG_SAR_PU_VREF_DR | REG_SAR_PU_REG | REG_SAR_PU_DR);
    pmu_write(PMU_REG_SAR_PU_CFG, val);

    pmu_read(PMU_REG_AVDD_EN, &val);
    val &= ~REG_PU_AVDD25_ANA;
    pmu_write(PMU_REG_AVDD_EN, val);
}

static int pmu_vsys_div_ctrl(int enable)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    pmu_read(PMU_REG_PU_VSYS_DIV, &val);
    if (enable) {
        val |= REG_PU_VSYS_DIV;
    } else {
        val &= ~REG_PU_VSYS_DIV;
    }
    pmu_write(PMU_REG_PU_VSYS_DIV, val);
    int_unlock(lock);

    return 0;
}

static int pmu_vbat_div_ctrl(int enable)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    pmu_read(PMU_REG_POWER_KEY_CFG, &val);
    if (enable) {
        val |= REG_PU_VBAT_DIV;
    } else {
        val &= ~REG_PU_VBAT_DIV;
    }
    pmu_write(PMU_REG_POWER_KEY_CFG, val);
    int_unlock(lock);

    return 0;
}

int pmu_gpadc_div_ctrl(enum HAL_GPADC_CHAN_T channel, int enable)
{
    int ret = 0;

    if (channel == HAL_GPADC_CHAN_BATTERY) {
        ret = pmu_vsys_div_ctrl(enable);
    } else if (channel == HAL_GPADC_CHAN_6) {
        ret = pmu_vbat_div_ctrl(enable);
    }

    return ret;
}

static void pmu_ntc_irq_handler(uint16_t raw, uint16_t volt)
{
    pmu_ntc_capture_disable();

    ntc_temperature = pmu_volt2temperature(volt);
    ntc_raw = raw;

    if (pmu_ntc_cb) {
        pmu_ntc_cb(ntc_raw, ntc_temperature);
    }

    ntc_irq_busy = false;
}

int pmu_ntc_capture_start(PMU_NTC_IRQ_HANDLER_T cb)
{
    int nRet = 0;
    uint32_t lock = 0;

    lock = int_lock();
    if (ntc_irq_busy) {
        nRet = -1;
        goto exit;
    }
    pmu_ntc_cb = cb;
    ntc_irq_busy = true;
    pmu_ntc_capture_enable();
    hal_gpadc_open(HAL_GPADC_CHAN_0, HAL_GPADC_ATP_ONESHOT, pmu_ntc_irq_handler);
exit:
    int_unlock(lock);

    return nRet;
}

int pmu_volt2temperature(const uint16_t volt)
{
    const int Kvtherm_T = 385;
    static uint16_t temp_ref_raw2volt = 0;
    int temp_centigrade = 0;

    if (!temp_ref_raw2volt) {
        bool calibed = true;

        pmu_get_efuse(PMU_EFUSE_PAGE_TEMPERATURE, &temp_ref_raw2volt);
        if (!temp_ref_raw2volt) {
            temp_ref_raw2volt = PMU_EFUSE_TEMPSENSOR_DEFAULT;
            calibed = false;
        }
        temp_ref_raw2volt = hal_gpadc_adc2volt(temp_ref_raw2volt);
        TRACE(0, "%s calibed=%d temp_ref_raw2volt=%d", __func__, calibed, temp_ref_raw2volt);
    }

    temp_centigrade = (int)(((volt - temp_ref_raw2volt) * 100 + Kvtherm_T / 2) / Kvtherm_T + 25);

    return temp_centigrade;
}

void pmu_power_key_hw_reset_enable(uint8_t seconds)
{
    uint16_t val;
    uint32_t lock;
    const int sample_clk_freq_hz = 4;

    seconds *= sample_clk_freq_hz;
    if (seconds > 60) {
        seconds = 60;
    }

#ifdef SIMU
    pmu_write(PMU_REG_DIV_HW_RESET_CFG, 32 / sample_clk_freq_hz - 2);
#else
    pmu_write(PMU_REG_DIV_HW_RESET_CFG, CONFIG_SYSTICK_HZ / sample_clk_freq_hz * 2 - 2);
#endif

    lock = int_lock();
    pmu_read(PMU_REG_WDT_CFG, &val);
    val = SET_BITFIELD(val, REG_HW_RESET_TIME, seconds) | REG_HW_RESET_EN;
    pmu_write(PMU_REG_WDT_CFG, val);
    int_unlock(lock);
}

void pmu_power_key_hw_reset_disable(void)
{
    uint16_t val;

    pmu_read(PMU_REG_WDT_CFG, &val);
    val &= ~REG_HW_RESET_EN;
    pmu_write(PMU_REG_WDT_CFG, val);
}

#if defined(PMU_CLK_USE_EXT_CRYSTAL) || defined(RTC_CLK_USE_EXT_CRYSTAL)
static void BOOT_TEXT_FLASH_LOC pmu_external_crystal_init(void)
{
    uint16_t val;

    pmu_read(PMU_REG_XO_32K_CFG, &val);
    if (val & REG_XO_32K_PU_LDO) {
        return;
    }

    pmu_external_crystal_capbit_set(PMU_EXT_32K_CAPBIT_DEFAULT);

    // power on external crystal.
    pmu_read(PMU_REG_XO_32K_CFG, &val);
    val = SET_BITFIELD(val, REG_XO_32K_LDO_PRECHARGE_VALUE, 0x1);
    val = SET_BITFIELD(val, REG_XO_32K_ISEL_VALUE, 0x10);
    pmu_write(PMU_REG_XO_32K_CFG, val);

    pmu_read(PMU_REG_XO_32K_ISEL_RAMP, &val);
    val = SET_BITFIELD(val, REG_XO_32K_ISEL_RAMP_START_DLY, 0x100);
    pmu_write(PMU_REG_XO_32K_ISEL_RAMP, val);

    pmu_read(PMU_REG_XO_32K_CFG, &val);
    val |= REG_XO_32K_PU_LDO | REG_XO_32K_PU_LDO_DR | REG_XO_32K_ISEL_DR;
    pmu_write(PMU_REG_XO_32K_CFG, val);

    pmu_ext_crystal_init_stime = hal_sys_timer_get();
}

static void pmu_external_crystal_clkout_after_pwroff(void)
{
    if (pmu_ext_crystal_clkout_usr_map & PMU_EXT_CRYSTAL_CLKOUT_PIN_0) {
        pmu_external_crystal_clkout_enable(PMU_EXT_CRYSTAL_CLKOUT_PIN_0);
    }
    if (pmu_ext_crystal_clkout_usr_map & PMU_EXT_CRYSTAL_CLKOUT_PIN_1) {
        pmu_external_crystal_clkout_enable(PMU_EXT_CRYSTAL_CLKOUT_PIN_1);
    }
    if (pmu_ext_crystal_clkout_usr_map & PMU_EXT_CRYSTAL_CLKOUT_PIN_2) {
        pmu_external_crystal_clkout_enable(PMU_EXT_CRYSTAL_CLKOUT_PIN_2);
    }
}

void pmu_external_crystal_enable(void)
{
    // External crystal can not be cleared by reg reset and will resume xo32k config after hw reset.
    while (hal_sys_timer_get() - pmu_ext_crystal_init_stime < MS_TO_TICKS(PMU_EXT_32K_STABLE_TIME));

#ifdef PMU_CLK_USE_EXT_CRYSTAL
    uint16_t val;

    // PMU clock slecet to external crystal.
    pmu_read(PMU_REG_CLK_SEL_CFG, &val);
    val = SET_BITFIELD(val, CLK_32K_SEL, 0x2);
    pmu_write(PMU_REG_CLK_SEL_CFG, val);
    hal_sys_timer_delay(MS_TO_TICKS(PMU_CLK_SWITCH_STABLE_TIME));
#endif
}

void pmu_external_crystal_clkout_enable(enum PMU_EXT_CRYSTAL_CLKOUT_PIN_T pin)
{
    uint16_t val;

    if (pin == PMU_EXT_CRYSTAL_CLKOUT_PIN_0) {
        pmu_read(PMU_REG_PU_VSYS_DIV, &val);
        val |= REG_CLK_32K_EXT1_EN;
        pmu_write(PMU_REG_PU_VSYS_DIV, val);
    } else if (pin == PMU_EXT_CRYSTAL_CLKOUT_PIN_1) {
        pmu_read(PMU_REG_PU_VSYS_DIV, &val);
        val |= REG_CLK_32K_EXT2_EN;
        pmu_write(PMU_REG_PU_VSYS_DIV, val);
    } else {
        pmu_read(PMU_REG_PU_VSYS_DIV, &val);
        val |= REG_CLK_32K_EXT3_EN;
        pmu_write(PMU_REG_PU_VSYS_DIV, val);
    }
    pmu_ext_crystal_clkout_usr_map |= pin;
    hal_sys_timer_delay(MS_TO_TICKS(PMU_EXT_32K_CLK_OUT_STABLE_TIME));
}

void pmu_external_crystal_clkout_disable(enum PMU_EXT_CRYSTAL_CLKOUT_PIN_T pin)
{
    uint16_t val;

    if (pin == PMU_EXT_CRYSTAL_CLKOUT_PIN_0) {
        pmu_read(PMU_REG_PU_VSYS_DIV, &val);
        val &= ~REG_CLK_32K_EXT1_EN;
        pmu_write(PMU_REG_PU_VSYS_DIV, val);
    } else if (pin == PMU_EXT_CRYSTAL_CLKOUT_PIN_1) {
        pmu_read(PMU_REG_PU_VSYS_DIV, &val);
        val &= ~REG_CLK_32K_EXT2_EN;
        pmu_write(PMU_REG_PU_VSYS_DIV, val);
    } else {
        pmu_read(PMU_REG_PU_VSYS_DIV, &val);
        val &= ~REG_CLK_32K_EXT3_EN;
        pmu_write(PMU_REG_PU_VSYS_DIV, val);
    }
    pmu_ext_crystal_clkout_usr_map &= ~pin;
}

void pmu_external_crystal_capbit_set(uint32_t capbit)
{
    uint16_t val;

    pmu_read(PMU_REG_PU_VSYS_DIV, &val);
    val = SET_BITFIELD(val, REG_XO_32K_CAPBIT, capbit);
    pmu_write(PMU_REG_PU_VSYS_DIV, val);
}

void pmu_external_crystal_capbit_get(uint32_t *p_capbit)
{
    uint16_t val;

    pmu_read(PMU_REG_PU_VSYS_DIV, &val);
    *p_capbit = GET_BITFIELD(val, REG_XO_32K_CAPBIT);
}

#ifdef PMU_CLK_USE_EXT_CRYSTAL
int pmu_external_crystal_calib(uint32_t *p_wait_ms)
{
    uint32_t time;
    int ret;
    float hz;
    uint32_t capbit;

    TRACE(0, "%s: cur_state=%d", __func__, xtal_32k_calib_state);

    *p_wait_ms = 0;
    ret = -100;

    if (xtal_32k_calib_state == XTAL_32K_CALIB_STATE_NULL) {
_null_start:
        xtal_32k_calib_prev_valid = false;
_calib_start:
        hal_sys_timer_calib_start();
        hal_bus_wake_lock(HAL_BUS_WAKE_LOCK_USER_PMU);
        xtal_32k_calib_state = XTAL_32K_CALIB_STATE_CALIB;
        xtal_32k_calib_time = hal_sys_timer_get();
        *p_wait_ms = xtal_32k_calib_intvl_ms;
        ret = 1;
        goto _exit;
    } else if (xtal_32k_calib_state == XTAL_32K_CALIB_STATE_CALIB) {
        time = hal_sys_timer_get();
        time = TICKS_TO_MS(time - xtal_32k_calib_time);
        if (time < xtal_32k_calib_intvl_ms) {
            *p_wait_ms = xtal_32k_calib_intvl_ms - time;
            ret = 2;
            goto _exit;
        }
        ret = hal_sys_timer_calib_end();
        hal_bus_wake_unlock(HAL_BUS_WAKE_LOCK_USER_PMU);
        if (ret) {
            TRACE(0, "%s: calib_end failed with %d", __func__, ret);
            xtal_32k_calib_state = XTAL_32K_CALIB_STATE_NULL;
            ret = -1;
            goto _exit;
        }
        hz = hal_sys_timer_systick_hz_float();
        pmu_external_crystal_capbit_get(&capbit);
        if (xtal_32k_calib_prev_valid) {
            TRACE(0, "%s: hz=%u.%02u capbit=0x%X prev=%u.%02u prev_capbit=0x%X",
                __func__,
                (unsigned)hz, ((unsigned)(hz * 100) - ((unsigned)hz * 100)),
                capbit,
                (unsigned)xtal_32k_calib_prev_hz, ((unsigned)(xtal_32k_calib_prev_hz * 100) - ((unsigned)xtal_32k_calib_prev_hz * 100)),
                xtal_32k_calib_prev_capbit);
        } else {
            TRACE(0, "%s: hz=%u.%02u", __func__, (unsigned)hz, ((unsigned)(hz * 100) - ((unsigned)hz * 100)));
        }
        if (ABS(hz - CONFIG_SYSTICK_HZ_NOMINAL) < 0.1) {
            // Calib result OK
            TRACE(0, "%s: Calib result OK", __func__);
            xtal_32k_calib_state = XTAL_32K_CALIB_STATE_NULL;
            ret = 0;
            goto _exit;
        } else if (xtal_32k_calib_prev_valid) {
            // If no better result
            if (hz < CONFIG_SYSTICK_HZ_NOMINAL && CONFIG_SYSTICK_HZ_NOMINAL < xtal_32k_calib_prev_hz) {
                if (CONFIG_SYSTICK_HZ_NOMINAL - hz <= xtal_32k_calib_prev_hz - CONFIG_SYSTICK_HZ_NOMINAL) {
                    TRACE(0, "%s: Calib no better result: Select cur", __func__);
                    xtal_32k_calib_state = XTAL_32K_CALIB_STATE_NULL;
                    ret = 0;
                    goto _exit;
                } else {
                    TRACE(0, "%s: Calib no better result: Select prev", __func__);
                    pmu_external_crystal_capbit_set(xtal_32k_calib_prev_capbit);
                    xtal_32k_calib_state = XTAL_32K_CALIB_STATE_ROLLBACK;
                    xtal_32k_calib_time = hal_sys_timer_get();
                    ret = 0;
                    goto _exit;
                }
            } else if (xtal_32k_calib_prev_hz < CONFIG_SYSTICK_HZ_NOMINAL && CONFIG_SYSTICK_HZ_NOMINAL < hz) {
                if (hz - CONFIG_SYSTICK_HZ_NOMINAL <= CONFIG_SYSTICK_HZ_NOMINAL - xtal_32k_calib_prev_hz) {
                    TRACE(0, "%s: Calib no better result: Select cur", __func__);
                    xtal_32k_calib_state = XTAL_32K_CALIB_STATE_NULL;
                    ret = 0;
                    goto _exit;
                } else {
                    TRACE(0, "%s: Calib no better result: Select prev", __func__);
                    pmu_external_crystal_capbit_set(xtal_32k_calib_prev_capbit);
                    xtal_32k_calib_state = XTAL_32K_CALIB_STATE_ROLLBACK;
                    xtal_32k_calib_time = hal_sys_timer_get();
                    ret = 0;
                    goto _exit;
                }

            }
        }
        // Save as previous values
        xtal_32k_calib_prev_hz = hz;
        xtal_32k_calib_prev_capbit = capbit;
        xtal_32k_calib_prev_valid = true;
        if (hz > CONFIG_SYSTICK_HZ_NOMINAL) {
            if (capbit >= (REG_XO_32K_CAPBIT_MASK >> REG_XO_32K_CAPBIT_SHIFT)) {
                TRACE(0, "%s: capbit too large: 0x%X", __func__, capbit);
                xtal_32k_calib_state = XTAL_32K_CALIB_STATE_NULL;
                ret = -2;
                goto _exit;
            }
            capbit += 0x10;
            if (capbit > (REG_XO_32K_CAPBIT_MASK >> REG_XO_32K_CAPBIT_SHIFT)) {
                capbit = (REG_XO_32K_CAPBIT_MASK >> REG_XO_32K_CAPBIT_SHIFT);
            }
        } else {
            if (capbit <= 1) {
                TRACE(0, "%s: capbit too small: 0x%X", __func__, capbit);
                xtal_32k_calib_state = XTAL_32K_CALIB_STATE_NULL;
                ret = -3;
                goto _exit;
            }
            if (capbit > 0x10) {
                capbit -= 0x10;
            } else {
                capbit = 1;
            }
        }
        TRACE(0, "%s: Set capbit=0x%X", __func__, capbit);
        pmu_external_crystal_capbit_set(capbit);
        xtal_32k_calib_state = XTAL_32K_CALIB_STATE_SET;
        xtal_32k_calib_time = hal_sys_timer_get();
        *p_wait_ms = xtal_32k_calib_stable_ms;
        ret = 3;
        goto _exit;
    } else if (xtal_32k_calib_state == XTAL_32K_CALIB_STATE_SET) {
        time = hal_sys_timer_get();
        time = TICKS_TO_MS(time - xtal_32k_calib_time);
        if (time < xtal_32k_calib_stable_ms) {
            *p_wait_ms = xtal_32k_calib_stable_ms - time;
            ret = 4;
            goto _exit;
        }
        goto _calib_start;
    } else if (xtal_32k_calib_state == XTAL_32K_CALIB_STATE_ROLLBACK) {
        time = hal_sys_timer_get();
        time = TICKS_TO_MS(time - xtal_32k_calib_time);
        if (time < xtal_32k_calib_stable_ms) {
            *p_wait_ms = xtal_32k_calib_stable_ms - time;
            ret = 5;
            goto _exit;
        }
        goto _null_start;
    } else {
        ASSERT(false, "%s: Bad xtal_32k_calib_state=%d", __func__, xtal_32k_calib_state);
    }

_exit:
    TRACE(0, "%s: ret=%d next_state=%d", __func__, ret, xtal_32k_calib_state);
    return ret;
}
#endif
#endif

static void pmu_rc_clock_dr_cfg(bool en)
{
    uint16_t val;

    pmu_read(PMU_REG_RCOSC_192M_CFG, &val);
    if (en) {
        val |= REG_RCOSC_192M_EN_DR | REG_RCOSC_192M_EN | REG_RCOSC_192M_PU_LDO_DR | REG_RCOSC_192M_PU_LDO;
    } else {
        val &= ~(REG_RCOSC_192M_EN_DR | REG_RCOSC_192M_EN | REG_RCOSC_192M_PU_LDO_DR | REG_RCOSC_192M_PU_LDO);
    }
    pmu_write(PMU_REG_RCOSC_192M_CFG, val);
}

void pmu_rc_clk_enable(enum PMU_CLK_SEL_RC_USER_T user)
{
    int ret = 0;
    uint16_t val;

    if (rc_clk_user_map == 0) {
        pmu_rc_clock_dr_cfg(true);

        pmu_read(PMU_REG_RCOSC_192M_CFG, &val);
        if (user == PMU_CLK_SEL_RC_USER_CAP) {
            val &= ~REG_RCOSC_192M_EN_CLK_DIG;
        } else {
            val |= REG_RCOSC_192M_EN_CLK_DIG;
        }
        pmu_write(PMU_REG_RCOSC_192M_CFG, val);

        ret = pmu_rc_clk_calib();
        if (ret) {
            PMU_INFO_TRACE_IMM(0, "\nError: pmu_rc_clk_calib failed! \n");
        }
    }

    rc_clk_user_map |= user;

    if (user == PMU_CLK_SEL_RC_USER_SYS) {
        pmu_read(PMU_REG_RCOSC_192M_CFG, &val);
        val |= REG_RCOSC_192M_EN_CLK_DIG;
        pmu_write(PMU_REG_RCOSC_192M_CFG, val);
    }

    pmu_read(PMU_REG_RCOSC_192M_CFG, &val);
    val &= ~(REG_RCOSC_192M_PU_LDO_DR | REG_RCOSC_192M_EN_DR);
    pmu_write(PMU_REG_RCOSC_192M_CFG, val);
}

void pmu_rc_clk_disable(enum PMU_CLK_SEL_RC_USER_T user)
{
    uint16_t val;

    if (rc_clk_user_map == 0) {
        ASSERT(0, "no user use rc clk");
    }

    rc_clk_user_map &= ~user;

    if (user == PMU_CLK_SEL_RC_USER_SYS) {
        hal_cmu_select_osc_clock(HAL_CMU_CLK_SEL_OSC_USER_SYS);

        pmu_read(PMU_REG_RCOSC_192M_CFG, &val);
        val &= ~REG_RCOSC_192M_EN_CLK_DIG;
        pmu_write(PMU_REG_RCOSC_192M_CFG, val);
    }

    if (rc_clk_user_map == 0) {
        pmu_rc_clock_dr_cfg(false);
    }
}

int pmu_rc_clk_calib(void)
{
    int ret = 0;
    uint16_t val;

    pmu_read(PMU_REG_DBG_RCOSC6M_REF_CNT, &val);
    val |= REG_RCOSC_192M_VOUT_SEL(0x4);
    pmu_write(PMU_REG_DBG_RCOSC6M_REF_CNT, val);

    pmu_read(PMU_REG_RCOSC_6M_GOAL_CNT, &val);
    val |= REG_RCOSC6M_GOAL_CNT_MARK_15_0(0x2fff);
    pmu_write(PMU_REG_RCOSC_6M_GOAL_CNT, val);

    pmu_read(PMU_REG_RCOSC_6M_CFG, &val);
    val &= ~REG_RCOSC6M_REF_DIV_NUM_MASK;
    val |= REG_RCOSC6M_REF_DIV_NUM(0x1FF);
    pmu_write(PMU_REG_RCOSC_6M_CFG, val);

    pmu_read(PMU_REG_RCOSC_192M_CFG, &val);
    val |= REG_RCOSC_192M_EN_CLK_DLATCH | REG_RCOSC_192M_EN_CLK_ADC_DIG;
    pmu_write(PMU_REG_RCOSC_192M_CFG, val);

    pmu_read(PMU_REG_RCOSC_192M_CALIB, &val);
    val |= REG_RCOSC_192M_CALIB_GATE;
    pmu_write(PMU_REG_RCOSC_192M_CALIB, val);

    pmu_read(PMU_REG_RCOSC_6M_CFG, &val);
    val |= REG_RCOSC6M_SFT_RSTN | REG_RCOSC6M_CALIB_EN;
    pmu_write(PMU_REG_RCOSC_6M_CFG, val);

#ifndef SIMU
    hal_sys_timer_delay(MS_TO_TICKS(10));
    pmu_read(PMU_REG_DBG_RCOSC_6M_CFG, &val);
    if ((val & DBG_RCOSC6M_TUN_DONE) == 0) {
        ret = 1;
    }
#endif

    // Following bits should be set to 0 after RC calib
    pmu_read(PMU_REG_RCOSC_192M_CFG, &val);
    val &= ~(REG_RCOSC_192M_EN_CLK_DLATCH | REG_RCOSC_192M_EN_CLK_ADC_DIG);
    pmu_write(PMU_REG_RCOSC_192M_CFG, val);

    pmu_read(PMU_REG_RCOSC_192M_CALIB, &val);
    val &= ~(REG_RCOSC_192M_CALIB_GATE);
    pmu_write(PMU_REG_RCOSC_192M_CALIB, val);

    return ret;
}

bool pmu_rc_clock_enabled(void)
{
    uint16_t val;
    bool enabled = false;

    pmu_read(PMU_REG_PWR_CLK_STATUS, &val);
    if (((val & RC_SLEEP_MODE) == 0) && (val & DEEPSLEEP_MODE)) {
        enabled = true;
    }

    return enabled;
}

void pmu_rc_clock_cap_enable(bool en)
{
#if !defined(CAP_RC_CLK_DR_PU) && !defined(SYS_RC_ENABLE)
    pmu_rc_clock_dr_cfg(en);
#endif
}

void pmu_capsensor_close(void)
{
}

void pmu_capsensor_open(uint8_t clk_cap_div)
{
    uint16_t val;

#if 0  //agpio test 16
    hal_sys_timer_delay(US_TO_TICKS(500));

    pmu_read(PMU_REG_RCOSC_192M_CFG, &val);
    val |= REG_RCOSC_192M_EN_AGPIO_TEST | REG_RCOSC_192M_EN_AGPIO_TEST_SEL;
    pmu_write(PMU_REG_RCOSC_192M_CFG, val);
#endif

    pmu_rc_clock_dr_cfg(true);

    pmu_read(PMU_REG_RCOSC_192M_CFG, &val);
    val |= REG_RCOSC_192M_EN_CLK_DLATCH;
    pmu_write(PMU_REG_RCOSC_192M_CFG, val);

    pmu_read(PMU_REG_DBG_RCOSC6M_REF_CNT, &val); //pmu pu_rc_cap msk
    val |= REG_PU_RC_CAP_MSK;
    pmu_write(PMU_REG_DBG_RCOSC6M_REF_CNT, val);

    pmu_read(PMU_REG_RCOSC_192M_CALIB, &val);
    val |= REG_RCOSC_192M_EN_CLK_CODEC_DIG;
    val = SET_BITFIELD(val, REG_RCOSC_192M_CODEC_SEL, 0x0);
    pmu_write(PMU_REG_RCOSC_192M_CALIB, val);
}

void pmu_ldo_vsensor_off_pull_down(void)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    pmu_module_config(PMU_SENSOR,PMU_MANUAL_MODE,PMU_LDO_OFF,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_OFF);

    pmu_read(PMU_REG_LDO_SENSOR_CFG, &val);
    val |= REG_LDO_VSENSOR_PULLDOWN;
    pmu_write(PMU_REG_LDO_SENSOR_CFG, val);
    int_unlock(lock);
}

void pmu_ldo_vsensor_on_lp_dr(bool dr_lp)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    pmu_read(PMU_REG_SENSOR_CFG, &val);
    val &= ~REG_PULL_DOWN_VSENSOR;
    pmu_write(PMU_REG_SENSOR_CFG, val);

    pmu_module_config(PMU_SENSOR,PMU_MANUAL_MODE,PMU_LDO_ON,PMU_LP_MODE_ON,PMU_DSLEEP_MODE_ON);

#ifndef PMU_FORCE_LP_MODE
    if (dr_lp) {
        pmu_read(PMU_REG_LDO_SENSOR_CFG, &val);
        val |= REG_LP_EN_LDO_VSENSOR_REG | REG_LP_EN_LDO_VSENSOR_DR;
        pmu_write(PMU_REG_LDO_SENSOR_CFG, val);
    }
#endif
    int_unlock(lock);
}
