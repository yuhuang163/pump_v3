obj-y := CMSIS/RTOS2/FreeRTOS/ Source/

OS_HAVE_POSIX ?= 0
ifeq ($(OS_HAVE_POSIX),1)
obj-y +=  POSIX/
subdir-ccflags-y += -DOS_HAVE_POSIX=1
endif

subdir-ccflags-y += -Iplatform/cmsis/inc -Iplatform/hal
