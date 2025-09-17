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
#include "charger.h"
#include "pmu.h"
#include CHIP_SPECIFIC_HDR(reg_charger)
#include "analog.h"
#include "cmsis.h"
#include "cmsis_nvic.h"
#include "hal_aud.h"
#include "hal_bootmode.h"
#include "hal_cache.h"
#include "hal_chipid.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "patch.h"
#include "tgt_hardware.h"

#define CHG_IRQ_MODULE_ENABLE                               true
#define CHG_IRQ_MODULE_DISABLE                              false

#define CHG_STATUS_STABLE_TIME_US                           100

// PMU_EFUSE_PAGE_CHG_CAL_PART1
#define CHG_PMU_EFUSE_CHG_CAL_PART1_ICHARGE_RES_DR          15
#define CHG_PMU_EFUSE_CHG_CAL_PART1_ICHARGE_RES_SHIFT       12
#define CHG_PMU_EFUSE_CHG_CAL_PART1_ICHARGE_RES_MASK        (0x7 << CHG_PMU_EFUSE_CHG_CAL_PART1_ICHARGE_RES_SHIFT)
#define CHG_PMU_EFUSE_CHG_CAL_PART1_ICHARGE_RES(n)          BITFIELD_VAL(CHG_PMU_EFUSE_CHG_CAL_PART1_ICHARGE_RES, n)
#define CHG_PMU_EFUSE_CHG_CAL_PART1_VBG_DR                  11
#define CHG_PMU_EFUSE_CHG_CAL_PART1_VBG_SHIFT               5
#define CHG_PMU_EFUSE_CHG_CAL_PART1_VBG_MASK                (0x3F << CHG_PMU_EFUSE_CHG_CAL_PART1_VBG_SHIFT)
#define CHG_PMU_EFUSE_CHG_CAL_PART1_VBG(n)                  BITFIELD_VAL(CHG_PMU_EFUSE_CHG_CAL_PART1_VBG, n)
#define CHG_PMU_EFUSE_CHG_CAL_PART1_BGTEMP_DR               4
#define CHG_PMU_EFUSE_CHG_CAL_PART1_BGTEMP_SHIFT            0
#define CHG_PMU_EFUSE_CHG_CAL_PART1_BGTEMP_MASK             (0xF << CHG_PMU_EFUSE_CHG_CAL_PART1_BGTEMP_SHIFT)
#define CHG_PMU_EFUSE_CHG_CAL_PART1_BGTEMP(n)               BITFIELD_VAL(CHG_PMU_EFUSE_CHG_CAL_PART1_BGTEMP, n)

// PMU_EFUSE_PAGE_CHG_CAL_PART2
#define CHG_PMU_EFUSE_CHG_CAL_PART2_VOREG_DR                13
#define CHG_PMU_EFUSE_CHG_CAL_PART2_VOREG_SHIFT             8
#define CHG_PMU_EFUSE_CHG_CAL_PART2_VOREG_MASK              (0x1F << CHG_PMU_EFUSE_CHG_CAL_PART2_VOREG_SHIFT)
#define CHG_PMU_EFUSE_CHG_CAL_PART2_VOREG(n)                BITFIELD_VAL(CHG_PMU_EFUSE_CHG_CAL_PART2_VOREG, n)
#define CHG_PMU_EFUSE_CHG_CAL_PART2_VREF_CC_DR              7
#define CHG_PMU_EFUSE_CHG_CAL_PART2_VREF_CC_SHIFT           4
#define CHG_PMU_EFUSE_CHG_CAL_PART2_VREF_CC_MASK            (0x7 << CHG_PMU_EFUSE_CHG_CAL_PART2_VREF_CC_SHIFT)
#define CHG_PMU_EFUSE_CHG_CAL_PART2_VREF_CC(n)              BITFIELD_VAL(CHG_PMU_EFUSE_CHG_CAL_PART2_VREF_CC, n)
#define CHG_PMU_EFUSE_CHG_CAL_PART2_ITRIM_RES_DR            3
#define CHG_PMU_EFUSE_CHG_CAL_PART2_ITRIM_RES_SHIFT         0
#define CHG_PMU_EFUSE_CHG_CAL_PART2_ITRIM_RES_MASK          (0x7 << CHG_PMU_EFUSE_CHG_CAL_PART2_ITRIM_RES_SHIFT)
#define CHG_PMU_EFUSE_CHG_CAL_PART2_ITRIM_RES(n)            BITFIELD_VAL(CHG_PMU_EFUSE_CHG_CAL_PART2_ITRIM_RES, n)

