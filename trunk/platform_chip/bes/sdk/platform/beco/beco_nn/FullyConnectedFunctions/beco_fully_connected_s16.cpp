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

int32_t beco_fully_connected_s16_get_buffer_size(const cmsis_nn_dims *output_dims)
{
    int32_t output_dim = output_dims->c;
    int32_t buffer_len = 0;
    if (output_dim % 32 != 0) {
        buffer_len = 32 * sizeof(int16_t);
    }
#if defined(ARM_MATH_MVEI)
    buffer_len += 32 * sizeof(int32_t);
#endif

    return buffer_len;
}

arm_cmsis_nn_status beco_fully_connected_s16(const cmsis_nn_context *ctx,
                                             const cmsis_nn_fc_params *fc_params,
                                             const cmsis_nn_per_tensor_quant_params *quant_params,
                                             const cmsis_nn_dims *input_dims,
                                             const int16_t *input,
                                             const cmsis_nn_dims *filter_dims,
                                             const int8_t *kernel,
                                             const cmsis_nn_dims *bias_dims,
                                             const int32_t *bias,
                                             const cmsis_nn_dims *output_dims,
                                             int16_t *output)
{
    (void)filter_dims;
    (void)bias_dims;

    int16_t *buffer = (int16_t *)ctx->buf;

    if (output_dims->c % 32 == 0) {
        beco_fully_connected_q15(input, kernel, input_dims->c, output_dims->c,
                                 quant_params->multiplier, quant_params->shift,
                                 bias, output, fc_params->activation.min, fc_params->activation.max, buffer);
    } else {
        beco_fully_connected_norm_q15(input, kernel, input_dims->c, output_dims->c,
                                      quant_params->multiplier, quant_params->shift,
                                      bias, output, fc_params->activation.min, fc_params->activation.max, buffer);
    }

    return ARM_CMSIS_NN_SUCCESS;
}

