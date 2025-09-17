/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#if defined(DAC_TEST) || defined(DAC1_TEST) || defined(DAC2_TEST) || defined(DAC3_TEST)
#include "hwtest.h"
#include "audioflinger.h"
#include "hal_cmu.h"
#include "hal_sysfreq.h"
#include "string.h"
#ifndef NOSTD
#include <math.h>
#endif

#ifdef DAC1_TEST
#ifndef NO_LOOP_STREAM
#define NO_LOOP_STREAM
#endif
#endif

#define AF_STREAM_ID_PLAY1 AUD_STREAM_ID_0
#define AF_STREAM_ID_PLAY2 AUD_STREAM_ID_1
#define AF_STREAM_ID_PLAY3 AUD_STREAM_ID_2

#define ADC1_DEBUG 0
#define ADC2_DEBUG 0
#define DAC1_DEBUG 0
#define DAC2_DEBUG 0
#define DAC3_DEBUG 0

#ifndef ADC1_SAMP_RATE
#define ADC1_SAMP_RATE 48000
#endif

#define ADC1_SAMP_SIZE 2 //16 bits
#define ADC2_SAMP_RATE (ADC1_SAMP_RATE)
#define ADC2_SAMP_SIZE (ADC1_SAMP_SIZE)

#ifndef DAC1_SAMP_RATE
#define DAC1_SAMP_RATE 48000
#endif

#define DAC1_SAMP_SIZE 2 //16 bits

#define DAC2_SAMP_RATE (DAC1_SAMP_RATE)
#define DAC2_SAMP_SIZE (DAC1_SAMP_SIZE)

#define DAC3_SAMP_RATE (DAC1_SAMP_RATE)
#define DAC3_SAMP_SIZE (DAC1_SAMP_SIZE)

#define CAPTURE_CHAN_NUM   1
#define CAPTURE2_CHAN_NUM  1
#define PLAYBACK1_CHAN_NUM 2
#define PLAYBACK2_CHAN_NUM 2
#define PLAYBACK3_CHAN_NUM 2

#define CAPTURE_FRM_NUM   16
#define CAPTURE2_FRM_NUM  16
#define PLAYBACK1_FRM_NUM 4
#define PLAYBACK2_FRM_NUM 8
#define PLAYBACK3_FRM_NUM 8

#define SR2CNT(sr) (sr/1000)

#define CAPTURE_BUF_SIZE   (SR2CNT(ADC1_SAMP_RATE)*ADC1_SAMP_SIZE*CAPTURE_CHAN_NUM*CAPTURE_FRM_NUM)
#define CAPTURE2_BUF_SIZE  (SR2CNT(ADC2_SAMP_RATE)*ADC2_SAMP_SIZE*CAPTURE2_CHAN_NUM*CAPTURE2_FRM_NUM)
#define PLAYBACK1_BUF_SIZE (SR2CNT(DAC1_SAMP_RATE)*DAC1_SAMP_SIZE*PLAYBACK1_CHAN_NUM*PLAYBACK1_FRM_NUM)
#define PLAYBACK2_BUF_SIZE (SR2CNT(DAC2_SAMP_RATE)*DAC2_SAMP_SIZE*PLAYBACK2_CHAN_NUM*PLAYBACK2_FRM_NUM)
#define PLAYBACK3_BUF_SIZE (SR2CNT(DAC3_SAMP_RATE)*DAC3_SAMP_SIZE*PLAYBACK3_CHAN_NUM*PLAYBACK3_FRM_NUM)

#define TIME_PERIOD_DAC_ON  5000
#define TIME_PERIOD_DAC_OFF 500
#define TEST_MAX_COUNT 3
#define TIME_PERIOD_DAC_COMB_ON     3000
#define TIME_PERIOD_DAC_COMB_OFF    3000

static bool adda_opened = false;
static bool adda_started = false;
static uint32_t adda_samp_rate = DAC1_SAMP_RATE;
static uint32_t adda_samp_size = DAC1_SAMP_SIZE;

static bool adc1_started = false;
static uint32_t adc1_samp_rate = ADC1_SAMP_RATE;
static uint32_t adc1_samp_size = ADC1_SAMP_SIZE;

static bool adc2_started = false;
static uint32_t adc2_samp_rate = ADC2_SAMP_RATE;
static uint32_t adc2_samp_size = ADC2_SAMP_SIZE;

static bool dac1_started = false;
static uint32_t dac1_samp_rate = DAC1_SAMP_RATE;
static uint32_t dac1_samp_size = DAC1_SAMP_SIZE;

static bool dac2_started = false;
static uint32_t dac2_samp_rate = DAC2_SAMP_RATE;
static uint32_t dac2_samp_size = DAC2_SAMP_SIZE;

static bool dac3_started = false;
static uint32_t dac3_samp_rate = DAC3_SAMP_RATE;
static uint32_t dac3_samp_size = DAC3_SAMP_SIZE;

#ifndef ALIGN4
#define ALIGN4 __attribute__((aligned(4)))
#endif

static ALIGN4 uint8_t capture_buff[CAPTURE_BUF_SIZE];
static ALIGN4 uint8_t capture2_buff[CAPTURE2_BUF_SIZE];
static ALIGN4 uint8_t playback1_buff[PLAYBACK1_BUF_SIZE];
static ALIGN4 uint8_t playback2_buff[PLAYBACK2_BUF_SIZE];
static ALIGN4 uint8_t playback3_buff[PLAYBACK3_BUF_SIZE];

static uint32_t capture_buff_size = CAPTURE_BUF_SIZE;
static uint32_t wpos = 0;
static uint32_t rpos = 0;

static uint32_t capture2_buff_size = CAPTURE2_BUF_SIZE;
static uint32_t wpos2 = 0;
static uint32_t rpos2 = 0;

static bool en_adc2 = false;

enum AUDIO_INPUT_ITF_T {
    AUDIO_INPUT_ITF_ADC1,
    AUDIO_INPUT_ITF_ADC2,

    AUDIO_INPUT_ITF_QTY,
};

enum AUDIO_OUTPUT_ITF_T {
    AUDIO_OUTPUT_ITF_DAC1,
    AUDIO_OUTPUT_ITF_DAC2,
    AUDIO_OUTPUT_ITF_DAC3,

    AUDIO_OUTPUT_ITF_QTY,
};

enum AUDIO_TEST_RET_T {
    AUDIO_TEST_SUCCESS,
    AUDIO_TEST_ERROR,
    AUDIO_TEST_EXIT,
};

enum AUDIO_DATA_SOURCE_T {
    AUDIO_ZERO_DATA,
    AUDIO_MIC_DATA,
    AUDIO_SIN_1K_DATA,
    AUDIO_SIN_2K_DATA,
    AUDIO_SIN_4K_DATA,
};

typedef uint32_t (*AUDIO_OUTPUT_DATA_HANDLER_T)(uint8_t *buf, uint32_t len);

static enum AUDIO_INPUT_ITF_T  g_aud_in_itf         = AUDIO_INPUT_ITF_QTY;
static enum AUDIO_OUTPUT_ITF_T g_aud_out_itf        = AUDIO_OUTPUT_ITF_QTY;
static enum AUDIO_INPUT_ITF_T  g_loop_in_itf        = AUDIO_INPUT_ITF_QTY;
static enum AUDIO_OUTPUT_ITF_T g_loop_out_itf       = AUDIO_OUTPUT_ITF_QTY;
static bool g_loop_audio_en = false;
static bool g_dma_audio_en = false;
static bool g_dac_audio_en = false;
static bool g_loop_always_on = false;
static bool g_dac_always_on = false;
static bool g_dma_aud_always_on = false;
static bool g_dma_aud_opened = false;
static bool g_dma_aud_started = false;
static enum AUDIO_INPUT_ITF_T  g_dma_aud_in_itf     = AUDIO_INPUT_ITF_QTY;
static enum AUDIO_OUTPUT_ITF_T g_dma_aud_out_itf    = AUDIO_OUTPUT_ITF_QTY;
static uint32_t time_on;
static uint32_t time_off;
static bool g_dac_combined_test = false;
static AUDIO_OUTPUT_DATA_HANDLER_T play_data_handler[AUDIO_OUTPUT_ITF_QTY];

