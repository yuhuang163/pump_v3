obj-y := usb_dev/

ifeq ($(USB_HOST),1)
obj-y += usb_host/
KBUILD_CPPFLAGS += -DUSB_HOST
endif
