cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))


obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y +=  \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BT_SERVICE_INCLUDES) \
    -Iplatform/drivers/ana \
    -Iservices/audioflinger \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/audio_process \
    -Iapps/apptester \
    -Iapps/factory \
    -Iutils/crc32 \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iapps/audioplayers/rbplay \
    -Iservices/communication \
    -Iutils/cqueue \
    -Iutils/hsm \
    -Iservices/audio_manager \
    -Imultimedia/inc/audio/codec/sbc/inc \
    -Imultimedia/audio/codec/sbc/src/inc \
    -Iservices/interconnection \
    -Iservices/audio_bt \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/userdata_section

