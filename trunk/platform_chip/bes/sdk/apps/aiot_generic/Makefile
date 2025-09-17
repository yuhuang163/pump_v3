cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))


obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ifeq ($(MODULE_KERNEL_STUB),1)
ccflags-y += -DMODULE_KERNEL_STUB
endif
ccflags-y += -DDEBUG_PORT=$(DEBUG_PORT)
ifeq ($(BT_RPC_EN),1)
ccflags-y += -DBT_RPC_EN
endif

ifeq ($(BT_APP),1)
CFLAGS_aiot_generic.o += -D__BT_APP__
endif

ccflags-y +=  \
    -Iservices/norflash_api \
    -Iservices/aiot/bt_rpc \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/sensor_hub \
    -Iplatform/drivers/bt_host \
    -Iapps/common \
    -Iapps/main \
    -Iservices/nv_section/factory_section \
    -Iutils/hwtimer_list \
    -Iutils/heap/ \
    -Iutils/hwtimer_list

ifeq ($(DSP_ENABLE), 1)
ccflags-y += \
    -Iplatform/drivers/hifi4 \
    -Iapps/dsp_hifi4
endif

ifeq ($(UTILS_ESHELL_EN),1)
ccflags-y += \
    -Iapps/app_eshell
endif

ifeq ($(ALSA_EN), 1)
ccflags-y += -Iservices/aiot/alsa_adapter
endif
ifeq ($(BT_HOST),1)
CFLAGS_aiot_generic.o += -DBT_HOST
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
ifeq ($(CP_SUBSYS_TEST),1)
ccflags-y += -Iplatform/drivers/cp_subsys
ccflags-y += -DCP_SUBSYS_TEST
endif
ifeq ($(WIFI_TEST),1)
ccflags-y += -Itests/wifi
ccflags-y += -DWIFI_TEST
endif

ifeq ($(NET_MBEDTLS_SUPPORT),1)
ccflags-y += -Inet/mbedtls/include
endif

ifeq ($(SHANHAI_MBEDTLS_SUPPORT),1)
ccflags-y += -Ithirdparty/shanhai/tee/spe/mbedtls/include
ccflags-y += -Ithirdparty/shanhai/tee/spe/mbedtls/configs
CFLAGS_aiot_generic.o += -DMBEDTLS_INIT_SUPPORT
endif

ifeq ($(MBEDTLS_SUPPORT),1)
ifeq ($(MBEDTLS_PORTS),1)
CFLAGS_aiot_generic.o += -DMBEDTLS_INIT_SUPPORT
ccflags-y += -Iutils/mbedtls/ports/include
ccflags-y += -Iutils/mbedtls/include
endif
endif

ifeq ($(NET),1)
ccflags-y += -Inet
endif

ifeq ($(VOWLAN_SUPPORT),1)
ccflags-y += -Iapps/vowlan
endif

ifeq ($(PSRAM_INIT_CHECK),1)
ccflags-y += -DPSRAM_INIT_CHECK
endif

ifeq ($(USB_DISK_TEST),1)
ccflags-y += -DUSB_DISK_TEST
ccflags-y +=  -Iservices/fs/mbed/fat/ \
              -Iservices/fs/mbed/fat/ChaN/ \
              -Iapps/usbhost/
endif
