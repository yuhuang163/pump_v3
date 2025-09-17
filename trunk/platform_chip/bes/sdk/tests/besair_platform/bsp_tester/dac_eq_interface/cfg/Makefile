obj-y := eq_cfg.o

ifeq ($(AUDIO_RESAMPLE),1)
CFLAGS_eq_cfg.o += -D__AUDIO_RESAMPLE__
endif

ccflags-y := \
	-Iinclude/cmsis_dsp \
	-Itests/besair_platform/bsp_tester/dac_eq_interface/
