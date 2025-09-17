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


#include "beco_test.h"

#include "beco_print_matrix.hpp"

#include "bi_gru_s16_test01.h"

#include "hal_timer.h"
#include "hal_trace.h"
#include "dwt.h"

#define PRINT_RESULTS 0

void beco_bi_gru_s16_test01(void)
{
    int16_t output_data[BI_GRU_S16_PT_TEST01_NUM_TIMES*BI_GRU_S16_PT_TEST01_NUM_BATCHES*2*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS];
    int16_t hidden_state[BI_GRU_S16_PT_TEST01_NUM_LAYERS*BI_GRU_S16_PT_TEST01_NUM_BATCHES*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS];
    int16_t hidden_state_reverse[BI_GRU_S16_PT_TEST01_NUM_LAYERS*BI_GRU_S16_PT_TEST01_NUM_BATCHES*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS];
    memset(hidden_state, 0, BI_GRU_S16_PT_TEST01_NUM_LAYERS*BI_GRU_S16_PT_TEST01_NUM_BATCHES*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS * sizeof(int16_t));
    memset(hidden_state_reverse, 0, BI_GRU_S16_PT_TEST01_NUM_LAYERS*BI_GRU_S16_PT_TEST01_NUM_BATCHES*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS * sizeof(int16_t));
    int32_t buf_size = beco_bi_gru_s16_get_buffer_size(BI_GRU_S16_PT_TEST01_NUM_TIMES,
                                                       BI_GRU_S16_PT_TEST01_NUM_BATCHES,
                                                       BI_GRU_S16_PT_TEST01_NUM_OUTPUTS);
    int16_t *scratch_buffers = (int16_t *)malloc(buf_size);

    uint32_t s_time, e_time, n_cycle, lock;
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_bi_gru_s16(bi_gru_s16_pt_test01_input,
                    bi_gru_s16_pt_test01_input_weights_0,
                    bi_gru_s16_pt_test01_input_weights_n,
                    bi_gru_s16_pt_test01_recurrent_weight,
                    bi_gru_s16_pt_test01_input_weights_0_reverse,
                    bi_gru_s16_pt_test01_input_weights_n_reverse,
                    bi_gru_s16_pt_test01_recurrent_weight_reverse,
                    BI_GRU_S16_PT_TEST01_NUM_TIMES,
                    BI_GRU_S16_PT_TEST01_NUM_BATCHES,
                    BI_GRU_S16_PT_TEST01_NUM_INPUTS,
                    BI_GRU_S16_PT_TEST01_NUM_OUTPUTS,
                    BI_GRU_S16_PT_TEST01_NUM_LAYERS,
                    bi_gru_s16_pt_test01_input_bias,
                    bi_gru_s16_pt_test01_recurrent_bias,
                    bi_gru_s16_pt_test01_input_bias_reverse,
                    bi_gru_s16_pt_test01_recurrent_bias_reverse,
                    bi_gru_s16_pt_test01_input_multiplier,
                    bi_gru_s16_pt_test01_input_shift,
                    bi_gru_s16_pt_test01_recurrent_multiplier,
                    bi_gru_s16_pt_test01_recurrent_shift,
                    bi_gru_s16_pt_test01_input_multiplier_reverse,
                    bi_gru_s16_pt_test01_input_shift_reverse,
                    bi_gru_s16_pt_test01_recurrent_multiplier_reverse,
                    bi_gru_s16_pt_test01_recurrent_shift_reverse,
                    scratch_buffers,
                    hidden_state,
                    hidden_state_reverse,
                    output_data);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "beco_bi_gru_s16",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(scratch_buffers);

#if PRINT_RESULTS
    TRACE(3, "beco test01 results:%s", "");
    for (int i = BI_GRU_S16_PT_TEST01_NUM_TIMES-1; i < BI_GRU_S16_PT_TEST01_NUM_TIMES; i++) {
        print_matrix(&output_data[i*BI_GRU_S16_PT_TEST01_NUM_BATCHES*2*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS],
                     2*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS,
                     BI_GRU_S16_PT_TEST01_NUM_BATCHES,
                     2*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS);
    }
#endif
}


extern "C"
void beco_bi_gru_s16_test(void)
{
    TRACE(3, "===== beco_bi_gru_s16_test START =====%s", "");
    beco_bi_gru_s16_test01();
    TRACE(3, "===== beco_bi_gru_s16_test END =====%s", "");
}
