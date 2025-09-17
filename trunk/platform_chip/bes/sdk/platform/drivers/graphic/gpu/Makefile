cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj_c += drv/vglite/

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifneq ($(GPU_MHZ),)
subdir-ccflags-y += -DGPU_MHZ=$(GPU_MHZ)
endif

subdir-ccflags-y += \
				   -Iplatform/drivers/graphic/gpu/ \
				   -Iplatform/drivers/graphic/gpu/drv/vglite/inc \
				   -Iutils/heap/ \
				   -Iplatform/drivers/graphic/gpu/drv/vglite/VGLiteKernel/ \
				   -Iplatform/drivers/graphic/gpu/drv/vglite/VGLiteKernel/rtos/ \
				   -Irtos/ \
				   -Irtos/rtx5/rtx_config/ \
				   -Iinclude/rtos/rtx5/

subdir-ccflags-y += \
    -Wno-implicit-fallthrough
