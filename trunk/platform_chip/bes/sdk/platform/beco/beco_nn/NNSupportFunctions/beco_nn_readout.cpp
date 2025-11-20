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

#include "beco_nnsupportfunctions.h"

void beco_fully_connected_q7_readout_pc(beco_vec32_out_t *out,
                                        const int32_t *multiplier,
                                        const int32_t *shift,
                                        const int8_t offset,
                                        const int8_t min,
                                        const int8_t max)
{
    out[0].i8[0] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 0).i32, *multiplier++, *shift++, offset, min, max);
    out[0].i8[1] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 2).i32, *multiplier++, *shift++, offset, min, max);
    out[0].i8[2] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 8).i32, *multiplier++, *shift++, offset, min, max);
    out[0].i8[3] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 10).i32, *multiplier++, *shift++, offset, min, max);
    out[1].i8[0] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 4).i32, *multiplier++, *shift++, offset, min, max);
    out[1].i8[1] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 6).i32, *multiplier++, *shift++, offset, min, max);
    out[1].i8[2] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 12).i32, *multiplier++, *shift++, offset, min, max);
    out[1].i8[3] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 14).i32, *multiplier++, *shift++, offset, min, max);

    out[2].i8[0] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 0).i32, *multiplier++, *shift++, offset, min, max);
    out[2].i8[1] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 2).i32, *multiplier++, *shift++, offset, min, max);
    out[2].i8[2] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 8).i32, *multiplier++, *shift++, offset, min, max);
    out[2].i8[3] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 10).i32, *multiplier++, *shift++, offset, min, max);
    out[3].i8[0] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 4).i32, *multiplier++, *shift++, offset, min, max);
    out[3].i8[1] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 6).i32, *multiplier++, *shift++, offset, min, max);
    out[3].i8[2] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 12).i32, *multiplier++, *shift++, offset, min, max);
    out[3].i8[3] = beco_requantize_q7(beco_read_acc(BECO_ACC1, 14).i32, *multiplier++, *shift++, offset, min, max);

    out[4].i8[0] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 0).i32, *multiplier++, *shift++, offset, min, max);
    out[4].i8[1] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 2).i32, *multiplier++, *shift++, offset, min, max);
    out[4].i8[2] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 8).i32, *multiplier++, *shift++, offset, min, max);
    out[4].i8[3] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 10).i32, *multiplier++, *shift++, offset, min, max);
    out[5].i8[0] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 4).i32, *multiplier++, *shift++, offset, min, max);
    out[5].i8[1] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 6).i32, *multiplier++, *shift++, offset, min, max);
    out[5].i8[2] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 12).i32, *multiplier++, *shift++, offset, min, max);
    out[5].i8[3] = beco_requantize_q7(beco_read_acc(BECO_ACC2, 14).i32, *multiplier++, *shift++, offset, min, max);

    out[6].i8[0] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 0).i32, *multiplier++, *shift++, offset, min, max);
    out[6].i8[1] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 2).i32, *multiplier++, *shift++, offset, min, max);
    out[6].i8[2] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 8).i32, *multiplier++, *shift++, offset, min, max);
    out[6].i8[3] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 10).i32, *multiplier++, *shift++, offset, min, max);
    out[7].i8[0] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 4).i32, *multiplier++, *shift++, offset, min, max);
    out[7].i8[1] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 6).i32, *multiplier++, *shift++, offset, min, max);
    out[7].i8[2] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 12).i32, *multiplier++, *shift++, offset, min, max);
    out[7].i8[3] = beco_requantize_q7(beco_read_acc(BECO_ACC3, 14).i32, *multiplier++, *shift++, offset, min, max);
}

