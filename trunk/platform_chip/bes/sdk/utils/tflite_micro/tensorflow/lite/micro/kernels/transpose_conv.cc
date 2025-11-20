/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/lite/kernels/internal/reference/transpose_conv.h"

#include "arm_nnfunctions.h"
#include "arm_nnsupportfunctions.h"

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/common.h"
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/kernels/internal/reference/integer_ops/transpose_conv.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/kernels/padding.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"
#include "tensorflow/lite/micro/micro_log.h"

namespace tflite {
namespace {

// For the TfLite transpose_conv implementation, input tensor 0 corresponds to
// the OutputShapeTensor. However, since TFLM does not support dynamic tensors,
// the TFLM implementation ignores input tensor 0 and the only inputs we care
// about are kFilterTensor, kInputTensor and kBiasTensor.
constexpr int kFilterTensor = 1;
constexpr int kInputTensor = 2;
constexpr int kBiasTensor = 3;
constexpr int kOutputTensor = 0;

// Conv is quantized along dimension 0:
// https://www.tensorflow.org/lite/performance/quantization_spec
constexpr int kConvQuantizedDimension = 0;

struct OpData {
  ConvParams params;

  // A scratch buffer is required for quantized implementations.
  int scratch_buffer_index;

  // TODO(b/192090531): Remove this once all 8x16 transpose conv models use
  // 64-bit biases.
  int bias_converted_buffer_index;

  // Multiplier and shift arrays are required for the int8 implementation.
  int32_t* per_channel_output_multiplier;
  int32_t* per_channel_output_shift;
};

inline PaddingType RuntimePaddingType(TfLitePadding padding) {
  switch (padding) {
    case TfLitePadding::kTfLitePaddingSame:
      return PaddingType::kSame;
    case TfLitePadding::kTfLitePaddingValid:
      return PaddingType::kValid;
    case TfLitePadding::kTfLitePaddingUnknown:
    default:
      return PaddingType::kNone;
  }
}

TfLiteStatus CalculateOpData(TfLiteContext* context, TfLiteNode* node,
                             const TfLiteTransposeConvParams* params, int width,
                             int height, int filter_width, int filter_height,
                             const TfLiteType data_type, OpData* data) {
  bool has_bias = node->inputs->size == 4;
  // Check number of inputs/outputs
  TF_LITE_ENSURE(context, has_bias || node->inputs->size == 3);
  TF_LITE_ENSURE_EQ(context, node->outputs->size, 1);

  // Matching GetWindowedOutputSize in TensorFlow.
  auto padding = params->padding;
  int unused_output_width;
  int unused_output_height;
  TfLitePaddingValues padding_values = ComputePaddingHeightWidth(
      params->stride_height, params->stride_width, 1,
      1,  // Dilation height and width are always 1 for transpose_conv.
      height, width, filter_height, filter_width, padding,
      &unused_output_height, &unused_output_width);

  data->params.padding_type = RuntimePaddingType(padding);
  data->params.padding_values.width = padding_values.width;
  data->params.padding_values.height = padding_values.height;

  // Note that quantized inference requires that all tensors have their
  // parameters set. This is usually done during quantized training.
  if (data_type != kTfLiteFloat32) {
    MicroContext* micro_context = GetMicroContext(context);

    TfLiteTensor* input =
        micro_context->AllocateTempInputTensor(node, kInputTensor);
    TF_LITE_ENSURE(context, input != nullptr);
    TfLiteTensor* filter =
        micro_context->AllocateTempInputTensor(node, kFilterTensor);
    TF_LITE_ENSURE(context, filter != nullptr);
    TfLiteTensor* bias =
        micro_context->AllocateTempInputTensor(node, kBiasTensor);
    TfLiteTensor* output =
        micro_context->AllocateTempOutputTensor(node, kOutputTensor);
    TF_LITE_ENSURE(context, output != nullptr);
    int output_channels = filter->dims->data[kConvQuantizedDimension];

    TF_LITE_ENSURE_STATUS(tflite::PopulateConvolutionQuantizationParams(
        context, input, filter, bias, output, kTfLiteActNone,
        &data->params.output_multiplier, &data->params.output_shift,
        &data->params.quantized_activation_min,
        &data->params.quantized_activation_max,
        data->per_channel_output_multiplier, data->per_channel_output_shift,
        output_channels));

    // TODO(b/192090531): Remove this once all 8x16 transpose conv models use
    // 64-bit biases.
    if (input->type == kTfLiteInt16) {
      TFLITE_DCHECK(filter->type == kTfLiteInt8);
      TFLITE_DCHECK(output->type == kTfLiteInt16);
      if (bias->type == kTfLiteInt16) {
        TFLITE_DCHECK(
            context->RequestScratchBufferInArena(
                context, GetTensorShape(bias).FlatSize() * sizeof(std::int64_t),
                &(data->bias_converted_buffer_index)) == kTfLiteOk);
      }
    }

    micro_context->DeallocateTempTfLiteTensor(input);
    micro_context->DeallocateTempTfLiteTensor(filter);
    micro_context->DeallocateTempTfLiteTensor(output);
    if (bias != nullptr) {
      micro_context->DeallocateTempTfLiteTensor(bias);
    }
  }
  return kTfLiteOk;
}

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  OpData* data = static_cast<OpData*>(node->user_data);
  const auto params =
      static_cast<const TfLiteTransposeConvParams*>(node->builtin_data);

