CHIP		?= best1503

DEBUG		?= 1

DEBUG_PORT	?= 2

FAULT_DUMP	?= 1

NOSTD		?= 0

export CHIP_ROLE_CP ?= 1

ifneq ($(NOSTD),1)
RTOS 		?= 1
#KERNEL 		:=  FREERTOS
export DSP_LIB ?= 1
endif

ifeq ($(RTOS),1)
USE_MEM_CFG ?= 1
else
NOAPP		?= 1
export NOAPP
endif

KERNEL ?= RTX5

export RTOS := 0

ifeq ($(RTOS), 1)
export OS_DYNAMIC_MEM_SIZE ?= 16*1024
ifeq ($(KERNEL), FREERTOS)
KBUILD_CPPFLAGS += -DconfigTOTAL_HEAP_SIZE=$(OS_DYNAMIC_MEM_SIZE)
endif
endif


export CORE_SLEEP_POWER_DOWN ?= 1

AUDIO_RESAMPLE ?= 1

export OSC_26M_X4_AUD2BB ?= 1

export ULTRA_LOW_POWER ?= 1

export SLEEP_TEST	?= 1

ifeq ($(PSAP_SW_APP),1)
export SPEECH_LIB ?= 1
export DMA_AUDIO_APP ?= 1
endif

export DMA_AUDIO_APP ?= 0
ifeq ($(DMA_AUDIO_APP),1)
export DMA_RPC_SVR ?=1
ifeq ($(CHIP),best1600)
DMA_AUD_CFG_PATH := best1600
else ifeq ($(CHIP),best1502x)
DMA_AUD_CFG_PATH := best1502x_ibrt
else
DMA_AUD_CFG_PATH := $(CHIP)
endif
include $(srctree)/config/$(DMA_AUD_CFG_PATH)/dma_aud_cfg.mk
endif

export NO_OVERLAY := 1

export ASSERT_SHOW_FILE_FUNC ?= 1

init-y		:=
core-y		:= platform/main/cp_subsys/ platform/cmsis/ platform/hal/ utils/hwtimer_list/

core-y += tests/besair_platform/bsp_tester/

ifeq ($(UTILS_ESHELL_EN), 1)
ifneq ($(NOAPP),0)
core-y += apps/common/
endif
endif

ifeq ($(IMS_SUPPORT),1)
export NET_OPENSSL_VER := openssl-1.1.1i
export WIFI_RF_TEST_ONLY :=1
export IMS_ON_CP :=1
ifeq ($(SMF),1)
core-y += multimedia/
KBUILD_CPPFLAGS += -DSMF_NUTTX_INIT
endif
endif

ifneq ($(EXT_SRC_DIR),)
core-y += $(EXT_SRC_DIR)
else
ifneq ($(NOAPP),0)
core-y		+= utils/heap/ utils/cqueue/
endif
endif

ifeq ($(OPENAMP_APP),1)
ifneq ($(NOAPP),0)
core-y += apps/openamp/
endif
endif

ifeq ($(SUBSYS_FLASH_BOOT),1)
core-y		+= platform/drivers/norflash/
LDS_FILE	:= best1000.lds
else
LDS_FILE	:= subsys.lds
endif

ifeq ($(AUDIO_DSP_ACCEL_USE_CP_SUBSYS),1)
KBUILD_CPPFLAGS += -DAUDIO_DSP_ACCEL_USE_CP_SUBSYS
export APP_MCPP_CLI := CP_SUBSYS
export APP_MCPP_SRV ?= thirdparty
export APP_MCPP_BTH_CP := 1
endif

ifeq ($(APP_MCPP_CLI),CP_SUBSYS)
export APP_MCPP_BTH_CP_SUBSYS := 1
core-y		+= utils/kfifo/ services/mcpp/ utils/stream_mcps/
KBUILD_CPPFLAGS += -DAPP_MCPP_CLI
KBUILD_CPPFLAGS += -DAPP_MCPP_CLI_CP_SUBSYS
export MCPP_TRIGGER_DSP_PROCESS ?= 0
endif

ifneq ($(APP_MCPP_SRV),)
KBUILD_CPPFLAGS += -DAPP_MCPP_SRV
endif

export CP_SUBSYS_TRC_TO_MCU ?= 1
ifeq ($(CP_SUBSYS_TRC_TO_MCU),1)
export TRACE_BUF_SIZE ?= 0
export TRACE_GLOBAL_TAG ?= 1
export DUMP_LOG_ENABLE ?= 1
export LOG_DUMP_SECTION_SIZE ?= 0
endif

ifeq ($(PSAP_SW_APP),1)
core-y += apps/psap_sw/dsp/
endif

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal

ifeq ($(USE_MEM_CFG),1)
-include $(srctree)/config/$(CHIP)/mem_cfg.mk
endif

export CP_INTR_RELAY ?= 1
ifeq ($(APP_RPC_MCU_CP_EN),1)
KBUILD_CPPFLAGS += -DAPP_RPC_ENABLE
KBUILD_CPPFLAGS += -DAPP_RPC_MCU_CP_EN
core-y += apps/app_rpc/
endif

ifeq ($(SMF_PLAYER_EN),1)
KBUILD_CPPFLAGS += -DCHIP_DMA_CFG_IDX=1
KBUILD_CPPFLAGS += -DSMF_PLAYER_EN
ifeq ($(SMF),1)
core-y += multimedia/
core-y += utils/kfifo/
endif
endif

# CP_SUBSYS XIP
ifeq ($(SUBSYS_FLASH_BOOT),1)
export NO_FLASH_INIT ?= 1
ifneq ($(XIP_BOOT_OFFSET),)
KBUILD_CPPFLAGS += -DOTA_CODE_OFFSET=$(XIP_BOOT_OFFSET)
endif
endif

KBUILD_CFLAGS +=

ifeq ($(CONFIG_HAVE_CXX),1)
8
LIB_LDFLAGS += -lstdc++ -lsupc++
else
LIB_LDFLAGS +=
endif

CFLAGS_IMAGE +=

LDFLAGS_IMAGE +=

export ALLOW_WARNING := 1

export ENABLE_PRINT_FLOAT ?= 1

ifneq ($(PSRAM_REGION_SIZE), )
KBUILD_CPPFLAGS += -DPSRAM_REGION_SIZE=$(PSRAM_REGION_SIZE)
endif

ifeq ($(SUBSYS_IMAGE_SEGMENT),1)
KBUILD_CPPFLAGS += -DSUBSYS_IMAGE_SEGMENT
endif

ifneq ($(OS_DYNAMIC_MEM_SIZE), )
KBUILD_CPPFLAGS += -DOS_DYNAMIC_MEM_SIZE=$(OS_DYNAMIC_MEM_SIZE)
endif

export COREMARK_DHRY_BUILD ?= gcc

export CORE_DUMP ?= 0
ifeq ($(CORE_DUMP), 1)
export DUMP_RAM_RAW_ONLY ?= 0
ifeq ($(DUMP_RAM_RAW_ONLY), 1)
KBUILD_CPPFLAGS += -DDUMP_RAM_RAW_ONLY
endif
endif

export CHIP_DMA_CFG_IDX ?= 1
ifeq ($(CHIP_DMA_CFG_IDX), 1)
KBUILD_CPPFLAGS += -DCHIP_DMA_CFG_IDX
endif
