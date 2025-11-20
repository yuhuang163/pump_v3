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

static void beco_depthwise_conv_nx3_q15_ACC0_impl(const beco_vec64_in_t *beco_in,
                                                  const int8_t *wt,
                                                  const uint16_t in_row,
                                                  const uint16_t outw_stride,
                                                  const uint16_t i_stride_w,
                                                  const uint16_t stride_w,
                                                  const uint16_t stride_w4)
{
    beco_in += i_stride_w;
    beco_in += in_row * outw_stride;

    beco_write_reg(BECO_REG1, beco_in[0]);
    if (i_stride_w == 0)
        beco_move(BECO_REG0, BECO_REG5);
    else
        beco_write_reg(BECO_REG0, *(beco_in-1));
    beco_shift_block5(BECO_REG0, 6);
    beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.i8 = {wt[0], 0, 0, 0}}), BECO_REG0);

    beco_shift_block5(BECO_REG0, 2);
    beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.i8 = {wt[1], 0, 0, 0}}), BECO_REG0);

    if ((i_stride_w+1) == (4*stride_w4 + stride_w))
        beco_move(BECO_REG1, BECO_REG5);
    else
        beco_write_reg(BECO_REG1, beco_in[1]);
    beco_shift_block5(BECO_REG0, 2);
    beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.i8 = {wt[2], 0, 0, 0}}), BECO_REG0);
}

static void beco_depthwise_conv_nx3_q15_4ACC_impl(const beco_vec64_in_t *beco_in,
                                                  const int8_t *wt,
                                                  const uint16_t in_row,
                                                  const uint16_t outw_stride,
                                                  const uint16_t i_stride_w,
                                                  const uint16_t stride_w,
                                                  const uint16_t stride_w4)
{
    beco_in += 4 * i_stride_w;
    beco_in += in_row * outw_stride;

    beco_write_reg(BECO_REG1, beco_in[0]);
    beco_write_reg(BECO_REG2, beco_in[1]);
    beco_write_reg(BECO_REG3, beco_in[2]);
    beco_write_reg(BECO_REG4, beco_in[3]);
    if (i_stride_w == 0)
        beco_move(BECO_REG0, BECO_REG5);
    else
        beco_write_reg(BECO_REG0, *(beco_in-1));
    beco_shift_block5(BECO_REG0, 6);
    beco_mmacgr4(((beco_vec32_in_t){.i8 = {wt[0], 0, 0, 0}}), BECO_REG0);

    beco_shift_block5(BECO_REG0, 2);
    beco_mmacgr4(((beco_vec32_in_t){.i8 = {wt[1], 0, 0, 0}}), BECO_REG0);

    if ((i_stride_w+1) == stride_w4 && stride_w == 0)
        beco_move(BECO_REG4, BECO_REG5);
    else
        beco_write_reg(BECO_REG4, beco_in[4]);
    beco_shift_block5(BECO_REG0, 2);
    beco_mmacgr4(((beco_vec32_in_t){.i8 = {wt[2], 0, 0, 0}}), BECO_REG0);
}

