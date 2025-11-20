cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cc := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cc))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

MICROLITE_CC_KERNEL_SRCS := \
tensorflow/lite/micro/kernels/activations.cc \
tensorflow/lite/micro/kernels/activations_common.cc \
tensorflow/lite/micro/kernels/add.cc \
tensorflow/lite/micro/kernels/add_common.cc \
tensorflow/lite/micro/kernels/add_n.cc \
tensorflow/lite/micro/kernels/arg_min_max.cc \
tensorflow/lite/micro/kernels/assign_variable.cc \
tensorflow/lite/micro/kernels/batch_to_space_nd.cc \
tensorflow/lite/micro/kernels/broadcast_args.cc \
tensorflow/lite/micro/kernels/broadcast_to.cc \
tensorflow/lite/micro/kernels/call_once.cc \
tensorflow/lite/micro/kernels/cast.cc \
tensorflow/lite/micro/kernels/ceil.cc \
tensorflow/lite/micro/kernels/circular_buffer.cc \
tensorflow/lite/micro/kernels/circular_buffer_common.cc \
tensorflow/lite/micro/kernels/comparisons.cc \
tensorflow/lite/micro/kernels/concatenation.cc \
tensorflow/lite/micro/kernels/conv.cc \
tensorflow/lite/micro/kernels/conv_common.cc \
tensorflow/lite/micro/kernels/cumsum.cc \
tensorflow/lite/micro/kernels/depth_to_space.cc \
tensorflow/lite/micro/kernels/depthwise_conv.cc \
tensorflow/lite/micro/kernels/depthwise_conv_common.cc \
tensorflow/lite/micro/kernels/dequantize.cc \
tensorflow/lite/micro/kernels/dequantize_common.cc \
tensorflow/lite/micro/kernels/detection_postprocess.cc \
tensorflow/lite/micro/kernels/div.cc \
tensorflow/lite/micro/kernels/elementwise.cc \
tensorflow/lite/micro/kernels/elu.cc \
tensorflow/lite/micro/kernels/ethosu.cc \
tensorflow/lite/micro/kernels/exp.cc \
tensorflow/lite/micro/kernels/expand_dims.cc \
tensorflow/lite/micro/kernels/fill.cc \
tensorflow/lite/micro/kernels/floor.cc \
tensorflow/lite/micro/kernels/floor_div.cc \
tensorflow/lite/micro/kernels/floor_mod.cc \
tensorflow/lite/micro/kernels/fully_connected.cc \
tensorflow/lite/micro/kernels/fully_connected_common.cc \
tensorflow/lite/micro/kernels/gather.cc \
tensorflow/lite/micro/kernels/gather_nd.cc \
tensorflow/lite/micro/kernels/hard_swish.cc \
tensorflow/lite/micro/kernels/hard_swish_common.cc \
tensorflow/lite/micro/kernels/if.cc \
tensorflow/lite/micro/kernels/kernel_runner.cc \
tensorflow/lite/micro/kernels/kernel_util.cc \
tensorflow/lite/micro/kernels/l2norm.cc \
tensorflow/lite/micro/kernels/l2_pool_2d.cc \
tensorflow/lite/micro/kernels/leaky_relu.cc \
tensorflow/lite/micro/kernels/leaky_relu_common.cc \
tensorflow/lite/micro/kernels/logical.cc \
tensorflow/lite/micro/kernels/logical_common.cc \
tensorflow/lite/micro/kernels/logistic.cc \
tensorflow/lite/micro/kernels/logistic_common.cc \
tensorflow/lite/micro/kernels/log_softmax.cc \
tensorflow/lite/micro/kernels/lstm_eval.cc \
tensorflow/lite/micro/kernels/lstm_eval_common.cc \
tensorflow/lite/micro/kernels/maximum_minimum.cc \
tensorflow/lite/micro/kernels/micro_tensor_utils.cc \
tensorflow/lite/micro/kernels/mirror_pad.cc \
tensorflow/lite/micro/kernels/mul.cc \
tensorflow/lite/micro/kernels/mul_common.cc \
tensorflow/lite/micro/kernels/neg.cc \
tensorflow/lite/micro/kernels/pack.cc \
tensorflow/lite/micro/kernels/pad.cc \
tensorflow/lite/micro/kernels/pooling.cc \
tensorflow/lite/micro/kernels/pooling_common.cc \
tensorflow/lite/micro/kernels/prelu.cc \
tensorflow/lite/micro/kernels/prelu_common.cc \
tensorflow/lite/micro/kernels/quantize.cc \
tensorflow/lite/micro/kernels/quantize_common.cc \
tensorflow/lite/micro/kernels/read_variable.cc \
tensorflow/lite/micro/kernels/reduce.cc \
tensorflow/lite/micro/kernels/reduce_common.cc \
tensorflow/lite/micro/kernels/reshape.cc \
tensorflow/lite/micro/kernels/resize_bilinear.cc \
tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc \
tensorflow/lite/micro/kernels/round.cc \
tensorflow/lite/micro/kernels/select.cc \
tensorflow/lite/micro/kernels/shape.cc \
tensorflow/lite/micro/kernels/slice.cc \
tensorflow/lite/micro/kernels/softmax.cc \
tensorflow/lite/micro/kernels/softmax_common.cc \
tensorflow/lite/micro/kernels/space_to_batch_nd.cc \
tensorflow/lite/micro/kernels/space_to_depth.cc \
tensorflow/lite/micro/kernels/split.cc \
tensorflow/lite/micro/kernels/split_v.cc \
tensorflow/lite/micro/kernels/squared_difference.cc \
tensorflow/lite/micro/kernels/squeeze.cc \
tensorflow/lite/micro/kernels/strided_slice.cc \
tensorflow/lite/micro/kernels/sub.cc \
tensorflow/lite/micro/kernels/sub_common.cc \
tensorflow/lite/micro/kernels/svdf.cc \
tensorflow/lite/micro/kernels/svdf_common.cc \
tensorflow/lite/micro/kernels/tanh.cc \
tensorflow/lite/micro/kernels/transpose.cc \
tensorflow/lite/micro/kernels/transpose_conv.cc \
tensorflow/lite/micro/kernels/unidirectional_sequence_lstm.cc \
tensorflow/lite/micro/kernels/unpack.cc \
tensorflow/lite/micro/kernels/var_handle.cc \
tensorflow/lite/micro/kernels/while.cc \
tensorflow/lite/micro/kernels/zeros_like.cc

