/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
/*******************************************************************************
** namer    : Audio Process
** description  : Manage auido process algorithms, include :hw iir eq, sw iir eq,
**                  hw fir eq, drc...
** version  : V1.0
** author   : Yunjie Huo
** modify   : 2018.12.4
** todo     : NULL
** MIPS     :   NO PROCESS: 34M(TWS, one ear, Mono, AAC, 48k)
**              DRC1: 36M(3 bands)
**              DRC2: 12M
*******************************************************************************/
/*
Audio Flow:
    DECODE --> SW IIR EQ --> DRC --> LIMTER --> VOLUME --> HW IIR EQ --> SPK

                                                       +-----------------------------+
                                                       |             DAC             |
                                                       |                             |
+--------+     +-----------+    +-----+    +--------+  | +--------+    +-----------+ |  +-----+
|        | PCM |           |    |     |    |        |  | |        |    |           | |  |     |
| DECODE +---->+ SW IIR EQ +--->+ DRC +--->+ LIMTER +--->+ VOLUME +--->+ HW IIR EQ +--->+ SPK |
|        |     |           |    |     |    |        |  | |        |    |           | |  |     |
+--------+     +-----------+    +-----+    +--------+  | +--------+    +-----------+ |  +-----+
                                                       +-----------------------------+

| ------------ | ------------------------- | -------- | ----------- |
| Algorithm    | description               | MIPS(M)  | RAM(kB)     |
| ------------ | ------------------------- | -------- | ----------- |
| DRC          | Dynamic Range Compression | 12M/band | 13          |
| Limiter/DRC2 | Limiter                   | 12M      | 5           |
| EQ           | Equalizer                 | 1M/band  | Almost zero |
*/

#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "string.h"
#include "audio_process.h"
#include "audio_process_eq.h"
#include "stdbool.h"
#include "hal_location.h"
#include "hw_codec_iir_process.h"
#include "hw_iir_process.h"
#include "tgt_hardware.h"
#include "drc.h"
#include "limiter.h"
#include "reverb.h"
#include "bass_enhancer.h"
#include "audio_cfg.h"
#include "hal_codec.h"
#include "hw_limiter_cfg.h"
#include "audioflinger.h"
#include "af_stream_sw_gain.h"
#include "arm_math.h"
#include "heap_api.h"

#if defined(ANC_APP)
#include "app_anc.h"
#endif

//#define AUDIO_PROCESS_DUMP
#ifdef AUDIO_PROCESS_DUMP
#include "audio_dump.h"
#endif

#if defined(USB_EQ_TUNING)
#if !defined(__HW_DAC_IIR_EQ_PROCESS__) && !defined(__SW_IIR_EQ_PROCESS__)
#error "Either HW_DAC_IIR_EQ_PROCESS or SW_IIR_EQ_PROCESS should be defined when enabling USB_EQ_TUNING"
#endif
#endif

#ifdef AUDIO_DSP_ACCEL
#include "app_mcpp.h"
#endif

#if defined(AUDIO_EQ_TUNING)
#include "hal_cmd.h"

#ifdef __AUDIO_DRC__
#define AUDIO_DRC_UPDATE_CFG
#endif

#ifdef __AUDIO_LIMITER__
#define AUDIO_LIMITER_UPDATE_CFG
#endif

#ifdef __AUDIO_REVERB__
#define AUDIO_REVERB_UPDATE_CFG
#endif

#endif

#ifdef __AUDIO_DRC__
extern const DrcConfig audio_drc_cfg;
#endif

#ifdef __AUDIO_LIMITER__
extern const LimiterConfig audio_limiter_cfg;
#endif

#ifdef __AUDIO_REVERB__
extern const ReverbConfig audio_reverb_cfg;
#endif

#ifdef __AUDIO_BASS_ENHANCER__
extern const BassEnhancerConfig audio_bass_cfg;
#endif

#if defined(AUDIO_DRC_UPDATE_CFG) || defined(AUDIO_LIMITER_UPDATE_CFG)
#define AUDIO_UPDATE_CFG
#endif

