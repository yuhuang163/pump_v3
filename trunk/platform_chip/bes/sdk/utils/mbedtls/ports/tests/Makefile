
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
    -Iutils/mbedtls/library \
    -Iutils/mbedtls/include \
    -Iutils/mbedtls/include/mbedtls \
    -Iplatform/hal \
    -Iutils/mbedtls/ports \
    -Iutils/mbedtls/ports/include