  MicroContext* micro_context = GetMicroContext(context);

  TfLiteTensor* output =
      micro_context->AllocateTempOutputTensor(node, kOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);
  TfLiteTensor* input =
      micro_context->AllocateTempInputTensor(node, kInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  TfLiteTensor* filter =
      micro_context->AllocateTempInputTensor(node, kFilterTensor);
  TF_LITE_ENSURE(context, filter != nullptr);

  TF_LITE_ENSURE_MSG(
      context,
      input->type == filter->type ||
          (input->type == kTfLiteInt16 && filter->type == kTfLiteInt8),
      "Hybrid models are not supported on TFLite Micro.");

  // Get height and width of the output.
  const int width = SizeOfDimension(output, 2);
  const int height = SizeOfDimension(output, 1);
  const int filter_width = SizeOfDimension(filter, 2);
  const int filter_height = SizeOfDimension(filter, 1);

  // Dynamically allocate per-channel quantization parameters.
  const int num_channels = filter->dims->data[kConvQuantizedDimension];
  data->per_channel_output_multiplier =
      static_cast<int32_t*>(context->AllocatePersistentBuffer(
          context, num_channels * sizeof(int32_t)));
  data->per_channel_output_shift =
      static_cast<int32_t*>(context->AllocatePersistentBuffer(
          context, num_channels * sizeof(int32_t)));

  // Quantized kernels use an int32 scratch buffer.
  if (input->type == kTfLiteInt8) {
    TFLITE_DCHECK(context->RequestScratchBufferInArena != nullptr);
    TFLITE_DCHECK(context->RequestScratchBufferInArena(
                      context,
                      GetTensorShape(output).FlatSize() * sizeof(int32_t),
                      &(data->scratch_buffer_index)) == kTfLiteOk);
  }

  // Quantized 16x8 kernels use an int64 scratch buffer.
  if (input->type == kTfLiteInt16) {
    TFLITE_DCHECK(context->RequestScratchBufferInArena != nullptr);
    TFLITE_DCHECK(context->RequestScratchBufferInArena(
                      context,
                      GetTensorShape(output).FlatSize() * sizeof(std::int64_t),
                      &(data->scratch_buffer_index)) == kTfLiteOk);
  }

  // All per-channel quantized tensors need valid zero point and scale arrays.
  if (input->type == kTfLiteInt8 || input->type == kTfLiteInt16) {
    TF_LITE_ENSURE_EQ(context, filter->quantization.type,
                      kTfLiteAffineQuantization);

    const auto* affine_quantization =
        static_cast<TfLiteAffineQuantization*>(filter->quantization.params);
    TF_LITE_ENSURE(context, affine_quantization);
    TF_LITE_ENSURE(context, affine_quantization->scale);
    TF_LITE_ENSURE(context, affine_quantization->zero_point);

    TF_LITE_ENSURE(context,
                   affine_quantization->scale->size == 1 ||
                       affine_quantization->scale->size ==
                           filter->dims->data[kConvQuantizedDimension]);
    TF_LITE_ENSURE_EQ(context, affine_quantization->scale->size,
                      affine_quantization->zero_point->size);
  }

  TF_LITE_ENSURE_STATUS(CalculateOpData(context, node, params, width, height,
                                        filter_width, filter_height,
                                        input->type, data));

  // Offsets (zero points)
  data->params.input_offset = -input->params.zero_point;
  data->params.weights_offset = -filter->params.zero_point;
  data->params.output_offset = output->params.zero_point;

  // Stride
  data->params.stride_width = params->stride_width;
  data->params.stride_height = params->stride_height;

  micro_context->DeallocateTempTfLiteTensor(output);
  micro_context->DeallocateTempTfLiteTensor(input);
  micro_context->DeallocateTempTfLiteTensor(filter);
  return kTfLiteOk;
}

// TfLiteStatus EvalQuantizedPerChannel_deconv(
//                           const ConvParams& params, int32_t* output_multiplier,
//                           int32_t* output_shift, const RuntimeShape& input_shape,
//                           const int8_t* input_data, const RuntimeShape& filter_shape,
//                           int8_t* filter_data, const RuntimeShape& bias_shape,
//                           const int32_t* bias_data, const RuntimeShape& output_shape,
//                           int8_t* output_data) {
//   cmsis_nn_conv_params conv_params;
//   conv_params.dilation.h = 1;
//   conv_params.dilation.w = 1;

//   // Initialize cmsis_nn convolution parameters
//   conv_params.input_offset = -params.input_offset;
//   conv_params.output_offset = params.output_offset;
//   conv_params.stride.h = params.stride_height;
//   conv_params.stride.w = params.stride_width;
//   conv_params.padding.h = params.padding_values.height;
//   conv_params.padding.w = params.padding_values.width;
//   conv_params.activation.min = std::numeric_limits<int8_t>::min();
//   conv_params.activation.max = std::numeric_limits<int8_t>::max();

//   // Initialize cmsis_nn per channel quantization parameters
//   cmsis_nn_per_channel_quant_params quant_params;
//   quant_params.multiplier = output_multiplier;
//   quant_params.shift = output_shift;

//   // Consistency check.
//   TFLITE_DCHECK_LE(conv_params.activation.min, conv_params.activation.max);
//   TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);
//   TFLITE_DCHECK_EQ(filter_shape.DimensionsCount(), 4);
//   TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);

//   const int batch_size = MatchingDim(input_shape, 0, output_shape, 0);
//   const int input_depth = MatchingDim(input_shape, 3, filter_shape, 3);
//   const int output_depth = MatchingDim(filter_shape, 0, output_shape, 3);
//   if (bias_data) {
//     TFLITE_DCHECK_EQ(bias_shape.FlatSize(), output_depth);
//   }
//   // Initialize cmsis_nn dimensions
//   // Input
//   cmsis_nn_dims input_dims;
//   input_dims.n = batch_size;
//   input_dims.h = input_shape.Dims(1);
//   input_dims.w = input_shape.Dims(2);
//   input_dims.c = input_depth;

//   // Filter
//   cmsis_nn_dims filter_dims;
//   filter_dims.n = output_depth;
//   filter_dims.h = filter_shape.Dims(1);
//   filter_dims.w = filter_shape.Dims(2);
//   filter_dims.c = input_depth;

//   // Bias
//   cmsis_nn_dims bias_dims;
//   bias_dims.n = 1;
//   bias_dims.h = 1;
//   bias_dims.w = 1;
//   bias_dims.c = output_depth;

//   // Output
//   cmsis_nn_dims output_dims;
//   output_dims.n = batch_size;
//   output_dims.h = output_shape.Dims(1);
//   output_dims.w = output_shape.Dims(2);
//   output_dims.c = output_depth;

//   // Initialize cmsis_nn context
//   cmsis_nn_context ctx;
//   int32_t buf_size = arm_convolve_s8_get_buffer_size(&input_dims, &filter_dims);
//   ctx.buf = malloc(buf_size);
//   ctx.size = 0;
//   TFLITE_DCHECK_EQ(
//      arm_deconvolve_s8(&ctx, &conv_params, &quant_params, 
//                        &input_dims, input_data,
//                        &filter_dims, filter_data,
//                        &bias_dims, bias_data,
//                        &output_dims, output_data),
//       ARM_MATH_SUCCESS);
//   if (ctx.buf)
//     {
//       free(ctx.buf);
//     }
//   return kTfLiteOk;
// }

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kFilterTensor);
  const TfLiteEvalTensor* bias =
      (NumInputs(node) == 4)
          ? tflite::micro::GetEvalInput(context, node, kBiasTensor)
          : nullptr;
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  TF_LITE_ENSURE_EQ(context, input->type, output->type);

