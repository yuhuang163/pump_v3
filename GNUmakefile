SDK_SRCTREE := $(CURDIR)/trunk/platform_chip/bes/sdk
PROJECT_ROOT := $(CURDIR)
O ?= $(PROJECT_ROOT)/project/build
BIN_DIR ?= $(PROJECT_ROOT)/project/bin
CHIP ?= best1306p
CONFIG_CROSS_COMPILE ?= arm-none-eabi-
CROSS_COMPILE ?= $(CONFIG_CROSS_COMPILE)

# -------------- 应用源码与编译参数 --------------
CSTD ?= gnu11
CFLAGS ?= -O2 -g -std=$(CSTD) -fvisibility=default
EXTRA_SDK_ARCH_FLAGS := -mthumb -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard
EXTRA_CFLAGS ?= -std=gnu99 $(EXTRA_SDK_ARCH_FLAGS)

# Release trace switch for both EXTRA_LIB and SDK build
REL_TRACE_ENABLE ?= 1

# RTC calendar feature switch
RTC_CALENDAR ?= 1

# littlefs section size
LITTLEFS_SECTION_SIZE ?= 0x4000

# factory data section size
FACTORY_DATA_SECTION_SIZE ?= 0x2000

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
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/services/nv_section/log_section \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/services/norflash_api \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/services/nv_section/factory_section \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/utils/heap \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/utils/libc/inc \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/utils/hwtimer_list \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/cmsis/inc \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/apps/main \
	-I$(PROJECT_ROOT)/boards \
	-I$(PROJECT_ROOT)/drivers/audio \
	-I$(PROJECT_ROOT)/drivers/base \
	-I$(PROJECT_ROOT)/drivers/battery \
	-I$(PROJECT_ROOT)/drivers/ble \
	-I$(PROJECT_ROOT)/drivers/common \
	-I$(PROJECT_ROOT)/drivers/database \
	-I$(PROJECT_ROOT)/drivers/fatfs \
	-I$(PROJECT_ROOT)/drivers/flash \
	-I$(PROJECT_ROOT)/drivers/force_sensor \
	-I$(PROJECT_ROOT)/drivers/imu \
	-I$(PROJECT_ROOT)/drivers/key \
	-I$(PROJECT_ROOT)/drivers/lcd \
	-I$(PROJECT_ROOT)/drivers/led \
	-I$(PROJECT_ROOT)/drivers/motor \
	-I$(PROJECT_ROOT)/drivers/wifi \
	-I$(PROJECT_ROOT)/overlay/framework_hooks \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/algo \
	-I$(PROJECT_ROOT)/overlay/windows/battery \
	-I$(PROJECT_ROOT)/overlay/windows/breath_pump \
	-I$(PROJECT_ROOT)/overlay/windows/charge \
	-I$(PROJECT_ROOT)/overlay/windows/dummy \
	-I$(PROJECT_ROOT)/overlay/windows/home \
	-I$(PROJECT_ROOT)/overlay/windows/logo \
	-I$(PROJECT_ROOT)/overlay/windows/ota \
	-I$(PROJECT_ROOT)/overlay/windows/pop_windows \
	-I$(PROJECT_ROOT)/overlay/windows/setting \
	-I$(PROJECT_ROOT)/overlay/windows \
	-I$(PROJECT_ROOT)/overlay/sensor/imu \
	-I$(PROJECT_ROOT)/overlay/sensor/force_sensor \
	-I$(PROJECT_ROOT)/overlay/sensor/air_pressure \
	-I$(PROJECT_ROOT)/overlay/storage/factory \
	-I$(PROJECT_ROOT)/overlay/storage/breath_pump \
	-I$(PROJECT_ROOT)/overlay/storage/resource \
	-I$(PROJECT_ROOT)/overlay/storage\
	-I$(PROJECT_ROOT)/trunk/include \
	-I$(PROJECT_ROOT)/trunk/extern \
	-I$(PROJECT_ROOT)/trunk/external/littlefs \
	-I$(PROJECT_ROOT)/trunk/external/littlefs/fds \
	-I$(PROJECT_ROOT)/trunk/root_source/airplane \
	-I$(PROJECT_ROOT)/trunk/root_source/algorithm \
	-I$(PROJECT_ROOT)/trunk/root_source/audio \
	-I$(PROJECT_ROOT)/trunk/root_source/base \
	-I$(PROJECT_ROOT)/trunk/root_source/ble \
	-I$(PROJECT_ROOT)/trunk/root_source/breath_pump \
	-I$(PROJECT_ROOT)/trunk/root_source/hal \
	-I$(PROJECT_ROOT)/trunk/root_source/key_input \
	-I$(PROJECT_ROOT)/trunk/root_source/misc \
	-I$(PROJECT_ROOT)/trunk/root_source/motor \
	-I$(PROJECT_ROOT)/trunk/root_source/network \
	-I$(PROJECT_ROOT)/trunk/root_source/ota \
	-I$(PROJECT_ROOT)/trunk/root_source/pm \
	-I$(PROJECT_ROOT)/trunk/root_source/protocol \
	-I$(PROJECT_ROOT)/trunk/root_source/resource \
	-I$(PROJECT_ROOT)/trunk/root_source/sensor \
	-I$(PROJECT_ROOT)/trunk/root_source/storage \
	-I$(PROJECT_ROOT)/trunk/root_source/utils \
	-I$(PROJECT_ROOT)/trunk/root_source/windows

CPPFLAGS ?= -I$(PROJECT_ROOT) -I$(PROJECT_ROOT)/trunk $(SDK_INCLUDE_PATHS)

