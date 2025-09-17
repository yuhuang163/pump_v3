cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

BIN_FILE := platform/drivers/graphic/pngdec/test/png_image.bin

ifeq ($(wildcard $(cur_dir)$(notdir $(BIN_FILE))),)
$(error Please copy the png image binary file to $(BIN_FILE))
endif

ccflags-y += -DLODEPNG_NO_COMPILE_ANCILLARY_CHUNKS

$(obj)/png_image_bin.o : $(BIN_FILE)
AFLAGS_png_image_bin.o += -Iplatform/drivers/graphic/pngdec/test/


ccflags-y += \
	-Iplatform/drivers/graphic/osl_rtx5/ \
    -Iplatform/drivers/graphic/osl/ \
    -Iinclude/rtos/rtx5/
