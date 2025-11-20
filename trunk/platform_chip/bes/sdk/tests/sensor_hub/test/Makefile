cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

ccflags-y := -Iapps/sensorhub \
	-Iplatform/hal \
	-Iplatform/cmsis \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/sensor_hub \
	-Iservices/audioflinger \
	-Itests/hwtest \
	-Itests/sensor_hub/inc \
	-Itests/sensor_hub/core \
	-Itests/sensor_hub/vad \
	-Iutils/hwtimer_list \
	-Iservices/openamp/open-amp/apps/machine/bes\
	-Iservices/openamp/open-amp/apps/examples/bes_echo_bth2sens\

ifeq ($(BECO_TEST), 1)
obj-y += ../../../platform/beco/
obj-y += ../../hwtest/beco_test/
subdir-ccflags-y += -DBECO_TEST
endif

ifneq ($(TEST_HAL_PATTERN),)
obj-y += ../../../tests/besair_platform/bsp_tester/
ccflags-y += -DTEST_HAL_PATTERN -Itests/besair_platform/bsp_tester -Itests/besair_platform/bsp_tester/dhrystone2
ccflags-y += -D$(TEST_HAL_PATTERN)
endif

ifeq ($(BT_CONN_TEST),1)
obj-y += ../../../tests/hwtest/bt_conn_test.c
ccflags-y += -DBT_CONN_TEST
endif

ifeq ($(CHIP_FUNCTION_TEST),1)
obj-y += ../../../tests/chip_function_test/
ccflags-y +=-Itests/chip_function_test
ccflags-y += -DCHIP_FUNCTION_TEST
endif

ifeq ($(COREMARK_TEST),1)
obj-y += ../../../tests/coremark/
ccflags-y += -Itests/coremark
ccflags-y += -DCOREMARK_TEST
ifeq ($(APP_COREMARK_TEST),1)
ccflags-y += -DAPP_COREMARK_TEST
endif
ifeq ($(IS_NOT_LOCK_IRQ_DURING_COREMARK_TEST),1)
KBUILD_CPPFLAGS += -DIS_NOT_LOCK_IRQ_DURING_COREMARK_TEST
endif
endif

ifeq ($(DHRYSTONE_TEST),1)
obj-y += ../../../tests/dhrystone2/
ccflags-y += -DDHRYSTONE_TEST
endif

ifeq ($(I2C_TEST),1)
obj-y += ../../../tests/hwtest/i2c_test.c
ccflags-y += -DI2C_TEST
endif

ifeq ($(I2S_TEST),1)
obj-y += ../../../tests/hwtest/i2s_test.c
ccflags-y += -DI2S_TEST
endif

ifeq ($(CPU_MAX_POWER_TEST),1)
obj-y += ../../../tests/cpu_max_power/
ccflags-y += -DCPU_MAX_POWER_TEST
endif

ifeq ($(MCU2SENS_MSG_TEST),1)
obj-y += ../../../tests/hwtest/rmt_msg_test.c
ccflags-y += -DMCU2SENS_MSG_TEST
endif

ifeq ($(MINIMA_TEST),1)
ccflags-y += -DMINIMA_TEST -Itests/minima_test
endif

ifeq ($(PT_APP),1)
obj-y += ../../../tests/mem_test/cases/pt_app/
ccflags-y += -DPT_APP -Itests/mem_test/cases/pt_app
endif

ifeq ($(SBC_TEST),1)
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

ifeq ($(SENSOR_ENGINE_TEST),1)
obj-y += ../../../tests/hwtest/sensor_engine_test.c
ccflags-y += -DSENSOR_ENGINE_TEST
endif

ifeq ($(SENSOR_HUB_MISC_TEST),1)
ccflags-y += -DSENSOR_HUB_MISC_TEST
endif

ifeq ($(SENSOR_TEST),1)
ccflags-y += -DSENSOR_TEST -Itests/sensor_test
endif

ifeq ($(SLEEP_TEST),1)
ccflags-y += -DSLEEP_TEST
ifeq ($(NO_TIMER),1)
ccflags-y += -DNO_TIMER
endif
endif

ifeq ($(UART_TEST),1)
obj-y += ../../../utils/crc32/
obj-y += ../../../tests/hwtest/uart_test.c
ccflags-y += -Iutils/crc32
ccflags-y += -DUART_TEST
endif

ifeq ($(VD_TEST),1)
ccflags-y += -DVD_TEST -Iapps/voice_detector
endif

ifeq ($(VOICE_DETECTOR_EN),1)
ccflags-y += -DVOICE_DETECTOR_EN
endif

ifeq ($(VAD_CODEC_TEST),1)
ccflags-y += -DVAD_CODEC_TEST
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)
