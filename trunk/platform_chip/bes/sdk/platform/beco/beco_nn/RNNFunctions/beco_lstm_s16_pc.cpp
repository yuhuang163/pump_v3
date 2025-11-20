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
#include "arm_nnfunctions.h"
#include "arm_nnsupportfunctions.h"
#include "dsp/basic_math_functions.h"

static void beco_lstm_s16_pc_process_per_batch(const int16_t *input_data,
                                               const int8_t *input_weights_0,
                                               const int8_t *input_weights_n,
                                               const int8_t *recurrent_weight,
                                               const uint16_t num_inputs,
                                               const uint16_t num_outputs,
                                               const uint16_t num_layers,
                                               const int32_t *input_bias,
                                               const int32_t *recurrent_bias,
                                               const int32_t *input_multiplier,
                                               const int32_t *input_shift,
                                               const int32_t *recurrent_multiplier,
                                               const int32_t *recurrent_shift,
                                               int16_t *scratch_buffers,
                                               int16_t *hidden_state,
                                               int16_t *cell_state,
                                               int16_t *output_data)
{
    int num_of_cols = 4 * num_outputs;
    int16_t *mid_input_i16 = scratch_buffers;
    int16_t *mid_recurrent_i16 = mid_input_i16 + num_of_cols;
    int16_t *buffer = (int16_t *)(mid_recurrent_i16 + num_of_cols);
    int aligned_num = ALIGN(num_outputs * num_of_cols, 8);

    for (int i = 0; i < num_layers; i++) {
        if (i == 0) {
            beco_fully_connected_pc_q15(input_data, input_weights_0, num_inputs, num_of_cols,
                                        input_multiplier, input_shift,
                                        input_bias, mid_input_i16, buffer);
            beco_fully_connected_pc_q15(hidden_state, recurrent_weight, num_outputs, num_of_cols,
                                        recurrent_multiplier, recurrent_shift,
                                        recurrent_bias, mid_recurrent_i16, buffer);
        } else {
            beco_fully_connected_pc_q15(hidden_state - num_outputs, input_weights_n, num_outputs, num_of_cols,
                                        input_multiplier, input_shift,
                                        input_bias, mid_input_i16, buffer);
            beco_fully_connected_pc_q15(hidden_state, recurrent_weight, num_outputs, num_of_cols,
                                        recurrent_multiplier, recurrent_shift,
                                        recurrent_bias, mid_recurrent_i16, buffer);
            input_weights_n += aligned_num;
        }

        // update input, forget, cell, and output gates
        arm_add_q15(mid_input_i16, mid_recurrent_i16, mid_input_i16, num_of_cols);

        int16_t *i_t_i16 = mid_input_i16;
        int16_t *f_t_i16 = i_t_i16 + num_outputs;
        int16_t *g_t_i16 = f_t_i16 + num_outputs;
        int16_t *o_t_i16 = g_t_i16 + num_outputs;

        arm_nn_activation_s16(i_t_i16, i_t_i16, num_outputs, 0, ARM_SIGMOID);
        arm_nn_activation_s16(f_t_i16, f_t_i16, num_outputs, 0, ARM_SIGMOID);
        arm_nn_activation_s16(g_t_i16, g_t_i16, num_outputs, 0, ARM_TANH);
        arm_nn_activation_s16(o_t_i16, o_t_i16, num_outputs, 0, ARM_SIGMOID);

        // update hidden and cell state
        arm_mult_q15(f_t_i16, cell_state, f_t_i16, num_outputs);
        arm_mult_q15(i_t_i16, g_t_i16, i_t_i16, num_outputs);
        arm_shift_q15(i_t_i16, -3, i_t_i16, num_outputs);
        arm_add_q15(f_t_i16, i_t_i16, cell_state, num_outputs);

        arm_nn_activation_s16(cell_state, g_t_i16, num_outputs, 0, ARM_TANH);
        arm_mult_q15(o_t_i16, g_t_i16, hidden_state, num_outputs);

        recurrent_weight += aligned_num;
        if (input_bias) input_bias += num_of_cols;
        if (recurrent_bias) recurrent_bias += num_of_cols;
        input_multiplier += num_of_cols;
        input_shift += num_of_cols;
        recurrent_multiplier += num_of_cols;
        recurrent_shift += num_of_cols;
        hidden_state += num_outputs;
        cell_state += num_outputs;
    }

    memcpy(output_data, hidden_state - num_outputs, num_outputs*sizeof(int16_t));
}

void beco_lstm_s16_pc_process_per_frame(const int16_t *input_data,
                                        const int8_t *input_weights_0,
                                        const int8_t *input_weights_n,
                                        const int8_t *recurrent_weight,
                                        const uint16_t num_batches,
                                        const uint16_t num_inputs,
                                        const uint16_t num_outputs,
                                        const uint16_t num_layers,
                                        const int32_t *input_bias,
                                        const int32_t *recurrent_bias,
                                        const int32_t *input_multiplier,
                                        const int32_t *input_shift,
                                        const int32_t *recurrent_multiplier,
                                        const int32_t *recurrent_shift,
                                        int16_t *scratch_buffers,
                                        int16_t *hidden_state,
                                        int16_t *cell_state,
                                        int16_t *output_data)
{
    for (int i = 0; i < num_batches; i++) {
        beco_lstm_s16_pc_process_per_batch(input_data + i*num_inputs,
                                           input_weights_0,
                                           input_weights_n,
                                           recurrent_weight,
                                           num_inputs,
                                           num_outputs,
                                           num_layers,
                                           input_bias,
                                           recurrent_bias,
                                           input_multiplier,
                                           input_shift,
                                           recurrent_multiplier,
                                           recurrent_shift,
                                           scratch_buffers,
                                           hidden_state + i*num_layers*num_outputs,
                                           cell_state + i*num_layers*num_outputs,
                                           output_data + i*num_outputs);
    }
}

arm_cmsis_nn_status beco_lstm_s16_pc(const int16_t *input_data,
                                     const int8_t *input_weights_0,
                                     const int8_t *input_weights_n,
                                     const int8_t *recurrent_weight,
                                     const uint16_t num_times,
                                     const uint16_t num_batches,
                                     const uint16_t num_inputs,
                                     const uint16_t num_outputs,
                                     const uint16_t num_layers,
                                     const int32_t *input_bias,
                                     const int32_t *recurrent_bias,
                                     const int32_t *input_multiplier,
                                     const int32_t *input_shift,
                                     const int32_t *recurrent_multiplier,
                                     const int32_t *recurrent_shift,
                                     int16_t *scratch_buffers,
                                     int16_t *hidden_state,
                                     int16_t *cell_state,
                                     int16_t *output_data)
{
    if (num_times <= 0 || num_batches <= 0 || num_inputs <= 0 || num_outputs <= 0) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }
    if (num_layers <= 0) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }

    for (int i = 0; i < num_times; i++) {
        beco_lstm_s16_pc_process_per_frame(input_data + i*num_batches*num_inputs,
                                           input_weights_0,
                                           input_weights_n,
                                           recurrent_weight,
                                           num_batches,
                                           num_inputs,
                                           num_outputs,
                                           num_layers,
                                           input_bias,
                                           recurrent_bias,
                                           input_multiplier,
                                           input_shift,
                                           recurrent_multiplier,
                                           recurrent_shift,
                                           scratch_buffers,
                                           hidden_state,
                                           cell_state,
                                           output_data + i*num_batches*num_outputs);
    }

    return ARM_CMSIS_NN_SUCCESS;
}
