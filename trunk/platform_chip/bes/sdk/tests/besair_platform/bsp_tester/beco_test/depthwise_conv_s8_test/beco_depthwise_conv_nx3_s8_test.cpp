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

#include "depthwise_conv_nx3_s8_test01.h"
#include "depthwise_conv_nx3_s8_test02.h"
#include "depthwise_conv_nx3_s8_test03.h"

#include "hal_timer.h"
#include "hal_trace.h"
#include "dwt.h"

#define IN_H 64
#define IN_W 32
#define CH 2
#define KER_H 3
#define KER_W 3
#define PAD_H 1
#define PAD_W 1
#define STRIDE_H 1
#define STRIDE_W 2
#define OUT_H 64
#define OUT_W 16
#define PRINT_RESULTS 0

static void add_input_offset_2_bias(int32_t *bias,
                                    int8_t *weight,
                                    int input_offset,
                                    int input_ch,
                                    int kernel_h,
                                    int kernel_w)
{
    for (int i = 0; i < input_ch; i++) {
        int sum = 0;
        for (int j = 0; j < kernel_h; j++) {
            for (int k = 0; k < kernel_w; k++) {
                sum += weight[i*kernel_h*kernel_w + j*kernel_w + k] * input_offset;
            }
        }
        bias[i] += sum;
    }
}

