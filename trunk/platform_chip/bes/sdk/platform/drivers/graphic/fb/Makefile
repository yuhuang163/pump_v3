
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := -Itests/hwtest/ \

ifeq ($(FB_DSI_60HZ_TEST_CURRENT_TEST_1HZ_REFRESH),1)
ccflags-y += -DFB_DSI_60HZ_TEST_CURRENT_TEST_1HZ_REFRESH
endif
