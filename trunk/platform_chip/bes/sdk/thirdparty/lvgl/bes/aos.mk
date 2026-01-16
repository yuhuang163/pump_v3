
subdir-ccflags-y += -DLV_CONF_INCLUDE_SIMPLE=1

ifeq ($(KERNEL),FREERTOS)
subdir-ccflags-y += -DKERNEL_FREERTOS
endif

# Disable some compile warnings
subdir-ccflags-y += \
    -Wno-shadow \
    -Wno-float-conversion \
    -Wno-double-promotion

# LCD panel select
ifeq ($(DISPLAY_LCD),gc9503np)
subdir-ccflags-y += -DDISPLAY_GC9503NP
else ifeq ($(DISPLAY_LCD),st77916)
subdir-ccflags-y += -DDISPLAY_ST77916
else ifeq ($(DISPLAY_LCD),a064)
subdir-ccflags-y += -DDISPLAY_A064
else ifeq ($(DISPLAY_LCD),hx8394f)
subdir-ccflags-y += -DDISPLAY_HX8394F
else ifeq ($(DISPLAY_LCD),hx8394f720)
subdir-ccflags-y += -DDISPLAY_HX8394F720
else ifeq ($(DISPLAY_LCD),hx8399c)
subdir-ccflags-y += -DDISPLAY_HX8399C
else ifeq ($(DISPLAY_LCD),icna3310)
subdir-ccflags-y += -DDISPLAY_ICNA3310
else ifeq ($(DISPLAY_LCD),eqt400)
subdir-ccflags-y += -DDISPLAY_EQT400
else ifeq ($(DISPLAY_LCD),st7102rgb)
subdir-ccflags-y += -DDISPLAY_ST7102RGB
else
subdir-ccflags-y += -DDISPLAY_RM69330
endif

ifneq ($(DISPLAY_RST_PIN),)
subdir-ccflags-y += -DDISPLAY_RST_PIN=$(DISPLAY_RST_PIN)
endif
ifneq ($(DISPLAY_TE_PIN),)
subdir-ccflags-y += -DDISPLAY_TE_PIN=$(DISPLAY_TE_PIN)
endif

# Display BUS select
ifneq ($(filter best1306p best1503,$(CHIP)),)
export DISPLAY_BUS ?= qspi
else ifneq ($(filter best3601,$(CHIP)),)
export DISPLAY_BUS ?= dsi
export LVGL_GPU_VGLITE := 0
else
export DISPLAY_BUS ?= dsi
endif
ifeq ($(DISPLAY_BUS),qspi)
subdir-ccflags-y += -DCONFIG_LCDC_QSPI
else ifeq ($(DISPLAY_BUS),rgb)
subdir-ccflags-y += -DCONFIG_LCDC_RGBIF
else ifeq ($(DISPLAY_BUS),dual_dsi)
subdir-ccflags-y += -DCONFIG_LCDC_DUAL_DSI -DCONFIG_LCDC_DSI
else
subdir-ccflags-y += -DCONFIG_LCDC_DSI
endif

ifeq ($(LVGL_GPU_VGLITE),1)
subdir-ccflags-y += -DLVGL_GPU_VGLITE
endif

ifneq ($(LVGL_COMPRESS_MODE),)
subdir-ccflags-y += -DCONFIG_LCDC_COMPRESS=$(LVGL_COMPRESS_MODE) -DLVGL_GPU_VGLITE
endif

ifneq ($(LVGL_STACK_SIZE),)
subdir-ccflags-y += -DLVGL_STACK_SIZE=$(LVGL_STACK_SIZE)
endif

ifeq ($(ZE7_EVB),1)
subdir-ccflags-y += -DZE7_EVB
endif

ifeq ($(LVGL_GPU_ARM2D),1)
subdir-ccflags-y += -DLVGL_GPU_ARM2D
subdir-ccflags-y += \
    -Iinclude/cmsis_dsp \
    -Ithirdparty/arm2d/bes-aos \
    -Ithirdparty/arm2d/Library/Include
subdir-ccflags-y += \
    -Wno-restrict \
    -Wno-unused-variable
endif

