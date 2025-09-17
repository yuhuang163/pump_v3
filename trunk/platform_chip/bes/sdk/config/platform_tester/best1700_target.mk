CHIP		?= best1700

DEBUG       ?= 1

FPGA        ?= 0

RTOS        ?= 1

LIBC_ROM    ?= 1

KERNEL      ?= RTX5

VERSION_INFO ?= best1700_tester

NO_CP := 1

export LIBC_OVERRIDE ?= 1


export USER_SECURE_BOOT ?= 0
# enable:1
# disable:0

export OSC_26M_X4_AUD2BB	        ?= 1

export ULTRA_LOW_POWER		        ?= 1

export POWER_MODE			?= DIG_DCDC

DEBUG_PORT  ?= 1
# 0: usb
# 1: uart0
# 2: uart1

FLASH_CHIP	?= ALL
# GD25Q80C
# GD25Q32C
# ALL


ifeq ($(CHIP),best1700)
include $(srctree)/config/best1700/mem_cfg.mk
endif

DSP_TEST ?= 0

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

export FLASH_SIZE ?= 0x800000
export FLASH_SUSPEND ?= 1

ifeq ($(DSP_TEST),1)
ifneq ($(DSP_LOGIC_SROM_BASE),)
KBUILD_CPPFLAGS += -DDSP_LOGIC_SROM_BASE=$(DSP_LOGIC_SROM_BASE)
endif
export DSP_HIFI4_TRC_TO_MCU     ?= 1
KBUILD_CPPFLAGS += -DDSP_TEST
endif

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

export PSRAM_ENABLE ?= 1
export PSRAM_SIZE ?= 0x4000000
export PSRAM_SPEED ?= 200

KBUILD_CPPFLAGS += -DSYS_PSRAM_BASE=PSRAM_BASE -DSYS_PSRAM_NC_BASE=PSRAM_NC_BASE

export DSI_USE_USBPLL ?= 0
export TEST_RGB565 ?= 0
ifeq ($(DSI_USE_USBPLL),1)
export LCDCPIX_MHZ ?= 40
ifeq ($(TEST_RGB565),1)
KBUILD_CPPFLAGS += -DTEST_RGB565
export DSIPLL_MHZ ?= 320
else
export DSIPLL_MHZ ?= 480
endif
else
export LCDCPIX_MHZ ?= 32
endif

export ENABLE_LVGL ?= 0
ifeq ($(TEST_HAL_PATTERN),TEST_LVGL)
export ENABLE_LVGL := 1
endif
ifeq ($(ENABLE_LVGL),1)
export LVGL_GPU_VGLITE ?= 0
export LVGL_GPU_ARM2D ?= 0
export LVGL_TP_TMA525C ?= 1
export I2C0_IOMUX_INDEX := 34
core-y += thirdparty/lvgl/
ifeq ($(LVGL_GPU_ARM2D),1)
export ENABLE_ARM2D := 1
endif
endif

export ENABLE_ARM2D ?= 0
ifeq ($(TEST_HAL_PATTERN),TEST_ARM2D)
export ENABLE_ARM2D := 1
endif
ifeq ($(ENABLE_ARM2D),1)
core-y += thirdparty/arm2d/
core-y += platform/cmsis/DSP_Lib/
endif
ifeq ($(OPENAMP_ENABLE),1)
export RPMSG_ROLE_MASTER := 1
KBUILD_CPPFLAGS +=  -DRPMSG_ROLE_MASTER

export RPTUN_M55C0_2_DSPC0 := 1
KBUILD_CPPFLAGS +=  -DRPTUN_M55C0_2_DSPC0

RPTUN_M55C0_2_DSPC0_SHMEM_BASE ?= 0
ifneq ($(RPTUN_M55C0_2_DSPC0_SHMEM_BASE),0)
KBUILD_CPPFLAGS += -DRPTUN_M55C0_2_DSPC0_SHMEM_BASE=$(RPTUN_M55C0_2_DSPC0_SHMEM_BASE)
endif

export SYSLOG_RPMSG := 1
KBUILD_CPPFLAGS +=  -DSYSLOG_RPMSG

OPENAMP_ECHO_EXAMPLE := 0
core-y += services/openamp/
endif


ifeq ($(TEST_HAL_PATTERN),TEST_TMA525C)
export I2C0_IOMUX_INDEX := 34
endif

export ALLOW_WARNING ?= 0

export OS_TASKCNT ?= 16
export OS_SCHEDULERSTKSIZE ?= 256
export OS_IDLESTKSIZE ?= 1024
export OS_FIFOSZ ?= 24

LIB_LDFLAGS += -lstdc++ -lsupc++

export ENABLE_PRINT_FLOAT ?= 1

KBUILD_CPPFLAGS += -DSTACK_SECTION_SIZE=0x8000 -DHEAP_SECTION_SIZE=0x8000
