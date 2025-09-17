cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(COREMARK_DHRY_BUILD),gcc)
obj-y += src/
endif

ifeq ($(COREMARK_DHRY_BUILD),iar)
obj-y += dhry_iar.a
endif

ifeq ($(COREMARK_DHRY_BUILD),keil)
obj-y += dhry_keil.a
endif

TEST_OPT_OPTION := -O$(OPT_LEVEL)
EXTRA_PROMPT :=
ifeq ($(TOOLCHAIN),armclang)
ifeq ($(OMAX),1)
TEST_OPT_OPTION := -Omax
else
ifeq ($(OPT_LEVEL),)
#TEST_OPT_OPTION := -Omax
TEST_OPT_OPTION := -Ofast
ccflags-y += $(TEST_OPT_OPTION)
endif
EXTRA_PROMPT := Recommend building the target with OMAX=1
endif
else # GCC
ifeq ($(OPT_LEVEL),)
TEST_OPT_OPTION := -Ofast
ccflags-y += $(TEST_OPT_OPTION)
endif
endif # GCC
$(info )
$(info $(notdir $(patsubst %/,%,$(cur_dir))) optimization level: $(TEST_OPT_OPTION))
$(info $(EXTRA_PROMPT))
$(info )

ifeq ($(FORCE_TEST_FREQ),1)
ccflags-y += -DFORCE_TEST_FREQ
endif

TEST_FREQ_MHZ ?= 26
ccflags-y += -DTEST_FREQ_MHZ=$(TEST_FREQ_MHZ)

TEST_LOOPS ?= 2
ccflags-y += -DTEST_LOOPS=$(TEST_LOOPS)

NUMBER_OF_RUNS ?= 200000
ccflags-y += -DNUMBER_OF_RUNS=$(NUMBER_OF_RUNS)

TEST_MIN_TIME_US ?= 1000000
ccflags-y += -DTEST_MIN_TIME_US=$(TEST_MIN_TIME_US)