static uint8_t aud_output_data_srcs[AUDIO_OUTPUT_ITF_QTY] = {
#ifdef NO_LOOP_STREAM
    AUDIO_SIN_1K_DATA, //dac1 data
#else
    AUDIO_MIC_DATA,    //dac1 data
#endif
    AUDIO_SIN_1K_DATA, //dac2 data
    AUDIO_SIN_2K_DATA, //dac3 data
};

static int codec_loop_stream_open(enum AUDIO_INPUT_ITF_T in_itf, enum AUDIO_OUTPUT_ITF_T out_itf);
static int codec_loop_stream_close(enum AUDIO_INPUT_ITF_T in, enum AUDIO_OUTPUT_ITF_T out);
static int codec_loop_stream_start(enum AUDIO_INPUT_ITF_T in, enum AUDIO_OUTPUT_ITF_T out);
static int codec_loop_stream_stop(enum AUDIO_INPUT_ITF_T in, enum AUDIO_OUTPUT_ITF_T out);

/*
 * SIN WAVE GENERATOR
 *****************************************************************
 */
#ifndef PI
#define PI   (3.141592654)
#endif

#define FIVAL(x) ((int)x)
#define FPVAL(x) ((int)((x)*1000)-(int)((int)(x)*1000))

#define AUD_WAVE_BUF_SIZE (4*1024)
static float aud_wave_buf[AUD_WAVE_BUF_SIZE];

typedef struct {
    float *buf;
    uint32_t buf_size;
    uint32_t freq;
    uint32_t samp_rate;
    uint32_t samp_size;
    uint32_t samp_chan;
    uint32_t samp_count;
    uint32_t rpos;
    float amp;
    float swgain;
} aud_wave_t;

static aud_wave_t aud_wave1;
static aud_wave_t aud_wave2;

static int audio_wave_gen_sin(float *wave_array, uint32_t wave_size, float f, float fs, float a)
{
    float T = 1 / f;
    float Ts = 1 / fs, Td;
    float samp_cnt = fs / f;
    float y = 0, x = 0;
    int i, sig_len = 0, n;

    if (fs <= f * 2) {
        TRACE(3,"SIG_GEN: invalid param: fs(%d) <= f(%d) * 2\n", (int)fs, (int)f);
        return -1;
    }
    samp_cnt = fs * 10.0 / f;
    Td = 2*PI*10.0/samp_cnt;
    n = (int)samp_cnt;
    if (n > wave_size) {
        n = wave_size;
    }

    TRACE(3,"SIG_GEN: freq=%d.%d Hz, sample freq=%d.%d Hz, wave_size=%d\n",
        FIVAL(f),FPVAL(f),FIVAL(fs),FPVAL(fs),wave_size);
    TRACE(3,"SIG_GEN: T=%d.%d, Ts=%d.%d",FIVAL(T),FPVAL(T),FIVAL(Ts),FPVAL(Ts));
    TRACE(3,"SIG_GEN: samp_cnt=%d.%d, Td=%d.%d, a=%d.%d\n",
        FIVAL(samp_cnt),FPVAL(samp_cnt), FIVAL(Td), FPVAL(Td), FIVAL(a),FPVAL(a));

    for (i = 0; i < n; i++) {
#ifdef NOSTD
        y = 0.0f;
#else
        y = (float)sin(x)*a;
#endif
//        TRACE(3,"[%4d]: sin(%d.%d)=%d.%d\n",i,FIVAL(x),FPVAL(x),FIVAL(y),FPVAL(y));
        if (wave_array) {
            wave_array[i] = y;
        }
        x += Td;
        sig_len++;
    }
    TRACE(3,"SIG_GEN: sig_len=%d, done\n", sig_len);
    return sig_len;
}

static int init_wave_data(aud_wave_t *wave, uint32_t f, uint32_t fs, uint32_t bits, uint32_t chan_num)
{
    int rlen;

    TRACE(3,"%s: f=%d Hz, fs=%d Hz, bits=%d, chan_num=%d",
        __func__, f, fs, bits, chan_num);

    wave->buf = aud_wave_buf;
    wave->buf_size = sizeof(aud_wave_buf);
    wave->freq = f;
    wave->samp_rate = fs;
    wave->samp_size = (bits == 24 ? 4 : (bits / 8));
    wave->samp_chan = chan_num;
    wave->rpos = 0;
    wave->amp = 1000;
    wave->swgain = 1.0;
    rlen = audio_wave_gen_sin(wave->buf, ARRAY_SIZE(aud_wave_buf), (float)f, (float)fs, wave->amp);
    ASSERT(rlen > 0, "%s: invalid rlen=%d", __func__, rlen);
    wave->samp_count = rlen;
    TRACE(3,"%s: wave samp count=%d", __func__, wave->samp_count);
    return 0;
}

static uint32_t fill_wave_data(aud_wave_t *wave, uint8_t *buf, uint32_t len)
{
    float *src_start = &(wave->buf[0]);
    float *src       = &(wave->buf[wave->rpos]);
    float *src_end   = &(wave->buf[wave->samp_count]);
    uint32_t i;

    if (wave->samp_size == 2) {
        int16_t *dst     = (int16_t *)buf;
        int16_t *dst_end = (int16_t *)(buf+len);
        int32_t samp_val;

        while (dst < dst_end) {
            if (src == src_end) {
                src = src_start;
            }
            samp_val = (int32_t)(wave->swgain*(*src++));
            for (i = 0; i < wave->samp_chan; i++) {
                *dst++ = (int16_t)samp_val;
            }
        }
        wave->rpos += (len / wave->samp_chan / sizeof(int16_t));
        if (wave->rpos >= wave->samp_count) {
            wave->rpos -= wave->samp_count;
        }
    } else {
        ASSERT(false, "%s: invalid wave samp_size=%d", __func__, wave->samp_size);
    }
    return 0;
}

/*
 * AUDIO STREAM PLAYER
 *****************************************************************
 */

static enum AUD_BITS_T sample_size_to_bits(uint32_t size)
{
    if (size == 2) {
        return AUD_BITS_16;
    } else if (size == 4) {
        return AUD_BITS_24;
    } else {
        ASSERT(false, "%s: Invalid sample size: %u", __FUNCTION__, size);
    }
    return 0;
}

static uint32_t codec_capture1_data_handler(uint8_t *buf, uint32_t len)
{
#if defined(ADC1_DEBUG) && (ADC1_DEBUG > 0)
    uint32_t time = TICKS_TO_MS(hal_sys_timer_get());
    uint32_t cap_pos = af_stream_get_cur_dma_pos(AF_STREAM_ID_PLAY1, AUD_STREAM_CAPTURE);
    uint32_t ply_pos = af_stream_get_cur_dma_pos(AF_STREAM_ID_PLAY1, AUD_STREAM_PLAYBACK);
#endif
    wpos += len;
    if (wpos >= capture_buff_size) {
        wpos -= capture_buff_size;
    }
#if defined(ADC1_DEBUG) && (ADC1_DEBUG > 0)
    TRACE(3, "[%u] CAP: buf=%x, len=%d, wpos=%d, cap_pos=%d, ply_pos=%d",
        time, (int)buf, len, wpos, cap_pos, ply_pos);
#endif
    return 0;
}

static uint32_t codec_capture2_data_handler(uint8_t *buf, uint32_t len)
{
#if defined(ADC2_DEBUG) && (ADC2_DEBUG > 0)
    uint32_t time = TICKS_TO_MS(hal_sys_timer_get());
    uint32_t cap_pos = af_stream_get_cur_dma_pos(AF_STREAM_ID_PLAY2, AUD_STREAM_CAPTURE);
    uint32_t ply_pos = af_stream_get_cur_dma_pos(AF_STREAM_ID_PLAY2, AUD_STREAM_PLAYBACK);
#endif
    wpos2 += len;
    if (wpos2 >= capture2_buff_size) {
        wpos2 -= capture2_buff_size;
    }
#if defined(ADC2_DEBUG) && (ADC2_DEBUG > 0)
    TRACE(3, "[%u] %s: CAP: buf=%x, len=%d, wpos=%d, cap_pos=%d, ply_pos=%d",
        time, __func__, (int)buf, len, wpos2, cap_pos, ply_pos);
#endif
    return 0;
}

