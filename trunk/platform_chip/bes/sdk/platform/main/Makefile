cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

#obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y :=

ifeq ($(ROM_BUILD),1)
obj-y += ../../tests/rom/startup_ARMCM.S
else ifeq ($(CHIP_SIMULATOR),1)
obj-y += simu_main.o
else ifneq ($(SUBSYS_ONLY),1)
obj-y += startup_main.S
ifeq ($(NO_STARTFILES),1)
AFLAGS_startup_main.o += -D__NO_STARTFILES__
endif
endif

ifneq ($(CHIP_SIMULATOR),1)
obj-y += ../../utils/hwtimer_list/
endif

# Startup config
ifeq ($(BOOT_LOADER_ENTRY_HOOK),1)
asflags-y += -DBOOT_LOADER_ENTRY_HOOK
endif
ifeq ($(BOOT_LOADER_PRE_INIT_HOOK),1)
asflags-y += -DBOOT_LOADER_PRE_INIT_HOOK
ccflags-y += -DBOOT_LOADER_PRE_INIT_HOOK
endif
ifeq ($(BOOT_LOADER_POST_INIT_HOOK),1)
asflags-y += -DBOOT_LOADER_POST_INIT_HOOK
endif
ifeq ($(NO_NVIC_INIT),1)
asflags-y += -DNO_NVIC_INIT
endif
ifeq ($(NO_BOOT_INIT),1)
asflags-y += -DNO_BOOT_INIT
endif
ifeq ($(NO_SYSTEM_INIT),1)
asflags-y += -DNO_SYSTEM_INIT
endif

ifneq ($(USE_OS_MAIN),1)

ifeq ($(FACTORY_SUITE),1)
obj-y += ../../tests/factory_suite/
else ifeq ($(RELIABILITY_TEST),1)
obj-y += ../../tests/reliability_test/
else ifeq ($(NOSTD),1)
obj-y += nostd_main.c
else ifeq ($(NUTTX_BUILD),1)
obj-y += nuttx_main.cpp
ifeq ($(SENS_TRC_TO_MCU),1)
obj-y += ../../tests/sensor_hub/core/sens_trc_to_mcu.c
CFLAGS_sens_trc_to_mcu.o += -Iplatform/drivers/rmt_trace -DSENS_TRC_TO_MCU
endif
else ifeq ($(ARM_CMSE),1)
obj-y += cmse_main.c
ifeq ($(SECURE_BOOT),1)
ccflags-y += -DSECURE_BOOT
endif
else ifeq ($(CHIP_SIMULATOR), 1)
#use the simu_main.c
else ifeq ($(NOAPP),1)
obj-y += noapp_main.cpp
else ifeq ($(BT_WATCH_APP), 1)
obj-y += main_watch.cpp
else ifeq ($(PLATFORM_BES2700IL), 1)
obj-y += besair_main.c
else ifeq ($(PLATFORM_BES2710IMP), 1)
KBUILD_CPPFLAGS += -DPLATFORM_BES2710IMP
obj-y += besair_main.c
else ifeq ($(BLE_ONLY_ENABLED), 1)
obj-y += bleonly_main.c
ccflags-y += -DOTA_CODE_OFFSET=$(OTA_CODE_OFFSET)
ccflags-y += -Iutils/intersyshci/
else
obj-y += main.cpp
endif

ifeq ($(ARM_CMNS),1)
obj-y += cmse/tz_trace_ns.c
endif

ifeq ($(CHIP_FUNCTION_TEST),1)
obj-y += ../../tests/chip_function_test/
ccflags-y +=-Itests/chip_function_test
ccflags-y += -DCHIP_FUNCTION_TEST
endif

ifeq ($(NOAPP),1)
ccflags-y += -DNOAPP

ifeq ($(CHIP_HAS_PSRAM),1)
obj-y += ../../platform/drivers/psram/
endif
endif

ifeq ($(CHIP_HAS_DDR),1)
obj-y += ../../platform/drivers/ddr/
endif

ifeq ($(LOAD_LINUX),1)
ccflags-y += -DLOAD_LINUX -Iplatform/drivers/fw_linux/inc
endif

ifeq ($(NOAPP_WAIT_PWRKEY),1)
ccflags-y += -DNOAPP_WAIT_PWRKEY
endif

ifeq ($(REBOOT_TEST),1)
ccflags-y += -DREBOOT_TEST
endif

ifeq ($(LPO_TEST),1)
ccflags-y += -DLPO_TEST
endif

ifeq ($(TX_PULLING_CAL),1)
ccflags-y += -DTX_PULLING_CAL
endif

ifneq ($(filter 1,$(COMPRESS_SRAM_CODE) $(COMPRESS_PSRAM_CODE)),)
obj-y += compress/
ifeq ($(COMPRESS_SRAM_CODE),1)
subdir-ccflags-y += -DCOMPRESS_SRAM_CODE
endif
ifeq ($(COMPRESS_PSRAM_CODE),1)
subdir-ccflags-y += -DCOMPRESS_PSRAM_CODE
endif
endif

ifneq ($(filter 1,$(USB_SERIAL_TEST) $(USB_SERIAL_SPEED_TEST) $(USB_SERIAL_DIRECT_XFER_TEST) $(USB_AUDIO_TEST) $(USB_MTP_TEST) $(USB_MSD_TEST) $(I2C_TEST) $(I2S_TEST) \
					$(AF_TEST) $(VD_TEST) $(CP_TEST) $(CP_SUBSYS_TEST) $(CSP_SUBSYS_TEST) $(TDM_TEST) $(SEC_ENG_TEST) $(A7_DSP_TEST) $(A55_TEST) \
					$(TRANSQ_TEST) $(FLASH_TEST) $(PSRAM_TEST) $(PSRAMUHS_TEST) $(SPI_NORFLASH_TEST) \
					$(SENSOR_HUB_TEST) $(SENSOR_TEST) $(MBW_TEST) $(BECO_TEST) $(TZ_TEST) $(FT_TEST) \
					$(BT_CONN_TEST) $(BT_CONN_SIMU) $(MINIMA_TEST) $(LIS25BA_TEST) $(PWM_TEST) $(TRNG_TEST) $(IR_TEST) \
					$(RSA_TEST) $(DAC_TEST) $(DAC_TEST_COMBINED) $(ADC2_TEST) $(NANDFLASH_TEST) $(SDMMC_TEST) $(SDIO_HOST_TEST) $(SDIO_HOST_SIGNAL_TEST) \
					$(SDIO_HOST_FIRMWARE_TEST) $(DSI_TEST) $(CSI_TEST) $(QSPI_1501_TEST) $(I2S_HOST_TEST) \
					$(SPI_TEST) $(SDIO_DEVICE_TEST) $(UART_TEST) $(SCI_TEST) $(BT_HOST_TEST) $(DSP_M55_TEST) $(DSP_M55C1_TEST) $(DSP_HIFI4_TEST)\
					$(SPI_DEBUG_SLAVE_TEST) $(PSRAM_WINDOW_TEST) $(PSRAMUHS_WINDOW_TEST) $(EMMC_TEST) $(CMSIS_TEST) $(SBC_TEST) $(SBC_ROM_TEST) $(LC3_TEST) $(AACDEC_TEST) \
					$(PSRAM_DUAL_PORT_TEST) $(CPU_MAX_POWER_TEST) $(I2C_DEBUG_SLAVE_TEST) $(GPIO_TEST) $(GPU_TEST) $(DMA2D_TEST) $(GAMU_TEST) $(GRADRV) \
					$(BREATHING_LED_TEST) $(GPADC_TEST) $(WDT_TEST) $(I2C_SLAVE_TEST) $(HEAP_TEST) $(CHARGER_TEST) $(JPEG_TEST) $(PNG_TEST) \
					$(COREMARK_TEST) $(DHRYSTONE_TEST) $(SDIO_HOST_PROGRAMMER_TEST) $(SPI2SDIO_TEST) $(QSPI_LCDC_TEST) $(RGBIF_LCDC_TEST) $(QSPI_TEST) \
					$(SDEMMC_TEST) $(MBEDTLS_TEST) $(FULLMAP_MMU_TEST) $(WIFI_TEST) $(BTHCI_ENABLE) $(PVT_TEST) $(SHANHAI_TEST) $(SINGLE_WIRE_UART_TEST) $(PCIE_TEST) \
					$(EXT_CRYSTAL_TEST) $(RTC_TEST) $(BOOT_REASON_TEST) $(TIMER_TEST) $(POWER_PROTECT_TEST) $(CPUDUMP_TEST) $(SPI_MODEM_TEST) \
					$(JIAYU_TEST) $(I3C_TEST) $(PMU_WDT_TEST) $(LOAD_LINUX) $(CRC_TEST) $(USB3_CDC_TEST) $(USB3_SPEED_TEST) $(CHECKSUM_TEST) $(USB_GENERAL_SPEED_TEST)\
					$(OSC_WAKEUP_TEST) $(DDR_TEST) $(SDIO_THROUGHPUT_TEST) $(L2HC_TEST) $(CHIP_WAKEUP_TEST)),)

