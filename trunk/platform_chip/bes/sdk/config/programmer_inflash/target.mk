CHIP		?= best1700

DEBUG		?= 1

NOSTD		?= 1

LIBC_ROM	?= 1

PROGRAMMER	:= 1

FAULT_DUMP	?= 0

WATCHER_DOG ?= 1

init-y		:=
core-y		:= tests/programmer/ platform/cmsis/ platform/hal/
core-y      += tests/programmer_inflash/

ifeq ($(OTA_BIN_COMPRESSED),1)
core-y		+= utils/lzma/
core-y		+= utils/heap/

KBUILD_CPPFLAGS += -Iutils/lzma/
KBUILD_CPPFLAGS += -Iutils/heap/

export OTA_REBOOT_FLASH_REMAP := 0
endif

LDS_FILE	:= programmer_inflash.lds

export CRC32_ROM ?= 1

export PROGRAMMER_INFLASH := 1

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal

KBUILD_CFLAGS +=

LIB_LDFLAGS +=

CFLAGS_IMAGE +=

LDFLAGS_IMAGE +=

OTA_ENABLE ?= 1

ULTRA_LOW_POWER ?= 1

ifeq ($(CHIP),best1501)
export SINGLE_WIRE_UART_PORT ?= 2
endif

ifneq ($(filter best1501 best1600,$(CHIP)),)
export OTA_REBOOT_FLASH_REMAP ?= 1
else
export OTA_REBOOT_FLASH_REMAP ?= 0
endif

ifeq ($(OTA_BOOT_TO_DUAL_CHIP),1)
FLASH_DUAL_CHIP := 1
ifneq ($(FLASH_DUAL_CHIP),1)
$(error "OTA_BOOT_TO_DUAL_CHIP requires FLASH_DUAL_CHIP")
endif
ifeq ($(FLASH_DUAL_CHIP_INIT_ON),1)
$(error "OTA_BOOT_TO_DUAL_CHIP conflicts with FLASH_DUAL_CHIP_INIT_ON")
endif
export BOOT_CODE_IN_RAM ?= 1
export TRACE_CODE_IN_RAM ?= 1
OTA_CODE_OFFSET ?= 0x40000
endif

# change the flash size to 0x1000000 if 1600 uses 16MB single flash die
export FLASH_SIZE ?= 0x800000

export USE_MULTI_FLASH ?= 0
ifeq ($(USE_MULTI_FLASH),1)
export BTH_USE_SYS_FLASH ?= 1
export BTH_USE_SYS_PERIPH ?= 1
export FLASH1_SIZE ?= 0x800000
KBUILD_CPPFLAGS += \
        -DUSE_MULTI_FLASH \
        -DFLASH1_SIZE=$(FLASH1_SIZE)
LDS_CPPFLAGS += \
        -DUSE_MULTI_FLASH \
        -DFLASH1_SIZE=$(FLASH1_SIZE)
endif

export DOWNLOAD_UART_BAUD_RATE ?= 921600

ifneq ($(filter best1400 best2300 best2300p best2001,$(CHIP)),)
OTA_BOOT_SIZE ?= 0x10000
OTA_CODE_OFFSET ?= 0x18000
else
OTA_BOOT_SIZE ?= 0x18000
OTA_CODE_OFFSET ?= 0x20000
endif

ifeq ($(OTA_BOOT_INFO_OFFSET),)
ifneq ($(OTA_BOOT_OFFSET),)
OTA_BOOT_INFO_OFFSET ?= $(OTA_BOOT_OFFSET)+$(OTA_BOOT_SIZE)
else
OTA_BOOT_INFO_OFFSET ?= $(OTA_BOOT_SIZE)
endif
ifeq ($(OTA_BOOT_TO_DUAL_CHIP),1)
# Avoid using (...)*2, for excape needed on Linux but not on Win
OTA_BOOT_INFO_OFFSET := $(OTA_BOOT_INFO_OFFSET)+$(OTA_BOOT_INFO_OFFSET)
endif
endif

ifneq ($(filter best1600 best1603 best1700,$(CHIP)),)
ifneq ($(SYS_AS_MAIN),1)
CHIP_SUBSYS ?= bth
export BTH_AS_MAIN_MCU ?= 1
ifeq ($(CHIP),best1600)
BTH_USE_SYS_FLASH ?= 1
endif
endif

# For ISPI iomux and DMA config
export PROGRAMMER_HAL_FULL_INIT ?= 1
ifeq ($(OTA_BIN_COMPRESSED),1)
KBUILD_CPPFLAGS += -DBTH_RAM_SIZE=0xE0000
endif   #OTA_BIN_COMPRESSED
endif

export SINGLE_WIRE_DOWNLOAD ?= 1
export UNCONDITIONAL_ENTER_SINGLE_WIRE_DOWNLOAD ?= 1

export PROGRAMMER_WATCHDOG ?= 1
export FLASH_UNIQUE_ID ?= 1
export TRACE_BAUD_RATE := 10*115200

ifeq ($(OTA_REBOOT_FLASH_REMAP),1)
KBUILD_CPPFLAGS += -DOTA_REBOOT_FLASH_REMAP
endif

ifeq ($(USER_SECURE_BOOT),1)
export USER_SECURE_BOOT_JUMP_ENTRY_ADDR ?= $(OTA_CODE_OFFSET)
endif
