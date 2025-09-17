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

static void beco_bi_gru_s16_process_per_batch(const int16_t *input_data,
                                              const int16_t *input_data_reverse,
                                              const int8_t *input_weights_0,
                                              const int8_t *input_weights_n,
                                              const int8_t *recurrent_weight,
                                              const int8_t *input_weights_0_reverse,
                                              const int8_t *input_weights_n_reverse,
                                              const int8_t *recurrent_weight_reverse,
                                              const uint16_t num_inputs,
                                              const uint16_t num_outputs,
                                              const int32_t *input_bias,
                                              const int32_t *recurrent_bias,
                                              const int32_t *input_bias_reverse,
                                              const int32_t *recurrent_bias_reverse,
                                              const int32_t input_multiplier,
                                              const int32_t input_shift,
                                              const int32_t recurrent_multiplier,
                                              const int32_t recurrent_shift,
                                              const int32_t input_multiplier_reverse,
                                              const int32_t input_shift_reverse,
                                              const int32_t recurrent_multiplier_reverse,
                                              const int32_t recurrent_shift_reverse,
                                              int16_t *scratch_buffers,
                                              int16_t *hidden_state,
                                              int16_t *hidden_state_reverse,
                                              int16_t *output_data,
                                              int16_t *output_data_reverse)
{
    int num_of_cols = 3 * num_outputs;
    int16_t *mid_input_i16 = scratch_buffers;
    int16_t *mid_recurrent_i16 = mid_input_i16 + num_of_cols;
    int16_t *buffer = (int16_t *)(mid_recurrent_i16 + num_of_cols);
    // forward gru
    if (input_weights_0) {
        beco_fully_connected_pt_q15(input_data, input_weights_0, num_inputs, num_of_cols,
                                    input_multiplier, input_shift,
                                    input_bias, mid_input_i16, buffer);
    } else {
        beco_fully_connected_pt_q15(input_data, input_weights_n, num_inputs, num_of_cols,
                                    input_multiplier, input_shift,
                                    input_bias, mid_input_i16, buffer);
    }
    beco_fully_connected_pt_q15(hidden_state, recurrent_weight, num_outputs, num_of_cols,
                                recurrent_multiplier, recurrent_shift,
                                recurrent_bias, mid_recurrent_i16, buffer);

    // updata and reset gate
    int16_t *z_r_i16 = mid_input_i16;
    arm_add_q15(mid_input_i16, mid_recurrent_i16, z_r_i16, 2*num_outputs);
    arm_nn_activation_s16(z_r_i16, z_r_i16, 2*num_outputs, 0, ARM_SIGMOID);

    int16_t *r = z_r_i16;
    int16_t *z = r + num_outputs;
    int16_t *h = z + num_outputs;
    int16_t *h_mid_recurrent = mid_recurrent_i16 + 2*num_outputs;
    arm_mult_q15(h_mid_recurrent, r, h_mid_recurrent, num_outputs);
    arm_add_q15(h, h_mid_recurrent, h, num_outputs);
    arm_nn_activation_s16(h, h, num_outputs, 0, ARM_TANH);

    arm_mult_q15(z, hidden_state, h_mid_recurrent, num_outputs);
    arm_offset_q15(z, -32768, z, num_outputs);
    arm_negate_q15(z, z, num_outputs);
    arm_mult_q15(z, h, h, num_outputs);
    arm_add_q15(h, h_mid_recurrent, hidden_state, num_outputs);

    // reverse gru
    if (input_weights_0) {
        beco_fully_connected_pt_q15(input_data_reverse, input_weights_0_reverse, num_inputs, num_of_cols,
                                    input_multiplier_reverse, input_shift_reverse,
                                    input_bias_reverse, mid_input_i16, buffer);
    } else {
        beco_fully_connected_pt_q15(input_data_reverse, input_weights_n_reverse, num_inputs, num_of_cols,
                                    input_multiplier_reverse, input_shift_reverse,
                                    input_bias_reverse, mid_input_i16, buffer);
    }
    beco_fully_connected_pt_q15(hidden_state_reverse, recurrent_weight_reverse, num_outputs, num_of_cols,
                                recurrent_multiplier_reverse, recurrent_shift_reverse,
                                recurrent_bias_reverse, mid_recurrent_i16, buffer);

    // updata and reset gate
    arm_add_q15(mid_input_i16, mid_recurrent_i16, z_r_i16, 2*num_outputs);
    arm_nn_activation_s16(z_r_i16, z_r_i16, 2*num_outputs, 0, ARM_SIGMOID);

    arm_mult_q15(h_mid_recurrent, r, h_mid_recurrent, num_outputs);
    arm_add_q15(h, h_mid_recurrent, h, num_outputs);
    arm_nn_activation_s16(h, h, num_outputs, 0, ARM_TANH);

    arm_mult_q15(z, hidden_state_reverse, h_mid_recurrent, num_outputs);
    arm_offset_q15(z, -32768, z, num_outputs);
    arm_negate_q15(z, z, num_outputs);
    arm_mult_q15(z, h, h, num_outputs);
    arm_add_q15(h, h_mid_recurrent, hidden_state_reverse, num_outputs);

    // update outputs
    memcpy(output_data, hidden_state, num_outputs*sizeof(int16_t));
    memcpy(output_data_reverse + num_outputs, hidden_state_reverse, num_outputs*sizeof(int16_t));
}

