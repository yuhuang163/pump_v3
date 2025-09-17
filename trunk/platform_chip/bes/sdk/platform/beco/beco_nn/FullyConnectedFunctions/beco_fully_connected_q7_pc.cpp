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

static void beco_fully_connected_q7_impl(const int8_t *p_v,
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

static void beco_fully_connected_q7_process(const int8_t *pV,
                                            const int8_t *pM,
                                            const uint16_t dim_vec,
                                            const uint16_t num_of_cols,
                                            const int32_t *multiplier,
                                            const int32_t *out_shift,
                                            const int32_t *bias,
                                            int8_t * pOut,
                                            const int8_t out_offset,
                                            const int8_t min,
                                            const int8_t max)
{
    const int beco_num_of_cols = num_of_cols >> 5;

    beco_vec32_out_t *beco_out = (beco_vec32_out_t *) pOut;

    const beco_vec64_in_t *beco_w = (const beco_vec64_in_t *) pM;
    const uint32_t beco_w_stride = num_of_cols >> 3;
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

        beco_fully_connected_q7_impl(pV, beco_w, dim_vec, beco_w_stride);
        beco_fully_connected_q7_readout_pc(beco_out, multiplier, out_shift, out_offset, min, max);
        beco_out += 8;
        beco_w += 4;
        multiplier += 32;
        out_shift += 32;
    }
}

void beco_fully_connected_q7_pc(const int8_t *pV,
                                const int8_t *pM,
                                const uint16_t dim_vec,
                                const uint16_t num_of_cols,
                                const int32_t *multiplier,
                                const int32_t *out_shift,
                                const int32_t *bias,
                                int8_t *pOut,
                                const int8_t out_offset,
                                const int8_t min,
                                const int8_t max)
{
    uint32_t config =
        BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 |
        BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT8 |
        BECO_CONF_RSHIFT(0) |
        BECO_CONF_PACK_INT32 | BECO_CONF_RD_16x8_ROT90;
    beco_write_config(config);

    beco_fully_connected_q7_process(pV, pM, dim_vec, num_of_cols,
                                    multiplier, out_shift, bias, pOut,
                                    out_offset, min, max);
}