#ifdef __AUDIO_DRC__
#if CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV ==  (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)
#define AUDIO_DRC_NEEDED_SIZE (1024*27)
#else
#define AUDIO_DRC_NEEDED_SIZE (1024*15)
#endif
#else
#define AUDIO_DRC_NEEDED_SIZE (0)
#endif

#ifdef __AUDIO_LIMITER__
#define AUDIO_LIMITER_NEEDED_SIZE (1024*9)
#else
#define AUDIO_LIMITER_NEEDED_SIZE (0)
#endif

#ifdef __AUDIO_REVERB__
#define AUDIO_REVERB_NEEDED_SIZE (1024*9)
#else
#define AUDIO_REVERB_NEEDED_SIZE (0)
#endif

#ifdef __AUDIO_BASS_ENHANCER__
#define AUDIO_BASS_NEEDED_SIZE (1024*1)
#else
#define AUDIO_BASS_NEEDED_SIZE (0)
#endif

#define AUDIO_MEMORY_SIZE (AUDIO_DRC_NEEDED_SIZE + AUDIO_LIMITER_NEEDED_SIZE + AUDIO_REVERB_NEEDED_SIZE + AUDIO_BASS_NEEDED_SIZE)

#define SPEECH_MEMORY_SIZE (AUDIO_ADJ_EQ_NEEDED_SIZE)

#if AUDIO_MEMORY_SIZE > 0
#include "audio_memory.h"
#endif

#if SPEECH_MEMORY_SIZE > 0
#include "speech_memory.h"
#include "audio_memory.h"
#endif

#ifndef CODEC_OUTPUT_DEV
#define CODEC_OUTPUT_DEV                    CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV
#endif

typedef signed int          pcm_24bits_t;
typedef signed short int    pcm_16bits_t;

typedef struct{
    enum AUD_BITS_T         sample_bits;
    enum AUD_SAMPRATE_T     sample_rate;
    enum AUD_CHANNEL_NUM_T  sw_ch_num;
    enum AUD_CHANNEL_NUM_T  hw_ch_num;

#if AUDIO_MEMORY_SIZE > 0
    uint8_t *audio_heap;
#endif

#if SPEECH_MEMORY_SIZE > 0
    uint8_t *speech_heap;
#endif

#ifdef __AUDIO_DRC__
    DrcState *drc_st;
#endif

#ifdef __AUDIO_LIMITER__
    LimiterState *limiter_st;
#endif

#ifdef __AUDIO_REVERB__
    ReverbState *reverb_st;
#endif

#ifdef __AUDIO_BASS_ENHANCER__
    BassEnhancerState *bass_st;
#endif

#ifdef AUDIO_UPDATE_CFG
    bool update_cfg;
#endif

#ifdef USB_EQ_TUNING
	bool eq_updated_cfg;
#endif

#ifdef AUDIO_DRC_UPDATE_CFG
    bool drc_update;
    DrcConfig drc_cfg;
#endif

#ifdef AUDIO_LIMITER_UPDATE_CFG
    bool limiter_update;
    LimiterConfig limiter_cfg;
#endif

#ifdef AUDIO_REVERB_UPDATE_CFG
    bool reverb_update;
    ReverbConfig reverb_cfg;
#endif
} AUDIO_PROCESS_T;

