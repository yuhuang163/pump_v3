/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "wdt_tester.h"

static uint32_t feedPeriod, timeoutTime;
static enum HAL_WDT_ID_T id = HAL_WDT_ID_0;

static void wdt_irq_handler(enum HAL_WDT_ID_T id, enum HAL_WDT_EVENT_T event)
{
    CMD_TRACE("id: %d\tevent: %d", id, event);
}

static void wdt_tester_wdt_init()
{
    hal_wdt_set_irq_callback(id, wdt_irq_handler);
    hal_wdt_set_timeout(id, timeoutTime);
    hal_wdt_start(id);
}

void wdt_tester(const char **params)
{
    /* set feed period and timeout time */
    ASSIGN_VAL(feedPeriod,  params[0]);
    ASSIGN_VAL(timeoutTime, params[1]);

    CMD_TRACE("feed period (s) :\t\t%d", feedPeriod);
    CMD_TRACE("timeout time (s):\t\t%d", timeoutTime);

    /* watchdog timer initialization */
    wdt_tester_wdt_init();

    /* feed watchdog timer at specific time period */
    for (; timeoutTime; timeoutTime--) {
        hal_sys_timer_delay(MS_TO_TICKS(feedPeriod * 1000));
        CMD_TRACE("reset the watchdog timer");
        hal_wdt_ping(id);
    }
}