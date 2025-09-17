/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "hal_cmu.h"
#include "hal_timer.h"
#include "hal_gpadc.h"
#include "hal_trace.h"
#include "pmu.h"
#ifdef UTILS_ESHELL_EN
#include "eshell.h"
#endif

// #define GPADC_TEST_MODE                 HAL_GPADC_ATP_60MS
#define GPADC_TEST_MODE                 HAL_GPADC_ATP_ONESHOT
#define ONESHOT_INTERVAL_TIME           1000

#define GPADC_TRACE(str, ...)           TRACE(TR_MOD(HAL), str, ##__VA_ARGS__)
#define GPADC_TRACE_VAL(str, ...)       TRACE_IMM(TR_MOD(HAL), str, ##__VA_ARGS__)

//#define GPADC_LOOP_TEST

#ifdef GPADC_LOOP_TEST
static void gpadc_loop_test(void);
#endif

POSSIBLY_UNUSED static void gpadc_delay(uint32_t delay_ms)
{
    hal_sys_timer_delay(MS_TO_TICKS(delay_ms));
}

POSSIBLY_UNUSED static void gpadc_ch0_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
#ifdef CHIP_BEST1600
    pmu_ntc_capture_disable();
    pmu_volt2temperature(volt);
#endif
    GPADC_TRACE_VAL("%s raw_val=%d volt=%d ", __func__, irq_val, volt);
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

void gpadc_test(void)
{
#ifdef GPADC_LOOP_TEST
    return gpadc_loop_test();
#endif

    GPADC_TRACE("------gpadc test started------");

    if (GPADC_TEST_MODE == HAL_GPADC_ATP_ONESHOT) {
        GPADC_TRACE("gpadc uses oneshot mode, oneshot interval time is %d ms", ONESHOT_INTERVAL_TIME);
    } else {
        GPADC_TRACE("gpadc uses continuous mode");
    }

    int ret = 0;
    do {
        GPADC_TRACE("------gpadc irq read mode test start------\n");
        for (int i = 0; i < 3; i++) {
#ifdef CHIP_BEST1600
            pmu_ntc_capture_enable();
#endif
            ret = hal_gpadc_open(HAL_GPADC_CHAN_0, GPADC_TEST_MODE, gpadc_ch0_irq_handler);
            ASSERT(0 == ret, "gpadc ch0 init fail, ret=%d", ret);
            gpadc_delay(10);
        }
        TRACE(0, "\n");

        for (int i = 0; i < 3; i++) {
            ret = hal_gpadc_open(HAL_GPADC_CHAN_BATTERY, GPADC_TEST_MODE, gpadc_ch1_irq_handler);
            ASSERT(0 == ret, "gpadc ch1 init fail, ret=%d", ret);
            gpadc_delay(10);
        }
        TRACE(0, "\n");

        for (int i = 0; i < 3; i++) {
            ret = hal_gpadc_open(HAL_GPADC_CHAN_2, GPADC_TEST_MODE, gpadc_ch2_irq_handler);
            ASSERT(0 == ret, "gpadc ch2 init fail, ret=%d", ret);
            gpadc_delay(10);
        }
        TRACE(0, "\n");

        for (int i = 0; i < 3; i++) {
            ret = hal_gpadc_open(HAL_GPADC_CHAN_3, GPADC_TEST_MODE, gpadc_ch3_irq_handler);
            ASSERT(0 == ret, "gpadc ch3 init fail, ret=%d", ret);
            gpadc_delay(10);
        }
        TRACE(0, "\n");

        for (int i = 0; i < 3; i++) {
            ret = hal_gpadc_open(HAL_GPADC_CHAN_4, GPADC_TEST_MODE, gpadc_ch4_irq_handler);
            ASSERT(0 == ret, "gpadc ch4 init fail, ret=%d", ret);
            gpadc_delay(10);
        }
        TRACE(0, "\n");

        for (int i = 0; i < 3; i++) {
            ret = hal_gpadc_open(HAL_GPADC_CHAN_5, GPADC_TEST_MODE, gpadc_ch5_irq_handler);
            ASSERT(0 == ret, "gpadc ch5 init fail, ret=%d", ret);
            gpadc_delay(10);
        }
        TRACE(0, "\n");

        for (int i = 0; i < 3; i++) {
            ret = hal_gpadc_open(HAL_GPADC_CHAN_6, GPADC_TEST_MODE, gpadc_ch6_irq_handler);
            ASSERT(0 == ret, "gpadc ch6 init fail, ret=%d", ret);
            gpadc_delay(10);
        }
        TRACE(0, "\n");
        GPADC_TRACE("------gpadc irq read mode test stop------\n");

        GPADC_TRACE("------gpadc block read mode test start------\n");
        uint16_t volt;
        for (enum HAL_GPADC_CHAN_T ch = HAL_GPADC_CHAN_0; ch < HAL_GPADC_CHAN_ADCKEY; ch++) {
            for (int i = 0; i < 3; i++) {
#ifdef CHIP_BEST1600
                if (ch == HAL_GPADC_CHAN_0) {
                    pmu_ntc_capture_enable();
                }
#endif
                ret = hal_gpadc_open(ch, GPADC_TEST_MODE, NULL);
                ASSERT(0 == ret, "gpadc block read, ch:%d init fail", ch);

                ret = hal_gpadc_get_volt_blocked(ch, &volt);
                ASSERT(ret, "gpadc block read, ch:%d get volt fail", ch);
                TRACE(0, "gpadc block read, ch:%d volt:%d", ch, volt);

                hal_gpadc_close(ch);
#ifdef CHIP_BEST1600
                if (ch == HAL_GPADC_CHAN_0) {
                    pmu_ntc_capture_disable();
                }
#endif
            }
            TRACE(0, "\n");
        }
        GPADC_TRACE("------gpadc block read mode test stop------\n");

#ifdef UTILS_ESHELL_EN
            break;
#else
        if (GPADC_TEST_MODE == HAL_GPADC_ATP_ONESHOT) {
            gpadc_delay(ONESHOT_INTERVAL_TIME);
            TRACE_OUTPUT_LINEFEED();
            TRACE_OUTPUT_LINEFEED();
        }
#endif
    } while (GPADC_TEST_MODE == HAL_GPADC_ATP_ONESHOT);
}