#ifdef AUDIO_PROCESS_IN_CP
CP_DATA_LOC
#endif
static AUDIO_PROCESS_T audio_process = {
    .sample_bits = AUD_BITS_24,
    .sample_rate = AUD_SAMPRATE_44100,
    .sw_ch_num = AUD_CHANNEL_NUM_2,
    .hw_ch_num = AUD_CHANNEL_NUM_2,

#ifdef AUDIO_UPDATE_CFG
    .update_cfg = false,
#endif

#ifdef AUDIO_DRC_UPDATE_CFG
    .drc_update = false,
    .drc_cfg = {
        .knee = 0,
        .filter_type = {-1, -1},
        .band_num = 1,
        .look_ahead_time = 0,
        .band_settings = {
            {0, 0, 1, 1, 1, 1},
            {0, 0, 1, 1, 1, 1},
        }
    },
#endif

#ifdef AUDIO_LIMITER_UPDATE_CFG
    .limiter_update = false,
    .limiter_cfg = {
        .knee = 0,
        .look_ahead_time = 0,
        .threshold = 0,
        .makeup_gain = 0,
        .ratio = 1000,
        .attack_time = 1,
        .release_time = 1,
    },
#endif

#ifdef AUDIO_REVERB_UPDATE_CFG
    .reverb_update = false,
    .reverb_cfg = {
        .bypass = 1,
        .high_pass_f0 = 100,
        .gain = 0,
    },
#endif
};

#ifdef __AUDIO_LIMITER__
static custom_allocator a2dp_allocator = {
    .malloc = audio_malloc,
    .calloc = audio_calloc,
    .free = audio_free,
};
#endif

#if defined(AUDIO_OUTPUT_SW_GAIN) && defined(AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC)
static enum AUD_STREAM_ID_T audio_aud_stream_id = AUD_STREAM_ID_0;
void audio_process_set_aud_stream_id(enum AUD_STREAM_ID_T id)
{
    audio_aud_stream_id = id;
}
#endif

uint32_t audio_process_need_audio_buf_size(void)
{
    return AUDIO_MEMORY_SIZE;
}

#ifdef AUDIO_PROCESS_IN_CP
static CP_BSS_LOC uint32_t cp_heap_buf[AUDIO_MEMORY_SIZE/4];
#endif

