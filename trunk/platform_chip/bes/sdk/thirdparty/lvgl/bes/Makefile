export LVGL_DIR := thirdparty/lvgl

ifeq ($(NUTTX_BUILD),1)
# build with nuttx
include $(LVGL_DIR)/bes/nuttx.mk
else
# build with bsp
include $(LVGL_DIR)/bes/aos.mk
endif
