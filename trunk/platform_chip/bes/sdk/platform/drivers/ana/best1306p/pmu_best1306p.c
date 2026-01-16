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
#include "analog.h"
#include "charger.h"
#include "cmsis.h"
#include "cmsis_nvic.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
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
#include "hwtimer_list.h"

#define PMU_EFUSE_INIT_IN_BOOT
// #define VRTC_SWITCH_TO_VCORE
#ifndef MTEST_ENABLED
#define PMU_INTF_CALIBED_VOLT
#endif

#ifdef RC_FAST_WAKEUP
#ifndef RC_CLK_ENABLE
#error "Invalid config, RC_FAST_WAKEUP need define RC_CLK_ENABLE!"
#endif
// The fast ramp logic is only used when waking up, and the 2m clock needs to be actively turned on when working.
// TODO: When using fast wake up, the voltage ramp action needs to be advanced. chenz
#define DCDC_VOLT_FAST_HW_RAMP
#endif

#ifndef RC_CLK_ENABLE
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

#define ana_read(reg,val)               hal_analogif_reg_read(ANA_REG(reg),val)
#define ana_write(reg,val)              hal_analogif_reg_write(ANA_REG(reg),val)
#define rf_read(reg,val)                hal_analogif_reg_read(RF_REG(reg),val)
#define rf_write(reg,val)               hal_analogif_reg_write(RF_REG(reg),val)

// LDO soft start interval is about 1000 us
#define PMU_LDO_PU_STABLE_TIME_US       1800
#define PMU_LDO_PULLDOWN_STABLE_TIME_US 1000
#define PMU_LDO_VRTC_STABLE_TIME_US     1000
#define PMU_DCDC_PU_STABLE_TIME_US      100
#define PMU_DCDC_HW_RAMP_1_CLK_TIME_US  40
#define PMU_DCDC_HW_RAMP_1_CLK_STEP     16
#define PMU_DCDC_HW_RAMP_1_CLK_STEP_HALF 8
#ifdef DCDC_VOLT_FAST_HW_RAMP
#define PMU_DCDC_F_HW_RAMP_1_CLK_STEP   2
#define PMU_DCDC_F_HW_RAMP_1_CLK_STEP_HALF 1
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
#define PMU_DCDC_DIG_1_1V               0x1A0
#define PMU_DCDC_DIG_1_05V              0x187
#define PMU_DCDC_DIG_1_0V               0x16E
#define PMU_DCDC_DIG_0_95V              0x155
#define PMU_DCDC_DIG_0_9V               0x13C
#define PMU_DCDC_DIG_0_85V              0x123
#define PMU_DCDC_DIG_0_8V               0x10A
#define PMU_DCDC_DIG_0_78V              0x100
#define PMU_DCDC_DIG_0_75V              0xF1
#define PMU_DCDC_DIG_0_725V             0xE5
#define PMU_DCDC_DIG_0_7V               0xD8
#define PMU_DCDC_DIG_0_65V              0xBF
#define PMU_DCDC_DIG_0_6V               0xA6
#define PMU_DCDC_DIG_0_55V              0x8D
#define PMU_DCDC_DIG_0_5V               0x74
#define PMU_DCDC_DIG_50_MV              (PMU_DCDC_DIG_0_85V - PMU_DCDC_DIG_0_8V)
#define PMU_DCDC_DIG_DEFAULT            0x10A /* 0.8V */

// 4mV
#define PMU_DCDC_ANA_1_4V               0xD4
#define PMU_DCDC_ANA_1_3V               0xBB
#define PMU_DCDC_ANA_1_25V              0xAF
#define PMU_DCDC_ANA_1_2V               0xA2
#define PMU_DCDC_ANA_1_1V               0x89
#define PMU_DCDC_ANA_1_0V               0x70
#define PMU_DCDC_ANA_DEFAULT            0xBB /* 1.3V */

// 4mV
#define PMU_DCDC_HPPA_1_95V             0x166
#define PMU_DCDC_HPPA_1_9V              0x159
#define PMU_DCDC_HPPA_1_8V              0x140
#define PMU_DCDC_HPPA_1_75V             0x134
#define PMU_DCDC_HPPA_1_7V              0x127
#define PMU_DCDC_HPPA_1_65V             0x11B
#define PMU_DCDC_HPPA_1_6V              0x10E
#define PMU_DCDC_HPPA_1_55V             0x102
#define PMU_DCDC_HPPA_1_5V              0xF5
#define PMU_DCDC_HPPA_1_45V             0xE9
#define PMU_DCDC_HPPA_1_4V              0xDC
#define PMU_DCDC_HPPA_DEFAULT           0x140 /* 1.8V */

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

#define PMU_VMIC_2_8V                   0x37
#define PMU_VMIC_1_6V                   0x16

#define PMU_VMIC_RES_3_3V               0xF
#define PMU_VMIC_RES_2_8V               0xA

#define PMU_USB_3_3V                    0x2A
#define PMU_USB_3_2V                    0x28
#define PMU_USB_3_1V                    0x26
#define PMU_USB_3_0V                    0x24
#define PMU_USB_2_9V                    0x22
#define PMU_USB_2_8V                    0x20
#define PMU_USB_2_7V                    0x1E
#define PMU_USB_2_6V                    0x1C
#define PMU_USB_2_5V                    0x1A
#define PMU_USB_2_4V                    0x18
#define PMU_USB_2_3V                    0x16
#define PMU_USB_2_2V                    0x14
#define PMU_USB_2_1V                    0x12
#define PMU_USB_2_0V                    0x10
#define PMU_USB_1_9V                    0xE
#define PMU_USB_1_8V                    0xC

#define MAX_BUCK_VANA_BIT_VAL                   (REG_BUCK_VANA_VBIT_NORMAL_MASK >> REG_BUCK_VANA_VBIT_NORMAL_SHIFT)
#define MAX_BUCK_VCORE_BIT_VAL                  (REG_BUCK_VCORE_VBIT_NORMAL_MASK >> REG_BUCK_VCORE_VBIT_NORMAL_SHIFT)
#define MAX_BUCK_VHPPA_BIT_VAL                  (REG_BUCK_VHPPA_VBIT_NORMAL_MASK >> REG_BUCK_VHPPA_VBIT_NORMAL_SHIFT)
#define MAX_LDO_VCORE_L_BIT_VAL                 (REG_LDO_VCORE_L_VBIT_NORMAL_MASK >> REG_LDO_VCORE_L_VBIT_NORMAL_SHIFT)

#define INTR_MSKED_CHARGE_OUT                   INTR_MSKED_CHARGE(1 << 1)
#define INTR_MSKED_CHARGE_IN                    INTR_MSKED_CHARGE(1 << 0)

#define REG_DCDC1_RAMP_EN                       (1 << 1)
#define REG_DCDC2_RAMP_EN                       (1 << 2)
#define REG_DCDC3_RAMP_EN                       (1 << 3)

#define REG_DCDC_CALIB_MTEST                    PMU_REG_RTC_LOAD_HOURS

// PMU_VMIN_SEARCH
#define PMU_VMIN_SET_REG_ADDR                   PMU_REG_RTC_LOAD_MIN_SEC_EN
#define PMU_VMIN_VCORE_SET_DONE                 (1 << 15)
#define PMU_VMIN_VCORE_SET_PLUS                 (1 << 14)
#define PMU_VMIN_VCORE_SET_OFFSET_SHIFT         0
#define PMU_VMIN_VCORE_SET_OFFSET_MASK          (0x1F << PMU_VMIN_VCORE_SET_OFFSET_SHIFT)
#define PMU_VMIN_VCORE_SET_OFFSET(n)            BITFIELD_VAL(PMU_VMIN_VCORE_SET_OFFSET, n)

// PMU_EFUSE_PAGE_BATTER_LV
#define PMU_EFUSE_PAGE_BATTER_LV_OFFSET_SHIFT   0
#define PMU_EFUSE_PAGE_BATTER_LV_OFFSET_MASK    (0xFFF << PMU_EFUSE_PAGE_BATTER_LV_OFFSET_SHIFT)
#define PMU_EFUSE_PAGE_BATTER_LV_OFFSET(n)      BITFIELD_VAL(PMU_EFUSE_PAGE_BATTER_LV_OFFSET, n)
#define PMU_EFUSE_PAGE_BATTER_LV_BASE           26600
#define PMU_EFUSE_VANA_THRESHOLD_SHIFT          12
#define PMU_EFUSE_VANA_THRESHOLD_MASK           (0xF << PMU_EFUSE_VANA_THRESHOLD_SHIFT)
#define PMU_EFUSE_VANA_THRESHOLD(n)             BITFIELD_VAL(PMU_EFUSE_VANA_THRESHOLD, n)

// PMU_EFUSE_PAGE_BATTER_HV
#define PMU_EFUSE_PAGE_BATTER_HV_OFFSET_SHIFT   0
#define PMU_EFUSE_PAGE_BATTER_HV_OFFSET_MASK    (0xFFF << PMU_EFUSE_PAGE_BATTER_HV_OFFSET_SHIFT)
#define PMU_EFUSE_PAGE_BATTER_HV_OFFSET(n)      BITFIELD_VAL(PMU_EFUSE_PAGE_BATTER_HV_OFFSET, n)
#define PMU_EFUSE_PAGE_BATTER_HV_BASE           30700
#define PMU_EFUSE_VCORE_THRESHOLD_SHIFT         12
#define PMU_EFUSE_VCORE_THRESHOLD_MASK          (0x7 << PMU_EFUSE_VCORE_THRESHOLD_SHIFT)
#define PMU_EFUSE_VCORE_THRESHOLD(n)            BITFIELD_VAL(PMU_EFUSE_VCORE_THRESHOLD, n)
#define PMU_EFUSE_VCORE_THRESHOLD_BIT_0         (1 << 15)

// PMU_EFUSE_PAGE_SW_CFG
#define PMU_EFUSE_VHPPA_IS_GAIN_BIT_1_0_SHIFT   12
#define PMU_EFUSE_VHPPA_IS_GAIN_BIT_1_0_MASK    (0x3 << PMU_EFUSE_VHPPA_IS_GAIN_BIT_1_0_SHIFT)
#define PMU_EFUSE_VHPPA_IS_GAIN_BIT_1_0(n)      BITFIELD_VAL(PMU_EFUSE_VHPPA_IS_GAIN_BIT_1_0, n)
#define PMU_EFUSE_VANA_IS_GAIN_BIT_1_0_SHIFT    14
#define PMU_EFUSE_VANA_IS_GAIN_BIT_1_0_MASK     (0x3 << PMU_EFUSE_VANA_IS_GAIN_BIT_1_0_SHIFT)
#define PMU_EFUSE_VANA_IS_GAIN_BIT_1_0(n)       BITFIELD_VAL(PMU_EFUSE_VANA_IS_GAIN_BIT_1_0, n)

// PMU_EFUSE_PAGE_EXT_GPADC_LV
#define PMU_EFUSE_PAGE_EXT_GPADC_LV_OFFSET_SHIFT 0
#define PMU_EFUSE_PAGE_EXT_GPADC_LV_OFFSET_MASK  (0xFFF << PMU_EFUSE_PAGE_EXT_GPADC_LV_OFFSET_SHIFT)
#define PMU_EFUSE_PAGE_EXT_GPADC_LV_OFFSET(n)    BITFIELD_VAL(PMU_EFUSE_PAGE_EXT_GPADC_LV_OFFSET, n)
#define PMU_EFUSE_PAGE_EXT_GPADC_LV_BASE         20000
#define PMU_EFUSE_VHPPA_THRESHOLD_SHIFT          12
#define PMU_EFUSE_VHPPA_THRESHOLD_MASK           (0x7 << PMU_EFUSE_VHPPA_THRESHOLD_SHIFT)
#define PMU_EFUSE_VHPPA_THRESHOLD(n)             BITFIELD_VAL(PMU_EFUSE_VHPPA_THRESHOLD, n)
#define PMU_EFUSE_VHPPA_THRESHOLD_BIT_0          (1 << 15)

// PMU_EFUSE_PAGE_EXT_GPADC_HV
#define PMU_EFUSE_PAGE_EXT_GPADC_HV_OFFSET_SHIFT 0
#define PMU_EFUSE_PAGE_EXT_GPADC_HV_OFFSET_MASK  (0xFFF << PMU_EFUSE_PAGE_EXT_GPADC_HV_OFFSET_SHIFT)
#define PMU_EFUSE_PAGE_EXT_GPADC_HV_OFFSET(n)    BITFIELD_VAL(PMU_EFUSE_PAGE_EXT_GPADC_HV_OFFSET, n)
#define PMU_EFUSE_PAGE_EXT_GPADC_HV_BASE         36500
#define PMU_EFUSE_PAGE_LPO_STABLE                (1 << 13)
#define PMU_EFUSE_PAGE_ULTRA_LOW_SLEEP_VCORE     (1 << 14)

#define PMU_EFUSE_GPADC_CALIB_DEFAULT            2000

// PMU_EFUSE_PAGE_VBG_VRTC_CALIB
#define PMU_EFUSE_VBG_PMU_INTF_CALIB_ENABLE     (1 << 10)

