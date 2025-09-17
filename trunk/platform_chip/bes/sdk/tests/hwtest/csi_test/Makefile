cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))

VIDEO_PLAYER ?= 1

ccflags-y += -Itests/hwtest \
	-Itests/hwtest/display_resource \
	-Itests/hwtest/csi_test \
	-Itests/hwtest/csi_test/inc \
	-Itests/hwtest/csi_test/video_player \
	-Iplatform/hal \
	-Iplatform/drivers/ana \
	-Iservices/a7_dsp/common \
	-Iutils/hwtimer_list \
	-Iutils/heap \
	-Iutils/cqueue \
	-Iservices/fs \
	-Inet/ims/webrtc/modules/video_coding/codecs/h264/src/

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

obj-y += \
	video_player/video_player.o \
	video_player/video_misc.o \
	video_player/video_stream.o \
	video_player/video_stream_rpc.o \
	video_player/video_record.o \
	video_player/camera_drv.o \
	video_player/camera_dump.o \
	video_player/camera_sens_gc2145.o \
	video_player/camera_sens_sc132gs.o \
	video_player/display_drv_a064.o \
	video_player/display_drv.o \
	video_player/lcdc_driver.o \

ifeq ($(NOAPP), 1)
obj-y += ../../../utils/cqueue/
obj-y += ../../../utils/heap/
ccflags-y += -DNOAPP
endif

ifeq ($(SIMU),1)
ccflags-y += -DSIMU
endif

ifeq ($(RTOS),1)
ccflags-y += -DRTOS
endif

ifeq ($(PSRAMUHS_ENABLE),1)
ccflags-y += -DPSRAMUHS_ENABLE
endif

ifeq ($(PSRAM_ENABLE),1)
ccflags-y += -DPSRAM_ENABLE
endif

ifeq ($(GX6D),1)
ccflags-y += -DGX6D
else ifeq ($(AX4F),1)
ccflags-y += -DAX4F
endif

ifeq ($(VIDEO_PLAYER),1)
ccflags-y += -DVIDEO_PLAYER
endif

ifeq ($(CHIP_ROLE_DSP),1)
obj-y += ../../../utils/heap/
ccflags-y += -DCHIP_ROLE_DSP
MEDIA_HEAP_ENABLE         ?= 1
MEDIA_HEAP_USE_STATIC_BUF ?= 0
DSP_VPLAYER_APP_USE_HEAP  ?= 1
endif

ifeq ($(CHIP_ROLE_AP),1)
ccflags-y += -DCHIP_ROLE_AP
MEDIA_HEAP_ENABLE         ?= 1
MEDIA_HEAP_USE_STATIC_BUF ?= 0
AP_VPLAYER_APP_USE_HEAP   ?= 1
endif

################ DISPLAY DEVICE OPTION ################
DISPLAY_DEVICE       ?= A064

ifeq ($(DISPLAY_DEVICE),A064)
DISPLAY_IMAGE_WIDTH  ?= 480
DISPLAY_IMAGE_HEIGHT ?= 480
DSI_SENS_A064        ?= 1
DSI_FPS_STAT_ENABLE  ?= 1
else
$(error Invalid DISPLAY_DEVICE=$(DISPLAY_DEVICE))
endif

ifneq ($(DISPLAY_IMAGE_WIDTH),)
ccflags-y += -DDISPLAY_IMAGE_WIDTH=$(DISPLAY_IMAGE_WIDTH)
endif

ifneq ($(DISPLAY_IMAGE_HEIGHT),)
ccflags-y += -DDISPLAY_IMAGE_HEIGHT=$(DISPLAY_IMAGE_HEIGHT)
endif

ifeq ($(DSI_SENS_A064),1)
ccflags-y += -DDSI_SENS_A064
endif

ifeq ($(DSI_FPS_STAT_ENABLE),1)
ccflags-y += -DDSI_FPS_STAT_ENABLE
endif

ifeq ($(DSI_TEST),1)
ccflags-y += -DDSI_TEST
endif

ifeq ($(DSI_IRQ_DEBUG),1)
ccflags-y += -DDSI_IRQ_DEBUG
endif

ifeq ($(DSI_FB_DEBUG),1)
ccflags-y += -DDSI_FB_DEBUG
endif

################ CAMERA DEVICE OPTION ################
CSI_SENS_GC2145         ?= 1
CSI_SENS_SC132GS        ?= 1

ifneq ($(M33_H264_TEST),1)
CSI_SENS_CALIB          ?= 1
endif

CSI_USE_GPIO_TRIG       ?= 1
CSI_FPS_STAT_SLOW       ?= 1
GC2145MIPI_2LANE        ?= 1

CAMERA_DATA_RAW10_TO_YUV ?= 1
CAMERA_USE_SOFT_ISP      ?= 1

ifneq ($(GC2145_IMAGE_WIDTH),)
ccflags-y += -DGC2145_IMAGE_WIDTH=$(GC2145_IMAGE_WIDTH)
endif

