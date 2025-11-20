cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y += ../../../utils/mbedtls/
#noapp and hwtest already include
#obj-y += ../../../utils/heap/

obj-y += ../../../utils/hexdump/

ccflags-y += -Iutils/heap \
             -Iutils/hexdump \
             -Iutils/mbedtls \
             -Iutils/mbedtls/configs \
             -Iutils/mbedtls/library \
             -Iutils/mbedtls/include \
             -Iutils/mbedtls/include/psa \
             -Iutils/mbedtls/include/mbedtls \

ifneq ($(MBEDTLS_CONFIG_FILE),)
ccflags-y += -DMBEDTLS_CONFIG_FILE='"$(MBEDTLS_CONFIG_FILE)"'
endif

CFLAGS_mbedtls_test.o += -DMBEDTLS_TEST
