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
#include "assert.h"

static void beco_convolve_q7_impl(beco_vec64_in_t *beco_in,
                                  beco_vec64_in_t *beco_wt,
                                  const uint16_t ch_im_in,
                                  const uint16_t dim_kernel_x,
                                  const uint32_t beco_och_stride,
                                  const uint32_t beco_wt_stride,
                                  const uint32_t grouped_index,
                                  const uint32_t beco_in_stride)
{
    uint16_t kernelCnt = dim_kernel_x;
    while (kernelCnt) {
        beco_vec64_in_t *cur_beco_in = beco_in + grouped_index;
        beco_vec64_in_t *cur_beco_wt = beco_wt;
        uint16_t ichCnt = ch_im_in >> 2;
        while (ichCnt) {
            beco_write_reg(BECO_REG0, *cur_beco_in++);
            beco_write_reg(BECO_REG1, *cur_beco_wt);
            beco_mmacrr(BECO_REG0, BECO_REG1);
            cur_beco_wt += beco_wt_stride;

            beco_write_reg(BECO_REG0, *cur_beco_in++);
            beco_write_reg(BECO_REG1, *cur_beco_wt);
            beco_mmacrr(BECO_REG0, BECO_REG1);
            cur_beco_wt += beco_wt_stride;

            beco_write_reg(BECO_REG0, *cur_beco_in++);
            beco_write_reg(BECO_REG1, *cur_beco_wt);
            beco_mmacrr(BECO_REG0, BECO_REG1);
            cur_beco_wt += beco_wt_stride;

            beco_write_reg(BECO_REG0, *cur_beco_in++);
            beco_write_reg(BECO_REG1, *cur_beco_wt);
            beco_mmacrr(BECO_REG0, BECO_REG1);
            cur_beco_wt += beco_wt_stride;

            ichCnt--;
        }

        ichCnt = ch_im_in & 3;
        while (ichCnt) {
            beco_write_reg(BECO_REG0, *cur_beco_in++);
            beco_write_reg(BECO_REG1, *cur_beco_wt);
            beco_mmacrr(BECO_REG0, BECO_REG1);
            cur_beco_wt += beco_wt_stride;

            ichCnt--;
        }

        beco_in += beco_in_stride;
        beco_wt += beco_och_stride;
        kernelCnt--;
    }
}

static void beco_convolve_q7_offset_impl(beco_vec64_in_t beco_in,
                                         beco_vec64_in_t *beco_wt,
                                         const uint16_t ch_im_in,
                                         const uint16_t dim_kernel_x,
                                         const uint32_t beco_och_stride,
                                         const uint32_t beco_wt_stride)
{
    uint16_t kernelCnt = dim_kernel_x;
    beco_write_reg(BECO_REG0, beco_in);
    while (kernelCnt) {
        beco_vec64_in_t *cur_beco_wt = beco_wt;
        uint16_t ichCnt = ch_im_in >> 2;
        while (ichCnt) {
            beco_write_reg(BECO_REG1, *cur_beco_wt);
            beco_mmacrr(BECO_REG0, BECO_REG1);
            cur_beco_wt += beco_wt_stride;

            beco_write_reg(BECO_REG1, *cur_beco_wt);
            beco_mmacrr(BECO_REG0, BECO_REG1);
            cur_beco_wt += beco_wt_stride;

            beco_write_reg(BECO_REG1, *cur_beco_wt);
            beco_mmacrr(BECO_REG0, BECO_REG1);
            cur_beco_wt += beco_wt_stride;

            beco_write_reg(BECO_REG1, *cur_beco_wt);
            beco_mmacrr(BECO_REG0, BECO_REG1);
            cur_beco_wt += beco_wt_stride;

            ichCnt--;
        }

        ichCnt = ch_im_in & 3;
        while (ichCnt) {
            beco_write_reg(BECO_REG1, *cur_beco_wt);
            beco_mmacrr(BECO_REG0, BECO_REG1);
            cur_beco_wt += beco_wt_stride;

            ichCnt--;
        }

        beco_wt += beco_och_stride;
        kernelCnt--;
    }
}

