CHIP		?= best1502x

DEBUG       ?= 1

FPGA        ?= 0

RTOS        ?= 1

LIBC_ROM    ?= 1

KERNEL      ?= RTX5

VERSION_INFO ?= best1502x_tester

export LIBC_OVERRIDE ?= 1

export USER_SECURE_BOOT ?= 0
# enable:1
# disable:0

export OSC_26M_X4_AUD2BB	        ?= 1

export SYS_USE_BBPLL				?= 1

export ULTRA_LOW_POWER		        ?= 1

export POWER_MODE			?= DIG_DCDC

BT_APP  ?= 1

DEBUG_PORT  ?= 1
# 0: usb
# 1: uart0
# 2: uart1

FLASH_CHIP	?= ALL
# GD25Q80C
# GD25Q32C
# ALL

export CURRENT_TEST ?= 0

ifeq ($(CURRENT_TEST),1)
export SMALL_RET_RAM ?= 1
export CORE_SLEEP_POWER_DOWN 		?= 1
#INTSRAM_RUN ?= 1
endif
ifeq ($(INTSRAM_RUN),1)
LDS_FILE ?= best1000_intsram.lds
else
LDS_FILE ?= best1000.lds
endif

KBUILD_CPPFLAGS += -D__SYS_AS_MAIN__

export FLASH_SIZE ?= 0x400000
export FLASH_SUSPEND ?= 1

export UTILS_ESHELL_EN ?= 0

export ESHELL_ON_TRACE ?= 1

export TRACE_BUF_SIZE ?= 4*1024

export TRACE_BAUD_RATE ?= 8*115200

init-y		:=
core-y		:= tests/besair_platform/ platform/cmsis/ platform/drivers/ platform/hal/ utils/hwtimer_list/
core-y      += utils/crc32/ utils/heap/ utils/cqueue/

ifeq ($(FATFS_ENABLE),1)
KBUILD_CPPFLAGS += -DFS_FATFS_ENABLE
core-y      += services/fs/mbed/fat/ services/fs/fatfs/
core-y      += services/fs/block/
endif

ifeq ($(LITTLEFS_ENABLE),1)
KBUILD_CPPFLAGS += -DFS_LITTLEFS_ENABLE
core-y      += services/fs/mbed/littlefs/ services/fs/littlefs/
core-y      += services/fs/block/
endif

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal -Iservices/audioflinger -Iapps/common -Iapps/audioplayers -Imultimedia/inc/audio/codec/sbc/inc

export RTC_CALENDAR ?= 0

CORE_DUMP ?= 0

export PSRAM_ENABLE ?=1
export PSRAM_SIZE ?= 0x800000
export PSRAM_SPEED ?= 200
export PSRAM_XCCELA_MODE ?= 1

KBUILD_CPPFLAGS += -DSYS_PSRAM_BASE=PSRAM_BASE -DSYS_PSRAM_NC_BASE=PSRAM_NC_BASE

export USE_DSI_PLL ?= 1
export DSIPLL_MHZ ?= 400

export ALLOW_WARNING ?= 0

export OS_TASKCNT ?= 16
export OS_SCHEDULERSTKSIZE ?= 256
export OS_IDLESTKSIZE ?= 1024
export OS_FIFOSZ ?= 24

LIB_LDFLAGS += -lstdc++ -lsupc++

export ENABLE_PRINT_FLOAT ?= 1

ifeq ($(OPENAMP_ENABLE),1)
export RPMSG_ROLE_MASTER 	?= 1
export RPTUN_CPU2SENS    	?= 1

core-y += services/openamp/
endif

export TEST_RGB565 ?= 0
ifeq ($(TEST_RGB565),1)
KBUILD_CPPFLAGS += -DTEST_RGB565
endif

export ENABLE_LVGL ?= 0
ifeq ($(TEST_HAL_PATTERN),TEST_LVGL)
export ENABLE_LVGL := 1
export I2C2_IOMUX_INDEX := 56
endif
ifeq ($(ENABLE_LVGL),1)
export LVGL_GPU_VGLITE ?= 1
export LVGL_GPU_ARM2D ?= 0
export LVGL_TP_TMA525C ?= 1
core-y += thirdparty/lvgl/
ifeq ($(LVGL_GPU_ARM2D),1)
export ENABLE_ARM2D := 1
endif
endif

ifeq ($(TEST_HAL_PATTERN),TEST_TMA525C)
export I2C2_IOMUX_INDEX := 56
endif

export ENABLE_ARM2D ?= 0
ifeq ($(TEST_HAL_PATTERN),TEST_ARM2D)
export ENABLE_ARM2D := 1
endif
ifeq ($(ENABLE_ARM2D),1)
core-y += thirdparty/arm2d/
core-y += platform/cmsis/DSP_Lib/
endif

KBUILD_CPPFLAGS += -DSTACK_SECTION_SIZE=0x8000 -DHEAP_SECTION_SIZE=0x8000

ifneq ($(ENABLE_BECO),1)
export NN_LIB ?= 1
KBUILD_CPPFLAGS += -Iinclude/cmsis_dsp -Iinclude/beco -Iinclude/cmsis_dsp
core-y += platform/beco/
endif
