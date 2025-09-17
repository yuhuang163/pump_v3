CHIP    ?= 	best1501p

export OSC_26M_X4_AUD2BB          ?= 1

export ULTRA_LOW_POWER            ?= 1

export POWER_MODE      ?= DIG_DCDC

FLASH_CHIP  ?= ALL

DEBUG    ?= 1

DEBUG_PORT      ?= 1
ifeq ($(DEBUG_PORT),2)
KBUILD_CPPFLAGS += -DUART1_IOMUX_INDEX=26
endif

RTOS    ?= 1

AUDIO_RESAMPLE ?= 1

AUDIOFLINGER_SUPPORT ?= 1

export BTC_NO_UART_BRIDGE ?= 1

SENSOR_HUB_TEST             ?= 0

SENSOR_HUB_BSP_TEST			?= 0

ifeq ($(SENSOR_HUB_TEST),1)
export SENSOR_HUB ?= 1
export SENS_TRC_TO_MCU ?= 1
KBUILD_CPPFLAGS += -DSENSOR_HUB_TEST
endif

export TRACE_BUF_SIZE ?= 16*1024

export TRACE_BAUD_RATE ?= 8*115200

init-y    :=

core-y   := \
	platform/cmsis/ \
	platform/drivers/ \
	platform/hal/ \
	utils/hwtimer_list/ \
	utils/crc32/ \
	utils/heap/ \
	utils/cqueue/ \
	apps/common/ \
	services/audioflinger/ \
	tests/besair_platform/ \
	tests/besair_platform/bsp_tester/audio_tester/ \
	tests/besair_platform/bsp_tester/bt_tester/ \
	tests/besair_platform/bsp_tester/flash_tester/ \
	tests/besair_platform/bsp_tester/gpio_tester/ \
	tests/besair_platform/bsp_tester/i2c_tester/ \
	tests/besair_platform/bsp_tester/spi_tester/ \
	tests/besair_platform/bsp_tester/wdt_tester/ \
	tests/besair_platform/bsp_tester/common_debug_interface/

LDS_FILE  := best1000.lds

KBUILD_CPPFLAGS += \
	-Iapps/common \
	-Iapps/audioplayers \
	-Iplatform/cmsis/inc \
	-Iplatform/hal \
	-Iservices/audioflinger \
	-Imultimedia/inc/audio/codec/sbc/inc \
	-Itests/besair_platform

LIB_LDFLAGS += -lstdc++ -lsupc++

OS_SCHEDULERSTKSIZE := 256
OS_TASKCNT := 14

CORE_DUMP ?= 0

ifeq ($(AUDIOFLINGER_SUPPORT),1)
KBUILD_CPPFLAGS += -DAUDIOFLINGER_SUPPORT
endif

ifeq ($(CHIP),best1502x)
KBUILD_CPPFLAGS += -DAUDIO_OUTPUT_SWAP
endif

ifeq ($(SENSOR_HUB_BSP_TEST),1)
core-y += apps/sensorhub/
endif
