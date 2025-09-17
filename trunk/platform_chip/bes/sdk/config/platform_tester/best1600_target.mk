CHIP		?= best1600

export OSC_26M_X4_AUD2BB	        ?= 1

export ULTRA_LOW_POWER		        ?= 1

export POWER_MODE			?= DIG_DCDC

FLASH_CHIP	?= ALL
# GD25Q80C
# GD25Q32C
# ALL
export RC48M_ENABLE ?= 0
export RC48M_X2_ENABLE ?= 0
#export FLASH_LOW_SPEED      ?= 1
GRAPH_MODULE := 0
##  for slim earphone config
## for security core do not define it
CHIP_SUBSYS ?=

ifeq ($(CHIP),best1600)
include $(srctree)/config/best1600/mem_cfg.mk
endif

# best1600 part number z66/z6602/d66/w67/867
# z66:   Flash  8MB+8MB+4MB, PSRAM 32MB;
# z6602: Flash  8MB+4MB,     PSRAM 32MB;
# d66:   Flash 16MB+4MB,     PSRAM 32MB;
# w67:   Flash  8MB+2MB,     PSRAM 64MB;
# 867:   Flash  8MB,         PSRAM 64MB;
export CHIP_PARTNO ?= z66

export SYS_USE_BTH_FLASH ?= 1

export SLEEP_PD_CURRENT_TEST ?= 0

ifeq ($(SLEEP_PD_CURRENT_TEST),1)
CORE_SLEEP_POWER_DOWN 		?= 1
export FLASH_DEEP_SLEEP		?= 1
export FORCE_FLASH1_SLEEP	?= 1
KBUILD_CPPFLAGS += -DSLEEP_PD_CURRENT_TEST
endif

##  for slim earphone config

DEBUG		?= 1

DEBUG_PORT      ?= 1
ifeq ($(DEBUG_PORT),2)
KBUILD_CPPFLAGS += -DUART1_IOMUX_INDEX=26
endif

export RMT_TRACE ?= 1

FPGA 		?= 0

RTOS		?= 1

export BTC_NO_UART_BRIDGE ?= 1

BTC_DEBUG_PORT_MAPOUT     ?= 1

ifeq ($(BT_CONTROLLER_TEST),1)
ifeq ($(BTC_DEBUG_PORT_MAPOUT),1)
ifeq ($(DEBUG_PORT),1)
$(error MAIN_CPU_DEBUG_PORT SAME AS BTC_DEBUG_PORT)
endif
KBUILD_CPPFLAGS += -DBTC_DEBUG_PORT_MAPOUT
endif
endif

DSP_TEST                  ?= 0
export DSP_COMBINE_BIN    ?= 0

# Set hifi boot address.
# export DSP_LOGIC_SROM_BASE ?= 0x28050000

# BBPLL Settings
export SYS_USE_BBPLL ?= 1
export BBPLL_MHZ ?= 400
export AUD_PLL_DOUBLE ?= 1

# PSRAM Settings
export PSRAM_ENABLE ?= 1
export PSRAM_SPEED ?= 200
ifeq ($(CHIP_PARTNO),z66)
export PSRAM_SIZE ?= 0x2000000
endif
ifeq ($(CHIP_PARTNO),z6602)
export PSRAM_SIZE ?= 0x2000000
endif
ifeq ($(CHIP_PARTNO),d66)
export PSRAM_SIZE ?= 0x2000000
endif
ifeq ($(CHIP_PARTNO),w67)
export PSRAM_SIZE ?= 0x4000000
export PSRAM_DUAL_8BIT := 1
endif
ifeq ($(CHIP_PARTNO),867)
export PSRAM_SIZE ?= 0x4000000
endif

export FLASH_DUAL_CHIP ?= 1
ifeq ($(CHIP_PARTNO),z66)
export FLASH_DUAL_CHIP ?= 1
export FLASH_SIZE ?= 0x1000000
endif
ifeq ($(CHIP_PARTNO),z6602)
export FLASH_DUAL_CHIP := 0
export FLASH_SIZE ?= 0x800000
endif
ifeq ($(CHIP_PARTNO),d66)
export FLASH_DUAL_CHIP := 0
export FLASH_SIZE ?= 0x1000000
endif
ifeq ($(CHIP_PARTNO),w67)
export FLASH_DUAL_CHIP := 0
export FLASH_SIZE ?= 0x800000
endif
ifeq ($(CHIP_PARTNO),867)
export FLASH_DUAL_CHIP := 0
export FLASH_SIZE ?= 0x800000
endif

