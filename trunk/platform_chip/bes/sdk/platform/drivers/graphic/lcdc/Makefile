
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o)

ccflags-y += \
    -Iplatform/drivers/ana

ifeq ($(CHIP_HAS_MIP),1)
ccflags-y += -DCHIP_HAS_MIP
endif
