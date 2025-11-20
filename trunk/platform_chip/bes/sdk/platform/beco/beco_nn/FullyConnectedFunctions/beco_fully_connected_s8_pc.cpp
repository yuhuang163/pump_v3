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

arm_cmsis_nn_status beco_fully_connected_s8_pc(const cmsis_nn_context *ctx,
                                               const cmsis_nn_fc_params *fc_params,
                                               const cmsis_nn_per_channel_quant_params *quant_params,
                                               const cmsis_nn_dims *input_dims,
                                               const int8_t *input,
                                               const cmsis_nn_dims *filter_dims,
                                               const int8_t *kernel,
                                               const cmsis_nn_dims *bias_dims,
                                               const int32_t *bias,
                                               const cmsis_nn_dims *output_dims,
                                               int8_t *output)
{
    (void)ctx;
    (void)fc_params;
    (void)filter_dims;
    (void)bias_dims;

    if (output_dims->c % 32 == 0) {
        beco_fully_connected_q7_pc(input, kernel, input_dims->c, output_dims->c,
                                   quant_params->multiplier, quant_params->shift,
                                   bias, output, fc_params->output_offset,
                                   fc_params->activation.min, fc_params->activation.max);
    } else {
        beco_fully_connected_norm_q7_pc(input, kernel, input_dims->c, output_dims->c,
                                        quant_params->multiplier, quant_params->shift,
                                        bias, output, fc_params->output_offset,
                                        fc_params->activation.min, fc_params->activation.max);
    }

    return ARM_CMSIS_NN_SUCCESS;
}

