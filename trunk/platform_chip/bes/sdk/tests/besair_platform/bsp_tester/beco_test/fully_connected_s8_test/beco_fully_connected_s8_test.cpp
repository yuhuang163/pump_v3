/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#include "fully_connected_s8_test01.h"
#include "fully_connected_s8_test02.h"
#include "fully_connected_s8_test03.h"

#include "hal_timer.h"
#include "hal_trace.h"
#include "dwt.h"

#define DIM_VEC 128
#define DIM_COL 512
#define PRINT_RESULTS 0

static void add_input_offset_2_bias(int32_t *bias,
                                    int8_t *weight,
                                    int input_offset,
                                    int dim_vec,
                                    int dim_col)
{
    for (int i = 0; i < dim_col; i++) {
        int sum = 0;
        for (int j = 0; j < dim_vec; j++) {
            sum += weight[j*dim_col + i] * input_offset;
        }
        bias[i] += sum;
    }
}

void compare_fully_connected_s8(void)
{
    int8_t pV[DIM_VEC];
    int8_t pM[DIM_VEC*DIM_COL];
    int32_t bias_beco[DIM_COL];
    int32_t bias_cmsis[DIM_COL];

    // Create test pattern
    for(int i=0; i< DIM_VEC; i++) { pV[i] = i % 100 - 50; }
    for(int i=0; i< DIM_VEC*DIM_COL; i++) { pM[i] = i % 100 - 50; }
    for(int i=0; i< DIM_COL; i++) { bias_beco[i] = 500 * (i % 50 - 25);
                                    bias_cmsis[i] = 500 * (i % 50 - 25); }

    // Init FC params
    cmsis_nn_context ctx;
    cmsis_nn_fc_params fc_params;
    cmsis_nn_per_tensor_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims;
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = 1;
    input_dims.h = 1;
    input_dims.c = DIM_VEC;
    filter_dims.n = DIM_VEC;
    filter_dims.c = DIM_COL;
    output_dims.n = 1;
    output_dims.c = DIM_COL;

    fc_params.input_offset = -10;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 30;
    fc_params.activation.min = NN_Q7_MIN;
    fc_params.activation.max = NN_Q7_MAX;

    quant_params.multiplier = NN_Q31_MAX;
    quant_params.shift = -8;

    if (fc_params.input_offset != 0) {
        add_input_offset_2_bias(bias_beco, pM, fc_params.input_offset, DIM_VEC, DIM_COL);
    }

    uint32_t s_time, e_time, n_cycle, lock;
    int32_t buf_size = beco_fully_connected_s8_get_buffer_size(&output_dims);
    ctx.buf = malloc(buf_size);
    /*------------------------------- beco --------------------------*/
    int8_t pOut_beco[DIM_COL];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_fully_connected_s8(
        &ctx, &fc_params, &quant_params, &input_dims, pV, &filter_dims, \
        pM, &bias_dims, bias_beco, &output_dims, pOut_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "beco_fully_connected_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(ctx.buf);

    /*------------------------------- cmsis --------------------------*/
    // weight transpose
    for (int i = 0; i<DIM_VEC*DIM_COL; ++i) {
        int next = getNext(i, DIM_VEC, DIM_COL);
        while (next > i)
            next = getNext(next, DIM_VEC, DIM_COL);
        if (next == i)
            movedata(pM, i, DIM_VEC, DIM_COL);
    }

    int8_t pOut_cmsis[DIM_COL];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_fully_connected_s8(&ctx, &fc_params, &quant_params, &input_dims, pV, &filter_dims, \
                           pM, &bias_dims, bias_cmsis, &output_dims, pOut_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "arm_fully_connected_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));

#if PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(pOut_beco, DIM_COL, 1, DIM_COL);

    TRACE(3, "cmsis results:");
    print_matrix(pOut_cmsis, DIM_COL, 1, DIM_COL);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < DIM_COL; i++) {
        if (abs(pOut_beco[i] - pOut_cmsis[i]) > 1) {
            match = false;
            break;
        }
    }
    if (match) {
        TRACE(3, "Beco result MATCH Reference");
    }
    else {
        TRACE(3, "Beco result doesn't match Reference");
    }
}

void compare_fully_connected_s8_test01(void)
{
    // Init FC params
    cmsis_nn_context ctx = {nullptr, 0};
    cmsis_nn_fc_params fc_params;
    cmsis_nn_per_tensor_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = 1;
    input_dims.h = 1;
    input_dims.c = FC_S8_TEST01_DIM_VEC;
    filter_dims.n = FC_S8_TEST01_DIM_VEC;
    filter_dims.c = FC_S8_TEST01_DIM_COL;
    output_dims.n = 1;
    output_dims.c = FC_S8_TEST01_DIM_COL;

    fc_params.input_offset = FC_S8_TEST01_INPUT_OFFSET;
    fc_params.filter_offset = 0;
    fc_params.output_offset = FC_S8_TEST01_OUTPUT_OFFSET;
    fc_params.activation.min = NN_Q7_MIN;
    fc_params.activation.max = NN_Q7_MAX;

    quant_params.multiplier = FC_S8_TEST01_MULTIPLIER;
    quant_params.shift = FC_S8_TEST01_SHIFT;

    uint32_t s_time, e_time, n_cycle, lock;
    /*------------------------------- beco -------------------------------*/
    int32_t buf_size = beco_fully_connected_s8_get_buffer_size(&output_dims);
    ctx.buf = malloc(buf_size);
    int8_t pOut_beco[FC_S8_TEST01_DIM_COL];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_fully_connected_s8(
        &ctx, &fc_params, &quant_params, &input_dims, fc_s8_test01_input, &filter_dims, \
        fc_s8_test01_beco_weight, &bias_dims, fc_s8_test01_beco_bias, &output_dims, pOut_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test01 beco_fully_connected_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(ctx.buf);

    /*------------------------------- cmsis -------------------------------*/
    int8_t pOut_cmsis[FC_S8_TEST01_DIM_COL];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params, &input_dims, fc_s8_test01_input, &filter_dims, \
        fc_s8_test01_cmsis_weight, &bias_dims, fc_s8_test01_cmsis_bias, &output_dims, pOut_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test01 arm_fully_connected_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));

#if PRINT_RESULTS
    TRACE(3, "beco test01 results:");
    print_matrix(pOut_beco, FC_S8_TEST01_DIM_COL, 1, FC_S8_TEST01_DIM_COL);

    TRACE(3, "cmsis test01 results:");
    print_matrix(pOut_cmsis, FC_S8_TEST01_DIM_COL, 1, FC_S8_TEST01_DIM_COL);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < FC_S8_TEST01_DIM_COL; i++) {
        if (abs(pOut_beco[i] - pOut_cmsis[i]) > 1) {
            match = false;
            break;
        }
    }
    if (match) {
        TRACE(3, "Beco result MATCH Reference");
    }
    else {
        TRACE(3, "Beco result doesn't match Reference");
    }
}

