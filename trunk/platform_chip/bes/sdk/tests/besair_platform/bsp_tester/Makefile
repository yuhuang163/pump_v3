cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iutils/boot_struct \
	-Iutils/cqueue \
	-Iutils/hwtimer_list \
	-Iservices/audioflinger \
	-Iplatform/drivers/codec \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/ana/$(CHIP) \
	-Imultimedia/inc/codec \
	-Imultimedia/inc/speech \
	-Imultimedia/inc/audio \
	-Imultimedia/inc/audio/codec/sbc \
	-Imultimedia/inc/audio/codec/sbc/src \
	-Imultimedia/audio/codec/sbc/src/inc \
	-Iservices/bt_app/a2dp_codecs/include \
	-Iservices/bt_if_enhanced/me \
	-Iservices/openamp/open-amp/apps/machine/bes\
	-Iservices/openamp/open-amp/apps/examples/bes_echo_bth2sens\
	-Iplatform/drivers/bt \
	-Iplatform/drivers/rmt_trace \
	-Iplatform/drivers/hifi4 \
	-Iplatform/drivers/sensor_hub \
	-Iutils/list \
	-Iutils/heap \
	-Iutils/hsm \
	-Iutils/hexdump	\
	-Iutils/crc32 \
	-Iapps/audioplayers \
	-Iapps/key \
	-Iapps/common \
	-Iapps/main	\
	-Iinclude/open-amp \
	-Iservices/nv_section/log_section \
	-Iplatform/drivers/cp_subsys \
	-Ithirdparty/lvgl/bes/ports

ifneq ($(DEBUG_PORT),)
ccflags-y += -DDEBUG_PORT=$(DEBUG_PORT)
endif

ifeq ($(BOOT_LOADER_ENTRY_HOOK),1)
asflags-y += -DBOOT_LOADER_ENTRY_HOOK
endif

ifeq ($(BT_CONTROLLER_TEST), 1)
obj-y += ../../../platform/drivers/bt_controller/
KBUILD_CPPFLAGS += -DBT_CONTROLLER_TEST
ccflags-y += -Iplatform/drivers/bt_controller
endif

ifeq ($(HIFI4_ADDA_LOOP_TEST),1)
obj-y += ../../../tests/besair_platform/bsp_tester/codec_interface/
ccflags-y += -Itests/besair_platform/bsp_tester/codec_interface/
CFLAGS_besair_main.o += -DHIFI4_ADDA_LOOP_TEST
endif

ccflags-y += -Itests/besair_platform/bsp_tester/dhrystone2/

