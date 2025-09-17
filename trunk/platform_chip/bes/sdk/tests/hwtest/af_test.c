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
#include "hwtest.h"
#include "audioflinger.h"
#include "hal_cmu.h"
#include "hal_sysfreq.h"
#include "string.h"
#include "hal_wifitsf.h"

#ifdef ANC_APP
#include "anc_process.h"
#include "aud_section.h"

extern const struct_anc_cfg * anc_coef_list_50p7k[];
#endif

// AF_TEST

//#define PLAYBACK_CODEC3
//#define PLAYBACK_CODEC2
//#define CAPTURE_CODEC3
//#define CAPTURE_CODEC2

//#define PLAYBACK_ONLY
//#define PLAYBACK_TONE
//#define PLAYBACK_START_STOP_LOOP

#define PLAYBACK_SAMPLE_RATE            48000
#define CAPTURE_SAMPLE_RATE             48000

#define ALIGNED4                        ALIGNED(4)

#define NON_EXP_ALIGN(val, exp)         (((val) + ((exp) - 1)) / (exp) * (exp))

#define AF_DMA_LIST_CNT                 4

#ifndef CHAN_NUM_CAPTURE
#if defined(CHIP_BEST2000) && defined(AF_INT_CODEC_TEST)
#define CHAN_NUM_CAPTURE                4
#elif defined(CHIP_BEST2000) && defined(AF_I2S_TEST)
#define CHAN_NUM_CAPTURE                8
#else
#define CHAN_NUM_CAPTURE                2
#endif
#endif

#if !defined(CHIP_BEST1000) && defined(AF_CAP_SEP_BUF)
#define CHAN_SEP_BUF_CAPTURE            true
#else
#define CHAN_SEP_BUF_CAPTURE            false
#endif

#ifdef AF_INT_CODEC_TEST
#ifdef PLAYBACK_CODEC3
#define PLAYBACK_STREAM_DEVICE          AUD_STREAM_USE_INT_CODEC3
#elif defined(PLAYBACK_CODEC2)
#define PLAYBACK_STREAM_DEVICE          AUD_STREAM_USE_INT_CODEC2
#else
#define PLAYBACK_STREAM_DEVICE          AUD_STREAM_USE_INT_CODEC
#endif
#ifdef CAPTURE_CODEC3
#define CAPTURE_STREAM_DEVICE           AUD_STREAM_USE_INT_CODEC3
#elif defined(CAPTURE_CODEC2)
#define CAPTURE_STREAM_DEVICE           AUD_STREAM_USE_INT_CODEC2
#else
#define CAPTURE_STREAM_DEVICE           AUD_STREAM_USE_INT_CODEC
#endif
#elif defined(AF_EXT_CODEC_TEST)
#define PLAYBACK_STREAM_DEVICE          AUD_STREAM_USE_EXT_CODEC
#define CAPTURE_STREAM_DEVICE           AUD_STREAM_USE_EXT_CODEC
#elif defined(AF_SPDIF_TEST)
#define PLAYBACK_STREAM_DEVICE          AUD_STREAM_USE_SPDIF0
#define CAPTURE_STREAM_DEVICE           AUD_STREAM_USE_SPDIF0
#else
#define PLAYBACK_STREAM_DEVICE          AUD_STREAM_USE_I2S0_MASTER
#define CAPTURE_STREAM_DEVICE           AUD_STREAM_USE_I2S0_MASTER
#endif

#define TX_BUFF_ALIGN                   (4 * AF_DMA_LIST_CNT)
#ifdef CHIP_BEST1000
#define RX_BUFF_ALIGN                   (4 * AF_DMA_LIST_CNT)
#else
#define RX_BURST_NUM                    4
#define RX_ALL_CH_DMA_BURST_SIZE        (RX_BURST_NUM * 2 * CHAN_NUM_CAPTURE)
#define RX_BUFF_ALIGN                   (RX_ALL_CH_DMA_BURST_SIZE * AF_DMA_LIST_CNT)
#endif

#define PLAYBACK_FRAME_SIZE             (PLAYBACK_SAMPLE_RATE / 1000 * 2 * 2)
#define CAPTURE_FRAME_SIZE              (CAPTURE_SAMPLE_RATE / 1000 * 2 * CHAN_NUM_CAPTURE)

#define PLAYBACK_SIZE                   NON_EXP_ALIGN(PLAYBACK_FRAME_SIZE * 4, TX_BUFF_ALIGN)
#define CAPTURE_SIZE                    NON_EXP_ALIGN(CAPTURE_FRAME_SIZE * 4, RX_BUFF_ALIGN)

static uint8_t ALIGNED4 playback_buf[PLAYBACK_SIZE];
#ifndef PLAYBACK_ONLY
#ifdef FIXED_BUF_CAPTURE
static uint8_t *capture_buf = (uint8_t *)0x20010000;
#else
static uint8_t ALIGNED4 capture_buf[CAPTURE_SIZE];
#endif
#endif

#ifdef PLAYBACK_TONE
static const int16_t tone_1k[] = {
    #include "../../apps/factory/1k_2ch_48k_16bit.txt"
};
static uint32_t tone_offset;
#endif

static uint32_t af_playback(uint8_t *buf, uint32_t len)
{
#ifdef PLAYBACK_TONE
    const uint8_t *src = (uint8_t *)tone_1k;
    const uint32_t src_len = sizeof(tone_1k);
    uint32_t remain;

    while (len) {
        if (tone_offset >= src_len) {
            tone_offset = 0;
        }
        remain = src_len - tone_offset;
        if (remain > len) {
            remain = len;
        }
        memcpy(buf, src + tone_offset, remain);
        len -= remain;
        tone_offset += remain;
    }
#else
    TRACE(0, "AF_PLAYBACK: buf=%p len=%u", buf, len);
#endif

    return 0;
}

