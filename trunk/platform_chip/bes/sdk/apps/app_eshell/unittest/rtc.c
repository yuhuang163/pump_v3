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

#if (defined(UTILS_ESHELL_EN) && defined(__SYS_AS_MAIN__) && \
    (defined(CHIP_BEST1600))  || defined(CHIP_BEST1503))


#include "app_eshell.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "eshell.h"
#include "hal_bootmode.h"
#include "hal_gpio.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_wdt.h"
#include "pmu.h"
#include "string.h"
#include <stdlib.h>

#define RTC_MONTH_MAX (12)
#define RTC_MONTH_MIN (1)
#define RTC_DAY_MIN (1)
#define RTC_WEEK_MAX (7)
#define RTC_WEEK_MIN (1)
#define RTC_HOUR_MAX (23)
#define RTC_HOUR_MIN (0)
#define RTC_MINUTE_MAX (59)
#define RTC_MINUTE_MIN (0)
#define RTC_SECOND_MAX (59)
#define RTC_SECOND_MIN (0)

uint8_t table_month[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};
char table_week[7][3] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

static void rtc_calendar_read(void);

static bool rtc_calendar_check(struct RTC_CALENDAR_FORMAT_T *calendar)
{
    char month_buf[RTC_MONTH_MAX] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool ret = true;
    if (calendar->month > RTC_MONTH_MAX || calendar->month < RTC_MONTH_MIN) {
        ret = false;
        eshell_putstring(0, "invalid month!\r\n");
    }

    if (calendar->week > RTC_WEEK_MAX || calendar->week < RTC_WEEK_MIN) {
        ret = false;
        eshell_putstring(0, "invalid week!\r\n");
    }

    if (calendar->month == 2) {
        if (((calendar->year % 4 == 0) && (calendar->year % 100 != 0)) ||
            (calendar->year % 400 == 0)) {
            month_buf[1] += 1;
        }
    }

    if (calendar->day > month_buf[calendar->month - 1] || calendar->day < RTC_DAY_MIN) {
        ret = false;
        eshell_putstring(0, "invalid data!\r\n");
    }

    if ((calendar->hour > RTC_HOUR_MAX || calendar->minute > RTC_MINUTE_MAX ||
         calendar->second > RTC_SECOND_MAX)) {
        ret = false;
        eshell_putstring(0, "invalid time!\r\n");
    }
    return ret;
}

static uint8_t rtc_get_week(uint16_t year, uint8_t month, uint8_t day)
{
    uint16_t temp;
    uint8_t yearH, yearL;

    yearH = year / 100;
    yearL = year % 100;

    if (yearH > 19)
        yearL += 100;

    temp = yearL + yearL / 4;
    temp = temp % 7;
    temp = temp + day + table_month[month - 1];
    if ((yearL % 4 == 0) && (month < 3))
        temp--;
    return (temp % 7);
}

static void rtc_alarm_trace(void)
{
    struct RTC_CALENDAR_FORMAT_T alarm_time;
    memset(&alarm_time, 0, sizeof(alarm_time));
    pmu_rtc_calendar_alarm_get(&alarm_time);
    eshell_putstring("year/mouth/day/ week: %d/%02d/%02d %c%c%c\r\n", alarm_time.year,
                     alarm_time.month, alarm_time.day, table_week[alarm_time.week - 1][0],
                     table_week[alarm_time.week - 1][1], table_week[alarm_time.week - 1][2]);
    eshell_putstring("hours:min:sec %02d:%02d:%02d\r\n", alarm_time.hour, alarm_time.minute,
                     alarm_time.second);
}

static void rtc_irq_handler_test()
{
    eshell_putstring("rtc_irq_handler_test\r\n");
    rtc_alarm_trace();
}

static bool rtc_alarm_set(uint8_t hour, uint8_t minute, uint8_t second)
{
    struct RTC_CALENDAR_FORMAT_T cur_time;
    memset(&cur_time, 0, sizeof(cur_time));
    pmu_rtc_calendar_get(&cur_time);
    struct RTC_CALENDAR_FORMAT_T alarm_time = cur_time;
    alarm_time.hour = hour;
    alarm_time.minute = minute;
    alarm_time.second = second;
    if (!rtc_calendar_check(&alarm_time))
        return false;
    pmu_rtc_calendar_alarm_set(&alarm_time);
    eshell_putstring("----current time----\r\n");
    rtc_calendar_read();
    eshell_putstring("----alarm time----\r\n");
    rtc_alarm_trace();
    return true;
}

