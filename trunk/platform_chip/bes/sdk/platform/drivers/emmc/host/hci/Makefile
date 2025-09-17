cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
	-I$(EMMC_REL_TOPDIR)/inc \
	-I$(EMMC_REL_TOPDIR)/card \
	-I$(EMMC_REL_TOPDIR)/core \
	-I$(EMMC_REL_TOPDIR)/host \
	-I$(EMMC_REL_TOPDIR)/hci

ifeq ($(CHIP),best1600)
	ccflags-y += -Iplatform/drivers/ana
endif

ifneq ($(EMMC_CFLAGS),)
ccflags-y += $(EMMC_CFLAGS)
endif