enum CHG_IRQ_MODULE_E {
    CHG_CHARGE_INTR     = 0,
    CHG_AC_ON_DET_IN    = 1,
    CHG_AC_ON_DET_OUT   = 2,
    CHG_TRIPRE_TIMEOUT  = 3,
    CHG_FAST_TIMEOUT    = 4,
    CHG_ACIN_OV         = 5,
    CHG_OTP             = 6,
    CHG_CHRG_TRICKLE    = 7,
    CHG_CHRG_PRE        = 8,
    CHG_CHRG_FAST       = 9,
    CHG_CHRG_OFF        = 10,
    CHG_CHRG_DONE       = 11,

    CHG_IRQ_MODULE_QTY,
};

struct CHG_IRQ_MODULE_RAW_MAP_T {
    enum CHG_IRQ_MODULE_E module;
    uint32_t raw;
};

struct CHG_IRQ_MODULE_CFG_T {
    unsigned short intr_clr;
    unsigned short intr_mask;
    unsigned short intr_en;
    unsigned short intr_raw;
};

#define CHG_IRQ_MOD_CFG_VAL(m)              { \
    REG_##m##_INTR_CLR, REG_##m##_INTR_MASK, \
    REG_##m##_INTR_EN, REG_##m##_INTR_RAW }

static const struct CHG_IRQ_MODULE_CFG_T chg_irq_module_cfg[] = {
    CHG_IRQ_MOD_CFG_VAL(CHARGE),
    CHG_IRQ_MOD_CFG_VAL(CHARGE_AC_ON_DET_IN),
    CHG_IRQ_MOD_CFG_VAL(CHARGE_AC_ON_DET_OUT),
    CHG_IRQ_MOD_CFG_VAL(CHARGE_TRIPRE_TIMEOUT),
    CHG_IRQ_MOD_CFG_VAL(CHARGE_FAST_TIMEOUT),
    CHG_IRQ_MOD_CFG_VAL(CHARGE_ACIN_OV),
    CHG_IRQ_MOD_CFG_VAL(CHARGE_OTP),
    CHG_IRQ_MOD_CFG_VAL(CHARGE_CHRG_TRICKLE),
    CHG_IRQ_MOD_CFG_VAL(CHARGE_CHRG_PRE),
    CHG_IRQ_MOD_CFG_VAL(CHARGE_CHRG_FAST),
    CHG_IRQ_MOD_CFG_VAL(CHARGE_CHRG_OFF),
    CHG_IRQ_MOD_CFG_VAL(CHARGE_CHRG_DONE),
};

static const struct CHG_IRQ_MODULE_RAW_MAP_T chg_irq_module_raw_map[] = {
    {CHG_AC_ON_DET_IN,   CHARGER_IRQ_CAUSE_AC_ON_DET_IN     },
    {CHG_AC_ON_DET_OUT,  CHARGER_IRQ_CAUSE_AC_ON_DET_OUT    },
    {CHG_TRIPRE_TIMEOUT, CHARGER_IRQ_CAUSE_TRIPRE_TIMEOUT   },
    {CHG_FAST_TIMEOUT,   CHARGER_IRQ_CAUSE_FAST_TIMEOUT     },
    {CHG_ACIN_OV,        CHARGER_IRQ_CAUSE_ACIN_OV          },
    {CHG_OTP,            CHARGER_IRQ_CAUSE_OTP              },
    {CHG_CHRG_TRICKLE,   CHARGER_IRQ_CAUSE_CHARGE_TRICKLE   },
    {CHG_CHRG_PRE,       CHARGER_IRQ_CAUSE_CHARGE_PRE       },
    {CHG_CHRG_FAST,      CHARGER_IRQ_CAUSE_CHARGE_FAST      },
    {CHG_CHRG_OFF,       CHARGER_IRQ_CAUSE_CHARGE_OFF       },
    {CHG_CHRG_DONE,      CHARGER_IRQ_CAUSE_CHARGE_DONE      },
};

static CHARGER_CHARGE_IRQ_HANDLER_T chg_irq_handler;

static enum HAL_CHIP_METAL_ID_T BOOT_BSS_LOC chg_metal_id;

#ifdef CHG_FORCE_LOW_CV_CURRENT
static bool chg_half_icc_busy = false;

static void charger_charge_rechage_enable(bool enable);
#endif

static bool BOOT_BSS_LOC chg_opened = false;

