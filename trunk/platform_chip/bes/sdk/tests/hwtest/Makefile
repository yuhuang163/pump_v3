cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))
obj-y += ../../tests/rom/pmu_rom.c
obj-y += ../../tests/rom/startup_ARMCM.S

ifneq ($(CHIP_HAS_USB),1)
obj-y := $(filter-out usb_dev_test.c usb_host_test.c usb3_dev_test.c,$(obj-y))
endif

ifneq ($(CHIP_HAS_USB3),1)
obj-y := $(filter-out usb3_dev_test.c,$(obj-y))
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

obj-y += ../../utils/hwtimer_list/

obj-y += ../../utils/hexdump/

obj-y += ../../platform/drivers/ana/

ifeq ($(CHIP_HAS_USB),1)
obj-y += ../../platform/drivers/usb/usb_dev/
ccflags-y += -Iplatform/drivers/usb/usb_dev/inc
endif

ifeq ($(CHIP_HAS_PSRAM),1)
obj-y += ../../platform/drivers/psram/
endif

ifeq ($(CHIP_HAS_DDR),1)
obj-y += ../../platform/drivers/ddr/
ccflags-y += -Iplatform/drivers/ddr/inc
endif

ccflags-y += -Itests/rom -Iutils/hwtimer_list -Iutils/hexdump -Iplatform/drivers/ana

ccflags-y += -Iservices/audioflinger

ifeq ($(ROM_ENTRY_HOOK),1)
asflags-y += -DROM_ENTRY_HOOK
endif

ifeq ($(NOAPP),1)
ccflags-y += -DNOAPP
endif

ifeq ($(ROM_CACHE_MEM),1)
CFLAGS_hwtest_main.o += -DROM_CACHE_MEM
ifeq ($(NO_LOAD_EXTRA_RAM),1)
CFLAGS_hwtest_main.o += -DNO_LOAD_EXTRA_RAM
endif
endif

ifeq ($(SPI_ROM_ONLY),1)
CFLAGS_hwtest_main.o += -DSPI_ROM_ONLY
CFLAGS_efuse_test.o += -DSPI_ROM_ONLY
endif

ifeq ($(I2C_TEST_DMA_MODE),1)
CFLAGS_i2c_test.o += -DI2C_TEST_DMA_MODE
endif
ifeq ($(I2C_TEST_INT_MODE),1)
CFLAGS_i2c_test.o += -DI2C_TEST_INT_MODE
endif
ifneq ($(I2C_SPEED),)
CFLAGS_i2c_test.o += -DI2C_SPEED=$(I2C_SPEED)
endif

ifeq ($(NO_CLOCK_INIT),1)
CFLAGS_hwtest_main.o += -DNO_CLOCK_INIT
endif
ifeq ($(NO_PMU),1)
CFLAGS_hwtest_main.o += -DNO_PMU
endif
ifeq ($(NO_LPU_26M),1)
CFLAGS_hwtest_main.o += -DNO_LPU_26M
endif
ifeq ($(NO_PLL),1)
CFLAGS_hwtest_main.o += -DNO_PLL
endif

ifeq ($(NO_WAKEUP),1)
CFLAGS_sleep_test.o += -DNO_WAKEUP
endif
ifeq ($(NO_TIMER),1)
CFLAGS_sleep_test.o += -DNO_TIMER
endif

ifeq ($(CHIP_HAS_A7_DSP),1)
ccflags-y += -DCHIP_HAS_A7_DSP
endif

ifeq ($(USB_CLK_SRC_24M_X2),1)
USB_CFG_FLAGS += -DUSB_CLK_SRC_24M_X2
else
ifeq ($(USB_CLK_SRC_48M),1)
USB_CFG_FLAGS += -DUSB_CLK_SRC_48M
else
ifeq ($(USB_CLK_SRC_26M_X4),1)
USB_CFG_FLAGS += -DUSB_CLK_SRC_26M_X4
else
ifeq ($(USB_CLK_SRC_26M_X2),1)
USB_CFG_FLAGS += -DUSB_CLK_SRC_26M_X2
endif
endif
endif
endif

