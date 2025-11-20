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

static void beco_fully_connected_q15_impl(const int16_t *p_v,
                                          const beco_vec64_in_t *beco_w,
                                          const uint16_t dim_vec,
                                          const uint32_t beco_cols,
                                          const uint32_t remainder)
{
    int shift_num, offset_num, index;
    for (int i = 0; i < dim_vec; i++) {
        shift_num = (remainder*i) >> 3;
        offset_num = (remainder*i) & 7;

        index = beco_cols*i + shift_num;
        if (offset_num == 0) {
            beco_write_reg(BECO_REG0, beco_w[index]);
            beco_write_reg(BECO_REG1, beco_w[index+1]);
            beco_write_reg(BECO_REG2, beco_w[index+2]);
            beco_write_reg(BECO_REG3, beco_w[index+3]);
            beco_mmacgr4(((beco_vec32_in_t){.i16 = {*p_v++, 0}}), BECO_REG0);
        } else {
            beco_write_reg(BECO_REG0, beco_w[index]);
            beco_write_reg(BECO_REG1, beco_w[index+1]);
            beco_write_reg(BECO_REG2, beco_w[index+2]);
            beco_write_reg(BECO_REG3, beco_w[index+3]);
            beco_write_reg(BECO_REG4, beco_w[index+4]);
            beco_fully_connected_q15_shift(offset_num);
            beco_mmacgr4(((beco_vec32_in_t){.i16 = {*p_v++, 0}}), BECO_REG0);
        }
    }
}

static void beco_fully_connected_norm_q15_process(const int16_t *pV,
                                                  const int8_t *pM,
                                                  const uint16_t dim_vec,
                                                  const uint16_t num_of_cols,
                                                  const int32_t *multiplier,
                                                  const int32_t *out_shift,
                                                  const int32_t *bias,
                                                  int16_t * pOut,
                                                  const int16_t min,
                                                  const int16_t max)
{
    const int beco_num_of_cols = num_of_cols >> 5;
    const uint32_t remainder = num_of_cols & 31;
    const uint32_t beco_cols = (num_of_cols - remainder) >> 3;

    beco_vec32_out_t *beco_out = (beco_vec32_out_t *) pOut;

    const beco_vec64_in_t *beco_m = (const beco_vec64_in_t *) pM;
    const beco_vec64_in_t *beco_bias = (const beco_vec64_in_t *) bias;

    for (int i = 0; i < beco_num_of_cols; i++) {
        if (bias) {
            beco_fully_connected_set_bias(beco_bias);
            beco_bias += 16;
        } else {
            beco_clear_acc(BECO_ACC0);
            beco_clear_acc(BECO_ACC1);
            beco_clear_acc(BECO_ACC2);
            beco_clear_acc(BECO_ACC3);
        }

        beco_fully_connected_q15_impl(pV, &beco_m[4*i], dim_vec, beco_cols, remainder);
        beco_fully_connected_q15_readout_pc(beco_out, multiplier, out_shift, min, max);
        beco_out += 16;
        multiplier += 32;
        out_shift += 32;
    }

    if (remainder) {
        if (bias) {
            beco_fully_connected_set_bias(beco_bias);
        } else {
            beco_clear_acc(BECO_ACC0);
            beco_clear_acc(BECO_ACC1);
            beco_clear_acc(BECO_ACC2);
            beco_clear_acc(BECO_ACC3);
        }

        beco_fully_connected_q15_impl(pV, &beco_m[beco_cols], dim_vec, beco_cols, remainder);
        beco_vec32_out_t beco_out_temp[16];
        beco_fully_connected_q15_readout_pc(beco_out_temp, multiplier, out_shift, min, max);
        memcpy(beco_out, beco_out_temp, remainder * sizeof(int16_t));
    }
}

void beco_fully_connected_norm_q15_pc(const int16_t *pV,
                                      const int8_t *pM,
                                      const uint16_t dim_vec,
                                      const uint16_t num_of_cols,
                                      const int32_t *multiplier,
                                      const int32_t *out_shift,
                                      const int32_t *bias,
                                      int16_t *pOut,
                                      const int16_t min,
                                      const int16_t max)
{
    uint32_t config =
        BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 |
        BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT8 |
        BECO_CONF_RSHIFT(0) |
        BECO_CONF_PACK_INT32 | BECO_CONF_RD_16x8_ROT90;
    beco_write_config(config);

    beco_fully_connected_norm_q15_process(pV, pM, dim_vec, num_of_cols,
                                          multiplier, out_shift, bias, pOut, min, max);
}