static uint32_t play_zero_data(uint8_t *buf, uint32_t len)
{
    memset(buf, 0, len);
    return 0;
}

static uint32_t play_sin1_data(uint8_t *buf, uint32_t len)
{
    fill_wave_data(&aud_wave1, buf, len);
    return 0;
}

static uint32_t play_sin2_data(uint8_t *buf, uint32_t len)
{
    fill_wave_data(&aud_wave2, buf, len);
    return 0;
}

static uint32_t play_chan_num;
static uint32_t play_samp_bits;
static uint32_t cap_chan_num;
static uint32_t cap_samp_bits;
static uint32_t play2_chan_num;
static uint32_t play2_samp_bits;
static uint32_t cap2_chan_num;
static uint32_t cap2_samp_bits;

static uint32_t play_mic_data(uint8_t *buf, uint32_t len)
{
    uint32_t avail;

    if (wpos >= rpos) {
        avail = wpos - rpos;
    } else {
        avail = capture_buff_size - rpos + wpos;
    }
    // TRACE(1, "play_n=[%d], cap_n=[%d], a=[%d], l=[%d], b=%d, r=%d", play_chan_num, cap_chan_num, avail, len, play_samp_bits, rpos);
    if ((avail * play_chan_num / cap_chan_num) >= len) {
        // memcpy
        if (play_samp_bits == 16) {
            int16_t *src     = (int16_t *)(capture_buff + rpos);
            int16_t *src_end = (int16_t *)(capture_buff + capture_buff_size);
            int16_t *dst     = (int16_t *)(buf);
            int16_t *dst_end = (int16_t *)(buf + len);

            while (dst < dst_end) {
                *dst++ = *src++;
                if (src == src_end) {
                    src = (int16_t *)capture_buff;
                }
                if ((play_chan_num == 2) && (cap_chan_num == 1)) {
                    *dst = *(dst - 1);
                    dst++;
                }
            }
        } else {
            int32_t *src     = (int32_t *)(capture_buff + rpos);
            int32_t *src_end = (int32_t *)(capture_buff + capture_buff_size);
            int32_t *dst     = (int32_t *)(buf);
            int32_t *dst_end = (int32_t *)(buf + len);

            while (dst < dst_end) {
                *dst++ = *src++;
                if (src == src_end) {
                    src = (int32_t *)capture_buff;
                }
                if ((play_chan_num == 2) && (cap_chan_num == 1)) {
                    *dst = *(dst - 1);
                    dst++;
                }
            }
        }
        // update rpos
        rpos += len / PLAYBACK1_CHAN_NUM * CAPTURE_CHAN_NUM;
        if (rpos >= capture_buff_size) {
            rpos -= capture_buff_size;
        }
    } else {
        memset(buf, 0, len);
        TRACE(3, "%s:PLAY WARNING: zero data", __func__);
    }
    return 0;
}

static uint32_t play_mic_data2(uint8_t *buf, uint32_t len)
{
    uint32_t avail;

    if (wpos2 >= rpos2) {
        avail = wpos2 - rpos2;
    } else {
        avail = capture2_buff_size - rpos2 + wpos2;
    }
    // TRACE(1, "play2_n=[%d], cap_n=[%d], a=[%d], l=[%d], b=%d, r=%d", play2_chan_num, cap2_chan_num, avail, len, play2_samp_bits, rpos2);
    if ((avail * play2_chan_num / cap2_chan_num) >= len) {
        // memcpy
        if (play2_samp_bits == 16) {
            int16_t *src     = (int16_t *)(capture2_buff + rpos2);
            int16_t *src_end = (int16_t *)(capture2_buff + capture2_buff_size);
            int16_t *dst     = (int16_t *)(buf);
            int16_t *dst_end = (int16_t *)(buf + len);

            while (dst < dst_end) {
                *dst++ = *src++;
                if (src == src_end) {
                    src = (int16_t *)capture2_buff;
                }
                if ((play2_chan_num == 2) && (cap2_chan_num == 1)) {
                    *dst = *(dst - 1);
                    dst++;
                }
            }
        } else {
            int32_t *src     = (int32_t *)(capture2_buff + rpos2);
            int32_t *src_end = (int32_t *)(capture2_buff + capture2_buff_size);
            int32_t *dst     = (int32_t *)(buf);
            int32_t *dst_end = (int32_t *)(buf + len);

            while (dst < dst_end) {
                *dst++ = *src++;
                if (src == src_end) {
                    src = (int32_t *)capture2_buff;
                }
                if ((play2_chan_num == 2) && (cap2_chan_num == 1)) {
                    *dst = *(dst - 1);
                    dst++;
                }
            }
        }
        // update rpos
        rpos2 += len / play2_chan_num * cap2_chan_num;
        if (rpos2 >= capture2_buff_size) {
            rpos2 -= capture2_buff_size;
        }
    } else {
        memset(buf, 0, len);
        TRACE(3, "%s, PLAY WARNING: zero data", __func__);
    }
    return 0;
}

static uint32_t codec_playback1_data_handler(uint8_t *buf, uint32_t len)
{
#if defined(DAC1_DEBUG) && (DAC1_DEBUG > 0)
    uint32_t dma_pos = af_stream_get_cur_dma_pos(AF_STREAM_ID_PLAY1, AUD_STREAM_PLAYBACK);
    uint32_t time = TICKS_TO_MS(hal_sys_timer_get());
#endif

    if (play_data_handler[AUDIO_OUTPUT_ITF_DAC1]) {
        play_data_handler[AUDIO_OUTPUT_ITF_DAC1](buf, len);
    }

#if defined(DAC1_DEBUG) && (DAC1_DEBUG > 0)
    TRACE(3, "[%u] PLAY1: buf=%x, len=%d, dma_pos=%d", time, (int)buf, len, dma_pos);
#endif
    return 0;
}

static uint32_t codec_playback2_data_handler(uint8_t *buf, uint32_t len)
{
#if defined(DAC2_DEBUG) && (DAC2_DEBUG > 0)
    uint32_t dma_pos = af_stream_get_cur_dma_pos(AF_STREAM_ID_PLAY2, AUD_STREAM_PLAYBACK);
    uint32_t time = TICKS_TO_MS(hal_sys_timer_get());
#endif

    if (play_data_handler[AUDIO_OUTPUT_ITF_DAC2]) {
        play_data_handler[AUDIO_OUTPUT_ITF_DAC2](buf, len);
    }

#if defined(DAC2_DEBUG) && (DAC2_DEBUG > 0)
    TRACE(3, "[%u] PLAY2: buf=%x, len=%d, dma_pos=%d", time, (int)buf, len, dma_pos);
#endif
    return 0;
}

static uint32_t codec_playback3_data_handler(uint8_t *buf, uint32_t len)
{
#if defined(DAC3_DEBUG) && (DAC3_DEBUG > 0)
    uint32_t dma_pos = af_stream_get_cur_dma_pos(AF_STREAM_ID_PLAY3, AUD_STREAM_PLAYBACK);
    uint32_t time = TICKS_TO_MS(hal_sys_timer_get());
#endif

    if (play_data_handler[AUDIO_OUTPUT_ITF_DAC3]) {
        play_data_handler[AUDIO_OUTPUT_ITF_DAC3](buf, len);
    }

#if defined(DAC3_DEBUG) && (DAC3_DEBUG > 0)
    TRACE(3, "[%u] PLAY3: buf=%x, len=%d, dma_pos=%d", time, (int)buf, len, dma_pos);
#endif
    return 0;
}

#define AUDIO_SYSFREQ_USER0 HAL_SYSFREQ_USER_APP_0
#define AUDIO_SYSFREQ_USER1 HAL_SYSFREQ_USER_APP_1
#define AUDIO_SYSFREQ_USER2 HAL_SYSFREQ_USER_APP_2
#define AUDIO_SYSFREQ_USER3 HAL_SYSFREQ_USER_APP_3
#define AUDIO_SYSFREQ_USER4 HAL_SYSFREQ_USER_APP_4