#ifdef AUDIO_PROCESS_DUMP
short dump_buf[1024] = {0};
#endif
int SRAM_TEXT_LOC audio_process_run(uint8_t *buf, uint32_t len)
{
    int POSSIBLY_UNUSED pcm_len = 0;

    if(audio_process.sample_bits == AUD_BITS_16)
    {
        pcm_len = len / sizeof(pcm_16bits_t);
    }
    else if(audio_process.sample_bits == AUD_BITS_24)
    {
        pcm_len = len / sizeof(pcm_24bits_t);
    }
    else
    {
        ASSERT(0, "[%s] bits(%d) is invalid", __func__, audio_process.sample_bits);
    }

#ifdef __AUDIO_REVERB__
    reverb_preprocess(audio_process.reverb_st, buf, len);
#endif

#ifdef __AUDIO_BASS_ENHANCER__
    bass_enhancer_process(audio_process.bass_st, buf, len);
#endif

    if (audio_process.sw_ch_num == audio_process.hw_ch_num) {
        // do nothing
    } else if (audio_process.sw_ch_num == AUD_CHANNEL_NUM_1 &&
               audio_process.hw_ch_num == AUD_CHANNEL_NUM_2) {
        if (audio_process.sample_bits == AUD_BITS_16) {
            int16_t *pcm_buf = (int16_t *)buf;
            for (uint32_t i = 0, j = 0; i < pcm_len; i += 2, j++) {
                pcm_buf[j] = pcm_buf[i];
            }
        } else {
            int32_t *pcm_buf = (int32_t *)buf;
            for (uint32_t i = 0, j = 0; i < pcm_len; i += 2, j++) {
                pcm_buf[j] = pcm_buf[i];
            }
        }

        pcm_len /= 2;
        len /= 2;
    } else {
        ASSERT(0, "[%s] sw_ch_num(%d) or hw_ch_num(%d) is invalid", __FUNCTION__, audio_process.sw_ch_num, audio_process.hw_ch_num);
    }

#ifdef AUDIO_DSP_ACCEL
    APP_MCPP_PLAY_PCM_T pcm_cfg;
    memset(&pcm_cfg, 0, sizeof(pcm_cfg));
    pcm_cfg.in = buf;
    pcm_cfg.out = buf;
    pcm_cfg.frame_len = pcm_len;
    app_mcpp_playback_process(APP_MCPP_USER_AUDIO, &pcm_cfg);
#endif

#ifdef AUDIO_PROCESS_DUMP
    int *buf32 = (int *)buf;
    for(int i=0;i<1024;i++)
        dump_buf[i] = buf32[2 * i]>>8;
    audio_dump_clear_up();
    audio_dump_add_channel_data(0, dump_buf, 1024);

#endif

    //int32_t s_time,e_time;
    //s_time = hal_fast_sys_timer_get();

#if defined(AUDIO_OUTPUT_SW_GAIN) && defined(AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC)
    af_stream_sw_gain_process(audio_aud_stream_id, AUD_STREAM_PLAYBACK, buf, len);
#endif

    audio_process_eq_run(buf, len);

#ifdef AUDIO_PROCESS_DUMP
    for(int i=0;i<1024;i++)
        dump_buf[i] = buf32[2 * i+0]>>8;
    audio_dump_add_channel_data(1, dump_buf, 1024);
#endif

#ifdef __AUDIO_DRC__
#ifdef AUDIO_DRC_UPDATE_CFG
	if(audio_process.drc_update)
	{
        drc_set_config(audio_process.drc_st, &audio_process.drc_cfg);
        audio_process.drc_update =false;
	}
#endif

	drc_process(audio_process.drc_st, buf, pcm_len);
#endif

    //int32_t m_time = hal_fast_sys_timer_get();

#ifdef __AUDIO_LIMITER__
#ifdef AUDIO_LIMITER_UPDATE_CFG
    if(audio_process.limiter_update)
    {
        limiter_set_config(audio_process.limiter_st, &audio_process.limiter_cfg);
        audio_process.limiter_update =false;
    }
#endif

    limiter_process(audio_process.limiter_st, buf, pcm_len);
#endif

#ifdef __AUDIO_REVERB__
#ifdef AUDIO_REVERB_UPDATE_CFG
    if(audio_process.reverb_update)
    {
        reverb_set_config(audio_process.reverb_st, &audio_process.reverb_cfg);
        audio_process.reverb_update = false;
    }
#endif

    reverb_process(audio_process.reverb_st, buf, len);
#endif

#if defined(__SW_IIR_EQ_PROCESS__) || defined(__AUDIO_DRC__) || defined(__AUDIO_LIMITER__) || defined(__AUDIO_REVERB__) || defined(__AUDIO_BASS_ENHANCER__)
    if (audio_process.sample_bits == AUD_BITS_24) {
        int32_t *buf32 = (int32_t *)buf;
        for (int32_t i = 0; i < len / sizeof(int32_t); i++) {
            buf32[i] = __SSAT(buf32[i], 24);
        }
    }
#endif

    if (audio_process.sw_ch_num == audio_process.hw_ch_num) {
        // do nothing
    } else if (audio_process.sw_ch_num == AUD_CHANNEL_NUM_1 &&
        audio_process.hw_ch_num == AUD_CHANNEL_NUM_2) {
        if (audio_process.sample_bits == AUD_BITS_16) {
            int16_t *pcm_buf = (int16_t *)buf;
            for (int32_t i = pcm_len - 1, j = 2 * pcm_len - 2; i >= 0; i--, j -= 2) {
                pcm_buf[j + 1] = pcm_buf[i];
                pcm_buf[j + 0] = pcm_buf[i];
            }
        } else {
            int32_t *pcm_buf = (int32_t *)buf;
            for (int32_t i = pcm_len - 1, j = 2 * pcm_len -  2; i >= 0; i--, j -= 2) {
                pcm_buf[j + 1] = pcm_buf[i];
                pcm_buf[j + 0] = pcm_buf[i];
            }
        }

        pcm_len *= 2;
        len *= 2;
    } else {
        ASSERT(0, "[%s] sw_ch_num(%d) or hw_ch_num(%d) is invalid", __FUNCTION__, audio_process.sw_ch_num, audio_process.hw_ch_num);
    }

#ifdef AUDIO_PROCESS_DUMP
        //for(int i=0;i<1024;i++)
        //    dump_buf[i] = buf32[2 * i+0]>>8;
        //audio_dump_add_channel_data(1, dump_buf, 1024);
        audio_dump_run();
#endif

    //e_time = hal_fast_sys_timer_get();
    //TRACE(4,"[%s] Sample len = %d, drc1 %d us, limiter %d us",
    //    __func__, pcm_len, FAST_TICKS_TO_US(m_time - s_time), FAST_TICKS_TO_US(e_time - m_time));

    return 0;
}

