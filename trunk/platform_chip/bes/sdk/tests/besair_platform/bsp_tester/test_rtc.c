/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifdef TEST_RTC
#include "cmsis_os.h"
#include "cmsis.h"
#include "pmu.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_wdt.h"
#include "string.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_bootmode.h"
#include "hal_sleep.h"
#include <stdlib.h>
#include "hal_timer.h"
#include "hal_gpio.h"

enum RTC_TEST_MODE_T {
    TIME_ACCURACY_TEST_MODE,
    HW_RESET_TEST_MODE,
    SW_RESET_TEST_MODE,
    SYS_WDT_RESET_TEST_MODE,
    PMU_WDT_TEST_MODE,
    RTC_ALARM_TEST_MODE,
    PMU_SHUTDOWN_ALARM_MODE,
    RTC_AON_TIMER_CMP,
};
#define RTC_TEST_MODE                   TIME_ACCURACY_TEST_MODE

#define RTC_CALENDAR_TRACE_INTERVAL     1000
#define HAL_SW_RTC_SET                  HAL_SW_BOOTMODE_REBOOT

static const struct RTC_CALENDAR_FORMAT_T _rtc_cfg_default = {
    .year = 2021,
    .month = 11,
    .day = 25,
    .week = 4,
    .hour = 16,
    .minute = 11,
    .second = 0,
};

static uint32_t user_timer_old_time = 0;
static uint32_t user_timer_new_time = 0;

static void rtc_alarm_trace(void);
static void rtc_calendar_trace(void);
static void _pmu_rtc_calendar_set(struct RTC_CALENDAR_FORMAT_T *time_set);
static void _pmu_rtc_alarm_set(struct RTC_CALENDAR_FORMAT_T *time_set);


/* ###################### PWRKEY to Set Calendar ###################### */
extern int pmu_pwrkey_set_irq(enum HAL_PWRKEY_IRQ_T type);
static void hal_pwrkey_tester_irqhandler(uint16_t irq_status)
{
    struct RTC_CALENDAR_FORMAT_T rtc_cal_cfg = _rtc_cfg_default;
    _pmu_rtc_calendar_set(&rtc_cal_cfg);

    enum HAL_PWRKEY_IRQ_T state;
    state = pmu_pwrkey_irq_value_to_state(irq_status);
    TRACE_IMM(2,"%s: irq_status:%04x key:%d", __func__, irq_status, state);
}

/* ###################### AON Timer Config ###################### */

extern void hal_timer_setup(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler);
extern void hal_timer_start(uint32_t load);
extern void hal_timer_stop(void);

static void acon_timer_periodic_irq_handler(uint32_t elapsed)
{
    user_timer_new_time = hal_sys_timer_get();
    if(user_timer_old_time) {
        TRACE(2, "acon_timer periodic run time: %dus", TICKS_TO_US(user_timer_new_time - user_timer_old_time));
        TRACE(2, "aon timer running %d", TICKS_TO_US(hal_sys_timer_get()) / 1000000);

        rtc_calendar_trace();
    }
    user_timer_old_time = user_timer_new_time;


}
/* ###################### RTC Calendar Config ###################### */
static void _pmu_rtc_calendar_set(struct RTC_CALENDAR_FORMAT_T *time_set)
{
    pmu_rtc_calendar_set(time_set);
}

static void _pmu_rtc_alarm_set(struct RTC_CALENDAR_FORMAT_T *time_set)
{
    pmu_rtc_calendar_alarm_set(time_set);
}

static void rtc_calendar_trace(void)
{
    struct RTC_CALENDAR_FORMAT_T cur_time;

    memset(&cur_time, 0, sizeof(cur_time));

    pmu_rtc_calendar_get(&cur_time);

    TRACE(0, "year/mouth/day week: %d/%02d/%02d/ %02d", cur_time.year, cur_time.month, cur_time.day, cur_time.week);
    TRACE(0, "hours:min:sec %02d:%02d:%02d\n", cur_time.hour, cur_time.minute, cur_time.second);
}