static void audio_output_interface_setup_data_handler(enum AUDIO_OUTPUT_ITF_T itf,
    bool setup, struct AF_STREAM_CONFIG_T *stream_cfg)
{
    uint8_t src_data_idx = aud_output_data_srcs[itf];

    if (setup) {
        switch (src_data_idx) {
        case AUDIO_ZERO_DATA:
            play_data_handler[itf] = play_zero_data;
            TRACE(1, "OUTPUT ITF[%d], PLAY AUDIO_ZERO_DATA", itf);
            break;
        case AUDIO_MIC_DATA:
            if (en_adc2) {
                play_data_handler[itf] = play_mic_data2;
                play2_chan_num = stream_cfg->channel_num;
                play2_samp_bits = stream_cfg->bits;
                TRACE(1, "OUTPUT ITF[%d], PLAY AUDIO_MIC_DATA2", itf);
            } else {
                play_data_handler[itf] = play_mic_data;
                play_chan_num = stream_cfg->channel_num;
                play_samp_bits = stream_cfg->bits;
                TRACE(1, "OUTPUT ITF[%d], PLAY AUDIO_MIC_DATA", itf);
            }
            break;
        case AUDIO_SIN_1K_DATA:
            init_wave_data(&aud_wave1, 1000, stream_cfg->sample_rate,
                                stream_cfg->bits, stream_cfg->channel_num);
            play_data_handler[itf] = play_sin1_data;
            TRACE(1, "OUTPUT ITF[%d], PLAY AUDIO_SIN_1K_DATA", itf);
            break;
        case AUDIO_SIN_2K_DATA:
            init_wave_data(&aud_wave2, 2000, stream_cfg->sample_rate,
                                stream_cfg->bits, stream_cfg->channel_num);
            play_data_handler[itf] = play_sin2_data;
            TRACE(1, "OUTPUT ITF[%d], PLAY AUDIO_SIN_2K_DATA", itf);
            break;
        case AUDIO_SIN_4K_DATA:
            init_wave_data(&aud_wave2, 4000, stream_cfg->sample_rate,
                                stream_cfg->bits, stream_cfg->channel_num);
            play_data_handler[itf] = play_sin2_data;
            TRACE(1, "OUTPUT ITF[%d], PLAY AUDIO_SIN_4K_DATA", itf);
            break;
        }
    } else {
        play_data_handler[itf] = NULL;
        TRACE(1, "OUTPUT ITF[%d], NULL", itf);
    }
}

static int codec_capture1_stream_start(bool on)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

    if (on == adc1_started) {
        TRACE(3, "%s: already started", __func__);
        return 0;
    }
    TRACE(3,"%s: on=%d",__func__, on);
    if (on) {
        hal_sysfreq_req(AUDIO_SYSFREQ_USER1, HAL_CMU_FREQ_52M);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.sample_rate = adc1_samp_rate;
        if (CAPTURE_CHAN_NUM == 1) {
            stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0;
        } else if (CAPTURE_CHAN_NUM == 2) {
            stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0|AUD_CHANNEL_MAP_CH1;
        } else if (CAPTURE_CHAN_NUM == 3) {
            stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0|AUD_CHANNEL_MAP_CH1|AUD_CHANNEL_MAP_CH2;
        }
        stream_cfg.channel_num = CAPTURE_CHAN_NUM;
        stream_cfg.bits        = sample_size_to_bits(adc1_samp_size);
        stream_cfg.device      = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path     = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.vol         = 7;
        stream_cfg.handler     = codec_capture1_data_handler;
        stream_cfg.data_ptr    = capture_buff;
        stream_cfg.data_size   = sizeof(capture_buff);

        cap_chan_num = stream_cfg.channel_num;
        cap_samp_bits = stream_cfg.bits;

        TRACE(3,"start capture: sr=%d, bits=%d, data_ptr=%x,data_size=%d",
            stream_cfg.sample_rate, stream_cfg.bits, (uint32_t)(stream_cfg.data_ptr), stream_cfg.data_size);

        ret = af_stream_open(AF_STREAM_ID_PLAY1, AUD_STREAM_CAPTURE, &stream_cfg);
        ASSERT(ret == 0, "af_stream_open catpure failed: %d", ret);

        ret = af_stream_start(AF_STREAM_ID_PLAY1, AUD_STREAM_CAPTURE);
        ASSERT(ret == 0, "af_stream_start catpure failed: %d", ret);
        adc1_started = true;

    } else {
        af_stream_stop(AF_STREAM_ID_PLAY1, AUD_STREAM_CAPTURE);
        af_stream_close(AF_STREAM_ID_PLAY1, AUD_STREAM_CAPTURE);
        adc1_started = false;

        hal_sysfreq_req(AUDIO_SYSFREQ_USER1, HAL_CMU_FREQ_32K);
    }
    adc1_started = on;
    TRACE(3,"%s: done",__func__);
    return 0;
}

static int codec_capture2_stream_start(bool on)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

    if (on == adc2_started) {
        TRACE(3, "%s: already started", __func__);
        return 0;
    }
    TRACE(3,"%s: on=%d",__func__, on);
    if (on) {
        hal_sysfreq_req(AUDIO_SYSFREQ_USER1, HAL_CMU_FREQ_52M);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.sample_rate = adc2_samp_rate;
        stream_cfg.channel_num = CAPTURE2_CHAN_NUM;
        if (stream_cfg.channel_num == 1) {
            stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0;
        } else if (stream_cfg.channel_num == 2) {
            stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0|AUD_CHANNEL_MAP_CH1;
        } else if (stream_cfg.channel_num == 3) {
            stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0|AUD_CHANNEL_MAP_CH1|AUD_CHANNEL_MAP_CH2;
        }
        stream_cfg.bits        = sample_size_to_bits(adc2_samp_size);
        stream_cfg.device      = AUD_STREAM_USE_INT_CODEC2;
        stream_cfg.io_path     = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.vol         = 7;
        stream_cfg.handler     = codec_capture2_data_handler;
        stream_cfg.data_ptr    = capture2_buff;
        stream_cfg.data_size   = sizeof(capture2_buff);

        cap2_chan_num = stream_cfg.channel_num;
        cap2_samp_bits = stream_cfg.bits;

        TRACE(3,"start capture2: sr=%d, bits=%d, data_ptr=%x,data_size=%d",
            stream_cfg.sample_rate, stream_cfg.bits, (uint32_t)(stream_cfg.data_ptr), stream_cfg.data_size);

        ret = af_stream_open(AF_STREAM_ID_PLAY2, AUD_STREAM_CAPTURE, &stream_cfg);
        ASSERT(ret == 0, "af_stream_open catpure failed: %d", ret);

        ret = af_stream_start(AF_STREAM_ID_PLAY2, AUD_STREAM_CAPTURE);
        ASSERT(ret == 0, "af_stream_start catpure failed: %d", ret);
        adc2_started = true;

    } else {
        af_stream_stop(AF_STREAM_ID_PLAY2, AUD_STREAM_CAPTURE);
        af_stream_close(AF_STREAM_ID_PLAY2, AUD_STREAM_CAPTURE);
        adc2_started = false;

        hal_sysfreq_req(AUDIO_SYSFREQ_USER1, HAL_CMU_FREQ_32K);
    }
    adc2_started = on;
    TRACE(3,"%s: done",__func__);
    return 0;
}

static int codec_playback1_stream_start(bool on)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

    if (dac1_started == on) {
        TRACE(3, "%s: already on=%d", __func__, on);
        return 0;
    }
    if (on) {
        hal_sysfreq_req(AUDIO_SYSFREQ_USER0, HAL_CMU_FREQ_52M);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.sample_rate = dac1_samp_rate;
        stream_cfg.channel_num = PLAYBACK1_CHAN_NUM;
        if (stream_cfg.channel_num == 1) {
            stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0;
        } else {
            stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
        }
        stream_cfg.bits        = sample_size_to_bits(dac1_samp_size);
        stream_cfg.device      = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path     = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.vol         = 17;
        stream_cfg.handler     = codec_playback1_data_handler;
        stream_cfg.data_ptr    = playback1_buff;
        stream_cfg.data_size   = sizeof(playback1_buff);

        TRACE(3,"start playback: sr=%d, bits=%d, data_ptr=%x,data_size=%d",
            stream_cfg.sample_rate, stream_cfg.bits, (uint32_t)(stream_cfg.data_ptr), stream_cfg.data_size);

        audio_output_interface_setup_data_handler(AUDIO_OUTPUT_ITF_DAC1, true, &stream_cfg);

        ret = af_stream_open(AF_STREAM_ID_PLAY1, AUD_STREAM_PLAYBACK, &stream_cfg);
        ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);

        ret = af_stream_start(AF_STREAM_ID_PLAY1, AUD_STREAM_PLAYBACK);
        ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);
        dac1_started = true;
    } else {
        af_stream_stop(AF_STREAM_ID_PLAY1, AUD_STREAM_PLAYBACK);
        af_stream_close(AF_STREAM_ID_PLAY1, AUD_STREAM_PLAYBACK);

        audio_output_interface_setup_data_handler(AUDIO_OUTPUT_ITF_DAC1, false, NULL);
        dac1_started = false;

        hal_sysfreq_req(AUDIO_SYSFREQ_USER0, HAL_CMU_FREQ_32K);
    }
    return 0;
}