MICROLITE_TEST_SRCS := \
tensorflow/lite/micro/fake_micro_context_test.cc \
tensorflow/lite/micro/flatbuffer_utils_test.cc \
tensorflow/lite/micro/memory_arena_threshold_test.cc \
tensorflow/lite/micro/memory_helpers_test.cc \
tensorflow/lite/micro/micro_allocator_test.cc \
tensorflow/lite/micro/micro_allocation_info_test.cc \
tensorflow/lite/micro/micro_context_test.cc \
tensorflow/lite/micro/micro_log_test.cc \
tensorflow/lite/micro/micro_interpreter_test.cc \
tensorflow/lite/micro/micro_mutable_op_resolver_test.cc \
tensorflow/lite/micro/micro_resource_variable_test.cc \
tensorflow/lite/micro/micro_string_test.cc \
tensorflow/lite/micro/micro_time_test.cc \
tensorflow/lite/micro/micro_utils_test.cc \
tensorflow/lite/micro/recording_micro_allocator_test.cc \
tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator_test.cc \
tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator_test.cc \
tensorflow/lite/micro/arena_allocator/recording_single_arena_buffer_allocator_test.cc \
tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator_test.cc \
tensorflow/lite/micro/testing_helpers_test.cc \
tensorflow/lite/micro/memory_planner/greedy_memory_planner_test.cc \
tensorflow/lite/micro/memory_planner/linear_memory_planner_test.cc \
tensorflow/lite/micro/memory_planner/non_persistent_buffer_planner_shim_test.cc

