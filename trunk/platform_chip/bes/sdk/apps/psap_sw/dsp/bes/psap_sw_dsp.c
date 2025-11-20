/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "string.h"
#include "cmsis.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_cache.h"
#include "dma_audio_stream.h"
#include "dma_audio_stream_conf.h"
#include "dma_audio_algo.h"
#include "dma_audio.h"
#include "psap_freq_psap.h"
#include "psap_sw_app_ctrl.h"
#include "psap_sw_comm.h"

#define PSAP_SW_MAX_SUPPORT_CAP_CHANS    (4)
#define PSAP_SW_MAX_SUPPORT_PLAY_CHANS   (2)

#define CHAR_BYTES          (1)
#define SHORT_BYTES         (2)
#define INT_BYTES           (4)

#if DMA_AUD_SAMP_SIZE == SHORT_BYTES
typedef short   VOICE_PCM_T;
#elif DMA_AUD_SAMP_SIZE == INT_BYTES
typedef int     VOICE_PCM_T;
#else
#error "Invalid DMA_AUD_SAMP_SIZE!!!"
#endif

static VOICE_PCM_T *g_pcm_mic[PSAP_SW_MAX_SUPPORT_CAP_CHANS] = {NULL};
static VOICE_PCM_T *g_pcm_spk[PSAP_SW_MAX_SUPPORT_PLAY_CHANS] = {NULL};
static uint32_t g_pcm_len_per_frame = 0;
static struct DAUD_STREAM_CONFIG_T g_play_stream_cfg;
static struct DAUD_STREAM_CONFIG_T g_cap_stream_cfg;

static PsapFreqPsapState      *freq_psap_st = NULL;
extern PsapFreqPsapConfig     psap_freq_psap_cfg;

extern int32_t psap_freq_set_total_gain(float *gain_l);
extern int32_t psap_wdrc_set_dynamic_gain(float *gain, uint32_t band_num);
extern int32_t psap_bf_control(bool enable);
extern int32_t psap_ns_set_denoise_level(int32_t level);

static void psap_sw_info_init(bool init)
{
    if (init) {
        dma_audio_stream_get_config(AUD_STREAM_PLAYBACK, &g_play_stream_cfg);
        dma_audio_stream_get_config(AUD_STREAM_CAPTURE, &g_cap_stream_cfg);

        if (PSAP_SW_MAX_SUPPORT_CAP_CHANS < g_cap_stream_cfg.channel_num
            || PSAP_SW_MAX_SUPPORT_PLAY_CHANS < g_play_stream_cfg.channel_num) {
            ASSERT(0, "error chans:cap:%d paly:%d", g_cap_stream_cfg.channel_num, g_play_stream_cfg.channel_num);
        }

        g_pcm_len_per_frame = g_play_stream_cfg.data_size / 2 / g_play_stream_cfg.channel_num / sizeof(VOICE_PCM_T);
        if (NULL == g_pcm_mic[0]) {
            uint32_t psap_sw_mallco_size = (PSAP_SW_MAX_SUPPORT_CAP_CHANS + PSAP_SW_MAX_SUPPORT_PLAY_CHANS) * g_pcm_len_per_frame * sizeof(VOICE_PCM_T);
            g_pcm_mic[0] = (VOICE_PCM_T *)daud_heap_malloc(psap_sw_mallco_size);

            for (int i = 1; i < PSAP_SW_MAX_SUPPORT_CAP_CHANS; i++) {
                g_pcm_mic[i] = g_pcm_mic[i - 1] + g_pcm_len_per_frame;
            }
            for (int i = 0; i < PSAP_SW_MAX_SUPPORT_PLAY_CHANS; i++) {
                g_pcm_spk[i] = g_pcm_mic[0] + (PSAP_SW_MAX_SUPPORT_CAP_CHANS + i) * g_pcm_len_per_frame;
            }
        }
    } else {
        if (NULL != g_pcm_mic[0]) {
            daud_heap_free(g_pcm_mic[0]);

            for (int i = 0; i < PSAP_SW_MAX_SUPPORT_CAP_CHANS; i++) {
                g_pcm_mic[i] = NULL;
            }
            for (int i = 0; i < PSAP_SW_MAX_SUPPORT_PLAY_CHANS; i++) {
                g_pcm_spk[i] = NULL;
            }
        }
    }
}

