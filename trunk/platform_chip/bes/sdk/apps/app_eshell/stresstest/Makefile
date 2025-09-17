cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o)

subdir-ccflags-y += \
    -Iapps/app_eshell \
    -Iplatform/hal \
    -Iplatform/drivers/ana

ifeq ($(CHIP_HAS_EMMC),1)
subdir-ccflags-y += -DCHIP_HAS_EMMC
subdir-ccflags-y += \
    -Iplatform/drivers/emmc/host/inc \
    -Iplatform/drivers/emmc/host/card \
    -Iplatform/drivers/emmc/host/core \
    -Iplatform/drivers/emmc/host/hci
endif
