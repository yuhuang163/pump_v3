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
#ifdef CHARGER_TEST
#if 1
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_key.h"
#include "hal_gpadc.h"
#include "hal_sleep.h"
#include "pmu.h"
#include "hwtest.h"
#include "hwtimer_list.h"
#include CHIP_SPECIFIC_HDR(pmu)
#include CHIP_SPECIFIC_HDR(charger)

//#define CHARGER_PATTERN_TEST

#define CHARGER_GPADC_TEST_LOOP_CNT                                 3
#define CHARGER_GPADC_TEST_DELAY_MS                                 5
#define CHARGER_GPADC_TIMER_PERIOD_MS                               1000

static const struct CHARGER_CHARGE_MODULE_CFG_T g_charger_module_config_base = {
    .cc_current = CHARGER_CHARGE_CONSTANT_CURRENT_60MA,
    .cv_volt = CHARGER_CHARGE_CONSTANT_VOLTAGE_4350MV,
    .rechg_en = true,
    .chg_en = true,
#ifdef CHIP_BEST1306
    .prechg_current = CHARGER_CHARGE_PRECHARGE_CURRENT_2P9MA,
    .stop_current = CHARGER_CHARGE_STOP_CURRENT_4MA,
    .rechg_volt = CHARGER_CHARGE_RECHARGE_VOLTAGE_100MV,
    .ntc_det_en = true,
#elif defined(CHIP_BEST1503)
    .prechg_current = CHARGER_CHARGE_PRECHARGE_CURRENT_4MA,
    .stop_current = CHARGER_CHARGE_STOP_CURRENT_2MA,
    .rechg_volt = CHARGER_CHARGE_RECHARGE_VOLTAGE_150MV,
    .prechg_volt = CHARGER_CHARGE_PRECHARGE_VOLTAGE_2800MV,
#endif
};

#ifdef CHIP_BEST1306
static enum CHARGER_PACKAGE_TYPE_E g_charger_package_type;
#endif
static enum HAL_GPADC_CHAN_T g_gpadc_channel = HAL_GPADC_CHAN_BATTERY;

#ifdef CHARGER_PATTERN_TEST
enum CHARGER_PATTERN_TYPE_T {
    CHARGER_PATTERN_RESET_CHARGER,
    CHARGER_PATTERN_SHIP_MODE_ON,
    CHARGER_PATTERN_SHIP_MODE_OFF,
    CHARGER_PATTERN_SINGLE_UART_ON,
    CHARGER_PATTERN_SINGLE_UART_OFF,

    CHARGER_PATTERN_QTY,
};

struct CHARGER_PATTERN_MATCH_MAP {
    enum CHARGER_PATTERN_TYPE_T type;
    uint16_t raw;
};

#define CHARGER_PATTERN_SIZE                                        17
#define CHARGER_PATTERN_ERROR_TYPE                                  0xFFFFFFFF
#define CHARGER_PATTERN_STABLE_TIME_MS                              5

static const struct CHARGER_PATTERN_MATCH_MAP g_charger_pattern_map[] = {
    {CHARGER_PATTERN_RESET_CHARGER,    0x195},
    {CHARGER_PATTERN_SHIP_MODE_ON,     0x1B3},
    {CHARGER_PATTERN_SHIP_MODE_OFF,    0x1B5},
    {CHARGER_PATTERN_SINGLE_UART_ON,   0x1AB},
    {CHARGER_PATTERN_SINGLE_UART_OFF,  0x1AD},
};

static enum CHARGER_PACKAGE_TYPE_E g_charger_pattern_type = CHARGER_PATTERN_RESET_CHARGER;

