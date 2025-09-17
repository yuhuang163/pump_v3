CHIP		?= best1600

export OSC_26M_X4_AUD2BB	        ?= 1

export ULTRA_LOW_POWER		        ?= 1

export POWER_MODE			?= DIG_DCDC

FLASH_CHIP	?= ALL
# GD25Q80C
# GD25Q32C
# ALL

#GRAPH_MODULE := 0
##  for slim earphone config
## for security core do not define it
##CHIP_SUBSYS ?= bth

##  for slim earphone config

DEBUG		?= 1

DEBUG_PORT      ?= 1
ifeq ($(DEBUG_PORT),2)
KBUILD_CPPFLAGS += -DUART1_IOMUX_INDEX=26
endif

RTOS		?= 1

export BTC_NO_UART_BRIDGE ?= 1

SENSOR_HUB_TEST             ?= 0

ifeq ($(SENSOR_HUB_TEST),1)
export SENSOR_HUB ?= 1
export SENS_TRC_TO_MCU ?= 1
KBUILD_CPPFLAGS += -DSENSOR_HUB_TEST
endif

export TRACE_BUF_SIZE ?= 16*1024

export TRACE_BAUD_RATE ?= 8*115200

init-y		:=
core-y		:= tests/besair_platform/ platform/cmsis/ platform/drivers/ platform/hal/ utils/hwtimer_list/ utils/crc32/ apps/common/

LDS_FILE	:= best1000.lds

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal -Iservices/audioflinger -Iapps/common -Iapps/audioplayers -Imultimedia/inc/audio/codec/sbc/inc

LIB_LDFLAGS += -lstdc++ -lsupc++

#export ENABLE_PRINT_FLOAT ?= 1 #浮点数

# RTOS CFG
OS_SCHEDULERSTKSIZE := 256
OS_TASKCNT := 14

#crash dump
CORE_DUMP ?= 0
