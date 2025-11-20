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
#ifdef BOOT_REASON_TEST
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_gpio.h"
#include "hal_wdt.h"
#include "pmu.h"

/* make
*
* 1809
* make T=noapp_test -j BTH_AS_MAIN_MCU=1 CHIP_SUBSYS=bth CHIP=best1603 CURRENT_TEST=1 BOOT_REASON_TEST=1 DEBUG_PORT=1 RTC_CALENDAR=1 CHG_OUT_PWRON=1
*
* 1808
* make T=noapp_test -j BTH_AS_MAIN_MCU=1 CHIP_SUBSYS=bth CHIP=best1603 CURRENT_TEST=1 BOOT_REASON_TEST=1 DEBUG_PORT=1 RTC_ENABLE=1 CHG_OUT_PWRON=1
*
* 1503
* make T=noapp_test -j CHIP=best1503 CURRENT_TEST=1 BOOT_REASON_TEST=1 DEBUG_PORT=1 RTC_CALENDAR=1 CHG_OUT_PWRON=1
*/

static void boot_reason_test_pmu_wdt_irq_handler_dummy(void)
{
}

static void boot_reason_test_pmu_rtc_alarm_power_on(void)
{
#ifdef RTC_ENABLE
    pmu_rtc_set_alarm(38);
    pmu_rtc_set(30);
    pmu_rtc_enable();
    while(1) {
        TRACE(0, "RTC v1 tick=%d", pmu_rtc_get());
        if (pmu_rtc_get() == 35) {
            TRACE_IMM(0, "Chip will do soft power off and wait RTC power on. power_on_tick=38");
            pmu_shutdown();
        }
        hal_sys_timer_delay(MS_TO_TICKS(500));
    }
#elif defined(RTC_CALENDAR)
    static struct RTC_CALENDAR_FORMAT_T rtc_calendar_base = {
        .year = 2022,
        .month = 12,
        .day = 31,
        .week = 6,
        .hour = 23,
        .minute = 59,
        .second = 50,
    };

    static struct RTC_CALENDAR_FORMAT_T rtc_calendar_alarm_pwron = {
        .year = 2022,
        .month = 12,
        .day = 31,
        .week = 6,
        .hour = 23,
        .minute = 59,
        .second = 58,
    };

    struct RTC_CALENDAR_FORMAT_T rtc_cal_now;

    pmu_rtc_calendar_set(&rtc_calendar_base);

    while(1) {
        pmu_rtc_calendar_get(&rtc_cal_now);
        TRACE(0, "RTC V2 tick=%d", rtc_cal_now.second);
        if (rtc_cal_now.second == 55) {
            TRACE(0, "Chip will do soft power off and wait RTC power on");
            pmu_rtc_calendar_alarm_set(&rtc_calendar_alarm_pwron);
            pmu_rtc_calendar_pwron_enable(true);
            pmu_shutdown();
        }
        hal_sys_timer_delay(MS_TO_TICKS(500));
    }
#endif
}

static void boot_reason_print(enum PMU_BOOT_CAUSE_T cause)
{
    if (cause == PMU_BOOT_CAUSE_NULL) {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_NULL");
    } else if (cause == PMU_BOOT_CAUSE_POWER_KEY) {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_POWER_KEY");
    } else if (cause == PMU_BOOT_CAUSE_DIG_WDT) {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_DIG_WDT");
    } else if (cause == PMU_BOOT_CAUSE_DIG_REBOOT) {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_DIG_REBOOT");
    } else if (cause == PMU_BOOT_CAUSE_AC_IN) {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_AC_IN");
    } else if (cause == PMU_BOOT_CAUSE_AC_OUT) {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_AC_OUT");
    } else if (cause == PMU_BOOT_CAUSE_RTC) {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_RTC");
    } else if (cause == PMU_BOOT_CAUSE_PMU_WDT) {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_PMU_WDT");
    } else if (cause == PMU_BOOT_CAUSE_RESET_KEY) {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_RESET_KEY");
    } else if (cause == PMU_BOOT_CAUSE_LONG_PRESS) {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_LONG_PRESS");
    } else if (cause == PMU_BOOT_CAUSE_VCHG_COMMAND) {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_VCHG_COMMAND");
    } else if (cause == PMU_BOOT_CAUSE_VCORE_LOW) {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_VCORE_LOW");
    } else {
        TRACE(0, "boot reason: PMU_BOOT_CAUSE_VBAT_OVP");
    }
}

