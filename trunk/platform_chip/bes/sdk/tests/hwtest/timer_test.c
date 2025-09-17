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

#ifdef USER_TIMER_TEST
#define IGNORE_HAL_TIMER_RAW_API_CHECK
#endif
#include "hal_timer_raw.h"
#include "hal_timer_user_irq.h"

#define TIMER_DIV   ((6 * 1000 * 1000) / (16 * 1000))
// TIMER_TEST
static void oneshot_timer_handler(uint32_t elapsed)
{
    TRACE(2,"%s: elapsed=%d", __FUNCTION__, elapsed);
}

static void periodic_timer_handler(uint32_t elapsed)
{
    TRACE(2,"%s: elapsed=%d", __FUNCTION__, elapsed);
}

static void hal_timer_setup_test(enum HAL_TIMER_TYPE_T type, HAL_TIMER_IRQ_HANDLER_T handler)
{
#ifndef USER_TIMER_TEST
    hal_timer_setup(type, handler);
#else
    hal_user_timer_setup(type, handler);
#endif
}

static void hal_timer_start_test(uint32_t load)
{
#ifndef USER_TIMER_TEST
    hal_timer_start(load);
#else
    hal_user_timer_start(load * TIMER_DIV);
#endif
}

static void hal_timer_reload_test(uint32_t load)
{
#ifndef USER_TIMER_TEST
    hal_timer_reload(load);
#else
    hal_user_timer_reload(load * TIMER_DIV);
#endif
}

static void hal_timer_stop_test(void)
{
#ifndef USER_TIMER_TEST
    hal_timer_stop();
#else
    hal_user_timer_stop();
#endif
}

void timer_test(void)
{
    uint32_t cur_ticks = hal_sys_timer_get();
    uint32_t st;

    TRACE(2,"SYSTIMER: %u ticks, %u ms", cur_ticks, TICKS_TO_MS(cur_ticks));
#if 0
    hal_sys_timer_delay(MS_TO_TICKS(10000));
    cur_ticks = hal_sys_timer_get();
    TRACE(2,"SYSTIMER: %u ticks, %u ms", cur_ticks, TICKS_TO_MS(cur_ticks));
#endif
    hal_timer_setup_test(HAL_TIMER_TYPE_ONESHOT, oneshot_timer_handler);
    hal_timer_start_test(0);
    hal_sys_timer_delay(3);
    hal_timer_start_test(20);
    TRACE(0,"TIMER: oneshot start 20");
    hal_sys_timer_delay(3);
    hal_timer_reload_test(3);
    TRACE(0,"TIMER: oneshot reload 3");
    st = int_lock();
    hal_sys_timer_delay(8);
    int_unlock(st);
    hal_timer_start_test(2);
    TRACE(0,"TIMER: oneshot start 2");
    st = int_lock();
    hal_sys_timer_delay(5);
    int_unlock(st);
    TRACE(0,"TIMER: oneshot end");

    hal_timer_setup_test(HAL_TIMER_TYPE_PERIODIC, periodic_timer_handler);
    hal_timer_start_test(30);
    TRACE(0,"TIMER: periodic start 30");
    hal_sys_timer_delay(2);
    hal_timer_reload_test(3);
    TRACE(0,"TIMER: periodic reload 3");
    st = int_lock();
    hal_sys_timer_delay(4);
    int_unlock(st);
    hal_sys_timer_delay(4);
    st = int_lock();
    TRACE(0,"TIMER: periodic int lock delay 8");
    hal_sys_timer_delay(8);
    int_unlock(st);
    hal_timer_stop_test();
}


