cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += -Iplatform/hal
ccflags-y += -Iplatform/drivers/ana
ccflags-y += -Iservices/audioflinger
ccflags-y += -Iutils/cqueue
ccflags-y += -Iutils/hwtimer_list

ifeq ($(CHIP_FUNCTION_TEST_HWTIMER),1)

ccflags-y += -DCHIP_FUNCTION_TEST_HWTIMER

ifeq ($(OUTPUT_SQUARE_WAVE),1)
ccflags-y += -DOUTPUT_SQUARE_WAVE
export  TEST_TIMER_IND ?= 1
ccflags-y += -DTEST_TIMER_IND=$(TEST_TIMER_IND)
endif

ifeq ($(CALIB_SLOW_TIMER_WITH_SLEEP),1)
ccflags-y += -DCALIB_SLOW_TIMER_WITH_SLEEP
endif

ifeq ($(CALIB_SLOW_TIMER_CALIB_IN_THREAD),1)
ccflags-y += -DCALIB_SLOW_TIMER_CALIB_IN_THREAD
endif

ifeq ($(FAST_TIMER_COMPENSATE),1)
ccflags-y += -DFAST_TIMER_COMPENSATE
endif

ifeq ($(HWTIMER_TEST_ACCURACY),1)
ccflags-y += -DHWTIMER_TEST_ACCURACY
endif

export TIMER_TEST_PERIOD_US ?= 100000
ccflags-y += -DTIMER_TEST_PERIOD_US=$(TIMER_TEST_PERIOD_US)

export TIMER_TEST_MS ?= 60000
ccflags-y += -DTIMER_TEST_MS=$(TIMER_TEST_MS)

export TIMER_CALIB_INTERVAL_TIME_1 ?= 500
ccflags-y += -DTIMER_CALIB_INTERVAL_TIME_1=$(TIMER_CALIB_INTERVAL_TIME_1)

export TIMER_CALIB_INTERVAL_TIME_2 ?= 100000
ccflags-y += -DTIMER_CALIB_INTERVAL_TIME_2=$(TIMER_CALIB_INTERVAL_TIME_2)

endif

ifeq ($(CHIP_FUNCTION_TEST_RTC),1)
ccflags-y += -DCHIP_FUNCTION_TEST_RTC
ccflags-y += -DRTC_ENABLE
endif

ifeq ($(CHIP_FUNCTION_TEST_I2S),1)
obj-y += ../../utils/cqueue/
obj-y += ../../services/audioflinger/
obj-y += ../../platform/drivers/codec/
ccflags-y += -DCHIP_FUNCTION_TEST_I2S
#obj-y += ../codec_vad/i2s_audio/
#obj-y += ../anc_usb/safe_queue.o
#ccflags-y +=-Itests/codec_vad/i2s_audio

ifeq ($(AD_I2S_LOOP_TEST),1)
ccflags-y += -DAD_I2S_LOOP_TEST
endif

ifeq ($(ADC_DAC_LOOP_TEST),1)
ccflags-y += -DADC_DAC_LOOP_TEST
endif

ifeq ($(ADC_I2S_LOOP_MASTER_TEST),1)
ccflags-y += -DADC_I2S_LOOP_MASTER_TEST
endif

ifeq ($(I2S_DAC_LOOP_MASTER_TEST),1)
ccflags-y += -DI2S_DAC_LOOP_MASTER_TEST
endif

ifeq ($(I2S_I2S_LOOP_MASTER_TEST),1)
ccflags-y += -DI2S_I2S_LOOP_MASTER_TEST
endif

ifeq ($(ADC_I2S_LOOP_SLAVER_TEST),1)
ccflags-y += -DADC_I2S_LOOP_SLAVER_TEST
endif

ifeq ($(I2S_DAC_LOOP_SLAVER_TEST),1)
ccflags-y += -DI2S_DAC_LOOP_SLAVER_TEST
endif

ifeq ($(I2S_I2S_LOOP_SLAVER_TEST),1)
ccflags-y += -DI2S_I2S_LOOP_SLAVER_TEST
endif

