cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))


obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y +=  \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iservices/audioflinger \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/audio_process \
    -Iapps/apptester \
    -Iapps/factory \
    -Iutils/crc32 \
    -Iplatform/drivers/bt \
    -Iplatform/hal \
    -Iplatform/drivers/ana \
    -Iapps/audioplayers/rbplay \
    -Itests/anc_usb \
    -Iapps/anc/inc \
    -Iapps/ota \
    -Ithirdparty/userapi \
    -Iservices/voicepath \
    -Iservices/voicepath/gsound/gsound_service \
    -Iservices/voicepath/gsound/gsound_target \
    -Iservices/communication \
    -Iutils/cqueue \
    -Iservices/ai_voice/ama/ama_manager \
    -Iservices/ai_voice/manager \
    -Imultimedia/inc/audio/codec/sbc/inc \
    -Imultimedia/audio/codec/sbc/src/inc \
    -Iservices/interconnection \
    $(EPLAYER_INCLUDES) \
    $(ECOMM_INCLUDES) \
    $(EAUDIO_INCLUDES) \
    $(BT_SERVICE_INCLUDES) \
    -Iutils/hwtimer_list \
    -Iservices/nv_section/factory_section \
    -Iapps/audioplayers/a2dp_decoder \
    -Iservices/audio_bt \
    -Iplatform/drivers/bt/ \
    -Iservices/custom_api \
    -Ibthost/adapter/inc/bt/common \
    -Iservices/ux/common/ibrt/ibrt_middleware/inc \
    -Iservices/ux/common/ibrt/ibrt_core/inc \
    -Iservices/ux/common/ibrt/ibrt_ui/inc \
    -Iservices/ux/common/ibrt/app_ibrt/inc \
    -Iservices/ux/product/earbud_tws/inc \
    -Iapps/earbuds/conn \
    -Iutils/hsm \
    -Iservices/ble_audio/ble_audio_core/inc \
    -Ibthost/service/ble_audio/aob_app/inc