// PMU_EFUSE_PAGE_DCDC_VOLT_CALIB
#define PMU_EFUSE_DCDC1_VOLT_COMP_SHIFT         0
#define PMU_EFUSE_DCDC1_VOLT_COMP_MASK          (0x1F << PMU_EFUSE_DCDC1_VOLT_COMP_SHIFT)
#define PMU_EFUSE_DCDC1_VOLT_COMP(n)            BITFIELD_VAL(PMU_EFUSE_DCDC1_VOLT_COMP, n)
#define PMU_EFUSE_DCDC1_VOLT_COMP_SIGN          (1 << 5)
#define PMU_EFUSE_DCDC2_VOLT_COMP_SHIFT         6
#define PMU_EFUSE_DCDC2_VOLT_COMP_MASK          (0xF << PMU_EFUSE_DCDC2_VOLT_COMP_SHIFT)
#define PMU_EFUSE_DCDC2_VOLT_COMP(n)            BITFIELD_VAL(PMU_EFUSE_DCDC2_VOLT_COMP, n)
#define PMU_EFUSE_DCDC2_VOLT_COMP_SIGN          (1 << 10)
#define PMU_EFUSE_DCDC3_VOLT_COMP_SHIFT         11
#define PMU_EFUSE_DCDC3_VOLT_COMP_MASK          (0xF << PMU_EFUSE_DCDC3_VOLT_COMP_SHIFT)
#define PMU_EFUSE_DCDC3_VOLT_COMP(n)            BITFIELD_VAL(PMU_EFUSE_DCDC3_VOLT_COMP, n)
#define PMU_EFUSE_DCDC3_VOLT_COMP_SIGN          (1 << 15)

// REG_CP_WAFER_CTX_XY
#define PMU_EFUSE_CP_WAFER_CTX_X_SHIFT          0
#define PMU_EFUSE_CP_WAFER_CTX_X_MASK           (0x7F << PMU_EFUSE_CP_WAFER_CTX_X_SHIFT)
#define PMU_EFUSE_CP_WAFER_CTX_X(n)             BITFIELD_VAL(PMU_EFUSE_CP_WAFER_CTX_X, n)
#define PMU_EFUSE_CP_WAFER_CTX_Y_SHIFT          7
#define PMU_EFUSE_CP_WAFER_CTX_Y_MASK           (0x7F << PMU_EFUSE_CP_WAFER_CTX_Y_SHIFT)
#define PMU_EFUSE_CP_WAFER_CTX_Y(n)             BITFIELD_VAL(PMU_EFUSE_CP_WAFER_CTX_Y, n)
#define PMU_EFUSE_PACKAGE_TYPE_SHIFT            14
#define PMU_EFUSE_PACKAGE_TYPE_MASK             (0x3 << PMU_EFUSE_PACKAGE_TYPE_SHIFT)
#define PMU_EFUSE_PACKAGE_TYPE(n)               BITFIELD_VAL(PMU_EFUSE_PACKAGE_TYPE, n)

// REG_CP_WAFER_CTX_ID
#define PMU_EFUSE_CP_WAFER_CTX_ID_SHIFT         0
#define PMU_EFUSE_CP_WAFER_CTX_ID_MASK          (0x1F << PMU_EFUSE_CP_WAFER_CTX_ID_SHIFT)
#define PMU_EFUSE_CP_WAFER_CTX_ID(n)            BITFIELD_VAL(PMU_EFUSE_CP_WAFER_CTX_ID, n)

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

// RF_REG_50F
#define REG_BT_XTAL_PU                          (1 << 8)
#define REG_BT_XTAL_PU_DR                       (1 << 7)

// RF_REG_517
#define REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH22_SHIFT  0
#define REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH22_MASK   (0x7 << REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH22_SHIFT)
#define REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH22(n)     BITFIELD_VAL(REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH22, n)
#define REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH46_SHIFT  3
#define REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH46_MASK   (0x7 << REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH46_SHIFT)
#define REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH46(n)     BITFIELD_VAL(REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH46, n)
#define REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH70_SHIFT  6
#define REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH70_MASK   (0x7 << REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH70_SHIFT)
#define REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH70(n)     BITFIELD_VAL(REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH70, n)
#define REG_XTAL_MDLL_OUTBUFFER_SWRC_I_OTHER_SHIFT 9
#define REG_XTAL_MDLL_OUTBUFFER_SWRC_I_OTHER_MASK  (0x7 << REG_XTAL_MDLL_OUTBUFFER_SWRC_I_OTHER_SHIFT)
#define REG_XTAL_MDLL_OUTBUFFER_SWRC_I_OTHER(n)    BITFIELD_VAL(REG_XTAL_MDLL_OUTBUFFER_SWRC_I_OTHER, n)

// RF_REG_521
#define REG_XTAL_MDLL_TIMING_T1_SHIFT           0
#define REG_XTAL_MDLL_TIMING_T1_MASK            (0xFF << REG_XTAL_MDLL_TIMING_T1_SHIFT)
#define REG_XTAL_MDLL_TIMING_T1(n)              BITFIELD_VAL(REG_XTAL_MDLL_TIMING_T1, n)

// RF_REG_524
#define REG_XTAL_MDLL_CAL_WAIT_TIME_SHIFT       3
#define REG_XTAL_MDLL_CAL_WAIT_TIME_MASK        (0x3F << REG_XTAL_MDLL_CAL_WAIT_TIME_SHIFT)
#define REG_XTAL_MDLL_CAL_WAIT_TIME(n)          BITFIELD_VAL(REG_XTAL_MDLL_CAL_WAIT_TIME, n)

// RF_REG_526
#define PU_OSC_DLY_DR                           (1 << 1)

// RF_REG_527
#define REG_BT_BG_PU                            (1 << 1)
#define REG_BT_BG_PU_DR                         (1 << 0)

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
#define REG_LP_BBPLL_SDM_FREQWORD_32            (1 << 0)

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

// RF_REG_544
#define REG_XTAL_MDLL_DLL_SWRC_SHIFT            0
#define REG_XTAL_MDLL_DLL_SWRC_MASK             (0x7 << REG_XTAL_MDLL_DLL_SWRC_SHIFT)
#define REG_XTAL_MDLL_DLL_SWRC(n)               BITFIELD_VAL(REG_XTAL_MDLL_DLL_SWRC, n)

// ANALOG REG_51
#define SOFT_RESETN_CAPSENSOR                   (1 << 14)
#define SOFT_RESETN_CAPSENSOR_REG               (1 << 15)

// ANALOG REG_16D
#define REG_ANALOG_PU_OSC                       (1 << 6)

// ANALOG REG_1AD
#define REG_ANALOG_CLK_SEL_DSI                  (1 << 0)
#define REG_ANALOG_EN_CLK_DSI                   (1 << 1)

#define PMU_CONFIG_SYSTICK_HZ_NOMINAL               (16000)
#define PMU_MAX_CALIB_SYSTICK_HZ                    (PMU_CONFIG_SYSTICK_HZ_NOMINAL * 2)
#define PMU_MIN_CALIB_TICKS                         (30 * (PMU_CONFIG_SYSTICK_HZ_NOMINAL / 1000))
#define PMU_MAX_CALIB_TICKS                         (30 * PMU_CONFIG_SYSTICK_HZ_NOMINAL)
#ifdef FAST_TIMER_COMPENSATE
#define PMU_FAST_TICK_RATIO_NUM                     3
#endif

#define PMU_EFUSE_TEMPSENSOR_DEFAULT            32347

#define REG_CP_WAFER_CTX_XY                     PMU_EFUSE_PAGE_CP_WAFER_XY_CTX
#define REG_CP_WAFER_CTX_ID                     PMU_EFUSE_PAGE_CP_WAFER_ID

#ifndef BBPLL_MHZ
#define BBPLL_MHZ                               (384)
#endif

enum RF_ANA_CHG_REG_T {
    CHG_REG_METAL_ID            = 0x00,

    ANA_REG_51                  = 0x51,
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
    RF_REG_50F                  = 0x50F,
    RF_REG_517                  = 0x517,
    RF_REG_521                  = 0x521,
    RF_REG_524                  = 0x524,
    RF_REG_526                  = 0x526,
    RF_REG_527                  = 0x527,
    RF_REG_52D                  = 0x52D,
    RF_REG_52F                  = 0x52F,
    RF_REG_530                  = 0x530,
    RF_REG_531                  = 0x531,
    RF_REG_534                  = 0x534,
    RF_REG_535                  = 0x535,
    RF_REG_537                  = 0x537,
    RF_REG_544                  = 0x544,
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

enum PMU_VUSB_REQ_T {
    PMU_VUSB_REQ_INIT               = (1 << 0),
    PMU_VUSB_REQ_USB                = (1 << 1),
    PMU_VUSB_REQ_GPIO               = (1 << 2),
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
    PMU_USB,

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
    PMU_MOD_CFG_VAL(USB),
};

#define OPT_TYPE                        const

static OPT_TYPE uint8_t ana_act_dcdc =
    PMU_DCDC_ANA_1_3V;
static OPT_TYPE uint8_t ana_lp_dcdc = ana_act_dcdc;

static OPT_TYPE POSSIBLY_UNUSED uint16_t vcodec_mv = (uint16_t)(VCODEC_VOLT * 1000);
static OPT_TYPE POSSIBLY_UNUSED uint16_t vhppa_mv = (uint16_t)(VHPPA_VOLT * 1000);

#ifdef CAPSENSOR_ENABLE
static OPT_TYPE uint8_t dig_lp_dcdc = PMU_DCDC_DIG_0_7V;
#else
static uint8_t BOOT_DATA_LOC dig_lp_dcdc = PMU_DCDC_DIG_0_65V;
#endif

static OPT_TYPE enum PMU_POWER_MODE_T BOOT_DATA_LOC pmu_power_mode = PMU_POWER_MODE_DIG_DCDC;

// TODO: Use the value in BL as the default value
static enum PMU_VCORE_REQ_T BOOT_BSS_LOC pmu_vcore_req;

static enum PMU_VUSB_REQ_T SRAM_BSS_DEF(pmu_vusb_req);

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

#ifndef MTEST_ENABLED
const
#endif
#if defined(PSRAM_ENABLE) || defined(PMU_ALREADY_INIT)
BOOT_DATA_LOC
#endif
static bool intf_calib_en = true;

static uint16_t SRAM_BSS_DEF(dcdc_ramp_map);

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

static bool pmu_sys_ctrl_work = false;

#ifdef RC_CLK_ENABLE
static enum PMU_CLK_SEL_RC_USER_T rc_clk_user_map = 0;
#endif

static enum PMU_PACKAGE_TYPE_T pmu_pkg_type = PMU_PACKAGE_TYPE_IHC_V0;

#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
#ifdef ARM_CMSE
static bool BOOT_BSS_LOC pmu_ns_running;
#endif
static void pmu_hppa_set_volt(uint16_t volt);
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

#ifdef PMU_NTC_MONITOR
static struct PMU_NTC_MONITOR_CTX_T ntc_monitor_ctx[PMU_NTC_USER_QTY];
static struct PMU_NTC_CTX_T ntc_ctx;
static bool ntc_irq_busy = false;
static void pmu_ntc_monitor_init(void);
#ifdef NTC_MONITOR_USE_OSTIMER
static void pmu_ntc_monitor_ostimer_handler(void const *param);
osTimerDef(ntc_monitor_ostimer, pmu_ntc_monitor_ostimer_handler);
static osTimerId ntc_monitor_ostimer_id = NULL;
#else
static HWTIMER_ID ntc_monitor_timer = NULL;
#endif
#endif

static HWTIMER_ID charger_1622_timer = NULL;

static HWTIMER_ID charger_1622_timer_2 = NULL;

#if defined(_AUTO_TEST_)
static bool at_skip_shutdown = false;

void pmu_at_skip_shutdown(bool enable)
{
    at_skip_shutdown = enable;
}
#endif

#ifdef RTC_CALENDAR
#define RTC_REG_VAL2CALENDAR(TENS, UNITS)                     ((TENS) * 10 + (UNITS))
#define RTC_CALENDAR2REG_VAL_TENS(RTC_CALENDAR)               ((RTC_CALENDAR) / 10 % 10)
#define RTC_CALENDAR2REG_VAL_UNITS(RTC_CALENDAR)              ((RTC_CALENDAR) / 1 % 10)

#define RTC_CALENDAR_LOAD_TIME_STABLE_TIME_US                 (100)
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
        pmu_write(PMU_REG_INT_CLR, RTC_INT_0);
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

    pmu_write(PMU_REG_INT_CLR, RTC_INT_0);

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
    alarm_get->year = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.year_tens, rtc_reg_fmt.year_units) + RTC_CALENDAR_YEAR_REF;

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
    hal_sys_timer_delay_us(RTC_CALENDAR_LOAD_TIME_STABLE_TIME_US);

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
    calendar_get->year = RTC_REG_VAL2CALENDAR(rtc_reg_fmt.year_tens, rtc_reg_fmt.year_units) + RTC_CALENDAR_YEAR_REF;

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
static int8_t PMU_OPEN_BSS_LOC pmu_ldo_usb_comp = 0;

#ifdef PMU_ALREADY_INIT
BOOT_TEXT_FLASH_LOC
#endif
void pmu_get_ldo_dig_calib_value(void)
{
    // TODO:
    return;
}
#endif

#ifdef PMU_DCDC_CALIB
static int16_t PMU_OPEN_BSS_LOC pmu_dcdc_dig_comp = 0;
static int8_t PMU_OPEN_BSS_LOC pmu_dcdc_ana_comp = 0;
static int8_t PMU_OPEN_BSS_LOC pmu_dcdc_hppa_comp = 0;