ifeq ($(REL_TRACE_ENABLE),1)
CPPFLAGS += -DREL_TRACE_ENABLE
endif

ifeq ($(RTC_CALENDAR),1)
CPPFLAGS += -DRTC_CALENDAR
endif

CPPFLAGS += -DLITTLEFS_SECTION_SIZE=$(LITTLEFS_SECTION_SIZE)
CPPFLAGS += -DFACTORY_DATA_SECTION_SIZE=$(FACTORY_DATA_SECTION_SIZE)

# 根据 CHIP 映射芯片宏，确保与 SDK 头文件匹配（避免将 T=ble_only_lt 误当作芯片）
ifeq ($(CHIP),best1306p)
CHIP_DEFINES := -DCHIP_BEST1306P
else ifeq ($(CHIP),best1306)
CHIP_DEFINES := -DCHIP_BEST1306
else ifeq ($(CHIP),best1305)
CHIP_DEFINES := -DCHIP_BEST1305
else ifeq ($(CHIP),best1000)
CHIP_DEFINES := -DCHIP_BEST1000
else ifeq ($(CHIP),best1400)
CHIP_DEFINES := -DCHIP_BEST1400
else ifeq ($(CHIP),best1501)
CHIP_DEFINES := -DCHIP_BEST1501
else ifeq ($(CHIP),best1502p)
CHIP_DEFINES := -DCHIP_BEST1502P
else ifeq ($(CHIP),best1502x)
CHIP_DEFINES := -DCHIP_BEST1502X
else ifeq ($(CHIP),best1503)
CHIP_DEFINES := -DCHIP_BEST1503
else ifeq ($(CHIP),best1600)
CHIP_DEFINES := -DCHIP_BEST1600
else ifeq ($(CHIP),best2000)
CHIP_DEFINES := -DCHIP_BEST2000
else ifeq ($(CHIP),best2001)
CHIP_DEFINES := -DCHIP_BEST2001
else ifeq ($(CHIP),best2003)
CHIP_DEFINES := -DCHIP_BEST2003
else ifeq ($(CHIP),best2300)
CHIP_DEFINES := -DCHIP_BEST2300
else ifeq ($(CHIP),best2300a)
CHIP_DEFINES := -DCHIP_BEST2300A
else ifeq ($(CHIP),best2300p)
CHIP_DEFINES := -DCHIP_BEST2300P
else ifeq ($(CHIP),best3001)
CHIP_DEFINES := -DCHIP_BEST3001
else ifeq ($(CHIP),best3003)
CHIP_DEFINES := -DCHIP_BEST3003
else ifeq ($(CHIP),best3005)
CHIP_DEFINES := -DCHIP_BEST3005
else ifeq ($(CHIP),fpga1000)
CHIP_DEFINES := -DCHIP_FPGA1000
else
CHIP_DEFINES := -DCHIP_$(shell echo $(CHIP) | tr '[:lower:]' '[:upper:]')
endif
CPPFLAGS += $(CHIP_DEFINES)

# 应用源码（满足链接 root_main 与 platform_prepare_init 等）
EXTRA_SRCS := \
	$(wildcard $(PROJECT_ROOT)/trunk/platform_chip/bes/os_base/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/utils/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/base/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/audio/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/base/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/battery/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/battery/charge/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/battery/adc/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/ble/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/common/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/database/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/fatfs/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/flash/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/force_sensor/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/i2c/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/imu/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/key/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/lcd/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/led/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/motor/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/wifi/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/algo/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/sensor/imu/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/sensor/force_sensor/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/sensor/air_pressure/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/storage/factory/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/storage/breath_pump/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/storage/resource/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/storage/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/windows/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/windows/battery/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/windows/breath_pump/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/windows/charge/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/windows/dummy/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/windows/home/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/windows/logo/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/windows/ota/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/windows/pop_windows/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/windows/setting/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/external/littlefs/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/external/littlefs/fds/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/airplane/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/algorithm/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/audio/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/ble/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/breath_pump/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/key_input/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/misc/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/motor/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/network/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/ota/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/pm/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/protocol/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/resource/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/sensor/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/storage/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/windows/*.c) 

CC := $(if $(strip $(CROSS_COMPILE)),$(CROSS_COMPILE)gcc,gcc)
AR := $(if $(strip $(CROSS_COMPILE)),$(CROSS_COMPILE)ar,ar)

EXTRA_LIB := $(O)/lib/libextra.a
EXTRA_OBJS := $(patsubst $(PROJECT_ROOT)/%.c,$(O)/extra/%.o,$(filter %.c,$(EXTRA_SRCS)))

.PHONY: all extra sdk clean allclean help print-vars

all: extra sdk

sdk: $(EXTRA_LIB) | $(BIN_DIR)
	$(MAKE) -C $(SDK_SRCTREE) O=$(O) T=$(T) CONFIG_CROSS_COMPILE=$(CONFIG_CROSS_COMPILE) \
	  C++_ONLY_FLAGS+=" -fno-threadsafe-statics " \
	  LIB_LDFLAGS+=" $(EXTRA_LIB) -Wl,-defsym,__EXTRA_LIB_MTIME=$(shell stat -c %Y $(EXTRA_LIB)) " \
	  REL_TRACE_ENABLE=$(REL_TRACE_ENABLE) \
	  LITTLEFS_SECTION_SIZE=$(LITTLEFS_SECTION_SIZE) \
	  FACTORY_DATA_SECTION_SIZE=$(FACTORY_DATA_SECTION_SIZE)
	#   RTC_CALENDAR=$(RTC_CALENDAR)
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
	@echo RTC_CALENDAR=$(RTC_CALENDAR)


