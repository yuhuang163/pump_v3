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

#include "beco_nnfunctions.h"
#include "arm_nnsupportfunctions.h"
#include "assert.h"

static int8_t beco_avgpool_q7_requant(int32_t val,
                                      int count,
                                      int8_t min,
                                      int8_t max)
{
    const int half_count = count >> 1;

#if defined(CHIP_BEST1603) || defined(CHIP_BEST1306)
    val = val > 67108864 ? val + 4160749568 : val;
#endif

    int32_t out = val > 0 ? (val + half_count) : (val - half_count);
    out = out / count;
    out = MAX(out, min);
    out = MIN(out, max);
    return (int8_t)out;
}

static void beco_avgpool_q7_readoutACC0(beco_vec32_out_t *out,
                                        const int count,
                                        const int8_t min,
                                        const int8_t max)
{
    out[0].i8[0] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 0).i32, count, min, max);
    out[0].i8[1] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 2).i32, count, min, max);
    out[0].i8[2] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 8).i32, count, min, max);
    out[0].i8[3] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 10).i32, count, min, max);
    out[1].i8[0] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 4).i32, count, min, max);
    out[1].i8[1] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 6).i32, count, min, max);
    out[1].i8[2] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 12).i32, count, min, max);
    out[1].i8[3] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 14).i32, count, min, max);
}

static void beco_avgpool_q7_readout4ACC(beco_vec32_out_t *out,
                                        const int count,
                                        const int8_t min,
                                        const int8_t max)
{
    out[0].i8[0] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 0).i32, count, min, max);
    out[0].i8[1] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 2).i32, count, min, max);
    out[0].i8[2] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 8).i32, count, min, max);
    out[0].i8[3] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 10).i32, count, min, max);
    out[1].i8[0] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 4).i32, count, min, max);
    out[1].i8[1] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 6).i32, count, min, max);
    out[1].i8[2] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 12).i32, count, min, max);
    out[1].i8[3] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC0, 14).i32, count, min, max);

    out[2].i8[0] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC1, 0).i32, count, min, max);
    out[2].i8[1] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC1, 2).i32, count, min, max);
    out[2].i8[2] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC1, 8).i32, count, min, max);
    out[2].i8[3] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC1, 10).i32, count, min, max);
    out[3].i8[0] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC1, 4).i32, count, min, max);
    out[3].i8[1] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC1, 6).i32, count, min, max);
    out[3].i8[2] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC1, 12).i32, count, min, max);
    out[3].i8[3] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC1, 14).i32, count, min, max);

    out[4].i8[0] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC2, 0).i32, count, min, max);
    out[4].i8[1] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC2, 2).i32, count, min, max);
    out[4].i8[2] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC2, 8).i32, count, min, max);
    out[4].i8[3] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC2, 10).i32, count, min, max);
    out[5].i8[0] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC2, 4).i32, count, min, max);
    out[5].i8[1] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC2, 6).i32, count, min, max);
    out[5].i8[2] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC2, 12).i32, count, min, max);
    out[5].i8[3] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC2, 14).i32, count, min, max);

    out[6].i8[0] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC3, 0).i32, count, min, max);
    out[6].i8[1] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC3, 2).i32, count, min, max);
    out[6].i8[2] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC3, 8).i32, count, min, max);
    out[6].i8[3] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC3, 10).i32, count, min, max);
    out[7].i8[0] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC3, 4).i32, count, min, max);
    out[7].i8[1] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC3, 6).i32, count, min, max);
    out[7].i8[2] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC3, 12).i32, count, min, max);
    out[7].i8[3] = beco_avgpool_q7_requant(beco_read_acc(BECO_ACC3, 14).i32, count, min, max);
}

static int8_t beco_avgpool_q7_ACC0_impl(const beco_vec64_in_t *beco_in,
                                        const uint16_t dim_im_in_x,
                                        const uint16_t dim_im_in_y,
                                        const uint16_t ker_x_start,
                                        const uint16_t ker_y_start,
                                        const uint16_t kernel_x_end,
                                        const uint16_t kernel_y_end,
                                        const int ch_stride,
                                        int base_idx_x,
                                        int base_idx_y)
{
    int k_y, k_x;
    int count = 0;
    int index;

    for (k_y = ker_y_start; k_y < kernel_y_end; k_y++) {
        const beco_vec64_in_t *cur_beco_in_row = beco_in + \
            (k_y + base_idx_y) * dim_im_in_x * ch_stride;

        for (k_x = ker_x_start; k_x < kernel_x_end; k_x++) {
            index = (k_x + base_idx_x) * ch_stride;
            beco_write_reg(BECO_REG0, cur_beco_in_row[index]);
            beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.i16 = {1, 0}}), BECO_REG0);
            count++;
        }

    }

    return count;
}

