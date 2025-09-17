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
#ifndef __REG_RF_BEST1503_H__
#define __REG_RF_BEST1503_H__

#include "plat_types.h"

// REG_505
#define REG_XTAL_EN_CLKOUT                  (1 << 1)
#define REG_XTAL_LDO_BYPASS                 (1 << 2)
#define REG_XTAL_MDLL_CP_LPF_PRECHARGE_I    (1 << 4)
#define REG_XTAL_MANAAC_EN                  (1 << 5)
#define REG_XTAL_MANKICK                    (1 << 6)
#define REG_XTAL_MANKICK_EN                 (1 << 7)
#define REG_XTAL_PU_DIV1024                 (1 << 8)
#define REG_XTAL_RCOSC_CAL_EN               (1 << 9)
#define REG_XTAL_STARTUP_EN                 (1 << 10)
#define REG_XTAL_STARTUP_INJ_MODE           (1 << 11)

// REG_507
#define REG_XTAL_KICK_DRV_SEL_T_SHIFT       0
#define REG_XTAL_KICK_DRV_SEL_T_MASK        (0x3 << REG_XTAL_KICK_DRV_SEL_T_SHIFT)
#define REG_XTAL_KICK_DRV_SEL_T(n)          BITFIELD_VAL(REG_XTAL_KICK_DRV_SEL_T, n)
#define REG_XTAL_STARTUP_SEL_DRV_SHIFT      2
#define REG_XTAL_STARTUP_SEL_DRV_MASK       (0x3 << REG_XTAL_STARTUP_SEL_DRV_SHIFT)
#define REG_XTAL_STARTUP_SEL_DRV(n)         BITFIELD_VAL(REG_XTAL_STARTUP_SEL_DRV, n)
#define REG_XTAL_STARTUP_PRECHG_MANU        (1 << 4)
#define REG_XTAL_SEL_DRV_HP_OTHER_SHIFT     8
#define REG_XTAL_SEL_DRV_HP_OTHER_MASK      (0x3 << REG_XTAL_SEL_DRV_HP_OTHER_SHIFT)
#define REG_XTAL_SEL_DRV_HP_OTHER(n)        BITFIELD_VAL(REG_XTAL_SEL_DRV_HP_OTHER, n)
#define REG_XTAL_STARTUP_CNT_1ST_SHIFT      10
#define REG_XTAL_STARTUP_CNT_1ST_MASK       (0x3F << REG_XTAL_STARTUP_CNT_1ST_SHIFT)
#define REG_XTAL_STARTUP_CNT_1ST(n)         BITFIELD_VAL(REG_XTAL_STARTUP_CNT_1ST, n)

// REG_508
#define REG_XTAL_STARTUP_CNT_HOLD_SHIFT     0
#define REG_XTAL_STARTUP_CNT_HOLD_MASK      (0x3F << REG_XTAL_STARTUP_CNT_HOLD_SHIFT)
#define REG_XTAL_STARTUP_CNT_HOLD(n)        BITFIELD_VAL(REG_XTAL_STARTUP_CNT_HOLD, n)
#define REG_XTAL_MDLL_PFD_DZDLY_SHIFT       6
#define REG_XTAL_MDLL_PFD_DZDLY_MASK        (0x3 << REG_XTAL_MDLL_PFD_DZDLY_SHIFT)
#define REG_XTAL_MDLL_PFD_DZDLY(n)          BITFIELD_VAL(REG_XTAL_MDLL_PFD_DZDLY, n)
#define REG_XTAL_BUF_RC_OTHER_SHIFT         8
#define REG_XTAL_BUF_RC_OTHER_MASK          (0x3F << REG_XTAL_BUF_RC_OTHER_SHIFT)
#define REG_XTAL_BUF_RC_OTHER(n)            BITFIELD_VAL(REG_XTAL_BUF_RC_OTHER, n)
#define REG_XTAL_SEL_DRV_OTHER_SHIFT        14
#define REG_XTAL_SEL_DRV_OTHER_MASK         (0x3 << REG_XTAL_SEL_DRV_OTHER_SHIFT)
#define REG_XTAL_SEL_DRV_OTHER(n)           BITFIELD_VAL(REG_XTAL_SEL_DRV_OTHER, n)

