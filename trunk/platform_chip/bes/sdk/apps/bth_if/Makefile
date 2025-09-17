cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))

#bthost server include files
ifneq ($(filter bth sensor_hub,$(CHIP_SUBSYS)),)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)bt_if_server/*.c))
else
#bthost client include files
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)bt_if_client/*.c))
ifeq ($(BLE_WALKIE_TALKIE),1)
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)bt_if_client/bth_walkie_talkie_stream.cpp))
endif
#tests folder
ifeq ($(BTH_IF_TEST), 1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)tests/*.c))
endif
endif #end if $(CHIP_SUBSYS)

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(filter bth sensor_hub,$(CHIP_SUBSYS)),)
ifeq ($(BTH_IF_CUSTOM_API),1)
obj-y += bt_if_custom_api/
endif
endif

subdir-ccflags-y += \
    -Iutils/list \
    -Iutils/heap \
    -Iutils/cqueue \
    -Iutils/kfifo \
    -Iutils/hwtimer_list \
    -Iapps/app_rpc \
    -Iapps/app_rpc/rpc_bth_dsp \
    -Iapps/dsp_m55 \
    -Iapps/sensorhub \
    -Iapps/ble_walkie_talkie \
    -Iservices/walkie_talkie \
    -Iapps/common \
    -Iinclude/watchdog \
    -Iplatform/hal \
    -Iplatform/drivers/ana \
    -Iapps/bth_if \
    -Iutils/eshell \
    -Iservices/ota/bes_ota/inc \
    -Ibthost/inc/bt/common \
    -Ibthost/inc/bt/bt_app \
    -Iservices/overlay \
    -Imultimedia/inc/speech/inc/ \
    -Imultimedia/audio/smf/plugin/sbc_c/ \
    -Itests/hwtest \
    $(ECOMM_INCLUDES) \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_STACK_INCLUDES)

ifneq ($(filter bth sensor_hub,$(CHIP_SUBSYS)),)
subdir-ccflags-y += \
    -Iapps/bth_if/bt_if_server \
    -Iservices/audio_manager \
    -Iplatform/drivers/bt
else
subdir-ccflags-y += \
    -Iapps/bth_if/bt_if_client/ \
    -Iapps/audioplayers/
subdir-ccflags-y += \
    -Iservices/aiot/lplayer/component/audio_sink/include \
    -Iservices/aiot/lplayer/component/include \
    -Imultimedia/inc/audio/smf/core/inc/ \
    -Imultimedia/inc/audio/smf/core/include/ \
    -Iservices/audio_dump/include/ \
    -Iservices/mcpp/bes/ \
    -Imultimedia/inc/speech/inc \
    -Iservices/overlay/
endif

