CHIP		?= best1600

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
CHIP_SUBSYS ?= bth
export SUBSYS_FLASH_BOOT ?= 1
export BTH_AS_MAIN_MCU   ?= 0
BTH_USE_SYS_FLASH        ?= 1
BTH_USE_SYS_SENS_PERIPH  ?= 1
export BTH_USE_SYS_GPIO  ?= 3

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

export BT_CONTROLLER_TEST ?= 0

BTC_DEBUG_PORT_MAPOUT     ?= 1

ifeq ($(BT_CONTROLLER_TEST),1)
ifeq ($(BTC_DEBUG_PORT_MAPOUT),1)
ifeq ($(DEBUG_PORT),1)
$(error MAIN_CPU_DEBUG_PORT SAME AS BTC_DEBUG_PORT)
endif
KBUILD_CPPFLAGS += -DBTC_DEBUG_PORT_MAPOUT
endif
endif

DSP_TEST                ?= 0
export DSP_COMBIN_BIN   ?= 0

ifeq ($(DSP_TEST),1)
export DSP_HIFI4_TRC_TO_MCU     ?= 1
KBUILD_CPPFLAGS += -DDSP_TEST
endif

SENSOR_HUB_TEST             ?= 0

ifeq ($(SENSOR_HUB_TEST),1)
export SENSOR_HUB ?= 1
export SENS_TRC_TO_MCU ?= 1
KBUILD_CPPFLAGS += -DSENSOR_HUB_TEST
endif

export TRACE_BUF_SIZE ?= 16*1024

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

##define BTH_RAM0_BASE                           0x20500000
##define BTH_RAM1_BASE                           0x20540000
##define BTH_RAM2_BASE                           0x20560000
##define BTH_RAM3_BASE                           0x20580000
##define BTH_RAM4_BASE                           0x205A0000
##define BTH_RAM5_BASE                           0x205C0000
##define BTH_RAM5_END                            0x205DFFFF

KBUILD_CPPFLAGS +=  -DBTC_ROM_RAM_SIZE=0x00020000*12+0x00010000 \
					-DBTH_RAM_SIZE=0x80000 \
					-DSYS_RAM_SIZE=0 \
					-DHIFI4_ITCM_SIZE=0x20000 \
					-DHIFI4_DTCM_SIZE=0x40000 \
					-DM55_ITCM_SIZE=0 \
					-DM55_DTCM_SIZE=0

##mode 0
##BTC_ROM_RAM_BASE=0xC0400000 \
##BTC_ROM_RAM_SIZE=0x00020000*12+0x00010000
##mode 1
##BTC_ROM_RAM_BASE=0xC0400000 \
##BTC_ROM_RAM_SIZE=0x00020000*10+0x00010000
##mode 2
##BTC_ROM_RAM_BASE=0xC0440000 \
##BTC_ROM_RAM_SIZE=0x00020000*10+0x00010000

CORE_DUMP ?= 0
