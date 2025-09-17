
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	-Iplatform/drivers/codec \
	-Iplatform/drivers/ana \
	-Itests/besair_platform/bsp_tester/dac_eq_interface

# ifeq ($(AUDIO_RESAMPLE),1)
# CFLAGS_codec_interface.o += -D__AUDIO_RESAMPLE__
# endif

ifeq ($(HIFI4_ADDA_LOOP_TEST),1)
CFLAGS_codec_interface.o += -DHIFI4_ADDA_LOOP_TEST
endif

ifeq ($(HW_DAC_IIR_EQ),1)
CFLAGS_codec_interface.o += -DHW_DAC_IIR_EQ
endif
#-------------------------------
# End
#-------------------------------

