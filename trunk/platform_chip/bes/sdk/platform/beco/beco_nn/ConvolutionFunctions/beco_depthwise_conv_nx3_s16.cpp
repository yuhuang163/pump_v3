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

int32_t beco_depthwise_conv_nx3_s16_get_buffer_size(void)
{
    int32_t output_buffer_len = 0;
#if defined(ARM_MATH_MVEI)
    output_buffer_len += 16 * sizeof(int32_t);
#endif

    return output_buffer_len;
}

arm_cmsis_nn_status beco_depthwise_conv_nx3_s16(const cmsis_nn_context *ctx,
                                                const cmsis_nn_dw_conv_params *dw_conv_params,
                                                const cmsis_nn_per_channel_quant_params *quant_params,
                                                const cmsis_nn_dims *input_dims,
                                                const int16_t *input,
                                                const cmsis_nn_dims *filter_dims,
                                                const int8_t *kernel,
                                                const cmsis_nn_dims *bias_dims,
                                                const int32_t *bias,
                                                const cmsis_nn_dims *output_dims,
                                                int16_t *output)
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

    if (input_ch != output_ch) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }
    if (kernel_x != 3 || pad_x != 1 || output_x % 4 != 0) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }
    if (stride_x != 1 && stride_x != 2) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }

    int16_t *buffer = (int16_t *)ctx->buf;

    if (stride_x == 1) {
        beco_depthwise_conv_nx3_q15(input, input_x, input_y, input_ch,
                                    kernel, output_ch, kernel_x, kernel_y,
                                    pad_x, pad_y, stride_x, stride_y,
                                    bias, quant_params->multiplier, quant_params->shift,
                                    output, output_x, output_y,
                                    dw_conv_params->activation.min, dw_conv_params->activation.max, buffer);
    } else {
        beco_depthwise_conv_nx3s2_q15(input, input_x, input_y, input_ch,
                                    kernel, output_ch, kernel_x, kernel_y,
                                    pad_x, pad_y, stride_x, stride_y,
                                    bias, quant_params->multiplier, quant_params->shift,
                                    output, output_x, output_y,
                                    dw_conv_params->activation.min, dw_conv_params->activation.max, buffer);
    }

    return ARM_CMSIS_NN_SUCCESS;
}

