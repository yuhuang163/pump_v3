
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)patch/*.c))

ifeq ($(BLE_ONLY_ENABLED),1)
ifeq ($(BLE_STACK_NEW_DESIGN),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)l2cap/*.c))
endif
else
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)l2cap/*.c))
endif

ifeq ($(IBRT), 1)
BTCOMMON_LIB_NAME := ibrt_libbt_common
else
BTCOMMON_LIB_NAME := $(CHIP)_libbt_common
endif

ifeq ($(BTH_IN_ROM),1)
BTCOMMON_LIB_NAME := $(BTCOMMON_LIB_NAME)_romaac
endif

ifeq ($(BLE),1)
BTCOMMON_LIB_NAME := $(BTCOMMON_LIB_NAME)_ble
endif

ifeq ($(USE_TRACE_ID), 1)
BTCOMMON_LIB_NAME := $(BTCOMMON_LIB_NAME)_log_compressed
endif

ifeq ($(BLE_AUDIO_ENABLED), 1)
BTCOMMON_LIB_NAME := $(BTCOMMON_LIB_NAME)_aob
endif

$(BTCOMMON_LIB_NAME)-y := $(obj_c:.c=.o)

obj-y := $(BTCOMMON_LIB_NAME).a

ccflags-y += -DBLUETOOTH_BT_IMPL

ccflags-y += \
	-Iutils/intersyshci/ \
	-Iutils/cqueue/ \
	-Iplatform/drivers/bt/ \
	-Iservices/osif/ \
	-Iapps/common/ \
	-Iutils/heap/ \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BES_BT_IMPL_INCLUDES) \
	$(BLE_STACK_INCLUDES) \
    -I$(BT_ROM_PATCH_PATH)/ \
	$(BLE_STACK_INC_INTERNAL) \
    -Ibthost/porting
