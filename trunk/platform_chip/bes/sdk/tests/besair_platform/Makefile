cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj_s := startup_main.S

obj-y := $(obj_c:.c=.o) $(obj_cpp:.cpp=.o) $(obj_s:.S=.o)

obj-y += bsp_tester/

ccflags-y := \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iapps/audioplayers \
	-Iapps/key \
	-Iapps/common \
	-Iapps/main	\
	-Iapps/sensorhub \
	-Iapps/bth_if/bt_if_client \
	-Iapps/bth_if \
	-Iapps/app_rpc \
	-Iapps/dsp_hifi4 \
	-Iutils/boot_struct \
	-Iutils/cqueue \
	-Iutils/hwtimer_list \
	-Iutils/list \
	-Iutils/heap \
	-Iutils/hsm \
	-Iutils/hexdump	\
	-Iutils/crc32 \
	-Iplatform/hal/$(CHIP) \
	-Iplatform/drivers/codec \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/bt \
	-Iplatform/drivers/dsp_m55 \
	-Iplatform/drivers/rmt_trace \
	-Iplatform/drivers/hifi4 \
	-Iplatform/drivers/sensor_hub \
	-Iplatform/drivers/bt_host \
	-Iservices/bt_app \
	-Iservices/audioflinger \
	-Imultimedia/inc/codec \
	-Imultimedia/inc/speech \
	-Imultimedia/inc/audio \
	-Imultimedia/inc/audio/codec/sbc \
	-Imultimedia/inc/audio/codec/sbc/src \
	-Imultimedia/audio/codec/sbc/src/inc \
	-Iservices/bt_app/a2dp_codecs/include \
	-Iservices/bt_if_enhanced/me \
	-Itests/besair_platform/bsp_tester \
	-Itests/besair_platform/cmd_test \
	-Itests/besair_platform/bsp_tester/coremark \
	-Itests/besair_platform/bsp_tester/audio_tester \
	-Itests/besair_platform/bsp_tester/bt_tester \
	-Itests/besair_platform/bsp_tester/flash_tester \
	-Itests/besair_platform/bsp_tester/gpio_tester \
	-Itests/besair_platform/bsp_tester/i2c_tester \
	-Itests/besair_platform/bsp_tester/spi_tester \
	-Itests/besair_platform/bsp_tester/wdt_tester \
	-Itests/besair_platform/bsp_tester/common_debug_interface \
	-Itests/besair_platform/subsys_m55_trc \
	-Iapps/dsp_m55

ifeq ($(OPENAMP_CONTROLLER), 1)
ccflags-y += -Iservices\openamp\rptun\chips
endif

ifeq ($(APP_RPC_ENABLE), 1)
ccflags-y += -Iapps/app_rpc
endif

ifneq ($(TEST_HAL_PATTERN),)
ccflags-y += -DTEST_HAL_PATTERN
endif

ifeq ($(UTILS_ESHELL_EN), 1)
ccflags-y += -Iutils/eshell
ccflags-y += -Iapps/app_eshell
endif

ifneq ($(DEBUG_PORT),)
ccflags-y += -DDEBUG_PORT=$(DEBUG_PORT)
endif

ifeq ($(APP_TRACE_RX_ENABLE),1)
ccflags-y += -DAPP_TRACE_RX_ENABLE
endif

ifeq ($(BOOT_LOADER_ENTRY_HOOK),1)
asflags-y += -DBOOT_LOADER_ENTRY_HOOK
endif

ifeq ($(COREMARK_TEST),1)
ccflags-y += -DCOREMARK_TEST
ifeq ($(IS_NOT_LOCK_IRQ_DURING_COREMARK_TEST),1)
KBUILD_CPPFLAGS += -DIS_NOT_LOCK_IRQ_DURING_COREMARK_TEST
endif
endif

ifeq ($(BT_CONTROLLER_TEST), 1)
obj-y += ../../platform/drivers/bt_controller/
KBUILD_CPPFLAGS += -DBT_CONTROLLER_TEST
ccflags-y += -Iplatform/drivers/bt_controller
endif

ifeq ($(HIFI4_ADDA_LOOP_TEST),1)
obj-y += bsp_tester/codec_interface/
ccflags-y += -Itests/besair_platform/bsp_tester/codec_interface/
CFLAGS_besair_main.o += -DHIFI4_ADDA_LOOP_TEST
endif

ifeq ($(TEST_CAPSENSE_SPI),1)
obj-y += ../../tests/besair_platform/bsp_tester/cap_sensor_interface/
ccflags-y += -Itests/besair_platform/bsp_tester/cap_sensor_interface/
CFLAGS_besair_main.o += -DTEST_CAPSENSE_SPI
endif

ifeq ($(HW_DAC_IIR_EQ),1)
obj-y += bsp_tester/dac_eq_interface/
ccflags-y += -Itests//besair_platform/bsp_tester/dac_eq_interface/
CFLAGS_besair_main.o += -DHW_DAC_IIR_EQ
endif

ifeq ($(DSP_TEST),1)
obj-y += ../../apps/dsp_hifi4/
endif

ifeq ($(DSP_M55_TRC_TO_MCU),1)
obj-y += ../../tests/dsp_m55/dsp_m55_trc_to_mcu.o
TRC_FLAGS := -Iplatform/drivers/rmt_trace -Itests/dsp_m55 -DDSP_M55_TRC_TO_MCU
ifeq ($(RMT_TRC_IN_MSG_CHAN),1)
TRC_FLAGS += -DRMT_TRC_IN_MSG_CHAN
endif
CFLAGS_besair_main.o += $(TRC_FLAGS)
CFLAGS_dsp_m55_trc_to_mcu.o += $(TRC_FLAGS)
endif

ifeq ($(FORCE_SIGNALINGMODE),1)
CFLAGS_besair_main.o += -DFORCE_SIGNALINGMODE
endif

ifeq ($(SENSOR_HUB_BSP_TEST),1)
ccflags-y += -DSENSOR_HUB_BSP_TEST
endif

ifeq ($(BTH_IF_COMM_RPC_EN), 1)
ccflags-y += -Iapps/bth_if
ccflags-y += -Iapps/bth_if/bt_if_client
endif

ifeq ($(POWER_TEST),1)
ccflags-y += -DPOWER_TEST
endif

ifeq ($(COREMARK_TEST), 1)
ccflags-y += -Iapps/app_eshell/powertest
endif
