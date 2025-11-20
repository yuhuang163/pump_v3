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

#include "beco_test.h"

#include "stdio.h"
#include "hal_trace.h"

static void beco_shift_block5_test()
{
    q15_t data_q15[20];
    beco_vec64_in_t *beco_in = (beco_vec64_in_t *)data_q15;
    for (int i = 0; i < 20; i++) { data_q15[i] = i - 10; }

    beco_write_reg(BECO_REG0, beco_in[0]);
    beco_write_reg(BECO_REG1, beco_in[1]);
    beco_write_reg(BECO_REG2, beco_in[2]);
    beco_write_reg(BECO_REG3, beco_in[3]);
    beco_write_reg(BECO_REG4, beco_in[4]);

    q15_t out_q15[20];
    beco_vec64_out_t *beco_out = (beco_vec64_out_t *)out_q15;

    beco_out[0] = beco_read_reg(BECO_REG0);
    beco_out[1] = beco_read_reg(BECO_REG1);
    beco_out[2] = beco_read_reg(BECO_REG2);
    beco_out[3] = beco_read_reg(BECO_REG3);
    beco_out[4] = beco_read_reg(BECO_REG4);

    TRACE(3, "===== befor shift =====");
    TRACE(3, "BECO_REG0 : %d %d %d %d", out_q15[0], out_q15[1], out_q15[2], out_q15[3]);
    TRACE(3, "BECO_REG1 : %d %d %d %d", out_q15[4], out_q15[5], out_q15[6], out_q15[7]);
    TRACE(3, "BECO_REG2 : %d %d %d %d", out_q15[8], out_q15[9], out_q15[10], out_q15[11]);
    TRACE(3, "BECO_REG3 : %d %d %d %d", out_q15[12], out_q15[13], out_q15[14], out_q15[15]);
    TRACE(3, "BECO_REG4 : %d %d %d %d", out_q15[16], out_q15[17], out_q15[18], out_q15[19]);

#define PIXSIZE 2*sizeof(q15_t)
    beco_shift_block5(BECO_REG0, PIXSIZE);

    beco_out[0] = beco_read_reg(BECO_REG0);
    beco_out[1] = beco_read_reg(BECO_REG1);
    beco_out[2] = beco_read_reg(BECO_REG2);
    beco_out[3] = beco_read_reg(BECO_REG3);
    beco_out[4] = beco_read_reg(BECO_REG4);

    TRACE(3, "===== after shift =====");
    TRACE(3, "BECO_REG0 : %d %d %d %d", out_q15[0], out_q15[1], out_q15[2], out_q15[3]);
    TRACE(3, "BECO_REG1 : %d %d %d %d", out_q15[4], out_q15[5], out_q15[6], out_q15[7]);
    TRACE(3, "BECO_REG2 : %d %d %d %d", out_q15[8], out_q15[9], out_q15[10], out_q15[11]);
    TRACE(3, "BECO_REG3 : %d %d %d %d", out_q15[12], out_q15[13], out_q15[14], out_q15[15]);
    TRACE(3, "BECO_REG4 : %d %d %d %d", out_q15[16], out_q15[17], out_q15[18], out_q15[19]);
}

static void beco_set_add_bias_test()
{
    uint32_t beco_cfg = BECO_CONF_AMODE_REP32    | BECO_CONF_BMODE_REP32  |
                        BECO_CONF_ATYPE_INT16    | BECO_CONF_BTYPE_INT16  |
                        BECO_CONF_RSHIFT(0)      | BECO_CONF_RD_16x16 |
                        BECO_CONF_PACK_INT32;
    beco_write_config(beco_cfg);

    int32_t bias[4] = {-100000, -200, 300, 400};
    int32_t out[4] = {0};

    beco_write_reg(BECO_REG0, (beco_vec64_in_t){.i32 = {bias[0], 0}});
    beco_write_reg(BECO_REG1, (beco_vec64_in_t){.i32 = {bias[1], 0}});
    beco_write_reg(BECO_REG2, (beco_vec64_in_t){.i32 = {bias[2], 0}});
    beco_write_reg(BECO_REG3, (beco_vec64_in_t){.i32 = {bias[3], 0}});

    beco_set_acc_bias(BECO_ACC0_0, BECO_REG0, BECO_REG7);
    beco_set_acc_bias(BECO_ACC0_4, BECO_REG1, BECO_REG7);
    beco_set_acc_bias(BECO_ACC0_8, BECO_REG2, BECO_REG7);
    beco_set_acc_bias(BECO_ACC0_12, BECO_REG3, BECO_REG7);

    out[0] = beco_read_acc(BECO_ACC0, 0).i32;
    out[1] = beco_read_acc(BECO_ACC0, 4).i32;
    out[2] = beco_read_acc(BECO_ACC0, 8).i32;
    out[3] = beco_read_acc(BECO_ACC0, 12).i32;

    TRACE(3, "===>beco_set_add_bias_test out:");
    TRACE(3, "%d %d %d %d", out[0], out[1], out[2], out[3]);
}

