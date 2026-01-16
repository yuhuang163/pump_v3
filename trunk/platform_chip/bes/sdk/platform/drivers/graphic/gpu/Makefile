cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj_c += drv/vglite/

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifneq ($(GPU_MHZ),)
subdir-ccflags-y += -DGPU_MHZ=$(GPU_MHZ)
endif
ifneq ($(GPU_HEAP_SIZE),)
subdir-ccflags-y += -DGPU_HEAP_SIZE=$(GPU_HEAP_SIZE)
else
subdir-ccflags-y += -DGPU_HEAP_SIZE=0x8000
endif
ifneq ($(GPU_CMDBUF_SIZE),)
subdir-ccflags-y += -DGPU_CMDBUF_SIZE=$(GPU_CMDBUF_SIZE)
else
subdir-ccflags-y += -DGPU_CMDBUF_SIZE=0x8000
endif
ifneq ($(GPU_TSBUF_SIZE),)
subdir-ccflags-y += -DGPU_TSBUF_SIZE=$(GPU_TSBUF_SIZE)
else
subdir-ccflags-y += -DGPU_TSBUF_SIZE=0x24000
endif
ifneq ($(GPU_TASK_COUNT),)
subdir-ccflags-y += -DGPU_TASK_COUNT=$(GPU_TASK_COUNT)
else
subdir-ccflags-y += -DGPU_TASK_COUNT=1
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
