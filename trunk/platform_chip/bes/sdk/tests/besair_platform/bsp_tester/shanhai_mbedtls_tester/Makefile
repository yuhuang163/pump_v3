cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ifeq ($(USE_SHANHAI_LIB),1)
obj-y += shanhailib.a
else
obj-y += ../../../../thirdparty/shanhai/
endif

obj-y += ../../../../utils/WjCryptLib/

ccflags-y += \
	-Iutils/hexdump \
    -Iutils/WjCryptLib/lib \
	-Ithirdparty/shanhai/pal \
	-Ithirdparty/shanhai/tee/spe/mbedtls \
	-Ithirdparty/shanhai/tee/spe/mbedtls/configs \
	-Ithirdparty/shanhai/tee/spe/mbedtls/library \
	-Ithirdparty/shanhai/tee/spe/mbedtls/library/dubhe_driver \
	-Ithirdparty/shanhai/tee/spe/mbedtls/library/dubhe_driver/inc \
	-Ithirdparty/shanhai/tee/spe/mbedtls/library/dubhe_driver/inc/common \
	-Ithirdparty/shanhai/tee/spe/mbedtls/library/dubhe_driver/inc/crypto \
	-Ithirdparty/shanhai/tee/spe/mbedtls/include \
	-Ithirdparty/shanhai/tee/spe/mbedtls/include/mbedtls \
	-Ithirdparty/shanhai/tee/spe/mbedtls/include/psa

ccflags-y += \
    -DDUBHE_SECURE  \
    -DARM_CE_DUBHE_ACA \
    -DARM_CE_DUBHE_SCA \
    -DARM_CE_DUBHE_HASH \
    -DARM_CE_DUBHE_TRNG \

ifneq ($(MBEDTLS_CONFIG_FILE),)
ccflags-y += -DMBEDTLS_CONFIG_FILE='"$(MBEDTLS_CONFIG_FILE)"'
endif
