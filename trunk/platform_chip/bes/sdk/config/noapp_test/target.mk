CHIP		?= best1700

ifeq ($(CHIP), best1503)
export PSRAM_ENABLE := 0
export PSRAM_SPEED := 0
endif

export RC_CLK_ENABLE ?= 0
ifeq ($(RC_CLK_ENABLE),1)
KBUILD_CPPFLAGS += -DRC_CLK_ENABLE
export PMU_FOCE_LP_MODE ?= 1
export SYS_RC_ENABLE ?= 1
else
export SYS_USE_BBPLL ?= 1
endif

ifeq ($(SLEEP_PD_CURRENT_TEST),1)
KBUILD_CPPFLAGS += -DSLEEP_PD_CURRENT_TEST

CURRENT_TEST 				?= 1
CORE_SLEEP_POWER_DOWN 		?= 1
endif

ifeq ($(CURRENT_TEST),1)
DEBUG_PORT 					?= 2
ifeq ($(MTP_ENABLE),1)
NOSTD						?= 0
else
NOSTD						?= 1
endif
export OSC_26M_X4_AUD2BB	?= 1
export ULTRA_LOW_POWER		?= 1
export POWER_MODE			?= DIG_DCDC
export SLEEP_TEST			?= 1
export NO_TIMER				?= 1
export FLASH_DEEP_SLEEP		?= 1
export FORCE_FLASH1_SLEEP 	?= 1
export FORCE_FLASH2_SLEEP 	?= 1
ifeq ($(CHIP),best1600)
ifeq ($(CHIP_SUBSYS),)
SYS_USE_BTH_FLASH 			?=1
else ifeq ($(CHIP_SUBSYS),bth)
BTH_USE_SYS_FLASH 			?=1
endif
endif
else ifeq ($(SENS_CPU_BOOT_TEST),1)
DEBUG_PORT 					?= 2
NOSTD						?= 1
AUDIO_RESAMPLE				?= 1
export OSC_26M_X4_AUD2BB	?= 1
export ULTRA_LOW_POWER		?= 1
export POWER_MODE			?= DIG_DCDC
export SENSOR_HUB_TEST		?= 1
export SENSOR_HUB			?= 1
else ifeq ($(SENS_CODEC_ADDA_LOOP),1)
DEBUG_PORT 					?= 2
NOSTD						?= 1
AUDIO_RESAMPLE				?= 1
export AF_DEVICE_I2S		?= 0
export AF_DEVICE_INT_CODEC	?= 1
export OSC_26M_X4_AUD2BB	?= 1
export ULTRA_LOW_POWER		?= 1
export POWER_MODE			?= DIG_DCDC
export SENSOR_HUB_TEST		?= 1
export SENSOR_HUB			?= 1
export VAD_CODEC_TEST		?= 1
export VAD_CODEC_LOOP		?= 1
export VAD_USE_SAR_ADC		?= 0
ifeq ($(VAD_USE_SAR_ADC),1)
export VOICE_DETECTOR_SENS_EN	?= 1
endif
else ifeq ($(SENS_CODEC_VAD_PWR_TEST),1)
DEBUG_PORT 					?= 2
AUDIO_RESAMPLE				?= 1
LARGE_SENS_RAM				?= 1
export AF_DEVICE_I2S		?= 0
export AF_DEVICE_INT_CODEC	?= 1
export OSC_26M_X4_AUD2BB	?= 1
export ULTRA_LOW_POWER		?= 1
export POWER_MODE			?= DIG_DCDC
export FLASH_LOW_SPEED      ?= 1
export SENSOR_HUB			?= 1
export SENSOR_HUB_TEST		?= 1
export SENSOR_HUB_MINIMA	?= 0
export VAD_CODEC_TEST		?= 0
export VAD_APP_TEST			?= 1
export VAD_KEY_EVT			?= 1
export VAD_USE_SAR_ADC		?= 0
export VOICE_DETECTOR_SENS_EN ?= 1
endif

DEBUG		?= 1

