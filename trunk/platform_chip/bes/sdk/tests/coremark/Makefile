cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y :=

ccflags-y += -Wno-double-promotion

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

TEST_LOOPS ?= 1
ccflags-y += -DTEST_LOOPS=$(TEST_LOOPS)

ifneq ($(ITERATIONS),)
ccflags-y += -DITERATIONS=$(ITERATIONS)
endif

TEST_FREQ_MHZ ?= 26
ccflags-y += -DTEST_FREQ_MHZ=$(TEST_FREQ_MHZ)

ifeq ($(CORE_DEBUG),1)
ccflags-y += -DCORE_DEBUG
endif
