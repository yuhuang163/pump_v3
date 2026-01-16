cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(DISPLAY_BUS),rgb)
obj_c := lcdc_rgbif.c
ifneq ($(LCDCPIX_MHZ),)
subdir-ccflags-y += -DLCDCPIX_MHZ=$(LCDCPIX_MHZ)
endif
else ifeq ($(DISPLAY_BUS),qspi)
obj_c := lcdc_qspi.c
else
obj_c := lcdc_dsi.c
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)
