/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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

arm_cmsis_nn_status beco_max_pool_s8(const cmsis_nn_context *ctx,
                                     const cmsis_nn_pool_params *pool_params,
                                     const cmsis_nn_dims *input_dims,
                                     const int8_t *src,
                                     const cmsis_nn_dims *filter_dims,
                                     const cmsis_nn_dims *output_dims,
                                     int8_t *dst)
{
    (void)ctx;

    const uint16_t input_y = input_dims->h;
    const uint16_t input_x = input_dims->w;
    const uint16_t output_y = output_dims->h;
    const uint16_t output_x = output_dims->w;
    const uint16_t stride_y = pool_params->stride.h;
    const uint16_t stride_x = pool_params->stride.w;
    const uint16_t kernel_y = filter_dims->h;
    const uint16_t kernel_x = filter_dims->w;
    const uint16_t pad_y = pool_params->padding.h;
    const uint16_t pad_x = pool_params->padding.w;
    const uint16_t channel_in = input_dims->c;

    if (channel_in % 8 != 0 || pad_y >= kernel_y || pad_x >= kernel_x) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }

    beco_max_pool_q7(src, input_x, input_y, channel_in, kernel_x, kernel_y,
                     pad_x, pad_y, stride_x, stride_y, dst, output_x, output_y,
                     pool_params->activation.min, pool_params->activation.max);

    return ARM_CMSIS_NN_SUCCESS;
}