static void rtc_normal_alarm_set(uint8_t hour, uint8_t minute, uint8_t second)
{
    pmu_rtc_calendar_alarm_irq_handler_set(rtc_irq_handler_test);
    if (!rtc_alarm_set(hour, minute, second))
        return;
}

static void rtc_shutdown_alarm_set(uint8_t hour, uint8_t minute, uint8_t second)
{
    if (!rtc_alarm_set(hour, minute, second))
        return;
    eshell_putstring("system shutdown\r\n");
    pmu_rtc_calendar_pwron_enable(true);
    pmu_shutdown();
}

static void rtc_calendar_set(struct RTC_CALENDAR_FORMAT_T *time_set)
{

    time_set->week = rtc_get_week(time_set->year, time_set->month, time_set->day);
    if (!rtc_calendar_check(time_set))
        return;
    pmu_rtc_calendar_set(time_set);
}

static void rtc_calendar_read(void)
{
    struct RTC_CALENDAR_FORMAT_T cur_time;

    memset(&cur_time, 0, sizeof(cur_time));

    pmu_rtc_calendar_get(&cur_time);

    eshell_putstring("year/mouth/day/ week: %d/%02d/%02d/  %c%c%c\r\n", cur_time.year,
                     cur_time.month, cur_time.day, table_week[cur_time.week - 1][0],
                     table_week[cur_time.week - 1][1], table_week[cur_time.week - 1][2]);
    eshell_putstring("hours:min:sec %02d:%02d:%02d\r\n", cur_time.hour, cur_time.minute,
                     cur_time.second);
}

static void unitest_rtc(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("Usage: \r\n");
        eshell_putstring("  utest_rtc read\r\n");
        eshell_putstring("  utest_rtc set year month day [hour] [minute] [second]\r\n");
        eshell_putstring("  utest_rtc normal_alarm hour minute [second]\r\n");
        eshell_putstring("  utest_rtc shutdown_alarm hour minute [second]\r\n");
        return;
    }

    if (strncmp(argv[1], "read", 4) == 0) {
        rtc_calendar_read();
    } else if (strncmp(argv[1], "set", 3) == 0) {
        if (argc < 5)
            eshell_putstring("Calendar setting error\r\n");
        else {
            struct RTC_CALENDAR_FORMAT_T calendar = {
                0,
            };
            calendar.year = atoi(argv[2]);
            calendar.month = atoi(argv[3]);
            calendar.day = atoi(argv[4]);
            // calendar.week = 1;
            if (argc == 6) {
                calendar.hour = atoi(argv[5]);
                calendar.minute = 0;
                calendar.second = 0;
            } else if (argc == 7) {
                calendar.hour = atoi(argv[5]);
                calendar.minute = atoi(argv[6]);
                calendar.second = 0;
            } else if (argc > 7) {
                calendar.hour = atoi(argv[5]);
                calendar.minute = atoi(argv[6]);
                calendar.second = atoi(argv[7]);
            }
            rtc_calendar_set(&calendar);
        }
    } else if (strncmp(argv[1], "shutdown_alarm", 14) == 0) {
        if (argc < 4)
            eshell_putstring("shutdown_alarm setting error\r\n");
        else {
            uint8_t hour, minute, second;
            hour = atoi(argv[2]);
            minute = atoi(argv[3]);
            if (argc > 4)
                second = atoi(argv[4]);
            else
                second = 0;
            rtc_shutdown_alarm_set(hour, minute, second);
        }
    } else if (strncmp(argv[1], "normal_alarm", 12) == 0) {
        if (argc < 4)
            eshell_putstring("normal_alarm setting error\r\n");
        else {
            uint8_t hour, minute, second;
            hour = atoi(argv[2]);
            minute = atoi(argv[3]);
            if (argc > 4)
                second = atoi(argv[4]);
            else
                second = 0;
            rtc_normal_alarm_set(hour, minute, second);
        }
    } else
        eshell_putstring("eshell cmd error\r\n");
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_rtc", "usage: utest_rtc help", unitest_rtc);

#endif
