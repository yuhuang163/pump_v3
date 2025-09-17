/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#include <string.h>
#include "plat_types.h"
#include "hal_trace.h"
#include "hal_location.h"
#include "audioflinger.h"
#include "app_utils.h"
#include "app_overlay.h"
#include "bluetooth_bt_api.h"
#include "bt_sco_codec.h"
#include "speech_memory.h"
#include "algo_process.h"
#include "bth_sco_stream.h"

//#define SPEECH_ENABLE

#define SAMPLE_BITS         (AUD_BITS_16)
#define SAMPLE_BYTES        (2 * ((SAMPLE_BITS + 8) / 16))

static hfp_sco_codec_t current_sco_codec_type;

#define CODEC_PP_BUFF_SIZE (960)
#define BTPCM_PP_BUFF_SIZE (480)

static void adjust_btpcm_msbc_tx(uint8_t *buf, uint32_t len);

static SRAM_BSS_LOC uint8_t __attribute__((aligned(4))) codec_capture[CODEC_PP_BUFF_SIZE];
static SRAM_BSS_LOC uint8_t __attribute__((aligned(4))) codec_playback[CODEC_PP_BUFF_SIZE];
static SRAM_BSS_LOC uint8_t __attribute__((aligned(4))) btpcm_capture[BTPCM_PP_BUFF_SIZE];
static SRAM_BSS_LOC uint8_t __attribute__((aligned(4))) btpcm_playback[BTPCM_PP_BUFF_SIZE];
static SRAM_BSS_LOC uint8_t __attribute__((aligned(4))) speech_ref_cache[CODEC_PP_BUFF_SIZE/2];
POSSIBLY_UNUSED static SRAM_BSS_LOC uint8_t __attribute__((aligned(4))) speech_codec_play_cache[CODEC_PP_BUFF_SIZE/2];
POSSIBLY_UNUSED static SRAM_BSS_LOC uint8_t __attribute__((aligned(4))) speech_codec_capture_cache[CODEC_PP_BUFF_SIZE/2];

volatile static bool sco_call_runing;
static SRAM_BSS_LOC uint8_t __attribute__((aligned(4))) codec_playback_cache[CODEC_PP_BUFF_SIZE/2];
static SRAM_BSS_LOC uint8_t __attribute__((aligned(4))) btpcm_playback_cache[BTPCM_PP_BUFF_SIZE/2];

#ifdef BT_SCO_CODEC_SMF
extern const bt_sco_codec_t bt_sco_codec_msbc_smf;
extern const bt_sco_codec_t bt_sco_codec_cvsd_smf;
#else
extern const bt_sco_codec_t bt_sco_codec_msbc;
extern const bt_sco_codec_t bt_sco_codec_cvsd;
#endif
static const bt_sco_codec_t *g_bt_sco_codec_ptr = NULL;

static int sco_stream_get_frame_size(int fs, int ch, int ms)
{
    return (fs / 1000 * ch * ms);
}

static int sco_stream_get_codecpcm_buf_len(int fs, int ch, int ms)
{
    return sco_stream_get_frame_size(fs, ch, ms) * 2 * 2;
}


static int speech_get_btpcm_buf_len(hfp_sco_codec_t sco_codec, int fs,int ms)
{
    //RAW size per ms(Bytes).
    int buf_len=(fs/1000)*ms*2;

    //after encoding.
    switch(sco_codec) {
        case BT_HFP_SCO_CODEC_CVSD: {
            buf_len=buf_len/2;
        }
        break;
        case BT_HFP_SCO_CODEC_MSBC: {
            buf_len=buf_len/4;
        }
        break;
        default: {
            ASSERT(0, "sco codec is not correct!");
        }
        break;
    }

    //after BES platform.
    buf_len=buf_len*2;

    //add private data.
#if defined(PCM_PRIVATE_DATA_FLAG)
    buf_len=buf_len+BTPCM_PRIVATE_DATA_LENGTH*((int)(ms/7.5f));
#endif

    //after pingpang
    buf_len=buf_len*2;

    //if low delay. we will use half buffer(should be 7.5ms);
#if defined(LOW_DELAY_SCO)
    buf_len=buf_len/2;
#endif

    return buf_len;
}



uint32_t bt_sco_codec_capture_cb(uint8_t *buf, uint32_t len)
{
    POSSIBLY_UNUSED uint8_t *speech_data[PCM_CHANNEL_INDEX_QTY];
    if(!sco_call_runing) {
        return 0;
    }

    for (uint32_t i = 0; i < SPEECH_CODEC_CAPTURE_CHANNEL_NUM; i++) {
        // If buf is multi-MIC, the buf needs to be de-interlaved first
        speech_data[i] = buf;
    }

    speech_data[PCM_CHANNEL_INDEX_REF] = speech_ref_cache;
#ifdef SPEECH_ENABLE
    algo_process_call_capture.process(speech_data, speech_codec_capture_cache, len/2);
#endif

    if (current_sco_codec_type != BT_HFP_SCO_CODEC_CVSD) {
        g_bt_sco_codec_ptr->encoder(btpcm_playback_cache, len/4, buf, len);
        adjust_btpcm_msbc_tx(btpcm_playback_cache, len/4);
    }
    else {
        g_bt_sco_codec_ptr->encoder(btpcm_playback_cache, len, buf, len);
    }

    return 0;
}

