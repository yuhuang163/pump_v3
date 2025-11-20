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

#if defined(ARM_MATH_MVEI)

static void beco_fully_connected_q15_readout32(beco_vec32_out_t *out)
{
    beco_read_acc(BECO_ACC0, -2);
    for (int i = 0; i < 8; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 2);
        out[i + 8] = beco_read_next_acc(BECO_ACC1, 0);
        out[i + 16] = beco_read_next_acc(BECO_ACC2, 0);
        out[i + 24] = beco_read_next_acc(BECO_ACC3, 0);
    }
}

static void beco_fully_connected_pt_q15_requant(int32_t *val,
                                                const int32_t multiplier,
                                                const int32_t shift,
                                                int16_t *output)
{
    const int32x4_t right_shift = vminq_s32(vdupq_n_s32(-1), vdupq_n_s32(shift));
    const int32x4_t left_shift = vqsubq_s32(vdupq_n_s32(shift), right_shift);
    for (int i = 0; i < 4; i++) {
        int32x4_t acc1 = {val[0], val[1], val[4], val[5]};
        int32x4_t acc2 = {val[2], val[3], val[6], val[7]};

        acc1 = vqdmulhq_n_s32(vshlq_s32(acc1, left_shift), multiplier);
        acc1 = vrshlq_s32(acc1, right_shift);
        acc2 = vqdmulhq_n_s32(vshlq_s32(acc2, left_shift), multiplier);
        acc2 = vrshlq_s32(acc2, right_shift);

        vstrhq_s32(output, acc1);
        output += 4;
        vstrhq_s32(output, acc2);
        output += 4;
        val += 8;
    }
}

static void beco_fully_connected_pc_q15_requant(int32_t *val,
                                                const int32_t *multiplier,
                                                const int32_t *shift,
                                                int16_t *output)
{
    for (int i = 0; i < 4; i++) {
        int32x4_t acc1 = {val[0], val[1], val[4], val[5]};
        int32x4_t acc2 = {val[2], val[3], val[6], val[7]};
        int32x4_t multiplier1 = {*multiplier++, *multiplier++, *multiplier++, *multiplier++};
        int32x4_t multiplier2 = {*multiplier++, *multiplier++, *multiplier++, *multiplier++};
        int32x4_t shift1 = {*shift++, *shift++, *shift++, *shift++};
        int32x4_t shift2 = {*shift++, *shift++, *shift++, *shift++};
        int32x4_t right_shift = vminq_s32(vdupq_n_s32(-1), shift1);
        int32x4_t left_shift = vqsubq_s32(shift1, right_shift);
        acc1 = vqdmulhq_s32(vshlq_s32(acc1, left_shift), multiplier1);
        acc1 = vrshlq_s32(acc1, right_shift);
        right_shift = vminq_s32(vdupq_n_s32(-1), shift2);
        left_shift = vqsubq_s32(shift2, right_shift);
        acc2 = vqdmulhq_s32(vshlq_s32(acc2, left_shift), multiplier2);
        acc2 = vrshlq_s32(acc2, right_shift);

        vstrhq_s32(output, acc1);
        output += 4;
        vstrhq_s32(output, acc2);
        output += 4;
        val += 8;
    }
}
#else
static int16_t beco_fully_connected_q15_requant(int32_t val, int32_t mult, int32_t shift)
{
    int32_t in, out;

#if defined(CHIP_BEST1603) || defined(CHIP_BEST1306)
    val = val > 67108864 ? val + 4160749568 : val;
#endif

    in = val;
    mult = REDUCE_MULTIPLIER(mult);
    in = ((int64_t) in * mult) >> (14 - shift);
    out = (in + 1) >> 1;

    return (int16_t)__SSAT(out, 16);
}