#ifdef CHG_FORCE_LOW_CV_CURRENT
static void charger_charge_cv_force_low_iterm_config_set(void)
{
    uint32_t lock;
    uint16_t val;
    uint16_t val_rechg_en;
    uint16_t val_icc_half;

    lock = int_lock();
    chg_read(CHG_REG_AC_IN_PP_CFG, &val_rechg_en);
    chg_read(CHG_REG_ICC_HALF_INTF_PMU, &val_icc_half);
    TRACE(0, "%s val_rechg_en=0x%04x val_icc_half=0x%04x", __func__, val_rechg_en, val_icc_half);
    if (val_rechg_en & REG_CHARGER_RECHARGE_EN) {
        TRACE(0, "OFF->DONE");
        val_rechg_en &= ~REG_CHARGER_RECHARGE_EN;
        chg_write(CHG_REG_AC_IN_PP_CFG, val_rechg_en);

        val_icc_half &= ~REG_NTC_HALF_ICC;
        chg_write(CHG_REG_ICC_HALF_INTF_PMU, val_icc_half);

        chg_half_icc_busy = false;
    } else {
        TRACE(0, "FAST->DONE");
        val_icc_half |= REG_NTC_HALF_ICC;
        chg_write(CHG_REG_ICC_HALF_INTF_PMU, val_icc_half);

        val_rechg_en |= REG_CHARGER_RECHARGE_EN;
        chg_write(CHG_REG_AC_IN_PP_CFG, val_rechg_en);

        chg_read(CHG_REG_STANDBY_SHIP_MODE, &val);
        val |= REG_DONE_SW_RECHARGE;
        chg_write(CHG_REG_STANDBY_SHIP_MODE, val);

        //  Avoid half icc be cleared by charger_charge_half_icc_current_disable().
        chg_half_icc_busy = true;
    }
    int_unlock(lock);
}
#endif

static inline uint16_t charger_charge_irq_module_addr_get(enum CHG_IRQ_MODULE_E module)
{
    if (module == CHG_AC_ON_DET_IN || module == CHG_AC_ON_DET_OUT) {
        return CHG_REG_AC_ON_DET_INTR_CFG;
    } else if (module == CHG_FAST_TIMEOUT || module == CHG_TRIPRE_TIMEOUT) {
        return CHG_REG_FAST_TRI_TIMEOUT_INTR_CFG;
    } else if (module == CHG_ACIN_OV || module == CHG_OTP) {
        return CHG_REG_OTP_ACIN_VSYS_SC_INTR_CFG;
    } else {
        return CHG_REG_MAIN_PU_CHG_STATE_INTR_EN;
    }
}

static void charger_charge_irq_module_clr(enum CHG_IRQ_MODULE_E module)
{
    unsigned short val;
    unsigned short module_address;
    const struct CHG_IRQ_MODULE_CFG_T *module_cfg_p = &chg_irq_module_cfg[module];

    if (module >= CHG_CHRG_TRICKLE) {
        module_address = CHG_REG_MAIN_PU_CHG_STATE_INTR_CLR;
    } else {
        module_address = charger_charge_irq_module_addr_get(module);
    }

    if (module >= CHG_CHRG_TRICKLE) {
        val = module_cfg_p->intr_clr;
    } else {
        chg_read(module_address, &val);
        val |= module_cfg_p->intr_clr;
    }
    chg_write(module_address, val);
}

static void charger_charge_irq_module_cfg(enum CHG_IRQ_MODULE_E module, int irq_en)
{
    unsigned short val;
    unsigned short module_address;
    const struct CHG_IRQ_MODULE_CFG_T *module_cfg_p = &chg_irq_module_cfg[module];

    if (module == CHG_IRQ_MODULE_QTY) {
        return;
    }

    module_address = charger_charge_irq_module_addr_get(module);

    if (module >= CHG_CHRG_TRICKLE) {
        chg_read(module_address, &val);
        if (irq_en) {
            val |= module_cfg_p->intr_en;
        } else {
            val &= ~module_cfg_p->intr_en;
        }
        chg_write(module_address, val);

        module_address = CHG_REG_MAIN_PU_CHG_STATE_INTR_MASK;
        chg_read(module_address, &val);
        if (irq_en) {
            val |= module_cfg_p->intr_mask;
        } else {
            val &= ~module_cfg_p->intr_mask;
        }
        chg_write(module_address, val);
    } else {
        chg_read(module_address, &val);
        if (irq_en) {
            val |= module_cfg_p->intr_en | module_cfg_p->intr_mask;
        } else {
            val &= ~(module_cfg_p->intr_en | module_cfg_p->intr_mask);
        }
        chg_write(module_address, val);
    }
}

