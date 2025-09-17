
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(RANDOM_DEBUG),1)
CFLAGS_random.o += -DRANDOM_DEBUG
endif

ifeq ($(RAND_OVERRIDE),1)
CFLAGS_random.o += -DRAND_OVERRIDE
endif

ifeq ($(HW_RANDOM),1)
CFLAGS_random.o += -DHW_RANDOM
endif