static void beco_fully_connected_pt_q15_readout(beco_vec32_out_t *out,
                                                const int32_t multiplier,
                                                const int32_t shift)
{
    beco_read_acc(BECO_ACC0, -2);
    uint8_t index1[] = {0, 0, 2, 2, 1, 1, 3, 3};
    uint8_t index2[] = {0, 1, 0, 1, 0, 1, 0, 1};
    for (int i = 0; i < 8; i++) {
        out[index1[i]].i16[index2[i]] = beco_fully_connected_q15_requant(beco_read_next_acc(BECO_ACC0, 2).i32, multiplier, shift);
        out[index1[i]+4].i16[index2[i]] = beco_fully_connected_q15_requant(beco_read_next_acc(BECO_ACC1, 0).i32, multiplier, shift);
        out[index1[i]+8].i16[index2[i]] = beco_fully_connected_q15_requant(beco_read_next_acc(BECO_ACC2, 0).i32, multiplier, shift);
        out[index1[i]+12].i16[index2[i]] = beco_fully_connected_q15_requant(beco_read_next_acc(BECO_ACC3, 0).i32, multiplier, shift);
    }
}

static void beco_fully_connected_pc_q15_readout(beco_vec32_out_t *out,
                                                const int32_t *multiplier,
                                                const int32_t *shift)
{
    out[0].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC0, 0).i32, *multiplier++, *shift++);
    out[0].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC0, 2).i32, *multiplier++, *shift++);
    out[1].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC0, 8).i32, *multiplier++, *shift++);
    out[1].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC0, 10).i32, *multiplier++, *shift++);
    out[2].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC0, 4).i32, *multiplier++, *shift++);
    out[2].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC0, 6).i32, *multiplier++, *shift++);
    out[3].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC0, 12).i32, *multiplier++, *shift++);
    out[3].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC0, 14).i32, *multiplier++, *shift++);

    out[4].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC1, 0).i32, *multiplier++, *shift++);
    out[4].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC1, 2).i32, *multiplier++, *shift++);
    out[5].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC1, 8).i32, *multiplier++, *shift++);
    out[5].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC1, 10).i32, *multiplier++, *shift++);
    out[6].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC1, 4).i32, *multiplier++, *shift++);
    out[6].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC1, 6).i32, *multiplier++, *shift++);
    out[7].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC1, 12).i32, *multiplier++, *shift++);
    out[7].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC1, 14).i32, *multiplier++, *shift++);

    out[8].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC2, 0).i32, *multiplier++, *shift++);
    out[8].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC2, 2).i32, *multiplier++, *shift++);
    out[9].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC2, 8).i32, *multiplier++, *shift++);
    out[9].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC2, 10).i32, *multiplier++, *shift++);
    out[10].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC2, 4).i32, *multiplier++, *shift++);
    out[10].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC2, 6).i32, *multiplier++, *shift++);
    out[11].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC2, 12).i32, *multiplier++, *shift++);
    out[11].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC2, 14).i32, *multiplier++, *shift++);

    out[12].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC3, 0).i32, *multiplier++, *shift++);
    out[12].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC3, 2).i32, *multiplier++, *shift++);
    out[13].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC3, 8).i32, *multiplier++, *shift++);
    out[13].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC3, 10).i32, *multiplier++, *shift++);
    out[14].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC3, 4).i32, *multiplier++, *shift++);
    out[14].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC3, 6).i32, *multiplier++, *shift++);
    out[15].i16[0] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC3, 12).i32, *multiplier++, *shift++);
    out[15].i16[1] = beco_fully_connected_q15_requant(beco_read_acc(BECO_ACC3, 14).i32, *multiplier++, *shift++);
}
#endif