#ifdef PMU_ALREADY_INIT
BOOT_TEXT_FLASH_LOC
#endif
static POSSIBLY_UNUSED void pmu_get_dcdc_calib_value(void)
{
    uint16_t val;

#ifdef MTEST_ENABLED
    pmu_get_efuse(PMU_EFUSE_PAGE_VBG_VRTC_CALIB, &val);

    intf_calib_en = !!(val & PMU_EFUSE_VBG_PMU_INTF_CALIB_ENABLE);

    TRACE(0, "%s: intf_calib_en:%d", __func__, intf_calib_en);
#endif

    if (intf_calib_en) {
        pmu_get_efuse(PMU_EFUSE_PAGE_DCDC_VOLT_CALIB, &val);
    } else {
        pmu_read(REG_DCDC_CALIB_MTEST, &val);
    }

    pmu_dcdc_dig_comp = GET_BITFIELD(val, PMU_EFUSE_DCDC1_VOLT_COMP);
    if (val & PMU_EFUSE_DCDC1_VOLT_COMP_SIGN) {
        pmu_dcdc_dig_comp = -pmu_dcdc_dig_comp;
    }

    pmu_dcdc_ana_comp = GET_BITFIELD(val, PMU_EFUSE_DCDC2_VOLT_COMP);
    if (val & PMU_EFUSE_DCDC2_VOLT_COMP_SIGN) {
        pmu_dcdc_ana_comp = -pmu_dcdc_ana_comp;
    }

    pmu_dcdc_hppa_comp = GET_BITFIELD(val, PMU_EFUSE_DCDC3_VOLT_COMP);
    if (val & PMU_EFUSE_DCDC3_VOLT_COMP_SIGN) {
        pmu_dcdc_hppa_comp = -pmu_dcdc_hppa_comp;
    }
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

    return metal_id;
}

static void BOOT_TEXT_FLASH_LOC pmu_boot_cause_init(void)
{
    uint16_t val = 0;
    uint16_t val_reason = 0;

    // ROM will execute global rst and open dig wdt by default,
    // so first look at the reason of pmu and then look at the reason of dig
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

    if (pmu_boot_reason == PMU_BOOT_CAUSE_NULL) {
        union HAL_HW_BOOTMODE_T hw_bm = hal_hw_bootmode_get();

        if (hw_bm.watchdog) {
            pmu_boot_reason = PMU_BOOT_CAUSE_DIG_WDT;
        } else if (hw_bm.global) {
            pmu_boot_reason = PMU_BOOT_CAUSE_DIG_REBOOT;
        }
#if !(defined(OTA_BOOT_IMAGE) || defined(ARM_CMSE))
    } else {
        val = ABORT_SOURCE_CLR | PMU_LDO_ON_SOURCE_CLR;
        pmu_write(PMU_REG_BOOT_SOURCE, val);
        // One lpo clock
        hal_sys_timer_delay_us(30);
        pmu_write(PMU_REG_BOOT_SOURCE, val);
#endif
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

#if !defined(FPGA) && !defined(PROGRAMMER)
    pmu_charger_save_context();
#endif

#ifndef PMU_ALREADY_INIT
    // Reset PMU (to recover from a possible insane state, e.g., ESD reset)
    pmu_write(PMU_REG_METAL_ID, 0xCAFE);
    pmu_write(PMU_REG_METAL_ID, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));

    pmu_read(PMU_REG_BUCK_VCORE_EN, &val);
    val &= ~REG_BUCK_VCORE_RAMP_EN;
    pmu_write(PMU_REG_BUCK_VCORE_EN, val);
#endif

    pmu_read(PMU_REG_CHARGER_CFG, &val);
    val |= REG_CHARGE_INTR_EN;
    pmu_write(PMU_REG_CHARGER_CFG, val);

#if defined(PMU_CLK_USE_EXT_CRYSTAL) || defined(RTC_CLK_USE_EXT_CRYSTAL)
    pmu_external_crystal_init();
#else
    pmu_read(PMU_REG_XO_32K_CFG, &val);
    val &= ~(REG_XO_32K_PU_LDO | REG_XO_32K_PU_LDO_DR);
    pmu_write(PMU_REG_XO_32K_CFG, val);
#endif

#ifndef __WATCHER_DOG_RESET__
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

void BOOT_TEXT_FLASH_LOC bbpll_freq_pll_config(uint32_t freq)
{
    uint64_t PLL_cfg_val;
    uint32_t crystal = hal_cmu_get_crystal_freq();

    PLL_cfg_val = ((uint64_t)PMU_RF_BBPLL_FREQWORD_SHIFT * freq + crystal / 2) / crystal;

    bbpll_pll_update(PLL_cfg_val, false);
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

__STATIC_FORCEINLINE void bbpll_set_spd(uint32_t spd)
{
    uint16_t val;

    rf_read(RF_REG_535, &val);
    val = SET_BITFIELD(val, REG_LP_BBPLL_VCO_SPD, spd);
    rf_write(RF_REG_535, val);
}

__STATIC_FORCEINLINE uint16_t bbpll_get_spd(uint64_t PLL_cfg_val)
{
    uint16_t bbpll_spd_val = 0x4;
    bool found = true;

    if (0x179DB22D <= PLL_cfg_val && PLL_cfg_val <= 0x18624DD3) {
        // 288M � 1.6%
        bbpll_spd_val = 0x1;
    } else if (0x1D9A1CAC <= PLL_cfg_val && PLL_cfg_val <= 0x214624DD) {
        // 361M - 1.6% ~ 393M + 1.6%
        bbpll_spd_val = 0x4;
    } else {
        found = false;
    }

    if (!found) {
        uint32_t freq_mhz, crystal;

        crystal = hal_cmu_get_crystal_freq();
        freq_mhz = ((PLL_cfg_val * crystal - crystal / 2) / (1 << 25)  + (1000000 / 2)) / 1000000;

        ASSERT(0, "[%s] freq_mhz:%d is not verified by FT", __func__, freq_mhz);
    }

    return bbpll_spd_val;
}

void BOOT_TEXT_FLASH_LOC bbpll_pll_update(uint64_t pll_cfg_val, bool calib)
{
    uint16_t v[3];
    uint16_t val;

    v[0] = pll_cfg_val & 0xFFFF;
    v[1] = (pll_cfg_val >> 16) & 0xFFFF;
    v[2] = (pll_cfg_val >> 32) & 0xFFFF;

    val = v[0];
    rf_write(RF_REG_52F, val);

    val = v[1];
    rf_write(RF_REG_530, val);

    val = v[2];
    rf_write(RF_REG_531, val);

    rf_read(RF_REG_52D, &val);
    val |= REG_LP_BBPLL_FREQ_EN_COMB;
    rf_write(RF_REG_52D, val);

    val = bbpll_get_spd(pll_cfg_val);
    bbpll_set_spd(val);
}

void bbpll_set_codec_div(uint32_t div)
{
    uint16_t val;

    rf_read(RF_REG_537, &val);
    val = SET_BITFIELD(val, REG_LP_BBPLL_DIVN_AD, div);
    rf_write(RF_REG_537, val);
}

static void POSSIBLY_UNUSED bbpll_auto_calib_spd(uint32_t codec_freq, uint32_t div)
{
    // TODO:
    // Set FT calibration values in pmu_rf_ana_inti();
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
        TRACE(0, "Valid spd=[%2u, %2u] cnt=%2u sel=%2u", valid_start, valid_end, valid_cnt, sel);
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
    return 0;
}

void BOOT_TEXT_FLASH_LOC pmu_rc_2m_enable(void)
{
}

#define CRYSTAL                     (24000000)
#ifdef  BBPLL_MHZ
#define BBPLL_FREQ                  (BBPLL_MHZ * 1000000)
#else
#define CODEC_FREQ                  (48000 * 512)
#define PLL_DIV                     (16)
#define BBPLL_FREQ                  (CODEC_FREQ * PLL_DIV)
#endif
void BOOT_TEXT_FLASH_LOC pmu_rf_ana_init(void)
{
#if defined(PMU_FULL_INIT) || (!defined(FPGA) && !defined(PROGRAMMER) && !defined(PMU_ALREADY_INIT))
    uint16_t val;

    // Reset RF
    rf_write(RF_REG_00, 0xCAFE);
    rf_write(RF_REG_00, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));

    // Reset ANA
    ana_write(ANA_REG_60, 0xCAFE);
    ana_write(ANA_REG_60, 0x5FEE);
    hal_sys_timer_delay(US_TO_TICKS(500));

    // Reset CAP
    ana_read(ANA_REG_51, &val);
    val &= ~(SOFT_RESETN_CAPSENSOR | SOFT_RESETN_CAPSENSOR_REG);
    ana_write(ANA_REG_51, val);
    hal_sys_timer_delay(US_TO_TICKS(500));

    // Default BBPLL freq 296m
    uint16_t mdll_t1_us;

    rf_read(RF_REG_537, &val);
#if defined(RC_CLK_ENABLE) || (defined(PSRAM_ENABLE) && (PSRAM_SPEED >= 200)) || (defined(AUD_PLL_DOUBLE))
    val &= ~REG_LP_BBPLL_DIG_CLK_FREQ_SEL; // div=0
#else
    val |= REG_LP_BBPLL_DIG_CLK_FREQ_SEL; // div=2
#endif
    rf_write(RF_REG_537, val);

#ifdef ANALOG_XTAL_ALWAYS_ON
    rf_read(RF_REG_527, &val);
    val |= REG_BT_BG_PU_DR | REG_BT_BG_PU;
    rf_write(RF_REG_527, val);

    rf_read(RF_REG_50F, &val);
    val |= REG_BT_XTAL_PU_DR | REG_BT_XTAL_PU;
    rf_write(RF_REG_50F, val);
#endif

    mdll_t1_us = hal_cmu_get_oscx4_locked_time_us();

    if (mdll_t1_us >= 10) {
        // intf delay: reg_xtal_mdll_timing_t1
        // analog_delay: 10
        rf_read(RF_REG_521, &val);
        val= SET_BITFIELD(val, REG_XTAL_MDLL_TIMING_T1, (mdll_t1_us - 10));
        rf_write(RF_REG_521, val);
    }

    // intf reduces pu_osc=0 processing completion time to 2 lpo cycles.(3 -> 2)
    rf_read(RF_REG_526, &val);
    val &= ~PU_OSC_DLY_DR;
    rf_write(RF_REG_526, val);

    // modify xtal drv 001->010
    uint16_t swrc_i_val;
    swrc_i_val = 0x2;

    rf_read(RF_REG_517, &val);
    val = SET_BITFIELD(val, REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH22, swrc_i_val);
    val = SET_BITFIELD(val, REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH46, swrc_i_val);
    val = SET_BITFIELD(val, REG_XTAL_MDLL_OUTBUFFER_SWRC_I_CH70, swrc_i_val);
    val = SET_BITFIELD(val, REG_XTAL_MDLL_OUTBUFFER_SWRC_I_OTHER, swrc_i_val);
    rf_write(RF_REG_517, val);

    rf_read(RF_REG_544, &val);
    val = SET_BITFIELD(val, REG_XTAL_MDLL_DLL_SWRC, swrc_i_val);
    rf_write(RF_REG_544, val);

    // Add 1us delay for mdll calib flag ready
    rf_read(RF_REG_524, &val);
    val = SET_BITFIELD(val, REG_XTAL_MDLL_CAL_WAIT_TIME, 0x10);
    rf_write(RF_REG_524, val);

#if !defined(OSC_26M_X4_AUD2BB) || (defined(PSRAM_ENABLE) && (PSRAM_SPEED == 200))
    uint64_t PLL_cfg_val;

    PLL_cfg_val = ((uint64_t)PMU_RF_BBPLL_FREQWORD_SHIFT * BBPLL_FREQ + CRYSTAL / 2) / CRYSTAL;
    bbpll_pll_update(PLL_cfg_val, true);
#endif
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
    pmu_sys_ctrl_work = true;
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    // Default vcore might not be high enough to support high performance mode
    pmu_high_performance_mode_enable(false);
#endif
    hal_cmu_sys_set_freq(HAL_CMU_FREQ_26M);
    pmu_vcore_req |= PMU_VCORE_PMU_RESET;
#ifdef AUD_PLL_DOUBLE
    // Restore the default div
    rf_read(RF_REG_537, &val);
    val |= REG_LP_BBPLL_DIG_CLK_FREQ_SEL;
    rf_write(RF_REG_537, val);
#endif
#ifdef SPLIT_CORE_M_CORE_L
    pmu_read(PMU_REG_CORE_L_CFG2, &val);
    val |= REG_LDO_VCORE_L_BYPASS;
    pmu_write(PMU_REG_CORE_L_CFG2, val);
    hal_sys_timer_delay(MS_TO_TICKS(1));
#endif

    pmu_hppa_set_volt(PMU_DCDC_HPPA_DEFAULT);
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
#ifndef PMU_CLK_USE_EXT_CRYSTAL
    if (shutdown) {
        uint32_t old_systick = CONFIG_SYSTICK_HZ;

        pmu_read(PMU_REG_LP_RTC_CFG, &val);
        val |= LP_MODE_RTC_REG | LP_MODE_RTC_DR;
        pmu_write(PMU_REG_LP_RTC_CFG, val);

        hal_sys_timer_delay_us(50);

        hal_sys_timer_calib();

        val &= ~(LP_MODE_RTC_REG | LP_MODE_RTC_DR);
        pmu_write(PMU_REG_LP_RTC_CFG, val);

        hal_sys_timer_delay_us(50);

        PMU_INFO_TRACE_IMM(0, "%s: Calib lp lpo freq done! freq(act/lp)=%u/%u",
                __func__, old_systick << 1, CONFIG_SYSTICK_HZ << 1);
    }
#endif
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
#if defined(RTC_CALENDAR) || defined(PMU_CLK_USE_EXT_CRYSTAL) || defined(RTC_CLK_USE_EXT_CRYSTAL)
            // ac_on will en lpo
            // wdt_rst will reset 32k_sel and ac_out_pwron cfg
            true ||
#endif
            false) {
            pmu_wdt_stop();
        } else {
            pmu_wdt_config(500, 500);
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

        // Power off
        pmu_read(PMU_REG_POWER_OFF,&val);
        val |= SOFT_POWER_OFF;
        for (int i = 0; i < 100; i++) {
            pmu_write(PMU_REG_POWER_OFF,val);
            hal_sys_timer_delay(MS_TO_TICKS(5));
        }

        hal_sys_timer_delay(MS_TO_TICKS(50));

        //can't reach here
        PMU_INFO_TRACE_IMM(0, "\nError: pmu shutdown failed!\n");
        hal_sys_timer_delay(MS_TO_TICKS(5));

        pmu_read(PMU_REG_POWER_OFF, &val);
        if (val & SOFT_POWER_OFF) {
            PMU_INFO_TRACE_IMM(0, "\nError: SOFT_POWER_OFF=1!\n");
        }
        pmu_wdt_reboot();
        while (1) {}
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
    return PMU_REG_LDO_USB_CFG;
}

static inline uint16_t pmu_get_module_volt_addr(enum PMU_MODULE_T module)
{
    return PMU_REG_LDO_USB_VOLT;
}

void pmu_module_config(enum PMU_MODULE_T module,unsigned short is_manual,unsigned short ldo_on,unsigned short lp_mode,unsigned short dpmode)
{
    unsigned short val;
    unsigned short module_address;
    const struct PMU_MODULE_CFG_T *module_cfg_p = &pmu_module_cfg[module];

    module_address = pmu_get_module_addr(module);

    if (module == PMU_USB && (ldo_on == PMU_LDO_OFF || dpmode == PMU_DSLEEP_MODE_OFF) && pmu_vusb_req & PMU_VUSB_REQ_GPIO) {
        return;
    }

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

    val |= module_cfg_p->lp_en_dr | module_cfg_p->lp_en;

    if (dpmode) {
        val |= module_cfg_p->dsleep_mode;
    } else {
        val &= ~module_cfg_p->dsleep_mode;
    }
    pmu_write(module_address, val);
}

void pmu_module_set_volt(unsigned char module, unsigned short sleep_v, unsigned short normal_v)
{
    unsigned short val;
    unsigned short module_volt_address;
    unsigned short rc_v = normal_v;
    const struct PMU_MODULE_CFG_T *module_cfg_p = &pmu_module_cfg[module];

#ifdef PMU_LDO_VCORE_CALIB
    normal_v = pmu_reg_val_add(normal_v, pmu_ldo_usb_comp, MAX_LDO_VUSB_BIT_VAL);
    sleep_v = pmu_reg_val_add(sleep_v, pmu_ldo_usb_comp, MAX_LDO_VUSB_BIT_VAL);
    rc_v = pmu_reg_val_add(rc_v, pmu_ldo_usb_comp, MAX_LDO_VUSB_BIT_VAL);
#endif

    module_volt_address = pmu_get_module_volt_addr(module);

    pmu_read(module_volt_address, &val);
    val &= ~module_cfg_p->normal_v;
    val |= (normal_v << module_cfg_p->normal_v_shift) & module_cfg_p->normal_v;
    pmu_write(module_volt_address, val);

    pmu_read(PMU_REG_LDO_USB_LP_RC_VOLT, &val);
    val &= ~module_cfg_p->dsleep_v;
    val |= (sleep_v << module_cfg_p->dsleep_v_shift) & module_cfg_p->dsleep_v;
    val &= ~module_cfg_p->rc_v;
    val |= (rc_v << module_cfg_p->rc_v_shift) & module_cfg_p->rc_v;
    pmu_write(PMU_REG_LDO_USB_LP_RC_VOLT, val);
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

    pmu_read(PMU_REG_LDO_USB_LP_RC_VOLT, &val);
    if (sleep_vp) {
        *sleep_vp = (val & module_cfg_p->dsleep_v) >> module_cfg_p->dsleep_v_shift;
    }

#ifdef PMU_LDO_VCORE_CALIB
    if (normal_vp) {
        *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_ldo_usb_comp, MAX_LDO_VUSB_BIT_VAL);
    }
    if (sleep_vp) {
        *sleep_vp = pmu_reg_val_add(*sleep_vp, -pmu_ldo_usb_comp, MAX_LDO_VUSB_BIT_VAL);
    }
#endif

    return 0;
}