void beco_fully_connected_q7_readout(beco_vec32_out_t *out,
                                     const int32_t multiplier,
                                     const int32_t shift,
                                     const int8_t offset,
                                     const int8_t min,
                                     const int8_t max)
{
    beco_read_acc(BECO_ACC0, -2);
    const uint8_t index1[] = {0, 0, 1, 1, 0, 0, 1, 1};
    const uint8_t index2[] = {0, 1, 0, 1, 2, 3, 2, 3};
    for (int i = 0; i < 8; i++) {
        out[index1[i]].i8[index2[i]] = beco_requantize_q7(beco_read_next_acc(BECO_ACC0, 2).i32, multiplier, shift, offset, min, max);
        out[index1[i] + 2].i8[index2[i]] = beco_requantize_q7(beco_read_next_acc(BECO_ACC1, 0).i32, multiplier, shift, offset, min, max);
        out[index1[i] + 4].i8[index2[i]] = beco_requantize_q7(beco_read_next_acc(BECO_ACC2, 0).i32, multiplier, shift, offset, min, max);
        out[index1[i] + 6].i8[index2[i]] = beco_requantize_q7(beco_read_next_acc(BECO_ACC3, 0).i32, multiplier, shift, offset, min, max);
    }
}

void beco_fully_connected_q15_readout_pc(beco_vec32_out_t *out,
                                         const int32_t *multiplier,
                                         const int32_t *shift,
                                         const int16_t min,
                                         const int16_t max)
{
    out[0].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC0, 0).i32, *multiplier++, *shift++, min, max);
    out[0].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC0, 2).i32, *multiplier++, *shift++, min, max);
    out[1].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC0, 8).i32, *multiplier++, *shift++, min, max);
    out[1].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC0, 10).i32, *multiplier++, *shift++, min, max);
    out[2].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC0, 4).i32, *multiplier++, *shift++, min, max);
    out[2].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC0, 6).i32, *multiplier++, *shift++, min, max);
    out[3].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC0, 12).i32, *multiplier++, *shift++, min, max);
    out[3].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC0, 14).i32, *multiplier++, *shift++, min, max);

    out[4].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC1, 0).i32, *multiplier++, *shift++, min, max);
    out[4].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC1, 2).i32, *multiplier++, *shift++, min, max);
    out[5].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC1, 8).i32, *multiplier++, *shift++, min, max);
    out[5].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC1, 10).i32, *multiplier++, *shift++, min, max);
    out[6].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC1, 4).i32, *multiplier++, *shift++, min, max);
    out[6].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC1, 6).i32, *multiplier++, *shift++, min, max);
    out[7].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC1, 12).i32, *multiplier++, *shift++, min, max);
    out[7].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC1, 14).i32, *multiplier++, *shift++, min, max);

    out[8].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC2, 0).i32, *multiplier++, *shift++, min, max);
    out[8].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC2, 2).i32, *multiplier++, *shift++, min, max);
    out[9].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC2, 8).i32, *multiplier++, *shift++, min, max);
    out[9].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC2, 10).i32, *multiplier++, *shift++, min, max);
    out[10].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC2, 4).i32, *multiplier++, *shift++, min, max);
    out[10].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC2, 6).i32, *multiplier++, *shift++, min, max);
    out[11].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC2, 12).i32, *multiplier++, *shift++, min, max);
    out[11].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC2, 14).i32, *multiplier++, *shift++, min, max);

    out[12].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC3, 0).i32, *multiplier++, *shift++, min, max);
    out[12].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC3, 2).i32, *multiplier++, *shift++, min, max);
    out[13].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC3, 8).i32, *multiplier++, *shift++, min, max);
    out[13].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC3, 10).i32, *multiplier++, *shift++, min, max);
    out[14].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC3, 4).i32, *multiplier++, *shift++, min, max);
    out[14].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC3, 6).i32, *multiplier++, *shift++, min, max);
    out[15].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC3, 12).i32, *multiplier++, *shift++, min, max);
    out[15].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC3, 14).i32, *multiplier++, *shift++, min, max);
}

