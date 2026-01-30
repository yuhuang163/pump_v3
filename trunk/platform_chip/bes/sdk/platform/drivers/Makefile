obj-y := ana/ codec/ norflash/ sbcacc/ spi_flash/ random/ nandflash/

ifneq ($(NO_BT_DRIVER),1)
obj-y += bt/ btpcm/
endif

ifneq ($(filter-out 1 2 3 4,$(CHIP_FLASH_CTRL_VER)),)
ifeq ($(NANDFLASH_SUPPORT),1)
obj-y += nandflash/
endif
endif

ifeq ($(CHIP_HAS_PSRAM),1)
obj-y += psram/
endif

ifeq ($(CHIP_HAS_USB),1)
obj-y += usb/
endif

ifeq ($(CHIP_HAS_USB3),1)
obj-y += usb3/
endif

ifeq ($(CHIP_HAS_FM),1)
obj-y += fm/
endif

ifeq ($(CP_IN_SAME_EE),1)
obj-y += cp_accel/
endif

ifeq ($(CP_AS_SUBSYS),1)
obj-y += cp_subsys/
endif

ifeq ($(SENSOR_HUB),1)
obj-y += sensor_hub/
endif

ifeq ($(DSP_M55),1)
obj-y += dsp_m55/
endif

ifneq ($(filter 1,$(SENS_TRC_TO_MCU) $(BTH_TRC_TO_SYS) \
				$(DSP_M55_TRC_TO_MCU) $(DSP_HIFI4_TRC_TO_MCU) \
				$(CP_SUBSYS_TRC_TO_MCU)),)
obj-y += rmt_trace/
endif

ifeq ($(SENSOR),1)
obj-y += sensor/
endif

ifneq ($(filter 1, $(CHIP_HAS_DMA2D) $(CHIP_HAS_LCDC) $(CHIP_HAS_GPU)), )
obj-y += graphic/
endif

ifeq ($(CHIP_HAS_EMMC),1)
obj-y += emmc/
endif

ifeq ($(BT_RAMRUN_NEW),1)
obj-y += bt_controller/
endif

ifeq ($(CHIP_HAS_HIFI4),1)
obj-y += hifi4/
endif

ifeq ($(CHIP_HAS_JPEG),1)
obj-y += jpeg/
endif

ifeq ($(BT_HOST),1)
ifeq ($(CHIP_SUBSYS),)
obj-y += bt_host/
endif
endif

ifneq ($(filter 1, $(DMA_RPC_CLI) $(DMA_RPC_SVR)),)
obj-y += stream_dma_rpc/
endif

ifeq ($(WIFI_DRV),1)
obj-y += wifi/
endif

ifeq ($(LINUX_FWDRV),1)
obj-y += fw_linux/
endif