ifeq ($(FLASH_DUAL_CHIP),1)
export FLASH_DUAL_CHIP_INIT_ON := 1
export OTA_CODE_OFFSET := 0x20000
endif

KBUILD_CPPFLAGS += -DFLASH_UNIQUE_ID

export TEST_RGB565 ?= 0
export USE_DSI_PLL ?= 1
export DSIPLL_MHZ ?= 480
export LCDCPIX_MHZ ?= 20
ifeq ($(TEST_RGB565),1)
export DSIPLL_MHZ := 320
KBUILD_CPPFLAGS += -DDSIPLL_MHZ=320 -DTEST_RGB565 -DCONFIG_DSIPHY_OUTFMT_RGB565
endif

ifeq ($(DSP_TEST),1)
ifneq ($(DSP_LOGIC_SROM_BASE),)
KBUILD_CPPFLAGS += -DDSP_LOGIC_SROM_BASE=$(DSP_LOGIC_SROM_BASE)
endif
export DSP_HIFI4_TRC_TO_MCU     ?= 1
KBUILD_CPPFLAGS += -DDSP_TEST
endif

SENSOR_HUB_TEST             ?= 0

ifeq ($(SENSOR_HUB_TEST),1)
export SENSOR_HUB ?= 1
export SENS_TRC_TO_MCU ?= 1
KBUILD_CPPFLAGS += -DSENSOR_HUB_TEST
endif

BT_APP                     ?= 1
BT_HOST_TEST               ?= 0

ifeq ($(BT_HOST_TEST),1)
export BT_HOST        ?= 1
export BTH_FLASH_BOOT ?= 1
FORCE_FLASH1_SLEEP := 0
KBUILD_CPPFLAGS += -DBT_HOST_TEST
endif

export APP_TRACE_RX_ENABLE ?= 0
ifeq ($(APP_TRACE_RX_ENABLE),1)
KBUILD_CPPFLAGS += -DAPP_TRACE_RX_ENABLE
endif

export UTILS_ESHELL_EN ?= 0

export TRACE_BUF_SIZE ?= 4*1024

export TRACE_BAUD_RATE ?= 8*115200

init-y		:=
core-y		:= tests/besair_platform/ platform/cmsis/ platform/drivers/ platform/hal/ utils/hwtimer_list/ apps/custom_apps/

core-y      += utils/crc32/ utils/heap/ utils/cqueue/ utils/kfifo/ services/overlay/

core-y      += services/audioflinger/ multimedia/ services/audio_dump/

export HW_RANDOM ?= 1

ifeq ($(AUDIOFLINGER_SUPPORT),1)
KBUILD_CPPFLAGS += -DAUDIOFLINGER_SUPPORT
endif

export SPEECH_LIB ?= 1
export ONLY_BUILD_SCO_CODEC ?= 1
core-y += apps/audioplayers/

ifeq ($(APP_RPC_BTH_DSP_EN),1)
KBUILD_CPPFLAGS += -DAPP_RPC_BTH_DSP_EN -DNO_BTH_IPC_WHEN_OPEN
endif

KBUILD_CPPFLAGS += -DSPEECH_CODEC_CAPTURE_CHANNEL_NUM=1
export SPEECH_SCO_FRAME_MS        ?= 15
KBUILD_CPPFLAGS += -DSPEECH_SCO_FRAME_MS=$(SPEECH_SCO_FRAME_MS)
export SPEECH_PROCESS_FRAME_MS := 15
KBUILD_CPPFLAGS += -DSPEECH_PROCESS_FRAME_MS=$(SPEECH_PROCESS_FRAME_MS)

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

ifeq ($(SENSOR_HUB),1)
core-y		+= apps/sensorhub/
endif

