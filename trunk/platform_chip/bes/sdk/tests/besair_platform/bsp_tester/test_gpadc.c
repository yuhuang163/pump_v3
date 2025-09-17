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
#ifdef TEST_GPADC
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_iomux.h"
#include "hal_psc.h"
#include "hal_gpadc.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include "pmu.h"

// #define GPADC_TEST_MODE                 HAL_GPADC_ATP_60MS
#define GPADC_TEST_MODE                 HAL_GPADC_ATP_ONESHOT
#define ONESHOT_INTERVAL_TIME           1000

#define GPADC_TRACE(str, ...)           TRACE(TR_MOD(HAL), str, ##__VA_ARGS__)
#define GPADC_TRACE_VAL(str, ...)       TRACE_IMM(TR_MOD(HAL), str, ##__VA_ARGS__)

POSSIBLY_UNUSED static void gpadc_delay(uint32_t delay_ms)
{
    hal_sys_timer_delay(MS_TO_TICKS(delay_ms));
}

POSSIBLY_UNUSED static void gpadc_ch0_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    GPADC_TRACE_VAL("%s raw_val=%d volt=%d ", __func__, irq_val, volt);
#ifdef CHIP_BEST1600
    pmu_volt2temperature(volt);
#endif
}

POSSIBLY_UNUSED static void gpadc_ch1_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    GPADC_TRACE_VAL("%s raw_val=%d volt=%d", __func__, irq_val, volt);
}

POSSIBLY_UNUSED static void gpadc_ch2_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    GPADC_TRACE_VAL("%s raw_val=%d volt=%d", __func__, irq_val, volt);
}

POSSIBLY_UNUSED static void gpadc_ch3_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    GPADC_TRACE_VAL("%s raw_val=%d volt=%d", __func__, irq_val, volt);
}

POSSIBLY_UNUSED static void gpadc_ch4_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    GPADC_TRACE_VAL("%s raw_val=%d volt=%d", __func__, irq_val, volt);
}

POSSIBLY_UNUSED static void gpadc_ch5_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    GPADC_TRACE_VAL("%s raw_val=%d volt=%d", __func__, irq_val, volt);
}

POSSIBLY_UNUSED static void gpadc_ch6_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    GPADC_TRACE_VAL("%s raw_val=%d volt=%d", __func__, irq_val, volt);
}

void test_gpadc(void)
{
    GPADC_TRACE("------gpadc test started------");

#ifdef CHIP_BEST1600
    GPADC_TRACE("enable ch0 for temperature testing");
    pmu_ntc_capture_enable();
    GPADC_TRACE("enable adc vref output");
    pmu_adc_vref_enable();
#endif

    if (GPADC_TEST_MODE == HAL_GPADC_ATP_ONESHOT) {
        GPADC_TRACE("gpadc uses oneshot mode, oneshot interval time is %d ms", ONESHOT_INTERVAL_TIME);
    } else {
        GPADC_TRACE("gpadc uses continuous mode");
    }

    int ret = 0;
    do {
        ret = hal_gpadc_open(HAL_GPADC_CHAN_0, GPADC_TEST_MODE, gpadc_ch0_irq_handler);
        ASSERT(0 == ret, "gpadc ch0 init fail, ret=%d", ret);

        ret = hal_gpadc_open(HAL_GPADC_CHAN_BATTERY, GPADC_TEST_MODE, gpadc_ch1_irq_handler);
        ASSERT(0 == ret, "gpadc ch1 init fail, ret=%d", ret);

        ret = hal_gpadc_open(HAL_GPADC_CHAN_2, GPADC_TEST_MODE, gpadc_ch2_irq_handler);
        ASSERT(0 == ret, "gpadc ch2 init fail, ret=%d", ret);

        ret = hal_gpadc_open(HAL_GPADC_CHAN_3, GPADC_TEST_MODE, gpadc_ch3_irq_handler);
        ASSERT(0 == ret, "gpadc ch3 init fail, ret=%d", ret);

        ret = hal_gpadc_open(HAL_GPADC_CHAN_4, GPADC_TEST_MODE, gpadc_ch4_irq_handler);
        ASSERT(0 == ret, "gpadc ch4 init fail, ret=%d", ret);

        ret = hal_gpadc_open(HAL_GPADC_CHAN_5, GPADC_TEST_MODE, gpadc_ch5_irq_handler);
        ASSERT(0 == ret, "gpadc ch5 init fail, ret=%d", ret);

        ret = hal_gpadc_open(HAL_GPADC_CHAN_6, GPADC_TEST_MODE, gpadc_ch6_irq_handler);
        ASSERT(0 == ret, "gpadc ch6 init fail, ret=%d", ret);

        if (GPADC_TEST_MODE == HAL_GPADC_ATP_ONESHOT) {
            gpadc_delay(ONESHOT_INTERVAL_TIME);
            TRACE_OUTPUT_LINEFEED();
            TRACE_OUTPUT_LINEFEED();
        }
    } while (GPADC_TEST_MODE == HAL_GPADC_ATP_ONESHOT);
}


#endif


