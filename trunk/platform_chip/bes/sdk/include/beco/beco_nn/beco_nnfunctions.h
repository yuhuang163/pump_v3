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


#ifndef _BECO_NNFUNCTIONS_H
#define _BECO_NNFUNCTIONS_H

#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"

#include "arm_nn_math_types.h"
#include "arm_nn_types.h"
#include "plat_types.h"

BECO_C_DECLARATIONS_START

void beco_fully_connected_q7(const int8_t *pV,
                             const int8_t *pM,
                             const uint16_t dim_vec,
                             const uint16_t num_of_cols,
                             const int32_t multiplier,
                             const int32_t out_shift,
                             const int32_t *bias,
                             int8_t *pOut,
                             const int8_t out_offset,
                             const int8_t min,
                             const int8_t max,
                             int8_t *buffer);

void beco_fully_connected_norm_q7(const int8_t *pV,
                                  const int8_t *pM,
                                  const uint16_t dim_vec,
                                  const uint16_t num_of_cols,
                                  const int32_t multiplier,
                                  const int32_t out_shift,
                                  const int32_t *bias,
                                  int8_t *pOut,
                                  const int8_t out_offset,
                                  const int8_t min,
                                  const int8_t max,
                                  int8_t *buffer);

void beco_fully_connected_q15(const int16_t *pV,
                              const int8_t *pM,
                              const uint16_t dim_vec,
                              const uint16_t num_of_cols,
                              const int32_t multiplier,
                              const int32_t out_shift,
                              const int32_t *bias,
                              int16_t *pOut,
                              const int16_t min,
                              const int16_t max,
                              int16_t *buffer);

void beco_fully_connected_norm_q15(const int16_t *pV,
                                   const int8_t *pM,
                                   const uint16_t dim_vec,
                                   const uint16_t num_of_cols,
                                   const int32_t multiplier,
                                   const int32_t out_shift,
                                   const int32_t *bias,
                                   int16_t *pOut,
                                   const int16_t min,
                                   const int16_t max,
                                   int16_t *buffer);

/**
 * @brief Basic s8 Fully Connected function.
 *
 * @param[in, out] ctx           Function context that contains the additional buffer if required by the function.
 *                               See beco_fully_connected_s8_get_buffer_size for derails.
 * @param[in]      fc_params     Fully Connected layer parameters.
 *                               Range of fc_params->input_offset  : [-127, 128]
 *                               fc_params->filter_offset : Not used
 *                               Range of fc_params->output_offset : [-128, 127]
 * @param[in]      quant_params  Per-tensor quantization info.
 *                               It contains the multiplier and shift values to be applied to the output tensor.
 * @param[in]      input_dims    Input (activation) tensor dimensions.
 * @param[in]      input_data    Input (activation) data pointer. Data type: int8
 *                               Format: [1, 1, 1, C_IN]
 * @param[in]      filter_dims   Two dimensional filter dimensions.
 * @param[in]      filter_data   Filter data pointer. Data type: int8
 *                               Format: [C_IN, 1, 1, C_OUT]
 * @param[in]      bias_dims     Bias tensor dimensions.
 * @param[in]      bias_data     Bias data pointer. Data type: int32
 *                               Format: [C_OUT]
 * @param[in]      output_dims   Output tensor dimensions.
 * @param[in, out] output_data   Output data pointer. Data type: int8
 *                               Format: [1, 1, 1, C_OUT]
 * @return     The function returns <code>ARM_CMSIS_NN_SUCCESS</code>
 *
 */
arm_cmsis_nn_status beco_fully_connected_s8(const cmsis_nn_context *ctx,
                                            const cmsis_nn_fc_params *fc_params,
                                            const cmsis_nn_per_tensor_quant_params *quant_params,
                                            const cmsis_nn_dims *input_dims,
                                            const int8_t *input,
                                            const cmsis_nn_dims *filter_dims,
                                            const int8_t *kernel,
                                            const cmsis_nn_dims *bias_dims,
                                            const int32_t *bias,
                                            const cmsis_nn_dims *output_dims,
                                            int8_t *output);

int32_t beco_fully_connected_s8_get_buffer_size(const cmsis_nn_dims *output_dims);

/**
 * @brief Basic s16 Fully Connected function.
 *
 * @param[in, out] ctx           Function context that contains the additional buffer if required by the function.
 *                               See beco_fully_connected_s16_get_buffer_size for details.
 * @param[in]      fc_params     Fully Connected layer parameters.
 *                               Range of fc_params->input_offset  : Not used
 *                               fc_params->filter_offset : Not used
 *                               Range of fc_params->output_offset : Not used
 * @param[in]      quant_params  Per-tensor quantization info.
 *                               It contains the multiplier and shift values to be applied to the output tensor.
 * @param[in]      input_dims    Input (activation) tensor dimensions.
 * @param[in]      input_data    Input (activation) data pointer. Data type: int16
 *                               Format: [1, 1, 1, C_IN]
 * @param[in]      filter_dims   Two dimensional filter dimensions.
 * @param[in]      filter_data   Filter data pointer. Data type: int8
 *                               Format: [C_IN, 1, 1, C_OUT]
 * @param[in]      bias_dims     Bias tensor dimensions.
 * @param[in]      bias_data     Bias data pointer. Data type: int32
 *                               Format: [C_OUT]
 * @param[in]      output_dims   Output tensor dimensions.
 * @param[in, out] output_data   Output data pointer. Data type: int16
 *                               Format: [1, 1, 1, C_OUT]
 * @return     The function returns <code>ARM_CMSIS_NN_SUCCESS</code>
 *
 */
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
                                             int16_t *output);

int32_t beco_fully_connected_s16_get_buffer_size(const cmsis_nn_dims *output_dims);

void beco_fully_connected_norm_q7_pc(const int8_t *pV,
                                     const int8_t *pM,
                                     const uint16_t dim_vec,
                                     const uint16_t num_of_cols,
                                     const int32_t *multiplier,
                                     const int32_t *out_shift,
                                     const int32_t *bias,
                                     int8_t *pOut,
                                     const int8_t out_offset,
                                     const int8_t min,
                                     const int8_t max);

void beco_fully_connected_norm_q15_pc(const int16_t *pV,
                                      const int8_t *pM,
                                      const uint16_t dim_vec,
                                      const uint16_t num_of_cols,
                                      const int32_t *multiplier,
                                      const int32_t *out_shift,
                                      const int32_t *bias,
                                      int16_t *pOut,
                                      const int16_t min,
                                      const int16_t max);

void beco_fully_connected_q7_pc(const int8_t *pV,
                                const int8_t *pM,
                                const uint16_t dim_vec,
                                const uint16_t num_of_cols,
                                const int32_t *multiplier,
                                const int32_t *out_shift,
                                const int32_t *bias,
                                int8_t *pOut,
                                const int8_t out_offset,
                                const int8_t min,
                                const int8_t max);

void beco_fully_connected_q15_pc(const int16_t *pV,
                                 const int8_t *pM,
                                 const uint16_t dim_vec,
                                 const uint16_t num_of_cols,
                                 const int32_t *multiplier,
                                 const int32_t *out_shift,
                                 const int32_t *bias,
                                 int16_t *pOut,
                                 const int16_t min,
                                 const int16_t max);

/**
 * @brief Basic s8 Fully Connected function.
 *
 * @param[in, out] ctx           Function context that contains the additional buffer if required by the function.
 *                               None is required by this function.
 * @param[in]      fc_params     Fully Connected layer parameters.
 *                               Range of fc_params->input_offset  : [-127, 128]
 *                               fc_params->filter_offset : Not used
 *                               Range of fc_params->output_offset : [-128, 127]
 * @param[in]      quant_params  Per-channel quantization info.
 *                               It contains the multiplier and shift values to be applied to the output tensor.
 * @param[in]      input_dims    Input (activation) tensor dimensions.
 * @param[in]      input_data    Input (activation) data pointer. Data type: int8
 *                               Format: [1, 1, 1, C_IN]
 * @param[in]      filter_dims   Two dimensional filter dimensions.
 * @param[in]      filter_data   Filter data pointer. Data type: int8
 *                               Format: [C_IN, 1, 1, C_OUT]
 * @param[in]      bias_dims     Bias tensor dimensions.
 * @param[in]      bias_data     Bias data pointer. Data type: int32
 *                               Format: [C_OUT]
 * @param[in]      output_dims   Output tensor dimensions.
 * @param[in, out] output_data   Output data pointer. Data type: int8
 *                               Format: [1, 1, 1, C_OUT]
 * @return     The function returns <code>ARM_CMSIS_NN_SUCCESS</code>
 *
 */
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
                                               int8_t *output);

/**
 * @brief Basic s16 Fully Connected function.
 *
 * @param[in, out] ctx           Function context that contains the additional buffer if required by the function.
 *                               None is required by this function.
 * @param[in]      fc_params     Fully Connected layer parameters.
 *                               Range of fc_params->input_offset  : Not used
 *                               fc_params->filter_offset : Not used
 *                               Range of fc_params->output_offset : Not used
 * @param[in]      quant_params  Per-channel quantization info.
 *                               It contains the multiplier and shift values to be applied to the output tensor.
 * @param[in]      input_dims    Input (activation) tensor dimensions.
 * @param[in]      input_data    Input (activation) data pointer. Data type: int16
 *                               Format: [1, 1, 1, C_IN]
 * @param[in]      filter_dims   Two dimensional filter dimensions.
 * @param[in]      filter_data   Filter data pointer. Data type: int8
 *                               Format: [C_IN, 1, 1, C_OUT]
 * @param[in]      bias_dims     Bias tensor dimensions.
 * @param[in]      bias_data     Bias data pointer. Data type: int32
 *                               Format: [C_OUT]
 * @param[in]      output_dims   Output tensor dimensions.
 * @param[in, out] output_data   Output data pointer. Data type: int16
 *                               Format: [1, 1, 1, C_OUT]
 * @return     The function returns <code>ARM_CMSIS_NN_SUCCESS</code>
 *
 */