#ifndef PLAYBACK_ONLY
static uint32_t af_capture(uint8_t *buf, uint32_t len)
{
    if (CHAN_SEP_BUF_CAPTURE) {
        uint8_t *chan_buf[CHAN_NUM_CAPTURE];
        uint32_t chan_len;
        int i;

        chan_len = len / CHAN_NUM_CAPTURE;
        for (i = 0; i < CHAN_NUM_CAPTURE; i++) {
            chan_buf[i] = buf + i * chan_len * 2;
            TRACE(0, "AF_CAPTURE : chan=%d chan_buf=%p chan_len=%u", i, chan_buf[i], chan_len);
        }
    } else {
        TRACE(0, "AF_CAPTURE : buf=%p len=%u", buf, len);
    }

    return 0;
}
#endif

#ifdef AF_TEST_TSF_TRIGGER
static void wifitsf_set_trigger_time(uint64_t trigger_time)
{
    uint64_t current_time;
    uint32_t low;
    uint32_t high;

    current_time = hal_wifitsf_timer_get();
    trigger_time += current_time;
    low = (uint32_t)trigger_time;
    high = (uint32_t)(trigger_time >> 32);
    hal_wifitsf_set_trigger_time(low, high);
}
#endif

void af_test(void)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

    af_open();

#ifdef ANC_APP
    af_anc_open(ANC_FEEDFORWARD, AUD_SAMPRATE_48000, AUD_SAMPRATE_48000, NULL);
    anc_open(ANC_FEEDFORWARD);
    anc_set_cfg(anc_coef_list_50p7k[0], ANC_FEEDFORWARD, ANC_GAIN_NO_DELAY);
    anc_set_gain(512, 512, ANC_FEEDFORWARD);
#endif

    memset(&stream_cfg, 0, sizeof(stream_cfg));

    stream_cfg.bits = AUD_BITS_16;
    stream_cfg.channel_num = AUD_CHANNEL_NUM_2;
    stream_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    stream_cfg.sample_rate = PLAYBACK_SAMPLE_RATE;
    stream_cfg.device = PLAYBACK_STREAM_DEVICE;
    stream_cfg.vol = TGT_VOLUME_LEVEL_MAX;
    stream_cfg.handler = af_playback;
    stream_cfg.io_path = AUD_IO_PATH_NULL;
    stream_cfg.data_ptr = playback_buf;
    stream_cfg.data_size = PLAYBACK_SIZE;

    ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
    ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);

#ifndef PLAYBACK_ONLY
    enum AUD_CHANNEL_MAP_T chan_map_cap;

    chan_map_cap = 0;
    for (int i = 0; i < CHAN_NUM_CAPTURE; i++) {
        chan_map_cap |= AUD_CHANNEL_MAP_CH0 << i;
    }

    memset(&stream_cfg, 0, sizeof(stream_cfg));

    stream_cfg.bits = AUD_BITS_16;
    stream_cfg.channel_num = CHAN_NUM_CAPTURE;
    stream_cfg.channel_map = chan_map_cap;
    stream_cfg.chan_sep_buf = CHAN_SEP_BUF_CAPTURE;
    stream_cfg.sample_rate = CAPTURE_SAMPLE_RATE;
    stream_cfg.device = CAPTURE_STREAM_DEVICE;
    stream_cfg.vol = TGT_ADC_VOL_LEVEL_7;
    stream_cfg.handler = af_capture;
    if (CAPTURE_STREAM_DEVICE == AUD_STREAM_USE_INT_CODEC ||
            CAPTURE_STREAM_DEVICE == AUD_STREAM_USE_INT_CODEC2 ||
            CAPTURE_STREAM_DEVICE == AUD_STREAM_USE_INT_CODEC3) {
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
    } else {
        stream_cfg.io_path = AUD_IO_PATH_NULL;
    }
    stream_cfg.data_ptr = capture_buf;
    stream_cfg.data_size = CAPTURE_SIZE;

    ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
    ASSERT(ret == 0, "af_stream_open capture failed: %d", ret);
#endif

#ifdef AF_TEST_TSF_TRIGGER
    hal_wifitsf_enable();
    if (0) {
#ifdef AF_INT_CODEC_TEST
    } else if (stream_cfg.device == AUD_STREAM_USE_INT_CODEC) {
        af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_WIFI, true);
        af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_WIFI, true);
#else
    } else if (stream_cfg.device == AUD_STREAM_USE_I2S0_MASTER) {
        af_i2s_sync_config(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, AF_I2S_SYNC_TYPE_WIFI, true);
#endif
    }
    wifitsf_set_trigger_time(5 * 1000000);
#endif

    ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);
#ifndef PLAYBACK_ONLY
    ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "af_stream_start capture failed: %d", ret);
#endif

#ifdef PLAYBACK_START_STOP_LOOP
    uint32_t time;
    int started;

    time = hal_sys_timer_get();
    started = true;
#endif

#ifndef RTOS
    while (1) {
        extern void af_thread(void const *argument);
        af_thread(NULL);

#ifdef PLAYBACK_START_STOP_LOOP
        if (started) {
            if (hal_sys_timer_get() - time > MS_TO_TICKS(8)) {
                ret = af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
                ASSERT(ret == 0, "af_stream_stop playback failed: %d", ret);
                time = hal_sys_timer_get();
                started = false;
            }
        } else {
            if (hal_sys_timer_get() - time > MS_TO_TICKS(2)) {
                ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
                ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);
                time = hal_sys_timer_get();
                started = true;
            }

        }
#endif
    }
#endif
}