static int codec_playback2_stream_start(bool on)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

    if (on == dac2_started) {
        TRACE(3, "%s: already started", __func__);
        return 0;
    }

    if (on) {
        hal_sysfreq_req(AUDIO_SYSFREQ_USER2, HAL_CMU_FREQ_52M);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.sample_rate = dac2_samp_rate;
        stream_cfg.channel_num = PLAYBACK2_CHAN_NUM;
        if (stream_cfg.channel_num == 1) {
            stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0;
        } else {
            stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
        }
        stream_cfg.bits        = sample_size_to_bits(dac2_samp_size);
        stream_cfg.device      = AUD_STREAM_USE_INT_CODEC2;
        stream_cfg.io_path     = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.vol         = 17;
        stream_cfg.handler     = codec_playback2_data_handler;
        stream_cfg.data_ptr    = playback2_buff;
        stream_cfg.data_size   = sizeof(playback2_buff);

        audio_output_interface_setup_data_handler(AUDIO_OUTPUT_ITF_DAC2, true, &stream_cfg);

        TRACE(3,"start playback2: sr=%d, bits=%d, data_ptr=%x,data_size=%d",
            stream_cfg.sample_rate, stream_cfg.bits, (uint32_t)(stream_cfg.data_ptr), stream_cfg.data_size);

        ret = af_stream_open(AF_STREAM_ID_PLAY2, AUD_STREAM_PLAYBACK, &stream_cfg);
        ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);

        ret = af_stream_start(AF_STREAM_ID_PLAY2, AUD_STREAM_PLAYBACK);
        ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);
    } else {
        af_stream_stop(AF_STREAM_ID_PLAY2, AUD_STREAM_PLAYBACK);
        af_stream_close(AF_STREAM_ID_PLAY2, AUD_STREAM_PLAYBACK);

        audio_output_interface_setup_data_handler(AUDIO_OUTPUT_ITF_DAC2, false, NULL);

        hal_sysfreq_req(AUDIO_SYSFREQ_USER2, HAL_CMU_FREQ_32K);
    }
    dac2_started = on;

    TRACE(3,"%s: done",__func__);
    return ret;
}

static int codec_playback3_stream_start(bool on)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

    if (on == dac3_started) {
        TRACE(3, "%s: already done on=%d", __func__, on);
        return 0;
    }
    if (on) {
        hal_sysfreq_req(AUDIO_SYSFREQ_USER3, HAL_CMU_FREQ_104M);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.sample_rate = dac3_samp_rate;
        stream_cfg.channel_num = PLAYBACK3_CHAN_NUM;
        if (stream_cfg.channel_num == 1) {
            stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0;
        } else {
            stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
        }
        stream_cfg.bits        = sample_size_to_bits(dac3_samp_size);
        stream_cfg.device      = AUD_STREAM_USE_INT_CODEC3;
        stream_cfg.io_path     = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.vol         = 17;
        stream_cfg.handler     = codec_playback3_data_handler;
        stream_cfg.data_ptr    = playback3_buff;
        stream_cfg.data_size   = sizeof(playback3_buff);

        audio_output_interface_setup_data_handler(AUDIO_OUTPUT_ITF_DAC3, true, &stream_cfg);

        TRACE(3,"start playback3: sr=%d, bits=%d, data_ptr=%x,data_size=%d",
            stream_cfg.sample_rate, stream_cfg.bits, (uint32_t)(stream_cfg.data_ptr), stream_cfg.data_size);

        ret = af_stream_open(AF_STREAM_ID_PLAY3, AUD_STREAM_PLAYBACK, &stream_cfg);
        ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);

        ret = af_stream_start(AF_STREAM_ID_PLAY3, AUD_STREAM_PLAYBACK);
        ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);
    } else {
        af_stream_stop(AF_STREAM_ID_PLAY3, AUD_STREAM_PLAYBACK);
        af_stream_close(AF_STREAM_ID_PLAY3, AUD_STREAM_PLAYBACK);

        audio_output_interface_setup_data_handler(AUDIO_OUTPUT_ITF_DAC3, false, NULL);

        hal_sysfreq_req(AUDIO_SYSFREQ_USER3, HAL_CMU_FREQ_32K);
    }
    dac3_started = on;

    TRACE(3,"%s: done, on=%d",__func__, on);
    return ret;
}

static int audio_loop_itf_set_output_data_sources(enum AUDIO_OUTPUT_ITF_T out)
{
    ASSERT(out < AUDIO_OUTPUT_ITF_QTY, "wrong param out:%d", out);
    if (g_dac_combined_test) {
        if (out == g_aud_out_itf) {
            if (out == AUDIO_OUTPUT_ITF_DAC3) {
                aud_output_data_srcs[out] = AUDIO_SIN_2K_DATA;
            } else {
                aud_output_data_srcs[out] = AUDIO_SIN_1K_DATA;
            }
        } else {
            aud_output_data_srcs[out] = AUDIO_MIC_DATA;
        }
    } else {
        switch (out) {
        case AUDIO_OUTPUT_ITF_DAC1:
            TRACE(1, "DAC1 OUTPUT MIC DATA FOR LOOP ITF");
            aud_output_data_srcs[AUDIO_OUTPUT_ITF_DAC1] = AUDIO_MIC_DATA;
            aud_output_data_srcs[AUDIO_OUTPUT_ITF_DAC2] = AUDIO_SIN_1K_DATA;
            aud_output_data_srcs[AUDIO_OUTPUT_ITF_DAC3] = AUDIO_SIN_2K_DATA;
            break;
        case AUDIO_OUTPUT_ITF_DAC2:
            TRACE(1, "DAC2 OUTPUT MIC DATA FOR LOOP ITF");
            aud_output_data_srcs[AUDIO_OUTPUT_ITF_DAC1] = AUDIO_SIN_1K_DATA;
            aud_output_data_srcs[AUDIO_OUTPUT_ITF_DAC2] = AUDIO_MIC_DATA;
            aud_output_data_srcs[AUDIO_OUTPUT_ITF_DAC3] = AUDIO_SIN_2K_DATA;
            break;
        case AUDIO_OUTPUT_ITF_DAC3:
            TRACE(1, "DAC3 OUTPUT MIC DATA FOR LOOP ITF");
            aud_output_data_srcs[AUDIO_OUTPUT_ITF_DAC1] = AUDIO_SIN_1K_DATA;
            aud_output_data_srcs[AUDIO_OUTPUT_ITF_DAC2] = AUDIO_SIN_2K_DATA;
            aud_output_data_srcs[AUDIO_OUTPUT_ITF_DAC3] = AUDIO_MIC_DATA;
            break;
        default:
            break;
        }
    }
    return 0;
}

