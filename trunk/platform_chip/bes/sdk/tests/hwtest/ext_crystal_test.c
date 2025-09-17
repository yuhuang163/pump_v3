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
#ifdef EXT_CRYSTAL_TEST
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_key.h"
#include "hwtest.h"
#include "pmu.h"

/*
*
* CALIB_SLOW_TIMER=1
* PMU_CLK_USE_EXT_CRYSTAL=1
* #define CONFIG_SYSTICK_HZ_NOMINAL     (32768 / 2)
*
*/

#define LONGPRESS_PWRKEY_RESET_TIME_S           5

static void pmu_wdt_irq_handler_dummy(void)
{
}

static void ext_crystal_test_pmu_wdt_reset(void)
{
    pmu_wdt_set_irq_handler(pmu_wdt_irq_handler_dummy);
    pmu_wdt_config(100, 100);
    pmu_wdt_start();
}

static int ext_crystal_test_key_event_process(uint32_t key_code, uint8_t key_event)
{
    if (key_code == HAL_KEY_CODE_PWR) {
        if (key_event == HAL_KEY_EVENT_CLICK) {
            pmu_shutdown();
        }
    }

    return 0;
}

void ext_crystal_test(void)
{
    int ret;
    uint32_t delay_ms = 0;
    uint32_t s_time;

    if (CONFIG_SYSTICK_HZ_NOMINAL != 32768 / 2) {
        ASSERT(false, "%s: Please define CONFIG_SYSTICK_HZ_NOMINAL (32768 / 2)", __func__);
    }

    pmu_external_crystal_clkout_enable(PMU_EXT_CRYSTAL_CLKOUT_PIN_0);
    pmu_external_crystal_clkout_enable(PMU_EXT_CRYSTAL_CLKOUT_PIN_1);
    pmu_external_crystal_clkout_enable(PMU_EXT_CRYSTAL_CLKOUT_PIN_2);

    s_time = hal_fast_sys_timer_get();
    while(1) {
        ret = pmu_external_crystal_calib(&delay_ms);
        hal_sys_timer_delay(MS_TO_TICKS(delay_ms));
        if (ret == 0) {
            break;
        } else if (ret == -2 || ret == -3) {
            ASSERT(false, "%s: Bad calibration! ret:%d", __func__, ret);
        }
    }
    TRACE(0, "%s: Calibration cost time:%ums", __func__, FAST_TICKS_TO_MS(hal_fast_sys_timer_get() - s_time));

    pmu_power_key_hw_reset_enable(LONGPRESS_PWRKEY_RESET_TIME_S);
    TRACE(0, "%s: Enable long press pwrkey reset function! seconds:%ds", __func__, LONGPRESS_PWRKEY_RESET_TIME_S);

    if (pmu_boot_cause_get() != PMU_BOOT_CAUSE_PMU_WDT && pmu_boot_cause_get() != PMU_BOOT_CAUSE_LONG_PRESS) {
        ext_crystal_test_pmu_wdt_reset();
    }

    hwtest_register_key_callback(ext_crystal_test_key_event_process);
    TRACE(0, "POWERKEY: HAL_KEY_EVENT_CLICK         pmu_shutdown");
}
#endif