ifneq ($(GC2145_IMAGE_HEIGHT),)
ccflags-y += -DGC2145_IMAGE_HEIGHT=$(GC2145_IMAGE_HEIGHT)
endif

ifneq ($(SC132GS_IMAGE_WIDTH),)
ccflags-y += -DSC132GS_IMAGE_WIDTH=$(SC132GS_IMAGE_WIDTH)
endif

ifneq ($(SC132GS_IMAGE_HEIGHT),)
ccflags-y += -DSC132GS_IMAGE_HEIGHT=$(SC132GS_IMAGE_HEIGHT)
endif

ifeq ($(CAMERA_IMAGE_DUMP),1)
ccflags-y += -DCAMERA_IMAGE_DUMP
CAMERA_IMAGE_DUMP_NUM ?= 3
ccflags-y += -DCAMERA_IMAGE_DUMP_NUM=$(CAMERA_IMAGE_DUMP_NUM)
else
CSI_FPS_STAT_ENABLE ?= 1
endif

ifeq ($(CAMERA_DATA_DUMP),1)
ccflags-y += -DCAMERA_DATA_DUMP
CAMERA_DATA_DUMP_NUM ?= 512
ccflags-y += -DCAMERA_DATA_DUMP_NUM=$(CAMERA_DATA_DUMP_NUM)
endif

ifeq ($(CAMERA_DATA_RAW10_TO_YUV),1)
ccflags-y += -DCAMERA_DATA_RAW10_TO_YUV
endif

ifeq ($(CAMERA_USE_SOFT_ISP),1)
ccflags-y += -DCAMERA_USE_SOFT_ISP
endif

ifeq ($(CAMERA_DUMMY_DEVICE),1)
ccflags-y += -DCAMERA_DUMMY_DEVICE
endif

ifeq ($(CAMERA_TEST_MODE_INIT_ON),1)
ccflags-y += -DCAMERA_TEST_MODE_INIT_ON
endif

ifeq ($(CAMERA_TEST_MODE_ENABLE),1)
ccflags-y += -DCAMERA_TEST_MODE_ENABLE
endif

ifeq ($(CAMERA_TEST_DATA_CHECK),1)
ccflags-y += -DCAMERA_TEST_DATA_CHECK
endif

ifeq ($(CSI_SLT_TEST),1)
ccflags-y += -DCSI_SLT_TEST
endif

ifeq ($(CSI_SENS_CALIB),1)
ccflags-y += -DCSI_SENS_CALIB
endif

ifeq ($(CSI_SENS_GC2145),1)
ccflags-y += -DCSI_SENS_GC2145
endif

ifeq ($(CSI_SENS_SC132GS),1)
ccflags-y += -DCSI_SENS_SC132GS
endif

ifeq ($(CSI_FPS_STAT_ENABLE),1)
ccflags-y += -DCSI_FPS_STAT_ENABLE
endif

ifeq ($(CSI_FPS_STAT_SLOW),1)
ccflags-y += -DCSI_FPS_STAT_SLOW
endif

CSI_SENS_AEC_ENABLE ?= 1
ifeq ($(CSI_SENS_AEC_ENABLE),1)
ccflags-y += -DCSI_SENS_AEC_ENABLE
endif

ifeq ($(CSI_USE_GPIO_TRIG),1)
ccflags-y += -DCSI_USE_GPIO_TRIG
endif

ifeq ($(CSI_PHY_NO_CALIB),1)
ccflags-y += -DCSI_PHY_NO_CALIB
endif

ifeq ($(GC2145MIPI_2LANE),1)
ccflags-y += -DGC2145MIPI_2LANE
endif

ifeq ($(CSI_TEST),1)
ccflags-y += -DCSI_TEST
endif

ifeq ($(CSI_DSI_LOOP),1)
ccflags-y += -DCSI_DSI_LOOP
endif

ifeq ($(CSI_DSI_FT_LOOP_TEST),1)
ccflags-y += -DCSI_DSI_FT_LOOP_TEST
endif

ifeq ($(DSI_CLOCK_TEST),1)
ccflags-y += -DDSI_CLOCK_TEST
endif

ifeq ($(MTEST_ENABLED),1)
ccflags-y += -DMTEST_ENABLED
endif

ifeq ($(MTEST_OPEN_I2C_ONLY),1)
ccflags-y += -DMTEST_OPEN_I2C_ONLY
endif

ifeq ($(MTEST_WAIT_CMD),1)
ccflags-y += -DMTEST_WAIT_CMD
endif

ifeq ($(CSI_DEBUG),1)
ccflags-y += -DCSI_DEBUG
endif

ifeq ($(CSI_IRQ_DEBUG),1)
ccflags-y += -DCSI_IRQ_DEBUG
endif

ifeq ($(CSI_FB_DEBUG),1)
ccflags-y += -DCSI_FB_DEBUG
endif

ifeq ($(CSI_SENS_I2C_DEBUG),1)
ccflags-y += -DCSI_SENS_I2C_DEBUG
endif

