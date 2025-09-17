obj-y := common/ main/ key/ battery/ pwl/ cmd/

ifneq ($(BTH_SUBSYS_ONLY),1)
obj-y += \
    audioplayers/   \
    audio_test/
endif

ifeq ($(FACTORY_MODE),1)
obj-y += factory/
endif

ifeq ($(CAPSENSOR_SPP_SERVER),1)
obj-y += app_spp/
endif

ifeq ($(APP_TEST_AUDIO),1)
obj-y += apptester/
endif

ifneq ($(filter 1,$(BTUSB_AUDIO_MODE) $(BT_USB_AUDIO_DUAL_MODE) $(BLE_USB_AUDIO_SUPPORT)),)
obj-y += usbaudio/
endif

ifeq ($(BLE), 1)
ifeq ($(BLE_AUDIO_ENABLED),1)
obj-y += ble_audio_test/
endif
endif

ifeq ($(USB_DISK_TEST),1)
obj-y += usbhost/
ccflags-y += -DUSB_DISK_TEST
endif

ifeq ($(APP_TEST_SDMMC),1)
obj-y += sdmmc/
endif

ifeq ($(MSD_MODE),1)
obj-y += sdmmc_msd/
endif

ifeq ($(PSAP_SW_APP),1)
obj-y += psap_sw/
endif

ifeq ($(ANC_APP),1)
obj-y += anc/
endif

ifeq ($(ANC_ASSIST_ENABLED),1)
obj-y += anc_assist/
obj-y += voice_assist/
endif

ifeq ($(VOICE_DETECTOR_EN),1)
obj-y += voice_detector/
endif

ifeq ($(SENSOR_HUB),1)
obj-y += sensorhub/
endif

ifeq ($(DSP_HIFI4),1)
obj-y += dsp_hifi4/
endif

ifeq ($(DSP_M55),1)
obj-y += dsp_m55/
endif

ifeq ($(DMA_AUDIO_APP)-$(DMA_RPC_CLI),1-1)
obj-y += ../services/dma_audio/app/
endif

ifeq ($(MTEST_BUILTIN),1)
obj-y += mem_scan/
endif

ifeq ($(AUDIO_HEARING_COMPSATN),1)
obj-y += hearing_detec/
endif

_APP_BT_WATCH ?= 0
ifeq ($(BT_WATCH_MASTER),1)
_APP_BT_WATCH := 1
endif
ifeq ($(BT_WATCH_SLAVE),1)
_APP_BT_WATCH := 1
endif
ifeq ($(_APP_BT_WATCH),1)
obj-y += btwatch/
endif

ifeq ($(APP_RPC_ENABLE),1)
obj-y += app_rpc/
endif

ifeq ($(APP_SOUND_ENABLE),1)
obj-y += sound/
endif

ifeq ($(BLE_WALKIE_TALKIE),1)
obj-y += ble_walkie_talkie/
endif

ifeq ($(BTH_IF_COMM_RPC_EN),1)
obj-y += bth_if/
endif

ifeq ($(A7_DSP_ENABLE),1)
obj-y += mic/
endif

ifeq ($(VOWLAN_SUPPORT),1)
obj-y += vowlan/
endif

ifeq ($(WIRELESS_MIC),1)
obj-y += wireless_mic/
endif

ifeq ($(BES_OTA),1)
obj-y += ota/
endif

ifeq ($(ANDROID_TEST),1)
obj-y += android_test/
endif

# include earbud in default
ifneq ($(BT_WATCH_APP),1)
ifeq ($(IBRT_UI),1)
obj-y += earbuds/
endif
endif

export APP_SPP_DEMO ?= 0
ifeq ($(APP_SPP_DEMO),1)
obj-y += example/app_spp/
endif

subdir-ccflags-y += -Iapps/apptester \
					-Iapps/audioplayers \
					-Iapps/anc/inc \
					-Iapps/voice_assist/inc \
					-Iapps/common \
					-Iapps/sdmmc \
					-Iapps/sdmmc_msd \
					-Iapps/main \
					-Iapps/cmd \
					-Iapps/key \
					-Iapps/pwl \
					-Iapps/battery \
					-Iservices/voicepath \
					-Iservices/audio_dump/include \
					$(BLUETOOTH_ADAPTER_INCLUDES) \
					-Iutils/heap \
					-Imultimedia/inc/audio/process/filters/include 

ifeq ($(BT_USB_AUDIO_DUAL_MODE),1)
subdir-ccflags-y += -Iapps/usbaudio/bt_usb_audio
endif

ifeq ($(A2DP_LDAC_ON),1)
subdir-ccflags-y += -Ithirdparty/audio_codec_lib/ldac/inc
endif

ifneq ($(HCI_BRIDGE_MODE),0)
subdir-ccflags-y += -Iapps/common/hci_bridge
endif

ifeq ($(APP_SAMPLES),1)
obj-y += samples/
subdir-ccflags-y += -Iapps/samples
endif
