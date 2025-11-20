
#include "tensorflow/lite/micro/kernels/depthwise_conv.h"

#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"
#include "beco_bias.hpp"
#include "beco_nn/beco_nnfunctions.h"

#include "arm_math.h"
#include "arm_nnfunctions.h"
#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/common.h"
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/kernels/internal/reference/depthwiseconv_float.h"
#include "tensorflow/lite/kernels/internal/reference/integer_ops/depthwise_conv.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/kernels/padding.h"
#include "tensorflow/lite/micro/kernels/conv.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "hal_trace.h"

namespace tflite {
namespace {

struct OpData {
  OpDataConv reference_op_data;

  // Index to buffer for optimizations if applicable.
  int buffer_idx;

  int* kernel_sum;
};

// Always inline for optimal code size.
void PopulateDwConvParams(
    cmsis_nn_dw_conv_params* const dw_conv_params,
    cmsis_nn_per_channel_quant_params* const quant_params,
    cmsis_nn_dims* const input_dims, cmsis_nn_dims* const filter_dims,
    cmsis_nn_dims* const bias_dims, cmsis_nn_dims* const output_dims,
    const TfLiteDepthwiseConvParams& params, const OpData& data,
    const TfLiteEvalTensor* input, const TfLiteEvalTensor* filter,
    const TfLiteEvalTensor* bias, TfLiteEvalTensor* output)
    __attribute__((always_inline));

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

static void add_input_offset_2_bias(int32_t *bias,
                                    int8_t *weight,
                                    int input_offset,
                                    int input_ch,
                                    int kernel_h,
                                    int kernel_w)
{
    for (int i = 0; i < input_ch; i++) {
        int sum = 0;
        for (int j = 0; j < kernel_h; j++) {
            for (int k = 0; k < kernel_w; k++) {
                sum += weight[i*kernel_h*kernel_w + j*kernel_w + k] * input_offset;
            }
        }
        bias[i] += sum;
    }
}

template<typename T>
static void HWC_2_CHW(const T* src_data, T* dst_data, int h, int w, int channel)
{
    for(int i = 0; i < channel; i++){
        for(int j = 0; j < h; j++){
            for(int k = 0; k < w; k++){
              *dst_data++ = src_data[channel*w*j+channel*k+i];
            }
        }
    }
}

template<typename T>
static void CHW_2_HWC(const T* src_data, T* dst_data, int h, int w, int channel)
{
    for(int i = 0; i < h; i++){
        for(int j = 0; j < w; j++){
            for(int k = 0; k < channel; k++){
              *dst_data++ = src_data[h*w*k+w*i+j];
            }
        }
    }
}


TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  OpData* data = static_cast<OpData*>(node->user_data);
  const auto& params =
      *(reinterpret_cast<TfLiteDepthwiseConvParams*>(node->builtin_data));

  MicroContext* micro_context = GetMicroContext(context);

