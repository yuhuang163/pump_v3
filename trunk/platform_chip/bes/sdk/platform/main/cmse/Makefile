cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y :=

ifeq ($(ARM_CMSE),1)
obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
else ifeq ($(ARM_CMNS),1)
obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*_ns.c))
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ifneq ($(TZ_PROCESS_STACK_SLOTS),)
CFLAGS_tz_context.o += -DTZ_PROCESS_STACK_SLOTS=$(TZ_PROCESS_STACK_SLOTS)
endif
