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

int32_t beco_depthwise_conv_s8_get_buffer_size(const cmsis_nn_dims *input_dims,
                                               const cmsis_nn_dims *output_dims,
                                               const cmsis_nn_dw_conv_params *dw_conv_params,
                                               const cmsis_nn_dims *filter_dims)
{
    const uint16_t input_ch = input_dims->c;
    const uint16_t kernel_x = filter_dims->w;
    const uint16_t kernel_y = filter_dims->h;
    const uint16_t dilation_y = dw_conv_params->dilation.h;
    const uint16_t dilated_kernel_y = kernel_y + (kernel_y-1) * (dilation_y-1);
    const uint16_t output_x = output_dims->w;
    uint16_t simd_width = output_x < 32 ? 8 : 32;

    int32_t im2col_buffer_len = dilated_kernel_y * input_ch * kernel_x * simd_width * sizeof(int8_t);
    int32_t output_buffer_len = 0;
#if defined(ARM_MATH_MVEI)
    output_buffer_len += simd_width * sizeof(int32_t);
#endif
    if (output_x % 8 != 0) {
        output_buffer_len += 8 * sizeof(int8_t);
    }
    return im2col_buffer_len + output_buffer_len;
}

arm_cmsis_nn_status beco_depthwise_conv_s8(const cmsis_nn_context *ctx,
                                           const cmsis_nn_dw_conv_params *dw_conv_params,
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
    (void)bias_dims;

    const uint16_t input_x = input_dims->w;
    const uint16_t input_y = input_dims->h;
    const uint16_t input_ch = input_dims->c;
    const uint16_t output_ch = output_dims->c;
    const uint16_t kernel_x = filter_dims->w;
    const uint16_t kernel_y = filter_dims->h;
    const uint16_t pad_x = dw_conv_params->padding.w;
    const uint16_t pad_y = dw_conv_params->padding.h;
    const uint16_t stride_x = dw_conv_params->stride.w;
    const uint16_t stride_y = dw_conv_params->stride.h;
    const uint16_t output_x = output_dims->w;
    const uint16_t output_y = output_dims->h;
    const uint16_t dilation_x = dw_conv_params->dilation.w;
    const uint16_t dilation_y = dw_conv_params->dilation.h;

    if (input_ch != output_ch) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }

    uint16_t dilated_kernel_x = kernel_x + (kernel_x-1) * (dilation_x-1);
    uint16_t dilated_kernel_y = kernel_y + (kernel_y-1) * (dilation_y-1);
    uint16_t expected_output_x = (input_x + 2*pad_x - dilated_kernel_x) / stride_x + 1;
    uint16_t expected_output_y = (input_y + 2*pad_y - dilated_kernel_y) / stride_y + 1;
    if (output_x != expected_output_x || output_y != expected_output_y) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }

    int8_t *buffer = (int8_t *)ctx->buf;
    beco_depthwise_conv_q7(input, input_x, input_y, input_ch,
                           kernel, output_ch, kernel_x, kernel_y,
                           pad_x, pad_y, stride_x, stride_y, dilation_x, dilation_y,
                           bias, quant_params->multiplier, quant_params->shift,
                           output, output_x, output_y,
                           -dw_conv_params->input_offset, dw_conv_params->output_offset,
                           dw_conv_params->activation.min, dw_conv_params->activation.max, buffer);

    return ARM_CMSIS_NN_SUCCESS;
}