static void POSSIBLY_UNUSED pmu_module_ramp_volt(unsigned char module, unsigned short sleep_v, unsigned short normal_v)
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
#ifndef PMU_INTF_CALIBED_VOLT
    if (!intf_calib_en) {
        if (normal_vp) {
            *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_dcdc_ana_comp, MAX_BUCK_VANA_BIT_VAL);
        }
    }
#endif
    if (dsleep_vp) {
        *dsleep_vp = pmu_reg_val_add(*dsleep_vp, -pmu_dcdc_ana_comp, MAX_BUCK_VANA_BIT_VAL);
    }
#endif
}

static void pmu_dcdc_ana_set_volt(unsigned short normal_v,unsigned short dsleep_v)
{
    unsigned short val;
    unsigned short rc_v = normal_v;

#ifdef PMU_DCDC_CALIB
#ifndef PMU_INTF_CALIBED_VOLT
    if (!intf_calib_en) {
        normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_ana_comp, MAX_BUCK_VANA_BIT_VAL);
    }
#endif
    dsleep_v = pmu_reg_val_add(dsleep_v, pmu_dcdc_ana_comp, MAX_BUCK_VANA_BIT_VAL);
    rc_v = pmu_reg_val_add(rc_v, pmu_dcdc_ana_comp, MAX_BUCK_VANA_BIT_VAL);
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
#ifdef PMU_DCDC_CALIB
        if (intf_calib_en) {
            // get real vbit
            normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_ana_comp, MAX_BUCK_VANA_BIT_VAL);
        }