MICROLITE_REF_KERNEL_SRCS := \
tensorflow/lite/micro/kernels/add.cc \
tensorflow/lite/micro/kernels/conv.cc \
tensorflow/lite/micro/kernels/depthwise_conv.cc \
tensorflow/lite/micro/kernels/fully_connected.cc \
tensorflow/lite/micro/kernels/mul.cc \
tensorflow/lite/micro/kernels/pooling.cc \
tensorflow/lite/micro/kernels/softmax.cc \
tensorflow/lite/micro/kernels/svdf.cc \
tensorflow/lite/micro/kernels/unidirectional_sequence_lstm.cc

MICROLITE_CMSIS_KERNEL_SRCS := \
tensorflow/lite/micro/kernels/cmsis_nn/add.cc \
tensorflow/lite/micro/kernels/cmsis_nn/conv.cc \
tensorflow/lite/micro/kernels/cmsis_nn/depthwise_conv.cc \
tensorflow/lite/micro/kernels/cmsis_nn/fully_connected.cc \
tensorflow/lite/micro/kernels/cmsis_nn/mul.cc \
tensorflow/lite/micro/kernels/cmsis_nn/pooling.cc \
tensorflow/lite/micro/kernels/cmsis_nn/softmax.cc \
tensorflow/lite/micro/kernels/cmsis_nn/svdf.cc \
tensorflow/lite/micro/kernels/cmsis_nn/unidirectional_sequence_lstm.cc

MICROLITE_CC_KERNEL_SRCS := $(filter-out $(MICROLITE_REF_KERNEL_SRCS), $(MICROLITE_CC_KERNEL_SRCS))

# MICROLITE_BENCHMARK_SRCS := \
# $(wildcard tensorflow/lite/micro/benchmarks/*benchmark.cc)

# TFL_CC_SRCS := \
# $(shell find tensorflow/lite -type d \( -path tensorflow/lite/experimental -o -path tensorflow/lite/micro \) -prune -false -o -name "*.cc" -o -name "*.c")

# MICROLITE_CC_BASE_SRCS := \
# $(wildcard tensorflow/lite/micro/*.cc) \
# $(wildcard tensorflow/lite/micro/arena_allocator/*.cc) \
# $(wildcard tensorflow/lite/micro/memory_planner/*.cc) \
# $(wildcard tensorflow/lite/micro/tflite_bridge/*.cc) \
# $(TFL_CC_SRCS)

# MICROLITE_CC_SRCS := $(filter-out $(MICROLITE_TEST_SRCS), $(MICROLITE_CC_BASE_SRCS))
# MICROLITE_CC_SRCS := $(filter-out $(MICROLITE_BENCHMARK_SRCS), $(MICROLITE_CC_SRCS))

# obj_cc += \
#     $(MICROLITE_CC_KERNEL_SRCS) \
#     $(MICROLITE_CMSIS_KERNEL_SRCS) \
#     $(MICROLITE_CC_SRCS) \

