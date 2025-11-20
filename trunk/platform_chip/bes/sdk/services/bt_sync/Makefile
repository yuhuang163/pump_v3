cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))


obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iapps/key \
	-Iplatform/drivers/bt  \
	-Iplatform/hal  \
	-Iservices/bt_app/a2dp_codecs/include 	\
	-Iservices/ux/common/ibrt/ibrt_middleware/inc   \
	-Iutils/cqueue \
	-Iservices/audio_manager \
	-Iapps/audioplayers/a2dp_decoder \
    -Ibthost/service/ble_app_new/inc

ifeq ($(IBRT),1)
ifeq ($(IBRT_UI),1)
ccflags-y += \
    -Iservices/ux/common/ibrt/app_ibrt/inc \
    -Iservices/ux/common/ibrt/ibrt_core/inc  \
    -Iservices/ux/common/ibrt/custom_api \
    -Iservices/ux/common/ibrt/ibrt_ui/inc \
    -Iservices/ux/product/earbud_tws/inc
endif
endif

ifeq ($(BLE_AUDIO_ENABLED),1)
ccflags-y += \
    -Iservices/audio_manager
endif