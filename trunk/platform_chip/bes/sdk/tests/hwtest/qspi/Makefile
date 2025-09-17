cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y := -DCHIP_FLASH_CTRL_VER=$(CHIP_FLASH_CTRL_VER) -DQSPI_DISPLAY_TEST
obj-y += ../../../platform/drivers/qspi_1501/

ifeq ($(BAREMETAL), 1)
ccflags-y += -D_BAREMETAL=1
endif



ifeq ($(TEST_WITH_LCD), 1)
ccflags-y += -DTEST_WITH_LCD
endif

ccflags-y += \
             -Itests/hwtest/ \
             -Iutils/hwtimer_list/ \
             -Iplatform/drivers/qspi_1501/ \
             -Iplatform/drivers/graphic/gpu/ \


