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

#include "beco_nnfunctions.h"
#include "beco_nnsupportfunctions.h"
#include "beco_bias.hpp"

#if !defined(ARM_MATH_MVEI)
static void beco_depthwise_conv_q7_readout(int8_t *out,
                                           const int32_t multiplier,
                                           const int32_t shift,
                                           const int8_t offset,
                                           const int8_t min,
                                           const int8_t max)
{
    out[0] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 0).i32, multiplier, shift, offset, min, max);
    out[1] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 4).i32, multiplier, shift, offset, min, max);
    out[2] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 2).i32, multiplier, shift, offset, min, max);
    out[3] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 6).i32, multiplier, shift, offset, min, max);
    out[4] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 8).i32, multiplier, shift, offset, min, max);
    out[5] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 12).i32, multiplier, shift, offset, min, max);
    out[6] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 10).i32, multiplier, shift, offset, min, max);
    out[7] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 14).i32, multiplier, shift, offset, min, max);
}

static void beco_depthwise_conv_q7_4accs_readout(int8_t *out,
                                                 const int32_t multiplier,
                                                 const int32_t shift,
                                                 const int8_t offset,
                                                 const int8_t min,
                                                 const int8_t max)
{
    out[0] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 0).i32, multiplier, shift, offset, min, max);
    out[1] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 4).i32, multiplier, shift, offset, min, max);
    out[2] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 2).i32, multiplier, shift, offset, min, max);
    out[3] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 6).i32, multiplier, shift, offset, min, max);
    out[4] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 8).i32, multiplier, shift, offset, min, max);
    out[5] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 12).i32, multiplier, shift, offset, min, max);
    out[6] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 10).i32, multiplier, shift, offset, min, max);
    out[7] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 14).i32, multiplier, shift, offset, min, max);

    out[8] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 0).i32, multiplier, shift, offset, min, max);
    out[9] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 4).i32, multiplier, shift, offset, min, max);
    out[10] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 2).i32, multiplier, shift, offset, min, max);
    out[11] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 6).i32, multiplier, shift, offset, min, max);
    out[12] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 8).i32, multiplier, shift, offset, min, max);
    out[13] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 12).i32, multiplier, shift, offset, min, max);
    out[14] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 10).i32, multiplier, shift, offset, min, max);
    out[15] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 14).i32, multiplier, shift, offset, min, max);

    out[16] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 0).i32, multiplier, shift, offset, min, max);
    out[17] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 4).i32, multiplier, shift, offset, min, max);
    out[18] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 2).i32, multiplier, shift, offset, min, max);
    out[19] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 6).i32, multiplier, shift, offset, min, max);
    out[20] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 8).i32, multiplier, shift, offset, min, max);
    out[21] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 12).i32, multiplier, shift, offset, min, max);
    out[22] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 10).i32, multiplier, shift, offset, min, max);
    out[23] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 14).i32, multiplier, shift, offset, min, max);

    out[24] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 0).i32, multiplier, shift, offset, min, max);
    out[25] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 4).i32, multiplier, shift, offset, min, max);
    out[26] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 2).i32, multiplier, shift, offset, min, max);
    out[27] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 6).i32, multiplier, shift, offset, min, max);
    out[28] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 8).i32, multiplier, shift, offset, min, max);
    out[29] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 12).i32, multiplier, shift, offset, min, max);
    out[30] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 10).i32, multiplier, shift, offset, min, max);
    out[31] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 14).i32, multiplier, shift, offset, min, max);
}
#endif

static void beco_depthwise_conv_q7_impl(beco_vec64_in_t *beco_in,
                                        int8_t *kernel,
                                        const uint16_t kernel_x)
{
    uint16_t kernelCnt = kernel_x;
    while (kernelCnt) {
        beco_write_reg(BECO_REG0, *beco_in++);
        beco_mmacgr(BECO_ACC0, (beco_vec32_in_t){.i8 = {*kernel++, 0, 0, 0}}, BECO_REG0);
        kernelCnt--;
    }
}