ccflags-y += -Itests/hwtest -DHWTEST

obj-y += ../../tests/hwtest/rmt_msg_test.c

ifeq ($(DEFAULT_BSS_IN_PSRAM),1)
subdir-ccflags-y += -DDEFAULT_BSS_IN_PSRAM
endif

ifeq ($(CMSIS_TEST),1)
ccflags-y += -Iinclude/cmsis_dsp/
ccflags-y += -Iinclude/cmsis_nn/

ccflags-y  += -DCMSIS_TEST
obj-y += ../../tests/hwtest/cmsis_test.c
endif

ifeq ($(SBC_TEST),1)
obj-y += ../../multimedia/audio/codec/sbc/
obj-y += ../../tests/hwtest/sbc_test.c
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

ifeq ($(SBC_ROM_TEST),1)
obj-y += ../../multimedia/audio/codec/sbc/sbc_api/
obj-y += ../../tests/hwtest/test_rom_sbc.c
ccflags-y  += -Imultimedia/inc/
ccflags-y  += -DSBC_ROM_TEST
endif

ifeq ($(LC3_TEST),1)
obj-y += ../../tests/hwtest/lc3test.cpp
ccflags-y  += -DLC3_TEST
subdir-ccflags-y += -Imultimedia/inc/
ifeq ($(LC3_TEST_ROM_V2),1)
ifeq ($(LC3_RAM_RUN),1)
obj-y += ../../multimedia/audio/codec/lc3/sneak_peek_rom_v2/
endif
subdir-ccflags-y += -Imultimedia/audio/codec/lc3/sneak_peek_rom_v2/Common/Tables
ccflags-y  += -DLC3_TEST_ROM_V2
else
ifeq ($(LC3_RAM_RUN),1)
obj-y += ../../multimedia/audio/codec/lc3/sneak_peek_rom/
endif
subdir-ccflags-y += -Imultimedia/audio/codec/lc3/sneak_peek_rom/Common/Tables
endif
ifeq ($(LC3_RAM_RUN),1)
subdir-ccflags-y += -Imultimedia/inc/
subdir-ccflags-y += -Imultimedia/audio/codec/com/inc
endif
endif

ifeq ($(AACDEC_TEST),1)
ifeq ($(AAC_IN_ROM), 1)
ccflags-y += -DAAC_IN_ROM
endif
obj-y += ../../multimedia/audio/codec/aac/
obj-y += ../../tests/hwtest/aac_dec_test.c
ccflags-y += -Imultimedia/inc/
ccflags-y += -DAACDEC_TEST
ccflags-y += -Wno-unused-function
endif

ifeq ($(L2HC_TEST),1)
obj-y += ../../multimedia/audio/codec/l2hc/
obj-y += ../../tests/hwtest/l2hc_test.c
ifeq ($(L2HC_FLOAT),1)
ccflags-y += -Imultimedia/audio/codec/l2hc/float/api/
ccflags-y += -DL2HC_FLOAT
ccflags-y += -Wno-unused-function
ccflags-y += -Wno-unused-variable
else
ccflags-y += -Imultimedia/audio/codec/l2hc/fixed_point/api/
endif
ccflags-y += -DL2HC_TEST
endif

ifeq ($(CHIP_HAS_A7_DSP),1)
ccflags-y += -DCHIP_HAS_A7_DSP
asflags-y += -DCHIP_HAS_A7_DSP
endif

ifeq ($(NANDFLASH_TEST),1)
obj-y += ../../utils/hexdump/ ../../platform/drivers/nandflash/
obj-y += ../../tests/hwtest/nandflash_test.c
subdir-ccflags-y += -Iplatform/drivers/nandflash -Iutils/hexdump
subdir-ccflags-y += -DNANDFLASH_TEST
ifeq ($(NANDFLASH_TEST_PD_SLEEP),1)
subdir-ccflags-y += -DNANDFLASH_TEST_PD_SLEEP
endif
endif

ifeq ($(RSA_TEST),1)
obj-y += ../../utils/secure_boot/
subdir-ccflags-y += -DRSA_TEST
endif

ifneq ($(filter 1,$(USB_SERIAL_TEST) $(USB_SERIAL_SPEED_TEST) $(USB_SERIAL_DIRECT_XFER_TEST) $(USB_AUDIO_TEST) $(USB_MTP_TEST) $(USB_MSD_TEST) $(USB_GENERAL_SPEED_TEST)),)
obj-y += ../../tests/hwtest/usb_dev_test.c
ifeq ($(USB_MTP_TEST),1)
ifneq ($(RTOS),1)
BUILD_USB_DEV := 1
obj-y += ../../utils/string_convert/
endif
ccflags-y += -Iservices/usb/usb_dev/mtp
ccflags-y += -DUSB_MTP_TEST
else
BUILD_USB_DEV := 1
ifeq ($(USB_SERIAL_TEST),1)
ccflags-y += -DUSB_SERIAL_TEST
else ifeq ($(USB_SERIAL_SPEED_TEST),1)
ccflags-y += -DUSB_SERIAL_SPEED_TEST
else ifeq ($(USB_SERIAL_DIRECT_XFER_TEST),1)
ccflags-y += -DUSB_SERIAL_DIRECT_XFER_TEST
ifeq ($(USB_SLEEP_TEST),1)
ccflags-y += -DUSB_SLEEP_TEST
endif
else ifeq ($(USB_AUDIO_TEST),1)
ccflags-y += -DUSB_AUDIO_TEST
else ifeq ($(USB_MSD_TEST),1)
ccflags-y += -Iservices/usb/usb_dev/msd
ccflags-y += -DUSB_MSD_TEST
ifeq ($(EMMC_ENABLE), 1)
ccflags-y += -DEMMC_ENABLE
endif
ifeq ($(SDMMC_ENABLE), 1)
ccflags-y += -DSDMMC_ENABLE
endif
else ifeq ($(USB_GENERAL_SPEED_TEST),1)
ccflags-y += -DUSB_GENERAL_SPEED_TEST
endif
endif
endif

ifneq ($(filter 1,$(USB3_CDC_TEST) $(USB3_SPEED_TEST) $(USB3_WIFI_TEST)),)
ccflags-y += -Iplatform/drivers/usb3/
ccflags-y += -Iplatform/drivers/usb3/usb3_dev/inc
obj-y += ../../platform/drivers/usb3/
obj-y += ../../tests/hwtest/usb3_dev_test.c
ifeq ($(USB3_CDC_TEST),1)
ccflags-y += -DUSB3_CDC_TEST
obj-y += ../../platform/drivers/usb3/usb3_dev/src/usb3_cdc.c
endif
ifeq ($(USB3_SPEED_TEST),1)
ccflags-y += -DUSB3_SPEED_TEST
obj-y += ../../platform/drivers/usb3/usb3_dev/src/usb3_general.c
endif
ifeq ($(USB3_WIFI_TEST),1)
obj-y += ../../platform/drivers/usb3/usb3_dev/src/usb3_wifi_conn.c
endif
endif

