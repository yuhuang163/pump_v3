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

int32_t beco_convolve_1x1_s16_get_buffer_size(const cmsis_nn_dims *output_dims,
                                              const cmsis_nn_dims *filter_dims)
{
    const uint16_t grouped_in_ch = filter_dims->c;
    const uint16_t im_size = output_dims->h * output_dims->w;

    int32_t output_buffer_len = 0;
#if defined(ARM_MATH_MVEI)
    output_buffer_len += 32 * sizeof(int32_t);
#endif
    if (im_size % 4 != 0) {
        output_buffer_len += 4 * grouped_in_ch * sizeof(int16_t) + 32 * sizeof(int16_t);
    }

    return output_buffer_len;
}

arm_cmsis_nn_status beco_convolve_1x1_s16(const cmsis_nn_context *ctx,
                                          const cmsis_nn_conv_params *conv_params,
                                          const cmsis_nn_per_channel_quant_params *quant_params,
                                          const cmsis_nn_dims *input_dims,
                                          const int16_t *input_data,
                                          const cmsis_nn_dims *filter_dims,
                                          const int8_t *filter_data,
                                          const cmsis_nn_dims *bias_dims,
                                          const int32_t *bias_data,
                                          const cmsis_nn_dims *output_dims,
                                          int16_t *output_data)
{
    (void)bias_dims;

    const uint16_t input_x = input_dims->w;
    const uint16_t input_y = input_dims->h;
    const uint16_t input_ch = input_dims->c;
    const uint16_t kernel_ch = filter_dims->c;
    const uint16_t output_x = output_dims->w;
    const uint16_t output_y = output_dims->h;
    const uint16_t output_ch = output_dims->c;
    const uint16_t im_size = output_x * output_y;

    const uint16_t groups = input_ch / kernel_ch;

    if ((output_ch / groups) % 8 != 0 || input_ch % groups != 0) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }

    if (conv_params->padding.h != 0 || conv_params->padding.w != 0 ||
        conv_params->stride.h != 1 || conv_params->stride.w != 1) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }

    int16_t *buffer = (int16_t *)ctx->buf;

    if (im_size % 4 == 0) {
        beco_convolve_1x1_q15(input_data, input_x, input_y, input_ch,
                              filter_data, output_ch, bias_data,
                              quant_params->multiplier, quant_params->shift,
                              output_data, output_x, output_y, groups,
                              conv_params->activation.min, conv_params->activation.max, buffer);
    } else {
        beco_convolve_1x1_norm_q15(input_data, input_x, input_y, input_ch,
                                   filter_data, output_ch, bias_data,
                                   quant_params->multiplier, quant_params->shift,
                                   output_data, output_x, output_y, groups,
                                   conv_params->activation.min, conv_params->activation.max, buffer);
    }

    return ARM_CMSIS_NN_SUCCESS;
}

