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


#ifndef _BECO_NNSUPPORTFUNCTIONS_H
#define _BECO_NNSUPPORTFUNCTIONS_H

#include "beco.h"
#include "beco_types.h"
#include "arm_nnsupportfunctions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Beco nn requantize
 *
 * Beco requantize functions used in readout.
 *
 */

__STATIC_FORCEINLINE int8_t beco_requantize_q7(int32_t val, int32_t mult,
                                               int32_t shift, int8_t out_offset,
                                               int8_t min, int8_t max)
{
    int32_t in, out;
    int64_t long_long = 1 << 30;

#if defined(CHIP_BEST1603) || defined(CHIP_BEST1306)
    val = val > 67108864 ? val + 4160749568 : val;
#endif

    in = val;
    in = ((int64_t) in * mult + long_long) >> 31;
    out = (in + NN_ROUND(-shift)) >> -shift;

    out += out_offset;
    out = MAX(out, min);
    out = MIN(out, max);
    return (int8_t)out;
}


__STATIC_FORCEINLINE int16_t beco_requantize_q15(int32_t val, int32_t mult,
                                                 int32_t shift, int16_t min, int16_t max)
{
    int32_t in, out;

#if defined(CHIP_BEST1603) || defined(CHIP_BEST1306)
    val = val > 67108864 ? val + 4160749568 : val;
#endif

    in = val;
    mult = REDUCE_MULTIPLIER(mult);
    in = ((int64_t) in * mult) >> (14 - shift);
    out = (in + 1) >> 1;

    out = MAX(out, min);
    out = MIN(out, max);
    return (int16_t)out;
}


/**
 * @defgroup Beco nn set bias
 *
 * Preload bias data in beco ACCs.
 *
 */

void beco_fully_connected_set_bias(const beco_vec64_in_t *bias);


void beco_convolve_q7_set_bias(const beco_vec64_in_t *beco_bias);

void beco_convolve_q15_set_bias(const beco_vec64_in_t *beco_bias);

/**
 * @defgroup Beco nn readout
 *
 * Readout data from beco ACCs to the output pointer.
 *
 */

void beco_fully_connected_q7_readout(beco_vec32_out_t *out,
                                     const int32_t multiplier,
                                     const int32_t shift,
                                     const int8_t offset,
                                     const int8_t min,
                                     const int8_t max);

void beco_fully_connected_q7_readout_pc(beco_vec32_out_t *out,
                                        const int32_t *multiplier,
                                        const int32_t *shift,
                                        const int8_t offset,
                                        const int8_t min,
                                        const int8_t max);

void beco_fully_connected_q15_readout(beco_vec32_out_t *out,
                                      const int32_t multiplier,
                                      const int32_t shift,
                                      const int16_t min,
                                      const int16_t max);

void beco_fully_connected_q15_readout_pc(beco_vec32_out_t *out,
                                         const int32_t *multiplier,
                                         const int32_t *shift,
                                         const int16_t min,
                                         const int16_t max);

void beco_convolve_q7_readout(beco_vec32_out_t *out,
                              uint32_t stride,
                              const int32_t *multiplier,
                              const int32_t *shift,
                              const int8_t offset,
                              const int8_t min,
                              const int8_t max);

void beco_convolve_q15_readout(beco_vec32_out_t *out,
                               uint32_t stride,
                               const int32_t *multiplier,
                               const int32_t *shift,
                               const int16_t min,
                               const int16_t max);


void beco_depthwise_conv_q7_readoutACC0(beco_vec32_out_t *out,
                                        const int32_t multiplier,
                                        const int32_t shift,
                                        const int8_t offset,
                                        const int8_t min,
                                        const int8_t max);

void beco_depthwise_conv_q7_readout4ACC(beco_vec32_out_t *out,
                                        const int32_t multiplier,
                                        const int32_t shift,
                                        const int8_t offset,
                                        const int8_t min,
                                        const int8_t max);

void beco_depthwise_conv_q15_readoutACC0(beco_vec32_out_t *out,
                                         const int32_t multiplier,
                                         const int32_t shift,
                                         const int16_t min,
                                         const int16_t max);

void beco_depthwise_conv_q15_readout4ACC(beco_vec32_out_t *out,
                                         const int32_t multiplier,
                                         const int32_t shift,
                                         const int16_t min,
                                         const int16_t max);


#if defined(ARM_MATH_MVEI)
void beco_fully_connected_readout32(beco_vec32_out_t *out);

void beco_fully_connected_q7_requant(int32_t *val,
                                     const int32_t multiplier,
                                     const int32_t shift,
                                     const int8_t offset,
                                     const int8_t min,
                                     const int8_t max,
                                     int8_t *output);