ifneq ($(USB_SERIAL_DIRECT_XFER_SIZE),)
ccflags-y += -DUSB_SERIAL_DIRECT_XFER_SIZE=$(USB_SERIAL_DIRECT_XFER_SIZE)
endif

ifeq ($(CUSTOM_CDC_HID),1)
ccflags-y += -DCUSTOM_CDC_HID
endif

ifeq ($(I2C_TEST),1)
obj-y += ../../tests/hwtest/i2c_test.c
ccflags-y += -DI2C_TEST

ifeq ($(I2C_TEST_DMA_MODE),1)
CFLAGS_i2c_test.o += -DI2C_TEST_DMA_MODE
endif
ifeq ($(I2C_TEST_INT_MODE),1)
CFLAGS_i2c_test.o += -DI2C_TEST_INT_MODE
endif
ifneq ($(I2C_SPEED),)
CFLAGS_i2c_test.o += -DI2C_SPEED=$(I2C_SPEED)
endif
endif # I2C_TEST

ifeq ($(DSI_TEST),1)
obj-y += ../../tests/hwtest/dsi_test.c
ccflags-y += -DDSI_TEST -Iplatform/drivers/graphic/lcd
endif

ifeq ($(CSI_TEST),1)
obj-y += ../../tests/hwtest/csi_test/
ccflags-y += -DCSI_TEST
endif

ifeq ($(QSPI_1501_TEST),1)
obj-y += ../../tests/hwtest/qspi_1501_test.c ../../platform/drivers/qspi_1501/
ccflags-y += -DQSPI_1501_TEST -Iplatform/drivers/qspi_1501
endif

ifeq ($(QSPI_LCDC_TEST),1)
obj-y += ../../tests/hwtest/qspi_lcdc_test.c
ccflags-y += -DQSPI_LCDC_TEST -Iplatform/drivers/graphic/lcd
endif

ifeq ($(RGBIF_LCDC_TEST),1)
obj-y += ../../tests/hwtest/rgbif_lcdc_test.c
ccflags-y += -DRGBIF_LCDC_TEST -Iplatform/drivers/graphic/lcd
ifneq ($(LCDCPIX_MHZ),)
ccflags-y += -DLCDCPIX_MHZ=$(LCDCPIX_MHZ)
endif
endif

ifeq ($(QSPI_TEST),1)
obj-y += ../../tests/hwtest/qspi_test.c
ccflags-y += -DQSPI_TEST
endif

ifeq ($(JPEG_TEST),1)
obj-y += ../../tests/hwtest/jpeg_test.c
obj-y += ../../platform/drivers/jpeg/
ccflags-y += -DJPEG_TEST
ccflags-y += -Iplatform/drivers/jpeg
endif

ifeq ($(PNG_TEST),1)
obj-y += ../../tests/hwtest/png_test.c
ccflags-y += -DPNG_TEST
endif

ifeq ($(AF_TEST),1)
obj-y += ../../services/audioflinger/  ../../tests/hwtest/af_test.c
ccflags-y += -Iservices/audioflinger -DAF_TEST

ifeq ($(AF_TEST_TSF_TRIGGER),1)
CFLAGS_af_test.o  += -DAF_TEST_TSF_TRIGGER
endif

ifneq ($(filter 1,$(AF_INT_CODEC_TEST) $(AF_EXT_CODEC_TEST)),)
obj-y += ../../platform/drivers/codec/
ifeq ($(AF_INT_CODEC_TEST),1)
CFLAGS_af_test.o += -DAF_INT_CODEC_TEST
export AF_DEVICE_INT_CODEC ?= 1
export AF_DEVICE_EXT_CODEC ?= 0
else
CFLAGS_af_test.o += -DAF_EXT_CODEC_TEST
export AF_DEVICE_INT_CODEC ?= 0
export AF_DEVICE_EXT_CODEC ?= 1
endif
export AF_DEVICE_SPDIF ?= 0
export AF_DEVICE_I2S ?= 0
else ifeq ($(AF_SPDIF_TEST),1)
CFLAGS_af_test.o += -DAF_SPDIF_TEST
export AF_DEVICE_INT_CODEC ?= 0
export AF_DEVICE_EXT_CODEC ?= 0
export AF_DEVICE_SPDIF ?= 1
export AF_DEVICE_I2S ?= 0
else
CFLAGS_af_test.o += -DAF_I2S_TEST
export AF_DEVICE_INT_CODEC ?= 0
export AF_DEVICE_EXT_CODEC ?= 0
export AF_DEVICE_SPDIF ?= 0
export AF_DEVICE_I2S ?= 1
endif

ifneq ($(CHAN_NUM_CAPTURE),)
CFLAGS_af_test.o += -DCHAN_NUM_CAPTURE=$(CHAN_NUM_CAPTURE)
endif
ifeq ($(AF_CAP_SEP_BUF),1)
CFLAGS_af_test.o += -DAF_CAP_SEP_BUF
endif
ifeq ($(FIXED_BUF_CAPTURE),1)
CFLAGS_af_test.o += -DFIXED_BUF_CAPTURE
endif
endif # AF_TEST

ifeq ($(VD_TEST),1)
ifneq ($(VOICE_DETECTOR_EN),1)
$(error VOICE_DETECTOR_EN=1 missing when VD_TEST=1)
endif
obj-y += \
	../../apps/voice_detector/ \
    ../../apps/common/app_utils.c \
    ../../services/audioflinger/ \
    ../../services/audio_dump/ \
    ../../platform/drivers/codec/
ccflags-y += -Iapps/voice_detector -Iservices/audioflinger -DVD_TEST
endif

ifeq ($(CP_TEST),1)
obj-y += ../../tests/hwtest/cp_test.c
ccflags-y += -DCP_TEST
ifeq ($(CP_MPU_CFG),1)
ccflags-y += -DCP_MPU_CFG
ifeq ($(CP_FLASH_ACCESS),1)
ccflags-y += -DCP_FLASH_ACCESS
endif
endif
endif

ifeq ($(CP_SUBSYS_TEST),1)
ifneq ($(CP_AS_SUBSYS),1)
$(error CP_AS_SUBSYS=1 missing when CP_SUBSYS_TEST=1)
endif
ifeq ($(NOAPP),1)
obj-y += ../../tests/hwtest/cp_subsys_test.c
endif
# obj-y += ../../platform/drivers/cp_subsys/
ccflags-y += -DCP_SUBSYS_TEST -Iplatform/drivers/cp_subsys
ifeq ($(NO_TIMER),1)
CFLAGS_cp_subsys_test.o += -DNO_TIMER
endif
ifeq ($(MCU2CP_MSG_TEST),1)
CFLAGS_cp_subsys_test.o += -DMCU2CP_MSG_TEST
endif
ifeq ($(CP_SUBSYS_TRC_TO_MCU),1)
RMT_TRACE := 1
endif
endif

ifeq ($(CSP_SUBSYS_TEST),1)
obj-y += ../../tests/hwtest/csp_subsys_test.c ../../platform/drivers/csp_subsys/
ccflags-y += -DCSP_SUBSYS_TEST -Iplatform/drivers/csp_subsys
ifeq ($(NO_TIMER),1)
CFLAGS_csp_subsys_test.o += -DNO_TIMER
endif
ifeq ($(MCU2CSP_MSG_TEST),1)
CFLAGS_csp_subsys_test.o += -DMCU2CSP_MSG_TEST
endif
ifeq ($(CSP_SUBSYS_TRC_TO_MCU),1)
RMT_TRACE := 1
endif
endif

ifeq ($(TDM_TEST),1)
obj-y += ../../tests/hwtest/tdm_test.c
ccflags-y += -DTDM_TEST
ifeq ($(LIS25_TEST), 1)
ccflags-y += -DLIS25_TEST
obj-y += ../../tests/codec_vad/lis25ba.c
ccflags-y += -Itests/codec_vad/
endif
ifeq ($(NOAPP),1)
obj-y += ../../platform/drivers/codec/
obj-y += ../../services/audioflinger/
ccflags-y += -Iservices/audioflinger/
endif