obj_cc += \
    tensorflow/lite/micro/micro_op_resolver.cc \
    tensorflow/lite/micro/test_helper_custom_ops.cc \
    tensorflow/lite/micro/memory_helpers.cc \
    tensorflow/lite/micro/test_helpers.cc \
    tensorflow/lite/micro/micro_resource_variable.cc \
    tensorflow/lite/micro/fake_micro_context.cc \
    tensorflow/lite/micro/debug_log.cc \
    tensorflow/lite/micro/micro_allocator.cc \
    tensorflow/lite/micro/micro_string.cc \
    tensorflow/lite/micro/micro_profiler.cc \
    tensorflow/lite/micro/micro_utils.cc \
    tensorflow/lite/micro/recording_micro_allocator.cc \
    tensorflow/lite/micro/micro_allocation_info.cc \
    tensorflow/lite/micro/micro_time.cc \
    tensorflow/lite/micro/micro_graph.cc \
    tensorflow/lite/micro/mock_micro_graph.cc \
    tensorflow/lite/micro/micro_context.cc \
    tensorflow/lite/micro/all_ops_resolver.cc \
    tensorflow/lite/micro/system_setup.cc \
    tensorflow/lite/micro/micro_interpreter.cc \
    tensorflow/lite/micro/micro_log.cc \
    tensorflow/lite/micro/flatbuffer_utils.cc \
    tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator.cc \
    tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator.cc \
    tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator.cc \
    tensorflow/lite/micro/arena_allocator/recording_single_arena_buffer_allocator.cc \
    tensorflow/lite/micro/memory_planner/linear_memory_planner.cc \
    tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc \
    tensorflow/lite/micro/memory_planner/non_persistent_buffer_planner_shim.cc \
    tensorflow/lite/micro/tflite_bridge/micro_error_reporter.cc \
    tensorflow/lite/micro/tflite_bridge/flatbuffer_conversions_bridge.cc \
    tensorflow/lite/core/api/error_reporter.cc \
    tensorflow/lite/core/api/flatbuffer_conversions.cc \
    tensorflow/lite/core/api/op_resolver.cc \
    tensorflow/lite/core/api/tensor_utils.cc \
    tensorflow/lite/core/c/common.cc \
    tensorflow/lite/kernels/internal/portable_tensor_utils.cc \
    tensorflow/lite/kernels/internal/quantization_util.cc \
    tensorflow/lite/kernels/internal/reference/portable_tensor_utils.cc \
    tensorflow/lite/kernels/internal/tensor_utils.cc \
    tensorflow/lite/kernels/kernel_util.cc \
    tensorflow/lite/schema/schema_utils.cc \
    $(MICROLITE_CC_KERNEL_SRCS) \
    $(MICROLITE_CMSIS_KERNEL_SRCS) \

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cc:.cc=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    -Iinclude/cmsis_dsp/ \
    -Iinclude/cmsis_nn/ \
    -Iinclude/beco/ \
    -Imultimedia/inc/speech/inc \
    -Iutils/heap/ \
    -Iutils/tflite_micro/ \
    -Iutils/tflite_micro/tensorflow/lite/micro/tools/make/downloads/flatbuffers/include/ \
    -Iutils/tflite_micro/tensorflow/lite/micro/tools/make/downloads/gemmlowp/ \
    -Iutils/tflite_micro/tensorflow/lite/micro/tools/make/downloads/kissfft/  \
    -Iutils/tflite_micro/tensorflow/lite/micro/tools/make/downloads/ruy/ \
    -Iutils/tflite_micro/tensorflow/lite/kernels/internal/

# subdir-ccflags-y += -std=c++11 -fno-rtti -fno-exceptions -fno-threadsafe-statics -Wnon-virtual-dtor -march=armv8.1-m.main+mve.fp+fp.dp \
#                     -mfpu=auto -mfloat-abi=hard -mthumb -fshort-enums -fshort-wchar -MD -MP -fno-unwind-tables -ffunction-sections \
#                     -fdata-sections -fmessage-length=0 -Wsign-compare -Wshadow -Wunused-variable -Wunused-function \
#                     -Wswitch -Wvla -Wall -Wextra -Wmissing-field-initializers -Wstrict-aliasing -Wno-unused-parameter -O2

# subdir-ccflags-y += -DTF_LITE_STATIC_MEMORY -DCMSIS_NN -DTF_LITE_DISABLE_X86_NEON -DTF_LITE_USE_CTIME -DNDEBUG -DTF_LITE_STRIP_ERROR_STRINGS

subdir-ccflags-y += -DTF_LITE_STATIC_MEMORY -DCMSIS_NN -DTF_LITE_DISABLE_X86_NEON -DTF_LITE_USE_CTIME -DNDEBUG

# TEST_TIME := 1

ifeq ($(TEST_TIME), 1)
    subdir-ccflags-y += -DTEST_TIME
endif

# list_library_sources:
# 	@echo $(obj_cc)


# libtflite_micro_lib_m55-y := $(obj-y)
# obj-y := libtflite_micro_lib_m55.a