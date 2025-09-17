cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(BT_WATCH_APP),1)
obj-y := watch_src/
else
obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
endif

ifeq ($(BTH_BOOT_AP),1)
obj-y += ../../platform/drivers/fw_linux/loader/
endif

ifneq ($(wildcard $(cur_dir)../../config/$(T)/app_status_ind.c),)
obj-y += ../../config/$(T)/app_status_ind.c
else
obj-y += ../../config/$(DEFAULT_CFG_SRC)/app_status_ind.c
endif

ifneq ($(filter 1,$(AUDIO_ADC_DC_AUTO_CALIB) $(AUDIO_OUTPUT_DC_AUTO_CALIB)),)
obj-y += ../../utils/codec_calib/
ccflags-y += -Iutils/codec_calib
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

subdir-ccflags-y +=  \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(EPLAYER_INCLUDES) \
    $(ECOMM_INCLUDES) \
    -Iservices/audio_bt \
    -Iservices/audioflinger \
    -Iservices/norflash_api \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/log_section \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/activity_manager \
    -Iservices/audio_process \
    -Iapps/apptester \
    -Iapps/factory \
    -Iapps/voice_detector \
    -Iservices/ux/common/sound \
    -Iutils/crc32 \
    -Iutils/list \
    -Iutils/codec_calib \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iapps/audioplayers/rbplay \
    -Itests/anc_usb \
    -Iapps/anc/inc \
    -Ithirdparty/userapi \
    -Iservices/communication \
    -Iutils/cqueue \
    -Iservices/ai_voice/audio \
    -Iservices/ai_voice/manager \
    -Iservices/ai_voice/transport \
    -Iservices/app_ai/inc \
    -Iservices/interconnection/red \
    -Iservices/interconnection/green \
    -Iservices/bridge/ \
    -Iservices/voicepath \
    -Iservices/voicepath/gsound/gsound_service \
    -Iservices/voicepath/gsound/gsound_target \
    -Iservices/voicepath/gsound/gsound_custom/inc \
    -Iservices/voicepath/gsound/gsound_target_api_read_only \
    -Iservices/ota/ai_ota/ \
    -Iservices/bes_bth/ble/aob_app/inc \
    -Iservices/aob_bes_ux/inc \
    -Iservices/aob_custom_ux \
    -Ithirdparty/tile \
    -Iservices/ux/common/ibrt/ibrt_middleware/inc \
    -Ithirdparty/tile/tile_target \
    -Iinclude/ \
    -Iapps/btwatch \
    -Iapps/sensorhub \
    -Iapps/dsp_hifi4 \
    -Iapps/dsp_m55 \
    -Iplatform/drivers/cp_accel \
    -Iplatform/drivers/hifi4 \
    -Iplatform/drivers/fw_linux/inc \
    -Iplatform/main \
    -Iapps/mem_scan \
    -Iservices/audio_manager

ifeq ($(BLE_USB_AUDIO_SUPPORT),1)
ccflags-y += \
    -Iservices/ble_usb_audio \
    -Iservices/audio_sync_tdm_codec/inc
endif

ifeq ($(IBRT),1)
ifeq ($(IBRT_UI),1)
ccflags-y += \
    -Iservices/ux/common/ibrt/app_ibrt/inc \
    -Iservices/ux/common/ibrt/ibrt_core/inc \
    -Iservices/ux/common/ibrt/custom_api \
    -Iservices/ux/common/ibrt/ibrt_ui/inc \
    -Iapps/earbuds/conn \
    -Iapps/earbuds/test
endif
endif

ifeq ($(CAPSENSOR_ENABLE), 1)
obj-y += \
    ../capsensor/

ccflags-y += \
    -Iapps/capsensor/

ifeq ($(CAPSENSOR_SPP_SERVER), 1)
ccflags-y += \
    -Iservices/capsensor/ \
    -Iplatform/drivers/capsensor/
endif

endif

ifeq ($(DUAL_MODE_BLUETOOTH_UI_ENABLED),1)
ccflags-y += \
    -Iservices/bluetooth_ui
endif

ifeq ($(BISTO_ENABLE),1)
ccflags-y += \
    -Iservices/voicepath \
    -Iservices/voicepath/gsound/gsound_service \
    -Iservices/voicepath/gsound/gsound_target \
    -Iservices/voicepath/gsound/gsound_custom/inc \
    -Iservices/voicepath/gsound/gsound_target_api_read_only
endif

ifeq ($(BLE_WALKIE_TALKIE),1)
ccflags-y += \
    -Iapps/ble_walkie_talkie \
    -Iservices/walkie_talkie \
    -Iservices/walkie_talkie/wt_test
endif

ifeq ($(APP_TEST_AUDIO),1)
CFLAGS_apps_tester.o += -DAPP_TEST_AUDIO
endif

ifeq ($(OTA_ENABLE),1)
ccflags-y += -Iservices/ota
endif

ifeq ($(BES_OTA),1)
ccflags-y += -Iservices/ota/bes_ota/inc
endif