  TfLiteTensor* input =
      micro_context->AllocateTempInputTensor(node, kDepthwiseConvInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  TfLiteTensor* filter =
      micro_context->AllocateTempInputTensor(node, kDepthwiseConvWeightsTensor);
  TF_LITE_ENSURE(context, filter != nullptr);
  TfLiteTensor* output =
      micro_context->AllocateTempOutputTensor(node, kDepthwiseConvOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);
  TfLiteTensor* bias =
      micro_context->AllocateTempInputTensor(node, kDepthwiseConvBiasTensor);
  TF_LITE_ENSURE(context, bias != nullptr);
//     const TfLiteEvalTensor* bias =
//       (NumInputs(node) == 3)
//           ? tflite::micro::GetEvalInput(context, node, kDepthwiseConvBiasTensor)
//           : nullptr;

  const TfLiteType data_type = input->type;
  int input_width = SizeOfDimension(input, 2);
  int input_height = SizeOfDimension(input, 1);
  int filter_width = SizeOfDimension(filter, 2);
  int filter_height = SizeOfDimension(filter, 1);
  int output_width = SizeOfDimension(output, 2);
  int output_height = SizeOfDimension(output, 1);
  int output_channel = SizeOfDimension(output, 3);

//   修改权重形状以及bias的值

  int size_filter = filter_width*filter_height*output_channel;
  int8_t* new_filter_data = new int8_t[size_filter];
  int8_t* filter_data = filter->data.int8;

  HWC_2_CHW(filter_data, new_filter_data, filter_height, filter_width, output_channel);

  memmove(filter_data, new_filter_data, size_filter);
  delete[] new_filter_data;

  if (input->type == kTfLiteInt8){
    int zp = input->params.zero_point;                     
    if (bias != nullptr){
      int32_t* bias_data = bias->data.i32;
      if (zp != 0)
      {
        add_input_offset_2_bias(bias_data, filter_data, -zp, output_channel, filter_height, filter_width);
      }
    }
    else{
      if (zp != 0){
        int32_t* new_bias_data = new int32_t[output_channel];
        memset(new_bias_data, 0, 4*output_channel);
        add_input_offset_2_bias(new_bias_data, filter_data, -zp, output_channel, filter_height, filter_width);
        data->kernel_sum = new_bias_data;
      }
    }   
  }
  if (input->type == kTfLiteInt16){
    if (bias != nullptr){
      int64_t* bias_data = bias->data.i64;
      int32_t* new_bias_i32 = new int32_t[output_channel];
      for(int i = 0; i < output_channel; i++)
      {
        new_bias_i32[i] = (int32_t)bias_data[i];
      }
      memcpy((int8_t*)bias_data, (int8_t*)new_bias_i32, 4*output_channel);
      delete[] new_bias_i32;
    }
  }


//
  if (input->type == kTfLiteInt8 || input->type == kTfLiteInt16) {
    TF_LITE_ENSURE_EQ(context, filter->quantization.type,
                      kTfLiteAffineQuantization);

    if (input->type == kTfLiteInt16) {
      TF_LITE_ENSURE_EQ(context, input->params.zero_point, 0);
      TF_LITE_ENSURE_EQ(context, output->params.zero_point, 0);
    }

    // All per-channel quantized tensors need valid zero point and scale arrays.
    const auto* affine_quantization =
        reinterpret_cast<TfLiteAffineQuantization*>(
            filter->quantization.params);
    TF_LITE_ENSURE(context, affine_quantization);
    TF_LITE_ENSURE(context, affine_quantization->scale);
    TF_LITE_ENSURE(context, affine_quantization->zero_point);
    TF_LITE_ENSURE(
        context, affine_quantization->scale->size == 1 ||
                     affine_quantization->scale->size ==
                         filter->dims->data[kDepthwiseConvQuantizedDimension]);
    TF_LITE_ENSURE_EQ(context, affine_quantization->scale->size,
                      affine_quantization->zero_point->size);

    // Allocate memory for per-channel quantization parameters
    const int num_channels =
        filter->dims->data[kDepthwiseConvQuantizedDimension];

    data->reference_op_data.per_channel_output_multiplier =
        reinterpret_cast<int32_t*>(context->AllocatePersistentBuffer(
            context, num_channels * sizeof(int32_t)));
    data->reference_op_data.per_channel_output_shift =
        reinterpret_cast<int32_t*>(context->AllocatePersistentBuffer(
            context, num_channels * sizeof(int32_t)));
  }

  TF_LITE_ENSURE_STATUS(CalculateOpDataDepthwiseConv(
      context, node, params, input_width, input_height, filter_width,
      filter_height, output_width, output_height, data_type,
      &data->reference_op_data));

  if (input->type == kTfLiteInt8) {
    RuntimeShape input_shape = GetTensorShape(input);
    RuntimeShape output_shape = GetTensorShape(output);
    RuntimeShape filter_shape = GetTensorShape(filter);
    TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);
    TFLITE_DCHECK_EQ(filter_shape.DimensionsCount(), 4);
    TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);

    const int batch_size = MatchingDim(input_shape, 0, output_shape, 0);
    const int output_depth = MatchingDim(output_shape, 3, filter_shape, 3);
    TFLITE_DCHECK_EQ(batch_size, 1); /* Only batch = 1 is supported */

    cmsis_nn_dims input_dims;
    input_dims.n = batch_size;
    input_dims.h = input_height;
    input_dims.w = input_width;
    input_dims.c = input_shape.Dims(3);

    cmsis_nn_dims filter_dims;
    filter_dims.n = 1;
    filter_dims.h = filter_height;
    filter_dims.w = filter_width;
    filter_dims.c = output_depth;

    cmsis_nn_dims output_dims;
    output_dims.n = batch_size;
    output_dims.h = output_height;
    output_dims.w = output_width;
    output_dims.c = output_depth;

