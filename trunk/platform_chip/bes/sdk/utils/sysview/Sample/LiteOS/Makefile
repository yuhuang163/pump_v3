
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += SEGGER_SYSVIEW_LiteOS.o
obj-y += Config/Cortex-M/SEGGER_SYSVIEW_Config_LiteOS.o

ccflags-y := \
	-Iutils/crash_catcher/include \
	-Iplatform/cmsis/inc/ \
	-Iservices/nv_section/log_section \
	-Iutils/xyzmodem/ \
	-Iutils/system-view/Config/ \
	-Irtos/liteos/liteos_m/ \
	-Irtos/liteos/liteos_m/utils/ \
	-Irtos/liteos/liteos_m/kernel/include/ \
	-Irtos/liteos/liteos_m/arch/include/ \
	-Iinclude/

subdir-ccflags-y := \
	-Iutils/system-view/Config/ \
	-Iutils/system-view/SEGGER/

