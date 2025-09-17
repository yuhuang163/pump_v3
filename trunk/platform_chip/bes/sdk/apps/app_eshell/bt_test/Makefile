cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o)

ccflags-y += -DUTILS_ESHELL_BT_TEST
subdir-ccflags-y += \
    -Iapps/app_eshell \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana

