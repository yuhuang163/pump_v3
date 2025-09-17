ifeq ($(KERNEL),RTX)
rtos_lib-y := rtx/TARGET_CORTEX_M/
else ifeq ($(KERNEL),RTX5)
rtos_lib-y := rtx5/
else ifeq ($(KERNEL),FREERTOS)
rtos_lib-y := freertos/
else ifeq ($(KERNEL),NUTTX)
rtos_lib-y := nuttx/
else ifeq ($(KERNEL),RTTHREAD)
rtos_lib-y := rt_thread/
KBUILD_CFLAGS += -Wno-error=date-time
else ifeq ($(KERNEL),LITEOS_M)
rtos_lib-y := liteos/liteos_m/
else ifeq ($(KERNEL),LITEOS_A)
rtos_lib-y := liteos/liteos_a/
else ifeq ($(KERNEL),RHINO)
rtos_lib-y := rhino/
endif

ifeq ($(MBED),1)
rtos_lib-y += rtos/
endif

ifeq ($(TOOLCHAIN),armclang)
obj-y := $(rtos_lib-y)
else
obj-y := rtos_lib.o
endif

subdir-ccflags-y += -Iplatform/cmsis/inc -Iplatform/hal -Iutils/hwtimer_list

ifneq ($(OS_CLOCK_NOMINAL),)
subdir-ccflags-y += -DOS_CLOCK_NOMINAL=$(OS_CLOCK_NOMINAL)
endif

ifeq ($(OSTICK_USE_FAST_TIMER),1)
subdir-ccflags-y += -DOSTICK_USE_FAST_TIMER
endif

ifeq ($(CPU_PC_DUMP),1)
subdir-asflags-y += -DCPU_PC_DUMP
endif
