cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))

src_obj := $(obj_c:.c=.o)

GRABOX_LIB_NAME := libgrabox

$(GRABOX_LIB_NAME)-y := $(src_obj)

obj-y := $(GRABOX_LIB_NAME).a

subdir-ccflags-y += \
            -Iplatform/drivers/graphic/gpu/drv/vglite/inc \
            -Iplatform/drivers/graphic/gpu/drv/vglite/VGLiteKernel/ \
            -Iplatform/drivers/graphic/gpu/ \
            -Iplatform/drivers/graphic/ \
            -Iplatform/drivers/jpeg/ \
            -Iservices/grabox/inc/ \
            -Iutils/heap/

