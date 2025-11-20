cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y := -Iplatform/hal

ifeq ($(SECURE_BOOT),1)
ifneq ($(CHIP_HAS_SECURE_BOOT),1)
$(error CHIP=$(CHIP) does NOT support secure boot!)
endif
ccflags-y += -DSECURE_BOOT
ifneq ($(DEFAULT_NORFLASH_SAMDLY),)
ccflags-y += -DDEFAULT_NORFLASH_SAMDLY=$(DEFAULT_NORFLASH_SAMDLY)
endif
ifneq ($(DEFAULT_NORFLASH_MOD_CLK),)
ccflags-y += -DDEFAULT_NORFLASH_MOD_CLK=$(DEFAULT_NORFLASH_MOD_CLK)
endif
ifneq ($(DEFAULT_NORFLASH_RDCMD),)
ccflags-y += -DDEFAULT_NORFLASH_RDCMD=$(DEFAULT_NORFLASH_RDCMD)
endif
endif

ifneq ($(SECURE_BOOT_CODE_VER),)
ccflags-y += -DSECURE_BOOT_CODE_VER=$(SECURE_BOOT_CODE_VER)
endif
