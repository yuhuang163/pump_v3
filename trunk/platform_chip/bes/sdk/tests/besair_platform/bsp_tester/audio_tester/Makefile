cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

ccflags-y := \
	-Itests/besair_platform/bsp_tester/audio_tester \
	-Iservices/resources \
	-Iservices/bt_app \
	-Iutils/heap \
	-Iutils/cqueue \
	-Iservices/audioflinger \
	-Iplatform/drivers/ana \
	-Imultimedia/inc/audio/process/filters/include \
	-Imultimedia/inc/audio/process/anc/include \
	-Iservices/nv_section/aud_section

subdir-ccflags-y := \
	-Iinclude/cmsis_dsp \
	-Iservices/nv_section/include \
	-Iutils/hwtimer_list

subdir-ccflags-y += -Iplatform/drivers/ana

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)
obj-y +=	../../../../multimedia/audio/process/filters/src/ \
			../../../../multimedia/audio/process/anc/src/ \


