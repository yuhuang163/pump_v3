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
#ifdef TEST_WDT
#include "hal_sleep.h"
#include "hal_trace.h"
//#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_wdt.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "cmsis.h"

#define WDT_TIMEOUT_DEFAULT                 (10)


static void wdt_irq_handle(enum HAL_WDT_ID_T id, enum HAL_WDT_EVENT_T event)
{
    TRACE(3,"%s id:%d event:%d",__func__, id, event);
}

static int wdt_kick(void)
{
    static uint8_t cnt = 0;
    hal_wdt_ping(HAL_WDT_ID_0);
    TRACE(3,"feeded dog. times: %d!!!", ++cnt);
    return 0;
}

static int wdt_open(int seconds)
{
    hal_wdt_set_irq_callback(HAL_WDT_ID_0, wdt_irq_handle);
    hal_wdt_set_timeout(HAL_WDT_ID_0, seconds);
    hal_wdt_start(HAL_WDT_ID_0);
    return 0;
}

//static int wdt_feed_cnt = 0;
static int wdt_feed_num = 60;

//前一分钟喂狗
//然后不喂狗，等待超时，watchdog中断
void test_wdt(void)
{
    wdt_open(WDT_TIMEOUT_DEFAULT);
    TRACE(2,"WDT ADDR BASE[0x%08x] wdt open[timeout:%ds].", WDT_BASE, WDT_TIMEOUT_DEFAULT);
#if 0
    TRACE_IMM(0, "int lock");
    int_lock();
#endif
    while (wdt_feed_num) {
        //TRACE(2,"%s, running: %dms", __func__, GET_CURRENT_MS());
        //wdt_feed_cnt++;
		wdt_feed_num--;
        TRACE(2,"dealy %dms, and then feeding dog.", (60 - wdt_feed_num) * 1000);
        hal_sys_timer_delay(MS_TO_TICKS(1000));
        // osDelay(1000);
        wdt_kick();
    }
	
	TRACE(2,"wait wdt timeout...%dms", WDT_TIMEOUT_DEFAULT * 1000);
	while(1) {
		hal_sys_timer_delay(MS_TO_TICKS(1000));
        // osDelay(1000);
		TRACE(2,"delay 1000ms...");
	}
}

#endif
