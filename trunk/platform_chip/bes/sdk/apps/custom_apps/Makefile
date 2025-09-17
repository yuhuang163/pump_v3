cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))


obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y +=  \
    -Iservices/norflash_api \
    -Iservices/aiot/bt_rpc \
    -Iplatform/drivers/ana \
    -Iapps/common \
    -Iapps/main \
    -Iutils/heap \
    -Iapps/aiot_generic \
    -Iservices/nv_section/factory_section \
    -Iutils/hwtimer_list

ifeq ($(UTILS_ESHELL_EN),1)
ccflags-y += \
    -Iapps/app_eshell
endif

ifeq ($(ALSA_EN), 1)
ccflags-y += -Iservices/aiot/alsa_adapter
endif
ifeq ($(BTH_IF_COMM_RPC_EN), 1)
ccflags-y += -Iapps/bth_if
ccflags-y += -Iapps/bth_if/bt_if_client
endif
ifeq ($(APP_RPC_BTH_DSP_EN), 1)
ccflags-y += -Iapps/app_rpc
ccflags-y += -Iapps/app_rpc/rpc_bth_dsp
endif
ifeq ($(APP_RPC_MCU_SENSOR_EN),1)
ccflags-y += -Iapps/sensorhub
ccflags-y += -Iapps/app_rpc
ccflags-y += -DSENSOR_HUB
endif