uint32_t bt_sco_codec_playback_cb(uint8_t *buf, uint32_t len)
{
    if(!sco_call_runing) {
        return 0;
    }

#ifdef SPEECH_ENABLE
    algo_process_call_playback.process((uint8_t *)&codec_playback_cache, buf, len/2);
#else
    memcpy(buf, codec_playback_cache, len);
#endif
    memcpy(speech_ref_cache, buf, len);
    return 0;
}

uint32_t bt_sco_btpcm_capture_cb(uint8_t *buf, uint32_t len)
{
    if(!sco_call_runing) {
        return 0;
    }
    //TRACE(0, "%s", __func__);
    //DUMP8("%02x ", buf, 16);
    uint8_t *source = buf;
    uint32_t source_len = len;

    if (current_sco_codec_type != BT_HFP_SCO_CODEC_CVSD) {
        uint16_t *source_u16 = (uint16_t *)source;
        for (uint32_t i = 0; i < source_len/2; i++) {
            source[i] = (source_u16[i] >> 8);
        }
        source_len >>= 1;
        g_bt_sco_codec_ptr->decoder(source, source_len, codec_playback_cache, len*2);
    }
    else {
        g_bt_sco_codec_ptr->decoder(source, source_len, codec_playback_cache, len);
    }
    //TRACE(0, "source data:");
    //DUMP8("%02x ", source, 16);
    //process_downlink_bt_voice_frames(buf, len, codec_playback_cache, len*2);
    //bt_sco_msbc_decode(buf, len, codec_playback_cache, &out_len);
    return 0;
}

uint32_t bt_sco_btpcm_playback_cb(uint8_t *buf, uint32_t len)
{
    if(!sco_call_runing) {
        return 0;
    }

    memcpy(buf, btpcm_playback_cache, len);

    return 0;
}

#define SCO_MEM_POOL_SIZE  (12*1024)
uint8_t g_bt_sco_memory[SCO_MEM_POOL_SIZE];

int bt_sco_player_start(uint8_t codec_type)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
    enum AUD_SAMPRATE_T  stream_samprate = AUD_SAMPRATE_NULL;
    //bool decoder_open_ret, encoder_open_ret;

    hal_trace_set_log_level(TR_LEVEL_DEBUG);
    app_sysfreq_req(APP_SYSFREQ_USER_BT_SCO, APP_SYSFREQ_104M);

    sco_call_runing = false;

    speech_heap_init(g_bt_sco_memory, SCO_MEM_POOL_SIZE);
    app_overlay_select(APP_OVERLAY_HFP);

    current_sco_codec_type = (hfp_sco_codec_t)codec_type;

    if(current_sco_codec_type == BT_HFP_SCO_CODEC_MSBC) {
#ifdef BT_SCO_CODEC_SMF
        g_bt_sco_codec_ptr = &bt_sco_codec_msbc_smf;
#else
        g_bt_sco_codec_ptr = &bt_sco_codec_msbc;
#endif
        stream_samprate = AUD_SAMPRATE_16000;
    }
    else if(current_sco_codec_type == BT_HFP_SCO_CODEC_CVSD) {
#ifdef BT_SCO_CODEC_SMF
        g_bt_sco_codec_ptr = &bt_sco_codec_cvsd_smf;
#else
        g_bt_sco_codec_ptr = &bt_sco_codec_cvsd;
#endif
        stream_samprate = AUD_SAMPRATE_8000;
    }
    else {
        ASSERT(g_bt_sco_codec_ptr!=NULL, "invalid codec type :%d",current_sco_codec_type);
    }
    TRACE(0, "%s codec type:%d",__func__, (uint32_t)current_sco_codec_type);

    g_bt_sco_codec_ptr->init((uint32_t)stream_samprate);

    memset(&stream_cfg, 0, sizeof(stream_cfg));
    stream_cfg.bits = SAMPLE_BITS;
    stream_cfg.sample_rate = stream_samprate;
    stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)AUD_CHANNEL_MAP_CH0;
    stream_cfg.channel_num = SPEECH_CODEC_CAPTURE_CHANNEL_NUM;
    stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
    stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
    stream_cfg.vol = 12;
    stream_cfg.data_ptr = codec_capture;
    stream_cfg.data_size = sco_stream_get_codecpcm_buf_len(stream_samprate, SPEECH_CODEC_CAPTURE_CHANNEL_NUM, SPEECH_SCO_FRAME_MS);
    stream_cfg.handler = bt_sco_codec_capture_cb;
    af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

    stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
    stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
    stream_cfg.data_ptr = codec_playback;
    stream_cfg.handler  = bt_sco_codec_playback_cb;
    af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

    hal_cmu_btpcm_sel_cfg(true);