ifeq ($(ADC_TDM_LOOP_MASTER_TEST),1)
ccflags-y += -DADC_TDM_LOOP_MASTER_TEST
endif

ifeq ($(TDM_DAC_LOOP_MASTER_TEST),1)
ccflags-y += -DTDM_DAC_LOOP_MASTER_TEST
endif

ifeq ($(TDM_TDM_LOOP_MASTER_TEST),1)
ccflags-y += -DTDM_TDM_LOOP_MASTER_TEST
endif

ifeq ($(ADC_TDM_LOOP_SLAVER_TEST),1)
ccflags-y += -DADC_TDM_LOOP_SLAVER_TEST
endif

ifeq ($(TDM_DAC_LOOP_SLAVER_TEST),1)
ccflags-y += -DTDM_DAC_LOOP_SLAVER_TEST
endif

ifeq ($(TDM_TDM_LOOP_SLAVER_TEST),1)
ccflags-y += -DTDM_TDM_LOOP_SLAVER_TEST
endif

endif

ifeq ($(CHIP_FUNCTION_TEST_DMA),1)
ccflags-y += -DCHIP_FUNCTION_TEST_DMA
endif

ifeq ($(CHIP_FUNCTION_TEST_NORFLASH),1)

ccflags-y += -DCHIP_FUNCTION_TEST_NORFLASH
ccflags-y += -Iplatform/drivers/norflash
ccflags-y += -Iservices/norflash_api
obj-y += ../../services/norflash_api/

ifeq ($(FLASH_PROTECTION),1)
CFLAGS_chip_function_test_norflash.o += -DFLASH_PROTECTION
endif

ifeq ($(PUYA_FLASH_ERASE_PAGE_ENABLE),1)
CFLAGS_chip_function_test_norflash.o += -DPUYA_FLASH_ERASE_PAGE_ENABLE
endif

ifeq ($(FLASH_UNIQUE_ID),1)
ccflags-y += -DFLASH_UNIQUE_ID
endif

ifeq ($(FLASH_REMAP_TEST),1)
CFLAGS_chip_function_test_norflash.o += -DFLASH_REMAP
endif

ifeq ($(FLASH_PROTECTION_TEST),1)
CFLAGS_chip_function_test_norflash.o += -DFLASH_PROTECTION_TEST
endif

ifneq ($(OTA_CODE_OFFSET),)
CFLAGS_chip_function_test_norflash.o += -DOTA_CODE_OFFSET=$(OTA_CODE_OFFSET)
endif

ifneq ($(FLASH_ERASE_CURRENT_TEST),)
CFLAGS_chip_function_test_norflash.o += -DFLASH_ERASE_CURRENT_TEST
endif

ifneq ($(FLASH_PROGRAM_CURRENT_TEST),)
CFLAGS_chip_function_test_norflash.o += -DFLASH_PROGRAM_CURRENT_TEST
endif

ifneq ($(FLASH_READ_CURRENT_TEST),)
CFLAGS_chip_function_test_norflash.o += -DFLASH_READ_CURRENT_TEST
endif

ifneq ($(FLASH_IDLE_CURRENT_TEST),)
CFLAGS_chip_function_test_norflash.o += -DFLASH_IDLE_CURRENT_TEST
endif

ifneq ($(FLASH_LIFE_TEST_OFFSET),)
CFLAGS_chip_function_test_norflash.o += -DFLASH_LIFE_TEST_OFFSET=$(FLASH_LIFE_TEST_OFFSET)
endif

ifneq ($(FLASH_LIFE_TEST_SIZE),)
CFLAGS_chip_function_test_norflash.o += -DFLASH_LIFE_TEST_SIZE=$(FLASH_LIFE_TEST_SIZE)
endif

ifeq ($(FLASH_SIMPLE_TEST),1)
CFLAGS_chip_function_test_norflash.o += -DFLASH_SIMPLE_TEST
endif

ifeq ($(NO_FLASH1_ADDR_ACCESS),1)
    CFLAGS_chip_function_test_norflash.o += -DNO_FLASH1_ADDR_ACCESS
endif