static void beco_fully_connected_q15_impl(const int16_t *p_v,
                                          const beco_vec64_in_t *beco_m,
                                          const uint16_t dim_vec,
                                          const uint32_t beco_m_cols)
{
    uint16_t vecCnt = dim_vec >> 2;
    const uint32_t beco_m_stride = beco_m_cols - 4;
    while (vecCnt) {
        beco_write_reg(BECO_REG0, *beco_m++);
        beco_write_reg(BECO_REG1, *beco_m++);
        beco_write_reg(BECO_REG2, *beco_m++);
        beco_write_reg(BECO_REG3, *beco_m++);
        beco_mmacgr4(((beco_vec32_in_t){.i16 = {*p_v++, 0}}), BECO_REG0);
        beco_m += beco_m_stride;

        beco_write_reg(BECO_REG0, *beco_m++);
        beco_write_reg(BECO_REG1, *beco_m++);
        beco_write_reg(BECO_REG2, *beco_m++);
        beco_write_reg(BECO_REG3, *beco_m++);
        beco_mmacgr4(((beco_vec32_in_t){.i16 = {*p_v++, 0}}), BECO_REG0);
        beco_m += beco_m_stride;

        beco_write_reg(BECO_REG0, *beco_m++);
        beco_write_reg(BECO_REG1, *beco_m++);
        beco_write_reg(BECO_REG2, *beco_m++);
        beco_write_reg(BECO_REG3, *beco_m++);
        beco_mmacgr4(((beco_vec32_in_t){.i16 = {*p_v++, 0}}), BECO_REG0);
        beco_m += beco_m_stride;

        beco_write_reg(BECO_REG0, *beco_m++);
        beco_write_reg(BECO_REG1, *beco_m++);
        beco_write_reg(BECO_REG2, *beco_m++);
        beco_write_reg(BECO_REG3, *beco_m++);
        beco_mmacgr4(((beco_vec32_in_t){.i16 = {*p_v++, 0}}), BECO_REG0);
        beco_m += beco_m_stride;

        vecCnt--;
    }

    vecCnt = dim_vec & 3;
    while (vecCnt) {
        beco_write_reg(BECO_REG0, *beco_m++);
        beco_write_reg(BECO_REG1, *beco_m++);
        beco_write_reg(BECO_REG2, *beco_m++);
        beco_write_reg(BECO_REG3, *beco_m++);
        beco_mmacgr4(((beco_vec32_in_t){.i16 = {*p_v++, 0}}), BECO_REG0);
        beco_m += beco_m_stride;
        vecCnt--;
    }
}

void beco_fully_connected_pt_q15_process(const int16_t *pV,
                                         const int8_t *pM,
                                         const uint16_t dim_vec,
                                         const uint16_t num_of_cols,
                                         const int32_t multiplier,
                                         const int32_t out_shift,
                                         const int32_t *bias,
                                         int16_t *pOut,
                                         int16_t *buffer)
{
    const uint32_t beco_num_of_cols = num_of_cols >> 5;

#if defined(ARM_MATH_MVEI)
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *) buffer;
#else
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *) pOut;
#endif

    const beco_vec64_in_t *beco_w = (const beco_vec64_in_t *) pM;
    const uint32_t beco_w_stride = num_of_cols >> 3;
    const beco_vec64_in_t *beco_bias = (const beco_vec64_in_t *) bias;

    unsigned i;
    if (bias) {
        for (i = 0; i < beco_num_of_cols; i++) {
            beco_fully_connected_set_bias(beco_bias);
            beco_fully_connected_q15_impl(pV, beco_w, dim_vec, beco_w_stride);
#if defined(ARM_MATH_MVEI)
            beco_fully_connected_q15_readout32(beco_out);
            beco_fully_connected_pt_q15_requant(
                (int32_t *)beco_out, multiplier, out_shift, pOut);
            pOut += 32;
#else
            beco_fully_connected_pt_q15_readout(beco_out, multiplier, out_shift);
            beco_out += 16;
#endif
            beco_w += 4;
            beco_bias += 16;
        }
    } else {
        for (i = 0; i < beco_num_of_cols; i++) {
            beco_clear_acc(BECO_ACC0);
            beco_clear_acc(BECO_ACC1);
            beco_clear_acc(BECO_ACC2);
            beco_clear_acc(BECO_ACC3);
            beco_fully_connected_q15_impl(pV, beco_w, dim_vec, beco_w_stride);
#if defined(ARM_MATH_MVEI)
            beco_fully_connected_q15_readout32(beco_out);
            beco_fully_connected_pt_q15_requant(
                (int32_t *)beco_out, multiplier, out_shift, pOut);
            pOut += 32;
#else
            beco_fully_connected_pt_q15_readout(beco_out, multiplier, out_shift);
            beco_out += 16;
#endif
            beco_w += 4;
        }
    }
}

