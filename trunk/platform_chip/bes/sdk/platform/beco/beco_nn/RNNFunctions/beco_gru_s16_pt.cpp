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
#include "beco_bias.hpp"

static void beco_gru_s16_pt_process_per_batch(const int16_t *input_data,
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
                                              int16_t *output_data)
{
    int num_of_cols = 3 * num_outputs;
    int16_t *mid_input_i16 = scratch_buffers;
    int16_t *mid_recurrent_i16 = mid_input_i16 + num_of_cols;
    int16_t *buffer = (int16_t *)(mid_recurrent_i16 + num_of_cols);
    int aligned_num = ALIGN(num_outputs * num_of_cols, 8);

    for (int i = 0; i < num_layers; i++) {
        if (i == 0) {
            beco_fully_connected_pt_q15(input_data, input_weights_0, num_inputs, num_of_cols,
                                        input_multiplier[i], input_shift[i],
                                        input_bias, mid_input_i16, buffer);

            beco_fully_connected_pt_q15(hidden_state, recurrent_weight, num_outputs, num_of_cols,
                                        recurrent_multiplier[i], recurrent_shift[i],
                                        recurrent_bias, mid_recurrent_i16, buffer);
        } else {
            beco_fully_connected_pt_q15(hidden_state - num_outputs, input_weights_n, num_outputs, num_of_cols,
                                        input_multiplier[i], input_shift[i],
                                        input_bias, mid_input_i16, buffer);

            beco_fully_connected_pt_q15(hidden_state, recurrent_weight, num_outputs, num_of_cols,
                                        recurrent_multiplier[i], recurrent_shift[i],
                                        recurrent_bias, mid_recurrent_i16, buffer);
            input_weights_n += aligned_num;
        }

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

        recurrent_weight += aligned_num;
        if (input_bias) input_bias += num_of_cols;
        if (recurrent_bias) recurrent_bias += num_of_cols;
        hidden_state += num_outputs;
    }

    memcpy(output_data, hidden_state - num_outputs, num_outputs*sizeof(int16_t));
}

void beco_gru_s16_pt_process_per_frame(const int16_t *input_data,
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
                                       int16_t *output_data)
{
    for (int i = 0; i < num_batches; i++) {
        beco_gru_s16_pt_process_per_batch(input_data + i*num_inputs,
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
                                          output_data + i*num_outputs);
    }
}

arm_cmsis_nn_status beco_gru_s16_pt(const int16_t *input_data,
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
                                    int16_t *output_data)
{
    if (num_times <= 0 || num_batches <= 0 || num_inputs <= 0 || num_outputs <= 0) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }

    for (int i = 0; i < num_times; i++) {
        beco_gru_s16_pt_process_per_frame(input_data + i*num_batches*num_inputs,
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
                                          output_data + i*num_batches*num_outputs);
    }

    return ARM_CMSIS_NN_SUCCESS;
}
