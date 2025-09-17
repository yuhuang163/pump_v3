cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y :=
#obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
#obj-y := $(obj-y:.c=.o)
#obj-y := $(obj-y:.cpp=.o)
#obj-y := $(obj-y:.S=.o)

obj-y += vowlan_main.o
obj-y += wui/

ifeq ($(VOWLAN_MASTER),1)
ifeq ($(VOWLAN_SUPPORT_INPUT_BY_USB),1)
obj-y += wusb/
endif
ifeq ($(VOWLAN_SUPPORT_INPUT_BY_I2S),1)
obj-y += wi2s/
endif
endif

ifeq ($(VOWLAN_SLAVE),1)
ifeq ($(VOWLAN_SUPPORT_INPUT_BY_RF_SINK),1)
obj-y += wsink/wsink.o
endif
endif

subdir-ccflags-y += \
	-Iapps/vowlan/ \
	-Iapps/vowlan/wui/ \
	-Iapps/vowlan/wusb/ \
	-Iapps/vowlan/wi2s/ \
	-Iapps/vowlan/wsink/ \
	-Inet/utils/ \
	-Inet/vowlan/ \
	-Iutils/ \
	-Inet/lwip_2.1.2/src/include/ \
	-Iutils/hwtimer_list/ \
	-Inet/wifi/net_al/ \
	-Iapps/usbaudio/


ifeq ($(BLE_WIFI_DUAL_CHIP_MODE),1)
subdir-ccflags-y += -Inet/vowlan/vow_i2s_dev/
endif

ifeq ($(APP_UART_MODULE),1)
ifneq ($(BT_NATIVE_SUPPORT),1)
obj-y += ../common/uart_dma/
endif
subdir-ccflags-y += -Iapps/common/uart_dma/
endif

ifeq ($(APP_UART_BRIDGE_MODULE),1)
ifneq ($(BT_NATIVE_SUPPORT),1)
obj-y += ../common/uart_bridge/
endif
subdir-ccflags-y += -Iapps/common/uart_bridge/
endif