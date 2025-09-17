cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(LCDC_DMA2D_TEST),1)
ccflags-y += -DFB_GRAPHIC_TEST
ccflags-y += -DDMA2D_TEST
endif

ifeq ($(FB_GRAPHIC_TEST),1)
ccflags-y += -DFB_GRAPHIC_TEST
endif

ccflags-y += \
				   -Itests/hwtest/ \
				   -Iplatform/drivers/graphic/fb \
				   -Iutils/hwtimer_list/ \

