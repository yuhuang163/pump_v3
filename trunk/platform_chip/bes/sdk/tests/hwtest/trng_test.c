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
#include "plat_addr_map.h"

#ifdef TRNG_BASE

#include "cmsis.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_trng.h"
#include "hwtest.h"
#include "string.h"
#ifdef RANDOM_TEST
#include "random.h"
#include "stdlib.h"
#endif
#ifdef RTOS
#include "cmsis_os.h"
#endif

//Available for NIST SP 800-22
//#define BINARY_ASCII_OUTPUT

//#define TRNG_MASS_DATA_TEST_MODE
#define ASCII_RM_HEAD_ENABLE
#define ASCII_RM_HEAD_TIME          15  //second
#define LOOP_TEST_ENABLE            0
#ifdef TRNG_MASS_DATA_TEST_MODE
#define CALC_RATE_TIME_BYTE         HAL_TRNG_DATA_LEN
#define TRNG_TEST_BUF_LEN           (32 * 1024)
#else
#define TRNG_TEST_BUF_LEN           HAL_TRNG_DATA_LEN
#endif
static uint8_t test_data[TRNG_TEST_BUF_LEN];
static const struct HAL_TRNG_CFG_T trng_cfg = { 0x40, HAL_TRNG_RND_SRC_SHORTEST, };

#if defined(BINARY_ASCII_OUTPUT) && defined(TRNG_MASS_DATA_TEST_MODE)
    #error "Please turn off the macro TRNG_MASS_DATA_TEST_MODE"
#endif

#ifndef BINARY_ASCII_OUTPUT
static uint32_t irq_cnt = 0;
static volatile bool irq_end = false;

static void trng_cb(const uint32_t *buf, uint32_t len, uint32_t error)
{
    if (!buf || !len || error) {
        ASSERT(false, "TRNG irq error: buf=%p len=%u error=0x%0X", buf, len, error);
    }

    TRACE(TR_ATTR_NO_LF, "TRNG IRQ  %u:", irq_cnt);
    DUMP8("%02X", buf, len);
    if (++irq_cnt < 10) {
        int ret = hal_trng_run(trng_cb);
        ASSERT(ret == 0, "TRNG RUN ERROR, ret:%d", ret);
    } else {
        irq_end = true;
        irq_cnt = 0;
    }
}
#endif

void trng_test(void)
{
    int ret;
    uint32_t time_start, time;

    TRACE(0, "  ");
    do {
        TRACE(0, "%s start", __func__);
        TRACE(0, "  ");
        time_start = hal_fast_sys_timer_get();
        hal_trng_open(&trng_cfg);
        time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
        TRACE(0, "===hal_trng_open() execution time:%uus", time);
        TRACE(0, "  ");

#ifdef BINARY_ASCII_OUTPUT
#ifdef ASCII_RM_HEAD_ENABLE
        uint32_t timeout = MS_TO_TICKS(ASCII_RM_HEAD_TIME * 1000), busy_t;
        busy_t = hal_sys_timer_get();
#endif
        while (1) {
            ret = hal_trng_get_data(test_data, TRNG_TEST_BUF_LEN);
            ASSERT(ret == 0, "TRNG GET ERROR, ret:%d", ret);

#ifdef ASCII_RM_HEAD_ENABLE
            if (hal_sys_timer_get() - busy_t < timeout) {
                continue;
            }
#endif
            for (int i = 0; i < TRNG_TEST_BUF_LEN; ++i) {
                for (int j = 0; j < 8; ++j) {
                    //Attention: close other log, eg hal_sleep_start_stats();
                    REL_TRACE_NOCRLF_NOTS(TR_ATTR_IMM, "%c", (test_data[i] & (1 << j)) ? '1' : '0');
                }
            }
        }
#else
        for (int i = 0; i < 10; i++) {
#ifdef TRNG_MASS_DATA_TEST_MODE
            time_start = hal_fast_sys_timer_get();
#endif
            ret = hal_trng_get_data(test_data, TRNG_TEST_BUF_LEN);
            ASSERT(ret == 0, "TRNG ERROR, ret:%d", ret);
#ifdef TRNG_MASS_DATA_TEST_MODE
            time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
            TRACE(0, "===hal_trng_get_data() execution time:%uus/%ubytes=%uus/%ubytes", time, TRNG_TEST_BUF_LEN,
                  (time * CALC_RATE_TIME_BYTE) / TRNG_TEST_BUF_LEN, CALC_RATE_TIME_BYTE);
#else
            TRACE(TR_ATTR_NO_LF, "TRNG DATA %u:", i);
            DUMP8("%02X", test_data, sizeof(test_data));
#endif
            memset(test_data, 0, sizeof(test_data));
        }

        if (TRNG_IRQn != INVALID_IRQn) {
            TRACE(0, "  ");
            ret = hal_trng_run(trng_cb);
            ASSERT(ret == 0, "TRNG RUN ERROR, ret:%d", ret);
            while (!irq_end) {
                osDelay(1);
            }
            irq_end = false;
        }
#endif

        time_start = hal_fast_sys_timer_get();
        hal_trng_close();
        time = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
        TRACE(0, "  ");
        TRACE(0, "===hal_trng_close() execution time:%uus", time);
        TRACE(0, "  ");
    } while (LOOP_TEST_ENABLE);

#ifdef RANDOM_TEST
    srand(hal_sys_timer_get() * hal_fast_sys_timer_get());

    TRACE(0, "  ");
    for (int i = 0; i < 10; i++) {
        ret = random_get_data(test_data, TRNG_TEST_BUF_LEN);
        ASSERT(ret == 0, "random ERROR, ret:%d", ret);
        TRACE(TR_ATTR_NO_LF, "random data %u:", i);
        DUMP8("%02X", test_data, sizeof(test_data));
        memset(test_data, 0, sizeof(test_data));

        int r = rand();
        TRACE(0, "random int   :%d / 0x%08X", r, r);
    }
    TRACE(0, "rand_max=%d=0x%X", RAND_MAX, RAND_MAX);
#endif

    TRACE(0, "  ");
    TRACE(0, "%s end", __func__);
}

#endif