ifeq ($(TDM_LOOPBACK_TEST), 1)
ccflags-y += -DTDM_LOOPBACK_TEST
endif
ifeq ($(TDM_PLAYBACK_ONLY_TEST), 1)
ccflags-y += -DTDM_PLAYBACK_ONLY_TEST
endif
endif #TDM_TEST

ifeq ($(SPI_NORFLASH_TEST),1)
obj-y += ../../tests/hwtest/spi_norflash_test.c
obj-y += ../../platform/drivers/spi_norflash/
ccflags-y += -DSPI_NORFLASH_TEST
endif #SPI_NORFLASH_TEST

ifeq ($(SEC_ENG_TEST),1)
obj-y += ../../tests/hwtest/sec_eng_test.c ../../utils/hexdump/
ccflags-y += -DSEC_ENG_TEST -Iutils/hexdump
ifneq ($(filter 1,$(AES_LARGE_DATA) $(HASH_LARGE_DATA) $(HMAC_LARGE_DATA)),)
obj-y += ../../tests/hwtest/asm_test.S
asflags-y += -Itests/hwtest
ifeq ($(AES_LARGE_DATA),1)
asflags-y += -DAES_LARGE_DATA
ccflags-y += -DAES_LARGE_DATA
endif
ifeq ($(HASH_LARGE_DATA),1)
asflags-y += -DHASH_LARGE_DATA
ccflags-y += -DHASH_LARGE_DATA
endif
ifeq ($(HMAC_LARGE_DATA),1)
asflags-y += -DHMAC_LARGE_DATA
ccflags-y += -DHMAC_LARGE_DATA
endif
endif
endif

ifeq ($(A7_DSP_TEST),1)
$(call echo-help,INFO    Please make sure DSP bin is up to date: out/$(DSP_BIN_NAME)/$(DSP_BIN_NAME).bin)
obj-y += ../../tests/hwtest/a7_dsp_test.c
obj-y += ../../tests/hwtest/asm_test.S
ccflags-y += -DA7_DSP_TEST
AFLAGS_asm_test.o += -DA7_DSP_TEST -DDSP_BIN_NAME=$(DSP_BIN_NAME).bin -Wa,-Iout/$(T)
DSP_BIN_NAME ?= hwtest_a7_dsp
$(obj)/../../tests/hwtest/asm_test.o : out/$(T)/$(DSP_BIN_NAME).bin
out/$(T)/$(DSP_BIN_NAME).bin : $(srctree)/out/$(DSP_BIN_NAME)/$(DSP_BIN_NAME).bin
	$(call CMDCPFILE,$(srctree)/out/$(DSP_BIN_NAME)/$(DSP_BIN_NAME).bin,$(srctree)/$@)
	$(call CMDCPFILE,$(srctree)/out/$(DSP_BIN_NAME)/$(DSP_BIN_NAME).elf,$(srctree)/$(@:.bin=.elf))
	$(call CMDCPFILE,$(srctree)/out/$(DSP_BIN_NAME)/$(DSP_BIN_NAME).map,$(srctree)/$(@:.bin=.map))
	$(call CMDCPFILE,$(srctree)/out/$(DSP_BIN_NAME)/$(DSP_BIN_NAME).lst,$(srctree)/$(@:.bin=.lst))
$(srctree)/out/$(DSP_BIN_NAME)/$(DSP_BIN_NAME).bin : FORCE
	$(call echo-help,)
	$(call echo-help,INFO    Please make sure DSP bin is up to date: out/$(DSP_BIN_NAME)/$(DSP_BIN_NAME).bin)
# We cannot build $(DSP_BIN_NAME) automatically here, because the env inherited by sub-make cannot be cleaned:
# E.g., KBUILD_SRC, TC, ...
	$(call echo-help,)
endif #A7_DSP_TEST

ifeq ($(A55_TEST),1)
obj-y += ../drivers/fw_linux/loader/
obj-y += ../../tests/hwtest/a55_test.c
ccflags-y += -DA55_TEST -Iplatform/drivers/fw_linux/inc
ifeq ($(AP_REBOOT_TEST),1)
ccflags-y += -DAP_REBOOT_TEST
endif
ifeq ($(RPMSG_MBTEST),1)
ccflags-y += -DRPMSG_MBTEST
endif
ifeq ($(SLEEP_TEST),1)
ccflags-y += -DSLEEP_TEST
endif
endif #A55_TEST

ifeq ($(TRANSQ_TEST),1)
obj-y += ../../tests/hwtest/transq_test.c
ccflags-y += -DTRANSQ_TEST
ifneq ($(RX_ROLE),)
CFLAGS_transq_test.o += -DRX_ROLE
endif
ifneq  ($(TRANSQ_ID),)
CFLAGS_transq_test.o += -DTRANSQ_ID=$(TRANSQ_ID)
endif
endif #TRANSQ_TEST

ifeq ($(FLASH_TEST),1)
obj-y += ../../tests/hwtest/flash_test.c
ccflags-y += -DFLASH_TEST
endif

ifeq ($(FULLMAP_MMU_TEST),1)
obj-y += ../../tests/hwtest/fullmap_mmu_test.c
ccflags-y += -DFULLMAP_MMU_TEST
endif

ifeq ($(DDR_TEST),1)
ccflags-y += -DDDR_TEST
endif

ifeq ($(PSRAM_TEST),1)
ifeq ($(PSRAM_SIMU_TEST),1)
ccflags-y += -DPSRAM_SIMU_TEST
endif
ifeq ($(PSRAM_SIMPLE_TEST),1)
ccflags-y += -DPSRAM_SIMPLE_TEST
endif
ifeq ($(PSRAM_SLEEP_TEST),1)
ccflags-y += -DPSRAM_SLEEP_TEST
endif

ifeq ($(PSRAM_CURRENT_TEST),1)
ccflags-y += -DPSRAM_CURRENT_TEST
ifeq ($(PSRAM_CURRENT_TEST_DMA_RW_LOOP),1)
ccflags-y += -DPSRAM_CURRENT_TEST_DMA_RW_LOOP
endif
ifeq ($(PSRAM_CURRENT_TEST_DMA_RW_1HZ),1)
ccflags-y += -DPSRAM_CURRENT_TEST_DMA_RW_1HZ
endif
endif

obj-y += ../../tests/hwtest/psram_test.c
ccflags-y += -DPSRAM_TEST -DCHIP_PSRAM_CTRL_VER=$(CHIP_PSRAM_CTRL_VER)
endif #PSRAM_TEST

ifeq ($(PSRAMUHS_TEST),1)
ifeq ($(PSRAMUHS_SIMU_TEST),1)
ccflags-y += -DPSRAMUHS_SIMU_TEST
endif
obj-y += ../../tests/hwtest/psram_test.c
ccflags-y += -DPSRAMUHS_TEST
endif #PSRAMUHS_TEST

ifeq ($(PSRAM_WINDOW_TEST),1)
obj-y += ../../tests/hwtest/psram_window_test.c
ccflags-y += -DPSRAM_WINDOW_TEST
ccflags-y += -DPSRAM_SPEED=$(PSRAM_SPEED)
ccflags-y += -DCHIP_PSRAM_CTRL_VER=$(CHIP_PSRAM_CTRL_VER)
ifeq ($(PSRAM_CALIB_ON_1D),1)
CFLAGS_psram_window_test.o += -DPSRAM_CALIB_ON_1D
endif
ifeq ($(PSRAM_WINDOW_TEST_FT),1)
CFLAGS_psram_window_test.o += -DPSRAM_WINDOW_TEST_FT
endif
endif

ifeq ($(PSRAMUHS_WINDOW_TEST),1)
obj-y += ../../tests/hwtest/psramuhs_window_test.c
ccflags-y += -DPSRAMUHS_WINDOW_TEST
ccflags-y += -DPSRAMUHS_SPEED=$(PSRAMUHS_SPEED)
endif

