
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y += Syscalls/SEGGER_RTT_Syscalls_GCC.o

ccflags-y += \
	-Iutils/crash_catcher/include \
	-Iplatform/cmsis/inc/ \
	-Iservices/nv_section/log_section \
	-Irtos/rtx/TARGET_CORTEX_M/ \
	-Iutils/xyzmodem/ \
	-Iutils/system-view/Config/ \
	-Itests/besair_platform/bsp_tester \
	-Iservices/audio_dump/include \
	-Iinclude/

subdir-ccflags-y += -Iutils/system-view/Config/

asflags-y += -Iutils/system-view/Config/