    cmsis_nn_dw_conv_params dw_conv_params;
    dw_conv_params.padding.h = data->reference_op_data.padding.height;
    dw_conv_params.padding.w = data->reference_op_data.padding.width;
    dw_conv_params.dilation.h = params.dilation_height_factor;
    dw_conv_params.dilation.w = params.dilation_width_factor;

    int buf_size = 0;
    if (buf_size > 0) {
      TF_LITE_ENSURE_STATUS(context->RequestScratchBufferInArena(
          context, buf_size, &data->buffer_idx));
    } else {
      data->buffer_idx = -1;
    }
  }

  micro_context->DeallocateTempTfLiteTensor(output);
  micro_context->DeallocateTempTfLiteTensor(input);
  micro_context->DeallocateTempTfLiteTensor(filter);
  if (bias != nullptr) {
    micro_context->DeallocateTempTfLiteTensor(bias);
  }

  return kTfLiteOk;
}

inline void PopulateDwConvParams(
    cmsis_nn_dw_conv_params* const dw_conv_params,
    cmsis_nn_per_channel_quant_params* const quant_params,
    cmsis_nn_dims* const input_dims, cmsis_nn_dims* const filter_dims,
    cmsis_nn_dims* const bias_dims, cmsis_nn_dims* const output_dims,
    const TfLiteDepthwiseConvParams& params, const OpData& data,
    const TfLiteEvalTensor* input, const TfLiteEvalTensor* filter,
    const TfLiteEvalTensor* bias, TfLiteEvalTensor* output) {
  dw_conv_params->dilation.h = params.dilation_height_factor;
  dw_conv_params->dilation.w = params.dilation_width_factor;

  dw_conv_params->input_offset = -data.reference_op_data.input_zero_point;
  dw_conv_params->output_offset = data.reference_op_data.output_zero_point;
  dw_conv_params->stride.h = params.stride_height;
  dw_conv_params->stride.w = params.stride_width;
  dw_conv_params->padding.h = data.reference_op_data.padding.height;
  dw_conv_params->padding.w = data.reference_op_data.padding.width;

  dw_conv_params->activation.min = data.reference_op_data.output_activation_min;
  dw_conv_params->activation.max = data.reference_op_data.output_activation_max;

  dw_conv_params->ch_mult = params.depth_multiplier;

  quant_params->multiplier =
      data.reference_op_data.per_channel_output_multiplier;
  quant_params->shift = data.reference_op_data.per_channel_output_shift;
  
  RuntimeShape filter_shape = tflite::micro::GetTensorShape(filter);
  RuntimeShape input_shape = tflite::micro::GetTensorShape(input);
  RuntimeShape output_shape = tflite::micro::GetTensorShape(output);
  RuntimeShape bias_shape = tflite::micro::GetTensorShape(bias);

  TFLITE_DCHECK_LE(dw_conv_params->activation.min,
                   dw_conv_params->activation.max);

  const int batch_size = MatchingDim(input_shape, 0, output_shape, 0);
  const int output_depth = MatchingDim(filter_shape, 3, output_shape, 3);

  if (tflite::micro::GetOptionalTensorData<int8_t>(bias)) {
    TFLITE_DCHECK_EQ(bias_shape.FlatSize(), output_depth);
  }

  input_dims->n = batch_size;
  input_dims->h = input_shape.Dims(1);
  input_dims->w = input_shape.Dims(2);
  input_dims->c = input_shape.Dims(3);

  filter_dims->n = filter_shape.Dims(0);
  filter_dims->h = filter_shape.Dims(1);
  filter_dims->w = filter_shape.Dims(2);
  filter_dims->c = output_depth;

  bias_dims->n = 1;
  bias_dims->h = 1;
  bias_dims->w = 1;
  bias_dims->c = output_depth;

  output_dims->n = batch_size;
  output_dims->h = output_shape.Dims(1);
  output_dims->w = output_shape.Dims(2);
  output_dims->c = output_depth;

}

