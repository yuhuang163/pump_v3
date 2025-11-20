cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(filter 6,$(CHIP_PSRAM_CTRL_VER)),)
obj-y += psram_dummy.o

$(obj)/psram_dummy.o :
	$(call if_changed,cc_dummy)

else
obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))
obj-y := $(obj-y:.c=.o)

CFLAGS_PSRAM :=

PSRAM_CLASSIFY ?= XOPI

CFLAGS_PSRAM += -DCHIP_HAS_PSRAM

CFLAGS_PSRAM += -DCHIP_PSRAM_CTRL_VER=$(CHIP_PSRAM_CTRL_VER)

ifneq ($(filter-out OPI XOPI HYPERBUS UHS ULS, $(PSRAM_CLASSIFY)),)
$(error Invalid PSRAM_CLASSIFY=$(PSRAM_CLASSIFY))
endif

CFLAGS_PSRAM += -DPSRAM_CLASSIFY_$(PSRAM_CLASSIFY)

ifneq ($(PSRAM_SPEED),)
CFLAGS_PSRAM += -DPSRAM_SPEED=$(PSRAM_SPEED)
endif

ifeq ($(FPGA),1)
CFLAGS_PSRAM += -DFPGA
endif

ifeq ($(NO_ISPI),1)
CFLAGS_PSRAM += -DNO_ISPI
endif

ifeq ($(PSRAM_DEBUG),1)
CFLAGS_PSRAM += -DPSRAM_DEBUG
endif

ifeq ($(PSRAM_DUAL_8BIT),1)
CFLAGS_PSRAM += -DPSRAM_DUAL_8BIT
else ifeq ($(PSRAM_X16_MODE),1)
CFLAGS_PSRAM += -DPSRAM_X16_MODE
else ifeq ($(PSRAM_X8_MODE),1)
CFLAGS_PSRAM += -DPSRAM_X8_MODE
else ifeq ($(PSRAM_2X16_MODE),1)
CFLAGS_PSRAM += -DPSRAM_2X16_MODE
endif

ifeq ($(PSRAM_WINDOW_TEST),1)
CFLAGS_PSRAM += -DPSRAM_WINDOW_TEST
endif

ifneq ($(CFLAGS_PSRAM),)
ccflags-y += $(CFLAGS_PSRAM)
endif

ccflags-y += \
	-Iplatform/drivers/psram/psram_best1700/inc \
	-Iplatform/drivers/ana

endif
