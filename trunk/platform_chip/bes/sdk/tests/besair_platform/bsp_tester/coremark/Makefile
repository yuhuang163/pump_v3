cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)


obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(COREMARK_DHRY_BUILD),gcc)
obj-y += src/
endif

ifeq ($(COREMARK_DHRY_BUILD),iar)
obj-y += coremark_iar.a
endif

ifeq ($(COREMARK_DHRY_BUILD),keil)
obj-y += coremark_keil.a
endif

ccflags-y :=

ccflags-y += -Wno-double-promotion
ccflags-y += -Ofast
