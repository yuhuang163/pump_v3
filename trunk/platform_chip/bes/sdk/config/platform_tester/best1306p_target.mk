CHIP		?= best1306p

DEBUG       ?= 1

FPGA        ?= 0

RTOS        ?= 1

NOAPP       ?= 1

LIBC_ROM    ?= 1

KERNEL      ?= RTX5

VERSION_INFO ?= best1306p_tester

export LIBC_OVERRIDE ?= 1

export USER_SECURE_BOOT ?= 0
# enable:1
# disable:0

export OSC_26M_X4_AUD2BB	        ?= 1

export SYS_USE_BBPLL				?= 1

export ULTRA_LOW_POWER		        ?= 1

export POWER_MODE			?= DIG_DCDC

BT_APP  ?= 0

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
#export FLASH_SUSPEND ?= 1
export FLASH_DEEP_SLEEP     ?= 1
export FORCE_FLASH1_SLEEP   ?= 1
export FORCE_FLASH2_SLEEP   ?= 1

export UTILS_ESHELL_EN ?= 0

export ESHELL_ON_TRACE ?= 1

export TRACE_BUF_SIZE ?= 4*1024

export TRACE_BAUD_RATE ?= 8*115200

init-y		:=
core-y		:= tests/besair_platform/ platform/cmsis/ platform/drivers/ platform/hal/ utils/hwtimer_list/
core-y      += utils/crc32/ utils/heap/ utils/cqueue/

ifeq ($(ESHELL_UNITTEST),1)
core-y      += services/audioflinger/
endif

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal -Iservices/audioflinger -Iapps/common -Iapps/audioplayers -Imultimedia/inc

export RTC_CALENDAR ?= 0

CORE_DUMP ?= 0

export PSRAM_ENABLE ?= 0
export PSRAM_SIZE ?= 0x800000
export PSRAM_SPEED ?= 200
export PSRAM_XCCELA_MODE ?= 1

KBUILD_CPPFLAGS += -DSYS_PSRAM_BASE=PSRAM_BASE -DSYS_PSRAM_NC_BASE=PSRAM_NC_BASE

export ALLOW_WARNING ?= 0

export OS_TASKCNT ?= 16
export OS_SCHEDULERSTKSIZE ?= 256
export OS_IDLESTKSIZE ?= 1024
export OS_FIFOSZ ?= 24

LIB_LDFLAGS += -lstdc++ -lsupc++

export ENABLE_PRINT_FLOAT ?= 1

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

ifeq ($(ENABLE_BECO),1)
export NN_LIB ?= 1
KBUILD_CPPFLAGS += -Iinclude/cmsis_dsp -Iinclude/beco -Iinclude/cmsis_nn \
    -DENABLE_BECO -Iinclude/beco/beco_nn
core-y += platform/beco/
endif

ifeq ($(UTILS_ESHELL_EN),1)
export SDMMC0_PULLUP_ENABLE ?= 1
export SPI_IOMUX_4WIRE ?= 1
export SPILCD_IOMUX_4WIRE ?= 1
endif

ifneq ($(NOAPP),0)
core-y += apps/common/
endif

#****LITEOS_M****
ifeq ($(KERNEL), LITEOS_M)
export OSTICK_USE_FAST_TIMER := 0
export OS_HEAP_SIZE ?= 0x20000
KBUILD_CPPFLAGS += -DKERNEL_LITEOS_M -DOS_HEAP_LINK_NAME=m_aucSysMem0 -DOS_HEAP_SIZE=${OS_HEAP_SIZE} -DOS_WRAP_MALLOC
KBUILD_CPPFLAGS += -DLOSCFG_BASE_IPC_MUX_LIMIT=128
ifneq ($(NOAPP),1)
#export USE_OS_MAIN := 1
#KBUILD_CPPFLAGS += -DUSE_OS_MAIN
endif
export LIBC_INIT_AFTER_OS_INIT := 1
ifeq ($(LIBC_INIT_AFTER_OS_INIT),1)
KBUILD_CPPFLAGS += -DLIBC_INIT_AFTER_OS_INIT
endif
export BOARD_OS_WRAP_MALLOC ?= 0
ifeq ($(BOARD_OS_WRAP_MALLOC),1)
KBUILD_CPPFLAGS += -DOHOS_WRAP_MALLOC
else
LDFLAGS_IMAGE += --wrap _malloc_r --wrap _calloc_r --wrap _free_r --wrap _realloc_r
endif
KBUILD_CPPFLAGS += -Irtos/liteos/liteos_m/
KBUILD_CPPFLAGS += -Irtos/liteos/liteos_m/utils/
KBUILD_CPPFLAGS += -Irtos/liteos/liteos_m/kernel/include/
endif
#****LITEOS_M****
