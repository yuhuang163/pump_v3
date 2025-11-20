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

#include "beco_test.h"

#include "naive_utils.hpp"
#include "beco_print_matrix.hpp"

#include "hal_timer.h"
#include "hal_trace.h"
#include "dwt.h"

#define CH  64
#define IH  8
#define IW  8
#define OH  4
#define OW  4
#define PX  0
#define PY  0
#define SX  2
#define SY  2
#define KX  2
#define KY  2
#define PRINT_RESULTS 0

void compare_max_pool_s8(void)
{
    int8_t src[CH*IH*IW];
    int8_t dst_beco[CH*OH*OW];
    int8_t dst_cmsis[CH*OH*OW];

    // Create test pattern
    for (int i = 0; i < CH*IH*IW; i++) { src[i] = i % 100 - 50; }

    // Init Pool params
    cmsis_nn_context ctx;
    cmsis_nn_pool_params pool_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = IW;
    input_dims.h = IH;
    input_dims.c = CH;
    filter_dims.w = KX;
    filter_dims.h = KY;
    output_dims.w = OW;
    output_dims.h = OH;
    output_dims.c = CH;

    pool_params.padding.w = PX;
    pool_params.padding.h = PY;
    pool_params.stride.w = SX;
    pool_params.stride.h = SY;

    pool_params.activation.min = NN_Q7_MIN;
    pool_params.activation.max = NN_Q7_MAX;

    uint32_t s_time, e_time, n_cycle, lock;
    /*-------------------------- beco_nn --------------------------*/
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_max_pool_s8(&ctx, &pool_params, &input_dims, src, &filter_dims, &output_dims, dst_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "beco_max_pool_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));

    /*-------------------------- cmsis --------------------------*/
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_max_pool_s8(&ctx, &pool_params, &input_dims, src, &filter_dims, &output_dims, dst_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "arm_max_pool_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));

#if PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(dst_beco, OW*CH, OH, OW*CH);

    TRACE(3, "cmsis results:");
    print_matrix(dst_cmsis, OW*CH, OH, OW*CH);
#endif

    // Compare reference to beco
    if (memcmp(dst_beco, dst_cmsis, sizeof(dst_beco)) != 0) {
        TRACE(3, "Beco result doesn't match Cmsis Reference");
    } else {
        TRACE(3, "Beco result MATCH Cmsis Reference");
    }
}


void beco_max_pool_s8_test(void)
{
    TRACE(3, "===== beco_max_pool_s8_test START =====");
    compare_max_pool_s8();
    TRACE(3, "===== beco_max_pool_s8_test END =====");
}