void beco_depthwise_conv_nx3_q15_process(const int16_t *input,
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
                                         const int32_t *bias,
                                         const int32_t *multiplier,
                                         const int32_t *out_shift,
                                         int16_t *output,
                                         const uint16_t output_x,
                                         const uint16_t output_y,
                                         const int16_t min,
                                         const int16_t max,
                                         int16_t *buffer)
{
    int i_ch, i_out_y, i_out_x, i_ker_y;
#if defined(ARM_MATH_MVEI)
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *)buffer;
#else
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *)output;
#endif
    beco_vec64_in_t  *beco_in  = (beco_vec64_in_t *)input;

    const int beco_4acc_cols = output_x >> 4;
    const int beco_acc_cols = (output_x & 15) >> 2;
    const uint16_t outw_stride = output_x >> 2;
    beco_vec64_in_t padding_val = (beco_vec64_in_t){.u32 = {0, 0}};
    beco_write_reg(BECO_REG5, padding_val);

    for (i_ch = 0; i_ch < input_ch; i_ch++) {
        for (i_out_y = 0; i_out_y < output_y; i_out_y++) {
            // cal by beco acc0-3
            for (i_out_x = 0; i_out_x < beco_4acc_cols; i_out_x++) {
                if (bias) {
                    beco_set_preload_8x16_4ACC(bias[i_ch]);
                } else {
                    beco_clear_acc(BECO_ACC0);
                    beco_clear_acc(BECO_ACC1);
                    beco_clear_acc(BECO_ACC2);
                    beco_clear_acc(BECO_ACC3);
                }
                for (i_ker_y = 0; i_ker_y < kernel_y; i_ker_y++) {
                    int in_row = stride_y * i_out_y + i_ker_y - pad_y;
                    if (in_row >= 0 && in_row < input_y) {
                        beco_depthwise_conv_nx3_q15_4ACC_impl(beco_in, &kernel[3*i_ker_y], in_row,
                            outw_stride, i_out_x, beco_acc_cols, beco_4acc_cols);
                    }
                }
#if defined(ARM_MATH_MVEI)
                beco_depthwise_conv_q15_readout32_4ACC(beco_out);
                beco_depthwise_conv_q15_requant((int32_t *)beco_out, multiplier[i_ch], out_shift[i_ch],
                                                min, max, 4, output);
                output += 16;
#else
                beco_depthwise_conv_q15_readout4ACC( \
                    beco_out, multiplier[i_ch], out_shift[i_ch], min, max);
                beco_out += 8;
#endif
            }

            // cal by beco acc0
            int col_start = 4*beco_4acc_cols;
            for (i_out_x = col_start; i_out_x < col_start + beco_acc_cols; i_out_x++) {
                if (bias) {
                    beco_set_preload_8x16(bias[i_ch]);
                } else {
                    beco_clear_acc(BECO_ACC0);
                }
                for (i_ker_y = 0; i_ker_y < kernel_y; i_ker_y++) {
                    int in_row = stride_y * i_out_y + i_ker_y - pad_y;
                    if (in_row >= 0 && in_row < input_y) {
                        beco_depthwise_conv_nx3_q15_ACC0_impl(beco_in, &kernel[3*i_ker_y], in_row,
                            outw_stride, i_out_x, beco_acc_cols, beco_4acc_cols);
                    }
                }
#if defined(ARM_MATH_MVEI)
                beco_depthwise_conv_q15_readout32_ACC0(beco_out);
                beco_depthwise_conv_q15_requant((int32_t *)beco_out, multiplier[i_ch], out_shift[i_ch],
                                                min, max, 1, output);
                output += 4;
#else
                beco_depthwise_conv_q15_readoutACC0( \
                    beco_out, multiplier[i_ch], out_shift[i_ch], min, max);
                beco_out += 2;
#endif
            }
        }
        beco_in += (input_x * input_y) >> 2;
        kernel += kernel_x * kernel_y;
    }
}

void beco_depthwise_conv_nx3_q15(const int16_t *input,
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
                                 const int32_t *bias,
                                 const int32_t *multiplier,
                                 const int32_t *out_shift,
                                 int16_t *output,
                                 const uint16_t output_x,
                                 const uint16_t output_y,
                                 const int16_t min,
                                 const int16_t max,
                                 int16_t *buffer)
{
    uint32_t config =
        BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 |
        BECO_CONF_ATYPE_INT8  | BECO_CONF_BTYPE_INT16 |
        BECO_CONF_RSHIFT(0) |
        BECO_CONF_PACK_INT32   | BECO_CONF_RD_8x16;
    beco_write_config(config);

    beco_depthwise_conv_nx3_q15_process(input, input_x, input_y, input_ch,
                                        kernel, output_ch, kernel_x, kernel_y, pad_x, pad_y,
                                        stride_x, stride_y, bias, multiplier, out_shift,
                                        output, output_x, output_y, min, max, buffer);
}

