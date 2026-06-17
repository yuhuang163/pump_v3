# V3_2710IMC_GNUmakefile

SDK_SRCTREE := $(CURDIR)/trunk/platform_chip/bes/sdk
PROJECT_ROOT := $(CURDIR)
O ?= $(PROJECT_ROOT)/project/build
BIN_DIR ?= $(PROJECT_ROOT)/project/bin/firmware
CHIP ?= best1306p
CONFIG_CROSS_COMPILE ?= arm-none-eabi-
CROSS_COMPILE ?= $(CONFIG_CROSS_COMPILE)

# -------------- 应用配置 --------------
# PWM IOMUX 配置（传递给 SDK 的编译宏）
PWM2_IOMUX_INDEX ?= 42
PWM3_IOMUX_INDEX ?= 43
PWM4_IOMUX_INDEX ?= 44
PWM5_IOMUX_INDEX ?= 45

# LCD 引脚配置（可通过命令行覆盖）
DISPLAY_RST_PIN ?= HAL_GPIO_PIN_P4_1
DISPLAY_TE_PIN ?= HAL_GPIO_PIN_P4_0

# -------------- 工具宏定义 --------------
# 递归查找指定目录下的所有 .c 文件
# 用法: $(call rwildcard,目录路径)
rwildcard = $(shell find $(1) -name '*.c')

# -------------- 应用源码与编译参数 --------------
CSTD ?= gnu11
CFLAGS ?= -O2 -g -std=$(CSTD) -fvisibility=default
EXTRA_SDK_ARCH_FLAGS := -mthumb -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard
EXTRA_CFLAGS ?= -std=gnu99 $(EXTRA_SDK_ARCH_FLAGS)

# -------------- 单个告警关闭（Clang/GCC）--------------
# 关闭 pointer-to-int-cast: -Wno-pointer-to-int-cast
# 关闭 int-conversion:     -Wno-int-conversion
WARN_SUPPRESS_FLAGS += -Wno-pointer-to-int-cast -Wno-int-conversion
CFLAGS += $(WARN_SUPPRESS_FLAGS)
EXTRA_CFLAGS += $(WARN_SUPPRESS_FLAGS)

# Add -Werror if WERROR_ENABLE is set to 1
ifeq ($(WERROR_ENABLE),1)
CFLAGS += -Werror
EXTRA_CFLAGS += -Werror
endif

# crash reboot switch
CRASH_REBOOT ?= 1

# watchdog switch
WATCHER_DOG ?= 1

# warnings as errors switch (0=disabled, 1=enabled)
WERROR_ENABLE ?= 1

# custom app switch
CFG_CUSTOM_APP ?= 1

# Release trace switch for both EXTRA_LIB and SDK build
REL_TRACE_ENABLE ?= 1

# RTC calendar feature switch
RTC_CALENDAR ?= 1

# PMU clock use external crystal switch
# 影响tick精度，未贴外部晶振时，需要设置为0
PMU_CLK_USE_EXT_CRYSTAL ?= 0

# USB voltage switch
VUSB_ON ?= 1

# PSRAM enable switch
PSRAM_ENABLE ?= 1

# 硬件版本
V3_2710IMP_HARDWARE_V1 ?= 0
V3_2710IMC_HARDWARE_V1 ?= 1
V3_2710IMC_HARDWARE_V2 ?= 2
V3_2710IMC_HARDWARE_V3 ?= 3 #DV1
V3_2710IMC_HARDWARE_V4 ?= 4 #DV4
V3_HARDWARE_VERSION ?= $(V3_2710IMC_HARDWARE_V4)

ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
# littlefs section size 256k
LITTLEFS_SECTION_SIZE ?= 0x40000

# factory data section size
FACTORY_DATA_SECTION_SIZE ?= 0x2000

# lute picture resources section size
LUTE_PICTURE_RESOURCES_SECTION_SIZE ?= 0x00000

# lute reserved section size
LUTE_RESERVED_SECTION_SIZE ?= 0x0000	
else
# littlefs section size
LITTLEFS_SECTION_SIZE ?= 0x4000

# factory data section size
FACTORY_DATA_SECTION_SIZE ?= 0x2000

# lute picture resources section size
LUTE_PICTURE_RESOURCES_SECTION_SIZE ?= 0x200000

# lute reserved section size
LUTE_RESERVED_SECTION_SIZE ?= 0x2000	
endif
# BLE 自定义服务开关
CFG_CUSTOM_BLE_SERVICE ?= 1

# BLE 发射功率: -1=SDK默认分级, 0-12=自定义统一功率(dBm)
CFG_CUSTOM_BLE_RF_PWR ?= 0

# OTA 事件回调开关
CFG_OTA_EVENT_CALLBACK ?= 1

# LVGL开关
CFG_LVGL_ENABLE := 1

# LVGL snapshot开关
CFG_LVGL_SNAPSHOT_ENABLE := 1

# 外部flash开关
CFG_EXTERNAL_FLASH_ENABLE := 1

# BT 校准开关
CFG_BT_CALIB ?= 1

# 产品型号
ROOT_PRODUCT_V3_ID ?= 0x08
ROOT_PRODUCT_V3PRO_ID ?= 0x09
ROOT_PRODUCT_ID ?= $(ROOT_PRODUCT_V3_ID)

# 产品UI版本
ROOT_UI_V2 ?= 0x02
ROOT_UI_V3 ?= 0x03
ROOT_UI_V4 ?= 0x04
ROOT_UI_V5 ?= 0x05
ROOT_UI_V6 ?= 0x06

ROOT_UI_VERSION ?= $(ROOT_UI_V5)

# V3 UI版本
V3_UI_V2 ?= 0x02
V3_UI_V3 ?= 0x03
V3_UI_V4 ?= 0x04
V3_UI_V5 ?= 0x05
V3_UI_V6 ?= 0x06

# V3PRO UI版本
V3PRO_UI_V2 ?= 0x12
V3PRO_UI_V3 ?= 0x13
V3PRO_UI_V4 ?= 0x14
V3PRO_UI_V5 ?= 0x15
V3PRO_UI_V6 ?= 0x16

# 产品UX版本
V3_UX_V1 ?= 0x01
V3_UX_V2 ?= 0x02
V3_UX_V3 ?= 0x03
V3_UX_V4 ?= 0x04

# 根据产品型号和UI版本确定V3/V3PRO UI版本
ifeq ($(ROOT_PRODUCT_ID), $(ROOT_PRODUCT_V3PRO_ID))
ifeq ($(ROOT_UI_VERSION), $(ROOT_UI_V2))
V3_UI_VERSION ?= $(V3PRO_UI_V2)
V3_UX_VERSION ?= $(V3_UX_V1)
PUMP_RHYTHM_MODE_NUM := 6
else ifeq ($(ROOT_UI_VERSION), $(ROOT_UI_V3))
V3_UI_VERSION ?= $(V3PRO_UI_V3)
V3_UX_VERSION ?= $(V3_UX_V1)
PUMP_RHYTHM_MODE_NUM := 6
else ifeq ($(ROOT_UI_VERSION), $(ROOT_UI_V4))
V3_UI_VERSION ?= $(V3PRO_UI_V4)
V3_UX_VERSION ?= $(V3_UX_V2)
PUMP_RHYTHM_MODE_NUM := 3
else ifeq ($(ROOT_UI_VERSION), $(ROOT_UI_V5))
V3_UI_VERSION ?= $(V3PRO_UI_V5)
V3_UX_VERSION ?= $(V3_UX_V4)
PUMP_RHYTHM_MODE_NUM := 3
else ifeq ($(ROOT_UI_VERSION), $(ROOT_UI_V6))
V3_UI_VERSION ?= $(V3PRO_UI_V6)
V3_UX_VERSION ?= $(V3_UX_V4)
PUMP_RHYTHM_MODE_NUM := 3
else 
$(warning Invalid UI version)
endif
# 长按硬件重启
CFG_POWER_KEY_HW_RESET_ENABLE := 1
else
ifeq ($(ROOT_UI_VERSION), $(ROOT_UI_V2))
V3_UI_VERSION ?= $(V3_UI_V2)
V3_UX_VERSION ?= $(V3_UX_V1)
PUMP_RHYTHM_MODE_NUM := 6
else ifeq ($(ROOT_UI_VERSION), $(ROOT_UI_V3))
V3_UI_VERSION ?= $(V3_UI_V3)
V3_UX_VERSION ?= $(V3_UX_V1)
PUMP_RHYTHM_MODE_NUM := 6
else ifeq ($(ROOT_UI_VERSION), $(ROOT_UI_V4))
V3_UI_VERSION ?= $(V3_UI_V4)
V3_UX_VERSION ?= $(V3_UX_V2)
PUMP_RHYTHM_MODE_NUM := 3
else ifeq ($(ROOT_UI_VERSION), $(ROOT_UI_V5))
V3_UI_VERSION ?= $(V3_UI_V5)
V3_UX_VERSION ?= $(V3_UX_V4)
PUMP_RHYTHM_MODE_NUM := 3
else ifeq ($(ROOT_UI_VERSION), $(ROOT_UI_V6))
V3_UI_VERSION ?= $(V3_UI_V6)
V3_UX_VERSION ?= $(V3_UX_V4)
PUMP_RHYTHM_MODE_NUM := 3
else
$(warning Invalid UI version)
endif
# 长按硬件重启
CFG_POWER_KEY_HW_RESET_ENABLE := 0
endif

# BLE 头文件子目录
BLE_INC_DIRS := $(shell find $(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/bthost -type d)\
# BLE 头文件
BLE_INC_FLAGS := $(patsubst %,-I%,$(BLE_INC_DIRS))

# UI overlay：按 V3_UI_VERSION 选定子目录（仅计算当前版本）
ifeq ($(V3_UI_VERSION), $(V3_UI_V2))
UI_OVERLAY_SUBDIR := v3_ui_v2
else ifeq ($(V3_UI_VERSION), $(V3PRO_UI_V2))
UI_OVERLAY_SUBDIR := v3pro_ui_v2
else ifeq ($(V3_UI_VERSION), $(V3_UI_V3))
UI_OVERLAY_SUBDIR := v3_ui_v3
else ifeq ($(V3_UI_VERSION), $(V3PRO_UI_V3))
UI_OVERLAY_SUBDIR := v3pro_ui_v3
else ifeq ($(V3_UI_VERSION), $(V3_UI_V4))
UI_OVERLAY_SUBDIR := v3_ui_v4
else ifeq ($(V3_UI_VERSION), $(V3PRO_UI_V4))
UI_OVERLAY_SUBDIR := v3pro_ui_v4
else ifeq ($(V3_UI_VERSION), $(V3_UI_V5))
UI_OVERLAY_SUBDIR := v3_ui_v5
else ifeq ($(V3_UI_VERSION), $(V3PRO_UI_V5))
UI_OVERLAY_SUBDIR := v3pro_ui_v5
else ifeq ($(V3_UI_VERSION), $(V3_UI_V6))
UI_OVERLAY_SUBDIR := v3_ui_v6
else ifeq ($(V3_UI_VERSION), $(V3PRO_UI_V6))
UI_OVERLAY_SUBDIR := v3pro_ui_v6
else
$(warning Invalid UI version)
endif
ifneq ($(UI_OVERLAY_SUBDIR),)
UI_OVERLAY_DIR := $(PROJECT_ROOT)/overlay/ui/$(UI_OVERLAY_SUBDIR)
UI_INC_DIRECT_FLAGS := $(patsubst %,-I%,$(shell find $(UI_OVERLAY_DIR) -type d))
UI_LCD_DIRECT_SRCS := $(call rwildcard,$(UI_OVERLAY_DIR))
endif

# windows overlay：按 V3_UX_VERSION 选定子目录（仅计算当前版本）
ifeq ($(V3_UX_VERSION), $(V3_UX_V1))
WINDOWS_OVERLAY_SUBDIR := windows_v1
else ifeq ($(V3_UX_VERSION), $(V3_UX_V2))
WINDOWS_OVERLAY_SUBDIR := windows_v2
else ifeq ($(V3_UX_VERSION), $(V3_UX_V3))
WINDOWS_OVERLAY_SUBDIR := windows_v3
else ifeq ($(V3_UX_VERSION), $(V3_UX_V4))
WINDOWS_OVERLAY_SUBDIR := windows_v4
else
$(warning Invalid UX version)
endif
ifneq ($(WINDOWS_OVERLAY_SUBDIR),)
WINDOWS_OVERLAY_DIR := $(PROJECT_ROOT)/overlay/windows/$(WINDOWS_OVERLAY_SUBDIR)
WINDOWS_UX_INC_FLAGS := $(patsubst %,-I%,$(shell find $(WINDOWS_OVERLAY_DIR) -type d))
WINDOWS_UX_SRCS := $(call rwildcard,$(WINDOWS_OVERLAY_DIR))
endif


# Protocol 头文件子目录
PROTOCOL_INC_DIRS := $(shell find $(PROJECT_ROOT)/overlay/protocol -type d)
# Protocol 头文件
PROTOCOL_INC_FLAGS := $(patsubst %,-I%,$(PROTOCOL_INC_DIRS))

# Servers 头文件子目录
COMMON_SERVERS_INC_DIRS := $(shell find $(PROJECT_ROOT)/overlay/servers/common_server -type d)
# Servers 头文件
COMMON_SERVERS_INC_FLAGS := $(patsubst %,-I%,$(COMMON_SERVERS_INC_DIRS))
# Servers 源码目录
COMMON_SERVERS_DIR := $(PROJECT_ROOT)/overlay/servers/common_server
# Servers 源码文件
COMMON_SERVERS_SRCS := $(call rwildcard,$(COMMON_SERVERS_DIR))

# App old protocol Servers 头文件子目录
APP_OLD_PROTOCOL_SERVERS_INC_DIRS := $(shell find $(PROJECT_ROOT)/overlay/servers/app_old_protocol_server -type d)
# App old protocol Servers 头文件
APP_OLD_PROTOCOL_SERVERS_INC_FLAGS := $(patsubst %,-I%,$(APP_OLD_PROTOCOL_SERVERS_INC_DIRS))
# App old protocol Servers 源码目录
APP_OLD_PROTOCOL_SERVERS_DIR := $(PROJECT_ROOT)/overlay/servers/app_old_protocol_server
# App old protocol Servers 源码文件
APP_OLD_PROTOCOL_SERVERS_SRCS := $(call rwildcard,$(APP_OLD_PROTOCOL_SERVERS_DIR))

# Factory Server Client 头文件子目录
FACTORY_SERVER_CLIENT_INC_DIRS := $(shell find $(PROJECT_ROOT)/overlay/servers/factory_server_client -type d)
# Factory Server Client 头文件
FACTORY_SERVER_CLIENT_INC_FLAGS := $(patsubst %,-I%,$(FACTORY_SERVER_CLIENT_INC_DIRS))
# Factory Server Client 源码目录
FACTORY_SERVER_CLIENT_DIR := $(PROJECT_ROOT)/overlay/servers/factory_server_client
# Factory Server Client 源码文件
FACTORY_SERVER_CLIENT_SRCS := $(call rwildcard,$(FACTORY_SERVER_CLIENT_DIR))

# master Server Client 头文件子目录
MASTER_SERVER_CLIENT_INC_DIRS := $(shell find $(PROJECT_ROOT)/overlay/servers/master_server_client -type d)
# master Server Client 头文件
MASTER_SERVER_CLIENT_INC_FLAGS := $(patsubst %,-I%,$(MASTER_SERVER_CLIENT_INC_DIRS))
# master Server Client 源码目录
MASTER_SERVER_CLIENT_DIR := $(PROJECT_ROOT)/overlay/servers/master_server_client
# master Server Client 源码文件
MASTER_SERVER_CLIENT_SRCS := $(call rwildcard,$(MASTER_SERVER_CLIENT_DIR))

# 数据埋点协议 Server Client 头文件子目录
DATA_BURY_PROTOCOL_SERVER_CLIENT_INC_DIRS := $(shell find $(PROJECT_ROOT)/overlay/servers/data_bury_protocol_server_client -type d)
# 数据埋点协议 Server Client 头文件
DATA_BURY_PROTOCOL_SERVER_CLIENT_INC_FLAGS := $(patsubst %,-I%,$(DATA_BURY_PROTOCOL_SERVER_CLIENT_INC_DIRS))
# 数据埋点协议 Server Client 源码目录
DATA_BURY_PROTOCOL_SERVER_CLIENT_DIR := $(PROJECT_ROOT)/overlay/servers/data_bury_protocol_server_client
# 数据埋点协议 Server Client 源码文件
DATA_BURY_PROTOCOL_SERVER_CLIENT_SRCS := $(call rwildcard,$(DATA_BURY_PROTOCOL_SERVER_CLIENT_DIR))

# 数据埋点业务 Server 头文件子目录
DATA_BURY_SERVER_INC_DIRS := $(shell find $(PROJECT_ROOT)/overlay/servers/data_bury_server -type d)
# 数据埋点业务 Server 头文件
DATA_BURY_SERVER_INC_FLAGS := $(patsubst %,-I%,$(DATA_BURY_SERVER_INC_DIRS))
# 数据埋点业务 Server 源码目录
DATA_BURY_SERVER_DIR := $(PROJECT_ROOT)/overlay/servers/data_bury_server
# 数据埋点业务 Server 源码文件
DATA_BURY_SERVER_SRCS := $(call rwildcard,$(DATA_BURY_SERVER_DIR))

# OTA Server Client 头文件子目录
OTA_SERVER_CLIENT_INC_DIRS := $(shell find $(PROJECT_ROOT)/overlay/servers/ota_server_client -type d)
# OTA Server Client 头文件
OTA_SERVER_CLIENT_INC_FLAGS := $(patsubst %,-I%,$(OTA_SERVER_CLIENT_INC_DIRS))
# OTA Server Client 源码目录
OTA_SERVER_CLIENT_DIR := $(PROJECT_ROOT)/overlay/servers/ota_server_client
# OTA Server Client 源码文件
OTA_SERVER_CLIENT_SRCS := $(call rwildcard,$(OTA_SERVER_CLIENT_DIR))


# -------------- SDK 头文件路径（按功能模块拆分）--------------

# SDK 核心头文件
SDK_CORE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/include \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/os_base/include \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/apps/main \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/apps/factory

# SDK RTOS 头文件
SDK_RTOS_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/include/rtos/rtx5 \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/include/rtos/rtx \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/rtos/rtx5 \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/rtos/rtx5/rtx_config

# SDK HAL 头文件
SDK_HAL_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/hal \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/drivers/ana \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/drivers/bt \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/drivers/spi_flash \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/cmsis/inc

# SDK 服务头文件
SDK_SERVICES_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/services/nv_section/include \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/services/nv_section/log_section \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/services/norflash_api \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/services/nv_section/factory_section \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/services/ota/bes_ota/inc

# SDK 工具头文件
SDK_UTILS_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/utils/heap \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/utils/libc/inc \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/utils/hwtimer_list \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/utils/mbedtls/include \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/utils/mbedtls/include/mbedtls \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/utils/mbedtls/include/psa

# SDK LVGL 头文件
SDK_LVGL_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/thirdparty/lvgl \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/thirdparty/lvgl/bes \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/thirdparty/lvgl/bes/examples/res \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/thirdparty/lvgl/bes/ports \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/thirdparty/lvgl/bes/disp \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/thirdparty/lvgl/lv_drivers \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/thirdparty/lvgl/lv_widgets

# SDK LCD 头文件
SDK_LCD_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/drivers/graphic/lcd \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/drivers/graphic/lcd/chip \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/platform/drivers/graphic/lcd/interface

# SDK JPEG 头文件
SDK_JPEG_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/platform_chip/bes/sdk/tests/hwtest/jpegdec \

# Boards 头文件
BOARDS_INC_FLAGS := \
	-I$(PROJECT_ROOT)/boards

# ADC 驱动头文件
DRIVERS_ADC_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/adc

# 气压传感器驱动头文件
DRIVERS_AIR_PRESSURE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/air_pressure
ifeq ($(V3_HARDWARE_VERSION), $(V3_2710IMC_HARDWARE_V1))
DRIVERS_AIR_PRESSURE_INC_FLAGS += -I$(PROJECT_ROOT)/drivers/air_pressure/chip/wf100d
else
ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_AIR_PRESSURE_INC_FLAGS += -I$(PROJECT_ROOT)/drivers/air_pressure/chip/limscg04pcns5
endif
endif	

# 光传感器驱动头文件
DRIVERS_LIGHT_SENSOR_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/light_sensor

# 音频驱动头文件
DRIVERS_AUDIO_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/audio

# 基础驱动头文件
DRIVERS_BASE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/base

# 电池驱动头文件
DRIVERS_BATTERY_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/battery \
	-I$(PROJECT_ROOT)/drivers/battery/charge

ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V1) $(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_BATTERY_INC_FLAGS += -I$(PROJECT_ROOT)/drivers/battery/charge_pd
endif

# BLE 驱动头文件
DRIVERS_BLE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/ble \
	-I$(PROJECT_ROOT)/drivers/ble/services \
	-I$(PROJECT_ROOT)/drivers/ble/advs \
	-I$(PROJECT_ROOT)/drivers/ble/connect \
	-I$(PROJECT_ROOT)/drivers/ble/ble_test

# 通用驱动头文件
DRIVERS_COMMON_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/common

# 数据库驱动头文件
DRIVERS_DATABASE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/database

# FatFS 驱动头文件
DRIVERS_FATFS_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/fatfs

# Flash 驱动头文件
DRIVERS_FLASH_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/flash

# 力传感器驱动头文件
DRIVERS_FORCE_SENSOR_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/force_sensor

# I2C 驱动头文件
DRIVERS_I2C_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/i2c

# IMU 驱动头文件
DRIVERS_IMU_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/imu

DRIVERS_KEY_INC_FLAGS := -I$(PROJECT_ROOT)/drivers/key
ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_KEY_INC_FLAGS += -I$(PROJECT_ROOT)/drivers/key/chip/hyn
else
DRIVERS_KEY_INC_FLAGS += -I$(PROJECT_ROOT)/drivers/key/chip/sc09b
endif
ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V1) $(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_KEY_INC_FLAGS += -I$(PROJECT_ROOT)/drivers/key/chip/knob
endif

# LCD 驱动头文件
DRIVERS_LCD_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/lcd 

ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V1) $(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_LCD_INC_FLAGS += -I$(PROJECT_ROOT)/drivers/lcd/interface/lcd_bes_qspi
else
DRIVERS_LCD_INC_FLAGS += -I$(PROJECT_ROOT)/drivers/lcd/interface/lcd_uart
endif

# LED 驱动头文件
DRIVERS_LED_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/led

# 电机驱动头文件
DRIVERS_MOTOR_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/motor
DRIVERS_MOTOR_INC_FLAGS += -I$(PROJECT_ROOT)/drivers/motor/chip/motor_pump

# UART 驱动头文件
DRIVERS_UART_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/uart

# WiFi 驱动头文件
DRIVERS_WIFI_INC_FLAGS := \
	-I$(PROJECT_ROOT)/drivers/wifi

# 汇总所有驱动头文件
DRIVERS_INC_FLAGS := \
	$(BOARDS_INC_FLAGS) \
	$(DRIVERS_ADC_INC_FLAGS) \
	$(DRIVERS_AIR_PRESSURE_INC_FLAGS) \
	$(DRIVERS_AUDIO_INC_FLAGS) \
	$(DRIVERS_BASE_INC_FLAGS) \
	$(DRIVERS_BATTERY_INC_FLAGS) \
	$(DRIVERS_BLE_INC_FLAGS) \
	$(DRIVERS_COMMON_INC_FLAGS) \
	$(DRIVERS_DATABASE_INC_FLAGS) \
	$(DRIVERS_FATFS_INC_FLAGS) \
	$(DRIVERS_FLASH_INC_FLAGS) \
	$(DRIVERS_FORCE_SENSOR_INC_FLAGS) \
	$(DRIVERS_I2C_INC_FLAGS) \
	$(DRIVERS_IMU_INC_FLAGS) \
	$(DRIVERS_KEY_INC_FLAGS) \
	$(DRIVERS_LCD_INC_FLAGS) \
	$(DRIVERS_LED_INC_FLAGS) \
	$(DRIVERS_MOTOR_INC_FLAGS) \
	$(DRIVERS_UART_INC_FLAGS) \
	$(DRIVERS_WIFI_INC_FLAGS)

ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V1) $(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_INC_FLAGS += $(DRIVERS_LIGHT_SENSOR_INC_FLAGS)
endif	

# 基础模块头文件
BASE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/include \
	-I$(PROJECT_ROOT)/trunk/extern \
	-I$(PROJECT_ROOT)/trunk/root_source/base \
	-I$(PROJECT_ROOT)/trunk/root_source/hal \
	-I$(PROJECT_ROOT)/trunk/root_source/utils \
	-I$(PROJECT_ROOT)/trunk/root_source/misc

# 外部库头文件
EXTERNAL_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/external/littlefs \
	-I$(PROJECT_ROOT)/trunk/external/littlefs/fds \
	-I$(PROJECT_ROOT)/trunk/external/scan_key \
	-I$(PROJECT_ROOT)/trunk/external/md5

# 算法模块头文件
ALGORITHM_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/algo \
	-I$(PROJECT_ROOT)/library/root_algo/include \
	-I$(PROJECT_ROOT)/library/root_algo/wear_detection \
	-I$(PROJECT_ROOT)/library/root_algo \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/algo \
	-I$(PROJECT_ROOT)/trunk/root_source/algorithm

# BLE 模块头文件
BLE_MODULE_INC_FLAGS := \
	$(BLE_INC_FLAGS) \
	-I$(PROJECT_ROOT)/overlay/ble/factory_service \
	-I$(PROJECT_ROOT)/overlay/ble/master_service \
	-I$(PROJECT_ROOT)/overlay/ble/remot_service \
	-I$(PROJECT_ROOT)/overlay/ble/data_bury_service \
	-I$(PROJECT_ROOT)/trunk/root_source/ble

# 吸奶泵模块头文件
BREAST_PUMP_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/breast_pump/mode \
	-I$(PROJECT_ROOT)/overlay/breast_pump/table \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/breast_pump \
	-I$(PROJECT_ROOT)/trunk/root_source/breast_pump

# 电机模块头文件
MOTOR_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/motor \
	-I$(PROJECT_ROOT)/trunk/root_source/motor

MOTOR_INC_FLAGS += -I$(PROJECT_ROOT)/trunk/root_source/motor/breast_pump_policy

# 传感器模块头文件
SENSOR_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/sensor/imu \
	-I$(PROJECT_ROOT)/overlay/sensor/force_sensor \
	-I$(PROJECT_ROOT)/overlay/sensor/air_pressure \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/sensor \
	-I$(PROJECT_ROOT)/trunk/root_source/sensor
ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V1) $(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
SENSOR_INC_FLAGS += -I$(PROJECT_ROOT)/overlay/sensor/light_sensor
endif	

# 音频模块头文件
AUDIO_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/root_source/audio

# 按键输入模块头文件
INPUT_KEY_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/input_key \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/input_key \
	-I$(PROJECT_ROOT)/trunk/root_source/key_input

# UI 模块头文件
UI_MODULE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/ui \
	-I$(PROJECT_ROOT)/trunk/root_source/ui \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/ui/
UI_MODULE_INC_FLAGS += $(UI_INC_DIRECT_FLAGS)

# 字体模块头文件
FONT_V2_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/font/font_oppo_sans_12 \
	-I$(PROJECT_ROOT)/overlay/font/font_oppo_sans_14 \
	-I$(PROJECT_ROOT)/overlay/font/font_oppo_sans_16 \
	-I$(PROJECT_ROOT)/overlay/font/font_oppo_sans_20 \
	-I$(PROJECT_ROOT)/overlay/font/font_oppo_sans_24 \
	-I$(PROJECT_ROOT)/overlay/font/font_oppo_sans_72 \
	-I$(PROJECT_ROOT)/overlay/font/font_oppo_sans_96 \
	-I$(PROJECT_ROOT)/overlay/font/font_quicksand_64 \
	-I$(PROJECT_ROOT)/overlay/font/font_quicksand_96 

FONT_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/font
FONT_INC_FLAGS += $(FONT_V2_INC_FLAGS)

# 窗口模块头文件
WINDOWS_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/windows \
	-I$(PROJECT_ROOT)/trunk/root_source/windows
WINDOWS_INC_FLAGS += $(WINDOWS_UX_INC_FLAGS)

# 飞行模式模块头文件
AIRPLANE_MODE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/airplane_mode \
	-I$(PROJECT_ROOT)/trunk/root_source/airplane_mode

# 电源管理模块头文件
POWER_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/power \
	-I$(PROJECT_ROOT)/trunk/root_source/pm

# 协议模块头文件
PROTOCOL_MODULE_INC_FLAGS := \
	$(PROTOCOL_INC_FLAGS) \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/protocol \
	-I$(PROJECT_ROOT)/trunk/root_source/protocol

# 服务模块头文件
SERVERS_INC_FLAGS := \
	$(COMMON_SERVERS_INC_FLAGS) \
	$(FACTORY_SERVER_CLIENT_INC_FLAGS) \
	$(APP_OLD_PROTOCOL_SERVERS_INC_FLAGS) \
	$(MASTER_SERVER_CLIENT_INC_FLAGS) \
	$(DATA_BURY_PROTOCOL_SERVER_CLIENT_INC_FLAGS) \
	$(DATA_BURY_SERVER_INC_FLAGS) \
	$(OTA_SERVER_CLIENT_INC_FLAGS) \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/servers

# 存储模块头文件
STORAGE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/storage \
	-I$(PROJECT_ROOT)/overlay/storage/factory \
	-I$(PROJECT_ROOT)/overlay/storage/breast_pump \
	-I$(PROJECT_ROOT)/overlay/storage/resource \
	-I$(PROJECT_ROOT)/overlay/storage/system \
	-I$(PROJECT_ROOT)/overlay/storage/data_bury \
	-I$(PROJECT_ROOT)/overlay/protocol/app_data_bury_protocol \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/storage \
	-I$(PROJECT_ROOT)/trunk/root_source/storage

# 资源模块头文件
RESOURCE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/root_source/resource

# 网络模块头文件
NETWORK_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/root_source/network

# OTA 模块头文件
OTA_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/root_source/ota

# 产品测试模块头文件
PRODUCT_TESTS_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/product_tests/comm \
	-I$(PROJECT_ROOT)/overlay/product_tests/factory_tests \
	-I$(PROJECT_ROOT)/overlay/product_tests/hardware_self_tests \
	-I$(PROJECT_ROOT)/overlay/product_tests/stress_tests \
	-I$(PROJECT_ROOT)/overlay/product_tests \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/product_tests \
	-I$(PROJECT_ROOT)/trunk/root_source/tests

# 数据埋点 Manager 模块头文件
DATA_BURY_MODULE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/root_source/data_bury \
	-I$(PROJECT_ROOT)/overlay/data_bury \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/data_bury \
	$(DATA_BURY_SERVER_INC_FLAGS)

# 资源模块头文件
RESOURCE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/trunk/root_source/resource \
	-I$(PROJECT_ROOT)/overlay/resources\
	-I$(PROJECT_ROOT)/overlay/resources/reference

ifeq ($(V3_UI_VERSION), $(V3_UI_V2))
RESOURCE_INC_FLAGS += -I$(PROJECT_ROOT)/overlay/resources/v3_res_v2
else ifeq ($(V3_UI_VERSION), $(V3PRO_UI_V2))
RESOURCE_INC_FLAGS += -I$(PROJECT_ROOT)/overlay/resources/v3pro_res_v2
else ifeq ($(V3_UI_VERSION), $(V3_UI_V3))
RESOURCE_INC_FLAGS += -I$(PROJECT_ROOT)/overlay/resources/v3_res_v3
else ifeq ($(V3_UI_VERSION), $(V3PRO_UI_V3))
RESOURCE_INC_FLAGS += -I$(PROJECT_ROOT)/overlay/resources/v3pro_res_v3
else ifeq ($(V3_UI_VERSION), $(V3_UI_V4))
RESOURCE_INC_FLAGS += -I$(PROJECT_ROOT)/overlay/resources/v3_res_v4
else ifeq ($(V3_UI_VERSION), $(V3PRO_UI_V4))
RESOURCE_INC_FLAGS += -I$(PROJECT_ROOT)/overlay/resources/v3pro_res_v4
else ifeq ($(V3_UI_VERSION), $(V3_UI_V5))
RESOURCE_INC_FLAGS += -I$(PROJECT_ROOT)/overlay/resources/v3_res_v5
else ifeq ($(V3_UI_VERSION), $(V3PRO_UI_V5))
RESOURCE_INC_FLAGS += -I$(PROJECT_ROOT)/overlay/resources/v3pro_res_v5
else ifeq ($(V3_UI_VERSION), $(V3_UI_V6))
RESOURCE_INC_FLAGS += -I$(PROJECT_ROOT)/overlay/resources/v3_res_v6
else ifeq ($(V3_UI_VERSION), $(V3PRO_UI_V6))
RESOURCE_INC_FLAGS += -I$(PROJECT_ROOT)/overlay/resources/v3pro_res_v6
else
$(warning Invalid UI version)
endif

# 电池模块头文件
BATTERY_MODULE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/battery \
	-I$(PROJECT_ROOT)/trunk/root_source/battery

# 夜灯（仅 V3PRO）
ifeq ($(ROOT_PRODUCT_ID),$(ROOT_PRODUCT_V3PRO_ID))
LED_NIGHT_LIGHT_INC := -I$(PROJECT_ROOT)/overlay/night_light
LED_NIGHT_LIGHT_DRIVER_SRCS := $(wildcard $(PROJECT_ROOT)/drivers/led/night_light/*.c)
LED_NIGHT_LIGHT_OVERLAY_SRCS := $(wildcard $(PROJECT_ROOT)/overlay/night_light/*.c)
else
LED_NIGHT_LIGHT_INC :=
LED_NIGHT_LIGHT_DRIVER_SRCS :=
LED_NIGHT_LIGHT_OVERLAY_SRCS :=
endif

# 灯模块头文件
LED_MODULE_INC_FLAGS := \
	-I$(PROJECT_ROOT)/overlay/adaptive_dimming \
	$(LED_NIGHT_LIGHT_INC) \
	-I$(PROJECT_ROOT)/overlay/framework_hooks/led \
	-I$(PROJECT_ROOT)/trunk/root_source/led

# 汇总所有 SDK 头文件路径
SDK_INCLUDE_PATHS := \
	$(SDK_CORE_INC_FLAGS) \
	$(SDK_RTOS_INC_FLAGS) \
	$(SDK_HAL_INC_FLAGS) \
	$(SDK_SERVICES_INC_FLAGS) \
	$(SDK_UTILS_INC_FLAGS) \
	$(SDK_LVGL_INC_FLAGS) \
	$(SDK_LCD_INC_FLAGS) \
	$(SDK_JPEG_INC_FLAGS) \
	$(DRIVERS_INC_FLAGS) \
	$(BASE_INC_FLAGS) \
	$(EXTERNAL_INC_FLAGS) \
	$(ALGORITHM_INC_FLAGS) \
	$(BLE_MODULE_INC_FLAGS) \
	$(BREAST_PUMP_INC_FLAGS) \
	$(MOTOR_INC_FLAGS) \
	$(SENSOR_INC_FLAGS) \
	$(AUDIO_INC_FLAGS) \
	$(INPUT_KEY_INC_FLAGS) \
	$(UI_MODULE_INC_FLAGS) \
	$(BATTERY_MODULE_INC_FLAGS) \
	$(FONT_INC_FLAGS) \
	$(WINDOWS_INC_FLAGS) \
	$(AIRPLANE_MODE_INC_FLAGS) \
	$(POWER_INC_FLAGS) \
	$(PROTOCOL_MODULE_INC_FLAGS) \
	$(SERVERS_INC_FLAGS) \
	$(STORAGE_INC_FLAGS) \
	$(RESOURCE_INC_FLAGS) \
	$(NETWORK_INC_FLAGS) \
	$(OTA_INC_FLAGS) \
	$(PRODUCT_TESTS_INC_FLAGS) \
	$(DATA_BURY_MODULE_INC_FLAGS) \
	$(RESOURCE_INC_FLAGS) \
	$(LED_MODULE_INC_FLAGS) 

# 添加项目根目录和 SDK 头文件路径
CPPFLAGS ?= -I$(PROJECT_ROOT) -I$(PROJECT_ROOT)/trunk $(SDK_INCLUDE_PATHS)

# custom app switch
ifeq ($(CFG_CUSTOM_APP),1)
CPPFLAGS += -DCFG_CUSTOM_APP
endif

# LVGL开关
ifeq ($(CFG_LVGL_ENABLE),1)
CPPFLAGS += -DCFG_LVGL_ENABLE
endif

# LVGL snapshot开关
ifeq ($(CFG_LVGL_SNAPSHOT_ENABLE),1)
CPPFLAGS += -DLVGL_SNAPSHOT
endif

# 外部flash开关
ifeq ($(CFG_EXTERNAL_FLASH_ENABLE),1)
SPI_FLASH1 := 1
ENABLE_FLASH1 := 1
SPI_IOMUX_4WIRE := 1
CPPFLAGS += -DSPI_FLASH1=$(SPI_FLASH1)
CPPFLAGS += -DENABLE_FLASH1=$(ENABLE_FLASH1)
CPPFLAGS += -DSPI_IOMUX_4WIRE=$(SPI_IOMUX_4WIRE)
endif

# BT 校准开关
ifeq ($(CFG_BT_CALIB),1)
BT_CALIB := 1
CPPFLAGS += -DBT_CALIB=$(BT_CALIB)
endif
# LVGL 日志级别配置
# 可选值: 0=TRACE, 1=INFO, 2=WARN, 3=ERROR, 4=USER, 5=NONE
CONFIG_LV_LOG_LEVEL ?= 2
CPPFLAGS += -DCONFIG_LV_LOG_LEVEL=$(CONFIG_LV_LOG_LEVEL)

# LVGL 主题暗色模式配置
# 可选值: 0=亮色模式, 1=暗色模式（黑色背景）
CONFIG_LV_THEME_DEFAULT_DARK ?= 1
CPPFLAGS += -DCONFIG_LV_THEME_DEFAULT_DARK=$(CONFIG_LV_THEME_DEFAULT_DARK)

# 硬件版本
CPPFLAGS += -DV3_2710IMC_HARDWARE_V1=$(V3_2710IMC_HARDWARE_V1)
CPPFLAGS += -DV3_2710IMC_HARDWARE_V2=$(V3_2710IMC_HARDWARE_V2)
CPPFLAGS += -DV3_2710IMP_HARDWARE_V1=$(V3_2710IMP_HARDWARE_V1)
CPPFLAGS += -DV3_2710IMC_HARDWARE_V3=$(V3_2710IMC_HARDWARE_V3)
CPPFLAGS += -DV3_2710IMC_HARDWARE_V4=$(V3_2710IMC_HARDWARE_V4)
CPPFLAGS += -DV3_HARDWARE_VERSION=$(V3_HARDWARE_VERSION)

# 产品型号
CPPFLAGS += -DROOT_PRODUCT_ID=$(ROOT_PRODUCT_ID)
CPPFLAGS += -DROOT_PRODUCT_V3_ID=$(ROOT_PRODUCT_V3_ID)
CPPFLAGS += -DROOT_PRODUCT_V3PRO_ID=$(ROOT_PRODUCT_V3PRO_ID)

# UI版本
CPPFLAGS += -DV3_UI_VERSION=$(V3_UI_VERSION)
CPPFLAGS += -DV3_UI_V2=$(V3_UI_V2)
CPPFLAGS += -DV3PRO_UI_V2=$(V3PRO_UI_V2)
CPPFLAGS += -DV3_UI_V3=$(V3_UI_V3)
CPPFLAGS += -DV3PRO_UI_V3=$(V3PRO_UI_V3)
CPPFLAGS += -DV3_UI_V4=$(V3_UI_V4)
CPPFLAGS += -DV3PRO_UI_V4=$(V3PRO_UI_V4)
CPPFLAGS += -DPUMP_RHYTHM_MODE_NUM=$(PUMP_RHYTHM_MODE_NUM)

CPPFLAGS += -DROOT_UI_VERSION=$(ROOT_UI_VERSION)
CPPFLAGS += -DROOT_UI_V2=$(ROOT_UI_V2)
CPPFLAGS += -DROOT_UI_V3=$(ROOT_UI_V3)
CPPFLAGS += -DROOT_UI_V4=$(ROOT_UI_V4)
CPPFLAGS += -DROOT_UI_V5=$(ROOT_UI_V5)

CPPFLAGS += -DV3_UX_VERSION=$(V3_UX_VERSION)
CPPFLAGS += -DV3_UX_V1=$(V3_UX_V1)
CPPFLAGS += -DV3_UX_V2=$(V3_UX_V2)
CPPFLAGS += -DV3_UX_V3=$(V3_UX_V3)
CPPFLAGS += -DV3_UX_V4=$(V3_UX_V4)

# 调试日志相关宏定义
ifeq ($(REL_TRACE_ENABLE),1)
CPPFLAGS += -DREL_TRACE_ENABLE
endif

# RTC 日历相关宏定义
ifeq ($(RTC_CALENDAR),1)
CPPFLAGS += -DRTC_CALENDAR
endif

# OTA 事件回调相关宏定义
ifeq ($(CFG_OTA_EVENT_CALLBACK),1)
CPPFLAGS += -DCFG_OTA_EVENT_CALLBACK
endif

# PMU clock use external crystal switch
ifeq ($(PMU_CLK_USE_EXT_CRYSTAL),1)
export CALIB_SLOW_TIMER := 0
CPPFLAGS += -DPMU_CLK_USE_EXT_CRYSTAL
else
export CALIB_SLOW_TIMER := 1
CPPFLAGS += -DCALIB_SLOW_TIMER
endif

# PSRAM enable switch
ifeq ($(PSRAM_ENABLE),1)
CPPFLAGS += -DPSRAM_ENABLE
endif

# PWM IOMUX 相关宏定义
CPPFLAGS += -DPWM2_IOMUX_INDEX=$(PWM2_IOMUX_INDEX)
CPPFLAGS += -DPWM3_IOMUX_INDEX=$(PWM3_IOMUX_INDEX)
ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
CPPFLAGS += -DPWM4_IOMUX_INDEX=$(PWM4_IOMUX_INDEX)
CPPFLAGS += -DPWM5_IOMUX_INDEX=$(PWM5_IOMUX_INDEX)
endif

# LITTLEFS 相关宏定义
CPPFLAGS += -DLITTLEFS_SECTION_SIZE=$(LITTLEFS_SECTION_SIZE)

# LUTE 相关宏定义
CPPFLAGS += -DLUTE_PICTURE_RESOURCES_SECTION_SIZE=$(LUTE_PICTURE_RESOURCES_SECTION_SIZE)
CPPFLAGS += -DLUTE_RESERVED_SECTION_SIZE=$(LUTE_RESERVED_SECTION_SIZE)

# 工厂数据相关宏定义
CPPFLAGS += -DFACTORY_DATA_SECTION_SIZE=$(FACTORY_DATA_SECTION_SIZE)

# 工厂模式相关宏定义（蓝牙测试功能需要）
FACTORY_MODE ?= 0
ifeq ($(FACTORY_MODE),1)
export FACTORY_MODE
CPPFLAGS += -D__FACTORY_MODE_SUPPORT__
endif

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

# -------------- 应用源码（按功能模块拆分）--------------

# OS 基础源码
OS_BASE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/trunk/platform_chip/bes/os_base/*.c)

# ADC 驱动源码
DRIVERS_ADC_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/adc/*.c)

# 气压传感器驱动源码
DRIVERS_AIR_PRESSURE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/air_pressure/*.c) 
ifeq ($(V3_HARDWARE_VERSION), $(V3_2710IMC_HARDWARE_V1))
DRIVERS_AIR_PRESSURE_SRCS += $(wildcard $(PROJECT_ROOT)/drivers/air_pressure/chip/wf100d/*.c)
else 
ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_AIR_PRESSURE_SRCS += $(wildcard $(PROJECT_ROOT)/drivers/air_pressure/chip/limscg04pcns5/*.c)
endif
endif

# 音频驱动源码
DRIVERS_AUDIO_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/audio/*.c)

# 基础驱动源码
DRIVERS_BASE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/base/*.c)

# 电池驱动源码
DRIVERS_BATTERY_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/battery/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/battery/charge/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/battery/adc/*.c)
ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V1) $(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_BATTERY_SRCS += $(wildcard $(PROJECT_ROOT)/drivers/battery/charge_pd/*.c)
endif

# BLE 驱动源码
DRIVERS_BLE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/ble/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/ble/advs/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/ble/connect/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/ble/services/*.c) \
	$(wildcard $(PROJECT_ROOT)/drivers/ble/ble_test/*.c)

# 通用驱动源码
DRIVERS_COMMON_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/common/*.c)

# 数据库驱动源码
DRIVERS_DATABASE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/database/*.c)

# FatFS 驱动源码
DRIVERS_FATFS_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/fatfs/*.c)

# Flash 驱动源码
DRIVERS_FLASH_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/flash/*.c)

# 力传感器驱动源码
DRIVERS_FORCE_SENSOR_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/force_sensor/*.c)

# 光传感器驱动源码
DRIVERS_LIGHT_SENSOR_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/light_sensor/*.c)

# I2C 驱动源码
DRIVERS_I2C_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/i2c/*.c)

# IMU 驱动源码
DRIVERS_IMU_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/imu/*.c)

# 按键驱动源码（触摸芯片二选一）
DRIVERS_KEY_SRCS := $(wildcard $(PROJECT_ROOT)/drivers/key/*.c)
ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_KEY_SRCS += $(wildcard $(PROJECT_ROOT)/drivers/key/chip/hyn/*.c)
$(info [KEY] TOUCH_KEY_CHIP=hyn -> building HYN driver)
else
DRIVERS_KEY_SRCS += $(wildcard $(PROJECT_ROOT)/drivers/key/chip/sc09b/*.c)
$(info [KEY] TOUCH_KEY_CHIP=$(TOUCH_KEY_CHIP) -> building SC09B driver)
endif
ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V1) $(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_KEY_SRCS += $(wildcard $(PROJECT_ROOT)/drivers/key/chip/knob/*.c)
endif

# LCD 驱动源码
DRIVERS_LCD_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/lcd/*.c) 

ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V1) $(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_LCD_SRCS += $(wildcard $(PROJECT_ROOT)/drivers/lcd/interface/lcd_bes_qspi/*.c)
else
DRIVERS_LCD_SRCS += $(wildcard $(PROJECT_ROOT)/drivers/lcd/interface/lcd_uart/*.c)
endif

# LED 驱动源码
DRIVERS_LED_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/led/*.c) \
	$(LED_NIGHT_LIGHT_DRIVER_SRCS)

ifeq ($(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V1))
DRIVERS_LED_SRCS += $(wildcard $(PROJECT_ROOT)/drivers/led/led_tm1650/*.c)
else ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_LED_SRCS += $(wildcard $(PROJECT_ROOT)/drivers/led/led_htr3316/*.c)
else
DRIVERS_LED_SRCS += $(wildcard $(PROJECT_ROOT)/drivers/led/led_control/*.c)
endif

# 电机驱动源码C
DRIVERS_MOTOR_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/motor/*.c)
DRIVERS_MOTOR_SRCS += $(wildcard $(PROJECT_ROOT)/drivers/motor/chip/motor_pump/*.c)

# UART 驱动源码
DRIVERS_UART_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/uart/*.c)

# WiFi 驱动源码
DRIVERS_WIFI_SRCS := \
	$(wildcard $(PROJECT_ROOT)/drivers/wifi/*.c)

# 汇总所有驱动源码
DRIVERS_SRCS := \
	$(DRIVERS_ADC_SRCS) \
	$(DRIVERS_AIR_PRESSURE_SRCS) \
	$(DRIVERS_AUDIO_SRCS) \
	$(DRIVERS_BASE_SRCS) \
	$(DRIVERS_BATTERY_SRCS) \
	$(DRIVERS_BLE_SRCS) \
	$(DRIVERS_COMMON_SRCS) \
	$(DRIVERS_DATABASE_SRCS) \
	$(DRIVERS_FATFS_SRCS) \
	$(DRIVERS_FLASH_SRCS) \
	$(DRIVERS_FORCE_SENSOR_SRCS) \
	$(DRIVERS_I2C_SRCS) \
	$(DRIVERS_IMU_SRCS) \
	$(DRIVERS_KEY_SRCS) \
	$(DRIVERS_LCD_SRCS) \
	$(DRIVERS_LED_SRCS) \
	$(DRIVERS_MOTOR_SRCS) \
	$(DRIVERS_UART_SRCS) \
	$(DRIVERS_WIFI_SRCS)

ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V1) $(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
DRIVERS_SRCS += $(DRIVERS_LIGHT_SENSOR_SRCS)
endif	

# 基础模块源码
BASE_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/base/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/utils/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/misc/*.c)

# 外部库源码
EXTERNAL_SRCS := \
	$(wildcard $(PROJECT_ROOT)/trunk/external/littlefs/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/external/littlefs/fds/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/external/scan_key/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/external/md5/*.c)

# 佩戴检测库（源码目录有库则优先链接；无库则回退源码编译）
WEAR_DETECTION_LIB := $(PROJECT_ROOT)/library/root_algo/wear_detecttion/libwear_detection.a

# 算法模块源码
ALGORITHM_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/algo/*.c) \
	$(wildcard $(PROJECT_ROOT)/library/root_algo/*.c) \
	$(wildcard $(PROJECT_ROOT)/library/root_algo/include/*.c) \
	$(wildcard $(PROJECT_ROOT)/library/root_algo/wear_detection/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/algo/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/algorithm/*.c)

WEAR_DETECTION_SRC_CONFLICTS := \
	$(PROJECT_ROOT)/library/root_algo/wear_detecttion/v3_wear_detection.c \
	$(PROJECT_ROOT)/library/root_algo/wear_detecttion/v3_pressure_compensation_algo.c \
	$(PROJECT_ROOT)/library/root_algo/wear_detecttion/v3_target_curve_table.c

ifneq ($(wildcard $(WEAR_DETECTION_LIB)),)
ALGORITHM_MODULE_SRCS := $(filter-out $(WEAR_DETECTION_SRC_CONFLICTS),$(ALGORITHM_MODULE_SRCS))
WEAR_DETECTION_LINK_LIB := $(WEAR_DETECTION_LIB)
else
WEAR_DETECTION_LINK_LIB :=
endif

# BLE 模块源码
BLE_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/ble/factory_service/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/ble/master_service/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/ble/remot_service/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/ble/data_bury_service/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/ble/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/ble/*.c)

# 吸奶泵模块源码
BREAST_PUMP_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/breast_pump/mode/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/breast_pump/table/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/breast_pump/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/breast_pump/*.c)

# 电机模块源码
MOTOR_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/motor/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/motor/*.c) 
MOTOR_MODULE_SRCS += $(wildcard $(PROJECT_ROOT)/trunk/root_source/motor/breast_pump_policy/*.c)

# 传感器模块源码
SENSOR_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/sensor/imu/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/sensor/force_sensor/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/sensor/air_pressure/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/sensor/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/sensor/*.c)
ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V1) $(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
SENSOR_MODULE_SRCS += $(wildcard $(PROJECT_ROOT)/overlay/sensor/light_sensor/*.c)
endif	

# 音频模块源码
AUDIO_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/audio/*.c)

# 按键输入模块源码
INPUT_KEY_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/input_key/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/input_key/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/key_input/*.c)

# UI 模块源码
UI_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/ui/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/ui/*.c) 
UI_MODULE_SRCS += $(UI_LCD_DIRECT_SRCS)

# 字体模块源码
FONT_V2_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/font/font_oppo_sans_12/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/font/font_oppo_sans_14/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/font/font_oppo_sans_16/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/font/font_oppo_sans_20/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/font/font_oppo_sans_24/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/font/font_oppo_sans_72/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/font/font_oppo_sans_96/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/font/font_quicksand_64/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/font/font_quicksand_96/*.c) 

FONT_MODULE_SRCS := \
   $(wildcard $(PROJECT_ROOT)/overlay/font/*.c)
FONT_MODULE_SRCS += $(FONT_V2_SRCS)

# 窗口模块源码
WINDOWS_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/windows/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/windows/*.c)
WINDOWS_MODULE_SRCS += $(WINDOWS_UX_SRCS)

# 飞行模式模块源码
AIRPLANE_MODE_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/airplane_mode/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/airplane_mode/*.c)

# 电源管理模块源码
POWER_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/power/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/pm/*.c)

# 协议模块源码
PROTOCOL_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/protocol/app_old_commication_protocol/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/protocol/factory_protocol/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/protocol/common_protocl/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/protocol/app_data_bury_protocol/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/protocol/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/protocol/*.c)

# 服务模块源码
SERVERS_MODULE_SRCS := \
	$(COMMON_SERVERS_SRCS) \
	$(FACTORY_SERVER_CLIENT_SRCS) \
	$(MASTER_SERVER_CLIENT_SRCS) \
	$(APP_OLD_PROTOCOL_SERVERS_SRCS) \
	$(DATA_BURY_PROTOCOL_SERVER_CLIENT_SRCS) \
	$(DATA_BURY_SERVER_SRCS) \
	$(OTA_SERVER_CLIENT_SRCS) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/servers/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/servers/*.c)

# 存储模块源码
STORAGE_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/storage/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/storage/factory/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/storage/breast_pump/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/storage/resource/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/storage/system/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/storage/data_bury/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/storage/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/storage/*.c)

# 资源模块源码
RESOURCE_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/resource/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/resources/*.c)

# 网络模块源码
NETWORK_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/network/*.c)

# OTA 模块源码
OTA_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/ota/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/ota/*.c)

# 产品测试模块源码
PRODUCT_TESTS_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/product_tests/comm/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/product_tests/factory_tests/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/product_tests/hardware_self_tests/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/product_tests/stress_tests/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/product_tests/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/tests/*.c)

# 数据埋点 Manager 模块源码
DATA_BURY_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/data_bury/*.c) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/data_bury/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/data_bury/*.c)

# 框架钩子基础源码
FRAMEWORK_HOOKS_BASE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/*.c)

# 电池模块源码
BATTERY_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/battery/*.c)

# 灯模块源码
LED_MODULE_SRCS := \
	$(wildcard $(PROJECT_ROOT)/overlay/adaptive_dimming/*.c) \
	$(LED_NIGHT_LIGHT_OVERLAY_SRCS) \
	$(wildcard $(PROJECT_ROOT)/overlay/framework_hooks/led/*.c) \
	$(wildcard $(PROJECT_ROOT)/trunk/root_source/led/*.c)

# 汇总所有应用源码
EXTRA_SRCS := \
	$(OS_BASE_SRCS) \
	$(DRIVERS_SRCS) \
	$(BASE_MODULE_SRCS) \
	$(EXTERNAL_SRCS) \
	$(ALGORITHM_MODULE_SRCS) \
	$(BLE_MODULE_SRCS) \
	$(BREAST_PUMP_MODULE_SRCS) \
	$(MOTOR_MODULE_SRCS) \
	$(SENSOR_MODULE_SRCS) \
	$(AUDIO_MODULE_SRCS) \
	$(INPUT_KEY_MODULE_SRCS) \
	$(UI_MODULE_SRCS) \
	$(BATTERY_MODULE_SRCS) \
	$(FONT_MODULE_SRCS) \
	$(WINDOWS_MODULE_SRCS) \
	$(AIRPLANE_MODE_MODULE_SRCS) \
	$(POWER_MODULE_SRCS) \
	$(PROTOCOL_MODULE_SRCS) \
	$(SERVERS_MODULE_SRCS) \
	$(STORAGE_MODULE_SRCS) \
	$(RESOURCE_MODULE_SRCS) \
	$(NETWORK_MODULE_SRCS) \
	$(OTA_MODULE_SRCS) \
	$(PRODUCT_TESTS_MODULE_SRCS) \
	$(DATA_BURY_MODULE_SRCS) \
	$(FRAMEWORK_HOOKS_BASE_SRCS) \
	$(LED_MODULE_SRCS)

CC := $(if $(strip $(CROSS_COMPILE)),$(CROSS_COMPILE)gcc,gcc)
AR := $(if $(strip $(CROSS_COMPILE)),$(CROSS_COMPILE)ar,ar)

EXTRA_LIB := $(O)/lib/libextra.a
EXTRA_OBJS := $(patsubst $(PROJECT_ROOT)/%.c,$(O)/extra/%.o,$(filter %.c,$(EXTRA_SRCS)))

.PHONY: all extra sdk sdk_only clean allclean help print-vars firmware bootloader

# PWM V2 硬件版本特定参数（仅在 V3_2710IMC_HARDWARE_V2 时传递）
PWM_V2_PARAMS :=
ifneq ($(filter $(V3_HARDWARE_VERSION),$(V3_2710IMC_HARDWARE_V2) $(V3_2710IMC_HARDWARE_V3) $(V3_2710IMC_HARDWARE_V4)),)
PWM_V2_PARAMS += PWM4_IOMUX_INDEX=$(PWM4_IOMUX_INDEX)
PWM_V2_PARAMS += PWM5_IOMUX_INDEX=$(PWM5_IOMUX_INDEX)
endif

all: extra sdk

sdk: $(EXTRA_LIB) $(WEAR_DETECTION_LINK_LIB) | $(BIN_DIR)
	$(MAKE) -C $(SDK_SRCTREE) O=$(O) T=$(SDK_T) CONFIG_CROSS_COMPILE=$(CONFIG_CROSS_COMPILE) \
		C++_ONLY_FLAGS+=" -fno-threadsafe-statics " \
		LIB_LDFLAGS+=" $(EXTRA_LIB) $(WEAR_DETECTION_LINK_LIB) -Wl,-defsym,__EXTRA_LIB_MTIME=$(shell stat -c %Y $(EXTRA_LIB)) " \
		WATCHER_DOG=$(WATCHER_DOG) \
		CRASH_REBOOT=$(CRASH_REBOOT) \
		WERROR_ENABLE=$(WERROR_ENABLE) \
		REL_TRACE_ENABLE=$(REL_TRACE_ENABLE) \
		LITTLEFS_SECTION_SIZE=$(LITTLEFS_SECTION_SIZE) \
		FACTORY_DATA_SECTION_SIZE=$(FACTORY_DATA_SECTION_SIZE) \
		PWM2_IOMUX_INDEX=$(PWM2_IOMUX_INDEX) \
		PWM3_IOMUX_INDEX=$(PWM3_IOMUX_INDEX) \
		$(PWM_V2_PARAMS) \
		RTC_CALENDAR=$(RTC_CALENDAR) \
		PMU_CLK_USE_EXT_CRYSTAL=$(PMU_CLK_USE_EXT_CRYSTAL) \
		CALIB_SLOW_TIMER=$(CALIB_SLOW_TIMER) \
		CFG_CUSTOM_BLE_SERVICE=$(CFG_CUSTOM_BLE_SERVICE) \
		CFG_CUSTOM_BLE_RF_PWR=$(CFG_CUSTOM_BLE_RF_PWR) \
		CFG_OTA_EVENT_CALLBACK=$(CFG_OTA_EVENT_CALLBACK) \
		CFG_CUSTOM_APP=$(CFG_CUSTOM_APP) \
		CFG_POWER_KEY_HW_RESET_ENABLE=$(CFG_POWER_KEY_HW_RESET_ENABLE) \
		LUTE_PICTURE_RESOURCES_SECTION_SIZE=$(LUTE_PICTURE_RESOURCES_SECTION_SIZE) \
		LUTE_RESERVED_SECTION_SIZE=$(LUTE_RESERVED_SECTION_SIZE) \
		FACTORY_MODE=$(FACTORY_MODE)

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
	@echo PMU_CLK_USE_EXT_CRYSTAL=$(PMU_CLK_USE_EXT_CRYSTAL)
	@echo WERROR_ENABLE=$(WERROR_ENABLE)

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
export WERROR_ENABLE
export CHIP_HAS_GRABOX
export GRAPH_MODULE
export VUSB_ON
export PSRAM_ENABLE
export PSRAM_SIZE
export PSRAM_SPEED
export PSRAM_XCCELA_MODE

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
		OTA_CODE_OFFSET=0x66000 \
		OTA_BOOT_INFO_OFFSET=0x62000 \
		PSRAM_ENABLE=$(PSRAM_ENABLE) \
		PSRAM_SIZE=0x800000 \
		PSRAM_SPEED=200 \
		PSRAM_XCCELA_MODE=1 \
		FAST_XRAM_SECTION_SIZE=0x800 \
		NO_WDT_TIMER=1 \
		HWTIMER_NUM=26 \
		LVGL_TEST=1\
		LVGL_BES_PNG=1 \
		LVGL_BES_JPEG=1 \
		LVGL_BES_BAJ=1 \
		LVGL_SNAPSHOT=$(CFG_LVGL_SNAPSHOT_ENABLE) \
		LVGL_NO_TOUCHSCREEN=1 \
		CONFIG_LV_LOG_LEVEL=$(CONFIG_LV_LOG_LEVEL) \
		CONFIG_LV_THEME_DEFAULT_DARK=$(CONFIG_LV_THEME_DEFAULT_DARK) \
		DISPLAY_LCD=st77916 \
		DISPLAY_RST_PIN=$(DISPLAY_RST_PIN) \
		DISPLAY_TE_PIN=$(DISPLAY_TE_PIN) \
		SPI_FLASH1=$(CFG_EXTERNAL_FLASH_ENABLE) \
		ENABLE_FLASH1=$(CFG_EXTERNAL_FLASH_ENABLE) \
		SPI_IOMUX_4WIRE=$(CFG_EXTERNAL_FLASH_ENABLE) \
		BT_CALIB=$(CFG_BT_CALIB) \
		GRAPH_MODULE=1 \
		CHIP_HAS_GRABOX=1 \
		VUSB_ON=$(VUSB_ON) \
		MULTI_HEAP_POISONING=0\
		MULTI_HEAP_POISONING_SLOW=0\
		DEBUG_PORT=1 \
		all

# Bootloader 构建（默认参数）
bootloader:
	@$(MAKE) \
		T=ota_boot_v2 \
		CHIP=best1306p \
		O=$(PROJECT_ROOT)/project/build/bootloader \
		BIN_DIR=$(PROJECT_ROOT)/project/bin/bootloader \
		SYS_AS_MAIN=1 \
		OTA_BOOT_SIZE=0x62000 \
		OTA_BOOT_INFO_OFFSET=0x62000 \
		OTA_CODE_OFFSET=0x66000 \
		OTA_BIN_COMPRESSED=1 \
		FLASH_SIZE=0x400000 \
		OTA_BOOT_OFFSET=0 \
		DEBUG_PORT=1 \
		TRACE_CPU_IMM_OUTPUT=1 \
		TRACE_BUF_SIZE=0 \
		PSRAM_ENABLE=0 \
		PSRAM_SIZE=0x800000 \
		PSRAM_SPEED=0 \
		sdk_only

