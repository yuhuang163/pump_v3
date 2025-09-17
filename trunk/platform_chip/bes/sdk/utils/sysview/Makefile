
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))
obj-y += SEGGER/
ifeq ($(KERNEL),FREERTOS)
obj-y += Sample/FreeRTOSV10/
else ifeq ($(KERNEL),RTX5)
obj-y += Sample/Rtx5/
else ifeq ($(KERNEL),LITEOS_M)
obj-y += Sample/LiteOS/
endif

ccflags-y := \
	-Iutils/crash_catcher/include \
	-Iplatform/cmsis/inc/ \
	-Iservices/nv_section/log_section \
	-Irtos/rtx/TARGET_CORTEX_M/ \
	-Iutils/xyzmodem/ \
	-Iinclude/

