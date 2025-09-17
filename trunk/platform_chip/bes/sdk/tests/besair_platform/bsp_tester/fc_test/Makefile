cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cc := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cc))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y += $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cc:.cc=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    -Iinclude/cmsis_dsp/ \
    -Iinclude/cmsis_nn/ \
    -Iutils/heap/ \
	-Imultimedia/inc/speech/inc \
    -Itests/besair_platform/bsp_tester/fc_test/inc/ \
    -Iutils/tflite_micro \
    -Iutils/tflite_micro/tensorflow/lite/micro/tools/make/downloads/flatbuffers/include/

subdir-ccflags-y += -DTF_LITE_STATIC_MEMORY -DCMSIS_NN

ifeq ($(PRINT), 1)
    subdir-ccflags-y += -DPRINT
endif