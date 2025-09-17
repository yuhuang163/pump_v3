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

static void beco_max_pool_q15_impl(const beco_vec64_in_t *beco_in,
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

    beco_move(BECO_REG4, BECO_REG0);

    for (k_y = ker_y_start; k_y < kernel_y_end; k_y++) {
        for (k_x = ker_x_start; k_x < kernel_x_end; k_x++) {
            beco_write_reg(BECO_REG3, \
                beco_in[(k_y + base_idx_y)*dim_im_in_x*ch_stride + (k_x + base_idx_x)*ch_stride]);
            beco_salu_max_16(BECO_REG4, BECO_REG3, BECO_REG4);
        }
    }

    beco_salu_max_16(BECO_REG4, BECO_REG0, BECO_REG4);
    beco_salu_min_16(BECO_REG4, BECO_REG1, BECO_REG4);
}

void beco_max_pool_q15_process(const int16_t *src,
                               const uint16_t dim_im_in_x,
                               const uint16_t dim_im_in_y,
                               const uint16_t ch_im,
                               const uint16_t dim_kernel_x,
                               const uint16_t dim_kernel_y,
                               const uint16_t padding_x,
                               const uint16_t padding_y,
                               const uint16_t stride_x,
                               const uint16_t stride_y,
                               int16_t *dst,
                               const uint16_t dim_im_out_x,
                               const uint16_t dim_im_out_y,
                               const int16_t min,
                               const int16_t max)
{
    const int beco_ch_num = ch_im >> 2;
    const int ch_stride = beco_ch_num;

    const beco_vec64_in_t *beco_in = (const beco_vec64_in_t *)src;
    beco_vec64_out_t *beco_out = (beco_vec64_out_t *)dst;
    beco_vec64_in_t beco_min_val = (beco_vec64_in_t){.i16 = {min,min,min,min}};
    beco_vec64_in_t beco_max_val = (beco_vec64_in_t){.i16 = {max,max,max,max}};
    beco_write_reg(BECO_REG0, beco_min_val);
    beco_write_reg(BECO_REG1, beco_max_val);

    int i_y, i_x, i_ch;
    int base_idx_y, base_idx_x;
    for (i_y = 0, base_idx_y = -padding_y; i_y < dim_im_out_y; base_idx_y += stride_y, i_y++) {
        for (i_x = 0, base_idx_x = -padding_x; i_x < dim_im_out_x; base_idx_x += stride_x, i_x++) {

            const beco_vec64_in_t *cur_beco_in = beco_in;
            const uint16_t ker_y_start = MAX(0, -base_idx_y);
            const uint16_t ker_x_start = MAX(0, -base_idx_x);
            const uint16_t kernel_y_end = MIN(dim_kernel_y, dim_im_in_y - base_idx_y);
            const uint16_t kernel_x_end = MIN(dim_kernel_x, dim_im_in_x - base_idx_x);

            for (i_ch = 0; i_ch < beco_ch_num; i_ch++) {

                beco_max_pool_q15_impl(cur_beco_in, dim_im_in_x, dim_im_in_y,
                                       ker_x_start, ker_y_start, kernel_x_end, kernel_y_end,
                                       ch_stride, base_idx_x, base_idx_y);
                *beco_out++ = beco_read_reg(BECO_REG4);
                cur_beco_in++;
            }
        }
    }
}

void beco_max_pool_q15(const int16_t *src,
                       const uint16_t dim_im_in_x,
                       const uint16_t dim_im_in_y,
                       const uint16_t ch_im,
                       const uint16_t dim_kernel_x,
                       const uint16_t dim_kernel_y,
                       const uint16_t padding_x,
                       const uint16_t padding_y,
                       const uint16_t stride_x,
                       const uint16_t stride_y,
                       int16_t *dst,
                       const uint16_t dim_im_out_x,
                       const uint16_t dim_im_out_y,
                       const int16_t min,
                       const int16_t max)
{
    beco_max_pool_q15_process(src, dim_im_in_x, dim_im_in_y, ch_im,
                              dim_kernel_x, dim_kernel_y, padding_x, padding_y, stride_x, stride_y,
                              dst, dim_im_out_x, dim_im_out_y, min, max);
}