static int32_t psap_sw_cmd_handler(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, void *cfg, uint32_t len, bool set)
{
    ASSERT(len == sizeof(psap_sw_cmd_t), "[%s] bes input len(%d) != cfg len", __func__, len);

    psap_sw_cmd_t cmd;

    memcpy(&cmd, (psap_sw_cmd_t *)cfg, sizeof(psap_sw_cmd_t));

    TRACE(2, "[%s] mode: %d", __func__, cmd.mode);

    if (cmd.mode == PSAP_SW_CMD_TOTAL_GAIN) {
        TRACE(5, "[%s] Total Gain(x100): [L] %d, [R] %d", __func__,
                (int32_t)(cmd.total_gain[0] *100), (int32_t)(cmd.total_gain[1] *100));
        psap_freq_set_total_gain(cmd.total_gain);
    } else if (cmd.mode == PSAP_SW_CMD_MODE_WDRC) {
        TRACE(5, "[%s] Gain(x100): [0] %d, [1] %d, [-2] %d, [-1] %d", __func__,
                (int32_t)(cmd.band_gain[0] * 100),
                (int32_t)(cmd.band_gain[1] * 100),
                (int32_t)(cmd.band_gain[PSAP_BAND_NUM-2] * 100),
                (int32_t)(cmd.band_gain[PSAP_BAND_NUM-1] * 100));
        psap_wdrc_set_dynamic_gain(cmd.band_gain, PSAP_BAND_NUM);
    } else if (cmd.mode == PSAP_SW_CMD_MODE_BF) {
        TRACE(2, "[%s] bf_enable: %d", __func__, cmd.bf_enable);
        psap_bf_control(cmd.bf_enable);
    } else if (cmd.mode == PSAP_SW_CMD_MODE_NS) {
        TRACE(2, "[%s] ns_level: %d", __func__, cmd.ns_level);
        psap_ns_set_denoise_level(cmd.ns_level);
    } else if (cmd.mode == PSAP_SW_CMD_MODE_EQ) {
        TRACE(3, "[%s] eq config: %p/%p", __func__, cmd.eq_config[0], cmd.eq_config[1]);
        psap_freq_psap_set_eq_config(freq_psap_st, cmd.eq_config[0], AUD_CHANNEL_MAP_CH0);
        psap_freq_psap_set_eq_config(freq_psap_st, cmd.eq_config[1], AUD_CHANNEL_MAP_CH1);
    } else if (cmd.mode == PSAP_SW_CMD_MODE_DEHOWLING) {
        TRACE(3, "[%s] dehowling config: %d", __func__, cmd.dehowling_config.onoff);
        psap_freq_set_dehowling_config(freq_psap_st, &cmd.dehowling_config);
    } else if (cmd.mode == PSAP_SW_CMD_MODE_REVERB) {
        TRACE(3, "[%s] reverb config", __func__);
        psap_freq_set_reverb_config(freq_psap_st, cmd.reverb_cfg);
    } else if (cmd.mode == PSAP_SW_CMD_MODE_ALL) {
        TRACE(5, "[%s] ALL Gain(x100): [0] %d, [1] %d, [-2] %d, [-1] %d", __func__,
                (int32_t)(cmd.band_gain[0] * 100),
                (int32_t)(cmd.band_gain[1] * 100),
                (int32_t)(cmd.band_gain[PSAP_BAND_NUM-2] * 100),
                (int32_t)(cmd.band_gain[PSAP_BAND_NUM-1] * 100));
        TRACE(2, "[%s] ALL bf_enable: %d", __func__, cmd.bf_enable);
        TRACE(2, "[%s] ALL ns_level: %d", __func__, cmd.ns_level);
        psap_wdrc_set_dynamic_gain(cmd.band_gain, PSAP_BAND_NUM);
        psap_bf_control(cmd.bf_enable);
        psap_ns_set_denoise_level(cmd.ns_level);
    } else {
        TRACE(2, "[%s] WARNING: mode(%d) is invalid", __func__, cmd.mode);
    }

    return 0;
}