#endif
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

    if (pmu_sys_ctrl_work) {
        new_act_dcdc = PMU_DCDC_ANA_DEFAULT;
    } else {
        new_act_dcdc = ana_act_dcdc;
    }

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
                delay_us = (new_act_dcdc - old_act_dcdc) / PMU_DCDC_HW_RAMP_1_CLK_STEP_HALF * PMU_DCDC_HW_RAMP_1_CLK_TIME_US + 100;
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
#ifndef PMU_INTF_CALIBED_VOLT
    if (!intf_calib_en) {
        if (normal_vp) {
            *normal_vp = pmu_reg_val_add(*normal_vp, -pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
        }
    }
#endif
    if (dsleep_vp) {
        *dsleep_vp = pmu_reg_val_add(*dsleep_vp, -pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
    }
#endif
}

static void pmu_dcdc_dig_set_volt(unsigned short normal_v, unsigned short dsleep_v, bool set_sleep_v)
{
    unsigned short val;
    unsigned short rc_v = normal_v;

#ifdef PMU_DCDC_CALIB
#ifndef PMU_INTF_CALIBED_VOLT
    if (!intf_calib_en) {
        normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
    }
#endif
    dsleep_v = pmu_reg_val_add(dsleep_v, pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
    rc_v = pmu_reg_val_add(rc_v, pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
#endif

    pmu_read(PMU_REG_BUCK_VCORE_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_VCORE_VBIT_NORMAL, normal_v);
    pmu_write(PMU_REG_BUCK_VCORE_CFG, val);

#if defined(RC_CLK_ENABLE) && !defined(PMU_VCORE_RC_MODE_FIXED)
    pmu_read(PMU_REG_BUCK_VCORE_RC_CFG, &val);
    val = SET_BITFIELD(val, REG_BUCK_VCORE_VBIT_RC, rc_v);
    pmu_write(PMU_REG_BUCK_VCORE_RC_CFG, val);
#endif

    if (set_sleep_v) {
        pmu_read(PMU_REG_BUCK_VCORE_LP_CFG, &val);
        val = SET_BITFIELD(val, REG_BUCK_VCORE_VBIT_DSLEEP, dsleep_v);
        pmu_write(PMU_REG_BUCK_VCORE_LP_CFG, val);
    }

    if (!dcdc_ramp_en) {
#ifdef PMU_DCDC_CALIB
        if (intf_calib_en) {
            // get real vbit
            normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_dig_comp, MAX_BUCK_VCORE_BIT_VAL);
        }
#endif
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
    const static uint16_t dcdc_volt_list[][2] = {
        {1100, PMU_DCDC_DIG_1_1V },
        {1050, PMU_DCDC_DIG_1_05V},
        {1000, PMU_DCDC_DIG_1_0V },
        {950,  PMU_DCDC_DIG_0_95V},
        {900,  PMU_DCDC_DIG_0_9V },
        {850,  PMU_DCDC_DIG_0_85V},
        {800,  PMU_DCDC_DIG_0_8V },
        {750,  PMU_DCDC_DIG_0_75V},
        {700,  PMU_DCDC_DIG_0_7V },
    };

    uint16_t dcdc_volt;
    uint32_t i, find = 0;
    bool success = false;

    for (i = 0; i < ARRAY_SIZE(dcdc_volt_list); i++) {
        if (dcdc_volt_mv == dcdc_volt_list[i][0]) {
            dcdc_volt = dcdc_volt_list[i][1];
            find |= 0x1;
            break;
        }
    }
    if (find == 0x1) {
        pmu_dcdc_dig_set_volt(dcdc_volt, dig_lp_dcdc, true);
        success = true;
    }
    return success;
}
#endif /* MTEST_ENABLED */

static void BOOT_TEXT_SRAM_LOC pmu_dig_get_target_volt(uint16_t *dcdc)
{
    uint16_t dcdc_volt;

    if (0) {
    } else if (pmu_vcore_req & PMU_VCORE_PMU_RESET) {
        dcdc_volt = PMU_DCDC_DIG_DEFAULT;
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    } else if (pmu_vcore_req & (PMU_VCORE_SYS_FREQ_HIGH)) {
        if (high_perf_freq_mhz <= 260) {
            dcdc_volt = PMU_DCDC_DIG_1_05V;
        } else {
            dcdc_volt = PMU_DCDC_DIG_1_1V;
        }
#endif
#if defined(AUD_PLL_DOUBLE)
    } else if (pmu_vcore_req & (PMU_VCORE_SYS_FREQ_HIGH)) {
        dcdc_volt = PMU_DCDC_DIG_1_0V;
#endif
    } else if (pmu_vcore_req & PMU_VCORE_PSRAM_FREQ_HIGH) {
        dcdc_volt = PMU_DCDC_DIG_0_9V;
    } else if (pmu_vcore_req & (PMU_VCORE_RS_FREQ_HIGH | PMU_VCORE_SYS_FREQ_MEDIUM)) {
        dcdc_volt = PMU_DCDC_DIG_0_85V;
    } else if (pmu_vcore_req & PMU_VCORE_PSRAM_FREQ_MEDIUM) {
        dcdc_volt = PMU_DCDC_DIG_0_8V;
    } else {
        // Common cases
        dcdc_volt = PMU_DCDC_DIG_0_78V;
    }

#ifdef PMU_VCORE_RISE_MV
    dcdc_volt += (PMU_DCDC_DIG_0_8V - PMU_DCDC_DIG_0_75V) * (PMU_VCORE_RISE_MV / 25) / 2;
#endif

#if (defined(PROGRAMMER) || defined(__BES_OTA_MODE__)) && !defined(PMU_FULL_INIT)
    // Try to keep the same vcore voltage as ROM (hardware default)
    if (dcdc_volt < PMU_DCDC_DIG_DEFAULT) {
        dcdc_volt = PMU_DCDC_DIG_DEFAULT;
    }
#endif

    if (dcdc) {
        *dcdc = dcdc_volt;
    }
}

static void pmu_dig_set_volt(enum PMU_POWER_MODE_T mode)
{
    uint32_t lock;
    uint32_t delay_us;
    uint16_t dcdc_volt, old_act_dcdc, old_lp_dcdc;
    bool volt_inc = false;
    const uint8_t ramp_step = 12;

    lock = int_lock();

    if (mode == PMU_POWER_MODE_NONE) {
        mode = pmu_power_mode;
    }

    pmu_dig_get_target_volt(&dcdc_volt);
    pmu_dcdc_dig_get_volt(&old_act_dcdc, &old_lp_dcdc);

    if (mode == PMU_POWER_MODE_DIG_DCDC) {
        if (old_act_dcdc < dcdc_volt) {
            volt_inc = true;
            if (dcdc_ramp_en) {
                pmu_dcdc_dig_set_volt(dcdc_volt, dig_lp_dcdc, false);
            } else {
                while (old_act_dcdc < dcdc_volt) {
                    if (old_act_dcdc + ramp_step < dcdc_volt) {
                        old_act_dcdc += ramp_step;
                    } else {
                        old_act_dcdc = dcdc_volt;
                    }
                    pmu_dcdc_dig_set_volt(old_act_dcdc, dig_lp_dcdc, false);
                }
            }
        } else {
            pmu_dcdc_dig_set_volt(dcdc_volt, dig_lp_dcdc, false);
        }
    }

    if (volt_inc) {
        if (dcdc_ramp_en) {
            delay_us = (dcdc_volt - old_act_dcdc) / PMU_DCDC_HW_RAMP_1_CLK_STEP * PMU_DCDC_HW_RAMP_1_CLK_TIME_US + 100;
            hal_sys_timer_delay_us(delay_us);
        } else {
            hal_sys_timer_delay_us(PMU_VCORE_STABLE_TIME_US);
        }
    }

    int_unlock(lock);
}

static void POSSIBLY_UNUSED pmu_ana_reserved_en(bool en)
{
}

static void POSSIBLY_UNUSED pmu_ldo_mode_en(void)
{
}

static void POSSIBLY_UNUSED pmu_dcdc_ana_mode_en(void)
{
}

static void POSSIBLY_UNUSED pmu_dcdc_dual_mode_en(void)
{
}

void pmu_mode_change(enum PMU_POWER_MODE_T mode)
{
    uint32_t lock;

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
    } else if (mv == 1950) {
        val = PMU_DCDC_HPPA_1_95V;
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
        *dsleep_vp = pmu_reg_val_add(*dsleep_vp, -pmu_dcdc_hppa_comp, MAX_BUCK_VHPPA_BIT_VAL);
    }
#endif
}

static void pmu_dcdc_hppa_set_volt(unsigned short normal_v,unsigned short dsleep_v)
{
    unsigned short val;
    unsigned short rc_v = normal_v;

#ifdef PMU_DCDC_CALIB
    normal_v = pmu_reg_val_add(normal_v, pmu_dcdc_hppa_comp, MAX_BUCK_VHPPA_BIT_VAL);
    dsleep_v = pmu_reg_val_add(dsleep_v, pmu_dcdc_hppa_comp, MAX_BUCK_VHPPA_BIT_VAL);
    rc_v = pmu_reg_val_add(dsleep_v, pmu_dcdc_hppa_comp, MAX_BUCK_VHPPA_BIT_VAL);
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

static void POSSIBLY_UNUSED pmu_dcdc_hppa_en(int enable)
{
}

static void POSSIBLY_UNUSED pmu_hppa_dcdc_to_ldo(void)
{
}

static void pmu_hppa_set_volt(uint16_t volt)
{
#ifndef HPPA_LDO_ON
    uint16_t new_act_dcdc;
    uint16_t old_act_dcdc;
    uint16_t old_lp_dcdc;

    new_act_dcdc = volt;

    pmu_dcdc_hppa_get_volt(&old_act_dcdc, &old_lp_dcdc);
    if (old_act_dcdc < new_act_dcdc) {
        if (dcdc_ramp_en) {
            pmu_dcdc_hppa_set_volt(new_act_dcdc, new_act_dcdc);
        } else {
            while (old_act_dcdc++ < new_act_dcdc) {
                pmu_dcdc_hppa_set_volt(old_act_dcdc, new_act_dcdc);
            }
        }
        if (dcdc_ramp_en) {
            uint32_t delay_us;

            delay_us = (new_act_dcdc - old_act_dcdc) / PMU_DCDC_HW_RAMP_1_CLK_STEP_HALF * PMU_DCDC_HW_RAMP_1_CLK_TIME_US + 100;
            hal_sys_timer_delay_us(delay_us);
        } else {
            hal_sys_timer_delay_us(PMU_VANA_STABLE_TIME_US);
        }
    } else if (old_act_dcdc != new_act_dcdc || old_lp_dcdc != new_act_dcdc) {
        pmu_dcdc_hppa_set_volt(new_act_dcdc, new_act_dcdc);
    }
#endif
}
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
    uint16_t tgt_volt;
    uint16_t old_volt;
    uint16_t val;

    pmu_dig_get_target_volt(&tgt_volt);

    pmu_read(PMU_REG_BUCK_VCORE_CFG, &val);
    old_volt = GET_BITFIELD(val, REG_BUCK_VCORE_VBIT_NORMAL);

    if (old_volt < tgt_volt) {
        while (old_volt++ < tgt_volt) {
            val = SET_BITFIELD(val, REG_BUCK_VCORE_VBIT_NORMAL, old_volt);
            pmu_write(PMU_REG_BUCK_VCORE_CFG, val);
        }
        hal_sys_timer_delay_us(PMU_VCORE_STABLE_TIME_US);
    }
}

void pmu_reg_val_print(void)
{
    uint16_t val;

    TRACE(0, "%s start", __func__);
    TRACE(0, "dump pmu regs : reg=val");
    for (uint16_t reg = 0x00; reg <= 0xFE; reg++) {
        pmu_read(reg, &val);
        TRACE_IMM(0, "0x%04X=0x%04X", reg, val);
    }

    for (uint16_t reg = 0x10A; reg <= 0x15F; reg++) {
        pmu_read(reg, &val);
        TRACE_IMM(0, "0x%04X=0x%04X", reg, val);
    }

    TRACE(0, "dump efuse regs : reg=val");
    for (uint16_t reg = PMU_EFUSE_PAGE_SECURITY; reg <= PMU_EFUSE_PAGE_RF_TX_PWR; reg++) {
        pmu_get_efuse(reg, &val);
        TRACE_IMM(0, "0x%04X=0x%04X", reg, val);
    }
    TRACE(0, "%s done", __func__);
}

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
    // TODO:
}
#endif

#ifdef DCDC_VOLT_FAST_HW_RAMP
static void pmu_buck_volt_hw_ramp_step_set(uint8_t step)
{
    uint16_t val;

    if (dcdc_ramp_map & REG_DCDC1_RAMP_EN) {
        pmu_read(PMU_REG_BUCK_VCORE_EN, &val);
        val = SET_BITFIELD(val, REG_BUCK_VCORE_RAMP_STEP, step);
        pmu_write(PMU_REG_BUCK_VCORE_EN, val);
    }

    step /= 2;

    if (dcdc_ramp_map & REG_DCDC2_RAMP_EN) {
        pmu_read(PMU_REG_BUCK_VANA_EN, &val);
        val = SET_BITFIELD(val, REG_BUCK_VANA_RAMP_STEP, step);
        pmu_write(PMU_REG_BUCK_VANA_EN, val);
    }

    if (dcdc_ramp_map & REG_DCDC3_RAMP_EN) {
        pmu_read(PMU_REG_BUCK_VHPPA_EN, &val);
        val = SET_BITFIELD(val, REG_BUCK_VHPPA_RAMP_STEP, step);
        pmu_write(PMU_REG_BUCK_VHPPA_EN, val);
    }
}
#endif

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
}

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

#if defined(ARM_CMSE) || defined(ARM_CMNS)
uint32_t TZ_pmu_get_vcore_req_S(void);

#ifdef ARM_CMSE
CMSE_API
uint32_t TZ_pmu_get_vcore_req_S(void)
{
#if defined(PMU_INIT) || (!defined(FPGA) && !defined(PROGRAMMER))
    pmu_ns_running = true;
#endif
    return pmu_vcore_req;
}
#endif
#endif

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

#ifdef ARM_CMNS
    pmu_vcore_req = TZ_pmu_get_vcore_req_S();
#endif

    ASSERT(vcodec_mv == 1400 || vcodec_mv == 1450 || vcodec_mv == 1500 || vcodec_mv == 1550 || vcodec_mv == 1600 || vcodec_mv == 1650 ||
        vcodec_mv == 1700 || vcodec_mv == 1750 || vcodec_mv == 1800 || vcodec_mv == 1900 || vcodec_mv == 1950,
        "Invalid vcodec cfg: vcodec_mv=%u", vcodec_mv);
    ASSERT(vhppa_mv == 1400 || vhppa_mv == 1450 || vhppa_mv == 1500 || vhppa_mv == 1550 || vhppa_mv == 1600 || vhppa_mv == 1650 ||
        vhppa_mv == 1700 || vhppa_mv == 1750 || vhppa_mv == 1800 || vhppa_mv == 1900 || vhppa_mv == 1950,
        "Invalid vhppa cfg: vhppa_mv=%u", vhppa_mv);
    TRACE(0, "%s vcodec_mv:%d vhppa_mv:%d", __func__, vcodec_mv, vhppa_mv);

#if !defined(ROM_BUILD) && defined(PMU_EFUSE_INIT_IN_BOOT)
    pmu_efuse_init();
#endif

    pmu_get_efuse(PMU_EFUSE_PAGE_CP_WAFER_XY_CTX, &val);
    pmu_pkg_type = GET_BITFIELD(val, PMU_EFUSE_PACKAGE_TYPE);
    TRACE(0, "%s pmu_pkg_type:%d", __func__, pmu_pkg_type);

#ifdef PMU_WAFER_PRINT
    pmu_get_efuse(REG_CP_WAFER_CTX_ID, &val);
    TRACE(0, "wafer_ctx_print: wafer_ID=%d", GET_BITFIELD(val, PMU_EFUSE_CP_WAFER_CTX_ID));

    pmu_get_efuse(REG_CP_WAFER_CTX_XY, &val);
    TRACE(0, "wafer_ctx_print: wafer_X=%d, wafer_Y=%d",
        GET_BITFIELD(val, PMU_EFUSE_CP_WAFER_CTX_X), GET_BITFIELD(val, PMU_EFUSE_CP_WAFER_CTX_Y));
#endif

    TRACE(0, "%s boot_cause=0x%04X first_pwron:%d", __func__, pmu_boot_reason, pmu_boot_first_pwr_up);

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

    // Disable sar vref output
    pmu_sar_adc_vref_sw_pu(false);
    pmu_set_sar_adc_vref();

    // Set sar adc config
    // reg_sar_adc_offset[15:0]
    pmu_write(PMU_REG_SAR_ADC_OFFSET_CFG, 0x7C01);

    pmu_read(PMU_REG_SAR_CFG_8C, &val);
    val |= REG_SAR_WEIGHT_DR | REG_SAR_ADC_OFFSET_DR;
    pmu_write(PMU_REG_SAR_CFG_8C, val);

    pmu_read(PMU_REG_SAR_CFG_91, &val);
    val |= REG_SAR_DONE_DLY_SEL;
    val = SET_BITFIELD(val, REG_SAR_THERM_GAIN_IN, 0x2);
    pmu_write(PMU_REG_SAR_CFG_91, val);

    pmu_sar_input_buf_enable(false);

#ifdef PMU_IRQ_UNIFIED
    pmu_read(PMU_REG_WDT_CFG, &val);
    val |= POWERON_DETECT_EN | MERGE_INTR;
    pmu_write(PMU_REG_WDT_CFG, val);
#endif

#if defined(PMU_DCDC_CALIB) && !defined(PMU_ALREADY_INIT)
    pmu_get_dcdc_calib_value();
    TRACE(0, "dcdc_cal: normal=lp vcore=%d vana=%d vcodec=%d", pmu_dcdc_dig_comp, pmu_dcdc_ana_comp, pmu_dcdc_hppa_comp);
#endif

#if defined(PMU_LDO_VCORE_CALIB) && !defined(PMU_ALREADY_INIT)
    pmu_get_ldo_dig_calib_value();
    TRACE(0, "ldo_cal: normal=lp vusb=%d", pmu_ldo_usb_comp);
#endif

#ifdef PMU_VMIN_SEARCH
    pmu_vmin_search_enable();
#endif

#ifndef NO_SLEEP
    pmu_sleep_en(1);  //enable sleep