void compare_fully_connected_s8_test02(void)
{
    // Init FC params
    cmsis_nn_context ctx = {nullptr, 0};
    cmsis_nn_fc_params fc_params;
    cmsis_nn_per_tensor_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = 1;
    input_dims.h = 1;
    input_dims.c = FC_S8_TEST02_DIM_VEC;
    filter_dims.n = FC_S8_TEST02_DIM_VEC;
    filter_dims.c = FC_S8_TEST02_DIM_COL;
    output_dims.n = 1;
    output_dims.c = FC_S8_TEST02_DIM_COL;

    fc_params.input_offset = FC_S8_TEST02_INPUT_OFFSET;
    fc_params.filter_offset = 0;
    fc_params.output_offset = FC_S8_TEST02_OUTPUT_OFFSET;
    fc_params.activation.min = NN_Q7_MIN;
    fc_params.activation.max = NN_Q7_MAX;

    quant_params.multiplier = FC_S8_TEST02_MULTIPLIER;
    quant_params.shift = FC_S8_TEST02_SHIFT;

    uint32_t s_time, e_time, n_cycle, lock;
    /*------------------------------- beco -------------------------------*/
    int32_t buf_size = beco_fully_connected_s8_get_buffer_size(&output_dims);
    ctx.buf = malloc(buf_size);
    int8_t pOut_beco[FC_S8_TEST02_DIM_COL];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_fully_connected_s8(
        &ctx, &fc_params, &quant_params, &input_dims, fc_s8_test02_input, &filter_dims, \
        fc_s8_test02_beco_weight, &bias_dims, fc_s8_test02_beco_bias, &output_dims, pOut_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test02 beco_fully_connected_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(ctx.buf);

    /*------------------------------- cmsis -------------------------------*/
    int8_t pOut_cmsis[FC_S8_TEST02_DIM_COL];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params, &input_dims, fc_s8_test02_input, &filter_dims, \
        fc_s8_test02_cmsis_weight, &bias_dims, fc_s8_test02_cmsis_bias, &output_dims, pOut_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test02 arm_fully_connected_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));