FPGA		?= 0

DEBUG_PORT	?= 1

FLASH_CHIP	?= ALL

NOAPP		?= 1
export NOAPP

ifeq ($(MTP_ENABLE),1)

MBED		?= 1

endif

ifneq ($(NOSTD),1)

RTOS		?= 1

endif # NOSTD != 1

ifeq ($(MBED)-$(RTOS),1-1)
init-y		:=
core-y		:= platform/

ifeq ($(MTP_ENABLE),1)

core-y += services/fs/block/
ifeq ($(FATFS_ENABLE),1)
KBUILD_CPPFLAGS += -DFS_FATFS_ENABLE
KBUILD_CPPFLAGS += -Iservices/fs/fatfs -Iservices/fs/block
core-y += services/fs/mbed/fat/ services/fs/fatfs/
endif

ifeq ($(LITTLEFS_ENABLE),1)
KBUILD_CPPFLAGS += -DFS_LITTLEFS_ENABLE
KBUILD_CPPFLAGS += -Iservices/fs/littlefs -Iservices/fs/mbed/littlefs/lfs -Iservices/fs/block
core-y += services/fs/mbed/littlefs/ services/fs/littlefs/
endif

core-y += services/mtp/ utils/list/ utils/string_convert/
endif

else # !(MBED-RTOS)
init-y		:=
core-y		:= platform/main/ platform/cmsis/ platform/hal/ \
	platform/drivers/norflash/  platform/drivers/ana/

ifeq ($(BECO),1)
core-y += platform/beco/
endif

ifeq ($(BECO_TEST),1)
export NN_LIB ?= 1
export DSP_LIB ?= 1
KBUILD_CPPFLAGS += -Iinclude/cmsis_dsp
endif

endif # !(MBED-RTOS)

core-y += utils/heap/

ifeq ($(LOAD_LINUX),1)
core-y += platform/drivers/fw_linux/
endif

ifeq ($(OPENAMP_ENABLE),1)
export RPMSG_ROLE_MASTER 	?= 1
export RPTUN_BTH2M55    	?= 1
export RPTUN_BTH2SENS    	?= 1
export RPTUN_BTH2HIFI4    	?= 1

core-y += services/openamp/
endif

ifneq ($(filter 1,$(RTOS) $(GRAPHIC_TEST) $(ARM_CMSE) $(ARM_CMNS)),)
USE_MEM_CFG ?= 1
endif

ifeq ($(USE_MEM_CFG),1)
-include $(srctree)/config/$(CHIP)/mem_cfg.mk
endif

ifeq ($(SENSOR_HUB),1)
ifeq ($(VOICE_DETECTOR_SENS_EN),1)
CODEC_VAD_CFG_BUF_SIZE	?= 0x18000
SENS_RAM_USED_SIZE		?= 0x68000
else
SENS_RAM_USED_SIZE		?= 0x80000
endif
KBUILD_CPPFLAGS += -DSENS_RAM_SIZE=$(SENS_RAM_USED_SIZE)
endif

ifeq ($(DSP_HIFI4_TEST),1)
export DSP_HIFI4_TRC_TO_MCU ?= 1
endif

ifneq ($(filter best1600 best1603,$(CHIP)),)
ifeq ($(BTH_AS_MAIN_MCU),1)
export BTH_USE_SYS_PERIPH := 1
endif
ifeq ($(DSP_HIFI4_TRC_TO_MCU),1)
KBUILD_CPPFLAGS += -DDSP_HIFI4_TRC_TO_MCU
endif
ifeq ($(RMT_TRC_IN_MSG_CHAN),1)
KBUILD_CPPFLAGS += -DRMT_TRC_IN_MSG_CHAN
endif
endif

ifeq ($(BT_RAMRUN_NEW),1)
KBUILD_CPPFLAGS += -D__BT_RAMRUN_NEW__
endif

