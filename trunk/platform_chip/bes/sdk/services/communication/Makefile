cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ifeq ($(USE_KNOWLES),1)
obj-y += comminication_knowles/
endif

ifneq ($(SINGLE_WIRE_UART_PORT),)
ccflags-y += -DSINGLE_WIRE_UART_PORT=$(SINGLE_WIRE_UART_PORT)
endif

ifneq ($(UART1_IOMUX_INDEX),)
ccflags-y += -DUART1_IOMUX_INDEX=$(UART1_IOMUX_INDEX)
endif

ccflags-y += \
	-Iapps/common \
	-Iplatform/drivers/usb/usb_dev/inc \
	-Iplatform/drivers/ana \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iservices/communication/comminication_knowles \
	-Ithirdparty/knowles_uart_lib \
	-Iutils/crc32 \
	-Iservices/app_ai/inc

