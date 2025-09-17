cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += -Iplatform/drivers/usb/usb_host/inc \
             -Iservices/fs/mbed/fat/ \
             -Iservices/fs/mbed/fat/ChaN/ \
             -Iplatform/drivers/ana/ \
             -Iutils/hwtimer_list/ \
             -Iservices/fs/fatfs/ \
             -Imbed/api
