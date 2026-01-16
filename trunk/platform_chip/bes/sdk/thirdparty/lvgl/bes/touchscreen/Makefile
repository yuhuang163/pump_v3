cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

#obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

ifeq ($(LVGL_TP_CHSC6X),1)
KBUILD_CPPFLAGS += -DLVGL_TP_CHSC6X
subdir-ccflags-y += -Ithirdparty/user_driver/touch/chsc6x
endif


ifeq ($(LVGL_TP_TMA525C),1)
obj_c +=  bes_tp_tma525c.c
KBUILD_CPPFLAGS += -DLVGL_TP_TMA525C
endif

ifeq ($(LVGL_TP_ZINITIX),1)
obj_c +=  bes_tp_zinitix.c
KBUILD_CPPFLAGS += -DLVGL_TP_ZINITIX
endif

ifeq ($(LVGL_TP_NT38350),1)
obj_c +=  bes_tp_nt38350.c
KBUILD_CPPFLAGS += -DLVGL_TP_NT38350
endif

obj_c += bes_tp_hal.c

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

