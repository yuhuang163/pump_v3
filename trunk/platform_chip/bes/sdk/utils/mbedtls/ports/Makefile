
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)tests/*.c))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y := \
	-Iplatform/hal \
	-Iplatform/drivers/random \
	-Iutils/mbedtls/include \
	-Iutils/mbedtls/include/mbedtls \
	-Iutils/mbedtls/library \
	-Iutils/mbedtls/ports/include \
	-Iutils/mbedtls/ports/tests
