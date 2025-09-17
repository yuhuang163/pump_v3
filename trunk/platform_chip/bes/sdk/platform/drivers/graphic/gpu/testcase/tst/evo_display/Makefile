
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))


obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

TEST_EVO_FILE := platform/drivers/graphic/gpu/testcase/tst/evo_display/test.evo

ifeq ($(wildcard $(cur_dir)$(notdir $(TEST_EVO_FILE))),)
$(error Please copy the evo file to $(TEST_EVO_FILE))
endif

$(obj)/test_evo.o : $(TEST_EVO_FILE)
AFLAGS_test_evo.o += -Wa,-Iplatform/drivers/graphic/gpu/testcase/tst/evo_display/

ccflags-y := \
           -Iplatform/drivers/graphic/gpu/testcase/tst/util/Elementary/inc/ \
           -Iplatform/drivers/graphic/gpu/testcase/tst/util/ \
           -Iplatform/drivers/graphic/gpu/ \