static void flush(int8_t **imCache, int8_t *imKernel,
                  const int dim_kernel_dila_y, const int strip_size)
{
    int8_t *p = imKernel;

    for (int i = 0; i < dim_kernel_dila_y; i++) {
        imCache[i] = p;
        p += strip_size / dim_kernel_dila_y;
    }
}

static inline void clearCacheValid(bool *cache_valid, const int dim_kernel_dila_y)
{
    memset(cache_valid, false, sizeof(*cache_valid) * dim_kernel_dila_y);
}

static void swap(int8_t* &a, int8_t* &b) { int8_t* c; c = a; a = b; b = c; }

static void copy_channels(int8_t *_pout,
                          const int8_t *_pin,
                          const int ch_im_in,
                          const int stride_x,
                          const int SIMDWidth,
                          const int dim_im_in_x,
                          const int dim_im_in_y)
{
    if (stride_x == 1) {
        for (int c = 0; c < ch_im_in; c++) {
            memcpy(_pout, _pin, sizeof(*_pout) * SIMDWidth);
            _pin += dim_im_in_x * dim_im_in_y;
            _pout += SIMDWidth;
        }
        return;
    }

    const int8_t *_pin2 = _pin;
    for (int c = 0; c < ch_im_in; c++) {
        _pin2 = _pin;
        for (int x = 0; x < SIMDWidth; x++) {
            *_pout++ = *_pin2;
            _pin2 += stride_x;
        }
        _pin += dim_im_in_x * dim_im_in_y;
    }
}

static inline int obtainStep(int a, int b, int c, int d)
{
    return ((a - b * d) % c == 0)?((a - b * d) / c):((a - b * d) / c + 1);
}

static void getStrip(int y,
                     int x,
                     const int8_t *pim,
                     int8_t *pout,
                     const int ch_im_in,
                     const int dim_im_in_x,
                     const int dim_im_in_y,
                     const int dim_kernel_x,
                     const int padding_start_x,
                     const int padding_end_x,
                     const int stride_x,
                     const int maxStepX,
                     const int stepX,
                     const int SIMDWidth,
                     const int remainder_x,
                     const int dilation_x,
                     const int8_t input_offset)
{
    int kx;
    int currentStep;

    int8_t *po = pout;
    pim +=  dim_im_in_x * y - padding_start_x + x;
    for (kx = 0; kx < dim_kernel_x; kx++) {
        copy_channels(pout, pim, ch_im_in, stride_x, SIMDWidth, dim_im_in_x, dim_im_in_y);
        pim += dilation_x;
        pout += ch_im_in * SIMDWidth;
    }

    // Clear values outside input matrix
    if (stepX <= padding_start_x / SIMDWidth) {
        for (int px = 0; px < padding_start_x; px++) {
            if (px / SIMDWidth == stepX) {
                currentStep = obtainStep(padding_start_x, px, dilation_x, stride_x);
                for (int c = 0; c < currentStep * ch_im_in; c++) {
                    po[c * SIMDWidth + px % SIMDWidth] = input_offset;
                }
            }
        }
    }

    if (maxStepX - stepX <= padding_end_x / SIMDWidth) {
        for (int px = 0; px < padding_end_x; px++) {
            if (maxStepX - stepX == px / SIMDWidth) {
                currentStep = obtainStep(padding_end_x, px, dilation_x, stride_x);
                for (int c = 0; c < currentStep * ch_im_in; c++) {
                    pout[-(c * SIMDWidth + (SIMDWidth-remainder_x) + (px % SIMDWidth)) - 1] = input_offset;
                }
            }
        }
    }
}