static void charger_charge_irq_handler(void)
{
    enum CHG_IRQ_MODULE_E irq_module = CHG_CHARGE_INTR;
    uint32_t lock;
    uint32_t irq_raw_total;
    uint16_t irq1_raw;
    uint16_t irq2_raw;

    lock = int_lock();
    chg_read(CHG_REG_CHG_INTR1, &irq1_raw);
    chg_read(CHG_REG_CHG_INTR2, &irq2_raw);

    irq1_raw &= ~CHRG_INTR;
    irq1_raw = GET_BITFIELD(irq1_raw, IRQ1_STATUS);
    irq2_raw = GET_BITFIELD(irq2_raw, IRQ2_STATUS);
    irq_raw_total = irq1_raw | irq2_raw << IRQ1_NUM;
    //TRACE(0, "%s irq_raw_total=0x%08X", __func__, irq_raw_total);

    for (int i = 0; i < ARRAY_SIZE(chg_irq_module_raw_map); i++) {
        if (!!(chg_irq_module_raw_map[i].raw & irq_raw_total)) {
            irq_module = chg_irq_module_raw_map[i].module;
            charger_charge_irq_module_clr(irq_module);
            //TRACE(0, "%s module=%d", __func__, irq_module);
            if (irq_module >= CHG_TRIPRE_TIMEOUT && irq_module <= CHG_OTP) {
                TRACE(0, "%s ##FAULT## module=%d", __func__, irq_module);
            }
        }
    }
    int_unlock(lock);

#ifdef CHG_FORCE_LOW_CV_CURRENT
    if ((irq_raw_total & (CHARGER_IRQ_CAUSE_AC_ON_DET_IN | CHARGER_IRQ_CAUSE_AC_ON_DET_OUT)) && chg_half_icc_busy) {
        // Reset force low CV cunrrent status.
        chg_half_icc_busy = false;
        charger_charge_half_icc_current_disable();
        charger_charge_rechage_enable(false);
    }
    if (irq_raw_total & CHARGER_IRQ_CAUSE_CHARGE_DONE) {
        charger_charge_cv_force_low_iterm_config_set();
    }
#endif

    if (chg_irq_handler) {
        chg_irq_handler(irq_raw_total);
    }
}

static void charger_charge_pre_voltage_set(uint16_t pre_volt_new, enum CHARGER_CHARGE_STATUE_E status)
{
    uint16_t val_read;
    uint16_t val;

    chg_read(CHG_REG_VRECHG_ITERM_PRECHG_V_CFG, &val_read);
    val = GET_BITFIELD(val_read, CHARGER_VREF_PRECHARGE_SEL);
    if ((status == CHARGER_CHARGE_STATUS_TRICKLE || status == CHARGER_CHARGE_STATUS_PRE) && val < pre_volt_new) {
        while (val++ < pre_volt_new) {
            val_read = SET_BITFIELD(val_read, CHARGER_VREF_PRECHARGE_SEL, val);
            chg_write(CHG_REG_VRECHG_ITERM_PRECHG_V_CFG, val_read);
        }
    } else {
        val_read = SET_BITFIELD(val_read, CHARGER_VREF_PRECHARGE_SEL, pre_volt_new);
        chg_write(CHG_REG_VRECHG_ITERM_PRECHG_V_CFG, val_read);
    }
}

static void charger_charge_pre_current_set(uint16_t pre_current_new, enum CHARGER_CHARGE_STATUE_E status)
{
    uint16_t val_read;
    uint16_t val;

    chg_read(CHG_REG_CC_PRE_CV_CFG, &val_read);
    val = GET_BITFIELD(val_read, REG_CHARGER_IPRE_SEL);
    if ((status == CHARGER_CHARGE_STATUS_TRICKLE || status == CHARGER_CHARGE_STATUS_PRE) && val < pre_current_new) {
        while (val++ < pre_current_new) {
            val_read = SET_BITFIELD(val_read, REG_CHARGER_IPRE_SEL, val);
            chg_write(CHG_REG_CC_PRE_CV_CFG, val_read);
        }
    } else {
        val_read = SET_BITFIELD(val_read, REG_CHARGER_IPRE_SEL, pre_current_new);
        chg_write(CHG_REG_CC_PRE_CV_CFG, val_read);
    }
}