ifeq ($(PSRAM_DUAL_PORT_TEST),1)
obj-y += ../../tests/hwtest/psram_test.c
ccflags-y += -DCHIP_PSRAM_CTRL_VER=$(CHIP_PSRAM_CTRL_VER)
obj-y += ../../tests/hwtest/psram_dual_port_test.c
ccflags-y += -DPSRAM_DUAL_PORT_TEST
ccflags-y += -DPSRAM_SPEED=$(PSRAM_SPEED)
endif

ifeq ($(MBW_TEST),1)
obj-y += ../../tests/hwtest/mbw_test.c
include ../../tests/hwtest/mbw_flags.mk
endif #MBW_TEST

ifeq ($(SENSOR_HUB_TEST),1)
obj-y += ../../tests/hwtest/sensor_hub_test.c ../../platform/drivers/sensor_hub/
ccflags-y += -DSENSOR_HUB_TEST
CFLAGS_sensor_hub_test.o += -DSENSOR_HUB_TEST -Iplatform/drivers/sensor_hub -Iplatform/drivers/ana
ifeq ($(SENSOR_BT_TEST),1)
obj-y += ../../platform/drivers/bt/
ccflags-y += -DSENSOR_BT_TEST -Iplatform/drivers/bt
endif
ifeq ($(NO_PMU),1)
CFLAGS_sensor_hub_test.o += -DNO_PMU
endif
ifeq ($(PMU_FULL_INIT),1)
CFLAGS_sensor_hub_test.o += -DPMU_FULL_INIT
endif
ifeq ($(SENS_TRC_TO_MCU),1)
RMT_TRACE := 1
endif
ifeq ($(MCU2SENS_MSG_TEST),1)
CFLAGS_sensor_hub_test.o += -DMCU2SENS_MSG_TEST
endif
ifeq ($(VAD_CODEC_TEST),1)
obj-y += ../../services/audioflinger/
obj-y += ../../platform/drivers/codec/
export AF_DEVICE_INT_CODEC ?= 1
export AF_DEVICE_I2S ?= 1
CFLAGS_sensor_hub_test.o += -DVAD_CODEC_TEST -Iservices/audioflinger -Itests/sensor_hub/inc
ifeq ($(VAD_CODEC_LOOP),1)
CFLAGS_sensor_hub_test.o += -DVAD_CODEC_LOOP
endif
endif # VAD_CODEC_TEST
ifeq ($(VAD_APP_TEST),1)
CFLAGS_sensor_hub_test.o += -DVAD_APP_TEST
ifeq ($(VAD_KEY_EVT),1)
CFLAGS_sensor_hub_test.o += -DVAD_KEY_EVT
endif
ifeq ($(VAD_IF_TEST),1)
obj-y += ../../tests/sensor_hub/sensor_hub_ai/
CFLAGS_sensor_hub_test.o += -DVAD_IF_TEST -Iapps/sensor_hub/ -Itests/sensor_hub/sensor_hub_ai
endif
obj-y += \
	../../platform/drivers/codec/ \
	../../services/audioflinger/ \
	../../apps/common/ \
	../../apps/sensorhub/ \
	../../utils/cqueue/
ccflags-y += -Iapps/sensorhub \
	-Iservices/audioflinger \
	-Itests/sensor_hub/inc
endif # VAD_APP_TEST
endif # SENSOR_HUB_TEST

ifeq ($(CODEC_SIMU),1)
obj-y += ../../tests/hwtest/codec_test.o
ccflags-y += -DCODEC_SIMU
endif

ifeq ($(I2S_TEST),1)
obj-y += ../../tests/hwtest/i2s_test.o
ccflags-y += -DI2S_TEST
endif

ifeq ($(SENSOR_TEST),1)
obj-y += ../../tests/sensor_test/
ccflags-y += -DSENSOR_TEST
ccflags-y += -Itests/sensor_test
endif #SENSOR_TEST

ifeq ($(BECO_TEST),1)
# obj-y += ../../platform/cmsis/DSP_Lib/
# obj-y += ../../platform/cmsis/NN_Lib/
obj-y += ../../tests/hwtest/beco_test/ ../../platform/beco/
subdir-ccflags-y += -DBECO_TEST -Iinclude/beco -Iinclude/cmsis_dsp -Iinclude/cmsis_nn
endif

ifeq ($(TZ_TEST),1)
obj-y += ../../tests/hwtest/tz_test.c
CFLAGS_tz_test.o += -Iplatform/main/cmse
ccflags-y += -DTZ_TEST
endif

ifeq ($(LIS25BA_TEST),1)
obj-y += \
	../../services/audioflinger/ \
	../../platform/drivers/codec/ \
	../../services/audio_dump/ \
	../../services/bone_sensor/lis25ba_v2/ \

ccflags-y += -DLIS25BA_TEST -Iservices/bone_sensor/lis25ba_v2 -Iservices/audioflinger
endif

ifeq ($(DAC1_TEST),1)
DAC_TEST :=1
ccflags-y += -DDAC1_TEST
endif
ifeq ($(DAC2_TEST),1)
export AUDIO_OUTPUT_DAC2 ?=1
DAC_TEST :=1
ccflags-y += -DDAC2_TEST
endif
ifeq ($(DAC3_TEST),1)
export AUDIO_OUTPUT_DAC3 ?=1
DAC_TEST :=1
ccflags-y += -DDAC3_TEST
endif
ifeq ($(DAC_TEST_COMBINED),1)
ifneq ($(filter 2 3,$(CHIP_HAS_DAC_FIFO)),)
ifeq ($(CHIP_HAS_DAC_FIFO),2)
ccflags-y += -DDAC_TEST_COMBINED
endif
endif
endif
ifneq ($(ADC1_SAMP_RATE),)
ccflags-y += -DADC1_SAMP_RATE=$(ADC1_SAMP_RATE)
endif
ifneq ($(DAC1_SAMP_RATE),)
ccflags-y += -DDAC1_SAMP_RATE=$(DAC1_SAMP_RATE)
endif
ifeq ($(PLAY_NO_SWITCH),1)
ccflags-y += -DPLAY_NO_SWITCH
endif
ifeq ($(USE_ADC2), 1)
ccflags-y += -DUSE_ADC2
endif
ifeq ($(DAC_TEST),1)
obj-y += \
	../../services/audioflinger/ \
	../../platform/drivers/codec/ \
	../../tests/hwtest/dac_test.o \

ccflags-y += -DDAC_TEST -Iservices/audioflinger
endif

ifeq ($(ADC2_TEST),1)
obj-y += \
	../../services/audioflinger/ \
	../../platform/drivers/codec/ \
	../../tests/hwtest/adc2_test.o \

ccflags-y += -DADC2_TEST -Iservices/audioflinger
endif

ifeq ($(I2S_HOST_TEST),1)
obj-y += ../../tests/codec_vad/i2s_host/
ccflags-y += -DI2S_HOST_TEST -Itests/codec_vad/i2s_host/inc
endif

ifeq ($(FT_TEST),1)
obj-y += ../../tests/hwtest/ft_test.c
ccflags-y += -DFT_TEST
endif

ifeq ($(BT_CONN_TEST),1)
obj-y += ../../tests/hwtest/bt_conn_test.o
ccflags-y += -DBT_CONN_TEST
SLEEP_TEST ?= 1
NO_TIMER ?= 1
ifeq ($(BT_SLEEP_ONLY),1)
ccflags-y += -DBT_SLEEP_ONLY
endif
ifeq ($(BT_DRV_TEST),1)
obj-y += ../../platform/drivers/bt/
CFLAGS_bt_conn_test.o += -Iplatform/drivers/bt -DBT_DRV_TEST
ifeq ($(BT_DRV_TEST_BRIDGE_MODE),1)
ccflags-y += -DBT_DRV_TEST_BRIDGE_MODE
endif
endif
ifeq ($(BTPCM_TEST),1)
ccflags-y += -DBTPCM_TEST
endif
ifeq ($(DPD_BT_TEST),1)
ccflags-y += -DDPD_BT_TEST
endif
endif

