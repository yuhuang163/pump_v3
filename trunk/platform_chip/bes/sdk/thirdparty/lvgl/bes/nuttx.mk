
# Disable some compile warnings
subdir-ccflags-y += \
    -Wno-shadow \
    -Wno-float-conversion \
    -Wno-double-promotion

LVGL_FREETYPE ?= 1
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
    -Ithirdparty/svg/inc \
    -Iservices/grabox/inc \
    -Iutils/heap \
    -Iutils/hwtimer_list

obj-y += jpeg/
obj-y += png/
obj-y += etc2/
obj-y += svg/

obj-y += vglite/
obj-y += examples/

obj-y += ../src/
obj-y += ../demos/
obj-y += ../examples/