void beco_fully_connected_pc_q15_process(const int16_t *pV,
                                         const int8_t *pM,
                                         const uint16_t dim_vec,
                                         const uint16_t num_of_cols,
                                         const int32_t *multiplier,
                                         const int32_t *out_shift,
                                         const int32_t *bias,
                                         int16_t *pOut,
                                         int16_t *buffer)
{
    const uint32_t beco_num_of_cols = num_of_cols >> 5;

#if defined(ARM_MATH_MVEI)
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *) buffer;
#else
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *) pOut;
#endif

    const beco_vec64_in_t *beco_w = (const beco_vec64_in_t *) pM;
    const uint32_t beco_w_stride = num_of_cols >> 3;
    const beco_vec64_in_t *beco_bias = (const beco_vec64_in_t *) bias;

    unsigned i;
    if (bias) {
        for (i = 0; i < beco_num_of_cols; i++) {
            beco_fully_connected_set_bias(beco_bias);
            beco_fully_connected_q15_impl(pV, beco_w, dim_vec, beco_w_stride);
#if defined(ARM_MATH_MVEI)
            beco_fully_connected_q15_readout32(beco_out);
            beco_fully_connected_pc_q15_requant(
                (int32_t *)beco_out, multiplier, out_shift, pOut);
            pOut += 32;
#else
            beco_fully_connected_pc_q15_readout(beco_out, multiplier, out_shift);
            beco_out += 16;
#endif
            multiplier += 32;
            out_shift += 32;
            beco_w += 4;
            beco_bias += 16;
        }
    } else {
        for (i = 0; i < beco_num_of_cols; i++) {
            beco_clear_acc(BECO_ACC0);
            beco_clear_acc(BECO_ACC1);
            beco_clear_acc(BECO_ACC2);
            beco_clear_acc(BECO_ACC3);
            beco_fully_connected_q15_impl(pV, beco_w, dim_vec, beco_w_stride);
#if defined(ARM_MATH_MVEI)
            beco_fully_connected_q15_readout32(beco_out);
            beco_fully_connected_pc_q15_requant(
                (int32_t *)beco_out, multiplier, out_shift, pOut);
            pOut += 32;
#else
            beco_fully_connected_pc_q15_readout(beco_out, multiplier, out_shift);
            beco_out += 16;
#endif
            multiplier += 32;
            out_shift += 32;
            beco_w += 4;
        }
    }
}

static void beco_fully_connected_q15_shift(int i)
{
    switch (i) {
    case 1:
        beco_shift_block5(BECO_REG0, 1);
        break;
    case 2:
        beco_shift_block5(BECO_REG0, 2);
        break;
    case 3:
        beco_shift_block5(BECO_REG0, 3);
        break;
    case 4:
        beco_shift_block5(BECO_REG0, 4);
        break;
    case 5:
        beco_shift_block5(BECO_REG0, 5);
        break;
    case 6:
        beco_shift_block5(BECO_REG0, 6);
        break;
    case 7:
        beco_shift_block5(BECO_REG0, 7);
        break;
    default:
        break;
    }
}

static void beco_fully_connected_norm_q15_impl(const int16_t *p_v,
                                               const beco_vec64_in_t *beco_w,
                                               const uint16_t dim_vec,
                                               const uint32_t beco_cols,
                                               const uint32_t remainder)
{
    int shift_num, offset_num, index;
    for (int i = 0; i < dim_vec; i++) {
        shift_num = (remainder * i) >> 3;
        offset_num = (remainder * i) & 7;

        index = beco_cols * i + shift_num;
        beco_write_reg(BECO_REG0, beco_w[index]);
        beco_write_reg(BECO_REG1, beco_w[index+1]);
        beco_write_reg(BECO_REG2, beco_w[index+2]);
        beco_write_reg(BECO_REG3, beco_w[index+3]);
        if (offset_num) {
            beco_write_reg(BECO_REG4, beco_w[index+4]);
            beco_fully_connected_q15_shift(offset_num);
        }
        beco_mmacgr4(((beco_vec32_in_t){.i16 = {*p_v++, 0}}), BECO_REG0);
    }
}