static uint32_t psap_sw_handler(uint8_t *dst, uint32_t dst_len, uint8_t *buf, uint32_t len)
{
    VOICE_PCM_T *pcm_buf = (VOICE_PCM_T *)buf;
    VOICE_PCM_T *dst_buf = (VOICE_PCM_T *)dst;
    VOICE_PCM_T *pcm_mic1 = g_pcm_mic[0];
    VOICE_PCM_T *pcm_mic2 = g_pcm_mic[1];
    VOICE_PCM_T *pcm_mic3 = g_pcm_mic[2];
    VOICE_PCM_T *pcm_mic4 = g_pcm_mic[3];
    VOICE_PCM_T *pcm_spk1 = g_pcm_spk[0];
    VOICE_PCM_T *pcm_spk2 = g_pcm_spk[1];

    for(int i = 0; i < g_pcm_len_per_frame; i++) {
        pcm_mic1[i] = pcm_buf[0 + g_cap_stream_cfg.channel_num * i];    // ch3 ff_l
        pcm_mic2[i] = pcm_buf[0 + g_cap_stream_cfg.channel_num * i];    // ch1 ff_r
        pcm_mic3[i] = pcm_buf[1 + g_cap_stream_cfg.channel_num * i];    // ch2 talk_l
        pcm_mic4[i] = pcm_buf[1 + g_cap_stream_cfg.channel_num * i];    // ch0 talk_r
    }

    // uint32_t start_time = FAST_TICKS_TO_US(hal_fast_sys_timer_get());
    psap_freq_psap_process(freq_psap_st, (int32_t *)pcm_mic1, (int32_t *)pcm_mic2, (int32_t *)pcm_mic3, (int32_t *)pcm_mic4, (int32_t *)pcm_spk1, (int32_t *)pcm_spk2, &psap_freq_psap_cfg);
    // TRACE(2,"psap time = %d us",FAST_TICKS_TO_US(hal_fast_sys_timer_get())-start_time);

    if (g_play_stream_cfg.channel_num == 1) {
        for(int i = 0; i < g_pcm_len_per_frame; i++) {
            dst_buf[0 + g_play_stream_cfg.channel_num * i] = pcm_spk1[i];
        }
    } else if (g_play_stream_cfg.channel_num == 2) {
        for(int i = 0; i < g_pcm_len_per_frame; i++) {
            dst_buf[0 + g_play_stream_cfg.channel_num * i] = pcm_spk1[i];
            dst_buf[1 + g_play_stream_cfg.channel_num * i] = pcm_spk2[i];
        }
    }

    return 0;
}

static void psap_sw_init(bool init)
{
    if (true == init) {
        psap_sw_info_init(true);
        freq_psap_st = psap_freq_psap_create(g_play_stream_cfg.sample_rate, g_play_stream_cfg.bits, g_pcm_len_per_frame, &psap_freq_psap_cfg);
    } else {
        psap_sw_info_init(false);
        psap_freq_psap_destroy(freq_psap_st);
    }
}

int32_t psap_sw_dsp_init(struct DAUD_STREAM_CONFIG_T *cap_cfg, struct DAUD_STREAM_CONFIG_T *play_cfg)
{
    psap_sw_init(true);

    return 0;
}

int32_t psap_sw_dsp_deinit(void)
{
    psap_sw_init(false);

    return 0;
}

int32_t psap_sw_dsp_process(uint8_t *play_buf, uint32_t play_len, uint8_t *cap_buf, uint32_t cap_len)
{
    psap_sw_handler(play_buf, play_len, cap_buf, cap_len);

    return 0;
}

int32_t psap_sw_dsp_register_cb(bool en)
{
    if (en) {
        dma_audio_algo_setup_init_callback(psap_sw_init);
        dma_audio_algo_setup_config_callback(psap_sw_cmd_handler);
        dma_audio_algo_setup_algo_proc_callback(psap_sw_dsp_process);
    } else {
        dma_audio_algo_setup_init_callback(NULL);
        dma_audio_algo_setup_config_callback(NULL);
        dma_audio_algo_setup_algo_proc_callback(NULL);
    }

    return 0;
}