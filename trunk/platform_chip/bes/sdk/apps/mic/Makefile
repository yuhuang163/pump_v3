cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
	-Iservices/audio_process \
    -Iservices/app_ai/inc \
	-Iservices/bt_app \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iservices/overlay \
	-Iservices/resources \
	-Imultimedia/inc/audio/process/resample/include \
	-Imultimedia/inc/audio/process/filters/include \
	-Imultimedia/inc/audio/process/drc/include \
	-Imultimedia/inc/audio/process/anc/include\
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/factory_section \
	-Iservices/nv_section/include   \
	-Iplatform/drivers/uarthci \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/bt \
	-Iutils/cqueue \
	-Iservices/audioflinger \
	-Iutils/lockcqueue \
	-Iutils/intersyshci \
	-Iapps/key \
	-Iapps/main \
	-Iapps/common \
	-Iapps/audioplayers \
	-Iapps/audioplayers/rbplay \
	-Iapps/factory \
	-Iservices/ble_app \
	-Iservices/tws/inc \
	-Iutils/hwtimer_list \
	-Iservices/ble_stack/ble_ip \
	-Iservices/ble_stack/hl/api \
	-Iservices/ble_stack/app/api/  \
	-Iservices/ble_stack/common/api/ \
	-Iservices/ble_stack/hl/inc/ \
	-Iservices/ble_stack/ke/api \
	-Iservices/ble_app/app_tws \
	-Iapps/wings \
	-Iapps/battery \
	-Iapps/ota \
	-Iapps/mic \
	-Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
	-Ithirdparty/cyberon/cspotter/src \
	-Iutils/crc32 \
	-Imultimedia/inc/audio/codec/sbc/inc \
	-Imultimedia/audio/codec/sbc/src/inc \
	-Iservices/bt_app/a2dp_codecs/include

ifeq ($(BLE_AUDIO_ENABLED),1)
ccflags-y += \
    -Iservices/audio_manager
endif