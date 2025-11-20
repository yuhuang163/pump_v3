cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y :=

#ifneq ($(NUTTX_BUILD),1)
obj-y += C/LzmaDec.c
#endif

obj-y += bes_lzma_api.c  lzma_decode.c bes_lzma_dec_code.c
#obj-y := bes_lzma_api.c lzma_decode.c

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

LIC_FILE_PATTERN := NOTICE

ccflags-y := \
	-Iplatform/cmsis/inc/ \
	-Iutils/heap/ \
	-Iutils/lzma/C \
    -Iapps/audioplayers \
    -Iapps/battery \
    -Iapps/common \
    -Iapps/main \
    -Iapps/factory \
    -Iapps/pwl \
    -Iapps/key \
    -Iutils/boot_struct \
	-Iutils/crc16   \
    -Iutils/crc32   \
    -Iutils/heap    \
    -Iutils/lzma    \
    -Iservices/bt_app \
	-Iservices/nv_section/factory_section \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iservices/ble_stack/ble_ip \
    -Iservices/resources \
    -Iservices/overlay \
    -Iservices/osif \
    -Iservices/norflash_api \
	-Iservices/bt_if_enhanced \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iplatform/hal \
	-Itests/ota_boot/inc \
    -Iutils/hwtimer_list \
	-Iutils/cqueue \
	-Iutils/intersyshci \
	-Iutils/retention_ram \
	-Iplatform/hal/best1000 \
	-Iplatform/drivers/norflash \
	-Iutils/boot_struct \
	-Imbed/api \
	-Iinclude

ifeq ($(NOSTD),1)
ccflags-y += \
	-Iutils/libc/inc/
endif