void boot_reason_test(void)
{
    enum PMU_BOOT_CAUSE_T boot_reason;

    boot_reason = pmu_boot_cause_get();
    boot_reason_print(boot_reason);

    ASSERT(boot_reason != PMU_BOOT_CAUSE_NULL, "%s Invalid boot reason!", __func__);

    if (boot_reason == PMU_BOOT_CAUSE_POWER_KEY) {
        TRACE(0, "next reason: DIG WDT");
        hal_wdt_set_timeout(HAL_WDT_ID_0, 2);
        hal_wdt_start(HAL_WDT_ID_0);
        while(1) {
            TRACE(0, "Wait DIG WDT reset! 2s");
            hal_sys_timer_delay(MS_TO_TICKS(500));
        }
    } else if (boot_reason == PMU_BOOT_CAUSE_DIG_WDT) {
        TRACE(0, "next reason: DIG REBOOT");
        hal_sys_timer_delay(MS_TO_TICKS(1000));
        pmu_reboot();
    } else if (boot_reason == PMU_BOOT_CAUSE_DIG_REBOOT) {
        TRACE(0, "next reason: AC IN");
        hal_sys_timer_delay(MS_TO_TICKS(1000));
        pmu_shutdown();
    } else if (boot_reason == PMU_BOOT_CAUSE_AC_IN) {
        TRACE(0, "next reason: AC OUT");
        hal_sys_timer_delay(MS_TO_TICKS(1000));
        pmu_shutdown();
    } else if (boot_reason == PMU_BOOT_CAUSE_AC_OUT) {
        TRACE(0, "next reason: RTC");
        boot_reason_test_pmu_rtc_alarm_power_on();
    } else if (boot_reason == PMU_BOOT_CAUSE_RTC) {
        TRACE(0, "next reason: PMU WDT");
        pmu_wdt_set_irq_handler(boot_reason_test_pmu_wdt_irq_handler_dummy);
        pmu_wdt_config(1000, 1000);
        pmu_wdt_start();
        while(1) {
            TRACE(0, "Wait PMU WDT reset! 2s");
            hal_sys_timer_delay(MS_TO_TICKS(500));
        }
    } else if (boot_reason == PMU_BOOT_CAUSE_PMU_WDT) {
        TRACE(0, "next reason: RESET KEY");
        while(1) {
            TRACE(0, "Click reset key!");
            hal_sys_timer_delay(MS_TO_TICKS(500));
        }
    } else if (boot_reason == PMU_BOOT_CAUSE_RESET_KEY) {
        TRACE(0, "next reason: LONG PRESS");
        pmu_power_key_hw_reset_enable(2);
        while(1) {
            TRACE(0, "Long press power key for 2s");
            hal_sys_timer_delay(MS_TO_TICKS(500));
        }
    } else if (boot_reason == PMU_BOOT_CAUSE_LONG_PRESS) {
        TRACE(0, "next reason: ACIN COMMAND");
        while(1) {
            TRACE(0, "Wait ACIN pattern reset!");
            hal_sys_timer_delay(MS_TO_TICKS(500));
        }
    } else if (boot_reason == PMU_BOOT_CAUSE_VCHG_COMMAND) {
        TRACE(0, "Boot reason test done! Click power key for the next test.");
        hal_sys_timer_delay(MS_TO_TICKS(1000));
        pmu_shutdown();
    }
}
#endif