/*
 * frame_size stands for samples per channel
 */
int audio_process_open(enum AUD_SAMPRATE_T sample_rate, enum AUD_BITS_T sample_bits, enum AUD_CHANNEL_NUM_T sw_ch_num, enum AUD_CHANNEL_NUM_T hw_ch_num, int32_t frame_size, void *eq_buf, uint32_t len)
{
    TRACE(5,"[%s] sample_rate = %d, sample_bits = %d, sw_ch_num = %d, hw_ch_num = %d", __func__, sample_rate, sample_bits, sw_ch_num, hw_ch_num);
#ifdef AUDIO_PROCESS_DUMP
    audio_dump_init(1024, sizeof(short), 2);
#endif
    audio_process.sample_rate = sample_rate;
    audio_process.sample_bits = sample_bits;
    audio_process.sw_ch_num = sw_ch_num;
    audio_process.hw_ch_num = hw_ch_num;

    audio_process_eq_open(sample_rate, sample_bits, sw_ch_num, hw_ch_num, frame_size, eq_buf, len);

#if AUDIO_MEMORY_SIZE > 0
#ifdef AUDIO_PROCESS_IN_CP
    audio_process.audio_heap = (uint8_t *)cp_heap_buf;
#else
    syspool_get_buff(&audio_process.audio_heap, AUDIO_MEMORY_SIZE);
#endif
    audio_heap_init(audio_process.audio_heap, AUDIO_MEMORY_SIZE);
#endif

#if SPEECH_MEMORY_SIZE > 0
    syspool_get_buff(&audio_process.speech_heap, SPEECH_MEMORY_SIZE);
    speech_heap_init(audio_process.speech_heap, SPEECH_MEMORY_SIZE);
#endif

#ifdef __AUDIO_DRC__
    audio_process.drc_st = drc_create(sample_rate, sample_bits, sw_ch_num,
#ifdef AUDIO_DRC_UPDATE_CFG
        &audio_process.drc_cfg
#else
        &audio_drc_cfg
#endif
        );
#ifdef AUDIO_DRC_UPDATE_CFG
    audio_process.drc_update = false;
#endif
#endif

#ifdef __AUDIO_LIMITER__
    audio_process.limiter_st = limiter_create_with_custom_allocator(sample_rate, sample_bits, sw_ch_num,
#ifdef AUDIO_LIMITER_UPDATE_CFG
        &audio_process.limiter_cfg
#else
        &audio_limiter_cfg
#endif
        , &a2dp_allocator);
#ifdef AUDIO_LIMITER_UPDATE_CFG
    audio_process.limiter_update = false;
#endif
#endif

#ifdef __AUDIO_REVERB__
    audio_process.reverb_st = reverb_create(sample_rate, frame_size, sample_bits, hw_ch_num,
#ifdef AUDIO_REVERB_UPDATE_CFG
        &audio_process.reverb_cfg
#else
        &audio_reverb_cfg
#endif
        );
#ifdef AUDIO_REVERB_UPDATE_CFG
    audio_process.reverb_update = false;
#endif
#endif

#ifdef __AUDIO_BASS_ENHANCER__
    audio_process.bass_st = bass_enhancer_init(sample_rate, sample_bits, hw_ch_num, &audio_bass_cfg);
#endif

#ifdef __AUDIO_HW_LIMITER__
    hwlimiter_open(CODEC_OUTPUT_DEV);
#ifdef __AUDIO_RESAMPLE__
    hwlimiter_set_cfg(AUD_SAMPRATE_50781,0);
#else
    hwlimiter_set_cfg(sample_rate,0);
#endif
    hwlimiter_enable(CODEC_OUTPUT_DEV);
#endif

#if defined(PC_CMD_UART_ID) && defined(USB_AUDIO_APP)
    hal_cmd_open();
#endif

#ifdef AUDIO_DSP_ACCEL
    APP_MCPP_CFG_T dsp_cfg;
    memset(&dsp_cfg, 0, sizeof(APP_MCPP_CFG_T));
    dsp_cfg.capture.stream_enable = false;
    dsp_cfg.playback.stream_enable = true;
    dsp_cfg.playback.sample_rate = sample_rate;
    if (sample_bits == AUD_BITS_24){
        dsp_cfg.playback.sample_bytes = sizeof(int32_t);
    }
    else if (sample_bits == AUD_BITS_16){
        dsp_cfg.playback.sample_bytes = sizeof(int16_t);
    }
    dsp_cfg.playback.algo_frame_len = frame_size;
    dsp_cfg.playback.channel_num = sw_ch_num;
#if defined(AUDIO_DSP_ACCEL_HIFI)
    dsp_cfg.playback.core_server = APP_MCPP_CORE_HIFI;
#elif defined(AUDIO_DSP_ACCEL_M55)
    dsp_cfg.playback.core_server = APP_MCPP_CORE_M55;
#elif defined(AUDIO_DSP_ACCEL_SENS)
    dsp_cfg.playback.core_server = APP_MCPP_CORE_SENS;
#elif defined(AUDIO_DSP_ACCEL_CP_SUBSYS)
    dsp_cfg.playback.core_server = APP_MCPP_CORE_CP_SUBSYS;
#endif

    app_mcpp_open(APP_MCPP_USER_AUDIO, &dsp_cfg);
#endif

#if defined(__AUDIO_ADJ_EQ__)
    int anc_assist_frame, anc_assist_sample_rate;
    get_anc_assist_cfg(&anc_assist_frame, &anc_assist_sample_rate);
    adj_eq_init(anc_assist_frame/4, frame_size, anc_assist_sample_rate/4, sample_rate);
#endif

    audio_peocess_eq_set_eq();

    return 0;
}

