
#include "tensorflow/lite/micro/kernels/conv.h"

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
#include "tensorflow/lite/kernels/internal/reference/conv.h"
#include "tensorflow/lite/kernels/internal/reference/integer_ops/conv.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/kernels/padding.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"
#include "tensorflow/lite/micro/micro_log.h"

#include "tensorflow/lite/kernels/internal/reference/transpose.h"
#include "hal_trace.h"
namespace tflite {
namespace {

struct OpData {
  OpDataConv reference_op_data;

  // Index to buffer for optimizations if applicable.
  int buffer_idx;
  int32_t* kernel_sum;
};

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

void CoutHWCin_2_CinHWCout(const int8_t* src_data, int8_t* dst_data, int h, int w, int in_channel, int out_channel)
{
    for(int i = 0; i < in_channel; i++){
        for(int j = 0; j < h; j++){
            for(int k = 0; k < w; k++){
                for(int m = 0; m < out_channel; m++){
                    *dst_data++ = src_data[m*h*w*in_channel + j*w*in_channel + k*in_channel + i];
                }
            }
        }
    }
}

static void add_input_offset_2_bias_conv(int32_t *bias,
                                         int8_t *weight,
                                         int input_offset,
                                         int input_ch,
                                         int output_ch,
                                         int kernel_h,
                                         int kernel_w)
{
    for (int i = 0; i < output_ch; i++) {
        int sum = 0;
        for (int j = 0; j < input_ch; j++) {
            for (int k = 0; k < kernel_h; k++) {
                for (int l = 0; l < kernel_w; l++) {
                    sum += weight[j*kernel_h*kernel_w*output_ch + k*kernel_w*output_ch + l*output_ch + i] * input_offset;
                }
            }
        }
        bias[i] += sum;
    }
}

static void add_input_offset_2_bias_1x1(int32_t *bias,
                                    int8_t *weight,
                                    int input_offset,
                                    int input_ch,
                                    int output_ch)
{
    for (int i = 0; i < output_ch; i++) {
        int sum = 0;
        for (int j = 0; j < input_ch; j++) {
            sum += weight[j*output_ch + i] * input_offset;
        }
        bias[i] += sum;
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

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  int32_t buf_size = 0;
  const auto& params =
      *(static_cast<const TfLiteConvParams*>(node->builtin_data));
  OpData* data = static_cast<OpData*>(node->user_data);

  MicroContext* micro_context = GetMicroContext(context);

  TfLiteTensor* input =
      micro_context->AllocateTempInputTensor(node, kConvInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  TfLiteTensor* filter =
      micro_context->AllocateTempInputTensor(node, kConvWeightsTensor);
  TF_LITE_ENSURE(context, filter != nullptr);
  TfLiteTensor* output =
      micro_context->AllocateTempOutputTensor(node, kConvOutputTensor);
  TfLiteTensor* bias =
      micro_context->AllocateTempInputTensor(node, kConvBiasTensor);
  TF_LITE_ENSURE(context, bias != nullptr);

  RuntimeShape input_shape = GetTensorShape(input);
  RuntimeShape output_shape = GetTensorShape(output);

//   修改权重形状以及bias的值
  int ICH = input_shape.Dims(3);
  int OCH = output_shape.Dims(3);
  int KY = filter->dims->data[1];
  int KX = filter->dims->data[2];

  if (KY == 1 && KX == 1)
  {
    int size_filter = ICH*OCH*KY*KX;
    int8_t* new_filter_data = new int8_t[size_filter];
    int8_t* filter_data = filter->data.int8;

    CoutHWCin_2_CinHWCout(filter_data, new_filter_data, KY, KX, ICH, OCH);

    memmove(filter_data, new_filter_data, size_filter);
    delete[] new_filter_data;

    if (input->type == kTfLiteInt8){
      int zp = input->params.zero_point;                             
      if (bias != nullptr){
          int32_t* bias_data = bias->data.i32;
          if (zp != 0)
          {
            add_input_offset_2_bias_1x1(bias_data, filter_data, -zp, ICH, OCH);
          }   
      }
      else{
        if (zp != 0){
          int32_t* new_bias_data = new int32_t[OCH];
          memset(new_bias_data, 0, 4*OCH);
          add_input_offset_2_bias_1x1(new_bias_data, filter_data, -zp, ICH, OCH);
          data->kernel_sum = new_bias_data;
        }
      }
    }
    if (input->type == kTfLiteInt16){
      if (bias != nullptr){
        int64_t* bias_data = bias->data.i64;
        int32_t* new_bias_i32 = new int32_t[OCH];
        for(int i = 0; i < OCH; i++)
        {
          new_bias_i32[i] = (int32_t)bias_data[i];
        }
        memcpy((int8_t*)bias_data, (int8_t*)new_bias_i32, 4*OCH);
        delete[] new_bias_i32;
      }
    }
  }
  else
  {
    int size_filter = ICH*OCH*KY*KX;
    int8_t* new_filter_data = new int8_t[size_filter];
    int8_t* filter_data = filter->data.int8;

    CoutHWCin_2_CinHWCout(filter_data, new_filter_data, KY, KX, ICH, OCH);

    memmove(filter_data, new_filter_data, size_filter);
    delete[] new_filter_data;

    if (input->type == kTfLiteInt8){
      int zp = input->params.zero_point;                     
      if (bias != nullptr){
        int32_t* bias_data = bias->data.i32;
        if (zp != 0)
        {
          add_input_offset_2_bias_conv(bias_data, filter_data, -zp, ICH, OCH, KY, KX);
        }
      }
      else{
        if (zp != 0){
          int32_t* new_bias_data = new int32_t[OCH];
          memset(new_bias_data, 0, 4*OCH);
          add_input_offset_2_bias_conv(new_bias_data, filter_data, -zp, ICH, OCH, KY, KX);
          data->kernel_sum = new_bias_data;
        }
      }   
    }
    if (input->type == kTfLiteInt16){
      if (bias != nullptr){
        int64_t* bias_data = bias->data.i64;
        int32_t* new_bias_i32 = new int32_t[OCH];
        for(int i = 0; i < OCH; i++)
        {
          new_bias_i32[i] = (int32_t)bias_data[i];
        }
        memcpy((int8_t*)bias_data, (int8_t*)new_bias_i32, 4*OCH);
        delete[] new_bias_i32;
      }
    }
  }

//

  // Initialize cmsis_nn input dimensions
  cmsis_nn_dims input_dims;
  input_dims.n = MatchingDim(input_shape, 0, output_shape, 0);
  input_dims.h = input->dims->data[1];
  input_dims.w = input->dims->data[2];
  input_dims.c = input_shape.Dims(3);

  // Initialize cmsis_nn filter dimensions
  cmsis_nn_dims filter_dims;
  filter_dims.n = output_shape.Dims(3);
  filter_dims.h = filter->dims->data[1];
  filter_dims.w = filter->dims->data[2];
  filter_dims.c = input_dims.c;

  // Initialize cmsis_nn output dimensions
  cmsis_nn_dims output_dims;
  output_dims.n = input_dims.n;
  output_dims.h = output->dims->data[1];
  output_dims.w = output->dims->data[2];
  output_dims.c = output_shape.Dims(3);


  if (input->type == kTfLiteInt8 || input->type == kTfLiteInt16) {
    const int num_channels = filter->dims->data[kConvQuantizedDimension];
    data->reference_op_data.per_channel_output_multiplier =
        static_cast<int32_t*>(context->AllocatePersistentBuffer(
            context, num_channels * sizeof(int32_t)));
    data->reference_op_data.per_channel_output_shift =
        static_cast<int32_t*>(context->AllocatePersistentBuffer(
            context, num_channels * sizeof(int32_t)));
  }

  TF_LITE_ENSURE_STATUS(CalculateOpDataConv(
      context, node, params, input_dims.w, input_dims.h, filter_dims.w,
      filter_dims.h, output_dims.w, output_dims.h, input->type,
      &data->reference_op_data));

  if (input->type == kTfLiteInt8 || input->type == kTfLiteInt16) {
    // Initialize cmsis_nn convolution parameters
    cmsis_nn_conv_params conv_params;
    conv_params.input_offset = -input->params.zero_point;
    conv_params.output_offset = output->params.zero_point;
    conv_params.stride.h = params.stride_height;
    conv_params.stride.w = params.stride_width;
    conv_params.dilation.h = params.dilation_height_factor;
    conv_params.dilation.w = params.dilation_width_factor;
    conv_params.padding.h = data->reference_op_data.padding.height;
    conv_params.padding.w = data->reference_op_data.padding.width;
    conv_params.activation.min = data->reference_op_data.output_activation_min;
    conv_params.activation.max = data->reference_op_data.output_activation_max;

    if (input->type == kTfLiteInt16) {
      TF_LITE_ENSURE_EQ(context, input->params.zero_point, 0);
      TF_LITE_ENSURE_EQ(context, output->params.zero_point, 0);
    }

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

TfLiteStatus EvalQuantizedPerChannel(TfLiteContext* context, TfLiteNode* node,
                                     const TfLiteConvParams& params,
                                     const OpData& data,
                                     const TfLiteEvalTensor* input,
                                     const TfLiteEvalTensor* filter,
                                     const TfLiteEvalTensor* bias,
                                     TfLiteEvalTensor* output) {
  cmsis_nn_conv_params conv_params;
  conv_params.dilation.h = params.dilation_height_factor;
  conv_params.dilation.w = params.dilation_width_factor;

  // Initialize cmsis_nn convolution parameters
  conv_params.input_offset = -data.reference_op_data.input_zero_point;
  conv_params.output_offset = data.reference_op_data.output_zero_point;
  conv_params.stride.h = params.stride_height;
  conv_params.stride.w = params.stride_width;
  conv_params.padding.h = data.reference_op_data.padding.height;
  conv_params.padding.w = data.reference_op_data.padding.width;
  conv_params.activation.min = data.reference_op_data.output_activation_min;
  conv_params.activation.max = data.reference_op_data.output_activation_max;

  // Initialize cmsis_nn per channel quantization parameters
  cmsis_nn_per_channel_quant_params quant_params;
  quant_params.multiplier = const_cast<int32_t*>(
      data.reference_op_data.per_channel_output_multiplier);
  quant_params.shift =
      const_cast<int32_t*>(data.reference_op_data.per_channel_output_shift);
  
  RuntimeShape filter_shape = tflite::micro::GetTensorShape(filter);
  RuntimeShape input_shape = tflite::micro::GetTensorShape(input);
  RuntimeShape output_shape = tflite::micro::GetTensorShape(output);
  RuntimeShape bias_shape = tflite::micro::GetTensorShape(bias);

  // Consistency check.
  TFLITE_DCHECK_LE(conv_params.activation.min, conv_params.activation.max);
  TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(filter_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);
  const int batch_size = MatchingDim(input_shape, 0, output_shape, 0);
  const int input_depth = MatchingDim(input_shape, 3, filter_shape, 3);
  const int output_depth = MatchingDim(filter_shape, 0, output_shape, 3);
  if (tflite::micro::GetOptionalTensorData<int8_t>(bias)) {
    TFLITE_DCHECK_EQ(bias_shape.FlatSize(), output_depth);
  }

  // Initialize cmsis_nn dimensions
  // Input
  cmsis_nn_dims input_dims;
  input_dims.n = batch_size;
  input_dims.h = input_shape.Dims(1);
  input_dims.w = input_shape.Dims(2);
  input_dims.c = input_depth;

  // Filter
  cmsis_nn_dims filter_dims;
  filter_dims.n = output_depth;
  filter_dims.h = filter_shape.Dims(1);
  filter_dims.w = filter_shape.Dims(2);
  filter_dims.c = input_depth;

  // Bias
  cmsis_nn_dims bias_dims;
  bias_dims.n = 1;
  bias_dims.h = 1;
  bias_dims.w = 1;
  bias_dims.c = output_depth;

  // Output
  cmsis_nn_dims output_dims;
  output_dims.n = batch_size;
  output_dims.h = output_shape.Dims(1);
  output_dims.w = output_shape.Dims(2);
  output_dims.c = output_depth;

  // Initialize cmsis_nn context
  cmsis_nn_context ctx;
  ctx.buf = nullptr;
  ctx.size = 0;

  if (data.buffer_idx > -1) {
    ctx.buf = context->GetScratchBuffer(context, data.buffer_idx);
    // Note: ctx.size is currently not used in cmsis_nn.
    // The buffer should be allocated in the Prepare function through
    // arm_convolve_wrapper_s8_get_buffer_size
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

  if ((filter_dims.w == 1) && (filter_dims.h == 1))
  {
    TFLITE_DCHECK_EQ(
      beco_convolve_1x1_s8(
        &ctx, &conv_params, &quant_params, &input_dims,
        tflite::micro::GetTensorData<int8_t>(input), &filter_dims,
        tflite::micro::GetTensorData<int8_t>(filter), &bias_dims,
        bias_data, &output_dims,
        tflite::micro::GetTensorData<int8_t>(output)),
    ARM_CMSIS_NN_SUCCESS); 
  }
  else
  {
    TFLITE_DCHECK_EQ(
      beco_convolve_s8(
          &ctx, &conv_params, &quant_params, &input_dims,
          tflite::micro::GetTensorData<int8_t>(input), &filter_dims,
          tflite::micro::GetTensorData<int8_t>(filter), &bias_dims,
          bias_data, &output_dims,
          tflite::micro::GetTensorData<int8_t>(output)),
      ARM_CMSIS_NN_SUCCESS);
  }
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

  return kTfLiteOk;
}

TfLiteStatus EvalQuantizedPerChannel16x8(
    TfLiteContext* context, TfLiteNode* node, const TfLiteConvParams& params,
    const OpData& data, const TfLiteEvalTensor* input,
    const TfLiteEvalTensor* filter, const TfLiteEvalTensor* bias,
    TfLiteEvalTensor* output) {
  cmsis_nn_conv_params conv_params;
  conv_params.dilation.h = params.dilation_height_factor;
  conv_params.dilation.w = params.dilation_width_factor;

  // Initialize cmsis_nn convolution parameters
  conv_params.input_offset = -data.reference_op_data.input_zero_point;
  conv_params.output_offset = data.reference_op_data.output_zero_point;
  conv_params.stride.h = params.stride_height;
  conv_params.stride.w = params.stride_width;
  conv_params.padding.h = data.reference_op_data.padding.height;
  conv_params.padding.w = data.reference_op_data.padding.width;
  conv_params.activation.min = data.reference_op_data.output_activation_min;
  conv_params.activation.max = data.reference_op_data.output_activation_max;

  // Initialize cmsis_nn per channel quantization parameters
  cmsis_nn_per_channel_quant_params quant_params;
  quant_params.multiplier = const_cast<int32_t*>(
      data.reference_op_data.per_channel_output_multiplier);
  quant_params.shift =
      const_cast<int32_t*>(data.reference_op_data.per_channel_output_shift);

  RuntimeShape filter_shape = tflite::micro::GetTensorShape(filter);
  RuntimeShape input_shape = tflite::micro::GetTensorShape(input);
  RuntimeShape output_shape = tflite::micro::GetTensorShape(output);
  RuntimeShape bias_shape = tflite::micro::GetTensorShape(bias);

  // Consistency check.
  TFLITE_DCHECK_LE(conv_params.activation.min, conv_params.activation.max);
  TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(filter_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);
  const int batch_size = MatchingDim(input_shape, 0, output_shape, 0);
  const int input_depth = MatchingDim(input_shape, 3, filter_shape, 3);
  const int output_depth = MatchingDim(filter_shape, 0, output_shape, 3);
  if (tflite::micro::GetOptionalTensorData<int8_t>(bias)) {
    TFLITE_DCHECK_EQ(bias_shape.FlatSize(), output_depth);
  }

  // Initialize cmsis_nn dimensions
  // Input
  cmsis_nn_dims input_dims;
  input_dims.n = batch_size;
  input_dims.h = input_shape.Dims(1);
  input_dims.w = input_shape.Dims(2);
  input_dims.c = input_depth;

  // Filter
  cmsis_nn_dims filter_dims;
  filter_dims.n = output_depth;
  filter_dims.h = filter_shape.Dims(1);
  filter_dims.w = filter_shape.Dims(2);
  filter_dims.c = input_depth;

  // Bias
  cmsis_nn_dims bias_dims;
  bias_dims.n = 1;
  bias_dims.h = 1;
  bias_dims.w = 1;
  bias_dims.c = output_depth;

  // Output
  cmsis_nn_dims output_dims;
  output_dims.n = batch_size;
  output_dims.h = output_shape.Dims(1);
  output_dims.w = output_shape.Dims(2);
  output_dims.c = output_depth;

  // Initialize cmsis_nn context
  cmsis_nn_context ctx;
  ctx.buf = nullptr;
  ctx.size = 0;

  if (data.buffer_idx > -1) {
    ctx.buf = context->GetScratchBuffer(context, data.buffer_idx);
    // Note: ctx.size is currently not used in cmsis_nn.
    // The buffer should be allocated in the Prepare function through
    // arm_convolve_wrapper_s8_get_buffer_size
  }

  const int32_t* bias_data = tflite::micro::GetOptionalTensorData<int32_t>(bias);

  int size_input = input_dims.h*input_dims.w*input_dims.c;
  int16_t* new_input_data = new int16_t[size_input];
  const short int* input_data = tflite::micro::GetTensorData<int16_t>(input);
  HWC_2_CHW(input_data, new_input_data, input_dims.h, input_dims.w, input_dims.c);
  memmove((int8_t*)input_data, (int8_t*)new_input_data, 2*size_input);
  delete[] new_input_data;


  if ((filter_dims.w == 1) && (filter_dims.h == 1))
  {
    TFLITE_DCHECK_EQ(
    beco_convolve_1x1_s16(
        &ctx, &conv_params, &quant_params, &input_dims,
        tflite::micro::GetTensorData<int16_t>(input), &filter_dims,
        tflite::micro::GetTensorData<int8_t>(filter), &bias_dims,
        bias_data, &output_dims,
        tflite::micro::GetTensorData<int16_t>(output)),
    ARM_CMSIS_NN_SUCCESS); 
  }
  else
  {
     TFLITE_DCHECK_EQ(
      beco_convolve_s16(
          &ctx, &conv_params, &quant_params, &input_dims,
          tflite::micro::GetTensorData<int16_t>(input), &filter_dims,
          tflite::micro::GetTensorData<int8_t>(filter), &bias_dims,
          bias_data, &output_dims,
          tflite::micro::GetTensorData<int16_t>(output)),
      ARM_CMSIS_NN_SUCCESS); 
  }

  int size_output = output_dims.h*output_dims.w*output_dims.c;
  int16_t* new_output_data = new int16_t[size_output];
  short int* output_data = tflite::micro::GetTensorData<int16_t>(output);
  CHW_2_HWC(output_data, new_output_data, output_dims.h, output_dims.w, output_dims.c);
  memmove((int8_t*)output_data, (int8_t*)new_output_data, 2*size_output);
  delete[] new_output_data;

  return kTfLiteOk;
}

TfLiteStatus EvalInt8(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kConvInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kConvWeightsTensor);
  const TfLiteEvalTensor* bias =
      (NumInputs(node) == 3)
          ? tflite::micro::GetEvalInput(context, node, kConvBiasTensor)
          : nullptr;
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kConvOutputTensor);

  TFLITE_DCHECK(node->builtin_data != nullptr);
  const auto& params =
      *(reinterpret_cast<TfLiteConvParams*>(node->builtin_data));
  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));
  TfLiteEvalTensor filter_int8 = tflite::micro::MakeUnpackedInt4Tensor(
      context, data.reference_op_data.filter_buffer_index, filter);

  return EvalQuantizedPerChannel(context, node, params, data, input,
                                 &filter_int8, bias, output);
}

TfLiteStatus EvalInt16x8(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kConvInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kConvWeightsTensor);
  const TfLiteEvalTensor* bias =
      (NumInputs(node) == 3)
          ? tflite::micro::GetEvalInput(context, node, kConvBiasTensor)
          : nullptr;
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kConvOutputTensor);

