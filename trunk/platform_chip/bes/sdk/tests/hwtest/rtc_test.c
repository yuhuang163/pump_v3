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
#ifdef RTC_TEST
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_gpio.h"
#include "hal_key.h"
#include "hwtest.h"
#include "pmu.h"

static bool rtc_alarm_on = false;

#ifdef RTC_ENABLE
#define RTC_TEST_V1_START_TICK              30
#define RTC_TEST_V1_ALARM_TICK              50
#define RTC_TEST_V1_PWRON_TICK              60

static void rtc_test_v1_alarm_handler_test(uint32_t tick)
{
    TRACE(0, "%s tick=%d", __func__, tick);
    rtc_alarm_on = true;
}

static void rtc_test_v1(void)
{
    uint32_t rtc_tick_now;

    TRACE(0, "%s start~", __func__);

    TRACE(0, "RTC v1 tick=%u", pmu_rtc_get());

    TRACE(0, "%s start_tick=%u alarm_tick=%u pwron_tick=%u",
            __func__, RTC_TEST_V1_START_TICK, RTC_TEST_V1_ALARM_TICK, RTC_TEST_V1_PWRON_TICK);

    pmu_rtc_set_irq_handler(rtc_test_v1_alarm_handler_test);
    pmu_rtc_set_alarm(RTC_TEST_V1_ALARM_TICK);
    pmu_rtc_set(RTC_TEST_V1_START_TICK);
    pmu_rtc_enable();
    while(1) {
        rtc_tick_now = pmu_rtc_get();
        TRACE(0, "RTC v1 tick=%u", rtc_tick_now);
        if (rtc_alarm_on) {
            TRACE(0, "Chip will do soft power off and wait RTC power on");
            pmu_rtc_set_alarm(RTC_TEST_V1_PWRON_TICK);
            pmu_shutdown();
        }
        hal_sys_timer_delay(MS_TO_TICKS(1000));
    }
}

#elif defined(RTC_CALENDAR)
static struct RTC_CALENDAR_FORMAT_T rtc_calendar_base = {
    .year = 2022,
    .month = 12,
    .day = 31,
    .week = 6,
    .hour = 23,
    .minute = 59,
    .second = 30,
};

static struct RTC_CALENDAR_FORMAT_T rtc_calendar_set = {
    .year = 2023,
    .month = 12,
    .day = 31,
    .week = 1,
    .hour = 23,
    .minute = 59,
    .second = 59,
};

static struct RTC_CALENDAR_FORMAT_T rtc_calendar_alarm = {
    .year = 2023,
    .month = 1,
    .day = 1,
    .week = 7,
    .hour = 0,
    .minute = 0,
    .second = 0,
};

static struct RTC_CALENDAR_FORMAT_T rtc_calendar_alarm_pwron = {
    .year = 2023,
    .month = 1,
    .day = 1,
    .week = 7,
    .hour = 0,
    .minute = 0,
    .second = 10,
};

static void rtc_test_v2_calendar_print(struct RTC_CALENDAR_FORMAT_T *rtc_cal)
{
    TRACE(0, "year/month/day/week: %d-%02d-%02d %02d",
        rtc_cal->year, rtc_cal->month, rtc_cal->day, rtc_cal->week);
    TRACE(0, "hours/minute/second: %02d:%02d:%02d\n",
        rtc_cal->hour, rtc_cal->minute, rtc_cal->second);
}

static void rtc_test_v2_alarm_handler_test(struct RTC_CALENDAR_FORMAT_T *cal)
{
    TRACE(0, "%s: ", __func__);
    rtc_test_v2_calendar_print(cal);
    rtc_alarm_on = true;
}

static void rtc_test_v2(void)
{
    struct RTC_CALENDAR_FORMAT_T rtc_cal_now;

    TRACE(0, "%s: start~", __func__);

    for (int i = 0; i < 3; i++) {
        TRACE(0, "Get RTC current time!");
        pmu_rtc_calendar_get(&rtc_cal_now);
        rtc_test_v2_calendar_print(&rtc_cal_now);
        hal_sys_timer_delay(MS_TO_TICKS(1000));
    }

    TRACE(0, "[RTC_BASE_TIME]:");
    rtc_test_v2_calendar_print(&rtc_calendar_base);

    TRACE(0, "[RTC_RELOAD_TIME]:");
    rtc_test_v2_calendar_print(&rtc_calendar_set);

    TRACE(0, "[RTC_ALARM_IRQ_TIME]:");
    rtc_test_v2_calendar_print(&rtc_calendar_alarm);

    TRACE(0, "Set RTC alarm irq time!");
    pmu_rtc_calendar_alarm_irq_handler_set(rtc_test_v2_alarm_handler_test);
    pmu_rtc_calendar_alarm_set(&rtc_calendar_alarm);

    TRACE(0, "Set RTC base time!\n");
    pmu_rtc_calendar_set(&rtc_calendar_base);

    while(1) {
        TRACE(0, "Get RTC current time!");
        pmu_rtc_calendar_get(&rtc_cal_now);
        rtc_test_v2_calendar_print(&rtc_cal_now);
        if (rtc_alarm_on) {
            TRACE(0, "Chip will do soft power off and wait RTC alarm power on.");

            TRACE(0, "[RTC_ALARM_POWER_ON_TIME]:");
            rtc_test_v2_calendar_print(&rtc_calendar_alarm_pwron);

            TRACE(0, "Set RTC alarm power on time!");
            pmu_rtc_calendar_alarm_set(&rtc_calendar_alarm_pwron);

            TRACE(0, "Enable RTC alarm power on!");
            pmu_rtc_calendar_pwron_enable(true);

            pmu_shutdown();
        }
        hal_sys_timer_delay(MS_TO_TICKS(1000));
    }
}
#endif

static void rtc_test_rtc_reload(void)
{
#ifdef RTC_ENABLE
    pmu_rtc_set(RTC_TEST_V1_START_TICK);
#elif defined(RTC_CALENDAR)
    pmu_rtc_calendar_set(&rtc_calendar_set);
#endif
}

static void rtc_test_pmu_irq_handler_dummy(void)
{
}

static void rtc_test_pmu_wdt_reset(void)
{
    pmu_wdt_set_irq_handler(rtc_test_pmu_irq_handler_dummy);
    pmu_wdt_config(100, 100);
    pmu_wdt_start();
}

static int rtc_test_key_event_process(uint32_t key_code, uint8_t key_event)
{
    if (key_code == HAL_KEY_CODE_PWR) {
        if (key_event == HAL_KEY_EVENT_CLICK) {
            TRACE(0, "rtc_reload");
            rtc_test_rtc_reload();
        } else if (key_event == HAL_KEY_EVENT_DOUBLECLICK) {
            TRACE(0, "pmu_wdt_reset");
            rtc_test_pmu_wdt_reset();
        } else if (key_event == HAL_KEY_EVENT_LONGPRESS) {
            TRACE(0, "pmu_reboot");
            pmu_reboot();
        }
    }

    return 0;
}

void rtc_test(void)
{
    hwtest_register_key_callback(rtc_test_key_event_process);
    TRACE(0, "Power_key: HAL_KEY_EVENT_CLICK         rtc_reload");
    TRACE(0, "Power_key: HAL_KEY_EVENT_DOUBLECLICK   pmu_wdt_reset");
    TRACE(0, "Power_key: HAL_KEY_EVENT_LONGPRESS     pmu_reboot");

#ifdef RTC_ENABLE
    rtc_test_v1();
#elif defined(RTC_CALENDAR)
    rtc_test_v2();
#endif
}

#endif