int audio_process_close(void)
{
    audio_process_eq_close();

#ifdef __AUDIO_DRC__
#ifdef AUDIO_DRC_UPDATE_CFG
    audio_process.drc_update = false;
#endif
    drc_destroy(audio_process.drc_st);
    audio_process.drc_st = NULL;
#endif

#ifdef __AUDIO_LIMITER__
#ifdef AUDIO_LIMITER_UPDATE_CFG
    audio_process.limiter_update = false;
#endif
    limiter_destroy(audio_process.limiter_st);
    audio_process.limiter_st = NULL;
#endif

#ifdef __AUDIO_REVERB__
#ifdef AUDIO_REVERB_UPDATE_CFG
    audio_process.reverb_update = false;
#endif
    reverb_destroy(audio_process.reverb_st);
    audio_process.reverb_st = NULL;
#endif

#ifdef __AUDIO_BASS_ENHANCER__
    bass_enhancer_destroy(audio_process.bass_st);
#endif

#ifdef AUDIO_DSP_ACCEL
    app_mcpp_close(APP_MCPP_USER_AUDIO);
#endif

#if AUDIO_MEMORY_SIZE > 0
    size_t total = 0, used = 0, max_used = 0;
    audio_memory_info(&total, &used, &max_used);
    TRACE(3,"AUDIO MALLOC MEM: total - %d, used - %d, max_used - %d.", total, used, max_used);
    ASSERT(used == 0, "[%s] used != 0", __func__);
#endif

#if SPEECH_MEMORY_SIZE > 0
    size_t total_speech = 0, used_speech = 0, max_used_speech = 0;
    audio_memory_info(&total_speech, &used_speech, &max_used_speech);
    TRACE(3,"SPEECH MALLOC MEM: total - %d, used - %d, max_used - %d.", total_speech, used_speech, max_used_speech);
    ASSERT(used_speech == 0, "[%s] used != 0", __func__);
#endif


#ifdef __AUDIO_HW_LIMITER__
    hwlimiter_disable(CODEC_OUTPUT_DEV);
    hwlimiter_close();
#endif

#if defined(PC_CMD_UART_ID) && defined(USB_AUDIO_APP)
    hal_cmd_close();
#endif

    return 0;
}

