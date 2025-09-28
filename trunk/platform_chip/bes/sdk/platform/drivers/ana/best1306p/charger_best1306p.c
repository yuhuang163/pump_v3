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
#include "pmu.h"
#include CHIP_SPECIFIC_HDR(reg_charger)
#include CHIP_SPECIFIC_HDR(charger)
#include "cmsis.h"
#include "cmsis_nvic.h"
#include "hal_chipid.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_i2c.h"
#include "tgt_hardware.h"

#define NO_METAL_ID_0

#define CHARGER_1622_I2C_DEV_ADDR               0x1A
#define CHARGER_I2C_RETRY_CNT                   3

#define CHARGER_BANDGAP_STABLE_TIME_US          100

#if (CHARGER_1620_PIN_INDEX == 20)
#define CHARGER_1620_PIN_CHGEN_INDEX            HAL_IOMUX_PIN_P2_0  // Package name is CHGENB
#define CHARGER_1620_PIN_DONE_DET_INDEX         HAL_IOMUX_PIN_P2_1  // Package name is CHGB
#else
#define CHARGER_1620_PIN_CHGEN_INDEX            HAL_IOMUX_PIN_NUM
#define CHARGER_1620_PIN_DONE_DET_INDEX         HAL_IOMUX_PIN_NUM
#endif

#define CHARGER_1620_IRQ_PIN                    CHARGER_1620_PIN_DONE_DET_INDEX
#define CHARGER_1622_IRQ_PIN                    HAL_IOMUX_PIN_P4_7

#define CHARGER_1622_I2C_ID                     HAL_I2C_ID_1

#define PMU_EFUSE_CHARGER_CHIP_TYPE_1622        (1 << 12)

// CHARGER_EFUSE_PAGE_0
#define CHARGE_EFUSE_BAT_CL_IBIT_BIT_0          (1 << 0)
#define CHARGE_EFUSE_CL_VOS_BIT_COMP_SHIFT      1
#define CHARGE_EFUSE_CL_VOS_BIT_COMP_MASK       (0x3 << CHARGE_EFUSE_CL_VOS_BIT_COMP_SHIFT)
#define CHARGE_EFUSE_CL_VOS_BIT_COMP(n)         BITFIELD_VAL(CHARGE_EFUSE_CL_VOS_BIT_COMP, n)
#define CHARGE_EFUSE_CL_VOS_BIT_COMP_SIGN       (1 << 3)
#define CHARGE_EFUSE_BAT_OC_SIFT1               (1 << 2)
#define CHARGE_EFUSE_BAT_OC_SIFT2               (1 << 3)
#define CHARGE_EFUSE_TRIM_CC_SHIFT              4
#define CHARGE_EFUSE_TRIM_CC_MASK               (0x1F << CHARGE_EFUSE_TRIM_CC_SHIFT)
#define CHARGE_EFUSE_TRIM_CC(n)                 BITFIELD_VAL(CHARGE_EFUSE_TRIM_CC, n)
#define CHARGE_EFUSE_TRIM_VOREG_SHIFT           9
#define CHARGE_EFUSE_TRIM_VOREG_MASK            (0xF << CHARGE_EFUSE_TRIM_VOREG_SHIFT)
#define CHARGE_EFUSE_TRIM_VOREG(n)              BITFIELD_VAL(CHARGE_EFUSE_TRIM_VOREG, n)
#define CHARGE_EFUSE_BG_TEMP_TRIM_COMP_SHIFT    13
#define CHARGE_EFUSE_BG_TEMP_TRIM_COMP_MASK     (0x3 << CHARGE_EFUSE_BG_TEMP_TRIM_COMP_SHIFT)
#define CHARGE_EFUSE_BG_TEMP_TRIM_COMP(n)       BITFIELD_VAL(CHARGE_EFUSE_BG_TEMP_TRIM_COMP, n)
#define CHARGE_EFUSE_BG_TEMP_TRIM_COMP_SIGN     (1 << 15)
#define CHARGE_EFUSE_BG_TRIM_L_SHIFT            13
#define CHARGE_EFUSE_BG_TRIM_L_MASK             (0x7 << CHARGE_EFUSE_BG_TRIM_L_SHIFT)
#define CHARGE_EFUSE_BG_TRIM_L(n)               BITFIELD_VAL(CHARGE_EFUSE_BG_TRIM_L, n)

// CHARGER_EFUSE_PAGE_1
#define CHARGE_EFUSE_BG_TRIM_COMP_SHIFT         8
#define CHARGE_EFUSE_BG_TRIM_COMP_MASK          (0x3 << CHARGE_EFUSE_BG_TRIM_COMP_SHIFT)
#define CHARGE_EFUSE_BG_TRIM_COMP(n)            BITFIELD_VAL(CHARGE_EFUSE_BG_TRIM_COMP, n)
#define CHARGE_EFUSE_BG_TRIM_COMP_SIGN          (1 << 10)
#define CHARGE_EFUSE_BG_TRIM_H_SHIFT            8
#define CHARGE_EFUSE_BG_TRIM_H_MASK             (0x7 << CHARGE_EFUSE_BG_TRIM_H_SHIFT)
#define CHARGE_EFUSE_BG_TRIM_H(n)               BITFIELD_VAL(CHARGE_EFUSE_BG_TRIM_H, n)
#define CHARGE_EFUSE_IPRE_TRIM_COMP_SHIFT       11
#define CHARGE_EFUSE_IPRE_TRIM_COMP_MASK        (0x3 << CHARGE_EFUSE_IPRE_TRIM_COMP_SHIFT)
#define CHARGE_EFUSE_IPRE_TRIM_COMP(n)          BITFIELD_VAL(CHARGE_EFUSE_IPRE_TRIM_COMP, n)
#define CHARGE_EFUSE_IPRE_TRIM_COMP_SIGN        (1 << 13)
#define CHARGE_EFUSE_VBG_CAL_V2_FLAG            (1 << 14)