void beco_fully_connected_q15_readout(beco_vec32_out_t *out,
                                      const int32_t multiplier,
                                      const int32_t shift,
                                      const int16_t min,
                                      const int16_t max)
{
    beco_read_acc(BECO_ACC0, -2);
    const uint8_t index1[] = {0, 0, 2, 2, 1, 1, 3, 3};
    const uint8_t index2[] = {0, 1, 0, 1, 0, 1, 0, 1};
    for (int i = 0; i < 8; i++) {
        out[index1[i]].i16[index2[i]] = beco_requantize_q15(beco_read_next_acc(BECO_ACC0, 2).i32, multiplier, shift, min, max);
        out[index1[i] + 4].i16[index2[i]] = beco_requantize_q15(beco_read_next_acc(BECO_ACC1, 0).i32, multiplier, shift, min, max);
        out[index1[i] + 8].i16[index2[i]] = beco_requantize_q15(beco_read_next_acc(BECO_ACC2, 0).i32, multiplier, shift, min, max);
        out[index1[i] + 12].i16[index2[i]] = beco_requantize_q15(beco_read_next_acc(BECO_ACC3, 0).i32, multiplier, shift, min, max);
    }
}


void beco_convolve_q7_readout(beco_vec32_out_t *out,
                              uint32_t stride,
                              const int32_t *multiplier,
                              const int32_t *shift,
                              const int8_t offset,
                              const int8_t min,
                              const int8_t max)
{
    beco_vec32_out_t *out1 = out + 4*stride;
    const int32_t *multiplier1 = multiplier + 4;
    const int32_t *shift1 = shift + 4;
    beco_read_acc(BECO_ACC0, -1);
    for (int i = 0; i < 4; i++) {
        out[0].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC0, 1).i32, *multiplier, *shift, offset, min, max);
        out1[0].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC1, 0).i32, *multiplier1, *shift1, offset, min, max);
        out[1].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC2, 0).i32, *multiplier, *shift, offset, min, max);
        out1[1].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC3, 0).i32, *multiplier1, *shift1, offset, min, max);
    }
    out += stride;
    out1 += stride;
    multiplier++;
    multiplier1++;
    shift++;
    shift1++;
    for (int i = 0; i < 4; i++) {
        out[0].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC0, 1).i32, *multiplier, *shift, offset, min, max);
        out1[0].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC1, 0).i32, *multiplier1, *shift1, offset, min, max);
        out[1].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC2, 0).i32, *multiplier, *shift, offset, min, max);
        out1[1].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC3, 0).i32, *multiplier1, *shift1, offset, min, max);
    }
    out += stride;
    out1 += stride;
    multiplier++;
    multiplier1++;
    shift++;
    shift1++;
    for (int i = 0; i < 4; i++) {
        out[0].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC0, 1).i32, *multiplier, *shift, offset, min, max);
        out1[0].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC1, 0).i32, *multiplier1, *shift1, offset, min, max);
        out[1].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC2, 0).i32, *multiplier, *shift, offset, min, max);
        out1[1].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC3, 0).i32, *multiplier1, *shift1, offset, min, max);
    }
    out += stride;
    out1 += stride;
    multiplier++;
    multiplier1++;
    shift++;
    shift1++;
    for (int i = 0; i < 4; i++) {
        out[0].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC0, 1).i32, *multiplier, *shift, offset, min, max);
        out1[0].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC1, 0).i32, *multiplier1, *shift1, offset, min, max);
        out[1].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC2, 0).i32, *multiplier, *shift, offset, min, max);
        out1[1].i8[i] = beco_requantize_q7(beco_read_next_acc(BECO_ACC3, 0).i32, *multiplier1, *shift1, offset, min, max);
    }
}

