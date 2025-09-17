cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(CHIP_PSRAM_CTRL_VER),6)
obj-y += psram_best1700/
else ifeq ($(filter 1 2 3 4 5 7 8,$(CHIP_PSRAM_CTRL_VER)),)
obj-y += psram/
else
obj-y += psram_dummy.o

$(obj)/psram_dummy.o :
	$(call if_changed,cc_dummy)

endif
