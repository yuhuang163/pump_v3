SDK_SRCTREE := $(CURDIR)/trunk/platform_chip/bes/sdk
PROJECT_ROOT := $(CURDIR)
O ?= $(PROJECT_ROOT)/project/build
BIN_DIR ?= $(PROJECT_ROOT)/project/bin
T ?= best1306p
CONFIG_CROSS_COMPILE ?= arm-none-eabi-
CROSS_COMPILE ?= $(CONFIG_CROSS_COMPILE)

# -------------- 应用源码与编译参数 --------------
CSTD ?= gnu11
CFLAGS ?= -O2 -g -std=$(CSTD) -fvisibility=default
EXTRA_SDK_ARCH_FLAGS := -mthumb -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard
EXTRA_CFLAGS ?= -std=gnu99 $(EXTRA_SDK_ARCH_FLAGS)

# Release trace switch for both EXTRA_LIB and SDK build
REL_TRACE_ENABLE ?= 1

# SDK 头文件（最小必需集）
SDK_INCLUDE_PATHS := \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/include \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/os_base/include \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/include/rtos/rtx5 \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/include/rtos/rtx \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/rtos/rtx5 \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/rtos/rtx5/rtx_config \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/hal \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/drivers/ana \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/cmsis/inc \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/services/nv_section/include \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/utils/heap \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/utils/libc/inc \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/cmsis/inc \
	-I$(PROJECT_ROOT)/boards \
	-I$(PROJECT_ROOT)/trunk/include

CPPFLAGS ?= -I$(PROJECT_ROOT) -I$(PROJECT_ROOT)/trunk $(SDK_INCLUDE_PATHS)

ifeq ($(REL_TRACE_ENABLE),1)
CPPFLAGS += -DREL_TRACE_ENABLE
endif

# 根据 T 映射芯片宏，确保与 SDK 头文件匹配
ifeq ($(T),best1306p)
CHIP_DEFINES := -DCHIP_BEST1306P
else ifeq ($(T),best1306)
CHIP_DEFINES := -DCHIP_BEST1306
else ifeq ($(T),best1305)
CHIP_DEFINES := -DCHIP_BEST1305
else ifeq ($(T),best1000)
CHIP_DEFINES := -DCHIP_BEST1000
else ifeq ($(T),best1400)
CHIP_DEFINES := -DCHIP_BEST1400
else ifeq ($(T),best1501)
CHIP_DEFINES := -DCHIP_BEST1501
else ifeq ($(T),best1502p)
CHIP_DEFINES := -DCHIP_BEST1502P
else ifeq ($(T),best1502x)
CHIP_DEFINES := -DCHIP_BEST1502X
else ifeq ($(T),best1503)
CHIP_DEFINES := -DCHIP_BEST1503
else ifeq ($(T),best1600)
CHIP_DEFINES := -DCHIP_BEST1600
else ifeq ($(T),best2000)
CHIP_DEFINES := -DCHIP_BEST2000
else ifeq ($(T),best2001)
CHIP_DEFINES := -DCHIP_BEST2001
else ifeq ($(T),best2003)
CHIP_DEFINES := -DCHIP_BEST2003
else ifeq ($(T),best2300)
CHIP_DEFINES := -DCHIP_BEST2300
else ifeq ($(T),best2300a)
CHIP_DEFINES := -DCHIP_BEST2300A
else ifeq ($(T),best2300p)
CHIP_DEFINES := -DCHIP_BEST2300P
else ifeq ($(T),best3001)
CHIP_DEFINES := -DCHIP_BEST3001
else ifeq ($(T),best3003)
CHIP_DEFINES := -DCHIP_BEST3003
else ifeq ($(T),best3005)
CHIP_DEFINES := -DCHIP_BEST3005
else ifeq ($(T),fpga1000)
CHIP_DEFINES := -DCHIP_FPGA1000
else
CHIP_DEFINES := -DCHIP_$(shell echo $(T) | tr '[:lower:]' '[:upper:]')
endif
CPPFLAGS += $(CHIP_DEFINES)

