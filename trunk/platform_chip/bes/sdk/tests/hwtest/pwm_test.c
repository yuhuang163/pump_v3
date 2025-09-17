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
#ifdef PWM_TEST

#include "hwtest.h"
#include "hal_pwm.h"
#include "hal_cmu.h"
#ifndef FPGA
#include "hal_iomux.h"
#endif

//#define ENABLE_PWM_BREATHING_LED_TEST

void pwm_test(void)
{
    TRACE(0, "  ");

#if defined(CHIP_BEST2300P)
    while (1) {
        TRACE(0, "***1302 chip does not support this test example temporarily");
        hal_sys_timer_delay(MS_TO_TICKS(500));
    }
#endif

    int ret;
    struct HAL_PWM_CFG_T pwm_cfg = {
        .freq = 8000,
        .ratio = 25,
        .inv = false,
        .sleep_on = false,
    };

    TRACE(0, "PWM test has started...");
    TRACE(0, "Please use an oscilloscope to view the set pin waveform!");
#ifndef FPGA
    hal_iomux_set_pwm0();
    hal_iomux_set_pwm1();
    hal_iomux_set_pwm2();
    hal_iomux_set_pwm3();
#ifdef PWM1_BASE
    hal_iomux_set_pwm4();
    hal_iomux_set_pwm5();
    hal_iomux_set_pwm6();
    hal_iomux_set_pwm7();
#endif
#endif

    ret = hal_pwm_enable(HAL_PWM_ID_0, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM0: %d", ret);

    pwm_cfg.freq = 4000;
    ret = hal_pwm_enable(HAL_PWM_ID_1, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM1: %d", ret);

    pwm_cfg.freq = 2000;
    ret = hal_pwm_enable(HAL_PWM_ID_2, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM2: %d", ret);

    pwm_cfg.freq = 1000;
    ret = hal_pwm_enable(HAL_PWM_ID_3, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM3: %d", ret);

#ifdef PWM1_BASE
    pwm_cfg.freq = 8000;
    ret = hal_pwm_enable(HAL_PWM1_ID_0, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM0: %d", ret);

    pwm_cfg.freq = 4000;
    ret = hal_pwm_enable(HAL_PWM1_ID_1, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM1: %d", ret);

    pwm_cfg.freq = 2000;
    ret = hal_pwm_enable(HAL_PWM1_ID_2, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM2: %d", ret);

    pwm_cfg.freq = 1000;
    ret = hal_pwm_enable(HAL_PWM1_ID_3, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM3: %d", ret);
#endif

#ifdef ENABLE_PWM_BREATHING_LED_TEST
    //breathing led test,tanwenchen,2021-02-19
    struct HAL_PWM_BR_CFG_T pwm_br_cfg = {
        .off_time_ms = 100,
        .on_time_ms  = 100,
        .fade_time_ms = 1500,
    };

    ret = hal_pwm_breathing_led_enable(HAL_PWM_ID_0, &pwm_br_cfg);
    TRACE(1, "open breath0:%d,%s", ret, ret ? "error" : "success");
    ret = hal_pwm_breathing_led_enable(HAL_PWM_ID_1, &pwm_br_cfg);
    TRACE(1, "open breath1:%d,%s", ret, ret ? "error" : "success");
    ret = hal_pwm_breathing_led_enable(HAL_PWM_ID_2, &pwm_br_cfg);
    TRACE(1, "open breath2:%d,%s", ret, ret ? "error" : "success");
    ret = hal_pwm_breathing_led_enable(HAL_PWM_ID_3, &pwm_br_cfg);
    TRACE(1, "open breath3:%d,%s", ret, ret ? "error" : "success");
#ifdef PWM1_BASE
    ret = hal_pwm_breathing_led_enable(HAL_PWM1_ID_0, &pwm_br_cfg);
    TRACE(1, "open breath4:%d,%s", ret, ret ? "error" : "success");
    ret = hal_pwm_breathing_led_enable(HAL_PWM1_ID_1, &pwm_br_cfg);
    TRACE(1, "open breath5:%d,%s", ret, ret ? "error" : "success");
    ret = hal_pwm_breathing_led_enable(HAL_PWM1_ID_2, &pwm_br_cfg);
    TRACE(1, "open breath6:%d,%s", ret, ret ? "error" : "success");
    ret = hal_pwm_breathing_led_enable(HAL_PWM1_ID_3, &pwm_br_cfg);
    TRACE(1, "open breath7:%d,%s", ret, ret ? "error" : "success");
#endif

#endif
}

#endif