// METAL ID > 1
#define CHARGE_EFUSE_NEW_BAT_CL_IBIT_BIT_1      (1 << 4)
#define CHARGE_EFUSE_NEW_TRIM_CC_SHIFT          5
#define CHARGE_EFUSE_NEW_TRIM_CC_MASK           (0x1F << CHARGE_EFUSE_NEW_TRIM_CC_SHIFT)
#define CHARGE_EFUSE_NEW_TRIM_CC(n)             BITFIELD_VAL(CHARGE_EFUSE_NEW_TRIM_CC, n)
#define CHARGE_EFUSE_NEW_TRIM_VOREG_SHIFT       10
#define CHARGE_EFUSE_NEW_TRIM_VOREG_MASK        (0xF << CHARGE_EFUSE_NEW_TRIM_VOREG_SHIFT)
#define CHARGE_EFUSE_NEW_TRIM_VOREG(n)          BITFIELD_VAL(CHARGE_EFUSE_NEW_TRIM_VOREG, n)
#define CHARGE_EFUSE_NEW_IPRE_TRIM_COMP_SHIFT   14
#define CHARGE_EFUSE_NEW_IPRE_TRIM_COMP_MASK    (0x3 << CHARGE_EFUSE_NEW_IPRE_TRIM_COMP_SHIFT)
#define CHARGE_EFUSE_NEW_IPRE_TRIM_COMP(n)      BITFIELD_VAL(CHARGE_EFUSE_NEW_IPRE_TRIM_COMP, n)

#define CHARGE_EFUSE_NEW_IPRE_TRIM_COMP_SIGN    (1 << 8)
#define CHARGE_EFUSE_NEW_BG_TRIM_SHIFT          9
#define CHARGE_EFUSE_NEW_BG_TRIM_MASK           (0xF << CHARGE_EFUSE_NEW_BG_TRIM_SHIFT)
#define CHARGE_EFUSE_NEW_BG_TRIM(n)             BITFIELD_VAL(CHARGE_EFUSE_NEW_BG_TRIM, n)
#define CHARGE_EFUSE_NEW_BG_TRIM_COMP_SIGN      (1 << 13)
#define CHARGE_EFUSE_NEW_METAL_ID               (1 << 15)

enum CHARGER_EFUSE_PAGE_E {
    CHARGER_EFUSE_PAGE_0,
    CHARGER_EFUSE_PAGE_1,
};

struct CHARGER_1620_CTX_T {
    enum CHARGER_CHARGE_PRE2CC_VOLTAGE_E pre2cc_volt;
    enum CHARGER_CHARGE_PRE2CC_CURRENT_E pre2cc_current;
    enum CHARGER_CHARGE_CONSTANT_CURRENT_E cc_current;
    enum CHARGER_CHARGE_STOP_CURRENT_E stop_curent;
    enum CHARGER_CHARGE_CONSTANT_VOLTAGE_E cv_volt;
    enum CHARGER_CHARGE_FORWARD_DELTA_VOLTAGE_E fr_volt;
    enum CHARGER_ACIN2VSYS_LIMIT_CURRENT_E limit_current;
    enum CHARGER_ACIN2VSYS_VSYS_VOLTAGE_E vsys_volt;
    enum CHARGER_ACIN2VSYS_VSYS_MIN_VOLTAGE_E vsys_min_volt;
    enum CHARGER_ACIN2VSYS_ACIN_MIN_VOLTAGE_E acin_min_volt;
};

static const struct CHARGER_1620_CTX_T chg_1620_ctx = {
    CHARGER_CHARGE_PRE2CC_VOLTAGE_2800MV,
    CHARGER_CHARGE_PRE2CC_CURRENT_5MA,
    CHARGER_CHARGE_CONSTANT_CURRENT_50MA,
    CHARGER_CHARGE_STOP_CURRENT_5MA,
    CHARGER_CHARGE_CONSTANT_VOLTAGE_4440MV,
    CHARGER_CHARGE_FORWARD_DELTA_VOLTAGE_175MV,
    CHARGER_ACIN2VSYS_LIMIT_CURRENT_500MA,
    CHARGER_ACIN2VSYS_VSYS_VOLTAGE_4600MV,
    CHARGER_ACIN2VSYS_VSYS_MIN_VOLTAGE_4600MV,
    CHARGER_ACIN2VSYS_ACIN_MIN_VOLTAGE_3600MV,
};

static bool charger_opened = false;
static const struct HAL_I2C_CONFIG_T i2c_cfg = {
    .mode = HAL_I2C_API_MODE_SIMPLE,
    .use_dma  = 0,
    .use_sync = 0,
    .speed = 400 * 1000,
    .as_master = 1,
    .rising_time_ns = 0,
    .addr_as_slave = 0,
};

static enum CHARGER_CHIP_TYPE_E chg_type;

static enum HAL_CHIP_METAL_ID_T chg_metal_id;

static CHARGER_CHARGE_IRQ_HANDLER_T chg_irq_handler;

static uint8_t bat_cl_org;
static uint8_t cl_vos_org;

static uint8_t bat_cl_plugin;
static uint8_t cl_vos_plugin;