#ifdef SPEECH_ENABLE
    algo_process_cfg_t  cap_speech_cfg = {0};
    cap_speech_cfg.sample_rate = stream_samprate;
    cap_speech_cfg.sample_bytes = SAMPLE_BYTES;
    cap_speech_cfg.frame_len = stream_cfg.data_size / cap_speech_cfg.sample_bytes / 2;
    cap_speech_cfg.mic_num = SPEECH_CODEC_CAPTURE_CHANNEL_NUM;
    cap_speech_cfg.mode = 0;
    cap_speech_cfg.delay = 0;
    algo_process_call_capture.open(&cap_speech_cfg);

    algo_process_cfg_t  play_speech_cfg = {0};
    play_speech_cfg.sample_rate = stream_samprate;
    play_speech_cfg.sample_bytes = SAMPLE_BYTES;
    play_speech_cfg.frame_len = stream_cfg.data_size / play_speech_cfg.sample_bytes / 2;
    play_speech_cfg.mic_num = 1;
    play_speech_cfg.mode = 0;
    play_speech_cfg.delay = 0;
    algo_process_call_playback.open(&play_speech_cfg);
#endif
    stream_cfg.data_size = speech_get_btpcm_buf_len(current_sco_codec_type, stream_samprate, SPEECH_SCO_FRAME_MS);
    stream_cfg.device = AUD_STREAM_USE_BT_PCM;
    stream_cfg.io_path = AUD_IO_PATH_NULL;
    stream_cfg.data_ptr = btpcm_capture;
    stream_cfg.handler  = bt_sco_btpcm_capture_cb;
    af_stream_open(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE, &stream_cfg);

    stream_cfg.device = AUD_STREAM_USE_BT_PCM;
    stream_cfg.io_path = AUD_IO_PATH_NULL;
    stream_cfg.data_ptr = btpcm_playback;
    stream_cfg.handler  = bt_sco_btpcm_playback_cb;
    af_stream_open(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK, &stream_cfg);

    af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);

    af_stream_start(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
    af_stream_start(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);

    sco_call_runing = true;
    return 0;
}

int bt_sco_player_stop(void)
{
    sco_call_runing = false;

    af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    af_stream_stop(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
    af_stream_stop(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);

    af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    af_stream_close(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
    af_stream_close(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);

#ifdef SPEECH_ENABLE
    algo_process_call_capture.close();
    algo_process_call_playback.close();
#endif
    app_sysfreq_req(APP_SYSFREQ_USER_BT_SCO, APP_SYSFREQ_32K);

    return 0;
}

#define PCM_FAST_MODE

#ifdef TX_RX_PCM_MASK
#define BTPCM_TX_OFFSET_BYTES   (0)
#else
#if defined (PCM_FAST_MODE)
#define BTPCM_TX_OFFSET_BYTES   (1)
#else
#define BTPCM_TX_OFFSET_BYTES   (2)
#endif
#endif

#define SCO_MSBC_PACKET_SIZE    (60)

#ifndef PCM_PRIVATE_DATA_FLAG
static uint8_t g_btpcm_tx_buf[BTPCM_TX_OFFSET_BYTES];
#endif
static void adjust_btpcm_msbc_tx(uint8_t *buf, uint32_t len)
{
    uint16_t *buf_u16 = (uint16_t *)buf;
#ifndef PCM_PRIVATE_DATA_FLAG
    uint8_t *buf_ptr = (uint8_t *)buf;
    uint32_t loop_cnt = len / SCO_MSBC_PACKET_SIZE;

    ASSERT(len % SCO_MSBC_PACKET_SIZE == 0, "[%s] len(%d) is invalid!", __func__, len);

    // Shift
    for (uint32_t cnt=0; cnt <loop_cnt; cnt++) {
        for (uint32_t i=0; i<BTPCM_TX_OFFSET_BYTES; i++) {
            g_btpcm_tx_buf[i] = buf_ptr[i];
        }

        for (uint32_t i=0; i<SCO_MSBC_PACKET_SIZE - BTPCM_TX_OFFSET_BYTES; i++) {
            buf_ptr[i] = buf_ptr[i + BTPCM_TX_OFFSET_BYTES];
        }

        for (uint32_t i=0; i<BTPCM_TX_OFFSET_BYTES; i++) {
            buf_ptr[i + SCO_MSBC_PACKET_SIZE - BTPCM_TX_OFFSET_BYTES] = g_btpcm_tx_buf[i];
        }

        buf_ptr += SCO_MSBC_PACKET_SIZE;
    }
#endif
    // BTCPM trans data with 16bits format and valid data is in high 8 bits.
    for (int32_t i = len-1; i >= 0; i--) {
        buf_u16[i] = ((int16_t)buf[i]) << 8;
    }

}