arm_cmsis_nn_status beco_fully_connected_s16_pc(const cmsis_nn_context *ctx,
                                                const cmsis_nn_fc_params *fc_params,
                                                const cmsis_nn_per_channel_quant_params *quant_params,
                                                const cmsis_nn_dims *input_dims,
                                                const int16_t *input,
                                                const cmsis_nn_dims *filter_dims,
                                                const int8_t *kernel,
                                                const cmsis_nn_dims *bias_dims,
                                                const int32_t *bias,
                                                const cmsis_nn_dims *output_dims,
                                                int16_t *output);

void beco_convolve_q7(const int8_t *Im_in,
                      const uint16_t dim_im_in_x,
                      const uint16_t dim_im_in_y,
                      const uint16_t ch_im_in,
                      const int8_t *wt,
                      const uint16_t ch_im_out,
                      const uint16_t dim_kernel_x,
                      const uint16_t dim_kernel_y,
                      const uint16_t padding_x,
                      const uint16_t padding_y,
                      const uint16_t stride_x,
                      const uint16_t stride_y,
                      const int32_t *bias,
                      const int32_t *multiplier,
                      const int32_t *out_shift,
                      int8_t *Im_out,
                      const uint16_t dim_im_out_x,
                      const uint16_t dim_im_out_y,
                      const uint16_t dilation_x,
                      const uint16_t dilation_y,
                      const uint16_t groups,
                      const int8_t input_offset,
                      const int8_t out_offset,
                      const int8_t min,
                      const int8_t max,
                      int8_t *buffer);

void beco_convolve_norm_q7(const int8_t *Im_in,
                           const uint16_t dim_im_in_x,
                           const uint16_t dim_im_in_y,
                           const uint16_t ch_im_in,
                           const int8_t *wt,
                           const uint16_t ch_im_out,
                           const uint16_t dim_kernel_x,
                           const uint16_t dim_kernel_y,
                           const uint16_t padding_x,
                           const uint16_t padding_y,
                           const uint16_t stride_x,
                           const uint16_t stride_y,
                           const int32_t *bias,
                           const int32_t *multiplier,
                           const int32_t *out_shift,
                           int8_t *Im_out,
                           const uint16_t dim_im_out_x,
                           const uint16_t dim_im_out_y,
                           const uint16_t dilation_x,
                           const uint16_t dilation_y,
                           const uint16_t groups,
                           const int8_t input_offset,
                           const int8_t out_offset,
                           const int8_t min,
                           const int8_t max,
                           int8_t *buffer);

void beco_convolve_q15(const int16_t *Im_in,
                       const uint16_t dim_im_in_x,
                       const uint16_t dim_im_in_y,
                       const uint16_t ch_im_in,
                       const int8_t *wt,
                       const uint16_t ch_im_out,
                       const uint16_t dim_kernel_x,
                       const uint16_t dim_kernel_y,
                       const uint16_t padding_x,
                       const uint16_t padding_y,
                       const uint16_t stride_x,
                       const uint16_t stride_y,
                       const int32_t *bias,
                       const int32_t *multiplier,
                       const int32_t *out_shift,
                       int16_t *Im_out,
                       const uint16_t dim_im_out_x,
                       const uint16_t dim_im_out_y,
                       const uint16_t dilation_x,
                       const uint16_t dilation_y,
                       const uint16_t groups,
                       const int16_t min,
                       const int16_t max,
                       int16_t *buffer);

void beco_convolve_norm_q15(const int16_t *Im_in,
                            const uint16_t dim_im_in_x,
                            const uint16_t dim_im_in_y,
                            const uint16_t ch_im_in,
                            const int8_t *wt,
                            const uint16_t ch_im_out,
                            const uint16_t dim_kernel_x,
                            const uint16_t dim_kernel_y,
                            const uint16_t padding_x,
                            const uint16_t padding_y,
                            const uint16_t stride_x,
                            const uint16_t stride_y,
                            const int32_t *bias,
                            const int32_t *multiplier,
                            const int32_t *out_shift,
                            int16_t *Im_out,
                            const uint16_t dim_im_out_x,
                            const uint16_t dim_im_out_y,
                            const uint16_t dilation_x,
                            const uint16_t dilation_y,
                            const uint16_t groups,
                            const int16_t min,
                            const int16_t max,
                            int16_t *buffer);

/**
 * @brief Basic s8 convolution function
 * @param[in, out] ctx            Function context that contains the additional buffer if required by the function.
 *                                See beco_convolve_s8_get_buffer_size for details.
 * @param[in]      conv_params    Convolution parameters (e.g. strides, dilations, pads,...).
 *                                Range of conv_params->input_offset  : [-127, 128]
 *                                Range of conv_params->output_offset : [-128, 127]
 * @param[in]      quant_params   Per-channel quantization info.
 *                                It contains the multiplier and shift values to be applied to each output channel
 * @param[in]      input_dims     Input (activation) tensor dimensions.
 * @param[in]      input_data     Input (activation) data pointer. Data type: int8
 *                                Format: [N, C_IN, H, W]
 * @param[in]      filter_dims    Filter tensor dimensions.
 * @param[in]      filter_data    Filter data pointer. Data type: int8
 *                                Format: [C_IN, HK, WK, C_OUT]
 * @param[in]      bias_dims      Bias tensor dimensions.
 * @param[in]      bias_data      Optional bias data pointer. Data type: int32 Format: [C_OUT]
 * @param[in]      output_dims    Output tensor dimensions.
 * @param[out]     output_data    Output data pointer. Data type: int8
 *                                Format: [N, C_OUT, H, W]

 * @return     The function returns <code>ARM_CMSIS_NN_SUCCESS</code>
 *
 * @details
 *   - The following constrains on the arguments apply
 *      -# (output_dims->c / groups) % 8 = 0
 *      -# input_dims->c % groups = 0
 *
 */
arm_cmsis_nn_status beco_convolve_s8(const cmsis_nn_context *ctx,
                                     const cmsis_nn_conv_params *conv_params,
                                     const cmsis_nn_per_channel_quant_params *quant_params,
                                     const cmsis_nn_dims *input_dims,
                                     const int8_t *input_data,
                                     const cmsis_nn_dims *filter_dims,
                                     const int8_t *filter_data,
                                     const cmsis_nn_dims *bias_dims,
                                     const int32_t *bias_data,
                                     const cmsis_nn_dims *output_dims,
                                     int8_t *output_data);

int32_t beco_convolve_s8_get_buffer_size(const cmsis_nn_dims *input_dims,
                                         const cmsis_nn_dims *output_dims,
                                         const cmsis_nn_conv_params *conv_params,
                                         const cmsis_nn_dims *filter_dims);

/**
 * @brief Basic s16 convolution function
 * @param[in, out] ctx            Function context that contains the additional buffer if required by the function.
 *                                See beco_convolve_s16_get_buffer_size for details.
 * @param[in]      conv_params    Convolution parameters (e.g. strides, dilations, pads,...).
 *                                conv_params->input_offset  : Not used
 *                                conv_params->output_offset : Not used
 * @param[in]      quant_params   Per-channel quantization info.
 *                                It contains the multiplier and shift values to be applied to each output channel
 * @param[in]      input_dims     Input (activation) tensor dimensions.
 * @param[in]      input_data     Input (activation) data pointer. Data type: int16
 *                                Format: [N, C_IN, H, W]
 * @param[in]      filter_dims    Filter tensor dimensions.
 * @param[in]      filter_data    Filter data pointer. Data type: int8
 *                                Format: [C_IN, HK, WK, C_OUT]
 * @param[in]      bias_dims      Bias tensor dimensions.
 * @param[in]      bias_data      Optional bias data pointer. Data type: int32
 *                                Format: [C_OUT]
 * @param[in]      output_dims    Output tensor dimensions.
 * @param[out]     output_data    Output data pointer. Data type: int16
 *                                Format: [N, C_OUT, H, W]

 * @return     The function returns <code>ARM_CMSIS_NN_SUCCESS</code>
 *
 * @details
 *   - The following constrains on the arguments apply
 *      -# (output_dims->c / groups) % 8 = 0
 *      -# input_dims->c % groups = 0
 *
 */
arm_cmsis_nn_status beco_convolve_s16(const cmsis_nn_context *ctx,
                                      const cmsis_nn_conv_params *conv_params,
                                      const cmsis_nn_per_channel_quant_params *quant_params,
                                      const cmsis_nn_dims *input_dims,
                                      const int16_t *input_data,
                                      const cmsis_nn_dims *filter_dims,
                                      const int8_t *filter_data,
                                      const cmsis_nn_dims *bias_dims,
                                      const int32_t *bias_data,
                                      const cmsis_nn_dims *output_dims,
                                      int16_t *output_data);

int32_t beco_convolve_s16_get_buffer_size(const cmsis_nn_dims *input_dims,
                                          const cmsis_nn_dims *output_dims,
                                          const cmsis_nn_conv_params *conv_params,
                                          const cmsis_nn_dims *filter_dims);