static void rtc_alarm_trace(void)
{
    struct RTC_CALENDAR_FORMAT_T alarm_time;

    memset(&alarm_time, 0, sizeof(alarm_time));

    pmu_rtc_calendar_alarm_get(&alarm_time);

    TRACE(0, "current alarm config:");
    TRACE(0, "year/mouth/day week: %d/%02d/%02d/ %02d", alarm_time.year, alarm_time.month, alarm_time.day, alarm_time.week);
    TRACE(0, "hours:min:sec %02d:%02d:%02d\n", alarm_time.hour, alarm_time.minute, alarm_time.second);
}

static void rtc_irq_handler_test(struct RTC_CALENDAR_FORMAT_T *cal)
{
    TRACE(0, "%s", __func__);
#if 0
    struct RTC_CALENDAR_FORMAT_T rtc_cal_cfg;
    pmu_rtc_calendar_get(&rtc_cal_cfg);
    rtc_cal_cfg.second = (10 + rtc_cal_cfg.second) % 60;
    struct RTC_CALENDAR_CFG_T alarm_time = rtc_cal_cfg;
#else
    static int cnt = 1;
    cnt++;
    struct RTC_CALENDAR_FORMAT_T alarm_time = _rtc_cfg_default;
    alarm_time.second += 10 * cnt;
#endif
    _pmu_rtc_alarm_set(&alarm_time);
    rtc_alarm_trace();
}

/* ###################### bootmode info output ###################### */

static void boot_info_output(void)
{
    // TRACE(0,"%d %d %d %d %d ", hal_hw_bootmode_get().reg, hal_hw_bootmode_get().charger, hal_hw_bootmode_get().global, hal_hw_bootmode_get().rtc, hal_hw_bootmode_get().watchdog);
    // TRACE(0,"%d %d ", hal_rom_hw_bootmode_get().reg, hal_sw_bootmode_get());
    if (hal_hw_bootmode_get().charger) {
        TRACE(0, "CMU BOOTMODE CHARGER");
    } else if (hal_hw_bootmode_get().global) {
        TRACE(0, "CMU BOOTMODE GLOBAL");
    } else if (hal_hw_bootmode_get().rtc) {
        TRACE(0, "CMU BOOTMODE RTC");
    } else if (hal_hw_bootmode_get().watchdog) {
        TRACE(0, "CMU BOOTMODE WATCHDOG");
    } else {
        TRACE(0, "CMU BOOTMODE RESET PIN or PMU WDT");
    }

    if(hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_MODE) {
        TRACE(0, "HAL_SW_BOOTMODE_TEST_MODE set");
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MODE);
    } else {
        TRACE(0, "HAL_SW_BOOTMODE_TEST_MODE clr");
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE);
    }
    TRACE(0, "SW_BOOTMODE_TEST = %d", !!(hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_MODE));
}

/* ###################### test rtc config RTC_TEST_MODE first###################### */