ifeq ($(FLASH_SECURITY_REGISTER_TEST),1)
    CFLAGS_chip_function_test_norflash.o += -DFLASH_SECURITY_REGISTER_TEST
endif

ifeq ($(FLASH_SECURITY_REGISTER),1)
    CFLAGS_chip_function_test_norflash.o += -DFLASH_SECURITY_REGISTER
endif

ifeq ($(FLASH_SET_DUAL_CHIP_TEST),1)
    CFLAGS_chip_function_test_norflash.o += -DFLASH_SET_DUAL_CHIP_TEST
endif

ifeq ($(FLASH_RANDOM_WRITE_TEST),1)
    CFLAGS_chip_function_test_norflash.o += -DFLASH_RANDOM_WRITE_TEST
endif

ifeq ($(FLASH_RANDOM_READ_TEST),1)
    CFLAGS_chip_function_test_norflash.o += -DFLASH_RANDOM_READ_TEST
ifneq ($(RANDOM_READ_START),)
    CFLAGS_chip_function_test_norflash.o += -DRANDOM_READ_START=$(RANDOM_READ_START)
endif

ifneq ($(RANDOM_READ_LEN),)
    CFLAGS_chip_function_test_norflash.o += -DRANDOM_READ_LEN=$(RANDOM_READ_LEN)
endif

ifneq ($(BAK_RANDOM_READ_START),)
    CFLAGS_chip_function_test_norflash.o += -DBAK_RANDOM_READ_START=$(BAK_RANDOM_READ_START)
endif
ifneq ($(RAND_READ_REBOOT_COUNT),)
    CFLAGS_chip_function_test_norflash.o += -DRAND_READ_REBOOT_COUNT=$(RAND_READ_REBOOT_COUNT)
endif
endif
endif

ifneq ($(OTA_REMAP_OFFSET),)
CFLAGS_chip_function_test_norflash.o += -DOTA_REMAP_OFFSET=$(OTA_REMAP_OFFSET)
endif
ifeq ($(CHIP_FUNCTION_TEST_POWER_RESET),1)
ccflags-y += -DCHIP_FUNCTION_TEST_POWER_RESET
endif

ifeq ($(CHIP_FUNCTION_TEST_GPIO),1)
ccflags-y += -DCHIP_FUNCTION_TEST_GPIO
ifeq ($(GPIO_IRQ_TEST),1)
ccflags-y += -DGPIO_IRQ_TEST
endif
ifeq ($(TEST_GPIO_IN_05),1)
ccflags-y += -DTEST_GPIO_IN_05
endif
ifeq ($(TEST_GPIO_OUT_07),1)
ccflags-y += -DTEST_GPIO_OUT_07
endif
ifeq ($(SQUARE_WAVE_OUTPUT),1)
ccflags-y += -DSQUARE_WAVE_OUTPUT
endif
ifeq ($(GPIO_IRQ_WAKEUP_TEST),1)
ccflags-y += -DGPIO_IRQ_WAKEUP_TEST
endif
ifneq ($(SQUARE_WAVE_TIME_CYCLE_MS),)
ccflags-y += -DSQUARE_WAVE_TIME_CYCLE_MS=$(SQUARE_WAVE_TIME_CYCLE_MS)
endif
ifeq ($(SQUARE_WAVE_OUTPUT_ONCE),1)
ccflags-y += -DSSQUARE_WAVE_OUTPUT_ONCE
endif
ifeq ($(IRQ_WAKEUP_TIMER0_IRQ0),1)
ccflags-y += -DIRQ_WAKEUP_TIMER0_IRQ0
endif
ifeq ($(IRQ_WAKEUP_TIMER0_IRQ1),1)
ccflags-y += -DIRQ_WAKEUP_TIMER0_IRQ1
endif

ifeq ($(IRQ_WAKEUP_GPIO_82_IRQ),1)
ccflags-y += -DIRQ_WAKEUP_GPIO_82_IRQ
endif
ifeq ($(IRQ_WAKEUP_GPIO_04_IRQ),1)
ccflags-y += -DIRQ_WAKEUP_GPIO_04_IRQ
endif

