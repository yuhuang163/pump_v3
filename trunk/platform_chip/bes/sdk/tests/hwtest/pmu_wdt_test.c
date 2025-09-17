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
#ifdef PMU_WDT_TEST
#include "hwtest.h"
#include "cmsis.h"
#include "pmu.h"

#define PMU_WDT_IRQ_MS              1000
#define PMU_WDT_RESET_MS            1000
#define PMU_WDT_FEED_PERIOD_MS      (PMU_WDT_IRQ_MS / 2)
#define PMU_WDT_FEED_CNT            10

static void _wdt_irq_handler(void)
{
    TRACE(0, "%s", __func__);
}

static void do_feed_dog(void)
{
    TRACE(0, "%s", __func__);
    pmu_wdt_feed();
}

void pmu_wdt_test(void)
{
    pmu_open();

    pmu_wdt_config(PMU_WDT_IRQ_MS, PMU_WDT_RESET_MS);
    pmu_wdt_set_irq_handler(_wdt_irq_handler);
    pmu_wdt_start();

    for (int i = 0; i < PMU_WDT_FEED_CNT; i++) {
        if (i == (PMU_WDT_FEED_CNT - 1)) {
            TRACE(0, "%s feed wdt pass, wait until system reset", __func__);
        } else {
            hal_sys_timer_delay(MS_TO_TICKS(PMU_WDT_FEED_PERIOD_MS));
            do_feed_dog();
        }
    }

    for(;;);
}

#endif

