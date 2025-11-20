
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	-Iutils/mbedtls \
	-Iutils/mbedtls/configs \
	-Iutils/mbedtls/library \
	-Iutils/mbedtls/include \
	-Iutils/mbedtls/include/mbedtls \
	-Iutils/mbedtls/include/psa

ccflags-y += -Wall -Wextra -Wformat=2 \
			 -Wno-format-nonliteral \
			 -Wno-unused-parameter \
			 -Wno-unused-but-set-variable \
			 -Wno-error=unused-but-set-variable \
