CHIP		?= best1600

export SUBSYS_CACHE_ENABLE ?= 1

export OSC_26M_X4_AUD2BB	        ?= 1

export ULTRA_LOW_POWER		        ?= 1

export POWER_MODE			?= DIG_DCDC

FLASH_CHIP	?= ALL
# GD25Q80C
# GD25Q32C
# ALL

GRAPH_MODULE := 0
##  for slim earphone config
## for security core do not define it
export SUBSYS_FLASH_BOOT ?= 0
export BTH_USE_SYS_PERIPH := 1
export BTH_AS_MAIN_MCU ?= 1
export BTH_USE_SYS_FLASH         ?= 1
export BTH_USE_SYS_SENS_PERIPH   ?= 1
export BTH_USE_SYS_GPIO   ?= 3

##  for slim earphone config

DEBUG		?= 1

DEBUG_PORT      ?= 1
ifeq ($(DEBUG_PORT),2)
KBUILD_CPPFLAGS += -DUART1_IOMUX_INDEX=26
endif

export RMT_TRACE ?= 1

FPGA 		?= 0

FPGA_VERSION	?= 1600

RTOS		?= 1

export BTC_NO_UART_BRIDGE ?= 1

export TRACE_BUF_SIZE ?= 4*1024

export TRACE_BAUD_RATE ?= 8*115200

init-y		:=
core-y		:= tests/besair_platform/ platform/cmsis/ platform/drivers/ platform/hal/ utils/hwtimer_list/ utils/crc32/

ifeq ($(SUBSYS_FLASH_BOOT),1)
LDS_FILE	:= best1000.lds
else
LDS_FILE	:= subsys.lds
endif


KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal -Iservices/audioflinger -Iapps/common -Iapps/audioplayers -Imultimedia/inc/audio/codec/sbc/inc

LIB_LDFLAGS += -lstdc++ -lsupc++

#export ENABLE_PRINT_FLOAT ?= 1

OS_SCHEDULERSTKSIZE := 256
OS_TASKCNT := 14

KBUILD_CPPFLAGS += -D__HAL_IPC_NOTIFY_SUPPORT__


#KBUILD_CPPFLAGS += -DMAIN_RAM_USE_SYS_RAM -DSYS_RAM_SIZE=0x40000
#KBUILD_CPPFLAGS +=  -DMAIN_RAM_USE_SYS_RAM

CORE_DUMP ?= 0

ifeq ($(RMT_TRACE),1)
export DSP_M55_TRC_TO_MCU ?= 1
export RMT_TRC_IN_MSG_CHAN ?= 1

ifeq ($(DSP_M55_TRC_TO_MCU),1)
export TRACE_GLOBAL_TAG ?= 1
export DUMP_LOG_ENABLE ?= 1
export LOG_DUMP_SECTION_SIZE ?= 0
endif
endif
