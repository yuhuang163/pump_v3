cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y +=  \
	-Iservices/nv_section/userdata_section \
	-Ibthost/adapter/inc/bt \
	-Ibthost/adapter/inc/bt/common \
	-Ibthost/adapter/inc/adapter_service \
	-Ibthost/stack/bt_profiles/inc \
	-Ibthost/adapter/inc/bt/source \
	-Iutils/boot_struct \
    -Iutils/intersyshci \
    -Iutils/hwtimer_list \
    -Iutils/list \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iservices/audio_bt \
    -Iservices/resources \
    -Iservices/overlay \
    -Imultimedia/inc/algorithm/fft/include \
    -Iapps/key \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/usb/usb_dev/inc \
	-Iapps/battery \
	-Imultimedia/inc/audio/codec/sbc/inc \
	-Imultimedia/audio/codec/sbc/src/inc \
    -Iutils/cqueue \
    -Iservices/nv_section/factory_section \
    -Iinclude/ \
    -Iservices/app_ai/inc \
	-Iservices/audio_manager \
	-Iapps/pwl