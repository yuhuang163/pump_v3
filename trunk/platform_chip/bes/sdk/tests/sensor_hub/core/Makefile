cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

ccflags-y := -Iplatform/hal \
	-Iplatform/cmsis \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/sensor_hub \
	-Iutils/hwtimer_list \
	-Iservices/audioflinger \
	-Iservices/norflash_api \
	-Iapps/sensorhub \
	-Iapps/main \
	-Iapps/common \
	-Itests/sensor_hub/inc \
	-Itests/sensor_hub/core \
	-Itests/sensor_hub/vad \
	-Itests/sensor_hub/minima \
	-Itests/sensor_hub/sensor_hub_ai \
	-Itests/sensor_hub/test \
	-Itests/besair_platform/cmd_test \
	-Iapps/app_eshell \
	-Iutils/eshell

ifeq ($(OPENAMP_TARGET),1)
ccflags-y += -Iservices\openamp\rptun\chips
endif

ifeq ($(AI_VOICE),1)
ccflags-y += -DAI_VOICE
endif

ifeq ($(SENSOR_HUB_MINIMA),1)
ccflags-y += -DSENSOR_HUB_MINIMA
endif

ifeq ($(CAPSENSOR_ENABLE),1)
obj-y += \
    ../../../services/capsensor/

ccflags-y += \
    -Iservices/capsensor/
endif

ifeq ($(CORE_BRIDGE_DEMO_MSG),1)
CFLAGS_sensor_hub_core_app.o += -DCORE_BRIDGE_DEMO_MSG
endif

ifeq ($(NOAPP),1)
obj-y := $(filter-out sensor_hub_core_app.c,$(obj-y))
ccflags-y += -DNOAPP
endif

ifeq ($(BT_APP),1)
CFLAGS_sensor_hub_main.o += -D__BT_APP__
endif

ifneq ($(DEBUG_PORT),)
CFLAGS_sensor_hub_main.o += -DDEBUG_PORT=$(DEBUG_PORT)
endif

ifeq ($(SUBSYS_FLASH_BOOT),1)
CFLAGS_sensor_hub_main.o += -DSUBSYS_FLASH_BOOT
endif

ifeq ($(NO_FLASH_INIT),1)
CFLAGS_sensor_hub_main.o += -DNO_FLASH_INIT
endif

ifeq ($(SENS_TRC_TO_MCU),1)
ifneq ($(BT_APP),1)
obj-y += ../../../platform/drivers/rmt_trace/
endif
CFLAGS_sens_trc_to_mcu.o += -Iplatform/drivers/rmt_trace -DSENS_TRC_TO_MCU
CFLAGS_sensor_hub_main.o += -DSENS_TRC_TO_MCU
endif

ifeq ($(DMA_AUDIO_APP),1)
ccflags-y += -DDMA_AUDIO_APP -Iservices/audioflinger -Iservices/dma_audio/dsp -Iservices/dma_audio/common
ifeq ($(DMA_AUDIO_APP_DYN_ON),1)
ccflags-y += -DDMA_AUDIO_APP_DYN_ON
endif
obj-y += ../../../platform/drivers/stream_dma_rpc/
obj-y += ../../../services/dma_audio/dsp/
endif

ifeq ($(FULL_WORKLOAD_MODE_ENABLED),1)
ccflags-y += -DFULL_WORKLOAD_MODE_ENABLED
endif

ifeq ($(FAST_TIMER_COMPENSATE),1)
ccflags-y += -DFAST_TIMER_COMPENSATE
endif

ifeq ($(SENSOR_HUB_BSP_TEST),1)
ccflags-y += -DSENSOR_HUB_BSP_TEST
endif

ifeq ($(VPU_CFG_ON_SENSOR_HUB),1)
CFLAGS_sensor_hub_core_app.o += -DVPU_CFG_ON_SENSOR_HUB
ccflags-y += -Iservices/bone_sensor/vpu_cfg
endif

ifeq ($(SPA_AUDIO_ENABLE),1)
subdir-ccflags-y += -Ithirdparty/userapi/spa_dummy_app/inc
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)