static uint32_t i2cif_reg_read(uint8_t addr, uint16_t *val)
{
    uint32_t ret = 0xFF;
    uint8_t buf[3] = {0, };
    uint8_t retry_cnt = 0;

    if (val) {
        buf[0] = addr;

        do {
            ret = hal_i2c_simple_recv(CHARGER_1622_I2C_ID, CHARGER_1622_I2C_DEV_ADDR, buf, 1, &buf[1], 2);
            if (ret) {
                TRACE(0, "##### %s: error! ret=0x%x addr=0x%x retry_cnt=%d", __func__, ret, addr, retry_cnt);
                hal_i2c_close(CHARGER_1622_I2C_ID);
                hal_i2c_open(CHARGER_1622_I2C_ID, &i2c_cfg);
                retry_cnt++;
            }
        } while (retry_cnt < CHARGER_I2C_RETRY_CNT && ret);

        if (ret) {
            *val = 0xFFFF;
        } else {
            *val = buf[1] << 8 | buf[2];
        }
    }

    return ret;
}

static uint32_t i2cif_reg_write(uint8_t addr, uint16_t val)
{
    uint32_t ret;
    uint8_t buf[3] = {0, };
    uint8_t retry_cnt = 0;

    buf[0] = addr;
    buf[1] = (val & 0xFF00) >> 8;
    buf[2] = val & 0xFF;

    do {
        ret = hal_i2c_simple_send(CHARGER_1622_I2C_ID, CHARGER_1622_I2C_DEV_ADDR, buf, 3);
        if (ret) {
            TRACE(0, "##### %s: error! ret=0x%x addr=0x%x retry_cnt=%d", __func__, ret, addr, retry_cnt);
            hal_i2c_close(CHARGER_1622_I2C_ID);
            hal_i2c_open(CHARGER_1622_I2C_ID, &i2c_cfg);
            retry_cnt++;
        }
    } while (retry_cnt < CHARGER_I2C_RETRY_CNT && ret);

    return ret;
}

#ifdef CHARGER_TEST
uint32_t charger_reg_read(uint8_t addr, uint16_t *val)
{
    return i2cif_reg_read(addr, val);
}

uint32_t charger_reg_write(uint8_t addr, uint16_t val)
{
    return i2cif_reg_write(addr, val);
}
#endif

static void charger_1622_bus_init(void)
{
    uint32_t ret;

    ret = hal_i2c_open(CHARGER_1622_I2C_ID, &i2c_cfg);
    if (ret) {
        TRACE(0, "##### %s: fail, ret=%u", __func__, ret);
    }
}

static void charger_1622_bus_deinit(void)
{
    uint32_t ret;

    ret = hal_i2c_close(CHARGER_1622_I2C_ID);
    if (ret) {
        TRACE(0, "##### %s: fail, ret=%u", __func__, ret);
    }
}

static void charger_1620_bus_init(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pin_map[2] = {
        {CHARGER_1620_PIN_CHGEN_INDEX, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {CHARGER_1620_PIN_DONE_DET_INDEX, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
    };

    if (CHARGER_1620_PIN_CHGEN_INDEX != HAL_IOMUX_PIN_NUM) {
        for (int i = 0; i < ARRAY_SIZE(pin_map); i++) {
            hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&pin_map[i], 1);
        }
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pin_map[0].pin, HAL_GPIO_DIR_OUT, 0);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pin_map[1].pin, HAL_GPIO_DIR_IN, 0);
    } else {
        TRACE(0, "%s: ###Warning: No define CHARGER_1620_PIN_INDEX ?", __func__);
    }
}

static void charger_1620_bus_deinit(void)
{
// Do nothing.
}

static void charger_chip_type_match(void)
{
    uint16_t val;

    pmu_get_efuse(PMU_EFUSE_PAGE_EXT_GPADC_HV, &val);
    if (val & PMU_EFUSE_CHARGER_CHIP_TYPE_1622) {
        chg_type = CHARGER_CHIP_TYPE_1622;
    } else {
        chg_type = CHARGER_CHIP_TYPE_NONE;
    }
}

static void charger_bus_init(void)
{
    if (chg_type == CHARGER_CHIP_TYPE_1622) {
        charger_1622_bus_init();
    } else if (chg_type == CHARGER_CHIP_TYPE_1620) {
        charger_1620_bus_init();
    }
}

static void charger_bus_deinit(void)
{
    if (chg_type == CHARGER_CHIP_TYPE_1622) {
        charger_1622_bus_deinit();
    } else if (chg_type == CHARGER_CHIP_TYPE_1620) {
        charger_1620_bus_deinit();
    }
}

static void charger_gpio_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    uint32_t lock;
    uint32_t status = 0;
    uint16_t val = 0;
    uint16_t val_irq_raw;

    if (chg_type == CHARGER_CHIP_TYPE_1620) {
        status = CHARGER_IRQ_CAUSE_CHARGE_DONE;
    } else if (chg_type == CHARGER_CHIP_TYPE_1622) {
        lock = int_lock();
        i2cif_reg_read(CHG_REG_34, &val_irq_raw);
        if (val_irq_raw & CHARGE_DONE_INTR) {
            status |= CHARGER_IRQ_CAUSE_CHARGE_DONE;
            i2cif_reg_write(CHG_REG_28, (REG_CHARGE_DONE_INTR_EN | REG_CHARGE_DONE_INTR_CLR));
        }
        if (val_irq_raw & VIN_OV_INTR) {
            status |= CHARGER_IRQ_CAUSE_VIN_OV;
            i2cif_reg_read(CHG_REG_26, &val);
            val |= REG_VIN_OV_INTR_CLR;
            i2cif_reg_write(CHG_REG_26, val);
        }
        if (val_irq_raw & VIN_OC_INTR) {
            status |= CHARGER_IRQ_CAUSE_VIN_OC;
            if (val == 0) {
                i2cif_reg_read(CHG_REG_26, &val);
            }
            val |= REG_VIN_OC_INTR_CLR;
            i2cif_reg_write(CHG_REG_26, val);
        }
        int_unlock(lock);
    }

    if (chg_irq_handler) {
        chg_irq_handler(status);
    }
}