void beco_convolve_q15_readout(beco_vec32_out_t *out,
                               uint32_t stride,
                               const int32_t *multiplier,
                               const int32_t *shift,
                               const int16_t min,
                               const int16_t max)
{
    beco_vec32_out_t *out1 = out + 4*stride;
    const int32_t *multiplier1 = multiplier + 4;
    const int32_t *shift1 = shift + 4;
    beco_read_acc(BECO_ACC0, -2);
    for (int i = 0; i < 2; i++) {
        out[0].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC0, 2).i32, *multiplier, *shift, min, max);
        out1[0].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC1, 0).i32, *multiplier1, *shift1, min, max);
        out[1].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC2, 0).i32, *multiplier, *shift, min, max);
        out1[1].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC3, 0).i32, *multiplier1, *shift1, min, max);
    }
    out += stride;
    out1 += stride;
    multiplier++;
    multiplier1++;
    shift++;
    shift1++;
    for (int i = 0; i < 2; i++) {
        out[0].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC0, 2).i32, *multiplier, *shift, min, max);
        out1[0].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC1, 0).i32, *multiplier1, *shift1, min, max);
        out[1].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC2, 0).i32, *multiplier, *shift, min, max);
        out1[1].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC3, 0).i32, *multiplier1, *shift1, min, max);
    }
    out += stride;
    out1 += stride;
    multiplier++;
    multiplier1++;
    shift++;
    shift1++;
    for (int i = 0; i < 2; i++) {
        out[0].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC0, 2).i32, *multiplier, *shift, min, max);
        out1[0].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC1, 0).i32, *multiplier1, *shift1, min, max);
        out[1].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC2, 0).i32, *multiplier, *shift, min, max);
        out1[1].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC3, 0).i32, *multiplier1, *shift1, min, max);
    }
    out += stride;
    out1 += stride;
    multiplier++;
    multiplier1++;
    shift++;
    shift1++;
    for (int i = 0; i < 2; i++) {
        out[0].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC0, 2).i32, *multiplier, *shift, min, max);
        out1[0].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC1, 0).i32, *multiplier1, *shift1, min, max);
        out[1].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC2, 0).i32, *multiplier, *shift, min, max);
        out1[1].i16[i] = beco_requantize_q15(beco_read_next_acc(BECO_ACC3, 0).i32, *multiplier1, *shift1, min, max);
    }
}


void beco_depthwise_conv_q7_readoutACC0(beco_vec32_out_t *out,
                                        const int32_t multiplier,
                                        const int32_t shift,
                                        const int8_t offset,
                                        const int8_t min,
                                        const int8_t max)
{
    out[0].i8[0] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 0).i32, multiplier, shift, offset, min, max);
    out[0].i8[1] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 4).i32, multiplier, shift, offset, min, max);
    out[0].i8[2] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 2).i32, multiplier, shift, offset, min, max);
    out[0].i8[3] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 6).i32, multiplier, shift, offset, min, max);
    out[1].i8[0] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 8).i32, multiplier, shift, offset, min, max);
    out[1].i8[1] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 12).i32, multiplier, shift, offset, min, max);
    out[1].i8[2] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 10).i32, multiplier, shift, offset, min, max);
    out[1].i8[3] = beco_requantize_q7(beco_read_acc(BECO_ACC0, 14).i32, multiplier, shift, offset, min, max);
}

void beco_depthwise_conv_q7_readout4ACC(beco_vec32_out_t *out,
                                        const int32_t multiplier,
                                        const int32_t shift,
                                        const int8_t offset,
                                        const int8_t min,
                                        const int8_t max)
{
    beco_read_acc(BECO_ACC0, -2);
    const uint8_t index1[] = {0, 0, 0, 0, 1, 1, 1, 1};
    const uint8_t index2[] = {0, 2, 1, 3, 0, 2, 1, 3};
    for (int i = 0; i < 8; i++) {
        out[index1[i]].i8[index2[i]] = beco_requantize_q7(beco_read_next_acc(BECO_ACC0, 2).i32, multiplier, shift, offset, min, max);
        out[index1[i] + 2].i8[index2[i]] = beco_requantize_q7(beco_read_next_acc(BECO_ACC1, 0).i32, multiplier, shift, offset, min, max);
        out[index1[i] + 4].i8[index2[i]] = beco_requantize_q7(beco_read_next_acc(BECO_ACC2, 0).i32, multiplier, shift, offset, min, max);
        out[index1[i] + 6].i8[index2[i]] = beco_requantize_q7(beco_read_next_acc(BECO_ACC3, 0).i32, multiplier, shift, offset, min, max);
    }
}

void beco_depthwise_conv_q15_readoutACC0(beco_vec32_out_t *out,
                                         const int32_t multiplier,
                                         const int32_t shift,
                                         const int16_t min,
                                         const int16_t max)
{
    out[0].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC0, 0).i32, multiplier, shift, min, max);
    out[0].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC0, 4).i32, multiplier, shift, min, max);
    out[1].i16[0] = beco_requantize_q15(beco_read_acc(BECO_ACC0, 8).i32, multiplier, shift, min, max);
    out[1].i16[1] = beco_requantize_q15(beco_read_acc(BECO_ACC0, 12).i32, multiplier, shift, min, max);
}