void EvalQuantizedPerChannel(TfLiteContext* context, TfLiteNode* node,
                             const TfLiteDepthwiseConvParams& params,
                             const OpData& data, const TfLiteEvalTensor* input,
                             const TfLiteEvalTensor* filter,
                             const TfLiteEvalTensor* bias,
                             TfLiteEvalTensor* output) {
  cmsis_nn_dw_conv_params dw_conv_params;
  cmsis_nn_per_channel_quant_params quant_params;
  cmsis_nn_dims input_dims;
  cmsis_nn_dims filter_dims;
  cmsis_nn_dims bias_dims;
  cmsis_nn_dims output_dims;

  PopulateDwConvParams(&dw_conv_params, &quant_params, &input_dims,
                       &filter_dims, &bias_dims, &output_dims, params, data,
                       input, filter, bias, output);

  cmsis_nn_context ctx;
  ctx.buf = nullptr;
  /* 'size' is unused */
  ctx.size = 0;

  if (data.buffer_idx > -1) {
    ctx.buf = context->GetScratchBuffer(context, data.buffer_idx);
  }

  const int32_t* bias_data = tflite::micro::GetOptionalTensorData<int32_t>(bias);

  if (bias_data == nullptr && data.reference_op_data.input_zero_point != 0)
  {
    bias_data = data.kernel_sum;
  }

  int size_input = input_dims.h*input_dims.w*input_dims.c;
  int8_t* new_input_data = new int8_t[size_input];
  const signed char* input_data = tflite::micro::GetTensorData<int8_t>(input);
  HWC_2_CHW(input_data, new_input_data, input_dims.h, input_dims.w, input_dims.c);
  memmove((signed char*)input_data, new_input_data, size_input);
  delete[] new_input_data;

  TFLITE_DCHECK_EQ(
      beco_depthwise_conv_3x3_s8(
          &ctx, &dw_conv_params, &quant_params, &input_dims,
          tflite::micro::GetTensorData<int8_t>(input), &filter_dims,
          tflite::micro::GetTensorData<int8_t>(filter), &bias_dims,
          bias_data, &output_dims,
          tflite::micro::GetTensorData<int8_t>(output)),
      ARM_CMSIS_NN_SUCCESS);

  if (bias_data == nullptr && data.reference_op_data.input_zero_point != 0 && data.kernel_sum != nullptr)
  {
    delete[] data.kernel_sum;
  }

  int size_output = output_dims.h*output_dims.w*output_dims.c;
  int8_t* new_output_data = new int8_t[size_output];
  signed char* output_data = tflite::micro::GetTensorData<int8_t>(output);
  CHW_2_HWC(output_data, new_output_data, output_dims.h, output_dims.w, output_dims.c);
  memmove(output_data, new_output_data, size_output);
  delete[] new_output_data;

}