static uint32_t charger_get_efuse(enum CHARGER_EFUSE_PAGE_E page, uint16_t *efuse_val)
{
    uint32_t ret, start;
    uint16_t val;

    *efuse_val = 0;

    ret = i2cif_reg_read(CHG_REG_40, &val);
    if (ret) {
        goto _exit;
    }

    val = (val & ~(REG_EFUSE_READ_EN | REG_EFUSE_PGM_EN)) | REG_EFUSE_CLOCK_EN;
    ret = i2cif_reg_write(CHG_REG_40, val);
    if (ret) {
        goto _exit;
    }

    val |= REG_EFUSE_READ_EN;
    ret = i2cif_reg_write(CHG_REG_40, val);
    if (ret) {
        goto _exit;
    }

    start = hal_sys_timer_get();
    do {
        i2cif_reg_read(CHG_REG_40, &val);
        if (GET_BITFIELD(val, EFUSE_STATE) == 0x5) {
            break;
        }
    } while ((hal_sys_timer_get() - start) < MS_TO_TICKS(10));

    if (ret) {
        goto _exit;
    }

    // Page1 only 8 bits
    if (page == CHARGER_EFUSE_PAGE_1) {
        ret = i2cif_reg_read(CHG_REG_2A, &val);
    } else {
        ret = i2cif_reg_read(CHG_REG_42, &val);
    }
    if (ret) {
        goto _exit;
    }

    *efuse_val = val;

_exit:
    val &= ~REG_EFUSE_CLOCK_EN;
    i2cif_reg_write(CHG_REG_40, val);

    val &= ~REG_EFUSE_READ_EN;
    i2cif_reg_write(CHG_REG_40, val);

    return ret;
}

static void charger_sys_ctrl_enable(bool shipmode)
{
    uint32_t lock = int_lock();
    uint16_t val;
    int i;

    TRACE_IMM(0, "Start charger %s", shipmode ? "shipmode" : "reboot");

    if (shipmode) {
        if (chg_type == CHARGER_CHIP_TYPE_1622) {
            i2cif_reg_read(CHG_REG_24, &val);
            val |= REG_STANDBY_SHIP_MODE | REG_SHIP_MODE_EN;
            for (i = 0; i < 10; i++) {
                i2cif_reg_write(CHG_REG_24, val);
                hal_sys_timer_delay(MS_TO_TICKS(5));
            }
            TRACE_IMM(0, "%s: Fail!", __func__);
        }
        pmu_shutdown();
    } else {
        if (chg_type == CHARGER_CHIP_TYPE_1622) {
            i2cif_reg_read(CHG_REG_21, &val);
            val |= FET_RESET;
            for (i = 0; i < 10; i++) {
                i2cif_reg_write(CHG_REG_21, val);
                hal_sys_timer_delay(MS_TO_TICKS(5));
            }
            TRACE_IMM(0, "%s: Fail!", __func__);
        }
        pmu_wdt_reboot();
    }

    //can't reach here
    int_unlock(lock);
}

static void charger_set_vbat_oc(uint8_t bat_cl, uint8_t cl_vos)
{
    uint16_t val, val_tmp;
    uint8_t reg;
    int i;

    for (i = 0; i < 3; i++) {
        if (i == 0) {
            reg = CHG_REG_02;
            val_tmp = BAT_CL_IBIT_2_0_BIT_0;
        } else if (i == 1) {
            reg = CHG_REG_30;
            val_tmp = BAT_CL_IBIT_2_0_BIT_1;
        } else {
            reg = CHG_REG_31;
            val_tmp = BAT_CL_IBIT_2_0_BIT_2;
        }
        i2cif_reg_read(reg, &val);
        if (bat_cl & (1 << i)) {
            val |= val_tmp;
        } else {
            val &= ~val_tmp;
        }
        i2cif_reg_write(reg, val);
    }

    i2cif_reg_read(CHG_REG_30, &val);
    val_tmp = cl_vos & 0x7;
    val = SET_BITFIELD(val, CL_VOS_BIT_2_0, val_tmp);
    i2cif_reg_write(CHG_REG_30, val);

    val_tmp = CL_VOS_BIT_3;
    i2cif_reg_read(CHG_REG_31, &val);
    if (cl_vos & (1 << 3)) {
        val |= val_tmp;
    } else {
        val &= ~val_tmp;
    }
    i2cif_reg_write(CHG_REG_31, val);
}

