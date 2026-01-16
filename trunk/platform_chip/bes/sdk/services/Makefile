obj-y := \
    ../utils/crc32/     \
    ../utils/crc16/     \
    ../utils/heap/      \
    osif/               \
    overlay/            \
    nv_section/         \
    norflash_api/

ifneq ($(BTH_SUBSYS_ONLY),1)
obj-y +=            \
    audio_bt/       \
    audio_dump/     \
    audioflinger/   \
    audio_process/  \
    hw_dsp/         \
    bt_sync/        \
    communication/  \
    audio_manager/
endif

ifeq ($(SPEECH_BONE_SENSOR),1)
obj-y += bone_sensor/
endif

ifeq ($(BINAURAL_RECORD_PROCESS),1)
obj-y += binaural_record_process/
endif

ifneq ($(filter 1,$(INTERCONNECTION) $(INTERACTION) $(AI_VOICE)),)
obj-y += interconnection/
endif

obj-y += ../bthost/

ifeq ($(BTHCI_ENABLE),1)
obj-y += bthci/
endif

ifeq ($(MBED),1)
obj-y += fs/
endif

ifneq ($(filter 1, $(MBED) $(AI_VOICE) $(IBRT) $(TOTA_v2)),)
obj-y += ../utils/kfifo/
endif

ifeq ($(VOICE_DATAPATH_ENABLED),1)
obj-y += voicepath/
obj-y += voicepath/$(VOICE_DATAPATH_TYPE)
endif

ifneq ($(filter 1, $(VOICE_DATAPATH_ENABLED) $(AI_VOICE)),)
obj-y += app_ai/
endif

ifneq ($(IBRT_UI),1)
obj-y += ../utils/hsm/
endif

ifeq ($(AUTO_TEST),1)
obj-y += auto_test/
endif

ifeq ($(VOICE_RECOGNITION),1)
obj-y += voice_recognition/
endif

ifeq ($(APP_ANC_TEST),1)
obj-y += anc_spp_tool/
endif

ifeq ($(TEST_OVER_THE_AIR),1)
obj-y += tota/
endif

ifeq ($(TEST_OVER_THE_AIR_v2),1)
obj-y += tota_v2/
endif

ifeq ($(OTA_ENABLE),1)
ifeq ($(FPGA_IBRT_OTA),1)
obj-y += fpga_ibrt_ota/
else
obj-y += ota/
endif
endif

ifeq ($(TILE_DATAPATH_ENABLED),1)
obj-y += ../thirdparty/tile/
endif

ifeq ($(AI_VOICE),1)
obj-y += ai_voice/
endif


ifeq ($(GFPS_ENABLE),1)
obj-y += gfps/
endif

ifeq ($(IBRT),1)
obj-y += ../utils/hsm/
obj-y += ux/common/ibrt/app_ibrt/
obj-y += ux/common/config/
endif

ifneq ($(filter 1,$(BLE_AUDIO_ENABLED) $(IBRT)),)
obj-y += ux/
endif

ifeq ($(RSA_SHA),1)
obj-y +=../utils/sha256/
endif

ifeq ($(AES256_FOR_BIN),1)
obj-y +=../utils/aes256/
endif

ifeq ($(THROUGH_PUT),1)
obj-y += through_put/
endif

ifeq ($(EPLAYER_ON),1)
obj-y += eplayer/eplayer/
ifeq ($(EPLAYER_TEST),1)
obj-y += eplayer/tests/
endif
endif

ifeq ($(ETRANSPORT_ON),1)
obj-y += etransport/
endif

ifeq ($(ECOMM_ON),1)
obj-y += ecomm/
endif

ifeq ($(BT_SERVICE_ON),1)
ifeq ($(BT_SERVICE_NATIVE),1)
obj-y += bt_service/native/
endif
ifeq ($(BT_SERVICE_DISTRIBUTE),1)
obj-y += bt_service/distribute/
endif
ifeq ($(BT_SERVICE_TEST),1)
obj-y += bt_service/tests/
endif
endif

ifeq ($(EAUDIO_ON),1)
obj-y += eaudio/
obj-y += eaudio/device/
obj-y += eaudio/policy/
obj-y += eaudio/effect/
obj-y += eaudio/resample/
obj-y += eaudio/heap/
ifeq ($(EAUDIO_TEST),1)
obj-y += eaudio/tests/
endif
endif

ifeq ($(TDM_RELAY),1)
obj-y += tdm_relay/
endif

ifeq ($(BLE_WALKIE_TALKIE),1)
obj-y += walkie_talkie/
endif

ifeq ($(MTP_ENABLE),1)
obj-y += mtp/
endif

ifeq ($(SPEECH_ALGO_DSP_TEST),1)
obj-y += speech_algo_dsp_test/
endif

ifeq ($(ACTIVITY_MANAGER_ENABLE),1)
obj-y += activity_manager/
endif

ifneq ($(APP_MCPP_CLI),)
obj-y += mcpp/
endif

ifeq ($(CAPSENSOR_ENABLE),1)
obj-y += capsensor/
endif

ifeq ($(BLE),1)
ifeq ($(BLE_AUDIO_ENABLED),1)
obj-y += lea_player/
endif
endif

ifeq ($(OPENAMP_ENABLE),1)
obj-y += openamp/
endif

ifeq ($(HEAD_TRACK_ENABLE),1)
obj-y += head_track/
endif

ifeq ($(SERVICE_GRABOX_ENABLE),1)
obj-y += grabox/
endif
