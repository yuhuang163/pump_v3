cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

PROGRAMMER_EXT_LIB_NAME := libprogrammer_ext
$(PROGRAMMER_EXT_LIB_NAME)-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)
obj-y += $(PROGRAMMER_EXT_LIB_NAME).a

ccflags-y += \
    -Iplatform/drivers/ana \
    -Itests/programmer/inc \
    -Iutils/boot_struct \
	-Iutils/crc32 \
	-Iutils/crc16 \
    -Iutils/rom_utils

ccflags-y += -DOTA_CODE_OFFSET=$(OTA_CODE_OFFSET)

ifeq ($(SINGLE_WIRE_DOWNLOAD),1)
ccflags-y += -DSINGLE_WIRE_DOWNLOAD
endif

ifeq ($(PROGRAMMER_WATCHDOG),1)
ccflags-y += -DPROGRAMMER_WATCHDOG
endif

ifeq ($(FORCE_COPY_NEW_IMAGE),1)
ccflags-y += -DFORCE_COPY_NEW_IMAGE
endif

ifeq ($(OTA_BOOT_TO_DUAL_CHIP),1)
ccflags-y += -DOTA_BOOT_TO_DUAL_CHIP
endif

ifeq ($(OTA_BOOT_REMAP_SWAP),1)
ccflags-y += -DOTA_BOOT_REMAP_SWAP
endif

ifeq ($(OTA_BOOT_DEBUG),1)
ccflags-y += -DOTA_BOOT_DEBUG
endif
