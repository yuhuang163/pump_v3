
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(NOAPP),1)
CFLAGS_cp_subsys_msg_test.o += -DNOAPP
endif

ifeq ($(NO_CP_SUBSYS_IPC_WHEN_OPEN),1)
CFLAGS_cp_subsys.o += -DNO_CP_SUBSYS_IPC_WHEN_OPEN
endif

ifeq ($(CP_SUBSYS_TRC_TO_MCU),1)
CFLAGS_rx_cp_subsys_trc.o += -DCP_SUBSYS_TRC_TO_MCU -Iplatform/drivers/rmt_trace
CFLAGS_cp_subsys.o += -DCP_SUBSYS_TRC_TO_MCU
endif

ifeq ($(CP_SUBSYS_FLASH_BOOT),1)
CFLAGS_cp_subsys.o += -DCP_SUBSYS_FLASH_BOOT
INCLUDE_CP_SUBSYS_BIN := 0
ifneq ($(CP_SUBSYS_FLASH_BOOT_STRUCT_ADDR),)
CFLAGS_cp_subsys.o += -DCP_SUBSYS_FLASH_BOOT_STRUCT_ADDR=$(CP_SUBSYS_FLASH_BOOT_STRUCT_ADDR)
endif
else ifneq ($(CP_SUBSYS_LOAD_FLASH_ADDR),)
CFLAGS_cp_subsys.o += \
	-DCP_SUBSYS_LOAD_FLASH_ADDR=$(CP_SUBSYS_LOAD_FLASH_ADDR) \
	-DCP_SUBSYS_LOAD_FLASH_SIZE=$(CP_SUBSYS_LOAD_FLASH_SIZE)
INCLUDE_CP_SUBSYS_BIN := 0
endif
INCLUDE_CP_SUBSYS_BIN ?= 1

ifeq ($(INCLUDE_CP_SUBSYS_BIN),1)
CP_SUBSYS_TGT ?= cp_subsys
$(obj)/cp_subsys_bin.o : out/$(CP_SUBSYS_TGT)/$(CP_SUBSYS_TGT).bin
AFLAGS_cp_subsys_bin.o += -DCP_SUBSYS_BIN_NAME=$(CP_SUBSYS_TGT).bin -Wa,-Iout/$(CP_SUBSYS_TGT)
else
obj-y := $(filter-out cp_subsys_bin.o,$(obj-y))
endif

ifeq ($(APP_RPC_MCU_CP_EN),1)
subdir-ccflags-y += -Iapps/app_rpc/
endif