void beco_convolve_1x1_q7(const int8_t *Im_in,
                          const uint16_t dim_im_in_x,
                          const uint16_t dim_im_in_y,
                          const uint16_t ch_im_in,
                          const int8_t *wt,
                          const uint16_t ch_im_out,
                          const int32_t *bias,
                          const int32_t *multiplier,
                          const int32_t *out_shift,
                          int8_t *Im_out,
                          const uint16_t dim_im_out_x,
                          const uint16_t dim_im_out_y,
                          const uint16_t groups,
                          const int8_t input_offset,
                          const int8_t out_offset,
                          const int8_t min,
                          const int8_t max,
                          int8_t *buffer);

void beco_convolve_1x1_norm_q7(const int8_t *Im_in,
                               const uint16_t dim_im_in_x,
                               const uint16_t dim_im_in_y,
                               const uint16_t ch_im_in,
                               const int8_t *wt,
                               const uint16_t ch_im_out,
                               const int32_t *bias,
                               const int32_t *multiplier,
                               const int32_t *out_shift,
                               int8_t *Im_out,
                               const uint16_t dim_im_out_x,
                               const uint16_t dim_im_out_y,
                               const uint16_t groups,
                               const int8_t input_offset,
                               const int8_t out_offset,
                               const int8_t min,
                               const int8_t max,
                               int8_t *buffer);

void beco_convolve_1x1_q15(const int16_t *Im_in,
                           const uint16_t dim_im_in_x,
                           const uint16_t dim_im_in_y,
                           const uint16_t ch_im_in,
                           const int8_t *wt,
                           const uint16_t ch_im_out,
                           const int32_t *bias,
                           const int32_t *multiplier,
                           const int32_t *out_shift,
                           int16_t *Im_out,
                           const uint16_t dim_im_out_x,
                           const uint16_t dim_im_out_y,
                           const uint16_t groups,
                           const int16_t min,
                           const int16_t max,
                           int16_t *buffer);

void beco_convolve_1x1_norm_q15(const int16_t *Im_in,
                                const uint16_t dim_im_in_x,
                                const uint16_t dim_im_in_y,
                                const uint16_t ch_im_in,
                                const int8_t *wt,
                                const uint16_t ch_im_out,
                                const int32_t *bias,
                                const int32_t *multiplier,
                                const int32_t *out_shift,
                                int16_t *Im_out,
                                const uint16_t dim_im_out_x,
                                const uint16_t dim_im_out_y,
                                const uint16_t groups,
                                const int16_t min,
                                const int16_t max,
                                int16_t *buffer);

/**
 * @brief s8 version for 1x1 convolution
 *
 * @param[in, out] ctx           Function context that contains the additional buffer if required by the function.
 *                               See beco_convolve_1x1_s8_get_buffer_size for details.
 * @param[in]      conv_params   Convolution parameters (e.g. strides, dilations, pads,...).
 *                               Range of conv_params->input_offset  : [-127, 128]
 *                               Range of conv_params->output_offset : [-128, 127]
 * @param[in]      quant_params  Per-channel quantization info.
 *                               It contains the multiplier and shift values to be applied to each output channel
 * @param[in]      input_dims    Input (activation) tensor dimensions.
 * @param[in]      input_data    Input (activation) data pointer. Data type: int8
 *                               Format: [N, C_IN, H, W]
 * @param[in]      filter_dims   Filter tensor dimensions.
 * @param[in]      filter_data   Filter data pointer. Data type: int8
 *                               Format: [C_IN, 1, 1, C_OUT]
 * @param[in]      bias_dims     Bias tensor dimensions.
 * @param[in]      bias_data     Optional bias data pointer. Data type: int32
 *                               Format: [C_OUT]
 * @param[in]      output_dims   Output tensor dimensions.
 * @param[out]     output_data   Output data pointer. Data type: int8
 *                               Format: [N, C_OUT, H, W]
 *
 * @return     The function returns either
 *                  <code>ARM_CMSIS_NN_ARG_ERROR</code> if argument constraints fail. or,
 *                  <code>ARM_CMSIS_NN_SUCCESS</code> on successful completion.
 * @details
 *   - The following constrains on the arguments apply
 *      -# conv_params->padding.w = conv_params->padding.h = 0
 *      -# conv_params->stride.w = conv_params->stride.h = 1
 *      -# (output_dims->c / groups) % 8 = 0
 *      -# input_dims->c / groups = 0
 *
 */
arm_cmsis_nn_status beco_convolve_1x1_s8(const cmsis_nn_context *ctx,
                                         const cmsis_nn_conv_params *conv_params,
                                         const cmsis_nn_per_channel_quant_params *quant_params,
                                         const cmsis_nn_dims *input_dims,
                                         const int8_t *input_data,
                                         const cmsis_nn_dims *filter_dims,
                                         const int8_t *filter_data,
                                         const cmsis_nn_dims *bias_dims,
                                         const int32_t *bias_data,
                                         const cmsis_nn_dims *output_dims,
                                         int8_t *output_data);

int32_t beco_convolve_1x1_s8_get_buffer_size(const cmsis_nn_dims *output_dims,
                                             const cmsis_nn_dims *filter_dims);

/**
 * @brief s16 version for 1x1 convolution
 *
 * @param[in, out] ctx           Function context that contains the additional buffer if required by the function.
 *                               See beco_convolve_1x1_s16_get_buffer_size for details.
 * @param[in]      conv_params   Convolution parameters (e.g. strides, dilations, pads,...).
 *                               Range of conv_params->input_offset  : Not used
 *                               Range of conv_params->output_offset : Not used
 * @param[in]      quant_params  Per-channel quantization info.
 *                               It contains the multiplier and shift values to be applied to each output channel
 * @param[in]      input_dims    Input (activation) tensor dimensions.
 * @param[in]      input_data    Input (activation) data pointer. Data type: int16
 *                               Format: [N, C_IN, H, W]
 * @param[in]      filter_dims   Filter tensor dimensions.
 * @param[in]      filter_data   Filter data pointer. Data type: int8
 *                               Format: [C_IN, 1, 1, C_OUT]
 * @param[in]      bias_dims     Bias tensor dimensions.
 * @param[in]      bias_data     Optional bias data pointer. Data type: int32
 *                               Format: [C_OUT]
 * @param[in]      output_dims   Output tensor dimensions.
 * @param[out]     output_data   Output data pointer. Data type: int16
 *                               Format: [N, C_OUT, H, W]
 *
 * @return     The function returns either
 *                  <code>ARM_CMSIS_NN_ARG_ERROR</code> if argument constraints fail. or,
 *                  <code>ARM_CMSIS_NN_SUCCESS</code> on successful completion.
 * @details
 *   - The following constrains on the arguments apply
 *      -# conv_params->padding.w = conv_params->padding.h = 0
 *      -# conv_params->stride.w = conv_params->stride.h = 1
 *      -# (output_dims->c / groups) % 8 = 0
 *      -# input_dims->c / groups = 0
 *
 */
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
                                          int16_t *output_data);

int32_t beco_convolve_1x1_s16_get_buffer_size(const cmsis_nn_dims *output_dims,
                                              const cmsis_nn_dims *filter_dims);


void beco_depthwise_conv_nx3_q7(const int8_t *input,
                                const uint16_t input_x,
                                const uint16_t input_y,
                                const uint16_t input_ch,
                                const int8_t *kernel,
                                const uint16_t output_ch,
                                const uint16_t kernel_x,
                                const uint16_t kernel_y,
                                const uint16_t pad_x,
                                const uint16_t pad_y,
                                const uint16_t stride_x,
                                const uint16_t stride_y,
                                const int32_t *bias,
                                const int32_t *multiplier,
                                const int32_t *out_shift,
                                int8_t *output,
                                const uint16_t output_x,
                                const uint16_t output_y,
                                const int8_t input_offset,
                                const int8_t out_offset,
                                const int8_t min,
                                const int8_t max,
                                int8_t *buffer);

void beco_depthwise_conv_nx3s2_q7(const int8_t *input,
                                  const uint16_t input_x,
                                  const uint16_t input_y,
                                  const uint16_t input_ch,
                                  const int8_t *kernel,
                                  const uint16_t output_ch,
                                  const uint16_t kernel_x,
                                  const uint16_t kernel_y,
                                  const uint16_t pad_x,
                                  const uint16_t pad_y,
                                  const uint16_t stride_x,
                                  const uint16_t stride_y,
                                  const int32_t *bias,
                                  const int32_t *multiplier,
                                  const int32_t *out_shift,
                                  int8_t *output,
                                  const uint16_t output_x,
                                  const uint16_t output_y,
                                  const int8_t input_offset,
                                  const int8_t out_offset,
                                  const int8_t min,
                                  const int8_t max,
                                  int8_t *buffer);

void beco_depthwise_conv_nx3_q15(const int16_t *input,
                                 const uint16_t input_x,
                                 const uint16_t input_y,
                                 const uint16_t input_ch,
                                 const int8_t *kernel,
                                 const uint16_t output_ch,
                                 const uint16_t kernel_x,
                                 const uint16_t kernel_y,
                                 const uint16_t pad_x,
                                 const uint16_t pad_y,
                                 const uint16_t stride_x,
                                 const uint16_t stride_y,
                                 const int32_t *bias,
                                 const int32_t *multiplier,
                                 const int32_t *out_shift,
                                 int16_t *output,
                                 const uint16_t output_x,
                                 const uint16_t output_y,
                                 const int16_t min,
                                 const int16_t max,
                                 int16_t *buffer);