ifeq ($(USB_PMU_CFG),1)
USB_CFG_FLAGS += -DUSB_PMU_CFG
endif

CFLAGS_usb_dev_test.o += $(USB_CFG_FLAGS)
CFLAGS_usb_host_test.o += $(USB_CFG_FLAGS)

ifneq ($(USB_SERIAL_DIRECT_XFER_SIZE),)
CFLAGS_usb_dev_test.o += -DUSB_SERIAL_DIRECT_XFER_SIZE=$(USB_SERIAL_DIRECT_XFER_SIZE)
endif

TEST_CASE_LIST := $(subst $(comma),$(space),$(TC))

obj-y += ../../platform/drivers/norflash/
ifneq ($(filter NANDFLASH_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../platform/drivers/nandflash/
CFLAGS_nandflash_test.o += -Iplatform/drivers/nandflash -Iutils/boot_struct
endif

ifneq ($(filter PMU_WDT_TEST,$(TEST_CASE_LIST)),)
ifeq ($(PMU_WDT_IRQ),1)
CFLAGS_pmu_wdt_test.o += -DPMU_WDT_IRQ
endif
endif

ifneq ($(filter HWFFT_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../platform/drivers/hwfft/
CFLAGS_hwfft_test.o += -Iplatform/drivers/hwfft
endif

ifneq ($(filter SBC_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../multimedia/audio/codec/sbc/
ccflags-y += -Imultimedia/inc/
SBC_REDUCE_SIZE := 1
ifeq ($(SBC_REDUCE_SIZE),1)
ccflags-y += -DSBC_REDUCE_SIZE
endif
ifeq ($(SBC_CURRENT_TEST),1)
ccflags-y += -DSBC_CURRENT_TEST
endif
endif

ifneq ($(filter AF_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../services/audioflinger/

ifneq ($(filter 1,$(AF_INT_CODEC_TEST) $(AF_EXT_CODEC_TEST)),)
obj-y += ../../platform/drivers/codec/
ifeq ($(AF_INT_CODEC_TEST),1)
CFLAGS_af_test.o += -DAF_INT_CODEC_TEST
ifeq ($(ANC_APP),1)
obj-y += ../../multimedia/audio/process/anc/src/$(CHIP)/anc_process_$(CHIP).o
obj-y += ../../multimedia/inc/audio/process/anc/cfg/anc_cfg.o
ccflags-y += -Imultimedia/inc/audio/process/anc/include \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/include
endif
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

ifneq ($(filter USB_AUDIO_TEST,$(TEST_CASE_LIST)),)
ifneq ($(USB_ISO),1)
$(error USB_AUDIO_TEST requires USB_ISO=1)
endif
endif

CFLAGS_sec_eng_test.o += -Iutils/hexdump
ifneq ($(filter SEC_ENG_TEST,$(TEST_CASE_LIST)),)
#obj-y += ../../utils/hexdump/
asflags-y += -Itests/hwtest
ifeq ($(AES_LARGE_DATA),1)
asflags-y += -DAES_LARGE_DATA
CFLAGS_sec_eng_test.o += -DAES_LARGE_DATA
endif
ifeq ($(HASH_LARGE_DATA),1)
asflags-y += -DHASH_LARGE_DATA
CFLAGS_sec_eng_test.o += -DHASH_LARGE_DATA
endif
ifeq ($(HMAC_LARGE_DATA),1)
asflags-y += -DHMAC_LARGE_DATA
CFLAGS_sec_eng_test.o += -DHMAC_LARGE_DATA
endif
endif

ifneq ($(filter A7_DSP_TEST,$(TEST_CASE_LIST)),)
DSP_BIN_NAME ?= hwtest_a7_dsp
$(obj)/asm_test.o : out/$(T)/$(DSP_BIN_NAME).bin
AFLAGS_asm_test.o += -DA7_DSP_TEST -DDSP_BIN_NAME=$(DSP_BIN_NAME).bin -Wa,-Iout/$(T)
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
endif

ifneq ($(filter TRANSQ_TEST,$(TEST_CASE_LIST)),)
ifneq ($(RX_ROLE),)
CFLAGS_transq_test.o += -DRX_ROLE
endif
ifneq  ($(TRANSQ_ID),)
CFLAGS_transq_test.o += -DTRANSQ_ID=$(TRANSQ_ID)
endif
endif

ifneq ($(filter CP_SUBSYS_TEST,$(TEST_CASE_LIST)),)
ifneq ($(CP_AS_SUBSYS),1)
$(error CP_AS_SUBSYS=1 missing when CP_SUBSYS_TEST=1)
endif
obj-y += ../../platform/drivers/cp_subsys/
CFLAGS_cp_subsys_test.o += -Iplatform/drivers/cp_subsys -Iplatform/main/cp_subsys
ifeq ($(MCU2CP_MSG_TEST),1)
CFLAGS_cp_subsys_test.o += -DMCU2CP_MSG_TEST
endif
ifeq ($(NO_TIMER),1)
CFLAGS_cp_subsys_test.o += -DNO_TIMER
endif
ifeq ($(CP_SUBSYS_TRC_TO_MCU),1)
RMT_TRACE := 1
endif
endif

ifneq ($(filter CSP_SUBSYS_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../platform/drivers/csp_subsys/
CFLAGS_csp_subsys_test.o += -Iplatform/drivers/csp_subsys -Itests/csp_subsys
ifeq ($(MCU2CSP_MSG_TEST),1)
CFLAGS_csp_subsys_test.o += -DMCU2CSP_MSG_TEST
endif
ifeq ($(NO_TIMER),1)
CFLAGS_csp_subsys_test.o += -DNO_TIMER
endif
ifeq ($(CSP_SUBSYS_TRC_TO_MCU),1)
RMT_TRACE := 1
endif
endif

ifneq ($(filter SENSOR_HUB_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../platform/drivers/sensor_hub/
CFLAGS_sensor_hub_test.o += -Iplatform/drivers/sensor_hub -Itests/sensor_hub/inc
ifeq ($(MCU2SENS_MSG_TEST),1)
CFLAGS_sensor_hub_test.o += -DMCU2SENS_MSG_TEST
endif
ifeq ($(SENSOR_BT_TEST),1)
obj-y += ../../platform/drivers/bt/ ../../utils/heap/
ccflags-y += -DSENSOR_BT_TEST -Iplatform/drivers/bt
endif
ifeq ($(NO_TIMER),1)
CFLAGS_sensor_hub_test.o += -DNO_TIMER
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
endif

ifneq ($(filter BT_HOST_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../platform/drivers/bt_host/
CFLAGS_bt_host_test.o += -Iplatform/drivers/bt_host -Itests/bt_host
ifeq ($(SYS2BTH_MSG_TEST),1)
CFLAGS_bt_host_test.o += -DSYS2BTH_MSG_TEST
endif
ifeq ($(NO_TIMER),1)
CFLAGS_bt_host_test.o += -DNO_TIMER
endif
ifeq ($(NO_PMU),1)
CFLAGS_bt_host_test.o += -DNO_PMU
endif
ifeq ($(PMU_FULL_INIT),1)
CFLAGS_bt_host_test.o += -DPMU_FULL_INIT
endif
ifeq ($(BTH_TRC_TO_SYS),1)
RMT_TRACE := 1
endif
endif

ifneq ($(filter DSP_HIFI4_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../platform/drivers/hifi4/
CFLAGS_dsp_hifi4_test.o += -Iplatform/drivers/hifi4
ifeq ($(DSP_HIFI4_TRC_TO_MCU),1)
RMT_TRACE := 1
endif
endif

ifeq ($(RMT_TRACE),1)
obj-y += ../../platform/drivers/rmt_trace/
endif

ifneq ($(filter RSA_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../utils/secure_boot/
subdir-ccflags-y += -DRSA_TEST
endif

ifneq ($(filter RANDOM_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../platform/drivers/random/
export HW_RANDOM ?= 1
# RANDOM_TEST is one set of TRNG_TEST
CFLAGS_trng_test.o += -DTRNG_TEST -Iplatform/drivers/random
endif

ifneq ($(filter BT_AF_SIMU,$(TEST_CASE_LIST)),)
obj-y += ../../services/audioflinger/
obj-y += ../../platform/drivers/codec/
endif

ifneq ($(filter BT_DRV_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../platform/drivers/bt/ ../../utils/heap/
CFLAGS_bt_conn_test.o += -Iplatform/drivers/bt
endif

ifneq ($(filter DPD_BT_TEST,$(TEST_CASE_LIST)),)
CFLAGS_bt_conn_test.o += -DDPD_BT_TEST
endif

ifneq ($(filter EMMC_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../platform/drivers/emmc/
ccflags-y += -DEMMC_TEST -Iplatform/drivers/emmc/host/test
endif

ifneq ($(filter SDIO_HOST_SIGNAL_TEST,$(TEST_CASE_LIST)),)
ifneq ($(PERIPH_PLL_FREQ),)
CFLAGS_sdio_host_signal_test.o += -DPERIPH_PLL_FREQ=$(PERIPH_PLL_FREQ)
endif
endif

ifneq ($(filter SDIO_HOST_FIRMWARE_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../tests/sdio_download_test/
endif

ifneq ($(filter SDIO_HOST_PROGRAMMER_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../tests/sdio_download_test/
endif

ifneq ($(filter PCIE_TEST,$(TEST_CASE_LIST)),)
CFLAGS_pcie_test.o += -DPCIE_TEST
endif

ifneq ($(filter SPI_NORFLASH_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../platform/drivers/spi_norflash/
ccflags-y += -Iplatform/drivers/spi_norflash/
endif

ifneq ($(filter UART_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../utils/crc32/
ccflags-y += -Iutils/crc32
endif

ifneq ($(filter SPI_DEBUG_SLAVE_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../utils/crc32/
ccflags-y += -Iutils/crc32
endif

ifneq ($(filter I2C_SLAVE_TEST,$(TEST_CASE_LIST)),)
ifeq ($(I2C_USE_DMA),1)
CFLAGS_i2c_slave_test.o += -DI2C_USE_DMA
endif
endif

ifneq ($(filter SENSOR_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../tests/sensor_test/
export SENSOR_TEST ?=1
ccflags-y += -DSENSOR_TEST
ccflags-y += -Itests/sensor_test
endif #SENSOR_TEST

ifneq ($(filter I3C_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../tests/sensor_test/
export I3C_TEST ?=1
ccflags-y += -DI3C_TEST
ccflags-y += -Itests/sensor_test
endif

ifneq ($(filter MBW_TEST,$(TEST_CASE_LIST)),)
include $(cur_dir)mbw_flags.mk
endif

ifneq ($(filter GPU_TEST GRAPHIC_TEST, $(TEST_CASE_LIST)), )
obj-y += ../../platform/drivers/graphic/
obj-y += ../../utils/heap/
endif

ifneq ($(filter GPU_TEST,$(TEST_CASE_LIST)),)
obj-y += gpu/
ifeq ($(TEST_WITH_LCD), 1)
obj-y += qspi/
endif
endif

ifneq ($(filter GAMU_TEST,$(TEST_CASE_LIST)),)
ccflags-y += -DGAMU_TEST
endif

ifneq ($(filter GRAPHIC_TEST,$(TEST_CASE_LIST)),)
obj-y += graphic/
ccflags-y += -DDMA2D_TEST
endif

ifneq ($(filter GPADC_TEST,$(TEST_CASE_LIST)),)
obj-y += ../hwtest/gpadc_test.c
endif

ifneq ($(filter HEAP_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../utils/heap/
ccflags-y += -Iutils/heap/
endif

ifneq ($(filter CHARGER_TEST,$(TEST_CASE_LIST)),)
CFLAGS_charger_test.o += -DCHARGER_TEST
endif

ifneq ($(filter COREMARK_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../tests/coremark/
endif

ifneq ($(filter DHRYSTONE_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../tests/dhrystone2/
endif

ifneq ($(filter CPU_MAX_POWER_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../tests/cpu_max_power/
endif

ifneq ($(filter MBEDTLS_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../utils/mbedtls/test/
ccflags-y += -DMBEDTLS_TEST
endif

ifneq ($(filter SHANHAI_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../tests/shanhai/
ccflags-y += -DSHANHAI_TEST
endif

ifneq ($(filter JIAYU_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../tests/jiayu/
ccflags-y += -DJIAYU_TEST
endif

ifneq ($(filter FULLMAP_MMU_TEST,$(TEST_CASE_LIST)),)
CFLAGS_fullmap_mmu_test.o += -DFULLMAP_MMU_TEST
endif

CHIP_PSRAM_CTRL_VER ?= 1
CFLAGS_psram_test.o += -DCHIP_PSRAM_CTRL_VER=$(CHIP_PSRAM_CTRL_VER)

ifneq ($(filter PSRAM_WINDOW_TEST,$(TEST_CASE_LIST)),)
CFLAGS_psram_window_test.o += -DPSRAM_SPEED=$(PSRAM_SPEED)
CFLAGS_psram_window_test.o += -DCHIP_PSRAM_CTRL_VER=$(CHIP_PSRAM_CTRL_VER)
ifeq ($(PSRAM_CALIB_ON_1D),1)
CFLAGS_psram_window_test.o += -DPSRAM_CALIB_ON_1D
endif
ifeq ($(PSRAM_WINDOW_TEST_FT),1)
CFLAGS_psram_window_test.o += -DPSRAM_WINDOW_TEST_FT
endif
endif

ifneq ($(filter PSRAMUHS_WINDOW_TEST,$(TEST_CASE_LIST)),)
CFLAGS_psramuhs_window_test.o += -DPSRAMUHS_SPEED=$(PSRAMUHS_SPEED)
endif

ifeq ($(PSRAM_TEST),1)
ifeq ($(PSRAM_CURRENT_TEST),1)

ccflags-y += -DPSRAM_CURRENT_TEST
ifeq ($(PSRAM_CURRENT_TEST_DMA_RW_LOOP),1)
ccflags-y += -DPSRAM_CURRENT_TEST_DMA_RW_LOOP
endif
ifeq ($(PSRAM_CURRENT_TEST_DMA_RW_1HZ),1)
ccflags-y += -DPSRAM_CURRENT_TEST_DMA_RW_1HZ
endif

endif
endif

ifneq ($(filter WIFI_TEST,$(TEST_CASE_LIST)),)
obj-y += ../../tests/wifi/
ccflags-y += -Itests/wifi
ccflags-y += -DWIFI_TEST
endif

ifneq ($(DEBUG_PORT),)
CFLAGS_hwtest_main.o += -DDEBUG_PORT=$(DEBUG_PORT)
endif

ifeq ($(DEFAULT_BSS_IN_PSRAM),1)
subdir-ccflags-y += -DDEFAULT_BSS_IN_PSRAM
endif

ifneq ($(TEST_CASE_LIST),)
ccflags-y += $(addprefix -D,$(TEST_CASE_LIST))
endif