static void charger_load_efuse_calib(void)
{
    uint16_t val, efuse_val, efuse_val_1, tmp_val;
    uint16_t val_01, val_02, val_23;
    bool load_efuse = false;

    // Enable AC_ON LPO manually to ensure efuse read can proceed even if AC_ON has not been triggered
    i2cif_reg_read(CHG_REG_23, &val_23);
    val = (val_23 | REG_PU_LPO_DR | REG_PU_LPO);
    i2cif_reg_write(CHG_REG_23, val);

    // Set fast AC_ON LPO
    i2cif_reg_read(CHG_REG_01, &val_01);
    val = (val_01 | REG_SYS_DET_EN_DR | REG_SYS_DET_EN);
    i2cif_reg_write(CHG_REG_01, val);

    i2cif_reg_read(CHG_REG_02, &val_02);
    val = (val_02 | REG_BG_CORE_EN_DR | REG_BG_CORE_EN);
    i2cif_reg_write(CHG_REG_02, val);

    hal_sys_timer_delay(MS_TO_TICKS(1));

    efuse_val_1 = 0;

    charger_get_efuse(CHARGER_EFUSE_PAGE_0, &efuse_val);

    if (chg_metal_id > HAL_CHIP_METAL_ID_0) {
        charger_get_efuse(CHARGER_EFUSE_PAGE_1, &efuse_val_1);
        if (efuse_val_1 && efuse_val) {
            load_efuse = true;
        }
#if 0 // Always metal id 2
        if (efuse_val_1 & CHARGE_EFUSE_NEW_METAL_ID) {
            chg_metal_id = HAL_CHIP_METAL_ID_2;
        }
#endif
    } else {
        if (efuse_val) {
            load_efuse = true;
        }
    }

    TRACE(0, "%s: efuse_page_0/1=0x%x/0x%x, load_efuse=%d", __func__, efuse_val, efuse_val_1, load_efuse);

    // Resume
    i2cif_reg_write(CHG_REG_01, val_01);
    i2cif_reg_write(CHG_REG_02, val_02);
    i2cif_reg_write(CHG_REG_23, val_23);

    if (!load_efuse) {
        return;
    }

    // Disable charge_en before load efuse calib value by default
    charger_charge_disable();

    if (chg_metal_id >= HAL_CHIP_METAL_ID_2) {
        // Load BG calib value
        tmp_val = GET_BITFIELD(efuse_val_1, CHARGE_EFUSE_NEW_BG_TRIM);
        if (efuse_val_1 & CHARGE_EFUSE_NEW_BG_TRIM_COMP_SIGN) {
            tmp_val = 0x20 - tmp_val;
        } else {
            tmp_val = 0x20 + tmp_val;
        }
        i2cif_reg_read(CHG_REG_0E, &val);
        val = SET_BITFIELD(val, BG_TRIM, tmp_val);
        i2cif_reg_write(CHG_REG_0E, val);
        hal_sys_timer_delay_us(CHARGER_BANDGAP_STABLE_TIME_US);
        TRACE(0, "bg_cal_v3: bg_trim=0x%x", tmp_val);

        // Load CC calib value
        tmp_val = GET_BITFIELD(efuse_val, CHARGE_EFUSE_NEW_TRIM_CC);
        i2cif_reg_read(CHG_REG_0B, &val);
        val = SET_BITFIELD(val, REG_CHARGER_TRIM_CC, tmp_val);
        i2cif_reg_write(CHG_REG_0B, val);
        TRACE(0, "cc_cal_v3: trim_cc=0x%x", tmp_val);

        // Load CV calib value
        tmp_val = (GET_BITFIELD(efuse_val, CHARGE_EFUSE_NEW_TRIM_VOREG) << 1) | (1 << 0);
        i2cif_reg_read(CHG_REG_0A, &val);
        val = SET_BITFIELD(val, CHARGER_TRIM_VOREG, tmp_val);
        i2cif_reg_write(CHG_REG_0A, val);
        TRACE(0, "cv_cal_v3: trim_voreg=0x%x", tmp_val);

        // Load IPRE calib value
        tmp_val = GET_BITFIELD(efuse_val, CHARGE_EFUSE_NEW_IPRE_TRIM_COMP);
        if (efuse_val_1 & CHARGE_EFUSE_NEW_IPRE_TRIM_COMP_SIGN) {
            tmp_val = 0x10 - tmp_val;
        } else {
            tmp_val = 0x10 + tmp_val;
        }
        val = SET_BITFIELD(val, CHARGER_TRIM_PRE, tmp_val);
        i2cif_reg_write(CHG_REG_0A, val);
        TRACE(0, "ipre_cal_v3: trim_pre=0x%x", tmp_val);

        // Load VBAT_OCP calib value
        // Load bat_cl_ibit[2:0]
        bat_cl_org = 0;
        if (efuse_val & CHARGE_EFUSE_BAT_CL_IBIT_BIT_0) {
            bat_cl_org |= (1 << 0);
        }

        if (efuse_val & CHARGE_EFUSE_NEW_BAT_CL_IBIT_BIT_1) {
            bat_cl_org |= (1 << 1);
        }

        // Load cl_vos_bit[3:0]
        tmp_val = GET_BITFIELD(efuse_val, CHARGE_EFUSE_CL_VOS_BIT_COMP);
        if (efuse_val & CHARGE_EFUSE_CL_VOS_BIT_COMP_SIGN) {
            cl_vos_org = 0x3 - tmp_val;
        } else {
            cl_vos_org = 0x3 + tmp_val;
        }
        charger_set_vbat_oc(bat_cl_org, cl_vos_org);
        TRACE(0, "ocp_cal_v3: bat_cl_org=0x%x, cl_vos_org=0x%x", bat_cl_org, cl_vos_org);
    }

    charger_charge_enable();
}

enum CHARGER_CHARGE_STATUE_E charger_charge_status_get(void)
{
    uint16_t val = CHARGER_CHARGE_STATUS_IDLE;

    if (chg_type != CHARGER_CHIP_TYPE_NONE && pmu_charger_get_status() == PMU_CHARGER_PLUGIN) {
        if (chg_type == CHARGER_CHIP_TYPE_1622) {
            i2cif_reg_read(CHG_REG_33, &val);
            val = GET_BITFIELD(val, CHARGE_STATE);
        } else {
            if (CHARGER_1620_PIN_CHGEN_INDEX != HAL_IOMUX_PIN_NUM) {
                if (hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)CHARGER_1620_PIN_CHGEN_INDEX)) {
                    val = CHARGER_CHARGE_STATUS_FAST;
                } else {
                    val = CHARGER_CHARGE_STATUS_DONE;
                }
            }
        }
    }

    return (enum CHARGER_CHARGE_STATUE_E)val;
}