ifeq ($(BT_RAMRUN_NEW),1)
obj-y += ../../platform/drivers/bt_controller/
endif

ifeq ($(BT_CONN_SIMU),1)
obj-y += ../../tests/hwtest/bt_conn_simu.o
ccflags-y += -DBT_CONN_SIMU
endif

CFLAGS_af_test.o += -Iservices/audioflinger

ifeq ($(BT_AF_SIMU),1)
obj-y += ../../tests/hwtest/af_test.o
obj-y += ../../services/audioflinger/
obj-y += ../../platform/drivers/codec/
CFLAGS_bt_conn_simu.o += -DBT_AF_SIMU
endif

ifeq ($(MINIMA_TEST),1)
ccflags-y += -DMINIMA_TEST
endif

ifeq ($(PWM_TEST),1)
obj-y += ../../tests/hwtest/pwm_test.c
ccflags-y += -DPWM_TEST
endif

ifeq ($(BREATHING_LED_TEST),1)
obj-y += ../../tests/hwtest/breathing_led_test.c
ccflags-y += -DBREATHING_LED_TEST
endif

ifeq ($(IR_TEST),1)
obj-y += ../../tests/hwtest/ir_test.c
ccflags-y += -DIR_TEST
ifeq ($(IR_DONOT_USE_DMA),1)
ccflags-y += -DIR_DONOT_USE_DMA
endif
endif

ifeq ($(TRNG_TEST),1)
obj-y += ../../tests/hwtest/trng_test.c
ccflags-y += -DTRNG_TEST
ifeq ($(RANDOM_TEST),1)
obj-y += ../../platform/drivers/random/
export HW_RANDOM ?= 1
CFLAGS_trng_test.o += -DRANDOM_TEST -Iplatform/drivers/random
endif
endif

ifeq ($(SDMMC_TEST),1)
obj-y += ../../tests/hwtest/sdmmc_test.c
ccflags-y += -DSDMMC_TEST
endif

ifeq ($(SDEMMC_TEST),1)
obj-y += ../../tests/hwtest/sdemmc_test.c
ccflags-y += -DSDEMMC_TEST
endif

ifeq ($(EMMC_TEST),1)
obj-y += ../../platform/drivers/emmc/
obj-y += ../../tests/hwtest/emmc_test.c
ccflags-y += -DEMMC_TEST -Iplatform/drivers/emmc/host/test
endif

ifeq ($(SDIO_HOST_TEST),1)
obj-y += ../../tests/hwtest/sdio_host_test.c
ccflags-y += -DSDIO_HOST_TEST
endif

ifeq ($(SDIO_HOST_SIGNAL_TEST),1)
obj-y += ../../tests/hwtest/sdio_host_signal_test.c
ccflags-y += -DSDIO_HOST_SIGNAL_TEST
ifneq ($(PERIPH_PLL_FREQ),)
ccflags-y += -DPERIPH_PLL_FREQ=$(PERIPH_PLL_FREQ)
endif
endif

ifeq ($(SDIO_HOST_FIRMWARE_TEST),1)
obj-y += ../../tests/sdio_download_test/
ccflags-y += -DSDIO_HOST_FIRMWARE_TEST
endif

ifeq ($(SDIO_HOST_PROGRAMMER_TEST),1)
obj-y += ../../tests/sdio_download_test/
ccflags-y += -DSDIO_HOST_PROGRAMMER_TEST
endif

ifeq ($(SDIO_DEVICE_TEST),1)
obj-y += ../../tests/hwtest/sdio_device_test.c
ccflags-y += -DSDIO_DEVICE_TEST
endif

ifeq ($(SDIO_THROUGHPUT_TEST),1)
ccflags-y += -DSDIO_THROUGHPUT_TEST
obj-y += ../../tests/hwtest/sdio_throughput_test.c
endif

ifeq ($(SPI_TEST),1)
obj-y += ../../tests/hwtest/spi_test.c
ccflags-y += -DSPI_TEST
endif

ifeq ($(PCIE_TEST),1)
obj-y += ../../tests/hwtest/pcie_test.c
ccflags-y += -DPCIE_TEST
endif

ifeq ($(SPI2SDIO_TEST),1)
obj-y += ../../tests/hwtest/spi2sdio_test.c
ccflags-y += -DSPI2SDIO_TEST
endif

ifeq ($(UART_TEST),1)
obj-y += ../../tests/hwtest/uart_test.c
obj-y += ../../utils/crc32/
ccflags-y += -Iutils/crc32
ccflags-y += -DUART_TEST
endif

ifeq ($(SCI_TEST),1)
obj-y += ../../tests/hwtest/sci_test.c
ccflags-y += -DSCI_TEST
endif

ifeq ($(BT_HOST_TEST),1)
obj-y += ../../tests/hwtest/bt_host_test.c ../../platform/drivers/bt_host/
ccflags-y += -DBT_HOST_TEST -Iplatform/drivers/bt_host
ifeq ($(NO_TIMER),1)
CFLAGS_bt_host_test.o += -DNO_TIMER
endif
ifeq ($(SYS2BTH_MSG_TEST),1)
CFLAGS_bt_host_test.o += -DSYS2BTH_MSG_TEST
endif
ifeq ($(BTH_TRC_TO_SYS),1)
RMT_TRACE := 1
endif
endif

ifeq ($(DSP_TEST),1)
obj-y += ../../tests/besair_platform/bsp_tester/test_gpadc.c \
		../../tests/besair_platform/bsp_tester/test_gpio.c \
		../../tests/besair_platform/bsp_tester/test_ipc_notify.c \
		../../tests/besair_platform/bsp_tester/test_norflash.c \
		../../tests/besair_platform/bsp_tester/test_ostick.c \
		../../tests/besair_platform/bsp_tester/test_pwm.c \
		../../tests/besair_platform/bsp_tester/test_rpc.c \
		../../tests/besair_platform/bsp_tester/test_spi.c \
		../../tests/besair_platform/bsp_tester/test_timer.c \
		../../tests/besair_platform/bsp_tester/test_trng.c \
		../../tests/besair_platform/bsp_tester/test_vmic.c

obj-y += ../../platform/drivers/hifi4/ \
		../../apps/dsp_hifi4/

ccflags-y += -DDSP_TEST -Iplatform/drivers/hifi4 \
	-Iapps/dsp_hifi4 \
	-Itests/besair_platform/bsp_tester
endif #DSP_TEST

ifeq ($(DSP_HIFI4_TEST),1)
obj-y += ../../tests/hwtest/dsp_hifi4_test.c ../../platform/drivers/hifi4/
ccflags-y += -DDSP_HIFI4_TEST -Iplatform/drivers/hifi4
ifeq ($(MCU2DSP_HIFI4_MSG_TEST),1)
CFLAGS_dsp_hifi4_test.o += -DMCU2DSP_HIFI4_MSG_TEST
endif
ifeq ($(DSP_HIFI4_TRC_TO_MCU),1)
RMT_TRACE := 1
endif
endif # DSP_HIFI4_TEST

ifeq ($(DSP_M55_TEST),1)
obj-y += ../../tests/hwtest/dsp_m55_test.c ../../platform/drivers/dsp_m55/
ccflags-y += -DDSP_M55_TEST -Iplatform/drivers/dsp_m55 -Iservices/dma_audio/dsp
ifeq ($(NO_TIMER),1)
CFLAGS_dsp_m55_test.o += -DNO_TIMER
endif
ifeq ($(MCU2DSP_M55_MSG_TEST),1)
CFLAGS_dsp_m55_test.o += -DMCU2DSP_M55_MSG_TEST
endif
ifeq ($(DSP_M55_TRC_TO_MCU),1)
RMT_TRACE := 1
endif
endif # DSP_M55_TEST

