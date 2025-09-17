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


static void beco_convolve_1x1_q7_impl(const beco_vec64_in_t *pIn,
                                      const beco_vec64_in_t *pKernel,
                                      const uint16_t in_ch,
                                      const int beco_in_stride,
                                      const int beco_wt_stride)
{
    uint16_t ichCnt = in_ch >> 2;
    while(ichCnt) {
        beco_write_reg(BECO_REG0, *pIn);
        beco_write_reg(BECO_REG1, *pKernel);
        beco_mmacrr(BECO_REG0, BECO_REG1);
        pIn += beco_in_stride;
        pKernel += beco_wt_stride;

        beco_write_reg(BECO_REG0, *pIn);
        beco_write_reg(BECO_REG1, *pKernel);
        beco_mmacrr(BECO_REG0, BECO_REG1);
        pIn += beco_in_stride;
        pKernel += beco_wt_stride;

        beco_write_reg(BECO_REG0, *pIn);
        beco_write_reg(BECO_REG1, *pKernel);
        beco_mmacrr(BECO_REG0, BECO_REG1);
        pIn += beco_in_stride;
        pKernel += beco_wt_stride;

        beco_write_reg(BECO_REG0, *pIn);
        beco_write_reg(BECO_REG1, *pKernel);
        beco_mmacrr(BECO_REG0, BECO_REG1);
        pIn += beco_in_stride;
        pKernel += beco_wt_stride;

        ichCnt--;
    }

    ichCnt = in_ch & 3;
    while(ichCnt) {
        beco_write_reg(BECO_REG0, *pIn);
        beco_write_reg(BECO_REG1, *pKernel);
        beco_mmacrr(BECO_REG0, BECO_REG1);
        pIn += beco_in_stride;
        pKernel += beco_wt_stride;

        ichCnt--;
    }
}

void beco_convolve_1x1_q7_process(const int8_t *Im_in,
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
    const uint16_t grouped_in_ch = ch_im_in / groups;
    const uint16_t grouped_out_ch = (ch_im_out / groups) >> 3;
    const uint16_t is_group = groups == 1 ? 0 : 1;

    const beco_vec64_in_t *beco_im_in =  (const beco_vec64_in_t *)Im_in;
    const beco_vec64_in_t *beco_wt =  (const beco_vec64_in_t *)wt;
    const beco_vec64_in_t *beco_bias = (const beco_vec64_in_t *)bias;
#if defined(ARM_MATH_MVEI)
    beco_vec32_out_t *beco_im_out =  (beco_vec32_out_t *)buffer;
#else
    beco_vec32_out_t *beco_im_out =  (beco_vec32_out_t *)Im_out;
    const uint32_t beco_im_out_stride = im_size >> 2;
    const uint32_t beco_im_out_7ch_stride = beco_im_out_stride * 7;
#endif

    const uint32_t beco_in_stride = im_size >> 3;
    const uint32_t beco_wt_stride = ch_im_out >> 3;

    for (unsigned i = 0; i < beco_out_ch_num; i++) {
        const beco_vec64_in_t *cur_beco_im_in = beco_im_in + \
            is_group * (i/grouped_out_ch) * grouped_in_ch * beco_im_num;
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
            beco_convolve_1x1_q7_impl(
                cur_beco_im_in, beco_wt, grouped_in_ch, beco_in_stride, beco_wt_stride);
#if defined(ARM_MATH_MVEI)
            beco_convolve_q7_readout32(beco_im_out);
            beco_convolve_q7_requant((int32_t *)beco_im_out, &multiplier[i*8], &out_shift[i*8],
                                     out_offset, min, max, im_size, Im_out);
            Im_out += 8;
            cur_beco_im_in++;
        }
        Im_out += 7 * im_size;
#else
            beco_convolve_q7_readout(beco_im_out, beco_im_out_stride,
                                     &multiplier[i*8], &out_shift[i*8], out_offset, min, max);
            beco_im_out += 2;
            cur_beco_im_in++;
        }
        beco_im_out += beco_im_out_7ch_stride;
#endif
        beco_wt++;
    }
}

void beco_convolve_1x1_q7(const int8_t *Im_in,
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
    beco_convolve_1x1_q7_process(Im_in, dim_im_in_x, dim_im_in_y, ch_im_in, wt, ch_im_out,
                                 bias, multiplier, out_shift, Im_out, dim_im_out_x, dim_im_out_y,
                                 groups, input_offset, out_offset, min, max, buffer);
}