static void charger_charge_cc_current_set(uint16_t cc_current_new, enum CHARGER_CHARGE_STATUE_E status)
{
    uint16_t val_read;
    uint16_t val;

    chg_read(CHG_REG_CC_PRE_CV_CFG, &val_read);
    val = GET_BITFIELD(val_read, REG_CHARGER_ICC_SEL);
    if ((status == CHARGER_CHARGE_STATUS_PRE || status == CHARGER_CHARGE_STATUS_FAST) && val < cc_current_new) {
        while (val++ < cc_current_new) {
            val_read = SET_BITFIELD(val_read, REG_CHARGER_ICC_SEL, val);
            chg_write(CHG_REG_CC_PRE_CV_CFG, val_read);
        }
    } else {
        val_read = SET_BITFIELD(val_read, REG_CHARGER_ICC_SEL, cc_current_new);
        chg_write(CHG_REG_CC_PRE_CV_CFG, val_read);
    }
}

static void charger_charge_stop_current_set(uint16_t stop_current_new, enum CHARGER_CHARGE_STATUE_E status)
{
    uint16_t val_read;
    uint16_t val;

    chg_read(CHG_REG_VRECHG_ITERM_PRECHG_V_CFG, &val_read);
    val = GET_BITFIELD(val_read, CHARGER_ITERM_SEL);
    if ((status == CHARGER_CHARGE_STATUS_FAST || status == CHARGER_CHARGE_STATUS_DONE) && val < stop_current_new) {
        while (val++ < stop_current_new) {
            val_read = SET_BITFIELD(val_read, CHARGER_ITERM_SEL, val);
            chg_write(CHG_REG_VRECHG_ITERM_PRECHG_V_CFG, val_read);
        }
    } else {
        val_read = SET_BITFIELD(val_read, CHARGER_ITERM_SEL, stop_current_new);
        chg_write(CHG_REG_VRECHG_ITERM_PRECHG_V_CFG, val_read);
    }
}

static void charger_charge_cv_voltage_set(uint16_t cv_volt_new, enum CHARGER_CHARGE_STATUE_E status)
{
    uint16_t val_read;
    uint16_t val;

    chg_read(CHG_REG_CC_PRE_CV_CFG, &val_read);
    val = GET_BITFIELD(val_read, REG_CHARGER_VOREG_BIT);
    if ((status == CHARGER_CHARGE_STATUS_FAST || status == CHARGER_CHARGE_STATUS_DONE) && val < cv_volt_new) {
        while (val++ < cv_volt_new) {
            val_read = SET_BITFIELD(val_read, REG_CHARGER_VOREG_BIT, val);
            chg_write(CHG_REG_CC_PRE_CV_CFG, val_read);
        }
    } else {
        val_read = SET_BITFIELD(val_read, REG_CHARGER_VOREG_BIT, cv_volt_new);
        chg_write(CHG_REG_CC_PRE_CV_CFG, val_read);
    }
}

static void charger_charge_recharge_voltage_set(uint16_t rechg_volt)
{
    uint16_t val;

    chg_read(CHG_REG_VRECHG_ITERM_PRECHG_V_CFG, &val);
    val= SET_BITFIELD(val, CHARGER_VRECHARGE_SEL, rechg_volt);
    chg_write(CHG_REG_VRECHG_ITERM_PRECHG_V_CFG, val);
}

static void charger_charge_rechage_enable(bool enable)
{
    uint16_t val;

    chg_read(CHG_REG_AC_IN_PP_CFG, &val);
    if (enable) {
        val |= REG_CHARGER_RECHARGE_EN;
    } else {
        val &= ~REG_CHARGER_RECHARGE_EN;
    }
    chg_write(CHG_REG_AC_IN_PP_CFG, val);
}

#ifdef ACIN_PATTERN_DISABLE
void charger_charge_pattern_disable(void)
{
    uint16_t val;

    chg_read(CHG_PATTERN0_RST_CHARGER_CFG, &val);
    val &= ~REG_COMMAND_0_ENABLE;
    chg_write(CHG_PATTERN0_RST_CHARGER_CFG, val);

    chg_read(CHG_PATTERN3_SINGLE_UART_ON_CFG, &val);
    val &= ~REG_COMMAND_3_ENABLE;
    chg_write(CHG_PATTERN3_SINGLE_UART_ON_CFG, val);

    chg_read(CHG_PATTERN4_SINGLE_UART_OFF_CFG, &val);
    val &= ~REG_COMMAND_4_ENABLE;
    chg_write(CHG_PATTERN4_SINGLE_UART_OFF_CFG, val);
}
#endif

