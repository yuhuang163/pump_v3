cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o)

ifeq ($(SENSOR_HUB),1)
ccflags-y += -DSENSOR_HUB
endif

subdir-ccflags-y += \
    -Iapps/app_eshell \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/ana/$(CHIP) \
    -Iplatform/hal \
    -Itests/besair_platform/bsp_tester/coremark \
    -Iutils/boot_struct