ifeq ($(DSP_M55C1_TEST),1)
obj-y += ../../tests/hwtest/dsp_m55c1_test.c ../../platform/drivers/dsp_m55c1/
ccflags-y += -DDSP_M55C1_TEST -Iplatform/drivers/dsp_m55c1 -Iservices/dma_audio/dsp
ifeq ($(NO_TIMER),1)
CFLAGS_dsp_m55c1_test.o += -DNO_TIMER
endif
ifeq ($(MCU2DSP_M55C1_MSG_TEST),1)
CFLAGS_dsp_m55c1_test.o += -DMCU2DSP_M55C1_MSG_TEST
endif
ifeq ($(DSP_M55C1_TRC_TO_MCU),1)
RMT_TRACE := 1
endif
endif # DSP_M55C1_TEST

ifeq ($(DMA_AUDIO_APP),1)
obj-y += ../../services/audioflinger/ \
	../../platform/drivers/codec/ \
	../../platform/drivers/stream_dma_rpc/ \
	../../services/dma_audio/app/ \

ccflags-y += -DDMA_AUDIO_APP \
	-Iservices/audioflinger \
	-Iplatform/drivers/stream_dma_rpc \
	-Iservices/dma_audio/common \
	-Iservices/dma_audio/app \

ifeq ($(DMA_RPC_CLI),1)
ccflags-y += -DDMA_RPC_CLI
endif
ifeq ($(DMA_AUDIO_LOW_LATENCY),1)
ccflags-y += -DDMA_AUDIO_LOW_LATENCY
endif
ifeq ($(DMA_AUDIO_NORM_PLAY_ON),1)
ccflags-y += -DDMA_AUDIO_NORM_PLAY_ON
endif
ifeq ($(CODEC_SW_SYNC),1)
ccflags-y += -DCODEC_SW_SYNC
endif
endif # DMA_AUDIO_APP

# ifeq ($(RMT_TRACE),1)
# obj-y += ../../platform/drivers/rmt_trace/
# endif

ifeq ($(SPI_MODEM_TEST),1)
obj-y += ../../utils/crc16/
obj-y += ../../utils/hexdump/
obj-y += ../../platform/drivers/modem/spi/
ccflags-y += -DSPI_MODEM_TEST
endif

ifeq ($(SPI_DEBUG_SLAVE_TEST),1)
obj-y += ../../tests/hwtest/spi_debug_slave_test.c
obj-y += ../../utils/crc32/
ccflags-y += -Iutils/crc32
ccflags-y += -DSPI_DEBUG_SLAVE_TEST
endif

ifeq ($(CPU_MAX_POWER_TEST),1)
obj-y += ../../tests/cpu_max_power/
ccflags-y += -DCPU_MAX_POWER_TEST
endif

ifeq ($(I2C_DEBUG_SLAVE_TEST),1)
obj-y += ../../tests/hwtest/i2c_debug_slave_test.c
ccflags-y += -DI2C_DEBUG_SLAVE_TEST
endif

ifeq ($(I2C_SLAVE_TEST),1)
obj-y += ../../tests/hwtest/i2c_slave_test.c
ccflags-y += -DI2C_SLAVE_TEST
ifeq ($(I2C_USE_DMA),1)
ccflags-y += -DI2C_USE_DMA
endif
endif

ifeq ($(I3C_TEST),1)
obj-y += ../../tests/hwtest/i3c_test.c
obj-y += ../../tests/sensor_test/
ccflags-y += -DI3C_TEST
endif

ifeq ($(GRADRV),1)
obj-y += ../../platform/drivers/graphic/
endif

ifeq ($(GPU_TEST),1)
export DISPLAY_TEST_LIB := 1
export TEST_LCD ?= nr320x320
export TEST_WIDTH ?= 320
export TEST_HEIGHT ?= 320
ccflags-y += -DGPU_TEST -DDISPLAY_TEST_LIB
ccflags-y += -DTEST_WIDTH=$(TEST_WIDTH) -DTEST_HEIGHT=$(TEST_HEIGHT)
ifeq ($(TEST_WITH_LCDC),1)
ccflags-y += -DTEST_WITH_LCDC
endif
ifeq ($(TEST_WITH_FS), 1)
ccflags-y += -DTEST_WITH_FS
endif
ifeq ($(DECNANO_TEST), 1)
ccflags-y += -DDECNANO_TEST
endif
ifeq ($(PERF_TEST), 1)
ccflags-y += -DPERF_TEST
endif
obj-y += ../../tests/hwtest/gpu_test.c
obj-y += ../../tests/display_test/
ccflags-y += \
            -Iplatform/drivers/graphic/gpu/drv/vglite/inc \
            -Itests/hwtest/ \
            -Itests/display_test/ \
            -Iplatform/drivers/graphic/gpu/drv/vglite/VGLiteKernel/ \
            -Iplatform/drivers/graphic/gpu/ \
            -Iplatform/drivers/graphic/ \
            -Iutils/hwtimer_list/
endif #end of GPU_TEST

ifeq ($(DMA2D_TEST),1)
export DISPLAY_TEST_LIB := 1
export TEST_LCD ?= nr320x320
export TEST_WIDTH ?= 320
export TEST_HEIGHT ?= 320
ccflags-y += -DDMA2D_TEST -DDISPLAY_TEST_LIB
ccflags-y += -DTEST_WIDTH=$(TEST_WIDTH) -DTEST_HEIGHT=$(TEST_HEIGHT)
ifeq ($(TEST_WITH_LCDC),1)
ccflags-y += -DTEST_WITH_LCDC
endif
ifeq ($(PERF_TEST), 1)
ccflags-y += -DPERF_TEST
endif
obj-y += ../../tests/hwtest/dma2d_test.c
obj-y += ../../tests/display_test/
ccflags-y += -Itests/display_test/ \
             -Iplatform/drivers/graphic/gpu/drv/vglite/inc/ \
             -Iplatform/drivers/graphic/gpu/ \
             -Iplatform/drivers/graphic/
endif

ifeq ($(GAMU_TEST),1)
export DISPLAY_TEST_LIB := 1
export TEST_LCD := nr320x320
export TEST_WITH_LCDC ?= 1
obj-y += ../../tests/hwtest/gamu_test.c
obj-y += ../../tests/display_test/
ccflags-y += -DGAMU_TEST -DDISPLAY_TEST_LIB
ifeq ($(TEST_WITH_LCDC),1)
ccflags-y += -DTEST_WITH_LCDC
endif
ccflags-y += \
            -Iplatform/drivers/graphic/gpu/drv/vglite/inc \
            -Itests/hwtest/ \
            -Itests/display_test/ \
            -Iplatform/drivers/graphic/gpu/drv/vglite/VGLiteKernel/ \
            -Iplatform/drivers/graphic/gpu/ \
            -Iplatform/drivers/graphic/ \
            -Iutils/hwtimer_list/
endif

ifeq ($(GRAPHIC_TEST),1)
obj-y += ../../tests/hwtest/graphic/test_cases.c
ccflags-y += -DGRAPHIC_TEST
ccflags-y += \
				   -Itests/hwtest/ \
				   -Iplatform/drivers/graphic/fb \
				   -Iutils/hwtimer_list/
endif

ifeq ($(FB_DSI_1S_TEST), 1)
ccflags-y += -DFB_DSI_1S_TEST
endif
ifeq ($(FB_DSI_60HZ_TEST), 1)
ccflags-y += -DFB_DSI_60HZ_TEST
endif
ifeq ($(FB_DSI_60HZ_CPU_SIM_TEST), 1)
ccflags-y += -DFB_DSI_60HZ_CPU_SIM_TEST
endif
ifeq ($(FB_GRAPHIC_TEST), 1)
ccflags-y += -DFB_GRAPHIC_TEST
endif

ifeq ($(GPIO_TEST),1)
obj-y += ../../tests/hwtest/gpio_test.c
ccflags-y += -DGPIO_TEST
endif

ifeq ($(GPADC_TEST),1)
obj-y += ../../tests/hwtest/gpadc_test.c
ccflags-y += -DGPADC_TEST
endif

ifeq ($(WDT_TEST),1)
obj-y += ../../tests/hwtest/wdt_test.c
ccflags-y += -DWDT_TEST
endif

ifeq ($(HEAP_TEST),1)
obj-y += ../../tests/hwtest/heap_test.c
ccflags-y += -DHEAP_TEST
endif