void charger_charge_irq_handler_set(CHARGER_CHARGE_IRQ_HANDLER_T handler)
{
    uint32_t lock;

    if (handler) {
        lock = int_lock();
        chg_irq_handler = handler;
        int_unlock(lock);
    }
}

enum CHARGER_CHARGE_STATUE_E charger_charge_status_get(void)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    chg_read(CHG_REG_CHG_STATUS3, &val);
    val= GET_BITFIELD(val, CHARGE_STATE);
    int_unlock(lock);

    return (enum CHARGER_CHARGE_STATUE_E)val;
}

void charger_charge_module_cfg_get(struct CHARGER_CHARGE_MODULE_CFG_T *cfg)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    chg_read(CHG_REG_CC_PRE_CV_CFG, &val);
    cfg->cv_volt = GET_BITFIELD(val, REG_CHARGER_VOREG_BIT);
    cfg->prechg_current = GET_BITFIELD(val, REG_CHARGER_IPRE_SEL);
    cfg->cc_current = GET_BITFIELD(val, REG_CHARGER_ICC_SEL);

    chg_read(CHG_REG_VRECHG_ITERM_PRECHG_V_CFG, &val);
    cfg->stop_current = GET_BITFIELD(val, CHARGER_ITERM_SEL);
    cfg->rechg_volt = GET_BITFIELD(val, CHARGER_VRECHARGE_SEL);
    cfg->prechg_volt = GET_BITFIELD(val, CHARGER_VREF_PRECHARGE_SEL);

    chg_read(CHG_REG_CHG_FUNC_CFG, &val);
    cfg->chg_en = !!(val & REG_CHARGER_CHARGE_EN);

    chg_read(CHG_REG_AC_IN_PP_CFG, &val);
    cfg->rechg_en = !!(val & REG_CHARGER_RECHARGE_EN);
    int_unlock(lock);
}

int charger_charge_module_cfg_set(struct CHARGER_CHARGE_MODULE_CFG_T *cfg)
{
    enum CHARGER_CHARGE_STATUE_E status;
    uint32_t lock;

    if (cfg->prechg_current >= CHARGER_CHARGE_PRECHARGE_CURRENT_QTY) {
        return 1;
    }

    if (cfg->cc_current >= CHARGER_CHARGE_CONSTANT_CURRENT_QTY) {
        return 2;
    }

    if (cfg->stop_current >= CHARGER_CHARGE_STOP_CURRENT_QTY) {
        return 3;
    }

    if (cfg->cv_volt >= CHARGER_CHARGING_CONSTANT_VOLTAGE_QTY) {
        return 4;
    }

    if (cfg->rechg_volt >= CHARGER_CHARGE_RECHARGE_VOLTAGE_QTY) {
        return 5;
    }

    if (cfg->prechg_volt >= CHARGER_CHARGE_PRECHARGE_VOLTAGE_QTY) {
        return 6;
    }

    lock = int_lock();
    status = charger_charge_status_get();

    charger_charge_pre_voltage_set(cfg->prechg_volt, status);
    charger_charge_pre_current_set(cfg->prechg_current, status);
    charger_charge_cc_current_set(cfg->cc_current, status);
    charger_charge_stop_current_set(cfg->stop_current, status);
    charger_charge_cv_voltage_set(cfg->cv_volt, status);
    charger_charge_recharge_voltage_set(cfg->rechg_volt);

#ifndef CHG_FORCE_LOW_CV_CURRENT
    // Set rechg_en by charger irq handler only.
    charger_charge_rechage_enable(cfg->rechg_en);
#endif

    if (cfg->chg_en) {
        charger_charge_enable();
    } else {
        charger_charge_disable();
    }
    int_unlock(lock);

    return 0;
}

