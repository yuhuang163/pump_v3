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
#ifdef OSC_WAKEUP_TEST
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hwtimer_list.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"

/* make_cmd
* 1503
* make -j T=noapp_test CURRENT_TEST=1 CHIP=best1503 OSC_WAKEUP_TEST=1 CLKOUT_PU_OSC=1 CLOCK_OUT_ID=HAL_CMU_CLOCK_OUT_MCU_SYS
*/

// cmu->SIMU_RES
#define CMU_OSC_WAKEUP_TEST_WT24M_US_SHIFT                  (0)
#define CMU_OSC_WAKEUP_TEST_WT24M_US_MASK                   (0xFFF << CMU_OSC_WAKEUP_TEST_WT24M_US_SHIFT)
#define CMU_OSC_WAKEUP_TEST_WT24M_US(n)                     (((n) & 0xFFF) << CMU_OSC_WAKEUP_TEST_WT24M_US_MASK)
#define CMU_OSC_WAKEUP_TEST_TIMER_CNT_SHIFT                 (12)
#define CMU_OSC_WAKEUP_TEST_TIMER_CNT_MASK                  (0xFFF << CMU_OSC_WAKEUP_TEST_TIMER_CNT_SHIFT)
#define CMU_OSC_WAKEUP_TEST_TIMER_CNT(n)                    (((n) & 0xFFF) << CMU_OSC_WAKEUP_TEST_TIMER_CNT_SHIFT)
#define CMU_OSC_WAKEUP_TEST_TIMER_CNT_BYPASS                (1 << 24)
#define CMU_OSC_WAKEUP_TEST_TIMER_PERIOD_MS_SHIFT           (25)
#define CMU_OSC_WAKEUP_TEST_TIMER_PERIOD_MS_MASK            (0x1F << CMU_OSC_WAKEUP_TEST_TIMER_PERIOD_MS_SHIFT)
#define CMU_OSC_WAKEUP_TEST_TIMER_PERIOD_MS(n)              (((n) & 0x1F) << CMU_OSC_WAKEUP_TEST_TIMER_PERIOD_MS_SHIFT)
#define CMU_OSC_WAKEUP_TEST_RESERVED                        (1 << 29)
#define CMU_OSC_WAKEUP_TEST_CFG_DONE                        (1 << 30)
#define CMU_OSC_WAKEUP_TEST_READY                           (1 << 31)

static HWTIMER_ID wakeup_timer = NULL;

static uint32_t wakeup_timer_cnt = 100;
static uint32_t wakeup_timer_period_ms = 20;

static void osc_wakeup_test_init(void)
{
    hal_cmu_simu_set_val(CMU_OSC_WAKEUP_TEST_READY);
}

static bool osc_wakeup_test_check_cfg_done(void)
{
    return !!(hal_cmu_simu_get_val() & CMU_OSC_WAKEUP_TEST_CFG_DONE);
}

static uint32_t osc_wakeup_test_get_timer_loop_cnt(void)
{
    return GET_BITFIELD(hal_cmu_simu_get_val(), CMU_OSC_WAKEUP_TEST_TIMER_CNT);
}

static uint32_t osc_wakeup_test_get_timer_period_ms(void)
{
    return GET_BITFIELD(hal_cmu_simu_get_val(), CMU_OSC_WAKEUP_TEST_TIMER_PERIOD_MS);
}

static uint32_t osc_wakeup_test_get_wt24m_us(void)
{
    return GET_BITFIELD(hal_cmu_simu_get_val(), CMU_OSC_WAKEUP_TEST_WT24M_US);
}

static void wakeup_timer_handler(void *param)
{
    static uint32_t cnt = 0;

    cnt++;

    if (wakeup_timer_cnt != 0 && cnt == wakeup_timer_cnt) {
        TRACE(0, "%s pass", __func__);
        hal_cmu_simu_pass();
        while (1) {}
    }

    hwtimer_start(wakeup_timer, MS_TO_TICKS(wakeup_timer_period_ms));
}

void osc_wakeup_test(void)
{
    uint32_t wt24m_us;
    uint32_t timer_period_ms;
    int ret;

    osc_wakeup_test_init();

    TRACE(0, "%s: ready", __func__);

    while (osc_wakeup_test_check_cfg_done() == false);

    wt24m_us = osc_wakeup_test_get_wt24m_us();
    ASSERT(wt24m_us, "%s: Bad wt24m_us=%d", __func__, wt24m_us);

    ret = hal_cmu_osc_wakeup_test_reload_wt24m_us(wt24m_us);
    ASSERT(!ret, "%s: Set wt24m fail! ret=%d", __func__, ret);

    timer_period_ms = osc_wakeup_test_get_timer_period_ms();

    wakeup_timer_period_ms = timer_period_ms > 0 ? timer_period_ms : wakeup_timer_period_ms;

    wakeup_timer_cnt = osc_wakeup_test_get_timer_loop_cnt();

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);

    TRACE(0, "%s: timer_period=%dms timer_cnt=%d wt24m_us=%d", __func__, wakeup_timer_period_ms, wakeup_timer_cnt, wt24m_us);

    wakeup_timer = hwtimer_alloc(wakeup_timer_handler, 0);
    hwtimer_start(wakeup_timer, MS_TO_TICKS(wakeup_timer_period_ms));

    while (1) {
        hal_sleep_enter_sleep();
    }
}
#endif