void beco_depthwise_conv_q15_readout4ACC(beco_vec32_out_t *out,
                                         const int32_t multiplier,
                                         const int32_t shift,
                                         const int16_t min,
                                         const int16_t max)
{
    beco_read_acc(BECO_ACC0, -4);
    const uint8_t index1[] = {0, 0, 1, 1};
    const uint8_t index2[] = {0, 1, 0, 1};
    for (int i = 0; i < 4; i++) {
        out[index1[i]].i16[index2[i]] = beco_requantize_q15(beco_read_next_acc(BECO_ACC0, 4).i32, multiplier, shift, min, max);
        out[index1[i] + 2].i16[index2[i]] = beco_requantize_q15(beco_read_next_acc(BECO_ACC1, 0).i32, multiplier, shift, min, max);
        out[index1[i] + 4].i16[index2[i]] = beco_requantize_q15(beco_read_next_acc(BECO_ACC2, 0).i32, multiplier, shift, min, max);
        out[index1[i] + 6].i16[index2[i]] = beco_requantize_q15(beco_read_next_acc(BECO_ACC3, 0).i32, multiplier, shift, min, max);
    }
}


#if defined(ARM_MATH_MVEI)

void beco_fully_connected_readout32(beco_vec32_out_t *out)
{
    beco_read_acc(BECO_ACC0, -2);
    for (int i = 0; i < 8; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 2);
        out[i + 8] = beco_read_next_acc(BECO_ACC1, 0);
        out[i + 16] = beco_read_next_acc(BECO_ACC2, 0);
        out[i + 24] = beco_read_next_acc(BECO_ACC3, 0);
    }
}

void beco_fully_connected_q7_requant(int32_t *val,
                                     const int32_t multiplier,
                                     const int32_t shift,
                                     const int8_t offset,
                                     const int8_t min,
                                     const int8_t max,
                                     int8_t *output)
{
    for (int i = 0; i < 4; i++) {
        int32x4_t acc1 = {val[0], val[1], val[4], val[5]};
        int32x4_t acc2 = {val[2], val[3], val[6], val[7]};

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

void beco_fully_connected_q15_requant(int32_t *val,
                                      const int32_t multiplier,
                                      const int32_t shift,
                                      const int16_t min,
                                      const int16_t max,
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

        acc1 = vmaxq_s32(acc1, vdupq_n_s32(min));
        acc1 = vminq_s32(acc1, vdupq_n_s32(max));
        acc2 = vmaxq_s32(acc2, vdupq_n_s32(min));
        acc2 = vminq_s32(acc2, vdupq_n_s32(max));

        vstrhq_s32(output, acc1);
        output += 4;
        vstrhq_s32(output, acc2);
        output += 4;
        val += 8;
    }
}


void beco_convolve_q7_readout32(beco_vec32_out_t *out)
{
    beco_vec32_out_t *out1 = out + 32;
    beco_read_acc(BECO_ACC0, -1);
    for (int i = 0; i < 4; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 1);
        out1[i] = beco_read_next_acc(BECO_ACC1, 0);
        out[i + 4] = beco_read_next_acc(BECO_ACC2, 0);
        out1[i + 4] = beco_read_next_acc(BECO_ACC3, 0);
    }
    out += 8;
    out1 += 8;
    for (int i = 0; i < 4; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 1);
        out1[i] = beco_read_next_acc(BECO_ACC1, 0);
        out[i + 4] = beco_read_next_acc(BECO_ACC2, 0);
        out1[i + 4] = beco_read_next_acc(BECO_ACC3, 0);
    }
    out += 8;
    out1 += 8;
    for (int i = 0; i < 4; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 1);
        out1[i] = beco_read_next_acc(BECO_ACC1, 0);
        out[i + 4] = beco_read_next_acc(BECO_ACC2, 0);
        out1[i + 4] = beco_read_next_acc(BECO_ACC3, 0);
    }
    out += 8;
    out1 += 8;
    for (int i = 0; i < 4; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 1);
        out1[i] = beco_read_next_acc(BECO_ACC1, 0);
        out[i + 4] = beco_read_next_acc(BECO_ACC2, 0);
        out1[i + 4] = beco_read_next_acc(BECO_ACC3, 0);
    }
}