static void updateImcache(int iy,
                          int ix,
                          const int8_t *Im_in,
                          int8_t **imCache,
                          bool *cache_valid,
                          const int ch_im_in,
                          const int dim_im_in_x,
                          const int dim_im_in_y,
                          const int dim_kernel_x,
                          const int dim_kernel_dila_y,
                          const int padding_start_x,
                          const int padding_end_x,
                          const int padding_y,
                          const int stride_x,
                          const int stride_y,
                          const int maxStepX,
                          const int stepX,
                          const int SIMDWidth,
                          const int remainder_x,
                          const int dilation_x,
                          const int dilation_y,
                          const int8_t input_offset)
{
    // Discard expired entries.
    if (iy != -padding_y) {
        for (int i = 0; i < dim_kernel_dila_y - stride_y; i++) {
            swap(imCache[i], imCache[i + stride_y]);
            cache_valid[i] = cache_valid[i + stride_y];
            cache_valid[i + stride_y] = false;
        }
        for (int i = dim_kernel_dila_y - stride_y; i < dim_kernel_dila_y; i++) {
            cache_valid[i] = false;
        }
    }

    // Load new entries.
    for (int i = 0; i < dim_kernel_dila_y; i += dilation_y) {
        if (iy + i >= dim_im_in_y)
            cache_valid[i] = false;
        else if (iy + i >= 0 && cache_valid[i] == false) {
            assert(iy + i < dim_im_in_y);
            getStrip(iy + i, ix, Im_in, imCache[i], ch_im_in, dim_im_in_x, dim_im_in_y,
                     dim_kernel_x, padding_start_x, padding_end_x, stride_x, maxStepX,
                     stepX, SIMDWidth, remainder_x, dilation_x, input_offset);
            cache_valid[i] = true;
        }
    }
}

static inline bool valid(const int iy, const int ky, bool *cache_valid)
{
    return (iy + ky >= 0 && cache_valid[ky]);
}

static inline bool getArrangedInput(const int iy, const int ky, int8_t **imcache, 
                             bool *cache_valid, int8_t *&col)
{
    if (!valid(iy, ky, cache_valid)) return false;
    col = imcache[ky];
    return true;
}

