cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_cpp:.cpp=.o) $(obj_c:.c=.o)

ifeq ($(EMMC_USE_PLL),1)
subdir-ccflags-y += -DEMMC_USE_PLL
endif

ifeq ($(EMMC_HS_MODE),1)
subdir-ccflags-y += -DEMMC_HS_MODE
endif

subdir-ccflags-y += \
    -Iapps/common \
    -Iapps/app_eshell \
    -Iutils/crc32 \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/ana/$(CHIP) \
    -Iplatform/hal \
    -Imbed/api

ifeq ($(CHIP_HAS_EMMC),1)
subdir-ccflags-y += -DCHIP_HAS_EMMC
subdir-ccflags-y += \
    -Iplatform/drivers/emmc/host/inc \
    -Iplatform/drivers/emmc/host/core \
    -Iplatform/drivers/emmc/host/hci \
    -Iplatform/drivers/emmc/host/card
endif

ifeq ($(KERNEL),RHINO)
subdir-ccflags-y += -Irtos/rhino/$(RHINO_VER)/components/debug/
endif