#if defined(AUDIO_EQ_TUNING)
int audio_ping_callback(uint8_t *buf, uint32_t len)
{
    //TRACE(0,"");
    return 0;
}

int audio_anc_switch_callback(uint8_t *buf, uint32_t len)
{
    TRACE(0, "[%s] len = %d, sizeof(uint32_t) = %d", __func__, len, sizeof(uint32_t));

    if (len != sizeof(uint32_t)) {
        return 1;
    }

#if defined(ANC_APP)
    uint32_t mode = *((uint32_t *)buf);

    if (mode < APP_ANC_MODE_QTY) {
        TRACE(0, "[%s] mode: %d", __func__, mode);
        app_anc_switch(mode);
    } else {
        TRACE(0, "[%s] WARNING: mode(%d) >= APP_ANC_MODE_QTY", __func__, mode);
    }
#else
    TRACE(0, "[%s] WARNING: ANC_APP is disabled", __func__);
#endif

    return 0;
}

#ifdef AUDIO_DRC_UPDATE_CFG
int audio_drc_callback(uint8_t *buf, uint32_t  len)
{
	TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(DrcConfig));

    if (len != sizeof(DrcConfig))
    {
        TRACE(1,"[%s] WARNING: Length is different", __func__);

        return 1;
    }

    if (audio_process.drc_st == NULL)
    {
        TRACE(1,"[%s] WARNING: audio_process.limiter_st = NULL", __func__);
        TRACE(1,"[%s] WARNING: Please Play music, then tuning Limiter", __func__);

        return 2;
    }

	memcpy(&audio_process.drc_cfg, buf, sizeof(DrcConfig));
    audio_process.drc_update = true;

    return 0;
}
#endif

#ifdef AUDIO_LIMITER_UPDATE_CFG
int audio_limiter_callback(uint8_t *buf, uint32_t  len)
{
	TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(LimiterConfig));

    if (len != sizeof(LimiterConfig))
    {
        TRACE(1,"[%s] WARNING: Length is different", __func__);

        return 1;
    }

    if (audio_process.limiter_st == NULL)
    {
        TRACE(1,"[%s] WARNING: audio_process.limiter_st = NULL", __func__);
        TRACE(1,"[%s] WARNING: Please Play music, then tuning Limiter", __func__);

        return 2;
    }

	memcpy(&audio_process.limiter_cfg, buf, sizeof(LimiterConfig));
    audio_process.limiter_update = true;

    return 0;
}
#endif

#ifdef AUDIO_REVERB_UPDATE_CFG
int audio_reverb_callback(uint8_t *buf, uint32_t len)
{
    TRACE(3, "[%s] len = %d, sizeof(struct) = %d", __FUNCTION__, len, sizeof(ReverbConfig));

    if (len != sizeof(ReverbConfig))
    {
        TRACE(1, "[%s] WARNING: length is different", __FUNCTION__);
        DUMP8("0x%x, ", buf, len);
        return 1;
    }

    if (audio_process.reverb_st == NULL)
    {
        TRACE(1, "[%s] WARNING: audio_process.reverb_st = NULL", __FUNCTION__);
        TRACE(1, "[%s] WARNING: Please Play music, then tuning Reverb", __FUNCTION__);

        return 2;
    }

	memcpy(&audio_process.reverb_cfg, buf, sizeof(ReverbConfig));
    audio_process.reverb_update = true;

    return 0;
}
#endif
#endif  // #if defined(AUDIO_EQ_TUNING)