static int codec_loop_stream_open(enum AUDIO_INPUT_ITF_T in_itf, enum AUDIO_OUTPUT_ITF_T out_itf)
{
    int r = 0;

    TRACE(1, "[loop_stream_open]: in_itf=%d, out_itf=%d", in_itf, out_itf);

#ifdef NO_LOOP_STREAM
    return 0;
#endif
    bool *stream_opened = &adda_opened;
    bool *is_running = &adda_started;
    bool is_dma_dac = false;
    if (in_itf == AUDIO_INPUT_ITF_ADC2) {
        en_adc2 = true;
    } else {
        en_adc2 = false;
    }
    if (in_itf == AUDIO_INPUT_ITF_ADC2 && out_itf == AUDIO_OUTPUT_ITF_DAC3) {
        stream_opened = &g_dma_aud_opened;
        is_running = &g_dma_aud_started;
        is_dma_dac = true;
    }

    if ((in_itf == g_loop_in_itf) && (out_itf == g_loop_out_itf)) {
        goto _end;
    }
    TRACE(1, "update itf: input[%d]->[%d], output[%d]->[%d]",
        g_loop_in_itf, in_itf, g_loop_out_itf, out_itf);
    if (is_dma_dac) {
        if (*stream_opened) {

            if (!(*is_running)) {
                r = codec_loop_stream_start(in_itf, out_itf);
                if (r) {
                    TRACE(1, "%s: open stream failed, r=%d", __func__, r);
                    return r;
                }
            }
        }
    } else {
        if (*stream_opened) {
            bool running = *is_running;
            if (running) {
                r = codec_loop_stream_stop(g_loop_in_itf, g_loop_out_itf);
                if (r) {
                    TRACE(1, "%s: stop stream failed, r=%d", __func__, r);
                    return r;
                }
            }
            g_loop_in_itf  = in_itf;
            g_loop_out_itf = out_itf;
            if (running) {
                r = codec_loop_stream_start(in_itf, out_itf);
                if (r) {
                    TRACE(1, "%s: start stream failed, r=%d", __func__, r);
                    return r;
                }
            }
        } else {
            g_loop_in_itf  = in_itf;
            g_loop_out_itf = out_itf;
        }
    }

_end:
    *stream_opened = true;
    TRACE(1, "[loop_stream_open]: done");

    return r;
}

static int codec_loop_stream_close(enum AUDIO_INPUT_ITF_T in, enum AUDIO_OUTPUT_ITF_T out)
{
    int r = 0;

#ifdef NO_LOOP_STREAM
    return 0;
#endif
    bool *stream_opened = &adda_opened;
    bool *is_running = &adda_started;
    bool is_dma_dac = false;
    if (in == AUDIO_INPUT_ITF_ADC2 && out == AUDIO_OUTPUT_ITF_DAC3) {
        stream_opened = &g_dma_aud_opened;
        is_running = &g_dma_aud_started;
        is_dma_dac = true;
    }
    if (!(*stream_opened)) {
        return 0;
    }
    if (*is_running) {
        if (g_dac_combined_test) {
            if (g_loop_audio_en) {
                r += codec_loop_stream_stop(g_loop_in_itf, g_loop_out_itf);
            }
            if (g_dma_audio_en && is_dma_dac) {
                r += codec_loop_stream_stop(g_dma_aud_in_itf, g_dma_aud_out_itf);
            }
        } else {
            r = codec_loop_stream_stop(in, out);
        }
        if (r) {
            return r;
        }
    }
    *stream_opened = false;
    return r;
}

static int codec_loop_stream_start(enum AUDIO_INPUT_ITF_T in, enum AUDIO_OUTPUT_ITF_T out)
{
    enum AUDIO_INPUT_ITF_T in_itf = in;
    enum AUDIO_OUTPUT_ITF_T out_itf = out;

#ifdef NO_LOOP_STREAM
    return 0;
#endif
    bool *adda_is_open = &adda_opened;
    bool *adda_is_start = &adda_started;
    if (in == AUDIO_INPUT_ITF_ADC2 && out == AUDIO_OUTPUT_ITF_DAC3) {
        adda_is_open = &g_dma_aud_opened;
        adda_is_start = &g_dma_aud_started;
    }
    ASSERT(*adda_is_open == true, "%s: not opened", __func__);

    if (*adda_is_start) {
        TRACE(1, "loop already started");
        return 0;
    }

    TRACE(1, "[loop_stream_start]: in_itf=%d, out_itf=%d", in_itf, out_itf);

    audio_loop_itf_set_output_data_sources(out_itf);

    switch (out_itf) {
    case AUDIO_OUTPUT_ITF_DAC1:
        dac1_samp_rate = adda_samp_rate;
        dac1_samp_size = adda_samp_size;
        codec_playback1_stream_start(true);
        break;
    case AUDIO_OUTPUT_ITF_DAC2:
        dac2_samp_rate = adda_samp_rate;
        dac2_samp_size = adda_samp_size;
        codec_playback2_stream_start(true);
        break;
    case AUDIO_OUTPUT_ITF_DAC3:
        dac3_samp_rate = adda_samp_rate;
        dac3_samp_size = adda_samp_size;
        codec_playback3_stream_start(true);
        break;
    default:
        break;
    }
    switch (in_itf) {
    case AUDIO_INPUT_ITF_ADC1:
        adc1_samp_size = adda_samp_size;
        codec_capture1_stream_start(true);
        break;
    case AUDIO_INPUT_ITF_ADC2:
        // adc2_samp_rate = adda_samp_rate;
        adc2_samp_size = adda_samp_size;
        codec_capture2_stream_start(true);
        break;
    default:
        break;
    }
    *adda_is_start = true;
    return 0;
}

static int codec_loop_stream_stop(enum AUDIO_INPUT_ITF_T in, enum AUDIO_OUTPUT_ITF_T out)
{
    enum AUDIO_INPUT_ITF_T in_itf = in;
    enum AUDIO_OUTPUT_ITF_T out_itf = out;

#ifdef NO_LOOP_STREAM
    return 0;
#endif
    bool *adda_is_open = &adda_opened;
    bool *adda_is_start = &adda_started;
    if (in == AUDIO_INPUT_ITF_ADC2 && out == AUDIO_OUTPUT_ITF_DAC3) {
        adda_is_open = &g_dma_aud_opened;
        adda_is_start = &g_dma_aud_started;
    }
    TRACE(1, "[loop_stream_stop]: in_itf=%d, out_itf=%d", in_itf, out_itf);
    ASSERT(*adda_is_open == true, "%s: not opened", __func__);

    if (!(*adda_is_start)) {
        TRACE(1, "loop already stopped");
        return 0;
    }

    switch (in_itf) {
    case AUDIO_INPUT_ITF_ADC1:
        codec_capture1_stream_start(false);
        break;
    case AUDIO_INPUT_ITF_ADC2:
        codec_capture2_stream_start(false);
        break;
    default:
        break;
    }
    switch (out_itf) {
    case AUDIO_OUTPUT_ITF_DAC1:
        codec_playback1_stream_start(false);
        break;
    case AUDIO_OUTPUT_ITF_DAC2:
        codec_playback2_stream_start(false);
        break;
    case AUDIO_OUTPUT_ITF_DAC3:
        codec_playback3_stream_start(false);
        break;
    default:
        break;
    }
    *adda_is_start = false;
    return 0;
}

static void audio_player_thread(void)
{
#ifndef RTOS
   extern void af_thread(void const *arguments);
   af_thread(NULL);
#endif
}

enum AUDIO_TEST_PHASE_T {
    AUDIO_TEST_PHASE_IDLE = -1,
    AUDIO_TEST_PHASE_INIT = 0,
    AUDIO_TEST_PHASE_START_PLAY,
    AUDIO_TEST_PHASE_STOP_PLAY,
    AUDIO_TEST_PHASE_DONE,
    AUDIO_TEST_PHASE_STOP,
};

static enum AUDIO_TEST_PHASE_T phase = AUDIO_TEST_PHASE_IDLE;
static enum AUDIO_TEST_PHASE_T bk_phase = AUDIO_TEST_PHASE_IDLE;
static int count = 0;

static void audio_stream_test_itf_open(enum AUDIO_INPUT_ITF_T in, enum AUDIO_OUTPUT_ITF_T out)
{
    g_aud_in_itf = in;
    g_aud_out_itf = out;
    count = 0;
    phase = AUDIO_TEST_PHASE_IDLE;
}

static void audio_stream_test_itf_close(void)
{
}

static void audio_stream_test_start(void)
{
    TRACE(1, "%s:", __func__);
    count = TEST_MAX_COUNT;
    phase = AUDIO_TEST_PHASE_INIT;
}

static void audio_stream_test_stop(void)
{
    TRACE(1, "%s:", __func__);
    phase = AUDIO_TEST_PHASE_STOP;
}

