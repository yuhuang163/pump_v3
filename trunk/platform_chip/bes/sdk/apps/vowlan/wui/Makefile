cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
	-Iapps/vowlan \
	-Iapps/vowlan/wadc \
	-Iapps/vowlan/wusb \
	-Iapps/vowlan/wsink \
	-Iapps/vowlan/i2s \
	-Iapps/vowlan/knob \
	-Iapps/vowlan/wiot \
	-Iapps/vowlan/wbt_audio \
	-Iapps/vowlan/wi2s \
	-Iapps/vowlan/ir \
	-Iutils/ \
	-Iutils/cqueue/ \
	-Inet/ \
	-Iservices/aiot/vowlan/

