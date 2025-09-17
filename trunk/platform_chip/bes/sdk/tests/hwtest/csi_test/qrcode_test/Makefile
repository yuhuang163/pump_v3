cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y :=

ifeq ($(CHIP_ROLE_DSP),1)
ccflags-y += -DCHIP_ROLE_DSP
LIB_BIN_DIR := tests/hwtest/csi_test/qrcode_test/zbar/lib
obj-y += libzbar.a
obj-y += dsp_qrcode_process.o
obj-y += zbar_detector.o

obj-y += qr_gui/
endif

ifeq ($(CHIP_ROLE_AP),1)
ccflags-y += -DCHIP_ROLE_AP
obj-y += ap_qrcode_app.o
endif


ifeq ($(QRCODE_TEST),1)
ccflags-y += -DQRCODE_TEST
endif

ifeq ($(PSRAMUHS_ENABLE),1)
ccflags-y += -DPSRAMUHS_ENABLE
endif

ifeq ($(PSRAM_ENABLE),1)
ccflags-y += -DPSRAM_ENABLE
endif

ccflags-y += \
    -Iplatform/drivers/graphic/lcdc/ \
    -Iplatform/drivers/graphic/ \
    -Iplatform/drivers/graphic/ \
    -Iservices/a7_dsp/common/ \
    -Iservices/transq_msg/ \
    -Itests/hwtest/csi_test \
    -Itests/hwtest/csi_test/video_player \
    -Itests/hwtest/csi_test/qrcode_test/qr_gui \
    -Itests/hwtest/csi_test/qrcode_test/zbar/include \
    -Iapps/key