ifeq ($(BT_DRV_TEST),1)
ifneq ($(BTH_AS_MAIN_MCU),1)
ifeq ($(CHIP_SUBSYS),)
KBUILD_CPPFLAGS += -D__SYS_AS_MAIN__
endif
endif
endif

export DMA_AUDIO_APP ?= 0
ifeq ($(DMA_AUDIO_APP),1)
export DMA_RPC_CLI ?= 1

ifeq ($(CHIP),best1600)
DMA_AUD_CFG_PATH := best1600
else ifeq ($(CHIP),best1502x)
DMA_AUD_CFG_PATH := best1502x_ibrt
else ifeq ($(CHIP),best1603)
DMA_AUD_CFG_PATH := best1603
else
DMA_AUD_CFG_PATH := $(CHIP)
endif
include $(srctree)/config/$(DMA_AUD_CFG_PATH)/dma_aud_cfg.mk
endif

ifeq ($(CSI_DSI_CONFIG),1)
include $(srctree)/tests/hwtest/csi_test/cfg/csi_dsi_cfg.mk
endif

ifeq ($(ROM_BUILD),1)
LDS_FILE	?= rom.lds
ROM_SIZE	?= 0x80000
else ifeq ($(INTSRAM_RUN),1)
LDS_FILE	?= best1000_intsram.lds
else ifeq ($(NANDFLASH_BUILD),1)
LDS_FILE	?= best1000_nand.lds
else
LDS_FILE	?= best1000.lds
endif

ifeq ($(KERNEL), LITEOS_M)
export OSTICK_USE_FAST_TIMER := 0
KBUILD_CPPFLAGS += -DOS_HEAP_LINK_NAME=m_aucSysMem0 -DOS_HEAP_SIZE=0x80000
KBUILD_CPPFLAGS += -DOS_WRAP_MALLOC
LDFLAGS_IMAGE += --wrap malloc --wrap calloc --wrap free --wrap realloc
endif

export CMSIS_TEST ?= 0
ifeq ($(CMSIS_TEST),1)
LDS_CPPFLAGS += \
        -DCMSIS_TEST
endif

export USE_MULTI_FLASH ?= 0
ifeq ($(USE_MULTI_FLASH),1)
export FLASH1_SIZE ?= 0x400000
KBUILD_CPPFLAGS += \
        -DUSE_MULTI_FLASH \
        -DFLASH1_SIZE=$(FLASH1_SIZE)
LDS_CPPFLAGS += \
        -DUSE_MULTI_FLASH \
        -DFLASH1_SIZE=$(FLASH1_SIZE)
endif

ifeq ($(SEPARATE_RAM_RAMX),1)
export SEPARATE_RAM_RAMX
RAMX_REGION_OFFSET ?= 0
RAMX_REGION_SIZE ?= 0x40000
RAM_REGION_OFFSET ?= $(RAMX_REGION_OFFSET)+$(RAMX_REGION_SIZE)
RAM_REGION_SIZE ?= 0x20000
LDS_CPPFLAGS += -DSEPARATE_RAM_RAMX
KBUILD_CPPFLAGS += \
		-DRAMX_REGION_OFFSET=$(RAMX_REGION_OFFSET) \
		-DRAMX_REGION_SIZE=$(RAMX_REGION_SIZE) \
		-DRAM_REGION_OFFSET=$(RAM_REGION_OFFSET) \
		-DRAM_REGION_SIZE=$(RAM_REGION_SIZE)
endif

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal -Iplatform/drivers/usb/usb_dev/inc

KBUILD_CPPFLAGS +=

KBUILD_CFLAGS +=

KBUILD_CFLAGS +=

LIB_LDFLAGS += -lstdc++ -lsupc++

CORE_DUMP ?= 1

ifneq ($(TOOLCHAIN),armclang)
ifneq ($(filter 1,$(COREMARK_TEST) $(DHRYSTONE_TEST) $(SENSOR_TEST) $(SDMMC_TEST) $(I3C_TEST)),)
export ENABLE_PRINT_FLOAT ?= 1
endif
endif

#LDFLAGS_IMAGE += --wrap main
