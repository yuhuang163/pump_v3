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

#include "lstm_s16_pt_test01.h"
#include "lstm_s16_pt_test02.h"

#include "hal_timer.h"
#include "hal_trace.h"
#include "dwt.h"

#define PRINT_RESULTS 0

void beco_lstm_s16_pt_test01(void)
{
    int16_t output_data[LSTM_S16_PT_TEST01_NUM_TIMES * LSTM_S16_PT_TEST01_NUM_BATCHES * LSTM_S16_PT_TEST01_NUM_OUTPUTS];
    int16_t hidden_state[LSTM_S16_PT_TEST01_NUM_BATCHES * LSTM_S16_PT_TEST01_NUM_LAYERS * LSTM_S16_PT_TEST01_NUM_OUTPUTS];
    int16_t cell_state[LSTM_S16_PT_TEST01_NUM_BATCHES * LSTM_S16_PT_TEST01_NUM_LAYERS * LSTM_S16_PT_TEST01_NUM_OUTPUTS];
    memset(hidden_state, 0, LSTM_S16_PT_TEST01_NUM_BATCHES * LSTM_S16_PT_TEST01_NUM_LAYERS * LSTM_S16_PT_TEST01_NUM_OUTPUTS * sizeof(int16_t));
    memset(cell_state, 0, LSTM_S16_PT_TEST01_NUM_BATCHES * LSTM_S16_PT_TEST01_NUM_LAYERS * LSTM_S16_PT_TEST01_NUM_OUTPUTS * sizeof(int16_t));
    int32_t buf_size = beco_lstm_s16_get_buffer_size(LSTM_S16_PT_TEST01_NUM_OUTPUTS);
    int16_t *scratch_buffers = (int16_t *)malloc(buf_size);

    uint32_t s_time, e_time, n_cycle, lock;
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_lstm_s16_pt(lstm_s16_pt_test01_input,
                     lstm_s16_pt_test01_input_weights_0,
                     lstm_s16_pt_test01_input_weights_n,
                     lstm_s16_pt_test01_recurrent_weight,
                     LSTM_S16_PT_TEST01_NUM_TIMES,
                     LSTM_S16_PT_TEST01_NUM_BATCHES,
                     LSTM_S16_PT_TEST01_NUM_INPUTS,
                     LSTM_S16_PT_TEST01_NUM_OUTPUTS,
                     LSTM_S16_PT_TEST01_NUM_LAYERS,
                     lstm_s16_pt_test01_input_bias,
                     lstm_s16_pt_test01_recurrent_bias,
                     lstm_s16_pt_test01_input_multiplier,
                     lstm_s16_pt_test01_input_shift,
                     lstm_s16_pt_test01_recurrent_multiplier,
                     lstm_s16_pt_test01_recurrent_shift,
                     scratch_buffers,
                     hidden_state,
                     cell_state,
                     output_data);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "beco_lstm_s16_pt_test01",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(scratch_buffers);

#if PRINT_RESULTS
    TRACE(3, "beco test01 results:");
    print_matrix(hidden_state,
                 LSTM_S16_PT_TEST01_NUM_LAYERS * LSTM_S16_PT_TEST01_NUM_OUTPUTS,
                 LSTM_S16_PT_TEST01_NUM_BATCHES,
                 LSTM_S16_PT_TEST01_NUM_LAYERS * LSTM_S16_PT_TEST01_NUM_OUTPUTS);
#endif
}

void beco_lstm_s16_pt_test02(void)
{
    int16_t output_data[LSTM_S16_PT_TEST02_NUM_TIMES * LSTM_S16_PT_TEST02_NUM_BATCHES * LSTM_S16_PT_TEST02_NUM_OUTPUTS];
    int16_t hidden_state[LSTM_S16_PT_TEST02_NUM_BATCHES * LSTM_S16_PT_TEST02_NUM_LAYERS * LSTM_S16_PT_TEST02_NUM_OUTPUTS];
    int16_t cell_state[LSTM_S16_PT_TEST02_NUM_BATCHES * LSTM_S16_PT_TEST02_NUM_LAYERS * LSTM_S16_PT_TEST02_NUM_OUTPUTS];
    memset(hidden_state, 0, LSTM_S16_PT_TEST02_NUM_BATCHES * LSTM_S16_PT_TEST02_NUM_LAYERS * LSTM_S16_PT_TEST02_NUM_OUTPUTS * sizeof(int16_t));
    memset(cell_state, 0, LSTM_S16_PT_TEST02_NUM_BATCHES * LSTM_S16_PT_TEST02_NUM_LAYERS * LSTM_S16_PT_TEST02_NUM_OUTPUTS * sizeof(int16_t));
    int32_t buf_size = beco_lstm_s16_get_buffer_size(LSTM_S16_PT_TEST02_NUM_OUTPUTS);
    int16_t *scratch_buffers = (int16_t *)malloc(buf_size);

    uint32_t s_time, e_time, n_cycle, lock;
    dwt_enable();

    TRACE_FLUSH();
    hal_sys_timer_delay(16);
    s_time = hal_fast_sys_timer_get();
    lock = int_lock();
    dwt_reset_cycle_cnt();

    beco_lstm_s16_pt(lstm_s16_pt_test02_input,
                     lstm_s16_pt_test02_input_weights_0,
                     lstm_s16_pt_test02_input_weights_n,
                     lstm_s16_pt_test02_recurrent_weight,
                     LSTM_S16_PT_TEST02_NUM_TIMES,
                     LSTM_S16_PT_TEST02_NUM_BATCHES,
                     LSTM_S16_PT_TEST02_NUM_INPUTS,
                     LSTM_S16_PT_TEST02_NUM_OUTPUTS,
                     LSTM_S16_PT_TEST02_NUM_LAYERS,
                     lstm_s16_pt_test02_input_bias,
                     lstm_s16_pt_test02_recurrent_bias,
                     lstm_s16_pt_test02_input_multiplier,
                     lstm_s16_pt_test02_input_shift,
                     lstm_s16_pt_test02_recurrent_multiplier,
                     lstm_s16_pt_test02_recurrent_shift,
                     scratch_buffers,
                     hidden_state,
                     cell_state,
                     output_data);

    n_cycle = dwt_read_cycle_cnt();
    int_unlock(lock);
    e_time = hal_fast_sys_timer_get();
    TRACE_FLUSH();
    hal_sys_timer_delay(16);

    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d",
        "beco_lstm_s16_pt_test02",
        FAST_TICKS_TO_US(e_time - s_time),
        n_cycle,
        hal_sys_timer_calc_cpu_freq(5, 0));
    free(scratch_buffers);

#if PRINT_RESULTS
    TRACE(3, "beco test02 results:");
    print_matrix(hidden_state,
                 LSTM_S16_PT_TEST02_NUM_LAYERS * LSTM_S16_PT_TEST02_NUM_OUTPUTS,
                 LSTM_S16_PT_TEST02_NUM_BATCHES,
                 LSTM_S16_PT_TEST02_NUM_LAYERS * LSTM_S16_PT_TEST02_NUM_OUTPUTS);
#endif
}


void beco_lstm_s16_pt_test(void)
{
    TRACE(3, "===== beco_lstm_s16_pt_test START =====");
    beco_lstm_s16_pt_test01();
    beco_lstm_s16_pt_test02();
    TRACE(3, "===== beco_lstm_s16_pt_test END =====");
}