void beco_bi_gru_s16_process_per_frame(const int16_t *input_data,
                                       const int16_t *input_data_reverse,
                                       const int8_t *input_weights_0,
                                       const int8_t *input_weights_n,
                                       const int8_t *recurrent_weight,
                                       const int8_t *input_weights_0_reverse,
                                       const int8_t *input_weights_n_reverse,
                                       const int8_t *recurrent_weight_reverse,
                                       const uint16_t num_batches,
                                       const uint16_t num_inputs,
                                       const uint16_t num_outputs,
                                       const int32_t *input_bias,
                                       const int32_t *recurrent_bias,
                                       const int32_t *input_bias_reverse,
                                       const int32_t *recurrent_bias_reverse,
                                       const int32_t input_multiplier,
                                       const int32_t input_shift,
                                       const int32_t recurrent_multiplier,
                                       const int32_t recurrent_shift,
                                       const int32_t input_multiplier_reverse,
                                       const int32_t input_shift_reverse,
                                       const int32_t recurrent_multiplier_reverse,
                                       const int32_t recurrent_shift_reverse,
                                       int16_t *scratch_buffers,
                                       int16_t *hidden_state,
                                       int16_t *hidden_state_reverse,
                                       int16_t *output_data,
                                       int16_t *output_data_reverse)
{
    for (int i = 0; i < num_batches; i++) {
        beco_bi_gru_s16_process_per_batch(input_data + i*num_inputs,
                                           input_data_reverse + i*num_inputs,
                                           input_weights_0,
                                           input_weights_n,
                                           recurrent_weight,
                                           input_weights_0_reverse,
                                           input_weights_n_reverse,
                                           recurrent_weight_reverse,
                                           num_inputs,
                                           num_outputs,
                                           input_bias,
                                           recurrent_bias,
                                           input_bias_reverse,
                                           recurrent_bias_reverse,
                                           input_multiplier,
                                           input_shift,
                                           recurrent_multiplier,
                                           recurrent_shift,
                                           input_multiplier_reverse,
                                           input_shift_reverse,
                                           recurrent_multiplier_reverse,
                                           recurrent_shift_reverse,
                                           scratch_buffers,
                                           hidden_state + i*num_outputs,
                                           hidden_state_reverse + i*num_outputs,
                                           output_data + i*2*num_outputs,
                                           output_data_reverse + i*2*num_outputs);
    }
}

void beco_bi_gru_s16_process_per_layer(const int16_t *input_data,
                                       const int16_t *input_data_reverse,
                                       const int8_t *input_weights_0,
                                       const int8_t *input_weights_n,
                                       const int8_t *recurrent_weight,
                                       const int8_t *input_weights_0_reverse,
                                       const int8_t *input_weights_n_reverse,
                                       const int8_t *recurrent_weight_reverse,
                                       const uint16_t num_times,
                                       const uint16_t num_batches,
                                       const uint16_t num_inputs,
                                       const uint16_t num_outputs,
                                       const int32_t *input_bias,
                                       const int32_t *recurrent_bias,
                                       const int32_t *input_bias_reverse,
                                       const int32_t *recurrent_bias_reverse,
                                       const int32_t input_multiplier,
                                       const int32_t input_shift,
                                       const int32_t recurrent_multiplier,
                                       const int32_t recurrent_shift,
                                       const int32_t input_multiplier_reverse,
                                       const int32_t input_shift_reverse,
                                       const int32_t recurrent_multiplier_reverse,
                                       const int32_t recurrent_shift_reverse,
                                       int16_t *scratch_buffers,
                                       int16_t *hidden_state,
                                       int16_t *hidden_state_reverse,
                                       int16_t *output_data,
                                       int16_t *output_data_reverse)
{
    for (int i = 0; i < num_times; i++) {
        beco_bi_gru_s16_process_per_frame(input_data + i*num_batches*num_inputs,
                                           input_data_reverse - i*num_batches*num_inputs,
                                           input_weights_0,
                                           input_weights_n,
                                           recurrent_weight,
                                           input_weights_0_reverse,
                                           input_weights_n_reverse,
                                           recurrent_weight_reverse,
                                           num_batches,
                                           num_inputs,
                                           num_outputs,
                                           input_bias,
                                           recurrent_bias,
                                           input_bias_reverse,
                                           recurrent_bias_reverse,
                                           input_multiplier,
                                           input_shift,
                                           recurrent_multiplier,
                                           recurrent_shift,
                                           input_multiplier_reverse,
                                           input_shift_reverse,
                                           recurrent_multiplier_reverse,
                                           recurrent_shift_reverse,
                                           scratch_buffers,
                                           hidden_state,
                                           hidden_state_reverse,
                                           output_data + i*num_batches*2*num_outputs,
                                           output_data_reverse - i*num_batches*2*num_outputs);
    }
}

