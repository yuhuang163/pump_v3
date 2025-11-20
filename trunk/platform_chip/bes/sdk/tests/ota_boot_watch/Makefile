cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s += ../../platform/main/startup_main.S
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

BOOT_WATCH_LIB_NAME := lib_boot_watch
$(BOOT_WATCH_LIB_NAME)-y := $(obj_c:.c=.o) $(obj_cpp:.cpp=.o)
obj-y += $(obj_s:.S=.o) $(BOOT_WATCH_LIB_NAME).a

obj-y += ../../utils/crc32/
obj-y += ../../utils/mbedtls/
export MBEDTLS_CONFIG_FILE := config-boot-watch.h

# Startup config
ifeq ($(BOOT_LOADER_ENTRY_HOOK),1)
asflags-y += -DBOOT_LOADER_ENTRY_HOOK
endif
ifeq ($(BOOT_LOADER_PRE_INIT_HOOK),1)
asflags-y += -DBOOT_LOADER_PRE_INIT_HOOK
ccflags-y += -DBOOT_LOADER_PRE_INIT_HOOK
endif
ifeq ($(BOOT_LOADER_POST_INIT_HOOK),1)
asflags-y += -DBOOT_LOADER_POST_INIT_HOOK
endif
ifeq ($(NO_NVIC_INIT),1)
asflags-y += -DNO_NVIC_INIT
endif
ifeq ($(NO_BOOT_INIT),1)
asflags-y += -DNO_BOOT_INIT
endif
ifeq ($(NO_SYSTEM_INIT),1)
asflags-y += -DNO_SYSTEM_INIT
endif

ifeq ($(CHIP_HAS_PSRAM)-$(PSRAM_ENABLE),1-1)
obj-y += ../../platform/drivers/psram/
endif

ifeq ($(DEFAULT_BSS_IN_PSRAM),1)
subdir-ccflags-y += -DDEFAULT_BSS_IN_PSRAM
endif

ifeq ($(RTOS),1)
ifeq ($(KERNEL),RTX)
ccflags-y += -Irtos/rtx/TARGET_CORTEX_M
endif
endif

ifeq ($(DEBUG_MODE_USB_DOWNLOAD),1)
ccflags-y += -DDEBUG_MODE_USB_DOWNLOAD
endif

ifeq ($(USER_SECURE_BOOT),1)
ccflags-y += -Iutils/user_secure_boot
endif

ifeq ($(PSRAM_INIT_CHECK),1)
ccflags-y += -DPSRAM_INIT_CHECK
endif

CFLAGS_ota_boot_watch.o += \
	-DOTA_BOOT_OFFSET=$(OTA_BOOT_OFFSET) \
	-DOTA_BOOT_SIZE=$(OTA_BOOT_SIZE) \
	-DOTA_CODE_OFFSET=$(OTA_CODE_OFFSET) \
	-DOTA_REMAP_OFFSET=$(OTA_REMAP_OFFSET)

ifeq ($(OTA_BARE_BOOT),1)
CFLAGS_ota_boot_watch.o += -DOTA_BARE_BOOT
endif

ifneq ($(DEBUG_PORT),)
CFLAGS_ota_boot_watch.o += -DDEBUG_PORT=$(DEBUG_PORT)
endif

ifeq ($(FLASH_DEC_ENABLE),1)
CFLAGS_ota_boot_watch.o += -DFLASH_DEC_ENABLE
endif

ifneq ($(FLASH_DEC_OFFSET),)
CFLAGS_ota_boot_watch.o += -DFLASH_DEC_OFFSET=$(FLASH_DEC_OFFSET)
endif # FLASH_DEC_OFFSET

ifneq ($(FLASH_DEC_SIZE),)
CFLAGS_ota_boot_watch.o += -DFLASH_DEC_SIZE=$(FLASH_DEC_SIZE)
endif # FLASH_DEC_SIZE

ifeq ($(FLASH_DEC_ENABLE),1)
ifeq ($(FLASH_DEC_OFFSET),)
$(error FLASH_DEC_OFFSET should be set along with FLASH_DEC_ENABLE)
endif

ifeq ($(FLASH_DEC_SIZE),)
$(error FLASH_DEC_SIZE should be set along with FLASH_DEC_ENABLE)
endif
endif # FLASH_DEC_ENABLE

ifeq ($(FLASH_CRYPTO_ENABLE),1)
CFLAGS_ota_boot_watch.o += -DFLASH_CRYPTO_ENABLE

ifneq ($(FLASH_CRYPTO_ADDR0),)
CFLAGS_ota_boot_watch.o += -DFLASH_CRYPTO_ADDR0=$(FLASH_CRYPTO_ADDR0)
endif

ifneq ($(FLASH_CRYPTO_SIZE0),)
CFLAGS_ota_boot_watch.o += -DFLASH_CRYPTO_SIZE0=$(FLASH_CRYPTO_SIZE0)
endif
endif # FLASH_CRYPTO_ENABLE

ifeq ($(OTA_USE_DIFF),1)
ccflags-y += -Iutils/bsdiff
endif

ifeq ($(OTA_BIN_COMPRESSED),1)
ccflags-y +=  \
	-Iutils/lzma \
	-Iutils/lzma/C \
	-Iutils/heap
endif

ccflags-y +=  \
	-Iutils/boot_struct \
	-Iutils/crc32   \
	-Iinclude/watchdog \
	-Iplatform/drivers/ana \
	-Iservices/fs \
	-Iutils/mbedtls/include \
	-Iutils/hwtimer_list