static void beco_fully_connected_norm_pt_q15_process(const int16_t *pV,
                                                     const int8_t *pM,
                                                     const uint16_t dim_vec,
                                                     const uint16_t num_of_cols,
                                                     const int32_t multiplier,
                                                     const int32_t out_shift,
                                                     const int32_t *bias,
                                                     int16_t *pOut,
                                                     int16_t *buffer)
{
    const uint32_t beco_num_of_cols = num_of_cols >> 5;
    const uint32_t remainder = num_of_cols & 31;
    const uint32_t beco_cols = (num_of_cols - remainder) >> 3;

#if defined(ARM_MATH_MVEI)
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *) buffer;
#else
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *) pOut;
#endif

    const beco_vec64_in_t *beco_m = (const beco_vec64_in_t *) pM;
    const beco_vec64_in_t *beco_bias = (const beco_vec64_in_t *) bias;

    unsigned i;
    if (bias) {
        for (i = 0; i < beco_num_of_cols; i++) {
            beco_fully_connected_set_bias(beco_bias);
            beco_fully_connected_norm_q15_impl(pV, &beco_m[4*i], dim_vec, beco_cols, remainder);
#if defined(ARM_MATH_MVEI)
            beco_fully_connected_q15_readout32(beco_out);
            beco_fully_connected_pt_q15_requant(
                (int32_t *)beco_out, multiplier, out_shift, pOut);
            pOut += 32;
#else
            beco_fully_connected_pt_q15_readout(beco_out, multiplier, out_shift);
            beco_out += 16;
#endif
            beco_bias += 16;
        }
        if (remainder) {
            beco_fully_connected_set_bias(beco_bias);
            beco_fully_connected_norm_q15_impl(pV, &beco_m[beco_cols], dim_vec, beco_cols, remainder);
#if defined(ARM_MATH_MVEI)
            int16_t *beco_out_temp = (int16_t *)(beco_out + 32);
            beco_fully_connected_q15_readout32(beco_out);
            beco_fully_connected_pt_q15_requant(
                    (int32_t *)beco_out, multiplier, out_shift, beco_out_temp);
            memcpy(pOut, beco_out_temp, remainder * sizeof(int16_t));
#else
            beco_vec32_out_t *beco_out_temp = (beco_vec32_out_t *)buffer;
            beco_fully_connected_pt_q15_readout(beco_out_temp, multiplier, out_shift);
            memcpy(beco_out, beco_out_temp, remainder * sizeof(int16_t));
#endif
        }
    } else {
        for (i = 0; i < beco_num_of_cols; i++) {
            beco_clear_acc(BECO_ACC0);
            beco_clear_acc(BECO_ACC1);
            beco_clear_acc(BECO_ACC2);
            beco_clear_acc(BECO_ACC3);
            beco_fully_connected_norm_q15_impl(pV, &beco_m[4*i], dim_vec, beco_cols, remainder);
#if defined(ARM_MATH_MVEI)
        beco_fully_connected_q15_readout32(beco_out);
        beco_fully_connected_pt_q15_requant(
                (int32_t *)beco_out, multiplier, out_shift, pOut);
            pOut += 32;
#else
            beco_fully_connected_pt_q15_readout(beco_out, multiplier, out_shift);
            beco_out += 16;
#endif
        }
        if (remainder) {
            beco_clear_acc(BECO_ACC0);
            beco_clear_acc(BECO_ACC1);
            beco_clear_acc(BECO_ACC2);
            beco_clear_acc(BECO_ACC3);
            beco_fully_connected_norm_q15_impl(pV, &beco_m[beco_cols], dim_vec, beco_cols, remainder);
#if defined(ARM_MATH_MVEI)
            int16_t *beco_out_temp = (int16_t *)(beco_out + 32);
            beco_fully_connected_q15_readout32(beco_out);
            beco_fully_connected_pt_q15_requant(
                    (int32_t *)beco_out, multiplier, out_shift, beco_out_temp);
            memcpy(pOut, beco_out_temp, remainder * sizeof(int16_t));
#else
            beco_vec32_out_t *beco_out_temp = (beco_vec32_out_t *)buffer;
            beco_fully_connected_pt_q15_readout(beco_out_temp, multiplier, out_shift);
            memcpy(beco_out, beco_out_temp, remainder * sizeof(int16_t));
#endif
        }
    }
}

