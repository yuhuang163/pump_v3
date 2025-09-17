cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/audio_bt \
    -Iservices/audioflinger \
    -Iservices/audio_process \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Imultimedia/inc/audio/process/resample/include \
    -Imultimedia/inc/audio/process/filters/include \
    -Imultimedia/inc/audio/codec/sbc/inc \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iapps/main \
    -Iapps/key \
    -Iapps/common \
    -Iapps/battery \
    -Iservices/ux/common/ibrt/ibrt_middleware/inc \
    -Iservices/app_bt_source/inc \
    -Iservices/app_bt_source/src/a2dp_encoder_cp \
    -Iservices/bt_app/ \
    -Iservices/osif/ \
    -Iservices/tota/ \
    -Iservices/lea_player/inc/ \
    -Iservices/lea_player/usb_audio/ \
    -Ithirdparty/audio_codec_lib/liblhdc-enc/inc \
    $(AAC_INCLUDES) \
    -Iplatform/drivers/cp_accel \
    -Iservices/norflash_api \
    -Itests/anc_usb \
    $(BLE_APP_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    -Iutils/list \
    -Iutils/hsm \
    -Imultimedia/inc/audio/codec/lc3/inc \
    -Iservices/bt_sync \
    -Iapps/main \
    -Iservices/ble_audio/ble_audio_core/inc \
    -Ibthost/porting