#endif

    // Optimize time for buck work mode switch.(burst<->pwm)
    pmu_read(PMU_REG_BUCK_VHPPA_CFG_159, &val);
    val &= ~REG_BUCK_VHPPA_PWR_OR_SEL;
    pmu_write(PMU_REG_BUCK_VHPPA_CFG_159, val);

    pmu_read(PMU_REG_BUCK_VANA_CFG_15A, &val);
    val &= ~REG_BUCK_VANA_PWR_OR_SEL;
    pmu_write(PMU_REG_BUCK_VANA_CFG_15A, val);

    pmu_read(PMU_REG_BUCK_VCORE_CFG_15B, &val);
    val &= ~REG_BUCK_VCORE_PWR_OR_SEL;
    pmu_write(PMU_REG_BUCK_VCORE_CFG_15B, val);

    uint16_t val_burst_threshold_calib, vtemp, val_efuse_5;

    pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_LV, &val);
    val_burst_threshold_calib = GET_BITFIELD(val, PMU_EFUSE_VANA_THRESHOLD);
    if (val_burst_threshold_calib) {
        val_burst_threshold_calib |= (1 << 4);
        pmu_read(PMU_REG_BUCK_ANA_CFG_C4, &val);
        val = SET_BITFIELD(val, REG_BUCK_VANA_BURST_THRESHOLD, val_burst_threshold_calib);
        pmu_write(PMU_REG_BUCK_ANA_CFG_C4, val);
    }

    pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_HV, &val);
    val_burst_threshold_calib = GET_BITFIELD(val, PMU_EFUSE_VCORE_THRESHOLD);
    vtemp = !!(val & PMU_EFUSE_VCORE_THRESHOLD_BIT_0);

    if (val_burst_threshold_calib || vtemp) {
        val_burst_threshold_calib = (1 << 4) | (val_burst_threshold_calib << 1) | (vtemp << 0);
        pmu_read(PMU_REG_BUCK_CORE_CFG_C7, &val);
        val = SET_BITFIELD(val, REG_BUCK_VCORE_BURST_THRESHOLD, val_burst_threshold_calib);
        pmu_write(PMU_REG_BUCK_CORE_CFG_C7, val);
    }

    pmu_get_efuse(PMU_EFUSE_PAGE_EXT_GPADC_LV, &val);
    val_burst_threshold_calib = GET_BITFIELD(val, PMU_EFUSE_VHPPA_THRESHOLD);
    if (val_burst_threshold_calib) {
        val_burst_threshold_calib = (1 << 4) | (val_burst_threshold_calib << 1) | (1 << 0);
        pmu_read(PMU_REG_BUCK_HPPA_FREQ_CFG, &val);
        val = SET_BITFIELD(val, REG_BUCK_VHPPA_BURST_THRESHOLD, val_burst_threshold_calib);
        pmu_write(PMU_REG_BUCK_HPPA_FREQ_CFG, val);
    }

    pmu_get_efuse(PMU_EFUSE_PAGE_SW_CFG, &val_efuse_5);
    vtemp = GET_BITFIELD(val_efuse_5, PMU_EFUSE_VHPPA_IS_GAIN_BIT_1_0);
    if (vtemp) {
        vtemp |= (1 << 2);

        pmu_read(PMU_REG_BUCK_VHPPA_CFG_144, &val);
        val = SET_BITFIELD(val, REG_BUCK_VHPPA_IS_GAIN_NORMAL, vtemp);
        pmu_write(PMU_REG_BUCK_VHPPA_CFG_144, val);

        pmu_read(PMU_REG_BUCK_VHPPA_LP_CFG_145, &val);
        val = SET_BITFIELD(val, REG_BUCK_VHPPA_IS_GAIN_DSLEEP, vtemp);
        pmu_write(PMU_REG_BUCK_VHPPA_LP_CFG_145, val);

        pmu_read(PMU_REG_BUCK_VHPPA_RC_CFG_146, &val);
        val = SET_BITFIELD(val, REG_BUCK_VHPPA_IS_GAIN_RC, vtemp);
        pmu_write(PMU_REG_BUCK_VHPPA_RC_CFG_146, val);
    }

    vtemp = GET_BITFIELD(val_efuse_5, PMU_EFUSE_VANA_IS_GAIN_BIT_1_0);
    if (vtemp) {
        vtemp |= (1 << 2);

        pmu_read(PMU_REG_BUCK_VANA_CFG_139, &val);
        val = SET_BITFIELD(val, REG_BUCK_VANA_IS_GAIN_NORMAL, vtemp);
        pmu_write(PMU_REG_BUCK_VANA_CFG_139, val);

        pmu_read(PMU_REG_BUCK_VANA_LP_CFG_13A, &val);
        val = SET_BITFIELD(val, REG_BUCK_VANA_IS_GAIN_DSLEEP, vtemp);
        pmu_write(PMU_REG_BUCK_VANA_LP_CFG_13A, val);

        pmu_read(PMU_REG_BUCK_VANA_RC_CFG_140, &val);
        val = SET_BITFIELD(val, REG_BUCK_VANA_IS_GAIN_RC, vtemp);
        pmu_write(PMU_REG_BUCK_VANA_RC_CFG_140, val);
    }

    if (hal_get_chip_metal_id() > HAL_CHIP_METAL_ID_3) {
        pmu_read(PMU_REG_RESERVED_ANA, &val);
        val &= ~REG_BUCK_CURRENT_REF_ULP;
        pmu_write(PMU_REG_RESERVED_ANA, val);

        pmu_read(PMU_REG_BUCK_ANA_CFG_C5, &val);
        val = SET_BITFIELD(val, REG_BUCK_VANA_RESERVED, 0x7);
        pmu_write(PMU_REG_BUCK_ANA_CFG_C5, val);

        pmu_read(PMU_REG_BUCK_CORE_CFG_C8, &val);
        val = SET_BITFIELD(val, REG_BUCK_VCORE_RESERVED, 0x7);
        pmu_write(PMU_REG_BUCK_CORE_CFG_C8, val);

        pmu_read(PMU_REG_BUCK_VHPPA_CFG_E8, &val);
        val = SET_BITFIELD(val, REG_BUCK_VHPPA_RESERVED, 0x7);
        pmu_write(PMU_REG_BUCK_VHPPA_CFG_E8, val);
    } else {
        // Disable buck ulp mode by default
        pmu_read(PMU_REG_BUCK_VANA_LP_CFG_13A, &val);
        val &= ~REG_BUCK_VANA_ULP_MODE_DSLEEP;
        pmu_write(PMU_REG_BUCK_VANA_LP_CFG_13A, val);

        pmu_read(PMU_REG_BUCK_VCORE_LP_CFG_142, &val);
        val &= ~REG_BUCK_VCORE_ULP_MODE_DSLEEP;
        pmu_write(PMU_REG_BUCK_VCORE_LP_CFG_142, val);

        pmu_read(PMU_REG_BUCK_VHPPA_LP_CFG_145, &val);
        val &= ~REG_BUCK_VHPPA_ULP_MODE_DSLEEP;
        pmu_write(PMU_REG_BUCK_VHPPA_LP_CFG_145, val);

        pmu_read(PMU_REG_BUCK_VANA_RC_CFG_140, &val);
        val &= ~REG_BUCK_VANA_ULP_MODE_RC;
        pmu_write(PMU_REG_BUCK_VANA_RC_CFG_140, val);

        pmu_read(PMU_REG_BUCK_VCORE_RC_CFG_143, &val);
        val &= ~REG_BUCK_VCORE_ULP_MODE_RC;
        pmu_write(PMU_REG_BUCK_VCORE_RC_CFG_143, val);

        pmu_read(PMU_REG_BUCK_VHPPA_RC_CFG_146, &val);
        val &= ~REG_BUCK_VHPPA_ULP_MODE_RC;
        pmu_write(PMU_REG_BUCK_VHPPA_RC_CFG_146, val);
    }

    pmu_read(PMU_REG_BUCK_VHPPA_CFG, &val);
#ifdef PMU_DCDC_PWM
    val &= ~REG_BUCK_VHPPA_BURST_MODE_SEL_NORMAL;
#else
    val |= REG_BUCK_VHPPA_BURST_MODE_SEL_NORMAL;
#endif
    pmu_write(PMU_REG_BUCK_VHPPA_CFG, val);

    pmu_read(PMU_REG_BUCK_VANA_CFG, &val);
#ifdef PMU_DCDC_PWM
    val &= ~REG_BUCK_VANA_BURST_MODE_SEL_NORMAL;
#else
    val |= REG_BUCK_VANA_BURST_MODE_SEL_NORMAL;
#endif
    pmu_write(PMU_REG_BUCK_VANA_CFG, val);

    pmu_read(PMU_REG_BUCK_VCORE_CFG, &val);
#ifdef PMU_DCDC_PWM
    val &= ~REG_BUCK_VCORE_BURST_MODE_SEL_NORMAL;
#else
    val |= REG_BUCK_VCORE_BURST_MODE_SEL_NORMAL;
#endif
    pmu_write(PMU_REG_BUCK_VCORE_CFG, val);

    pmu_read(PMU_REG_BUCK_VHPPA_RC_CFG, &val);
#ifdef PMU_DCDC_PWM
    val &= ~REG_BUCK_VHPPA_BURST_MODE_SEL_RC;
#else
    val |= REG_BUCK_VHPPA_BURST_MODE_SEL_RC;
#endif
    pmu_write(PMU_REG_BUCK_VHPPA_RC_CFG, val);

    pmu_read(PMU_REG_BUCK_VANA_RC_CFG, &val);
#ifdef PMU_DCDC_PWM
    val &= ~REG_BUCK_VANA_BURST_MODE_SEL_RC;
#else
    val |= REG_BUCK_VANA_BURST_MODE_SEL_RC;
#endif
    pmu_write(PMU_REG_BUCK_VANA_RC_CFG, val);

    pmu_read(PMU_REG_BUCK_VCORE_RC_CFG, &val);
#ifdef PMU_DCDC_PWM
    val &= ~REG_BUCK_VCORE_BURST_MODE_SEL_RC;
#else
    val |= REG_BUCK_VCORE_BURST_MODE_SEL_RC;
#endif
    pmu_write(PMU_REG_BUCK_VCORE_RC_CFG, val);

    pmu_read(PMU_REG_LDO_USB_CFG, &val);
    val &= ~REG_LDO_VUSB_RAMP_EN;
    pmu_write(PMU_REG_LDO_USB_CFG, val);

    pmu_read(PMU_REG_BUCK_VCORE_EN, &val);
#ifdef DCDC_VOLT_HW_RAMP_ONLY
    val |= REG_BUCK_VCORE_RAMP_EN;
#else
    val &= ~REG_BUCK_VCORE_RAMP_EN;
#endif
    val = SET_BITFIELD(val, REG_BUCK_VCORE_RAMP_STEP, PMU_DCDC_HW_RAMP_1_CLK_STEP);
    val |= REG_PU_BUCK_VCORE_DR | REG_PU_BUCK_VCORE;
    pmu_write(PMU_REG_BUCK_VCORE_EN, val);

    pmu_read(PMU_REG_BUCK_VANA_EN, &val);
#ifdef DCDC_VOLT_HW_RAMP_ONLY
    val |= REG_BUCK_VANA_RAMP_EN;
#else
    val &= ~REG_BUCK_VANA_RAMP_EN;
#endif
    val = SET_BITFIELD(val, REG_BUCK_VANA_RAMP_STEP, PMU_DCDC_HW_RAMP_1_CLK_STEP_HALF);
    val |= REG_PU_BUCK_VANA_DR | REG_PU_BUCK_VANA;
    pmu_write(PMU_REG_BUCK_VANA_EN, val);

    pmu_read(PMU_REG_BUCK_VHPPA_EN, &val);
#ifdef DCDC_VOLT_HW_RAMP_ONLY
    val |= REG_BUCK_VHPPA_RAMP_EN;
#else
    val &= ~REG_BUCK_VHPPA_RAMP_EN;
#endif
    val = SET_BITFIELD(val, REG_BUCK_VHPPA_RAMP_STEP, PMU_DCDC_HW_RAMP_1_CLK_STEP_HALF);
    val |= REG_PU_BUCK_VHPPA_DR | REG_PU_BUCK_VHPPA;
    pmu_write(PMU_REG_BUCK_VHPPA_EN, val);

#ifndef DCDC_VOLT_FAST_HW_RAMP
    pmu_read(PMU_REG_FAST_HW_RAMP_CFG, &val);
    val = (val & ~REG_RAMP_FAST_EN) | REG_RAMP_FAST_EN_DR;
    pmu_write(PMU_REG_FAST_HW_RAMP_CFG, val);
#endif

#ifndef PROGRAMMER
    if (pmu_pkg_type == PMU_PACKAGE_TYPE_IA) {
        pmu_vusb_req |= PMU_VUSB_REQ_GPIO;
    }
    // Disable usbphy power, and vusb if possible
    pmu_usb_config(PMU_USB_CONFIG_TYPE_NONE);
#endif

    pmu_codec_mic_bias_set_volt(AUD_VMIC_MAP_VMIC1 | AUD_VMIC_MAP_VMIC2, 2200);

    // Set LDO force lp mode by default
    pmu_read(PMU_REG_LDO_USB_CFG, &val);
    val |= REG_LP_EN_LDO_VUSB_DR | REG_LP_EN_LDO_VUSB_REG;
    pmu_write(PMU_REG_LDO_USB_CFG, val);

    pmu_read(PMU_REG_MIC_PULL_DOWN, &val);
    val |= REG_MIC_LP_ENABLE;
    pmu_write(PMU_REG_MIC_PULL_DOWN, val);

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

#ifdef HPPA_LDO_ON
    ASSERT(false, "1306p don't support HPPA_LDO_ON!");
#else
    val = pmu_dcdc_hppa_mv_to_val(vhppa_mv);
    pmu_hppa_set_volt(val);
#endif

#ifdef DIG_DCDC_MODE
    mode = PMU_POWER_MODE_DIG_DCDC;
#elif defined(ANA_DCDC_MODE)
    mode = PMU_POWER_MODE_ANA_DCDC;
#else // LDO_MODE
    mode = PMU_POWER_MODE_LDO;
#endif

    ASSERT(mode == PMU_POWER_MODE_DIG_DCDC, "1306p only support DIG_DCDC_MODE!");

    // Set VANA VCORE voltage
    pmu_mode_change(mode);

#if defined(MCU_HIGH_PERFORMANCE_MODE)
#error "Not support MCU_HIGH_PERFORMANCE_MODE!"
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

#ifdef PMU_NTC_MONITOR
    pmu_ntc_monitor_init();
#ifdef LOW_SLEEP_VCORE
    pmu_get_efuse(PMU_EFUSE_PAGE_EXT_GPADC_HV, &val);
    if ((val & PMU_EFUSE_PAGE_ULTRA_LOW_SLEEP_VCORE) && ntc_ctx.temperature > 5) {
        dig_lp_dcdc = PMU_DCDC_DIG_0_6V;
    }