static inline void beco_convolve_norm_q7_copy_out(int8_t *pin,
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

void beco_convolve_norm_q7_process(const int8_t *Im_in,
                                   const uint16_t dim_im_in_x,
                                   const uint16_t dim_im_in_y,
                                   const uint16_t ch_im_in,
                                   const int8_t *wt,
                                   const uint16_t ch_im_out,
                                   const uint16_t dim_kernel_x,
                                   const uint16_t dim_kernel_y,
                                   const uint16_t padding_x,
                                   const uint16_t padding_y,
                                   const uint16_t stride_x,
                                   const uint16_t stride_y,
                                   const int32_t *bias,
                                   const int32_t *multiplier,
                                   const int32_t *out_shift,
                                   int8_t *Im_out,
                                   const uint16_t dim_im_out_x,
                                   const uint16_t dim_im_out_y,
                                   const uint16_t dilation_x,
                                   const uint16_t dilation_y,
                                   const uint16_t groups,
                                   const int8_t input_offset,
                                   const int8_t out_offset,
                                   const int8_t min,
                                   const int8_t max,
                                   int8_t *buffer)
{
    int x, y, ch;
    int ix, iy, ky;

    const int dim_kernel_dila_y = dim_kernel_y + (dim_kernel_y-1) * (dilation_y-1);
    const int padding_start_x = padding_x;
    const int padding_end_x = (dim_im_out_x - 1) * stride_x + dim_kernel_x +
                              (dim_kernel_x - 1) * (dilation_x - 1) - dim_im_in_x - padding_x;
    assert((padding_start_x - padding_end_x) == 1 || (padding_start_x - padding_end_x) == 0);

    const int beco_im_out_x_num = dim_im_out_x >> 3;
    const int remainder = dim_im_out_x & 7;
    const int maxStepX = (beco_im_out_x_num - 1) + (remainder ? 1 : 0);

    const int kernel_och_stride = (ch_im_out * dim_kernel_x * dim_kernel_y) >> 3;
    const int och_stride = ch_im_out >> 3;
    const int out_stride_parchannel = dim_im_out_x * dim_im_out_y;
    const int grouped_in_ch = ch_im_in / groups;
    const int grouped_out_ch = ch_im_out / groups;
    const int is_group = groups == 1 ? 0 : 1;

    int8_t *imCache[dim_kernel_dila_y];
    bool cache_valid[dim_kernel_dila_y];
    const int strip_size = 8 * ch_im_in * dim_kernel_dila_y * dim_kernel_x;
    int8_t *imKernel = (int8_t *)buffer;
    flush(imCache, imKernel, dim_kernel_dila_y, strip_size); // point imCache to imKernel
    beco_vec64_in_t beco_input_offset = {.i8 = {input_offset, input_offset, input_offset, input_offset,
                                                input_offset, input_offset, input_offset, input_offset}};

#if defined(ARM_MATH_MVEI)
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *)(buffer + strip_size);
#else
    int8_t *out_buffer = (int8_t *)(buffer + strip_size);
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *)out_buffer;
#endif

    x = 0;
    ix = 0;
    const beco_vec64_in_t *beco_bias = (const beco_vec64_in_t *)bias;
    for (x = 0; x < beco_im_out_x_num; x++) {
        iy = -padding_y;
        int8_t *cur_im_out = Im_out;
        clearCacheValid(cache_valid, dim_kernel_dila_y);

        for (y = 0; y < dim_im_out_y; y++, iy += stride_y) {
            // first do img2col here
            updateImcache(iy, ix, Im_in, imCache, cache_valid, ch_im_in, dim_im_in_x, dim_im_in_y,
                          dim_kernel_x, dim_kernel_dila_y, padding_start_x, padding_end_x,
                          padding_y, stride_x, stride_y, maxStepX, x,
                          8, 8, dilation_x, dilation_y, input_offset);

            for (ch = 0; ch < ch_im_out; ch += 8) {
                beco_vec64_in_t *beco_wt = (beco_vec64_in_t *)(wt + ch);
                if (bias) {
                    beco_convolve_q7_set_bias(&beco_bias[ch/2]);
                }
                else {
                    beco_clear_acc(BECO_ACC0);
                    beco_clear_acc(BECO_ACC1);
                    beco_clear_acc(BECO_ACC2);
                    beco_clear_acc(BECO_ACC3);
                }

                for (ky = 0; ky < dim_kernel_dila_y; ky += dilation_y) {
                    int8_t *col;
                    if (getArrangedInput(iy, ky, imCache, cache_valid, col)) {
                        beco_vec64_in_t *beco_in = (beco_vec64_in_t *)col;
                        int32_t grouped_index = is_group*(ch/grouped_out_ch)*grouped_in_ch;
                        beco_convolve_q7_impl(beco_in, beco_wt, grouped_in_ch, dim_kernel_x,
                                              och_stride, kernel_och_stride, grouped_index, ch_im_in);
                    } else {
                        if (input_offset != 0) {
                            beco_convolve_q7_offset_impl(beco_input_offset, beco_wt, grouped_in_ch, dim_kernel_x,
                                                         och_stride, kernel_och_stride);
                        }
                    }
                    beco_wt += (ch_im_out * dim_kernel_x) >> 3;
                }
#if defined(ARM_MATH_MVEI)
                beco_convolve_q7_readout32(beco_out);
                beco_convolve_q7_requant((int32_t *)beco_out, &multiplier[ch], &out_shift[ch],
                                         out_offset, min, max, out_stride_parchannel,
                                         &cur_im_out[ch * out_stride_parchannel]);
#else
                beco_convolve_q7_readout(beco_out, 2, &multiplier[ch], &out_shift[ch], out_offset, min, max);
                beco_convolve_norm_q7_copy_out(out_buffer, &cur_im_out[ch*out_stride_parchannel],
                                               8, out_stride_parchannel);
#endif
            }
            cur_im_out += dim_im_out_x;
        }
        Im_out += 8;
        ix += 8 * stride_x;
    }

    // remainder
    if (remainder) {
        iy = -padding_y;
        int8_t *cur_im_out = Im_out;
        clearCacheValid(cache_valid, dim_kernel_dila_y);

        for (y = 0; y < dim_im_out_y; y++, iy += stride_y) {
            // first do img2col here
            updateImcache(iy, ix, Im_in, imCache, cache_valid, ch_im_in, dim_im_in_x, dim_im_in_y,
                          dim_kernel_x, dim_kernel_dila_y, padding_start_x, padding_end_x,
                          padding_y, stride_x, stride_y, maxStepX, x,
                          8, remainder, dilation_x, dilation_y, input_offset);

            for (ch = 0; ch < ch_im_out; ch += 8) {
                beco_vec64_in_t *beco_wt = (beco_vec64_in_t *)(wt + ch);
                if (bias) {
                    beco_convolve_q7_set_bias(&beco_bias[ch/2]);
                }
                else {
                    beco_clear_acc(BECO_ACC0);
                    beco_clear_acc(BECO_ACC1);
                    beco_clear_acc(BECO_ACC2);
                    beco_clear_acc(BECO_ACC3);
                }

                for (ky = 0; ky < dim_kernel_dila_y; ky += dilation_y) {
                    int8_t *col;
                    if (getArrangedInput(iy, ky, imCache, cache_valid, col)) {
                        beco_vec64_in_t *beco_in = (beco_vec64_in_t *)col;
                        int32_t grouped_index = is_group*(ch/grouped_out_ch)*grouped_in_ch;
                        beco_convolve_q7_impl(beco_in, beco_wt, grouped_in_ch, dim_kernel_x,
                                              och_stride, kernel_och_stride, grouped_index, ch_im_in);
                    } else {
                        if (input_offset != 0) {
                            beco_convolve_q7_offset_impl(beco_input_offset, beco_wt, grouped_in_ch, dim_kernel_x,
                                                         och_stride, kernel_och_stride);
                        }
                    }
                    beco_wt += (ch_im_out * dim_kernel_x) >> 3;
                }
#if defined(ARM_MATH_MVEI)
                beco_convolve_q7_readout32(beco_out);
                int8_t *out_buffer = (int8_t *)(beco_out + 64);
                beco_convolve_q7_requant((int32_t *)beco_out, &multiplier[ch], &out_shift[ch],
                                         out_offset, min, max, 8, out_buffer);
                beco_convolve_norm_q7_copy_out(out_buffer, &cur_im_out[ch*out_stride_parchannel],
                                               remainder, out_stride_parchannel);
#else
                beco_convolve_q7_readout(beco_out, 2, &multiplier[ch], &out_shift[ch], out_offset, min, max);
                beco_convolve_norm_q7_copy_out(out_buffer, &cur_im_out[ch*out_stride_parchannel],
                                               remainder, out_stride_parchannel);
#endif
            }
            cur_im_out += dim_im_out_x;
        }
    }
}