void beco_depthwise_conv_nx3s2_q15(const int16_t *input,
                                   const uint16_t input_x,
                                   const uint16_t input_y,
                                   const uint16_t input_ch,
                                   const int8_t *kernel,
                                   const uint16_t output_ch,
                                   const uint16_t kernel_x,
                                   const uint16_t kernel_y,
                                   const uint16_t pad_x,
                                   const uint16_t pad_y,
                                   const uint16_t stride_x,
                                   const uint16_t stride_y,
                                   const int32_t *bias,
                                   const int32_t *multiplier,
                                   const int32_t *out_shift,
                                   int16_t *output,
                                   const uint16_t output_x,
                                   const uint16_t output_y,
                                   const int16_t min,
                                   const int16_t max,
                                   int16_t *buffer);

/**
 * @brief s8 version for depthwise convolution function
 *
 * @param[in, out] ctx             Function context that contains the additional buffer if required by the function.
 *                                 See beco_depthwise_conv_nx3_s8_get_buffer_size for details.
 * @param[in]      dw_conv_params  Depthwise convolution parameters (e.g. strides, dilations, pads,...)
 *                                 dw_conv_params->dilation is not used.
 *                                 Range of dw_conv_params->input_offset : [-127, 128]
 *                                 Range of dw_conv_params->output_offset : [-128, 127]
 * @param[in]      quant_params    Per-channel quantization info.
 *                                 It contains the multiplier and shift values to be applied to each output channel
 * @param[in]      input_dims      Input (activation) tensor dimensions.
 *                                 Batch argument N is not used and assumed to be 1.
 * @param[in]      input_data      Input (activation) data pointer. Data type: int8
 *                                 Format: [1, C_IN, H, W]
 * @param[in]      filter_dims     Filter tensor dimensions.
 * @param[in]      filter_data     Filter data pointer. Data type: int8
 *                                 Format: [1, C_OUT, H, W]
 * @param[in]      bias_dims       Bias tensor dimensions.
 * @param[in]      bias_data       Bias data pointer. Data type: int32
 *                                 Format: [C_OUT]
 * @param[in]      output_dims     Output tensor dimensions.
 * @param[in, out] output_data     Output data pointer. Data type: int8
 *                                 Format: [1, C_OUT, H, W]
 * @return     The function returns
 *                <code>ARM_CMSIS_NN_SUCCESS</code>   -  Successful completion.
 *
 * @details
 *   - The following constrains on the arguments apply
 *      -# filter_dims->w = 3
 *      -# dw_conv_params->padding.w = 1
 *      -# dw_conv_params->stride.w = 1 or dw_conv_params->stride.w = 2
 *      -# output_dims->w % 8 = 0
 *
 */
arm_cmsis_nn_status beco_depthwise_conv_nx3_s8(const cmsis_nn_context *ctx,
                                               const cmsis_nn_dw_conv_params *dw_conv_params,
                                               const cmsis_nn_per_channel_quant_params *quant_params,
                                               const cmsis_nn_dims *input_dims,
                                               const int8_t *input,
                                               const cmsis_nn_dims *filter_dims,
                                               const int8_t *kernel,
                                               const cmsis_nn_dims *bias_dims,
                                               const int32_t *bias,
                                               const cmsis_nn_dims *output_dims,
                                               int8_t *output);

int32_t beco_depthwise_conv_nx3_s8_get_buffer_size(void);

/**
 * @brief s16 version for depthwise convolution function
 *
 * @param[in, out] ctx             Function context that contains the additional buffer if required by the function.
 *                                 See beco_depthwise_conv_nx3_s16_get_buffer_size for details.
 * @param[in]      dw_conv_params  Depthwise convolution parameters (e.g. strides, dilations, pads,...)
 *                                 dw_conv_params->dilation is not used.
 *                                 Range of dw_conv_params->input_offset : Not used
 *                                 Range of dw_conv_params->output_offset : Not used
 * @param[in]      quant_params    Per-channel quantization info.
 *                                 It contains the multiplier and shift values to be applied to each output channel
 * @param[in]      input_dims      Input (activation) tensor dimensions.
 *                                 Batch argument N is not used and assumed to be 1.
 * @param[in]      input_data      Input (activation) data pointer. Data type: int16
 *                                 Format: [1, C_IN, H, W]
 * @param[in]      filter_dims     Filter tensor dimensions.
 * @param[in]      filter_data     Filter data pointer. Data type: int8
 *                                 Format: [1, C_OUT, H, W]
 * @param[in]      bias_dims       Bias tensor dimensions.
 * @param[in]      bias_data       Bias data pointer. Data type: int32
 *                                 Format: [C_OUT]
 * @param[in]      output_dims     Output tensor dimensions.
 * @param[in, out] output_data     Output data pointer. Data type: int16
 *                                 Format: [1, C_OUT, H, W]
 * @return     The function returns
 *                <code>ARM_CMSIS_NN_SUCCESS</code>   -  Successful completion.
 *
 * @details
 *   - The following constrains on the arguments apply
 *      -# filter_dims->w = 3
 *      -# dw_conv_params->padding.w = 1
 *      -# dw_conv_params->stride.w = 1 dw_conv_params->stride.w = 2
 *      -# output_dims->w % 4 = 0
 *
 */
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
                                                int16_t *output);

int32_t beco_depthwise_conv_nx3_s16_get_buffer_size(void);

void beco_depthwise_conv_q7(const int8_t* input,
                            const uint16_t input_x,
                            const uint16_t input_y,
                            const uint16_t input_ch,
                            const int8_t *kernel,
                            const uint16_t output_ch,
                            const uint16_t kernel_x,
                            const uint16_t kernel_y,
                            const uint16_t pad_x,
                            const uint16_t pad_y,
                            const uint16_t stride_x,
                            const uint16_t stride_y,
                            const uint16_t dilation_x,
                            const uint16_t dilation_y,
                            const int32_t *bias,
                            const int32_t *multiplier,
                            const int32_t *out_shift,
                            int8_t *output,
                            const uint16_t output_x,
                            const uint16_t output_y,
                            const int8_t input_offset,
                            const int8_t out_offset,
                            const int8_t min,
                            const int8_t max,
                            int8_t *buffer);

void beco_depthwise_conv_q15(const int16_t* input,
                             const uint16_t input_x,
                             const uint16_t input_y,
                             const uint16_t input_ch,
                             const int8_t *kernel,
                             const uint16_t output_ch,
                             const uint16_t kernel_x,
                             const uint16_t kernel_y,
                             const uint16_t pad_x,
                             const uint16_t pad_y,
                             const uint16_t stride_x,
                             const uint16_t stride_y,
                             const uint16_t dilation_x,
                             const uint16_t dilation_y,
                             const int32_t *bias,
                             const int32_t *multiplier,
                             const int32_t *out_shift,
                             int16_t *output,
                             const uint16_t output_x,
                             const uint16_t output_y,
                             const int16_t min,
                             const int16_t max,
                             int16_t *buffer);

/**
 * @brief s8 version for depthwise convolution function
 *
 * @param[in, out] ctx             Function context that contains the additional buffer if required by the function.
 *                                 See beco_depthwise_conv_s8_get_buffer_size for details.
 * @param[in]      dw_conv_params  Depthwise convolution parameters (e.g. strides, dilations, pads,...)
 *                                 dw_conv_params->dilation is not used.
 *                                 Range of dw_conv_params->input_offset : [-127, 128]
 *                                 Range of dw_conv_params->output_offset : [-128, 127]
 * @param[in]      quant_params    Per-channel quantization info.
 *                                 It contains the multiplier and shift values to be applied to each output channel
 * @param[in]      input_dims      Input (activation) tensor dimensions.
 *                                 Batch argument N is not used and assumed to be 1.
 * @param[in]      input_data      Input (activation) data pointer. Data type: int8
 *                                 Format: [1, C_IN, H, W]
 * @param[in]      filter_dims     Filter tensor dimensions.
 * @param[in]      filter_data     Filter data pointer. Data type: int8
 *                                 Format: [1, C_OUT, H, W]
 * @param[in]      bias_dims       Bias tensor dimensions.
 * @param[in]      bias_data       Bias data pointer. Data type: int32
 *                                 Format: [C_OUT]
 * @param[in]      output_dims     Output tensor dimensions.
 * @param[in, out] output_data     Output data pointer. Data type: int8
 *                                 Format: [1, C_OUT, H, W]
 * @return     The function returns
 *                <code>ARM_CMSIS_NN_SUCCESS</code>   -  Successful completion.
 *
 */
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
                                           int8_t *output);

/**
 * @brief s16 version for depthwise convolution function
 *
 * @param[in, out] ctx             Function context that contains the additional buffer if required by the function.
 *                                 See beco_depthwise_conv_s16_get_buffer_size for details.
 * @param[in]      dw_conv_params  Depthwise convolution parameters (e.g. strides, dilations, pads,...)
 *                                 dw_conv_params->dilation is not used.
 *                                 Range of dw_conv_params->input_offset : Not used
 *                                 Range of dw_conv_params->output_offset : Not used
 * @param[in]      quant_params    Per-channel quantization info.
 *                                 It contains the multiplier and shift values to be applied to each output channel
 * @param[in]      input_dims      Input (activation) tensor dimensions.
 *                                 Batch argument N is not used and assumed to be 1.
 * @param[in]      input_data      Input (activation) data pointer. Data type: int16
 *                                 Format: [1, C_IN, H, W]
 * @param[in]      filter_dims     Filter tensor dimensions.
 * @param[in]      filter_data     Filter data pointer. Data type: int8
 *                                 Format: [1, C_OUT, H, W]
 * @param[in]      bias_dims       Bias tensor dimensions.
 * @param[in]      bias_data       Bias data pointer. Data type: int32
 *                                 Format: [C_OUT]
 * @param[in]      output_dims     Output tensor dimensions.
 * @param[in, out] output_data     Output data pointer. Data type: int16
 *                                 Format: [1, C_OUT, H, W]
 * @return     The function returns
 *                <code>ARM_CMSIS_NN_SUCCESS</code>   -  Successful completion.
 *
 */