typedef struct {
    uint8_t type;
    // EQ
    uint8_t max_eq_band_num;
    uint16_t sample_rate_num;
    uint8_t sample_rate[20];
    // ANC
    uint32_t anc_mode;

    // Add new items in order for compatibility
} query_eq_info_t;

extern int getMaxEqBand(void);
extern int getSampleArray(uint8_t* buf, uint16_t *num);

extern void hal_cmd_set_res_playload(uint8_t* data, int len);
#define CMD_TYPE_QUERY_DUT_EQ_INFO  0x00
int audio_cmd_callback(uint8_t *buf, uint32_t len)
{
    uint8_t type;
    query_eq_info_t  info;

    memset(&info, 0, sizeof(info));

    type = buf[0];

    TRACE(2,"%s type: %d", __func__, type);
    switch (type) {
        case CMD_TYPE_QUERY_DUT_EQ_INFO:
            info.type = CMD_TYPE_QUERY_DUT_EQ_INFO;

            // EQ
            info.max_eq_band_num = getMaxEqBand();
            getSampleArray(info.sample_rate, &info.sample_rate_num);

            // ANC
#if defined(ANC_APP)
            info.anc_mode = app_anc_get_curr_mode();
#else
            info.anc_mode = 0;
#endif

            hal_cmd_set_res_playload((uint8_t*)&info, sizeof(info));
            break;
        default:
            break;
    }

    return 0;
}

#ifdef AUDIO_SECTION_ENABLE
int audio_cfg_burn_callback(uint8_t *buf, uint32_t  len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof_audio_cfg());

    if (len != sizeof_audio_cfg())
    {
        return 1;
    }

    int res = 0;
    res = store_audio_cfg_into_audio_section((AUDIO_CFG_T *)buf);

    if(res)
    {
        TRACE(2,"[%s] ERROR: res = %d", __func__, res);
        res += 100;
    }
    else
    {
        TRACE(1,"[%s] Store audio cfg into audio section!!!", __func__);
    }

    return res;
}
#endif

int audio_process_init(void)
{
#if defined(AUDIO_EQ_TUNING)
#if defined(USB_EQ_TUNING) || defined(PC_CMD_UART_ID)
    hal_cmd_init();
#endif

#if !defined(USB_EQ_TUNING)
#ifdef AUDIO_DRC_UPDATE_CFG
    hal_cmd_register("drc", audio_drc_callback);
#endif

#ifdef AUDIO_LIMITER_UPDATE_CFG
    hal_cmd_register("limiter", audio_limiter_callback);
#endif

#ifdef AUDIO_REVERB_UPDATE_CFG
    hal_cmd_register("reverb", audio_reverb_callback);
#endif

#ifdef AUDIO_SECTION_ENABLE
    hal_cmd_register("burn", audio_cfg_burn_callback);
    hal_cmd_register("audio_burn", audio_cfg_burn_callback);
#endif
#endif  // #if defined(USB_EQ_TUNING)

    hal_cmd_register("anc_switch", audio_anc_switch_callback);
    hal_cmd_register("cmd", audio_cmd_callback);
    hal_cmd_register("ping", audio_ping_callback);
#endif  // #if defined(AUDIO_EQ_TUNING)

    // load default cfg
#ifdef AUDIO_DRC_UPDATE_CFG
    memcpy(&audio_process.drc_cfg, &audio_drc_cfg, sizeof(DrcConfig));
#endif

#ifdef AUDIO_LIMITER_UPDATE_CFG
    memcpy(&audio_process.limiter_cfg, &audio_limiter_cfg, sizeof(LimiterConfig));
#endif

#ifdef AUDIO_REVERB_UPDATE_CFG
    memcpy(&audio_process.reverb_cfg, &audio_reverb_cfg, sizeof(ReverbConfig));
#endif

    audio_process_eq_init();

    return 0;
}
