
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += SEGGER_SYSVIEW_Rtx5.o
obj-y += Config/Cortex-M/SEGGER_SYSVIEW_Config_Rtx5.o

ccflags-y := \
	-Iutils/crash_catcher/include \
	-Iplatform/cmsis/inc/ \
	-Iservices/nv_section/log_section \
	-Irtos/rtx/TARGET_CORTEX_M/ \
	-Iutils/xyzmodem/ \
	-Iutils/system-view/Config/ \
	-Iinclude/

subdir-ccflags-y := -Iutils/system-view/Config/ \
					-Iutils/system-view/SEGGER/

