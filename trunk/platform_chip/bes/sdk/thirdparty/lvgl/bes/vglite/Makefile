cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o)

ifeq ($(NUTTX_BUILD),1)
subdir-ccflags-y += -DLV_USE_DRAW_BES_VGLITE=1
subdir-ccflags-y += \
    -Iplatform/drivers/graphic/gpu \
    -Iplatform/drivers/graphic/gpu/drv/vglite/inc \
    -I$(APPDIR)/graphics/lvgl/lvgl/ \
    -I$(APPDIR)/graphics/lvgl/lvgl/src
endif