void beco_convolve_q7_requant(int32_t *val,
                              const int32_t *multiplier,
                              const int32_t *shift,
                              const int8_t offset,
                              const int8_t min,
                              const int8_t max,
                              const uint16_t stride,
                              int8_t *output)
{
    for (int i = 0; i < 8; i++) {
        int32x4_t acc1 = {*val++, *val++, *val++, *val++};
        int32x4_t acc2 = {*val++, *val++, *val++, *val++};

        acc1 = arm_requantize_mve(acc1, multiplier[i], shift[i]);
        acc2 = arm_requantize_mve(acc2, multiplier[i], shift[i]);

        acc1 = vaddq_s32(acc1, vdupq_n_s32(offset));
        acc1 = vmaxq_s32(acc1, vdupq_n_s32(min));
        acc1 = vminq_s32(acc1, vdupq_n_s32(max));
        acc2 = vaddq_s32(acc2, vdupq_n_s32(offset));
        acc2 = vmaxq_s32(acc2, vdupq_n_s32(min));
        acc2 = vminq_s32(acc2, vdupq_n_s32(max));

        vstrbq_s32(&output[i * stride], acc1);
        vstrbq_s32(&output[i * stride + 4], acc2);
    }
}

void beco_convolve_q15_readout32(beco_vec32_out_t *out)
{
    beco_vec32_out_t *out1 = out + 16;
    beco_read_acc(BECO_ACC0, -2);
    for (int i = 0; i < 2; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 2);
        out1[i] = beco_read_next_acc(BECO_ACC1, 0);
        out[i + 2] = beco_read_next_acc(BECO_ACC2, 0);
        out1[i + 2] = beco_read_next_acc(BECO_ACC3, 0);
    }
    out += 4;
    out1 += 4;
    for (int i = 0; i < 2; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 2);
        out1[i] = beco_read_next_acc(BECO_ACC1, 0);
        out[i + 2] = beco_read_next_acc(BECO_ACC2, 0);
        out1[i + 2] = beco_read_next_acc(BECO_ACC3, 0);
    }
    out += 4;
    out1 += 4;
    for (int i = 0; i < 2; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 2);
        out1[i] = beco_read_next_acc(BECO_ACC1, 0);
        out[i + 2] = beco_read_next_acc(BECO_ACC2, 0);
        out1[i + 2] = beco_read_next_acc(BECO_ACC3, 0);
    }
    out += 4;
    out1 += 4;
    for (int i = 0; i < 2; i++) {
        out[i] = beco_read_next_acc(BECO_ACC0, 2);
        out1[i] = beco_read_next_acc(BECO_ACC1, 0);
        out[i + 2] = beco_read_next_acc(BECO_ACC2, 0);
        out1[i + 2] = beco_read_next_acc(BECO_ACC3, 0);
    }
}

void beco_convolve_q15_requant(int32_t *val,
                               const int32_t *multiplier,
                               const int32_t *shift,
                               const int16_t min,
                               const int16_t max,
                               const uint16_t stride,
                               int16_t *output)
{
    for (int i = 0; i < 8; i++) {
        int32x4_t right_shift = vminq_s32(vdupq_n_s32(-1), vdupq_n_s32(shift[i]));
        int32x4_t left_shift = vqsubq_s32(vdupq_n_s32(shift[i]), right_shift);
        int32x4_t acc = {*val++, *val++, *val++, *val++};

        acc = vqdmulhq_n_s32(vshlq_s32(acc, left_shift), multiplier[i]);
        acc = vrshlq_s32(acc, right_shift);

        acc = vmaxq_s32(acc, vdupq_n_s32(min));
        acc = vminq_s32(acc, vdupq_n_s32(max));
        vstrhq_s32(&output[i*stride], acc);
    }
}

#endif