static int8_t beco_avgpool_q7_4ACC_impl(const beco_vec64_in_t *beco_in,
                                        const uint16_t dim_im_in_x, // 4
                                        const uint16_t dim_im_in_y, // 1
                                        const uint16_t ker_x_start, // 0
                                        const uint16_t ker_y_start,
                                        const uint16_t kernel_x_end, // 3
                                        const uint16_t kernel_y_end,
                                        const int ch_stride,
                                        int base_idx_x, // -1
                                        int base_idx_y)
{
    int k_y, k_x;
    int count = 0;
    int index;

    for (k_y = ker_y_start; k_y < kernel_y_end; k_y++) {
        const beco_vec64_in_t *cur_beco_in_row = beco_in + \
            (k_y + base_idx_y) * dim_im_in_x * ch_stride;

        for (k_x = ker_x_start; k_x < kernel_x_end; k_x++) {
            index = (k_x + base_idx_x) * ch_stride;
            beco_write_reg(BECO_REG0, cur_beco_in_row[index]);
            beco_write_reg(BECO_REG1, cur_beco_in_row[index+1]);
            beco_write_reg(BECO_REG2, cur_beco_in_row[index+2]);
            beco_write_reg(BECO_REG3, cur_beco_in_row[index+3]);
            beco_mmacgr4(((beco_vec32_in_t){.i16 = {1, 0}}), BECO_REG0);
            count++;
        }

    }

    return count;
}

void beco_avgpool_q7_process(const int8_t *src,
                             const uint16_t dim_im_in_x,
                             const uint16_t dim_im_in_y,
                             const uint16_t ch_im,
                             const uint16_t dim_kernel_x,
                             const uint16_t dim_kernel_y,
                             const uint16_t padding_x,
                             const uint16_t padding_y,
                             const uint16_t stride_x,
                             const uint16_t stride_y,
                             int8_t *dst,
                             const uint16_t dim_im_out_x,
                             const uint16_t dim_im_out_y,
                             const int8_t min,
                             const int8_t max)
{
    const int beco_ch_4acc_nums = ch_im >> 5;
    const int beco_ch_1acc_nums = (ch_im & 31) >> 3;
    const int beco_ch_stride = ch_im >> 3;

    const beco_vec64_in_t *beco_in = (const beco_vec64_in_t *)src;
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *)dst;

    int i_y, i_x, i_ch;
    int base_idx_y, base_idx_x;
    int count = 0;
    for (i_y = 0, base_idx_y = -padding_y; i_y < dim_im_out_y; base_idx_y += stride_y, i_y++) {
        for (i_x = 0, base_idx_x = -padding_x; i_x < dim_im_out_x; base_idx_x += stride_x, i_x++) {

            const beco_vec64_in_t *cur_beco_in = beco_in;
            const uint16_t ker_y_start = MAX(0, -base_idx_y);
            const uint16_t ker_x_start = MAX(0, -base_idx_x);
            const uint16_t kernel_y_end = MIN(dim_kernel_y, dim_im_in_y - base_idx_y);
            const uint16_t kernel_x_end = MIN(dim_kernel_x, dim_im_in_x - base_idx_x);

            // cal by beco acc0 ~ acc3
            for (i_ch = 0; i_ch < beco_ch_4acc_nums; i_ch++) {

                beco_clear_acc(BECO_ACC0);
                beco_clear_acc(BECO_ACC1);
                beco_clear_acc(BECO_ACC2);
                beco_clear_acc(BECO_ACC3);

                count = beco_avgpool_q7_4ACC_impl( \
                                cur_beco_in, dim_im_in_x, dim_im_in_y,
                                ker_x_start, ker_y_start, kernel_x_end, kernel_y_end,
                                beco_ch_stride, base_idx_x, base_idx_y);
                assert(count > 0);
                beco_avgpool_q7_readout4ACC(beco_out, count, min, max);

                beco_out += 8;
                cur_beco_in += 4;
            }

            // cal by beco acc0
            for (i_ch = 0; i_ch < beco_ch_1acc_nums; i_ch++) {

                beco_clear_acc(BECO_ACC0);

                count = beco_avgpool_q7_ACC0_impl( \
                                cur_beco_in, dim_im_in_x, dim_im_in_y,
                                ker_x_start, ker_y_start, kernel_x_end, kernel_y_end,
                                beco_ch_stride, base_idx_x, base_idx_y);
                assert(count > 0);
                beco_avgpool_q7_readoutACC0(beco_out, count, min, max);

                beco_out += 2;
                cur_beco_in++;
            }
        }
    }
}

void beco_avgpool_q7(const int8_t *src,
                     const uint16_t dim_im_in_x,
                     const uint16_t dim_im_in_y,
                     const uint16_t ch_im,
                     const uint16_t dim_kernel_x,
                     const uint16_t dim_kernel_y,
                     const uint16_t padding_x,
                     const uint16_t padding_y,
                     const uint16_t stride_x,
                     const uint16_t stride_y,
                     int8_t *dst,
                     const uint16_t dim_im_out_x,
                     const uint16_t dim_im_out_y,
                     const int8_t min,
                     const int8_t max)
{
    uint32_t config =
        BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 |
        BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT8 |
        BECO_CONF_RSHIFT(0) |
        BECO_CONF_PACK_INT32 | BECO_CONF_RD_16x8_ROT90;
    beco_write_config(config);

    beco_avgpool_q7_process(src, dim_im_in_x, dim_im_in_y, ch_im,
                            dim_kernel_x, dim_kernel_y, padding_x, padding_y, stride_x, stride_y,
                            dst, dim_im_out_x, dim_im_out_y, min, max);
}