void audio_stream_test_pause(void)
{
    TRACE(1, "%s:", __func__);
    bk_phase = phase;
    phase = AUDIO_TEST_PHASE_IDLE;
}

void audio_stream_test_resume(void)
{
    TRACE(1, "%s:", __func__);
    phase = bk_phase;
    bk_phase = AUDIO_TEST_PHASE_IDLE;
}

//#define BYPASS_TESTER
#ifdef PLAY_NO_SWITCH
#define START_PLAY_NO_SWITCH
#define STOP_PLAY_NO_SWITCH
#endif
//#define BOTH_DAC

static enum AUDIO_TEST_RET_T audio_stream_tester(void)
{
    static uint32_t time_a = 0, time_b = 0;

    enum AUDIO_TEST_RET_T ret = AUDIO_TEST_SUCCESS;
    enum AUDIO_OUTPUT_ITF_T itf = g_aud_out_itf;

#ifdef BYPASS_TESTER
    return ret;
#endif
    time_b = TICKS_TO_MS(hal_sys_timer_get());

    switch (phase) {
    case AUDIO_TEST_PHASE_IDLE:
        break;
    case AUDIO_TEST_PHASE_INIT:
        TRACE(1, "PHASE_INIT");
        time_a = time_b;
        phase++;
        break;
    case AUDIO_TEST_PHASE_START_PLAY:
        // TRACE(3, "b=%d, a=%d, b-a=%d", time_b, time_a, time_b-time_a);
        if (time_b - time_a < time_on) {
            if (itf == AUDIO_OUTPUT_ITF_DAC1) {
                if (!dac1_started) {
                    TRACE(1, "PHASE_START_PLAY: DAC1");
                    codec_playback1_stream_start(true);
                }
            } else if (itf == AUDIO_OUTPUT_ITF_DAC2) {
                if (!dac2_started) {
                    TRACE(1, "PHASE_START_PLAY: DAC2");
                    codec_playback2_stream_start(true);
                }
#ifdef BOTH_DAC
                if (!dac3_started) {
                    TRACE(1, "PHASE_START_PLAY: DAC3");
                    codec_playback3_stream_start(true);
                }
#endif
            } else if (itf == AUDIO_OUTPUT_ITF_DAC3) {
                if (!dac3_started) {
                    TRACE(1, "PHASE_START_PLAY: DAC3");
                    codec_playback3_stream_start(true);
                }
#ifdef BOTH_DAC
                if (!dac2_started) {
                    TRACE(1, "PHASE_START_PLAY: DAC2");
                    codec_playback2_stream_start(true);
                }
#endif
            }
            if (g_dac_combined_test) {
                if (g_loop_audio_en && !g_loop_always_on) {
                    if (!adda_started) {
                        codec_loop_stream_open(g_loop_in_itf, g_loop_out_itf);
                        codec_loop_stream_start(g_loop_in_itf, g_loop_out_itf);
                    }
                }
                if (g_dma_audio_en && !g_dma_aud_always_on) {
                    if (!g_dma_aud_started) {
                        codec_loop_stream_open(g_dma_aud_in_itf, g_dma_aud_out_itf);
                        codec_loop_stream_start(g_dma_aud_in_itf, g_dma_aud_out_itf);
                    }
                }
            }
        } else {
            time_a = time_b;
#ifndef START_PLAY_NO_SWITCH
            phase++;
            TRACE(1, "PHASE_START_PLAY: SWITCH");
#endif
        }
        break;
    case AUDIO_TEST_PHASE_STOP_PLAY:
        if (time_b - time_a < time_off) {
            if (!g_dac_always_on || (count == 1)) {    // the last time, stop dac stream
                if (itf == AUDIO_OUTPUT_ITF_DAC1) {
                    if (dac1_started) {
                        TRACE(1, "PHASE_STOP_PLAY: DAC1");
                        codec_playback1_stream_start(false);
                    }
                } else if (itf == AUDIO_OUTPUT_ITF_DAC2) {
                    if (dac2_started) {
                        TRACE(1, "PHASE_STOP_PLAY: DAC2");
                        codec_playback2_stream_start(false);
                    }
#ifdef BOTH_DAC
                    if (dac3_started) {
                        TRACE(1, "PHASE_STOP_PLAY: DAC3");
                        codec_playback3_stream_start(false);
                    }
#endif
                } else if (itf == AUDIO_OUTPUT_ITF_DAC3) {
                    if (dac3_started) {
                        TRACE(1, "PHASE_STOP_PLAY: DAC3");
                        codec_playback3_stream_start(false);
                    }
#ifdef BOTH_DAC
                    if (dac2_started) {
                        TRACE(1, "PHASE_STOP_PLAY: DAC2");
                        codec_playback2_stream_start(false);
                    }
#endif
                }
            }
            if (g_dac_combined_test) {
                if (g_loop_audio_en && !g_loop_always_on) {
                    if (adda_started) {
                        codec_loop_stream_stop(g_loop_in_itf, g_loop_out_itf);
                    }
                }
                if (g_dma_audio_en && !g_dma_aud_always_on) {
                    if (g_dma_aud_started) {
                        codec_loop_stream_stop(g_dma_aud_in_itf, g_dma_aud_out_itf);
                    }
                }
            }
        } else {
            time_a = time_b;
#ifndef STOP_PLAY_NO_SWITCH
            phase++;
            TRACE(1, "PHASE_STOP_PLAY: SWITCH");
#endif
        }
        break;
    case AUDIO_TEST_PHASE_DONE:
        TRACE(1, "PHASE_DONE");
        if (count > 0) {
            count--;
        }
        if (count == 0) {
            ret = AUDIO_TEST_EXIT;
            phase = AUDIO_TEST_PHASE_IDLE;
        } else {
            phase = AUDIO_TEST_PHASE_INIT;
        }
        break;
    case AUDIO_TEST_PHASE_STOP:
        if (!g_dac_always_on || (count == 1)) {    // the last time, stop dac stream
            if (itf == AUDIO_OUTPUT_ITF_DAC1) {
                if (dac1_started) {
                    TRACE(1, "PHASE_STOP_PLAY: DAC1");
                    codec_playback1_stream_start(false);
                }
            } else if (itf == AUDIO_OUTPUT_ITF_DAC2) {
                if (dac2_started) {
                    TRACE(1, "PHASE_STOP_PLAY: DAC2");
                    codec_playback2_stream_start(false);
                }
#ifdef BOTH_DAC
                if (dac3_started) {
                    TRACE(1, "PHASE_STOP_PLAY: DAC3");
                    codec_playback3_stream_start(false);
                }
#endif
            } else if (itf == AUDIO_OUTPUT_ITF_DAC3) {
                if (dac3_started) {
                    TRACE(1, "PHASE_STOP_PLAY: DAC3");
                    codec_playback3_stream_start(false);
                }
#ifdef BOTH_DAC
                if (dac2_started) {
                    TRACE(1, "PHASE_STOP_PLAY: DAC2");
                    codec_playback2_stream_start(false);
                }
#endif
            }
        }
        phase = AUDIO_TEST_PHASE_IDLE;
        break;
    default:
        TRACE(1, "reset to PHASE_IDLE");
        phase = AUDIO_TEST_PHASE_IDLE;
        break;
    }
    return ret;
}

static void display_config(bool start)
{
    TRACE(3, "****************************");
    TRACE(3, "g_loop_in=%d, out=%d, en=%d, always_on=%d", g_loop_in_itf, g_loop_out_itf, g_loop_audio_en, g_loop_always_on);
    TRACE(3, "g_dma_aud_in=%d, out=%d, en=%d, lways_on=%d", g_dma_aud_in_itf, g_dma_aud_out_itf, g_dma_audio_en, g_dma_aud_always_on);
    TRACE(3, "g_dac_output=%d, en=%d, always_on=%d", g_aud_out_itf, g_dac_audio_en, g_dac_always_on);
    if (!start) {
        TRACE(3, "test done");
    }
    TRACE(3, "****************************\n");
}