static void beco_depthwise_conv_q7_4accs_impl(beco_vec64_in_t *beco_in,
                                              int8_t *kernel,
                                              const uint16_t kernel_x)
{
    uint16_t kernelCnt = kernel_x;
    while (kernelCnt) {
        beco_write_reg(BECO_REG0, *beco_in++);
        beco_write_reg(BECO_REG1, *beco_in++);
        beco_write_reg(BECO_REG2, *beco_in++);
        beco_write_reg(BECO_REG3, *beco_in++);
        beco_mmacgr4((beco_vec32_in_t){.i8 = {*kernel++, 0, 0, 0}}, BECO_REG0);
        kernelCnt--;
    }
}

static void beco_depthwise_conv_q7_offset_impl(beco_vec64_in_t beco_offset,
                                               int8_t *kernel,
                                               const uint16_t kernel_x)
{
    uint16_t kernelCnt = kernel_x;
    beco_write_reg(BECO_REG0, beco_offset);
    while (kernelCnt) {
        beco_mmacgr(BECO_ACC0, (beco_vec32_in_t){.i8 = {*kernel++, 0, 0, 0}}, BECO_REG0);
        kernelCnt--;
    }
}

static void beco_depthwise_conv_q7_offset_4accs_impl(beco_vec64_in_t beco_offset,
                                                     int8_t *kernel,
                                                     const uint16_t kernel_x)
{
    uint16_t kernelCnt = kernel_x;
    beco_write_reg(BECO_REG0, beco_offset);
    while (kernelCnt) {
        beco_mmacgr(BECO_ACC0, (beco_vec32_in_t){.i8 = {*kernel, 0, 0, 0}}, BECO_REG0);
        beco_mmacgr(BECO_ACC1, (beco_vec32_in_t){.i8 = {*kernel, 0, 0, 0}}, BECO_REG0);
        beco_mmacgr(BECO_ACC2, (beco_vec32_in_t){.i8 = {*kernel, 0, 0, 0}}, BECO_REG0);
        beco_mmacgr(BECO_ACC3, (beco_vec32_in_t){.i8 = {*kernel++, 0, 0, 0}}, BECO_REG0);
        kernelCnt--;
    }
}

static void swap(int8_t* &a, int8_t* &b) { int8_t* c; c = a; a = b; b = c; }

static void beco_depthwise_conv_q7_copy_input(const int8_t *pin,
                                              int8_t *pout,
                                              const uint16_t stride_x,
                                              const uint16_t simd_width,
                                              const uint16_t copy_width,
                                              const uint16_t input_ch,
                                              const uint16_t input_x,
                                              const uint16_t input_y,
                                              const uint16_t kernel_x)
{
    if (stride_x == 1) {
        for (int c = 0; c < input_ch; c++) {
            memcpy(pout, pin, copy_width * sizeof(int8_t));
            pin += input_x * input_y;
            pout += simd_width * kernel_x;
        }
    } else {
        for (int c = 0; c < input_ch; c++) {
            const int8_t *pin1 = pin;
            int8_t *pout1 = pout;
            for (int i = 0; i < copy_width; i++) {
                *pout1++ = *pin1;
                pin1 += stride_x;
            }
            pin += input_x * input_y;
            pout += simd_width * kernel_x;
        }
    }
}

static void beco_depthwise_conv_q7_copy_out(int8_t *pin,
                                            int8_t *pout,
                                            const uint16_t copy_width)
{
    memcpy(pout, pin, copy_width * sizeof(int8_t));
}

