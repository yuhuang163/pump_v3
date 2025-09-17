
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

MEM_TEST_TGT ?= mem_test_pgm
$(obj)/mem_scan_bin.o : out/mem_test/$(MEM_TEST_TGT).bin
AFLAGS_mem_scan_bin.o += -DMEM_TEST_BIN_NAME=$(MEM_TEST_TGT).bin -Wa,-Iout/mem_test/

ccflags-y += \
					-Iutils/boot_struct \
					-Iplatform/hal \
					-Iplatform/drivers/ana \
					 $(BLUETOOTH_ADAPTER_INCLUDES) \
					-Iservices/nv_section/userdata_section \
					-Iinclude/watchdog

ifeq ($(MTEST_BUILTIN_OTA), 1)
ccflags-y += -DMTEST_BUILTIN_OTA
endif
