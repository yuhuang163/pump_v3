cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(IBRT_UI),1)
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)conn/*c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)conn/*.cpp))
endif

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)test/*c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)test/*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iservices/osif \
    -Iservices/nv_section/factory_section \
    -Iservices/app_ai/inc \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/sensor_hub \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iservices/audioflinger \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iutils/list \
    -Iapps \
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Iapps/factory \
    -Iservices/nv_section/log_section \
    -Iservices/audio_bt/ \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/voicepath \
    -Iservices/ux/common/ibrt/ibrt_middleware/inc \
    -Iservices/ai_voice/protocol/gma/gma_manager \
    -Iservices/ai_voice/protocol/gma/gma_crypto \
    -Iservices/ai_voice/protocol/dual_mic_recording/voice_manager \
    -Iservices/ai_voice/protocol/bixbyvoice \
    -Iservices/ai_voice/protocol/bixbyvoice/bixbyvoice_manager \
    -Iservices/ai_voice/protocol/dual_mic_recording/voice_manager \
    -Iservices/ai_voice/codec/compression \
    -Iservices/ai_voice/codec/opus121/include \
    -Iservices/norflash_api \
    -Iservices/bt_app \
    -Iservices/audio_manager \
    -Iservices/audio_manager/test \
    -Iservices/bt_sync \
    -Iutils/hsm \
    -Iservices/ux/common/ibrt/custom_api \
    -Iservices/ux/common/ibrt/app_ibrt/inc \
    -Iservices/ux/common/ibrt/ibrt_core/inc \
    -Iservices/ux/product/earbud_tws/inc \
    -Iapps/earbuds/conn/

ifeq ($(IBRT_UI),1)
subdir-ccflags-y += \
    -Iservices/ux/common/ibrt/ibrt_ui/inc \
    -Iapps/earbuds/conn/
endif

ifeq ($(BLE_AUDIO_ENABLED),1)
subdir-ccflags-y += \
    -Iservices/ux/common/ble_audio_core/inc \
    -Iservices/log_manager
endif

ifeq ($(BES_OTA),1)
subdir-ccflags-y += \
    -Iservices/ota/bes_ota/inc
endif

ifeq ($(A2DP_AUDIO_STEREO_MIX_CTRL),1)
CFLAGS_app_ibrt_customif_ui.o += -DA2DP_AUDIO_STEREO_MIX_CTRL
endif

ifeq ($(SPA_AUDIO_ENABLE),1)
subdir-ccflags-y += -Ithirdparty/userapi/spa_dummy_app/inc
endif

ifeq ($(GFPS_ENABLE),1)
subdir-ccflags-y += \
    -Iservices/gfps/inc \
    -Ibthost/porting
endif


GPIO_WAKEUP_ENABLE ?= 0
ifeq ($(GPIO_WAKEUP_ENABLE),1)
subdir-ccflags-y += -DGPIO_WAKEUP_ENABLE
endif