ifeq ($(CHARGER_TEST),1)
obj-y += ../../tests/hwtest/charger_test.c
ccflags-y += -DCHARGER_TEST
endif

ifeq ($(COREMARK_TEST),1)
obj-y += ../../tests/coremark/
ccflags-y += -DCOREMARK_TEST
endif

ifeq ($(DHRYSTONE_TEST),1)
obj-y += ../../tests/dhrystone2/
ccflags-y += -DDHRYSTONE_TEST
endif

ifeq ($(MBEDTLS_TEST),1)
obj-y += ../../utils/mbedtls/test/
ccflags-y += -DMBEDTLS_TEST
endif

ifeq ($(SHANHAI_TEST),1)
obj-y += ../../tests/shanhai/
obj-y += ../../utils/hexdump/
ccflags-y += -Iutils/hexdump
ccflags-y += -DSHANHAI_TEST
endif

ifeq ($(JIAYU_TEST),1)
obj-y += ../../tests/jiayu/
obj-y += ../../utils/hexdump/
ccflags-y += -Iutils/hexdump
ccflags-y += -DJIAYU_TEST
endif

ifeq ($(WIFI_TEST),1)
obj-y += ../../tests/wifi/
ccflags-y += -Itests/wifi
ccflags-y += -Iplatform/drivers/conn/linux_device
ccflags-y += -DWIFI_TEST
endif

ifeq ($(BTHCI_ENABLE),1)
ccflags-y += -Iservices/bthci
ccflags-y += -DBTHCI_ENABLE
endif

ifeq ($(PVT_TEST),1)
obj-y += ../../tests/hwtest/pvt_test.c
ccflags-y += -DPVT_TEST
endif

ifeq ($(SINGLE_WIRE_UART_TEST),1)
obj-y += ../../tests/hwtest/single_uart_test.c
obj-y += ../../services/communication/
ccflags-y += -Iservices/communication
ccflags-y += -DSINGLE_WIRE_UART_TEST
endif

ifeq ($(EXT_CRYSTAL_TEST),1)
obj-y += ../../tests/hwtest/ext_crystal_test.c
ccflags-y += -DEXT_CRYSTAL_TEST
endif

ifeq ($(RTC_TEST),1)
obj-y += ../../tests/hwtest/rtc_test.c
ccflags-y += -DRTC_TEST
ifeq ($(RTC_ENABLE),1)
ccflags-y += -DRTC_ENABLE
else ifeq ($(RTC_CALENDAR),1)
ccflags-y += -DRTC_CALENDAR
endif
endif

ifeq ($(BOOT_REASON_TEST),1)
obj-y += ../../tests/hwtest/boot_reason_test.c
ccflags-y += -DBOOT_REASON_TEST
ifeq ($(RTC_ENABLE),1)
ccflags-y += -DRTC_ENABLE
else ifeq ($(RTC_CALENDAR),1)
ccflags-y += -DRTC_CALENDAR
endif
ifeq ($(CHG_OUT_PWRON),1)
ccflags-y += -DCHG_OUT_PWRON
endif
endif

ifeq ($(TIMER_TEST),1)
obj-y += ../../tests/hwtest/timer_test.c
ccflags-y += -DTIMER_TEST
ifeq ($(USER_TIMER_TEST),1)
ccflags-y += -DUSER_TIMER_TEST
endif
endif

ifeq ($(POWER_PROTECT_TEST),1)
obj-y += ../../tests/hwtest/power_protect_test.c
ccflags-y += -DPOWER_PROTECT_TEST
endif

ifeq ($(CPUDUMP_TEST),1)
obj-y += ../../tests/hwtest/cpudump_test.c
ccflags-y += -DCPUDUMP_TEST
endif

ifeq ($(PMU_WDT_TEST),1)
obj-y += ../../tests/hwtest/pmu_wdt_test.c
ccflags-y += -DPMU_WDT_TEST
endif

ifeq ($(CRC_TEST),1)
obj-y += ../../tests/hwtest/crc_test.c
ccflags-y += -DCRC_TEST
endif

ifeq ($(CHECKSUM_TEST),1)
obj-y += ../../tests/hwtest/checksum_test.c
ccflags-y += -DCHECKSUM_TEST
endif

# FT and SLT are used
ifeq ($(OSC_WAKEUP_TEST),1)
obj-y += ../../tests/hwtest/osc_wakeup_test.c
ccflags-y += -DOSC_WAKEUP_TEST
endif

ifeq ($(CHIP_WAKEUP_TEST),1)
obj-y += ../../tests/hwtest/chip_wakeup_test.c
ccflags-y += -DCHIP_WAKEUP_TEST
endif

endif # hwtest cases

ifeq ($(RTOS)-$(THREAD_CREATION_TEST),1-1)
ccflags-y += -DTHREAD_CREATION_TEST
endif

ifeq ($(TRACE_ON_USB)-$(NOAPP),1-1)
BUILD_USB_DEV := 1
endif
ifeq ($(BUILD_USB_DEV),1)
obj-y += ../../platform/drivers/usb/usb_dev/
endif

ccflags-y += \
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

ifeq ($(ARM_CMSE),1)
obj-y += cmse/
ifeq ($(CMSE_CUSTOM_APP_INIT),1)
CFLAGS_cmse_main.o += -DCMSE_CUSTOM_APP_INIT
endif
CFLAGS_cmse_main.o += -Iplatform/drivers/mpc
CFLAGS_nuttx_main.o += -Iplatform/drivers/mpc
endif

ccflags-y +=  \
	-Iapps/audioplayers  \
	-Iapps/common  \
	-Iapps/factory \
	-Iapps/main \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iinclude \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/random \
	-Iplatform/drivers/security_engine \
	-Iplatform/drivers/usb/usb_dev/inc \
	-Iservices/norflash_api  \
	-Iservices/nv_section/factory_section \
	-Iservices/nv_section/log_section \
	-Itests/sensor_test                \
	-Iutils/boot_struct  \
	-Iutils/cqueue \
	-Iutils/heap \
	-Iutils/hwtimer_list \
	-Iutils/list \
	-Iservices/ota/bes_ota/inc \
	-Iservices/ota

ifneq ($(FLASH_FILL),)
ccflags-y += -DFLASH_FILL=$(FLASH_FILL)
endif

ifneq ($(DEBUG_PORT),)
ccflags-y += -DDEBUG_PORT=$(DEBUG_PORT)
endif

ifeq ($(ROM_UTILS_ON),1)
ccflags-y += -DROM_UTILS_ON
endif

ifeq ($(ULTRA_LOW_POWER),1)
ccflags-y += -DULTRA_LOW_POWER
endif

ifeq ($(NO_PMU),1)
ccflags-y += -DNO_PMU
endif

ifeq ($(NO_TIMER),1)
ccflags-y += -DNO_TIMER
endif

ifeq ($(DEBUG_TIME),1)
ccflags-y += -DDEBUG_TIME
endif

ifneq ($(SLEEP_TEST),)
ccflags-y += -DSLEEP_TEST=$(SLEEP_TEST)
endif

ifeq ($(FLASH_BOOT_TEST),1)
ccflags-y += -DFLASH_BOOT_TEST
endif

ifeq ($(CHIP_WAKE_TEST),1)
ccflags-y += -DCHIP_WAKE_TEST
endif

ifeq ($(RTOS),1)
ifeq ($(KERNEL),RTX)
ccflags-y += -Irtos/rtx/TARGET_CORTEX_M
endif
endif

ifeq ($(DEBUG_MODE_USB_DOWNLOAD),1)
ccflags-y += -DDEBUG_MODE_USB_DOWNLOAD
endif

ifeq ($(ARM_CMNS),1)
ccflags-y += -Iplatform/main/cmse
endif

ifeq ($(USER_SECURE_BOOT),1)
ccflags-y += -Iutils/user_secure_boot
endif

ifeq ($(PSRAM_INIT_CHECK),1)
ccflags-y += -DPSRAM_INIT_CHECK
endif

endif # !USE_OS_MAIN

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)