static void charger_charge_efuse_calib_load(void)
{
    uint16_t val_efuse;
    uint16_t val_efuse2;
    uint16_t val;

    pmu_get_efuse(PMU_EFUSE_PAGE_CHG_CAL_PART1, &val_efuse);
    pmu_get_efuse(PMU_EFUSE_PAGE_CHG_CAL_PART2, &val_efuse2);

    TRACE(0, "%s val_efuse:0x%x val_efuse2:0x%x", __func__, val_efuse, val_efuse2);

    // bgtem --> vbg --> vbg_iset --> voreg --> icc --> iterm
    if (val_efuse && val_efuse2) {
        charger_charge_disable();

        // bgtemp
        chg_read(CHG_REG_EFUSE_CFG_20, &val);
        val |= EFUSE_TRIM_BGTEMP_DR;
        val = SET_BITFIELD(val, EFUSE_TRIM_BGTEMP, GET_BITFIELD(val_efuse, CHG_PMU_EFUSE_CHG_CAL_PART1_BGTEMP));
        chg_write(CHG_REG_EFUSE_CFG_20, val);
        hal_sys_timer_delay_us(CHG_STATUS_STABLE_TIME_US);

        // vbg
        val |= EFUSE_TRIM_VBG_DR;
        val = SET_BITFIELD(val, EFUSE_TRIM_VBG, GET_BITFIELD(val_efuse, CHG_PMU_EFUSE_CHG_CAL_PART1_VBG));
        chg_write(CHG_REG_EFUSE_CFG_20, val);
        hal_sys_timer_delay_us(CHG_STATUS_STABLE_TIME_US);

        // vbg_iset
        chg_read(CHG_REG_EFUSE_CFG_23, &val);
        val |= EFUSE_TRIM_VREF_CC_DR;
        val = SET_BITFIELD(val, EFUSE_TRIM_VREF_CC, GET_BITFIELD(val_efuse2, CHG_PMU_EFUSE_CHG_CAL_PART2_VREF_CC));
        chg_write(CHG_REG_EFUSE_CFG_23, val);
        hal_sys_timer_delay_us(CHG_STATUS_STABLE_TIME_US);

        // voreg
        val |= EFUSE_TRIM_VOREG_DR;
        val = SET_BITFIELD(val, EFUSE_TRIM_VOREG, GET_BITFIELD(val_efuse2, CHG_PMU_EFUSE_CHG_CAL_PART2_VOREG));
        chg_write(CHG_REG_EFUSE_CFG_23, val);
        hal_sys_timer_delay_us(CHG_STATUS_STABLE_TIME_US);

        // icc
        chg_read(CHG_REG_EFUSE_CFG_22, &val);
        val |= EFUSE_TRIM_ICHARGE_RES_DR;
        val = SET_BITFIELD(val, EFUSE_TRIM_ICHARGE_RES, GET_BITFIELD(val_efuse, CHG_PMU_EFUSE_CHG_CAL_PART1_ICHARGE_RES));
        chg_write(CHG_REG_EFUSE_CFG_22, val);
        hal_sys_timer_delay_us(CHG_STATUS_STABLE_TIME_US);

        // iterm
        val |= EFUSE_TRIM_ITERM_RES_DR;
        val = SET_BITFIELD(val, EFUSE_TRIM_ITERM_RES, GET_BITFIELD(val_efuse2, CHG_PMU_EFUSE_CHG_CAL_PART2_ITRIM_RES));
        chg_write(CHG_REG_EFUSE_CFG_22, val);
        hal_sys_timer_delay_us(CHG_STATUS_STABLE_TIME_US);

        charger_charge_enable();
    }
}