# 应用源码（满足链接 root_main 与 platform_prepare_init 等）
EXTRA_SRCS := \
	$(wildcard $(PROJECT_ROOT)/trunk/platform_chip/bes/os_base/*.c) \
	$(PROJECT_ROOT)/trunk/root_source/base/root_list.c \
	$(PROJECT_ROOT)/trunk/root_source/base/root_main.c \
	$(PROJECT_ROOT)/trunk/root_source/utils/log_utils.c \
	$(PROJECT_ROOT)/trunk/root_source/algorithm/algorithm_manager.c 

CC := $(if $(strip $(CROSS_COMPILE)),$(CROSS_COMPILE)gcc,gcc)
AR := $(if $(strip $(CROSS_COMPILE)),$(CROSS_COMPILE)ar,ar)

EXTRA_LIB := $(O)/lib/libextra.a
EXTRA_OBJS := $(patsubst $(PROJECT_ROOT)/%.c,$(O)/extra/%.o,$(filter %.c,$(EXTRA_SRCS)))

.PHONY: all extra sdk clean allclean help print-vars

all: extra sdk

sdk: $(EXTRA_LIB) | $(BIN_DIR)
	$(MAKE) -C $(SDK_SRCTREE) O=$(O) T=$(T) CONFIG_CROSS_COMPILE=$(CONFIG_CROSS_COMPILE) \
	  C++_ONLY_FLAGS+=" -fno-threadsafe-statics " \
	  LIB_LDFLAGS+=" $(EXTRA_LIB) -Wl,-defsym,__EXTRA_LIB_MTIME=$(shell stat -c %Y $(EXTRA_LIB)) " REL_TRACE_ENABLE=$(REL_TRACE_ENABLE)
	@set -e; \
	  out_dir="$(O)/$(T)"; \
	  for ext in elf bin map lst; do \
	    f="$$out_dir/$(T).$$ext"; \
	    if [ -f "$$f" ]; then cp -f "$$f" "$(BIN_DIR)/"; fi; \
	  done; \
	  echo "[OK] Output copied to $(BIN_DIR)"

$(BIN_DIR):
	@mkdir -p $@

extra: $(EXTRA_LIB)

$(EXTRA_LIB): $(EXTRA_OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $(EXTRA_OBJS)

$(O)/extra/%.o: $(PROJECT_ROOT)/%.c
	@mkdir -p $(dir $@)
	@echo "[CC] $< -> $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

-include $(EXTRA_OBJS:.o=.d)

clean:
	$(MAKE) -C $(SDK_SRCTREE) O=$(O) T=$(T) CONFIG_CROSS_COMPILE=$(CONFIG_CROSS_COMPILE) clean
	@rm -rf $(O)/extra $(EXTRA_LIB) $(BIN_DIR)

allclean:
	$(MAKE) -C $(SDK_SRCTREE) O=$(O) T=$(T) CONFIG_CROSS_COMPILE=$(CONFIG_CROSS_COMPILE) allclean
	@rm -rf $(O) $(BIN_DIR)

help:
	$(MAKE) -C $(SDK_SRCTREE) O=$(O) T=$(T) CONFIG_CROSS_COMPILE=$(CONFIG_CROSS_COMPILE) help

print-vars:
	@echo PROJECT_ROOT=$(PROJECT_ROOT)
	@echo SDK_SRCTREE=$(SDK_SRCTREE)
	@echo O=$(O)
	@echo BIN_DIR=$(BIN_DIR)
	@echo T=$(T)
	@echo CONFIG_CROSS_COMPILE=$(CONFIG_CROSS_COMPILE)
	@echo EXTRA_LIB=$(EXTRA_LIB)
	@echo EXTRA_OBJS=$(EXTRA_OBJS)
	@echo REL_TRACE_ENABLE=$(REL_TRACE_ENABLE)