ifeq ($(OPENAMP_CONTROLLER),1)
KBUILD_CPPFLAGS += -DOPENAMP_CONTROLLER
endif
ifeq ($(OPENAMP_TARGET),1)
KBUILD_CPPFLAGS += -DOPENAMP_TARGET
endif

export ENABLE_LVGL ?= 0
ifeq ($(TEST_HAL_PATTERN),TEST_LVGL)
export ENABLE_LVGL := 1
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

export ENABLE_ARM2D ?= 0
ifeq ($(TEST_HAL_PATTERN),TEST_ARM2D)
export ENABLE_ARM2D := 1
endif
ifeq ($(ENABLE_ARM2D),1)
core-y += thirdparty/arm2d/
endif

LDS_FILE	:= best1000.lds

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal -Iservices/audioflinger -Iapps/common -Iapps/audioplayers -Imultimedia/inc/audio/codec/sbc/inc

ifeq ($(SLEEP_PD_CURRENT_TEST),0)
ifeq ($(BT_HOST_TEST),1)
KBUILD_CPPFLAGS +=  -DBTH_RAM_SIZE=0xE0000
endif
endif

ifeq ($(SENSOR_HUB_TEST),1)
KBUILD_CPPFLAGS +=  -DSENS_RAM_SIZE=0x80000
endif

ifeq ($(BT_HOST_TEST),1)
export APP_RPC_ENABLE ?= 1
ifeq ($(APP_RPC_ENABLE),1)
KBUILD_CPPFLAGS += -DAPP_RPC_ENABLE
core-y += apps/app_rpc/ apps/common/
export BTH_FLASH_BOOT ?= 1
export DSP_M55_FLASH_BOOT ?= 1
export APP_RPC_BTH_M55_EN ?= 0
export NO_BTH_IPC_WHEN_OPEN ?= 1
endif

ifeq ($(APP_RPC_BTH_M55_EN),1)
KBUILD_CPPFLAGS += -DAPP_RPC_BTH_M55_EN
endif

ifeq ($(APP_RPC_MCU_SENSOR_EN), 1)
EXT_SRC_DIR += apps/app_rpc/
KBUILD_CPPFLAGS += -DAPP_RPC_MCU_SENSOR_EN
endif

export APP_RPC_BTH_DSP_EN ?= 1
ifeq ($(APP_RPC_BTH_DSP_EN),1)
KBUILD_CPPFLAGS += -DAPP_RPC_BTH_DSP_EN
endif

export BTH_IF_COMM_RPC_EN ?= 1
ifeq ($(BTH_IF_COMM_RPC_EN),1)
core-y += apps/bth_if/
KBUILD_CPPFLAGS += -DBTH_IF_COMM_RPC_EN
endif
endif

ifeq ($(OPENAMP_ENABLE),1)
export RPMSG_ROLE_MASTER 	?= 1
export RPTUN_BTH2M55    	?= 1
export RPTUN_BTH2SENS    	?= 1

core-y += services/openamp/
endif

ifeq ($(CHIP),best1600)
export RTC_CALENDAR ?= 1
endif
ifeq ($(RTC_CALENDAR),1)
KBUILD_CPPFLAGS += -DRTC_CALENDAR
endif

LIB_LDFLAGS += -lstdc++ -lsupc++

ifneq ($(filter 1,$(SENSOR_TEST) $(POWER_TEST)),)
export ENABLE_PRINT_FLOAT ?= 1
endif

OS_SCHEDULERSTKSIZE := 256
OS_TASKCNT := 14


##define BTH_RAM0_BASE                           0x20500000
##define BTH_RAM1_BASE                           0x20540000
##define BTH_RAM2_BASE                           0x20560000
##define BTH_RAM3_BASE                           0x20580000
##define BTH_RAM4_BASE                           0x205A0000
##define BTH_RAM5_BASE                           0x205C0000
##define BTH_RAM5_END                            0x205DFFFF

CORE_DUMP ?= 0

KBUILD_CPPFLAGS += -DSTACK_SECTION_SIZE=0x8000 -DHEAP_SECTION_SIZE=0x8000
