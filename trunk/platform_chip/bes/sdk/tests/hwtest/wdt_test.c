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
#include "hwtest.h"
#include "cmsis.h"
#include "hal_wdt.h"
#include "hal_sleep.h"

static void wdt_irq_handle(enum HAL_WDT_ID_T id, enum HAL_WDT_EVENT_T event)
{
    TRACE(3,"%s id:%d event:%d",__func__, id, event);
}

static int wdt_kick(void)
{
    hal_wdt_ping(HAL_WDT_ID_0);
    return 0;
}

static int wdt_open(int seconds)
{
    hal_wdt_set_irq_callback(HAL_WDT_ID_0, wdt_irq_handle);
    hal_wdt_set_timeout(HAL_WDT_ID_0, seconds);
    hal_wdt_start(HAL_WDT_ID_0);
    hal_sleep_set_deep_sleep_hook(HAL_DEEP_SLEEP_HOOK_USER_WDT, wdt_kick);
    return 0;
}

static int wdt_feed_cnt = 0;

void wdt_test(void)
{
    wdt_open(4);

    while (1) {
        TRACE(2,"%s, %d", __func__, GET_CURRENT_MS());
        wdt_feed_cnt++;
        hal_sys_timer_delay(MS_TO_TICKS(wdt_feed_cnt*1000));
        wdt_kick();
    }
}

