cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y +=  \
	-Iapps/usbaudio/ \
	-Iplatform/drivers/usb/usb_dev/inc \
	-Iapps/usbaudio/ \
	-Imultimedia/inc/audio/process/adp/include \
	-Imultimedia/inc/audio/process/anc/include \
	-Imultimedia/inc/audio/process/psap/include \
	-Imultimedia/inc/audio/process/filters/include \
	-Imultimedia/inc/audio/process/resample/include \
	-Iservices/audio_process \
	-Itests/anc_usb/ \
	-Iapps/main/ \
	-Iapps/common/ \
	-Iplatform/drivers/ana/ \
	-Iplatform/hal/ \
	-Iplatform/hal/best2005/ \
	-Iapps/usbaudio/usb_audio_control/

ifeq ($(VOW_SPLIT_L_R_CHANNEL), 1)
subdir-ccflags-y += -D__VOW_SPLIT_L_R_CHANNEL__
endif

ifeq ($(BLE_WIFI_DUAL_CHIP_MODE),1)
ccflags-y += -Inet/vowlan/vow_i2s_dev
endif