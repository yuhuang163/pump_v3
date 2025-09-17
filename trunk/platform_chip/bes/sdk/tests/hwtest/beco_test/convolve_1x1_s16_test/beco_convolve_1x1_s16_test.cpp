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

#include "convolve_1x1_s16_test01.h"
#include "convolve_1x1_s16_test02.h"
#include "convolve_1x1_s16_test03.h"

#include "hal_timer.h"
#include "hal_trace.h"
#include "dwt.h"

#define IH 3
#define IW 32
#define ICH 8
#define OH 3
#define OW 32
#define OCH 16
#define PRINT_RESULTS 0

void compare_convolve_1x1_s16(void)
{
    int16_t Im_in[ICH*IH*IW];
    int8_t wt[ICH*OCH];
    int32_t bias_beco[OCH];
    int64_t bias_cmsis[OCH];
    int32_t multiplier[OCH];
    int32_t shift[OCH];

    // Create test pattern
    for (int i = 0; i < ICH*IH*IW; i++) { Im_in[i] = i % 100 - 50; }
    for (int i = 0; i < ICH*OCH; i++) { wt[i] = i % 100 - 50; }
    for (int i = 0; i < OCH; i++) { multiplier[i] = NN_Q31_MAX;
                                    shift[i] = -8;
                                    bias_beco[i] = 500 * (i % 50 - 25);
                                    bias_cmsis[i] = 500 * (i % 50 - 25); }

    // Init Conv1x1 params
    cmsis_nn_context ctx;
    cmsis_nn_conv_params conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims;
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = IW;
    input_dims.h = IH;
    input_dims.c = ICH;
    filter_dims.w = 1;
    filter_dims.h = 1;
    filter_dims.c = ICH;
    output_dims.w = OW;
    output_dims.h = OH;
    output_dims.c = OCH;

    conv_params.padding.w = 0;
    conv_params.padding.h = 0;
    conv_params.stride.w = 1;
    conv_params.stride.h = 1;
    conv_params.dilation.w = 1;
    conv_params.dilation.h = 1;

    conv_params.input_offset = 0;
    conv_params.output_offset = 0;
    conv_params.activation.min = NN_Q15_MIN;
    conv_params.activation.max = NN_Q15_MAX;
    quant_params.multiplier = multiplier;
    quant_params.shift = shift;

    uint32_t s_time, e_time, n_cycle, lock;
    /*-------------------------- beco_nn --------------------------*/
    int32_t buf_size = beco_convolve_1x1_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    int16_t out_beco[OCH*IH*IW];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_convolve_1x1_s16(&ctx, &conv_params, &quant_params, &input_dims, Im_in,
                          &filter_dims, wt, &bias_dims, bias_beco, &output_dims, out_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "beco_convolve_1x1_s16",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    int16_t out_cmsis[OCH*IH*IW];
    CMSIS_Input_Transform(Im_in, wt, ICH, IW, IH, OCH, 1, 1);
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_convolve_s16(&ctx, &conv_params, &quant_params, &input_dims, Im_in, &filter_dims,
                     wt, &bias_dims, bias_cmsis, &output_dims, out_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "arm_convolve_s16",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    CMSIS_Output_Transform(out_cmsis, OCH, OW, OH);

#if PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(out_beco, OH*OW, OCH, OH*OW);

    TRACE(3, "cmsis results:");
    print_matrix(out_cmsis, OH*OW, OCH, OH*OW);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < OCH*OH*OW; i++) {
        if (abs(out_beco[i] - out_cmsis[i]) > 1) {
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

void compare_convolve_1x1_s16_test01(void)
{
    // Init Conv1x1 params
    cmsis_nn_context ctx;
    cmsis_nn_conv_params conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = CONV_1X1_S16_TEST01_INPUT_W;
    input_dims.h = CONV_1X1_S16_TEST01_INPUT_H;
    input_dims.c = CONV_1X1_S16_TEST01_INPUT_CH;
    filter_dims.w = 1;
    filter_dims.h = 1;
    filter_dims.c = CONV_1X1_S16_TEST01_INPUT_CH;
    output_dims.w = CONV_1X1_S16_TEST01_OUTPUT_W;
    output_dims.h = CONV_1X1_S16_TEST01_OUTPUT_H;
    output_dims.c = CONV_1X1_S16_TEST01_OUTPUT_CH;

    conv_params.padding.w = 0;
    conv_params.padding.h = 0;
    conv_params.stride.w = 1;
    conv_params.stride.h = 1;
    conv_params.dilation.w = 1;
    conv_params.dilation.h = 1;

    conv_params.input_offset = CONV_1X1_S16_TEST01_INPUT_OFFSET;
    conv_params.output_offset = CONV_1X1_S16_TEST01_OUTPUT_OFFSET;
    conv_params.activation.min = NN_Q15_MIN;
    conv_params.activation.max = NN_Q15_MAX;
    quant_params.multiplier = conv_1x1_s16_test01_multiplier;
    quant_params.shift = conv_1x1_s16_test01_shift;

    uint32_t s_time, e_time, n_cycle, lock;
    /*-------------------------- beco_nn --------------------------*/
    int32_t buf_size = beco_convolve_1x1_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    int16_t out_beco[CONV_1X1_S16_TEST01_OUTPUT_CH*CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_convolve_1x1_s16(&ctx, &conv_params, &quant_params, &input_dims,
                          conv_1x1_s16_test01_beco_input, &filter_dims, conv_1x1_s16_test01_beco_weight,
                          &bias_dims, conv_1x1_s16_test01_beco_bias, &output_dims, out_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test01 beco_convolve_1x1_s16",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    int16_t out_cmsis[CONV_1X1_S16_TEST01_OUTPUT_CH*CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W];
    buf_size = arm_convolve_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    ctx.size = 0;
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_convolve_s16(&ctx, &conv_params, &quant_params, &input_dims,
                     conv_1x1_s16_test01_cmsis_input, &filter_dims, conv_1x1_s16_test01_cmsis_weight,
                     &bias_dims, conv_1x1_s16_test01_cmsis_bias, &output_dims, out_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test01 arm_convolve_s16",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    CMSIS_Output_Transform(out_cmsis, CONV_1X1_S16_TEST01_OUTPUT_CH, CONV_1X1_S16_TEST01_OUTPUT_W, CONV_1X1_S16_TEST01_OUTPUT_H);

#if PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(out_beco, CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W,
                 CONV_1X1_S16_TEST01_OUTPUT_CH,
                 CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W);

    TRACE(3, "cmsis results:");
    print_matrix(out_cmsis, CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W,
                 CONV_1X1_S16_TEST01_OUTPUT_CH,
                 CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < CONV_1X1_S16_TEST01_OUTPUT_CH*CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W; i++) {
        if (abs(out_beco[i] - out_cmsis[i]) > 1) {
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

void compare_convolve_1x1_s16_test02(void)
{
    // Init Conv1x1 params
    cmsis_nn_context ctx;
    cmsis_nn_conv_params conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = CONV_1X1_S16_TEST02_INPUT_W;
    input_dims.h = CONV_1X1_S16_TEST02_INPUT_H;
    input_dims.c = CONV_1X1_S16_TEST02_INPUT_CH;
    filter_dims.w = 1;
    filter_dims.h = 1;
    filter_dims.c = CONV_1X1_S16_TEST02_INPUT_CH;
    output_dims.w = CONV_1X1_S16_TEST02_OUTPUT_W;
    output_dims.h = CONV_1X1_S16_TEST02_OUTPUT_H;
    output_dims.c = CONV_1X1_S16_TEST02_OUTPUT_CH;

    conv_params.padding.w = 0;
    conv_params.padding.h = 0;
    conv_params.stride.w = 1;
    conv_params.stride.h = 1;
    conv_params.dilation.w = 1;
    conv_params.dilation.h = 1;

    conv_params.input_offset = CONV_1X1_S16_TEST02_INPUT_OFFSET;
    conv_params.output_offset = CONV_1X1_S16_TEST02_OUTPUT_OFFSET;
    conv_params.activation.min = NN_Q15_MIN;
    conv_params.activation.max = NN_Q15_MAX;
    quant_params.multiplier = conv_1x1_s16_test02_multiplier;
    quant_params.shift = conv_1x1_s16_test02_shift;

    uint32_t s_time, e_time, n_cycle, lock;
    /*-------------------------- beco_nn --------------------------*/
    int32_t buf_size = beco_convolve_1x1_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    int16_t out_beco[CONV_1X1_S16_TEST02_OUTPUT_CH*CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_convolve_1x1_s16(&ctx, &conv_params, &quant_params, &input_dims,
                          conv_1x1_s16_test02_beco_input, &filter_dims, conv_1x1_s16_test02_beco_weight,
                          &bias_dims, conv_1x1_s16_test02_beco_bias, &output_dims, out_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test02 beco_convolve_1x1_s16",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    int16_t out_cmsis[CONV_1X1_S16_TEST02_OUTPUT_CH*CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W];
    buf_size = arm_convolve_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    ctx.size = 0;
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_convolve_s16(&ctx, &conv_params, &quant_params, &input_dims,
                     conv_1x1_s16_test02_cmsis_input, &filter_dims, conv_1x1_s16_test02_cmsis_weight,
                     &bias_dims, conv_1x1_s16_test02_cmsis_bias, &output_dims, out_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test02 arm_convolve_s16",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    CMSIS_Output_Transform(out_cmsis, CONV_1X1_S16_TEST02_OUTPUT_CH, CONV_1X1_S16_TEST02_OUTPUT_W, CONV_1X1_S16_TEST02_OUTPUT_H);

#if PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(out_beco, CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W,
                 CONV_1X1_S16_TEST02_OUTPUT_CH,
                 CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W);

    TRACE(3, "cmsis results:");
    print_matrix(out_cmsis, CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W,
                 CONV_1X1_S16_TEST02_OUTPUT_CH,
                 CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < CONV_1X1_S16_TEST02_OUTPUT_CH*CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W; i++) {
        if (abs(out_beco[i] - out_cmsis[i]) > 1) {
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

void compare_convolve_1x1_s16_test03(void)
{
    // Init Conv1x1 params
    cmsis_nn_context ctx;
    cmsis_nn_conv_params conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = CONV_1X1_S16_TEST03_INPUT_W;
    input_dims.h = CONV_1X1_S16_TEST03_INPUT_H;
    input_dims.c = CONV_1X1_S16_TEST03_INPUT_CH;
    filter_dims.w = 1;
    filter_dims.h = 1;
    filter_dims.c = CONV_1X1_S16_TEST03_INPUT_CH;
    output_dims.w = CONV_1X1_S16_TEST03_OUTPUT_W;
    output_dims.h = CONV_1X1_S16_TEST03_OUTPUT_H;
    output_dims.c = CONV_1X1_S16_TEST03_OUTPUT_CH;

    conv_params.padding.w = 0;
    conv_params.padding.h = 0;
    conv_params.stride.w = 1;
    conv_params.stride.h = 1;
    conv_params.dilation.w = 1;
    conv_params.dilation.h = 1;

    conv_params.input_offset = CONV_1X1_S16_TEST03_INPUT_OFFSET;
    conv_params.output_offset = CONV_1X1_S16_TEST03_OUTPUT_OFFSET;
    conv_params.activation.min = NN_Q15_MIN;
    conv_params.activation.max = NN_Q15_MAX;
    quant_params.multiplier = conv_1x1_s16_test03_multiplier;
    quant_params.shift = conv_1x1_s16_test03_shift;

    uint32_t s_time, e_time, n_cycle, lock;
    /*-------------------------- beco_nn --------------------------*/
    int32_t buf_size = beco_convolve_1x1_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    int16_t out_beco[CONV_1X1_S16_TEST03_OUTPUT_CH*CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_convolve_1x1_s16(&ctx, &conv_params, &quant_params, &input_dims,
                          conv_1x1_s16_test03_beco_input, &filter_dims, conv_1x1_s16_test03_beco_weight,
                          &bias_dims, conv_1x1_s16_test03_beco_bias, &output_dims, out_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test03 beco_convolve_1x1_s16",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    int16_t out_cmsis[CONV_1X1_S16_TEST03_OUTPUT_CH*CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W];
    buf_size = arm_convolve_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    ctx.size = 0;
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_convolve_s16(&ctx, &conv_params, &quant_params, &input_dims,
                     conv_1x1_s16_test03_cmsis_input, &filter_dims, conv_1x1_s16_test03_cmsis_weight,
                     &bias_dims, conv_1x1_s16_test03_cmsis_bias, &output_dims, out_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test03 arm_convolve_s16",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    CMSIS_Output_Transform(out_cmsis, CONV_1X1_S16_TEST03_OUTPUT_CH, CONV_1X1_S16_TEST03_OUTPUT_W, CONV_1X1_S16_TEST03_OUTPUT_H);

#if PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(out_beco, CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W,
                 CONV_1X1_S16_TEST03_OUTPUT_CH,
                 CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W);

    TRACE(3, "cmsis results:");
    print_matrix(out_cmsis, CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W,
                 CONV_1X1_S16_TEST03_OUTPUT_CH,
                 CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < CONV_1X1_S16_TEST03_OUTPUT_CH*CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W; i++) {
        if (abs(out_beco[i] - out_cmsis[i]) > 1) {
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


void beco_convolve_1x1_s16_test(void)
{
    TRACE(3, "===== beco_convolve_1x1_s16_test START =====");
    compare_convolve_1x1_s16();
    compare_convolve_1x1_s16_test01();
    compare_convolve_1x1_s16_test02();
    compare_convolve_1x1_s16_test03();
    TRACE(3, "===== beco_convolve_1x1_s16_test END =====");
}