arm_cmsis_nn_status beco_bi_gru_s16(const int16_t *input_data,
                                    const int8_t *input_weights_0,
                                    const int8_t *input_weights_n,
                                    const int8_t *recurrent_weight,
                                    const int8_t *input_weights_0_reverse,
                                    const int8_t *input_weights_n_reverse,
                                    const int8_t *recurrent_weight_reverse,
                                    const uint16_t num_times,
                                    const uint16_t num_batches,
                                    const uint16_t num_inputs,
                                    const uint16_t num_outputs,
                                    const uint16_t num_layers,
                                    const int32_t *input_bias,
                                    const int32_t *recurrent_bias,
                                    const int32_t *input_bias_reverse,
                                    const int32_t *recurrent_bias_reverse,
                                    const int32_t *input_multiplier,
                                    const int32_t *input_shift,
                                    const int32_t *recurrent_multiplier,
                                    const int32_t *recurrent_shift,
                                    const int32_t *input_multiplier_reverse,
                                    const int32_t *input_shift_reverse,
                                    const int32_t *recurrent_multiplier_reverse,
                                    const int32_t *recurrent_shift_reverse,
                                    int16_t *scratch_buffers,
                                    int16_t *hidden_state,
                                    int16_t *hidden_state_reverse,
                                    int16_t *output_data)
{
    if (num_times <= 0 || num_batches <= 0 || num_inputs <= 0 || num_outputs <= 0) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }
    if (num_layers <= 0) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }

    const uint16_t num_of_cols = 3 * num_outputs;
    for (int i = 0; i < num_layers; i++) {
        int16_t *cru_input;
        uint16_t cur_num_inputs;
        int8_t *cur_input_weights_0;
        int8_t *cur_input_weights_n;
        int8_t *cur_input_weights_0_reverse;
        int8_t *cur_input_weights_n_reverse;
        if (i == 0) {
            cru_input = (int16_t *)input_data;
            cur_num_inputs = num_inputs;
            cur_input_weights_0 = (int8_t *)input_weights_0;
            cur_input_weights_0_reverse = (int8_t *)input_weights_0_reverse;
            cur_input_weights_n = NULL;
            cur_input_weights_n_reverse = NULL;
        } else {
            cur_num_inputs = 2*num_outputs;
            cru_input = scratch_buffers;
            memcpy(cru_input, output_data, num_times*num_batches*cur_num_inputs*sizeof(int16_t));
            cur_input_weights_0 = NULL;
            cur_input_weights_0_reverse = NULL;
            cur_input_weights_n = (int8_t *)input_weights_n;
            cur_input_weights_n_reverse = (int8_t *)input_weights_n_reverse;
        }

        int16_t *output_buffers = scratch_buffers + num_times * num_batches * 2 * num_outputs;
        beco_bi_gru_s16_process_per_layer(cru_input,
                                           cru_input + (num_times-1)*num_batches*cur_num_inputs,
                                           cur_input_weights_0,
                                           cur_input_weights_n,
                                           recurrent_weight,
                                           cur_input_weights_0_reverse,
                                           cur_input_weights_n_reverse,
                                           recurrent_weight_reverse,
                                           num_times,
                                           num_batches,
                                           cur_num_inputs,
                                           num_outputs,
                                           input_bias,
                                           recurrent_bias,
                                           input_bias_reverse,
                                           recurrent_bias_reverse,
                                           input_multiplier[i],
                                           input_shift[i],
                                           recurrent_multiplier[i],
                                           recurrent_shift[i],
                                           input_multiplier_reverse[i],
                                           input_shift_reverse[i],
                                           recurrent_multiplier_reverse[i],
                                           recurrent_shift_reverse[i],
                                           output_buffers,
                                           hidden_state + i*num_batches*num_outputs,
                                           hidden_state_reverse + i*num_batches*num_outputs,
                                           output_data,
                                           output_data + (num_times-1)*num_batches*2*num_outputs);

        if (i > 0) {
            input_weights_n += ALIGN(cur_num_inputs * num_of_cols, 8);
            input_weights_n_reverse += ALIGN(cur_num_inputs * num_of_cols, 8);
        }

        recurrent_weight += ALIGN(num_outputs * num_of_cols, 8);
        recurrent_weight_reverse += ALIGN(num_outputs * num_of_cols, 8);
        if (input_bias) input_bias += num_of_cols;
        if (recurrent_bias) recurrent_bias += num_of_cols;
        if (input_bias_reverse) input_bias_reverse += num_of_cols;
        if (recurrent_bias_reverse) recurrent_bias_reverse += num_of_cols;
    }

    return ARM_CMSIS_NN_SUCCESS;
}