void beco_fully_connected_q15_requant(int32_t *val,
                                      const int32_t multiplier,
                                      const int32_t shift,
                                      const int16_t min,
                                      const int16_t max,
                                      int16_t *output);


void beco_convolve_q7_readout32(beco_vec32_out_t *out);

void beco_convolve_q15_readout32(beco_vec32_out_t *out);

void beco_convolve_q7_requant(int32_t *val,
                              const int32_t *multiplier,
                              const int32_t *shift,
                              const int8_t offset,
                              const int8_t min,
                              const int8_t max,
                              const uint16_t stride,
                              int8_t *output);

void beco_convolve_q15_requant(int32_t *val,
                               const int32_t *multiplier,
                               const int32_t *shift,
                               const int16_t min,
                               const int16_t max,
                               const uint16_t stride,
                               int16_t *output);

__STATIC_FORCEINLINE void beco_depthwise_conv_q7_readout32_ACC0(beco_vec32_out_t *out)
{
    beco_read_acc(BECO_ACC0, -2);
    for (int i = 0; i < 8; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 2);
    }
}

__STATIC_FORCEINLINE void beco_depthwise_conv_q7_readout32_4ACC(beco_vec32_out_t *out)
{
    beco_read_acc(BECO_ACC0, -2);
    for (int i = 0; i < 8; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 2);
        out[i + 8] = beco_read_next_acc(BECO_ACC1, 0);
        out[i + 16] = beco_read_next_acc(BECO_ACC2, 0);
        out[i + 24] = beco_read_next_acc(BECO_ACC3, 0);
    }
}

__STATIC_FORCEINLINE void beco_depthwise_conv_q15_readout32_ACC0(beco_vec32_out_t *out)
{
    out[0] = beco_read_acc(BECO_ACC0, 0);
    out[1] = beco_read_acc(BECO_ACC0, 4);
    out[2] = beco_read_acc(BECO_ACC0, 8);
    out[3] = beco_read_acc(BECO_ACC0, 12);
}

__STATIC_FORCEINLINE void beco_depthwise_conv_q15_readout32_4ACC(beco_vec32_out_t *out)
{
    beco_read_acc(BECO_ACC0, -4);
    for (int i = 0; i < 4; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 4);
        out[i + 4] = beco_read_next_acc(BECO_ACC1, 0);
        out[i + 8] = beco_read_next_acc(BECO_ACC2, 0);
        out[i + 12] = beco_read_next_acc(BECO_ACC3, 0);
    }
}

__STATIC_FORCEINLINE void beco_depthwise_conv_q7_requant(int32_t *val,
                                                         const int32_t multiplier,
                                                         const int32_t shift,
                                                         const int8_t offset,
                                                         const int8_t min,
                                                         const int8_t max,
                                                         const int8_t accs,
                                                         int8_t *output)
{
    for (int i = 0; i < accs; i++) {
        int32x4_t acc1 = {val[0], val[2], val[1], val[3]};
        int32x4_t acc2 = {val[4], val[6], val[5], val[7]};

        acc1 = arm_requantize_mve(acc1, multiplier, shift);
        acc2 = arm_requantize_mve(acc2, multiplier, shift);

        acc1 = vaddq_s32(acc1, vdupq_n_s32(offset));
        acc1 = vmaxq_s32(acc1, vdupq_n_s32(min));
        acc1 = vminq_s32(acc1, vdupq_n_s32(max));
        acc2 = vaddq_s32(acc2, vdupq_n_s32(offset));
        acc2 = vmaxq_s32(acc2, vdupq_n_s32(min));
        acc2 = vminq_s32(acc2, vdupq_n_s32(max));

        vstrbq_s32(output, acc1);
        output += 4;
        vstrbq_s32(output, acc2);
        output += 4;
        val += 8;
    }
}

__STATIC_FORCEINLINE void beco_depthwise_conv_q15_requant(int32_t *val,
                                                          const int32_t multiplier,
                                                          const int32_t shift,
                                                          const int16_t min,
                                                          const int16_t max,
                                                          const int16_t accs,
                                                          int16_t *output)
{
    const int32x4_t right_shift = vminq_s32(vdupq_n_s32(-1), vdupq_n_s32(shift));
    const int32x4_t left_shift = vqsubq_s32(vdupq_n_s32(shift), right_shift);
    for (int i = 0; i < accs; i++) {
        int32x4_t acc1 = {*val++, *val++, *val++, *val++};

        acc1 = vqdmulhq_n_s32(vshlq_s32(acc1, left_shift), multiplier);
        acc1 = vrshlq_s32(acc1, right_shift);

        acc1 = vmaxq_s32(acc1, vdupq_n_s32(min));
        acc1 = vminq_s32(acc1, vdupq_n_s32(max));

        vstrhq_s32(output, acc1);
        output += 4;
    }
}
#endif

#ifdef __cplusplus
}
#endif

#endif