ifeq ($(IRQ_WAKEUP_GPIO_83_IRQ),1)
ccflags-y += -DIRQ_WAKEUP_GPIO_83_IRQ
endif
ifeq ($(IRQ_WAKEUP_GPIO_05_IRQ),1)
ccflags-y += -DIRQ_WAKEUP_GPIO_05_IRQ
endif

ifeq ($(IRQ_WAKEUP_GPIO_84_IRQ),1)
ccflags-y += -DIRQ_WAKEUP_GPIO_84_IRQ
endif
ifeq ($(IRQ_WAKEUP_GPIO_85_IRQ),1)
ccflags-y += -DIRQ_WAKEUP_GPIO_85_IRQ
endif

ifeq ($(IRQ_WAKEUP_GPIO_86_IRQ),1)
ccflags-y += -DIRQ_WAKEUP_GPIO_86_IRQ
endif
ifeq ($(IRQ_WAKEUP_GPIO_87_IRQ),1)
ccflags-y += -DIRQ_WAKEUP_GPIO_87_IRQ
endif
endif

ifeq ($(CHIP_FUNCTION_TEST_SAM),1)
ccflags-y += -DCHIP_FUNCTION_TEST_SAM
SAM_UART_BANDRATE ?= 19200
ccflags-y += -DSAM_UART_BANDRATE=$(SAM_UART_BANDRATE)

ifneq ($(UART_PORT_ID),)
ccflags-y += -DUART_PORT_ID=$(UART_PORT_ID)
endif
ifeq ($(SAM_UART_ENABLE_DMA),1)
ccflags-y += -DSAM_UART_ENABLE_DMA=$(SAM_UART_ENABLE_DMA)
endif
endif

ifeq ($(CHIP_FUNCTION_TEST_CURRENT),1)
ccflags-y += -DCHIP_FUNCTION_TEST_CURRENT
endif

ifeq ($(FUNCTION_TIMER_TEST),1)
ccflags-y += -DTIMER_TEST
endif

ifeq ($(CHIP_FUNCTION_TEST_SDMMC),1)
ccflags-y += -DCHIP_FUNCTION_TEST_SDMMC
ifeq ($(SDMMC_FULL_TEST),1)
    CFLAGS_chip_function_test_sdmmc.o += -DSDMMC_FULL_TEST
else
    SDMMC_TEST_BLOCK_COUNT ?= 64
    CFLAGS_chip_function_test_sdmmc.o += -DSDMMC_TEST_BLOCK_COUNT=$(SDMMC_TEST_BLOCK_COUNT)
endif
ifeq ($(SDMMC_TEST_DATA_RESTORE),1)
    CFLAGS_chip_function_test_sdmmc.o += -DSDMMC_TEST_DATA_RESTORE
endif
endif

ifeq ($(CHIP_FUNCTION_TEST_I2C),1)
ccflags-y += -DCHIP_FUNCTION_TEST_I2C
I2C_TEST_I2C_ID ?= 1
CFLAGS_chip_function_test_i2c.o += -DI2C_TEST_I2C_ID=$(I2C_TEST_I2C_ID)
endif

ifeq ($(CURRENT_TEST),1)
# cp to noapp/target.mk
CURRENT_TEST ?= 1
export STRESS_WORKAROUND ?= 0
export CORE_SLEEP_POWER_DOWN ?= 1

export SENSOR_HUB_TEST ?= 0
export SENSOR_HUB_MINIMA	?= 0
export RC48M_ENABLE ?= 0
export FORCE_SENS_RC_CLOCK ?= 0
export FORCE__RC_CLOCK ?= 0
export FORCE_XTAL_OFF ?= 0
endif

ifeq ($(CHIP_FUNCTION_TEST_CURRENT),1)
ifneq ($(CURRENT_TEST_SYSFREQ),)
ccflags-y += -DCURRENT_TEST_SYSFREQ=$(CURRENT_TEST_SYSFREQ)
endif

ifeq ($(WFI_TEST),1)
ccflags-y += -DWFI_TEST
endif

ifeq ($(WHILE1_TEST),1)
ccflags-y += -DWHILE1_TEST
endif
endif