#endif
#endif

    // Set sleep VCORE
    pmu_dcdc_dig_get_volt(&val, NULL);
    pmu_dcdc_dig_set_volt(val, dig_lp_dcdc, true);

    // charger_charge_open();

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

    if (pmu_pkg_type == PMU_PACKAGE_TYPE_IHC_V1_NO_I2S) {
        hal_iomux_set_analog_io(HAL_IOMUX_PIN_P4_0);
        hal_iomux_set_analog_io(HAL_IOMUX_PIN_P4_1);
        hal_iomux_set_analog_io(HAL_IOMUX_PIN_P4_2);
        hal_iomux_set_analog_io(HAL_IOMUX_PIN_P4_3);
    } else if (pmu_pkg_type == PMU_PACKAGE_TYPE_IHC_V1_I2S) {
        hal_iomux_set_analog_io(HAL_IOMUX_PIN_P1_2);
        hal_iomux_set_analog_io(HAL_IOMUX_PIN_P1_3);
        hal_iomux_set_analog_io(HAL_IOMUX_PIN_P1_4);
        hal_iomux_set_analog_io(HAL_IOMUX_PIN_P1_5);
    }

    return 0;
}

void pmu_big_bandgap_enable(enum PMU_BIG_BANDGAP_USER_T user, int enable)
{
}

void pmu_sleep(void)
{
#ifdef DCDC_VOLT_FAST_HW_RAMP
    pmu_buck_volt_hw_ramp_step_set(PMU_DCDC_F_HW_RAMP_1_CLK_STEP);
#endif
    if (!dcdc_ramp_en) {
        // Enable DCDC ramp
        pmu_buck_volt_hw_ramp_enable(dcdc_ramp_map);
    }
}