static void charger_pattern_type_print(enum CHARGER_PATTERN_TYPE_T type, uint32_t pattern_raw)
{
    TRACE(0, "%s", __func__);

    if (type == CHARGER_PATTERN_RESET_CHARGER) {
        TRACE(0, "cur_type :CHARGER_PATTERN_RESET_CHARGER raw=[0x%08x]", pattern_raw);
        TRACE(0, "next_type:CHARGER_PATTERN_SHIP_MODE_ON");
    } else if (type == CHARGER_PATTERN_SHIP_MODE_ON) {
        TRACE(0, "cur_type :CHARGER_PATTERN_SHIP_MODE_ON RAW=[0x%08x]", pattern_raw);
        TRACE(0, "next_type:CHARGER_PATTERN_SHIP_MODE_OFF");
    } else if (type == CHARGER_PATTERN_SHIP_MODE_OFF) {
        TRACE(0, "cur_type :CHARGER_PATTERN_SHIP_MODE_OFF RAW=[0x%08x]", pattern_raw);
        TRACE(0, "next_type:CHARGER_PATTERN_SINGLE_UART_ON");
    } else if (type == CHARGER_PATTERN_SINGLE_UART_ON) {
        TRACE(0, "cur_type :CHARGER_PATTERN_SINGLE_UART_ON RAW=[0x%08x]", pattern_raw);
        TRACE(0, "next_type:CHARGER_PATTERN_SINGLE_UART_OFF");
    } else if (type == CHARGER_PATTERN_SINGLE_UART_OFF) {
        TRACE(0, "cur_type :CHARGER_PATTERN_SINGLE_UART_OFF RAW=[0x%08x]", pattern_raw);
        TRACE(0, "next_type:CHARGER_PATTERN_RESET_CHARGER");
    } else {
        TRACE(0, "%s ERROR TYPE! type:%d", __func__, type);
    }
}

static uint32_t charger_pattern_raw_get(enum CHARGER_PATTERN_TYPE_T pattern)
{
    uint32_t pattern_raw;
    uint16_t val;

    if (pattern >= CHARGER_PATTERN_QTY) {
        return CHARGER_PATTERN_ERROR_TYPE;
    }

    for (int i = 0; i < ARRAY_SIZE(g_charger_pattern_map); i++) {
        if (g_charger_pattern_map[i].type == pattern) {
            val = g_charger_pattern_map[i].raw;
            break;
        }
    }

    pattern_raw = 0x15001;
    pattern_raw &= ~(0x1ff << 2);
    pattern_raw |= (val << 2);

    charger_pattern_type_print(pattern, pattern_raw);

    return pattern_raw;
}

static void charger_pattern_set(enum CHARGER_PATTERN_TYPE_T pattern)
{
    uint32_t pattern_raw;
    bool output_high;

    pattern_raw = charger_pattern_raw_get(pattern);

    hal_iomux_tportopen();
    for (int i = CHARGER_PATTERN_SIZE - 1; i >= 0; i--) {
        output_high = !!(pattern_raw & (1 << i));
        if (output_high) {
            hal_iomux_tportset(0);
        } else {
            hal_iomux_tportclr(0);
        }
        hal_sys_timer_delay(MS_TO_TICKS(CHARGER_PATTERN_STABLE_TIME_MS));
    }
    hal_iomux_tportclr(0);
}

static int charger_pattern_power_key_event_process(uint32_t key_code, uint8_t key_event)
{
    TR_INFO(TR_MOD(MAIN), "%s: code=0x%X event=%u", __FUNCTION__, key_code, key_event);

    if (key_code == HAL_KEY_CODE_PWR) {
        if (key_event == HAL_KEY_EVENT_CLICK) {
            charger_pattern_set(g_charger_pattern_type);
            g_charger_pattern_type++;
            if (g_charger_pattern_type == CHARGER_PACKAGE_TYPE_QTY) {
                g_charger_pattern_type = CHARGER_PATTERN_RESET_CHARGER;
            }
        }
    }

    return 0;
}

static void charger_pattern_test(void)
{
    hal_key_open(false, charger_pattern_power_key_event_process);
    while(1) {
        TRACE(0, "[TIPS]: Click power key send charger pattern!");
        hal_sys_timer_delay(MS_TO_TICKS(1000 * 5));
    }
}
#endif