arm_cmsis_nn_status beco_depthwise_conv_s16(const cmsis_nn_context *ctx,
                                            const cmsis_nn_dw_conv_params *dw_conv_params,
                                            const cmsis_nn_per_channel_quant_params *quant_params,
                                            const cmsis_nn_dims *input_dims,
                                            const int16_t *input,
                                            const cmsis_nn_dims *filter_dims,
                                            const int8_t *kernel,
                                            const cmsis_nn_dims *bias_dims,
                                            const int32_t *bias,
                                            const cmsis_nn_dims *output_dims,
                                            int16_t *output);

int32_t beco_depthwise_conv_s8_get_buffer_size(const cmsis_nn_dims *input_dims,
                                               const cmsis_nn_dims *output_dims,
                                               const cmsis_nn_dw_conv_params *dw_conv_params,
                                               const cmsis_nn_dims *filter_dims);

int32_t beco_depthwise_conv_s16_get_buffer_size(const cmsis_nn_dims *input_dims,
                                                const cmsis_nn_dims *output_dims,
                                                const cmsis_nn_dw_conv_params *dw_conv_params,
                                                const cmsis_nn_dims *filter_dims);

/**
 * @brief s8 version for transposed depthwise convolution function
 *
 * @param[in, out] ctx             Function context that contains the additional buffer if required by the function.
 *                                 See beco_transpose_depthwise_conv_s8_get_buffer_size for details.
 * @param[in]      dw_conv_params  Depthwise convolution parameters (e.g. strides, dilations, pads,...)
 *                                 dw_conv_params->dilation is not used.
 *                                 Range of dw_conv_params->input_offset : [-127, 128]
 *                                 Range of dw_conv_params->output_offset : [-128, 127]
 * @param[in]      quant_params    Per-channel quantization info.
 *                                 It contains the multiplier and shift values to be applied to each output channel
 * @param[in]      input_dims      Input (activation) tensor dimensions.
 *                                 Batch argument N is not used and assumed to be 1.
 * @param[in]      input_data      Input (activation) data pointer. Data type: int8
 *                                 Format: [1, C_IN, H, W]
 * @param[in]      filter_dims     Filter tensor dimensions.
 * @param[in]      filter_data     Filter data pointer. Data type: int8
 *                                 Format: [1, C_OUT, H, W]
 *                                 The input filter data needs to be reversed.
 *                                 e.g. A 3x3 filter data:
 *                                      source:     [[ 1,  2,  3],[11, 12, 13],[21, 22, 23]]
 *                                      reversed:   [[23, 22, 21],[13, 12, 11],[ 3,  2,  1]]
 * @param[in]      bias_dims       Bias tensor dimensions.
 * @param[in]      bias_data       Bias data pointer. Data type: int32
 *                                 Format: [C_OUT]
 * @param[in]      output_dims     Output tensor dimensions.
 * @param[in, out] output_data     Output data pointer. Data type: int8
 *                                 Format: [1, C_OUT, H, W]
 * @return     The function returns
 *                <code>ARM_CMSIS_NN_SUCCESS</code>   -  Successful completion.
 *
 */
arm_cmsis_nn_status beco_transpose_depthwise_conv_s8(
                                    const cmsis_nn_context *ctx,
                                    const cmsis_nn_dw_conv_params *dw_conv_params,
                                    const cmsis_nn_per_channel_quant_params *quant_params,
                                    const cmsis_nn_dims *input_dims,
                                    const int8_t *input,
                                    const cmsis_nn_dims *filter_dims,
                                    const int8_t *kernel,
                                    const cmsis_nn_dims *bias_dims,
                                    const int32_t *bias,
                                    const cmsis_nn_dims *output_dims,
                                    int8_t *output);

/**
 * @brief s16 version for transposed depthwise convolution function
 *
 * @param[in, out] ctx             Function context that contains the additional buffer if required by the function.
 *                                 See beco_transpose_depthwise_conv_s16_get_buffer_size for details.
 * @param[in]      dw_conv_params  Depthwise convolution parameters (e.g. strides, dilations, pads,...)
 *                                 dw_conv_params->dilation is not used.
 *                                 Range of dw_conv_params->input_offset : Not used
 *                                 Range of dw_conv_params->output_offset : Not used
 * @param[in]      quant_params    Per-channel quantization info.
 *                                 It contains the multiplier and shift values to be applied to each output channel
 * @param[in]      input_dims      Input (activation) tensor dimensions.
 *                                 Batch argument N is not used and assumed to be 1.
 * @param[in]      input_data      Input (activation) data pointer. Data type: int16
 *                                 Format: [1, C_IN, H, W]
 * @param[in]      filter_dims     Filter tensor dimensions.
 * @param[in]      filter_data     Filter data pointer. Data type: int8
 *                                 Format: [1, C_OUT, H, W]
 *                                 The input filter data needs to be reversed.
 *                                 e.g. A 3x3 filter data:
 *                                      source:     [[ 1,  2,  3],[11, 12, 13],[21, 22, 23]]
 *                                      reversed:   [[23, 22, 21],[13, 12, 11],[ 3,  2,  1]]
 * @param[in]      bias_dims       Bias tensor dimensions.
 * @param[in]      bias_data       Bias data pointer. Data type: int32
 *                                 Format: [C_OUT]
 * @param[in]      output_dims     Output tensor dimensions.
 * @param[in, out] output_data     Output data pointer. Data type: int16
 *                                 Format: [1, C_OUT, H, W]
 * @return     The function returns
 *                <code>ARM_CMSIS_NN_SUCCESS</code>   -  Successful completion.
 *
 */
arm_cmsis_nn_status beco_transpose_depthwise_conv_s16(
                                    const cmsis_nn_context *ctx,
                                    const cmsis_nn_dw_conv_params *dw_conv_params,
                                    const cmsis_nn_per_channel_quant_params *quant_params,
                                    const cmsis_nn_dims *input_dims,
                                    const int16_t *input,
                                    const cmsis_nn_dims *filter_dims,
                                    const int8_t *kernel,
                                    const cmsis_nn_dims *bias_dims,
                                    const int32_t *bias,
                                    const cmsis_nn_dims *output_dims,
                                    int16_t *output);

int32_t beco_transpose_depthwise_conv_s8_get_buffer_size(
                                    const cmsis_nn_dims *input_dims,
                                    const cmsis_nn_dims *output_dims,
                                    const cmsis_nn_dw_conv_params *dw_conv_params,
                                    const cmsis_nn_dims *filter_dims);

int32_t beco_transpose_depthwise_conv_s16_get_buffer_size(
                                    const cmsis_nn_dims *input_dims,
                                    const cmsis_nn_dims *output_dims,
                                    const cmsis_nn_dw_conv_params *dw_conv_params,
                                    const cmsis_nn_dims *filter_dims);

void beco_max_pool_q7(const int8_t *src,
                      const uint16_t dim_im_in_x,
                      const uint16_t dim_im_in_y,
                      const uint16_t ch_im,
                      const uint16_t dim_kernel_x,
                      const uint16_t dim_kernel_y,
                      const uint16_t padding_x,
                      const uint16_t padding_y,
                      const uint16_t stride_x,
                      const uint16_t stride_y,
                      int8_t *dst,
                      const uint16_t dim_im_out_x,
                      const uint16_t dim_im_out_y,
                      const int8_t min,
                      const int8_t max);

void beco_max_pool_q15(const int16_t *src,
                       const uint16_t dim_im_in_x,
                       const uint16_t dim_im_in_y,
                       const uint16_t ch_im,
                       const uint16_t dim_kernel_x,
                       const uint16_t dim_kernel_y,
                       const uint16_t padding_x,
                       const uint16_t padding_y,
                       const uint16_t stride_x,
                       const uint16_t stride_y,
                       int16_t *dst,
                       const uint16_t dim_im_out_x,
                       const uint16_t dim_im_out_y,
                       const int16_t min,
                       const int16_t max);

/**
 * @brief s8 max pooling function.
 *
 * @param[in, out] ctx          Function context that contains the additional buffer if required by the function.
 *                              None is required by this function.
 * @param[in]      pool_params  Pooling parameters
 * @param[in]      input_dims   Input (activation) tensor dimensions. Format: [H, W, C_IN]
 *                              Argument 'N' is not used.
 * @param[in]      input_data   Input (activation) data pointer. The input tensor must not
 *                              overlap with the output tensor. Data type: int8
 * @param[in]      filter_dims  Filter tensor dimensions. Format: [H, W]
 *                              Argument N and C are not used.
 * @param[in]      output_dims  Output tensor dimensions. Format: [H, W, C_OUT]
 *                              Argument N is not used.
 *                              C_OUT equals C_IN.
 * @param[in, out] output_data  Output data pointer. Data type: int8
 * @return                      The function returns
 *                              <code>ARM_CMSIS_NN_SUCCESS</code> - Successful operation
 *
 * @details
 *   - The following constrains on the arguments apply
 *      -# pool_params->padding.h < filter_dims->h
 *      -# pool_params->padding.w < filter_dims->w
 *      -# input_dims->c % 8 = 0
 *
 */