  TFLITE_DCHECK(node->builtin_data != nullptr);
  const auto& params =
      *(reinterpret_cast<TfLiteConvParams*>(node->builtin_data));
  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  return EvalQuantizedPerChannel16x8(context, node, params, data, input, filter,
                                     bias, output);
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kConvInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kConvWeightsTensor);
  const TfLiteEvalTensor* bias =
      (NumInputs(node) == 3)
          ? tflite::micro::GetEvalInput(context, node, kConvBiasTensor)
          : nullptr;
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kConvOutputTensor);

  TFLITE_DCHECK(node->builtin_data != nullptr);
  const auto& params =
      *(reinterpret_cast<TfLiteConvParams*>(node->builtin_data));
  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  TF_LITE_ENSURE_EQ(context, input->type, output->type);
  TF_LITE_ENSURE_MSG(
      context,
      input->type == filter->type ||
          (input->type == kTfLiteInt16 && filter->type == kTfLiteInt8) ||
          (input->type == kTfLiteInt8 && filter->type == kTfLiteInt4),
      "Hybrid models are not supported on TFLite Micro.");

  TfLiteEvalTensor filter_int8 = tflite::micro::MakeUnpackedInt4Tensor(
      context, data.reference_op_data.filter_buffer_index, filter);

  switch (input->type) {  // Already know in/out types are same.
    case kTfLiteFloat32: {
      tflite::reference_ops::Conv(
          ConvParamsFloat(params, data.reference_op_data),
          tflite::micro::GetTensorShape(input),
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
    case kTfLiteInt8:
      switch (filter_int8.type) {
        case kTfLiteInt8: {
          return EvalQuantizedPerChannel(context, node, params, data, input,
                                         &filter_int8, bias, output);
        }

        default: {
          MicroPrintf("Filter type %s (%d) not supported.",
                      TfLiteTypeGetName(filter->type), filter->type);
          return kTfLiteError;
        }
      }

      break;
    case kTfLiteInt16:
      return EvalQuantizedPerChannel16x8(context, node, params, data, input,
                                         filter, bias, output);
      break;
    default:
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
  }
  return kTfLiteOk;
}

}  // namespace

TfLiteRegistration_V1 Register_CONV_2D() {
  return tflite::micro::RegisterOp(Init, Prepare, Eval);
}

TfLiteRegistration_V1 Register_CONV_2D_INT8() {
  return tflite::micro::RegisterOp(Init, Prepare, EvalInt8);
}

TfLiteRegistration_V1 Register_CONV_2D_INT16() {
  return tflite::micro::RegisterOp(Init, Prepare, EvalInt16x8);
}

}  // namespace tflite