  switch (input->type) {  // Already know in/out types are same.
    case kTfLiteFloat32: {
      const auto& params =
          *(reinterpret_cast<TfLiteConvParams*>(node->builtin_data));
      ConvParams op_params = data.params;
      CalculateActivationRange(params.activation,
                               &op_params.float_activation_min,
                               &op_params.float_activation_max);

      reference_ops::TransposeConv(
          op_params, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<float>(input),
          tflite::micro::GetTensorShape(filter),
          tflite::micro::GetTensorData<float>(filter),
          tflite::micro::GetTensorShape(bias),
          tflite::micro::GetOptionalTensorData<float>(bias),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<float>(output),
          tflite::micro::GetTensorShape(nullptr), nullptr);
      break;
    }
    case kTfLiteInt8: {
      int deconv_output_width = output->dims->data[2];
      // if (deconv_output_width == 1){
      //   EvalQuantizedPerChannel_deconv(
      //         data.params, data.per_channel_output_multiplier,
      //         data.per_channel_output_shift, 
      //         tflite::micro::GetTensorShape(input),
      //         tflite::micro::GetTensorData<int8_t>(input), 
      //         tflite::micro::GetTensorShape(filter),
      //         const_cast<int8_t*>(tflite::micro::GetTensorData<int8_t>(filter)), 
      //         tflite::micro::GetTensorShape(bias),
      //         tflite::micro::GetOptionalTensorData<int32_t>(bias),
      //         tflite::micro::GetTensorShape(output),
      //         tflite::micro::GetTensorData<int8_t>(output));
      // }
      // else{
      //   int32_t* scratch_buffer = static_cast<int32_t*>(
      //     context->GetScratchBuffer(context, data.scratch_buffer_index));
      //   reference_integer_ops::TransposeConv(
      //     data.params, data.per_channel_output_multiplier,
      //     data.per_channel_output_shift, tflite::micro::GetTensorShape(input),
      //     tflite::micro::GetTensorData<int8_t>(input),
      //     tflite::micro::GetTensorShape(filter),
      //     tflite::micro::GetTensorData<int8_t>(filter),
      //     tflite::micro::GetTensorShape(bias),
      //     tflite::micro::GetOptionalTensorData<int32_t>(bias),
      //     tflite::micro::GetTensorShape(output),
      //     tflite::micro::GetTensorData<int8_t>(output),
      //     tflite::micro::GetTensorShape(nullptr), nullptr, scratch_buffer);
      // }
      int32_t* scratch_buffer = static_cast<int32_t*>(
        context->GetScratchBuffer(context, data.scratch_buffer_index));
      reference_integer_ops::TransposeConv(
        data.params, data.per_channel_output_multiplier,
        data.per_channel_output_shift, tflite::micro::GetTensorShape(input),
        tflite::micro::GetTensorData<int8_t>(input),
        tflite::micro::GetTensorShape(filter),
        tflite::micro::GetTensorData<int8_t>(filter),
        tflite::micro::GetTensorShape(bias),
        tflite::micro::GetOptionalTensorData<int32_t>(bias),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int8_t>(output),
        tflite::micro::GetTensorShape(nullptr), nullptr, scratch_buffer);
      break;
    }
    case kTfLiteInt16: {
      std::int64_t* scratch_buffer = static_cast<int64_t*>(
          context->GetScratchBuffer(context, data.scratch_buffer_index));
      // TODO(b/192090531): Remove this once all 8x16 transpose conv models use
      // 64-bit biases.
      if (bias != nullptr && bias->type == kTfLiteInt16) {
        std::int64_t* bias_converted_buffer =
            static_cast<int64_t*>(context->GetScratchBuffer(
                context, data.bias_converted_buffer_index));
        for (int i = 0; i < tflite::micro::GetTensorShape(bias).FlatSize();
             i++) {
          bias_converted_buffer[i] = bias->data.i16[i];
        }
        reference_integer_ops::TransposeConv(
            data.params, data.per_channel_output_multiplier,
            data.per_channel_output_shift, tflite::micro::GetTensorShape(input),
            tflite::micro::GetTensorData<int16_t>(input),
            tflite::micro::GetTensorShape(filter),
            tflite::micro::GetTensorData<int8_t>(filter),
            tflite::micro::GetTensorShape(bias), bias_converted_buffer,
            tflite::micro::GetTensorShape(output),
            tflite::micro::GetTensorData<int16_t>(output),
            tflite::micro::GetTensorShape(nullptr), nullptr, scratch_buffer);
      } else {
        reference_integer_ops::TransposeConv(
            data.params, data.per_channel_output_multiplier,
            data.per_channel_output_shift, tflite::micro::GetTensorShape(input),
            tflite::micro::GetTensorData<int16_t>(input),
            tflite::micro::GetTensorShape(filter),
            tflite::micro::GetTensorData<int8_t>(filter),
            tflite::micro::GetTensorShape(bias),
            tflite::micro::GetOptionalTensorData<std::int64_t>(bias),
            tflite::micro::GetTensorShape(output),
            tflite::micro::GetTensorData<int16_t>(output),
            tflite::micro::GetTensorShape(nullptr), nullptr, scratch_buffer);
      }
      break;
    }
    default:
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
  }
  return kTfLiteOk;
}

}  // namespace

TfLiteRegistration_V1 Register_TRANSPOSE_CONV() {
  return tflite::micro::RegisterOp(Init, Prepare, Eval);
}

}  // namespace tflite