int charger_charge_pre2cc_volt_set(enum CHARGER_CHARGE_PRE2CC_VOLTAGE_E pre2cc_volt)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620 || pre2cc_volt >= CHARGER_CHARGE_PRE2CC_VOLTAGE_QTY) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_0B, &val);
    val = SET_BITFIELD(val, REG_CHARGER_PRE2CC_VBIT, pre2cc_volt);
    i2cif_reg_write(CHG_REG_0B, val);

    return CHARGER_RET_OK;
}

int charger_charge_pre2cc_current_set(enum CHARGER_CHARGE_PRE2CC_CURRENT_E pre2cc_current)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620 || pre2cc_current >= CHARGER_CHARGE_PRE2CC_CURRENT_QTY) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_04, &val);
    val = SET_BITFIELD(val, REG_CHARGER_IPRE_SEL, pre2cc_current);
    i2cif_reg_write(CHG_REG_04, val);

    return CHARGER_RET_OK;
}

int charger_charge_cc_current_set(enum CHARGER_CHARGE_CONSTANT_CURRENT_E cc_current)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620 || cc_current >= CHARGER_CHARGE_CONSTANT_CURRENT_QTY) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_04, &val);
    val = SET_BITFIELD(val, REG_CHARGER_ICC_SEL, cc_current);
    i2cif_reg_write(CHG_REG_04, val);

    return CHARGER_RET_OK;
}

int charger_charge_stop_current_set(enum CHARGER_CHARGE_STOP_CURRENT_E stop_current)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620 || stop_current >= CHARGER_CHARGE_STOP_CURRENT_QTY) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_0C, &val);
    val = SET_BITFIELD(val, CHARGER_ITERM_SEL, stop_current);
    i2cif_reg_write(CHG_REG_0C, val);

    return CHARGER_RET_OK;
}

int charger_charge_cv_volt_set(enum CHARGER_CHARGE_CONSTANT_VOLTAGE_E cv_volt)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620 || cv_volt >= CHARGER_CHARGE_CONSTANT_VOLTAGE_QTY) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_03, &val);
    val = SET_BITFIELD(val, REG_CHARGER_VOREG_BIT, cv_volt);
    i2cif_reg_write(CHG_REG_03, val);

    return CHARGER_RET_OK;
}

enum CHARGER_CHARGE_CONSTANT_CURRENT_E charger_charge_cc_current_get(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_CHARGE_CONSTANT_CURRENT_QTY;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1622) {
        i2cif_reg_read(CHG_REG_04, &val);
        val = GET_BITFIELD(val, REG_CHARGER_ICC_SEL);
    } else {
        val = chg_1620_ctx.cc_current;
    }

    return (enum CHARGER_CHARGE_CONSTANT_CURRENT_E)val;
}

enum CHARGER_CHARGE_PRE2CC_VOLTAGE_E charger_charge_pre2cc_volt_get(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_CHARGE_CONSTANT_CURRENT_QTY;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1622) {
        i2cif_reg_read(CHG_REG_0B, &val);
        val = GET_BITFIELD(val, REG_CHARGER_PRE2CC_VBIT);
    } else {
        val = chg_1620_ctx.pre2cc_volt;
    }

    return (enum CHARGER_CHARGE_CONSTANT_CURRENT_E)val;
}

enum CHARGER_CHARGE_PRE2CC_CURRENT_E charger_charge_pre2cc_current_get(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_CHARGE_CONSTANT_CURRENT_QTY;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1622) {
        i2cif_reg_read(CHG_REG_04, &val);
        val = GET_BITFIELD(val, REG_CHARGER_IPRE_SEL);
    } else {
        val = chg_1620_ctx.pre2cc_current;
    }

    return (enum CHARGER_CHARGE_CONSTANT_CURRENT_E)val;
}

enum CHARGER_CHARGE_STOP_CURRENT_E charger_charge_stop_current_get(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_CHARGE_STOP_CURRENT_QTY;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1622) {
        i2cif_reg_read(CHG_REG_0C, &val);
        val = GET_BITFIELD(val, CHARGER_ITERM_SEL);
    } else {
        val = chg_1620_ctx.stop_curent;
    }

    return (enum CHARGER_CHARGE_STOP_CURRENT_E)val;
}

enum CHARGER_CHARGE_CONSTANT_VOLTAGE_E charger_charge_cv_volt_get(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_CHARGE_CONSTANT_VOLTAGE_QTY;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1622) {
        i2cif_reg_read(CHG_REG_03, &val);
        val = GET_BITFIELD(val, REG_CHARGER_VOREG_BIT);
    } else {
        val = chg_1620_ctx.cv_volt;
    }

    return (enum CHARGER_CHARGE_CONSTANT_VOLTAGE_E)val;
}

int charger_acin2vsys_limit_current_set(enum CHARGER_ACIN2VSYS_LIMIT_CURRENT_E  limit_current)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620 || limit_current >= CHARGER_ACIN2VSYS_LIMIT_CURRENT_QTY) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_08, &val);
    val = SET_BITFIELD(val, PP_ILIMIT_SEL, limit_current);
    i2cif_reg_write(CHG_REG_08, val);

    return CHARGER_RET_OK;
}

enum CHARGER_ACIN2VSYS_LIMIT_CURRENT_E charger_acin2vsys_limit_current_get(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_CHARGE_CONSTANT_VOLTAGE_QTY;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1622) {
        i2cif_reg_read(CHG_REG_08, &val);
        val = GET_BITFIELD(val, PP_ILIMIT_SEL);
    } else {
        val = chg_1620_ctx.limit_current;
    }

    return (enum CHARGER_ACIN2VSYS_LIMIT_CURRENT_E)val;
}

