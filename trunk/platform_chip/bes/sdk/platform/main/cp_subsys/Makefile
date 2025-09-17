cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))
obj-y += ../../../platform/drivers/cp_subsys/cp_subsys_msg_test.c

ifeq ($(PSAP_SW_APP),1)
export SPEECH_LIB := 1
export NO_MULTIMEDIA_CODEC ?= 1
obj-y += ../../../multimedia/
endif

ccflags-y += \
	-Iplatform/drivers/cp_subsys \
	-Iutils/hwtimer_list \
	-Itests/besair_platform/bsp_tester

ifeq ($(NOAPP),1)
ccflags-y += -DNOAPP
endif

ifeq ($(CP_SUBSYS_TRC_TO_MCU),1)
obj-y += ../../../platform/drivers/rmt_trace/
TRC_FLAGS := -Iplatform/drivers/rmt_trace -DCP_SUBSYS_TRC_TO_MCU
CFLAGS_cp_subsys_main.o += $(TRC_FLAGS)
CFLAGS_cp_subsys_trc_to_mcu.o += $(TRC_FLAGS)
endif

ifeq ($(MCU2CP_MSG_TEST),1)
CP_SUBSYS_HWTEST := 1
obj-y += ../../../tests/hwtest/rmt_msg_test.c
ccflags-y += -DMCU2CP_MSG_TEST
endif

ifeq ($(MCU2CP_COMM_DEMO_TEST),1)
CP_SUBSYS_HWTEST := 1
obj-y += ../../../tests/hwtest/mcu2cp_comm_test.c
ccflags-y += -DMCU2CP_COMM_DEMO_TEST
endif

ifeq ($(CPU_MAX_POWER_TEST),1)
CP_SUBSYS_HWTEST := 1
obj-y += ../../../tests/cpu_max_power/
ccflags-y += -DCPU_MAX_POWER_TEST
endif

ifeq ($(COREMARK_TEST),1)
CP_SUBSYS_HWTEST := 1
obj-y += ../../../tests/coremark/
ccflags-y += -DCOREMARK_TEST
endif

ifeq ($(DHRYSTONE_TEST),1)
CP_SUBSYS_HWTEST := 1
obj-y += ../../../tests/dhrystone2/
ccflags-y += -DDHRYSTONE_TEST
endif

ifeq ($(SBC_TEST),1)
CP_SUBSYS_HWTEST := 1
obj-y += ../../../multimedia/audio/codec/sbc/
obj-y += ../../../tests/hwtest/sbc_test.c
ccflags-y += -Imultimedia/inc/
ccflags-y += -DSBC_TEST
SBC_REDUCE_SIZE := 1
ifeq ($(SBC_REDUCE_SIZE),1)
ccflags-y += -DSBC_REDUCE_SIZE
endif
ifeq ($(SBC_CURRENT_TEST),1)
ccflags-y += -DSBC_CURRENT_TEST
endif
endif

ifeq ($(SENSOR_TEST),1)
obj-y += ../../../tests/sensor_test/
obj-y += ../../../platform/drivers/ana/
ccflags-y += -DSENSOR_TEST
ccflags-y += -Itests/sensor_test
endif #SENSOR_TEST

ifeq ($(CP_SUBSYS_HWTEST),1)
ccflags-y += -DHWTEST -Itests/hwtest
endif

ifeq ($(SLEEP_TEST),1)
CFLAGS_cp_subsys_main.o += -DSLEEP_TEST
ifeq ($(NO_TIMER),1)
CFLAGS_cp_subsys_main.o += -DNO_TIMER
endif
endif

ifeq ($(DMA_AUDIO_APP),1)
obj-y += \
	../../../platform/drivers/stream_dma_rpc/ \
	../../../services/dma_audio/dsp/

ccflags-y += -DDMA_AUDIO_APP -Iservices/audioflinger -Iservices/dma_audio/dsp -Iservices/dma_audio/common
ifeq ($(DMA_AUDIO_APP_DYN_ON),1)
ccflags-y += -DDMA_AUDIO_APP_DYN_ON
endif
endif

ifneq ($(APP_MCPP_SRV),)
ccflags-y += \
	-Iservices/mcpp \
	-Iutils/kfifo
endif

ifneq ($(DEBUG_PORT),)
CFLAGS_cp_subsys_main.o += -DDEBUG_PORT=$(DEBUG_PORT)
endif

ifeq ($(SUBSYS_FLASH_BOOT),1)
CFLAGS_cp_subsys_main.o += -DSUBSYS_FLASH_BOOT
endif

ifeq ($(NO_FLASH_INIT),1)
CFLAGS_cp_subsys_main.o += -DNO_FLASH_INIT
endif

ifeq ($(BECO_TEST),1)
obj-y += ../../../tests/hwtest/beco_test/ \
		 ../../../platform/beco/libbeco/
subdir-ccflags-y += -DBECO_TEST -Iinclude/beco -Iinclude/cmsis_dsp \
					-Iinclude/cmsis_nn -Itests/hwtest/beco_test/  \
					-Iinclude/beco/beco_nn
endif

ifeq ($(APP_RPC_MCU_CP_EN), 1)
ccflags-y += -Iapps/app_rpc
endif

ifeq ($(UTILS_ESHELL_EN),1)
ccflags-y += -Iutils/eshell
ccflags-y += -Iapps/app_eshell
endif

ifeq ($(FLASH_NV_FACTORY),1)
ccflags-y += -Iservices/nv_section/factory_section
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ifneq ($(TEST_HAL_PATTERN),)
subdir-ccflags-y += -DTEST_HAL_PATTERN
endif