static void data_arrange(int y,
                         int x,
                         const int8_t *pin,
                         int8_t *pout,
                         const uint16_t input_ch,
                         const uint16_t input_x,
                         const uint16_t input_y,
                         const uint16_t kernel_x,
                         const uint16_t padding_x,
                         const uint16_t stride_x,
                         const uint16_t max_step_x,
                         const uint16_t step_x,
                         const uint16_t sw,
                         const uint16_t cw,
                         const uint16_t dilation_x,
                         const int8_t input_offset)
{
    int8_t *po = pout;
    pin += input_x * y - padding_x + x;
    for (int kx = 0; kx < kernel_x; kx++) {
        beco_depthwise_conv_q7_copy_input(pin, po, stride_x, sw, cw,
                                          input_ch, input_x, input_y, kernel_x);
        pin += dilation_x;
        po += sw;
    }

    // set values left-side
    int cur_step_x = step_x * sw * stride_x - padding_x;
    if (cur_step_x < 0) {
        for (int i = 0; i < sw; i++) {
            for (int j = 0; j < kernel_x; j++) {
                int in_col = (sw*step_x + i) * stride_x + j * dilation_x - padding_x;
                if (in_col < 0) {
                    for (int c = 0; c < input_ch; c++) {
                        pout[c*sw*kernel_x + j*sw + i] = input_offset;
                    }
                } else {
                    break;
                }
            }
        }
    }

    // set values right-side
    cur_step_x = (step_x+1) * sw * stride_x + (kernel_x-1) * dilation_x - padding_x;
    if (cur_step_x >= input_x) {
        for (int i = 0; i < sw; i++) {
            for (int j = kernel_x-1; j >= 0; j--) {
                int in_col = (sw*step_x + i) * stride_x + j * dilation_x - padding_x;
                if (in_col >= input_x) {
                    for (int c = 0; c < input_ch; c++) {
                        pout[c*sw*kernel_x + j*sw + i] = input_offset;
                    }
                } else {
                    break;
                }
            }
        }
    }
}

static void update_image_cache(int iy,
                               int ix,
                               const int8_t *input,
                               int8_t **image_cache,
                               bool *image_cache_valid,
                               const uint16_t input_ch,
                               const uint16_t input_x,
                               const uint16_t input_y,
                               const uint16_t kernel_x,
                               const uint16_t kernel_dila_y,
                               const uint16_t padding_x,
                               const uint16_t padding_y,
                               const uint16_t stride_x,
                               const uint16_t stride_y,
                               const uint16_t max_step_x,
                               const uint16_t step_x,
                               const uint16_t simd_width,
                               const uint16_t copy_width,
                               const uint16_t dilation_x,
                               const uint16_t dilation_y,
                               const int8_t input_offset)
{
    // discard expired entries.
    if (iy != -padding_y) {
        for (int i = 0; i < kernel_dila_y - stride_y; i++) {
            swap(image_cache[i], image_cache[i + stride_y]);
            image_cache_valid[i] = image_cache_valid[i + stride_y];
            image_cache_valid[i + stride_y] = false;
        }
        memset(&image_cache_valid[kernel_dila_y - stride_y], false, stride_y * sizeof(bool));
    }

    // load new entries.
    for (int i = 0; i < kernel_dila_y; i += dilation_y) {
        if (iy + i >= input_y) {
            image_cache_valid[i] = false;
        } else if (iy + i >= 0 && image_cache_valid[i] == false) {
            data_arrange(iy + i, ix, input, image_cache[i], input_ch, input_x, input_y,
                         kernel_x, padding_x, stride_x, max_step_x,
                         step_x, simd_width, copy_width, dilation_x, input_offset);
            image_cache_valid[i] = true;
        }
    }
}

static bool get_arranged_input(int iy, int ky,
                               int8_t **image_cache,
                               bool *image_cache_valid,
                               int8_t *&cur_input)
{
    bool valid = iy + ky >= 0 && image_cache_valid[ky];
    if (valid) {
        cur_input = image_cache[ky];
        return true;
    } else {
        return false;
    }
}