int charger_acin2vsys_vsys_volt_set(enum CHARGER_ACIN2VSYS_VSYS_VOLTAGE_E vsys_volt)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620 || vsys_volt >= CHARGER_ACIN2VSYS_VSYS_VOLTAGE_QTY) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_07, &val);
    val = SET_BITFIELD(val, PP_VSYS_CV_VBIT, vsys_volt);
    i2cif_reg_write(CHG_REG_07, val);

    return CHARGER_RET_OK;
}

enum CHARGER_ACIN2VSYS_VSYS_VOLTAGE_E charger_acin2vsys_vsys_volt_get(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_CHARGE_CONSTANT_VOLTAGE_QTY;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1622) {
        i2cif_reg_read(CHG_REG_07, &val);
        val = GET_BITFIELD(val, PP_VSYS_CV_VBIT);
    } else {
        val = chg_1620_ctx.vsys_volt;
    }

    return (enum CHARGER_ACIN2VSYS_VSYS_VOLTAGE_E)val;
}

int charger_acin2vsys_vsys_min_volt_set(enum CHARGER_ACIN2VSYS_VSYS_MIN_VOLTAGE_E vsys_min_volt)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620 || vsys_min_volt >= CHARGER_ACIN2VSYS_VSYS_MIN_VOLTAGE_QTY) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_07, &val);
    val = SET_BITFIELD(val, PP_VSYS_MIN_VBIT, vsys_min_volt);
    i2cif_reg_write(CHG_REG_07, val);

    return CHARGER_RET_OK;
}

enum CHARGER_ACIN2VSYS_VSYS_MIN_VOLTAGE_E charger_acin2vsys_vsys_min_volt_get(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_CHARGE_CONSTANT_VOLTAGE_QTY;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1622) {
        i2cif_reg_read(CHG_REG_07, &val);
        val = GET_BITFIELD(val, PP_VSYS_MIN_VBIT);
    } else {
        val = chg_1620_ctx.vsys_min_volt;
    }

    return (enum CHARGER_ACIN2VSYS_VSYS_MIN_VOLTAGE_E)val;
}

int charger_acin2vsys_acin_min_volt_set(enum CHARGER_ACIN2VSYS_ACIN_MIN_VOLTAGE_E acin_min_volt)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620 || acin_min_volt >= CHARGER_ACIN2VSYS_ACIN_MIN_VOLTAGE_QTY) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_08, &val);
    val = SET_BITFIELD(val, PP_VIN_DPM_VBIT, acin_min_volt);
    i2cif_reg_write(CHG_REG_08, val);

    return CHARGER_RET_OK;
}

enum CHARGER_ACIN2VSYS_ACIN_MIN_VOLTAGE_E charger_acin2vsys_acin_min_volt_get(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_CHARGE_CONSTANT_VOLTAGE_QTY;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1622) {
        i2cif_reg_read(CHG_REG_08, &val);
        val = GET_BITFIELD(val, PP_VIN_DPM_VBIT);
    } else {
        val = chg_1620_ctx.acin_min_volt;
    }

    return (enum CHARGER_ACIN2VSYS_ACIN_MIN_VOLTAGE_E)val;
}

int charger_charge_forward_delta_volt_set(enum CHARGER_CHARGE_FORWARD_DELTA_VOLTAGE_E fr_volt)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620 || fr_volt >= CHARGER_CHARGE_FORWARD_DELTA_VOLTAGE_QTY) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_09, &val);
    val = SET_BITFIELD(val, PP_FR_DELTA_VBIT, fr_volt);
    i2cif_reg_write(CHG_REG_09, val);

    return CHARGER_RET_OK;
}

enum CHARGER_CHARGE_FORWARD_DELTA_VOLTAGE_E charger_charge_forward_delta_volt_get(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_CHARGE_CONSTANT_VOLTAGE_QTY;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1622) {
        i2cif_reg_read(CHG_REG_09, &val);
        val = GET_BITFIELD(val, PP_FR_DELTA_VBIT);
    } else {
        val = chg_1620_ctx.fr_volt;
    }

    return (enum CHARGER_CHARGE_FORWARD_DELTA_VOLTAGE_E)val;
}

int charger_charge_forward_enable(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_06, &val);
    val &= ~(REG_PP_CV_MODE_EN | REG_PP_VIN_DPM_EN);
    val |= REG_PP_CV_MODE_EN_DR | REG_PP_VIN_DPM_EN_DR;
    i2cif_reg_write(CHG_REG_06, val);

    return CHARGER_RET_OK;
}

int charger_charge_forward_disable(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_06, &val);
    val &= ~(REG_PP_CV_MODE_EN | REG_PP_VIN_DPM_EN);
    val &= ~(REG_PP_CV_MODE_EN_DR | REG_PP_VIN_DPM_EN_DR);
    i2cif_reg_write(CHG_REG_06, val);

    return CHARGER_RET_OK;
}

int charger_charge_recharge_enable(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_01, &val);
    val |= REG_CHARGER_RECHARGE_EN;
    i2cif_reg_write(CHG_REG_01, val);

    return CHARGER_RET_OK;
}

int charger_charge_recharge_disable(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_01, &val);
    val &= ~REG_CHARGER_RECHARGE_EN;
    i2cif_reg_write(CHG_REG_01, val);

    return CHARGER_RET_OK;
}

int charger_charge_enable(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_21, &val);
    val |= REG_SW_CHARGE_EN;
    i2cif_reg_write(CHG_REG_21, val);

    return CHARGER_RET_OK;
}

int charger_charge_disable(void)
{
    uint16_t val;

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (chg_type == CHARGER_CHIP_TYPE_1620) {
        return CHARGER_RET_INVALID_OPTION;
    }

    i2cif_reg_read(CHG_REG_21, &val);
    val &= ~REG_SW_CHARGE_EN;
    i2cif_reg_write(CHG_REG_21, val);

    return CHARGER_RET_OK;
}

