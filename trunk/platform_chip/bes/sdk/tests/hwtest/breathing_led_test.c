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
#ifdef BREATHING_LED_TEST

#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sleep.h"
#include "pmu.h"

void breathing_led_test(void)
{
    TRACE_IMM(0, "  ");
    TRACE_IMM(0, "------pmu led breathing test started------");
    TRACE_IMM(0, "Please use an oscilloscope to view the set pin(led1/led2) waveform!");
    TRACE_IMM(0, "***log will stop output***");
    TRACE_IMM(0, "  ");

    struct PMU_LED_BR_CFG_T pmu_led_breathing_cfg = {
        .off_time_ms = 500,
        .on_time_ms  = 400,
        .fade_time_ms = 600,
    };

    pmu_led_breathing_enable(HAL_IOMUX_PIN_LED1, &pmu_led_breathing_cfg);//pin led1

    pmu_led_breathing_cfg.off_time_ms = 3000;
    pmu_led_breathing_cfg.on_time_ms = 2000;
    pmu_led_breathing_cfg.fade_time_ms = 2000;
    pmu_led_breathing_enable(HAL_IOMUX_PIN_LED2, &pmu_led_breathing_cfg);//pin led2
    while (1) {
        hal_sleep_enter_sleep();
    }
}

#endif