void EvalQuantizedPerChannel16x8(TfLiteContext* context, TfLiteNode* node,
                                 const TfLiteDepthwiseConvParams& params,
                                 const OpData& data,
                                 const TfLiteEvalTensor* input,
                                 const TfLiteEvalTensor* filter,
                                 const TfLiteEvalTensor* bias,
                                 TfLiteEvalTensor* output) {
  cmsis_nn_dw_conv_params dw_conv_params;
  cmsis_nn_per_channel_quant_params quant_params;
  cmsis_nn_dims input_dims;
  cmsis_nn_dims filter_dims;
  cmsis_nn_dims bias_dims;
  cmsis_nn_dims output_dims;

  PopulateDwConvParams(&dw_conv_params, &quant_params, &input_dims,
                       &filter_dims, &bias_dims, &output_dims, params, data,
                       input, filter, bias, output);

  cmsis_nn_context ctx;
  ctx.buf = nullptr;
  /* 'size' is unused */
  ctx.size = 0;

  const int32_t* bias_data = tflite::micro::GetOptionalTensorData<int32_t>(bias);

  int size_input = input_dims.h*input_dims.w*input_dims.c;
  int16_t* new_input_data = new int16_t[size_input];
  const short int* input_data = tflite::micro::GetTensorData<int16_t>(input);
  HWC_2_CHW(input_data, new_input_data, input_dims.h, input_dims.w, input_dims.c);
  memmove((int8_t*)input_data, (int8_t*)new_input_data, 2*size_input);
  delete[] new_input_data;

  TFLITE_DCHECK_EQ(
      beco_depthwise_conv_3x3_s16(
          &ctx, &dw_conv_params, &quant_params, &input_dims,
          tflite::micro::GetTensorData<int16_t>(input), &filter_dims,
          tflite::micro::GetTensorData<int8_t>(filter), &bias_dims,
          bias_data, &output_dims,
          tflite::micro::GetTensorData<int16_t>(output)),
      ARM_CMSIS_NN_SUCCESS);

  int size_output = output_dims.h*output_dims.w*output_dims.c;
  int16_t* new_output_data = new int16_t[size_output];
  short int* output_data = tflite::micro::GetTensorData<int16_t>(output);
  CHW_2_HWC(output_data, new_output_data, output_dims.h, output_dims.w, output_dims.c);
  memmove((int8_t*)output_data, (int8_t*)new_output_data, 2*size_output);
  delete[] new_output_data;
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  const auto& params =
      *(reinterpret_cast<TfLiteDepthwiseConvParams*>(node->builtin_data));
  const OpData& data = *(static_cast<OpData*>(node->user_data));

  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kDepthwiseConvOutputTensor);
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kDepthwiseConvInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kDepthwiseConvWeightsTensor);
  const TfLiteEvalTensor* bias =
      (NumInputs(node) == 3)
          ? tflite::micro::GetEvalInput(context, node, kDepthwiseConvBiasTensor)
          : nullptr;

  TfLiteEvalTensor filter_int8 = tflite::micro::MakeUnpackedInt4Tensor(
      context, data.reference_op_data.filter_buffer_index, filter);

  switch (input->type) {  // Already know in/out types are same.
    case kTfLiteFloat32: {
      tflite::reference_ops::DepthwiseConv(
          DepthwiseConvParamsFloat(params, data.reference_op_data),
          tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<float>(input),
          tflite::micro::GetTensorShape(filter),
          tflite::micro::GetTensorData<float>(filter),
          tflite::micro::GetTensorShape(bias),
          tflite::micro::GetOptionalTensorData<float>(bias),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<float>(output));
      break;
    }
    case kTfLiteInt8:
      switch (filter_int8.type) {
        case kTfLiteInt8: {
          EvalQuantizedPerChannel(context, node, params, data, input,
                                  &filter_int8, bias, output);
          break;
        }
        default: {
          MicroPrintf("Filter type %s (%d) not supported.",
                      TfLiteTypeGetName(filter->type), filter->type);
          return kTfLiteError;
        }
      }
      break;
    case kTfLiteInt16:
      EvalQuantizedPerChannel16x8(context, node, params, data, input, filter,
                                  bias, output);
      break;
    default:
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
  }
  return kTfLiteOk;
}

TfLiteStatus EvalInt8(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  const auto& params =
      *(reinterpret_cast<TfLiteDepthwiseConvParams*>(node->builtin_data));
  const OpData& data = *(static_cast<OpData*>(node->user_data));

  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kDepthwiseConvOutputTensor);
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kDepthwiseConvInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kDepthwiseConvWeightsTensor);
  const TfLiteEvalTensor* bias =
      (NumInputs(node) == 3)
          ? tflite::micro::GetEvalInput(context, node, kDepthwiseConvBiasTensor)
          : nullptr;

  TfLiteEvalTensor filter_int8 = tflite::micro::MakeUnpackedInt4Tensor(
      context, data.reference_op_data.filter_buffer_index, filter);

  EvalQuantizedPerChannel(context, node, params, data, input, &filter_int8,
                          bias, output);
  return kTfLiteOk;
}

TfLiteStatus EvalInt16x8(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  const auto& params =
      *(reinterpret_cast<TfLiteDepthwiseConvParams*>(node->builtin_data));
  const OpData& data = *(static_cast<OpData*>(node->user_data));

  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kDepthwiseConvOutputTensor);
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kDepthwiseConvInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kDepthwiseConvWeightsTensor);
  const TfLiteEvalTensor* bias =
      (NumInputs(node) == 3)
          ? tflite::micro::GetEvalInput(context, node, kDepthwiseConvBiasTensor)
          : nullptr;

  EvalQuantizedPerChannel16x8(context, node, params, data, input, filter, bias,
                              output);
  return kTfLiteOk;
}

}  // namespace

TfLiteRegistration_V1 Register_DEPTHWISE_CONV_2D() {
  return tflite::micro::RegisterOp(Init, Prepare, Eval);
}

TfLiteRegistration_V1 Register_DEPTHWISE_CONV_2D_INT8() {
  return tflite::micro::RegisterOp(Init, Prepare, EvalInt8);
}

TfLiteRegistration_V1 Register_DEPTHWISE_CONV_2D_INT16() {
  return tflite::micro::RegisterOp(Init, Prepare, EvalInt16x8);
}

}  // namespace tflite