static void charger_gpadc_test(enum HAL_GPADC_CHAN_T ch)
{
    uint16_t volt;
    int ret = 0;

    for (int i = 0; i < CHARGER_GPADC_TEST_LOOP_CNT; i++) {
        ret = hal_gpadc_open(ch, HAL_GPADC_ATP_ONESHOT, NULL);
        ASSERT(0 == ret, "gpadc block read, ch:%d init fail", ch);
        ret = hal_gpadc_get_volt_blocked(ch, &volt);
        ASSERT(ret, "gpadc block read, ch:%d get volt fail", ch);
        TRACE(0, "gpadc block read, ch:%d volt:%d", ch, volt);
        hal_gpadc_close(ch);
    }
}

static int charger_key_event_process(uint32_t key_code, uint8_t key_event)
{
    static bool chg_enabled = true;

    TR_INFO(TR_MOD(MAIN), "%s: code=0x%X event=%u", __FUNCTION__, key_code, key_event);

    if (key_code == HAL_KEY_CODE_PWR) {
        if (key_event == HAL_KEY_EVENT_CLICK) {
            if (chg_enabled) {
                charger_charge_disable();
            } else {
                charger_charge_enable();
            }
            chg_enabled = !chg_enabled;
        } else if (key_event == HAL_KEY_EVENT_LONGPRESS) {
#ifdef CHIP_BEST1306
            if (g_charger_package_type != CHARGER_PACKAGE_TYPE_SIP_1620) {
                pmu_shipment_mode_enable(true);
            }
#endif
            pmu_shutdown();
        }
    }

    return 0;
}

static void charger_charge_irq_handler_test(uint32_t irq_raw)
{
    TRACE(0, "%s irq_raw=0x%08x", __func__, irq_raw);
}

static void charger_charge_module_config_print(struct CHARGER_CHARGE_MODULE_CFG_T *cfg)
{
    TRACE(0, "%s", __func__);

    TRACE(0, "prechg_current: %d", cfg->prechg_current);
    TRACE(0, "cc_current:     %d", cfg->cc_current);
    TRACE(0, "stop_current:   %d", cfg->stop_current);
    TRACE(0, "cv_volt:        %d", cfg->cv_volt);
    TRACE(0, "rechg_volt:     %d", cfg->rechg_volt);
    TRACE(0, "rechg_en:       %d", cfg->rechg_en);
    TRACE(0, "chg_en:         %d", cfg->chg_en);
#ifdef CHIP_BEST1306
    TRACE(0, "ntc_det_en:     %d", cfg->ntc_det_en);
#elif defined(CHIP_BEST1503)
    TRACE(0, "prechg_volt:    %d", cfg->prechg_volt);
#endif
    TRACE(0, "chg_status:     %d\n", charger_charge_status_get());
}

static void charger_charge_module_set_test(void)
{
    int ret = 0;
    struct CHARGER_CHARGE_MODULE_CFG_T cfg_set;

    cfg_set = g_charger_module_config_base;

    ret = charger_charge_module_cfg_set(&cfg_set);
    if (ret) {
        ASSERT(false, "charger_charge_module_cfg_set error! ret=%d", ret);
    }
}

static HWTIMER_ID g_charger_hw_timer = NULL;

static void charger_charge_timer_handler(void *param)
{
    struct CHARGER_CHARGE_MODULE_CFG_T cfg_get;

    // Check GPADC
    charger_gpadc_test(g_gpadc_channel);

    // Check charge module settings
    charger_charge_module_cfg_get(&cfg_get);
    charger_charge_module_config_print(&cfg_get);

    hwtimer_start(g_charger_hw_timer, MS_TO_TICKS(CHARGER_GPADC_TIMER_PERIOD_MS));
}