void beco_depthwise_conv_q7_process(const int8_t *input,
                                    const uint16_t input_x,
                                    const uint16_t input_y,
                                    const uint16_t input_ch,
                                    const int8_t *kernel,
                                    const uint16_t output_ch,
                                    const uint16_t kernel_x,
                                    const uint16_t kernel_y,
                                    const uint16_t pad_x,
                                    const uint16_t pad_y,
                                    const uint16_t stride_x,
                                    const uint16_t stride_y,
                                    const uint16_t dilation_x,
                                    const uint16_t dilation_y,
                                    const int32_t *bias,
                                    const int32_t *multiplier,
                                    const int32_t *out_shift,
                                    int8_t *output,
                                    const uint16_t output_x,
                                    const uint16_t output_y,
                                    const int8_t input_offset,
                                    const int8_t out_offset,
                                    const int8_t min,
                                    const int8_t max,
                                    int8_t *buffer)
{
    const uint16_t dim_kernel_dila_y = kernel_y + (kernel_y - 1) * (dilation_y - 1);
    const uint16_t beco_output_x_4acc_cols = output_x >> 5;
    const uint16_t beco_output_x_cols = (output_x & 31) >> 3;
    const uint16_t remainder = (output_x & 31) & 7;
    const uint16_t max_step_x_4acc =  beco_output_x_4acc_cols - 1 + \
        ((beco_output_x_cols+remainder) ? 1 : 0);
    const uint16_t max_step_x = 4 * beco_output_x_4acc_cols + \
        beco_output_x_cols - 1 + (remainder ? 1 : 0);

    const uint16_t output_size_perchannel = output_x * output_y;
    const uint16_t kernel_size_perchannel = kernel_x * kernel_y;
    uint16_t simd_width = output_x < 32 ? 8 : 32;

    uint16_t im2col_buffer_len = dim_kernel_dila_y * kernel_x * simd_width * input_ch;
    int8_t *im2col_buffer = (int8_t *)buffer;
    int8_t *image_cache[dim_kernel_dila_y];
    bool image_cache_valid[dim_kernel_dila_y];
    // point image_cache to im2col_buffer
    for (int i = 0; i < dim_kernel_dila_y; i++) {
        image_cache[i] = &im2col_buffer[i*kernel_x*simd_width*input_ch];
    }

    beco_vec64_in_t beco_input_offset = {
        .i8 = {input_offset, input_offset, input_offset, input_offset,
               input_offset, input_offset, input_offset, input_offset}};

    int ix=0, iy=0;
#if defined(ARM_MATH_MVEI)
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *)(im2col_buffer + im2col_buffer_len);
#endif
    int8_t *cur_output = output;
    for (int i_out_x = 0; i_out_x < beco_output_x_4acc_cols; i_out_x++) {
        iy = -pad_y;
        int8_t *cur_im_out = cur_output;
        // clear image_cache_valid
        memset(image_cache_valid, false, dim_kernel_dila_y * sizeof(bool));

        for (int i_out_y = 0; i_out_y < output_y; i_out_y++, iy += stride_y) {

            // do img2col here
            update_image_cache(iy, ix, input, image_cache, image_cache_valid, input_ch, input_x, input_y,
                               kernel_x, dim_kernel_dila_y, pad_x, pad_y,
                               stride_x, stride_y, max_step_x_4acc, i_out_x, 32, 32,
                               dilation_x, dilation_y, input_offset);

            for (int i_ch = 0; i_ch < output_ch; i_ch++) {
                int8_t *cur_kernel = (int8_t *)kernel + i_ch * kernel_size_perchannel;
                if (bias) {
                    beco_set_preload_8x8_4ACC(bias[i_ch]);
                } else {
                    beco_clear_acc(BECO_ACC0);
                    beco_clear_acc(BECO_ACC1);
                    beco_clear_acc(BECO_ACC2);
                    beco_clear_acc(BECO_ACC3);
                }

                for (int ky = 0; ky < dim_kernel_dila_y; ky += dilation_y) {
                    int8_t *cur_input = NULL;
                    if (get_arranged_input(iy, ky, image_cache, image_cache_valid, cur_input)) {
                        beco_vec64_in_t *beco_in = (beco_vec64_in_t *)(cur_input + i_ch*kernel_x*32);
                        beco_depthwise_conv_q7_4accs_impl(beco_in, cur_kernel, kernel_x);
                    } else {
                        if (input_offset) {
                            beco_depthwise_conv_q7_offset_4accs_impl(beco_input_offset, cur_kernel, kernel_x);
                        }
                    }
                    cur_kernel += kernel_x;
                }
#if defined(ARM_MATH_MVEI)
                beco_depthwise_conv_q7_readout32_4ACC(beco_out);
                beco_depthwise_conv_q7_requant((int32_t *)beco_out, multiplier[i_ch], out_shift[i_ch],
                                               out_offset, min, max, 4,
                                               &cur_im_out[i_ch*output_size_perchannel]);
#else
                beco_depthwise_conv_q7_4accs_readout(&cur_im_out[i_ch*output_size_perchannel],
                                                     multiplier[i_ch], out_shift[i_ch],
                                                     out_offset, min, max);
#endif
            }
            cur_im_out += output_x;
        }
        cur_output += 32;
        ix += 32 * stride_x;
    }

    for (int i_out_x = 0; i_out_x < beco_output_x_cols; i_out_x++) {
        iy = -pad_y;
        int8_t *cur_im_out = cur_output;
        // clear image_cache_valid
        memset(image_cache_valid, false, dim_kernel_dila_y * sizeof(bool));

        for (int i_out_y = 0; i_out_y < output_y; i_out_y++, iy += stride_y) {

            // do img2col here
            update_image_cache(iy, ix, input, image_cache, image_cache_valid, input_ch, input_x, input_y,
                               kernel_x, dim_kernel_dila_y, pad_x, pad_y,
                               stride_x, stride_y, max_step_x, 4*beco_output_x_4acc_cols+i_out_x, 8, 8,
                               dilation_x, dilation_y, input_offset);

            for (int i_ch = 0; i_ch < output_ch; i_ch++) {
                int8_t *cur_kernel = (int8_t *)kernel + i_ch * kernel_size_perchannel;
                if (bias) {
                    beco_set_preload_8x8(bias[i_ch]);
                } else {
                    beco_clear_acc(BECO_ACC0);
                }

                for (int ky = 0; ky < dim_kernel_dila_y; ky += dilation_y) {
                    int8_t *cur_input = NULL;
                    if (get_arranged_input(iy, ky, image_cache, image_cache_valid, cur_input)) {
                        beco_vec64_in_t *beco_in = (beco_vec64_in_t *)(cur_input + i_ch*kernel_x*8);
                        beco_depthwise_conv_q7_impl(beco_in, cur_kernel, kernel_x);
                    } else {
                        if (input_offset) {
                            beco_depthwise_conv_q7_offset_impl(beco_input_offset, cur_kernel, kernel_x);
                        }
                    }
                    cur_kernel += kernel_x;
                }
#if defined(ARM_MATH_MVEI)
                beco_depthwise_conv_q7_readout32_ACC0(beco_out);
                beco_depthwise_conv_q7_requant((int32_t *)beco_out, multiplier[i_ch], out_shift[i_ch],
                                               out_offset, min, max, 1,
                                               &cur_im_out[i_ch*output_size_perchannel]);
#else
                beco_depthwise_conv_q7_readout(&cur_im_out[i_ch*output_size_perchannel], multiplier[i_ch], out_shift[i_ch],
                                               out_offset, min, max);
#endif
            }
            cur_im_out += output_x;
        }
        cur_output += 8;
        ix += 8 * stride_x;
    }

    if (remainder) {
        iy = -pad_y;
        int8_t *cur_im_out = cur_output;
#if defined(ARM_MATH_MVEI)
        int8_t *out_buffer = (int8_t *)(beco_out + simd_width);
#else
        int8_t *out_buffer = (int8_t *)(im2col_buffer + im2col_buffer_len);
#endif
        // clear image_cache_valid
        memset(image_cache_valid, false, dim_kernel_dila_y * sizeof(bool));

        for (int i_out_y = 0; i_out_y < output_y; i_out_y++, iy += stride_y) {

            // do img2col here
            update_image_cache(iy, ix, input, image_cache, image_cache_valid, input_ch, input_x, input_y,
                               kernel_x, dim_kernel_dila_y, pad_x, pad_y,
                               stride_x, stride_y, max_step_x, 4*beco_output_x_4acc_cols+beco_output_x_cols, 8, remainder,
                               dilation_x, dilation_y, input_offset);

            for (int i_ch = 0; i_ch < output_ch; i_ch++) {
                int8_t *cur_kernel = (int8_t *)kernel + i_ch * kernel_size_perchannel;
                if (bias) {
                    beco_set_preload_8x8(bias[i_ch]);
                } else {
                    beco_clear_acc(BECO_ACC0);
                }

                for (int ky = 0; ky < dim_kernel_dila_y; ky += dilation_y) {
                    int8_t *cur_input = NULL;
                    if (get_arranged_input(iy, ky, image_cache, image_cache_valid, cur_input)) {
                        beco_vec64_in_t *beco_in = (beco_vec64_in_t *)(cur_input + i_ch*kernel_x*8);
                        beco_depthwise_conv_q7_impl(beco_in, cur_kernel, kernel_x);
                    } else {
                        if (input_offset) {
                            beco_depthwise_conv_q7_offset_impl(beco_input_offset, cur_kernel, kernel_x);
                        }
                    }
                    cur_kernel += kernel_x;
                }
#if defined(ARM_MATH_MVEI)
                beco_depthwise_conv_q7_readout32_ACC0(beco_out);
                beco_depthwise_conv_q7_requant((int32_t *)beco_out, multiplier[i_ch], out_shift[i_ch],
                                               out_offset, min, max, 1, out_buffer);
#else
                beco_depthwise_conv_q7_readout(out_buffer, multiplier[i_ch], out_shift[i_ch],
                                               out_offset, min, max);
#endif
                beco_depthwise_conv_q7_copy_out(out_buffer, &cur_im_out[i_ch*output_size_perchannel], remainder);
            }
            cur_im_out += output_x;
        }
    }
}

