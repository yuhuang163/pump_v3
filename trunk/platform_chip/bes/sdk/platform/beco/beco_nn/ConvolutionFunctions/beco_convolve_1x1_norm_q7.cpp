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
static void beco_convolve_1x1_norm_q7_readout(
                                    int8_t *out, uint32_t stride,
                                    const int32_t *multiplier,
                                    const int32_t *shift,
                                    const int8_t offset,
                                    const int8_t min,
                                    const int8_t max)
{
    int8_t *out1 = out + 4*stride;
    const int32_t *multiplier1 = multiplier + 4;
    const int32_t *shift1 = shift + 4;
    beco_read_acc(BECO_ACC0, -1);
    for (int i = 0; i < 4; i++) {
        out[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC0, 1).i32, *multiplier, *shift, offset, min, max);
        out1[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC1, 0).i32, *multiplier1, *shift1, offset, min, max);
        out[i + 4] = beco_requantize_q7(beco_read_next_acc(BECO_ACC2, 0).i32, *multiplier, *shift, offset, min, max);
        out1[i + 4] = beco_requantize_q7(beco_read_next_acc(BECO_ACC3, 0).i32, *multiplier1, *shift1, offset, min, max);
    }
    out += stride;
    out1 += stride;
    multiplier++;
    multiplier1++;
    shift++;
    shift1++;
    for (int i = 0; i < 4; i++) {
        out[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC0, 1).i32, *multiplier, *shift, offset, min, max);
        out1[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC1, 0).i32, *multiplier1, *shift1, offset, min, max);
        out[i + 4] = beco_requantize_q7(beco_read_next_acc(BECO_ACC2, 0).i32, *multiplier, *shift, offset, min, max);
        out1[i + 4] = beco_requantize_q7(beco_read_next_acc(BECO_ACC3, 0).i32, *multiplier1, *shift1, offset, min, max);
    }
    out += stride;
    out1 += stride;
    multiplier++;
    multiplier1++;
    shift++;
    shift1++;
    for (int i = 0; i < 4; i++) {
        out[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC0, 1).i32, *multiplier, *shift, offset, min, max);
        out1[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC1, 0).i32, *multiplier1, *shift1, offset, min, max);
        out[i + 4] = beco_requantize_q7(beco_read_next_acc(BECO_ACC2, 0).i32, *multiplier, *shift, offset, min, max);
        out1[i + 4] = beco_requantize_q7(beco_read_next_acc(BECO_ACC3, 0).i32, *multiplier1, *shift1, offset, min, max);
    }
    out += stride;
    out1 += stride;
    multiplier++;
    multiplier1++;
    shift++;
    shift1++;
    for (int i = 0; i < 4; i++) {
        out[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC0, 1).i32, *multiplier, *shift, offset, min, max);
        out1[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC1, 0).i32, *multiplier1, *shift1, offset, min, max);
        out[i + 4] = beco_requantize_q7(beco_read_next_acc(BECO_ACC2, 0).i32, *multiplier, *shift, offset, min, max);
        out1[i + 4] = beco_requantize_q7(beco_read_next_acc(BECO_ACC3, 0).i32, *multiplier1, *shift1, offset, min, max);
    }
}
#endif

