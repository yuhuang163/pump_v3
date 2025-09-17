
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)


IMG_BIN_FILE := platform/drivers/graphic/gpu/testcase/tst/resource/landscape.bin
ifeq ($(wildcard $(cur_dir)$(notdir $(IMG_BIN_FILE))),)
$(error Please copy the binary file to $(IMG_BIN_FILE))
endif
$(obj)/landscape_bin.o : $(IMG_BIN_FILE)
AFLAGS_landscape_bin.o += -Iplatform/drivers/graphic/gpu/testcase/tst/resource/


IMG_BIN_NV12_FILE := platform/drivers/graphic/gpu/testcase/tst/resource/nv12.bin
ifeq ($(wildcard $(cur_dir)$(notdir $(IMG_BIN_NV12_FILE))),)
$(error Please copy the binary file to $(IMG_BIN_NV12_FILE))
endif
$(obj)/nv12_bin.o : $(IMG_BIN_NV12_FILE)
AFLAGS_nv12_bin.o += -Iplatform/drivers/graphic/gpu/testcase/tst/resource/


IMG_BIN_NV16_FILE := platform/drivers/graphic/gpu/testcase/tst/resource/nv16.bin
ifeq ($(wildcard $(cur_dir)$(notdir $(IMG_BIN_NV16_FILE))),)
$(error Please copy the binary file to $(IMG_BIN_NV16_FILE))
endif
$(obj)/nv16_bin.o : $(IMG_BIN_NV16_FILE)
AFLAGS_nv16_bin.o += -Iplatform/drivers/graphic/gpu/testcase/tst/resource/


IMG_BIN_YV12_FILE := platform/drivers/graphic/gpu/testcase/tst/resource/yv12.bin
ifeq ($(wildcard $(cur_dir)$(notdir $(IMG_BIN_YV12_FILE))),)
$(error Please copy the binary file to $(IMG_BIN_YV12_FILE))
endif
$(obj)/yv12_bin.o : $(IMG_BIN_YV12_FILE)
AFLAGS_yv12_bin.o += -Iplatform/drivers/graphic/gpu/testcase/tst/resource/

ccflags-y := \
           -Iplatform/drivers/graphic/gpu/testcase/tst/util/
