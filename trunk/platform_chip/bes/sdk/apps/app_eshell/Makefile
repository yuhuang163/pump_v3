cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))

ifeq ($(ESHELL_UNITTEST),1)
obj_c += unittest/
ESHELL_GRAPHIC_EN := 1
endif

ifeq ($(ESHELL_STRESSTEST),1)
obj_c += stresstest/
ESHELL_GRAPHIC_EN := 1
endif

ifeq ($(UTILS_ESHELL_BT_TEST),1)
obj_c += bt_test/
endif

ifeq ($(POWER_TEST),1)
obj_c += powertest/
endif

ifeq ($(IMS_SUPPORT),1)
obj_c += imstest/
endif

obj_c += system/

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iutils/eshell \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/ana/$(CHIP) \
    -Iapps/common \
    -Iapps/app_rpc \
    -Iapps/app_rpc/rpc_bth_dsp \
    -Iapps/dsp_m55 \
    -Iapps/sensorhub/ \
    -Iplatform/drivers/dsp_m55 \
    -Iplatform/hal

ifeq ($(APP_RPC_BTH_M55_EN),1)
subdir-ccflags-y += -DAPP_RPC_BTH_M55_EN
endif

ifeq ($(CORE_BRIDGE_DEMO_MSG),1)
subdir-ccflags-y += -DCORE_BRIDGE_DEMO_MSG
endif

ifeq ($(ESHELL_GRAPHIC_EN),1)
ifneq ($(filter best1700 best1600 best1502x best1503,$(CHIP)),)

obj-y += ../../tests/display_test/

subdir-ccflags-y += \
    -DTEST_WITH_GPU \
    -DTEST_WITH_LCD

subdir-ccflags-y += \
    -Iutils/heap/ \
    -Iplatform/drivers/graphic \
    -Iplatform/drivers/graphic/lcdc \
    -Iplatform/drivers/graphic/lcd \
    -Iplatform/drivers/graphic/dma2d \
    -Iplatform/drivers/graphic/fb \
    -Iplatform/drivers/graphic/gpu \
    -Iplatform/drivers/graphic/gpu/drv/vglite/inc \
    -Iplatform/drivers/graphic/gpu/drv/vglite/VGLiteKernel \
    -Iplatform/drivers/graphic/osl/ \
    -Irtos/ \
    -Irtos/rtx5/rtx_config/ \
    -Itests/display_test/
endif
endif

ifneq ($(TEST_HAL_PATTERN), TEST_LVGL)
ifeq ($(ENABLE_LVGL), 1)
subdir-ccflags-y += -DENABLE_LVGL
subdir-ccflags-y += \
    -Ithirdparty/lvgl/bes-aos/ \
    -Ithirdparty/lvgl/ \
    -Ithirdparty/lvgl/bes-aos/touchscreen/ \
    -Ithirdparty/lvgl/bes-aos/demo/include/
endif
endif

