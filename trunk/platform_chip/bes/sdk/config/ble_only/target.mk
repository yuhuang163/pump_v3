CHIP		?= best1503

DEBUG		?= 1

FPGA		?= 0

RTOS		?= 1

FLASH_CHIP	?= ALL
# GD25Q80C
# GD25Q32C
#GD25LQ32C
#EN25S80B
# ALL
export HW_AGC ?= 1

export LIBC_OVERRIDE ?= 1

export OSC_26M_X4_AUD2BB	        ?= 1

export ULTRA_LOW_POWER		        ?= 1
ifneq ($(CHIP), best1306p)
export CORE_SLEEP_POWER_DOWN ?= 1
export BT_LOG_POWEROFF ?= 1
endif
export FLASH_DEEP_SLEEP ?= 1
export SPLIT_CORE_M_CORE_L ?= 0
export ALLOW_WARNING ?= 1
export PSRAM_ENABLE ?=0
export PSRAM_SPEED ?= 0
export POWER_MODE			?= DIG_DCDC

DEBUG_PORT  ?= 1

export FLASH_SIZE ?= 0x400000
export FLASH_SUSPEND ?= 1
export FAULT_DUMP := 0

export NO_OVERLAY := 1

export TRACE_BAUD_RATE := 8*115200
export TRACE_BUF_SIZE ?= 4*1024

init-y		:=
core-y		:= platform/main/ platform/cmsis/ platform/drivers/ platform/hal/

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal -Iplatform/drivers/bt -Iapps/common

LDS_FILE ?= best1000.lds

export OTA_BOOT_SIZE ?= 0x62000
export OTA_BOOT_INFO_OFFSET ?= $(OTA_BOOT_SIZE)
export OTA_CODE_OFFSET ?= $(OTA_BOOT_INFO_OFFSET)+0x2000


BT_APP :=1
export BTH_SUBSYS_ONLY := 1
BLE_STACK_NEW_DESIGN := 1
FULL_APP_PROJECT := 1
export BOOT_LOADER_PRE_INIT_HOOK := 1
export BLE_CONNECTION_MAX := 1
export BLE_ONLY_ENABLED := 1
ifeq ($(BLE_ONLY_ENABLED),1)
INTERSYS_DEBUG := 1
core-y += services/ apps/common/ utils/intersyshci/
endif

export NO_CP := 1

export USE_NORFLASH_API ?= 1
ifeq ($(USE_NORFLASH_API), 1)
KBUILD_CPPFLAGS += -DUSE_NORFLASH_API
# core-y += services/norflash_api/
# core-y += services/nv_section/log_section/

export USER_DATA_TO_FLASH ?= 0
ifeq ($(USER_DATA_TO_FLASH), 1)
export USER_DATA_SECTION_SIZE ?= 4*1024
endif # USER_DATA_TO_FLASH
endif # USE_NORFLASH_API