ifeq ($(AI_OTA),1)
ccflags-y += -Iservices/ota/ai_ota
endif

ifeq ($(WIRELESS_MIC),1)
ccflags-y += -Iapps/wireless_mic
endif

CFLAGS_apps.o += $(LDS_SECTION_FLAGS)

ifeq ($(ANC_APP),1)
CFLAGS_apps_tester.o += -DANC_APP
CFLAGS_apps.o += -DANC_APP
endif

ifeq ($(VOICE_ASSIST_WD_ENABLED),1)
CFLAGS_apps.o += -DVOICE_ASSIST_WD_ENABLED
endif

ifeq ($(VOICE_ASSIST_PILOT_ANC_ENABLED),1)
CFLAGS_apps.o += -DVOICE_ASSIST_PILOT_ANC_ENABLED
endif

ifeq ($(RB_CODEC),1)
CFLAGS_apps.o += -DRB_CODEC
endif

ifeq ($(VOICE_PROMPT),1)
CFLAGS_apps.o += -DMEDIA_PLAYER_SUPPORT
endif

ifeq ($(ENGINEER_MODE),1)
CFLAGS_apps.o += -D__ENGINEER_MODE_SUPPORT__
endif

ifeq ($(MCU_HIGH_PERFORMANCE_MODE),1)
CFLAGS_apps.o += -DMCU_HIGH_PERFORMANCE_MODE
endif

ifeq ($(BTADDR_FOR_DEBUG),1)
CFLAGS_apps.o += -DBTADDR_FOR_DEBUG
endif

ifeq ($(FORCE_SIGNALINGMODE),1)
CFLAGS_apps.o += -DFORCE_SIGNALINGMODE
endif

ifeq ($(FORCE_NOSIGNALINGMODE),1)
CFLAGS_apps.o += -DFORCE_NOSIGNALINGMODE
endif

ifeq ($(POWERKEY_I2C_SWITCH),1)
CFLAGS_apps.o += -DPOWERKEY_I2C_SWITCH
endif

ifeq ($(UTILS_ESHELL_EN),1)
ccflags-y += \
    $(ESHELL_INCLUDES)
endif

ifeq ($(AUDIO_PROMPT_USE_DAC2_ENABLED),1)
CFLAGS_apps.o += -DAUDIO_PROMPT_USE_DAC2_ENABLED
endif

ifeq ($(AUDIO_ADAPTIVE_IIR_EQ),1)
CFLAGS_apps.o += -DAUDIO_ADAPTIVE_IIR_EQ
CFLAGS_apps.o += -DAUDIO_ADAPTIVE_EQ
endif

ifeq ($(AUDIO_ADAPTIVE_FIR_EQ),1)
CFLAGS_apps.o += -DAUDIO_ADAPTIVE_FIR_EQ
CFLAGS_apps.o += -DAUDIO_ADAPTIVE_EQ
endif

ifeq ($(BTH_SUBSYS_ONLY),1)
CFLAGS_apps.o += -DBTH_SUBSYS_ONLY
endif

ifeq ($(AUDIO_DEBUG_CMD),1)
CFLAGS_apps.o += -DAUDIO_DEBUG_CMD
endif

ifneq ($(filter 1,$(APP_RPC_ENABLE) $(APP_RPC_MCU_SENSOR_EN)),)
ccflags-y += -Iapps/app_rpc
endif

ifeq ($(APP_RPC_TEST),1)
ccflags-y += -Iapps/app_rpc/test
endif

ifeq ($(DMA_AUDIO_APP),1)
ccflags-y += -DDMA_AUDIO_APP -Iservices/dma_audio/app
endif

ifeq ($(SPEECH_ALGO_DSP_TEST),1)
CFLAGS_apps.o += -DSPEECH_ALGO_DSP_TEST
endif

ifeq ($(ANC_ASSIST_ENABLED),1)
ccflags-y += \
    -Iapps/anc_assist/inc
endif

ifeq ($(FLASH_UNIQUE_ID),1)
CFLAGS_apps.o += -DFLASH_UNIQUE_ID
endif

ifeq ($(BTH_IF_COMM_RPC_EN),1)
ccflags-y += \
    -Iapps/bth_if \
    -Iapps/bth_if/bt_if_server
endif

ifeq ($(BLE_BIS_TRANSPORT),1)
ccflags-y += \
    -Iservices/ux/common/bis_transport/inc
endif

ifeq ($(APP_UART_MODULE),1)
ccflags-y += -Iapps/common/uart_dma
endif

ifeq ($(APP_UART_BRIDGE_MODULE),1)
ccflags-y += -Iapps/common/uart_bridge
endif

ifeq ($(APP_SAMPLES),1)
subdir-ccflags-y += \
    -Iapps/samples
endif

ifeq ($(GFPS_ENABLE),1)
subdir-ccflags-y += \
    -Iservices/gfps/inc \
    -Ibthost/porting
endif
