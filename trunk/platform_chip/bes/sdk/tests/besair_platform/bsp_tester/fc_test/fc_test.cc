#include "fc_test.h"

#include "fc_test_model_data.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "multi_heap.h"
#include <string.h>
#include "hal_timer.h"
#include "hal_trace.h"
#include <stdio.h>
#include <stdlib.h>

#define FC_TEST_HEAP_BUFF_SIZE (1200)
static multi_heap_handle_t fc_test_multi_heap_handle;
static int8_t heap_buff[FC_TEST_HEAP_BUFF_SIZE];

struct ModelState_s
{
    uint8_t* model_data;
    int model_data_size;
    uint8_t *tensor_arena;
    int tensor_arena_size;
    tflite::MicroInterpreter* interpreter;
};


ModelState_t* model_create()
{
  printf("%s: entry\n", __func__);
  fc_test_multi_heap_handle = multi_heap_register((void *)(heap_buff), FC_TEST_HEAP_BUFF_SIZE);
  ModelState_t* st = (ModelState_t*)multi_heap_malloc(fc_test_multi_heap_handle, sizeof(ModelState_t));

  const tflite::Model* model = tflite::GetModel(g_fc_test_model_data);

  static tflite::MicroMutableOpResolver<1> micro_op_resolver;
  micro_op_resolver.AddFullyConnected();
  // micro_op_resolver.AddSoftmax();

  tflite::MicroErrorReporter micro_error_reporter;
  st->tensor_arena_size = 1000;
  st->tensor_arena = (uint8_t*)multi_heap_malloc(fc_test_multi_heap_handle, st->tensor_arena_size * sizeof(uint8_t));

  st->interpreter = new tflite::MicroInterpreter(model, micro_op_resolver, st->tensor_arena,
                                                 st->tensor_arena_size);
  st->interpreter->AllocateTensors();

  printf("%s: exit\n", __func__);
  return st;
}

void loop_dtln(ModelState_t* model_st)
{
  TfLiteTensor* input0 = model_st->interpreter->input(0);

  for (int32_t i = 0; i < 4; i++) {
    int8_t x_quantized = (int8_t)(1 / input0->params.scale + input0->params.zero_point);
    input0->data.int8[i] = x_quantized;

    // int16_t x_quantized = (int16_t)(1 / input0->params.scale + input0->params.zero_point);
    // input0->data.i16[i] = x_quantized;
  }

  uint32_t s_time = hal_fast_sys_timer_get();
  TfLiteStatus invoke_status = model_st->interpreter->Invoke();
  uint32_t e_time = hal_fast_sys_timer_get();
  TRACE(3, "[%s] cost %dus at sysfreq %d", __FUNCTION__,
        FAST_TICKS_TO_US(e_time - s_time), hal_sys_timer_calc_cpu_freq(5, 0));
  // Obtain a pointer to the output tensor
  TfLiteTensor* output0 = model_st->interpreter->output(0);

  printf("\n--------------result---------------\n");
  for (int32_t i = 0; i < 4; i++) {
    int8_t y_quantized = output0->data.int8[i];
    // int16_t y_quantized = output0->data.i16[i];
    float y = (y_quantized - output0->params.zero_point) * output0->params.scale;
    printf("%d.%.8d ", (int)y, (int)((y-(int)y)*100000000));
  }
  printf("\n--------------Finish---------------\n");
}

void model_destroy(ModelState_t *st)
{
    multi_heap_free(fc_test_multi_heap_handle, st->tensor_arena);
    delete st->interpreter;
    multi_heap_free(fc_test_multi_heap_handle, st);
}


void run_fc_test()
{
    ModelState_t* model_st = nullptr;
    model_st = model_create();
    uint32_t s_time = hal_fast_sys_timer_get();
    loop_dtln(model_st);
    uint32_t e_time = hal_fast_sys_timer_get();
    TRACE(3, "[%s] cost %dus at sysfreq %d", __FUNCTION__,
        FAST_TICKS_TO_US(e_time - s_time), hal_sys_timer_calc_cpu_freq(5, 0));
    model_destroy(model_st);
}