static void beco_convolve_1x1_norm_q7_impl(const beco_vec64_in_t *pIn,
                                           const beco_vec64_in_t *pKernel,
                                           const uint16_t in_ch,
                                           const int beco_wt_stride)
{
    uint16_t ichCnt = in_ch >> 2;
    while(ichCnt) {
        beco_write_reg(BECO_REG0, *pIn++);
        beco_write_reg(BECO_REG1, *pKernel);
        beco_mmacrr(BECO_REG0, BECO_REG1);
        pKernel += beco_wt_stride;

        beco_write_reg(BECO_REG0, *pIn++);
        beco_write_reg(BECO_REG1, *pKernel);
        beco_mmacrr(BECO_REG0, BECO_REG1);
        pKernel += beco_wt_stride;

        beco_write_reg(BECO_REG0, *pIn++);
        beco_write_reg(BECO_REG1, *pKernel);
        beco_mmacrr(BECO_REG0, BECO_REG1);
        pKernel += beco_wt_stride;

        beco_write_reg(BECO_REG0, *pIn++);
        beco_write_reg(BECO_REG1, *pKernel);
        beco_mmacrr(BECO_REG0, BECO_REG1);
        pKernel += beco_wt_stride;

        ichCnt--;
    }

    ichCnt = in_ch & 3;
    while(ichCnt) {
        beco_write_reg(BECO_REG0, *pIn++);
        beco_write_reg(BECO_REG1, *pKernel);
        beco_mmacrr(BECO_REG0, BECO_REG1);
        pKernel += beco_wt_stride;

        ichCnt--;
    }
}

static void beco_convolve_1x1_q7_copy_channels(const int8_t *pin,
                                               int8_t *pout,
                                               const int channels,
                                               const int copy_width,
                                               const int stride)
{
    for (int i = 0; i < channels; i++) {
        memcpy(pout, pin, copy_width);
        pin += stride;
        pout += 8;
    }
}

static void beco_convolve_1x1_q7_copy_out(int8_t *pin,
                                          int8_t *pout,
                                          const int copy_width,
                                          const int stride)
{
    for (int i = 0; i < 8; i++) {
        memcpy(pout, pin, copy_width);
        pin += 8;
        pout += stride;
    }
}