CSI_NO_TRACE ?= 1
ifeq ($(CSI_NO_TRACE),1)
ccflags-y += -DCSI_NO_TRACE
endif

ifeq ($(CSI_SENS_TEST_PATT),1)
ccflags-y += -DCSI_SENS_TEST_PATT
endif

ifneq ($(CSI_DATA_DUMP_SIZE),)
ccflags-y += -DCSI_DATA_DUMP_SIZE=$(CSI_DATA_DUMP_SIZE)
endif

################ APPLICATION OPTION ################
VPLAYER_APP_DEBUG         ?= 0

ifeq ($(VIDEO_RECORD_ENABLE),1)
ccflags-y += -DVIDEO_RECORD_ENABLE
endif

ifeq ($(VPLAYER_TEST),1)
ccflags-y += -DVPLAYER_TEST
endif

ifneq ($(VPLAYER_APP_DEBUG),)
ccflags-y += -DVPLAYER_APP_DEBUG=$(VPLAYER_APP_DEBUG)
ccflags-y += -DDSP_VPLAYER_APP_DEBUG=$(VPLAYER_APP_DEBUG)
ccflags-y += -DAP_VPLAYER_APP_DEBUG=$(VPLAYER_APP_DEBUG)
endif

ifeq ($(BOOT_IMAGE_ONLY),1)
ccflags-y += -DBOOT_IMAGE_ONLY
endif

ifeq ($(CAMERA_TEST_ONLY),1)
ccflags-y += -DCAMERA_TEST_ONLY
endif

ifeq ($(DISPLAY_TEST_ONLY),1)
ccflags-y += -DDISPLAY_TEST_ONLY
endif

ifeq ($(DSP_VPLAYER_APP_USE_HEAP),1)
ccflags-y += -DDSP_VPLAYER_APP_USE_HEAP
endif

ifeq ($(AP_VPLAYER_APP_USE_HEAP),1)
ccflags-y += -DAP_VPLAYER_APP_USE_HEAP
endif

ifeq ($(MEDIA_HEAP_ENABLE),1)
ccflags-y += -DMEDIA_HEAP_ENABLE
endif

ifeq ($(MEDIA_HEAP_USE_STATIC_BUF),1)
ccflags-y += -DMEDIA_HEAP_USE_STATIC_BUF
endif

ifeq ($(H264_TEST),1)
ccflags-y += -DH264_TEST
endif

ifeq ($(M33_H264_TEST),1)
ccflags-y += -DM33_H264_TEST
endif
ifeq ($(H264_STERAM_A7_TO_M33),1)
ccflags-y += -DH264_STERAM_A7_TO_M33
endif

ifeq ($(CSI_ENCODER_320_240),1)
ccflags-y += -DCSI_ENCODER_320_240
endif

ifeq ($(CSI_ENCODER_640_480),1)
ccflags-y += -DCSI_ENCODER_640_480
endif

ifeq ($(QRCODE_TEST),1)
ccflags-y += -DQRCODE_TEST
ccflags-y += \
	-Itests/hwtest/csi_test/qrcode_test \
	-Itests/hwtest/csi_test/qrcode_test/qr_gui
obj-y += qrcode_test/
endif

ifeq ($(SC132GS_CONTINUE_CLOCK_MODE),1)
ccflags-y += -DSC132GS_CONTINUE_CLOCK_MODE
endif

ifeq ($(SC132GS_STREAM_CHECK_ERR),1)
ccflags-y += -DSC132GS_STREAM_CHECK_ERR
endif

LCDC_DRV_FLAGS :=

ifeq ($(CONFIG_SWAP_DSI_CLK_PN),1)
LCDC_DRV_FLAGS += -DCONFIG_SWAP_DSI_CLK_PN
endif

ifeq ($(CONFIG_DSI_2LANE_MODE),1)
LCDC_DRV_FLAGS += -DCONFIG_DSI_2LANE_MODE
endif

ifeq ($(CONFIG_DSI_VIDEO_MODE),1)
LCDC_DRV_FLAGS += -DCONFIG_DSI_VIDEO_MODE
endif

ifeq ($(CHIP_BEST2003),1)
LCDC_DRV_FLAGS += -DCHIP_BEST2003
endif

ifeq ($(LCD_CSI_DMA_MODE),1)
LCDC_DRV_FLAGS += -DLCD_CSI_DMA_MODE
endif

ifeq ($(PSRAM_ENABLE),1)
LCDC_DRV_FLAGS += -DPSRAM_ENABLE
endif

ifeq ($(PSRAMUHS_ENABLE),1)
LCDC_DRV_FLAGS += -DPSRAMUHS_ENABLE
endif

LCDC_DRIVER ?= 1
ifeq ($(LCDC_DRIVER),1)
LCDC_DRV_FLAGS += -DLCDC_DRIVER
endif

ccflags-y += $(LCDC_DRV_FLAGS)
