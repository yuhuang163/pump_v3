
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := a2dp_decoder.o

ifneq ($(A2DP_SMF),1)
obj-y += a2dp_decoder_sbc.o
ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp_decoder_sbc.o += -O3
endif

ifeq ($(A2DP_AAC_ON),1)
obj-y += a2dp_decoder_aac_lc.o
ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp_decoder_aac_lc.o += -O3
endif
AAC_INCLUDES =  \
	-Imultimedia/inc/
else
AAC_INCLUDES =
endif

ifeq ($(A2DP_SCALABLE_ON),1)
obj-y += a2dp_decoder_scalable.o
ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp_decoder_scalable.o += -O3
endif
SCALABLE_INCLUDES =  \
	-Ithirdparty/audio_codec_lib/scalable
else
SCALABLE_INCLUDES =
endif

ifeq ($(A2DP_LHDC_ON),1)
obj-y += a2dp_decoder_lhdc.o
ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp_decoder_lhdc.o += -O3
endif
LHDC_INCLUDES =  \
	-Ithirdparty/audio_codec_lib/liblhdc-dec/inc
else
LHDC_INCLUDES =
endif

ifeq ($(A2DP_LHDCV5_ON),1)
obj-y += a2dp_decoder_lhdcv5.o
CFLAGS_a2dp_decoder_lhdcv5.o += -O3
LHDC_INCLUDES =  \
	-Ithirdparty/audio_codec_lib/liblhdcv5-dec/inc
endif

ifeq ($(A2DP_LDAC_ON),1)
obj-y += a2dp_decoder_ldac.o
ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp_decoder_ldac.o += -O3
endif
LDAC_INCLUDES =  \
	-Ithirdparty/audio_codec_lib/ldac/inc
else
LDAC_INCLUDES =
endif

ifeq ($(A2DP_LC3_ON),1)
obj-y += a2dp_decoder_lc3.o
ifeq ($(OPT_LEVEL),)
#CFLAGS_a2dp_decoder_lc3.o += -O3
endif
LC3_INCLUDES =  \
	-Imultimedia/inc/audio/codec/lc3/inc
else
LC3_INCLUDES =
endif

else
obj-y += a2dp_decoder_smf.o
ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp_decoder_smf.o += -O3
endif
SMF_INCLUDES :=  
SMF_INCLUDES += -Imultimedia/inc/audio/smf/core/inc 
SMF_INCLUDES += -Imultimedia/inc/audio/smf/core/include
SMF_INCLUDES += -DA2DP_SMF=1
endif

ccflags-y := \
	$(AAC_INCLUDES) \
	$(SCALABLE_INCLUDES) \
	$(LHDC_INCLUDES) \
	$(LDAC_INCLUDES) \
	$(LC3_INCLUDES) \
	$(SMF_INCLUDES) \
	-Iservices/audio_bt \
	-Iservices/audio_process \
	-Iservices/app_ai/inc \
	-Iservices/resources \
	-Iservices/bt_app \
	-Iservices/bt_app/a2dp_codecs/include \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BES_BT_IMPL_INCLUDES) \
	-Iservices/bes_bth/bt/bt_app/ \
	-Iplatform/drivers/uarthci \
	-Iutils/cqueue \
	-Iservices/audio_dump/include \
	-Imultimedia/inc/speech/inc \
	-Imultimedia/inc/rbcodec/inc \
	-Imultimedia/inc/audio/process/eq/include \
	-Imultimedia/inc/audio/process/resample/include \
	-Imultimedia/inc/audio/process/filters/include \
	-Imultimedia/inc/audio/process/sbcplc/include \
	-Imultimedia/inc/fm/inc \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/include \
	-Iservices/overlay \
	-Iservices/norflash_api \
	-Iservices/nv_section/log_section \
	-Iapps/main \
	-Iapps/audioplayers/rbplay/ \
	-Iapps/audioplayers/a2dp_decoder \
	-Iutils/list \
	-Iutils/heap \
	-Iplatform/drivers/ana \
	-Iapps/apptester \
	-Iapps/key  \
	-Iplatform/drivers/bt \
	-Iutils/crc32 \
	-Iservices/ai_voice/audio \
	-Iservices/ai_voice/protocol/bixbyvoice \
	-Iservices/ai_voice/protocol/bixbyvoice/bixbyvoice_manager \
	-Iapps/dsp_m55 \
    -Iservices/audio_manager

ccflags-y += -Imultimedia/inc/

ifeq ($(A2DP_DECODER_CROSS_CORE),1)
obj-y += a2dp_decoder_cc_bth.o
ccflags-y += \
    -Iapps/audioplayers/a2dp_decoder_off_bth

CFLAGS_a2dp_decoder.o += -DA2DP_DECODER_CROSS_CORE
endif

ifeq ($(A2DP_LHDC_ON),1)
ccflags-y += -Ibthost/stack/bt_if/lhdc_license
endif

ifeq ($(A2DP_LHDCV5_ON),1)
ccflags-y += -Ibthost/stack/bt_if/lhdc_license
endif

ifeq ($(IBRT),1)
ifeq ($(IBRT_UI),1)
ccflags-y += \
    -Iservices/ux/common/ibrt/app_ibrt/inc \
    -Iservices/ux/common/ibrt/ibrt_core/inc \
    -Iservices/ux/common/ibrt/custom_api
endif

ifeq ($(IBRT_UI),1)
subdir-ccflags-y += \
    -Iservices/ux/common/ibrt/ibrt_ui/inc
endif

subdir-ccflags-y += \
    -Iservices/ux/common/ibrt/ibrt_middleware/inc
endif

ifeq ($(A2DP_CP_ACCEL),1)
obj-y += a2dp_decoder_cp.o
ccflags-y += -Iplatform/drivers/cp_accel
endif

ifeq ($(A2DP_TRACE_CP_ACCEL),1)
ccflags-y += -DA2DP_TRACE_CP_ACCEL
endif

ifeq ($(A2DP_TRACE_DEC_TIME),1)
ccflags-y += -DA2DP_TRACE_DEC_TIME
endif

ifeq ($(A2DP_TRACE_CP_DEC_TIME),1)
ccflags-y += -DA2DP_TRACE_CP_DEC_TIME
endif

ifeq ($(A2DP_HEAP_DEBUG),1)
CFLAGS_a2dp_decoder.o += -DA2DP_HEAP_DEBUG
endif
ifeq ($(A2DP_CP_HEAP_DEBUG),1)
CFLAGS_a2dp_decoder_cp.o += -DA2DP_CP_HEAP_DEBUG
endif

ifeq ($(PLAYBACK_FORCE_48K),1)
CFLAGS_a2dp_decoder.o += -DPLAYBACK_FORCE_48K
endif

ifeq ($(A2DP_SBC_PLC_ENABLED),1)
CFLAGS_a2dp_decoder_sbc.o += -DA2DP_SBC_PLC_ENABLED
endif

ifeq ($(AUDIO_REVERB),1)
ccflags-y += -D__AUDIO_REVERB__
endif

ifeq ($(A2DP_AUDIO_STEREO_MIX_CTRL),1)
CFLAGS_a2dp_decoder.o += -DA2DP_AUDIO_STEREO_MIX_CTRL
ccflags-y += -Iinclude/cmsis_dsp
endif

ifneq ($(CROSS_CORE_HEAP_SIZE),)
CFLAGS_a2dp_decoder_cc_bth.o += -DCROSS_CORE_HEAP_SIZE=$(CROSS_CORE_HEAP_SIZE)
endif
