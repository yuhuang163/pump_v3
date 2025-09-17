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
#ifdef TEST_PWM
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_pwm.h"
#include "hal_cmu.h"
//#ifndef FPGA
#include "hal_iomux.h"
//#endif
#include "cmsis_os2.h"
#define PWM_ID_DEFAULT                  (HAL_PWM_ID_0)
#ifndef TEST_PWM_FREQ
#define TEST_PWM_FREQ                   (1000000)
#endif
#ifndef TEST_PWM_RATIO
#define TEST_PWM_RATIO                  (10)
#endif
/* true -- slow sck;
 * false -- fast clk
 */
#ifndef TEST_PWM_CLK_SRC
#define TEST_PWM_CLK_SRC        (false)
#endif

static void pwm_iomux(enum HAL_PWM_ID_T id)
{
    switch(id) {
        case HAL_PWM_ID_0:
            hal_iomux_set_pwm0();
            break;
        case HAL_PWM_ID_1:
            hal_iomux_set_pwm1();
            break;
        case HAL_PWM_ID_2:
            hal_iomux_set_pwm2();
            break;
        case HAL_PWM_ID_3:
            hal_iomux_set_pwm3();
            break;
        default:
            break;
    }
}

static int pwm_init(void)
{
    int ret = 0;

    struct HAL_PWM_CFG_T pwm_cfg = {
        .freq = TEST_PWM_FREQ,
        .ratio = TEST_PWM_RATIO,
        .inv = false,
        .sleep_on = TEST_PWM_CLK_SRC,
    };

    TRACE(0,"PWM1 test has started... TEST_PWM_FREQ:%d", TEST_PWM_FREQ);
    TRACE(0,"Please use an oscilloscope to view the set pin waveform!");

    /* IOMUX: pwm 0~3 -- GPIO S4~7 */
    pwm_iomux(PWM_ID_DEFAULT);

    ret = hal_pwm_enable(PWM_ID_DEFAULT, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM: %d", ret);

    osDelay(1000);
    
    hal_pwm_disable(PWM_ID_DEFAULT);

    osDelay(1000);
    pwm_cfg.ratio = 25;

    ret = hal_pwm_enable(PWM_ID_DEFAULT, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM: %d", ret);

    osDelay(1000);
    
    hal_pwm_disable(PWM_ID_DEFAULT);

    osDelay(1000);
    pwm_cfg.ratio = 33;

    ret = hal_pwm_enable(PWM_ID_DEFAULT, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM: %d", ret);

    osDelay(1000);
    
    hal_pwm_disable(PWM_ID_DEFAULT);

    osDelay(1000);
    pwm_cfg.ratio = 50;

    ret = hal_pwm_enable(PWM_ID_DEFAULT, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM: %d", ret);

    osDelay(1000);
    
    hal_pwm_disable(PWM_ID_DEFAULT);

    osDelay(1000);
    pwm_cfg.ratio = 75;

    ret = hal_pwm_enable(PWM_ID_DEFAULT, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM: %d", ret);

    osDelay(1000);
    
    hal_pwm_disable(PWM_ID_DEFAULT);

    osDelay(1000);
    pwm_cfg.ratio = 90;

    ret = hal_pwm_enable(PWM_ID_DEFAULT, &pwm_cfg);
    ASSERT(ret == 0, "Failed to enable PWM: %d", ret);
    return ret;
}

static int pwm_deinit(void)
{
    return hal_pwm_disable(PWM_ID_DEFAULT);
}

int test_pwm(void)
{
    pwm_init();

    while(1) {
    }

    return pwm_deinit();
}
#endif

