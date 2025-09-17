
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	-Iplatform/cmsis/inc/ \
	-Iutils/heap/ \
	-Iutils/lzma/

ifeq ($(CHIP_HAS_PSRAM),1)
ccflags-y += -DCHIP_HAS_PSRAM
endif

ifeq ($(TOOLCHAIN),armclang)
# Disable special handling and optimization of standard C library
# functions for booting codes (before memcpy/memset/... are ready)
CFLAGS_system_compress.o += -fno-builtin
endif