void charger_charge_open(void)
{
    uint32_t lock;
    uint16_t val;

    if (chg_opened) {
        return;
    }

    chg_opened = true;

    TRACE(0, "%s chg_metal_id=%d", __func__, chg_metal_id);

    lock = int_lock();

    // Disable and clear all CHG irqs by default
    // CHG irqs cannot be cleared by CHG soft reset
    val = REG_AC_ON_DET_IN_INTR_CLR | REG_AC_ON_DET_OUT_INTR_CLR;
    chg_write(CHG_REG_AC_ON_DET_INTR_CFG, val);

    val = REG_CHARGE_TRIPRE_TIMEOUT_INTR_CLR | REG_CHARGE_FAST_TIMEOUT_INTR_CLR;
    chg_write(CHG_REG_FAST_TRI_TIMEOUT_INTR_CFG, val);

    val = REG_ACIN_OV_INTR_CLR | REG_OTP_INTR_CLR;
    chg_write(CHG_REG_OTP_ACIN_VSYS_SC_INTR_CFG, val);

    val = MAIN_STATE_INTR_CLR_MASK | PU_STATE_INTR_CLR_MASK | CHRG_STATE_INTR_CLR_MASK;
    chg_write(CHG_REG_MAIN_PU_CHG_STATE_INTR_CLR, val);

    // Charge_en will be disabled in charger_charge_efuse_calib_load()
    charger_charge_efuse_calib_load();

    // Improve CV stop current stability.
    chg_read(CHG_REG_CV_CFG_07, &val);
    val= SET_BITFIELD(val, CHARGER_CV_GM_SEL, 0x4);
    val= SET_BITFIELD(val, CHARGER_CV_COMP_RES, 0x0);
    val= SET_BITFIELD(val, CHARGER_CV_COMP_CAP, 0x7);
    chg_write(CHG_REG_CV_CFG_07, val);

    chg_write(CHG_REG_ITERM_POS_DB_CNT_CFG, 0x3);

    chg_read(CHG_REG_ITERM_POS_DB_CNT2_CFG, &val);
    val= SET_BITFIELD(val, REG_ITERM_POS_DB_CNT_20_16, 0x0);
    chg_write(CHG_REG_ITERM_POS_DB_CNT2_CFG, val);

#ifdef CHG_FORCE_LOW_CV_CURRENT
    charger_charge_rechage_enable(false);
#endif

#ifdef ACIN_PATTERN_DISABLE
    charger_charge_pattern_disable();
#endif

    charger_charge_irq_module_cfg(CHG_TRIPRE_TIMEOUT, CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_FAST_TIMEOUT,   CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_ACIN_OV,        CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_OTP,            CHG_IRQ_MODULE_ENABLE);
    charger_charge_irq_module_cfg(CHG_CHRG_DONE,      CHG_IRQ_MODULE_ENABLE);

    charger_charge_irq_module_cfg(CHG_AC_ON_DET_IN,   CHG_IRQ_MODULE_DISABLE);
    charger_charge_irq_module_cfg(CHG_AC_ON_DET_OUT,  CHG_IRQ_MODULE_DISABLE);
    charger_charge_irq_module_cfg(CHG_CHRG_TRICKLE,   CHG_IRQ_MODULE_DISABLE);
    charger_charge_irq_module_cfg(CHG_CHRG_PRE,       CHG_IRQ_MODULE_DISABLE);
    charger_charge_irq_module_cfg(CHG_CHRG_FAST,      CHG_IRQ_MODULE_DISABLE);
    charger_charge_irq_module_cfg(CHG_CHRG_OFF,       CHG_IRQ_MODULE_DISABLE);

    NVIC_SetVector(CHARGER_IRQn, (uint32_t)charger_charge_irq_handler);
    NVIC_SetPriority(CHARGER_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(CHARGER_IRQn);
    NVIC_EnableIRQ(CHARGER_IRQn);

    int_unlock(lock);
}

void charger_charge_enable(void)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    chg_read(CHG_REG_CHG_FUNC_CFG, &val);
    val |= REG_CHARGER_CHARGE_EN_DR | REG_CHARGER_CHARGE_EN;
    chg_write(CHG_REG_CHG_FUNC_CFG, val);
    int_unlock(lock);
}

void charger_charge_disable(void)
{
    uint32_t lock;
    uint16_t val;

    lock = int_lock();
    chg_read(CHG_REG_CHG_FUNC_CFG, &val);
    val = (val | REG_CHARGER_CHARGE_EN_DR) & ~REG_CHARGER_CHARGE_EN;
    chg_write(CHG_REG_CHG_FUNC_CFG, val);
    int_unlock(lock);
}

enum CHARGER_ACIN_STATUS_TYPE_E charger_acin_status_get(void)
{
    enum CHARGER_ACIN_STATUS_TYPE_E status = CHARGER_ACIN_STATUS_PLUGOUT;
    uint16_t val;

    chg_read(CHG_REG_CHG_STATUS1, &val);
    if (!!(val & AC_ON_DB)) {
        // db_time=16ms
        status = CHARGER_ACIN_STATUS_PLUGIN;
    }

    return status;
}

void charger_charge_half_icc_current_enable(void)
{
    uint32_t lock;
    uint16_t val;

#ifdef CHG_FORCE_LOW_CV_CURRENT
    if (chg_half_icc_busy) {
        return;
    }
#endif

    lock = int_lock();
    chg_read(CHG_REG_ICC_HALF_INTF_PMU, &val);
    val |= REG_NTC_HALF_ICC;
    chg_write(CHG_REG_ICC_HALF_INTF_PMU, val);
    int_unlock(lock);
}

void charger_charge_half_icc_current_disable(void)
{
    uint32_t lock;
    uint16_t val;

#ifdef CHG_FORCE_LOW_CV_CURRENT
    if (chg_half_icc_busy) {
        return;
    }
#endif

    lock = int_lock();
    chg_read(CHG_REG_ICC_HALF_INTF_PMU, &val);
    val &= ~REG_NTC_HALF_ICC;
    chg_write(CHG_REG_ICC_HALF_INTF_PMU, val);
    int_unlock(lock);
}