arm_cmsis_nn_status beco_max_pool_s8(const cmsis_nn_context *ctx,
                                     const cmsis_nn_pool_params *pool_params,
                                     const cmsis_nn_dims *input_dims,
                                     const int8_t *src,
                                     const cmsis_nn_dims *filter_dims,
                                     const cmsis_nn_dims *output_dims,
                                     int8_t *dst);

/**
 * @brief s16 max pooling function.
 *
 * @param[in, out] ctx          Function context that contains the additional buffer if required by the function.
 *                              None is required by this function.
 * @param[in]      pool_params  Pooling parameters
 * @param[in]      input_dims   Input (activation) tensor dimensions. Format: [H, W, C_IN]
 *                              Argument 'N' is not used.
 * @param[in]      input_data   Input (activation) data pointer. The input tensor must not
 *                              overlap with the output tensor. Data type: int16
 * @param[in]      filter_dims  Filter tensor dimensions. Format: [H, W]
 *                              Argument N and C are not used.
 * @param[in]      output_dims  Output tensor dimensions. Format: [H, W, C_OUT]
 *                              Argument N is not used.
 *                              C_OUT equals C_IN.
 * @param[in, out] output_data  Output data pointer. Data type: int16
 * @return                      The function returns
 *                              <code>ARM_CMSIS_NN_SUCCESS</code> - Successful operation
 *
 * @details
 *   - The following constrains on the arguments apply
 *      -# pool_params->padding.h < filter_dims->h
 *      -# pool_params->padding.w < filter_dims->w
 *      -# input_dims->c % 4 = 0
 *
 */
arm_cmsis_nn_status beco_max_pool_s16(const cmsis_nn_context *ctx,
                                      const cmsis_nn_pool_params *pool_params,
                                      const cmsis_nn_dims *input_dims,
                                      const int16_t *src,
                                      const cmsis_nn_dims *filter_dims,
                                      const cmsis_nn_dims *output_dims,
                                      int16_t *dst);


void beco_avgpool_q7(const int8_t *src,
                     const uint16_t dim_im_in_x,
                     const uint16_t dim_im_in_y,
                     const uint16_t ch_im,
                     const uint16_t dim_kernel_x,
                     const uint16_t dim_kernel_y,
                     const uint16_t padding_x,
                     const uint16_t padding_y,
                     const uint16_t stride_x,
                     const uint16_t stride_y,
                     int8_t *dst,
                     const uint16_t dim_im_out_x,
                     const uint16_t dim_im_out_y,
                     const int8_t min,
                     const int8_t max);

void beco_avgpool_q15(const int16_t *src,
                      const uint16_t dim_im_in_x,
                      const uint16_t dim_im_in_y,
                      const uint16_t ch_im,
                      const uint16_t dim_kernel_x,
                      const uint16_t dim_kernel_y,
                      const uint16_t padding_x,
                      const uint16_t padding_y,
                      const uint16_t stride_x,
                      const uint16_t stride_y,
                      int16_t *dst,
                      const uint16_t dim_im_out_x,
                      const uint16_t dim_im_out_y,
                      const int16_t min,
                      const int16_t max);

/**
 * @brief s8 average pooling function.
 *
 * @param[in, out] ctx          Function context that contains the additional buffer if required by the function.
 *                              None is required by this function.
 * @param[in]      pool_params  Pooling parameters
 * @param[in]      input_dims   Input (activation) tensor dimensions. Format: [H, W, C_IN]
 *                              Argument 'N' is not used.
 * @param[in]      input_data   Input (activation) data pointer. Data type: int8
 * @param[in]      filter_dims  Filter tensor dimensions. Format: [H, W]
 *                              Argument N and C are not used.
 * @param[in]      output_dims  Output tensor dimensions. Format: [H, W, C_OUT]
 *                              Argument N is not used.
 *                              C_OUT equals C_IN.
 * @param[in, out] output_data  Output data pointer. Data type: int8
 * @return                      The function returns
 *                              <code>ARM_CMSIS_NN_SUCCESS</code> - Successful operation
 *
 * @details
 *   - The following constrains on the arguments apply
 *      -# pool_params->padding.h < filter_dims->h
 *      -# pool_params->padding.w < filter_dims->w
 *      -# input_dims->c % 8 = 0
 *
 */
arm_cmsis_nn_status beco_avgpool_s8(const cmsis_nn_context *ctx,
                                    const cmsis_nn_pool_params *pool_params,
                                    const cmsis_nn_dims *input_dims,
                                    const int8_t *src,
                                    const cmsis_nn_dims *filter_dims,
                                    const cmsis_nn_dims *output_dims,
                                    int8_t *dst);

/**
 * @brief s16 average pooling function.
 *
 * @param[in, out] ctx          Function context that contains the additional buffer if required by the function.
 *                              None is required by this function.
 * @param[in]      pool_params  Pooling parameters
 * @param[in]      input_dims   Input (activation) tensor dimensions. Format: [H, W, C_IN]
 *                              Argument 'N' is not used.
 * @param[in]      input_data   Input (activation) data pointer. Data type: int16
 * @param[in]      filter_dims  Filter tensor dimensions. Format: [H, W]
 *                              Argument N and C are not used.
 * @param[in]      output_dims  Output tensor dimensions. Format: [H, W, C_OUT]
 *                              Argument N is not used.
 *                              C_OUT equals C_IN.
 * @param[in, out] output_data  Output data pointer. Data type: int16
 * @return                      The function returns
 *                              <code>ARM_CMSIS_NN_SUCCESS</code> - Successful operation
 *
 * @details
 *   - The following constrains on the arguments apply
 *      -# pool_params->padding.h < filter_dims->h
 *      -# pool_params->padding.w < filter_dims->w
 *      -# input_dims->c % 4 = 0
 *
 */
arm_cmsis_nn_status beco_avgpool_s16(const cmsis_nn_context *ctx,
                                     const cmsis_nn_pool_params *pool_params,
                                     const cmsis_nn_dims *input_dims,
                                     const int16_t *src,
                                     const cmsis_nn_dims *filter_dims,
                                     const cmsis_nn_dims *output_dims,
                                     int16_t *dst);

void beco_fully_connected_pt_q15(const int16_t *pV,
                                 const int8_t *pM,
                                 const uint16_t dim_vec,
                                 const uint16_t num_of_cols,
                                 const int32_t multiplier,
                                 const int32_t out_shift,
                                 const int32_t *bias,
                                 int16_t *pOut,
                                 int16_t *buffer);

void beco_fully_connected_pc_q15(const int16_t *pV,
                                 const int8_t *pM,
                                 const uint16_t dim_vec,
                                 const uint16_t num_of_cols,
                                 const int32_t *multiplier,
                                 const int32_t *out_shift,
                                 const int32_t *bias,
                                 int16_t *pOut,
                                 int16_t *buffer);

/**
 * @brief s16 gru function.
 *
 * @param[in]      input_data           Input data pointer. Data type: int16.
 *                                      Format: [num_times, num_batches, num_inputs]
 * @param[in]      input_weights_0      Filter data pointer. Data type: int8.
 *                                      Format: [num_inputs, 3*num_outputs]
 * @param[in]      input_weights_n      Filter data pointer. Data type: int8.
 *                                      Format: [num_inputs, 3*num_outputs] * (num_layers-1)
 *                                      input_weights consists of three parts: W_ir, W_iz, W_in
 * @param[in]      recurrent_weights    Filter data pointer. Data type: int8.
 *                                      Format: [num_outputs, 3*num_outputs] * num_layers
 *                                      recurrent_weights consists of three parts: W_hr, W_hz, W_hn
 * @param[in]      num_times            GRU time dimension.
 * @param[in]      num_batches          GRU batch dimension.
 * @param[in]      num_inputs           GRU input dimension.
 * @param[in]      num_outputs          GRU output dimension.
 * @param[in]      num_layers           Number of recurrent layers.
 * @param[in]      input_bias           Bias data pointer. Data type: int32.
 *                                      Format: [3*num_outputs]
 *                                      input_bias consists of three parts: b_ir, b_iz, b_in
 * @param[in]      recurrent_bias       Bias data pointer. Data type: int32.
 *                                      Format: [3*num_outputs]
 *                                      recurrent_bias consists of three parts: b_hr, b_hz, b_hn
 * @param[in]      input_multiplier     Requant params for fc 1. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in]      input_shift          Requant params for fc 1. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in]      recurrent_multiplier Requant params for fc 2. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in]      recurrent_shift      Requant params for fc 2. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in]      scratch_buffers      Additional buffer required.
 *                                      See beco_gru_s16_get_buffer_size for details.
 * @param[in, out] hidden_state         State data pointer. Data type: int16.
 *                                      Format: [num_batches, num_layers, num_outputs]
 * @param[in, out] output_data          Output data pointer. Data type: int16
 *                                      Format: [num_times, num_batches, num_outputs]
 *
 * @return                              The function returns
 *                                      <code>ARM_CMSIS_NN_SUCCESS</code> - Successful operation
 *
 * @details
 *      -# The output_scale is fixed to 1/32768.
 *      -# Pertensor is used in fc quantization.
 *      -# Currently only supported with GRU bidirectional=False.
 *      -# Access to multiplier and shift:
 *          -a. scale_i = (input_scale * input_weights_scale[0] * 4096)
 *              multiplier_i, shift_i = frexp(scale_i)
 *              input_multiplier = (int32)(multiplier_i * (2 ^ 31 - 1))
 *              input_shift = shift_i
 *          -b. scale_i = (output_scale * recurrent_weights_scale[0] * 4096)
 *              multiplier_i, shift_i = frexp(scale_i)
 *              recurrent_multiplier = (int32)(multiplier_i * (2 ^ 31 - 1))
 *              recurrent_shift = shift_i
 *
 */
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
                                    int16_t *output_data);