static void beco_mmacrr_test()
{
    uint32_t config =
        BECO_CONF_AMODE_REP32 | BECO_CONF_BMODE_REP32 |
        BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT16 |
        BECO_CONF_RSHIFT(0)   |
        BECO_CONF_PACK_INT16  | BECO_CONF_RD_16x16;
    beco_write_config(config);

    q15_t image[] = { -1, -2, 3, 4};
    q15_t ker[] = { 10, 20, -30, -40};
    q15_t out[16];

    beco_write_reg(BECO_REG0, (beco_vec64_in_t){.i16 = {image[0], image[1], image[2], image[3]}});
    beco_write_reg(BECO_REG1, (beco_vec64_in_t){.i16 = {ker[0], ker[1], ker[2], ker[3]}});
    beco_clear_acc(BECO_ACC0);
    beco_clear_acc(BECO_ACC1);
    beco_clear_acc(BECO_ACC2);
    beco_clear_acc(BECO_ACC3);
    beco_mmacrr(BECO_REG0, BECO_REG1);

    out[0] = beco_read_acc(BECO_ACC0, 0).i16[0];
    out[1] = beco_read_acc(BECO_ACC0, 4).i16[0];
    out[2] = beco_read_acc(BECO_ACC0, 8).i16[0];
    out[3] = beco_read_acc(BECO_ACC0, 12).i16[0];
    out[4] = beco_read_acc(BECO_ACC1, 0).i16[0];
    out[5] = beco_read_acc(BECO_ACC1, 4).i16[0];
    out[6] = beco_read_acc(BECO_ACC1, 8).i16[0];
    out[7] = beco_read_acc(BECO_ACC1, 12).i16[0];
    out[8] = beco_read_acc(BECO_ACC2, 0).i16[0];
    out[9] = beco_read_acc(BECO_ACC2, 4).i16[0];
    out[10] = beco_read_acc(BECO_ACC2, 8).i16[0];
    out[11] = beco_read_acc(BECO_ACC2, 12).i16[0];
    out[12] = beco_read_acc(BECO_ACC3, 0).i16[0];
    out[13] = beco_read_acc(BECO_ACC3, 4).i16[0];
    out[14] = beco_read_acc(BECO_ACC3, 8).i16[0];
    out[15] = beco_read_acc(BECO_ACC3, 12).i16[0];
    TRACE(3, "===>test_beco_mmacrr out:");
    TRACE(3, "acc0 = %d %d %d %d", out[0], out[1], out[2], out[3]);
    TRACE(3, "acc1 = %d %d %d %d", out[4], out[5], out[6], out[7]);
    TRACE(3, "acc2 = %d %d %d %d", out[8], out[9], out[10], out[11]);
    TRACE(3, "acc3 = %d %d %d %d", out[12], out[13], out[14], out[15]);
}

static void beco_mmacgr_test()
{
    uint32_t config = BECO_CONF_AMODE_REP16    | BECO_CONF_BMODE_REP64  |
                      BECO_CONF_ATYPE_INT8     | BECO_CONF_BTYPE_INT16  |
                      BECO_CONF_RSHIFT(0)      | BECO_CONF_RD_8x16 |
                      BECO_CONF_PACK_INT16;

    beco_write_config(config);

    q15_t image[] = { -1, -2, 3, 4};
    q7_t ker[] = { -100 };
    q15_t out[4];
    beco_clear_acc(BECO_ACC0);
    beco_write_reg(BECO_REG0, (beco_vec64_in_t){.i16 = {image[0], image[1], image[2], image[3]}});
    beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.i8 = {ker[0]}}), BECO_REG0);

    out[0] = beco_read_acc(BECO_ACC0, 0).i16[0];
    out[1] = beco_read_acc(BECO_ACC0, 4).i16[0];
    out[2] = beco_read_acc(BECO_ACC0, 8).i16[0];
    out[3] = beco_read_acc(BECO_ACC0, 12).i16[0];

    TRACE(3, "===>test_beco_mmacgr out:");
    TRACE(3, "%d %d %d %d", out[0], out[1], out[2], out[3]);
}

void beco_api_test(void)
{
    TRACE(3, "===== beco_api_test START =====");
    beco_shift_block5_test();
    beco_set_add_bias_test();
    beco_mmacrr_test();
    beco_mmacgr_test();
    TRACE(3, "===== beco_api_test END =====");
}

