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

#ifdef TEST_BECO
// #include "../hwtest.h"
#include "cmsis.h"
#include "beco.h"
#include "beco_l1.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "beco_test.h"
#include "hal_sysfreq.h"
#include "hal_cmu.h"

#define BECO_BANK BECO_ACC0

static void coprocess_init(void)
{
    uint32_t cpid, config, status;
    beco_vec64_out_t r,r1;
    beco_vec32_out_t acc0;

    cpid = beco_cpid();

    beco_write_config(0x123);

    config = beco_config();
    status = beco_status();
    acc0   = beco_read_acc(BECO_BANK, 4);

    beco_write_reg(BECO_REG4, ((beco_vec64_in_t){.u32={0x90abcdef,0x12345678}}));
    r = beco_read_reg(BECO_REG4);
    r1 = beco_read_reg(BECO_REG4);

    TRACE(0, "cpid:%08x config:%08x status:%08x\n", cpid, config, status);
    TRACE(0, "ACC(4) 4x8  : (%02x,%02x,%02x,%02x) (expect 00,00,00,00)\n",acc0.u8[3], acc0.u8[2], acc0.u8[1], acc0.u8[0]);
    TRACE(0, "reg4:%08x:%08x again:%08x:%08x\n",  r.u32[0],r.u32[1], r1.u32[0],r1.u32[1]);
}

void beco_test(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_52M);

    BECO_INIT();

    coprocess_init();

#ifdef BECO_API_TEST
    beco_api_test();
#endif

#ifdef BECO_MATRIX_TEST
    matrix_test();
#endif

#ifdef BECO_FULLY_CONNECTED_S8_TEST
    beco_fully_connected_s8_test();
#endif

#ifdef BECO_FULLY_CONNECTED_S16_TEST
    beco_fully_connected_s16_test();
#endif

#ifdef BECO_CONVOLVE_S8_TEST
    beco_convolve_s8_test();
#endif

#ifdef BECO_CONVOLVE_S16_TEST
    beco_convolve_s16_test();
#endif

#ifdef BECO_CONVOLVE_1X1_S8_TEST
    beco_convolve_1x1_s8_test();
#endif

#ifdef BECO_CONVOLVE_1X1_S16_TEST
    beco_convolve_1x1_s16_test();
#endif

#ifdef BECO_DEPTHWISE_CONV_S8_TEST
    beco_depthwise_conv_nx3_s8_test();
    beco_transpose_depthwise_conv_s8_test();
#endif

#ifdef BECO_DEPTHWISE_CONV_S16_TEST
    beco_depthwise_conv_nx3_s16_test();
    beco_transpose_depthwise_conv_s16_test();
#endif

#ifdef BECO_AVGPOOL_S8_TEST
    beco_avgpool_s8_test();
#endif

#ifdef BECO_AVGPOOL_S16_TEST
    beco_avgpool_s16_test();
#endif

#ifdef BECO_MAX_POOL_S8_TEST
    beco_max_pool_s8_test();
#endif

#ifdef BECO_MAX_POOL_S16_TEST
    beco_max_pool_s16_test();
#endif

#ifdef BECO_GRU_S16_TEST
    beco_gru_s16_pt_test();
    beco_gru_s16_pc_test();
    beco_bi_gru_s16_test();
#endif

#ifdef BECO_LSTM_S16_TEST
    beco_lstm_s16_pt_test();
    beco_lstm_s16_pc_test();
    beco_bi_lstm_s16_test();
#endif

    BECO_EXIT(0);
}
#endif /*BECO_TEST*/
