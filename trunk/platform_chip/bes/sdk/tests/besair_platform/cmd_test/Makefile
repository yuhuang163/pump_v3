cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o)

ccflags-y := \
	-Iapps/common \
	-Iapps/sensorhub \
	-Itests/besair_platform/bsp_tester/audio_tester \
	-Itests/besair_platform/bsp_tester/bt_tester \
	-Itests/besair_platform/bsp_tester/flash_tester \
	-Itests/besair_platform/bsp_tester/gpio_tester \
	-Itests/besair_platform/bsp_tester/i2c_tester \
	-Itests/besair_platform/bsp_tester/spi_tester \
	-Itests/besair_platform/bsp_tester/wdt_tester \
	-Itests/besair_platform/bsp_tester/common_debug_interface \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/bt

ifeq ($(SENSOR_HUB_BSP_TEST),1)
ccflags-y += -DSENSOR_HUB_BSP_TEST
endif