ifneq ($(filter TEST_OPENAMP,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_OPENAMP
endif
ifneq ($(filter TEST_UART,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_UART
endif
ifneq ($(filter TEST_OSTICK,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_OSTICK
endif
ifneq ($(filter TEST_WDT,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_WDT
endif
ifneq ($(filter TEST_TIMER,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_TIMER
endif
ifneq ($(filter TEST_RTC,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_RTC
endif
ifneq ($(filter TEST_GPIO,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_GPIO
ccflags-y += -Iplatform/drivers/ana/best1600
endif
ifneq ($(filter TEST_I2C,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_I2C

ifneq ($(TEST_I2C_ID),"")

#CFLAGS_test_i2c.o += -DI2C_DEBUG
KBUILD_CPPFLAGS += -DTEST_I2C_ID=$(TEST_I2C_ID)
endif
endif

ifneq ($(filter TEST_SPI,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_SPI
ifneq ($(TEST_SPI_ID),)
CFLAGS_test_spi.o += -DTEST_SPI_ID=$(TEST_SPI_ID)
endif
ifneq ($(TEST_SPI_RATE),)
CFLAGS_test_spi.o += -DTEST_SPI_RATE=$(TEST_SPI_RATE)
endif
endif

ifneq ($(filter TEST_PWM,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_PWM
ifneq ($(TEST_PWM_FREQ),)
CFLAGS_test_pwm.o += -DTEST_PWM_FREQ=$(TEST_PWM_FREQ)
endif
ifneq ($(TEST_PWM_RATIO),)
CFLAGS_test_pwm.o += -DTEST_PWM_RATIO=$(TEST_PWM_RATIO)
endif
ifneq ($(TEST_PWM_CLK_SRC),)
CFLAGS_test_pwm.o += -DTEST_PWM_CLK_SRC=$(TEST_PWM_CLK_SRC)
endif
endif
ifneq ($(filter TEST_TRNG,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_TRNG
ifeq ($(TEST_RANDOM),1)
obj-y += ../../../platform/drivers/random/
export HW_RANDOM ?= 1
CFLAGS_test_trng.o += -DTEST_RANDOM -Iplatform/drivers/random
endif
endif
ifneq ($(filter TEST_SEC_ENG,$(TEST_HAL_PATTERN)),)
obj-y += ../../../utils/hexdump/
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_SEC_ENG
endif

ifneq ($(filter TEST_GPADC,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_GPADC
endif

ifneq ($(filter TEST_NORFLASH,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_NORFLASH
endif

ifneq ($(filter TEST_VMIC,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_VMIC
endif

ifneq ($(filter TEST_DMA,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_DMA
endif

ifneq ($(filter TEST_I2S,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_I2S
endif

ifneq ($(filter TEST_SDIO_HOST_SIGNAL,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_SDIO_HOST_SIGNAL
endif

ifneq ($(filter TEST_SDIO_HOST,$(TEST_HAL_PATTERN)),)
obj-y += ../../hwtest/sdmmc_test.o
obj-y += ../../hwtest/sdio_host_test.o
ccflags-y += -DSDIO_HOST_TEST -Itests/hwtest
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_SDIO_HOST -DSDMMC_TEST
endif

ifneq ($(filter TEST_PSRAM,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_PSRAM
endif

ifneq ($(filter TEST_EMMC,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_EMMC -DEMMC_CLOCK_OUTPUT
obj-y += ../../../platform/drivers/emmc/host/test/
subdir-ccflags-y += \
    -Iplatform/drivers/emmc/host/inc \
    -Iplatform/drivers/emmc/host/core \
    -Iplatform/drivers/emmc/host/hci \
    -Iplatform/drivers/emmc/host/card \
    -Iplatform/drivers/emmc/host/test
endif

ifneq ($(filter TEST_DISPLAY,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_DISPLAY
ENABLE_DISPLAY_DRIVERS := y
endif

ifneq ($(filter TEST_LVGL,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_LVGL
ENABLE_DISPLAY_DRIVERS := y
endif

ifneq ($(filter TEST_BLEND,$(TEST_HAL_PATTERN)),)
subdir-ccflags-y += -Iutils/blendutils/
obj-y += ../../../utils/blendutils/
endif

ifneq ($(filter TEST_ARM2D TEST_DMA2D TEST_GPU TEST_LCDC TEST_BLEND TEST_FB,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -D$(TEST_HAL_PATTERN)
subdir-ccflags-y += -DDISPLAY_TEST_LIB
obj-y += ../../display_test/
ENABLE_DISPLAY_DRIVERS := y
subdir-ccflags-y += -Itests/display_test
endif

ifeq ($(ENABLE_DISPLAY_DRIVERS),y)
subdir-ccflags-y += -DTEST_WITH_GPU
subdir-ccflags-y += -D_BAREMETAL=1
subdir-ccflags-y += -DTEST_WITH_LCD
subdir-ccflags-y += -DGRAPHICDRV_TIME_STATIS
subdir-ccflags-y += -DPSRAM_FB
subdir-ccflags-y += \
				   -Iutils/heap/ \
				   -Iplatform/drivers/graphic \
				   -Iplatform/drivers/graphic/fb \
				   -Iplatform/drivers/graphic/lcdc \
				   -Iplatform/drivers/graphic/lcd \
				   -Iplatform/drivers/graphic/dma2d \
				   -Iplatform/drivers/graphic/gpu \
				   -Iplatform/drivers/graphic/gpu/drv/vglite/inc \
				   -Iplatform/drivers/graphic/gpu/drv/vglite/VGLiteKernel \
				   -Iplatform/drivers/graphic/osl/ \
				   -Irtos/ \
				   -Irtos/rtx5/rtx_config/
endif


ifneq ($(filter TEST_TMA525C,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_TMA525C
endif

ifneq ($(filter TEST_DHRY,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_DHRY
#obj-y += dhrystone2/
obj-y += dhry_bbk/
endif

ifneq ($(filter TEST_COREMARK,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_COREMARK
obj-y += coremark/
endif

ifneq ($(filter TEST_SENSOR_ENGINE,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_SENSOR_ENGINE
endif

ifneq ($(filter TEST_SPI_SLAVE,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_SPI_SLAVE
endif

ifneq ($(filter TEST_FREQ_SW_TIME,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_FREQ_SW_TIME
endif

ifneq ($(filter TEST_MBEDTLS,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_MBEDTLS
KBUILD_CPPFLAGS += -DMBEDTLS_CONFIG_FILE='"mbedtls_ports_config.h"'
obj-y += ../../../utils/mbedtls/
subdir-ccflags-y += \
    -Iutils/mbedtls/include \
    -Iutils/mbedtls/ports/include \
    -Iutils/mbedtls/ports/tests
endif

ifneq ($(filter TEST_SHANHAI_MBEDTLS,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_SHANHAI_MBEDTLS
obj-y += shanhai_mbedtls_tester/ \
         ../../../utils/hexdump/
endif

ifneq ($(filter TEST_TIMER_BBK,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_TIMER_BBK
obj-y += cp_timer_bbk/

export TRIGGER_MODE ?= timer

ifeq ($(TRIGGER_MODE),timer)
KBUILD_CPPFLAGS += -DTIMER_TRIGGER
endif

ifeq ($(TRIGGER_MODE),gpio)
KBUILD_CPPFLAGS += -DGPIO_TRIGGER
endif
endif # TEST_TIMER_BBK

ifneq ($(filter TEST_FC,$(TEST_HAL_PATTERN)),)
obj-y += fc_test/
KBUILD_CPPFLAGS += -DTEST_FC 
subdir-ccflags-y += \
	-Itests/besair_platform/bsp_tester/fc_test/inc \
	-Iinclude/cmsis_nn \
	-Iutils/hwtimer_list \
	-Iinclude/beco \
	-Iinclude/beco/beco_nn
endif # TEST_FC

ifneq ($(filter TEST_USER_GPS_TO_FLASH,$(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_USER_GPS_TO_FLASH
subdir-ccflags-y += -Iservices/nv_section/log_section
endif

ifneq ($(filter TEST_DWT, $(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_DWT
subdir-ccflags-y += -Iplatform/cmsis/inc
endif

ifneq ($(filter TEST_MPU, $(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_MPU
subdir-ccflags-y += -Iplatform/cmsis/inc
endif

ifneq ($(filter TEST_SYSVIEW, $(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_SYSVIEW
endif

ifneq ($(filter TEST_BECO, $(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DTEST_BECO
obj-y += beco_test/
endif

ifneq ($(filter CMSIS_TEST, $(TEST_HAL_PATTERN)),)
KBUILD_CPPFLAGS += -DTEST_HAL_PATTERN -DCMSIS_TEST
ccflags-y += -Iinclude/cmsis_dsp/
ccflags-y += -Iinclude/cmsis_nn/
endif

ifeq ($(LVGL_TEST), 1)
ccflags-y += -DLVGL_TEST
endif