void beco_depthwise_conv_q7(const int8_t* input,
                            const uint16_t input_x,
                            const uint16_t input_y,
                            const uint16_t input_ch,
                            const int8_t *kernel,
                            const uint16_t output_ch,
                            const uint16_t kernel_x,
                            const uint16_t kernel_y,
                            const uint16_t pad_x,
                            const uint16_t pad_y,
                            const uint16_t stride_x,
                            const uint16_t stride_y,
                            const uint16_t dilation_x,
                            const uint16_t dilation_y,
                            const int32_t *bias,
                            const int32_t *multiplier,
                            const int32_t *out_shift,
                            int8_t *output,
                            const uint16_t output_x,
                            const uint16_t output_y,
                            const int8_t input_offset,
                            const int8_t out_offset,
                            const int8_t min,
                            const int8_t max,
                            int8_t *buffer)
{
    uint32_t config =
        BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 |
        BECO_CONF_ATYPE_INT8  | BECO_CONF_BTYPE_INT8  |
        BECO_CONF_RSHIFT(0)   |
        BECO_CONF_PACK_INT32  | BECO_CONF_RD_8x8;
    beco_write_config(config);

    beco_depthwise_conv_q7_process(input, input_x, input_y, input_ch,
                                   kernel, output_ch, kernel_x, kernel_y, pad_x, pad_y,
                                   stride_x, stride_y, dilation_x, dilation_y,
                                   bias, multiplier, out_shift, output, output_x, output_y,
                                   input_offset, out_offset, min, max, buffer);
}

