cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    -Iutils/list \
    -Iutils/heap \
    -Iutils/cqueue \
    -Iutils/kfifo \
    -Iutils/hwtimer_list \
    -Iapps/app_rpc \
    -Iapps/app_rpc/rpc_bth_dsp \
    -Iapps/dsp_m55 \
    -Iapps/sensorhub \
    -Iplatform/hal \
    -Iapps/bth_if \
    -Iutils/eshell \
    -Ibthost/adapter/inc/bt/common \
    -Iapps/bth_if/bt_custom_api \
    $(ECOMM_INCLUDES) \
    $(BLUETOOTH_ADAPTER_INCLUDES)

    ccflags-y += \
    -Iapps/bth_if/bt_if_client