void pmu_wakeup(void)
{
#ifdef DCDC_VOLT_FAST_HW_RAMP
    pmu_buck_volt_hw_ramp_step_set(PMU_DCDC_HW_RAMP_1_CLK_STEP);
#endif
    if (!dcdc_ramp_en) {
        pmu_buck_volt_hw_ramp_disable(dcdc_ramp_map);
    }
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
        volt = PMU_VMIC_1_6V + (mv - 1600 + step / 2) / step;
        if (volt > PMU_VMIC_2_8V) {
            volt = PMU_VMIC_2_8V;
        }
    }

    if (vcodec_mv > 1700) {
        volt -= 2;
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
        pmu_read(PMU_REG_MIC_BIAS_A, &val);
        val |= REG_MIC_LDO_LOOPCTRL;
        pmu_write(PMU_REG_MIC_BIAS_A, val);

        pmu_read(PMU_REG_MIC_BIAS_B, &val);
        val |= REG_MIC_LDO_EN;
        pmu_write(PMU_REG_MIC_BIAS_B, val);
        reg_mic_ldo_en = true;
    } else if (need_ldo_on_cnt == 0) {
        pmu_read(PMU_REG_MIC_BIAS_B, &val);
        val &= ~REG_MIC_LDO_EN;
        pmu_write(PMU_REG_MIC_BIAS_B, val);

        pmu_read(PMU_REG_MIC_BIAS_A, &val);
        val &= ~REG_MIC_LDO_LOOPCTRL;
        pmu_write(PMU_REG_MIC_BIAS_A, val);
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

    if (pmu_power_mode == PMU_POWER_MODE_NONE || !pmu_opened) {
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

    if (pmu_power_mode == PMU_POWER_MODE_NONE || !pmu_opened) {
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
    uint32_t lock;
    enum PMU_VCORE_REQ_T old_req;
    bool update = false;

#ifdef ARM_CMSE
    ASSERT(!pmu_ns_running, "pmu_sys_freq_config: NS is running");
#endif

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
    if (0) {
#ifdef AUD_PLL_DOUBLE
    } else if (freq > HAL_CMU_FREQ_208M) {
        pmu_vcore_req |= PMU_VCORE_SYS_FREQ_HIGH;
#endif
    } else if (freq > HAL_CMU_FREQ_104M) {
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

    if (pmu_power_mode == PMU_POWER_MODE_NONE || !pmu_opened) {
        // PMU not init yet
        pmu_dig_init_volt();
        return;
    }

    pmu_dig_set_volt(PMU_POWER_MODE_NONE);
#endif
}

void pmu_high_performance_mode_enable(bool enable)
{
}

void pmu_usb_config(enum PMU_USB_CONFIG_TYPE_T type)
{
    uint32_t lock;
    bool update;
    unsigned short ldo_on, deep_sleep_on;

    update = false;

    if (type == PMU_USB_CONFIG_TYPE_NONE) {
#ifdef VUSB_ON
        ldo_on = PMU_LDO_ON;
        deep_sleep_on = PMU_DSLEEP_MODE_ON;
#else
        ldo_on = PMU_LDO_OFF;
        deep_sleep_on = PMU_DSLEEP_MODE_OFF;
#endif
    } else {
        ldo_on = PMU_LDO_ON;
        deep_sleep_on = PMU_DSLEEP_MODE_ON;
    }

    lock = int_lock();

    if (type == PMU_USB_CONFIG_TYPE_NONE) {
        pmu_vusb_req &= ~PMU_VUSB_REQ_USB;
        if (pmu_vusb_req == 0) {
            update = true;
        }
    } else {
        if (pmu_vusb_req == 0) {
            update = true;
        }
        pmu_vusb_req |= PMU_VUSB_REQ_USB;
    }

    if (update) {
        pmu_module_config(PMU_USB, PMU_MANUAL_MODE, ldo_on, PMU_LP_MODE_ON, deep_sleep_on);
    }

#ifdef USB_HIGH_SPEED
    if (type == PMU_USB_CONFIG_TYPE_NONE) {
        pmu_vcore_req &= ~PMU_VCORE_USB_HS_ENABLED;
    } else {
        pmu_vcore_req |= PMU_VCORE_USB_HS_ENABLED;
    }
#endif

    int_unlock(lock);

    if (pmu_power_mode != PMU_POWER_MODE_NONE) {
        // PMU has been inited
        pmu_dig_set_volt(PMU_POWER_MODE_NONE);
    }
}

int pmu_usb_config_pin_status_check(enum PMU_USB_PIN_CHK_STATUS_T status, PMU_USB_PIN_CHK_CALLBACK callback, int enable)
{
    return 0;
}

void pmu_usb_enable_pin_status_check(void)
{
}

void pmu_usb_disable_pin_status_check(void)
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

static void pmu_charger_1622_timer_handler(void *param)
{
    charger_plug_conig(PMU_CHARGER_PLUGOUT);
}

static void pmu_charger_1622_timer_2_handler(void *param)
{
    charger_acin2vsys_ldo_soft_start_enable(true);
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
    if (charger_get_charger_type() == CHARGER_CHIP_TYPE_1622) {
        if (status == PMU_CHARGER_PLUGIN) {
            if (charger_1622_timer) {
                hwtimer_stop(charger_1622_timer);
            }
            charger_plug_conig(PMU_CHARGER_PLUGIN);
            // Resume ACIN2VSYS LDO soft startup to avoid VSYS and ACIN voltages being equally high.
            if (charger_1622_timer_2 == NULL) {
                charger_1622_timer_2 = hwtimer_alloc(pmu_charger_1622_timer_2_handler, 0);
            }
            if (charger_1622_timer_2) {
                hwtimer_stop(charger_1622_timer_2);
                hwtimer_start(charger_1622_timer_2, MS_TO_TICKS(20));
            }
        } else {
            if (charger_1622_timer == NULL) {
                charger_1622_timer = hwtimer_alloc(pmu_charger_1622_timer_handler, 0);
            }
            if (charger_1622_timer) {
                hwtimer_stop(charger_1622_timer);
                hwtimer_start(charger_1622_timer, MS_TO_TICKS(100));
            }
            // Disable ACIN2VSYS LDO soft startup to avoid VSYS drop.
            if (charger_1622_timer_2) {
                hwtimer_stop(charger_1622_timer_2);
            }
            charger_acin2vsys_ldo_soft_start_enable(false);
        }
    }

    if (charger_irq_handler) {
        charger_irq_handler(status);
    }
}

void pmu_charger_set_irq_handler(PMU_CHARGER_IRQ_HANDLER_T handler)
{
    uint32_t lock;
    uint16_t val;
    bool charger_1622 = false;

    if (charger_get_charger_type() == CHARGER_CHIP_TYPE_1622) {
        charger_1622 = true;
    }

    charger_irq_handler = handler;

    lock = int_lock();
    pmu_read(PMU_REG_CHARGER_CFG, &val);
    if (handler || charger_1622) {
        val |= REG_CHARGE_IN_INTR_MSK | REG_CHARGE_OUT_INTR_MSK;
    } else {
        val &= ~(REG_CHARGE_IN_INTR_MSK | REG_CHARGE_OUT_INTR_MSK);
    }
    pmu_write(PMU_REG_CHARGER_CFG, val);

#ifdef PMU_IRQ_UNIFIED
    if (charger_1622) {
        pmu_set_irq_unified_handler(PMU_IRQ_TYPE_CHARGER, pmu_charger_irq_handler);
    } else {
        pmu_set_irq_unified_handler(PMU_IRQ_TYPE_CHARGER, handler ? pmu_charger_irq_handler : NULL);
    }
#else
    if (handler || charger_1622) {
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
        val |= REG_WDT_INTR_MSK;
    } else {
        val &= ~REG_WDT_INTR_MSK;
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

    pmu_read(PMU_REG_WDT_INT_CFG, &val);
    val |= REG_WDT_INTR_EN;
    pmu_write(PMU_REG_WDT_INT_CFG, val);
}

#ifndef __WATCHER_DOG_RESET__
BOOT_TEXT_SRAM_LOC
#endif
void pmu_wdt_stop(void)
{
    uint16_t val;

    pmu_read(PMU_REG_WDT_INT_CFG, &val);
    val &= ~REG_WDT_INTR_EN;
    pmu_write(PMU_REG_WDT_INT_CFG, val);

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

void pmu_bt_reconn(bool en)
{
}

void pmu_bt_2v5pa(bool en)
{
}

void pmu_vio_3v3(bool en)
{
}

void pmu_sar_adc_vref_enable(void)
{
    uint16_t val;

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

    pmu_sar_adc_vref_sw_pu(false);

    pmu_read(PMU_REG_SAR_PU_CFG, &val);
    val &= ~(REG_SAR_PU_VREF_REG | REG_SAR_PU_VREF_DR | REG_SAR_PU_REG | REG_SAR_PU_DR);
    pmu_write(PMU_REG_SAR_PU_CFG, val);

    pmu_read(PMU_REG_AVDD_EN, &val);
    val &= ~REG_PU_AVDD25_ANA;
    pmu_write(PMU_REG_AVDD_EN, val);
}

int pmu_sar_input_buf_enable(int enable)
{
    uint16_t val;

    pmu_read(PMU_REG_SAR_PU_CFG, &val);
    if (enable) {
        val |= REG_SAR_PU_VREF_IN;
    } else {
        val &= ~REG_SAR_PU_VREF_IN;
    }
    pmu_write(PMU_REG_SAR_PU_CFG, val);

    pmu_read(PMU_REG_SAR_CLK_CFG, &val);
    if (enable) {
        val |= REG_SAR_INPUT_BUF_EN;
    } else {
        val &= ~REG_SAR_INPUT_BUF_EN;
    }
    pmu_write(PMU_REG_SAR_CLK_CFG, val);

    return 0;
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
    pmu_read(PMU_REG_15C, &val);
    if (enable) {
        val |= REG_PU_BATTERY_SENSE;
    } else {
        val &= ~REG_PU_BATTERY_SENSE;
    }
    pmu_write(PMU_REG_15C, val);
    int_unlock(lock);

    return 0;
}

static int pmu_vchgr_div_ctrl(int enable)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    pmu_read(PMU_REG_15C, &val);
    if (enable) {
        val |= REG_PU_ACIN_SENSE;
    } else {
        val &= ~REG_PU_ACIN_SENSE;
    }
    pmu_write(PMU_REG_15C, val);
    int_unlock(lock);

    return 0;
}

int pmu_gpadc_div_ctrl(enum HAL_GPADC_CHAN_T channel, int enable)
{
    int ret = 0;

    if (channel == HAL_GPADC_CHAN_BATTERY) {
        ret = pmu_vsys_div_ctrl(enable);
    } else if (channel == HAL_GPADC_CHAN_5) {
        ret = pmu_vchgr_div_ctrl(enable);
    } else if (channel == HAL_GPADC_CHAN_6) {
        ret = pmu_vbat_div_ctrl(enable);
    }

    return ret;
}

void pmu_ntc_capture_enable(void)
{
    uint16_t val;

    pmu_read(PMU_REG_IPTAT_CORE_I_CFG, &val);
    val |= DIG_IPTAT_CORE_EN;
    pmu_write(PMU_REG_IPTAT_CORE_I_CFG, val);
}

void pmu_ntc_capture_disable(void)
{
    uint16_t val;

    pmu_read(PMU_REG_IPTAT_CORE_I_CFG, &val);
    val &= ~DIG_IPTAT_CORE_EN;
    pmu_write(PMU_REG_IPTAT_CORE_I_CFG, val);
}

int pmu_volt2temperature(const uint16_t volt)
{
    const int Kvtherm_T = 346;
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

#ifdef PMU_NTC_MONITOR
static void pmu_ntc_irq_handler(uint16_t raw, uint16_t volt)
{
    pmu_ntc_capture_disable();

    ntc_ctx.temperature = pmu_volt2temperature(volt);
    ntc_ctx.temperature_update_ms = TICKS_TO_MS(hal_sys_timer_get());

    ntc_irq_busy = false;
}

static int pmu_ntc_capture_start(void)
{
    int nRet = 0;
    uint32_t lock = 0;

    lock = int_lock();
    if (ntc_irq_busy) {
        nRet = -1;
        goto exit;
    }
    ntc_irq_busy = true;
    pmu_ntc_capture_enable();
    hal_gpadc_open(HAL_GPADC_CHAN_0, HAL_GPADC_ATP_ONESHOT, pmu_ntc_irq_handler);
exit:
    int_unlock(lock);

    return nRet;
}

#ifdef NTC_MONITOR_USE_OSTIMER
static void pmu_ntc_monitor_ostimer_handler(void const *param)
{
    pmu_ntc_capture_start();
    osTimerStart(ntc_monitor_ostimer_id, ntc_ctx.monitor_interval_ms);
}
#else
static void pmu_ntc_monitor_hwtimer_handler(void *param)
{
    pmu_ntc_capture_start();
    hwtimer_start(ntc_monitor_timer, MS_TO_TICKS(ntc_ctx.monitor_interval_ms));
}
#endif

static void pmu_setup_ntc_monitor_timer(void)
{
#ifdef NTC_MONITOR_USE_OSTIMER
    if (ntc_monitor_ostimer_id == NULL) {
        ntc_monitor_ostimer_id = osTimerCreate(osTimer(ntc_monitor_ostimer), osTimerPeriodic, NULL);
        ASSERT(ntc_monitor_ostimer_id, "%s fail!", __func__);
    }
#else
    if (ntc_monitor_timer == NULL) {
        ntc_monitor_timer = hwtimer_alloc(pmu_ntc_monitor_hwtimer_handler, 0);
        ASSERT(ntc_monitor_timer, "%s fail!", __func__);
    }
#endif
}

static void pmu_stop_ntc_monitor_timer(void)
{
#ifdef NTC_MONITOR_USE_OSTIMER
    osTimerStop(ntc_monitor_ostimer_id);
#else
    hwtimer_stop(ntc_monitor_timer);
#endif
}

static void pmu_start_ntc_monitor_timer(void)
{
#ifdef NTC_MONITOR_USE_OSTIMER
    osTimerStart(ntc_monitor_ostimer_id, ntc_ctx.monitor_interval_ms);
#else
    hwtimer_start(ntc_monitor_timer, MS_TO_TICKS(ntc_ctx.monitor_interval_ms));
#endif
}

static void pmu_ntc_monitor_init(void)
{
    uint32_t sum_val = 0;
    uint16_t volt;
    int i, valid_cnt, ret;

    valid_cnt = 0;

    pmu_ntc_capture_enable();

    hal_gpadc_open(HAL_GPADC_CHAN_0, HAL_GPADC_ATP_125US, NULL);

    for (i = 0; i < 5; i++) {
        ret = hal_gpadc_get_volt_blocked(HAL_GPADC_CHAN_0, &volt);
        if (ret) {
            sum_val += volt;
            valid_cnt++;
        }
    }

    hal_gpadc_close(HAL_GPADC_CHAN_0);

    pmu_ntc_capture_disable();

    ASSERT(valid_cnt, "%s: fail! Invalid gpadc read!", __func__);

    volt = sum_val / valid_cnt;

    ntc_ctx.temperature = pmu_volt2temperature(volt);
    ntc_ctx.temperature_update_ms = TICKS_TO_MS(hal_sys_timer_get());
    ntc_ctx.monitor_interval_ms = 0;

    pmu_setup_ntc_monitor_timer();
}

int pmu_ntc_monitor_register(enum PMU_NTC_USER_T user, struct PMU_NTC_MONITOR_CTX_T *ctx)
{
    uint32_t lock;
    uint32_t min_interval_ms = UINT32_MAX;

    if (user >= PMU_NTC_USER_QTY) {
        return -1;
    }

    if (ctx->enable && ctx->interval_ms < 5) {
        return -2;
    }

    lock = int_lock();

    ntc_monitor_ctx[user].interval_ms = ctx->interval_ms;
    ntc_monitor_ctx[user].enable = ctx->enable;

    for (int i = 0; i < PMU_NTC_USER_QTY; i++) {
        if (ntc_monitor_ctx[i].enable && ntc_monitor_ctx[i].interval_ms > 0) {
            if (ntc_monitor_ctx[i].interval_ms < min_interval_ms) {
                min_interval_ms = ntc_monitor_ctx[i].interval_ms;
            }
        }
    }

    if (min_interval_ms == UINT32_MAX) {
        ntc_ctx.monitor_interval_ms = 0;
        pmu_ntc_capture_disable();
        hal_gpadc_close(HAL_GPADC_CHAN_0);
        pmu_stop_ntc_monitor_timer();
        ntc_irq_busy = false;
    } else {
        ntc_ctx.monitor_interval_ms = min_interval_ms;
    }

    int_unlock(lock);

    // Update ntc_ctx once if ntc not in work
    if (!ntc_irq_busy && ntc_ctx.monitor_interval_ms) {
        pmu_start_ntc_monitor_timer();
    }

    return 0;
}

int pmu_ntc_monitor_deregister(enum PMU_NTC_USER_T user)
{
    struct PMU_NTC_MONITOR_CTX_T ctx = {0, false};

    return pmu_ntc_monitor_register(user, &ctx);
}

int pmu_ntc_ctx_get(struct PMU_NTC_CTX_T *ctx)
{
    int ret = 0;

    ctx->temperature = ntc_ctx.temperature;
    ctx->temperature_update_ms = ntc_ctx.temperature_update_ms;
    ctx->monitor_interval_ms = ntc_ctx.monitor_interval_ms;

    // ntc_monitor_timer has been disabled
    // maybe ntc_ctx is invalid
    if (ntc_ctx.monitor_interval_ms == 0) {
        ret = -1;
    }

    return ret;
}
#endif // PMU_NTC_MONITOR

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

        pmu_ext_crystal_clkout_usr_map |= pin;
        hal_sys_timer_delay(MS_TO_TICKS(PMU_EXT_32K_CLK_OUT_STABLE_TIME));
    }
}

void pmu_external_crystal_clkout_disable(enum PMU_EXT_CRYSTAL_CLKOUT_PIN_T pin)
{
    uint16_t val;

    if (pin == PMU_EXT_CRYSTAL_CLKOUT_PIN_0) {
        pmu_read(PMU_REG_PU_VSYS_DIV, &val);
        val &= ~REG_CLK_32K_EXT1_EN;
        pmu_write(PMU_REG_PU_VSYS_DIV, val);

        pmu_ext_crystal_clkout_usr_map &= ~pin;
    }
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

#ifdef RC_CLK_ENABLE
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

        hal_cmu_select_rc_clock(HAL_CMU_CLK_SEL_OSC_USER_SYS);
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

#endif

void pmu_capsensor_close(void)
{
}

void pmu_capsensor_open(uint8_t clk_cap_div)
{
    uint16_t val;

#if 0  //agpio test 10
    hal_sys_timer_delay(US_TO_TICKS(500));

    pmu_read(PMU_REG_RCOSC_192M_CFG, &val);
    val |= REG_RCOSC_192M_EN_AGPIO_TEST | REG_RCOSC_192M_EN_AGPIO_TEST_SEL;
    pmu_write(PMU_REG_RCOSC_192M_CFG, val);
#endif

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
}

void pmu_ldo_vsensor_on_lp_dr(bool dr_lp)
{
}

#ifdef GPADC_CUSTOM_CALIB_VAL
void pmu_get_vbat_calib_value(unsigned short *val_lv, unsigned short *val_hv)
{
    pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_LV, val_lv);
    pmu_get_efuse(PMU_EFUSE_PAGE_BATTER_HV, val_hv);
    *val_lv = GET_BITFIELD(*val_lv, PMU_EFUSE_PAGE_BATTER_LV_OFFSET);
    *val_hv = GET_BITFIELD(*val_hv, PMU_EFUSE_PAGE_BATTER_HV_OFFSET);

    TRACE(0, "%s, efuse_lv/hv:%d/%d", __func__, *val_lv, *val_hv);

    if (*val_lv == 0 || *val_hv == 0) {
        *val_lv = PMU_EFUSE_GPADC_CALIB_DEFAULT;
        *val_hv = PMU_EFUSE_GPADC_CALIB_DEFAULT;
    }

    *val_lv += PMU_EFUSE_PAGE_BATTER_LV_BASE;
    *val_hv += PMU_EFUSE_PAGE_BATTER_HV_BASE;
}

#ifdef GPADC_HAS_EXT_SLOPE_CAL
void pmu_get_ext_gpadc_calib_value(unsigned short *val_lv, unsigned short *val_hv)
{
    pmu_get_efuse(PMU_EFUSE_PAGE_EXT_GPADC_LV, val_lv);
    pmu_get_efuse(PMU_EFUSE_PAGE_EXT_GPADC_HV, val_hv);
    *val_lv = GET_BITFIELD(*val_lv, PMU_EFUSE_PAGE_EXT_GPADC_LV_OFFSET);
    *val_hv = GET_BITFIELD(*val_hv, PMU_EFUSE_PAGE_EXT_GPADC_HV_OFFSET);

    TRACE(0, "%s, efuse_lv/hv:%d/%d", __func__, *val_lv, *val_hv);

    if (*val_lv == 0 || *val_hv == 0) {
        *val_lv = PMU_EFUSE_GPADC_CALIB_DEFAULT;
        *val_hv = PMU_EFUSE_GPADC_CALIB_DEFAULT;
    }

    *val_lv += PMU_EFUSE_PAGE_EXT_GPADC_LV_BASE;
    *val_hv += PMU_EFUSE_PAGE_EXT_GPADC_HV_BASE;
}
#endif
#endif

int pmu_get_acin_volt(uint16_t *volt)
{
    uint32_t sum_val = 0;
    uint16_t v;
    int valid_cnt, nRet;
    const enum HAL_GPADC_CHAN_T ch = HAL_GPADC_CHAN_5;
    const uint8_t div = 5;
    const uint8_t db_cnt = 3;

    valid_cnt = 0;

    hal_gpadc_open(ch, HAL_GPADC_ATP_125US, NULL);

    for (int i = 0; i < db_cnt; i++) {
        if (hal_gpadc_get_volt_blocked(ch, &v)) {
            sum_val += v;
            valid_cnt++;
        }
    }

    hal_gpadc_close(ch);

    if (valid_cnt) {
        *volt = (sum_val / valid_cnt) * div;
        nRet = 0;
    } else {
        nRet = -1;
    }

    return nRet;
}

enum PMU_PACKAGE_TYPE_T pmu_get_chip_package_type(void)
{
    return pmu_pkg_type;
}

#ifdef RTC_CALENDAR
static float pmu_sys_tick_hz = PMU_CONFIG_SYSTICK_HZ_NOMINAL;
static uint32_t pmu_slow_val;
static uint32_t pmu_fast_val;

static void BOOT_TEXT_SRAM_LOC pmu_sys_timer_calib_start(void)
{
    uint32_t lock;
    uint32_t slow;
    uint32_t fast;

    lock = int_lock();
    slow = hal_sys_timer_get();
    while (hal_sys_timer_get() == slow);
    __ISB();
    fast = hal_fast_sys_timer_get();
    int_unlock(lock);

    pmu_slow_val = slow + 1;
    pmu_fast_val = fast;
}

static int BOOT_TEXT_SRAM_LOC pmu_sys_timer_calib_end(void)
{
    uint32_t lock;
    uint32_t slow;
    uint32_t fast;
    uint32_t slow_diff;

    lock = int_lock();
    slow = hal_sys_timer_get();
    while (hal_sys_timer_get() == slow);
    __ISB();
    fast = hal_fast_sys_timer_get();
    int_unlock(lock);

    slow += 1;
    slow_diff = slow - pmu_slow_val;

    // Avoid computation error
    if (slow_diff < PMU_MIN_CALIB_TICKS) {
        return 1;
    }
    // Avoid fast tick overflow
    if (slow_diff > PMU_MAX_CALIB_TICKS) {
        return 2;
    }

    pmu_sys_tick_hz = (float)CONFIG_FAST_SYSTICK_HZ / (fast - pmu_fast_val) * slow_diff;

    if (pmu_sys_tick_hz > PMU_MAX_CALIB_SYSTICK_HZ) {
        pmu_sys_tick_hz = PMU_MAX_CALIB_SYSTICK_HZ;
    }

    return 0;
}

static void BOOT_TEXT_SRAM_LOC pmu_sys_timer_calib(void)
{
    pmu_sys_timer_calib_start();
    hal_sys_timer_delay(PMU_MIN_CALIB_TICKS);
    pmu_sys_timer_calib_end();
}

static uint32_t pmu_sys_timer_systick_hz(void)
{
    return (uint32_t)(pmu_sys_tick_hz + 0.5);
}

void pmu_rtc_clk_div_reload(void)
{
    uint32_t lpo_freq;

    pmu_sys_timer_calib();
    lpo_freq = pmu_sys_timer_systick_hz() << 1;

    pmu_write(PMU_REG_RTC_DIV_1HZ, lpo_freq - 2);
}
#endif