#if PRINT_RESULTS
    TRACE(3, "beco test02 results:");
    print_matrix(pOut_beco, FC_S8_TEST02_DIM_COL, 1, FC_S8_TEST02_DIM_COL);

    TRACE(3, "cmsis test02 results:");
    print_matrix(pOut_cmsis, FC_S8_TEST02_DIM_COL, 1, FC_S8_TEST02_DIM_COL);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < FC_S8_TEST02_DIM_COL; i++) {
        if (abs(pOut_beco[i] - pOut_cmsis[i]) > 1) {
            match = false;
            break;
        }
    }
    if (match) {
        TRACE(3, "Beco result MATCH Reference");
    }
    else {
        TRACE(3, "Beco result doesn't match Reference");
    }
}

void compare_fully_connected_s8_test03(void)
{
    // Init FC params
    cmsis_nn_context ctx = {nullptr, 0};
    cmsis_nn_fc_params fc_params;
    cmsis_nn_per_tensor_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = 1;
    input_dims.h = 1;
    input_dims.c = FC_S8_TEST03_DIM_VEC;
    filter_dims.n = FC_S8_TEST03_DIM_VEC;
    filter_dims.c = FC_S8_TEST03_DIM_COL;
    output_dims.n = 1;
    output_dims.c = FC_S8_TEST03_DIM_COL;

    fc_params.input_offset = FC_S8_TEST03_INPUT_OFFSET;
    fc_params.filter_offset = 0;
    fc_params.output_offset = FC_S8_TEST03_OUTPUT_OFFSET;
    fc_params.activation.min = NN_Q7_MIN;
    fc_params.activation.max = NN_Q7_MAX;

    quant_params.multiplier = FC_S8_TEST03_MULTIPLIER;
    quant_params.shift = FC_S8_TEST03_SHIFT;

    uint32_t s_time, e_time, n_cycle, lock;
    /*------------------------------- beco -------------------------------*/
    int32_t buf_size = beco_fully_connected_s8_get_buffer_size(&output_dims);
    ctx.buf = malloc(buf_size);
    int8_t pOut_beco[FC_S8_TEST03_DIM_COL];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_fully_connected_s8(
        &ctx, &fc_params, &quant_params, &input_dims, fc_s8_test03_input, &filter_dims, \
        fc_s8_test03_beco_weight, &bias_dims, fc_s8_test03_beco_bias, &output_dims, pOut_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test03 beco_fully_connected_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(ctx.buf);

    /*------------------------------- cmsis -------------------------------*/
    int8_t pOut_cmsis[FC_S8_TEST03_DIM_COL];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params, &input_dims, fc_s8_test03_input, &filter_dims, \
        fc_s8_test03_cmsis_weight, &bias_dims, fc_s8_test03_cmsis_bias, &output_dims, pOut_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test03 arm_fully_connected_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));

#if PRINT_RESULTS
    TRACE(3, "beco test03 results:");
    print_matrix(pOut_beco, FC_S8_TEST03_DIM_COL, 1, FC_S8_TEST03_DIM_COL);

    TRACE(3, "cmsis test03 results:");
    print_matrix(pOut_cmsis, FC_S8_TEST03_DIM_COL, 1, FC_S8_TEST03_DIM_COL);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < FC_S8_TEST03_DIM_COL; i++) {
        if (abs(pOut_beco[i] - pOut_cmsis[i]) > 1) {
            match = false;
            break;
        }
    }
    if (match) {
        TRACE(3, "Beco result MATCH Reference");
    }
    else {
        TRACE(3, "Beco result doesn't match Reference");
    }
}


void beco_fully_connected_s8_test(void)
{
    TRACE(3, "===== beco_fully_connected_s8_test START =====");
    compare_fully_connected_s8();
    compare_fully_connected_s8_test01();
    compare_fully_connected_s8_test02();
    compare_fully_connected_s8_test03();
    TRACE(3, "===== beco_fully_connected_s8_test END =====");
}