ifeq ($(LVGL_RUN_DEMO_LIST),1)
subdir-ccflags-y += -DLVGL_RUN_DEMO_LIST
endif
ifeq ($(LVGL_RUN_DEMO_STRESS),1)
subdir-ccflags-y += -DLVGL_RUN_DEMO_STRESS
endif
ifeq ($(LVGL_RUN_DEMO_WIDGETS),1)
subdir-ccflags-y += -DLVGL_RUN_DEMO_WIDGETS
endif
ifeq ($(LVGL_RUN_DEMO_MUSIC),1)
subdir-ccflags-y += -DLVGL_RUN_DEMO_MUSIC
endif
ifeq ($(LVGL_RUN_DEMO_BENCHMARK8),1)
subdir-ccflags-y += -DLVGL_RUN_DEMO_BENCHMARK8
endif
ifeq ($(LVGL_RUN_DEMO_VECTOR),1)
subdir-ccflags-y += -DLVGL_RUN_DEMO_VECTOR
endif
ifeq ($(LVGL_RUN_DEMO_IMAGE),1)
subdir-ccflags-y += -DLVGL_RUN_DEMO_IMAGE
endif
ifeq ($(LVGL_RUN_DEMO_FREETYPE),1)
subdir-ccflags-y += -DLVGL_RUN_DEMO_FREETYPE
LVGL_FREETYPE := 1
endif

ifeq ($(LVGL_FREETYPE),1)
export FREETYPE_DIR := thirdparty/freetype
subdir-ccflags-y += \
    -Wno-unused-variable \
    -Wno-logical-op \
    -Wno-dangling-pointer

subdir-ccflags-y += \
    -DLVGL_FREETYPE \
    -I$(LVGL_DIR)/src/libs/freetype \
    -I$(FREETYPE_DIR)/include \
    -I$(FREETYPE_DIR)/fonts \
    -DFT2_BUILD_LIBRARY \
    -DFT_CONFIG_MODULES_H=\<ftmodule.h\> \
    -DFT_CONFIG_OPTIONS_H=\<ftoption.h\>

include $(LVGL_DIR)/bes/freetype.mk
endif

subdir-ccflags-y += \
    -I$(LVGL_DIR) \
    -I$(LVGL_DIR)/src \
    -I$(LVGL_DIR)/demos \
    -I$(LVGL_DIR)/examples \
    -I$(LVGL_DIR)/bes/ports \
    -I$(LVGL_DIR)/bes/disp \
    -I$(LVGL_DIR)/bes/touchscreen \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/graphic \
    -Iplatform/drivers/graphic/gpu \
    -Iplatform/drivers/graphic/gpu/drv/vglite/inc/ \
    -Iplatform/drivers/graphic/gpu/drv/vglite/VGLite/ \
    -Iplatform/drivers/graphic/lcdc \
    -Iplatform/drivers/graphic/fb \
    -Iplatform/drivers/graphic/lcd \
    -Iplatform/drivers/jpeg \
    -Iplatform/cmsis/inc \
    -Iutils/heap \
    -Iutils/hwtimer_list

ifeq ($(LVGL_NO_TOUCHSCREEN),1)
subdir-ccflags-y += -DLVGL_NO_TOUCHSCREEN
else
obj-y += touchscreen/
endif

ifeq ($(LVGL_BES_BAJ),1)
subdir-ccflags-y += -DLVGL_BES_BAJ
subdir-ccflags-y += -Iservices/grabox/inc
export LVGL_BES_JPEG := 1
endif
ifeq ($(LVGL_BES_JPEG),1)
subdir-ccflags-y += -DLVGL_BES_JPEG
obj-y += jpeg/
endif

ifeq ($(LVGL_BES_PNG),1)
subdir-ccflags-y += -DLVGL_BES_PNG
obj-y += png/
endif

ifeq ($(LVGL_BES_ETC2),1)
subdir-ccflags-y += -DLVGL_BES_ETC2
obj-y += etc2/
endif

ifeq ($(LVGL_BES_SVG),1)
subdir-ccflags-y += -DLVGL_BES_SVG
subdir-ccflags-y += -Ithirdparty/svg/inc
obj-y += svg/
endif

ifeq ($(LVGL_SNAPSHOT),1)
subdir-ccflags-y += -DLVGL_SNAPSHOT
endif

# LVGL configuration
ifneq ($(CONFIG_LV_LOG_LEVEL),)
subdir-ccflags-y += -DCONFIG_LV_LOG_LEVEL=$(CONFIG_LV_LOG_LEVEL)
endif

ifneq ($(CONFIG_LV_THEME_DEFAULT_DARK),)
subdir-ccflags-y += -DCONFIG_LV_THEME_DEFAULT_DARK=$(CONFIG_LV_THEME_DEFAULT_DARK)
endif

obj-y += ports/
obj-y += disp/
obj-y += vglite/
obj-y += examples/

obj-y += ../src/
obj-y += ../demos/
obj-y += ../examples/