void charger_test(void)
{
#ifdef CHARGER_PATTERN_TEST
    // Will enter while(1){}
    charger_pattern_test();
#endif

    charger_charge_open();

#ifdef CHIP_BEST1306
    g_charger_package_type = charger_package_type_get();

    if (g_charger_package_type == CHARGER_PACKAGE_TYPE_SIP_1620) {
        g_gpadc_channel = HAL_GPADC_CHAN_BATTERY;
    } else {
        g_gpadc_channel = HAL_GPADC_CHAN_6;
    }

    if (g_charger_package_type != CHARGER_PACKAGE_TYPE_SIP_1620)
#endif
    {
        charger_charge_irq_handler_set(charger_charge_irq_handler_test);
        charger_charge_module_set_test();
    }

    hwtest_register_key_callback(charger_key_event_process);
    TRACE(0, "[TIPS]: Click power key switch charger on off!");
    TRACE(0, "[TIPS]: Longpress power key will enter ship mode!");

    g_charger_hw_timer = hwtimer_alloc(charger_charge_timer_handler, 0);
    hwtimer_start(g_charger_hw_timer, MS_TO_TICKS(CHARGER_GPADC_TIMER_PERIOD_MS));

    while (1) {
        hal_sleep_enter_sleep();
    }
} /* Charge intf chips */
#else
#include "hal_trace.h"
#include "hal_timer.h"
#include CHIP_SPECIFIC_HDR(charger)

#if 1
    #define CHARGER_TRACE(n, s, ...)      TRACE_IMM(n, s, ##__VA_ARGS__)
#else
    #define CHARGER_TRACE(n, s, ...)
#endif

#if 1
    #define CHARGER_DELAY_MS(ms)          hal_sys_timer_delay(MS_TO_TICKS(ms))
#else
    #define CHARGER_DELAY_MS(ms)
#endif

void charger_test(void)
{
    int ret;
    enum CHARGER_STATUS_E status;

    CHARGER_TRACE(0, "   ");
    CHARGER_TRACE(0, "------1501+charger test started------");
    CHARGER_TRACE(1, "CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    CHARGER_TRACE(0, "   ");

#if !defined(CHIP_BEST1501)
    while (1) {
        CHARGER_TRACE(0, "Currently support 1501 + charger chip, other chips do not support");
        CHARGER_DELAY_MS(2000);
    }
#endif

    while (1) {
        ret = charger_enable_charging();
        CHARGER_TRACE(0, "enable charging %s, ret :%d", ret ? "fail" : "success", ret);
        CHARGER_DELAY_MS(100);

        ret = charger_status_get(&status);
        if (!ret) {
            switch (status) {
                case CHARGER_NO_CHARGING:
                    CHARGER_TRACE(0, "charger stauts :NO_CHARGING");
                    break;
                case CHARGER_PRE_CHARGING:
                    CHARGER_TRACE(0, "charger stauts :PRE_CHARGING");
                    break;
                case CHARGER_CHARGING_CC:
                    CHARGER_TRACE(0, "charger stauts :CHARGING_CC");
                    break;
                case CHARGER_CHARGING_NONE:
                    CHARGER_TRACE(0, "charger stauts :CHARGING_CC");
                    break;
                default:
                    CHARGER_TRACE(0, "charger stauts :Unknown");
                    break;
            }
        } else {
            CHARGER_TRACE(0, "get charger stauts fail :%d", ret);
        }

        CHARGER_DELAY_MS(100);
        ret = charger_temp_status_get();
        switch (ret) {
            case CHARGER_BAT_TEMP_COLD:
                CHARGER_TRACE(0, "charger temp   :COLD[~, 0]");
                break;
            case CHARGER_BAT_TEMP_COOL:
                CHARGER_TRACE(0, "charger temp   :COOL[0, 10]");
                break;
            case CHARGER_BAT_TEMP_NORMAL:
                CHARGER_TRACE(0, "charger temp   :NORMAL[10, 45]");
                break;
            case CHARGER_BAT_TEMP_WARM:
                CHARGER_TRACE(0, "charger temp   :WARM[45, 60]");
                break;
            case CHARGER_BAT_TEMP_HOT:
                CHARGER_TRACE(0, "charger temp   :HOT[60, ~]");
                break;
            case CHARGER_BAT_TEMP_NONE:
                CHARGER_TRACE(0, "charger temp   :Unknown status");
                break;
            default:
                CHARGER_TRACE(0, "charger temp   :Unknown");
                break;
        }
        CHARGER_DELAY_MS(100);

        ret = charger_disable_charging();
        CHARGER_TRACE(0, "disable charging %s, ret :%d", ret ? "fail" : "success", ret);
        CHARGER_DELAY_MS(100);
    }
}

#endif
#endif