void test_rtc(void)
{
    int cnt = 0;

    hal_sys_timer_delay(MS_TO_TICKS(1000));
    boot_info_output();

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_24M);
    TRACE(0, "CPU frequency changed to: %uM (cal 5ms)", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);

    TRACE(0, "-----------enable pmu use external clock-----------");
    //pmu_external_crystal_enable();        // pmu 32.768KHz 3 paths output config are also here
    hal_sys_timer_calib();
    TRACE(0, "After the calibration, sys_clk = %d", hal_sys_timer_systick_hz());

    TRACE(0, "-----------rtc test ready-----------");
    for (int i=0; i < 5; i++) {
        rtc_calendar_trace();
        osDelay(RTC_CALENDAR_TRACE_INTERVAL);
    }

    TRACE_IMM(0, "-----------press pwr button to config rtc calendar-----------");
    pmu_set_irq_unified_handler(PMU_IRQ_TYPE_PWRKEY, hal_pwrkey_tester_irqhandler);
    pmu_pwrkey_set_irq(HAL_PWRKEY_IRQ_RISING_EDGE);

    TRACE(0, "-----------rtc test begin-----------");
    switch (RTC_TEST_MODE) {
    case TIME_ACCURACY_TEST_MODE:
        TRACE(0, "-----------rtc time accuracy test begin-----------");
        while (1) {
            rtc_calendar_trace();
            osDelay(RTC_CALENDAR_TRACE_INTERVAL);
        }

    case HW_RESET_TEST_MODE:
        TRACE(0, "-----------rtc hardware reset test begin-----------");
        TRACE(0, "-----------please press the reset button and view the calendar trace output-----------");

        while (1) {
            rtc_calendar_trace();
            osDelay(RTC_CALENDAR_TRACE_INTERVAL);
        }

    case SW_RESET_TEST_MODE:
        TRACE(0, "-----------rtc software reset test begin-----------");
        TRACE_IMM(0, "-----------soc will software reset after 10s-----------");

        while (1) {
            hal_sys_timer_delay(MS_TO_TICKS(RTC_CALENDAR_TRACE_INTERVAL));
            rtc_calendar_trace();

            cnt++;
            if (cnt > (10000 / RTC_CALENDAR_TRACE_INTERVAL)) {
                TRACE_IMM(0, "-----------soc will software reset -----------");
                pmu_reboot();
            }
        }
    case SYS_WDT_RESET_TEST_MODE:
        TRACE(0, "-----------rtc wdt test begin-----------");
        TRACE(0, "-----------sys will reset after 20s-----------");

        hal_wdt_set_timeout(HAL_WDT_ID_0, 20 / 2);
        hal_wdt_start(HAL_WDT_ID_0);

        while (1) {
            rtc_calendar_trace();
            osDelay(RTC_CALENDAR_TRACE_INTERVAL);
        }

    case PMU_WDT_TEST_MODE:
        TRACE(0, "-----------rtc pmu wdt test begin-----------");
        TRACE(0, "-----------pmu wdt will reset the soc after 4s-----------");

        pmu_wdt_config(0, 4000);
        pmu_wdt_start();

        while (1) {
            rtc_calendar_trace();
            osDelay(RTC_CALENDAR_TRACE_INTERVAL);
        }
    case RTC_ALARM_TEST_MODE:
        TRACE(0, "-----------rtc alarm test begin-----------");
        TRACE(0, "-----------cpu will enter deep sleep mode-----------");
        hal_sleep_start_stats(10000, 10000);
        pmu_rtc_calendar_alarm_irq_handler_set(rtc_irq_handler_test);
        do {
            struct RTC_CALENDAR_FORMAT_T alarm_time = _rtc_cfg_default;
            alarm_time.second = 10;
            _pmu_rtc_alarm_set(&alarm_time);
            struct RTC_CALENDAR_FORMAT_T rtc_cal_cfg = _rtc_cfg_default;
            _pmu_rtc_calendar_set(&rtc_cal_cfg);
        }while (0);
        hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
        while (1) {
            osSignalWait(0x0, osWaitForever);
        }
    case PMU_SHUTDOWN_ALARM_MODE:
        TRACE(0, "-----------rtc pmu shutdown test begin-----------");
        // TODO add 'rtc alarm will wakeup cpu from pmu shutdown' function
        do {
            struct RTC_CALENDAR_FORMAT_T alarm_time = _rtc_cfg_default;
            alarm_time.second = 10;
            _pmu_rtc_alarm_set(&alarm_time);
            struct RTC_CALENDAR_FORMAT_T rtc_cal_cfg = _rtc_cfg_default;
            _pmu_rtc_calendar_set(&rtc_cal_cfg);
        }while (0);
        pmu_shutdown();
        while (1);
    case RTC_AON_TIMER_CMP:
        TRACE(0, "-----------rtc aon timer compare test begin-----------");
        TRACE(0, "-----------cpu will enter deep sleep mode-----------");

        hal_sleep_start_stats(10000, 10000);
        hal_timer_setup(HAL_TIMER_TYPE_PERIODIC, acon_timer_periodic_irq_handler);
        hal_timer_start(MS_TO_TICKS(1000));
        hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
        while (1) {
            osSignalWait(0x0, 1000);
        }
    default:
        ASSERT(0,"rtc no such test mode");
    }
}
#endif
