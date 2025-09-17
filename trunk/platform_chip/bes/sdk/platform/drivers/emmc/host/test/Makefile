cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ifeq ($(EMMC_RPMB_ENABLE),1)
obj-y += ../../../../../utils/mbedtls/
endif

ccflags-y += \
	-I$(EMMC_REL_TOPDIR)/inc \
	-I$(EMMC_REL_TOPDIR)/card \
	-I$(EMMC_REL_TOPDIR)/core \
	-I$(EMMC_REL_TOPDIR)/host \
	-I$(EMMC_REL_TOPDIR)/hci \

ifneq ($(EMMC_CFLAGS),)
ccflags-y += $(EMMC_CFLAGS)
endif

ifeq ($(EMMC_SLEEP_ENABLE),1)
CFLAGS_emmc_card_test.o += -DEMMC_SLEEP_ENABLE
endif
