SDK_SRCTREE := $(CURDIR)/trunk/platform_chip/bes/sdk
PROJECT_ROOT := $(CURDIR)
O ?= $(PROJECT_ROOT)/project/build
BIN_DIR ?= $(PROJECT_ROOT)/project/bin/firmware
CHIP ?= best1306p
CONFIG_CROSS_COMPILE ?= arm-none-eabi-
CROSS_COMPILE ?= $(CONFIG_CROSS_COMPILE)

# -------------- 应用配置 --------------
# UART1 IOMUX 配置（传递给 SDK 的编译宏）
UART1_IOMUX_INDEX ?= 20
# UART3 IOMUX 配置（传递给 SDK 的编译宏）
UART3_IOMUX_INDEX ?= 24

# PWM IOMUX 配置（传递给 SDK 的编译宏）
PWM0_IOMUX_INDEX ?= 40
PWM1_IOMUX_INDEX ?= 41
PWM2_IOMUX_INDEX ?= 42
PWM3_IOMUX_INDEX ?= 43

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

# BLE 头文件子目录
BLE_INC_DIRS := $(shell find $(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/bthost -type d)\
# BLE 头文件
BLE_INC_FLAGS := $(patsubst %,-I%,$(BLE_INC_DIRS))

# BLE 自定义服务开关
CFG_CUSTOM_BLE_SERVICE ?= 1

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
	$(BLE_INC_FLAGS) \
	-I$(PROJECT_ROOT)/boards \
	-I$(PROJECT_ROOT)/drivers/audio \
	-I$(PROJECT_ROOT)/drivers/base \
	-I$(PROJECT_ROOT)/drivers/battery \
	-I$(PROJECT_ROOT)/drivers/ble \
	-I$(PROJECT_ROOT)/drivers/ble/services \
	-I$(PROJECT_ROOT)/drivers/ble/advs \
	-I$(PROJECT_ROOT)/drivers/ble/connect \
	-I$(PROJECT_ROOT)/drivers/common \
	-I$(PROJECT_ROOT)/drivers/database \
	-I$(PROJECT_ROOT)/drivers/fatfs \
	-I$(PROJECT_ROOT)/drivers/flash \
	-I$(PROJECT_ROOT)/drivers/force_sensor \
	-I$(PROJECT_ROOT)/drivers/i2c \
	-I$(PROJECT_ROOT)/drivers/imu \
	-I$(PROJECT_ROOT)/drivers/key \
	-I$(PROJECT_ROOT)/drivers/key/chip/sc09b \
	-I$(PROJECT_ROOT)/drivers/lcd \
	-I$(PROJECT_ROOT)/drivers/lcd/interface/lcd_uart \
	-I$(PROJECT_ROOT)/drivers/lcd/chip/uart_extension \
	-I$(PROJECT_ROOT)/drivers/led \
	-I$(PROJECT_ROOT)/drivers/led/chip/led_pwm \
	-I$(PROJECT_ROOT)/drivers/motor \
	-I$(PROJECT_ROOT)/drivers/motor/chip/motor_pwm \
	-I$(PROJECT_ROOT)/drivers/uart \
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
	-I$(PROJECT_ROOT)/trunk/external/scan_key \
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

# 添加项目根目录和 SDK 头文件路径
CPPFLAGS ?= -I$(PROJECT_ROOT) -I$(PROJECT_ROOT)/trunk $(SDK_INCLUDE_PATHS)

# 调试日志相关宏定义
ifeq ($(REL_TRACE_ENABLE),1)
CPPFLAGS += -DREL_TRACE_ENABLE
endif

# RTC 日历相关宏定义
ifeq ($(RTC_CALENDAR),1)
CPPFLAGS += -DRTC_CALENDAR
endif

# PWM IOMUX 相关宏定义
CPPFLAGS += -DPWM0_IOMUX_INDEX=$(PWM0_IOMUX_INDEX)
CPPFLAGS += -DPWM1_IOMUX_INDEX=$(PWM1_IOMUX_INDEX)
CPPFLAGS += -DPWM2_IOMUX_INDEX=$(PWM2_IOMUX_INDEX)
CPPFLAGS += -DPWM3_IOMUX_INDEX=$(PWM3_IOMUX_INDEX)

# LITTLEFS 相关宏定义
CPPFLAGS += -DLITTLEFS_SECTION_SIZE=$(LITTLEFS_SECTION_SIZE)

# 工厂数据相关宏定义
CPPFLAGS += -DFACTORY_DATA_SECTION_SIZE=$(FACTORY_DATA_SECTION_SIZE)

# BLE 相关宏定义
CPPFLAGS += -DBLE_HOST_SUPPORT
CPPFLAGS += -DNEW_NV_RECORD_ENABLED
CPPFLAGS += -I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/services/nv_section/userdata_section
CPPFLAGS += -I$(SDK_SRCTREE)/config/_default_cfg_src_

# SDK 目标映射：若传入的 T 在 SDK/config 下无效，则回退到 ble_only_lt
SDK_T ?= $(T)
ifeq ($(wildcard $(SDK_SRCTREE)/config/$(SDK_T)/target.mk),)
SDK_T := ble_only_lt
endif

CPPFLAGS += -I$(SDK_SRCTREE)/config/$(SDK_T)

# 当 T 含有斜杠时，取其 basename 用于产物文件名匹配
T_BASENAME := $(notdir $(T))

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
	$(wildcard $(PROJECT_ROOT)/drivers/ble/advs/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/ble/connect/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/ble/services/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/common/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/database/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/fatfs/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/flash/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/force_sensor/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/i2c/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/imu/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/key/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/key/chip/sc09b/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/lcd/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/lcd/interface/lcd_uart/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/lcd/chip/uart_extension/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/led/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/led/chip/led_pwm/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/motor/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/motor/chip/motor_pwm/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/uart/*.c) \
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
	$(wildcard $(PROJECT_ROOT)/trunk/external/scan_key/*.c) \
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

.PHONY: all extra sdk sdk_only clean allclean help print-vars firmware bootloader

all: extra sdk

sdk: $(EXTRA_LIB) | $(BIN_DIR)
	$(MAKE) -C $(SDK_SRCTREE) O=$(O) T=$(SDK_T) CONFIG_CROSS_COMPILE=$(CONFIG_CROSS_COMPILE) \
		C++_ONLY_FLAGS+=" -fno-threadsafe-statics " \
		LIB_LDFLAGS+=" $(EXTRA_LIB) -Wl,-defsym,__EXTRA_LIB_MTIME=$(shell stat -c %Y $(EXTRA_LIB)) " \
		REL_TRACE_ENABLE=$(REL_TRACE_ENABLE) \
		LITTLEFS_SECTION_SIZE=$(LITTLEFS_SECTION_SIZE) \
		FACTORY_DATA_SECTION_SIZE=$(FACTORY_DATA_SECTION_SIZE) \
		UART1_IOMUX_INDEX=$(UART1_IOMUX_INDEX) \
		UART3_IOMUX_INDEX=$(UART3_IOMUX_INDEX) \
		PWM0_IOMUX_INDEX=$(PWM0_IOMUX_INDEX) \
		PWM1_IOMUX_INDEX=$(PWM1_IOMUX_INDEX) \
		PWM2_IOMUX_INDEX=$(PWM2_IOMUX_INDEX) \
		PWM3_IOMUX_INDEX=$(PWM3_IOMUX_INDEX) \
		CFG_CUSTOM_BLE_SERVICE=$(CFG_CUSTOM_BLE_SERVICE)
	#   RTC_CALENDAR=$(RTC_CALENDAR)
	@set -e; \
		out_dir="$(O)/$(T)"; \
		for ext in elf bin map lst; do \
			copied=0; \
			for nm in "$(T)" "$(T_BASENAME)"; do \
				f="$$out_dir/$$nm.$$ext"; \
				if [ -f "$$f" ]; then \
					cp -f "$$f" "$(BIN_DIR)/"; \
					copied=1; \
					break; \
				fi; \
			done; \
			if [ "$$copied" -eq 0 ]; then \
				:; \
			fi; \
		done; \
		echo "[OK] Output copied to $(BIN_DIR)"

# 仅构建 SDK，不链接 EXTRA_LIB（供 bootloader 使用）
sdk_only: | $(BIN_DIR)
	$(MAKE) -C $(SDK_SRCTREE) O=$(O) T=$(SDK_T) CONFIG_CROSS_COMPILE=$(CONFIG_CROSS_COMPILE) \
		C++_ONLY_FLAGS+=" -fno-threadsafe-statics " 
	@set -e; \
		out_dir="$(O)/$(T)"; \
		for ext in elf bin map lst; do \
			copied=0; \
			for nm in "$(T)" "$(T_BASENAME)"; do \
				f="$$out_dir/$$nm.$$ext"; \
				if [ -f "$$f" ]; then \
					cp -f "$$f" "$(BIN_DIR)/"; \
					copied=1; \
					break; \
				fi; \
			done; \
			if [ "$$copied" -eq 0 ]; then \
				:; \
			fi; \
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
	@echo T_BASENAME=$(T_BASENAME)
	@echo CONFIG_CROSS_COMPILE=$(CONFIG_CROSS_COMPILE)
	@echo EXTRA_LIB=$(EXTRA_LIB)
	@echo EXTRA_OBJS=$(EXTRA_OBJS)
	@echo REL_TRACE_ENABLE=$(REL_TRACE_ENABLE)
	@echo RTC_CALENDAR=$(RTC_CALENDAR)

# -------------- 便捷目标：固件与 Bootloader 隔离构建 --------------

# 透传关键编译变量到子 make（仅导出，不设默认值，避免影响普通构建）
export NO_CP
export BES_OTA
export BLE
export OTA_ENABLE
export FLASH_SIZE
export FREEMAN_OTA_ENABLE
export OTA_BIN_COMPRESSED
export DEBUG
export FLASH_REMAP
export CHIP_HAS_CP
export CHIP

# 固件构建（默认参数）
firmware:
	@$(MAKE) \
		T=ble_only_lt \
		O=$(PROJECT_ROOT)/project/build/firmware \
		BIN_DIR=$(PROJECT_ROOT)/project/bin/firmware \
		NO_CP=1 \
		BES_OTA=1 \
		BLE=1 \
		OTA_ENABLE=1 \
		FLASH_SIZE=0x400000 \
		FREEMAN_OTA_ENABLE=1 \
		OTA_BIN_COMPRESSED=1 \
		all

# Bootloader 构建（默认参数）
bootloader:
	@$(MAKE) \
		T=prod_test/ota_copy \
		CHIP=best1306p \
		O=$(PROJECT_ROOT)/project/build/bootloader \
		BIN_DIR=$(PROJECT_ROOT)/project/bin/bootloader \
		DEBUG=1 \
		BES_OTA=1 \
		FLASH_REMAP=0 \
		FLASH_SIZE=0x400000 \
		CHIP_HAS_CP=0 \
		FREEMAN_OTA_ENABLE=1 \
		OTA_BIN_COMPRESSED=1 \
		sdk_only