#ifdef UTILS_ESHELL_EN
static void watch_gpadc_test_event(int argc, char *argv[])
{
    eshell_putstring_nl("Auto Test command:gpadc_test\n");
    gpadc_test();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_AUTO_TEST, "gpadc_test", "gpadc_test", watch_gpadc_test_event);
#endif

#ifdef GPADC_LOOP_TEST
#define GPADC_LOOP_RAW_BUF_CNT                          (128 * 1000)
#define GPADC_LOOP_CHANNEL                              HAL_GPADC_CHAN_2

static uint16_t g_gpadc_raw_buf[GPADC_LOOP_RAW_BUF_CNT] = {0, };
static uint32_t g_gpadc_loop_cnt = 0;
static bool g_gpadc_dump_start = false;

static void gpadc_loop_test_irq_handler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    //GPADC_TRACE_VAL("%s raw_val=%d volt=%d", __func__, irq_val, volt);
    g_gpadc_raw_buf[g_gpadc_loop_cnt] = irq_val;
    g_gpadc_loop_cnt++;
    if (g_gpadc_loop_cnt == GPADC_LOOP_RAW_BUF_CNT) {
        hal_gpadc_close(GPADC_LOOP_CHANNEL);
        g_gpadc_dump_start = true;
    }
}

static void gpadc_loop_test_raw_print(void)
{
    TRACE(0, "%s ============== start!", __func__);
    for (uint32_t i = 0; i < sizeof(g_gpadc_raw_buf) / sizeof(g_gpadc_raw_buf[0]); i++) {
        TRACE_IMM(0, "%d", g_gpadc_raw_buf[i]);
        hal_sys_timer_delay(MS_TO_TICKS(1));
    }
    TRACE(0, "%s ============== stop!", __func__);
}

static void gpadc_loop_test(void)
{
    hal_gpadc_open(GPADC_LOOP_CHANNEL, HAL_GPADC_ATP_125US, gpadc_loop_test_irq_handler);
    while(1) {
        if (g_gpadc_dump_start) {
            gpadc_loop_test_raw_print();
            TRACE_IMM(0,"store buf cnt=%d", GPADC_LOOP_RAW_BUF_CNT);
            hal_sys_timer_delay(MS_TO_TICKS(5000));
            pmu_shutdown();
        }
    }
}
#endif
