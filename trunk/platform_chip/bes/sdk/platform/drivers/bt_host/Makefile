
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(NO_BTH_IPC_WHEN_OPEN),1)
CFLAGS_bt_host.o += -DNO_BTH_IPC_WHEN_OPEN
endif

ifeq ($(BTH_TRC_TO_SYS),1)
TRC_FLAGS := -DBTH_TRC_TO_SYS
ifeq ($(RMT_TRC_IN_MSG_CHAN),1)
TRC_FLAGS += -DRMT_TRC_IN_MSG_CHAN
endif
CFLAGS_rx_bth_trc.o += $(TRC_FLAGS) -Iplatform/drivers/rmt_trace
CFLAGS_bt_host.o += $(TRC_FLAGS)
endif

ifeq ($(BTH_FLASH_BOOT),1)
CFLAGS_bt_host.o += -DBTH_FLASH_BOOT
INCLUDE_BTH_BIN := 0
ifneq ($(BTH_FLASH_BOOT_STRUCT_ADDR),)
CFLAGS_bt_host.o += -DBTH_FLASH_BOOT_STRUCT_ADDR=$(BTH_FLASH_BOOT_STRUCT_ADDR)
endif
else ifneq ($(BTH_LOAD_FLASH_ADDR),)
CFLAGS_bt_host.o += \
	-DBTH_LOAD_FLASH_ADDR=$(BTH_LOAD_FLASH_ADDR) \
	-DBTH_LOAD_FLASH_SIZE=$(BTH_LOAD_FLASH_SIZE)
INCLUDE_BTH_BIN := 0
endif
INCLUDE_BTH_BIN ?= 1

ifeq ($(INCLUDE_BTH_BIN),1)
BT_HOST_TGT ?= bth_core
$(obj)/bt_host_bin.o : out/$(BT_HOST_TGT)/$(BT_HOST_TGT).bin
AFLAGS_bt_host_bin.o += -DBT_HOST_BIN_NAME=$(BT_HOST_TGT).bin -Wa,-Iout/$(BT_HOST_TGT)
else
obj-y := $(filter-out bt_host_bin.o,$(obj-y))
endif