// REG_509
#define REG_XTAL_RCOSC_FRE_TRIM_SHIFT       0
#define REG_XTAL_RCOSC_FRE_TRIM_MASK        (0x1FF << REG_XTAL_RCOSC_FRE_TRIM_SHIFT)
#define REG_XTAL_RCOSC_FRE_TRIM(n)          BITFIELD_VAL(REG_XTAL_RCOSC_FRE_TRIM, n)
#define REG_XTAL_STARTUP_ROTATE_SHIFT       9
#define REG_XTAL_STARTUP_ROTATE_MASK        (0x7 << REG_XTAL_STARTUP_ROTATE_SHIFT)
#define REG_XTAL_STARTUP_ROTATE(n)          BITFIELD_VAL(REG_XTAL_STARTUP_ROTATE, n)

// REG_50A
#define REG_XTAL_STARTUP_INJ_PERIOD_SHIFT   0
#define REG_XTAL_STARTUP_INJ_PERIOD_MASK    (0x3F << REG_XTAL_STARTUP_INJ_PERIOD_SHIFT)
#define REG_XTAL_STARTUP_INJ_PERIOD(n)      BITFIELD_VAL(REG_XTAL_STARTUP_INJ_PERIOD, n)
#define REG_XTAL_STARTUP_VLDO_ADJUST_SHIFT  6
#define REG_XTAL_STARTUP_VLDO_ADJUST_MASK   (0x7 << REG_XTAL_STARTUP_VLDO_ADJUST_SHIFT)
#define REG_XTAL_STARTUP_VLDO_ADJUST(n)     BITFIELD_VAL(REG_XTAL_STARTUP_VLDO_ADJUST, n)
#define REG_XTAL_MDLL_CP_LDO_BYPASS_I_SHIFT 9
#define REG_XTAL_MDLL_CP_LDO_BYPASS_I_MASK  (0x3 << REG_XTAL_MDLL_CP_LDO_BYPASS_I_SHIFT)
#define REG_XTAL_MDLL_CP_LDO_BYPASS_I(n)    BITFIELD_VAL(REG_XTAL_MDLL_CP_LDO_BYPASS_I, n)
#define REG_XTAL_ICORE_SEL_SHIFT            11
#define REG_XTAL_ICORE_SEL_MASK             (0x1F << REG_XTAL_ICORE_SEL_SHIFT)
#define REG_XTAL_ICORE_SEL(n)               BITFIELD_VAL(REG_XTAL_ICORE_SEL, n)

// REG_50C
#define REG_XTAL_AMP_T_SHIFT                0
#define REG_XTAL_AMP_T_MASK                 (0x7 << REG_XTAL_AMP_T_SHIFT)
#define REG_XTAL_AMP_T(n)                   BITFIELD_VAL(REG_XTAL_AMP_T, n)
#define REG_XTAL_KICK_DRV_SEL_DR            (1 << 3)
#define REG_XTAL_AMP_DR                     (1 << 4)
#define REG_XTAL_DIV_SEL_IN_DR              (1 << 5)
#define REG_BT_XTAL_CMOM_DR                 (1 << 6)
#define REG_XTAL_LDO_VTUNE_OTHER_SHIFT      7
#define REG_XTAL_LDO_VTUNE_OTHER_MASK       (0x7 << REG_XTAL_LDO_VTUNE_OTHER_SHIFT)
#define REG_XTAL_LDO_VTUNE_OTHER(n)         BITFIELD_VAL(REG_XTAL_LDO_VTUNE_OTHER, n)
#define REG_XTAL_RCOSC_MODE_SHIFT           10
#define REG_XTAL_RCOSC_MODE_MASK            (0x3 << REG_XTAL_RCOSC_MODE_SHIFT)
#define REG_XTAL_RCOSC_MODE(n)              BITFIELD_VAL(REG_XTAL_RCOSC_MODE, n)
#define REG_BT_TXON_DR                      (1 << 12)
#define REG_BT_TXON                         (1 << 13)
#define REG_BT_RXON_DR                      (1 << 14)
#define REG_BT_RXON                         (1 << 15)

