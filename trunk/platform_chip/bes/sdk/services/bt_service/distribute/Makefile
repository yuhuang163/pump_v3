cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c :=

ifeq ($(BT_SERVICE_DISTRIBUTE_CLIENT),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)client/src/*.c))
endif

ifeq ($(BT_SERVICE_DISTRIBUTE_SERVER),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)server/src/*.c))
endif

ifeq ($(BT_SERVICE_DISTRIBUTE_TRANSPORT_ECOMM),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)transport/ecomm/*.c))
endif

ifeq ($(BT_SERVICE_DISTRIBUTE_TRANSPORT_INTERSYS),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)transport/intersys/*.c))
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    -Iservices/audio_process \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/voicepath/ \
    -Iservices/voicepath/$(VOICE_DATAPATH_TYPE) \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iutils/retention_ram \
    -Iservices/audioflinger \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iutils/heap \
    -Iutils/crc16 \
    -Iapps/battery \
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/factory \
    -Iservices/bridge/ \
    -Imultimedia/inc/speech/inc \
    -Iservices/bt_app \
    -Iservices/voicepath/gsound/gsound_custom/inc \
    -Iservices/voicepath/gsound/gsound_custom/src \
    -Iservices/voicepath/gsound/gsound_target_api_read_only \
    -Iservices/ai_voice/manager \
    -Iservices/ai_voice/transport \
    -Iservices/app_ai/inc \
    -Imultimedia/inc/audio/codec/sbc/inc \
    -Imultimedia/audio/codec/sbc/src/inc \
    -Iservices/ux/common/ibrt/ibrt_middleware/inc \
    -Iservices/bt_app/a2dp_codecs/include  \
    -Ithirdparty/tile \
    -Ithirdparty/tile/tile_target \
    -Ithirdparty/tile/tile_common/tile_assert \
    -Ithirdparty/tile/tile_common/tile_features \
    -Ithirdparty/tile/tile_common/tile_gatt_db \
    -Ithirdparty/tile/tile_common/tile_player \
    -Ithirdparty/tile/tile_common/tile_service \
    -Ithirdparty/tile/tile_common/tile_storage \
    -Iservices/voicepath/gsound/gsound_target \
    -I$(BT_SERVICE_INCLUDES) \
    -Iservices/app_bt_source \
    -Iservices/app_bt_source/inc \
    $(ECOMM_INCLUDES) \
    -Iutils/transport_if