static void beco_fully_connected_norm_pc_q15_process(const int16_t *pV,
                                                     const int8_t *pM,
                                                     const uint16_t dim_vec,
                                                     const uint16_t num_of_cols,
                                                     const int32_t *multiplier,
                                                     const int32_t *out_shift,
                                                     const int32_t *bias,
                                                     int16_t *pOut,
                                                     int16_t *buffer)
{
    const uint32_t beco_num_of_cols = num_of_cols >> 5;
    const uint32_t remainder = num_of_cols & 31;
    const uint32_t beco_cols = (num_of_cols - remainder) >> 3;

#if defined(ARM_MATH_MVEI)
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *) buffer;
#else
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *) pOut;
#endif

    const beco_vec64_in_t *beco_m = (const beco_vec64_in_t *) pM;
    const beco_vec64_in_t *beco_bias = (const beco_vec64_in_t *) bias;

    unsigned i;
    if (bias) {
        for (i = 0; i < beco_num_of_cols; i++) {
            beco_fully_connected_set_bias(beco_bias);
            beco_fully_connected_norm_q15_impl(pV, &beco_m[4*i], dim_vec, beco_cols, remainder);
#if defined(ARM_MATH_MVEI)
            beco_fully_connected_q15_readout32(beco_out);
            beco_fully_connected_pc_q15_requant(
                (int32_t *)beco_out, multiplier, out_shift, pOut);
            pOut += 32;
#else
            beco_fully_connected_pc_q15_readout(beco_out, multiplier, out_shift);
            beco_out += 16;
#endif
            multiplier += 32;
            out_shift += 32;
            beco_bias += 16;
        }
        if (remainder) {
            beco_fully_connected_set_bias(beco_bias);
            beco_fully_connected_norm_q15_impl(pV, &beco_m[beco_cols], dim_vec, beco_cols, remainder);
#if defined(ARM_MATH_MVEI)
            int16_t *beco_out_temp = (int16_t *)(beco_out + 32);
            beco_fully_connected_q15_readout32(beco_out);
            beco_fully_connected_pc_q15_requant(
                    (int32_t *)beco_out, multiplier, out_shift, beco_out_temp);
            memcpy(pOut, beco_out_temp, remainder * sizeof(int16_t));
#else
            beco_vec32_out_t *beco_out_temp = (beco_vec32_out_t *)buffer;
            beco_fully_connected_pc_q15_readout(beco_out_temp, multiplier, out_shift);
            memcpy(beco_out, beco_out_temp, remainder * sizeof(int16_t));
#endif
        }
    } else {
        for (i = 0; i < beco_num_of_cols; i++) {
            beco_clear_acc(BECO_ACC0);
            beco_clear_acc(BECO_ACC1);
            beco_clear_acc(BECO_ACC2);
            beco_clear_acc(BECO_ACC3);
            beco_fully_connected_norm_q15_impl(pV, &beco_m[4*i], dim_vec, beco_cols, remainder);
#if defined(ARM_MATH_MVEI)
            beco_fully_connected_q15_readout32(beco_out);
            beco_fully_connected_pc_q15_requant(
                (int32_t *)beco_out, multiplier, out_shift, pOut);
            pOut += 32;
#else
            beco_fully_connected_pc_q15_readout(beco_out, multiplier, out_shift);
            beco_out += 16;
#endif
            multiplier += 32;
            out_shift += 32;
        }
        if (remainder) {
            beco_clear_acc(BECO_ACC0);
            beco_clear_acc(BECO_ACC1);
            beco_clear_acc(BECO_ACC2);
            beco_clear_acc(BECO_ACC3);
            beco_fully_connected_norm_q15_impl(pV, &beco_m[beco_cols], dim_vec, beco_cols, remainder);
#if defined(ARM_MATH_MVEI)
            int16_t *beco_out_temp = (int16_t *)(beco_out + 32);
            beco_fully_connected_q15_readout32(beco_out);
            beco_fully_connected_pc_q15_requant(
                    (int32_t *)beco_out, multiplier, out_shift, beco_out_temp);
            memcpy(pOut, beco_out_temp, remainder * sizeof(int16_t));
#else
            beco_vec32_out_t *beco_out_temp = (beco_vec32_out_t *)buffer;
            beco_fully_connected_pc_q15_readout(beco_out_temp, multiplier, out_shift);
            memcpy(beco_out, beco_out_temp, remainder * sizeof(int16_t));
#endif
        }
    }
}

