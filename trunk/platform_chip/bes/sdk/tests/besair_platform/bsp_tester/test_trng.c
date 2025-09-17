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
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_trng.h"
#include "string.h"
#ifdef TEST_RANDOM
#include "random.h"
#include "stdlib.h"
#endif
#ifdef TEST_TRNG
//#define BINARY_ASCII_OUTPUT

#define TRNG_TEST_BUF_LEN                       24

static const struct HAL_TRNG_CFG_T trng_cfg = { 0x40, HAL_TRNG_RND_SRC_SHORTEST, };

static uint8_t test_data[TRNG_TEST_BUF_LEN];

#ifndef BINARY_ASCII_OUTPUT
static uint32_t irq_cnt;
static volatile bool irq_end;

static void trng_cb(const uint32_t *buf, uint32_t len, uint32_t error)
{
    if (!buf || !len || error) {
        ASSERT(false, "TRNG irq error: buf=%p len=%u error=0x%0X", buf, len, error);
    }

    TRACE(0, "TRNG IRQ %u:", irq_cnt);
    DUMP8("%02X", buf, len);
    if (++irq_cnt < 10) {
        int ret = hal_trng_run(trng_cb);
        ASSERT(ret == 0, "TRNG RUN ERROR, ret:%d", ret);
    } else {
        irq_end = true;
    }
}
#endif

void test_trng(void)
{
    int ret;

    hal_trng_open(&trng_cfg);

#ifdef BINARY_ASCII_OUTPUT
    while (1) {
        ret = hal_trng_get_data(test_data, TRNG_TEST_BUF_LEN);
        ASSERT(ret == 0, "TRNG GET ERROR, ret:%d", ret);

        for (int i = 0; i < TRNG_TEST_BUF_LEN; ++i) {
            for (int j = 0; j < 8; ++j) {
                //Attention: close other log, eg hal_sleep_start_stats();
                REL_TRACE_NOCRLF_NOTS(TR_ATTR_IMM, "%c", (test_data[i]&(1<<j))?'1':'0');
            }
        }
    }
#else
    for (int i = 0; i < 10; i++) {
        ret = hal_trng_get_data(test_data, TRNG_TEST_BUF_LEN);
        ASSERT(ret == 0, "TRNG ERROR, ret:%d", ret);
        TRACE(0, "TRNG DATA %u:", i);
        DUMP8("%02X", test_data, sizeof(test_data));
        memset(test_data, 0, sizeof(test_data));
    }

    ret = hal_trng_run(trng_cb);
    ASSERT(ret == 0, "TRNG RUN ERROR, ret:%d", ret);
    while (!irq_end) {
        osDelay(1);
    }
#endif

    hal_trng_close();

#ifdef TEST_RANDOM
    srand(3);

    for (int i = 0; i < 10; i++) {
        ret = random_get_data(test_data, TRNG_TEST_BUF_LEN);
        ASSERT(ret == 0, "random ERROR, ret:%d", ret);
        TRACE(0, "random data %u:", i);
        DUMP8("%02X", test_data, sizeof(test_data));
        memset(test_data, 0, sizeof(test_data));

        int r = rand();
        TRACE(0, "random int: %d / 0x%08X", r, r);
    }
    TRACE(0, "rand_max=%d", RAND_MAX);
#endif
}
#endif
