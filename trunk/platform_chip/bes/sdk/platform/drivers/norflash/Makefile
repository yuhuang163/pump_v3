cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ifneq ($(LTO_CFLAGS),)
KBUILD_CFLAGS := $(filter-out $(LTO_CFLAGS),$(KBUILD_CFLAGS))
KBUILD_CPPFLAGS := $(filter-out $(LTO_CPPFLAGS),$(KBUILD_CPPFLAGS))
endif

ccflags-y := -I$(obj)/inc -Itests/programmer/inc -Iutils/boot_struct

CFLAGS_norflash_drv.o += -DCHIP_FLASH_CTRL_VER=$(CHIP_FLASH_CTRL_VER)

ifeq ($(SIMU),1)
FLASH_SAMP_DELAY_INDEX ?= 1
endif
ifneq ($(FLASH_SAMP_DELAY_INDEX),)
CFLAGS_norflash_drv.o += -DFLASH_SAMP_DELAY_INDEX=$(FLASH_SAMP_DELAY_INDEX)
endif

ifeq ($(FLASH_HPM),1)
ccflags-y += -DFLASH_HPM
endif

ifeq ($(FLASH_CALIB_DEBUG),1)
CFLAGS_norflash_drv.o += -DFLASH_CALIB_DEBUG
endif
ifeq ($(FLASH_CALIB_MAGIC_WORD),1)
CFLAGS_norflash_drv.o += -DFLASH_CALIB_MAGIC_WORD
endif
ifeq ($(FLASH1_CALIB_MAGIC_WORD),1)
CFLAGS_norflash_drv.o += -DFLASH1_CALIB_MAGIC_WORD
endif
ifeq ($(FLASH2_CALIB_MAGIC_WORD),1)
CFLAGS_norflash_drv.o += -DFLASH2_CALIB_MAGIC_WORD
endif
ifeq ($(FLASH_CALIB_SEQ_PATTERN),1)
CFLAGS_norflash_drv.o += -DFLASH_CALIB_SEQ_PATTERN
endif
ifneq ($(FLASH0_CALIB_SEQ_ADDR),)
CFLAGS_norflash_drv.o += -DFLASH0_CALIB_SEQ_ADDR=$(FLASH0_CALIB_SEQ_ADDR)
endif
ifneq ($(FLASH1_CALIB_SEQ_ADDR),)
CFLAGS_norflash_drv.o += -DFLASH1_CALIB_SEQ_ADDR=$(FLASH1_CALIB_SEQ_ADDR)
endif
ifneq ($(FLASH2_CALIB_SEQ_ADDR),)
CFLAGS_norflash_drv.o += -DFLASH2_CALIB_SEQ_ADDR=$(FLASH2_CALIB_SEQ_ADDR)
endif
ifeq ($(FLASH_RX_CLOCK_DELAY),1)
CFLAGS_norflash_drv.o += -DFLASH_RX_CLOCK_DELAY
ifeq ($(FLASH_RX_CLOCK_DELAY_ITERATE),1)
CFLAGS_norflash_drv.o += -DFLASH_RX_CLOCK_DELAY_ITERATE
endif
endif

ifeq ($(FLASH_SUSPEND),1)
CFLAGS_norflash_drv.o += -DFLASH_SUSPEND
endif
ifeq ($(FLASH_DTR),1)
ccflags-y += -DFLASH_DTR
endif
ifeq ($(FLASH_SECURITY_REGISTER),1)
CFLAGS_norflash_drv.o += -DFLASH_SECURITY_REGISTER
ifeq ($(FLASH_SEC_REG_DUAL_CHIP),1)
CFLAGS_norflash_drv.o += -DFLASH_SEC_REG_DUAL_CHIP
endif
endif
ifeq ($(FLASH_LOCK_MEM_READ),1)
CFLAGS_norflash_drv.o += -DFLASH_LOCK_MEM_READ
endif
ifeq ($(FLASH_FETCH_64B),1)
CFLAGS_norflash_drv.o += -DFLASH_FETCH_64B
endif
ifeq ($(PUYA_FLASH_ERASE_PAGE_ENABLE),1)
CFLAGS_norflash_drv.o += -DPUYA_FLASH_ERASE_PAGE_ENABLE
endif
ifeq ($(FLASH_DEC_ENABLE), 1)
CFLAGS_norflash_drv.o += -DFLASH_DEC_ENABLE
endif

ifeq ($(FLASH_PAGE_MPM_ENABLE),1)
ccflags-y += -DFLASH_PAGE_MPM_ENABLE
endif

ifeq ($(FLASH_CONT_MODE_SK25LE032),1)
ccflags-y += -DFLASH_CONT_MODE_SK25LE032
endif

ifeq ($(FLASH_CHIP_LIST),)
$(error *** Error: FLASH_CHIP_LIST is NOT defined!)
endif

ccflags-y += $(foreach flash,$(FLASH_CHIP_LIST),-D__NORFLASH_$(flash)__)