void beco_fully_connected_pt_q15(const int16_t *pV,
                                 const int8_t *pM,
                                 const uint16_t dim_vec,
                                 const uint16_t num_of_cols,
                                 const int32_t multiplier,
                                 const int32_t out_shift,
                                 const int32_t *bias,
                                 int16_t *pOut,
                                 int16_t *buffer)
{
    uint32_t config =
        BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 |
        BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT8 |
        BECO_CONF_RSHIFT(0) |
        BECO_CONF_PACK_INT32 | BECO_CONF_RD_16x8_ROT90;
    beco_write_config(config);

    if (num_of_cols % 32 == 0) {
        beco_fully_connected_pt_q15_process(pV, pM, dim_vec, num_of_cols,
                                            multiplier, out_shift, bias, pOut, buffer);
    } else {
        beco_fully_connected_norm_pt_q15_process(pV, pM, dim_vec, num_of_cols,
                                                 multiplier, out_shift, bias, pOut, buffer);
    }
}

void beco_fully_connected_pc_q15(const int16_t *pV,
                                 const int8_t *pM,
                                 const uint16_t dim_vec,
                                 const uint16_t num_of_cols,
                                 const int32_t *multiplier,
                                 const int32_t *out_shift,
                                 const int32_t *bias,
                                 int16_t *pOut,
                                 int16_t *buffer)
{
    uint32_t config =
        BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 |
        BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT8 |
        BECO_CONF_RSHIFT(0) |
        BECO_CONF_PACK_INT32 | BECO_CONF_RD_16x8_ROT90;
    beco_write_config(config);

    if (num_of_cols % 32 == 0) {
        beco_fully_connected_pc_q15_process(pV, pM, dim_vec, num_of_cols,
                                            multiplier, out_shift, bias, pOut, buffer);
    } else {
        beco_fully_connected_norm_pc_q15_process(pV, pM, dim_vec, num_of_cols,
                                                 multiplier, out_shift, bias, pOut, buffer);
    }
}


int32_t beco_gru_s16_get_buffer_size(const uint16_t num_outputs)
{
    int32_t buffer_len = 6 * num_outputs * sizeof(int16_t);
#if defined(ARM_MATH_MVEI)
    buffer_len += 32 * sizeof(int32_t);
#endif
    if ((3 * num_outputs) % 32 != 0) {
        buffer_len += 32 * sizeof(int16_t);
    }

    return buffer_len;
}

int32_t beco_lstm_s16_get_buffer_size(const uint16_t num_outputs)
{
    int32_t buffer_len = 8 * num_outputs * sizeof(int16_t);
#if defined(ARM_MATH_MVEI)
    buffer_len += 32 * sizeof(int32_t);
#endif
    if ((4 * num_outputs) % 32 != 0) {
        buffer_len += 32 * sizeof(int16_t);
    }

    return buffer_len;
}

int32_t beco_bi_gru_s16_get_buffer_size(const uint16_t num_times,
                                        const uint16_t num_batches,
                                        const uint16_t num_outputs)
{
    int32_t buffer_len = 6 * num_outputs * sizeof(int16_t);
#if defined(ARM_MATH_MVEI)
    buffer_len += 32 * sizeof(int32_t);
#endif
    if ((3 * num_outputs) % 32 != 0) {
        buffer_len += 32 * sizeof(int16_t);
    }
    buffer_len += num_times * num_batches * 2 * num_outputs * sizeof(int16_t);

    return buffer_len;
}

int32_t beco_bi_lstm_s16_get_buffer_size(const uint16_t num_times,
                                         const uint16_t num_batches,
                                         const uint16_t num_outputs)
{
    int32_t buffer_len = 8 * num_outputs * sizeof(int16_t);
#if defined(ARM_MATH_MVEI)
    buffer_len += 32 * sizeof(int32_t);
#endif
    if ((4 * num_outputs) % 32 != 0) {
        buffer_len += 32 * sizeof(int16_t);
    }
    buffer_len += num_times * num_batches * 2 * num_outputs * sizeof(int16_t);

    return buffer_len;
}