/**
 * @brief s16 gru function.
 *
 * @param[in]      input_data           Input data pointer. Data type: int16.
 *                                      Format: [num_times, num_batches, num_inputs]
 * @param[in]      input_weights_0      Filter data pointer. Data type: int8.
 *                                      Format: [num_inputs, 3*num_outputs]
 * @param[in]      input_weights_n      Filter data pointer. Data type: int8.
 *                                      Format: [num_inputs, 3*num_outputs] * (num_layers-1)
 *                                      input_weights consists of three parts: W_ir, W_iz, W_in
 * @param[in]      recurrent_weights    Filter data pointer. Data type: int8.
 *                                      Format: [num_outputs, 3*num_outputs] * num_layers
 *                                      recurrent_weights consists of three parts: W_hr, W_hz, W_hn
 * @param[in]      num_times            GRU time dimension.
 * @param[in]      num_batches          GRU batch dimension.
 * @param[in]      num_inputs           GRU input dimension.
 * @param[in]      num_outputs          GRU output dimension.
 * @param[in]      num_layers           Number of recurrent layers.
 * @param[in]      input_bias           Bias data pointer. Data type: int32.
 *                                      Format: [3*num_outputs]
 *                                      input_bias consists of three parts: b_ir, b_iz, b_in
 * @param[in]      recurrent_bias       Bias data pointer. Data type: int32.
 *                                      Format: [3*num_outputs]
 *                                      recurrent_bias consists of three parts: b_hr, b_hz, b_hn
 * @param[in]      input_multiplier     Requant params for fc 1. Data type: int32.
 *                                      Format: [3*num_outputs] * num_layers.
 * @param[in]      input_shift          Requant params for fc 1. Data type: int32.
 *                                      Format: [3*num_outputs] * num_layers.
 * @param[in]      recurrent_multiplier Requant params for fc 2. Data type: int32.
 *                                      Format: [3*num_outputs] * num_layers.
 * @param[in]      recurrent_shift      Requant params for fc 2. Data type: int32.
 *                                      Format: [3*num_outputs] * num_layers.
 * @param[in]      scratch_buffers      Additional buffer required.
 *                                      See beco_gru_s16_get_buffer_size for details.
 * @param[in, out] hidden_state         State data pointer. Data type: int16.
 *                                      Format: [num_batches, num_layers, num_outputs]
 * @param[in, out] output_data          Output data pointer. Data type: int16
 *                                      Format: [num_times, num_batches, num_outputs]
 *
 * @return                              The function returns
 *                                      <code>ARM_CMSIS_NN_SUCCESS</code> - Successful operation
 *
 * @details
 *      -# The output_scale is fixed to 1/32768.
 *      -# Perchannel is used in fc quantization.
 *      -# Currently only supported with GRU bidirectional=False.
 *      -# Access to multiplier and shift:
 *          -a. scale_i = (input_scale * input_weights_scale[i] * 4096)
 *              multiplier_i, shift_i = frexp(scale_i)
 *              input_multiplier[i] = (int32)(multiplier_i * (2 ^ 31 - 1))
 *              input_shift[i] = shift_i
 *          -b. scale_i = (output_scale * recurrent_weights_scale[i] * 4096)
 *              multiplier_i, shift_i = frexp(scale_i)
 *              recurrent_multiplier[i] = (int32)(multiplier_i * (2 ^ 31 - 1))
 *              recurrent_shift[i] = shift_i
 *
 */
arm_cmsis_nn_status beco_gru_s16_pc(const int16_t *input_data,
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
                                    int16_t *output_data);

int32_t beco_gru_s16_get_buffer_size(const uint16_t num_outputs);

/**
 * @brief s16 bi-gru function.
 *
 * @param[in]      input_data           Input data pointer. Data type: int16.
 *                                      Format: [num_times, num_batches, num_inputs]
 * @param[in]      input_weights_0      Filter data pointer. Data type: int8.
 *                                      Format: [num_inputs, 3*num_outputs]
 * @param[in]      input_weights_n      Filter data pointer. Data type: int8.
 *                                      Format: [2*num_outputs, 3*num_outputs] * (num_layers-1)
 *                                      input_weights consists of three parts: W_ir, W_iz, W_in
 * @param[in]      recurrent_weights    Filter data pointer. Data type: int8.
 *                                      Format: [num_outputs, 3*num_outputs] * num_layers
 *                                      recurrent_weights consists of three parts: W_hr, W_hz, W_hn
 * @param[in]      input_weights_0_reverse      Filter data pointer. Data type: int8.
 *                                              Format: [num_inputs, 3*num_outputs]
 * @param[in]      input_weights_n_reverse      Filter data pointer. Data type: int8.
 *                                              Format: [2*num_outputs, 3*num_outputs] * (num_layers-1)
 * @param[in]      recurrent_weight_reverse     Filter data pointer. Data type: int8.
 *                                              Format: [num_outputs, 3*num_outputs] * num_layers
 * @param[in]      num_times            GRU time dimension.
 * @param[in]      num_batches          GRU batch dimension.
 * @param[in]      num_inputs           GRU input dimension.
 * @param[in]      num_outputs          GRU output dimension.
 * @param[in]      num_layers           Number of recurrent layers.
 * @param[in]      input_bias           Bias data pointer. Data type: int32.
 *                                      Format: [3*num_outputs]
 *                                      input_bias consists of three parts: b_ir, b_iz, b_in
 * @param[in]      recurrent_bias       Bias data pointer. Data type: int32.
 *                                      Format: [3*num_outputs]
 * @param[in]      input_bias_reverse   Bias data pointer. Data type: int32.
 *                                      Format: [3*num_outputs]
 * @param[in]   recurrent_bias_reverse  Bias data pointer. Data type: int32.
 *                                      Format: [3*num_outputs]
 * @param[in]      input_multiplier     Requant params for fc 1. Data type: int32.
 *                                      Format: [3*num_outputs] * num_layers.
 * @param[in]      input_shift          Requant params for fc 1. Data type: int32.
 *                                      Format: [3*num_outputs] * num_layers.
 * @param[in]      recurrent_multiplier Requant params for fc 2. Data type: int32.
 *                                      Format: [3*num_outputs] * num_layers.
 * @param[in]      recurrent_shift      Requant params for fc 2. Data type: int32.
 *                                      Format: [3*num_outputs] * num_layers.
 * @param[in] input_multiplier_reverse  Requant params for fc 1. Data type: int32.
 *                                      Format: [3*num_outputs] * num_layers.
 * @param[in]      input_shift_reverse  Requant params for fc 1. Data type: int32.
 *                                      Format: [3*num_outputs] * num_layers.
 * @param[in] recurrent_multiplier_reverse
 *                                      Requant params for fc 2. Data type: int32.
 *                                      Format: [3*num_outputs] * num_layers.
 * @param[in] recurrent_shift_reverse   Requant params for fc 2. Data type: int32.
 *                                      Format: [3*num_outputs] * num_layers.
 * @param[in]      scratch_buffers      Additional buffer required.
 *                                      See beco_bi_gru_s16_get_buffer_size for details.
 * @param[in, out] hidden_state         State data pointer. Data type: int16.
 *                                      Format: [num_layers, num_batches, num_outputs]
 * @param[in, out] hidden_state_reverse State data pointer. Data type: int16.
 *                                      Format: [num_layers, num_batches, num_outputs]
 * @param[in, out] output_data          Output data pointer. Data type: int16
 *                                      Format: [num_times, num_batches, 2*num_outputs]
 *
 * @return                              The function returns
 *                                      <code>ARM_CMSIS_NN_SUCCESS</code> - Successful operation
 *
 * @details
 *      -# The output_scale is fixed to 1/32768.
 *      -# Pertensor is used in fc quantization.
 *
 */
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
                                    int16_t *output_data);

int32_t beco_bi_gru_s16_get_buffer_size(const uint16_t num_times,
                                        const uint16_t num_batches,
                                        const uint16_t num_outputs);