int charger_charge_open(void)
{
    static bool first_open = true;

    if (!first_open) {
        if (!charger_opened) {
            charger_bus_init();
            charger_opened = true;
        }
    } else {
        // Init once
        charger_chip_type_match();
        charger_bus_init();
        if (chg_type == CHARGER_CHIP_TYPE_1622) {
            uint16_t val;

            // Enable LPO manually to ensure register writes can proceed even if AC_ON has not been triggered
            i2cif_reg_read(CHG_REG_21, &val);
            val |= REG_LPO_ON;
            i2cif_reg_write(CHG_REG_21, val);

            hal_sys_timer_delay(MS_TO_TICKS(3));

            i2cif_reg_write(CHG_REG_21, val);

            i2cif_reg_read(CHG_REG_21, &val);
            if (val & REG_LPO_ON) {
                TRACE(0, "%s: Invalid status!", __func__);
            }

#ifdef NO_METAL_ID_0
            chg_metal_id = HAL_CHIP_METAL_ID_2;
#else
            i2cif_reg_read(CHG_REG_2A, &val);
            if (val == 0xFF00) {
                chg_metal_id = HAL_CHIP_METAL_ID_0;
            } else {
                chg_metal_id = HAL_CHIP_METAL_ID_1;
            }
#endif

            i2cif_reg_read(CHG_REG_05, &val);
            val = (val & ~REG_CHARGER_BAT_OVP_DET_EN) | REG_CHARGER_BAT_OVP_DET_EN_DR;
            i2cif_reg_write(CHG_REG_05, val);

            i2cif_reg_write(CHG_REG_28, REG_CHARGE_DONE_INTR_CLR);
            i2cif_reg_write(CHG_REG_26, (REG_VIN_OV_INTR_CLR | REG_VIN_OC_INTR_CLR));

            i2cif_reg_write(CHG_REG_28, REG_CHARGE_DONE_INTR_EN);
            i2cif_reg_write(CHG_REG_26, 0x0);

            charger_load_efuse_calib();
            if (chg_metal_id > HAL_CHIP_METAL_ID_0) {
                bat_cl_plugin = 7;
                cl_vos_plugin = cl_vos_org + 3;
                pmu_charger_init();
                pmu_charger_set_irq_handler(NULL);
                if (pmu_charger_get_status() == PMU_CHARGER_PLUGIN) {
                    charger_plug_conig(PMU_CHARGER_PLUGIN);
                }
                charger_acin2vsys_limit_current_set(CHARGER_ACIN2VSYS_LIMIT_CURRENT_300MA);
            }

            i2cif_reg_read(CHG_REG_06, &val);
            val |= REG_PP_VIN_FR_EN;
            i2cif_reg_write(CHG_REG_06, val);
        }
        first_open = false;
        charger_opened = true;
        TRACE(0, "%s: chg_type=%d, chg_metal_id=%d", __func__, chg_type, chg_metal_id);
    }

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    return CHARGER_RET_OK;
}

int charger_charge_close(void)
{
    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (charger_opened) {
        charger_bus_deinit();
        charger_opened = false;
    }

    return CHARGER_RET_OK;
}

enum CHARGER_CHIP_TYPE_E charger_get_charger_type(void)
{
    return chg_type;
}

int charger_charge_irq_handler_set(CHARGER_CHARGE_IRQ_HANDLER_T handler)
{
    uint32_t lock;
    uint8_t index;
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pin_mux[2] = {
        {CHARGER_1620_IRQ_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {CHARGER_1622_IRQ_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
    };
    static const struct HAL_GPIO_IRQ_CFG_T irq_cfg = {
        true, false, HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE, HAL_GPIO_IRQ_POLARITY_LOW_FALLING, charger_gpio_irq_handler
    };

    if (chg_type == CHARGER_CHIP_TYPE_NONE) {
        return CHARGER_RET_INVALID_CHIP;
    }

    if (handler) {
        lock = int_lock();
        chg_irq_handler = handler;
        if (chg_type == CHARGER_CHIP_TYPE_1620) {
            index = 0;
        } else {
            index = 1;
        }
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&pin_mux[index], 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pin_mux[index].pin, HAL_GPIO_DIR_IN, 0);
        hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)pin_mux[index].pin, &irq_cfg);
        int_unlock(lock);
    }

    return CHARGER_RET_OK;
}

void charger_shipmode(void)
{
#ifdef PMU_SHUTDOWN_VIA_SHIPMODE
    pmu_shutdown();
    return;
#endif
    charger_sys_ctrl_enable(true);
}

void charger_reboot(void)
{
    charger_sys_ctrl_enable(false);
}

void charger_plug_conig(enum PMU_CHARGER_STATUS_T status)
{
    if (chg_type == CHARGER_CHIP_TYPE_1622 && chg_metal_id > HAL_CHIP_METAL_ID_0) {
        if (status == PMU_CHARGER_PLUGIN) {
            charger_set_vbat_oc(bat_cl_plugin, cl_vos_plugin);
        } else {
            charger_set_vbat_oc(bat_cl_org, cl_vos_org);
        }
    }
}

void charger_acin2vsys_ldo_soft_start_enable(int enable)
{
    uint16_t val;

    if (chg_type != CHARGER_CHIP_TYPE_1622) {
        return;
    }

    i2cif_reg_read(CHG_REG_06, &val);
    if (enable) {
        val &= ~(REG_PP_CS_EN | REG_PP_CS_EN_DR);
    } else {
        val = (val & ~REG_PP_CS_EN) | REG_PP_CS_EN_DR;
    }
    i2cif_reg_write(CHG_REG_06, val);
}
