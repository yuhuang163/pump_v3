/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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
#include "cmsis.h"
#include "dhry_test.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"

#define TEST_FREQ                       MACRO_CONCAT(HAL_CMU_FREQ_, MACRO_CONCAT(TEST_FREQ_MHZ, M))

#ifdef SIMU
static unsigned freq_factor;
#endif

void dhrystone_test(void)
{
    const unsigned int max_loop = TEST_LOOPS;
    unsigned int count = 0;
    enum HAL_CMU_FREQ_T freq;

    freq = TEST_FREQ;
#ifdef SIMU
#ifndef FORCE_TEST_FREQ
    uint32_t mhz;

    hal_cmu_simu_init();
    mhz = hal_cmu_simu_get_val();
    if (mhz == 208 || mhz == 192) {
        freq = HAL_CMU_FREQ_208M;
    } else if (mhz == 104 || mhz == 96) {
        freq = HAL_CMU_FREQ_104M;
    } else if (mhz == 52 || mhz == 48) {
        freq = HAL_CMU_FREQ_52M;
    } else {
        freq = HAL_CMU_FREQ_26M;
    }
#endif

    if (freq == HAL_CMU_FREQ_208M) {
        freq_factor = 8;
    } else if (freq == HAL_CMU_FREQ_104M) {
        freq_factor = 4;
    } else if (freq == HAL_CMU_FREQ_52M) {
        freq_factor = 2;
    } else {
        freq_factor = 1;
    }
#endif
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, freq);

    TRACE_OUTPUT_LINEFEED();
    TRACE_IMM(0, "CPU frequency changed to: %u MHz", (hal_sys_timer_calc_cpu_freq(5, 0) / 1000000));
    TRACE_IMM(0, "------dhrystone test started------");
    TRACE_IMM(0, "Min test time: %u us", TEST_MIN_TIME_US);
    TRACE_OUTPUT_LINEFEED();

    for (count = 0; count < max_loop; count++) {
        TRACE_IMM(0, "LOOP: %u / %u", count + 1, max_loop);
        dhrystone_main();
    }
}

unsigned dhrystone_get_freq_factor(void)
{
#ifdef SIMU
    return freq_factor;
#else
    return 1;
#endif
}

void dhrystone_event_begin(void)
{
#ifdef SIMU
#ifdef __arm__
    __SEV();
#else
    hal_cmu_simu_set_val((get_cpu_id() << 16) | 1);
#endif
#endif
}

void dhrystone_event_end(void)
{
#ifdef SIMU
#ifdef __arm__
    __SEV();
#else
    hal_cmu_simu_set_val((get_cpu_id() << 16) | 0);
#endif
#endif
}