void beco_convolve_norm_q7(const int8_t *Im_in,
                           const uint16_t dim_im_in_x,
                           const uint16_t dim_im_in_y,
                           const uint16_t ch_im_in,
                           const int8_t *wt,
                           const uint16_t ch_im_out,
                           const uint16_t dim_kernel_x,
                           const uint16_t dim_kernel_y,
                           const uint16_t padding_x,
                           const uint16_t padding_y,
                           const uint16_t stride_x,
                           const uint16_t stride_y,
                           const int32_t *bias,
                           const int32_t *multiplier,
                           const int32_t *out_shift,
                           int8_t *Im_out,
                           const uint16_t dim_im_out_x,
                           const uint16_t dim_im_out_y,
                           const uint16_t dilation_x,
                           const uint16_t dilation_y,
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
        BECO_CONF_PACK_INT32 | BECO_CONF_RD_8x8;
    beco_write_config(config);
    beco_convolve_norm_q7_process(Im_in, dim_im_in_x, dim_im_in_y, ch_im_in, wt, ch_im_out,
                                  dim_kernel_x, dim_kernel_y, padding_x, padding_y,
                                  stride_x, stride_y, bias, multiplier, out_shift, Im_out,
                                  dim_im_out_x, dim_im_out_y, dilation_x, dilation_y, groups,
                                  input_offset, out_offset, min, max, buffer);
}

