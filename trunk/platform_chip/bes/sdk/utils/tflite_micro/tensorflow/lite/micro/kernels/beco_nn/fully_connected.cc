#include "tensorflow/lite/micro/kernels/fully_connected.h"

#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"
#include "beco_bias.hpp"
#include "beco_nn/beco_nnfunctions.h"
#include "dsp/matrix_functions.h"

#include "arm_math.h"
#include "arm_nnfunctions.h"
#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/common.h"
#include "tensorflow/lite/kernels/internal/portable_tensor_utils.h"
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/kernels/internal/reference/fully_connected.h"
#include "tensorflow/lite/kernels/internal/reference/integer_ops/fully_connected.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"
#include "tensorflow/lite/micro/micro_log.h"

#include "hal_trace.h"

namespace tflite {
namespace {

struct OpData {
  OpDataFullyConnected reference_op_data;

  // Conv 1x1 that may be invoked in some cases currently need per channel
  // quantization.
  int32_t* per_channel_output_multiplier;
  int32_t* per_channel_output_shift;

  // Index to buffer for optimizations if applicable.
  int buffer_idx;

  int32_t batches;
  int32_t accum_depth;
  int32_t output_depth;

  // 
  int32_t* kernel_sum;
};

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

static void add_input_offset_2_bias(int32_t *bias,
                                    int8_t *weight,
                                    int input_offset,
                                    int dim_vec,
                                    int dim_col)
{
    for (int i = 0; i < dim_col; i++) {
        int sum = 0;
        for (int j = 0; j < dim_vec; j++) {
            sum += weight[j*dim_col + i] * input_offset;
        }
        bias[i] += sum;
    }
}

static int find_ptr(std::vector<int8_t*>& v, int8_t* ptr)
{
  int flag = 0;
  for(std::vector<int8_t*>::iterator iptr = v.begin(); iptr != v.end(); iptr++){
    if (ptr == *iptr)
    {
      flag = 1;
    }
  }
  return flag;
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  OpData* data = static_cast<OpData*>(node->user_data);
  const auto params =
      static_cast<const TfLiteFullyConnectedParams*>(node->builtin_data);

  MicroContext* micro_context = GetMicroContext(context);
  TfLiteTensor* input =
      micro_context->AllocateTempInputTensor(node, kFullyConnectedInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  TfLiteTensor* filter = micro_context->AllocateTempInputTensor(
      node, kFullyConnectedWeightsTensor);
  TF_LITE_ENSURE(context, filter != nullptr);
  TfLiteTensor* bias =
      micro_context->AllocateTempInputTensor(node, kFullyConnectedBiasTensor);
  TfLiteTensor* output = micro_context->AllocateTempOutputTensor(
      node, kFullyConnectedOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);

  TF_LITE_ENSURE_TYPES_EQ(context, input->type, output->type);

  const RuntimeShape filter_shape = GetTensorShape(filter);
  const RuntimeShape output_shape = GetTensorShape(output);
  const int filter_dim_count = filter_shape.DimensionsCount();
  const int output_dim_count = output_shape.DimensionsCount();

  data->accum_depth = filter_shape.Dims(filter_dim_count - 1);
  data->batches = FlatSizeSkipDim(output_shape, output_dim_count - 1);
  data->output_depth = output_shape.Dims(output_dim_count - 1);

  //   修改权重形状以及bias的值

  int8_t* filter_data = filter->data.int8;
  TRACE(0, "Prepare-----%p------", filter_data);
  int is_in_v = find_ptr(v_filter_ptr, filter_data);
  if (is_in_v == 0){
    v_filter_ptr.push_back(filter_data);

    int size_filter = data->output_depth * data->accum_depth;
    int8_t* new_filter_data = new int8_t[size_filter];

    arm_matrix_instance_q7 pISrc;
    pISrc.numRows = (int16_t)data->output_depth;
    pISrc.numCols = (int16_t)data->accum_depth;
    pISrc.pData = filter_data;
    arm_matrix_instance_q7 pIDst;
    pIDst.numRows = (int16_t)data->accum_depth;
    pIDst.numCols = (int16_t)data->output_depth;
    pIDst.pData = new_filter_data;
    arm_mat_trans_q7(&pISrc, &pIDst);
    memcpy(filter_data, new_filter_data, size_filter);
    delete[] new_filter_data;
  }

  if (input->type == kTfLiteInt8){
    int zp = input->params.zero_point;                     
    if (bias != nullptr){
         int32_t* bias_data = bias->data.i32;
         if (zp != 0) {
          add_input_offset_2_bias(bias_data, filter_data, -zp, data->accum_depth, data->output_depth);
      }
    }
    else{
      if (zp != 0){
        int32_t* new_bias_data = new int32_t[data->output_depth];
        memset(new_bias_data, 0, 4*data->output_depth);
        add_input_offset_2_bias(new_bias_data, filter_data, -zp, data->accum_depth, data->output_depth);
        data->kernel_sum = new_bias_data;
      }
    }
  }
  if (input->type == kTfLiteInt16){
    if (bias != nullptr){
      int64_t* bias_data = bias->data.i64;
      int32_t* new_bias_i32 = new int32_t[data->output_depth];
      for(int i = 0; i < data->output_depth; i++)
      {
        new_bias_i32[i] = (int32_t)bias_data[i];
      }
      memcpy((int8_t*)bias_data, (int8_t*)new_bias_i32, 4*data->output_depth);
      delete[] new_bias_i32;
    }
  }

//

  // Set buffer index to a reset value
  data->buffer_idx = -1;
  TF_LITE_ENSURE_STATUS(CalculateOpDataFullyConnected(
      context, params->activation, input->type, input, filter, bias, output,
      &(data->reference_op_data)));

  if (input->type == kTfLiteInt16) {
    TF_LITE_ENSURE_EQ(context, input->params.zero_point, 0);
    TF_LITE_ENSURE_EQ(context, output->params.zero_point, 0);
  } else if (input->type == kTfLiteInt8) {
    const RuntimeShape input_shape = GetTensorShape(input);

    TFLITE_DCHECK_GE(output_dim_count, 2);
    TFLITE_DCHECK_LE(output_dim_count, 4);

    if (output_dim_count > 2 && data->accum_depth % 4 == 0) {
      data->per_channel_output_multiplier =
          static_cast<int32_t*>(context->AllocatePersistentBuffer(
              context, data->output_depth * sizeof(int32_t)));
      data->per_channel_output_shift =
          static_cast<int32_t*>(context->AllocatePersistentBuffer(
              context, data->output_depth * sizeof(int32_t)));
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

void PopulateCommonParams(TfLiteContext* context,
                          cmsis_nn_per_tensor_quant_params* const quant_params,
                          cmsis_nn_dims* const input_dims,
                          cmsis_nn_dims* const filter_dims,
                          cmsis_nn_dims* const bias_dims,
                          cmsis_nn_dims* const output_dims,
                          cmsis_nn_context* const ctx, const OpData& data) {
  quant_params->multiplier = data.reference_op_data.output_multiplier;
  quant_params->shift = data.reference_op_data.output_shift;

  input_dims->n = data.batches;
  input_dims->h = 1;
  input_dims->w = 1;
  input_dims->c = data.accum_depth;

  filter_dims->n = data.accum_depth;
  filter_dims->h = 1;
  filter_dims->w = 1;
  filter_dims->c = data.output_depth;

  bias_dims->n = 1;
  bias_dims->h = 1;
  bias_dims->w = 1;
  bias_dims->c = data.output_depth;

  output_dims->n = data.batches;
  output_dims->h = 1;
  output_dims->w = 1;
  output_dims->c = data.output_depth;

  ctx->buf = nullptr;
  ctx->size = 0;
  if (data.buffer_idx > -1) {
    ctx->buf = context->GetScratchBuffer(context, data.buffer_idx);
  }
}

TfLiteStatus EvalQuantizedInt8(TfLiteContext* context, TfLiteNode* node,
                               const OpData& data,
                               const TfLiteEvalTensor* input,
                               const TfLiteEvalTensor* filter,
                               const TfLiteEvalTensor* bias,
                               TfLiteEvalTensor* output) {
  const RuntimeShape output_shape = tflite::micro::GetTensorShape(output);
  const int output_dim_count = output_shape.DimensionsCount();
  TFLITE_DCHECK_GE(output_dim_count, 2);
  TFLITE_DCHECK_LE(output_dim_count, 4);

  cmsis_nn_per_tensor_quant_params quant_params;
  cmsis_nn_dims input_dims;
  cmsis_nn_dims filter_dims;
  cmsis_nn_dims bias_dims;
  cmsis_nn_dims output_dims;
  cmsis_nn_context ctx;

  PopulateCommonParams(context, &quant_params, &input_dims, &filter_dims,
                       &bias_dims, &output_dims, &ctx, data);

  const int32_t* bias_data =
      tflite::micro::GetOptionalTensorData<int32_t>(bias);
  
  cmsis_nn_fc_params fc_params;
  fc_params.input_offset = -data.reference_op_data.input_zero_point;
  fc_params.output_offset = data.reference_op_data.output_zero_point;
  fc_params.filter_offset = 0;
  fc_params.activation.min = data.reference_op_data.output_activation_min;
  fc_params.activation.max = data.reference_op_data.output_activation_max;
  
  if (bias_data == nullptr && fc_params.input_offset != 0)
  {
    bias_data = data.kernel_sum;
  }

  const signed char* input_ptr = tflite::micro::GetTensorData<int8_t>(input);
  const signed char* filter_ptr = tflite::micro::GetTensorData<int8_t>(filter);
  TRACE(0, "EvalQuantizedInt8-----%p------", filter_ptr);
  // TRACE(0, "-----------------------------------------------------------");
  // TRACE(0, "%d, %d, %d, %d", input_ptr[0], input_ptr[1], input_ptr[2], input_ptr[3]);

  // TRACE(0, "-----------------------------------------------------------");

  // for (int i = 0; i<filter_dims.n; i++){
  //   TRACE(0, "%d, %d, %d, %d %d, %d, %d, %d %d, %d, %d, %d %d, %d, %d, %d", filter_ptr[filter_dims.c*i+0], filter_ptr[filter_dims.c*i+1], filter_ptr[filter_dims.c*i+2], filter_ptr[filter_dims.c*i+3],
  //                                                                           filter_ptr[filter_dims.c*i+4], filter_ptr[filter_dims.c*i+5], filter_ptr[filter_dims.c*i+6], filter_ptr[filter_dims.c*i+7],
  //                                                                           filter_ptr[filter_dims.c*i+8], filter_ptr[filter_dims.c*i+9], filter_ptr[filter_dims.c*i+10], filter_ptr[filter_dims.c*i+11],
  //                                                                           filter_ptr[filter_dims.c*i+12], filter_ptr[filter_dims.c*i+13], filter_ptr[filter_dims.c*i+14], filter_ptr[filter_dims.c*i+15]);
  // }
  

  TF_LITE_ENSURE_EQ(
       context,
       beco_fully_connected_s8(
            &ctx, &fc_params, &quant_params, &input_dims,
            tflite::micro::GetTensorData<int8_t>(input), &filter_dims,
            tflite::micro::GetTensorData<int8_t>(filter), &bias_dims, bias_data,
            &output_dims, tflite::micro::GetTensorData<int8_t>(output)),
       ARM_CMSIS_NN_SUCCESS);
  
  // const signed char* output_ptr = tflite::micro::GetTensorData<int8_t>(output);
  // TRACE(0, "-----------------------------------------------------------");
  // TRACE(0, "%d, %d, %d, %d %d, %d, %d, %d %d, %d, %d, %d %d, %d, %d, %d", output_ptr[0], output_ptr[1], output_ptr[2], output_ptr[3],
  //                                                                         output_ptr[4], output_ptr[5], output_ptr[6], output_ptr[7],
  //                                                                         output_ptr[8], output_ptr[9], output_ptr[10], output_ptr[11],
  //                                                                         output_ptr[12], output_ptr[13], output_ptr[14], output_ptr[15]);

  if (bias_data == nullptr && fc_params.input_offset != 0 && data.kernel_sum != nullptr)
  {
    delete[] data.kernel_sum;
  }

  for(std::vector<int8_t*>::iterator iptr = v_filter_ptr.begin(); iptr != v_filter_ptr.end(); iptr++){
		TRACE(0, "vector<int8_t*>-----%p------", *iptr);
	}

  return kTfLiteOk;
}

TfLiteStatus EvalQuantizedInt16(TfLiteContext* context, TfLiteNode* node,
                                const OpData& data,
                                const TfLiteEvalTensor* input,
                                const TfLiteEvalTensor* filter,
                                const TfLiteEvalTensor* bias,
                                TfLiteEvalTensor* output) {
  cmsis_nn_per_tensor_quant_params quant_params;
  cmsis_nn_dims input_dims;
  cmsis_nn_dims filter_dims;
  cmsis_nn_dims bias_dims;
  cmsis_nn_dims output_dims;
  cmsis_nn_context ctx;

  PopulateCommonParams(context, &quant_params, &input_dims, &filter_dims,
                       &bias_dims, &output_dims, &ctx, data);

  const int32_t* bias_data = tflite::micro::GetOptionalTensorData<int32_t>(bias);

  cmsis_nn_fc_params fc_params;
  fc_params.input_offset = -data.reference_op_data.input_zero_point;
  fc_params.output_offset = data.reference_op_data.output_zero_point;
  fc_params.filter_offset = 0;
  fc_params.activation.min = data.reference_op_data.output_activation_min;
  fc_params.activation.max = data.reference_op_data.output_activation_max;

  TF_LITE_ENSURE_EQ(
      context,
      beco_fully_connected_s16(
          &ctx, &fc_params, &quant_params, &input_dims,
          tflite::micro::GetTensorData<int16_t>(input), &filter_dims,
          tflite::micro::GetTensorData<int8_t>(filter), &bias_dims, bias_data,
          &output_dims, tflite::micro::GetTensorData<int16_t>(output)),
      ARM_CMSIS_NN_SUCCESS);

  return kTfLiteOk;
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  const auto* params =
      static_cast<const TfLiteFullyConnectedParams*>(node->builtin_data);

  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kFullyConnectedInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kFullyConnectedWeightsTensor);
  const TfLiteEvalTensor* bias =
      tflite::micro::GetEvalInput(context, node, kFullyConnectedBiasTensor);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kFullyConnectedOutputTensor);


  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  TfLiteEvalTensor filter_int8 = tflite::micro::MakeUnpackedInt4Tensor(
      context, data.reference_op_data.filter_buffer_index, filter);

  // Checks in Prepare ensure input, output and filter types are all the same.
  switch (input->type) {
    case kTfLiteFloat32: {
      const float* bias_data =
          tflite::micro::GetOptionalTensorData<float>(bias);
      tflite::reference_ops::FullyConnected(
          FullyConnectedParamsFloat(params->activation),
          tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<float>(input),
          tflite::micro::GetTensorShape(filter),
          tflite::micro::GetTensorData<float>(filter),
          tflite::micro::GetTensorShape(bias), bias_data,
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<float>(output));
      break;
    }
    case kTfLiteInt8: {
      switch (filter_int8.type) {
        case kTfLiteInt8:
          return EvalQuantizedInt8(context, node, data, input, &filter_int8,
                                   bias, output);
        default:
          MicroPrintf("Filter Type %s (%d) not supported.",
                      TfLiteTypeGetName(filter->type), filter->type);
          return kTfLiteError;
      }
      break;
    }
    case kTfLiteInt16: {
      return EvalQuantizedInt16(context, node, data, input, filter, bias,
                                output);
    }
    default: {
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
    }
  }
  return kTfLiteOk;
}

// Note that the current function names are not ideal at all (this EvalInt8
// function internally calls EvalQuantizedInt8, and there is similar name
// aliasing in the Eval function too). We will be attempting to have a more
// descriptive naming convention but holding off on that for now, since the
// renaming might be coupled with reducing code duplication and some additional
// refactoring.
TfLiteStatus EvalInt8(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kFullyConnectedInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kFullyConnectedWeightsTensor);
  const TfLiteEvalTensor* bias =
      tflite::micro::GetEvalInput(context, node, kFullyConnectedBiasTensor);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kFullyConnectedOutputTensor);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  // Checks in Prepare ensure input, output and filter types are all the same.
  if (input->type != kTfLiteInt8) {
    MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                input->type);
    return kTfLiteError;
  }

