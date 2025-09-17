cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))

ifeq ($(BTH_TRC_TO_SYS),1)
ccflags-y += -DBTH_TRC_TO_SYS
obj-y += ../../../tests/bt_host/bth_trc_to_sys.c
ccflags-y += \
    -Iplatform/hal \
    -Iplatform/drivers/rmt_trace \
    -Itests/bt_host
ifeq ($(RMT_TRC_IN_MSG_CHAN),1)
ccflags-y += -DRMT_TRC_IN_MSG_CHAN
endif
endif

ifeq ($(APP_COREMARK_TEST),1)
obj-y += ../../../tests/coremark/
ccflags-y += -Itests/coremark/ \
			 -Itests/hwtest/ \
			 -Iutils/hwtimer_list
ccflags-y += -DAPP_COREMARK_TEST
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y +=  \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(EPLAYER_INCLUDES) \
    $(ECOMM_INCLUDES) \
    -Iservices/audioflinger \
    -Iservices/norflash_api \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/log_section \
    -Iservices/overlay \
    -Iservices/resources \
    -Iapps/apptester \
    -Iapps/factory \
    -Iutils/crc32 \
    -Iutils/list \
    -Iutils/codec_calib \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Itests/anc_usb \
    -Iapps/anc/inc \
    -Ithirdparty/userapi \
    -Iservices/communication \
    -Iutils/cqueue \
    -Iservices/ai_voice/audio \
    -Iservices/ai_voice/manager \
    -Iservices/ai_voice/transport \
    -Iservices/app_ai/inc \
    -Iservices/bridge/ \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/bes_bth/ble/aob_app/inc \
    -Iservices/aob_bes_ux/inc \
    -Iservices/aob_custom_ux \
    -Iinclude/ \
    -Iapps/btwatch \
    -Iapps/sensorhub \
    -Iapps/dsp_m55 \
    -Iplatform/drivers/cp_accel \
    -Iplatform/drivers/hifi4 \
    -I./utils/codec_calib   \
    -Iapps/mem_scan


ifeq ($(BLE_USB_AUDIO_SUPPORT),1)
ccflags-y += \
    -Iservices/ble_usb_audio \
    -Iservices/audio_sync_tdm_codec/inc
endif

ifneq ($(DEBUG_PORT),)
ccflags-y += -DDEBUG_PORT=$(DEBUG_PORT)
endif

ifeq ($(BLE_WALKIE_TALKIE),1)
ccflags-y += \
    -Iapps/ble_walkie_talkie \
    -Iservices/walkie_talkie \
    -Iservices/walkie_talkie/wt_test
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

ifeq ($(BLE_AUDIO_ENABLED),1)
ccflags-y += \
    -Iservices/audio_manager
endif

CFLAGS_apps_watch.o += $(LDS_SECTION_FLAGS)

ifeq ($(ENGINEER_MODE),1)
CFLAGS_apps_watch.o += -D__ENGINEER_MODE_SUPPORT__
endif

ifeq ($(MCU_HIGH_PERFORMANCE_MODE),1)
CFLAGS_apps_watch.o += -DMCU_HIGH_PERFORMANCE_MODE
endif

ifeq ($(BTADDR_FOR_DEBUG),1)
CFLAGS_apps_watch.o += -DBTADDR_FOR_DEBUG
endif

ifeq ($(FORCE_SIGNALINGMODE),1)
CFLAGS_apps_watch.o += -DFORCE_SIGNALINGMODE
endif

ifeq ($(FORCE_NOSIGNALINGMODE),1)
CFLAGS_apps_watch.o += -DFORCE_NOSIGNALINGMODE
endif

ifeq ($(UTILS_ESHELL_EN),1)
ccflags-y += \
    $(ESHELL_INCLUDES)
endif

ifneq ($(filter 1,$(APP_RPC_ENABLE) $(APP_RPC_MCU_SENSOR_EN)),)
ccflags-y += -Iapps/app_rpc
ccflags-y += -Iapps/app_rpc/rpc_bth_dsp
endif

ifeq ($(APP_RPC_TEST),1)
ccflags-y += -Iapps/app_rpc/test
endif

ifeq ($(FLASH_UNIQUE_ID),1)
CFLAGS_apps_watch.o += -DFLASH_UNIQUE_ID
endif

ifeq ($(BTH_IF_COMM_RPC_EN),1)
ccflags-y += \
    -Iapps/bth_if \
    -Iapps/bth_if/bt_if_server
endif