// REG_50D
#define REG_XTAL_BUF_RC_SSC_OTHER_SHIFT     0
#define REG_XTAL_BUF_RC_SSC_OTHER_MASK      (0x3F << REG_XTAL_BUF_RC_SSC_OTHER_SHIFT)
#define REG_XTAL_BUF_RC_SSC_OTHER(n)        BITFIELD_VAL(REG_XTAL_BUF_RC_SSC_OTHER, n)
#define REG_XTAL_MDLL_MULT_SHIFT            6
#define REG_XTAL_MDLL_MULT_MASK             (0x7 << REG_XTAL_MDLL_MULT_SHIFT)
#define REG_XTAL_MDLL_MULT(n)               BITFIELD_VAL(REG_XTAL_MDLL_MULT, n)
#define REG_XTAL_PO_DELAY_CTRL_SHIFT        9
#define REG_XTAL_PO_DELAY_CTRL_MASK         (0x3 << REG_XTAL_PO_DELAY_CTRL_SHIFT)
#define REG_XTAL_PO_DELAY_CTRL(n)           BITFIELD_VAL(REG_XTAL_PO_DELAY_CTRL, n)
#define REG_XTAL_PO_FINE_DELAY_CTRL_SHIFT   11
#define REG_XTAL_PO_FINE_DELAY_CTRL_MASK    (0xF << REG_XTAL_PO_FINE_DELAY_CTRL_SHIFT)
#define REG_XTAL_PO_FINE_DELAY_CTRL(n)      BITFIELD_VAL(REG_XTAL_PO_FINE_DELAY_CTRL, n)
#define REG_XTAL_MDLL_CLK_ADC_SEL_I_1M      (1 << 15)

// REG_512
#define REG_XTAL_BUF_RC_CH70_SHIFT          0
#define REG_XTAL_BUF_RC_CH70_MASK           (0x3F << REG_XTAL_BUF_RC_CH70_SHIFT)
#define REG_XTAL_BUF_RC_CH70(n)             BITFIELD_VAL(REG_XTAL_BUF_RC_CH70, n)
#define REG_XTAL_BUF_RC_SSC_CH70_SHIFT      6
#define REG_XTAL_BUF_RC_SSC_CH70_MASK       (0x3F << REG_XTAL_BUF_RC_SSC_CH70_SHIFT)
#define REG_XTAL_BUF_RC_SSC_CH70(n)         BITFIELD_VAL(REG_XTAL_BUF_RC_SSC_CH70, n)
#define REG_XTAL_STARTUP_RELEASE_EN         (1 << 12)
#define REG_XTAL_MDLL_RSTB_DR               (1 << 14)
#define REG_XTAL_MDLL_RSTB                  (1 << 15)

// REG_514
#define DIG_BT_RCOSC_CAL_CNT_SHIFT          0
#define DIG_BT_RCOSC_CAL_CNT_MASK           (0x7FFF << DIG_BT_RCOSC_CAL_CNT_SHIFT)
#define DIG_BT_RCOSC_CAL_CNT(n)             BITFIELD_VAL(DIG_BT_RCOSC_CAL_CNT, n)
#define DIG_BT_RCOSC_CAL_DONE               (1 << 15)

// REG_525
#define REG_BT_RCOSC_CAL_RESETN             (1 << 0)
#define REG_BT_RCOSC_CAL_START              (1 << 1)
#define REG_BT_RCOSC_CAL_TIME_SEL_SHIFT     2
#define REG_BT_RCOSC_CAL_TIME_SEL_MASK      (0x7 << REG_BT_RCOSC_CAL_TIME_SEL_SHIFT)
#define REG_BT_RCOSC_CAL_TIME_SEL(n)        BITFIELD_VAL(REG_BT_RCOSC_CAL_TIME_SEL, n)

enum RF_REG_T {
    RF_REG_500     = 0x500,
    RF_REG_501,
    RF_REG_502,
    RF_REG_503,
    RF_REG_504,
    RF_REG_505,
    RF_REG_506,
    RF_REG_507,
    RF_REG_508,
    RF_REG_509,
    RF_REG_50A,
    RF_REG_50B,
    RF_REG_50C,
    RF_REG_50D,
    RF_REG_50E,
    RF_REG_50F,
    RF_REG_510,
    RF_REG_511,
    RF_REG_512,
    RF_REG_513,
    RF_REG_514,
    RF_REG_515,
    RF_REG_516,
    RF_REG_517,
    RF_REG_518,
    RF_REG_519,
    RF_REG_51A,
    RF_REG_51B,
    RF_REG_51C,
    RF_REG_51D,
    RF_REG_51E,
    RF_REG_51F,
    RF_REG_520,
    RF_REG_521,
    RF_REG_522,
    RF_REG_523,
    RF_REG_524,
    RF_REG_525,
    RF_REG_526,
    RF_REG_527,
    RF_REG_528,
    RF_REG_529,
    RF_REG_52A,
    RF_REG_52B,
    RF_REG_52C,
    RF_REG_52D,
    RF_REG_52E,
    RF_REG_52F,
    RF_REG_530,
};

#endif
