cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

CFLAGS_bt_drv_uart_bridge_intsys.o += $(BT_DRV_UART_BRIDGE_FLAGS)

CFLAGS_bt_drv.o += $(BT_DRV_FLAGS)

CFLAGS_bt_drv_patch.o += $(BT_DRV_PATCH_FLAGS)

CFLAGS_bt_drv_config.o += $(BT_DRV_CONFIG_FLAGS)

subdir-ccflags-y += -Iservices/nv_section/userdata_section
ccflags-y += $(BLUETOOTH_ADAPTER_INCLUDES)

ifeq ($(TX_IQ_CAL),1)
subdir-ccflags-y += -DTX_IQ_CAL
endif

ifeq ($(RX_DC_CAL),1)
subdir-ccflags-y += -DRX_DC_CAL
endif

ifeq ($(DPD_ONCHIP_CAL),1)
subdir-ccflags-y += -DDPD_ONCHIP_CAL
endif
ifeq ($(TX_PULLING_CAL),1)
subdir-ccflags-y += -DTX_PULLING_CAL
endif

ifeq ($(BLE_EXT_ADV_TX_PWR_INDEPEND),1)
ccflags-y += -DBLE_EXT_ADV_TX_PWR_INDEPEND
endif

ifeq ($(BT_WATCH_PATCH),1)
ccflags-y += -DBT_WATCH_PATCH
endif

ifeq ($(BT_ACTIVE_OUTPUT),1)
ccflags-y += -DBT_ACTIVE_OUTPUT
endif

ifeq ($(BESTRX_SUPPORT),1)
ccflags-y += -D__BESTRX_SUPPORT__
endif

ifeq ($(BESTRX_RELAY),1)
ccflags-y += -D__BESRELAY_SUPPORT__
endif

ifeq ($(ANT_SUPPORT),1)
ccflags-y += -D__ANT_DRIVER_SUPPORT__
endif

ifeq ($(RF_APB_RESTORE),1)
ccflags-y += -D__RF_APB_RESTORE_SUPPORT__
endif

REDUCE_EDGE_CHL_TXPWR ?= 0
ifeq ($(REDUCE_EDGE_CHL_TXPWR),1)
ccflags-y += -DREDUCE_EDGE_CHL_TXPWR
endif

ifeq ($(BT_RF_I2V_BYPASS),1)
ccflags-y += -DBT_RF_I2V_BYPASS
endif

ifeq ($(PMU_NTC_MONITOR),1)
ccflags-y += -DPMU_NTC_MONITOR
endif