cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))
obj-y := $(obj-y:.c=.o)

CFLAGS_PSRAM :=

PSRAM_CLASSIFY ?= XOPI

CFLAGS_PSRAM += -DCHIP_HAS_PSRAM

CFLAGS_PSRAM += -DPSRAM_CTRL_ENABLE

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

ifeq ($(PSRAM_1V2_ENABLE),1)
CFLAGS_PSRAM += -DPSRAM_1V2_ENABLE
endif

ifeq ($(PSRAM_POLLING),1)
CFLAGS_PSRAM += -DPSRAM_POLLING
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

ifeq ($(PSRAMPHY_DUAL_8BIT),1)
CFLAGS_PSRAM += -DPSRAMPHY_DUAL_8BIT
endif

ifneq ($(CFLAGS_PSRAM),)
ccflags-y += $(CFLAGS_PSRAM)
endif

ccflags-y += \
	-Iplatform/drivers/psram/psram/inc \
	-Iplatform/drivers/ana

