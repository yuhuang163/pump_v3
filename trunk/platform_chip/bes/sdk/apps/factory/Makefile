cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y +=  \
    -Iutils/boot_struct \
    -Iutils/intersyshci \
    -Iutils/hwtimer_list \
    -Iutils/list \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iservices/audio_bt \
    -Iservices/resources \
    -Iservices/overlay \
    -Imultimedia/inc/algorithm/fft/include \
    -Iapps/key \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/usb/usb_dev/inc \
	-Iapps/battery \
	-Imultimedia/inc/audio/codec/sbc/inc \
	-Imultimedia/audio/codec/sbc/src/inc \
    -Iutils/cqueue \
    -Iservices/nv_section/factory_section \
    -Iinclude/ \
    -Iservices/app_ai/inc \
	-Iservices/audio_manager

ifeq ($(BLE_AUDIO_ENABLED),1)
ccflags-y += \
    -Iservices/audio_manager
endif

ifeq ($(AUDIO_RESAMPLE),1)
CFLAGS_app_factory_audio.o += -D__AUDIO_RESAMPLE__
endif
ifeq ($(SW_CAPTURE_RESAMPLE),1)
CFLAGS_app_factory_audio.o += -DSW_CAPTURE_RESAMPLE
endif

ifeq ($(POWERKEY_I2C_SWITCH),1)
CFLAGS_app_factory.o += -DPOWERKEY_I2C_SWITCH
endif

ifeq ($(BTH_SUBSYS_ONLY),1)
CFLAGS_app_factory.o += -DBTH_SUBSYS_ONLY
endif

ifeq ($(AUDIO_QUEUE_SUPPORT),1)
CFLAGS_app_factory.o += -D__AUDIO_QUEUE_SUPPORT__
endif

ifeq ($(SPEECH_TX_AEC_CODEC_REF),1)
CFLAGS_app_factory_audio.o += -DSPEECH_TX_AEC_CODEC_REF
endif

ifeq ($(BT_SIGNALTEST_SLEEP_EN),1)
CFLAGS_app_factory_bt.o += -DBT_SIGNALTEST_SLEEP_EN
endif

