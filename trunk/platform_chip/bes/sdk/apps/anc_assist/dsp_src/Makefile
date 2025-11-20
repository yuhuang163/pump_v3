cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

subdir-ccflags-y += \
    -Iplatform/hal_trace \
    -Imultimedia/inc/speech/inc \
    -Iapps/common \
    -Iapps/anc/inc \
    -Iapps/app_rpc \
    -Iapps/app_rpc/rpc_bth_dsp \
    -Iapps/anc_assist/inc \
    -Iapps/anc_assist/src \
    -Iapps/anc_assist/src/assist \
    -Iapps/dsp_m55 \
    -Iapps/voice_assist/inc \
    -Iapps/anc/src/assist \
    -Iapps/voice_assist/src \
    -Iutils/list \
    -Iutils/heap
