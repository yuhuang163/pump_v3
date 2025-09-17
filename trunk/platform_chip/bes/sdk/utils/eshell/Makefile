
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

ESHELL_LIB := libeshell

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

$(ESHELL_LIB)-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y += $(ESHELL_LIB).a

ccflags-y := \
	-Iplatform/cmsis/inc/ \
	-Iutils/eshell \
	-Iutils/cqueue \
	-Iutils/hwtimer_list \
	-Itests/anc_usb \
	-Iplatform/drivers/ana

ifneq ($(ESHELL_AUTORUN),)
subdir-ccflags-y += -DESHELL_AUTORUN="\"$(ESHELL_AUTORUN)\""
endif