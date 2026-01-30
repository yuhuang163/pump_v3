cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y := -I$(obj)/inc -Itests/programmer/inc -Iutils/boot_struct

CFLAGS_nandflash_drv.o += -DCHIP_FLASH_CTRL_VER=$(CHIP_FLASH_CTRL_VER)

ifneq ($(FLASH_SRC_FREQ_MHZ),)
CFLAGS_nandflash_drv.o += -DFLASH_SRC_FREQ_MHZ=$(FLASH_SRC_FREQ_MHZ)
endif
ifneq ($(FLASH_SPEED_MHZ),)
CFLAGS_nandflash_drv.o += -DFLASH_SPEED_MHZ=$(FLASH_SPEED_MHZ)
endif
ifeq ($(FLASH_ULTRA_LOW_SPEED),1)
CFLAGS_nandflash_drv.o += -DFLASH_ULTRA_LOW_SPEED
endif
ifeq ($(FLASH_LOW_SPEED),1)
CFLAGS_nandflash_drv.o += -DFLASH_LOW_SPEED
endif
ifeq ($(FLASH_HIGH_SPEED),1)
CFLAGS_nandflash_drv.o += -DFLASH_HIGH_SPEED
endif
ifeq ($(FLASH_CLK_DIV_1),1)
CFLAGS_nandflash_drv.o += -DFLASH_CLK_DIV_1
endif
ifeq ($(OSC_26M_X4_AUD2BB),1)
CFLAGS_nandflash_drv.o += -DOSC_26M_X4_AUD2BB
endif

ifeq ($(FLASH_RESET_PIN),1)
CFLAGS_nandflash_drv.o += -DFLASH_RESET_PIN
endif
ifeq ($(FLASH_DTR),1)
ccflags-y += -DFLASH_DTR
endif
ifneq ($(FLASH_DTR_CALIB_BLOCK),)
CFLAGS_nandflash_drv.o += -DFLASH_DTR_CALIB_BLOCK=$(FLASH_DTR_CALIB_BLOCK)
endif
ifneq ($(FLASH_DTR_CALIB_PAGE),)
CFLAGS_nandflash_drv.o += -DFLASH_DTR_CALIB_PAGE=$(FLASH_DTR_CALIB_PAGE)
endif

ifeq ($(FLASH_DMA),1)
CFLAGS_nandflash_drv.o += -DFLASH_DMA
endif
ifeq ($(FLASH_CALIB_DEBUG),1)
CFLAGS_nandflash_drv.o += -DFLASH_CALIB_DEBUG
endif
ifeq ($(FLASH_SECURITY_REGISTER),1)
CFLAGS_nandflash_drv.o += -DFLASH_SECURITY_REGISTER
endif

ifneq ($(NANDFLASH_NC_BASE),)
CFLAGS_nandflash_drv.o +=-DNANDFLASH_NC_BASE=$(NANDFLASH_NC_BASE)
endif

ifeq ($(FLASH_DEEP_SLEEP),1)
CFLAGS_nandflash_drv.o += -DFLASH_DEEP_SLEEP
endif

ifeq ($(NANDFLASH_DELAY_WAKEUP),1)
CFLAGS_nandflash_drv.o += -DNANDFLASH_DELAY_WAKEUP
endif

ifeq ($(NANDFLASH_CHIP_LIST),)
NANDFLASH_CHIP_LIST := ALL
endif
ifeq ($(NANDFLASH_CHIP_LIST),)
$(error *** Error: NANDFLASH_CHIP_LIST is NOT defined!)
endif

CFLAGS_nandflash_drv.o += $(foreach flash,$(NANDFLASH_CHIP_LIST),-D__NANDFLASH_$(flash)__)