  TfLiteEvalTensor filter_int8 = tflite::micro::MakeUnpackedInt4Tensor(
      context, data.reference_op_data.filter_buffer_index, filter);

  return EvalQuantizedInt8(context, node, data, input, &filter_int8, bias,
                           output);
}

TfLiteStatus EvalInt16(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kFullyConnectedInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kFullyConnectedWeightsTensor);
  const TfLiteEvalTensor* bias =
      tflite::micro::GetEvalInput(context, node, kFullyConnectedBiasTensor);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kFullyConnectedOutputTensor);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  // Checks in Prepare ensure input, output and filter types are all the same.
  if (input->type != kTfLiteInt16) {
    MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                input->type);
    return kTfLiteError;
  }

  return EvalQuantizedInt16(context, node, data, input, filter, bias, output);
}

}  // namespace

TfLiteRegistration_V1 Register_FULLY_CONNECTED() {
  return tflite::micro::RegisterOp(Init, Prepare, Eval);
}

TfLiteRegistration_V1 Register_FULLY_CONNECTED_INT8() {
  return tflite::micro::RegisterOp(Init, Prepare, EvalInt8);
}

TfLiteRegistration_V1 Register_FULLY_CONNECTED_INT16() {
  return tflite::micro::RegisterOp(Init, Prepare, EvalInt16);
}

}  // namespace tflite
