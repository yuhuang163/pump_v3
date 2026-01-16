cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y = \
            $(CMSIS_DSP_INCLUDE) \
            -Iplatform/drivers/graphic/gpu/drv/vglite/inc/ \
            -Iplatform/drivers/graphic/gpu/drv/vglite/VGLiteKernel/ \
            -Iplatform/drivers/graphic/gpu/drv/vglite/VGLiteKernel/rtos/ \
            -Iplatform/drivers/graphic/gpu/sfs/

ifeq ($(VGLITE_DUMP_PNG), 1)
obj-y += ../../../../../../../thirdparty/lodepng/
ccflags-y += \
             -Ithirdparty/lodepng/ \
             -DVGLITE_DUMP_PNG
endif