void compare_depthwise_conv_nx3_s8(void)
{
    // input
    int8_t in_beco[CH * IN_H * IN_W];
    int8_t in_cmsis[CH * IN_H * IN_W];

    int32_t bias_beco[CH];
    int32_t bias_cmsis[CH];
    int32_t multiplier[CH];
    int32_t shift[CH];

    // kernel
    int8_t ker_beco[CH * KER_H * KER_W];
    int8_t ker_cmsis[CH * KER_H * KER_W];

    // output
    int8_t out_beco[OUT_H * OUT_W * CH];
    int8_t out_cmsis[OUT_H * OUT_W * CH];
    int8_t out[OUT_H * OUT_W * CH];

    // Create test pattern
    for (int i = 0; i < CH*IN_H*IN_W; i++) { in_beco[i] = i % 100 - 50; }
    for (int i = 0; i < CH*KER_H*KER_W; i++) { ker_beco[i] = i % 100 - 50; }
    for (int i = 0; i < CH; i++) { multiplier[i] = NN_Q31_MAX;
                                   shift[i] = -8; }
    for (int i = 0; i < CH; i++) { bias_beco[i] = 500 * (i % 50 - 25);
                                   bias_cmsis[i] = 500 * (i % 50 - 25); }

    // Init DW Conv params
    cmsis_nn_context ctx;
    cmsis_nn_dw_conv_params dw_conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.h = IN_H;
    input_dims.w = IN_W;
    input_dims.c = CH;
    filter_dims.h = KER_H;
    filter_dims.w = KER_W;
    output_dims.h = OUT_H;
    output_dims.w = OUT_W;
    output_dims.c = CH;

    dw_conv_params.padding.w = PAD_W;
    dw_conv_params.padding.h = PAD_H;
    dw_conv_params.stride.w = STRIDE_W;
    dw_conv_params.stride.h = STRIDE_H;
    dw_conv_params.dilation.w = 1;
    dw_conv_params.dilation.h = 1;
    dw_conv_params.ch_mult = 1;

    dw_conv_params.input_offset = -20;
    dw_conv_params.output_offset = 20;
    dw_conv_params.activation.min = NN_Q7_MIN;
    dw_conv_params.activation.max = NN_Q7_MAX;
    quant_params.multiplier = multiplier;
    quant_params.shift = shift;

    ctx.buf = NULL;
    ctx.size = 0;

    if (dw_conv_params.input_offset != 0) {
        add_input_offset_2_bias(bias_beco, ker_beco, dw_conv_params.input_offset, CH, KER_H, KER_W);
    }

    uint32_t s_time, e_time, n_cycle, lock;
    /*-------------------------- beco_nn --------------------------*/
    int32_t buf_size = beco_depthwise_conv_nx3_s8_get_buffer_size();
    ctx.buf = malloc(buf_size);
    memset(out_beco, 0, sizeof(out_beco));
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_depthwise_conv_nx3_s8(&ctx, &dw_conv_params, &quant_params, &input_dims,
        in_beco, &filter_dims, ker_beco, &bias_dims, bias_beco, &output_dims, out_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "beco_depthwise_conv_nx3_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    memset(out_cmsis, 0, sizeof(out_cmsis));
    chw2hwc(in_beco, in_cmsis, IN_W, IN_H, CH);
    chw2hwc(ker_beco, ker_cmsis, KER_W, KER_H, CH);
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_depthwise_conv_3x3_s8(&ctx, &dw_conv_params, &quant_params, &input_dims,
        in_cmsis, &filter_dims, ker_cmsis, &bias_dims, bias_cmsis, &output_dims, out_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "arm_depthwise_conv_3x3_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));

    hwc2chw(out_cmsis, out, OUT_W, OUT_H, CH);

#if PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(out_beco, OUT_H*OUT_W, CH, OUT_H*OUT_W);

    TRACE(3, "cmsis results:");
    print_matrix(out, OUT_H*OUT_W, CH, OUT_H*OUT_W);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < CH*OUT_H*OUT_W; i++) {
        if (abs(out_beco[i] - out[i]) > 1) {
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

void compare_depthwise_conv_nx3_s8_test01(void)
{
    // Init DW Conv params
    cmsis_nn_context ctx;
    cmsis_nn_dw_conv_params dw_conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.h = DEPTH_S8_TEST01_INPUT_H;
    input_dims.w = DEPTH_S8_TEST01_INPUT_W;
    input_dims.c = DEPTH_S8_TEST01_INPUT_CH;
    filter_dims.h = DEPTH_S8_TEST01_KERNEL_Y;
    filter_dims.w = DEPTH_S8_TEST01_KERNEL_X;
    output_dims.h = DEPTH_S8_TEST01_OUTPUT_H;
    output_dims.w = DEPTH_S8_TEST01_OUTPUT_W;
    output_dims.c = DEPTH_S8_TEST01_OUTPUT_CH;

    dw_conv_params.padding.w = DEPTH_S8_TEST01_PADDING_X;
    dw_conv_params.padding.h = DEPTH_S8_TEST01_PADDING_Y;
    dw_conv_params.stride.w = DEPTH_S8_TEST01_STRIDE_X;
    dw_conv_params.stride.h = DEPTH_S8_TEST01_STRIDE_Y;
    dw_conv_params.dilation.w = 1;
    dw_conv_params.dilation.h = 1;
    dw_conv_params.ch_mult = 1;

    dw_conv_params.input_offset = DEPTH_S8_TEST01_INPUT_OFFSET;
    dw_conv_params.output_offset = DEPTH_S8_TEST01_OUTPUT_OFFSET;
    dw_conv_params.activation.min = NN_Q7_MIN;
    dw_conv_params.activation.max = NN_Q7_MAX;
    quant_params.multiplier = depth_s8_test01_multiplier;
    quant_params.shift = depth_s8_test01_shift;

    ctx.buf = NULL;
    ctx.size = 0;

    uint32_t s_time, e_time, n_cycle, lock;
    /*-------------------------- beco_nn --------------------------*/
    int32_t buf_size = beco_depthwise_conv_nx3_s8_get_buffer_size();
    ctx.buf = malloc(buf_size);
    int8_t out_beco[DEPTH_S8_TEST01_OUTPUT_H*DEPTH_S8_TEST01_OUTPUT_W*DEPTH_S8_TEST01_OUTPUT_CH];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_depthwise_conv_nx3_s8(&ctx, &dw_conv_params, &quant_params, &input_dims,
                               depth_s8_test01_beco_input, &filter_dims, depth_s8_test01_beco_weight,
                               &bias_dims, depth_s8_test01_beco_bias, &output_dims, out_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test01 beco_depthwise_conv_nx3_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    int8_t out_cmsis[DEPTH_S8_TEST01_OUTPUT_H*DEPTH_S8_TEST01_OUTPUT_W*DEPTH_S8_TEST01_OUTPUT_CH];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_depthwise_conv_3x3_s8(&ctx, &dw_conv_params, &quant_params, &input_dims,
                              depth_s8_test01_cmsis_input, &filter_dims, depth_s8_test01_cmsis_weight,
                              &bias_dims, depth_s8_test01_cmsis_bias, &output_dims, out_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test01 arm_depthwise_conv_3x3_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));

    CMSIS_Output_Transform(out_cmsis, DEPTH_S8_TEST01_OUTPUT_CH, DEPTH_S8_TEST01_OUTPUT_W, DEPTH_S8_TEST01_OUTPUT_H);

#if PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(out_beco, DEPTH_S8_TEST01_OUTPUT_H*DEPTH_S8_TEST01_OUTPUT_W,
                 DEPTH_S8_TEST01_OUTPUT_CH, DEPTH_S8_TEST01_OUTPUT_H*DEPTH_S8_TEST01_OUTPUT_W);

    TRACE(3, "cmsis results:");
    print_matrix(out_cmsis, DEPTH_S8_TEST01_OUTPUT_H*DEPTH_S8_TEST01_OUTPUT_W,
                 DEPTH_S8_TEST01_OUTPUT_CH, DEPTH_S8_TEST01_OUTPUT_H*DEPTH_S8_TEST01_OUTPUT_W);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < DEPTH_S8_TEST01_OUTPUT_CH*DEPTH_S8_TEST01_OUTPUT_H*DEPTH_S8_TEST01_OUTPUT_W; i++) {
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

void compare_depthwise_conv_nx3_s8_test02(void)
{
    // Init DW Conv params
    cmsis_nn_context ctx;
    cmsis_nn_dw_conv_params dw_conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.h = DEPTH_S8_TEST02_INPUT_H;
    input_dims.w = DEPTH_S8_TEST02_INPUT_W;
    input_dims.c = DEPTH_S8_TEST02_INPUT_CH;
    filter_dims.h = DEPTH_S8_TEST02_KERNEL_Y;
    filter_dims.w = DEPTH_S8_TEST02_KERNEL_X;
    output_dims.h = DEPTH_S8_TEST02_OUTPUT_H;
    output_dims.w = DEPTH_S8_TEST02_OUTPUT_W;
    output_dims.c = DEPTH_S8_TEST02_OUTPUT_CH;

    dw_conv_params.padding.w = DEPTH_S8_TEST02_PADDING_X;
    dw_conv_params.padding.h = DEPTH_S8_TEST02_PADDING_Y;
    dw_conv_params.stride.w = DEPTH_S8_TEST02_STRIDE_X;
    dw_conv_params.stride.h = DEPTH_S8_TEST02_STRIDE_Y;
    dw_conv_params.dilation.w = 1;
    dw_conv_params.dilation.h = 1;
    dw_conv_params.ch_mult = 1;

    dw_conv_params.input_offset = DEPTH_S8_TEST02_INPUT_OFFSET;
    dw_conv_params.output_offset = DEPTH_S8_TEST02_OUTPUT_OFFSET;
    dw_conv_params.activation.min = NN_Q7_MIN;
    dw_conv_params.activation.max = NN_Q7_MAX;
    quant_params.multiplier = depth_s8_test02_multiplier;
    quant_params.shift = depth_s8_test02_shift;

    ctx.buf = NULL;
    ctx.size = 0;

    uint32_t s_time, e_time, n_cycle, lock;
    /*-------------------------- beco_nn --------------------------*/
    int32_t buf_size = beco_depthwise_conv_nx3_s8_get_buffer_size();
    ctx.buf = malloc(buf_size);
    int8_t out_beco[DEPTH_S8_TEST02_OUTPUT_H*DEPTH_S8_TEST02_OUTPUT_W*DEPTH_S8_TEST02_OUTPUT_CH];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_depthwise_conv_nx3_s8(&ctx, &dw_conv_params, &quant_params, &input_dims,
                               depth_s8_test02_beco_input, &filter_dims, depth_s8_test02_beco_weight,
                               &bias_dims, depth_s8_test02_beco_bias, &output_dims, out_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test02 beco_depthwise_conv_nx3_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    int8_t out_cmsis[DEPTH_S8_TEST02_OUTPUT_H*DEPTH_S8_TEST02_OUTPUT_W*DEPTH_S8_TEST02_OUTPUT_CH];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_depthwise_conv_3x3_s8(&ctx, &dw_conv_params, &quant_params, &input_dims,
                              depth_s8_test02_cmsis_input, &filter_dims, depth_s8_test02_cmsis_weight,
                              &bias_dims, depth_s8_test02_cmsis_bias, &output_dims, out_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test02 arm_depthwise_conv_3x3_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));

    CMSIS_Output_Transform(out_cmsis, DEPTH_S8_TEST02_OUTPUT_CH, DEPTH_S8_TEST02_OUTPUT_W, DEPTH_S8_TEST02_OUTPUT_H);

#if PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(out_beco, DEPTH_S8_TEST02_OUTPUT_H*DEPTH_S8_TEST02_OUTPUT_W,
                 DEPTH_S8_TEST02_OUTPUT_CH, DEPTH_S8_TEST02_OUTPUT_H*DEPTH_S8_TEST02_OUTPUT_W);

    TRACE(3, "cmsis results:");
    print_matrix(out_cmsis, DEPTH_S8_TEST02_OUTPUT_H*DEPTH_S8_TEST02_OUTPUT_W,
                 DEPTH_S8_TEST02_OUTPUT_CH, DEPTH_S8_TEST02_OUTPUT_H*DEPTH_S8_TEST02_OUTPUT_W);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < DEPTH_S8_TEST02_OUTPUT_CH*DEPTH_S8_TEST02_OUTPUT_H*DEPTH_S8_TEST02_OUTPUT_W; i++) {
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

void compare_depthwise_conv_nx3_s8_test03(void)
{
    // Init DW Conv params
    cmsis_nn_context ctx;
    cmsis_nn_dw_conv_params dw_conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.h = DEPTH_S8_TEST03_INPUT_H;
    input_dims.w = DEPTH_S8_TEST03_INPUT_W;
    input_dims.c = DEPTH_S8_TEST03_INPUT_CH;
    filter_dims.h = DEPTH_S8_TEST03_KERNEL_Y;
    filter_dims.w = DEPTH_S8_TEST03_KERNEL_X;
    output_dims.h = DEPTH_S8_TEST03_OUTPUT_H;
    output_dims.w = DEPTH_S8_TEST03_OUTPUT_W;
    output_dims.c = DEPTH_S8_TEST03_OUTPUT_CH;

    dw_conv_params.padding.w = DEPTH_S8_TEST03_PADDING_X;
    dw_conv_params.padding.h = DEPTH_S8_TEST03_PADDING_Y;
    dw_conv_params.stride.w = DEPTH_S8_TEST03_STRIDE_X;
    dw_conv_params.stride.h = DEPTH_S8_TEST03_STRIDE_Y;
    dw_conv_params.dilation.w = 1;
    dw_conv_params.dilation.h = 1;
    dw_conv_params.ch_mult = 1;

    dw_conv_params.input_offset = DEPTH_S8_TEST03_INPUT_OFFSET;
    dw_conv_params.output_offset = DEPTH_S8_TEST03_OUTPUT_OFFSET;
    dw_conv_params.activation.min = NN_Q7_MIN;
    dw_conv_params.activation.max = NN_Q7_MAX;
    quant_params.multiplier = depth_s8_test03_multiplier;
    quant_params.shift = depth_s8_test03_shift;

    ctx.buf = NULL;
    ctx.size = 0;

    uint32_t s_time, e_time, n_cycle, lock;
    /*-------------------------- beco_nn --------------------------*/
    int32_t buf_size = beco_depthwise_conv_nx3_s8_get_buffer_size();
    ctx.buf = malloc(buf_size);
    int8_t out_beco[DEPTH_S8_TEST03_OUTPUT_H*DEPTH_S8_TEST03_OUTPUT_W*DEPTH_S8_TEST03_OUTPUT_CH];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_depthwise_conv_nx3_s8(&ctx, &dw_conv_params, &quant_params, &input_dims,
                               depth_s8_test03_beco_input, &filter_dims, depth_s8_test03_beco_weight,
                               &bias_dims, depth_s8_test03_beco_bias, &output_dims, out_beco);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test03 beco_depthwise_conv_nx3_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    int8_t out_cmsis[DEPTH_S8_TEST03_OUTPUT_H*DEPTH_S8_TEST03_OUTPUT_W*DEPTH_S8_TEST03_OUTPUT_CH];
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    arm_depthwise_conv_3x3_s8(&ctx, &dw_conv_params, &quant_params, &input_dims,
                              depth_s8_test03_cmsis_input, &filter_dims, depth_s8_test03_cmsis_weight,
                              &bias_dims, depth_s8_test03_cmsis_bias, &output_dims, out_cmsis);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "test03 arm_depthwise_conv_3x3_s8",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));

    CMSIS_Output_Transform(out_cmsis, DEPTH_S8_TEST03_OUTPUT_CH, DEPTH_S8_TEST03_OUTPUT_W, DEPTH_S8_TEST03_OUTPUT_H);

#if PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(out_beco, DEPTH_S8_TEST03_OUTPUT_H*DEPTH_S8_TEST03_OUTPUT_W,
                 DEPTH_S8_TEST03_OUTPUT_CH, DEPTH_S8_TEST03_OUTPUT_H*DEPTH_S8_TEST03_OUTPUT_W);

    TRACE(3, "cmsis results:");
    print_matrix(out_cmsis, DEPTH_S8_TEST03_OUTPUT_H*DEPTH_S8_TEST03_OUTPUT_W,
                 DEPTH_S8_TEST03_OUTPUT_CH, DEPTH_S8_TEST03_OUTPUT_H*DEPTH_S8_TEST03_OUTPUT_W);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < DEPTH_S8_TEST03_OUTPUT_CH*DEPTH_S8_TEST03_OUTPUT_H*DEPTH_S8_TEST03_OUTPUT_W; i++) {
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


void beco_depthwise_conv_nx3_s8_test(void)
{
    TRACE(3, "===== beco_depthwise_conv_nx3_s8_test START =====");
    compare_depthwise_conv_nx3_s8();
    compare_depthwise_conv_nx3_s8_test01();
    compare_depthwise_conv_nx3_s8_test02();
    compare_depthwise_conv_nx3_s8_test03();
    TRACE(3, "===== beco_depthwise_conv_nx3_s8_test END =====");
}