/**
 * @brief s16 lstm function.
 *
 * @param[in]      input_data           Input data pointer. Data type: int16.
 *                                      Format: [num_times, num_batches, num_inputs]
 * @param[in]      input_weights_0      Filter data pointer. Data type: int8.
 *                                      Format: [num_inputs, 4*num_outputs
 * @param[in]      input_weights_n      Filter data pointer. Data type: int8.
 *                                      Format: [num_outputs, 4*num_outputs] * (num_layers-1)
 * @param[in]      recurrent_weight     Filter data pointer. Data type: int8.
 *                                      Format: [num_outputs, 4*num_outputs] * num_layers
 * @param[in]      num_times            LSTM time dimension.
 * @param[in]      num_batches          LSTM batch dimension.
 * @param[in]      num_inputs           LSTM input dimension.
 * @param[in]      num_outputs          LSTM output dimension.
 * @param[in]      num_layers           Number of recurrent layers.
 * @param[in]      input_bias           Bias data pointer. Data type: int32.
 *                                      Format: [4*num_outputs] * num_layers
 * @param[in]      recurrent_bias       Bias data pointer. Data type: int32.
 *                                      Format: [4*num_outputs]
 * @param[in]      input_multiplier     Requant params for fc 1. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in]      input_shift          Requant params for fc 1. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in]      recurrent_multiplier Requant params for fc 2. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in]      recurrent_shift      Requant params for fc 2. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in]      scratch_buffers      Additional buffer required.
 *                                      See beco_lstm_s16_get_buffer_size for details.
 * @param[in, out] hidden_state         Hihhen state data pointer. Data type: int16.
 *                                      Format: [num_batches, num_layers, num_outputs]
 * @param[in, out] cell_state           Cell state data pointer. Data type: int16.
 *                                      Format: [num_batches, num_layers, num_outputs]
 * @param[in, out] output_data          Output data pointer. Data type: int16
 *                                      Format: [num_times, num_batches, num_outputs]
 *
 * @return                              The function returns
 *                                      <code>ARM_CMSIS_NN_SUCCESS</code> - Successful operation
 *
 * @details
 *      -# The output_scale is fixed to 1/32768.
 *      -# Pertensor is used in fc quantization.
 *      -# Currently only supported with LSTM bidirectional=False.
 *      -# Access to multiplier and shift:
 *          -a. scale_i = (input_scale * input_weights_scale[0] * 4096)
 *              multiplier_i, shift_i = frexp(scale_i)
 *              input_multiplier = (int32)(multiplier_i * (2 ^ 31 - 1))
 *              input_shift = shift_i
 *          -b. scale_i = (output_scale * recurrent_weights_scale[0] * 4096)
 *              multiplier_i, shift_i = frexp(scale_i)
 *              recurrent_multiplier = (int32)(multiplier_i * (2 ^ 31 - 1))
 *              recurrent_shift = shift_i
 *
 */
arm_cmsis_nn_status beco_lstm_s16_pt(const int16_t *input_data,
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
                                     int16_t *output_data);

/**
 * @brief s16 lstm function.
 *
 * @param[in]      input_data           Input data pointer. Data type: int16.
 *                                      Format: [num_times, num_batches, num_inputs]
 * @param[in]      input_weights_0      Filter data pointer. Data type: int8.
 *                                      Format: [num_inputs, 4*num_outputs
 * @param[in]      input_weights_n      Filter data pointer. Data type: int8.
 *                                      Format: [num_outputs, 4*num_outputs] * (num_layers-1)
 * @param[in]      recurrent_weight     Filter data pointer. Data type: int8.
 *                                      Format: [num_outputs, 4*num_outputs] * num_layers
 * @param[in]      num_times            LSTM time dimension.
 * @param[in]      num_batches          LSTM batch dimension.
 * @param[in]      num_inputs           LSTM input dimension.
 * @param[in]      num_outputs          LSTM output dimension.
 * @param[in]      num_layers           Number of recurrent layers.
 * @param[in]      input_bias           Bias data pointer. Data type: int32.
 *                                      Format: [4*num_outputs] * num_layers
 * @param[in]      recurrent_bias       Bias data pointer. Data type: int32.
 *                                      Format: [4*num_outputs]
 * @param[in]      input_multiplier     Requant params for fc 1. Data type: int32.
 *                                      Format: [4*num_outputs] * num_layers.
 * @param[in]      input_shift          Requant params for fc 1. Data type: int32.
 *                                      Format: [4*num_outputs] * num_layers.
 * @param[in]      recurrent_multiplier Requant params for fc 2. Data type: int32.
 *                                      Format: [4*num_outputs] * num_layers.
 * @param[in]      recurrent_shift      Requant params for fc 2. Data type: int32.
 *                                      Format: [4*num_outputs] * num_layers.
 * @param[in]      scratch_buffers      Additional buffer required.
 *                                      See beco_lstm_s16_get_buffer_size for details.
 * @param[in, out] hidden_state         Hihhen state data pointer. Data type: int16.
 *                                      Format: [num_batches, num_layers, num_outputs]
 * @param[in, out] cell_state           Cell state data pointer. Data type: int16.
 *                                      Format: [num_batches, num_layers, num_outputs]
 * @param[in, out] output_data          Output data pointer. Data type: int16
 *                                      Format: [num_times, num_batches, num_outputs]
 *
 * @return                              The function returns
 *                                      <code>ARM_CMSIS_NN_SUCCESS</code> - Successful operation
 *
 * @details
 *      -# The output_scale is fixed to 1/32768.
 *      -# Perchannel is used in fc quantization.
 *      -# Currently only supported with LSTM bidirectional=False.
 *      -# Access to multiplier and shift:
 *          -a. scale_i = (input_scale * input_weights_scale[i] * 4096)
 *              multiplier_i, shift_i = frexp(scale_i)
 *              input_multiplier[i] = (int32)(multiplier_i * (2 ^ 31 - 1))
 *              input_shift[i] = shift_i
 *          -b. scale_i = (output_scale * recurrent_weights_scale[i] * 4096)
 *              multiplier_i, shift_i = frexp(scale_i)
 *              recurrent_multiplier[i] = (int32)(multiplier_i * (2 ^ 31 - 1))
 *              recurrent_shift[i] = shift_i
 *
 */
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
                                     int16_t *output_data);

int32_t beco_lstm_s16_get_buffer_size(const uint16_t num_outputs);

/**
 * @brief s16 bi-lstm function.
 *
 * @param[in]      input_data           Input data pointer. Data type: int16.
 *                                      Format: [num_times, num_batches, num_inputs]
 * @param[in]      input_weights_0      Filter data pointer. Data type: int8.
 *                                      Format: [num_inputs, 4*num_outputs
 * @param[in]      input_weights_n      Filter data pointer. Data type: int8.
 *                                      Format: [2*num_outputs, 4*num_outputs] * (num_layers-1)
 * @param[in]      recurrent_weight     Filter data pointer. Data type: int8.
 *                                      Format: [num_outputs, 4*num_outputs] * num_layers
 * @param[in] input_weights_0_reverse   Filter data pointer. Data type: int8.
 *                                      Format: [num_inputs, 4*num_outputs
 * @param[in] input_weights_n_reverse   Filter data pointer. Data type: int8.
 *                                      Format: [2*num_outputs, 4*num_outputs] * (num_layers-1)
 * @param[in] recurrent_weight_reverse  Filter data pointer. Data type: int8.
 *                                      Format: [num_outputs, 4*num_outputs] * num_layers
 * @param[in]      num_times            LSTM time dimension.
 * @param[in]      num_batches          LSTM batch dimension.
 * @param[in]      num_inputs           LSTM input dimension.
 * @param[in]      num_outputs          LSTM output dimension.
 * @param[in]      num_layers           Number of recurrent layers.
 * @param[in]      input_bias           Bias data pointer. Data type: int32.
 *                                      Format: [4*num_outputs] * num_layers
 * @param[in]      recurrent_bias       Bias data pointer. Data type: int32.
 *                                      Format: [4*num_outputs]
 * @param[in] input_bias_reverse        Bias data pointer. Data type: int32.
 *                                      Format: [4*num_outputs] * num_layers
 * @param[in] recurrent_bias_reverse    Bias data pointer. Data type: int32.
 *                                      Format: [4*num_outputs]
 * @param[in]      input_multiplier     Requant params for fc 1. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in]      input_shift          Requant params for fc 1. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in]      recurrent_multiplier Requant params for fc 2. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in]      recurrent_shift      Requant params for fc 2. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in] input_multiplier_reverse  Requant params for fc 1. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in] input_shift_reverse       Requant params for fc 1. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in] recurrent_multiplier_reverse
 *                                      Requant params for fc 2. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in] recurrent_shift_reverse   Requant params for fc 2. Data type: int32.
 *                                      Format: [1] * num_layers.
 * @param[in]      scratch_buffers      Additional buffer required.
 *                                      See beco_bi_lstm_s16_get_buffer_size for details.
 * @param[in, out] hidden_state         Hihhen state data pointer. Data type: int16.
 *                                      Format: [num_layers, num_batches, num_outputs]
 * @param[in, out] cell_state           Cell state data pointer. Data type: int16.
 *                                      Format: [num_layers, num_batches, num_outputs]
 * @param[in, out] hidden_state_reverse Hihhen state data pointer. Data type: int16.
 *                                      Format: [num_layers, num_batches, num_outputs]
 * @param[in, out] cell_state_reverse   Cell state data pointer. Data type: int16.
 *                                      Format: [num_layers, num_batches, num_outputs]
 * @param[in, out] output_data          Output data pointer. Data type: int16
 *                                      Format: [num_times, num_batches, 2*num_outputs]
 *
 * @return                              The function returns
 *                                      <code>ARM_CMSIS_NN_SUCCESS</code> - Successful operation
 *
 * @details
 *      -# The output_scale is fixed to 1/32768.
 *      -# Pertensor is used in fc quantization.
 *
 */
arm_cmsis_nn_status beco_bi_lstm_s16(const int16_t *input_data,
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
                                     int16_t *cell_state,
                                     int16_t *hidden_state_reverse,
                                     int16_t *cell_state_reverse,
                                     int16_t *output_data);

int32_t beco_bi_lstm_s16_get_buffer_size(const uint16_t num_times,
                                         const uint16_t num_batches,
                                         const uint16_t num_outputs);

BECO_C_DECLARATIONS_END

#endif
