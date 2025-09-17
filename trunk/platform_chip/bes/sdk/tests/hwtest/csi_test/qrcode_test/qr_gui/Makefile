
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := lcd_show.o
obj-y += qrcode_gui.o


ccflags-y += \
    -Iplatform/drivers/graphic/lcdc/ \
    -Iplatform/drivers/graphic/ \
    -Iservices/a7_dsp/common/ \
    -Iservices/transq_msg/ \
    -Itests/hwtest/csi_test \
    -Itests/hwtest/csi_test/video_player \
    -Itests/hwtest/csi_test/qrcode_test