void beco_convolve_1x1_norm_q7_process(const int8_t *Im_in,
                                       const uint16_t dim_im_in_x,
                                       const uint16_t dim_im_in_y,
                                       const uint16_t ch_im_in,
                                       const int8_t *wt,
                                       const uint16_t ch_im_out,
                                       const int32_t *bias,
                                       const int32_t *multiplier,
                                       const int32_t *out_shift,
                                       int8_t *Im_out,
                                       const uint16_t dim_im_out_x,
                                       const uint16_t dim_im_out_y,
                                       const uint16_t groups,
                                       const int8_t input_offset,
                                       const int8_t out_offset,
                                       const int8_t min,
                                       const int8_t max,
                                       int8_t *buffer)
{
    const uint32_t im_size = dim_im_in_y * dim_im_in_x;
    const unsigned beco_out_ch_num = ch_im_out >> 3;
    const unsigned beco_im_num = im_size >> 3;
    const int remainder = im_size & 7;
    const uint16_t grouped_in_ch = ch_im_in / groups;
    const uint16_t grouped_out_ch = (ch_im_out / groups) >> 3;
    const uint16_t is_group = groups == 1 ? 0 : 1;

    const beco_vec64_in_t *beco_wt =  (const beco_vec64_in_t *)wt;
    const beco_vec64_in_t *beco_bias = (const beco_vec64_in_t *)bias;
    int8_t *imCache = (int8_t *)buffer;
    const beco_vec64_in_t *beco_im_in =  (const beco_vec64_in_t *)imCache;

#if defined(ARM_MATH_MVEI)
    beco_vec32_out_t *beco_im_out =  (beco_vec32_out_t *)(buffer + 8 * grouped_in_ch);
#else
    int8_t *beco_im_out = Im_out;
    const uint32_t beco_im_out_stride = im_size;
    const uint32_t beco_im_out_7ch_stride = beco_im_out_stride * 7;
#endif

    const uint32_t beco_wt_stride = ch_im_out >> 3;

    for (unsigned i = 0; i < beco_out_ch_num; i++) {
        int grouped_index = is_group * (i/grouped_out_ch) * grouped_in_ch * im_size;
        for (unsigned j = 0; j < beco_im_num; j++) {
            if (bias) {
                beco_convolve_q7_set_bias(&beco_bias[4*i]);
            }
            else {
                beco_clear_acc(BECO_ACC0);
                beco_clear_acc(BECO_ACC1);
                beco_clear_acc(BECO_ACC2);
                beco_clear_acc(BECO_ACC3);
            }

            beco_convolve_1x1_q7_copy_channels( \
                &Im_in[8*j] + grouped_index, imCache, grouped_in_ch, 8, im_size);

            beco_convolve_1x1_norm_q7_impl( \
                beco_im_in, beco_wt, grouped_in_ch, beco_wt_stride);
#if defined(ARM_MATH_MVEI)
            beco_convolve_q7_readout32(beco_im_out);
            beco_convolve_q7_requant((int32_t *)beco_im_out, &multiplier[i*8], &out_shift[i*8],
                                     out_offset, min, max, im_size, Im_out);
            Im_out += 8;
#else
            beco_convolve_1x1_norm_q7_readout( \
                    beco_im_out, beco_im_out_stride, \
                    &multiplier[i*8], &out_shift[i*8], out_offset, min, max);
            beco_im_out += 8;
#endif
        }

        if (remainder) {
            if (bias) {
                beco_convolve_q7_set_bias(&beco_bias[4*i]);
            }
            else {
                beco_clear_acc(BECO_ACC0);
                beco_clear_acc(BECO_ACC1);
                beco_clear_acc(BECO_ACC2);
                beco_clear_acc(BECO_ACC3);
            }

            beco_convolve_1x1_q7_copy_channels( \
                &Im_in[8*beco_im_num] + grouped_index, imCache, grouped_in_ch, remainder, im_size);

            beco_convolve_1x1_norm_q7_impl( \
                beco_im_in, beco_wt, grouped_in_ch, beco_wt_stride);
#if defined(ARM_MATH_MVEI)
            beco_convolve_q7_readout32(beco_im_out);
            int8_t *out_buffer = (int8_t *)(beco_im_out + 64);
            beco_convolve_q7_requant((int32_t *)beco_im_out, &multiplier[i*8], &out_shift[i*8],
                                     out_offset, min, max, 8, out_buffer);
            beco_convolve_1x1_q7_copy_out(out_buffer, Im_out, remainder, im_size);
            Im_out += remainder;
        }

        Im_out += 7 * im_size;
#else
            int8_t *out_temp = (int8_t *)(buffer + 8 * grouped_in_ch);
            beco_convolve_1x1_norm_q7_readout(out_temp, 8, \
                    &multiplier[i*8], &out_shift[i*8], out_offset, min, max);
            beco_convolve_1x1_q7_copy_out( \
                out_temp, beco_im_out, remainder, im_size);
            beco_im_out += remainder;
        }

        beco_im_out += beco_im_out_7ch_stride;
#endif
        beco_wt++;
    }
}

void beco_convolve_1x1_norm_q7(const int8_t *Im_in,
                               const uint16_t dim_im_in_x,
                               const uint16_t dim_im_in_y,
                               const uint16_t ch_im_in,
                               const int8_t *wt,
                               const uint16_t ch_im_out,
                               const int32_t *bias,
                               const int32_t *multiplier,
                               const int32_t *out_shift,
                               int8_t *Im_out,
                               const uint16_t dim_im_out_x,
                               const uint16_t dim_im_out_y,
                               const uint16_t groups,
                               const int8_t input_offset,
                               const int8_t out_offset,
                               const int8_t min,
                               const int8_t max,
                               int8_t *buffer)
{
    uint32_t config =
        BECO_CONF_AMODE_REP32 | BECO_CONF_BMODE_REP32 |
        BECO_CONF_ATYPE_INT8  | BECO_CONF_BTYPE_INT8  |
        BECO_CONF_RSHIFT(0) |
        BECO_CONF_PACK_INT32   | BECO_CONF_RD_8x8;
    beco_write_config(config);
    beco_convolve_1x1_norm_q7_process(Im_in, dim_im_in_x, dim_im_in_y, ch_im_in, wt, ch_im_out,
                                      bias, multiplier, out_shift, Im_out, dim_im_out_x, dim_im_out_y,
                                      groups, input_offset, out_offset, min, max, buffer);
}