void dac_hw_test(void)
{
    int ret;

    display_config(true);

    af_open();

    if (g_loop_audio_en) {
        codec_loop_stream_open(g_loop_in_itf, g_loop_out_itf);
        codec_loop_stream_start(g_loop_in_itf, g_loop_out_itf);
    }
    if (g_dma_audio_en) {
        codec_loop_stream_open(g_dma_aud_in_itf, g_dma_aud_out_itf);
        codec_loop_stream_start(g_dma_aud_in_itf, g_dma_aud_out_itf);
    }
    if (g_dac_audio_en) {
        audio_stream_test_itf_open(AUDIO_INPUT_ITF_QTY, g_aud_out_itf);  // g_aud_in_itf is not used for now.
    }

    audio_stream_test_start();

    while (1) {
        audio_player_thread();
        ret = audio_stream_tester();
        if (ret == AUDIO_TEST_ERROR) {
            TRACE(3, "audio test error !!");
            break;
        } else if (ret == AUDIO_TEST_EXIT) {
            TRACE(3, "audio test exit !!");
            break;
        }
    }

    audio_stream_test_stop();
    audio_stream_test_itf_close();

    if (g_loop_audio_en) {
        if (adda_started) {
            codec_loop_stream_stop(g_loop_in_itf, g_loop_out_itf);
        }
        if (adda_opened) {
            codec_loop_stream_close(g_loop_in_itf, g_loop_out_itf);
        }
    }

    if (g_dma_audio_en) {
        if (g_dma_aud_started) {
            codec_loop_stream_stop(g_dma_aud_in_itf, g_dma_aud_out_itf);
        }
        if (g_dma_aud_opened) {
            codec_loop_stream_close(g_dma_aud_in_itf, g_dma_aud_out_itf);
        }
    }

    af_close();

    display_config(false);
    if (ret == AUDIO_TEST_ERROR) {
        TRACE(3, "====> AUDIO DAC TEST ERROR ###");
    } else {
        TRACE(3, "====> AUDIO DAC TEST SUCCESS !!!");
    }
}

static void loop_stream_config(enum AUDIO_INPUT_ITF_T in, enum AUDIO_OUTPUT_ITF_T out)
{
    ASSERT(in < AUDIO_INPUT_ITF_QTY && out < AUDIO_OUTPUT_ITF_QTY, "wrong itf, in=%d, out=%d", in, out);
    if (out == AUDIO_OUTPUT_ITF_DAC3 && in == AUDIO_INPUT_ITF_ADC2) {
        g_dma_audio_en = true;
        g_dma_aud_out_itf = out;
        g_dma_aud_in_itf = in;
    } else {
        g_loop_audio_en = true;
        g_loop_out_itf = in;
        g_loop_in_itf = out;
    }
}

static void loop_stream_deconfig(void)
{
    g_dma_audio_en = false;
    g_dma_aud_in_itf = AUDIO_INPUT_ITF_QTY;
    g_dma_aud_out_itf = AUDIO_OUTPUT_ITF_QTY;
    g_loop_audio_en = false;
    g_loop_out_itf = AUDIO_OUTPUT_ITF_QTY;
    g_loop_in_itf = AUDIO_INPUT_ITF_QTY;
}

static void audio_stream_config(enum AUDIO_OUTPUT_ITF_T out)
{
    g_aud_out_itf = out;
    g_dac_audio_en = true;
}

static void audio_stream_deconfig(void)
{
    g_aud_out_itf = AUDIO_OUTPUT_ITF_QTY;
    g_dac_audio_en = false;
}

static void stream_test_always_on(bool loop_always_on, bool dma_aud_always_on, bool dac_always_on)
{
    g_loop_always_on = loop_always_on;
    g_dac_always_on = dac_always_on;
    g_dma_aud_always_on = dma_aud_always_on;
}
static void stream_test_always_on_clear(void)
{
    g_loop_always_on = false;
    g_dac_always_on = false;
    g_dma_aud_always_on = false;
}

static void audio_loop_itf_init_output_data_sources(void)
{
    aud_output_data_srcs[AUDIO_OUTPUT_ITF_DAC1] = AUDIO_SIN_2K_DATA;
    aud_output_data_srcs[AUDIO_OUTPUT_ITF_DAC2] = AUDIO_SIN_2K_DATA;
    aud_output_data_srcs[AUDIO_OUTPUT_ITF_DAC3] = AUDIO_SIN_2K_DATA;
}

static void test_init(bool dac_combined_test)
{
    if (dac_combined_test) {
        time_on = TIME_PERIOD_DAC_COMB_ON;
        time_off = TIME_PERIOD_DAC_COMB_OFF;
        g_dac_combined_test = true;
    } else {
        time_on= TIME_PERIOD_DAC_ON;
        time_off = TIME_PERIOD_DAC_OFF;
        g_dac_combined_test = false;
    }
    stream_test_always_on_clear();
    audio_stream_deconfig();
    loop_stream_deconfig();
    audio_loop_itf_init_output_data_sources();
}

void dac_test_combined(void)
{
    /* adc1+dac1 as loop, dac2 as prompt */
    test_init(true);
    loop_stream_config(AUDIO_INPUT_ITF_ADC1, AUDIO_OUTPUT_ITF_DAC1);
    audio_stream_config(AUDIO_OUTPUT_ITF_DAC2);
    stream_test_always_on(true, false, false);
    dac_hw_test();

    /* adc1+dac1 as loop, dac3 as audio output */
    test_init(true);
    loop_stream_config(AUDIO_INPUT_ITF_ADC1, AUDIO_OUTPUT_ITF_DAC1);
    audio_stream_config(AUDIO_OUTPUT_ITF_DAC3);
    stream_test_always_on(true, false, false);
    dac_hw_test();

    /* adc2+dac3 as loop (low latency), dac1 as music */
    /* case1: loop always on, dac1 turn on/off periodically */
    test_init(true);
    loop_stream_config(AUDIO_INPUT_ITF_ADC2, AUDIO_OUTPUT_ITF_DAC3);
    audio_stream_config(AUDIO_OUTPUT_ITF_DAC1);
    stream_test_always_on(false, true, false);
    dac_hw_test();

    /* case2: loop turn on/off periodcally, dac1 always on */
    test_init(true);
    loop_stream_config(AUDIO_INPUT_ITF_ADC2, AUDIO_OUTPUT_ITF_DAC3);
    audio_stream_config(AUDIO_OUTPUT_ITF_DAC1);
    stream_test_always_on(false, false, true);
    dac_hw_test();

    /* adc1+dac1 as loop, adc2+dac3 as dma audio */
    /* loop always on and dma audio turn on/off periodically */
    test_init(true);
    loop_stream_config(AUDIO_INPUT_ITF_ADC1, AUDIO_OUTPUT_ITF_DAC1);
    loop_stream_config(AUDIO_INPUT_ITF_ADC2, AUDIO_OUTPUT_ITF_DAC3);
    stream_test_always_on(true, false, false);
    dac_hw_test();

    /* loop turn on/off periodically and dma audio always on */
    test_init(true);
    loop_stream_config(AUDIO_INPUT_ITF_ADC2, AUDIO_OUTPUT_ITF_DAC3);
    loop_stream_config(AUDIO_INPUT_ITF_ADC1, AUDIO_OUTPUT_ITF_DAC1);
    stream_test_always_on(false, true, false);
    dac_hw_test();
    test_init(false);

}

void dac_test(void)
{
#if defined(DAC1_TEST) || defined(DAC_TEST)
    test_init(false);
    loop_stream_config(AUDIO_INPUT_ITF_ADC1, AUDIO_OUTPUT_ITF_DAC1);
    audio_stream_config(AUDIO_OUTPUT_ITF_DAC1);
    stream_test_always_on(false, false, false);
    dac_hw_test();
#endif
#ifdef DAC2_TEST
    test_init(false);
    loop_stream_config(AUDIO_INPUT_ITF_ADC1, AUDIO_OUTPUT_ITF_DAC1);
    audio_stream_config(AUDIO_OUTPUT_ITF_DAC2);
    stream_test_always_on(false, false, false);
    dac_hw_test();
#endif
#ifdef DAC3_TEST
    test_init(false);
    loop_stream_config(AUDIO_INPUT_ITF_ADC1, AUDIO_OUTPUT_ITF_DAC1);
    audio_stream_config(AUDIO_OUTPUT_ITF_DAC3);
    stream_test_always_on(false, false, false);
    dac_hw_test();
#endif

#ifdef DAC_TEST_COMBINED
    dac_test_combined();
#endif

}

#endif
